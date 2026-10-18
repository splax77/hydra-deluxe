// See song_length.h.

#include "app/song_length.h"

#include <algorithm>

#include "app/preview_source.h"  // preview_audio_offset_ms
#include "app/preview_view.h"    // has_song_length
#include "audio/frames.h"        // chart_ms_of_audio_ms (header-only)
#include "core/strutil.h"        // parse_finite_number

namespace hydra::app {

std::optional<double> stated_length_ms(std::optional<double> raw_ms) {
    if (raw_ms && has_song_length(*raw_ms)) return raw_ms;
    return std::nullopt;
}

std::optional<double> stated_length_ms_of_text(std::string_view text) {
    return stated_length_ms(parse_finite_number(text));
}

std::optional<double> last_note_start_ms(const Song& song) {
    // Only a timestamp that holds a note counts, as in the Preview's scene
    // (last_drawn_note), which draws no note-less timestamp.
    for (auto it = song.sequence.rbegin(); it != song.sequence.rend(); ++it)
        if (it->chord.count() > 0) return it->timecode.ms();
    return std::nullopt;
}

bool end_event_counts(const Song& song) {
    if (!song.end_event_tick) return false;
    const int64_t last_note_end = song.last_note_end_tick.value_or(0);
    return *song.end_event_tick >= last_note_end + song.tick_resolution();
}

std::optional<double> song_length_ms(const store::ChartTimingMeta& meta, const Song& song) {
    const auto chart_ms = [&](int64_t tick) { return song.timecode(tick).ms(); };
    const std::optional<double> last_note =
        song.last_note_end_tick ? std::optional<double>(chart_ms(*song.last_note_end_tick))
                                : std::nullopt;
    const std::optional<double> end_event =
        end_event_counts(song) ? std::optional<double>(chart_ms(*song.end_event_tick))
                               : std::nullopt;

    std::optional<double> stated;
    if (const std::optional<double> raw = stated_length_ms(meta.length_ms)) {
        const double stated_chart_ms = audio::chart_ms_of_audio_ms(
            *raw, preview_audio_offset_ms(meta.delay_ms, song.chart_offset_s));
        // A stated length that ends before the last note is wrong (a song.ini
        // that says 6:17 for a 13:41 song), so it is dropped.
        if (!last_note || stated_chart_ms >= *last_note) stated = stated_chart_ms;
    }

    // D255, the user's answer 1: the end event caps a stated length; without
    // one, the end event stands in, then the last note.
    std::optional<double> length;
    if (stated)
        length = end_event ? std::min(*end_event, *stated) : *stated;
    else
        length = end_event ? end_event : last_note;

    if (!length || !has_song_length(*length)) return std::nullopt;
    return *length + kSongEndPadMs;
}

}  // namespace hydra::app
