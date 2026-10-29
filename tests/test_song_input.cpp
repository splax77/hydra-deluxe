// What the parsers make of odd and malformed input, and the cleaned display
// text for a song's title, artist and charter. The corpus sweeps, the
// per-difficulty reads and the regex-replacement pins are in
// test_song_parse.cpp.

#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/preview_source.h"  // read_ini_delay_ms
#include "app/preview_view.h"    // build_time_box
#include "core/model.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "chart_text.h"
#include "display_fixtures.h"  // kTagOnlyTitle
#include "load_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "song_equal.h"
#include "temp_util.h"

using namespace hydra;

// ---- parser cases moved in from the deleted grab bag (wave B, task 2.2) ----

// Finding R7.7; D27; D253. The time box checks stay with the parse: the
// Preview reads the parser's order, and a copy of this fixture in
// test_preview_boxes.cpp would be a clone the single-owner scan refuses.
TEST_CASE(".mid: practice sections come from the first EVENTS track only, and the time box "
          "names the one in force") {
    using namespace testmidi;
    const std::vector<uint8_t> tempo = concat({set_tempo(), end_of_track()});
    const std::vector<uint8_t> drums =
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()});
    // First EVENTS track: Intro at 0, Chorus at 1920.
    const std::vector<uint8_t> events1 =
        concat({track_name("EVENTS"), text_event("[section Intro]"),
                after(1920, text_event("[section Chorus]")), end_of_track()});
    // Second EVENTS track: Verse at 960. Clone Hero reads only the first
    // events track (MidiFile::events_track), so Verse is not a section.
    const std::vector<uint8_t> events2 =
        concat({track_name("EVENTS"), after(960, text_event("[section Verse]")),
                end_of_track()});
    const Song song =
        load_songbytes_mid(smf_tracks({tempo, drums, events1, events2}), ChartReading{});

    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].name == "Chorus");
    CHECK(song.practice_sections[1].tick == 1920);

    // The time box names the section the playhead is in. At 120 BPM and 480
    // ticks a beat, tick 1000 is 1041.67 ms (still Intro) and tick 2000 is
    // 2083.33 ms (Chorus).
    const app::PreviewScene scene = app::build_preview_scene(song, nullptr);
    CHECK(app::build_time_box(scene, 1041.7, 3000.0).section_line == "Section Intro");
    CHECK(app::build_time_box(scene, 2083.4, 3000.0).section_line == "Section Chorus");
}

// D252: Clone Hero plays a drum track named "PART DRUM" (is_drums_track_name
// in parse/midi.h), so Hydra reads it instead of failing with no notes.
TEST_CASE(".mid: a drum track named PART DRUM reads the same Expert notes as PART DRUMS") {
    using namespace testmidi;
    const auto chart = [](const std::string& name) {
        return smf_drums(concat({track_name(name), note_on(96, 100), note_on(97, 100),
                                 after(480, note_on(98, 100)), end_of_track()}));
    };
    const Song drum = load_songbytes_mid(chart("PART DRUM"), ChartReading{});
    CHECK_NOTHROW(require_notes(drum, Difficulty::Expert, true));
    const Song drums = load_songbytes_mid(chart("PART DRUMS"), ChartReading{});
    CHECK(drums.sequence.size() == 2);
    CHECK(testsong::songs_equal(drum, drums));
}

// Finding 331; D27.
TEST_CASE("Chord: a cymbal, ghost or accent on a missing note is a chart-file error") {
    // hydra:: because windows.h declares a GDI function named Chord.
    hydra::Chord c;
    c.add_note(NoteColor::Red);
    CHECK_THROWS_AS(c.apply_cymbal(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_ghost(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_accent(NoteColor::Blue), ChartFileError);
    c.apply_ghost(NoteColor::Red);
    CHECK(c.at(NoteColor::Red)->is_ghost());
}

// Finding 331; D27.
TEST_CASE(".chart: a modifier with no note under it is skipped") {
    // Tick 0: red alone, plus a yellow cymbal, a yellow ghost and a yellow
    // accent marker with no yellow note to change. Tick 384: a cymbal marker
    // with no chord at all. Tick 768: a yellow note whose cymbal marker is real.
    const std::string drums =
        "  0 = N 1 0\n  0 = N 66 0\n  0 = N 41 0\n  0 = N 35 0\n"
        "  384 = N 66 0\n"
        "  768 = N 2 0\n  768 = N 66 0\n";
    const std::vector<uint8_t> data =
        testchart::chart_bytes(testchart::section("ExpertDrums", drums));
    REQUIRE_NOTHROW(load_songbytes_chart(data, ChartReading{}));
    const Song song = load_songbytes_chart(data, ChartReading{});
    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].chord.code() == ".n...");
    CHECK(song.sequence[1].timecode.ticks() == 768);
    CHECK(song.sequence[1].chord.code() == "..N..");
}

// Finding R7.5; D27.
TEST_CASE("parse_finite_number: reads only a whole finite number") {
    CHECK(parse_finite_number("0.25") == 0.25);
    CHECK(parse_finite_number(" -250 ") == -250.0);
    CHECK(parse_finite_number("+500") == 500.0);
    CHECK(parse_finite_number("1e3") == 1000.0);
    for (const char* bad : {"", "  ", "500ms", "0.25s", "nan", "NaN", "inf", "-inf",
                            "1e999", "soon", "+-5", "5 5", "0x1F4"}) {
        CAPTURE(bad);
        CHECK_FALSE(parse_finite_number(bad).has_value());
    }
}

// Finding R7.5; D27.
TEST_CASE(".chart: an Offset that is not a plain number is absent") {
    auto offset = [](const std::string& text) {
        return load_songbytes_chart(
                   testchart::chart_bytes(testchart::section("ExpertDrums", "  0 = N 1 0\n"), 192,
                                          "  Offset = " + text + "\n"),
                   ChartReading{})
            .chart_offset_s;
    };
    CHECK(offset("0.25") == 0.25);
    CHECK(offset("1") == 1.0);
    CHECK(offset("-0.5") == -0.5);
    CHECK_FALSE(offset("500ms").has_value());  // 500 seconds before this change
    CHECK_FALSE(offset("0.25s").has_value());
    CHECK_FALSE(offset("nan").has_value());
}

// Finding R7.5; D27.
TEST_CASE("song.ini: a delay that is not a plain number is absent") {
    namespace fs = std::filesystem;
    const fs::path dir = hydra::os_path(testtemp::temp_dir("song_ini_delay"));
    const fs::path ini = dir / "song.ini";
    auto delay = [&](const std::string& value) {
        {
            std::ofstream f(ini, std::ios::binary | std::ios::trunc);
            f << "[song]\ndelay = " << value << "\n";
        }
        return app::read_ini_delay_ms(ini.u8string());
    };
    CHECK(delay("1016") == 1016.0);
    CHECK_FALSE(delay("nan").has_value());  // NaN before, and it beat the Offset
    CHECK_FALSE(delay("inf").has_value());
    CHECK_FALSE(delay("250ms").has_value());
    fs::remove_all(dir);
}

// Malformed .chart lines keep the handling they have always had. Each case
// was checked against the regex-era reader before it was replaced.
TEST_CASE(".chart: malformed lines keep their handling") {
    const std::string song = "[Song]\n{\n  Resolution = 192\n}\n";
    const std::string sync = "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
    auto parse = [](const std::string& text) {
        const std::vector<uint8_t> data(text.begin(), text.end());
        return load_songbytes_chart(data, ChartReading{});
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
        // chorus is in Clone Hero's list as "Chorus"
        // (data/clone-hero-1.1.0.6142-section-names.tsv, line 423).
        CHECK(s.practice_sections[1].name == "Chorus");
        CHECK(s.practice_sections[2].tick == 48);
        CHECK(s.practice_sections[2].name == "Verse");
    }
}

namespace {

// What a load refuses with: its kind and its text. Empty text means it loaded.
struct Refusal {
    ErrorKind kind = ErrorKind::ChartUnreadable;
    std::string what;
};

template <typename Load>
Refusal refusal_of(Load load) {
    try {
        load();
    } catch (const ChartFileError& e) {
        return Refusal{e.kind(), e.what()};
    }
    return Refusal{};
}

Refusal chart_refusal(const std::string& text) {
    return refusal_of([&] {
        load_songbytes_chart(std::vector<uint8_t>(text.begin(), text.end()), ChartReading{});
    });
}

}  // namespace

// A chart whose first tempo comes after tick 0 (or that has none) is refused
// with a timing refusal, the same way as a tempo of 0 BPM. Hydra does not
// guess a tempo for it (the user's ruling, PLAINERR-RES).
TEST_CASE("a chart with no tempo at tick 0 is a timing refusal in both formats") {
    const std::string drums =
        testchart::section("ExpertDrums", "  0 = N 0 0\n  384 = N 1 0\n  576 = N 2 0\n");
    for (const std::string& sync : {std::string("  0 = TS 4\n  384 = B 60000\n"),
                                    std::string("  0 = TS 4\n"), std::string()}) {
        CAPTURE(sync);
        const Refusal r = chart_refusal(testchart::chart_text(drums, 192, "", sync));
        CHECK(r.kind == ErrorKind::ChartTimingRefused);
        CHECK(r.what == "it has no tempo at tick 0");
    }
    // No [SyncTrack] section at all.
    const Refusal none =
        chart_refusal(testchart::section("Song", "  Resolution = 192\n") + drums);
    CHECK(none.kind == ErrorKind::ChartTimingRefused);
    CHECK(none.what == "it has no tempo at tick 0");

    // A .mid with no set-tempo event.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf_drums(
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        std::vector<uint8_t>{});
    const Refusal m = refusal_of([&] { load_songbytes_mid(mid, ChartReading{}); });
    CHECK(m.kind == ErrorKind::ChartTimingRefused);
    CHECK(m.what == "it has no tempo at tick 0");
}

// Resolution is read once, by one number rule (ChartParser::parse). Text it
// can't read refuses the chart as a timing refusal with a plain reason, not
// the standard library's words.
TEST_CASE(".chart: a Resolution that can't be read is a timing refusal") {
    const std::string sync = testchart::section("SyncTrack", "  0 = TS 4\n  0 = B 120000\n");
    const std::string drums = testchart::section("ExpertDrums", "  0 = N 1 0\n");
    auto with_song = [&](const std::string& song_lines) {
        return chart_refusal(testchart::section("Song", song_lines) + sync + drums);
    };
    struct Bad {
        std::string text;
        const char* what;
    };
    for (const Bad& bad : {Bad{sync + drums, "the chart's resolution is missing"},
                           Bad{testchart::section("Song", "  Name = x\n") + sync + drums,
                               "the chart's resolution is missing"}}) {
        CAPTURE(bad.text);
        const Refusal r = chart_refusal(bad.text);
        CHECK(r.kind == ErrorKind::ChartTimingRefused);
        CHECK(r.what == bad.what);
    }
    for (const Bad& bad : {Bad{"  Resolution = abc\n", "the chart's resolution is not a whole number"},
                           Bad{"  Resolution = x192\n", "the chart's resolution is not a whole number"},
                           Bad{"  Resolution = 99999999999999999999\n",
                               "the chart's resolution is too large"}}) {
        CAPTURE(bad.text);
        const Refusal r = with_song(bad.text);
        CHECK(r.kind == ErrorKind::ChartTimingRefused);
        CHECK(r.what == bad.what);
    }
    // Valid text keeps its number rule: leading digits read, trailing junk ignored.
    const std::string ok = testchart::section("Song", "  Resolution = 480abc\n") + sync + drums;
    CHECK(load_songbytes_chart(std::vector<uint8_t>(ok.begin(), ok.end()), ChartReading{})
              .tick_resolution() == 480);
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

TEST_CASE("display_charter: tags go and the ends are trimmed") {
    CHECK(display_charter(" <b>Bob</b> ") == "Bob");
    CHECK(display_charter("<color=red> Hoph2o </color>") == "Hoph2o");
}

TEST_CASE("display_charter: a missing charter reads (unknown) however it is stored (D266 item 2)") {
    // Empty, the scan's placeholder, or only tags: all three read "(unknown)",
    // the same text a missing artist shows.
    CHECK(display_charter("") == "(unknown)");
    CHECK(display_charter("<unknown charter>") == "(unknown)");
    CHECK(display_charter(test::kTagOnlyTitle) == "(unknown)");
    CHECK(display_charter(" <b></b> ") == "(unknown)");
    // The stored text is not touched: the scan still writes its placeholder.
    CHECK(charter_or_unknown("") == "<unknown charter>");
}

TEST_CASE("display_album: tags go and the ends are trimmed, and a blank stays blank") {
    CHECK(display_album(" <b>Dookie</b> ") == "Dookie");
    CHECK(display_album("<color=red> Chair </color>") == "Chair");
    CHECK(display_album(test::kTagOnlyTitle) == "");
    CHECK(display_album("") == "");
    CHECK(display_album("   ") == "");
    // The artist's placeholder means nothing to an album.
    CHECK(display_album("<unknown artist>") == "<unknown artist>");
}
