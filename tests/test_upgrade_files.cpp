// Tests for store/upgrade_files.{h,cpp}: the library file's side files, and
// the one rename that sets a file too old to convert aside. The files here
// are plain text, not databases: the rules never read inside one.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <filesystem>
#include <string>
#include <vector>

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

TEST_CASE("the library file's side files and set-aside name sit beside it") {
    CHECK(with_side_files("C:\\x\\hydra.db") ==
          std::vector<std::string>{"C:\\x\\hydra.db", "C:\\x\\hydra.db-wal",
                                   "C:\\x\\hydra.db-shm"});
    CHECK(set_aside_path("C:\\x\\hydra.db") == "C:\\x\\hydra.db.pre-2.2.0");
}

TEST_CASE("side_files_present and remove_with_side_files see each side file") {
    const std::string db = fresh_db("upg_side_files");
    put(db, "db");
    CHECK_FALSE(side_files_present(db));
    put(db + "-shm", "index");
    CHECK(side_files_present(db));
    CHECK(remove_with_side_files(db));
    for (const std::string& f : with_side_files(db)) CHECK_FALSE(there(f));
}

TEST_CASE("rename_file refuses to replace a file that exists") {
    const std::string db = fresh_db("upg_no_replace");
    put(db, "a");
    put(db + ".b", "b");
    CHECK(rename_file(db, db + ".b").has_value());
    CHECK(read_file_text(db) == "a");
    CHECK(read_file_text(db + ".b") == "b");
}

TEST_CASE("set_aside_old_library moves the file and its side files, replacing an earlier one") {
    const std::string db = fresh_db("upg_set_aside");
    const std::string aside = set_aside_path(db);
    put(db, "old");
    put(db + "-wal", "old's log");
    put(aside, "earlier");
    put(aside + "-shm", "earlier's index");

    CHECK_FALSE(set_aside_old_library(db).has_value());

    for (const std::string& f : with_side_files(db)) CHECK_FALSE(there(f));
    CHECK(read_file_text(aside) == "old");
    CHECK(read_file_text(aside + "-wal") == "old's log");
    CHECK_FALSE(there(aside + "-shm"));  // the earlier file's index went with it
}

TEST_CASE("set_aside_old_library leaves the file in place when it is held open") {
    const std::string db = fresh_db("upg_set_aside_held");
    put(db, "old");
    HANDLE held = CreateFileW(win32_path(db).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(held != INVALID_HANDLE_VALUE);
    const std::optional<std::string> why = set_aside_old_library(db);
    CloseHandle(held);
    REQUIRE(why.has_value());
    CHECK_FALSE(why->empty());
    CHECK(read_file_text(db) == "old");
    CHECK_FALSE(there(set_aside_path(db)));
}
