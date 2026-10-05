// How many ms a count of audio frames lasts, and how many frames a span of ms
// holds, at a sample rate. The one conversion every audio caller uses (audit
// finding 182): the playhead's seek, position and length, the load job's
// front pad, and the Preview transport's audio end. Header-only, so the UI
// can call it as well as the audio code.

#ifndef HYDRA_AUDIO_FRAMES_H
#define HYDRA_AUDIO_FRAMES_H

#include <cmath>
#include <cstdint>

namespace hydra::audio {

// How long `frames` frames last at `sample_rate`, in ms. 0 when the rate is
// not above 0 (no audio).
inline double ms_of_frames(int64_t frames, int sample_rate) {
    return sample_rate > 0 ? static_cast<double>(frames) * 1000.0 / sample_rate : 0.0;
}

// How many frames `ms` holds at `sample_rate`, rounded to the nearest frame
// (a half rounds away from zero).
inline int64_t frames_of_ms(double ms, int sample_rate) {
    return static_cast<int64_t>(std::llround(ms * sample_rate / 1000.0));
}

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_FRAMES_H
