#include "parse/section_names.h"

#include <algorithm>
#include <optional>

namespace hydra {

namespace {

// The name after a marker's prefix, or nothing when the body is not a section
// marker. Both formats spell a marker one of two ways: "section Verse 2B"
// (Clone Hero) or "prc_verse_2b" (Rock Band). Nothing else is a section.
std::optional<std::string_view> marker_name(std::string_view body) {
    if (body.rfind("section ", 0) == 0) return body.substr(8);
    if (body.rfind("prc_", 0) == 0) return body.substr(4);
    return std::nullopt;
}

}  // namespace

bool is_section_marker(std::string_view marker_body) {
    return marker_name(marker_body).has_value();
}

std::string section_display_name(std::string_view marker_body) {
    const std::string_view name = marker_name(marker_body).value_or(marker_body);
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
