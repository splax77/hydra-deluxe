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
#include <cstdint>
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

// The part of a scanned chart's row (store::ChartLibraryEntry) the library
// keeps for every chart: its hash, its file and its folder. The rest of the
// scan's row (the names as written, sig, timing) is read from the store when
// a song is clicked or a batch starts (AppState::select, library_matches).
struct LibraryChart {
    std::string md5;         // the chart's hash
    std::string notespath;   // which row is selected, and the sort's last tie-break
    std::string rootfolder;  // the Folder column
};

// What tells one library row from another, for a LibraryChart or a store
// entry alike. Never the md5: the same chart can sit in two folders, and each
// copy is its own row (audit finding 143). Which row is selected, a click's
// copy and a batch's entries all compare this.
inline const std::string& row_key(const LibraryChart& row) { return row.notespath; }
inline const std::string& row_key(const store::ChartLibraryEntry& row) { return row.notespath; }

// One scanned chart as the table shows it.
struct LibraryRow {
    LibraryChart entry;
    std::string title, artist, charter;   // colour tags removed: what the table draws
    app::SearchableRow searchable;        // folded copies, for matching and sorting
    store::RecordStatus status = store::RecordStatus::NotAnalyzed;
    bool stale_build = false;             // why a Stale row is Stale (the store's
    bool stale_rules = false;             // SummaryLookup); the tooltip names it
    std::string bestpath;                 // set when Ready
    store::PathSummary summary;           // set when Ready (T7)
    // The Best path cell, made when it is drawn (best_path_label).
    std::string best_label() const;
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

// The text the table shows in column `column` (kColumnTitle...) of `row`:
// what each cell draws, and what the column's width is measured from.
std::string library_cell_text(const LibraryRow& row, int column);

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

    // Changes when set_charts replaces the rows, and at no other time: a new
    // summary changes a row's text but not which rows there are. The table
    // measures its column widths again when this moves.
    std::uint64_t rows_version() const { return rows_version_; }
    // The rows (indices into rows()) whose summary changed since the last
    // call, which this call forgets. set_charts forgets them too, since every
    // row is new then. The table re-measures just these rows' cells in the
    // columns whose text reads the summary.
    std::vector<size_t> take_summary_changes();

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
    // Applies one lookup to row `index` and records the row for
    // take_summary_changes when it changed. The one place a row's summary is
    // replaced, so the record can never miss a row.
    bool apply_summary_at(size_t index, const store::SummaryLookup& lookup);

    std::vector<LibraryRow> rows_;
    std::uint64_t rows_version_ = 0;
    std::vector<size_t> summary_changes_;  // take_summary_changes
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
