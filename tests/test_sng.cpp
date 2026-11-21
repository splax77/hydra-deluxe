// Tests for parse/sng: the one reader of the .sng container layout, used by
// the note loader, the Preview's audio extractor and the library scan.
//
// No real .sng is checked in, so the containers here are written by make_sng
// (sng_util.h) around a real corpus chart's notes file. What a container load
// gives is checked against that chart's notes read straight from the file.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/user_messages.h"
#include "byte_source_util.h"
#include "core/error_kind.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "parse/chart_files.h"
#include "load_util.h"
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
    return testmidi::smf_drums(testmidi::concat({testmidi::track_name("PART DRUMS"),
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

// The notes file of the first corpus chart with Expert notes: its path, its
// file name (notes.mid or notes.chart, the names pick_notes_file takes) and
// its bytes.
struct CorpusNotes {
    std::string path;
    std::string name;
    std::vector<uint8_t> bytes;
};

CorpusNotes corpus_notes() {
    const std::string path = corpus::first_chart_with_notes();
    return CorpusNotes{path, std::filesystem::u8path(path).filename().u8string(),
                       read_file_bytes(path)};
}

// The chart read straight from its notes file, with no song.ini keys: what a
// .sng holding that file, with no drum keys in its metadata, must load as.
Song notes_song(const CorpusNotes& notes) {
    if (chart_format_of(notes.name) == ChartFormat::Mid)
        return load_songbytes_mid(notes.bytes, ChartReading{});
    return load_songbytes_chart(notes.bytes, ChartReading{});
}

// `buf` with the little-endian u64 at `at` replaced by `value`, written the
// way make_sng writes its numbers.
std::vector<uint8_t> with_u64_at(std::vector<uint8_t> buf, size_t at, uint64_t value) {
    std::vector<uint8_t> le;
    testbytes::put_u64(le, value);
    std::copy(le.begin(), le.end(), buf.begin() + static_cast<std::ptrdiff_t>(at));
    return buf;
}

// What a load that fails throws: its kind, its raw text, and the one line the
// scan's problem list shows for it (app::plain_error_line). Empty when the
// load throws nothing.
struct LoadFailure {
    ErrorKind kind;
    std::string what;
    std::string line;
};

template <class Load>
std::optional<LoadFailure> load_failure(Load load) {
    try {
        load();
    } catch (const KindedError& e) {
        return LoadFailure{e.kind(), std::string(e.what()), app::plain_error_line(e)};
    }
    return std::nullopt;
}

// The failure of a container whose file table names no notes file: an
// audio-only .sng, read from its bytes.
LoadFailure no_notes_failure() {
    const std::vector<uint8_t> audio_only = make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}});
    const std::optional<LoadFailure> failed =
        load_failure([&] { load_songbytes_sng(audio_only, ChartReading{}); });
    REQUIRE(failed.has_value());
    return *failed;
}

// The failure a damaged container gives, read through its path (ranged
// reads) and from its bytes (a whole read). The two must agree.
LoadFailure damaged_failure(const char* name, const std::vector<uint8_t>& bytes) {
    const std::string path = sng_fixture_path(name);
    write_bytes(path, bytes);
    const std::optional<LoadFailure> by_path =
        load_failure([&] { load_songpath(path, ChartReading{}); });
    const std::optional<LoadFailure> by_bytes =
        load_failure([&] { load_songbytes_sng(bytes, ChartReading{}); });
    REQUIRE(by_path.has_value());
    REQUIRE(by_bytes.has_value());
    CHECK(by_path->kind == ErrorKind::ChartUnreadable);
    CHECK(by_bytes->kind == by_path->kind);
    CHECK(by_bytes->line == by_path->line);
    return *by_path;
}

// load_songpath's own dispatch on the file, counting the bytes it reads.
Song load_counting(const std::string& path, uint64_t& bytes_read) {
    bytes_read = 0;
    return load_songpath_reading(testbytes::counting(file_byte_source(path), bytes_read), path,
                                 ChartReading{});
}

// The MD5 of `bytes` as lowercase hex, from Windows' own CNG provider rather
// than through Hydra's chart-id code.
std::string cng_md5_hex(const std::vector<uint8_t>& bytes) {
    BCRYPT_ALG_HANDLE alg = nullptr;
    REQUIRE(BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg, BCRYPT_MD5_ALGORITHM, nullptr, 0)));
    DWORD digest_len = 0;
    ULONG got = 0;
    REQUIRE(BCRYPT_SUCCESS(BCryptGetProperty(alg, BCRYPT_HASH_LENGTH,
                                             reinterpret_cast<PUCHAR>(&digest_len),
                                             static_cast<ULONG>(sizeof(digest_len)), &got, 0)));
    BCRYPT_HASH_HANDLE hash = nullptr;
    REQUIRE(BCRYPT_SUCCESS(BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0)));
    REQUIRE(BCRYPT_SUCCESS(BCryptHashData(hash, const_cast<PUCHAR>(bytes.data()),
                                          static_cast<ULONG>(bytes.size()), 0)));
    std::vector<uint8_t> digest(digest_len);
    REQUIRE(BCRYPT_SUCCESS(BCryptFinishHash(hash, digest.data(), digest_len, 0)));
    BCryptDestroyHash(hash);
    BCryptCloseAlgorithmProvider(alg, 0);
    return hex_lower(digest.data(), digest.size());
}

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
    std::vector<uint8_t> got_mid, got_ogg;
    REQUIRE(sng_decode_file_into(buf, table[0], got_mid));
    REQUIRE(sng_decode_file_into(buf, table[1], got_ogg));
    CHECK(got_mid == mid);
    CHECK(got_ogg == ogg);
}

TEST_CASE("sng: truncated input stops early instead of reading past the end") {
    const CorpusNotes notes = corpus_notes();
    const std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {{notes.name, notes.bytes}});
    const std::vector<SngFileEntry> table = sng_read_file_table(whole);
    REQUIRE(table.size() == 1);
    CHECK(sng_read_metadata({1, 2, 3}).empty());
    CHECK(sng_read_file_table({1, 2, 3}).empty());

    // make_sng puts the notes right after the file table, so cutting one byte
    // before the notes cuts inside the table: the entry is dropped, and
    // nothing is read past the end.
    const std::vector<uint8_t> cut(whole.begin(),
                                   whole.begin() + static_cast<std::ptrdiff_t>(table[0].offset - 1));
    CHECK(sng_read_file_table(cut).empty());

    // An entry whose offset + length wraps around is refused. The entry's
    // last two fields are its u64 length, then its u64 offset (sng.h), so its
    // stored length sits 16 bytes before the table's end.
    const uint64_t wrapping_length = UINT64_MAX - table[0].offset + 1;
    const std::vector<uint8_t> wrapped =
        with_u64_at(whole, static_cast<size_t>(table[0].offset) - 16, wrapping_length);
    const std::vector<SngFileEntry> wrapped_table = sng_read_file_table(wrapped);
    REQUIRE(wrapped_table.size() == 1);
    REQUIRE(wrapped_table[0].length == wrapping_length);
    CHECK(wrapped_table[0].offset + wrapped_table[0].length < wrapped_table[0].offset);
    std::vector<uint8_t> wrapped_out = {42};
    CHECK_FALSE(sng_decode_file_into(wrapped, wrapped_table[0], wrapped_out));
    CHECK(wrapped_out == std::vector<uint8_t>{42});
    // The note loader refuses it too, through both of its reads.
    damaged_failure("wrapped.sng", wrapped);
}

TEST_CASE("sng: the note loader reads the chart through the shared reader") {
    const CorpusNotes notes = corpus_notes();
    // The notes entry comes after the audio, under an upper-case name, so the
    // loader has to pick it by name in any case.
    std::string upper_name = notes.name;
    for (char& c : upper_name) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    const std::string path = sng_fixture_path("loader.sng");
    write_bytes(path, make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}, {upper_name, notes.bytes}}));
    const Song want = notes_song(notes);
    REQUIRE_FALSE(want.is_empty());
    CHECK(songs_equal(load_songpath_sng(path, ChartReading{}), want));

    // A file too short to hold a table names no notes file, and fails as an
    // unreadable chart, the way a container with no notes file does.
    const std::vector<uint8_t> three = {1, 2, 3};
    CHECK(sng_read_file_table(three).empty());
    const std::string tiny = sng_fixture_path("tiny.sng");
    write_bytes(tiny, three);
    const std::optional<LoadFailure> failed =
        load_failure([&] { load_songpath_sng(tiny, ChartReading{}); });
    REQUIRE(failed.has_value());
    CHECK(failed->kind == ErrorKind::ChartUnreadable);
    CHECK(failed->line == no_notes_failure().line);
}

TEST_CASE("sng: unmasking into the caller's buffer undoes the stored masking") {
    // 1,000 bytes cross the key's 256-byte period several times and end mid-block.
    std::vector<uint8_t> payload(1000);
    for (size_t i = 0; i < payload.size(); ++i)
        payload[i] = static_cast<uint8_t>((i * 37 + 11) ^ (i >> 3));
    const std::vector<uint8_t> buf =
        make_sng({}, {{"a.bin", {9, 8, 7}}, {"song.ogg", payload}, {"empty.bin", {}}});
    const auto table = sng_read_file_table(buf);
    REQUIRE(table.size() == 3);

    std::vector<uint8_t> out = {1, 2, 3, 4, 5};  // stale contents get replaced
    REQUIRE(sng_decode_file_into(buf, table[1], out));
    CHECK(out == payload);

    // The unmasking reads its mask from the file's header. Flipping one mask
    // byte flips exactly the payload bytes that mask byte unmasks: every
    // kSngXorMaskSize-th byte, from that byte's place in the mask.
    for (size_t m = 0; m < kSngXorMaskSize; ++m) {
        CAPTURE(m);
        std::vector<uint8_t> flipped = buf;
        flipped[kSngXorMaskOffset + m] = static_cast<uint8_t>(~flipped[kSngXorMaskOffset + m]);
        std::vector<uint8_t> want = payload;
        for (size_t i = m; i < want.size(); i += kSngXorMaskSize)
            want[i] = static_cast<uint8_t>(~want[i]);
        std::vector<uint8_t> got;
        REQUIRE(sng_decode_file_into(flipped, table[1], got));
        CHECK(got == want);
    }

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
}

// A real container is mostly audio; the notes are a small part of it.
TEST_CASE("sng: the note loader reads the header and the notes, not the audio") {
    const CorpusNotes notes = corpus_notes();
    std::vector<uint8_t> audio(16 << 20);
    for (size_t i = 0; i < audio.size(); ++i) audio[i] = static_cast<uint8_t>(i * 131 + 7);
    const std::vector<uint8_t> buf =
        make_sng({{"name", "Song"}}, {{"song.ogg", audio}, {notes.name, notes.bytes}});
    const std::vector<SngFileEntry> table = sng_read_file_table(buf);
    REQUIRE(table.size() == 2);
    const SngFileEntry& audio_entry = table[0];
    const SngFileEntry& notes_entry = table[1];
    // make_sng puts the audio right after the file table, so this table fits
    // in the header's first piece, and the audio runs far past that piece.
    REQUIRE(audio_entry.offset <= kFirstPieceRead);
    REQUIRE(audio_entry.offset + audio_entry.length > kFirstPieceRead);
    const std::string path = sng_fixture_path("big_audio.sng");
    write_bytes(path, buf);

    uint64_t bytes_read = 0;
    const Song via_reads = load_counting(path, bytes_read);
    CHECK(songs_equal(via_reads, notes_song(notes)));
    // The header's first piece, then the notes entry: no other byte of the
    // audio is read.
    CHECK(bytes_read == kFirstPieceRead + notes_entry.length);
}

TEST_CASE("sng: a header longer than the first read still loads") {
    const CorpusNotes notes = corpus_notes();
    // A metadata value as long as the first piece puts the file table past it.
    const std::string big(kFirstPieceRead, 'x');
    const std::vector<uint8_t> buf = make_sng(
        {{"name", "Song"}, {"loading_phrase", big}},
        {{"song.ogg", {1, 2, 3}}, {"album.png", {4, 5}}, {notes.name, notes.bytes}});
    const std::string path = sng_fixture_path("big_header.sng");
    write_bytes(path, buf);
    const std::vector<SngFileEntry> table = sng_read_file_table(buf);
    REQUIRE(table.size() == 3);

    uint64_t bytes_read = 0;
    CHECK(songs_equal(load_counting(path, bytes_read), notes_song(notes)));
    // One first piece and the notes entry would come to exactly
    // kFirstPieceRead plus the notes' length. The first piece came back whole
    // but too short for the table, so the reader asked again for more.
    CHECK(bytes_read > kFirstPieceRead + table[2].length);
}

// A file can shrink after it was opened and sized: the source then claims
// more bytes than its reads hand back. A short read ends the header read,
// because waiting for the claimed size would never finish.
TEST_CASE("sng: a file that shrinks after it was opened ends the header read") {
    const CorpusNotes notes = corpus_notes();
    const std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {{notes.name, notes.bytes}});
    const std::string path = sng_fixture_path("shrunk.sng");
    write_bytes(path, whole);
    const ByteSource opened = file_byte_source(path);

    // The file on disk is cut before its metadata length while the source
    // still holds it open.
    std::filesystem::resize_file(os_path(path), kSngMetadataLenOffset);
    const std::vector<uint8_t> cut(whole.begin(),
                                   whole.begin() + static_cast<std::ptrdiff_t>(kSngMetadataLenOffset));
    CHECK(sng_read_head(opened) == cut);

    // What is left holds no file table, so the load fails the way a container
    // with no notes file does.
    const std::optional<LoadFailure> failed =
        load_failure([&] { load_songpath_reading(opened, path, ChartReading{}); });
    REQUIRE(failed.has_value());
    CHECK(failed->kind == ErrorKind::ChartUnreadable);
    CHECK(failed->line == no_notes_failure().line);
}

TEST_CASE("sng: ranged reads fail a damaged container the way a whole read does") {
    const CorpusNotes notes = corpus_notes();
    const std::vector<uint8_t> whole = make_sng({{"name", "Song"}}, {{notes.name, notes.bytes}});
    const std::vector<SngFileEntry> table = sng_read_file_table(whole);
    REQUIRE(table.size() == 1);
    const LoadFailure no_notes = no_notes_failure();

    // The notes entry runs past the end of the file: the table still names
    // it, but it cannot be read, which is a different failure from no notes.
    const std::vector<uint8_t> cut_notes(whole.begin(), whole.end() - 5);
    const std::vector<SngFileEntry> cut_notes_table = sng_read_file_table(cut_notes);
    REQUIRE(cut_notes_table.size() == 1);
    std::vector<uint8_t> cut_out;
    CHECK_FALSE(sng_decode_file_into(cut_notes, cut_notes_table[0], cut_out));
    CHECK(damaged_failure("cut_notes.sng", cut_notes).line != no_notes.line);

    // The file table is cut (make_sng puts the notes right after it): no
    // notes entry is found.
    const std::vector<uint8_t> cut_table(
        whole.begin(), whole.begin() + static_cast<std::ptrdiff_t>(table[0].offset - 1));
    CHECK(sng_read_file_table(cut_table).empty());
    CHECK(damaged_failure("cut_table.sng", cut_table).line == no_notes.line);

    // A metadata length as long as the whole file cannot fit after the header.
    const std::vector<uint8_t> huge_meta = with_u64_at(whole, kSngMetadataLenOffset, whole.size());
    CHECK(sng_read_file_table(huge_meta).empty());
    CHECK(damaged_failure("huge_meta.sng", huge_meta).line == no_notes.line);
}

TEST_CASE("sng: a container parses the same from bytes as from its path") {
    const CorpusNotes notes = corpus_notes();
    const std::vector<uint8_t> buf =
        make_sng({{"name", "Song"}}, {{"song.ogg", {1, 2}}, {notes.name, notes.bytes}});
    const std::string path = sng_fixture_path("from_bytes.sng");
    write_bytes(path, buf);
    const Song want = notes_song(notes);
    REQUIRE_FALSE(want.is_empty());
    CHECK(songs_equal(load_songpath_from_bytes(path, buf, ChartReading{}), want));
    CHECK(songs_equal(load_songpath(path, ChartReading{}), want));

    // The extension still decides the format: a path whose extension names no
    // chart format fails as an unreadable chart, naming that path.
    const std::string unknown = "x.txt";
    REQUIRE(chart_format_of(unknown) == ChartFormat::None);
    const std::optional<LoadFailure> failed =
        load_failure([&] { load_songpath_from_bytes(unknown, buf, ChartReading{}); });
    REQUIRE(failed.has_value());
    CHECK(failed->kind == ErrorKind::ChartUnreadable);
    CHECK(failed->what.find(unknown) != std::string::npos);
}

// Clone Hero, its song cache and dmleaderboards key a .sng by the MD5 of the
// notes file inside it, as they key a folder chart by its notes file's MD5.
// The audio and art around the notes never change the id.
TEST_CASE("sng: a chart's id is the MD5 of the notes file inside it") {
    const CorpusNotes notes = corpus_notes();
    const std::string id = cng_md5_hex(notes.bytes);
    const std::string title = "Song";
    const std::string dir = testtemp::temp_dir("sng_scan_id");
    const std::string with_ogg = dir + "\\a.sng";
    write_bytes(with_ogg, make_sng({{"name", title}, {"artist", "Band"}},
                                   {{"song.ogg", {1, 2}}, {notes.name, notes.bytes}}));
    const std::string other_audio = dir + "\\b.sng";
    write_bytes(other_audio, make_sng({{"name", title}},
                                      {{notes.name, notes.bytes}, {"drums.ogg", {9, 9, 9}}}));

    // The folder chart's own id is the same MD5.
    CHECK(app::hash_chart_file(notes.path) == id);
    CHECK(app::hash_chart_file(with_ogg) == id);
    CHECK(app::hash_chart_file(other_audio) == id);

    // The scan gives the same id, and still reads the names.
    auto [items, errors] = app::discover_charts({dir});
    CHECK(errors.empty());
    REQUIRE(items.size() == 2);
    for (const app::ScanItem& item : items) {
        CAPTURE(item.notespath);
        CHECK(item.md5 == id);
        CHECK(item.title == title);
    }
}

// A name field of only spaces is blank whatever the file format: the scan
// stores the same placeholder a blank song.ini field gets, and padding around
// a name is gone from the stored text.
TEST_CASE("sng: the scan trims each name field before the unknown-name fallback") {
    const std::string dir = testtemp::temp_dir("sng_scan_trim");
    write_bytes(dir + "\\padded.sng",
                make_sng({{"name", "  Song  "}, {"artist", "\tBand "}, {"charter", " "}},
                         {{"notes.mid", tiny_mid()}}));

    auto [items, errors] = app::discover_charts({dir});
    CHECK(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].title == "Song");
    CHECK(items[0].artist == "Band");
    CHECK(items[0].charter == charter_or_unknown(""));
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
    // The scan's problem line is the line for the loader's own failure on
    // this file: the failure of a container with no notes file.
    const std::optional<LoadFailure> failed =
        load_failure([&] { load_songpath(path, ChartReading{}); });
    REQUIRE(failed.has_value());
    CHECK(failed->kind == ErrorKind::ChartUnreadable);
    CHECK(failed->line == no_notes_failure().line);
    CHECK(errors[0] == failed->line);
    CHECK(app::hash_chart_file(path).empty());
}

// More chart_files cases are in test_song_parse.cpp.
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
