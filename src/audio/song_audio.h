// A song's audio as one piece: its stems opened, mixed into the one stream the
// Preview plays. Nothing here says where the song ends: app::song_length_ms
// answers that from the chart's metadata, with no audio opened (D75, D255),
// and the Preview's playback and beat lines stop there.
//
// Nothing here decodes audio in the usual case. Opening a stem reads its
// layout, not its sound (open_stem_reader).

#ifndef HYDRA_AUDIO_SONG_AUDIO_H
#define HYDRA_AUDIO_SONG_AUDIO_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/preview_source.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "parse/song.h"

namespace hydra::audio {

// The format the song's stems are mixed to: what the audio device plays.
inline constexpr int kOutRate = 48000;
inline constexpr int kOutChannels = 2;

// The bytes each stem's reader will walk, in the stems' order: a loose stem's
// file mapped read-only, a container stem's extracted bytes moved in. A loose
// file that will not map is empty, and open_song_stems skips it, as it skips
// a stem that will not open.
std::vector<std::optional<StemBytes>> map_song_stems(std::vector<app::PreviewAudioStem> stems);

// The total compressed bytes across all mapped stems: the denominator for the
// loading bar's progress and for open_song_stems' progress callback.
uint64_t stems_total_bytes(const std::vector<std::optional<StemBytes>>& stems);

// Opens each stem's bytes with open_stem_reader, in order, and returns every
// reader that opened. A stem that will not open is skipped, so one unreadable
// or corrupt stem never silences the rest of the song.
//
// `progress`, when set, hears the bytes done across all stems (the stems
// before the current one counted whole) out of their total, during each open
// that reports and once after each stem. Returning false cancels: the open
// throws OpenCancelled, which this lets through.
std::vector<std::unique_ptr<StemReader>> open_song_stems(
    std::vector<std::optional<StemBytes>> stems, const OpenProgress& progress = nullptr);

// The song's stems mixed into one stream.
struct SongMix {
    // Every reader mixed while it plays, at kOutRate and kOutChannels, and
    // played at the song speed (audio::play_at_speed). A negative chart
    // offset is silence in front of the stems (the mix's front pad).
    std::unique_ptr<MixSource> mix;
    // Where chart time 0 sits in `mix`, in the file's own time, as
    // app::audio_ms_of_chart_ms takes it. Never negative: a negative offset
    // is padded into the front of `mix` instead, because the playhead cannot
    // seek below 0.
    double audio_offset_ms = 0.0;
};

// Mixes `readers` for a chart whose time 0 sits `offset_ms` into the audio
// (app::chart_audio_offset_ms), played at `speed_ratio` (the song's
// SongTiming::speed_ratio) through audio::play_at_speed. The Preview's load
// mixes here. Throws a KindedError of kind AudioDecode when the mix cannot
// be set up (StreamMix, StretchSource).
SongMix mix_song_stems(std::vector<std::unique_ptr<StemReader>> readers, double offset_ms,
                       double speed_ratio);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_SONG_AUDIO_H
