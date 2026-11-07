// Audio decode layer for the Preview engine.
//
// One entry point, decode_audio, turns a chart stem's compressed bytes into
// interleaved float PCM. It picks its decoder from the bytes, not from a file
// extension, because container stems (.sng/.srb) carry bytes with no name.
// sniff_format in core/audio_sniff.h owns that rule, so the Preview's container
// extraction asks the same one.
//
// This layer never opens an output device: it is pure and fully testable off a
// real audio device (see the player for the device side).
//
// The decoders themselves are seekable StemReaders (audio/stem_reader.h);
// decode_audio opens one and reads it start to end.

#ifndef HYDRA_AUDIO_DECODE_H
#define HYDRA_AUDIO_DECODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/audio_sniff.h"  // AudioFormat, sniff_format, kOpusHeadTag

namespace hydra::app {
struct PreviewAudioStem;
}

namespace hydra::audio {

// PCM decoded from one stem: interleaved float samples in [-1, 1], with the
// channel count and sample rate they were decoded at. frames() is the count of
// per-channel sample frames.
struct DecodedAudio {
    std::vector<float> samples;  // interleaved by channel
    int channels = 0;
    int sample_rate = 0;

    int64_t frames() const {
        return channels > 0 ? static_cast<int64_t>(samples.size()) / channels : 0;
    }
    bool empty() const { return samples.empty(); }
};

// Decode compressed audio bytes to float PCM. Throws a KindedError of kind
// AudioDecode on an unrecognized container or a decode failure.
DecodedAudio decode_audio(const uint8_t* data, std::size_t size);
inline DecodedAudio decode_audio(const std::vector<uint8_t>& bytes) {
    return decode_audio(bytes.data(), bytes.size());
}

// Decode a Preview stem from whichever source it holds: its file on disk
// (from_file()) or its already-extracted container bytes. Throws as
// open_stem_reader does, and a KindedError of kind AudioDecode on a decode
// failure.
DecodedAudio decode_stem(const app::PreviewAudioStem& stem);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_DECODE_H
