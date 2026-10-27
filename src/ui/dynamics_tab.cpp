#include "ui/details_parts.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "core/model.h"  // group_thousands, counted
#include "imgui.h"
#include "ui/app_state.h"  // ViewedSong
#include "ui/column_widths.h"
#include "ui/fonts.h"
#include "ui/library_parts.h"  // format_duration
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

namespace hydra::ui {

namespace detail {

// A number column of the Dynamics and Stars tables: its header is a Label and
// its cells are Figures, each measured in its own role's font. The Stars tab
// declares it beside its use.
ColumnSpec figure_column_spec(std::string header) {
    ColumnSpec spec{std::move(header), false, 0.0f, measure_in_role(TextRole::Figure)};
    spec.header_width_of = measure_in_role(TextRole::Label);
    return spec;
}

}  // namespace detail

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
// Pad, the one text column, takes all the room the counts leave (a share of
// 100), so its dot and name are never measured. The counts are figure columns.
std::vector<ColumnSpec> dynamics_column_specs() {
    return {{"Pad", true, 0.0f, measure_in_role(TextRole::Label), 100},
            detail::figure_column_spec(dynamic_label(NoteDynamicType::Ghost)),
            detail::figure_column_spec(dynamic_label(NoteDynamicType::Accent)),
            detail::figure_column_spec("Normal"),
            detail::figure_column_spec("All")};
}

// A Ghost/Accent/Normal/All table, its columns as wide as the width rule
// (ui/column_widths.h) says. The rows are a handful, so they are measured
// each frame. A disabled row is dimmed.
void dynamics_table(const char* id, const std::vector<DynamicsTableRow>& rows) {
    const std::vector<ColumnSpec> specs = dynamics_column_specs();
    if (!begin_small_table(id, ImGuiTableFlags_RowBg, specs, rows.size(),
                           [&](size_t r, size_t c) { return rows[r].cells[c]; }))
        return;
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
            ScopedBaselineFont font(TextRole::Figure);
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
    // A failed count always has its message; its details line may be empty
    // (app::plain_error_detail).
    if (!viewed.dynamics_message.empty()) {
        ImGui::TextWrapped("Dynamics failed: %s", viewed.dynamics_message.c_str());
        error_detail_line(viewed.dynamics_error);
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

    // How many of the chart's kick notes are 2x, and whether they count.
    ImGui::TextWrapped("%s", app::dynamics_kicks_heading(bd, bass2x).c_str());

    {
        std::vector<DynamicsTableRow> rows;
        const app::DynamicsCounts& k = bd.row(app::DynamicsRow::Kick);
        rows.push_back(dynamics_table_row(app::dynamics_row_label(app::DynamicsRow::Kick, pro), k,
                                          !k.has_dynamics(), pad_color(app::DynamicsRow::Kick)));
        const app::DynamicsCounts& k2 = bd.row(app::DynamicsRow::Kick2x);
        // With 2x Bass off, always draw the 2x kick row disabled
        // but keep its numbers.
        rows.push_back(dynamics_table_row(app::dynamics_kick2x_row_label(pro, bass2x),
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

    // The Kicks line's 2x kick share, then whether those kicks count.
    ImGui::TextWrapped("%s", app::dynamics_kick2x_line(bd).c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, kSubtleTextColor);
    ImGui::TextWrapped("%s", app::dynamics_kick2x_status(bass2x).c_str());
    ImGui::PopStyleColor();

    ImGui::EndChild();
}

}  // namespace detail

}  // namespace hydra::ui
