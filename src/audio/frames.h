// How many audio frames a span of milliseconds is, and back.
//
// A frame is one sample on every channel at one instant, so one second holds
// `sample_rate` frames. This header is the one place that turns milliseconds
// into frames and frames into milliseconds. It includes nothing from
// miniaudio and no player, so UI code (the Preview load job's front pad) can
// include it too.
//
// Neither function guards a rate of 0: a missing source is the caller's
// question (the Playhead answers it before calling).

#ifndef HYDRA_AUDIO_FRAMES_H
#define HYDRA_AUDIO_FRAMES_H

#include <cmath>
#include <cstdint>

namespace hydra::audio {

// `ms` milliseconds as frames at `sample_rate`, rounded to the nearest frame
// (a half rounds away from zero).
inline int64_t frames_of_ms(double ms, int sample_rate) {
    return static_cast<int64_t>(std::llround(ms * sample_rate / 1000.0));
}

// `frames` frames at `sample_rate` as milliseconds, not rounded.
inline double ms_of_frames(int64_t frames, int sample_rate) {
    return frames * 1000.0 / sample_rate;
}

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_FRAMES_H
