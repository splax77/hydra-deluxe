// The lasting bank-list checks (D3, finding 89), shared by the corpus test
// (test_search.cpp) and the fast-tempo charts (test_fast_tempo.cpp).
//
// These are facts the chart and the window give on their own. They do not
// restate when the engine says a bar arrives. A banked bar arrives on a real
// SP-phrase-end note, or, for a squeezed-out bar, at the deact node; every
// list is strictly ascending (a phrase banks once); nothing arrives before the
// window that precedes it closed; and a phrase a squeeze-in spent never banks
// a bar. The D34 one-step-per-SqIn check lives here too, beside the
// spent-phrase check.

#ifndef HYDRA_TESTS_BANK_CHECK_H
#define HYDRA_TESTS_BANK_CHECK_H

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <vector>

#include "doctest.h"

#include "core/model.h"
#include "core/replay.h"  // sqin_phrase_ticks: a window's SqIn step ticks
#include "parse/song.h"

namespace bank_check {

// The ticks of the song's SP-phrase-end notes.
inline std::set<int64_t> phrase_ends(const hydra::Song& song) {
    std::set<int64_t> out;
    for (const hydra::SongTimestamp& ts : song.sequence)
        if (ts.flag_sp) out.insert(ts.timecode.ticks());
    return out;
}

// Checks one bank list: the bars banked after `prev` closed (nullptr: since the
// chart began), up to `last_tick`.
inline void check_bank_list(const std::vector<int64_t>& ticks, const hydra::Activation* prev,
                            int64_t last_tick, const std::set<int64_t>& phrase_ends) {
    const std::optional<int64_t> deact = prev ? prev->deact_tick() : std::nullopt;
    const int64_t floor = deact ? *deact : std::numeric_limits<int64_t>::min();
    // A squeeze-out leaves one bar, banked by the player's hit on the phrase.
    // That hit can land at the deact node itself.
    const bool squeezed_out = prev && prev->sqout_tick.has_value();
    if (squeezed_out) REQUIRE_FALSE(ticks.empty());
    for (size_t k = 0; k < ticks.size(); ++k) {
        CAPTURE(k);
        CAPTURE(ticks[k]);
        CHECK(ticks[k] >= floor);
        CHECK(ticks[k] <= last_tick);
        if (k > 0) CHECK(ticks[k] > ticks[k - 1]);
        const bool at_deact_node = squeezed_out && k == 0 && deact && ticks[k] == *deact;
        CHECK((phrase_ends.count(ticks[k]) == 1 || at_deact_node));
    }
}

// A squeeze-in's phrase went into SP, so none of the path's bank lists holds
// it. That includes a late squeeze-in whose new end comes before its phrase
// (D32): the path deactivates first, and hitting the spent phrase later must
// add no bar.
inline void check_spent_phrases(const hydra::Path& p) {
    std::set<int64_t> sqin_phrases;
    for (const hydra::Activation& act : p.walk_activations())
        for (const int64_t t : hydra::sqin_phrase_ticks(act)) sqin_phrases.insert(t);
    auto check = [&sqin_phrases](const std::vector<int64_t>& ticks) {
        for (const int64_t t : ticks) {
            CAPTURE(t);
            CHECK(sqin_phrases.count(t) == 0);
        }
    };
    for (const hydra::Activation& act : p.walk_activations()) {
        CAPTURE(act.timecode.ticks());
        check(act.bank_rise_ticks);
    }
    check(p.trailing_bank_ticks);
}

// A late squeeze-in writes its own SqIn step on its phrase. The phrase is
// spent then, so no window holds two SqIn steps on one phrase (a late SqIn's
// phrase squeezed in again at the next SP end wrote a second one). D34: a
// phrase is squeezed in once, so each SqIn has its own step (Appendix B's
// first guarantee: a relabel that found nothing cannot pass silently), and
// the phrase a window squeezes out is none it squeezed in.
inline void check_one_step_per_sqin(const hydra::Activation& a) {
    CAPTURE(a.timecode.ticks());
    std::vector<int64_t> ticks = hydra::sqin_phrase_ticks(a);
    std::sort(ticks.begin(), ticks.end());
    CHECK(std::adjacent_find(ticks.begin(), ticks.end()) == ticks.end());
    const size_t sqins = static_cast<size_t>(
        std::count_if(a.sqinouts.begin(), a.sqinouts.end(), hydra::is_sqin_squeeze));
    CHECK(sqins == ticks.size());
    if (a.sqout_tick) CHECK_FALSE(std::binary_search(ticks.begin(), ticks.end(), *a.sqout_tick));
}

// The same, on every window of one path.
inline void check_one_step_per_sqin(const hydra::Path& p) {
    CAPTURE(p.pathstring());
    for (const hydra::Activation& a : p.walk_activations()) check_one_step_per_sqin(a);
}

// Every bank list of one path: each window's, then the bars still banked when
// the last window closed (or from the start, for a path with no window), plus
// the spent-phrase check above. Returns the number of windows walked.
inline int check_path_banks(const hydra::Path& p, const std::set<int64_t>& phrase_ends,
                            int64_t chart_end) {
    int acts = 0;
    const hydra::Activation* prev = nullptr;
    for (const hydra::Activation& act : p.walk_activations()) {
        CAPTURE(act.timecode.ticks());
        ++acts;
        check_bank_list(act.bank_rise_ticks, prev, act.timecode.ticks(), phrase_ends);
        prev = &act;
    }
    check_bank_list(p.trailing_bank_ticks, prev, chart_end, phrase_ends);
    check_spent_phrases(p);
    return acts;
}

}  // namespace bank_check

#endif  // HYDRA_TESTS_BANK_CHECK_H
