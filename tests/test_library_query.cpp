// Unit tests for app/library_query: how the library search folds text,
// strips Clone Hero's rich-text tags, reads a query, matches a row and marks
// what to highlight. Named rows are the corpus scan's own rows from
// testdata\input, and a row's stars and timing come from analysing its chart.

#include "doctest.h"

#include <algorithm>
#include <cctype>
#include <charconv>
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

// The analysed corpus charts whose best path the library shows with a timing:
// the 0.0 ms chart, and Burnout when its path is timed. Throws when the 0.0 ms
// chart has no shown timing, so a test that loops over these never passes on
// no chart at all.
std::vector<const ChartFacts*> timed_facts() {
    if (!on_end_facts().facts.stars || !on_end_facts().facts.hardest_ms)
        throw std::runtime_error("the 0.0 ms chart has no shown timing: " + on_end_facts().chart);
    std::vector<const ChartFacts*> out{&on_end_facts()};
    if (burnout_facts().facts.stars && burnout_facts().facts.hardest_ms) out.push_back(&burnout_facts());
    return out;
}

// A number as the shortest text that reads back as the same double, so a
// filter typed with it sits exactly on that value.
std::string ms_text(double ms) {
    char buf[64];
    const std::to_chars_result r = std::to_chars(buf, buf + sizeof buf, ms);
    if (r.ec != std::errc{}) throw std::runtime_error("cannot write a number as text");
    return std::string(buf, r.ptr);
}

// The shown text each highlight span of `query` covers in `shown`, in order:
// what the library table lights up.
std::vector<std::string> lit_texts(std::string_view query, QueryField field,
                                   std::string_view shown) {
    std::vector<std::string> out;
    for (const MatchSpan& s : match_spans(parse_library_query(query), field, shown))
        out.emplace_back(shown.substr(s.begin, s.end - s.begin));
    return out;
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

TEST_CASE("library query: a match on any letter of a folded character lights the whole character") {
    // Each character the fold changes, shown alone, is searched by each
    // letter or digit of its folded form, as a player types it: "s" in "ß"
    // (folded "ss"), "e" in "É", "a" in a full-width "Ａ". The highlight is
    // the whole character, every byte of it, never part of one.
    const std::vector<FoldEntry> table = search_fold_table();
    REQUIRE_FALSE(table.empty());
    for (const FoldEntry& entry : table) {
        for (char letter : entry.to) {
            if (!std::isalnum(static_cast<unsigned char>(letter))) continue;
            const std::string query(1, letter);
            INFO("character: " << entry.from << ", typed: " << query);
            CHECK(lit_texts(query, QueryField::Title, entry.from) ==
                  std::vector<std::string>{entry.from});
        }
    }
    // A real name: the "a" typed for the ä of The Fall of Troy's album
    // "Doppelgänger" lights up the ä's two bytes. The folded album holds that
    // letter once, so the ä is the one span.
    const std::string album = scanned_in("The Fall of Troy - Act One, Scene One").album;
    const auto first_wide = std::find_if(album.begin(), album.end(),
                                         [](char c) { return static_cast<unsigned char>(c) >= 0x80; });
    REQUIRE(first_wide != album.end());
    const size_t begin = static_cast<size_t>(first_wide - album.begin());
    size_t end = begin + 1;
    while (end < album.size() && (static_cast<unsigned char>(album[end]) & 0xC0) == 0x80) ++end;
    const std::string accented = album.substr(begin, end - begin);
    const std::string typed = fold_for_search(accented);
    REQUIRE(typed.size() == 1);
    const std::string folded_album = fold_for_search(album);
    REQUIRE(folded_album.find(typed) != std::string::npos);
    REQUIRE(folded_album.find(typed) == folded_album.rfind(typed));
    CHECK(lit_texts(typed, QueryField::Album, album) == std::vector<std::string>{accented});
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

TEST_CASE("library query: the report pages' fold table removes accents, lowercases and lists no kept character") {
    // The report pages fold a typed query with this table. Each spot check is
    // one of the fold's rules (fold_for_search, library_query.h): accents are
    // removed, letters lowercased, full-width ASCII made ASCII, and a
    // character the fold keeps is not listed.
    const std::vector<FoldEntry> table = search_fold_table();
    REQUIRE_FALSE(table.empty());
    auto folded = [&](std::string_view from) -> std::optional<std::string> {
        for (const FoldEntry& e : table)
            if (e.from == from) return e.to;
        return std::nullopt;
    };
    CHECK(folded("\xc3\x89") == std::optional<std::string>("e"));  // É
    CHECK(folded("\xc3\x84") == std::optional<std::string>("a"));  // Ä
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

// An error sentence's last word is the example it offers the player. That
// example has to be a filter the parser takes: no error, no words, and a
// squeeze side set. Returns the example.
std::string check_example_parses(const std::string& sentence) {
    INFO("sentence: " << sentence);
    const size_t space = sentence.find_last_of(' ');
    REQUIRE(space != std::string::npos);
    const std::string example = sentence.substr(space + 1);
    INFO("example: " << example);
    const LibraryQuery q = parse_library_query(example);
    CHECK(q.errors.empty());
    CHECK(q.terms.empty());
    CHECK((q.squeeze_max_ms.has_value() || q.squeeze_min_ms.has_value()));
    return example;
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
    for (const ChartFacts* c : timed_facts()) {
        INFO("chart: " << c->chart);
        const double ms = *c->facts.hardest_ms;
        // A floor below the chart's own timing keeps it, in any letter case.
        CHECK(matches("SQUEEZE>" + ms_text(ms - 1.0), c->row, c->facts));
        // squeeze>N is the exact opposite of squeeze<=N: a timed row matches
        // one, wherever N sits against its figure.
        for (double n : {ms - 1.0, ms, ms + 1.0}) {
            const std::string text = ms_text(n);
            CAPTURE(text);
            CHECK(matches("squeeze>" + text, c->row, c->facts) !=
                  matches("squeeze<=" + text, c->row, c->facts));
        }
        // An unrated row matches no floor.
        CHECK_FALSE(matches("squeeze>" + ms_text(ms - 1.0), c->row, RowFacts{}));
        CHECK_FALSE(matches("squeeze>=" + ms_text(ms - 1.0), c->row, RowFacts{}));
    }

    // A path that needs no timing has nothing to squeeze, so it matches no
    // > or >= filter, however low.
    const ChartFacts& untimed = untimed_facts();
    REQUIRE(untimed.facts.stars.has_value());
    REQUIRE_FALSE(untimed.facts.hardest_ms.has_value());
    CHECK_FALSE(matches("squeeze>20", untimed.row, untimed.facts));
    CHECK_FALSE(matches("squeeze>=0", untimed.row, untimed.facts));
    CHECK_FALSE(matches("squeeze>-5", untimed.row, untimed.facts));

    const LibraryQuery q = parse_library_query("squeeze>20");
    CHECK(q.terms.empty());
    CHECK(q.errors.empty());
    CHECK_FALSE(q.empty());
    CHECK(parse_library_query("squeeze>=20").terms.empty());
}

TEST_CASE("library query: a squeeze floor and limit combine into a range, last of each side counts") {
    // Each chart's own shown timing, and 1 and 10 ms on either side of it.
    for (const ChartFacts* c : timed_facts()) {
        INFO("chart: " << c->chart);
        const double ms = *c->facts.hardest_ms;
        const std::string at = ms_text(ms);
        const std::string far_below = ms_text(ms - 10.0);
        const std::string below = ms_text(ms - 1.0);
        const std::string above = ms_text(ms + 1.0);
        const std::string far_above = ms_text(ms + 10.0);
        auto keeps = [&](const std::string& query) { return matches(query, c->row, c->facts); };

        // A range around the timing keeps the row; one wholly below or above it does not.
        CHECK(keeps("squeeze>=" + far_below + " squeeze<=" + far_above));
        CHECK_FALSE(keeps("squeeze>=" + far_below + " squeeze<=" + below));
        CHECK_FALSE(keeps("squeeze>=" + above + " squeeze<=" + far_above));

        // The same side twice: the last one counts (parse_library_query's rule).
        CHECK(keeps("squeeze>" + far_above + " squeeze>" + below));
        CHECK_FALSE(keeps("squeeze>" + below + " squeeze>" + far_above));
        CHECK(keeps("squeeze>" + at + " squeeze>=" + at));
        CHECK_FALSE(keeps("squeeze>=" + at + " squeeze>" + at));
        CHECK(keeps("squeeze<" + at + " squeeze<=" + at));
        CHECK_FALSE(keeps("squeeze<=" + at + " squeeze<" + at));
    }
}

TEST_CASE("library query: an unknown squeeze operator gives a plain error and filters nothing") {
    // Every wrong operator gets the same one sentence. It names squeeze and
    // the operators it does take, and the example it ends on is itself a
    // filter that parses.
    const std::vector<std::string> first = parse_library_query("squeeze=20").errors;
    REQUIRE(first.size() == 1);
    const std::string& sentence = first[0];
    CHECK(sentence.find("squeeze") != std::string::npos);
    for (const char* op : {"<=", ">=", "<", ">"}) {
        CAPTURE(op);
        CHECK(sentence.find(op) != std::string::npos);
    }
    check_example_parses(sentence);
    for (const char* text :
         {"squeeze=20", "squeeze==20", "squeeze:20", "squeeze=>20", "squeeze<>20", "SQUEEZE=20"}) {
        CAPTURE(text);
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.terms.empty());
        CHECK(q.errors == first);
        CHECK_FALSE(q.squeeze_max_ms.has_value());
        CHECK_FALSE(q.squeeze_min_ms.has_value());
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
}

TEST_CASE("library query: bad filter values give a plain error and filter nothing") {
    // A count past the top of the stars: filter (kMaxStars) is refused, and
    // the rest of the query still works.
    const std::string too_many = "stars:" + std::to_string(hydra::kMaxStars + 1);
    const LibraryQuery stars = parse_library_query("burnout " + too_many);
    CHECK_FALSE(stars.stars.has_value());
    REQUIRE(stars.errors.size() == 1);
    // The sentence names the filter and the top of its range.
    const std::string stars_sentence = stars.errors[0];
    CHECK(stars_sentence.find("stars:") == 0);
    CHECK(stars_sentence.find(std::to_string(hydra::kMaxStars)) != std::string::npos);
    REQUIRE(stars.terms.size() == 1);
    CHECK(stars.terms[0].folded == "burnout");
    CHECK(query_matches(stars, scanned_burnout(), RowFacts{}));

    // Every bad stars: value gets that same one sentence and sets no filter.
    for (const char* text : {"stars:x", "stars:-1", "stars:"}) {
        CAPTURE(text);
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.errors == std::vector<std::string>{stars_sentence});
        CHECK_FALSE(q.stars.has_value());
    }

    // A bad squeeze value gets one sentence that starts with the filter and
    // operator the user typed and ends on an example with that operator,
    // which itself parses.
    for (const char* op : {"<=", "<", ">=", ">"}) {
        const std::string filter = std::string("squeeze") + op;
        for (const std::string& value : {std::string("abc"), std::string(""), std::string("20ms")}) {
            const std::string text = filter + value;
            CAPTURE(text);
            const LibraryQuery q = parse_library_query(text);
            CHECK_FALSE(q.squeeze_max_ms.has_value());
            CHECK_FALSE(q.squeeze_min_ms.has_value());
            REQUIRE(q.errors.size() == 1);
            CHECK(q.errors[0].find(filter + " ") == 0);
            const std::string example = check_example_parses(q.errors[0]);
            CHECK(example.find(filter) == 0);
            const char after = example.size() > filter.size() ? example[filter.size()] : '\0';
            CHECK(after >= '0');
            CHECK(after <= '9');  // the operator is exactly the one typed, then a number
        }
    }

    // The same mistake twice is reported once.
    CHECK(parse_library_query(too_many + " " + too_many).errors.size() == 1);
}

TEST_CASE("library query: an empty query matches every row") {
    for (std::string_view text : {"", "   ", "\"\"", "\" \"", "title:"}) {
        const LibraryQuery q = parse_library_query(text);
        CHECK(q.empty());
        CHECK(q.errors.empty());
        // Rows with and without analysed facts, timed and untimed, all pass.
        CHECK(query_matches(q, scanned_acid_romance(), RowFacts{}));
        for (const ChartFacts* c : {&burnout_facts(), &on_end_facts(), &untimed_facts()}) {
            INFO("chart: " << c->chart);
            CHECK(query_matches(q, c->row, c->facts));
        }
    }
    CHECK_FALSE(parse_library_query("x").empty());
    CHECK_FALSE(parse_library_query("stars:7").empty());
    CHECK_FALSE(parse_library_query("squeeze<=20").empty());
}

TEST_CASE("library query: highlight spans cover the displayed bytes") {
    // Typed without its accent, The Fall of Troy's album "Doppelgänger" lights
    // up whole, the accented letter's two bytes included.
    const std::string accented = scanned_in("The Fall of Troy - Act One, Scene One").album;
    REQUIRE(std::any_of(accented.begin(), accented.end(),
                        [](char c) { return static_cast<unsigned char>(c) >= 0x80; }));
    CHECK(lit_texts(fold_for_search(accented), QueryField::Album, accented) ==
          std::vector<std::string>{accented});

    // The caller passes the tag-stripped charter, as the table shows it: Acid
    // Romance's tagged charter lights up whole by its plain name.
    const std::string charter = strip_rich_tags(scanned_in("Alpha Wolf - Acid Romance").charter);
    CHECK(lit_texts(fold_for_search(charter), QueryField::Charter, charter) ==
          std::vector<std::string>{charter});

    // A phrase in Burnout's folder lights up as shown, with its capital T.
    const hydra::store::ChartLibraryEntry& burnout = scanned_in("Green Day - Burnout");
    CHECK(lit_texts("\"tier 4\"", QueryField::Folder, burnout.rootfolder) ==
          std::vector<std::string>{"Tier 4"});

    // Every occurrence lights up: the s of Illusion and the S of Safety in
    // Deadbolt's album "The Illusion of Safety".
    const std::string album = scanned_in("Thrice - Deadbolt").album;
    CHECK(lit_texts("s", QueryField::Album, album) == std::vector<std::string>{"s", "S"});

    // Touching matches merge: "burn" and "out" light up Burnout as one span.
    CHECK(lit_texts("burn out artist:green", QueryField::Title, burnout.title) ==
          std::vector<std::string>{"Burnout"});
    // A field-limited term lights up only its field.
    CHECK(lit_texts("burn out artist:green", QueryField::Artist, burnout.artist) ==
          std::vector<std::string>{"Green"});
    CHECK(lit_texts("artist:burnout", QueryField::Title, burnout.title).empty());

    // An album lights up for a plain word and for album:, and not for artist:.
    CHECK(lit_texts("illusion", QueryField::Album, album) == std::vector<std::string>{"Illusion"});
    CHECK(lit_texts("album:safety", QueryField::Album, album) == std::vector<std::string>{"Safety"});
    CHECK(lit_texts("artist:safety", QueryField::Album, album).empty());
}

TEST_CASE("library query: a chart with no album matches nothing through its album") {
    // Pokemon Theme's song.ini has an empty album line and its notes.chart
    // names no album, so the scan gives it none.
    const hydra::store::ChartLibraryEntry& e = scanned_in("Jason Paige - Pokemon Theme");
    REQUIRE(e.album.empty());
    const SearchableRow row = searchable_of(e);
    CHECK(row.album.empty());
    REQUIRE_FALSE(row.title.empty());
    // No album: term finds it, not even its own title's words or one letter of them.
    CHECK_FALSE(matches("album:" + row.title, row));
    CHECK_FALSE(matches("album:" + row.title.substr(0, 1), row));
    CHECK(matches(row.title, row));  // the title still matches as a plain word
    CHECK(match_spans(parse_library_query(row.title), QueryField::Album, e.album).empty());
    CHECK(match_spans(parse_library_query("album:" + row.title), QueryField::Album, e.album).empty());
}

TEST_CASE("library query: the report pages' search text holds no album") {
    // Burnout's album is a word no other field of its row holds ("dookie"
    // finds it only through the album, as the test above shows).
    const hydra::store::ChartLibraryEntry& e = scanned_in("Green Day - Burnout");
    const SearchableRow row = searchable_of(e);
    REQUIRE_FALSE(row.album.empty());
    REQUIRE_FALSE(row.charter.empty());
    REQUIRE_FALSE(row.folder.empty());
    const std::string text = search_field(e.title, e.artist, e.charter, e.rootfolder);
    CHECK(text.find(row.album) == std::string::npos);
    // Every other field, folded as the library folds it, joined by single spaces.
    CHECK(text == row.title + " " + row.artist + " " + row.charter + " " + row.folder);
    // An empty field is left out, not joined as a blank.
    CHECK(search_field(e.title, e.artist, "") == row.title + " " + row.artist);

    // A tagged charter goes in as its plain name (Acid Romance's song.ini).
    const hydra::store::ChartLibraryEntry& tagged = scanned_in("Alpha Wolf - Acid Romance");
    REQUIRE(tagged.charter.find('<') != std::string::npos);
    const std::string tagged_text = search_field(tagged.title, tagged.artist, tagged.charter);
    CHECK(tagged_text.find('<') == std::string::npos);
    CHECK(tagged_text.find(searchable_of(tagged).charter) != std::string::npos);
}

TEST_CASE("library query: every Search tips example parses into the key its entry names") {
    // The popup draws search_tips() and writes no key of its own, so between
    // them the tips must cover every key parse_library_query reads.
    std::set<QueryField> fields;
    bool has_phrase = false, has_stars = false, has_limit = false, has_floor = false;
    for (const SearchTip& tip : search_tips()) {
        const std::string example(tip.example);
        const std::string meaning(tip.meaning);
        CAPTURE(example);
        const LibraryQuery q = parse_library_query(example);
        CHECK(q.errors.empty());
        // Each example sets exactly one key: one term or one filter.
        const size_t keys = q.terms.size() + (q.stars ? 1u : 0u) + (q.squeeze_max_ms ? 1u : 0u) +
                            (q.squeeze_min_ms ? 1u : 0u);
        REQUIRE(keys == 1);
        if (!q.terms.empty()) {
            const QueryTerm& term = q.terms[0];
            const size_t colon = example.find(':');
            // The term searches for the example's own text after its key,
            // without its quotes.
            std::string value = colon == std::string::npos ? example : example.substr(colon + 1);
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
                value = value.substr(1, value.size() - 2);
            CHECK(term.folded == fold_for_search(value));
            CHECK(term.phrase == (example.find('"') != std::string::npos));
            if (term.field == QueryField::Any) {
                CHECK(colon == std::string::npos);
            } else {
                REQUIRE(colon != std::string::npos);
                // The meaning names the field the example's key limits it to.
                CHECK(meaning.find("in the " + example.substr(0, colon)) != std::string::npos);
            }
            fields.insert(term.field);
            has_phrase = has_phrase || term.phrase;
            // A player can try it on the test library: a scanned corpus row matches it.
            const std::vector<hydra::store::ChartLibraryEntry>& rows = corpus_scan_entries();
            CHECK(std::any_of(rows.begin(), rows.end(), [&](const hydra::store::ChartLibraryEntry& e) {
                return query_matches(q, searchable_of(e), RowFacts{});
            }));
        } else if (q.stars) {
            // The meaning names the star count the filter keeps.
            CHECK(meaning.find(std::to_string(*q.stars)) != std::string::npos);
            has_stars = true;
        } else {
            // The meaning names the milliseconds the filter compares with.
            const double ms = q.squeeze_max_ms ? *q.squeeze_max_ms : *q.squeeze_min_ms;
            CHECK(meaning.find(ms_text(ms) + " ms") != std::string::npos);
            has_limit = has_limit || q.squeeze_max_ms.has_value();
            has_floor = has_floor || q.squeeze_min_ms.has_value();
        }
    }
    for (QueryField f : {QueryField::Title, QueryField::Artist, QueryField::Album,
                         QueryField::Charter, QueryField::Folder}) {
        CAPTURE(static_cast<int>(f));
        CHECK(fields.count(f) == 1);
    }
    CHECK(has_phrase);
    CHECK(has_stars);
    CHECK(has_limit);
    CHECK(has_floor);

    CHECK_FALSE(search_tips_note().empty());
}
