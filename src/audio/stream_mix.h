// Where the Preview's Playhead gets its audio from.
//
// A MixSource is a cursor over one playable signal: read the next N frames,
// or jump to any frame. Two kinds exist:
//   * BufferSource plays an already-decoded buffer. Tests and any caller that
//     still holds a whole DecodedAudio use it.
//   * StreamMix plays several stems at once, straight from their compressed
//     bytes. Every stem is read at one shared position, converted to the
//     output rate and channel count as it is read, and added into the output.
//     Because one cursor drives all stems, they can never drift apart, and no
//     decoded copy of the whole song ever exists.
//
// Neither type is thread-safe. The audio device thread drives a source through
// the Playhead, under the Preview Transport's lock (ui/preview_transport.h).

#ifndef HYDRA_AUDIO_STREAM_MIX_H
#define HYDRA_AUDIO_STREAM_MIX_H

#include <cstdint>
#include <memory>
#include <vector>

#include "audio/decode.h"
#include "audio/stem_reader.h"

namespace hydra::audio {

// A seekable source of interleaved float frames at one rate and channel count.
class MixSource {
public:
    virtual ~MixSource() = default;
    virtual int channels() const = 0;
    virtual int sample_rate() const = 0;
    virtual int64_t length_frames() const = 0;
    // Reads up to `frames` frames into `out`; returns frames read (0 at the
    // end). Never throws.
    virtual int64_t read(float* out, int64_t frames) = 0;
    // Moves to `frame`, clamped to [0, length_frames()]. Never throws.
    virtual void seek(int64_t frame) = 0;
};

// Plays a buffer that is already decoded and mixed.
class BufferSource : public MixSource {
public:
    explicit BufferSource(DecodedAudio audio);
    int channels() const override { return audio_.channels; }
    int sample_rate() const override { return audio_.sample_rate; }
    int64_t length_frames() const override { return length_; }
    int64_t read(float* out, int64_t frames) override;
    void seek(int64_t frame) override;

private:
    DecodedAudio audio_;
    int64_t length_ = 0;
    int64_t pos_ = 0;
};

// Mixes N stems while reading them.
//
// The result matches mix_stems() of every stem fully decoded, with
// `front_pad_frames` of silence in front: same length, same samples (within
// float rounding of resampling in blocks instead of in one call).
//
// Length = front pad + the longest stem after conversion. A shorter stem reads
// as silence past its end. A stem whose reader stops early (a decode error)
// goes silent; the others keep playing.
//
// read() never touches the heap: every scratch buffer is sized here, for a
// 4096-frame block, and a bigger request is served in 4096-frame pieces.
// seek() doesn't allocate either: it resets each stem's converter in place.
class StreamMix : public MixSource {
public:
    // `front_pad_frames` of silence come before every stem (a negative chart
    // offset). Each stem converts to out_rate / out_channels as it is read.
    // A null reader, or one with no channels or no rate, is dropped.
    // Throws std::runtime_error if a converter cannot be set up.
    StreamMix(std::vector<std::unique_ptr<StemReader>> stems, int out_rate,
              int out_channels, int64_t front_pad_frames);
    ~StreamMix() override;
    StreamMix(const StreamMix&) = delete;
    StreamMix& operator=(const StreamMix&) = delete;

    int channels() const override { return out_channels_; }
    int sample_rate() const override { return out_rate_; }
    int64_t length_frames() const override { return length_; }
    int64_t read(float* out, int64_t frames) override;
    void seek(int64_t frame) override;

    static constexpr int64_t kBlockFrames = 4096;

private:
    struct Stem;  // reader + its converter; defined in stream_mix.cpp

    int out_rate_;
    int out_channels_;
    int64_t pad_;
    int64_t length_ = 0;
    int64_t pos_ = 0;
    std::vector<std::unique_ptr<Stem>> stems_;
    std::vector<float> block_;  // one converted block of one stem
};

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_STREAM_MIX_H
