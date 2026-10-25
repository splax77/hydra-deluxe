// What the precheck reads: which commits a range names, which lines it adds,
// how big it is, and what kind of file each path is. Every check asks these
// questions here, so each answer is written once.
#pragma once

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "git_text.h"

namespace precheck {

// ------------------------------------------------------------- file kinds

// Which top folder of the repo is this file in (src, tools, tests...)? Empty
// for a file at the repo root.
std::string top_folder(const std::string& file);
// Is this a C++ file? Asked of single_owner::is_cpp_source, so the precheck
// reads the same files the single-owner scan does and no others.
bool is_cpp(const std::string& file);
// Is this a C++ file that is not a header? Check 5 compares free functions
// between these only.
bool is_cpp_implementation(const std::string& file);
// Is this a Python file? Asked of codelex::language_of_extension, which keeps
// the extension's case as the scan does; only test_file and skipped_file
// ignore case.
bool is_py(const std::string& file);
// Is this a test file? Any file that is not a test is production code.
bool test_file(const std::string& file);
// Is this one of the scan's own files, whose strings and literals are pattern
// examples, so no check reads it line by line? Old ranges still add lines to
// the file the scan's C++ lived in before it moved, so that file counts too.
bool skipped_file(const std::string& file);

// The top folders the precheck lists and diffs: the ones the scan walks
// (sourcetree::kWalkedFolders).
const std::vector<std::string>& code_folders();

// ------------------------------------------------------------------ range

// The commits a range spelling names.
struct Range {
    std::string spelling;  // as given: "main...HEAD", "A..B", a commit name
    bool whole_tree = false;
    std::string tip;        // the range's last commit
    std::string left;       // A of "A...B" or "A..B" (its decisions count too); empty otherwise
    std::string base;       // what the range is diffed against; empty for a single commit with whole_tree
    std::string rules_rev;  // where the scan rows are read: rules_at, or the tip
};

// Resolves "A...B" (what B adds since it forked from A), "A..B" (over A
// itself), or a single commit (over its first parent). An empty side means
// HEAD, as in git. With whole_tree a single commit has no base. `rules_at`
// empty means the tip.
Range resolve_range(const Repo& repo, const std::string& spelling, bool whole_tree,
                    const std::string& rules_at);

// Every file under code_folders() at `rev`, in git's order.
std::vector<std::string> all_files(const Repo& repo, const std::string& rev);

// ------------------------------------------------------------ added lines

// Which lines the range adds, per file: 1-based line numbers in the tip's
// numbering. A file whose entry holds nothing (std::nullopt) has every line
// added; that is every listed file of a whole-tree run.
class AddedLines {
public:
    // Every line of every file in `files` counts as added.
    static AddedLines whole(const std::vector<std::string>& files);
    // The lines `diff -U0 base tip` adds under code_folders().
    static AddedLines from_diff(const Repo& repo, const std::string& base, const std::string& tip);
    // The same, read from the diff's text lines (the parser from_diff uses).
    static AddedLines parse_diff(const std::vector<std::string>& diff_lines);

    bool has_file(const std::string& file) const { return sets_.count(file) != 0; }
    bool added(const std::string& file, int line) const;
    // The added lines of `file` up to `count`, in order.
    std::vector<int> lines(const std::string& file, int count) const;
    // The files the range touches (has an entry for), in byte order.
    std::vector<std::string> files() const;

private:
    std::map<std::string, std::optional<std::set<int>>> sets_;
};

// --------------------------------------------------------------- the size

// How many lines a range changes, outside the folders range_size leaves out.
struct RangeSize {
    long long added = 0;
    long long deleted = 0;
    long long total() const { return added + deleted; }
};

// The folders that do not count toward the size: vendored code and test
// data, which the reviewer does not read line by line.
const std::vector<std::string>& size_excluded_folders();

// Added plus deleted lines over every file the range touches outside
// size_excluded_folders(). A binary file counts no lines (git gives it no
// line counts). The one place that rule lives.
RangeSize range_size(const Repo& repo, const std::string& base, const std::string& tip);
// The same, read from `diff --numstat` lines.
RangeSize parse_numstat(const std::vector<std::string>& numstat_lines);

}  // namespace precheck
