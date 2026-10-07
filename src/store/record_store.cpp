#include "store/record_store.h"

#include <sqlite3.h>

#include <algorithm>
#include <cstdlib>
#include <map>
#include <new>
#include <stdexcept>
#include <string_view>

#include "core/error_kind.h"
#include "core/stars.h"
#include "core/winstr.h"
#include "store/serialize.h"
#include "store/stored_versions.h"

namespace hydra::store {

namespace {

// RAII wrapper so every query site finalizes even on an early throw. Movable
// but not copyable: a copy would finalize the same handle twice. Moving lets a
// statement be parked in a std::optional and destroyed on purpose later --
// for_each_blob does that, so its two per-row statements live across the whole
// walk instead of being compiled again for every row.
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

// Clears a reused statement's cursor so it can be bound and stepped again.
// Bindings survive a reset and are overwritten by the next bind, so
// sqlite3_clear_bindings is not needed. Every reuse site calls this before its
// caller drops the store lock: a half-stepped statement holds a read cursor
// open on the table, and the locking rule is that no sqlite state outlives the
// locked block.
struct ResetOnExit {
    sqlite3_stmt* s;
    ~ResetOnExit() {
        if (s) sqlite3_reset(s);
    }
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

// ---- tempomap blob (songmeta.tempomap) ------------------------------------

std::vector<uint8_t> encode_tempomap(const Song& song) {
    BinaryWriter w;
    w.i64(song.tick_resolution());
    w.u32(static_cast<uint32_t>(song.tpm_changes.size()));
    for (const auto& [tick, tpm] : song.tpm_changes) {
        w.i64(tick);
        w.i64(tpm);
    }
    w.u32(static_cast<uint32_t>(song.bpm_changes.size()));
    for (const auto& [tick, bpm] : song.bpm_changes) {
        w.i64(tick);
        w.f64(bpm);
    }
    return std::move(w.bytes);
}

SongTiming decode_tempomap(const std::vector<uint8_t>& blob) {
    BinaryReader r(blob);
    int64_t res = r.i64();

    std::map<int64_t, int64_t> tpm;
    uint32_t ntpm = r.u32();
    for (uint32_t i = 0; i < ntpm; ++i) {
        int64_t tick = r.i64();
        tpm[tick] = r.i64();
    }

    std::map<int64_t, double> bpm;
    uint32_t nbpm = r.u32();
    for (uint32_t i = 0; i < nbpm; ++i) {
        int64_t tick = r.i64();
        bpm[tick] = r.f64();
    }

    return SongTiming(res, tpm, bpm);
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

const char* sort_column_name(SortColumn c) {
    switch (c) {
        case SortColumn::Score: return "score";
        case SortColumn::ActCount: return "actcount";
        case SortColumn::MaxSkip: return "maxskip";
        case SortColumn::HardestMs: return "hardest_ms";
        case SortColumn::AvgMult: return "avgmult";
        case SortColumn::NoteCount: return "notecount";
        case SortColumn::SqInCount: return "sqin_count";
        case SortColumn::SqOutCount: return "sqout_count";
        case SortColumn::PathCount: return "pathcount";
        case SortColumn::RefName: return "ref_name";
        case SortColumn::RefArtist: return "ref_artist";
        case SortColumn::RefCharter: return "ref_charter";
    }
    return "score";
}
bool sort_column_is_songmeta(SortColumn c) {
    return c == SortColumn::RefName || c == SortColumn::RefArtist ||
           c == SortColumn::RefCharter;
}

// The results table's columns. result_id is the rowid alias: a bigger one
// means "written later". rules_fp is the rules fingerprint the structure blob
// carries (rules_fp_of; path_codec.h has the layout), copied out so the UNIQUE key can hold
// it: one row per chart, mode, cap, lens and rules, so a result made under
// other rules sits beside this build's (schema 4, D51 call 8).
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
    "  structure   BLOB NOT NULL,"
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

}  // namespace

// Schema 2's results table, declared in record_store.h. The column list is
// what upgrade_results_key copies across; the rebuild fills the other two,
// legacy_fills and rules_fp. The table text's only reader is the store test,
// which builds a schema 2 file from it.
const char* const kSchema2ResultsColumns =
    "result_id, hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value, depth_mode,"
    " depth_value, bestpath, structure, score, actcount, maxskip, hardest_ms, avgmult,"
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
// They are a cache of the structure blob, kept as columns so the library can
// sort and filter without decoding a single record. bestpath belongs to the
// same cache, and so do stars and hardest_ms. prepare_row writes all of them
// from summarize_record, and reindex rewrites all of them, bestpath included,
// from the stored paths of every row this build can read. Nothing else writes them, except fill_missing_stars,
// which only fills a stars column an older Hydra left empty. A rule change
// that alters any of them bumps kResultsStamp (stored_versions.h), so every
// row written before it reads Stale and no old cached number is shown.
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
// two always agree. The INSERT, reindex's UPDATE and list_records work out
// their slots from it. bind_summary and read_summary walk the slots by hand,
// so the assert stops the build when the list grows and they don't.
constexpr int kSummaryColumnCount = count_list_names(kSummaryColumnList);
static_assert(kSummaryColumnCount == 10,
              "bind_summary and read_summary walk ten slots: grow them with the list");

// kSummaryColumnList with every name written as before + name + after,
// joined by ", ": "r.score, r.actcount, ..." or "score=?, actcount=?, ...".
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
// bind_lens's mirror: the five lens columns a query selected from `idx` on,
// in Lens's field order.
Lens read_lens(sqlite3_stmt* s, int idx) {
    Lens lens;
    lens.ms_enabled = sqlite3_column_int(s, idx);
    lens.ms_value = sqlite3_column_int(s, idx + 1);
    lens.depth_mode = sqlite3_column_int(s, idx + 2);
    lens.depth_value = sqlite3_column_int(s, idx + 3);
    lens.legacy_fills = sqlite3_column_int(s, idx + 4);
    return lens;
}

// The facts that decide whether a row is readable and how it places among
// the candidates for its chart and mode, and why a row that is not ready is
// Stale. rank_row is the only place they are read off a row. Whether a chart
// has a current result under some settings is get_summaries' answer, from its
// winner's ready() (D79).
struct Candidate {
    bool current = false;  // stamped with this build's results version
    bool format = false;   // this build's path-structure format, analyzed
                           // under the rules this process runs
    int64_t result_id = 0;
    // Why a row that is not ready is Stale, for callers that explain it
    // (hydra_replay dump, the song panel, the library row's tooltip). Both
    // can be true; neither is on a ready row. An older layout has no
    // fingerprint to compare, so it is only ever `stale_build`.
    bool stale_build = false;  // another Hydra build or an older path layout
    bool stale_rules = false;  // this layout, analyzed under other rules
    // Readable: stamped with this results version, in a layout this build
    // reads, under these rules. Anything else is Stale: another results
    // version's bytes, or a path tree whose activations this build would read
    // back half-empty.
    bool ready() const { return current && format; }
};

// `structure_head` is the whole blob or just the head a query selected
// (structure_head_of); path_codec.h read_structure_head reads it once.
Candidate rank_row(const std::string& hyversion, const std::vector<uint8_t>& structure_head,
                   int64_t result_id, const core::RulesStamp& rules) {
    const std::optional<StructureHead> head = read_structure_head(structure_head);
    const bool layout_current = head && kPathFormatStamp.is_current(head->path_format);
    const bool same_rules = layout_current && head->rules_fingerprint == rules.fixed;
    Candidate c;
    c.current = kResultsStamp.is_current(hyversion);
    c.format = same_rules;
    c.result_id = result_id;
    c.stale_build = !c.current || !layout_current;
    c.stale_rules = layout_current && !same_rules;
    return c;
}

// `n` comma-separated "?" placeholders.
std::string placeholders(size_t n) {
    std::string out;
    for (size_t i = 0; i < n; ++i) out += i ? ", ?" : "?";
    return out;
}

// One field of a structure blob's head spelled in SQL, from path_codec.h's
// offsets and widths. `blob` is a column name or a bound parameter. SQLite's
// substr counts bytes from 1, so it starts one past the 0-based offset.
std::string head_field_sql(const char* blob, size_t offset, size_t width) {
    return std::string("substr(") + blob + "," + std::to_string(offset + 1) + "," +
           std::to_string(width) + ")";
}

// The rules fingerprint inside a structure blob, spelled in SQL. The one SQL
// spelling of "which rules was this row analyzed under".
std::string rules_fp_of(const char* blob) {
    return head_field_sql(blob, kRulesFingerprintOffset, kRulesFingerprintBytes);
}
// The path format a structure blob is in, spelled in SQL.
std::string path_format_of(const char* blob) {
    return head_field_sql(blob, kPathFormatOffset, kPathFormatBytes);
}
// The whole head, the bytes rank_row reads, spelled in SQL. The head is the
// blob's first bytes.
std::string structure_head_of(const char* blob) {
    return head_field_sql(blob, 0, kStructureHeadBytes);
}

// A path format or a rules fingerprint as the bytes a structure blob holds
// it in, to bind against path_format_of or rules_fp_of.
std::vector<uint8_t> path_format_bytes(uint32_t format) {
    BinaryWriter w;
    w.u32(format);
    return w.bytes;
}
std::vector<uint8_t> rules_fp_bytes(uint64_t fingerprint) {
    BinaryWriter w;
    w.u64(fingerprint);
    return w.bytes;
}

// "This build can read the row at all", spelled in SQL for write_row's first
// purge, which deletes in the database the rows that fail it: this results
// version and a path format this build reads. It leaves out the rules, so a
// row made under other rules is kept (D51 call 8). Negate with "NOT ", never
// by spelling the opposite, so the rule has one SQL spelling. Both columns
// are NOT NULL, so NOT never meets a NULL.
//
// Built from the same StampRule lists is_current reads, so the two spellings
// cannot drift: every accepted results version and every accepted path
// format, the format where path_codec.h's head layout puts it.
// bind_readable_params binds them in that order and returns the next free
// index.
const std::string& row_readable_sql() {
    static const std::string sql =
        "(hyversion IN (" + placeholders(kResultsStamp.accepted.size()) + ") AND " +
        path_format_of("structure") + " IN (" + placeholders(kPathFormatStamp.accepted.size()) +
        "))";
    return sql;
}

int bind_readable_params(sqlite3_stmt* s, int idx) {
    for (std::string_view v : kResultsStamp.accepted) bind_text(s, idx++, std::string(v));
    for (uint32_t f : kPathFormatStamp.accepted) bind_blob(s, idx++, path_format_bytes(f));
    return idx;
}

// Does `a` beat `b`? This version before another, then this path format
// under these rules before the rest, then the newest write. Write order is
// result_id: add_row deletes and re-inserts, so a rewritten row is newest.
// Since Auto went (2026-09-27) every lookup names one exact cap and lens. The
// results table holds one row per cap, lens and rules, so a chart offers one
// candidate per set of rules it was analyzed under, and the one made under
// this process's rules wins.
bool outranks(const Candidate& a, const Candidate& b) {
    if (a.current != b.current) return a.current;
    if (a.format != b.format) return a.format;
    return a.result_id > b.result_id;
}

// Which chart a winner is picked for: one per chart and mode.
using GroupKey = std::pair<std::string, std::string>;

// The one owner of "which row wins". A lookup offers its candidate rows here
// in any order, then asks which offer won each chart and mode: get_summary
// and get_record for one chart, for_each_blob and list_records for many.
// Nothing else compares two rows.
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
    // The winning offer of a one-chart lookup, or nullopt when nothing was
    // offered.
    std::optional<size_t> only_winner() const {
        if (winner_.empty()) return std::nullopt;
        return winner_.begin()->second;
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

// The one way a stored row becomes a record: its structure blob, the path
// nodes it names, and the fill rule the row was filed under. The blob does
// not carry the fill rule, so every reader passes the one it knows: the
// key's lens (get_record), the walk's lens (for_each_blob) or the row's own
// legacy_fills column (reindex, fill_missing_stars). Timecodes are not
// restored here; get_record does that with the song's tempo map.
HydraRecord decode_record(const std::vector<uint8_t>& structure,
                          const std::unordered_map<std::string, std::vector<uint8_t>>& nodes,
                          bool legacy_fills) {
    HydraRecord record = rebuild_record(
        structure, [&nodes](const std::string& hash) -> const std::vector<uint8_t>* {
            auto it = nodes.find(hash);
            return it == nodes.end() ? nullptr : &it->second;
        });
    record.legacy_fills = legacy_fills;
    return record;
}

}  // namespace

// The bestpath column writes this text (prepare_row, and reindex when it
// rewrites a row), and hydra_replay's result block shows it.
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
    row.summary = summarize_record(record);

    FlatRecord flat = flatten_record(record);
    row.structure = std::move(flat.structure);
    row.nodes = std::move(flat.nodes);
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

RecordStore::RecordStore(const std::string& dbpath, core::RulesStamp rules_fingerprint)
    : rules_fingerprint_(rules_fingerprint) {
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
    // memory keeps its own type, which plain_error answers by.
    try {
        set_up_schema();
    } catch (const std::bad_alloc&) {
        close();
        throw;
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
    // synchronous is per connection, so both are set on every open. A
    // ":memory:" store answers "memory" and is unaffected.
    exec("PRAGMA journal_mode=WAL");
    exec("PRAGMA synchronous=NORMAL");

    exec(
        "CREATE TABLE IF NOT EXISTS songmeta ("
        "  hyhash      TEXT PRIMARY KEY,"
        "  ref_name    TEXT,"
        "  ref_artist  TEXT,"
        "  ref_charter TEXT,"
        "  tempomap    BLOB NOT NULL"
        ");"
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
        ");"
        "CREATE TABLE IF NOT EXISTS dynamics ("
        "  md5           TEXT NOT NULL,"
        "  difficulty    TEXT NOT NULL,"
        "  pro           INTEGER NOT NULL,"
        "  blob          BLOB NOT NULL,"
        "  count_version INTEGER NOT NULL DEFAULT 0,"
        "  PRIMARY KEY (md5, difficulty, pro)"
        ");");
    // A dynamics table from before the stamp gets the column. Its rows read
    // 0, which matches no real stamp, so each is recounted once.
    if (!has_column("dynamics", "count_version"))
        exec("ALTER TABLE dynamics ADD COLUMN count_version INTEGER NOT NULL DEFAULT 0");
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
    // A songmeta table from before stored lengths. Old rows read NULL.
    if (!has_column("songmeta", "length_ms")) exec("ALTER TABLE songmeta ADD COLUMN length_ms REAL");
    // A songmeta table from before the length stamp (kSongLengthStamp). Its
    // rows read 0, which matches no real stamp, so every length worked out
    // from notes reads as not read until it is worked out again (D75).
    if (!has_column("songmeta", "length_version"))
        exec("ALTER TABLE songmeta ADD COLUMN length_version INTEGER NOT NULL DEFAULT 0");
    // Each difficulty's own last-note length (D51 call 9), which D69
    // replaced with one length per song. Nothing reads it.
    exec("DROP TABLE IF EXISTS songlength");
    // The library page sorts by name (list_chart_library's ORDER BY name).
    exec("CREATE INDEX IF NOT EXISTS charts_by_name ON charts (name)");
    // Every save runs naming_copy_of_one_sql. Without this index it reads the
    // whole library table each time (D76).
    exec("CREATE INDEX IF NOT EXISTS charts_by_md5 ON charts (md5)");
    // Schema 2 = results keyed by the full settings, with shared paths. A
    // database from Hydra 1.6 or older still holds its old `records` table.
    // Nothing reads it (user decision 2026-09-26), so its charts read Not
    // analyzed until they are analyzed again.
    create_result_tables();
    // A results table from before the stars summary has no column for it.
    // Its Ready rows are filled below, from their stored paths; nothing is
    // analyzed again. A row that isn't Ready (another build, other rules) is
    // left alone and filled on a later open once it reads Ready. Added before
    // the schema 3 rebuild, which copies the column across.
    if (!has_column("results", "stars")) exec("ALTER TABLE results ADD COLUMN stars INTEGER");
    // Schema 3 = the fill rule joins a result's key; schema 4 = the rules
    // fingerprint does.
    upgrade_results_key();
    // Auto was removed (2026-09-27). Its results go the first time this
    // build opens the file, before the stars backfill below.
    delete_auto_results();
    fill_missing_stars();
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

bool RecordStore::has_column(const char* table, const char* column) {
    std::string sql = std::string("PRAGMA table_info(") + table + ")";
    Stmt info = prepare_read(db_, sql.c_str());
    while (step_row(info))
        if (column_text(info, 1) == column) return true;
    return false;
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

void RecordStore::put_dynamics(const DynamicsKey& key, const std::vector<uint8_t>& blob,
                               int count_version) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    insert_dynamics(key, blob, count_version);
}

void RecordStore::insert_dynamics(const DynamicsKey& key, const std::vector<uint8_t>& blob,
                                  int count_version) {
    CachedStmt s = use_write(db_, stmt_cache_,
        "INSERT OR REPLACE INTO dynamics (md5, difficulty, pro, blob, count_version)"
        " VALUES (?,?,?,?,?)");
    bind_text(s, 1, key.md5);
    bind_text(s, 2, key.difficulty);
    sqlite3_bind_int(s, 3, key.pro ? 1 : 0);
    bind_blob(s, 4, blob);
    sqlite3_bind_int(s, 5, count_version);
    step_done(s, "put_dynamics");
}

void RecordStore::save_analysis(const std::string& hyhash, const std::string& ref_name,
                                const std::string& ref_artist, const std::string& ref_charter,
                                const Song& song, const PreparedRow& row,
                                const std::optional<DynamicsEntry>& dynamics,
                                const SongLength& length) {
    const std::vector<uint8_t> tempomap = encode_tempomap(song);  // not a sqlite call
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
        upsert_song(hyhash, ref_name, ref_artist, ref_charter, tempomap);
        // The length belongs to the song, so an analysis of any difficulty
        // saves its one length (D75). With no read, it stays as it was.
        if (length.read) write_song_length(hyhash, length.ms);
        write_row(row);
        if (dynamics) {
            // Best effort, inside the same transaction: a failed count write
            // is undone on its own and never costs the result.
            ctl("SAVEPOINT dynamics");
            try {
                insert_dynamics(dynamics->key, dynamics->blob, dynamics->count_version);
                ctl("RELEASE dynamics");
            } catch (const std::exception&) {
                ctl("ROLLBACK TO dynamics");
                ctl("RELEASE dynamics");
            }
        }
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

std::optional<std::vector<uint8_t>> RecordStore::get_dynamics(const DynamicsKey& key) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // A row whose count stamp this build doesn't accept reads as missing, so
    // the caller recounts it and put_dynamics restamps it.
    Stmt s = prepare_read(db_,
        "SELECT blob, count_version FROM dynamics WHERE md5=? AND difficulty=? AND pro=?");
    bind_text(s, 1, key.md5);
    bind_text(s, 2, key.difficulty);
    sqlite3_bind_int(s, 3, key.pro ? 1 : 0);
    if (!step_row(s)) return std::nullopt;
    if (!kDynamicsCountStamp.is_current(sqlite3_column_int(s, 1))) return std::nullopt;
    return column_blob(s, 0);
}

void RecordStore::create_result_tables() {
    exec((std::string("CREATE TABLE IF NOT EXISTS results (") + kResultsColumnDefs + ");")
             .c_str());
    // A node belongs to one chart+mode: the same bytes under a different chart
    // are a different path, and scoping the table this way keeps the garbage
    // collection after a write to the rows that write could have orphaned.
    exec("CREATE TABLE IF NOT EXISTS paths ("
         "  hyhash    TEXT NOT NULL,"
         "  chartmode TEXT NOT NULL,"
         "  phash     TEXT NOT NULL,"
         "  payload   BLOB NOT NULL,"
         "  PRIMARY KEY (hyhash, chartmode, phash)"
         ");"
         // hyhash/chartmode are denormalized here on purpose: collecting a
         // chart's orphaned nodes must never have to decode a structure blob.
         "CREATE TABLE IF NOT EXISTS path_refs ("
         "  result_id INTEGER NOT NULL,"
         "  hyhash    TEXT NOT NULL,"
         "  chartmode TEXT NOT NULL,"
         "  phash     TEXT NOT NULL,"
         "  PRIMARY KEY (result_id, phash)"
         ");"
         "CREATE INDEX IF NOT EXISTS path_refs_by_node"
         "  ON path_refs (hyhash, chartmode, phash);");
}

// The SQL for the two per-row reads a walk repeats. Named here so
// for_each_blob can compile each of them once and reuse it, and the one-off
// callers still get the same text.
constexpr const char* kLoadNodesSql =
    "SELECT p.phash, p.payload FROM path_refs pr JOIN paths p"
    "  ON p.hyhash = pr.hyhash AND p.chartmode = pr.chartmode AND p.phash = pr.phash"
    " WHERE pr.result_id = ?";
constexpr const char* kReloadRowSql =
    "SELECT hyhash, chartmode, hyversion, sp_cap, structure,"
    " ms_enabled, ms_value, depth_mode, depth_value, legacy_fills"
    " FROM results WHERE result_id = ?";

std::unordered_map<std::string, std::vector<uint8_t>> RecordStore::load_nodes(
    sqlite3_stmt* stmt, int64_t result_id) {
    // `stmt` is kLoadNodesSql, compiled by the caller. Reset before returning,
    // so the caller may drop the lock the moment this comes back.
    ResetOnExit reset{stmt};
    std::unordered_map<std::string, std::vector<uint8_t>> nodes;
    sqlite3_bind_int64(stmt, 1, result_id);
    while (step_row(stmt))
        nodes.emplace(column_text(stmt, 0), column_blob(stmt, 1));
    return nodes;
}

std::unordered_map<std::string, std::vector<uint8_t>> RecordStore::load_nodes(
    int64_t result_id) {
    Stmt s = prepare_read(db_, kLoadNodesSql);
    return load_nodes(s, result_id);
}

bool RecordStore::reload_row(sqlite3_stmt* stmt, const BlobRow& meta, const Lens& lens,
                             int64_t result_id, std::vector<uint8_t>& structure) {
    // `stmt` is kReloadRowSql, compiled once by for_each_blob. Every path out
    // of here resets it first, including the skip paths below, so nothing is
    // left mid-step when the caller unlocks.
    ResetOnExit reset{stmt};
    sqlite3_bind_int64(stmt, 1, result_id);
    if (!step_row(stmt)) return false;  // deleted since the walk listed it
    // The row's whole identity (RecordKey), not just "a row is here".
    // result_id is a plain INTEGER PRIMARY KEY, so sqlite hands the same id
    // out again after a delete and a replacement row can occupy it.
    const RecordKey found{column_text(stmt, 0), column_text(stmt, 1),
                          CapQuery::at(sqlite3_column_int(stmt, 3)), read_lens(stmt, 5)};
    if (found != RecordKey{meta.hyhash, meta.chartmode, CapQuery::at(meta.sp_cap), lens})
        return false;
    // The stamp is not identity, but a row another build rewrote is not the
    // row the walk ranked either.
    if (column_text(stmt, 2) != meta.hyversion) return false;
    structure = column_blob(stmt, 4);
    return true;
}

void RecordStore::add_song(const std::string& hyhash, const std::string& ref_name,
                           const std::string& ref_artist, const std::string& ref_charter,
                           const Song& song) {
    // Encoded before the lock: the lock covers sqlite calls only.
    const std::vector<uint8_t> tempomap = encode_tempomap(song);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    upsert_song(hyhash, ref_name, ref_artist, ref_charter, tempomap);
}

void RecordStore::fill_song_length(const std::string& hyhash, std::optional<double> length_ms) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt read = prepare_read(db_, "SELECT length_version FROM songmeta WHERE hyhash = ?");
    bind_text(read, 1, hyhash);
    if (!step_row(read)) return;  // not registered: left alone
    const int stamp = sqlite3_column_int(read, 0);
    if (kSongLengthStamp.is_current(stamp)) return;  // an analysis or a backfill got here first
    // Written only while the stamp is still the one just read, so another
    // connection's write in between is kept.
    write_song_length(hyhash, length_ms, stamp);
}

void RecordStore::write_song_length(const std::string& hyhash, std::optional<double> length_ms,
                                    std::optional<int> only_from_stamp) {
    CachedStmt s = use_write(db_, stmt_cache_, only_from_stamp
        ? "UPDATE songmeta SET length_ms = ?1, length_version = ?2"
          " WHERE hyhash = ?3 AND length_version = ?4"
        : "UPDATE songmeta SET length_ms = ?1, length_version = ?2 WHERE hyhash = ?3");
    bind_opt_f64(s, 1, length_ms);
    sqlite3_bind_int(s, 2, kSongLengthStamp.written);
    bind_text(s, 3, hyhash);
    if (only_from_stamp) sqlite3_bind_int(s, 4, *only_from_stamp);
    step_done(s, "saving the song's length");
}

void RecordStore::upsert_song(const std::string& hyhash, const std::string& ref_name,
                              const std::string& ref_artist, const std::string& ref_charter,
                              const std::vector<uint8_t>& tempomap) {
    // A chart already registered takes the names this call carries, so a
    // fixed song.ini reaches the reports on the next analysis (user decision
    // 2026-09-26). The one exception is a chart with duplicate copies in the
    // library: the copy kNamingCopiesSql picks names it, whichever copy was
    // analyzed (D63, D51 call 10). Each analysis rewrites the tempo map too
    // (D51 call 12): the map is whatever the chart reader made of the file
    // this time, so a reader fix reaches the stored map on the next analysis
    // instead of never. The song's length is left alone: only
    // write_song_length writes it.
    std::string name = ref_name, artist = ref_artist, charter = ref_charter;
    {
        static const std::string sql = naming_copy_of_one_sql();
        CachedStmt copy = use_read(db_, stmt_cache_, sql);
        bind_text(copy, 1, hyhash);
        if (step_row(copy)) {
            name = column_text(copy, 0);
            artist = column_text(copy, 1);
            charter = column_text(copy, 2);
        }
    }
    CachedStmt s = use_write(db_, stmt_cache_,
        "INSERT INTO songmeta (hyhash, ref_name, ref_artist, ref_charter, tempomap) "
        "VALUES (?,?,?,?,?) "
        "ON CONFLICT(hyhash) DO UPDATE SET ref_name = excluded.ref_name, "
        "ref_artist = excluded.ref_artist, ref_charter = excluded.ref_charter, "
        "tempomap = excluded.tempomap");
    bind_text(s, 1, hyhash);
    bind_text(s, 2, name);
    bind_text(s, 3, artist);
    bind_text(s, 4, charter);
    bind_blob(s, 5, tempomap);
    step_done(s, "add_song");
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

void RecordStore::write_row(const PreparedRow& row) {
    // Deleting a result means deleting its refs first, always: the refs are
    // what keep its paths alive, and the final sweep collects whatever they
    // stopped pointing at. The caller holds the lock and an open transaction,
    // so a failure anywhere leaves the store exactly as it was.
    auto run = [](sqlite3_stmt* s, const char* what) {
        step_done(s, std::string("add_row ") + what);
    };
    // Deletes the results a subquery names, and their refs. `where` is a
    // fragment over `results`, bound by `bind`.
    auto purge = [&](const std::string& where,
                     const std::function<void(sqlite3_stmt*)>& bind, const char* what) {
        std::string refs = "DELETE FROM path_refs WHERE result_id IN"
                           " (SELECT result_id FROM results WHERE " + where + ")";
        CachedStmt r = use_write(db_, stmt_cache_, refs);
        bind(r);
        run(r, what);

        std::string rows = "DELETE FROM results WHERE " + where;
        CachedStmt d = use_write(db_, stmt_cache_, rows);
        bind(d);
        run(d, what);
    };

    // (1) Anything this chart+mode holds that this build can never read --
    //     another Hydra version's stamp (which includes every row an old
    //     migration left) or an older path layout -- is superseded by a write
    //     here. A result analyzed under other rules is not: it reads Ready
    //     again once the rules match (D51 call 8). The test is against what
    //     is current, not against this row: a test writing a deliberately
    //     old-stamped row must not take the real rows with it, and this runs
    //     before the insert so the new row is untouched.
    purge("hyhash=? AND chartmode=? AND NOT " + row_readable_sql(),
          [&](sqlite3_stmt* s) {
              bind_text(s, 1, row.hyhash);
              bind_text(s, 2, row.chartmode);
              bind_readable_params(s, 3);
          },
          "unreadable purge");

    // (2) The row this one replaces: the same key under the same rules, the
    //     row the UNIQUE key would refuse a second copy of. Deleted
    //     explicitly rather than by INSERT OR REPLACE: the refs bookkeeping
    //     has to be ours, and the re-insert must take a fresh result_id so
    //     the newest write ranks first. The rules compared are the new row's
    //     own, which are this store's for every real write.
    purge("hyhash=? AND chartmode=? AND sp_cap=? AND " + lens_match("") +
              " AND rules_fp = " + rules_fp_of("?"),
          [&](sqlite3_stmt* s) {
              bind_text(s, 1, row.hyhash);
              bind_text(s, 2, row.chartmode);
              sqlite3_bind_int(s, 3, row.sp_cap);
              const int next = bind_lens(s, 4, row.lens);
              bind_blob(s, next, row.structure);
          },
          "replace purge");

    // (3) The result, then its paths (shared, so first writer wins) and the
    //     refs that tie the two together. rules_fp is read out of the
    //     structure blob's own parameter, so the two cannot disagree.
    {
        // The row's own columns, structure last; the summary columns follow.
        static constexpr const char* kRowColumns =
            "hyhash, chartmode, hyversion, sp_cap, ms_enabled, ms_value, depth_mode,"
            " depth_value, legacy_fills, bestpath, structure";
        static constexpr int kRowColumnCount = count_list_names(kRowColumns);
        const std::string structure_param = "?" + std::to_string(kRowColumnCount);
        CachedStmt s = use_write(db_, stmt_cache_,
            std::string("INSERT INTO results (") + kRowColumns + ", " + kSummaryColumnList +
             ", rules_fp) VALUES (" + placeholders(kRowColumnCount) + ", " +
             placeholders(kSummaryColumnCount) + ", " + rules_fp_of(structure_param.c_str()) +
             ")");
        bind_text(s, 1, row.hyhash);
        bind_text(s, 2, row.chartmode);
        bind_text(s, 3, row.hyversion);
        sqlite3_bind_int(s, 4, row.sp_cap);
        bind_lens(s, 5, row.lens);
        bind_text(s, kRowColumnCount - 1, row.bestpath);
        bind_blob(s, kRowColumnCount, row.structure);
        bind_summary(s, kRowColumnCount + 1, row.summary);
        run(s, "insert");
    }
    const int64_t result_id = sqlite3_last_insert_rowid(db_);

    {
        // Taken from the statement cache once per row, not once per node, and
        // bound once with what every node shares. Bindings survive a reset, so
        // each node only rebinds its own hash and payload.
        CachedStmt path_insert = use_write(db_, stmt_cache_,
            "INSERT OR IGNORE INTO paths (hyhash, chartmode, phash, payload)"
            " VALUES (?,?,?,?)");
        CachedStmt ref_insert = use_write(db_, stmt_cache_,
            "INSERT OR IGNORE INTO path_refs (result_id, hyhash, chartmode, phash)"
            " VALUES (?,?,?,?)");
        bind_text(path_insert, 1, row.hyhash);
        bind_text(path_insert, 2, row.chartmode);
        sqlite3_bind_int64(ref_insert, 1, result_id);
        bind_text(ref_insert, 2, row.hyhash);
        bind_text(ref_insert, 3, row.chartmode);
        for (const StoredPathNode& node : row.nodes) {
            bind_text(path_insert, 3, node.hash);
            bind_blob(path_insert, 4, node.payload);
            run(path_insert, "path insert");
            sqlite3_reset(path_insert);
            bind_text(ref_insert, 4, node.hash);
            run(ref_insert, "path ref insert");
            sqlite3_reset(ref_insert);
        }
    }

    // (4) Whatever the replaced row was the last owner of.
    collect_orphan_paths(row.hyhash, row.chartmode, "add_row");
}

void RecordStore::collect_orphan_paths(const std::string& hyhash,
                                       const std::string& chartmode, const char* caller) {
    CachedStmt s = use_write(db_, stmt_cache_,
        "DELETE FROM paths WHERE hyhash=? AND chartmode=? AND phash NOT IN"
        " (SELECT phash FROM path_refs WHERE hyhash=? AND chartmode=?)");
    bind_text(s, 1, hyhash);
    bind_text(s, 2, chartmode);
    bind_text(s, 3, hyhash);
    bind_text(s, 4, chartmode);
    // Same message as before for write_row: "add_row path gc failed: ...".
    step_done(s, std::string(caller) + " path gc");
}

namespace {
// The meta key that marks the Auto results deleted for this file.
constexpr const char* kAutoResultsDeletedKey = "auto_results_deleted";
}  // namespace

void RecordStore::delete_auto_results() {
    // Under RulesStamp::none() (a bad hydra_rules.ini) there is no
    // fingerprint to look for. Leave the key unset, so the next start with
    // good rules does it.
    if (rules_fingerprint_.retired_auto == core::kNoRulesFingerprint) return;
    if (meta_get(kAutoResultsDeletedKey)) return;

    // An Auto row is known only by the rules fingerprint in its structure
    // blob's head (rules_fp_of).
    // Auto rows made under other rules are already Stale, and stay so: no
    // rules' own fingerprint is ever an Auto one. A new result for the same
    // chart, cap and lens outranks such a row (WinnerPicker).
    const std::vector<uint8_t> auto_fp = rules_fp_bytes(rules_fingerprint_.retired_auto);
    const std::string is_auto = rules_fp_of("structure") + " = ?";
    exec("BEGIN");
    try {
        // The charts that hold one, so their orphaned paths can be collected.
        std::vector<std::pair<std::string, std::string>> charts;
        {
            Stmt s = prepare_read(db_,
                             ("SELECT DISTINCT hyhash, chartmode FROM results WHERE " + is_auto).c_str());
            bind_blob(s, 1, auto_fp);
            while (step_row(s))
                charts.emplace_back(column_text(s, 0), column_text(s, 1));
        }
        // Refs first, always: they are what keep a result's paths alive.
        for (const std::string& sql :
             {"DELETE FROM path_refs WHERE result_id IN"
              " (SELECT result_id FROM results WHERE " + is_auto + ")",
              "DELETE FROM results WHERE " + is_auto}) {
            Stmt s = prepare_write(db_, sql.c_str());
            bind_blob(s, 1, auto_fp);
            step_done(s, "deleting Auto results");
        }
        for (const auto& [hyhash, chartmode] : charts)
            collect_orphan_paths(hyhash, chartmode, "Auto cleanup");
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
    // follow, then the structure head the ranking reads.
    static constexpr const char* kLeadColumns = "hyhash, hyversion, bestpath, result_id";
    static constexpr int kFirstSummary = count_list_names(kLeadColumns);
    static constexpr int kAfterSummary = kFirstSummary + kSummaryColumnCount;
    WinnerPicker picker;
    std::vector<Offered> offered;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        for (size_t first = 0; first < distinct.size(); first += kHashesPerQuery) {
            const size_t n = std::min(kHashesPerQuery, distinct.size() - first);
            std::string sql = std::string("SELECT ") + kLeadColumns + ", " + kSummaryColumnList +
                              ", " + structure_head_of("structure") +
                              " FROM results WHERE chartmode=? AND hyhash IN (" +
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
                                      sqlite3_column_int64(s, 3), rules_fingerprint_));
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

RecordLookup RecordStore::get_record(const RecordKey& key) {
    RecordLookup out;
    std::vector<uint8_t> structure;
    std::unordered_map<std::string, std::vector<uint8_t>> nodes;
    std::optional<SongMetaRead> songmeta;
    {
        // The lock covers the reads and nothing else. The winning row, the
        // nodes it names and the song's tempo map are all read under this one
        // lock, so a write in between can never pair one row's shape with
        // another's paths. Decoding happens after the lock is released: the
        // UI thread calls this, and a big record must not hold up the batch
        // writer.
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::string sql =
            "SELECT result_id, hyversion, structure FROM results"
            " WHERE hyhash=? AND chartmode=?";
        append_candidate_filter(sql, "");
        Stmt s = prepare_read(db_, sql.c_str());
        bind_text(s, 1, key.hyhash);
        bind_text(s, 2, key.chartmode);
        bind_candidate_filter(s, 3, key.cap, key.lens);

        struct Row {
            int64_t result_id = 0;
            std::string hyversion;
            std::vector<uint8_t> structure;
        };
        std::vector<Row> rows;  // by offer index
        WinnerPicker picker;
        while (step_row(s)) {
            Row row{sqlite3_column_int64(s, 0), column_text(s, 1), column_blob(s, 2)};
            // The whole blob is here, and rank_row reads only its head.
            picker.offer(key.hyhash, key.chartmode,
                         rank_row(row.hyversion, row.structure, row.result_id,
                                  rules_fingerprint_));
            rows.push_back(std::move(row));
        }
        const std::optional<size_t> won = picker.only_winner();
        if (!won) return RecordLookup{};
        Row& best = rows[*won];

        out.hyversion = best.hyversion;
        // Stamped by a different version or holding a path tree in an older
        // layout or under other rules: nothing stored is decoded at all.
        // Callers see Stale and prompt a re-analyze.
        const Candidate& rank = picker.rank(*won);
        if (!rank.ready()) {
            out.status = RecordStatus::Stale;
            out.stale_build = rank.stale_build;
            out.stale_rules = rank.stale_rules;
            return out;
        }
        structure = std::move(best.structure);
        nodes = load_nodes(best.result_id);
        songmeta = read_tempomap(key.hyhash);
    }

    out.status = RecordStatus::Ready;
    // The row matched the key's lens, so its fill rule is the key's.
    HydraRecord record = decode_record(structure, nodes, key.lens.legacy_fills == 1);
    // The tempomap is decoded once, here, and handed back with the record --
    // the display layer needs the same timing and must not query for it again.
    if (songmeta) {
        out.timing = decode_tempomap(songmeta->tempomap);
        restore_timecodes(record, *out.timing);
        // A length under another stamp is not read yet: the backfill works it
        // out again (SongLengthJob).
        out.song_length_read = kSongLengthStamp.is_current(songmeta->length_version);
        if (out.song_length_read) out.song_length_ms = songmeta->length_ms;
    }
    out.record = std::move(record);
    return out;
}

std::optional<RecordStore::SongMetaRead> RecordStore::read_tempomap(const std::string& hyhash) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Stmt s = prepare_read(db_,
                     "SELECT tempomap, length_ms, length_version FROM songmeta WHERE hyhash=?");
    bind_text(s, 1, hyhash);
    if (!step_row(s)) return std::nullopt;
    return SongMetaRead{column_blob(s, 0), column_opt_f64(s, 1), sqlite3_column_int(s, 2)};
}

std::optional<SongTiming> RecordStore::get_timing(const std::string& hyhash) {
    // Read under the lock, decoded outside it.
    const std::optional<SongMetaRead> meta = read_tempomap(hyhash);
    if (!meta) return std::nullopt;
    return decode_tempomap(meta->tempomap);
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

int RecordStore::copies_of(const std::unordered_map<std::string, int>& copies,
                           const std::string& md5) {
    const auto listed = copies.find(md5);
    return listed == copies.end() ? 1 : listed->second;
}

void RecordStore::for_each_blob(
    const std::optional<std::string>& chartmode, const CapQuery& cap, const Lens& lens,
    const std::function<void(const BlobRow&, const HydraRecord*)>& fn,
    const std::atomic<bool>* cancel) {
    // Two passes, and the lock is short in both. The first lists which rows to
    // visit, and their blobs, under one lock. The second takes the lock once
    // per row, just long enough to read that row's nodes -- and to re-read the
    // row itself, but only when something wrote in between -- then decodes and
    // calls fn with nothing held. A walk of the whole library used to hold the
    // lock end to end, so a click on the UI thread waited for the whole report.
    //
    // The two per-row reads share one statement each, compiled before the loop
    // and reset before every unlock, instead of being compiled per row --
    // 37,000 compilations on an 18.5k-record library, which cost more than the
    // shorter lock saved.
    struct Row {
        BlobRow meta;
        int64_t result_id = 0;
        std::vector<uint8_t> structure;
    };
    std::vector<Row> rows;
    // How many rows this connection had written when the listing below ran.
    // Phase 2 compares against it to decide whether the listing is still exact
    // -- see the comment at the per-row read.
    int64_t snapshot_changes = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        // The whole structure blob, not just its head. It is small in total
        // (a few megabytes across a big library) and reading it here means the
        // common walk -- nothing writing -- never re-reads a row.
        std::string sql =
            "SELECT s.hyhash, s.ref_name, s.ref_artist, s.ref_charter, "
            "r.chartmode, r.hyversion, r.sp_cap, r.result_id, r.structure "
            "FROM results r JOIN songmeta s ON s.hyhash = r.hyhash WHERE 1=1";
        if (chartmode) sql += " AND r.chartmode = ?";
        append_candidate_filter(sql, "r.");
        // Python's iter_blobs has no ORDER BY and gets insertion order from
        // sqlite's table scan; say so explicitly here.
        sql += " ORDER BY r.result_id";

        Stmt s = prepare_read(db_, sql.c_str());
        int idx = 1;
        if (chartmode) bind_text(s, idx++, *chartmode);
        bind_candidate_filter(s, idx, cap, lens);

        std::vector<Row> candidates;  // by offer index
        WinnerPicker picker;
        while (step_row(s)) {
            Row row;
            row.meta.hyhash = column_text(s, 0);
            row.meta.ref_name = column_text(s, 1);
            row.meta.ref_artist = column_text(s, 2);
            row.meta.ref_charter = column_text(s, 3);
            row.meta.chartmode = column_text(s, 4);
            row.meta.hyversion = column_text(s, 5);
            row.meta.sp_cap = sqlite3_column_int(s, 6);
            row.result_id = sqlite3_column_int64(s, 7);
            row.structure = column_blob(s, 8);
            // rank_row reads only the blob's head, so the whole blob serves.
            const Candidate rank = rank_row(row.meta.hyversion, row.structure,
                                            row.result_id, rules_fingerprint_);
            row.meta.status = rank.ready() ? RecordStatus::Ready : RecordStatus::Stale;
            picker.offer(row.meta.hyhash, row.meta.chartmode, rank);
            candidates.push_back(std::move(row));
        }

        // One row per chart and mode, the same one a lookup would pick, kept
        // in result_id order. A stale winner is still yielded: this is the
        // export path, and dropping a row here would lose it for good.
        const std::vector<bool> keep = picker.winners();
        for (size_t i = 0; i < candidates.size(); ++i)
            if (keep[i]) rows.push_back(std::move(candidates[i]));

        // Read under the same lock as the listing, so it names exactly the
        // database state the rows above came from.
        snapshot_changes = sqlite3_total_changes64(db_);
    }

    // Both statements are sqlite objects, so they are compiled, used, reset and
    // destroyed with the lock held. The guard is what makes the destroy happen
    // on every way out of this function -- the cancel return below, and a throw
    // out of decode_record or fn -- since a Stmt destroyed on a plain unwind
    // would finalize with no lock held.
    std::optional<Stmt> reload_stmt;
    std::optional<Stmt> nodes_stmt;
    struct StmtGuard {
        std::recursive_mutex& mutex;
        std::optional<Stmt>& reload_stmt;
        std::optional<Stmt>& nodes_stmt;
        ~StmtGuard() {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            reload_stmt.reset();  // optional::reset -- destroys, so finalizes
            nodes_stmt.reset();
        }
    } stmt_guard{mutex_, reload_stmt, nodes_stmt};
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        reload_stmt = prepare_read(db_, kReloadRowSql);
        nodes_stmt = prepare_read(db_, kLoadNodesSql);
    }

    for (Row& row : rows) {
        // Between records, with nothing held: the caller (app shutdown) gets
        // its thread back within one record instead of one library.
        if (cancel && cancel->load()) return;

        if (row.meta.status != RecordStatus::Ready) {
            fn(row.meta, nullptr);  // no lock: a stale row has nothing to read
            continue;
        }

        std::vector<uint8_t> structure;
        std::unordered_map<std::string, std::vector<uint8_t>> nodes;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            // Has anything been written on this connection since the listing?
            // sqlite3_total_changes64 counts rows this connection changed with
            // INSERT, UPDATE or DELETE, ever, and only goes up. (An INSERT OR
            // IGNORE that ignores counts nothing, which is exactly right here:
            // nothing changed, so the snapshot is still good.)
            //
            // When the count has not moved, no row can have been rewritten, so
            // the blob listed in phase 1 is still this row's blob and we use it
            // as is. That is the whole point of the check: re-reading every row
            // costs about 40% of the walk on a big library, and it only ever
            // matters when a write landed mid-walk -- which the common case (a
            // report right after a batch, nothing else writing) never does.
            //
            // Otherwise a write did land, so fall back to re-reading the row:
            // reload_row returns false when the row is gone or a different
            // record now sits on its id, and that chart is left out of this
            // walk rather than decoded against paths that are not its own. The
            // next walk picks it up.
            //
            // Either way the blob and the nodes it names come from inside one
            // lock, so a write between them can never pair one row's shape with
            // another's paths. Both calls reset their statement before they
            // return, so this block leaves no cursor open -- the skip path
            // included.
            if (sqlite3_total_changes64(db_) == snapshot_changes) {
                structure = std::move(row.structure);  // rows is not walked again
            } else if (!reload_row(*reload_stmt, row.meta, lens, row.result_id, structure)) {
                continue;
            }
            nodes = load_nodes(*nodes_stmt, row.result_id);
        }

        // Every row the walk lists matched its lens, so its fill rule is
        // the lens's.
        HydraRecord record = decode_record(structure, nodes, lens.legacy_fills == 1);
        fn(row.meta, &record);
    }
}

int RecordStore::reindex() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    struct Row {
        int64_t result_id;
        std::string hyversion;
        std::vector<uint8_t> structure;
        bool legacy_fills;
    };
    std::vector<Row> rows;
    {
        Stmt s = prepare_read(db_, "SELECT result_id, hyversion, structure, legacy_fills"
                                   " FROM results ORDER BY result_id");
        while (step_row(s))
            rows.push_back({sqlite3_column_int64(s, 0), column_text(s, 1), column_blob(s, 2),
                            sqlite3_column_int(s, 3) == 1});
    }

    // One transaction for the whole pass, and each statement compiled once.
    // This used to commit once per record: 18,000 commits on a full library.
    // Every summary column and bestpath is rewritten: they are one cache of
    // the blob (kSummaryColumnList).
    exec("BEGIN");
    try {
        Stmt nodes_stmt = prepare_read(db_, kLoadNodesSql);
        Stmt update = prepare_write(db_, ("UPDATE results SET " + summary_columns("", "=?") +
                                    ", bestpath=? WHERE result_id=?")
                                       .c_str());
        int done = 0;
        for (const Row& row : rows) {
            // A row this build can't read (another results stamp, path format
            // or rules fingerprint) is left untouched and not counted: there
            // is nothing to recompute from, and a result kept under other
            // rules (D51 call 8) keeps its cached columns (D55 item 3).
            if (!rank_row(row.hyversion, row.structure, row.result_id, rules_fingerprint_)
                     .ready())
                continue;
            const HydraRecord record = decode_record(
                row.structure, load_nodes(nodes_stmt, row.result_id), row.legacy_fills);
            ResetOnExit reset{update};
            bind_summary(update, 1, summarize_record(record));
            bind_text(update, kSummaryColumnCount + 1, best_path_text(record));
            sqlite3_bind_int64(update, kSummaryColumnCount + 2, row.result_id);
            step_done(update, "reindex");
            ++done;
        }
        exec("COMMIT");
        return done;
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

int RecordStore::fill_missing_stars() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // Rows with a score but no stars: written before the column existed. Only
    // Ready rows are filled. The rest are skipped, never blanked, as reindex
    // skips them: under a bad hydra_rules.ini every row reads Stale, and
    // blanking them would wipe the whole library's scores on one bad start.
    std::vector<int64_t> ids;
    {
        Stmt s = prepare_read(db_, ("SELECT result_id, hyversion, " + structure_head_of("structure") +
                                    " FROM results"
                                    " WHERE stars IS NULL AND score IS NOT NULL ORDER BY result_id")
                                       .c_str());
        while (step_row(s)) {
            const int64_t id = sqlite3_column_int64(s, 0);
            if (rank_row(column_text(s, 1), column_blob(s, 2), id, rules_fingerprint_).ready())
                ids.push_back(id);
        }
    }
    if (ids.empty()) return 0;

    // One transaction, each statement compiled once, as in reindex. Only the
    // stars column is written: every other summary stays byte for byte.
    exec("BEGIN");
    try {
        Stmt structure_stmt =
            prepare_read(db_, "SELECT structure, legacy_fills FROM results WHERE result_id=?");
        Stmt nodes_stmt = prepare_read(db_, kLoadNodesSql);
        Stmt update = prepare_write(db_, "UPDATE results SET stars=? WHERE result_id=?");
        int filled = 0;
        for (int64_t id : ids) {
            std::vector<uint8_t> structure;
            bool legacy_fills = false;
            {
                ResetOnExit reset{structure_stmt};
                sqlite3_bind_int64(structure_stmt, 1, id);
                if (!step_row(structure_stmt)) continue;
                structure = column_blob(structure_stmt, 0);
                legacy_fills = sqlite3_column_int(structure_stmt, 1) == 1;
            }
            const PathSummary summary = summarize_record(
                decode_record(structure, load_nodes(nodes_stmt, id), legacy_fills));
            ResetOnExit reset{update};
            if (summary.stars) sqlite3_bind_int(update, 1, *summary.stars);
            else sqlite3_bind_null(update, 1);
            sqlite3_bind_int64(update, 2, id);
            step_done(update, "fill_missing_stars");
            ++filled;
        }
        exec("COMMIT");
        return filled;
    } catch (...) {
        rollback_if_open(db_);
        throw;
    }
}

std::vector<RecordListing> RecordStore::list_records(
    const std::optional<std::string>& chartmode, const CapQuery& cap, const Lens& lens,
    SortColumn order_by, bool descending, std::optional<int> limit) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // The listing's own columns, read below at 0 onwards; the summary columns
    // follow, then the four the ranking reads.
    static constexpr const char* kLeadColumns =
        "s.hyhash, s.ref_name, s.ref_artist, s.ref_charter, r.chartmode, r.bestpath";
    static constexpr int kFirstSummary = count_list_names(kLeadColumns);
    static constexpr int kAfterSummary = kFirstSummary + kSummaryColumnCount;
    std::string sql = std::string("SELECT ") + kLeadColumns + ", " +
                      summary_columns("r.", "") + ", r.sp_cap, r.hyversion, r.result_id, " +
                      structure_head_of("r.structure") +
                      " FROM results r JOIN songmeta s ON s.hyhash = r.hyhash WHERE 1=1";
    if (chartmode) sql += " AND r.chartmode = ?";
    append_candidate_filter(sql, "r.");

    // The sort stays in SQL, so the listing keeps sqlite's own ordering; the
    // passes below only drop rows, never reorder them. The limit cannot stay
    // here: a SQL LIMIT would count rows that are about to be dropped and hand
    // back fewer than the caller asked for.
    const char* prefix = sort_column_is_songmeta(order_by) ? "s." : "r.";
    sql += " ORDER BY ";
    sql += prefix;
    sql += sort_column_name(order_by);
    sql += descending ? " DESC" : " ASC";

    Stmt s = prepare_read(db_, sql.c_str());
    int idx = 1;
    if (chartmode) bind_text(s, idx++, *chartmode);
    bind_candidate_filter(s, idx, cap, lens);

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
                              sqlite3_column_int64(s, kAfterSummary + 2), rules_fingerprint_));
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
    Stmt songs = prepare_read(db_, "SELECT COUNT(*) FROM songmeta");
    const int64_t nsongs = step_row(songs) ? sqlite3_column_int64(songs, 0) : 0;

    Stmt records = prepare_read(db_, "SELECT COUNT(*) FROM results");
    const int64_t nrecords = step_row(records) ? sqlite3_column_int64(records, 0) : 0;

    return {nsongs, nrecords};
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
        // Song names follow song.ini (user decision 2026-09-26): a chart that
        // already has a song row takes the names this scan read. When the
        // scan found the same chart twice, the first copy it listed names it
        // (kNamingCopiesSql).
        exec((std::string("UPDATE songmeta SET ref_name = c.name, ref_artist = c.artist,"
                          " ref_charter = c.charter FROM ") +
              kNamingCopiesSql + " AS c WHERE songmeta.hyhash = c.md5")
                 .c_str());
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
        if (sig.empty()) continue;
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

std::vector<ChartLibraryEntry> RecordStore::list_chart_library(int offset, int limit) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // Rows an older scan wrote never read their timing: none, not "none
    // stated".
    const bool timing_read = chart_meta_current();
    Stmt s = prepare_read(db_,
                          "SELECT md5, name, artist, charter, path, folder, sig, stated_length_ms,"
                          " delay_ms FROM charts ORDER BY name LIMIT ? OFFSET ?");
    sqlite3_bind_int(s, 1, limit);
    sqlite3_bind_int(s, 2, offset);

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

}  // namespace hydra::store
