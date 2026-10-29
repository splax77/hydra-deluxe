// doctest entry point for the Hydra C++ parity/unit harness. It runs doctest
// between the two checks of tests/run_guard.h, which refuse a command line
// that would test less than it names.
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "run_guard.h"

namespace {

// How many test cases passed the filters, from doctest's own run stats; empty
// until a run ends.
std::optional<unsigned> g_passing_filters;

// Records the run's count of test cases that passed the filters. It prints
// nothing, so every reporter's output stays as doctest writes it.
struct PassingFiltersListener : doctest::IReporter {
    explicit PassingFiltersListener(const doctest::ContextOptions&) {}
    void report_query(const doctest::QueryData&) override {}
    void test_run_start() override {}
    void test_run_end(const doctest::TestRunStats& stats) override {
        g_passing_filters = stats.numTestCasesPassingFilters;
    }
    void test_case_start(const doctest::TestCaseData&) override {}
    void test_case_reenter(const doctest::TestCaseData&) override {}
    void test_case_end(const doctest::CurrentTestCaseStats&) override {}
    void test_case_exception(const doctest::TestCaseException&) override {}
    void subcase_start(const doctest::SubcaseSignature&) override {}
    void subcase_end() override {}
    void log_assert(const doctest::AssertData&) override {}
    void log_message(const doctest::MessageData&) override {}
    void test_case_skipped(const doctest::TestCaseData&) override {}
};

REGISTER_LISTENER("passing_filters", 0, PassingFiltersListener);

}  // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> args(argv + (argc > 0 ? 1 : 0), argv + argc);
    if (const std::optional<std::string> why = run_guard::refuse_arguments(args)) {
        std::cerr << *why << "\n";
        return run_guard::kRefusedExit;
    }

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    const int code = context.run();
    if (code != 0 || !g_passing_filters) return code;

    if (const std::optional<std::string> why = run_guard::refuse_zero_match(args, *g_passing_filters)) {
        std::cerr << *why << "\n";
        return run_guard::kRefusedExit;
    }
    return code;
}
