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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "json.hpp"

#include "app/analysis.h"
#include "app/config.h"
#include "app/rules_file.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "search/pather.h"
#include "store/record_store.h"

using namespace hydra;
using clk = std::chrono::steady_clock;

// hydra_rules.ini (or --rules <path>): the rule choices every mode runs under.
static core::Rules g_rules;

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
                                           settings.difficulty, settings.rules));
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
        store.add_song(it.md5, it.title, it.artist, it.charter, song);
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
        std::vector<store::ChartLibraryEntry> entries;
        entries.reserve(items.size());
        for (const app::ScanItem& it : items) entries.push_back(app::to_library_entry(it));
        t0 = clk::now();
        store->rebuild_chart_library(entries);
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

static int bench_main() {
    // --rules <path> may sit anywhere; take it out so the positional mode
    // checks below see the same argv they always did.
    const std::vector<std::string> all = utf8_argv();
    std::vector<std::string> argv;
    std::string rules_path;
    for (size_t i = 0; i < all.size(); ++i) {
        if (i > 0 && all[i] == "--rules" && i + 1 < all.size()) {
            rules_path = all[++i];
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

    if (argc > 3 && argv[1] == "--dump-db") {
        std::string dumprel;
        for (int i = 4; i < argc; ++i)
            if (argv[i] == "--dump-rel" && i + 1 < argc) dumprel = argv[++i];
        dump_db(argv[2], argv[3], dumprel);
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
