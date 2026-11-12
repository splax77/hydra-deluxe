// Practice-section names as Clone Hero shows them (D115). The list is
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
    // The count read from the source file.
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
    // The lookup is exact, case included: the listed "aco_gtr_intro" shows
    // "Acoustic Guitar Intro", but the same letters in capitals are a miss.
    CHECK(section_display_name("prc_ACO_GTR_INTRO") == "ACO GTR INTRO");
    CHECK(section_display_name("section Intro") == "Intro");
    CHECK(section_display_name("section Verse 2B") == "Verse 2B");
    CHECK(section_display_name("section ") == "");
}

TEST_CASE("section names: only the two prefixes mark a section") {
    for (const SectionReader reader : {SectionReader::Mid, SectionReader::Chart}) {
        INFO("reader " << (reader == SectionReader::Mid ? ".mid" : ".chart"));
        CHECK(is_section_marker("section Intro", reader));
        CHECK(is_section_marker("prc_verse_1", reader));
        CHECK(is_section_marker("section ", reader));
        CHECK(is_section_marker("prc_", reader));
        CHECK_FALSE(is_section_marker("Section Intro", reader));
        CHECK_FALSE(is_section_marker("sectionIntro", reader));
        CHECK_FALSE(is_section_marker("lighting (chase)", reader));
        CHECK_FALSE(is_section_marker("", reader));
    }
}

// Block CR-24 on docs/clone-hero/01-reading-a-chart.md: the game's .mid reader
// drops a section text holding _ENDOFSONG, case counting, on its "[section"
// path only. Its .chart reader has no such test.
TEST_CASE("section names: the .mid reader drops a section body holding _ENDOFSONG") {
    CHECK_FALSE(is_section_marker("section _ENDOFSONG", SectionReader::Mid));
    CHECK_FALSE(is_section_marker("section Outro_ENDOFSONG_x", SectionReader::Mid));
    // Case counts: the lower-case spelling is an ordinary section.
    CHECK(is_section_marker("section _endofsong", SectionReader::Mid));
    // The Rock Band spelling never meets the veto.
    CHECK(is_section_marker("prc__ENDOFSONG", SectionReader::Mid));
    // The .chart reader keeps it.
    CHECK(is_section_marker("section _ENDOFSONG", SectionReader::Chart));
    // The shown name is the same whichever reader asks.
    CHECK(section_display_name("section _ENDOFSONG") == " ENDOFSONG");
}
