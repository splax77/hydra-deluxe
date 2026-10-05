// Chart folders with real audio beside the notes, for the tests that load a
// song's audio from disk: the Preview controller's and the song length's. Each
// kind of folder is built here once, so every test that needs one builds it the
// same way.
#ifndef HYDRA_TESTS_AUDIO_CHART_FIXTURES_H
#define HYDRA_TESTS_AUDIO_CHART_FIXTURES_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "doctest.h"

#include "audio_util.h"  // fixture_path
#include "chart_text.h"
#include "core/winstr.h"
#include "corpus_util.h"

namespace audiochart {

inline void copy_file_utf8(const std::string& from, const std::string& to) {
    std::vector<uint8_t> bytes = hydra::read_file_bytes(from);
    std::FILE* f = hydra::fopen_utf8(to, L"wb");
    REQUIRE(f != nullptr);
    if (!bytes.empty()) std::fwrite(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
}

// Writes `text` to `path` as it is, replacing the file.
inline void write_text_file(const std::string& path, const std::string& text) {
    std::FILE* f = hydra::fopen_utf8(path, L"wb");
    REQUIRE(f != nullptr);
    std::fputs(text.c_str(), f);
    std::fclose(f);
}

// This process's own scratch folder for one test's chart, under the temp
// folder, as UTF-8. `tag` keeps the tests' folders apart. Every chart folder
// these fixtures write is made here.
inline std::string temp_chart_dir(const wchar_t* tag) {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    const std::wstring dir = std::wstring(tmp) + L"hydra_prevctl_" + tag +
                             std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(dir.c_str(), nullptr);
    return hydra::wide_to_utf8(dir);
}

// A chart folder that has audio: a corpus .chart plus the test sine as
// song.ogg. The GUI test library has no audio at all, so a chart with audio
// can only be reached here.
inline std::string chart_with_audio() {
    const std::string d = temp_chart_dir(L"");
    copy_file_utf8(corpus::first_chart_with_suffix(".chart"), d + "\\notes.chart");
    copy_file_utf8(testaudio::fixture_path("sine220.ogg"), d + "\\song.ogg");
    return d + "\\notes.chart";
}

// A chart folder whose audio outlasts its notes: two notes at 600 BPM
// (resolution 192, so a beat is 100 ms and a measure 400 ms), the last at
// tick 192 (100 ms), and the 5 s test sine as song.ogg. Without the audio's
// end the beat lines would stop two measures past the last note, at tick
// 1728 (900 ms). `tag` names the folder (temp_chart_dir), so a test that
// changes the folder's files gets its own.
inline std::string short_chart_with_long_audio(const wchar_t* tag = L"tail_") {
    const std::string d = temp_chart_dir(tag);
    using namespace testchart;
    write_text_file(d + "\\notes.chart",
                    chart_text(section("ExpertDrums", line(0, "N 0 0") + line(192, "N 1 0")),
                               192, "", line(0, "TS 4") + line(0, "B 600000")));
    copy_file_utf8(testaudio::fixture_path("sine220.ogg"), d + "\\song.ogg");
    return d + "\\notes.chart";
}

}  // namespace audiochart

#endif  // HYDRA_TESTS_AUDIO_CHART_FIXTURES_H
