// hydra_fillcompare — build a sortable HTML page comparing the same charts
// scored under Clone Hero 1.0's fill-spawn rule against Clone Hero 1.1's.
// Each result carries its fill rule in its profile, so one database holds a
// chart's 1.0 record beside its 1.1 record: hydra_batch fills it with one run
// plain and one with --legacy-fills, and the app's "1.0 fills" setting does
// the same. This reads both from that one database, joins them by chart hash
// and reports where they agree or disagree. Without --db it reads the app's
// own database, as hydra_batch does.
//
//     hydra_fillcompare
//     hydra_fillcompare --db both.db
//     hydra_fillcompare --db both.db --out fill_compare.html
//     hydra_fillcompare --db both.db --no-open
//     hydra_fillcompare --db both.db --rules hydra_rules.ini
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
#include "core/unknown_option.h"
#include "core/winstr.h"
#include "store/record_store.h"

namespace {

int fillcompare_main() {
    SetConsoleOutputCP(CP_UTF8);
    const std::vector<std::string> args = hydra::utf8_argv();
    const int argc = static_cast<int>(args.size());

    std::optional<std::string> dbpath;
    std::string out = "fill_compare.html";
    bool open_when_done = true;
    std::optional<std::string> rulespath;

    for (int i = 1; i < argc; ++i) {
        const std::string& arg = args[i];
        if (arg == "--db" && i + 1 < argc) dbpath = args[++i];
        else if (arg == "--out" && i + 1 < argc) out = args[++i];
        else if (arg == "--rules" && i + 1 < argc) rulespath = args[++i];
        else if (arg == "--no-open") open_when_done = false;
        else if (arg == "--old" || arg == "--new") {
            // The two-file form is gone: one database holds both rules' results.
            std::fprintf(stderr,
                         "%s is gone: one database holds both the 1.0 and the 1.1 results. "
                         "Name it with --db <path>, or leave --db out to read the app's own.\n",
                         arg.c_str());
            return 2;
        } else {
            std::fprintf(stderr, "%s\n", hydra::unknown_option_line(arg).c_str());
            std::fprintf(stderr,
                "Usage: hydra_fillcompare [--db <path>] [--out fill_compare.html] "
                "[--no-open] [--rules hydra_rules.ini]\n");
            return 2;
        }
    }

    // A bad settings file stops the run before the store opens, as a bad
    // rules file does (D270 call 4).
    hydra::app::Settings settings;
    try {
        settings = hydra::app::Settings::load();
    } catch (const hydra::app::SettingsFileError& e) {
        std::fprintf(stderr, "%s\n",
                     hydra::app::ini_file_tool_text(hydra::app::kSettingsFileName, e).c_str());
        return 2;
    }
    if (const int code = hydra::app::load_tool_rules(rulespath, settings.rules)) return code;
    const std::string db = dbpath ? *dbpath : hydra::app::db_path();

    // A database that won't open is a run that can't start (D72 item 5).
    std::unique_ptr<hydra::store::RecordStore> store;
    try {
        store = hydra::app::open_store(db, hydra::core::RulesStamp::of(settings.rules));
    } catch (const std::exception& e) {
        return hydra::app::tool_error(e, 2);
    }

    // generate_fill_report reads the database under each fill rule itself.
    hydra::app::fill_report::GeneratedFillReport report =
        hydra::app::fill_report::generate_fill_report(*store, settings);

    store->close();

    if (report.html.empty()) {
        std::printf("%s\n", report.reason.c_str());
        return 1;
    }

    std::filesystem::path outpath = std::filesystem::absolute(hydra::os_path(std::filesystem::u8path(out)));
    std::error_code ec;
    std::filesystem::create_directories(hydra::os_path(outpath.parent_path()), ec);

    try {
        hydra::app::write_report_file(outpath, report.html);
    } catch (const std::exception& e) {
        return hydra::app::tool_error(e, 1);
    }

    const hydra::app::fill_report::FillCompareStats& stats = report.stats;
    // The last count's words are the page's (D114 item 26).
    std::printf(
        "Compared %s: %s same, %s 1.0 higher, %s 1.1 higher, "
        "%s only under 1.0, %s only under 1.1, %s %s\n",
        hydra::counted(stats.total, "chart", "charts").c_str(),
        hydra::group_thousands(stats.same).c_str(),
        hydra::group_thousands(stats.ch10_higher).c_str(),
        hydra::group_thousands(stats.ch11_higher).c_str(),
        hydra::group_thousands(stats.only_old).c_str(),
        hydra::group_thousands(stats.only_new).c_str(),
        hydra::group_thousands(stats.in_both).c_str(),
        hydra::app::fill_report::kInBothPhrase);
    std::printf("Wrote %s\n", out.c_str());

    if (open_when_done) {
        if (!hydra::app::open_in_browser(outpath.wstring()))
            std::fprintf(stderr, "Could not open the page automatically.\n");
    }
    return 0;
}

}  // namespace

int main() { return hydra::app::run_tool(fillcompare_main); }
