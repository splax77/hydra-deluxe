// Check 2: recompute spellings in tests (kind B). The scan rows print their
// own B and A items (check_rows.cpp); this file adds the spellings no scan row
// asks about yet. Each is a question the scan has no row for; when a row
// lands, its entry here goes.
#include <algorithm>
#include <cctype>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include "check_rows.h"
#include "checks.h"
#include "core/strutil.h"
#include "definitions.h"
#include "views.h"

namespace precheck {

namespace {

bool word_char(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalnum(u) || c == '_';
}

// One spelling: a line matches when `rx` finds a match whose character
// before is not one of `not_after` (std::regex has no lookbehind), and, with
// a context pattern, when that pattern is found within two lines either side.
struct Spelling {
    std::regex rx;
    std::string not_after;  // "w" stands for any word character
    const std::regex* context;
    const char* what;
    const char* why;
};

bool spelling_matches(const Spelling& s, const std::string& code) {
    for (auto it = std::sregex_iterator(code.begin(), code.end(), s.rx); it != std::sregex_iterator(); ++it) {
        const size_t at = static_cast<size_t>(it->position(0));
        if (at == 0 || s.not_after.empty()) return true;
        const char before = code[at - 1];
        const bool blocked = s.not_after.find(before) != std::string::npos ||
                             (s.not_after.find('w') != std::string::npos && word_char(before));
        if (!blocked) return true;
    }
    return false;
}

const std::regex& hit_window_context() {
    static const std::regex rx("hit.?window", std::regex::icase);
    return rx;
}

const std::vector<Spelling>& cpp_spellings() {
    static const std::vector<Spelling> list = [] {
        std::vector<Spelling> v;
        // A row owns part of this question, not this spelling. The row "What
        // is the two-hit budget at the identity scale?" gives the budget to
        // nominal_budget_ms and catches its product shapes in src/ and tools/.
        // It does not read tests/ and does not match a bare 2.0. This entry
        // covers that: a typed 2.0 near the words "hit window" in a test.
        // When the row widens to tests/, this entry goes.
        v.push_back({std::regex("2\\.0*(?![\\w.'])"), "w.'", &hit_window_context(),
                     "a typed 2.0 beside a hit-window check",
                     "the hit-window rule has one owner; call it or pin the literal"});
        // No row: the scan has no row for counting tied paths a second way.
        v.push_back({std::regex("tied_pathcount\\(\\)\\s*[!=]=\\s*[^;]*(\\d\\s*\\+|\\+\\s*\\d|\\.size\\(\\)|count\\b)"),
                     "", nullptr, "the tied-path count worked out a second way",
                     "Path::recount_tied_paths owns the count; pin it"});
        // No row: this is a shape of test, not one rule's question, so no
        // single owner fits a scan row.
        v.push_back({std::regex(assert_call_pattern() +
                                "[^;]*==\\s*[^;]*([\\w)\\]]\\s*\\([^()]*\\)\\s*(?:\\+|-(?!>)|\\*|/)\\s*[A-Za-z_(]|"
                                "[\\w)\\]]\\s*(?:\\+|-(?!>)|\\*|/)\\s*[\\w:<>]+\\s*\\()"),
                     "", nullptr, "an expected value computed from other calls",
                     "a test pins literals from one run; it never computes the expected value"});
        // No row: the same, a shape of test with no single owner.
        v.push_back({std::regex("\\b(want|wanted|expect|expected)\\w*\\s*=\\s*[^;]*[!=]=[^;]*(\\|\\||&&)"), "",
                     nullptr, "the expected answer built from the predicate's own comparisons",
                     "pin a literal list of inputs and answers instead of restating the rule"});
        return v;
    }();
    return list;
}

// No row can own these: the scan reads only .cpp and .h files.
const std::vector<Spelling>& py_spellings() {
    static const std::vector<Spelling> list = [] {
        std::vector<Spelling> v;
        v.push_back({std::regex("[<>]=?\\s*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}\\b"), "", nullptr,
                     "a fixture decides with the module's own constant",
                     "the fixture makes the comparison production makes; pin the inputs as literals"});
        v.push_back({std::regex(assert_call_pattern() +
                                "\\s*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}\\s*,[^)]*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}"),
                     "", nullptr, "a module constant checked against a formula of other constants",
                     "the test restates the module's formula; pin the value"});
        return v;
    }();
    return list;
}

// Does this line end a C++ statement or open or close a block?
bool ends_statement(const std::string& line) {
    static const std::regex rx("[;{}]\\s*$");
    return std::regex_search(line, rx);
}

}  // namespace

void run_check_2(Context& ctx) {
    // The scan rows' own items first, then the spellings below.
    add_row_items_for_check(2, ctx);

    for (const std::string& f : ctx.added.files()) {
        if (skipped_file(f)) continue;
        const bool is_test = test_file(f);
        const bool cpp_test = is_test && is_cpp(f);
        const bool py_test = is_test && is_py(f);
        if (!cpp_test && !py_test) continue;
        const View v = make_view(ctx.repo.files().text(ctx.range.tip, f), view_lang(f));
        const int count = v.line_count();
        const std::vector<Spelling>& rules = py_test ? py_spellings() : cpp_spellings();
        std::set<std::pair<const Spelling*, int>> reported;  // spelling and line already shown
        for (const int ln : ctx.added.lines(f, count)) {
            std::string code = v.line(v.code, ln);
            std::string shown = hydra::trim(v.line(v.no_comments, ln));
            int last = ln;
            // A C++ statement split over lines is read whole, up to 4 lines
            // (decision D49).
            if (!py_test && !hydra::trim(code).empty() && !ends_statement(code)) {
                for (int k = ln + 1; k <= std::min(count, ln + 3); ++k) {
                    code += " " + hydra::trim(v.line(v.code, k));
                    shown += " " + hydra::trim(v.line(v.no_comments, k));
                    last = k;
                    if (ends_statement(v.line(v.code, k))) break;
                }
            }
            for (const Spelling& r : rules) {
                if (reported.count({&r, ln})) continue;
                if (!spelling_matches(r, code)) continue;
                if (r.context) {
                    // Context within 2 lines either side. D49 records this
                    // reach for the typed-500 rule, which is now a scan row;
                    // the 2.0 hit-window rule kept the same reach.
                    const int from = std::max(1, ln - 2);
                    const int to = std::min(count, last + 2);
                    std::string near;
                    for (int k = from; k <= to; ++k) near += (k > from ? "\n" : "") + v.line(v.raw, k);
                    if (!std::regex_search(near, *r.context)) continue;
                }
                for (int k = ln; k <= last; ++k) reported.insert({&r, k});
                ctx.items.add("B", f, ln, std::string(r.what) + ": " + shown, r.why);
            }
        }
    }
}

}  // namespace precheck
