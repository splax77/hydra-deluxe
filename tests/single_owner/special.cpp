// The single-owner cases that a row cannot express: a rule written as a
// comment (D23), and a sentence the User Guide must quote as the code has it
// (D94). The rows themselves live in tests/single_owner/rules/.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "../source_tree.h"
#include "rows.h"

namespace fs = std::filesystem;
using namespace single_owner;

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, which does not compile and which
// the row scan skips, so this case reads it: the block between the "Results"
// banner and the kResultsStamp line in src/store/stored_versions.h must name
// src/parse next to src/search and src/core. Moved here from
// a stamps test; the repo root comes from tests/source_tree.h.
TEST_CASE("single-owner: the results stamp's bump rule names the chart readers (D23)") {
    std::ifstream in(sourcetree::root() / "src" / "store" / "stored_versions.h");
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();
    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}

// D94: the User Guide quotes the Preview's changed-chart line in backticks.
// The sentence is read off the owner line of the row that guards it, and that
// line is checked against src/ui/preview_tab.cpp, so the words are typed only
// in the code and in that row.
TEST_CASE("single-owner: the User Guide quotes the Preview's changed-chart line as the code has it") {
    const OwnerLine* owner = nullptr;
    for (const OwnerRule& r : rules())
        if (r.question == "What does the Preview say when the chart changed since it was analyzed?")
            for (const OwnerLine& o : r.owner_lines)
                if (o.file == "src/ui/preview_tab.cpp") owner = &o;
    REQUIRE(owner != nullptr);

    const auto slurp = [](const fs::path& p) {
        std::ifstream in(p);
        REQUIRE(in.good());
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    };
    INFO("owner line: " << owner->line_text);
    REQUIRE(slurp(sourcetree::root() / fs::u8path(owner->file)).find(owner->line_text) !=
            std::string::npos);

    // The C++ literal on that line, between its first and last double quote.
    const size_t open = owner->line_text.find('"');
    const size_t close = owner->line_text.rfind('"');
    REQUIRE(open != std::string::npos);
    REQUIRE(close > open);
    const std::string sentence = owner->line_text.substr(open + 1, close - open - 1);
    REQUIRE(sentence.find('\\') == std::string::npos);  // no escapes to undo

    INFO("docs/UserGuide.md should quote `" << sentence << "`");
    CHECK(slurp(sourcetree::root() / "docs" / "UserGuide.md").find("`" + sentence + "`") !=
          std::string::npos);
}
