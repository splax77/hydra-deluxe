// The library search box's rules, in one place: how text is folded for
// matching (case, accents, full-width letters, whitespace), how Clone Hero's
// rich-text tags are removed, how a typed query is read, whether a row
// matches it, and which bytes of a shown string to highlight. Pure functions
// with no ImGui and no store, so the library table and the song panel share
// them and the unit tests pin them.

#ifndef HYDRA_APP_LIBRARY_QUERY_H
#define HYDRA_APP_LIBRARY_QUERY_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "parse/song.h"  // strip_rich_tags

namespace hydra::app {

// Lowercase, accents removed (NFD-style fold for Latin-1 and Latin Extended-A:
// "é" -> "e", "ß" -> "ss"), full-width ASCII folded to ASCII, runs of
// whitespace collapsed to one space. Non-Latin text (Japanese, etc.) passes
// through unchanged apart from full-width folding. Input and output are UTF-8.
// Whitespace includes the no-break space (U+00A0) and the ideographic space
// (U+3000). The signs × and ÷ are kept. Invalid UTF-8 passes through byte for
// byte; nothing is dropped and nothing throws.
std::string fold_for_search(std::string_view text);

// One character fold_for_search changes, and what it becomes. Both are UTF-8.
struct FoldEntry {
    std::string from;
    std::string to;
};

// Every single character fold_for_search changes, with its folded form: each
// character of the fold's own named runs (library_query.cpp) run through
// fold_for_search one at a time, keeping only those that come out different.
// Whitespace is left out; the pages split a query on it. The report pages fold a
// typed query by looking each character up in this table, so they search the
// way the library does without a second copy of the fold. In code point order.
std::vector<FoldEntry> search_fold_table();

// Removes Clone Hero rich-text tags. The rule lives in parse/song.h, beside
// display_title; the library reads the same function under this name.
using hydra::strip_rich_tags;

enum class QueryField { Any, Title, Artist, Album, Charter, Folder };

struct QueryTerm {
    QueryField field = QueryField::Any;
    std::string folded;   // fold_for_search of the word or quoted phrase
    bool phrase = false;  // true when it came from "quotes"
};

struct LibraryQuery {
    std::vector<QueryTerm> terms;         // every term must match
    std::optional<int> stars;             // stars:N, N in 0..kMaxStars
    std::optional<double> squeeze_max_ms; // squeeze<=N or squeeze<N
    bool squeeze_max_strict = false;      // true for squeeze<N, false for squeeze<=N
    std::optional<double> squeeze_min_ms; // squeeze>=N or squeeze>N
    bool squeeze_min_strict = false;      // true for squeeze>N, false for squeeze>=N
    std::vector<std::string> errors;      // one plain sentence per bad filter value
    bool empty() const;                   // no terms and no filters
};

// Parses what the user typed. Words match in any order. "quoted text" is one
// phrase. artist:x, album:x, charter:x, folder:x, title:x limit a word or
// "phrase" to one field (field_named in library_query.cpp). stars:N and
// squeeze<=N, squeeze<N, squeeze>=N, squeeze>N filter on the stored best path; a low and a high squeeze filter combine into a
// range. Unknown field names are treated as plain words.
// A word or phrase matches when its folded text appears anywhere in a folded
// field. An unclosed quote runs to the end. A field name with nothing after it
// is dropped. A bad filter value, or squeeze followed by an operator it does
// not take, adds one sentence to `errors` and filters nothing. When a filter
// appears twice, the last one counts; for squeeze, each side counts apart.
LibraryQuery parse_library_query(std::string_view text);

// The fields one library row offers to a query, already folded.
struct SearchableRow {
    std::string title, artist, album, charter, folder;  // fold_for_search(strip_rich_tags(x))
};
// A chart with no album passes none; an empty field matches no term.
SearchableRow make_searchable(std::string_view title, std::string_view artist,
                              std::string_view charter, std::string_view folder,
                              std::string_view album = {});

// A report row's search text: the fields make_searchable folds and strips,
// joined by single spaces, with empty fields left out. A report keeps a row
// when every word of the folded query appears in it. The path report passes
// its path in the folder slot; the comparisons pass none. Reports show no
// album, so none goes in.
std::string search_field(std::string_view song, std::string_view artist,
                         std::string_view charter, std::string_view path = {});

// Whether a term limited to `term_field` counts in `column`: true when either
// is Any or the two are the same field. The one rule for both matching
// (query_matches) and highlighting (match_spans).
bool term_applies_to(QueryField term_field, QueryField column);

// The best path's stored facts a filter can test. A row with no facts matches
// no filter; when a row has facts is the library's call (facts_of in
// ui/library_model.cpp).
struct RowFacts {
    std::optional<int> stars;
    // nullopt = the path needs no timing (Path::needs_timing).
    std::optional<double> hardest_ms;
};

// True when every term counts (see term_applies_to) and every filter holds.
// A row with no facts matches no filter. A path that needs no timing
// (Path::needs_timing) passes any squeeze<= or squeeze< limit and matches no
// squeeze>= or squeeze> filter, since it has nothing to squeeze. Allocates
// nothing.
bool query_matches(const LibraryQuery& q, const SearchableRow& row, const RowFacts& facts);

// Where the query's terms appear in one displayed (unfolded, tag-stripped)
// string, as byte ranges, for highlighting. Ranges are sorted and don't overlap.
// Only terms that term_applies_to says count for `field`. A match covers every
// byte of each character it touches, so "beyonce"
// in "Beyoncé" covers both bytes of "é". Touching ranges merge.
struct MatchSpan { size_t begin = 0, end = 0; };
std::vector<MatchSpan> match_spans(const LibraryQuery& q, QueryField field,
                                   std::string_view display_text);

// One line of the library's Search tips popup: what to type, with an example
// a player can try on the test library, and what it does in plain words.
struct SearchTip {
    std::string_view example;  // e.g. album:dookie
    std::string_view meaning;
};

// Every key of the query language parse_library_query reads, in the order the
// popup lists them. The popup draws these and writes no key of its own.
const std::vector<SearchTip>& search_tips();

// The one line the popup shows under the tips: which rows the number filters
// can never match (facts_of in ui/library_model.cpp decides).
std::string_view search_tips_note();

}  // namespace hydra::app

#endif  // HYDRA_APP_LIBRARY_QUERY_H
