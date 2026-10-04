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
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
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
#include "search/engine.h"
#include "core/model.h"  // kSqueezeWindowMs, the horizon the warning uses
#include "search/pather.h"

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

            std::vector<int64_t> ticks;
            bool have_ticks = true;
            for (const Activation& act : want_acts) {
                ticks.push_back(act.timecode.ticks());
            }
            REQUIRE(have_ticks);

            const std::vector<Path> got = search_target(song, cfg, ticks);

            // Somewhere in the returned variants must be this exact path.
            const Path* match = nullptr;
            HydraRecord holder;
            holder.paths = got;
            for (const Path* q : holder.all_paths()) {
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
                                 std::to_string(holder.all_paths().size()) +
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

std::string ticks_text(const std::vector<int64_t>& ticks) {
    std::string s = "{";
    for (size_t i = 0; i < ticks.size(); ++i)
        s += (i ? ", " : "") + std::to_string(ticks[i]);
    return s + "}";
}

std::string steps_text(const std::vector<SpEndStep>& steps) {
    std::string s = "{";
    for (size_t i = 0; i < steps.size(); ++i)
        s += (i ? ", " : "") + std::to_string(steps[i].tick) + "->" +
             std::to_string(steps[i].end_tick) + " kind " +
             std::to_string(static_cast<int>(steps[i].kind));
    return s + "}";
}

std::string opt_text(const std::optional<int64_t>& t) {
    return t ? std::to_string(*t) : std::string("unset");
}

// Each activation's squeeze symbols, e.g. "{+, -, .}" ("." for none).
std::string kinds_text(const std::vector<Activation>& acts) {
    std::string s = "{";
    for (size_t i = 0; i < acts.size(); ++i) {
        s += i ? ", " : "";
        if (acts[i].sqinouts.empty()) s += ".";
        for (const SPSqueeze& q : acts[i].sqinouts) s += q.symbol();
    }
    return s + "}";
}

bool same_ticks(const std::vector<Activation>& a, const std::vector<Activation>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].timecode.ticks() != b[i].timecode.ticks()) return false;
    return true;
}

bool same_kinds(const std::vector<Activation>& a, const std::vector<Activation>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].sqinouts.size() != b[i].sqinouts.size()) return false;
        for (size_t k = 0; k < a[i].sqinouts.size(); ++k)
            if (a[i].sqinouts[k].kind != b[i].sqinouts[k].kind) return false;
    }
    return true;
}

struct TiedVariantCount {
    int variants = 0;  // tied variants the analysis listed
    int compared = 0;  // of those, the ones a lone search could price
    int differing = 0; // of those, the ones whose stored facts differ
    int skipped() const { return variants - compared; }
};

// Decision D3 for one set of analysis settings, over the whole corpus. Each
// variant is priced alone with a targeted search, and its stored facts must
// equal that search's. Only a root of the search is an oracle: a root was
// never folded. Skips, the early-fill offset and the skipped fills are not
// compared; they are finding 97. All-zero variants are not visited.
//
// A variant with no oracle is skipped, and each skip is printed with its
// reason. The caller pins how many there are, so a regression that turns a
// compared variant into a skipped one fails instead of passing quietly.
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
            const std::vector<Activation> want = p->all_activations();
            std::vector<int64_t> ticks;
            for (const Activation& a : want) ticks.push_back(a.timecode.ticks());
            const std::string where = path + " [" + p->pathstring() + "] ";

            HydraRecord alone;
            alone.paths = search_target(song, cfg, ticks);
            // The oracle: a root with the same score and squeeze kinds.
            const Path* match = nullptr;
            // A root with the same score and ticks but other squeeze kinds.
            const Path* other_kinds = nullptr;
            for (const Path& q : alone.paths) {
                if (q.totalscore() != p->totalscore()) continue;
                const std::vector<Activation> qa = q.all_activations();
                if (same_kinds(qa, want)) { match = &q; break; }
                if (!other_kinds && same_ticks(qa, want)) other_kinds = &q;
            }

            if (!match) {
                // The lone search may itself have tied the variant's kinds
                // under a root with other kinds. Then there is no oracle, and
                // that is a skip. Otherwise the variant holds squeeze kinds a
                // lone search never gives it, and that is a difference.
                bool tied_alone = false;
                for (const Path* q : alone.all_paths())
                    if (q->totalscore() == p->totalscore() &&
                        same_kinds(q->all_activations(), want))
                        tied_alone = true;
                if (other_kinds && !tied_alone) {
                    ++n.compared;
                    ++n.differing;
                    const std::string d = where + "squeeze kinds: stored " + kinds_text(want) +
                                          ", alone " +
                                          kinds_text(other_kinds->all_activations());
                    CHECK_MESSAGE(false, d);
                    continue;
                }
                std::string why;
                if (alone.paths.empty()) why = "the lone search found no path";
                else if (tied_alone) why = "the lone search tied it under a root too";
                else why = "no lone root has its score and squeeze kinds";
                std::string roots;
                for (const Path& q : alone.paths)
                    roots += " [" + q.pathstring() + "] score " +
                             std::to_string(q.totalscore()) + " kinds " +
                             kinds_text(q.all_activations()) + ";";
                MESSAGE("skipped " << where << "score " << p->totalscore() << " kinds "
                                   << kinds_text(want) << ": " << why << ". Lone roots:"
                                   << roots);
                continue;
            }
            ++n.compared;

            // Every difference, spelled out with both values, so a failure
            // names the chart, the path, the field and what each side holds.
            std::vector<std::string> diffs;
            if (match->trailing_bank_ticks != p->trailing_bank_ticks)
                diffs.push_back(where + "trailing_bank_ticks: stored " +
                                ticks_text(p->trailing_bank_ticks) + ", alone " +
                                ticks_text(match->trailing_bank_ticks));
            const std::vector<Activation> got = match->all_activations();
            for (size_t i = 0; i < got.size(); ++i) {
                const std::string act = where + "activation at " +
                                        std::to_string(want[i].timecode.ticks()) + " ";
                if (got[i].sp_end_steps != want[i].sp_end_steps)
                    diffs.push_back(act + "sp_end_steps: stored " +
                                    steps_text(want[i].sp_end_steps) + ", alone " +
                                    steps_text(got[i].sp_end_steps));
                if (got[i].bank_rise_ticks != want[i].bank_rise_ticks)
                    diffs.push_back(act + "bank_rise_ticks: stored " +
                                    ticks_text(want[i].bank_rise_ticks) + ", alone " +
                                    ticks_text(got[i].bank_rise_ticks));
                if (got[i].sqout_tick != want[i].sqout_tick)
                    diffs.push_back(act + "sqout_tick: stored " + opt_text(want[i].sqout_tick) +
                                    ", alone " + opt_text(got[i].sqout_tick));
                if (got[i].display_backends() != want[i].display_backends())
                    diffs.push_back(act + "backend rows: stored " +
                                    std::to_string(want[i].display_backends().size()) +
                                    " rows, alone " +
                                    std::to_string(got[i].display_backends().size()) +
                                    " rows, not equal");
            }
            if (!diffs.empty()) ++n.differing;
            for (const std::string& d : diffs) CHECK_MESSAGE(false, d);
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
    struct Setting { int cap; int depth; int skipped; };
    for (const Setting s : {Setting{4, 4, 0}, Setting{4, 40, 0}, Setting{2, 40, 4}}) {
        app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
        cfg.sp_cap = s.cap;
        cfg.depth_value = s.depth;
        INFO("cap " << s.cap << ", score range " << s.depth);
        const TiedVariantCount n = check_tied_variants(cfg);
        CHECK(n.variants > 0);
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
    song.tpm_changes[0] = 768;
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3256}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        ts.flag_sp = tick == 3256;
        song.sequence.push_back(ts);
    }

    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    w.sqout_offset_ms = song.timecode(3256).ms() - song.timecode(3072).ms();
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
        song.tpm_changes[0] = 768;
        song.bpm_changes[0] = 120.0;
        song.build_timing();
        for (int64_t tick : {0, 768, 1536, 2304, 2976, 3072}) {
            SongTimestamp ts;
            ts.timecode = song.timecode(tick);
            ts.chord.add_note(NoteColor::Red);
            if (tick != 2976 || two_notes) ts.chord.add_note(NoteColor::Yellow);
            ts.flag_sp = tick == 2976;
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
    song.tpm_changes[0] = 768;
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 864, 1536}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);  // one note: a squeeze-out loses all its doubling
        ts.flag_sp = tick == 864;
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
    CHECK(ambiguous_window_warnings(song, r, {a, b}).empty());

    // With no squeezed-out window the chord still is not paid (250 ms is past
    // the leeway), so the answer is the same.
    const ReplayResult alone = replay_path(song, {a});
    CHECK(ambiguous_window_warnings(song, alone, {a}).empty());
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
// high. So what has to be pinned is that the offset survives the trip.
TEST_CASE("a path JSON becomes windows with the squeeze-out offset intact") {
    const json path = json::parse(R"({
      "index": 0,
      "activations": [
        {"act_tick": 480, "deact_tick": 3840, "sqinouts": []},
        {"act_tick": 7680, "deact_tick": 11520, "sqout_tick": 11532,
         "sqinouts": [{"kind": "SqIn",  "offset_ms": 12.5},
                      {"kind": "SqOut", "offset_ms": 31.25}]}
      ]})");

    const std::vector<ReplayWindow> w = windows_from_json(path);
    REQUIRE(w.size() == 2);
    CHECK(w[0].act_tick == 480);
    CHECK(w[0].deact_tick == 3840);
    CHECK_FALSE(w[0].sqout_offset_ms.has_value());

    CHECK(w[1].act_tick == 7680);
    CHECK(w[1].deact_tick == 11520);
    REQUIRE(w[1].sqout_offset_ms.has_value());
    // The SqIn sits in the same list and must not be mistaken for the SqOut.
    CHECK(*w[1].sqout_offset_ms == doctest::Approx(31.25));
    CHECK_FALSE(w[0].sqout_tick.has_value());
    REQUIRE(w[1].sqout_tick.has_value());
    CHECK(*w[1].sqout_tick == 11532);

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
                CHECK(got[i].sqout_offset_ms == want[i].sqout_offset_ms);
                CHECK(got[i].sqout_tick == want[i].sqout_tick);
            }
            ++checked;
        }
        if (checked > 0) break;  // one chart's paths are the whole contract
    }
    CHECK(checked > 0);
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
                              "sp_meter", "skips", "chord_code", "sqinouts"})
            CHECK_MESSAGE(a0.contains(k), k);

        checked = true;
        break;  // one chart's first path is the whole contract
    }
    CHECK(checked);
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
        if (last_phrase == phrase_note && !just_after &&
            c.ms - phrase_note->ms < kSqueezeWindowMs)
            just_after = &c;
        if (!long_after && c.ms - last_phrase->ms > kSqueezeWindowMs)
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
        return ambiguous_window_warnings(song, replay_path(song, {win}), {win});
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
    CHECK(ambiguous_window_warnings(song, r, {settled}).empty());
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
    song.tpm_changes[0] = 768;
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (const auto& [tick, phrase] : chords) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        ts.flag_sp = phrase;
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

// The engine only ever squeezes out the first phrase chord strictly within
// 500 ms of the SP end. A typed offset that lands on a later one names a
// squeeze-out the search can never produce, so it is refused and nothing is
// priced (plan decision 20 of 2026-09-24).
TEST_CASE("a typed squeeze-out on a chord the engine never squeezes out is refused") {
    // Phrase chords 375 ms (tick 2928) and 93.75 ms (tick 3036) before D.
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3036, true}, {3072, false}});

    ReplayWindow late;
    late.act_tick = 0;
    late.deact_tick = 3072;
    late.sqout_offset_ms = -93.73;
    CHECK_THROWS_WITH_AS(
        resolve_sqout_note(two, late),
        "window 0:3072: the SqOut offset -93.73 ms lands on the phrase chord "
        "at tick 3036 (-93.75 ms from the SP end), which the engine never "
        "squeezes out. The only chord it can squeeze out here is the first "
        "phrase chord within 500 ms of the SP end, at tick 2928 (-375.00 ms). "
        "Not priced.",
        std::runtime_error);

    // The engine's own chord is accepted.
    ReplayWindow first = late;
    first.sqout_offset_ms = -375.0;
    const SqOutNote n = resolve_sqout_note(two, first);
    CHECK(n.tick == 2928);
    CHECK(n.offset_ms == doctest::Approx(-375.0));
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
        ambiguous_window_warnings(song, r, {first, second});
    REQUIRE(warned.size() == 1);
    CHECK(warned[0].rfind("window 0:2950 ", 0) == 0);
}

// The graph lets a deactivation squeeze out exactly one chord: the first
// phrase chord strictly within 500 ms of the SP end (core::sqout_chord, which
// graph.cpp add_deact_edge calls). The warning names that chord, and only
// when the window actually paid it.
TEST_CASE("the squeeze-out warning names the chord the graph would squeeze") {
    // Phrase chords 375 ms (tick 2928) and 125 ms (tick 3024) before D.
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3024, true}, {3072, false}});
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    const ReplayResult r = replay_path(two, {w});
    const std::vector<std::string> warned = ambiguous_window_warnings(two, r, {w});
    REQUIRE(warned.size() == 1);
    CHECK(warned[0].find("tick 2928") != std::string::npos);

    // Exactly 500 ms before D is outside the graph's window: no warning.
    const Song edge = song_with({{0, false}, {768, false}, {2880, true},
                                 {3072, false}});
    const ReplayResult re = replay_path(edge, {w});
    CHECK(ambiguous_window_warnings(edge, re, {w}).empty());

    // A phrase chord one tick (2.6 ms) after D is inside the leeway, so the
    // window paid it and squeezing it out would change the score.
    const Song after = song_with({{0, false}, {768, false}, {3072, false},
                                  {3073, true}});
    const ReplayResult ra = replay_path(after, {w});
    const std::vector<std::string> late = ambiguous_window_warnings(after, ra, {w});
    REQUIRE(late.size() == 1);
    CHECK(late[0].find("tick 3073") != std::string::npos);
}

// The one rule for which phrase chord an SP end can squeeze out.
TEST_CASE("sqout_chord: the first phrase chord strictly inside the window, in chart order") {
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3036, true}, {3072, false}});
    const SongTimestamp* c = core::sqout_chord(two, two.timecode(3072));
    REQUIRE(c != nullptr);
    CHECK(c->timecode.ticks() == 2928);  // 375 ms before D comes before 93.75 ms

    // Exactly 500 ms before D is outside.
    const Song edge = song_with({{0, false}, {2880, true}, {3072, false}});
    CHECK(core::sqout_chord(edge, edge.timecode(3072)) == nullptr);

    // Only a phrase chord after D: that one.
    const Song after = song_with({{0, false}, {3072, false}, {3073, true}});
    REQUIRE(core::sqout_chord(after, after.timecode(3072)) != nullptr);
    CHECK(core::sqout_chord(after, after.timecode(3072))->timecode.ticks() == 3073);

    // An SP end between two chords, and no phrase chord at all.
    REQUIRE(core::sqout_chord(two, two.timecode(3000)) != nullptr);
    CHECK(core::sqout_chord(two, two.timecode(3000))->timecode.ticks() == 2928);
    const Song bare = song_with({{0, false}, {3072, false}});
    CHECK(core::sqout_chord(bare, bare.timecode(3072)) == nullptr);
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
// The open-window walk against the every-window walk it replaced.
//
// replay_path used to check every window for every chord. It now keeps a short
// list of the windows that can still pay the current chord. The function
// below is the old loop, copied verbatim from core/replay.cpp before the
// change (only renamed), so the tests can prove the two give identical rows.

namespace {

struct RefWindow {
    int64_t act_tick = 0;
    int64_t deact_tick = 0;
    double deact_ms = 0.0;
    std::optional<int64_t> sqout_tick;
};

ReplayResult reference_replay_path(const Song& song,
                                   const std::vector<ReplayWindow>& windows,
                                   const core::Rules& rules,
                                   bool d2_in_sp = true) {
    const SongTiming& timing = song.timing();

    std::vector<RefWindow> wins;
    wins.reserve(windows.size());
    for (const ReplayWindow& w : windows) {
        RefWindow win;
        win.act_tick = w.act_tick;
        win.deact_tick = w.deact_tick;
        win.deact_ms = timing.timecode(w.deact_tick).ms();
        win.sqout_tick = w.sqout_tick;
        wins.push_back(win);
    }
    std::sort(wins.begin(), wins.end(), [](const RefWindow& a, const RefWindow& b) {
        return a.act_tick < b.act_tick;
    });

    ReplayResult out;
    out.chords.reserve(song.sequence.size());

    int combo = 0;
    ReplayScore cum;
    int64_t solo_pending = 0;

    const size_t n = song.sequence.size();
    std::vector<CategoryScores> per_note;

    for (size_t i = 0; i < n; ++i) {
        const SongTimestamp& ts = song.sequence[i];
        const CategoryScores sg = category_scores(ts.chord, combo, &per_note, rules.sqout_rule);

        ReplayChord row;
        row.index = static_cast<int>(i);
        row.tick = ts.timecode.ticks();
        row.ms = ts.timecode.ms();
        const int64_t* mbt = ts.timecode.measure_beats_ticks();
        row.measure = mbt[0];
        row.beat = mbt[1];
        row.measure_tick = mbt[2];
        row.measures_decimal = ts.timecode.measures_decimal();
        row.chord_code = ts.chord.code();
        row.is_fill = ts.has_activation();
        row.is_solo = ts.flag_solo;
        row.is_sp_phrase_end = ts.flag_sp;
        row.combo_before = combo;
        row.multiplier = sg.multiplier;
        row.multiplier_after = sg.multiplier_after;

        const std::vector<ChordNote> ordering = ts.chord.notes(true);
        row.notes.reserve(ordering.size());
        for (size_t k = 0; k < ordering.size(); ++k) {
            ReplayNote note;
            note.color = ordering[k].colortype;
            note.cymbal = ordering[k].is_cymbal();
            note.sp_points = k < per_note.size() ? per_note[k].sp : 0;
            note.multiplier = k < per_note.size() ? per_note[k].multiplier : 1;
            note.dynamics_bonus =
                k < per_note.size() ? per_note[k].dynamics_bonus : 0;
            note.dynamic = ordering[k].dynamictype;
            row.notes.push_back(note);
        }

        // The old gate counts a window that reaches the chord (sp_claims);
        // decision D2's rule counts one that actually pays it (sp_paid).
        // d2_in_sp picks which one sets in_sp, so a test can show both.
        int64_t sp_points = 0;
        int sp_claims = 0;
        int sp_paid = 0;
        for (const RefWindow& w : wins) {
            if (row.tick < w.act_tick) continue;
            const double offset = row.tick <= w.deact_tick
                                      ? std::min(row.ms - w.deact_ms, 0.0)
                                      : row.ms - w.deact_ms;
            const core::SqOutPosition pos =
                core::sqout_position(row.tick, w.sqout_tick);
            if (pos == core::SqOutPosition::After ||
                !core::counted_without_squeeze(offset, rules.backend_leeway_ms))
                continue;
            ++sp_claims;
            if (core::paid_by_sp(offset, sg.sp, sg.sqout_sp(), pos,
                                 rules.backend_leeway_ms))
                ++sp_paid;
            sp_points += core::backend_row_value(
                offset, sg.sp, sg.sqout_sp(), pos,
                rules.backend_leeway_ms);
        }
        row.in_sp = d2_in_sp ? sp_paid > 0 : sp_claims > 0;
        row.multiplier_shown = shown_multiplier(row.multiplier_after, row.in_sp);

        row.points.base = sg.base;
        row.points.combo = sg.combo;
        row.points.sp = sp_points;
        row.points.solo =
            ts.flag_solo ? static_cast<int64_t>(kSoloBonusPerNote) * ts.chord.count() : 0;
        row.points.accent = sg.accent;
        row.points.ghost = sg.ghost;

        combo += ts.chord.count();
        row.combo_after = combo;

        cum.add(row.points);
        row.cum = cum;

        if (ts.flag_solo) {
            solo_pending += row.points.solo;
            const bool last_of_run =
                i + 1 >= n || !song.sequence[i + 1].flag_solo;
            if (last_of_run) solo_pending = 0;
        }
        row.cum_onscreen_total = cum.total() - solo_pending;

        out.chords.push_back(std::move(row));
    }

    out.final = cum;
    return out;
}

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

}  // namespace

TEST_CASE("replay: the open-window walk equals the every-window walk on every corpus path") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int charts = 0, runs = 0;
    std::string first_diff;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        ++charts;

        // No Star Power at all, then every stored path's own windows.
        std::vector<std::vector<ReplayWindow>> lists{{}};
        const HydraRecord& rec = corpus::analyzed(path, cfg);
        for (const Path* p : rec.all_paths()) lists.push_back(windows_for_path(*p));

        for (const std::vector<ReplayWindow>& wl : lists) {
            ++runs;
            const std::string d = first_result_difference(
                reference_replay_path(song, wl, cfg.rules), replay_path(song, wl, cfg.rules));
            if (!d.empty() && first_diff.empty()) first_diff = path + ": " + d;
        }
    }

    CHECK(charts > 0);
    CHECK(runs > charts);
    INFO("first difference: " << first_diff);
    CHECK(first_diff.empty());
}

// D2 moves only the yes/no on a squeezed-out chord SP paid nothing. Every
// chord's points, and so every score, stays as it was.
TEST_CASE("D2: only a squeezed-out chord SP pays nothing loses its doubled disc") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int changed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        for (const Path* p : corpus::analyzed(path, cfg).all_paths()) {
            const std::vector<ReplayWindow> wl = windows_for_path(*p);
            const ReplayResult before = reference_replay_path(song, wl, cfg.rules, false);
            const ReplayResult after = replay_path(song, wl, cfg.rules);
            REQUIRE(before.chords.size() == after.chords.size());
            CHECK(before.final == after.final);
            for (size_t i = 0; i < after.chords.size(); ++i) {
                const ReplayChord& b = before.chords[i];
                const ReplayChord& a = after.chords[i];
                CHECK(a.points == b.points);
                if (a.in_sp == b.in_sp) continue;
                ++changed;
                CHECK(b.in_sp);
                CHECK(a.points.sp == 0);
                bool squeezed_out = false;
                for (const ReplayWindow& w : wl)
                    if (w.sqout_tick == a.tick) squeezed_out = true;
                CHECK(squeezed_out);
                CHECK(a.multiplier_shown == a.multiplier_after);
            }
        }
    }
    MESSAGE(changed << " corpus chords now show the plain multiplier");
    // Pinned, so a change that made this 0 (the test proving nothing) or moved
    // more chords fails loudly. If the corpus grows, update it on purpose.
    CHECK(changed == 9);
}

TEST_CASE("replay: the open-window walk equals the every-window walk on hand-built windows") {
    // The squeeze-out chart from "a squeezed-out chord past the leeway earns
    // nothing", under every leeway variant, with its window and with a
    // squeeze-out that sits before D.
    Song song(192);
    song.tpm_changes[0] = 768;
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t tick : {0, 768, 1536, 2304, 3072, 3256}) {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord.add_note(NoteColor::Red);
        ts.chord.add_note(NoteColor::Yellow);
        ts.flag_sp = tick == 3256;
        song.sequence.push_back(ts);
    }
    ReplayWindow late;
    late.act_tick = 0;
    late.deact_tick = 3072;
    late.sqout_tick = 3256;
    ReplayWindow early;
    early.act_tick = 768;
    early.deact_tick = 3072;
    early.sqout_tick = 2304;
    ReplayWindow plain;
    plain.act_tick = 1536;
    plain.deact_tick = 3100;

    const std::vector<std::vector<ReplayWindow>> lists = {
        {late}, {early}, {plain}, {late, early, plain}, {plain, early, late}};
    for (const core::Rules& rules : leeway_variants())
        for (const std::vector<ReplayWindow>& wl : lists) {
            const std::string d = first_result_difference(
                reference_replay_path(song, wl, rules), replay_path(song, wl, rules));
            INFO("leeway " << rules.backend_leeway_ms << ", " << wl.size() << " window(s)");
            CHECK(d.empty());
        }
}

TEST_CASE("replay: the open-window walk equals the every-window walk with 2,000 activations") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();

    // The longest corpus chart, by chord count.
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

    for (uint64_t seed : {1ULL, 2ULL, 3ULL}) {
        const std::vector<ReplayWindow> wl = synthetic_windows(*longest, 2000, seed);
        for (const core::Rules& rules : leeway_variants()) {
            const ReplayResult want = reference_replay_path(*longest, wl, rules);
            const std::string d = first_result_difference(want, replay_path(*longest, wl, rules));
            INFO("seed " << seed << ", leeway " << rules.backend_leeway_ms);
            CHECK(d.empty());
            // The windows must actually pay something, or the test proves
            // nothing about the exit rule.
            CHECK(want.final.sp > 0);
        }
    }
}

// The random windows above overlap freely, so nearly every squeezed-out chord
// is also paid by another window and its in_sp is the same under the old and
// the new rule: that test cannot see decision D2. These windows never overlap,
// and each ends on a one-note chord it squeezes out, which SP pays 0 for. The
// open-window walk must match the paid-based reference walk, and the old
// claim-based walk must differ from it somewhere, or this proves nothing.
TEST_CASE("replay: disjoint squeezed-out windows are told apart from the old in_sp rule") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    const Song* longest = nullptr;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (!longest || song.sequence.size() > longest->sequence.size()) longest = &song;
    }
    REQUIRE(longest != nullptr);

    std::vector<ReplayWindow> wl;
    size_t free_from = 0;  // the first chord the next window may start on
    for (size_t i = 0; i < longest->sequence.size(); ++i) {
        if (i < free_from + 2 || longest->sequence[i].chord.count() != 1) continue;
        ReplayWindow w;
        w.act_tick = longest->sequence[free_from].timecode.ticks();
        w.deact_tick = longest->sequence[i].timecode.ticks();
        w.sqout_tick = w.deact_tick;
        wl.push_back(w);
        free_from = i + 1;
    }
    REQUIRE(wl.size() > 50);

    size_t differing_chords = 0;
    for (const core::Rules& rules : leeway_variants()) {
        const ReplayResult paid = reference_replay_path(*longest, wl, rules, true);
        const ReplayResult claimed = reference_replay_path(*longest, wl, rules, false);
        INFO("leeway " << rules.backend_leeway_ms);
        CHECK(first_result_difference(paid, replay_path(*longest, wl, rules)).empty());
        REQUIRE(paid.chords.size() == claimed.chords.size());
        for (size_t i = 0; i < paid.chords.size(); ++i)
            if (paid.chords[i].in_sp != claimed.chords[i].in_sp) ++differing_chords;
    }
    CHECK(differing_chords > 0);
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
    ReplayResult a, b, c, z;
    const double old_ms = best_ms([&] { a = reference_replay_path(song, wl, rules); });
    const double new_ms = best_ms([&] { b = replay_path(song, wl, rules); });
    const double lean_ms = best_ms([&] { c = replay_path(song, wl, rules, scores); });
    const double none_ms = best_ms([&] { z = replay_path(song, {}, rules); });
    CHECK(first_result_difference(a, b).empty());
    CHECK(first_result_difference(a, c, true).empty());
    MESSAGE(song.sequence.size() << " chords, " << wl.size() << " windows, best of 3:"
            << " every-window walk " << old_ms << " ms, open-window walk " << new_ms
            << " ms, open-window scores-only " << lean_ms << " ms, no windows "
            << none_ms << " ms");
}
