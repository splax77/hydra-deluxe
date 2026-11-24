// The Preview's audio at another song speed: the same mix, faster or slower,
// at the same pitch.
//
// StretchSource wraps a MixSource (the stem mix, audio/stream_mix.h) and plays
// it through SoundTouch's tempo change, the effect Clone Hero's BASS_FX tempo
// stream is built on. It counts in played frames, the frames the Playhead
// serves (audio/player.h): length_frames() and seek() are in played frames,
// and played frame F comes from file frame F x speed ratio. So this is the one
// place a file position turns into played frames; the speed ratio itself comes
// from SongTiming::speed_ratio (core/song.h owns it).
//
// play_at_speed() is how a caller gets the Preview's source: at ratio 1 it
// hands the plain mix back untouched, so normal speed plays today's samples.
//
// Like StreamMix, it is not thread-safe, and read() and seek() never touch the
// heap: SoundTouch's buffers only grow, never shrink, so the constructor grows
// them once past anything a read can need (see the constructor in
// stretch_mix.cpp).

#ifndef HYDRA_AUDIO_STRETCH_MIX_H
#define HYDRA_AUDIO_STRETCH_MIX_H

#include <cstdint>
#include <memory>
#include <vector>

#include "audio/stream_mix.h"

namespace soundtouch {
class SoundTouch;
}

namespace hydra::audio {

class StretchSource : public MixSource {
public:
    // Plays `plain` at `speed_ratio` (2.0 is twice as fast). Throws a
    // KindedError of kind AudioDecode if `plain` is null or has no playable
    // format, if the ratio is not a positive finite number, or if SoundTouch
    // cannot be set up or fails the start-delay probe.
    StretchSource(std::unique_ptr<MixSource> plain, double speed_ratio);
    ~StretchSource() override;
    StretchSource(const StretchSource&) = delete;
    StretchSource& operator=(const StretchSource&) = delete;

    int channels() const override { return channels_; }
    int sample_rate() const override { return sample_rate_; }
    // The wrapped source's length in played frames: its file length divided
    // by the ratio, to the nearest frame.
    int64_t length_frames() const override { return length_; }
    // Reads up to `frames` played frames (0 at the end). Never throws.
    int64_t read(float* out, int64_t frames) override;
    // Moves to played frame `frame`, clamped to [0, length_frames()]. The next
    // frame read comes from file frame `frame` x ratio. Never throws.
    void seek(int64_t frame) override;

    double speed_ratio() const { return ratio_; }
    // The file frame played frame `frame` comes from: frame x ratio, to the
    // nearest frame, clamped to the wrapped source.
    int64_t file_frame_of(int64_t played_frame) const;

    // SoundTouch's start delay, measured in the constructor with an impulse:
    // how many played frames after a restart the first fed frame comes out
    // (negative if it comes out early). Every seek corrects for it, so it
    // never shows in what read() returns.
    int64_t start_delay_frames() const { return start_delay_; }

private:
    // Feeds the wrapped source (or silence past its end) into SoundTouch until
    // `need` played frames are ready after any pending drop. False when no
    // more can come.
    bool fill(int64_t need);
    // One block of input into SoundTouch. False once the tail is pushed out.
    bool feed();
    void measure_start_delay();
    void grow_buffers();

    std::unique_ptr<MixSource> plain_;
    std::unique_ptr<soundtouch::SoundTouch> st_;
    double ratio_;
    int channels_;
    int sample_rate_;
    int64_t file_length_;
    int64_t length_;
    int64_t pos_ = 0;
    int64_t start_delay_ = 0;
    int64_t pending_drop_ = 0;   // played frames to drop after a seek (the start delay)
    int64_t tail_feeds_ = 0;     // silent blocks fed since the wrapped source ended
    int64_t max_tail_feeds_ = 0; // enough silence to push SoundTouch's whole buffer out
    std::vector<float> in_;      // one block of input frames
};

// The source the Preview plays at `speed_ratio`: `plain` itself at exactly 1,
// otherwise `plain` inside a StretchSource. A null `plain` stays null.
std::unique_ptr<MixSource> play_at_speed(std::unique_ptr<MixSource> plain, double speed_ratio);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_STRETCH_MIX_H
