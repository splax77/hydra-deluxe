// How long a song is (D75, as amended by D255): the one owner,
// song_length_ms, and the rules it is built from. The Paths timeline, the
// Preview's scrub bar and its SP meter all show this one length. Nothing
// stores it: a click works it out through app::analysis_song_length (D87).
//
// No audio is opened here, and nothing measured from audio is an input. The
// length comes from the chart's own metadata, read by the library scan with
// the names (store::ChartTimingMeta), and from three facts the parsers store
// on the Song: the end event (Song::end_event_tick), the last note's end over
// every instrument (Song::last_note_end_tick) and the tempo map. Clone Hero
// works out a song's end from the same facts, with the audio's length where
// Hydra uses the stated length (D255).

#ifndef HYDRA_APP_SONG_LENGTH_H
#define HYDRA_APP_SONG_LENGTH_H

#include <optional>
#include <string_view>

#include "parse/song.h"
#include "store/record_store.h"  // ChartTimingMeta

namespace hydra::app {

// A length the metadata states (D75 item 1): `raw_ms` when has_song_length
// (app/preview_view.h) accepts it, else none. Every format's number comes
// through here: a .srb's field as it is, song.ini's and a .sng's text through
// stated_length_ms_of_text.
std::optional<double> stated_length_ms(std::optional<double> raw_ms);

// stated_length_ms of a metadata value written as text, read by
// core/strutil.h parse_finite_number.
std::optional<double> stated_length_ms_of_text(std::string_view text);

// The time Clone Hero adds after a song's end before its results screen
// (D255, the user's answer 2). song_length_ms adds it last, in chart time.
inline constexpr double kSongEndPadMs = 1000.0;

// Does `song`'s end event count toward its length? Only when the parser
// stored one and it sits at least one beat (Song::tick_resolution ticks) past
// Song::last_note_end_tick, compared in ticks, as Clone Hero compares it
// (D255, the user's answer 4). A song with no notes measures from tick 0.
bool end_event_counts(const Song& song);

// The song's one length, in chart time (D255, amending D75 items 1 and 2).
// Three candidates:
// - the stated length, `meta.length_ms` through stated_length_ms, moved from
//   audio time into chart time by the chart's delay or Offset
//   (preview_audio_offset_ms with `meta.delay_ms` and `song.chart_offset_s`,
//   then app/preview_source.h chart_ms_of_audio_ms). It is dropped when it ends
//   before the last note's end;
// - the end event, when end_event_counts says so, through the tempo map;
// - the last note's end (Song::last_note_end_tick), through the tempo map.
// Which one wins is the user's answer 1 in D255; the code below is its one
// statement. kSongEndPadMs is then added. Empty when no candidate gives a
// length has_song_length accepts (app/preview_view.h): the Preview's clock
// then runs free.
//
// `song` may be parsed at any difficulty and with any drum options: none of
// the facts read here depends on them.
std::optional<double> song_length_ms(const store::ChartTimingMeta& meta, const Song& song);

}  // namespace hydra::app

#endif  // HYDRA_APP_SONG_LENGTH_H
