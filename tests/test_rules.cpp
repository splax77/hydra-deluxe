// Tests for core::Rules and hydra_rules.ini (app/rules_file.h).
//
// Every rule the user chose by hand lives in one Rules value. Defaults must
// equal the values Hydra always used, and each non-default value must
// actually change the behavior it names.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/display_format.h"
#include "app/rules_file.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"
#include "temp_util.h"

using namespace hydra;

namespace {

// A scratch rules file: ScopedFile handles the cleanup, and write_rules
// fills it with the given text. The returned ScopedFile stays alive until the
// full expression ends, so load_rules_file(write_rules(...)) reads the file
// before ScopedFile's destructor removes it.
testtemp::ScopedFile write_rules(const char* tag, const std::string& text) {
    testtemp::ScopedFile file(std::string("rules_") + tag, ".ini");
    std::ofstream f(hydra::os_path(file.path), std::ios::trunc);
    f << text;
    return file;
}

// One measure per note on a 4/4 120 BPM song with no authored fills, so
// check_activations has to generate them.
Song fill_song() {
    Song song(192);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t m = 0; m < 16; ++m) {
        SongTimestamp ts;
        ts.timecode = song.timecode(m * 768);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(ts);
    }
    return song;
}

int fill_count(const Song& song) {
    int n = 0;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) ++n;
    return n;
}

}  // namespace

TEST_CASE("rules: defaults are the values Hydra always used") {
    const core::Rules& r = core::default_rules();
    CHECK(r.backend_leeway_ms == 3.0);
    CHECK(r.sqout_rule == core::SqOutRule::FirstNote);
    CHECK(r.max_tied_paths == 4);
    CHECK(r.fill_cooldown_measures == 4);
    CHECK(r.fill_max_distance_beats == 0.5);
    CHECK(r.fill_land_slop_beats == 1.0 / 32);
}

TEST_CASE("rules: a missing file and an empty file both load the defaults") {
    const uint64_t fp = core::default_rules().fingerprint();
    CHECK(app::load_rules_file(
              hydra::os_path(testtemp::temp_path("rules_does_not_exist", ".ini")))
              .fingerprint() == fp);
    CHECK(app::load_rules_file(write_rules("empty", "# nothing here\n\n")).fingerprint() == fp);
}

TEST_CASE("rules: every key in the file is read") {
    core::Rules r = app::load_rules_file(write_rules("full",
        "backend_leeway_ms = 5\n"
        "sqout_rule = whole_chord\n"
        "max_tied_paths = 2\n"
        "fill_cooldown_measures = 2\n"
        "fill_max_distance_beats = 0.25\n"
        "fill_land_slop_beats = 0.125\n"));
    CHECK(r.backend_leeway_ms == 5.0);
    CHECK(r.sqout_rule == core::SqOutRule::WholeChord);
    CHECK(r.max_tied_paths == 2);
    CHECK(r.fill_cooldown_measures == 2);
    CHECK(r.fill_max_distance_beats == 0.25);
    CHECK(r.fill_land_slop_beats == 0.125);
    CHECK(r.fingerprint() != core::default_rules().fingerprint());
}

TEST_CASE("rules: a # starts a comment anywhere on a line") {
    core::Rules r = app::load_rules_file(write_rules("comments",
        "# my rules\n"
        "max_tied_paths = 2   # fewer ties\n"
        "sqout_rule = whole_chord#no space before the comment\n"));
    CHECK(r.max_tied_paths == 2);
    CHECK(r.sqout_rule == core::SqOutRule::WholeChord);
}

TEST_CASE("rules: a bad value or an unknown key is an error that names the key") {
    auto message_for = [](const char* tag, const std::string& text) -> std::string {
        try {
            app::load_rules_file(write_rules(tag, text));
        } catch (const app::RulesFileError& e) {
            return e.what();
        }
        return "";
    };
    CHECK(message_for("bad1", "max_tied_paths = 0\n").find("max_tied_paths") != std::string::npos);
    CHECK(message_for("bad2", "sqout_rule = every_note\n").find("sqout_rule") != std::string::npos);
    CHECK(message_for("bad3", "backend_leeway_ms = fast\n").find("backend_leeway_ms") !=
          std::string::npos);
    // A typo is an unknown key, never a silent default.
    CHECK(message_for("bad5", "max_tied_path = 4\n").find("max_tied_path") != std::string::npos);
    // D114 item 8 (M73): numbers read through parse_finite_number, so the C
    // runtime's extra spellings are errors for decimal and whole keys alike.
    for (const char* spelling : {"nan", "inf", "NaN", "infinity", "0x10"}) {
        INFO("value: ", std::string(spelling));
        CHECK(message_for("bad6", std::string("fill_max_distance_beats = ") + spelling + "\n")
                  .find("fill_max_distance_beats") != std::string::npos);
        CHECK(message_for("bad7", std::string("max_tied_paths = ") + spelling + "\n")
                  .find("max_tied_paths") != std::string::npos);
    }
    // A whole-number key takes no fraction.
    CHECK(message_for("bad8", "max_tied_paths = 2.5\n").find("max_tied_paths") !=
          std::string::npos);
}

TEST_CASE("rules: a whole-number key takes a whole value written as a decimal") {
    // D114 item 24: "4.0" and "1e1" are whole, so they load as 4 and 10.
    core::Rules r = app::load_rules_file(write_rules("wholedec",
        "max_tied_paths = 4.0\n"
        "fill_cooldown_measures = 1e1\n"));
    CHECK(r.max_tied_paths == 4);
    CHECK(r.fill_cooldown_measures == 10);
}

TEST_CASE("rules: no rules value has the no-rules fingerprint") {
    // Task 2 opens the store with kNoRulesFingerprint when the file is bad,
    // so no stored row can read Ready. That only works if no real rules
    // value ever hashes to it.
    CHECK(core::default_rules().fingerprint() != core::kNoRulesFingerprint);
    core::Rules other = core::default_rules();
    other.max_tied_paths = 2;
    CHECK(other.fingerprint() != core::kNoRulesFingerprint);
}

TEST_CASE("rules: whole_chord takes every note's SP doubling on a squeeze-out") {
    hydra::Chord chord;  // qualified: windows.h (through temp_util.h) declares a Chord too
    chord.add_note(NoteColor::Red);
    chord.add_note(NoteColor::Blue);
    // Combo 0, so both notes sit at a 1x multiplier and are worth 50 each.
    CHECK(category_scores(chord, 0, nullptr, core::SqOutRule::FirstNote).sqout_reduction == 50);
    CHECK(category_scores(chord, 0, nullptr, core::SqOutRule::WholeChord).sqout_reduction == 100);

    std::vector<CategoryScores> per_note;
    category_scores(chord, 0, &per_note, core::SqOutRule::WholeChord);
    REQUIRE(per_note.size() == 2);
    CHECK(per_note[0].sqout_reduction == 50);
    CHECK(per_note[1].sqout_reduction == 50);
}

// The leeway decides only whether a plain row is counted (D269 call 4).
TEST_CASE("rules: the leeway decides whether a plain backend row is counted") {
    BackendSqueeze b;
    b.offset_ms = 4.0;
    CHECK(b.summarystr(false, std::nullopt) == "Hard (uncounted)");
    CHECK(b.summarystr(false, std::nullopt, 5.0) == "Normal");
}

// D269: the timing the player needs for each row. The SP end is one point
// (call 4), so a squeezed-out row needs nothing at or past it, even while
// the leeway still has the engine count the chord.
TEST_CASE("backend_row_timing: the timing the player needs for each row") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw == 3.0);
    // A plain row the engine counts needs nothing.
    for (double off : {-100.0, -5.0, 0.0, 2.999}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, false, lw) == 0.0);
    }
    // A plain row it does not count needs its offset early.
    CHECK(backend_row_timing(3.0, false, lw) == 3.0);
    CHECK(backend_row_timing(50.0, false, lw) == 50.0);
    // The squeezed-out row needs its SqOut's late hit.
    CHECK(backend_row_timing(-163.0, true, lw) == 163.0);
    CHECK(backend_row_timing(-5.0, true, lw) == 5.0);
    for (double off : {0.0, 2.0, 50.0}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, true, lw) <= 0.0);
    }
    // At +2 ms the squeezed-out row needs nothing, yet the engine still
    // counts the chord, so the squeeze-out still costs points.
    CHECK(core::counted_without_squeeze(2.0, lw));
    BackendSqueeze b;
    b.offset_ms = 2.0;
    CHECK(b.summarystr(true, std::nullopt, lw) == "Normal");
}

// D276 item 2: every counted plain row reads Normal.
TEST_CASE("BackendSqueeze::summarystr: every counted plain row reads Normal") {
    BackendSqueeze b;
    for (double off : {-100.0, -5.0, 2.999}) {
        CAPTURE(off);
        b.offset_ms = off;
        CHECK(b.summarystr(false, std::nullopt) == "Normal");
    }
}

// D269 call 1: one rule for every row, so a squeezed-out row and a plain row
// at the same offset differ only by what each needs.
TEST_CASE("BackendSqueeze::summarystr: one rule for the squeezed-out and the plain row") {
    BackendSqueeze b;
    b.offset_ms = -50.0;
    CHECK(b.summarystr(true, std::nullopt) == "Extreme");
    CHECK(b.summarystr(false, std::nullopt) == "Normal");
    b.offset_ms = 150.0;
    CHECK(b.summarystr(true, std::nullopt) == "Normal (uncounted)");
    CHECK(b.summarystr(false, std::nullopt) == "Insane+ (uncounted)");
}

// Finding 33, D48 Q6: a row the engine does not count says so, squeezed out
// or not, under the same test (counted_without_squeeze).
TEST_CASE("BackendSqueeze::summarystr: an uncounted squeezed-out row carries the tag") {
    BackendSqueeze b;
    b.offset_ms = 50.0;
    CHECK(b.summarystr(true, std::nullopt) == "Normal (uncounted)");
    b.offset_ms = 4.0;
    CHECK(b.summarystr(true, std::nullopt) == "Normal (uncounted)");
    CHECK(b.summarystr(true, std::nullopt, 5.0) == "Normal");
    b.offset_ms = -5.0;
    CHECK(b.summarystr(true, std::nullopt) == "Hard");
}

// D269 call 3, D48 Q3: the rows band on the report's timing tiers, and a
// timing exactly on an edge reads the band below it. The plain rows use a
// 0 ms leeway so that every offset past the end is uncounted and needs
// itself, which puts the first edge within reach.
TEST_CASE("BackendSqueeze::summarystr: the tier edges for both row kinds") {
    const struct {
        double ms;
        const char* word;
    } kEdges[] = {
        {2.0, "Normal"},     {2.001, "Hard"},      {42.5, "Hard"},
        {85.0, "Extreme"},   {127.5, "Insane"},    {170.0, "Insane+"},
        {170.001, "Beyond"},
    };
    BackendSqueeze b;
    for (const auto& e : kEdges) {
        CAPTURE(e.ms);
        b.offset_ms = -e.ms;
        CHECK(b.summarystr(true, std::nullopt) == e.word);
        b.offset_ms = e.ms;
        CHECK(b.summarystr(false, std::nullopt, 0.0) == std::string(e.word) + " (uncounted)");
    }
}

TEST_CASE("rules: the leeway changes what the engine counts") {
    // A wider leeway can only add backend points and a zero leeway can only
    // remove them. Somewhere in the corpus at least one chart must move.
    core::Rules none = core::default_rules();
    none.backend_leeway_ms = 0.0;
    core::Rules wide = core::default_rules();
    wide.backend_leeway_ms = 50.0;

    bool any_moved = false;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        auto best = [&](const core::Rules& r) {
            ScoreGraph graph(song, 4, FillDeadlineRule::Ch11, r);
            return run_search(graph, EngineOptions{}).front().totalscore();
        };
        const int64_t s_none = best(none);
        const int64_t s_default = best(core::default_rules());
        const int64_t s_wide = best(wide);
        CHECK(s_none <= s_default);
        CHECK(s_default <= s_wide);
        if (s_none != s_default || s_default != s_wide) {
            any_moved = true;
            break;
        }
    }
    CHECK(any_moved);
}

TEST_CASE("rules: max_tied_paths caps the tied paths the engine keeps") {
    core::Rules one = core::default_rules();
    one.max_tied_paths = 1;
    int charts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4, FillDeadlineRule::Ch11, one);
        std::vector<Path> paths = run_search(graph, EngineOptions{});
        REQUIRE(!paths.empty());
        CHECK(paths.front().tied_pathcount() == 1);
        if (++charts == 5) break;
    }
    CHECK(charts > 0);
}

// Finding 95, D51 call 1: the tie limit is one count per score, whichever
// side of the Path limit a path falls on, and a path inside the limit leads.
// On this chart two paths tie the top score under 1.0 fills; only the E0
// path's early fill puts it over a 0 ms limit, so the inside one is kept.
TEST_CASE("rules: max_tied_paths is one count per score, inside paths first") {
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("black midi - Sugar") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.sp_cap = 4;
    settings.legacy_fills = true;
    settings.ms_filter = 0.0;
    settings.rules.max_tied_paths = 1;
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(record.paths.empty());

    const Path& best = record.best_path();
    int at_top = 0;
    std::string seen;
    for (const Path* p : record.all_paths()) {
        if (p->totalscore() != best.totalscore()) continue;
        seen += "'" + p->pathstring() + "' ";
    }
    for (const Path& p : record.paths)
        if (p.totalscore() == best.totalscore()) at_top += p.tied_pathcount();
    INFO("paths at the top score: ", seen);
    CHECK(at_top == 1);
    CHECK(best.tied_pathcount() == 1);
    CHECK(best.pathstring() == "0 E3+ E5 E1");
}

// Finding 330, D51 call 2: a path over the Path limit is still kept when it
// ties the optimal score, and one over the limit below it is dropped.
TEST_CASE("rules: a path over the Path limit stays kept when it ties the optimal score") {
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("HopH2O - I Am... All Of Me") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.sp_cap = 4;
    settings.ms_filter = 10.0;
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(record.paths.empty());

    const int64_t top = record.best_path().totalscore();
    CHECK(top == 694985);
    bool kept = false;
    bool below_kept = false;
    std::string seen;
    for (const Path* p : record.all_paths())
        seen += "'" + p->pathstring() + "' " + std::to_string(p->totalscore()) + "; ";
    INFO("kept paths: ", seen);
    for (const Path* p : record.all_paths()) {
        if (p->pathstring() == "3 E0 1 0- 2 E0") below_kept = true;
        if (p->totalscore() != top || p->pathstring() != "0+ 2 0 0- 2 E0") continue;
        REQUIRE(p->difficulty().has_value());
        CHECK(app::format_ms(*p->difficulty()) == "78.9 ms");
        kept = true;
    }
    CHECK(kept);
    CHECK_FALSE(below_kept);
}

// D55 item 5: one count per score must not bring back over-limit paths below
// the best score. On this chart '2 2+ 0- 3' needs a 428.6 ms squeeze and ties
// the inside path '2 2+ 3 0+' below the top. It is dropped, not filed under
// that path as a tie.
TEST_CASE("rules: a path over the Path limit below the best score is dropped even as a tie") {
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Unbound (The Wild Ride)") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.sp_cap = 4;
    settings.legacy_fills = false;
    settings.ms_filter = 10.0;
    settings.rules.max_tied_paths = 4;
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(record.paths.empty());

    std::string seen;
    for (const Path* p : record.all_paths())
        seen += "'" + p->pathstring() + "' " + std::to_string(p->totalscore()) + "; ";
    INFO("kept paths: ", seen);
    bool inside_kept = false;
    bool over_kept = false;
    for (const Path* p : record.all_paths()) {
        if (p->pathstring() == "2 2+ 0- 3") over_kept = true;
        if (p->pathstring() == "2 2+ 3 0+" && p->totalscore() == 859580) inside_kept = true;
    }
    CHECK(inside_kept);
    CHECK_FALSE(over_kept);
}

// Finding 179: the engine's running tie count (bookkeeping for the limit)
// must equal the recount of the finished tree, which is what is stored.
// rebuild throws when they differ; this runs it over real charts.
TEST_CASE("rules: the engine's tied count matches the recount") {
    int charts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, ChartReading{});
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4, FillDeadlineRule::Ch11, core::default_rules());
        std::vector<Path> paths;
        REQUIRE_NOTHROW(paths = run_search(graph, EngineOptions{}));
        REQUIRE_FALSE(paths.empty());
        for (const Path& p : paths) CHECK(p.tied_pathcount() >= 1);
        if (++charts == 5) break;
    }
    CHECK(charts == 5);
}

TEST_CASE("rules: the generated-fill values come from the rules") {
    Song by_default = fill_song();
    by_default.check_activations(core::default_rules(), kTestFillRule);
    const int default_fills = fill_count(by_default);
    REQUIRE(default_fills > 0);

    core::Rules tight = core::default_rules();
    tight.fill_cooldown_measures = 2;
    Song with_tight = fill_song();
    with_tight.check_activations(tight, kTestFillRule);
    CHECK(fill_count(with_tight) > default_fills);
}

TEST_CASE("rules: the retired fill_length_measures key is read and ignored") {
    // D115: a generated fill starts on its measure line, so the key no longer
    // does anything, but a hydra_rules.ini that still sets it keeps loading.
    core::Rules r = app::load_rules_file(write_rules("retired_fill_length",
        "fill_length_measures = 0.25\n"
        "max_tied_paths = 2\n"));
    core::Rules expected = core::default_rules();
    expected.max_tied_paths = 2;
    CHECK(r.fingerprint() == expected.fingerprint());
}

// ---- the fingerprint's scope (docs/adr/0014, amended 2026-09-26) ----------

TEST_CASE("rules: the retired Auto keys are read and ignored") {
    // Auto is gone (2026-09-27), but a hydra_rules.ini that still sets its
    // two keys must keep loading: an error here would switch analysis off.
    // Any value is accepted, since the keys no longer do anything.
    core::Rules r = app::load_rules_file(write_rules("retired",
        "auto_cap_ladder = 32, 16\n"
        "auto_budget_s = banana\n"
        "max_tied_paths = 2\n"));
    CHECK(r.max_tied_paths == 2);
    core::Rules expected = core::default_rules();
    expected.max_tied_paths = 2;
    CHECK(r.fingerprint() == expected.fingerprint());
}

TEST_CASE("rules: the fingerprint text is pinned") {
    // Pinned from one run at D115 B2-3, which retired fill_length_measures
    // (until then the values were 1.8.4's, from 6a1bb49). Every stored result
    // carries one of these, so a change to the text's names, number form or
    // line order would read every row Stale.
    CHECK(core::default_rules().fingerprint() == 0xaac02b481589a70eull);
    // Every field moved off its default, so each line's name and number form
    // (whole numbers, fractions, the whole_chord word) is in the hash.
    core::Rules all;
    all.backend_leeway_ms = 5.0;
    all.sqout_rule = core::SqOutRule::WholeChord;
    all.max_tied_paths = 2;
    all.fill_cooldown_measures = 3;
    all.fill_max_distance_beats = 0.25;
    all.fill_land_slop_beats = 0.1;
    CHECK(all.fingerprint() == 0x3ea9ab5620beca19ull);
}

TEST_CASE("rules: the default stamp is built once and matches a fresh record") {
    // HydraRecord's default fingerprint used to re-hash the default rules for
    // every record built, which includes every record decoded.
    CHECK(&core::default_stamp() == &core::default_stamp());
    CHECK(core::default_stamp().fixed == core::default_rules().fingerprint());
    CHECK(HydraRecord{}.rules_fingerprint == core::default_stamp().fixed);
    CHECK(core::RulesStamp::none().fixed == core::kNoRulesFingerprint);
}

TEST_CASE("rules: a run is stamped with the rules fingerprint at cap 4 and at cap 8") {
    const Song& song = corpus::song(corpus::first_chart_with_notes(), ChartReading{});
    SearchSettings settings;
    settings.sp_cap = 8;
    CHECK(analyze_chart(song, settings).rules_fingerprint == settings.rules.fingerprint());
    settings.sp_cap = 4;
    CHECK(analyze_chart(song, settings).rules_fingerprint == settings.rules.fingerprint());
}
