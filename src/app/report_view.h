// The table both report windows show, as plain C++ with no ImGui, so the
// tests pin it. A TableView holds a report's rows, each row's search text,
// the report's column table and a keep-rule from the window's controls. It
// answers which rows show, in what order, and the count line. Each report's
// columns and keep-rule live in its own file (app/path_report_view.h,
// app/dm_report_view.h); the window draws what this hands it.

#ifndef HYDRA_APP_REPORT_VIEW_H
#define HYDRA_APP_REPORT_VIEW_H

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace hydra::app::report_view {

// What a column sorts on for one row: nothing (an empty value), a number or
// text. Empty values sink to the bottom whichever way the column sorts.
using SortKey = std::variant<std::monostate, double, std::string>;

enum class SortDir { Ascending, Descending };

// One sort key the table is ordered by: a column id and its direction.
struct SortSpec {
    std::string column;
    SortDir dir = SortDir::Ascending;
};

// How a cell is drawn, from the pages' cell classes. Fixed per column.
struct CellLook {
    bool dim = false;       // the dim text colour
    bool mono = false;      // the mono font
    bool truncate = false;  // cut with an ellipsis, the full text on hover
    bool chip = false;      // an outlined chip in its token's colour
};

// A colour one row's cell takes over its column's look, where the page
// picked one per row.
enum class Tone { Normal, Dim, Alert };

// One column of a report table.
template <class Row>
struct Column {
    std::string id;
    std::string title;
    // Holds numbers: right-aligned, and first sorted high to low
    // (first_direction).
    bool numeric = false;
    // The header's hover text. Empty when the page gave the column none.
    std::string definition;
    std::function<SortKey(const Row&)> sort_key;
    std::function<std::string(const Row&)> cell;
    CellLook look;
    // Empty when every row of the column reads in its look's colour.
    std::function<Tone(const Row&)> tone;
};

// The words a typed search matches on: the text folded with the library's
// fold_for_search (app/library_query.h), split on whitespace. No query
// language: quotes and field prefixes are ordinary words (D56 item 1).
std::vector<std::string> search_words(std::string_view typed);

// Whether a row whose search text is `search_text` stays for `words`: every
// word appears in it, in any order. No words keeps every row.
bool search_matches(const std::vector<std::string>& words, std::string_view search_text);

// A sort key in the form it is compared in: text goes through
// fold_for_search, so case and accents don't split names apart. The windows
// take the Library table's text order, not the pages' (D103 item 16).
SortKey folded_key(SortKey key);

// The order two folded_key results of one column take: below zero when `a`
// comes first going up, zero when they tie. Empty values are the caller's
// (TableView's) to sink.
int compare_keys(const SortKey& a, const SortKey& b);

// "X of Y <noun>", with thousands grouped the way every count reads.
std::string count_line(size_t shown, size_t total, std::string_view noun);

// Which way a column sorts when it is first picked: numbers high to low,
// text A to Z.
SortDir first_direction(bool numeric);

template <class Row>
class TableView {
public:
    using Keep = std::function<bool(const Row&)>;

    // `search_texts` holds one entry per row, in row order: the text the
    // search box matches against (each report's *_search_text).
    TableView(std::vector<Row> rows, std::vector<std::string> search_texts,
              std::vector<Column<Row>> columns)
        : rows_(std::move(rows)),
          search_texts_(std::move(search_texts)),
          columns_(std::move(columns)) {
        if (search_texts_.size() != rows_.size())
            throw std::invalid_argument("TableView needs one search text per row");
        sorted_.resize(rows_.size());
        std::iota(sorted_.begin(), sorted_.end(), size_t{0});
    }

    const std::vector<Row>& rows() const { return rows_; }
    const std::vector<Column<Row>>& columns() const { return columns_; }

    // The column with this id. Throws for an id the table doesn't have.
    const Column<Row>& column(std::string_view id) const {
        for (const Column<Row>& c : columns_)
            if (c.id == id) return c;
        throw std::invalid_argument("no report column \"" + std::string(id) + "\"");
    }

    // The words typed in the search box.
    void set_search(std::string_view typed) {
        search_ = std::string(typed);
        words_ = search_words(typed);
        dirty_ = true;
    }
    void clear_search() { set_search(std::string_view()); }
    const std::string& search() const { return search_; }

    // The rows the window's controls keep. An empty rule keeps every row.
    void set_keep(Keep keep) {
        keep_ = std::move(keep);
        dirty_ = true;
    }

    // Orders the table by up to two columns, the first deciding and the
    // second breaking its ties. Rows that tie on both keep their row order.
    void set_sort(std::vector<SortSpec> specs) {
        if (specs.size() > 2)
            throw std::invalid_argument("a report table sorts by at most two columns");
        for (const SortSpec& s : specs) column(s.column);  // throws for an unknown id
        sort_ = std::move(specs);
        resort();
        dirty_ = true;
    }
    const std::vector<SortSpec>& sort() const { return sort_; }

    // Which way `column` sorts when it is first picked.
    SortDir first_direction(std::string_view column_id) const {
        return report_view::first_direction(column(column_id).numeric);
    }

    // The indices into rows() to draw, in order: the rows the keep-rule and
    // the search both keep, in the current sort.
    const std::vector<size_t>& visible() const {
        if (dirty_) refilter();
        return visible_;
    }

    // "X of Y <noun>": the rows shown out of every row.
    std::string count_line(std::string_view noun) const {
        return report_view::count_line(visible().size(), rows_.size(), noun);
    }

private:
    void resort() {
        std::iota(sorted_.begin(), sorted_.end(), size_t{0});
        if (sort_.empty()) return;
        // Each sort column's keys, worked out once per row.
        std::vector<std::vector<SortKey>> keys;
        std::vector<SortDir> dirs;
        for (const SortSpec& s : sort_) {
            const Column<Row>& c = column(s.column);
            std::vector<SortKey> col_keys;
            col_keys.reserve(rows_.size());
            for (const Row& r : rows_) col_keys.push_back(folded_key(c.sort_key(r)));
            keys.push_back(std::move(col_keys));
            dirs.push_back(s.dir);
        }
        std::stable_sort(sorted_.begin(), sorted_.end(), [&](size_t a, size_t b) {
            for (size_t k = 0; k < keys.size(); ++k) {
                const SortKey& x = keys[k][a];
                const SortKey& y = keys[k][b];
                const bool xe = std::holds_alternative<std::monostate>(x);
                const bool ye = std::holds_alternative<std::monostate>(y);
                if (xe && ye) continue;
                if (xe != ye) return ye;  // the empty one sinks
                const int c = compare_keys(x, y);
                if (c != 0) return dirs[k] == SortDir::Ascending ? c < 0 : c > 0;
            }
            return false;
        });
    }

    void refilter() const {
        visible_.clear();
        for (size_t i : sorted_) {
            if (keep_ && !keep_(rows_[i])) continue;
            if (!search_matches(words_, search_texts_[i])) continue;
            visible_.push_back(i);
        }
        dirty_ = false;
    }

    std::vector<Row> rows_;
    std::vector<std::string> search_texts_;
    std::vector<Column<Row>> columns_;
    std::string search_;
    std::vector<std::string> words_;
    Keep keep_;
    std::vector<SortSpec> sort_;
    std::vector<size_t> sorted_;  // every row, in sort order
    mutable std::vector<size_t> visible_;
    mutable bool dirty_ = true;
};

}  // namespace hydra::app::report_view

#endif  // HYDRA_APP_REPORT_VIEW_H
