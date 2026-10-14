// Practice-section names as Clone Hero shows them (D115 B2-3). The list is
// Clone Hero 1.1.0.6142's, kept in data/ and turned into
// src/parse/section_names_table.cpp by tools/gen_section_names.py.
#include "doctest.h"

#include <fstream>
#include <map>
#include <string>

#include "parse/section_names.h"
#include "source_tree.h"

using namespace hydra;

TEST_CASE("section names: the generated table agrees with the TSV row for row") {
    const auto tsv = sourcetree::root() / "data" / "clone-hero-1.1.0.6142-section-names.tsv";
    std::ifstream in(tsv, std::ios::binary);
    REQUIRE(in.good());
    std::map<std::string, std::string> rows;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.front() == '#') continue;
        const size_t tab = line.find('\t');
        REQUIRE(tab != std::string::npos);
        INFO(line);
        CHECK(rows.emplace(line.substr(0, tab), line.substr(tab + 1)).second);
    }
    // The count the first executor and this task read from the source file.
    CHECK(rows.size() == 2718);

    const SectionNameTable table = section_name_table();
    REQUIRE(table.count == rows.size());
    size_t i = 0;
    for (const auto& [key, shown] : rows) {
        INFO("row " << i << ": " << key);
        CHECK(table.rows[i].key == key);
        CHECK(table.rows[i].shown == shown);
        ++i;
    }
}

TEST_CASE("section names: a listed marker shows Clone Hero's words") {
    // The TSV's first two rows, and one that is not a plain capitalisation.
    CHECK(section_display_name("prc_a") == "A section");
    CHECK(section_display_name("section a1") == "A section 1");
    CHECK(section_display_name("prc_aco_gtr_intro") == "Acoustic Guitar Intro");
}

TEST_CASE("section names: an unlisted marker shows its name with spaces for underscores") {
    CHECK(section_display_name("prc_my_own_part") == "my own part");
    // The lookup is exact, case included: "Intro" is not the listed "intro".
    CHECK(section_display_name("section Intro") == "Intro");
    CHECK(section_display_name("section Verse 2B") == "Verse 2B");
    CHECK(section_display_name("section ") == "");
}

TEST_CASE("section names: only the two prefixes mark a section") {
    CHECK(is_section_marker("section Intro"));
    CHECK(is_section_marker("prc_verse_1"));
    CHECK(is_section_marker("section "));
    CHECK(is_section_marker("prc_"));
    CHECK_FALSE(is_section_marker("Section Intro"));
    CHECK_FALSE(is_section_marker("sectionIntro"));
    CHECK_FALSE(is_section_marker("lighting (chase)"));
    CHECK_FALSE(is_section_marker(""));
}
