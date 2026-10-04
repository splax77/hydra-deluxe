// Command-line arguments reach every entry point as UTF-8. main()'s char**
// argv is the ANSI copy, where Windows swaps a fullwidth slash for '/', so
// "Sugar<U+FF0F>Tzu" named a folder that does not exist; these pin that the split
// keeps each character as it was typed.

#include "doctest.h"

#include <cstdint>
#include <cstdio>
#include <optional>
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

// Makes a file of `bytes` bytes that costs no disk space: the sparse flag
// leaves the never-written range unallocated.
void make_sparse(const TempFile& tmp, long long bytes) {
    HANDLE h = CreateFileW(tmp.path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
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
    TempFile tmp(L"hydra_sparse_test.bin");
    make_sparse(tmp, 2'500'000'000LL);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == 2'500'000'000ULL);
}

// Past 4 GB the size needs its high 32 bits; a helper that dropped them would
// answer 705,032,704 here.
TEST_CASE("every size helper reports a size past 4 GB, open or not") {
    TempFile tmp(L"hydra_sparse_test_5gb.bin");
    make_sparse(tmp, 5'000'000'000LL);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == 5'000'000'000ULL);

    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"rb");
    REQUIRE(f != nullptr);
    char three[3];
    REQUIRE(std::fread(three, 1, sizeof(three), f) == sizeof(three));
    CHECK(hydra::open_file_size_bytes(f) == std::optional<uint64_t>(5'000'000'000ULL));
    CHECK(_ftelli64(f) == 3);  // the read position did not move
    std::fclose(f);

    HANDLE h = CreateFileW(tmp.path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(h != INVALID_HANDLE_VALUE);
    CHECK(hydra::open_handle_size_bytes(h) == std::optional<uint64_t>(5'000'000'000ULL));
    CloseHandle(h);
}

// The seek to the end that ImFileGetSize used to do flushed the stream first,
// so bytes not yet on disk counted. The helper keeps that.
TEST_CASE("open_file_size_bytes counts bytes still in a stream's write buffer") {
    TempFile tmp(L"hydra_file_size_buffered.bin");
    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"wb");
    REQUIRE(f != nullptr);
    const char payload[] = "hello, hydra";
    std::fwrite(payload, 1, sizeof(payload) - 1, f);
    CHECK(hydra::open_file_size_bytes(f) == std::optional<uint64_t>(sizeof(payload) - 1));
    std::fclose(f);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == sizeof(payload) - 1);
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
    TempFile tmp(L"hydra_file_size_locked.bin");
    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"wb");
    REQUIRE(f != nullptr);
    std::fwrite("12345", 1, 5, f);
    std::fclose(f);
    HANDLE lock = CreateFileW(tmp.path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(lock != INVALID_HANDLE_VALUE);
    CHECK(hydra::file_size_bytes(tmp.utf8()) == 5);
    CloseHandle(lock);

    TempFile dir(L"hydra_file_size_empty_folder");
    RemoveDirectoryW(dir.path.c_str());  // a leftover from an aborted run
    REQUIRE(CreateDirectoryW(dir.path.c_str(), nullptr));
    CHECK(hydra::file_size_bytes(dir.utf8()) == 0);
    RemoveDirectoryW(dir.path.c_str());
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

// A container's notes sit in a small part of a large file, so the note loader
// reads only that part.
TEST_CASE("read_file_range reads a slice, fewer bytes at the end, none past it") {
    TempFile tmp(L"hydra_file_range.bin");
    std::FILE* f = hydra::fopen_utf8(tmp.utf8(), L"wb");
    REQUIRE(f != nullptr);
    std::fwrite("0123456789", 1, 10, f);
    std::fclose(f);
    auto range = [&](uint64_t offset, size_t length) {
        const std::vector<uint8_t> b = hydra::read_file_range(tmp.utf8(), offset, length);
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

    TempFile missing(L"hydra_file_range_missing_does_not_exist.bin");
    CHECK_THROWS_AS(hydra::read_file_range(missing.utf8(), 0, 4), std::runtime_error);

    // The in-memory reader answers as the file does, offset by offset.
    const std::vector<uint8_t> bytes = hydra::read_file_bytes(tmp.utf8());
    const hydra::ByteRangeReader over = hydra::range_reader_over(bytes);
    for (uint64_t offset : {0ULL, 2ULL, 8ULL, 10ULL, 99ULL})
        for (size_t length : {size_t{0}, size_t{3}, size_t{5}, SIZE_MAX})
            CHECK(over(offset, length) == hydra::read_file_range(tmp.utf8(), offset, length));
}

TEST_CASE("read_file_range reads at an offset past 4 GB") {
    TempFile tmp(L"hydra_file_range_5gb.bin");
    make_sparse(tmp, 5'000'000'000LL);
    const std::vector<uint8_t> tail = hydra::read_file_range(tmp.utf8(), 4'999'999'990ULL, 100);
    CHECK(tail == std::vector<uint8_t>(10, 0));
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
