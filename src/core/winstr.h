// UTF-8 <-> UTF-16 conversion and UTF-8-path file helpers for Win32.
//
// Chart libraries contain non-ASCII paths (e.g. a fullwidth slash). The
// narrow CRT / std::ifstream path APIs go through the ANSI codepage on
// Windows and mangle them, so every file open routes through the wide API
// via these helpers.
//
// They also contain very long paths (nested pack folders past 260
// characters). win32_path is the one place a path is made safe for that; every
// helper here uses it, and code that must hand a path to the OS itself calls
// win32_path or os_path, never utf8_to_wide.

#ifndef HYDRA_CORE_WINSTR_H
#define HYDRA_CORE_WINSTR_H

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace hydra {

std::wstring utf8_to_wide(const std::string& s);
std::string wide_to_utf8(const std::wstring& w);

// A path ready for any wide Win32 file function, at any length. Windows caps
// ordinary paths at 260 characters unless the machine has opted in, so a long
// path is made full (GetFullPathNameW resolves ".", ".." and forward slashes,
// which the prefix would otherwise take literally) and given the \\?\ prefix
// (\\?\UNC\ for a network share), which works whatever the machine setting.
// A short path comes back as the plain wide string, so nothing changes for it.
// A path already starting \\?\ or \\.\ is left alone.
std::wstring win32_path(const std::wstring& path);
std::wstring win32_path(const std::string& utf8_path);

// A path the Windows shell will take (ShellExecute, Explorer, a browser's
// command line). The shell accepts no \\?\ path and nothing of 260 characters
// or more, so a long path is swapped for its short 8.3 form (C:\CLONEH~1\...),
// which names the same file. A short path comes back unchanged. Empty when the
// path is long and has no short form: the file is missing, or its drive keeps
// no short names (Windows turns them off on most drives other than C:).
std::wstring shell_path(const std::wstring& path);

// win32_path as a std::filesystem::path, for std::filesystem calls and
// fstreams, which take a path object.
std::filesystem::path os_path(const std::filesystem::path& p);
std::filesystem::path os_path(const std::string& utf8_path);

// _wfopen with a UTF-8 path; nullptr on failure, like fopen.
std::FILE* fopen_utf8(const std::string& utf8_path, const wchar_t* mode);

bool file_exists_utf8(const std::string& utf8_path);
bool is_directory_utf8(const std::string& utf8_path);

// One entry of a folder listing ("." and ".." left out).
struct DirEntry {
    std::string name;  // UTF-8, the entry's own name
    bool is_dir = false;
    // Size and last-write time (FILETIME ticks) straight from the find data:
    // the library rescan cache's change fingerprint, at no extra stat cost.
    uint64_t size = 0;
    uint64_t mtime = 0;
};

// The entries of one folder, in the order Windows lists them. Empty when the
// folder can't be read.
std::vector<DirEntry> list_dir(const std::string& dir_utf8);

// This executable's full path, UTF-8, at any length (the buffer grows until
// GetModuleFileNameW stops truncating).
std::string exe_path_utf8();

// The size of a file in bytes, read from the file system (no read; the file is
// opened for its metadata only, so a file someone else holds open exclusively
// still answers). 64 bits, so sizes past 2 GB come out right. Throws
// std::runtime_error when the file can't be found.
uint64_t file_size_bytes(const std::string& utf8_path);

// The size in bytes of a file the caller already holds open, asked of the open
// file (the read position does not move). Same 64-bit answer as
// file_size_bytes, which shares its code. For a stdio stream, bytes still
// waiting in its write buffer count, as a seek to the end would count them.
// Empty when the file isn't an open disk file or Windows can't say.
std::optional<uint64_t> open_file_size_bytes(std::FILE* f);
// The same for a Win32 HANDLE (void* here so this header needs no windows.h).
std::optional<uint64_t> open_handle_size_bytes(void* win32_handle);

// The whole file's bytes, files over 2 GB included. Throws std::runtime_error
// when the open fails or the file's size can't be read.
std::vector<uint8_t> read_file_bytes(const std::string& utf8_path);

// Up to `length` bytes of the file starting at byte `offset`, at any offset
// (past 4 GB included): fewer where the file ends, none at or past its end.
// The buffer is never larger than what the file holds from `offset`. For
// containers whose notes sit in a small part of a large file. Throws
// std::runtime_error when the open or a read fails.
std::vector<uint8_t> read_file_range(const std::string& utf8_path, uint64_t offset,
                                     size_t length);

// Reads up to `length` bytes of one file from `offset`, like read_file_range
// with the path already chosen. Tests pass an in-memory or counting one.
using ByteRangeReader = std::function<std::vector<uint8_t>(uint64_t offset, size_t length)>;

// The whole file as text, bytes as they are (no newline translation); throws
// std::runtime_error when the open fails.
std::string read_file_text(const std::string& utf8_path);

// A command line split into arguments (CommandLineToArgvW's rules), each one
// UTF-8. Separate from utf8_argv() so tests can feed it any command line.
std::vector<std::string> split_command_line_utf8(const std::wstring& command_line);

// This process's arguments as UTF-8, argv[0] included. main()'s char** argv
// is the ANSI copy of the command line: Windows swaps each character the code
// page lacks for a look-alike (a fullwidth slash becomes '/'), so a path to a
// chart folder with such a name points somewhere else. Every entry point
// reads its arguments from here instead.
std::vector<std::string> utf8_argv();

}  // namespace hydra

#endif  // HYDRA_CORE_WINSTR_H
