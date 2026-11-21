// The audio test helpers the audio tests share: where a fixture under
// testdata/audio lives, its bytes, the FLAC fixture with its length zeroed, a
// small ID3 tag, a PCM16 WAV writer, a block
// reader and a sample-difference measure for a MixSource, a rough frequency
// check for the 220 Hz sine fixtures, and a recognizable stereo ramp with a
// source that plays it.

#ifndef HYDRA_TESTS_AUDIO_UTIL_H
#define HYDRA_TESTS_AUDIO_UTIL_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "doctest.h"

#include "bytes_util.h"

#include "audio/decode.h"
#include "audio/mixer.h"
#include "audio/stream_mix.h"
#include "core/winstr.h"
#include "testdata_dirs.h"  // testdata_root

namespace testaudio {

// The path of testdata/audio/<name>, for a stem opened from a file.
inline std::string fixture_path(const std::string& name) {
    return corpus::testdata_root() + "/audio/" + name;
}

// The bytes of testdata/audio/<name>, for a stem held in memory.
inline std::vector<uint8_t> read_fixture(const std::string& name) {
    return hydra::read_file_bytes(fixture_path(name));
}

// A small hand-built ID3v2.3 tag for the sniff and stem-reader tests: the
// header with `flags` and a syncsafe size of 20, 20 bytes of padding, and a
// footer when `flags` asks for one. Nothing reads the padding.
inline std::vector<uint8_t> id3_tag(int flags) {
    std::vector<uint8_t> t = {'I', 'D', '3', 3, 0, static_cast<uint8_t>(flags),
                              0,   0,   0,   20};
    t.resize(t.size() + 20 + ((flags & 0x10) ? 10 : 0), 0);
    return t;
}

// The FLAC fixture with its STREAMINFO total-samples field set to 0, which
// RFC 9639 reads as "length unknown" (an encoder writing to a pipe leaves it
// so). The 36-bit field follows the 20-bit sample rate, 3-bit channels and
// 5-bit bits-per-sample fields: the low 4 bits of STREAMINFO byte 13 and all of
// bytes 14 to 17, where STREAMINFO starts 8 bytes into the file ("fLaC" and
// the block header). Shared by test_stem_reader.cpp and test_stream_mix.cpp.
inline std::vector<uint8_t> flac_with_unknown_length() {
    std::vector<uint8_t> b = read_fixture("sine220.flac");
    REQUIRE(b.size() > 8 + 18);
    REQUIRE(std::memcmp(b.data(), "fLaC", 4) == 0);
    uint8_t* info = b.data() + 8;
    info[13] = static_cast<uint8_t>(info[13] & 0xF0);
    for (int i = 14; i <= 17; ++i) info[i] = 0;
    return b;
}

// A canonical 44-byte-header PCM16 WAV around `samples`, interleaved
// `channels` to a frame, at `rate` Hz. The numbers go through bytes_util.h.
inline std::vector<uint8_t> pcm16_wav(int channels, uint32_t rate,
                                      const std::vector<int16_t>& samples) {
    using testbytes::put_u16;
    using testbytes::put_u32;
    const uint32_t data_len = static_cast<uint32_t>(samples.size()) * 2;
    std::vector<uint8_t> o;
    o.insert(o.end(), {'R', 'I', 'F', 'F'});
    put_u32(o, 36 + data_len);
    o.insert(o.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    put_u32(o, 16);
    put_u16(o, 1);  // PCM
    put_u16(o, static_cast<uint16_t>(channels));
    put_u32(o, rate);
    put_u32(o, rate * channels * 2);                // byte rate
    put_u16(o, static_cast<uint16_t>(channels * 2));  // block align
    put_u16(o, 16);                                 // bits per sample
    o.insert(o.end(), {'d', 'a', 't', 'a'});
    put_u32(o, data_len);
    for (int16_t s : samples) put_u16(o, static_cast<uint16_t>(s));
    return o;
}

// `frames` stereo frames at 48 kHz where frame i holds {L=i, R=i+0.5}, so a
// copied block is trivially recognizable. Shared by test_audio_player.cpp and
// test_preview_transport.cpp.
inline hydra::audio::DecodedAudio make_ramp(int frames) {
    hydra::audio::DecodedAudio a;
    a.channels = 2;
    a.sample_rate = 48000;
    a.samples.resize(static_cast<std::size_t>(frames) * 2);
    for (int i = 0; i < frames; ++i) {
        a.samples[i * 2] = static_cast<float>(i);
        a.samples[i * 2 + 1] = static_cast<float>(i) + 0.5f;
    }
    return a;
}

// Plays a buffer that is already decoded, for a Playhead under test. Hydra
// itself plays a StreamMix; only tests hold a whole DecodedAudio.
class BufferSource : public hydra::audio::MixSource {
public:
    explicit BufferSource(hydra::audio::DecodedAudio audio)
        : audio_(std::move(audio)), length_(audio_.frames()) {}
    int channels() const override { return audio_.channels; }
    int sample_rate() const override { return audio_.sample_rate; }
    int64_t length_frames() const override { return length_; }
    int64_t read(float* out, int64_t frames) override {
        const int64_t n = std::clamp<int64_t>(length_ - pos_, 0, std::max<int64_t>(frames, 0));
        if (n > 0) {
            const std::size_t ch = static_cast<std::size_t>(audio_.channels);
            std::memcpy(out, audio_.samples.data() + static_cast<std::size_t>(pos_) * ch,
                        static_cast<std::size_t>(n) * ch * sizeof(float));
            pos_ += n;
        }
        return n;
    }
    void seek(int64_t frame) override { pos_ = std::clamp<int64_t>(frame, 0, length_); }

private:
    hydra::audio::DecodedAudio audio_;
    int64_t length_ = 0;
    int64_t pos_ = 0;
};

// make_ramp(frames) as a source a Playhead plays.
inline std::unique_ptr<hydra::audio::MixSource> ramp_source(int frames) {
    return std::make_unique<BufferSource>(make_ramp(frames));
}

// Reads `frames` frames from `src` (or to its end) in `block`-frame pieces.
inline std::vector<float> read_frames(hydra::audio::MixSource& src, int64_t frames,
                                      int64_t block = 511) {
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

// The largest sample difference between a[from, count) and b[from, count).
inline double max_diff(const std::vector<float>& a, const float* b, std::size_t from,
                       std::size_t count) {
    double worst = 0.0;
    for (std::size_t i = from; i < count; ++i)
        worst = std::max(worst, std::fabs(static_cast<double>(a[i]) - b[i]));
    return worst;
}

// Dominant frequency of one channel, from zero crossings over the middle half
// of the signal (the edges carry encoder padding and fades). Good enough to
// confirm decoded or mixed audio really is the tone, not silence or garbage.
inline double estimate_freq_hz(const hydra::audio::DecodedAudio& a, int channel) {
    if (a.channels <= 0 || a.frames() < 4) return 0.0;
    const int64_t n = a.frames(), lo = n / 4, hi = n - n / 4;
    int crossings = 0;
    float prev = a.samples[static_cast<std::size_t>(lo) * a.channels + channel];
    for (int64_t i = lo + 1; i < hi; ++i) {
        const float s = a.samples[static_cast<std::size_t>(i) * a.channels + channel];
        if ((prev < 0.0f && s >= 0.0f) || (prev >= 0.0f && s < 0.0f)) ++crossings;
        prev = s;
    }
    const double dur = static_cast<double>(hi - lo) / a.sample_rate;
    return (crossings / 2.0) / dur;
}

// The whole-stem mix a StreamMix read start to end must equal: each stem is
// converted to `out_rate` and `out_channels` through stem_converter_config (the
// converter StreamMix uses), then added into the mix in order, the mix growing
// with silence where a stem is longer. Empty input gives an empty mix at the
// requested format. Throws std::runtime_error if a converter fails.
inline hydra::audio::DecodedAudio mix_stems(const std::vector<hydra::audio::DecodedAudio>& stems,
                                            int out_rate, int out_channels) {
    hydra::audio::DecodedAudio out;
    out.sample_rate = out_rate;
    out.channels = out_channels;
    for (const hydra::audio::DecodedAudio& s : stems) {
        if (s.channels <= 0 || s.samples.empty()) continue;
        const hydra::audio::StemConverter sc = hydra::audio::stem_converter_config(
            s.sample_rate, s.channels, out_rate, out_channels);
        std::vector<float> stem;
        if (sc.passthrough) {
            stem = s.samples;
        } else {
            ma_data_converter conv;
            if (ma_data_converter_init(&sc.config, nullptr, &conv) != MA_SUCCESS)
                throw std::runtime_error("mix_stems: data converter init failed");
            ma_uint64 in_frames = static_cast<ma_uint64>(s.frames());
            ma_uint64 out_frames = 0;
            ma_data_converter_get_expected_output_frame_count(&conv, in_frames, &out_frames);
            stem.resize(static_cast<std::size_t>(out_frames) * out_channels);
            ma_uint64 in_consumed = in_frames;
            ma_uint64 out_produced = out_frames;
            const ma_result r = ma_data_converter_process_pcm_frames(
                &conv, s.samples.data(), &in_consumed, stem.data(), &out_produced);
            ma_data_converter_uninit(&conv, nullptr);
            if (r != MA_SUCCESS) throw std::runtime_error("mix_stems: data converter process failed");
            stem.resize(static_cast<std::size_t>(out_produced) * out_channels);
        }
        if (stem.size() > out.samples.size()) out.samples.resize(stem.size(), 0.0f);
        for (std::size_t i = 0; i < stem.size(); ++i) out.samples[i] += stem[i];
    }
    return out;
}

}  // namespace testaudio

#endif  // HYDRA_TESTS_AUDIO_UTIL_H
