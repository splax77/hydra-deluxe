// The single-owner cases that a row cannot express: a rule written as a
// comment (D23), and a sentence the User Guide must quote as the code has it
// (D94). The rows themselves live in tests/single_owner/rules/.
#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "../source_tree.h"
#include "core/strutil.h"
#include "listed.h"
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

// E3 (findings 180, 243, 245 and 56): "does this path need any timing?" is
// Path::needs_timing's, so no code line under src/ or tools/ asks it with a
// zero test of its own. The all-0 list's 0 ms limit (D85) is set in one
// place, allzero_options, and the shortcut that skips the all-0 pass reads it
// from there. What the Score range's INI int means is
// Settings::search_depth_mode's, the one line in src/app/config.cpp that
// compares it, however the comparison is spelled. Comment lines are skipped
// like the row scan does.
TEST_CASE("single-owner: the all-0 limit and the depth-mode int each have one owner (E3, D85)") {
    const std::regex zero_limit(
        R"(difficulty\(\)\.value_or\(0(\.0+)?\)\s*(<=|>|<|>=)\s*0(\.0+)?(?![\d.]))"
        R"(|value_or\(0(\.0+)?\)\s*<=\s*0(\.0+)?(?![\d.]))"
        R"(|ms_filter\s*=\s*(std::optional<double>\()?0(\.0+)?(?![\d.]))"
        R"(|within_(ms_)?limit\(\s*0(\.0+)?\s*\))");
    const std::regex depth_int(R"(\bdepth_mode\s*(==|!=|>=|<=|>|<)\s*[01]\b|case\s+1\s*:.*depth)");
    std::vector<std::string> zero_hits, depth_hits;
    sourcetree::for_each_source_file([&](const fs::path& file, const std::string& rel) {
        if (rel.compare(0, 6, "tests/") == 0) return;
        if (!is_cpp_source(file)) return;
        std::ifstream in(file);
        std::string line;
        while (std::getline(in, line)) {
            const std::string t = hydra::trim(line);
            if (is_line_comment(t)) continue;
            if (std::regex_search(t, zero_limit)) zero_hits.push_back(rel + ": " + t);
            if (std::regex_search(t, depth_int)) depth_hits.push_back(rel + ": " + t);
        }
    });
    REQUIRE_MESSAGE(zero_hits.size() == 1,
                    (zero_hits.size() < 2 ? std::string() : zero_hits[1]));
    CHECK(zero_hits.front() == "src/search/pather.cpp: options.ms_filter = 0.0;");
    REQUIRE(depth_hits.size() == 1);
    CHECK(depth_hits.front() ==
          "src/app/config.cpp: return depth_mode == 1 ? DepthMode::Points : DepthMode::Scores;");
}
