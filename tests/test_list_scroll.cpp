// Tests for ui/list_scroll: whether a selection change scrolls the list, and
// how far (plan docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md,
// Change 2, the user's answers 4 and 5).

#include "doctest.h"

#include "ui/list_scroll.h"

using hydra::ui::kNearestEdgeScrollFlags;
using hydra::ui::ScrollTo;
using hydra::ui::selection_scroll;
using hydra::ui::SelectionSource;

TEST_CASE("a row click never scrolls the list") {
    CHECK(selection_scroll(SelectionSource::Click) == ScrollTo::None);
}

TEST_CASE("Previous, Next, Up and Down bring the row in at its nearest edge (answer 4)") {
    CHECK(selection_scroll(SelectionSource::Step) == ScrollTo::NearestEdge);
}

TEST_CASE("a selection made elsewhere scrolls like a step (answer 5)") {
    CHECK(selection_scroll(SelectionSource::Elsewhere) == ScrollTo::NearestEdge);
}

TEST_CASE("the nearest-edge scroll passes ImGui these flags") {
    // 66 is ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_NoScrollParent
    // in the vendored ImGui, read once from third_party/imgui/imgui_internal.h.
    CHECK(kNearestEdgeScrollFlags == 66);
}
