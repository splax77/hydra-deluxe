#include "app/fill_report.h"

#include <map>
#include <unordered_map>
#include <utility>

#include "app/html_page.h"
#include "app/report.h"  // records_by_hash
#include "core/model.h"  // group_thousands, counted
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "search/graph.h"  // fill_rule_name, fill_rule_description

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
    <input type="search" id="q" aria-label="Search charts" placeholder="Search song, artist, or charter">
    <select id="status" aria-label="Status">
      <option value="">All charts</option>
      <option value="1.1 higher">1.1 higher</option>
      <option value="1.0 higher">1.0 higher</option>
      <option value="same">Same score</option>
      <option value="only 1.0">Only in 1.0 db</option>
      <option value="only 1.1">Only in 1.1 db</option>
      <option value="in both">In both</option>
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

const char* const kPageJs = R"page(const STATUS_CLASS = {'1.1 higher':'s-newhigh', '1.0 higher':'s-oldhigh',
                      'same':'s-same', 'only 1.0':'s-only', 'only 1.1':'s-only',
                      'in both':'s-only'};

// A chart in both databases with a score on one side reads "no score" on the
// other; any other missing score reads as a dash.
function scoreText(r, s) {
  return s === null && r.status === 'in both' ? 'no score' : fmt(s);
}

const PAGE = {
  rows: DATA,
  noun: 'charts',
  sortKey: 'delta',
  sortDir: -1,
  cols: [
    {k:'song',    t:'Song',        num:false},
    {k:'artist',  t:'Artist',      num:false},
    {k:'charter', t:'Charter',     num:false},
    {k:'s10',     t:'__OLD_RULE__',      num:true},
    {k:'s11',     t:'__NEW_RULE__',      num:true},
    {k:'delta',   t:'Delta',       num:true},
    {k:'p10',     t:'__OLD_RULE__ path', num:false},
    {k:'p11',     t:'__NEW_RULE__ path', num:false},
    {k:'acts',    t:'Acts',        num:true},
    {k:'notes',   t:'Notes',       num:true},
    {k:'status',  t:'Status',      num:false},
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
    const deltaCls = !hasDelta ? 'num dim' : (r.delta > 0 ? 'num pos'
                   : (r.delta < 0 ? 'num neg' : 'num dim'));
    const deltaTxt = !hasDelta ? DASH
                   : (r.delta > 0 ? '+' : '') + fmt(r.delta);
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
    const n = s => rows.filter(r => r.status === s).length;
    const gains = rows.filter(r => r.delta > 0).reduce((a, r) => a + r.delta, 0);
    const losses = rows.filter(r => r.delta < 0).reduce((a, r) => a - r.delta, 0);
    return [
      ['Charts', fmt(rows.length)],
      ['1.1 higher', fmt(n('1.1 higher'))],
      ['1.0 higher', fmt(n('1.0 higher'))],
      ['Same', fmt(n('same'))],
      ['Only one side', fmt(n('only 1.0') + n('only 1.1'))],
      ['Score on one side only', fmt(n('in both'))],
      ['Points gained in 1.1', fmt(gains)],
      ['Points lost in 1.1', fmt(losses)],
    ];
  },
};
)page";

// The page shell, built once on first use. The title, heading and column
// names read each rule's short name from fill_rule_name.
const std::string& page_template() {
    static const std::string page = html::replace_all(
        html::replace_all(html::page_template(kTitle, kBody, kPageJs), "__OLD_RULE__",
                          fill_rule_name(FillDeadlineRule::Ch10, FillRuleNameStyle::Short)),
        "__NEW_RULE__", fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Short));
    return page;
}

}  // namespace

std::vector<FillCompareRow> collect_fill_rows(store::RecordStore& old_store,
                                              store::RecordStore& new_store,
                                              const std::string& chartmode,
                                              const store::CapQuery& cap,
                                              const store::Lens& lens) {
    // One query per store under identical settings but the fill rule: the old
    // side asks for 1.0 results, the new side for 1.1, whatever `lens` says.
    // RecordListing already carries the best path and its summary, so no blob
    // is ever inflated.
    store::Lens old_lens = lens, new_lens = lens;
    old_lens.legacy_fills = 1;
    new_lens.legacy_fills = 0;
    const std::unordered_map<std::string, store::RecordListing> old_by_hash =
        report::records_by_hash(old_store, chartmode, cap, old_lens);
    const std::unordered_map<std::string, store::RecordListing> new_by_hash =
        report::records_by_hash(new_store, chartmode, cap, new_lens);

    // Walk the union of both key sets so a chart in only one database still
    // gets a row. Ordered so the page's rows come out deterministically.
    std::map<std::string, std::pair<const store::RecordListing*,
                                    const store::RecordListing*>> united;
    for (const auto& [hash, rec] : old_by_hash) united[hash].first = &rec;
    for (const auto& [hash, rec] : new_by_hash) united[hash].second = &rec;

    std::vector<FillCompareRow> rows;
    rows.reserve(united.size());
    for (const auto& [hash, pair] : united) {
        const store::RecordListing* old_rec = pair.first;
        const store::RecordListing* new_rec = pair.second;

        FillCompareRow row;
        row.hyhash = hash;

        // Identity prefers the 1.1 side; either side names the same chart.
        const store::RecordListing* id = new_rec ? new_rec : old_rec;
        row.song = display_title(id->ref_name);
        row.artist = display_artist(id->ref_artist);
        row.charter = display_charter(id->ref_charter);

        if (old_rec) {
            row.old_score = old_rec->summary.score;
            row.old_path = old_rec->bestpath;
            row.old_acts = old_rec->summary.actcount;
        }
        if (new_rec) {
            row.new_score = new_rec->summary.score;
            row.new_path = new_rec->bestpath;
            row.new_acts = new_rec->summary.actcount;
        }
        row.notes = new_rec ? new_rec->summary.notecount : old_rec->summary.notecount;

        // A chart with a score on both sides compares them. A chart only one
        // database holds a record for is labelled by that database, score or
        // not. When both hold a record but only one has a score, the chart
        // is in both, and the page writes "no score" on the empty side
        // (D50 item 2).
        if (row.old_score && row.new_score) {
            int64_t delta = *row.new_score - *row.old_score;
            row.delta = delta;
            row.status = delta == 0 ? "same" : (delta > 0 ? "1.1 higher" : "1.0 higher");
        } else if (!new_rec) {
            row.status = "only 1.0";
        } else if (!old_rec) {
            row.status = "only 1.1";
        } else {
            row.status = "in both";
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

FillCompareStats tally_fill_rows(const std::vector<FillCompareRow>& rows) {
    FillCompareStats stats;
    stats.total = static_cast<int>(rows.size());
    for (const FillCompareRow& r : rows) {
        if (r.status == "same") ++stats.same;
        else if (r.status == "1.0 higher") ++stats.ch10_higher;
        else if (r.status == "1.1 higher") ++stats.ch11_higher;
        else if (r.status == "only 1.0") ++stats.only_old;
        else if (r.status == "only 1.1") ++stats.only_new;
        else if (r.status == "in both") ++stats.in_both;
    }
    return stats;
}

std::string build_fill_html(const std::vector<FillCompareRow>& rows,
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

        data += "{\"song\":";
        json_escape_into(data, r.song);
        data += ",\"artist\":";
        json_escape_into(data, r.artist);
        data += ",\"charter\":";
        json_escape_into(data, r.charter);
        data += ",\"search\":";
        json_escape_into(data, html::search_field(r.song, r.artist, r.charter));
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
                                           : std::string("\xe2\x80\x94"));
        acts_txt += " / ";
        acts_txt += (r.new_acts ? std::to_string(*r.new_acts)
                                : std::string("\xe2\x80\x94"));
        data += ",\"acts_txt\":";
        json_escape_into(data, acts_txt);
        data += ",\"notes\":" + opt_num(r.notes);
        data += ",\"status\":";
        json_escape_into(data, r.status);
        data.push_back('}');
    }
    data.push_back(']');
    return html::render_page(page_template().c_str(), std::move(data), subtitle,
                             footer);
}

GeneratedFillReport generate_fill_report(store::RecordStore& old_store,
                                         store::RecordStore& new_store,
                                         const std::string& chartmode,
                                         const store::CapQuery& cap,
                                         const store::Lens& lens) {
    GeneratedFillReport out;
    std::vector<FillCompareRow> rows =
        collect_fill_rows(old_store, new_store, chartmode, cap, lens);
    out.stats = tally_fill_rows(rows);
    if (rows.empty()) {
        // Same sentence whether the databases are empty or hold results under
        // other settings (finding 105, D50 item 3); the mode is the chart mode
        // both sides were looked up under.
        out.reason =
            report::nothing_under_settings(cap.exact, chartmode, " in either database");
        return out;
    }

    std::string subtitle =
        counted(out.stats.total, "chart", "charts") + " in " + chartmode + ": " +
        group_thousands(out.stats.ch11_higher) + " score higher under 1.1, " +
        group_thousands(out.stats.ch10_higher) + " higher under 1.0, " +
        group_thousands(out.stats.same) + " unchanged, " +
        group_thousands(out.stats.only_old + out.stats.only_new) +
        " in one database only, " + group_thousands(out.stats.in_both) +
        " with a score on one side only";
    std::string footer =
        "A drum fill only appears if your Star Power meter filled up in time. " +
        fill_rule_description(FillDeadlineRule::Ch10) + " " +
        fill_rule_description(FillDeadlineRule::Ch11) + " " +
        // The rule's length is fill_rule_description's to say; this sentence
        // only names the rule.
        fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Long) +
        "'s wait is usually the longer one, so short fills got stricter and most "
        "charts tie or drop. "
        "Long fills got looser, which is where the rare gains come from. "
        "Delta is the 1.1 score minus the 1.0 score.";
    out.html = build_fill_html(rows, subtitle, footer);
    return out;
}

}  // namespace hydra::app::fill_report
