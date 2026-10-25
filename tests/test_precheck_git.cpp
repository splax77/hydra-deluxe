// hydra_precheck's git layer and output (tools/precheck/): the range
// spellings, the added-line sets, the file kinds, the size lines, the item
// order and wording, --disable's refusal and --code-mask. A tiny git repo in a
// temp folder is the fixture; the exe runs as a child process on it.
#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/checks.h"
#include "precheck/code_mask.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/range.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;

namespace {

void write_fixture_file(const std::string& dir, const std::string& rel, const std::string& text) {
    const std::string path = dir + "\\" + rel;
    fs::create_directories(hydra::os_path(fs::path(hydra::os_path(path)).parent_path()));
    testtemp::write_bytes(path, std::vector<uint8_t>(text.begin(), text.end()));
}

void fixture_git(const std::string& dir, std::vector<std::string> args) {
    std::vector<std::string> full = {"-c", "user.name=precheck", "-c", "user.email=precheck@example.invalid",
                                     "-c", "core.autocrlf=false"};
    full.insert(full.end(), args.begin(), args.end());
    run_git(dir, full);
}

void commit_all(const std::string& dir, const std::string& message) {
    fixture_git(dir, {"add", "-A"});
    fixture_git(dir, {"commit", "-q", "-m", message});
}

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

struct ExeRun {
    int exit_code = -1;
    std::string out;
    std::string err;
};

ExeRun run_precheck(const std::vector<std::string>& args) {
    std::vector<std::string> full = {HYDRA_PRECHECK_EXE};
    full.insert(full.end(), args.begin(), args.end());
    ChildProcess p(full);
    ExeRun r;
    r.out = p.read_all();
    r.exit_code = p.wait(r.err);
    return r;
}

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

TEST_CASE("precheck: one cat-file process reads every file") {
    Fixture fx;
    Repo repo(fx.dir);
    CatFileBatch& files = repo.files();
    CHECK(files.raw("main~1", "src/a.cpp") == std::string("int one;\nint two;\nint three;\n"));
    CHECK_FALSE(files.raw("main~1", "tools/new.py").has_value());
    // The same process goes on after a missing file.
    CHECK(files.text("feature", "tools/new.py") == "print(1)\nprint(2)");
    CHECK(files.raw("feature", "src/bin.dat") == std::string("\0\1\2\0", 4));
    CHECK_THROWS_AS(files.text("main", "tools/new.py"), GitError);
    CHECK(&repo.files() == &files);

    CHECK(tip_text_form("\xEF\xBB\xBF" "a\r\nb\rc\n") == "a\nb\nc");
    CHECK(tip_text_form("a\n\n") == "a\n");
    CHECK(tip_text_form("") == "");
}

TEST_CASE("precheck: what kind of file each path is") {
    CHECK(top_folder("src/core/x.cpp") == "src");
    CHECK(top_folder("README.md") == "");
    CHECK(is_cpp("src/x.cpp"));
    CHECK(is_cpp("src/x.h"));
    CHECK_FALSE(is_cpp("src/x.CPP"));
    CHECK_FALSE(is_cpp("src/x.hpp"));
    CHECK(is_cpp_implementation("src/x.cpp"));
    CHECK_FALSE(is_cpp_implementation("src/x.h"));
    CHECK(is_py("tools/x.py"));
    CHECK_FALSE(is_py("tools/x.PY"));
    CHECK(test_file("tests/x.cpp"));
    CHECK(test_file("Tests/x.cpp"));
    CHECK(test_file("tools/test_x.py"));
    CHECK(test_file("tools/sub/test_x.py"));
    CHECK(test_file("tools/tests/x.cpp"));
    CHECK(test_file("tools/a/tests/x.cpp"));
    CHECK_FALSE(test_file("tools/test_x.cpp"));
    CHECK_FALSE(test_file("tools/x_test.py"));
    CHECK_FALSE(test_file("src/tests/x.cpp"));
    CHECK_FALSE(test_file("tools/replay.cpp"));
    CHECK(skipped_file("tests/single_owner/scan.cpp"));
    CHECK(skipped_file("tests/test_single_owner.cpp"));
    CHECK_FALSE(skipped_file("tests/test_single_owner_x.cpp"));
    CHECK_FALSE(skipped_file("tests/Single_owner/scan.cpp"));
}

TEST_CASE("precheck: the range size leaves out third_party and testdata") {
    // numstat lines: a counted file, a binary, and both excluded folders.
    const RangeSize s = parse_numstat({"3\t1\tsrc/a.cpp", "-\t-\tsrc/bin.dat", "50\t0\tthird_party/x.txt",
                                       "7\t2\ttestdata/y.txt", "0\t2\ttests/b.cpp", "2\t0\t\"tools/n w.py\""});
    CHECK(s.added == 5);
    CHECK(s.deleted == 3);
    CHECK(size_line(s) == "range size: 8 changed lines (5 added, 3 deleted), not counting third_party/ or testdata/");

    CHECK_FALSE(large_range_line(RangeSize{400, 0}).has_value());
    CHECK(large_range_line(RangeSize{300, 101}) ==
          std::string("large range: 401 changed lines is over the 400-line threshold. Split this range into "
                      "smaller merges before review. This is a warning, not a gate."));

    // The same count from git on the fixture's range.
    Fixture fx;
    Repo repo(fx.dir);
    const Range r = resolve_range(repo, "main...feature", false, "");
    const RangeSize g = range_size(repo, r.base, r.tip);
    CHECK(g.added == 5);
    CHECK(g.deleted == 3);
}

TEST_CASE("precheck: items print in kind order, then file, line and text") {
    Items items;
    items.add("B", "src/foo.h", 3, "x", "why b");
    items.add("C", "src/z.cpp", 9, "y", "why c");
    items.add("B", "src/foo_bar.h", 7, "x", "why b2");
    items.add("B", "src/foo.h", 3, "x", "a repeat");
    items.add("F", "src/a.cpp", 1, "z", "why f");
    items.add("B", "src/Foo.h", 2, "x", "why b3");
    const std::vector<Item> sorted = items.sorted();
    std::vector<std::string> lines;
    for (const Item& i : sorted) lines.push_back(item_line(i));
    // "_" sorts before "." and case is ignored, as Sort-Object had it.
    CHECK(lines == std::vector<std::string>{
                       "C src/z.cpp:9  y  -- why c",
                       "B src/foo_bar.h:7  x  -- why b2",
                       "B src/Foo.h:2  x  -- why b3",
                       "B src/foo.h:3  x  -- why b",
                       "F src/a.cpp:1  z  -- why f",
                   });
    Range r;
    r.spelling = "main...HEAD";
    r.tip = "0123456789abcdef";
    CHECK(summary_line(sorted, r) == "derive_once_precheck: 5 items (C 1, B 3, F 1) for main...HEAD at 0123456");
    r.whole_tree = true;
    CHECK(summary_line({}, r) == "derive_once_precheck: 0 items (none) for the whole tree at 0123456");
}

TEST_CASE("precheck: --disable takes only check numbers 1 to the number of checks") {
    CHECK(kChecks.size() == 5);
    CHECK(parse_disable({"2", "1,3", ",5,"}) == std::set<int>{1, 2, 3, 5});
    CHECK_THROWS_WITH(parse_disable({"6"}), "--disable takes check numbers 1 to 5, not '6'");
    CHECK_THROWS_WITH(parse_disable({"1,0"}), "--disable takes check numbers 1 to 5, not '0'");
    CHECK_THROWS_WITH(parse_disable({"two"}), "--disable takes check numbers 1 to 5, not 'two'");

    Fixture fx;
    const ExeRun bad = run_precheck({"--repo", fx.dir, "--range", "main...feature", "--disable", "1,9"});
    CHECK(bad.exit_code != 0);
    CHECK(bad.err == "--disable takes check numbers 1 to 5, not '9'\n");
    CHECK(bad.out.empty());
}

TEST_CASE("precheck: the exe prints the size and the summary for a range") {
    Fixture fx;
    Repo repo(fx.dir);
    const std::string tip7 = repo.resolve_commit("feature").substr(0, 7);
    const ExeRun r = run_precheck({"--repo", fx.dir, "--range", "main...feature", "--disable", "2"});
    CHECK(r.exit_code == 0);
    CHECK(r.err.empty());
    CHECK(r.out ==
          "range size: 8 changed lines (5 added, 3 deleted), not counting third_party/ or testdata/\n"
          "derive_once_precheck: 0 items (none) for main...feature at " + tip7 + "\n");

    const ExeRun whole = run_precheck({"--repo", fx.dir, "--whole-tree", "feature"});
    CHECK(whole.exit_code == 0);
    CHECK(whole.out == "derive_once_precheck: 0 items (none) for the whole tree at " + tip7 + "\n");

    const ExeRun list = run_precheck({"--list-checks"});
    CHECK(list.exit_code == 0);
    CHECK(list.out.substr(0, 2) == "1 ");
    CHECK(list.out.find("\n5 ") != std::string::npos);

    // Nothing in the fixture changed.
    CHECK(run_git(fx.dir, {"status", "--porcelain"}).empty());

    const std::string plain = testtemp::temp_dir("precheck_not_a_repo");
    const ExeRun no_repo = run_precheck({"--repo", plain});
    CHECK(no_repo.exit_code != 0);
    CHECK(no_repo.err == "not a git repository: " + plain + "\n");
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
    const ExeRun r = run_precheck({"--code-mask", dir + "\\x.cpp"});
    CHECK(r.exit_code == 0);
    CHECK(r.out == want);
    testtemp::remove_dir(dir);
}
