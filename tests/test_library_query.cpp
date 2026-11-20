// Unit tests for app/library_query: how the library search folds text,
// strips Clone Hero's rich-text tags, reads a query, matches a row and marks
// what to highlight. Named rows are the corpus scan's own rows from
// testdata\input, and a row's stars and timing come from analysing its chart.

#include "doctest.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <functional>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "app/analysis.h"  // discover_charts, BatchRun
#include "app/config.h"    // Settings::batch_run: the default analysis
#include "app/library_query.h"
#include "core/model.h"   // engine_hardest_ms, shown_timing
#include "core/stars.h"   // kMaxStars: the top of the stars: filter
#include "core/winstr.h"  // wide_to_utf8
#include "corpus_util.h"  // chart_paths, analyzed: corpus charts and their analyses
#include "store/record_store.h"
#include "testdata_dirs.h"  // corpus::root

using namespace hydra::app;

namespace {

bool matches(std::string_view query, const SearchableRow& row, const RowFacts& facts = {}) {
    return query_matches(parse_library_query(query), row, facts);
}

// The facts of an analyzed row: its best path's stars and hardest squeeze.
RowFacts analyzed(int stars, std::optional<double> hardest_ms) {
    RowFacts facts;
    facts.stars = stars;
    facts.hardest_ms = hardest_ms;
    return facts;
}

// The rows a library scan of the corpus saves, read once.
const std::vector<hydra::store::ChartLibraryEntry>& corpus_scan_entries() {
    static const std::vector<hydra::store::ChartLibraryEntry> entries = [] {
        auto scan = hydra::app::discover_charts({corpus::root()});
        if (!scan.second.empty())
            throw std::runtime_error("corpus scan error: " + scan.second.front());
        return scan.first;
    }();
    return entries;
}

// The scanned row whose chart file sits in a folder named `song_folder`.
const hydra::store::ChartLibraryEntry& scanned_in(std::string_view song_folder) {
    for (const hydra::store::ChartLibraryEntry& e : corpus_scan_entries())
        if (e.notespath.find(song_folder) != std::string::npos) return e;
    throw std::runtime_error("no corpus chart in a folder named " + std::string(song_folder));
}

// The search fields the library makes for a scanned row: the same
// make_searchable call, on the same columns, as LibraryModel::set_charts.
SearchableRow searchable_of(const hydra::store::ChartLibraryEntry& e) {
    return make_searchable(e.title, e.artist, e.charter, e.rootfolder, e.album);
}

SearchableRow scanned_row(std::string_view song_folder) {
    return searchable_of(scanned_in(song_folder));
}

SearchableRow scanned_burnout() { return scanned_row("Green Day - Burnout"); }

SearchableRow scanned_deadbolt() { return scanned_row("Thrice - Deadbolt"); }

SearchableRow scanned_acid_romance() { return scanned_row("Alpha Wolf - Acid Romance"); }

// One corpus chart's search row and the facts the library reads for it.
struct ChartFacts {
    std::string chart;
    SearchableRow row;
    RowFacts facts;
};

// A chart's facts as the library reads them: its analysis under the default
// settings saved to a store and read back, then facts_of's rule
// (ui/library_model.cpp): a Ready result's best-path stars and shown timing.
ChartFacts saved_facts(const hydra::store::ChartLibraryEntry& e) {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    hydra::store::RecordStore store(":memory:");
    store.save(hydra::store::prepare_record(e.md5, run.profile,
                                            corpus::analyzed(e.notespath, run.settings)));
    const hydra::store::RecordLookup got = store.lookup_one(e.md5, run.profile);
    const hydra::store::PathSummary summary = got.record.summary();
    if (got.status != hydra::store::RecordStatus::Ready || !summary.has_scored_best_path())
        throw std::runtime_error("no scored best path saved for " + e.notespath);
    return ChartFacts{e.notespath, searchable_of(e),
                      RowFacts{summary.stars, hydra::shown_timing(summary.hardest_ms)}};
}

const ChartFacts& burnout_facts() {
    static const ChartFacts facts = saved_facts(scanned_in("Green Day - Burnout"));
    return facts;
}

// The first corpus chart, in chart_paths() order, whose best path's engine
// figure (engine_hardest_ms) passes `want`. Charts that do not analyze to a
// path are passed over. Throws when no chart has such a figure.
ChartFacts first_chart_whose_figure(const std::function<bool(double)>& want) {
    const hydra::app::BatchRun run = hydra::app::Settings{}.batch_run();
    for (const std::string& path : corpus::chart_paths()) {
        std::optional<double> figure;
        try {
            const hydra::HydraRecord& rec = corpus::analyzed(path, run.settings);
            if (rec.paths.empty()) continue;
            figure = hydra::engine_hardest_ms(rec.best_path());
        } catch (const std::exception&) {
            continue;
        }
        if (!figure || !want(*figure)) continue;
        for (const hydra::store::ChartLibraryEntry& e : corpus_scan_entries())
            if (e.notespath == path) return saved_facts(e);
    }
    throw std::runtime_error("no corpus chart's best path has the figure this test needs");
}

// A chart whose best path's figure is exactly 0.0 ms: a timing to hit.
const ChartFacts& on_end_facts() {
    static const ChartFacts facts = first_chart_whose_figure([](double ms) { return ms == 0.0; });
    return facts;
}

// A chart whose best path's figure is below 0 ms: a path that needs no timing.
const ChartFacts& untimed_facts() {
    static const ChartFacts facts = first_chart_whose_figure([](double ms) { return ms < 0.0; });
    return facts;
}

// A number as the shortest text that reads back as the same double, so a
// filter typed with it sits exactly on that value.
std::string ms_text(double ms) {
    char buf[64];
    const std::to_chars_result r = std::to_chars(buf, buf + sizeof buf, ms);
    if (r.ec != std::errc{}) throw std::runtime_error("cannot write a number as text");
    return std::string(buf, r.ptr);
}

bool spans_equal(const std::vector<MatchSpan>& got, const std::vector<MatchSpan>& want) {
    return std::equal(got.begin(), got.end(), want.begin(), want.end(),
                      [](const MatchSpan& a, const MatchSpan& b) {
                          return a.begin == b.begin && a.end == b.end;
                      });
}

}  // namespace

TEST_CASE("library query: folding lowercases and removes accents") {
    // Every name the corpus scan reads folds with no capital left. The
    // corpus's one accented name, the album of The Fall of Troy's "Act One,
    // Scene One" (song.ini: album = Doppelgänger), folds to plain ASCII, one
    // letter for each letter shown, its unaccented letters lowercased in place.
    for (const hydra::store::ChartLibraryEntry& e : corpus_scan_entries()) {
        for (const std::string* s : {&e.title, &e.artist, &e.album, &e.charter, &e.rootfolder}) {
            INFO("text: " << *s);
            const std::string folded = fold_for_search(*s);
            CHECK(std::none_of(folded.begin(), folded.end(),
                               [](char c) { return c >= 'A' && c <= 'Z'; }));
        }
    }
    const std::string shown = scanned_in("The Fall of Troy - Act One, Scene One").album;
    REQUIRE(std::any_of(shown.begin(), shown.end(),
                        [](char c) { return static_cast<unsigned char>(c) >= 0x80; }));
    const std::string folded = fold_for_search(shown);
    CHECK(std::all_of(folded.begin(), folded.end(),
                      [](char c) { return static_cast<unsigned char>(c) < 0x80; }));
    // The accented letter is two bytes in UTF-8 and folds to one.
    REQUIRE(folded.size() + 1 == shown.size());
    size_t at = 0;
    for (char c : shown) {
        if (static_cast<unsigned char>(c) >= 0x80) continue;
        while (at < folded.size() && folded[at] != fold_for_search(std::string(1, c))[0]) ++at;
        CHECK(at < folded.size());  // each plain letter is kept, in order
        ++at;
    }
}

TEST_CASE("library query: folding turns full-width ASCII into ASCII") {
    // black midi's "Sugar/Tzu" names itself with an ASCII slash in song.ini
    // and with the full-width slash U+FF0F in its folder's name. Folded, the
    // folder's name holds the folded title.
    const hydra::store::ChartLibraryEntry& e = scanned_in("black midi - Sugar");
    const std::string folder_name = e.notespath.substr(0, e.notespath.find_last_of("\\/"));
    REQUIRE(folder_name.find(e.title) == std::string::npos);
    CHECK(fold_for_search(folder_name).find(fold_for_search(e.title)) != std::string::npos);
}

TEST_CASE("library query: the plain fold and the traced fold give the same text") {
    // fold_for_search may take shortcuts the traced fold (match_spans') does
    // not, so the two are compared on real and made-up text.
    std::vector<std::string> texts;
    // Every name, folder and file path a scan of the corpus reads, as stored
    // and with its tags stripped.
    const auto [items, errors] = hydra::app::discover_charts({corpus::root()});
    REQUIRE(errors.empty());
    REQUIRE_FALSE(items.empty());
    for (const hydra::store::ChartLibraryEntry& e : items) {
        for (const std::string* s :
             {&e.title, &e.artist, &e.album, &e.charter, &e.rootfolder, &e.notespath}) {
            texts.push_back(*s);
            texts.push_back(strip_rich_tags(*s));
        }
    }
    // Non-ASCII text: every character the fold changes, alone and between
    // ASCII letters and whitespace, then kept characters, whitespace of every
    // kind and invalid UTF-8 in runs with ASCII. Separate literals keep a hex
    // escape from running into the next letter.
    for (const FoldEntry& entry : search_fold_table()) {
        texts.push_back(entry.from);
        texts.push_back("Ab " + entry.from + entry.from + "\tCd");
    }
    for (const char* s : {"東京事変 Tokyo Jihen", "a × b ÷ c", "  Ｔｉｅｒ　 4  ",
                          "x" "\xC2\xA0" " \t" "\xE3\x80\x80" "y", " " "\xC2\xA0",
                          "A" "\xC3" "(B", "\xFF\xFE" "AB", "x" "\xC3", "Q" "\xEF\xBC",
                          "\xC1\xBF" "ab", "\xF5\x80\x80\x80" "a", "\xF0\x9F\x8E\xB8" " Guitar ",
                          "", " ", "\t\t", "MiXeD CaSe 123 !?"})
        texts.push_back(s);
    // Every ASCII byte (control bytes, the whitespace kinds, 0x7F) alone, in a
    // run of itself, and between letters and a non-ASCII character.
    for (int b = 0; b < 0x80; ++b) {
        const std::string c(1, static_cast<char>(b));
        texts.push_back(c);
        texts.push_back(c + c + c);
        texts.push_back("Ab" + c + "Cd");
        texts.push_back(c + "Ｘ" + c + "é" + c);
    }
    for (const std::string& t : texts) {
        INFO("text: " << t);
        CHECK(fold_for_search(t) == traced_fold_for_search(t));
    }
}

TEST_CASE("library query: only a real lead byte starts a kept character") {
    // A kept character maps back to all of its bytes, so a match that starts
    // on its last byte lights up the whole character. "©" (C2 A9), "€"
    // (E2 82 AC) and the guitar (F0 9F 8E B8) are two, three and four bytes.
    CHECK(spans_equal(match_spans(parse_library_query("\xA9" "y"), QueryField::Title, "\xC2\xA9" "y"),
                      std::vector<MatchSpan>{MatchSpan{0, 3}}));
    CHECK(spans_equal(match_spans(parse_library_query("\xAC" "y"), QueryField::Title, "\xE2\x82\xAC" "y"),
                      std::vector<MatchSpan>{MatchSpan{0, 4}}));
    CHECK(spans_equal(match_spans(parse_library_query("\xB8" "y"), QueryField::Title, "\xF0\x9F\x8E\xB8" "y"),
                      std::vector<MatchSpan>{MatchSpan{0, 5}}));
    // C1 and F5 never start a character, so the byte after them is its own
    // and the span starts there.
    CHECK(spans_equal(match_spans(parse_library_query("\xBF" "ab"), QueryField::Title, "\xC1\xBF" "ab"),
                      std::vector<MatchSpan>{MatchSpan{1, 4}}));
    CHECK(spans_equal(match_spans(parse_library_query("\x80" "a"), QueryField::Title, "\xF5\x80\x80\x80" "a"),
                      std::vector<MatchSpan>{MatchSpan{3, 5}}));
}

namespace {

// The corpus's tagged names: every scanned title, artist, album or charter
// that holds a '<'. Each one there is a Clone Hero colour tag, such as
// Bloodline's "<color=#e02222>Blood</color>line" (Acid Romance's song.ini).
std::vector<std::string> tagged_names() {
    std::vector<std::string> out;
    for (const hydra::store::ChartLibraryEntry& e : corpus_scan_entries())
        for (const std::string* s : {&e.title, &e.artist, &e.album, &e.charter})
            if (s->find('<') != std::string::npos) out.push_back(*s);
    return out;
}

}  // namespace

TEST_CASE("library query: rich-text tags are stripped and the text between them kept") {
    const std::vector<std::string> tagged = tagged_names();
    REQUIRE_FALSE(tagged.empty());
    for (const std::string& shown : tagged) {
        INFO("text: " << shown);
        const std::string plain = strip_rich_tags(shown);
        CHECK(plain.find('<') == std::string::npos);
        CHECK(plain.find('>') == std::string::npos);
        CHECK_FALSE(plain.empty());
        // What is left is the text outside the tags, in its order.
        size_t at = 0;
        for (char c : plain) {
            at = shown.find(c, at);
            CHECK(at != std::string::npos);
            if (at == std::string::npos) break;
            ++at;
        }
    }
}

TEST_CASE("library query: a tagged charter is found by its plain name") {
    // Acid Romance's charter is the tagged "<color=#e02222>Blood</color>line".
    const hydra::store::ChartLibraryEntry& e = scanned_in("Alpha Wolf - Acid Romance");
    const SearchableRow row = searchable_of(e);
    const std::string plain = strip_rich_tags(e.charter);
    REQUIRE(plain != e.charter);
    CHECK(matches(plain, row));
    CHECK(matches("charter:" + plain.substr(0, plain.size() / 2), row));
    // The opening tag's name and value, "color" and "#e02222", are gone.
    const size_t open = e.charter.find('<');
    const size_t eq = e.charter.find('=', open);
    const size_t close = e.charter.find('>', open);
    REQUIRE(open != std::string::npos);
    REQUIRE(eq != std::string::npos);
    REQUIRE(close != std::string::npos);
    REQUIRE(eq < close);
    const std::string tag_name = e.charter.substr(open + 1, eq - open - 1);
    const std::string tag_value = e.charter.substr(eq + 1, close - eq - 1);
    CHECK_FALSE(matches("charter:" + tag_name, row));
    CHECK_FALSE(matches("charter:" + tag_value, row));
}

TEST_CASE("library query: an accented name is found without its accents") {
    // The Fall of Troy's album, as its song.ini spells it: "Doppelgänger".
    const hydra::store::ChartLibraryEntry& e = scanned_in("The Fall of Troy - Act One, Scene One");
    const SearchableRow row = searchable_of(e);
    CHECK(matches("doppelganger", row));
    CHECK(matches("ALBUM:DOPPELGANGER", row));
    CHECK(matches(e.album, row));  // typed with its accent, as shown
}

TEST_CASE("library query: the report pages' fold table is the fold, one character at a time") {
    // The report pages fold a typed query with this table, so every entry has
    // to be exactly what the library's own fold does to that character.
    const std::vector<FoldEntry> table = search_fold_table();
    REQUIRE_FALSE(table.empty());
    for (const FoldEntry& e : table) {
        CHECK(e.to == fold_for_search(e.from));
        CHECK(e.to != e.from);  // a character the fold keeps is left out
    }
    auto folded = [&](std::string_view from) -> std::optional<std::string> {
        for (const FoldEntry& e : table)
            if (e.from == from) return e.to;
        return std::nullopt;
    };
    CHECK(folded("\xc3\x89") == std::optional<std::string>("e"));  // É
    CHECK(folded("\xc3\x84").has_value());                         // Ä
    CHECK(folded("B") == std::optional<std::string>("b"));
    CHECK(folded("\xef\xbc\xa1") == std::optional<std::string>("a"));  // full-width A
    CHECK_FALSE(folded("\xc3\x97").has_value());  // × is kept, so it is not listed
    CHECK_FALSE(folded("b").has_value());
}

TEST_CASE("library query: the report pages' fold table holds every character the fold changes") {
    // Walk every character below U+10000 (the surrogate halves are not
    // characters on their own). Each one the library's fold changes has to be
    // in the table, or a report page would stop folding it. The one exception
    // is whitespace: the fold turns it into a space, and the page splits the
    // query on whitespace instead of looking it up.
    std::set<std::string> listed;
    for (const FoldEntry& e : search_fold_table()) listed.insert(e.from);
    std::vector<unsigned> missing;
    for (unsigned cp = 0; cp <= 0xFFFF; ++cp) {
        if (cp >= 0xD800 && cp <= 0xDFFF) continue;
        const std::string from = hydra::wide_to_utf8(std::wstring(1, static_cast<wchar_t>(cp)));
        const std::string to = fold_for_search(from);
        if (to == from || to == " ") continue;
        if (!listed.count(from)) missing.push_back(cp);
    }
    INFO("first missing code point: " << (missing.empty() ? 0u : missing.front()));
    CHECK(missing.size() == 0);
}

TEST_CASE("library query: words match in any order and across fields") {
    const SearchableRow row = scanned_burnout();
    CHECK(matches("green burnout", row));
    CHECK(matches("burnout green", row));
    CHECK(matches("  Green   BURNOUT  ", row));
    CHECK(matches("hoph2o tier", row));  // charter and folder
    CHECK(matches("dookie", row));       // only the album holds it
    CHECK(matches("DOOKIE burnout", row));
    CHECK_FALSE(matches("dookie", scanned_deadbolt()));
    CHECK_FALSE(matches("green deadbolt", row));
    CHECK_FALSE(matches("thrice burnout", row));
}

TEST_CASE("library query: a quoted phrase must appear whole in one field") {
    const LibraryQuery q = parse_library_query("\"tier 4\"");
    REQUIRE(q.terms.size() == 1);
    CHECK(q.terms[0].phrase);
    CHECK(q.terms[0].field == QueryField::Any);
    CHECK(q.terms[0].folded == "tier 4");

    const RowFacts none;
    CHECK(query_matches(q, scanned_burnout(), none));
    // Everlong sits in IB24's "T4", which is not "tier 4".
    CHECK_FALSE(query_matches(q, scanned_row("Foo Fighters - Everlong"), none));

    // As two words, "tier" and "4" may sit apart. As a phrase they may not:
    // "I Am... All Of Me (Crush 40)" has a 4 in its title and sits in Tier 5.
    const SearchableRow apart = scanned_row("HopH2O - I Am... All Of Me (Crush 40)");
    CHECK(matches("tier 4", apart));
    CHECK_FALSE(matches("\"tier 4\"", apart));
    // A phrase can't be split across two fields either.
    CHECK_FALSE(matches("\"burnout green\"", scanned_burnout()));
    // A quote that is never closed runs to the end.
    CHECK(matches("\"summer blast", scanned_burnout()));
}

TEST_CASE("library query: artist: charter: folder: and title: limit a term to one field") {
    CHECK(matches("artist:thrice", scanned_deadbolt()));
    CHECK_FALSE(matches("artist:thrice", scanned_burnout()));
    CHECK_FALSE(matches("title:thrice", scanned_deadbolt()));
    CHECK(matches("album:dookie", scanned_burnout()));
    CHECK(matches("album:\"illusion of safety\"", scanned_deadbolt()));
    CHECK_FALSE(matches("album:dookie", scanned_deadbolt()));
    CHECK_FALSE(matches("title:dookie", scanned_burnout()));
    // album: looks in the album only, not at the same word in the title.
    CHECK_FALSE(matches("album:burnout", scanned_burnout()));
    CHECK(matches("charter:hoph2o", scanned_burnout()));
    CHECK(matches("charter:hoph2o", scanned_deadbolt()));
    CHECK_FALSE(matches("charter:hoph2o", scanned_acid_romance()));
    CHECK(matches("folder:ib24", scanned_deadbolt()));
    CHECK_FALSE(matches("folder:ib24", scanned_burnout()));
    CHECK(matches("title:burnout", scanned_burnout()));
    CHECK(matches("Artist:\"green day\"", scanned_burnout()));
    CHECK_FALSE(matches("title:\"green day\"", scanned_burnout()));

    const LibraryQuery album = parse_library_query("ALBUM:Dookie");
    REQUIRE(album.terms.size() == 1);
    CHECK(album.terms[0].field == QueryField::Album);
    CHECK(album.terms[0].folded == "dookie");

    const LibraryQuery q = parse_library_query("ARTIST:\"Green Day\" folder:tier");
    REQUIRE(q.terms.size() == 2);
    CHECK(q.terms[0].field == QueryField::Artist);
    CHECK(q.terms[0].folded == "green day");
    CHECK(q.terms[0].phrase);
    CHECK(q.terms[1].field == QueryField::Folder);
    CHECK(q.terms[1].folded == "tier");
    CHECK_FALSE(q.terms[1].phrase);
}

TEST_CASE("library query: a term applies to its own column, and Any applies everywhere") {
    CHECK(term_applies_to(QueryField::Any, QueryField::Title));
    CHECK(term_applies_to(QueryField::Title, QueryField::Title));
    CHECK_FALSE(term_applies_to(QueryField::Title, QueryField::Artist));
    CHECK(term_applies_to(QueryField::Album, QueryField::Album));
    CHECK(term_applies_to(QueryField::Any, QueryField::Album));
    CHECK_FALSE(term_applies_to(QueryField::Album, QueryField::Artist));
    CHECK_FALSE(term_applies_to(QueryField::Artist, QueryField::Album));
    // A column of Any counts every term (match_spans' rule).
    CHECK(term_applies_to(QueryField::Title, QueryField::Any));
    CHECK(term_applies_to(QueryField::Any, QueryField::Any));
}

TEST_CASE("library query: unknown field names are words and empty field values are dropped") {
    const LibraryQuery unknown = parse_library_query("genre:rock");
    REQUIRE(unknown.terms.size() == 1);
    CHECK(unknown.terms[0].field == QueryField::Any);
    CHECK(unknown.terms[0].folded == "genre:rock");
    CHECK(unknown.errors.empty());

    // A field name with nothing after it is dropped, so the list doesn't
    // empty out while the user is still typing.
    const LibraryQuery typing = parse_library_query("artist:");
    CHECK(typing.terms.empty());
    CHECK(typing.empty());
}

namespace {

// The squeeze filters at and beside one chart's own shown timing. A path that
// needs no timing passes any limit and meets no floor; a timed path sits
// inside <=, >= and outside <, > at its own figure, and 1 ms past it on
// either side is outside the other way.
void check_squeeze_edges(const ChartFacts& c) {
    INFO("chart: " << c.chart);
    REQUIRE(c.facts.stars.has_value());
    if (!c.facts.hardest_ms) {
        for (const char* limit : {"squeeze<=0", "squeeze<0", "squeeze<=-5", "squeeze<20"})
            CHECK(matches(limit, c.row, c.facts));
        for (const char* floor : {"squeeze>=0", "squeeze>0", "squeeze>-5", "squeeze>=-5"})
            CHECK_FALSE(matches(floor, c.row, c.facts));
        return;
    }
    const double ms = *c.facts.hardest_ms;
    const std::string at = ms_text(ms);
    const std::string above = ms_text(ms + 1.0);
    const std::string below = ms_text(ms - 1.0);
    CHECK(matches("squeeze<=" + at, c.row, c.facts));
    CHECK_FALSE(matches("squeeze<" + at, c.row, c.facts));
    CHECK(matches("squeeze>=" + at, c.row, c.facts));
    CHECK_FALSE(matches("squeeze>" + at, c.row, c.facts));
    CHECK(matches("squeeze<" + above, c.row, c.facts));
    CHECK_FALSE(matches("squeeze<=" + below, c.row, c.facts));
    CHECK(matches("squeeze>" + below, c.row, c.facts));
    CHECK_FALSE(matches("squeeze>=" + above, c.row, c.facts));
}

}  // namespace

TEST_CASE("library query: stars:N keeps analyzed rows with exactly N stars") {
    const ChartFacts& c = burnout_facts();
    REQUIRE(c.facts.stars.has_value());
    const int stars = *c.facts.stars;
    const std::string own = "stars:" + std::to_string(stars);
    const LibraryQuery q = parse_library_query(own);
    REQUIRE(q.stars.has_value());
    CHECK(*q.stars == stars);
    CHECK(q.errors.empty());
    CHECK(query_matches(q, c.row, c.facts));
    CHECK_FALSE(query_matches(q, c.row, RowFacts{}));  // not analyzed
    // Every other count the filter takes leaves the row out.
    for (int n = 0; n <= hydra::kMaxStars; ++n) {
        if (n == stars) continue;
        CAPTURE(n);
        CHECK_FALSE(matches("stars:" + std::to_string(n), c.row, c.facts));
    }
    CHECK(matches("green " + own, c.row, c.facts));
    CHECK_FALSE(matches("thrice " + own, c.row, c.facts));
}

TEST_CASE("library query: squeeze<=N keeps analyzed rows whose hardest squeeze is at most N ms") {
    // A timed chart (its best path's figure is exactly 0.0 ms) and Burnout.
    for (const ChartFacts* c : {&on_end_facts(), &burnout_facts()}) {
        INFO("chart: " << c->chart);
        if (!c->facts.hardest_ms) continue;  // the untimed case is the next block's
        const double ms = *c->facts.hardest_ms;
        const LibraryQuery q = parse_library_query("squeeze<=" + ms_text(ms));
        REQUIRE(q.squeeze_max_ms.has_value());
        CHECK(*q.squeeze_max_ms == ms);
        CHECK_FALSE(q.squeeze_max_strict);
        CHECK(query_matches(q, c->row, c->facts));
        CHECK(matches("SQUEEZE<=" + ms_text(ms + 1.0), c->row, c->facts));
        CHECK_FALSE(matches("squeeze<=" + ms_text(ms - 1.0), c->row, c->facts));
        CHECK_FALSE(query_matches(q, c->row, RowFacts{}));  // not analyzed
    }
    // A path that needs no timing passes any limit.
    const ChartFacts& untimed = untimed_facts();
    REQUIRE_FALSE(untimed.facts.hardest_ms.has_value());
    CHECK(matches("squeeze<=0", untimed.row, untimed.facts));
    CHECK(matches("squeeze<=-5", untimed.row, untimed.facts));
    CHECK_FALSE(matches("squeeze<=0", untimed.row, RowFacts{}));  // not analyzed
}

TEST_CASE("library query: a row at exactly N ms matches <=N and >=N, not <N or >N") {
    CHECK(parse_library_query("squeeze<20").squeeze_max_strict);
    CHECK_FALSE(parse_library_query("squeeze<=20").squeeze_max_strict);
    CHECK(parse_library_query("squeeze>20").squeeze_min_strict);
    CHECK_FALSE(parse_library_query("squeeze>=20").squeeze_min_strict);

    const ChartFacts& on_end = on_end_facts();
    REQUIRE(on_end.facts.hardest_ms.has_value());
    const ChartFacts& untimed = untimed_facts();
    REQUIRE_FALSE(untimed.facts.hardest_ms.has_value());
    for (const ChartFacts* c : {&on_end, &untimed, &burnout_facts()}) check_squeeze_edges(*c);

    // squeeze<N is the exact opposite of squeeze>=N: a timed row matches one,
    // wherever N sits against its figure.
    const double ms = *on_end.facts.hardest_ms;
    for (double n : {ms - 1.0, ms, ms + 1.0}) {
        const std::string text = ms_text(n);
        CAPTURE(text);
        CHECK(matches("squeeze<" + text, on_end.row, on_end.facts) !=
              matches("squeeze>=" + text, on_end.row, on_end.facts));
    }
}

TEST_CASE("squeeze filters: a path whose hardest is 0.0 passes squeeze<=0, matches squeeze>=0, "
          "fails squeeze<0 and squeeze<=-5") {
    // A chart whose best path's engine figure is exactly 0.0 ms: the library
    // shows it as a timing (shown_timing), so the strict < drops it at 0.
    const ChartFacts& c = on_end_facts();
    REQUIRE(c.facts.hardest_ms == std::optional<double>(0.0));
    CHECK(matches("squeeze<=0", c.row, c.facts));
    CHECK(matches("squeeze>=0", c.row, c.facts));
    CHECK_FALSE(matches("squeeze<0", c.row, c.facts));
    CHECK_FALSE(matches("squeeze<=-5", c.row, c.facts));
}

TEST_CASE("library query: squeeze>N and squeeze>=N keep rows whose hardest timing is above N ms") {
    // A row at exactly N ms matches >=N and not >N.
    CHECK(matches("squeeze>=163", scanned_burnout(), analyzed(7, 163.0)));  // Burnout's real 163.0 ms
    CHECK_FALSE(matches("squeeze>163", scanned_burnout(), analyzed(7, 163.0)));
    CHECK(matches("squeeze>20", scanned_burnout(), analyzed(7, 163.0)));
    CHECK_FALSE(matches("squeeze>=164", scanned_burnout(), analyzed(7, 163.0)));
    CHECK(matches("SQUEEZE>20", scanned_burnout(), analyzed(7, 163.0)));
    CHECK(matches("squeeze>-5", scanned_burnout(), analyzed(7, -2.5)));

    // squeeze>N is the exact opposite of squeeze<=N: a timed row matches one.
    for (double ms : {12.5, 20.0, 163.0}) {
        CHECK(matches("squeeze>20", scanned_burnout(), analyzed(7, ms)) !=
              matches("squeeze<=20", scanned_burnout(), analyzed(7, ms)));
    }

    // A path that needs no timing has nothing to squeeze, so it matches no
    // > or >= filter. An unrated row matches no filter.
    CHECK_FALSE(matches("squeeze>20", scanned_burnout(), analyzed(7, std::nullopt)));
    CHECK_FALSE(matches("squeeze>=0", scanned_burnout(), analyzed(7, std::nullopt)));
    CHECK_FALSE(matches("squeeze>20", scanned_burnout(), RowFacts{}));
    CHECK_FALSE(matches("squeeze>=20", scanned_burnout(), RowFacts{}));

    const LibraryQuery q = parse_library_query("squeeze>20");
    CHECK(q.terms.empty());
    CHECK(q.errors.empty());
    CHECK_FALSE(q.empty());
    CHECK(parse_library_query("squeeze>=20").terms.empty());
}

TEST_CASE("library query: a squeeze floor and limit combine into a range, last of each side counts") {
    CHECK(matches("squeeze>=10 squeeze<=200", scanned_burnout(), analyzed(7, 163.0)));
    CHECK_FALSE(matches("squeeze>=10 squeeze<=50", scanned_burnout(), analyzed(7, 163.0)));
    CHECK_FALSE(matches("squeeze>=170 squeeze<=200", scanned_burnout(), analyzed(7, 163.0)));
    CHECK(matches("squeeze<=200 squeeze>163 green", scanned_burnout(), analyzed(7, 163.5)));

    // The same side twice: the last one counts.
    CHECK(matches("squeeze>200 squeeze>20", scanned_burnout(), analyzed(7, 163.0)));
    CHECK_FALSE(matches("squeeze>20 squeeze>200", scanned_burnout(), analyzed(7, 163.0)));
    CHECK(matches("squeeze>163 squeeze>=163", scanned_burnout(), analyzed(7, 163.0)));
    CHECK_FALSE(matches("squeeze>=163 squeeze>163", scanned_burnout(), analyzed(7, 163.0)));
}

TEST_CASE("library query: an unknown squeeze operator gives a plain error and filters nothing") {
    for (const char* text :
         {"squeeze=20", "squeeze==20", "squeeze:20", "squeeze=>20", "squeeze<>20", "SQUEEZE=20"}) {
        CAPTURE(text);
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.terms.empty());
        CHECK(q.errors ==
              std::vector<std::string>{"squeeze takes <=, <, >= or >, like squeeze<=20"});
        CHECK(query_matches(q, scanned_burnout(), RowFacts{}));
    }

    // Words that only start with "squeeze" still search as text.
    for (const char* text : {"squeeze", "squeezed", "squeeze!"}) {
        CAPTURE(text);
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.errors.empty());
        REQUIRE(q.terms.size() == 1);
        CHECK(q.terms[0].folded == fold_for_search(text));
    }
    CHECK(matches("squeeze", make_searchable("Squeeze Me", "", "", ""), RowFacts{}));
}

TEST_CASE("library query: bad filter values give a plain error and filter nothing") {
    const LibraryQuery stars = parse_library_query("burnout stars:9");
    CHECK_FALSE(stars.stars.has_value());
    REQUIRE(stars.errors.size() == 1);
    CHECK(stars.errors[0] == "stars: needs a number from 0 to 7");
    REQUIRE(stars.terms.size() == 1);  // the rest of the query still works
    CHECK(stars.terms[0].folded == "burnout");
    CHECK(query_matches(stars, scanned_burnout(), RowFacts{}));

    CHECK(parse_library_query("stars:x").errors ==
          std::vector<std::string>{"stars: needs a number from 0 to 7"});
    CHECK(parse_library_query("stars:-1").errors.size() == 1);
    CHECK(parse_library_query("stars:").errors.size() == 1);

    const LibraryQuery squeeze = parse_library_query("squeeze<=abc");
    CHECK_FALSE(squeeze.squeeze_max_ms.has_value());
    REQUIRE(squeeze.errors.size() == 1);
    CHECK(squeeze.errors[0] == "squeeze<= needs a number of milliseconds, like squeeze<=20");
    CHECK(parse_library_query("squeeze<=").errors.size() == 1);
    CHECK(parse_library_query("squeeze<=20ms").errors.size() == 1);

    // The message names the operator the user typed.
    CHECK(parse_library_query("squeeze>abc").errors ==
          std::vector<std::string>{"squeeze> needs a number of milliseconds, like squeeze>20"});
    CHECK(parse_library_query("squeeze>=").errors ==
          std::vector<std::string>{"squeeze>= needs a number of milliseconds, like squeeze>=20"});
    CHECK(parse_library_query("squeeze<x").errors ==
          std::vector<std::string>{"squeeze< needs a number of milliseconds, like squeeze<20"});

    // The same mistake twice is reported once.
    CHECK(parse_library_query("stars:9 stars:10").errors.size() == 1);
}

TEST_CASE("library query: an empty query matches every row") {
    for (std::string_view text : {"", "   ", "\"\"", "\" \"", "title:"}) {
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.empty());
        CHECK(q.errors.empty());
        CHECK(query_matches(q, scanned_burnout(), RowFacts{}));
        CHECK(query_matches(q, scanned_acid_romance(), analyzed(3, 40.0)));
    }
    CHECK_FALSE(parse_library_query("x").empty());
    CHECK_FALSE(parse_library_query("stars:7").empty());
    CHECK_FALSE(parse_library_query("squeeze<=20").empty());
}

TEST_CASE("library query: highlight spans cover the displayed bytes") {
    // "é" is two bytes, so the span runs to byte 14, not 13.
    const std::string title = "Halo (Beyoncé cover)";  // made up
    CHECK(spans_equal(match_spans(parse_library_query("beyonce"), QueryField::Title, title),
                      std::vector<MatchSpan>{MatchSpan{6, 14}}));
    CHECK(title.substr(6, 8) == "Beyoncé");

    // The caller passes the tag-stripped charter, as the table shows it.
    const std::string charter = strip_rich_tags("<color=#e02222>Blood</color>line");
    CHECK(spans_equal(match_spans(parse_library_query("bloodline"), QueryField::Charter, charter),
                      std::vector<MatchSpan>{MatchSpan{0, 9}}));

    // A phrase in the folder, despite the capital T.
    const std::string folder = "common\\Summer Blast _25 Setlist\\Tier 4";
    CHECK(spans_equal(match_spans(parse_library_query("\"tier 4\""), QueryField::Folder, folder),
                      std::vector<MatchSpan>{MatchSpan{32, 38}}));

    // Two spaces fold to one, and the span still covers both.
    CHECK(spans_equal(match_spans(parse_library_query("\"tier 4\""), QueryField::Folder, "Tier  4"),
                      std::vector<MatchSpan>{MatchSpan{0, 7}}));

    // Every occurrence lights up.
    CHECK(spans_equal(match_spans(parse_library_query("tier"), QueryField::Folder, "Tier 1\\Tier 2"),
                      std::vector<MatchSpan>{MatchSpan{0, 4}, MatchSpan{7, 11}}));

    // Touching matches merge. A field-limited term lights up only its field.
    const LibraryQuery words = parse_library_query("burn out artist:green");
    CHECK(spans_equal(match_spans(words, QueryField::Title, "Burnout"),
                      std::vector<MatchSpan>{MatchSpan{0, 7}}));
    CHECK(spans_equal(match_spans(words, QueryField::Artist, "Green Day"),
                      std::vector<MatchSpan>{MatchSpan{0, 5}}));
    CHECK(match_spans(parse_library_query("artist:green"), QueryField::Title, "Green Light").empty());

    // An album lights up for a plain word and for album:, and not for artist:.
    const std::string album = "The Illusion of Safety";
    CHECK(spans_equal(match_spans(parse_library_query("illusion"), QueryField::Album, album),
                      std::vector<MatchSpan>{MatchSpan{4, 12}}));
    CHECK(spans_equal(match_spans(parse_library_query("album:safety"), QueryField::Album, album),
                      std::vector<MatchSpan>{MatchSpan{16, 22}}));
    CHECK(match_spans(parse_library_query("artist:safety"), QueryField::Album, album).empty());
}

TEST_CASE("library query: a chart with no album matches nothing through its album") {
    const SearchableRow row = make_searchable("Song", "Band", "Charter", "Pack");  // made up
    CHECK(row.album.empty());
    CHECK_FALSE(matches("album:song", row));
    CHECK_FALSE(matches("album:a", row));
    CHECK(matches("song", row));  // the title still matches
    CHECK(make_searchable("Song", "Band", "Charter", "Pack", "").album.empty());
    CHECK(match_spans(parse_library_query("song"), QueryField::Album, "").empty());
    CHECK(match_spans(parse_library_query("album:song"), QueryField::Album, "").empty());
}

TEST_CASE("library query: the report pages' search text holds no album") {
    // The report pages call search_field, which the album leaves unchanged.
    CHECK(search_field("Burnout", "Green Day", "<b>Hoph2o</b>", "1 2 3") ==
          "burnout green day hoph2o 1 2 3");
    CHECK(search_field("Deadbolt", "Thrice", "") == "deadbolt thrice");
}

TEST_CASE("library query: every Search tips example parses into the key its entry names") {
    const std::vector<SearchTip>& tips = search_tips();
    std::vector<std::string_view> examples;
    for (const SearchTip& tip : tips) {
        examples.push_back(tip.example);
        CAPTURE(tip.example);
        CHECK_FALSE(tip.meaning.empty());
        CHECK(parse_library_query(tip.example).errors.empty());
    }
    // The order the popup lists them in (D263 call 10).
    CHECK(examples == std::vector<std::string_view>{"title:actors", "artist:green", "album:dookie",
                                                    "charter:boo", "folder:ib24", "\"tier 4\"",
                                                    "stars:7", "squeeze<=20", "squeeze>20"});

    // Each example yields one term in the field it names, or the filter it names.
    struct FieldTip {
        size_t index;
        QueryField field;
        const char* folded;
        bool phrase;
    };
    for (const FieldTip& want : {FieldTip{0, QueryField::Title, "actors", false},
                                 FieldTip{1, QueryField::Artist, "green", false},
                                 FieldTip{2, QueryField::Album, "dookie", false},
                                 FieldTip{3, QueryField::Charter, "boo", false},
                                 FieldTip{4, QueryField::Folder, "ib24", false},
                                 FieldTip{5, QueryField::Any, "tier 4", true}}) {
        REQUIRE(want.index < tips.size());
        CAPTURE(tips[want.index].example);
        const LibraryQuery q = parse_library_query(tips[want.index].example);
        REQUIRE(q.terms.size() == 1);
        CHECK(q.terms[0].field == want.field);
        CHECK(q.terms[0].folded == want.folded);
        CHECK(q.terms[0].phrase == want.phrase);
        CHECK_FALSE(q.stars.has_value());
        CHECK_FALSE(q.squeeze_max_ms.has_value());
        CHECK_FALSE(q.squeeze_min_ms.has_value());
    }

    REQUIRE(tips.size() == 9);
    const LibraryQuery stars = parse_library_query(tips[6].example);
    CHECK(stars.terms.empty());
    CHECK(stars.stars == std::optional<int>(7));
    CHECK_FALSE(stars.squeeze_max_ms.has_value());
    CHECK_FALSE(stars.squeeze_min_ms.has_value());

    const LibraryQuery at_most = parse_library_query(tips[7].example);
    CHECK(at_most.terms.empty());
    CHECK(at_most.squeeze_max_ms == std::optional<double>(20.0));
    CHECK_FALSE(at_most.squeeze_max_strict);
    CHECK_FALSE(at_most.squeeze_min_ms.has_value());
    CHECK_FALSE(at_most.stars.has_value());

    const LibraryQuery above = parse_library_query(tips[8].example);
    CHECK(above.terms.empty());
    CHECK(above.squeeze_min_ms == std::optional<double>(20.0));
    CHECK(above.squeeze_min_strict);
    CHECK_FALSE(above.squeeze_max_ms.has_value());
    CHECK_FALSE(above.stars.has_value());

    // The fixture rows are the tips' own charts, so the examples find them.
    CHECK(matches(tips[1].example, scanned_burnout()));
    CHECK(matches(tips[2].example, scanned_burnout()));
    CHECK(matches(tips[4].example, scanned_deadbolt()));
    CHECK(matches(tips[5].example, scanned_burnout()));

    CHECK_FALSE(search_tips_note().empty());
}

TEST_CASE("library query: matching 20000 rows finds the pinned hit count") {
    std::vector<SearchableRow> rows;  // made up, library-shaped
    rows.reserve(20000);
    for (int i = 0; i < 20000; ++i) {
        rows.push_back(make_searchable(
            "Song number " + std::to_string(i), "Artist " + std::to_string(i % 500),
            "Charter " + std::to_string(i % 50),
            "common\\Pack " + std::to_string(i % 40) + "\\Tier " + std::to_string(i % 8)));
    }
    const LibraryQuery q = parse_library_query("artist 12 \"tier 3\"");
    const RowFacts none;

    // The time is printed for a person to read, not checked: a busy machine
    // can fail any wall-clock limit.
    const auto start = std::chrono::steady_clock::now();
    size_t hits = 0;
    for (const SearchableRow& row : rows)
        if (query_matches(q, row, none)) ++hits;
    const std::chrono::duration<double, std::milli> took = std::chrono::steady_clock::now() - start;
    MESSAGE("matching 20000 rows: " << took.count() << " ms, " << hits << " hits");
    // Pinned from one run.
    CHECK(hits == 189);
}
