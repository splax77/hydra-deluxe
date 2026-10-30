#include "pathbuilder/pb_strip.h"

#include <algorithm>

#include "imgui.h"

#include "ui/fonts.h"  // px
#include "ui/theme.h"  // kStarPowerColor

namespace hydra::pathbuilder {

std::vector<int64_t> step_levels(const app::LadderRun& run) {
    std::vector<int64_t> out;
    if (run.ladder.bf.empty()) return out;
    const app::CostList costs = app::cost_list(run.ladder);
    out.push_back(run.ladder.bf.front());
    for (int64_t c : costs.windows) out.push_back(out.back() - c);
    if (run.complete) out.push_back(out.back() - costs.after_last);
    return out;
}

namespace {

// Where chart time `ms` falls on a strip `width` wide spanning 0 to `end_ms`.
float strip_x(double ms, double end_ms, float width) {
    if (end_ms <= 0.0) return 0.0f;
    return static_cast<float>(ms / end_ms) * width;
}

}  // namespace

std::vector<StripMarker> strip_markers(const app::PreviewScene& scene, double end_ms,
                                       float width, float min_w) {
    std::vector<StripMarker> out;
    for (size_t i = 0; i < scene.fills.size(); ++i) {
        const app::PreviewFill& f = scene.fills[i];
        if (f.state == app::PreviewFillState::Hidden) continue;
        StripMarker m;
        m.fill = i;
        m.state = f.state;
        m.x0 = strip_x(f.span.start_ms, end_ms, width);
        m.x1 = strip_x(f.span.end_ms, end_ms, width);
        if (m.x1 - m.x0 < min_w) {
            const float mid = (m.x0 + m.x1) * 0.5f;
            m.x0 = mid - min_w * 0.5f;
            m.x1 = mid + min_w * 0.5f;
        }
        out.push_back(m);
    }
    return out;
}

std::optional<size_t> marker_at(const std::vector<StripMarker>& markers, float x) {
    for (size_t i = markers.size(); i-- > 0;)
        if (x >= markers[i].x0 && x <= markers[i].x1) return markers[i].fill;
    return std::nullopt;
}

std::optional<size_t> draw_strip(const app::PreviewScene& scene, double end_ms,
                                 const app::LadderRun* run, float height) {
    const float width = ImGui::GetContentRegionAvail().x;
    if (width <= 0.0f || height <= 0.0f) return std::nullopt;
    const ImVec2 mn = ImGui::GetCursorScreenPos();
    const ImVec2 mx(mn.x + width, mn.y + height);
    ImGui::InvisibleButton(kStripId, ImVec2(width, height));
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(mn, mx, ImGui::GetColorU32(ImGuiCol_FrameBg));

    // The markers are sized as the Preview's scrub marks are (plan Q4): as
    // wide as one at least, and inset from the strip's edges as they are.
    const float inset = ui::px(2.0f);
    const std::vector<StripMarker> markers = strip_markers(scene, end_ms, width, ui::px(3.0f));
    for (const StripMarker& m : markers) {
        const ImU32 col = m.state == app::PreviewFillState::Taken
                              ? ImGui::GetColorU32(ui::kStarPowerColor)
                              : ImGui::GetColorU32(ImGuiCol_TextDisabled);
        dl->AddRectFilled(ImVec2(mn.x + m.x0, mn.y + inset), ImVec2(mn.x + m.x1, mx.y - inset),
                          col);
    }

    // The step line: each level held until the next window's activation,
    // the last one (the player's total) reaching the right edge.
    if (run) {
        const std::vector<int64_t> levels = step_levels(*run);
        if (!levels.empty()) {
            const int64_t top = levels.front();
            const int64_t bottom = *std::min_element(levels.begin(), levels.end());
            auto y_of = [&](int64_t v) {
                if (top == bottom) return (mn.y + mx.y) * 0.5f;
                const float f = static_cast<float>(top - v) / static_cast<float>(top - bottom);
                return mn.y + inset + f * (height - 2.0f * inset);
            };
            std::vector<float> step_x;
            for (size_t k = 1; k < levels.size(); ++k) {
                const size_t act = k - 1;
                step_x.push_back(act < scene.activations.size()
                                     ? mn.x + strip_x(scene.activations[act].ms, end_ms, width)
                                     : mx.x);
            }
            const ImU32 col = ImGui::GetColorU32(ImGuiCol_PlotLines);
            float x = mn.x;
            for (size_t k = 0; k < levels.size(); ++k) {
                const float next_x = k < step_x.size() ? step_x[k] : mx.x;
                const float y = y_of(levels[k]);
                dl->AddLine(ImVec2(x, y), ImVec2(next_x, y), col);
                if (k + 1 < levels.size())
                    dl->AddLine(ImVec2(next_x, y), ImVec2(next_x, y_of(levels[k + 1])), col);
                x = next_x;
            }
        }
    }

    if (!clicked) return std::nullopt;
    return marker_at(markers, ImGui::GetIO().MousePos.x - mn.x);
}

}  // namespace hydra::pathbuilder
