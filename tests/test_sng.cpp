// Tests for parse/sng: the one reader of the .sng container layout, used by
// the note loader, the Preview's audio extractor and the library scan.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "byte_source_util.h"
#include "core/winstr.h"
#include "midi_util.h"
#include "parse/chart_files.h"
#include "parse/sng.h"
#include "parse/song.h"
#include "sng_util.h"
#include "song_equal.h"
#include "temp_util.h"

using namespace hydra;
using testsong::songs_equal;

namespace {

using testsng::make_sng;

std::vector<uint8_t> tiny_mid() {
    return testmidi::smf(testmidi::concat({testmidi::track_name("PART DRUMS"),
                                           testmidi::set_tempo(),
                                           testmidi::note_on(96, 100),
                                           {0x83, 0x60, 0x90, 97, 100},  // tick 480: red
                                           testmidi::end_of_track()}));
}

// A scratch path (testtemp::temp_path) for one fixture. The name's extension
// stays last, where the loaders look for it.
std::string sng_fixture_path(const char* name) {
    const std::filesystem::path p = std::filesystem::u8path(name);
    return testtemp::temp_path("sng_" + p.stem().u8string(), p.extension().u8string());
}

using testtemp::write_bytes;

}  // namespace

TEST_CASE("sng: metadata pairs read back in order") {
    std::vector<uint8_t> buf = make_sng({{"name", "Song"}, {"Artist", "Band"}}, {});
    auto pairs = sng_read_metadata(buf);
    REQUIRE(pairs.size() == 2);
    CHECK(pairs[0] == std::make_pair(std::string("name"), std::string("Song")));
    CHECK(pairs[1] == std::make_pair(std::string("Artist"), std::string("Band")));
}

TEST_CASE("sng: the file table and each file decode") {
    const std::vector<uint8_t> mid = tiny_mid();
    const std::vector<uint8_t> ogg = {'O', 'g', 'g', 'S', 1, 2, 3};
    std::vector<uint8_t> buf = make_sng({{"name", "Song"}}, {{"notes.mid", mid}, {"song.ogg", ogg}});
    auto table = sng_read_file_table(buf);
    REQUIRE(table.size() == 2);
    CHECK(table[0].name == "notes.mid");
    CHECK(table[1].name == "song.ogg");
    auto got_mid = sng_decode_file(buf, table[0]);
    auto got_ogg = sng_decode_file(buf, table[1]);
    REQUIRE(got_mid.has_value());
    REQUIRE(got_ogg.has_value());
    CHECK(*got_mid == mid);
    CHECK(*got_ogg == ogg);
}

TEST_CASE("sng: truncated input stops early instead of reading past the end") {
    std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {{"notes.mid", tiny_mid()}});
    CHECK(sng_read_metadata({1, 2, 3}).empty());
    CHECK(sng_read_file_table({1, 2, 3}).empty());

    // Cut inside the file table: the entry is dropped, nothing is read past the end.
    std::vector<uint8_t> cut(whole.begin(), whole.begin() + (whole.size() - tiny_mid().size() - 4));
    CHECK(sng_read_file_table(cut).empty());

    // An entry whose offset + length wraps around is refused.
    SngFileEntry bad;
    bad.name = "notes.mid";
    bad.offset = 40;
    bad.length = UINT64_MAX - 10;
    CHECK_FALSE(sng_decode_file(whole, bad).has_value());
}

TEST_CASE("sng: the note loader reads the chart through the shared reader") {
    const std::string path = sng_fixture_path("loader.sng");
    write_bytes(path, make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}, {"NOTES.MID", tiny_mid()}}));
    CHECK(songs_equal(load_songpath_sng(path, true, true),
                      load_songbytes_mid(tiny_mid(), true, true)));

    // A file too short to hold a table throws a clear error.
    const std::string tiny = sng_fixture_path("tiny.sng");
    write_bytes(tiny, {1, 2, 3});
    CHECK_THROWS_AS(load_songpath_sng(tiny, true, true), std::runtime_error);
}

TEST_CASE("sng: unmasking into the caller's buffer matches the byte formula") {
    // 1,000 bytes cross the 256-byte key period several times and end mid-block.
    std::vector<uint8_t> payload(1000);
    for (size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<uint8_t>((i * 37 + 11) ^ (i >> 3));
    const std::vector<uint8_t> buf =
        make_sng({}, {{"a.bin", {9, 8, 7}}, {"song.ogg", payload}, {"empty.bin", {}}});
    const auto table = sng_read_file_table(buf);
    REQUIRE(table.size() == 3);

    // The formula itself, written out per byte, on the stored bytes.
    const uint8_t* mask = buf.data() + kSngXorMaskOffset;
    std::vector<uint8_t> by_formula(payload.size());
    for (size_t i = 0; i < by_formula.size(); ++i)
        by_formula[i] = static_cast<uint8_t>(buf[static_cast<size_t>(table[1].offset) + i] ^
                                             mask[i % 16] ^ (i & 0xff));
    CHECK(by_formula == payload);

    std::vector<uint8_t> out = {1, 2, 3, 4, 5};  // stale contents get replaced
    REQUIRE(sng_decode_file_into(buf, table[1], out));
    CHECK(out == payload);
    REQUIRE(sng_decode_file_into(buf, table[0], out));
    CHECK(out == std::vector<uint8_t>{9, 8, 7});
    REQUIRE(sng_decode_file_into(buf, table[2], out));
    CHECK(out.empty());

    // Out of range: false, and the buffer is left as it was.
    SngFileEntry bad = table[1];
    bad.length += 1;
    std::vector<uint8_t> keep = {42};
    CHECK_FALSE(sng_decode_file_into(buf, bad, keep));
    CHECK(keep == std::vector<uint8_t>{42});
    CHECK_FALSE(sng_decode_file(buf, bad).has_value());
}

namespace {

// load_songpath's own dispatch on the file, counting the bytes it reads.
Song load_counting(const std::string& path, uint64_t& bytes_read) {
    bytes_read = 0;
    return load_songpath_reading(testbytes::counting(file_byte_source(path), bytes_read), path,
                                 true, true);
}

}  // namespace

// A real container is mostly audio (the library's Endless Setlist .sng files
// are about 1 GB each); the notes are a few hundred KB of it.
TEST_CASE("sng: the note loader reads the header and the notes, not the audio") {
    std::vector<uint8_t> audio(16 << 20);
    for (size_t i = 0; i < audio.size(); ++i) audio[i] = static_cast<uint8_t>(i * 131 + 7);
    const std::vector<uint8_t> buf =
        make_sng({{"name", "Song"}}, {{"song.ogg", audio}, {"notes.mid", tiny_mid()}});
    const std::string path = sng_fixture_path("big_audio.sng");
    write_bytes(path, buf);

    uint64_t bytes_read = 0;
    const Song via_reads = load_counting(path, bytes_read);
    CHECK(songs_equal(via_reads, load_songbytes_sng(buf, true, true)));
    CHECK(bytes_read < (1u << 20));
    CHECK(bytes_read > 0);
}

TEST_CASE("sng: a header longer than the first read still loads") {
    // A 300 KB metadata value pushes the file table past any small first read.
    const std::string big(300 * 1024, 'x');
    const std::vector<uint8_t> buf = make_sng(
        {{"name", "Song"}, {"loading_phrase", big}},
        {{"song.ogg", {1, 2, 3}}, {"album.png", {4, 5}}, {"notes.mid", tiny_mid()}});
    const std::string path = sng_fixture_path("big_header.sng");
    write_bytes(path, buf);

    uint64_t bytes_read = 0;
    CHECK(songs_equal(load_counting(path, bytes_read), load_songbytes_sng(buf, true, true)));
}

// A file can shrink after it was opened and sized: the source then claims
// more bytes than its reads hand back. A short read ends the header read;
// waiting for the claimed size used to loop forever.
TEST_CASE("sng: a source shorter than its stated size ends the header read") {
    const std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {{"notes.mid", tiny_mid()}});
    const std::vector<uint8_t> cut(whole.begin(), whole.begin() + 10);
    ByteSource shrunk = memory_byte_source(cut);
    shrunk.size = 200000;
    CHECK(sng_read_head(shrunk) == cut);
    CHECK_THROWS_WITH(load_songpath_reading(shrunk, "shrunk.sng", true, true),
                      "No chart files found in SNG file.");

    // A short first read ends the header even when later reads would be whole.
    CHECK(sng_read_head(testbytes::short_first_read(memory_byte_source(whole), 10)) ==
          std::vector<uint8_t>(whole.begin(), whole.begin() + 10));
}

TEST_CASE("sng: ranged reads fail a damaged container the way a whole read does") {
    const std::vector<uint8_t> whole =
        make_sng({{"name", "Song"}}, {{"notes.mid", tiny_mid()}});

    // The notes entry runs past the end of the file.
    std::vector<uint8_t> cut_notes(whole.begin(), whole.end() - 5);
    const std::string cut_path = sng_fixture_path("cut_notes.sng");
    write_bytes(cut_path, cut_notes);
    CHECK_THROWS_WITH(load_songpath(cut_path, true, true), "Truncated SNG file.");
    CHECK_THROWS_WITH(load_songbytes_sng(cut_notes, true, true), "Truncated SNG file.");

    // The file table is cut: no notes entry is found.
    std::vector<uint8_t> cut_table(whole.begin(),
                                   whole.begin() + (whole.size() - tiny_mid().size() - 4));
    const std::string table_path = sng_fixture_path("cut_table.sng");
    write_bytes(table_path, cut_table);
    CHECK_THROWS_WITH(load_songpath(table_path, true, true), "No chart files found in SNG file.");

    // A metadata length far past the end of the file.
    std::vector<uint8_t> huge_meta = whole;
    for (int i = 0; i < 8; ++i) huge_meta[kSngMetadataLenOffset + i] = 0x7f;
    const std::string meta_path = sng_fixture_path("huge_meta.sng");
    write_bytes(meta_path, huge_meta);
    CHECK_THROWS_WITH(load_songpath(meta_path, true, true), "No chart files found in SNG file.");
}

TEST_CASE("sng: a container parses the same from bytes as from its path") {
    const std::vector<uint8_t> buf =
        make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}, {"notes.mid", tiny_mid()}});
    const std::string path = sng_fixture_path("from_bytes.sng");
    write_bytes(path, buf);
    CHECK(songs_equal(load_songpath_from_bytes(path, buf, true, true),
                      load_songpath(path, true, true)));
    // The extension still decides the format; an unknown one throws.
    CHECK_THROWS_AS(load_songpath_from_bytes("x.txt", buf, true, true), std::runtime_error);
}

// Clone Hero, its song cache and dmleaderboards key a .sng by the MD5 of the
// notes file inside it, as they key a folder chart by its notes file's MD5
// (54 of 54 .sng files in the user's songcache.bin, 2026-10-09). The audio
// and art around the notes never change the id.
TEST_CASE("sng: a chart's id is the MD5 of the notes file inside it") {
    const std::string dir = testtemp::temp_dir("sng_scan_id");
    const std::string loose = dir + "\\notes.mid";
    write_bytes(loose, tiny_mid());
    const std::string with_ogg = dir + "\\a.sng";
    write_bytes(with_ogg, make_sng({{"name", "Song"}, {"artist", "Band"}},
                                   {{"song.ogg", {1, 2}}, {"notes.mid", tiny_mid()}}));
    const std::string other_audio = dir + "\\b.sng";
    write_bytes(other_audio, make_sng({{"name", "Song"}},
                                      {{"notes.mid", tiny_mid()}, {"drums.ogg", {9, 9, 9}}}));

    const std::string id = app::hash_chart_file(loose);
    REQUIRE(id.size() == 32);
    CHECK(app::hash_chart_file(with_ogg) == id);
    CHECK(app::hash_chart_file(other_audio) == id);

    // The scan gives the same id, and still reads the names.
    auto [items, errors] = app::discover_charts({dir});
    CHECK(errors.empty());
    REQUIRE(items.size() == 2);  // the loose notes.mid has no song.ini: no chart
    for (const app::ScanItem& item : items) {
        CAPTURE(item.notespath);
        CHECK(item.md5 == id);
        CHECK(item.title == "Song");
    }
}

// A .sng that holds no notes file has no id: the scan lists it as an error
// instead of a chart, and a lookup by path finds nothing.
TEST_CASE("sng: a container without a notes file is a scan error") {
    const std::string dir = testtemp::temp_dir("sng_scan_no_notes");
    const std::string path = dir + "\\audio_only.sng";
    write_bytes(path, make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}}));

    auto [items, errors] = app::discover_charts({dir});
    CHECK(items.empty());
    REQUIRE(errors.size() == 1);
    CHECK(errors[0].find("No chart files found in SNG file.") != std::string::npos);
    CHECK(app::hash_chart_file(path).empty());
}

// The chart-files pins live here because test_song.cpp belongs to another
// task this phase.
TEST_CASE("chart_files: pick_notes_file takes the first notes.mid, else the last notes.chart") {
    const std::optional<NotesFilePick> mid =
        pick_notes_file({"song.ogg", "notes.chart", "NOTES.MID", "notes.chart"});
    REQUIRE(mid.has_value());
    CHECK(mid->index == 2);
    CHECK(mid->format == ChartFormat::Mid);

    const std::optional<NotesFilePick> chart =
        pick_notes_file({"notes.chart", "song.ogg", "Notes.Chart"});
    REQUIRE(chart.has_value());
    CHECK(chart->index == 2);
    CHECK(chart->format == ChartFormat::Chart);

    CHECK_FALSE(pick_notes_file({"song.ogg", "album.png"}).has_value());
}

TEST_CASE("chart_files: find_song_ini picks the folder's song.ini in any case") {
    const std::vector<DirEntry> listing = {{"notes.mid"}, {"Song.INI"}, {"song.ogg"}};
    const DirEntry* ini = find_song_ini(listing);
    REQUIRE(ini != nullptr);
    CHECK(ini->name == "Song.INI");
    CHECK(find_song_ini(std::vector<DirEntry>{{"notes.mid"}, {"song.ogg"}}) == nullptr);
    DirEntry folder;
    folder.name = "song.ini";
    folder.is_dir = true;
    CHECK(find_song_ini(std::vector<DirEntry>{folder}) == nullptr);

    // The folder form lists a real folder and picks from it.
    const std::string dir = sng_fixture_path("ini_folder");
    const std::string empty_dir = sng_fixture_path("ini_folder_empty");
    std::filesystem::create_directory(os_path(dir));
    std::filesystem::create_directory(os_path(empty_dir));
    write_bytes(dir + "\\SONG.INI", {'[', 's', 'o', 'n', 'g', ']'});
    CHECK(find_song_ini(dir) == dir + "\\SONG.INI");
    CHECK(find_song_ini(empty_dir).empty());
}
