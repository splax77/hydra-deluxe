#include "ui/list_scroll.h"

#include "imgui.h"
#include "imgui_internal.h"  // ScrollToItem and ImGuiScrollFlags_ (work-in-progress API)

namespace hydra::ui {

// Keep the item in view at the nearest edge, and scroll only the list's own
// window, never the pane or report window around it.
const int kNearestEdgeScrollFlags = ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_NoScrollParent;

ScrollTo selection_scroll(SelectionSource source) {
    switch (source) {
        case SelectionSource::Click:
            // The row is under the pointer already; moving the list would put
            // a different song under the next click at the same spot.
            return ScrollTo::None;
        case SelectionSource::Step:       // answer 4
        case SelectionSource::Elsewhere:  // answer 5
            return ScrollTo::NearestEdge;
    }
    return ScrollTo::None;
}

void scroll_last_row(ScrollTo how) {
    switch (how) {
        case ScrollTo::None:
            return;
        case ScrollTo::NearestEdge:
            // ScrollToItem is marked work-in-progress in imgui_internal.h; if
            // ImGui renames it, this is the one line to change.
            ImGui::ScrollToItem(kNearestEdgeScrollFlags);
            return;
    }
}

bool row_keeps_place(const RowPlace& last, float row_h, const std::string& row) {
    // A click never moves the song under the pointer (selection_scroll), and
    // a change of row height alone would.
    return row_h != last.row_h && !row.empty() && row == last.row && last.top.has_value();
}

// The row's top is measured from the list window's top, the same way in
// place_of_last_row and here, so whatever sits above the rows (a frozen
// header) cancels out.
void keep_last_row_in_place(const RowPlace& last) {
    if (!last.top) return;
    const float top_now = ImGui::GetItemRectMin().y - ImGui::GetWindowPos().y;
    ImGui::SetScrollY(ImGui::GetScrollY() + top_now - *last.top);
}

RowPlace place_of_last_row(float row_h, const std::string& row) {
    RowPlace place{row_h, row, std::nullopt};
    if (ImGui::IsItemVisible()) place.top = ImGui::GetItemRectMin().y - ImGui::GetWindowPos().y;
    return place;
}

}  // namespace hydra::ui
