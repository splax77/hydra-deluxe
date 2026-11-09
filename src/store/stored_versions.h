// Every version stamp on stored, computed data, in one file (ADR 0018).
//
// A saved row was computed by some build of Hydra. Before this build shows
// it, it asks one question: would this build have computed the same thing?
// Each kind of stored data answers with a stamp saved on the row. A StampRule
// holds the stamp this build writes and every stamp it reads as current, and
// StampRule::is_current is the only place a stored stamp is compared. The
// store's SQL twin of the results check (record_store.cpp) is built from the
// same lists.
//
// None of these is the app version. A release that changes nothing a stamp
// covers keeps every saved row.

#ifndef HYDRA_STORE_STORED_VERSIONS_H
#define HYDRA_STORE_STORED_VERSIONS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace hydra::store {

template <typename T, size_t N>
struct StampRule {
    T written;                  // the stamp this build writes
    std::array<T, N> accepted;  // every stamp read as current; holds `written`

    constexpr bool is_current(const T& stored) const {
        for (const T& a : accepted)
            if (a == stored) return true;
        return false;
    }
};

// ---- Results (the profiles, records and record_paths tables) ----------------

// The stamp lives on the profile row a record is filed under, not on the
// record itself: RecordStore::profile_id_for finds or files that row
// (record_store.cpp).

// The analysis version. BUMP IT, to the version of the release that ships the
// change, whenever analysis output changes in a way the hydra_rules.ini
// fingerprint doesn't already catch: the engine (src/search), the scoring
// (src/core), the chart readers (src/parse) when a chart reads differently:
// anything a fresh analysis would compute differently. Then shrink `accepted`
// to the new stamp alone. A stale result reads Stale and asks for
// re-analysis. Changes that ship in the same release share one bump (decision
// D23, 2026-10-03). A change to how a saved figure is shown is not a bump:
// Stale is for a change in the analysis, and display text reads what the
// engine already saved (the user's ruling, 2026-10-31). hardest_ms is the
// example: the store saves the engine's own figure (engine_hardest_ms,
// core/model.h) and the screens apply shown_timing when they show it. The
// stamp is only compared as a whole string, never parsed.
inline constexpr StampRule<std::string_view, 1> kResultsStamp{"2.14.1+coda", {"2.14.1+coda"}};

// ---- Scanned chart facts (the charts table) --------------------------------

// How a scan reads a chart's identity and names. BUMP IT (add 1) whenever any
// of these changes what an unchanged file reads as: hash_chart_file (the song
// id rule, song_id_read in app/analysis.cpp), sig_of (the size and mtime fingerprint
// the rescan cache is keyed on), and the song.ini, .sng and .srb metadata
// readers (names, stated length and delay). The sig only says a file is
// unchanged; this stamp says the rows
// read from it still hold what this build would read. Stored once per file,
// as the meta row chart_meta_version. A stale or missing stamp drops the
// whole rescan cache, so the next scan reads every chart once (D51 call 12).
inline constexpr StampRule<int, 1> kChartMetaStamp{6, {6}};

// A build always reads back what it writes.
static_assert(kResultsStamp.is_current(kResultsStamp.written));
static_assert(kChartMetaStamp.is_current(kChartMetaStamp.written));

}  // namespace hydra::store

#endif  // HYDRA_STORE_STORED_VERSIONS_H
