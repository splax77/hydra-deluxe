// The rows and the baselines the single-owner cases read. The data lives in
// tests/single_owner/rules/ (one JSON file per area) and
// tests/single_owner/baseline.json; load_tables in scan.cpp is their one
// reader, and these accessors return what it read.
#pragma once

#include <string>
#include <vector>

namespace single_owner {

// A file whose matches answer a different question than the rule's.
struct Exempt {
    std::string file;  // repo-relative, forward slashes
    std::string why;
};

// One line inside the owner that answers the question. A rule that lists its
// owner's lines instead of naming the whole file guards the owner too: a
// second answer added there fails like one anywhere else.
struct OwnerLine {
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
    std::string why;        // the owner, or the recorded decision that allows it
};

struct OwnerRule {
    std::string question;                  // plain English, shown on failure
    std::string owner;                     // the one place that answers it
    std::string pattern;                   // ECMAScript regex, matched per line
    std::string calls_owner;               // a line also matching this calls the owner: it passes
    std::vector<std::string> owner_files;  // repo-relative files that may match freely
    std::vector<Exempt> exempt;
    std::string date;                      // the day the row was added
    std::string decision;                  // ADR, D-number or review key that decided it
    std::string added_by;                  // the task or review that added the row
    std::string note;                      // why the pattern is shaped as it is
    // The owner-gone guard (owner_gone) checks this file still names this
    // symbol, so a moved or deleted owner cannot leave the row checking nothing.
    std::string owner_file;                // repo-relative
    std::string owner_symbol;
    std::string rules_file;                // the file the row came from, as "rules/<area>.json"
    std::vector<std::string> must_match;   // lines the rule flags
    std::vector<std::string> must_not_match;
    std::vector<OwnerLine> owner_lines;    // the owner's own lines; each must still match
    // Top folders scanned, or single repo-relative files; empty means src and
    // tools.
    std::vector<std::string> scope;
    // When set, the rule checks only one function's body in one file: from
    // the line in function_file that contains `function` to the next line
    // that is a lone "}". The scan fails if it never finds that line.
    std::string function_file;
    std::string function;
    // Whole-line comments are skipped unless this is set (a rule about a
    // name that comments must not use).
    bool scan_comments = false;
};

struct KnownCopy {
    std::string question;   // equals a rule's question
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // the line with leading and trailing space trimmed
    std::string removed_by; // the fix that deletes it
};

// A copy the clone scan tolerates for now. The key survives edits elsewhere in
// either file: the two files, the block's length and its first line. Line
// numbers are not part of it.
struct KnownClone {
    std::string file_a;  // the earlier file in sorted path order
    std::string file_b;  // the same as file_a when a file repeats itself
    int lines;           // code lines, as clone_line_text counts them
    std::string first;   // the block's first code line, as clone_line_text gives it
};

const std::vector<OwnerRule>& rules();
const std::vector<KnownCopy>& known_copies();
const std::vector<KnownClone>& known_clones();

}  // namespace single_owner
