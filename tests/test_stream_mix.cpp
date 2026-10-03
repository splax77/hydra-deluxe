// Tests for audio/stream_mix: the streaming mix the Preview plays from, and the
// Playhead's pull from a MixSource.
//
// The reference is today's path: decode every stem whole, mix_stems() them,
// and put the front pad in front. Reading a StreamMix start to end must give
// the same frames (within float rounding of resampling block by block), and a
// seek must land on the same audio (within 1e-3, after 20 ms of warm-up).

#include "doctest.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "audio/decode.h"
#include "audio/mixer.h"
#include "audio/player.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "core/winstr.h"

using namespace hydra::audio;

// ---- Heap-allocation counter -------------------------------------------------
// Replaces the global operator new for the test binary (a plain malloc
// pass-through). It only counts while g_count_allocs is set on this thread, so
// a test can prove a block of code never allocates.
namespace {
thread_local bool g_count_allocs = false;
std::atomic<int> g_allocs{0};
}  // namespace

void* operator new(std::size_t n) {
    if (g_count_allocs) ++g_allocs;
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace {

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

constexpr double kPi = 3.14159265358979323846;

void put_u16(std::vector<uint8_t>& o, uint16_t n) {
    o.push_back(static_cast<uint8_t>(n));
    o.push_back(static_cast<uint8_t>(n >> 8));
}
void put_u32(std::vector<uint8_t>& o, uint32_t n) {
    for (int i = 0; i < 4; ++i) o.push_back(static_cast<uint8_t>(n >> (8 * i)));
}

// A PCM16 WAV of `seconds` of a sine per channel (channel c at freq * (c + 1)),
// so every channel and every stem is recognizably different.
std::vector<uint8_t> tone_wav(int channels, uint32_t rate, double seconds, double freq) {
    const auto frames = static_cast<uint32_t>(rate * seconds);
    const uint32_t data_len = frames * channels * 2;
    std::vector<uint8_t> o;
    o.insert(o.end(), {'R', 'I', 'F', 'F'});
    put_u32(o, 36 + data_len);
    o.insert(o.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put_u32(o, 16);
    put_u16(o, 1);  // PCM
    put_u16(o, static_cast<uint16_t>(channels));
    put_u32(o, rate);
    put_u32(o, rate * channels * 2);
    put_u16(o, static_cast<uint16_t>(channels * 2));
    put_u16(o, 16);
    o.insert(o.end(), {'d', 'a', 't', 'a'});
    put_u32(o, data_len);
    for (uint32_t i = 0; i < frames; ++i)
        for (int c = 0; c < channels; ++c) {
            const double v = 0.4 * std::sin(2 * kPi * freq * (c + 1) * i / rate);
            put_u16(o, static_cast<uint16_t>(static_cast<int16_t>(std::lround(v * 32767))));
        }
    return o;
}

std::vector<uint8_t> fixture(const std::string& name) {
    return hydra::read_file_bytes(std::string(HYDRA_TESTDATA_DIR) + "/audio/" + name);
}

std::unique_ptr<StemReader> open_bytes(const std::vector<uint8_t>& bytes) {
    return open_stem_reader(StemBytes{bytes, nullptr});
}

StreamMix make_mix(const std::vector<std::vector<uint8_t>>& stems, int64_t pad) {
    std::vector<std::unique_ptr<StemReader>> readers;
    for (const auto& b : stems) readers.push_back(open_bytes(b));
    return StreamMix(std::move(readers), 48000, 2, pad);
}

// Today's path: decode each stem whole, mix, then prepend the pad.
DecodedAudio reference(const std::vector<std::vector<uint8_t>>& stems, int64_t pad) {
    std::vector<DecodedAudio> decoded;
    for (const auto& b : stems) decoded.push_back(decode_audio(b));
    DecodedAudio mix = mix_stems(decoded, 48000, 2);
    mix.samples.insert(mix.samples.begin(), static_cast<std::size_t>(pad) * 2, 0.0f);
    return mix;
}

// Reads `frames` frames (or to the end) in `block`-frame pieces.
std::vector<float> read_frames(MixSource& src, int64_t frames, int64_t block = 511) {
    std::vector<float> out;
    std::vector<float> buf(static_cast<std::size_t>(block) * src.channels());
    int64_t left = frames;
    while (left > 0) {
        const int64_t k = src.read(buf.data(), std::min(block, left));
        if (k == 0) break;
        out.insert(out.end(), buf.begin(), buf.begin() + k * src.channels());
        left -= k;
    }
    return out;
}

double max_diff(const std::vector<float>& a, const float* b, std::size_t from, std::size_t count) {
    double worst = 0.0;
    for (std::size_t i = from; i < count; ++i)
        worst = std::max(worst, std::fabs(static_cast<double>(a[i]) - b[i]));
    return worst;
}

// Each case: the stems and the front pad.
struct MixCase {
    const char* name;
    std::vector<std::vector<uint8_t>> stems;
    int64_t pad;
};

std::vector<MixCase> mix_cases() {
    return {
        {"two same-rate stems, different lengths",
         {tone_wav(2, 48000, 1.0, 220), tone_wav(2, 48000, 0.6, 330)}, 0},
        {"two 44.1 kHz stems, different lengths",
         {tone_wav(1, 44100, 0.7, 250), tone_wav(2, 44100, 1.1, 180)}, 0},
        {"44.1 kHz mono + 48 kHz stereo",
         {tone_wav(1, 44100, 1.0, 220), tone_wav(2, 48000, 0.8, 330)}, 0},
        {"44.1 kHz mono + 48 kHz stereo, 250 ms front pad",
         {tone_wav(1, 44100, 1.0, 220), tone_wav(2, 48000, 0.8, 330)}, 12000},
    };
}

// Wraps a real reader and stops after `fail_at` frames, as a decode error does.
class FailingReader : public StemReader {
public:
    FailingReader(std::unique_ptr<StemReader> inner, int64_t fail_at)
        : inner_(std::move(inner)), fail_at_(fail_at) {}
    int channels() const override { return inner_->channels(); }
    int sample_rate() const override { return inner_->sample_rate(); }
    int64_t length_frames() const override { return inner_->length_frames(); }
    int64_t read(float* out, int64_t frames) override {
        const int64_t n = std::min(frames, fail_at_ - pos_);
        if (n <= 0) return 0;
        const int64_t got = inner_->read(out, n);
        pos_ += got;
        return got;
    }
    void seek(int64_t frame) override {
        inner_->seek(frame);
        pos_ = std::clamp<int64_t>(frame, 0, length_frames());
    }

private:
    std::unique_ptr<StemReader> inner_;
    int64_t fail_at_;
    int64_t pos_ = 0;
};

// A MixSource that records what the Playhead asks of it.
class CountingSource : public MixSource {
public:
    int channels() const override { return 2; }
    int sample_rate() const override { return 48000; }
    int64_t length_frames() const override { return 48000; }
    int64_t read(float* out, int64_t frames) override {
        ++reads;
        for (int64_t i = 0; i < frames * 2; ++i) out[i] = 0.25f;
        pos += frames;
        return frames;
    }
    void seek(int64_t frame) override {
        ++seeks;
        last_seek = frame;
        pos = frame;
    }
    int reads = 0;
    int seeks = 0;
    int64_t last_seek = -1;
    int64_t pos = 0;
};

}  // namespace

TEST_CASE("StreamMix: reading start to end equals mix_stems of every decoded stem") {
    for (const MixCase& c : mix_cases()) {
        CAPTURE(c.name);
        DecodedAudio ref = reference(c.stems, c.pad);
        StreamMix mix = make_mix(c.stems, c.pad);
        CHECK(mix.channels() == 2);
        CHECK(mix.sample_rate() == 48000);
        CHECK(mix.length_frames() == ref.frames());
        std::vector<float> got = read_frames(mix, mix.length_frames() + 1000);
        REQUIRE(got.size() == ref.samples.size());
        CHECK(max_diff(got, ref.samples.data(), 0, got.size()) <= 1e-6);
        // The pad really is silence.
        CHECK(std::all_of(got.begin(), got.begin() + c.pad * 2, [](float v) { return v == 0.0f; }));
        // At the end: nothing more.
        float tail[8];
        CHECK(mix.read(tail, 4) == 0);
    }
}

TEST_CASE("StreamMix: a seek back to 0 replays the start exactly, every channel") {
    // Guards the converter restart: miniaudio 0.11.25's ma_data_converter_reset
    // wrecks the low-pass filter (see stream_mix.cpp), so a resampled stereo
    // stem would come back different after a seek.
    const auto w = tone_wav(2, 44100, 1.0, 220);
    StreamMix mix = make_mix({w}, 0);
    const std::vector<float> first = read_frames(mix, 9600);
    mix.seek(20000);
    read_frames(mix, 3000);
    mix.seek(0);
    const std::vector<float> again = read_frames(mix, 9600);
    CHECK(again == first);
}

TEST_CASE("StreamMix: a passthrough stem seeks exactly") {
    const auto w = tone_wav(2, 48000, 1.0, 220);
    DecodedAudio full = decode_audio(w);
    StreamMix mix = make_mix({w}, 0);
    mix.seek(7777);
    const std::vector<float> got = read_frames(mix, 4800);
    REQUIRE(got.size() == 9600);
    CHECK(std::equal(got.begin(), got.end(), full.samples.begin() + 7777 * 2));
}

TEST_CASE("StreamMix: an aligned seek on a resampled stem matches a straight read") {
    // 44.1 -> 48 kHz lines up every 160 output frames; a seek there restarts
    // the converter on the same input frame a straight read passes through.
    const auto w = tone_wav(1, 44100, 1.0, 220);
    DecodedAudio full = reference({w}, 0);
    StreamMix mix = make_mix({w}, 0);
    mix.seek(160 * 100);
    const std::vector<float> got = read_frames(mix, 4800);
    REQUIRE(got.size() == 9600);
    // After 20 ms of filter warm-up, the audio is the straight read's audio.
    const float* want = full.samples.data() + static_cast<std::size_t>(160 * 100) * 2;
    CHECK(max_diff(got, want, 960 * 2, got.size()) <= 1e-3);
    // And it is the same tone at the same phase: the zero crossings line up.
    int crossings_got = 0, crossings_want = 0;
    for (std::size_t i = 2; i < got.size(); i += 2) {
        if ((got[i - 2] < 0.0f) != (got[i] < 0.0f)) ++crossings_got;
        if ((want[i - 2] < 0.0f) != (want[i] < 0.0f)) ++crossings_want;
    }
    CHECK(std::abs(crossings_got - crossings_want) <= 1);
}

TEST_CASE("StreamMix: an odd sample rate plays exactly and seeks within one input frame") {
    // 44056 Hz lines up with 48 kHz only every 6000 output frames, over a
    // block, so a seek restarts at the target itself (within one input frame)
    // rather than converting up to 6000 frames to throw away.
    const auto w = tone_wav(1, 44056, 1.0, 220);
    DecodedAudio ref = reference({w}, 0);
    StreamMix mix = make_mix({w}, 0);
    REQUIRE(mix.length_frames() == ref.frames());
    std::vector<float> got = read_frames(mix, mix.length_frames());
    REQUIRE(got.size() == ref.samples.size());
    CHECK(max_diff(got, ref.samples.data(), 0, got.size()) <= 1e-6);
    // One input frame of a 0.4-amplitude 220 Hz sine moves it at most
    // 0.4 * 2 * pi * 220 / 44056 = 0.0126.
    for (int64_t f : {int64_t{6000 * 3}, int64_t{12345}, int64_t{31111}}) {
        CAPTURE(f);
        mix.seek(f);
        std::vector<float> part = read_frames(mix, 4800);
        CHECK(max_diff(part, ref.samples.data() + f * 2, 960 * 2, part.size()) <= 0.013);
    }
}

TEST_CASE("StreamMix: length is the front pad plus the longest converted stem") {
    const auto a = tone_wav(1, 44100, 1.0, 220);  // 44100 -> 48000 frames
    const auto b = tone_wav(2, 48000, 0.5, 330);  // 24000 frames
    StreamMix mix = make_mix({a, b}, 4800);
    CHECK(mix.length_frames() == 4800 + 48000);
    // Past the shorter stem's end, only the longer one sounds.
    DecodedAudio only_a = reference({a}, 4800);
    std::vector<float> got = read_frames(mix, mix.length_frames());
    const std::size_t from = static_cast<std::size_t>(4800 + 24000) * 2;
    CHECK(max_diff(got, only_a.samples.data(), from, got.size()) <= 1e-6);
}

TEST_CASE("StreamMix: reading in big blocks matches reading in small ones") {
    const MixCase c = mix_cases()[3];
    StreamMix small = make_mix(c.stems, c.pad);
    StreamMix big = make_mix(c.stems, c.pad);
    std::vector<float> a = read_frames(small, small.length_frames(), 97);
    std::vector<float> b = read_frames(big, big.length_frames(), 10000);  // > 4096
    REQUIRE(a.size() == b.size());
    CHECK(max_diff(a, b.data(), 0, a.size()) <= 1e-6);
}

TEST_CASE("StreamMix: a seek lands on the same audio as a straight read") {
    for (const MixCase& c : mix_cases()) {
        CAPTURE(c.name);
        DecodedAudio ref = reference(c.stems, c.pad);
        StreamMix mix = make_mix(c.stems, c.pad);
        const int64_t len = mix.length_frames();
        const int64_t warm = 48000 / 50;  // 20 ms
        for (int64_t f : {int64_t{0}, int64_t{1}, std::max<int64_t>(c.pad - 10, 0), c.pad + 1,
                          c.pad + 12345, len / 2, len - 5000, len - 100}) {
            CAPTURE(f);
            mix.seek(f);
            std::vector<float> got = read_frames(mix, 4800);
            const int64_t expect = std::min<int64_t>(4800, len - f);
            REQUIRE(static_cast<int64_t>(got.size()) == expect * 2);
            const std::size_t skip = static_cast<std::size_t>(std::min(warm, expect)) * 2;
            CHECK(max_diff(got, ref.samples.data() + f * 2, skip, got.size()) <= 1e-3);
            // Inside the front pad nothing is warming up: silence is exact.
            if (f < c.pad) {
                const std::size_t pad_left = static_cast<std::size_t>(std::min(c.pad - f, expect)) * 2;
                CHECK(std::all_of(got.begin(), got.begin() + pad_left, [](float v) { return v == 0.0f; }));
            }
        }
        // Past the end reads nothing; back to 0 restarts cleanly.
        mix.seek(len + 50);
        float tail[8];
        CHECK(mix.read(tail, 4) == 0);
        mix.seek(0);
        std::vector<float> again = read_frames(mix, len);
        REQUIRE(again.size() == ref.samples.size());
        CHECK(max_diff(again, ref.samples.data(), 0, again.size()) <= 1e-6);
    }
}

TEST_CASE("StreamMix: real fixtures mix and seek like the decoded mix") {
    // Opus (48 kHz), Vorbis and MP3 together: three different readers.
    const std::vector<std::vector<uint8_t>> stems = {
        fixture("sine220.opus"), fixture("sine220.ogg"), fixture("sine220.mp3")};
    DecodedAudio ref = reference(stems, 0);
    StreamMix mix = make_mix(stems, 0);
    REQUIRE(mix.length_frames() == ref.frames());
    std::vector<float> got = read_frames(mix, mix.length_frames());
    REQUIRE(got.size() == ref.samples.size());
    CHECK(max_diff(got, ref.samples.data(), 0, got.size()) <= 1e-6);
    const int64_t len = mix.length_frames();
    for (int64_t f : {int64_t{1}, len / 3, len / 2, len - 6000}) {
        CAPTURE(f);
        mix.seek(f);
        std::vector<float> part = read_frames(mix, 4800);
        CHECK(max_diff(part, ref.samples.data() + f * 2, 960 * 2, part.size()) <= 1e-3);
    }
}

TEST_CASE("StreamMix: a stem that stops early goes silent, the others keep playing") {
    const auto good = tone_wav(2, 48000, 1.0, 220);
    for (int bad_rate : {48000, 44100}) {
        CAPTURE(bad_rate);
        const auto bad = tone_wav(bad_rate == 48000 ? 2 : 1, static_cast<uint32_t>(bad_rate), 1.0, 330);
        const int64_t fail_at = bad_rate / 4;  // a quarter second in
        std::vector<std::unique_ptr<StemReader>> readers;
        readers.push_back(open_bytes(good));
        readers.push_back(std::make_unique<FailingReader>(open_bytes(bad), fail_at));
        StreamMix mix(std::move(readers), 48000, 2, 0);
        DecodedAudio both = reference({good, bad}, 0);
        DecodedAudio alone = reference({good}, 0);
        CHECK(mix.length_frames() == both.frames());
        std::vector<float> got = read_frames(mix, mix.length_frames());
        REQUIRE(got.size() == both.samples.size());
        // Before the failure: both stems (allow a few frames for the resampler).
        const std::size_t cut = static_cast<std::size_t>(12000 - 64) * 2;
        CHECK(max_diff(got, both.samples.data(), 0, cut) <= 1e-6);
        // Well after it: the good stem alone, still playing to the end.
        const std::size_t after = static_cast<std::size_t>(12000 + 64) * 2;
        CHECK(max_diff(got, alone.samples.data(), after, got.size()) <= 1e-6);
    }
}

TEST_CASE("StreamMix: read and seek never touch the heap after construction") {
    const MixCase c = mix_cases()[3];  // a resampled stem, a passthrough one, a pad
    StreamMix mix = make_mix(c.stems, c.pad);
    std::vector<float> buf(static_cast<std::size_t>(10000) * 2);
    g_allocs = 0;
    g_count_allocs = true;
    mix.read(buf.data(), 511);
    mix.read(buf.data(), 10000);  // more than one 4096-frame block
    mix.seek(30000);
    mix.read(buf.data(), 4096);
    mix.seek(5);
    mix.read(buf.data(), 7000);
    mix.seek(mix.length_frames() - 10);
    mix.read(buf.data(), 100);
    g_count_allocs = false;
    CHECK(g_allocs.load() == 0);
}

TEST_CASE("StreamMix: no stems is just the front pad") {
    StreamMix mix({}, 48000, 2, 480);
    CHECK(mix.length_frames() == 480);
    std::vector<float> got = read_frames(mix, 1000);
    CHECK(got.size() == 960);
    CHECK(std::all_of(got.begin(), got.end(), [](float v) { return v == 0.0f; }));
}

TEST_CASE("BufferSource plays a decoded buffer and seeks within it") {
    DecodedAudio a;
    a.channels = 1;
    a.sample_rate = 48000;
    a.samples = {0.f, 1.f, 2.f, 3.f, 4.f};
    BufferSource src(a);
    CHECK(src.length_frames() == 5);
    float out[8] = {};
    CHECK(src.read(out, 3) == 3);
    CHECK(out[2] == 2.f);
    src.seek(4);
    CHECK(src.read(out, 3) == 1);
    CHECK(out[0] == 4.f);
    CHECK(src.read(out, 3) == 0);
    src.seek(-7);
    CHECK(src.read(out, 1) == 1);
    CHECK(out[0] == 0.f);
}

TEST_CASE("Playhead: a seek only records; the source moves on the next playing read") {
    auto owned = std::make_unique<CountingSource>();
    CountingSource& src = *owned;
    Playhead p(std::move(owned));
    CHECK(p.length_frames() == 48000);
    std::vector<float> buf(256 * 2, 1.0f);

    p.seek_frames(100);
    CHECK(src.seeks == 0);
    // Paused: silence, and the source is left alone.
    CHECK(p.read_frames(buf.data(), 256) == 0);
    CHECK(std::all_of(buf.begin(), buf.end(), [](float v) { return v == 0.0f; }));
    CHECK(src.reads == 0);
    CHECK(src.seeks == 0);

    // Playing: one seek to the recorded position, then the read.
    p.play();
    CHECK(p.read_frames(buf.data(), 256) == 256);
    CHECK(src.seeks == 1);
    CHECK(src.last_seek == 100);
    CHECK(src.reads == 1);
    CHECK(buf[0] == 0.25f);
    CHECK(p.position_frames() == 356);

    // Carrying on needs no new seek.
    p.read_frames(buf.data(), 256);
    CHECK(src.seeks == 1);

    // Several seeks before a read: only the last one reaches the source.
    p.seek_frames(1000);
    p.seek_frames(2000);
    CHECK(src.seeks == 1);
    p.read_frames(buf.data(), 256);
    CHECK(src.seeks == 2);
    CHECK(src.last_seek == 2000);

    // Pause and resume in place: the source is already there.
    p.pause();
    p.play();
    p.read_frames(buf.data(), 256);
    CHECK(src.seeks == 2);

    // Gain scales what the source gave.
    p.set_gain(0.5f);
    p.read_frames(buf.data(), 4);
    CHECK(buf[0] == 0.125f);
}

TEST_CASE("Playhead plays a StreamMix to the end and auto-pauses") {
    const MixCase c = mix_cases()[3];
    DecodedAudio ref = reference(c.stems, c.pad);
    std::vector<std::unique_ptr<StemReader>> readers;
    for (const auto& b : c.stems) readers.push_back(open_bytes(b));
    Playhead p(std::make_unique<StreamMix>(std::move(readers), 48000, 2, c.pad));
    REQUIRE(p.length_frames() == ref.frames());
    p.play();
    std::vector<float> got;
    std::vector<float> buf(512 * 2);
    while (p.playing()) {
        const int64_t n = p.read_frames(buf.data(), 512);
        got.insert(got.end(), buf.begin(), buf.begin() + n * 2);
    }
    REQUIRE(got.size() == ref.samples.size());
    CHECK(max_diff(got, ref.samples.data(), 0, got.size()) <= 1e-6);
    CHECK(p.position_frames() == p.length_frames());
}
