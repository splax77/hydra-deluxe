#include "ui/details_parts.h"

#include "app/display_format.h"  // kAverageMultiplierLabel
#include "core/model.h"          // group_thousands
#include "core/stars.h"
#include "imgui.h"
#include "ui/column_widths.h"
#include "ui/widgets.h"  // overflow_tooltip

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace hydra::ui::detail {

// ---- Stars tab -------------------------------------------------------------

// Clone Hero's star cutoffs for this chart. Every number is star_cutoffs()'s,
// read from the record's best path (the base score and solo bonus are the
// same on every path); this only draws them. The states before the table are
// render_record_state's, shared with the Paths tab.
void render_stars_panel(AppState& app) {
    if (!render_record_state(app)) return;

    const StarCutoffs sc = star_cutoffs(app.viewed.record->best_path());
    const bool has_solo = sc.solo_bonus > 0;

    ImGui::Text("Base score: %s", group_thousands(sc.base).c_str());
    if (has_solo)
        ImGui::Text("Solo bonus: %s (not counted toward stars)",
                    group_thousands(sc.solo_bonus).c_str());
    ImGui::Spacing();

    // Every column is a number and never cuts. Each is as wide as the width
    // rule (ui/column_widths.h) says; the rows are a handful, so they are
    // measured each frame.
    const WidthOf text = measure_in_font();
    constexpr size_t kMultiplierColumn = 1;
    std::vector<ColumnSpec> specs = {{"Stars", false, 0.0f, text},
                                     {app::kAverageMultiplierLabel, false, 0.0f, text},
                                     {"Cutoff", false, 0.0f, text}};
    if (has_solo) specs.push_back({"With full solo bonus", false, 0.0f, text});
    std::vector<std::vector<std::string>> rows;
    for (int stars = 1; stars <= kMaxStars; ++stars) {
        char multiplier[16];
        std::snprintf(multiplier, sizeof(multiplier), "%.1f",
                      static_cast<double>(kStarMultipliers[stars - 1]));
        rows.push_back({std::to_string(stars), multiplier, group_thousands(sc.cutoffs[stars - 1])});
        if (has_solo) rows.back().push_back(group_thousands(sc.with_solo[stars - 1]));
    }
    if (begin_small_table("##startable", ImGuiTableFlags_RowBg, specs, rows.size(),
                          [&](size_t r, size_t c) { return rows[r][c]; })) {
        // The headers, drawn one by one so the factor column can explain
        // itself on hover.
        draw_table_headers(specs, [&](size_t c) {
            if (c == kMultiplierColumn)
                overflow_tooltip(
                    "The average multiplier a path needs for this many stars. The cutoff is "
                    "the Base score times this factor.");
        });
        for (const std::vector<std::string>& row : rows) {
            ImGui::TableNextRow();
            for (const std::string& cell : row) {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(cell.c_str());
            }
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextUnformatted(
        "A star counts once your score, without the solo bonus, reaches its cutoff.");
}

}  // namespace hydra::ui::detail
