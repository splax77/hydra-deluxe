// The precheck's checks, in order. kChecks is the one place that says how
// many there are: --disable's validation, --list-checks and the run loop all
// read it.
#pragma once

#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "git_text.h"
#include "items.h"
#include "range.h"

namespace precheck {

// What a check reads, and where it puts what it finds.
struct Context {
    Repo& repo;
    const Range& range;
    const AddedLines& added;
    const std::vector<std::string>& all_files;  // all_files() at the tip
    Items& items;
    std::ostream& out;  // for a line a check prints itself, before the items
};

struct Check {
    const char* name;
    void (*run)(Context&);
};

// Check N lives in check_N.cpp; kChecks names each one.
void run_check_1(Context& ctx);
void run_check_2(Context& ctx);
void run_check_3(Context& ctx);
void run_check_4(Context& ctx);
void run_check_5(Context& ctx);

// Check N is kChecks[N - 1].
extern const std::vector<Check> kChecks;

// The check numbers in --disable's values, each a number or a
// comma-separated list ("2", "1,3"). Throws std::invalid_argument with the
// sentence the exe prints for anything outside 1 to kChecks.size().
std::set<int> parse_disable(const std::vector<std::string>& values);

}  // namespace precheck
