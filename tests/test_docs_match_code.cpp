// The docs say what the code does. Three checks over the decision records
// and the guides:
//
// 1. Every backticked code name in them is still a name the code uses (see
//    code_words for which words count). A name that left the code fails with
//    the doc, the line and the name, so the fixer knows which sentence to edit.
// 2. The User Guide's hydra_rules.ini sample, read by the app's own rules-file
//    reader, equals core::Rules{}.
// 3. A default written in prose carries a marker right after it,
//    `10 ms<!-- default: Settings::mslimit_value -->`, and the number before
//    the marker equals the value in code. For a flag the marker follows "on"
//    or "off".
//
// It finds the repo and walks the code through tests/source_tree.h, as
// tests/test_single_owner.cpp does.
#include "doctest.h"

#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "app/config.h"
#include "app/rules_file.h"
#include "code_lexer.h"
#include "core/model.h"
#include "core/rules.h"
#include "core/strutil.h"
#include "source_tree.h"
#include "temp_util.h"

namespace fs = std::filesystem;

namespace {

// ---- the docs ----------------------------------------------------------

struct Doc {
    std::string rel;                 // repo-relative, forward slashes
    std::vector<std::string> lines;  // without line endings
};

std::vector<std::string> read_lines(const fs::path& p) {
    std::ifstream in(p);
    std::vector<std::string> out;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        out.push_back(line);
    }
    return out;
}

// Every docs/adr/*.md and docs/agents/*.md, the README, CONTEXT.md, the User
// Guide, development.md and the cap-clamped squeeze note.
std::vector<Doc> docs() {
    const fs::path root = sourcetree::root();
    std::vector<std::string> rels;
    for (const char* dir : {"adr", "agents"}) {
        for (const auto& e : fs::directory_iterator(root / "docs" / dir)) {
            if (e.path().extension() != ".md") continue;
            rels.push_back(fs::relative(e.path(), root).generic_u8string());
        }
    }
    for (const char* rel : {"README.md", "CONTEXT.md", "docs/UserGuide.md", "docs/development.md",
                            "docs/cap-clamped-squeeze-frontend-anchor.md"})
        rels.push_back(rel);
    std::vector<Doc> out;
    for (const std::string& rel : rels) {
        Doc d{rel, read_lines(root / fs::u8path(rel))};
        REQUIRE_MESSAGE(!d.lines.empty(), rel << " is missing or empty");
        out.push_back(std::move(d));
    }
    return out;
}

// A Markdown heading's level (1 to 6), or 0 for any other line.
int heading_level(const std::string& line) {
    size_t n = 0;
    while (n < line.size() && line[n] == '#') ++n;
    if (n == 0 || n > 6 || n >= line.size() || line[n] != ' ') return 0;
    return static_cast<int>(n);
}

bool is_fence(const std::string& line) { return hydra::trim(line).rfind("```", 0) == 0; }

// ---- check 1: code names exist -----------------------------------------

// Adds every identifier-like word in text[begin, end) to `words`.
void add_words(const std::string& text, size_t begin, size_t end, std::set<std::string>& words) {
    size_t i = begin;
    while (i < end) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (std::isalnum(c) || c == '_') {
            size_t j = i;
            while (j < end && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_'))
                ++j;
            words.insert(text.substr(i, j - i));
            i = j;
        } else {
            ++i;
        }
    }
}

// The words check 1 accepts as real code names. The user chose this level on
// 2026-10-10 (the single-owner redesign plan, question 7): a comment never
// vouches for a name, and a test's string literal never does either, because
// production strings name real INI keys, chart tags and stored columns while
// test strings name made-up ones. This file is left out: its allow-list would
// otherwise vouch for the names it lists.
std::set<std::string> code_words() {
    std::set<std::string> words;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            const fs::path ext = path.extension();
            codelex::Lang lang;
            if (ext == ".cpp" || ext == ".h" || ext == ".c" || ext == ".hlsl")
                lang = codelex::Lang::Cpp;
            else if (ext == ".py")
                lang = codelex::Lang::Python;
            else if (ext == ".ps1")
                lang = codelex::Lang::PowerShell;
            else
                return;
            if (rel == "tests/test_docs_match_code.cpp") return;
            const bool strings_count = rel.rfind("tests/", 0) != 0;
            std::ifstream in(path, std::ios::binary);
            std::ostringstream ss;
            ss << in.rdbuf();
            const std::string text = ss.str();
            for (const codelex::Piece& p : codelex::split(text, lang))
                if (p.kind == codelex::Kind::Code || (p.kind == codelex::Kind::String && strings_count))
                    add_words(text, p.begin, p.end, words);
    });
    // Build targets, options and CMake functions: hydra_batch, HYDRA_LTCG,
    // hydra_use_mimalloc and the like.
    std::ifstream in(sourcetree::root() / "CMakeLists.txt");
    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string cmake = ss.str();
    static const std::regex target(
        R"(\b(?:add_executable|add_library|add_custom_target|option|function|macro)\s*\(\s*([A-Za-z_]\w*))");
    for (auto it = std::sregex_iterator(cmake.begin(), cmake.end(), target);
         it != std::sregex_iterator(); ++it)
        words.insert((*it)[1].str());
    return words;
}

// A backticked span that names code, split into its `::` parts; empty when
// the span is not code by this test's rule. Code is `name(`, `name()` or
// `name(args)`, `Type::name`, `kName`, or a name with an underscore. File
// paths and `key = value` lines are not code.
std::vector<std::string> code_name_parts(std::string span) {
    span = hydra::trim(span);
    if (span.empty()) return {};
    if (span.find_first_of("/\\=") != std::string::npos) return {};
    static const std::regex file_ext(
        R"(\.(cpp|h|c|md|ini|json|py|ps1|exe|db|txt|mid|chart|sng|srb|opus|ogg|mp3|wav|flac|png|jpg|cmake|hlsl|dll)$)",
        std::regex::icase);
    if (std::regex_search(span, file_ext)) return {};
    bool call = false;
    const size_t paren = span.find('(');
    if (paren != std::string::npos) {
        // `name(` with the parenthesis right after the name: "Kick (Ghost)"
        // is prose, not a call.
        if (paren == 0 || std::isspace(static_cast<unsigned char>(span[paren - 1]))) return {};
        if (span.back() != ')' && paren != span.size() - 1) return {};
        span = hydra::trim(span.substr(0, paren));
        call = true;
    }
    static const std::regex qualified(R"(^(?:[A-Za-z_]\w*::)*[A-Za-z_]\w*$)");
    if (!std::regex_match(span, qualified)) return {};
    std::vector<std::string> parts;
    size_t from = 0;
    for (size_t at; (at = span.find("::", from)) != std::string::npos; from = at + 2)
        parts.push_back(span.substr(from, at - from));
    parts.push_back(span.substr(from));
    const std::string& last = parts.back();
    const bool k_name = last.size() > 1 && last[0] == 'k' && std::isupper(static_cast<unsigned char>(last[1]));
    const bool underscore = last.find('_') != std::string::npos;
    if (!call && parts.size() == 1 && !k_name && !underscore) return {};
    return parts;
}

// Names a doc may mention although no code has them, each with the reason.
struct Allowed {
    std::string doc;   // repo-relative doc path prefix
    std::string name;  // the last `::` part, as the doc spells it
    std::string why;
};

const std::vector<Allowed>& allowed() {
    static const std::vector<Allowed> a = {
        // ADR 0011's decision says these two were removed; the sentence is
        // the record of their removal.
        {"docs/adr/0011", "deact_tick_from_rows", "ADR 0011 records that it is gone"},
        {"docs/adr/0011", "deact_tick_for", "ADR 0011 records that it is gone"},
        // ADR 0017 records that the whole-record blob format these two spoke
        // was deleted.
        {"docs/adr/0017", "write_record", "ADR 0017 records that it is gone"},
        {"docs/adr/0017", "read_record", "ADR 0017 records that it is gone"},
        // ADR 0015's decision names it, and its dated line records that it
        // was removed later (34137fb).
        {"docs/adr/0015", "from_code", "ADR 0015 records that it is gone"},
        // hydra_report, the command-line report tool, was deleted when the
        // reports became Hydra windows (D103 item 5). ADR 0027 records the
        // deletion; ADRs 0002 and 0010 carry dated notes that it is gone.
        {"docs/adr/0002", "hydra_report", "ADR 0002's dated note records that it is gone"},
        {"docs/adr/0010", "hydra_report", "ADR 0010's dated note records that it is gone"},
        {"docs/adr/0027", "hydra_report", "ADR 0027 records that it is gone"},
        // ADR 0014 records that the edge's sqout_time field went with T10's
        // clamp-origin guard.
        {"docs/adr/0014", "sqout_time", "ADR 0014 records that it is gone"},
        // ADRs 0017 and 0021 record that the single stored transfer_pre pair
        // was replaced by one scale per SqIn.
        {"docs/adr/0017", "transfer_pre", "ADR 0017 records that it is gone"},
        {"docs/adr/0021", "transfer_pre", "ADR 0021 records that it is gone"},
    };
    return a;
}

std::string first_heading_skip_reason(const std::string& line) {
    if (line.find("Superseded") != std::string::npos) return "Superseded";
    if (line.find("Kept as history") != std::string::npos) return "Kept as history";
    return "";
}

// ---- check 3: marked defaults -------------------------------------------

struct KnownDefault {
    bool flag;     // a flag is written "on" or "off"
    double value;  // a number, or 1/0 for a flag
};

// Every marker name the docs may use, with the value the code holds.
const std::map<std::string, KnownDefault>& known_defaults() {
    static const std::map<std::string, KnownDefault> m = [] {
        const hydra::app::Settings s{};
        const hydra::core::Rules r{};
        std::map<std::string, KnownDefault> out;
        out["Settings::mslimit_value"] = {false, static_cast<double>(s.mslimit_value)};
        out["Settings::mslimit_enabled"] = {true, s.mslimit_enabled ? 1.0 : 0.0};
        out["Settings::backendlimit_value"] = {false, static_cast<double>(s.backendlimit_value)};
        out["Settings::backendlimit_enabled"] = {true, s.backendlimit_enabled ? 1.0 : 0.0};
        out["Rules::backend_leeway_ms"] = {false, r.backend_leeway_ms};
        out["kSqueezeWindowMs"] = {false, hydra::kSqueezeWindowMs};
        out["kEarlyFillWindowMs"] = {false, hydra::kEarlyFillWindowMs};
        out["kDefaultDepthValue"] = {false, static_cast<double>(hydra::kDefaultDepthValue)};
        out["kSpActivationBars"] = {false, static_cast<double>(hydra::kSpActivationBars)};
        return out;
    }();
    return m;
}

// The problems check 3 finds on one doc line; `seen` collects the marker
// names found.
void check_markers(const std::string& rel, int lineno, const std::string& line,
                   std::set<std::string>& seen, std::vector<std::string>& problems) {
    static const std::regex marker(R"(<!--\s*default:\s*([^\s>]+)\s*-->)");
    static const std::regex number(R"((\d+(?:\.\d+)?))");
    static const std::regex last_word(R"(([A-Za-z]+)\s*$)");
    const std::string where = rel + ":" + std::to_string(lineno) + ": ";
    for (auto it = std::sregex_iterator(line.begin(), line.end(), marker);
         it != std::sregex_iterator(); ++it) {
        const std::string name = (*it)[1].str();
        const std::string before = line.substr(0, static_cast<size_t>(it->position()));
        const auto known = known_defaults().find(name);
        if (known == known_defaults().end()) {
            problems.push_back(where + "unknown default marker \"" + name + "\"");
            continue;
        }
        seen.insert(name);
        const KnownDefault& k = known->second;
        if (k.flag) {
            std::smatch w;
            const std::string word =
                std::regex_search(before, w, last_word) ? hydra::to_lower_ascii(w[1].str()) : "";
            if (word != "on" && word != "off") {
                problems.push_back(where + "marker " + name +
                                   " must follow the word \"on\" or \"off\"");
                continue;
            }
            const bool doc_on = word == "on";
            if (doc_on != (k.value != 0.0))
                problems.push_back(where + name + ": the doc says \"" + word +
                                   "\", the code starts " + (k.value != 0.0 ? "on" : "off"));
            continue;
        }
        std::string last;
        for (auto n = std::sregex_iterator(before.begin(), before.end(), number);
             n != std::sregex_iterator(); ++n)
            last = (*n)[1].str();
        if (last.empty()) {
            problems.push_back(where + "marker " + name + " has no number before it on its line");
            continue;
        }
        const double doc_value = std::stod(last);
        if (std::fabs(doc_value - k.value) > 1e-9) {
            std::ostringstream msg;
            msg << where << name << ": the doc says " << last << ", the code says " << k.value;
            problems.push_back(msg.str());
        }
    }
}

std::string report(const std::vector<std::string>& problems) {
    std::ostringstream out;
    for (const std::string& p : problems) out << p << "\n";
    return out.str();
}

}  // namespace

TEST_CASE("docs match code: every backticked code name exists in src/, tools/ or tests/") {
    const std::set<std::string> words = code_words();
    REQUIRE(words.size() > 1000);  // the walk found the sources
    std::vector<bool> allow_used(allowed().size(), false);
    std::vector<std::string> problems;
    int names_checked = 0;
    for (const Doc& d : docs()) {
        bool fenced = false;
        int skip_level = 0;  // inside a skipped section of this level; 0 = not
        for (size_t i = 0; i < d.lines.size(); ++i) {
            const std::string& line = d.lines[i];
            if (is_fence(line)) {
                fenced = !fenced;
                continue;
            }
            if (fenced) continue;
            if (const int level = heading_level(line)) {
                if (skip_level && level <= skip_level) skip_level = 0;
                if (!skip_level && !first_heading_skip_reason(line).empty()) skip_level = level;
            }
            if (skip_level) continue;
            // Inline code spans: text between a pair of backticks on one line.
            size_t from = 0;
            while (true) {
                const size_t open = line.find('`', from);
                if (open == std::string::npos) break;
                const size_t close = line.find('`', open + 1);
                if (close == std::string::npos) break;
                from = close + 1;
                const std::vector<std::string> parts =
                    code_name_parts(line.substr(open + 1, close - open - 1));
                if (parts.empty()) continue;
                ++names_checked;
                bool all_found = true;
                for (const std::string& p : parts) all_found = all_found && words.count(p);
                if (all_found) continue;
                bool ok = false;
                for (size_t a = 0; a < allowed().size(); ++a) {
                    if (d.rel.rfind(allowed()[a].doc, 0) == 0 && parts.back() == allowed()[a].name) {
                        allow_used[a] = true;
                        ok = true;
                    }
                }
                if (ok) continue;
                std::string missing;
                for (const std::string& p : parts)
                    if (!words.count(p)) missing += (missing.empty() ? "" : ", ") + p;
                problems.push_back(d.rel + ":" + std::to_string(i + 1) + ": `" +
                                   line.substr(open + 1, close - open - 1) + "` names " +
                                   missing + ", which the code does not use (see code_words)");
            }
        }
    }
    // An allow-list entry that no doc needs any more is removed, so the list
    // only shrinks.
    for (size_t a = 0; a < allowed().size(); ++a)
        if (!allow_used[a])
            problems.push_back("allow-list entry no doc needs: " + allowed()[a].doc + " `" +
                               allowed()[a].name + "`");
    CHECK(names_checked > 100);  // the docs had names to check
    INFO(report(problems));
    CHECK(problems.empty());
}

TEST_CASE("docs match code: the User Guide's hydra_rules.ini sample is the defaults") {
    const fs::path root = sourcetree::root();
    const std::vector<std::string> guide = read_lines(root / "docs" / "UserGuide.md");
    // The first fenced block after the "Scoring rules" heading.
    std::string sample;
    bool in_section = false, in_block = false, found = false;
    for (const std::string& line : guide) {
        if (!in_section) {
            in_section = heading_level(line) && line.find("Scoring rules") != std::string::npos;
            continue;
        }
        if (is_fence(line)) {
            if (in_block) {
                found = true;
                break;
            }
            in_block = true;
            continue;
        }
        if (in_block) sample += line + "\n";
    }
    REQUIRE_MESSAGE(found, "docs/UserGuide.md: no fenced block under the \"Scoring rules\" heading");
    REQUIRE(sample.find('=') != std::string::npos);

    const fs::path p = hydra::os_path(testtemp::temp_path("docs_match_code_rules", ".ini"));
    {
        std::ofstream f(p, std::ios::trunc);
        f << sample;
    }
    hydra::core::Rules loaded;
    try {
        loaded = hydra::app::load_rules_file(p);
    } catch (const hydra::app::RulesFileError& e) {
        fs::remove(p);
        FAIL("docs/UserGuide.md: the hydra_rules.ini sample does not load: " << e.what());
    }
    fs::remove(p);

    const hydra::core::Rules def{};
    const std::string where = "docs/UserGuide.md, the hydra_rules.ini sample: ";
    INFO(where << "backend_leeway_ms");
    CHECK(loaded.backend_leeway_ms == def.backend_leeway_ms);
    INFO(where << "sqout_rule");
    CHECK(loaded.sqout_rule == def.sqout_rule);
    INFO(where << "max_tied_paths");
    CHECK(loaded.max_tied_paths == def.max_tied_paths);
    INFO(where << "fill_cooldown_measures");
    CHECK(loaded.fill_cooldown_measures == def.fill_cooldown_measures);
    INFO(where << "fill_max_distance_beats");
    CHECK(loaded.fill_max_distance_beats == def.fill_max_distance_beats);
    INFO(where << "fill_length_measures");
    CHECK(loaded.fill_length_measures == def.fill_length_measures);
    INFO(where << "fill_land_slop_beats");
    CHECK(loaded.fill_land_slop_beats == def.fill_land_slop_beats);
    // Every field, the ones named above and any added later.
    INFO(where << "the fingerprint (a rules field differs from Rules{})");
    CHECK(loaded.fingerprint() == def.fingerprint());
}

TEST_CASE("docs match code: every marked default equals the code's value") {
    std::set<std::string> seen;
    std::vector<std::string> problems;
    for (const Doc& d : docs())
        for (size_t i = 0; i < d.lines.size(); ++i)
            check_markers(d.rel, static_cast<int>(i + 1), d.lines[i], seen, problems);
    // Every known marker is used somewhere, so none of them checks nothing.
    for (const auto& [name, k] : known_defaults())
        if (!seen.count(name))
            problems.push_back("no doc carries the marker <!-- default: " + name + " -->");
    INFO(report(problems));
    CHECK(problems.empty());
}

TEST_CASE("docs match code: the marker check catches a wrong number and a wrong word") {
    std::set<std::string> seen;
    std::vector<std::string> problems;
    check_markers("x.md", 1, "starts at 11 ms<!-- default: Settings::mslimit_value -->", seen,
                  problems);
    check_markers("x.md", 2, "starts off<!-- default: Settings::mslimit_enabled -->", seen,
                  problems);
    check_markers("x.md", 3, "is 3ms<!-- default: Rules::backend_leeway_ms -->", seen, problems);
    check_markers("x.md", 4, "is 9<!-- default: Settings::no_such_default -->", seen, problems);
    REQUIRE(problems.size() == 3);
    CHECK(problems[0] == "x.md:1: Settings::mslimit_value: the doc says 11, the code says 10");
    CHECK(problems[1] ==
          "x.md:2: Settings::mslimit_enabled: the doc says \"off\", the code starts on");
    CHECK(problems[2] == "x.md:4: unknown default marker \"Settings::no_such_default\"");
}

TEST_CASE("docs match code: what counts as a code name") {
    CHECK(code_name_parts("allows_dynamics()") == std::vector<std::string>{"allows_dynamics"});
    CHECK(code_name_parts("ambiguous_window_warnings(song, result, windows)") ==
          std::vector<std::string>{"ambiguous_window_warnings"});
    CHECK(code_name_parts("MidiParser::optype") ==
          std::vector<std::string>{"MidiParser", "optype"});
    CHECK(code_name_parts("kRowReadySql") == std::vector<std::string>{"kRowReadySql"});
    CHECK(code_name_parts("ChordNote::str()") == std::vector<std::string>{"ChordNote", "str"});
    // Not code: a plain word, a path, an ini line, a file, a command.
    CHECK(code_name_parts("Kick (Ghost, 2x)").empty());
    CHECK(code_name_parts("drums0dnoflip").empty());
    CHECK(code_name_parts("src/store/record_store.cpp").empty());
    CHECK(code_name_parts("max_tied_paths = 4").empty());
    CHECK(code_name_parts("hydra_rules.ini").empty());
    CHECK(code_name_parts("hydra_batch --legacy-fills").empty());
    CHECK(code_name_parts("[ENABLE_CHART_DYNAMICS]").empty());
}

namespace {

// The pieces of `text` as "C:", "#:" or "S:" (code, comment, string) plus the
// piece's text.
std::vector<std::string> lexed(const std::string& text, codelex::Lang lang) {
    std::vector<std::string> out;
    for (const codelex::Piece& p : codelex::split(text, lang)) {
        const char* tag = p.kind == codelex::Kind::Code      ? "C:"
                          : p.kind == codelex::Kind::Comment ? "#:"
                                                             : "S:";
        out.push_back(tag + text.substr(p.begin, p.end - p.begin));
    }
    return out;
}

using V = std::vector<std::string>;

}  // namespace

TEST_CASE("code lexer: C++ and Python comments and string literals") {
    const auto cpp = codelex::Lang::Cpp;
    const auto py = codelex::Lang::Python;
    // C++ line and block comments.
    CHECK(lexed("a // c\nb", cpp) == V{"C:a ", "#:// c", "C:\nb"});
    CHECK(lexed("a /* c\nd */ b", cpp) == V{"C:a ", "#:/* c\nd */", "C: b"});
    // A string with an escaped quote, and a comment marker inside a string.
    CHECK(lexed(R"(x = "q\"r" + y)", cpp) == V{"C:x = ", R"(S:"q\"r")", "C: + y"});
    CHECK(lexed(R"("// no" z)", cpp) == V{R"(S:"// no")", "C: z"});
    // A raw string holds a quote and a parenthesis without ending.
    CHECK(lexed(R"--(s = R"d(a")b)d"; t)--", cpp) ==
          V{"C:s = ", R"--(S:R"d(a")b)d")--", "C:; t"});
    // A character literal, and a digit separator that is not one.
    CHECK(lexed(R"(c = '\''; n = 1'000;)", cpp) == V{"C:c = ", R"(S:'\'')", "C:; n = 1'000;"});
    // Python line comments.
    CHECK(lexed("a # c\nb", py) == V{"C:a ", "#:# c", "C:\nb"});
    // Single and double quotes, with an escape; a prefix belongs to its string.
    CHECK(lexed(R"(x = 'q\'r' + "s")", py) == V{"C:x = ", R"(S:'q\'r')", "C: + ", R"(S:"s")"});
    CHECK(lexed("rb'x' y", py) == V{"S:rb'x'", "C: y"});
    // Triple-quoted strings span lines and hold a lone quote and a #.
    CHECK(lexed("'''a\n#b''' c", py) == V{"S:'''a\n#b'''", "C: c"});
    CHECK(lexed(R"("""a"b""" c)", py) == V{R"(S:"""a"b""")", "C: c"});
    // PowerShell: # and <# #> comments, a string that ends in a backslash, a
    // backtick-escaped quote, a doubled quote, and a here-string.
    const auto ps = codelex::Lang::PowerShell;
    CHECK(lexed("a # c\nb", ps) == V{"C:a ", "#:# c", "C:\nb"});
    CHECK(lexed("a <# c\n#d #> b", ps) == V{"C:a ", "#:<# c\n#d #>", "C: b"});
    CHECK(lexed(R"(x = 'C:\' + y # c)", ps) == V{"C:x = ", R"(S:'C:\')", "C: + y ", "#:# c"});
    CHECK(lexed("\"a`\"# b\" c", ps) == V{"S:\"a`\"# b\"", "C: c"});
    CHECK(lexed("'it''s # not' z", ps) == V{"S:'it''s # not'", "C: z"});
    CHECK(lexed("@'\na # b\n'@ c", ps) == V{"S:@'\na # b\n'@", "C: c"});
}
