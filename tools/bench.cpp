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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/rules_file.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "replay_windows.h"
#include "search/graph.h"
#include "search/pather.h"
#include "song_digest.h"
#include "store/record_store.h"

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

    store::RecordStore store(":memory:", core::RulesStamp::of(rules));

    for (const app::ScanItem& it : items) {
        std::printf("%s\n", it.notespath.c_str());

        auto t = clk::now();
        std::optional<Song> song_opt;
        try {
            song_opt.emplace(load_songpath(it.notespath, settings.prodrums, settings.bass2x,
                                           settings.difficulty, settings.rules,
                                           settings.noteshuffle));
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
        store.add_record(gui.record_key(it.md5), rec);
        double store_s = secs_since(t);

        // summarize_record's best score: none for a record with no paths,
        // printed "-" the way hydra_batch prints it.
        const std::optional<int64_t> best = store::summarize_record(rec).score;
        const std::string best_text = best ? std::to_string(*best) : "-";
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
    if (!dbpath.empty()) store = std::make_unique<store::RecordStore>(dbpath, core::RulesStamp::of(g_rules));

    // With --db, a prior scan's rows become the rescan cache — running the
    // same command twice measures cold full scan then warm rescan.
    store::ChartLibraryCache cache;
    if (store) cache = store->chart_library_cache();
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
    auto [items, errors] =
        app::discover_charts({folder}, callbacks, cache.empty() ? nullptr : &cache);
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
            songs.push_back(load_songpath(path, true, true, Difficulty::Expert, g_rules));
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
// app::Settings::load_for_command_line, under this run's rules. Pointing a run
// at other settings means a copy of the exe beside another ini.
static app::Settings ini_settings() {
    app::Settings st = app::Settings::load_for_command_line(g_legacy_fills);
    st.rules = g_rules;
    return st;
}

// Engine mode, single-threaded:
//   hydra_bench --engine <folder> [--cache <db>] [--reps N] [--out <file>]
// Each chart once (plan_batch's first copy), in scan order: parse, a
// throwaway ScoreGraph build (timed on its own), analyze_chart N times, then
// prepare_row. Prints the phase sums and one digest over every prepared row
// (digest::row_hash); --out writes each chart's md5 and row hash. --cache
// takes a database's rescan cache, so the scan skips rehashing. A chart that
// fails is counted and left out of the digest.
static void engine_mode(const std::string& folder, const std::string& cachedb, int reps,
                        const std::string& outpath) {
    store::ChartLibraryCache cache;
    std::unique_ptr<store::RecordStore> cstore;
    if (!cachedb.empty()) {
        cstore = std::make_unique<store::RecordStore>(cachedb, core::RulesStamp::of(g_rules));
        cache = cstore->chart_library_cache();
    }
    auto [items, errors] = app::discover_charts({folder}, app::ScanCallbacks{},
                                                cache.empty() ? nullptr : &cache);
    const app::Settings st = ini_settings();
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
            song.emplace(load_songpath_with_notes(it.notespath, settings.prodrums, settings.bass2x,
                                                  settings.difficulty, settings.rules,
                                                  settings.noteshuffle));
        } catch (const std::exception&) {
            ++failed;
            continue;
        }
        t_parse += secs_since(t);
        t = clk::now();
        {
            ScoreGraph g(*song,
                         std::optional<int>(graph_build_cap(settings.sp_cap, song->sp_phrase_count())),
                         fill_rule_for(settings.legacy_fill_deadline), settings.rules);
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
        store::PreparedRow row = store::prepare_row(st.record_key(it.md5), *rec);
        t_prep += secs_since(t);
        const uint64_t h = digest::row_hash(row, *rec);
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
// time in microseconds, its hash and any failure.
static void parse_mode(const std::string& arg, int reps, const std::string& outpath, bool dyn) {
    std::vector<std::string> paths;
    if (arg.size() > 4 && arg.substr(arg.size() - 4) == ".txt") {
        std::ifstream in(hydra::os_path(arg));
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (!line.empty()) paths.push_back(line);
        }
    } else {
        auto [items, errors] = app::discover_charts({arg}, app::ScanCallbacks{}, nullptr);
        for (const app::ScanItem& it : app::plan_batch(items, {}).todo)
            paths.push_back(it.notespath);
    }
    const app::AnalysisSettings settings = ini_settings().batch_run().settings;
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
                Song song = load_songpath_with_notes(p, settings.prodrums, settings.bass2x,
                                                     settings.difficulty, settings.rules,
                                                     settings.noteshuffle);
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
    store::RecordStore db(dbpath, core::RulesStamp::of(g_rules));
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
// the scores-only one and one with no windows. The blink-182 Discography chart
// was the stress chart (Task 9 of the 2026-10-03 preview-loading plan). That
// the scores-only replay matches the full one is test_replay.cpp's job.
static void replay_mode(const std::string& chart) {
    const Song song = load_songpath(chart, true, true);
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
    const double full_ms = best_ms([&] { replay_path(song, wl, g_rules); });
    const double lean_ms = best_ms([&] { replay_path(song, wl, g_rules, scores); });
    const double none_ms = best_ms([&] { replay_path(song, {}, g_rules); });
    std::printf("%zu chords, %zu windows, best of 3: open-window walk %.1f ms, "
                "scores-only %.1f ms, no windows %.1f ms\n",
                song.sequence.size(), wl.size(), full_ms, lean_ms, none_ms);
}

// Upgrade mode:
//   hydra_bench --upgrade <db file>
// Opens the file as the app would, which upgrades an older layout for good,
// and prints how long each step of the open took. Point it at a scratch copy
// of a real library file (db, -wal and -shm together), never at the live one.
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
    { store::RecordStore db(dbpath, core::RulesStamp::of(g_rules), record); }
    const clk::time_point end = clk::now();

    auto name = [](store::OpenStep s) {
        switch (s) {
            case store::OpenStep::Opening: return "Opening";
            case store::OpenStep::UpdatingResultsKey: return "UpdatingResultsKey";
            case store::OpenStep::Copying: return "Copying";
            case store::OpenStep::Finishing: return "Finishing";
        }
        return "?";
    };
    for (size_t i = 0; i < began.size(); ++i) {
        const clk::time_point to = i + 1 < began.size() ? began[i + 1].second : end;
        std::printf("%s: %.3f s\n", name(began[i].first),
                    std::chrono::duration<double>(to - began[i].second).count());
    }
    std::printf("total: %.3f s\n", std::chrono::duration<double>(end - start).count());
    std::printf("rows copied: %lld of %lld\n", static_cast<long long>(last.rows_done),
                static_cast<long long>(last.rows_total));
    std::printf("file: %llu bytes before, %llu bytes after\n",
                static_cast<unsigned long long>(bytes_before),
                static_cast<unsigned long long>(file_size_bytes(dbpath)));
}

static int bench_main() {
    // --rules <path> and the legacy fills flag may sit anywhere; take them out
    // so the positional mode checks below see the same argv they always did.
    const std::vector<std::string> all = utf8_argv();
    std::vector<std::string> argv;
    std::string rules_path;
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
    try {
        g_rules = app::load_rules_file(rules_path.empty()
                                           ? app::default_rules_path()
                                           : std::filesystem::u8path(rules_path));
    } catch (const app::RulesFileError& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }

    if (argc > 2 && argv[1] == "--replay") {
        replay_mode(argv[2]);
        return 0;
    }
    if (argc > 2 && argv[1] == "--upgrade") {
        upgrade_mode(argv[2]);
        return 0;
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
        engine_mode(argv[2], cachedb, reps, outp);
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
        parse_mode(argv[2], reps, outp, dyn);
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
