// Smoke test for parse/song: every chart in the corpus parses without
// throwing, and the resulting timestamp sequence holds the structural
// invariants the search depends on (monotone ticks, decodable chords,
// consistent activation-fill placement). Covers .mid, .chart, and .sng.

#include "doctest.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "core/model.h"
#include "core/strutil.h"
#include "chart_text.h"
#include "corpus_util.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "midi_util.h"
#include "multidiff_chart.h"
#include "parse/chart_files.h"
#include "parse/song.h"
#include "song_digest.h"

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// Whether check_invariants checks that every activation fill is positive. A
// chart with a fill starting on its own last note has a 0-length activation,
// so its case skips that one check.
enum class FillCheck { Check, Skip };

// The structural invariants the search depends on, checked on one parsed song.
// Each failed invariant is one failed CHECK naming `what`.
void check_invariants(const Song& song, const std::string& what,
                      FillCheck fill_check = FillCheck::Check) {
    bool ticks_ok = true, codes_ok = true, fills_ok = true;
    int64_t prev = -1;
    for (const SongTimestamp& ts : song.sequence) {
        // Strictly increasing: two chords never share a tick.
        if (ts.timecode.ticks() <= prev) ticks_ok = false;
        prev = ts.timecode.ticks();

        // Every chord spells as a code (Chord::code throws on one it can't).
        if (ts.chord.code().empty()) codes_ok = false;

        // An activation fill length is always positive.
        if (ts.activation_length.has_value() && *ts.activation_length <= 0)
            fills_ok = false;
    }
    CHECK_MESSAGE(ticks_ok, what << ": ticks not strictly increasing");
    CHECK_MESSAGE(codes_ok, what << ": a chord with no code");
    if (fill_check == FillCheck::Check)
        CHECK_MESSAGE(fills_ok, what << ": non-positive activation fill");
}

}  // namespace

TEST_CASE("song parse holds its invariants over the corpus") {
    int charts = 0, nonempty = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        ++charts;
        if (song.is_empty()) continue;
        ++nonempty;
        check_invariants(song, path);
    }

    CHECK(nonempty > 0);
    MESSAGE("checked " << charts << " charts (" << nonempty << " non-empty)");
}

TEST_CASE("a parsed song keeps no spare room in its chord list") {
    // Every loader ends at load_songbytes_mid or load_songbytes_chart, so the
    // corpus covers .mid, .chart and the containers alike. Parsed fresh here,
    // not through corpus::song's cache.
    int nonempty = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song song = load_songpath(path, true, true);
        if (song.is_empty()) continue;
        ++nonempty;
        CHECK_MESSAGE(song.sequence.capacity() == song.sequence.size(), path);
    }
    CHECK(nonempty > 0);
}

TEST_CASE("song parse holds its invariants at Hard too") {
    // Same sweep at Hard. Not every chart has Hard charting (an empty song is
    // the honest answer there), but plenty do, and whatever parses must hold
    // exactly the same structure Expert does.
    int charts = 0, nonempty = 0, mids = 0, mids_nonempty = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true, Difficulty::Hard);
        ++charts;
        const bool is_mid = chart_format_of(path) == ChartFormat::Mid;
        if (is_mid) ++mids;
        if (song.is_empty()) continue;
        ++nonempty;
        if (is_mid) ++mids_nonempty;
        check_invariants(song, path + " [Hard]");
    }

    // The corpus has 18 .chart files with a [HardDrums] section, so this is a
    // real sweep and not a silently-empty one.
    CHECK(nonempty > 0);
    // .mid charts keep every difficulty in the one "PART DRUMS" track, so a
    // corpus .mid with Hard notes proves the pitch base works end to end.
    CHECK(mids_nonempty > 0);
    MESSAGE("checked " << charts << " charts at Hard (" << nonempty << " non-empty, "
                       << mids_nonempty << "/" << mids << " .mid)");
}

TEST_CASE(".chart: each difficulty reads its own section") {
    const std::vector<uint8_t> data = multidiff::chart_bytes();

    Song expert = load_songbytes_chart(data, true, true);
    Song hard = load_songbytes_chart(data, true, true, Difficulty::Hard);
    Song medium = load_songbytes_chart(data, true, true, Difficulty::Medium);
    Song easy = load_songbytes_chart(data, true, true, Difficulty::Easy);

    CHECK(expert.sequence.size() == multidiff::kExpertChords);
    CHECK(hard.sequence.size() == multidiff::kHardChords);
    CHECK(easy.sequence.size() == multidiff::kEasyChords);
    // No [MediumDrums] section at all: a missing difficulty parses empty
    // rather than throwing or falling back to another difficulty.
    CHECK(medium.is_empty());
    CHECK(medium.sequence.size() == multidiff::kMediumChords);

    // The sections really are different charting, not the same notes read
    // four times.
    REQUIRE(!expert.sequence.empty());
    REQUIRE(!hard.sequence.empty());
    REQUIRE(!easy.sequence.empty());
    CHECK(expert.sequence[0].chord.code() != hard.sequence[0].chord.code());
    CHECK(easy.sequence[0].chord.code() != hard.sequence[0].chord.code());

    // Everything inside the section follows it: the ghost, the pro cymbal, the
    // SP phrase and the activation fill are all in [HardDrums] alone.
    CHECK(hard.sequence[0].flag_sp);
    bool any_activation = false;
    for (const SongTimestamp& ts : hard.sequence)
        if (ts.has_activation()) any_activation = true;
    CHECK(any_activation);
    // The fill was charted, so no fills had to be synthesized.
    CHECK(hard.features.empty());
    // Pro cymbals are still a pro-drums-only reading at Hard.
    Song hard_nonpro = load_songbytes_chart(data, false, true, Difficulty::Hard);
    CHECK(hard_nonpro.sequence[1].chord.code() != hard.sequence[1].chord.code());

    check_invariants(hard, "multidiff [Hard]");
    check_invariants(easy, "multidiff [Easy]");
}

TEST_CASE(".mid: each difficulty reads its own pitch base") {
    // Corpus .mid files that carry Hard notes parse to a different sequence
    // than their Expert one — the pitch base is what selects the difficulty.
    int compared = 0, differed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        if (chart_format_of(path) != ChartFormat::Mid) continue;
        Song expert = load_songpath_mid(path, true, true);
        Song hard = load_songpath_mid(path, true, true, Difficulty::Hard);
        if (hard.is_empty()) continue;
        ++compared;
        if (hard.sequence.size() != expert.sequence.size()) ++differed;
        check_invariants(hard, path + " [Hard]");
        // Medium and Easy sit at their own bases and must not throw either.
        check_invariants(load_songpath_mid(path, true, true, Difficulty::Medium),
                         path + " [Medium]");
        check_invariants(load_songpath_mid(path, true, true, Difficulty::Easy),
                         path + " [Easy]");
        if (compared >= 5) break;  // a handful is enough; the sweep covers the rest
    }
    CHECK(compared > 0);
    // Hard is a reduction of Expert, so the two streams are not the same one
    // read twice. (Per-chart the counts could coincide; across the sample they
    // cannot all coincide.)
    CHECK(differed > 0);
}

TEST_CASE(".chart: [Events] section markers become practice sections") {
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("Events",
                           "  1536 = E \"prc_chorus_1a\"\n"
                           "  0 = E \"section Intro\"\n"
                           "  768 = E \"section Verse 1\"\n"
                           "  1920 = E \"lighting (blackout)\"\n") +
        testchart::section("ExpertDrums", "  0 = N 0 0\n  192 = N 1 0\n"));
    Song song = load_songbytes_chart(data, true, true);

    // Both spellings are read, non-section text events are not, and the file's
    // own order does not have to be sorted.
    REQUIRE(song.practice_sections.size() == 3);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 768);
    CHECK(song.practice_sections[1].name == "Verse 1");
    CHECK(song.practice_sections[2].tick == 1536);
    CHECK(song.practice_sections[2].name == "chorus_1a");

    // A chart with no [Events] section has no sections and still parses.
    Song plain = load_songbytes_chart(multidiff::chart_bytes(), true, true);
    CHECK(plain.practice_sections.empty());
}

namespace {

// The .chart fixture with one solo whose `E soloend` sits on its third note.
std::vector<uint8_t> solo_end_chart() {
    return testchart::chart_bytes(
        testchart::section("ExpertDrums",
                           "  0 = E solo\n  0 = N 1 0\n"
                           "  192 = N 2 0\n"
                           "  384 = N 3 0\n  384 = E soloend\n"
                           "  576 = N 4 0\n"));
}

// A .mid with one solo marker held from tick 0 to tick 960 (480 ticks a beat,
// midi_util's division) over three notes: Red at 0, Yellow at 480 and Blue on
// the marker's note-off tick.
std::vector<uint8_t> solo_marker_mid() {
    using namespace testmidi;
    return smf(concat({
        track_name("PART DRUMS"), set_tempo(),
        note_on(103, 100), note_on(97, 100),   // tick 0: solo on, Red
        after(480, note_on(98, 100)),          // tick 480: Yellow
        after(480, note_on(103, 0)),           // tick 960: solo marker off
        note_on(99, 100),                      // tick 960: Blue
        end_of_track(),
    }));
}

}  // namespace

// In a .chart the `E soloend` event sits on the solo's last note, so that note
// is in the solo. The parser runs solo end after the notes at its tick.
TEST_CASE(".chart: the note on the solo end tick is in the solo") {
    Song song = load_songbytes_chart(solo_end_chart(), true, true);
    REQUIRE(song.sequence.size() == 4);
    CHECK(song.sequence[2].flag_solo);
    CHECK_FALSE(song.sequence[3].flag_solo);
}

// The parser keeps each time signature as the chart wrote it, beside the
// measure length the engine reads. The length alone cannot tell 6/8 from 3/4.
TEST_CASE(".chart: time signatures are kept as written") {
    const std::vector<uint8_t> data = testchart::chart_bytes(
        testchart::section("ExpertDrums", "  0 = N 0 0\n  1536 = N 1 0\n"), 192, "",
        testchart::kSync44At120 + "  768 = TS 6 3\n  1344 = TS 3\n");
    Song song = load_songbytes_chart(data, true, true);

    // 6/8 and 3/4 are both 576 ticks at 192 per quarter note.
    CHECK(song.tpm_changes.at(768) == 576);
    CHECK(song.tpm_changes.at(1344) == 576);
    CHECK(song.timesig_changes.at(0) == std::make_pair(4, 4));
    CHECK(song.timesig_changes.at(768) == std::make_pair(6, 8));
    CHECK(song.timesig_changes.at(1344) == std::make_pair(3, 4));

    // A chart with no signature at all reads the default, 4/4 from tick 0.
    Song blank(480);
    CHECK(blank.timesig_changes.size() == 1);
    CHECK(blank.timesig_changes.at(0) == std::make_pair(4, 4));
}

// In a .mid the solo is a held marker note (103); its note-off tick is where
// the marker stops covering, so a note on that tick is outside the solo.
TEST_CASE(".mid: the note on the solo marker's note-off tick is outside the solo") {
    const Song song = load_songbytes_mid(solo_marker_mid(), true, true);
    REQUIRE(song.sequence.size() == 3);
    CHECK(song.sequence[0].flag_solo);
    CHECK(song.sequence[1].flag_solo);
    CHECK_FALSE(song.sequence[2].flag_solo);
}

TEST_CASE("song: solo_sections lists each run of solo chords once") {
    // The two fixtures above: .chart chords 0 to 2 are in the solo, .mid
    // chords 0 and 1.
    const Song chart = load_songbytes_chart(solo_end_chart(), true, true);
    REQUIRE(chart.solo_sections.size() == 1);
    CHECK(chart.solo_sections[0].first == 0);
    CHECK(chart.solo_sections[0].last == 2);

    const Song mid = load_songbytes_mid(solo_marker_mid(), true, true);
    REQUIRE(mid.solo_sections.size() == 1);
    CHECK(mid.solo_sections[0].first == 0);
    CHECK(mid.solo_sections[0].last == 1);

    // Two solos split by one plain chord: chords 0 and 1, then chord 3.
    const Song two =
        load_songbytes_chart(testchart::chart_bytes(testchart::kTwoSolosDrums), true, true);
    REQUIRE(two.sequence.size() == 4);
    REQUIRE(two.solo_sections.size() == 2);
    CHECK(two.solo_sections[0].first == 0);
    CHECK(two.solo_sections[0].last == 1);
    CHECK(two.solo_sections[1].first == 3);
    CHECK(two.solo_sections[1].last == 3);

    // No solo at all: no sections.
    const Song plain = load_songbytes_chart(
        testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 1 0\n  192 = N 2 0\n")),
        true, true);
    REQUIRE(plain.sequence.size() == 2);
    CHECK(plain.solo_sections.empty());
}

TEST_CASE("Song::note_count: the chart's note total under each 2x Bass setting") {
    // Tick 0: a kick, a 2x kick on the same tick, and a red. Tick 192: a lone
    // 2x kick. Tick 384: a yellow. Tick 576: a 2x kick written before a kick
    // on the same tick. The two kicks on one tick merge into one 2x kick
    // (D105), so 2x Bass off removes all three kicks, and on keeps one per
    // tick.
    const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
        "ExpertDrums",
        "  0 = N 0 0\n  0 = N 32 0\n  0 = N 1 0\n"
        "  192 = N 32 0\n"
        "  384 = N 2 0\n"
        "  576 = N 32 0\n  576 = N 0 0\n"));
    CHECK(load_songbytes_chart(data, true, /*bass2x=*/true).note_count() == 5);
    CHECK(load_songbytes_chart(data, true, /*bass2x=*/false).note_count() == 2);
    // A chart with no notes counts none.
    CHECK(Song(192).note_count() == 0);
}

TEST_CASE(".mid: EVENTS text metas become practice sections") {
    using namespace testmidi;
    const std::vector<uint8_t> tempo_track =
        concat({track_name("tempo"), set_tempo(), end_of_track()});
    const std::vector<uint8_t> events_track = concat({
        track_name("EVENTS"),
        text_event("[section Intro]"),
        after(384, text_event("[prc_verse_1]")),
        after(384, text_event("[crowd_realtime]")),
        end_of_track(),
    });
    const std::vector<uint8_t> drums_track = concat({
        track_name("PART DRUMS"),
        note_on(96, 100),  // Expert kick
        note_off(96),
        end_of_track(),
    });

    Song song = load_songbytes_mid(smf_tracks({tempo_track, events_track, drums_track}), true,
                                   true);
    REQUIRE(song.sequence.size() == 1);
    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].tick == 384);
    CHECK(song.practice_sections[1].name == "verse_1");
}

namespace {
std::vector<uint8_t> chart_with(int64_t resolution, const std::string& sync) {
    return testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 0 0\n  768 = N 1 0\n"),
                                  resolution, "", sync);
}
}  // namespace

// Timing that would make a measure or beat last zero, negative or infinite
// time is refused at load with a plain error naming the tick (D6). A time
// signature with a top number of 0 is the one line that is ignored instead.
TEST_CASE(".chart: a timing line that can't measure time is refused") {
    const std::string ok = "  0 = TS 4\n  0 = B 120000\n";
    CHECK_NOTHROW(load_songbytes_chart(chart_with(192, ok), true, true));
    struct Bad {
        const char* sync;
        const char* error;
    };
    for (const Bad& bad :
         {Bad{"  0 = TS 4\n  0 = B 0\n", "the tempo at tick 0 is not above 0 BPM"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = B -60000\n",
              "the tempo at tick 384 is not above 0 BPM"},
          Bad{"  0 = TS -3\n  0 = B 120000\n",
              "the time signature at tick 0 makes a measure -576 ticks long"},
          Bad{"  0 = TS 1 12\n  0 = B 120000\n",
              "the time signature at tick 0 makes a measure 0 ticks long"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 40\n",
              "the time signature at tick 384 has a bottom number that is out of range"},
          Bad{"  0 = TS 4\n  0 = B 120000\n  384 = TS 4 -1\n",
              "the time signature at tick 384 has a bottom number that is out of range"}}) {
        CAPTURE(bad.sync);
        CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with(192, bad.sync), true, true),
                             bad.error, ChartFileError);
    }
    CHECK_THROWS_WITH_AS(load_songbytes_chart(chart_with(0, ok), true, true),
                         "the chart's resolution is 0, and it must be above 0",
                         ChartFileError);
}

TEST_CASE(".chart: a TS 0 timing line is still ignored") {
    Song song = load_songbytes_chart(
        chart_with(192, "  0 = TS 4\n  0 = B 120000\n  768 = TS 0\n"), true, true);
    CHECK(song.tpm_changes.size() == 1);
    CHECK(song.tpm_changes.at(0) == 768);
    // Ignored whatever its bottom number says.
    Song odd = load_songbytes_chart(
        chart_with(192, "  0 = TS 4\n  0 = B 120000\n  768 = TS 0 40\n"), true, true);
    CHECK(odd.tpm_changes.size() == 1);
}

TEST_CASE(".mid: a timing line that can't measure time") {
    auto track = [](std::vector<uint8_t> timing) {
        return testmidi::smf(testmidi::concat({testmidi::track_name("PART DRUMS"),
                                               testmidi::set_tempo(), timing,
                                               testmidi::note_on(96, 100),
                                               testmidi::end_of_track()}));
    };
    // TS 0/4: ignored, the 4/4 default stays (finding 100's crash).
    Song song = load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x00, 0x02, 0x18, 0x08}),
                                   true, true);
    CHECK(song.tpm_changes.at(0) == 480 * 4);
    // A 0 us tempo would be an infinite BPM: refused, and the message says so
    // (D12) instead of calling it "not above 0 BPM".
    CHECK_THROWS_WITH_AS(load_songbytes_mid(track(testmidi::set_tempo(0)), true, true),
                         "the tempo at tick 0 is infinite (0 microseconds per beat)",
                         ChartFileError);
    // A denominator exponent of 40: refused, never shifted.
    CHECK_THROWS_WITH_AS(
        load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x04, 40, 0x18, 0x08}), true, true),
        "the time signature at tick 0 has a bottom number that is out of range",
        ChartFileError);
}

// A division of 0 ticks per beat and a negative .chart resolution cannot place
// any note in time. Both were already refused (through check_timing_maps);
// this pins it.
TEST_CASE("a resolution that is 0 or negative is refused in both formats") {
    std::vector<uint8_t> mid = testmidi::smf(testmidi::concat(
        {testmidi::track_name("PART DRUMS"), testmidi::set_tempo(),
         testmidi::note_on(96, 100), testmidi::end_of_track()}));
    mid[12] = 0x00;  // division = 0
    mid[13] = 0x00;
    CHECK_THROWS_WITH_AS(load_songbytes_mid(mid, true, true),
                         "the chart's resolution is 0, and it must be above 0", ChartFileError);
    CHECK_THROWS_WITH_AS(
        load_songbytes_chart(chart_with(-192, "  0 = TS 4\n  0 = B 120000\n"), true, true),
        "the chart's resolution is -192, and it must be above 0", ChartFileError);
}

TEST_CASE("mid: kick velocity is read as ghost/accent, like a pad's") {
    // Clone Hero prices a velocity-1 kick as a ghost and a velocity-127 kick
    // as an accent, both worth double. Hydra used to hand every kick Normal.
    auto drums_track = [](bool dynamics) {
        std::vector<std::vector<uint8_t>> ev;
        ev.push_back(testmidi::track_name("PART DRUMS"));
        ev.push_back(testmidi::set_tempo());
        if (dynamics)
            ev.push_back(testmidi::text_event("[ENABLE_CHART_DYNAMICS]"));
        ev.push_back(testmidi::note_on(96, 1));     // tick 0:   kick, vel 1
        ev.push_back({0x40, 0x90, 95, 127});        // tick 64:  2x kick, vel 127
        ev.push_back({0x40, 0x90, 97, 1});          // tick 128: Red pad, vel 1
        ev.push_back(testmidi::end_of_track());
        return testmidi::smf(testmidi::concat(ev));
    };

    SUBCASE("with [ENABLE_CHART_DYNAMICS]") {
        Song song = load_songbytes_mid(drums_track(true), true, true);
        REQUIRE(song.sequence.size() == 3);

        const auto& kick = song.sequence[0].chord.at(NoteColor::Kick);
        REQUIRE(kick.has_value());
        CHECK(kick->dynamictype == NoteDynamicType::Ghost);
        CHECK(kick->is2x == false);
        CHECK(kick->str() == "Kick (Ghost)");

        const auto& kick2x = song.sequence[1].chord.at(NoteColor::Kick);
        REQUIRE(kick2x.has_value());
        CHECK(kick2x->dynamictype == NoteDynamicType::Accent);
        CHECK(kick2x->is2x == true);
        CHECK(kick2x->str() == "2x kick (Accent)");

        const auto& red = song.sequence[2].chord.at(NoteColor::Red);
        REQUIRE(red.has_value());
        CHECK(red->dynamictype == NoteDynamicType::Ghost);

        check_invariants(song, "kick dynamics fixture");
    }

    SUBCASE("without the text event, every note is Normal") {
        Song song = load_songbytes_mid(drums_track(false), true, true);
        REQUIRE(song.sequence.size() == 3);
        CHECK(song.sequence[0].chord.at(NoteColor::Kick)->dynamictype ==
              NoteDynamicType::Normal);
        CHECK(song.sequence[1].chord.at(NoteColor::Kick)->dynamictype ==
              NoteDynamicType::Normal);
        CHECK(song.sequence[1].chord.at(NoteColor::Kick)->is2x == true);
        CHECK(song.sequence[2].chord.at(NoteColor::Red)->dynamictype ==
              NoteDynamicType::Normal);
        check_invariants(song, "kick dynamics fixture (no marker)");
    }
}

TEST_CASE("mid: Won't Get Fooled Again (O) has 36 ghost kicks") {
    // The chart that proved the bug: Hydra called it 1,134,335 while a real FC
    // on Hydra's own path scored 1,142,235. 36 velocity-1 kicks, no accents.
    const std::string path =
        std::string(HYDRA_TESTDATA_DIR) + "/midi/wgfa_onyxite/notes.mid";
    Song song = load_songpath(path, true, true, Difficulty::Expert);
    REQUIRE(!song.is_empty());

    int ghost = 0, accent = 0, kicks = 0;
    for (const SongTimestamp& ts : song.sequence) {
        const auto& kick = ts.chord.at(NoteColor::Kick);
        if (!kick.has_value()) continue;
        ++kicks;
        if (kick->dynamictype == NoteDynamicType::Ghost) ++ghost;
        if (kick->dynamictype == NoteDynamicType::Accent) ++accent;
    }
    CHECK(kicks > 36);
    CHECK(ghost == 36);
    CHECK(accent == 0);

    // Every chord code still round-trips, which is what the new ghost-kick
    // codes have to prove. The fill check is skipped: this chart has a fill
    // that starts on its own last note, so its activation length is 0 (true
    // before the ghost-kick change and unrelated to it).
    check_invariants(song, path, FillCheck::Skip);
}

TEST_CASE("mid: the marker pitch table is the one list the parser reads") {
    // The marker pitches typed in the parser's switches before the table.
    for (int pitch : {103, 109, 110, 111, 112, 116, 120})
        CHECK_MESSAGE(is_midi_marker_pitch(pitch), pitch);
    // Expert's 2x kick (95), kick (96) and Green (100), and the pitches on
    // either side of the solo and fill markers.
    for (int pitch : {95, 96, 100, 102, 104, 121})
        CHECK_MESSAGE(!is_midi_marker_pitch(pitch), pitch);
}

TEST_CASE("chart_files: loose-folder notes names match in any case") {
    CHECK(notes_file_format("notes.mid") == ChartFormat::Mid);
    CHECK(notes_file_format("NOTES.MID") == ChartFormat::Mid);
    CHECK(notes_file_format("Notes.Chart") == ChartFormat::Chart);
    CHECK(notes_file_format("notes.sng") == ChartFormat::None);
    CHECK(notes_file_format("mynotes.mid") == ChartFormat::None);
    CHECK(is_song_ini("song.ini"));
    CHECK(is_song_ini("Song.INI"));
    CHECK_FALSE(is_song_ini("song.ini.bak"));
}

TEST_CASE("chart_files: a path's format comes from its extension in any case") {
    CHECK(chart_format_of("C:\\songs\\a\\notes.mid") == ChartFormat::Mid);
    CHECK(chart_format_of("x.CHART") == ChartFormat::Chart);
    CHECK(chart_format_of("C:\\songs\\bundle.SNG") == ChartFormat::Sng);
    CHECK(chart_format_of("pack.Srb") == ChartFormat::Srb);
    CHECK(chart_format_of("notes.txt") == ChartFormat::None);
    CHECK(chart_format_of("mid") == ChartFormat::None);
}

TEST_CASE("mid: a stray SP note-off flags nothing") {
    std::vector<std::vector<uint8_t>> ev;
    ev.push_back(testmidi::track_name("PART DRUMS"));
    ev.push_back(testmidi::set_tempo());
    ev.push_back(testmidi::note_on(116, 100));       // tick 0:   SP phrase starts
    ev.push_back(testmidi::note_on(96, 100));        // tick 0:   kick, inside the phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 240: SP phrase ends
    ev.push_back({0x81, 0x70, 0x90, 96, 100});       // tick 480: kick, outside any phrase
    ev.push_back({0x81, 0x70, 0x80, 116, 0});        // tick 720: stray SP note-off
    ev.push_back(testmidi::end_of_track());
    Song song = load_songbytes_mid(testmidi::smf(testmidi::concat(ev)), true, true);

    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].flag_sp);
    CHECK_FALSE(song.sequence[1].flag_sp);
}

TEST_CASE(".chart: [Song] Offset is read in seconds") {
    std::string text = multidiff::chart_text();
    const std::string at = "  Resolution = 192\n";
    text.insert(text.find(at) + at.size(), "  Offset = 0.25\n");
    const std::vector<uint8_t> data(text.begin(), text.end());

    Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.chart_offset_s.has_value());
    CHECK(*song.chart_offset_s == doctest::Approx(0.25));

    Song plain = load_songbytes_chart(multidiff::chart_bytes(), true, true);
    CHECK_FALSE(plain.chart_offset_s.has_value());
}

// ---- the parsers' hand matchers against the regexes they replaced ----------
//
// The parsers used std::regex for the disco-flip markers and the .chart section
// header, and still match them exactly as those regexes did. All of them match
// by hand now, for speed. The disco and section-header cases keep the old
// regexes as the oracle. The dynamics tag has no regex oracle any more: it is
// Clone Hero's two exact strings (finding 64, D24), so its cases check against
// those two strings. Every case drives the real parsers with strings built
// around every byte value, so any difference in what matches shows up.

namespace {

const std::regex& oracle_disco_on() {
    static const std::regex r(R"(\[?mix.3.drums\d?d\]?)");
    return r;
}
const std::regex& oracle_disco_off() {
    static const std::regex r(R"(\[?mix.3.drums\d?(dnoflip)?\]?)");
    return r;
}

// Disco markers with byte `b` placed in every spot the regexes care about.
std::vector<std::string> disco_candidates(char b) {
    const std::string c(1, b);
    return {"mix" + c + "3_drums0d", "mix_3" + c + "drums0d", "mix_3_drums" + c + "d",
            "mix_3_drums" + c, "mix_3_drums" + c + "dnoflip", "[mix_3_drums0d" + c,
            c + "mix_3_drums0d", "mix_3_drums0" + c, c + "mix_3_drums0d]"};
}

// The MIDI reader stores each text byte as latin-1 decoded to UTF-8.
std::string latin1_to_utf8(const std::string& s) {
    std::string out;
    for (unsigned char b : s) {
        if (b < 0x80) {
            out.push_back(static_cast<char>(b));
        } else {
            out.push_back(static_cast<char>(0xC0 | (b >> 6)));
            out.push_back(static_cast<char>(0x80 | (b & 0x3F)));
        }
    }
    return out;
}

}  // namespace

TEST_CASE(".chart: disco markers match the regexes they replaced") {
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        // A .chart event word never holds whitespace or '=' (both split it).
        if (std::isspace(static_cast<unsigned char>(b)) || b == '=') continue;
        for (const std::string& marker : disco_candidates(b)) {
            for (bool prior_on : {false, true}) {
                const std::vector<uint8_t> data = testchart::chart_bytes(testchart::section(
                    "ExpertDrums", std::string(prior_on ? "  0 = E mix_3_drums0d\n" : "") +
                                       "  0 = N 0 0\n  192 = E " + marker + "\n  192 = N 1 0\n"));
                const Song song = load_songbytes_chart(data, true, true);
                REQUIRE(song.sequence.size() == 2);

                const bool off = std::regex_match(marker, oracle_disco_off());
                const bool on = std::regex_match(marker, oracle_disco_on());
                const bool want_flip = off ? false : on ? true : prior_on;
                // A flipped red pad reads as a yellow cymbal.
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip,
                              "byte " << byte << " marker index, prior " << prior_on);
                ++checked;
            }
        }
    }
    CHECK(checked > 4000);
}

TEST_CASE(".mid: disco markers match the regexes they replaced") {
    using namespace testmidi;
    int checked = 0;
    for (int byte = 0; byte < 256; ++byte) {
        const char b = static_cast<char>(byte);
        for (const std::string& marker : disco_candidates(b)) {
            const std::string as_read = latin1_to_utf8(marker);
            const bool on = std::regex_match(as_read, oracle_disco_on());
            const bool off = std::regex_match(as_read, oracle_disco_off());
            for (bool prior_on : {false, true}) {
                std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
                if (prior_on) ev.push_back(text_event("mix_3_drums0d"));
                ev.push_back(note_on(96, 100));                    // tick 0: kick
                ev.push_back(after(480, text_event(marker)));  // tick 480
                ev.push_back(note_on(97, 100));                    // tick 480: red
                ev.push_back(end_of_track());
                const Song song = load_songbytes_mid(smf(concat(ev)), true, true);
                REQUIRE(song.sequence.size() == 2);
                const bool want_flip = on ? true : off ? false : prior_on;
                const bool flipped = song.sequence[1].chord.at(NoteColor::Yellow).has_value();
                CHECK_MESSAGE(flipped == want_flip, "byte " << byte << ", prior " << prior_on);
                ++checked;
            }
        }
    }
    // The dynamics marker's two exact spellings are pinned as literal lists in
    // "dynamics tag: only Clone Hero's two exact spellings count"
    // (test_s2_dynamics_tag.cpp).
    CHECK(checked > 4000);
}

TEST_CASE(".chart: section headers are found as the regex found them") {
    // The first line outside a section must hold a `[name]`, found as
    // std::regex_search(`\[.*\]`) found it: leftmost '[', greedy to the last
    // ']' reachable without crossing a '\r'. No header throws ChartFileError;
    // a header that is not [Song] leaves the chart without its [Song] section.
    static const std::regex header(R"(\[.*\])");
    const std::vector<std::string> lines = {
        "[Song]", "x[Song]", "[Song]x", "[Song] [x]", "[[Song]", "[Song]]",
        "[So\rng]", "[Song\r]", "[x\r][Song]", "[x\r]]", "]Song[", "[", "Song",
        "[]", "\x01[Song]", "[\r[Song]", "[a\r[b]", "[Song]\r]", "[x]\r[Song]",
        "\xC3\xA9[Song]\xC3\xA9", "[S\xC3\xA9]",
    };
    for (const std::string& raw : lines) {
        const std::string text = raw + "\n{\n  Resolution = 192\n}\n"
                                       "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
        const std::vector<uint8_t> data(text.begin(), text.end());
        const std::string line = trim(raw);
        std::smatch m;
        if (!std::regex_search(line, m, header)) {
            CHECK_THROWS_AS(load_songbytes_chart(data, true, true), ChartFileError);
            continue;
        }
        const std::string bracket = m.str(0);
        if (bracket.substr(1, bracket.size() - 2) == "Song") {
            CHECK_MESSAGE(load_songbytes_chart(data, true, true).tick_resolution() == 192, raw);
        } else {
            CHECK_THROWS_AS(load_songbytes_chart(data, true, true), std::out_of_range);
        }
    }
}

// Malformed .chart lines keep the handling they have always had. Each case
// was checked against the regex-era reader before it was replaced.
TEST_CASE(".chart: malformed lines keep their handling") {
    const std::string song = "[Song]\n{\n  Resolution = 192\n}\n";
    const std::string sync = "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
    auto parse = [](const std::string& text) {
        const std::vector<uint8_t> data(text.begin(), text.end());
        return load_songbytes_chart(data, true, true);
    };

    // A blank line outside a section is not a header: the chart is refused.
    CHECK_THROWS_AS(parse(song + "\n" + sync), ChartFileError);
    // So is trailing whitespace after the last section, or a stray '}'.
    CHECK_THROWS_AS(parse(song + sync + "   "), ChartFileError);
    CHECK_THROWS_AS(parse(song + "}\n" + sync), ChartFileError);

    // A second '=' ends the value: "N 1 0 = junk" reads as "N 1 0".
    {
        Song s = parse(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0 = junk\n  192 = N 2 0=\n}\n");
        REQUIRE(s.sequence.size() == 2);
        CHECK(s.sequence[0].chord.at(NoteColor::Red).has_value());
        CHECK(s.sequence[1].chord.at(NoteColor::Yellow).has_value());
    }

    // Lines with no '=' or no value, and an N with too few words, add nothing.
    {
        Song s = parse(song + sync +
                       "[ExpertDrums]\n{\n  garbage\n  =\n  0 = N 1\n  0 = N 3 0\n}\n");
        REQUIRE(s.sequence.size() == 1);
        CHECK(s.sequence[0].chord.count() == 1);
        CHECK(s.sequence[0].chord.at(NoteColor::Blue).has_value());
    }

    // A note number that is not a number refuses the chart, as std::stoi does.
    CHECK_THROWS_AS(parse(song + sync + "[ExpertDrums]\n{\n  0 = N x 0\n}\n"),
                    std::invalid_argument);

    // A section that never closes is dropped.
    CHECK(parse(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0\n").sequence.empty());

    // A repeated section replaces the earlier one.
    {
        Song s = parse(song + "[Song]\n{\n  Resolution = 480\n}\n" + sync +
                       "[ExpertDrums]\n{\n  0 = N 1 0\n}\n[ExpertDrums]\n{\n  0 = N 2 0\n}\n");
        CHECK(s.tick_resolution() == 480);
        REQUIRE(s.sequence.size() == 1);
        CHECK(s.sequence[0].chord.at(NoteColor::Yellow).has_value());
    }

    // Tabs and CRLF are whitespace; tick keys read like std::stoll ("+192").
    {
        Song s = parse("\t[Song]\v\f\n{\n Resolution\t=\t192 \r\n}\r\n" + sync +
                       "[ExpertDrums]\n{\n  0\t=\tN\t1\t0\n  +192 = N 2 0\n}\n");
        REQUIRE(s.sequence.size() == 2);
        CHECK(s.sequence[1].timecode.ticks() == 192);
    }

    // Generic E events: quotes and outer spaces come off, inner spaces stay.
    {
        Song s = parse(song + sync +
                       "[Events]\n{\n  0 = E   \"section  Intro  \"  \n  96 = E\n"
                       "  48 = E section Verse\n  10 = E \"prc_chorus\"\n}\n"
                       "[ExpertDrums]\n{\n  0 = N 1 0\n}\n");
        REQUIRE(s.practice_sections.size() == 3);
        CHECK(s.practice_sections[0].tick == 0);
        CHECK(s.practice_sections[0].name == " Intro  ");
        CHECK(s.practice_sections[1].tick == 10);
        CHECK(s.practice_sections[1].name == "chorus");
        CHECK(s.practice_sections[2].tick == 48);
        CHECK(s.practice_sections[2].name == "Verse");
    }
}

TEST_CASE("the no-notes error carries no_notes_message's sentence") {
    const NoNotesError err(Difficulty::Hard, true);
    CHECK(std::string(err.what()) == "No Hard Pro Drums notes in this chart.");
    // Callers that catch any chart-file problem still catch this one.
    CHECK_THROWS_AS(throw NoNotesError(Difficulty::Hard, true), ChartFileError);
}

TEST_CASE("load-and-check throws the no-notes error for a missing difficulty") {
    const std::string no_hard = corpus::first_chart_without_notes(Difficulty::Hard);
    const std::string has_hard = corpus::first_chart_with_notes(Difficulty::Hard);
    CAPTURE(no_hard);
    CAPTURE(has_hard);

    CHECK_THROWS_WITH_AS(load_songpath_with_notes(no_hard, true, true, Difficulty::Hard),
                         "No Hard Pro Drums notes in this chart.", NoNotesError);
    CHECK_NOTHROW(load_songpath_with_notes(has_hard, true, true, Difficulty::Hard));
}

// ---- display_title: the one cleaned song title (findings 8 and 111) ----------

TEST_CASE("display_title: a title made only of Clone Hero tags reads (unknown)") {
    CHECK(display_title(test::kTagOnlyTitle) == "(unknown)");
    CHECK(display_title("   ") == "(unknown)");
}

TEST_CASE("display_title: tags go, words stay, spaces are trimmed") {
    CHECK(display_title("<b>Bold</b>") == "Bold");
    CHECK(display_title("<color=#e02222>Blood</color>line") == "Bloodline");
    CHECK(display_title(" Some Song ") == "Some Song");
}

TEST_CASE("display_title: a clean name and the old placeholder behave like title_or_unknown") {
    CHECK(display_title("Some Song") == "Some Song");
    CHECK(display_title("<unknown title>") == "(unknown)");
}

TEST_CASE("display_title: an artist reads by the same rule (D50 item 5)") {
    // An artist made only of tags reads "(unknown)", like a title.
    CHECK(display_artist(test::kTagOnlyTitle) == "(unknown)");
    CHECK(display_artist(" <i>Tagged</i> Artist ") == "Tagged Artist");
}

TEST_CASE("display_artist: a missing artist reads (unknown) however it is stored (D56 item 2)") {
    // Empty, the scan's placeholder, or only tags: all three read "(unknown)".
    CHECK(display_artist("") == "(unknown)");
    CHECK(display_artist("<unknown artist>") == "(unknown)");
    CHECK(display_artist(" <b></b> ") == "(unknown)");
    // The stored text is not touched: the scan still writes its placeholder.
    CHECK(artist_or_unknown("") == "<unknown artist>");
}

TEST_CASE("display_charter: tags go and the ends are trimmed, with no fallback") {
    CHECK(display_charter(" <b>Bob</b> ") == "Bob");
    CHECK(display_charter("<color=red> Hoph2o </color>") == "Hoph2o");
    CHECK(display_charter(test::kTagOnlyTitle) == "");
    CHECK(display_charter("") == "");
    // The scan's charter placeholder keeps today's text.
    CHECK(display_charter("<unknown charter>") == "<unknown charter>");
}

// ---- the crafted edge files (testdata/parse_edge, speedups task P1) ----
//
// gen_edge.py writes 41 small charts, each one odd or broken in its own way,
// and says what each one tests. The two expected files hold what the readers
// made of them before P1 rewrote the readers, captured with the old readers'
// hydra_bench --parse (one line per file: its name relative to the folder,
// tests/song_digest.h's hash in hex, and for a file that failed "FAIL " and
// its exception's type and message). This test reads every file again and
// must get the same hash and the same failure text.

namespace {

std::string edge_dir() { return std::string(HYDRA_TESTDATA_DIR) + "/parse_edge/"; }

// One expected line, split at its tabs. The middle field hydra_bench writes
// (the parse time) was taken out when the file was captured.
struct EdgeExpect {
    std::string file;
    uint64_t hash = 0;
    std::string fail;  // empty when the file parsed
};

std::vector<EdgeExpect> read_edge_expect(const std::string& name) {
    std::vector<EdgeExpect> out;
    std::ifstream in(edge_dir() + name, std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        const size_t t1 = line.find('\t');
        const size_t t2 = line.find('\t', t1 + 1);
        EdgeExpect e;
        e.file = line.substr(0, t1);
        e.hash = std::stoull(line.substr(t1 + 1, t2 - t1 - 1), nullptr, 16);
        if (t2 != std::string::npos) e.fail = line.substr(t2 + 1);
        out.push_back(std::move(e));
    }
    return out;
}

// The settings an expected file was captured under.
app::BatchRun edge_run(const char* difficulty, bool pro, bool bass2x) {
    return digest::digest_settings(difficulty, pro, bass2x).batch_run();
}

void check_edge_files(const std::string& expected_name, const app::BatchRun& run) {
    const std::vector<EdgeExpect> expect = read_edge_expect(expected_name);
    // Every file the generator lists has its line, in the list's order.
    std::vector<std::string> listed;
    {
        std::ifstream list(edge_dir() + "list.txt", std::ios::binary);
        std::string name;
        while (std::getline(list, name)) {
            if (!name.empty() && name.back() == '\r') name.pop_back();
            if (!name.empty()) listed.push_back(name);
        }
    }
    REQUIRE(listed.size() == expect.size());

    for (size_t i = 0; i < expect.size(); ++i) {
        const EdgeExpect& e = expect[i];
        CHECK_MESSAGE(e.file == listed[i], "line " << i + 1 << " of " << expected_name);
        std::string fail;
        const uint64_t got = digest::chart_parse_hash(edge_dir() + e.file, run.settings, &fail);
        if (!fail.empty()) fail = "FAIL " + fail;
        CHECK_MESSAGE(fail == e.fail, e.file << " (" << expected_name << ") failed differently");
        CHECK_MESSAGE(got == e.hash, e.file << " (" << expected_name << ") parses differently");
    }
}

}  // namespace

TEST_CASE("the crafted edge files parse as the old readers did") {
    SUBCASE("Expert, Pro Drums, 2x Bass") {
        check_edge_files("expected_expert.tsv", edge_run("Expert", true, true));
    }
    SUBCASE("Hard, Pro Drums off, 2x Bass off") {
        check_edge_files("expected_hard.tsv", edge_run("Hard", false, false));
    }
}
