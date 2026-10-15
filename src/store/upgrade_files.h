// The library file's own files on disk: the database, the side files SQLite
// keeps beside it, and the one rename that sets a file too old to convert
// aside (store/library_file.h says which files those are). Nothing here reads
// inside a database, so the rules are testable without SQLite.

#ifndef HYDRA_STORE_UPGRADE_FILES_H
#define HYDRA_STORE_UPGRADE_FILES_H

#include <optional>
#include <string>
#include <vector>

namespace hydra::store {

// A database file and the side files SQLite keeps beside it in WAL mode (its
// log and its shared-memory index), file first.
std::vector<std::string> with_side_files(const std::string& db);
// Whether either of `db`'s side files exists. After the only connection to a
// WAL database closes, SQLite removes both; one still there means another
// program has the file open.
bool side_files_present(const std::string& db);

// Removes `db` and its side files, each where present. True when none of
// them is left (a folder at one of the names stays, and gives false).
bool remove_with_side_files(const std::string& db);

// Renames `from` to `to`, which must not exist, writing the change through to
// the disk before returning. Empty on success, else Windows' own text for
// the failure.
std::optional<std::string> rename_file(const std::string& from, const std::string& to);

// The name a library file too old to convert is set aside under: beside it,
// named after the oldest release this build converts
// (kOldestConvertibleVersion), so the notice and the file agree.
std::string set_aside_path(const std::string& db);

// The one rename of a library file. Moves `db` and its side files to
// set_aside_path(db)'s names, replacing an earlier set-aside file there (the
// user's choice, 2026-10-14). The earlier file is deleted only after the move
// has worked; a move that fails leaves both where they were. The file moves
// first and its side files after; when a side file cannot move, the ones moved
// come back, so a stray log never sits beside the fresh library that takes the
// name. Empty on success, else Windows' own text for the failure. The file
// must be closed.
std::optional<std::string> set_aside_old_library(const std::string& db);

}  // namespace hydra::store

#endif  // HYDRA_STORE_UPGRADE_FILES_H
