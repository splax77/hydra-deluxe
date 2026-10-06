// How many ms a count of audio frames lasts, and how many frames a span of ms
// holds, at a sample rate. The one conversion every audio caller uses (audit
// finding 182): the playhead's seek, position and length, the mix's front
// pad, and the audio's end. Header-only, so the UI can call it as well as the
// audio code.
//
// The chart sync rule lives here too: where a chart time sits in the audio,
// and where the audio ends in chart time. The Preview's mix (song_audio.h),
// its transport and the song's length (app::song_length_ms) all ask it.

#ifndef HYDRA_AUDIO_FRAMES_H
#define HYDRA_AUDIO_FRAMES_H

#include <cmath>
#include <cstdint>
#include <optional>

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

// The chart sync rule, chart time to audio time. `audio_offset_ms` is where
// chart time 0 sits in the audio (app::chart_audio_offset_ms works it out).
// chart_ms_of_audio_ms is the same rule run backwards.
inline double audio_ms_of_chart_ms(double chart_ms, double audio_offset_ms) {
    return chart_ms + audio_offset_ms;
}

inline double chart_ms_of_audio_ms(double audio_ms, double audio_offset_ms) {
    return audio_ms - audio_offset_ms;
}

// Where `audio` stops in chart time: its length in ms, through
// chart_ms_of_audio_ms. Empty when there is no audio. The one rule for the
// audio's end: the song's mix step asks it of the mix it built
// (mix_song_stems) and the Preview transport asks it of the playhead it is
// handed (its playback range). `audio` is anything with
// channels(), sample_rate() and length_frames(): a MixSource or a Playhead.
template <class Audio>
std::optional<double> audio_end_chart_ms(const Audio& audio, double audio_offset_ms) {
    if (audio.channels() <= 0 || audio.sample_rate() <= 0 || audio.length_frames() <= 0)
        return std::nullopt;
    return chart_ms_of_audio_ms(ms_of_frames(audio.length_frames(), audio.sample_rate()),
                                audio_offset_ms);
}

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_FRAMES_H
