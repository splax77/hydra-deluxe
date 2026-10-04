// Tests for audio/mixer: converting stems to a common format and summing them.
// Summation is checked exactly with hand-built PCM; the resample and channel
// conversion is checked on a synthesized tone whose frequency must survive.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "app/preview_source.h"
#include "audio/decode.h"
#include "audio/mixer.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "audio_util.h"

using namespace hydra::audio;
using testaudio::estimate_freq_hz;
using testaudio::fixture_path;
using testaudio::read_fixture;

namespace {

DecodedAudio make_pcm(std::vector<float> samples, int channels, int rate) {
    DecodedAudio a;
    a.samples = std::move(samples);
    a.channels = channels;
    a.sample_rate = rate;
    return a;
}

// A mono sine of `freq` Hz, `seconds` long, sampled at `rate`.
DecodedAudio synth_tone(double freq, int rate, double seconds) {
    const double pi = 3.14159265358979323846;
    int n = static_cast<int>(rate * seconds);
    DecodedAudio a;
    a.channels = 1;
    a.sample_rate = rate;
    a.samples.resize(n);
    for (int i = 0; i < n; ++i)
        a.samples[i] = static_cast<float>(0.5 * std::sin(2 * pi * freq * i / rate));
    return a;
}

// A whole MixSource read start to end (through the shared block reader), as
// audio at the source's format.
DecodedAudio read_all(MixSource& src) {
    DecodedAudio out;
    out.channels = src.channels();
    out.sample_rate = src.sample_rate();
    out.samples = testaudio::read_frames(src, src.length_frames());
    return out;
}

// Same format and the same float bits, sample for sample.
bool same_bits(const DecodedAudio& a, const DecodedAudio& b) {
    if (a.sample_rate != b.sample_rate || a.channels != b.channels) return false;
    if (a.samples.size() != b.samples.size()) return false;
    return a.samples.empty() ||
           std::memcmp(a.samples.data(), b.samples.data(),
                       a.samples.size() * sizeof(float)) == 0;
}

}  // namespace

TEST_CASE("mix_stems sums same-format stems and zero-extends the shorter") {
    DecodedAudio s1 = make_pcm({0.10f, 0.20f, 0.30f}, 1, 48000);
    DecodedAudio s2 = make_pcm({0.01f, 0.02f}, 1, 48000);

    DecodedAudio out = mix_stems({s1, s2}, 48000, 1);

    CHECK(out.channels == 1);
    CHECK(out.sample_rate == 48000);
    REQUIRE(out.frames() == 3);  // longest stem
    CHECK(out.samples[0] == doctest::Approx(0.11f));
    CHECK(out.samples[1] == doctest::Approx(0.22f));
    CHECK(out.samples[2] == doctest::Approx(0.30f));  // s2 silent past its end
}

TEST_CASE("stem_converter_config: same rate and channels is a passthrough, anything else converts") {
    CHECK(stem_converter_config(48000, 2, 48000, 2).passthrough);
    CHECK_FALSE(stem_converter_config(44100, 2, 48000, 2).passthrough);
    CHECK_FALSE(stem_converter_config(48000, 1, 48000, 2).passthrough);
    // The config echoes the formats it was given.
    const StemConverter c = stem_converter_config(44100, 1, 48000, 2);
    CHECK(c.config.sampleRateIn == 44100);
    CHECK(c.config.channelsIn == 1);
    CHECK(c.config.sampleRateOut == 48000);
    CHECK(c.config.channelsOut == 2);
}

TEST_CASE("mix_stems of no stems is empty at the requested format") {
    DecodedAudio out = mix_stems({}, 44100, 2);
    CHECK(out.channels == 2);
    CHECK(out.sample_rate == 44100);
    CHECK(out.samples.empty());
}

TEST_CASE("mix_stems resamples to the output rate and unifies channels") {
    DecodedAudio mono24k = synth_tone(300.0, 24000, 1.0);  // 24000 frames, mono

    DecodedAudio out = mix_stems({mono24k}, 48000, 2);

    CHECK(out.channels == 2);
    CHECK(out.sample_rate == 48000);
    // Resampled 24k -> 48k over ~1 s lands near 48000 frames (resampler latency
    // trims a few).
    CHECK(static_cast<double>(out.frames()) == doctest::Approx(48000).epsilon(0.02));
    CHECK(estimate_freq_hz(out, 0) == doctest::Approx(300.0).epsilon(0.05));
    // A mono stem upmixed to stereo puts the same signal in both channels.
    for (int64_t i = 100; i < out.frames() - 100; i += 977) {
        CHECK(out.samples[static_cast<size_t>(i) * 2] ==
              doctest::Approx(out.samples[static_cast<size_t>(i) * 2 + 1]));
    }
}

// The Preview load opens each stem and skips one that won't open (the job's
// open loop); the rest mix in a StreamMix. This was decode_and_mix's test.
TEST_CASE("StreamMix of the stems that open: an undecodable stem is skipped") {
    hydra::app::PreviewAudioStem ogg;  // a file-path stem
    ogg.label = "song";
    ogg.path = fixture_path("sine220.ogg");

    hydra::app::PreviewAudioStem mp3;  // a container-bytes stem
    mp3.label = "drums";
    mp3.bytes = read_fixture("sine220.mp3");

    hydra::app::PreviewAudioStem junk;  // undecodable — must be skipped
    junk.label = "broken";
    junk.bytes = {'n', 'o', 't', ' ', 'a', 'u', 'd', 'i', 'o'};

    std::vector<std::unique_ptr<StemReader>> readers;
    int skipped = 0;
    for (const hydra::app::PreviewAudioStem& s : {ogg, mp3, junk}) {
        try {
            readers.push_back(open_stem_reader(s));
        } catch (const std::exception&) {
            ++skipped;
        }
    }
    CHECK(skipped == 1);
    StreamMix mix(std::move(readers), 48000, 2, 0);
    DecodedAudio out = read_all(mix);
    CHECK(out.channels == 2);
    CHECK(out.sample_rate == 48000);
    REQUIRE(out.frames() > 4800);  // both real 220 Hz stems mixed in
    CHECK(estimate_freq_hz(out, 0) == doctest::Approx(220.0).epsilon(0.07));

    // No stem opened -> an empty mix, never a throw.
    StreamMix none({}, 48000, 2, 0);
    CHECK(none.length_frames() == 0);
    CHECK(none.channels() == 2);
}

TEST_CASE("mix_stems adds hand-built stems bit for bit; a zero-channel stem adds nothing") {
    // Mono stems already at the output rate, so no resampler runs. Quarters
    // and eighths add exactly in a float, so the hand sum below is exact.
    const DecodedAudio a = make_pcm({0.25f, 0.5f, 1.0f}, 1, 48000);
    const DecodedAudio b = make_pcm({0.125f, 0.25f}, 1, 48000);
    const DecodedAudio c = make_pcm({0.5f}, 0, 48000);  // no channels: adds nothing
    // A plus B for two frames, then A alone past B's end.
    const DecodedAudio want = make_pcm({0.375f, 0.75f, 1.0f}, 1, 48000);

    CHECK(same_bits(mix_stems({a, b, c}, 48000, 1), want));
    CHECK(same_bits(mix_stems({c, a, b}, 48000, 1), want));
}

TEST_CASE("StreamMix of real stems matches decoding every stem then mixing") {
    hydra::app::PreviewAudioStem ogg;
    ogg.label = "song";
    ogg.path = fixture_path("sine220.ogg");
    hydra::app::PreviewAudioStem mp3;
    mp3.label = "drums";
    mp3.bytes = read_fixture("sine220.mp3");
    hydra::app::PreviewAudioStem opus;
    opus.label = "guitar";
    opus.bytes = read_fixture("sine220.opus");
    hydra::app::PreviewAudioStem junk;
    junk.label = "broken";
    junk.bytes = {'n', 'o', 't', ' ', 'a', 'u', 'd', 'i', 'o'};
    const std::vector<hydra::app::PreviewAudioStem> stems = {ogg, junk, mp3, opus};

    std::vector<DecodedAudio> decoded;
    std::vector<std::unique_ptr<StemReader>> readers;
    for (const hydra::app::PreviewAudioStem& s : stems) {
        try {
            decoded.push_back(decode_stem(s));
            readers.push_back(open_stem_reader(s));
        } catch (const std::exception&) {
        }
    }
    REQUIRE(decoded.size() == 3);
    REQUIRE(readers.size() == 3);

    // Resampling in blocks instead of in one call: equal within float rounding.
    StreamMix mix(std::move(readers), 48000, 2, 0);
    const DecodedAudio want = mix_stems(decoded, 48000, 2);
    const DecodedAudio got = read_all(mix);
    REQUIRE(got.samples.size() == want.samples.size());
    CHECK(testaudio::max_diff(got.samples, want.samples.data(), 0, got.samples.size()) <= 1e-6);
}