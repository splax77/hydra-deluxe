// Check 2: recompute spellings and other bad test shapes in tests (kind B),
// each a candidate for the reviewer to decide. The scan rows print their
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
    static const std::regex rx("hit.?window|timing.?step", std::regex::icase);
    return rx;
}

// The backreference to the first group a pattern adds after `head`, so the
// group's number follows head's own groups however many it has.
std::string group_after(const std::string& head) {
    return "\\" + std::to_string(std::regex(head + "()").mark_count());
}

// How one language spells the parts the constant-pin and echo entries are
// built from.
struct Shapes {
    std::string head;    // what opens an assertion
    std::string number;  // a typed number
    std::string whole;   // what may not follow a typed number that is read whole
    std::string name;    // a product constant read by name
    std::string approx;  // the framework's approximate match, up to its "("
    std::string before;  // what may stand just before a typed number on the left of ==
    std::string ends;    // what may follow the last operand of a comparison
};

const Shapes& cpp_shapes() {
    static const Shapes s = {"(?:" + assert_call_pattern() + "|\\bstatic_assert\\s*\\()",
                             "-?\\d[\\w.']*",
                             "(?![\\w.'])",
                             "\\b(?:\\w+::)*k[A-Z]\\w*",
                             "(?:doctest::)?Approx",
                             "[(,&|]",
                             "(?=\\s*(?:[),;&|]|$))"};
    return s;
}

const Shapes& py_shapes() {
    static const Shapes s = {"(?:\\bassert\\b|" + assert_call_pattern() + ")",
                             "-?\\d[\\w.]*",
                             "(?![\\w.])",
                             "\\b(?:[A-Za-z_]\\w*\\.)*[A-Z][A-Z0-9_]*\\b",
                             "(?:pytest\\.)?approx",
                             "(?:[(,]|\\band\\b|\\bor\\b)",
                             "(?=\\s*(?:[),]|$|and\\b|or\\b))"};
    return s;
}

// A product constant read by name, compared with a typed number in one
// assertion: on either side of ==, or as either argument of a two-argument
// assertion. The typed number may sit inside the approximate match.
std::string constant_pin_pattern(const Shapes& s) {
    const std::string number = s.number + s.whole;
    const std::string typed = "(?:" + s.approx + "\\s*\\(\\s*" + number + "\\s*\\)|" + number + ")";
    return s.head + "(?:[^;]*?" + s.name + "\\s*==\\s*" + typed + s.ends + "|(?:[^;]*?" + s.before + ")?\\s*" +
           typed + "\\s*==\\s*" + s.name + s.ends + "|\\s*" + s.name + "\\s*,\\s*" + typed + s.ends + "|\\s*" +
           typed + "\\s*,\\s*" + s.name + s.ends + ")";
}

// One assertion that types the same number as a call's argument and as the
// expected side. Either the argument comes first and the expected side
// follows == or a comma, or the expected side opens the assertion and the
// argument follows.
std::string echo_pattern(const Shapes& s) {
    const std::string same = group_after(s.head);
    return s.head + "(?:[^;]*?[(,]|(?=\\s*-?\\d))\\s*(" + s.number + ")\\s*(?:[,)][^;]*(?:==\\s*" + same + s.whole +
           "|,\\s*" + same + "\\s*[,)])|(?:==|,)[^;]*[(,]\\s*" + same + "\\s*[,)])";
}

// A stand-in's name, as each language writes it. A longer word such as
// "Mockup" is not one.
const char* const kStandInName = "(?:Fake|Stub|Mock)(?![a-z])";
const char* const kPyStandInName = "(?:[Ff]ake|[Ss]tub|[Mm]ock)(?![a-z])";

// The advice the constant-pin, echo and stand-in entries print, the same in
// both languages.
const char* const kConstantPinWhy =
    "a constant pin proves only that the value is still the value; read the constant by name and check the "
    "behaviour it drives (docs/agents/test-rubric.md, question 5)";
const char* const kEchoWhy =
    "an echo states nothing the code works out; feed it a value the real code makes and require that back, or "
    "check what the code works out from its input (docs/agents/test-rubric.md, question 8)";
const char* const kStandInWhy =
    "a fake stand-in says nothing about how the app behaves; unless it is input the real code reads, feed the "
    "real code real input (docs/agents/test-rubric.md, question 9)";

const std::vector<Spelling>& cpp_spellings() {
    static const std::vector<Spelling> list = [] {
        std::vector<Spelling> v;
        // A row owns part of this question, not this spelling. The row "What
        // is the two-hit budget at the identity scale?" gives the budget to
        // nominal_budget_ms and catches its product shapes in src/ and tools/.
        // It does not read tests/ and does not match a bare 2.0. This entry
        // covers that: a typed 2.0 near the words "hit window" or "timing step" in a test.
        // When the row widens to tests/, this entry goes.
        v.push_back({std::regex("2\\.0*(?![\\w.'])"), "w.'", &hit_window_context(),
                     "a typed 2.0 beside a hit-window check",
                     "the hit-window rule has one owner; read it by name and check the behaviour it drives "
                     "(docs/agents/test-rubric.md, question 4)"});
        // No row: the scan has no row for counting tied paths a second way.
        v.push_back({std::regex("tied_pathcount\\(\\)\\s*[!=]=\\s*[^;]*(\\d\\s*\\+|\\+\\s*\\d|\\.size\\(\\)|count\\b)"),
                     "", nullptr, "the tied-path count worked out a second way",
                     "Path::recount_tied_paths owns the count; compare with what it returns "
                     "(docs/agents/test-rubric.md, question 4)"});
        // No row: this is a shape of test, not one rule's question, so no
        // single owner fits a scan row.
        v.push_back({std::regex(assert_call_pattern() +
                                "[^;]*==\\s*[^;]*([\\w)\\]]\\s*\\([^()]*\\)\\s*(?:\\+|-(?!>)|\\*|/)\\s*[A-Za-z_(]|"
                                "[\\w)\\]]\\s*(?:\\+|-(?!>)|\\*|/)\\s*[\\w:<>]+\\s*\\()"),
                     "", nullptr, "an expected value computed from other calls",
                     "a copy of the module's steps agrees with any bug in them; state the expected behaviour "
                     "directly (docs/agents/test-rubric.md, questions 4 and 5)"});
        // No row: the same, a shape of test with no single owner.
        v.push_back({std::regex("\\b(want|wanted|expect|expected)\\w*\\s*=\\s*[^;]*[!=]=[^;]*(\\|\\||&&)"), "",
                     nullptr, "the expected answer built from the predicate's own comparisons",
                     "a copy of the predicate agrees with any bug in it; compare with the owner's answer or a "
                     "property (docs/agents/test-rubric.md, question 4)"});
        // No row: the three below are kinds of bad test the rubric names, not
        // one rule's question. Each line is a candidate the reviewer decides;
        // a test's own constant, or a stand-in that is input the real code
        // reads, looks the same on one line.
        v.push_back({std::regex(constant_pin_pattern(cpp_shapes())), "", nullptr,
                     "a product constant compared with a typed number", kConstantPinWhy});
        v.push_back({std::regex(echo_pattern(cpp_shapes())), "", nullptr,
                     "the same number typed as the input and as the expected value", kEchoWhy});
        v.push_back({std::regex(std::string("\\b(?:struct|class|using)\\s+") + kStandInName), "", nullptr,
                     "a stand-in named Fake, Stub or Mock", kStandInWhy});
        return v;
    }();
    return list;
}

// No row: these too are shapes of test with no single owner.
const std::vector<Spelling>& py_spellings() {
    static const std::vector<Spelling> list = [] {
        std::vector<Spelling> v;
        v.push_back({std::regex("[<>]=?\\s*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}\\b"), "", nullptr,
                     "a fixture decides with the module's own constant",
                     "the fixture makes the comparison production makes; feed the real code the inputs and check "
                     "what it decides (docs/agents/test-rubric.md, question 5)"});
        v.push_back({std::regex(assert_call_pattern() +
                                "\\s*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}\\s*,[^)]*[A-Za-z_]\\w*\\.[A-Z][A-Z0-9_]{2,}"),
                     "", nullptr, "a module constant checked against a formula of other constants",
                     "a constant checked against its own formula is a copy; check the behaviour the constant "
                     "drives (docs/agents/test-rubric.md, question 5)"});
        v.push_back({std::regex(constant_pin_pattern(py_shapes())), "", nullptr,
                     "a product constant compared with a typed number", kConstantPinWhy});
        v.push_back({std::regex(echo_pattern(py_shapes())), "", nullptr,
                     "the same number typed as the input and as the expected value", kEchoWhy});
        v.push_back({std::regex(std::string("\\b(?:class|def)\\s+") + kPyStandInName + "|\\bimport\\b.*\\b" +
                                kPyStandInName + "|\\bunittest\\.mock\\b"),
                     "", nullptr, "a stand-in named Fake, Stub or Mock, or unittest.mock", kStandInWhy});
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
