// The audio test helpers the audio tests share: where a fixture under
// testdata/audio lives, its bytes, a PCM16 WAV writer, a block reader and a
// sample-difference measure for a MixSource, and a rough frequency check for
// the 220 Hz sine fixtures. Audit finding 277: test_audio_decode.cpp,
// test_audio_mixer.cpp and test_stream_mix.cpp each kept their own copies
// before.

#ifndef HYDRA_TESTS_AUDIO_UTIL_H
#define HYDRA_TESTS_AUDIO_UTIL_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bytes_util.h"

#include "audio/decode.h"
#include "audio/stream_mix.h"
#include "core/winstr.h"

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

namespace testaudio {

// The path of testdata/audio/<name>, for a stem opened from a file.
inline std::string fixture_path(const std::string& name) {
    return std::string(HYDRA_TESTDATA_DIR) + "/audio/" + name;
}

// The bytes of testdata/audio/<name>, for a stem held in memory.
inline std::vector<uint8_t> read_fixture(const std::string& name) {
    return hydra::read_file_bytes(fixture_path(name));
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

}  // namespace testaudio

#endif  // HYDRA_TESTS_AUDIO_UTIL_H
