// hydra_fillcompare — build a sortable HTML page comparing the same charts
// scored under Clone Hero 1.0's fill-spawn rule against Clone Hero 1.1's.
// It reads the 1.0 results from --old and the 1.1 results from --new, joins
// them by chart hash and reports where they agree or disagree. Each result
// carries its rule (docs/adr/0010), so --old and --new may name the same file:
// the app's own hydra.db once it holds both.
//
//     hydra_fillcompare --old ch10.db --new ch11.db
//     hydra_fillcompare --old ch10.db --new ch11.db --out fill_compare.html
//     hydra_fillcompare --old ch10.db --new ch11.db --no-open
//     hydra_fillcompare --old ch10.db --new ch11.db --rules hydra_rules.ini
//
// The page is self-contained: open it anywhere, click any column to sort. It
// opens in the default browser unless --no-open is given.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/rules_file.h"
#include "app/fill_report.h"
#include "app/report_files.h"
#include "app/user_messages.h"
#include "core/model.h"
#include "core/winstr.h"
#include "search/graph.h"
#include "store/record_store.h"

namespace {

int fillcompare_main() {
    SetConsoleOutputCP(CP_UTF8);
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());

    std::optional<std::string> old_path;
    std::optional<std::string> new_path;
    std::string out = "fill_compare.html";
    bool open_when_done = true;
    std::optional<std::string> rulespath;

    for (int i = 1; i < argc; ++i) {
        const std::string& arg = args[i];
        if (arg == "--old" && i + 1 < argc) old_path = args[++i];
        else if (arg == "--new" && i + 1 < argc) new_path = args[++i];
        else if (arg == "--out" && i + 1 < argc) out = args[++i];
        else if (arg == "--rules" && i + 1 < argc) rulespath = args[++i];
        else if (arg == "--no-open") open_when_done = false;
        else {
            std::fprintf(stderr, "Unknown option: %s\n", arg.c_str());
            return 2;
        }
    }

    if (!old_path || !new_path) {
        std::fprintf(stderr,
            "Usage: hydra_fillcompare --old <ch10.db> --new <ch11.db> "
            "[--out fill_compare.html] [--no-open] [--rules hydra_rules.ini]\n");
        return 2;
    }

    hydra::app::Settings settings = hydra::app::Settings::load();
    try {
        settings.rules = hydra::app::load_rules_file(
            rulespath ? std::filesystem::u8path(*rulespath) : hydra::app::default_rules_path());
    } catch (const hydra::app::RulesFileError& e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 2;
    }
    std::string chartmode = settings.chartmode_key();
    hydra::store::CapQuery cap = settings.cap_query();
    hydra::store::Lens lens = settings.lens();

    // A database that won't open is a run that can't start (D72 item 5).
    std::unique_ptr<hydra::store::RecordStore> old_store, new_store;
    try {
        old_store = hydra::app::open_store(*old_path, hydra::core::RulesStamp::of(settings.rules));
        new_store = hydra::app::open_store(*new_path, hydra::core::RulesStamp::of(settings.rules));
    } catch (const std::exception& e) {
        return hydra::app::tool_error(e, 2);
    }

    // Engine-mode sanity check (D65, ADR 0010): warn only when the file's
    // stamp names the other side's rule. An unstamped file never warns, so this
    // reads the stamp itself rather than asking stamped_fill_rule.
    auto warn_if_not = [](const std::string& path, hydra::store::RecordStore& store,
                          hydra::FillDeadlineRule expected) {
        const std::optional<std::string> stamp = store.engine_mode();
        if (!stamp) return;  // no stamp: nothing to disagree (D65)
        if (hydra::fill_rule_from_stamp(*stamp) == expected) return;
        std::fprintf(stderr, "Warning: %s is stamped engine_mode=%s, not %s\n",
                     path.c_str(), stamp->c_str(), hydra::engine_mode_stamp(expected));
    };
    warn_if_not(*old_path, *old_store, hydra::FillDeadlineRule::Ch10);
    warn_if_not(*new_path, *new_store, hydra::FillDeadlineRule::Ch11);

    hydra::app::fill_report::GeneratedFillReport report =
        hydra::app::fill_report::generate_fill_report(*old_store, *new_store, chartmode, cap, lens);

    old_store->close();
    new_store->close();

    if (report.html.empty()) {
        std::printf("%s\n", report.reason.c_str());
        return 1;
    }

    std::filesystem::path outpath = std::filesystem::absolute(hydra::os_path(std::filesystem::u8path(out)));
    std::error_code ec;
    std::filesystem::create_directories(hydra::os_path(outpath.parent_path()), ec);

    try {
        hydra::app::write_report_file(outpath, report.html);
    } catch (const std::exception&) {
        std::fprintf(stderr, "Cannot write %s\n", out.c_str());
        return 1;
    }

    const hydra::app::fill_report::FillCompareStats& stats = report.stats;
    std::printf(
        "Compared %s: %s same, %s 1.0 higher, %s 1.1 higher, "
        "%s only in 1.0, %s only in 1.1, %s with a score on one side only\n",
        hydra::counted(stats.total, "chart", "charts").c_str(),
        hydra::group_thousands(stats.same).c_str(),
        hydra::group_thousands(stats.ch10_higher).c_str(),
        hydra::group_thousands(stats.ch11_higher).c_str(),
        hydra::group_thousands(stats.only_old).c_str(),
        hydra::group_thousands(stats.only_new).c_str(),
        hydra::group_thousands(stats.in_both).c_str());
    std::printf("Wrote %s\n", out.c_str());

    if (open_when_done) {
        if (!hydra::app::open_in_browser(outpath.wstring()))
            std::fprintf(stderr, "Could not open the page automatically.\n");
    }
    return 0;
}

}  // namespace

int main() { return hydra::app::run_tool(fillcompare_main); }
