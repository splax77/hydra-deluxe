// Command-line arguments reach every entry point as UTF-8. main()'s char**
// argv is the ANSI copy, where Windows swaps a fullwidth slash for '/', so
// "Sugar<U+FF0F>Tzu" named a folder that does not exist; these pin that the split
// keeps each character as it was typed.

#include "doctest.h"

#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winioctl.h>  // FSCTL_SET_SPARSE

#include "core/winstr.h"

namespace {

// A file in the temp folder that is deleted when the test ends, pass or fail.
struct TempFile {
    std::wstring path;
    explicit TempFile(const wchar_t* name) {
        wchar_t dir[MAX_PATH];
        GetTempPathW(MAX_PATH, dir);
        path = std::wstring(dir) + name;
    }
    ~TempFile() { DeleteFileW(path.c_str()); }
    std::string utf8() const { return hydra::wide_to_utf8(path); }
};

}  // namespace

// ftell returns a 32-bit long on Windows, so a file past 2 GB used to read
// back as empty. The sparse flag makes the 2.5 GB file cost no disk space.
TEST_CASE("file_size_bytes reports sizes past 2 GB") {
    TempFile tmp(L"hydra_sparse_test.bin");
    HANDLE h = CreateFileW(tmp.path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    DWORD ret = 0;
    const BOOL sparse =
        DeviceIoControl(h, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &ret, nullptr);
    LARGE_INTEGER size;
    size.QuadPart = 2'500'000'000LL;
    const BOOL moved = SetFilePointerEx(h, size, nullptr, FILE_BEGIN);
    const BOOL ended = SetEndOfFile(h);
    CloseHandle(h);
    REQUIRE(sparse);
    REQUIRE(moved);
    REQUIRE(ended);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == 2'500'000'000ULL);
}

TEST_CASE("file_size_bytes matches a small file's bytes and throws for a missing one") {
    TempFile tmp(L"hydra_file_size_small.bin");
    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"wb");
    REQUIRE(f != nullptr);
    const char payload[] = "hello, hydra";
    std::fwrite(payload, 1, sizeof(payload) - 1, f);
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == sizeof(payload) - 1);
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(tmp.utf8());
    CHECK(std::string(bytes.begin(), bytes.end()) == "hello, hydra");

    TempFile missing(L"hydra_file_size_missing_does_not_exist.bin");
    CHECK_THROWS_AS(hydra::file_size_bytes(missing.utf8()), std::runtime_error);
    CHECK_THROWS_AS(hydra::read_file_bytes(missing.utf8()), std::runtime_error);
}

TEST_CASE("read_file_bytes of an empty file is empty, not an error") {
    TempFile tmp(L"hydra_file_size_empty.bin");
    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"wb");
    REQUIRE(f != nullptr);
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == 0);
    CHECK(hydra::read_file_bytes(tmp.utf8()).empty());
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
