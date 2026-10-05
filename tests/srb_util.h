// The one .srb (Clone Hero bundled-song) fixture writer the tests share.
//
// No .srb ships in the corpus (the real ones are Clone Hero's copyrighted
// bundles), so tests fabricate one the way parse/srb.h reads it: 16 header
// bytes, a deflated metadata block, the deflated notes, then any trailing
// streams (real bundles carry audio and art there, which the notes reader
// skips). Audit finding 117: test_srb.cpp, test_preview_source.cpp and
// test_s2_parser_owners.cpp all build theirs through here.

#ifndef HYDRA_TESTS_SRB_UTIL_H
#define HYDRA_TESTS_SRB_UTIL_H

#include <cstdint>
#include <string>
#include <vector>

#include "bytes_util.h"
#include "doctest.h"
#include "miniz.h"

namespace testsrb {

// Raw deflate, no zlib header: what .srb streams use.
inline std::vector<uint8_t> deflate_raw(const std::vector<uint8_t>& src) {
    size_t out_len = 0;
    void* p = tdefl_compress_mem_to_heap(src.data(), src.size(), &out_len,
                                         TDEFL_DEFAULT_MAX_PROBES);
    REQUIRE(p != nullptr);
    std::vector<uint8_t> out(static_cast<uint8_t*>(p), static_cast<uint8_t*>(p) + out_len);
    mz_free(p);
    return out;
}

// A little-endian u32 length (written by bytes_util.h), then the string's
// bytes.
inline void push_str(std::vector<uint8_t>& out, const std::string& s) {
    testbytes::put_u32(out, static_cast<uint32_t>(s.size()));
    out.insert(out.end(), s.begin(), s.end());
}

// The metadata block: a 4-byte tag, the notes entry's file name, then name,
// artist, album, genre, charter, year and description. Real files carry
// trailing binary fields (difficulties, sizes, ...); junk bytes stand in.
inline std::vector<uint8_t> make_metadata(const std::string& notes_filename,
                                          const std::string& name = "Name",
                                          const std::string& artist = "Artist",
                                          const std::string& charter = "Charter") {
    std::vector<uint8_t> meta = {'4', 'b', '4', 1};
    push_str(meta, notes_filename);
    push_str(meta, name);
    push_str(meta, artist);
    push_str(meta, "Test Album");
    push_str(meta, "Test Genre");
    push_str(meta, charter);
    push_str(meta, "2026");
    push_str(meta, "A synthetic bundle for the test suite.");
    for (int i = 0; i < 24; ++i) meta.push_back(static_cast<uint8_t>(i * 7));
    return meta;
}

// A stand-in audio stream for the trailing slot.
inline std::vector<uint8_t> stand_in_audio() { return std::vector<uint8_t>(4096, 0x55); }

// The container: 12 arbitrary bytes and an arbitrary u32, like the real
// header, then the deflated metadata, the deflated notes and one deflated
// stream per `trailing` entry (by default one stand-in audio stream).
inline std::vector<uint8_t> make_srb(
    const std::vector<uint8_t>& metadata, const std::vector<uint8_t>& notes,
    const std::vector<std::vector<uint8_t>>& trailing = {stand_in_audio()}) {
    std::vector<uint8_t> out;
    for (int i = 0; i < 12; ++i) out.push_back(static_cast<uint8_t>(0xA0 + i));
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(i == 0 ? 17 : 0));
    auto put = [&out](const std::vector<uint8_t>& stream) {
        const std::vector<uint8_t> d = deflate_raw(stream);
        out.insert(out.end(), d.begin(), d.end());
    };
    put(metadata);
    put(notes);
    for (const std::vector<uint8_t>& t : trailing) put(t);
    return out;
}

}  // namespace testsrb

#endif  // HYDRA_TESTS_SRB_UTIL_H
