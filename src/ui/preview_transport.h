// PreviewTransport — the Preview's Transport (CONTEXT.md): play, pause and
// seek together with the master clock, and the audio playhead that follows it
// (docs/adr/0008).
//
// The clock is the master. It is the song time the highway is drawn at, and
// the audio is seeked to it on play and then just follows. A seek re-syncs
// the audio to the clock: while the clock plays, the playhead is set playing
// again, even if it had paused itself at the end of its audio. A chart with
// no audio still plays and scrubs — the clock runs on its own.
//
// The playhead is shared with the audio thread, so every access to it goes
// through this object's lock, including the device's pull via read_frames().
// The clock is GUI-thread state and is not locked. The transport also holds
// the stop-at-end rule: tick() pauses and pins the time at the end of the
// song, so call it once per frame where the highway is drawn.

#ifndef HYDRA_UI_PREVIEW_TRANSPORT_H
#define HYDRA_UI_PREVIEW_TRANSPORT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>

#include "app/preview_clock.h"
#include "audio/frames.h"
#include "audio/player.h"

namespace hydra::ui {

// The chart sync rule and the audio's end live in audio/frames.h, so the
// song's length can ask them without reaching into the UI.
using audio::audio_end_chart_ms;
using audio::audio_ms_of_chart_ms;
using audio::chart_ms_of_audio_ms;

class PreviewTransport {
public:
    explicit PreviewTransport(app::PreviewClock::Now now = &app::PreviewClock::steady_now);

    PreviewTransport(const PreviewTransport&) = delete;
    PreviewTransport& operator=(const PreviewTransport&) = delete;

    // Load a chart's audio (may be null/empty for a chart with no audio), the
    // chart's last drawn note time (app::last_note_ms), and where chart time
    // 0 sits in the audio (audio_ms_of_chart_ms; never negative, see
    // audio::mix_song_stems). length_ms() becomes the playback range: the
    // later of `last_note_ms` and the audio's end in chart time, so the audio
    // plays out and a chart with no audio still plays to its last note (D48,
    // Q25). It is where playback stops, not a song length: the scrubber ends
    // at the song's length, the audio's end (audio::song_length_ms), through
    // app::scrub_end_ms. Resets the playhead to the offset, paused.
    void load(std::unique_ptr<audio::Playhead> playhead, double last_note_ms,
              double audio_offset_ms = 0.0);
    void unload();  // drop the playhead, back to an empty paused transport

    void play();    // seeks the playhead to the clock and starts both
    void pause();
    void toggle();
    // Clamped to [0, length_ms()]. Moves the playhead with the clock and,
    // while the clock plays, sets the playhead playing again.
    void seek_ms(double ms);
    bool playing() const;
    double length_ms() const;  // the playback range (see load), not the song's length
    bool has_audio() const;  // a loaded playhead with > 0 frames

    // The song time now. If playing and at/after the end, pauses and pins the
    // time to length_ms() (the stop-at-end rule) — so call this once per frame
    // where the highway is drawn.
    double tick();
    double now_ms() const;  // read-only, no end handling

    // Applied to the playhead if any (audio::Playhead::set_gain owns the
    // floor at 0), and remembered as given for the next load.
    void set_gain(float gain);
    float gain() const { return gain_; }

    // Device-facing: the pull source. Locks internally. Returns real frames
    // written.
    int64_t read_frames(float* out, int64_t frame_count);
    int channels() const;  // 0 when no audio
    int sample_rate() const;

private:
    app::PreviewClock clock_;  // GUI thread only
    double length_ms_ = 0.0;
    double audio_offset_ms_ = 0.0;  // as audio_ms_of_chart_ms takes it
    float gain_ = 1.0f;
    mutable std::mutex mu_;  // guards playhead_ (device thread pulls, GUI controls)
    std::unique_ptr<audio::Playhead> playhead_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_PREVIEW_TRANSPORT_H
