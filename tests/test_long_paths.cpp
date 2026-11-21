// Paths past 260 characters. Windows refuses an ordinary path that long unless
// the machine has opted in, and chart packs nest deep enough to hit it. Every
// file call goes through core/winstr's win32_path, which adds the \\?\ prefix
// that lifts the limit; these pin that the conversion is right and that each
// kind of file Hydra touches opens at that depth. That no code goes around it
// is checked by the single-owner rows in tests/single_owner/rules/core.json.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "imgui.h"
#include "imgui_internal.h"  // ImFileOpen, ImFileLoadToMemory

#include "audio_util.h"

#include "app/analysis.h"
#include "app/config.h"
#include "app/preview_source.h"
#include "app/report_files.h"
#include "audio/mapped_file.h"
#include "audio/stem_reader.h"
#include "core/winstr.h"
#include "load_util.h"  // kTestFillRule
#include "store/record_store.h"
#include "temp_util.h"
#include "testdata_dirs.h"  // root

namespace fs = std::filesystem;

namespace {

// Fifty characters, so a few of them nest past the limit.
const std::wstring kSegment = L"A Very Long Chart Pack Folder Name For Testing 0001";

// A fresh scratch folder (testtemp::temp_dir; the root, short) with a folder
// nested more than 300 characters deep inside it. Both removed at the end of
// the test.
struct LongDir {
    std::string root;  // UTF-8
    std::string deep;  // UTF-8, longer than 300 characters

    explicit LongDir(const char* tag)
        : root(testtemp::temp_dir(std::string("long_") + tag)) {
        std::wstring wdeep = hydra::utf8_to_wide(root);
        while (wdeep.size() <= 300) wdeep += L"\\" + kSegment;
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

const std::string kChartDir = corpus::root() + "/common/IB24/T1/Allister - Overrated";
const std::string kOpus = testaudio::fixture_path("sine220.opus");

// What the Preview opens for the chart at `notes`, read for Expert Pro Drums.
hydra::app::PreviewSource preview_of(const std::string& notes) {
    return hydra::app::resolve_preview_source(
        notes, hydra::ChartReading{hydra::Difficulty::Expert, true, false, false},
        hydra::core::default_rules(), hydra::kTestFillRule);
}

}  // namespace

// A corpus chart's own paths: the full one the scan hands the file calls, and
// the folder-and-file part a path relative to the library folder holds.
TEST_CASE("win32_path leaves a short path as it is") {
    const std::string notes = kChartDir + "/notes.mid";
    const std::wstring wnotes = hydra::utf8_to_wide(notes);
    CHECK(hydra::win32_path(wnotes) == wnotes);
    CHECK(hydra::win32_path(notes) == wnotes);
    const std::wstring relative = (fs::path(wnotes).parent_path().filename() / L"notes.mid").native();
    CHECK(hydra::win32_path(relative) == relative);
    CHECK(hydra::win32_path(std::wstring()) == L"");
}

// A real folder nested past 300 characters on this machine's scratch drive.
TEST_CASE("win32_path prefixes a long path and makes it full") {
    LongDir dir("prefix");
    const std::wstring deep = hydra::utf8_to_wide(dir.deep);
    REQUIRE(deep.size() > 300);
    const std::wstring prefixed = L"\\\\?\\" + deep;
    CHECK(hydra::win32_path(deep) == prefixed);

    // Under the prefix Windows takes '/' and ".." literally, so they are
    // resolved before it goes on.
    std::wstring slashed = deep + L"/sub/../notes.mid";
    for (wchar_t& c : slashed)
        if (c == L'\\') c = L'/';
    CHECK(hydra::win32_path(slashed) == prefixed + L"\\notes.mid");

    // A network share gets the UNC form of the prefix. The same folders,
    // under a share's name in place of the drive.
    const std::wstring folders = fs::path(deep).relative_path().native();
    CHECK(hydra::win32_path(L"\\\\server\\share\\" + folders) ==
          L"\\\\?\\UNC\\server\\share\\" + folders);

    // Already prefixed: left alone.
    CHECK(hydra::win32_path(prefixed) == prefixed);

    // The UTF-8 and std::filesystem forms are the same conversion.
    CHECK(hydra::win32_path(dir.deep) == prefixed);
    CHECK(hydra::os_path(dir.deep).native() == prefixed);
    CHECK(hydra::os_path(fs::path(deep)).native() == prefixed);
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
    // The same chart's hash as the scan takes it from its short path.
    CHECK(items[0].md5 == hydra::app::hash_chart_file(kChartDir + "/notes.mid"));
    CHECK_FALSE(items[0].md5.empty());

    // The Preview finds the audio beside it and streams it: the same notes as
    // the chart read from its short path, and the same audio as the file
    // opened from its short path.
    const hydra::app::PreviewSource src = preview_of(notes);
    CHECK(src.song.sequence.size() == preview_of(kChartDir + "/notes.mid").song.sequence.size());
    REQUIRE(src.stems.size() == 1);
    std::unique_ptr<hydra::audio::StemReader> reader = hydra::audio::open_stem_reader(src.stems[0]);
    REQUIRE(reader);
    hydra::app::PreviewAudioStem short_stem;
    short_stem.label = src.stems[0].label;
    short_stem.path = kOpus;
    std::unique_ptr<hydra::audio::StemReader> short_reader = hydra::audio::open_stem_reader(short_stem);
    REQUIRE(short_reader);
    CHECK(reader->length_frames() == short_reader->length_frames());
}

TEST_CASE("settings, reports, and the database work past 260 characters") {
    LongDir dir("files");

    // The same settings saved at a short path and at depth: the two files are
    // the same, and each loads back the same library folder. The folder is a
    // real corpus one, so the file is not the defaults, and a load at depth
    // that fell back to them would not match.
    const std::string ini = dir.deep + "\\hydra_settings.ini";
    const std::string short_ini = dir.root + "\\hydra_settings.ini";
    hydra::app::Settings s;
    s.chartfolders = {kChartDir};
    REQUIRE(s.save_file(short_ini));
    REQUIRE(s.save_file(ini));
    CHECK(hydra::read_file_bytes(ini) == hydra::read_file_bytes(short_ini));
    const hydra::app::Settings short_loaded = hydra::app::Settings::load_file(short_ini);
    REQUIRE(short_loaded.chartfolders != hydra::app::Settings{}.chartfolders);
    CHECK(hydra::app::Settings::load_file(ini).chartfolders == short_loaded.chartfolders);

    const fs::path page(hydra::utf8_to_wide(dir.deep + "\\hydra_paths.html"));
    hydra::app::write_report_file(page, "<html>long</html>");
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(page.wstring())) == "<html>long</html>");
    CHECK_FALSE(hydra::file_exists_utf8(
        hydra::wide_to_utf8(hydra::app::replacing_temp_path(page).wstring())));

    // SQLite writes a -wal and -shm file beside the database while it is open
    // (the store opens in WAL mode), so those go through the same long path too.
    const std::string db = dir.deep + "\\hydra.db";
    {
        hydra::store::RecordStore store(db);
        CHECK(store.record_count() == 0);
        CHECK(hydra::file_exists_utf8(db + "-wal"));
        CHECK(hydra::file_exists_utf8(db + "-shm"));
        store.close();
    }
    CHECK(hydra::file_exists_utf8(db));

    sqlite3* raw = nullptr;
    CHECK(hydra::store::open_sqlite(db, &raw, SQLITE_OPEN_READONLY) == SQLITE_OK);
    sqlite3_close(raw);
}

// ImGui opens its font files and hydra_ui.ini itself, through Hydra's
// ImFileOpen (ui/imgui_files.cpp). Every file of the app's resource folder
// (its fonts and icons) is written at depth through ImGui's calls and loaded
// back the way ImGui loads a font, byte for byte.
TEST_CASE("ImGui's own file calls write and load the app's resource files past 260 characters") {
    LongDir dir("imgui");
    const std::string resources = corpus::resource_root();
    const std::vector<hydra::DirEntry> entries = hydra::list_dir(resources);
    REQUIRE_FALSE(entries.empty());
    for (const hydra::DirEntry& e : entries) {
        if (e.is_dir) continue;
        INFO(e.name);
        const std::vector<uint8_t> bytes = hydra::read_file_bytes(resources + "/" + e.name);
        const std::string deep = dir.deep + "\\" + e.name;
        ImFileHandle f = ImFileOpen(deep.c_str(), "wb");
        REQUIRE(f != nullptr);
        CHECK(ImFileWrite(bytes.data(), 1, bytes.size(), f) == bytes.size());
        CHECK(ImFileClose(f));

        size_t size = 0;
        void* data = ImFileLoadToMemory(deep.c_str(), "rb", &size);
        REQUIRE(data != nullptr);
        const uint8_t* loaded = static_cast<const uint8_t*>(data);
        CHECK(std::vector<uint8_t>(loaded, loaded + size) == bytes);
        IM_FREE(data);
    }
}

// fits_shell is the one place that asks whether the shell takes a path: under
// 260 characters and without a \\?\ or \\.\ prefix. The edge is pinned here as
// a literal so the test doesn't share the owner's arithmetic.
TEST_CASE("fits_shell takes 259 characters, not 260, and no prefixed path") {
    const std::wstring at_259 = L"C:\\" + std::wstring(251, L'a') + L".html";
    const std::wstring at_260 = L"C:\\" + std::wstring(252, L'a') + L".html";
    REQUIRE(at_259.size() == 259);
    REQUIRE(at_260.size() == 260);
    CHECK(hydra::fits_shell(at_259));
    CHECK_FALSE(hydra::fits_shell(at_260));
    CHECK_FALSE(hydra::fits_shell(at_260 + L"x"));
    CHECK(hydra::fits_shell(L"C:\\Songs\\hydra_paths.html"));
    CHECK_FALSE(hydra::fits_shell(L"\\\\?\\C:\\Songs\\hydra_paths.html"));
    CHECK_FALSE(hydra::fits_shell(L"\\\\?\\UNC\\server\\share\\hydra_paths.html"));
    CHECK_FALSE(hydra::fits_shell(L"\\\\.\\C:\\Songs\\hydra_paths.html"));
}

// The shell (ShellExecute, Explorer) opens no path of 260 characters or more,
// and no \\?\ or \\.\ path at any length (fits_shell), so the report buttons
// hand it the short 8.3 name instead.
TEST_CASE("shell_path gives the shell a short name for a long path") {
    CHECK(hydra::shell_path(L"C:\\Songs\\hydra_paths.html") == L"C:\\Songs\\hydra_paths.html");

    LongDir dir("shell");
    const std::string page = dir.deep + "\\hydra_paths.html";
    copy_file_to(kOpus, page);  // any bytes will do
    const std::wstring wpage = hydra::utf8_to_wide(page);

    // The shell takes what comes back, and it names the same file: the same
    // bytes. A drive without 8.3 short names gives the shell nothing for a
    // long path, so this test needs a scratch drive that has them, and fails
    // on one that does not.
    const std::wstring got = hydra::shell_path(wpage);
    const std::string drive = fs::path(wpage).root_name().string();
    const std::string needs = "the scratch folder's drive " + drive + " must keep 8.3 short names";
    INFO(needs);
    REQUIRE_FALSE(got.empty());
    CHECK(hydra::fits_shell(got));
    CHECK(hydra::read_file_bytes(hydra::wide_to_utf8(got)) == hydra::read_file_bytes(kOpus));

    CHECK(hydra::shell_path(wpage + L".missing") == L"");
}

TEST_CASE("a long report page is copied to a short temp path for the browser") {
    LongDir dir("copy");
    const fs::path page(hydra::utf8_to_wide(dir.deep + "\\hydra_longpath_probe.html"));
    hydra::app::write_report_file(page, "<html>copy</html>");

    const fs::path copy = hydra::app::copy_to_short_temp(page);
    REQUIRE_FALSE(copy.empty());
    CHECK(hydra::fits_shell(copy.native()));
    CHECK(copy.filename() == page.filename());
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(copy.wstring())) == "<html>copy</html>");

    // A second open overwrites the older copy.
    hydra::app::write_report_file(page, "<html>newer</html>");
    CHECK(hydra::app::copy_to_short_temp(page) == copy);
    CHECK(hydra::read_file_text(hydra::wide_to_utf8(copy.wstring())) == "<html>newer</html>");
    std::error_code ec;
    fs::remove(copy, ec);
}
