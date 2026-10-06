#include "app/dm_report.h"

#include <stdexcept>
#include <unordered_map>

#include "app/display_format.h"  // format_percent, percent_steps
#include "app/html_page.h"
#include "app/report.h"  // records_by_hash, library_copies_by_hash
#include "core/error_kind.h"
#include "core/model.h"  // counted, group_thousands
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "search/graph.h"  // fill_rule_name

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
      <option value="under optimal">Under optimal</option>
      <option value="at optimal">At optimal</option>
      <option value="above optimal">Above optimal</option>
      <option value="not analyzed">Not analyzed (in your library)</option>
      <option value="no paths">No paths (analyzed, none kept)</option>
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

const char* const kPageJs = R"page(const STATUS_CLASS = {'under optimal':'s-matched', 'at optimal':'s-matched',
                      'above optimal':'s-above',
                      'not analyzed':'s-notanalyzed', 'no paths':'s-notanalyzed',
                      'not in library':'s-unmatched',
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
    {k:'optimal', t:'Hydra opt', num:true,  d:'The optimal score Hydra found for the chart at SP cap __SP_CAP__, the Clone Hero rule.'},
    {k:'delta',   t:'Points left', num:true, d:'Hydra opt minus Actual. Marked over when the posted score is higher.'},
    {k:'pct_h',   t:'% of opt',  num:true,  d:'Actual as a percent of Hydra opt. Only for scores played at __BASE_SPEED__% speed.'},
    {k:'fc',      t:'FC',        num:true,  d:'Full combo: every note hit.'},
    {k:'percent', t:'Percent',   num:true,  d:'The percent the leaderboard lists for this score.'},
    {k:'speed',   t:'Speed',     num:true,  d:'The playback speed the score was set at. __BASE_SPEED__% is normal speed.'},
    {k:'rank',    t:'Rank',      num:true,  d:'The score rank on this chart leaderboard.'},
    {k:'posted',  t:'Posted',    num:false, d:'The date the score was posted.'},
    {k:'status',  t:'Status',    num:false, d:'Under optimal, At optimal or Above optimal when Hydra has a result. Not analyzed: the chart is in your library but has no current result for this mode at SP cap __SP_CAP__. No paths: analyzed, but the analysis kept no path. Not in your library: the last scan did not find it. Other speed: played at a speed other than __BASE_SPEED__%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared.'},
  ],
  controls: [['q', 'input'], ['status', 'change']],
  // The search box is matched against each row's search text in the shared
  // script; this keeps rows by the status control.
  filter() {
    const status = document.getElementById('status').value;
    return r => !status || r.status === status;
  },
  cells(r) {
    const noDelta = r.delta === null || r.delta === undefined;
    const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.status === 'above optimal' ? 'num neg' : 'num');
    const deltaTxt = noDelta ? DASH
                   : (r.above_optimal ? '+' + fmt(-r.delta) + ' over' : fmt(r.delta));
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
  // A "no paths" row has no tile of its own (D62 item 1); Scores counts it.
  stats(rows) {
    const under = rows.filter(r => r.status === 'under optimal');
    const at = rows.filter(r => r.status === 'at optimal');
    const above = rows.filter(r => r.status === 'above optimal');
    const notAnalyzed = rows.filter(r => r.status === 'not analyzed');
    const notInLibrary = rows.filter(r => r.status === 'not in library');
    const otherSpeed = rows.filter(r => r.status === 'other speed');
    // The one percent the page works out itself, because it follows the
    // filters: the mean of the cells' percents, which the payload carries in
    // whole hundredths (percent_steps, the number format_percent writes).
    // Whole numbers only, and the mean rounds half up like format_percent, so
    // one row's tile reads exactly its cell.
    const withPct = rows.filter(r => r.pct_h !== null && r.pct_h !== undefined);
    let avgPct = DASH;
    if (withPct.length) {
      const n = withPct.length;
      const sum = withPct.reduce((a, r) => a + r.pct_h, 0);
      const h = Math.floor((2 * sum + n) / (2 * n));
      avgPct = Math.floor(h / 100) + '.' + String(h % 100).padStart(2, '0') + '%';
    }
    // Only a score under optimal leaves points on the table, and every such
    // row's delta is the points it left.
    const left = under.reduce((a, r) => a + r.delta, 0);
    return [
      ['Scores', fmt(rows.length)],
      ['Under optimal', fmt(under.length)],
      ['At optimal', fmt(at.length)],
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
    // The help texts name the base speed through __BASE_SPEED__ and Clone
    // Hero's cap through __SP_CAP__, so the page reads net::kBaseSpeedPercent
    // and kCloneHeroSpCap instead of repeating them.
    static const std::string page = html::replace_all(
        html::replace_all(html::page_template(kTitle, kBody, kPageJs), "__BASE_SPEED__",
                          std::to_string(net::kBaseSpeedPercent)),
        "__SP_CAP__", std::to_string(kCloneHeroSpCap));
    return page;
}


}  // namespace

std::string why_not_comparable(Difficulty difficulty, int sp_cap, bool legacy_fills) {
    if (difficulty != Difficulty::Expert)
        return "Needs Expert: the leaderboard only has Expert scores.";
    if (sp_cap != kCloneHeroSpCap)
        return "Needs SP cap " + std::to_string(kCloneHeroSpCap) +
               ", Clone Hero's rule: the leaderboard's scores were played under it.";
    if (legacy_fills)
        return std::string("Needs ") +
               fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Long) +
               " fills: untick \"1.0 fills\". The leaderboard is played on current Clone Hero.";
    return std::string();
}

std::vector<DmReportRow> collect_dm_rows(store::RecordStore& store,
                                         const std::vector<net::DmScore>& scores,
                                         const std::string& chartmode,
                                         const store::Lens& lens) {
    // The Clone Hero rules come from why_not_comparable. Of its three inputs,
    // only the fill rule arrives here: the cap is forced to Clone Hero's
    // below, and the difficulty is folded into `chartmode`, so the caller's
    // settings take that rule to the same owner (the library toolbar asks it
    // for all three before a comparison can start).
    const std::string refused =
        why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, lens.legacy_fills != 0);
    if (!refused.empty()) throw KindedError(ErrorKind::AlreadyPlain, refused);

    // One query for every stored record in this chartmode, indexed by hash.
    // Only records at Clone Hero's cap: a what-if cap's score would read as
    // "above optimal" nonsense.
    const std::unordered_map<std::string, store::RecordListing> by_hash =
        report::records_by_hash(store, chartmode, store::CapQuery::at(kCloneHeroSpCap), lens);

    // Every chart the last scan found, keyed like the leaderboard join, so a
    // score with no current result can say whether analyzing would fix it.
    // The page counts scores, not charts, so the copies go unused (D78).
    const std::unordered_map<std::string, int> library = report::library_copies_by_hash(store);

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
        // joined Hydra record's, else whatever the leaderboard sent. Either
        // source goes through the same display owners (D74 item 2).
        const bool board_names = (s.known && !s.song_name.empty()) || !rec;
        row.song = display_title(board_names ? s.song_name : rec->ref_name);
        row.artist = display_artist(board_names ? s.artist : rec->ref_artist);
        row.charter = display_charter(board_names ? s.charter : rec->ref_charter);
        if (row.charter.empty() && rec) row.charter = display_charter(rec->ref_charter);

        // Hydra's optimal is a base-speed answer, and Clone Hero keeps a
        // leaderboard per speed. An off-speed score shows Hydra's numbers when
        // it has them, but is never called under, at or above optimal.
        const bool base = net::is_base_speed(s.speed);
        if (rec && rec->summary.has_scored_best_path()) {
            int64_t opt = *rec->summary.score;
            row.optimal = opt;
            row.delta = opt - s.score;
            if (base && opt > 0) row.pct_h = percent_steps(s.score, opt, 2);
            const bool above = s.score > opt;
            row.status = above       ? "above optimal"
                         : s.score == opt ? "at optimal"
                                          : "under optimal";
            // Kept apart from the status, which an off-speed score overwrites
            // below; the page's "+N over" reads it (D64).
            row.above_optimal = above;
        } else if (rec) {
            // A Ready result whose analysis kept no path (D51 call 11).
            row.status = "no paths";
        } else {
            row.status = library.count(s.identifier) ? "not analyzed" : "not in library";
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
    for (const DmReportRow& r : rows) {
        if (!first) data.push_back(',');
        first = false;

        data += "{\"song\":";
        json_escape_into(data, r.song);
        data += ",\"artist\":";
        json_escape_into(data, r.artist);
        data += ",\"charter\":";
        json_escape_into(data, r.charter);
        data += ",\"search\":";
        json_escape_into(data, html::search_field(r.song, r.artist, r.charter));
        data += ",\"actual\":" + std::to_string(r.actual);
        data += ",\"optimal\":" + (r.optimal ? std::to_string(*r.optimal) : std::string("null"));
        data += ",\"delta\":" + (r.delta ? std::to_string(*r.delta) : std::string("null"));
        data += ",\"above_optimal\":" + std::string(r.above_optimal ? "1" : "0");
        // `pct_h` is the row's one percent, in whole hundredths: the column
        // sorts on it and the average tile reads it. `pct_txt` is the same
        // rounded percent as the cell shows it, written by format_percent.
        if (r.pct_h && r.optimal) {
            data += ",\"pct_h\":" + std::to_string(*r.pct_h);
            data += ",\"pct_txt\":";
            json_escape_into(data, format_percent(r.actual, *r.optimal, 2));
        } else {
            data += ",\"pct_h\":null,\"pct_txt\":null";
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
        if (r.status == "under optimal") ++stats.under_optimal;
        else if (r.status == "at optimal") ++stats.at_optimal;
        else if (r.status == "above optimal") ++stats.above_optimal;
        else if (r.status == "not analyzed") ++stats.not_analyzed;
        else if (r.status == "no paths") ++stats.no_paths;
        else if (r.status == "other speed") ++stats.other_speed;
        else ++stats.not_in_library;
    }
    return stats;
}

std::string counts_phrase(const DmReportStats& stats) {
    std::string out = group_thousands(stats.under_optimal) + " under optimal, " +
                      group_thousands(stats.at_optimal) + " at optimal, " +
                      group_thousands(stats.above_optimal) + " above optimal, " +
                      group_thousands(stats.not_analyzed) + " not analyzed, " +
                      group_thousands(stats.not_in_library) + " not in your library";
    // Only when there is one, so every other phrase reads as before (D62).
    if (stats.no_paths > 0) out += ", " + group_thousands(stats.no_paths) + " with no paths";
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
        "at SP cap " + std::to_string(kCloneHeroSpCap) + ": analyze them, then compare again.";
    out.html = build_dm_html(rows, subtitle, footer);
    return out;
}

}  // namespace hydra::app::dm_report
