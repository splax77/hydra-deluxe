#include "store/upgrade_files.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <system_error>

#include "core/winstr.h"
#include "store/library_file.h"

namespace hydra::store {

std::vector<std::string> with_side_files(const std::string& db) {
    // SQLite's own names for a WAL database's log and shared-memory index.
    return {db, db + "-wal", db + "-shm"};
}

bool side_files_present(const std::string& db) {
    const std::vector<std::string> files = with_side_files(db);
    for (size_t i = 1; i < files.size(); ++i)
        if (file_exists_utf8(files[i])) return true;
    return false;
}

bool remove_with_side_files(const std::string& db) {
    bool all_gone = true;
    for (const std::string& f : with_side_files(db)) {
        if (!file_exists_utf8(f)) continue;
        if (!DeleteFileW(win32_path(f).c_str())) all_gone = false;
    }
    return all_gone;
}

std::optional<std::string> rename_file(const std::string& from, const std::string& to) {
    // No MOVEFILE_REPLACE_EXISTING: on FAT and exFAT a replacing rename is a
    // delete and then a rename, and a stop between the two loses the file.
    if (MoveFileExW(win32_path(from).c_str(), win32_path(to).c_str(), MOVEFILE_WRITE_THROUGH))
        return std::nullopt;
    const DWORD code = GetLastError();
    return std::system_category().message(static_cast<int>(code));
}

std::string set_aside_path(const std::string& db) {
    return db + ".pre-" + kOldestConvertibleVersion;
}

namespace {

// Moves each of `from`'s files that exists to the same place in `to`. When
// one cannot move, the ones already moved go back, and the reason is
// returned. A folder at one of `from`'s names is never moved: it stays, and
// the caller's own rename onto that name fails and says why.
std::optional<std::string> move_files(const std::vector<std::string>& from,
                                      const std::vector<std::string>& to) {
    for (size_t i = 0; i < from.size(); ++i) {
        if (!file_exists_utf8(from[i]) || is_directory_utf8(from[i])) continue;
        if (std::optional<std::string> why = rename_file(from[i], to[i])) {
            for (size_t j = 0; j < i; ++j)
                if (file_exists_utf8(to[j])) rename_file(to[j], from[j]);
            return why;
        }
    }
    return std::nullopt;
}

}  // namespace

std::optional<std::string> set_aside_old_library(const std::string& db) {
    const std::vector<std::string> from = with_side_files(db);
    const std::vector<std::string> to = with_side_files(set_aside_path(db));
    // An earlier set-aside file steps out of the way under a temporary name,
    // and is deleted only once the old file has moved. A move that fails puts
    // it back, so a failure costs the user no file.
    const std::vector<std::string> earlier = with_side_files(to[0] + ".replaced");
    // A run that stopped part way left the temporary name behind. With no
    // set-aside file in place, it is the earlier one and goes back; beside
    // one, it is older still and goes.
    if (!file_exists_utf8(to[0])) move_files(earlier, to);
    remove_with_side_files(earlier[0]);
    if (std::optional<std::string> why = move_files(to, earlier)) return why;
    // The file first, and its side files after it.
    std::optional<std::string> why = rename_file(from[0], to[0]);
    if (!why) {
        why = move_files({from.begin() + 1, from.end()}, {to.begin() + 1, to.end()});
        if (why) rename_file(to[0], from[0]);
    }
    if (why) {
        move_files(earlier, to);
        return why;
    }
    remove_with_side_files(earlier[0]);
    return std::nullopt;
}

}  // namespace hydra::store
