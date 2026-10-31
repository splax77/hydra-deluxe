// The version bar across the top of the main window
// (D260): the app's name and version, then a dot and the update status, with
// the update phase's buttons when one shows. Every word comes from
// AppState::version_bar_status and AppState::update; this only draws them.

#include "ui/library_parts.h"

#include "core/app_version.h"  // named_version, running_version
#include "imgui.h"
#include "ui/fonts.h"  // px
#include "ui/text_role.h"
#include "ui/theme.h"
#include "ui/widgets.h"

#include <algorithm>
#include <string>

namespace hydra::ui::detail {

namespace {

const ImVec4& dot_color(VersionBarStatus::Dot dot) {
    switch (dot) {
        case VersionBarStatus::Dot::UpToDate: return kUpToDateDotColor;
        case VersionBarStatus::Dot::Available: return kUpdateAvailableDotColor;
        case VersionBarStatus::Dot::Neutral: break;
    }
    return kNeutralDotColor;
}

float dot_radius() { return px(5.0f); }
float dot_slot_width() { return 2.0f * dot_radius() + px(4.0f); }

// The status line's pieces sit one after another on the name's line. The
// first line's pieces start at `row_top`, so a button's label and a sentence
// share the name's baseline; a piece that no longer fits starts a new line.
class StatusRow {
public:
    explicit StatusRow(float row_top) : row_top_(row_top) {}

    // Places the next piece, `width` wide, after the last one.
    void next(float width) {
        ImGui::SameLine();
        if (!wrapped_ && ImGui::GetContentRegionAvail().x < width) {
            ImGui::NewLine();
            wrapped_ = true;
        }
        if (!wrapped_) ImGui::SetCursorPosY(row_top_);
    }
    // The next piece is glued to the last one: it never starts a new line.
    void glued() {
        ImGui::SameLine();
        if (!wrapped_) ImGui::SetCursorPosY(row_top_);
    }

private:
    float row_top_;
    bool wrapped_ = false;
};

float button_width(const char* label) {
    return ImGui::CalcTextSize(label, nullptr, true).x + 2.0f * ImGui::GetStyle().FramePadding.x;
}

// A sentence on the status line, in `color`, with its label's baseline.
void status_text(const std::string& text, const ImVec4& color) {
    ImGui::AlignTextToFramePadding();
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();
}

bool status_button(StatusRow& row, const char* label) {
    row.next(button_width(label));
    return ImGui::Button(label);
}

// The phase's buttons after its sentence (UpdateState::Phase says which phase
// offers which).
void phase_buttons(AppState& app, StatusRow& row) {
    using Phase = UpdateState::Phase;
    const UpdateState& update = app.update;
    const bool three_buttons = update.phase == Phase::Offered || update.phase == Phase::Declined;
    const bool failed = update.phase == Phase::Failed;
    if (three_buttons || (failed && update.release)) {
        if (status_button(row, "Update##update")) app.update_now();
    }
    if (three_buttons) {
        if (status_button(row, "Skip this version")) app.skip_version();
    }
    // A failed update can be put away too: Later hides it until the next start.
    if (three_buttons || failed) {
        if (status_button(row, "Later")) app.later();
    }
    if (update.phase == Phase::Downloading) {
        const std::string megabytes = app.update_megabytes_text();
        row.next(ImGui::CalcTextSize(megabytes.c_str()).x);
        status_text(megabytes, kDefaultTextColor);
        // The bar fills the line up to the Cancel button.
        const ImGuiStyle& style = ImGui::GetStyle();
        const float cancel_w = button_slot_width("Cancel");
        row.next(ImGui::GetFrameHeight() + style.ItemSpacing.x + cancel_w);
        const float bar_w = std::max(ImGui::GetContentRegionAvail().x - cancel_w - style.ItemSpacing.x,
                                     ImGui::GetFrameHeight());
        const double total = update.release ? static_cast<double>(update.release->installer_size) : 0.0;
        ImGui::ProgressBar(app::progress_fraction(static_cast<double>(app.update_received()), total),
                           ImVec2(bar_w, 0.0f), "");
        row.glued();
        if (ImGui::Button("Cancel##update")) app.cancel_update();
    }
    if (update.phase == Phase::Updated) {
        if (status_button(row, "OK##update")) app.dismiss_update_note();
    }
}

}  // namespace

void render_version_bar(AppState& app) {
    const VersionBarStatus status = app.version_bar_status();
    const UpdateState& update = app.update;
    const bool phase_shows = update.phase != UpdateState::Phase::None;

    ImGui::PushStyleColor(ImGuiCol_ChildBg, kVersionBarBg);
    ImGui::BeginChild("##versionbar", ImVec2(0.0f, 0.0f),
                      ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleColor();

    // The name's baseline and a framed label's baseline line up: whichever
    // sits lower sets the line, and the other starts that much further down.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float top = ImGui::GetCursorPosY();
    const float shift = baseline_shift(TextRole::Prose, TextRole::AppVersion);
    const float row_top = top + std::max(0.0f, shift - style.FramePadding.y);
    const float name_top = row_top + style.FramePadding.y - shift;

    // The name is text only: no click, no hover.
    ImGui::SetCursorPosY(name_top);
    {
        ScopedFont font(TextRole::AppVersion);
        ImGui::TextUnformatted(core::named_version(core::running_version()).c_str());
    }

    StatusRow row(row_top);
    // The dot and the sentence after it stay together.
    const char* const first = !status.text.empty() ? status.text.c_str() : status.dimmed.c_str();
    row.next(dot_slot_width() + style.ItemSpacing.x + ImGui::CalcTextSize(first).x);
    {
        const float r = dot_radius();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        const float centre_y = p.y + ImGui::GetFrameHeight() * 0.5f;
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(p.x + r, centre_y), r,
                                                    ImGui::ColorConvertFloat4ToU32(dot_color(status.dot)));
        ImGui::Dummy(ImVec2(dot_slot_width(), ImGui::GetFrameHeight()));
    }
    const app::UpdateRelease* const named = app.bar_release();
    if (!status.text.empty()) {
        row.glued();
        status_text(status.text, kDefaultTextColor);
        // The release's notes, as plain text: nothing in them is read as markup.
        if (named && !named->notes.empty() && ImGui::IsItemHovered())
            wrapped_tooltip(named->notes.c_str());
    }
    if (!status.dimmed.empty()) {
        if (status.text.empty())
            row.glued();
        else
            row.next(ImGui::CalcTextSize(status.dimmed.c_str()).x);
        status_text(status.dimmed, kDimTextColor);
    }

    if (phase_shows) {
        phase_buttons(app, row);
    } else if (status.update_button) {
        if (status_button(row, "Update##update")) app.update_now();
    }
    error_detail_line(update.detail);

    ImGui::EndChild();
    // The hairline between the bar and the toolbar. It is drawn straight on
    // the draw list, not with ImGui::Separator(), because Separator also
    // writes a row of dashes into ImGui's text log. It takes Separator's
    // thickness, colour and vertical space, so the toolbar sits where a
    // Separator would put it.
    const float thickness = std::max(style.SeparatorSize, 1.0f);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float end_x = start.x + ImGui::GetContentRegionAvail().x;
    const float line_y = start.y + thickness * 0.5f - 0.5f;
    ImGui::GetWindowDrawList()->AddLine(ImVec2(start.x, line_y), ImVec2(end_x, line_y),
                                        ImGui::GetColorU32(ImGuiCol_Separator), thickness);
    ImGui::Dummy(ImVec2(0.0f, thickness));
}

}  // namespace hydra::ui::detail
