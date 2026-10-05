// How long a song is (D75): the one owner, song_length_ms, and the rule for
// which stated lengths count. The Paths timeline, the Preview's scrub bar and
// its SP meter all show this one length; analysis, hydra_batch and the
// open-song backfill save it (RecordStore::save_analysis, fill_song_length).
//
// No audio is opened here. The length comes from the chart's own metadata,
// read by the library scan with the names (store::ChartTimingMeta), or from
// the chart's last Expert drum note when the metadata states none. The
// Preview's playback range is a different question (PreviewTransport::load,
// D48): audio past the song's length still plays.

#ifndef HYDRA_APP_SONG_LENGTH_H
#define HYDRA_APP_SONG_LENGTH_H

#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "app/preview_source.h"  // SharedBytes
#include "core/rules.h"
#include "parse/song.h"
#include "store/record_store.h"  // ChartTimingMeta

namespace hydra::app {

// A length the metadata states (D75 item 1): `raw_ms` when it counts, else
// none. Missing, zero, negative and non-finite values do not count. Every
// format's number comes through here: a .srb's field as it is, song.ini's and
// a .sng's text through stated_length_ms_of_text.
std::optional<double> stated_length_ms(std::optional<double> raw_ms);

// stated_length_ms of a metadata value written as text: empty or not a number
// (core/strutil.h parse_finite_number) counts as not stated.
std::optional<double> stated_length_ms_of_text(std::string_view text);

// When the last note of `song` starts, in chart time; empty when it has no
// notes. For the backup length (D75 item 2) `song` is the Expert drums chart
// parsed with 2x kick; chart_song_length_ms says how a caller gets one.
std::optional<double> last_note_start_ms(const Song& song);

// The song's one length, in chart time (D75). The metadata's length when it
// states one, moved from audio time into chart time by the chart's delay or
// Offset (preview_audio_offset_ms with `meta.delay_ms` and `chart_offset_s`,
// then audio/frames.h chart_ms_of_audio_ms). Otherwise the start of the last
// note of `expert_chart`, which is already in chart time. Empty when neither
// gives a length has_song_length accepts (app/preview_view.h).
//
// `expert_chart` is asked only when the metadata states no length. It returns
// the chart's Expert drums with 2x kick, every note in the file, whatever
// difficulty and drum options the caller analyzed at. The song it hands back
// need only live until song_length_ms returns.
using ExpertChart = std::function<const Song&()>;
std::optional<double> song_length_ms(const store::ChartTimingMeta& meta,
                                     std::optional<double> chart_offset_s,
                                     const ExpertChart& expert_chart);

// song_length_ms for the chart at `notespath`, already parsed as `song` at
// `difficulty` with 2x kick on or off (`bass2x`). The Offset is read from
// `song`. The backup reads `song` too when it is the Expert chart with 2x
// kick; otherwise the chart is parsed once more at Expert with 2x, from
// `container` when given (read_preview_container's bytes, so a .sng or .srb
// is not read from disk again). `rules` places that parse's fills, which no
// note time depends on.
std::optional<double> chart_song_length_ms(const store::ChartTimingMeta& meta,
                                           const std::string& notespath, const Song& song,
                                           Difficulty difficulty, bool bass2x,
                                           const core::Rules& rules,
                                           const SharedBytes& container = nullptr);

}  // namespace hydra::app

#endif  // HYDRA_APP_SONG_LENGTH_H
