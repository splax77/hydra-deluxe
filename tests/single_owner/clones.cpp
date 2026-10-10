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
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../source_tree.h"
#include "core/strutil.h"
#include "listed.h"
#include "rows.h"

namespace fs = std::filesystem;
using namespace single_owner;

namespace single_owner {

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

}  // namespace single_owner

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
