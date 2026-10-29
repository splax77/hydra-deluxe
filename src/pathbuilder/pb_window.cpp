#include "pathbuilder/pb_window.h"

#include <cstdint>
#include <exception>

#include "imgui.h"

#include "app/display_format.h"  // clock_str
#include "app/preview_view.h"    // path_overlay_key
#include "app/user_messages.h"
#include "pathbuilder/pb_menu.h"
#include "pathbuilder/pb_open.h"
#include "search/graph.h"  // fill_rule_for
#include "ui/preview_controller.h"
#include "ui/widgets.h"  // WarnColor

namespace hydra::pathbuilder {

namespace {

// The share of the window's width each side column takes.
constexpr float kSideColumnShare = 0.22f;

// What the window says when the file dialog never appeared.
constexpr const char* kDialogFailedText =
    "The file dialog didn't open. Drag a chart file onto this window instead.";

}  // namespace

PbApp::PbApp(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (device && context) preview_ = std::make_unique<ui::PreviewController>(device, context);
}

PbApp::~PbApp() { close_preview(); }

void PbApp::open(const std::string& notespath) { state.open(notespath); }

bool PbApp::busy() const { return state.busy() || (preview_ && preview_->busy()); }

void PbApp::close_preview() {
    if (preview_) preview_->close();
}

const Path* PbApp::shown_path() const {
    if (list_ui_.picked && *list_ui_.picked < rows_.size()) return rows_[*list_ui_.picked].path;
    const std::optional<Path>& mine = state.history().current().path;
    return mine ? &*mine : nullptr;
}

void PbApp::take_in_chart() {
    state.poll();
    const OpenChart* chart = state.chart();
    if (chart == rows_chart_) return;
    rows_chart_ = chart;
    rows_ = chart ? path_rows(chart->record, {}) : std::vector<PathRow>{};
    list_ui_ = PathListUi{};
    overlay_built_ = false;
}

void PbApp::act_on_menu() {
    const PathHistory& h = state.history();
    switch (draw_menu(state.chart() != nullptr && h.can_undo(),
                      state.chart() != nullptr && h.can_redo())) {
        case MenuChoice::Open: {
            bool failed = false;
            if (const std::optional<std::string> path = ask_for_chart(owner, &failed)) open(*path);
            dialog_failed_ = failed;
            break;
        }
        case MenuChoice::Undo:
            state.history().undo();
            list_ui_.picked.reset();
            break;
        case MenuChoice::Redo:
            state.history().redo();
            list_ui_.picked.reset();
            break;
        case MenuChoice::None:
            break;
    }
}

void PbApp::draw() {
    take_in_chart();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_MenuBar |
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(kMainWindowName, nullptr, kFlags);
    act_on_menu();

    if (dialog_failed_) {
        ui::WarnColor warn;
        ImGui::TextWrapped("%s", kDialogFailedText);
    }
    if (state.opening()) ImGui::TextWrapped("Opening %s ...", state.requested_path().c_str());
    if (!state.error().empty()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", state.error().c_str());
        }
        if (!state.error_detail().empty()) ImGui::TextDisabled("%s", state.error_detail().c_str());
    }

    if (const OpenChart* chart = state.chart())
        draw_body(*chart);
    else if (!state.opening() && state.error().empty())
        ImGui::TextWrapped("Open a chart with File > Open, or drag one onto this window.");
    ImGui::End();
}

void PbApp::draw_body(const OpenChart& chart) {
    const float side = ImGui::GetContentRegionAvail().x * kSideColumnShare;

    ImGui::BeginChild("##paths", ImVec2(side, 0), ImGuiChildFlags_Borders);
    ImGui::TextWrapped("%s", chart.chart_path.c_str());
    ImGui::Separator();
    if (!chart.has_star_power()) {
        ImGui::TextWrapped("%s", kNothingToBuildText);
    } else if (const std::optional<size_t> edit = draw_path_list(rows_, list_ui_)) {
        try {
            state.history().edit(player_copy_of(*rows_[*edit].path));
            list_ui_.picked.reset();
        } catch (const std::exception& e) {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", app::plain_error(e).c_str());
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("##highway", ImVec2(ImGui::GetContentRegionAvail().x - side, 0));
    draw_highway(chart, shown_path());
    ImGui::EndChild();

    ImGui::SameLine();
    // The decision list's column; its rows come with the cost ladder.
    ImGui::BeginChild("##decisions", ImVec2(0, 0), ImGuiChildFlags_Borders);
    if (!chart.has_star_power()) ImGui::TextWrapped("%s", kNothingToBuildText);
    ImGui::EndChild();
}

void PbApp::draw_highway(const OpenChart& chart, const Path* shown) {
    if (!preview_) return;
    if (!overlay_built_ || shown != overlay_for_) {
        overlay_key_ = app::path_overlay_key(shown);
        overlay_for_ = shown;
        overlay_built_ = true;
    }
    const app::AnalysisSettings& s = chart.settings;
    preview_->open(chart.entry, s.reading(), fill_rule_for(s.legacy_fills), shown, overlay_key_,
                   s.sp_cap, s.rules);
    preview_->poll();

    if (preview_->has_error()) {
        {
            ui::WarnColor warn;
            ImGui::TextWrapped("%s", preview_->error().c_str());
        }
        if (!preview_->error_detail().empty())
            ImGui::TextDisabled("%s", preview_->error_detail().c_str());
        return;
    }
    if (preview_->loading()) {
        const ui::PreviewController::LoadProgress p = preview_->load_progress();
        ImGui::ProgressBar(p.fraction, ImVec2(-1, 0), p.label.c_str());
        return;
    }

    if (ImGui::Button(preview_->playing() ? "Pause" : "Play")) preview_->toggle();
    ImGui::SameLine();
    ImGui::Text("%s", app::clock_str(preview_->position_ms()).c_str());
    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false))
        preview_->toggle();

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const int w = static_cast<int>(avail.x);
    const int h = static_cast<int>(avail.y);
    if (w <= 0 || h <= 0) return;
    if (ID3D11ShaderResourceView* srv = preview_->render(w, h))
        ImGui::Image((ImTextureID)(intptr_t)srv, ImVec2(static_cast<float>(w), static_cast<float>(h)));
}

}  // namespace hydra::pathbuilder
