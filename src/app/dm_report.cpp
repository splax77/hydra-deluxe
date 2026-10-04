#include "app/dm_report.h"

#include <cstdio>
#include <unordered_map>
#include <unordered_set>

#include "app/display_format.h"  // format_percent
#include "app/html_page.h"
#include "app/report.h"  // records_by_hash
#include "core/model.h"  // counted
#include "core/strutil.h"  // to_lower_ascii
#include "parse/song.h"  // display_title, strip_rich_tags

namespace hydra::app::dm_report {

using html::json_escape_into;

namespace {

// The comparison page's own pieces. The stylesheet and the script that sorts,
// filters and draws the table are shared with the other report pages
// (html::page_template, docs/adr/0016). __SUBTITLE__/__FOOTER__/__DATA__ are
// filled by build_dm_html.
const char* const kTitle = "Hydra vs dmleaderboards";

const char* const kBody = R"page(<div class="wrap dm">
  <header>
    <h1>Hydra <span class="accent">vs dmleaderboards</span></h1>
    <div class="sub">__SUBTITLE__</div>
  </header>

  <div class="stats" id="stats"></div>

  <div class="controls">
    <span class="sorter">
      <label for="sortby">Sort by</label>
      <select id="sortby"></select>
      <button id="sortdir" type="button" title="Switch between highest-first and lowest-first"></button>
    </span>
    <input type="search" id="q" aria-label="Search scores" placeholder="Search song, artist, or charter">
    <select id="status" aria-label="Status">
      <option value="">All charts</option>
      <option value="matched">Matched</option>
      <option value="above optimal">Above optimal</option>
      <option value="not analyzed">Not analyzed (in your library)</option>
      <option value="not in library">Not in your library</option>
      <option value="other speed">Other speed</option>
    </select>
    <span class="count" id="count"></span>
  </div>

  <div class="tablewrap">
    <table>
      <thead><tr id="head"></tr></thead>
      <tbody id="body"></tbody>
    </table>
    <div class="empty" id="empty">Joining scores&hellip;</div>
  </div>

  <footer>
    <p>__FOOTER__</p>
    <dl class="legend" id="legend"></dl>
  </footer>
</div>

)page";

const char* const kPageJs = R"page(const STATUS_CLASS = {'matched':'s-matched', 'above optimal':'s-above',
                      'not analyzed':'s-notanalyzed', 'not in library':'s-unmatched',
                      'other speed':'s-otherspeed'};

const PAGE = {
  rows: DATA,
  noun: 'scores',
  sortKey: 'delta',
  sortDir: -1,
  cols: [
    {k:'song',    t:'Song',      num:false},
    {k:'artist',  t:'Artist',    num:false},
    {k:'charter', t:'Charter',   num:false},
    {k:'actual',  t:'Actual',    num:true,  d:'The score the player posted.'},
    {k:'optimal', t:'Hydra opt', num:true,  d:'The optimal score Hydra found for the chart at SP cap 4, the Clone Hero rule.'},
    {k:'delta',   t:'Points left', num:true, d:'Hydra opt minus Actual. Marked over when the posted score is higher.'},
    {k:'pct',     t:'% of opt',  num:true,  d:'Actual as a percent of Hydra opt. Only for scores played at __BASE_SPEED__% speed.'},
    {k:'fc',      t:'FC',        num:true,  d:'Full combo: every note hit.'},
    {k:'percent', t:'Percent',   num:true,  d:'The percent the leaderboard lists for this score.'},
    {k:'speed',   t:'Speed',     num:true,  d:'The playback speed the score was set at. __BASE_SPEED__% is normal speed.'},
    {k:'rank',    t:'Rank',      num:true,  d:'The score rank on this chart leaderboard.'},
    {k:'posted',  t:'Posted',    num:false, d:'The date the score was posted.'},
    {k:'status',  t:'Status',    num:false, d:'Matched or Above optimal when Hydra has a result. Not analyzed: the chart is in your library but has no current result for this mode at SP cap 4. Not in your library: the last scan did not find it. Other speed: played at a speed other than __BASE_SPEED__%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared.'},
  ],
  controls: [['q', 'input'], ['status', 'change']],
  filter(q) {
    const status = document.getElementById('status').value;
    return r => {
      if (status && r.status !== status) return false;
      if (!q) return true;
      return (r.song + ' ' + r.artist + ' ' + r.charter).toLowerCase().includes(q);
    };
  },
  cells(r) {
    const noDelta = r.delta === null || r.delta === undefined;
    const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.delta < 0 ? 'num neg' : 'num');
    const deltaTxt = noDelta ? DASH
                   : (r.delta < 0 ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));
    return [
      ['song trunc', r.song],
      ['dim trunc artist', r.artist],
      ['dim trunc charter', r.charter],
      ['num', fmt(r.actual)],
      ['num', fmt(r.optimal)],
      [deltaCls, deltaTxt],
      ['num', r.pct_txt === null || r.pct_txt === undefined ? DASH : r.pct_txt],
      ['num', r.fc ? '\u2713' : DASH],
      ['num', r.percent + '%'],
      ['num', r.speed + '%'],
      ['num', r.rank === null || r.rank === undefined ? DASH : '#' + r.rank],
      ['dim', r.posted ? r.posted.slice(0, 10) : DASH],
      ['chip ' + (STATUS_CLASS[r.status] || 's-unmatched'), r.status, 'chip'],
    ];
  },
  // Mirrors tally_dm_rows in dm_report.cpp; the test "s2 offspeed: the page counts the same statuses" checks the two agree.
  stats(rows) {
    const matched = rows.filter(r => r.status === 'matched');
    const above = rows.filter(r => r.status === 'above optimal');
    const notAnalyzed = rows.filter(r => r.status === 'not analyzed');
    const notInLibrary = rows.filter(r => r.status === 'not in library');
    const otherSpeed = rows.filter(r => r.status === 'other speed');
    // The one percent the page works out itself: the mean of the full
    // percents of the rows the filter shows, rounded once here.
    const withPct = rows.filter(r => r.pct !== null && r.pct !== undefined);
    const avgPct = withPct.length
      ? (withPct.reduce((a, r) => a + r.pct, 0) / withPct.length).toFixed(2) + '%' : DASH;
    const left = matched.reduce((a, r) => a + (r.delta > 0 ? r.delta : 0), 0);
    return [
      ['Scores', fmt(rows.length)],
      ['Matched', fmt(matched.length)],
      ['Above optimal', fmt(above.length)],
      ['Not analyzed', fmt(notAnalyzed.length)],
      ['Not in library', fmt(notInLibrary.length)],
      ['Other speed', fmt(otherSpeed.length)],
      ['Avg % of optimal', avgPct],
      ['Points left on table', fmt(left)],
    ];
  },
};
)page";

// The page shell, built once on first use.
const std::string& page_template() {
    // The help texts name the base speed through __BASE_SPEED__, so the page
    // reads net::kBaseSpeedPercent instead of repeating 100.
    static const std::string page =
        html::replace_all(html::page_template(kTitle, kBody, kPageJs), "__BASE_SPEED__",
                          std::to_string(net::kBaseSpeedPercent));
    return page;
}


}  // namespace

std::vector<DmReportRow> collect_dm_rows(store::RecordStore& store,
                                         const std::vector<net::DmScore>& scores,
                                         const std::string& chartmode,
                                         const store::Lens& lens) {
    // One query for every stored record in this chartmode, indexed by hash.
    // Only 4-bar records: the leaderboard plays by Clone Hero's rules, and a
    // what-if cap's score would read as "above optimal" nonsense.
    const std::unordered_map<std::string, store::RecordListing> by_hash =
        report::records_by_hash(store, chartmode, store::CapQuery::at(kCloneHeroSpCap), lens);

    // Every chart the last scan found, lower-cased like the leaderboard's
    // identifiers, so a score with no current result can say whether
    // analyzing would fix it.
    std::unordered_set<std::string> in_library;
    for (const store::ChartLibraryEntry& e : store.list_chart_library(std::nullopt, 0, -1))
        in_library.insert(to_lower_ascii(e.md5));

    std::vector<DmReportRow> rows;
    rows.reserve(scores.size());
    for (const net::DmScore& s : scores) {
        DmReportRow row;
        row.identifier = s.identifier;
        row.actual = s.score;
        row.is_fc = s.is_fc;
        row.percent = s.percent;
        row.speed = s.speed;
        row.rank = s.rank;
        row.posted = s.posted;

        auto it = by_hash.find(s.identifier);
        const store::RecordListing* rec = it != by_hash.end() ? &it->second : nullptr;

        // Identity: the leaderboard's own metadata when it has it, else the
        // matched Hydra record's, else the "Unknown Song: <hash>" placeholder.
        if (s.known && !s.song_name.empty()) {
            row.song = s.song_name;
            row.artist = s.artist;
            row.charter = s.charter;
        } else if (rec) {
            row.song = display_title(rec->ref_name);
            row.artist = strip_rich_tags(rec->ref_artist);
            row.charter = strip_rich_tags(rec->ref_charter);
        } else {
            row.song = s.song_name;
            row.artist = s.artist;
            row.charter = s.charter;
        }
        if (row.charter.empty() && rec) row.charter = strip_rich_tags(rec->ref_charter);

        // Hydra's optimal is a base-speed answer, and Clone Hero keeps a
        // leaderboard per speed. An off-speed score shows Hydra's numbers when
        // it has them, but is never called matched or above optimal.
        const bool base = net::is_base_speed(s.speed);
        if (rec && rec->summary.score) {
            int64_t opt = *rec->summary.score;
            row.optimal = opt;
            row.delta = opt - s.score;
            if (base && opt > 0)
                row.pct = static_cast<double>(s.score) / static_cast<double>(opt) * 100.0;
            row.status = s.score > opt ? "above optimal" : "matched";
        } else {
            row.status = in_library.count(s.identifier) ? "not analyzed" : "not in library";
        }
        if (!base) row.status = "other speed";
        rows.push_back(std::move(row));
    }
    return rows;
}

std::string build_dm_html(const std::vector<DmReportRow>& rows, const std::string& subtitle,
                          const std::string& footer) {
    std::string data;
    data.reserve(rows.size() * 200 + 2);
    data.push_back('[');
    bool first = true;
    char num[32];
    for (const DmReportRow& r : rows) {
        if (!first) data.push_back(',');
        first = false;

        data += "{\"song\":";
        json_escape_into(data, r.song);
        data += ",\"artist\":";
        json_escape_into(data, r.artist);
        data += ",\"charter\":";
        json_escape_into(data, r.charter);
        data += ",\"actual\":" + std::to_string(r.actual);
        data += ",\"optimal\":" + (r.optimal ? std::to_string(*r.optimal) : std::string("null"));
        data += ",\"delta\":" + (r.delta ? std::to_string(*r.delta) : std::string("null"));
        // `pct` is the full percent the column sorts on and the average tile
        // reads; `pct_txt` is what the cell shows, rounded once by
        // format_percent.
        if (r.pct && r.optimal) {
            std::snprintf(num, sizeof(num), "%.17g", *r.pct);
            data += ",\"pct\":" + std::string(num);
            data += ",\"pct_txt\":";
            json_escape_into(data, format_percent(r.actual, *r.optimal, 2));
        } else {
            data += ",\"pct\":null,\"pct_txt\":null";
        }
        data += ",\"fc\":" + std::string(r.is_fc ? "1" : "0");
        data += ",\"percent\":" + std::to_string(r.percent);
        data += ",\"speed\":" + std::to_string(r.speed);
        data += ",\"rank\":" + (r.rank ? std::to_string(*r.rank) : std::string("null"));
        data += ",\"posted\":";
        json_escape_into(data, r.posted);
        data += ",\"status\":";
        json_escape_into(data, r.status);
        data.push_back('}');
    }
    data.push_back(']');
    return html::render_page(page_template().c_str(), std::move(data), subtitle,
                             footer);
}

DmReportStats tally_dm_rows(const std::vector<DmReportRow>& rows) {
    DmReportStats stats;
    stats.total = static_cast<int>(rows.size());
    for (const DmReportRow& r : rows) {
        if (r.status == "matched") ++stats.matched;
        else if (r.status == "above optimal") ++stats.above_optimal;
        else if (r.status == "not analyzed") ++stats.not_analyzed;
        else if (r.status == "other speed") ++stats.other_speed;
        else ++stats.not_in_library;
    }
    return stats;
}

std::string counts_phrase(const DmReportStats& stats) {
    std::string out = group_thousands(stats.matched) + " matched, " +
                      group_thousands(stats.above_optimal) + " above optimal, " +
                      group_thousands(stats.not_analyzed) + " not analyzed, " +
                      group_thousands(stats.not_in_library) + " not in your library";
    if (stats.other_speed > 0)
        out += ", " + hydra::counted(stats.other_speed, "at another speed", "at other speeds");
    return out;
}

GeneratedDmReport generate_dm_report(store::RecordStore& store,
                                     const std::vector<net::DmScore>& scores,
                                     const std::string& chartmode,
                                     const store::Lens& lens,
                                     const std::string& username) {
    GeneratedDmReport out;
    std::vector<DmReportRow> rows = collect_dm_rows(store, scores, chartmode, lens);
    out.stats = tally_dm_rows(rows);
    if (rows.empty()) return out;

    std::string subtitle = username + " — " +
                           hydra::counted(out.stats.total, "score", "scores") + ": " +
                           counts_phrase(out.stats);
    std::string footer =
        "Actual scores from dmleaderboards.com against Hydra's optimal for " + chartmode +
        ". Above-optimal scores are expected — Hydra's optimal excludes several score "
        "backends, and older Clone Hero versions allowed fills that are impossible now. "
        "Not analyzed charts are in your library without a current result for this mode "
        "at SP cap 4: analyze them, then compare again.";
    out.html = build_dm_html(rows, subtitle, footer);
    return out;
}

}  // namespace hydra::app::dm_report
