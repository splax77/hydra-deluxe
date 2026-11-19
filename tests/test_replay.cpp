// Replay invariants: core/replay.h must price a path exactly as the engine
// priced it.
//
// The engine sums scores along graph edges, priced when the graph is built;
// the replay walks chords and never looks at the graph. Both read their chord
// points from chord_score_table (core/scoring.h) and their doubling from
// core::backend_row_value (core/backend_value.h). If the two agree on all
// six score categories, for every path of every corpus chart, then the
// replay's one own question, which windows can pay a chord, agrees with the
// graph's. How the replay walks its open windows is in
// test_replay_windows.cpp.

#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "chart_text.h"
#include "core/backend_value.h"
#include "core/model.h"
#include "core/replay.h"
#include "replay_json.h"
#include "replay_util.h"
#include "core/scoring.h"
#include "core/sqout_chord.h"
#include "core/squeeze_rating.h"  // frontend_transfer_scales, the copy-out's stamp
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "core/model.h"  // within_squeeze_window, the horizon the warning uses
#include "search/pather.h"
#include "song_digest.h"  // loading_chart_count, the load the digest counts with
#include "store/record_store.h"

using namespace hydra;
using json = nlohmann::json;

// Shared with test_replay_windows.cpp (replay_util.h): where two replays
// first differ, row by row.
using test::first_result_difference;

namespace {

// What a walk over every stored corpus path found.
struct CorpusPathRun {
    int charts = 0, paths = 0, mismatches = 0;
    int no_part = 0;         // charts with no notes at these settings, skipped
    std::string first_diff;  // the first mismatch, named by chart and path
};

// Walks every stored path of every corpus chart `cfg` analyzes. `check` is
// called with the chart's song and one stored path; it returns "" when the
// path passes, or what differs. A chart with no notes at these settings
// (require_notes) is counted in no_part; any other load failure throws.
template <class Check>
CorpusPathRun check_every_corpus_path(const app::AnalysisSettings& cfg, Check check) {
    CorpusPathRun run;
    for (const std::string& path : corpus::chart_paths()) {
        // corpus::song parses under kTestFillRule only, so under another fill
        // rule the chart is loaded and analysed the app's way instead.
        std::optional<app::AnalysisResult> own;
        if (fill_rule_for(cfg.legacy_fills) != kTestFillRule) {
            try {
                own.emplace(app::analyze_chart_file(path, cfg));
            } catch (const NoNotesError&) {
                ++run.no_part;
                continue;
            }
        } else if (corpus::song(path, cfg.reading()).is_empty()) {
            ++run.no_part;
            continue;
        }
        ++run.charts;

        const Song& song = own ? own->song : corpus::song(path, cfg.reading());
        const HydraRecord& rec = own ? own->record : corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            ++run.paths;
            const std::string diff = check(song, *p);
            if (diff.empty()) continue;
            ++run.mismatches;
            if (run.first_diff.empty()) run.first_diff = path + " [" + p->pathstring() + "]" + diff;
        }
    }
    return run;
}

}  // namespace

TEST_CASE("replay reproduces the engine's score for every corpus path") {
    // Each setting starts from the GUI's defaults, straight from app::Settings
    // rather than hand-written literals, and changes one thing.
    struct Case {
        std::string name;
        app::Settings settings;
    };
    app::Settings legacy;
    legacy.legacy_fills = true;
    app::Settings hard;
    hard.view_difficulty = "Hard";
    const Case cases[] = {
        {"defaults", app::Settings()},
        {"CH 1.0 fill rule", legacy},
        {"Hard", hard},
    };

    for (const Case& c : cases) {
        CAPTURE(c.name);
        const app::AnalysisSettings cfg = c.settings.to_analysis_settings();
        const CorpusPathRun run = check_every_corpus_path(
            cfg, [](const Song& song, const Path& p) -> std::string {
                const PathReplay pr = replay_stored_path(song, p);
                REQUIRE(pr.all_windows());
                if (pr.totals_match()) return "";
                return ": replay " + std::to_string(pr.result.final.total()) + " vs stored " +
                       std::to_string(p.totalscore());
            });

        MESSAGE(c.name << ": " << run.charts << " charts, " << run.no_part << " with no part, "
                       << run.paths << " paths");
        // Every chart that loads at these settings was walked.
        const size_t loading = digest::loading_chart_count(corpus::chart_paths(), cfg);
        CHECK(run.charts > 0);
        CHECK(static_cast<size_t>(run.charts) == loading);
        // A walk that priced no path at a setting checked nothing.
        CHECK(run.paths > 0);
        INFO("first mismatch: " << run.first_diff);
        CHECK(run.mismatches == 0);
    }
}

// The combo the game's counter shows once a chord is hit. The Preview's score
// box reads it straight off the replay rather than adding note counts itself.
TEST_CASE("replay: combo_after is the combo once the chord is hit") {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    const std::vector<std::vector<NoteColor>> chords = {
        {NoteColor::Red},
        {NoteColor::Red, NoteColor::Yellow},
        {NoteColor::Kick, NoteColor::Blue, NoteColor::Green}};
    int64_t tick = 0;
    for (const std::vector<NoteColor>& colors : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        for (NoteColor c : colors) ts.chord.add_note(c);
        song.sequence.push_back(std::move(ts));
        tick += 480;
    }

    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords.size() == 3);
    CHECK(r.chords[0].combo_before == 0);
    CHECK(r.chords[0].combo_after == 1);
    CHECK(r.chords[1].combo_before == 1);
    CHECK(r.chords[1].combo_after == 3);
    CHECK(r.chords[2].combo_before == 3);
    CHECK(r.chords[2].combo_after == 6);
}

// The on-screen total holds a solo's bonus back until the section's last
// chord. The chart is kTwoSolosDrums (tests/chart_text.h).
TEST_CASE("replay: a solo's bonus reaches the on-screen total on the section's last chord") {
    const Song song =
        load_songbytes_chart(testchart::chart_bytes(testchart::kTwoSolosDrums), ChartReading{});
    REQUIRE(song.solo_sections.size() == 2);
    REQUIRE(song.solo_sections[0].first == 0);
    REQUIRE(song.solo_sections[0].last == 1);
    REQUIRE(song.solo_sections[1].first == 3);
    REQUIRE(song.solo_sections[1].last == 3);

    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords.size() == 4);
    // The screen never runs ahead of the real total. A chord outside every
    // section shows the whole total, so does a section's last chord (the
    // bonus is paid there), and a chord inside a section, short of its last,
    // that earned a solo bonus shows less. The sections are the song's own.
    bool held_somewhere = false;
    for (size_t i = 0; i < r.chords.size(); ++i) {
        CAPTURE(i);
        const int64_t total = r.chords[i].cum.total();
        const int64_t shown = r.chords[i].cum_onscreen_total;
        CHECK(shown <= total);
        bool inside = false, ends_section = false;
        for (const SoloSection& s : song.solo_sections) {
            if (i >= s.first && i < s.last) inside = true;
            if (i == s.last) ends_section = true;
        }
        if (!inside) CHECK(shown == total);
        if (ends_section) CHECK(shown == total);
        if (inside && r.chords[i].points.solo > 0) {
            CHECK(shown < total);
            held_somewhere = true;
        }
    }
    // The first section spans two chords and its first chord pays a solo
    // bonus, so at least one chord must hold something back.
    CHECK(held_somewhere);
}

// A targeted search is only useful if it gives back the same path the ordinary
// search would have found. So take every path the ordinary search DID find,
// hand its activation ticks back to search_target, and require the engine to
// price it identically -- same total, same deactivation node per activation,
// same squeezes. That is the whole contract: "activate exactly here" must not
// change how the engine scores what happens next.
TEST_CASE("targeted search reproduces every corpus path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    const CorpusPathRun run = check_every_corpus_path(
        cfg, [&cfg](const Song& song, const Path& p) -> std::string {
            const std::vector<Activation> want_acts = p.all_activations();

            const std::vector<Path> got = search_target(song, cfg, test::act_ticks(p));

            // Somewhere in the returned variants must be this exact path.
            const Path* match = nullptr;
            const std::vector<const Path*> targeted = flatten_paths(got);
            for (const Path* q : targeted) {
                if (q->totalscore() != p.totalscore()) continue;
                const std::vector<Activation> qa = q->all_activations();
                if (qa.size() != want_acts.size()) continue;
                bool same = true;
                for (size_t i = 0; i < qa.size() && same; ++i) {
                    if (qa[i].deact_tick() != want_acts[i].deact_tick()) same = false;
                    if (qa[i].sqinouts.size() != want_acts[i].sqinouts.size())
                        same = false;
                    for (size_t k = 0; k < qa[i].sqinouts.size() && same; ++k) {
                        if (qa[i].sqinouts[k].kind != want_acts[i].sqinouts[k].kind ||
                            qa[i].sqinouts[k].offset_ms !=
                                want_acts[i].sqinouts[k].offset_ms)
                            same = false;
                    }
                }
                if (same) { match = q; break; }
            }

            if (!match)
                return " score " + std::to_string(p.totalscore()) + ": " +
                       std::to_string(targeted.size()) + " targeted path(s), none matching";

            // The recovered path also has to replay to its own score, which is
            // the invariant the first test pins for search-found paths.
            CHECK(replay_stored_path(song, *match).totals_match());
            return "";
        });

    CHECK(run.charts > 0);
    // A walk that priced no path checked nothing.
    REQUIRE(run.paths > 0);
    INFO("first mismatch: " << run.first_diff);
    CHECK(run.mismatches == 0);
}

namespace {

struct TiedVariantCount {
    int variants = 0;  // tied variants the analysis listed
    int compared = 0;  // of those, the ones a lone search could price
    int differing = 0; // of those, the ones whose stored facts differ
    int no_part = 0;   // charts with no notes at these settings
    // Each variant with no lone root to compare against, named by chart and
    // path, with what the lone search gave instead.
    std::vector<std::string> skipped;
};

// Decision D3 for one set of analysis settings, over the whole corpus. Each
// variant is priced alone with a targeted search, and its stored facts must
// equal a lone root's (test::lone_pricing in record_fixtures.h: the score
// and every window fact, early-fill facts included). All-zero variants are
// not visited.
//
// A variant the lone search folded under a root of its own has no oracle.
// It is not compared; it goes on the skipped list by name, with its reason.
TiedVariantCount check_tied_variants(const app::AnalysisSettings& cfg) {
    TiedVariantCount n;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading(), cfg.rules);
        if (song.is_empty()) {
            ++n.no_part;
            continue;
        }
        const HydraRecord& rec = corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            if (!p->var_point) continue;
            ++n.variants;
            const test::LonePricing lone = test::lone_pricing(song, cfg, *p);
            if (!lone.diff.empty() && lone.tied_under_root) {
                n.skipped.push_back(path + " [" + p->pathstring() +
                                    "]: folded under a lone root of its own, " + lone.diff);
                continue;
            }
            ++n.compared;
            if (lone.diff.empty()) continue;
            ++n.differing;
            CHECK_MESSAGE(false, path << ": " << lone.diff);
        }
    }
    return n;
}

}  // namespace

// Decision D3: a tied variant is stored as a branch of its leader, but its
// facts are its own. The fixtures in test_tied_variants.cpp prove the
// mechanism on purpose-built charts; this guard catches any later case they do
// not shape.
//
// The app's defaults (kCloneHeroSpCap, kDefaultDepthValue) hold few ties, and
// none of them folded while SP ran or banked bars at different ticks from its
// leader. So the guard also runs at score range 40, at cap 4 and at cap 2.
// Those two hold hundreds of variants, and with either D3 fix taken out of the
// engine dozens of them store their leader's facts instead of their own.
TEST_CASE("every tied variant stores what a search pricing it alone stores") {
    // A variant with no lone root to compare against is printed by name,
    // with the reason, so the run shows each one rather than a count.
    struct Setting { int cap; int depth; };
    for (const Setting s : {Setting{4, 4}, Setting{4, 40}, Setting{2, 40}}) {
        app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
        cfg.sp_cap = s.cap;
        cfg.depth_value = s.depth;
        INFO("cap " << s.cap << ", score range " << s.depth);
        const TiedVariantCount n = check_tied_variants(cfg);
        for (const std::string& why : n.skipped) MESSAGE("skipped " << why);
        // A walk that compared no variant checked nothing at this setting.
        CHECK(n.compared > 0);
        CHECK(n.differing == 0);
        MESSAGE("cap " << s.cap << ", score range " << s.depth << ": compared " << n.compared
                       << " of " << n.variants << " variants against a lone search, "
                       << n.differing << " differ, " << n.skipped.size() << " skipped, "
                       << n.no_part << " charts with no notes at these settings");
    }
}

// A tick that is not an activation fill cannot be honoured, and the engine says
// so by giving back nothing rather than quietly pricing a different path.
TEST_CASE("targeted search rejects a tick that is not a fill") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    // Every corpus chart whose best path activates. The non-fill tick is the
    // chart's own: the nearest chord before the first activation whose
    // SongTimestamp carries no fill (has_activation() false), so Star Power
    // has had the most time to fill by then.
    int checked = 0, no_part = 0, no_activation = 0;
    for (const std::string& path : corpus::chart_paths()) {
        CAPTURE(path);
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) {
            ++no_part;
            continue;
        }
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        REQUIRE_MESSAGE(!rec.paths.empty(), "a chart with notes stored no path");
        const std::vector<Activation> acts = rec.best_path().all_activations();
        if (acts.empty()) {
            ++no_activation;
            continue;
        }

        const int64_t act_tick = acts[0].timecode.ticks();
        std::optional<int64_t> not_a_fill;
        for (const SongTimestamp& ts : song.sequence) {
            if (ts.timecode.ticks() >= act_tick) break;
            if (!ts.has_activation()) not_a_fill = ts.timecode.ticks();
        }
        CHECK_MESSAGE(not_a_fill.has_value(), "no chord without a fill before tick " << act_tick);
        if (!not_a_fill) continue;
        ++checked;
        CHECK_MESSAGE(search_target(song, cfg, {*not_a_fill}).empty(),
                      "a path came back for tick " << *not_a_fill << ", which is not a fill");
    }
    MESSAGE(checked << " charts checked, " << no_part << " with no notes at these settings, "
                    << no_activation << " whose best path never activates");
    // A walk that met no chart with an activation built no negative case.
    CHECK(checked > 0);
}

TEST_CASE("replay without Star Power scores no doubling at all") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), ChartReading{});
    REQUIRE_FALSE(song.is_empty());

    const ReplayResult r = replay_path(song, {});
    CHECK(r.final.sp == 0);
    CHECK(r.final.notes > 0);
    CHECK(r.chords.size() == song.sequence.size());

    // No chord is in Star Power, none earns SP points, and the disc shows
    // the plain combo multiplier on every one.
    for (const ReplayChord& c : r.chords) {
        CAPTURE(c.tick);
        CHECK_FALSE(c.in_sp);
        CHECK(c.points.sp == 0);
        CHECK(c.multiplier_shown == c.multiplier_after);
    }
}

// D2 on the engine's own squeeze-outs: every window the search stored with a
// squeezed-out chord, on every corpus chart at the app's defaults. Under the
// default first-note rule a one-note chord squeezed out loses all its
// doubling, whether it sits inside the window or past the leeway. On any
// squeezed-out chord, the disc doubles exactly when SP paid it something.
TEST_CASE("a squeezed-out chord SP pays nothing shows the plain multiplier, on corpus paths") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    REQUIRE(cfg.rules.sqout_rule == core::SqOutRule::FirstNote);
    int sqout_chords = 0, one_note = 0;
    const CorpusPathRun run = check_every_corpus_path(
        cfg, [&](const Song& song, const Path& p) -> std::string {
            const std::vector<ReplayWindow> wl = windows_for_path(p);
            const ReplayResult r = replay_path(song, wl, cfg.rules);
            for (const ReplayWindow& w : wl) {
                if (!w.sqout_tick) continue;
                const ReplayChord* c = nullptr;
                for (const ReplayChord& row : r.chords)
                    if (row.tick == *w.sqout_tick) c = &row;
                if (!c) return ": no chord on squeeze-out tick " + std::to_string(*w.sqout_tick);
                ++sqout_chords;
                const std::string at = " on squeeze-out tick " + std::to_string(c->tick);
                if (c->in_sp != (c->points.sp > 0)) return ": in_sp disagrees with SP points" + at;
                const int want = c->in_sp ? c->multiplier_after * kStarPowerMultiplier
                                          : c->multiplier_after;
                if (c->multiplier_shown != want) return ": disc shows the wrong multiplier" + at;
                if (c->notes.size() == 1) {
                    ++one_note;
                    if (c->points.sp != 0) return ": a one-note squeeze-out kept SP points" + at;
                }
            }
            return "";
        });
    MESSAGE(sqout_chords << " squeezed-out chords, " << one_note << " of one note");
    CHECK(run.charts > 0);
    // The corpus stores squeeze-outs (test_engine_sweeps.cpp walks them), so
    // a walk that met none, or no one-note one, checked nothing here.
    CHECK(sqout_chords > 0);
    CHECK(one_note > 0);
    INFO("first mismatch: " << run.first_diff);
    CHECK(run.mismatches == 0);
}

TEST_CASE("paid_by_sp: yes exactly when the row's SP value is above zero") {
    using core::SqOutPosition;
    // The leeway past the SP end, read from the rules the app runs with.
    const double leeway = core::default_rules().backend_leeway_ms;
    REQUIRE(leeway > 0.0);
    // A row's full SP value, and what is left of it when it is the
    // squeezed-out chord: some, or none at all.
    const int sp_points = 100, sqout_some = 50, sqout_none = 0;
    const double inside = -10.0;  // before the SP end

    struct Case {
        const char* name;
        double offset_ms;
        int sqout_sp_points;
        SqOutPosition pos;
        bool paid;
    };
    const Case cases[] = {
        {"inside the window, no squeeze-out", inside, sqout_none, SqOutPosition::NoSqOut, true},
        {"the squeezed-out chord with nothing left", inside, sqout_none, SqOutPosition::Exact,
         false},
        {"the squeezed-out chord with points left", inside, sqout_some, SqOutPosition::Exact, true},
        {"a chord after the squeezed-out one", inside, sqout_some, SqOutPosition::After, false},
        {"inside the leeway", leeway / 2.0, sqout_some, SqOutPosition::NoSqOut, true},
        {"exactly the leeway after the end (D29: strict)", leeway, sqout_some,
         SqOutPosition::NoSqOut, false},
        {"past the leeway", leeway * 2.0, sqout_some, SqOutPosition::NoSqOut, false},
    };
    for (const Case& c : cases) {
        CAPTURE(c.name);
        const bool paid = core::paid_by_sp(c.offset_ms, sp_points, c.sqout_sp_points, c.pos, leeway);
        CHECK(paid == c.paid);
        // The yes is the row's SP value being above zero, the name's claim.
        const int value =
            core::backend_row_value(c.offset_ms, sp_points, c.sqout_sp_points, c.pos, leeway);
        CHECK(paid == (value > 0));
    }
}


/// ambiguous_window_warnings reads in_sp, so a chord the replay did not pay SP
// for is never named, just because the engine could squeeze it out. Every
// corpus path is replayed with its windows typed plain (the squeeze-outs
// taken off, so each window has candidates to be asked about). For every
// window and every chord core::sqout_chords offers, the warning names the
// chord exactly when the replay's row for it is in Star Power.
TEST_CASE("replay: the squeeze-out warning names a candidate chord exactly when the replay paid it") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int paid = 0, unpaid = 0;
    const CorpusPathRun run = check_every_corpus_path(
        cfg, [&](const Song& song, const Path& p) -> std::string {
            std::vector<ReplayWindow> wl = windows_for_path(p);
            for (ReplayWindow& w : wl) {
                w.sqout_tick.reset();
                w.sqout_offset_ms.reset();
                w.from_record = false;
            }
            const ReplayResult r = replay_path(song, wl, cfg.rules);
            const std::vector<std::string> warned =
                ambiguous_window_warnings(song, r, wl, cfg.rules);
            for (const ReplayWindow& w : wl) {
                const std::string head =
                    "window " + std::to_string(w.act_tick) + ":" + std::to_string(w.deact_tick) + " ";
                for (const SongTimestamp* engine :
                     core::sqout_chords(song, song.timing().timecode(w.deact_tick), w.act_tick,
                                        w.sqin_ticks, w.from_record)) {
                    const int64_t tick = engine->timecode.ticks();
                    const ReplayChord* c = nullptr;
                    for (const ReplayChord& row : r.chords)
                        if (row.tick == tick) c = &row;
                    if (!c) return ": no chord on candidate tick " + std::to_string(tick);
                    bool named = false;
                    for (const std::string& warn : warned)
                        if (warn.rfind(head, 0) == 0 &&
                            warn.find("tick " + std::to_string(tick) + " ") != std::string::npos)
                            named = true;
                    (c->in_sp ? paid : unpaid)++;
                    if (named != c->in_sp)
                        return std::string(named ? ": warned about an unpaid chord"
                                                 : ": no warning about a paid chord") +
                               " at tick " + std::to_string(tick);
                }
            }
            return "";
        });
    MESSAGE(paid << " paid candidates, " << unpaid << " unpaid");
    CHECK(run.charts > 0);
    // Both sides must occur, or the walk never asked the question.
    CHECK(paid > 0);
    CHECK(unpaid > 0);
    INFO("first mismatch: " << run.first_diff);
    CHECK(run.mismatches == 0);
}

TEST_CASE("shown_multiplier doubles the combo multiplier only under Star Power") {
    // The doubling is kStarPowerMultiplier, read from its owner by name.
    for (int m : {1, 2, 3, 4}) {
        CAPTURE(m);
        CHECK(shown_multiplier(m, false) == m);
        CHECK(shown_multiplier(m, true) == m * kStarPowerMultiplier);
    }
}

// `score --path` prices a path straight out of the JSON `dump` and `target`
// write. The reason to read the file rather than retype the path as an
// "act:deact,..." string is that the file carries the squeeze-out offset and
// a retyped string usually drops it -- and without the offset the squeezed
// note is doubled as if Star Power were still running, so the score comes out
// high. So what has to be pinned is that the squeezed-out chord survives the
// trip. The stamped tick names that chord; the SqOut offset is read only for a
// dump from before the tick existed (read_sqout in tools/replay_json.cpp).
TEST_CASE("a path JSON names the squeezed-out chord by its tick, the offset only for older dumps") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int by_tick = 0, by_offset = 0, plain = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const std::vector<const Path*> all = corpus::analyzed(chart, cfg).all_paths();
        const json dumped = paths_json(all, song.timing());
        REQUIRE(dumped.size() == all.size());
        for (size_t k = 0; k < all.size(); ++k) {
            CAPTURE(chart);
            CAPTURE(all[k]->pathstring());
            const std::vector<ReplayWindow> want = windows_for_path(*all[k]);

            // The dump as written: a squeezed-out window is named by its tick
            // and the offset is not read.
            const std::vector<ReplayWindow> got = windows_from_json(dumped[k]);
            REQUIRE(got.size() == want.size());
            for (size_t i = 0; i < got.size(); ++i) {
                CHECK(got[i].sqout_tick == want[i].sqout_tick);
                CHECK_FALSE(got[i].sqout_offset_ms.has_value());
                ++(want[i].sqout_tick ? by_tick : plain);
            }

            // An older dump, with no tick: this real dump with the tick taken
            // out. The offset then names the chord, read from the SqOut in
            // the same list as any SqIn, never from the SqIn.
            json old = dumped[k];
            for (json& act : old["activations"]) act.erase("sqout_tick");
            const std::vector<ReplayWindow> older = windows_from_json(old);
            REQUIRE(older.size() == want.size());
            for (size_t i = 0; i < older.size(); ++i) {
                CHECK_FALSE(older[i].sqout_tick.has_value());
                CHECK(older[i].sqout_offset_ms == want[i].sqout_offset_ms);
                if (want[i].sqout_offset_ms) ++by_offset;
            }
        }
    }
    MESSAGE(by_tick << " squeezed-out windows by tick, " << by_offset << " by offset, "
                    << plain << " plain");
    // A walk that met no squeezed-out window checked nothing here.
    CHECK(by_tick > 0);
    CHECK(by_offset == by_tick);

    // JSON that is not a path at all says so rather than scoring something.
    CHECK_THROWS(windows_from_json(json::object()));
    // -1 is how a dump writes "this record has no deactivation node".
    CHECK_THROWS(windows_from_json(json::parse(
        R"({"activations": [{"act_tick": 480, "deact_tick": -1}]})")));
}

// A path read from a file has to give the same windows as the same path read
// from the record it was dumped from, or `--path` would price something the
// database does not agree with.
TEST_CASE("windows read from a path JSON match the ones read from the record") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    // Every stored path of every corpus chart, a path with no windows too
    // (its dump must read back as none). A chart that fails to load throws;
    // one with no notes at these settings is counted and printed.
    int windows = 0;
    const CorpusPathRun run = check_every_corpus_path(
        cfg, [&](const Song& song, const Path& p) -> std::string {
            const std::vector<ReplayWindow> want = windows_for_path(p);
            // The real dump producer, so a format change on either side fails.
            const json dumped = paths_json({&p}, song.timing());
            if (dumped.size() != 1) return ": the dump wrote " + std::to_string(dumped.size()) + " paths";
            std::vector<ReplayWindow> got;
            try {
                got = windows_from_json(dumped[0]);
            } catch (const std::runtime_error& e) {
                return std::string(": the dump does not read back: ") + e.what();
            }
            if (got.size() != want.size())
                return ": window count " + std::to_string(got.size()) + " vs " +
                       std::to_string(want.size());
            for (size_t i = 0; i < got.size(); ++i) {
                ++windows;
                // read_sqout (tools/replay_json.cpp) decides when the offset
                // is read: only when the window has no squeeze-out tick.
                if (got[i].act_tick != want[i].act_tick ||
                    got[i].deact_tick != want[i].deact_tick ||
                    got[i].sqout_tick != want[i].sqout_tick ||
                    (!want[i].sqout_tick && got[i].sqout_offset_ms != want[i].sqout_offset_ms) ||
                    got[i].sqin_ticks != want[i].sqin_ticks ||
                    got[i].from_record != want[i].from_record)
                    return ": window " + std::to_string(i) + " differs";
            }
            return "";
        });
    MESSAGE(run.charts << " charts, " << run.paths << " paths, " << windows << " windows compared, "
                       << run.no_part << " charts with no notes at these settings");
    CHECK(run.charts > 0);
    // A walk that compared no window checked nothing.
    CHECK(windows > 0);
    INFO("first mismatch: " << run.first_diff);
    CHECK(run.mismatches == 0);
}

// D34: a window never squeezes out a phrase it already squeezed in, and the
// replay's squeeze-out lookup and warnings skip those phrases
// (ReplayWindow::sqin_ticks). `score --path <dump>` reads its windows from the
// dump, so the dump has to carry each window's SqIn phrases. Without them, a
// path on spent_then_next.chart at cap 2 would warn that a phrase might have
// been squeezed out, though it was squeezed in. A dump without the field reads
// as no SqIns.
TEST_CASE("a dump carries each window's SqIn phrases back to the replay") {
    const Song song = load_songpath(
        corpus::root() + "/test_fast_tempo/spent_then_next.chart", ChartReading{});
    const HydraRecord rec = analyze_chart(song, test::scores_settings(2));
    const std::vector<const Path*> all = rec.all_paths();
    const json dumped = paths_json(all, song.timing());
    REQUIRE(dumped.size() == all.size());
    int with_sqins = 0;
    for (size_t k = 0; k < all.size(); ++k) {
        CAPTURE(all[k]->pathstring());
        const std::vector<ReplayWindow> want = windows_for_path(*all[k]);
        const std::vector<ReplayWindow> got = windows_from_json(dumped[k]);
        REQUIRE(got.size() == want.size());
        std::vector<int64_t> squeezed_in;
        for (size_t i = 0; i < got.size(); ++i) {
            CHECK(got[i].sqin_ticks == want[i].sqin_ticks);
            with_sqins += want[i].sqin_ticks.empty() ? 0 : 1;
            squeezed_in.insert(squeezed_in.end(), want[i].sqin_ticks.begin(),
                               want[i].sqin_ticks.end());
        }
        // A phrase a window squeezed in is never offered as a squeeze-out.
        for (const std::string& warn :
             ambiguous_window_warnings(song, replay_path(song, got), got,
                                       core::default_rules()))
            for (int64_t t : squeezed_in)
                CHECK_MESSAGE(warn.find("tick " + std::to_string(t) + " ") == std::string::npos,
                              warn);

        // An older dump, without the key, still reads: no SqIn phrases. It is
        // this real dump with the key taken out.
        json old = dumped[k];
        for (json& act : old["activations"]) act.erase("sqin_ticks");
        for (const ReplayWindow& w : windows_from_json(old)) CHECK(w.sqin_ticks.empty());
    }
    CHECK(with_sqins > 0);
}

// fcvideo and `hydra_replay score --path` read these fields out of a dump. A
// dump-format change that drops one has to fail here, not in the video tools.
TEST_CASE("paths_json writes every field the dump readers use") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    bool checked = false;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const std::vector<const Path*> all = rec.all_paths();
        if (all.empty() || all[0]->all_activations().empty()) continue;

        const json dumped = paths_json(all, song.timing());
        REQUIRE(dumped.size() == all.size());

        const json& p0 = dumped[0];
        for (const char* k : {"index", "pathstring", "total", "score", "activations"})
            CHECK_MESSAGE(p0.contains(k), k);
        for (const char* k : {"notes", "combo", "sp", "solo", "accent", "ghost"})
            CHECK_MESSAGE(p0["score"].contains(k), k);
        CHECK(p0["pathstring"].get<std::string>() == all[0]->pathstring());

        REQUIRE(!p0["activations"].empty());
        const json& a0 = p0["activations"][0];
        for (const char* k : {"act_tick", "deact_tick", "sqout_tick", "nominal_deact_tick",
                              "sp_meter", "sp_ready_tick", "sp_ready_ms", "skips", "skipped_fill_ticks",
                              "chord_code", "sqinouts",
                              "sqin_ticks", "end_anchor_tick", "end_scale_early", "end_scale_late",
                              "steady_ms_before_act", "steady_ms_after_act",
                              "steady_ms_before_end", "steady_ms_after_end"})
            CHECK_MESSAGE(a0.contains(k), k);
        // The real search stores the end's ratio on every activation it
        // returns, so a fresh dump never writes it null.
        CHECK(a0["end_anchor_tick"].is_number());
        CHECK(a0["end_scale_early"].is_number());
        CHECK(a0["end_scale_late"].is_number());

        checked = true;
        break;  // one chart's first path is the whole contract
    }
    CHECK(checked);
}

namespace {

// A hand-built activation at `act_tick` on a made-up timing, with no SP-end
// history yet.
Activation act_at(const SongTiming& timing, int64_t act_tick) {
    Activation a;
    a.timecode = timing.timecode(act_tick);
    return a;
}

// `a` dumped alone, as the dump prints it: paths_json is the dump's real
// writer. With `stamp`, the end's ratio is stored first the way the search's
// copy-out stores it, through frontend_transfer_scales; without, the test has
// set transfer_post itself.
json dump_alone(const SongTiming& timing, Activation a, bool stamp = true) {
    if (stamp)
        if (const std::optional<ActTransferScales> s = frontend_transfer_scales(a, timing))
            a.transfer_post = s->post;
    Path p;
    p.activations = {a};
    const json dumped = paths_json({&p}, timing);
    REQUIRE(dumped.size() == 1);
    REQUIRE(dumped[0]["activations"].size() == 1);
    return dumped[0]["activations"][0];
}

// An activation at `act_tick` whose Star Power ends at `end_tick`, or one with
// no end at all when `end_tick` is empty, dumped with its ratio stamped.
json dump_one_activation(const SongTiming& timing, int64_t act_tick,
                         std::optional<int64_t> end_tick) {
    Activation a = act_at(timing, act_tick);
    if (end_tick) test::set_plain_window(a, *end_tick);
    return dump_alone(timing, a);
}

double rounded_4(double v) { return std::round(v * 1e4) / 1e4; }

const char* const kSteadyFields[] = {"steady_ms_before_act", "steady_ms_after_act",
                                     "steady_ms_before_end", "steady_ms_after_end"};

}  // namespace

// On one steady tempo and meter the end moves exactly as far as the
// activation, both ways, and no change lies on either side of either tick. A
// marker that repeats the tempo or meter in force is no change.
TEST_CASE("paths_json: a constant tempo gives end ratios of 1 and no change on any side") {
    // Every corpus chart whose tempo and meter never change (a marker that
    // repeats the one in force is no change), dumped by the real writer.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const auto steady = [](const auto& changes) {
        for (const auto& kv : changes)
            if (kv.second != changes.begin()->second) return false;
        return true;
    };
    int charts = 0, activations = 0, ratios = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.reading());
        if (song.is_empty() || !steady(song.bpm_changes) || !steady(song.tpm_changes)) continue;
        const std::vector<const Path*> all = corpus::analyzed(chart, cfg).all_paths();
        const json dumped = paths_json(all, song.timing());
        REQUIRE(dumped.size() == all.size());
        ++charts;
        for (size_t k = 0; k < all.size(); ++k) {
            CAPTURE(chart);
            CAPTURE(all[k]->pathstring());
            for (const json& a : dumped[k]["activations"]) {
                ++activations;
                // Null when the search stored no ratio: only a stored one is read.
                for (const char* key : {"end_scale_early", "end_scale_late"}) {
                    REQUIRE_MESSAGE(a.contains(key), key);
                    if (!a[key].is_number()) continue;
                    ++ratios;
                    CHECK_MESSAGE(a[key].get<double>() == doctest::Approx(1.0), key);
                }
                for (const char* key : kSteadyFields) {
                    REQUIRE_MESSAGE(a.contains(key), key);
                    CHECK_MESSAGE(a[key].is_null(), key);
                }
            }
        }
    }
    MESSAGE(charts << " constant-tempo charts, " << activations << " activations, " << ratios
                   << " stored ratios");
    // A walk that met no such chart, or read no ratio, checked nothing here.
    CHECK(charts > 0);
    CHECK(ratios > 0);
}

// Rescued's shape: a tempo marker sits on the activation tick itself (150.67
// BPM before it, 155.656 from it), and the end's tick follows 150 BPM. An
// early hit lands in the segment before the marker, so the early ratio reads
// that segment's tempo, not the marker's, and the early side stays exact back
// to the marker at 83520: one beat at 150.67 BPM, about 398 ms. The end's
// early side reaches back to the 150 BPM marker at 89000: 856 ticks at 150
// BPM. Nothing changes after the end.
TEST_CASE("paths_json: a tempo marker on the activation tick gives the earlier segment's ratio") {
    const SongTiming timing(192, {{0, 768}},
                            {{0, 120.0}, {83520, 150.67}, {83712, 155.656}, {89000, 150.0}},
                            {{0, {4, 4}}});
    const json a = dump_one_activation(timing, 83712, 89856);
    REQUIRE(a["end_scale_early"].is_number());
    REQUIRE(a["end_scale_late"].is_number());
    CHECK(rounded_4(a["end_scale_early"].get<double>()) == doctest::Approx(1.0045));
    CHECK(rounded_4(a["end_scale_late"].get<double>()) == doctest::Approx(1.0377));
    REQUIRE(a["steady_ms_before_act"].is_number());
    CHECK(a["steady_ms_before_act"].get<double>() == doctest::Approx(398.22).epsilon(1e-4));
    REQUIRE(a["steady_ms_before_end"].is_number());
    CHECK(a["steady_ms_before_end"].get<double>() == doctest::Approx(1783.33).epsilon(1e-5));
    CHECK(a["steady_ms_after_end"].is_null());
}

// A change exactly on the tick starts the late side's section, so it bounds
// neither side: the early side measures back to the change before (768, 768
// ticks at 60 BPM = 4000 ms) and the late side on to the change after (2304,
// 768 ticks at 120 BPM = 2000 ms), never 0. The end at 3840 reaches back to
// 2304 (1536 ticks at 60 BPM = 8000 ms) with nothing after it.
TEST_CASE("paths_json: a change on the tick bounds neither of its steady spans") {
    const SongTiming timing(192, {{0, 768}},
                            {{0, 120.0}, {768, 60.0}, {1536, 120.0}, {2304, 60.0}},
                            {{0, {4, 4}}});
    const json a = dump_one_activation(timing, 1536, 3840);
    REQUIRE(a["steady_ms_before_act"].is_number());
    CHECK(a["steady_ms_before_act"].get<double>() == doctest::Approx(4000.0));
    REQUIRE(a["steady_ms_after_act"].is_number());
    CHECK(a["steady_ms_after_act"].get<double>() == doctest::Approx(2000.0));
    REQUIRE(a["steady_ms_before_end"].is_number());
    CHECK(a["steady_ms_before_end"].get<double>() == doctest::Approx(8000.0));
    CHECK(a["steady_ms_after_end"].is_null());
}

// A meter change between the activation and its end changes the measure
// length while the tempo stays put, so the ratio is the measure ratio (3/4
// against 4/4), not the BPM ratio of 1. The meter change bounds the spans the
// way a tempo change does: 768 ticks at 120 BPM after the activation, 1536
// before the end.
TEST_CASE("paths_json: a time signature change between the ticks gives the measure ratio") {
    const SongTiming timing(192, {{0, 768}, {1536, 576}}, {{0, 120.0}},
                            {{0, {4, 4}}, {1536, {3, 4}}});
    const json a = dump_one_activation(timing, 768, 3072);
    REQUIRE(a["end_scale_early"].is_number());
    REQUIRE(a["end_scale_late"].is_number());
    CHECK(a["end_scale_early"].get<double>() == doctest::Approx(0.75));
    CHECK(a["end_scale_late"].get<double>() == doctest::Approx(0.75));
    CHECK(a["end_scale_early"].get<double>() != doctest::Approx(1.0));
    CHECK(a["steady_ms_before_act"].is_null());
    REQUIRE(a["steady_ms_after_act"].is_number());
    CHECK(a["steady_ms_after_act"].get<double>() == doctest::Approx(2000.0));
    REQUIRE(a["steady_ms_before_end"].is_number());
    CHECK(a["steady_ms_before_end"].get<double>() == doctest::Approx(4000.0));
    CHECK(a["steady_ms_after_end"].is_null());
}

// When the cap pinned the end to a collecting note, that note's timing moves
// the end, not the activation's: end_anchor_tick names it and the "act" spans
// are measured from it. Here the clamp at 1536 sits 768 ticks at 60 BPM past
// the change at 768; from the activation at 384 there would be no change
// before at all.
TEST_CASE("paths_json: a clamped end is anchored on its collecting note") {
    const SongTiming timing(192, {{0, 768}}, {{0, 120.0}, {768, 60.0}}, {{0, {4, 4}}});
    Activation a = act_at(timing, 384);
    test::set_clamped_window(a, 1536, 3072);
    const json d = dump_alone(timing, a);
    CHECK(d["act_tick"].get<int64_t>() == 384);
    CHECK(d["end_anchor_tick"].get<int64_t>() == 1536);
    REQUIRE(d["steady_ms_before_act"].is_number());
    CHECK(d["steady_ms_before_act"].get<double>() == doctest::Approx(4000.0));
    CHECK(d["steady_ms_after_act"].is_null());
}

// The ratios are the ones the search stored, never worked out again in the
// writer: a stored pair that no timing would give comes out as stored, and an
// activation the search stored none for writes null ratios beside its spans.
TEST_CASE("paths_json writes the end ratio the search stored") {
    const SongTiming timing(192, {{0, 768}}, {{0, 120.0}}, {{0, {4, 4}}});
    Activation a = act_at(timing, 3840);
    test::set_plain_window(a, 9984);
    a.transfer_post = TransferScale{0.5, 2.0};
    const json stored = dump_alone(timing, a, false);
    CHECK(stored["end_scale_early"].get<double>() == 0.5);
    CHECK(stored["end_scale_late"].get<double>() == 2.0);

    a.transfer_post.reset();
    const json unknown = dump_alone(timing, a, false);
    CHECK(unknown["end_anchor_tick"].get<int64_t>() == 3840);
    CHECK(unknown["end_scale_early"].is_null());
    CHECK(unknown["end_scale_late"].is_null());
    for (const char* k : kSteadyFields) CHECK_MESSAGE(unknown.contains(k), k);
}

// An activation with no deactivation node has no end to move, so every end
// field the dump writes for it is null, and its deact_tick is -1 as before.
TEST_CASE("paths_json: an activation with no deact tick writes every end field null") {
    const SongTiming timing(192, {{0, 768}}, {{0, 120.0}, {768, 60.0}}, {{0, {4, 4}}});
    const json a = dump_one_activation(timing, 3840, std::nullopt);
    CHECK(a["deact_tick"].get<int64_t>() == -1);
    for (const char* k : {"end_anchor_tick", "end_scale_early", "end_scale_late"}) {
        REQUIRE_MESSAGE(a.contains(k), k);
        CHECK_MESSAGE(a[k].is_null(), k);
    }
    for (const char* k : kSteadyFields) {
        REQUIRE_MESSAGE(a.contains(k), k);
        CHECK_MESSAGE(a[k].is_null(), k);
    }
}

// dump and target print one "result" block. A record with no paths has no best
// score (HydraRecord::optimal_score's answer, which hydra_batch prints as "-"), so the
// block says null there rather than a real-looking 0 (audit finding 98).
TEST_CASE("result block: an empty record writes a null score and an empty bestpath") {
    const json empty = result_json(HydraRecord{});
    CHECK(empty["score"].is_null());
    CHECK(empty["bestpath"].get<std::string>() == "");

    // One corpus chart, named by its path, with its best score and path as one
    // run printed them.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const std::string chart = corpus::root() + "/common/IB24/T1/Allister - Overrated/notes.mid";
    const json full = result_json(corpus::analyzed(chart, cfg));
    CHECK(full["score"].get<int64_t>() == 228710);
    CHECK(full["bestpath"].get<std::string>() == "1 2");
}

// dump prints each activation's passed-over fills as the record stores them.
// On the 1.0-rule fill song the stored fill (19200) is not the one nearest
// the activation (24960), so a dump that guessed would print the wrong tick.
TEST_CASE("paths_json prints the stored passed-over fills, not a guess") {
    const Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    HydraRecord rec;
    rec.paths = run_search(graph, opts);
    REQUIRE(!rec.paths.empty());
    const std::vector<const Path*> all = rec.all_paths();
    const json dumped = paths_json(all, song.timing());
    REQUIRE(dumped.size() == all.size());
    for (size_t k = 0; k < all.size(); ++k) {
        const ActivationWalk acts = all[k]->walk_activations();
        REQUIRE(dumped[k]["activations"].size() == acts.size());
        for (size_t i = 0; i < acts.size(); ++i) {
            const json& a = dumped[k]["activations"][i];
            CHECK(a["skipped_fill_ticks"].get<std::vector<int64_t>>() ==
                  acts[i].skipped_fill_ticks);
            CHECK(a["skips"].get<int>() == acts[i].skips());
        }
    }
    // The pinned activation passes fills on its way, so the list is not empty,
    // and each tick it names is a fill the chart carries before that
    // activation. The ticks come from the song, never a typed number.
    const int64_t act_tick = opts.target_act_ticks->front();
    const std::vector<int64_t> skipped =
        dumped[0]["activations"][0]["skipped_fill_ticks"].get<std::vector<int64_t>>();
    CHECK_FALSE(skipped.empty());
    for (int64_t t : skipped) {
        CAPTURE(t);
        CHECK(t < act_tick);
        bool is_fill = false;
        for (const SongTimestamp& ts : song.sequence)
            if (ts.timecode.ticks() == t && ts.has_activation()) is_fill = true;
        CHECK(is_fill);
    }
}

// A window that ends on the note closing a Star Power phrase is exactly where
// a squeeze-out hides. If the player squeezed, that note was hit after Star
// Power ran out and is not doubled; if they did not, it is. The window list
// alone cannot tell the two apart, so the tool says so instead of guessing.
TEST_CASE("a window ending on a phrase note with no offset is flagged") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), ChartReading{});
    REQUIRE_FALSE(song.is_empty());

    const ReplayResult r = replay_path(song, {});

    // The first phrase note, the chord just after it, the first chord that is
    // further past a phrase note than a squeeze could ever reach, and a chord
    // with no phrase note behind it at all.
    const ReplayChord* phrase_note = nullptr;    // the first phrase note
    const ReplayChord* just_after = nullptr;     // the chord right after it
    const ReplayChord* long_after = nullptr;     // first chord out of reach
    const ReplayChord* no_phrase_yet = nullptr;  // a chord before any of them
    const ReplayChord* last_phrase = nullptr;
    for (const ReplayChord& c : r.chords) {
        if (c.is_sp_phrase_end) {
            last_phrase = &c;
            if (!phrase_note) phrase_note = &c;
            continue;
        }
        if (!last_phrase) {
            if (!no_phrase_yet) no_phrase_yet = &c;
            continue;
        }
        // Inside or out of reach is asked of the window's owner,
        // within_squeeze_window, on the gap in ms.
        if (last_phrase == phrase_note && !just_after &&
            within_squeeze_window(c.ms - phrase_note->ms))
            just_after = &c;
        if (!long_after && !within_squeeze_window(c.ms - last_phrase->ms))
            long_after = &c;
    }
    REQUIRE(phrase_note != nullptr);
    REQUIRE(just_after != nullptr);
    REQUIRE(long_after != nullptr);
    REQUIRE(no_phrase_yet != nullptr);

    ReplayWindow w;
    w.act_tick = r.chords.front().tick;

    // The warning only fires for a chord the window paid, so it reads the
    // replay of that same window, as `hydra_replay score` does.
    auto warnings_for = [&](const ReplayWindow& win) {
        return ambiguous_window_warnings(song, replay_path(song, {win}), {win},
                                         core::default_rules());
    };

    // Ending on the phrase note itself.
    w.deact_tick = phrase_note->tick;
    const std::vector<std::string> flagged = warnings_for(w);
    REQUIRE(flagged.size() == 1);
    CHECK(flagged[0].find(std::to_string(phrase_note->tick)) !=
          std::string::npos);

    // Ending a hair after it: still the same doubt. This is the case a rule
    // that only looked at the deactivation tick itself would miss, and it is
    // a real one -- five of Hail The Sun - Wake's six squeeze-outs sit on the
    // node and the sixth sits 93.75 ms before it.
    w.deact_tick = just_after->tick;
    CHECK(warnings_for(w).size() == 1);

    // Far enough past it that no squeeze could have reached: no doubt left.
    w.deact_tick = long_after->tick;
    CHECK(warnings_for(w).empty());

    // No phrase note in the window at all: never in doubt.
    w.deact_tick = no_phrase_yet->tick;
    CHECK(warnings_for(w).empty());

    // An offset settles the question, so there is nothing left to warn about.
    ReplayWindow settled;
    settled.act_tick = r.chords.front().tick;
    settled.deact_tick = phrase_note->tick;
    settled.sqout_offset_ms = 8.0;
    CHECK(ambiguous_window_warnings(song, r, {settled}, core::default_rules()).empty());
}

TEST_CASE("per-note sp points sum to the chord's sp points") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), ChartReading{});
    REQUIRE_FALSE(song.is_empty());

    // One window over the whole chart, so every chord is under Star Power and
    // its points.sp is the full doubling.
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = song.sequence.back().timecode.ticks();
    const ReplayResult r = replay_path(song, {w});

    for (const ReplayChord& c : r.chords) {
        CHECK(c.in_sp);
        int64_t sum = 0;
        for (const ReplayNote& n : c.notes) sum += n.sp_points;
        CHECK(sum == c.points.sp);
        CHECK(static_cast<int>(c.notes.size()) > 0);
    }
}

namespace {

// 4/4, 120 BPM, 192 ticks per beat: 768 ticks and 2000 ms per measure, so
// 36 ticks are exactly 93.75 ms and 192 ticks exactly 500 ms.
Song song_with(const std::vector<std::pair<int64_t, bool>>& chords) {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (const auto& [tick, phrase] : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        if (phrase) test::mark_phrase_end(ts, tick, song.tick_resolution());
        song.sequence.push_back(ts);
    }
    return song;
}

// One chord per beat at 120 BPM; chord i holds the colors listed at i.
Song make_chord_song(const std::vector<std::vector<NoteColor>>& chords) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (size_t i = 0; i < chords.size(); ++i) {
        SongTimestamp ts;
        ts.timecode = song.timecode(480 * static_cast<int64_t>(i));
        for (NoteColor c : chords[i]) ts.chord.add_note(c);
        song.sequence.push_back(ts);
    }
    return song;
}

// Adds a dynamic Yellow cymbal to `chord`.
void add_dynamic_cymbal(Chord& chord, NoteDynamicType dyn) {
    ChordNote& y = chord.add_note(NoteColor::Yellow);
    y.cymbaltype = NoteCymbalType::Cymbal;
    y.dynamictype = dyn;
}

}  // namespace

// A hand-built phrase end carries what the parser writes on one: the flag
// and where the phrase starts (test::mark_phrase_end, one resolution back,
// clamped at 0).
TEST_CASE("fixtures: mark_phrase_end sets the flag and the phrase start together") {
    const Song song = song_with({{0, true}, {192, false}, {768, true}});
    REQUIRE(song.sequence.size() == 3);
    CHECK(song.sequence[0].flag_sp);
    CHECK(song.sequence[0].sp_phrase_start == 0);
    CHECK_FALSE(song.sequence[1].flag_sp);
    CHECK_FALSE(song.sequence[1].sp_phrase_start.has_value());
    CHECK(song.sequence[2].flag_sp);
    CHECK(song.sequence[2].sp_phrase_start == 576);
}

// A typed offset is only ever an approximation of a chord that sits on a
// tick. The tool resolves it to the phrase chord it means and says which.
TEST_CASE("a typed squeeze-out offset resolves to the phrase chord") {
    const Song song = song_with(
        {{0, false}, {768, false}, {1536, false}, {3036, true}, {3072, false}});

    ReplayWindow typed;
    typed.act_tick = 0;
    typed.deact_tick = 3072;
    typed.sqout_offset_ms = -93.73;  // what a person copies off a screen

    const SqOutNote n = resolve_sqout_note(song, typed);
    CHECK(n.tick == 3036);
    CHECK(n.offset_ms == doctest::Approx(-93.75));

    typed.sqout_tick = n.tick;
    ReplayWindow exact;
    exact.act_tick = 0;
    exact.deact_tick = 3072;
    exact.sqout_tick = 3036;
    CHECK(replay_path(song, {typed}).final == replay_path(song, {exact}).final);

    // No phrase chord inside the SP end's squeeze window: nothing to resolve to.
    const Song bare = song_with({{0, false}, {3072, false}});
    CHECK_THROWS(resolve_sqout_note(bare, typed));

    // replay_path refuses an offset it was never told the chord for.
    ReplayWindow unresolved;
    unresolved.act_tick = 0;
    unresolved.deact_tick = 3072;
    unresolved.sqout_offset_ms = -93.75;
    CHECK_THROWS(replay_path(song, {unresolved}));
}

// The engine only ever squeezes out a window's newest phrase chord at or
// before the SP end, or the first one after it (D36): a phrase hit earlier
// than a later one cannot be hit after SP ran out while the later one is
// hit before. A typed offset that lands on an older one names a squeeze-out
// the search can never produce, so it is refused and nothing is priced
// (D274 item 20).
TEST_CASE("a typed squeeze-out on a chord the engine never squeezes out is refused") {
    // Phrase chords 375 ms (tick 2928) and 93.75 ms (tick 3036) before D.
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3036, true}, {3072, false}});

    ReplayWindow older;
    older.act_tick = 0;
    older.deact_tick = 3072;
    older.sqout_offset_ms = -375.0;
    CHECK_THROWS_WITH_AS(
        resolve_sqout_note(two, older),
        "window 0:3072: the SqOut offset -375.00 ms lands on the phrase chord "
        "at tick 2928 (-375.00 ms from the SP end), which the engine never "
        "squeezes out. A window squeezes out only its newest phrase chord at "
        "or before the SP end or the first one after it: here tick 3036 "
        "(-93.75 ms). Not priced.",
        std::runtime_error);

    // The engine's own chord is accepted.
    ReplayWindow newest = older;
    newest.sqout_offset_ms = -93.73;
    const SqOutNote n = resolve_sqout_note(two, newest);
    CHECK(n.tick == 3036);
    CHECK(n.offset_ms == doctest::Approx(-93.75));
}

// A typed offset exactly halfway between two phrase chords the engine could
// both squeeze out names neither, so it is refused and nothing is priced.
// Tie rule owner: resolve_sqout_note (decision D83).
TEST_CASE("a typed squeeze-out offset halfway between two phrase chords is refused") {
    // Phrase chords 93.75 ms before (tick 3036) and after (tick 3108) D = 3072.
    const Song song = song_with({{0, false}, {768, false}, {3036, true},
                                 {3072, false}, {3108, true}});
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    w.sqout_offset_ms = 0.0;
    CHECK_THROWS_WITH_AS(
        resolve_sqout_note(song, w),
        "window 0:3072: the SqOut offset 0.00 ms is equally close to the "
        "phrase chords at tick 3036 (-93.75 ms) and tick 3108 (93.75 ms). "
        "Type an offset nearer the one you mean. Not priced.",
        std::runtime_error);

    // A hair off the middle is no tie: the nearer chord is used.
    w.sqout_offset_ms = 0.01;
    const SqOutNote late = resolve_sqout_note(song, w);
    CHECK(late.tick == 3108);
    CHECK(late.offset_ms == doctest::Approx(93.75));
}

// A phrase chord at or before the activation was banked before Star Power
// started, so that window cannot squeeze it out, even when it sits inside the
// squeeze window around the SP end. The replay refuses the typed offset and
// warns about no such chord, as the engine never offers it.
TEST_CASE("a typed squeeze-out on a phrase banked before the activation is refused") {
    // Phrase chord at 2928, 375 ms before D = 3072; the window starts at 3000.
    const Song song = song_with({{0, false}, {768, false}, {2928, true},
                                 {3000, false}, {3072, false}});
    ReplayWindow w;
    w.act_tick = 3000;
    w.deact_tick = 3072;
    w.sqout_offset_ms = -375.0;
    CHECK_THROWS_WITH_AS(
        resolve_sqout_note(song, w),
        "window 3000:3072: the SqOut offset -375.00 ms lands on the phrase "
        "chord at tick 2928, at or before the activation at tick 3000. That "
        "phrase was banked before Star Power started, so this window cannot "
        "squeeze it out. Not priced.",
        std::runtime_error);

    // An earlier window paid the chord. The later window's SP end has it in
    // range, but cannot squeeze it out, so only the earlier window is flagged.
    ReplayWindow first;
    first.act_tick = 0;
    first.deact_tick = 2950;
    ReplayWindow second;
    second.act_tick = 3000;
    second.deact_tick = 3072;
    const ReplayResult r = replay_path(song, {first, second});
    const std::vector<std::string> warned =
        ambiguous_window_warnings(song, r, {first, second}, core::default_rules());
    REQUIRE(warned.size() == 1);
    CHECK(warned[0].rfind("window 0:2950 ", 0) == 0);
}

// D34: a phrase chord banked before the activation is never the window's to
// squeeze. Before the end the engine offers the window's newest phrase
// (D36), here the one after the banked chord. The replay accepts a typed
// squeeze-out on it, and warns about it, as the engine would squeeze it.
TEST_CASE("a typed squeeze-out on the phrase after a banked one is the engine's") {
    // Phrase chords at 2928 (banked: the activation is on 3000) and 3036,
    // 375 ms and 93.75 ms before D = 3072.
    const Song song = song_with({{0, false}, {768, false}, {2928, true},
                                 {3000, false}, {3036, true}, {3072, false}});
    ReplayWindow w;
    w.act_tick = 3000;
    w.deact_tick = 3072;
    w.sqout_offset_ms = -93.73;
    const SqOutNote n = resolve_sqout_note(song, w);
    CHECK(n.tick == 3036);

    ReplayWindow plain;
    plain.act_tick = 3000;
    plain.deact_tick = 3072;
    const ReplayResult r = replay_path(song, {plain});
    const std::vector<std::string> warned =
        ambiguous_window_warnings(song, r, {plain}, core::default_rules());
    REQUIRE(warned.size() == 1);
    CHECK(warned[0].find("tick 3036") != std::string::npos);
}

// A typed window may squeeze out its newest phrase chord at or before the SP
// end, or the first one after it (core::sqout_chords, D36). The warning names
// such a chord, and only when the window actually paid it.
TEST_CASE("the squeeze-out warning names the chord the graph would squeeze") {
    // Phrase chords 375 ms (tick 2928) and 125 ms (tick 3024) before D.
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3024, true}, {3072, false}});
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    const core::Rules& rules = core::default_rules();
    const ReplayResult r = replay_path(two, {w});
    const std::vector<std::string> warned = ambiguous_window_warnings(two, r, {w}, rules);
    REQUIRE(warned.size() == 1);
    CHECK(warned[0].find("tick 3024") != std::string::npos);  // the newest, not 2928

    // The same window off a record that ended plainly at D: its end was D,
    // so no phrase at or before D can have been squeezed out there.
    ReplayWindow stored = w;
    stored.from_record = true;
    CHECK(ambiguous_window_warnings(two, r, {stored}, rules).empty());

    // On the window's edge before D is outside (within_squeeze_window): no warning.
    const Song edge = song_with({{0, false}, {768, false}, {2880, true},
                                 {3072, false}});
    const ReplayResult re = replay_path(edge, {w});
    CHECK(ambiguous_window_warnings(edge, re, {w}, rules).empty());

    // A phrase chord one tick (2.6 ms) after D is inside the leeway, so the
    // window paid it and squeezing it out would change the score.
    const Song after = song_with({{0, false}, {768, false}, {3072, false},
                                  {3073, true}});
    const ReplayResult ra = replay_path(after, {w});
    const std::vector<std::string> late = ambiguous_window_warnings(after, ra, {w}, rules);
    REQUIRE(late.size() == 1);
    CHECK(late[0].find("tick 3073") != std::string::npos);
}

// The warning quotes what squeezing the chord out would cost, under the rules
// the replay ran with, and that cost is the whole drop, not just the phrase
// chord's share. song_with's chords are two notes (red and yellow, 50 points
// each). The phrase chord at 3024 is the fourth chord (combo 6), both notes
// at 1x, so its doubling is 50 + 50. The chord at 3072 is the fifth (combo
// 8): its first note is the ninth hit, at 1x, and its second the tenth, the
// first one paid at 2x, so its doubling is 50 + 100 = 150. Squeezing 3024
// out ends Star Power before it:
//   first_note:  3024 loses its first note's 50, 3072 loses its 150 = 200.
//   whole_chord: 3024 loses both notes' 100,     3072 loses its 150 = 250.
// Ending the window on 3024 itself leaves no later chord, so there the cost
// is the phrase chord's share alone: 50 and 100.
TEST_CASE("the squeeze-out warning quotes the whole drop under sqout_rule") {
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3024, true}, {3072, false}});
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    ReplayWindow on = w;
    on.deact_tick = 3024;

    const core::Rules& first = core::default_rules();
    REQUIRE(first.sqout_rule == core::SqOutRule::FirstNote);
    const ReplayResult r1 = replay_path(two, {w}, first);
    const std::vector<std::string> one = ambiguous_window_warnings(two, r1, {w}, first);
    REQUIRE(one.size() == 1);
    CHECK(one[0] ==
          "window 0:3072 ends just after the Star Power phrase note at tick 3024 "
          "(125.00 ms earlier) with no squeeze-out offset; if the player squeezed "
          "it out, this score is 200 points high (the same windows replayed with "
          "that squeeze-out, under sqout_rule)");
    // The same window typed with that squeeze-out really scores 200 less.
    ReplayWindow out = w;
    out.sqout_tick = 3024;
    CHECK(r1.final.total() - replay_path(two, {out}, first).final.total() == 200);

    const std::vector<std::string> one_on =
        ambiguous_window_warnings(two, replay_path(two, {on}, first), {on}, first);
    REQUIRE(one_on.size() == 1);
    CHECK(one_on[0].find("this score is 50 points high") != std::string::npos);

    core::Rules whole = core::default_rules();
    whole.sqout_rule = core::SqOutRule::WholeChord;
    const std::vector<std::string> both =
        ambiguous_window_warnings(two, replay_path(two, {w}, whole), {w}, whole);
    REQUIRE(both.size() == 1);
    CHECK(both[0].find("this score is 250 points high") != std::string::npos);

    const std::vector<std::string> both_on =
        ambiguous_window_warnings(two, replay_path(two, {on}, whole), {on}, whole);
    REQUIRE(both_on.size() == 1);
    CHECK(both_on[0].find("this score is 100 points high") != std::string::npos);
}

// The phrase chords strictly inside an SP end's squeeze window, in chart
// order: the list the graph puts on each deactivation edge.
TEST_CASE("squeeze_window_phrases: the phrase chords strictly inside the window, in chart order") {
    auto ticks = [](const std::vector<const SongTimestamp*>& w) {
        std::vector<int64_t> out;
        for (const SongTimestamp* c : w) out.push_back(c->timecode.ticks());
        return out;
    };
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3036, true}, {3072, false}});
    // 375 ms before D comes before 93.75 ms.
    CHECK(ticks(core::squeeze_window_phrases(two, two.timecode(3072))) ==
          std::vector<int64_t>{2928, 3036});

    // On the window's edge before D is outside (within_squeeze_window).
    const Song edge = song_with({{0, false}, {2880, true}, {3072, false}});
    CHECK(core::squeeze_window_phrases(edge, edge.timecode(3072)).empty());

    // Only a phrase chord after D: that one.
    const Song after = song_with({{0, false}, {3072, false}, {3073, true}});
    CHECK(ticks(core::squeeze_window_phrases(after, after.timecode(3072))) ==
          std::vector<int64_t>{3073});

    // An SP end between two chords, and no phrase chord at all.
    CHECK(ticks(core::squeeze_window_phrases(two, two.timecode(3000))) ==
          std::vector<int64_t>{2928, 3036});
    const Song bare = song_with({{0, false}, {3072, false}});
    CHECK(core::squeeze_window_phrases(bare, bare.timecode(3072)).empty());
}

// The one rule for which phrase an SP end offers a window (D36), on plain
// ticks: the newest step's phrase when that step moved the end from this SP
// end and the window has not squeezed it in (D34); else, when the window's
// end is this SP end, the first phrase after it not squeezed in; else
// nothing. A step that moved the end from here while the list lacks its
// phrase is an impossible state, and throws.
TEST_CASE("offered_phrase: the newest step's phrase early, the first unsqueezed one late") {
    const std::vector<int64_t> window = {2928, 3036, 3100, 3200};  // the end is 3072
    const auto tick = [](int64_t t) { return t; };
    const auto none = [](int64_t) { return false; };
    const auto in_3100 = [](int64_t t) { return t == 3100; };
    auto at = [&](std::optional<int64_t> path_end, int64_t newest, std::optional<int64_t> from,
                  auto squeezed_in) {
        const auto it = core::offered_phrase(window.begin(), window.end(), 3072, path_end,
                                             newest, from, tick, squeezed_in);
        return it == window.end() ? int64_t{-1} : *it;
    };
    CHECK(at(4000, 3036, 3072, none) == 3036);          // newest moved from here
    CHECK(at(4000, 2928, 3072, none) == 2928);          // whichever it is
    CHECK(at(4000, 3036, 3071, none) == -1);            // moved from a twin end
    CHECK(at(4000, 3036, std::nullopt, none) == -1);    // no squeeze node
    CHECK(at(4000, 3036, 3072, [](int64_t t) { return t == 3036; }) == -1);  // squeezed in
    CHECK(at(3072, 3036, std::nullopt, none) == 3100);  // the end is here: late
    CHECK(at(3072, 3036, std::nullopt, in_3100) == 3200);
    CHECK(at(3072, 0, std::nullopt, [](int64_t) { return true; }) == -1);
    CHECK_THROWS_AS(at(4000, 3000, 3072, none), std::logic_error);  // 3000 is not listed
}

// The replay's form of the rule (core::sqout_chords): a typed window has no
// history, so either side; a stored window that ended plainly, only late.
// D36.
TEST_CASE("sqout_chords: a window's newest phrase at or before D, or the first after D") {
    const Song s = song_with({{0, false}, {768, false}, {2928, true}, {3000, false},
                              {3036, true}, {3072, false}, {3100, true}, {3200, true}});
    const Timecode d = s.timecode(3072);
    auto ticks = [&](int64_t act, const std::vector<int64_t>& in, bool plain) {
        std::vector<int64_t> out;
        for (const SongTimestamp* c : core::sqout_chords(s, d, act, in, plain))
            out.push_back(c->timecode.ticks());
        return out;
    };
    using V = std::vector<int64_t>;
    CHECK(ticks(0, {}, false) == V{3036, 3100});      // 3036 is newest, never 2928
    CHECK(ticks(3000, {}, false) == V{3036, 3100});   // 2928 banked, 3036 still newest
    CHECK(ticks(3036, {}, false) == V{3100});         // both banked: late only
    CHECK(ticks(0, {3036}, false) == V{3100});        // the newest squeezed in
    CHECK(ticks(0, {3100}, false) == V{3036, 3200});  // the late one squeezed in
    CHECK(ticks(0, {}, true) == V{3100});             // ended plainly at D
    CHECK(ticks(0, {3036, 3100, 3200}, false).empty());
}

TEST_CASE("category_scores reports the multiplier each note was paid at") {
    Chord one;
    one.add_note(NoteColor::Red);
    CHECK(category_scores(one, 8).multiplier == 1);  // the 9th note: 1x
    CHECK(category_scores(one, 9).multiplier == 2);  // the 10th note: 2x
    CHECK(category_scores(one, 19).multiplier == 3);
    CHECK(category_scores(one, 29).multiplier == 4);
    CHECK(category_scores(one, 40).multiplier == 4);
    CHECK(category_scores(one, 9).multiplier_after == 2);

    // A two-note chord after 8 notes of combo straddles the step: its first
    // note is the 9th (1x) and its second the 10th (2x).
    Chord two;
    two.add_note(NoteColor::Red);
    two.add_note(NoteColor::Kick);
    std::vector<CategoryScores> per_note;
    const CategoryScores s = category_scores(two, 8, &per_note);
    CHECK(s.multiplier == 1);
    CHECK(s.multiplier_after == 2);
    CHECK(s.combo == 50);  // only the second note earns combo points
    REQUIRE(per_note.size() == 2);
    CHECK(per_note[0].multiplier == 1);
    CHECK(per_note[1].multiplier == 2);
}

TEST_CASE("category_scores prices a dynamic cymbal's bonus at its own note") {
    // A kick and a ghost cymbal after 9 notes of combo: both notes are at 2x.
    // The cymbal's dynamics are the pad's 50 plus the cymbal's 15, times 2.
    // The chord's ghost category holds only the raw 50.
    Chord chord;
    chord.add_note(NoteColor::Kick);
    add_dynamic_cymbal(chord, NoteDynamicType::Ghost);
    std::vector<CategoryScores> per_note;
    const CategoryScores s = category_scores(chord, 9, &per_note);
    CHECK(s.ghost == 50);

    const std::vector<ChordNote> order = chord.notes(true);
    REQUIRE(order.size() == 2);
    REQUIRE(per_note.size() == 2);
    for (size_t k = 0; k < order.size(); ++k) {
        if (order[k].colortype == NoteColor::Yellow) {
            CHECK(per_note[k].dynamics_bonus == 130);  // (50 + 15) x 2
            // Without its dynamics the cymbal pays a plain 2x cymbal: 130.
            CHECK(per_note[k].sp - per_note[k].dynamics_bonus == 130);
        } else {
            CHECK(per_note[k].dynamics_bonus == 0);
        }
    }
}

TEST_CASE("replay reports the multipliers category_scores applied") {
    // Eight Red singles, a Red+Kick chord that straddles 10, then eleven more
    // singles so a later chord crosses 20 on its own.
    std::vector<std::vector<NoteColor>> chords(8, {NoteColor::Red});
    chords.push_back({NoteColor::Red, NoteColor::Kick});
    for (int i = 0; i < 11; ++i) chords.push_back({NoteColor::Red});
    const ReplayResult r = replay_path(make_chord_song(chords), {});
    REQUIRE(r.chords.size() == 20);

    // The straddling chord: first note 1x, second note 2x.
    const ReplayChord& straddle = r.chords[8];
    CHECK(straddle.combo_before == 8);
    CHECK(straddle.multiplier == 1);
    CHECK(straddle.multiplier_after == 2);
    CHECK(straddle.points.combo == 50);
    REQUIRE(straddle.notes.size() == 2);
    CHECK(straddle.notes[0].multiplier == 1);
    CHECK(straddle.notes[1].multiplier == 2);

    // The chord before it leaves the disc at 1x.
    CHECK(r.chords[7].multiplier_after == 1);

    // A single note that is the 20th: paid at 3x.
    const ReplayChord& third = r.chords[18];
    CHECK(third.combo_before == 19);
    CHECK(third.multiplier == 3);
    CHECK(third.multiplier_after == 3);
    CHECK(third.notes[0].multiplier == 3);
    CHECK(third.points.combo == 100);  // 50 base x (3 - 1)
    CHECK(r.chords[17].multiplier_after == 2);
}

TEST_CASE("replay copies each note's dynamics bonus") {
    Song song = make_chord_song({{NoteColor::Red}});
    add_dynamic_cymbal(song.sequence[0].chord, NoteDynamicType::Accent);
    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords.size() == 1);
    REQUIRE(r.chords[0].notes.size() == 2);
    for (const ReplayNote& n : r.chords[0].notes)
        CHECK(n.dynamics_bonus == (n.color == NoteColor::Yellow ? 65 : 0));
    // The accent category holds only the pad part; the note field holds all.
    CHECK(r.chords[0].points.accent == 50);
}

TEST_CASE("replay names each note's dynamic") {
    // A kick, a ghost Red pad and an accent Yellow cymbal in one chord: each
    // note reports its own kind, so a mixed chord can be worded per note.
    Song song = make_chord_song({{NoteColor::Kick}});
    Chord& chord = song.sequence[0].chord;
    chord.add_note(NoteColor::Red).dynamictype = NoteDynamicType::Ghost;
    add_dynamic_cymbal(chord, NoteDynamicType::Accent);
    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords.size() == 1);
    REQUIRE(r.chords[0].notes.size() == 3);
    for (const ReplayNote& n : r.chords[0].notes) {
        if (n.color == NoteColor::Red)
            CHECK(n.dynamic == NoteDynamicType::Ghost);
        else if (n.color == NoteColor::Yellow)
            CHECK(n.dynamic == NoteDynamicType::Accent);
        else
            CHECK(n.dynamic == NoteDynamicType::Normal);
    }
    CHECK(dynamic_str(NoteDynamicType::Ghost) == "ghost");
    CHECK(dynamic_str(NoteDynamicType::Accent) == "accent");
    CHECK(dynamic_str(NoteDynamicType::Normal) == "none");
}

// The chord-score table on ten hand-built chords: seven Red singles, then a
// Kick + Red + accent Yellow cymbal that crosses a step of to_multiplier
// (core/timing.h), a Kick +
// ghost Red, and a plain Red + Yellow inside a solo. Every value is a literal
// pinned from one run.
TEST_CASE("chord_score_table: literal rows on a hand-built chart") {
    std::vector<std::vector<NoteColor>> chords(7, {NoteColor::Red});
    chords.push_back({NoteColor::Kick, NoteColor::Red});
    chords.push_back({NoteColor::Kick});
    chords.push_back({NoteColor::Red, NoteColor::Yellow});
    Song song = make_chord_song(chords);
    add_dynamic_cymbal(song.sequence[7].chord, NoteDynamicType::Accent);
    song.sequence[8].chord.add_note(NoteColor::Red).dynamictype = NoteDynamicType::Ghost;
    song.sequence[9].flag_solo = true;

    const ChordScoreTable table =
        chord_score_table(song, core::SqOutRule::FirstNote, ChordScoreDetail::WithNotes);
    REQUIRE(table.rows.size() == 10);
    REQUIRE(table.notes.size() == 14);

    // The straddle: its first two notes are paid below the step and the
    // accent cymbal (the dearest, so last) above it.
    const ChordScoreRow& straddle = table.rows[7];
    CHECK(straddle.notes == 180);
    CHECK(straddle.combo == 130);
    CHECK(straddle.sp == 360);
    CHECK(straddle.sqout_sp == 310);
    CHECK(straddle.accent == 50);
    CHECK(straddle.ghost == 0);
    CHECK(straddle.solo == 0);
    CHECK(straddle.multiplier == 1);
    CHECK(straddle.multiplier_after == 2);
    CHECK(straddle.combo_before == 7);
    CHECK(straddle.combo_after == 10);
    CHECK(straddle.note_begin == 7);
    CHECK(straddle.note_end == 10);
    CHECK(table.notes[7].multiplier == 1);
    CHECK(table.notes[8].multiplier == 1);
    CHECK(table.notes[9].multiplier == 2);
    CHECK(table.notes[9].dynamics_bonus == 130);

    const ChordScoreRow& ghost = table.rows[8];
    CHECK(ghost.notes == 100);
    CHECK(ghost.combo == 150);
    CHECK(ghost.sp == 300);
    CHECK(ghost.sqout_sp == 200);
    CHECK(ghost.accent == 0);
    CHECK(ghost.ghost == 50);
    CHECK(ghost.solo == 0);
    CHECK(ghost.multiplier == 2);
    CHECK(ghost.multiplier_after == 2);
    CHECK(ghost.combo_before == 10);
    CHECK(ghost.combo_after == 12);
    CHECK(ghost.note_begin == 10);
    CHECK(ghost.note_end == 12);

    const ChordScoreRow& solo = table.rows[9];
    CHECK(solo.notes == 100);
    CHECK(solo.combo == 100);
    CHECK(solo.sp == 200);
    CHECK(solo.sqout_sp == 100);
    CHECK(solo.accent == 0);
    CHECK(solo.ghost == 0);
    CHECK(solo.solo == 200);
    CHECK(solo.multiplier == 2);
    CHECK(solo.multiplier_after == 2);
    CHECK(solo.combo_before == 12);
    CHECK(solo.combo_after == 14);
    CHECK(solo.note_begin == 12);
    CHECK(solo.note_end == 14);

    // Without the notes, every chord row's points are the same and every
    // note range is empty.
    const ChordScoreTable bare = chord_score_table(song, core::SqOutRule::FirstNote);
    REQUIRE(bare.rows.size() == 10);
    CHECK(bare.notes.empty());
    for (size_t i = 0; i < bare.rows.size(); ++i) {
        CHECK(bare.rows[i].sp == table.rows[i].sp);
        CHECK(bare.rows[i].combo_after == table.rows[i].combo_after);
        CHECK(bare.rows[i].note_begin == bare.rows[i].note_end);
    }
}

TEST_CASE("replay multipliers agree with the combo on every corpus chord") {
    int charts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        ++charts;
        const ReplayResult r = replay_path(song, {});
        INFO(path);
        for (size_t i = 0; i < r.chords.size(); ++i) {
            const ReplayChord& c = r.chords[i];
            REQUIRE_FALSE(c.notes.empty());
            // The chord's multiplier is its first note's; multiplier_after is
            // its last note's.
            CHECK(c.multiplier == c.notes.front().multiplier);
            CHECK(c.multiplier_after == c.notes.back().multiplier);
            // What the disc shows after this chord is what the next chord
            // starts from.
            if (i + 1 < r.chords.size())
                CHECK(c.multiplier_after == to_multiplier(r.chords[i + 1].combo_before));
            // A note's dynamics bonus is part of what it pays, never more.
            for (const ReplayNote& n : c.notes) {
                CHECK(n.dynamics_bonus >= 0);
                CHECK(n.dynamics_bonus < n.sp_points);
            }
        }
    }
    CHECK(charts > 0);
}

TEST_CASE("replay score fields: one list in schema order") {
    REQUIRE(std::size(kReplayScoreFields) == 6);
    const char* want[] = {"notes", "combo", "sp", "solo", "accent", "ghost"};
    for (size_t i = 0; i < 6; ++i) CHECK(std::string(kReplayScoreFields[i].name) == want[i]);

    ReplayScore s;
    s.notes = 1; s.combo = 2; s.sp = 3; s.solo = 4; s.accent = 5; s.ghost = 6;
    for (size_t i = 0; i < 6; ++i)
        CHECK(s.*(kReplayScoreFields[i].member) == static_cast<int64_t>(i + 1));
}

TEST_CASE("assign_score and score_of are each other's reverse") {
    ReplayScore s;
    s.notes = 1; s.combo = 2; s.sp = 3; s.solo = 4; s.accent = 5; s.ghost = 6;
    Path p;
    assign_score(p, s);
    CHECK(p.notes_points == 1);
    CHECK(p.score_combo == 2);
    CHECK(p.score_sp == 3);
    CHECK(p.score_solo == 4);
    CHECK(p.score_accents == 5);
    CHECK(p.score_ghosts == 6);
    CHECK(score_of(p) == s);
}

TEST_CASE("replay: scores_only leaves chord_code and notes empty and every score the same") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    ReplayOptions scores;
    scores.scores_only = true;
    int runs = 0;
    std::string first_diff;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.reading());
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            ++runs;
            const PathReplay full = replay_stored_path(song, *p, cfg.rules);
            const PathReplay lean = replay_stored_path(song, *p, cfg.rules, scores);
            CHECK(lean.faithful() == full.faithful());
            const std::string d = first_result_difference(full.result, lean.result, true);
            if (!d.empty() && first_diff.empty()) first_diff = path + ": " + d;
            for (const ReplayChord& c : lean.result.chords) {
                if (!c.chord_code.empty() || !c.notes.empty()) {
                    if (first_diff.empty()) first_diff = path + ": a scores-only row kept its notes";
                    break;
                }
            }
        }
    }
    CHECK(runs > 0);
    INFO("first difference: " << first_diff);
    CHECK(first_diff.empty());
}
