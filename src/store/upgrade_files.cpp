#include "store/upgrade_files.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <system_error>

#include "core/error_kind.h"
#include "core/winstr.h"

namespace hydra::store {

std::string upgrading_path(const std::string& db) { return db + ".upgrading"; }

std::string old_path(const std::string& db) { return db + ".old"; }

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
    // Two plain renames and recover_upgrade_files are safe on every file
    // system.
    if (MoveFileExW(win32_path(from).c_str(), win32_path(to).c_str(), MOVEFILE_WRITE_THROUGH))
        return std::nullopt;
    const DWORD code = GetLastError();
    return std::system_category().message(static_cast<int>(code));
}

namespace {

// A rename recovery cannot do. The database stays missing, so the open must
// not go ahead and create an empty one.
[[noreturn]] void recovery_rename_failed(const std::string& from, const std::string& to,
                                         const std::string& why) {
    throw KindedError(ErrorKind::DatabaseUpgrade,
                      "renaming '" + from + "' to '" + to + "' failed: " + why);
}

void rename_or_throw(const std::string& from, const std::string& to) {
    if (std::optional<std::string> why = rename_file(from, to)) recovery_rename_failed(from, to, *why);
}

}  // namespace

UpgradeRecovery recover_upgrade_files(const std::string& db) {
    const std::string fresh = upgrading_path(db);
    const std::string original = old_path(db);
    const bool fresh_there = file_exists_utf8(fresh);
    const bool original_there = file_exists_utf8(original);

    if (file_exists_utf8(db)) {
        if (!fresh_there && !original_there) return UpgradeRecovery::Nothing;
        remove_with_side_files(fresh);
        remove_with_side_files(original);
        return UpgradeRecovery::RemovedLeftovers;
    }

    if (!original_there) {
        if (!fresh_there) return UpgradeRecovery::Nothing;
        // An orphan: the database it was made from is gone, so nothing says
        // it is complete.
        remove_with_side_files(fresh);
        return UpgradeRecovery::RemovedLeftovers;
    }

    // The database is missing and its original stepped aside. Neither file's
    // log is the database's, so a stray one goes before anything takes the
    // name.
    remove_with_side_files(db);
    if (fresh_there) {
        rename_or_throw(fresh, db);
        remove_with_side_files(fresh);
        remove_with_side_files(original);
        return UpgradeRecovery::FinishedSwap;
    }
    rename_or_throw(original, db);
    return UpgradeRecovery::RestoredOriginal;
}

}  // namespace hydra::store
