// The pure, device-free audio playhead of the Preview.
//
// It owns one audio source (a MixSource: a streaming mix of the chart's stems,
// see audio/stream_mix.h, or a plain decoded buffer) and the playback
// position, and serves frames to an output callback through read_frames(). It
// has no clock of its own: the Preview Transport (src/ui/preview_transport.h)
// drives it, seeking it to the master display clock on play
// (app/preview_clock.h, as in Onyx — see docs/adr/0008), after which every
// frame served advances the audio position from there. This type deliberately
// holds no output device — it is pure and fully unit-tested. The Transport
// wraps a miniaudio ma_device around a Playhead and calls read_frames() from
// the device callback; opening the device needs real hardware and stays out of
// the tests.
//
// A seek only records the new position. The source itself moves on the next
// read_frames() while playing, on the device thread, so a seek from the UI
// never decodes anything on the UI thread.

#ifndef HYDRA_AUDIO_PLAYER_H
#define HYDRA_AUDIO_PLAYER_H

#include <cstdint>
#include <memory>

#include "audio/decode.h"
#include "audio/stream_mix.h"

namespace hydra::audio {

class Playhead {
public:
    // Plays an already-decoded buffer (through a BufferSource).
    explicit Playhead(DecodedAudio mixed);
    // Plays any source; a null source plays as empty audio.
    explicit Playhead(std::unique_ptr<MixSource> source);

    // Playback state. A new playhead is paused at the start.
    void play();
    void pause();
    void toggle();
    bool playing() const { return playing_; }

    // Move the playhead. Positions are clamped to [0, length]. Only records
    // the position; the source follows on the next read_frames() while playing.
    void seek_frames(int64_t frame);
    void seek_ms(double ms);

    int64_t position_frames() const { return position_; }
    double position_ms() const;
    int64_t length_frames() const { return length_; }
    double length_ms() const;
    int channels() const { return channels_; }
    int sample_rate() const { return sample_rate_; }

    // Output gain applied to every frame served (1.0 = the mix as decoded).
    // A summed multi-stem mix is loud, so the GUI defaults well below 1.
    void set_gain(float gain) { gain_ = gain < 0.0f ? 0.0f : gain; }
    float gain() const { return gain_; }

    // Fill `out` with `frame_count` interleaved frames for the device. While
    // playing, first moves the source to the position if a seek changed it,
    // then reads from the current position and advances the clock, then
    // zero-fills any frames past the end and auto-pauses there. While paused,
    // writes silence and does not touch the source. Returns the count of real
    // audio frames written (never counting the silence tail).
    int64_t read_frames(float* out, int64_t frame_count);

private:
    std::unique_ptr<MixSource> source_;
    int channels_;
    int sample_rate_;
    int64_t length_;
    int64_t position_ = 0;
    int64_t source_position_ = 0;  // where the source's own cursor is
    bool playing_ = false;
    float gain_ = 1.0f;
};

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_PLAYER_H
