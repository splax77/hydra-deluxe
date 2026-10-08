// Test helpers that reach a database file from outside the store: SQL run
// through a connection of the test's own, and a file that isn't a database.
// The sqlite calls are db_file_sql.h's, shared with the GUI scripts; these
// wrappers fail the test when they fail.

#ifndef HYDRA_TESTS_DB_FILE_UTIL_H
#define HYDRA_TESTS_DB_FILE_UTIL_H

#include <cstdint>
#include <fstream>
#include <string>

#include "doctest.h"

#include "core/winstr.h"
#include "db_file_sql.h"

namespace hydra::test {

// Runs a batch of SQL on a database file through a connection of its own,
// whether or not a store has the file open.
inline void exec_on_file(const std::string& path, const char* sql) {
    std::string msg;
    const bool ok = run_sql_on_file(path, sql, &msg);
    INFO(msg);
    REQUIRE(ok);
}

// The first column of the first row `sql` returns, read through a connection
// of the test's own.
inline int64_t scalar_on_file(const std::string& path, const std::string& sql) {
    const std::optional<int64_t> v = first_int_on_file(path, sql);
    REQUIRE(v.has_value());
    return *v;
}

// Writes a file of junk bytes at `path`, where a database should be.
inline void write_junk_db(const std::string& path) {
    std::ofstream f(hydra::os_path(path), std::ios::binary);
    f << std::string(4096, 'x');
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_DB_FILE_UTIL_H
