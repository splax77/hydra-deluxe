// One owner per rule, checked where a grep can check it. The rows are data,
// in tests/single_owner/rules/ (select_rule_files in engine.cpp says which
// files), and the baselines are in tests/single_owner/baseline.json; the
// engine's loader (engine.h) is their one reader, and tables() below reads
// them once. Each row names a question, the one place that answers it, and
// a pattern for the text that answers it. A matching line anywhere else
// fails here, unless it calls the owner on that same line, or the baseline
// lists it with the fix that will remove it. A baseline entry that no longer
// matches also fails, so the list only shrinks. Each listed line (baseline
// or owner) covers exactly one line of source, so a word-for-word copy of it
// fails too.
//
// This is the one scan of src/, tools/ and tests/. A row covers src/ and
// tools/ unless it names its own scope; the rows about tests (a test that
// recomputes an engine fact, or walks the source tree itself) cover tests/.
// The long-path rules (ADR 0020) are rows here too, so a new one-place rule
// is a new row, not a new walker. The walk itself, and the repo root, come
// from tests/source_tree.h. The clone scan in clones.cpp (ADR 0025) reads the
// same walk for blocks of code pasted into two places. Whether a line
// answers a row is the engine's (engine.cpp); this file holds the cases.
#include "doctest.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../audio_chart_fixtures.h"  // write_text_file
#include "../code_lexer.h"
#include "../source_tree.h"
#include "../temp_util.h"
#include "check_scan.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "engine.h"
#include "json.hpp"
#include "listed.h"
#include "rows.h"

namespace fs = std::filesystem;
using namespace single_owner;

namespace {

using json = nlohmann::json;

const RuleTables& tables() {
    static const RuleTables t = load_tables(sourcetree::root());
    return t;
}

}  // namespace

namespace single_owner {

const std::vector<OwnerRule>& rules() { return tables().rules; }
const std::vector<KnownCopy>& known_copies() { return tables().copies; }
const std::vector<KnownClone>& known_clones() { return tables().clones; }

}  // namespace single_owner

namespace {

// Today on this machine's calendar, as YYYY-MM-DD.
std::string today_iso() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &local);
    return buf;
}

// Empty while `today` (YYYY-MM-DD) is on or before the entry's remove_by,
// else the failure. ISO dates compare as plain text.
std::string baseline_due(const std::string& entry, const BaselineDates& d, const std::string& today) {
    if (today <= d.remove_by) return {};
    return "baseline.json: " + entry + ": past its remove_by date " + d.remove_by + " (added by " +
           d.added_by + " on " + d.date + "): fix the copy and delete the entry";
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

std::vector<CompiledRule> compile_rules() {
    std::vector<CompiledRule> out;
    for (const OwnerRule& r : rules()) out.push_back(compile_rule(r));
    return out;
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

// The message for line `t` at `where` (file:line) that answers the row's
// question without its owner. Both scans give it.
std::string answers_message(const OwnerRule& rule, const std::string& where, const std::string& t) {
    return where + ": answers \"" + rule.question + "\", which belongs to " + rule.owner + ": " + t +
           rule_suffix(rule.rules_file);
}

// The engine's scan state plus the failures the cases report.
struct RowScan : ScanState {
    std::vector<std::string> problems;
};

RowScan start_row_scan(const std::vector<CompiledRule>& compiled, const std::vector<ListedLine>& listed) {
    return {start_scan(compiled, listed), {}};
}

// The row scan of one file, with each flagged line no listed line took
// turned into a failure.
void scan_rows(RowScan& s, const std::string& rel, bool py, std::istream& in) {
    scan_file(s, rel, py, in, [&](const RowHit& h) {
        if (h.take == Take::taken) return;
        const OwnerRule& rule = *s.compiled[h.rule_index].rule;
        const std::string where = rel + ":" + std::to_string(h.line);
        if (h.take == Take::used_up)
            s.problems.push_back(where + ": a second copy of a listed line (each "
                                 "entry covers one line) answers \"" +
                                 rule.question + "\": " + h.text + rule_suffix(rule.rules_file));
        else
            s.problems.push_back(answers_message(rule, where, h.text));
    });
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

TEST_CASE("single-owner rules hold across src/, tools/ and tests/") {
    const fs::path root = sourcetree::root();
    const std::vector<CompiledRule> compiled = compile_rules();
    const std::vector<ListedLine> listed = listed_lines(rules(), known_copies());
    RowScan scan = start_row_scan(compiled, listed);
    int files = 0;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            const std::optional<codelex::Lang> lang = walk_language(rel);
            if (!lang) return;
            ++files;
            std::ifstream in(path);
            scan_rows(scan, rel, *lang == codelex::Lang::Python, in);
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

namespace {

// The made-up row both made-up file cases scan with: two owner lines, the
// second of which no line of kMadeUpFile uses.
OwnerRule made_up_owner_row() {
    OwnerRule r;
    r.question = "Is this a made-up question?";
    r.owner = "made_up in src/a.cpp";
    r.rules_file = "rules/made_up.json";
    r.pattern = "made_up\\(";
    r.anchor = "made_up(";
    r.owner_lines = {{"src/a.cpp", "return made_up(1);", "the owner"},
                     {"src/a.cpp", "return made_up(9);", "the owner, gone"}};
    return r;
}

// The made-up src/a.cpp: an owner line, a comment, an unlisted answer and a
// second copy of the owner line.
const char* const kMadeUpFile = "int f() {\n  return made_up(1);\n}\n// made_up(2) in a comment\n"
                                "x = made_up(3);\n  return made_up(1);\n";

}  // namespace

TEST_CASE("single-owner: the row scan's messages, pinned on a made-up file") {
    const OwnerRule r = made_up_owner_row();
    const std::vector<CompiledRule> compiled = {compile_rule(r)};
    std::vector<ListedLine> listed;
    for (const OwnerLine& o : r.owner_lines)
        listed.push_back({r.question, o.file, o.line_text, "owner line no longer matches (update it)",
                          r.rules_file});
    RowScan scan = start_row_scan(compiled, listed);
    std::istringstream in(kMadeUpFile);
    scan_rows(scan, "src/a.cpp", false, in);
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

TEST_CASE("single-owner: the hit sink reports taken, used-up and unlisted lines") {
    const OwnerRule r = made_up_owner_row();
    const std::vector<CompiledRule> compiled = {compile_rule(r)};
    const std::vector<ListedLine> listed = listed_lines({r}, {});
    ScanState scan = start_scan(compiled, listed);
    std::istringstream in(kMadeUpFile);
    std::vector<RowHit> hits;
    scan_file(scan, "src/a.cpp", false, in, [&](const RowHit& h) { hits.push_back(h); });
    REQUIRE(hits.size() == 3);
    CHECK(hits[0].rule_index == 0);
    CHECK(hits[0].line == 2);
    CHECK(hits[0].text == "return made_up(1);");
    CHECK(hits[0].take == Take::taken);
    CHECK(hits[0].listed_index == std::optional<size_t>(0));
    CHECK(hits[1].line == 5);
    CHECK(hits[1].text == "x = made_up(3);");
    CHECK(hits[1].take == Take::unlisted);
    CHECK_FALSE(hits[1].listed_index.has_value());
    CHECK(hits[2].line == 6);
    CHECK(hits[2].text == "return made_up(1);");
    CHECK(hits[2].take == Take::used_up);
    CHECK_FALSE(hits[2].listed_index.has_value());
}

TEST_CASE("single-owner: select_rule_files picks the rules files from a path list") {
    // An area file, a file one folder down, a .txt and a file two folders
    // down, given out of order; the names come back in byte order, so an
    // upper-case name sorts first.
    const std::vector<std::string> paths = {"ui.json",          "app/topic.json", "app/notes.txt",
                                            "app/deep/row.json", "app.json",       "Area.json"};
    CHECK(select_rule_files(paths) == std::vector<std::string>{"rules/Area.json", "rules/app.json",
                                                               "rules/app/topic.json",
                                                               "rules/ui.json"});
    CHECK(select_rule_files({}).empty());
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

TEST_CASE("single-owner: a row in an area's folder is loaded and named by its path") {
    const std::string dir = testtemp::temp_dir("single_owner_rules");
    testtemp::remove_dir(dir);
    fs::create_directories(hydra::os_path(dir + "\\made_up"));
    const std::string row = R"({"owner": "x in y", "owner_file": "src/core/winstr.cpp",
        "owner_symbol": "fits_shell", "decision": "D0", "added_by": "rl-glob", "pattern": "x", )";
    audiochart::write_text_file(dir + "\\made_up.json",
        R"({"rules": [)" + row + R"("question": "Is this an area row?", "date": "2026-10-10"}]})");
    audiochart::write_text_file(dir + "\\made_up\\topic.json",
        R"({"rules": [)" + row + R"("question": "Is this a topic row?", "date": "2026-10-10"}]})");
    audiochart::write_text_file(dir + "\\made_up\\notes.txt", "not a rules file");

    const std::vector<OwnerRule> rows = load_rules(hydra::os_path(dir));
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].question == "Is this an area row?");
    CHECK(rows[0].rules_file == "rules/made_up.json");
    CHECK(rows[1].question == "Is this a topic row?");
    CHECK(rows[1].rules_file == "rules/made_up/topic.json");

    audiochart::write_text_file(dir + "\\made_up\\topic.json",
        R"({"rules": [)" + row + R"("question": "Is this a topic row?"}]})");
    CHECK_THROWS_WITH(load_rules(hydra::os_path(dir)),
                      "rules/made_up/topic.json: \"Is this a topic row?\": missing field \"date\"");
    testtemp::remove_dir(dir);
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
