// The library as the main window browses it: every scanned chart, its stored
// summary, the user's search, the status chip and the sort. Plain data with no
// ImGui, so it can be unit-tested and timed. AppState owns one.
//
// Why in memory and not in SQL: whether a row is Stale or Ready is the store's
// C++ winner rule, and search folding (accents, colour tags) lives in
// app/library_query. Writing either again in SQL would be a second copy of the
// rule. A library of about 20,000 charts filters here in a few milliseconds.

#ifndef HYDRA_UI_LIBRARY_MODEL_H
#define HYDRA_UI_LIBRARY_MODEL_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "app/library_query.h"
#include "store/record_store.h"

namespace hydra::ui {

// The four filter chips above the table.
enum class StatusChip { All, NotAnalyzed, Stale, Analyzed };

// The table's sortable columns. The values are the columns' user IDs in the
// table, so a sort spec maps straight back to one of these.
enum class LibrarySort { Title = 0, Artist = 1, Charter = 2, Folder = 3, BestPath = 4 };

// The table's columns, by index: each column sits at its LibrarySort value.
// The GUI tests read them too.
inline constexpr int kColumnTitle = static_cast<int>(LibrarySort::Title);
inline constexpr int kColumnArtist = static_cast<int>(LibrarySort::Artist);
inline constexpr int kColumnCharter = static_cast<int>(LibrarySort::Charter);
inline constexpr int kColumnFolder = static_cast<int>(LibrarySort::Folder);
inline constexpr int kColumnBestPath = static_cast<int>(LibrarySort::BestPath);
// How many columns the table has: one per LibrarySort value.
inline constexpr int kLibraryColumnCount = kColumnBestPath + 1;

// One scanned chart as the table shows it.
struct LibraryRow {
    store::ChartLibraryEntry entry;       // as scanned; entry.md5 is the chart's hash
    std::string title, artist, charter;   // colour tags removed: what the table draws
    app::SearchableRow searchable;        // folded copies, for matching and sorting
    store::RecordStatus status = store::RecordStatus::NotAnalyzed;
    bool stale_build = false;             // why a Stale row is Stale (the store's
    bool stale_rules = false;             // SummaryLookup); the tooltip names it
    std::string bestpath;                 // set when Ready
    store::PathSummary summary;           // set when Ready (T7)
    std::string best_label;               // the Best path cell (best_path_label)
};

// The word for a record's status: "Analyzed", "Stale" or "Not analyzed". The
// status chips, the Best path cell and hydra_uitest's state dump all read it.
const char* status_label(store::RecordStatus status);

// The record status a chip filters to, so the chip can show status_label's
// word. All filters to no one status, so it has none.
std::optional<store::RecordStatus> status_of(StatusChip chip);

// A filter chip's button label: its word, its count, and the id after "##"
// that keeps the label's identity when the count changes. render_chips draws
// it and the GUI tests look the button up by it.
std::string chip_label(StatusChip chip, size_t count);

// The Best path cell: "Not analyzed", "Stale", or "<score>  <path>" such as
// "378,315  3- 1 2". The score is the stored summary's, never recomputed.
std::string best_path_label(store::RecordStatus status, const std::string& bestpath,
                            const store::PathSummary& summary);

// Rows the current query matches, by status. The chips show these; the
// selected chip never changes them.
struct ChipCounts {
    size_t all = 0;
    size_t not_analyzed = 0;
    size_t stale = 0;
    size_t analyzed = 0;
    size_t of(StatusChip chip) const;
};

class LibraryModel {
public:
    // Replaces every row (startup, after a scan). Rows start Not analyzed
    // until set_summaries.
    void set_charts(std::vector<store::ChartLibraryEntry> charts);
    // Each row's chart hash, parallel to rows(): what get_summaries asks about.
    std::vector<std::string> hashes() const;
    // `lookups` is parallel to rows(). Returns how many rows changed; the
    // order and counts are rebuilt only when some did.
    size_t set_summaries(const std::vector<store::SummaryLookup>& lookups);
    // One chart's new answer, applied to every row that lists it (a chart can
    // sit in two folders). Returns how many rows changed.
    size_t set_summary_for(const std::string& md5, const store::SummaryLookup& lookup);

    void set_query(std::string_view text);
    void set_chip(StatusChip chip);
    void set_sort(LibrarySort column, bool ascending);

    const std::vector<LibraryRow>& rows() const { return rows_; }
    // The rows shown, as indices into rows(), in sort order: the query and the
    // chip both applied.
    const std::vector<size_t>& order() const { return order_; }
    // The rows the query matches, whatever the chip, in sort order: what
    // "Analyze search (N)..." analyzes.
    std::vector<size_t> matches() const;
    const app::LibraryQuery& query() const { return query_; }
    // Is the typed search narrowing the library? False for an empty box and
    // for a filter that does not parse (both leave the query empty). The
    // toolbar's batch button (library_toolbar.cpp) and the table's footer
    // both ask this.
    bool searching() const { return !query_.empty(); }
    const ChipCounts& counts() const { return counts_; }
    StatusChip chip() const { return chip_; }
    LibrarySort sort_column() const { return sort_column_; }
    bool ascending() const { return ascending_; }

private:
    void resort();     // rebuilds sorted_ from rows_ and the sort
    void refilter();   // rebuilds order_ and counts_ from sorted_, the query and the chip
    void summaries_changed();

    std::vector<LibraryRow> rows_;
    std::vector<size_t> sorted_;  // every row, in sort order
    std::vector<size_t> order_;
    std::string query_text_;
    app::LibraryQuery query_;
    ChipCounts counts_;
    StatusChip chip_ = StatusChip::All;
    LibrarySort sort_column_ = LibrarySort::Title;
    bool ascending_ = true;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_LIBRARY_MODEL_H
