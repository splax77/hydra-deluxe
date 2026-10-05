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
#include "core/audio_sniff.h"  // AudioFormat, for open_ma_reader
#include "core/error_kind.h"

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
    // The number of frames the stem decodes to. A header that says 0 means
    // "unknown" (RFC 9639 for FLAC), not empty, so the reader replaces it on
    // open with a count from decoding the stem once; every other header keeps
    // today's fast open.
    virtual int64_t length_frames() const = 0;
    // Reads up to `frames` frames into `out`; returns frames read (0 at the
    // end). Never throws: a decode error ends the stem early.
    virtual int64_t read(float* out, int64_t frames) = 0;
    // Moves to `frame`, clamped to [0, length_frames()]. Never throws. A seek
    // to a frame before a decode error plays again from there, even after the
    // error has ended the stem once.
    virtual void seek(int64_t frame) = 0;
    // True once any decode error has ended the stem early, and stays true for
    // the reader's life, even after a seek plays the stem again. decode_audio
    // uses it to keep failing loudly on a damaged stream, as the old decoders
    // did.
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

struct OpenCancelled : KindedError {
    OpenCancelled() : KindedError(ErrorKind::Cancelled, "cancelled") {}
};
// bytes_done / bytes_total; return false to cancel (open throws OpenCancelled).
using OpenProgress = std::function<bool(uint64_t bytes_done, uint64_t bytes_total)>;

// Throws a KindedError of kind AudioDecode ("decode_audio: ...") for an
// unrecognized or unreadable stream. A loose file that can't be opened throws
// kind SongFileMissing ("cannot open file: ...").
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
// `format` is sniff_format's answer for these bytes (Wav, Mp3 or Flac), so
// the reader does not sniff again: Mp3 goes to dr_mp3, the rest to
// ma_decoder.
std::unique_ptr<StemReader> open_ma_reader(StemBytes bytes, AudioFormat format);

// What one decode call did, as CountedLength asks it: the frames it decoded,
// whether the stem goes on after them, and whether a decode error stopped it.
struct DecodeStep {
    int64_t frames = 0;
    bool more = false;
    bool error = false;
};

// A stem whose header says it has 0 frames, shared by MaReader and
// VorbisReader. The 0 means "unknown" (RFC 9639 for FLAC; stb_vorbis says it
// when it finds no end page), not empty. So the reader counts the stem by
// decoding it once on open, then goes back to frame 0. Its decoder still
// believes the header and can't seek, so a counted stem seeks by decoding:
// forward from where it is when the target is ahead, otherwise from frame 0.
// Exact, and slower than a real seek, in this rare case only.
//
// The reader passes its decoder in as two calls. restart() goes back to
// frame 0 and returns false if it could not. decode(n) decodes up to n frames
// into the reader's scratch buffer and returns a DecodeStep.
class CountedLength {
public:
    // The stem's length: the header's, or, when the header says 0, a count
    // made by decoding the stem in chunks of `chunk` frames. After a count
    // the decoder is back at frame 0, and at_end says whether that failed.
    // A decode error during the count only ends the count.
    template <class Restart, class Decode>
    int64_t length(int64_t header_frames, int64_t chunk, bool& at_end, Restart restart,
                   Decode decode) {
        if (header_frames != 0) return header_frames;
        int64_t total = 0;
        for (;;) {
            const DecodeStep step = decode(chunk);
            total += step.frames;
            if (!step.more) break;
        }
        counted_ = true;
        at_end = !restart();
        return total;
    }

    // True when length() counted the stem, so seeks go through seek() below.
    bool counted() const { return counted_; }

    // Moves a counted stem to `frame` (already clamped below its length) by
    // decoding. Returns true when a decode error ended the skip; the reader
    // decides what that means for it.
    template <class Restart, class Decode>
    bool seek(int64_t frame, int64_t chunk, int64_t& pos, bool& at_end, Restart restart,
              Decode decode) const {
        if (frame < pos || at_end) {
            if (!restart()) {
                at_end = true;
                return false;
            }
            pos = 0;
            at_end = false;
        }
        while (pos < frame) {
            const DecodeStep step = decode(frame - pos < chunk ? frame - pos : chunk);
            pos += step.frames;
            if (!step.more) {
                at_end = true;
                return step.error;
            }
        }
        return false;
    }

private:
    bool counted_ = false;
};

}  // namespace detail

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_STEM_READER_H
