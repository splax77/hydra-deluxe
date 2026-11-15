// How SQLite plans the statements a per-chart write runs. A batch saves every
// chart it analyzes, so a statement there that reads all of records or
// record_paths costs one whole-table read per chart: minutes on a big
// library, nothing on a test's small one. Timing a small store cannot see
// that, so these cases ask SQLite instead. They record the SQL of every
// statement the real write runs and fail when its EXPLAIN QUERY PLAN reads
// a whole large table.
//
// A plan says how SQLite means to read; the work-count case checks what a save
// actually did. SQLite counts the operations each statement runs
// (sqlite3_stmt_status), and saving one chart into two stores that differ only
// in how many records they hold must count the same operations in both.
//
// The next per-chart write gets its own case here, reusing the recorder and
// check_no_full_reads.
#include "doctest.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <map>
#include <mutex>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "store/record_store.h"
#include "store_fixtures.h"  // fixture, at_cap, at4
#include "temp_util.h"

using namespace hydra;
using namespace hydra::store;

using hydra::test::at4;
using hydra::test::at_cap;
using hydra::test::fixture;

namespace {

// The tables a library holds one row or more per chart in. A read of all of
// one is what these cases catch.
const std::vector<std::string> kLargeTables = {"records", "record_paths"};

// The work SQLite counted for a statement's runs, summed over them.
struct StatementWork {
    int64_t vm_steps = 0;        // SQLITE_STMTSTATUS_VM_STEP
    int64_t fullscan_steps = 0;  // SQLITE_STMTSTATUS_FULLSCAN_STEP
};

// What every traced connection writes into while a StatementRecorder lives.
// It outlives every connection, so a trace left on a store that closes later
// never writes into a dead object.
struct Recording {
    std::mutex mutex;
    bool active = false;
    std::vector<std::string> statements;  // distinct, in the order first run
    std::set<std::string> seen;
    std::map<std::string, StatementWork> work;  // by SQL text
};

Recording& recording() {
    static Recording r;
    return r;
}

// SQLite calls this when a statement's run finishes (SQLITE_TRACE_PROFILE),
// after the run's operations are added to its counters. Each counter is read
// with reset on every run, recorded or not: the store caches its statements,
// so a counter left unread would carry one run's work into the next.
void on_profile(sqlite3_stmt* stmt) {
    const int vm = sqlite3_stmt_status(stmt, SQLITE_STMTSTATUS_VM_STEP, 1);
    const int scan = sqlite3_stmt_status(stmt, SQLITE_STMTSTATUS_FULLSCAN_STEP, 1);
    const char* sql = sqlite3_sql(stmt);
    if (!sql) return;
    Recording& r = recording();
    std::lock_guard<std::mutex> lock(r.mutex);
    if (!r.active) return;
    StatementWork& w = r.work[sql];
    w.vm_steps += vm;
    w.fullscan_steps += scan;
}

int on_trace(unsigned type, void*, void* p, void*) {
    if (type == SQLITE_TRACE_PROFILE) {
        on_profile(static_cast<sqlite3_stmt*>(p));
        return 0;
    }
    if (type != SQLITE_TRACE_STMT) return 0;
    const char* sql = sqlite3_sql(static_cast<sqlite3_stmt*>(p));
    // A trigger's program reports itself as a "--" comment line; its own
    // statements are not separate SQL to plan.
    if (!sql || std::string(sql).rfind("--", 0) == 0) return 0;
    Recording& r = recording();
    std::lock_guard<std::mutex> lock(r.mutex);
    if (r.active && r.seen.insert(sql).second) r.statements.push_back(sql);
    return 0;
}

// Runs for every connection SQLite opens while it is registered, the store's
// own included, so the store needs no seam that hands its handle out.
int trace_new_connection(sqlite3* db, char**, const sqlite3_api_routines*) {
    sqlite3_trace_v2(db, SQLITE_TRACE_STMT | SQLITE_TRACE_PROFILE, on_trace, nullptr);
    return SQLITE_OK;
}

void (*entry_point())(void) {
    // SQLite's documented calling shape for an automatic extension.
    return reinterpret_cast<void (*)(void)>(&trace_new_connection);
}

// Records the SQL text of every statement run on any connection opened while
// it lives, and the work each one did. Make it before the store, so the
// store's connection is traced from its open.
class StatementRecorder {
public:
    StatementRecorder() {
        Recording& r = recording();
        {
            std::lock_guard<std::mutex> lock(r.mutex);
            r.active = true;
            r.statements.clear();
            r.seen.clear();
            r.work.clear();
        }
        REQUIRE(sqlite3_auto_extension(entry_point()) == SQLITE_OK);
    }
    ~StatementRecorder() {
        sqlite3_cancel_auto_extension(entry_point());
        Recording& r = recording();
        std::lock_guard<std::mutex> lock(r.mutex);
        r.active = false;
    }
    StatementRecorder(const StatementRecorder&) = delete;
    StatementRecorder& operator=(const StatementRecorder&) = delete;

    // Forgets what was recorded so far, so the store's own open is not read
    // as part of the write under test.
    void clear() {
        Recording& r = recording();
        std::lock_guard<std::mutex> lock(r.mutex);
        r.statements.clear();
        r.seen.clear();
        r.work.clear();
    }
    std::vector<std::string> statements() const {
        Recording& r = recording();
        std::lock_guard<std::mutex> lock(r.mutex);
        return r.statements;
    }
    std::map<std::string, StatementWork> work() const {
        Recording& r = recording();
        std::lock_guard<std::mutex> lock(r.mutex);
        return r.work;
    }
};

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// The names a plan row may give `table` in `sql`: the table's own and any
// alias the statement gives it (a plan row names an aliased table by its
// alias).
std::vector<std::string> plan_names(const std::string& sql, const std::string& table) {
    static const std::set<std::string> kNotAliases = {
        "WHERE", "SET",     "JOIN",  "ON",    "VALUES", "USING", "LEFT",   "INNER",
        "CROSS", "NATURAL", "ORDER", "GROUP", "LIMIT",  "UNION", "INDEXED", "NOT",
        "AND",   "OR",      "DEFAULT", "SELECT", "AS",  "RETURNING"};
    std::vector<std::string> out = {table};
    const std::regex alias("\\b(?:FROM|JOIN|UPDATE|INTO)\\s+" + table +
                               "\\s+(?:AS\\s+)?([A-Za-z_][A-Za-z0-9_]*)",
                           std::regex::icase);
    for (std::sregex_iterator it(sql.begin(), sql.end(), alias), end; it != end; ++it) {
        const std::string name = (*it)[1].str();
        if (!kNotAliases.count(upper(name))) out.push_back(name);
    }
    return out;
}

// Whether `sql` names `table` as a whole word.
bool names_table(const std::string& sql, const std::string& table) {
    return std::regex_search(sql, std::regex("\\b" + table + "\\b", std::regex::icase));
}

// Whether the plan row `detail` reads every row of the table a plan row calls
// `name`. A SCAN row walks the whole table (or a whole index of it). An
// automatic index is one SQLite builds for this statement by reading the
// whole table first, so it costs the same.
bool reads_whole(const std::string& detail, const std::string& name) {
    for (const std::string& lead : {"SCAN " + name, "SCAN TABLE " + name})
        if (detail == lead || detail.rfind(lead + " ", 0) == 0) return true;
    const std::string searched = "SEARCH " + name + " ";
    return detail.rfind(searched, 0) == 0 && detail.find("AUTOMATIC") != std::string::npos;
}

// The EXPLAIN QUERY PLAN rows of `sql` on `db`, each row's detail text.
std::vector<std::string> plan_rows(sqlite3* db, const std::string& sql) {
    sqlite3_stmt* s = nullptr;
    const std::string eqp = "EXPLAIN QUERY PLAN " + sql;
    const int rc = sqlite3_prepare_v2(db, eqp.c_str(), -1, &s, nullptr);
    INFO("statement: " << sql);
    INFO("sqlite: " << sqlite3_errmsg(db));
    REQUIRE(rc == SQLITE_OK);
    std::vector<std::string> out;
    while (sqlite3_step(s) == SQLITE_ROW) {
        const unsigned char* detail = sqlite3_column_text(s, 3);
        out.emplace_back(detail ? reinterpret_cast<const char*>(detail) : "");
    }
    sqlite3_finalize(s);
    return out;
}

// Fails once for every plan row of `statements` that reads all of a large
// table, naming the statement and the row. Plans are asked of a connection of
// its own on the file at `dbpath`, which holds the same tables and indexes as
// the store's. Returns how many statements touch a large table, so a case can
// require that its write was recorded at all.
int check_no_full_reads(const std::string& dbpath, const std::vector<std::string>& statements) {
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open_v2(dbpath.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK);
    int touched = 0;
    for (const std::string& sql : statements) {
        bool touches = false;
        for (const std::string& table : kLargeTables) {
            if (!names_table(sql, table)) continue;
            touches = true;
            const std::vector<std::string> rows = plan_rows(db, sql);
            for (const std::string& name : plan_names(sql, table))
                for (const std::string& row : rows)
                    if (reads_whole(row, name))
                        FAIL_CHECK("this statement reads every row of " << table << "\n  statement: "
                                                                        << sql << "\n  plan row: "
                                                                        << row);
        }
        if (touches) ++touched;
    }
    sqlite3_close(db);
    return touched;
}

// A file store in the temp folder, removed when the case ends and before it
// starts, so a crashed run's file never adds records. A file, not ":memory:",
// so check_no_full_reads can open the same tables. `tag` names the file, so
// one case can hold two stores.
struct FileStore {
    std::string path;
    explicit FileStore(const std::string& tag = "store_query_plans")
        : path(testtemp::temp_path(tag, ".db")) {
        remove();
    }
    ~FileStore() { remove(); }
    FileStore(const FileStore&) = delete;
    FileStore& operator=(const FileStore&) = delete;
    void remove() {
        for (const char* ext : {"", "-wal", "-shm"}) std::remove((path + ext).c_str());
    }
};

// How many records the large profile holds in the plan cases' store, and in
// the smaller of the work-count case's two. Fixture size only: no check
// depends on it.
constexpr int kManyRecords = 20;

// Fills `store` with several profiles: one holding `many` records, one
// holding a single record, and one at another cap holding a single record. A
// statement that walks records once per profile then has more than one
// profile to walk. The single records' song id sorts after every "many" one,
// so a read that walks records in key order to find them passes all of those
// first.
void fill_profiles(RecordStore& store, int many = kManyRecords) {
    for (int i = 0; i < many; ++i)
        store.save(prepare_record("many" + std::to_string(i), at4("guitar"), fixture().record));
    store.save(prepare_record("single", at4("drums"), fixture().record));
    store.save(prepare_record("single", Profile{"guitar", CapQuery::at(6), Lens{}}, at_cap(6)));
}

// One new chart and one replaced chart, the two shapes a batch's save takes.
void save_two_charts(RecordStore& store) {
    store.save(prepare_record("new_chart", at4("guitar"), fixture().record));
    store.save(prepare_record("single", at4("drums"), fixture().record));
}

// The work every statement did while save_two_charts ran on `store`, alone or
// inside a save group as `grouped` says.
std::map<std::string, StatementWork> work_of_save(StatementRecorder& recorder, RecordStore& store,
                                                  bool grouped) {
    recorder.clear();
    if (grouped) store.begin_save_group();
    save_two_charts(store);
    if (grouped) store.commit_save_group();
    return recorder.work();
}

}  // namespace

TEST_CASE("store query plans: a chart's save reads records and record_paths only by key") {
    FileStore file;
    StatementRecorder recorder;
    RecordStore store(file.path);
    fill_profiles(store);

    SUBCASE("saved alone, in a transaction of its own") {
        recorder.clear();
        save_two_charts(store);
    }
    SUBCASE("saved inside a save group, as run_batch saves") {
        recorder.clear();
        store.begin_save_group();
        save_two_charts(store);
        store.commit_save_group();
    }

    const std::vector<std::string> statements = recorder.statements();
    // The save's writes were recorded, so the check below read them.
    CHECK(check_no_full_reads(file.path, statements) > 0);
}

TEST_CASE("store query plans: a chart's save does the same work however many records the store "
          "holds") {
    // Two stores with the same profiles, the larger's large profile holding
    // ten times the records. Any two different counts would do.
    FileStore smaller_file("store_work_smaller");
    FileStore larger_file("store_work_larger");
    StatementRecorder recorder;
    RecordStore smaller(smaller_file.path);
    RecordStore larger(larger_file.path);
    fill_profiles(smaller, kManyRecords);
    fill_profiles(larger, 10 * kManyRecords);

    bool grouped = false;
    SUBCASE("saved alone, in a transaction of its own") { grouped = false; }
    SUBCASE("saved inside a save group, as run_batch saves") { grouped = true; }

    const std::map<std::string, StatementWork> in_smaller = work_of_save(recorder, smaller, grouped);
    const std::map<std::string, StatementWork> in_larger = work_of_save(recorder, larger, grouped);
    // The save's statements were recorded, so the checks below read them.
    REQUIRE(!in_smaller.empty());

    // Every statement either store ran does the same work in both. The
    // full-scan count is compared, not required to be zero: the save may
    // read all of a table that holds no row per chart (the unreadable purge
    // reads all of profiles), and the counter counts that scan too.
    std::set<std::string> sqls;
    for (const auto& entry : in_smaller) sqls.insert(entry.first);
    for (const auto& entry : in_larger) sqls.insert(entry.first);
    int64_t steps_smaller = 0;
    int64_t steps_larger = 0;
    for (const std::string& sql : sqls) {
        const auto a = in_smaller.find(sql);
        const auto b = in_larger.find(sql);
        const StatementWork wa = a == in_smaller.end() ? StatementWork{} : a->second;
        const StatementWork wb = b == in_larger.end() ? StatementWork{} : b->second;
        steps_smaller += wa.vm_steps;
        steps_larger += wb.vm_steps;
        if (wa.vm_steps != wb.vm_steps || wa.fullscan_steps != wb.fullscan_steps)
            FAIL_CHECK("this statement's work grows with the records the store holds"
                       << "\n  statement: " << sql << "\n  operations: " << wa.vm_steps
                       << " in the smaller store, " << wb.vm_steps << " in the larger"
                       << "\n  full-scan steps: " << wa.fullscan_steps << " in the smaller store, "
                       << wb.fullscan_steps << " in the larger");
    }
    INFO("operations the whole save ran: " << steps_smaller << " in the smaller store, "
                                           << steps_larger << " in the larger");
    REQUIRE(steps_smaller == steps_larger);
}
