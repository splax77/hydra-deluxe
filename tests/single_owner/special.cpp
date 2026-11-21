// The single-owner case that a row cannot express: a rule written as a
// comment (D23). The rows themselves live in tests/single_owner/rules/.
#include "doctest.h"

#include <filesystem>
#include <string>

#include "../source_tree.h"
#include "core/winstr.h"
#include "rows.h"

using namespace single_owner;

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, which does not compile and which
// the row scan skips, so this case reads it: the block between the "Results"
// banner and the kResultsStamp line in src/store/stored_versions.h must name
// src/parse next to src/search and src/core. The repo root comes from
// tests/source_tree.h.
TEST_CASE("single-owner: the results stamp's bump rule names the chart readers (D23)") {
    const std::string text =
        hydra::read_file_text((sourcetree::root() / "src" / "store" / "stored_versions.h").u8string());
    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}
