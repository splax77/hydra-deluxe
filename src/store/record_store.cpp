#include "store/record_store.h"

#include <sqlite3.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdlib>
#include <map>
#include <new>
#include <stdexcept>
#include <string_view>

#include "core/error_kind.h"
#include "core/little_endian.h"
#include "core/stars.h"
#include "core/winstr.h"
#include "store/chart_index.h"
#include "store/stored_versions.h"
#include "store/upgrade_files.h"

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
    // An empty vector's data() may be null, and SQLite binds a null pointer
    // as NULL. An old file's rules_fp can be the empty blob (cut from a short
    // structure blob by a release before 2.2.0), which must stay a blob, not
    // become NULL.
    if (v.empty()) sqlite3_bind_zeroblob(s, i, 0);
    else sqlite3_bind_blob(s, i, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT);
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

// ---- an older file's summary columns, in its declared order ----------------

// convert_results reads an older file's results row through this.
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

// The column list_records sorts by, with its table alias: a column of the
// record's best path (`rp.`), a count of the record (`r.`, NULL like the path
// columns when the record kept no paths, as RecordRow::summary leaves it
// unset), or a name of the chart's naming copy (`c.`).
std::string sort_column_sql(SortColumn c) {
    switch (c) {
        case SortColumn::Score: return "rp.score";
        case SortColumn::ActCount: return "rp.actcount";
        case SortColumn::MaxSkip: return "rp.maxskip";
        case SortColumn::HardestMs: return "rp.hardest_ms";
        case SortColumn::AvgMult: return "rp.avgmult";
        case SortColumn::NoteCount: return "(CASE WHEN rp.rank IS NULL THEN NULL ELSE r.notecount END)";
        case SortColumn::SqInCount: return "rp.sqin_count";
        case SortColumn::SqOutCount: return "rp.sqout_count";
        case SortColumn::PathCount: return "(CASE WHEN rp.rank IS NULL THEN NULL ELSE r.pathcount END)";
        case SortColumn::RefName: return "c.name";
        case SortColumn::RefArtist: return "c.artist";
        case SortColumn::RefCharter: return "c.charter";
    }
    return "rp.score";
}

// The lens's columns, in Lens's field order, the order bind_lens binds them.
// The profile rows, lens_match and convert_results's read of an older file's
// results are all built from this list.
constexpr const char* kLensColumnList =
    "ms_enabled, ms_value, depth_mode, depth_value, legacy_fills";

// An older file's results summary columns, in the order read_summary reads
// them. Only convert_results reads them, to turn each row into a record.
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

// read_summary walks the slots by hand, so the assert stops the build when
// the list grows and it doesn't.
static_assert(count_list_names(kSummaryColumnList) == 10,
              "read_summary walks ten slots: grow it with the list");

// bind_lens walks the lens's slots by hand, so the assert stops the build
// when kLensColumnList grows and it doesn't.
static_assert(count_list_names(kLensColumnList) == 5,
              "bind_lens binds five slots: grow it and Lens with the list");

// A comma-separated column list with every name written as before + name +
// after, joined by `sep`: "r.score, r.actcount, ..." or "a=? AND b=?".
std::string each_column(const char* list, const char* before, const char* after,
                        const char* sep) {
    std::string out;
    std::string_view rest = list;
    while (!rest.empty()) {
        const size_t comma = rest.find(',');
        std::string_view name = rest.substr(0, comma);
        while (!name.empty() && name.front() == ' ') name.remove_prefix(1);
        if (!out.empty()) out += sep;
        out += before;
        out += name;
        out += after;
        rest = comma == std::string_view::npos ? std::string_view{} : rest.substr(comma + 1);
    }
    return out;
}

// ---- the record tables (D115) ----------------------------------------------
//
// `profiles`: the settings a record is filed under, plus the results stamp
// (hyversion) and the rules fingerprint (rules_fp, rules_fp_bytes). Its lens
// columns are kLensColumnList, so lens_match and bind_lens read this table.
// `records` and `record_paths` are WITHOUT ROWID tables: each row lives inside
// its key's index, so the key is stored once (sqlite.org/withoutrowid.html).
std::string record_tables_sql() {
    return std::string(
               "CREATE TABLE IF NOT EXISTS profiles ("
               "  id          INTEGER PRIMARY KEY,"
               "  chartmode   TEXT NOT NULL,"
               "  sp_cap      INTEGER NOT NULL, ") +
           each_column(kLensColumnList, "", " INTEGER NOT NULL", ", ") +
           ","
           "  hyversion   TEXT NOT NULL,"
           "  rules_fp    BLOB NOT NULL,"
           "  UNIQUE (chartmode, sp_cap, " + kLensColumnList + ", hyversion, rules_fp));"
           "CREATE TABLE IF NOT EXISTS records ("
           "  song_id     TEXT NOT NULL,"
           "  profile_id  INTEGER NOT NULL,"
           "  pathcount   INTEGER NOT NULL,"
           "  stars       INTEGER,"
           "  notecount   INTEGER NOT NULL,"
           "  PRIMARY KEY (song_id, profile_id)) WITHOUT ROWID;"
           "CREATE TABLE IF NOT EXISTS record_paths ("
           "  song_id     TEXT NOT NULL,"
           "  profile_id  INTEGER NOT NULL,"
           "  rank        INTEGER NOT NULL,"
           "  path        TEXT NOT NULL,"
           "  score       INTEGER NOT NULL,"
           "  actcount    INTEGER NOT NULL,"
           "  maxskip     INTEGER NOT NULL,"
           "  hardest_ms  REAL,"
           "  hardest_efill REAL,"
           "  avgmult     REAL NOT NULL,"
           "  sqin_count  INTEGER NOT NULL,"
           "  sqout_count INTEGER NOT NULL,"
           "  optimal     INTEGER NOT NULL,"
           "  PRIMARY KEY (song_id, profile_id, rank)) WITHOUT ROWID;";
}

// A record_paths row's own columns, in the order write_record binds them and
// read_path_row reads them (rank is bound and read on its own).
constexpr const char* kPathRowColumnList =
    "path, score, actcount, maxskip, hardest_ms, hardest_efill, avgmult, sqin_count, "
    "sqout_count, optimal";
static_assert(count_list_names(kPathRowColumnList) == 10,
              "write_record and read_path_row walk ten slots: grow them with the list");

// kPathRowColumnList with every name as `a` + name.
std::string path_row_columns(const char* a) {
    return each_column(kPathRowColumnList, a, "", ", ");
}

// A path row read at `idx` onwards (kPathRowColumnList's order). Unset when
// the row's path is NULL: a LEFT JOIN found no path at that rank.
std::optional<PathRow> read_path_row(sqlite3_stmt* s, int idx, int rank) {
    if (sqlite3_column_type(s, idx) == SQLITE_NULL) return std::nullopt;
    PathRow p;
    p.rank = rank;
    p.path = column_text(s, idx);
    p.score = sqlite3_column_int64(s, idx + 1);
    p.actcount = sqlite3_column_int(s, idx + 2);
    p.maxskip = sqlite3_column_int(s, idx + 3);
    p.hardest_ms = column_opt_f64(s, idx + 4);
    p.hardest_efill = column_opt_f64(s, idx + 5);
    p.avgmult = sqlite3_column_double(s, idx + 6);
    p.sqin_count = sqlite3_column_int(s, idx + 7);
    p.sqout_count = sqlite3_column_int(s, idx + 8);
    p.optimal = sqlite3_column_int(s, idx + 9) != 0;
    return p;
}

// A record's own columns after song_id, in the order write_record binds them
// and read_record_columns reads them.
constexpr const char* kRecordColumnList = "pathcount, stars, notecount";

// The record's own columns read at `idx` onwards (kRecordColumnList's order).
void read_record_columns(sqlite3_stmt* s, int idx, RecordRow& r) {
    r.pathcount = sqlite3_column_int(s, idx);
    if (auto v = column_opt_i64(s, idx + 1)) r.stars = static_cast<int>(*v);
    r.notecount = sqlite3_column_int(s, idx + 2);
}

// A summary as a record: what an older file's results row and a PreparedRow
// both hold, the best path's text and its summary. The best path becomes
// rank 1 and is optimal (it ties the top score by definition). A summary
// with no score is a record that kept no paths: no path rows, pathcount and
// notecount 0, no stars. convert_results and write_row both file through
// here, so the two cannot drift.
RecordRow record_from_summary(std::string song_id, const std::string& bestpath,
                              const PathSummary& s) {
    RecordRow r;
    r.song_id = std::move(song_id);
    if (!s.has_scored_best_path()) return r;
    r.pathcount = s.pathcount.value_or(0);
    r.stars = s.stars;
    r.notecount = s.notecount.value_or(0);
    PathRow best;
    best.rank = 1;
    best.path = bestpath;
    best.score = *s.score;
    best.actcount = s.actcount.value_or(0);
    best.maxskip = s.maxskip.value_or(0);
    best.hardest_ms = s.hardest_ms;
    best.avgmult = s.avgmult.value_or(0.0);
    best.sqin_count = s.sqin_count.value_or(0);
    best.sqout_count = s.sqout_count.value_or(0);
    best.optimal = true;
    r.paths.push_back(std::move(best));
    return r;
}

// The FROM clause every record lookup reads: each record (alias r) with its
// profile (p) and its best path (rp, NULL columns when it kept none).
constexpr const char* kRecordJoinSql =
    " FROM records r JOIN profiles p ON p.id = r.profile_id"
    " LEFT JOIN record_paths rp ON rp.song_id = r.song_id AND rp.profile_id = r.profile_id"
    " AND rp.rank = 1";

// SQLite refuses a statement with more than 32,766 bound values (the vendored
// 3.46's SQLITE_MAX_VARIABLE_NUMBER). lookup sends a whole library's
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
    return "(" + each_column(kLensColumnList, a, "=?", " AND ") + ")";
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
// has a current result under some settings is lookup's answer, from its
// winner's ready() (D79).
struct Candidate {
    bool current = false;     // stamped with this build's results version
    bool same_rules = false;  // analyzed under the rules this process runs
    int64_t profile_id = 0;   // the profile row the record is filed under
    // Why a row that is not ready is Stale, for callers that explain it
    // (the song panel, the library row's tooltip). Both can be true; neither
    // is on a ready row.
    bool stale_build = false;  // another Hydra build
    bool stale_rules = false;  // analyzed under other rules
    // Readable: stamped with this results version, under these rules.
    // Anything else is Stale.
    bool ready() const { return current && same_rules; }
};

// `hyversion` and `rules_fp` are the record's profile row's columns, and
// `profile_id` its id; `fixed_fp` is rules_fp_bytes of the fingerprint this
// process runs under.
Candidate rank_row(const std::string& hyversion, const std::vector<uint8_t>& rules_fp,
                   int64_t profile_id, const std::vector<uint8_t>& fixed_fp) {
    Candidate c;
    c.current = kResultsStamp.is_current(hyversion);
    c.same_rules = rules_fp == fixed_fp;
    c.profile_id = profile_id;
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
// the rest, then the later profile. A profile row is made the first time a
// record is filed under its settings, stamp and rules, so a bigger id means
// "made later". Since Auto went (2026-09-27) every lookup names one exact cap
// and lens. A chart holds one record per profile, so it offers one candidate
// per stamp and set of rules it was analyzed under, and the one made under
// this build's stamp and this process's rules wins. Two candidates that tie
// on both are both Stale for the same reasons, so the id only picks which of
// two identical answers to give.
bool outranks(const Candidate& a, const Candidate& b) {
    if (a.current != b.current) return a.current;
    if (a.same_rules != b.same_rules) return a.same_rules;
    return a.profile_id > b.profile_id;
}

// Which chart a winner is picked for: one per chart and mode.
using GroupKey = std::pair<std::string, std::string>;

// The one owner of "which row wins". A lookup offers its candidate rows here
// in any order, then asks which offer won each chart and mode: lookup
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

template <class F>
void RecordStore::in_transaction(F work) {
    exec("BEGIN");
    try {
        work();
        exec("COMMIT");
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

// The old bestpath column writes this text (prepare_row); it goes with that
// write path (D115 A1-5). Readers ask RecordRow::best_path_text.
std::string best_path_text(const HydraRecord& record) {
    return record.paths.empty() ? std::string() : record.best_path().pathstring();
}

// ---- an analysis as a record: prepare_record and its shims -----------------

namespace {

// One path's own facts as a path row, at `rank`. Whether the path is optimal
// is the analysis's answer, so record_of sets it. The one place a path's
// facts are read off it: summarize_path and record_of both come here.
PathRow path_row_of(const Path& path, int rank) {
    PathRow row;
    row.rank = rank;
    row.path = path.pathstring();
    row.score = path.totalscore();
    const ActivationWalk acts = path.walk_activations();
    row.actcount = static_cast<int>(acts.size());
    for (const Activation& a : acts) {
        if (a.skips() > row.maxskip) row.maxskip = a.skips();
        for (const SPSqueeze& sq : a.sqinouts) {
            if (sq.kind == SqueezeKind::SqIn) ++row.sqin_count;
            else ++row.sqout_count;
        }
        const std::optional<double> efill = a.e_difficulty();
        if (efill && (!row.hardest_efill || *efill > *row.hardest_efill))
            row.hardest_efill = efill;
    }
    row.hardest_ms = path.difficulty();  // the path's own hardest squeeze or required fill
    row.avgmult = path.avg_mult();
    return row;
}

}  // namespace

// The record an analysis becomes: its kept paths in all_paths()'s order,
// which is best first (rank 1 is best_path()), at most kPathsPerRecord of
// them. Every path of one analysis carries the same note count, so the
// record keeps rank 1's once.
RecordRow record_of(std::string song_id, const HydraRecord& analysis) {
    RecordRow r;
    r.song_id = std::move(song_id);
    const std::vector<const Path*> paths = analysis.all_paths();
    // Every kept path, roots and variants: the count the Paths tab shows.
    r.pathcount = static_cast<int>(paths.size());
    if (paths.empty()) return r;
    r.stars = path_stars(*paths.front());
    r.notecount = paths.front()->notecount;
    const size_t kept = std::min(paths.size(), static_cast<size_t>(kPathsPerRecord));
    for (size_t i = 0; i < kept; ++i) {
        PathRow row = path_row_of(*paths[i], static_cast<int>(i + 1));
        row.optimal = analysis.is_optimal(*paths[i]);
        r.paths.push_back(std::move(row));
    }
    return r;
}

namespace {

// Throws std::invalid_argument when `profile` is not what `analysis` ran
// under: its cap, its ms limit or its fill rule. Each mismatch would file the
// analysis under settings it doesn't belong to. `who` starts the message.
void check_profile_fits(const Profile& profile, const HydraRecord& record, const char* who) {
    const std::string w = std::string(who) + ": ";
    if (!record.sp_cap) throw std::invalid_argument(w + "record carries no sp_cap");
    if (profile.cap.exact != *record.sp_cap)
        throw std::invalid_argument(w + "key asks for sp_cap " +
                                    std::to_string(profile.cap.exact) +
                                    " but the record was analyzed at " +
                                    std::to_string(*record.sp_cap));
    // Both sides come from the same int (Settings::mslimit_value, widened to
    // the engine's double), so an exact comparison is the right one.
    if (profile.lens.ms_enabled == 1 &&
        (!record.ms_limit || *record.ms_limit != static_cast<double>(profile.lens.ms_value)))
        throw std::invalid_argument(
            w + "key asks for an ms limit of " + std::to_string(profile.lens.ms_value) +
            " but the record was analyzed " +
            (record.ms_limit ? "at " + std::to_string(*record.ms_limit) : "without one"));
    // And the other way: a record searched with a limit never files under a
    // lens that has the limit off (finding 130).
    if (profile.lens.ms_enabled == 0 && record.ms_limit)
        throw std::invalid_argument(w + "key asks for no ms limit but the record was analyzed at " +
                                    std::to_string(*record.ms_limit));
    // The record's own fill rule as a lens stores it (Lens::from).
    const Lens record_fills = Lens::from(std::nullopt, 0, 0, record.legacy_fills);
    if (profile.lens.legacy_fills != record_fills.legacy_fills)
        throw std::invalid_argument(
            w + "key asks for " +
            fill_rule_name(fill_rule_for(profile.lens.legacy_fills != 0), FillRuleNameStyle::Long) +
            " fills but the record was analyzed under the " +
            fill_rule_name(fill_rule_for(record.legacy_fills), FillRuleNameStyle::Long) + " rule");
}

}  // namespace

PreparedRecord prepare_record(std::string song_id, const Profile& profile,
                              const HydraRecord& analysis) {
    check_profile_fits(profile, analysis, "prepare_record");
    PreparedRecord out;
    out.profile = profile;
    out.rules_fp = analysis.rules_fingerprint;
    out.record = record_of(std::move(song_id), analysis);
    return out;
}

// summarize_path and summarize_record stay until their callers move to
// prepare_record (D115); both read through path_row_of and record_of.
PathSummary summarize_path(const Path& path) {
    const PathRow row = path_row_of(path, 1);
    PathSummary s;
    s.score = row.score;
    s.actcount = row.actcount;
    s.maxskip = row.maxskip;
    s.hardest_ms = row.hardest_ms;
    s.avgmult = row.avgmult;
    s.notecount = path.notecount;
    s.sqin_count = row.sqin_count;
    s.sqout_count = row.sqout_count;
    s.stars = path_stars(path);
    return s;
}

// A Ready record with no paths has no facts (D51 call 11): every field is
// unset and has_scored_best_path() is false (RecordRow::summary).
PathSummary summarize_record(const HydraRecord& record) {
    return record_of(std::string(), record).summary();
}

PathSummary RecordRow::summary() const {
    if (paths.empty()) return PathSummary{};

    const PathRow& best = paths.front();
    PathSummary s;
    s.score = best.score;
    s.actcount = best.actcount;
    s.maxskip = best.maxskip;
    s.hardest_ms = best.hardest_ms;
    s.avgmult = best.avgmult;
    s.notecount = notecount;
    s.sqin_count = best.sqin_count;
    s.sqout_count = best.sqout_count;
    s.pathcount = pathcount;
    s.stars = stars;
    return s;
}

std::string RecordRow::best_path_text() const {
    return paths.empty() ? std::string() : paths.front().path;
}

std::string current_record_version() { return std::string(kResultsStamp.written); }

PreparedRow prepare_row(const RecordKey& key, const HydraRecord& record) {
    check_profile_fits(key.settings(), record, "prepare_row");

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
    // "win32-longpath" is SQLite's own Windows layer with its path buffer
    // raised from 260 characters to 32,767; it is otherwise the default one.
    // An attached file goes through the same layer as its connection. SQLite
    // takes UTF-8 and understands the \\?\ prefix win32_path adds (ADR 0020).
    return sqlite3_open_v2(wide_to_utf8(win32_path(utf8_path)).c_str(), db, flags,
                           "win32-longpath");
}

namespace {

// "A result with a score and no stars", spelled once in SQL. Such a row was
// written before the stars column existed. 2.2.0's writer never leaves one,
// but 2.2.0's own open kept them in a pre-stars file that had no path details,
// so a Convertible file can still hold some. Filling their stars needed the
// stored paths, so the
// conversion leaves them behind, and a click or a batch writes them again. The
// conversion's row count and its read both use it.
constexpr const char* kNoStarsResultSql = "stars IS NULL AND score IS NOT NULL";

// The one integer a counting query returns.
int64_t count_of(sqlite3* db, const std::string& sql) {
    Stmt s = prepare_read(db, sql.c_str());
    if (!step_row(s)) return 0;
    return sqlite3_column_int64(s, 0);
}

}  // namespace

// ---- RecordStore ------------------------------------------------------

RecordStore::RecordStore(const std::string& dbpath, core::RulesStamp rules_fingerprint,
                         OpenProgressFn progress)
    : RecordStore(dbpath, OldFilePolicy::Refuse, rules_fingerprint, std::move(progress)) {}

RecordStore::RecordStore(const std::string& dbpath, OldFilePolicy old_files,
                         core::RulesStamp rules_fingerprint, OpenProgressFn progress)
    : rules_fingerprint_(rules_fingerprint), progress_(std::move(progress)) {
    // Whatever happens below, the callback is not kept past the constructor.
    struct ForgetProgress {
        OpenProgressFn& fn;
        ~ForgetProgress() { fn = nullptr; }
    } forget{progress_};

    report(OpenStep::Opening);
    // Read before anything opens the file for writing. An Unreadable file
    // goes on to the open below, whose first statement fails with SQLite's
    // own words, as any unreadable file's open does.
    const LibraryFileKind kind = classify_library_file(dbpath);
    if (kind == LibraryFileKind::TooOld) {
        if (old_files == OldFilePolicy::Refuse)
            throw KindedError(ErrorKind::OldLibraryRefused, dbpath);
        if (std::optional<std::string> why = set_aside_old_library(dbpath))
            throw KindedError(ErrorKind::OldLibrarySetAsideFailed, *why);
        set_aside_path_ = store::set_aside_path(dbpath);
    }
    open_file(dbpath);

    // SQLite notices a locked file, or one that isn't a database, only at
    // the first statement below. Any throw from here on is still a failed
    // open (D72 item 2), with the raw text kept. A constructor that throws
    // never runs the destructor, so the handle is closed here. Running out of
    // memory keeps its own type, which plain_error answers by. A stop the
    // callback asked for, and a failed conversion that left the file whole,
    // keep their own kinds too.
    try {
        set_up_schema(kind == LibraryFileKind::Convertible);
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

void RecordStore::set_up_schema(bool convertible) {
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
    // The library page sorts by name (list_chart_library's ORDER BY name).
    exec("CREATE INDEX IF NOT EXISTS charts_by_name ON charts (name)");
    // The scan's purge, list_records and list_chart_library_copies find a
    // chart's library rows by md5 (D76).
    exec("CREATE INDEX IF NOT EXISTS charts_by_md5 ON charts (md5)");
    create_result_tables();
    if (convertible) convert_results();
}

// The meta row where 2.2.0 to 2.7.0 kept one fill-rule label for the whole
// file. Each record's profile carries its own rule (Lens::legacy_fills), so
// one file holds both rules and the label is retired (D115 A1-6c): the
// conversion deletes the row, and nothing reads it.
constexpr const char* kRetiredFillLabelKey = "engine_mode";

void RecordStore::convert_results() {
    const std::string kept = std::string("NOT (") + kNoStarsResultSql + ")";
    // A failure undoes the whole conversion (in_transaction), so the file is
    // as it was: DatabaseUpgrade says so. A stop asked for and running out of
    // memory keep their own kinds.
    try {
        const int64_t rows_total = count_of(db_, "SELECT COUNT(*) FROM results WHERE " + kept);
        int64_t rows_done = 0;
        report(OpenStep::ConvertingRecords, rows_done, rows_total);
        in_transaction([&] {
        {
            // Oldest first, so of two rows that become one record (a hash
            // that differed only in case) the newer write is kept, as a
            // lookup ranked them.
            static constexpr const char* kLeadColumns =
                "lower(hyhash), chartmode, sp_cap, hyversion, rules_fp, bestpath";
            static constexpr int kFirstLens = count_list_names(kLeadColumns);
            static constexpr int kFirstSummary = kFirstLens + count_list_names(kLensColumnList);
            const std::string sql = std::string("SELECT ") + kLeadColumns + ", " +
                                    kLensColumnList + ", " + kSummaryColumnList +
                                    " FROM results WHERE " + kept + " ORDER BY result_id";
            Stmt s = prepare_read(db_, sql.c_str());
            while (step_row(s)) {
                Profile profile;
                profile.chartmode = column_text(s, 1);
                profile.cap = CapQuery::at(sqlite3_column_int(s, 2));
                profile.lens.ms_enabled = sqlite3_column_int(s, kFirstLens);
                profile.lens.ms_value = sqlite3_column_int(s, kFirstLens + 1);
                profile.lens.depth_mode = sqlite3_column_int(s, kFirstLens + 2);
                profile.lens.depth_value = sqlite3_column_int(s, kFirstLens + 3);
                profile.lens.legacy_fills = sqlite3_column_int(s, kFirstLens + 4);
                const int64_t profile_id =
                    profile_id_for(profile, column_text(s, 3), column_blob(s, 4));
                write_record(profile_id, record_from_summary(column_text(s, 0), column_text(s, 5),
                                                             read_summary(s, kFirstSummary)));
                report(OpenStep::ConvertingRecords, ++rows_done, rows_total);
            }
        }
        // The library's ids take the records' spelling (ADR 0028).
        exec("UPDATE charts SET md5 = lower(md5)");
        exec("DROP TABLE results");
        exec((std::string("DELETE FROM meta WHERE key = '") + kRetiredFillLabelKey + "'").c_str());
        });
    } catch (const KindedError& e) {
        if (e.kind() == ErrorKind::Cancelled) throw;
        throw KindedError(ErrorKind::DatabaseUpgrade, e.what());
    } catch (const std::bad_alloc&) {
        throw;
    } catch (const std::exception& e) {
        throw KindedError(ErrorKind::DatabaseUpgrade, e.what());
    }
}

void RecordStore::open_file(const std::string& dbpath) {
    if (open_sqlite(dbpath, &db_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
        close();
        throw KindedError(ErrorKind::DatabaseOpen,
                          "failed to open database '" + dbpath + "': " + msg);
    }
}

void RecordStore::report(OpenStep step, int64_t rows_done, int64_t rows_total) {
    if (!progress_) return;
    OpenProgress p;
    p.step = step;
    p.rows_done = rows_done;
    p.rows_total = rows_total;
    if (!progress_(p))
        throw Cancelled("opening the database was stopped");
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

// The meta row that holds the charts table's kChartMetaStamp, written once
// per file by rebuild_chart_library and checked by chart_library_cache.
constexpr const char* kChartMetaKey = "chart_meta_version";

template <class F>
void RecordStore::in_save_frame(F work) {
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
        work();
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

void RecordStore::save_analysis(const PreparedRow& row) {
    in_save_frame([&] { write_row(row); });
}

void RecordStore::save(const PreparedRecord& prepared) {
    in_save_frame([&] {
        file_record(prepared.profile, current_record_version(), prepared.rules_fp,
                    prepared.record, PurgeScope::EveryMode);
    });
}

void RecordStore::create_result_tables() { exec(record_tables_sql().c_str()); }

void RecordStore::add_record(const RecordKey& key, const HydraRecord& record) {
    add_row(prepare_row(key, record));
}

void RecordStore::add_row(const PreparedRow& row) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    in_transaction([&] { write_row(row); });
}

void RecordStore::delete_records_where(const std::string& where,
                                       const std::function<void(sqlite3_stmt*)>& bind,
                                       const std::string& what) {
    for (const char* table : {"record_paths", "records"}) {
        CachedStmt d = use_write(db_, stmt_cache_, std::string("DELETE FROM ") + table +
                                                       " WHERE " + where);
        bind(d);
        step_done(d, what);
    }
}

int64_t RecordStore::profile_id_for(const Profile& profile, const std::string& hyversion,
                                    const std::vector<uint8_t>& rules_fp) {
    // Binds the profile's columns in the order both statements name them:
    // the mode, the cap, the lens, the stamp, the rules.
    auto bind_profile = [&](sqlite3_stmt* s) {
        bind_text(s, 1, profile.chartmode);
        int idx = bind_lens(s, bind_cap(s, 2, profile.cap), profile.lens);
        bind_text(s, idx++, hyversion);
        bind_blob(s, idx, rules_fp);
    };
    {
        CachedStmt find = use_read(db_, stmt_cache_,
            "SELECT id FROM profiles WHERE chartmode=? AND " + cap_match("") + " AND " +
                lens_match("") + " AND hyversion=? AND rules_fp=?");
        bind_profile(find);
        if (step_row(find)) return sqlite3_column_int64(find, 0);
    }
    CachedStmt insert = use_write(db_, stmt_cache_,
        std::string("INSERT INTO profiles (chartmode, sp_cap, ") + kLensColumnList +
            ", hyversion, rules_fp) VALUES (" +
            placeholders(2 + count_list_names(kLensColumnList) + 2) + ")");
    bind_profile(insert);
    step_done(insert, "filing a record's profile");
    return sqlite3_last_insert_rowid(db_);
}

void RecordStore::write_record(int64_t profile_id, const RecordRow& record) {
    delete_records_where(
        "song_id=? AND profile_id=?",
        [&](sqlite3_stmt* s) {
            bind_text(s, 1, record.song_id);
            sqlite3_bind_int64(s, 2, profile_id);
        },
        "replacing a record");
    {
        CachedStmt s = use_write(db_, stmt_cache_,
            std::string("INSERT INTO records (song_id, profile_id, ") + kRecordColumnList +
                ") VALUES (" + placeholders(2 + count_list_names(kRecordColumnList)) + ")");
        bind_text(s, 1, record.song_id);
        sqlite3_bind_int64(s, 2, profile_id);
        sqlite3_bind_int(s, 3, record.pathcount);
        if (record.stars) sqlite3_bind_int(s, 4, *record.stars);
        else sqlite3_bind_null(s, 4);
        sqlite3_bind_int(s, 5, record.notecount);
        step_done(s, "saving a record");
    }
    for (const PathRow& p : record.paths) {
        CachedStmt s = use_write(db_, stmt_cache_,
            std::string("INSERT INTO record_paths (song_id, profile_id, rank, ") +
                kPathRowColumnList + ") VALUES (" +
                placeholders(3 + count_list_names(kPathRowColumnList)) + ")");
        bind_text(s, 1, record.song_id);
        sqlite3_bind_int64(s, 2, profile_id);
        sqlite3_bind_int(s, 3, p.rank);
        bind_text(s, 4, p.path);
        sqlite3_bind_int64(s, 5, p.score);
        sqlite3_bind_int(s, 6, p.actcount);
        sqlite3_bind_int(s, 7, p.maxskip);
        bind_opt_f64(s, 8, p.hardest_ms);
        bind_opt_f64(s, 9, p.hardest_efill);
        sqlite3_bind_double(s, 10, p.avgmult);
        sqlite3_bind_int(s, 11, p.sqin_count);
        sqlite3_bind_int(s, 12, p.sqout_count);
        sqlite3_bind_int(s, 13, p.optimal ? 1 : 0);
        step_done(s, "saving a record's path");
    }
}

void RecordStore::drop_empty_profiles() {
    CachedStmt d = use_write(db_, stmt_cache_,
        "DELETE FROM profiles WHERE NOT EXISTS"
        " (SELECT 1 FROM records WHERE records.profile_id = profiles.id)");
    step_done(d, "deleting emptied profiles");
}

void RecordStore::file_record(const Profile& profile, const std::string& hyversion,
                              uint64_t rules_fp, const RecordRow& record, PurgeScope scope) {
    // The caller holds the lock and an open transaction, so a failure
    // anywhere leaves the store exactly as it was.

    // (1) Anything this chart holds, in the modes `scope` names (PurgeScope:
    //     ThisMode is the old write_row's, until A1-5 deletes it), that this
    //     build can never read -- another Hydra version's stamp, which
    //     includes every row an old migration left -- is superseded by a
    //     write here. A record made
    //     under other rules is not: it reads Ready again once the rules match
    //     (D51 call 8). The test is against what is current, not against this
    //     record: a test writing a deliberately old-stamped record must not
    //     take the real ones with it, and this runs before the write so the
    //     new record is untouched.
    const bool one_mode = scope == PurgeScope::ThisMode;
    const std::string mode_term = one_mode ? "chartmode=? AND " : "";
    delete_records_where(
        "song_id=? AND profile_id IN (SELECT id FROM profiles WHERE " + mode_term + "NOT " +
            row_readable_sql() + ")",
        [&](sqlite3_stmt* s) {
            int idx = 1;
            bind_text(s, idx++, record.song_id);
            if (one_mode) bind_text(s, idx++, profile.chartmode);
            bind_readable_params(s, idx);
        },
        "unreadable purge");

    // (2) The record, replacing the chart's record under the same profile.
    write_record(profile_id_for(profile, hyversion, rules_fp_bytes(rules_fp)), record);
    // (3) A profile the purge emptied goes with its last record.
    drop_empty_profiles();
}

void RecordStore::write_row(const PreparedRow& row) {
    Profile profile;
    profile.chartmode = row.chartmode;
    profile.cap = CapQuery::at(row.sp_cap);
    profile.lens = row.lens;
    file_record(profile, row.hyversion, row.rules_fp,
                record_from_summary(row.hyhash, row.bestpath, row.summary), PurgeScope::ThisMode);
}

namespace {
// "No library row lists this chart", spelled once in SQL for
// purge_charts_not_in_library. `hash` is the column holding the chart's hash
// in the table the statement reads.
std::string not_in_library(const char* hash) {
    return std::string("NOT EXISTS (SELECT 1 FROM charts WHERE charts.md5 = ") + hash + ")";
}
}  // namespace

void RecordStore::reidentify_chart(const std::string& notespath, const std::string& new_md5,
                                   const std::string& new_sig) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    in_transaction([&] {
        {
            CachedStmt s =
                use_write(db_, stmt_cache_, "UPDATE charts SET md5 = ?, sig = ? WHERE path = ?");
            bind_text(s, 1, new_md5);
            bind_text(s, 2, new_sig);
            bind_text(s, 3, notespath);
            step_done(s, "reidentify_chart");
        }
        purge_charts_not_in_library("reidentify_chart");
    });
}

void RecordStore::purge_charts_not_in_library(const char* caller) {
    delete_records_where(not_in_library("song_id"), [](sqlite3_stmt*) {},
                         std::string(caller) + " purge");
    drop_empty_profiles();
}

SummaryLookup RecordStore::get_summary(const RecordKey& key) {
    return get_summaries({key.hyhash}, key.settings()).front();
}

std::vector<SummaryLookup> RecordStore::get_summaries(const std::vector<std::string>& hyhashes,
                                                      const ResultSettings& want) {
    // The old answer's shape, read off lookup's: the best path's text and the
    // record's summary, for a Ready record only.
    const std::vector<RecordLookup> found = lookup(hyhashes, want);
    std::vector<SummaryLookup> out(found.size());
    for (size_t i = 0; i < found.size(); ++i) {
        out[i].status = found[i].status;
        out[i].stale_build = found[i].stale_build;
        out[i].stale_rules = found[i].stale_rules;
        if (found[i].status != RecordStatus::Ready) continue;
        if (!found[i].record.paths.empty()) out[i].bestpath = found[i].record.paths.front().path;
        out[i].summary = found[i].record.summary();
    }
    return out;
}

std::vector<RecordLookup> RecordStore::lookup(const std::vector<std::string>& song_ids,
                                              const Profile& want) {
    std::vector<RecordLookup> out(song_ids.size());
    if (song_ids.empty()) return out;

    // Each chart is asked about once, however many folders list it. A chart's
    // rows all come back in its own chunk, so one picker sees every
    // candidate a chart has.
    std::vector<std::string> distinct(song_ids);
    std::sort(distinct.begin(), distinct.end());
    distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());

    // The lookup's own columns, read below at 0 onwards: the record and its
    // profile (the profile id is the ranking's newest-write tiebreak), then
    // the record's counts. The candidates are every record under a profile
    // with the wanted mode, cap and lens, whatever its stamp and rules;
    // rank_row says which is readable.
    static constexpr const char* kLeadColumns = "r.song_id, p.hyversion, p.id, p.rules_fp";
    static constexpr int kFirstRecord = count_list_names(kLeadColumns);
    const std::vector<uint8_t> fixed_fp = ready_rules_fp(rules_fingerprint_);
    WinnerPicker picker;
    std::vector<RecordRow> offered;
    std::unordered_map<std::string, RecordLookup> by_id;
    // The profile each Ready record is filed under, by song id: whose path
    // rows the second read keeps.
    std::unordered_map<std::string, int64_t> ready_profile;
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (size_t first = 0; first < distinct.size(); first += kHashesPerQuery) {
        const size_t n = std::min(kHashesPerQuery, distinct.size() - first);
        std::string sql = std::string("SELECT ") + kLeadColumns + ", " +
                          each_column(kRecordColumnList, "r.", "", ", ") +
                          " FROM records r JOIN profiles p ON p.id = r.profile_id"
                          " WHERE p.chartmode=? AND r.song_id IN (" +
                          placeholders(n) + ")";
        append_candidate_filter(sql, "p.");
        Stmt s = prepare_read(db_, sql.c_str());
        int idx = 1;
        bind_text(s, idx++, want.chartmode);
        for (size_t i = 0; i < n; ++i) bind_text(s, idx++, distinct[first + i]);
        bind_candidate_filter(s, idx, want.cap, want.lens);
        while (step_row(s)) {
            RecordRow record;
            record.song_id = column_text(s, 0);
            read_record_columns(s, kFirstRecord, record);
            picker.offer(record.song_id, want.chartmode,
                         rank_row(column_text(s, 1), column_blob(s, 3),
                                  sqlite3_column_int64(s, 2), fixed_fp));
            offered.push_back(std::move(record));
        }
    }

    // One answer per chart. A stale winner is reported as Stale, not hidden:
    // the library has to tell "analyzed by another build" apart from "never
    // analyzed".
    const std::vector<bool> won = picker.winners();
    for (size_t i = 0; i < offered.size(); ++i) {
        if (!won[i]) continue;
        RecordLookup& answer = by_id[offered[i].song_id];
        const Candidate& rank = picker.rank(i);
        if (rank.ready()) {
            answer.status = RecordStatus::Ready;
            answer.record = std::move(offered[i]);
            ready_profile.emplace(answer.record.song_id, rank.profile_id);
        } else {
            answer.status = RecordStatus::Stale;
            answer.stale_build = rank.stale_build;
            answer.stale_rules = rank.stale_rules;
        }
    }

    // The Ready records' paths, in rank order: one more read over
    // record_paths for those charts, keeping each chart's winning profile's.
    std::vector<std::string> ready_ids;
    ready_ids.reserve(ready_profile.size());
    for (const auto& kv : ready_profile) ready_ids.push_back(kv.first);
    for (size_t first = 0; first < ready_ids.size(); first += kHashesPerQuery) {
        const size_t n = std::min(kHashesPerQuery, ready_ids.size() - first);
        const std::string sql = std::string("SELECT song_id, profile_id, rank, ") +
                                path_row_columns("") +
                                " FROM record_paths WHERE song_id IN (" + placeholders(n) +
                                ") ORDER BY song_id, profile_id, rank";
        Stmt s = prepare_read(db_, sql.c_str());
        for (size_t i = 0; i < n; ++i) bind_text(s, static_cast<int>(i + 1), ready_ids[first + i]);
        while (step_row(s)) {
            const std::string id = column_text(s, 0);
            if (ready_profile.at(id) != sqlite3_column_int64(s, 1)) continue;
            if (std::optional<PathRow> p = read_path_row(s, 3, sqlite3_column_int(s, 2)))
                by_id[id].record.paths.push_back(std::move(*p));
        }
    }

    for (size_t i = 0; i < song_ids.size(); ++i) {
        auto it = by_id.find(song_ids[i]);
        if (it != by_id.end()) out[i] = it->second;
    }
    return out;
}

RecordLookup RecordStore::lookup_one(const std::string& song_id, const Profile& want) {
    return lookup({song_id}, want).front();
}

bool RecordStore::has_record(const RecordKey& key) {
    // lookup owns "this chart has a current result under these settings"
    // (D79): the library's Analyzed chip reads the same answer.
    return lookup_one(key.hyhash, key.settings()).status == RecordStatus::Ready;
}

std::vector<std::string> RecordStore::candidate_song_ids(const Profile& want) {
    std::vector<std::string> out;
    std::string sql =
        "SELECT DISTINCT r.song_id FROM records r JOIN profiles p ON p.id = r.profile_id"
        " WHERE p.chartmode=?";
    append_candidate_filter(sql, "p.");
    Stmt s = prepare_read(db_, sql.c_str());
    bind_text(s, 1, want.chartmode);
    bind_candidate_filter(s, 2, want.cap, want.lens);
    while (step_row(s)) out.push_back(column_text(s, 0));
    return out;
}

std::unordered_set<std::string> RecordStore::ready_song_ids(const Profile& want) {
    // Every chart with a candidate record under these settings, then
    // lookup's answer for each (D79), so the batch skips exactly the charts
    // the library's Analyzed chip counts. One lock over both reads, so no
    // write lands between them.
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    const std::vector<std::string> candidates = candidate_song_ids(want);
    const std::vector<RecordLookup> found = lookup(candidates, want);
    std::unordered_set<std::string> out;
    for (size_t i = 0; i < candidates.size(); ++i)
        if (found[i].status == RecordStatus::Ready) out.insert(candidates[i]);
    return out;
}

std::unordered_set<std::string> RecordStore::analyzed_hashes(const ResultSettings& want) {
    return ready_song_ids(want);
}

std::vector<RecordRow> RecordStore::ready_records(const Profile& want) {
    std::vector<RecordRow> out;
    {
        // As ready_song_ids: one lock over both reads.
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        const std::vector<std::string> candidates = candidate_song_ids(want);
        for (RecordLookup& found : lookup(candidates, want))
            if (found.status == RecordStatus::Ready) out.push_back(std::move(found.record));
    }
    // Best score first; a record that kept no paths has no score and goes
    // last. Equal scores keep the song id's order, so a listing never
    // shuffles between two reads.
    std::sort(out.begin(), out.end(), [](const RecordRow& a, const RecordRow& b) {
        if (a.paths.empty() != b.paths.empty()) return b.paths.empty();
        if (!a.paths.empty() && a.paths.front().score != b.paths.front().score)
            return a.paths.front().score > b.paths.front().score;
        return a.song_id < b.song_id;
    });
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

    // The listing's own columns, read below at 0 onwards; the record's
    // counts follow, then its best path, then the four the ranking reads. A
    // record whose chart the library doesn't list (hydra_batch with folder
    // arguments) is still listed, with no names (D77).
    static constexpr const char* kLeadColumns =
        "r.song_id, c.name, c.artist, c.charter, p.chartmode";
    static constexpr int kFirstRecord = count_list_names(kLeadColumns);
    static constexpr int kFirstPath = kFirstRecord + count_list_names(kRecordColumnList);
    static constexpr int kAfterPath = kFirstPath + count_list_names(kPathRowColumnList);
    std::string sql = std::string("SELECT ") + kLeadColumns + ", " +
                      each_column(kRecordColumnList, "r.", "", ", ") + ", " +
                      path_row_columns("rp.") + ", p.sp_cap, p.hyversion, p.id, p.rules_fp" +
                      kRecordJoinSql + " LEFT JOIN " + kNamingCopiesSql +
                      " AS c ON c.md5 = r.song_id WHERE 1=1";
    if (chartmode) sql += " AND p.chartmode = ?";
    append_candidate_filter(sql, "p.");

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
        RecordRow record;
        read_record_columns(s, kFirstRecord, record);
        if (std::optional<PathRow> best = read_path_row(s, kFirstPath, 1)) {
            listing.bestpath = best->path;
            record.paths.push_back(std::move(*best));
        }
        listing.summary = record.summary();
        listing.sp_cap = sqlite3_column_int(s, kAfterPath);
        picker.offer(listing.hyhash, listing.chartmode,
                     rank_row(column_text(s, kAfterPath + 1), column_blob(s, kAfterPath + 3),
                              sqlite3_column_int64(s, kAfterPath + 2), fixed_fp));
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

int64_t RecordStore::record_count() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_, "SELECT COUNT(*) FROM records");
    if (!step_row(s)) return 0;
    return sqlite3_column_int64(s, 0);
}

std::pair<int64_t, int64_t> RecordStore::counts() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_, "SELECT COUNT(DISTINCT song_id) FROM records");
    const int64_t charts = step_row(s) ? sqlite3_column_int64(s, 0) : 0;
    return {charts, record_count()};
}

// ---- chart library ------------------------------------------------------

namespace {

// The charts table's columns as one library entry fills them, in the order
// rebuild_chart_library writes them and read_library_entries reads them.
constexpr const char* kLibraryColumnList =
    "md5, name, artist, charter, path, folder, sig, stated_length_ms, delay_ms";
// Both walk the slots by hand, so the assert stops the build when the list
// grows and they don't.
static_assert(count_list_names(kLibraryColumnList) == 9,
              "rebuild_chart_library and read_library_entries walk nine slots: grow them "
              "with the list");

// The query read_library_entries reads; callers add a WHERE or an ORDER BY.
std::string library_entry_select() {
    return std::string("SELECT ") + kLibraryColumnList + " FROM charts";
}

// Every row `s` (a library_entry_select query) steps to. Rows an older scan
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

void RecordStore::rebuild_chart_library(const std::vector<ChartLibraryEntry>& items) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // One transaction around the whole swap, opened before anything changes:
    // a failure anywhere rolls back to the previous scan's rows, which are
    // also the rescan cache. The table is emptied rather than dropped, so its
    // columns and index stay as they are.
    in_transaction([&] {
        exec("DELETE FROM charts");
        Stmt s = prepare_write(db_,
            (std::string("INSERT INTO charts (") + kLibraryColumnList + ") VALUES (" +
             placeholders(count_list_names(kLibraryColumnList)) + ")")
                .c_str());
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
    });
}

ChartLibraryCache RecordStore::chart_library_cache() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ChartLibraryCache cache;
    // Rows the current readers didn't vouch for are no cache at all: a file
    // from before the stamp, or one an older reader filled (D51 call 12).
    // The next scan reads every chart once and stamps what it writes.
    if (!chart_meta_current()) return cache;

    Stmt s = prepare_read(db_, library_entry_select().c_str());
    for (ChartLibraryEntry& e : read_library_entries(s, /*timing_read=*/true)) {
        if (!sig_can_show_unchanged(e.sig)) continue;
        std::string key = e.notespath;
        cache[std::move(key)] = std::move(e);
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

std::vector<ChartLibraryEntry> RecordStore::list_chart_library(int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(
        db_, (library_entry_select() + " ORDER BY name LIMIT ? OFFSET ?").c_str());
    sqlite3_bind_int(s, 1, limit);
    sqlite3_bind_int(s, 2, offset);
    return read_library_entries(s, timing_read);
}

std::vector<ChartLibraryEntry> RecordStore::list_chart_library_copies(const std::string& md5) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(db_, (library_entry_select() + " WHERE md5 = ?").c_str());
    bind_text(s, 1, md5);
    return read_library_entries(s, timing_read);
}

ChartIndex RecordStore::chart_index() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // rebuild_chart_library inserts the rows in the scan's order, so rowid
    // order is scan order.
    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(db_, (library_entry_select() + " ORDER BY rowid").c_str());
    return ChartIndex::from_rows(read_library_entries(s, timing_read));
}

}  // namespace hydra::store
