// The replay's open-window walk. The rest of the replay's invariants are in
// test_replay.cpp.
//
// replay_path keeps a short list of the windows that can still pay the
// current chord, rather than asking every window about every chord. The cases
// below hold that walk to the naive answer on the corpus's own stored paths:
// each window pays each chord what the engine's one price for a backend row
// (core::backend_row_value) says, a chord's SP points are the sum of what each
// window pays it, and the order the windows are listed in does not matter.

#include "doctest.h"

#include <rapidcheck.h>

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/backend_value.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "replay_util.h"
#include "store/record_store.h"

using namespace hydra;

// Shared with test_replay.cpp (replay_util.h): Round and Round's shape on a
// hand-built chart, and where two replays first differ, row by row.
using test::first_result_difference;
using test::squeeze_chart;

namespace {

// The squeeze-out rules a rules file can pick (core::sqout_rule_from_name
// reads either), each over `base`.
std::vector<core::Rules> sqout_rule_variants(const core::Rules& base) {
    std::vector<core::Rules> out;
    for (const core::SqOutRule rule : {core::SqOutRule::FirstNote, core::SqOutRule::WholeChord}) {
        core::Rules r = base;
        r.sqout_rule = rule;
        out.push_back(r);
    }
    return out;
}

std::string rules_label(const core::Rules& r) { return core::sqout_rule_name(r.sqout_rule); }

ReplayWindow window(int64_t act, int64_t deact, std::optional<int64_t> sqout = std::nullopt) {
    ReplayWindow w;
    w.act_tick = act;
    w.deact_tick = deact;
    w.sqout_tick = sqout;
    return w;
}

// How many rows break a rule, and the first of them.
struct BadRows {
    size_t count = 0;
    size_t first = 0;
    void add(size_t i) {
        if (count++ == 0) first = i;
    }
};

// The chart's ChordsOnly chord-score table under each squeeze-out rule among
// `variants`, built once so every replay below reuses it instead of building
// its own.
class TablesByRule {
public:
    TablesByRule(const Song& song, const std::vector<core::Rules>& variants) {
        for (const core::Rules& r : variants) {
            bool have = false;
            for (const auto& t : tables_) have = have || t.first == r.sqout_rule;
            if (!have) tables_.emplace_back(r.sqout_rule, chord_score_table(song, r.sqout_rule));
        }
    }
    const ChordScoreTable& operator()(const core::Rules& r) const {
        for (const auto& t : tables_)
            if (t.first == r.sqout_rule) return t.second;
        throw std::logic_error("no chord-score table built for this squeeze-out rule");
    }

private:
    std::vector<std::pair<core::SqOutRule, ChordScoreTable>> tables_;
};

// A scores-only replay; its score fields are a full replay's.
ReplayOptions scores_only() {
    ReplayOptions o;
    o.scores_only = true;
    return o;
}

// The chord `i`'s offset from `w`'s SP end, in ms, as the engine measures
// every backend row (offset_from_sp_end).
double offset_from_end(const Song& song, size_t i, const ReplayWindow& w) {
    return offset_from_sp_end(song.sequence[i].timecode.ms(),
                              song.timing().timecode(w.deact_tick).ms());
}

// What window `w` alone pays chord `i`: nothing before the window opens, and
// from then on the engine's one price for a backend row
// (core::backend_row_value) at the chord's offset from w's SP end.
int window_pays(const Song& song, const ChordScoreTable& table, size_t i, const ReplayWindow& w,
                const core::Rules& rules) {
    const int64_t tick = song.sequence[i].timecode.ticks();
    if (tick < w.act_tick) return 0;
    const ChordScoreRow& sg = table.rows[i];
    return core::backend_row_value(offset_from_end(song, i, w), sg.sp, sg.sqout_sp,
                                   core::sqout_position(tick, w.sqout_tick),
                                   rules.backend_leeway_ms);
}

// The rows of `all`, the replay of `wl`, that the windows' own prices do not
// give: a row's SP points must be the sum of what each window pays it, the
// windows that pay it must be the ones its row lists, Star Power must mark it
// exactly when one pays it (D2), and its disc must be the chart's multiplier
// after the chord (the chord-score table's) doubled exactly then.
BadRows rows_off_price(const Song& song, const std::vector<ReplayWindow>& wl,
                       const core::Rules& rules, const ReplayResult& all,
                       const ChordScoreTable& table) {
    if (all.chords.size() != song.sequence.size())
        throw std::logic_error("the replay has another row count than the chart");
    BadRows bad;
    for (size_t i = 0; i < all.chords.size(); ++i) {
        int64_t sum = 0;
        size_t payers = 0;
        for (const ReplayWindow& w : wl) {
            const int paid = window_pays(song, table, i, w, rules);
            sum += paid;
            if (paid > 0) ++payers;
        }
        const ReplayChord& c = all.chords[i];
        const bool sp_paid = payers > 0;
        if (c.points.sp != sum || c.paid_by.size() != payers || c.in_sp != sp_paid ||
            c.multiplier_shown != shown_multiplier(table.rows[i].multiplier_after, sp_paid))
            bad.add(i);
    }
    return bad;
}

void check_rows_priced(const Song& song, const std::vector<ReplayWindow>& wl,
                       const core::Rules& rules, const ReplayResult& all,
                       const ChordScoreTable& table, const std::string& what) {
    const BadRows bad = rows_off_price(song, wl, rules, all, table);
    INFO(what << ": first row its windows' prices do not give: " << bad.first);
    CHECK(bad.count == 0);
}

// One corpus chart under the app's default settings: its song and the record
// the engine stores for it.
struct CorpusChart {
    std::string path;
    const Song* song = nullptr;
    const HydraRecord* record = nullptr;
};

// Every corpus chart under `cfg`. A chart that loads with no notes fails the
// calling test, by name; it is never passed over.
std::vector<CorpusChart> analyzed_corpus_charts(const app::AnalysisSettings& cfg) {
    std::vector<CorpusChart> out;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        const std::string empty_msg = path + " loads with no notes";
        CHECK_MESSAGE(!song.is_empty(), empty_msg);
        if (song.is_empty()) continue;
        out.push_back(CorpusChart{path, &song, &corpus::analyzed(path, cfg)});
    }
    return out;
}

}  // namespace

// Each window of every corpus chart's stored root paths, replayed alone under
// each squeeze-out rule, pays each chord exactly the engine's price for it.
// Its named edges: a chord at or before the SP end, not after a squeezed-out
// chord, is paid; a chord at least the leeway past the end is not; nothing
// after the squeezed-out chord is paid. The leeway's own strict edge is
// core::counted_without_squeeze's, held in test_model.cpp.
TEST_CASE("replay: each window alone pays each chord the engine's price, on every corpus path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::vector<core::Rules> variants = sqout_rule_variants(cfg.rules);
    int windows = 0;
    int paid_to_end = 0;
    int unpaid_past_leeway = 0;
    int unpaid_after_sqout = 0;
    int paid_in_leeway = 0;
    for (const CorpusChart& c : analyzed_corpus_charts(cfg)) {
        INFO(c.path);
        const Song& song = *c.song;
        const TablesByRule tables(song, variants);
        for (const Path& p : c.record->paths) {
            for (const ReplayWindow& w : windows_for_path(p)) {
                ++windows;
                for (const core::Rules& rules : variants) {
                    const ChordScoreTable& table = tables(rules);
                    const std::vector<ReplayWindow> one = {w};
                    const ReplayResult alone = replay_path(song, one, rules, scores_only());
                    check_rows_priced(song, one, rules, alone, table, rules_label(rules));
                    for (size_t i = 0; i < alone.chords.size(); ++i) {
                        const ReplayChord& row = alone.chords[i];
                        if (row.tick < w.act_tick) {
                            CHECK_FALSE(row.in_sp);
                            continue;
                        }
                        const double offset = offset_from_end(song, i, w);
                        const core::SqOutPosition pos = core::sqout_position(row.tick, w.sqout_tick);
                        if (pos == core::SqOutPosition::After) {
                            ++unpaid_after_sqout;
                            CHECK_FALSE(row.in_sp);
                        } else if (pos == core::SqOutPosition::Exact) {
                            // The squeezed-out chord keeps what the table says
                            // it keeps; check_rows_priced holds that.
                        } else if (core::paid_by_sp_walk(offset)) {
                            ++paid_to_end;
                            CHECK(row.in_sp);
                        } else if (offset >= rules.backend_leeway_ms) {
                            ++unpaid_past_leeway;
                            CHECK_FALSE(row.in_sp);
                        } else {
                            ++paid_in_leeway;
                            CHECK(row.in_sp);
                        }
                    }
                }
            }
        }
    }
    MESSAGE("windows " << windows << ", paid inside the leeway " << paid_in_leeway
                       << ", after a squeezed-out chord " << unpaid_after_sqout);
    // Every stored window pays the chord its activation starts on, which sits
    // before its SP end, and every window that is not the chart's last has
    // chords more than the leeway past its end.
    CHECK(windows > 0);
    CHECK(paid_to_end > 0);
    CHECK(unpaid_past_leeway > 0);
}

// A one-note chord squeezed out keeps none of its doubling under either
// squeeze-out rule (D2), so its price on the squeeze-out is 0 and Star Power
// does not mark it: the disc stays plain. Checked on every one-note chord of
// every corpus chart; the rows a stored path squeezes out are held to that
// price by the case above.
TEST_CASE("replay: a one-note chord squeezed out on D keeps none of its doubling") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::vector<core::Rules> variants = sqout_rule_variants(cfg.rules);
    int one_note = 0;
    for (const CorpusChart& c : analyzed_corpus_charts(cfg)) {
        INFO(c.path);
        const Song& song = *c.song;
        const TablesByRule tables(song, variants);
        for (const core::Rules& rules : variants) {
            INFO(rules_label(rules));
            const ChordScoreTable& table = tables(rules);
            for (size_t i = 0; i < song.sequence.size(); ++i) {
                if (song.sequence[i].chord.note_list(true).size() != 1) continue;
                ++one_note;
                const ChordScoreRow& sg = table.rows[i];
                // Outside a squeeze-out SP doubles the note; squeezed out on
                // D (offset 0) it pays nothing.
                CHECK(sg.sp > 0);
                CHECK(sg.sqout_sp == 0);
                CHECK_FALSE(core::paid_by_sp(0.0, sg.sp, sg.sqout_sp, core::SqOutPosition::Exact,
                                             rules.backend_leeway_ms));
            }
        }
    }
    // Every drum chart has single hits: a lone kick or snare is a one-note
    // chord.
    CHECK(one_note > 0);
}

// How RapidCheck prints a window in a counterexample.
namespace hydra {
void showValue(const ReplayWindow& w, std::ostream& os) {
    os << "{act " << w.act_tick << ", deact " << w.deact_tick;
    if (w.sqout_tick) os << ", sqout " << *w.sqout_tick;
    os << "}";
}
}  // namespace hydra

namespace {

// Do two windows replay alike? replay_path reads only these three fields.
bool same_window(const ReplayWindow& a, const ReplayWindow& b) {
    return a.act_tick == b.act_tick && a.deact_tick == b.deact_tick && a.sqout_tick == b.sqout_tick;
}

// One corpus chart's stored windows, every path's pooled once each, so a
// window list drawn from them overlaps the way alternate paths do.
struct WindowPool {
    std::string path;
    const Song* song = nullptr;
    std::vector<ReplayWindow> windows;
};

std::vector<WindowPool> window_pools(const std::vector<CorpusChart>& charts) {
    std::vector<WindowPool> out;
    for (const CorpusChart& c : charts) {
        WindowPool pool;
        pool.path = c.path;
        pool.song = c.song;
        for (const Path* p : c.record->all_paths()) {
            for (const ReplayWindow& w : windows_for_path(*p)) {
                bool have = false;
                for (const ReplayWindow& kept : pool.windows) have = have || same_window(kept, w);
                if (!have) pool.windows.push_back(w);
            }
        }
        if (!pool.windows.empty()) out.push_back(std::move(pool));
    }
    return out;
}

// The pool's windows `picks` names, in the order first named, each once.
std::vector<ReplayWindow> picked_windows(const WindowPool& pool, const std::vector<size_t>& picks) {
    std::vector<bool> taken(pool.windows.size(), false);
    std::vector<ReplayWindow> out;
    for (const size_t i : picks) {
        if (taken[i]) continue;
        taken[i] = true;
        out.push_back(pool.windows[i]);
    }
    return out;
}

}  // namespace

// The walk's two properties as a RapidCheck property, the trial of plan
// 2026-10-10-ci-test-tooling (task 7; removed, read it with git show
// ec91f8d1:docs/superpowers/plans/2026-10-10-ci-test-tooling.md). Each test
// draws a corpus chart, a squeeze-out rule and a list of the windows that
// chart's stored paths hold; listing them in reverse changes no row, and each
// row is what its windows' prices give. A failure is shrunk to the fewest
// windows that still break a check. The seed is fixed so the suite runs the
// same tests every time (the plan's recommendation on its question 7); a
// failure prints the RC_PARAMS line that replays it.
TEST_CASE("replay: window order does not matter and windows add up, shrunk by RapidCheck") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::vector<core::Rules> variants = sqout_rule_variants(cfg.rules);
    const std::vector<WindowPool> pools = window_pools(analyzed_corpus_charts(cfg));
    REQUIRE_FALSE(pools.empty());
    std::vector<TablesByRule> tables;
    tables.reserve(pools.size());
    for (const WindowPool& pool : pools) tables.emplace_back(*pool.song, variants);

    // Runs whose windows paid some chord, so the checks were not all on rows
    // no window reaches.
    int paying_runs = 0;
    const auto property = [&] {
        const size_t c = *rc::gen::inRange<size_t>(0, pools.size());
        const size_t v = *rc::gen::inRange<size_t>(0, variants.size());
        const WindowPool& pool = pools[c];
        const std::vector<size_t> picks = *rc::gen::container<std::vector<size_t>>(
            rc::gen::inRange<size_t>(0, pool.windows.size()));
        const std::vector<ReplayWindow> wl = picked_windows(pool, picks);
        const core::Rules& rules = variants[v];
        const ChordScoreTable& table = tables[c](rules);
        RC_LOG() << pool.path << ", " << rules_label(rules) << "\n";
        const ReplayResult r = replay_path(*pool.song, wl, rules, scores_only());
        if (r.final.sp > 0) ++paying_runs;
        const std::vector<ReplayWindow> reversed(wl.rbegin(), wl.rend());
        RC_ASSERT(first_result_difference(
                      r, replay_path(*pool.song, reversed, rules, scores_only())) == "");
        const BadRows off = rows_off_price(*pool.song, wl, rules, r, table);
        RC_LOG() << "first row its windows' prices do not give: " << off.first << "\n";
        RC_ASSERT(off.count == 0u);
    };

    // RapidCheck reads RC_PARAMS once, on its first check. When the caller
    // has not set it, the seed and the number of passing tests are fixed
    // here. The user chose fewer trials (docs/sessions/2026-10-25-495286b5.md);
    // 50 is the number the plan set for that.
    size_t env_len = 0;
    getenv_s(&env_len, nullptr, 0, "RC_PARAMS");
    if (env_len == 0) _putenv_s("RC_PARAMS", "seed=1 max_success=50");
    rc::detail::TestMetadata metadata;
    metadata.id = metadata.description = "replay window order and sums";
    const rc::detail::TestResult result = rc::detail::checkTestable(property, metadata);
    std::ostringstream message;
    rc::detail::printResultMessage(result, message);
    MESSAGE(message.str());
    CHECK(result.is<rc::detail::SuccessResult>());
    CHECK(paying_runs > 0);
}

// Every stored path on the corpus, root paths and alternates, under each
// squeeze-out rule: each row is what its windows' prices give (the sum, the
// payers, Star Power's mark and the disc), and listing the windows in reverse
// changes no row. (The totals themselves are held to the engine's by "replay
// reproduces the engine's score for every corpus path".)
TEST_CASE("replay: on every corpus path the disc follows SP's payment and windows add up") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::vector<core::Rules> variants = sqout_rule_variants(cfg.rules);
    int charts = 0;
    int runs = 0;
    for (const CorpusChart& c : analyzed_corpus_charts(cfg)) {
        ++charts;
        const Song& song = *c.song;
        const TablesByRule tables(song, variants);
        for (const Path* p : c.record->all_paths()) {
            ++runs;
            const std::vector<ReplayWindow> wl = windows_for_path(*p);
            const std::vector<ReplayWindow> reversed(wl.rbegin(), wl.rend());
            for (const core::Rules& rules : variants) {
                const std::string what = c.path + ", " + rules_label(rules);
                INFO(what);
                const ChordScoreTable& table = tables(rules);
                const ReplayResult r = replay_path(song, wl, rules, scores_only());
                check_rows_priced(song, wl, rules, r, table, what);
                CHECK(first_result_difference(
                          r, replay_path(song, reversed, rules, scores_only())) == "");
            }
        }
    }
    MESSAGE("charts " << charts << ", runs " << runs);
    CHECK(charts > 0);
    CHECK(runs > 0);
}
