#include "ui/details_parts.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "app/preview_view.h"  // path_overlay_key
#include "core/model.h"
#include "imgui.h"
#include "render/overlay_layout.h"  // ellipsize
#include "ui/preview_controller.h"
#include "ui/preview_panel.h"
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <optional>
#include <string>

namespace hydra::ui::detail {

namespace {

// "Showing" and the ##previewpath list: the same paths, in the same order and
// from the same cache, as the Paths tab's buttons. A pick sets the one
// selection both tabs read (DetailsViewState::selected_path). Drawn only when
// the song has a Ready record with paths. A held row (draw_held_row).
void render_path_picker(AppState& app) {
    HeldRow& row = app.details_ui.path_picker;
    if (!app.viewed.ready() || app.viewed.record->paths.empty()) {
        draw_held_row(app, row);
        return;
    }
    draw_held_row(app, row, [&] {
        const hydra::app::PathButtonsView& list = app.details_ui.paths_tab.buttons(
            *app.viewed.record, app.record_generation.n, app.settings.depth_mode,
            app.settings.depth_value);
        const Path*& selected = app.details_ui.selected_path;
        std::string current;
        for (const hydra::app::PathButtonView& b : list.buttons)
            if (b.path == selected) current = hydra::app::preview_path_label(b);

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Showing");
        ImGui::SameLine();
        ScopedFont notation(TextRole::Notation);
        // On its own line, as wide as the longest path in the list (or the
        // line). A path longer than the line (40 activations and "  (optimal)"
        // run to about 100 characters) ends in "…" rather than vanish under
        // the arrow, with the whole path a hover away; the open list shows it
        // whole.
        float widest = 0.0f;
        for (const hydra::app::PathButtonView& b : list.buttons)
            widest = std::max(
                widest,
                text_size(TextRole::Notation, hydra::app::preview_path_label(b).c_str()).x);
        const float chrome = ImGui::GetStyle().FramePadding.x * 2.0f + ImGui::GetFrameHeight();
        const float box_w = std::min(widest + chrome, ImGui::GetContentRegionAvail().x);
        const std::string shown = render::ellipsize(current, box_w - chrome, text_width);
        ImGui::SetNextItemWidth(box_w);
        const bool open = ImGui::BeginCombo("##previewpath", shown.c_str());
        if (!open && shown != current) overflow_tooltip(current.c_str());
        if (open) {
            for (size_t i = 0; i < list.buttons.size(); ++i) {
                const hydra::app::PathButtonView& b = list.buttons[i];
                const std::string item =
                    hydra::app::path_item_id(hydra::app::preview_path_label(b), i);
                const bool is_selected = b.path == selected;
                if (ImGui::Selectable(item.c_str(), is_selected)) selected = b.path;
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    });
}

}  // namespace

// The Preview tab: a transport row over the 3D note highway. Reached only while
// the tab is shown, so the controller (and its decode + GPU work) spins up lazily
// on first view, per the "render only while active" gating.
void render_preview_panel(AppState& app, const Path* selected_path) {
    PreviewController* pc = app.preview_controller();
    if (pc == nullptr) {
        ImGui::TextUnformatted("Preview is unavailable (no render device).");
        return;
    }
    if (!app.selected) {
        ImGui::TextUnformatted("Select a song to preview.");
        return;
    }

    // Open (or keep open) for the current selection; a no-op once running for
    // this chart. This is where the async decode starts.
    pc->set_volume(app.settings.preview_volume);  // before the audio exists too
    // The meter's ceiling is the Settings cap, Ready record or not. The
    // profile owns a record's cap (finding 130): the viewed record is fetched
    // under the Settings cap's profile, and prepare_record refuses an
    // analysis whose cap differs from its profile's. A chart with no record
    // yet previews at the cap the next analysis will run at (D48, Q24).
    const int sp_cap = app.settings.sp_cap;
    // The overlay key is the path's verbose string: rebuilt when the
    // selection or the record changes, not every frame.
    DetailsViewState& ui = app.details_ui;
    if (selected_path != ui.overlay_key_path ||
        app.record_generation.n != ui.overlay_key_generation) {
        ui.overlay_key = hydra::app::path_overlay_key(selected_path);
        ui.overlay_key_path = selected_path;
        ui.overlay_key_generation = app.record_generation.n;
    }
    pc->open(*app.selected, app.settings.reading(), fill_rule_for(app.settings.legacy_fills),
             selected_path, ui.overlay_key, sp_cap, app.settings.rules);
    pc->poll();

    if (pc->has_error()) {
        // Wrapped: an error naming a file path runs far past the panel's edge.
        {
            WarnColor warn;
            ImGui::TextWrapped("Preview failed: %s", pc->error().c_str());
        }
        error_detail_line(pc->error_detail());
        return;
    }
    if (pc->loading()) {
        // A big chart can take a moment; the step label and bar are what
        // tell the user it is still moving, and the time left (when known)
        // how long it will be.
        PreviewController::LoadProgress lp = pc->load_progress();
        if (lp.detail.empty())
            ImGui::Text("Loading preview: %s", lp.label.c_str());
        else
            ImGui::Text("Loading preview: %s, %s", lp.label.c_str(), lp.detail.c_str());
        progress_bar_percent(lp.fraction);
        return;
    }

    // The shared Preview panel (ui/preview_panel.h). Hydra's own rows go over
    // its transport: the chart-changed line and the path picker.
    PreviewPanelState panel;
    panel.volume_pct = app.settings.preview_volume;
    draw_preview_panel(*pc, panel, [&] {
        // The chart file changed since its record was analyzed: the highway
        // draws the new notes with no path over them, and one line says why
        // (D51 call 18; the words are D94's).
        if (pc->chart_changed()) {
            WarnColor warn;
            ImGui::TextWrapped(
                "This chart changed since it was analyzed. Click the song again to see its path.");
        }

        // "Show in Preview" on the Paths tab: once the overlay for the selected
        // path is in, move the playhead to that activation.
        std::optional<size_t>& jump = app.details_ui.paths_tab.ui().preview_jump;
        if (jump && pc->shows_path(ui.overlay_key)) {
            pc->seek_activation(*jump);
            jump.reset();
        }

        render_path_picker(app);
    });

    // Volume: applied live and remembered in the settings file, which is
    // written when the slider is let go.
    using hydra::app::Settings;
    if (panel.volume_changed) {
        app.settings.preview_volume = Settings::clamp(&Settings::preview_volume, panel.volume_pct);
        pc->set_volume(app.settings.preview_volume);
    }
    if (panel.volume_committed) app.commit_settings();
}

}  // namespace hydra::ui::detail
