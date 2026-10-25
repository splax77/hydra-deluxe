// The changed lines the merge-test picker reads (tools/precheck/range.h):
// the deleted side of the one -U0 diff parser, in the base's numbering, beside
// the added side in the tip's; and a merge commit given alone as the range.
#include "doctest.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/git_text.h"
#include "precheck/range.h"
#include "precheck_fixture.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using namespace precheck;
using namespace precheck_fixture;

namespace {

struct TempRepo {
    std::string dir;
    explicit TempRepo(const std::string& name) : dir(testtemp::temp_dir(name)) {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
    }
    ~TempRepo() { testtemp::remove_dir(dir); }
};

std::vector<int> as_vector(const std::set<int>& s) { return std::vector<int>(s.begin(), s.end()); }

}  // namespace

TEST_CASE("precheck pick: a deleted class has its lines in the base's numbering") {
    TempRepo fx("precheck_pick_delete");
    commit_texts(fx.dir,
                 {{"src/a.cpp", "int keep;\nclass Gone {\n    int x;\n};\nint tail;\n"},
                  {"tests/old.cpp", "int o1;\nint o2;\n"}},
                 "base");
    // The tip drops class Gone (base lines 2-4) and adds "int added;" as its
    // own line 3, in another hunk; tests/old.cpp goes away.
    write_fixture_file(fx.dir, "src/a.cpp", "int keep;\nint tail;\nint added;\n");
    fs::remove(hydra::os_path(fx.dir + "\\tests\\old.cpp"));
    commit_all(fx.dir, "tip");

    Repo repo(fx.dir);
    const Range r = resolve_range(repo, "main", false, "");
    const ChangedLines c = ChangedLines::from_diff(repo, r.base, r.tip);

    CHECK(c.added.lines("src/a.cpp", 100) == std::vector<int>{3});
    REQUIRE(c.deleted.count("src/a.cpp") == 1);
    CHECK(as_vector(c.deleted.at("src/a.cpp")) == std::vector<int>{2, 3, 4});
    // A deleted file is keyed by its base path and has nothing added.
    REQUIRE(c.deleted.count("tests/old.cpp") == 1);
    CHECK(as_vector(c.deleted.at("tests/old.cpp")) == std::vector<int>{1, 2});
    CHECK_FALSE(c.added.has_file("tests/old.cpp"));

    // AddedLines is the added side of the same parse.
    const AddedLines a = AddedLines::from_diff(repo, r.base, r.tip);
    CHECK(a.files() == c.added.files());
    CHECK(a.lines("src/a.cpp", 100) == std::vector<int>{3});
}

TEST_CASE("precheck pick: a hunk that deletes and adds records both sides") {
    TempRepo fx("precheck_pick_hunk");
    commit_texts(fx.dir, {{"src/h.cpp", "l1\nl2\nl3\nl4\nl5\n"}}, "base");
    // One line goes in on top (tip line 1); base lines 2-4 become one new
    // line, which is tip line 3 because of the line above.
    write_fixture_file(fx.dir, "src/h.cpp", "l0\nl1\nN\nl5\n");
    commit_all(fx.dir, "tip");

    Repo repo(fx.dir);
    const Range r = resolve_range(repo, "main", false, "");
    const ChangedLines c = ChangedLines::from_diff(repo, r.base, r.tip);
    CHECK(c.added.lines("src/h.cpp", 100) == std::vector<int>{1, 3});
    REQUIRE(c.deleted.count("src/h.cpp") == 1);
    CHECK(as_vector(c.deleted.at("src/h.cpp")) == std::vector<int>{2, 3, 4});
}

TEST_CASE("precheck pick: body lines that look like file headers stay body lines") {
    // A deleted "-- x" shows as "--- x" and an added "++ y" as "+++ y".
    const std::vector<std::string> diff = {
        "diff --git a/src/q.cpp b/src/q.cpp",
        "--- a/src/q.cpp",
        "+++ b/src/q.cpp",
        "@@ -7,2 +7 @@ int f() {",
        "--- x",
        "-int gone;",
        "+++ y",
    };
    const ChangedLines c = ChangedLines::parse_diff(diff);
    CHECK(c.added.files() == std::vector<std::string>{"src/q.cpp"});
    CHECK(c.added.lines("src/q.cpp", 100) == std::vector<int>{7});
    CHECK(c.deleted.size() == 1);
    REQUIRE(c.deleted.count("src/q.cpp") == 1);
    CHECK(as_vector(c.deleted.at("src/q.cpp")) == std::vector<int>{7, 8});
}

TEST_CASE("precheck pick: a merge commit alone resolves to first parent...second parent") {
    TempRepo fx("precheck_pick_merge");
    commit_texts(fx.dir, {{"src/m.cpp", "int one;\nint two;\n"}}, "main1");
    fixture_git(fx.dir, {"checkout", "-q", "-b", "feature"});
    commit_texts(fx.dir, {{"src/f.cpp", "int f;\n"}}, "f1");
    fixture_git(fx.dir, {"checkout", "-q", "main"});
    commit_texts(fx.dir, {{"src/m.cpp", "int uno;\nint two;\n"}}, "main2");
    fixture_git(fx.dir, {"merge", "-q", "--no-ff", "-m", "merge", "feature"});

    Repo repo(fx.dir);
    const std::string main1 = repo.resolve_commit("main~1~1");
    const std::string main2 = repo.resolve_commit("main^1");
    const std::string f1 = repo.resolve_commit("feature");
    const std::string merge = repo.resolve_commit("main");

    const Range m = resolve_range(repo, merge, false, "");
    CHECK(m.left == main2);
    CHECK(m.tip == f1);
    CHECK(m.base == main1);  // where feature forked
    CHECK(m.rules_rev == f1);

    // The reviewed range adds feature's line only, not main2's edit.
    const ChangedLines c = ChangedLines::from_diff(repo, m.base, m.tip);
    CHECK(c.added.files() == std::vector<std::string>{"src/f.cpp"});
    CHECK(c.deleted.empty());

    // A commit with one parent still resolves over that parent.
    const Range one = resolve_range(repo, "main^1", false, "");
    CHECK(one.tip == main2);
    CHECK(one.left.empty());
    CHECK(one.base == main1);
}
