#include "audio/mixer.h"

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

}  // namespace hydra::audio
