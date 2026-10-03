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

#ifndef HYDRA_AUDIO_MIXER_H
#define HYDRA_AUDIO_MIXER_H

#include <vector>

#include "audio/decode.h"

namespace hydra::audio {

// Mix `stems` into one interleaved buffer at `out_rate` Hz and `out_channels`
// channels. Empty input yields an empty result at the requested format.
DecodedAudio mix_stems(const std::vector<DecodedAudio>& stems, int out_rate,
                       int out_channels);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_MIXER_H
