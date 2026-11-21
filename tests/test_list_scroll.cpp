// Tests for ui/list_scroll: whether a selection change or a change of row
// height scrolls a list, and how far. Each test draws a real ImGui list on a
// headless ImGui, one row per corpus chart keyed the way the Library keys its
// rows, and reads where the rows and the scroll offsets end up.

#include "doctest.h"

#include "ui/library_model.h"  // row_key
#include "ui/list_scroll.h"

#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"  // GetCurrentWindow, to read the list's visible area

#include "corpus_util.h"  // chart_paths, scanned_entry
#include "headless_imgui_fixture.h"

using hydra::ui::keep_last_row_in_place;
using hydra::ui::place_of_last_row;
using hydra::ui::row_keeps_place;
using hydra::ui::RowPlace;
using hydra::ui::scroll_last_row;
using hydra::ui::selection_scroll;
using hydra::ui::SelectionSource;

namespace {

// Every corpus chart, as the key the Library gives its row (row_key of the
// row a scan writes). Hashed once per run.
const std::vector<std::string>& corpus_row_keys() {
    static const std::vector<std::string> keys = [] {
        std::vector<std::string> out;
        for (const std::string& path : corpus::chart_paths())
            out.push_back(hydra::ui::row_key(corpus::scanned_entry(path)));
        return out;
    }();
    return keys;
}

// The two row heights a list draws: one text line, or two, as the Library's
// rows grow when the song panel hides a column.
enum class RowHeight { OneLine, TwoLines };

float height_of(RowHeight h) {
    const float line = ImGui::GetTextLineHeight();
    return h == RowHeight::OneLine ? line : line * 2.0f;
}

// What one frame of the list showed, in screen pixels.
struct ListFrame {
    float list_scroll = 0.0f;       // the list's own scroll offset
    float parent_scroll = 0.0f;     // the scroll offset of the window around it
    float view_top = 0.0f;          // the list's visible area
    float view_bottom = 0.0f;
    std::vector<float> row_top;     // each row's top and bottom
    std::vector<float> row_bottom;
    std::vector<bool> row_shown;    // whether ImGui showed the row at all
    RowPlace place;                 // place_of_last_row for the selected row
};

// One real ImGui list of Selectables in a child window, the way the Library
// draws its rows. The window around it scrolls, and its view ends part-way
// down the list, so a scroll that also moved the parent would show.
class RealList {
public:
    // ImGui lays a new window out on its first frame, so the list draws one
    // frame, with nothing selected, before a test reads it.
    explicit RealList(std::vector<std::string> keys) : keys_(std::move(keys)) {
        draw(RowHeight::OneLine, std::string(), std::nullopt);
    }

    const std::string& key(std::size_t k) const { return keys_[k]; }

    // Draws one frame with rows `height` tall and the row keyed `selected`
    // selected. A set `source` means the selection changed this frame: the
    // selected row asks selection_scroll how to move to it, as
    // library_table.cpp does. When row_keeps_place says the row goes back to
    // last frame's place, it asks keep_last_row_in_place to put it there.
    ListFrame draw(RowHeight height, const std::string& selected, std::optional<SelectionSource> source) {
        ImGui::NewFrame();
        const float row_h = height_of(height);
        const bool keep = row_keeps_place(last_, row_h, selected);
        ListFrame f;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f));
        ImGui::Begin("List scroll test", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoSavedSettings);
        // Space above the list, so its bottom sits below this window's view.
        ImGui::Dummy(ImVec2(10.0f, 150.0f));
        ImGui::BeginChild("##rows", ImVec2(0.0f, 200.0f));
        const ImGuiWindow* list = ImGui::GetCurrentWindow();
        f.view_top = list->InnerRect.Min.y;
        f.view_bottom = list->InnerRect.Max.y;
        RowPlace place{row_h, std::string(), std::nullopt};
        for (std::size_t k = 0; k < keys_.size(); ++k) {
            ImGui::PushID(static_cast<int>(k));
            const bool is_selected = keys_[k] == selected;
            ImGui::Selectable("##row", is_selected, ImGuiSelectableFlags_None, ImVec2(0.0f, row_h));
            if (is_selected) {
                if (source) scroll_last_row(selection_scroll(*source));
                if (keep) keep_last_row_in_place(last_);
                place = place_of_last_row(row_h, keys_[k]);
            }
            f.row_top.push_back(ImGui::GetItemRectMin().y);
            f.row_bottom.push_back(ImGui::GetItemRectMax().y);
            f.row_shown.push_back(ImGui::IsItemVisible());
            ImGui::PopID();
        }
        f.list_scroll = ImGui::GetScrollY();
        ImGui::EndChild();
        // More below the list, so the window around it has room to scroll.
        ImGui::Dummy(ImVec2(10.0f, 400.0f));
        f.parent_scroll = ImGui::GetScrollY();
        ImGui::End();
        ImGui::EndFrame();
        last_ = place;
        f.place = place;
        return f;
    }

private:
    std::vector<std::string> keys_;
    RowPlace last_;
};

// Whether row `k` lies wholly inside the list's view. ImGui's own test, in
// ScrollToRectEx, lets a row reach one pixel past either edge.
bool fully_in_view(const ListFrame& f, std::size_t k) {
    return f.row_top[k] >= f.view_top - 1.0f && f.row_bottom[k] <= f.view_bottom + 1.0f;
}

// A row well below the list's view: twice as far down as the first row
// wholly out of view, with a row after it.
std::size_t far_row(const ListFrame& f) {
    std::size_t hidden = 0;
    while (hidden < f.row_top.size() && f.row_top[hidden] < f.view_bottom) ++hidden;
    REQUIRE(hidden > 0);
    REQUIRE(2 * hidden + 1 < f.row_top.size());
    return 2 * hidden;
}

// The list one frame after `source` selects row `k`, when ImGui has applied
// the scroll the selection asked for.
ListFrame select_row(RealList& list, std::size_t k, SelectionSource source) {
    list.draw(RowHeight::OneLine, list.key(k), source);
    return list.draw(RowHeight::OneLine, list.key(k), std::nullopt);
}

// A list before and after `source` selects its far row.
struct Moved {
    ListFrame before;
    ListFrame after;
    std::size_t row = 0;
};

Moved move_to_far_row(RealList& list, SelectionSource source) {
    Moved m;
    m.before = list.draw(RowHeight::OneLine, std::string(), std::nullopt);
    m.row = far_row(m.before);
    m.after = select_row(list, m.row, source);
    return m;
}

// The same, on a fresh ImGui and a fresh list.
Moved moved_in_fresh_list(SelectionSource source) {
    const hydra::test::HeadlessImGui imgui;
    RealList list(corpus_row_keys());
    return move_to_far_row(list, source);
}

}  // namespace

TEST_CASE("a row click never scrolls the list, even to a row out of view") {
    const Moved click = moved_in_fresh_list(SelectionSource::Click);
    CHECK(click.after.list_scroll == click.before.list_scroll);
    CHECK(click.after.parent_scroll == click.before.parent_scroll);
    CHECK_FALSE(fully_in_view(click.after, click.row));
}

TEST_CASE("Previous, Next, Up and Down bring the row in at its nearest edge and move only the list") {
    const hydra::test::HeadlessImGui imgui;
    RealList list(corpus_row_keys());
    const Moved down = move_to_far_row(list, SelectionSource::Step);
    // The row below the view comes in at the bottom edge: wholly in view,
    // with the row after it still cut off, so the list moved no further than
    // it had to. The window around the list did not move.
    CHECK(fully_in_view(down.after, down.row));
    CHECK_FALSE(fully_in_view(down.after, down.row + 1));
    CHECK(down.after.parent_scroll == down.before.parent_scroll);

    // A step to a row already in view moves nothing.
    REQUIRE(fully_in_view(down.after, down.row - 1));
    const ListFrame stay = select_row(list, down.row - 1, SelectionSource::Step);
    CHECK(stay.list_scroll == down.after.list_scroll);

    // A step to a row above the view brings it in at the top edge.
    REQUIRE_FALSE(fully_in_view(stay, 1));
    const ListFrame up = select_row(list, 1, SelectionSource::Step);
    CHECK(fully_in_view(up, 1));
    CHECK_FALSE(fully_in_view(up, 0));
    CHECK(up.parent_scroll == down.before.parent_scroll);
}

TEST_CASE("a selection made elsewhere moves the list the way a step does") {
    const Moved step = moved_in_fresh_list(SelectionSource::Step);
    const Moved elsewhere = moved_in_fresh_list(SelectionSource::Elsewhere);
    REQUIRE(step.after.list_scroll != step.before.list_scroll);
    CHECK(elsewhere.row == step.row);
    CHECK(elsewhere.after.list_scroll == step.after.list_scroll);
    CHECK(elsewhere.after.parent_scroll == step.after.parent_scroll);
}

namespace {

// The lowest row ImGui shows in `f`, one row up from the last one it shows
// at all, so the whole row is on screen.
std::size_t shown_row(const ListFrame& f) {
    std::size_t last = 0;
    for (std::size_t k = 0; k < f.row_shown.size(); ++k)
        if (f.row_shown[k]) last = k;
    REQUIRE(last > 1);
    return last - 1;
}

// A list whose rows are one line tall, then two, with `before` selected at
// one line and `after` at two. Returns the frame at one line and the frame
// after the change, once ImGui has applied any scroll the change asked for.
struct HeightChange {
    ListFrame one_line;
    ListFrame two_lines;
};

HeightChange change_height(RealList& list, const std::string& before, const std::string& after) {
    HeightChange c;
    c.one_line = list.draw(RowHeight::OneLine, before, std::nullopt);
    list.draw(RowHeight::TwoLines, after, std::nullopt);
    c.two_lines = list.draw(RowHeight::TwoLines, after, std::nullopt);
    return c;
}

}  // namespace

TEST_CASE("the selected row goes back to its place only when the row height changes under it") {
    const hydra::test::HeadlessImGui imgui;
    RealList list(corpus_row_keys());
    const ListFrame first = list.draw(RowHeight::OneLine, std::string(), std::nullopt);
    const std::size_t k = shown_row(first);
    const std::string& row = list.key(k);

    SUBCASE("taller rows under a row on screen: it stays where it was") {
        const HeightChange c = change_height(list, row, row);
        REQUIRE(c.one_line.place.top.has_value());
        REQUIRE(c.two_lines.place.top.has_value());
        CHECK(c.two_lines.list_scroll > c.one_line.list_scroll);
        // ImGui keeps scroll offsets in whole pixels.
        CHECK(std::fabs(*c.two_lines.place.top - *c.one_line.place.top) < 1.0f);
    }

    SUBCASE("the same height: nothing moves") {
        list.draw(RowHeight::OneLine, row, std::nullopt);
        const ListFrame again = list.draw(RowHeight::OneLine, row, std::nullopt);
        CHECK(again.list_scroll == first.list_scroll);
    }

    SUBCASE("another row selected: the old place belongs to another song") {
        const HeightChange c = change_height(list, row, list.key(k - 1));
        CHECK(c.two_lines.list_scroll == c.one_line.list_scroll);
    }

    SUBCASE("nothing selected: nothing moves") {
        const HeightChange c = change_height(list, row, std::string());
        CHECK(c.two_lines.list_scroll == c.one_line.list_scroll);
    }

    SUBCASE("a row off screen last frame: the list stays where it is") {
        const std::size_t off = far_row(first);
        const HeightChange c = change_height(list, list.key(off), list.key(off));
        REQUIRE_FALSE(c.one_line.place.top.has_value());
        CHECK(c.two_lines.list_scroll == c.one_line.list_scroll);
        CHECK_FALSE(fully_in_view(c.two_lines, off));
    }
}
