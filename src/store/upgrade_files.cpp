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

std::optional<std::string> set_aside_old_library(const std::string& db) {
    const std::vector<std::string> from = with_side_files(db);
    const std::vector<std::string> to = with_side_files(set_aside_path(db));
    // An earlier set-aside file goes first. One that stays makes the rename
    // below fail, and that failure's text is the reason shown.
    remove_with_side_files(to[0]);
    if (std::optional<std::string> why = rename_file(from[0], to[0])) return why;
    for (size_t i = 1; i < from.size(); ++i) {
        if (!file_exists_utf8(from[i])) continue;
        if (std::optional<std::string> why = rename_file(from[i], to[i])) {
            for (size_t j = 0; j < i; ++j)
                if (file_exists_utf8(to[j])) rename_file(to[j], from[j]);
            return why;
        }
    }
    return std::nullopt;
}

}  // namespace hydra::store
