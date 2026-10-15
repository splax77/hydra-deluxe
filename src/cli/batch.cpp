// hydra_batch — batch-analyze many charts straight into the record store.
// The C++ port of hydra_batch.py.
//
//     hydra_batch                    # every folder in the app's settings; saves the scan as the library
//     hydra_batch <folder> [...]     # specific folders instead; the library is left alone
//     hydra_batch --redo             # re-analyze charts already stored
//     hydra_batch --db <path>        # target a specific database
//     hydra_batch --legacy-fills     # score fills by Clone Hero 1.0's rule
//     hydra_batch --rules <path>     # rule choices from this file, not the exe's hydra_rules.ini
//
// Every result carries its fill rule in its key, so one file, the app's own
// hydra.db included, can hold a 1.0 run and a 1.1 run side by side, each read
// under its own rule; hydra_fillcompare compares the two.
//
// Reads difficulty / pro drums / 2x bass / depth / SP cap from the app's
// settings INI (app/config.h), so results match what the app would produce
// for the same songs. Safe to interrupt and re-run: charts already stored for
// the current chartmode and SP cap are skipped unless --redo is given.
//
// Unlike the Python version this analyzes charts across a thread pool (the
// same pool the GUI's "Analyze library" uses), so lines can complete out of
// chart-discovery order.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/rules_file.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/winstr.h"
#include "parse/song.h"
#include "search/graph.h"
#include "store/record_store.h"

namespace {

// Codepoint-safe prefix of a UTF-8 string, mirroring Python's label[:n] which
// slices characters, not bytes. Multi-byte sequences count as one column —
// close enough for console alignment.
std::string clip_utf8(const std::string& s, size_t max_chars) {
    size_t chars = 0, i = 0;
    while (i < s.size() && chars < max_chars) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
        i += len;
        ++chars;
    }
    return s.substr(0, i) + std::string(max_chars > chars ? max_chars - chars : 0, ' ');
}

int batch_main() {
    SetConsoleOutputCP(CP_UTF8);  // chart titles/artists are UTF-8
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());

    bool redo = false, legacy_fills = false;
    std::optional<std::string> dbpath;
    std::optional<std::string> rulespath;
    std::vector<std::string> folder_args;

    for (int i = 1; i < argc; ++i) {
        const std::string& arg = args[i];
        if (arg == "--redo") redo = true;
        else if (arg == hydra::app::kLegacyFillsFlag) legacy_fills = true;
        else if (arg == "--db" && i + 1 < argc) dbpath = args[++i];
        else if (arg == "--rules" && i + 1 < argc) rulespath = args[++i];
        else if (arg.rfind("--", 0) == 0) {
            std::fprintf(stderr, "Unknown option: %s\n", arg.c_str());
            return 2;
        } else {
            folder_args.push_back(arg);
        }
    }

    hydra::app::Settings settings = hydra::app::Settings::load_for_command_line(legacy_fills);
    if (const int code = hydra::app::load_tool_rules(rulespath, settings.rules)) return code;
    hydra::app::BatchRun run = settings.batch_run();
    const hydra::app::AnalysisSettings& analysis = run.settings;
    const std::string& chartmode = run.profile.chartmode;
    std::string db = dbpath ? *dbpath : hydra::app::db_path();

    // A database that won't open is a run that can't start (D72 item 5).
    std::unique_ptr<hydra::store::RecordStore> store_ptr;
    try {
        store_ptr = hydra::app::open_store(db, hydra::core::RulesStamp::of(settings.rules));
    } catch (const std::exception& e) {
        return hydra::app::tool_error(e, 2);
    }
    hydra::store::RecordStore& store = *store_ptr;
    // Each result is saved under this run's profile, which carries the rule.
    const hydra::FillDeadlineRule run_rule = hydra::fill_rule_for(legacy_fills);

    std::vector<std::string> folders =
        folder_args.empty() ? settings.chartfolders : folder_args;
    if (folders.empty()) {
        std::printf("No chart folders. Add them in Hydra, or pass folders as arguments.\n");
        return 1;
    }

    // The app's names and words for each setting (D114 items 9 and 23).
    std::printf("Database   : %s\n", db.c_str());
    std::printf("Chart mode : %s\n", chartmode.c_str());
    const hydra::app::SettingsText described = hydra::app::describe_settings(analysis);
    std::printf("Score range: %s\n", described.depth.c_str());
    std::printf("SP cap     : %s\n", described.cap.c_str());
    std::printf("Path limit : %s\n", described.timing.c_str());
    std::printf("Squeeze win: %d ms\n", static_cast<int>(hydra::kSqueezeWindowMs));
    std::printf("Fills      : %s\n",
                hydra::fill_rule_name(run_rule, hydra::FillRuleNameStyle::Long));
    std::printf("Folders    : %zu\n", folders.size());
    for (const std::string& f : folders) std::printf("    %s\n", f.c_str());

    // The scan and the plan it makes. The rescan cache and the scan's list are
    // read only here, so they go when this returns, before the first chart is
    // analyzed (memory audit fix 4); the plan keeps its own copy of each item.
    const hydra::app::BatchPlan plan = [&] {
        std::printf("\nDiscovering charts...\n");
        // The last library scan, if this database has one. A chart whose files
        // are unchanged (sig_unchanged) reuses its hash and song fields instead
        // of being read again. A run over the app's own folders saves its scan
        // as the library afterwards, as Scan library does (D79); a run over
        // folder arguments leaves the library alone.
        auto [scanitems, folder_errors] = hydra::app::discover_charts(
            folders, hydra::app::ScanCallbacks{}, hydra::app::load_scan_cache(store));
        if (folder_args.empty()) {
            if (const std::optional<std::string> problem =
                    hydra::app::save_scan_as_library(store, scanitems))
                std::printf("  ! %s\n", problem->c_str());
        }
        for (const std::string& err : folder_errors) std::printf("  ! %s\n", err.c_str());
        std::printf(
            "Found %s.\n\n",
            hydra::counted(static_cast<int64_t>(scanitems.size()), "chart", "charts").c_str());

        // The one plan for this run (D79). A run that fails as a whole (this
        // store read) ends through run_tool.
        return hydra::app::plan_batch(scanitems, hydra::app::charts_with_result(store, run, redo));
    }();

    auto started = std::chrono::steady_clock::now();

    // Every number printed is run_batch's: the progress for a row arrives
    // before that row's line.
    hydra::app::BatchProgress last;
    std::vector<std::string> failures;

    hydra::app::BatchCallbacks callbacks;
    callbacks.on_progress = [&](const hydra::app::BatchProgress& p) { last = p; };
    // hydra_batch prints one line per failure: app::plain_error_line.
    callbacks.on_error = [&](const std::string& raw_title, const std::string& sentence,
                             const std::string& error) {
        const std::string title = hydra::display_title(raw_title);
        const std::string line = hydra::app::plain_error_line(sentence, error);
        failures.push_back(title + ": " + line);
        std::printf("[%d/%d] FAILED %s: %s\n", last.completed, last.total, title.c_str(),
                    line.c_str());
    };
    callbacks.on_result = [&](const hydra::app::ScanItem& item,
                              const hydra::store::PreparedRecord& prepared) {
        // The artist and the title read the one cleaned form every screen shows.
        std::string label =
            hydra::display_artist(item.artist) + " - " + hydra::display_title(item.title);
        // The score and best path are the record's rank-1 path; a record
        // with no paths has neither (PathSummary::has_scored_best_path).
        const hydra::store::PathSummary summary = prepared.record.summary();
        std::string score =
            summary.has_scored_best_path() ? hydra::group_thousands(*summary.score) : "-";
        const std::string best = prepared.record.best_path_text();
        std::printf("[%d/%d] %10s  %s %s\n", last.completed, last.total, score.c_str(),
                    clip_utf8(label, 52).c_str(), clip_utf8(best, 36).c_str());
        std::fflush(stdout);
    };
    hydra::app::run_batch(plan, run, store, hydra::app::batch_worker_count(), callbacks);

    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    // Library rows, every copy of a chart counted (D76), in the app's words.
    std::printf("\n%s in %.1fs.\n",
                hydra::app::batch_counts_text(last.analyzed, last.failed, last.skipped).c_str(),
                elapsed);

    // The first 20 failure lines; the count is the progress's.
    constexpr int kFailuresShown = 20;
    if (last.failed > 0) {
        std::printf("\nFailures:\n");
        const int shown = std::min(last.failed, kFailuresShown);
        for (int i = 0; i < shown; ++i) std::printf("  %s\n", failures[i].c_str());
        if (last.failed > shown)
            std::printf("  ...and %s more.\n", hydra::group_thousands(last.failed - shown).c_str());
    }

    return 0;
}

}  // namespace

int main() { return hydra::app::run_tool(batch_main); }
