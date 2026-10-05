// Tests for parse/srb — the Clone Hero bundled-song (.srb) container.
//
// No .srb ships in the corpus (the real ones are Clone Hero's copyrighted
// bundles), so these tests fabricate containers with miniz's compressor:
// 16 header bytes + a deflated metadata block + a deflated notes payload,
// wrapping real corpus charts. A wrapped chart must parse identically to the
// loose file, and discovery must surface the embedded metadata.

#define _CRT_SECURE_NO_WARNINGS  // _wfopen; matches parse/song.cpp's file open.

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
#include "core/winstr.h"
#include "corpus_util.h"
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

void write_bytes(const std::string& path, const std::vector<uint8_t>& data) {
    FILE* f = hydra::fopen_utf8(path, L"wb");
    REQUIRE_MESSAGE(f != nullptr, "cannot write " << path);
    if (!data.empty()) std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

// This process's scratch folder for the fixtures (testtemp::temp_dir).
std::string fixture_dir() { return testtemp::temp_dir("srb"); }

using testsrb::make_metadata;
using testsrb::make_srb;
using testsong::songs_equal;

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

        Song direct = load_songpath(src, true, true);
        Song via_srb = load_songpath(path, true, true);
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

    Song direct = load_songpath(src, true, true);
    Song via_srb = load_songpath_srb(path, true, true);
    CHECK(songs_equal(direct, via_srb));
}

TEST_CASE("srb: malformed containers throw instead of crashing") {
    std::string tiny = fixture_dir() + "\\tiny.srb";
    write_bytes(tiny, {1, 2, 3});
    CHECK_THROWS_AS(load_songpath_srb(tiny, true, true),
                    std::runtime_error);

    std::string garbage = fixture_dir() + "\\garbage.srb";
    std::vector<uint8_t> junk(64);
    for (size_t i = 0; i < junk.size(); ++i)
        junk[i] = static_cast<uint8_t>(i * 37 + 11);
    write_bytes(garbage, junk);
    CHECK_THROWS_AS(load_songpath_srb(garbage, true, true),
                    std::runtime_error);

    // Metadata stream present but the notes stream is cut off mid-way.
    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".mid"));
    std::vector<uint8_t> whole =
        make_srb(make_metadata("notes.mid", "N", "A", "C"), notes, {});
    whole.resize(whole.size() / 2);
    std::string truncated = fixture_dir() + "\\truncated.srb";
    write_bytes(truncated, whole);
    CHECK_THROWS_AS(load_songpath_srb(truncated, true, true),
                    std::runtime_error);
}

TEST_CASE("srb: a truncated file reads as an unreadable chart") {
    std::vector<uint8_t> whole = make_srb(make_metadata("notes.mid", "N", "A", "C"),
                                          read_bytes(corpus_chart_path(".mid")), {});
    whole.resize(whole.size() / 2);
    try {
        load_songbytes_srb(whole, true, true);
        FAIL("a truncated .srb loaded");
    } catch (const std::exception& e) {
        CHECK(app::plain_error(e) ==
              "Hydra couldn't read this chart file. It may be damaged or in a format Hydra "
              "doesn't support; try downloading the song again.");
    }
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
        testbytes::counting(file_byte_source(path), bytes_read), path, true, true);
    CHECK(songs_equal(via_reads, load_songbytes_srb(srb, true, true)));
    CHECK(bytes_read < (2u << 20));
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

// D75: the field after the string table that states the song's length. The
// reader hands the raw number on; app::stated_length_ms decides what counts.
TEST_CASE("srb: metadata parser reads song_length_ms after the string table") {
    // Biology.srb stores 196,905 (the user's .srb format reference).
    SrbMetadata md;
    REQUIRE(srb_parse_metadata(testsrb::make_metadata_with_length("notes.chart", 196905), md));
    CHECK(md.name == "Name");
    REQUIRE(md.song_length_ms.has_value());
    CHECK(*md.song_length_ms == 196905);

    // Two shipped files store 0 (unknown). The raw 0 comes back as it is.
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

TEST_CASE("srb: discovery surfaces the embedded metadata") {
    // A dedicated folder so the corpus snapshot tests are unaffected.
    std::string dir = fixture_dir() + "\\scan";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".chart"));
    std::vector<uint8_t> srb = make_srb(
        make_metadata("notes.chart", "Scanned Song", "Scanned Artist",
                      "Scanned Charter"),
        notes);
    write_bytes(dir + "\\bundle.srb", srb);

    auto [items, errors] = hydra::app::discover_charts({dir});
    REQUIRE(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].title == "Scanned Song");
    CHECK(items[0].artist == "Scanned Artist");
    CHECK(items[0].charter == "Scanned Charter");
    CHECK(items[0].md5.size() == 32);
    CHECK(!items[0].sig.empty());
}

TEST_CASE("srb: an empty embedded name reads (unknown)") {
    std::string dir = fixture_dir() + "\\scan_blank";
    CreateDirectoryW(utf8_to_wide(dir).c_str(), nullptr);

    std::vector<uint8_t> notes = read_bytes(corpus_chart_path(".chart"));
    write_bytes(dir + "\\blank.srb",
                make_srb(make_metadata("notes.chart", "", "Scanned Artist",
                                       "Scanned Charter"),
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
    CHECK(songs_equal(load_songbytes_mid(notebytes, true, true), load_songpath(src, true, true)));

    // Only a header, or junk where the metadata should be: it throws.
    const std::vector<uint8_t> header_only(srb.begin(), srb.begin() + kSrbHeaderSize);
    CHECK_THROWS_AS(srb_read_metadata(memory_byte_source(header_only)), std::runtime_error);
    std::vector<uint8_t> junk = srb;
    for (size_t i = kSrbHeaderSize; i < kSrbHeaderSize + 64; ++i) junk[i] = 0xFF;
    CHECK_THROWS_AS(srb_read_metadata(memory_byte_source(junk)), std::runtime_error);
}
