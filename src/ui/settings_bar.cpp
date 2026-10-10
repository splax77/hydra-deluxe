// The "Analysis settings" button on the action row and the panel it opens:
// every setting an analysis runs with, for every song. Locked while a batch
// runs, because a result is filed under the settings it ran with -- changing
// SP cap mid-run used to hide the result it had just made.

#include "app/config.h"  // Settings::clamp, the one range each box asks
#include "core/model.h"
#include "imgui.h"
#include "imgui_internal.h"  // IsPopupOpen by id, SetKeyOwner
#include "parse/song.h"    // display_title
#include "search/graph.h"  // fill_rule_name, fill_rule_description
#include "ui/app_state.h"
#include "ui/fonts.h"
#include "ui/library_parts.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <iterator>
#include <string>

namespace hydra::ui::detail {

std::string legacy_fills_help_text() {
    return std::string("Spawn drum fills by ") +
           fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Long) +
           "'s rule instead of 1.1's. A fill only appears if your Star Power was "
           "ready in time. " +
           fill_rule_description(FillDeadlineRule::Ch11) + " " +
           fill_rule_description(FillDeadlineRule::Ch10) +
           " For runs played on 1.0; current Clone Hero plays by 1.1.";
}

namespace {

constexpr const char* kPanelId = "##settingspanel";

// The gap each side of the divider before the button, and the divider's
// width.
float separator_gap() { return px(14.0f); }
constexpr float kSeparatorW = 1.0f;

// A 1 px divider at screen x, from top to bottom, drawn straight onto the
// window so it takes no room on the line.
void draw_divider(float x, float top, float bottom) {
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x, top), ImVec2(x + kSeparatorW, bottom),
                                              ImGui::GetColorU32(ImGuiCol_Separator));
}

// A group of the panel: its dimmed heading, then a two-column form. The
// three groups' tables share one ID, so Dear ImGui keeps their label columns
// one width (synced tables).
bool begin_group(const char* title) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::SeparatorText(title);
    ImGui::PopStyleColor();
    return ImGui::BeginTable("##settingsform", 2, ImGuiTableFlags_SizingFixedFit);
}

// A new form row. Every row reads the same way: the setting's name and its
// (?) in the left column, and only the control in the right one, where this
// leaves the cursor.
void begin_row(const char* name, const char* help) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(name);
    help_marker(help);
    ImGui::TableSetColumnIndex(1);
}

// A bare checkbox (its name sits in the left column), greyed out while the
// settings are locked. True when the user changed it.
bool setting_checkbox(const char* id, bool* value, bool locked) {
    begin_disabled_checkbox(locked);
    const bool changed = ImGui::Checkbox(id, value);
    end_disabled_checkbox(locked);
    return changed;
}

void render_chart_group(AppState& app, bool locked) {
    if (!begin_group("Chart")) return;
    begin_row("Difficulty", "Which charted difficulty to analyze, path and preview");
    ImGui::SetNextItemWidth(px(90));
    const char* names[std::size(kAllDifficulties)];
    for (size_t i = 0; i < std::size(kAllDifficulties); ++i)
        names[i] = difficulty_name(kAllDifficulties[i]);
    int idx = static_cast<int>(app.settings.difficulty());
    begin_disabled_input(locked);
    if (ImGui::Combo("##difficulty", &idx, names, IM_ARRAYSIZE(names))) {
        app.settings.view_difficulty = names[idx];
        // A different difficulty is a different chartmode: commit_settings
        // re-reads the library and rewrites the INI.
        app.commit_settings();
    }
    end_disabled_input(locked);

    begin_row("Pro Drums",
              "Analyze with cymbals and toms as separate notes, the way Clone Hero scores "
              "Pro Drums.");
    if (setting_checkbox("##prodrums", &app.settings.view_prodrums, locked)) app.commit_settings();

    // 2x Bass works at every difficulty, like Clone Hero's Double Kick (D20):
    // each difficulty has its own 2x kicks.
    begin_row("2x Bass", "Include the chart's 2x kicks, like Clone Hero's Double Kick.");
    bool bass2x_shown = app.settings.effective_bass2x();
    if (setting_checkbox("##bass2x", &bass2x_shown, locked)) {
        app.settings.view_bass2x = bass2x_shown;
        app.commit_settings();
    }

    // Part of a result's key, like the two before it (D104 item 1).
    begin_row("Note Shuffle",
              "Score the chart as Clone Hero 1.1's Note Shuffle modifier rearranges it. The "
              "rearrangement is the same every time for a given chart and drum setting.");
    if (setting_checkbox("##noteshuffle", &app.settings.view_noteshuffle, locked))
        app.commit_settings();
    ImGui::EndTable();
}

// The two Clone Hero rules a result is keyed by: the SP cap and when fills
// spawn.
void render_rules_group(AppState& app, bool locked) {
    if (!begin_group("Clone Hero rules")) return;
    begin_row("SP cap", (std::to_string(kCloneHeroSpCap) +
                         " bars is Clone Hero's rule. Higher caps are what-ifs; their scores "
                         "are not achievable in game.")
                            .c_str());
    ImGui::SetNextItemWidth(px(90));
    begin_disabled_input(locked);
    // Number boxes apply every step live but save the INI once the edit ends
    // (AppState::edit_settings, flushed by run_frame).
    int cap = app.settings.sp_cap;
    if (ImGui::InputInt("##spcap", &cap)) {
        app.settings.sp_cap = app::Settings::clamp(&app::Settings::sp_cap, cap);
        app.edit_settings();
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("bars");
    end_disabled_input(locked);

    begin_row("1.0 fills", legacy_fills_help_text().c_str());
    if (setting_checkbox("##legacyfills", &app.settings.legacy_fills, locked))
        app.commit_settings();
    ImGui::EndTable();
}

void render_paths_group(AppState& app, bool locked) {
    if (!begin_group("Paths kept")) return;
    begin_row("Score range",
              "How many extra paths below optimal to keep: a number of scores, or of "
              "points. More paths take longer to analyze.");
    // Room for six digits beside the two step buttons (each a frame-height
    // square after an inner gap), never less than the old 90 px.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float six_digits = ImGui::CalcTextSize(widest_digits(6).c_str()).x +
                             style.FramePadding.x * 2.0f +
                             (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x) * 2.0f;
    ImGui::SetNextItemWidth(std::max(px(90), six_digits));
    begin_disabled_input(locked);
    if (ImGui::InputInt("##depthvalue", &app.settings.depth_value)) {
        app.settings.depth_value =
            app::Settings::clamp(&app::Settings::depth_value, app.settings.depth_value);
        app.edit_settings();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(px(80));
    int mode_idx = app.settings.depth_mode;
    const char* modes[] = {"scores", "points"};
    if (ImGui::Combo("##depthmode", &mode_idx, modes, 2)) {
        app.settings.depth_mode = mode_idx;
        app.commit_settings();
    }
    end_disabled_input(locked);

    begin_row("Path limit",
              "Keep extra paths only when their hardest squeeze or required early fill is "
              "within this many ms. Lower or negative values demand more slack.");
    if (setting_checkbox("##mslimit", &app.settings.mslimit_enabled, locked))
        app.commit_settings();
    ImGui::SameLine();
    const bool off = locked || !app.settings.mslimit_enabled;
    begin_disabled_input(off);
    ImGui::SetNextItemWidth(px(100));
    if (ImGui::InputInt("##mslimitvalue", &app.settings.mslimit_value)) {
        app.settings.mslimit_value =
            app::Settings::clamp(&app::Settings::mslimit_value, app.settings.mslimit_value);
        app.edit_settings();
    }
    ImGui::SameLine();
    ImGui::TextUnformatted("ms");
    end_disabled_input(off);
    ImGui::EndTable();
}

// "Reset to defaults": puts the panel's eight settings back in one change,
// so the open song re-analyzes once. Greyed out while locked or when there
// is nothing to undo; the tooltip says which, or what the click undoes.
void render_reset_button(AppState& app, bool locked) {
    const bool off = locked || settings_at_defaults(app.settings);
    begin_disabled_button(off);
    if (ImGui::Button("Reset to defaults")) {
        app.settings = app.settings.with_analysis_defaults();
        app.commit_settings();
    }
    end_disabled_button(off);
    overflow_tooltip(reset_settings_tooltip(app.settings, locked).c_str());
}

// The panel's body: the three groups, the reset button under a line, then
// the lock line while a batch holds the settings (only a batch locks them,
// D90 item 2).
void render_panel(AppState& app, bool locked) {
    render_chart_group(app, locked);
    render_rules_group(app, locked);
    render_paths_group(app, locked);
    ImGui::Separator();
    render_reset_button(app, locked);
    if (locked) {
        ImGui::PushStyleColor(ImGuiCol_Text, kSubtleTextColor);
        ImGui::TextUnformatted(kSettingsLockedText);
        ImGui::PopStyleColor();
    }
}

// A small downward arrow, ending `right` px in from the button's right edge.
void draw_open_arrow(const ImVec2& min, const ImVec2& max, float w, float right) {
    const float x = max.x - right - w;
    const float mid = (min.y + max.y) * 0.5f;
    const float h = w * 0.5f;
    ImGui::GetWindowDrawList()->AddTriangleFilled(
        ImVec2(x, mid - h * 0.5f), ImVec2(x + w, mid - h * 0.5f), ImVec2(x + w * 0.5f, mid + h * 0.5f),
        ImGui::GetColorU32(kSubtleTextColor));
}

}  // namespace

void render_settings_button(AppState& app, float room_after) {
    // Read once: a batch that ends on its worker mid-frame must not leave the
    // button saying "(locked)" over a panel of enabled controls.
    const AppState::SettingsLock lock = app.settings_lock();
    const bool locked = lock != AppState::SettingsLock::None;

    const ImGuiID panel_id = ImGui::GetID(kPanelId);
    // Dear ImGui closes an open popup on Esc in NewFrame, but the key still
    // reads as pressed for the rest of the frame. Claim it, so the song
    // panel's own Esc (details_panel.cpp) doesn't close that too.
    bool& was_open = app.library_ui.settings_panel_was_open;
    if (was_open && !ImGui::IsPopupOpen(panel_id, ImGuiPopupFlags_None) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        ImGui::SetKeyOwner(ImGuiKey_Escape, panel_id, ImGuiInputFlags_LockThisFrame);

    const ImGuiStyle& style = ImGui::GetStyle();
    const std::string label = settings_button_label(app.settings, locked);
    const float arrow_w = ImGui::GetFontSize() * 0.5f;
    const float chrome = style.FramePadding.x * 2.0f + style.ItemInnerSpacing.x + arrow_w;
    const float lead = separator_gap() * 2.0f + kSeparatorW;
    // The button stays on the action row while the default label still fits
    // there; a longer label is cut to the room left. Below that it starts a
    // line of its own.
    static const std::string kDefaultLabel = settings_button_label(app::Settings{}, false);
    const float least = ImGui::CalcTextSize(kDefaultLabel.c_str()).x + chrome;
    if (fits_on_line(lead + least + room_after, style.ItemSpacing.x)) {
        const float top = ImGui::GetItemRectMin().y, bottom = ImGui::GetItemRectMax().y;
        draw_divider(ImGui::GetItemRectMax().x + separator_gap(), top, bottom);
        ImGui::SameLine(0.0f, lead);
    }
    const float room = std::max(ImGui::GetContentRegionAvail().x - room_after, least);
    const float text_room = std::min(ImGui::CalcTextSize(label.c_str()).x, room - chrome);
    const std::string shown = render::ellipsize(label, text_room, text_width);

    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBgActive));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));
    const bool clicked =
        ImGui::Button((shown + "###analysissettings").c_str(), ImVec2(text_room + chrome, 0.0f));
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    const ImVec2 button_min = ImGui::GetItemRectMin(), button_max = ImGui::GetItemRectMax();
    draw_open_arrow(button_min, button_max, arrow_w, style.FramePadding.x);
    if (shown != label) overflow_tooltip(label.c_str());

    if (clicked) ImGui::OpenPopup(kPanelId);
    // Under the button: from its left edge, or back from its right edge when
    // it sits in the window's right half, so the panel stays on screen.
    const bool right_half = button_min.x > ImGui::GetWindowPos().x + ImGui::GetWindowWidth() * 0.5f;
    ImGui::SetNextWindowPos(ImVec2(right_half ? button_max.x : button_min.x, button_max.y),
                            ImGuiCond_Always, ImVec2(right_half ? 1.0f : 0.0f, 0.0f));
    if (ImGui::BeginPopup(kPanelId)) {
        render_panel(app, locked);
        ImGui::EndPopup();
    }
    was_open = ImGui::IsPopupOpen(panel_id, ImGuiPopupFlags_None);
}

}  // namespace hydra::ui::detail
