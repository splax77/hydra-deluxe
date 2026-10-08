// Tests for store/upgrade_files.{h,cpp}: every state a stopped library file
// upgrade can leave comes back as one whole database under its own name. The
// files here are plain text, not databases: the rule never reads inside one.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <filesystem>
#include <string>
#include <vector>

#include "core/error_kind.h"
#include "core/winstr.h"
#include "store/upgrade_files.h"
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

namespace {

// An empty scratch folder for one test, and the database path inside it.
std::string fresh_db(const std::string& tag) {
    const std::string dir = testtemp::temp_dir(tag);
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(os_path(dir), ec))
        std::filesystem::remove_all(entry.path(), ec);
    return join_folder(dir, "hydra.db");
}

void put(const std::string& path, const std::string& text) {
    testtemp::write_bytes(path, std::vector<uint8_t>(text.begin(), text.end()));
}

bool there(const std::string& path) { return file_exists_utf8(path); }

}  // namespace

TEST_CASE("the upgrade's file names sit beside the database") {
    CHECK(upgrading_path("C:\\x\\hydra.db") == "C:\\x\\hydra.db.upgrading");
    CHECK(old_path("C:\\x\\hydra.db") == "C:\\x\\hydra.db.old");
    CHECK(with_side_files("C:\\x\\hydra.db") ==
          std::vector<std::string>{"C:\\x\\hydra.db", "C:\\x\\hydra.db-wal",
                                   "C:\\x\\hydra.db-shm"});
}

TEST_CASE("recovery: a database with no upgrade files is left alone") {
    const std::string db = fresh_db("upg_nothing");
    put(db, "old");
    put(db + "-wal", "its log");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::Nothing);
    CHECK(read_file_text(db) == "old");
    // The database's own log is its own business.
    CHECK(there(db + "-wal"));
}

TEST_CASE("recovery: no files at all is a fresh database") {
    const std::string db = fresh_db("upg_fresh");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::Nothing);
    CHECK_FALSE(there(db));
}

TEST_CASE("recovery: an unfinished fresh file beside a whole database is removed") {
    const std::string db = fresh_db("upg_stale_temp");
    put(db, "old");
    put(upgrading_path(db), "half");
    put(upgrading_path(db) + "-wal", "half's log");
    put(upgrading_path(db) + "-shm", "half's index");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::RemovedLeftovers);
    CHECK(read_file_text(db) == "old");
    for (const std::string& f : with_side_files(upgrading_path(db))) CHECK_FALSE(there(f));
}

TEST_CASE("recovery: a stepped-aside original beside the new database is removed") {
    // The run stopped after the second rename and before the original went.
    const std::string db = fresh_db("upg_after_swap");
    put(db, "new");
    put(old_path(db), "old");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::RemovedLeftovers);
    CHECK(read_file_text(db) == "new");
    CHECK_FALSE(there(old_path(db)));
}

TEST_CASE("recovery: between the two renames, the fresh file becomes the database") {
    const std::string db = fresh_db("upg_between");
    put(old_path(db), "old");
    put(upgrading_path(db), "new");
    // A stray log under the database's name belongs to neither file.
    put(db + "-wal", "stray");
    put(db + "-shm", "stray");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::FinishedSwap);
    CHECK(read_file_text(db) == "new");
    CHECK_FALSE(there(old_path(db)));
    CHECK_FALSE(there(upgrading_path(db)));
    CHECK_FALSE(side_files_present(db));
}

TEST_CASE("recovery: a stepped-aside original with no fresh file takes its name back") {
    const std::string db = fresh_db("upg_restore");
    put(old_path(db), "old");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::RestoredOriginal);
    CHECK(read_file_text(db) == "old");
    CHECK_FALSE(there(old_path(db)));
}

TEST_CASE("recovery: an orphan fresh file with no database is removed") {
    const std::string db = fresh_db("upg_orphan");
    put(upgrading_path(db), "half");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::RemovedLeftovers);
    CHECK_FALSE(there(upgrading_path(db)));
    CHECK_FALSE(there(db));
}

TEST_CASE("recovery: a rename it cannot do throws instead of leaving the name empty") {
    // Going on would let the open create an empty database while the rows sit
    // under the other names. The fresh file is held open without delete
    // sharing, so renaming it fails as a locked file's rename does.
    const std::string db = fresh_db("upg_rename_fails");
    put(old_path(db), "old");
    put(upgrading_path(db), "new");
    HANDLE held = CreateFileW(win32_path(upgrading_path(db)).c_str(), GENERIC_READ,
                              FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    REQUIRE(held != INVALID_HANDLE_VALUE);
    try {
        recover_upgrade_files(db);
        FAIL("recovery went ahead with the fresh file held open");
    } catch (const KindedError& e) {
        CHECK(e.kind() == ErrorKind::DatabaseUpgrade);
    }
    CloseHandle(held);
    CHECK_FALSE(there(db));
    // Both files are still there for the next start.
    CHECK(read_file_text(old_path(db)) == "old");
    CHECK(read_file_text(upgrading_path(db)) == "new");
    CHECK(recover_upgrade_files(db) == UpgradeRecovery::FinishedSwap);
    CHECK(read_file_text(db) == "new");
}

TEST_CASE("rename_file refuses to replace a file that exists") {
    const std::string db = fresh_db("upg_no_replace");
    put(db, "a");
    put(old_path(db), "b");
    CHECK(rename_file(db, old_path(db)).has_value());
    CHECK(read_file_text(db) == "a");
    CHECK(read_file_text(old_path(db)) == "b");
}
