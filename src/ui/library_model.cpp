#include "ui/library_model.h"

#include <algorithm>
#include <exception>
#include <numeric>
#include <string_view>
#include <thread>
#include <utility>

#include "core/model.h"  // group_thousands
#include "parse/song.h"  // display_title, display_artist, display_album, display_charter

namespace hydra::ui {

namespace {

StatusChip chip_of(store::RecordStatus status) {
    switch (status) {
        case store::RecordStatus::Ready: return StatusChip::Analyzed;
        case store::RecordStatus::Stale: return StatusChip::Stale;
        case store::RecordStatus::CantAnalyze: return StatusChip::CantAnalyze;
        case store::RecordStatus::NotAnalyzed: break;
    }
    return StatusChip::NotAnalyzed;
}

// Where a row with no score sorts under Best path, the same in both
// directions: a Ready result with no paths, then Stale, then Not analyzed,
// then Can't analyze. A chart that can't be analyzed goes last because it is
// the one row that can never gain a score: a Stale or Not analyzed row gains
// one from its next analysis.
int unscored_rank(store::RecordStatus status) {
    switch (status) {
        case store::RecordStatus::Ready: return 0;
        case store::RecordStatus::Stale: return 1;
        case store::RecordStatus::CantAnalyze: return 3;
        case store::RecordStatus::NotAnalyzed: break;
    }
    return 2;
}

// The one place that says when a row has facts a stars: or squeeze filter may
// test: a Ready result with a scored best path
// (PathSummary::has_scored_best_path). A result that kept no path is Analyzed
// but has none, like any other row (D51 call 11).
app::RowFacts facts_of(const LibraryRow& row) {
    if (row.status != store::RecordStatus::Ready || !row.summary.has_scored_best_path())
        return app::RowFacts{};
    return app::RowFacts{row.summary.stars, shown_timing(row.summary.hardest_ms)};
}

}  // namespace

// Beside facts_of on purpose: a summary field the library starts to read
// (here, in best_path_label or in the Best path sort) joins this list, or a
// refresh keeps the row's old value. The lookup's summary and path text are
// the record's own (RecordRow::summary and best_path_text); a lookup that is
// not Ready carries no record (store::RecordLookup), so both are empty. The
// failure's kind and text are set only on a CantAnalyze lookup.
bool LibraryRow::shows_same_as(const store::RecordLookup& lookup) const {
    const store::PathSummary found = lookup.record.summary();
    return status == lookup.status && stale_build == lookup.stale_build &&
           stale_rules == lookup.stale_rules &&
           bestpath == lookup.record.best_path_text() &&
           summary.score == found.score && summary.stars == found.stars &&
           summary.hardest_ms == found.hardest_ms && failure_kind == lookup.failure_kind &&
           failure_text == lookup.failure_text;
}

namespace {

bool matches_query(const app::LibraryQuery& q, const LibraryRow& row) {
    return q.empty() || app::query_matches(q, row.searchable, facts_of(row));
}

// Applies one lookup to one row. False when nothing the library shows or
// filters on changed (LibraryRow::shows_same_as), so a refresh that finds
// the same answers rebuilds nothing.
bool apply_summary(LibraryRow& row, const store::RecordLookup& lookup) {
    if (row.shows_same_as(lookup)) return false;
    row.status = lookup.status;
    row.stale_build = lookup.stale_build;
    row.stale_rules = lookup.stale_rules;
    row.bestpath = lookup.record.best_path_text();
    row.summary = lookup.record.summary();
    row.failure_kind = lookup.failure_kind;
    row.failure_text = lookup.failure_text;
    return true;
}

}  // namespace

const char* status_label(store::RecordStatus status) {
    switch (status) {
        case store::RecordStatus::Ready: return "Analyzed";
        case store::RecordStatus::Stale: return "Stale";
        case store::RecordStatus::CantAnalyze: return "Can't analyze";
        case store::RecordStatus::NotAnalyzed: break;
    }
    return "Not analyzed";
}

// The inverse of chip_of above, read from it: the status whose chip is
// `chip`. All groups every status, so it has none.
std::optional<store::RecordStatus> status_of(StatusChip chip) {
    for (store::RecordStatus s : {store::RecordStatus::NotAnalyzed, store::RecordStatus::Stale,
                                  store::RecordStatus::Ready, store::RecordStatus::CantAnalyze})
        if (chip_of(s) == chip) return s;
    return std::nullopt;
}

std::string chip_label(StatusChip chip, size_t count) {
    const std::optional<store::RecordStatus> status = status_of(chip);
    const char* name = status ? status_label(*status) : "All";
    const char* id = "chipall";
    switch (chip) {
        case StatusChip::All: break;
        case StatusChip::NotAnalyzed: id = "chipnew"; break;
        case StatusChip::Stale: id = "chipstale"; break;
        case StatusChip::Analyzed: id = "chipdone"; break;
        case StatusChip::CantAnalyze: id = "chipcant"; break;
    }
    return std::string(name) + " (" + group_thousands(static_cast<int64_t>(count)) + ")##" + id;
}

std::string best_path_label(store::RecordStatus status, const std::string& bestpath,
                            const store::PathSummary& summary) {
    // A row with no current result shows its status word.
    if (status != store::RecordStatus::Ready) return status_label(status);
    // A Ready result with no paths has no score; its cell shows its path
    // string (empty), as the table always has.
    if (!summary.has_scored_best_path()) return bestpath;
    return group_thousands(*summary.score) + "  " + bestpath;
}

std::string LibraryRow::best_label() const { return best_path_label(status, bestpath, summary); }

namespace {

// The text a column before Best path shows, read in place from the row.
// Empty for Best path, whose text is made when asked for (best_label).
std::string_view stored_cell_text(const LibraryRow& row, int column) {
    switch (sort_of(column)) {
        case LibrarySort::Title: return row.title;
        case LibrarySort::Artist: return row.artist;
        case LibrarySort::Album: return row.album;
        case LibrarySort::Charter: return row.charter;
        case LibrarySort::Folder: return row.entry.rootfolder;
        case LibrarySort::BestPath: break;
    }
    return {};
}

}  // namespace

std::string library_cell_text(const LibraryRow& row, int column) {
    if (column < 0 || column >= kLibraryColumnCount) return {};
    if (sort_of(column) == LibrarySort::BestPath) return row.best_label();
    return std::string(stored_cell_text(row, column));
}

size_t ChipCounts::of(StatusChip chip) const {
    switch (chip) {
        case StatusChip::NotAnalyzed: return not_analyzed;
        case StatusChip::Stale: return stale;
        case StatusChip::Analyzed: return analyzed;
        case StatusChip::CantAnalyze: return cant_analyze;
        case StatusChip::All: break;
    }
    return all;
}

namespace {

// The search text's field for one column before Best path: the stored text
// make_searchable folded, with its tags stripped, and that text folded.
struct SearchField {
    const std::string& stripped;
    const std::string& folded;
};

SearchField search_field_of(const app::StrippedRow& stripped, const app::SearchableRow& folded,
                            int column) {
    switch (sort_of(column)) {
        case LibrarySort::Title: return {stripped.title, folded.title};
        case LibrarySort::Artist: return {stripped.artist, folded.artist};
        case LibrarySort::Album: return {stripped.album, folded.album};
        case LibrarySort::Charter: return {stripped.charter, folded.charter};
        case LibrarySort::Folder: return {stripped.folder, folded.folder};
        case LibrarySort::BestPath: break;  // set_charts makes no Best path key
    }
    return {stripped.folder, folded.folder};
}

// One scanned chart's row. It touches only `entry` (read, then emptied), `row`
// and `stripped` (the caller's scratch, reused across rows), so rows build on
// several threads at once.
void build_row(store::ChartLibraryEntry& entry, LibraryRow& row, app::StrippedRow& stripped) {
    row.title = display_title(entry.title);  // "(unknown)" when only tags
    row.artist = display_artist(entry.artist);  // the same rule as the title
    row.album = display_album(entry.album);  // blank stays blank
    row.charter = display_charter(entry.charter);
    row.searchable = app::make_searchable(entry.title, entry.artist, entry.charter,
                                          entry.rootfolder, entry.album, stripped);
    row.entry.md5 = std::move(entry.md5);
    row.entry.notespath = std::move(entry.notespath);
    row.entry.rootfolder = std::move(entry.rootfolder);
    entry = {};  // the rest of the scan's row goes now, not when the load ends
    // A sort key is the shown text folded. Where the shown text is the
    // search text's stripped text, the search text holds that fold already.
    for (int c = 0; c < kColumnBestPath; ++c) {
        const std::string_view shown = stored_cell_text(row, c);  // library_cell_text, uncopied
        const SearchField field = search_field_of(stripped, row.searchable, c);
        row.sort_keys[static_cast<size_t>(c)] =
            shown == field.stripped ? field.folded : app::fold_for_search(shown);
    }
}

}  // namespace

void LibraryModel::set_charts(std::vector<store::ChartLibraryEntry> charts) {
    const size_t n = charts.size();
    rows_.clear();
    rows_.resize(n);
    // Each thread builds one contiguous share of the rows and writes only
    // those rows. The calling thread builds the first share itself
    // (store::kLibraryOpenThreads says how many threads).
    const size_t threads = std::min(store::kLibraryOpenThreads, n);
    auto build_share = [&](size_t t) {
        app::StrippedRow stripped;
        const size_t end = n * (t + 1) / threads;
        for (size_t i = n * t / threads; i < end; ++i) build_row(charts[i], rows_[i], stripped);
    };
    if (threads == 1) {
        build_share(0);
    } else if (threads > 1) {
        // A share that throws keeps its exception here; the first is rethrown
        // once every thread has joined.
        std::vector<std::exception_ptr> failures(threads);
        auto run_share = [&](size_t t) {
            try {
                build_share(t);
            } catch (...) {
                failures[t] = std::current_exception();
            }
        };
        std::vector<std::thread> helpers;
        helpers.reserve(threads - 1);
        try {
            for (size_t t = 1; t < threads; ++t) helpers.emplace_back(run_share, t);
        } catch (...) {
            for (std::thread& h : helpers) h.join();
            throw;
        }
        run_share(0);
        for (std::thread& h : helpers) h.join();
        for (const std::exception_ptr& failure : failures)
            if (failure) std::rethrow_exception(failure);
    }
    ++rows_version_;
    resort();
    refilter();
}

std::vector<std::string> LibraryModel::hashes() const {
    std::vector<std::string> out;
    out.reserve(rows_.size());
    for (const LibraryRow& row : rows_) out.push_back(row.entry.md5);
    return out;
}

size_t LibraryModel::set_summaries(const std::vector<store::RecordLookup>& lookups) {
    size_t changed = 0;
    const size_t n = std::min(rows_.size(), lookups.size());
    for (size_t i = 0; i < n; ++i)
        if (apply_summary(rows_[i], lookups[i])) ++changed;
    if (changed > 0) summaries_changed();
    return changed;
}

size_t LibraryModel::set_summary_for(const std::string& md5, const store::RecordLookup& lookup) {
    size_t changed = 0;
    for (size_t i = 0; i < rows_.size(); ++i)
        if (rows_[i].entry.md5 == md5 && apply_summary(rows_[i], lookup)) ++changed;
    if (changed > 0) summaries_changed();
    return changed;
}

void LibraryModel::summaries_changed() {
    // A new summary never moves a row by itself: the list would jump under
    // the user while analysis runs. Only the Best path sort reads a summary,
    // so only its order can go out of date, and it waits for
    // sort_if_out_of_date. A new status can still move a row between chips
    // and in or out of a stars:/squeeze filter.
    if (sort_column_ == LibrarySort::BestPath) order_out_of_date_ = true;
    refilter();
}

void LibraryModel::set_query(std::string_view text) {
    if (text == query_text_) return;
    query_text_.assign(text.data(), text.size());
    ++query_version_;
    query_ = app::parse_library_query(text);
    if (!sort_if_out_of_date()) refilter();
}

void LibraryModel::set_chip(StatusChip chip) {
    if (chip == chip_) return;
    chip_ = chip;
    if (!sort_if_out_of_date()) refilter();
}

void LibraryModel::set_sort(LibrarySort column, bool ascending) {
    if (column == sort_column_ && ascending == ascending_) return;
    sort_column_ = column;
    ascending_ = ascending;
    resort();
    refilter();
}

bool LibraryModel::sort_if_out_of_date() {
    if (!order_out_of_date_) return false;
    resort();
    refilter();
    return true;
}

void LibraryModel::resort() {
    order_out_of_date_ = false;
    sorted_.resize(rows_.size());
    std::iota(sorted_.begin(), sorted_.end(), size_t{0});

    // Equal keys fall back to title, folder, then file path, always
    // ascending: one fixed order, so a re-sort moves only the rows it must.
    // The folded strings sort without case or accents getting in the way.
    auto name_key = [](const LibraryRow& r, int column) -> const std::string& {
        return r.sort_keys[static_cast<size_t>(column)];
    };
    auto tie_break = [this, &name_key](size_t a, size_t b) {
        const LibraryRow& x = rows_[a];
        const LibraryRow& y = rows_[b];
        if (int c = name_key(x, kColumnTitle).compare(name_key(y, kColumnTitle))) return c < 0;
        if (int c = name_key(x, kColumnFolder).compare(name_key(y, kColumnFolder))) return c < 0;
        return x.entry.notespath < y.entry.notespath;
    };

    if (sort_column_ == LibrarySort::BestPath) {
        std::sort(sorted_.begin(), sorted_.end(), [&](size_t a, size_t b) {
            const LibraryRow& x = rows_[a];
            const LibraryRow& y = rows_[b];
            const bool xs = x.summary.has_scored_best_path();
            const bool ys = y.summary.has_scored_best_path();
            // Scored rows first in both directions: "not analyzed" is not a
            // low score.
            if (xs != ys) return xs;
            if (xs) {
                if (*x.summary.score != *y.summary.score)
                    return ascending_ ? *x.summary.score < *y.summary.score
                                      : *x.summary.score > *y.summary.score;
            } else {
                const int rx = unscored_rank(x.status), ry = unscored_rank(y.status);
                if (rx != ry) return rx < ry;
            }
            return tie_break(a, b);
        });
        return;
    }

    const int column = column_of(sort_column_);
    std::sort(sorted_.begin(), sorted_.end(), [&](size_t a, size_t b) {
        if (int c = name_key(rows_[a], column).compare(name_key(rows_[b], column)))
            return ascending_ ? c < 0 : c > 0;
        return tie_break(a, b);
    });
}

void LibraryModel::refilter() {
    counts_ = ChipCounts{};
    order_.clear();
    for (size_t i : sorted_) {
        const LibraryRow& row = rows_[i];
        if (!matches_query(query_, row)) continue;
        const StatusChip group = chip_of(row.status);
        ++counts_.all;
        switch (group) {
            case StatusChip::NotAnalyzed: ++counts_.not_analyzed; break;
            case StatusChip::Stale: ++counts_.stale; break;
            case StatusChip::Analyzed: ++counts_.analyzed; break;
            case StatusChip::CantAnalyze: ++counts_.cant_analyze; break;
            case StatusChip::All: break;  // chip_of never answers All
        }
        if (chip_ == StatusChip::All || chip_ == group) order_.push_back(i);
    }
    // A chip whose last chart left it (the last Stale chart was re-analyzed,
    // or the search matches none of that group) falls back to All, so the
    // table never sits empty behind a filter with nothing in it.
    if (chip_ != StatusChip::All && counts_.of(chip_) == 0) {
        chip_ = StatusChip::All;
        refilter();
    }
}

}  // namespace hydra::ui
