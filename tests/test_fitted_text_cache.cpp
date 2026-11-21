// The report window's fitted-text cache (ui/report_window.h): each cell is
// fitted once per text and width, and fitted again when either changes or
// the cache is cleared. The fit handed in is the one the report table hands
// (fitted_text in ui/report_window.cpp): the one cut rule, render::ellipsize,
// measured in the app's font (ui::text_width) inside a headless ImGui frame.
// It counts its calls so a test sees when the cache cuts again, and each
// cell's text is checked against the same cut made without the cache.

#include <map>
#include <string>

#include "parse/song.h"  // read_song_ini_keys
#include "ui/report_window.h"

#include "doctest.h"
#include "headless_imgui_fixture.h"
#include "testdata_dirs.h"  // root

using hydra::ui::report_frame::FittedTextCache;

namespace {

// What the report table shows for `text` in a cell `width` wide, cut with no
// cache in between.
std::string uncached_cut(const std::string& text, float width) {
    return hydra::render::ellipsize(text, width, hydra::ui::text_width);
}

// The table's cut, counting its calls.
struct CountingCut {
    int calls = 0;
    FittedTextCache::Fit fn() {
        return [this](const std::string& text, float width) {
            ++calls;
            return uncached_cut(text, width);
        };
    }
};

// One headless ImGui frame with the app's fonts, so text is measured as the
// window measures it.
struct FontFrame {
    hydra::test::HeadlessImGui imgui;
    FontFrame() { ImGui::NewFrame(); }
    ~FontFrame() { ImGui::EndFrame(); }
    FontFrame(const FontFrame&) = delete;
    FontFrame& operator=(const FontFrame&) = delete;
};

// A real song's title and artist, the text a Path report row's cells show,
// read from the corpus chart's song.ini.
std::map<std::string, std::string> real_song_keys() {
    return hydra::read_song_ini_keys(corpus::root() +
                                     "/common/IB24/T1/Allister - Overrated/song.ini");
}

// A frame, a real title and artist, the cache and the counted cut. `wide` is
// the title's whole width, so the title shows whole; `narrow` is half of it,
// so the title is cut.
struct CutCells {
    FontFrame frame;
    std::map<std::string, std::string> keys = real_song_keys();
    std::string title = keys.at("name");
    std::string artist = keys.at("artist");
    FittedTextCache cache;
    CountingCut cut;
    FittedTextCache::Fit fit = cut.fn();
    float wide = 0.0f;
    float narrow = 0.0f;

    CutCells() {
        wide = hydra::ui::text_width(title);
        narrow = wide * 0.5f;
        // Else a test could not tell the two widths, or the two texts, apart.
        REQUIRE(uncached_cut(title, wide) == title);
        REQUIRE(uncached_cut(title, narrow) != title);
        REQUIRE(uncached_cut(artist, narrow) != uncached_cut(title, narrow));
    }
    CutCells(const CutCells&) = delete;
    CutCells& operator=(const CutCells&) = delete;

    std::string shown(size_t row, size_t column, const std::string& text, float width) {
        return cache.fitted(row, column, text, width, fit);
    }
};

}  // namespace

TEST_CASE("FittedTextCache: one fit per cell and width") {
    CutCells t;

    CHECK(t.shown(0, 0, t.title, t.narrow) == uncached_cut(t.title, t.narrow));
    CHECK(t.shown(0, 0, t.title, t.narrow) == uncached_cut(t.title, t.narrow));
    CHECK(t.cut.calls == 1);

    // Another column of the same row, and another row of the same column,
    // are cells of their own.
    CHECK(t.shown(0, 1, t.artist, t.narrow) == uncached_cut(t.artist, t.narrow));
    CHECK(t.shown(1, 0, t.artist, t.narrow) == uncached_cut(t.artist, t.narrow));
    CHECK(t.cut.calls == 3);
    CHECK(t.cache.size() == 3);

    // Drawn again at the same widths: nothing is fitted.
    t.shown(0, 1, t.artist, t.narrow);
    t.shown(1, 0, t.artist, t.narrow);
    CHECK(t.cut.calls == 3);
}

TEST_CASE("FittedTextCache: a new width fits the cell again") {
    CutCells t;

    t.shown(2, 3, t.title, t.narrow);
    CHECK(t.shown(2, 3, t.title, t.wide) == uncached_cut(t.title, t.wide));
    CHECK(t.cut.calls == 2);
    // The new width is the one kept.
    CHECK(t.shown(2, 3, t.title, t.wide) == uncached_cut(t.title, t.wide));
    CHECK(t.cut.calls == 2);
}

TEST_CASE("FittedTextCache: new text in a cell fits it again") {
    CutCells t;

    t.shown(0, 0, t.title, t.narrow);
    CHECK(t.shown(0, 0, t.artist, t.narrow) == uncached_cut(t.artist, t.narrow));
    CHECK(t.cut.calls == 2);
}

TEST_CASE("FittedTextCache: a cleared cache fits every cell afresh") {
    CutCells t;

    t.shown(0, 0, t.title, t.narrow);
    t.shown(0, 1, t.artist, t.narrow);
    t.cache.clear();
    CHECK(t.cache.size() == 0);
    CHECK(t.shown(0, 0, t.title, t.narrow) == uncached_cut(t.title, t.narrow));
    CHECK(t.cut.calls == 3);
}

// reset starts a new result afresh: every field set up for the old one goes
// back to its starting value, except was_open, which it keeps.
TEST_CASE("FittedTextCache: Memory::reset drops the fits and what the old result set up") {
    CutCells t;
    hydra::ui::report_frame::Memory memory;

    memory.fitted.fitted(0, 0, t.title, t.narrow, t.fit);
    memory.search[0] = 'x';
    memory.selected = 2;
    memory.sort_pushed = true;
    memory.visible = true;
    memory.focused = true;
    memory.was_open = true;
    memory.controls_w = t.wide;
    memory.columns_w = t.wide;
    memory.table_top = t.wide;

    memory.reset();
    CHECK(memory.fitted.size() == 0);
    CHECK(memory.search[0] == '\0');
    CHECK_FALSE(memory.selected.has_value());
    CHECK_FALSE(memory.sort_pushed);
    CHECK_FALSE(memory.visible);
    CHECK_FALSE(memory.focused);
    CHECK(memory.was_open);
    CHECK(memory.controls_w == 0.0f);
    CHECK(memory.columns_w == 0.0f);
    CHECK(memory.table_top == 0.0f);
}

// The report table lets a dragged column go when this key moves (table_room
// in ui/column_widths.h).
TEST_CASE("report Memory: a new result or any change of the search moves the table's reset key") {
    hydra::ui::report_frame::Memory memory;
    const std::uint64_t start = memory.rows_key;

    memory.reset();
    CHECK(memory.rows_key != start);

    const std::uint64_t after_reset = memory.rows_key;
    memory.search_changed();
    CHECK(memory.rows_key != after_reset);

    // "Clear filters" empties the search by this call, so it moves the key too.
    const std::uint64_t before_clear = memory.rows_key;
    memory.search[0] = 'x';
    memory.clear_search();
    CHECK(memory.rows_key != before_clear);
    CHECK(memory.search[0] == '\0');
}
