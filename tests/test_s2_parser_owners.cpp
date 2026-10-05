// Step 2, task 6: the small parser owners (decision D27). Each rule below has
// one owner in the code, and each case pins what that owner answers.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/preview_source.h"
#include "app/preview_view.h"
#include "chart_text.h"
#include "core/model.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "srb_util.h"
#include "store/record_store.h"

using namespace hydra;

namespace {

// A .chart at 192 ticks a beat, 4/4 and 120 BPM, with `song_extra` lines in
// [Song] and `drums` lines in [ExpertDrums].
std::vector<uint8_t> chart_bytes(const std::string& song_extra, const std::string& drums) {
    return testchart::chart_bytes(testchart::section("ExpertDrums", drums), 192, song_extra);
}

// An .srb (tests/srb_util.h) whose metadata names its notes stream `notes_name`.
std::vector<uint8_t> srb_with(const std::string& notes_name, const std::vector<uint8_t>& notes) {
    return testsrb::make_srb(testsrb::make_metadata(notes_name), notes);
}

}  // namespace

TEST_CASE("s2 owners: Chord modifiers on a missing note are a chart-file error (331)") {
    // hydra:: because windows.h declares a GDI function named Chord.
    hydra::Chord c;
    c.add_note(NoteColor::Red);
    CHECK_THROWS_AS(c.apply_cymbal(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_ghost(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_accent(NoteColor::Blue), ChartFileError);
    c.apply_ghost(NoteColor::Red);
    CHECK(c.at(NoteColor::Red)->is_ghost());
}

TEST_CASE("s2 owners: a .chart modifier with no note under it is skipped (331)") {
    // Tick 0: red alone, plus a yellow cymbal, a yellow ghost and a yellow
    // accent marker with no yellow note to change. Tick 384: a cymbal marker
    // with no chord at all. Tick 768: a yellow note whose cymbal marker is real.
    const std::string drums =
        "  0 = N 1 0\n  0 = N 66 0\n  0 = N 41 0\n  0 = N 35 0\n"
        "  384 = N 66 0\n"
        "  768 = N 2 0\n  768 = N 66 0\n";
    const std::vector<uint8_t> data = chart_bytes("", drums);
    REQUIRE_NOTHROW(load_songbytes_chart(data, true, true));
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].chord.code() == ".n...");
    CHECK(song.sequence[1].timecode.ticks() == 768);
    CHECK(song.sequence[1].chord.code() == "..N..");
}

TEST_CASE("s2 owners: parse_finite_number reads only a whole finite number (R7.5)") {
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

TEST_CASE("s2 owners: a .chart Offset that is not a plain number is absent (R7.5)") {
    auto offset = [](const std::string& text) {
        return load_songbytes_chart(chart_bytes("  Offset = " + text + "\n", "  0 = N 1 0\n"),
                                    true, true)
            .chart_offset_s;
    };
    CHECK(offset("0.25") == 0.25);
    CHECK(offset("1") == 1.0);
    CHECK(offset("-0.5") == -0.5);
    CHECK_FALSE(offset("500ms").has_value());  // 500 seconds before this change
    CHECK_FALSE(offset("0.25s").has_value());
    CHECK_FALSE(offset("nan").has_value());
}

TEST_CASE("s2 owners: a song.ini delay that is not a plain number is absent (R7.5)") {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() /
                         ("hydra_s2_delay_" + std::to_string(GetCurrentProcessId()));
    fs::create_directories(dir);
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

TEST_CASE("s2 owners: .mid practice sections come out in tick order (R7.7)") {
    using namespace testmidi;
    const std::vector<uint8_t> tempo = concat({set_tempo(), end_of_track()});
    const std::vector<uint8_t> drums =
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()});
    // First EVENTS track: Intro at 0, Chorus at 1920.
    const std::vector<uint8_t> events1 =
        concat({track_name("EVENTS"), text_event("[section Intro]"),
                after(1920, text_event("[section Chorus]")), end_of_track()});
    // Second EVENTS track: Verse at 960.
    const std::vector<uint8_t> events2 =
        concat({track_name("EVENTS"), after(960, text_event("[section Verse]")),
                end_of_track()});
    const Song song = load_songbytes_mid(smf_tracks({tempo, drums, events1, events2}), true, true);

    REQUIRE(song.practice_sections.size() == 3);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].name == "Verse");
    CHECK(song.practice_sections[1].tick == 960);
    CHECK(song.practice_sections[2].name == "Chorus");

    // The time box names the section the playhead is in. At 120 BPM and 480
    // ticks a beat, tick 1000 is 1041.67 ms (Verse) and tick 2000 is 2083.33 ms
    // (Chorus). Before the sort it said Intro, then Verse.
    const app::PreviewScene scene = app::build_preview_scene(song, nullptr);
    CHECK(app::build_time_box(scene, 1041.7, 3000.0).section_line == "Section Verse");
    CHECK(app::build_time_box(scene, 2083.4, 3000.0).section_line == "Section Chorus");
}

TEST_CASE("s2 owners: Song's default meter is written once, through apply_timesig (258, 319)") {
    const Song song(192);
    CHECK(song.tpm_changes == std::map<int64_t, int64_t>{{0, 768}});
    CHECK(song.timesig_changes.at(0) ==
          std::make_pair(kDefaultTimeSigNumerator, kDefaultTimeSigDenominator));

    // Fixtures set a meter and its signature together, through the owner.
    Song fixture(480);
    apply_timesig(fixture, 2880, 3, 4);
    CHECK(fixture.tpm_changes.at(2880) == 1440);
    CHECK(fixture.timesig_changes.at(2880) == std::make_pair(3, 4));

    // A scene built from nothing reads Song's default, not a literal of its own.
    const app::PreviewTimeSig none;
    CHECK(none.numerator == kDefaultTimeSigNumerator);
    CHECK(none.denominator == kDefaultTimeSigDenominator);
}

// "No meter is written by hand outside apply_timesig" and "the Preview's time
// box keeps no 4/4 literal of its own" are rows in test_single_owner.cpp, the
// one scan of the source tree.

TEST_CASE("s2 owners: a container's notes entry is found by its exact name (61)") {
    using namespace testmidi;
    const std::vector<uint8_t> mid =
        smf(concat({track_name("PART DRUMS"), set_tempo(), note_on(96, 100), end_of_track()}));
    // "song.chart" is not a notes name. Before, the .srb trusted the extension
    // and read these MIDI bytes as .chart text, which failed. Now the name
    // decides nothing, so the stream's bytes do, and it loads as MIDI.
    CHECK(load_songbytes_srb(srb_with("song.chart", mid), true, true).sequence.size() == 1);
    // The exact names decide, in any case.
    CHECK(load_songbytes_srb(srb_with("NOTES.MID", mid), true, true).sequence.size() == 1);
}

TEST_CASE("s2 owners: a blank artist or charter reads the placeholder (60)") {
    CHECK(artist_or_unknown("") == kUnknownArtist);
    CHECK(charter_or_unknown("") == kUnknownCharter);
    CHECK(artist_or_unknown("X") == "X");
    CHECK(std::string(kUnknownArtist) == "<unknown artist>");
    CHECK(std::string(kUnknownCharter) == "<unknown charter>");

    namespace fs = std::filesystem;
    const std::string chart = corpus::first_chart_with_suffix(".chart");
    REQUIRE(!chart.empty());
    const fs::path root = fs::temp_directory_path() /
                          ("hydra_s2_blank_meta_" + std::to_string(GetCurrentProcessId()));
    fs::remove_all(root);
    // "artist =" and "charter =" present but blank (56 library song.ini files
    // have a blank charter).
    fs::create_directories(root / "blank");
    fs::copy_file(fs::u8path(chart), root / "blank" / "notes.chart");
    {
        std::ofstream ini(root / "blank" / "song.ini", std::ios::binary);
        ini << "[song]\nname = N\nartist =\ncharter =\n";
    }
    // Both keys missing.
    fs::create_directories(root / "missing");
    fs::copy_file(fs::u8path(chart), root / "missing" / "notes.chart");
    {
        std::ofstream ini(root / "missing" / "song.ini", std::ios::binary);
        ini << "[song]\nname = N\n";
    }

    auto [items, errors] = app::discover_charts({root.u8string()});
    CHECK(errors.empty());
    REQUIRE(items.size() == 2);
    for (const app::ScanItem& it : items) {
        CAPTURE(it.notespath);
        CHECK(it.artist == kUnknownArtist);
        CHECK(it.charter == kUnknownCharter);
    }

    // A rescan-cache row from before this change still holds the blanks. The
    // scan reads it through the same fallback.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> entries;
    for (const app::ScanItem& it : items) {
        store::ChartLibraryEntry e = app::to_library_entry(it);
        e.artist.clear();
        e.charter.clear();
        entries.push_back(std::move(e));
    }
    store.rebuild_chart_library(entries);
    store::ChartLibraryCache cache = store.chart_library_cache();
    auto [cached, errors2] = app::discover_charts({root.u8string()}, app::ScanCallbacks{}, &cache);
    CHECK(errors2.empty());
    REQUIRE(cached.size() == 2);
    for (const app::ScanItem& it : cached) {
        CHECK(it.artist == kUnknownArtist);
        CHECK(it.charter == kUnknownCharter);
    }
    fs::remove_all(root);
}
