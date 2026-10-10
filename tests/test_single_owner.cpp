// One owner per rule, checked where a grep can check it. The rows are data,
// in tests/single_owner/rules/ (one JSON file per area), and the baselines are
// in tests/single_owner/baseline.json; load_tables below is their one reader.
// Each row names a question, the one place that answers it, and a pattern for
// the text that answers it. A matching line anywhere else fails here, unless
// it calls the owner on that same line, or the baseline lists it with the fix
// that will remove it. A baseline entry that no longer matches also fails, so the list
// only shrinks. Each listed line (baseline or owner) covers exactly one line
// of source, so a word-for-word copy of it fails too.
//
// This is the one scan of src/, tools/ and tests/. A row covers src/ and
// tools/ unless it names its own scope; the rows about tests (a test that
// recomputes an engine fact, or walks the source tree itself) cover tests/.
// The long-path rules (ADR 0020) are rows here too, so a new one-place rule
// is a new row, not a new walker. The walk itself, and the repo root, come
// from tests/source_tree.h. The clone scan at the end of this file (ADR 0025)
// reads the same walk for blocks of code pasted into two places.
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <istream>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/strutil.h"
#include "json.hpp"
#include "source_tree.h"

namespace fs = std::filesystem;

namespace {

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
    // the line in function_file that contains unction to the next line
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

// Defined with the clone scan below; the owner-gone guard reads source the
// same way.
std::string clone_line_text(std::string_view line, bool& in_comment);
bool is_word_byte(char c);

// ---- Reading the rows and the baselines ----
//
// Rule files are read in file-name order and rows in file order. A missing
// required field, a field the loader does not know, or a value of the wrong
// type fails with the rules file and the row's question, so a typo cannot
// drop a field quietly.

using json = nlohmann::json;

const char* const kRulesDir = "tests/single_owner/rules";
const char* const kBaselineFile = "tests/single_owner/baseline.json";

void check_keys(const json& obj, std::initializer_list<const char*> known, const std::string& where) {
    if (!obj.is_object()) throw std::runtime_error(where + ": expected a JSON object");
    for (const auto& item : obj.items()) {
        bool ok = false;
        for (const char* k : known) ok = ok || item.key() == k;
        if (!ok) throw std::runtime_error(where + ": unknown field \"" + item.key() + "\"");
    }
}

std::string required_string(const json& obj, const char* key, const std::string& where) {
    if (!obj.contains(key) || obj.at(key).get<std::string>().empty())
        throw std::runtime_error(where + ": missing field \"" + key + "\"");
    return obj.at(key).get<std::string>();
}

std::string optional_string(const json& obj, const char* key) {
    return obj.contains(key) ? obj.at(key).get<std::string>() : std::string();
}

std::vector<std::string> optional_strings(const json& obj, const char* key) {
    return obj.contains(key) ? obj.at(key).get<std::vector<std::string>>()
                             : std::vector<std::string>();
}

// One row of a rules file. `index` is 0-based and names the row only when it
// has no question to name it by.
OwnerRule parse_rule(const json& row, const std::string& rules_file, size_t index) {
    std::string where = rules_file + ": row " + std::to_string(index + 1);
    try {
        if (!row.is_object()) throw std::runtime_error(where + ": expected a JSON object");
        OwnerRule r;
        r.question = required_string(row, "question", where);
        where = rules_file + ": \"" + r.question + "\"";
        check_keys(row,
                   {"question", "owner", "owner_file", "owner_symbol", "date", "decision",
                    "added_by", "pattern", "calls_owner", "owner_files", "exempt", "must_match",
                    "must_not_match", "owner_lines", "scope", "function_file", "function",
                    "scan_comments", "note"},
                   where);
        r.owner = required_string(row, "owner", where);
        r.owner_file = required_string(row, "owner_file", where);
        r.owner_symbol = required_string(row, "owner_symbol", where);
        r.date = required_string(row, "date", where);
        r.decision = required_string(row, "decision", where);
        r.added_by = required_string(row, "added_by", where);
        r.pattern = required_string(row, "pattern", where);
        r.calls_owner = optional_string(row, "calls_owner");
        r.owner_files = optional_strings(row, "owner_files");
        if (row.contains("exempt"))
            for (const json& x : row.at("exempt")) {
                check_keys(x, {"file", "why"}, where + ": exempt");
                r.exempt.push_back({required_string(x, "file", where + ": exempt"),
                                    required_string(x, "why", where + ": exempt")});
            }
        r.must_match = optional_strings(row, "must_match");
        r.must_not_match = optional_strings(row, "must_not_match");
        if (row.contains("owner_lines"))
            for (const json& o : row.at("owner_lines")) {
                const std::string w = where + ": owner_lines";
                check_keys(o, {"file", "line_text", "why"}, w);
                r.owner_lines.push_back({required_string(o, "file", w),
                                         required_string(o, "line_text", w),
                                         required_string(o, "why", w)});
            }
        r.scope = optional_strings(row, "scope");
        r.function_file = optional_string(row, "function_file");
        r.function = optional_string(row, "function");
        r.scan_comments = row.contains("scan_comments") && row.at("scan_comments").get<bool>();
        r.note = optional_string(row, "note");
        r.rules_file = rules_file;
        return r;
    } catch (const json::exception& e) {
        throw std::runtime_error(where + ": " + e.what());
    }
}

json read_json_file(const fs::path& path, const std::string& name) {
    std::ifstream in(path);
    if (!in.good()) throw std::runtime_error(name + ": cannot read " + path.u8string());
    try {
        return json::parse(in);
    } catch (const json::exception& e) {
        throw std::runtime_error(name + ": " + e.what());
    }
}

struct RuleTables {
    std::vector<OwnerRule> rules;
    std::vector<KnownCopy> copies;
    std::vector<KnownClone> clones;
};

RuleTables load_tables() {
    const fs::path root = sourcetree::root();
    std::vector<fs::path> files;
    for (const fs::directory_entry& e : fs::directory_iterator(root / fs::u8path(kRulesDir)))
        if (e.is_regular_file() && e.path().extension() == ".json") files.push_back(e.path());
    std::sort(files.begin(), files.end(), [](const fs::path& x, const fs::path& y) {
        return x.filename() < y.filename();
    });
    RuleTables t;
    for (const fs::path& p : files) {
        const std::string name = "rules/" + p.filename().generic_u8string();
        const json doc = read_json_file(p, name);
        check_keys(doc, {"rules"}, name);
        if (!doc.contains("rules") || !doc.at("rules").is_array())
            throw std::runtime_error(name + ": missing field \"rules\"");
        const json& rows = doc.at("rules");
        for (size_t i = 0; i < rows.size(); ++i) t.rules.push_back(parse_rule(rows[i], name, i));
    }

    const std::string name = "baseline.json";
    const json doc = read_json_file(root / fs::u8path(kBaselineFile), name);
    check_keys(doc, {"known_copies", "known_clones"}, name);
    try {
        if (doc.contains("known_copies"))
            for (const json& k : doc.at("known_copies")) {
                const std::string w = name + ": known_copies";
                check_keys(k, {"question", "file", "line_text", "removed_by"}, w);
                t.copies.push_back({required_string(k, "question", w), required_string(k, "file", w),
                                    required_string(k, "line_text", w),
                                    required_string(k, "removed_by", w)});
            }
        if (doc.contains("known_clones"))
            for (const json& k : doc.at("known_clones")) {
                const std::string w = name + ": known_clones";
                check_keys(k, {"file_a", "file_b", "lines", "first"}, w);
                if (!k.contains("lines")) throw std::runtime_error(w + ": missing field \"lines\"");
                t.clones.push_back({required_string(k, "file_a", w), required_string(k, "file_b", w),
                                    k.at("lines").get<int>(), required_string(k, "first", w)});
            }
    } catch (const json::exception& e) {
        throw std::runtime_error(name + ": " + e.what());
    }
    return t;
}

const RuleTables& tables() {
    static const RuleTables t = load_tables();
    return t;
}

const std::vector<OwnerRule>& rules() { return tables().rules; }
const std::vector<KnownCopy>& known_copies() { return tables().copies; }
const std::vector<KnownClone>& known_clones() { return tables().clones; }

// The owner-gone guard: empty when the row's owner_file exists under `root`
// and names owner_symbol as a whole word outside comments (comments as the
// clone scan's clone_line_text drops them), else the failure message.
std::string owner_gone(const OwnerRule& r, const fs::path& root) {
    const std::string gone = r.rules_file + ": \"" + r.question +
                             "\": owner gone: name the new owner or retire the row (" +
                             r.owner_file + ", " + r.owner_symbol + ")";
    const fs::path path = root / fs::u8path(r.owner_file);
    if (r.owner_symbol.empty() || !fs::is_regular_file(path)) return gone;
    std::ifstream in(path);
    std::string line;
    bool in_comment = false;
    while (std::getline(in, line)) {
        const std::string code = clone_line_text(line, in_comment);
        for (size_t at = code.find(r.owner_symbol); at != std::string::npos;
             at = code.find(r.owner_symbol, at + 1)) {
            const size_t end = at + r.owner_symbol.size();
            if ((at == 0 || !is_word_byte(code[at - 1])) &&
                (end == code.size() || !is_word_byte(code[end])))
                return {};
        }
    }
    return gone;
}

struct CompiledRule {
    const OwnerRule* rule;
    std::regex pattern;
    std::regex calls_owner;
};

std::vector<CompiledRule> compile_rules() {
    const auto flags = std::regex::ECMAScript | std::regex::optimize;
    std::vector<CompiledRule> out;
    for (const OwnerRule& r : rules())
        out.push_back({&r, std::regex(r.pattern, flags),
                       std::regex(r.calls_owner.empty() ? "$^" : r.calls_owner, flags)});
    return out;
}

// The one verdict: does this line answer the rule's question without calling
// its owner? The self-test and the scan both ask it.
bool flags_line(const CompiledRule& c, const std::string& line) {
    if (!std::regex_search(line, c.pattern)) return false;
    return c.rule->calls_owner.empty() || !std::regex_search(line, c.calls_owner);
}

// A line the scan accepts by its exact text: an owner's own line or a
// baseline copy. Both kinds go in one list and follow one rule.
struct ListedLine {
    std::string question;
    std::string file;       // repo-relative, forward slashes
    std::string line_text;  // trimmed
    std::string stale;      // the message when no source line uses it
};

// Owner lines first, so an owner line is used before a baseline entry with
// the same text.
std::vector<ListedLine> listed_lines() {
    std::vector<ListedLine> out;
    for (const OwnerRule& r : rules())
        for (const OwnerLine& o : r.owner_lines)
            out.push_back({r.question, o.file, o.line_text,
                           "owner line no longer matches (update it)"});
    for (const KnownCopy& k : known_copies())
        out.push_back({k.question, k.file, k.line_text,
                       "baseline entry no longer matches (remove it)"});
    return out;
}

enum class Take { taken, used_up, unlisted };

// Each listed entry covers exactly one line of source: the first entry with
// this question, file and text that no earlier line used takes it. A
// word-for-word copy finds every such entry used (used_up).
Take take_listed_line(const std::vector<ListedLine>& listed, std::vector<bool>& used,
                      const std::string& question, const std::string& file,
                      const std::string& text) {
    bool any = false;
    for (size_t i = 0; i < listed.size(); ++i) {
        const ListedLine& l = listed[i];
        if (l.question != question || l.file != file || l.line_text != text) continue;
        any = true;
        if (!used[i]) {
            used[i] = true;
            return Take::taken;
        }
    }
    return any ? Take::used_up : Take::unlisted;
}

// Whether a file the walk finds is C++ source, the files the C++ scans read.
bool is_cpp_source(const fs::path& path) {
    const fs::path ext = path.extension();
    return ext == ".cpp" || ext == ".h";
}

// Whether a trimmed source line is a comment line, which the scans skip.
bool is_line_comment(std::string_view trimmed) {
    return hydra::starts_with(trimmed, "//");
}

// How a scan that lists lines ends: each entry no source line took is stale,
// and the case fails with one line per problem.
void check_scan(std::vector<std::string> problems, const std::vector<ListedLine>& listed,
                const std::vector<bool>& used) {
    for (size_t i = 0; i < listed.size(); ++i) {
        if (used[i]) continue;
        problems.push_back(listed[i].stale + ": " + listed[i].file + ": " + listed[i].line_text);
    }
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}

}  // namespace

TEST_CASE("single-owner listed lines each cover one line of source") {
    const std::vector<ListedLine> listed = {{"Q?", "src/a.cpp", "x();", "stale"},
                                            {"Q?", "src/a.cpp", "y();", "stale"},
                                            {"Q?", "src/a.cpp", "y();", "stale"}};
    std::vector<bool> used(listed.size(), false);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "x();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "x();") == Take::used_up);
    // Two identical entries cover two identical lines, and no third.
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::taken);
    CHECK(take_listed_line(listed, used, "Q?", "src/a.cpp", "y();") == Take::used_up);
    CHECK(take_listed_line(listed, used, "Q?", "src/b.cpp", "x();") == Take::unlisted);
    CHECK(take_listed_line(listed, used, "Other?", "src/a.cpp", "x();") == Take::unlisted);
}

TEST_CASE("single-owner rules match their own examples") {
    for (const CompiledRule& c : compile_rules()) {
        for (const std::string& line : c.rule->must_match) {
            INFO(c.rule->question << " should flag: " << line);
            CHECK(flags_line(c, line));
        }
        for (const std::string& line : c.rule->must_not_match) {
            INFO(c.rule->question << " should not flag: " << line);
            CHECK_FALSE(flags_line(c, line));
        }
        for (const OwnerLine& o : c.rule->owner_lines) {
            INFO(c.rule->question << " owner line the rule does not flag: " << o.line_text);
            CHECK(flags_line(c, o.line_text));
            CHECK_FALSE(o.why.empty());
        }
    }
    // Every baseline entry names a real rule, and every question is unique.
    std::set<std::string> questions;
    for (const OwnerRule& r : rules()) {
        INFO("question listed twice: " << r.question);
        CHECK(questions.insert(r.question).second);
    }
    for (const KnownCopy& c : known_copies()) {
        INFO("baseline entry names no rule: " << c.question);
        CHECK(questions.count(c.question) == 1);
    }
}

// Does this rule scan the file `rel`, which sits under the top folder `sub`?
bool in_scope(const OwnerRule& r, const std::string& sub, const std::string& rel) {
    if (r.scope.empty()) return sub == "src" || sub == "tools";
    for (const std::string& s : r.scope)
        if (s == sub || s == rel) return true;
    return false;
}

TEST_CASE("single-owner rules hold across src/, tools/ and tests/") {
    const fs::path root = sourcetree::root();
    const std::vector<CompiledRule> compiled = compile_rules();

    const std::vector<ListedLine> listed = listed_lines();
    std::vector<bool> used(listed.size(), false);  // which listed entries a line took
    std::vector<std::string> problems;
    int files = 0;
    std::vector<int> functions_found(compiled.size(), 0);
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            if (!is_cpp_source(path)) return;
            // This file holds every rule's examples as text.
            if (rel == "tests/test_single_owner.cpp") return;
            const std::string sub = rel.substr(0, rel.find('/'));  // the top folder
            ++files;
            std::ifstream in(path);
            std::string line;
            int lineno = 0;
            std::vector<bool> in_function(compiled.size(), false);
            while (std::getline(in, line)) {
                ++lineno;
                const std::string t = hydra::trim(line);
                if (t.empty()) continue;
                const bool comment = is_line_comment(t);
                for (size_t ci = 0; ci < compiled.size(); ++ci) {
                    const CompiledRule& c = compiled[ci];
                    const OwnerRule& rule = *c.rule;
                    if (!in_scope(rule, sub, rel)) continue;
                    if (comment && !rule.scan_comments) continue;
                    if (!rule.function.empty()) {
                        if (rel != rule.function_file) continue;
                        if (!in_function[ci] && line.find(rule.function) != std::string::npos) {
                            in_function[ci] = true;
                            ++functions_found[ci];
                        }
                        if (!in_function[ci]) continue;
                        if (line == "}") in_function[ci] = false;
                    }
                    bool skip = false;
                    for (const std::string& o : rule.owner_files) skip = skip || rel == o;
                    for (const Exempt& x : rule.exempt) skip = skip || rel == x.file;
                    if (skip || !flags_line(c, line)) continue;
                    const Take take = take_listed_line(listed, used, rule.question, rel, t);
                    if (take == Take::taken) continue;
                    const std::string where = rel + ":" + std::to_string(lineno);
                    if (take == Take::used_up)
                        problems.push_back(where + ": a second copy of a listed line (each "
                                           "entry covers one line) answers \"" +
                                           rule.question + "\": " + t);
                    else
                        problems.push_back(where + ": answers \"" + rule.question +
                                           "\", which belongs to " + rule.owner + ": " + t);
                }
            }
    });
    // A function-scoped rule found its function exactly once; a renamed or
    // moved function would otherwise leave the rule checking nothing.
    for (size_t ci = 0; ci < compiled.size(); ++ci) {
        const OwnerRule& rule = *compiled[ci].rule;
        if (rule.function.empty()) continue;
        INFO(rule.question << ": \"" << rule.function << "\" in " << rule.function_file);
        CHECK(functions_found[ci] == 1);
    }
    // A rule scoped to single files names files that exist; a renamed one
    // would otherwise leave the rule checking nothing.
    for (const CompiledRule& c : compiled) {
        for (const std::string& s : c.rule->scope) {
            if (s.find('/') == std::string::npos) continue;
            INFO(c.rule->question << ": scoped to " << s);
            CHECK(fs::is_regular_file(root / fs::u8path(s)));
        }
    }
    CHECK(files > 100);  // the scan found the sources
    check_scan(problems, listed, used);
}

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, which does not compile and which
// the row scan skips, so this case reads it: the block between the "Results"
// banner and the kResultsStamp line in src/store/stored_versions.h must name
// src/parse next to src/search and src/core. Moved here from
// a stamps test; the repo root comes from tests/source_tree.h.
TEST_CASE("single-owner: the results stamp's bump rule names the chart readers (D23)") {
    std::ifstream in(sourcetree::root() / "src" / "store" / "stored_versions.h");
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();
    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}

// D94: the User Guide quotes the Preview's changed-chart line in backticks.
// The sentence is read off the owner line of the row that guards it, and that
// line is checked against src/ui/preview_tab.cpp, so the words are typed only
// in the code and in that row.
TEST_CASE("single-owner: the User Guide quotes the Preview's changed-chart line as the code has it") {
    const OwnerLine* owner = nullptr;
    for (const OwnerRule& r : rules())
        if (r.question == "What does the Preview say when the chart changed since it was analyzed?")
            for (const OwnerLine& o : r.owner_lines)
                if (o.file == "src/ui/preview_tab.cpp") owner = &o;
    REQUIRE(owner != nullptr);

    const auto slurp = [](const fs::path& p) {
        std::ifstream in(p);
        REQUIRE(in.good());
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    };
    INFO("owner line: " << owner->line_text);
    REQUIRE(slurp(sourcetree::root() / fs::u8path(owner->file)).find(owner->line_text) !=
            std::string::npos);

    // The C++ literal on that line, between its first and last double quote.
    const size_t open = owner->line_text.find('"');
    const size_t close = owner->line_text.rfind('"');
    REQUIRE(open != std::string::npos);
    REQUIRE(close > open);
    const std::string sentence = owner->line_text.substr(open + 1, close - open - 1);
    REQUIRE(sentence.find('\\') == std::string::npos);  // no escapes to undo

    INFO("docs/UserGuide.md should quote `" << sentence << "`");
    CHECK(slurp(sourcetree::root() / "docs" / "UserGuide.md").find("`" + sentence + "`") !=
          std::string::npos);
}

// E3 (findings 180, 243, 245 and 56): "does this path need any timing?" is
// Path::needs_timing's, so no code line under src/ or tools/ asks it with a
// zero test of its own. The all-0 list's 0 ms limit (D85) is set in one
// place, allzero_options, and the shortcut that skips the all-0 pass reads it
// from there. What the Score range's INI int means is
// Settings::search_depth_mode's, the one line in src/app/config.cpp that
// compares it, however the comparison is spelled. Comment lines are skipped
// like the row scan does.
TEST_CASE("single-owner: the all-0 limit and the depth-mode int each have one owner (E3, D85)") {
    const std::regex zero_limit(
        R"(difficulty\(\)\.value_or\(0(\.0+)?\)\s*(<=|>|<|>=)\s*0(\.0+)?(?![\d.]))"
        R"(|value_or\(0(\.0+)?\)\s*<=\s*0(\.0+)?(?![\d.]))"
        R"(|ms_filter\s*=\s*(std::optional<double>\()?0(\.0+)?(?![\d.]))"
        R"(|within_(ms_)?limit\(\s*0(\.0+)?\s*\))");
    const std::regex depth_int(R"(\bdepth_mode\s*(==|!=|>=|<=|>|<)\s*[01]\b|case\s+1\s*:.*depth)");
    std::vector<std::string> zero_hits, depth_hits;
    sourcetree::for_each_source_file([&](const fs::path& file, const std::string& rel) {
        if (rel.compare(0, 6, "tests/") == 0) return;
        if (!is_cpp_source(file)) return;
        std::ifstream in(file);
        std::string line;
        while (std::getline(in, line)) {
            const std::string t = hydra::trim(line);
            if (is_line_comment(t)) continue;
            if (std::regex_search(t, zero_limit)) zero_hits.push_back(rel + ": " + t);
            if (std::regex_search(t, depth_int)) depth_hits.push_back(rel + ": " + t);
        }
    });
    REQUIRE_MESSAGE(zero_hits.size() == 1,
                    (zero_hits.size() < 2 ? std::string() : zero_hits[1]));
    CHECK(zero_hits.front() == "src/search/pather.cpp: options.ms_filter = 0.0;");
    REQUIRE(depth_hits.size() == 1);
    CHECK(depth_hits.front() ==
          "src/app/config.cpp: return depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;");
}

// The walk reads only .cpp and .h files under src, tools and tests, so a row
// scoped to a single file outside them (installer/hydra.iss, Inno Setup's
// script) is checked here, with the same verdict. No listed line covers such
// a file, so any flagged line fails. Comment lines are skipped as the walk
// skips them.
TEST_CASE("single-owner rules hold in the single files outside the walk") {
    for (const CompiledRule& c : compile_rules()) {
        for (const std::string& s : c.rule->scope) {
            const std::string top = s.substr(0, s.find('/'));
            if (s.find('/') == std::string::npos || top == "src" || top == "tools" || top == "tests")
                continue;
            std::ifstream in(sourcetree::root() / fs::u8path(s));
            REQUIRE(in.good());
            std::string line;
            while (std::getline(in, line)) {
                const std::string t = hydra::trim(line);
                if (is_line_comment(t) && !c.rule->scan_comments) continue;
                INFO(s << " answers \"" << c.rule->question << "\", which belongs to "
                       << c.rule->owner << ": " << t);
                CHECK_FALSE(flags_line(c, line));
            }
        }
    }
}

// J3-7 (findings 229, 230, 231, 271, 272 and the probe's unit factor) and
// J3-8 (findings 232 to 236, 238, 239, 273, 328): the Clone Hero probe under
// tools/ is Python, which the row scan above skips (it reads .cpp and .h
// only), so these rows get their own scan of every .py file under tools/. A
// __pycache__ .pyc is never read: only .py files are. Each row lists, by
// exact text, the lines allowed to answer its question. Listed lines follow
// take_listed_line, like the row scan's. Python comment lines are skipped.
TEST_CASE("single-owner: the Clone Hero probe's facts each have one owner (J3-7, J3-8)") {
    struct ProbeRow {
        std::string question;
        std::string pattern;
        std::vector<std::string> allowed;  // "rel: trimmed line": the lines allowed to answer it
        std::vector<std::string> must_match;
        std::vector<std::string> must_not_match;
    };
    const std::vector<ProbeRow> rows = {
        {"Where does the probe skip GameAssembly's own copies of the window constants?",
         R"x(\b0x4000000\b)x",
         {"tools/ch_probe/engine_finder.py: MODULE_SPAN = 0x4000000"},
         {"module_end = proc.module_base + 0x4000000"},
         {"MODULE_SPAN = 0x40000000"}},
        {"How do raw engine bytes become numbers?",
         R"x(struct\.unpack(_from)?\()x",
         {R"x(tools/ch_probe/process.py: return struct.unpack("<d", raw)[0])x",
          R"x(tools/ch_probe/process.py: return struct.unpack("<I", raw)[0])x",
          R"x(tools/ch_probe/process.py: return struct.unpack("<Q", raw)[0])x"},
         {R"x(return struct.unpack_from("<d", raw, off)[0])x",
          R"x(return struct.unpack("<d", bytes(xmm0_bytes[:8]))[0])x"},
         {R"x(struct.pack_into("<d", block, C.OFF_SONG_CLOCK, 12.5))x"}},
        {"Is the engine in precision mode?",
         R"x(&\s*C\.PRECISION_MODE_BIT\)|flags\s*&\s*C\.PRECISION_MODE_BIT)x",
         {"tools/ch_probe/engine.py: return (flags & C.PRECISION_MODE_BIT) != 0"},
         {"return (flags & C.PRECISION_MODE_BIT) != 0",
          "return bool(self.flags & C.PRECISION_MODE_BIT)"},
         {"other_bits = 0xFFFFFFFF & ~C.PRECISION_MODE_BIT"}},
        {"How does the probe read and write game memory?",
         R"x(k32\.(Read|Write)ProcessMemory\()x",
         {"tools/ch_probe/process.py: ok = k32.ReadProcessMemory(",
          "tools/ch_probe/process.py: ok = k32.WriteProcessMemory("},
         {"ok = self._win32.k32.ReadProcessMemory(",
          "ok = self._win32.k32.WriteProcessMemory("},
         {"k32.FlushInstructionCache.restype = wintypes.BOOL"}},
        // The owner is constants.s_to_ms/ms_to_s, which multiply by MS_PER_S
        // and so match nothing here. The bare-name alternative catches a
        // variable called just ms or s with an optional subscript.
        {"How many milliseconds is a second (probe)?",
         R"x(\b\w*(_s|_S|raw|est|clock|back|front)\s*\*\s*1000(\.0)?(?![\d.]))x"
         R"x(|\b\w*(_ms|_MS)\s*/\s*1000(\.0)?(?![\d.])|\)\s*[*/]\s*1000(\.0)?(?![\d.]))x"
         R"x(|\b(ms\[\d+\]|s(\[\d+\])?)\s*[*/]\s*1000(\.0)?(?![\d.]))x",
         {},
         {"window_ms=dbl(C.OFF_TOTAL_WINDOW) * 1000.0,",
          "self._pending = (self._spacing_ms, thread_context.xmm0_double() * 1000.0)",
          "return ms[0] / 1000",
          "ms[0] += round(s * 1000)"},
         {"bpm_microbeats = int(round(bpm * 1000))",
          R"x(f"  0 = B {int(BPM * 1000)}",)x",
          R"x("-i", "anullsrc=r=44100:cl=stereo",)x"}},
        {"Which keys does EngineModel.constants() use?",
         R"x("(normal|precision)_(back|front)"|"hitcheck_threshold"|"(normal|precision)_")x",
         {R"x(tools/ch_probe/constants.py: CONST_KEY_PREFIX_NORMAL = "normal_")x",
          R"x(tools/ch_probe/constants.py: CONST_KEY_PREFIX_PRECISION = "precision_")x",
          R"x(tools/ch_probe/constants.py: CONST_KEY_HITCHECK_THRESHOLD = "hitcheck_threshold")x"},
         {R"x("normal_back": read(C.RVA_CONST_NORMAL_BACK),)x"},
         {"C.CONST_KEY_NORMAL_BACK: read(C.RVA_CONST_NORMAL_BACK),"}},
        // The owner is Process.resolve; the test fakes call it (tests/fakes.py).
        {"How does a test fake turn an RVA into an address?",
         R"x(return\s+(self\.)?(module_base|BASE)\s*\+\s*rva)x",
         {"tools/ch_probe/process.py: return self.module_base + rva"},
         {"return BASE + rva"},
         {"expected_addr = proc.resolve(C.RVA_DRUMS_ENGINE_CTOR)"}},
        {"How long does a runner wait before reading a press's result?",
         R"x(\b\w*SETTLE_MS\s*=(?!=))x",
         {"tools/ch_probe/constants.py: INPUT_SETTLE_MS = 250"},
         {"SETTLE_MS = 250        # wait this long past the note before reading the result",
          "SETTLE_MS = 250   # read the result this long after the note (as walk_edges.py)"},
         {"wait_for(max(p.second_ms, p.second_ms + p.offset_ms) + constants.INPUT_SETTLE_MS)"}},
        {"Did a press register as a hit?",
         R"x(\b(after|score_after)\s*>\s*(before|score_before)\b|\bafter_score\s*>\s*before_score\b)x"
         R"x(|\.score\s*>\s*\w+\.score\b)x",
         {"tools/ch_probe/engine.py: return score_after > score_before"},
         {"hit = after_score > before_score", "hit = score_after > score_before",
          "measured if from_engine else None, after.score > before.score, measured)",
          "score_rises = sum(1 for a, b in zip(samples, samples[1:]) if b.score > a.score)"},
         {"hit = pressed_input_hit(score_before, score_after)"}},
        {"How does a runner find the game window?",
         R"x(\bFindWindowW\b)x",
         {"tools/ch_probe/input_driver.py: find = ctypes.windll.user32.FindWindowW"},
         {R"x(ch_hwnd = user32.FindWindowW(None, "Clone Hero"))x",
          R"x(return ctypes.windll.user32.FindWindowW(None, "Clone Hero") or 0)x"},
         {"ch_hwnd = find_game_window()"}},
        {"Which .chart note is the probe's kick?",
         R"x(\b\w*KICK\s*=\s*0\b)x",
         {"tools/ch_probe/constants.py: PROBE_CHART_NOTE_KICK = 0"},
         {"KICK = 0  # .chart drum lane 0; play_chart.py maps it to the kick key"},
         {"KICK = 4", "DRUM_NOTE_KICK = C.PROBE_CHART_NOTE_KICK"}},
        // The test pins are today's text, pinned once (test_song_names_are_spelled_once).
        {"Where are the probe songs installed, and what are they called?",
         R"x(Clone Hero\\songs\\Hydra Probe|"Window Map"|"Edge Walk"|"Hydra Probe - ")x",
         {R"x(tools/ch_probe/probe_songs.py: DEFAULT_OUT = r"C:\Clone Hero\songs\Hydra Probe")x",
          R"x(tools/ch_probe/probe_songs.py: WINDOW_MAP = "Window Map")x",
          R"x(tools/ch_probe/probe_songs.py: EDGE_WALK = "Edge Walk")x",
          R"x(tools/ch_probe/probe_songs.py: NAME_PREFIX = "Hydra Probe - ")x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.WINDOW_MAP, "Window Map"))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.EDGE_WALK, "Edge Walk"))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(P.NAME_PREFIX, "Hydra Probe - "))x"},
         {R"x(PROBE_ROOT = r"C:\Clone Hero\songs\Hydra Probe")x",
          R"x(return os.path.join(live.PROBE_ROOT, "Window Map"))x"},
         {R"x(python tools\\ch_probe\\experiments\\watch_window.py [<probe song name> | folder])x"}},
        {"What is the probe's audio file called?",
         R"x("song\.ogg")x",
         {R"x(tools/ch_probe/probe_songs.py: SONG_OGG = "song.ogg")x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertIn('  MusicStream = "song.ogg"\n', text))x",
          R"x(tools/ch_probe/tests/test_probe_songs.py: self.assertEqual(calls, [(os.path.join(root, "x", "song.ogg"), 4000)]))x",
          R"x(tools/ch_probe/tests/test_probe_chart.py: music_stream="song.ogg"))x",
          R"x(tools/ch_probe/tests/test_probe_chart.py: self.assertIn('  MusicStream = "song.ogg"\n', with_stream))x"},
         {R"x(music_stream="song.ogg")x"},
         {R"x(music_stream=SONG_OGG)x"}},
        {"How many ticks is a millisecond (probe)?",
         R"x(/\s*60000(\.0)?\b|\*\s*\w+\s*/\s*60000)x",
         {"tools/ch_probe/probe_chart.py: return resolution * bpm / 60000.0"},
         {"self.assertEqual(P.RESOLUTION * P.BPM / 60000.0, 1.0)"},
         {"return int(round(ms * _ticks_per_ms(resolution, bpm)))"}},
        {"What window does the formula predict?",
         R"x(\*\*\s*exponent\b)x",
         {"tools/ch_probe/experiments/analysis.py: return (t * c1 - (t ** exponent) * c2) * c3"},
         {"return ((t * c1 - (t ** exponent) * c2) * c3 - c4) / divisor",
          "return (c0 - (t * c1 - (t ** exponent) * c2) * c3) / divisor"},
         {"exponent=exponent) - c4) / divisor"}},
        {"Which lines of a probe .chart are drum notes? (probe tests)",
         R"x(N\\s\+\(\\d\+\)\\s\+\(\\d\+\)|= N 0 0\$)x",
         {R"x(tools/ch_probe/tests/chart_reader.py: _NOTE_LINE = re.compile(r"^(\d+)\s*=\s*N\s+(\d+)\s+(\d+)$"))x"},
         {R"x(match = re.match(r"^(\d+)\s*=\s*N\s+(\d+)\s+(\d+)$", stripped))x",
          R"x(return [int(m) for m in re.findall(r"^\s*(\d+) = N 0 0$", text, re.M)])x"},
         {R"x(i_drums = text.find("[ExpertDrums]"))x"}},
        // The test line pins D54's recorded value.
        {"How long is a key held?",
         R"x(\b0\.003\b)x",
         {"tools/ch_probe/input_driver.py: KEY_HOLD_S = 0.003",
          "tools/ch_probe/tests/test_input_driver.py: self.assertEqual(input_driver.KEY_HOLD_S, 0.003)"},
         {"def press_chord(self, lanes: Iterable[int], *, hold_s: float = 0.003,",
          "def press_chord(self, lanes: Sequence[int], *, hold_s: float = 0.003) -> list:"},
         {"def press_chord(self, lanes: Iterable[int], *, hold_s: float = KEY_HOLD_S,"}},
        // The owner is the named constants in constants.py, which the pattern
        // (the old inline cut-offs) does not match.
        {"Which cut-offs judge the probe's verdicts?",
         R"x(<=\s*10\.0\b|fresh_s:\s*float\s*=\s*0\.002|max_fill_s:\s*float\s*=\s*0\.05)x"
         R"x(|tolerance_ms:\s*float\s*=\s*1\.0|decisive_fraction:\s*float\s*=\s*0\.8)x",
         {},
         {"if len(hits) == score_rises and max(abs(d) for d in diffs) <= 10.0:",
          "fresh_s: float = 0.002, max_fill_s: float = 0.05) -> None:",
          "tolerance_ms: float = 1.0,", "decisive_fraction: float = 0.8,"},
         {"clock = SongClock(engine.song_clock, max_fill_s=0.0)",
          "fresh_s: float = C.CLOCK_FRESH_S,"}},
    };

    std::vector<std::regex> compiled;
    for (const ProbeRow& r : rows) {
        compiled.emplace_back(r.pattern);
        for (const std::string& line : r.must_match) {
            INFO(r.question << " should flag: " << line);
            CHECK(std::regex_search(line, compiled.back()));
        }
        for (const std::string& line : r.must_not_match) {
            INFO(r.question << " should not flag: " << line);
            CHECK_FALSE(std::regex_search(line, compiled.back()));
        }
    }

    // The allowed lines count the way the row scan's do: through ListedLine and
    // take_listed_line. A repo-relative path holds no ": ", so the first one
    // splits an entry into its file and its text.
    std::vector<ListedLine> listed;
    for (const ProbeRow& r : rows)
        for (const std::string& a : r.allowed) {
            const size_t split = a.find(": ");
            REQUIRE(split != std::string::npos);
            listed.push_back({r.question, a.substr(0, split), a.substr(split + 2),
                              "listed line is gone (stale)"});
        }
    std::vector<bool> used(listed.size(), false);
    std::vector<std::string> problems;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.compare(0, 6, "tools/") != 0 || path.extension() != ".py") return;
        std::ifstream in(path);
        std::string line;
        int lineno = 0;
        while (std::getline(in, line)) {
            ++lineno;
            const std::string t = hydra::trim(line);
            if (t.empty() || t[0] == '#') continue;
            for (size_t i = 0; i < rows.size(); ++i) {
                if (!std::regex_search(t, compiled[i])) continue;
                const Take take = take_listed_line(listed, used, rows[i].question, rel, t);
                if (take == Take::taken) continue;
                const std::string where = rel + ":" + std::to_string(lineno);
                if (take == Take::used_up)
                    problems.push_back(where + ": a second copy of a listed line (each "
                                       "entry covers one line) answers \"" +
                                       rows[i].question + "\": " + t);
                else
                    problems.push_back(where + ": answers \"" + rows[i].question + "\": " + t);
            }
        }
    });
    check_scan(problems, listed, used);
}

// ---- Copied blocks of code (task MR1, ADR 0025) ----
//
// A block of code pasted into a second place is the plainest way to write a
// rule twice, and a text scan finds every one. This scan reads the .cpp and
// .h files the row scan reads and fails on any run of kCloneWindowLines code
// lines that appears in two places, in two files or twice in one. Overlapping
// runs merge into one copied block, named once with both places. The
// baseline (known_clones) lists the copies that were already there; like
// known_copies, it only shrinks. Vendored code in third_party/ is outside the
// walk (tests/source_tree.h), so it is never read.

namespace {

// How many code lines in a row count as a copy. The user's decision is D82
// item 1 in docs/audit/2026-10-03-fix-decisions.md; ADR 0025 says why.
constexpr int kCloneWindowLines = 8;

// One line the clone scan compares, and where it sits in its file.
struct CodeLine {
    std::string text;  // as clone_line_text gives it
    int lineno;        // 1-based, in the file as written
};

// A file as the clone scan sees it: only the lines that count.
struct CodeFile {
    std::string file;  // repo-relative, forward slashes
    std::vector<CodeLine> lines;
};

// A letter, digit or underscore, or any byte of a UTF-8 character. A line
// with none of these is only braces and punctuation.
bool is_word_byte(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') ||
           u == '_' || u >= 0x80;
}

// The text the clone scan compares for one line of source, or empty when the
// line doesn't count: a blank line, a comment-only line, an #include, or a
// line of only braces and punctuation. Ends are trimmed and every inner run of
// whitespace becomes one space. A comment line is one is_line_comment names.
// The clone scan also skips /* */ comments, which the row scans read as code,
// because comment text pasted twice is not a rule written twice; in_comment
// carries such a comment from one line to the next.
std::string clone_line_text(std::string_view line, bool& in_comment) {
    std::string_view t = hydra::trim_view(line);
    if (in_comment) {
        const size_t end = t.find("*/");
        if (end == std::string_view::npos) return {};
        in_comment = false;
        t = hydra::trim_view(t.substr(end + 2));
    }
    if (hydra::starts_with(t, "/*")) {
        const size_t end = t.find("*/", 2);
        if (end == std::string_view::npos) {
            in_comment = true;
            return {};
        }
        t = hydra::trim_view(t.substr(end + 2));
    }
    if (is_line_comment(t) || hydra::starts_with(t, "#include")) return {};
    if (std::none_of(t.begin(), t.end(), is_word_byte)) return {};
    std::string out;
    bool gap = false;
    for (const char c : t) {
        if (hydra::is_ascii_space(c)) {
            gap = true;
            continue;
        }
        if (gap) out += ' ';
        gap = false;
        out += c;
    }
    return out;
}

CodeFile read_code_lines(const std::string& rel, std::istream& in) {
    CodeFile f{rel, {}};
    std::string line;
    int lineno = 0;
    bool in_comment = false;
    while (std::getline(in, line)) {
        ++lineno;
        std::string t = clone_line_text(line, in_comment);
        if (!t.empty()) f.lines.push_back({std::move(t), lineno});
    }
    return f;
}

// One place a copied block sits.
struct CloneSide {
    std::string file;
    int first_line;  // as written, first and last code line of the block
    int last_line;
};

// A block of code found in two places. `a` comes before `b` in the files'
// order, or earlier in the file when a file repeats itself.
struct CloneRegion {
    CloneSide a, b;
    int lines;          // code lines in the block, as clone_line_text counts them
    std::string first;  // the block's first code line
};

// Every block of at least `window` code lines that appears in two places in
// `files`. A block found in three places is three pairs. Two runs in one file
// that overlap each other are not a copy.
std::vector<CloneRegion> find_clones(const std::vector<CodeFile>& files, int window) {
    // Each distinct line becomes a number, so a run's key is its numbers'
    // bytes and the map's own hash does the rest.
    std::unordered_map<std::string, int> ids;
    std::vector<std::vector<int>> coded(files.size());
    for (size_t f = 0; f < files.size(); ++f)
        for (const CodeLine& l : files[f].lines)
            coded[f].push_back(ids.emplace(l.text, static_cast<int>(ids.size())).first->second);
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> runs;  // key -> (file, start)
    for (size_t f = 0; f < files.size(); ++f) {
        const std::vector<int>& c = coded[f];
        for (size_t i = 0; i + static_cast<size_t>(window) <= c.size(); ++i) {
            const std::string key(reinterpret_cast<const char*>(c.data() + i),
                                  sizeof(int) * static_cast<size_t>(window));
            runs[key].push_back({static_cast<int>(f), static_cast<int>(i)});
        }
    }
    // Each matching pair of runs, the earlier place first. Runs went in by
    // file, then by start, so each list is already in that order.
    struct Pair {
        int fa, ia, fb, ib;
    };
    std::vector<Pair> pairs;
    for (const auto& [key, at] : runs)
        for (size_t x = 0; x < at.size(); ++x)
            for (size_t y = x + 1; y < at.size(); ++y) {
                const auto [fa, ia] = at[x];
                const auto [fb, ib] = at[y];
                if (fa == fb && ib < ia + window) continue;
                pairs.push_back({fa, ia, fb, ib});
            }
    // Pairs of runs that each step one line further in both places are one
    // block: sort them so those follow each other.
    const auto order = [](const Pair& p) { return std::make_tuple(p.fa, p.fb, p.ib - p.ia, p.ia); };
    std::sort(pairs.begin(), pairs.end(),
              [&](const Pair& x, const Pair& y) { return order(x) < order(y); });
    std::vector<CloneRegion> out;
    for (size_t s = 0; s < pairs.size();) {
        size_t e = s;
        while (e + 1 < pairs.size() && pairs[e + 1].fa == pairs[s].fa &&
               pairs[e + 1].fb == pairs[s].fb &&
               pairs[e + 1].ib - pairs[e + 1].ia == pairs[s].ib - pairs[s].ia &&
               pairs[e + 1].ia == pairs[e].ia + 1)
            ++e;
        const Pair& p = pairs[s];
        const int last = pairs[e].ia - p.ia + window - 1;  // the block's last line, from its start
        const std::vector<CodeLine>& la = files[p.fa].lines;
        const std::vector<CodeLine>& lb = files[p.fb].lines;
        out.push_back({{files[p.fa].file, la[p.ia].lineno, la[p.ia + last].lineno},
                       {files[p.fb].file, lb[p.ib].lineno, lb[p.ib + last].lineno},
                       last + 1,
                       la[p.ia].text});
        s = e + 1;
    }
    return out;
}

// The question every clone entry answers, so the baseline goes through
// take_listed_line like every other listed line.
const char* const kCloneQuestion = "Is this block of code written in two places?";

ListedLine clone_listed_line(const std::string& file_a, const std::string& file_b, int lines,
                             const std::string& first) {
    return {kCloneQuestion, file_a + " and " + file_b,
            std::to_string(lines) + " lines from: " + first,
            "baseline clone no longer found (remove it)"};
}

// Every .cpp and .h the walk finds, in sorted path order.
std::vector<CodeFile> read_source_code() {
    std::vector<CodeFile> files;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (!is_cpp_source(path)) return;
        std::ifstream in(path);
        files.push_back(read_code_lines(rel, in));
    });
    std::sort(files.begin(), files.end(),
              [](const CodeFile& x, const CodeFile& y) { return x.file < y.file; });
    return files;
}

}  // namespace

TEST_CASE("single-owner: the clone scan matches its own examples (MR1)") {
    bool in_comment = false;
    CHECK(clone_line_text("  int  a =\t1;  ", in_comment) == "int a = 1;");
    CHECK(clone_line_text("   ", in_comment).empty());
    CHECK(clone_line_text("   // a note", in_comment).empty());
    CHECK(clone_line_text("#include <vector>", in_comment).empty());
    CHECK(clone_line_text("});", in_comment).empty());
    CHECK(clone_line_text("x = 0;  // kept", in_comment) == "x = 0; // kept");
    CHECK(clone_line_text("/* opens", in_comment).empty());
    CHECK(in_comment);
    CHECK(clone_line_text("int inside = 1;", in_comment).empty());
    CHECK(clone_line_text("closes */ int after = 2;", in_comment) == "int after = 2;");
    CHECK_FALSE(in_comment);

    // Made-up files. `block` is ten different code lines; `seven` is its
    // first seven.
    std::string block, seven;
    for (int i = 0; i < 10; ++i) {
        const std::string line = "v" + std::to_string(i) + " = f(" + std::to_string(i) + ");\n";
        block += line;
        if (i < 7) seven += line;
    }
    const auto file = [](const std::string& rel, const std::string& text) {
        std::istringstream in(text);
        return read_code_lines(rel, in);
    };

    // Ten shared lines are one block, named once with both places. Blank
    // lines, comments and braces around one copy don't hide it.
    std::vector<CloneRegion> r = find_clones(
        {file("src/a.cpp", "only_a();\n" + block),
         file("src/b.cpp", "{\n\n// note\n" + block + "}\n")},
        8);
    REQUIRE(r.size() == 1);
    CHECK(r[0].lines == 10);
    CHECK(r[0].first == "v0 = f(0);");
    CHECK(r[0].a.file == "src/a.cpp");
    CHECK(r[0].a.first_line == 2);
    CHECK(r[0].a.last_line == 11);
    CHECK(r[0].b.file == "src/b.cpp");
    CHECK(r[0].b.first_line == 4);
    CHECK(r[0].b.last_line == 13);

    // Seven shared lines are shorter than a window of 8.
    CHECK(find_clones({file("src/a.cpp", seven + "x();\n"), file("src/b.cpp", "y();\n" + seven)}, 8)
              .empty());

    // A file that repeats its own block is a copy too.
    r = find_clones({file("src/a.cpp", block + "mid();\n" + block)}, 8);
    REQUIRE(r.size() == 1);
    CHECK(r[0].a.file == r[0].b.file);
    CHECK(r[0].a.first_line == 1);
    CHECK(r[0].b.first_line == 12);

    // Nine copies of one line overlap themselves: no copy.
    std::string same;
    for (int i = 0; i < 9; ++i) same += "x++;\n";
    CHECK(find_clones({file("src/a.cpp", same)}, 8).empty());

    // A block in three places is three pairs.
    CHECK(find_clones({file("src/a.cpp", block), file("src/b.cpp", block), file("src/c.cpp", block)}, 8)
              .size() == 3);
}

TEST_CASE("single-owner: no block of code is written in two places (MR1)") {
    const std::vector<CodeFile> files = read_source_code();
    CHECK(files.size() > 100);  // the scan found the sources
    std::vector<ListedLine> listed;
    for (const KnownClone& k : known_clones())
        listed.push_back(clone_listed_line(k.file_a, k.file_b, k.lines, k.first));
    std::vector<bool> used(listed.size(), false);
    std::vector<std::string> problems;
    for (const CloneRegion& c : find_clones(files, kCloneWindowLines)) {
        const ListedLine l = clone_listed_line(c.a.file, c.b.file, c.lines, c.first);
        if (take_listed_line(listed, used, l.question, l.file, l.line_text) == Take::taken)
            continue;
        problems.push_back(c.a.file + ":" + std::to_string(c.a.first_line) + "-" +
                           std::to_string(c.a.last_line) + " and " + c.b.file + ":" +
                           std::to_string(c.b.first_line) + "-" + std::to_string(c.b.last_line) +
                           " hold the same " + std::to_string(c.lines) +
                           " code lines, from: " + c.first +
                           " (move them into one place both can call)");
    }
    check_scan(problems, listed, used);
}

TEST_CASE("single-owner: every row's owner is still where the row says") {
    CHECK(rules().size() > 100);  // the loader found the rule files
    std::ostringstream gone;
    for (const OwnerRule& r : rules()) {
        const std::string message = owner_gone(r, sourcetree::root());
        if (!message.empty()) gone << message << "\n";
    }
    INFO(gone.str());
    CHECK(gone.str().empty());
}

TEST_CASE("single-owner: the owner-gone guard and the loader fail on a made-up row") {
    OwnerRule r;
    r.question = "Is this a made-up question?";
    r.rules_file = "rules/made_up.json";
    r.owner_file = "src/core/winstr.cpp";
    r.owner_symbol = "fits_shell";
    CHECK(owner_gone(r, sourcetree::root()).empty());
    r.owner_symbol = "fits_she";  // part of a word is not the word
    CHECK(owner_gone(r, sourcetree::root()) ==
          "rules/made_up.json: \"Is this a made-up question?\": owner gone: name the new owner "
          "or retire the row (src/core/winstr.cpp, fits_she)");
    r.owner_symbol = "no_such_owner_symbol";
    CHECK(owner_gone(r, sourcetree::root()) ==
          "rules/made_up.json: \"Is this a made-up question?\": owner gone: name the new owner "
          "or retire the row (src/core/winstr.cpp, no_such_owner_symbol)");
    r.owner_symbol = "fits_shell";
    r.owner_file = "src/core/no_such_file.cpp";
    CHECK_FALSE(owner_gone(r, sourcetree::root()).empty());

    json row = json::parse(R"({"question": "Is this a made-up question?", "owner": "x in y",
        "owner_file": "src/core/winstr.cpp", "owner_symbol": "fits_shell", "date": "2026-10-10",
        "decision": "D0", "added_by": "so-t1b", "pattern": "x"})");
    CHECK(parse_rule(row, "rules/made_up.json", 0).question == "Is this a made-up question?");
    json missing = row;
    missing.erase("date");
    CHECK_THROWS_WITH(parse_rule(missing, "rules/made_up.json", 0),
                      "rules/made_up.json: \"Is this a made-up question?\": missing field \"date\"");
    json typo = row;
    typo["must_macth"] = json::array();
    CHECK_THROWS_WITH(parse_rule(typo, "rules/made_up.json", 0),
                      "rules/made_up.json: \"Is this a made-up question?\": unknown field \"must_macth\"");
    json no_question = row;
    no_question.erase("question");
    CHECK_THROWS_WITH(parse_rule(no_question, "rules/made_up.json", 2),
                      "rules/made_up.json: row 3: missing field \"question\"");
}
