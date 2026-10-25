// hydra_precheck's check 5 (tools/precheck/check_5.cpp), second homes in
// src/, and the type-definition finder and record-field reader it is built
// from (tools/precheck/records.h). The check runs as the exe on a tiny git
// repo in a temp folder, with the other checks turned off; the repo holds the
// check-5 files of the precheck self-test's fixture (tests/test_precheck.cpp).
#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "core/winstr.h"
#include "precheck/braces.h"
#include "precheck/git_text.h"
#include "precheck/records.h"
#include "precheck_fixture.h"
#include "temp_util.h"

#ifndef HYDRA_PRECHECK_EXE
#error "HYDRA_PRECHECK_EXE must be defined (see CMakeLists.txt)"
#endif

namespace fs = std::filesystem;
using namespace precheck;

namespace {

using precheck_fixture::commit_all;
using precheck_fixture::exe_item_lines;
using precheck_fixture::fixture_git;
using precheck_fixture::write_fixture_file;

// main holds the first homes; feature adds one second home per probe and the
// near misses.
struct Fixture {
    std::string dir = testtemp::temp_dir("precheck_check5");
    Fixture() {
        testtemp::remove_dir(dir);
        fs::create_directories(hydra::os_path(dir));
        fixture_git(dir, {"init", "-q", "-b", "main"});
        // Some of SongEntry's fields are declared in one comma list, which
        // must count as separate names.
        write_fixture_file(dir, "src/core/old_types.h",
                   "#pragma once\n#include <string>\n#include <utility>\n\nnamespace fx {\n\n"
                   "struct SongEntry {\n"
                   "    std::string title, artist;\n"
                   "    std::string album;\n"
                   "    std::pair<std::string, std::string> credit;\n"
                   "    bool starred = false;\n"
                   "};\n\n"
                   "struct Letters {\n    int alpha;\n    int beta;\n    int gamma;\n    int delta;\n};\n\n"
                   "enum class Verdict { Pass, Fail };\n\n"
                   "}  // namespace fx\n");
        write_fixture_file(dir, "src/core/old_helpers.cpp",
                   "#include \"core/old_types.h\"\n\nnamespace fx {\nnamespace {\n\n"
                   "std::string trim_label(const std::string& s) { return s; }\n\n"
                   "}  // namespace\n\n"
                   "// Old wording: \"a comment quoting text\".\n"
                   "const char* missing_text() { return \"chart file is missing\"; }\n"
                   "const char* short_text() { return \"too short\"; }\n\n"
                   "}  // namespace fx\n");
        // A pair outside src/: not compared.
        write_fixture_file(dir, "tests/strings_a.cpp", "const char* a() { return \"a sentence in two test files\"; }\n");
        commit_all(dir, "main");
        fixture_git(dir, {"checkout", "-q", "-b", "feature"});
        // LibraryRow shares four names with SongEntry: printed. Greek shares
        // three with Letters: not printed.
        write_fixture_file(dir, "src/app/new_types.h",
                   "#pragma once\n#include <string>\n#include <utility>\n\nnamespace fx {\n\n"
                   "struct LibraryRow {\n"
                   "    std::string title;\n"
                   "    std::string artist, album;\n"
                   "    std::pair<std::string, std::string> credit;\n"
                   "    int plays = 0;\n"
                   "};\n\n"
                   "struct Greek {\n    int alpha;\n    int beta;\n    int gamma;\n    int omega;\n};\n\n"
                   "enum class Verdict { Good, Bad };\n\n"
                   "}  // namespace fx\n");
        // A short string, a string the other file has only in a comment and
        // the #include path both files write: not printed.
        write_fixture_file(dir, "src/app/new_helpers.cpp",
                   "#include \"core/old_types.h\"\n\nnamespace fx {\nnamespace {\n\n"
                   "std::string trim_label(const std::string& s) { return s + \" \"; }\n\n"
                   "}  // namespace\n\n"
                   "const char* load_error() { return \"chart file is missing\"; }\n"
                   "const char* brief_text() { return \"too short\"; }\n"
                   "const char* quoted_text() { return \"a comment quoting text\"; }\n\n"
                   "}  // namespace fx\n");
        write_fixture_file(dir, "tests/strings_b.cpp", "const char* b() { return \"a sentence in two test files\"; }\n");
        commit_all(dir, "feature");
    }
    ~Fixture() { testtemp::remove_dir(dir); }
};

std::vector<std::string> field_names(const std::string& code) {
    const Braces braces(code);
    const std::vector<TypeDefinition> types = type_definitions(code);
    REQUIRE(!types.empty());
    const std::optional<size_t> close = braces.close(types[0].open, code.size());
    REQUIRE(close.has_value());
    std::vector<std::string> out;
    for (const RecordField& f : record_fields(code, braces, types[0].open, *close)) out.push_back(f.name);
    return out;
}

}  // namespace

TEST_CASE("precheck type definitions: kinds, attributes, final and bases; not an out-of-line member type") {
    const std::string code =
        "struct [[nodiscard]] A final : public B<int> {\n};\n"
        "enum class E : int { X };\n"
        "struct Engine::Impl {\n};\n"
        "union U { int a; };\n"
        "class Fwd;\n"
        "struct Outer {\n    struct Inner {\n    };\n};\n";
    std::vector<std::string> got;
    for (const TypeDefinition& t : type_definitions(code)) got.push_back(t.kind + " " + t.name);
    CHECK(got == std::vector<std::string>{"struct A", "enum class E", "union U", "struct Outer", "struct Inner"});
    const std::vector<TypeDefinition> types = type_definitions(code);
    REQUIRE(!types.empty());
    CHECK(types[0].name_at == code.find("A final"));
    CHECK(types[0].open == code.find('{'));
}

TEST_CASE("precheck record fields: comma lists, initialisers and arrays; not methods, labels or nested members") {
    CHECK(field_names("struct S {\n"
                      "public:\n"
                      "    int64_t base, combo, sp;\n"
                      "    std::map<int, int> m{};\n"
                      "    int arr[4] = {1, 2, 3, 4};\n"
                      "    int Base;\n"
                      "    int size() const { return 1; }\n"
                      "    static int count;\n"
                      "    using T = int;\n"
                      "    struct Inner { int hidden; };\n"
                      "  private:\n"
                      "    bool ok = true;\n"
                      "};\n") == std::vector<std::string>{"base", "combo", "sp", "m", "arr", "ok"});
    // A name with no type before it declares nothing.
    CHECK(field_names("struct S { x; int y; };") == std::vector<std::string>{"y"});
}

TEST_CASE("precheck check 5: the four second homes print F with their wording, the near misses do not") {
    Fixture fx;
    const std::vector<std::string> lines = exe_item_lines(fx.dir, "1,2,3,4");
    INFO([&] {
        std::string all;
        for (const std::string& l : lines) all += l + "\n";
        return all;
    }());
    const std::string why =
        "  -- a second home for an idea that already has one; name the concept it extends, or reuse it "
        "(derive-once review, step 8)";
    CHECK(lines == std::vector<std::string>{
                       "F src/app/new_helpers.cpp:6  free function trim_label is also defined at "
                       "src/core/old_helpers.cpp:6" + why,
                       "F src/app/new_helpers.cpp:10  \"chart file is missing\" is also written at "
                       "src/core/old_helpers.cpp:11" + why,
                       "F src/app/new_types.h:7  struct LibraryRow shares 4 field names with struct SongEntry at "
                       "src/core/old_types.h:7: album, artist, credit, title" + why,
                       "F src/app/new_types.h:21  enum class Verdict is also defined at src/core/old_types.h:21" + why,
                   });
}
