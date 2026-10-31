// Tests for ui/list_scroll: whether a selection change or a change of row
// height scrolls the list, and how far (the user's ruling, D242).

#include "doctest.h"

#include "ui/list_scroll.h"

using hydra::ui::kNearestEdgeScrollFlags;
using hydra::ui::row_keeps_place;
using hydra::ui::RowPlace;
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

TEST_CASE("the selected row goes back to its place only when the row height changes under it") {
    RowPlace last;
    last.row_h = 20.0f;
    last.row = "songs/a/notes.mid";
    last.top = 300.0f;
    // New height, same row, on screen last frame: it goes back.
    CHECK(row_keeps_place(last, 42.0f, "songs/a/notes.mid"));
    // Same height: a click never scrolls, so nothing moves.
    CHECK_FALSE(row_keeps_place(last, 20.0f, "songs/a/notes.mid"));
    // A different selected row: the place belongs to another song.
    CHECK_FALSE(row_keeps_place(last, 42.0f, "songs/b/notes.mid"));
    // Nothing selected.
    CHECK_FALSE(row_keeps_place(last, 42.0f, ""));
    // Off screen last frame: the list stays where it is.
    last.top.reset();
    CHECK_FALSE(row_keeps_place(last, 42.0f, "songs/a/notes.mid"));
}

TEST_CASE("the nearest-edge scroll passes ImGui these flags") {
    // 66 is ImGuiScrollFlags_KeepVisibleEdgeY | ImGuiScrollFlags_NoScrollParent
    // in the vendored ImGui, read once from third_party/imgui/imgui_internal.h.
    CHECK(kNearestEdgeScrollFlags == 66);
}
