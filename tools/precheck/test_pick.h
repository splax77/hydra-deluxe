// Which test files a merge range should run: the test picker of
// docs/development.md, "The merge test picker", steps 2, 4, 5 and 6.
// It reads the range's changed names (changed_names.h), follows one level
// of callers, finds the test files that name any of those keys, and decides
// when the whole suite runs instead. It prints nothing; the exe's --tests
// mode prints the result.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "changed_names.h"
#include "git_text.h"
#include "range.h"

namespace precheck {

// The user's three thresholds (D299). They live here and
// nowhere else.

// Q1: a key that more test files than this mention picks nothing by itself;
// the user chose 30.
constexpr int kCommonNameLimit = 30;
// Q2: more picked test files than this runs the whole suite; the user chose 40.
constexpr int kPickSizeLimit = 40;
// Q3: a changed header that more files under code_folders() include directly
// than this runs the whole suite; the user chose 20.
constexpr int kHeaderIncluderLimit = 20;

// The three limits pick_tests applies. Tests lower them so a small fixture
// can cross them; the exe always uses the defaults.
struct PickLimits {
    int common_name = kCommonNameLimit;
    int pick_size = kPickSizeLimit;
    int header_includers = kHeaderIncluderLimit;
};

// A name the range changes (Step 3), with the production file it came from.
struct PickName {
    std::string name;
    NameKind kind = NameKind::Body;
    std::string file;
    // Reached only through the base's deleted lines: the tip no longer
    // changes it anywhere (a deleted class, for one).
    bool base_only = false;
};

// One key the caller walk adds (Step 4): `key` is either the calling
// function's name (selector false) or an A::B value on the call line or its
// guard line (selector true).
struct CallerKey {
    std::string key;
    std::string callee;  // the changed name that is called
    std::string caller;  // the function the call sits in
    bool selector = false;
};

// A key over the common-name limit, with how many test files mention it.
struct CommonName {
    std::string name;
    int files = 0;
};

// One picked test file and every reason it was picked, in the order found:
// a key it names, or "changed in range".
struct PickedTest {
    std::string file;
    std::vector<std::string> reasons;
};

// Why the whole suite runs: the rule that fired, the file it fired on (empty
// for the size rule) and the count it compared (includers, mentions or
// picked files; 0 for a rule with no count).
struct WholeSuite {
    std::string file;
    std::string rule;
    int count = 0;
};

struct TestPick {
    std::vector<PickedTest> tests;  // in byte order of file name
    std::vector<PickName> names;
    std::vector<CallerKey> callers;
    std::vector<CommonName> common;
    std::optional<WholeSuite> whole_suite;  // set: run everything, `tests` is empty
};

// The reason ("build file", "test harness", or the header-includer rule
// with its count) the change of `file` runs the whole suite, or nothing.
// The one place shared code is decided. A header's direct includers are
// counted at `rev`.
std::optional<WholeSuite> shared_code(Repo& repo, const std::string& rev, const std::string& file,
                                      int header_includer_limit = kHeaderIncluderLimit);

// The test files `range` should run. Throws GitError when the range has no
// base (a whole-tree range).
TestPick pick_tests(Repo& repo, const Range& range, const PickLimits& limits = {});

// The lines `hydra_precheck --tests` prints for `pick`, in the plan's Step 7
// shape: tests, always, names, callers, common, then the why lines.
std::vector<std::string> pick_lines(const TestPick& pick);

}  // namespace precheck
