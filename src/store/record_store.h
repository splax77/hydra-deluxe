// SQLite-backed storage for analysis records (D87, D115). hydra.db keeps the
// chart library, one record per chart and profile, and `meta`. Old
// Python-era .db files are not read; a fresh scan populates a new one.
//
//   * `profiles` — one row per set of settings a record is filed under: the
//     chart mode, the SP cap, the Lens (ms limit, score range and fill rule),
//     the results stamp (stored_versions.h) and the hydra_rules.ini
//     fingerprint. A record made under other rules or by another build sits
//     under its own profile, beside this build's.
//   * `records` — one row per chart and profile: the record's own counts.
//   * `record_paths` — the record's kept paths, ranked from 1 (the best).
//   * `charts` — the chart library, one row per scanned file. A record's
//     names come from here (kNamingCopiesSql).
//
// A Convertible file (store/library_file.h) held one `results` row per chart
// and settings instead. The first open by this build converts it into the
// three tables above, once (convert_results). A TooOld file is set aside or
// refused, as the caller's OldFilePolicy says; which file is which is
// classify_library_file's.
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
#include "store/library_file.h"
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

// A profile: the settings a record is filed under, without the chart. That
// is the chart mode, the SP cap (ADR-0003) and the lens. The store adds the
// results stamp and the rules fingerprint itself when it files or looks up a
// record. A lookup over many charts asks with this; RecordKey adds the chart
// to it.
struct Profile {
    std::string chartmode;
    CapQuery cap;
    Lens lens;
    // Spelled out rather than defaulted: this project builds as C++17.
    bool operator==(const Profile& other) const {
        return chartmode == other.chartmode && cap == other.cap && lens == other.lens;
    }
    bool operator!=(const Profile& other) const { return !(*this == other); }
};
// The old name, kept only while callers move to Profile (D115).
using ResultSettings = Profile;

// One result's identity: the chart and the ResultSettings it was filed under.
struct RecordKey : ResultSettings {
    std::string hyhash;

    RecordKey() = default;
    RecordKey(std::string hyhash_, std::string chartmode_, CapQuery cap_, Lens lens_ = {})
        : ResultSettings{std::move(chartmode_), cap_, lens_}, hyhash(std::move(hyhash_)) {}
    RecordKey(std::string hyhash_, ResultSettings settings_)
        : ResultSettings(std::move(settings_)), hyhash(std::move(hyhash_)) {}

    // The key without its chart.
    const ResultSettings& settings() const { return *this; }

    bool operator==(const RecordKey& other) const {
        return hyhash == other.hyhash && settings() == other.settings();
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

// ---- records (D115) --------------------------------------------------------

// How many paths a record keeps, best first, and so the most the path report
// can show for one chart.
inline constexpr int kPathsPerRecord = 5;

// One kept path of a record: a record_paths row.
struct PathRow {
    int rank = 1;  // the path's place in its record, 1 = best
    std::string path;  // the pathstring
    int64_t score = 0;
    int actcount = 0;
    int maxskip = 0;
    std::optional<double> hardest_ms;     // Path::difficulty()
    std::optional<double> hardest_efill;  // the path's hardest early fill
    double avgmult = 0.0;  // unrounded; format_avg_mult rounds it for display
    int sqin_count = 0;
    int sqout_count = 0;
    bool optimal = false;  // HydraRecord::is_optimal

    // Every column. Spelled out rather than defaulted: this project builds as
    // C++17.
    bool operator==(const PathRow& other) const {
        return rank == other.rank && path == other.path && score == other.score &&
               actcount == other.actcount && maxskip == other.maxskip &&
               hardest_ms == other.hardest_ms && hardest_efill == other.hardest_efill &&
               avgmult == other.avgmult && sqin_count == other.sqin_count &&
               sqout_count == other.sqout_count && optimal == other.optimal;
    }
    bool operator!=(const PathRow& other) const { return !(*this == other); }
};

// One record: a chart's saved result under one profile.
struct RecordRow {
    std::string song_id;  // lowercase hex (ADR 0028)
    int pathcount = 0;  // every kept path, roots and variants (HydraRecord::all_paths)
    std::optional<int> stars;  // rank 1's star count; unset when there are no paths
    int notecount = 0;
    // Rank order, at most kPathsPerRecord. Empty means the chart was analyzed
    // and kept no paths.
    std::vector<PathRow> paths;

    // The library's view of the record, read off rank 1 and the record's own
    // columns. Every field is unset when there are no paths, so
    // PathSummary::has_scored_best_path keeps its meaning (D51 call 11).
    PathSummary summary() const;
    bool has_scored_best_path() const { return !paths.empty(); }

    // Every column and every path, so a fresh record can be compared with the
    // stored one. Spelled out rather than defaulted: this project builds as
    // C++17.
    bool operator==(const RecordRow& other) const {
        return song_id == other.song_id && pathcount == other.pathcount &&
               stars == other.stars && notecount == other.notecount && paths == other.paths;
    }
    bool operator!=(const RecordRow& other) const { return !(*this == other); }
};

// What an analysis becomes before it is saved. It holds no db connection, so
// a worker thread can build it off the main store.
struct PreparedRecord {
    Profile profile;
    uint64_t rules_fp = 0;  // HydraRecord::rules_fingerprint
    RecordRow record;
};

// The one place an analysis becomes a record and its path rows. Throws
// std::invalid_argument when the profile does not match the analysis, as
// prepare_row does.
PreparedRecord prepare_record(std::string song_id, const Profile& profile,
                              const HydraRecord& analysis);

// What a record lookup found. `record` is filled only when status is Ready:
// a Stale record's numbers came from a build or rules this one doesn't trust.
// SummaryLookup stays its own type while its readers still read `bestpath`
// and `summary`; get_summaries reads them off this lookup.
struct RecordLookup {
    RecordStatus status = RecordStatus::NotAnalyzed;
    // Why a Stale record is Stale, as in SummaryLookup.
    bool stale_build = false;
    bool stale_rules = false;
    RecordRow record;
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

// What a rescan can reuse for a chart whose files are unchanged: the stored
// entry, keyed by notespath, valid while `sig` still matches what the walk
// sees on disk. Only rows the current readers stamped are listed, so every
// entry's timing is set (chart_library_cache).
using ChartLibraryCache = std::unordered_map<std::string, ChartLibraryEntry>;

// Whether a stored fingerprint could ever show a chart's files unchanged. An
// empty one is what a gone file or a folder chart without its song.ini gives
// (app::chart_files_sig), so matching it proves nothing. The one owner of that
// answer: chart_library_cache leaves such rows out, the rescan's "unchanged"
// test (sig_unchanged in app/analysis.cpp) asks it, and so does the click's
// save (ui::ViewJob::run).
inline bool sig_can_show_unchanged(const std::string& sig) { return !sig.empty(); }

// Which copy names an md5 (D51 call 10): the first copy the scan listed, the
// charts row with the smallest rowid for that md5. One row per md5, with its
// name, artist and charter (SQLite takes a bare column from the MIN(rowid)
// row), plus `copies`, how many rows the scan listed for it, and
// `naming_rowid`, the naming copy's own rowid. list_records, library_copies
// and naming_copy_paths all read through it.
inline constexpr const char* kNamingCopiesSql =
    "(SELECT md5, name, artist, charter, MIN(rowid) AS naming_rowid, COUNT(*) AS copies"
    " FROM charts"
    " GROUP BY md5)";

// The most charts one save group holds (D86 item 2). When a group closes
// sooner: see run_batch's flush_group (app/analysis.cpp).
inline constexpr int kSaveGroupSize = 16;
// The WAL checkpoint threshold, in pages, while a batch runs (D86 item 3).
// RecordStore::BatchWrites sets it and puts SQLite's own back at the end.
inline constexpr int kBatchWalAutocheckpointPages = 10000;
// The most bytes the WAL file keeps after a checkpoint resets it, set on every
// open (D93): about SQLite's default 1,000-page checkpoint, so the log
// never sits at its high-water size. Firefox ships the same default for the
// same reason (Mozilla bug 1820478).
inline constexpr int kJournalSizeLimitBytes = 4194304;

// ---- opening, and converting a Convertible file ----------------------------

// What the store is doing while it opens, in the order the steps run. An open
// of a file this build already wrote reports Opening alone.
enum class OpenStep {
    Opening,            // the journal settings and the tables
    ConvertingRecords,  // convert_results turning a Convertible file's results into records
};

// One progress report from an opening store.
struct OpenProgress {
    OpenStep step = OpenStep::Opening;
    // ConvertingRecords only: the rows converted so far, and the rows the
    // step will convert, both counted from the file itself. Zero otherwise.
    int64_t rows_done = 0;
    int64_t rows_total = 0;
};

// Called at every step change and, while ConvertingRecords, once per row
// converted. A false return asks the store to stop: it leaves the old file as
// it was and the constructor throws KindedError(ErrorKind::Cancelled). Called
// on the thread that builds the store, never after the constructor returns.
using OpenProgressFn = std::function<bool(const OpenProgress&)>;

class ChartIndex;  // store/chart_index.h

class RecordStore {
public:
    // dbpath may be ":memory:" for an ephemeral store (used by tests).
    // old_files: what to do with a file too old to convert (OldFilePolicy,
    // store/library_file.h). rules_fingerprint: core::RulesStamp::of() the
    // rules this process runs under. A row stamped with any other fingerprint
    // reads Stale. core::RulesStamp::none() (a bad hydra_rules.ini) makes every
    // row Stale. progress: told what the open is doing (OpenProgressFn); empty
    // for a caller with nothing to show.
    //
    // Throws KindedError: Cancelled when `progress` asked to stop,
    // OldLibraryRefused for a file too old to convert under Refuse,
    // OldLibrarySetAsideFailed when SetAside could not rename it,
    // DatabaseUpgrade when converting a Convertible file failed with the
    // file whole, and DatabaseOpen for any other failed open.
    RecordStore(const std::string& dbpath, OldFilePolicy old_files,
                core::RulesStamp rules_fingerprint, OpenProgressFn progress = {});
    // The tests' open, which never renames a file: Refuse. Shipped code names
    // its policy with the constructor above.
    explicit RecordStore(const std::string& dbpath,
                         core::RulesStamp rules_fingerprint = core::default_stamp(),
                         OpenProgressFn progress = {});
    ~RecordStore();

    RecordStore(const RecordStore&) = delete;
    RecordStore& operator=(const RecordStore&) = delete;

    void close();

    // Where this open set a file too old to convert aside
    // (OldFilePolicy::SetAside), so the app can say so; empty when it set
    // nothing aside.
    const std::optional<std::string>& set_aside_path() const { return set_aside_path_; }

    // ---- writing ----------------------------------------------------------

    // The one save (D115). In one transaction, or one SAVEPOINT of the open
    // save group when there is one: deletes the chart's records under every
    // profile whose results stamp this build cannot read, in any mode; files
    // the record under its profile, this build's stamp and its rules,
    // replacing the chart's record there; deletes any profile left with no
    // records. A record made under other rules stays (D51 call 8).
    void save(const PreparedRecord& prepared);

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

    // The one status lookup (D115): one answer per entry of `song_ids`, in
    // the same order (a repeated id gets the same answer twice). The
    // candidates are the chart's records under `want`'s mode, cap and lens,
    // whatever their stamp and rules; the winner is Ready, Stale (with why)
    // or, with no candidate, NotAnalyzed. A Ready answer carries the whole
    // record, its paths in rank order. Ids are sent in chunks under SQLite's
    // bound-value limit.
    std::vector<RecordLookup> lookup(const std::vector<std::string>& song_ids,
                                     const Profile& want);
    RecordLookup lookup_one(const std::string& song_id, const Profile& want);
    // Every chart lookup calls Ready under `want`: the batch's skip list.
    std::unordered_set<std::string> ready_song_ids(const Profile& want);
    // Every Ready record under `want`, paths included, best score first (a
    // record with no paths last, equal scores by song id). Names come from
    // chart_index(), not from here.
    std::vector<RecordRow> ready_records(const Profile& want);

    // The row's status, best-path string and summary.
    // Always returns a value; bestpath is set only when status is Ready.
    SummaryLookup get_summary(const RecordKey& key);

    // get_summary for many charts at once: one answer per entry of
    // `hyhashes`, in the same order (a repeated hash gets the same answer
    // twice). The library asks it about every chart, so the hashes are sent
    // in chunks under SQLite's bound-value limit.
    std::vector<SummaryLookup> get_summaries(const std::vector<std::string>& hyhashes,
                                             const ResultSettings& want);

    // True when lookup reads this exact key as Ready: lookup is the one
    // owner of "this chart has a current result under these settings" (D79).
    bool has_record(const RecordKey& key);

    // The old name of ready_song_ids, kept while callers move (D115).
    std::unordered_set<std::string> analyzed_hashes(const ResultSettings& want);

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

    // How many records the file holds, under any profile.
    int64_t record_count();

    // {charts with a stored result, results} row counts. The second is
    // record_count's; the old name stays while callers move (D115).
    std::pair<int64_t, int64_t> counts();

    // ---- chart library (scan results) ----------------------------------

    // Replaces the whole library with `items`: a scan always fully
    // supersedes the previous one. All or nothing: a failure keeps the
    // previous scan's rows. Stamps the table with kChartMetaStamp. In the
    // same transaction it deletes the results of every chart the library no
    // longer lists (D87 item 4; purge_charts_not_in_library).
    void rebuild_chart_library(const std::vector<ChartLibraryEntry>& items);

    // One chart's files changed since the scan (D87 item 3): the library row
    // at `notespath` takes the hash and fingerprint the files now give, and
    // whatever the old hash leaves unlisted is deleted, by
    // purge_charts_not_in_library, in the same transaction. A path the
    // library doesn't list changes nothing. The hash can be the row's own,
    // for files only saved again (D96); ui::ViewJob::run decides when a
    // click calls this.
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
    // The library's rows for one chart, one per copy the scan listed, read
    // the way list_chart_library reads them. A click on a library row picks
    // its copy from these (ui::AppState::select).
    std::vector<ChartLibraryEntry> list_chart_library_copies(const std::string& md5);

    // The library's charts, their copies and naming copies (ChartIndex),
    // built from the library rows in the order the scan listed them.
    ChartIndex chart_index();

private:
    sqlite3* db_ = nullptr;
    // The fingerprint of the rules this process runs under. A row stamped
    // with anything but `fixed` reads Stale. Computed once, when the store
    // opens.
    core::RulesStamp rules_fingerprint_;
    // What set_aside_path returns.
    std::optional<std::string> set_aside_path_;
    // The constructor's progress callback, held only while the constructor
    // runs and emptied before it returns.
    OpenProgressFn progress_;
    // Tells progress_ the open is at `step`; throws KindedError(Cancelled)
    // when it asks to stop.
    void report(OpenStep step, int64_t rows_done = 0, int64_t rows_total = 0);
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
    // tables, and, for a Convertible file (classify_library_file), the
    // conversion. The constructor turns any throw from it into a failed open.
    void set_up_schema(bool convertible);
    void exec(const char* sql);
    // Runs `work` inside one BEGIN ... COMMIT. A throw from it rolls back
    // and is rethrown. Defined in record_store.cpp, its only user. The save
    // paths keep their own frame: savepoints and save groups are another
    // shape.
    template <class F>
    void in_transaction(F work);
    // Makes the three record tables (profiles, records, record_paths) when
    // they are missing.
    void create_result_tables();
    // Turns a Convertible file's `results` table into records, once
    // (D115): every row becomes a record under its own profile (its own stamp
    // and rules), so a row this build cannot read reads Stale, as after any
    // stamp bump. Stored ids become lowercase, in `charts` too. Then `results`
    // is dropped, so a second open finds a Current file, and the retired
    // whole-file fill-rule label (kRetiredFillLabelKey) is deleted. One
    // transaction; a failure throws KindedError(DatabaseUpgrade) with the file
    // as it was.
    void convert_results();
    // Opens db_ on `dbpath`, creating the file if it is missing; throws
    // KindedError(DatabaseOpen) when SQLite refuses.
    void open_file(const std::string& dbpath);
    // The frame of save and save_analysis: takes the lock and runs `work`
    // in a SAVEPOINT of the open save group, or in a transaction of its own
    // when no group is open. A failure undoes only this chart's writes.
    template <class F>
    void in_save_frame(F work);
    // Which of the chart's unreadable records a save purges: in every mode
    // (save), or only in the record's own mode (write_row, the old save's
    // scope, which the old store tests' fixtures rely on; A1-5 deletes it
    // with write_row).
    enum class PurgeScope { EveryMode, ThisMode };
    // The body of every save (save, and save_analysis through write_row):
    // the purge, the record and the emptied profiles, as save says.
    // `hyversion` is the stamp the record is filed under. The caller holds
    // the lock and an open transaction.
    void file_record(const Profile& profile, const std::string& hyversion, uint64_t rules_fp,
                     const RecordRow& record, PurgeScope scope);
    // The body of add_row and save_analysis: a PreparedRow, which holds only
    // a summary, filed through file_record as a record with its best path as
    // rank 1, the way convert_results files an old row. The caller holds the
    // lock and an open transaction.
    void write_row(const PreparedRow& row);
    // Every chart with a record under `want`'s mode, cap and lens, any stamp
    // or rules: what lookup is asked about to list the Ready ones. The
    // caller holds the lock.
    std::vector<std::string> candidate_song_ids(const Profile& want);
    // The id of the profile row for these settings, stamp and rules
    // fingerprint (the rules_fp column's bytes), inserted when there is none.
    // The caller holds the lock and an open transaction.
    int64_t profile_id_for(const Profile& profile, const std::string& hyversion,
                           const std::vector<uint8_t>& rules_fp);
    // Files `record` under profile `profile_id`: the chart's record and path
    // rows under that profile are replaced. The caller holds the lock and an
    // open transaction.
    void write_record(int64_t profile_id, const RecordRow& record);
    // Deletes every profile no record is filed under. The caller holds the
    // lock and an open transaction.
    void drop_empty_profiles();
    // Deletes the records `where` names, and their path rows (an SQL
    // fragment over song_id and profile_id, which both tables have, bound by
    // `bind`). The caller holds the lock and an open transaction. `what`
    // names the write in the error message.
    void delete_records_where(const std::string& where,
                              const std::function<void(sqlite3_stmt*)>& bind,
                              const std::string& what);
    // Deletes the records of every chart the library no longer lists (D87
    // item 4). Which charts the library lists: not_in_library in
    // record_store.cpp. A listed chart keeps every row, in every chart mode,
    // Stale ones included. A database with no library rows loses every
    // result. The callers (rebuild_chart_library and reidentify_chart) own
    // the library and already hold the lock and an open transaction.
    // `caller` names the operation in the error message.
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
