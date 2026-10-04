// Per-pad ghost/accent/normal note counts for a parsed chart.
//
// count_dynamics walks the Song's sequence once and bins every ChordNote by
// pad, cymbal flag, and dynamic type. The GUI (task 2) draws the result;
// this module is pure data, no UI dependency.

#ifndef HYDRA_APP_DYNAMICS_BREAKDOWN_H
#define HYDRA_APP_DYNAMICS_BREAKDOWN_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "parse/song.h"
#include "store/record_store.h"
#include "store/stored_versions.h"

namespace hydra {
namespace app {

struct DynamicsCounts {
    int ghost = 0, accent = 0, normal = 0;
    int all() const { return ghost + accent + normal; }
    bool has_dynamics() const { return ghost + accent > 0; }
    // Field-by-field addition: the one sum every total below is built from.
    DynamicsCounts& operator+=(const DynamicsCounts& o) {
        ghost += o.ghost;
        accent += o.accent;
        normal += o.normal;
        return *this;
    }
};

enum class DynamicsRow {
    RedSnare,
    YellowCymbal,
    YellowTom,
    BlueCymbal,
    BlueTom,
    GreenCymbal,
    GreenTom,
    Kick,
    Kick2x,
    Count
};

struct DynamicsBreakdown {
    std::array<DynamicsCounts, static_cast<size_t>(DynamicsRow::Count)> rows{};
    bool dynamics_enabled = false;
    // A late .mid dynamics tag (finding 64): its time in chart ms, and how
    // many marked notes came before it. nullopt and 0 when the tag came first
    // or there is none. The time is stored because the Dynamics tab draws
    // from the stored blob and has no tempo map to turn a tick into m:ss.
    std::optional<uint32_t> late_tag_ms;
    int marks_before_tag = 0;

    const DynamicsCounts& row(DynamicsRow r) const;
    DynamicsCounts pads_total() const;
    // The kick notes that count under this 2x Bass setting: the Kick row,
    // plus the 2x kick row when 2x Bass is on. The one answer to "which kicks
    // count" (finding 12): "All kicks" and Totals both read it.
    DynamicsCounts kicks_total(bool bass2x) const;
    // Every note that counts: pads_total() plus kicks_total(bass2x).
    DynamicsCounts played_total(bool bass2x) const;
};

// A row's name in the Dynamics tab, from note_label (core/model.h), the one
// name of a drum note: "Red snare", "Yellow cymbal", "2x kick", and "Red" or
// "Yellow" for a pad with Pro Drums off.
std::string dynamics_row_label(DynamicsRow r, bool pro);

DynamicsBreakdown count_dynamics(const Song& song);

// Versioned binary encoding for storage in the dynamics table (record_store.h).
// Version byte, then dynamics_enabled (1 byte), then the nine rows in
// DynamicsRow order, each as ghost/accent/normal, then the late tag's ms
// (0xFFFFFFFF when none) and the count of markings before it. Every number is
// a little-endian uint32.
std::vector<uint8_t> encode_dynamics(const DynamicsBreakdown& b);

// Returns nullopt on an unknown version or data too short.
std::optional<DynamicsBreakdown> decode_dynamics(const std::vector<uint8_t>& blob);

// ---- the cache rules, in one place ----------------------------------------

// The Dynamics tab's background count always parses with 2x kicks kept, so
// the "2x kick" row is known even while the "2x Bass" box is off. The parser
// reads only the parsed difficulty's own 2x kicks (D20), so below Expert this
// row never holds Expert's.
constexpr bool kDynamicsParseBass2x = true;

// Stored counts carry store::kDynamicsCountStamp and their blobs start with
// store::kDynamicsBlobStamp (store/stored_versions.h says when to bump each).

// The stored count for this key, or nullopt when there is none, its stamp
// isn't current, or it fails to decode. The caller then recounts.
std::optional<DynamicsBreakdown> load_stored_dynamics(store::RecordStore& store,
                                                      const store::DynamicsKey& key);

// Saves a count under this key, stamped kDynamicsCountStamp.written. Throws on a
// store failure.
void save_dynamics(store::RecordStore& store, const store::DynamicsKey& key,
                   const DynamicsBreakdown& breakdown);

// The in-memory key of one count: the chart file, the pro-drums view and the
// difficulty, as "path|pro|Expert" or "path|std|Hard".
std::string dynamics_cache_key(const std::string& notespath, bool pro, Difficulty difficulty);

// The stored-row key for one count.
store::DynamicsKey dynamics_store_key(const std::string& md5, Difficulty difficulty, bool pro);

// After an analysis, its dynamics count as a free by-product (the chart is
// already parsed), ready for RecordStore::save_analysis. nullopt when the
// analysis parsed with bass2x off (the parse dropped the 2x kicks and the
// counts would be incomplete) or the count fails: best effort, never a reason
// to lose the analysis record.
std::optional<store::DynamicsEntry> dynamics_entry_from_analysis(
    const std::string& md5, const Song& song, bool bass2x, Difficulty difficulty, bool pro);

}  // namespace app
}  // namespace hydra

#endif  // HYDRA_APP_DYNAMICS_BREAKDOWN_H
