// hydra_precheck's git layer and output (tools/precheck/): the range
// spellings, the added-line sets, the file kinds, the size lines, the item
// order and wording, --disable's refusal and --code-mask. A tiny git repo in a
// temp folder is the fixture, and this repo's own files and history are the
// real input; the exe runs as a child process on both.
#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "code_lexer.h"
#include "core/strutil.h"
#include "core/winstr.h"
#include "precheck/check_rows.h"
#include "precheck/checks.h"
#include "precheck/code_mask.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "single_owner/engine.h"
#include "source_tree.h"
#include "precheck_fixture.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;

using namespace precheck_fixture;

namespace {

// The fixture: main1 on main; feature forks there with two commits (f1 edits
// src/a.cpp, deletes tests/b.cpp and touches the two excluded folders; f2
// adds tools/new.py and a binary file); then main moves on with main2, which
// edits line 1 of src/a.cpp.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_git");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        write_fixture_file(dir,"src/a.cpp", "int one;\nint two;\nint three;\n");
        write_fixture_file(dir,"tests/b.cpp", "int b1;\nint b2;\n");
        write_fixture_file(dir,"third_party/x.txt", "x\n");
        commit_all(dir, "main1");
        fixture_git(dir, {"checkout", "-q", "-b", "feature"});
        // Line 4 is "++ x": its diff line starts "+++ ", like a file header.
        write_fixture_file(dir,"src/a.cpp", "int one;\nint TWO;\nint three;\n++ x\nint four;\n");
        fs::remove(hydra::os_path(dir + "\\tests\\b.cpp"));
        write_fixture_file(dir,"third_party/x.txt", "x\ny\nz\nw\n");
        write_fixture_file(dir,"testdata/y.txt", "y1\ny2\n");
        // A name git quotes in its diff output, under an excluded folder.
        write_fixture_file(dir,"third_party/caf\xC3\xA9.txt", "c1\nc2\n");
        commit_all(dir, "f1");
        write_fixture_file(dir,"tools/new.py", "print(1)\nprint(2)\n");
        write_fixture_file(dir,"src/bin.dat", std::string("\0\1\2\0", 4));
        commit_all(dir, "f2");
        fixture_git(dir, {"checkout", "-q", "main"});
        write_fixture_file(dir,"src/a.cpp", "int uno;\nint two;\nint three;\n");
        commit_all(dir, "main2");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};

// The files of this repo git lists under any of `pathspecs`.
std::set<std::string> listed(const std::vector<std::string>& pathspecs) {
    std::set<std::string> files;
    for (const std::string& spec : pathspecs)
        for (const std::string& f : sourcetree::git_listed(sourcetree::root(), spec)) files.insert(f);
    return files;
}

// This repo's root, as the precheck's --repo takes it.
std::string source_root() { return sourcetree::root().u8string(); }

// What hydra_precheck prints for `range` with the checks in `disabled` off,
// built from the library calls its run makes. `passes` runs every check that
// many times into the one item list; only the first pass's printed lines are
// kept.
struct LibraryRun {
    std::string out;
    std::vector<Item> sorted;
};

LibraryRun library_run(Repo& repo, const Range& range, const std::set<int>& disabled, int passes) {
    const std::vector<std::string> files = all_files(repo, range.tip);
    const AddedLines added =
        range.whole_tree ? AddedLines::whole(files) : AddedLines::from_diff(repo, range.base, range.tip);
    Items items;
    std::ostringstream printed;
    std::ostringstream discarded;
    for (int pass = 0; pass < passes; ++pass) {
        Context ctx{repo, range, added, files, items, pass == 0 ? static_cast<std::ostream&>(printed) : discarded};
        for (size_t n = 1; n <= kChecks.size(); ++n)
            if (!disabled.count(static_cast<int>(n))) kChecks[n - 1].run(ctx);
        if (pass == 0) print_rule_warnings(ctx);
    }
    LibraryRun run;
    run.sorted = items.sorted();
    run.out = printed.str();
    for (const Item& i : run.sorted) run.out += item_line(i) + "\n";
    if (!range.whole_tree) {
        const RangeSize size = range_size(repo, range.base, range.tip);
        run.out += size_line(size) + "\n";
        if (const std::optional<std::string> big = large_range_line(size)) run.out += *big + "\n";
    }
    run.out += summary_line(run.sorted, range) + "\n";
    return run;
}

// Points this process's PATH at an empty folder while it lives, so no new
// git process can start, and puts PATH back after.
class NoProgramsOnPath {
public:
    NoProgramsOnPath() {
        wchar_t* old = nullptr;
        size_t n = 0;
        if (_wdupenv_s(&old, &n, L"PATH") == 0 && old != nullptr) saved_ = old;
        free(old);
        _wputenv_s(L"PATH", hydra::os_path(empty_).wstring().c_str());
    }
    ~NoProgramsOnPath() {
        _wputenv_s(L"PATH", saved_.c_str());
        testtemp::remove_dir(empty_);
    }
    NoProgramsOnPath(const NoProgramsOnPath&) = delete;
    NoProgramsOnPath& operator=(const NoProgramsOnPath&) = delete;

private:
    std::string empty_ = testtemp::temp_dir("precheck_no_programs");
    std::wstring saved_;
};

// The sentence parse_disable refuses `values` with, or "" when it takes them.
std::string disable_refusal(const std::vector<std::string>& values) {
    try {
        (void)parse_disable(values);
    } catch (const std::invalid_argument& e) {
        return e.what();
    }
    return "";
}

// A range of this repo's own history on which hydra_precheck lists items of
// two kinds over two files.
const char* const kRealRange =
    "08a01c9389c248a4af44388138706a5695797175...7c2ca470982c4cc1e94587a1ece400ddd7a30d52";

}  // namespace

TEST_CASE("precheck: the three range spellings name the commits git does") {
    Fixture fx;
    Repo repo(fx.dir);
    const std::string main1 = repo.resolve_commit("main~1");
    const std::string main2 = repo.resolve_commit("main");
    const std::string f1 = repo.resolve_commit("feature~1");
    const std::string f2 = repo.resolve_commit("feature");

    const Range three = resolve_range(repo, "main...feature", false, "");
    CHECK(three.tip == f2);
    CHECK(three.left == main2);
    CHECK(three.base == main1);  // the fork point
    CHECK(three.rules_rev == f2);

    const Range two = resolve_range(repo, "main..feature", false, "");
    CHECK(two.tip == f2);
    CHECK(two.base == main2);  // A itself

    const Range one = resolve_range(repo, "feature", false, "");
    CHECK(one.tip == f2);
    CHECK(one.left.empty());
    CHECK(one.base == f1);  // its first parent

    const Range whole = resolve_range(repo, "feature", true, "main");
    CHECK(whole.base.empty());
    CHECK(whole.rules_rev == main2);

    // An empty side means HEAD (main is checked out).
    const Range empty_side = resolve_range(repo, "feature...", false, "");
    CHECK(empty_side.tip == main2);
    CHECK(empty_side.left == f2);
}

TEST_CASE("precheck: the added lines of each range spelling") {
    Fixture fx;
    Repo repo(fx.dir);
    auto added_of = [&](const std::string& spelling, bool whole) {
        const Range r = resolve_range(repo, spelling, whole, "");
        return whole ? AddedLines::whole(all_files(repo, r.tip)) : AddedLines::from_diff(repo, r.base, r.tip);
    };

    const AddedLines three = added_of("main...feature", false);
    CHECK(three.files() == std::vector<std::string>{"src/a.cpp", "tools/new.py"});
    CHECK(three.lines("src/a.cpp", 100) == std::vector<int>{2, 4, 5});
    CHECK(three.lines("tools/new.py", 100) == std::vector<int>{1, 2});
    CHECK_FALSE(three.has_file("src/bin.dat"));  // binary: git gives it no lines
    CHECK_FALSE(three.has_file("tests/b.cpp"));  // deleted: nothing added
    CHECK_FALSE(three.has_file("third_party/x.txt"));  // outside the code folders
    CHECK(three.added("src/a.cpp", 4));
    CHECK_FALSE(three.added("src/a.cpp", 1));

    // Over main itself, line 1 differs from main2's "int uno;".
    const AddedLines two = added_of("main..feature", false);
    CHECK(two.lines("src/a.cpp", 100) == std::vector<int>{1, 2, 4, 5});

    // One commit: f2 only.
    const AddedLines one = added_of("feature", false);
    CHECK(one.files() == std::vector<std::string>{"tools/new.py"});

    const AddedLines whole = added_of("feature", true);
    CHECK(whole.files() == std::vector<std::string>{"src/a.cpp", "src/bin.dat", "tools/new.py"});
    CHECK(whole.lines("src/a.cpp", 3) == std::vector<int>{1, 2, 3});
    CHECK(whole.added("src/a.cpp", 999));
}

TEST_CASE("precheck: every file read goes through the one cat-file process") {
    Fixture fx;
    Repo repo(fx.dir);
    CatFileBatch& files = repo.files();
    // The first read starts the process; after it no new git can start, so
    // a read that started a second process would throw.
    const std::optional<std::string> a = files.raw("main~1", "src/a.cpp");
    CHECK(a == std::string("int one;\nint two;\nint three;\n"));
    NoProgramsOnPath no_git;
    CHECK_THROWS_AS(repo.git({"rev-parse", "HEAD"}), GitError);
    CHECK(&repo.files() == &files);
    CHECK_FALSE(files.raw("main~1", "tools/new.py").has_value());
    // The same process goes on after a missing file.
    CHECK(files.raw("main~1", "src/a.cpp") == a);
    CHECK(files.raw("feature", "src/bin.dat") == std::string("\0\1\2\0", 4));
    CHECK_THROWS_AS(files.text("main", "tools/new.py"), GitError);
    const std::optional<std::string> py = files.raw("feature", "tools/new.py");
    REQUIRE(py.has_value());
    CHECK(files.text("feature", "tools/new.py") == tip_text_form(*py));
}

// CatFileBatch::text hands every check a file's text in this form.
TEST_CASE("precheck: a file's text form drops a byte order mark and the last break and reads each break as one") {
    CHECK(tip_text_form("\xEF\xBB\xBF" "a\r\nb\rc\n") == "a\nb\nc");
    CHECK(tip_text_form("a\n\n") == "a\n");
    CHECK(tip_text_form("") == "");

    // A file saved with a byte order mark and Windows line breaks, read
    // through cat-file, has the lines git shows for it.
    const std::string dir = testtemp::temp_dir("precheck_text_form");
    testtemp::remove_dir(dir);
    fs::create_directories(hydra::os_path(dir));
    fixture_git(dir, {"init", "-q", "-b", "main"});
    write_fixture_file(dir, "src/crlf.cpp", "\xEF\xBB\xBF" "int one;\r\nint two;\r\n");
    commit_all(dir, "crlf");
    std::string shown;
    for (const std::string& l : run_git(dir, {"show", "HEAD:src/crlf.cpp"})) shown += (shown.empty() ? "" : "\n") + l;
    REQUIRE(shown.rfind("\xEF\xBB\xBF", 0) == 0);
    {
        Repo repo(dir);
        CHECK(repo.files().text("HEAD", "src/crlf.cpp") == shown.substr(3));
    }
    testtemp::remove_dir(dir);
}

// Each tracked file of this repo, as git lists it, against what git's own
// pathspecs and the scan's owners say it is.
TEST_CASE("precheck: what kind of file each tracked file of this repo is") {
    const std::set<std::string> all = listed({"."});
    REQUIRE_FALSE(all.empty());

    // top_folder: the folder git lists the file under, or nothing for a file
    // at the root.
    const std::set<std::string> at_root = listed({":(glob)*"});
    std::map<std::string, std::set<std::string>> by_top;
    for (const std::string& f : all) {
        INFO(f);
        const std::string top = top_folder(f);
        CHECK(top.empty() == (at_root.count(f) == 1));
        CHECK(top.find('/') == std::string::npos);
        if (!top.empty()) by_top[top].insert(f);
    }
    for (const auto& [top, files] : by_top) {
        INFO(top);
        const std::set<std::string> under = listed({":(literal)" + top});
        for (const std::string& f : files) CHECK(under.count(f) == 1);
    }

    // is_cpp and is_py read a file as the single-owner scan does.
    int implementations = 0;
    for (const std::string& f : all) {
        INFO(f);
        const fs::path p = fs::u8path(f);
        CHECK(is_cpp(f) == single_owner::is_cpp_source(p));
        CHECK(is_py(f) == (codelex::language_of_extension(p.extension().string()) == codelex::Lang::Python));
        if (is_cpp_implementation(f)) {
            CHECK(is_cpp(f));
            ++implementations;
        }
    }
    CHECK(implementations > 0);
    const std::set<std::string> headers = listed({":(glob)**/*.h"});
    REQUIRE_FALSE(headers.empty());
    for (const std::string& h : headers) {
        INFO(h);
        CHECK(is_cpp(h));
        CHECK_FALSE(is_cpp_implementation(h));
    }

    // test_file: all of tests/; under tools/, a file in a tests folder or a
    // test_*.py file; nothing else.
    const std::set<std::string> under_tests = listed({":(literal)tests"});
    const std::set<std::string> under_tools = listed({":(literal)tools"});
    const std::set<std::string> tool_tests =
        listed({":(glob,icase)tools/**/tests/**", ":(glob,icase)tools/**/test_*.py"});
    REQUIRE_FALSE(under_tests.empty());
    REQUIRE_FALSE(tool_tests.empty());
    REQUIRE(tool_tests.size() < under_tools.size());
    std::set<std::string> test_files = under_tests;
    test_files.insert(tool_tests.begin(), tool_tests.end());
    for (const std::string& f : all) {
        INFO(f);
        CHECK(test_file(f) == (test_files.count(f) == 1));
    }

    // skipped_file: exactly the files of the scan's own folder.
    const std::string scan_folder = fs::path(single_owner::kRulesDir).parent_path().generic_string();
    const std::set<std::string> scan_files = listed({":(literal)" + scan_folder});
    REQUIRE_FALSE(scan_files.empty());
    for (const std::string& f : all) {
        INFO(f);
        CHECK(skipped_file(f) == (scan_files.count(f) == 1));
    }
}

TEST_CASE("precheck: the range size leaves out third_party and testdata") {
    Fixture fx;
    Repo repo(fx.dir);
    const Range r = resolve_range(repo, "main...feature", false, "");
    // git's own numstat of the range, with its quoting of unusual names on.
    const std::vector<std::string> numstat =
        repo.git({"-c", "core.quotePath=true", "diff", "--numstat", r.base, r.tip});
    // git's numstat and shortstat with the excluded folders left out by
    // pathspec, and the numstat of those folders alone.
    std::vector<std::string> kept_numstat = {"diff", "--numstat", r.base, r.tip, "--", "."};
    std::vector<std::string> kept_shortstat = {"diff", "--shortstat", r.base, r.tip, "--", "."};
    std::vector<std::string> excluded_numstat = {"-c", "core.quotePath=true", "diff", "--numstat", r.base, r.tip, "--"};
    for (const std::string& folder : size_excluded_folders()) {
        kept_numstat.push_back(":(exclude)" + folder);
        kept_shortstat.push_back(":(exclude)" + folder);
        excluded_numstat.push_back(":(literal)" + folder);
    }
    // The excluded folders do change lines in this range, one under a quoted
    // name.
    const std::vector<std::string> excluded_lines = repo.git(excluded_numstat);
    REQUIRE_FALSE(excluded_lines.empty());
    bool quoted = false;
    for (const std::string& l : excluded_lines) quoted = quoted || l.find("\t\"") != std::string::npos;
    REQUIRE(quoted);

    const RangeSize s = parse_numstat(numstat);
    const RangeSize kept = parse_numstat(repo.git(kept_numstat));
    CHECK(s.added == kept.added);
    CHECK(s.deleted == kept.deleted);
    REQUIRE(s.added > 0);
    REQUIRE(s.deleted > 0);
    const std::vector<std::string> shortstat = repo.git(kept_shortstat);
    REQUIRE(shortstat.size() == 1);
    INFO(shortstat[0]);
    CHECK(shortstat[0].find(" " + std::to_string(s.added) + " insertion") != std::string::npos);
    CHECK(shortstat[0].find(" " + std::to_string(s.deleted) + " deletion") != std::string::npos);

    // range_size, which the exe prints, reads git the same way.
    const RangeSize g = range_size(repo, r.base, r.tip);
    CHECK(g.added == s.added);
    CHECK(g.deleted == s.deleted);

    // The size line names the three counts and every excluded folder.
    const std::string line = size_line(s);
    INFO(line);
    CHECK(line.rfind(kSizePrefix, 0) == 0);
    for (const long long n : {s.total(), s.added, s.deleted}) CHECK(line.find(std::to_string(n)) != std::string::npos);
    for (const std::string& folder : size_excluded_folders()) CHECK(line.find(folder + "/") != std::string::npos);

    // The warning starts one line past kLargeRangeLines and names both.
    CHECK_FALSE(large_range_line(RangeSize{kLargeRangeLines, 0}).has_value());
    const std::optional<std::string> big = large_range_line(RangeSize{kLargeRangeLines, 1});
    REQUIRE(big.has_value());
    CHECK(big->rfind(kLargeRangePrefix, 0) == 0);
    CHECK(big->find(std::to_string(kLargeRangeLines + 1)) != std::string::npos);
    CHECK(big->find(std::to_string(kLargeRangeLines)) != std::string::npos);
}

// parse_numstat reads only a path's top folder, which a closing quote never
// changes, so this asks the helper itself. git quotes a name in --name-only
// the same way as in --numstat.
TEST_CASE("precheck: a quoted numstat path loses both quotes") {
    Fixture fx;
    Repo repo(fx.dir);
    const Range r = resolve_range(repo, "main...feature", false, "");
    std::optional<std::string> quoted;
    std::optional<std::string> plain;
    for (const std::string& name : repo.git({"-c", "core.quotePath=true", "diff", "--name-only", r.base, r.tip})) {
        if (!name.empty() && name.front() == '"') quoted = name;
        else if (!plain) plain = name;
    }
    REQUIRE(quoted.has_value());
    REQUIRE(plain.has_value());
    INFO(*quoted);
    const std::string stripped = numstat_path(*quoted);
    CHECK(stripped.size() + 2 == quoted->size());
    CHECK(quoted->find(stripped) == 1);
    // The quoted name is under an excluded folder, which its top folder now
    // names.
    const std::vector<std::string>& excluded = size_excluded_folders();
    CHECK(std::find(excluded.begin(), excluded.end(), top_folder(stripped)) != excluded.end());
    CHECK(numstat_path(*plain) == *plain);
    CHECK(numstat_path("\"") == "\"");
}

// The items the real checks find on a range of this repo's history.
TEST_CASE("precheck: items print in kind order, then file, line and text") {
    Repo repo(source_root());
    const Range range = resolve_range(repo, kRealRange, false, "");
    const LibraryRun once = library_run(repo, range, {}, 1);
    const std::vector<Item>& sorted = once.sorted;

    // The range holds what the order is about: two kinds, a kind with two
    // files, and a file with two items.
    std::set<std::string> kinds;
    std::map<std::string, std::set<std::string>> files_of_kind;
    std::map<std::string, int> items_of_file;
    for (const Item& i : sorted) {
        kinds.insert(i.kind);
        files_of_kind[i.kind].insert(i.file);
        ++items_of_file[i.kind + " " + i.file];
    }
    REQUIRE(kinds.size() >= 2);
    bool two_files = false;
    for (const auto& entry : files_of_kind) two_files = two_files || entry.second.size() >= 2;
    REQUIRE(two_files);
    bool two_items = false;
    for (const auto& entry : items_of_file) two_items = two_items || entry.second >= 2;
    REQUIRE(two_items);

    for (size_t k = 1; k < sorted.size(); ++k) {
        const Item& a = sorted[k - 1];
        const Item& b = sorted[k];
        INFO(item_line(a));
        INFO(item_line(b));
        const size_t ka = kind_order().find(a.kind);
        const size_t kb = kind_order().find(b.kind);
        REQUIRE(ka != std::string::npos);
        REQUIRE(kb != std::string::npos);
        CHECK(ka <= kb);
        if (ka != kb) continue;
        CHECK_FALSE(text_before(b.file, a.file));
        if (text_before(a.file, b.file)) continue;
        CHECK(a.line <= b.line);
        // An item equal to the one before it is gone, so the text differs.
        if (a.line == b.line) CHECK(text_before(a.what, b.what));
    }

    // Every item found twice prints once.
    const LibraryRun twice = library_run(repo, range, {}, 2);
    std::vector<std::string> once_lines;
    std::vector<std::string> twice_lines;
    for (const Item& i : sorted) once_lines.push_back(item_line(i));
    for (const Item& i : twice.sorted) twice_lines.push_back(item_line(i));
    CHECK(twice_lines == once_lines);

    // The exe prints the same, and its summary names each kind's count, the
    // range and the tip as git abbreviates it.
    const ExeOutput r = run_precheck_exe({"--repo", source_root(), "--range", kRealRange});
    CHECK(r.exit_code == 0);
    CHECK(r.out == once.out);
    const std::string summary = summary_line(sorted, range);
    CHECK(hydra::ends_with(r.out, summary + "\n"));
    INFO(summary);
    CHECK(summary.find(std::to_string(sorted.size()) + " items") != std::string::npos);
    for (const std::string& kind : kinds) {
        size_t n = 0;
        for (const Item& i : sorted) n += i.kind == kind ? 1 : 0;
        CHECK(summary.find(kind + " " + std::to_string(n)) != std::string::npos);
    }
    CHECK(summary.find(kRealRange) != std::string::npos);
    const std::string short_tip = repo.git({"rev-parse", "--short=7", range.tip}).at(0);
    CHECK(hydra::ends_with(summary, " " + short_tip));
}

TEST_CASE("precheck: --disable takes only check numbers 1 to the number of checks") {
    const int n = static_cast<int>(kChecks.size());
    REQUIRE(n >= 1);
    // Every check, in one list with empty pieces, and in two values.
    std::string every_list = ",";
    std::set<int> every;
    for (int i = 1; i <= n; ++i) {
        every_list += std::to_string(i) + ",";
        every.insert(i);
    }
    CHECK(parse_disable({every_list}) == every);
    CHECK(parse_disable({std::to_string(n), "1"}) == std::set<int>{1, n});
    CHECK(disable_refusal({std::to_string(n)}).empty());

    // One past the last check, zero and a word are refused, and the refusal
    // names the bad piece and the last check number.
    for (const std::string& bad : std::vector<std::string>{std::to_string(n + 1), "0", "two"}) {
        const std::string refusal = disable_refusal({"1," + bad});
        INFO(bad);
        INFO(refusal);
        CHECK(refusal.find("'" + bad + "'") != std::string::npos);
        CHECK(refusal.find(" " + std::to_string(n) + ",") != std::string::npos);
    }

    // The exe prints parse_disable's refusal on stderr and nothing else.
    Fixture fx;
    const std::string past_last = "1," + std::to_string(n + 1);
    const ExeOutput bad = run_precheck_exe({"--repo", fx.dir, "--range", "main...feature", "--disable", past_last});
    CHECK(bad.exit_code != 0);
    const std::string refusal = disable_refusal({past_last});
    REQUIRE_FALSE(refusal.empty());
    CHECK(bad.err == refusal + "\n");
    CHECK(bad.out.empty());
}

TEST_CASE("precheck: the exe prints what the library gives for a range and the whole tree, and writes nothing") {
    Fixture fx;
    Repo repo(fx.dir);
    const std::string short_tip = run_git(fx.dir, {"rev-parse", "--short=7", "feature"}).at(0);

    const ExeOutput r = run_precheck_exe({"--repo", fx.dir, "--range", "main...feature", "--disable", "2"});
    CHECK(r.exit_code == 0);
    CHECK(r.err.empty());
    const Range range = resolve_range(repo, "main...feature", false, "");
    CHECK(r.out == library_run(repo, range, {2}, 1).out);
    CHECK(r.out.find(std::string("\n") + kSizePrefix) != std::string::npos);
    CHECK(hydra::ends_with(r.out, " " + short_tip + "\n"));

    const ExeOutput whole = run_precheck_exe({"--repo", fx.dir, "--whole-tree", "feature"});
    CHECK(whole.exit_code == 0);
    const Range whole_range = resolve_range(repo, "feature", true, "");
    CHECK(whole.out == library_run(repo, whole_range, {}, 1).out);
    CHECK(whole.out.find(kSizePrefix) == std::string::npos);
    CHECK(hydra::ends_with(whole.out, " " + short_tip + "\n"));

    // Nothing in the fixture changed.
    CHECK(run_git(fx.dir, {"status", "--porcelain"}).empty());
}

TEST_CASE("precheck: --list-checks prints each check's number and name") {
    const ExeOutput list = run_precheck_exe({"--list-checks"});
    CHECK(list.exit_code == 0);
    std::string expected;
    for (size_t i = 0; i < kChecks.size(); ++i) expected += std::to_string(i + 1) + " " + kChecks[i].name + "\n";
    CHECK(list.out == expected);
}

TEST_CASE("precheck: the exe refuses a folder outside any git repository") {
    const std::string plain = testtemp::temp_dir("precheck_not_a_repo");
    std::string refusal;
    try {
        Repo repo(plain);
    } catch (const GitError& e) {
        refusal = e.what();
    }
    REQUIRE_FALSE(refusal.empty());
    const ExeOutput no_repo = run_precheck_exe({"--repo", plain});
    CHECK(no_repo.exit_code != 0);
    CHECK(no_repo.err == refusal + "\n");
    testtemp::remove_dir(plain);
}

TEST_CASE("precheck: --code-mask blanks comments and string text and keeps every line break") {
    const std::string src =
        "int a = 1; // c\n"
        "s = \"hi // no\";\n"
        "/* a\n"
        " \xC3\xA9 */ c = '\\'';\n"
        "r = R\"d(x \" y)d\";\n"
        "// \xF0\x9F\x8E\xB8\n"
        "x; // y\r\n"
        "n = 1'000;\n";
    const std::string want =
        "int a = 1;     \n"
        "s = \"        \";\n"
        "    \n"
        "      c = '  ';\n"
        "r = R\"         \";\n"
        "     \n"
        "x;      \n"
        "n = 1'000;\n";
    CHECK(code_mask(src, "src/x.cpp") == want);
    CHECK(code_mask("x = 'a#b'  # c\n", "tools/x.py") == std::string("x = '   '     \n"));
    CHECK_FALSE(code_mask(src, "notes.txt").has_value());

    const std::string dir = testtemp::temp_dir("precheck_code_mask");
    write_fixture_file(dir,"x.cpp", src);
    const ExeOutput r = run_precheck_exe({"--code-mask", dir + "\\x.cpp"});
    CHECK(r.exit_code == 0);
    CHECK(r.out == want);
    testtemp::remove_dir(dir);
}
