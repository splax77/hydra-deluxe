#include "app/fill_report.h"

#include <cctype>
#include <map>
#include <unordered_map>
#include <utility>

#include "app/config.h"  // Settings::profile_with_fills
#include "app/html_page.h"
#include "app/library_query.h"  // search_field
#include "app/report.h"  // listed_records, lacks_chart_library, nothing_under_settings, kDash
#include "app/report_view.h"  // folded_key
#include "core/model.h"  // group_thousands, counted
#include "core/strutil.h"  // replace_all
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "search/graph.h"  // fill_rule_name, fill_rule_description
#include "store/chart_index.h"

namespace hydra::app::fill_report {

using html::json_escape_into;

namespace {

// This comparison page's own pieces. The stylesheet and the script that sorts,
// filters and draws the table are shared with the other report pages
// (html::page_template, docs/adr/0016). __SUBTITLE__/__FOOTER__/__DATA__ are
// filled by build_fill_html.
//
// Every literal here is ASCII: this file compiles into hydra_core, which is
// not built with /utf-8. Glyphs the page needs go in as HTML entities (markup)
// or \uXXXX escapes (JavaScript).
//
// __OLD_RULE__ and __NEW_RULE__ are the two fill rules' short names, filled
// from fill_rule_name when the page shell is built.
const char* const kTitle = "Fill spawn comparison &mdash; __OLD_RULE__ vs __NEW_RULE__";

const char* const kBody = R"page(<div class="wrap fill">
  <header>
    <h1>Fill spawn <span class="accent">__OLD_RULE__ vs __NEW_RULE__</span></h1>
    <div class="sub">__SUBTITLE__</div>
  </header>

  <div class="stats" id="stats"></div>

  <div class="controls">
    <span class="sorter">
      <label for="sortby">Sort by</label>
      <select id="sortby"></select>
      <button id="sortdir" type="button" title="Switch between highest-first and lowest-first"></button>
    </span>
    <input type="search" id="q" aria-label="Search charts" placeholder="__SEARCH_HINT__">
    <select id="status" aria-label="Status">
      <option value="">All charts</option>
      <option value="__ST_NEW_HIGHER__">1.1 higher</option>
      <option value="__ST_OLD_HIGHER__">1.0 higher</option>
      <option value="__ST_SAME__">Same score</option>
      <option value="__ST_ONLY_OLD__">Only in 1.0 db</option>
      <option value="__ST_ONLY_NEW__">Only in 1.1 db</option>
      <option value="__ST_IN_BOTH__">In both</option>
    </select>
    <span class="count" id="count"></span>
  </div>

  <div class="tablewrap">
    <table>
      <thead><tr id="head"></tr></thead>
      <tbody id="body"></tbody>
    </table>
    <div class="empty" id="empty">Joining databases&hellip;</div>
  </div>

  <footer>__FOOTER__</footer>
</div>

)page";

const char* const kPageJs = R"page(// STATUS: fill_report.h's kStatus* words, written in by page_template.
const STATUS = __STATUS__;
const STATUS_CLASS = {[STATUS.newHigher]:'s-newhigh', [STATUS.oldHigher]:'s-oldhigh',
                      [STATUS.sameScore]:'s-same', [STATUS.onlyOld]:'s-only',
                      [STATUS.onlyNew]:'s-only', [STATUS.inBoth]:'s-only'};

// A chart in both databases whose record on one side kept no path reads "no
// paths" on that side (D51 call 11); any other missing score reads as a dash.
function scoreText(r, s) {
  return s === null && r.status === STATUS.inBoth ? 'no paths' : fmt(s);
}

// How many charts a set of rows counts as: each row's library copies, "k", as
// tally_fill_rows adds them up in fill_report.cpp. The count beside the
// filters and every chart tile read this; the test "fill page: the tiles add
// up the copies tally_fill_rows adds up" checks the two agree.
const charts = rs => rs.reduce((a, r) => a + r.k, 0);

const PAGE = {
  rows: DATA,
  // Every row's copies with their noun, from counted: "1 chart", "2 charts".
  total: __COUNT_TOTAL__,
  count: charts,
  sortKey: 'delta',
  sortDir: -1,
  cols: [
    {k:'song',    t:'Song',        num:false, sort:'song_s'},
    {k:'artist',  t:'Artist',      num:false, sort:'artist_s'},
    {k:'charter', t:'Charter',     num:false, sort:'charter_s'},
    {k:'s10',     t:'__OLD_RULE__',      num:true},
    {k:'s11',     t:'__NEW_RULE__',      num:true},
    {k:'delta',   t:'Delta',       num:true},
    {k:'p10',     t:'__OLD_RULE__ path', num:false, sort:'p10_s'},
    {k:'p11',     t:'__NEW_RULE__ path', num:false, sort:'p11_s'},
    {k:'acts',    t:'Acts',        num:true},
    {k:'notes',   t:'Note count',  num:true},
    {k:'status',  t:'Status',      num:false, sort:'status_s'},
  ],
  controls: [['q', 'input'], ['status', 'change']],
  // The search box is matched against each row's search text in the shared
  // script; this keeps rows by the status control.
  filter() {
    const status = document.getElementById('status').value;
    return r => !status || r.status === status;
  },
  cells(r) {
    const hasDelta = r.delta !== null && r.delta !== undefined;
    const deltaCls = !hasDelta ? 'num dim' : (r.status === STATUS.newHigher ? 'num pos'
                   : (r.status === STATUS.oldHigher ? 'num neg' : 'num dim'));
    const deltaTxt = !hasDelta ? DASH
                   : (r.status === STATUS.newHigher ? '+' : '') + fmt(r.delta);
    return [
      ['song trunc', r.song],
      ['dim trunc artist', r.artist],
      ['dim trunc charter', r.charter],
      ['num', scoreText(r, r.s10)],
      ['num', scoreText(r, r.s11)],
      [deltaCls, deltaTxt],
      ['path trunc', r.p10 || DASH],
      ['path trunc', r.p11 || DASH],
      ['num', r.acts_txt],
      ['num', fmt(r.notes)],
      ['chip ' + (STATUS_CLASS[r.status] || 's-only'), r.status, 'chip'],
    ];
  },
  stats(rows) {
    const n = s => charts(rows.filter(r => r.status === s));
    // The points tiles add each chart's delta once, not once per copy (D79).
    const gains = rows.filter(r => r.status === STATUS.newHigher).reduce((a, r) => a + r.delta, 0);
    const losses = rows.filter(r => r.status === STATUS.oldHigher).reduce((a, r) => a - r.delta, 0);
    return [
      ['Charts', fmt(charts(rows))],
      [STATUS.newHigher, fmt(n(STATUS.newHigher))],
      [STATUS.oldHigher, fmt(n(STATUS.oldHigher))],
      ['Same', fmt(n(STATUS.sameScore))],
      ['Only one side', fmt(n(STATUS.onlyOld) + n(STATUS.onlyNew))],
      [__IN_BOTH_TILE__, fmt(n(STATUS.inBoth))],
      ['Points gained in 1.1', fmt(gains)],
      ['Points lost in 1.1', fmt(losses)],
    ];
  },
};
)page";

// The kStatus* constants: each placeholder the dropdown markup carries, and
// the name the script's STATUS object gives it.
struct StatusSlot {
    const char* placeholder;
    const char* js_name;
    const char* word;
};
constexpr StatusSlot kStatusSlots[] = {
    {"__ST_SAME__", "sameScore", kStatusSame},
    {"__ST_OLD_HIGHER__", "oldHigher", kStatusOldHigher},
    {"__ST_NEW_HIGHER__", "newHigher", kStatusNewHigher},
    {"__ST_ONLY_OLD__", "onlyOld", kStatusOnlyOld},
    {"__ST_ONLY_NEW__", "onlyNew", kStatusOnlyNew},
    {"__ST_IN_BOTH__", "inBoth", kStatusInBoth},
};

// The page shell, built once on first use. The title, heading and column
// names read each rule's short name from fill_rule_name; the dropdown, the
// script's STATUS and the in-both tile read fill_report.h's words.
std::string build_page_template() {
    std::string page = html::page_template(kTitle, kBody, kPageJs);
    std::string status_js = "{";
    for (const StatusSlot& s : kStatusSlots) {
        page = replace_all(std::move(page), s.placeholder, html::html_escape(s.word));
        if (status_js.size() > 1) status_js += ", ";
        status_js += s.js_name;
        status_js += ": ";
        json_escape_into(status_js, s.word);
    }
    status_js += "}";
    page = replace_all(std::move(page), "__STATUS__", status_js);
    // The search box's hint is the report windows' (html_page.h).
    page = replace_all(std::move(page), "__SEARCH_HINT__",
                       html::html_escape(html::kSearchChartsHint));
    std::string tile = kInBothPhrase;
    tile[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(tile[0])));
    std::string tile_js;
    json_escape_into(tile_js, tile);
    page = replace_all(std::move(page), "__IN_BOTH_TILE__", tile_js);
    page = replace_all(std::move(page), "__OLD_RULE__",
                       fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Short));
    return replace_all(std::move(page), "__NEW_RULE__",
                       fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Short));
}

const std::string& page_template() {
    static const std::string page = build_page_template();
    return page;
}

// A record's best path as text, or empty when it kept none.
std::string best_path(const store::RecordRow& record) {
    return record.paths.empty() ? std::string() : record.paths.front().path;
}

}  // namespace

std::vector<FillCompareRow> collect_fill_rows(store::RecordStore& old_store,
                                              store::RecordStore& new_store,
                                              const Settings& settings) {
    // One read per store under the same profile but the fill rule: the old
    // side asks for 1.0 results, the new side for 1.1, whatever `settings`
    // says. Each side keeps the charts its own library lists.
    const store::ChartIndex old_library = old_store.chart_index();
    const store::ChartIndex new_library = new_store.chart_index();
    const std::unordered_map<std::string, store::RecordRow> old_by_id = report::listed_records(
        old_store, settings.profile_with_fills(FillDeadlineRule::Ch10), old_library);
    const std::unordered_map<std::string, store::RecordRow> new_by_id = report::listed_records(
        new_store, settings.profile_with_fills(FillDeadlineRule::Ch11), new_library);

    // Walk the union of both key sets so a chart in only one database still
    // gets a row. Ordered so the page's rows come out deterministically.
    std::map<std::string, std::pair<const store::RecordRow*, const store::RecordRow*>> united;
    for (const auto& [id, rec] : old_by_id) united[id].first = &rec;
    for (const auto& [id, rec] : new_by_id) united[id].second = &rec;

    std::vector<FillCompareRow> rows;
    rows.reserve(united.size());
    for (const auto& [id, pair] : united) {
        const store::RecordRow* old_rec = pair.first;
        const store::RecordRow* new_rec = pair.second;

        FillCompareRow row;
        row.hyhash = id;
        // Copies come from the 1.1 database's library (D79 item 3).
        row.copies = new_library.copies(id);

        // Identity prefers the 1.1 side; either side names the same chart,
        // and the side that holds a record lists it.
        const store::ChartLibraryEntry* named =
            new_rec ? new_library.naming(id) : old_library.naming(id);
        row.song = display_title(named->title);
        row.artist = display_artist(named->artist);
        row.charter = display_charter(named->charter);

        // Which sides kept a scored best path: the store's one answer.
        const bool old_scored = old_rec && old_rec->has_scored_best_path();
        const bool new_scored = new_rec && new_rec->has_scored_best_path();
        if (old_rec) {
            const store::PathSummary s = old_rec->summary();
            if (old_scored) row.old_score = s.score;
            row.old_path = best_path(*old_rec);
            row.old_acts = s.actcount;
        }
        if (new_rec) {
            const store::PathSummary s = new_rec->summary();
            if (new_scored) row.new_score = s.score;
            row.new_path = best_path(*new_rec);
            row.new_acts = s.actcount;
        }
        row.notes = (new_rec ? new_rec : old_rec)->summary().notecount;

        // A chart with a score on both sides compares them. A chart only one
        // database holds a record for is labelled by that database, score or
        // not. When both hold a record but only one kept a path, the chart is
        // in both (D50 item 2), and the page writes "no paths" on the empty
        // side (D51 call 11).
        if (old_scored && new_scored) {
            int64_t delta = *row.new_score - *row.old_score;
            row.delta = delta;
            row.status = delta == 0 ? kStatusSame
                                    : (delta > 0 ? kStatusNewHigher : kStatusOldHigher);
        } else if (!new_rec) {
            row.status = kStatusOnlyOld;
        } else if (!old_rec) {
            row.status = kStatusOnlyNew;
        } else {
            row.status = kStatusInBoth;
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

FillCompareStats tally_fill_rows(const std::vector<FillCompareRow>& rows,
                                 const store::ChartIndex& chart_index) {
    // The page's stats() adds up each row's "k", the same index's copies, by
    // the same statuses; the test "fill page: the tiles add up the copies
    // tally_fill_rows adds up" pins the two together.
    std::vector<std::string> all, same, ch10_higher, ch11_higher, only_old, only_new, in_both;
    for (const FillCompareRow& r : rows) {
        all.push_back(r.hyhash);
        if (r.status == kStatusSame) same.push_back(r.hyhash);
        else if (r.status == kStatusOldHigher) ch10_higher.push_back(r.hyhash);
        else if (r.status == kStatusNewHigher) ch11_higher.push_back(r.hyhash);
        else if (r.status == kStatusOnlyOld) only_old.push_back(r.hyhash);
        else if (r.status == kStatusOnlyNew) only_new.push_back(r.hyhash);
        else if (r.status == kStatusInBoth) in_both.push_back(r.hyhash);
    }
    auto count = [&](const std::vector<std::string>& ids) {
        return static_cast<int>(chart_index.count(ids));
    };
    FillCompareStats stats;
    stats.total = count(all);
    stats.same = count(same);
    stats.ch10_higher = count(ch10_higher);
    stats.ch11_higher = count(ch11_higher);
    stats.only_old = count(only_old);
    stats.only_new = count(only_new);
    stats.in_both = count(in_both);
    return stats;
}

std::string build_fill_html(const std::vector<FillCompareRow>& rows,
                            const store::ChartIndex& library,
                            const std::string& subtitle,
                            const std::string& footer) {
    auto opt_num = [](const auto& o) {
        return o ? std::to_string(*o) : std::string("null");
    };

    std::string data;
    data.reserve(rows.size() * 220 + 2);
    data.push_back('[');
    bool first = true;
    for (const FillCompareRow& r : rows) {
        if (!first) data.push_back(',');
        first = false;

        // "k" is the chart's library copies, what the chart tiles add up.
        data += "{\"k\":" + std::to_string(r.copies);
        data += ",\"song\":";
        json_escape_into(data, r.song);
        data += ",\"artist\":";
        json_escape_into(data, r.artist);
        data += ",\"charter\":";
        json_escape_into(data, r.charter);
        data += ",\"search\":";
        json_escape_into(data, search_field(r.song, r.artist, r.charter));
        data += ",\"s10\":" + opt_num(r.old_score);
        data += ",\"s11\":" + opt_num(r.new_score);
        data += ",\"delta\":" + opt_num(r.delta);
        data += ",\"p10\":";
        json_escape_into(data, r.old_path);
        data += ",\"p11\":";
        json_escape_into(data, r.new_path);
        // `acts` is what the Acts column sorts on (the 1.1 count, falling back
        // to 1.0); `acts_txt` is what it shows: "1.0 / 1.1".
        data += ",\"acts\":" + opt_num(r.new_acts ? r.new_acts : r.old_acts);
        std::string acts_txt = (r.old_acts ? std::to_string(*r.old_acts)
                                           : std::string(report::kDash));
        acts_txt += " / ";
        acts_txt += (r.new_acts ? std::to_string(*r.new_acts)
                                : std::string(report::kDash));
        data += ",\"acts_txt\":";
        json_escape_into(data, acts_txt);
        data += ",\"notes\":" + opt_num(r.notes);
        data += ",\"status\":";
        json_escape_into(data, r.status);
        // Each text column's sort field: its text as the windows compare it
        // (report_view::folded_key), which the page's script compares as is
        // (D114 item 3).
        for (const auto& [name, text] :
             {std::pair<const char*, const std::string*>{"song_s", &r.song},
              {"artist_s", &r.artist}, {"charter_s", &r.charter}, {"p10_s", &r.old_path},
              {"p11_s", &r.new_path}, {"status_s", &r.status}}) {
            data += ",\"";
            data += name;
            data += "\":";
            json_escape_into(data, std::get<std::string>(
                                       report_view::folded_key(report_view::SortKey{*text})));
        }
        data.push_back('}');
    }
    data.push_back(']');
    // The count line's total, worded by counted so one chart reads "1 of 1
    // chart" (D114 item 4).
    std::string total;
    json_escape_into(total, counted(tally_fill_rows(rows, library).total, "chart", "charts"));
    const std::string page = replace_all(page_template(), "__COUNT_TOTAL__", total);
    return html::render_page(page.c_str(), std::move(data), subtitle, footer);
}

GeneratedFillReport generate_fill_report(store::RecordStore& old_store,
                                         store::RecordStore& new_store,
                                         const Settings& settings) {
    GeneratedFillReport out;
    // The page compares library charts only, as the path report does (D92),
    // so either database with results and no library stops it.
    if (report::lacks_chart_library(old_store) || report::lacks_chart_library(new_store)) {
        out.reason = report::kNoChartLibrary;
        return out;
    }
    std::vector<FillCompareRow> rows = collect_fill_rows(old_store, new_store, settings);
    // Every count is the 1.1 database's library's, as each row's copies are.
    const store::ChartIndex library = new_store.chart_index();
    out.stats = tally_fill_rows(rows, library);
    // Both sides were read under this profile's chart mode and cap; only the
    // fill rule differs.
    const store::Profile profile = settings.profile_with_fills(FillDeadlineRule::Ch11);
    const std::string& chartmode = profile.chartmode;
    if (rows.empty()) {
        // Same sentence whether the databases are empty or hold results under
        // other settings (finding 105, D50 item 3).
        out.reason =
            report::nothing_under_settings(profile.cap.exact, chartmode, " in either database");
        return out;
    }

    std::string subtitle =
        counted(out.stats.total, "chart", "charts") + " in " + chartmode + ": " +
        group_thousands(out.stats.ch11_higher) + " score higher under 1.1, " +
        group_thousands(out.stats.ch10_higher) + " higher under 1.0, " +
        group_thousands(out.stats.same) + " unchanged, " +
        group_thousands(out.stats.only_old + out.stats.only_new) +
        " in one database only, " + group_thousands(out.stats.in_both) + " " + kInBothPhrase;
    std::string footer =
        "A drum fill only appears if your Star Power meter filled up in time. " +
        fill_rule_description(FillDeadlineRule::Ch10) + " " +
        fill_rule_description(FillDeadlineRule::Ch11) + " " +
        // The rule's length is fill_rule_description's to say; this sentence
        // only names the rule.
        fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Long) +
        "'s deadline usually falls earlier, so short fills got stricter and most "
        "charts tie or drop. "
        "Long fills got looser, which is where the rare gains come from. "
        "Delta is the 1.1 score minus the 1.0 score.";
    out.html = build_fill_html(rows, library, subtitle, footer);
    return out;
}

}  // namespace hydra::app::fill_report
