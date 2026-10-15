// A hydra.db in the layout Hydra 2.2.0 to 2.7.0 wrote: one results table, no
// record tables (store::LibraryFileKind::Convertible). The store tests and
// the GUI scripts both build one, so the SQL lives here once. It is plain SQL
// text: each caller runs it through its own connection helper (the store
// tests through db_file_util.h, the GUI scripts through their own copy).
// Files from before 2.2.0 come from real releases instead (testdata/store).

#ifndef HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H
#define HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H

#include <string>

namespace hydra::test {

// The results table as Hydra 2.2.0 made it (record_store.cpp at the v2.2.0
// tag, kResultsColumnDefs), literal, so the conversion is tested against the
// real layout rather than one this build describes.
inline constexpr const char* kV220ResultsTableSql =
    "CREATE TABLE results (result_id INTEGER PRIMARY KEY, hyhash TEXT NOT NULL,"
    " chartmode TEXT NOT NULL, hyversion TEXT NOT NULL, sp_cap INTEGER NOT NULL,"
    " ms_enabled INTEGER NOT NULL, ms_value INTEGER NOT NULL, depth_mode INTEGER NOT NULL,"
    " depth_value INTEGER NOT NULL, legacy_fills INTEGER NOT NULL DEFAULT 0,"
    " bestpath TEXT NOT NULL, score INTEGER, actcount INTEGER, maxskip INTEGER,"
    " hardest_ms REAL, avgmult REAL, notecount INTEGER, sqin_count INTEGER,"
    " sqout_count INTEGER, pathcount INTEGER, stars INTEGER, rules_fp BLOB NOT NULL,"
    " UNIQUE (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value,"
    " legacy_fills, rules_fp))";

// The SQL that turns a file this build wrote back into the 2.2.0 layout:
// every record becomes one results row with its best path's columns and its
// profile's settings, stamp and rules. A record that kept no paths leaves
// every summary column NULL, as 2.2.0 wrote it. The record tables go. Run it
// on a file a RecordStore wrote and closed.
inline std::string v220_layout_sql() {
    return std::string(kV220ResultsTableSql) +
           ";INSERT INTO results (hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value,"
           " depth_mode, depth_value, legacy_fills, bestpath, score, actcount,"
           " maxskip, hardest_ms, avgmult, notecount, sqin_count, sqout_count, pathcount, stars,"
           " rules_fp)"
           " SELECT r.song_id, p.chartmode, p.hyversion, p.sp_cap, p.ms_enabled, p.ms_value,"
           " p.depth_mode, p.depth_value, p.legacy_fills, COALESCE(rp.path, ''),"
           " rp.score, rp.actcount, rp.maxskip,"
           " rp.hardest_ms, rp.avgmult, iif(rp.rank IS NULL, NULL, r.notecount), rp.sqin_count,"
           " rp.sqout_count, iif(rp.rank IS NULL, NULL, r.pathcount), r.stars, p.rules_fp"
           " FROM records r JOIN profiles p ON p.id = r.profile_id"
           " LEFT JOIN record_paths rp ON rp.song_id = r.song_id"
           " AND rp.profile_id = r.profile_id AND rp.rank = 1;"
           "DROP TABLE record_paths; DROP TABLE records; DROP TABLE profiles;";
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_OLD_LAYOUT_FIXTURE_H
