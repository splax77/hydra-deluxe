#include "test_pick.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>

#include "braces.h"
#include "core/strutil.h"
#include "definitions.h"  // word_char, ident_start
#include "text.h"         // skip_space
#include "views.h"

namespace precheck {

namespace {

using hydra::ends_with;
using hydra::starts_with;
using hydra::trim;

std::string file_name(const std::string& path) { return path.substr(path.rfind('/') + 1); }

// A test file hydra_tests compiles as its own source: a .cpp directly in
// tests/. Only these are picked, since -sf= selects by a test case's file;
// tests/single_owner/ and the ui folder are left out by this.
bool runnable_test(const std::string& file) {
    return starts_with(file, "tests/") && file.find('/', 6) == std::string::npos && ends_with(file, ".cpp");
}

// A test helper header: its code runs inside every test file that includes it.
bool helper_header(const std::string& file) {
    return starts_with(file, "tests/") && !starts_with(file, "tests/single_owner/") && is_cpp(file) &&
           !is_cpp_implementation(file);
}

// Every offset where `key` starts as a whole word in `text`.
std::vector<size_t> word_hits(const std::string& text, const std::string& key) {
    std::vector<size_t> out;
    if (key.empty()) return out;
    for (size_t i = text.find(key); i != std::string::npos; i = text.find(key, i + 1)) {
        const size_t e = i + key.size();
        if ((i == 0 || !word_char(text[i - 1])) && (e >= text.size() || !word_char(text[e]))) out.push_back(i);
    }
    return out;
}

// The word_hits of `name` that a "(" follows: calls, declarations and
// definitions of a function of that name.
std::vector<size_t> paren_hits(const std::string& text, const std::string& name) {
    std::vector<size_t> out;
    for (size_t h : word_hits(text, name)) {
        const size_t after = skip_space(text, h + name.size());
        if (after < text.size() && text[after] == '(') out.push_back(h);
    }
    return out;
}

// ------------------------------------------------------------ includes

// The folders an #include path is looked up under after the including
// file's own folder: the include directories CMakeLists.txt gives the
// targets whose code lives in src/, tools/ and tests/.
const char* const kIncludeRoots[] = {"src", "tools", "tests"};

// `path` with "." and ".." folders resolved; empty when ".." climbs out.
std::string normal_path(const std::string& path) {
    std::vector<std::string> parts;
    std::istringstream in(path);
    for (std::string part; std::getline(in, part, '/');) {
        if (part.empty() || part == ".") continue;
        if (part == "..") {
            if (parts.empty()) return "";
            parts.pop_back();
            continue;
        }
        parts.push_back(part);
    }
    std::string out;
    for (const std::string& p : parts) out += (out.empty() ? "" : "/") + p;
    return out;
}

// The path between the quotes or angle brackets of an #include line, or
// empty when the line is not one.
std::string include_target(const std::string& line) {
    size_t i = skip_space(line, 0);
    if (i >= line.size() || line[i] != '#') return "";
    i = skip_space(line, i + 1);
    static const std::string kWord = "include";
    if (line.compare(i, kWord.size(), kWord) != 0) return "";
    i = skip_space(line, i + kWord.size());
    if (i >= line.size() || (line[i] != '"' && line[i] != '<')) return "";
    const size_t close = line.find(line[i] == '"' ? '"' : '>', i + 1);
    return close == std::string::npos ? "" : line.substr(i + 1, close - i - 1);
}

// Which files under code_folders() include each file directly, at one
// revision. Built on first use: it reads every C++ file once.
class IncludeMap {
public:
    IncludeMap(Repo& repo, ViewCache& views, const std::string& rev) : repo_(repo), views_(views), rev_(rev) {}

    const std::vector<std::string>& files() {
        if (!listed_) {
            files_ = all_files(repo_, rev_);
            set_.insert(files_.begin(), files_.end());
            listed_ = true;
        }
        return files_;
    }
    bool has(const std::string& file) {
        files();
        return set_.count(file) != 0;
    }

    const std::vector<std::string>& includers(const std::string& header) {
        if (!built_) build();
        static const std::vector<std::string> kNone;
        const auto it = by_header_.find(header);
        return it == by_header_.end() ? kNone : it->second;
    }

    // The files `file` includes directly, as resolve() finds them.
    const std::vector<std::string>& included_by(const std::string& file) {
        if (!built_) build();
        static const std::vector<std::string> kNone;
        const auto it = by_file_.find(file);
        return it == by_file_.end() ? kNone : it->second;
    }

private:
    Repo& repo_;
    ViewCache& views_;
    std::string rev_;
    bool listed_ = false, built_ = false;
    std::vector<std::string> files_;
    std::set<std::string> set_;
    std::map<std::string, std::vector<std::string>> by_header_, by_file_;

    // The file an include of `spelled` in `from` reads, as the compiler
    // finds it: the including file's folder first, then each root.
    std::string resolve(const std::string& from, const std::string& spelled) {
        const size_t slash = from.rfind('/');
        std::vector<std::string> tries = {normal_path((slash == std::string::npos ? "" : from.substr(0, slash + 1)) +
                                                      spelled)};
        for (const char* root : kIncludeRoots) tries.push_back(normal_path(std::string(root) + "/" + spelled));
        for (const std::string& t : tries)
            if (!t.empty() && set_.count(t)) return t;
        return "";
    }

    void build() {
        for (const std::string& f : files()) {
            if (!is_cpp(f)) continue;
            const View& v = views_.get(rev_, f);
            std::set<std::string> seen;
            for (int n = 1; n <= v.line_count(); ++n) {
                const std::string target = include_target(v.line(v.no_comments, n));
                if (target.empty()) continue;
                const std::string header = resolve(f, target);
                if (!header.empty() && header != f && seen.insert(header).second) {
                    by_header_[header].push_back(f);
                    by_file_[f].push_back(header);
                }
            }
        }
        built_ = true;
    }
};

// The shared-code rule itself; shared_code() is its public face.
std::optional<WholeSuite> shared_code_in(const std::string& file, IncludeMap& includes, int limit) {
    if (file_name(file) == "CMakeLists.txt" || file == "CMakePresets.json" || starts_with(file, "cmake/") ||
        file == "build_cpp.ps1" || starts_with(file, ".github/workflows/"))
        return WholeSuite{file, "build file", 0};
    if (file == "tests/test_main.cpp" || file == "tests/shards.txt" || starts_with(file, "third_party/doctest/"))
        return WholeSuite{file, "test harness", 0};
    if (is_cpp(file) && !is_cpp_implementation(file)) {
        const int count = static_cast<int>(includes.includers(file).size());
        if (count > limit)
            return WholeSuite{file, "header with more direct includers than " + std::to_string(limit), count};
    }
    return std::nullopt;
}

// --------------------------------------------------------------- names

// Whether the declaration starting on line n (the next one, for a comment
// line) calls `name` as a function: `name(` before its first ";" or "{".
bool declares_function(const View& view, int n, const std::string& name) {
    const size_t b = view.line_starts[n - 1];
    const size_t stop = view.code.find_first_of(";{", skip_space(view.code, b));
    const std::string span = view.code.substr(b, stop == std::string::npos ? std::string::npos : stop - b);
    return !paren_hits(span, name).empty();
}

// The names changed_names gives for code starting at offset `at`, as if a
// line began there. changed_names reads whole lines, so a call that shares
// its line with the function's head (a one-line body) is read from a copy
// of the masked text with a line break put before it.
std::vector<ChangedName> names_at(const View& view, size_t at) {
    View w;
    w.code = view.code.substr(0, at) + "\n" + view.code.substr(at);
    w.raw = w.code;
    w.no_comments = w.code;
    w.line_starts.push_back(0);
    for (size_t i = 0; i < w.code.size(); ++i)
        if (w.code[i] == '\n') w.line_starts.push_back(i + 1);
    const Braces braces(w.code);
    return changed_names(w, braces, {w.line_of(at + 1)});
}

// The A::B values in a masked line, std:: left out. A longer chain gives its
// last two parts, the way a test usually spells an enum value.
std::vector<std::string> selector_values(const std::string& line) {
    std::vector<std::string> out;
    size_t i = 0;
    while (i < line.size()) {
        if (!ident_start(line[i]) || (i > 0 && (word_char(line[i - 1]) || line[i - 1] == ':'))) {
            ++i;
            continue;
        }
        std::vector<std::string> parts;
        size_t j = i;
        for (;;) {
            size_t e = j;
            while (e < line.size() && word_char(line[e])) ++e;
            parts.push_back(line.substr(j, e - j));
            const size_t colon = skip_space(line, e);
            if (line.compare(colon, 2, "::") != 0) {
                j = e;
                break;
            }
            const size_t next = skip_space(line, colon + 2);
            if (next >= line.size() || !ident_start(line[next])) {
                j = e;
                break;
            }
            j = next;
        }
        if (parts.size() >= 2 && parts[0] != "std") {
            const std::string v = parts[parts.size() - 2] + "::" + parts.back();
            if (std::find(out.begin(), out.end(), v) == out.end()) out.push_back(v);
        }
        i = std::max(j, i + 1);
    }
    return out;
}

// Whether masked `code` defines the type `type`: an enum, class, struct or
// union head naming it that a "{" follows before any ";". A forward
// declaration does not count, so the walk below keeps looking for the body.
bool declares_type(const std::string& code, const std::string& type) {
    static const std::set<std::string> kHeads = {"enum", "class", "struct", "union"};
    for (size_t h : word_hits(code, type)) {
        size_t b = h;
        while (b > 0 && (code[b - 1] == ' ' || code[b - 1] == '\t' || code[b - 1] == '\n' || code[b - 1] == '\r')) --b;
        size_t w = b;
        while (w > 0 && word_char(code[w - 1])) --w;
        if (!kHeads.count(code.substr(w, b - w))) continue;
        const size_t stop = code.find_first_of(";{", h + type.size());
        if (stop != std::string::npos && code[stop] == '{') return true;
    }
    return false;
}

// Whether masked `code` defines a program's entry point: `int main(` or
// `int wmain(` at file level.
bool defines_main(const std::string& code) {
    const Braces braces(code);
    for (const char* entry : {"main", "wmain"})
        for (size_t h : paren_hits(code, entry)) {
            if (braces.innermost(h) != Braces::kTop) continue;
            size_t b = h;
            while (b > 0 && (code[b - 1] == ' ' || code[b - 1] == '\t' || code[b - 1] == '\n')) --b;
            if (b >= 3 && code.compare(b - 3, 3, "int") == 0 && (b == 3 || !word_char(code[b - 4]))) return true;
        }
    return false;
}

// ----------------------------------------------------------------- exes

// CMakeLists.txt text with its # comments blanked.
std::string cmake_code(const std::string& text) {
    std::string out = text;
    for (size_t i = out.find('#'); i != std::string::npos; i = out.find('#', i)) {
        const size_t end = out.find('\n', i);
        const size_t stop = end == std::string::npos ? out.size() : end;
        std::fill(out.begin() + static_cast<std::ptrdiff_t>(i), out.begin() + static_cast<std::ptrdiff_t>(stop), ' ');
        i = stop;
    }
    return out;
}

// The targets whose add_executable(...) in `cmake` lists `file`.
std::vector<std::string> exe_targets_listing(const std::string& cmake, const std::string& file) {
    std::vector<std::string> out;
    for (size_t h : word_hits(cmake, "add_executable")) {
        const size_t open = skip_space(cmake, h + std::string("add_executable").size());
        if (open >= cmake.size() || cmake[open] != '(') continue;
        const size_t close = cmake.find(')', open);
        if (close == std::string::npos) continue;
        std::istringstream words(cmake.substr(open + 1, close - open - 1));
        std::string target;
        if (!(words >> target)) continue;
        for (std::string w; words >> w;)
            if (w == file) {
                out.push_back(target);
                break;
            }
    }
    return out;
}

// The compile definition naming an exe target's path in the tests: the
// user's HYDRA_<TARGET>_EXE (plan, "User decisions", last paragraph).
std::string exe_constant(const std::string& target) {
    static const std::string kPrefix = "hydra_";
    std::string name = starts_with(target, kPrefix) ? target.substr(kPrefix.size()) : target;
    for (char& c : name) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return "HYDRA_" + name + "_EXE";
}

// Whether `cmake` defines `constant` as a value: `constant=`.
bool defines_constant(const std::string& cmake, const std::string& constant) {
    for (size_t h : word_hits(cmake, constant))
        if (h + constant.size() < cmake.size() && cmake[h + constant.size()] == '=') return true;
    return false;
}

// Whether a masked line opens with an if, else if or case.
bool guard_line(const std::string& line) {
    std::string t = trim(line);
    while (!t.empty() && t[0] == '}') t = trim(t.substr(1));
    auto word = [&](const std::string& w) {
        return starts_with(t, w) && (t.size() == w.size() || !word_char(t[w.size()]));
    };
    static const std::string kElse = "else";
    if (word(kElse)) t = trim(t.substr(kElse.size()));
    return word("if") || word("case");
}

class Picker {
public:
    Picker(Repo& repo, const Range& range, const PickLimits& limits)
        : repo_(repo), range_(range), limits_(limits), views_(repo), tip_includes_(repo, views_, range.tip) {}

    TestPick run() {
        const std::vector<std::string> changed =
            repo_.git({"diff", "--name-only", "--no-color", "--no-renames", range_.base, range_.tip});
        // Step 2: shared code decides before any name work.
        for (const std::string& f : changed)
            if (std::optional<WholeSuite> w = shared_code_in(f, tip_includes_, limits_.header_includers)) {
                out_.whole_suite = w;
                return out_;
            }
        collect_names();
        collect_callers();
        for (const std::string& f : changed) {
            if (!tip_includes_.has(f)) continue;  // deleted at the tip
            if (runnable_test(f)) add_reason(f, "changed in range");
            if (helper_header(f))
                for (const std::string& inc : tip_includes_.includers(f))
                    if (runnable_test(inc)) add_reason(inc, f + " changed in range");
        }
        grep_tests();
        if (!out_.whole_suite && static_cast<int>(picks_.size()) > limits_.pick_size)
            out_.whole_suite =
                WholeSuite{"", "more picked test files than " + std::to_string(limits_.pick_size),
                           static_cast<int>(picks_.size())};
        if (!out_.whole_suite)
            for (const auto& [file, reasons] : picks_) out_.tests.push_back({file, reasons});
        return out_;
    }

private:
    Repo& repo_;
    const Range& range_;
    PickLimits limits_;
    ViewCache views_;
    IncludeMap tip_includes_;
    TestPick out_;
    std::map<std::string, size_t> name_index_;
    std::map<std::string, std::vector<std::string>> picks_;
    // Every production file each changed name came from, either side.
    std::map<std::string, std::set<std::string>> name_files_;
    // Every file a caller key's call site sat in, and whether the key was a
    // selector value there.
    std::map<std::string, std::set<std::pair<std::string, bool>>> key_sites_;

    // An exe target and the test files that run it.
    struct ExeTests {
        std::string target;
        std::vector<std::string> tests;
    };
    std::optional<std::string> cmake_;  // CMakeLists.txt at the tip, comments blanked
    std::map<std::string, std::optional<ExeTests>> exe_of_;

    // The runnable test files that name a word as a whole word in masked
    // code: directly, or by including a test helper header that names it.
    struct Naming {
        std::vector<std::string> direct;
        std::vector<std::string> via_helper;  // in helper-header order; may repeat
    };
    Naming naming(const std::string& word) {
        Naming out;
        for (const std::string& f : tip_includes_.files()) {
            const bool cpp = runnable_test(f);
            if (!cpp && !helper_header(f)) continue;
            if (word_hits(views_.get(range_.tip, f).code, word).empty()) continue;
            if (cpp) out.direct.push_back(f);
            else
                for (const std::string& inc : tip_includes_.includers(f))
                    if (runnable_test(inc)) out.via_helper.push_back(inc);
        }
        return out;
    }

    // naming(), both ways, as one sorted list.
    std::vector<std::string> tests_naming(const std::string& word) {
        const Naming n = naming(word);
        std::set<std::string> all(n.direct.begin(), n.direct.end());
        all.insert(n.via_helper.begin(), n.via_helper.end());
        return {all.begin(), all.end()};
    }

    // The exe a main file builds and the tests that run it (the user's ruling
    // in the plan's "User decisions", last paragraph): the first target in
    // CMakeLists.txt listing the file (and no other target), when its constant
    // is defined there and some test names it. Nothing for any other file.
    const ExeTests* exe_for(const std::string& file) {
        const auto memo = exe_of_.find(file);
        if (memo != exe_of_.end()) return memo->second ? &*memo->second : nullptr;
        std::optional<ExeTests>& slot = exe_of_[file];
        if (!tip_includes_.has(file) || !is_cpp_implementation(file) ||
            !defines_main(views_.get(range_.tip, file).code))
            return nullptr;
        if (!cmake_) {
            const std::optional<std::string> raw = repo_.files().raw(range_.tip, "CMakeLists.txt");
            cmake_ = raw ? cmake_code(tip_text_form(*raw)) : std::string();
        }
        const std::vector<std::string> targets = exe_targets_listing(*cmake_, file);
        if (targets.size() != 1) return nullptr;  // two exes share the file: their tests are not told apart
        const std::string constant = exe_constant(targets.front());
        if (!defines_constant(*cmake_, constant)) return nullptr;
        std::vector<std::string> tests = tests_naming(constant);
        if (tests.empty()) return nullptr;
        slot = ExeTests{targets.front(), tests};
        return &*slot;
    }

    // The exes behind `key`: one for each file the key is defined in that
    // exe_for finds an exe for.
    std::vector<const ExeTests*> key_exes(const std::string& key) {
        std::vector<const ExeTests*> out;
        auto add = [&](const std::string& f) {
            const ExeTests* e = exe_for(f);
            if (e && std::find(out.begin(), out.end(), e) == out.end()) out.push_back(e);
        };
        const auto named = name_index_.find(key);
        if (named != name_index_.end() && !out_.names[named->second].base_only)
            for (const std::string& f : name_files_[key]) add(f);
        for (const auto& [file, selector] : key_sites_[key])
            if (!selector) add(file);
        return out;
    }

    void add_name(const ChangedName& c, const std::string& file, bool from_base) {
        name_files_[c.name].insert(file);
        const auto it = name_index_.find(c.name);
        if (it != name_index_.end()) {
            PickName& n = out_.names[it->second];
            n.kind = std::min(n.kind, c.kind);
            n.base_only = n.base_only && from_base;
            return;
        }
        name_index_[c.name] = out_.names.size();
        out_.names.push_back({c.name, c.kind, file, from_base});
    }

    // Step 3 on one side of one file. A function declared inside a block
    // (a member of a type) is not a key of its own: the type is.
    void names_of(const std::string& rev, const std::string& file, const std::vector<int>& lines, bool from_base) {
        const View& view = views_.get(rev, file);
        const Braces braces(view.code);
        for (int n : lines) {
            const std::vector<ChangedName> got = changed_names(view, braces, {n});
            const bool enclosed =
                std::any_of(got.begin(), got.end(), [](const ChangedName& c) { return c.kind == NameKind::Body; });
            for (const ChangedName& c : got) {
                if (c.member) continue;  // an inline member's body: its type is the key
                if (enclosed && c.kind != NameKind::Body && c.kind != NameKind::Value &&
                    declares_function(view, n, c.name))
                    continue;
                add_name(c, file, from_base);
            }
        }
    }

    void collect_names() {
        const ChangedLines lines = ChangedLines::from_diff(repo_, range_.base, range_.tip);
        for (const std::string& f : lines.added.files()) {
            if (test_file(f) || !is_cpp(f)) continue;
            const View& view = views_.get(range_.tip, f);
            names_of(range_.tip, f, lines.added.lines(f, view.line_count()), false);
        }
        for (const auto& [f, set] : lines.deleted) {
            if (test_file(f) || !is_cpp(f) || set.empty()) continue;
            names_of(range_.base, f, std::vector<int>(set.begin(), set.end()), true);
        }
    }

    void add_caller(const std::string& key, const std::string& callee, const std::string& caller, bool selector) {
        for (const CallerKey& k : out_.callers)
            if (k.key == key && k.callee == callee && k.caller == caller) return;
        out_.callers.push_back({key, callee, caller, selector});
    }

    // The headers whose masked code `declares` that `file` includes directly
    // or, when none of those does, the nearest ones further down its include
    // chain: a .cpp need not include the header that declares what it
    // defines. The seen set stops include cycles.
    template <typename Declares>
    std::vector<std::string> declaring_headers(const std::string& file, const Declares& declares) {
        std::vector<std::string> found;
        std::set<std::string> seen;
        std::vector<std::string> level = tip_includes_.included_by(file);
        while (found.empty() && !level.empty()) {
            std::vector<std::string> next;
            for (const std::string& h : level) {
                if (!seen.insert(h).second) continue;
                if (!is_cpp(h) || is_cpp_implementation(h)) continue;
                if (declares(views_.get(range_.tip, h).code)) found.push_back(h);
                for (const std::string& inc : tip_includes_.included_by(h)) next.push_back(inc);
            }
            level = next;
        }
        return found;
    }

    // declaring_headers for a function: a header with `name(` in it.
    std::vector<std::string> function_headers(const std::string& file, const std::string& name) {
        return declaring_headers(file, [&](const std::string& code) { return !paren_hits(code, name).empty(); });
    }

    // The headers that declare `name` as defined in `file`: the file itself
    // when it is a header, else function_headers. Nothing when the file is
    // gone at the tip, defines main (the user's exception: tests reach such
    // a file by running its exe; exe_for picks those tests when it can), or
    // reaches no header declaring the name.
    std::optional<std::vector<std::string>> definer_headers(const std::string& file, const std::string& name) {
        if (!tip_includes_.has(file)) return std::nullopt;
        if (!is_cpp_implementation(file)) return std::vector<std::string>{file};
        if (defines_main(views_.get(range_.tip, file).code)) return std::nullopt;
        std::vector<std::string> found = function_headers(file, name);
        if (found.empty()) return std::nullopt;
        return found;
    }

    // `headers` and every file that includes one of them, through any chain
    // of includes.
    std::set<std::string> reaching(std::vector<std::string> todo) {
        std::set<std::string> out;
        std::set<std::string> seen(todo.begin(), todo.end());
        while (!todo.empty()) {
            const std::string h = todo.back();
            todo.pop_back();
            out.insert(h);
            for (const std::string& inc : tip_includes_.includers(h))
                if (seen.insert(inc).second) todo.push_back(inc);
        }
        return out;
    }

    // The files whose calls to `callee` can reach the changed function (the
    // user's ruling in the plan's "User decisions"): the files it changed in,
    // and every file that includes a header declaring it, through any chain
    // of includes. A name changed only in a .cpp has no declaring header, so
    // only its own file counts. A declaring header is a changed header
    // itself, or one function_headers finds for a changed .cpp.
    std::set<std::string> caller_scope(const std::string& callee) {
        const std::set<std::string>& own = name_files_[callee];
        std::vector<std::string> todo;
        for (const std::string& f : own) {
            if (!is_cpp_implementation(f)) todo.push_back(f);
            else
                for (const std::string& h : function_headers(f, callee)) todo.push_back(h);
        }
        std::set<std::string> scope = reaching(todo);
        scope.insert(own.begin(), own.end());
        return scope;
    }

    // The files a test must reach for its mention of `key` to count (the
    // user's Step 5 ruling in the plan's "User decisions"): every file that
    // reaches a header declaring the key, wherever the key came from. Nothing
    // means the plain-name search: a deleted name, or any source of the key
    // with no declaring header found (see definer_headers). A source that
    // exe_for finds an exe for adds no header: key_exes picks its tests.
    std::optional<std::set<std::string>> test_scope(const std::string& key) {
        std::vector<std::string> headers;
        bool exe = false;
        auto add = [&](const std::string& f) {
            if (exe_for(f)) {
                exe = true;
                return true;
            }
            const std::optional<std::vector<std::string>> hs = definer_headers(f, key);
            if (!hs) return false;
            headers.insert(headers.end(), hs->begin(), hs->end());
            return true;
        };
        const auto named = name_index_.find(key);
        if (named != name_index_.end()) {
            if (out_.names[named->second].base_only) return std::nullopt;
            for (const std::string& f : name_files_[key])
                if (!add(f)) return std::nullopt;
        }
        for (const auto& [file, selector] : key_sites_[key]) {
            if (!selector) {
                if (!add(file)) return std::nullopt;
                continue;
            }
            // A selector value is declared where its type is.
            const std::string type = key.substr(0, key.find("::"));
            auto declares = [&](const std::string& code) { return declares_type(code, type); };
            const std::string& code = views_.get(range_.tip, file).code;
            if (declares(code)) {
                if (is_cpp_implementation(file)) return std::nullopt;  // a type local to the .cpp
                headers.push_back(file);
                continue;
            }
            const std::vector<std::string> found = declaring_headers(file, declares);
            if (found.empty()) return std::nullopt;
            headers.insert(headers.end(), found.begin(), found.end());
        }
        if (headers.empty()) return exe ? std::optional<std::set<std::string>>(std::set<std::string>{}) : std::nullopt;
        return reaching(headers);
    }

    // Step 4: one level of callers in production code under the include
    // roots, with the A::B values on the call line or on its nearest guard
    // line in the same block.
    void collect_callers() {
        std::set<std::string> production;
        for (const std::string& f : tip_includes_.files()) {
            const std::string top = top_folder(f);
            const bool rooted = std::any_of(std::begin(kIncludeRoots), std::end(kIncludeRoots),
                                            [&](const char* r) { return top == r; });
            if (rooted && !test_file(f) && is_cpp(f)) production.insert(f);
        }
        const size_t changed = out_.names.size();
        for (size_t i = 0; i < changed; ++i) {
            const std::string callee = out_.names[i].name;
            if (callee.find("::") != std::string::npos) continue;
            // A function changed only by the comment above it behaves as before.
            if (out_.names[i].kind == NameKind::Comment) continue;
            for (const std::string& f : caller_scope(callee)) {
                if (!production.count(f)) continue;
                const View& view = views_.get(range_.tip, f);
                const std::vector<size_t> hits = paren_hits(view.code, callee);
                if (hits.empty()) continue;
                const Braces braces(view.code);
                for (size_t h : hits) call_site(f, view, braces, h, callee);
            }
        }
    }

    void call_site(const std::string& file, const View& view, const Braces& braces, size_t at,
                   const std::string& callee) {
        std::vector<std::string> callers;
        for (const ChangedName& c : names_at(view, at)) {
            if (c.name == callee && c.kind != NameKind::Body) return;  // its own definition or declaration
            if (c.kind == NameKind::Body && c.name != callee) callers.push_back(c.name);
        }
        if (callers.empty()) return;
        for (const std::string& c : callers) {
            add_caller(c, callee, c, false);
            key_sites_[c].insert({file, false});
        }
        const int line = view.line_of(at);
        std::vector<std::string> values = selector_values(view.line(view.code, line));
        const size_t open = braces.innermost(at);
        if (open != Braces::kTop) {
            const int top = view.line_of(open);
            for (int m = line - 1; m >= top; --m) {
                const size_t first = skip_space(view.code, view.line_starts[m - 1]);
                if (m != top && braces.innermost(first) != open) continue;
                const std::string text = view.line(view.code, m);
                if (!guard_line(text)) continue;
                for (const std::string& v : selector_values(text))
                    if (std::find(values.begin(), values.end(), v) == values.end()) values.push_back(v);
                break;
            }
        }
        for (const std::string& v : values) {
            add_caller(v, callee, callers[0], true);
            key_sites_[v].insert({file, true});
        }
    }

    void add_reason(const std::string& file, const std::string& reason) {
        std::vector<std::string>& r = picks_[file];
        if (std::find(r.begin(), r.end(), reason) == r.end()) r.push_back(reason);
    }

    // Steps 5 and 6: each key against the masked test files.
    void grep_tests() {
        std::vector<std::string> keys;
        std::set<std::string> heads;
        for (const PickName& n : out_.names) {
            keys.push_back(n.name);
            heads.insert(n.name);
        }
        for (const CallerKey& k : out_.callers)
            if (std::find(keys.begin(), keys.end(), k.key) == keys.end()) keys.push_back(k.key);
        for (const std::string& key : keys) {
            const Naming named = naming(key);
            const std::vector<std::string>& in_cpp = named.direct;
            const int count = static_cast<int>(in_cpp.size());
            if (count > limits_.common_name) {
                out_.common.push_back({key, count});
                if (heads.count(key) && !out_.whole_suite)
                    out_.whole_suite = WholeSuite{out_.names[name_index_.at(key)].file, "common name " + key, count};
                continue;
            }
            // The count above stays over every mention; only the picks need
            // the test to reach the key's declaring header.
            const std::optional<std::set<std::string>> scope = test_scope(key);
            auto reaches = [&](const std::string& f) { return !scope || scope->count(f) != 0; };
            for (const std::string& f : in_cpp)
                if (reaches(f)) add_reason(f, key);
            for (const std::string& inc : named.via_helper)
                if (reaches(inc)) add_reason(inc, key);
            for (const ExeTests* e : key_exes(key))
                for (const std::string& f : e->tests) add_reason(f, key + " (runs " + e->target + ")");
        }
    }
};

}  // namespace

std::optional<WholeSuite> shared_code(Repo& repo, const std::string& rev, const std::string& file,
                                      int header_includer_limit) {
    ViewCache views(repo);
    IncludeMap includes(repo, views, repo.resolve_commit(rev));
    return shared_code_in(file, includes, header_includer_limit);
}

TestPick pick_tests(Repo& repo, const Range& range, const PickLimits& limits) {
    if (range.base.empty())
        throw GitError("the test picker needs a range with a base; " + range.spelling + " has none");
    return Picker(repo, range, limits).run();
}

std::vector<std::string> pick_lines(const TestPick& pick) {
    // A `*name.cpp` pattern names one file: a pattern ending in a bare `*`
    // would also match every file that starts with the same name.
    std::vector<PickedTest> tests = pick.tests;
    auto file_name = [](const std::string& f) { return f.substr(f.rfind('/') + 1); };
    std::sort(tests.begin(), tests.end(), [&](const PickedTest& x, const PickedTest& y) {
        return file_name(x.file) != file_name(y.file) ? file_name(x.file) < file_name(y.file) : x.file < y.file;
    });

    std::vector<std::string> lines;
    std::string first = "tests: ";
    if (pick.whole_suite) first += "whole suite";
    else if (tests.empty()) first += "(none)";  // an empty -sf= filter would run every test case
    else {
        first += "-sf=";
        std::string last;
        for (const PickedTest& t : tests) {
            const std::string pattern = "*" + file_name(t.file);
            if (pattern == last) continue;
            if (!last.empty()) first += ",";
            first += pattern;
            last = pattern;
        }
    }
    lines.push_back(first);
    // The three repo-wide checks of CLAUDE.md rule 1, run at every merge.
    lines.push_back("always: \"-tc=docs match code*\" \"-sf=*single_owner*\" \"-sf=*test_precheck*\"");

    std::string names = "names:";
    for (const PickName& n : pick.names)
        names += " " + n.name + " (" + (n.base_only ? "deleted" : kind_word(n.kind)) + ")";
    lines.push_back(pick.names.empty() ? "names: (none)" : names);

    std::string callers = "callers: ";
    for (size_t i = 0; i < pick.callers.size(); ++i) {
        const CallerKey& k = pick.callers[i];
        if (i) callers += "; ";
        callers += k.selector ? k.key + " selects " + k.callee + " in " + k.caller : k.key + " <- " + k.callee;
    }
    lines.push_back(pick.callers.empty() ? "callers: (none)" : callers);

    std::string common = "common:";
    for (const CommonName& c : pick.common) common += " " + c.name + " (" + std::to_string(c.files) + " files)";
    lines.push_back(pick.common.empty() ? "common: (none)" : common);

    if (pick.whole_suite) {
        const WholeSuite& w = *pick.whole_suite;
        std::string why = "why: ";
        if (!w.file.empty()) why += w.file + "  ";
        why += w.rule;
        if (w.count) why += " (" + std::to_string(w.count) + ")";
        lines.push_back(why);
    } else if (tests.empty()) {
        lines.push_back("why: no changed code names a test");
    } else {
        for (const PickedTest& t : tests) {
            std::string why = "why: " + t.file + " ";
            for (size_t i = 0; i < t.reasons.size(); ++i) why += (i ? "; " : " ") + t.reasons[i];
            lines.push_back(why);
        }
    }
    return lines;
}

}  // namespace precheck
