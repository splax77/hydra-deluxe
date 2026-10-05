// A song's audio as one piece: its stems opened, mixed into the one stream the
// Preview plays, and how long that stream runs in chart time. That last answer
// is the song's length (D69). D69 names which readers use it.
//
// Nothing here decodes audio in the usual case. Opening a stem reads its
// layout, not its sound (open_stem_reader), so the length costs what the
// Preview's own open costs.

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

// The format the song's stems are mixed to: what the audio device plays, and
// what the song's length is counted in.
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

// The song's stems mixed into one stream, and where its audio ends.
struct SongMix {
    // Every reader mixed while it plays, at kOutRate and kOutChannels. A
    // negative chart offset is silence in front of the stems (the mix's
    // front pad).
    std::unique_ptr<StreamMix> mix;
    // Where chart time 0 sits in `mix`. Never negative: a negative offset is
    // padded into the front of `mix` instead, because the playhead cannot
    // seek below 0.
    double audio_offset_ms = 0.0;
    // Where the audio stops in chart time (audio_end_chart_ms of `mix`).
    // Empty when no stem has any audio: the front pad alone is not audio.
    std::optional<double> end_chart_ms;
};

// Mixes `readers` for a chart whose time 0 sits `offset_ms` into the audio
// (app::chart_audio_offset_ms). The Preview's load and song_length_ms both mix
// here, so what plays and how long the song is cannot disagree. Throws
// std::runtime_error when the mix cannot be set up (StreamMix).
SongMix mix_song_stems(std::vector<std::unique_ptr<StemReader>> readers, double offset_ms);

// How long the song at `notespath` is: the end of its mixed audio in chart
// time, the longest of its stems once the chart's offset is applied. This is
// the one answer to "how long is this song" (D69); nothing works a length out
// from notes. `song` is the chart already parsed from `notespath`, read only
// for its Offset. A .sng or .srb is read from disk once. Empty when no stem
// opens: such a song has no length.
std::optional<double> song_length_ms(const std::string& notespath, const Song& song);

}  // namespace hydra::audio

#endif  // HYDRA_AUDIO_SONG_AUDIO_H
