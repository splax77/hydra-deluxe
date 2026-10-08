// The files the library file upgrade moves around, and the rule that tidies
// up after a run that stopped partway (a crash, a kill, a power cut).
//
// The copy upgrade (RecordStore's copy_upgrade) writes the kept rows into a
// fresh file beside the database, then swaps the two with two renames: the
// database steps aside under its "old" name, then the fresh file takes the
// database's name. Every state those steps can stop in leaves either the whole
// old file or the whole new file, and recover_upgrade_files, run before every
// open, puts the names back in order. Nothing here reads inside a database,
// so the rule is testable without SQLite.

#ifndef HYDRA_STORE_UPGRADE_FILES_H
#define HYDRA_STORE_UPGRADE_FILES_H

#include <optional>
#include <string>
#include <vector>

namespace hydra::store {

// The fresh file the copy upgrade writes beside `db`, in the same folder so
// the renames stay on one volume.
std::string upgrading_path(const std::string& db);
// The name `db` takes while the fresh file moves into its place.
std::string old_path(const std::string& db);

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

// What recover_upgrade_files found and did.
enum class UpgradeRecovery {
    Nothing,           // no upgrade files: an ordinary open or a fresh database
    RemovedLeftovers,  // the database was whole; an unfinished fresh file or a
                       // stepped-aside original was removed
    FinishedSwap,      // the run stopped between the two renames: the fresh
                       // file, complete by then, became the database
    RestoredOriginal,  // the fresh file was gone: the original came back
};

// Puts `db`'s files in order before it opens, by the state the upgrade left:
//   * `db` exists: it is whole. Its fresh file and its stepped-aside
//     original are leftovers and go.
//   * `db` is missing and its stepped-aside original exists: the first
//     rename ran, which happens only once the fresh file is complete. The
//     fresh file takes the name and the original goes. With no fresh file
//     (someone removed it), the original takes its name back.
//   * neither exists: a fresh database. An orphan fresh file goes.
// When `db` is missing, its side files go first, so no stray log is applied
// to the file that takes its name. Throws KindedError(DatabaseUpgrade) when a
// rename fails, so the open never creates an empty database over the user's
// rows.
UpgradeRecovery recover_upgrade_files(const std::string& db);

}  // namespace hydra::store

#endif  // HYDRA_STORE_UPGRADE_FILES_H
