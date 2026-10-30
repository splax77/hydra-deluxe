#include "pathbuilder/pb_paths.h"

#include "imgui.h"

#include "app/path_view.h"  // kBestAllZeroLabel

namespace hydra::pathbuilder {

std::vector<PathRow> path_rows(const HydraRecord& record, const std::vector<Path>& branches) {
    std::vector<PathRow> rows;
    const std::optional<int64_t> best = record.optimal_score();
    if (!best) return rows;
    auto add = [&](std::string label, const Path& p) {
        PathRow r;
        r.label = std::move(label);
        r.path = &p;
        r.total = p.totalscore();
        r.text = p.pathstring();
        r.gap = *best - r.total;
        rows.push_back(std::move(r));
    };
    for (size_t i = 0; i < record.paths.size(); ++i)
        add(i == 0 ? "Best path" : "Path " + std::to_string(i + 1), record.paths[i]);
    for (size_t i = 0; i < record.allzero_paths.size(); ++i)
        add(i == 0 ? std::string(app::kBestAllZeroLabel) : "All-0 path " + std::to_string(i + 1),
            record.allzero_paths[i]);
    for (size_t i = 0; i < branches.size(); ++i)
        add("Your branch " + std::to_string(i + 1), branches[i]);
    return rows;
}

std::string total_text(const PathRow& row) { return group_thousands(row.total); }

std::string gap_text(const PathRow& row) {
    if (row.gap == 0) return "0";
    return row.gap > 0 ? "-" + group_thousands(row.gap) : "+" + group_thousands(-row.gap);
}

std::optional<size_t> draw_path_list(const std::vector<PathRow>& rows, PathListUi& ui) {
    if (ui.picked && *ui.picked >= rows.size()) ui.picked.reset();
    for (size_t i = 0; i < rows.size(); ++i) {
        const PathRow& r = rows[i];
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(r.label.c_str(), ui.picked == i)) ui.picked = i;
        ImGui::Indent();
        ImGui::Text("%s  (%s)", total_text(r).c_str(), gap_text(r).c_str());
        ImGui::TextDisabled("%s", r.text.c_str());
        ImGui::Unindent();
        ImGui::PopID();
    }
    std::optional<size_t> edit;
    ImGui::Spacing();
    ImGui::BeginDisabled(!ui.picked.has_value());
    if (ImGui::Button("Edit a copy")) edit = ui.picked;
    ImGui::EndDisabled();
    return edit;
}

}  // namespace hydra::pathbuilder
