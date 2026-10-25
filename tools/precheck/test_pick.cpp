#include "test_pick.h"

#include <algorithm>
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

private:
    Repo& repo_;
    ViewCache& views_;
    std::string rev_;
    bool listed_ = false, built_ = false;
    std::vector<std::string> files_;
    std::set<std::string> set_;
    std::map<std::string, std::vector<std::string>> by_header_;

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
                if (!header.empty() && header != f && seen.insert(header).second) by_header_[header].push_back(f);
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
    for (size_t h : word_hits(span, name)) {
        const size_t after = skip_space(span, h + name.size());
        if (after < span.size() && span[after] == '(') return true;
    }
    return false;
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

    void add_name(const ChangedName& c, const std::string& file, bool from_base) {
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

    // Step 4: one level of callers in src/, with the A::B values on the call
    // line or on its nearest guard line in the same block.
    void collect_callers() {
        std::vector<std::string> src;
        for (const std::string& f : tip_includes_.files())
            if (top_folder(f) == "src" && is_cpp(f)) src.push_back(f);
        const size_t changed = out_.names.size();
        for (size_t i = 0; i < changed; ++i) {
            const std::string callee = out_.names[i].name;
            if (callee.find("::") != std::string::npos) continue;
            // A function changed only by the comment above it behaves as before.
            if (out_.names[i].kind == NameKind::Comment) continue;
            for (const std::string& f : src) {
                const View& view = views_.get(range_.tip, f);
                const std::vector<size_t> hits = word_hits(view.code, callee);
                if (hits.empty()) continue;
                const Braces braces(view.code);
                for (size_t h : hits) {
                    const size_t after = skip_space(view.code, h + callee.size());
                    if (after >= view.code.size() || view.code[after] != '(') continue;
                    call_site(view, braces, h, callee);
                }
            }
        }
    }

    void call_site(const View& view, const Braces& braces, size_t at, const std::string& callee) {
        std::vector<std::string> callers;
        for (const ChangedName& c : names_at(view, at)) {
            if (c.name == callee && c.kind != NameKind::Body) return;  // its own definition or declaration
            if (c.kind == NameKind::Body && c.name != callee) callers.push_back(c.name);
        }
        if (callers.empty()) return;
        for (const std::string& c : callers) add_caller(c, callee, c, false);
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
        for (const std::string& v : values) add_caller(v, callee, callers[0], true);
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
        std::vector<std::string> cpps, headers;
        for (const std::string& f : tip_includes_.files()) {
            if (runnable_test(f)) cpps.push_back(f);
            else if (helper_header(f)) headers.push_back(f);
        }
        auto mentions = [&](const std::vector<std::string>& files, const std::string& key) {
            std::vector<std::string> out;
            for (const std::string& f : files)
                if (!word_hits(views_.get(range_.tip, f).code, key).empty()) out.push_back(f);
            return out;
        };
        for (const std::string& key : keys) {
            const std::vector<std::string> in_cpp = mentions(cpps, key);
            const int count = static_cast<int>(in_cpp.size());
            if (count > limits_.common_name) {
                out_.common.push_back({key, count});
                if (heads.count(key) && !out_.whole_suite)
                    out_.whole_suite = WholeSuite{out_.names[name_index_.at(key)].file, "common name " + key, count};
                continue;
            }
            for (const std::string& f : in_cpp) add_reason(f, key);
            for (const std::string& h : mentions(headers, key))
                for (const std::string& inc : tip_includes_.includers(h))
                    if (runnable_test(inc)) add_reason(inc, key);
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

}  // namespace precheck
