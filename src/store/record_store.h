// SQLite-backed storage for analysis summaries (D87). hydra.db keeps the
// chart library, one summary row per chart and settings, and `meta`. Every
// path detail comes from the engine's own analysis when a song is clicked or
// a report is built; nothing here stores a path. Old Python-era .db files are
// not read; a fresh scan populates a new one.
//
//   * `results` — one row per run, keyed by the FULL settings it ran under:
//     the chart, the chart mode, the SP cap, the Lens (ms limit, score
//     range and fill rule) and the hydra_rules.ini fingerprint (schema 4), so
//     a result made under other rules is kept beside this build's. The row
//     holds the run's summary columns, so a sortable library listing never
//     has to analyze anything.
//   * `charts` — the chart library, one row per scanned file. A result's
//     names come from here (kNamingCopiesSql).
//
// An older file also held the path details (`paths`, `path_refs`, a
// `structure` blob per result, `songmeta` and `dynamics`). The first open by
// this build deletes them and shrinks the file (set_up_schema's last step).
//
// Why the full settings and not just the cap: a run under a different ms
// limit or score range is a different answer, and overwriting one with the
// other lost the first. Now they coexist, and a lookup asks for the one it
// wants. The SP cap half of that identity is docs/adr/0003; lookups name the
// cap with a CapQuery and the rest with a Lens.

#ifndef HYDRA_STORE_RECORD_STORE_H
#define HYDRA_STORE_RECORD_STORE_H

#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <string>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/timing.h"
#include "parse/song.h"
#include "search/graph.h"

struct sqlite3;
struct sqlite3_stmt;

namespace hydra::store {

// Opens a SQLite database at a UTF-8 path of any length. The one place Hydra
// calls sqlite3_open: RecordStore and the tools both come through here.
// `flags` are sqlite3_open_v2's (SQLITE_OPEN_*). Returns its result code; *db
// is set even on failure, so the caller closes it either way.
int open_sqlite(const std::string& utf8_path, sqlite3** db, int flags);

// The summary columns computed from a record's best path. All fields are
// unset when the record has no paths (an empty/incompatible result);
// has_scored_best_path says which case a summary is.
struct PathSummary {
    std::optional<int64_t> score;
    std::optional<int> actcount;
    std::optional<int> maxskip;
    std::optional<double> hardest_ms;
    std::optional<double> avgmult;
    std::optional<int> notecount;
    std::optional<int> sqin_count;
    std::optional<int> sqout_count;
    std::optional<int> pathcount;
    // The best path's star count by core/stars' path_stars (solo bonus left
    // out, as Clone Hero counts it).
    std::optional<int> stars;

    // Does this record have a scored best path? The one answer (D51 call 11):
    // a record with no paths has no score and so no facts. Read off `score`,
    // so a summary read back from the stored columns answers the same as one
    // summarize_record just made.
    bool has_scored_best_path() const { return score.has_value(); }

    // Every field, unset against set counting as a difference. Spelled out
    // rather than defaulted: this project builds as C++17.
    bool operator==(const PathSummary& other) const {
        return score == other.score && actcount == other.actcount &&
               maxskip == other.maxskip && hardest_ms == other.hardest_ms &&
               avgmult == other.avgmult && notecount == other.notecount &&
               sqin_count == other.sqin_count && sqout_count == other.sqout_count &&
               pathcount == other.pathcount && stars == other.stars;
    }
    bool operator!=(const PathSummary& other) const { return !(*this == other); }
};

PathSummary summarize_path(const Path& path);
PathSummary summarize_record(const HydraRecord& record);
// A record's best path as text: its pathstring, or empty when it has no
// paths. The bestpath column and hydra_replay's result block both show it.
std::string best_path_text(const HydraRecord& record);

// Which cap's record a lookup wants: at(N), the record analyzed at exactly N
// bars. (Auto, which asked for "the newest row above 4 bars", was removed
// on 2026-09-27.)
struct CapQuery {
    int exact = kCloneHeroSpCap;
    static CapQuery at(int cap) { return CapQuery{cap}; }
    // Spelled out rather than defaulted: this project builds as C++17.
    bool operator==(const CapQuery& other) const { return exact == other.exact; }
    bool operator!=(const CapQuery& other) const { return !(*this == other); }
};

// The rest of the settings a run happened under: the ms limit, the score
// range and the fill rule. Two runs of the same chart at the same cap under
// different lenses are two results, neither overwriting the other.
//
// Canonical form, so equal settings always compare equal: a disabled ms limit
// stores value 0, because the engine ignores the number when the limit is off
// -- "off at 10" and "off at 42" ran the identical search.
struct Lens {
    // 1 = ms limit on, 0 = off. Rows an older Hydra migrated in carry -1
    // ("settings unknown"); no lens has it, so no lookup finds them.
    int ms_enabled = 0;
    int ms_value = 0;
    int depth_mode = 0;  // 0 = scores, 1 = points -- the INI's own ints
    int depth_value = 0;
    // 1 = fills spawn by Clone Hero 1.0's deadline, 0 = by 1.1's, the normal
    // rule (search/graph.h FillDeadlineRule). Part of the key since the GUI
    // got a switch for it; docs/adr/0010 has the history.
    int legacy_fills = 0;

    // `ms` is Settings::mslimit_value when the limit is on, nullopt when off.
    static Lens from(std::optional<int> ms, int depth_mode, int depth_value,
                     bool legacy_fills = false) {
        Lens lens;
        lens.ms_enabled = ms ? 1 : 0;
        lens.ms_value = ms ? *ms : 0;
        lens.depth_mode = depth_mode;
        lens.depth_value = depth_value;
        lens.legacy_fills = legacy_fills ? 1 : 0;
        return lens;
    }

    // Spelled out rather than defaulted: this project builds as C++17.
    bool operator==(const Lens& other) const {
        return ms_enabled == other.ms_enabled && ms_value == other.ms_value &&
               depth_mode == other.depth_mode && depth_value == other.depth_value &&
               legacy_fills == other.legacy_fills;
    }
    bool operator!=(const Lens& other) const { return !(*this == other); }
};

// One result's identity: the chart, the chart mode, the SP cap (ADR-0003) and
// the lens.
struct RecordKey {
    std::string hyhash;
    std::string chartmode;
    CapQuery cap;
    Lens lens;
    bool operator==(const RecordKey& other) const {
        return hyhash == other.hyhash && chartmode == other.chartmode &&
               cap == other.cap && lens == other.lens;
    }
    bool operator!=(const RecordKey& other) const { return !(*this == other); }
};

// A result's row, fully computed and ready to insert — the summary, kept free
// of any db connection so a worker thread can build it off the main store.
struct PreparedRow {
    std::string hyhash;
    std::string chartmode;
    std::string hyversion;
    int sp_cap = kCloneHeroSpCap;
    Lens lens;
    std::string bestpath;
    // The fingerprint of the rules the record was analyzed under
    // (HydraRecord::rules_fingerprint), stored in the rules_fp column.
    uint64_t rules_fp = 0;
    PathSummary summary;
};

// Throws std::invalid_argument if the record carries no sp_cap (every
// analyzer result does), if the key's cap isn't the cap the record was
// analyzed at, if the key's lens has the ms limit on at a value the record
// wasn't analyzed under, or if the lens names the other fill rule -- each
// mismatch would file the result under settings it doesn't belong to.
PreparedRow prepare_row(const RecordKey& key, const HydraRecord& record);

// Schema 2's results table: the columns it shares with this build's table
// (what upgrade_results_key copies across) and its CREATE TABLE text (read
// only by the store test, to build a schema 2 file).
extern const char* const kSchema2ResultsColumns;
extern const char* const kSchema2ResultsTableSql;

// The results version this build stamps on a row and accepts (ADR 0018; not
// the app version). For
// the store and its own tests only -- production callers must not compare
// version stamps themselves; ask a lookup for its RecordStatus instead.
std::string current_record_version();

// What a stored-row lookup found. The store is the only place that decides
// whether a row is usable: NotAnalyzed (no row at all), Stale (a row another
// Hydra version wrote, or one made under other rules -- its numbers are not
// trusted), or Ready (a real result -- which may legitimately have zero
// paths).
enum class RecordStatus { NotAnalyzed, Stale, Ready };

// A song's length as app::analysis_song_length found it. `read` says whether
// the length was worked out; a length the owner gave none for has `read` set
// and no `ms`. Nothing stores it: the click shows it.
struct SongLength {
    bool read = false;
    std::optional<double> ms;
    static SongLength found(std::optional<double> ms) { return SongLength{true, ms}; }
};
// The answer to get_summary.
struct SummaryLookup {
    RecordStatus status = RecordStatus::NotAnalyzed;
    // Why a Stale row is Stale; both can be true, neither is when not Stale.
    // The library's row tooltip names the cause.
    bool stale_build = false;  // another Hydra build
    bool stale_rules = false;  // analyzed under other rules
    std::string bestpath;  // meaningful only when status == Ready
    // The row's summary columns; filled only when status == Ready. A Stale
    // row's numbers came from a build or rules this one doesn't trust, so
    // they're not handed out.
    PathSummary summary;
};

// One row of list_records()/library browsing.
struct RecordListing {
    std::string hyhash;
    std::string ref_name;
    std::string ref_artist;
    std::string ref_charter;
    std::string chartmode;
    int sp_cap = kCloneHeroSpCap;
    std::string bestpath;
    PathSummary summary;
};

enum class SortColumn {
    Score, ActCount, MaxSkip, HardestMs, AvgMult, NoteCount,
    SqInCount, SqOutCount, PathCount, RefName, RefArtist, RefCharter,
};

// What a chart's own metadata states about its timing, read by the scan with
// its names (app::discover_charts, app::read_chart_timing_meta): the song's
// length, which app::song_length_ms turns into the song's one length (D75),
// and the delay that moves it into chart time. Both in ms, both empty when
// the metadata states none (app::stated_length_ms says which lengths count).
struct ChartTimingMeta {
    std::optional<double> length_ms;
    std::optional<double> delay_ms;

    // Spelled out rather than defaulted: this project builds as C++17.
    bool operator==(const ChartTimingMeta& other) const {
        return length_ms == other.length_ms && delay_ms == other.delay_ms;
    }
    bool operator!=(const ChartTimingMeta& other) const { return !(*this == other); }
};

// One scanned chart file, as browsed in the library table. It lives in the
// same db file as the records.
// `sig` is the chart's fingerprint (app::chart_files_sig says what it
// covers) that powers the rescan cache (see chart_library_cache). The
// Preview's changed-chart check and the click read it too, through
// app::chart_changed_since, to skip re-hashing the file.
struct ChartLibraryEntry {
    std::string md5;
    std::string title;
    std::string artist;
    std::string charter;
    std::string notespath;
    std::string rootfolder;
    std::string sig;
    // Empty for a row an older scan wrote, before the scan read timing
    // (list_chart_library says which rows those are). Whoever needs it then
    // reads it from the chart's files (app::chart_timing_meta).
    std::optional<ChartTimingMeta> timing;
};

// What a rescan can reuse for a chart whose files are unchanged: keyed by
// notespath, valid while `sig` still matches what the walk sees on disk.
struct ChartCacheEntry {
    std::string sig;
    std::string md5;
    std::string title;
    std::string artist;
    std::string charter;
    ChartTimingMeta timing;
};
using ChartLibraryCache = std::unordered_map<std::string, ChartCacheEntry>;

// Which copy names an md5 (D51 call 10): the first copy the scan listed, the
// charts row with the smallest rowid for that md5. One row per md5, with its
// name, artist and charter (SQLite takes a bare column from the MIN(rowid)
// row), plus `copies`, how many rows the scan listed for it. list_records,
// library_copies and naming_copy_paths all read through it.
inline constexpr const char* kNamingCopiesSql =
    "(SELECT md5, name, artist, charter, MIN(rowid), COUNT(*) AS copies FROM charts"
    " GROUP BY md5)";

// The most charts one save group holds (D86 item 2). When a group closes
// sooner: see run_batch's flush_group (app/analysis.cpp).
inline constexpr int kSaveGroupSize = 16;
// The WAL checkpoint threshold, in pages, while a batch runs (D86 item 3).
// RecordStore::BatchWrites sets it and puts SQLite's own back at the end.
inline constexpr int kBatchWalAutocheckpointPages = 10000;
// The most bytes the WAL file keeps after a checkpoint resets it, set on every
// open (D87 item 7): about SQLite's default 1,000-page checkpoint, so the log
// never sits at its high-water size. Firefox ships the same default for the
// same reason (Mozilla bug 1820478).
inline constexpr int kJournalSizeLimitBytes = 4194304;

class RecordStore {
public:
    // dbpath may be ":memory:" for an ephemeral store (used by tests). A db
    // from Hydra 1.6 or older keeps its old records table, unread: its charts
    // read Not analyzed (user decision 2026-09-26).
    // rules_fingerprint: core::RulesStamp::of() the rules this process runs
    // under. A row stamped with any other fingerprint reads Stale. The first
    // open by this build also deletes the results Auto saved (delete_auto_results).
    // core::RulesStamp::none() (a bad hydra_rules.ini) makes every row Stale.
    explicit RecordStore(const std::string& dbpath,
                         core::RulesStamp rules_fingerprint = core::default_stamp());
    ~RecordStore();

    RecordStore(const RecordStore&) = delete;
    RecordStore& operator=(const RecordStore&) = delete;

    void close();

    // ---- writing ----------------------------------------------------------

    void add_record(const RecordKey& key, const HydraRecord& record);
    void add_row(const PreparedRow& row);

    // One analyzed chart's summary row, saved as add_row does, but inside the
    // open save group when there is one (as a SAVEPOINT of its own).
    void save_analysis(const PreparedRow& row);

    // A save group (D86 items 1 and 2): one transaction around several
    // save_analysis calls, which the batch's writer opens so it commits once
    // per group instead of once per chart. begin_save_group takes the store
    // lock and keeps it until commit_save_group, so other users of the store
    // wait at most one group. Inside a group each save_analysis is its own
    // SAVEPOINT, so a save that fails still undoes only its own chart.
    // commit_save_group drops the lock whether or not it throws; when it
    // throws, the whole group was rolled back and none of it was kept.
    void begin_save_group();
    void commit_save_group();
    bool save_group_open() const { return group_open_; }
    // True once a save inside the open group failed in a way that made SQLite
    // roll back the whole transaction (which failures do: see transaction_open
    // in record_store.cpp). Every later save in the group then throws, and so
    // does its commit.
    bool save_group_lost() const { return group_lost_; }
    // Test seam: the next commit_save_group throws as a failed COMMIT does,
    // after rolling the group back.
    void fail_next_group_commit_for_test() { fail_next_group_commit_ = true; }

    // A batch's checkpoint setting, held for as long as the guard lives
    // (D86 item 3). While it lives the WAL log may grow to
    // kBatchWalAutocheckpointPages before SQLite folds it into the file. When
    // it ends, one checkpoint writes every result into the file and empties
    // the log, and SQLite's own threshold comes back.
    class BatchWrites {
    public:
        explicit BatchWrites(RecordStore& store);
        ~BatchWrites();
        BatchWrites(const BatchWrites&) = delete;
        BatchWrites& operator=(const BatchWrites&) = delete;

    private:
        RecordStore& store_;
    };
    // The connection's checkpoint threshold in pages, for the tests.
    int wal_autocheckpoint_for_test();
    // The connection's journal_size_limit in bytes, for the tests.
    int64_t journal_size_limit_for_test();

    // ---- reading ------------------------------------------------------

    // The row's status, best-path string and summary.
    // Always returns a value; bestpath is set only when status is Ready.
    SummaryLookup get_summary(const RecordKey& key);

    // get_summary for many charts at once: one answer per entry of
    // `hyhashes`, in the same order (a repeated hash gets the same answer
    // twice). The library asks it about every chart, so the hashes are sent
    // in chunks under SQLite's bound-value limit.
    std::vector<SummaryLookup> get_summaries(const std::vector<std::string>& hyhashes,
                                             const std::string& chartmode,
                                             const CapQuery& cap, const Lens& lens);

    // True when get_summary reads this exact key as Ready: get_summaries is
    // the one owner of "this chart has a current result under these
    // settings" (D79).
    bool has_record(const RecordKey& key);

    // Every chart has_record would say yes to, for one chart mode, cap and
    // lens: the batch's skip list for the whole library. get_summaries
    // answers for each chart, so the skip list and the library's Analyzed
    // chip cannot disagree (D79).
    std::unordered_set<std::string> analyzed_hashes(const std::string& chartmode,
                                                    const CapQuery& cap, const Lens& lens);

    // How many rows the library table lists for each chart, by md5: the
    // `copies` the naming rule counts (D51 call 10). A chart the library
    // doesn't list is absent.
    std::unordered_map<std::string, int> library_copies();

    // The notes file of the copy that stands for each md5 (kNamingCopiesSql's
    // pick), by md5: the file a pass over the library analyzes. A chart the
    // library doesn't list is absent.
    std::unordered_map<std::string, std::string> naming_copy_paths();

    // How many library rows a chart on a page counts as, read from a
    // library_copies map keyed the way `md5` is. A page lists only charts it
    // holds a result for, so one the library doesn't list (a result
    // hydra_batch saved from folder arguments, which leave the library
    // alone) still counts once (D77). This
    // is the one place that rule lives; every page's chart count reads it.
    static int copies_of(const std::unordered_map<std::string, int>& copies,
                         const std::string& md5);

    // ---- listing ------------------------------------------------------

    // The library listing: one row per chart and mode -- the same row a lookup
    // for those settings would pick -- and only the Ready ones. A chart whose
    // best row is stale is left out entirely, so a report reads it the same as
    // a chart nobody has analyzed. Names come from the chart's naming copy
    // (kNamingCopiesSql); a chart the library doesn't list has none. `limit`
    // caps the rows returned after that filtering; a negative limit means no
    // limit.
    std::vector<RecordListing> list_records(
        const std::optional<std::string>& chartmode, const CapQuery& cap, const Lens& lens,
        SortColumn order_by, bool descending, std::optional<int> limit = std::nullopt);

    // {charts with a stored result, results} row counts.
    std::pair<int64_t, int64_t> counts();

    // The fill-spawn stamp hydra_batch last wrote this file with, as stored
    // (search/graph.h engine_mode_stamp writes it). Unset on a db nothing has
    // stamped yet. Only a label on the file: each result carries its own rule
    // in its Lens (docs/adr/0010). A caller that needs the stamp itself (whether
    // there is one, and its text) reads it here; a caller that needs the rule a
    // file holds asks stamped_fill_rule.
    std::optional<std::string> engine_mode();
    void set_engine_mode(const std::string& mode);
    // Which fill rule this file holds: its engine_mode stamp read back through
    // search/graph.h fill_rule_from_stamp. A file with results and no stamp
    // holds the 1.1 rule, the one everything but --legacy-fills runs. An
    // unstamped file with no results, or a stamp fill_rule_from_stamp does not
    // know, holds none. upgrade_results_key reads it for a schema 2 file.
    std::optional<FillDeadlineRule> stamped_fill_rule();

    // ---- chart library (scan results) ----------------------------------

    // Replaces the whole library with `items`: a scan always fully
    // supersedes the previous one. All or nothing: a failure keeps the
    // previous scan's rows. Stamps the table with kChartMetaStamp. In the
    // same transaction it deletes what the library no longer lists, as
    // delete_results_without_chart does (D87 item 4).
    void rebuild_chart_library(const std::vector<ChartLibraryEntry>& items);

    // Deletes the results of every chart the library no longer lists (D87
    // item 4). Which charts the library lists: not_in_library in
    // record_store.cpp. A listed chart keeps every row, in every chart mode,
    // Stale ones included. One transaction. A database with no library rows
    // loses every result, so only callers that own the library call this.
    void delete_results_without_chart();

    // One chart's files changed since the scan (D87 item 3): the library row
    // at `notespath` takes the hash and fingerprint the edited files now
    // give, and whatever the old hash leaves unlisted is deleted, as
    // delete_results_without_chart does, in the same transaction. A path the
    // library doesn't list changes nothing.
    void reidentify_chart(const std::string& notespath, const std::string& new_md5,
                          const std::string& new_sig);

    // The previous scan's rows as a rescan cache (empty on a fresh db, or one
    // whose kChartMetaStamp is missing or not current). Read this BEFORE
    // rebuild_chart_library replaces the table.
    ChartLibraryCache chart_library_cache();

    // The whole library, by name. A negative limit means no limit (SQLite's
    // LIMIT convention). Searching is the library view's (query_matches in
    // app/library_query.h), not SQL's. Each entry's timing is set only when
    // the table's kChartMetaStamp is current, the stamp the scan that read it
    // wrote; rows an older scan wrote carry none.
    int64_t chart_library_count();
    std::vector<ChartLibraryEntry> list_chart_library(int offset, int limit);

private:
    sqlite3* db_ = nullptr;
    // The fingerprint of the rules this process runs under, and the one an
    // Auto run under them carried (read only by delete_auto_results). A row
    // stamped with anything but `fixed` reads Stale. Computed once, when the
    // store opens.
    core::RulesStamp rules_fingerprint_;
    // The rule: this lock covers sqlite calls and nothing else. A prepared
    // statement is compiled, stepped, reset and finalized with the lock held,
    // because all four are sqlite calls.
    std::recursive_mutex mutex_;
    // Compiled statements kept for reuse, keyed by their SQL, so the save
    // path compiles each statement once per connection. Used and reset under
    // the lock; close() finalizes them before the connection closes.
    std::unordered_map<std::string, sqlite3_stmt*> stmt_cache_;
    // The save group's state (begin_save_group). Written only by the thread
    // that holds the group, which also holds the lock.
    bool group_open_ = false;
    bool group_lost_ = false;
    bool fail_next_group_commit_ = false;
    // Runs a transaction-control statement (BEGIN, COMMIT, SAVEPOINT ...)
    // through the statement cache. The caller holds the lock.
    void ctl(const char* sql);

    // The constructor's work after the file opens: the journal settings, the
    // tables, and every upgrade an older file needs. The constructor turns
    // any throw from it into a failed open.
    void set_up_schema();
    void exec(const char* sql);
    bool has_column(const char* table, const char* column);
    bool has_table(const char* table);
    void create_result_tables();
    // Brings an older results table's key up to schema 4. Schema 3 added
    // the fill rule (legacy_fills) to the key, and schema 4 the rules
    // fingerprint (rules_fp, read out of each row's structure blob). Both
    // sit in the UNIQUE constraint, which SQLite cannot alter, so the table
    // is rebuilt once with every row and result_id kept; nothing is analyzed
    // again. A schema 2 file's rows go under the fill rule stamped_fill_rule
    // names, or 1.1 when it names none.
    void upgrade_results_key();
    // The summary-only upgrade (D87 items 1 and 7): a file that still holds
    // the path details loses them, then shrinks. Runs only when the `paths`
    // table exists, so a second open does nothing.
    void drop_stored_details();
    // The body of add_row and save_analysis. The caller holds the lock and an
    // open transaction.
    void write_row(const PreparedRow& row);
    // Deletes the results `where` names (an SQL fragment over `results`,
    // bound by `bind`). The caller holds the lock and an open transaction.
    // `what` names the write in the error message.
    void delete_results_where(const std::string& where,
                              const std::function<void(sqlite3_stmt*)>& bind,
                              const std::string& what);
    // Runs once per database file, when it opens: deletes every result
    // Hydra 1.8.4's Auto saved (user decision 7, 2026-09-27), and marks it
    // done in `meta`.
    void delete_auto_results();
    // The body of delete_results_without_chart, for the callers that already
    // hold the lock and an open transaction (rebuild_chart_library and
    // reidentify_chart). `caller` names the operation in the error message.
    void purge_charts_not_in_library(const char* caller);
    std::optional<std::string> meta_get(const std::string& key);
    void meta_set(const std::string& key, const std::string& value);
    // Whether the charts table's rows were read by this build's readers: its
    // stored kChartMetaStamp is current. The rescan cache and the library
    // listing's timing both ask it. Call under the lock.
    bool chart_meta_current();
};

}  // namespace hydra::store

#endif  // HYDRA_STORE_RECORD_STORE_H
