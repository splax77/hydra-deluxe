#include "ui/library_parts.h"

#include "app/display_format.h"  // kFactSeparator
#include "app/user_messages.h"
#include "core/model.h"
#include "core/strutil.h"
#include "imgui.h"
#include "search/graph.h"  // fill_rule_name
#include "store/record_store.h"
#include "ui/fonts.h"
#include "ui/library_view.h"  // kProgressBoxWidth
#include "ui/theme.h"
#include "ui/widgets.h"
#include "ui/win32_dialogs.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hydra::ui {

namespace {

HWND main_hwnd() {
    return static_cast<HWND>(ImGui::GetMainViewport()->PlatformHandleRaw);
}

}  // namespace

namespace detail {

namespace {

// The strips' colours. The done and problem backgrounds are the theme's
// (kDoneStripBg, kProblemStripBg), shared with the report windows. Each text
// colour was measured on its strip: (250,250,250) is 11.97:1 on the running
// strip; the secondary lines are 9.47:1, 9.27:1 and 9.38:1 on the running,
// done and problem strips; the teal bar on its track is 6.4:1.
const ImVec4 kStripBg{22 / 255.0f, 57 / 255.0f, 58 / 255.0f, 1.0f};
const ImVec4 kStripText2{200 / 255.0f, 230 / 255.0f, 230 / 255.0f, 1.0f};
const ImVec4 kStripTrack{11 / 255.0f, 35 / 255.0f, 36 / 255.0f, 1.0f};
const ImVec4 kDoneText2{196 / 255.0f, 220 / 255.0f, 199 / 255.0f, 1.0f};
const ImVec4 kProblemText2{230 / 255.0f, 205 / 255.0f, 180 / 255.0f, 1.0f};

// Enter or keypad Enter this frame.
bool enter_pressed() {
    return ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
           ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);
}

// The strip's counts, worded by app::batch_counts_text.
std::string batch_counts(const BatchJob::Snapshot& s) {
    return app::batch_counts_text(s.analyzed, s.failed, s.skipped);
}

// The setting values both summaries print, worded by app::describe_settings:
// the batch confirm and the settings button wrap them in their own words.
app::SettingsText setting_values(const app::Settings& s) {
    return app::describe_settings(s.to_analysis_settings());
}

}  // namespace

std::string format_duration(double seconds) {
    const long long total = seconds <= 0.0 ? 0 : static_cast<long long>(seconds + 0.5);
    const long long h = total / 3600, m = (total / 60) % 60, s = total % 60;
    char buf[32];
    if (h > 0)
        std::snprintf(buf, sizeof(buf), "%lld:%02lld:%02lld", h, m, s);
    else
        std::snprintf(buf, sizeof(buf), "%lld:%02lld", m, s);
    return buf;
}

BatchSettingsSummary batch_settings_summary(const app::Settings& s) {
    BatchSettingsSummary out;
    out.difficulty = difficulty_name(s.difficulty());
    if (s.view_prodrums) out.difficulty += app::kFactSeparator + std::string("Pro Drums");
    if (s.effective_bass2x()) out.difficulty += app::kFactSeparator + std::string("2x Bass");
    if (s.view_noteshuffle) out.difficulty += app::kFactSeparator + std::string("Note Shuffle");
    const app::SettingsText values = setting_values(s);
    out.sp_cap =
        values.cap + (is_clone_hero_cap(s.sp_cap) ? " (Clone Hero's rule)" : " (a what-if)");
    out.fills = fill_rule_name(fill_rule_for(s.legacy_fills), FillRuleNameStyle::Long);
    out.score_range = values.depth;
    out.path_limit = values.timing;
    return out;
}

namespace {

// One panel setting in the summary's words, as `s` has it.
std::string change_phrase(const app::Settings& s, const app::SettingsText& values,
                          app::AnalysisSetting setting) {
    using app::AnalysisSetting;
    switch (setting) {
    case AnalysisSetting::Difficulty: return difficulty_name(s.difficulty());
    case AnalysisSetting::ProDrums: return s.view_prodrums ? "Pro Drums" : "Pro Drums off";
    case AnalysisSetting::Bass2x: return s.effective_bass2x() ? "2x Bass" : "2x Bass off";
    case AnalysisSetting::NoteShuffle:
        return s.view_noteshuffle ? "Note Shuffle" : "Note Shuffle off";
    case AnalysisSetting::SpCap: return "SP cap " + values.cap;
    case AnalysisSetting::Fills: return s.legacy_fills ? "1.0 fills" : "1.0 fills off";
    case AnalysisSetting::ScoreRange: return "Score range " + values.depth;
    case AnalysisSetting::PathLimit: return "Path limit " + values.timing;
    }
    // A panel setting added to app::AnalysisSetting without words here; the
    // Reset test in tests/test_config.cpp walks every one and fails on this.
    throw std::logic_error("settings summary: a panel setting has no phrase");
}

// One phrase per panel setting that differs from its default, in panel order
// (Settings::analysis_changes, which is what a reset undoes).
// settings_changes_summary words it; settings_at_defaults asks if it's empty.
std::vector<std::string> settings_change_phrases(const app::Settings& s) {
    const app::SettingsText values = setting_values(s);
    std::vector<std::string> out;
    for (const app::AnalysisSetting setting : s.analysis_changes())
        out.push_back(change_phrase(s, values, setting));
    return out;
}

}  // namespace

std::string settings_changes_summary(const app::Settings& s) {
    std::string out;
    for (const std::string& phrase : settings_change_phrases(s)) {
        if (!out.empty()) out += app::kFactSeparator;
        out += phrase;
    }
    return out.empty() ? "defaults" : out;
}

std::string settings_button_label(const app::Settings& s, bool locked) {
    return "Analysis settings: " + settings_changes_summary(s) + (locked ? " (locked)" : "");
}

bool settings_at_defaults(const app::Settings& s) {
    return settings_change_phrases(s).empty();
}

std::string reset_settings_tooltip(const app::Settings& s, bool locked) {
    if (locked) return kSettingsLockedText;
    if (settings_at_defaults(s)) return "The settings are already the defaults.";
    return "Resets: " + settings_changes_summary(s);
}

std::string scan_folders_found_text(int64_t folders_seen) {
    return "Discovering folders... (" + group_thousands(folders_seen) + " found)";
}

std::string scan_reused_text(int64_t charts_cached) {
    return group_thousands(charts_cached) + " unchanged since last scan, reused.";
}

const char* empty_library_message(const app::Settings& s) {
    return s.chartfolders.empty()
               ? "No song folders yet. Click \"Manage folders...\" to add the folder your "
                 "charts are in, then \"Scan library\"."
               : "No charts found yet. Click \"Scan library\" to read your song folders.";
}

// The "Song folders" modal: the folder list with add/remove, opened from the
// "Manage folders..." button. Folder management moved off the main screen so
// the library table owns it — setup is a once-in-a-while task, but its list
// used to sit above the table on every launch.
void render_folder_manager(AppState& app) {
    // Folder pending removal, confirmed through the nested popup below —
    // deleting a library source was previously a single un-undoable click.
    std::optional<size_t>& confirm_remove = app.library_ui.confirm_remove;

    if (!ImGui::BeginPopupModal(kFoldersModalId, nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    ImGui::TextUnformatted("Hydra scans these folders (and their subfolders) for charts:");
    float row_h = ImGui::GetTextLineHeightWithSpacing();
    float list_rows =
        std::clamp((float)app.settings.chartfolders.size(), 1.0f, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kFolderListBg);
    ImGui::BeginChild("songfolders", ImVec2(px(620), row_h * list_rows + px(8)),
                      ImGuiChildFlags_Borders);
    if (app.settings.chartfolders.empty()) {
        ImGui::TextDisabled("(None.)");
    } else {
        for (size_t i = 0; i < app.settings.chartfolders.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            ImGui::PushStyleColor(ImGuiCol_Button, kDeleteButtonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kDeleteButtonHoveredColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, kDeleteButtonActiveColor);
            if (ImGui::Button("X")) confirm_remove = i;
            ImGui::PopStyleColor(3);
            hint("Remove this folder from the library");
            ImGui::SameLine();
            text_ellipsized(app.settings.chartfolders[i].c_str());
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (ImGui::Button("Add folder...")) {
        bool dialog_failed = false;
        if (auto folder = browse_for_folder(main_hwnd(), &dialog_failed)) {
            bool already = false;
            for (const std::string& f : app.settings.chartfolders)
                if (f == *folder) already = true;
            if (already) {
                app.set_status("That folder is already in the list.");
            } else {
                app.settings.chartfolders.push_back(*folder);
                app.settings.is_rescan = false;
                app.commit_settings();
                app.library_ui.folders_changed = true;
            }
        } else if (dialog_failed) {
            app.set_problem("The folder picker could not be opened.");
        }
    }
    ImGui::SameLine();
    const bool nested_open = ImGui::IsPopupOpen("Remove folder?");
    bool close = ImGui::Button("Close") ||
                 (!nested_open && ImGui::IsKeyPressed(ImGuiKey_Escape, false));
    // A changed list is only real after a scan: offer it right here.
    if (app.library_ui.folders_changed && !app.settings.chartfolders.empty()) {
        ImGui::SameLine();
        if (ImGui::Button("Scan now")) {
            app.request_scan = true;  // the main window starts it next frame
            close = true;
        }
    }
    if (close) {
        app.library_ui.folders_changed = false;
        ImGui::CloseCurrentPopup();
    }
    render_status_line(app);

    // Nested confirm, stacked on top of this modal.
    if (confirm_remove && !ImGui::IsPopupOpen("Remove folder?"))
        ImGui::OpenPopup("Remove folder?");
    if (ImGui::BeginPopupModal("Remove folder?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!confirm_remove || *confirm_remove >= app.settings.chartfolders.size()) {
            confirm_remove.reset();
            ImGui::CloseCurrentPopup();
        } else {
            ImGui::TextUnformatted("Remove this folder from the library?");
            ImGui::TextDisabled("%s", app.settings.chartfolders[*confirm_remove].c_str());
            ImGui::TextUnformatted("Its charts disappear from the list on the next scan.");
            ImGui::Spacing();
            if (ImGui::Button("Remove") || enter_pressed()) {
                app.settings.chartfolders.erase(app.settings.chartfolders.begin() +
                                                (long)*confirm_remove);
                app.settings.is_rescan = false;
                app.commit_settings();
                app.library_ui.folders_changed = true;
                confirm_remove.reset();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                confirm_remove.reset();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::EndPopup();
}

void render_scan_modal(AppState& app) {
    pin_next_modal_width(px(kProgressBoxWidth));
    if (!ImGui::BeginPopupModal(kScanModalId, nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }

    ScanProgress p = app.scan_job->snapshot();
    ImGui::TextUnformatted(scan_folders_found_text(p.folders_seen).c_str());
    if (p.phase != ScanProgress::Phase::Enumerating) {
        ImGui::TextUnformatted("Reading charts...");
        progress_bar_counted(p.charts_done, p.charts_total);
        if (p.charts_cached > 0) ImGui::TextUnformatted(scan_reused_text(p.charts_cached).c_str());
    }

    if (p.phase == ScanProgress::Phase::Writing)
        ImGui::TextUnformatted("Writing library...");

    if (p.phase == ScanProgress::Phase::Done) {
        if (p.cancelled) {
            ImGui::TextUnformatted("Scan cancelled. The library was left unchanged.");
            if (ImGui::Button("Continue") || enter_pressed() ||
                ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                app.scan_job.reset();
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::Text("Done: %s found.", counted(p.charts_found, "chart", "charts").c_str());
            if (!p.errors.empty()) {
                // "problems", not "skipped items": the list can also carry
                // a failed library write, which is not a skipped chart.
                ImGui::TextColored(kWarningColor, "%s during the scan:",
                                   counted((int64_t)p.errors.size(), "problem", "problems")
                                       .c_str());
                ImGui::BeginChild("scanerrors", ImVec2(-1, px(100)), ImGuiChildFlags_Borders);
                for (const std::string& e : p.errors) ImGui::TextUnformatted(e.c_str());
                ImGui::EndChild();
            }
            if (ImGui::Button("Continue") || enter_pressed() ||
                ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                app.settings.is_rescan = true;
                app.commit_settings();
                app.scan_job.reset();
                ImGui::CloseCurrentPopup();
            }
        }
    } else {
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            app.scan_job->cancel();
    }

    ImGui::EndPopup();
}

// The one question before a batch: how many charts, every setting it will
// run with, and whether to redo charts that already have a result. Esc and
// the title-bar X cancel; Enter starts (unless Cancel has keyboard focus).
void render_batch_confirm(AppState& app) {
    if (!app.batch_confirm_pending) return;
    if (!ImGui::IsPopupOpen("Analyze library")) ImGui::OpenPopup("Analyze library");
    pin_next_modal_width(px(520.0f));
    bool open = true;
    if (!ImGui::BeginPopupModal("Analyze library", &open, ImGuiWindowFlags_AlwaysAutoResize)) {
        app.close_batch_confirm();  // closed without our buttons
        return;
    }

    // Library rows, each copy of a chart counted, like the library's own
    // counts (D76). What runs is the plan the box picks (D79).
    const int64_t with = app.batch_scope_with_result();
    const int64_t to_run = app.batch_plan_for(app.batch_redo).todo_rows();
    std::string question;
    if (to_run == 0)
        question = "Every chart here already has a result.";
    else if (app.batch_redo && with > 0)
        question = "Analyze " + counted(to_run, "chart", "charts") + ", re-analyzing " +
                   group_thousands(with) + " that already " + has_have(with) + " a result?";
    else
        question = "Analyze " + counted(to_run, "chart", "charts") + " that " +
                   has_have(to_run) + " no result yet?";
    ImGui::PushFont(nullptr, 20.0f);
    ImGui::TextWrapped("%s", question.c_str());
    ImGui::PopFont();

    const BatchSettingsSummary d = batch_settings_summary(app.settings);
    if (ImGui::BeginTable("##batchsettings", 2,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersOuter)) {
        auto row = [](const char* key, const std::string& value) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", key);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(value.c_str());
        };
        row("Difficulty", d.difficulty);
        row("SP cap", d.sp_cap);
        row("Fills", d.fills);
        row("Score range", d.score_range);
        row("Path limit", d.path_limit);
        ImGui::EndTable();
    }
    ImGui::TextDisabled("To change these, cancel and edit Analysis settings on the main screen.");

    begin_disabled_checkbox(with == 0);
    ImGui::Checkbox("Also re-analyze charts that already have a result##redo", &app.batch_redo);
    end_disabled_checkbox(with == 0);
    ImGui::SameLine();
    ImGui::TextDisabled("(%s)", counted(with, "chart", "charts").c_str());

    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted("It runs in the background, so you can keep browsing. Analysis "
                           "settings stay locked until it finishes or you stop it. Stop keeps "
                           "every result finished so far.");
    ImGui::PopTextWrapPos();
    ImGui::Spacing();

    const bool can_start = to_run > 0 && app.can_analyze();
    const float start_w = button_slot_width("Start analyzing");
    const float cancel_w = button_slot_width("Cancel");
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - start_w - cancel_w -
                         ImGui::GetStyle().ItemSpacing.x);
    bool cancel = !open || ImGui::IsKeyPressed(ImGuiKey_Escape, false);
    if (ImGui::Button("Cancel")) cancel = true;
    const bool cancel_focused = ImGui::IsItemFocused();
    ImGui::SameLine();
    begin_disabled_button(!can_start);
    bool start = ImGui::Button("Start analyzing");
    end_disabled_button(!can_start);
    ImGui::SetItemDefaultFocus();
    if (can_start && !cancel_focused && enter_pressed()) start = true;

    if (start && can_start) {
        app.start_batch(app.batch_redo);  // clears batch_confirm_pending
        ImGui::CloseCurrentPopup();
    } else if (cancel) {
        app.close_batch_confirm();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// The running batch, in a strip under the toolbar: what it's doing, how far,
// how long, and Pause / Stop. The buttons sit at fixed x so live numbers
// never walk them around.
void render_batch_strip(AppState& app) {
    if (!app.batch_job) return;
    const BatchJob::Snapshot s = app.batch_job->snapshot();
    if (s.finished) return;
    const bool stopping = app.batch_job->is_cancelled();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, kStripBg);
    ImGui::BeginChild("##batchstrip", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleColor();

    const ImGuiStyle& style = ImGui::GetStyle();
    const float pause_w = std::max(button_slot_width("Pause"), button_slot_width("Resume"));
    const float stop_w = button_slot_width("Stop");
    const float buttons_x = ImGui::GetContentRegionMax().x - pause_w - stop_w - style.ItemSpacing.x;
    const float text_w = buttons_x - ImGui::GetCursorPosX() - style.ItemSpacing.x;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginChild("##striptext", ImVec2(text_w, 0.0f), ImGuiChildFlags_AutoResizeY);
    ImGui::PopStyleColor();
    if (s.preparing) {
        ImGui::TextUnformatted("Preparing the chart list...");
    } else {
        std::string line = stopping ? "Stopping..." : s.paused ? "Paused" : "Analyzing...";
        line += "  " + group_thousands(s.completed) + " of " + group_thousands(s.total);
        if (!s.current_title.empty() && !s.paused) {
            // Not kFactSeparator: this one dot has two spaces each side, like
            // the wide gap before the count, and stays as the strip shows it.
            line += "  \xC2\xB7  Now: " + s.current_title;
            if (!s.current_artist.empty()) line += app::kFactSeparator + s.current_artist;
        }
        text_ellipsized(line.c_str());

        ImGui::PushStyleColor(ImGuiCol_FrameBg, kStripTrack);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, kAccentColor);
        ImGui::ProgressBar(progress_fraction(s.completed, s.total), ImVec2(-1.0f, px(6.0f)), "");
        ImGui::PopStyleColor(2);

        std::string time = format_duration(s.elapsed_s) + " elapsed";
        if (s.eta_s) time += app::kFactSeparator + time_left_text(*s.eta_s);
        ImGui::PushStyleColor(ImGuiCol_Text, kStripText2);
        ImGui::TextUnformatted(batch_counts(s).c_str());
        // The time starts right after the counts: text at the left, as
        // apply_theme (ui/theme.cpp) has it (the user's answer 8).
        ImGui::SameLine();
        ImGui::TextUnformatted(time.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::SetCursorPosX(buttons_x);
    const bool pause_off = stopping || s.preparing;
    begin_disabled_button(pause_off);
    if (button_in_slot(s.paused ? "Resume" : "Pause", pause_w)) {
        if (s.paused) app.batch_job->resume();
        else app.batch_job->pause();
    }
    end_disabled_button(pause_off);
    ImGui::SameLine();
    begin_disabled_button(stopping);
    if (button_in_slot("Stop", stop_w)) app.batch_job->stop();
    end_disabled_button(stopping);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Keeps the %s already finished.",
                          counted(s.analyzed, "chart", "charts").c_str());
    }
    ImGui::EndChild();
}

// The finished batch: the counts, how long it took, how its path report
// went, and a way to open it. It stays until its X is clicked.
void render_batch_done(AppState& app) {
    if (!app.batch_job) return;
    const BatchJob::Snapshot s = app.batch_job->snapshot();
    if (!s.finished) return;
    const bool stopped = app.batch_job->is_cancelled();
    // The report's outcome shows once this run has started its report
    // (AppState::update_background_jobs); before that the slot holds an
    // older one.
    const ReportBuild build =
        app.report_started ? app.path_report_build() : ReportBuild::None;
    const bool building = build == ReportBuild::Building;
    const bool report_ok = build == ReportBuild::Ready;
    const bool report_failed = build == ReportBuild::Failed;

    const bool problem = report_failed;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, problem ? kProblemStripBg : kDoneStripBg);
    ImGui::BeginChild("##batchdone", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleColor();
    const ImVec4 text2 = problem ? kProblemText2 : kDoneText2;

    // Buttons at the right, placed first so the text can take the rest.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float close_w = ImGui::GetFrameHeight();
    const float open_w = button_slot_width("Open report");
    const float buttons_w = (report_ok ? open_w + style.ItemSpacing.x : 0.0f) + close_w;
    const float left_x = ImGui::GetCursorPosX();
    const float top_y = ImGui::GetCursorPosY();
    const float text_w = ImGui::GetContentRegionAvail().x - buttons_w - style.ItemSpacing.x;
    ImGui::SetCursorPosX(left_x + text_w + style.ItemSpacing.x);
    if (report_ok) {
        if (button_in_slot("Open report", open_w)) app.show_path_report();
        ImGui::SameLine();
    }
    if (ImGui::Button("X##dismissdone", ImVec2(close_w, close_w))) {
        app.batch_job.reset();  // update_background_jobs reaps the report after this
        ImGui::EndChild();
        return;
    }
    hint("Dismiss");

    ImGui::SetCursorPos(ImVec2(left_x, top_y));
    ImGui::PushTextWrapPos(left_x + text_w);
    ImGui::TextWrapped("%s: %s%stook %s", stopped ? "Stopped" : "Finished",
                       batch_counts(s).c_str(), app::kFactSeparator,
                       format_duration(s.elapsed_s).c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, text2);
    if (stopped) {
        ImGui::TextWrapped("Every result finished before Stop is kept.");
    } else if (building) {
        ImGui::TextWrapped("Building the path report...");
    } else if (report_failed) {
        ImGui::TextColored(kWarningColor, "%s", app::kPathReportFailed);
        ImGui::TextWrapped("%s", app.path_report.message.c_str());
        error_detail_line(app.path_report.error);
    } else if (report_ok) {
        ImGui::TextWrapped("%s", app::kPathReportReady);
    }
    ImGui::PopStyleColor();
    ImGui::PopTextWrapPos();

    // A setting with no other home, so it outlasts the rows a close lets go
    // (D103 item 28's 2026-10-09 answer).
    if (app.report_started && !building && !report_failed) {
        if (ImGui::Checkbox("Open automatically", &app.settings.auto_open_report))
            app.commit_settings();
        hint("Open the path report as soon as it's built.");
    }
    // A run that failed as a whole shows its error here, and counts no chart.
    if (!s.run_error.empty()) {
        ImGui::TextColored(kWarningColor, "%s", s.run_error.c_str());
        error_detail_line(s.run_error_detail);
    }
    if (s.failed > 0) {
        const std::string head = counted(s.failed, "chart", "charts") + " failed##batchfailures";
        if (ImGui::TreeNode(head.c_str())) {
            // Wrapped: a chart's name and its error can both run long.
            for (size_t i = 0; i < s.failures.size(); ++i) {
                ImGui::TextWrapped("%s", s.failures[i].c_str());
                if (i < s.failure_details.size()) error_detail_line(s.failure_details[i]);
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
}

// The "Compare dmleaderboards user" modal: fetch the ladder and let the user
// pick a player (filter-as-you-type). Picking closes the box and opens the
// comparison window, which builds the comparison (D103 item 8). The ladder
// fetch can cold-start the render.com backend, so it shows a "still waking
// up" note and a Cancel that abandons the job.
void render_dm_picker_modal(AppState& app) {
    // AppState::reopen_dm_picker asks; the popup opens from this window.
    if (app.dm_picker_popup_pending) {
        app.dm_picker_popup_pending = false;
        ImGui::OpenPopup("Compare dmleaderboards user");
    }
    if (!ImGui::BeginPopupModal("Compare dmleaderboards user", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize)) {
        return;
    }
    const bool escape = ImGui::IsKeyPressed(ImGuiKey_Escape, false);

    // Stage 0: still loading the user list.
    if (app.dm_fetch_job && !app.dm_fetch_job->finished()) {
        ImGui::TextUnformatted("Loading the dmleaderboards user list...");
        ImGui::TextDisabled("%s", app::kLeaderboardWaking);
        if (ImGui::Button("Cancel") || escape) {
            app.cancel_dm_fetch();  // parked, never joined here
            app.dm_picker_open = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
        return;
    }
    if (app.dm_fetch_job && app.dm_fetch_job->finished()) {
        if (!app.dm_fetch_job->ok()) {
            ImGui::TextColored(kWarningColor, "Could not load the user list:");
            ImGui::TextWrapped("%s", app.dm_fetch_job->message().c_str());
            error_detail_line(app.dm_fetch_job->error());
            ImGui::Spacing();
            if (ImGui::Button("Retry")) app.start_dm_fetch();
            ImGui::SameLine();
            if (ImGui::Button("Close") || escape) {
                app.dm_fetch_job.reset();
                app.dm_picker_open = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            return;
        }
        // Take ownership of the fetched list once, then let the job go.
        if (app.dm_users.empty() && !app.dm_fetch_job->users().empty())
            app.dm_users = std::move(app.dm_fetch_job->users());
        app.dm_fetch_job.reset();
    }

    // Stage 1: choose a player.
    ImGui::TextUnformatted("Pick a player to compare against your library:");
    char (&filter)[128] = app.library_ui.dm_filter;
    ImGui::SetNextItemWidth(px(380));
    ImGui::InputTextWithHint("##dmfilter", "filter by name", filter, sizeof(filter));

    const std::string needle = to_lower_ascii(filter);

    ImGui::BeginChild("dmusers", ImVec2(px(480), px(320)), ImGuiChildFlags_Borders);
    if (app.dm_users.empty()) {
        ImGui::TextDisabled("(No users found.)");
    } else {
        for (const net::DmUser& u : app.dm_users) {
            if (!needle.empty() &&
                to_lower_ascii(u.username).find(needle) == std::string::npos)
                continue;
            char label[256];
            const std::string scores = counted(u.total_scores, "score", "scores");
            if (u.elo)
                std::snprintf(label, sizeof(label), "%s  (%s, elo %d)###%s", u.username.c_str(),
                              scores.c_str(), *u.elo, u.id.c_str());
            else
                std::snprintf(label, sizeof(label), "%s  (%s)###%s", u.username.c_str(),
                              scores.c_str(), u.id.c_str());
            bool is_last = u.id == app.settings.dm_last_user;
            if (ImGui::Selectable(label, is_last)) {
                app.start_dm_report(u.id, u.username);
                app.dm_picker_open = false;
                ImGui::CloseCurrentPopup();
            }
        }
    }
    ImGui::EndChild();

    if (ImGui::Button("Close") || (escape && !ImGui::GetIO().WantTextInput)) {
        app.dm_picker_open = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

}  // namespace detail

}  // namespace hydra::ui
