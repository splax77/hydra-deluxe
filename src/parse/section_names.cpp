#include "parse/section_names.h"

#include <algorithm>
#include <optional>

namespace hydra {

namespace {

// Clone Hero's .mid reader drops a "[section" text that holds this anywhere,
// case counting; its "[prc" path and its .chart reader have no such test.
// The game's side is block CR-24 on docs/clone-hero/01-reading-a-chart.md.
constexpr std::string_view kMidSectionVeto = "_ENDOFSONG";

// The name after a marker's prefix, or nothing when the body is not a section
// marker. Both formats spell a marker one of two ways: "section Verse 2B"
// (Clone Hero) or "prc_verse_2b" (Rock Band). Nothing else is a section, and
// a .mid body the veto above names is not one either.
std::optional<std::string_view> marker_name(std::string_view body, SectionReader reader) {
    if (body.rfind("section ", 0) == 0) {
        if (reader == SectionReader::Mid && body.find(kMidSectionVeto) != std::string_view::npos)
            return std::nullopt;
        return body.substr(8);
    }
    if (body.rfind("prc_", 0) == 0) return body.substr(4);
    return std::nullopt;
}

}  // namespace

bool is_section_marker(std::string_view marker_body, SectionReader reader) {
    return marker_name(marker_body, reader).has_value();
}

std::string section_display_name(std::string_view marker_body) {
    // The name does not depend on the reader. The veto only says whether a
    // body is a section, and callers ask is_section_marker that first.
    const std::string_view name =
        marker_name(marker_body, SectionReader::Chart).value_or(marker_body);
    // Clone Hero matches the name exactly, case included: its keys are the
    // marker spellings charts use, so "Intro" written by hand is a miss.
    const SectionNameTable table = section_name_table();
    const SectionNameRow* end = table.rows + table.count;
    const SectionNameRow* it = std::lower_bound(
        table.rows, end, name,
        [](const SectionNameRow& row, std::string_view key) { return row.key < key; });
    if (it != end && it->key == name) return std::string(it->shown);
    // A miss shows the name with its underscores as spaces, as Clone Hero does.
    std::string shown(name);
    std::replace(shown.begin(), shown.end(), '_', ' ');
    return shown;
}

}  // namespace hydra
