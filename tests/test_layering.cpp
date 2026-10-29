// The layering test: every file includes only headers from the libraries its
// own library links. CMakeLists.txt is the one owner of "which library is
// this file in" (the add_library and add_executable source lists) and of
// "which library may it include from" (the target_link_libraries lines); this
// file reads both and owns neither. tests/layering/externals.json says which
// link line a third-party or system header stands for, and
// tests/layering/known_gaps.json lists the wrong-way includes a later task
// removes, in the shape of tests/single_owner/baseline.json, with its dates
// read by that baseline's reader (parse_dates in tests/single_owner/engine.h).
// The plan: docs/superpowers/plans/2026-10-28-hydra-libraries.md, "How the
// layering stays true".
#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "json.hpp"
#include "single_owner/engine.h"
#include "source_tree.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace {

// ---- Reading CMakeLists.txt ----

struct CMakeCommand {
    std::string name;  // lower case
    std::vector<std::string> args;
};

// The text with every `#` comment blanked to the end of its line. A `#`
// inside a quoted argument is text, not a comment.
std::string blank_comments(const std::string& text) {
    std::string out = text;
    bool quoted = false;
    for (size_t i = 0; i < out.size(); ++i) {
        const char c = out[i];
        if (c == '\n') {
            quoted = false;
        } else if (c == '"' && (i == 0 || out[i - 1] != '\\')) {
            quoted = !quoted;
        } else if (c == '#' && !quoted) {
            while (i < out.size() && out[i] != '\n') out[i++] = ' ';
            --i;
        }
    }
    return out;
}

// Every `name(args)` command in the text, in order, its arguments split on
// space outside quotes. Quotes are dropped from a quoted argument.
std::vector<CMakeCommand> read_commands(const std::string& raw) {
    const std::string text = blank_comments(raw);
    std::vector<CMakeCommand> out;
    size_t i = 0;
    while (i < text.size()) {
        if (!(std::isalpha(static_cast<unsigned char>(text[i])) || text[i] == '_')) {
            ++i;
            continue;
        }
        size_t j = i;
        while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) ++j;
        size_t k = j;
        while (k < text.size() && (text[k] == ' ' || text[k] == '\t')) ++k;
        if (k >= text.size() || text[k] != '(') {
            i = j;
            continue;
        }
        CMakeCommand cmd;
        for (size_t c = i; c < j; ++c)
            cmd.name += static_cast<char>(std::tolower(static_cast<unsigned char>(text[c])));
        int depth = 1;
        bool quoted = false;
        std::string token;
        bool have_token = false;
        size_t p = k + 1;
        for (; p < text.size() && depth > 0; ++p) {
            const char c = text[p];
            if (quoted) {
                if (c == '"' && text[p - 1] != '\\') quoted = false;
                else token += c;
                continue;
            }
            if (c == '"') {
                quoted = true;
                have_token = true;
            } else if (c == '(') {
                ++depth;
                token += c;
                have_token = true;
            } else if (c == ')') {
                if (--depth == 0) break;
                token += c;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (have_token) cmd.args.push_back(token);
                token.clear();
                have_token = false;
            } else {
                token += c;
                have_token = true;
            }
        }
        if (have_token) cmd.args.push_back(token);
        out.push_back(std::move(cmd));
        i = p + 1;
    }
    return out;
}

std::string replace_all(std::string s, const std::string& from, const std::string& to) {
    for (size_t at = s.find(from); at != std::string::npos; at = s.find(from, at + to.size()))
        s.replace(at, from.size(), to);
    return s;
}

// Whether `text` matches `pattern`, where `*` stands for any run of
// characters but `/`, as in a CMake GLOB.
bool glob_match(const std::string& pattern, const std::string& text) {
    size_t p = 0, t = 0, star = std::string::npos, mark = 0;
    while (t < text.size()) {
        if (p < pattern.size() && pattern[p] == '*') {
            star = p++;
            mark = t;
        } else if (p < pattern.size() && pattern[p] == text[t]) {
            ++p;
            ++t;
        } else if (star != std::string::npos && text[mark] != '/') {
            p = star + 1;
            t = ++mark;
        } else {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*') ++p;
    return p == pattern.size();
}

struct Links {
    std::vector<std::string> pub;   // PUBLIC and INTERFACE: seen by whoever links this
    std::vector<std::string> priv;  // PRIVATE: this target only
};

// What CMakeLists.txt says about every target it defines or names.
struct BuildModel {
    std::map<std::string, std::set<std::string>> file_targets;     // listed by name
    std::vector<std::pair<std::string, std::string>> glob_targets;  // (pattern, target)
    std::map<std::string, Links> links;
    std::map<std::string, Links> include_dirs;

    // The targets a repo-relative file is in: its own listing, else its
    // .cpp or .c twin's for a header.
    std::set<std::string> owners(const std::string& rel) const {
        std::set<std::string> out;
        auto add = [&](const std::string& f) {
            if (auto it = file_targets.find(f); it != file_targets.end())
                out.insert(it->second.begin(), it->second.end());
            for (const auto& [pattern, target] : glob_targets)
                if (glob_match(pattern, f)) out.insert(target);
        };
        add(rel);
        const std::string ext = fs::path(rel).extension().string();
        if (out.empty() && (ext == ".h" || ext == ".hpp")) {
            const std::string stem = rel.substr(0, rel.size() - ext.size());
            add(stem + ".cpp");
            add(stem + ".c");
        }
        return out;
    }

    // The targets `target` may include from: itself, every target it links,
    // and what those pass on through their PUBLIC links.
    std::set<std::string> allowed(const std::string& target) const {
        std::set<std::string> out{target};
        std::vector<std::string> todo;
        if (auto it = links.find(target); it != links.end()) {
            todo.insert(todo.end(), it->second.pub.begin(), it->second.pub.end());
            todo.insert(todo.end(), it->second.priv.begin(), it->second.priv.end());
        }
        while (!todo.empty()) {
            const std::string t = todo.back();
            todo.pop_back();
            if (!out.insert(t).second) continue;
            if (auto it = links.find(t); it != links.end())
                todo.insert(todo.end(), it->second.pub.begin(), it->second.pub.end());
        }
        return out;
    }

    // The include folders `target` compiles with: its own, and the PUBLIC
    // ones of every target it may include from.
    std::set<std::string> folders(const std::string& target) const {
        std::set<std::string> out;
        for (const std::string& t : allowed(target)) {
            auto it = include_dirs.find(t);
            if (it == include_dirs.end()) continue;
            out.insert(it->second.pub.begin(), it->second.pub.end());
            if (t == target) out.insert(it->second.priv.begin(), it->second.priv.end());
        }
        return out;
    }
};

const std::set<std::string> kTargetKeywords = {"STATIC",   "SHARED",          "MODULE", "INTERFACE",
                                               "OBJECT",   "EXCLUDE_FROM_ALL", "WIN32",  "IMPORTED",
                                               "GLOBAL",   "ALIAS",           "SYSTEM", "BEFORE",
                                               "AFTER"};

std::string strip_source_dir(const std::string& s) { return replace_all(s, "${CMAKE_SOURCE_DIR}/", ""); }

bool unresolved(const std::string& s) { return s.find("${") != std::string::npos || s.rfind("$<", 0) == 0; }

void apply(const std::vector<CMakeCommand>& cmds, size_t from, size_t to, BuildModel& m,
           std::map<std::string, std::vector<std::string>>& globs);

// The index just past the command that closes the block opened at `at`.
size_t block_end(const std::vector<CMakeCommand>& cmds, size_t at, const std::string& open,
                 const std::string& close) {
    int depth = 0;
    for (size_t i = at; i < cmds.size(); ++i) {
        if (cmds[i].name == open) ++depth;
        if (cmds[i].name == close && --depth == 0) return i;
    }
    throw std::runtime_error("CMakeLists.txt: " + open + "() at command " + std::to_string(at) +
                             " has no " + close + "()");
}

void add_sources(BuildModel& m, const std::string& target, const std::vector<std::string>& args, size_t first,
                 const std::map<std::string, std::vector<std::string>>& globs) {
    for (size_t i = first; i < args.size(); ++i) {
        const std::string a = strip_source_dir(args[i]);
        if (kTargetKeywords.count(a) || a == "PRIVATE" || a == "PUBLIC") continue;
        if (a.size() > 3 && a.rfind("${", 0) == 0 && a.back() == '}') {
            if (auto g = globs.find(a.substr(2, a.size() - 3)); g != globs.end())
                for (const std::string& pattern : g->second) m.glob_targets.emplace_back(pattern, target);
            continue;
        }
        if (unresolved(a)) continue;
        m.file_targets[a].insert(target);
    }
}

void add_by_mode(Links& into, const std::vector<std::string>& args, const std::set<std::string>& skip) {
    bool pub = true;  // no keyword: CMake passes the link on
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string a = strip_source_dir(args[i]);
        if (a == "PUBLIC" || a == "INTERFACE" || a == "LINK_PUBLIC") {
            pub = true;
        } else if (a == "PRIVATE" || a == "LINK_PRIVATE") {
            pub = false;
        } else if (!skip.count(a) && !unresolved(a)) {
            (pub ? into.pub : into.priv).push_back(a);
        }
    }
}

void apply(const std::vector<CMakeCommand>& cmds, size_t from, size_t to, BuildModel& m,
           std::map<std::string, std::vector<std::string>>& globs) {
    for (size_t i = from; i < to; ++i) {
        const CMakeCommand& c = cmds[i];
        if (c.name == "function" || c.name == "macro") {
            i = block_end(cmds, i, c.name, "end" + c.name);
            continue;
        }
        if (c.name == "foreach") {
            const size_t end = block_end(cmds, i, "foreach", "endforeach");
            std::vector<std::string> items;
            if (c.args.size() >= 3 && c.args[1] == "IN" && c.args[2] == "ITEMS")
                items.assign(c.args.begin() + 3, c.args.end());
            else if (c.args.size() >= 2 && c.args[1] != "IN")
                items.assign(c.args.begin() + 1, c.args.end());
            if (!c.args.empty()) {
                const std::string var = "${" + c.args[0] + "}";
                for (const std::string& item : items) {
                    std::vector<CMakeCommand> body(cmds.begin() + i + 1, cmds.begin() + end);
                    for (CMakeCommand& b : body)
                        for (std::string& a : b.args) a = replace_all(a, var, item);
                    apply(body, 0, body.size(), m, globs);
                }
                // A loop over a list variable cannot be read here; its body
                // still counts with the variable left unresolved.
                if (items.empty()) apply(cmds, i + 1, end, m, globs);
            }
            i = end;
            continue;
        }
        if (c.args.empty() || unresolved(c.args[0])) continue;
        if (c.name == "file" && c.args.size() >= 3 && (c.args[0] == "GLOB" || c.args[0] == "GLOB_RECURSE")) {
            auto& patterns = globs[c.args[1]];
            for (size_t a = 2; a < c.args.size(); ++a)
                if (c.args[a] != "CONFIGURE_DEPENDS") patterns.push_back(strip_source_dir(c.args[a]));
        } else if (c.name == "add_library" || c.name == "add_executable" || c.name == "target_sources") {
            add_sources(m, c.args[0], c.args, 1, globs);
            m.links[c.args[0]];
        } else if (c.name == "target_link_libraries") {
            add_by_mode(m.links[c.args[0]], c.args, {"debug", "optimized", "general"});
        } else if (c.name == "target_include_directories") {
            add_by_mode(m.include_dirs[c.args[0]], c.args, {"SYSTEM", "BEFORE", "AFTER"});
        }
    }
}

// Every `if()` branch counts as taken: if, else and endif are not read.
BuildModel read_build(const std::string& cmake_text) {
    const std::vector<CMakeCommand> cmds = read_commands(cmake_text);
    BuildModel m;
    std::map<std::string, std::vector<std::string>> globs;
    apply(cmds, 0, cmds.size(), m, globs);
    return m;
}

// ---- Externals ----

struct ExternalRow {
    std::string header;  // a pattern, `*` as in glob_match
    std::string target;  // the link line it stands for, or
    std::string folder;  // the include folder it sits in
    std::vector<std::string> must_match;
};

std::vector<ExternalRow> parse_externals(const std::string& text) {
    const json doc = json::parse(text);
    std::vector<ExternalRow> rows;
    for (const json& r : doc.at("externals")) {
        ExternalRow row;
        row.header = r.at("header").get<std::string>();
        if (r.contains("target")) row.target = r.at("target").get<std::string>();
        if (r.contains("include_dir")) row.folder = r.at("include_dir").get<std::string>();
        if (row.target.empty() == row.folder.empty())
            throw std::runtime_error("externals.json: row \"" + row.header +
                                     "\" needs exactly one of \"target\" and \"include_dir\"");
        for (const json& e : r.at("must_match")) row.must_match.push_back(e.get<std::string>());
        rows.push_back(std::move(row));
    }
    return rows;
}

// The first row whose pattern matches `include`, or null for a standard or
// Windows SDK header, which needs no link line.
const ExternalRow* external_for(const std::vector<ExternalRow>& rows, const std::string& include) {
    for (const ExternalRow& r : rows)
        if (glob_match(r.header, include)) return &r;
    return nullptr;
}

// ---- The include check ----

// The text of every `#include "x"` or `#include <x>` line in a file.
std::vector<std::string> includes_of(const std::string& text) {
    std::vector<std::string> out;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        size_t i = line.find_first_not_of(" \t");
        if (i == std::string::npos || line[i] != '#') continue;
        i = line.find_first_not_of(" \t", i + 1);
        if (i == std::string::npos || line.compare(i, 7, "include") != 0) continue;
        i = line.find_first_not_of(" \t", i + 7);
        if (i == std::string::npos || (line[i] != '"' && line[i] != '<')) continue;
        const size_t end = line.find(line[i] == '"' ? '"' : '>', i + 1);
        if (end != std::string::npos) out.push_back(line.substr(i + 1, end - i - 1));
    }
    return out;
}

// The repo file `include` names from `rel`, looked up in rel's own folder and
// then in `folders` (its library's include folders), or empty when it names
// none.
std::string resolve(const std::string& rel, const std::string& include, const std::set<std::string>& folders,
                    const std::set<std::string>& known) {
    std::vector<fs::path> tries{fs::path(rel).parent_path() / include};
    for (const std::string& root : folders) tries.push_back(fs::path(root) / include);
    for (const fs::path& p : tries) {
        const std::string s = p.lexically_normal().generic_string();
        if (known.count(s)) return s;
    }
    return {};
}

bool is_code(const std::string& rel) {
    const std::string ext = fs::path(rel).extension().string();
    return ext == ".h" || ext == ".hpp" || ext == ".cpp" || ext == ".c";
}

// Whether the include check reads `rel`: a file a target owns, under the
// folders a library or tool lives in, or listed in a source list by name.
bool scanned(const BuildModel& m, const std::string& rel) {
    if (!is_code(rel) || m.owners(rel).empty()) return false;
    for (const char* top : {"src/", "tools/", "tests/ui/"})
        if (rel.rfind(top, 0) == 0) return true;
    return m.file_targets.count(rel) > 0;
}

using Gap = std::pair<std::string, std::string>;  // (file, include text)

// One line per include that reaches past its library's links, skipping the
// listed gaps. `texts` holds the files to read, `known` every repo file.
std::vector<std::string> wrong_way_includes(const BuildModel& m, const std::map<std::string, std::string>& texts,
                                            const std::set<std::string>& known,
                                            const std::vector<ExternalRow>& externals,
                                            const std::set<Gap>& gaps) {
    std::vector<std::string> out;
    for (const auto& [rel, text] : texts) {
        if (!scanned(m, rel)) continue;
        for (const std::string& inc : includes_of(text)) {
            if (gaps.count({rel, inc})) continue;
            for (const std::string& lib : m.owners(rel)) {
                const std::set<std::string> allowed = m.allowed(lib);
                const std::set<std::string> folders = m.folders(lib);
                const std::string file = resolve(rel, inc, folders, known);
                if (!file.empty()) {
                    const std::set<std::string> owners = m.owners(file);
                    if (owners.empty()) continue;  // a test helper header, free to include
                    if (std::any_of(owners.begin(), owners.end(),
                                    [&](const std::string& t) { return allowed.count(t) > 0; }))
                        continue;
                    out.push_back(rel + ": include \"" + inc + "\" reaches target " + *owners.begin() +
                                  ", which " + lib + " does not link");
                    continue;
                }
                const ExternalRow* row = external_for(externals, inc);
                if (!row) continue;
                if (!row->target.empty() && !allowed.count(row->target))
                    out.push_back(rel + ": include \"" + inc + "\" reaches target " + row->target + ", which " +
                                  lib + " does not link");
                if (!row->folder.empty() && !folders.count(row->folder))
                    out.push_back(rel + ": include \"" + inc + "\" reaches include folder " + row->folder +
                                  ", which " + lib + " does not add");
            }
        }
    }
    return out;
}

// ---- The repo ----

std::string read_text(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot read " + path.u8string());
    std::ostringstream s;
    s << in.rdbuf();
    return s.str();
}

struct Repo {
    BuildModel build;
    std::set<std::string> known;
    std::map<std::string, std::string> texts;  // every code file the walk lists
    std::vector<ExternalRow> externals;
};

const Repo& repo() {
    static const Repo r = [] {
        Repo out;
        out.build = read_build(read_text(sourcetree::root() / "CMakeLists.txt"));
        sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
            out.known.insert(rel);
            if (is_code(rel)) out.texts[rel] = read_text(path);
        });
        out.externals = parse_externals(read_text(sourcetree::root() / "tests/layering/externals.json"));
        return out;
    }();
    return r;
}

struct KnownGap {
    std::string task;  // the task that removes it
    std::string file;
    std::string include;
    single_owner::BaselineDates dates;
};

std::vector<KnownGap> parse_gaps(const std::string& text) {
    const json doc = json::parse(text);
    std::vector<KnownGap> out;
    for (const auto& [task, entries] : doc.at("known_gaps").items())
        for (const json& e : entries) {
            KnownGap g{task, e.at("file").get<std::string>(), e.at("include").get<std::string>(), {}};
            g.dates = single_owner::parse_dates(e, "known_gaps.json: " + task + ": " + g.file + " includes \"" +
                                                       g.include + "\"");
            out.push_back(std::move(g));
        }
    return out;
}

const std::vector<KnownGap>& known_gaps() {
    static const std::vector<KnownGap> g =
        parse_gaps(read_text(sourcetree::root() / "tests/layering/known_gaps.json"));
    return g;
}

// Whether `name` is a key of `table` or sits in any of its lists.
bool names_anywhere(const std::map<std::string, Links>& table, const std::string& name) {
    for (const auto& [key, l] : table)
        if (key == name || std::count(l.pub.begin(), l.pub.end(), name) ||
            std::count(l.priv.begin(), l.priv.end(), name))
            return true;
    return false;
}

std::string joined(const std::vector<std::string>& lines) {
    std::string s;
    for (const std::string& l : lines) s += l + "\n";
    return s;
}

// A made-up build: three libraries in one PUBLIC chain and one PRIVATE link,
// a tool loop, a glob of tests and a link under an if().
const char* const kMadeUpCMake = R"(
# add_library(commented_out STATIC src/never.cpp)
add_library(base STATIC src/base/a.cpp src/base/only.h)
target_include_directories(base PUBLIC src PRIVATE third_party/json)
add_library(mid STATIC
    src/mid/b.cpp  # a comment after a source
)
target_link_libraries(mid PUBLIC base PRIVATE secret)
add_library(top STATIC src/top/c.cpp)
target_link_libraries(top PUBLIC mid)
add_executable(tool_a src/cli/a.cpp)
add_executable(tool_b src/cli/b.cpp)
foreach(tool IN ITEMS tool_a tool_b)
    target_link_libraries(${tool} PRIVATE base)
endforeach()
file(GLOB test_sources CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/test_*.cpp)
add_executable(tests ${test_sources} tests/helpers.cpp)
if(SOME_OPTION)
    target_link_libraries(tests PRIVATE top)
endif()
)";

}  // namespace

TEST_CASE("layering: the CMake reader, pinned on a made-up file") {
    const BuildModel m = read_build(kMadeUpCMake);

    CHECK(m.owners("src/base/a.cpp") == std::set<std::string>{"base"});
    CHECK(m.owners("src/base/a.h") == std::set<std::string>{"base"});  // the .cpp twin's
    CHECK(m.owners("src/base/only.h") == std::set<std::string>{"base"});
    CHECK(m.owners("src/mid/b.cpp") == std::set<std::string>{"mid"});
    CHECK(m.owners("src/never.cpp").empty());
    CHECK(m.owners("tests/test_one.cpp") == std::set<std::string>{"tests"});
    CHECK(m.owners("tests/sub/test_one.cpp").empty());  // a glob star stops at a folder
    CHECK(m.owners("tests/helpers.cpp") == std::set<std::string>{"tests"});

    CHECK(m.allowed("base") == std::set<std::string>{"base"});
    CHECK(m.allowed("mid") == std::set<std::string>{"mid", "base", "secret"});
    CHECK(m.allowed("top") == std::set<std::string>{"top", "mid", "base"});  // PRIVATE stops at mid
    CHECK(m.allowed("tool_a") == std::set<std::string>{"tool_a", "base"});
    CHECK(m.allowed("tool_b") == std::set<std::string>{"tool_b", "base"});
    CHECK(m.allowed("tests") == std::set<std::string>{"tests", "top", "mid", "base"});

    CHECK(m.folders("base") == std::set<std::string>{"src", "third_party/json"});
    CHECK(m.folders("mid") == std::set<std::string>{"src"});  // base's PRIVATE folder stays its own

    const std::vector<ExternalRow> externals{{"secret*.h", "secret", "", {"secret.h"}},
                                             {"json.hpp", "", "third_party/json", {"json.hpp"}}};
    const std::set<std::string> known{"src/base/a.cpp", "src/base/a.h",  "src/mid/b.cpp",
                                      "src/mid/b.h",    "src/top/c.cpp", "src/top/c.h"};
    const std::map<std::string, std::string> texts{
        {"src/base/a.cpp", "#include \"base/a.h\"\n#include <vector>\n#include \"json.hpp\"\n"},
        {"src/base/a.h", "#include \"mid/b.h\"\n"},  // the wrong way
        {"src/mid/b.cpp", "#include \"mid/b.h\"\n#include \"base/a.h\"\n  #  include <secret.h>\n"},
        {"src/mid/b.h", "#include \"json.hpp\"\n"},
        {"src/top/c.cpp", "#include \"top/c.h\"\n#include \"mid/b.h\"\n#include \"base/a.h\"\n"},
        {"src/top/c.h", "#include <secret.h>\n"},
    };
    CHECK(joined(wrong_way_includes(m, texts, known, externals, {})) ==
          "src/base/a.h: include \"mid/b.h\" reaches target mid, which base does not link\n"
          "src/mid/b.h: include \"json.hpp\" reaches include folder third_party/json, which mid does not add\n"
          "src/top/c.h: include \"secret.h\" reaches target secret, which top does not link\n");
    CHECK(wrong_way_includes(m, texts, known, externals,
                             {{"src/base/a.h", "mid/b.h"}, {"src/mid/b.h", "json.hpp"}, {"src/top/c.h", "secret.h"}})
              .empty());
}

TEST_CASE("layering: externals rows match their own examples") {
    const std::vector<ExternalRow>& rows = repo().externals;
    REQUIRE(!rows.empty());
    for (const ExternalRow& r : rows) {
        INFO("row \"", r.header, "\"");
        CHECK(!r.must_match.empty());
        for (const std::string& example : r.must_match) {
            INFO("example \"", example, "\"");
            const ExternalRow* first = external_for(rows, example);
            CHECK(first == &r);  // matched, and by this row, not an earlier one
        }
        // The folder or target the row names is one CMakeLists.txt names.
        if (!r.folder.empty())
            CHECK(names_anywhere(repo().build.include_dirs, r.folder));
        else
            CHECK(names_anywhere(repo().build.links, r.target));
    }
}

TEST_CASE("layering: every known gap names a real include and is not past its date") {
    const std::string today = single_owner::today_iso();
    std::vector<std::string> bad;
    for (const KnownGap& g : known_gaps()) {
        const std::string name = "known_gaps.json: " + g.task + ": " + g.file + " includes \"" + g.include + "\"";
        auto it = repo().texts.find(g.file);
        const bool real = it != repo().texts.end() && [&] {
            const std::vector<std::string> incs = includes_of(it->second);
            return std::find(incs.begin(), incs.end(), g.include) != incs.end();
        }();
        if (!real) bad.push_back(name + ": no such include line; delete the entry");
        if (const std::string due = single_owner::baseline_due(name, g.dates, today); !due.empty())
            bad.push_back(due);
    }
    INFO(joined(bad));
    CHECK(bad.empty());
}

TEST_CASE("layering: every src file is in one library") {
    std::vector<std::string> bad;
    for (const auto& [rel, text] : repo().texts)
        if (rel.rfind("src/", 0) == 0 && repo().build.owners(rel).empty())
            bad.push_back(rel + ": in no library's source list");
    INFO(joined(bad));
    CHECK(bad.empty());
}

TEST_CASE("layering: every include stays inside its library's links") {
    std::set<Gap> gaps;
    for (const KnownGap& g : known_gaps()) gaps.insert({g.file, g.include});
    const std::vector<std::string> bad =
        wrong_way_includes(repo().build, repo().texts, repo().known, repo().externals, gaps);
    INFO(joined(bad));
    CHECK(bad.empty());
}
