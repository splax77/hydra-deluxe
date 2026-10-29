// The single-owner row scan's engine; engine.h says what each part answers.
// The walk's folders come from tests/source_tree.h (walk_visits); nothing
// here reads the repo root.
#include "engine.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <stdexcept>

#include "../source_tree.h"
#include "core/strutil.h"

namespace fs = std::filesystem;

namespace single_owner {

namespace {

// Rule files are read in name order (select_rule_files) and rows in file
// order. A missing required field, a field the loader does not know, or a
// value of the wrong type fails with the rules file and the row's question,
// so a typo cannot drop a field quietly.

using json = nlohmann::json;

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

json parse_json_text(const std::string& text, const std::string& name) {
    try {
        return json::parse(text);
    } catch (const json::exception& e) {
        throw std::runtime_error(name + ": " + e.what());
    }
}

std::string read_text_file(const fs::path& path, const std::string& name) {
    std::ifstream in(path, std::ios::binary);
    if (!in.good()) throw std::runtime_error(name + ": cannot read " + path.u8string());
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

bool is_iso_shape(const std::string& s) {
    if (s.size() != 10) return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (i == 4 || i == 7 ? s[i] != '-' : !std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    return true;
}

// Whether a trimmed line is all comment in `lang`, as codelex splits it.
bool whole_line_comment(const std::string& trimmed, codelex::Lang lang) {
    const std::vector<codelex::Piece> pieces = codelex::split(trimmed, lang);
    return pieces.size() == 1 && pieces[0].kind == codelex::Kind::Comment;
}

}  // namespace

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

std::vector<OwnerRule> parse_rules_file(const std::string& name, const std::string& json_text) {
    const json doc = parse_json_text(json_text, name);
    check_keys(doc, {"rules"}, name);
    if (!doc.contains("rules") || !doc.at("rules").is_array())
        throw std::runtime_error(name + ": missing field \"rules\"");
    const json& rows = doc.at("rules");
    std::vector<OwnerRule> rules;
    for (size_t i = 0; i < rows.size(); ++i) rules.push_back(parse_rule(rows[i], name, i));
    return rules;
}

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

std::string today_iso() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &local);
    return buf;
}

// ISO dates compare as plain text.
std::string baseline_due(const std::string& where, const BaselineDates& d, const std::string& today) {
    if (today <= d.remove_by) return {};
    return where + ": past its remove_by date " + d.remove_by + " (added by " + d.added_by + " on " + d.date +
           "): fix it and delete the entry";
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

RuleTables parse_baseline(const std::string& json_text) {
    const std::string name = "baseline.json";
    const json doc = parse_json_text(json_text, name);
    check_keys(doc, {"known_copies", "known_clones"}, name);
    RuleTables t;
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

// A rules file is a .json in the rules folder itself (an area file) or one
// folder down, in a folder named after an area (ADR 0029).
std::vector<std::string> select_rule_files(const std::vector<std::string>& paths) {
    std::vector<std::string> names;
    for (const std::string& p : paths) {
        if (std::count(p.begin(), p.end(), '/') > 1) continue;
        if (fs::u8path(p).extension() != ".json") continue;
        names.push_back("rules/" + p);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<std::pair<std::string, fs::path>> rule_files(const fs::path& dir) {
    std::vector<std::string> paths;
    for (const fs::directory_entry& e : fs::recursive_directory_iterator(dir))
        if (e.is_regular_file()) paths.push_back(fs::relative(e.path(), dir).generic_u8string());
    std::vector<std::pair<std::string, fs::path>> files;
    for (const std::string& name : select_rule_files(paths))
        files.push_back({name, dir / fs::u8path(name.substr(std::string("rules/").size()))});
    return files;
}

std::vector<OwnerRule> load_rules(const fs::path& dir) {
    std::vector<OwnerRule> rules;
    for (const auto& [name, p] : rule_files(dir))
        for (OwnerRule& r : parse_rules_file(name, read_text_file(p, name))) rules.push_back(std::move(r));
    return rules;
}

RuleTables load_tables(const fs::path& root) {
    std::vector<OwnerRule> rules = load_rules(root / fs::u8path(kRulesDir));
    RuleTables t = parse_baseline(read_text_file(root / fs::u8path(kBaselineFile), "baseline.json"));
    t.rules = std::move(rules);
    return t;
}

CompiledRule compile_rule(const OwnerRule& r) {
    const auto flags = std::regex::ECMAScript | std::regex::optimize;
    return {&r, std::regex(r.pattern, flags),
            std::regex(r.calls_owner.empty() ? "$^" : r.calls_owner, flags)};
}

bool flags_line(const CompiledRule& c, const std::string& line) {
    if (!c.rule->anchor.empty() && line.find(c.rule->anchor) == std::string::npos) return false;
    if (!std::regex_search(line, c.pattern)) return false;
    return c.rule->calls_owner.empty() || !std::regex_search(line, c.calls_owner);
}

codelex::Lang lexer_language(const fs::path& path) {
    return codelex::language_of_extension(path.extension().string()).value_or(codelex::Lang::Cpp);
}

// Both row scans ask this: C++ keeps is_line_comment's test, every other
// language is split by codelex.
bool skipped_comment_line(const std::string& trimmed, codelex::Lang lang) {
    return lang == codelex::Lang::Cpp ? is_line_comment(trimmed) : whole_line_comment(trimmed, lang);
}

bool in_scope(const OwnerRule& r, const std::string& sub, const std::string& rel) {
    if (r.scope.empty()) return sub == "src" || sub == "tools";
    for (const std::string& s : r.scope)
        if (s == sub || s == rel) return true;
    return false;
}

// The walk and the case for the single files outside it both ask this, so a
// file is read by exactly one of them.
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

bool walk_reads_for(const OwnerRule& r, const std::string& rel) {
    const std::optional<codelex::Lang> lang = walk_language(rel);
    return lang && (*lang == codelex::Lang::Python) == (r.language == "py");
}

// Owner lines first, so an owner line is used before a baseline entry with
// the same text.
std::vector<ListedLine> listed_lines(const std::vector<OwnerRule>& rules,
                                     const std::vector<KnownCopy>& copies) {
    std::vector<ListedLine> out;
    for (const OwnerRule& r : rules)
        for (const OwnerLine& o : r.owner_lines)
            out.push_back({r.question, o.file, o.line_text,
                           "owner line no longer matches (update it)", r.rules_file});
    for (const KnownCopy& k : copies)
        out.push_back({k.question, k.file, k.line_text,
                       "baseline entry no longer matches (remove it)"});
    return out;
}

ScanState start_scan(const std::vector<CompiledRule>& compiled, const std::vector<ListedLine>& listed) {
    return {compiled, listed, std::vector<bool>(listed.size(), false),
            std::vector<int>(compiled.size(), 0)};
}

void scan_file(ScanState& s, const std::string& rel, bool py, std::istream& in, const HitSink& sink) {
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
            size_t taken_index = 0;
            const Take take = take_listed_line(s.listed, s.used, rule.question, rel, t, &taken_index);
            RowHit hit{ci, lineno, t, take, std::nullopt};
            if (take == Take::taken) hit.listed_index = taken_index;
            sink(hit);
        }
    }
}

}  // namespace single_owner
