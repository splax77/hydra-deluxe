// Replay invariants: core/replay.h must price a path exactly as the engine
// priced it.
//
// The engine sums scores along graph edges and never looks at a chord; the
// replay walks chords and never looks at the graph. If the two agree on all
// six score categories, for every path of every corpus chart, then the
// replay's three rules (one SP-free combo counter, an inclusive SP window
// with a 3 ms backend leeway, undoubled solos) are the engine's rules.

#include "doctest.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
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
#include "env_util.h"
#include "replay_json.h"
#include "core/scoring.h"
#include "core/sqout_chord.h"
#include "core/timing.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "core/model.h"  // within_squeeze_window, the horizon the warning uses
#include "search/pather.h"
#include "store/record_store.h"

using namespace hydra;
using json = nlohmann::json;

TEST_CASE("replay reproduces the engine's score for every corpus path") {
    int charts = 0, paths = 0, mismatches = 0;
    std::string first_diff;

    // The GUI's defaults, straight from app::Settings rather than five
    // hand-written literals: cap 4, score range 4, 10 ms.
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        ++charts;

        const HydraRecord& rec = corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            ++paths;
            const PathReplay pr = replay_stored_path(song, *p);
            REQUIRE(pr.all_windows());

            if (!pr.totals_match()) {
                ++mismatches;
                if (first_diff.empty())
                    first_diff = path + " [" + p->pathstring() + "]: replay " +
                                 std::to_string(pr.result.final.total()) + " vs stored " +
                                 std::to_string(p->totalscore());
            }
        }
    }

    CHECK(charts > 0);
    CHECK(paths > 0);
    INFO("first mismatch: " << first_diff);
    CHECK(mismatches == 0);
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
// chord. The chart is test_song.cpp's two solos split by one plain chord.
TEST_CASE("replay: a solo's bonus reaches the on-screen total on the section's last chord") {
    const Song song = load_songbytes_chart(
        testchart::chart_bytes(testchart::section("ExpertDrums",
                                                  "  0 = E solo\n  0 = N 1 0\n"
                                                  "  192 = N 2 0\n  192 = E soloend\n"
                                                  "  384 = N 3 0\n"
                                                  "  576 = E solo\n  576 = N 4 0\n"
                                                  "  576 = E soloend\n")),
        true, true);
    REQUIRE(song.solo_sections.size() == 2);
    REQUIRE(song.solo_sections[0].first == 0);
    REQUIRE(song.solo_sections[0].last == 1);
    REQUIRE(song.solo_sections[1].first == 3);
    REQUIRE(song.solo_sections[1].last == 3);

    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords.size() == 4);
    REQUIRE(r.chords[0].points.solo > 0);
    // Inside the first section: its own bonus is held back.
    CHECK(r.chords[0].cum_onscreen_total == r.chords[0].cum.total() - r.chords[0].points.solo);
    // Each section's last chord, and the plain chord between them, show it all.
    CHECK(r.chords[1].cum_onscreen_total == r.chords[1].cum.total());
    CHECK(r.chords[2].points.solo == 0);
    CHECK(r.chords[2].cum_onscreen_total == r.chords[2].cum.total());
    CHECK(r.chords[3].cum_onscreen_total == r.chords[3].cum.total());
}

// A targeted search is only useful if it gives back the same path the ordinary
// search would have found. So take every path the ordinary search DID find,
// hand its activation ticks back to search_target, and require the engine to
// price it identically -- same total, same deactivation node per activation,
// same squeezes. That is the whole contract: "activate exactly here" must not
// change how the engine scores what happens next.
TEST_CASE("targeted search reproduces every corpus path") {
    int charts = 0, paths = 0, mismatches = 0;
    std::string first_diff;

    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        ++charts;

        const HydraRecord& rec = corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            ++paths;
            const std::vector<Activation> want_acts = p->all_activations();

            const std::vector<Path> got = search_target(song, cfg, test::act_ticks(*p));

            // Somewhere in the returned variants must be this exact path.
            const Path* match = nullptr;
            const std::vector<const Path*> targeted = flatten_paths(got);
            for (const Path* q : targeted) {
                if (q->totalscore() != p->totalscore()) continue;
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

            if (!match) {
                ++mismatches;
                if (first_diff.empty())
                    first_diff = path + " [" + p->pathstring() + "] score " +
                                 std::to_string(p->totalscore()) + ": " +
                                 std::to_string(targeted.size()) +
                                 " targeted path(s), none matching";
                continue;
            }

            // The recovered path also has to replay to its own score, which is
            // the invariant the first test pins for search-found paths.
            CHECK(replay_stored_path(song, *match).totals_match());
        }
    }

    CHECK(charts > 0);
    REQUIRE(paths >= 300);
    INFO("first mismatch: " << first_diff);
    CHECK(mismatches == 0);
}

namespace {

struct TiedVariantCount {
    int variants = 0;  // tied variants the analysis listed
    int compared = 0;  // of those, the ones a lone search could price
    int differing = 0; // of those, the ones whose stored facts differ
    int skipped() const { return variants - compared; }
};

// Decision D3 for one set of analysis settings, over the whole corpus. Each
// variant is priced alone with a targeted search, and its stored facts must
// equal a lone root's (test::lone_pricing in record_fixtures.h: the score
// and every window fact, early-fill facts included). All-zero variants are
// not visited.
//
// A variant the lone search folded under a root of its own has no oracle,
// so it is skipped, and each skip is printed. The caller pins how many there
// are, so a regression that turns a compared variant into a skipped one
// fails instead of passing quietly.
TiedVariantCount check_tied_variants(const app::AnalysisSettings& cfg) {
    TiedVariantCount n;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song =
            corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty, cfg.rules);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            if (!p->var_point) continue;
            ++n.variants;
            const test::LonePricing lone = test::lone_pricing(song, cfg, *p);
            if (!lone.diff.empty() && lone.tied_under_root) {
                MESSAGE("skipped " << path << ": " << lone.diff);
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
// facts are its own. The fixtures in test_search.cpp prove the mechanism on
// purpose-built charts; this guard catches any later case they do not shape.
//
// The app's defaults (cap 4, score range 4) hold few ties, and none of them
// folded while SP ran or banked bars at different ticks from its leader. So
// the guard also runs at score range 40, at cap 4 and at cap 2. Those two
// hold hundreds of variants, and with either D3 fix taken out of the engine
// dozens of them store their leader's facts instead of their own.
TEST_CASE("every tied variant stores what a search pricing it alone stores") {
    // `skipped` is pinned exactly: the variants with no lone root to compare
    // against. Each is printed with its reason. If the count moves, read
    // those lines before changing it.
    //
    // `min_variants` is a floor, not a pin: today the three settings list 12,
    // 295 and 303 variants. A change that stopped listing most ties would
    // still pass every other check here (fewer variants, none skipped, none
    // differ), so the floor is what catches it.
    struct Setting { int cap; int depth; int skipped; int min_variants; };
    for (const Setting s : {Setting{4, 4, 0, 10}, Setting{4, 40, 0, 200},
                            Setting{2, 40, 4, 200}}) {
        app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
        cfg.sp_cap = s.cap;
        cfg.depth_value = s.depth;
        INFO("cap " << s.cap << ", score range " << s.depth);
        const TiedVariantCount n = check_tied_variants(cfg);
        CHECK(n.variants >= s.min_variants);
        CHECK(n.compared > 0);
        CHECK(n.differing == 0);
        CHECK(n.skipped() == s.skipped);
        MESSAGE("cap " << s.cap << ", score range " << s.depth << ": compared " << n.compared
                       << " of " << n.variants << " variants against a lone search, "
                       << n.differing << " differ, " << n.skipped() << " skipped");
    }
}

// A tick that is not an activation fill cannot be honoured, and the engine says
// so by giving back nothing rather than quietly pricing a different path.
TEST_CASE("targeted search rejects a tick that is not a fill") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        if (rec.paths.empty()) continue;
        const std::vector<Activation> acts = rec.best_path().all_activations();
        if (acts.empty()) continue;

        // One tick past a real activation fill: the fill node lives on the
        // tick itself, so tick + 1 is never one.
        const int64_t bogus = acts[0].timecode.ticks() + 1;
        CHECK(search_target(song, cfg, {bogus}).empty());
        return;
    }
    FAIL("no corpus chart with an activation to build the negative case from");
}

TEST_CASE("replay without Star Power scores no doubling at all") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), true, true);
    REQUIRE_FALSE(song.is_empty());

    const ReplayResult r = replay_path(song, {});
    CHECK(r.final.sp == 0);
    CHECK(r.final.base > 0);
    CHECK(r.chords.size() == song.sequence.size());

    // The running totals are a prefix sum of the per-chord points, and the
    // on-screen total only ever lags the real one (a solo's bonus is withheld
    // until the run ends, never paid early).
    ReplayScore running;
    for (const ReplayChord& c : r.chords) {
        running.add(c.points);
        CHECK(c.cum.total() == running.total());
        CHECK(c.cum_onscreen_total <= c.cum.total());
    }
    CHECK(r.final.total() == running.total());
    CHECK(r.chords.back().cum_onscreen_total == r.final.total());
}

// Round and Round's shape on a hand-built chart: a window whose squeeze-out
// sits on an R+Y phrase chord ~479 ms past the SP end. That chord is outside
// the window and outside the leeway, so it earns no doubling at all -- the
// squeeze-out changes nothing. The chord on the deactivation node is paid.
TEST_CASE("a squeezed-out chord past the leeway earns nothing") {
    // 4/4, 120 BPM, 192 ticks per beat: 768 ticks and 2000 ms per measure.
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3256}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        if (tick == 3256) test::mark_phrase_end(ts, tick, song.tick_resolution());
        song.sequence.push_back(ts);
    }

    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    w.sqout_offset_ms = offset_from_sp_end(song.timecode(3256).ms(), song.timecode(3072).ms());
    w.sqout_tick = 3256;

    const ReplayResult r = replay_path(song, {w});
    REQUIRE(r.chords.size() == 6);
    CHECK(r.chords[4].in_sp);  // on the deactivation node: paid in full
    CHECK(r.chords[4].points.sp > 0);
    CHECK_FALSE(r.chords[5].in_sp);
    CHECK(r.chords[5].points.sp == 0);

    // The disc follows the same decision: doubled on the paid chord only.
    CHECK(r.chords[4].multiplier_shown == r.chords[4].multiplier_after * kStarPowerMultiplier);
    CHECK(r.chords[5].multiplier_shown == r.chords[5].multiplier_after);
}

TEST_CASE("paid_by_sp: yes exactly when the row's SP value is above zero") {
    using core::SqOutPosition;
    CHECK(core::paid_by_sp(-10.0, 100, 0, SqOutPosition::NoSqOut, 3.0));
    CHECK_FALSE(core::paid_by_sp(-10.0, 100, 0, SqOutPosition::Exact, 3.0));
    CHECK(core::paid_by_sp(-10.0, 100, 50, SqOutPosition::Exact, 3.0));
    CHECK_FALSE(core::paid_by_sp(-10.0, 100, 50, SqOutPosition::After, 3.0));
    CHECK(core::paid_by_sp(2.0, 100, 50, SqOutPosition::NoSqOut, 3.0));        // in the leeway
    CHECK_FALSE(core::paid_by_sp(4.0, 100, 50, SqOutPosition::NoSqOut, 3.0));  // past it
}

// D2: the disc doubles only when Star Power paid the chord something.
TEST_CASE("replay: a squeezed-out chord SP pays nothing shows the plain multiplier") {
    // 120 BPM, 192 ticks per beat: 96 ticks are 250 ms.
    auto build = [](bool two_notes) {
        Song song(192);
        song.bpm_changes[0] = 120.0;
        song.build_timing();
        for (int64_t tick : {0, 768, 1536, 2304, 2976, 3072}) {
            SongTimestamp ts;
            ts.timecode = song.timecode(tick);
            ts.chord.add_note(NoteColor::Red);
            if (tick != 2976 || two_notes) ts.chord.add_note(NoteColor::Yellow);
            if (tick == 2976) test::mark_phrase_end(ts, tick, song.tick_resolution());
            song.sequence.push_back(ts);
        }
        return song;
    };
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    w.sqout_tick = 2976;  // a phrase chord 250 ms before the SP end

    const Song one = build(false);
    const ReplayResult r = replay_path(one, {w});
    REQUIRE(r.chords.size() == 6);
    CHECK(r.chords[3].in_sp);  // an ordinary chord inside the window: doubled
    CHECK(r.chords[3].multiplier_shown == r.chords[3].multiplier_after * kStarPowerMultiplier);
    CHECK(r.chords[4].points.sp == 0);  // first-note rule: one note loses all its doubling
    CHECK_FALSE(r.chords[4].in_sp);
    CHECK(r.chords[4].multiplier_shown == r.chords[4].multiplier_after);

    // Two notes: only the first note's share is lost, so SP still paid it.
    const Song two = build(true);
    const ReplayResult r2 = replay_path(two, {w});
    CHECK(r2.chords[4].points.sp > 0);
    CHECK(r2.chords[4].in_sp);
    CHECK(r2.chords[4].multiplier_shown == r2.chords[4].multiplier_after * kStarPowerMultiplier);
}

// ambiguous_window_warnings reads in_sp, so D2 changed it a little: a chord that
// no window pays anything for is no longer "paid" just because a squeezed-out
// window reached it. Here window A has no squeeze-out offset and ends on tick
// 768; the phrase chord 250 ms later is one note, and only window B (which
// squeezes it out, so it pays 0) reaches it. A pays nothing for it, so there is
// nothing for the warning to say. The old every-window gate flagged it.
TEST_CASE("replay: the squeeze-out warning ignores a chord only a zero-paying window reached") {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 864, 1536}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);  // one note: a squeeze-out loses all its doubling
        if (tick == 864) test::mark_phrase_end(ts, tick, song.tick_resolution());
        song.sequence.push_back(ts);
    }
    ReplayWindow a;  // no squeeze-out offset: the warning looks at it
    a.act_tick = 0;
    a.deact_tick = 768;
    ReplayWindow b;  // squeezes the chord at 864 out
    b.act_tick = 864;
    b.deact_tick = 1536;
    b.sqout_tick = 864;

    const ReplayResult r = replay_path(song, {a, b});
    REQUIRE(r.chords.size() == 4);
    CHECK(r.chords[2].points.sp == 0);
    CHECK_FALSE(r.chords[2].in_sp);
    CHECK(ambiguous_window_warnings(song, r, {a, b}, core::default_rules()).empty());

    // With no squeezed-out window the chord still is not paid (250 ms is past
    // the leeway), so the answer is the same.
    const ReplayResult alone = replay_path(song, {a});
    CHECK(ambiguous_window_warnings(song, alone, {a}, core::default_rules()).empty());
}

TEST_CASE("shown_multiplier doubles the combo multiplier only under Star Power") {
    CHECK(shown_multiplier(1, false) == 1);
    CHECK(shown_multiplier(1, true) == 2);
    CHECK(shown_multiplier(4, false) == 4);
    CHECK(shown_multiplier(4, true) == 8);
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
    const json path = json::parse(R"({
      "index": 0,
      "activations": [
        {"act_tick": 480, "deact_tick": 3840, "sqinouts": []},
        {"act_tick": 7680, "deact_tick": 11520, "sqout_tick": 11532,
         "sqinouts": [{"kind": "SqIn",  "offset_ms": 12.5},
                      {"kind": "SqOut", "offset_ms": 31.25}]},
        {"act_tick": 7680, "deact_tick": 11520,
         "sqinouts": [{"kind": "SqIn",  "offset_ms": 12.5},
                      {"kind": "SqOut", "offset_ms": 31.25}]}
      ]})");

    const std::vector<ReplayWindow> w = windows_from_json(path);
    REQUIRE(w.size() == 3);
    CHECK(w[0].act_tick == 480);
    CHECK(w[0].deact_tick == 3840);
    CHECK_FALSE(w[0].sqout_offset_ms.has_value());
    CHECK_FALSE(w[0].sqout_tick.has_value());

    CHECK(w[1].act_tick == 7680);
    CHECK(w[1].deact_tick == 11520);
    REQUIRE(w[1].sqout_tick.has_value());
    CHECK(*w[1].sqout_tick == 11532);
    CHECK_FALSE(w[1].sqout_offset_ms.has_value());

    // An older dump with no tick: the offset is what names the chord. The
    // SqIn sits in the same list and must not be mistaken for the SqOut.
    CHECK_FALSE(w[2].sqout_tick.has_value());
    REQUIRE(w[2].sqout_offset_ms.has_value());
    CHECK(*w[2].sqout_offset_ms == doctest::Approx(31.25));

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

    int checked = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        if (rec.paths.empty()) continue;

        const std::vector<const Path*> all = rec.all_paths();
        const json dumped = paths_json(all, song.timing());
        REQUIRE(dumped.size() == all.size());
        for (size_t k = 0; k < all.size(); ++k) {
            const Path* p = all[k];
            const std::vector<ReplayWindow> want = windows_for_path(*p);
            if (want.empty()) continue;

            // The real dump producer, so a format change on either side fails.
            const std::vector<ReplayWindow> got = windows_from_json(dumped[k]);
            REQUIRE(got.size() == want.size());
            for (size_t i = 0; i < got.size(); ++i) {
                CHECK(got[i].act_tick == want[i].act_tick);
                CHECK(got[i].deact_tick == want[i].deact_tick);
                CHECK(got[i].sqout_tick == want[i].sqout_tick);
                // read_sqout (tools/replay_json.cpp) decides when the offset
                // is read; today's records always stamp a tick.
                if (!want[i].sqout_tick)
                    CHECK(got[i].sqout_offset_ms == want[i].sqout_offset_ms);
            }
            ++checked;
        }
        if (checked > 0) break;  // one chart's paths are the whole contract
    }
    CHECK(checked > 0);
}

// D34: a window never squeezes out a phrase it already squeezed in, and the
// replay's squeeze-out lookup and warnings skip those phrases
// (ReplayWindow::sqin_ticks). `score --path <dump>` reads its windows from the
// dump, so the dump has to carry each window's SqIn phrases. Without them the
// replay fell back to "the window's first phrase": on spent_then_next.chart at
// cap 2, path '0++' warned that 17360 might have been squeezed out, though it
// was squeezed in. A dump written before the field reads as no SqIns, as
// before.
TEST_CASE("a dump carries each window's SqIn phrases back to the replay") {
    const Song song = load_songpath(
        std::string(HYDRA_INPUT_DIR) + "/test_fast_tempo/spent_then_next.chart", true, true);
    const HydraRecord rec = analyze_chart(song, test::scores_settings(2));
    const std::vector<const Path*> all = rec.all_paths();
    const json dumped = paths_json(all, song.timing());
    REQUIRE(dumped.size() == all.size());
    int with_sqins = 0;
    bool saw_0pp = false;
    for (size_t k = 0; k < all.size(); ++k) {
        CAPTURE(all[k]->pathstring());
        const std::vector<ReplayWindow> want = windows_for_path(*all[k]);
        const std::vector<ReplayWindow> got = windows_from_json(dumped[k]);
        REQUIRE(got.size() == want.size());
        for (size_t i = 0; i < got.size(); ++i) {
            CHECK(got[i].sqin_ticks == want[i].sqin_ticks);
            with_sqins += want[i].sqin_ticks.empty() ? 0 : 1;
        }
        if (all[k]->pathstring() != "0++") continue;
        saw_0pp = true;
        // Its last window squeezed 17360 in, so no warning may name it.
        for (const std::string& warn :
             ambiguous_window_warnings(song, replay_path(song, got), got,
                                       core::default_rules()))
            CHECK_MESSAGE(warn.find("17360") == std::string::npos, warn);
    }
    CHECK(with_sqins > 0);
    CHECK(saw_0pp);

    // An older dump, without the key, still reads: no SqIn phrases.
    const std::vector<ReplayWindow> old = windows_from_json(json::parse(
        R"({"activations": [{"act_tick": 480, "deact_tick": 3840, "sqinouts": []}]})"));
    REQUIRE(old.size() == 1);
    CHECK(old[0].sqin_ticks.empty());
}

// fcvideo and `hydra_replay score --path` read these fields out of a dump. A
// dump-format change that drops one has to fail here, not in the video tools.
TEST_CASE("paths_json writes every field the dump readers use") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    bool checked = false;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        const std::vector<const Path*> all = rec.all_paths();
        if (all.empty() || all[0]->all_activations().empty()) continue;

        const json dumped = paths_json(all, song.timing());
        REQUIRE(dumped.size() == all.size());

        const json& p0 = dumped[0];
        for (const char* k : {"index", "pathstring", "total", "score", "activations"})
            CHECK_MESSAGE(p0.contains(k), k);
        for (const char* k : {"base", "combo", "sp", "solo", "accent", "ghost"})
            CHECK_MESSAGE(p0["score"].contains(k), k);
        CHECK(p0["pathstring"].get<std::string>() == all[0]->pathstring());

        REQUIRE(!p0["activations"].empty());
        const json& a0 = p0["activations"][0];
        for (const char* k : {"act_tick", "deact_tick", "sqout_tick", "nominal_deact_tick",
                              "sp_meter", "skips", "skipped_fill_ticks", "chord_code", "sqinouts",
                              "sqin_ticks"})
            CHECK_MESSAGE(a0.contains(k), k);

        checked = true;
        break;  // one chart's first path is the whole contract
    }
    CHECK(checked);
}

// dump and target print one "result" block. A record with no paths has no best
// score (summarize_record's answer, which hydra_batch prints as "-"), so the
// block says null there rather than a real-looking 0 (audit finding 98).
TEST_CASE("result block: an empty record writes a null score and an empty bestpath") {
    const json empty = result_json(HydraRecord{});
    CHECK(empty["score"].is_null());
    CHECK(empty["bestpath"].get<std::string>() == "");

    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    bool checked = false;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        if (rec.paths.empty()) continue;

        // The first corpus chart with paths, and its best score and path as
        // one run printed them.
        const json full = result_json(rec);
        CHECK(chart.find("Allister - Overrated") != std::string::npos);
        CHECK(full["score"].get<int64_t>() == 228710);
        CHECK(full["bestpath"].get<std::string>() == "1 2");
        checked = true;
        break;  // one chart's record is the whole contract
    }
    CHECK(checked);
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
    CHECK((dumped[0]["activations"][0]["skipped_fill_ticks"].get<std::vector<int64_t>>() ==
           std::vector<int64_t>{19200}));
}

// A window that ends on the note closing a Star Power phrase is exactly where
// a squeeze-out hides. If the player squeezed, that note was hit after Star
// Power ran out and is not doubled; if they did not, it is. The window list
// alone cannot tell the two apart, so the tool says so instead of guessing.
TEST_CASE("a window ending on a phrase note with no offset is flagged") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), true, true);
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
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), true, true);
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

    // No phrase chord within 500 ms of the SP end: nothing to resolve to.
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
// the search can never produce, so it is refused and nothing is priced (plan
// decision 20 of 2026-09-24). Before D36 the roles were the other way round:
// the first chord in the window was the engine's.
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

// A phrase chord at or before the activation was banked before Star Power
// started, so that window cannot squeeze it out, even when it sits inside the
// 500 ms window around the SP end. The replay refuses the typed offset and
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
TEST_CASE("a typed squeeze-out on the phrase after a banked one is the engine's (D34)") {
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

    // Exactly 500 ms before D is outside the graph's window: no warning.
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

    // Exactly 500 ms before D is outside.
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
TEST_CASE("offered_phrase: the newest step's phrase early, the first unsqueezed one late (D36)") {
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
TEST_CASE("sqout_chords: a window's newest phrase at or before D, or the first after D (D36)") {
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
    // singles so a later chord crosses 20 on its own. The audit's Evans Blue,
    // Beg case is the single-note crossing: paid at 2x, reported as 1.
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

    // A single note that is the 20th: paid at 3x. The old field said 2.
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

TEST_CASE("replay multipliers agree with the combo on every corpus chord") {
    Song song = load_songpath(corpus::first_chart_with_suffix(".mid"), true, true);
    REQUIRE_FALSE(song.is_empty());
    const ReplayResult r = replay_path(song, {});

    for (size_t i = 0; i < r.chords.size(); ++i) {
        const ReplayChord& c = r.chords[i];
        REQUIRE_FALSE(c.notes.empty());
        // The chord's multiplier is its first note's; multiplier_after is its
        // last note's.
        CHECK(c.multiplier == c.notes.front().multiplier);
        CHECK(c.multiplier_after == c.notes.back().multiplier);
        CHECK(c.multiplier == to_multiplier(c.combo_before + 1));
        // What the disc shows after this chord is what the next chord starts
        // from: the old field's value on the next chord.
        if (i + 1 < r.chords.size())
            CHECK(c.multiplier_after == to_multiplier(r.chords[i + 1].combo_before));
        // A note's dynamics bonus is part of what it pays, never more.
        for (const ReplayNote& n : c.notes) {
            CHECK(n.dynamics_bonus >= 0);
            CHECK(n.dynamics_bonus < n.sp_points);
        }
    }
}

TEST_CASE("replay score fields: one list in schema order") {
    REQUIRE(std::size(kReplayScoreFields) == 6);
    const char* want[] = {"base", "combo", "sp", "solo", "accent", "ghost"};
    for (size_t i = 0; i < 6; ++i) CHECK(std::string(kReplayScoreFields[i].name) == want[i]);

    ReplayScore s;
    s.base = 1; s.combo = 2; s.sp = 3; s.solo = 4; s.accent = 5; s.ghost = 6;
    for (size_t i = 0; i < 6; ++i)
        CHECK(s.*(kReplayScoreFields[i].member) == static_cast<int64_t>(i + 1));
}

// ---------------------------------------------------------------------------
// The open-window walk.
//
// replay_path keeps a short list of the windows that can still pay the
// current chord, rather than asking every window about every chord. The cases
// below pin its rows on hand-built windows. On many windows they hold it to
// two properties instead: the order the windows are listed in does not
// matter, and each chord's SP points are the sum of what each window pays it
// alone, since the SP rule prices one window at a time.

namespace {

// The name of the first field where two rows differ, or "" when every field
// of ReplayChord (and of each ReplayNote) is identical. With `scores_only`,
// chord_code and notes are skipped (a scores-only row leaves them empty).
std::string first_row_difference(const ReplayChord& a, const ReplayChord& b,
                                 bool scores_only = false) {
    if (a.index != b.index) return "index";
    if (a.tick != b.tick) return "tick";
    if (a.ms != b.ms) return "ms";
    if (a.measure != b.measure) return "measure";
    if (a.beat != b.beat) return "beat";
    if (a.measure_tick != b.measure_tick) return "measure_tick";
    if (a.measures_decimal != b.measures_decimal) return "measures_decimal";
    if (!scores_only) {
        if (a.chord_code != b.chord_code) return "chord_code";
        if (a.notes.size() != b.notes.size()) return "notes.size";
        for (size_t k = 0; k < a.notes.size(); ++k) {
            const ReplayNote& x = a.notes[k];
            const ReplayNote& y = b.notes[k];
            if (x.color != y.color) return "notes.color";
            if (x.cymbal != y.cymbal) return "notes.cymbal";
            if (x.sp_points != y.sp_points) return "notes.sp_points";
            if (x.multiplier != y.multiplier) return "notes.multiplier";
            if (x.dynamics_bonus != y.dynamics_bonus) return "notes.dynamics_bonus";
            if (x.dynamic != y.dynamic) return "notes.dynamic";
        }
    }
    if (a.is_fill != b.is_fill) return "is_fill";
    if (a.is_solo != b.is_solo) return "is_solo";
    if (a.is_sp_phrase_end != b.is_sp_phrase_end) return "is_sp_phrase_end";
    if (a.combo_before != b.combo_before) return "combo_before";
    if (a.combo_after != b.combo_after) return "combo_after";
    if (a.multiplier != b.multiplier) return "multiplier";
    if (a.multiplier_after != b.multiplier_after) return "multiplier_after";
    if (a.in_sp != b.in_sp) return "in_sp";
    if (a.multiplier_shown != b.multiplier_shown) return "multiplier_shown";
    if (!(a.points == b.points)) return "points";
    if (!(a.cum == b.cum)) return "cum";
    if (a.cum_onscreen_total != b.cum_onscreen_total) return "cum_onscreen_total";
    return "";
}

// "" when the two results are identical row for row, else where they differ.
std::string first_result_difference(const ReplayResult& a, const ReplayResult& b,
                                    bool scores_only = false) {
    if (a.chords.size() != b.chords.size()) return "row count";
    for (size_t i = 0; i < a.chords.size(); ++i) {
        const std::string d = first_row_difference(a.chords[i], b.chords[i], scores_only);
        if (!d.empty()) return "row " + std::to_string(i) + ": " + d;
    }
    if (!(a.final == b.final)) return "final";
    return "";
}

// A small deterministic generator, so the synthetic windows are the same on
// every run and every machine.
struct Lcg {
    uint64_t state;
    uint32_t next() {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<uint32_t>(state >> 33);
    }
};

// `count` made-up windows over `song`: random activation chords, windows of
// 1 to 60 chords that overlap freely, deactivation nodes that are sometimes a
// few ticks off a chord, and a squeeze-out chord on about a quarter of them
// (anywhere from 3 chords before the SP end to 3 after, so some sit before D).
std::vector<ReplayWindow> synthetic_windows(const Song& song, size_t count,
                                            uint64_t seed) {
    std::vector<ReplayWindow> out;
    const size_t n = song.sequence.size();
    if (n == 0) return out;
    Lcg rng{seed};
    out.reserve(count);
    for (size_t j = 0; j < count; ++j) {
        const size_t a = rng.next() % n;
        const size_t len = 1 + rng.next() % 60;
        const size_t d = std::min(a + len, n - 1);
        ReplayWindow w;
        w.act_tick = song.sequence[a].timecode.ticks();
        w.deact_tick = song.sequence[d].timecode.ticks() +
                       (rng.next() % 3 == 0 ? static_cast<int64_t>(rng.next() % 10) : 0);
        if (rng.next() % 4 == 0) {
            const size_t lo = d >= 3 ? d - 3 : 0;
            const size_t q = std::min(lo + rng.next() % 7, n - 1);
            w.sqout_tick = song.sequence[q].timecode.ticks();
        }
        out.push_back(w);
    }
    return out;
}

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
// sits: index, tick, measure.beat.tick, combo before -> after, multiplier
// before / after, base, combo and SP points, "SP" when Star Power paid the
// chord, the disc's multiplier and the running total.
std::string row_line(const ReplayChord& c) {
    char buf[200];
    std::snprintf(buf, sizeof buf,
                  "%d t%lld m%lld.%lld.%lld c%d->%d x%d/%d base=%lld combo=%lld sp=%lld %s "
                  "shown=x%d cum=%lld",
                  c.index, static_cast<long long>(c.tick), static_cast<long long>(c.measure + 1),
                  static_cast<long long>(c.beat + 1), static_cast<long long>(c.measure_tick),
                  c.combo_before, c.combo_after, c.multiplier, c.multiplier_after,
                  static_cast<long long>(c.points.base), static_cast<long long>(c.points.combo),
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

// Six R+Y chords on the chart of "a squeezed-out chord past the leeway earns
// nothing": 192 ticks a beat, 120 BPM, 768 ticks (2000 ms) a measure. Ticks
// 0, 768, 1536, 2304, 3072 are 0, 2000, 4000, 6000, 8000 ms; the phrase chord
// at 3256 is 8479.2 ms.
Song squeeze_chart() {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3256}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        if (tick == 3256) test::mark_phrase_end(ts, tick, song.tick_resolution());
        song.sequence.push_back(ts);
    }
    return song;
}

ReplayWindow window(int64_t act, int64_t deact, std::optional<int64_t> sqout = std::nullopt) {
    ReplayWindow w;
    w.act_tick = act;
    w.deact_tick = deact;
    w.sqout_tick = sqout;
    return w;
}

// Each row's SP points must be the sum of what each window pays it alone,
// and SP pays the chord exactly when one of them does. Single-window replays
// are scores-only to keep this quick; their score fields are a full replay's.
void check_windows_add_up(const Song& song, const std::vector<ReplayWindow>& wl,
                          const core::Rules& rules, const ReplayResult& all,
                          const std::string& what) {
    ReplayOptions scores;
    scores.scores_only = true;
    std::vector<int64_t> sum(all.chords.size(), 0);
    std::vector<bool> paid(all.chords.size(), false);
    for (const ReplayWindow& w : wl) {
        const ReplayResult one = replay_path(song, {w}, rules, scores);
        REQUIRE(one.chords.size() == all.chords.size());
        for (size_t i = 0; i < one.chords.size(); ++i) {
            sum[i] += one.chords[i].points.sp;
            if (one.chords[i].in_sp) paid[i] = true;
        }
    }
    size_t bad = 0, first = 0;
    for (size_t i = 0; i < all.chords.size(); ++i)
        if ((all.chords[i].points.sp != sum[i] || all.chords[i].in_sp != paid[i]) && bad++ == 0)
            first = i;
    INFO(what << ": first row that does not add up: " << first);
    CHECK(bad == 0);
}

// What every row of any replay holds: SP paid the chord exactly when it paid
// it points (D2), and the disc doubles exactly then.
void check_disc_follows_payment(const ReplayResult& r, const std::string& what) {
    size_t bad = 0, first = 0;
    for (size_t i = 0; i < r.chords.size(); ++i) {
        const ReplayChord& c = r.chords[i];
        if ((c.in_sp != (c.points.sp > 0) ||
             c.multiplier_shown != shown_multiplier(c.multiplier_after, c.in_sp)) &&
            bad++ == 0)
            first = i;
    }
    INFO(what << ": first row whose disc does not follow SP's payment: " << first);
    CHECK(bad == 0);
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
    // at 50 points (base 100); the multiplier steps to x2 at the tenth note,
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
                {"0 t0 m1.1.0 c0->2 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=200",
                 "1 t768 m2.1.0 c2->4 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "2 t1536 m3.1.0 c4->6 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "3 t2304 m4.1.0 c6->8 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=800",
                 "4 t3072 m5.1.0 c8->10 x1/2 base=100 combo=50 sp=150 SP shown=x4 cum=1100",
                 "5 t3256 m5.1.184 c10->12 x2/2 base=100 combo=100 sp=0 -- shown=x2 cum=1300"},
                "late");
    check_lines(row_lines(replay_path(song, {early}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 base=100 combo=0 sp=0 -- shown=x1 cum=100",
                 "1 t768 m2.1.0 c2->4 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=300",
                 "2 t1536 m3.1.0 c4->6 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=500",
                 "3 t2304 m4.1.0 c6->8 x1/1 base=100 combo=0 sp=50 SP shown=x2 cum=650",
                 "4 t3072 m5.1.0 c8->10 x1/2 base=100 combo=50 sp=0 -- shown=x2 cum=800",
                 "5 t3256 m5.1.184 c10->12 x2/2 base=100 combo=100 sp=0 -- shown=x2 cum=1000"},
                "early");
    check_lines(row_lines(replay_path(song, {plain}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 base=100 combo=0 sp=0 -- shown=x1 cum=100",
                 "1 t768 m2.1.0 c2->4 x1/1 base=100 combo=0 sp=0 -- shown=x1 cum=200",
                 "2 t1536 m3.1.0 c4->6 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "3 t2304 m4.1.0 c6->8 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "4 t3072 m5.1.0 c8->10 x1/2 base=100 combo=50 sp=150 SP shown=x4 cum=900",
                 "5 t3256 m5.1.184 c10->12 x2/2 base=100 combo=100 sp=0 -- shown=x2 cum=1100"},
                "plain");
    check_lines(row_lines(replay_path(song, {near_w}, rules)),
                {"0 t0 m1.1.0 c0->2 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=200",
                 "1 t768 m2.1.0 c2->4 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=400",
                 "2 t1536 m3.1.0 c4->6 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=600",
                 "3 t2304 m4.1.0 c6->8 x1/1 base=100 combo=0 sp=100 SP shown=x2 cum=800",
                 "4 t3072 m5.1.0 c8->10 x1/2 base=100 combo=50 sp=0 -- shown=x2 cum=950",
                 "5 t3256 m5.1.184 c10->12 x2/2 base=100 combo=100 sp=0 -- shown=x2 cum=1150"},
                "near");
    // Together each chord's SP points are the five windows' sum: chord 2 is
    // 100 from four windows, chord 4 150 from late, plain and tick.
    const std::vector<std::string> all_rows = {
        "0 t0 m1.1.0 c0->2 x1/1 base=100 combo=0 sp=200 SP shown=x2 cum=300",
        "1 t768 m2.1.0 c2->4 x1/1 base=100 combo=0 sp=300 SP shown=x2 cum=700",
        "2 t1536 m3.1.0 c4->6 x1/1 base=100 combo=0 sp=500 SP shown=x2 cum=1300",
        "3 t2304 m4.1.0 c6->8 x1/1 base=100 combo=0 sp=450 SP shown=x2 cum=1850",
        "4 t3072 m5.1.0 c8->10 x1/2 base=100 combo=50 sp=450 SP shown=x4 cum=2450",
        "5 t3256 m5.1.184 c10->12 x2/2 base=100 combo=100 sp=0 -- shown=x2 cum=2650"};
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
    for (const core::Rules& r : leeway_variants()) {
        const ReplayResult res = replay_path(song, wl, r);
        got.push_back(rules_label(r) + ": " + compact_line(res));
        check_windows_add_up(song, wl, r, res, rules_label(r));
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
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song* longest = nullptr;
    std::string longest_path;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (!longest || song.sequence.size() > longest->sequence.size()) {
            longest = &song;
            longest_path = path;
        }
    }
    REQUIRE(longest != nullptr);
    REQUIRE(longest->sequence.size() > 500);
    INFO("chart: " << longest_path << " (" << longest->sequence.size() << " chords)");

    const std::vector<ReplayWindow> wl = synthetic_windows(*longest, 2000, 1);
    std::vector<ReplayWindow> reversed(wl.rbegin(), wl.rend());
    // The first 120 windows for the window-by-window sum: 120 replays each.
    // All 2,000 took 5.6 s against 0.5 s for 120 (2026-10-04), so 120 is a
    // test limit, like the others in D43.
    const std::vector<ReplayWindow> few(wl.begin(), wl.begin() + 120);
    for (const core::Rules& rules : leeway_variants()) {
        const std::string what = rules_label(rules);
        const ReplayResult r = replay_path(*longest, wl, rules);
        INFO(what);
        CHECK(first_result_difference(r, replay_path(*longest, reversed, rules)).empty());
        // The windows must actually pay something, or this proves nothing.
        CHECK(r.final.sp > 0);
        check_disc_follows_payment(r, what);
        check_windows_add_up(*longest, few, rules, replay_path(*longest, few, rules), what);
    }
}

// Every stored path on the corpus: the disc follows what SP paid on every
// row, and on each root path each row's SP points are the sum of what its
// windows pay one at a time. (The totals themselves are held to the engine's
// by "replay reproduces the engine's score for every corpus path".)
TEST_CASE("replay: on every corpus path the disc follows SP's payment and windows add up") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0, runs = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        ++charts;
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) {
            ++runs;
            const std::vector<ReplayWindow> wl = windows_for_path(*p);
            const ReplayResult r = replay_path(song, wl, cfg.rules);
            check_disc_follows_payment(r, path);
        }
        for (const Path& p : rec.paths) {
            const std::vector<ReplayWindow> wl = windows_for_path(p);
            check_windows_add_up(song, wl, cfg.rules, replay_path(song, wl, cfg.rules), path);
        }
    }
    CHECK(charts > 0);
    CHECK(runs > charts);
}

TEST_CASE("replay: scores_only leaves chord_code and notes empty and every score the same") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    ReplayOptions scores;
    scores.scores_only = true;
    int runs = 0;
    std::string first_diff;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
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

// Timing on the real stress chart, for the record (Task 9 of the
// 2026-10-03 preview-loading plan). Skipped by default; run it with
//   hydra_tests.exe -tc="replay timing*" --no-skip
// HYDRA_REPLAY_BENCH_CHART overrides the chart path.
TEST_CASE("replay timing: 2,000 activations on the Discography chart" * doctest::skip()) {
    const std::string chart = read_env("HYDRA_REPLAY_BENCH_CHART").value_or(
        "C:\\Clone Hero\\songs\\Misc Downloads\\blink-182 - Discography\\notes.mid");
    if (!std::filesystem::exists(std::filesystem::u8path(chart))) {
        MESSAGE("chart not found, skipped: " << chart);
        return;
    }
    const Song song = load_songpath(chart, true, true);
    REQUIRE_FALSE(song.is_empty());
    const std::vector<ReplayWindow> wl = synthetic_windows(song, 2000, 7);
    const core::Rules rules = core::default_rules();
    ReplayOptions scores;
    scores.scores_only = true;

    using clock = std::chrono::steady_clock;
    auto best_ms = [](auto&& fn) {
        double best = 1e300;
        for (int run = 0; run < 3; ++run) {
            const auto t0 = clock::now();
            fn();
            const double ms =
                std::chrono::duration<double, std::milli>(clock::now() - t0).count();
            best = std::min(best, ms);
        }
        return best;
    };
    ReplayResult b, c, z;
    const double new_ms = best_ms([&] { b = replay_path(song, wl, rules); });
    const double lean_ms = best_ms([&] { c = replay_path(song, wl, rules, scores); });
    const double none_ms = best_ms([&] { z = replay_path(song, {}, rules); });
    CHECK(first_result_difference(b, c, true).empty());
    MESSAGE(song.sequence.size() << " chords, " << wl.size() << " windows, best of 3:"
            << " open-window walk " << new_ms << " ms, scores-only " << lean_ms
            << " ms, no windows " << none_ms << " ms");
}
