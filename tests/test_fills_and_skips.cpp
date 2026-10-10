// Fills the search spawns, passes over or refuses: a later-ready path that an
// upcoming fill still spawns for, and the skipped fills each activation
// stores under the 1.0 and 1.1 deadline rules. The deadline math itself is
// pinned in test_fill_deadline.cpp.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"

using namespace hydra;

// Two paths with the same SP meter are not interchangeable when an upcoming
// fill spawns for one and refuses the other (Clone Hero's early-fill rule:
// SP must be ready by the fill's deadline). The later-ready path leads on
// score here; it may not knock out the earlier-ready one, whose activation at
// that fill is the best path.
TEST_CASE("search: a later-ready path never knocks out one an upcoming fill still spawns for") {
    Song song = test::ready_time_fold_song();
    ScoreGraph graph(song, 2);
    std::vector<Path> best;
    REQUIRE_NOTHROW(best = run_search(graph, EngineOptions{}));
    REQUIRE_FALSE(best.empty());

    EngineOptions target;
    target.target_act_ticks = std::vector<int64_t>{7680};
    std::vector<Path> late;
    REQUIRE_NOTHROW(late = run_search(graph, target));
    REQUIRE_FALSE(late.empty());

    // A, which activated at 2304, cannot spawn the fill at 7680.
    EngineOptions a_then_late;
    a_then_late.target_act_ticks = std::vector<int64_t>{2304, 7680};
    CHECK_THROWS(run_search(graph, a_then_late));

    CAPTURE(best.front().pathstring());
    CHECK(test::act_ticks(best.front()) == std::vector<int64_t>{7680});
    CHECK(best.front().totalscore() == late.front().totalscore());
}

TEST_CASE("Skipped fills: the 1.0 rule's offered fill is the one stored") {
    // Fill A (19200) was shown and passed over; fill B (24960) has the earlier
    // 1.0 deadline and never spawned. The nearest-fill guess would name B.
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.skipped_fill_ticks == std::vector<int64_t>{19200}));
}

// The 1.1 rule at the exact edge. A fill spawns when SP is ready no later
// than kEarlyFillWindowMs past its deadline (the engine refuses it only when
// e_offset < -kEarlyFillWindowMs). 125 BPM at 480 ticks a beat makes a tick
// one millisecond. Fill A ends at 9600 and is 480 ticks long, so its 1.1
// deadline is 4 beats before its start: tick 7200, 7200 ms. The first phrase
// (480) banks a bar; the second, at `ready`, banks the other. Fill C (19200)
// is the one taken. With SP ready at 7260 (exactly 60 ms late) A is shown and
// passed over; one tick later it never spawns.
TEST_CASE("Skipped fills: under the 1.1 rule a fill exactly at the window edge is stored") {
    auto passed_over = [](int64_t ready) {
        std::vector<test::FixtureNote> notes;
        for (int64_t t = 0; t <= 28800; t += 480) {
            test::FixtureNote n{t};
            n.phrase = t == 480;
            if (t == 9600 || t == 19200) n.fill_length = 480;
            notes.push_back(n);
        }
        notes.push_back({ready, true, 0});
        std::sort(notes.begin(), notes.end(),
                  [](const test::FixtureNote& a, const test::FixtureNote& b) {
                      return a.tick < b.tick;
                  });
        const Song song = test::build_fixture_song(480, 125.0, notes);
        // The arithmetic above, checked against the rule itself.
        REQUIRE(activation_fill_deadline_ms(song.timing(), 9600, 480, FillDeadlineRule::Ch11) ==
                7200.0);
        REQUIRE(song.timecode(ready).ms() == static_cast<double>(ready));
        ScoreGraph graph(song, 4, FillDeadlineRule::Ch11);
        EngineOptions opts;
        opts.target_act_ticks = std::vector<int64_t>{19200};
        const std::vector<Path> paths = run_search(graph, opts);
        REQUIRE(!paths.empty());
        REQUIRE(paths.front().activations.size() == 1);
        REQUIRE(paths.front().activations.front().timecode.ticks() == 19200);
        return paths.front().activations.front().skipped_fill_ticks;
    };
    const int64_t edge = 7200 + static_cast<int64_t>(kEarlyFillWindowMs);
    CHECK((passed_over(edge) == std::vector<int64_t>{9600}));
    CHECK(passed_over(edge + 1).empty());
}

// The lasting order checks, on root paths. A tied variant still carries its
// leader's list until finding 97 gets its own plan (Q3).
TEST_CASE("Skipped fills: every corpus root passes over real fills in order") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int skipped = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song =
            corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        for (const Path& root : corpus::analyzed(chart, cfg).paths) {
            const Activation* prev = nullptr;
            for (const Activation& act : root.walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                for (size_t k = 0; k < act.skipped_fill_ticks.size(); ++k) {
                    const int64_t t = act.skipped_fill_ticks[k];
                    ++skipped;
                    CHECK(t < act.timecode.ticks());
                    if (prev) CHECK(t > prev->timecode.ticks());
                    if (k > 0) CHECK(t > act.skipped_fill_ticks[k - 1]);
                    CHECK(std::any_of(song.sequence.begin(), song.sequence.end(),
                                      [t](const SongTimestamp& ts) {
                                          return ts.timecode.ticks() == t &&
                                                 ts.activation_length.has_value();
                                      }));
                }
                prev = &act;
            }
        }
    }
    CHECK(skipped > 100);  // a floor the user approved (D43)
}
