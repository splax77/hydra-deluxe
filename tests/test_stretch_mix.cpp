// Tests for audio/stretch_mix: the Preview's audio at another song speed. No
// device is opened; read() and seek() are what the Playhead calls.
//
// The signal is silence with one impulse, so where the impulse comes out tells
// which file frame a played frame came from. Every check reads the impulse
// inside the first stretch of audio after a restart, which SoundTouch copies
// straight through (stretch_mix.cpp, kProbeFrame, says why that holds).

#include "doctest.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "audio/decode.h"
#include "audio/player.h"
#include "audio/stretch_mix.h"
#include "audio_util.h"
#include "core/error_kind.h"

using namespace hydra::audio;
using testaudio::BufferSource;
using testaudio::read_frames;

namespace {

constexpr int kRate = 48000;
constexpr int kChannels = 2;

// `frames` frames of stereo silence at 48 kHz, with a full-scale impulse on
// both channels at `impulse_at` (none when it is negative).
DecodedAudio impulse_audio(int64_t frames, int64_t impulse_at) {
    DecodedAudio a;
    a.channels = kChannels;
    a.sample_rate = kRate;
    a.samples.assign(static_cast<std::size_t>(frames) * kChannels, 0.0f);
    if (impulse_at >= 0)
        for (int c = 0; c < kChannels; ++c)
            a.samples[static_cast<std::size_t>(impulse_at) * kChannels + c] = 1.0f;
    return a;
}

std::unique_ptr<MixSource> impulse_source(int64_t frames, int64_t impulse_at) {
    return std::make_unique<BufferSource>(impulse_audio(frames, impulse_at));
}

// The frame of `got` (interleaved stereo) whose left sample is loudest, or -1
// when every frame is silent.
int64_t loudest_frame(const std::vector<float>& got) {
    int64_t at = -1;
    float loudest = 0.0f;
    for (std::size_t i = 0; i * kChannels < got.size(); ++i) {
        const float v = std::fabs(got[i * kChannels]);
        if (v > loudest) {
            loudest = v;
            at = static_cast<int64_t>(i);
        }
    }
    return at;
}

// How far after the seek's own file frame each test puts the impulse, and how
// many played frames it reads to find it.
constexpr int64_t kAfter = 100;
constexpr int64_t kLook = 2 * kAfter;

}  // namespace

TEST_CASE("StretchSource: 200% plays half the file's frames, 50% twice as many") {
    constexpr int64_t kFile = kRate;  // one second
    struct Case {
        double ratio;
        int64_t played;
        double played_ms;
    };
    for (const Case c : {Case{2.0, 24000, 500.0}, Case{0.5, 96000, 2000.0}}) {
        CAPTURE(c.ratio);
        StretchSource src(impulse_source(kFile, -1), c.ratio);
        CHECK(src.length_frames() == c.played);
        CHECK(src.channels() == kChannels);
        CHECK(src.sample_rate() == kRate);
        // Reading to the end delivers exactly that many frames, then stops.
        const std::vector<float> all = read_frames(src, 2 * c.played);
        CHECK(static_cast<int64_t>(all.size()) == c.played * kChannels);
        std::vector<float> more(16 * kChannels);
        CHECK(src.read(more.data(), 16) == 0);

        // The Playhead counts the same played frames.
        Playhead p(std::make_unique<StretchSource>(impulse_source(kFile, -1), c.ratio));
        CHECK(p.length_frames() == c.played);
        CHECK(p.length_ms() == doctest::Approx(c.played_ms));
    }
}

TEST_CASE("StretchSource: a seek to played frame F plays file frame F x ratio first") {
    constexpr int64_t kFile = 4 * kRate;
    struct Case {
        double ratio;
        int64_t played;
        int64_t file;  // played x ratio
    };
    for (const Case c : {Case{0.5, 24000, 12000}, Case{1.5, 24000, 36000}, Case{2.0, 24000, 48000},
                         Case{2.0, 0, 0}, Case{0.5, 0, 0}}) {
        CAPTURE(c.ratio);
        CAPTURE(c.played);
        StretchSource src(impulse_source(kFile, c.file + kAfter), c.ratio);
        CAPTURE(src.start_delay_frames());
        CHECK(src.file_frame_of(c.played) == c.file);
        src.seek(c.played);
        CHECK(loudest_frame(read_frames(src, kLook)) == kAfter);
    }
}

TEST_CASE("StretchSource: through the Playhead, a seek in played ms lands at played ms x ratio in the file") {
    // At 200%, played 500 ms is file 1000 ms (48000 frames).
    Playhead p(std::make_unique<StretchSource>(impulse_source(4 * kRate, 48000 + kAfter), 2.0));
    p.seek_ms(500.0);
    p.play();
    std::vector<float> got(static_cast<std::size_t>(kLook) * kChannels);
    CHECK(p.read_frames(got.data(), kLook) == kLook);
    CHECK(loudest_frame(got) == kAfter);
    CHECK(p.position_frames() == 24000 + kLook);  // played 500 ms is 24000 frames, plus the read
}

// The start delay is measured with an impulse when the source is built
// (start_delay_frames), and every restart has to correct for it, not only the
// first. These seeks restart it in each way the Playhead can: fresh, twice in
// a row, after reading partway, back to the start, and at tempo 1.
TEST_CASE("StretchSource: the start-delay correction holds at every seek") {
    constexpr int64_t kFile = 4 * kRate;
    for (const double ratio : {0.5, 1.0, 1.5, 2.0}) {
        CAPTURE(ratio);
        // The impulse sits kAfter frames into the file.
        StretchSource src(impulse_source(kFile, kAfter), ratio);
        CAPTURE(src.start_delay_frames());

        // Fresh: the constructor leaves it at played frame 0.
        CHECK(loudest_frame(read_frames(src, kLook)) == kAfter);

        // Read on, then seek back to the start.
        read_frames(src, 30000);
        src.seek(0);
        CHECK(loudest_frame(read_frames(src, kLook)) == kAfter);

        // Two seeks with no read between: only the last one counts.
        src.seek(40000);
        src.seek(0);
        CHECK(loudest_frame(read_frames(src, kLook)) == kAfter);
    }
}

TEST_CASE("play_at_speed: at 100% the plain mix plays untouched") {
    std::unique_ptr<MixSource> plain = testaudio::ramp_source(4800);
    const MixSource* raw = plain.get();
    std::unique_ptr<MixSource> played = play_at_speed(std::move(plain), 1.0);
    CHECK(played.get() == raw);
    // So its samples are today's: the ramp, frame for frame.
    const std::vector<float> got = read_frames(*played, 4800);
    const DecodedAudio ramp = testaudio::make_ramp(4800);
    REQUIRE(got.size() == ramp.samples.size());
    CHECK(got == ramp.samples);

    // Any other speed wraps it.
    std::unique_ptr<MixSource> fast = play_at_speed(testaudio::ramp_source(4800), 1.5);
    CHECK(dynamic_cast<StretchSource*>(fast.get()) != nullptr);
    CHECK(fast->length_frames() == 3200);

    CHECK(play_at_speed(nullptr, 2.0).get() == nullptr);
}

TEST_CASE("StretchSource: no source, or a speed that is not above 0, is refused") {
    // Whether building a source at `ratio` throws an AudioDecode error.
    auto refused = [](std::unique_ptr<MixSource> plain, double ratio) {
        try {
            [[maybe_unused]] StretchSource s(std::move(plain), ratio);
        } catch (const hydra::KindedError& e) {
            return e.kind() == hydra::ErrorKind::AudioDecode;
        }
        return false;
    };
    CHECK(refused(nullptr, 2.0));
    CHECK(refused(impulse_source(480, -1), 0.0));
    CHECK(refused(impulse_source(480, -1), -1.0));
    CHECK(refused(impulse_source(480, -1), std::numeric_limits<double>::quiet_NaN()));
    CHECK_FALSE(refused(impulse_source(480, -1), 2.0));
}
