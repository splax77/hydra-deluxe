// Tests for parse/srb — the Clone Hero bundled-song (.srb) container.
//
// No .srb ships in the corpus (the real ones are Clone Hero's copyrighted
// bundles), so these tests fabricate containers with miniz's compressor:
// 16 header bytes + a deflated metadata block + a deflated notes payload,
// wrapping real corpus charts. A wrapped chart must parse identically to the
// loose file, and discovery must surface the embedded metadata.

#define _CRT_SECURE_NO_WARNINGS  // Turns off MSVC's C runtime warnings in this file.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/user_messages.h"
#include "byte_source_util.h"
#include "core/bounds.h"
#include "core/error_kind.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "parse/srb.h"
#include "song_equal.h"
#include "srb_util.h"
#include "temp_util.h"

using namespace hydra;

namespace {

std::vector<uint8_t> read_bytes(const std::string& path) {
    return hydra::read_file_bytes(path);
}

using testtemp::write_bytes;

// This process's scratch folder for the fixtures (testtemp::temp_dir).
std::string fixture_dir() { return testtemp::temp_dir("srb"); }

using testsrb::make_full_metadata;
using testsrb::make_metadata;
using testsrb::make_srb;
using testsong::songs_equal;

// No Known Suspects.srb's stored checksum, which Clone Hero's songcache.bin
// and dmleaderboards use as its id. Only these 16 bytes are taken from the
// real file; the containers around them are synthetic.
constexpr testsrb::Checksum kNoKnownSuspectsId = {0xc8, 0x7b, 0x09, 0xd0, 0xe0, 0xa8,
                                                  0x99, 0x9a, 0x73, 0x79, 0xfc, 0x6e,
                                                  0x8e, 0x86, 0xfd, 0x72};
constexpr const char* kNoKnownSuspectsHex = "c87b09d0e0a8999a7379fc6e8e86fd72";

// First corpus chart with the given extension.
std::string corpus_chart_path(const std::string& ext) {
    return corpus::first_chart_with_suffix(ext);
}

}  // namespace

TEST_CASE("srb: a wrapped chart parses identically to the loose file") {
    struct Case {
        const char* ext;
        const char* notes_filename;
    };
    for (Case c : {Case{".mid", "notes.mid"}, Case{".chart", "notes.chart"}}) {
        CAPTURE(c.ext);
        std::string src = corpus_chart_path(c.ext);
        std::vector<uint8_t> notes = read_bytes(src);

        std::vector<uint8_t> srb = make_srb(
            make_metadata(c.notes_filename, "Name", "Artist", "Charter"), notes);
        std::string path = fixture_dir() + "\\wrapped" + c.ext + ".srb";
        write_bytes(path, srb);

        Song direct = load_songpath(src, ChartReading{});
        Song via_srb = load_songpath(path, ChartReading{});
        CHECK_MESSAGE(songs_equal(direct, via_srb), src);
    }
}

TEST_CASE("srb: an unexpected notes filename falls back to payload sniffing") {
    std::string src = corpus_chart_path(".mid");
    std::vector<uint8_t> notes = read_bytes(src);
    std::vector<uint8_t> srb =
        make_srb(make_metadata("weird.bin", "N", "A", "C"), notes);
    std::string path = fixture_dir() + "\\sniffed.srb";
    write_bytes(path, srb);

    Song direct = load_songpath(src, ChartReading{});
    Song via_srb = load_songpath_srb(path, ChartReading{});
    CHECK(songs_equal(direct, via_srb));
}

TEST_CASE("srb: malformed containers throw instead of crashing") {
    std::string tiny = fixture_dir() + "\\tiny.srb";
    write_bytes(tiny, {1, 2, 3});
    CHECK_THROWS_WITH_AS(load_songpath_srb(tiny, ChartReading{}), "Truncated SRB file.",
                         std::runtime_error);

    std::string garbage = fixture_dir() + "\\garbage.srb";
    std::vector<uint8_t> junk(64);
    for (size_t i = 0; i < junk.size(); ++i)
        junk[i] = static_cast<uint8_t>(i * 37 + 11);
    write_bytes(garbage, junk);
    CHECK_THROWS_WITH_AS(load_songpath_srb(garbage, ChartReading{}), "SRB stream starts past end of file.",
                         std::runtime_error);

    // Metadata stream present but the notes stream is cut off mid-way.
    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".mid"));
    std::vector<uint8_t> whole =
        make_srb(make_metadata("notes.mid", "N", "A", "C"), notes, {});
    whole.resize(whole.size() / 2);
    std::string truncated = fixture_dir() + "\\truncated.srb";
    write_bytes(truncated, whole);
    CHECK_THROWS_WITH_AS(load_songpath_srb(truncated, ChartReading{}), "SRB stream is truncated.",
                         std::runtime_error);
}

namespace {

// Loads `srb` and checks it fails the way any damaged chart does: an error of
// kind ChartUnreadable, so the player reads plain_error's own sentence for
// that kind, never the reader's raw text. Returns the raw text, for a check
// of which failure fired.
std::string unreadable_chart_text(const std::vector<uint8_t>& srb) {
    try {
        load_songbytes_srb(srb, ChartReading{});
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::ChartUnreadable);
        CHECK(app::plain_error(e) ==
              app::plain_error(KindedError(ErrorKind::ChartUnreadable, "")));
        return e.what();
    }
    FAIL("the .srb loaded");
    return {};
}

}  // namespace

TEST_CASE("srb: a truncated file reads as an unreadable chart") {
    std::vector<uint8_t> whole = make_srb(make_metadata("notes.mid", "N", "A", "C"),
                                          read_bytes(corpus_chart_path(".mid")), {});
    whole.resize(whole.size() / 2);
    unreadable_chart_text(whole);
}

// The fuzzer's testdata/fuzz/srb/endless-zeros.srb. Its notes stream's
// Huffman table leaves codes unassigned, and miniz decodes an unassigned code
// as the literal 0 while reading no bits, so the stream makes zero bytes
// without end. Running on to the 1 GB cap (kSrbMaxStream) would take seconds
// in a release build and past 30 s in the fuzz build, so the inflater calls it
// corrupt as soon as the output passes what the bytes read could make
// (kMaxBytesPerInputByte in parse/srb.cpp owns that limit).
TEST_CASE("srb: a stream that makes bytes from no input is corrupt, not inflated to the cap") {
    const std::vector<uint8_t> file =
        read_bytes(corpus::testdata_root() + "/fuzz/srb/endless-zeros.srb");
    // "corrupt", the guard's words, not "exceeds size limit", the cap's.
    CHECK(unreadable_chart_text(file) == "SRB stream is corrupt.");
}

// Finding 61; D27.
TEST_CASE("srb: the notes entry is found by its exact name, never by its extension") {
    using namespace testmidi;
    const std::vector<uint8_t> mid =
        smf_drums(concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}));
    // "song.chart" is not a notes name. The name decides nothing, so the
    // stream's bytes do, and it loads as MIDI.
    CHECK(load_songbytes_srb(make_srb(make_metadata("song.chart"), mid), ChartReading{})
              .sequence.size() == 1);
    // The exact names decide, in any case.
    CHECK(load_songbytes_srb(make_srb(make_metadata("NOTES.MID"), mid), ChartReading{})
              .sequence.size() == 1);
}

namespace {

// Bytes that deflate barely at all, so their compressed stream is as long as
// they are.
std::vector<uint8_t> noise(size_t n, uint32_t seed) {
    std::vector<uint8_t> out(n);
    for (size_t i = 0; i < n; ++i) {
        seed = seed * 1664525u + 1013904223u;
        out[i] = static_cast<uint8_t>(seed >> 24);
    }
    return out;
}

// The most a reader that follows the piece rule (kFirstPieceRead, then
// next_piece_read, in core/winstr.h) takes in to inflate a stream of `n`
// compressed bytes: whole pieces until they cover the stream.
uint64_t piece_reads_covering(uint64_t n) {
    uint64_t total = 0;
    size_t ask = kFirstPieceRead;
    while (total < n) {
        total += ask;
        ask = next_piece_read(ask);
    }
    return total;
}

}  // namespace

// The real bundles are 12-57 MB, nearly all of it audio and art past a notes
// stream of a few hundred KB.
TEST_CASE("srb: the note loader reads the metadata and notes streams, not the rest") {
    const std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".mid"));
    const std::vector<uint8_t> srb =
        make_srb(make_metadata("notes.mid", "N", "A", "C"), notes, {noise(16 << 20, 1)});
    const std::string path = fixture_dir() + "\\big_tail.srb";
    write_bytes(path, srb);

    uint64_t bytes_read = 0;
    const Song via_reads = load_songpath_reading(
        testbytes::counting(file_byte_source(path), bytes_read), path, ChartReading{});
    CHECK(songs_equal(via_reads, load_songbytes_srb(srb, ChartReading{})));

    // Where the two streams the loader needs lie in the file, by the readers'
    // own end offsets.
    const SrbMetadataRead meta = srb_read_metadata(memory_byte_source(srb));
    uint64_t notes_end = 0;
    srb_inflate_stream_reading(memory_byte_source(srb), meta.notes_offset, kSrbMaxStream,
                               &notes_end);
    const uint64_t allowed = piece_reads_covering(meta.notes_offset - kSrbHeaderSize) +
                             piece_reads_covering(notes_end - meta.notes_offset);
    // The tail is big enough that a load reading the whole file breaks the
    // bound.
    REQUIRE(allowed < srb.size());
    CHECK(bytes_read <= allowed);
}

TEST_CASE("srb: a stream inflates the same from ranged reads as from the whole buffer") {
    // A 3 MB stream crosses many reads; the stream after it must not be eaten.
    const std::vector<uint8_t> payload = noise(3 << 20, 7);
    std::vector<uint8_t> buf(kSrbHeaderSize, 0xAB);
    const std::vector<uint8_t> d = testsrb::deflate_raw(payload);
    buf.insert(buf.end(), d.begin(), d.end());
    const std::vector<uint8_t> tail = testsrb::deflate_raw({1, 2, 3});
    buf.insert(buf.end(), tail.begin(), tail.end());

    size_t end_whole = 0;
    const std::vector<uint8_t> whole =
        srb_inflate_stream(buf.data(), buf.size(), kSrbHeaderSize, kSrbMaxStream, &end_whole);
    uint64_t end_reads = 0;
    const std::vector<uint8_t> reads = srb_inflate_stream_reading(
        memory_byte_source(buf), kSrbHeaderSize, kSrbMaxStream, &end_reads);
    CHECK(whole == payload);
    CHECK(reads == payload);
    CHECK(end_reads == end_whole);
    CHECK(end_whole == kSrbHeaderSize + d.size());

    // The same failures, by the same words.
    std::vector<uint8_t> cut(buf.begin(), buf.begin() + buf.size() / 2);
    CHECK_THROWS_WITH(srb_inflate_stream_reading(memory_byte_source(cut), kSrbHeaderSize,
                                                 kSrbMaxStream, nullptr),
                      "SRB stream is truncated.");
    CHECK_THROWS_WITH(srb_inflate_stream_reading(memory_byte_source(buf), buf.size(),
                                                 kSrbMaxStream, nullptr),
                      "SRB stream starts past end of file.");
    CHECK_THROWS_WITH(srb_inflate_stream_reading(memory_byte_source(buf), kSrbHeaderSize,
                                                 1 << 20, nullptr),
                      "SRB stream exceeds size limit.");
    // A short read ends the source, even when later reads would be whole.
    CHECK_THROWS_WITH(srb_inflate_stream_reading(
                          testbytes::short_first_read(memory_byte_source(buf), 32768),
                          kSrbHeaderSize, kSrbMaxStream, nullptr),
                      "SRB stream is truncated.");
    std::vector<uint8_t> junk = buf;
    for (size_t i = kSrbHeaderSize; i < kSrbHeaderSize + 64; ++i) junk[i] = 0xFF;
    CHECK_THROWS_WITH(srb_inflate_stream_reading(memory_byte_source(junk), kSrbHeaderSize,
                                                 kSrbMaxStream, nullptr),
                      "SRB stream is corrupt.");
}

TEST_CASE("srb: metadata parser reads the string table") {
    std::vector<uint8_t> meta =
        make_metadata("notes.chart", "Song Name", "The Artist", "The Charter");
    SrbMetadata md;
    REQUIRE(srb_parse_metadata(meta, md));
    CHECK(md.notes_filename == "notes.chart");
    CHECK(md.name == "Song Name");
    CHECK(md.artist == "The Artist");
    CHECK(md.album == "Test Album");
    CHECK(md.genre == "Test Genre");
    CHECK(md.charter == "The Charter");
    CHECK(md.year == "2026");

    // Truncated mid-table: earlier fields survive, later ones stay empty.
    std::vector<uint8_t> cut(meta.begin(), meta.begin() + 4 + 4 + 11 + 4 + 9 / 2);
    SrbMetadata partial;
    REQUIRE(srb_parse_metadata(cut, partial));
    CHECK(partial.notes_filename == "notes.chart");
    CHECK(partial.name.empty());

    std::vector<uint8_t> too_short = {'4', 'b'};
    SrbMetadata none;
    CHECK_FALSE(srb_parse_metadata(too_short, none));
}

// core::fits, the bounds test the metadata reader (and the .sng reader and the
// Preview's .srb audio walk) asks before each read.
TEST_CASE("srb: core::fits says whether n bytes at pos lie inside the buffer") {
    CHECK(core::fits(10, 10, 0));    // nothing at the very end fits
    CHECK(core::fits(10, 6, 4));     // the last four bytes
    CHECK_FALSE(core::fits(10, 7, 4));
    CHECK_FALSE(core::fits(10, 10, 1));
    CHECK_FALSE(core::fits(10, 11, 0));  // pos past the end never fits
    CHECK_FALSE(core::fits(10, 2, UINT64_MAX));  // a huge n never wraps to a yes
    const std::vector<uint8_t> buf(16);
    CHECK(core::fits(buf, 16, 0));
    CHECK_FALSE(core::fits(buf, 16, 1));
}

// D75: the field after the string table that states the song's length. The
// reader hands the raw number on; app::stated_length_ms decides what counts.
TEST_CASE("srb: metadata parser reads song_length_ms after the string table") {
    // Biology.srb stores 196,905 (docs/srb-format.md).
    SrbMetadata md;
    REQUIRE(srb_parse_metadata(testsrb::make_metadata_with_length("notes.chart", 196905), md));
    CHECK(md.name == "Name");
    REQUIRE(md.song_length_ms.has_value());
    CHECK(*md.song_length_ms == 196905);

    // A stored 0 means unknown (SrbMetadata::song_length_ms in parse/srb.h
    // says how many shipped files have one). The raw 0 comes back as it is.
    SrbMetadata zero;
    REQUIRE(srb_parse_metadata(testsrb::make_metadata_with_length("notes.chart", 0, ""), zero));
    CHECK(zero.song_length_ms == 0);

    // A block that ends before the field has none.
    const std::vector<uint8_t> full = testsrb::make_metadata_with_length("notes.chart", 196905);
    const std::vector<uint8_t> cut(full.begin(), full.end() - 16 - 2);
    SrbMetadata partial;
    REQUIRE(srb_parse_metadata(cut, partial));
    CHECK_FALSE(partial.song_length_ms.has_value());

    // make_metadata's junk tail spells an icon length past the block's end.
    SrbMetadata junk;
    REQUIRE(srb_parse_metadata(make_metadata("notes.chart"), junk));
    CHECK_FALSE(junk.song_length_ms.has_value());
}

TEST_CASE("srb: metadata parser reads the checksum after song_length_ms") {
    const std::vector<uint8_t> full =
        make_full_metadata("notes.chart", "N", "A", "C", 196905, kNoKnownSuspectsId);
    SrbMetadata md;
    REQUIRE(srb_parse_metadata(full, md));
    REQUIRE(md.checksum.has_value());
    CHECK(*md.checksum == kNoKnownSuspectsId);

    // One byte short: the length is read, the checksum is not.
    const std::vector<uint8_t> cut(full.begin(), full.end() - 1);
    SrbMetadata partial;
    REQUIRE(srb_parse_metadata(cut, partial));
    CHECK(partial.song_length_ms == 196905);
    CHECK_FALSE(partial.checksum.has_value());
}

TEST_CASE("srb: discovery surfaces the embedded metadata") {
    // A dedicated folder so the corpus snapshot tests are unaffected.
    std::string dir = fixture_dir() + "\\scan";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".chart"));
    std::vector<uint8_t> srb = make_srb(
        make_full_metadata("notes.chart", "Scanned Song", "Scanned Artist",
                           "Scanned Charter", 196905, kNoKnownSuspectsId),
        notes);
    write_bytes(dir + "\\bundle.srb", srb);

    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].title == "Scanned Song");
    CHECK(items[0].artist == "Scanned Artist");
    CHECK(items[0].charter == "Scanned Charter");
    CHECK(items[0].md5 == kNoKnownSuspectsHex);
    CHECK(!items[0].sig.empty());
}

// Clone Hero, its song cache and dmleaderboards key an .srb by the checksum
// its metadata stores. In 4 of the 30 shipped files that is not the MD5 of
// the notes stream (No Known Suspects stores c87b09d0...; CR-11 on
// docs/clone-hero/01-reading-a-chart.md names the four and how they were
// checked), so the id is read, never recomputed: not the notes' MD5 and not
// the whole container's.
TEST_CASE("srb: a chart's id is the checksum its metadata stores") {
    std::string dir = fixture_dir() + "\\scan_id";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    const std::string loose = corpus_chart_path(".chart");
    const std::string path = dir + "\\bundle.srb";
    write_bytes(path, make_srb(make_full_metadata("notes.chart", "N", "A", "C", 196905,
                                                  kNoKnownSuspectsId),
                               read_bytes(loose)));

    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].md5 == kNoKnownSuspectsHex);
    // The tools' and the click's lookup by path reads the same id.
    CHECK(hydra::app::hash_chart_file(path) == kNoKnownSuspectsHex);
    // The loose notes file hashes to its own MD5, a different id.
    CHECK(hydra::app::hash_chart_file(loose) != kNoKnownSuspectsHex);
}

// A block that ends before its checksum holds no id: the scan lists the file
// as an error instead of a chart, and a lookup by path finds nothing.
TEST_CASE("srb: a metadata block without its checksum is a scan error") {
    std::string dir = fixture_dir() + "\\scan_no_id";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    const std::string path = dir + "\\bundle.srb";
    std::vector<uint8_t> meta = testsrb::make_metadata_with_length("notes.chart", 196905);
    meta.resize(meta.size() - 1);  // one byte short of the checksum
    write_bytes(path, make_srb(meta, read_bytes(corpus_chart_path(".chart"))));

    auto [items, errors] = hydra::app::discover_charts({dir});
    CHECK(items.empty());
    REQUIRE(errors.size() == 1);
    CHECK(errors[0].find("checksum") != std::string::npos);
    CHECK(hydra::app::hash_chart_file(path).empty());
}

TEST_CASE("srb: an empty embedded name reads (unknown)") {
    std::string dir = fixture_dir() + "\\scan_blank";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".chart"));
    write_bytes(dir + "\\blank.srb",
                make_srb(make_full_metadata("notes.chart", "", "Scanned Artist",
                                            "Scanned Charter", 196905, kNoKnownSuspectsId),
                         notes));

    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].title == kUnknownTitle);
    CHECK(items[0].artist == "Scanned Artist");
}

TEST_CASE("srb: one named cap bounds every inflated stream") {
    // Notes files inflate to well under 100 MB even for mega-charts; the cap
    // exists only to bound hostile input.
    CHECK(kSrbMaxStream == (size_t{1} << 30));
    CHECK(kSrbMaxStream > kSrbMaxMetadata);
}

TEST_CASE("srb: one reader gives the metadata and where the notes stream starts") {
    const std::string src = corpus_chart_path(".mid");
    const std::vector<uint8_t> notes = read_bytes(src);
    const std::vector<uint8_t> meta = make_metadata("notes.mid");
    const std::vector<uint8_t> srb = make_srb(meta, notes);

    const SrbMetadataRead got = srb_read_metadata(memory_byte_source(srb));
    CHECK(got.parsed);
    CHECK(got.fields.notes_filename == "notes.mid");
    CHECK(got.fields.name == "Name");
    CHECK(got.fields.artist == "Artist");
    CHECK(got.fields.charter == "Charter");

    // The notes stream starts right there.
    const std::vector<uint8_t> notebytes =
        srb_inflate_stream_reading(memory_byte_source(srb), got.notes_offset, kSrbMaxStream,
                                   nullptr);
    CHECK(songs_equal(load_songbytes_mid(notebytes, ChartReading{}),
                      load_songpath(src, ChartReading{})));

    // Only a header, or junk where the metadata should be: it throws.
    const std::vector<uint8_t> header_only(srb.begin(), srb.begin() + kSrbHeaderSize);
    CHECK_THROWS_AS(srb_read_metadata(memory_byte_source(header_only)), std::runtime_error);
    std::vector<uint8_t> junk = srb;
    for (size_t i = kSrbHeaderSize; i < kSrbHeaderSize + 64; ++i) junk[i] = 0xFF;
    CHECK_THROWS_AS(srb_read_metadata(memory_byte_source(junk)), std::runtime_error);
}
