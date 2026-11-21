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
#include "parse/section_names.h"  // section_display_name
#include "parse/song.h"
#include "song_equal.h"
#include "temp_util.h"

using namespace hydra;

// ---- parser cases -----------------------------------------------------------

namespace {

// The Preview time box's section line with the playhead on `tick`: the tick's
// time on the song's own tempo map. The length is that same time, so the box
// shows the playhead unclamped.
std::string section_line_at(const Song& song, const app::PreviewScene& scene, int64_t tick) {
    const double ms = song.timing().timecode(tick).ms();
    return app::build_time_box(scene, ms, ms).section_line;
}

}  // namespace

// Finding R7.7; D27; D253. The time box checks stay with the parse: the
// Preview reads the parser's order, and a copy of this fixture in
// test_preview_boxes.cpp would be a clone the single-owner scan refuses.
TEST_CASE(".mid: practice sections come from the first EVENTS track only, and the time box "
          "names the one in force") {
    using namespace testmidi;
    constexpr uint32_t kVerseTick = 960;    // the second EVENTS track's section
    constexpr uint32_t kChorusTick = 1920;  // the first EVENTS track's second section
    const std::vector<uint8_t> tempo = concat({set_tempo(), end_of_track()});
    const std::vector<uint8_t> drums =
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()});
    const std::vector<uint8_t> events1 =
        concat({track_name("EVENTS"), text_event("[section Intro]"),
                after(kChorusTick, text_event("[section Chorus]")), end_of_track()});
    // Clone Hero reads only the first events track (MidiFile::events_track).
    const std::vector<uint8_t> events2 =
        concat({track_name("EVENTS"), after(kVerseTick, text_event("[section Verse]")),
                end_of_track()});
    const Song song =
        load_songbytes_mid(smf_tracks({tempo, drums, events1, events2}), ChartReading{});

    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].name == "Chorus");
    CHECK(song.practice_sections[1].tick == kChorusTick);

    // The time box names Intro from its start up to the tick before Chorus,
    // the second track's Verse tick included, and Chorus from its own tick.
    const app::PreviewScene scene = app::build_preview_scene(song, nullptr);
    const std::string intro = section_line_at(song, scene, 0);
    const std::string chorus = section_line_at(song, scene, kChorusTick);
    CHECK(intro.find(song.practice_sections[0].name) != std::string::npos);
    CHECK(chorus.find(song.practice_sections[1].name) != std::string::npos);
    CHECK(intro != chorus);
    CHECK(section_line_at(song, scene, kVerseTick) == intro);
    CHECK(section_line_at(song, scene, kChorusTick - 1) == intro);
}

// Block CR-24 on docs/clone-hero/01-reading-a-chart.md: the game's .mid reader
// makes no section from "[section _ENDOFSONG]", so the time box keeps naming
// the section before it.
TEST_CASE(".mid: after [section _ENDOFSONG] the time box still names the section before it") {
    using namespace testmidi;
    constexpr uint32_t kEndOfSongTick = 1920;
    const Song song = load_songbytes_mid(
        smf_tracks({concat({set_tempo(), end_of_track()}),
                    concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
                    concat({track_name("EVENTS"), text_event("[section Intro]"),
                            after(kEndOfSongTick, text_event("[section _ENDOFSONG]")),
                            end_of_track()})}),
        ChartReading{});
    const app::PreviewScene scene = app::build_preview_scene(song, nullptr);
    REQUIRE(scene.sections.size() == 1);
    // On the _ENDOFSONG tick and a beat past it, the line is the one Intro shows.
    const std::string intro = section_line_at(song, scene, 0);
    CHECK(intro.find(scene.sections[0].name) != std::string::npos);
    CHECK(section_line_at(song, scene, kEndOfSongTick) == intro);
    CHECK(section_line_at(song, scene, kEndOfSongTick + song.tick_resolution()) == intro);
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
    // No chord at tick 384: the lone cymbal marker made none.
    REQUIRE(song.sequence.size() == 2);
    // Tick 0 is red alone, a plain hit: no yellow, and the yellow markers
    // changed nothing on the red.
    const hydra::Chord& first = song.sequence[0].chord;
    CHECK(first.count() == 1);
    REQUIRE(first.at(NoteColor::Red).has_value());
    CHECK_FALSE(first.at(NoteColor::Yellow).has_value());
    CHECK_FALSE(first.at(NoteColor::Red)->is_cymbal());
    CHECK_FALSE(first.at(NoteColor::Red)->is_dynamic());
    // Tick 768 is a yellow note that keeps its real cymbal marker.
    const hydra::Chord& second = song.sequence[1].chord;
    CHECK(song.sequence[1].timecode.ticks() == 768);
    CHECK(second.count() == 1);
    REQUIRE(second.at(NoteColor::Yellow).has_value());
    CHECK(second.at(NoteColor::Yellow)->is_cymbal());
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
    CHECK_FALSE(offset("500ms").has_value());  // not 500 seconds
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
    CHECK_FALSE(delay("nan").has_value());  // absent, so the Offset applies
    CHECK_FALSE(delay("inf").has_value());
    CHECK_FALSE(delay("250ms").has_value());
    fs::remove_all(dir);
}

// Malformed .chart lines: each case pins what the reader does with one kind
// of bad line.
TEST_CASE(".chart: malformed lines keep their handling") {
    const std::string song = "[Song]\n{\n  Resolution = 192\n}\n";
    const std::string sync = "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n";
    auto parse = [](const std::string& text) { return load_songtext_chart(text); };

    // A blank line outside a section is not a header: the chart is refused.
    CHECK_THROWS_AS(parse(song + "\n" + sync), ChartFileError);
    // So is trailing whitespace after the last section, or a stray '}'.
    CHECK_THROWS_AS(parse(song + sync + "   "), ChartFileError);
    CHECK_THROWS_AS(parse(song + "}\n" + sync), ChartFileError);

    // Each malformed chart below must read exactly as the clean chart it
    // stands for (testsong::songs_equal: timing, features and every note).
    const auto reads_as = [&](const std::string& malformed, const std::string& clean) {
        CAPTURE(malformed);
        CHECK(testsong::songs_equal(parse(malformed), parse(clean)));
    };
    // The clean chart: a red at tick 0 and a yellow at tick 192.
    const std::string two_notes = song + sync + "[ExpertDrums]\n{\n  0 = N 1 0\n  192 = N 2 0\n}\n";

    // A second '=' ends the value: "N 1 0 = junk" reads as "N 1 0".
    reads_as(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0 = junk\n  192 = N 2 0=\n}\n", two_notes);

    // Lines with no '=' or no value, and an N with too few words, add nothing.
    reads_as(song + sync + "[ExpertDrums]\n{\n  garbage\n  =\n  0 = N 1\n  0 = N 3 0\n}\n",
             song + sync + "[ExpertDrums]\n{\n  0 = N 3 0\n}\n");

    // A note number that is not a number refuses the chart, as std::stoi does.
    CHECK_THROWS_AS(parse(song + sync + "[ExpertDrums]\n{\n  0 = N x 0\n}\n"),
                    std::invalid_argument);

    // A section that never closes is dropped.
    CHECK(parse(song + sync + "[ExpertDrums]\n{\n  0 = N 1 0\n").sequence.empty());

    // A repeated section replaces the earlier one.
    const std::string song480 = testchart::section("Song", "  Resolution = 480\n");
    reads_as(song + song480 + sync +
                 "[ExpertDrums]\n{\n  0 = N 1 0\n}\n[ExpertDrums]\n{\n  0 = N 2 0\n}\n",
             song480 + sync + "[ExpertDrums]\n{\n  0 = N 2 0\n}\n");

    // Tabs and CRLF are whitespace; tick keys read like std::stoll ("+192").
    reads_as("\t[Song]\v\f\n{\n Resolution\t=\t192 \r\n}\r\n" + sync +
                 "[ExpertDrums]\n{\n  0\t=\tN\t1\t0\n  +192 = N 2 0\n}\n",
             two_notes);

    // Generic E events: quotes and outer spaces come off, inner spaces stay,
    // and an E with no text makes no section. Each name is the one
    // section_display_name gives the event's text with only that taken off.
    {
        Song s = parse(song + sync +
                       "[Events]\n{\n  0 = E   \"section  Intro  \"  \n  96 = E\n"
                       "  48 = E section Verse\n  10 = E \"prc_chorus\"\n}\n"
                       "[ExpertDrums]\n{\n  0 = N 1 0\n}\n");
        REQUIRE(s.practice_sections.size() == 3);
        CHECK(s.practice_sections[0].tick == 0);
        CHECK(s.practice_sections[0].name == section_display_name("section  Intro  "));
        CHECK(s.practice_sections[1].tick == 10);
        CHECK(s.practice_sections[1].name == section_display_name("prc_chorus"));
        CHECK(s.practice_sections[2].tick == 48);
        CHECK(s.practice_sections[2].name == section_display_name("section Verse"));
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
    // The owner's refusal for timing maps with no tempo at all; every chart
    // below must be refused with this same reason.
    const Refusal no_tempo = refusal_of([] { check_timing_maps(192, {}, {}); });
    REQUIRE(no_tempo.kind == ErrorKind::ChartTimingRefused);
    REQUIRE_FALSE(no_tempo.what.empty());
    const std::string drums =
        testchart::section("ExpertDrums", "  0 = N 0 0\n  384 = N 1 0\n  576 = N 2 0\n");
    for (const std::string& sync : {std::string("  0 = TS 4\n  384 = B 60000\n"),
                                    std::string("  0 = TS 4\n"), std::string()}) {
        CAPTURE(sync);
        const Refusal r = chart_refusal(testchart::chart_text(drums, 192, "", sync));
        CHECK(r.kind == ErrorKind::ChartTimingRefused);
        CHECK(r.what == no_tempo.what);
    }
    // No [SyncTrack] section at all.
    const Refusal none =
        chart_refusal(testchart::section("Song", "  Resolution = 192\n") + drums);
    CHECK(none.kind == ErrorKind::ChartTimingRefused);
    CHECK(none.what == no_tempo.what);

    // A .mid with no set-tempo event.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf_drums(
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        std::vector<uint8_t>{});
    const Refusal m = refusal_of([&] { load_songbytes_mid(mid, ChartReading{}); });
    CHECK(m.kind == ErrorKind::ChartTimingRefused);
    CHECK(m.what == no_tempo.what);
}

// Resolution is read once, by one number rule (ChartParser::parse). Text it
// can't read refuses the chart as a timing refusal with a plain reason, not
// the standard library's words.
TEST_CASE(".chart: a Resolution that can't be read is a timing refusal") {
    const std::string sync = testchart::section("SyncTrack", "  0 = TS 4\n  0 = B 120000\n");
    const std::string drums = testchart::section("ExpertDrums", "  0 = N 1 0\n");
    auto with_song = [&](const std::string& song_lines) {
        return testchart::section("Song", song_lines) + sync + drums;
    };
    // One entry per cause: the Resolution is missing, is not a whole number,
    // or is too large for one. Each refusal is a timing refusal whose reason
    // the reader builds on kResolutionRefusalPrefix, so none is the standard
    // library's text. Charts with one cause share one reason, and two causes
    // never share one.
    const std::vector<std::vector<std::string>> causes = {
        {sync + drums, with_song("  Name = x\n")},
        {with_song("  Resolution = abc\n"), with_song("  Resolution = x192\n")},
        {with_song("  Resolution = 99999999999999999999\n")}};
    std::vector<std::string> reasons;
    for (const std::vector<std::string>& charts : causes) {
        const std::string reason = chart_refusal(charts.front()).what;
        for (const std::string& text : charts) {
            CAPTURE(text);
            const Refusal r = chart_refusal(text);
            CHECK(r.kind == ErrorKind::ChartTimingRefused);
            CHECK(starts_with(r.what, kResolutionRefusalPrefix));
            CHECK(r.what.size() > kResolutionRefusalPrefix.size());
            CHECK(r.what == reason);
        }
        for (const std::string& other : reasons) CHECK(reason != other);
        reasons.push_back(reason);
    }
    // Valid text keeps its number rule: leading digits read, trailing junk ignored.
    const std::string ok = testchart::section("Song", "  Resolution = 480abc\n") + sync + drums;
    CHECK(load_songbytes_chart(std::vector<uint8_t>(ok.begin(), ok.end()), ChartReading{})
              .tick_resolution() == 480);
}

// ---- display_title: the one cleaned song title (findings 8 and 111) ----------

TEST_CASE("display_title: a title made only of Clone Hero tags reads (unknown)") {
    CHECK(display_title(test::kTagOnlyTitle) == kUnknownTitle);
    CHECK(display_title("   ") == kUnknownTitle);
}

TEST_CASE("display_title: tags go, words stay, spaces are trimmed") {
    CHECK(display_title("<b>Bold</b>") == "Bold");
    CHECK(display_title("<color=#e02222>Blood</color>line") == "Bloodline");
    CHECK(display_title(" Some Song ") == "Some Song");
}

TEST_CASE("display_title: a clean name and the old placeholder behave like title_or_unknown") {
    CHECK(display_title("Some Song") == "Some Song");
    CHECK(display_title(kOldTitlePlaceholder) == kUnknownTitle);
}

TEST_CASE("display_title: an artist reads by the same rule (D50 item 5)") {
    // An artist made only of tags reads kUnknownTitle, like a title.
    CHECK(display_artist(test::kTagOnlyTitle) == kUnknownTitle);
    CHECK(display_artist(" <i>Tagged</i> Artist ") == "Tagged Artist");
}

TEST_CASE("display_artist: a missing artist reads (unknown) however it is stored (D56 item 2)") {
    // Empty, the scan's placeholder, or only tags: all three read kUnknownTitle.
    CHECK(display_artist("") == kUnknownTitle);
    CHECK(display_artist(kUnknownArtist) == kUnknownTitle);
    CHECK(display_artist(" <b></b> ") == kUnknownTitle);
    // The stored text is not touched: the scan still writes its placeholder.
    CHECK(artist_or_unknown("") == kUnknownArtist);
}

TEST_CASE("display_charter: tags go and the ends are trimmed") {
    CHECK(display_charter(" <b>Bob</b> ") == "Bob");
    CHECK(display_charter("<color=red> Hoph2o </color>") == "Hoph2o");
}

TEST_CASE("display_charter: a missing charter reads (unknown) however it is stored (D266 item 2)") {
    // Empty, the scan's placeholder, or only tags: all three read kUnknownTitle,
    // the same text a missing artist shows.
    CHECK(display_charter("") == kUnknownTitle);
    CHECK(display_charter(kUnknownCharter) == kUnknownTitle);
    CHECK(display_charter(test::kTagOnlyTitle) == kUnknownTitle);
    CHECK(display_charter(" <b></b> ") == kUnknownTitle);
    // The stored text is not touched: the scan still writes its placeholder.
    CHECK(charter_or_unknown("") == kUnknownCharter);
}

TEST_CASE("display_album: tags go and the ends are trimmed, and a blank stays blank") {
    CHECK(display_album(" <b>Dookie</b> ") == "Dookie");
    CHECK(display_album("<color=red> Chair </color>") == "Chair");
    CHECK(display_album(test::kTagOnlyTitle) == "");
    CHECK(display_album("") == "");
    CHECK(display_album("   ") == "");
    // The artist placeholder means nothing to an album: it stays as it is.
    CHECK(display_album(kUnknownArtist) == kUnknownArtist);
}
