#include "app/dynamics_breakdown.h"

#include <cstring>

#include "app/display_format.h"
#include "core/model.h"
#include "parse/song.h"
#include "store/serialize.h"  // BinaryWriter, BinaryReader

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
        for (const ChordNote& note : ts.chord.notes()) {
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

// ---- encode / decode --------------------------------------------------------

namespace {

constexpr size_t kRowCount = static_cast<size_t>(DynamicsRow::Count);  // 9
// version(1) + dynamics_enabled(1) + 9 rows * 3 fields * 4 bytes
// + late tag ms(4) + marks before it(4) = 118. The old layout was 110 bytes;
// decode reads it as missing, so the tab recounts.
constexpr size_t kDynamicsBlobSize = 1 + 1 + kRowCount * 3 * 4 + 4 + 4;
constexpr uint32_t kNoLateTag = 0xFFFFFFFF;

}  // namespace

// The numbers go through the store's own codec (store/serialize.h), so the
// byte order of a stored number is written in one place.
std::vector<uint8_t> encode_dynamics(const DynamicsBreakdown& b) {
    store::BinaryWriter w;
    w.bytes.reserve(kDynamicsBlobSize);
    w.u8(store::kDynamicsBlobStamp.written);
    w.boolean(b.dynamics_enabled);
    for (size_t i = 0; i < kRowCount; ++i) {
        w.u32(static_cast<uint32_t>(b.rows[i].ghost));
        w.u32(static_cast<uint32_t>(b.rows[i].accent));
        w.u32(static_cast<uint32_t>(b.rows[i].normal));
    }
    w.u32(b.late_tag_ms.value_or(kNoLateTag));
    w.u32(static_cast<uint32_t>(b.marks_before_tag));
    return std::move(w.bytes);
}

std::optional<DynamicsBreakdown> decode_dynamics(const std::vector<uint8_t>& blob) {
    if (blob.size() < kDynamicsBlobSize) return std::nullopt;
    if (!store::kDynamicsBlobStamp.is_current(blob[0])) return std::nullopt;

    // The size check above means every read below is in range.
    store::BinaryReader r(blob);
    r.u8();  // the stamp, checked above
    DynamicsBreakdown b;
    b.dynamics_enabled = r.boolean();
    for (size_t i = 0; i < kRowCount; ++i) {
        b.rows[i].ghost = static_cast<int>(r.u32());
        b.rows[i].accent = static_cast<int>(r.u32());
        b.rows[i].normal = static_cast<int>(r.u32());
    }
    const uint32_t tag_ms = r.u32();
    if (tag_ms != kNoLateTag) b.late_tag_ms = tag_ms;
    b.marks_before_tag = static_cast<int>(r.u32());
    return b;
}

// ---- the cache rules --------------------------------------------------------

store::DynamicsKey dynamics_store_key(const std::string& md5, Difficulty difficulty, bool pro) {
    return store::DynamicsKey{md5, difficulty_name(difficulty), pro};
}

std::optional<DynamicsBreakdown> load_stored_dynamics(store::RecordStore& store,
                                                      const store::DynamicsKey& key) {
    auto blob = store.get_dynamics(key);
    if (!blob) return std::nullopt;
    return decode_dynamics(*blob);
}

void save_dynamics(store::RecordStore& store, const store::DynamicsKey& key,
                   const DynamicsBreakdown& breakdown) {
    store.put_dynamics(key, encode_dynamics(breakdown), store::kDynamicsCountStamp.written);
}

std::optional<store::DynamicsEntry> dynamics_entry_from_analysis(
    const std::string& md5, const Song& song, bool bass2x, Difficulty difficulty, bool pro) {
    if (!bass2x) return std::nullopt;  // the 2x kicks were dropped; the counts would be incomplete
    try {
        return store::DynamicsEntry{dynamics_store_key(md5, difficulty, pro),
                                    encode_dynamics(count_dynamics(song)),
                                    store::kDynamicsCountStamp.written};
    } catch (...) {
        // Best effort: never block the analysis record.
        return std::nullopt;
    }
}

}  // namespace app
}  // namespace hydra
