// The replay's open-window walk. The rest of the replay's invariants are in
// test_replay.cpp.
//
// replay_path keeps a short list of the windows that can still pay the
// current chord, rather than asking every window about every chord. The cases
// below pin its rows on hand-built windows. On many windows they hold it to
// two properties instead: the order the windows are listed in does not
// matter, and each chord's SP points are the sum of what each window pays it
// alone, since the SP rule prices one window at a time.

#include "doctest.h"

#include <rapidcheck.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "replay_util.h"
#include "replay_windows.h"
#include "store/record_store.h"

using namespace hydra;

// Shared with test_replay.cpp (replay_util.h): Round and Round's shape on a
// hand-built chart, and where two replays first differ, row by row.
using test::first_result_difference;
using test::squeeze_chart;

namespace {

using testreplay::synthetic_windows;

// Leeways that stress the exit rule: the real 3 ms, none, a negative one, and
// one wide enough that chords well past D keep counting.
std::vector<core::Rules> leeway_variants() {
    std::vector<core::Rules> out;
    for (double leeway : {3.0, 0.0, -5.0, 250.0}) {
        core::Rules r = core::default_rules();
        r.backend_leeway_ms = leeway;
        out.push_back(r);
    }
    core::Rules whole = core::default_rules();
    whole.sqout_rule = core::SqOutRule::WholeChord;
    out.push_back(whole);
    return out;
}

std::string rules_label(const core::Rules& r) {
    if (r.sqout_rule == core::SqOutRule::WholeChord) return "whole-chord squeeze-out";
    return "leeway " + std::to_string(static_cast<int>(r.backend_leeway_ms)) + " ms";
}

// One row as text, every field the windows can move plus where the chord
// sits: index, tick, position, combo before -> after, multiplier
// before / after, notes, combo and SP points, "SP" when Star Power paid the
// chord, the disc's multiplier and the running total.
std::string row_line(const ReplayChord& c) {
    char buf[200];
    std::snprintf(buf, sizeof buf,
                  "%d t%lld %s c%d->%d x%d/%d notes=%lld combo=%lld sp=%lld %s "
                  "shown=x%d cum=%lld",
                  c.index, static_cast<long long>(c.tick), c.position.c_str(), c.combo_before,
                  c.combo_after, c.multiplier, c.multiplier_after,
                  static_cast<long long>(c.points.notes), static_cast<long long>(c.points.combo),
                  static_cast<long long>(c.points.sp), c.in_sp ? "SP" : "--", c.multiplier_shown,
                  static_cast<long long>(c.cum.total()));
    return buf;
}

std::vector<std::string> row_lines(const ReplayResult& r) {
    std::vector<std::string> out;
    for (const ReplayChord& c : r.chords) out.push_back(row_line(c));
    return out;
}

// The rows in one line: each chord's SP points, which chords SP paid ("Y"),
// and each chord's disc multiplier.
std::string compact_line(const ReplayResult& r) {
    std::string sp = "sp", paid = " paid ", shown = " shown";
    for (const ReplayChord& c : r.chords) {
        sp += " " + std::to_string(c.points.sp);
        paid += c.in_sp ? "Y" : "-";
        shown += " x" + std::to_string(c.multiplier_shown);
    }
    return sp + paid + shown;
}

// Built lines against pinned ones, printed as literals on a mismatch
// (record_fixtures.h).
using test::check_lines;

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
// `variants`, built once so every one-window replay below reuses it instead of
// building its own.
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

// Each row's SP points must be the sum of what each window pays it alone,
// and SP pays the chord exactly when one of them does. Single-window replays
// are scores-only to keep this quick; their score fields are a full replay's.
// `table` is the chart's ChordsOnly table under rules.sqout_rule.
BadRows rows_not_adding_up(const Song& song, const std::vector<ReplayWindow>& wl,
                           const core::Rules& rules, const ReplayResult& all,
                           const ChordScoreTable& table) {
    ReplayOptions scores;
    scores.scores_only = true;
    scores.table = &table;
    std::vector<int64_t> sum(all.chords.size(), 0);
    std::vector<bool> paid(all.chords.size(), false);
    for (const ReplayWindow& w : wl) {
        const ReplayResult one = replay_path(song, {w}, rules, scores);
        if (one.chords.size() != all.chords.size())
            throw std::logic_error("a one-window replay has another row count");
        for (size_t i = 0; i < one.chords.size(); ++i) {
            sum[i] += one.chords[i].points.sp;
            if (one.chords[i].in_sp) paid[i] = true;
        }
    }
    BadRows bad;
    for (size_t i = 0; i < all.chords.size(); ++i)
        if (all.chords[i].points.sp != sum[i] || all.chords[i].in_sp != paid[i]) bad.add(i);
    return bad;
}

void check_windows_add_up(const Song& song, const std::vector<ReplayWindow>& wl,
                          const core::Rules& rules, const ReplayResult& all,
                          const ChordScoreTable& table, const std::string& what) {
    const BadRows bad = rows_not_adding_up(song, wl, rules, all, table);
    INFO(what << ": first row that does not add up: " << bad.first);
    CHECK(bad.count == 0);
}

// What every row of any replay holds: SP paid the chord exactly when it paid
// it points (D2), and the disc doubles exactly then.
BadRows rows_off_payment(const ReplayResult& r) {
    BadRows bad;
    for (size_t i = 0; i < r.chords.size(); ++i) {
        const ReplayChord& c = r.chords[i];
        if (c.in_sp != (c.points.sp > 0) ||
            c.multiplier_shown != shown_multiplier(c.multiplier_after, c.in_sp))
            bad.add(i);
    }
    return bad;
}

void check_disc_follows_payment(const ReplayResult& r, const std::string& what) {
    const BadRows bad = rows_off_payment(r);
    INFO(what << ": first row whose disc does not follow SP's payment: " << bad.first);
    CHECK(bad.count == 0);
}

// The corpus chart with the most chords, and its path.
const Song& longest_corpus_song(std::string& path_out) {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song* longest = nullptr;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (!longest || song.sequence.size() > longest->sequence.size()) {
            longest = &song;
            path_out = path;
        }
    }
    if (!longest) throw std::logic_error("the chart corpus is empty");
    return *longest;
}

}  // namespace

// The open-window walk on five hand-built windows, alone and together, under
// every leeway variant. "late" squeezes out the phrase chord 479 ms past its
// SP end. "early" squeezes out the chord at 2304, before its SP end. "plain"
// ends at tick 3100, 28 ticks (72.9 ms) after the chord at 3072. "near" ends
// at tick 3000 (7812.5 ms), so the chord at 3072 is 187.5 ms past it. "tick"
// ends one tick (2.6 ms) before the chord at 3072.
TEST_CASE("replay: the open-window walk pins every row on hand-built windows") {
    const Song song = squeeze_chart();
    const ReplayWindow late = window(0, 3072, 3256);
    const ReplayWindow early = window(768, 3072, 2304);
    const ReplayWindow plain = window(1536, 3100);
    const ReplayWindow near_w = window(0, 3000);
    const ReplayWindow tick = window(1536, 3071);
    const std::vector<ReplayWindow> all = {late, early, plain, near_w, tick};
    const std::vector<ReplayWindow> reversed = {tick, near_w, plain, early, late};

    // The default rules, every row in full. By hand: each chord is two notes
    // at 50 points (notes 100); the multiplier steps to x2 at the tenth note,
    // the second note of chord 4 (combo 50). SP doubles what a chord scores,
    // so a window pays chords 0-3 100 points and chord 4 150. A chord at or
    // before D always counts; one after it counts only inside the 3 ms
    // leeway. So the late window pays chords 0-4 and not the phrase chord
    // 479 ms past D. The early window pays 1 and 2, on its squeezed-out
    // chord 3 only the second note (50), and nothing after it. The plain
    // window pays 2-4 and not chord 5, 406 ms past D. The near window pays
    // 0-3: chord 4 is 187.5 ms past D. The tick window pays 2-4: chord 4 is
    // 2.6 ms past D, inside the leeway.
    const core::Rules& rules = core::default_rules();
    check_lines(row_lines(replay_path(song, {late}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=200",
                 "1 t768 m2.1.0 c2->4 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "2 t1536 m3.1.0 c4->6 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "3 t2304 m4.1.0 c6->8 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=800",
                 "4 t3072 m5.1.0 c8->10 x1/2 notes=100 combo=50 sp=150 SP shown=x4 cum=1100",
                 "5 t3256 m5.1.184 c10->12 x2/2 notes=100 combo=100 sp=0 -- shown=x2 cum=1300"},
                "late");
    check_lines(row_lines(replay_path(song, {early}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 notes=100 combo=0 sp=0 -- shown=x1 cum=100",
                 "1 t768 m2.1.0 c2->4 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=300",
                 "2 t1536 m3.1.0 c4->6 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=500",
                 "3 t2304 m4.1.0 c6->8 x1/1 notes=100 combo=0 sp=50 SP shown=x2 cum=650",
                 "4 t3072 m5.1.0 c8->10 x1/2 notes=100 combo=50 sp=0 -- shown=x2 cum=800",
                 "5 t3256 m5.1.184 c10->12 x2/2 notes=100 combo=100 sp=0 -- shown=x2 cum=1000"},
                "early");
    check_lines(row_lines(replay_path(song, {plain}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 notes=100 combo=0 sp=0 -- shown=x1 cum=100",
                 "1 t768 m2.1.0 c2->4 x1/1 notes=100 combo=0 sp=0 -- shown=x1 cum=200",
                 "2 t1536 m3.1.0 c4->6 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "3 t2304 m4.1.0 c6->8 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "4 t3072 m5.1.0 c8->10 x1/2 notes=100 combo=50 sp=150 SP shown=x4 cum=900",
                 "5 t3256 m5.1.184 c10->12 x2/2 notes=100 combo=100 sp=0 -- shown=x2 cum=1100"},
                "plain");
    check_lines(row_lines(replay_path(song, {near_w}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=200",
                 "1 t768 m2.1.0 c2->4 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "2 t1536 m3.1.0 c4->6 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "3 t2304 m4.1.0 c6->8 x1/1 notes=100 combo=0 sp=100 SP shown=x2 cum=800",
                 "4 t3072 m5.1.0 c8->10 x1/2 notes=100 combo=50 sp=0 -- shown=x2 cum=950",
                 "5 t3256 m5.1.184 c10->12 x2/2 notes=100 combo=100 sp=0 -- shown=x2 cum=1150"},
                "near");
    // Together each chord's SP points are the five windows' sum: chord 2 is
    // 100 from four windows, chord 4 150 from late, plain and tick.
    const std::vector<std::string> all_rows = {
        "0 t0 m1.1.0 c0->2 x1/1 notes=100 combo=0 sp=200 SP shown=x2 cum=300",
        "1 t768 m2.1.0 c2->4 x1/1 notes=100 combo=0 sp=300 SP shown=x2 cum=700",
        "2 t1536 m3.1.0 c4->6 x1/1 notes=100 combo=0 sp=500 SP shown=x2 cum=1300",
        "3 t2304 m4.1.0 c6->8 x1/1 notes=100 combo=0 sp=450 SP shown=x2 cum=1850",
        "4 t3072 m5.1.0 c8->10 x1/2 notes=100 combo=50 sp=450 SP shown=x4 cum=2450",
        "5 t3256 m5.1.184 c10->12 x2/2 notes=100 combo=100 sp=0 -- shown=x2 cum=2650"};
    check_lines(row_lines(replay_path(song, all, rules)), all_rows, "all five");
    check_lines(row_lines(replay_path(song, reversed, rules)), all_rows,
                "all five, listed in reverse");

    // Every leeway variant, one line per window list: late, early, plain,
    // near, tick, all five. With no leeway (0 or -5 ms) the tick window stops
    // paying chord 4. With 250 ms the near window pays it too. Under the
    // whole-chord rule the early window's squeezed-out chord earns nothing.
    const std::vector<std::vector<std::string>> want = {
        {"sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 100 100 50 0 0 paid -YYY-- shown x1 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 100 100 100 100 0 0 paid YYYY-- shown x2 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 200 300 500 450 450 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2"},
        {"sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 100 100 50 0 0 paid -YYY-- shown x1 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 100 100 100 100 0 0 paid YYYY-- shown x2 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 0 0 paid --YY-- shown x1 x1 x2 x2 x2 x2",
         "sp 200 300 500 450 300 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2"},
        {"sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 100 100 50 0 0 paid -YYY-- shown x1 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 100 100 100 100 0 0 paid YYYY-- shown x2 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 0 0 paid --YY-- shown x1 x1 x2 x2 x2 x2",
         "sp 200 300 500 450 300 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2"},
        {"sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 100 100 50 0 0 paid -YYY-- shown x1 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 200 300 500 450 600 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2"},
        {"sp 100 100 100 100 150 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2",
         "sp 0 100 100 0 0 0 paid -YY--- shown x1 x2 x2 x1 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 100 100 100 100 0 0 paid YYYY-- shown x2 x2 x2 x2 x2 x2",
         "sp 0 0 100 100 150 0 paid --YYY- shown x1 x1 x2 x2 x4 x2",
         "sp 200 300 500 400 450 0 paid YYYYY- shown x2 x2 x2 x2 x4 x2"}};
    const std::vector<core::Rules> variants = leeway_variants();
    REQUIRE(variants.size() == want.size());
    for (size_t v = 0; v < variants.size(); ++v) {
        const core::Rules& r = variants[v];
        std::vector<std::string> got;
        for (const std::vector<ReplayWindow>& wl : std::vector<std::vector<ReplayWindow>>{
                 {late}, {early}, {plain}, {near_w}, {tick}, all})
            got.push_back(compact_line(replay_path(song, wl, r)));
        CHECK(compact_line(replay_path(song, reversed, r)) == got.back());
        check_lines(got, want[v], rules_label(r));
    }
}

// Two windows that never overlap, each ending on a one-note chord it squeezes
// out on its deactivation node. SP pays that chord nothing, so under D2 its
// disc stays plain, while the chords inside each window are paid. (The old
// rule, which counted any window that reached the chord, doubled it.)
TEST_CASE("replay: disjoint windows that squeeze out a one-note chord on D") {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3840, 4608}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    const std::vector<ReplayWindow> wl = {window(0, 1536, 1536), window(2304, 3840, 3840)};
    std::vector<std::string> got;
    const std::vector<core::Rules> variants = leeway_variants();
    const TablesByRule tables(song, variants);
    for (const core::Rules& r : variants) {
        const ReplayResult res = replay_path(song, wl, r);
        got.push_back(rules_label(r) + ": " + compact_line(res));
        check_windows_add_up(song, wl, r, res, tables(r), rules_label(r));
    }
    // By hand: one red note is 50 points, and SP doubles it. Chords 0 and 1
    // are paid by the first window, 3 and 4 by the second. Chords 2 and 5 sit
    // on their window's D (offset 0, so every leeway counts them) and are
    // squeezed out: a one-note chord keeps nothing of its doubling under
    // either rule, so SP pays 0 and the disc stays x1. Chord 6 is after both.
    check_lines(got,
                {"leeway 3 ms: sp 50 50 0 50 50 0 0 paid YY-YY-- shown x2 x2 x1 x2 x2 x1 x1",
                 "leeway 0 ms: sp 50 50 0 50 50 0 0 paid YY-YY-- shown x2 x2 x1 x2 x2 x1 x1",
                 "leeway -5 ms: sp 50 50 0 50 50 0 0 paid YY-YY-- shown x2 x2 x1 x2 x2 x1 x1",
                 "leeway 250 ms: sp 50 50 0 50 50 0 0 paid YY-YY-- shown x2 x2 x1 x2 x2 x1 x1",
                 "whole-chord squeeze-out: sp 50 50 0 50 50 0 0 paid YY-YY-- shown x2 x2 x1 x2 x2 "
                 "x1 x1"},
                "disjoint squeeze-outs");
}

// Many windows on the longest corpus chart: overlapping freely, deactivation
// nodes a few ticks off a chord, squeeze-outs before and after D. Listing
// them in another order gives the same rows, and each row's SP points are
// what the windows pay it one at a time.
TEST_CASE("replay: window order does not matter and SP points add up window by window") {
    std::string longest_path;
    const Song* longest = &longest_corpus_song(longest_path);
    REQUIRE(longest->sequence.size() > 500);
    INFO("chart: " << longest_path << " (" << longest->sequence.size() << " chords)");

    const std::vector<ReplayWindow> wl = synthetic_windows(*longest, 2000, 1);
    std::vector<ReplayWindow> reversed(wl.rbegin(), wl.rend());
    // The first 120 windows for the window-by-window sum: 120 replays each.
    // All 2,000 took 5.6 s against 0.5 s for 120 (2026-10-04), so 120 is a
    // test limit, like the others in D43.
    const std::vector<ReplayWindow> few(wl.begin(), wl.begin() + 120);
    const std::vector<core::Rules> variants = leeway_variants();
    const TablesByRule tables(*longest, variants);
    for (const core::Rules& rules : variants) {
        const std::string what = rules_label(rules);
        const ReplayResult r = replay_path(*longest, wl, rules);
        INFO(what);
        CHECK(first_result_difference(r, replay_path(*longest, reversed, rules)).empty());
        // The windows must actually pay something, or this proves nothing.
        CHECK(r.final.sp > 0);
        check_disc_follows_payment(r, what);
        check_windows_add_up(*longest, few, rules, replay_path(*longest, few, rules),
                             tables(rules), what);
    }
}

// How RapidCheck prints a window in a counterexample.
namespace hydra {
void showValue(const ReplayWindow& w, std::ostream& os) {
    os << "{act " << w.act_tick << ", deact " << w.deact_tick;
    if (w.sqout_tick) os << ", sqout " << *w.sqout_tick;
    os << "}";
}
}  // namespace hydra

// The case above as a RapidCheck property, the trial of plan
// 2026-10-10-ci-test-tooling (task 7; removed, read it with git show
// ec91f8d1:docs/superpowers/plans/2026-10-10-ci-test-tooling.md). The same three checks run on made-up
// windows over the same chart, one leeway variant per test, and a failure is
// shrunk to the fewest, smallest windows that still break a check. The seed
// is fixed so the suite runs the same tests every time (decision 7); a
// failure prints the RC_PARAMS line that replays it.
TEST_CASE("replay: window order does not matter and windows add up, shrunk by RapidCheck") {
    std::string longest_path;
    const Song& song = longest_corpus_song(longest_path);
    const size_t n = song.sequence.size();
    REQUIRE(n > 500);
    INFO("chart: " << longest_path << " (" << n << " chords)");
    const std::vector<core::Rules> variants = leeway_variants();
    const TablesByRule tables(song, variants);

    // One window: the same draws synthetic_windows makes from its Lcg, with
    // the same odds (all in tests/replay_windows.h), so RapidCheck can shrink
    // them. testreplay::window_over turns the draws into the window.
    using testreplay::kDeactOffsetOneIn;
    using testreplay::kDeactOffsetRange;
    using testreplay::kMaxWindowChords;
    using testreplay::kSqoutOneIn;
    using testreplay::kSqoutPicks;
    const rc::Gen<ReplayWindow> one_window = rc::gen::map(
        rc::gen::tuple(
            rc::gen::inRange<size_t>(0, n), rc::gen::inRange<size_t>(1, kMaxWindowChords + 1),
            rc::gen::weightedOneOf<int64_t>(
                {{kDeactOffsetOneIn - 1, rc::gen::just<int64_t>(0)},
                 {1, rc::gen::inRange<int64_t>(0, kDeactOffsetRange)}}),
            rc::gen::weightedOneOf<int>(
                {{kSqoutOneIn - 1, rc::gen::just(-1)},
                 {1, rc::gen::inRange(0, static_cast<int>(kSqoutPicks))}})),
        [&song](const std::tuple<size_t, size_t, int64_t, int>& t) {
            return testreplay::window_over(song, std::get<0>(t), std::get<1>(t), std::get<2>(t),
                                           std::get<3>(t));
        });

    const auto property = [&] {
        const size_t v = *rc::gen::inRange<size_t>(0, variants.size());
        const std::vector<ReplayWindow> wl =
            *rc::gen::container<std::vector<ReplayWindow>>(one_window);
        const core::Rules& rules = variants[v];
        RC_LOG() << rules_label(rules) << "\n";
        const ReplayResult r = replay_path(song, wl, rules);
        const std::vector<ReplayWindow> reversed(wl.rbegin(), wl.rend());
        RC_ASSERT(first_result_difference(r, replay_path(song, reversed, rules)) == "");
        const BadRows disc = rows_off_payment(r);
        RC_LOG() << "first row whose disc does not follow SP's payment: " << disc.first << "\n";
        RC_ASSERT(disc.count == 0u);
        const BadRows sum = rows_not_adding_up(song, wl, rules, r, tables(rules));
        RC_LOG() << "first row that does not add up: " << sum.first << "\n";
        RC_ASSERT(sum.count == 0u);
    };

    // RapidCheck reads RC_PARAMS once, on its first check. When the caller
    // has not set it, the seed and the number of passing tests are fixed
    // here; 50 passes is the user's choice (plan 2026-10-25-asan-slow-cases,
    // Question 2).
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
}

// Every stored path on the corpus: the disc follows what SP paid on every
// row, and on each root path each row's SP points are the sum of what its
// windows pay one at a time. (The totals themselves are held to the engine's
// by "replay reproduces the engine's score for every corpus path".)
TEST_CASE("replay: on every corpus path the disc follows SP's payment and windows add up") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0, runs = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            ++runs;
            const std::vector<ReplayWindow> wl = windows_for_path(*p);
            const ReplayResult r = replay_path(song, wl, cfg.rules);
            check_disc_follows_payment(r, path);
        }
        const ChordScoreTable table = chord_score_table(song, cfg.rules.sqout_rule);
        for (const Path& p : rec.paths) {
            const std::vector<ReplayWindow> wl = windows_for_path(p);
            check_windows_add_up(song, wl, cfg.rules, replay_path(song, wl, cfg.rules), table,
                                 path);
        }
    }
    MESSAGE("charts " << charts << ", runs " << runs);
    CHECK(charts > 0);
    CHECK(runs > charts);
}

namespace {

// The squeeze chart under two windows, and the first corpus chart's first
// stored path, each under the default and the whole-chord squeeze-out rule.
struct TableCase {
    std::string what;
    const Song* song;
    std::vector<ReplayWindow> windows;
};

std::vector<TableCase> table_cases(const Song& squeeze) {
    std::vector<TableCase> out;
    out.push_back({"squeeze chart", &squeeze, {window(0, 3072, 3256), window(1536, 3100)}});
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        if (rec.paths.empty()) continue;
        out.push_back({path, &song, windows_for_path(rec.paths.front())});
        break;
    }
    if (out.size() != 2) throw std::logic_error("no corpus chart with a stored path");
    return out;
}

}  // namespace

TEST_CASE("replay_path with a table handed in equals replay_path building its own") {
    const Song squeeze = squeeze_chart();
    core::Rules whole = core::default_rules();
    whole.sqout_rule = core::SqOutRule::WholeChord;
    for (const TableCase& c : table_cases(squeeze)) {
        for (const core::Rules& rules : {core::default_rules(), whole}) {
            for (bool scores_only : {false, true}) {
                const std::string what = c.what + ", " + rules_label(rules) +
                                         (scores_only ? ", scores only" : ", full");
                INFO(what);
                ReplayOptions own;
                own.scores_only = scores_only;
                const ChordScoreTable table = chord_score_table(
                    *c.song, rules.sqout_rule,
                    scores_only ? ChordScoreDetail::ChordsOnly : ChordScoreDetail::WithNotes);
                ReplayOptions handed = own;
                handed.table = &table;
                const ReplayResult want = replay_path(*c.song, c.windows, rules, own);
                const ReplayResult got = replay_path(*c.song, c.windows, rules, handed);
                CHECK(want.final.sp > 0);
                CHECK(first_result_difference(want, got) == "");
            }
        }
    }
}

TEST_CASE("replay_path refuses a table of the wrong size or detail") {
    const Song squeeze = squeeze_chart();
    const std::vector<TableCase> cases = table_cases(squeeze);
    const Song& other = *cases.back().song;
    REQUIRE(other.sequence.size() != squeeze.sequence.size());
    const core::Rules& rules = core::default_rules();
    const std::vector<ReplayWindow> wl = cases.front().windows;

    ReplayOptions scores;
    scores.scores_only = true;
    const ChordScoreTable wrong_size = chord_score_table(other, rules.sqout_rule);
    scores.table = &wrong_size;
    CHECK_THROWS_AS(replay_path(squeeze, wl, rules, scores), std::logic_error);

    const ChordScoreTable with_notes =
        chord_score_table(squeeze, rules.sqout_rule, ChordScoreDetail::WithNotes);
    scores.table = &with_notes;
    CHECK_THROWS_AS(replay_path(squeeze, wl, rules, scores), std::logic_error);

    // And the other way round: a full replay handed a table without notes.
    ReplayOptions full;
    const ChordScoreTable chords_only = chord_score_table(squeeze, rules.sqout_rule);
    full.table = &chords_only;
    CHECK_THROWS_AS(replay_path(squeeze, wl, rules, full), std::logic_error);
}
