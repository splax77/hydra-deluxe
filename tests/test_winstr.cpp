// Command-line arguments reach every entry point as UTF-8. main()'s char**
// argv is the ANSI copy, where Windows swaps a fullwidth slash for '/', so
// "Sugar<U+FF0F>Tzu" named a folder that does not exist; these pin that the split
// keeps each character as it was typed.

#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>  // FSCTL_SET_SPARSE

#include "app/user_messages.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "temp_util.h"

namespace {

// ScopedFile deletes the scratch file on destruction. The wide form for
// Win32 calls comes from hydra::win32_path of the UTF-8 path.
using TempFile = testtemp::ScopedFile;

// Makes a file of `bytes` bytes that costs no disk space: the sparse flag
// leaves the never-written range unallocated.
void make_sparse(const TempFile& tmp, long long bytes) {
    HANDLE h = CreateFileW(hydra::win32_path(tmp.path).c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD ret = 0;
    const BOOL sparse =
        DeviceIoControl(h, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &ret, nullptr);
    LARGE_INTEGER size;
    size.QuadPart = bytes;
    const BOOL moved = SetFilePointerEx(h, size, nullptr, FILE_BEGIN);
    const BOOL ended = SetEndOfFile(h);
    CloseHandle(h);
    REQUIRE(sparse);
    REQUIRE(moved);
    REQUIRE(ended);
}

}  // namespace

// ftell returns a 32-bit long on Windows, so a file past 2 GB used to read
// back as empty.
TEST_CASE("file_size_bytes reports sizes past 2 GB") {
    TempFile tmp("sparse_test", ".bin");
    make_sparse(tmp, 2'500'000'000LL);
    CHECK(hydra::file_size_bytes(tmp.path) == 2'500'000'000ULL);
}

// Past 4 GB the size needs its high 32 bits; a helper that dropped them would
// answer 705,032,704 here.
TEST_CASE("every size helper reports a size past 4 GB, open or not") {
    TempFile tmp("sparse_test_5gb", ".bin");
    make_sparse(tmp, 5'000'000'000LL);
    CHECK(hydra::file_size_bytes(tmp.path) == 5'000'000'000ULL);

    std::FILE* f = hydra::fopen_utf8(tmp.path, L"rb");
    REQUIRE(f != nullptr);
    char three[3];
    REQUIRE(std::fread(three, 1, sizeof(three), f) == sizeof(three));
    CHECK(hydra::open_file_size_bytes(f) == std::optional<uint64_t>(5'000'000'000ULL));
    CHECK(_ftelli64(f) == 3);  // the read position did not move
    std::fclose(f);

    HANDLE h = CreateFileW(hydra::win32_path(tmp.path).c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(hydra::open_handle_size_bytes(h) == std::optional<uint64_t>(5'000'000'000ULL));
    CloseHandle(h);
}

// The seek to the end that ImFileGetSize used to do flushed the stream first,
// so bytes not yet on disk counted. The helper keeps that.
TEST_CASE("open_file_size_bytes counts bytes still in a stream's write buffer") {
    TempFile tmp("file_size_buffered", ".bin");
    std::FILE* f = hydra::fopen_utf8(tmp.path, L"wb");
    REQUIRE(f != nullptr);
    const char payload[] = "hello, hydra";
    std::fwrite(payload, 1, sizeof(payload) - 1, f);
    CHECK(hydra::open_file_size_bytes(f) == std::optional<uint64_t>(sizeof(payload) - 1));
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.path) == sizeof(payload) - 1);
}

TEST_CASE("the open-file size helpers say nothing for something that is not a file") {
    CHECK_FALSE(hydra::open_file_size_bytes(nullptr).has_value());
    CHECK_FALSE(hydra::open_handle_size_bytes(nullptr).has_value());
    CHECK_FALSE(hydra::open_handle_size_bytes(INVALID_HANDLE_VALUE).has_value());
    HANDLE read_end = nullptr;
    HANDLE write_end = nullptr;
    REQUIRE(CreatePipe(&read_end, &write_end, nullptr, 0));
    CHECK_FALSE(hydra::open_handle_size_bytes(read_end).has_value());
    CloseHandle(read_end);
    CloseHandle(write_end);
}

// file_size_bytes opens the file for its metadata only, so a file someone else
// holds open exclusively still answers, and a folder answers rather than
// throwing (a fresh empty folder on NTFS holds 0 bytes).
TEST_CASE("file_size_bytes answers for a file held open exclusively, and for a folder") {
    TempFile tmp("file_size_locked", ".bin");
    std::FILE* f = hydra::fopen_utf8(tmp.path, L"wb");
    REQUIRE(f != nullptr);
    std::fwrite("12345", 1, 5, f);
    std::fclose(f);
    HANDLE lock = CreateFileW(hydra::win32_path(tmp.path).c_str(), GENERIC_READ | GENERIC_WRITE,
                              0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(lock != INVALID_HANDLE_VALUE);
    CHECK(hydra::file_size_bytes(tmp.path) == 5);
    CloseHandle(lock);

    TempFile dir("file_size_empty_folder", "");
    const std::wstring wdir = hydra::win32_path(dir.path);
    RemoveDirectoryW(wdir.c_str());  // a leftover from an aborted run
    REQUIRE(CreateDirectoryW(wdir.c_str(), nullptr));
    CHECK(hydra::file_size_bytes(dir.path) == 0);
    RemoveDirectoryW(wdir.c_str());
}

TEST_CASE("file_size_bytes matches a small file's bytes and throws for a missing one") {
    TempFile tmp("file_size_small", ".bin");
    std::FILE* f = hydra::fopen_utf8(tmp.path, L"wb");
    REQUIRE(f != nullptr);
    const char payload[] = "hello, hydra";
    std::fwrite(payload, 1, sizeof(payload) - 1, f);
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.path) == sizeof(payload) - 1);
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(tmp.path);
    CHECK(std::string(bytes.begin(), bytes.end()) == "hello, hydra");

    TempFile missing("file_size_missing_does_not_exist", ".bin");
    CHECK_THROWS_AS(hydra::file_size_bytes(missing.path), std::runtime_error);
    CHECK_THROWS_AS(hydra::read_file_bytes(missing.path), std::runtime_error);
}

// Finding 251's table, the rule the scan's stored rootfolder has always used.
TEST_CASE("parent_folder: the audit's four rows") {
    CHECK(hydra::parent_folder("C:\\a\\b\\notes.mid") == "C:\\a\\b");
    CHECK(hydra::parent_folder("C:\\a\\b\\") == "C:\\a");
    CHECK(hydra::parent_folder("notes.mid").empty());
    CHECK(hydra::parent_folder("").empty());
}

// How a folder and a name join (review of M6-J1a, finding 1): one backslash
// between them, none added when the folder already ends in either slash, and
// an empty folder gives the name alone, as the scan's join has always done.
TEST_CASE("join_folder: no trailing separator, a trailing backslash or slash, an empty folder") {
    CHECK(hydra::join_folder("C:\\a", "song.ini") == "C:\\a\\song.ini");
    CHECK(hydra::join_folder("C:\\a\\", "song.ini") == "C:\\a\\song.ini");
    CHECK(hydra::join_folder("C:\\a/", "song.ini") == "C:\\a/song.ini");
    CHECK(hydra::join_folder("", "song.ini") == "song.ini");
}

// Finding 217: 32 and below means failure, 33 and above means success.
TEST_CASE("shell_execute_ok: 32 and below fail, 33 and above succeed") {
    // ShellExecute hands the code back as an HINSTANCE.
    auto ok = [](intptr_t code) { return hydra::shell_execute_ok(reinterpret_cast<void*>(code)); };
    CHECK_FALSE(ok(0));
    CHECK_FALSE(ok(32));
    CHECK(ok(33));
}

TEST_CASE("a missing file reads as a moved song file") {
    const std::string path = testtemp::temp_path("no_such_song", ".chart");
    std::remove(path.c_str());
    try {
        hydra::read_file_bytes(path);
        FAIL("a missing file was read");
    } catch (const std::exception& e) {
        CHECK(std::string(e.what()).rfind("cannot open file: ", 0) == 0);
        CHECK(hydra::app::plain_error(e) ==
              "Hydra couldn't open the song file. It may have been moved or deleted; run Scan "
              "library to update the library.");
    }
}

TEST_CASE("read_file_bytes of an empty file is empty, not an error") {
    TempFile tmp("file_size_empty", ".bin");
    std::FILE* f = hydra::fopen_utf8(tmp.path, L"wb");
    REQUIRE(f != nullptr);
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.path) == 0);
    CHECK(hydra::read_file_bytes(tmp.path).empty());
}

// A container's notes sit in a small part of a large file, so the note loader
// reads only that part.
TEST_CASE("file_byte_source reads a slice, fewer bytes at the end, none past it") {
    TempFile tmp("file_range", ".bin");
    std::FILE* f = hydra::fopen_utf8(tmp.path, L"wb");
    REQUIRE(f != nullptr);
    std::fwrite("0123456789", 1, 10, f);
    std::fclose(f);
    const hydra::ByteSource file = hydra::file_byte_source(tmp.path);
    CHECK(file.size == 10);
    auto range = [&](uint64_t offset, size_t length) {
        const std::vector<uint8_t> b = file.read(offset, length);
        return std::string(b.begin(), b.end());
    };
    CHECK(range(2, 3) == "234");
    CHECK(range(0, 10) == "0123456789");
    CHECK(range(8, 5) == "89");
    CHECK(range(10, 4).empty());
    CHECK(range(99, 4).empty());
    CHECK(range(3, 0).empty());
    // A length far past the end holds only what the file has: no huge buffer.
    CHECK(range(0, SIZE_MAX) == "0123456789");

    TempFile missing("file_range_missing_does_not_exist", ".bin");
    CHECK_THROWS_AS(hydra::file_byte_source(missing.path), std::runtime_error);

    // Bytes in memory answer as the file does, offset by offset.
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(tmp.path);
    const hydra::ByteSource memory = hydra::memory_byte_source(bytes);
    CHECK(memory.size == file.size);
    for (uint64_t offset : {0ULL, 2ULL, 8ULL, 10ULL, 99ULL})
        for (size_t length : {size_t{0}, size_t{3}, size_t{5}, SIZE_MAX})
            CHECK(memory.read(offset, length) == file.read(offset, length));
}

TEST_CASE("file_byte_source reads at an offset past 4 GB") {
    TempFile tmp("file_range_5gb", ".bin");
    make_sparse(tmp, 5'000'000'000LL);
    const hydra::ByteSource file = hydra::file_byte_source(tmp.path);
    CHECK(file.size == 5'000'000'000ULL);
    CHECK(file.read(4'999'999'990ULL, 100) == std::vector<uint8_t>(10, 0));
}

TEST_CASE("next_piece_read doubles and never wraps") {
    CHECK(hydra::kFirstPieceRead == 65536);  // ADR 0024
    CHECK(hydra::next_piece_read(65536) == 131072);
    CHECK(hydra::next_piece_read(SIZE_MAX / 2 + 1) == SIZE_MAX);
}

TEST_CASE("split_command_line_utf8 keeps a fullwidth slash in a chart path") {
    const std::vector<std::string> args = hydra::split_command_line_utf8(
        L"hydra_replay.exe score --chart "
        L"\"C:\\songs\\black midi - Sugar\uFF0FTzu (Smoochums, Vasasasasa)\\notes.mid\"");
    REQUIRE(args.size() == 4);
    CHECK(args[0] == "hydra_replay.exe");
    CHECK(args[1] == "score");
    CHECK(args[2] == "--chart");
    // U+FF0F as UTF-8, quotes gone, spaces and comma kept inside the one argument.
    CHECK(args[3] ==
          "C:\\songs\\black midi - Sugar\xEF\xBC\x8FTzu (Smoochums, Vasasasasa)\\notes.mid");
    CHECK(args[3].find('/') == std::string::npos);
}

TEST_CASE("split_command_line_utf8 hands back UTF-8, not the ANSI code page") {
    // e-acute exists in code page 1252 (as byte E9), so the ANSI argv kept it
    // but in the wrong encoding for the UTF-8 path helpers; the kanji does not
    // exist there at all.
    const std::vector<std::string> args =
        hydra::split_command_line_utf8(L"hydra_batch.exe --db C:\\x\\caf\u00E9.db \u66F2");
    REQUIRE(args.size() == 4);
    CHECK(args[2] == "C:\\x\\caf\xC3\xA9.db");
    CHECK(args[3] == "\xE6\x9B\xB2");
}

TEST_CASE("split_command_line_utf8 of an empty command line is empty") {
    // CommandLineToArgvW alone would answer with the exe's own path.
    CHECK(hydra::split_command_line_utf8(L"").empty());
}

TEST_CASE("utf8_argv reads this process's command line") {
    const std::vector<std::string> args = hydra::utf8_argv();
    REQUIRE_FALSE(args.empty());
    CHECK(args[0].find("hydra_tests") != std::string::npos);
}

// tests/temp_util.h owns every test's per-process scratch name (audit finding
// 287). The temp folder is read from Windows once here, as the input; the rest
// is the helper's own output.
TEST_CASE("temp_util: the scratch path is the temp folder, the tag and this process") {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    const std::string temp = hydra::wide_to_utf8(tmp);
    const std::string pid = std::to_string(GetCurrentProcessId());

    const std::string file = testtemp::temp_path("probe", ".db");
    CHECK(hydra::starts_with(file, temp));
    CHECK(file.find("hydra_test_probe_") != std::string::npos);
    CHECK(hydra::ends_with_ci(file, "_" + pid + ".db"));

    const std::string dir = testtemp::temp_dir("probe");
    CHECK(hydra::is_directory_utf8(dir));
    CHECK(hydra::starts_with(dir, temp));
    CHECK(hydra::ends_with_ci(dir, "_" + pid));

    // A non-ASCII tag ("Zoë" as UTF-8) names a folder that exists.
    const std::string zoe = testtemp::temp_dir("Zo\xC3\xAB");
    CHECK(hydra::is_directory_utf8(zoe));
    CHECK(zoe.find("Zo\xC3\xAB") != std::string::npos);

    std::error_code ec;
    std::filesystem::remove_all(hydra::os_path(dir), ec);
    std::filesystem::remove_all(hydra::os_path(zoe), ec);
}
