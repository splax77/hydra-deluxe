#include "ui/details_parts.h"

#include <optional>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "core/model.h"  // group_thousands, counted
#include "imgui.h"
#include "ui/app_state.h"  // ViewedSong
#include "ui/column_widths.h"
#include "ui/fonts.h"
#include "ui/library_parts.h"  // format_duration
#include "ui/theme.h"
#include "ui/widgets.h"

namespace hydra::ui {

namespace {

// ---- Dynamics tab ----------------------------------------------------------

// A lane's dot colour: Clone Hero's standard lane colours.
ImVec4 lane_color(NoteColor color) {
    switch (color) {
        case NoteColor::Red:    return ImVec4(0.85f, 0.15f, 0.15f, 1.0f);
        case NoteColor::Yellow: return ImVec4(0.90f, 0.85f, 0.10f, 1.0f);
        case NoteColor::Blue:   return ImVec4(0.20f, 0.45f, 0.90f, 1.0f);
        case NoteColor::Green:  return ImVec4(0.15f, 0.75f, 0.20f, 1.0f);
        case NoteColor::Kick:   return ImVec4(0.90f, 0.55f, 0.10f, 1.0f);
    }
    return ImVec4(0.50f, 0.50f, 0.50f, 1.0f);  // unreachable
}

// A Dynamics row's dot colour: its lane's colour, read off the row table.
ImVec4 pad_color(app::DynamicsRow row) {
    return lane_color(app::dynamics_row_info(row).color);
}

// The dot's radius, and the room the dot item takes before the pad's name.
float pad_dot_radius() { return px(5.0f); }
float pad_dot_item_width() { return 2.0f * pad_dot_radius() + px(4.0f); }

// Draw a small filled circle in `color` before the next text on this line.
void pad_dot(const ImVec4& color) {
    float r = pad_dot_radius();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float y_off = (ImGui::GetTextLineHeight() - 2.0f * r) * 0.5f;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(p.x + r, p.y + y_off + r), r,
        ImGui::ColorConvertFloat4ToU32(color));
    ImGui::Dummy(ImVec2(pad_dot_item_width(), ImGui::GetTextLineHeight()));
    ImGui::SameLine();
}

// One row of a Ghost/Accent/Normal/All table, as its cells read.
struct DynamicsTableRow {
    std::string cells[5];  // Pad, Ghost, Accent, Normal, All
    bool disabled = false;
    std::optional<ImVec4> dot;
};

DynamicsTableRow dynamics_table_row(const std::string& label, const app::DynamicsCounts& c,
                                    bool disabled, std::optional<ImVec4> dot = std::nullopt) {
    return {{label, group_thousands(c.ghost), group_thousands(c.accent),
             group_thousands(c.normal), group_thousands(c.all())},
            disabled,
            dot};
}

// The columns of both tables. Pad is a name and may cut; the counts never do.
// Every Pad cell is measured with the dot's room beside it (the item and the
// gap after it), including "All kicks", which has no dot.
std::vector<ColumnSpec> dynamics_column_specs() {
    const WidthOf text = measure_in_font();
    const float dot = pad_dot_item_width() + ImGui::GetStyle().ItemSpacing.x;
    return {{"Pad", true, dot, text},
            {dynamic_label(NoteDynamicType::Ghost), false, 0.0f, text},
            {dynamic_label(NoteDynamicType::Accent), false, 0.0f, text},
            {"Normal", false, 0.0f, text},
            {"All", false, 0.0f, text}};
}

// A Ghost/Accent/Normal/All table, its columns as wide as the width rule
// (ui/column_widths.h) says. The rows are a handful, so they are measured
// each frame. A disabled row is dimmed.
void dynamics_table(const char* id, const std::vector<DynamicsTableRow>& rows) {
    const std::vector<ColumnSpec> specs = dynamics_column_specs();
    const MeasuredWidths measured = measure_widths(
        specs, rows.size(), [&](size_t r, size_t c) { return rows[r].cells[c]; });
    const TableRoom room = table_room(id, 0.0f, specs.size());
    const ColumnLayout layout = place_columns(measured, specs, room);
    if (!ImGui::BeginTable(id, static_cast<int>(specs.size()),
                           ImGuiTableFlags_RowBg | scroll_fixed_flags(),
                           ImVec2(0.0f, table_outer_height(rows.size(), layout, room)),
                           layout.inner_width))
        return;
    for (size_t c = 0; c < specs.size(); ++c) setup_column(specs[c], layout, static_cast<int>(c));
    apply_column_widths(layout);
    ImGui::TableHeadersRow();

    for (const DynamicsTableRow& row : rows) {
        ImGui::TableNextRow();
        // The style's own disabled text grey: it reads on the dark panel.
        if (row.disabled)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TableNextColumn();
        if (row.dot) pad_dot(*row.dot);
        text_ellipsized(row.cells[0].c_str());
        for (size_t c = 1; c < specs.size(); ++c) {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(row.cells[c].c_str());
        }
        if (row.disabled) ImGui::PopStyleColor();
    }
    ImGui::EndTable();
}

}  // namespace

namespace detail {

std::string dynamics_enabled_text(const app::DynamicsBreakdown& bd) {
    if (!bd.dynamics_enabled) return "Dynamics enabled: no (markings ignored by Clone Hero)";
    if (!bd.late_tag_ms) return "Dynamics enabled: yes";
    return "Dynamics enabled: from " + format_duration(*bd.late_tag_ms / 1000.0) + " on (" +
           counted(bd.marks_before_tag, "earlier marking", "earlier markings") +
           " ignored by Clone Hero)";
}

void render_dynamics_panel(AppState& app) {
    if (!app.selected) return;

    // The count comes from the click's job (AppState::viewed), which the
    // song panel's tick() collects whichever tab shows.
    const ViewedSong& viewed = app.viewed;
    if (viewed.state == ViewedSong::State::Analyzing) {
        ImGui::TextUnformatted("Reading chart...");
        return;
    }
    // Wrapped: the message can carry a long file path.
    if (!viewed.dynamics_error.empty()) {
        ImGui::TextWrapped("Dynamics failed: %s", viewed.dynamics_message.c_str());
        // The raw text, dimmed, as the song panel's error shows it.
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", viewed.dynamics_error.c_str());
        ImGui::PopStyleColor();
        return;
    }
    if (!viewed.dynamics) return;

    const app::DynamicsBreakdown& bd = *viewed.dynamics;
    bool pro = app.settings.view_prodrums;
    bool bass2x = app.settings.effective_bass2x();
    const app::DynamicsCounts played = bd.played_total(bass2x);

    // "This chart has no ghost or accent notes." above everything when no dynamics.
    if (!played.has_dynamics()) {
        ImGui::TextWrapped("This chart has no ghost or accent notes.");
        ImGui::Spacing();
    }

    float avail_w = ImGui::GetContentRegionAvail().x;
    float left_w = avail_w * 0.63f;

    // ---- Left box ----
    ImGui::BeginChild("dynleft", ImVec2(left_w, 0), ImGuiChildFlags_Borders);

    // Pads section.
    ImGui::SeparatorText("Pads");

    {
        // The pad rows (app::is_pad_row), in DynamicsRow order.
        // With Pro Drums off, skip the three Cymbal rows.
        std::vector<DynamicsTableRow> rows;
        for (int i = 0; i < static_cast<int>(app::DynamicsRow::Count); ++i) {
            auto r = static_cast<app::DynamicsRow>(i);
            if (!app::is_pad_row(r)) continue;
            // Skip cymbal rows when not pro.
            if (!pro && app::dynamics_row_info(r).cymbal) continue;
            const app::DynamicsCounts& c = bd.row(r);
            rows.push_back(dynamics_table_row(app::dynamics_row_label(r, pro), c,
                                              !c.has_dynamics(), pad_color(r)));
        }
        dynamics_table("##padtable", rows);
    }

    // Kicks section.
    ImGui::SeparatorText("Kicks");

    // How many of the chart's kick notes are 2x, counted or not.
    ImGui::TextWrapped("%s", app::dynamics_kick2x_line(bd).c_str());

    {
        std::vector<DynamicsTableRow> rows;
        const app::DynamicsCounts& k = bd.row(app::DynamicsRow::Kick);
        rows.push_back(dynamics_table_row(app::dynamics_row_label(app::DynamicsRow::Kick, pro), k,
                                          !k.has_dynamics(), pad_color(app::DynamicsRow::Kick)));
        const app::DynamicsCounts& k2 = bd.row(app::DynamicsRow::Kick2x);
        // With 2x Bass off, always draw the 2x kick row disabled
        // but keep its numbers.
        rows.push_back(dynamics_table_row(app::dynamics_row_label(app::DynamicsRow::Kick2x, pro),
                                          k2, !bass2x || !k2.has_dynamics(),
                                          pad_color(app::DynamicsRow::Kick2x)));
        // The same kicks Totals counts (finding 12).
        const app::DynamicsCounts ktot = bd.kicks_total(bass2x);
        rows.push_back(dynamics_table_row("All kicks", ktot, !ktot.has_dynamics()));
        dynamics_table("##kicktable", rows);
    }

    ImGui::EndChild();

    ImGui::SameLine();

    // ---- Right box ----
    // About 37% of the panel: every line wraps inside it rather than running
    // past its edge.
    ImGui::BeginChild("dynright", ImVec2(0, 0), ImGuiChildFlags_Borders);

    // Totals section.
    ImGui::SeparatorText("Totals");
    ImGui::TextWrapped("Ghosts: %s", group_thousands(played.ghost).c_str());
    ImGui::TextWrapped("Accents: %s", group_thousands(played.accent).c_str());
    {
        int dyn = played.dynamic();
        int total = played.all();
        ImGui::TextWrapped("Dynamic notes: %s of %s (%s)",
                           group_thousands(dyn).c_str(),
                           group_thousands(total).c_str(), app::dynamics_share(dyn, total).c_str());
    }

    // Chart section.
    ImGui::SeparatorText("Chart");
    ImGui::TextWrapped("%s", dynamics_enabled_text(bd).c_str());

    if (bass2x)
        ImGui::TextWrapped("2x kicks: counted (2x Bass on)");
    else
        ImGui::TextWrapped("2x kicks: not counted (2x Bass off)");

    ImGui::EndChild();
}

}  // namespace detail

}  // namespace hydra::ui
