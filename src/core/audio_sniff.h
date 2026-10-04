// Which audio container a run of bytes holds, read from the bytes themselves.
//
// This is the one "are these bytes playable audio" rule. The decoder
// (audio/decode.h, audio/stem_reader.cpp) dispatches on it, and the Preview's
// container extraction (app/preview_source.cpp) keeps only the streams it
// accepts, so what is extracted is exactly what the decoder can open. It is a
// pure byte check, so it lives in hydra_core where both can reach it.
//
// Container stems (.sng/.srb) carry bytes with no name, so the check reads the
// magic, not a file extension:
//   * OggS + a Vorbis identification header -> OggVorbis
//   * OggS + an OpusHead header             -> OggOpus
//   * RIFF/WAVE, fLaC, ID3 or an MP3 sync   -> Wav, Flac, Mp3
// Two OggS codecs share the container magic, so sniff_format looks past "OggS"
// for the codec tag in the first page.

#ifndef HYDRA_CORE_AUDIO_SNIFF_H
#define HYDRA_CORE_AUDIO_SNIFF_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace hydra::audio {

// The tag that opens an Ogg Opus stream's identification header (RFC 7845).
// sniff_format looks for it to tell Opus from Vorbis, and the Opus reader
// checks that each link's first packet starts with it.
inline constexpr char kOpusHeadTag[] = "OpusHead";

// The container Hydra recognizes for a stem, decided from its leading bytes.
enum class AudioFormat { Unknown, Wav, Mp3, Flac, OggVorbis, OggOpus };

// Classify audio bytes by content. Distinguishes the two OggS codecs by the
// codec tag in the first page. Returns Unknown for anything unrecognized or too
// short to tell.
AudioFormat sniff_format(const uint8_t* data, std::size_t size);
inline AudioFormat sniff_format(const std::vector<uint8_t>& bytes) {
    return sniff_format(bytes.data(), bytes.size());
}

}  // namespace hydra::audio

#endif  // HYDRA_CORE_AUDIO_SNIFF_H
