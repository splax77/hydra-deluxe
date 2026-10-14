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

}  // namespace hydra::ui
