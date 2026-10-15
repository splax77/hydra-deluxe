#include "store/library_file.h"

#include <sqlite3.h>

#include <set>

#include "core/winstr.h"
#include "store/record_store.h"

namespace hydra::store {

namespace {

// A read-only look at one open file's tables. Every question returns false
// when SQLite fails, and `failed` says whether any did.
class TableProbe {
public:
    explicit TableProbe(sqlite3* db) : db_(db) {}

    // The names of the file's tables.
    std::set<std::string> tables() {
        std::set<std::string> names;
        each_row("SELECT name FROM sqlite_master WHERE type='table'",
                 [&](sqlite3_stmt* s) { names.insert(text(s, 0)); });
        return names;
    }

    // Whether `table` has a column named `column`.
    bool has_column(const std::string& table, const char* column) {
        bool found = false;
        each_row("PRAGMA table_info(\"" + table + "\")", [&](sqlite3_stmt* s) {
            if (text(s, 1) == column) found = true;
        });
        return found;
    }

    bool failed() const { return failed_; }

private:
    template <class F>
    void each_row(const std::string& sql, F on_row) {
        sqlite3_stmt* s = nullptr;
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &s, nullptr) != SQLITE_OK) {
            failed_ = true;
            sqlite3_finalize(s);
            return;
        }
        int rc;
        while ((rc = sqlite3_step(s)) == SQLITE_ROW) on_row(s);
        if (rc != SQLITE_DONE) failed_ = true;
        sqlite3_finalize(s);
    }

    static std::string text(sqlite3_stmt* s, int i) {
        const unsigned char* t = sqlite3_column_text(s, i);
        return t ? reinterpret_cast<const char*>(t) : "";
    }

    sqlite3* db_;
    bool failed_ = false;
};

// The layouts, newest first. Each test reads only what a release wrote; the
// releases themselves are in testdata/store/README.md.
LibraryFileKind kind_of(TableProbe& probe) {
    const std::set<std::string> tables = probe.tables();
    if (probe.failed()) return LibraryFileKind::Unreadable;
    if (tables.empty()) return LibraryFileKind::Fresh;

    // Before 2.2.0 a file kept every path's details in their own table; 2.2.0
    // moved to summaries only and left that table behind.
    if (tables.count("paths")) return LibraryFileKind::TooOld;
    // A records table without profiles is Hydra 1.6's own, older than results.
    if (tables.count("records") && !probe.has_column("records", "profile_id"))
        return LibraryFileKind::TooOld;
    if (tables.count("results")) {
        // 2.2.0 to 2.7.0 wrote results with these three columns and no
        // structure blob; a file missing any of them, or still holding the
        // blob, was last opened by an older release.
        const bool convertible = !probe.has_column("results", "structure") &&
                                 probe.has_column("results", "stars") &&
                                 probe.has_column("results", "legacy_fills") &&
                                 probe.has_column("results", "rules_fp");
        if (probe.failed()) return LibraryFileKind::Unreadable;
        return convertible ? LibraryFileKind::Convertible : LibraryFileKind::TooOld;
    }
    // 2.8.0 wrote records without the two columns a stored failure fills.
    const bool lacks_failure_columns =
        tables.count("records") && !(probe.has_column("records", "failure_kind") &&
                                     probe.has_column("records", "failure_text"));
    if (probe.failed()) return LibraryFileKind::Unreadable;
    if (lacks_failure_columns) return LibraryFileKind::LacksFailureColumns;
    return LibraryFileKind::Current;
}

}  // namespace

LibraryFileKind classify_library_file(const std::string& dbpath) {
    if (dbpath.empty() || dbpath == ":memory:") return LibraryFileKind::Fresh;
    if (!file_exists_utf8(dbpath)) return LibraryFileKind::Fresh;
    sqlite3* db = nullptr;
    // Read-write without create: the file exists, and a read-only open of a
    // WAL file could leave its side files behind.
    if (open_sqlite(dbpath, &db, SQLITE_OPEN_READWRITE) != SQLITE_OK) {
        sqlite3_close(db);
        return LibraryFileKind::Unreadable;
    }
    TableProbe probe(db);
    const LibraryFileKind kind = kind_of(probe);
    sqlite3_close(db);
    return kind;
}

}  // namespace hydra::store
