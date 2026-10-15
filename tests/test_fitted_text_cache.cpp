// The report window's fitted-text cache (ui/report_window.h): each cell is
// fitted once per text and width, and fitted again when either changes or
// the cache is cleared. A counting fit stands in for the real cut, so no
// ImGui context is needed.

#include <string>

#include "doctest.h"
#include "ui/report_window.h"

using hydra::ui::report_frame::FittedTextCache;

namespace {

// A fit that counts its calls and names its inputs in what it returns.
struct CountingFit {
    int calls = 0;
    FittedTextCache::Fit fn() {
        return [this](const std::string& text, float width) {
            ++calls;
            return text + "@" + std::to_string(static_cast<int>(width));
        };
    }
};

}  // namespace

TEST_CASE("FittedTextCache: one fit per cell and width") {
    FittedTextCache cache;
    CountingFit counter;
    const FittedTextCache::Fit fit = counter.fn();

    CHECK(cache.fitted(0, 0, "Song", 40.0f, fit) == "Song@40");
    CHECK(cache.fitted(0, 0, "Song", 40.0f, fit) == "Song@40");
    CHECK(counter.calls == 1);

    // Another column of the same row, and another row of the same column,
    // are cells of their own.
    CHECK(cache.fitted(0, 1, "Artist", 40.0f, fit) == "Artist@40");
    CHECK(cache.fitted(1, 0, "Other", 40.0f, fit) == "Other@40");
    CHECK(counter.calls == 3);
    CHECK(cache.size() == 3);

    // Drawn again at the same widths: nothing is fitted.
    cache.fitted(0, 1, "Artist", 40.0f, fit);
    cache.fitted(1, 0, "Other", 40.0f, fit);
    CHECK(counter.calls == 3);
}

TEST_CASE("FittedTextCache: a new width fits the cell again") {
    FittedTextCache cache;
    CountingFit counter;
    const FittedTextCache::Fit fit = counter.fn();

    cache.fitted(2, 3, "Song", 40.0f, fit);
    CHECK(cache.fitted(2, 3, "Song", 55.0f, fit) == "Song@55");
    CHECK(counter.calls == 2);
    // The new width is the one kept.
    CHECK(cache.fitted(2, 3, "Song", 55.0f, fit) == "Song@55");
    CHECK(counter.calls == 2);
}

TEST_CASE("FittedTextCache: new text in a cell fits it again") {
    FittedTextCache cache;
    CountingFit counter;
    const FittedTextCache::Fit fit = counter.fn();

    cache.fitted(0, 0, "Song", 40.0f, fit);
    CHECK(cache.fitted(0, 0, "Other song", 40.0f, fit) == "Other song@40");
    CHECK(counter.calls == 2);
}

TEST_CASE("FittedTextCache: a cleared cache fits every cell afresh") {
    FittedTextCache cache;
    CountingFit counter;
    const FittedTextCache::Fit fit = counter.fn();

    cache.fitted(0, 0, "Song", 40.0f, fit);
    cache.fitted(0, 1, "Artist", 40.0f, fit);
    cache.clear();
    CHECK(cache.size() == 0);
    CHECK(cache.fitted(0, 0, "Song", 40.0f, fit) == "Song@40");
    CHECK(counter.calls == 3);
}

TEST_CASE("FittedTextCache: Memory::reset drops the fits") {
    hydra::ui::report_frame::Memory memory;
    CountingFit counter;
    const FittedTextCache::Fit fit = counter.fn();

    memory.fitted.fitted(0, 0, "Song", 40.0f, fit);
    memory.reset();
    CHECK(memory.fitted.size() == 0);
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
