// Check 4: scans outside the scan file (kind E). A test reading the source
// tree through HYDRA_SOURCE_DIR, or walking it, is a scan row ("Which test
// reads the source tree?", "Which test walks the source tree?"); those row
// items come from check_rows.cpp. This file adds the spellings no row can see
// yet, and the checks on the rule rows a range adds.
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "checks.h"
#include "core/strutil.h"
#include "single_owner/engine.h"
#include "views.h"

namespace precheck {

// What check 4 needs from the scan rows, which check_rows.cpp reads. The
// join fills it in run_check_4 below; tests fill it by hand. Declared here,
// not in a header, until the join gives it a home.
struct Check4Rows {
    // The file and line of every kind-E row hit on an added line. Check 4's
    // own spellings skip these, so a line prints once.
    std::set<std::pair<std::string, int>> e_hit_lines;
    // A rule row the range added (check_rows.cpp's "row added" answer), with
    // the rules file it is in and the line its object opens on.
    struct AddedRow {
        const single_owner::OwnerRule* rule;
        std::string file;
        int line;
    };
    std::vector<AddedRow> added_rows;
};

// Check 4 without the row items: the spellings and the new-row checks.
void run_check_4_spellings(Context& ctx, const Check4Rows& rows);

namespace {

const std::regex& py_reads_tree() {
    // No row can own this: the scan reads only .cpp and .h files.
    static const std::regex walk("(\\bos\\.walk|\\bos\\.listdir|\\.r?glob)\\s*\\([^)]*src", std::regex::icase);
    return walk;
}
const std::regex& py_opens_source() {
    static const std::regex open("\\bopen\\s*\\([^)]*\\.(cpp|h)['\"]", std::regex::icase);
    return open;
}
const std::regex& cpp_opens_source() {
    // No row: "Which test reads the source tree?" matches only
    // HYDRA_SOURCE_DIR, so a test that opens a source file by a path built
    // another way (sourcetree::root() / "src" / ...) passes the scan. When
    // that row widens, this entry goes.
    static const std::regex open("\\b(ifstream|fopen|_wfopen|open_file\\w*)\\b[^;]*(\\.(cpp|h)\"|\"[^\"]*/?src/)",
                                 std::regex::icase);
    return open;
}
// A negative lookahead or lookbehind naming variables inside a row's
// pattern, such as "(?!(base|combo)\s*\.".
const std::regex& variable_exemption() {
    static const std::regex rx("\\(\\?<?!\\(?[A-Za-z_]\\w*(\\|[A-Za-z_]\\w*)*\\)?\\\\s\\*\\(?\\\\?\\.",
                               std::regex::icase);
    return rx;
}

}  // namespace

void run_check_4_spellings(Context& ctx, const Check4Rows& rows) {
    const std::string why_spelling = std::string("the single-owner scan is the one scan of the source tree; make "
                                                 "this a row in ") +
                                     single_owner::kRulesDir;
    for (const std::string& f : ctx.added.files()) {
        // Every code folder but src: a scan is a test's or a tool's to make.
        if (top_folder(f) == "src") continue;
        if (skipped_file(f)) continue;
        const bool py = is_py(f);
        if (!is_cpp(f) && !py) continue;
        const View v = make_view(ctx.repo.files().text(ctx.range.tip, f), view_lang(f));
        for (const int ln : ctx.added.lines(f, v.line_count())) {
            if (rows.e_hit_lines.count({f, ln})) continue;
            const std::string t = v.line(v.no_comments, ln);
            const char* what = nullptr;
            if (py) {
                if (std::regex_search(t, py_reads_tree()) || std::regex_search(t, py_opens_source()))
                    what = "reads the source tree";
            } else if (std::regex_search(t, cpp_opens_source())) {
                what = "opens a source file";
            }
            if (what) ctx.items.add("E", f, ln, std::string(what) + ": " + hydra::trim(t), why_spelling);
        }
    }
    // New rule rows: every example list must hold something, and no pattern
    // may exempt variable names.
    for (const Check4Rows::AddedRow& r : rows.added_rows) {
        const std::pair<const char*, const std::vector<std::string>*> lists[] = {
            {"must-match", &r.rule->must_match}, {"must-not-match", &r.rule->must_not_match}};
        for (const auto& [name, list] : lists) {
            if (list->empty())
                ctx.items.add("E", r.file, r.line,
                              "scan row \"" + r.rule->question + "\" has no " + name + " examples",
                              "every scan row ships with lines it must match and must not match, so the self-test "
                              "proves its pattern");
        }
        std::smatch m;
        if (std::regex_search(r.rule->pattern, m, variable_exemption()))
            ctx.items.add("E", r.file, r.line,
                          "scan row \"" + r.rule->question + "\" exempts variable names inside its pattern (" +
                              m.str(0) + "...)",
                          "an exemption in the pattern applies in every file; list the allowed lines as owner lines "
                          "with their reason");
    }
}

void run_check_4(Context& ctx) {
    Check4Rows rows;
    // JOIN (check_rows.cpp): add the scan rows' E items here, as the script's
    // Add-RowItems @('E'), and fill `rows`: e_hit_lines from the same E hits,
    // added_rows from the rows the range added.
    run_check_4_spellings(ctx, rows);
}

}  // namespace precheck
