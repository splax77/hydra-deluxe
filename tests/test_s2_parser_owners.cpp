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
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/preview_source.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "miniz.h"
#include "parse/song.h"
#include "store/record_store.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// A .chart at 192 ticks a beat, 4/4 and 120 BPM, with `song_extra` lines in
// [Song] and `drums` lines in [ExpertDrums].
std::vector<uint8_t> chart_bytes(const std::string& song_extra, const std::string& drums) {
    const std::string s = "[Song]\n{\n  Resolution = 192\n" + song_extra + "}\n" +
                          "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n" +
                          "[ExpertDrums]\n{\n" + drums + "}\n";
    return std::vector<uint8_t>(s.begin(), s.end());
}

// A format-1 MIDI file, one MTrk chunk per track, 480 ticks a beat.
std::vector<uint8_t> smf_tracks(const std::vector<std::vector<uint8_t>>& tracks) {
    std::vector<uint8_t> d = {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1,
                              0, static_cast<uint8_t>(tracks.size()), 0x01, 0xE0};
    for (const std::vector<uint8_t>& t : tracks) {
        d.insert(d.end(), {'M', 'T', 'r', 'k'});
        const uint32_t n = static_cast<uint32_t>(t.size());
        for (int shift : {24, 16, 8, 0}) d.push_back(static_cast<uint8_t>(n >> shift));
        d.insert(d.end(), t.begin(), t.end());
    }
    return d;
}

// A text meta event `vlq` ticks after the previous event (the delta already
// written as MIDI variable-length bytes).
std::vector<uint8_t> text_after(std::vector<uint8_t> vlq, const std::string& text) {
    vlq.insert(vlq.end(), {0xFF, 0x01, static_cast<uint8_t>(text.size())});
    vlq.insert(vlq.end(), text.begin(), text.end());
    return vlq;
}

// Raw deflate, no zlib header: what .srb streams use (as in test_srb.cpp).
std::vector<uint8_t> deflate_raw(const std::vector<uint8_t>& src) {
    size_t out_len = 0;
    void* p = tdefl_compress_mem_to_heap(src.data(), src.size(), &out_len,
                                         TDEFL_DEFAULT_MAX_PROBES);
    REQUIRE(p != nullptr);
    std::vector<uint8_t> out(static_cast<uint8_t*>(p), static_cast<uint8_t*>(p) + out_len);
    mz_free(p);
    return out;
}

void push_str(std::vector<uint8_t>& out, const std::string& s) {
    const uint32_t n = static_cast<uint32_t>(s.size());
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(n >> (8 * i)));
    out.insert(out.end(), s.begin(), s.end());
}

// An .srb whose metadata names its notes stream `notes_name` (layout in
// parse/srb.h): 16 header bytes, the deflated metadata, the deflated notes,
// then one stand-in audio stream.
std::vector<uint8_t> srb_with(const std::string& notes_name, const std::vector<uint8_t>& notes) {
    std::vector<uint8_t> meta = {'4', 'b', '4', 1};
    push_str(meta, notes_name);
    for (const char* s : {"Name", "Artist", "Album", "Genre", "Charter", "2026", "desc"})
        push_str(meta, s);
    for (int i = 0; i < 24; ++i) meta.push_back(static_cast<uint8_t>(i * 7));
    std::vector<uint8_t> out;
    for (int i = 0; i < 12; ++i) out.push_back(static_cast<uint8_t>(0xA0 + i));
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(i == 0 ? 17 : 0));
    for (const std::vector<uint8_t>& stream :
         {deflate_raw(meta), deflate_raw(notes), deflate_raw(std::vector<uint8_t>(4096, 0x55))})
        out.insert(out.end(), stream.begin(), stream.end());
    return out;
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
