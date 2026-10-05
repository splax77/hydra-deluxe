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

// ---- Results (the results and paths tables) --------------------------------

// The analysis version. BUMP IT, to the version of the release that ships the
// change, whenever analysis output changes in a way the path format and the
// hydra_rules.ini fingerprint don't already catch: the engine (src/search),
// the scoring (src/core), the chart readers (src/parse) when a chart reads
// differently, or what a record holds. Then shrink `accepted` to the new stamp
// alone. A stale result reads Stale and asks for re-analysis. Changes that
// ship in the same release share one bump (decision D23, 2026-10-03). The
// summary columns (bestpath, score, stars, hardest_ms and the rest) are a
// cache of the stored paths, so a change that alters any of them is covered
// by this rule too (record_store.cpp, kSummaryColumnList).
//
// "2.1.0": the engine now stores each activation's SP-end history and
// re-anchors the transfer scales on it (ADR 0021), and each tied variant
// keeps its own facts (ADR 0022). Every release from 1.8.4 to 2.0.0 stamped
// "1.8.2", and those results hold different values, so they read Stale.
inline constexpr StampRule<std::string_view, 1> kResultsStamp{"2.1.0", {"2.1.0"}};

// The stored path layout: the structure blob and the node payloads it points
// at share this one number, because a node is only ever read through its
// structure. BUMP IT when either layout changes. Its history:
// 3 added clamp_tick to nodes. 4 put the u64 rules fingerprint right after
// the version and added sqout_tick and collected_phrase_ticks (ADR 0014).
// 5 spelled every chord as its lane code (ADR 0015). 6 moved the multiplier
// squeezes and root totals into the structure and left a node holding its
// activations alone (ADR 0017). 7 stored the SP-end history and the bank and
// fill lists, stored the squeeze-out once, and gave each SqIn its own
// transfer scale (ADR 0021). The history replaced three node fields: the
// deact node, the clamp note and the collected phrases. The single
// transfer_pre pair is gone. A squeeze entry lost its kind byte, since only
// SqIns are stored now. The squeeze-out tick now comes before the history,
// where it used to follow the deact and clamp ticks. A root's leftover SP
// changed from one number to its list of bank ticks. Format 7 also dropped
// each row's is_sp flag, and a transfer scale now starts with a presence
// byte (one byte saying whether the value follows), so it can be stored as
// unknown. Each tied variant got its own trailing bank in its tree entry
// (ADR 0022).
inline constexpr StampRule<uint32_t, 1> kPathFormatStamp{7, {7}};

// ---- Dynamics counts (the dynamics table) ----------------------------------

// How notes are counted. BUMP IT (add 1) whenever count_dynamics, or the
// parser that feeds it, changes what any chart counts: src/parse/song.cpp
// (the .mid, .chart and .sng loaders), src/parse/midi.cpp and
// src/parse/srb.cpp. The hydra_rules.ini fingerprint doesn't apply: the rules
// move activation marks, never which notes a chart has. A stale count reads as
// missing, and the Dynamics tab recounts it in the background.
// 0 = rows saved before the stamp existed. 1 = the first stamp.
// 2 = step 2: disco flip and 2x kicks are read per difficulty, and the
// dynamics tag counts only in Clone Hero's two spellings, in file order
// (decisions D19, D20 and D24, 2026-10-03).
inline constexpr StampRule<int, 1> kDynamicsCountStamp{2, {2}};

// The dynamics blob's byte layout. BUMP IT when encode_dynamics changes.
// 2 added late_tag_ms (a late dynamics tag's time in ms) and
// marks_before_tag (the marked notes before it), for the "from <time> on"
// line (D24).
inline constexpr StampRule<uint8_t, 1> kDynamicsBlobStamp{2, {2}};

// ---- Scanned chart facts (the charts table) --------------------------------

// How a scan reads a chart's identity and names. BUMP IT (add 1) whenever any
// of these changes what an unchanged file reads as: hash_chart_file and its
// 1 MB .sng head rule (D51 call 13), sig_of (the size and mtime fingerprint
// the rescan cache is keyed on), and the song.ini, .sng and .srb name
// readers. The sig only says a file is unchanged; this stamp says the rows
// read from it still hold what this build would read. Stored once per file,
// as the meta row chart_meta_version. A stale or missing stamp drops the
// whole rescan cache, so the next scan reads every chart once (D51 call 12).
// 1 = the first stamp.
inline constexpr StampRule<int, 1> kChartMetaStamp{1, {1}};

// A build always reads back what it writes.
static_assert(kResultsStamp.is_current(kResultsStamp.written));
static_assert(kPathFormatStamp.is_current(kPathFormatStamp.written));
static_assert(kDynamicsCountStamp.is_current(kDynamicsCountStamp.written));
static_assert(kDynamicsBlobStamp.is_current(kDynamicsBlobStamp.written));
static_assert(kChartMetaStamp.is_current(kChartMetaStamp.written));

}  // namespace hydra::store

#endif  // HYDRA_STORE_STORED_VERSIONS_H
