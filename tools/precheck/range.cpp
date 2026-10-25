#include "range.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <sstream>

#include "code_lexer.h"
#include "core/strutil.h"
#include "single_owner/listed.h"  // is_cpp_source
#include "source_tree.h"          // kWalkedFolders

namespace precheck {

namespace {

using hydra::ends_with;
using hydra::starts_with;
using hydra::to_lower_ascii;

bool all_digits(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; });
}

// One side of "A...B" or "A..B" as a name; as in git, an empty side means HEAD.
std::string range_side(const std::string& side) { return side.empty() ? "HEAD" : side; }

// Where `tip` forked from `left`: the base of "left...tip".
std::string fork_point(const Repo& repo, const std::string& left, const std::string& tip) {
    const std::vector<std::string> mb = repo.git({"merge-base", left, tip});
    if (mb.empty()) throw GitError("git merge-base " + left + " " + tip + " printed nothing");
    return mb[0];
}

// The start line and line count of one side of a hunk header ("+12,3" or "+12").
bool hunk_side(const std::string& field, char sign, long long& start, long long& count) {
    if (field.size() < 2 || field[0] != sign) return false;
    const size_t comma = field.find(',');
    const std::string a = field.substr(1, comma == std::string::npos ? std::string::npos : comma - 1);
    const std::string b = comma == std::string::npos ? "1" : field.substr(comma + 1);
    if (!all_digits(a) || !all_digits(b)) return false;
    start = std::stoll(a);
    count = std::stoll(b);
    return true;
}

}  // namespace

// ------------------------------------------------------------- file kinds

std::string top_folder(const std::string& file) {
    const size_t i = file.find('/');
    return i == std::string::npos ? "" : file.substr(0, i);
}

bool is_cpp(const std::string& file) {
    return single_owner::is_cpp_source(std::filesystem::u8path(file));
}

bool is_cpp_implementation(const std::string& file) { return is_cpp(file) && !ends_with(file, ".h"); }

bool is_py(const std::string& file) {
    const std::optional<codelex::Lang> lang =
        codelex::language_of_extension(std::filesystem::u8path(file).extension().string());
    return lang && *lang == codelex::Lang::Python;
}

bool test_file(const std::string& file) {
    // Everything under tests/; under tools/, a file in a tests folder at any
    // depth, or a Python file whose name starts test_.
    const std::string p = to_lower_ascii(file);
    if (top_folder(p) == "tests") return true;
    if (!starts_with(p, "tools/")) return false;
    if (p.find("/tests/", 5) != std::string::npos) return true;
    const std::string name = p.substr(p.rfind('/') + 1);
    return name.size() >= 8 && starts_with(name, "test_") && ends_with(name, ".py");
}

bool skipped_file(const std::string& file) {
    return starts_with(file, "tests/single_owner/") || to_lower_ascii(file) == "tests/test_single_owner.cpp";
}

const std::vector<std::string>& code_folders() {
    static const std::vector<std::string> folders(std::begin(sourcetree::kWalkedFolders),
                                                  std::end(sourcetree::kWalkedFolders));
    return folders;
}

// ------------------------------------------------------------------ range

Range resolve_range(const Repo& repo, const std::string& spelling, bool whole_tree,
                    const std::string& rules_at) {
    Range r;
    r.spelling = spelling;
    r.whole_tree = whole_tree;
    const size_t dots = spelling.find("..");
    if (dots != std::string::npos) {
        const bool three = dots + 2 < spelling.size() && spelling[dots + 2] == '.';
        const std::string a = spelling.substr(0, dots);
        const std::string b = spelling.substr(dots + (three ? 3 : 2));
        r.tip = repo.resolve_commit(range_side(b));
        r.left = repo.resolve_commit(range_side(a));
        r.base = three ? fork_point(repo, r.left, r.tip) : r.left;
    } else {
        r.tip = repo.resolve_commit(spelling);
        // "<commit> <parent> <parent>...", one line.
        const std::vector<std::string> line = repo.git({"rev-list", "--parents", "-n", "1", r.tip});
        std::vector<std::string> parents;
        if (!line.empty()) {
            std::istringstream in(line[0]);
            for (std::string h; in >> h;) parents.push_back(h);
            if (!parents.empty()) parents.erase(parents.begin());
        }
        if (parents.size() >= 2) {
            // A merge commit: the same as "<first parent>...<second parent>".
            r.left = parents[0];
            r.tip = parents[1];
            r.base = fork_point(repo, r.left, r.tip);
        } else if (!whole_tree) {
            r.base = repo.resolve_commit(r.tip + "^1");
        }
    }
    r.rules_rev = rules_at.empty() ? r.tip : repo.resolve_commit(rules_at);
    return r;
}

std::vector<std::string> all_files(const Repo& repo, const std::string& rev) {
    std::vector<std::string> args = {"ls-tree", "-r", "--name-only", rev, "--"};
    args.insert(args.end(), code_folders().begin(), code_folders().end());
    return repo.git(args);
}

// ------------------------------------------------------------ added lines

AddedLines AddedLines::whole(const std::vector<std::string>& files) {
    AddedLines a;
    for (const std::string& f : files) a.sets_[f] = std::nullopt;
    return a;
}

AddedLines AddedLines::from_diff(const Repo& repo, const std::string& base, const std::string& tip) {
    return ChangedLines::from_diff(repo, base, tip).added;
}

AddedLines AddedLines::parse_diff(const std::vector<std::string>& diff_lines) {
    return ChangedLines::parse_diff(diff_lines).added;
}

ChangedLines ChangedLines::from_diff(const Repo& repo, const std::string& base, const std::string& tip) {
    std::vector<std::string> args = {"diff", "--no-color", "--no-ext-diff", "--no-renames", "-U0", base, tip, "--"};
    args.insert(args.end(), code_folders().begin(), code_folders().end());
    return parse_diff(repo.git(args));
}

ChangedLines ChangedLines::parse_diff(const std::vector<std::string>& diff_lines) {
    ChangedLines c;
    std::map<std::string, std::optional<std::set<int>>>& added = c.added.sets_;
    std::string cur;  // the tip path the hunks belong to; empty for a deleted file
    std::string old_cur;  // the base path the hunks belong to; empty for a new file
    long long old_left = 0, new_left = 0;  // hunk body lines still to come
    for (const std::string& l : diff_lines) {
        // A hunk's own lines are skipped by count, so a body line that
        // starts "++ " or "-- " is never read as a file header.
        if (old_left > 0 || new_left > 0) {
            if (!l.empty() && l[0] == '-') --old_left;
            else if (!l.empty() && l[0] == '+') --new_left;
            continue;
        }
        if (starts_with(l, "--- ")) {
            static const std::string kOldFile = "--- a/";  // git's default prefix for the old side
            old_cur = starts_with(l, kOldFile) ? l.substr(kOldFile.size()) : "";
            if (!old_cur.empty()) c.deleted[old_cur];
            continue;
        }
        if (starts_with(l, "+++ ")) {
            static const std::string kNewFile = "+++ b/";  // git's default prefix for the new side
            cur = starts_with(l, kNewFile) ? l.substr(kNewFile.size()) : "";
            if (!cur.empty() && !added.count(cur)) added[cur] = std::set<int>{};
            continue;
        }
        if (!starts_with(l, "@@ ")) continue;
        // "@@ -a[,b] +c[,d] @@ ..."
        const size_t sp1 = l.find(' ', 3);
        const size_t sp2 = sp1 == std::string::npos ? std::string::npos : l.find(' ', sp1 + 1);
        if (sp2 == std::string::npos || l.compare(sp2, 3, " @@") != 0) continue;
        long long ostart = 0, ocount = 0, nstart = 0, ncount = 0;
        if (!hunk_side(l.substr(3, sp1 - 3), '-', ostart, ocount) ||
            !hunk_side(l.substr(sp1 + 1, sp2 - sp1 - 1), '+', nstart, ncount))
            continue;
        old_left = ocount;
        new_left = ncount;
        if (!old_cur.empty())
            for (long long i = 0; i < ocount; ++i) c.deleted[old_cur].insert(static_cast<int>(ostart + i));
        if (!cur.empty())
            for (long long i = 0; i < ncount; ++i) added[cur]->insert(static_cast<int>(nstart + i));
    }
    return c;
}

bool AddedLines::added(const std::string& file, int line) const {
    const auto it = sets_.find(file);
    if (it == sets_.end()) return false;
    return !it->second || it->second->count(line) != 0;
}

std::vector<int> AddedLines::lines(const std::string& file, int count) const {
    std::vector<int> out;
    const auto it = sets_.find(file);
    if (it == sets_.end()) return out;
    if (!it->second) {
        for (int i = 1; i <= count; ++i) out.push_back(i);
        return out;
    }
    for (const int l : *it->second)
        if (l <= count) out.push_back(l);
    return out;
}

std::vector<std::string> AddedLines::files() const {
    std::vector<std::string> out;
    for (const auto& kv : sets_) out.push_back(kv.first);
    return out;
}

// --------------------------------------------------------------- the size

const std::vector<std::string>& size_excluded_folders() {
    static const std::vector<std::string> folders = {"third_party", "testdata"};
    return folders;
}

RangeSize range_size(const Repo& repo, const std::string& base, const std::string& tip) {
    return parse_numstat(repo.git({"diff", "--numstat", "--no-color", "--no-ext-diff", "--no-renames", base, tip}));
}

RangeSize parse_numstat(const std::vector<std::string>& numstat_lines) {
    RangeSize s;
    for (const std::string& l : numstat_lines) {
        // "<added>\t<deleted>\t<path>", with "-" for both on a binary file.
        const size_t t1 = l.find('\t');
        const size_t t2 = t1 == std::string::npos ? std::string::npos : l.find('\t', t1 + 1);
        if (t2 == std::string::npos || t2 + 1 >= l.size()) continue;
        const std::string a = l.substr(0, t1), d = l.substr(t1 + 1, t2 - t1 - 1);
        if ((!all_digits(a) && a != "-") || (!all_digits(d) && d != "-")) continue;
        if (a == "-") continue;  // a binary file
        std::string path = l.substr(t2 + 1);
        path.erase(0, path.find_first_not_of('"'));  // git quotes an unusual path
        const std::vector<std::string>& skip = size_excluded_folders();
        if (std::find(skip.begin(), skip.end(), to_lower_ascii(top_folder(path))) != skip.end()) continue;
        s.added += std::stoll(a);
        s.deleted += d == "-" ? 0 : std::stoll(d);
    }
    return s;
}

}  // namespace precheck
