// Timing harness for the GUI's analysis path. The Hydra GUI calls
// app::analyze_chart_file -> hydra::analyze_chart -> run_search, i.e. the
// object-based C++ engine in src/search/engine.cpp, then serialize the record
// into the RecordStore. This times exactly that.
//
// With a folder argument (and an optional --rules <path>) it discovers the
// folder's charts and prints a per-chart breakdown at the GUI's default
// settings, taken from app::Settings so the two cannot drift -- parse,
// search, and DB store timed separately:
//   hydra_bench.exe "C:\Clone Hero\songs\...\blink-182 - Discography"
// With no argument it best-of-3 times the testdata corpus search at one
// fixed config, labelled "cap4 d4 no-ms" (corpus_bench).
//
// --engine and --parse print one digest over a whole folder, so two builds
// can be compared for identical output (engine_mode and parse_mode below say
// what each covers). The digests themselves live in tests/song_digest.h,
// which tests/test_perf_digest.cpp shares to pin the corpus's values.
//
// --replay times the replay on one chart and --upgrade times a database
// upgrade (replay_mode and upgrade_mode below).
//
// --open times opening a library the way the app opens it, part by part, beside
// prototypes of faster reads checked against today's rows (open_mode below).

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <sqlite3.h>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/library_query.h"
#include "app/rules_file.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/strutil.h"
#include "core/unknown_option.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "replay_windows.h"
#include "search/graph.h"
#include "search/pather.h"
#include "song_digest.h"
#include "store/record_store.h"
#include "ui/library_jobs.h"
#include "ui/library_model.h"

using namespace hydra;
using clk = std::chrono::steady_clock;

// hydra_rules.ini (or --rules <path>): the rule choices every mode runs under.
static core::Rules g_rules;
// app::kLegacyFillsFlag, given anywhere: --engine and --parse run under Clone
// Hero 1.0's fill rule (app::Settings::load_for_command_line).
static bool g_legacy_fills = false;

static double secs_since(clk::time_point t0) {
    return std::chrono::duration<double>(clk::now() - t0).count();
}

// Folder mode: the real GUI default (app::Settings) under `rules`, broken into
// the phases the app actually pays, per chart.
static void folder_breakdown(const std::string& folder, const core::Rules& rules) {
    auto [items, errors] = app::discover_charts({folder});
    std::printf("Discovered %zu chart(s) (%zu folder error(s)).\n", items.size(),
                errors.size());
    app::Settings gui;  // struct defaults are the GUI defaults
    gui.rules = rules;
    const app::AnalysisSettings settings = gui.to_analysis_settings();
    const app::SettingsText words = app::describe_settings(settings);
    std::printf("Settings: the GUI default (SP cap %s, depth %s, timing cap %s).\n\n",
                words.cap.c_str(), words.depth.c_str(), words.timing.c_str());

    store::RecordStore store(":memory:", store::OldFilePolicy::Refuse, core::RulesStamp::of(rules));

    for (const app::ScanItem& it : items) {
        std::printf("%s\n", it.notespath.c_str());

        auto t = clk::now();
        std::optional<Song> song_opt;
        try {
            song_opt.emplace(load_songpath(it.notespath, settings.reading(), settings.rules,
                                           fill_rule_for(settings.legacy_fills)));
        } catch (const std::exception& e) {
            std::printf("  (skipped: %s)\n\n", e.what());
            continue;
        }
        const Song& song = *song_opt;
        double parse_s = secs_since(t);

        t = clk::now();
        HydraRecord rec = analyze_chart(song, settings);
        double search_s = secs_since(t);

        t = clk::now();
        const store::PreparedRecord prepared = store::prepare_record(it.md5, gui.profile(), rec);
        store.save(prepared);
        double store_s = secs_since(t);

        // The record's rank-1 score: none for a record with no paths,
        // printed "-" the way hydra_batch prints it.
        const store::PathSummary summary = prepared.record.summary();
        const std::string best_text =
            summary.has_scored_best_path() ? std::to_string(*summary.score) : "-";
        std::printf("  parse %.2fs | search %.2fs | store %.2fs  => TOTAL %.2fs\n",
                    parse_s, search_s, store_s, parse_s + search_s + store_s);
        std::printf("  best score %s | %s | sp_cap %d\n\n", best_text.c_str(),
                    counted(static_cast<int64_t>(rec.all_paths().size()), "path", "paths").c_str(),
                    rec.sp_cap.value_or(-1));
        std::fflush(stdout);
    }
}

// One library row as the JSON both --dump and --dump-db write, so a scan's dump
// and a database's dump diff clean. Paths are written as the scan snapshot
// keys them (relative_slash_path under `rel`, which may be empty), so dumps
// also compare across machines.
static nlohmann::json row_json(const std::string& notespath, const std::string& rootfolder,
                               const std::string& md5, const std::string& title,
                               const std::string& artist, const std::string& charter,
                               const std::string& rel) {
    return {{"path", relative_slash_path(notespath, rel)},
            {"folder", relative_slash_path(rootfolder, rel)},
            {"md5", md5},
            {"title", title},
            {"artist", artist},
            {"charter", charter}};
}

// Scan mode: times the library scan (discovery + hashing) the way ScanJob
// runs it, without any chart analysis. Optionally writes the discovered rows
// into a store (--db, timing the library rebuild and exercising the rescan
// cache on a second run) and/or dumps the items as JSON for equivalence
// diffs (--dump; --dump-rel makes paths relative to the given root, forward
// slashes, so dumps compare across machines).
static void scan_mode(const std::string& folder, const std::string& dbpath,
                      const std::string& dumppath, const std::string& dumprel) {
    std::printf("Scanning %s\n", folder.c_str());

    std::unique_ptr<store::RecordStore> store;
    if (!dbpath.empty())
        store = std::make_unique<store::RecordStore>(dbpath, store::OldFilePolicy::Refuse,
                                                     core::RulesStamp::of(g_rules));

    // With --db, a prior scan's rows become the rescan cache — running the
    // same command twice measures cold full scan then warm rescan.
    const store::ChartLibraryCache cache =
        store ? app::load_scan_cache(*store) : store::ChartLibraryCache{};
    if (!cache.empty()) std::printf("  (rescan cache: %zu rows)\n", cache.size());

    int folders_seen = 0, cached = 0;
    double enumerate_s = 0.0;
    auto t0 = clk::now();
    app::ScanCallbacks callbacks;
    callbacks.on_folders = [&](int n) { folders_seen = n; };
    callbacks.on_charts = [&](int done, int, int cached_now) {
        if (done == 0) enumerate_s = secs_since(t0);  // walk finished, reads start
        cached = cached_now;
    };
    auto [items, errors] = app::discover_charts({folder}, callbacks, cache);
    double scan_s = secs_since(t0);

    std::printf("  folders %d | charts %zu | cached %d | errors %zu\n", folders_seen,
                items.size(), cached, errors.size());
    std::printf("  enumerate     : %7.2fs\n", enumerate_s);
    std::printf("  read+hash     : %7.2fs\n", scan_s - enumerate_s);
    std::printf("  discover total: %7.2fs\n", scan_s);
    for (size_t i = 0; i < errors.size() && i < 10; ++i)
        std::printf("  ! %s\n", errors[i].c_str());
    if (errors.size() > 10) std::printf("  ! ...and %zu more\n", errors.size() - 10);

    if (store) {
        t0 = clk::now();
        // Saved the way Scan library saves it (D79).
        if (const std::optional<std::string> problem = app::save_scan_as_library(*store, items))
            std::printf("  ! %s\n", problem->c_str());
        std::printf("  library write : %7.2fs (%lld rows)\n", secs_since(t0),
                    static_cast<long long>(store->chart_library_count()));
    }

    if (!dumppath.empty()) {
        nlohmann::json arr = nlohmann::json::array();
        std::vector<const app::ScanItem*> sorted;
        for (const app::ScanItem& it : items) sorted.push_back(&it);
        std::sort(sorted.begin(), sorted.end(),
                  [](const app::ScanItem* a, const app::ScanItem* b) {
                      return a->notespath < b->notespath;
                  });
        for (const app::ScanItem* it : sorted)
            arr.push_back(row_json(it->notespath, it->rootfolder, it->md5, it->title,
                                   it->artist, it->charter, dumprel));
        // Real libraries carry ANSI-encoded song.ini metadata; replace
        // invalid UTF-8 instead of throwing (both sides of a diff replace
        // identically, so equivalence still holds).
        std::ofstream f(hydra::os_path(dumppath), std::ios::binary | std::ios::trunc);
        f << arr.dump(1, ' ', false, nlohmann::json::error_handler_t::replace) << "\n";
        std::printf("  dumped %zu items to %s\n", sorted.size(), dumppath.c_str());
    }
}

static void corpus_bench() {
    std::vector<Song> songs;
    for (const std::string& path : corpus::chart_paths()) {
        try {
            songs.push_back(
                load_songpath(path, ChartReading{}, g_rules, fill_rule_for(g_legacy_fills)));
        } catch (const std::exception&) {
        }
    }
    std::printf("Test corpus: %zu charts. Engine = src/search/engine.cpp.\n\n",
                songs.size());
    // Fixed on purpose: this is the before/after yardstick, so it runs the
    // same search every time -- Clone Hero's SP cap, a 4-score range and no
    // timing limit -- whatever the GUI's defaults are. The label says so.
    auto bench = [&](int cap, int dvalue) {
        char name[32];
        std::snprintf(name, sizeof(name), "cap%d d%d no-ms", cap, dvalue);
        SearchSettings settings;
        settings.sp_cap = cap;
        settings.depth_mode = DepthMode::Scores;
        settings.depth_value = dvalue;
        settings.ms_filter = std::nullopt;
        settings.rules = g_rules;
        double best = 1e30;
        for (int rep = 0; rep < 3; ++rep) {
            auto t0 = clk::now();
            for (const Song& s : songs)
                try {
                    analyze_chart(s, settings);
                } catch (const std::exception&) {
                }
            best = std::min(best, secs_since(t0));
        }
        std::printf("  %-16s : %7.2fs (best of 3)\n", name, best);
    };
    bench(kCloneHeroSpCap, 4);
}

// The settings --engine and --parse run under: hydra_batch's, read through
// app::load_tool_settings, under this run's rules. Pointing a run at other
// settings means a copy of the exe beside another ini. Returns the exit code
// load_tool_settings gives, which the caller returns before its mode opens
// anything.
static int ini_settings(app::Settings& st) {
    if (const int code = app::load_tool_settings(st, g_legacy_fills)) return code;
    st.rules = g_rules;
    return 0;
}

// Engine mode, single-threaded:
//   hydra_bench --engine <folder> [--cache <db>] [--reps N] [--out <file>]
// Each chart once (plan_batch's first copy), in scan order: parse, a
// throwaway ScoreGraph build (timed on its own), analyze_chart N times, then
// prepare_record. Prints the phase sums and one digest over every prepared
// record (digest::row_hash); --out writes each chart's md5 and its hash. --cache
// takes a database's rescan cache, so the scan skips rehashing. A chart that
// fails is counted and left out of the digest. `st` is ini_settings'.
static void engine_mode(const std::string& folder, const app::Settings& st,
                        const std::string& cachedb, int reps, const std::string& outpath) {
    store::ChartLibraryCache cache;
    std::unique_ptr<store::RecordStore> cstore;
    if (!cachedb.empty()) {
        cstore = std::make_unique<store::RecordStore>(cachedb, store::OldFilePolicy::Refuse,
                                                      core::RulesStamp::of(g_rules));
        cache = app::load_scan_cache(*cstore);
    }
    auto [items, errors] = app::discover_charts({folder}, app::ScanCallbacks{}, cache);
    const app::AnalysisSettings settings = st.batch_run().settings;
    double t_parse = 0, t_graph = 0, t_an = 0, t_prep = 0, t_gdel = 0;
    int n = 0, failed = 0;
    uint64_t all = digest::kSeed;
    std::optional<std::ofstream> out;
    if (!outpath.empty()) out.emplace(hydra::os_path(outpath), std::ios::binary | std::ios::trunc);
    for (const app::ScanItem& it : app::plan_batch(items, {}).todo) {
        auto t = clk::now();
        std::optional<Song> song;
        try {
            song.emplace(
                load_songpath_with_notes(it.notespath, settings.reading(), settings.rules,
                                         fill_rule_for(settings.legacy_fills)));
        } catch (const std::exception&) {
            ++failed;
            continue;
        }
        t_parse += secs_since(t);
        t = clk::now();
        {
            ScoreGraph g(*song,
                         std::optional<int>(graph_build_cap(settings.sp_cap, song->sp_phrase_count())),
                         fill_rule_for(settings.legacy_fills), settings.rules);
            t_graph += secs_since(t);
            t = clk::now();
        }
        t_gdel += secs_since(t);
        std::optional<HydraRecord> rec;
        t = clk::now();
        try {
            for (int r = 0; r < reps; ++r) rec = analyze_chart(*song, settings);
        } catch (const std::exception&) {
            ++failed;
            continue;
        }
        t_an += secs_since(t);
        t = clk::now();
        const store::PreparedRecord prepared = store::prepare_record(it.md5, st.profile(), *rec);
        t_prep += secs_since(t);
        const uint64_t h = digest::row_hash(prepared, *rec);
        all = digest::fold(all, h);
        if (out) *out << it.md5 << ' ' << std::hex << h << std::dec << '\n';
        ++n;
    }
    std::printf("charts %d failed %d | parse %.3fs | graph %.3fs | analyze x%d %.3fs | "
                "prepare %.3fs | hash %016llx\n",
                n, failed, t_parse, t_graph, reps, t_an, t_prep,
                static_cast<unsigned long long>(all));
    std::printf("graph destroy %.3fs\n", t_gdel);
}

// Parse mode, single-threaded:
//   hydra_bench --parse <folder | list.txt> [--reps N] [--out <file>] [--nodyn]
// A folder is scanned and each chart taken once (plan_batch's first copy); a
// .txt argument lists notes paths, one per line. Each chart is parsed N times
// (the best time counts) and hashed once: digest::song_digest, with the
// dynamics count added (digest::with_dynamics) unless --nodyn. A chart that
// fails is hashed by its exception's type and message (digest::failure_text).
// Prints the sums
// and one digest over every chart; --out writes each path, its best parse
// time in microseconds, its hash and any failure. `st` is ini_settings'.
static void parse_mode(const std::string& arg, const app::Settings& st, int reps,
                       const std::string& outpath, bool dyn) {
    std::vector<std::string> paths;
    if (arg.size() > 4 && arg.substr(arg.size() - 4) == ".txt") {
        std::ifstream in(hydra::os_path(arg));
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) paths.push_back(line);
        }
    } else {
        auto [items, errors] = app::discover_charts({arg});
        for (const app::ScanItem& it : app::plan_batch(items, {}).todo)
            paths.push_back(it.notespath);
    }
    const app::AnalysisSettings settings = st.batch_run().settings;
    double t_parse = 0, t_dyn = 0;
    int n = 0, failed = 0;
    uint64_t all = digest::kSeed;
    std::optional<std::ofstream> out;
    if (!outpath.empty()) out.emplace(hydra::os_path(outpath), std::ios::binary | std::ios::trunc);
    for (const std::string& p : paths) {
        uint64_t h = 0;
        double best = 1e30;
        std::string fail;
        for (int r = 0; r < reps; ++r) {
            auto t = clk::now();
            try {
                Song song = load_songpath_with_notes(p, settings.reading(), settings.rules,
                                                     fill_rule_for(settings.legacy_fills));
                best = std::min(best, secs_since(t));
                if (r == 0) {
                    h = digest::song_digest(song);
                    if (dyn) {
                        auto t2 = clk::now();
                        if (app::analysis_parse_counts_dynamics(settings.bass2x)) {
                            const app::DynamicsBreakdown bd = app::count_dynamics(song);
                            h = digest::with_dynamics(h, bd);
                        }
                        t_dyn += secs_since(t2);
                    }
                }
            } catch (const std::exception& e) {
                best = std::min(best, secs_since(t));
                fail = digest::failure_text(e);
                h = digest::failure_hash(fail);
            }
        }
        t_parse += best;
        if (!fail.empty()) ++failed;
        all = digest::fold(all, h);
        if (out) {
            *out << p << '\t' << static_cast<long long>(best * 1e6) << '\t' << std::hex << h
                 << std::dec;
            if (!fail.empty()) *out << "\tFAIL " << fail;
            *out << '\n';
        }
        ++n;
    }
    std::printf("charts %d failed %d | parse(best of %d) %.3fs | dyn %.3fs | hash %016llx\n", n,
                failed, reps, t_parse, t_dyn, static_cast<unsigned long long>(all));
}

// Dump a store's charts table as the same JSON --dump writes (row_json, with
// the same --dump-rel root), so two scans' results can be diffed even when one
// came from another build.
static void dump_db(const std::string& dbpath, const std::string& outpath,
                    const std::string& dumprel) {
    store::RecordStore db(dbpath, store::OldFilePolicy::Refuse, core::RulesStamp::of(g_rules));
    std::vector<store::ChartLibraryEntry> rows =
        db.list_chart_library(0, INT_MAX);
    std::sort(rows.begin(), rows.end(),
              [](const store::ChartLibraryEntry& a, const store::ChartLibraryEntry& b) {
                  return a.notespath < b.notespath;
              });
    nlohmann::json arr = nlohmann::json::array();
    for (const store::ChartLibraryEntry& e : rows)
        arr.push_back(row_json(e.notespath, e.rootfolder, e.md5, e.title, e.artist,
                               e.charter, dumprel));
    std::ofstream f(hydra::os_path(outpath), std::ios::binary | std::ios::trunc);
    f << arr.dump(1, ' ', false, nlohmann::json::error_handler_t::replace) << "\n";
    std::printf("dumped %zu rows from %s\n", rows.size(), dbpath.c_str());
}

// Replay mode:
//   hydra_bench --replay <chart file>
// Times core::replay_path on one chart (pro drums, 2x bass) with 2,000 made-up
// windows (testreplay::synthetic_windows, seed 7), best of 3: the full replay,
// the scores-only one and one with no windows. That the scores-only replay
// matches the full one is test_replay.cpp's job.
static void replay_mode(const std::string& chart) {
    const Song song = load_songpath(chart, ChartReading{}, g_rules, fill_rule_for(g_legacy_fills));
    if (song.is_empty()) {
        std::printf("no notes in %s\n", chart.c_str());
        return;
    }
    const std::vector<ReplayWindow> wl = testreplay::synthetic_windows(song, 2000, 7);
    ReplayOptions scores;
    scores.scores_only = true;
    auto best_ms = [](auto&& fn) {
        double best = 1e300;
        for (int run = 0; run < 3; ++run) {
            const clk::time_point t0 = clk::now();
            fn();
            best = std::min(best, secs_since(t0) * 1000.0);
        }
        return best;
    };
    // The results are kept and printed, so no build can drop the calls.
    ReplayResult full, lean, none;
    const double full_ms = best_ms([&] { full = replay_path(song, wl, g_rules); });
    const double lean_ms = best_ms([&] { lean = replay_path(song, wl, g_rules, scores); });
    const double none_ms = best_ms([&] { none = replay_path(song, {}, g_rules); });
    std::printf("%zu chords, %zu windows, best of 3: open-window walk %.1f ms, "
                "scores-only %.1f ms, no windows %.1f ms\n",
                song.sequence.size(), wl.size(), full_ms, lean_ms, none_ms);
    std::printf("SP points: %lld with the windows (scores-only %lld), %lld without\n",
                static_cast<long long>(full.final.sp), static_cast<long long>(lean.final.sp),
                static_cast<long long>(none.final.sp));
}

// Upgrade mode:
//   hydra_bench --upgrade <db file>
// Opens the file, which converts a Convertible layout (store/library_file.h)
// for good, and prints how long each step of the open took. A file too old to
// convert is refused. Point it at a scratch copy of a real library file (db,
// -wal and -shm together), never at the live one.
static void upgrade_mode(const std::string& dbpath) {
    if (!file_exists_utf8(dbpath)) {
        std::printf("no such file: %s\n", dbpath.c_str());
        return;
    }
    const uint64_t bytes_before = file_size_bytes(dbpath);
    const clk::time_point start = clk::now();
    std::vector<std::pair<store::OpenStep, clk::time_point>> began;
    store::OpenProgress last;
    const store::OpenProgressFn record = [&](const store::OpenProgress& p) {
        if (began.empty() || began.back().first != p.step) began.emplace_back(p.step, clk::now());
        last = p;
        return true;
    };
    {
        store::RecordStore db(dbpath, store::OldFilePolicy::Refuse, core::RulesStamp::of(g_rules),
                              record);
    }
    const clk::time_point end = clk::now();

    auto name = [](store::OpenStep s) {
        switch (s) {
            case store::OpenStep::Opening: return "Opening";
            case store::OpenStep::ConvertingRecords: return "ConvertingRecords";
        }
        return "?";
    };
    for (size_t i = 0; i < began.size(); ++i) {
        const clk::time_point to = i + 1 < began.size() ? began[i + 1].second : end;
        std::printf("%s: %.3f s\n", name(began[i].first),
                    std::chrono::duration<double>(to - began[i].second).count());
    }
    std::printf("total: %.3f s\n", std::chrono::duration<double>(end - start).count());
    std::printf("rows converted: %lld of %lld\n", static_cast<long long>(last.rows_done),
                static_cast<long long>(last.rows_total));
    std::printf("file: %llu bytes before, %llu bytes after\n",
                static_cast<unsigned long long>(bytes_before),
                static_cast<unsigned long long>(file_size_bytes(dbpath)));
}

// ---- Open mode ---------------------------------------------------------------
//   hydra_bench --open <db file> [--runs N] [--ini <settings file>] [--as-ready]
//               [--rules <path>]
// Times what opening the library costs, in Hydra's own code, on the file it is
// given and no other. It refuses the installed Hydra's folder (installer/
// hydra.iss installs to {autopf}\Hydra) and the hydra.db beside this exe
// (app::db_path), before anything opens the file, so --as-ready is refused
// there too. Point it at a scratch copy of a real library file (db, -wal and
// -shm together): the store's open can convert an old layout, as the app's
// does. --ini reads that settings file (Settings::load_file) so the lookups ask
// for the user's profile; without it the defaults are used. --as-ready
// rewrites the copy's profile rows to this build's results stamp and rules
// fingerprint before anything is timed (stamp_as_ready), so a library saved
// by an older build times as one the user has re-analysed; it prints how
// many rows it stamped and how many charts are Ready after it. What counts as
// Ready is still lookup's own rule. A failure names the step or variant it
// happened in. Every read on the bench's own connections runs with a
// RecordStore open on the file, as the app's store is while it reads. Every time is
// printed as the median and the minimum of N runs, in milliseconds, each run
// with a fresh RecordStore and LibraryModel. One untimed reference read comes
// first: it gives the rows each variant is checked against, so the timed runs
// all start with the file in the OS cache.
//
// What each section times:
//   1. The app's counted read (ui::read_library with a progress callback that
//      only returns true), split into its parts by calling the same store and
//      model functions read_library calls, in its order, with the results read
//      in kResultsSliceCharts slices as read_library_summaries reads them.
//      Then the real read_library call as a whole.
//   2. read_library with no callback (AppState::reload_library's read).
//   3. The chart list: list_chart_library as today, then prototypes on a
//      connection of the bench's own: the same query, no ORDER BY, and only
//      the columns set_charts reads, each through store::read_library_entries.
//   4. The chart list read by 2 and 4 threads, one connection each, each
//      reading a share of the rowids, joined at the end.
//   5. RecordStore::lookup for every chart, then its second read (the Ready
//      charts' path rows) alone, as today and as a prototype that reads only
//      the winning profile's rows.
//   6. set_charts as a whole, then its steps one pass at a time, then the
//      sort and the filter it ends with.
// A prototype's rows are checked against today's before its time counts: the
// same row count as chart_library_count, the same values, and the same Library
// on screen once set_charts has them. One that differs prints why instead.

namespace {

using Entries = std::vector<store::ChartLibraryEntry>;
using PathsBySong = std::unordered_map<std::string, std::vector<store::PathRow>>;

double ms_since(clk::time_point t0) {
    return std::chrono::duration<double, std::milli>(clk::now() - t0).count();
}

double median_of(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const size_t n = v.size();
    return n % 2 ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

// A row count as read_library counts it: none below zero.
uint64_t rows_of(int64_t n) { return n > 0 ? static_cast<uint64_t>(n) : 0; }

// Every labelled time the runs measure, in the order first measured, with the
// section headings between them. A label whose rows differed from today's
// keeps why, and prints that in place of a time.
class OpenTimes {
public:
    void heading(const std::string& text) { slot(text).heading = true; }
    void add(const std::string& label, double ms) { slot(label).ms.push_back(ms); }
    void differs(const std::string& label, const std::string& why) {
        Slot& s = slot(label);
        if (s.why.empty()) s.why = why;
    }
    bool differed(const std::string& label) { return !slot(label).why.empty(); }
    double median(const std::string& label) { return median_of(slot(label).ms); }
    void print() const {
        for (const Slot& s : slots_) {
            if (s.heading) {
                std::printf("\n%s\n", s.label.c_str());
            } else if (!s.why.empty()) {
                std::printf("  %-68s rows differ from today's: %s\n", s.label.c_str(),
                            s.why.c_str());
            } else if (!s.ms.empty()) {
                std::printf("  %-68s median %8.1f ms   min %8.1f ms\n", s.label.c_str(),
                            median_of(s.ms), *std::min_element(s.ms.begin(), s.ms.end()));
            }
        }
    }

private:
    struct Slot {
        std::string label;
        bool heading = false;
        std::vector<double> ms;
        std::string why;
    };
    Slot& slot(const std::string& label) {
        for (Slot& s : slots_)
            if (s.label == label) return s;
        slots_.push_back(Slot{label, false, {}, {}});
        return slots_.back();
    }
    std::vector<Slot> slots_;
};

// A connection of the bench's own, opened read-write without create, so a
// wrong path fails instead of making a file. Only --as-ready's rewrite
// (stamp_as_ready) writes through it.
class OwnDb {
public:
    explicit OwnDb(const std::string& path) {
        if (store::open_sqlite(path, &db_, SQLITE_OPEN_READWRITE) != SQLITE_OK) {
            const std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
            sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("opening " + path + ": " + msg);
        }
    }
    ~OwnDb() { sqlite3_close(db_); }
    OwnDb(const OwnDb&) = delete;
    OwnDb& operator=(const OwnDb&) = delete;
    sqlite3* get() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};

// The rows of a chart-list query on `db`, built by the store's own row reader.
// `binds` are bound in order from parameter 1.
Entries read_entries(sqlite3* db, const std::string& sql, bool timing_read,
                     const std::vector<int64_t>& binds) {
    store::Stmt s = store::prepare_read(db, sql.c_str());
    for (size_t i = 0; i < binds.size(); ++i)
        sqlite3_bind_int64(s, static_cast<int>(i + 1), binds[i]);
    return store::read_library_entries(s, timing_read);
}

// The chart list read on `conns.size()` threads at once, one connection each,
// each reading its share of the rowids, joined in share order at the end.
Entries read_in_shares(const std::vector<std::unique_ptr<OwnDb>>& conns,
                       const std::string& select, bool timing_read, int64_t max_rowid) {
    const int64_t n = static_cast<int64_t>(conns.size());
    const std::string sql = select + " WHERE rowid > ? AND rowid <= ?";
    std::vector<Entries> parts(conns.size());
    std::vector<std::exception_ptr> failed(conns.size());
    std::vector<std::thread> threads;
    threads.reserve(conns.size());
    for (int64_t k = 0; k < n; ++k) {
        // The first share starts below every rowid and the last ends above
        // them all, so no row falls outside the shares.
        const int64_t after =
            k == 0 ? std::numeric_limits<int64_t>::min() : max_rowid * k / n;
        const int64_t upto =
            k + 1 == n ? std::numeric_limits<int64_t>::max() : max_rowid * (k + 1) / n;
        threads.emplace_back([&, k, after, upto] {
            const size_t at = static_cast<size_t>(k);
            try {
                parts[at] = read_entries(conns[at]->get(), sql, timing_read, {after, upto});
            } catch (...) {
                failed[at] = std::current_exception();
            }
        });
    }
    for (std::thread& th : threads) th.join();
    for (const std::exception_ptr& e : failed)
        if (e) std::rethrow_exception(e);
    size_t total = 0;
    for (const Entries& part : parts) total += part.size();
    Entries out;
    out.reserve(total);
    for (Entries& part : parts)
        for (store::ChartLibraryEntry& row : part) out.push_back(std::move(row));
    return out;
}

std::string column_string(sqlite3_stmt* s, int i) {
    const unsigned char* p = sqlite3_column_text(s, i);
    return p ? std::string(reinterpret_cast<const char*>(p),
                           static_cast<size_t>(sqlite3_column_bytes(s, i)))
             : std::string();
}

// The Ready charts' path rows, read the way RecordStore::lookup's second read
// reads them: in song, profile and rank order, keeping only the rows filed
// under each song's winning profile (`winner`). With `only` set, the SQL itself
// reads only that profile's rows: the winner-only prototype. Both send as many
// song ids per statement as the connection's bound-parameter limit allows, one
// kept back for the profile. `rows_read` counts every row the SQL returned.
PathsBySong read_paths(sqlite3* db, const std::vector<std::string>& ids,
                       const std::unordered_map<std::string, int64_t>& winner,
                       std::optional<int64_t> only, int64_t& rows_read) {
    PathsBySong out;
    rows_read = 0;
    const size_t per_query =
        static_cast<size_t>(sqlite3_limit(db, SQLITE_LIMIT_VARIABLE_NUMBER, -1)) - 1;
    for (size_t first = 0; first < ids.size(); first += per_query) {
        const size_t n = std::min(per_query, ids.size() - first);
        std::string sql = "SELECT song_id, profile_id, rank, " + store::path_row_columns("") +
                          " FROM record_paths WHERE ";
        if (only) sql += "profile_id = ? AND ";
        sql += "song_id IN (";
        for (size_t i = 0; i < n; ++i) sql += i ? ", ?" : "?";
        sql += ") ORDER BY song_id, profile_id, rank";
        store::Stmt s = store::prepare_read(db, sql.c_str());
        int idx = 1;
        if (only) sqlite3_bind_int64(s, idx++, *only);
        for (size_t i = 0; i < n; ++i) {
            const std::string& id = ids[first + i];
            sqlite3_bind_text(s, idx++, id.data(), static_cast<int>(id.size()), SQLITE_TRANSIENT);
        }
        while (store::step_row(s)) {
            ++rows_read;
            const std::string id = column_string(s, 0);
            if (winner.at(id) != sqlite3_column_int64(s, 1)) continue;
            if (std::optional<store::PathRow> p = store::read_path_row(s, 3, sqlite3_column_int(s, 2)))
                out[id].push_back(std::move(*p));
        }
    }
    return out;
}

// The first way `got` differs from lookup's paths for the Ready charts, or ""
// when they agree.
std::string paths_differ(const PathsBySong& got,
                         const std::map<std::string, std::vector<store::PathRow>>& want) {
    const std::vector<store::PathRow> none;
    for (const auto& kv : want) {
        const auto it = got.find(kv.first);
        if ((it == got.end() ? none : it->second) != kv.second)
            return "chart " + kv.first + "'s paths differ";
    }
    for (const auto& kv : got)
        if (want.find(kv.first) == want.end()) return "chart " + kv.first + " is not Ready";
    return "";
}

// The first way `got` differs from `want`, row by row, or "" when they agree.
// `all_columns` false compares only the fields set_charts reads, for the
// prototype that leaves the rest unread.
std::string entries_differ(const Entries& got, const Entries& want, bool all_columns) {
    if (got.size() != want.size())
        return std::to_string(got.size()) + " rows, today " + std::to_string(want.size());
    for (size_t i = 0; i < got.size(); ++i) {
        const store::ChartLibraryEntry& a = got[i];
        const store::ChartLibraryEntry& b = want[i];
        const bool same = a.md5 == b.md5 && a.title == b.title && a.artist == b.artist &&
                          a.charter == b.charter && a.album == b.album &&
                          a.notespath == b.notespath && a.rootfolder == b.rootfolder &&
                          (!all_columns || (a.sig == b.sig && a.timing == b.timing));
        if (!same) return "row " + std::to_string(i) + " (" + b.notespath + ")";
    }
    return "";
}

// The rows in one fixed order, for comparing two reads as sets.
Entries by_path(Entries rows) {
    std::sort(rows.begin(), rows.end(),
              [](const store::ChartLibraryEntry& a, const store::ChartLibraryEntry& b) {
                  return std::tie(a.notespath, a.md5, a.rootfolder) <
                         std::tie(b.notespath, b.md5, b.rootfolder);
              });
    return rows;
}

bool same_row(const ui::LibraryRow& a, const ui::LibraryRow& b) {
    return a.entry.md5 == b.entry.md5 && a.entry.notespath == b.entry.notespath &&
           a.entry.rootfolder == b.entry.rootfolder && a.title == b.title &&
           a.artist == b.artist && a.album == b.album && a.charter == b.charter &&
           a.searchable.title == b.searchable.title && a.searchable.artist == b.searchable.artist &&
           a.searchable.album == b.searchable.album &&
           a.searchable.charter == b.searchable.charter &&
           a.searchable.folder == b.searchable.folder && a.sort_keys == b.sort_keys &&
           a.status == b.status && a.stale_build == b.stale_build &&
           a.stale_rules == b.stale_rules && a.bestpath == b.bestpath &&
           a.summary == b.summary && a.failure_kind == b.failure_kind &&
           a.failure_text == b.failure_text;
}

// The first way two Libraries differ on screen (the rows shown, in their
// order, and everything each row shows or sorts on), or "" when they agree.
std::string models_differ(const ui::LibraryModel& got, const ui::LibraryModel& want) {
    if (got.rows().size() != want.rows().size())
        return std::to_string(got.rows().size()) + " rows, today " +
               std::to_string(want.rows().size());
    if (got.order().size() != want.order().size())
        return std::to_string(got.order().size()) + " rows shown, today " +
               std::to_string(want.order().size());
    for (size_t i = 0; i < want.order().size(); ++i) {
        const ui::LibraryRow& b = want.rows()[want.order()[i]];
        if (!same_row(got.rows()[got.order()[i]], b))
            return "shown row " + std::to_string(i) + " (" + b.entry.notespath + ")";
    }
    return "";
}

// Why a prototype's chart list is not today's, or "" when it is. `count` is
// chart_library_count's. `ordered` false compares the two as sets. The rows
// then go through set_charts and must show the same Library as today's rows.
std::string chart_list_differs(const Entries& got, const Entries& today, int64_t count,
                               bool ordered, bool all_columns, const ui::LibraryModel& today_model) {
    if (static_cast<int64_t>(got.size()) != count)
        return std::to_string(got.size()) + " rows, chart_library_count " + std::to_string(count);
    const std::string why = ordered ? entries_differ(got, today, all_columns)
                                    : entries_differ(by_path(got), by_path(today), all_columns);
    if (!why.empty()) return why;
    ui::LibraryModel model;
    model.set_charts(got);
    const std::string shown = models_differ(model, today_model);
    return shown.empty() ? "" : "on screen, " + shown;
}

// The folders an installed Hydra keeps its library in: installer/hydra.iss
// installs to {autopf}\Hydra, read here from the Program Files variables, plus
// the path the --open request named in case they are unset.
std::vector<std::string> installed_folders() {
    std::vector<std::string> out;
    for (const wchar_t* var : {L"ProgramFiles", L"ProgramW6432", L"ProgramFiles(x86)"}) {
        wchar_t* value = nullptr;
        size_t len = 0;
        if (_wdupenv_s(&value, &len, var) == 0 && value)
            out.push_back(join_folder(wide_to_utf8(value), "Hydra"));
        std::free(value);
    }
    out.push_back("C:\\Program Files\\Hydra");
    return out;
}

// Why --open will not open `dbpath`, or nothing when it may.
std::optional<std::string> refused_db(const std::string& dbpath) {
    if (!file_exists_utf8(dbpath)) return "no such file: " + dbpath;
    std::error_code ec;
    const std::filesystem::path full = std::filesystem::absolute(os_path(dbpath), ec);
    if (ec) return "cannot make a full path of " + dbpath + ": " + ec.message();
    const std::string full_text = wide_to_utf8(full.wstring());
    for (const std::string& folder : installed_folders())
        if (same_folder(parent_folder(full_text), folder))
            return dbpath + " is inside the installed Hydra's folder (" + folder +
                   "); copy the library somewhere else and point --open at the copy";
    // same_folder compares two paths by name only, so a file's path works too.
    if (same_folder(full_text, app::db_path()))
        return dbpath + " is the library beside this exe (" + app::db_path() +
               "); point --open at a copy";
    return std::nullopt;
}

// What --as-ready's rewrite did: how many profile rows the file holds, and
// how many of them carry this build's stamp afterwards.
struct AsReadyCounts {
    int64_t profiles = 0;
    int64_t stamped = 0;
};

// One statement on `db` that returns no rows, or a throw naming `what`.
void exec_or_throw(sqlite3* db, const char* sql, const std::string& what) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        const std::string msg = err ? err : sqlite3_errmsg(db);
        sqlite3_free(err);
        throw std::runtime_error(what + ": " + msg);
    }
}

// --as-ready: gives every profile row of the copy at `dbpath` the results
// stamp and rules fingerprint the store writes for a new record in this build
// (store::current_record_version and store::ready_rules_fp, which save and
// lookup read), in one transaction, so lookup finds those rows Ready. The
// profiles table holds one row per settings and stamp, so where two rows
// share their settings only the first one rewritten takes the stamp: rows go
// newest first, and a row that would clash keeps its old stamp (UPDATE OR
// IGNORE). The caller has already refused the installed library.
AsReadyCounts stamp_as_ready(const std::string& dbpath, const core::RulesStamp& stamp) {
    OwnDb conn(dbpath);
    sqlite3* db = conn.get();
    const std::string version = store::current_record_version();
    const std::vector<uint8_t> fp = store::ready_rules_fp(stamp);
    std::vector<int64_t> ids;
    {
        store::Stmt s = store::prepare_read(db, "SELECT id FROM profiles ORDER BY id DESC");
        while (store::step_row(s)) ids.push_back(sqlite3_column_int64(s, 0));
    }
    AsReadyCounts out;
    out.profiles = static_cast<int64_t>(ids.size());
    exec_or_throw(db, "BEGIN IMMEDIATE", "starting the rewrite");
    try {
        store::Stmt s = store::prepare_write(
            db, "UPDATE OR IGNORE profiles SET hyversion = ?, rules_fp = ? WHERE id = ?");
        for (const int64_t id : ids) {
            sqlite3_reset(s.p);
            store::bind_text(s, 1, version);
            store::bind_blob(s, 2, fp);
            sqlite3_bind_int64(s.p, 3, id);
            store::step_done(s, "rewriting profile " + std::to_string(id));
            out.stamped += sqlite3_changes(db);
        }
        sqlite3_reset(s.p);
        exec_or_throw(db, "COMMIT", "committing the rewrite");
    } catch (...) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        throw;
    }
    return out;
}

}  // namespace

static int open_runs(const std::string& dbpath, int runs, const std::optional<std::string>& ini,
                     app::Settings settings, bool as_ready, std::string& doing);

// The checks that need no database, then the runs. A failure in the runs
// names the step or variant it happened in (open_runs keeps `doing` up to
// date) instead of the tool's generic database line.
static int open_mode(const std::string& dbpath, int runs, const std::optional<std::string>& ini,
                     bool as_ready) {
    // Before anything opens the file, so --as-ready's rewrite is refused too.
    if (const std::optional<std::string> why = refused_db(dbpath)) {
        std::fprintf(stderr, "hydra_bench --open: %s\n", why->c_str());
        return 2;
    }
    if (runs < 1) {
        std::fprintf(stderr, "hydra_bench --open: --runs needs 1 or more\n");
        return 2;
    }
    app::Settings settings;  // the struct defaults are the app's defaults
    if (ini) {
        if (!file_exists_utf8(*ini)) {
            std::fprintf(stderr, "hydra_bench --open: no such settings file: %s\n", ini->c_str());
            return 2;
        }
        try {
            settings = app::Settings::load_file(*ini);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "hydra_bench --open: %s\n", e.what());
            return 2;
        }
    }
    std::string doing = "the first open";
    try {
        return open_runs(dbpath, runs, ini, std::move(settings), as_ready, doing);
    } catch (const std::exception& e) {
        std::fflush(stdout);
        std::fprintf(stderr, "\nhydra_bench --open: %s failed: %s\n", doing.c_str(), e.what());
        return 1;
    }
}

static int open_runs(const std::string& dbpath, int runs, const std::optional<std::string>& ini,
                     app::Settings settings, bool as_ready, std::string& doing) {
    settings.rules = g_rules;
    const store::Profile want = settings.profile();
    const core::RulesStamp stamp = core::RulesStamp::of(g_rules);
    auto open_store = [&] {
        return std::make_unique<store::RecordStore>(dbpath, store::OldFilePolicy::Refuse, stamp);
    };
    using Step = ui::StoreOpenProgress::Step;
    // The counted read's callback, doing nothing but letting the read go on.
    const ui::LibraryReadFn report = [](Step, uint64_t, uint64_t) { return true; };

    // ---- --as-ready's rewrite, before anything is read or timed ----
    AsReadyCounts stamped;
    if (as_ready) {
        // The store's open first: it converts an old layout, whose results
        // have no profiles table yet, as the app's open would.
        doing = "--as-ready's open of the store";
        open_store().reset();  // opened and closed again, so the rewrite has the file alone
        doing = "--as-ready's rewrite of the profiles";
        stamped = stamp_as_ready(dbpath, stamp);
    }

    // ---- the untimed reference read ----
    doing = "the untimed reference read";
    Entries today;
    int64_t count = 0;
    std::vector<std::string> hashes;
    std::vector<store::RecordLookup> answers;
    {
        std::unique_ptr<store::RecordStore> db = open_store();
        count = db->chart_library_count();
        today = db->list_chart_library(0, -1);  // -1 = no limit, as read_library asks
        ui::LibraryModel m;
        m.set_charts(today);
        hashes = m.hashes();
        answers = db->lookup(hashes, want);
    }
    ui::LibraryModel today_model;  // today's rows through set_charts, no results
    today_model.set_charts(today);
    // The rows list_chart_library read with their timing, when the store
    // trusts the timing columns; the prototypes ask for the same.
    const bool timing_read = !today.empty() && today.front().timing.has_value();
    size_t same_path = 0;
    {
        std::vector<std::string> paths;
        for (const store::ChartLibraryEntry& e : today) paths.push_back(e.notespath);
        std::sort(paths.begin(), paths.end());
        for (size_t i = 1; i < paths.size(); ++i)
            if (paths[i] == paths[i - 1]) ++same_path;
    }
    // Lookup's answer for every Ready chart, and the ids its second read asks about.
    std::map<std::string, std::vector<store::PathRow>> ready;
    for (size_t i = 0; i < hashes.size() && i < answers.size(); ++i)
        if (answers[i].status == store::RecordStatus::Ready)
            ready.emplace(hashes[i], answers[i].record.paths);
    std::vector<std::string> ready_ids;
    for (const auto& kv : ready) ready_ids.push_back(kv.first);
    int64_t max_rowid = 0;
    // The profile whose path rows give lookup's paths for every Ready chart:
    // found from the data, by trying each profile one Ready chart's rows are
    // filed under and keeping the one that reproduces all of lookup's answers.
    std::optional<int64_t> winner_profile;
    std::unordered_map<std::string, int64_t> winner;
    std::string winner_problem;
    int64_t rows_today_shape = 0, rows_winner_only = 0;
    doing = "the search for the winning profile";
    {
        OwnDb conn(dbpath);
        {
            store::Stmt s = store::prepare_read(conn.get(), "SELECT max(rowid) FROM charts");
            if (store::step_row(s)) max_rowid = sqlite3_column_int64(s, 0);
        }
        std::string probe;
        for (const auto& kv : ready)
            if (!kv.second.empty()) {
                probe = kv.first;
                break;
            }
        if (probe.empty()) {
            winner_problem = "no Ready chart has a path, so there is no path read to time";
        } else {
            std::vector<int64_t> candidates;
            {
                store::Stmt s = store::prepare_read(
                    conn.get(), "SELECT DISTINCT profile_id FROM record_paths WHERE song_id = ?");
                sqlite3_bind_text(s, 1, probe.data(), static_cast<int>(probe.size()),
                                  SQLITE_TRANSIENT);
                while (store::step_row(s)) candidates.push_back(sqlite3_column_int64(s, 0));
            }
            for (int64_t c : candidates) {
                std::unordered_map<std::string, int64_t> trial;
                for (const std::string& id : ready_ids) trial.emplace(id, c);
                int64_t rows = 0;
                if (paths_differ(read_paths(conn.get(), ready_ids, trial, c, rows), ready).empty()) {
                    winner_profile = c;
                    winner = std::move(trial);
                    break;
                }
            }
            if (!winner_profile)
                winner_problem = "no one profile's path rows give lookup's paths for every Ready chart";
        }
        if (winner_profile) {
            read_paths(conn.get(), ready_ids, winner, std::nullopt, rows_today_shape);
            read_paths(conn.get(), ready_ids, winner, winner_profile, rows_winner_only);
        }
    }

    std::printf("Library: %s\n", dbpath.c_str());
    if (as_ready) {
        std::printf("--as-ready rewrote this copy before any timing: %s of %s now carry this "
                    "build's results stamp (%s) and rules fingerprint.\n",
                    counted(stamped.stamped, "profile row", "profile rows").c_str(),
                    group_thousands(stamped.profiles).c_str(),
                    store::current_record_version().c_str());
        if (stamped.profiles > stamped.stamped)
            std::printf("  %s kept their old stamp: another profile row with the same settings "
                        "took this build's stamp first (newest row first), and the profiles "
                        "table allows one row per settings and stamp.\n",
                        counted(stamped.profiles - stamped.stamped, "row", "rows").c_str());
        std::printf("  %s Ready after it.\n",
                    counted(static_cast<int64_t>(ready.size()), "chart is", "charts are").c_str());
    }
    std::printf("Settings: %s; profile %s at SP cap %d.\n",
                ini ? ini->c_str() : "the defaults (no --ini)", want.chartmode.c_str(),
                want.cap.exact);
    std::printf("%s (chart_library_count); %s; %s with a path shared by another row.\n",
                counted(count, "chart row", "chart rows").c_str(),
                counted(static_cast<int64_t>(ready.size()), "Ready chart", "Ready charts").c_str(),
                counted(static_cast<int64_t>(same_path), "row", "rows").c_str());
    std::printf("The store %s the rows' timing columns.\n",
                timing_read ? "reads" : "does not read");
    if (winner_profile)
        std::printf("Lookup's second read returns %s today; the winner-only read returns %s "
                    "(profile id %lld).\n",
                    counted(rows_today_shape, "path row", "path rows").c_str(),
                    counted(rows_winner_only, "path row", "path rows").c_str(),
                    static_cast<long long>(*winner_profile));
    std::printf("Every read on the bench's own connections (sections 3 to 5) runs with a "
                "RecordStore open on the same file, as the app's store is while it reads.\n");
    std::printf("%s, each a median and a minimum in ms.\n",
                counted(runs, "timed run", "timed runs").c_str());

    OpenTimes times;
    const std::string h1 = "1. Today's open, the app's counted read (read_library with a callback)";
    const std::string k1_open = "open (RecordStore)";
    const std::string k1_count = "chart count (chart_library_count)";
    const std::string k1_list = "chart list (list_chart_library, per-row reports)";
    const std::string k1_set = "set_charts";
    const std::string k1_results = "results (lookup, all slices)";
    const std::string k1_summ = "set_summaries";
    const std::string k1_sort = "sort (sort_if_out_of_date)";
    const std::string k1_sum = "total of the parts above";
    const std::string k1_real = "read_library as one call, counted";
    const std::string k1_real_open = "open + read_library, counted";
    const std::string h2 = "2. read_library with no callback (AppState::reload_library's read)";
    const std::string k2_real = "read_library as one call, no callback";
    const std::string k2_real_open = "open + read_library, no callback";
    const std::string h3 = "3. The chart list, one connection";
    const std::string k3_today = "today: list_chart_library, ORDER BY name";
    const std::string k3_conn = "own connection: open it";
    const std::string k3_by_name = "own connection: today's query, ORDER BY name";
    const std::string k3_no_order = "own connection: no ORDER BY";
    const std::string k3_slim_name = "own connection: only set_charts' columns, ORDER BY name";
    const std::string k3_slim = "own connection: only set_charts' columns, no ORDER BY";
    const std::string h4 = "4. The chart list on threads, one connection each, no ORDER BY";
    const std::string h5 = "5. The results lookup";
    const std::string k5_lookup = "today: lookup for every chart in one call";
    const std::string k5_paths = "lookup's path read alone, as today (every profile's rows)";
    const std::string k5_winner = "lookup's path read alone, winner-only prototype";
    const std::string h6 = "6. Inside set_charts";
    const std::string k6_whole = "set_charts as one call";
    const std::string k6_make = "pass: make the row objects";
    const std::string k6_names = "pass: shown names (display_title/artist/album/charter)";
    const std::string k6_search = "pass: search text (make_searchable)";
    const std::string k6_keys = "pass: sort keys (fold_for_search of each column before Best path)";
    const std::string k6_sorted = "sort + filter (set_sort, title ascending)";
    const std::string k6_filter = "filter alone (set_chip)";

    for (int run = 0; run < runs; ++run) {
        const bool check = run == 0;
        // Names the variant under way, for open_mode's failure line.
        auto at = [&](const std::string& what) {
            doing = "run " + std::to_string(run + 1) + " of " + std::to_string(runs) + ", " + what;
        };

        // 1. The counted read, part by part, in read_library's order.
        times.heading(h1);
        at(h1 + ": the parts");
        ui::LibraryModel parts_model;
        {
            double sum = 0.0;
            auto part = [&](const std::string& label, clk::time_point t0) {
                const double ms = ms_since(t0);
                times.add(label, ms);
                sum += ms;
            };
            clk::time_point t = clk::now();
            std::unique_ptr<store::RecordStore> db = open_store();
            part(k1_open, t);
            t = clk::now();
            const uint64_t total = rows_of(db->chart_library_count());
            report(Step::ReadingCharts, 0, total);
            part(k1_count, t);
            t = clk::now();
            const store::RowsReadFn on_row = [&](int64_t rows_read) {
                return report(Step::ReadingCharts, rows_of(rows_read), total);
            };
            Entries rows = db->list_chart_library(0, -1, on_row);
            part(k1_list, t);
            t = clk::now();
            parts_model.set_charts(std::move(rows));
            part(k1_set, t);
            t = clk::now();
            report(Step::ReadingResults, 0, total);
            const std::vector<std::string> ids = parts_model.hashes();
            std::vector<store::RecordLookup> found;
            found.reserve(ids.size());
            for (size_t from = 0; from < ids.size(); from += ui::kResultsSliceCharts) {
                const size_t to = std::min(ids.size(), from + ui::kResultsSliceCharts);
                const std::vector<std::string> slice(ids.begin() + static_cast<std::ptrdiff_t>(from),
                                                     ids.begin() + static_cast<std::ptrdiff_t>(to));
                for (store::RecordLookup& answer : db->lookup(slice, want))
                    found.push_back(std::move(answer));
                report(Step::ReadingResults, static_cast<uint64_t>(found.size()), total);
            }
            part(k1_results, t);
            t = clk::now();
            parts_model.set_summaries(found);
            part(k1_summ, t);
            t = clk::now();
            parts_model.sort_if_out_of_date();
            part(k1_sort, t);
            times.add(k1_sum, sum);
        }
        at(h1 + ": " + k1_real);
        ui::LibraryModel counted_model;
        {
            clk::time_point t = clk::now();
            std::unique_ptr<store::RecordStore> db = open_store();
            const double open_ms = ms_since(t);
            t = clk::now();
            const std::string problem = ui::read_library(*db, settings, counted_model, report);
            const double read_ms = ms_since(t);
            if (!problem.empty()) throw std::runtime_error("read_library: " + problem);
            times.add(k1_real, read_ms);
            times.add(k1_real_open, open_ms + read_ms);
        }
        if (check) {
            const std::string why = models_differ(parts_model, counted_model);
            if (!why.empty()) times.differs(k1_sum, "the parts' Library is not read_library's: " + why);
        }

        // 2. The same read with no callback.
        times.heading(h2);
        at(h2);
        {
            ui::LibraryModel plain_model;
            clk::time_point t = clk::now();
            std::unique_ptr<store::RecordStore> db = open_store();
            const double open_ms = ms_since(t);
            t = clk::now();
            const std::string problem = ui::read_library(*db, settings, plain_model);
            const double read_ms = ms_since(t);
            if (!problem.empty()) throw std::runtime_error("read_library: " + problem);
            times.add(k2_real, read_ms);
            times.add(k2_real_open, open_ms + read_ms);
            if (check) {
                const std::string why = models_differ(plain_model, counted_model);
                if (!why.empty()) times.differs(k2_real, why);
            }
        }

        // 3. The chart list on one connection.
        times.heading(h3);
        at(h3 + ": " + k3_today);
        {
            std::unique_ptr<store::RecordStore> db = open_store();
            const clk::time_point t = clk::now();
            const Entries rows = db->list_chart_library(0, -1);
            times.add(k3_today, ms_since(t));
            if (check) {
                const std::string why = chart_list_differs(rows, today, count, true, true, today_model);
                if (!why.empty()) times.differs(k3_today, why);
            }
        }
        // The slots read_library_entries reads (kLibraryColumnList's order),
        // with sig and the two timing columns, which set_charts never reads,
        // left as constants.
        const std::string slim_select =
            "SELECT md5, name, artist, charter, album, path, folder, '', NULL, NULL FROM charts";
        auto own_read = [&](const std::string& label, const std::string& sql, bool timing,
                            const std::vector<int64_t>& binds, bool ordered, bool all_columns) {
            at(h3 + ": " + label);
            const std::unique_ptr<store::RecordStore> held = open_store();
            clk::time_point t = clk::now();
            OwnDb conn(dbpath);
            times.add(k3_conn, ms_since(t));
            t = clk::now();
            const Entries rows = read_entries(conn.get(), sql, timing, binds);
            times.add(label, ms_since(t));
            if (check) {
                const std::string why =
                    chart_list_differs(rows, today, count, ordered, all_columns, today_model);
                if (!why.empty()) times.differs(label, why);
            }
        };
        // LIMIT -1 OFFSET 0: list_chart_library's binds for read_library's
        // whole read.
        own_read(k3_by_name, store::library_entry_select() + " ORDER BY name LIMIT ? OFFSET ?",
                 timing_read, {-1, 0}, true, true);
        own_read(k3_no_order, store::library_entry_select(), timing_read, {}, false, true);
        own_read(k3_slim_name, slim_select + " ORDER BY name", false, {}, true, false);
        own_read(k3_slim, slim_select, false, {}, false, false);

        // 4. The chart list on threads. Like every read on the bench's own
        //    connections, it runs with a RecordStore open (`held`), as the
        //    app's store is while it reads. With no connection open, the
        //    threads' first reads race to rebuild the WAL index and all but
        //    one fail with "database is locked".
        times.heading(h4);
        for (const size_t threads : {size_t{2}, size_t{4}}) {
            for (const bool slim : {false, true}) {
                const std::string label = std::to_string(threads) + " threads, " +
                                          (slim ? "only set_charts' columns" : "every column");
                at(h4 + ": " + label);
                const std::unique_ptr<store::RecordStore> held = open_store();
                std::vector<std::unique_ptr<OwnDb>> conns;
                for (size_t i = 0; i < threads; ++i) conns.push_back(std::make_unique<OwnDb>(dbpath));
                const clk::time_point t = clk::now();
                const Entries rows =
                    read_in_shares(conns, slim ? slim_select : store::library_entry_select(),
                                   slim ? false : timing_read, max_rowid);
                times.add(label, ms_since(t));
                if (check) {
                    const std::string why =
                        chart_list_differs(rows, today, count, false, !slim, today_model);
                    if (!why.empty()) times.differs(label, why);
                }
            }
        }

        // 5. The results lookup.
        times.heading(h5);
        at(h5 + ": " + k5_lookup);
        {
            std::unique_ptr<store::RecordStore> db = open_store();
            const clk::time_point t = clk::now();
            const std::vector<store::RecordLookup> found = db->lookup(hashes, want);
            times.add(k5_lookup, ms_since(t));
            if (check && found.size() != answers.size())
                times.differs(k5_lookup, "a different answer count from the reference read");
        }
        if (!winner_profile) {
            times.differs(k5_paths, winner_problem);
            times.differs(k5_winner, winner_problem);
        } else {
            for (const bool only : {false, true}) {
                const std::string& label = only ? k5_winner : k5_paths;
                at(h5 + ": " + label);
                const std::unique_ptr<store::RecordStore> held = open_store();
                OwnDb conn(dbpath);
                int64_t rows = 0;
                const clk::time_point t = clk::now();
                const PathsBySong got = read_paths(conn.get(), ready_ids, winner,
                                                   only ? winner_profile : std::nullopt, rows);
                times.add(label, ms_since(t));
                if (check) {
                    const std::string why = paths_differ(got, ready);
                    if (!why.empty()) times.differs(label, why);
                }
            }
        }

        // 6. Inside set_charts.
        times.heading(h6);
        at(h6);
        {
            ui::LibraryModel m;
            Entries copy = today;
            clk::time_point t = clk::now();
            m.set_charts(std::move(copy));
            times.add(k6_whole, ms_since(t));
            // set_charts' own steps, one pass over every row each, with the
            // functions set_charts calls, in its order. The step that moves
            // each chart's hash, file and folder into its row has no function
            // of its own, so it is not timed apart: each row takes those from
            // set_charts' row instead, untimed, before the sort keys read them.
            const Entries& input = today;
            std::vector<ui::LibraryRow> rows;
            t = clk::now();
            rows.resize(input.size());
            times.add(k6_make, ms_since(t));
            t = clk::now();
            for (size_t i = 0; i < input.size(); ++i) {
                rows[i].title = display_title(input[i].title);
                rows[i].artist = display_artist(input[i].artist);
                rows[i].album = display_album(input[i].album);
                rows[i].charter = display_charter(input[i].charter);
            }
            times.add(k6_names, ms_since(t));
            t = clk::now();
            for (size_t i = 0; i < input.size(); ++i)
                rows[i].searchable =
                    app::make_searchable(input[i].title, input[i].artist, input[i].charter,
                                         input[i].rootfolder, input[i].album);
            times.add(k6_search, ms_since(t));
            for (size_t i = 0; i < rows.size() && i < m.rows().size(); ++i)
                rows[i].entry = m.rows()[i].entry;
            t = clk::now();
            for (ui::LibraryRow& row : rows)
                for (int c = 0; c < ui::kColumnBestPath; ++c)
                    row.sort_keys[static_cast<size_t>(c)] =
                        app::fold_for_search(ui::library_cell_text(row, c));
            times.add(k6_keys, ms_since(t));
            if (check) {
                // set_charts keeps its rows in the order it was given them.
                std::string why = rows.size() == m.rows().size() ? "" : "a different row count";
                for (size_t i = 0; why.empty() && i < rows.size(); ++i)
                    if (!same_row(rows[i], m.rows()[i])) why = "row " + std::to_string(i);
                if (!why.empty()) times.differs(k6_keys, "the passes' rows are not set_charts': " + why);
            }
            // The sort and the filter set_charts ends with, on its own rows:
            // a sort change re-sorts and re-filters, and a chip change only
            // re-filters. Every row is Not analyzed here (no results read),
            // so that chip shows every row, as All does.
            m.set_sort(ui::LibrarySort::Title, false);
            t = clk::now();
            m.set_sort(ui::LibrarySort::Title, true);
            times.add(k6_sorted, ms_since(t));
            t = clk::now();
            m.set_chip(ui::StatusChip::NotAnalyzed);
            times.add(k6_filter, ms_since(t));
        }
        std::fflush(stdout);
    }

    times.print();
    std::printf("\nWorked out from the medians above:\n");
    if (!times.differed(k1_real) && !times.differed(k2_real))
        std::printf("  the counted bar's cost (1's read_library minus 2's): %.1f ms\n",
                    times.median(k1_real) - times.median(k2_real));
    if (winner_profile && !times.differed(k5_paths) && !times.differed(k5_winner))
        std::printf("  lookup with the winner-only path read (lookup - today's path read + "
                    "winner-only path read): %.1f ms\n",
                    times.median(k5_lookup) - times.median(k5_paths) + times.median(k5_winner));
    return 0;
}

static int bench_main() {
    // --rules <path> and the legacy fills flag may sit anywhere; take them out
    // so the positional mode checks below see the same argv they always did.
    const std::vector<std::string> all = utf8_argv();
    std::vector<std::string> argv;
    std::optional<std::string> rules_path;
    for (size_t i = 0; i < all.size(); ++i) {
        if (i > 0 && all[i] == "--rules" && i + 1 < all.size()) {
            rules_path = all[++i];
            continue;
        }
        if (i > 0 && all[i] == app::kLegacyFillsFlag) {
            g_legacy_fills = true;
            continue;
        }
        argv.push_back(all[i]);
    }
    const int argc = static_cast<int>(argv.size());
    if (const int code = app::load_tool_rules(rules_path, g_rules)) return code;

    if (argc > 2 && argv[1] == "--replay") {
        replay_mode(argv[2]);
        return 0;
    }
    if (argc > 2 && argv[1] == "--upgrade") {
        upgrade_mode(argv[2]);
        return 0;
    }
    if (argc > 2 && argv[1] == "--open") {
        int runs = 7;  // the count the --open request asked for by default
        std::optional<std::string> ini;
        bool as_ready = false;
        // The options start after argv[0] (the program), argv[1] (--open)
        // and argv[2] (the file): the index of the first one is that count.
        const int first_option = 3;
        for (int i = first_option; i < argc; ++i) {
            if (argv[i] == "--runs" && i + 1 < argc) runs = std::stoi(argv[++i]);
            else if (argv[i] == "--ini" && i + 1 < argc) ini = argv[++i];
            else if (argv[i] == "--as-ready") as_ready = true;
            else {
                // A misspelt --as-ready would time a library that reads Stale
                // without saying so, so an option nobody reads is refused. A
                // known option with no value after it lands here too.
                std::fprintf(stderr, "hydra_bench --open: %s\n",
                             hydra::unknown_option_line(argv[i]).c_str());
                return 2;
            }
        }
        return open_mode(argv[2], runs, ini, as_ready);
    }
    if (argc > 3 && argv[1] == "--dump-db") {
        std::string dumprel;
        for (int i = 4; i < argc; ++i)
            if (argv[i] == "--dump-rel" && i + 1 < argc) dumprel = argv[++i];
        dump_db(argv[2], argv[3], dumprel);
        return 0;
    }
    if (argc > 2 && argv[1] == "--engine") {
        std::string cachedb, outp;
        int reps = 1;
        for (int i = 3; i < argc; ++i) {
            if (argv[i] == "--cache" && i + 1 < argc) cachedb = argv[++i];
            else if (argv[i] == "--reps" && i + 1 < argc) reps = std::stoi(argv[++i]);
            else if (argv[i] == "--out" && i + 1 < argc) outp = argv[++i];
        }
        app::Settings st;
        if (const int code = ini_settings(st)) return code;
        engine_mode(argv[2], st, cachedb, reps, outp);
        return 0;
    }
    if (argc > 2 && argv[1] == "--parse") {
        std::string outp;
        int reps = 1;
        bool dyn = true;
        for (int i = 3; i < argc; ++i) {
            if (argv[i] == "--reps" && i + 1 < argc) reps = std::stoi(argv[++i]);
            else if (argv[i] == "--out" && i + 1 < argc) outp = argv[++i];
            else if (argv[i] == "--nodyn") dyn = false;
        }
        app::Settings st;
        if (const int code = ini_settings(st)) return code;
        parse_mode(argv[2], st, reps, outp, dyn);
        return 0;
    }
    if (argc > 2 && argv[1] == "--scan") {
        std::string folder = argv[2], db, dump, dumprel;
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--db" && i + 1 < argc) db = argv[++i];
            else if (arg == "--dump" && i + 1 < argc) dump = argv[++i];
            else if (arg == "--dump-rel" && i + 1 < argc) dumprel = argv[++i];
        }
        scan_mode(folder, db, dump, dumprel);
    } else if (argc > 1) {
        folder_breakdown(argv[1], g_rules);
    } else {
        corpus_bench();
    }
    return 0;
}

int main() { return app::run_tool(bench_main); }
