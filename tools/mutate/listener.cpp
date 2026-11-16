// The doctest listener of the mutation build. tests/test_main.cpp includes
// this file only when HYDRA_MUTATION is defined, so the normal build never
// sees it. It writes what each test did to the file HYDRA_MUTANT_OUT names,
// for the runner to read; with that variable unset it writes nothing.
//
// The file is text, one record per line, fields split by tabs. A tab, a new
// line, a carriage return or a backslash inside a field is written as \t, \n,
// \r or \\. Each line is flushed as it is written, so a run that crashes or is
// stopped for time still leaves every line before that point. The records:
//
//   start      <test name> <test file> <test line>
//     A test began. A start with no matching test record is the test that
//     was running when the process died.
//   test       <test name> <test file> <test line> <passed|failed> <switches>
//     A test ended. passed or failed is doctest's own verdict for it. The
//     switches are the numbers noted while it ran, split by commas, in the
//     order they were first noted; empty when it reached none.
//   check      <test name> <check file> <check line>
//     A check or FAIL message in that test failed. Warnings are left out, as
//     doctest does not count them as failures.
//   exception  <test name> <message>
//     The test threw, or crashed, with that message.
//   outside    <when> <switches>
//     Switches noted while no test was running. <when> is "before any test"
//     for static start-up and anything else before the first test, and
//     "outside any test" for notes made after a test had ended.
//
// The switches and their notes live in tools/mutate/hydra_mutant.h.

#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "doctest.h"
#include "hydra_mutant.h"

namespace hydra_mutant_listener {

// Reads HYDRA_MUTANT during static start-up. Its first read allocates, and a
// test that counts heap allocations must see none from a switch, so the read
// is made here, in a file the test program always links, before main.
const long read_at_start = hydra_mutant::active_switch();

inline std::string escape_field(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '\t': out += "\\t"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            default: out += c; break;
        }
    }
    return out;
}

inline std::string join_switches(const std::vector<long>& switches) {
    std::string out;
    for (size_t i = 0; i < switches.size(); ++i) {
        if (i > 0) out += ',';
        out += std::to_string(switches[i]);
    }
    return out;
}

// The output file, opened once from HYDRA_MUTANT_OUT. A path that cannot be
// opened stops the program: a run with no result file would read as a run in
// which no test reached anything.
class ResultFile {
public:
    static ResultFile& get() {
        static ResultFile file;
        return file;
    }

    // Writes one record from its fields, escaped and joined by tabs.
    void write(const std::vector<std::string>& fields) {
        if (file_ == nullptr) return;
        std::string line;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) line += '\t';
            line += escape_field(fields[i]);
        }
        line += '\n';
        std::lock_guard<std::mutex> lock(mutex_);
        std::fputs(line.c_str(), file_);
        std::fflush(file_);
    }

private:
    ResultFile() {
        const std::optional<std::string> path = read_env("HYDRA_MUTANT_OUT");
        if (!path) return;
        if (fopen_s(&file_, path->c_str(), "wb") != 0 || file_ == nullptr) {
            std::fprintf(stderr, "HYDRA_MUTANT_OUT=\"%s\" cannot be opened for writing\n",
                         path->c_str());
            std::fflush(stderr);
            std::abort();
        }
    }
    ~ResultFile() {
        if (file_ != nullptr) std::fclose(file_);
    }
    ResultFile(const ResultFile&) = delete;
    ResultFile& operator=(const ResultFile&) = delete;

    std::FILE* file_ = nullptr;
    std::mutex mutex_;
};

struct MutationListener : doctest::IReporter {
    explicit MutationListener(const doctest::ContextOptions&) { ResultFile::get(); }

    void report_query(const doctest::QueryData&) override {}
    void test_run_start() override {}

    void test_run_end(const doctest::TestRunStats&) override { write_outside(); }

    void test_case_start(const doctest::TestCaseData& tc) override {
        write_outside();
        test_name_ = tc.m_name;
        test_file_ = tc.m_file.c_str();
        test_line_ = tc.m_line;
        ResultFile::get().write({"start", test_name_, test_file_, std::to_string(test_line_)});
    }

    // A subcase runs the test again; its notes belong to the same test.
    void test_case_reenter(const doctest::TestCaseData&) override {}

    void test_case_end(const doctest::CurrentTestCaseStats& st) override {
        const std::vector<long> reached = hydra_mutant::take_reached();
        ResultFile::get().write({"test", test_name_, test_file_, std::to_string(test_line_),
                                 st.testCaseSuccess ? "passed" : "failed", join_switches(reached)});
        any_test_ended_ = true;
    }

    void test_case_exception(const doctest::TestCaseException& e) override {
        ResultFile::get().write({"exception", test_name_, e.error_string.c_str()});
    }

    void subcase_start(const doctest::SubcaseSignature&) override {}
    void subcase_end() override {}

    void log_assert(const doctest::AssertData& ad) override {
        if (!ad.m_failed || (ad.m_at & doctest::assertType::is_warn)) return;
        ResultFile::get().write({"check", test_name_, ad.m_file, std::to_string(ad.m_line)});
    }

    void log_message(const doctest::MessageData& mb) override {
        if (mb.m_severity & doctest::assertType::is_warn) return;
        ResultFile::get().write({"check", test_name_, mb.m_file, std::to_string(mb.m_line)});
    }

    void test_case_skipped(const doctest::TestCaseData&) override {}

private:
    // Writes the notes made since the last test ended, if there are any.
    void write_outside() {
        const std::vector<long> reached = hydra_mutant::take_reached();
        if (reached.empty()) return;
        ResultFile::get().write({"outside", any_test_ended_ ? "outside any test" : "before any test",
                                 join_switches(reached)});
    }

    std::string test_name_;
    std::string test_file_;
    unsigned test_line_ = 0;
    bool any_test_ended_ = false;
};

REGISTER_LISTENER("hydra_mutation", 1, MutationListener);

}  // namespace hydra_mutant_listener
