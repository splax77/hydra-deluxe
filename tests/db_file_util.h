// Test helpers that reach a database file from outside the store: SQL run
// through a connection of the test's own, and a file that isn't a database.
// The GUI scripts keep their own copy in tests/ui; see the note there.

#ifndef HYDRA_TESTS_DB_FILE_UTIL_H
#define HYDRA_TESTS_DB_FILE_UTIL_H

#include <sqlite3.h>

#include <cstdint>
#include <fstream>
#include <string>

#include "doctest.h"

#include "core/winstr.h"

namespace hydra::test {

// Runs a batch of SQL on a database file through a connection of its own,
// whether or not a store has the file open.
inline void exec_on_file(const std::string& path, const char* sql) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    char* err = nullptr;
    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    const std::string msg = err ? err : "";
    sqlite3_free(err);
    sqlite3_close(db);
    INFO(msg);
    REQUIRE(rc == SQLITE_OK);
}

// The first column of the first row `sql` returns, read through a connection
// of the test's own.
inline int64_t scalar_on_file(const std::string& path, const std::string& sql) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(path.c_str(), &db) == SQLITE_OK);
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(s) == SQLITE_ROW);
    const int64_t v = sqlite3_column_int64(s, 0);
    sqlite3_finalize(s);
    sqlite3_close(db);
    return v;
}

// Writes a file of junk bytes at `path`, where a database should be.
inline void write_junk_db(const std::string& path) {
    std::ofstream f(hydra::os_path(path), std::ios::binary);
    f << std::string(4096, 'x');
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_DB_FILE_UTIL_H
