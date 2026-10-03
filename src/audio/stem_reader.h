// Seekable, pull-style decoders for the Preview's audio stems.
//
// A StemReader is a bookmark in a compressed audio file plus a way to move the
// bookmark. It unpacks only the frames it is asked for, so an 8-hour stem
// costs no more to open than a short one. open_stem_reader picks the decoder
// from the bytes' own magic (sniff_format), exactly as decode_audio always has:
//   * Ogg Opus   -> a page index + libopus over libogg (opus_reader.cpp)
//   * Ogg Vorbis -> stb_vorbis's pull decoder (vorbis_reader.cpp)
//   * WAV/MP3/FLAC -> miniaudio's ma_decoder (ma_reader.cpp)
// Reading a stem start to end gives exactly the samples the old whole-file
// decoders gave; tests/test_stem_reader.cpp pins that against copies of them.

#ifndef HYDRA_AUDIO_STEM_READER_H
#define HYDRA_AUDIO_STEM_READER_H

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <vector>

#include "audio/mapped_file.h"

namespace hydra::app {
struct PreviewAudioStem;
}

namespace hydra::audio {

// A seekable, pull-style decoder for one stem: it unpacks only the frames it
// is asked for, so an 8-hour stem costs no more to open than a short one.
// Frames are interleaved float in [-1, 1] at the stem's own rate and channel
// count. Not thread-safe: one thread drives a reader (the audio device thread,
// through StreamMix and the Transport's lock).
class StemReader {
public:
    virtual ~StemReader() = default;
    virtual int channels() const = 0;
    virtual int sample_rate() const = 0;
    virtual int64_t length_frames() const = 0;
    // Reads up to `frames` frames into `out`; returns frames read (0 at the
    // end). Never throws: a decode error ends the stem early.
    virtual int64_t read(float* out, int64_t frames) = 0;
    // Moves to `frame`, clamped to [0, length_frames()]. Never throws.
    virtual void seek(int64_t frame) = 0;
    // True once a decode error has ended the stem early. decode_audio uses it
    // to keep failing loudly on a damaged stream, as the old decoders did.
    virtual bool failed() const { return false; }
};

// The compressed bytes a reader decodes from. Either owned (a container stem's
// extracted bytes) or a read-only map of a loose file; the reader keeps it alive.
struct StemBytes {
    std::vector<uint8_t> owned;
    std::shared_ptr<const MappedFile> mapped;
    const uint8_t* data() const { return mapped ? mapped->data() : owned.data(); }
    std::size_t size() const { return mapped ? mapped->size() : owned.size(); }
};

struct OpenCancelled : std::exception {
    const char* what() const noexcept override { return "cancelled"; }
};
// bytes_done / bytes_total; return false to cancel (open throws OpenCancelled).
using OpenProgress = std::function<bool(uint64_t bytes_done, uint64_t bytes_total)>;

// Throws std::runtime_error ("decode_audio: ...") for an unrecognized or
// unreadable stream, so user_messages.cpp keeps classifying it as an audio
// decode error. A loose file that can't be opened throws "cannot open file: ...".
std::unique_ptr<StemReader> open_stem_reader(StemBytes bytes,
                                             const OpenProgress& progress = nullptr);
std::unique_ptr<StemReader> open_stem_reader(const app::PreviewAudioStem& stem,
                                             const OpenProgress& progress = nullptr);

// The per-format openers behind open_stem_reader. They assume the format was
// already sniffed; tests call them directly to exercise one decoder.
namespace detail {

// Knobs for the Opus reader that the Preview never changes.
struct OpusReaderOptions {
    // Stop each link at its last page's granule position minus pre-skip (end
    // trimming, RFC 7845 section 4.4) instead of playing the encoder's padding
    // at the very end. Off by default: the Preview has always played the
    // padding, and the pinned decode test holds that output.
    bool trim_end = false;
};

std::unique_ptr<StemReader> open_opus_reader(StemBytes bytes, const OpenProgress& progress,
                                             const OpusReaderOptions& options = {});
std::unique_ptr<StemReader> open_vorbis_reader(StemBytes bytes);
std::unique_ptr<StemReader> open_ma_reader(StemBytes bytes);

}  // namespace detail

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_STEM_READER_H
