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
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../code_lexer.h"
#include "../source_tree.h"
#include "core/strutil.h"
#include "core/winstr.h"
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
                    "scan_comments", "note", "language", "anchor"},
                   where);
        r.owner = required_string(row, "owner", where);
        r.owner_file = required_string(row, "owner_file", where);
        r.owner_symbol = required_string(row, "owner_symbol", where);
        r.date = required_string(row, "date", where);
        r.decision = required_string(row, "decision", where);
        r.added_by = required_string(row, "added_by", where);
        r.pattern = required_string(row, "pattern", where);
        r.anchor = optional_string(row, "anchor");
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

// How many days after its date a baseline entry may stay at most. The user's
// cap, from "Baselines that shrink" in
// docs/superpowers/plans/2026-10-10-single-owner-redesign.md.
constexpr int kBaselineMaxDays = 14;

bool is_iso_shape(const std::string& s) {
    if (s.size() != 10) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (i == 4 || i == 7 ? s[i] != '-' : !std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    return true;
}

// `iso` (YYYY-MM-DD) moved `days` later, as YYYY-MM-DD, or empty when `iso`
// is not in that form. A date that does not exist (2026-02-30) comes back as
// a different day, so iso_plus_days(s, 0) == s says s is a real date.
std::string iso_plus_days(const std::string& iso, int days) {
    if (!is_iso_shape(iso)) return {};
    std::tm t{};
    t.tm_year = std::stoi(iso.substr(0, 4)) - 1900;
    t.tm_mon = std::stoi(iso.substr(5, 2)) - 1;
    t.tm_mday = std::stoi(iso.substr(8, 2)) + days;
    t.tm_hour = 12;  // noon, so a daylight-saving change cannot move the day
    t.tm_isdst = -1;
    if (std::mktime(&t) == static_cast<std::time_t>(-1)) return {};
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &t);
    return buf;
}

// Today on this machine's calendar, as YYYY-MM-DD.
std::string today_iso() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &local);
    return buf;
}

BaselineDates parse_dates(const json& entry, const std::string& where) {
    BaselineDates d{required_string(entry, "date", where), required_string(entry, "added_by", where),
                    required_string(entry, "remove_by", where)};
    for (const std::string& s : {d.date, d.remove_by})
        if (iso_plus_days(s, 0) != s)
            throw std::runtime_error(where + ": \"" + s + "\" is not a YYYY-MM-DD date");
    const std::string cap = iso_plus_days(d.date, kBaselineMaxDays);
    if (d.remove_by > cap)
        throw std::runtime_error(where + ": remove_by " + d.remove_by + " is more than " +
                                 std::to_string(kBaselineMaxDays) + " days after its date " + d.date +
                                 " (" + cap + " at most)");
    return d;
}

std::string copy_entry_name(const KnownCopy& k) {
    return "known_copies \"" + k.question + "\" in " + k.file + ": " + k.line_text;
}

std::string clone_entry_name(const KnownClone& k) {
    return "known_clones " + k.file_a + " and " + k.file_b + ", " + std::to_string(k.lines) +
           " lines from: " + k.first;
}

// Empty while `today` (YYYY-MM-DD) is on or before the entry's remove_by,
// else the failure. ISO dates compare as plain text.
std::string baseline_due(const std::string& entry, const BaselineDates& d, const std::string& today) {
    if (today <= d.remove_by) return {};
    return "baseline.json: " + entry + ": past its remove_by date " + d.remove_by + " (added by " +
           d.added_by + " on " + d.date + "): fix the copy and delete the entry";
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
                check_keys(k,
                           {"question", "file", "line_text", "removed_by", "date", "added_by",
                            "remove_by"},
                           w);
                KnownCopy c{required_string(k, "question", w), required_string(k, "file", w),
                            required_string(k, "line_text", w), required_string(k, "removed_by", w),
                            {}};
                c.dates = parse_dates(k, name + ": " + copy_entry_name(c));
                t.copies.push_back(c);
            }
        if (doc.contains("known_clones"))
            for (const json& k : doc.at("known_clones")) {
                const std::string w = name + ": known_clones";
                check_keys(k, {"file_a", "file_b", "lines", "first", "date", "added_by", "remove_by"},
                           w);
                if (!k.contains("lines")) throw std::runtime_error(w + ": missing field \"lines\"");
                KnownClone c{required_string(k, "file_a", w), required_string(k, "file_b", w),
                             k.at("lines").get<int>(), required_string(k, "first", w), {}};
                c.dates = parse_dates(k, name + ": " + clone_entry_name(c));
                t.clones.push_back(c);
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

// The language codelex reads a file in (codelex::language_of_extension), or
// C++ for a file it does not know.
codelex::Lang lexer_language(const fs::path& path) {
    return codelex::language_of_extension(path.extension().string()).value_or(codelex::Lang::Cpp);
}

// Whether a trimmed line is all comment in `lang`, as codelex splits it.
bool whole_line_comment(const std::string& trimmed, codelex::Lang lang) {
    const std::vector<codelex::Piece> pieces = codelex::split(trimmed, lang);
    return pieces.size() == 1 && pieces[0].kind == codelex::Kind::Comment;
}

// Whether a trimmed line is a comment line the row scans skip, in the language
// the file is read in. Both row scans ask this: C++ keeps is_line_comment's
// test, every other language is split by codelex.
bool skipped_comment_line(const std::string& trimmed, codelex::Lang lang) {
    return lang == codelex::Lang::Cpp ? is_line_comment(trimmed) : whole_line_comment(trimmed, lang);
}

// Whether `text` names `symbol` as a whole word outside its comments, as
// codelex splits them. A string literal counts as naming it.
bool names_outside_comments(const std::string& text, const std::string& symbol, codelex::Lang lang) {
    if (symbol.empty()) return false;
    for (const codelex::Piece& p : codelex::split(text, lang)) {
        if (p.kind == codelex::Kind::Comment) continue;
        for (size_t at = text.find(symbol, p.begin); at != std::string::npos && at + symbol.size() <= p.end;
             at = text.find(symbol, at + 1)) {
            const size_t end = at + symbol.size();
            if ((at == 0 || !is_word_byte(text[at - 1])) &&
                (end == text.size() || !is_word_byte(text[end])))
                return true;
        }
    }
    return false;
}

// The owner-gone guard: empty when the row's owner_file exists under `root`
// and names owner_symbol (names_outside_comments), else the failure message.
std::string owner_gone(const OwnerRule& r, const fs::path& root) {
    const std::string gone = r.rules_file + ": \"" + r.question +
                             "\": owner gone: name the new owner or retire the row (" +
                             r.owner_file + ", " + r.owner_symbol + ")";
    const fs::path path = root / fs::u8path(r.owner_file);
    if (!fs::is_regular_file(path)) return gone;
    return names_outside_comments(hydra::read_file_text(path.u8string()), r.owner_symbol,
                                  lexer_language(path))
               ? std::string()
               : gone;
}

struct CompiledRule {
    const OwnerRule* rule;
    std::regex pattern;
    std::regex calls_owner;
};

CompiledRule compile_rule(const OwnerRule& r) {
    const auto flags = std::regex::ECMAScript | std::regex::optimize;
    return {&r, std::regex(r.pattern, flags),
            std::regex(r.calls_owner.empty() ? "$^" : r.calls_owner, flags)};
}

std::vector<CompiledRule> compile_rules() {
    std::vector<CompiledRule> out;
    for (const OwnerRule& r : rules()) out.push_back(compile_rule(r));
    return out;
}

// The one verdict: does this line answer the rule's question without calling
// its owner? The self-test and the scan both ask it.
bool flags_line(const CompiledRule& c, const std::string& line) {
    if (!c.rule->anchor.empty() && line.find(c.rule->anchor) == std::string::npos) return false;
    if (!std::regex_search(line, c.pattern)) return false;
    return c.rule->calls_owner.empty() || !std::regex_search(line, c.calls_owner);
}

// The anchor guard: one message per must_match line or owner line that lacks
// the row's anchor. flags_line skips such a line before the regex runs, so an
// anchor that misses one of the row's own examples could hide a real match.
std::vector<std::string> anchor_misses(const OwnerRule& r) {
    std::vector<std::string> out;
    if (r.anchor.empty()) return out;
    std::vector<std::string> lines = r.must_match;
    for (const OwnerLine& o : r.owner_lines) lines.push_back(o.line_text);
    for (const std::string& line : lines)
        if (line.find(r.anchor) == std::string::npos)
            out.push_back(r.rules_file + ": \"" + r.question + "\": anchor \"" + r.anchor +
                          "\" is not in its own example: " + line);
    return out;
}

// Owner lines first, so an owner line is used before a baseline entry with
// the same text.
std::vector<ListedLine> listed_lines() {
    std::vector<ListedLine> out;
    for (const OwnerRule& r : rules())
        for (const OwnerLine& o : r.owner_lines)
            out.push_back({r.question, o.file, o.line_text,
                           "owner line no longer matches (update it)", r.rules_file});
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
        for (const std::string& miss : anchor_misses(*c.rule)) {
            INFO(miss);
            CHECK(false);
        }
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

// The language the walk reads the file `rel` in, or nothing when the walk does
// not read it. The walk and the case for the single files outside it both ask
// this, so a file is read by exactly one of them.
std::optional<codelex::Lang> walk_language(const std::string& rel) {
    if (!sourcetree::walk_visits(rel)) return std::nullopt;
    // These files hold made-up rows and their examples as text.
    if (rel.compare(0, 19, "tests/single_owner/") == 0) return std::nullopt;
    // A Python row ("language": "py") reads .py files only, so a __pycache__
    // .pyc is never read; every other row reads C++.
    const fs::path path = fs::u8path(rel);
    if (is_cpp_source(path)) return codelex::Lang::Cpp;
    if (lexer_language(path) == codelex::Lang::Python) return codelex::Lang::Python;
    return std::nullopt;
}

// Whether the walk reads the file `rel` for this row: it reads the file at
// all, and in the row's language.
bool walk_reads_for(const OwnerRule& r, const std::string& rel) {
    const std::optional<codelex::Lang> lang = walk_language(rel);
    return lang && (*lang == codelex::Lang::Python) == (r.language == "py");
}

// The message for line `t` at `where` (file:line) that answers the row's
// question without its owner. Both scans give it.
std::string answers_message(const OwnerRule& rule, const std::string& where, const std::string& t) {
    return where + ": answers \"" + rule.question + "\", which belongs to " + rule.owner + ": " + t +
           rule_suffix(rule.rules_file);
}

// What the row scan carries from one file to the next.
struct ScanState {
    const std::vector<CompiledRule>& compiled;
    const std::vector<ListedLine>& listed;
    std::vector<bool> used;              // which listed entries a line took
    std::vector<std::string> problems;
    std::vector<int> functions_found;    // per compiled row
};

ScanState start_scan(const std::vector<CompiledRule>& compiled, const std::vector<ListedLine>& listed) {
    return {compiled, listed, std::vector<bool>(listed.size(), false), {},
            std::vector<int>(compiled.size(), 0)};
}

// The row scan of one file: `rel` is its repo-relative path, `py` says it is
// Python, `in` reads its text. A flagged line that no listed line covers
// becomes a problem.
void scan_file(ScanState& s, const std::string& rel, bool py, std::istream& in) {
    const std::string sub = rel.substr(0, rel.find('/'));  // the top folder
    std::string line;
    int lineno = 0;
    std::vector<bool> in_function(s.compiled.size(), false);
    while (std::getline(in, line)) {
        ++lineno;
        const std::string t = hydra::trim(line);
        if (t.empty()) continue;
        const bool comment = skipped_comment_line(t, py ? codelex::Lang::Python : codelex::Lang::Cpp);
        for (size_t ci = 0; ci < s.compiled.size(); ++ci) {
            const CompiledRule& c = s.compiled[ci];
            const OwnerRule& rule = *c.rule;
            if ((rule.language == "py") != py) continue;
            if (!in_scope(rule, sub, rel)) continue;
            if (comment && !rule.scan_comments) continue;
            if (!rule.function.empty()) {
                if (rel != rule.function_file) continue;
                if (!in_function[ci] && line.find(rule.function) != std::string::npos) {
                    in_function[ci] = true;
                    ++s.functions_found[ci];
                }
                if (!in_function[ci]) continue;
                if (line == "}") in_function[ci] = false;
            }
            bool skip = false;
            for (const std::string& o : rule.owner_files) skip = skip || rel == o;
            for (const Exempt& x : rule.exempt) skip = skip || rel == x.file;
            if (skip || !flags_line(c, line)) continue;
            const Take take = take_listed_line(s.listed, s.used, rule.question, rel, t);
            if (take == Take::taken) continue;
            const std::string where = rel + ":" + std::to_string(lineno);
            if (take == Take::used_up)
                s.problems.push_back(where + ": a second copy of a listed line (each "
                                     "entry covers one line) answers \"" +
                                     rule.question + "\": " + t + rule_suffix(rule.rules_file));
            else
                s.problems.push_back(answers_message(rule, where, t));
        }
    }
}

TEST_CASE("single-owner rules hold across src/, tools/ and tests/") {
    const fs::path root = sourcetree::root();
    const std::vector<CompiledRule> compiled = compile_rules();
    const std::vector<ListedLine> listed = listed_lines();
    ScanState scan = start_scan(compiled, listed);
    int files = 0;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            const std::optional<codelex::Lang> lang = walk_language(rel);
            if (!lang) return;
            ++files;
            std::ifstream in(path);
            scan_file(scan, rel, *lang == codelex::Lang::Python, in);
    });
    // A function-scoped rule found its function exactly once; a renamed or
    // moved function would otherwise leave the rule checking nothing.
    for (size_t ci = 0; ci < compiled.size(); ++ci) {
        const OwnerRule& rule = *compiled[ci].rule;
        if (rule.function.empty()) continue;
        INFO(rule.question << ": \"" << rule.function << "\" in " << rule.function_file
                           << rule_suffix(rule.rules_file));
        CHECK(scan.functions_found[ci] == 1);
    }
    // A rule scoped to single files names files that exist; a renamed one
    // would otherwise leave the rule checking nothing.
    for (const CompiledRule& c : compiled) {
        for (const std::string& s : c.rule->scope) {
            if (s.find('/') == std::string::npos) continue;
            INFO(c.rule->question << ": scoped to " << s << rule_suffix(c.rule->rules_file));
            CHECK(fs::is_regular_file(root / fs::u8path(s)));
        }
    }
    CHECK(files > 100);  // the scan found the sources
    check_scan(scan.problems, listed, scan.used);
}

TEST_CASE("single-owner: the row scan's messages, pinned on a made-up file") {
    OwnerRule r;
    r.question = "Is this a made-up question?";
    r.owner = "made_up in src/a.cpp";
    r.rules_file = "rules/made_up.json";
    r.pattern = "made_up\\(";
    r.anchor = "made_up(";
    r.owner_lines = {{"src/a.cpp", "return made_up(1);", "the owner"},
                     {"src/a.cpp", "return made_up(9);", "the owner, gone"}};
    const std::vector<CompiledRule> compiled = {compile_rule(r)};
    std::vector<ListedLine> listed;
    for (const OwnerLine& o : r.owner_lines)
        listed.push_back({r.question, o.file, o.line_text, "owner line no longer matches (update it)",
                          r.rules_file});
    ScanState scan = start_scan(compiled, listed);
    std::istringstream in("int f() {\n  return made_up(1);\n}\n// made_up(2) in a comment\n"
                          "x = made_up(3);\n  return made_up(1);\n");
    scan_file(scan, "src/a.cpp", false, in);
    REQUIRE(scan.problems.size() == 2);
    CHECK(scan.problems[0] ==
          "src/a.cpp:5: answers \"Is this a made-up question?\", which belongs to made_up in "
          "src/a.cpp: x = made_up(3); (rule: rules/made_up.json)");
    CHECK(scan.problems[1] ==
          "src/a.cpp:6: a second copy of a listed line (each entry covers one line) answers \"Is "
          "this a made-up question?\": return made_up(1); (rule: rules/made_up.json)");
    CHECK_FALSE(scan.used[1]);
    CHECK(stale_message(listed[1]) ==
          "owner line no longer matches (update it): src/a.cpp: return made_up(9); (rule: "
          "rules/made_up.json)");
}

namespace {

// The row scan of one single file the walk does not read for this row: `rel`
// names it, and its extension picks the language its comments are read in
// (lexer_language); `in` reads its text. No listed line covers such a file,
// so each flagged line is a problem. Comment lines are skipped as the walk
// skips them, unless the row sets scan_comments.
void scan_single_file(const CompiledRule& c, const std::string& rel, std::istream& in,
                      std::vector<std::string>& problems) {
    const codelex::Lang lang = lexer_language(fs::u8path(rel));
    std::string line;
    int lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        const std::string t = hydra::trim(line);
        if (t.empty()) continue;
        if (skipped_comment_line(t, lang) && !c.rule->scan_comments) continue;
        if (flags_line(c, line))
            problems.push_back(answers_message(*c.rule, rel + ":" + std::to_string(lineno), t));
    }
}

}  // namespace

// A row scoped to a single file the walk does not read for it (a workflow, the
// installer script, a .ps1 under tools/) is checked here, with the same
// verdict.
TEST_CASE("single-owner rules hold in the single files outside the walk") {
    std::vector<std::string> problems;
    for (const CompiledRule& c : compile_rules()) {
        for (const std::string& s : c.rule->scope) {
            if (s.find('/') == std::string::npos || walk_reads_for(*c.rule, s)) continue;
            std::ifstream in(sourcetree::root() / fs::u8path(s));
            INFO(c.rule->question << ": scoped to " << s << rule_suffix(c.rule->rules_file));
            REQUIRE(in.good());
            scan_single_file(c, s, in, problems);
        }
    }
    std::ostringstream report;
    for (const std::string& p : problems) report << p << "\n";
    INFO(report.str());
    CHECK(problems.empty());
}

TEST_CASE("single-owner: a row scoped to a file the walk does not read is read here") {
    OwnerRule r;
    r.question = "Is this a made-up question?";
    r.owner = "made_up in src/a.cpp";
    r.rules_file = "rules/made_up.json";
    r.pattern = "made_up\\(";
    r.anchor = "made_up(";
    // The walk reads C++ and Python under its folders, each for rows in that
    // language, and nothing else; the single-files case reads the rest.
    CHECK(walk_reads_for(r, "src/core/winstr.cpp"));
    CHECK_FALSE(walk_reads_for(r, "tools/fuzz_seeds.ps1"));
    CHECK_FALSE(walk_reads_for(r, "tools/made_up.py"));
    CHECK_FALSE(walk_reads_for(r, ".github/workflows/ci.yml"));
    CHECK_FALSE(walk_reads_for(r, "tests/single_owner/scan.cpp"));
    r.language = "py";
    CHECK(walk_reads_for(r, "tools/made_up.py"));
    CHECK_FALSE(walk_reads_for(r, "src/core/winstr.cpp"));
    r.language.clear();

    // A .ps1 is read with PowerShell's comments and a .yml with YAML's: only
    // the code lines are flagged, unless the row reads comments too.
    const std::string ps1 = "# made_up(1)\n<# made_up(2) #>\n$x = made_up(3)  # trailing\n";
    const std::string yml = "# made_up(1)\n  run: made_up(2)\n";
    std::vector<std::string> problems;
    std::istringstream ps1_in(ps1), yml_in(yml);
    scan_single_file(compile_rule(r), "tools/made_up.ps1", ps1_in, problems);
    scan_single_file(compile_rule(r), ".github/workflows/made_up.yml", yml_in, problems);
    CHECK(problems == std::vector<std::string>{
                          "tools/made_up.ps1:3: answers \"Is this a made-up question?\", which "
                          "belongs to made_up in src/a.cpp: $x = made_up(3)  # trailing (rule: "
                          "rules/made_up.json)",
                          ".github/workflows/made_up.yml:2: answers \"Is this a made-up "
                          "question?\", which belongs to made_up in src/a.cpp: run: made_up(2) "
                          "(rule: rules/made_up.json)"});
    // C++ (and a file codelex does not know) skips "//" lines only; the other
    // languages skip what codelex calls a comment.
    CHECK(skipped_comment_line("// x", codelex::Lang::Cpp));
    CHECK_FALSE(skipped_comment_line("/* x */", codelex::Lang::Cpp));
    CHECK(skipped_comment_line("# x", codelex::Lang::PowerShell));
    CHECK_FALSE(skipped_comment_line("x # y", codelex::Lang::Yaml));
    r.scan_comments = true;
    problems.clear();
    std::istringstream ps1_again(ps1);
    scan_single_file(compile_rule(r), "tools/made_up.ps1", ps1_again, problems);
    CHECK(problems.size() == 3);
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
    // A name only in a comment, trailing or whole-line, is not the owner.
    const codelex::Lang cpp = codelex::Lang::Cpp, py = codelex::Lang::Python;
    CHECK(names_outside_comments("bool fits_shell(int n);\n", "fits_shell", cpp));
    CHECK_FALSE(names_outside_comments("int n = 1;  // fits_shell\n", "fits_shell", cpp));
    CHECK_FALSE(names_outside_comments("/* fits_shell */ int n;\n", "fits_shell", cpp));
    CHECK(names_outside_comments("def fits_shell(n):\n", "fits_shell", py));
    CHECK_FALSE(names_outside_comments("n = 1  # fits_shell\n", "fits_shell", py));
    CHECK_FALSE(names_outside_comments("int fits_shell_x;\n", "fits_shell", cpp));

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

TEST_CASE("single-owner: no baseline entry is past its remove_by date") {
    const std::string today = today_iso();
    std::ostringstream due;
    for (const KnownCopy& k : known_copies()) {
        const std::string m = baseline_due(copy_entry_name(k), k.dates, today);
        if (!m.empty()) due << m << "\n";
    }
    for (const KnownClone& k : known_clones()) {
        const std::string m = baseline_due(clone_entry_name(k), k.dates, today);
        if (!m.empty()) due << m << "\n";
    }
    INFO(due.str());
    CHECK(due.str().empty());
}

TEST_CASE("single-owner: the baseline dates fail on a made-up entry") {
    CHECK(iso_plus_days("2026-10-05", 14) == "2026-10-19");
    CHECK(iso_plus_days("2026-12-25", 14) == "2027-01-08");
    CHECK(iso_plus_days("2026-02-30", 0) == "2026-03-02");  // not a real date
    CHECK(iso_plus_days("2026-1-05", 0).empty());

    const KnownClone clone{"src/a.cpp", "src/b.cpp", 9, "x = 1;", {"2026-10-05", "MR1", "2026-10-19"}};
    const std::string name = clone_entry_name(clone);
    CHECK(baseline_due(name, clone.dates, "2026-10-19").empty());  // the last allowed day
    CHECK(baseline_due(name, clone.dates, "2026-10-20") ==
          "baseline.json: known_clones src/a.cpp and src/b.cpp, 9 lines from: x = 1;: past its "
          "remove_by date 2026-10-19 (added by MR1 on 2026-10-05): fix the copy and delete the entry");

    json entry = json::parse(R"({"date": "2026-10-05", "added_by": "MR1", "remove_by": "2026-10-19"})");
    CHECK(parse_dates(entry, "made-up entry").remove_by == "2026-10-19");
    entry["remove_by"] = "2026-10-20";
    CHECK_THROWS_WITH(parse_dates(entry, "made-up entry"),
                      "made-up entry: remove_by 2026-10-20 is more than 14 days after its date "
                      "2026-10-05 (2026-10-19 at most)");
    entry["remove_by"] = "2026-13-01";
    CHECK_THROWS_WITH(parse_dates(entry, "made-up entry"),
                      "made-up entry: \"2026-13-01\" is not a YYYY-MM-DD date");
    entry.erase("remove_by");
    CHECK_THROWS_WITH(parse_dates(entry, "made-up entry"),
                      "made-up entry: missing field \"remove_by\"");
}

TEST_CASE("single-owner: the anchor guard fails on a made-up row") {
    OwnerRule r;
    r.question = "Is this a made-up question?";
    r.rules_file = "rules/made_up.json";
    r.pattern = "made_up\\(|other_name\\(";
    r.anchor = "made_up(";
    r.must_match = {"x = made_up(1);"};
    r.owner_lines = {{"src/a.cpp", "return made_up(2);", "the owner"}};
    CHECK(anchor_misses(r).empty());
    // The regex flags this line, but the anchor would skip it.
    r.must_match.push_back("y = other_name(3);");
    r.owner_lines.push_back({"src/a.cpp", "return other_name(4);", "the owner"});
    const std::vector<std::string> misses = anchor_misses(r);
    REQUIRE(misses.size() == 2);
    CHECK(misses[0] ==
          "rules/made_up.json: \"Is this a made-up question?\": anchor \"made_up(\" is not in its "
          "own example: y = other_name(3);");
    CHECK(misses[1] ==
          "rules/made_up.json: \"Is this a made-up question?\": anchor \"made_up(\" is not in its "
          "own example: return other_name(4);");
}
