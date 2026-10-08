#include "store/record_store.h"

#include <sqlite3.h>

#include <algorithm>
#include <cstdlib>
#include <map>
#include <new>
#include <stdexcept>
#include <string_view>

#include "core/error_kind.h"
#include "core/little_endian.h"
#include "core/stars.h"
#include "core/winstr.h"
#include "store/stored_versions.h"

namespace hydra::store {

namespace {

// RAII wrapper so every query site finalizes even on an early throw. Movable
// but not copyable: a copy would finalize the same handle twice.
struct Stmt {
    sqlite3_stmt* p = nullptr;
    Stmt() = default;
    Stmt(Stmt&& other) noexcept : p(other.p) { other.p = nullptr; }
    Stmt& operator=(Stmt&& other) noexcept {
        if (this != &other) {
            if (p) sqlite3_finalize(p);
            p = other.p;
            other.p = nullptr;
        }
        return *this;
    }
    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;
    ~Stmt() {
        if (p) sqlite3_finalize(p);
    }
    operator sqlite3_stmt*() const { return p; }
};

// Compiles a statement. A read that fails to compile is a failed read, and a
// write a failed write (D73), so each call site says which it is through
// prepare_read or prepare_write. SQLite's own sqlite3_stmt_readonly checks
// that word: a site that names the wrong one is a bug in this file.
Stmt prepare_as(sqlite3* db, const char* sql, bool read) {
    Stmt s;
    if (sqlite3_prepare_v2(db, sql, -1, &s.p, nullptr) != SQLITE_OK)
        throw KindedError(read ? ErrorKind::DatabaseRead : ErrorKind::DatabaseWrite,
                          std::string("prepare failed: ") + sqlite3_errmsg(db) + " (" + sql + ")");
    if ((sqlite3_stmt_readonly(s) != 0) != read)
        throw std::logic_error(std::string(read ? "a write" : "a read") +
                               " was prepared as the other kind: " + sql);
    return s;
}
Stmt prepare_read(sqlite3* db, const char* sql) { return prepare_as(db, sql, true); }
Stmt prepare_write(sqlite3* db, const char* sql) { return prepare_as(db, sql, false); }

// A statement borrowed from the store's cache (RecordStore::stmt_cache_). At
// scope end it is reset and its bindings cleared, so no cursor stays open and
// no value carries into its next use; the cache keeps it compiled.
struct CachedStmt {
    sqlite3_stmt* p = nullptr;
    explicit CachedStmt(sqlite3_stmt* p_) : p(p_) {}
    CachedStmt(const CachedStmt&) = delete;
    CachedStmt& operator=(const CachedStmt&) = delete;
    ~CachedStmt() {
        sqlite3_reset(p);
        sqlite3_clear_bindings(p);
    }
    operator sqlite3_stmt*() const { return p; }
};

// The cached statement for `sql`, compiled through prepare_as the first time
// this connection runs it. use_read and use_write say which kind it is, as
// prepare_read and prepare_write do.
using StmtCache = std::unordered_map<std::string, sqlite3_stmt*>;
CachedStmt use_stmt(sqlite3* db, StmtCache& cache, const std::string& sql, bool read) {
    auto it = cache.find(sql);
    if (it == cache.end()) {
        Stmt s = prepare_as(db, sql.c_str(), read);
        it = cache.emplace(sql, s.p).first;
        s.p = nullptr;
    }
    return CachedStmt(it->second);
}
CachedStmt use_read(sqlite3* db, StmtCache& cache, const std::string& sql) {
    return use_stmt(db, cache, sql, true);
}
CachedStmt use_write(sqlite3* db, StmtCache& cache, const std::string& sql) {
    return use_stmt(db, cache, sql, false);
}

// Steps a read. True: a row is ready. False: the query has finished. Any
// other answer throws, so a failed read never passes for an empty one (D73).
bool step_row(sqlite3_stmt* s) {
    const int rc = sqlite3_step(s);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw KindedError(ErrorKind::DatabaseRead, std::string("reading the database failed: ") +
                                                   sqlite3_errmsg(sqlite3_db_handle(s)));
}

// Steps a write to its end. `what` names the write in the raw text: "<what>
// failed: <sqlite's message>".
void step_done(sqlite3_stmt* s, const std::string& what) {
    if (sqlite3_step(s) != SQLITE_DONE)
        throw KindedError(ErrorKind::DatabaseWrite,
                          what + " failed: " + sqlite3_errmsg(sqlite3_db_handle(s)));
}

void bind_text(sqlite3_stmt* s, int i, const std::string& v) {
    sqlite3_bind_text(s, i, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT);
}
void bind_blob(sqlite3_stmt* s, int i, const std::vector<uint8_t>& v) {
    sqlite3_bind_blob(s, i, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT);
}
std::string column_text(sqlite3_stmt* s, int i) {
    const unsigned char* p = sqlite3_column_text(s, i);
    int n = sqlite3_column_bytes(s, i);
    return p ? std::string(reinterpret_cast<const char*>(p), static_cast<size_t>(n))
             : std::string();
}
std::vector<uint8_t> column_blob(sqlite3_stmt* s, int i) {
    const void* p = sqlite3_column_blob(s, i);
    int n = sqlite3_column_bytes(s, i);
    if (!p || n <= 0) return {};
    const uint8_t* b = static_cast<const uint8_t*>(p);
    return std::vector<uint8_t>(b, b + n);
}
std::optional<int64_t> column_opt_i64(sqlite3_stmt* s, int i) {
    if (sqlite3_column_type(s, i) == SQLITE_NULL) return std::nullopt;
    return sqlite3_column_int64(s, i);
}
std::optional<double> column_opt_f64(sqlite3_stmt* s, int i) {
    if (sqlite3_column_type(s, i) == SQLITE_NULL) return std::nullopt;
    return sqlite3_column_double(s, i);
}
// An optional number as SQL: NULL when empty, so column_opt_f64 reads it back.
void bind_opt_f64(sqlite3_stmt* s, int i, std::optional<double> v) {
    if (v) sqlite3_bind_double(s, i, *v);
    else sqlite3_bind_null(s, i);
}

// ---- summary column binding, in the schema's declared order ---------------

void bind_summary(sqlite3_stmt* s, int first_idx, const PathSummary& sum) {
    if (sum.score) sqlite3_bind_int64(s, first_idx, *sum.score);
    else sqlite3_bind_null(s, first_idx);
    if (sum.actcount) sqlite3_bind_int(s, first_idx + 1, *sum.actcount);
    else sqlite3_bind_null(s, first_idx + 1);
    if (sum.maxskip) sqlite3_bind_int(s, first_idx + 2, *sum.maxskip);
    else sqlite3_bind_null(s, first_idx + 2);
    if (sum.hardest_ms) sqlite3_bind_double(s, first_idx + 3, *sum.hardest_ms);
    else sqlite3_bind_null(s, first_idx + 3);
    if (sum.avgmult) sqlite3_bind_double(s, first_idx + 4, *sum.avgmult);
    else sqlite3_bind_null(s, first_idx + 4);
    if (sum.notecount) sqlite3_bind_int(s, first_idx + 5, *sum.notecount);
    else sqlite3_bind_null(s, first_idx + 5);
    if (sum.sqin_count) sqlite3_bind_int(s, first_idx + 6, *sum.sqin_count);
    else sqlite3_bind_null(s, first_idx + 6);
    if (sum.sqout_count) sqlite3_bind_int(s, first_idx + 7, *sum.sqout_count);
    else sqlite3_bind_null(s, first_idx + 7);
    if (sum.pathcount) sqlite3_bind_int(s, first_idx + 8, *sum.pathcount);
    else sqlite3_bind_null(s, first_idx + 8);
    if (sum.stars) sqlite3_bind_int(s, first_idx + 9, *sum.stars);
    else sqlite3_bind_null(s, first_idx + 9);
}

PathSummary read_summary(sqlite3_stmt* s, int first_idx) {
    PathSummary sum;
    sum.score = column_opt_i64(s, first_idx);
    if (auto v = column_opt_i64(s, first_idx + 1)) sum.actcount = static_cast<int>(*v);
    if (auto v = column_opt_i64(s, first_idx + 2)) sum.maxskip = static_cast<int>(*v);
    sum.hardest_ms = column_opt_f64(s, first_idx + 3);
    sum.avgmult = column_opt_f64(s, first_idx + 4);
    if (auto v = column_opt_i64(s, first_idx + 5)) sum.notecount = static_cast<int>(*v);
    if (auto v = column_opt_i64(s, first_idx + 6)) sum.sqin_count = static_cast<int>(*v);
    if (auto v = column_opt_i64(s, first_idx + 7)) sum.sqout_count = static_cast<int>(*v);
    if (auto v = column_opt_i64(s, first_idx + 8)) sum.pathcount = static_cast<int>(*v);
    if (auto v = column_opt_i64(s, first_idx + 9)) sum.stars = static_cast<int>(*v);
    return sum;
}

// The column list_records sorts by, with its table alias: a summary column of
// the result (`r.`) or a name of the chart's naming copy (`c.`).
std::string sort_column_sql(SortColumn c) {
    switch (c) {
        case SortColumn::Score: return "r.score";
        case SortColumn::ActCount: return "r.actcount";
        case SortColumn::MaxSkip: return "r.maxskip";
        case SortColumn::HardestMs: return "r.hardest_ms";
        case SortColumn::AvgMult: return "r.avgmult";
        case SortColumn::NoteCount: return "r.notecount";
        case SortColumn::SqInCount: return "r.sqin_count";
        case SortColumn::SqOutCount: return "r.sqout_count";
        case SortColumn::PathCount: return "r.pathcount";
        case SortColumn::RefName: return "c.name";
        case SortColumn::RefArtist: return "c.artist";
        case SortColumn::RefCharter: return "c.charter";
    }
    return "r.score";
}

// The results table's columns. result_id is the rowid alias: a bigger one
// means "written later". rules_fp is the fingerprint of the rules the row was
// analyzed under (rules_fp_bytes), in the UNIQUE key: one row per chart,
// mode, cap, lens and rules, so a result made under other rules sits beside
// this build's (schema 4, D51 call 8).
constexpr const char* kResultsColumnDefs =
    "  result_id   INTEGER PRIMARY KEY,"
    "  hyhash      TEXT NOT NULL,"
    "  chartmode   TEXT NOT NULL,"
    "  hyversion   TEXT NOT NULL,"
    "  sp_cap      INTEGER NOT NULL,"
    "  ms_enabled  INTEGER NOT NULL,"
    "  ms_value    INTEGER NOT NULL,"
    "  depth_mode  INTEGER NOT NULL,"
    "  depth_value INTEGER NOT NULL,"
    "  legacy_fills INTEGER NOT NULL DEFAULT 0,"
    "  bestpath    TEXT NOT NULL,"
    "  score       INTEGER,"
    "  actcount    INTEGER,"
    "  maxskip     INTEGER,"
    "  hardest_ms  REAL,"
    "  avgmult     REAL,"
    "  notecount   INTEGER,"
    "  sqin_count  INTEGER,"
    "  sqout_count INTEGER,"
    "  pathcount   INTEGER,"
    "  stars       INTEGER,"
    "  rules_fp    BLOB NOT NULL,"
    "  UNIQUE (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode, depth_value,"
    "          legacy_fills, rules_fp)";

// Where an older file's structure blob held the rules fingerprint: after its
// 4-byte path format, 8 bytes. Only upgrade_results_key reads it, to fill the
// rules_fp column of a schema 2 or 3 file before the blob is dropped.
constexpr size_t kOldRulesFingerprintOffset = 4;
constexpr size_t kOldRulesFingerprintBytes = 8;

}  // namespace

// Schema 2's results table, declared in record_store.h. The column list is
// what upgrade_results_key copies across; the rebuild fills the other two,
// legacy_fills and rules_fp. The table text's only reader is the store test,
// which builds a schema 2 file from it.
const char* const kSchema2ResultsColumns =
    "result_id, hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value, depth_mode,"
    " depth_value, bestpath, score, actcount, maxskip, hardest_ms, avgmult,"
    " notecount, sqin_count, sqout_count, pathcount, stars";
const char* const kSchema2ResultsTableSql =
    "CREATE TABLE results ("
    "  result_id INTEGER PRIMARY KEY, hyhash TEXT NOT NULL,"
    "  chartmode TEXT NOT NULL, hyversion TEXT NOT NULL,"
    "  sp_cap INTEGER NOT NULL, ms_enabled INTEGER NOT NULL,"
    "  ms_value INTEGER NOT NULL, depth_mode INTEGER NOT NULL,"
    "  depth_value INTEGER NOT NULL, bestpath TEXT NOT NULL,"
    "  structure BLOB NOT NULL, score INTEGER, actcount INTEGER,"
    "  maxskip INTEGER, hardest_ms REAL, avgmult REAL, notecount INTEGER,"
    "  sqin_count INTEGER, sqout_count INTEGER, pathcount INTEGER,"
    "  stars INTEGER,"
    "  UNIQUE (hyhash, chartmode, sp_cap, ms_enabled, ms_value, depth_mode,"
    "          depth_value))";

namespace {

// The summary columns, in the order bind_summary/read_summary use.
//
// prepare_row writes all of them, and bestpath with them, from
// summarize_record. Nothing else writes them. A rule change that alters any
// of them bumps kResultsStamp (stored_versions.h), so every row written
// before it reads Stale and no old number is shown.
constexpr const char* kSummaryColumnList =
    "score, actcount, maxskip, hardest_ms, avgmult, notecount, sqin_count, "
    "sqout_count, pathcount, stars";

// How many names a comma-separated column list holds.
constexpr int count_list_names(const char* list) {
    int n = 1;
    for (const char* c = list; *c; ++c)
        if (*c == ',') ++n;
    return n;
}

// How many summary columns there are, counted from kSummaryColumnList so the
// two always agree. The INSERT and list_records work out their slots from it.
// bind_summary and read_summary walk the slots by hand, so the assert stops
// the build when the list grows and they don't.
constexpr int kSummaryColumnCount = count_list_names(kSummaryColumnList);
static_assert(kSummaryColumnCount == 10,
              "bind_summary and read_summary walk ten slots: grow them with the list");

// kSummaryColumnList with every name written as before + name + after,
// joined by ", ": "r.score, r.actcount, ...".
std::string summary_columns(const char* before, const char* after) {
    std::string out;
    std::string_view rest = kSummaryColumnList;
    while (!rest.empty()) {
        const size_t comma = rest.find(',');
        std::string_view name = rest.substr(0, comma);
        while (!name.empty() && name.front() == ' ') name.remove_prefix(1);
        if (!out.empty()) out += ", ";
        out += before;
        out += name;
        out += after;
        rest = comma == std::string_view::npos ? std::string_view{} : rest.substr(comma + 1);
    }
    return out;
}

// SQLite refuses a statement with more than 32,766 bound values (the vendored
// 3.46's SQLITE_MAX_VARIABLE_NUMBER). get_summaries sends a whole library's
// hashes, so it sends them this many at a time.
constexpr size_t kHashesPerQuery = 10000;

// ---- which row answers a lookup -------------------------------------------
//
// Every lookup answers one question: "which row ran under these settings?".
// The candidates are the rows with the wanted lens at the wanted cap. An
// older Hydra's migration left some rows with ms_enabled = -1, meaning "the
// settings are unknown". No lens has -1, so those rows are never candidates
// and read as no row at all (user decision 2026-09-26).

// "the row at alias `a` carries exactly this lens". Five bound parameters, in
// Lens's field order. `a` is "" or "r.".
std::string lens_match(const char* a) {
    std::string p = a;
    return "(" + p + "ms_enabled=? AND " + p + "ms_value=? AND " + p +
           "depth_mode=? AND " + p + "depth_value=? AND " + p + "legacy_fills=?)";
}
int bind_lens(sqlite3_stmt* s, int idx, const Lens& lens) {
    sqlite3_bind_int(s, idx, lens.ms_enabled);
    sqlite3_bind_int(s, idx + 1, lens.ms_value);
    sqlite3_bind_int(s, idx + 2, lens.depth_mode);
    sqlite3_bind_int(s, idx + 3, lens.depth_value);
    sqlite3_bind_int(s, idx + 4, lens.legacy_fills);
    return idx + 5;
}
// "the row at alias `a` was stored at the lookup's SP cap". One bound
// parameter, bound by bind_cap. `a` is "" or "r.".
std::string cap_match(const char* a) {
    return std::string(a) + "sp_cap=?";
}
int bind_cap(sqlite3_stmt* s, int idx, const CapQuery& cap) {
    sqlite3_bind_int(s, idx, cap.exact);
    return idx + 1;
}

// A rules fingerprint as the rules_fp column holds it.
std::vector<uint8_t> rules_fp_bytes(uint64_t fingerprint) {
    std::vector<uint8_t> bytes;
    core::append_le_u64(bytes, fingerprint);
    return bytes;
}

// The fingerprint a row must carry to be Ready in this process, as the
// rules_fp column holds it: rank_row's `fixed_fp`.
std::vector<uint8_t> ready_rules_fp(const core::RulesStamp& stamp) {
    return rules_fp_bytes(stamp.fixed);
}

// The facts that decide whether a row is readable and how it places among
// the candidates for its chart and mode, and why a row that is not ready is
// Stale. rank_row is the only place they are read off a row. Whether a chart
// has a current result under some settings is get_summaries' answer, from its
// winner's ready() (D79).
struct Candidate {
    bool current = false;     // stamped with this build's results version
    bool same_rules = false;  // analyzed under the rules this process runs
    int64_t result_id = 0;
    // Why a row that is not ready is Stale, for callers that explain it
    // (the song panel, the library row's tooltip). Both can be true; neither
    // is on a ready row.
    bool stale_build = false;  // another Hydra build
    bool stale_rules = false;  // analyzed under other rules
    // Readable: stamped with this results version, under these rules.
    // Anything else is Stale.
    bool ready() const { return current && same_rules; }
};

// `rules_fp` is the row's rules_fp column; `fixed_fp` is rules_fp_bytes of
// the fingerprint this process runs under.
Candidate rank_row(const std::string& hyversion, const std::vector<uint8_t>& rules_fp,
                   int64_t result_id, const std::vector<uint8_t>& fixed_fp) {
    Candidate c;
    c.current = kResultsStamp.is_current(hyversion);
    c.same_rules = rules_fp == fixed_fp;
    c.result_id = result_id;
    c.stale_build = !c.current;
    c.stale_rules = !c.same_rules;
    return c;
}

// `n` comma-separated "?" placeholders.
std::string placeholders(size_t n) {
    std::string out;
    for (size_t i = 0; i < n; ++i) out += i ? ", ?" : "?";
    return out;
}

// The rules fingerprint inside an older file's structure blob, spelled in
// SQL. `blob` is a column name. SQLite's substr counts bytes from 1, so it
// starts one past the 0-based offset.
std::string rules_fp_of(const char* blob) {
    return std::string("substr(") + blob + "," + std::to_string(kOldRulesFingerprintOffset + 1) +
           "," + std::to_string(kOldRulesFingerprintBytes) + ")";
}

// "This build can read the row at all", spelled in SQL for write_row's first
// purge, which deletes in the database the rows that fail it: this results
// version. It leaves out the rules, so a row made under other rules is kept
// (D51 call 8). Negate with "NOT ", never by spelling the opposite, so the
// rule has one SQL spelling. hyversion is NOT NULL, so NOT never meets a NULL.
//
// Built from the same StampRule list is_current reads, so the two spellings
// cannot drift. bind_readable_params binds it and returns the next free
// index.
const std::string& row_readable_sql() {
    static const std::string sql =
        "(hyversion IN (" + placeholders(kResultsStamp.accepted.size()) + "))";
    return sql;
}

int bind_readable_params(sqlite3_stmt* s, int idx) {
    for (std::string_view v : kResultsStamp.accepted) bind_text(s, idx++, std::string(v));
    return idx;
}

// Does `a` beat `b`? This version before another, then these rules before
// the rest, then the newest write. Write order is result_id: add_row deletes
// and re-inserts, so a rewritten row is newest. Since Auto went (2026-09-27)
// every lookup names one exact cap and lens. The results table holds one row
// per cap, lens and rules, so a chart offers one candidate per set of rules
// it was analyzed under, and the one made under this process's rules wins.
bool outranks(const Candidate& a, const Candidate& b) {
    if (a.current != b.current) return a.current;
    if (a.same_rules != b.same_rules) return a.same_rules;
    return a.result_id > b.result_id;
}

// Which chart a winner is picked for: one per chart and mode.
using GroupKey = std::pair<std::string, std::string>;

// The one owner of "which row wins". A lookup offers its candidate rows here
// in any order, then asks which offer won each chart and mode: get_summaries
// for one chart or many, list_records for a listing. Nothing else compares
// two rows.
class WinnerPicker {
public:
    // Offers one candidate. Its offer index is the number of earlier offers.
    void offer(const std::string& hyhash, const std::string& chartmode,
               const Candidate& rank) {
        const size_t index = ranks_.size();
        ranks_.push_back(rank);
        auto [it, inserted] = winner_.emplace(GroupKey{hyhash, chartmode}, index);
        if (!inserted && outranks(rank, ranks_[it->second])) it->second = index;
    }
    // One flag per offer: true for each chart and mode's winner.
    std::vector<bool> winners() const {
        std::vector<bool> won(ranks_.size(), false);
        for (const auto& kv : winner_) won[kv.second] = true;
        return won;
    }
    const Candidate& rank(size_t index) const { return ranks_[index]; }

private:
    std::vector<Candidate> ranks_;
    std::map<GroupKey, size_t> winner_;  // chart+mode -> index of its best offer
};

// Every row that could answer a lookup: the wanted lens at the wanted cap.
// Which of them wins is WinnerPicker's decision and not SQL's, so there is
// deliberately no ORDER BY or LIMIT here. `a` is the table alias, "" or "r.".
// Appended after a WHERE that already has a term.
void append_candidate_filter(std::string& sql, const char* a) {
    sql += " AND " + lens_match(a);
    sql += " AND " + cap_match(a);
}
// Binds the lens's five parameters, then the cap.
int bind_candidate_filter(sqlite3_stmt* s, int idx, const CapQuery& cap, const Lens& lens) {
    idx = bind_lens(s, idx, lens);
    return bind_cap(s, idx, cap);
}

// Whether a transaction is still open on this connection. Some failures (a
// full disk, an I/O error) make sqlite roll the transaction back by itself,
// so after a failure this is the one way to ask whether it still stands.
bool transaction_open(sqlite3* db) { return !sqlite3_get_autocommit(db); }

// Rolls back the open transaction, if there still is one. A second ROLLBACK
// after sqlite rolled back by itself (see transaction_open) would throw over
// the error that caused it.
void rollback_if_open(sqlite3* db) {
    if (transaction_open(db)) sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
}

// The failure every save in a lost group reports, and its COMMIT too.
KindedError group_lost_error() {
    return KindedError(ErrorKind::DatabaseWrite, "the save group's transaction was rolled back");
}

}  // namespace

// The bestpath column writes this text (prepare_row), and hydra_replay's
// result block shows it.
std::string best_path_text(const HydraRecord& record) {
    return record.paths.empty() ? std::string() : record.best_path().pathstring();
}

// ---- summarize_path / summarize_record / prepare_row -----------------------

PathSummary summarize_path(const Path& path) {
    PathSummary s;
    const ActivationWalk acts = path.walk_activations();

    s.score = path.totalscore();
    s.actcount = static_cast<int>(acts.size());

    int maxskip = 0;
    int sqin = 0, sqout = 0;
    for (const Activation& a : acts) {
        if (a.skips() > maxskip) maxskip = a.skips();
        for (const SPSqueeze& sq : a.sqinouts) {
            if (sq.kind == SqueezeKind::SqIn) ++sqin;
            else ++sqout;
        }
    }
    s.maxskip = maxskip;
    s.hardest_ms = path.difficulty();  // the path's own hardest squeeze or required fill
    s.avgmult = path.avg_mult();
    s.notecount = path.notecount;
    s.sqin_count = sqin;
    s.sqout_count = sqout;
    s.stars = path_stars(path);
    return s;
}

// A Ready record with no paths has no facts (D51 call 11): every field is
// unset and has_scored_best_path() is false. That flag is the one answer to
// "is there a scored best path"; in wave 3 LB's facts_of and query_matches
// and RP's leaderboard and fill pages read it instead of each checking a
// field of their own.
PathSummary summarize_record(const HydraRecord& record) {
    if (record.paths.empty()) return PathSummary{};

    PathSummary s = summarize_path(record.best_path());
    // Every kept path, roots and variants: the count the Paths tab shows.
    s.pathcount = static_cast<int>(record.all_paths().size());
    return s;
}

std::string current_record_version() { return std::string(kResultsStamp.written); }

PreparedRow prepare_row(const RecordKey& key, const HydraRecord& record) {
    if (!record.sp_cap)
        throw std::invalid_argument("prepare_row: record carries no sp_cap");
    if (key.cap.exact != *record.sp_cap)
        throw std::invalid_argument("prepare_row: key asks for sp_cap " +
                                    std::to_string(key.cap.exact) +
                                    " but the record was analyzed at " +
                                    std::to_string(*record.sp_cap));
    // Both sides come from the same int (Settings::mslimit_value, widened to
    // the engine's double), so an exact comparison is the right one.
    if (key.lens.ms_enabled == 1 &&
        (!record.ms_limit || *record.ms_limit != static_cast<double>(key.lens.ms_value)))
        throw std::invalid_argument(
            "prepare_row: key asks for an ms limit of " +
            std::to_string(key.lens.ms_value) + " but the record was analyzed " +
            (record.ms_limit ? "at " + std::to_string(*record.ms_limit) : "without one"));
    // And the other way: a record searched with a limit never files under a
    // lens that has the limit off (finding 130).
    if (key.lens.ms_enabled == 0 && record.ms_limit)
        throw std::invalid_argument(
            "prepare_row: key asks for no ms limit but the record was analyzed at " +
            std::to_string(*record.ms_limit));
    // The record's own fill rule as a lens stores it (Lens::from).
    const Lens record_fills = Lens::from(std::nullopt, 0, 0, record.legacy_fills);
    if (key.lens.legacy_fills != record_fills.legacy_fills)
        throw std::invalid_argument(
            std::string("prepare_row: key asks for Clone Hero ") +
            (key.lens.legacy_fills ? "1.0" : "1.1") + " fills but the record was analyzed "
            "under the " + (record.legacy_fills ? "1.0" : "1.1") + " rule");

    PreparedRow row;
    row.hyhash = key.hyhash;
    row.chartmode = key.chartmode;
    row.hyversion = current_record_version();
    row.sp_cap = *record.sp_cap;
    row.lens = key.lens;
    row.bestpath = best_path_text(record);
    row.rules_fp = record.rules_fingerprint;
    row.summary = summarize_record(record);
    return row;
}

int open_sqlite(const std::string& utf8_path, sqlite3** db, int flags) {
    // SQLite takes UTF-8 and understands the \\?\ prefix, so a long database
    // path goes through the same one conversion as every other file.
    // "win32-longpath" is SQLite's own Windows layer with its path buffer
    // raised from 260 characters to 32,767; it is otherwise the default one.
    return sqlite3_open_v2(wide_to_utf8(win32_path(utf8_path)).c_str(), db, flags,
                           "win32-longpath");
}

// ---- RecordStore ------------------------------------------------------

RecordStore::RecordStore(const std::string& dbpath, core::RulesStamp rules_fingerprint,
                         OpenProgressFn progress)
    : rules_fingerprint_(rules_fingerprint), progress_(std::move(progress)) {
    // Whatever happens below, the callback is not kept past the constructor.
    struct ForgetProgress {
        OpenProgressFn& fn;
        ~ForgetProgress() { fn = nullptr; }
    } forget{progress_};

    report(OpenStep::Opening);
    if (open_sqlite(dbpath, &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
        close();
        throw KindedError(ErrorKind::DatabaseOpen,
                          "failed to open database '" + dbpath + "': " + msg);
    }

    // SQLite notices a locked file, or one that isn't a database, only at
    // the first statement below. Any throw from here on is still a failed
    // open (D72 item 2), with the raw text kept. A constructor that throws
    // never runs the destructor, so the handle is closed here. Running out of
    // memory keeps its own type, which plain_error answers by. A stop the
    // callback asked for, and a failed upgrade that left the old file whole,
    // keep their own kinds too.
    try {
        set_up_schema();
    } catch (const std::bad_alloc&) {
        close();
        throw;
    } catch (const KindedError& e) {
        close();
        if (e.kind() == ErrorKind::Cancelled || e.kind() == ErrorKind::DatabaseUpgrade) throw;
        throw KindedError(ErrorKind::DatabaseOpen, e.what());
    } catch (const std::exception& e) {
        close();
        throw KindedError(ErrorKind::DatabaseOpen, e.what());
    } catch (...) {
        close();
        throw;
    }
}

void RecordStore::set_up_schema() {
    // WAL journal mode (orchestrator's call, 2026-09-26 audit plan): a commit
    // appends to hydra.db-wal instead of rewriting pages in place, and with
    // synchronous=NORMAL it syncs to disk only at checkpoints, not on every
    // commit. A power cut can lose the last few commits but never corrupts
    // the file, and readers stop blocking the writer
    // (https://www.sqlite.org/wal.html). journal_mode is stored in the file;
    // synchronous and journal_size_limit are per connection, so all three are
    // set on every open. A ":memory:" store answers "memory" and is
    // unaffected.
    exec("PRAGMA journal_mode=WAL");
    exec("PRAGMA synchronous=NORMAL");
    // SQLite truncates the WAL file to this size when a full checkpoint
    // resets the log (D93).
    exec(("PRAGMA journal_size_limit=" + std::to_string(kJournalSizeLimitBytes)).c_str());

    exec(
        "CREATE TABLE IF NOT EXISTS charts ("
        "  md5    TEXT,"
        "  name   TEXT,"
        "  artist TEXT,"
        "  charter TEXT,"
        "  path   TEXT,"
        "  folder TEXT,"
        "  sig    TEXT,"
        "  stated_length_ms REAL,"
        "  delay_ms REAL"
        ");"
        "CREATE TABLE IF NOT EXISTS meta ("
        "  key   TEXT PRIMARY KEY,"
        "  value TEXT"
        ");");
    // A charts table from before the rescan cache has no sig column.
    // rebuild_chart_library empties the table rather than recreating it, so
    // the column is added here, once.
    if (!has_column("charts", "sig")) exec("ALTER TABLE charts ADD COLUMN sig TEXT");
    // A charts table from before the scan read each chart's stated length and
    // delay (D75). Old rows read NULL, and list_chart_library hands them out
    // with no timing until a scan under the current kChartMetaStamp rewrites
    // them.
    if (!has_column("charts", "stated_length_ms"))
        exec("ALTER TABLE charts ADD COLUMN stated_length_ms REAL");
    if (!has_column("charts", "delay_ms")) exec("ALTER TABLE charts ADD COLUMN delay_ms REAL");
    // Each difficulty's own last-note length (D51 call 9), which D69
    // replaced with one length per song. Nothing reads it.
    exec("DROP TABLE IF EXISTS songlength");
    // The library page sorts by name (list_chart_library's ORDER BY name).
    exec("CREATE INDEX IF NOT EXISTS charts_by_name ON charts (name)");
    // The scan's purge, list_records and list_chart_library_copies find a
    // chart's library rows by md5 (D76).
    exec("CREATE INDEX IF NOT EXISTS charts_by_md5 ON charts (md5)");
    // Schema 2 = results keyed by the full settings. A database from Hydra
    // 1.6 or older still holds its old `records` table. Nothing reads it
    // (user decision 2026-09-26), so its charts read Not analyzed until they
    // are analyzed again.
    create_result_tables();
    // A results table from before the stars summary has no column for it.
    // Its rows read NULL there, and the summary-only upgrade below deletes
    // the ones it can't fill. Added before the schema 3 rebuild, which copies
    // the column across.
    if (!has_column("results", "stars")) exec("ALTER TABLE results ADD COLUMN stars INTEGER");
    // Schema 3 = the fill rule joins a result's key; schema 4 = the rules
    // fingerprint does.
    upgrade_results_key();
    // Auto was removed (2026-09-27). Its results go the first time this
    // build opens the file.
    delete_auto_results();
    // Last, so the steps above read an older file in its own layout.
    drop_stored_details();
}

void RecordStore::upgrade_results_key() {
    // What the rebuilt table's legacy_fills column is filled from.
    std::string legacy_fills;
    if (!has_column("results", "legacy_fills")) {
        // Schema 2: every row ran under the rule the file's stamp names
        // (stamped_fill_rule), or 1.1 when it names none.
        legacy_fills = stamped_fill_rule() == FillDeadlineRule::Ch10 ? "1" : "0";
    } else if (!has_column("results", "rules_fp")) {
        // Schema 3: each row keeps its own fill rule.
        legacy_fills = "legacy_fills";
    } else {
        return;  // schema 4 already
    }
    report(OpenStep::UpdatingResultsKey);
    exec("BEGIN");
    try {
        exec("ALTER TABLE results RENAME TO results_before_upgrade");
        create_result_tables();
        exec((std::string("INSERT INTO results (") + kSchema2ResultsColumns +
              ", legacy_fills, rules_fp) SELECT " + kSchema2ResultsColumns + ", " +
              legacy_fills + ", " + rules_fp_of("structure") + " FROM results_before_upgrade")
                 .c_str());
        exec("DROP TABLE results_before_upgrade");
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

void RecordStore::drop_stored_details() {
    if (!has_table("paths")) return;
    // (1) One transaction. A row with a score and no stars was written before
    //     the stars column existed; filling it needed the stored paths, so it
    //     goes, and a click or a batch writes it again. Then the details.
    exec("BEGIN");
    try {
        exec("DELETE FROM results WHERE stars IS NULL AND score IS NOT NULL");
        exec("DROP TABLE IF EXISTS paths;"
             "DROP TABLE IF EXISTS path_refs;"
             "DROP TABLE IF EXISTS songmeta;"
             "DROP TABLE IF EXISTS dynamics;");
        if (has_column("results", "structure")) exec("ALTER TABLE results DROP COLUMN structure");
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
    // (2) VACUUM refuses to run inside a transaction, so it runs after the
    //     commit. It rewrites the file without the freed pages, and needs
    //     free disk space up to the file's size.
    exec("VACUUM");
    // (3) The rewrite went through the WAL; fold it into the file and empty
    //     the log.
    exec("PRAGMA wal_checkpoint(TRUNCATE)");
}

void RecordStore::report(OpenStep step, int64_t rows_done, int64_t rows_total) {
    if (!progress_) return;
    OpenProgress p;
    p.step = step;
    p.rows_done = rows_done;
    p.rows_total = rows_total;
    if (!progress_(p))
        throw KindedError(ErrorKind::Cancelled, "opening the database was stopped");
}

RecordStore::~RecordStore() { close(); }

void RecordStore::close() {
    // A statement still compiled keeps the connection from closing.
    for (auto& [sql, p] : stmt_cache_) sqlite3_finalize(p);
    stmt_cache_.clear();
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void RecordStore::exec(const char* sql) {
    char* errmsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string msg = errmsg ? errmsg : "unknown error";
        sqlite3_free(errmsg);
        throw KindedError(ErrorKind::DatabaseWrite, "sqlite exec failed: " + msg);
    }
}

void RecordStore::ctl(const char* sql) {
    // sqlite3_stmt_readonly calls transaction control read-only, so it is
    // compiled as a read; a failed step is a failed write, worded as exec's.
    CachedStmt s = use_read(db_, stmt_cache_, sql);
    step_done(s, "sqlite exec");
}

void RecordStore::begin_save_group() {
    mutex_.lock();
    try {
        ctl("BEGIN");
    } catch (...) {
        mutex_.unlock();
        throw;
    }
    group_open_ = true;
    group_lost_ = false;
}

void RecordStore::commit_save_group() {
    if (!group_open_) return;
    group_open_ = false;
    try {
        if (group_lost_) throw group_lost_error();
        if (fail_next_group_commit_) {
            fail_next_group_commit_ = false;
            throw KindedError(ErrorKind::DatabaseWrite,
                              "sqlite exec failed: the test seam failed this COMMIT");
        }
        ctl("COMMIT");
    } catch (...) {
        group_lost_ = false;
        rollback_if_open(db_);
        mutex_.unlock();
        throw;
    }
    mutex_.unlock();
}

RecordStore::BatchWrites::BatchWrites(RecordStore& store) : store_(store) {
    std::lock_guard<std::recursive_mutex> lock(store_.mutex_);
    store_.exec(("PRAGMA wal_autocheckpoint=" + std::to_string(kBatchWalAutocheckpointPages))
                    .c_str());
}

RecordStore::BatchWrites::~BatchWrites() {
    // Every result is already committed, so a checkpoint that cannot finish
    // (another connection still reading) loses nothing: SQLite folds the rest
    // of the log in at a later checkpoint. Nothing here throws out of a
    // destructor.
    std::lock_guard<std::recursive_mutex> lock(store_.mutex_);
    sqlite3_exec(store_.db_, "PRAGMA wal_checkpoint(TRUNCATE)", nullptr, nullptr, nullptr);
    // SQLite's own default threshold (D86 item 3).
    sqlite3_exec(store_.db_, "PRAGMA wal_autocheckpoint=1000", nullptr, nullptr, nullptr);
}

int RecordStore::wal_autocheckpoint_for_test() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_, "PRAGMA wal_autocheckpoint");
    return step_row(s) ? sqlite3_column_int(s, 0) : -1;
}

int64_t RecordStore::journal_size_limit_for_test() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_, "PRAGMA journal_size_limit");
    return step_row(s) ? sqlite3_column_int64(s, 0) : -1;
}

bool RecordStore::has_column(const char* table, const char* column) {
    std::string sql = std::string("PRAGMA table_info(") + table + ")";
    Stmt info = prepare_read(db_, sql.c_str());
    while (step_row(info))
        if (column_text(info, 1) == column) return true;
    return false;
}

bool RecordStore::has_table(const char* table) {
    Stmt s = prepare_read(db_, "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?");
    bind_text(s, 1, table);
    return step_row(s);
}

std::optional<std::string> RecordStore::meta_get(const std::string& key) {
    Stmt s = prepare_read(db_, "SELECT value FROM meta WHERE key=?");
    bind_text(s, 1, key);
    if (!step_row(s)) return std::nullopt;
    return column_text(s, 0);
}

void RecordStore::meta_set(const std::string& key, const std::string& value) {
    Stmt s = prepare_write(db_, "INSERT OR REPLACE INTO meta (key, value) VALUES (?,?)");
    bind_text(s, 1, key);
    bind_text(s, 2, value);
    step_done(s, "meta_set");
}

// The meta table's keys, each typed once.
//
// The meta row that holds the fill rule a file's results ran under, read by
// stamped_fill_rule through engine_mode.
constexpr const char* kEngineModeKey = "engine_mode";
// The meta row that holds the charts table's kChartMetaStamp, written once
// per file by rebuild_chart_library and checked by chart_library_cache.
constexpr const char* kChartMetaKey = "chart_meta_version";

std::optional<std::string> RecordStore::engine_mode() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return meta_get(kEngineModeKey);
}

void RecordStore::set_engine_mode(const std::string& mode) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    meta_set(kEngineModeKey, mode);
}

std::optional<FillDeadlineRule> RecordStore::stamped_fill_rule() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (const std::optional<std::string> mode = engine_mode())
        return fill_rule_from_stamp(*mode);
    // No stamp. Results written without one ran under the normal rule.
    Stmt any = prepare_read(db_, "SELECT 1 FROM results LIMIT 1");
    if (step_row(any)) return FillDeadlineRule::Ch11;
    return std::nullopt;
}

void RecordStore::save_analysis(const PreparedRow& row) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // Inside a save group this chart is one SAVEPOINT of the group's
    // transaction; otherwise it is a transaction of its own.
    const bool grouped = group_open_;
    if (grouped) {
        if (group_lost_) throw group_lost_error();
        ctl("SAVEPOINT chart");
    } else {
        ctl("BEGIN");
    }
    try {
        write_row(row);
        ctl(grouped ? "RELEASE chart" : "COMMIT");
    } catch (...) {
        if (!grouped) {
            rollback_if_open(db_);
        } else if (transaction_open(db_)) {
            sqlite3_exec(db_, "ROLLBACK TO chart", nullptr, nullptr, nullptr);
            sqlite3_exec(db_, "RELEASE chart", nullptr, nullptr, nullptr);
        } else {
            // SQLite rolled the whole group back itself: the group is lost,
            // not just this chart.
            group_lost_ = true;
        }
        throw;
    }
}

void RecordStore::create_result_tables() {
    exec((std::string("CREATE TABLE IF NOT EXISTS results (") + kResultsColumnDefs + ");")
             .c_str());
}

void RecordStore::add_record(const RecordKey& key, const HydraRecord& record) {
    add_row(prepare_row(key, record));
}

void RecordStore::add_row(const PreparedRow& row) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    exec("BEGIN");
    try {
        write_row(row);
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

void RecordStore::delete_results_where(const std::string& where,
                                       const std::function<void(sqlite3_stmt*)>& bind,
                                       const std::string& what) {
    CachedStmt d = use_write(db_, stmt_cache_, "DELETE FROM results WHERE " + where);
    bind(d);
    step_done(d, what);
}

void RecordStore::write_row(const PreparedRow& row) {
    // The caller holds the lock and an open transaction, so a failure
    // anywhere leaves the store exactly as it was.
    auto purge = [&](const std::string& where,
                     const std::function<void(sqlite3_stmt*)>& bind, const char* what) {
        delete_results_where(where, bind, std::string("add_row ") + what);
    };
    const std::vector<uint8_t> rules_fp = rules_fp_bytes(row.rules_fp);

    // (1) Anything this chart+mode holds that this build can never read --
    //     another Hydra version's stamp, which includes every row an old
    //     migration left -- is superseded by a write here. A result analyzed
    //     under other rules is not: it reads Ready again once the rules match
    //     (D51 call 8). The test is against what is current, not against this
    //     row: a test writing a deliberately old-stamped row must not take
    //     the real rows with it, and this runs before the insert so the new
    //     row is untouched.
    purge("hyhash=? AND chartmode=? AND NOT " + row_readable_sql(),
          [&](sqlite3_stmt* s) {
              bind_text(s, 1, row.hyhash);
              bind_text(s, 2, row.chartmode);
              bind_readable_params(s, 3);
          },
          "unreadable purge");

    // (2) The row this one replaces: the same key under the same rules, the
    //     row the UNIQUE key would refuse a second copy of. Deleted
    //     explicitly rather than by INSERT OR REPLACE, so the re-insert takes
    //     a fresh result_id and the newest write ranks first.
    purge("hyhash=? AND chartmode=? AND sp_cap=? AND " + lens_match("") + " AND rules_fp = ?",
          [&](sqlite3_stmt* s) {
              bind_text(s, 1, row.hyhash);
              bind_text(s, 2, row.chartmode);
              sqlite3_bind_int(s, 3, row.sp_cap);
              const int next = bind_lens(s, 4, row.lens);
              bind_blob(s, next, rules_fp);
          },
          "replace purge");

    // (3) The result.
    {
        // The row's own columns; the summary columns follow, then rules_fp.
        static constexpr const char* kRowColumns =
            "hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value, depth_mode,"
            " depth_value, legacy_fills, bestpath";
        static constexpr int kRowColumnCount = count_list_names(kRowColumns);
        CachedStmt s = use_write(db_, stmt_cache_,
            std::string("INSERT INTO results (") + kRowColumns + ", " + kSummaryColumnList +
             ", rules_fp) VALUES (" + placeholders(kRowColumnCount) + ", " +
             placeholders(kSummaryColumnCount) + ", ?)");
        bind_text(s, 1, row.hyhash);
        bind_text(s, 2, row.chartmode);
        bind_text(s, 3, row.hyversion);
        sqlite3_bind_int(s, 4, row.sp_cap);
        bind_lens(s, 5, row.lens);
        bind_text(s, kRowColumnCount, row.bestpath);
        bind_summary(s, kRowColumnCount + 1, row.summary);
        bind_blob(s, kRowColumnCount + kSummaryColumnCount + 1, rules_fp);
        step_done(s, "add_row insert");
    }
}

namespace {
// The meta key that marks the Auto results deleted for this file.
constexpr const char* kAutoResultsDeletedKey = "auto_results_deleted";

// "No library row lists this chart", spelled once in SQL for
// purge_charts_not_in_library. `hash` is the column holding the chart's hash
// in the table the statement reads.
std::string not_in_library(const char* hash) {
    return std::string("NOT EXISTS (SELECT 1 FROM charts WHERE charts.md5 = ") + hash + ")";
}
}  // namespace

void RecordStore::delete_results_without_chart() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    exec("BEGIN");
    try {
        purge_charts_not_in_library("delete_results_without_chart");
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

void RecordStore::reidentify_chart(const std::string& notespath, const std::string& new_md5,
                                   const std::string& new_sig) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    exec("BEGIN");
    try {
        {
            CachedStmt s =
                use_write(db_, stmt_cache_, "UPDATE charts SET md5 = ?, sig = ? WHERE path = ?");
            bind_text(s, 1, new_md5);
            bind_text(s, 2, new_sig);
            bind_text(s, 3, notespath);
            step_done(s, "reidentify_chart");
        }
        purge_charts_not_in_library("reidentify_chart");
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

void RecordStore::purge_charts_not_in_library(const char* caller) {
    delete_results_where(not_in_library("results.hyhash"), [](sqlite3_stmt*) {},
                         std::string(caller) + " purge");
}

void RecordStore::delete_auto_results() {
    // Under RulesStamp::none() (a bad hydra_rules.ini) there is no
    // fingerprint to look for. Leave the key unset, so the next start with
    // good rules does it.
    if (rules_fingerprint_.retired_auto == core::kNoRulesFingerprint) return;
    if (meta_get(kAutoResultsDeletedKey)) return;

    // An Auto row is known only by its rules fingerprint. Auto rows made
    // under other rules are already Stale, and stay so: no rules' own
    // fingerprint is ever an Auto one. A new result for the same chart, cap
    // and lens outranks such a row (WinnerPicker).
    const std::vector<uint8_t> auto_fp = rules_fp_bytes(rules_fingerprint_.retired_auto);
    exec("BEGIN");
    try {
        delete_results_where(
            "rules_fp = ?", [&](sqlite3_stmt* s) { bind_blob(s, 1, auto_fp); },
            "deleting Auto results");
        meta_set(kAutoResultsDeletedKey, "1");
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

SummaryLookup RecordStore::get_summary(const RecordKey& key) {
    return get_summaries({key.hyhash}, key.chartmode, key.cap, key.lens).front();
}

std::vector<SummaryLookup> RecordStore::get_summaries(const std::vector<std::string>& hyhashes,
                                                      const std::string& chartmode,
                                                      const CapQuery& cap, const Lens& lens) {
    std::vector<SummaryLookup> out(hyhashes.size());
    if (hyhashes.empty()) return out;

    // Each chart is asked about once, however many folders list it. A chart's
    // rows all come back in its own chunk, so one picker sees every
    // candidate a chart has.
    std::vector<std::string> distinct(hyhashes);
    std::sort(distinct.begin(), distinct.end());
    distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());

    struct Offered {
        std::string hyhash;
        std::string bestpath;
        PathSummary summary;
    };
    // The lookup's own columns, read below at 0 onwards; the summary columns
    // follow, then the rules column the ranking reads.
    static constexpr const char* kLeadColumns = "hyhash, hyversion, bestpath, result_id";
    static constexpr int kFirstSummary = count_list_names(kLeadColumns);
    static constexpr int kAfterSummary = kFirstSummary + kSummaryColumnCount;
    const std::vector<uint8_t> fixed_fp = ready_rules_fp(rules_fingerprint_);
    WinnerPicker picker;
    std::vector<Offered> offered;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (size_t first = 0; first < distinct.size(); first += kHashesPerQuery) {
            const size_t n = std::min(kHashesPerQuery, distinct.size() - first);
            std::string sql = std::string("SELECT ") + kLeadColumns + ", " + kSummaryColumnList +
                              ", rules_fp FROM results WHERE chartmode=? AND hyhash IN (" +
                              placeholders(n) + ")";
            append_candidate_filter(sql, "");
            Stmt s = prepare_read(db_, sql.c_str());
            int idx = 1;
            bind_text(s, idx++, chartmode);
            for (size_t i = 0; i < n; ++i) bind_text(s, idx++, distinct[first + i]);
            bind_candidate_filter(s, idx, cap, lens);
            while (step_row(s)) {
                std::string hyhash = column_text(s, 0);
                picker.offer(hyhash, chartmode,
                             rank_row(column_text(s, 1), column_blob(s, kAfterSummary),
                                      sqlite3_column_int64(s, 3), fixed_fp));
                offered.push_back(
                    {std::move(hyhash), column_text(s, 2), read_summary(s, kFirstSummary)});
            }
        }
    }

    // One answer per chart, then handed to every position that asked for it.
    // A stale winner is reported as Stale, not hidden: the library has to
    // tell "analyzed by another build" apart from "never analyzed".
    const std::vector<bool> won = picker.winners();
    std::unordered_map<std::string, SummaryLookup> by_hash;
    for (size_t i = 0; i < offered.size(); ++i) {
        if (!won[i]) continue;
        SummaryLookup& answer = by_hash[offered[i].hyhash];
        const Candidate& rank = picker.rank(i);
        if (rank.ready()) {
            answer.status = RecordStatus::Ready;
            answer.bestpath = std::move(offered[i].bestpath);
            answer.summary = offered[i].summary;
        } else {
            answer.status = RecordStatus::Stale;
            answer.stale_build = rank.stale_build;
            answer.stale_rules = rank.stale_rules;
        }
    }
    for (size_t i = 0; i < hyhashes.size(); ++i) {
        auto it = by_hash.find(hyhashes[i]);
        if (it != by_hash.end()) out[i] = it->second;
    }
    return out;
}

bool RecordStore::has_record(const RecordKey& key) {
    // get_summaries owns "this chart has a current result under these
    // settings" (D79): the library's Analyzed chip reads the same answer.
    return get_summary(key).status == RecordStatus::Ready;
}

std::unordered_set<std::string> RecordStore::analyzed_hashes(const std::string& chartmode,
                                                             const CapQuery& cap,
                                                             const Lens& lens) {
    // Every chart with a candidate row under these settings, then
    // get_summaries' answer for each (D79), so the batch skips exactly the
    // charts the library's Analyzed chip counts. One lock over both reads, so
    // no write lands between them.
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<std::string> candidates;
    {
        std::string sql = "SELECT DISTINCT hyhash FROM results WHERE chartmode=?";
        append_candidate_filter(sql, "");
        Stmt s = prepare_read(db_, sql.c_str());
        bind_text(s, 1, chartmode);
        bind_candidate_filter(s, 2, cap, lens);
        while (step_row(s)) candidates.push_back(column_text(s, 0));
    }

    const std::vector<SummaryLookup> found = get_summaries(candidates, chartmode, cap, lens);
    std::unordered_set<std::string> out;
    for (size_t i = 0; i < candidates.size(); ++i)
        if (found[i].status == RecordStatus::Ready) out.insert(candidates[i]);
    return out;
}

std::unordered_map<std::string, int> RecordStore::library_copies() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    const std::string sql = std::string("SELECT md5, copies FROM ") + kNamingCopiesSql;
    Stmt s = prepare_read(db_, sql.c_str());
    std::unordered_map<std::string, int> out;
    while (step_row(s)) out.emplace(column_text(s, 0), sqlite3_column_int(s, 1));
    return out;
}

std::unordered_map<std::string, std::string> RecordStore::naming_copy_paths() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // Joins the naming copy kNamingCopiesSql picks back to its own row.
    const std::string sql = std::string("SELECT c.md5, p.path FROM ") + kNamingCopiesSql +
                            " AS c JOIN charts AS p ON p.rowid = c.naming_rowid";
    Stmt s = prepare_read(db_, sql.c_str());
    std::unordered_map<std::string, std::string> out;
    while (step_row(s)) out.emplace(column_text(s, 0), column_text(s, 1));
    return out;
}

int RecordStore::copies_of(const std::unordered_map<std::string, int>& copies,
                           const std::string& md5) {
    const auto listed = copies.find(md5);
    return listed == copies.end() ? 1 : listed->second;
}

std::vector<RecordListing> RecordStore::list_records(
    const std::optional<std::string>& chartmode, const CapQuery& cap, const Lens& lens,
    SortColumn order_by, bool descending, std::optional<int> limit) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // The listing's own columns, read below at 0 onwards; the summary columns
    // follow, then the four the ranking reads. A result whose chart the
    // library doesn't list (hydra_batch with folder arguments) is still
    // listed, with no names (D77).
    static constexpr const char* kLeadColumns =
        "r.hyhash, c.name, c.artist, c.charter, r.chartmode, r.bestpath";
    static constexpr int kFirstSummary = count_list_names(kLeadColumns);
    static constexpr int kAfterSummary = kFirstSummary + kSummaryColumnCount;
    std::string sql = std::string("SELECT ") + kLeadColumns + ", " +
                      summary_columns("r.", "") + ", r.sp_cap, r.hyversion, r.result_id, " +
                      "r.rules_fp FROM results r LEFT JOIN " + kNamingCopiesSql +
                      " AS c ON c.md5 = r.hyhash WHERE 1=1";
    if (chartmode) sql += " AND r.chartmode = ?";
    append_candidate_filter(sql, "r.");

    // The sort stays in SQL, so the listing keeps sqlite's own ordering; the
    // passes below only drop rows, never reorder them. The limit cannot stay
    // here: a SQL LIMIT would count rows that are about to be dropped and hand
    // back fewer than the caller asked for.
    sql += " ORDER BY " + sort_column_sql(order_by) + (descending ? " DESC" : " ASC");

    Stmt s = prepare_read(db_, sql.c_str());
    int idx = 1;
    if (chartmode) bind_text(s, idx++, *chartmode);
    bind_candidate_filter(s, idx, cap, lens);

    const std::vector<uint8_t> fixed_fp = ready_rules_fp(rules_fingerprint_);
    std::vector<RecordListing> candidates;  // by offer index
    WinnerPicker picker;
    while (step_row(s)) {
        RecordListing listing;
        listing.hyhash = column_text(s, 0);
        listing.ref_name = column_text(s, 1);
        listing.ref_artist = column_text(s, 2);
        listing.ref_charter = column_text(s, 3);
        listing.chartmode = column_text(s, 4);
        listing.bestpath = column_text(s, 5);
        listing.summary = read_summary(s, kFirstSummary);
        listing.sp_cap = sqlite3_column_int(s, kAfterSummary);
        picker.offer(listing.hyhash, listing.chartmode,
                     rank_row(column_text(s, kAfterSummary + 1), column_blob(s, kAfterSummary + 3),
                              sqlite3_column_int64(s, kAfterSummary + 2), fixed_fp));
        candidates.push_back(std::move(listing));
    }

    // A listing shows only what this build can read. A stale winner takes its
    // chart out of the listing rather than handing the place to the next
    // candidate -- a chart whose answer nobody can read must read the same as
    // a chart nobody has analyzed. A negative limit means no limit, matching
    // sqlite's own LIMIT convention.
    const std::vector<bool> keep = picker.winners();
    std::vector<RecordListing> out;
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (!keep[i] || !picker.rank(i).ready()) continue;
        if (limit && *limit >= 0 && out.size() >= static_cast<size_t>(*limit)) break;
        out.push_back(std::move(candidates[i]));
    }
    return out;
}

std::pair<int64_t, int64_t> RecordStore::counts() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_, "SELECT COUNT(DISTINCT hyhash), COUNT(*) FROM results");
    if (!step_row(s)) return {0, 0};
    return {sqlite3_column_int64(s, 0), sqlite3_column_int64(s, 1)};
}

// ---- chart library ------------------------------------------------------

void RecordStore::rebuild_chart_library(const std::vector<ChartLibraryEntry>& items) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // One transaction around the whole swap, opened before anything changes:
    // a failure anywhere rolls back to the previous scan's rows, which are
    // also the rescan cache. The table is emptied rather than dropped, so its
    // columns and index stay as they are.
    exec("BEGIN");
    try {
        exec("DELETE FROM charts");
        Stmt s = prepare_write(db_,
            "INSERT INTO charts (md5, name, artist, charter, path, folder, sig,"
            " stated_length_ms, delay_ms) VALUES (?,?,?,?,?,?,?,?,?)");
        for (const ChartLibraryEntry& item : items) {
            sqlite3_reset(s);
            bind_text(s, 1, item.md5);
            bind_text(s, 2, item.title);
            bind_text(s, 3, item.artist);
            bind_text(s, 4, item.charter);
            bind_text(s, 5, item.notespath);
            bind_text(s, 6, item.rootfolder);
            bind_text(s, 7, item.sig);
            const ChartTimingMeta timing = item.timing.value_or(ChartTimingMeta{});
            bind_opt_f64(s, 8, timing.length_ms);
            bind_opt_f64(s, 9, timing.delay_ms);
            step_done(s, "rebuild_chart_library");
        }
        // What this scan no longer lists goes, in this transaction, so a
        // scan that fails deletes nothing (D87 item 4).
        purge_charts_not_in_library("rebuild_chart_library");
        // The readers that filled these rows answer for them (kChartMetaStamp).
        meta_set(kChartMetaKey, std::to_string(kChartMetaStamp.written));
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

ChartLibraryCache RecordStore::chart_library_cache() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ChartLibraryCache cache;
    // Rows the current readers didn't vouch for are no cache at all: a file
    // from before the stamp, or one an older reader filled (D51 call 12).
    // The next scan reads every chart once and stamps what it writes.
    if (!chart_meta_current()) return cache;

    Stmt s = prepare_read(db_,
                          "SELECT path, sig, md5, name, artist, charter, stated_length_ms, delay_ms"
                          " FROM charts");
    while (step_row(s)) {
        std::string sig = column_text(s, 1);
        if (!sig_can_show_unchanged(sig)) continue;
        cache[column_text(s, 0)] = {std::move(sig),      column_text(s, 2), column_text(s, 3),
                                    column_text(s, 4),   column_text(s, 5),
                                    {column_opt_f64(s, 6), column_opt_f64(s, 7)}};
    }
    return cache;
}

bool RecordStore::chart_meta_current() {
    const std::optional<std::string> stamp = meta_get(kChartMetaKey);
    return stamp && kChartMetaStamp.is_current(std::atoi(stamp->c_str()));
}

int64_t RecordStore::chart_library_count() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    Stmt s = prepare_read(db_, "SELECT COUNT(*) FROM charts");
    if (!step_row(s)) return 0;
    return sqlite3_column_int64(s, 0);
}

namespace {

// The columns a library entry is read from, in read_library_entries' order.
constexpr const char* kLibraryEntrySelect =
    "SELECT md5, name, artist, charter, path, folder, sig, stated_length_ms, delay_ms FROM charts";

// Every row `s` (a kLibraryEntrySelect query) steps to. Rows an older scan
// wrote never read their timing: none, not "none stated"; `timing_read`
// (RecordStore::chart_meta_current) says which rows those are.
std::vector<ChartLibraryEntry> read_library_entries(sqlite3_stmt* s, bool timing_read) {
    std::vector<ChartLibraryEntry> out;
    while (step_row(s)) {
        ChartLibraryEntry e;
        e.md5 = column_text(s, 0);
        e.title = column_text(s, 1);
        e.artist = column_text(s, 2);
        e.charter = column_text(s, 3);
        e.notespath = column_text(s, 4);
        e.rootfolder = column_text(s, 5);
        e.sig = column_text(s, 6);
        if (timing_read) e.timing = ChartTimingMeta{column_opt_f64(s, 7), column_opt_f64(s, 8)};
        out.push_back(std::move(e));
    }
    return out;
}

}  // namespace

std::vector<ChartLibraryEntry> RecordStore::list_chart_library(int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(
        db_, (std::string(kLibraryEntrySelect) + " ORDER BY name LIMIT ? OFFSET ?").c_str());
    sqlite3_bind_int(s, 1, limit);
    sqlite3_bind_int(s, 2, offset);
    return read_library_entries(s, timing_read);
}

std::vector<ChartLibraryEntry> RecordStore::list_chart_library_copies(const std::string& md5) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(db_, (std::string(kLibraryEntrySelect) + " WHERE md5 = ?").c_str());
    bind_text(s, 1, md5);
    return read_library_entries(s, timing_read);
}

}  // namespace hydra::store
