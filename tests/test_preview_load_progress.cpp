// Tests for the Preview load's loading bar and its cancel: the bar's numbers
// (PreviewLoadJob::Progress, ByteRateClock), a cancel in the middle of opening
// a big Opus stem, and a negative chart offset turned into front silence.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/preview_source.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "store/record_store.h"
#include "ui/preview_load_job.h"
#include "ui/widgets.h"  // progress_fraction

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using hydra::ui::ByteRateClock;
using hydra::ui::PreviewLoadJob;
using P = PreviewLoadJob::Progress;
using S = PreviewLoadJob::Step;

namespace {

P make(S step, uint64_t done, uint64_t total, double elapsed = 0.0, double left = -1.0) {
    P p;
    p.step = step;
    p.bytes_done = done;
    p.bytes_total = total;
    p.elapsed_s = elapsed;
    p.time_left_s = left;
    return p;
}

// ---- a scratch chart folder ------------------------------------------------

std::string make_temp_dir(const char* tag) {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    std::wstring dir = std::wstring(tmp) + L"hydra_" + hydra::utf8_to_wide(tag) + L"_" +
                       std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(dir.c_str(), nullptr);
    return hydra::wide_to_utf8(dir);
}

void write_file(const std::string& path, const std::vector<uint8_t>& bytes) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    REQUIRE(f != nullptr);
    if (!bytes.empty()) std::fwrite(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
}

void remove_file(const std::string& path) { DeleteFileW(hydra::utf8_to_wide(path).c_str()); }

hydra::store::ChartLibraryEntry entry_for(const std::string& notespath) {
    hydra::store::ChartLibraryEntry e;
    e.md5 = "loadprog";
    e.title = "Preview load progress test";
    e.notespath = notespath;
    return e;
}

template <class Job>
void wait_finished(const Job& job) {
    for (int i = 0; i < 1200 && !job.finished(); ++i) Sleep(50);
    REQUIRE(job.finished());
}

// ---- a big Ogg Opus file from the test fixture -----------------------------

// The Ogg page checksum (CRC-32, polynomial 0x04c11db7, no reflection).
uint32_t ogg_crc(const uint8_t* d, std::size_t n) {
    static uint32_t table[256];
    static bool ready = false;
    if (!ready) {
        for (uint32_t i = 0; i < 256; ++i) {
            uint32_t r = i << 24;
            for (int k = 0; k < 8; ++k) r = (r & 0x80000000u) ? (r << 1) ^ 0x04c11db7u : (r << 1);
            table[i] = r;
        }
        ready = true;
    }
    uint32_t crc = 0;
    for (std::size_t i = 0; i < n; ++i) crc = (crc << 8) ^ table[((crc >> 24) ^ d[i]) & 0xff];
    return crc;
}

struct OggPage {
    std::size_t offset;
    std::size_t length;
    int64_t granule;
};

std::vector<OggPage> split_pages(const std::vector<uint8_t>& b) {
    std::vector<OggPage> pages;
    std::size_t off = 0;
    while (off + 27 <= b.size() && std::memcmp(&b[off], "OggS", 4) == 0) {
        const int nseg = b[off + 26];
        std::size_t body = 0;
        for (int i = 0; i < nseg; ++i) body += b[off + 27 + i];
        int64_t g = 0;
        std::memcpy(&g, &b[off + 6], 8);
        pages.push_back({off, 27 + static_cast<std::size_t>(nseg) + body, g});
        off += pages.back().length;
    }
    return pages;
}

// Writes an Ogg Opus file of at least `min_bytes`: the fixture's header pages,
// then its audio pages over and over, renumbered and re-timed so the result is
// one valid stream (sequence numbers, granule positions, checksums, EOS only
// on the last page).
void write_big_opus(const std::string& path, uint64_t min_bytes) {
    const std::vector<uint8_t> src =
        hydra::read_file_bytes(std::string(HYDRA_TESTDATA_DIR) + "/audio/sine220.opus");
    const std::vector<OggPage> pages = split_pages(src);
    // Header pages (OpusHead, OpusTags) carry granule 0; audio pages don't.
    std::size_t first_audio = 0;
    while (first_audio < pages.size() && pages[first_audio].granule == 0) ++first_audio;
    REQUIRE(first_audio >= 2);
    REQUIRE(first_audio < pages.size());
    const int64_t span = pages.back().granule;  // samples in one pass of the audio
    std::size_t audio_bytes = 0;
    for (std::size_t i = first_audio; i < pages.size(); ++i) audio_bytes += pages[i].length;
    const uint64_t reps = (min_bytes + audio_bytes - 1) / audio_bytes;

    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    REQUIRE(f != nullptr);
    std::fwrite(src.data(), 1, pages[first_audio].offset, f);
    uint32_t seq = static_cast<uint32_t>(first_audio);
    std::vector<uint8_t> block;
    for (uint64_t r = 0; r < reps; ++r) {
        block.clear();
        for (std::size_t i = first_audio; i < pages.size(); ++i) {
            const std::size_t at = block.size();
            block.insert(block.end(), src.begin() + static_cast<std::ptrdiff_t>(pages[i].offset),
                         src.begin() + static_cast<std::ptrdiff_t>(pages[i].offset + pages[i].length));
            uint8_t* h = &block[at];
            const bool last = r + 1 == reps && i + 1 == pages.size();
            h[5] = static_cast<uint8_t>(last ? (h[5] | 0x04) : (h[5] & ~0x04));
            const int64_t g = pages[i].granule == -1
                                  ? -1
                                  : pages[i].granule + static_cast<int64_t>(r) * span;
            std::memcpy(h + 6, &g, 8);
            std::memcpy(h + 18, &seq, 4);
            ++seq;
            std::memset(h + 22, 0, 4);
            const uint32_t crc = ogg_crc(h, pages[i].length);
            std::memcpy(h + 22, &crc, 4);
        }
        std::fwrite(block.data(), 1, block.size(), f);
    }
    std::fclose(f);
}

}  // namespace

// ---- the bar's numbers -------------------------------------------------------

TEST_CASE("Preview load progress: the bar never moves backwards") {
    const uint64_t total = 625028440;
    const std::vector<P> script = {
        make(S::Reading, 0, 0),
        make(S::Reading, 0, total),  // audio already counting, chart not parsed
        make(S::Opening, 4u << 20, total),
        make(S::Opening, 100000000, total),
        make(S::Opening, 312000000, total),
        make(S::Opening, total, total),
        make(S::Building, total, total),
        make(S::Highway, total, total),
    };
    float prev = -1.0f;
    for (const P& p : script) {
        const float f = p.fraction();
        CAPTURE(p.label());
        CHECK(f >= prev);
        CHECK(f >= 0.0f);
        CHECK(f <= 1.0f);
        prev = f;
    }
    CHECK(script.front().fraction() == 0.0f);
    // Opening audio owns most of the bar: halfway through the bytes is well
    // past a third of it.
    CHECK(make(S::Opening, total / 2, total).fraction() > 0.4f);
}

TEST_CASE("Preview load progress: labels name the step, in real units") {
    CHECK(make(S::Reading, 0, 0).label() == "Reading chart");
    CHECK(make(S::Opening, 312000000, 625028440).label() == "Opening audio: 312 of 625 MB");
    CHECK(make(S::Opening, 0, 37000000).label() == "Opening audio: 0 of 37 MB");
    CHECK(make(S::Opening, 37000000, 37000000).label() == "Opening audio: 37 of 37 MB");
    // Under half a megabyte in all: just the step's name, never "0 of 0 MB".
    CHECK(make(S::Opening, 0, 400000).label() == "Opening audio");
    CHECK(make(S::Opening, 0, 0).label() == "Opening audio");
    CHECK(make(S::Building, 1, 1).label() == "Building scene");
    CHECK(make(S::Highway, 1, 1).label() == "Building highway");
}

TEST_CASE("Preview load progress: a zero total gives a finite fraction") {
    for (S s : {S::Reading, S::Opening, S::Building, S::Highway}) {
        const float f = make(s, 0, 0).fraction();
        CHECK(std::isfinite(f));
        CHECK(f >= 0.0f);
        CHECK(f <= 1.0f);
    }
    // Opening with 0 of 0 bytes: the audio slice reads empty, "nothing
    // reported yet" (D48, Q19), so the bar sits where 0 of any total puts it.
    // The chart's read share before it stays filled.
    CHECK(make(S::Opening, 0, 0).fraction() == make(S::Opening, 0, 625028440).fraction());
    // More done than the total (a file that grew after it was sized) is capped.
    CHECK(make(S::Opening, 200, 100).fraction() == make(S::Building, 1, 1).fraction());
}

TEST_CASE("progress_fraction: 0 of 0 is empty, and the fraction stays between 0 and 1") {
    using hydra::ui::progress_fraction;
    CHECK(progress_fraction(0, 0) == 0.0f);  // nothing reported yet
    CHECK(progress_fraction(1, 2) == 0.5f);
    // More done than the total, as in the zero-total case above: full, no further.
    CHECK(progress_fraction(200, 100) == 1.0f);
}

// The words are the batch strip's, "about m:ss left" (D48, Q20); the
// Preview keeps only its own 3-second wait before it says anything.
TEST_CASE("Preview load progress: time left waits 3 s, then reads m:ss") {
    CHECK(make(S::Opening, 10, 100, 2.9, 40.0).time_left_text() == "");
    CHECK(make(S::Opening, 10, 100, 3.0, -1.0).time_left_text() == "");  // rate unknown
    CHECK(make(S::Opening, 10, 100, 3.0, 40.0).time_left_text() == "about 0:40 left");
    CHECK(make(S::Opening, 10, 100, 5.0, 0.2).time_left_text() == "about 0:00 left");
    CHECK(make(S::Opening, 10, 100, 5.0, 59.0).time_left_text() == "about 0:59 left");
    CHECK(make(S::Opening, 10, 100, 5.0, 61.0).time_left_text() == "about 1:01 left");
    CHECK(make(S::Opening, 10, 100, 5.0, 185.0).time_left_text() == "about 3:05 left");
    // Only Opening audio has a byte rate to go on.
    CHECK(make(S::Building, 100, 100, 5.0, 10.0).time_left_text() == "");
}

TEST_CASE("ByteRateClock measures over a second and answers at most once a second") {
    ByteRateClock c;
    CHECK(c.update(0.0, 0, 1000) < 0.0);  // nothing measured yet
    CHECK(c.update(0.5, 50, 1000) < 0.0);  // under a second measured
    // 100 bytes in 1 s, 900 left: 9 s.
    CHECK(c.update(1.0, 100, 1000) == doctest::Approx(9.0));
    // Within the next second the answer holds, however the bytes move.
    CHECK(c.update(1.5, 600, 1000) == doctest::Approx(9.0));
    // A new second, measured over that second only: 500 bytes/s, 400 left.
    CHECK(c.update(2.0, 600, 1000) == doctest::Approx(0.8));
    // No bytes moved for a second: the rate isn't known any more.
    CHECK(c.update(3.0, 600, 1000) < 0.0);
}

// ---- the job ---------------------------------------------------------------

// Closing the details window joins the load's thread on the UI thread, so a
// cancel in the middle of opening a huge stem must be noticed within the
// Opus index's 4 MB reporting step, not after the whole file.
TEST_CASE("a Preview load cancelled while opening a 300 MB Opus stem stops promptly") {
    const std::string dir = make_temp_dir("loadcancel");
    const std::string notes = dir + "\\notes.chart";
    const std::string song = dir + "\\song.opus";
    write_file(notes, hydra::read_file_bytes(corpus::first_chart_with_suffix(".chart")));
    write_big_opus(song, 300000000ull);
    REQUIRE(hydra::file_size_bytes(song) >= 300000000ull);

    {
        PreviewLoadJob job(entry_for(notes), true, true, hydra::Difficulty::Expert, std::nullopt,
                           4);
        job.start();
        Sleep(50);
        REQUIRE_FALSE(job.finished());  // still opening the big stem
        const auto t0 = std::chrono::steady_clock::now();
        job.cancel();
        // Read after the cancel, so every byte past this mark was opened
        // while the job already knew it was cancelled.
        const uint64_t at_cancel = job.progress().bytes_done;
        while (!job.finished() &&
               std::chrono::steady_clock::now() - t0 < std::chrono::seconds(10))
            Sleep(1);
        const double ms = std::chrono::duration<double, std::milli>(
                              std::chrono::steady_clock::now() - t0)
                              .count();
        const uint64_t at_stop = job.progress().bytes_done;
        CAPTURE(ms);
        CAPTURE(at_cancel);
        CAPTURE(at_stop);
        CHECK(job.finished());
        // Prompt means the cancel was noticed at the next 4 MB report, not
        // after the whole 300 MB file. Counted in bytes, not milliseconds: a
        // busy machine slows the stop but never moves where it happens.
        // Two steps of room, because a report lands on the first Ogg page
        // past each 4 MB mark.
        CHECK(at_stop - at_cancel <= 8ull << 20);
        // The bytes say where the job noticed the cancel, not how long it then
        // took to stop. A stall after that (a teardown waiting on a read) would
        // freeze the UI thread, so a loose ceiling stays: ten times the old
        // 200 ms limit, far above load noise.
        CHECK(ms <= 2000.0);
        CHECK(job.error() == "cancelled");
    }
    remove_file(song);
    remove_file(notes);
    RemoveDirectoryW(hydra::utf8_to_wide(dir).c_str());
}

// A negative chart offset becomes silence in front of the audio, and the job
// reports offset 0, exactly as the old padded buffer did.
TEST_CASE("a Preview load turns a negative chart offset into front silence") {
    const std::string dir = make_temp_dir("loadpad");
    const std::string notes = dir + "\\notes.chart";
    const std::string song = dir + "\\song.ogg";
    const std::string ini = dir + "\\song.ini";
    write_file(notes, hydra::read_file_bytes(corpus::first_chart_with_suffix(".chart")));
    write_file(song, hydra::read_file_bytes(std::string(HYDRA_TESTDATA_DIR) + "/audio/sine220.ogg"));
    const std::string ini_text = "[song]\ndelay = -250\n";
    write_file(ini, std::vector<uint8_t>(ini_text.begin(), ini_text.end()));

    {
        PreviewLoadJob job(entry_for(notes), true, true, hydra::Difficulty::Expert, std::nullopt,
                           4);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        PreviewLoadJob::Result r = job.take_result();
        CHECK(r.audio_offset_ms == 0.0);
        REQUIRE(r.audio != nullptr);
        CHECK(r.audio->sample_rate() == 48000);
        CHECK(r.audio->channels() == 2);

        // The same stem mixed with no pad, for its length.
        std::vector<std::unique_ptr<hydra::audio::StemReader>> one;
        hydra::app::PreviewAudioStem s;
        s.label = "song";
        s.path = song;
        one.push_back(hydra::audio::open_stem_reader(s));
        hydra::audio::StreamMix plain(std::move(one), 48000, 2, 0);
        CHECK(r.audio->length_frames() == plain.length_frames() + 12000);  // 250 ms at 48 kHz

        // The first 250 ms are silence.
        std::vector<float> head(12000 * 2, 1.0f);
        CHECK(r.audio->read(head.data(), 12000) == 12000);
        bool silent = true;
        for (float v : head) silent = silent && v == 0.0f;
        CHECK(silent);
    }
    remove_file(song);
    remove_file(notes);
    remove_file(ini);
    RemoveDirectoryW(hydra::utf8_to_wide(dir).c_str());
}
