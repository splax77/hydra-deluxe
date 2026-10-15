// The name a practice-section marker shows, as Clone Hero shows it. Clone Hero
// 1.1.0.6142 carries a list of marker names and the words it shows for each;
// data/clone-hero-1.1.0.6142-section-names.tsv is that list, and
// tools/gen_section_names.py turns it into section_names_table.cpp.
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace hydra {

// True when a text event's body (the part inside the brackets, or the quoted
// part of a .chart "E" line) marks a practice section. The two spellings it
// accepts live in section_names.cpp, beside section_display_name.
bool is_section_marker(std::string_view marker_body);

// The shown name of a practice-section marker, from the whole marker body
// (prefix included). This is the one place that answers it; both readers in
// song.cpp call it through section_name_of.
std::string section_display_name(std::string_view marker_body);

// One row of Clone Hero's list: a marker name and the words shown for it.
struct SectionNameRow {
    std::string_view key;
    std::string_view shown;
};

// The generated list, sorted by key in byte order (section_names_table.cpp).
// Exposed so a test can check it against the TSV it was generated from.
struct SectionNameTable {
    const SectionNameRow* rows;
    std::size_t count;
};
SectionNameTable section_name_table();

}  // namespace hydra
