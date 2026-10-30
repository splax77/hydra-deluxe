#pragma once

// Whether a list scrolls, and how far: when the selected row changes, and
// when every row changes height under a selected row. The one owner of that
// question for every list in the app (plan
// docs/superpowers/plans/2026-10-14-column-shares-scroll-left-align.md,
// Change 2, the user's answers 4 and 5). selection_scroll and
// row_keeps_place decide; scroll_last_row and keep_last_row_in_place are the
// only places that ask ImGui to scroll.
//
// A table that moves its view to the selected row calls, right after that
// row's Selectable:
//
//   scroll_last_row(selection_scroll(source));

#include <optional>
#include <string>

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

// The ImGui scroll flags scroll_last_row passes for NearestEdge. Exposed so a
// test can pin them.
extern const int kNearestEdgeScrollFlags;

// Where a list's selected row sat on screen the last frame the list was
// drawn. A list whose rows all change height at once (the Library's, when the
// song panel opens or closes) keeps the same scroll offset in pixels, which
// would carry the selected song away from under the pointer; this is what it
// needs to put the song back.
struct RowPlace {
    float row_h = 0.0f;        // the height the list gave every row
    std::string row;           // the caller's key for the selected row; empty for none
    std::optional<float> top;  // that row's top below the list window's top;
                               // none when it was not on screen
};

// Whether the selected row, keyed `row`, goes back to where `last` saw it now
// that the list's rows are `row_h` tall. When it does, the list draws that row
// this frame even off screen, and calls keep_last_row_in_place(last) right
// after its Selectable. A row that was off screen stays off screen.
bool row_keeps_place(const RowPlace& last, float row_h, const std::string& row);

// Scrolls the list so the item ImGui drew last sits where `last` saw it.
// ImGui applies the scroll on the next frame.
void keep_last_row_in_place(const RowPlace& last);

// Where the item ImGui drew last sits, as the place of row `row` in a list
// whose rows are `row_h` tall.
RowPlace place_of_last_row(float row_h, const std::string& row);

}  // namespace hydra::ui
