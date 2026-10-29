// hydra_tests' guard against command lines that run fewer tests than they
// name. tests/test_main.cpp calls it before and after the doctest run.
//
// Two traps it catches. PowerShell splits an unquoted native argument that
// starts with a dash at its first dot, so `-sf=*test_x.cpp` reaches the exe as
// `-sf=*test_x` and `.cpp`; doctest ignores the stray piece and runs nothing
// from that file. And doctest keeps only one occurrence of a repeated filter,
// so `"-sf=*a*" "-sf=*b*"` runs only one of the two. Either way the run used
// to end in "Status: SUCCESS!" with nothing tested.
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace run_guard {

// The exit code of a refused command line. doctest's own failure code is 1.
inline constexpr int kRefusedExit = 2;

// The comma-separated filter lists doctest reads.
enum class FilterKind {
    SourceFile,
    SourceFileExclude,
    TestSuite,
    TestSuiteExclude,
    TestCase,
    TestCaseExclude,
    Subcase,
    SubcaseExclude,
    Reporters,
};

struct FilterSpelling {
    std::string_view name;
    FilterKind kind;
    bool include;  // narrows which test cases run (the zero-match check)
};

// A copy of the filter names Context::parseArgs in
// third_party/doctest/doctest.h reads; that function owns them.
inline constexpr FilterSpelling kFilterSpellings[] = {
    {"source-file", FilterKind::SourceFile, true},
    {"sf", FilterKind::SourceFile, true},
    {"source-file-exclude", FilterKind::SourceFileExclude, false},
    {"sfe", FilterKind::SourceFileExclude, false},
    {"test-suite", FilterKind::TestSuite, true},
    {"ts", FilterKind::TestSuite, true},
    {"test-suite-exclude", FilterKind::TestSuiteExclude, false},
    {"tse", FilterKind::TestSuiteExclude, false},
    {"test-case", FilterKind::TestCase, true},
    {"tc", FilterKind::TestCase, true},
    {"test-case-exclude", FilterKind::TestCaseExclude, false},
    {"tce", FilterKind::TestCaseExclude, false},
    {"subcase", FilterKind::Subcase, false},
    {"sc", FilterKind::Subcase, false},
    {"subcase-exclude", FilterKind::SubcaseExclude, false},
    {"sce", FilterKind::SubcaseExclude, false},
    {"reporters", FilterKind::Reporters, false},
    {"r", FilterKind::Reporters, false},
};

// The flags that make a run list, count or print instead of running tests.
// A copy of the names Context::parseArgs reads, which owns them.
inline constexpr std::string_view kQueryFlags[] = {
    "help", "h", "?", "version", "v", "count", "c", "list-test-cases", "ltc",
    "list-test-suites", "lts", "list-reporters", "lr",
};

// The no-run option's names, which Context::parseArgs also owns. It takes a
// value as well as standing alone.
inline constexpr std::string_view kNoRunNames[] = {"no-run", "nr"};

// An option's name and value as doctest reads them: the text after the leading
// dashes and the optional "dt-" prefix (parseOptionImpl in doctest.h owns how
// those are matched). Empty for an argument that does not start with a dash.
inline std::string_view option_body(std::string_view arg) {
    if (arg.empty() || arg.front() != '-') return {};
    const size_t name_at = arg.find_first_not_of('-');
    if (name_at == std::string_view::npos) return {};
    arg.remove_prefix(name_at);
    if (arg.substr(0, 3) == "dt-") arg.remove_prefix(3);
    return arg;
}

// Whether `body` names `name`, alone or with an "=value" after it.
inline bool names_option(std::string_view body, std::string_view name) {
    return body.substr(0, name.size()) == name && (body.size() == name.size() || body[name.size()] == '=');
}

// The filter spelling an argument gives, if it gives one doctest reads. doctest
// skips a filter with an empty value, so that is no filter.
inline std::optional<FilterSpelling> filter_of(std::string_view arg) {
    const std::string_view body = option_body(arg);
    for (const FilterSpelling& s : kFilterSpellings)
        if (body.size() > s.name.size() + 1 && names_option(body, s.name)) return s;
    return std::nullopt;
}

// The refusal for a command line, before anything runs; nothing when it is
// fine. `args` are the arguments after the program's own name.
inline std::optional<std::string> refuse_arguments(const std::vector<std::string>& args) {
    for (const std::string& a : args) {
        if (a.empty() || a.front() != '-')
            return "hydra_tests: the argument \"" + a +
                   "\" does not start with a dash, so it is not an option. PowerShell splits an unquoted "
                   "filter at its first dot, so put quotes around the whole filter, for example "
                   "\"-sf=*test_x.cpp\".";
    }
    for (size_t i = 0; i < args.size(); ++i) {
        const std::optional<FilterSpelling> first = filter_of(args[i]);
        if (!first) continue;
        for (size_t j = i + 1; j < args.size(); ++j) {
            const std::optional<FilterSpelling> again = filter_of(args[j]);
            if (again && again->kind == first->kind)
                return "hydra_tests: \"" + args[i] + "\" and \"" + args[j] +
                       "\" are the same kind of filter. doctest reads only the last of a repeated filter "
                       "and drops the others without a word. Join the patterns with commas in one "
                       "filter, or run each as its own command.";
        }
    }
    return std::nullopt;
}

// The refusal for a run that tested nothing, after it ends; nothing when it is
// fine. `passing` is the number of test cases that passed the filters, from
// doctest's run stats. Only a real run with an include filter can be refused:
// a listing may match nothing on purpose, and an exclude-only run is the rest
// of the suite.
inline std::optional<std::string> refuse_zero_match(const std::vector<std::string>& args, unsigned passing) {
    if (passing != 0) return std::nullopt;
    std::string named;
    bool any_include = false;
    for (const std::string& a : args) {
        const std::string_view body = option_body(a);
        for (std::string_view flag : kQueryFlags)
            if (body == flag) return std::nullopt;
        // no-run with any value skips the check, even "=false": a missed
        // check costs no more than a plain doctest run did.
        for (std::string_view name : kNoRunNames)
            if (names_option(body, name)) return std::nullopt;
        const std::optional<FilterSpelling> f = filter_of(a);
        if (!f || f->kind == FilterKind::Reporters) continue;
        any_include = any_include || f->include;
        named += (named.empty() ? "\"" : " \"") + a + "\"";
    }
    if (!any_include) return std::nullopt;
    return "hydra_tests: no test case matched the filters " + named +
           ", so this run tested nothing. Check the patterns.";
}

}  // namespace run_guard
