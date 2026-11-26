// The merge-test picker (tools/precheck/test_pick.h) on fixture repos: one
// level of callers with the selector values, deleted names, helper headers,
// changed test files, the three limits and the shared-code paths. Cases (j)
// and (k) follow the user's later rulings (D299). Cases (a) to (i) are Task 3 of
// docs/superpowers/plans/2026-10-25-merge-test-picker.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-25-merge-test-picker.md).
#include "doctest.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "core/strutil.h"
#include "core/winstr.h"
#include "precheck/git_text.h"
#include "precheck/range.h"
#include "precheck/test_pick.h"
#include "precheck_fixture.h"
#include "source_tree.h"
#include "temp_util.h"

namespace fs = std::filesystem;
using namespace precheck;
using namespace precheck_fixture;

namespace {

// A fixture repo whose main branch holds `base`; pick() commits the tip on
// top and picks for that one commit.
struct PickRepo {
    std::string dir;
    PickRepo(const std::string& name, const std::map<std::string, std::string>& base)
        : dir(testtemp::temp_dir(name)) {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        commit_texts(dir, base, "base");
    }
    ~PickRepo() { testtemp::remove_dir(dir); }

    TestPick pick(const std::map<std::string, std::string>& tip, const PickLimits& limits = {}) const {
        commit_texts(dir, tip, "tip");
        Repo repo(dir);
        return pick_tests(repo, resolve_range(repo, "HEAD", false, ""), limits);
    }
};

std::vector<std::string> picked_files(const TestPick& p) {
    std::vector<std::string> out;
    for (const PickedTest& t : p.tests) out.push_back(t.file);
    return out;
}

std::vector<std::string> reasons_of(const TestPick& p, const std::string& file) {
    for (const PickedTest& t : p.tests)
        if (t.file == file) return t.reasons;
    return {};
}

const PickName* name_in(const TestPick& p, const std::string& name) {
    for (const PickName& n : p.names)
        if (n.name == name) return &n;
    return nullptr;
}

const std::string kRunBase =
    "enum class Mode { A, B };\n"
    "void a() { int x = 1; }\n"
    "void b() { int y = 2; }\n"
    "void run(Mode m) { if (m == Mode::A) a(); else b(); }\n"
    "void outer() { run(Mode::B); }\n";

// Adds `count` test files to `files`, tests/test_<stem>100.cpp and up, each
// calling `callee`, and returns their paths in byte order.
std::vector<std::string> add_callers(std::map<std::string, std::string>& files, const std::string& stem,
                                     const std::string& callee, int count) {
    std::vector<std::string> paths;
    for (int i = 0; i < count; ++i) {
        const std::string id = stem + std::to_string(100 + i);
        const std::string path = "tests/test_" + id + ".cpp";
        files[path] = "int " + id + "() { return " + callee + "(); }\n";
        paths.push_back(path);
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

// A fixture where `count` test files each name one function of src/f.cpp,
// with no function named by more than kCommonNameLimit of them, so the
// common-name rule never fires and only the size limit can.
struct ManyNames {
    std::map<std::string, std::string> base;
    std::string tip;                 // src/f.cpp with every function changed
    std::vector<std::string> tests;  // every test file, in byte order
};

ManyNames many_names(int count) {
    ManyNames m;
    std::string base_src;
    for (int done = 0, fn = 0; done < count; ++fn) {
        const std::string name = "many" + std::to_string(fn);
        const int take = (std::min)(kCommonNameLimit, count - done);
        for (const std::string& t : add_callers(m.base, name, name, take)) m.tests.push_back(t);
        base_src += "int " + name + "() { return 1; }\n";
        m.tip += "int " + name + "() { return 2; }\n";
        done += take;
    }
    m.base["src/f.cpp"] = base_src;
    std::sort(m.tests.begin(), m.tests.end());
    return m;
}

}  // namespace

// The exe picks with a default PickLimits, so its three limits must be the
// picker's own constants. Cases (e) and (f) cross the first two at their real
// values.
TEST_CASE("precheck pick: the default limits are the picker's three constants") {
    const PickLimits d;
    CHECK(d.common_name == kCommonNameLimit);
    CHECK(d.pick_size == kPickSizeLimit);
    CHECK(d.header_includers == kHeaderIncluderLimit);
}

TEST_CASE("precheck pick (a): a test naming only the selector value of a changed function is picked") {
    PickRepo fx("precheck_pick_a",
                {{"src/run.cpp", kRunBase},
                 {"tests/test_mode.cpp", "int pick_mode() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_run.cpp", "void go_run() { run(Mode::B); }\n"},
                 {"tests/test_outer.cpp", "void go_outer() { outer(); }\n"},
                 {"tests/test_other.cpp", "int other() { return 7; }\n"}});
    std::string tip = kRunBase;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/run.cpp", tip}});

    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(p.names.size() == 1);
    CHECK(p.names[0].name == "a");
    CHECK(p.names[0].file == "src/run.cpp");
    CHECK_FALSE(p.names[0].base_only);

    REQUIRE(p.callers.size() == 2);
    CHECK(p.callers[0].key == "run");
    CHECK(p.callers[0].callee == "a");
    CHECK(p.callers[0].caller == "run");
    CHECK_FALSE(p.callers[0].selector);
    CHECK(p.callers[1].key == "Mode::A");
    CHECK(p.callers[1].callee == "a");
    CHECK(p.callers[1].caller == "run");
    CHECK(p.callers[1].selector);

    // test_outer names only outer, a caller two levels up: not followed.
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode.cpp", "tests/test_run.cpp"});
    CHECK(reasons_of(p, "tests/test_mode.cpp") == std::vector<std::string>{"Mode::A"});
    CHECK(reasons_of(p, "tests/test_run.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (a): a selector on the guard line above the call counts") {
    const std::string base =
        "enum class Rule { Old, New };\n"
        "void place() { int k = 1; }\n"
        "void check(Rule r) {\n"
        "    if (r == Rule::New) {\n"
        "        int z = 0;\n"
        "        place();\n"
        "    }\n"
        "}\n";
    PickRepo fx("precheck_pick_a_guard",
                {{"src/song.cpp", base}, {"tests/test_rule.cpp", "Rule pick_rule() { return Rule::New; }\n"}});
    std::string tip = base;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/song.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(reasons_of(p, "tests/test_rule.cpp") == std::vector<std::string>{"Rule::New"});
}

TEST_CASE("precheck pick (b): a deleted class is a name and its members are not keys") {
    const std::string base =
        "#pragma once\n"
        "class Square {\n"
        "public:\n"
        "    int side() const { return s_; }\n"
        "    int s_ = 0;\n"
        "};\n"
        "int keep_me();\n";
    PickRepo fx("precheck_pick_b", {{"src/shapes.h", base},
                                    {"tests/test_square.cpp", "int sq() { Square q; return 0; }\n"},
                                    {"tests/test_side.cpp", "int sd() { return side() + s_; }\n"}});
    const TestPick p = fx.pick({{"src/shapes.h", "#pragma once\nint keep_me();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    const PickName* sq = name_in(p, "Square");
    REQUIRE(sq != nullptr);
    CHECK(sq->base_only);
    CHECK(name_in(p, "side") == nullptr);
    CHECK(name_in(p, "s_") == nullptr);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_square.cpp"});
}

TEST_CASE("precheck pick (b): a deleted class's multi-line member body gives the class, not the member") {
    const std::string base =
        "#pragma once\n"
        "class Square {\n"
        "public:\n"
        "    int side() const {\n"
        "        const int twice = s_ * 2;\n"
        "        return twice / 2;\n"
        "    }\n"
        "    int s_ = 0;\n"
        "};\n"
        "int keep_me();\n";
    PickRepo fx("precheck_pick_b2", {{"src/shapes.h", base},
                                     {"tests/test_square.cpp", "int sq() { Square q; return 0; }\n"},
                                     {"tests/test_side.cpp", "int sd() { return side() + s_; }\n"}});
    const TestPick p = fx.pick({{"src/shapes.h", "#pragma once\nint keep_me();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(name_in(p, "Square") != nullptr);
    CHECK(name_in(p, "side") == nullptr);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_square.cpp"});
}

TEST_CASE("precheck pick (a): a function changed only by the comment above it does not walk its callers") {
    const std::string base =
        "void deep() { int x = 1; }\n"
        "// old words\n"
        "void shallow() { deep(); }\n"
        "void top() { shallow(); }\n";
    PickRepo fx("precheck_pick_a3", {{"src/chain.cpp", base},
                                     {"tests/test_shallow.cpp", "void t1() { shallow(); }\n"},
                                     {"tests/test_top.cpp", "void t2() { top(); }\n"}});
    std::string tip = base;
    tip.replace(tip.find("old words"), 9, "new words");
    const TestPick p = fx.pick({{"src/chain.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(p.names.size() == 1);
    CHECK(p.names[0].name == "shallow");
    CHECK(p.names[0].kind == NameKind::Comment);
    CHECK(p.callers.empty());
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_shallow.cpp"});
}

TEST_CASE("precheck pick (a): a call to an unrelated function of the same name is not a caller") {
    const std::string one_cpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_scope",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/user.cpp",
                  "#include \"one.h\"\nvoid use(Mode m) {\n    if (m == Mode::A) {\n        run();\n    }\n}\n"},
                 {"src/b/two.cpp",
                  "enum class Kind { A, B };\nstatic void run() { int y = 2; }\nvoid go(Kind k) {\n"
                  "    if (k == Kind::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_a.cpp",
                  "#include \"a/one.h\"\nint ma() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_kind_b.cpp", "int kb() { return static_cast<int>(Kind::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    REQUIRE(name_in(p, "run") != nullptr);
    for (const CallerKey& k : p.callers) CHECK(k.caller != "go");
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_a.cpp"});
    CHECK(reasons_of(p, "tests/test_mode_a.cpp") == std::vector<std::string>{"Mode::A"});
}

TEST_CASE("precheck pick (a): a caller that includes the declaring header through another header counts") {
    const std::string one_cpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_chain",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/mid.h", "#pragma once\n#include \"one.h\"\n"},
                 {"src/c/far.cpp",
                  "#include \"a/mid.h\"\nvoid far(Mode m) {\n    if (m == Mode::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_b.cpp",
                  "#include \"a/one.h\"\nint mb() { return static_cast<int>(Mode::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_b.cpp"});
    CHECK(reasons_of(p, "tests/test_mode_b.cpp") == std::vector<std::string>{"Mode::B"});
}

TEST_CASE("precheck pick (a): a .cpp that reaches its declaring header only through another header still has its callers") {
    const std::string one_cpp = "#include \"mid.h\"\nvoid run() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_indirect",
                {{"src/a/one.h", "#pragma once\nenum class Mode { A, B };\nvoid run();\n"},
                 {"src/a/mid.h", "#pragma once\n#include \"one.h\"\n"},
                 {"src/a/one.cpp", one_cpp},
                 {"src/a/user.cpp",
                  "#include \"one.h\"\nvoid use(Mode m) {\n    if (m == Mode::B) {\n        run();\n    }\n}\n"},
                 {"tests/test_mode_b.cpp",
                  "#include \"a/one.h\"\nint mb() { return static_cast<int>(Mode::B); }\n"}});
    std::string tip = one_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/a/one.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_b.cpp"});
}

TEST_CASE("precheck pick (a): a function only in a .cpp has callers only in that file") {
    const std::string here =
        "enum class Mode { A, B };\nstatic void helper() {\n    int x = 1;\n}\n"
        "void inside(Mode m) {\n    if (m == Mode::A) {\n        helper();\n    }\n}\n";
    PickRepo fx("precheck_pick_a_static",
                {{"src/s/here.cpp", here},
                 {"src/s/near.h", "#pragma once\nenum class Kind { A, B };\n"},
                 {"src/s/there.cpp",
                  "#include \"near.h\"\nvoid helper();\nvoid outside(Kind k) {\n    if (k == Kind::B) {\n"
                  "        helper();\n    }\n}\n"},
                 {"tests/test_mode_a.cpp", "int ma() { return static_cast<int>(Mode::A); }\n"},
                 {"tests/test_kind_b.cpp", "int kb() { return static_cast<int>(Kind::B); }\n"}});
    std::string tip = here;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"src/s/here.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_mode_a.cpp"});
}

TEST_CASE("precheck pick (a): a tools/ function's callers in tools/ are followed") {
    const std::string lib_cpp = "#include \"lib.h\"\nvoid step() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_a_tools",
                {{"tools/x/lib.h", "#pragma once\nenum class Opt { Fast, Slow };\nvoid step();\n"},
                 {"tools/x/lib.cpp", lib_cpp},
                 {"tools/x/main.cpp",
                  "#include \"lib.h\"\nvoid drive(Opt o) {\n    if (o == Opt::Fast) {\n        step();\n    }\n}\n"},
                 {"tests/test_fast.cpp", "#include \"x/lib.h\"\nint f() { return static_cast<int>(Opt::Fast); }\n"}});
    std::string tip = lib_cpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    const TestPick p = fx.pick({{"tools/x/lib.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_fast.cpp"});
}

TEST_CASE("precheck pick (c): a key in a helper header picks every test file that includes it") {
    PickRepo fx("precheck_pick_c",
                {{"src/calc.cpp", "int scale(int v) { return v * 2; }\n"},
                 {"tests/calc_helpers.h", "#pragma once\ninline int twice(int v) { return scale(v); }\n"},
                 {"tests/test_c1.cpp", "#include \"calc_helpers.h\"\nint c1() { return twice(1); }\n"},
                 {"tests/test_c2.cpp", "#include \"calc_helpers.h\"\nint c2() { return twice(2); }\n"},
                 {"tests/test_c3.cpp", "int c3() { return 3; }\n"}});
    const TestPick p = fx.pick({{"src/calc.cpp", "int scale(int v) { return v * 3; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_c1.cpp", "tests/test_c2.cpp"});
    CHECK(reasons_of(p, "tests/test_c1.cpp") == std::vector<std::string>{"scale"});
}

TEST_CASE("precheck pick (c): a key only a comment in a helper header names picks nothing") {
    PickRepo fx("precheck_pick_c_comment",
                {{"src/calc.cpp", "int scale(int v) { return v * 2; }\n"},
                 {"tests/calc_helpers.h", "#pragma once\n// Built on scale.\ninline int twice(int v) { return v; }\n"},
                 {"tests/test_c1.cpp", "#include \"calc_helpers.h\"\nint c1() { return twice(1); }\n"}});
    const TestPick p = fx.pick({{"src/calc.cpp", "int scale(int v) { return v * 3; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    // The picker saw the change, so an empty pick means the comment was not
    // counted, not that nothing was looked for.
    REQUIRE(name_in(p, "scale") != nullptr);
    CHECK(p.tests.empty());
}

// The picker's reason wording has no public owner, so this case reads it from
// the picker's own output: a changed test file's one reason is the range
// change, and the includer of a changed helper header gives the same reason
// with that header's path in front.
TEST_CASE("precheck pick (d): a test file the range changes is picked") {
    PickRepo fx("precheck_pick_d", {{"tests/test_d.cpp", "int d1() { return 1; }\n"},
                                    {"tests/d_helpers.h", "#pragma once\n// one\n"},
                                    {"tests/test_dh.cpp", "#include \"d_helpers.h\"\nint dh1() { return 1; }\n"},
                                    {"tests/test_e.cpp", "int e1() { return 1; }\n"}});
    const TestPick p = fx.pick({{"tests/test_d.cpp", "int d1() { return 2; }\n"},
                                {"tests/d_helpers.h", "#pragma once\n// two\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_d.cpp", "tests/test_dh.cpp"});
    const std::vector<std::string> direct = reasons_of(p, "tests/test_d.cpp");
    REQUIRE(direct.size() == 1);
    CHECK_FALSE(direct.front().empty());
    CHECK(name_in(p, direct.front()) == nullptr);
    CHECK(reasons_of(p, "tests/test_dh.cpp") == std::vector<std::string>{"tests/d_helpers.h " + direct.front()});
}

// Both cases run at the exe's own limits, kCommonNameLimit and
// kPickSizeLimit, with one more test file than the limit or exactly as many.
TEST_CASE("precheck pick (e): a common caller key picks nothing; a common changed name runs everything") {
    const std::string base = "int leaf() { return 1; }\nint hub() { return leaf(); }\n";
    const std::string hub_changed = "int leaf() { return 1; }\nint hub() { return leaf() + 1; }\n";
    std::map<std::string, std::string> files = {{"src/w.cpp", base},
                                                {"tests/test_e4.cpp", "int e4() { return leaf(); }\n"}};

    SUBCASE("the common key is a caller") {
        add_callers(files, "hub", "hub", kCommonNameLimit + 1);
        PickRepo fx("precheck_pick_e1", files);
        const TestPick p = fx.pick({{"src/w.cpp", "int leaf() { return 2; }\nint hub() { return leaf(); }\n"}});
        REQUIRE_FALSE(p.whole_suite);
        REQUIRE(p.common.size() == 1);
        CHECK(p.common[0].name == "hub");
        CHECK(p.common[0].files == kCommonNameLimit + 1);
        CHECK(picked_files(p) == std::vector<std::string>{"tests/test_e4.cpp"});
    }
    SUBCASE("the common key heads a changed definition") {
        add_callers(files, "hub", "hub", kCommonNameLimit + 1);
        PickRepo fx("precheck_pick_e2", files);
        const TestPick p = fx.pick({{"src/w.cpp", hub_changed}});
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "src/w.cpp");
        CHECK(p.whole_suite->count == kCommonNameLimit + 1);
        CHECK(p.tests.empty());
    }
    SUBCASE("at the limit the key still picks") {
        const std::vector<std::string> callers = add_callers(files, "hub", "hub", kCommonNameLimit);
        PickRepo fx("precheck_pick_e3", files);
        const TestPick p = fx.pick({{"src/w.cpp", hub_changed}});
        REQUIRE_FALSE(p.whole_suite);
        CHECK(p.common.empty());
        CHECK(picked_files(p) == callers);
    }
}

TEST_CASE("precheck pick (f): more picked files than the size limit runs the whole suite") {
    SUBCASE("over") {
        const ManyNames m = many_names(kPickSizeLimit + 1);
        PickRepo fx("precheck_pick_f1", m.base);
        const TestPick p = fx.pick({{"src/f.cpp", m.tip}});
        REQUIRE(p.common.empty());
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file.empty());
        CHECK(p.whole_suite->count == kPickSizeLimit + 1);
        CHECK(p.tests.empty());
    }
    SUBCASE("at the limit") {
        const ManyNames m = many_names(kPickSizeLimit);
        PickRepo fx("precheck_pick_f2", m.base);
        const TestPick p = fx.pick({{"src/f.cpp", m.tip}});
        REQUIRE_FALSE(p.whole_suite);
        CHECK(picked_files(p) == m.tests);
    }
}

TEST_CASE("precheck pick (g): a header with more includers than the limit runs the whole suite") {
    const std::map<std::string, std::string> files = {
        {"src/wide.h", "#pragma once\nint wide();\n"},
        {"src/u1.cpp", "#include \"wide.h\"\nint u1() { return wide(); }\n"},
        {"src/sub/u2.cpp", "#include \"wide.h\"\nint u2() { return 2; }\n"},
        {"tools/u3.cpp", "#include \"../src/wide.h\"\nint u3() { return 3; }\n"},
        {"tools/u4.cpp", "// #include \"wide.h\" is not a real include\nint u4() { return 4; }\n"}};
    const std::map<std::string, std::string> tip = {{"src/wide.h", "#pragma once\nint wide();\nint wider();\n"}};
    PickLimits tight;
    SUBCASE("over") {
        tight.header_includers = 2;
        PickRepo fx("precheck_pick_g1", files);
        const TestPick p = fx.pick(tip, tight);
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "src/wide.h");
        CHECK(p.whole_suite->count == 3);
    }
    SUBCASE("at the limit") {
        tight.header_includers = 3;
        PickRepo fx("precheck_pick_g2", files);
        CHECK_FALSE(fx.pick(tip, tight).whole_suite);
    }
}

namespace {

// shared_code on paths in a repo that holds nothing that includes them.
struct SharedProbe {
    PickRepo fx{"precheck_pick_h", {{"src/x.cpp", "int x;\n"}}};
    Repo repo{fx.dir};
    std::optional<WholeSuite> of(const std::string& path) { return shared_code(repo, "HEAD", path); }
};

std::optional<WholeSuite> shared_code_of(const std::string& path) { return SharedProbe().of(path); }

// git's answer to `args`, asked in the real repo the tests were built from.
std::vector<std::string> real_git(const std::vector<std::string>& args) {
    return run_git(sourcetree::root().u8string(), args);
}

// The real repo's committed text of `path`, one entry per line.
std::vector<std::string> real_lines(const std::string& path) { return real_git({"show", "HEAD:" + path}); }

// Every committed file under `dir` in the real repo.
std::vector<std::string> files_in(const std::string& dir) {
    return real_git({"ls-tree", "-r", "--name-only", "HEAD", "--", dir});
}

// What follows `call` on each line of `lines` that starts with it after its
// indent.
std::vector<std::string> after_calls(const std::vector<std::string>& lines, const std::string& call) {
    std::vector<std::string> out;
    for (const std::string& line : lines) {
        const std::string text = hydra::trim(line);
        if (hydra::starts_with(text, call)) out.push_back(text.substr(call.size()));
    }
    return out;
}

// `rest` up to its first space or ")".
std::string first_word(const std::string& rest) { return rest.substr(0, rest.find_first_of(" )")); }

// The files GitHub Actions runs: every committed file in .github/workflows/.
std::vector<std::string> real_workflows() { return files_in(".github/workflows"); }

// The line that gives a doctest harness its main(). It is split so this
// file's own text never matches it.
const std::string kImplementLine = std::string("#define DOCTEST_CONFIG_") + "IMPLEMENT";

// The harness main files: each committed file under tests/ with a line that
// is exactly kImplementLine. git grep has no whole-line switch, so the
// pattern is anchored at both ends; its text holds no character a basic
// regex treats as special.
std::vector<std::string> harness_mains() {
    return real_git({"grep", "-l", "--cached", "-e", "^" + kImplementLine + "$", "--", "tests"});
}

// The folders the root CMakeLists.txt adds to hydra_tests' include path: the
// words of each target_include_directories(hydra_tests PRIVATE ...) call,
// on its own line or on the lines up to its ")".
std::vector<std::string> test_include_dirs() {
    const std::string call = "target_include_directories(hydra_tests PRIVATE";
    std::vector<std::string> dirs;
    bool open = false;
    for (const std::string& line : real_lines("CMakeLists.txt")) {
        const std::string text = hydra::trim(line);
        if (open) {
            if (text == ")")
                open = false;
            else if (!text.empty())
                dirs.push_back(first_word(text));
            continue;
        }
        if (!hydra::starts_with(text, call)) continue;
        const std::string rest = hydra::trim(text.substr(call.size()));
        if (rest.empty())
            open = true;
        else
            dirs.push_back(first_word(rest));
    }
    return dirs;
}

// The test framework's folders: for each harness main, the include folder of
// hydra_tests that holds the header it includes on the line after
// kImplementLine.
std::vector<std::string> framework_dirs() {
    const std::string include = "#include \"";
    const std::vector<std::string> dirs = test_include_dirs();
    std::vector<std::string> out;
    for (const std::string& main_file : harness_mains()) {
        CAPTURE(main_file);
        const std::vector<std::string> lines = real_lines(main_file);
        const auto define = std::find_if(lines.begin(), lines.end(),
                                         [](const std::string& l) { return hydra::trim(l) == kImplementLine; });
        REQUIRE(define != lines.end());
        REQUIRE(define + 1 != lines.end());
        const std::string next = hydra::trim(*(define + 1));
        REQUIRE(hydra::starts_with(next, include));
        const std::string header = next.substr(include.size(), next.find('"', include.size()) - include.size());
        for (const std::string& dir : dirs) {
            const std::vector<std::string> held = files_in(dir);
            if (std::find(held.begin(), held.end(), dir + "/" + header) != held.end()) out.push_back(dir);
        }
    }
    return out;
}

// Checks that `files` is not empty and that each is a real committed file the
// picker counts as shared code.
void check_real_shared(const std::vector<std::string>& files) {
    REQUIRE_FALSE(files.empty());
    SharedProbe probe;
    for (const std::string& file : files) {
        CAPTURE(file);
        REQUIRE_FALSE(real_lines(file).empty());
        CHECK(probe.of(file));
    }
}

}  // namespace

// The cases below find each file from the real build that reads it, so none
// of them restates the picker's table of shared paths.

// CMake starts at the root CMakeLists.txt and reads the CMakeLists.txt of
// every folder it adds.
TEST_CASE("precheck pick (h): every CMakeLists.txt the real build reads runs the whole suite") {
    const std::vector<std::string> root = real_lines("CMakeLists.txt");
    REQUIRE_FALSE(root.empty());
    CHECK(shared_code_of("CMakeLists.txt"));
    std::vector<std::string> added;
    for (const std::string& rest : after_calls(root, "add_subdirectory("))
        added.push_back(first_word(rest) + "/CMakeLists.txt");
    check_real_shared(added);
}

// The presets file is the one the build's preset reader opens.
TEST_CASE("precheck pick (h): the presets file the build reads runs the whole suite") {
    const std::string opens = "Join-Path $Repo \"";
    std::vector<std::string> files;
    for (const std::string& line : real_lines("tools/cmake_presets.ps1")) {
        const size_t at = line.find(opens);
        if (at == std::string::npos) continue;
        const size_t from = at + opens.size();
        files.push_back(line.substr(from, line.find('"', from) - from));
    }
    check_real_shared(files);
}

// The build script is the one every workflow step that builds hydra_tests
// runs.
TEST_CASE("precheck pick (h): the build script CI runs makes the whole suite run") {
    std::vector<std::string> scripts;
    for (const std::string& workflow : real_workflows())
        for (const std::string& line : real_lines(workflow)) {
            if (line.find("-Target hydra_tests") == std::string::npos) continue;
            const size_t at = line.find(".\\");
            REQUIRE(at != std::string::npos);
            const size_t end = line.find(".ps1", at);
            REQUIRE(end != std::string::npos);
            scripts.push_back(hydra::replace_all(line.substr(at + 2, end + 4 - (at + 2)), "\\", "/"));
        }
    check_real_shared(scripts);
}

// GitHub Actions runs every file in .github/workflows/.
TEST_CASE("precheck pick (h): every real workflow file runs the whole suite") { check_real_shared(real_workflows()); }

// The file that defines DOCTEST_CONFIG_IMPLEMENT gives every hydra_tests case
// its main().
TEST_CASE("precheck pick (h): the test harness's main file runs the whole suite") {
    check_real_shared(harness_mains());
}

// The shard list is the file the root CMakeLists.txt reads line by line into
// a list it loops over to register the hydra_tests slices.
TEST_CASE("precheck pick (h): the shard list the build reads runs the whole suite") {
    const std::vector<std::string> root = real_lines("CMakeLists.txt");
    std::vector<std::string> sliced;
    for (const std::string& rest : after_calls(root, "file(STRINGS ${CMAKE_SOURCE_DIR}/")) {
        const size_t gap = rest.find(' ');
        if (gap == std::string::npos) continue;
        const std::string file = first_word(rest);
        const std::string loop = "IN LISTS " + first_word(rest.substr(gap + 1)) + ")";
        if (std::any_of(root.begin(), root.end(),
                        [&](const std::string& line) { return line.find(loop) != std::string::npos; }))
            sliced.push_back(file);
    }
    check_real_shared(sliced);
}

// The test framework is the header the harness main includes for its main(),
// and every file in its folder ships with it.
TEST_CASE("precheck pick (h): every file of the test framework the harness includes runs the whole suite") {
    const std::vector<std::string> dirs = framework_dirs();
    REQUIRE_FALSE(dirs.empty());
    for (const std::string& dir : dirs) check_real_shared(files_in(dir));
}

// Real files the build reads that are no build file, no harness file and no
// header anything includes: the root folder's .md pages, every test .cpp but
// the harness main, every .cpp under src/, and every file of hydra_tests'
// include folders other than the framework's, none of which holds a build
// file.
TEST_CASE("precheck pick (h): other files are not shared code") {
    const std::vector<std::string> mains = harness_mains();
    const std::vector<std::string> framework = framework_dirs();
    auto listed = [](const std::vector<std::string>& in, const std::string& f) {
        return std::find(in.begin(), in.end(), f) != in.end();
    };
    std::vector<std::string> files;
    for (const std::string& f : real_git({"ls-tree", "--name-only", "HEAD"}))
        if (hydra::ends_with(f, ".md")) files.push_back(f);
    for (const std::string& f : files_in("tests"))
        if (is_cpp_implementation(f) && !listed(mains, f)) files.push_back(f);
    for (const std::string& f : files_in("src"))
        if (is_cpp_implementation(f)) files.push_back(f);
    for (const std::string& dir : test_include_dirs()) {
        if (listed(framework, dir)) continue;
        for (const std::string& f : files_in(dir)) files.push_back(f);
    }
    REQUIRE_FALSE(files.empty());
    SharedProbe probe;
    for (const std::string& file : files) {
        CAPTURE(file);
        CHECK_FALSE(probe.of(file));
    }
}
TEST_CASE("precheck pick (h): a changed build file makes the pick the whole suite") {
    PickRepo fx("precheck_pick_h_pick", {{"CMakeLists.txt", "project(x)\n"}, {"tests/test_h.cpp", "int h;\n"}});
    const TestPick p = fx.pick({{"CMakeLists.txt", "project(y)\n"}, {"tests/test_h.cpp", "int h2;\n"}});
    REQUIRE(p.whole_suite);
    CHECK(p.whole_suite->file == "CMakeLists.txt");
    CHECK(p.tests.empty());
}

namespace {

// A root build file whose project() line holds the version and passes it to
// the code as FX_VERSION, a name the picker can only learn from this file.
const std::string kVersionCmake =
    "project(Fx VERSION 1.2.0 LANGUAGES CXX)\n"
    "add_library(fx_core src/core.cpp)\n"
    "target_compile_definitions(fx_core PUBLIC FX_VERSION=\"${PROJECT_VERSION}\")\n";

const std::map<std::string, std::string> kVersionBase = {
    {"CMakeLists.txt", kVersionCmake},
    {"src/core.cpp", "int core() { return 1; }\n"},
    {"tests/test_version.cpp", "const char* v() { return FX_VERSION; }\n"},
    {"tests/test_other.cpp", "int o() { return 2; }\n"}};

std::string bumped(const std::string& cmake) { return hydra::replace_all(cmake, "VERSION 1.2.0", "VERSION 1.3.0"); }

}  // namespace

TEST_CASE("precheck pick (h): a version bump alone in the root build file picks the tests naming the version") {
    PickRepo fx("precheck_pick_h_version", kVersionBase);
    const TestPick p = fx.pick({{"CMakeLists.txt", bumped(kVersionCmake)}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_version.cpp"});
    const std::vector<std::string> why = reasons_of(p, "tests/test_version.cpp");
    REQUIRE(why.size() == 1);
    CHECK(hydra::starts_with(why.front(), "FX_VERSION"));
}

TEST_CASE("precheck pick (h): a version bump beside another root build file edit still runs the whole suite") {
    PickRepo fx("precheck_pick_h_version_more", kVersionBase);
    const TestPick p = fx.pick({{"CMakeLists.txt", bumped(kVersionCmake) + "add_compile_options(-Wall)\n"}});
    REQUIRE(p.whole_suite);
    CHECK(p.whole_suite->file == "CMakeLists.txt");
    CHECK(p.tests.empty());
}

TEST_CASE("precheck pick (h): an edit of the project() line other than its version still runs the whole suite") {
    const std::string more_languages = hydra::replace_all(kVersionCmake, "LANGUAGES CXX", "LANGUAGES C CXX");
    SUBCASE("the languages alone") {
        PickRepo fx("precheck_pick_h_project_lang", kVersionBase);
        const TestPick p = fx.pick({{"CMakeLists.txt", more_languages}});
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "CMakeLists.txt");
    }
    SUBCASE("the languages and the version on the same line") {
        PickRepo fx("precheck_pick_h_project_both", kVersionBase);
        const TestPick p = fx.pick({{"CMakeLists.txt", bumped(more_languages)}});
        REQUIRE(p.whole_suite);
        CHECK(p.whole_suite->file == "CMakeLists.txt");
    }
}

TEST_CASE("precheck pick (h): a version bump in a CMakeLists.txt below the root still runs the whole suite") {
    std::map<std::string, std::string> base = kVersionBase;
    base["tools/fx/CMakeLists.txt"] = kVersionCmake;
    PickRepo fx("precheck_pick_h_version_sub", base);
    const TestPick p = fx.pick({{"tools/fx/CMakeLists.txt", bumped(kVersionCmake)}});
    REQUIRE(p.whole_suite);
    CHECK(p.whole_suite->file == "tools/fx/CMakeLists.txt");
}

TEST_CASE("precheck pick (i): files under tests/single_owner/ are never picked") {
    PickRepo fx("precheck_pick_i",
                {{"src/k.cpp", "constexpr int kStamp = 1;\n"},
                 {"tests/single_owner/special.cpp", "int s() { return kStamp; }\n"},
                 {"tests/single_owner/other.cpp", "int o;\n"},
                 {"tests/test_k.cpp", "int k() { return kStamp; }\n"}});
    const TestPick p =
        fx.pick({{"src/k.cpp", "constexpr int kStamp = 2;\n"}, {"tests/single_owner/other.cpp", "int o2;\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_k.cpp"});
}

// Step 5 under the user's later ruling on it (D299): a test file counts for a
// name only if it reaches the header declaring it.
namespace {

const std::string kOneCpp = "#include \"one.h\"\nvoid run() {\n    int x = 1;\n}\n";

TestPick pick_two_runs(const std::string& name) {
    PickRepo fx(name, {{"src/a/one.h", "#pragma once\nvoid run();\n"},
                       {"src/a/one.cpp", kOneCpp},
                       {"src/b/two.h", "#pragma once\nvoid run();\n"},
                       {"tests/fix.h", "#pragma once\n#include \"a/one.h\"\n"},
                       {"tests/test_t_a.cpp", "#include \"a/one.h\"\nvoid ta() { run(); }\n"},
                       {"tests/test_t_b.cpp", "#include \"b/two.h\"\nvoid tb() { run(); }\n"},
                       {"tests/test_t_c.cpp", "#include \"fix.h\"\nvoid tc() { run(); }\n"}});
    std::string tip = kOneCpp;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 3;");
    return fx.pick({{"src/a/one.cpp", tip}});
}

}  // namespace

TEST_CASE("precheck pick (j): a test naming an unrelated function of the same name is not picked") {
    const TestPick p = pick_two_runs("precheck_pick_j1");
    REQUIRE_FALSE(p.whole_suite);
    const std::vector<std::string> files = picked_files(p);
    CHECK(std::find(files.begin(), files.end(), "tests/test_t_a.cpp") != files.end());
    CHECK(std::find(files.begin(), files.end(), "tests/test_t_b.cpp") == files.end());
    CHECK(reasons_of(p, "tests/test_t_a.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (j): a test reaching the declaring header through a test helper header is picked") {
    const TestPick p = pick_two_runs("precheck_pick_j2");
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_t_a.cpp", "tests/test_t_c.cpp"});
    CHECK(reasons_of(p, "tests/test_t_c.cpp") == std::vector<std::string>{"run"});
}

// A name in a main file picks the tests that run its exe (the (k) cases
// below). This fixture has no CMakeLists.txt, so no exe target lists the
// file and the name keeps the plain-name search.
TEST_CASE("precheck pick (j): a name in a main file no exe target lists is found by its plain name") {
    const std::string main_cpp = "#include \"lib.h\"\nint step() {\n    return 1;\n}\nint main() { return step(); }\n";
    PickRepo fx("precheck_pick_j3", {{"tools/x/lib.h", "#pragma once\nint step();\n"},
                                     {"tools/x/main.cpp", main_cpp},
                                     {"tests/test_step.cpp", "int ts() { return step(); }\n"}});
    std::string tip = main_cpp;
    tip.replace(tip.find("return 1;"), 9, "return 2;");
    const TestPick p = fx.pick({{"tools/x/main.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_step.cpp"});
    CHECK(reasons_of(p, "tests/test_step.cpp") == std::vector<std::string>{"step"});
}

TEST_CASE("precheck pick (j): a deleted class still picks a test that names it, with no include") {
    const std::string base = "#pragma once\nclass Ghost {\npublic:\n    int g = 0;\n};\nint stay();\n";
    PickRepo fx("precheck_pick_j4", {{"src/g/ghost.h", base},
                                     {"tests/test_ghost.cpp", "int tg() { Ghost x; return x.g; }\n"},
                                     {"tests/test_none.cpp", "#include \"g/ghost.h\"\nint tn() { return 0; }\n"}});
    const TestPick p = fx.pick({{"src/g/ghost.h", "#pragma once\nint stay();\n"}});
    REQUIRE_FALSE(p.whole_suite);
    const PickName* g = name_in(p, "Ghost");
    REQUIRE(g != nullptr);
    CHECK(g->base_only);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_ghost.cpp"});
    CHECK(reasons_of(p, "tests/test_ghost.cpp") == std::vector<std::string>{"Ghost"});
}

TEST_CASE("precheck pick (j): a selector value picks only tests that reach its enum's header") {
    const std::string place_cpp =
        "#include \"rule.h\"\nvoid place() {\n    int k = 1;\n}\nvoid check(Rule r) {\n"
        "    if (r == Rule::New) {\n        place();\n    }\n}\n";
    PickRepo fx("precheck_pick_j5",
                {{"src/r/rule.h", "#pragma once\nenum class Rule { Old, New };\nvoid place();\nvoid check(Rule r);\n"},
                 {"src/r/place.cpp", place_cpp},
                 {"tests/test_rule_in.cpp", "#include \"r/rule.h\"\nRule ri() { return Rule::New; }\n"},
                 {"tests/test_rule_out.cpp", "enum class Rule { Old, New };\nRule ro() { return Rule::New; }\n"}});
    std::string tip = place_cpp;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/r/place.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    bool selector = false;
    for (const CallerKey& k : p.callers) selector = selector || (k.key == "Rule::New" && k.selector);
    CHECK(selector);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_rule_in.cpp"});
    CHECK(reasons_of(p, "tests/test_rule_in.cpp") == std::vector<std::string>{"Rule::New"});
}

TEST_CASE("precheck pick (j): a function with no declaring header is found by its plain name") {
    const std::string lone = "void lone() {\n    int x = 1;\n}\n";
    PickRepo fx("precheck_pick_j6", {{"src/n/lone.cpp", lone},
                                     {"tests/test_lone.cpp", "void tl() { lone(); }\n"}});
    std::string tip = lone;
    tip.replace(tip.find("int x = 1;"), 10, "int x = 2;");
    const TestPick p = fx.pick({{"src/n/lone.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_lone.cpp"});
    CHECK(reasons_of(p, "tests/test_lone.cpp") == std::vector<std::string>{"lone"});
}

TEST_CASE("precheck pick (j): a selector value whose left part is a namespace is found by its plain name") {
    const std::string place_cpp =
        "#include \"rule.h\"\nvoid place() {\n    int k = 1;\n}\nvoid check(int r) {\n"
        "    if (r == util::kFast) {\n        place();\n    }\n}\n";
    PickRepo fx("precheck_pick_j7",
                {{"src/r/rule.h", "#pragma once\nnamespace util { constexpr int kFast = 1; }\nvoid place();\n"},
                 {"src/r/place.cpp", place_cpp},
                 {"tests/test_util.cpp", "int u() { return util::kFast; }\n"}});
    std::string tip = place_cpp;
    tip.replace(tip.find("int k = 1;"), 10, "int k = 2;");
    const TestPick p = fx.pick({{"src/r/place.cpp", tip}});
    REQUIRE_FALSE(p.whole_suite);
    bool selector = false;
    for (const CallerKey& k : p.callers) selector = selector || (k.key == "util::kFast" && k.selector);
    CHECK(selector);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_util.cpp"});
}

// A name in a file that builds an exe's main picks the tests that name the
// exe's constant (D299, last paragraph).
namespace {

const std::string kToolMain = "int run() {\n    return 1;\n}\nint main() { return run(); }\n";
const std::string kToolTarget = "add_executable(hydra_tool src/tool/main.cpp)\n";
const std::string kToolConstant = "target_compile_definitions(hydra_tests PRIVATE HYDRA_TOOL_EXE=\"tool.exe\")\n";

// Commits `base` (with src/tool/main.cpp) and picks for a change to run.
TestPick pick_tool(const std::string& name, std::map<std::string, std::string> base) {
    base["src/tool/main.cpp"] = kToolMain;
    base["tests/test_b.cpp"] = "int tb() { return run(); }\n";
    PickRepo fx(name, base);
    std::string tip = kToolMain;
    tip.replace(tip.find("return 1;"), 9, "return 2;");
    return fx.pick({{"src/tool/main.cpp", tip}});
}

// Checks that `file` was picked by the exe rule for the changed name run: its
// reasons hold no plain "run" (it never names run), and one reason starts
// with run and names the target kToolTarget declares. The picker's wording
// of that reason has no public owner, so only its two parts are checked.
void check_exe_reason(const TestPick& p, const std::string& file) {
    const std::string open = "add_executable(";
    REQUIRE(hydra::starts_with(kToolTarget, open));
    const std::string target = first_word(kToolTarget.substr(open.size()));
    REQUIRE_FALSE(target.empty());
    const std::vector<std::string> r = reasons_of(p, file);
    CHECK(std::find(r.begin(), r.end(), "run") == r.end());
    CHECK(std::any_of(r.begin(), r.end(), [&](const std::string& s) {
        return hydra::starts_with(s, "run ") && s.find(target) != std::string::npos;
    }));
}

}  // namespace

TEST_CASE("precheck pick (k): a name in an exe's main picks only the tests naming the exe's constant") {
    const TestPick p = pick_tool("precheck_pick_k1",
                                 {{"CMakeLists.txt", kToolTarget + kToolConstant},
                                  {"tests/test_a.cpp", "const char* ta = HYDRA_TOOL_EXE;\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_a.cpp"});
    check_exe_reason(p, "tests/test_a.cpp");
}

TEST_CASE("precheck pick (k): a test including a helper header naming the exe's constant is picked") {
    const TestPick p = pick_tool("precheck_pick_k2",
                                 {{"CMakeLists.txt", kToolTarget + kToolConstant},
                                  {"tests/tool_fixture.h", "#pragma once\nconst char* tf = HYDRA_TOOL_EXE;\n"},
                                  {"tests/test_c.cpp", "#include \"tool_fixture.h\"\nint tc() { return 0; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_c.cpp"});
    check_exe_reason(p, "tests/test_c.cpp");
}

TEST_CASE("precheck pick (k): an exe with no constant in CMakeLists.txt keeps the plain-name search") {
    const TestPick p = pick_tool("precheck_pick_k3", {{"CMakeLists.txt", kToolTarget},
                                                      {"tests/test_a.cpp", "const char* ta = HYDRA_TOOL_EXE;\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_b.cpp"});
    CHECK(reasons_of(p, "tests/test_b.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (k): a main file two exe targets list keeps the plain-name search") {
    const TestPick p = pick_tool("precheck_pick_k5",
                                 {{"CMakeLists.txt", kToolTarget + "add_executable(hydra_other src/tool/main.cpp)\n" + kToolConstant},
                                  {"tests/test_a.cpp", "const char* ta = HYDRA_TOOL_EXE;\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_b.cpp"});
    CHECK(reasons_of(p, "tests/test_b.cpp") == std::vector<std::string>{"run"});
}

TEST_CASE("precheck pick (k): an exe constant no test names keeps the plain-name search") {
    const TestPick p = pick_tool("precheck_pick_k4", {{"CMakeLists.txt", kToolTarget + kToolConstant},
                                                      {"tests/test_a.cpp", "int ta() { return 0; }\n"}});
    REQUIRE_FALSE(p.whole_suite);
    CHECK(picked_files(p) == std::vector<std::string>{"tests/test_b.cpp"});
    CHECK(reasons_of(p, "tests/test_b.cpp") == std::vector<std::string>{"run"});
}
