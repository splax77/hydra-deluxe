// The single-owner row scan's engine: reading the rows and the baselines,
// deciding whether a line answers a row's question, and scanning one file.
// The doctest cases in scan.cpp and the hydra_precheck tool both call it, so
// this is the one place that decides whether a line answers a row. Nothing
// here includes doctest or reads the repo root; callers pass the root and
// the file text in.
#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <istream>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "../code_lexer.h"
#include "json.hpp"
#include "listed.h"
#include "rows.h"

namespace single_owner {

// ---- Reading the rows and the baselines ----

// Where the rows and the baselines live, relative to the repo root.
inline constexpr const char* kRulesDir = "tests/single_owner/rules";
inline constexpr const char* kBaselineFile = "tests/single_owner/baseline.json";

// How many days after its date a baseline entry may stay at most. The user's
// cap, from "Baselines that shrink" in
// docs/superpowers/plans/2026-10-10-single-owner-redesign.md.
inline constexpr int kBaselineMaxDays = 14;

struct RuleTables {
    std::vector<OwnerRule> rules;
    std::vector<KnownCopy> copies;
    std::vector<KnownClone> clones;
};

// One row of a rules file. `index` is 0-based and names the row only when it
// has no question to name it by. Throws std::runtime_error naming the rules
// file and the row on a missing, unknown or mistyped field.
OwnerRule parse_rule(const nlohmann::json& row, const std::string& rules_file, size_t index);

// Every row of one rules file, given its text; `name` is the name the scan
// reports it by (select_rule_files).
std::vector<OwnerRule> parse_rules_file(const std::string& name, const std::string& json_text);

// The known copies and known clones of baseline.json, given its text. The
// result's `rules` is empty.
RuleTables parse_baseline(const std::string& json_text);

// The baseline entry's dates, checked against kBaselineMaxDays. `where`
// starts every failure.
BaselineDates parse_dates(const nlohmann::json& entry, const std::string& where);

// `iso` (YYYY-MM-DD) moved `days` later, as YYYY-MM-DD, or empty when `iso`
// is not in that form. A date that does not exist (2026-02-30) comes back as
// a different day, so iso_plus_days(s, 0) == s says s is a real date.
std::string iso_plus_days(const std::string& iso, int days);

// How a baseline entry is named in failures.
std::string copy_entry_name(const KnownCopy& k);
std::string clone_entry_name(const KnownClone& k);

// Which of `paths` are rules files, as the names the scan reports them by,
// in byte order. Each path is relative to the rules folder, with forward
// slashes; the disk lister and hydra_precheck's git listing both ask this.
std::vector<std::string> select_rule_files(const std::vector<std::string>& paths);

// The rules files under the folder `dir` on disk, each with its name
// (select_rule_files), in name order.
std::vector<std::pair<std::string, std::filesystem::path>> rule_files(const std::filesystem::path& dir);

// Every row of every rules file under `dir` on disk, in name order.
std::vector<OwnerRule> load_rules(const std::filesystem::path& dir);

// The rows and baselines of the repo at `root` on disk.
RuleTables load_tables(const std::filesystem::path& root);

// ---- The verdict on one line ----

struct CompiledRule {
    const OwnerRule* rule;
    std::regex pattern;
    std::regex calls_owner;
};

CompiledRule compile_rule(const OwnerRule& r);

// The one verdict: does this line answer the rule's question without calling
// its owner?
bool flags_line(const CompiledRule& c, const std::string& line);

// The language codelex reads a file in (codelex::language_of_extension), or
// C++ for a file it does not know.
codelex::Lang lexer_language(const std::filesystem::path& path);

// Whether a trimmed line is a comment line the row scans skip, in the
// language the file is read in.
bool skipped_comment_line(const std::string& trimmed, codelex::Lang lang);

// ---- Which files a row reads ----

// Does this rule scan the file `rel`, which sits under the top folder `sub`?
bool in_scope(const OwnerRule& r, const std::string& sub, const std::string& rel);

// The language the walk reads the file `rel` in, or nothing when the walk
// does not read it.
std::optional<codelex::Lang> walk_language(const std::string& rel);

// Whether the walk reads the file `rel` for this row: it reads the file at
// all, and in the row's language.
bool walk_reads_for(const OwnerRule& r, const std::string& rel);

// ---- Scanning a file ----

// The lines a scan accepts by their exact text: every row's owner lines,
// then every baseline copy.
std::vector<ListedLine> listed_lines(const std::vector<OwnerRule>& rules,
                                     const std::vector<KnownCopy>& copies);

// What the row scan carries from one file to the next.
struct ScanState {
    const std::vector<CompiledRule>& compiled;
    const std::vector<ListedLine>& listed;
    std::vector<bool> used;            // which listed entries a line took
    std::vector<int> functions_found;  // per compiled row
};

ScanState start_scan(const std::vector<CompiledRule>& compiled, const std::vector<ListedLine>& listed);

// One flagged line: a line that answers a row's question without calling its
// owner, and what the listed lines said about it.
struct RowHit {
    size_t rule_index;  // into ScanState::compiled
    int line;           // 1-based
    std::string text;   // the line, trimmed
    Take take;
    // The listed entry it took, into ScanState::listed, when take is taken.
    std::optional<size_t> listed_index;
};

using HitSink = std::function<void(const RowHit&)>;

// The row scan of one file: `rel` is its repo-relative path, `py` says it is
// Python, `in` reads its text. Every flagged line goes to `sink`, taken or
// not.
void scan_file(ScanState& s, const std::string& rel, bool py, std::istream& in, const HitSink& sink);

}  // namespace single_owner
