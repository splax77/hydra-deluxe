// hydra_precheck's check 1 (tools/precheck/check_1.cpp), test helpers
// defined twice, and the pieces it is built from: the views (views.h), the
// brace matcher (braces.h) and the definition finder and normaliser
// (definitions.h). The check runs as the exe on a tiny git repo in a temp
// folder, with the other checks turned off.
#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/braces.h"
#include "precheck/check_1_text.h"
#include "precheck/checks.h"
#include "precheck/definitions.h"
#include "precheck/git_text.h"
#include "precheck/items.h"
#include "precheck/views.h"
#include "precheck_fixture.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;

using namespace precheck_fixture;

namespace {

// The body the precheck self-test's fixture (tests/test_precheck.cpp) gives sum_values,
// written with the names `total`, `values` and `v` swapped for others.
std::string summing(const std::string& fn, const std::string& list, const std::string& acc,
                    const std::string& item) {
    return "int " + fn + "(const std::vector<int>& " + list + ") {\n"
           "    int " + acc + " = 0;\n"
           "    for (int " + item + " : " + list + ") {\n"
           "        if (" + item + " > 0) " + acc + " += " + item + ";\n"
           "        else " + acc + " -= " + item + ";\n"
           "    }\n"
           "    return " + acc + ";\n"
           "}\n";
}

// A body over the floor that no other fixture file has.
std::string factorial(const std::string& fn, const std::string& n, const std::string& p) {
    return "int " + fn + "(int " + n + ") {\n"
           "    int " + p + " = 1;\n"
           "    for (int i = 1; i <= " + n + "; ++i) {\n"
           "        " + p + " *= i;\n"
           "    }\n"
           "    return " + p + ";\n"
           "}\n";
}

// A comment line between a namespace's name and its "{", as a long note on
// what the namespace holds would sit. The line alone is longer than the
// finder's look-back, so the window before the "{" holds no part of the head.
std::string comment_past_look_back() { return "// " + std::string(kNamespaceLookBack, 'x') + "\n"; }

// main holds sum_values and a short helper; feature adds the copies.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_check1");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        write_fixture_file(dir, "tests/helpers_a.cpp",
                   "#include <vector>\n\nnamespace {\n\n// Adds the positive values and subtracts the negative ones.\n" +
                       summing("sum_values", "values", "total", "v") + "\n}  // namespace\n");
        write_fixture_file(dir, "tests/short_a.cpp", "int tiny_a(int q) { return q + 1; }\n");
        commit_all(dir, "main");
        fixture_git(dir, {"checkout", "-q", "-b", "feature"});
        // The same helper with its names changed: printed.
        write_fixture_file(dir, "tests/helpers_b.cpp",
                   "#include <vector>\n\nnamespace {\n\n" + summing("add_up", "xs", "acc", "x") +
                       "\n}  // namespace\n");
        // A member function with that body: not a helper.
        write_fixture_file(dir, "tests/members.cpp",
                   "#include <vector>\n\nstruct Totals {\n" + summing("total_of", "ys", "t", "y") + "};\n");
        // A namespace whose head is longer than the look-back: not seen as a
        // namespace, so its function is not a helper.
        write_fixture_file(dir, "tests/far_ns.cpp",
                   "#include <vector>\n\nnamespace far\n" + comment_past_look_back() + "{\n" +
                       summing("far_sum", "zs", "s", "z") + "}\n");
        // Two copies inside one file: not two files.
        write_fixture_file(dir, "tests/one_file.cpp", factorial("first_copy", "n", "p") + factorial("second_copy", "m", "q"));
        // A short body under another name matches by name only: not printed.
        write_fixture_file(dir, "tests/short_b.cpp", "int tiny_b(int r) { return r + 1; }\n");
        // A short helper under the same name: printed, bodies differ.
        write_fixture_file(dir, "tests/short_c.cpp", "int tiny_a(int z) { return z * 2; }\n");
        // Test cases with one body in two files: not helpers.
        write_fixture_file(dir, "tests/test_one.cpp", "TEST_CASE(\"one\") {\n    int k = 0;\n    CHECK(k == 0);\n}\n");
        write_fixture_file(dir, "tests/test_two.cpp", "TEST_CASE(\"two\") {\n    int k = 0;\n    CHECK(k == 0);\n}\n");
        // A MIDI varlen delta typed by hand: printed outside its owner.
        write_fixture_file(dir, "tests/varlen.cpp", "void put(int d) {\n    out.push_back(0x80 | (d >> 7));\n}\n");
        write_fixture_file(dir, kVarlenOwner, "inline void put_varlen(int d) {\n    out.push_back(0x80 | (d >> 7));\n}\n");
        commit_all(dir, "feature");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};


int count_matching(const std::vector<std::string>& lines, const std::string& pattern) {
    const std::regex rx(pattern);
    int n = 0;
    for (const std::string& l : lines)
        if (std::regex_search(l, rx)) ++n;
    return n;
}

// The what and why of each printed item of `kind` at file:line. The line is
// split by the form item_line() prints, read from item_line() itself.
struct WhatWhy {
    std::string what;
    std::string why;
};
std::vector<WhatWhy> items_at(const std::vector<std::string>& lines, const std::string& kind,
                              const std::string& file, int line) {
    Item marks;
    marks.kind = kind;
    marks.file = file;
    marks.line = line;
    marks.what = "\x01";
    marks.why = "\x02";
    const std::string form = item_line(marks);
    const size_t what_at = form.find('\x01');
    const size_t why_at = form.find('\x02');
    REQUIRE(what_at != std::string::npos);
    REQUIRE(why_at != std::string::npos);
    const std::string head = form.substr(0, what_at);
    const std::string between = form.substr(what_at + 1, why_at - what_at - 1);
    std::vector<WhatWhy> out;
    for (const std::string& l : lines) {
        if (l.rfind(head, 0) != 0) continue;
        const size_t sep = l.find(between, head.size());
        if (sep == std::string::npos) continue;
        WhatWhy item;
        item.what = l.substr(head.size(), sep - head.size());
        item.why = l.substr(sep + between.size());
        out.push_back(item);
    }
    return out;
}

std::vector<std::string> names_of(const std::vector<Definition>& defs) {
    std::vector<std::string> out;
    for (const Definition& d : defs) out.push_back(d.name);
    return out;
}

}  // namespace

TEST_CASE("precheck views: comments blanked, string text blanked, line breaks kept") {
    const std::string text = "int a = 1; // note\nconst char* s = \"a{b\";\n/* x\ny */ char c = '}';\n";
    const View v = make_view(text, codelex::Lang::Cpp);
    CHECK(v.raw == text);
    CHECK(v.no_comments == "int a = 1;        \nconst char* s = \"a{b\";\n    \n     char c = '}';\n");
    CHECK(v.code == "int a = 1;        \nconst char* s = \"   \";\n    \n     char c = ' ';\n");
    CHECK(v.line_count() == 5);
    CHECK(v.line_of(0) == 1);
    CHECK(v.line_of(text.find('\n')) == 1);
    CHECK(v.line_of(text.find("const")) == 2);
    CHECK(v.line_of(text.find("char c")) == 4);
    CHECK(v.line(v.raw, 2) == "const char* s = \"a{b\";");

    const View py = make_view("x = '#'  # note\n", view_lang("tools/a.py"));
    CHECK(py.no_comments == "x = '#'        \n");
    CHECK(py.code == "x = ' '        \n");
}

TEST_CASE("precheck braces: each close, parent and children") {
    //                  0123456789
    const Braces b(std::string("a{b{}c{}}{"));
    CHECK(b.close(1, 100) == std::optional<size_t>(8));
    CHECK(b.close(1, 8) == std::nullopt);  // closes at the limit
    CHECK(b.close(3, 100) == std::optional<size_t>(4));
    CHECK(b.close(9, 100) == std::nullopt);  // never closes
    CHECK(b.parent(3) == 1);
    CHECK(b.parent(1) == Braces::kTop);
    CHECK(b.children(1) == std::vector<size_t>{3, 6});
    CHECK(b.children(Braces::kTop) == std::vector<size_t>{1, 9});
}

TEST_CASE("precheck normaliser: names renamed by first use, keywords, members and literals kept") {
    const Normalised a = normalise("int a = b + 1;");
    CHECK(a.key == "int v0 = v1 + 1 ; ");
    CHECK(a.tokens == 7);
    const Normalised b = normalise("x.size() + y->size() + std::max(x, y)");
    CHECK(b.key == "v0 . size ( ) + v1 -> size ( ) + v2 ( v0 , v1 ) ");
    CHECK(b.tokens == 18);
    // The code view leaves a string as its quotes around spaces.
    const Normalised c = normalise("f(\"  \", ' ', 1'000)");
    CHECK(c.key == "v0 ( S , C , 1'000 ) ");
    CHECK(c.tokens == 8);
}

TEST_CASE("precheck definitions: free functions and named lambdas, not keywords or test macros") {
    const std::string text =
        "void TEST_CASE(int a) {\n}\n"
        "int main() {\n}\n"
        "void CHECK_EQ(int a) {\n}\n"
        "else g(int y) {\n}\n"
        "static int keep(int a) {\n    auto twice = [&](int b) { return b * 2; };\n}\n"
        "struct S {\n    int member(int a) {\n    }\n};\n";
    const std::vector<Definition> defs = definitions(make_view(text, codelex::Lang::Cpp), "tests/x.cpp");
    CHECK(names_of(defs) == std::vector<std::string>{"keep", "twice"});
    REQUIRE(defs.size() == 2);
    CHECK(defs[0].line == 9);
    CHECK(defs[0].last_line == 11);
    CHECK_FALSE(defs[0].lambda);
    CHECK(defs[1].lambda);
    CHECK(defs[1].line == 10);
}

TEST_CASE("precheck definitions: a namespace head is found only within the look-back") {
    const std::string body = summing("f", "a", "b", "c");
    // ("near" and "far" are Windows macros.)
    const std::string close_head = "namespace n\n// short\n{\n" + body + "}\n";
    const std::string distant_head = "namespace n\n" + comment_past_look_back() + "{\n" + body + "}\n";
    const std::vector<Definition> found = definitions(make_view(close_head, codelex::Lang::Cpp), "tests/x.cpp");
    CHECK(names_of(found) == std::vector<std::string>{"f"});
    CHECK(definitions(make_view(distant_head, codelex::Lang::Cpp), "tests/x.cpp").empty());
    // The summing body is over the floor, so it can match by body.
    REQUIRE(found.size() == 1);
    CHECK(found[0].tokens >= kBodyTokenFloor);
}

TEST_CASE("precheck check 1: a helper copied under new names prints C, the near misses do not") {
    Fixture fx;
    const std::vector<std::string> lines = exe_item_lines(fx.dir, "2,3,4,5");
    INFO([&] {
        std::string all;
        for (const std::string& l : lines) all += l + "\n";
        return all;
    }());
    const std::vector<WhatWhy> copied = items_at(lines, "C", "tests/helpers_b.cpp", 5);
    const std::vector<WhatWhy> same_name = items_at(lines, "C", "tests/short_c.cpp", 1);
    const std::vector<WhatWhy> varlen = items_at(lines, "C", "tests/varlen.cpp", 2);
    REQUIRE(copied.size() == 1);
    REQUIRE(same_name.size() == 1);
    REQUIRE(varlen.size() == 1);
    // The renamed copy names itself, and the helper it copies with its place.
    CHECK(copied[0].what.find("add_up") != std::string::npos);
    CHECK(copied[0].what.find("sum_values") != std::string::npos);
    CHECK(copied[0].what.find("tests/helpers_a.cpp:6") != std::string::npos);
    // The same name with another body names the first definition's place.
    CHECK(same_name[0].what.find("tiny_a") != std::string::npos);
    CHECK(same_name[0].what.find("tests/short_a.cpp:1") != std::string::npos);
    // Both helper items give check 1's helper reason; no fixture line is a
    // known copy, so no note follows it.
    CHECK(copied[0].why == kHelperWhy);
    CHECK(same_name[0].why == kHelperWhy);
    // The hand-typed delta gives the varlen spelling's description and
    // reason, quotes its line, read from the fixture file, and names the
    // header that owns it.
    CHECK(varlen[0].why == kVarlenWhy);
    CHECK(varlen[0].what.rfind(std::string(kVarlenWhat) + ": ", 0) == 0);
    const View varlen_file = make_view(hydra::read_file_text(fx.dir + "/tests/varlen.cpp"), codelex::Lang::Cpp);
    const std::string varlen_line = varlen_file.line(varlen_file.raw, 2);
    const std::string quoted =
        varlen_line.substr((std::min)(varlen_line.find_first_not_of(' '), varlen_line.size()));
    REQUIRE_FALSE(quoted.empty());
    CHECK(varlen[0].what.find(quoted) != std::string::npos);
    CHECK(varlen[0].why.find(kVarlenOwner) != std::string::npos);
    CHECK(count_matching(lines, "total_of|far_sum|first_copy|second_copy|tiny_b|TEST_CASE|midi_util\\.h:") == 0);
    CHECK(count_matching(lines, "^C tests/helpers_a\\.cpp:") == 0);
    CHECK(lines.size() == 3);
}
