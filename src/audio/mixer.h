// Stem mixer reference for the Preview engine.
//
// A chart often ships several stems (song, drums, guitar, ...) and no combined
// file, so the Preview must sum them into one signal to play. Stems decode at
// their own sample rates and channel counts (see audio/decode.h), so the mixer
// converts each to a common output format and adds it into the mix, one stem
// at a time.
// The mixed length is the longest stem's; shorter stems contribute silence past
// their end. Summing can push peaks past [-1, 1]; clamping is the player's job,
// not the mixer's, so the raw sum is preserved here for testability.
//
// The Preview itself plays a StreamMix (audio/stream_mix.h), which does the
// same mix while playing, straight from the compressed stems, with no decoded
// buffer. mix_stems stays as its test reference: a StreamMix read start to end
// must equal it.
//
// This header includes miniaudio for the converter config. Only src/audio
// .cpp files and tests include it; the UI reaches audio through player.h and
// stream_mix.h, which never see miniaudio.

#ifndef HYDRA_AUDIO_MIXER_H
#define HYDRA_AUDIO_MIXER_H

#include <vector>

#include "audio/decode.h"

// miniaudio's configuration macros come from the miniaudio target.
#include "miniaudio.h"

namespace hydra::audio {

// How one stem turns into the output format. Both fields come from
// stem_converter_config.
struct StemConverter {
    ma_data_converter_config config;  // the converter settings
    bool passthrough = false;         // the stem needs no converter
};

// The one owner of two questions: which converter settings turn a stem into
// the output format, and whether a stem needs a converter at all. Both mixers
// call it (mix_stems here, StreamMix in stream_mix.cpp), so a stream and a
// whole-stem mix convert alike. The definition in mixer.cpp is the only place
// the settings and the passthrough rule are written. The settings are
// miniaudio's defaults, kept as they are by decision D54 (R7.28).
StemConverter stem_converter_config(int in_rate, int in_channels, int out_rate,
                                    int out_channels);

// Mix `stems` into one interleaved buffer at `out_rate` Hz and `out_channels`
// channels. Empty input yields an empty result at the requested format.
DecodedAudio mix_stems(const std::vector<DecodedAudio>& stems, int out_rate,
                       int out_channels);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_MIXER_H
