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

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "app/library_query.h"
#include "store/record_store.h"

namespace hydra::ui {

// The filter chips above the table, in the order they are drawn: All, then
// one per record status.
enum class StatusChip { All, NotAnalyzed, Stale, Analyzed, CantAnalyze };

// The table's sortable columns. The values are the columns' user IDs in the
// table, so a sort spec maps straight back to one of these. hydra_ui.ini saves
// each column's hidden state and the sort under this ID, so a value never
// changes once shipped and a new column takes the next free one.
enum class LibrarySort { Title = 0, Artist = 1, Charter = 2, Folder = 3, BestPath = 4, Album = 5 };

// The table's columns, by position, left to right. A column's position is not
// its LibrarySort value; sort_of and column_of below are the one place the two
// lists meet. The GUI tests read these too.
inline constexpr int kColumnTitle = 0;
inline constexpr int kColumnArtist = 1;
inline constexpr int kColumnAlbum = 2;
inline constexpr int kColumnCharter = 3;
inline constexpr int kColumnFolder = 4;
inline constexpr int kColumnBestPath = 5;
// How many columns the table has. Best path is the last one.
inline constexpr int kLibraryColumnCount = kColumnBestPath + 1;

// The LibrarySort value (the column's user ID) of the column at `column`.
constexpr LibrarySort sort_of(int column) {
    switch (column) {
        case kColumnArtist: return LibrarySort::Artist;
        case kColumnAlbum: return LibrarySort::Album;
        case kColumnCharter: return LibrarySort::Charter;
        case kColumnFolder: return LibrarySort::Folder;
        case kColumnBestPath: return LibrarySort::BestPath;
        default: return LibrarySort::Title;
    }
}

// The position of the column whose user ID is `sort`: the inverse of sort_of.
constexpr int column_of(LibrarySort sort) {
    for (int c = 0; c < kLibraryColumnCount; ++c)
        if (sort_of(c) == sort) return c;
    return kColumnTitle;
}

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
    std::string title, artist, album, charter;  // colour tags removed: what the table draws
    app::SearchableRow searchable;        // folded stored text, for matching
    // What each column before Best path sorts on, by column index: the
    // column's shown text (library_cell_text), folded. The Library sorts on
    // the text it shows, not the stored text (D114 item 3).
    std::array<std::string, kColumnBestPath> sort_keys;
    store::RecordStatus status = store::RecordStatus::NotAnalyzed;
    bool stale_build = false;             // why a Stale row is Stale (the store's
    bool stale_rules = false;             // RecordLookup); the tooltip names it
    std::string bestpath;                 // set when Ready
    store::PathSummary summary;           // set when Ready (T7)
    // Set when CantAnalyze: the stored failure's kind and raw text
    // (store::RecordLookup). The Best path cell's hover words them through
    // app::stored_failure_text.
    std::optional<ErrorKind> failure_kind;
    std::string failure_text;
    // The Best path cell, made when it is drawn (best_path_label).
    std::string best_label() const;
    // Whether `lookup` changes nothing the library shows, filters or sorts on:
    // the one list of the lookup's fields the library reads.
    bool shows_same_as(const store::RecordLookup& lookup) const;
};

// The word for a record's status, such as "Analyzed". The status chips, the
// Best path cell and hydra_uitest's state dump all read it.
const char* status_label(store::RecordStatus status);

// The record status a chip filters to, so the chip can show status_label's
// word. All filters to no one status, so it has none.
std::optional<store::RecordStatus> status_of(StatusChip chip);

// A filter chip's button label: its word, its count, and the id after "##"
// that keeps the label's identity when the count changes. render_chips draws
// it and the GUI tests look the button up by it.
std::string chip_label(StatusChip chip, size_t count);

// The Best path cell: status_label's word for a row with no Ready result, or
// "<score>  <path>" such as "378,315  3- 1 2". The score is the stored
// summary's, never recomputed.
std::string best_path_label(store::RecordStatus status, const std::string& bestpath,
                            const store::PathSummary& summary);

// The text the table shows in column `column` (kColumnTitle...) of `row`:
// what each cell draws, and what the column's width is measured from.
std::string library_cell_text(const LibraryRow& row, int column);

// Rows the current query matches, by status. The chips show these; the
// selected chip never changes them. Every row counts under exactly one status
// chip, so `all` is the sum of the others.
struct ChipCounts {
    size_t all = 0;
    size_t not_analyzed = 0;
    size_t stale = 0;
    size_t analyzed = 0;
    size_t cant_analyze = 0;
    size_t of(StatusChip chip) const;
};

class LibraryModel {
public:
    // Replaces every row (startup, after a scan). Rows start Not analyzed
    // until set_summaries.
    void set_charts(std::vector<store::ChartLibraryEntry> charts);
    // Each row's chart hash, parallel to rows(): what RecordStore::lookup
    // asks about.
    std::vector<std::string> hashes() const;
    // `lookups` is parallel to rows(). Returns how many rows changed; the
    // order and counts are rebuilt only when some did.
    size_t set_summaries(const std::vector<store::RecordLookup>& lookups);
    // One chart's new answer, applied to every row that lists it (a chart can
    // sit in two folders). Returns how many rows changed.
    size_t set_summary_for(const std::string& md5, const store::RecordLookup& lookup);

    // Changes when set_charts replaces the rows, and at no other time: a new
    // summary changes a row's text but not which rows there are. With
    // query_version, the table's reset key for dragged column widths.
    std::uint64_t rows_version() const { return rows_version_; }
    // Changes each time set_query takes a different search, and at no other
    // time. The table lets a dragged column go when this or rows_version moves.
    std::uint64_t query_version() const { return query_version_; }

    void set_query(std::string_view text);
    void set_chip(StatusChip chip);
    void set_sort(LibrarySort column, bool ascending);

    const std::vector<LibraryRow>& rows() const { return rows_; }
    // The rows shown, as indices into rows(), in sort order: the query and the
    // chip both applied. The one owner of "the rows on screen": the heading
    // counts them and the Analyze button counts and runs them (D115).
    const std::vector<size_t>& order() const { return order_; }
    const app::LibraryQuery& query() const { return query_; }
    // Is the typed search narrowing the library? False for an empty box and
    // for a filter that does not parse (both leave the query empty). The
    // table's footer asks this.
    bool searching() const { return !query_.empty(); }
    // Does the user's own choice, the search or a status chip, leave rows of
    // the library off screen? The toolbar's batch button asks this to pick
    // between its two labels (library_toolbar.cpp).
    bool narrowed() const { return searching() || chip_ != StatusChip::All; }
    const ChipCounts& counts() const { return counts_; }
    StatusChip chip() const { return chip_; }
    LibrarySort sort_column() const { return sort_column_; }
    bool ascending() const { return ascending_; }

private:
    void resort();     // rebuilds sorted_ from rows_ and the sort
    void refilter();   // rebuilds order_ and counts_ from sorted_, the query and the chip
    void summaries_changed();
    std::vector<LibraryRow> rows_;
    std::uint64_t rows_version_ = 0;
    std::uint64_t query_version_ = 0;
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
