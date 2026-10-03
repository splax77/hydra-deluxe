// Paths past 260 characters. Windows refuses an ordinary path that long unless
// the machine has opted in, and chart packs nest deep enough to hit it. Every
// file call goes through core/winstr's win32_path, which adds the \\?\ prefix
// that lifts the limit; these pin that the conversion is right, that each kind
// of file Hydra touches opens at that depth, and that no code goes around it.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "imgui.h"
#include "imgui_internal.h"  // ImFileOpen, ImFileLoadToMemory

#include "app/analysis.h"
#include "app/config.h"
#include "app/preview_source.h"
#include "app/report_files.h"
#include "audio/mapped_file.h"
#include "audio/stem_reader.h"
#include "core/winstr.h"
#include "store/record_store.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif
#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif
#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;

namespace {

// Fifty characters, so a few of them nest past the limit.
const std::wstring kSegment = L"A Very Long Chart Pack Folder Name For Testing 0001";

std::wstring long_tail(int segments) {
    std::wstring tail;
    for (int i = 0; i < segments; ++i) {
        if (i) tail += L"\\";
        tail += kSegment;
    }
    return tail;
}

// A fresh %TEMP%\hydra_long_<tag>_<pid> folder (the root, short) with a
// folder nested more than 300 characters deep inside it. Both removed at the
// end of the test.
struct LongDir {
    std::string root;  // UTF-8
    std::string deep;  // UTF-8, longer than 300 characters

    explicit LongDir(const char* tag) {
        wchar_t tmp[MAX_PATH];
        GetTempPathW(MAX_PATH, tmp);
        const std::wstring wroot = std::wstring(tmp) + L"hydra_long_" +
                                   hydra::utf8_to_wide(tag) + L"_" +
                                   std::to_wstring(GetCurrentProcessId());
        std::wstring wdeep = wroot;
        while (wdeep.size() <= 300) wdeep += L"\\" + kSegment;
        root = hydra::wide_to_utf8(wroot);
        deep = hydra::wide_to_utf8(wdeep);
        std::error_code ec;
        fs::remove_all(hydra::os_path(root), ec);
        fs::create_directories(hydra::os_path(deep));
    }
    ~LongDir() {
        std::error_code ec;
        fs::remove_all(hydra::os_path(root), ec);
    }
};

void copy_file_to(const std::string& from_utf8, const std::string& to_utf8) {
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(from_utf8);
    std::FILE* f = hydra::fopen_utf8(to_utf8, L"wb");
    REQUIRE(f != nullptr);
    REQUIRE(std::fwrite(bytes.data(), 1, bytes.size(), f) == bytes.size());
    std::fclose(f);
}

const std::string kChartDir = std::string(HYDRA_INPUT_DIR) + "/common/IB24/T1/Allister - Overrated";
const std::string kOpus = std::string(HYDRA_TESTDATA_DIR) + "/audio/sine220.opus";

}  // namespace

TEST_CASE("win32_path leaves a short path as it is") {
    CHECK(hydra::win32_path(std::wstring(L"C:\\Songs\\notes.mid")) == L"C:\\Songs\\notes.mid");
    CHECK(hydra::win32_path(std::string("relative\\notes.mid")) == L"relative\\notes.mid");
    CHECK(hydra::win32_path(std::wstring()) == L"");
}

TEST_CASE("win32_path prefixes a long path and makes it full") {
    const std::wstring tail = long_tail(6);  // 305 characters
    CHECK(hydra::win32_path(L"C:\\" + tail) == L"\\\\?\\C:\\" + tail);

    // Under the prefix Windows takes '/' and ".." literally, so they are
    // resolved before it goes on.
    std::wstring slashed = L"C:/" + tail + L"/sub/../notes.mid";
    for (wchar_t& c : slashed)
        if (c == L'\\') c = L'/';
    CHECK(hydra::win32_path(slashed) == L"\\\\?\\C:\\" + tail + L"\\notes.mid");

    // A network share gets the UNC form of the prefix.
    CHECK(hydra::win32_path(L"\\\\server\\share\\" + tail) ==
          L"\\\\?\\UNC\\server\\share\\" + tail);

    // Already prefixed: left alone.
    const std::wstring prefixed = L"\\\\?\\C:\\" + tail;
    CHECK(hydra::win32_path(prefixed) == prefixed);

    // The UTF-8 and std::filesystem forms are the same conversion.
    const std::string utf8 = hydra::wide_to_utf8(L"C:\\" + tail);
    CHECK(hydra::win32_path(utf8) == L"\\\\?\\C:\\" + tail);
    CHECK(hydra::os_path(utf8).native() == L"\\\\?\\C:\\" + tail);
    CHECK(hydra::os_path(fs::path(L"C:\\" + tail)).native() == L"\\\\?\\C:\\" + tail);
}

TEST_CASE("a chart folder past 260 characters scans, reads, and previews") {
    LongDir dir("chart");
    REQUIRE(dir.deep.size() > 300);
    const std::string notes = dir.deep + "\\notes.mid";
    const std::string ini = dir.deep + "\\song.ini";
    const std::string audio = dir.deep + "\\song.opus";
    copy_file_to(kChartDir + "/notes.mid", notes);
    copy_file_to(kChartDir + "/song.ini", ini);
    copy_file_to(kOpus, audio);

    CHECK(hydra::is_directory_utf8(dir.deep));
    CHECK(hydra::file_exists_utf8(notes));
    CHECK(hydra::file_size_bytes(audio) == hydra::file_size_bytes(kOpus));
    CHECK(hydra::read_file_bytes(notes) == hydra::read_file_bytes(kChartDir + "/notes.mid"));

    std::vector<std::string> names;
    for (const hydra::DirEntry& e : hydra::list_dir(dir.deep)) names.push_back(e.name);
    std::sort(names.begin(), names.end());
    CHECK(names == std::vector<std::string>{"notes.mid", "song.ini", "song.opus"});

    std::shared_ptr<const hydra::audio::MappedFile> mapped = hydra::audio::MappedFile::open(audio);
    REQUIRE(mapped);
    CHECK(mapped->size() == hydra::file_size_bytes(kOpus));

    // The library scan walks down to it and reads it.
    auto [items, errors] = hydra::app::discover_charts({dir.root});
    CHECK(errors.empty());
    REQUIRE(items.size() == 1);
    CHECK(items[0].notespath == notes);
    CHECK(items[0].md5 == hydra::app::hash_chart_file(notes));
    CHECK_FALSE(items[0].md5.empty());

    // The Preview finds the audio beside it and streams it.
    hydra::app::PreviewSource src = hydra::app::resolve_preview_source(notes, true, false);
    CHECK_FALSE(src.song.sequence.empty());
    REQUIRE(src.stems.size() == 1);
    std::unique_ptr<hydra::audio::StemReader> reader = hydra::audio::open_stem_reader(src.stems[0]);
    REQUIRE(reader);
    CHECK(reader->length_frames() > 0);
}

TEST_CASE("settings, reports, and the database work past 260 characters") {
    LongDir dir("files");

    const std::string ini = dir.deep + "\\hydra_settings.ini";
    hydra::app::Settings s;
    s.depth_value = 7;
    REQUIRE(s.save_file(ini));
    CHECK(hydra::app::Settings::load_file(ini).depth_value == 7);

    const fs::path page(hydra::utf8_to_wide(dir.deep + "\\hydra_paths.html"));
    hydra::app::write_report_file(page, "<html>long</html>");
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(page.wstring())) == "<html>long</html>");
    CHECK_FALSE(hydra::file_exists_utf8(hydra::wide_to_utf8(page.wstring()) + ".tmp"));

    // SQLite writes a -wal and -shm file beside the database, so those go
    // through the same long path too.
    const std::string db = dir.deep + "\\hydra.db";
    {
        hydra::store::RecordStore store(db);
        CHECK(store.counts() == std::pair<int64_t, int64_t>{0, 0});
        store.close();
    }
    CHECK(hydra::file_exists_utf8(db));

    sqlite3* raw = nullptr;
    CHECK(hydra::store::open_sqlite(db, &raw, SQLITE_OPEN_READONLY) == SQLITE_OK);
    sqlite3_close(raw);
}

TEST_CASE("ImGui's own file calls (hydra_ui.ini, fonts) work past 260 characters") {
    LongDir dir("imgui");
    const std::string ini = dir.deep + "\\hydra_ui.ini";
    ImFileHandle f = ImFileOpen(ini.c_str(), "wb");
    REQUIRE(f != nullptr);
    CHECK(ImFileWrite("[Window]\n", 1, 9, f) == 9);
    CHECK(ImFileClose(f));

    size_t size = 0;
    void* data = ImFileLoadToMemory(ini.c_str(), "rb", &size);
    REQUIRE(data != nullptr);
    CHECK(std::string(static_cast<const char*>(data), size) == "[Window]\n");
    IM_FREE(data);
}

// The shell (ShellExecute, Explorer) opens no path of 260 characters or more,
// \\?\ or not, so the report buttons hand it the short 8.3 name instead.
TEST_CASE("shell_path gives the shell a short name for a long path") {
    CHECK(hydra::shell_path(L"C:\\Songs\\hydra_paths.html") == L"C:\\Songs\\hydra_paths.html");

    LongDir dir("shell");
    const std::string page = dir.deep + "\\hydra_paths.html";
    copy_file_to(kOpus, page);  // any bytes will do
    const std::wstring wpage = hydra::utf8_to_wide(page);

    // What Windows itself says the short form is. A drive without short
    // names hands the long path back, and then there's nothing to give.
    const std::wstring full = hydra::win32_path(wpage);
    std::wstring raw(32768, L'\0');
    raw.resize(GetShortPathNameW(full.c_str(), &raw[0], static_cast<DWORD>(raw.size())));
    REQUIRE(raw.rfind(L"\\\\?\\", 0) == 0);
    const std::wstring expected = raw.size() - 4 < MAX_PATH ? raw.substr(4) : L"";

    const std::wstring got = hydra::shell_path(wpage);
    CHECK(got == expected);
    if (!got.empty()) {
        CHECK(got.size() < MAX_PATH);
        CHECK(hydra::read_file_bytes(hydra::wide_to_utf8(got)) == hydra::read_file_bytes(kOpus));
    }
    const std::string has_short = got.empty() ? "no" : "yes";
    MESSAGE("short names on this drive: " << has_short);

    CHECK(hydra::shell_path(wpage + L".missing") == L"");
}

TEST_CASE("a long report page is copied to a short temp path for the browser") {
    LongDir dir("copy");
    const fs::path page(hydra::utf8_to_wide(dir.deep + "\\hydra_longpath_probe.html"));
    hydra::app::write_report_file(page, "<html>copy</html>");

    const fs::path copy = hydra::app::copy_to_short_temp(page);
    REQUIRE_FALSE(copy.empty());
    CHECK(copy.native().size() < MAX_PATH);
    CHECK(copy.filename() == page.filename());
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(copy.wstring())) == "<html>copy</html>");

    // A second open overwrites the older copy.
    hydra::app::write_report_file(page, "<html>newer</html>");
    CHECK(hydra::app::copy_to_short_temp(page) == copy);
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(copy.wstring())) == "<html>newer</html>");
    std::error_code ec;
    fs::remove(copy, ec);
}

// The one-place rule, checked: no source file hands a path to the OS without
// going through win32_path. A new direct call fails here with its file and
// line, so the rule can't drift back to many hand-written conversions.
TEST_CASE("every file call in src/ and tools/ goes through win32_path") {
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    // Raw OS calls that take a path. Only winstr.cpp makes them freely; any
    // other line naming one must convert the path on that same line.
    const char* raw_calls[] = {"FindFirstFile", "CreateFileW", "CreateFileA", "fopen(",
                               "_wfreopen", "_wopen", "_wstat", "GetFileAttributes",
                               "DeleteFile", "MoveFile", "CopyFile", "CreateDirectory",
                               "RemoveDirectory", "sqlite3_open"};
    // std::filesystem calls and file streams: the path argument must be
    // os_path(...) on the same line.
    const char* fs_calls[] = {"create_directories(", "create_directory(", "is_directory(",
                              "is_regular_file(", "exists(", "remove(", "remove_all(",
                              "rename(", "copy(", "copy_file(", "file_size(",
                              "last_write_time(", "weakly_canonical(", "canonical(",
                              "directory_iterator("};

    std::vector<std::string> problems;
    int files = 0;
    for (const char* sub : {"src", "tools"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            ++files;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            const bool in_winstr = rel == "src/core/winstr.cpp" || rel == "src/core/winstr.h";
            std::ifstream in(e.path());
            std::string line;
            int lineno = 0;
            while (std::getline(in, line)) {
                ++lineno;
                const size_t first = line.find_first_not_of(" \t");
                if (first == std::string::npos || line.compare(first, 2, "//") == 0) continue;
                const std::string where = rel + ":" + std::to_string(lineno) + ": ";

                // utf8_to_wide is for text; paths take win32_path. The DMBot
                // client converts a URL.
                if (line.find("utf8_to_wide(") != std::string::npos && !in_winstr &&
                    rel != "src/net/dmbot_client.cpp")
                    problems.push_back(where + "utf8_to_wide outside winstr: " + line);

                if (!in_winstr && line.find("win32_path(") == std::string::npos) {
                    for (const char* call : raw_calls)
                        if (line.find(call) != std::string::npos)
                            problems.push_back(where + "raw file call without win32_path: " + line);
                }

                // The shell takes no long path, so its two callers swap one
                // for shell_path's short form; nothing else launches it.
                if ((line.find("ShellExecute") != std::string::npos ||
                     line.find("CreateProcess") != std::string::npos) &&
                    rel != "src/app/report_files.cpp" && rel != "src/ui/win32_dialogs.cpp")
                    problems.push_back(where + "shell launch outside the two report buttons: " +
                                       line);

                const bool names_fs = line.find("filesystem::") != std::string::npos ||
                                      line.find("fs::") != std::string::npos;
                const bool opens_stream = line.find("fstream ") != std::string::npos &&
                                          line.find("#include") == std::string::npos;
                if ((names_fs || opens_stream) && line.find("os_path(") == std::string::npos) {
                    bool hit = opens_stream;
                    for (const char* call : fs_calls)
                        if (line.find(std::string("::") + call) != std::string::npos) hit = true;
                    if (hit) problems.push_back(where + "file call without os_path: " + line);
                }
            }
        }
    }
    CHECK(files > 100);  // the scan found the sources
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}
