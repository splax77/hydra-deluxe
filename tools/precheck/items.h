// What hydra_precheck prints: the item list, its order, and every line's
// wording. The one place each printed line is spelled.
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "range.h"

namespace precheck {

// One thing for the reviewer to look at. `kind` is one letter; kind_order()
// lists them.
struct Item {
    std::string kind;
    std::string file;
    int line = 0;
    std::string what;
    std::string why;
};

// The kinds in the order they print.
const std::string& kind_order();

// The items the checks add, in the order they were added.
class Items {
public:
    void add(const std::string& kind, const std::string& file, int line, const std::string& what,
             const std::string& why);
    // Kind order, then file, line and what; an item equal to the one before
    // it on all four is dropped. Text compares as PowerShell's Sort-Object
    // compared it (text_before), so the order reads as it always has.
    std::vector<Item> sorted() const;

private:
    std::vector<Item> items_;
};

// Whether `a` sorts before `b` the way PowerShell 7's Sort-Object put them
// under en-US: case ignored, punctuation before digits before letters.
bool text_before(const std::string& a, const std::string& b);

// "<kind> <file>:<line>  <what>  -- <why>"
std::string item_line(const Item& item);

// Is this range too big to review in one read? A reviewer reads the whole
// range in one pass, and big ranges hide copies. The user's decision is D82
// item 2 in docs/audit/2026-10-03-fix-decisions.md, a starting value to be
// tuned later. This is the one place it lives.
constexpr long long kLargeRangeLines = 400;

// How the two size lines start.
extern const char* const kSizePrefix;
extern const char* const kLargeRangePrefix;

// "range size: N changed lines (A added, D deleted), not counting ..."
std::string size_line(const RangeSize& size);
// The over-the-threshold warning, or nothing when the range is not over it.
std::optional<std::string> large_range_line(const RangeSize& size);

// "derive_once_precheck: N items (<kind counts>|none) for <range|the whole
// tree> at <tip, 7 characters>"
std::string summary_line(const std::vector<Item>& sorted, const Range& range);

}  // namespace precheck
