// A hydra.db in the layout before summary-only storage (D87): the results
// table with its structure blob and the four detail tables. The store tests
// and the GUI scripts both build one, so the SQL lives here once. It is plain
// SQL text: each caller runs it through its own connection helper (the store
// tests through db_file_util.h, the GUI scripts through their own copy).

#ifndef HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H
#define HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H

#include <string>

#include "store/record_store.h"  // kSchema2ResultsColumns

namespace hydra::test {

// The results table as the last build that stored path details wrote it
// (schema 4 with the structure blob), literal, so the upgrade is tested
// against the real old layout rather than one this build describes.
inline constexpr const char* kDetailLayoutResultsTableSql =
    "CREATE TABLE results (result_id INTEGER PRIMARY KEY, hyhash TEXT NOT NULL,"
    " chartmode TEXT NOT NULL, hyversion TEXT NOT NULL, sp_cap INTEGER NOT NULL,"
    " ms_enabled INTEGER NOT NULL, ms_value INTEGER NOT NULL, depth_mode INTEGER NOT NULL,"
    " depth_value INTEGER NOT NULL, legacy_fills INTEGER NOT NULL DEFAULT 0,"
    " bestpath TEXT NOT NULL, structure BLOB NOT NULL, score INTEGER, actcount INTEGER,"
    " maxskip INTEGER, hardest_ms REAL, avgmult REAL, notecount INTEGER, sqin_count INTEGER,"
    " sqout_count INTEGER, pathcount INTEGER, stars INTEGER, rules_fp BLOB NOT NULL,"
    " UNIQUE (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value,"
    " legacy_fills, rules_fp))";

// The four detail tables that build kept beside it, as it made them, with
// one row each so the drop has something to free.
inline constexpr const char* kDetailTablesSql =
    "CREATE TABLE paths (hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,"
    "  phash TEXT NOT NULL, payload BLOB NOT NULL, PRIMARY KEY (hyhash, chartmode, phash));"
    "CREATE TABLE path_refs (result_id INTEGER NOT NULL, hyhash TEXT NOT NULL,"
    "  chartmode TEXT NOT NULL, phash TEXT NOT NULL, PRIMARY KEY (result_id, phash));"
    "CREATE INDEX path_refs_by_node ON path_refs (hyhash, chartmode, phash);"
    "CREATE TABLE songmeta (hyhash TEXT PRIMARY KEY, ref_name TEXT, ref_artist TEXT,"
    "  ref_charter TEXT, tempomap BLOB NOT NULL, length_ms REAL,"
    "  length_version INTEGER NOT NULL DEFAULT 0);"
    "CREATE TABLE dynamics (md5 TEXT NOT NULL, difficulty TEXT NOT NULL,"
    "  pro INTEGER NOT NULL, blob BLOB NOT NULL, count_version INTEGER NOT NULL DEFAULT 0,"
    "  PRIMARY KEY (md5, difficulty, pro));"
    "INSERT INTO paths VALUES ('ready', 'mode', 'p1', zeroblob(4096));"
    "INSERT INTO path_refs VALUES (1, 'ready', 'mode', 'p1');"
    "INSERT INTO songmeta VALUES ('ready', 'Song', 'Artist', 'Charter', zeroblob(4096),"
    "  1000.0, 2);"
    "INSERT INTO dynamics VALUES ('ready', 'Expert', 1, zeroblob(4096), 1);";

// The SQL that turns a file this build wrote back into the layout before
// summary-only storage: every result keeps its id and columns and gains the
// structure blob, whose head was the path format (7, the last one) and then
// the rules fingerprint, and the detail tables come back. Run it on a file a
// RecordStore wrote and closed.
inline std::string detail_layout_sql() {
    const std::string columns =
        std::string(hydra::store::kSchema2ResultsColumns) + ", legacy_fills, rules_fp";
    return std::string("ALTER TABLE results RENAME TO results_now;") +
           kDetailLayoutResultsTableSql + ";INSERT INTO results (" + columns +
           ", structure) SELECT " + columns +
           ", unhex('07000000' || hex(rules_fp)) FROM results_now;"
           "DROP TABLE results_now;" +
           kDetailTablesSql;
}

// How many of the detail tables (and their one index) a file holds: 5 in the
// old layout, 0 once upgraded.
inline constexpr const char* kDetailTablesCountSql =
    "SELECT COUNT(*) FROM sqlite_master WHERE name IN"
    " ('paths', 'path_refs', 'path_refs_by_node', 'songmeta', 'dynamics')";

// Whether the results table still has its structure column: 1 or 0.
inline constexpr const char* kStructureColumnCountSql =
    "SELECT COUNT(*) FROM pragma_table_info('results') WHERE name='structure'";

}  // namespace hydra::test

#endif  // HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H
