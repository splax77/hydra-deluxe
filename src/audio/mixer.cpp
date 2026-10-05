#include "audio/mixer.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace hydra::audio {

StemConverter stem_converter_config(int in_rate, int in_channels, int out_rate,
                                    int out_channels) {
    StemConverter c;
    c.config = ma_data_converter_config_init(
        ma_format_f32, ma_format_f32, static_cast<ma_uint32>(in_channels),
        static_cast<ma_uint32>(out_channels), static_cast<ma_uint32>(in_rate),
        static_cast<ma_uint32>(out_rate));
    c.passthrough = in_rate == out_rate && in_channels == out_channels;
    return c;
}

namespace {

// Convert one decoded stem to `out_channels` at `out_rate` with miniaudio's
// resampler and channel mapper, returning interleaved float. A stem already in
// the output format passes through unchanged.
// Takes the stem by value so a caller that owns it can move it in; a stem
// already in the output format then hands over its samples without a copy.
std::vector<float> convert_stem(DecodedAudio s, int out_rate,
                                int out_channels) {
    if (s.channels <= 0 || s.samples.empty()) return {};
    const StemConverter sc =
        stem_converter_config(s.sample_rate, s.channels, out_rate, out_channels);
    if (sc.passthrough) return std::move(s.samples);

    ma_data_converter conv;
    if (ma_data_converter_init(&sc.config, nullptr, &conv) != MA_SUCCESS)
        throw std::runtime_error("mix_stems: data converter init failed");

    ma_uint64 in_frames = static_cast<ma_uint64>(s.frames());
    ma_uint64 out_frames = 0;
    ma_data_converter_get_expected_output_frame_count(&conv, in_frames,
                                                      &out_frames);

    std::vector<float> converted(static_cast<std::size_t>(out_frames) *
                                 out_channels);
    ma_uint64 in_consumed = in_frames;
    ma_uint64 out_produced = out_frames;
    ma_result r = ma_data_converter_process_pcm_frames(
        &conv, s.samples.data(), &in_consumed, converted.data(), &out_produced);
    ma_data_converter_uninit(&conv, nullptr);
    if (r != MA_SUCCESS)
        throw std::runtime_error("mix_stems: data converter process failed");

    converted.resize(static_cast<std::size_t>(out_produced) * out_channels);
    return converted;
}

// Add one converted stem into the running mix, growing the mix with silence
// when this stem is longer. Every sample still starts at 0.0f and adds the
// stems in order, the same order as converting them all first, so the sum is
// bit-identical to the old mixer.
void add_into(std::vector<float>& mix, const std::vector<float>& stem) {
    if (stem.size() > mix.size()) mix.resize(stem.size(), 0.0f);
    for (std::size_t i = 0; i < stem.size(); ++i) mix[i] += stem[i];
}

}  // namespace

DecodedAudio mix_stems(const std::vector<DecodedAudio>& stems, int out_rate,
                       int out_channels) {
    DecodedAudio out;
    out.sample_rate = out_rate;
    out.channels = out_channels;
    for (const DecodedAudio& s : stems)
        add_into(out.samples, convert_stem(s, out_rate, out_channels));
    return out;
}

}  // namespace hydra::audio
