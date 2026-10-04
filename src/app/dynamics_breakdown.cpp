#include "app/dynamics_breakdown.h"

#include <cstring>

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

// ---- labels -------------------------------------------------------------

const char* dynamics_row_label(DynamicsRow r, bool pro) {
    switch (r) {
        case DynamicsRow::RedSnare:
            return pro ? "Red snare" : "Red";
        case DynamicsRow::YellowCymbal:
            return "Yellow cymbal";
        case DynamicsRow::YellowTom:
            return pro ? "Yellow tom" : "Yellow";
        case DynamicsRow::BlueCymbal:
            return "Blue cymbal";
        case DynamicsRow::BlueTom:
            return pro ? "Blue tom" : "Blue";
        case DynamicsRow::GreenCymbal:
            return "Green cymbal";
        case DynamicsRow::GreenTom:
            return pro ? "Green tom" : "Green";
        case DynamicsRow::Kick:
            return "Kick";
        case DynamicsRow::Kick2x:
            return "2x kick";
        default:
            return "";
    }
}

// ---- counting -----------------------------------------------------------

namespace {

DynamicsRow row_for(const ChordNote& note) {
    switch (note.colortype) {
        case NoteColor::Kick:
            return note.is2x ? DynamicsRow::Kick2x : DynamicsRow::Kick;
        case NoteColor::Red:
            return DynamicsRow::RedSnare;
        case NoteColor::Yellow:
            return note.is_cymbal() ? DynamicsRow::YellowCymbal
                                    : DynamicsRow::YellowTom;
        case NoteColor::Blue:
            return note.is_cymbal() ? DynamicsRow::BlueCymbal
                                    : DynamicsRow::BlueTom;
        case NoteColor::Green:
            return note.is_cymbal() ? DynamicsRow::GreenCymbal
                                    : DynamicsRow::GreenTom;
    }
    return DynamicsRow::Kick;  // unreachable
}

}  // namespace

DynamicsBreakdown count_dynamics(const Song& song) {
    DynamicsBreakdown bd;
    bd.dynamics_enabled = song.dynamics_enabled;
    if (song.dynamics_late_tag_tick)
        bd.late_tag_ms = static_cast<uint32_t>(song.timecode(*song.dynamics_late_tag_tick).ms());
    bd.marks_before_tag = song.dynamics_marks_before_tag;

    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& note : ts.chord.notes()) {
            DynamicsCounts& c = bd.rows[static_cast<size_t>(row_for(note))];
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

void write_u32_le(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>(v));
    out.push_back(static_cast<uint8_t>(v >> 8));
    out.push_back(static_cast<uint8_t>(v >> 16));
    out.push_back(static_cast<uint8_t>(v >> 24));
}

uint32_t read_u32_le(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

}  // namespace

std::vector<uint8_t> encode_dynamics(const DynamicsBreakdown& b) {
    std::vector<uint8_t> out;
    out.reserve(kDynamicsBlobSize);
    out.push_back(store::kDynamicsBlobStamp.written);
    out.push_back(b.dynamics_enabled ? 1 : 0);
    for (size_t i = 0; i < kRowCount; ++i) {
        write_u32_le(out, static_cast<uint32_t>(b.rows[i].ghost));
        write_u32_le(out, static_cast<uint32_t>(b.rows[i].accent));
        write_u32_le(out, static_cast<uint32_t>(b.rows[i].normal));
    }
    write_u32_le(out, b.late_tag_ms.value_or(kNoLateTag));
    write_u32_le(out, static_cast<uint32_t>(b.marks_before_tag));
    return out;
}

std::optional<DynamicsBreakdown> decode_dynamics(const std::vector<uint8_t>& blob) {
    if (blob.size() < kDynamicsBlobSize) return std::nullopt;
    if (!store::kDynamicsBlobStamp.is_current(blob[0])) return std::nullopt;

    DynamicsBreakdown b;
    b.dynamics_enabled = blob[1] != 0;
    const uint8_t* p = blob.data() + 2;
    for (size_t i = 0; i < kRowCount; ++i) {
        b.rows[i].ghost  = static_cast<int>(read_u32_le(p));      p += 4;
        b.rows[i].accent = static_cast<int>(read_u32_le(p));      p += 4;
        b.rows[i].normal = static_cast<int>(read_u32_le(p));      p += 4;
    }
    const uint32_t tag_ms = read_u32_le(p);                        p += 4;
    if (tag_ms != kNoLateTag) b.late_tag_ms = tag_ms;
    b.marks_before_tag = static_cast<int>(read_u32_le(p));
    return b;
}

// ---- the cache rules --------------------------------------------------------

std::string dynamics_cache_key(const std::string& notespath, bool pro, Difficulty difficulty) {
    return notespath + "|" + (pro ? "pro" : "std") + "|" + difficulty_name(difficulty);
}

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
