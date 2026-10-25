// The test picker's changed-name finder (tools/precheck/changed_names.h):
// each case is an inline C++ text and the lines a diff would change in it.

#include "doctest.h"

#include <set>
#include <string>

#include "precheck/braces.h"
#include "precheck/changed_names.h"
#include "precheck/views.h"

namespace {

// The names, each as "name:kind", space-separated, in the finder's order.
std::string names_of(const std::string& text, const std::set<int>& lines) {
    const precheck::View view = precheck::make_view(text, codelex::Lang::Cpp);
    std::string out;
    for (const precheck::ChangedName& n : precheck::changed_names(view, precheck::Braces(view.code), lines)) {
        if (!out.empty()) out += ' ';
        out += n.name + ":" + precheck::kind_word(n.kind);
    }
    return out;
}

}  // namespace

TEST_CASE("pick names: a line in a member function body gives the member without its Class::") {
    const std::string text =
        "namespace hydra {\n"            // 1
        "int Song::note_count() const {\n"  // 2
        "    return 3;\n"                // 3
        "}\n"                            // 4
        "}  // namespace hydra\n";       // 5
    CHECK(names_of(text, {3}) == "note_count:body");
}

TEST_CASE("pick names: a line in a local struct gives the struct and the function") {
    const std::string text =
        "void run(int a) {\n"      // 1
        "    struct Local {\n"     // 2
        "        int y = 0;\n"     // 3
        "    };\n"                 // 4
        "    Local l;\n"           // 5
        "}\n";                     // 6
    CHECK(names_of(text, {3}) == "Local:body run:body");
}

TEST_CASE("pick names: the CH10 shape, a changed line inside Cell in place_fills_ch10") {
    const std::string text =
        "namespace hydra {\n"                                                 // 1
        "\n"                                                                  // 2
        "void Song::place_fills_ch10(const std::vector<Note>& notes, int n) {\n"  // 3
        "    struct Cell {\n"                                                 // 4
        "        int x;\n"                                                    // 5
        "    };\n"                                                            // 6
        "    std::vector<Cell> cells(n);\n"                                   // 7
        "    for (const Note& note : notes) {\n"                              // 8
        "        cells[0].x += note.tick;\n"                                  // 9
        "    }\n"                                                             // 10
        "}\n"                                                                 // 11
        "\n"                                                                  // 12
        "}  // namespace hydra\n";                                            // 13
    CHECK(names_of(text, {5}) == "Cell:body place_fills_ch10:body");
    // A line in a plain block inside the function gives only the function.
    CHECK(names_of(text, {9}) == "place_fills_ch10:body");
}

TEST_CASE("pick names: a changed comment line above a function gives the function") {
    const std::string text =
        "namespace {\n"                        // 1
        "\n"                                   // 2
        "// Checks every activation once.\n"   // 3
        "// The second comment line.\n"        // 4
        "void check_activations(Song& s) {\n"  // 5
        "    s.go();\n"                        // 6
        "}\n"                                  // 7
        "\n"                                   // 8
        "}  // namespace\n";                   // 9
    CHECK(names_of(text, {3}) == "check_activations:comment");
}

TEST_CASE("pick names: a constexpr line gives the constant") {
    const std::string text =
        "namespace hydra {\n"                             // 1
        "constexpr int kResultsStamp = 7;\n"              // 2
        "static const char* const kName = \"x\";\n"       // 3
        "inline constexpr std::array<int, 2> kPair{1, 2};\n"  // 4
        "}\n";                                            // 5
    CHECK(names_of(text, {2}) == "kResultsStamp:value");
    CHECK(names_of(text, {3}) == "kName:value");
    CHECK(names_of(text, {4}) == "kPair:value");
}

TEST_CASE("pick names: a line inside an enum block gives Enum::Value") {
    const std::string text =
        "namespace hydra {\n"                           // 1
        "enum class FillDeadlineRule : uint8_t {\n"     // 2
        "    Legacy,\n"                                 // 3
        "    Ch10 = 2,  // the CH 1.0 rule\n"           // 4
        "};\n"                                          // 5
        "}\n";                                          // 6
    CHECK(names_of(text, {4}) == "FillDeadlineRule::Ch10:value FillDeadlineRule:body");
}

TEST_CASE("pick names: a class block line gives the class, and a declared function too") {
    const std::string text =
        "namespace hydra {\n"                   // 1
        "template <typename T>\n"               // 2
        "class Ch10Measures final : public Base<T> {\n"  // 3
        "public:\n"                             // 4
        "    int measure_of(int tick) const;\n" // 5
        "    int count_ = 0;\n"                 // 6
        "};\n"                                  // 7
        "}\n";                                  // 8
    CHECK(names_of(text, {5}) == "measure_of:declared Ch10Measures:body");
    CHECK(names_of(text, {6}) == "Ch10Measures:body");
    // The head line declares the class itself.
    CHECK(names_of(text, {3}) == "Ch10Measures:declared");
}

TEST_CASE("pick names: a line in a namespace with no enclosing head gives nothing") {
    const std::string text =
        "namespace hydra {\n"      // 1
        "namespace {\n"            // 2
        "\n"                       // 3
        "using Ticks = int;\n"     // 4
        "\n"                       // 5
        "}  // namespace\n"        // 6
        "}  // namespace hydra\n"; // 7
    CHECK(names_of(text, {4}) == "");
}

TEST_CASE("pick names: a name inside a comment or a string never appears") {
    const std::string text =
        "void play(Song& s) {\n"                                  // 1
        "    s.log(\"calls hidden_name() and kHidden = 1\");\n"   // 2
        "    // struct Ghost { int z; }; constexpr int kGhost = 2;\n"  // 3
        "}\n";                                                    // 4
    CHECK(names_of(text, {2, 3}) == "play:body");
}

TEST_CASE("pick names: control blocks, lambdas and initialiser lists name nothing of their own") {
    const std::string text =
        "Player::Player(int a)\n"                       // 1
        "    : a_(a), b_{2} {\n"                        // 2
        "    std::sort(v.begin(), v.end(), [](int x, int y) {\n"  // 3
        "        return x < y;\n"                       // 4
        "    });\n"                                     // 5
        "    if (a > 0) {\n"                            // 6
        "        local_ = 1;\n"                         // 7
        "    }\n"                                       // 8
        "}\n";                                          // 9
    CHECK(names_of(text, {4}) == "Player:body");
    CHECK(names_of(text, {7}) == "Player:body");
}

TEST_CASE("pick names: a name reached two ways is listed once, with the stronger kind") {
    const std::string text =
        "// Runs it.\n"      // 1
        "void run() {\n"     // 2
        "    go();\n"        // 3
        "}\n";               // 4
    CHECK(names_of(text, {1, 3}) == "run:body");
}

TEST_CASE("pick names: a one-line enum gives its values, and a changed value line gives Enum::Value") {
    const std::string text =
        "namespace hydra {\n"                   // 1
        "enum class M { A, B = 2 };\n"          // 2
        "enum Plain { X, Y };\n"                // 3
        "}\n";                                  // 4
    CHECK(names_of(text, {2}) == "M:declared M::A:value M::B:value");
    CHECK(names_of(text, {3}) == "Plain:declared Plain::X:value Plain::Y:value");
}

TEST_CASE("pick names: a data member with parentheses is not a function") {
    const std::string text =
        "class Panel {\n"                              // 1
        "    std::function<void(int)> on_click_;\n"    // 2
        "    std::function<Song(Chart)> make_;\n"      // 3
        "    int (*fp_)(int) = nullptr;\n"             // 4
        "    int table_[kSize(3)];\n"                  // 5
        "    Widget w_{make(1)};\n"                    // 6
        "    int area_ = compute(2);\n"                // 7
        "};\n";                                        // 8
    for (int n = 2; n <= 7; ++n) CHECK(names_of(text, {n}) == "Panel:body");
}

TEST_CASE("pick names: the plan's worked example shapes in one file") {
    const std::string text =
        "namespace hydra {\n"                                        // 1
        "namespace {\n"                                              // 2
        "// Ten measures of fill length.\n"                          // 3
        "constexpr int kCh10FillLengthMeasures = 2;\n"               // 4
        "}  // namespace\n"                                          // 5
        "constexpr int kResultsStamp = 9;\n"                         // 6
        "// Checks the activations.\n"                               // 7
        "bool Song::check_activations() const {\n"                   // 8
        "    return true;\n"                                         // 9
        "}\n"                                                        // 10
        "}  // namespace hydra\n";                                   // 11
    CHECK(names_of(text, {3}) == "kCh10FillLengthMeasures:comment");
    CHECK(names_of(text, {6}) == "kResultsStamp:value");
    CHECK(names_of(text, {7}) == "check_activations:comment");
}
