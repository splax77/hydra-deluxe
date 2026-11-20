// Tests for core::Rules and hydra_rules.ini (app/rules_file.h).
//
// Every rule the user chose by hand lives in one Rules value. Defaults must
// equal the values Hydra always used, and each non-default value must
// actually change the behavior it names.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "app/config.h"
#include "app/rules_file.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/scoring.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "db_file_util.h"  // scalar_on_file
#include "parse/song.h"
#include "search/engine.h"
#include "search/graph.h"
#include "search/pather.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "testdata_dirs.h"  // testdata_root

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

// The text hydra_rules.ini takes for field f's value in r: a number in the
// %.17g form, which reads back exactly, or the sqout_rule word.
std::string field_value_text(const core::Rules& r, const core::RulesField& f) {
    char buf[64];
    if (auto real = std::get_if<double core::Rules::*>(&f.member)) {
        std::snprintf(buf, sizeof buf, "%.17g", r.**real);
        return buf;
    }
    if (auto whole = std::get_if<int core::Rules::*>(&f.member)) return std::to_string(r.**whole);
    return core::sqout_rule_name(r.*std::get<core::SqOutRule core::Rules::*>(f.member));
}

// r with field f moved off its value: one more for a number, which stays
// above every field's smallest allowed value, or the other sqout_rule.
core::Rules with_field_moved(core::Rules r, const core::RulesField& f) {
    if (auto real = std::get_if<double core::Rules::*>(&f.member)) {
        r.**real += 1.0;
    } else if (auto whole = std::get_if<int core::Rules::*>(&f.member)) {
        r.**whole += 1;
    } else {
        const auto sqout = std::get<core::SqOutRule core::Rules::*>(f.member);
        r.*sqout = r.*sqout == core::SqOutRule::FirstNote ? core::SqOutRule::WholeChord
                                                          : core::SqOutRule::FirstNote;
    }
    return r;
}

// The line hydra_rules.ini takes to set field f to its value in r.
std::string field_line(const core::Rules& r, const core::RulesField& f) {
    return std::string(f.name) + " = " + field_value_text(r, f) + "\n";
}

// One Backend timings row the search kept, rated as the Paths tab rates it,
// with the chart file it came from.
struct RealRow {
    std::string chart;
    BackendRating rating;
};

// Every Backend timings row on every path the search keeps
// (HydraRecord::all_paths), under the app's default settings, for every
// chart library discovery lists (corpus::chart_paths). A chart with no notes
// under those settings has no rows. Built once per run.
const std::vector<RealRow>& real_backend_rows() {
    static const std::vector<RealRow> all = [] {
        const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
        std::vector<RealRow> out;
        for (const std::string& chart : corpus::chart_paths()) {
            if (corpus::song(chart, cfg.reading(), cfg.rules).is_empty()) continue;
            for (const Path* p : corpus::analyzed(chart, cfg).all_paths())
                for (const Activation& a : p->all_activations())
                    for (const BackendRating& br :
                         rate_activation(a, cfg.rules.backend_leeway_ms).backends)
                        out.push_back({chart, br});
        }
        return out;
    }();
    return all;
}

// The next double above ms: a hair past an edge that a <= or < test draws.
double just_above(double ms) {
    return std::nextafter(ms, std::numeric_limits<double>::infinity());
}

// What follows the tier name `band` in a Rating word: empty when the word is
// the band alone, nothing when the word does not start with it.
std::optional<std::string> tag_after(const std::string& word, const std::string& band) {
    if (word.compare(0, band.size(), band) != 0) return std::nullopt;
    return word.substr(band.size());
}

int fill_count(const Song& song) {
    int n = 0;
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) ++n;
    return n;
}

// Is p over the Path limit the record was searched under? The limit is the
// record's own (HydraRecord::ms_limit) and the judge is Path::within_ms_limit,
// the rule the search's Path limit asks. No limit means nothing is over.
bool over_limit(const HydraRecord& record, const Path& p) {
    return record.ms_limit.has_value() && !p.within_ms_limit(*record.ms_limit);
}

// Every path the record keeps, with its score and whether it is over the
// Path limit, for a failure message.
std::string kept_paths_text(const HydraRecord& record) {
    std::string seen;
    for (const Path* p : record.all_paths())
        seen += "'" + p->pathstring() + "' " + std::to_string(p->totalscore()) +
                (over_limit(record, *p) ? " over" : " inside") + "; ";
    return seen;
}

// D51 call 2 and D55 item 5: a kept path over the Path limit always has the
// best score; one below it is dropped, tie or not.
void check_over_limit_paths_tie_the_top(const HydraRecord& record) {
    const int64_t top = record.best_path().totalscore();
    for (const Path* p : record.all_paths()) {
        if (!over_limit(record, *p)) continue;
        CAPTURE(p->pathstring());
        CHECK(p->totalscore() == top);
    }
}

}  // namespace

// Each default comes from its owner (the Rules member initialisers, walked
// through rules_fields), and a hydra_rules.ini that writes it back loads the
// same rules, so no default sits outside what the file may set.
TEST_CASE("rules: every default is a value hydra_rules.ini accepts") {
    const core::Rules& defaults = core::default_rules();
    std::string text;
    for (const core::RulesField& f : core::rules_fields()) {
        CAPTURE(f.name);
        const core::Rules one =
            app::load_rules_file(write_rules("default_one", field_line(defaults, f)));
        CHECK(one.fingerprint() == defaults.fingerprint());
        text += field_line(defaults, f);
    }
    CHECK(app::load_rules_file(write_rules("default_all", text)).fingerprint() ==
          defaults.fingerprint());
}

TEST_CASE("rules: a missing file and an empty file both load the defaults") {
    const uint64_t fp = core::default_rules().fingerprint();
    CHECK(app::load_rules_file(
              hydra::os_path(testtemp::temp_path("rules_does_not_exist", ".ini")))
              .fingerprint() == fp);
    CHECK(app::load_rules_file(write_rules("empty", "# nothing here\n\n")).fingerprint() == fp);
}

// Every field rules_fields lists, so a field added there is read here too.
// Each key, written alone at a value off its default, sets its own field and
// moves the fingerprint; all of them in one file each set their own.
TEST_CASE("rules: every key in the file is read") {
    const core::Rules& defaults = core::default_rules();
    core::Rules all = defaults;
    std::string text;
    for (const core::RulesField& f : core::rules_fields()) {
        CAPTURE(f.name);
        const core::Rules moved = with_field_moved(defaults, f);
        REQUIRE(field_value_text(moved, f) != field_value_text(defaults, f));
        const core::Rules r = app::load_rules_file(write_rules("key_one", field_line(moved, f)));
        CHECK(field_value_text(r, f) == field_value_text(moved, f));
        CHECK(r.fingerprint() == moved.fingerprint());
        CHECK(r.fingerprint() != defaults.fingerprint());
        all = with_field_moved(all, f);
        text += field_line(all, f);
    }
    const core::Rules r = app::load_rules_file(write_rules("key_all", text));
    for (const core::RulesField& f : core::rules_fields()) {
        CAPTURE(f.name);
        CHECK(field_value_text(r, f) == field_value_text(all, f));
    }
    CHECK(r.fingerprint() == all.fingerprint());
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

TEST_CASE("rules: whole_chord takes every note's SP doubling on a squeeze-out") {
    hydra::Chord chord;  // qualified: windows.h (through temp_util.h) declares a Chord too
    chord.add_note(NoteColor::Red);
    chord.add_note(NoteColor::Blue);
    // Each note loses its own doubling: the whole value it is paid without
    // Star Power, the sum of its non-SP categories.
    std::vector<CategoryScores> per_note;
    const int whole =
        category_scores(chord, 0, &per_note, core::SqOutRule::WholeChord).sqout_reduction;
    REQUIRE(per_note.size() == 2);
    int sum = 0;
    for (const CategoryScores& n : per_note) {
        const int paid = n.notes + n.combo + n.accent + n.ghost;
        CHECK(paid > 0);
        CHECK(n.sqout_reduction == paid);
        sum += n.sqout_reduction;
    }
    CHECK(whole == sum);
    // first_note takes the first note's doubling only.
    CHECK(category_scores(chord, 0, nullptr, core::SqOutRule::FirstNote).sqout_reduction ==
          per_note[0].sqout_reduction);
    CHECK(whole > per_note[0].sqout_reduction);
}

// The leeway decides only whether a plain row is counted (D269 call 4). On
// every real plain row past its SP end, a leeway just above its offset counts
// it, so it needs nothing and reads the first tier with no tag; a leeway on
// its offset leaves it uncounted, banded on that offset and tagged.
TEST_CASE("rules: the leeway decides whether a plain backend row is counted") {
    const std::vector<TimingTier> tiers = timing_tiers();
    std::set<std::string> tags;
    int past_end = 0;
    for (const RealRow& rr : real_backend_rows()) {
        const double off = rr.rating.row.offset();
        if (rr.rating.squeezed_out || core::paid_by_sp_walk(off)) continue;
        ++past_end;
        CAPTURE(rr.chart);
        CAPTURE(off);
        CHECK(rr.rating.row.summarystr(false, std::nullopt, just_above(off)) == tiers.front().name);
        const std::optional<std::string> tag = tag_after(
            rr.rating.row.summarystr(false, std::nullopt, off), tier_for(off, tiers).first);
        REQUIRE(tag.has_value());
        CHECK_FALSE(tag->empty());
        tags.insert(*tag);
    }
    REQUIRE(past_end > 0);
    CHECK(tags.size() == 1);
}

// D269: the timing the player needs for each row. The SP end is one point
// (call 4). Inside SP a squeezed-out row needs its distance from that end;
// past the end it needs what a plain row there needs. Every edge is placed
// from the leeway's owner, Rules::backend_leeway_ms.
TEST_CASE("backend_row_timing: the timing the player needs for each row") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw > 0.0);
    // The last offset the strict edge (D29) still counts.
    const double just_inside = std::nextafter(lw, 0.0);
    // A plain row the engine counts needs nothing.
    for (double off : {-100.0, -5.0, 0.0, lw / 2, just_inside}) {
        CAPTURE(off);
        REQUIRE(core::counted_without_squeeze(off, lw));
        CHECK(backend_row_timing(off, false, lw) == 0.0);
    }
    // A plain row it does not count needs its offset early.
    for (double off : {lw, lw + 50.0}) {
        CAPTURE(off);
        REQUIRE_FALSE(core::counted_without_squeeze(off, lw));
        CHECK(backend_row_timing(off, false, lw) == off);
    }
    // Inside SP the squeezed-out row needs its SqOut's late hit: how far it
    // sits before the end.
    for (double off : {-163.0, -5.0}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, true, lw) == -off);
    }
    // On the end it needs a 0 ms hit, printed as +0.0, never -0.0.
    CHECK(backend_row_timing(0.0, true, lw) == 0.0);
    CHECK_FALSE(std::signbit(backend_row_timing(0.0, true, lw)));
    // Past the end but within the leeway it needs nothing, as a counted plain
    // row does.
    for (double off : {lw / 2, just_inside}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, true, lw) == 0.0);
        CHECK(backend_row_timing(off, true, lw) == backend_row_timing(off, false, lw));
    }
    // At or past the leeway it needs its offset, the early hit that would
    // pull it back into SP: the same figure an uncounted plain row needs.
    for (double off : {lw, lw + 50.0, lw + 478.7}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, true, lw) == off);
        CHECK(backend_row_timing(off, true, lw) == backend_row_timing(off, false, lw));
    }
    // Every past-end row follows the plain rule whatever the leeway.
    for (double off : {lw / 2, lw, lw + 50.0, lw + 478.7}) {
        CAPTURE(off);
        CHECK(backend_row_timing(off, true, 0.0) == backend_row_timing(off, false, 0.0));
    }
    // Within the leeway the squeezed-out row needs nothing, so it reads the
    // first tier, and the engine still counts the chord, so the word carries
    // no tag.
    BackendSqueeze b;
    b.offset_ms = lw / 2;
    CHECK(b.summarystr(true, std::nullopt, lw) == timing_tiers().front().name);
}

// D276 item 2: every counted plain row reads the first tier, Normal, with
// the eff. figure the Paths tab shows beside it. The rows are every plain row
// the search keeps on the corpus that the engine counts.
TEST_CASE("BackendSqueeze::summarystr: every counted plain row reads Normal") {
    const double lw = core::default_rules().backend_leeway_ms;
    const std::string first = timing_tiers().front().name;
    int counted = 0;
    for (const RealRow& rr : real_backend_rows()) {
        const double off = rr.rating.row.offset();
        if (rr.rating.squeezed_out || !core::counted_without_squeeze(off, lw)) continue;
        ++counted;
        CAPTURE(rr.chart);
        CAPTURE(off);
        CHECK(rr.rating.row.summarystr(false, rr.rating.note.effective_ms, lw) == first);
    }
    REQUIRE(counted > 0);
}

// D269 call 1: one rule for every row, so a squeezed-out row and a plain row
// at the same offset differ only by what each needs. On every real row read
// both ways: inside SP only the squeezed-out reading needs a timing, its
// distance from the end; past the leeway both need the same one.
TEST_CASE("BackendSqueeze::summarystr: one rule for the squeezed-out and the plain row") {
    const double lw = core::default_rules().backend_leeway_ms;
    const std::vector<TimingTier> tiers = timing_tiers();
    int inside = 0, past = 0;
    for (const RealRow& rr : real_backend_rows()) {
        const double off = rr.rating.row.offset();
        CAPTURE(rr.chart);
        CAPTURE(off);
        const std::string as_sqout = rr.rating.row.summarystr(true, std::nullopt, lw);
        const std::string as_plain = rr.rating.row.summarystr(false, std::nullopt, lw);
        if (core::paid_by_sp_walk(off)) {
            ++inside;
            CHECK(as_plain == tiers.front().name);
            CHECK(as_sqout == tier_for(-off, tiers).first);
        } else if (!core::counted_without_squeeze(off, lw)) {
            ++past;
            CHECK(as_sqout == as_plain);
        }
    }
    REQUIRE(inside > 0);
    REQUIRE(past > 0);
}

// Finding 33, D48 Q6: a row the engine does not count says so, squeezed out
// or not, under the same test (counted_without_squeeze), and the same tag.
// Once the leeway is past it, the engine counts it and the tag goes. The
// real squeezed-out rows carry the tag exactly when the engine does not
// count them.
TEST_CASE("BackendSqueeze::summarystr: an uncounted squeezed-out row carries the tag") {
    const double lw = core::default_rules().backend_leeway_ms;
    const std::vector<TimingTier> tiers = timing_tiers();
    std::set<std::string> tags;
    int uncounted = 0, squeezed = 0;
    for (const RealRow& rr : real_backend_rows()) {
        const BackendSqueeze& row = rr.rating.row;
        const double off = row.offset();
        CAPTURE(rr.chart);
        CAPTURE(off);
        const bool counted = core::counted_without_squeeze(off, lw);
        if (rr.rating.squeezed_out) {
            ++squeezed;
            const std::optional<std::string> tag =
                tag_after(row.summarystr(true, std::nullopt, lw),
                          tier_for(backend_row_timing(off, true, lw), tiers).first);
            REQUIRE(tag.has_value());
            CHECK(tag->empty() == counted);
        }
        if (counted) continue;
        ++uncounted;
        for (bool squeezed_out : {true, false}) {
            CAPTURE(squeezed_out);
            const std::optional<std::string> tag =
                tag_after(row.summarystr(squeezed_out, std::nullopt, lw),
                          tier_for(backend_row_timing(off, squeezed_out, lw), tiers).first);
            REQUIRE(tag.has_value());
            CHECK_FALSE(tag->empty());
            tags.insert(*tag);
        }
        CHECK(row.summarystr(true, std::nullopt, just_above(off)) == tiers.front().name);
    }
    REQUIRE(squeezed > 0);
    REQUIRE(uncounted > 0);
    CHECK(tags.size() == 1);
}

// D269 call 3, D48 Q3: the rows band on the report's timing tiers, and a
// timing exactly on an edge reads the band below it. Every edge is read from
// the tiers' owner, timing_tiers: on the edge a row reads that tier, and a
// hair past it the next one (Beyond past the last edge). The plain rows use
// a 0 ms leeway so that every offset past the end is uncounted and needs
// itself, which puts the first edge within reach; each carries the same tag.
TEST_CASE("BackendSqueeze::summarystr: the tier edges for both row kinds") {
    const std::vector<TimingTier> tiers = timing_tiers();
    std::set<std::string> plain_tags;
    int edges = 0;
    BackendSqueeze b;
    for (size_t i = 0; i < tiers.size(); ++i) {
        if (!tiers[i].cutoff) continue;
        ++edges;
        const double edge = *tiers[i].cutoff;
        const bool next_banded = i + 1 < tiers.size() && tiers[i + 1].cutoff.has_value();
        const std::string next = next_banded ? tiers[i + 1].name : beyond_tier(tiers).name;
        const std::vector<std::pair<double, std::string>> cases = {
            {edge, std::string(tiers[i].name)},
            {just_above(edge), next},
        };
        for (const std::pair<double, std::string>& c : cases) {
            CAPTURE(c.first);
            b.offset_ms = -c.first;
            CHECK(b.summarystr(true, std::nullopt) == c.second);
            b.offset_ms = c.first;
            const std::optional<std::string> tag =
                tag_after(b.summarystr(false, std::nullopt, 0.0), c.second);
            REQUIRE(tag.has_value());
            CHECK_FALSE(tag->empty());
            plain_tags.insert(*tag);
        }
    }
    REQUIRE(edges > 0);
    CHECK(plain_tags.size() == 1);
}

// The leeway reaches the search. Pitch Dark (testdata/input/
// test_pitch_dark_ratio) keeps plain rows past its SP ends, beyond the
// default leeway, on its best path: the Paths tab reads them "(uncounted)".
// A leeway just past the farthest of them makes the engine count them all.
// That same path is still there, now worth more, and the search keeps the
// best path, so the best score must rise. A zero leeway counts less, so its
// best score is never higher than the default's.
TEST_CASE("rules: the leeway changes what the engine counts") {
    const std::string chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(chart.empty());
    const Song& song = corpus::song(chart, ChartReading{});
    REQUIRE_FALSE(song.is_empty());
    auto best = [&](const core::Rules& r) {
        ScoreGraph graph(song, 4, FillDeadlineRule::Ch11, r);
        std::vector<Path> paths = run_search(graph, EngineOptions{});
        REQUIRE_FALSE(paths.empty());
        return paths.front();
    };
    const core::Rules& defaults = core::default_rules();
    const Path by_default = best(defaults);

    // The best path's plain rows the default leeway leaves uncounted, with
    // the SP points each would add once counted.
    double farthest = 0.0;
    int uncounted = 0;
    for (const Activation& a : by_default.all_activations())
        for (const BackendSqueeze& b : a.display_backends()) {
            if (a.is_sqout_backend(b) || b.sp_points <= 0) continue;
            if (core::counted_without_squeeze(b.offset(), defaults.backend_leeway_ms)) continue;
            ++uncounted;
            farthest = std::max(farthest, b.offset());
        }
    REQUIRE(uncounted > 0);

    core::Rules wide = defaults;
    wide.backend_leeway_ms = just_above(farthest);
    core::Rules none = defaults;
    none.backend_leeway_ms = 0.0;
    CHECK(best(wide).totalscore() > by_default.totalscore());
    CHECK(best(none).totalscore() <= by_default.totalscore());
}

// Finding 95, D51 call 1: the tie limit is one count per score, whichever
// side of the Path limit a path falls on, and a path inside the limit leads.
// On this chart, under 1.0 fills and a 0 ms Path limit, the top score is
// reached both by a path inside the limit and by one over it.
TEST_CASE("rules: max_tied_paths is one count per score, inside paths first") {
    const std::string chart = corpus::chart_named("black midi - Sugar");
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.legacy_fills = true;
    settings.ms_filter = 0.0;

    // Under the default tie limit both sides of the top score are kept, so
    // the case the inside-first rule decides is there.
    const HydraRecord& wide = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(wide.paths.empty());
    const int64_t top = wide.best_path().totalscore();
    bool top_inside = false;
    bool top_over = false;
    for (const Path* p : wide.all_paths()) {
        if (p->totalscore() != top) continue;
        if (over_limit(wide, *p)) top_over = true;
        else top_inside = true;
    }
    INFO("paths under the default tie limit: ", kept_paths_text(wide));
    REQUIRE(top_inside);
    REQUIRE(top_over);

    settings.rules.max_tied_paths = 1;
    const HydraRecord& one = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(one.paths.empty());
    INFO("paths under a tie limit of 1: ", kept_paths_text(one));
    REQUIRE(one.best_path().totalscore() == top);
    int at_top = 0;
    for (const Path& p : one.paths)
        if (p.totalscore() == top) at_top += p.tied_pathcount();
    CHECK(at_top == 1);
    CHECK(one.best_path().tied_pathcount() == 1);
    CHECK_FALSE(over_limit(one, one.best_path()));
}

// Finding 330, D51 call 2: a path over the Path limit is still kept when it
// ties the optimal score, and one over the limit below it is dropped.
TEST_CASE("rules: a path over the Path limit stays kept when it ties the optimal score") {
    const std::string chart = corpus::chart_named("HopH2O - I Am... All Of Me");
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.ms_filter = 10.0;
    const HydraRecord& record = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(record.paths.empty());
    INFO("kept paths: ", kept_paths_text(record));

    const int64_t top = record.best_path().totalscore();
    bool over_tie_kept = false;
    for (const Path* p : record.all_paths())
        if (p->totalscore() == top && over_limit(record, *p)) over_tie_kept = true;
    CHECK(over_tie_kept);
    check_over_limit_paths_tie_the_top(record);
}

// D55 item 5: one count per score must not bring back over-limit paths below
// the best score. With no Path limit this chart keeps, below its top score, a
// path over 10 ms that ties one inside it. Under a 10 ms limit the over one
// is dropped, not filed under the inside one as a tie, and the inside one
// stays.
TEST_CASE("rules: a path over the Path limit below the best score is dropped even as a tie") {
    const std::string chart = corpus::chart_named("Unbound (The Wild Ride)");
    REQUIRE_FALSE(chart.empty());

    SearchSettings settings;
    settings.ms_filter = 10.0;
    const HydraRecord& limited = corpus::analyzed(chart, settings);
    REQUIRE_FALSE(limited.paths.empty());
    const int64_t top = limited.best_path().totalscore();

    SearchSettings unlimited = settings;
    unlimited.ms_filter.reset();
    const HydraRecord& free_record = corpus::analyzed(chart, unlimited);
    std::set<int64_t> inside_scores;
    std::set<int64_t> over_scores;
    for (const Path* p : free_record.all_paths()) {
        if (p->totalscore() >= top) continue;
        if (p->within_ms_limit(*settings.ms_filter)) inside_scores.insert(p->totalscore());
        else over_scores.insert(p->totalscore());
    }
    std::vector<int64_t> tied_below;
    std::set_intersection(inside_scores.begin(), inside_scores.end(), over_scores.begin(),
                          over_scores.end(), std::back_inserter(tied_below));
    INFO("paths with no Path limit: ", kept_paths_text(free_record));
    INFO("paths under the 10 ms limit: ", kept_paths_text(limited));
    REQUIRE_FALSE(tied_below.empty());

    check_over_limit_paths_tie_the_top(limited);
    for (const int64_t score : tied_below) {
        CAPTURE(score);
        bool inside_kept = false;
        for (const Path* p : limited.all_paths())
            if (p->totalscore() == score && !over_limit(limited, *p)) inside_kept = true;
        CHECK(inside_kept);
    }
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

// ---- the fingerprint's scope (docs/adr/0014) ------------------------------

// Every stored result carries the rules fingerprint, so a change to the
// fingerprint text's names, number form or line order would read every row
// of a player's library Stale. testdata/store/v2.12.1_library.db is a real
// library an earlier Hydra wrote under the default rules: each of its
// profiles must still carry the fingerprint the store asks of a Ready row
// today (store::ready_rules_fp).
TEST_CASE("rules: a library an earlier Hydra wrote still matches the default rules") {
    const std::string source = corpus::testdata_root() + "/store/v2.12.1_library.db";
    REQUIRE(std::filesystem::exists(hydra::os_path(source)));
    const testtemp::ScopedFile db("rules_v2121_library", ".db");
    std::filesystem::copy_file(hydra::os_path(source), hydra::os_path(db.path),
                               std::filesystem::copy_options::overwrite_existing);
    const std::vector<uint8_t> want = store::ready_rules_fp(core::default_stamp());
    const std::string blob = "X'" + hex_lower(want.data(), want.size()) + "'";
    const int64_t profiles = test::scalar_on_file(db.path, "SELECT COUNT(*) FROM profiles");
    REQUIRE(profiles > 0);
    CHECK(test::scalar_on_file(db.path, "SELECT COUNT(*) FROM profiles WHERE rules_fp = " +
                                            blob) == profiles);
}

TEST_CASE("rules: the default stamp is the default rules' fingerprint, and a fresh record carries it") {
    CHECK(core::default_stamp().fixed == core::default_rules().fingerprint());
    CHECK(HydraRecord{}.rules_fingerprint == core::default_stamp().fixed);
    CHECK(core::RulesStamp::none().fixed == core::kNoRulesFingerprint);
}

// A rule moved off its default, so a stamp that wrote the default rules'
// fingerprint instead of the run's own would fail.
TEST_CASE("rules: a run is stamped with the fingerprint of the rules it ran under") {
    const Song& song = corpus::song(corpus::first_chart_with_notes(), ChartReading{});
    SearchSettings settings;
    settings.rules.max_tied_paths = core::default_rules().max_tied_paths + 1;
    REQUIRE(settings.rules.fingerprint() != core::default_rules().fingerprint());
    CHECK(analyze_chart(song, settings).rules_fingerprint == settings.rules.fingerprint());
}
