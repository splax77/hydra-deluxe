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

// The whole file's bytes, files over 2 GB included, read through
// file_byte_source. Throws std::runtime_error when the open fails or the
// file's size can't be read.
std::vector<uint8_t> read_file_bytes(const std::string& utf8_path);

// A file, or bytes in memory, read in pieces: how many bytes it holds, and up
// to `length` of them from `offset` (fewer where it ends, none at or past its
// end; the buffer is never larger than that). A container's notes sit in a
// small part of a large file, so the note loader reads only that part.
struct ByteSource {
    uint64_t size = 0;
    std::function<std::vector<uint8_t>(uint64_t offset, size_t length)> read;
};

// The file, opened once and held open while the source (or a copy of it)
// lives, read at any offset (past 4 GB included). Shared for reading and
// writing, as a C "rb" open is. Throws std::runtime_error when the open fails
// or the size can't be read. A read the system refuses stops early and
// returns what came before it, as fread does.
ByteSource file_byte_source(const std::string& utf8_path);

// Bytes already in memory, read the same way. `bytes` must outlive the source.
ByteSource memory_byte_source(const std::vector<uint8_t>& bytes);

// How a container read in pieces sizes its reads: the first is 64 KB, which
// holds any real .sng header or .srb metadata block and most notes streams,
// and each read after one of `last` bytes asks twice as much, so a part of n
// bytes takes a handful of reads and at most about 2n bytes come off disk.
// The sizes change only how many bytes are read, never what a load returns.
// The user's decision, ADR 0024.
constexpr size_t kFirstPieceRead = 64 * 1024;
size_t next_piece_read(size_t last);

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
