// The precheck's row hits: added lines that a single-owner scan row flags.
// Whether a line answers a row is the scan engine's answer
// (tests/single_owner/engine.h); this file only reads the rows at the rules
// revision, keeps the hits on added lines, gives each hit its kind letter and
// its note, and says which rows and listed lines the range added. Checks 1, 2
// and 4 each print their own kinds from the one list.
#pragma once

#include <string>
#include <vector>

#include "checks.h"
#include "single_owner/rows.h"

namespace precheck {

// One flagged added line, as the item it prints.
struct RowItem {
    std::string kind;  // A, B, C or E (row_kind)
    std::string file;
    int line = 0;
    std::string what;
    std::string why;
};

// The kind letter of a hit in `file` on a row whose owner is `owner`: A in
// production code; in a test file E for a row owned by the source-tree
// reader, C for a row owned by another test file, B for a row owned by
// production code.
std::string row_kind(const std::string& file, const std::string& owner);

// The row hits of kinds in `kinds` (each a letter), in the order the scan
// found them. The rows are read and the files scanned once per range; every
// later call reads the same list.
std::vector<RowItem> row_items(Context& ctx, const std::string& kinds);

// Adds row_items(ctx, kinds) to ctx.items.
void add_row_items(Context& ctx, const std::string& kinds);

// The row items check `check` prints: check 1 the C hits, check 2 the B and
// A hits, check 4 the E hits. Any other check adds nothing.
void add_row_items_for_check(int check, Context& ctx);

// A row the range added, with where it sits in its rules file.
struct AddedRow {
    single_owner::OwnerRule rule;
    std::string file;  // the rules file, repo-relative
    int line = 0;      // the line of the row's opening brace
};

// The rows the range added (none when the rows are read at another commit
// than the tip), in rules-file order.
std::vector<AddedRow> added_rows(Context& ctx);

// The baseline's known copies at the rules revision, read with the rows.
const std::vector<single_owner::KnownCopy>& known_copies(Context& ctx);

// How an item's reason names the known copy a line is (the script's
// Format-KnownCopy), without the brackets around it.
std::string known_copy_note(const std::string& fix);

// Prints the warning that no scan rows were applied, when the rules
// revision has no rules files. The run prints it after every check, whether
// or not a check read the rows.
void print_rule_warnings(Context& ctx);

}  // namespace precheck
