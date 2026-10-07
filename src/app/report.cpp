#include "app/report.h"

#include <algorithm>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

#include "app/analysis.h"  // normalize_chart_hash, batch_worker_count
#include "app/config.h"    // Settings::chartmode_key, to_analysis_settings
#include "app/display_format.h"
#include "app/html_page.h"
#include "app/work_pool.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"
#include "parse/song.h"
#include "search/graph.h"

namespace hydra::app::report {

using html::json_escape_into;

namespace {

// The path report's own pieces. The stylesheet and the script that sorts,
// filters and draws the table are shared with the other two report pages
// (html::page_template, docs/adr/0016).
const char* const kTitle = "Hydra Path Index";

const char* const kBody = R"page(<div class="wrap">
  <header>
    <h1>Hydra <span class="accent">Path Index</span></h1>
    <div class="sub">__SUBTITLE__</div>
  </header>

  <div class="stats" id="stats"></div>

  <div class="controls">
    <span class="sorter">
      <label for="sortby">Sort by</label>
      <select id="sortby"></select>
      <button id="sortdir" type="button" title="Switch between highest-first and lowest-first"></button>
    </span>
    <input type="search" id="q" aria-label="Search paths" placeholder="Search song, artist, charter, or path notation">
    <select id="tier" aria-label="Timing tier">
      <option value="">All timing tiers</option>
    </select>
    <label class="toggle"><input type="checkbox" id="bestonly" checked> Best path only</label>
    <span class="count" id="count"></span>
  </div>

  <div class="tablewrap">
    <table>
      <thead><tr id="head"></tr></thead>
      <tbody id="body"></tbody>
    </table>
    <div class="empty" id="empty">Reading paths&hellip;</div>
  </div>

  <footer>
    <p>__FOOTER__</p>
    <dl class="legend" id="legend"></dl>
  </footer>
</div>

)page";

// The payload is {hit_window, beyond_edge_ms, tiers, rows}. The tier dropdown
// reads the tier table, and the Beyond chip and the "Past N ms" tile read the
// edge C++ worked out, so they always match the bands the rows were labeled
// with.
const char* const kPageJs = R"page(// One name per tier, for both the dropdown and the chips, so a row's chip
// reads the same words as the filter that finds it.
function tierLabel(name) {
  return name === 'Beyond' ? 'Beyond ' + DATA.beyond_edge_ms + ' ms'
       : name === 'None' ? 'No squeezes'
       : name;
}

// The tier dropdown mirrors the bands the rows were labeled with.
{
  const sel = document.getElementById('tier');
  for (const t of DATA.tiers) {
    const o = document.createElement('option');
    o.value = t.name;
    o.textContent = tierLabel(t.name);
    sel.appendChild(o);
  }
}

const PAGE = {
  rows: DATA.rows,
  noun: 'paths',
  sortKey: 'score',
  sortDir: -1,
  cols: [
    {k:'song',    t:'Song',     num:false},
    {k:'artist',  t:'Artist',   num:false},
    {k:'charter', t:'Charter',  num:false},
    {k:'mode',    t:'Mode',     num:false, d:'The difficulty and drum options the path was found for.'},
    {k:'path',    t:'Path',     num:false, d:'The path in path notation: one entry per activation, with its skip count and squeeze symbols.'},
    {k:'score',   t:'Score',    num:true,  d:'The total score the path reaches.'},
    {k:'acts',    t:'Acts',     num:true,  d:'Activations: how many times the path uses Star Power.'},
    {k:'skip',    t:'Max skip', num:true,  d:'The most fills any one activation passes over before activating.'},
    {k:'ms',      t:'Hardest ms', num:true, d:'The hardest squeeze or required early fill the path needs, in raw ms. A dash means it needs none.'},
    {k:'tier',    t:'Timing',   num:false, d:'How hard Hardest ms is, in bands of your hit window. Beyond means more than twice the hit window.'},
    {k:'efill',   t:'Early fill (ms)', num:true, d:'The hardest early fill (E0) on the path: how many ms early you must hit to summon the fill. Negative means slack. A dash means the path has none.'},
    {k:'mult',    t:'Avg multiplier', num:true, d:'Average multiplier: the score without solo bonuses divided by the base score (every note at 1x).'},
    {k:'sqin',    t:'SqIn',     num:true,  d:'SP phrase notes squeezed into an active Star Power window (+ in the path).'},
    {k:'sqout',   t:'SqOut',    num:true,  d:'SP phrase notes squeezed out of an active Star Power window (- in the path).'},
    {k:'notes',   t:'Notes',    num:true,  d:'Notes in the chart.'},
  ],
  controls: [['q', 'input'], ['tier', 'change'], ['bestonly', 'change']],
  // The search box is matched against each row's search text in the shared
  // script; this keeps rows by the tier and best-path controls.
  filter() {
    const tier = document.getElementById('tier').value;
    const bestOnly = document.getElementById('bestonly').checked;
    return r => {
      if (bestOnly && !r.opt) return false;
      if (tier && r.tier !== tier) return false;
      return true;
    };
  },
  // Every path tied at the top score is optimal, as on the Paths tab.
  rowClass: r => r.opt ? 'best' : '',
  // The two timing columns print the app's own text; the numbers beside it
  // sort the column and feed the tiles.
  cells: r => [
    ['song trunc', r.song],
    ['dim trunc artist', r.artist],
    ['dim trunc charter', r.charter],
    ['dim trunc mode', r.mode],
    ['path mono trunc', r.path],
    ['num', fmt(r.score)],
    ['num', r.acts],
    ['num', r.skip],
    ['num', r.ms_text === null ? DASH : r.ms_text],
    ['chip ' + r.tok, tierLabel(r.tier), 'chip'],
    ['num', r.efill_text === null ? DASH : r.efill_text],
    ['num', r.mult_text],
    ['num', r.sqin],
    ['num', r.sqout],
    ['num', fmt(r.notes)],
  ],
  stats(rows) {
    // The row with the largest Hardest ms; the tile shows that row's text.
    const withMs = rows.filter(r => r.ms !== null && r.ms !== undefined);
    const hardest = withMs.length ? withMs.reduce((a, b) => b.ms > a.ms ? b : a) : null;
    const maxSkip = rows.length ? Math.max(...rows.map(r => r.skip)) : 0;
    // The rows tier_for put in Beyond (a timing on the edge itself is Insane+).
    const beyond = rows.filter(r => r.tier === 'Beyond').length;
    return [
      ['Charts', fmt([...new Map(rows.map(r => [r.c, r.k])).values()].reduce((n, k) => n + k, 0))],
      ['Paths shown', fmt(rows.length)],
      ['Hardest ms', hardest === null ? DASH : hardest.ms_text],
      ['Past ' + DATA.beyond_edge_ms + ' ms', fmt(beyond)],
      ['Highest skip', maxSkip],
    ];
  },
};
)page";

// The page shell, built once on first use.
const std::string& page_template() {
    static const std::string page = html::page_template(kTitle, kBody, kPageJs);
    return page;
}

// repr(float) / json.dumps float formatting for the page payload.
std::string py_repr(double v) {
    // std::to_chars with no precision produces the shortest string that
    // round-trips -- the same contract as CPython's float repr. The one
    // cosmetic difference: Python prints integral floats as "140.0" where
    // to_chars gives "140".
    char buf[32];
    auto res = std::to_chars(buf, buf + sizeof(buf), v);
    std::string s(buf, res.ptr);
    if (s.find_first_of(".eE") == std::string::npos &&
        s.find_first_of("0123456789") != std::string::npos)
        s += ".0";
    return s;
}

// A timing as the app prints it ("12.3 ms", format_ms), as a JSON
// string, or null when there is none. The page prints this text as it is.
void ms_text_into(std::string& data, const std::optional<double>& ms) {
    if (ms)
        json_escape_into(data, format_ms(*ms));
    else
        data += "null";
}

// The Beyond edge as the page and its footer print it: beyond_edge_ms, whole.
std::string beyond_edge_text(double hit_window_ms) {
    return std::to_string(static_cast<int64_t>(beyond_edge_ms(hit_window_ms)));
}

// One chart on the page: a small number, in order of first appearance among
// the rows, and its library copies (D76, D77).
struct PageChart {
    int id = 0;
    int copies = 0;  // the chart's ReportRow::copies
};

// The charts the rows belong to, each once. The page's "c" is the number, so
// the Charts tile finds each chart's "k" without the 32-character hash on
// every row, and the subtitle's chart count adds up the copies.
std::unordered_map<std::string, PageChart> page_charts(const std::vector<ReportRow>& rows) {
    std::unordered_map<std::string, PageChart> charts;
    for (const ReportRow& r : rows)
        charts.emplace(r.hyhash, PageChart{static_cast<int>(charts.size()), r.copies});
    return charts;
}

// Puts one chart's rows on the page: the first max_paths of `rows`
// (chart_rows' order), named from `meta` and labeled with their tier. `meta`
// is a list_records listing or a for_each_blob row; both carry the chart's
// hash, names and mode under the same field names.
template <typename ChartMeta>
void place_rows(std::vector<ReportRow>& out, const std::vector<ReportRow>& rows,
                int64_t max_paths, const ChartMeta& meta, int copies,
                const std::vector<TimingTier>& tiers) {
    const size_t shown =
        static_cast<size_t>(std::min<int64_t>(max_paths, static_cast<int64_t>(rows.size())));
    for (size_t i = 0; i < shown; ++i) {
        ReportRow row = rows[i];
        // The one cleaned title, artist and charter every screen shows.
        row.song = display_title(meta.ref_name);
        row.artist = display_artist(meta.ref_artist);
        row.charter = display_charter(meta.ref_charter);
        row.mode = meta.chartmode;
        std::tie(row.tier, row.tok) = tier_for(row.ms, tiers);
        row.hyhash = meta.hyhash;
        row.copies = copies;
        out.push_back(std::move(row));
    }
}

// The settings one chart mode is analyzed under. The run's own mode takes the
// run's settings as they are. Another mode keeps the run's search settings
// (the SearchSettings base) and takes the parse choices (AnalysisSettings'
// own fields) of the settings whose Settings::chartmode_key() spells it, so
// the key's one owner decides which choices a mode means. Throws when no
// choice spells the mode.
AnalysisSettings settings_for_mode(const BatchRun& run, const std::string& chartmode) {
    if (chartmode == run.chartmode) return run.settings;
    for (Difficulty difficulty : kAllDifficulties) {
        for (bool prodrums : {true, false}) {
            for (bool bass2x : {true, false}) {
                Settings mode;
                mode.view_difficulty = difficulty_name(difficulty);
                mode.view_prodrums = prodrums;
                mode.view_bass2x = bass2x;
                if (mode.chartmode_key() != chartmode) continue;
                AnalysisSettings out = mode.to_analysis_settings();
                static_cast<SearchSettings&>(out) = run.settings;
                return out;
            }
        }
    }
    throw std::runtime_error("No analysis settings give the chart mode \"" + chartmode + "\".");
}

}  // namespace

std::vector<ReportRow> chart_rows(const HydraRecord& record, int64_t max_paths) {
    // all_paths() is already best first (pather::read sorts the roots and
    // each variant sits under its parent), so ranks number it as it comes.
    const std::vector<const Path*> paths = record.all_paths();
    const int64_t shown = std::min<int64_t>(max_paths, static_cast<int64_t>(paths.size()));
    std::vector<ReportRow> rows;
    rows.reserve(static_cast<size_t>(std::max<int64_t>(shown, 0)));
    for (int64_t idx = 0; idx < shown; ++idx) {
        const Path* path = paths[static_cast<size_t>(idx)];
        store::PathSummary s = store::summarize_path(*path);

        ReportRow row;
        row.rank = static_cast<int>(idx + 1);
        row.optimal = record.is_optimal(*path);
        row.path = path->pathstring();
        row.score = *s.score;
        row.acts = *s.actcount;
        row.skip = *s.maxskip;
        row.ms = s.hardest_ms;
        for (const Activation& a : path->walk_activations()) {
            std::optional<double> ediff = a.e_difficulty();
            if (ediff.has_value() && (!row.efill || *ediff > *row.efill))
                row.efill = *ediff;
        }
        row.mult = py_round3(*s.avgmult);
        row.sqin = *s.sqin_count;
        row.sqout = *s.sqout_count;
        row.notes = *s.notecount;
        rows.push_back(std::move(row));
    }
    return rows;
}

ReportSeed ReportSeed::for_run(const BatchRun& run, int64_t max_paths) {
    ReportSeed seed;
    seed.chartmode = run.chartmode;
    seed.cap = run.cap_query();
    seed.lens = run.lens;
    seed.max_paths = max_paths;
    return seed;
}

std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             double hit_window_ms) {
    // The ladder itself lives in core/squeeze_rating.h (timing_tiers) so
    // these labels and the page's embedded tier table cannot drift apart.
    return tier_for(ms, timing_tiers(hit_window_ms));
}

std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             const std::vector<TimingTier>& tiers) {
    // The two open bands are the table's last two entries: "Beyond", then
    // the "None" (no squeeze) entry.
    const TimingTier& none = tiers.back();
    const TimingTier& beyond = tiers[tiers.size() - 2];
    if (!ms) return {none.name, none.tok};
    // Each edge belongs to the band below it (D48 Q3). The table's first row
    // is Normal up to and including the difficult floor, so a timing on the
    // floor reads Normal (past_difficult_floor's rule), and a timing exactly
    // on the two-hit budget is Insane+, not Beyond.
    for (const TimingTier& t : tiers)
        if (t.cutoff && *ms <= *t.cutoff) return {t.name, t.tok};
    return {beyond.name, beyond.tok};
}

std::unordered_map<std::string, store::RecordListing> records_by_hash(
    store::RecordStore& store, const std::string& chartmode, const store::CapQuery& cap,
    const store::Lens& lens) {
    std::unordered_map<std::string, store::RecordListing> by_hash;
    for (store::RecordListing& r : store.list_records(chartmode, cap, lens,
                                                       store::SortColumn::Score,
                                                       /*descending=*/true))
        by_hash.emplace(normalize_chart_hash(r.hyhash), std::move(r));
    return by_hash;
}

std::unordered_map<std::string, int> library_copies_by_hash(store::RecordStore& store) {
    std::unordered_map<std::string, int> by_hash;
    // Two md5s that differ only in case name one chart, so their rows add up.
    for (const auto& [md5, n] : store.library_copies()) by_hash[normalize_chart_hash(md5)] += n;
    return by_hash;
}

CollectedRows collect_rows(store::RecordStore& store,
                           const std::vector<store::ChartLibraryEntry>& library,
                           const ReportSeed& seed, const ReportOptions& options) {
    // Fail loudly rather than list paths found under other settings.
    if (!options.run)
        throw std::invalid_argument("collect_rows: the report has no analysis settings");
    const BatchRun& run = *options.run;
    if (run.cap_query() != options.cap || run.lens != options.lens)
        throw std::invalid_argument(
            "collect_rows: the analysis settings are not the report's cap and lens");
    if (!seed.rows.empty() && (seed.cap != options.cap || seed.lens != options.lens ||
                               seed.max_paths < options.max_paths))
        throw std::invalid_argument("collect_rows: the batch's rows are for other settings");

    // Each library chart's file, by the hash spelling a page joins on. Copies
    // of one md5 hold the same notes, so the first listed stands for all.
    std::unordered_map<std::string, const store::ChartLibraryEntry*> files;
    for (const store::ChartLibraryEntry& e : library) files.emplace(normalize_chart_hash(e.md5), &e);
    const std::unordered_map<std::string, int> copies = library_copies_by_hash(store);

    // One slot per chart and mode the page lists, in list_records' order.
    struct Slot {
        store::RecordListing listing;
        const store::ChartLibraryEntry* file = nullptr;
        const std::vector<ReportRow>* seeded = nullptr;  // the batch's rows, when it has them
        std::vector<ReportRow> rows;                     // this pass's rows otherwise
        std::string failure;
    };
    std::vector<Slot> slots;
    std::vector<size_t> to_analyze;  // slot indexes this pass analyzes
    std::unordered_map<std::string, AnalysisSettings> mode_settings;
    for (store::RecordListing& listing :
         store.list_records(std::nullopt, options.cap, options.lens, store::SortColumn::Score,
                            /*descending=*/true)) {
        const std::string hash = normalize_chart_hash(listing.hyhash);
        const auto file = files.find(hash);
        // A result whose chart left the library is not on the page (D87 item 4).
        if (file == files.end()) continue;
        Slot slot;
        slot.file = file->second;
        if (listing.chartmode == seed.chartmode) {
            const auto seeded = seed.rows.find(hash);
            if (seeded != seed.rows.end()) slot.seeded = &seeded->second;
        }
        if (!slot.seeded) {
            if (mode_settings.find(listing.chartmode) == mode_settings.end())
                mode_settings.emplace(listing.chartmode, settings_for_mode(run, listing.chartmode));
            to_analyze.push_back(slots.size());
        }
        slot.listing = std::move(listing);
        slots.push_back(std::move(slot));
    }

    // The analysis, on every core the batch would use. Workers only read
    // `slots` and `mode_settings`; each result is filed on this thread.
    struct Analyzed {
        size_t slot = 0;
        std::vector<ReportRow> rows;
        std::string failure;
        bool failed = false;
        bool cancelled = false;
    };
    const ChartAnalyzer analyze =
        options.analyze ? options.analyze : ChartAnalyzer(analyze_chart_file);
    const std::function<void(float)> check_cancel = stop_on_cancel(options.cancel);
    run_work_pool<Analyzed>(
        to_analyze.size(), batch_worker_count(), options.cancel,
        [&](size_t k) {
            Analyzed a;
            a.slot = to_analyze[k];
            const Slot& slot = slots[a.slot];
            try {
                const AnalysisResult ar = analyze(
                    slot.file->notespath, mode_settings.at(slot.listing.chartmode), check_cancel);
                a.rows = chart_rows(ar.record, options.max_paths);
            } catch (const AnalysisCancelled&) {
                a.cancelled = true;
            } catch (const std::exception& e) {
                a.failed = true;
                a.failure = slot.file->notespath + ": " + e.what();
            }
            return a;
        },
        [&](Analyzed&& a) {
            if (a.cancelled) return;
            Slot& slot = slots[a.slot];
            if (a.failed) slot.failure = std::move(a.failure);
            else slot.rows = std::move(a.rows);
        });

    CollectedRows out;
    // A stopped pass left charts unanalyzed: nothing is built from it.
    if (options.cancel && options.cancel->load()) return out;
    // Built once for the whole report, not once per row.
    const std::vector<TimingTier> tiers = timing_tiers(options.hit_window_ms);
    for (const Slot& slot : slots) {
        if (!slot.failure.empty()) {
            out.failures.push_back(slot.failure);
            continue;
        }
        // Every library copy counts (D76).
        place_rows(out.rows, slot.seeded ? *slot.seeded : slot.rows, options.max_paths,
                   slot.listing,
                   store::RecordStore::copies_of(copies, normalize_chart_hash(slot.listing.hyhash)),
                   tiers);
    }
    return out;
}

std::vector<ReportRow> collect_stored_rows(store::RecordStore& store, int64_t max_paths,
                                           const store::CapQuery& cap,
                                           const store::Lens& lens, double hit_window_ms,
                                           const std::atomic<bool>* cancel) {
    std::vector<ReportRow> rows;
    const std::vector<TimingTier> tiers = timing_tiers(hit_window_ms);
    const std::unordered_map<std::string, int> library = library_copies_by_hash(store);
    store.for_each_blob(std::nullopt, cap, lens,
                        [&](const store::RecordStore::BlobRow& meta,
                            const HydraRecord* record) {
        // Only rows the store calls Ready have a decoded record.
        if (!record) return;
        place_rows(rows, chart_rows(*record, max_paths), max_paths, meta,
                   store::RecordStore::copies_of(library, normalize_chart_hash(meta.hyhash)),
                   tiers);
    }, cancel);
    return rows;
}

std::string build_html(const std::vector<ReportRow>& rows, const std::string& subtitle,
                       const std::string& footer, double hit_window_ms) {
    // The payload: {hit_window, beyond_edge_ms, tiers, rows}. The page builds
    // its tier dropdown from the tiers and its Beyond chip and tile from the
    // edge, so the embedded UI can never drift from the bands the rows were
    // labeled with.
    std::string data;
    data.reserve(rows.size() * 160 + 256);
    data += "{\"hit_window\":" + py_repr(hit_window_ms);
    data += ",\"beyond_edge_ms\":" + beyond_edge_text(hit_window_ms);
    data += ",\"tiers\":[";
    {
        bool first_tier = true;
        for (const TimingTier& t : timing_tiers(hit_window_ms)) {
            if (!first_tier) data.push_back(',');
            first_tier = false;
            data += "{\"name\":";
            json_escape_into(data, t.name);
            data += ",\"tok\":\"";
            data += t.tok;
            data += "\",\"cutoff\":" +
                    (t.cutoff ? py_repr(*t.cutoff) : std::string("null"));
            data.push_back('}');
        }
    }
    data += "],\"rows\":[";
    const std::unordered_map<std::string, PageChart> charts = page_charts(rows);
    bool first_row = true;
    for (const ReportRow& r : rows) {
        if (!first_row) data.push_back(',');
        first_row = false;

        const PageChart& chart = charts.at(r.hyhash);
        data += "{\"c\":" + std::to_string(chart.id);
        data += ",\"k\":" + std::to_string(chart.copies);
        data += ",\"song\":";
        json_escape_into(data, r.song);
        data += ",\"artist\":";
        json_escape_into(data, r.artist);
        data += ",\"charter\":";
        json_escape_into(data, r.charter);
        data += ",\"mode\":";
        json_escape_into(data, r.mode);
        data += ",\"rank\":" + std::to_string(r.rank);
        data += ",\"opt\":";
        data += r.optimal ? "true" : "false";
        data += ",\"path\":";
        json_escape_into(data, r.path);
        data += ",\"search\":";
        json_escape_into(data, html::search_field(r.song, r.artist, r.charter, r.path));
        data += ",\"score\":" + std::to_string(r.score);
        data += ",\"acts\":" + std::to_string(r.acts);
        data += ",\"skip\":" + std::to_string(r.skip);
        data += ",\"ms\":" + (r.ms ? py_repr(*r.ms) : std::string("null"));
        data += ",\"ms_text\":";
        ms_text_into(data, r.ms);
        data += ",\"tier\":";
        json_escape_into(data, r.tier);
        data += ",\"tok\":";
        json_escape_into(data, r.tok);
        data += ",\"efill\":" + (r.efill ? py_repr(*r.efill) : std::string("null"));
        data += ",\"efill_text\":";
        ms_text_into(data, r.efill);
        // `mult` sorts the column; `mult_text` is what the cell shows.
        data += ",\"mult\":" + py_repr(r.mult);
        data += ",\"mult_text\":";
        json_escape_into(data, format_avg_mult(r.mult));
        data += ",\"sqin\":" + std::to_string(r.sqin);
        data += ",\"sqout\":" + std::to_string(r.sqout);
        data += ",\"notes\":" + std::to_string(r.notes);
        data.push_back('}');
    }
    data += "]}";
    return html::render_page(page_template().c_str(), std::move(data), subtitle,
                             footer);
}

std::string nothing_under_settings(int cap, const std::string& middle,
                                   const std::string& ending) {
    return std::string(kNothingUnderSettings) + " (SP cap " + group_thousands(cap) + ", " +
           middle + ")" + ending + ". Analyze with these settings, or change them.";
}

GeneratedReport generate_report(store::RecordStore& store, const ReportOptions& options,
                                const ReportSeed& seed) {
    GeneratedReport out;
    const double w = options.hit_window_ms;
    CollectedRows collected =
        collect_rows(store, store.list_chart_library(0, -1), seed, options);  // -1 = no limit
    // Cancelled part-way through: whatever the pass collected is a partial
    // library, so nothing is built from it. An empty result says "no report",
    // and the caller that set the flag already knows why.
    if (options.cancel && options.cancel->load()) {
        out.empty_reason = EmptyReason::Cancelled;
        return out;
    }
    out.failures = std::move(collected.failures);
    const std::vector<ReportRow>& rows = collected.rows;
    const FillDeadlineRule rule = fill_rule_for(options.lens.legacy_fills);
    out.rows = static_cast<int64_t>(rows.size());
    if (rows.empty()) {
        // Nothing listed. Either the database is empty, or it holds results
        // under another cap or fill rule (or only stale ones), and the user
        // needs to hear which (finding 105).
        if (store.counts().second == 0) {
            out.empty_reason = EmptyReason::NothingStored;
        } else {
            out.empty_reason = EmptyReason::NothingUnderSettings;
            out.why_empty = nothing_under_settings(
                options.cap.exact,
                std::string(fill_rule_name(rule, FillRuleNameStyle::Long)) + " fills");
        }
        return out;
    }
    // The subtitle counts what the page lists: every record on it has exactly
    // one rank-1 row, and its songs are the charts the page numbers. Both
    // count every library copy of a chart (D76, D77).
    for (const ReportRow& r : rows)
        if (r.rank == 1) out.records += r.copies;
    for (const auto& [hash, chart] : page_charts(rows)) out.songs += chart.copies;

    // Counts read the house rule (hydra::counted, D48 Q12). The cut is per
    // chart and mode, and the page lists every mode at the current cap.
    std::string shown = options.max_paths >= kEveryPathSentinel
                            ? "every path"
                            : "every mode at the current cap, top " +
                                  hydra::counted(options.max_paths, "path", "paths") +
                                  " per chart and mode";
    std::string cap_label = "SP cap " + hydra::counted(options.cap.exact, "bar", "bars");
    // The normal rule goes unsaid, so a 1.1 page reads as it always has.
    if (options.lens.legacy_fills)
        cap_label += std::string(" — ") + fill_rule_name(rule, FillRuleNameStyle::Long) + " fills";
    std::string subtitle = hydra::counted(out.records, "record", "records") + " across " +
                           hydra::counted(out.songs, "chart", "charts") + " — " + shown +
                           " — " + cap_label;
    std::string dbname =
        std::filesystem::u8path(options.db_path).filename().u8string();
    // D50 item 1. The window named is the Beyond edge, printed whole the way
    // the page's Beyond chip and "Past N ms" tile print it.
    std::string footer = "Generated from " + dbname +
                         ". Timing tiers measure how big each squeeze is, in steps "
                         "of your hit window. The Paths tab's row labels measure how "
                         "far a hit lands from the Star Power end, so the two can "
                         "differ. 'Beyond' means past the " + beyond_edge_text(w) +
                         " ms window.";
    out.html = build_html(rows, subtitle, footer, w);
    return out;
}

}  // namespace hydra::app::report
