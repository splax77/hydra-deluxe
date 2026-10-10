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
// from tests/source_tree.h. The clone scan in clones.cpp (ADR 0025) reads the
// same walk for blocks of code pasted into two places.
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../source_tree.h"
#include "core/strutil.h"
#include "json.hpp"
#include "listed.h"
#include "rows.h"

namespace fs = std::filesystem;
using namespace single_owner;

namespace {

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
                    "scan_comments", "note", "language"},
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
        r.language = optional_string(row, "language");
        if (!r.language.empty() && r.language != "py")
            throw std::runtime_error(where + ": unknown language \"" + r.language + "\"");
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

}  // namespace

namespace single_owner {

const std::vector<OwnerRule>& rules() { return tables().rules; }
const std::vector<KnownCopy>& known_copies() { return tables().copies; }
const std::vector<KnownClone>& known_clones() { return tables().clones; }

}  // namespace single_owner

namespace {

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
            // A Python row ("language": "py") reads .py files only, so a
            // __pycache__ .pyc is never read; every other row reads C++.
            const bool py = path.extension() == ".py";
            if (!is_cpp_source(path) && !py) return;
            // These files hold made-up rows and their examples as text.
            if (rel.compare(0, 19, "tests/single_owner/") == 0) return;
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
                const bool comment = py ? t[0] == '#' : is_line_comment(t);
                for (size_t ci = 0; ci < compiled.size(); ++ci) {
                    const CompiledRule& c = compiled[ci];
                    const OwnerRule& rule = *c.rule;
                    if ((rule.language == "py") != py) continue;
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
    json language = row;
    language["language"] = "rb";
    CHECK_THROWS_WITH(parse_rule(language, "rules/made_up.json", 0),
                      "rules/made_up.json: \"Is this a made-up question?\": unknown language \"rb\"");
    json no_question = row;
    no_question.erase("question");
    CHECK_THROWS_WITH(parse_rule(no_question, "rules/made_up.json", 2),
                      "rules/made_up.json: row 3: missing field \"question\"");
}
