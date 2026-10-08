// SQL run on a database file through a connection of the caller's own, with no
// test framework in it, so the doctest suite (db_file_util.h) and the GUI
// scripts (tests/ui) share one set of sqlite calls. Each caller decides what
// a failure means: db_file_util.h fails the test, a GUI script checks the
// result.

#ifndef HYDRA_TESTS_DB_FILE_SQL_H
#define HYDRA_TESTS_DB_FILE_SQL_H

#include <sqlite3.h>

#include <cstdint>
#include <optional>
#include <string>

namespace hydra::test {

// Runs a batch of SQL on the database file at `path`, whether or not a store
// has it open. False when the file would not open or the SQL failed, with
// SQLite's text in `*error` when it is given.
inline bool run_sql_on_file(const std::string& path, const std::string& sql,
                            std::string* error = nullptr) {
    sqlite3* db = nullptr;
    char* err = nullptr;
    const bool ok = sqlite3_open(path.c_str(), &db) == SQLITE_OK &&
                    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK;
    if (error) *error = err ? err : (db ? sqlite3_errmsg(db) : "could not open the file");
    sqlite3_free(err);
    sqlite3_close(db);
    return ok;
}

// The first column of the first row `sql` returns, or nothing when the file
// would not open, the SQL failed or it returned no row.
inline std::optional<int64_t> first_int_on_file(const std::string& path, const std::string& sql) {
    sqlite3* db = nullptr;
    sqlite3_stmt* s = nullptr;
    std::optional<int64_t> value;
    if (sqlite3_open(path.c_str(), &db) == SQLITE_OK &&
        sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr) == SQLITE_OK &&
        sqlite3_step(s) == SQLITE_ROW)
        value = sqlite3_column_int64(s, 0);
    sqlite3_finalize(s);
    sqlite3_close(db);
    return value;
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_DB_FILE_SQL_H
