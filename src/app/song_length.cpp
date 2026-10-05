// See song_length.h.

#include "app/song_length.h"

#include "app/preview_view.h"  // has_song_length
#include "audio/frames.h"      // chart_ms_of_audio_ms (header-only)
#include "core/strutil.h"      // parse_finite_number

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

std::optional<double> song_length_ms(const store::ChartTimingMeta& meta,
                                     std::optional<double> chart_offset_s,
                                     const ExpertChart& expert_chart) {
    std::optional<double> length;
    if (const std::optional<double> stated = stated_length_ms(meta.length_ms)) {
        length = audio::chart_ms_of_audio_ms(
            *stated, preview_audio_offset_ms(meta.delay_ms, chart_offset_s));
    } else {
        length = last_note_start_ms(expert_chart());
    }
    if (length && !has_song_length(*length)) length.reset();
    return length;
}

std::optional<double> chart_song_length_ms(const store::ChartTimingMeta& meta,
                                           const std::string& notespath, const Song& song,
                                           Difficulty difficulty, bool bass2x,
                                           const core::Rules& rules,
                                           const SharedBytes& container) {
    std::optional<Song> reparsed;
    return song_length_ms(meta, song.chart_offset_s, [&]() -> const Song& {
        if (difficulty == Difficulty::Expert && bass2x) return song;
        // Pro Drums only names a note's pad, never its time, so either value
        // gives the same last note.
        constexpr bool kPro = true;
        reparsed.emplace(container ? load_songpath_from_bytes(notespath, *container, kPro, true,
                                                              Difficulty::Expert, rules)
                                   : load_songpath(notespath, kPro, true, Difficulty::Expert,
                                                   rules));
        return *reparsed;
    });
}

}  // namespace hydra::app
