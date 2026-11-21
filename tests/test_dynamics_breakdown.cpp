// Tests for app/dynamics_breakdown: per-pad ghost/accent/normal counting.

#include "doctest.h"

#include <array>
#include <string>
#include <vector>

#include "app/display_format.h"  // kFactSeparator
#include "app/dynamics_breakdown.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "parse/song.h"
#include "ui/library_parts.h"  // kBass2xName
#include "corpus_util.h"

using namespace hydra;
using namespace hydra::app;

// ---------------------------------------------------------------------------
// Helper: build a minimal Song with hand-picked notes.
// ---------------------------------------------------------------------------

namespace {

// Append a single-note chord to the song at the given tick.
void add_note(Song& song, int64_t tick, NoteColor color,
              NoteDynamicType dyn = NoteDynamicType::Normal,
              NoteCymbalType cym = NoteCymbalType::Normal,
              bool is2x = false) {
    SongTimestamp ts;
    ts.timecode = song.timecode(tick);
    ChordNote& n = ts.chord.add_note(color);
    n.dynamictype = dyn;
    n.cymbaltype = cym;
    n.is2x = is2x;
    song.sequence.push_back(ts);
}

Song make_test_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    song.dynamics_enabled = true;

    // Red snare: 2 ghost, 1 accent, 3 normal
    add_note(song, 0,    NoteColor::Red, NoteDynamicType::Ghost);
    add_note(song, 480,  NoteColor::Red, NoteDynamicType::Ghost);
    add_note(song, 960,  NoteColor::Red, NoteDynamicType::Accent);
    add_note(song, 1440, NoteColor::Red);
    add_note(song, 1920, NoteColor::Red);
    add_note(song, 2400, NoteColor::Red);

    // Yellow cymbal: 1 accent, 2 normal
    add_note(song, 2880, NoteColor::Yellow, NoteDynamicType::Accent,
             NoteCymbalType::Cymbal);
    add_note(song, 3360, NoteColor::Yellow, NoteDynamicType::Normal,
             NoteCymbalType::Cymbal);
    add_note(song, 3840, NoteColor::Yellow, NoteDynamicType::Normal,
             NoteCymbalType::Cymbal);

    // Yellow tom: 1 normal
    add_note(song, 4320, NoteColor::Yellow, NoteDynamicType::Normal,
             NoteCymbalType::Normal);

    // Blue tom: 1 ghost
    add_note(song, 4800, NoteColor::Blue, NoteDynamicType::Ghost,
             NoteCymbalType::Normal);

    // Kick: 2 normal, 1 ghost
    add_note(song, 5280, NoteColor::Kick);
    add_note(song, 5760, NoteColor::Kick);
    add_note(song, 6240, NoteColor::Kick, NoteDynamicType::Ghost);

    // Kick 2x: 1 normal
    add_note(song, 6720, NoteColor::Kick, NoteDynamicType::Normal,
             NoteCymbalType::Normal, /*is2x=*/true);

    return song;
}

constexpr size_t kRowCount = static_cast<size_t>(DynamicsRow::Count);

// One corpus chart's Dynamics count, beside a tally taken note by note off
// the same song: how many notes each row was sent, how many of those carry a
// ghost or accent mark, the song's notes by mark, and how many notes went to
// a row of another lane.
struct CorpusCount {
    std::string path;
    DynamicsBreakdown bd;
    std::array<int, kRowCount> notes{};
    std::array<int, kRowCount> marked{};
    DynamicsCounts song_notes;
    int wrong_lane = 0;
};

// Every corpus chart, read the way the Dynamics tab counts it
// (load_dynamics_song) at the default reading: Expert, Pro Drums on. Parsed
// once and shared by every test below.
const std::vector<CorpusCount>& corpus_counts() {
    static const std::vector<CorpusCount> counts = [] {
        std::vector<CorpusCount> out;
        for (const std::string& path : corpus::chart_paths()) {
            const Song song = load_dynamics_song(path, ChartReading{});
            CorpusCount c;
            c.path = path;
            c.bd = count_dynamics(song);
            for (const SongTimestamp& ts : song.sequence) {
                for (const ChordNote& note : ts.chord.notes()) {
                    const DynamicsRow r = dynamics_row_for(note);
                    const size_t i = static_cast<size_t>(r);
                    ++c.notes[i];
                    if (dynamics_row_info(r).color != note.colortype) ++c.wrong_lane;
                    switch (note.dynamictype) {
                        case NoteDynamicType::Ghost:
                            ++c.song_notes.ghost;
                            ++c.marked[i];
                            break;
                        case NoteDynamicType::Accent:
                            ++c.song_notes.accent;
                            ++c.marked[i];
                            break;
                        case NoteDynamicType::Normal:
                            ++c.song_notes.normal;
                            break;
                    }
                }
            }
            out.push_back(c);
        }
        return out;
    }();
    return counts;
}

// The lines a two-part line can be: the Kicks line with 2x Bass on and off on
// every corpus chart, and the 2x kick row's name in all four settings.
std::vector<TwoPartLine> real_two_part_lines() {
    std::vector<TwoPartLine> lines;
    for (const CorpusCount& c : corpus_counts()) {
        lines.push_back(dynamics_kicks_heading(c.bd, true));
        lines.push_back(dynamics_kicks_heading(c.bd, false));
    }
    for (const bool pro : {true, false})
        for (const bool bass2x : {true, false})
            lines.push_back(dynamics_kick2x_row_label(pro, bass2x));
    return lines;
}

// The words of the 2x kick row's tag with 2x Bass off, without the brackets
// the row's name puts round them.
std::string kick2x_row_tag_words() {
    const std::string tag = dynamics_kick2x_row_label(true, false).tail;
    REQUIRE(tag.size() > 2);
    REQUIRE(tag.front() == '(');
    REQUIRE(tag.back() == ')');
    return tag.substr(1, tag.size() - 2);
}

}  // namespace

// ---------------------------------------------------------------------------
// A hand-built Song gives the expected per-row counts.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: hand-built song row counts") {
    const Song song = make_test_song();
    const DynamicsBreakdown bd = count_dynamics(song);

    CHECK(bd.dynamics_enabled);

    // Red snare
    CHECK(bd.row(DynamicsRow::RedSnare).ghost  == 2);
    CHECK(bd.row(DynamicsRow::RedSnare).accent == 1);
    CHECK(bd.row(DynamicsRow::RedSnare).normal == 3);
    CHECK(bd.row(DynamicsRow::RedSnare).all()  == 6);

    // Yellow cymbal
    CHECK(bd.row(DynamicsRow::YellowCymbal).ghost  == 0);
    CHECK(bd.row(DynamicsRow::YellowCymbal).accent == 1);
    CHECK(bd.row(DynamicsRow::YellowCymbal).normal == 2);

    // Yellow tom
    CHECK(bd.row(DynamicsRow::YellowTom).all() == 1);

    // Blue tom (ghost)
    CHECK(bd.row(DynamicsRow::BlueTom).ghost == 1);
    CHECK(bd.row(DynamicsRow::BlueTom).all() == 1);

    // Blue cymbal, Green cymbal, Green tom — untouched
    CHECK(bd.row(DynamicsRow::BlueCymbal).all()  == 0);
    CHECK(bd.row(DynamicsRow::GreenCymbal).all() == 0);
    CHECK(bd.row(DynamicsRow::GreenTom).all()    == 0);

    // Kick: 1 ghost + 2 normal
    CHECK(bd.row(DynamicsRow::Kick).ghost  == 1);
    CHECK(bd.row(DynamicsRow::Kick).normal == 2);
    CHECK(bd.row(DynamicsRow::Kick).all()  == 3);

    // Kick 2x: 1 normal
    CHECK(bd.row(DynamicsRow::Kick2x).normal == 1);
    CHECK(bd.row(DynamicsRow::Kick2x).all()  == 1);

    CHECK(bd.row(DynamicsRow::RedSnare).has_dynamics());
    CHECK_FALSE(bd.row(DynamicsRow::GreenTom).has_dynamics());
}

// ---------------------------------------------------------------------------
// played_total with and without bass2x, on every corpus chart.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: played_total includes/excludes 2x kick") {
    int charts_with_2x = 0;
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        const DynamicsBreakdown& bd = c.bd;
        const DynamicsCounts& kick = bd.row(DynamicsRow::Kick);
        const DynamicsCounts& kick2x = bd.row(DynamicsRow::Kick2x);
        // With 2x Bass off only the Kick row counts; with it on the 2x kick
        // row is added, and that is the only difference in the totals.
        CHECK(bd.kicks_total(false) == kick);
        DynamicsCounts both = kick;
        both += kick2x;
        CHECK(bd.kicks_total(true) == both);
        DynamicsCounts off_plus_2x = bd.played_total(false);
        off_plus_2x += kick2x;
        CHECK(bd.played_total(true) == off_plus_2x);
        if (kick2x.all() > 0) ++charts_with_2x;
    }
    // Acid Romance and The Decade Of Statues have 2x kicks, so the two totals
    // differ somewhere.
    CHECK(charts_with_2x > 0);
}

// ---------------------------------------------------------------------------
// Every corpus chart's rows hold each of its notes once.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: every corpus chart's rows hold each of its notes once") {
    int charts_with_dynamics = 0;
    std::array<bool, kRowCount> row_filled{};
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        // Every note sits in a row of its own lane, and the rows together
        // hold the song's ghosts, accents and normals, no note lost or added.
        CHECK(c.wrong_lane == 0);
        CHECK(c.bd.played_total(true) == c.song_notes);
        if (c.bd.dynamics_enabled && c.bd.played_total(true).has_dynamics())
            ++charts_with_dynamics;
        for (size_t i = 0; i < kRowCount; ++i)
            if (c.bd.rows[i].all() > 0) row_filled[i] = true;
    }
    // Acid Romance and The Decade Of Statues carry the dynamics tag and mark
    // ghosts and accents; The Decade Of Statues has notes in every row.
    CHECK(charts_with_dynamics > 0);
    for (size_t i = 0; i < kRowCount; ++i) {
        CAPTURE(i);
        CHECK(row_filled[i]);
    }
}

// ---------------------------------------------------------------------------
// A MIDI chart without ENABLE_CHART_DYNAMICS has dynamics_enabled false.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: no ENABLE_CHART_DYNAMICS tag") {
    const std::string path =
        corpus::root() + "/test_flammarker/flammarker_authored.mid";
    const Song song = load_songpath_mid(path, ChartReading{});

    CHECK_FALSE(song.dynamics_enabled);

    const DynamicsBreakdown bd = count_dynamics(song);
    CHECK_FALSE(bd.dynamics_enabled);
}

// ---------------------------------------------------------------------------
// The row table.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: every row's own note counts in that row") {
    for (int i = 0; i < static_cast<int>(DynamicsRow::Count); ++i) {
        const DynamicsRow r = static_cast<DynamicsRow>(i);
        INFO("row " << i);
        CHECK(dynamics_row_for(dynamics_row_note(r)) == r);
    }
}

TEST_CASE("dynamics_breakdown: the 2x kick line counts its noun from the total") {
    // On every corpus chart the line names the chart's 2x kicks, then every
    // kick note (counted picks the noun from that total), then the 2x share
    // of them, in that order.
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        const int twice = c.bd.row(DynamicsRow::Kick2x).all();
        const int total = c.bd.kicks_total(true).all();
        const std::string line = dynamics_kick2x_line(c.bd);
        CAPTURE(line);
        const std::string twice_text = group_thousands(twice);
        const std::string total_text = counted(total, "kick note", "kick notes");
        const size_t at_twice = line.find(twice_text);
        REQUIRE(at_twice != std::string::npos);
        const size_t at_total = line.find(total_text, at_twice + twice_text.size());
        REQUIRE(at_total != std::string::npos);
        CHECK(line.find(dynamics_share(twice, total), at_total + total_text.size()) !=
              std::string::npos);
    }
}

TEST_CASE("dynamics_breakdown: the 2x kick status names the 2x Bass setting") {
    // Both cases name the 2x Bass box, they differ, and only the off case
    // says what the 2x kick row's tag says.
    const std::string on = dynamics_kick2x_status(true);
    const std::string off = dynamics_kick2x_status(false);
    CHECK(on.find(ui::detail::kBass2xName) != std::string::npos);
    CHECK(off.find(ui::detail::kBass2xName) != std::string::npos);
    CHECK(on != off);
    const std::string tag = kick2x_row_tag_words();
    CHECK(off.find(tag) != std::string::npos);
    CHECK(on.find(tag) == std::string::npos);
}

TEST_CASE("dynamics_breakdown: the header's 2x kick fact, empty with no 2x kicks") {
    // On every corpus chart the fact is empty exactly when the chart has no
    // 2x kicks; otherwise it starts with the 2x kick count and carries their
    // share of every kick note.
    int with_2x = 0;
    int kicks_without_2x = 0;
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        const int twice = c.bd.row(DynamicsRow::Kick2x).all();
        const int total = c.bd.kicks_total(true).all();
        const std::string fact = dynamics_kick2x_fact(c.bd);
        CAPTURE(fact);
        if (twice == 0) {
            CHECK(fact.empty());
            if (total > 0) ++kicks_without_2x;
            continue;
        }
        ++with_2x;
        CHECK(starts_with(fact, group_thousands(twice)));
        CHECK(fact.find(dynamics_share(twice, total)) != std::string::npos);
    }
    // Acid Romance has 2x kicks; How You Remind Me's Expert drums have kicks
    // (note 0) and no 2x kick (note 32), so both cases are on real charts.
    CHECK(with_2x > 0);
    CHECK(kicks_without_2x > 0);
}

TEST_CASE("dynamics_breakdown: the Kicks line says not counted only with 2x Bass off") {
    // On every corpus chart, with or without 2x kicks: the lead is the 2x kick
    // line either way; with 2x Bass on there is no tail, and with it off the
    // tail is the fact dot and the not counted status.
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        const TwoPartLine on = dynamics_kicks_heading(c.bd, true);
        const TwoPartLine off = dynamics_kicks_heading(c.bd, false);
        CHECK(on.lead == dynamics_kick2x_line(c.bd));
        CHECK(on.tail.empty());
        CHECK(off.lead == on.lead);
        CHECK(starts_with(off.tail, fact_separator_dot()));
        CHECK(ends_with(off.tail, dynamics_kick2x_status(false)));
    }
}

TEST_CASE("dynamics_breakdown: the 2x kick row is tagged not counted with 2x Bass off") {
    // Pro Drums changes nothing for a kick: the name is the row's own label,
    // the tag shows only with 2x Bass off, and it is the same either way.
    const std::string tag = dynamics_kick2x_row_label(true, false).tail;
    for (bool pro : {true, false}) {
        INFO("pro " << pro);
        const TwoPartLine on = dynamics_kick2x_row_label(pro, true);
        const TwoPartLine off = dynamics_kick2x_row_label(pro, false);
        CHECK(on.lead == dynamics_row_label(DynamicsRow::Kick2x, pro));
        CHECK(off.lead == on.lead);
        CHECK(on.tail.empty());
        CHECK(off.tail == tag);
    }
}

TEST_CASE("dynamics_breakdown: a two-part line with no tail reads as its lead") {
    // On every line the Dynamics tab draws in two parts: no part starts or
    // ends with a space, a line with no tail reads as its lead, and a line
    // with one reads as the lead, one space, then the tail.
    for (const TwoPartLine& line : real_two_part_lines()) {
        const std::string whole = line.whole();
        CAPTURE(whole);
        REQUIRE_FALSE(line.lead.empty());
        CHECK(line.lead.front() != ' ');
        CHECK(line.lead.back() != ' ');
        if (line.tail.empty()) {
            CHECK(whole == line.lead);
            continue;
        }
        CHECK(line.tail.front() != ' ');
        CHECK(line.tail.back() != ' ');
        CHECK(whole.size() == line.lead.size() + 1 + line.tail.size());
        CHECK(starts_with(whole, line.lead + " "));
        CHECK(ends_with(whole, line.tail));
    }
}

TEST_CASE("dynamics_breakdown: the fact dot is the separator's dot alone") {
    // The dot has no space in it, and a space each side of it gives back the
    // separator every screen joins facts with.
    const std::string dot = fact_separator_dot();
    CHECK_FALSE(dot.empty());
    CHECK(dot.find(' ') == std::string::npos);
    CHECK(" " + dot + " " == std::string(kFactSeparator));
}

TEST_CASE("dynamics_breakdown: which kick rows count") {
    // The Kick row always counts; the 2x kick row only with 2x Bass on.
    CHECK(kick_row_counts(DynamicsRow::Kick, true));
    CHECK(kick_row_counts(DynamicsRow::Kick, false));
    CHECK(kick_row_counts(DynamicsRow::Kick2x, true));
    CHECK_FALSE(kick_row_counts(DynamicsRow::Kick2x, false));
}

TEST_CASE("dynamics_breakdown: one share rule for every Dynamics percent") {
    // The 2x kick line and the tab's "Dynamic notes" line both read their
    // percent here, so an empty total reads 0% in both.
    CHECK(dynamics_share(300, 2000) == "15%");
    CHECK(dynamics_share(1, 3) == "33%");
    CHECK(dynamics_share(0, 0) == "0%");
}

TEST_CASE("dynamics_breakdown: flags a row does not have are ignored") {
    // A red note counts as the snare even with a cymbal flag, a kick ignores
    // a cymbal flag, and only a kick can be 2x.
    ChordNote red_cymbal{NoteColor::Red};
    red_cymbal.cymbaltype = NoteCymbalType::Cymbal;
    CHECK(dynamics_row_for(red_cymbal) == DynamicsRow::RedSnare);
    ChordNote kick_cymbal{NoteColor::Kick};
    kick_cymbal.cymbaltype = NoteCymbalType::Cymbal;
    CHECK(dynamics_row_for(kick_cymbal) == DynamicsRow::Kick);
    ChordNote blue_2x{NoteColor::Blue};
    blue_2x.is2x = true;
    CHECK(dynamics_row_for(blue_2x) == DynamicsRow::BlueTom);
}

// ---------------------------------------------------------------------------
// Labels.
// ---------------------------------------------------------------------------

TEST_CASE("dynamics_breakdown: row labels pro vs non-pro") {
    // Every row, with Pro Drums on and off, is named as the app names the
    // row's own note (note_label, the one name of a drum note).
    for (size_t i = 0; i < kRowCount; ++i) {
        const DynamicsRow r = static_cast<DynamicsRow>(i);
        for (const bool pro : {true, false}) {
            CAPTURE(i);
            CAPTURE(pro);
            CHECK(dynamics_row_label(r, pro) == note_label(dynamics_row_note(r), pro));
        }
    }
}

TEST_CASE("dynamics_breakdown: dynamic() counts ghosts and accents and not normals") {
    // On every corpus chart, each row's dynamic notes are the notes it was
    // sent that carry a ghost or accent mark, and a row has dynamics exactly
    // when it has one.
    bool row_with_marks = false;
    bool row_without_marks = false;
    for (const CorpusCount& c : corpus_counts()) {
        CAPTURE(c.path);
        for (size_t i = 0; i < kRowCount; ++i) {
            CAPTURE(i);
            const DynamicsCounts& row = c.bd.rows[i];
            CHECK(row.all() == c.notes[i]);
            CHECK(row.dynamic() == c.marked[i]);
            CHECK(row.has_dynamics() == (c.marked[i] > 0));
            if (c.marked[i] > 0) row_with_marks = true;
            if (c.notes[i] > 0 && c.marked[i] == 0) row_without_marks = true;
        }
    }
    // Acid Romance's red snare has ghosts and accents and its blue tom has
    // notes with none, so both cases are on a real chart.
    CHECK(row_with_marks);
    CHECK(row_without_marks);
}

// The click reuses its analysis's Song for the Dynamics count
// (analysis_parse_counts_dynamics) even though the analysis parses under
// hydra_rules.ini and the count's own parse (load_dynamics_song) under the
// default rules. That is sound only if the rules never change the count.
// Every corpus chart is parsed under the defaults and under a rules set with
// every field changed, walked from rules_fields() so a new field is changed
// too. The fills must move on some chart (else the test proves nothing), and
// the whole stored count, every field of it, must be the same on every chart.
TEST_CASE("dynamics_breakdown: the rules never change a chart's Dynamics count") {
    // Each number moves up by one, which keeps it at or above its smallest
    // allowed value, and the squeeze rule flips to the other one.
    core::Rules other = core::default_rules();
    for (const core::RulesField& field : core::rules_fields()) {
        CAPTURE(std::string(field.name));
        if (const auto* d = std::get_if<double core::Rules::*>(&field.member)) {
            other.*(*d) += 1.0;
            CHECK(other.*(*d) != core::default_rules().*(*d));
        } else if (const auto* n = std::get_if<int core::Rules::*>(&field.member)) {
            other.*(*n) += 1;
            CHECK(other.*(*n) != core::default_rules().*(*n));
        } else if (const auto* s = std::get_if<core::SqOutRule core::Rules::*>(&field.member)) {
            other.*(*s) = other.*(*s) == core::SqOutRule::FirstNote ? core::SqOutRule::WholeChord
                                                                    : core::SqOutRule::FirstNote;
            CHECK(other.*(*s) != core::default_rules().*(*s));
        } else {
            FAIL("a Rules field of a kind this test does not change");
        }
    }

    const Difficulty difficulties[] = {Difficulty::Expert, Difficulty::Hard};
    int fills_moved = 0;
    int counted = 0;
    for (const std::string& path : corpus::chart_paths()) {
        for (const bool pro : {true, false}) {
            for (const Difficulty difficulty : difficulties) {
                CAPTURE(path);
                CAPTURE(pro);
                CAPTURE(static_cast<int>(difficulty));
                const ChartReading reading{difficulty, pro, kDynamicsParseBass2x, false};
                const Song base = load_songpath(path, reading, core::default_rules());
                const Song moved = load_songpath(path, reading, other);
                REQUIRE(base.sequence.size() == moved.sequence.size());
                for (size_t i = 0; i < base.sequence.size(); ++i)
                    if (base.sequence[i].activation_length != moved.sequence[i].activation_length) {
                        ++fills_moved;
                        break;
                    }
                CHECK(count_dynamics(base) == count_dynamics(moved));
                ++counted;
            }
        }
    }
    CHECK(counted > 0);
    CHECK(fills_moved > 0);
    MESSAGE("charts parsed: " << counted << ", fills moved on: " << fills_moved);
}
