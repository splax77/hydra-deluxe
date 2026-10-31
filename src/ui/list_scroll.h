#pragma once

// Whether a change of the selected row scrolls a list, and how far. The one
// owner of that question for every list in the app (plan
// docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md,
// Change 2, the user's answers 4 and 5). selection_scroll decides;
// scroll_last_row is the one place that asks ImGui to scroll.
//
// A table that moves its view to the selected row calls, right after that
// row's Selectable:
//
//   scroll_last_row(selection_scroll(source));

namespace hydra::ui {

// Why the selection changed.
enum class SelectionSource {
    Click,      // the user clicked the row itself
    Step,       // Previous / Next in the song panel, or the Up / Down keys
    Elsewhere,  // picked from another window, such as a report row's click
};

// How a list moves to show the newly selected row.
enum class ScrollTo {
    None,         // the list stays where it is
    NearestEdge,  // the row is brought fully into view, moving the list as
                  // little as it can; a row already on screen moves nothing
};

ScrollTo selection_scroll(SelectionSource source);

// Applies `how` to the item ImGui drew last (the row's Selectable).
void scroll_last_row(ScrollTo how);

// Brings a block of the current window fully into view, moving it as little
// as it can (the top first when the block is taller than the window). It is
// for a row that opens into more than its Selectable, such as a step that
// shows its sentence beneath it; the rectangle is in screen coordinates.
// `how` is selection_scroll's answer, or NearestEdge when the opening itself
// is what asks for the scroll.
void scroll_block_into_view(ScrollTo how, float left, float top, float right, float bottom);

// The ImGui scroll flags scroll_last_row passes for NearestEdge. Exposed so a
// test can pin them.
extern const int kNearestEdgeScrollFlags;

}  // namespace hydra::ui
