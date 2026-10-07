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
// --legacy-fills needs its own --db, and each run stamps its database with the
// rule it used; a run whose rule disagrees with an existing stamp exits 2
// without writing. Compare two such databases with
// hydra_fillcompare. Every result now carries its rule in its key, so the app
// keeps 1.0 and 1.1 results side by side in hydra.db; these two guards are the
// command line's behavior from before that, kept as it was (docs/adr/0010).
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
#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/rules_file.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/strutil.h"
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

// Do these two paths name the same file? Compared after resolving `.`, `..`
// and relative prefixes (weakly_canonical works on a file that doesn't exist
// yet), then case-insensitively, because Windows paths are. Used only by the
// --legacy-fills guard, where a false negative is the dangerous answer, so a
// filesystem error falls back to comparing the strings rather than allowing
// the write.
bool same_file(const std::string& a, const std::string& b) {
    auto normalize = [](const std::string& p) {
        std::error_code ec;
        std::filesystem::path canon =
            std::filesystem::weakly_canonical(hydra::os_path(p), ec);
        return hydra::to_lower_ascii(ec ? p : canon.u8string());
    };
    return normalize(a) == normalize(b);
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
    try {
        settings.rules = hydra::app::load_rules_file(
            rulespath ? std::filesystem::u8path(*rulespath) : hydra::app::default_rules_path());
    } catch (const hydra::app::RulesFileError& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }
    hydra::app::BatchRun run = settings.batch_run();
    const hydra::app::AnalysisSettings& analysis = run.settings;
    const std::string& chartmode = run.chartmode;
    std::string db = dbpath ? *dbpath : hydra::app::db_path();

    // A legacy run gets its own file, and never writes the one the GUI reads:
    // the command line's rule from before results carried their fill rule
    // (docs/adr/0010), kept unchanged. The app's "1.0 fills" setting is the
    // way to put 1.0 results in hydra.db.
    if (legacy_fills && same_file(db, hydra::app::db_path())) {
        std::fprintf(stderr,
                     "--legacy-fills would write Clone Hero 1.0 results into the "
                     "database Hydra itself reads:\n  %s\n"
                     "hydra_batch keeps 1.0 results out of hydra.db by design; use the "
                     "app's 1.0 fills setting for that.\n"
                     "Give the run its own database, e.g. --db legacy.db\n",
                     db.c_str());
        return 2;
    }

    // A database that won't open is a run that can't start (D72 item 5).
    std::unique_ptr<hydra::store::RecordStore> store_ptr;
    try {
        store_ptr = hydra::app::open_store(db, hydra::core::RulesStamp::of(settings.rules));
    } catch (const std::exception& e) {
        return hydra::app::tool_error(e, 2);
    }
    hydra::store::RecordStore& store = *store_ptr;

    // One database holds one fill rule (docs/adr/0010). Which rule a file
    // holds is RecordStore::stamped_fill_rule's answer. The raw stamp is read
    // as well, for the message and for a stamp no rule reads, which this run
    // refuses rather than overwrites.
    const hydra::FillDeadlineRule run_rule = hydra::fill_rule_for(legacy_fills);
    const std::string run_mode = hydra::engine_mode_stamp(run_rule);
    const std::optional<std::string> stamp = store.engine_mode();
    const std::optional<hydra::FillDeadlineRule> file_rule = store.stamped_fill_rule();
    if (file_rule ? *file_rule != run_rule : stamp.has_value()) {
        const std::string file_mode = stamp ? *stamp : hydra::engine_mode_stamp(*file_rule);
        std::fprintf(stderr,
                     "This database holds results scored by the %s fill rule "
                     "(engine_mode=%s):\n  %s\n"
                     "This run scores by the %s rule. This file is stamped with the "
                     "other rule.\n"
                     "%s --legacy-fills to write into this database, or give this run "
                     "its own database with --db.\n",
                     file_rule ? hydra::fill_rule_name(*file_rule, hydra::FillRuleNameStyle::Long)
                               : "unknown",
                     file_mode.c_str(), db.c_str(),
                     hydra::fill_rule_name(run_rule, hydra::FillRuleNameStyle::Long),
                     legacy_fills ? "Drop" : "Add");
        return 2;
    }
    // Stamp the file with the rule this run used, so hydra_fillcompare can tell
    // a 1.0 database from a 1.1 one.
    store.set_engine_mode(run_mode);

    std::vector<std::string> folders =
        folder_args.empty() ? settings.chartfolders : folder_args;
    if (folders.empty()) {
        std::printf("No chart folders. Add them in Hydra, or pass folders as arguments.\n");
        return 1;
    }

    std::printf("Database   : %s\n", db.c_str());
    std::printf("Chart mode : %s\n", chartmode.c_str());
    const hydra::app::SettingsText described = hydra::app::describe_settings(analysis);
    std::printf("Depth      : %s\n", described.depth.c_str());
    std::printf("SP cap     : %s\n", described.cap.c_str());
    std::printf("Timing cap : %s\n", described.timing.c_str());
    std::printf("Squeeze win: %d ms\n", static_cast<int>(hydra::kSqueezeWindowMs));
    std::printf("Fill rule  : %s\n",
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
        hydra::store::ChartLibraryCache cache;
        try {
            cache = store.chart_library_cache();
        } catch (const std::exception&) {
            // No cache is only a slower scan.
        }
        auto [scanitems, folder_errors] = hydra::app::discover_charts(
            folders, hydra::app::ScanCallbacks{}, cache.empty() ? nullptr : &cache);
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
    // hydra_batch prints the raw text; the sentence is the GUI's.
    callbacks.on_error = [&](const std::string& raw_title, const std::string& /*sentence*/,
                             const std::string& error) {
        const std::string title = hydra::display_title(raw_title);
        failures.push_back(title + ": " + error);
        std::printf("[%d/%d] FAILED %s: %s\n", last.completed, last.total, title.c_str(),
                    error.c_str());
    };
    callbacks.on_result = [&](const hydra::app::ScanItem& item,
                              const hydra::store::PreparedRow& row) {
        // The artist and the title read the one cleaned form every screen shows.
        std::string label =
            hydra::display_artist(item.artist) + " - " + hydra::display_title(item.title);
        std::string score =
            row.summary.has_scored_best_path() ? hydra::group_thousands(*row.summary.score) : "-";
        std::printf("[%d/%d] %10s  %s %s\n", last.completed, last.total, score.c_str(),
                    clip_utf8(label, 52).c_str(), clip_utf8(row.bestpath, 36).c_str());
        std::fflush(stdout);
    };
    hydra::app::run_batch(plan, run, store, hydra::app::batch_worker_count(), callbacks);

    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();

    // Library rows, every copy of a chart counted (D76).
    std::printf("\nAnalyzed %s, skipped %s already stored, %s failed in %.1fs.\n",
                hydra::group_thousands(last.analyzed).c_str(),
                hydra::group_thousands(last.skipped).c_str(),
                hydra::group_thousands(last.failed).c_str(), elapsed);

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
