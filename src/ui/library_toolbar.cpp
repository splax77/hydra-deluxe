#include "ui/library_parts.h"

#include "app/dm_report.h"  // why_not_comparable
#include "app/report_files.h"
#include "core/model.h"
#include "imgui.h"
#include "ui/fonts.h"
#include "ui/generation.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace hydra::ui::detail {

// The status line, on its own wrapping line under the toolbar. News is
// neutral and fades (AppState::kStatusFadeSeconds); a problem is orange and
// stays until its X is clicked.
void render_status_line(AppState& app) {
    // The watcher starts at "seen" for this app's counter, so startup does
    // not start a fade.
    GenerationWatcher& generation = app.library_ui.status_watcher;
    double& shown_at = app.library_ui.status_shown_at;
    if (generation.changed(app.status_generation)) shown_at = ImGui::GetTime();
    if (shown_at < 0.0 || app.status_message.empty()) return;
    if (!app.status_is_problem && ImGui::GetTime() - shown_at > AppState::kStatusFadeSeconds) return;
    if (app.status_is_problem) {
        if (ImGui::SmallButton("X##dismissstatus")) {
            app.dismiss_status();
            return;
        }
        hint("Dismiss");
        ImGui::SameLine();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, app.status_is_problem ? kWarningColor : kDefaultTextColor);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(app.status_message.c_str());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

std::string analyze_search_label(int64_t count) {
    return "Analyze search (" + group_thousands(count) + ")...";
}

// The toolbar: library-wide actions on the left, the last report on the right.
void render_actions_row(AppState& app) {
    const bool batch_busy = app.batch_running();
    auto busy_tooltip = [&] {
        if (batch_busy && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                                               ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Busy: a batch is running.");
    };

    // Buttons with live counts take the width of their widest label, so the
    // row doesn't reflow under the mouse when a count changes.
    int folder_count = (int)app.settings.chartfolders.size();
    char manage_label[64];
    std::snprintf(manage_label, sizeof(manage_label), "Manage folders... (%d)", folder_count);
    std::string manage_widest =
        "Manage folders... (" + widest_digits(digit_count(folder_count)) + ")";
    if (button_in_slot(manage_label, button_slot_width(manage_widest.c_str())))
        ImGui::OpenPopup("Song folders");
    hint("Add or remove the folders Hydra scans for charts");
    ImGui::SameLine();

    const bool scan_off = !app.can_scan();
    begin_disabled_button(scan_off);
    if (ImGui::Button("Scan library")) {
        app.start_scan();
        ImGui::OpenPopup("Scanning charts");
    }
    end_disabled_button(scan_off);
    busy_tooltip();
    ImGui::SameLine();

    // A search that narrows nothing (empty, or a filter that does not parse,
    // like "stars:9") leaves the button on the whole library (D48, Q15).
    const bool searching = app.library.searching();
    const int64_t library_size = static_cast<int64_t>(app.library.rows().size());
    const int64_t analyzable = searching ? static_cast<int64_t>(app.library_match_count()) : library_size;
    const std::string label =
        searching ? analyze_search_label(analyzable) : std::string("Analyze library...");
    // The slot fits the widest count this library can show, commas included:
    // the label's only digits are the count's.
    std::string sample = analyze_search_label(library_size);
    const char widest = widest_digits(1)[0];
    for (char& c : sample)
        if (c >= '0' && c <= '9') c = widest;
    const float analyze_w =
        std::max(button_slot_width("Analyze library..."), button_slot_width(sample.c_str()));
    // Off with nothing to analyze, under a bad hydra_rules.ini, or mid-batch.
    const bool analyze_off = analyzable == 0 || app.analysis_blocked() || batch_busy;
    begin_disabled_button(analyze_off);
    if (button_in_slot(label.c_str(), analyze_w)) app.open_batch_confirm();
    end_disabled_button(analyze_off);
    busy_tooltip();
    ImGui::SameLine();

    // app::dm_report::why_not_comparable says whether these settings can be
    // compared with the leaderboard. When they can't, the button is off and
    // its tooltip is the gate's sentence.
    const std::string refused = app::dm_report::why_not_comparable(
        app.settings.difficulty(), app.settings.sp_cap, app.settings.legacy_fills);
    const bool compare_off = !refused.empty();
    begin_disabled_button(compare_off);
    if (ImGui::Button("Compare with dmleaderboards...")) {
        // Fresh picker: drop a finished report (a running one is parked, not
        // joined), and refetch the ladder only when this session has none.
        app.cancel_dm_report();
        app.dm_picker_open = true;
        if (app.dm_users.empty()) app.start_dm_fetch();
        ImGui::OpenPopup("Compare dmleaderboards user");
    }
    end_disabled_button(compare_off);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
        if (compare_off)
            ImGui::SetTooltip("%s", refused.c_str());
        else
            ImGui::SetTooltip("Compare a dmleaderboards.com player's scores against your library");
    }

    // The last report, at the right. While one builds, a greyed button says so.
    const bool building = app.report_job && !app.report_job->finished();
    if (building || app.report_file_shown(ImGui::GetTime())) {
        const float report_w = std::max(button_slot_width("Open path report"),
                                        button_slot_width("Building path report..."));
        ImGui::SameLine();
        const float right = ImGui::GetContentRegionMax().x - report_w;
        if (right > ImGui::GetCursorPosX()) ImGui::SetCursorPosX(right);
        if (building) {
            begin_disabled_button(true);
            ImGui::Button("Building path report...", ImVec2(report_w, 0.0f));
            end_disabled_button(true);
        } else if (ImGui::Button("Open path report", ImVec2(report_w, 0.0f)) &&
                   !app::open_report_in_browser()) {
            app.set_problem("Windows couldn't open the path report in your browser.");
        }
    }

    render_status_line(app);
    // A bad rules file is not a passing message: it stays under the toolbar
    // for as long as analysis is off.
    if (app.analysis_blocked()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(kWarningColor,
                           "hydra_rules.ini has an error, so analysis is off until the "
                           "file is fixed and Hydra is restarted.");
        ImGui::TextColored(kWarningColor, "%s", app.rules_error.c_str());
        ImGui::PopTextWrapPos();
    }
    render_folder_manager(app);
}

}  // namespace hydra::ui::detail
