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
    T written;                  // stamped on every new row
    std::array<T, N> accepted;  // every stamp read as current; holds `written`

    constexpr bool is_current(const T& stored) const {
        for (const T& a : accepted)
            if (a == stored) return true;
        return false;
    }
};

// ---- Results (the results table) -------------------------------------------

// The analysis version. BUMP IT, to the version of the release that ships the
// change, whenever analysis output changes in a way the hydra_rules.ini
// fingerprint doesn't already catch: the engine (src/search), the scoring
// (src/core), the chart readers (src/parse) when a chart reads differently,
// or what a summary row holds. Then shrink `accepted` to the new stamp alone.
// A stale result reads Stale and asks for re-analysis. Changes that ship in
// the same release share one bump (decision D23, 2026-10-03). The summary
// columns (bestpath, score, stars, hardest_ms and the rest) are the
// analysis's own output, so a change that alters any of them is covered by
// this rule too (record_store.cpp, kSummaryColumnList).
//
// "2.1.0": the engine now stores each activation's SP-end history and
// re-anchors the transfer scales on it (ADR 0021), and each tied variant
// keeps its own facts (ADR 0022). Every release from 1.8.4 to 2.0.0 stamped
// "1.8.2", and those results hold different values, so they read Stale.
// "2.1.0+allzero": the all-0 list is found under its 0 ms limit again
// (D85). It is a second bump inside 2.1.0, made before it shipped, an
// exception to D23's one-bump-per-release rule that the user chose ("Mark
// all Stale"), so results saved by earlier 2.1.0 builds, which can be
// missing their all-0 path, read Stale too. The stamp is only compared as a
// whole string, never parsed.
// "2.4.0": the chart readers merge a 2x kick and a normal kick on one tick,
// and 2x Bass off then removes that kick, as Clone Hero does (D105). D105
// accepted that every saved result reads Stale.
inline constexpr StampRule<std::string_view, 1> kResultsStamp{"2.4.0", {"2.4.0"}};

// ---- Scanned chart facts (the charts table) --------------------------------

// How a scan reads a chart's identity and names. BUMP IT (add 1) whenever any
// of these changes what an unchanged file reads as: hash_chart_file and its
// 1 MB .sng head rule (D51 call 13), sig_of (the size and mtime fingerprint
// the rescan cache is keyed on), and the song.ini, .sng and .srb metadata
// readers (names, stated length and delay). The sig only says a file is
// unchanged; this stamp says the rows
// read from it still hold what this build would read. Stored once per file,
// as the meta row chart_meta_version. A stale or missing stamp drops the
// whole rescan cache, so the next scan reads every chart once (D51 call 12).
// 1 = the first stamp. 2 = the scan also reads each chart's stated length
// and delay (store::ChartTimingMeta, D75).
inline constexpr StampRule<int, 1> kChartMetaStamp{2, {2}};

// A build always reads back what it writes.
static_assert(kResultsStamp.is_current(kResultsStamp.written));
static_assert(kChartMetaStamp.is_current(kChartMetaStamp.written));

}  // namespace hydra::store

#endif  // HYDRA_STORE_STORED_VERSIONS_H
