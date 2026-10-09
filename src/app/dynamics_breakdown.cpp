#include "app/dynamics_breakdown.h"

#include <cstring>

#include "app/display_format.h"
#include "core/model.h"
#include "parse/song.h"

namespace hydra {
namespace app {

// ---- DynamicsBreakdown --------------------------------------------------

const DynamicsCounts& DynamicsBreakdown::row(DynamicsRow r) const {
    return rows[static_cast<size_t>(r)];
}

DynamicsCounts DynamicsBreakdown::pads_total() const {
    DynamicsCounts t;
    for (size_t i = 0; i <= static_cast<size_t>(DynamicsRow::GreenTom); ++i) t += rows[i];
    return t;
}

DynamicsCounts DynamicsBreakdown::kicks_total(bool bass2x) const {
    DynamicsCounts t = row(DynamicsRow::Kick);
    if (bass2x) t += row(DynamicsRow::Kick2x);
    return t;
}

DynamicsCounts DynamicsBreakdown::played_total(bool bass2x) const {
    DynamicsCounts t = pads_total();
    t += kicks_total(bass2x);
    return t;
}

// ---- the row table --------------------------------------------------------

namespace {

// The nine rows in DynamicsRow order, each with the notes it holds. Red has
// no cymbal row: a red note always counts as the snare.
constexpr DynamicsRowInfo kDynamicsRows[] = {
    {DynamicsRow::RedSnare,     NoteColor::Red,    false, false},
    {DynamicsRow::YellowCymbal, NoteColor::Yellow, true,  false},
    {DynamicsRow::YellowTom,    NoteColor::Yellow, false, false},
    {DynamicsRow::BlueCymbal,   NoteColor::Blue,   true,  false},
    {DynamicsRow::BlueTom,      NoteColor::Blue,   false, false},
    {DynamicsRow::GreenCymbal,  NoteColor::Green,  true,  false},
    {DynamicsRow::GreenTom,     NoteColor::Green,  false, false},
    {DynamicsRow::Kick,         NoteColor::Kick,   false, false},
    {DynamicsRow::Kick2x,       NoteColor::Kick,   false, true},
};
static_assert(std::size(kDynamicsRows) == static_cast<size_t>(DynamicsRow::Count),
              "one table entry per Dynamics row");

constexpr bool rows_in_order() {
    for (size_t i = 0; i < std::size(kDynamicsRows); ++i)
        if (static_cast<size_t>(kDynamicsRows[i].row) != i) return false;
    return true;
}
static_assert(rows_in_order(), "the table lists the rows in DynamicsRow order");

// Does this row hold its lane's notes with the lane flag on (lane_flag)?
constexpr bool row_has_flag(const DynamicsRowInfo& info) {
    return info.cymbal || info.is2x;
}

}  // namespace

const DynamicsRowInfo& dynamics_row_info(DynamicsRow r) {
    return kDynamicsRows[static_cast<size_t>(r)];
}

DynamicsRow dynamics_row_for(const ChordNote& note) {
    // A row is a lane with its flag on or off (lane_flag). A flag the lane
    // cannot carry (a red cymbal, if a chart ever set one) counts as off, so
    // that note still counts as the snare.
    const bool flag = lane_allows_flag(note.colortype) && lane_flag(note);
    for (const DynamicsRowInfo& info : kDynamicsRows)
        if (info.color == note.colortype && row_has_flag(info) == flag) return info.row;
    return DynamicsRow::Kick;  // unreachable: every lane has a row
}

// ---- labels -------------------------------------------------------------

ChordNote dynamics_row_note(DynamicsRow r) {
    const DynamicsRowInfo& info = dynamics_row_info(r);
    ChordNote note{info.color};
    if (row_has_flag(info)) set_lane_flag(note);
    return note;
}

std::string dynamics_row_label(DynamicsRow r, bool pro) {
    if (r == DynamicsRow::Count) return std::string();
    return note_label(dynamics_row_note(r), pro);
}

std::string dynamics_kick2x_line(const DynamicsBreakdown& bd) {
    // Every kick, 2x Bass on or off: the line is a fact about the chart.
    const int twice = bd.row(DynamicsRow::Kick2x).all();
    const int total = bd.kicks_total(/*bass2x=*/true).all();
    return "2x kicks: " + group_thousands(twice) + " of " +
           counted(total, "kick note", "kick notes") + " (" + dynamics_share(twice, total) + ")";
}

std::string dynamics_share(int part, int total) {
    return total > 0 ? format_percent(part, total, 0) : "0%";
}

DynamicsBreakdown count_dynamics(const Song& song) {
    DynamicsBreakdown bd;
    bd.dynamics_enabled = song.dynamics_enabled;
    if (song.dynamics_late_tag_tick)
        bd.late_tag_ms = static_cast<uint32_t>(song.timecode(*song.dynamics_late_tag_tick).ms());
    bd.marks_before_tag = song.dynamics_marks_before_tag;

    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& note : ts.chord.note_list(false)) {
            DynamicsCounts& c = bd.rows[static_cast<size_t>(dynamics_row_for(note))];
            switch (note.dynamictype) {
                case NoteDynamicType::Ghost:  ++c.ghost;  break;
                case NoteDynamicType::Accent: ++c.accent; break;
                case NoteDynamicType::Normal: ++c.normal; break;
            }
        }
    }
    return bd;
}

// ---- the count's parse ------------------------------------------------------

Song load_dynamics_song(const std::string& notespath, bool pro, Difficulty difficulty,
                        bool noteshuffle) {
    return load_songpath(notespath, pro, kDynamicsParseBass2x, difficulty,
                         core::default_rules(), noteshuffle);
}

bool analysis_parse_counts_dynamics(bool bass2x) { return bass2x == kDynamicsParseBass2x; }

DynamicsBreakdown dynamics_for_settings(const std::string& notespath, bool pro, bool bass2x,
                                        Difficulty difficulty, bool noteshuffle,
                                        const Song* analysis_song) {
    if (analysis_song && analysis_parse_counts_dynamics(bass2x))
        return count_dynamics(*analysis_song);
    // The shuffle's seed reads the first notes, kicks included, so with 2x
    // Bass off a shuffled chart's pads can differ from the count's own
    // 2x-kept parse. Kicks never move, so the kick rows still come from it.
    const bool pads_differ = noteshuffle && !analysis_parse_counts_dynamics(bass2x);
    DynamicsBreakdown bd =
        count_dynamics(load_dynamics_song(notespath, pro, difficulty, noteshuffle && !pads_differ));
    if (!pads_differ) return bd;
    const DynamicsBreakdown pads = count_dynamics(
        analysis_song ? *analysis_song
                      : load_songpath(notespath, pro, bass2x, difficulty, core::default_rules(),
                                      noteshuffle));
    for (size_t i = 0; i <= static_cast<size_t>(DynamicsRow::GreenTom); ++i)
        bd.rows[i] = pads.rows[i];
    return bd;
}

}  // namespace app
}  // namespace hydra
