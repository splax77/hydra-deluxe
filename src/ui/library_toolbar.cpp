#include "ui/library_parts.h"

#include "app/dm_report.h"  // why_not_comparable
#include "app/user_messages.h"
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

// The update strip (docs/superpowers/plans/2026-10-22-auto-updater.md,
// decisions 1 to 4, 10 and 11): AppState::update's sentence in the normal
// text colour, the phase's buttons after it on the same line, and its detail
// line under it. Every word comes from UpdateState; this only draws it.
void render_update_strip(AppState& app) {
    using Phase = UpdateState::Phase;
    const UpdateState& update = app.update;
    if (update.phase == Phase::None) return;

    ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_Text, kDefaultTextColor);
    ImGui::TextUnformatted(update.text.c_str());
    ImGui::PopStyleColor();
    // The release's notes, as plain text: nothing in them is read as markup.
    if (update.release && !update.release->notes.empty() && ImGui::IsItemHovered())
        wrapped_tooltip(update.release->notes.c_str());

    const bool three_buttons = update.phase == Phase::Offered || update.phase == Phase::Declined;
    const bool failed = update.phase == Phase::Failed;
    if (three_buttons || (failed && update.release)) {
        ImGui::SameLine();
        if (ImGui::Button("Update##update")) app.update_now();
    }
    if (three_buttons) {
        ImGui::SameLine();
        if (ImGui::Button("Skip this version")) app.skip_version();
    }
    // A failed strip can be dismissed too: Later hides it until the next check.
    if (three_buttons || failed) {
        ImGui::SameLine();
        if (ImGui::Button("Later")) app.later();
    }
    if (update.phase == Phase::Downloading) {
        const std::string megabytes = app.update_megabytes_text();
        ImGui::SameLine();
        ImGui::TextUnformatted(megabytes.c_str());
        // The bar fills the line up to the Cancel button.
        const ImGuiStyle& style = ImGui::GetStyle();
        const float cancel_w = button_slot_width("Cancel");
        ImGui::SameLine();
        const float bar_w = std::max(ImGui::GetContentRegionAvail().x - cancel_w - style.ItemSpacing.x,
                                     ImGui::GetFrameHeight());
        const double total = update.release ? static_cast<double>(update.release->installer_size) : 0.0;
        ImGui::ProgressBar(progress_fraction(static_cast<double>(app.update_received()), total),
                           ImVec2(bar_w, 0.0f), "");
        ImGui::SameLine();
        if (ImGui::Button("Cancel##update")) app.cancel_update();
    }
    if (update.phase == Phase::Updated || update.phase == Phase::UpToDate) {
        ImGui::SameLine();
        if (ImGui::Button("OK##update")) app.dismiss_update_note();
    }
    error_detail_line(update.detail);
}

std::string analyze_shown_label(int64_t count) {
    return "Analyze shown (" + group_thousands(count) + ")...";
}

// The toolbar: library-wide actions on the left, the last report on the right.
void render_actions_row(AppState& app) {
    const bool batch_busy = app.batch_running();
    auto busy_tooltip = [&] {
        if (batch_busy && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                                               ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", AppState::kBatchRunningStatus);  // D114 item 12
    };

    // Buttons with live counts take the width of their widest label, so the
    // row doesn't reflow under the mouse when a count changes.
    int folder_count = (int)app.settings.chartfolders.size();
    char manage_label[64];
    std::snprintf(manage_label, sizeof(manage_label), "Manage folders... (%d)", folder_count);
    std::string manage_widest =
        "Manage folders... (" + widest_digits(digit_count(folder_count)) + ")";
    if (button_in_slot(manage_label, button_slot_width(manage_widest.c_str())))
        ImGui::OpenPopup(kFoldersModalId);
    hint("Add or remove the folders Hydra scans for charts");
    ImGui::SameLine();

    const bool scan_off = !app.can_scan();
    begin_disabled_button(scan_off);
    if (ImGui::Button("Scan library")) {
        app.start_scan();
        ImGui::OpenPopup(kScanModalId);
    }
    end_disabled_button(scan_off);
    busy_tooltip();
    ImGui::SameLine();

    // The button counts and runs the rows on screen, the heading's own number
    // (D115). When neither the search nor a chip narrows the table, that is the
    // whole library and the button says so. A search that does not parse, like
    // "stars:9", narrows nothing (D48, Q15).
    const bool narrowed = app.library.narrowed();
    const int64_t library_size = static_cast<int64_t>(app.library.rows().size());
    const int64_t analyzable = static_cast<int64_t>(app.library_shown_count());
    const std::string label =
        narrowed ? analyze_shown_label(analyzable) : std::string("Analyze library...");
    // The slot fits the widest count this library can show, commas included:
    // the label's only digits are the count's.
    std::string sample = analyze_shown_label(library_size);
    const char widest = widest_digits(1)[0];
    for (char& c : sample)
        if (c >= '0' && c <= '9') c = widest;
    const float analyze_w =
        std::max(button_slot_width("Analyze library..."), button_slot_width(sample.c_str()));
    // Off with nothing to analyze, under a bad hydra_rules.ini, or mid-batch.
    const bool analyze_off = analyzable == 0 || !app.can_analyze();
    begin_disabled_button(analyze_off);
    if (button_in_slot(label.c_str(), analyze_w)) app.open_batch_confirm();
    end_disabled_button(analyze_off);
    busy_tooltip();
    ImGui::SameLine();

    // app::dm_report::why_not_comparable says whether these settings can be
    // compared with the leaderboard. When they can't, the button is off and
    // its tooltip is the gate's sentence.
    const std::string refused =
        app::dm_report::why_not_comparable(app.settings.difficulty(), app.settings.sp_cap,
                                           app.settings.legacy_fills,
                                           app.settings.view_noteshuffle);
    const bool compare_off = !refused.empty();
    begin_disabled_button(compare_off);
    if (ImGui::Button("Compare with dmleaderboards...")) {
        // Fresh picker: a comparison still building stops (parked, not
        // joined); the picker opens on its next frame.
        app.cancel_dm_report();
        app.reopen_dm_picker();
    }
    end_disabled_button(compare_off);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
        if (compare_off)
            ImGui::SetTooltip("%s", refused.c_str());
        else
            ImGui::SetTooltip("Compare a dmleaderboards player's scores against your library.");
    }

    // The path report, at the right, whenever opening it can build one
    // (AppState::path_report_offered) or one is in memory (D103). While one
    // builds the button says so, and still opens the window in its building
    // state.
    const bool building = app.path_report_build() == ReportBuild::Building;
    const bool show_report = building || app.path_report.result || app.path_report_offered();
    const float report_w = std::max(button_slot_width("Open path report"),
                                    button_slot_width("Building path report..."));

    // The analysis settings, after the library actions. Its label is cut
    // short of the path report button.
    render_settings_button(app,
                           show_report ? report_w + ImGui::GetStyle().ItemSpacing.x : 0.0f);

    if (show_report) {
        ImGui::SameLine();
        place_control_at_right_edge(report_w);
        if (ImGui::Button(building ? "Building path report..." : "Open path report",
                          ImVec2(report_w, 0.0f)))
            app.show_path_report();
    }

    render_status_line(app);
    // A bad rules file is not a passing message: it stays under the toolbar
    // for as long as analysis is off.
    if (app.analysis_blocked()) {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextColored(kWarningColor, "%s", app::kAnalysisOffSentence);
        ImGui::TextColored(kWarningColor, "%s", app.rules_error.c_str());
        ImGui::PopTextWrapPos();
    }
    render_update_strip(app);
    render_folder_manager(app);
}

}  // namespace hydra::ui::detail
