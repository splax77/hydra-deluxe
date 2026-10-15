#include "app/report.h"

#include <algorithm>
#include <filesystem>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

#include "app/analysis.h"
#include "app/config.h"    // Settings::profile
#include "app/display_format.h"
#include "core/model.h"
#include "core/squeeze_rating.h"
#include "core/strutil.h"

namespace hydra::app::report {

namespace {

// The song ids of the rows at `shown`, for store::ChartIndex::count.
std::vector<std::string> ids_of(const std::vector<ReportRow>& rows,
                                const std::vector<size_t>& shown) {
    std::vector<std::string> ids;
    ids.reserve(shown.size());
    for (size_t i : shown) ids.push_back(rows[i].hyhash);
    return ids;
}

// Puts one stored record on the page: its first max_paths paths in their
// stored rank order, named from the library row that names the chart and
// labeled with their tier. The page's one turn of a record into rows.
void place_record(std::vector<ReportRow>& out, const store::RecordRow& record,
                  int64_t max_paths, const store::ChartLibraryEntry& naming,
                  const std::vector<TimingTier>& tiers) {
    const size_t shown = static_cast<size_t>(
        std::min<int64_t>(max_paths, static_cast<int64_t>(record.paths.size())));
    for (size_t i = 0; i < shown; ++i) {
        const store::PathRow& p = record.paths[i];
        ReportRow row;
        // The one cleaned title, artist and charter every screen shows.
        row.song = display_title(naming.title);
        row.artist = display_artist(naming.artist);
        row.charter = display_charter(naming.charter);
        row.rank = p.rank;
        row.optimal = p.optimal;
        row.path = p.path;
        row.score = p.score;
        row.acts = p.actcount;
        row.skip = p.maxskip;
        row.ms = p.hardest_ms;
        row.efill = p.hardest_efill;
        row.mult = p.avgmult;
        row.sqin = p.sqin_count;
        row.sqout = p.sqout_count;
        row.notes = record.notecount;
        std::tie(row.tier, row.tok) = tier_for(row.ms, tiers);
        row.hyhash = record.song_id;
        out.push_back(std::move(row));
    }
}

}  // namespace

std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             double hit_window_ms) {
    // The ladder itself lives in core/squeeze_rating.h (timing_tiers) so
    // these labels and every other reader of the tier table cannot drift
    // apart.
    return tier_for(ms, timing_tiers(hit_window_ms));
}

const TimingTier& beyond_tier(const std::vector<TimingTier>& tiers) {
    return tiers[tiers.size() - 2];
}

const TimingTier& none_tier(const std::vector<TimingTier>& tiers) { return tiers.back(); }

std::pair<std::string, std::string> tier_for(const std::optional<double>& ms,
                                             const std::vector<TimingTier>& tiers) {
    const TimingTier& none = none_tier(tiers);
    const TimingTier& beyond = beyond_tier(tiers);
    if (!ms) return {none.name, none.tok};
    // Each edge belongs to the band below it (D48 Q3). The table's first row
    // is Normal up to and including the difficult floor, so a timing on the
    // floor reads Normal (past_difficult_floor's rule), and a timing exactly
    // on the two-hit budget is Insane+, not Beyond.
    for (const TimingTier& t : tiers)
        if (t.cutoff && *ms <= *t.cutoff) return {t.name, t.tok};
    return {beyond.name, beyond.tok};
}

std::string beyond_edge_text(double hit_window_ms) {
    return std::to_string(static_cast<int64_t>(beyond_edge_ms(hit_window_ms)));
}

std::unordered_map<std::string, store::RecordRow> listed_records(
    store::RecordStore& store, const store::Profile& profile, const store::ChartIndex& library) {
    std::unordered_map<std::string, store::RecordRow> by_id;
    for (store::RecordRow& r : store.ready_records(profile))
        if (library.lists(r.song_id)) {
            std::string id = r.song_id;
            by_id.emplace(std::move(id), std::move(r));
        }
    return by_id;
}

namespace {

// Whether the store holds a result at any setting. The one place report.cpp
// reads RecordStore::counts().
bool holds_results(store::RecordStore& store) { return store.counts().second > 0; }

// lacks_chart_library's rule, for generate_report, which has already asked
// holds_results and passes its answer on.
bool lacks_chart_library(store::RecordStore& store, bool any_results) {
    return any_results && store.chart_library_count() == 0;
}

}  // namespace

bool lacks_chart_library(store::RecordStore& store) {
    return lacks_chart_library(store, holds_results(store));
}

CollectedRows collect_rows(store::RecordStore& store, const ReportOptions& options) {
    // Fail loudly rather than list paths found under other settings.
    if (!options.run)
        throw std::invalid_argument("collect_rows: the report has no analysis settings");
    CollectedRows out;
    out.library = store.chart_index();
    const std::unordered_map<std::string, store::RecordRow> records =
        listed_records(store, options.run->profile(), out.library);
    // Built once for the whole report, not once per row.
    const std::vector<TimingTier> tiers = timing_tiers(options.hit_window_ms);
    // The library's scan order, so the page's rows come out the same way
    // every time.
    for (const std::string& id : out.library.song_ids()) {
        const auto it = records.find(id);
        if (it == records.end()) continue;
        place_record(out.rows, it->second, options.max_paths, *out.library.naming(id), tiers);
    }
    return out;
}

std::vector<Tile> path_tiles(const std::vector<ReportRow>& rows,
                             const std::vector<size_t>& shown, double hit_window_ms,
                             const store::ChartIndex& library) {
    const std::string beyond = beyond_tier(timing_tiers(hit_window_ms)).name;
    // The row with the largest Hardest ms, the first of a tie; the tile shows
    // that row's text.
    const ReportRow* hardest = nullptr;
    int64_t past_edge = 0;
    int highest_skip = shown.empty() ? 0 : rows[shown.front()].skip;
    for (size_t i : shown) {
        const ReportRow& r = rows[i];
        if (r.ms && (!hardest || *r.ms > *hardest->ms)) hardest = &r;
        // The rows tier_for labeled Beyond.
        if (r.tier == beyond) ++past_edge;
        highest_skip = std::max(highest_skip, r.skip);
    }
    return {
        {"Charts shown", group_thousands(library.count(ids_of(rows, shown)))},
        {"Paths shown", group_thousands(static_cast<int64_t>(shown.size()))},
        // The Hardest ms column's own text (format_ms).
        {"Hardest ms", hardest ? format_ms(*hardest->ms) : std::string(kDash)},
        {"Past " + beyond_edge_text(hit_window_ms) + " ms", group_thousands(past_edge)},
        // Written without thousands separators, as the old page wrote it.
        {"Highest skip", std::to_string(highest_skip)},
    };
}

std::string nothing_under_settings(const std::string& chartmode, int cap,
                                   const std::string& fills, const std::string& ending) {
    return std::string(kNothingUnderSettings) + " (" + chartmode + ", SP cap " +
           group_thousands(cap) + (fills.empty() ? "" : ", " + fills) + ")" + ending +
           ". Analyze with these settings, or change them.";
}

GeneratedReport generate_report(store::RecordStore& store, const ReportOptions& options) {
    GeneratedReport out;
    const double w = options.hit_window_ms;
    out.hit_window_ms = w;
    CollectedRows collected = collect_rows(store, options);
    // Cancelled: nothing is built, and the caller that set the flag already
    // knows why.
    if (options.cancel && options.cancel->load()) {
        out.empty_reason = EmptyReason::Cancelled;
        return out;
    }
    out.library = std::move(collected.library);
    const std::vector<ReportRow>& rows = collected.rows;
    // collect_rows refused a report with no run, so it is set here.
    const BatchRun& run = *options.run;
    const FillDeadlineRule rule = fill_rule_for(run.lens.legacy_fills);
    out.rows = static_cast<int64_t>(rows.size());
    if (rows.empty()) {
        // Nothing listed. Either the database is empty, or it holds results
        // under other settings (or only stale ones), and the user needs to
        // hear which (finding 105).
        const bool any_results = holds_results(store);
        if (!any_results) {
            out.empty_reason = EmptyReason::NothingStored;
        } else if (lacks_chart_library(store, any_results)) {
            // The report covers library charts only (D87 item 4).
            out.empty_reason = EmptyReason::NoLibrary;
            out.why_empty = kNoChartLibrary;
        } else {
            out.empty_reason = EmptyReason::NothingUnderSettings;
            out.why_empty = nothing_under_settings(
                run.chartmode, run.cap_query().exact,
                std::string(fill_rule_name(rule, FillRuleNameStyle::Long)) + " fills");
        }
        return out;
    }
    std::vector<size_t> every_row(rows.size());
    std::iota(every_row.begin(), every_row.end(), size_t{0});
    out.songs = out.library.count(ids_of(rows, every_row));

    // Counts read the house rule (hydra::counted, D48 Q12). The cut is per
    // chart.
    std::string shown = options.max_paths >= kEveryPathSentinel
                            ? "every path"
                            : "top " + hydra::counted(options.max_paths, "path", "paths") +
                                  " per chart";
    std::string cap_label = "SP cap " + bars_text(run.cap_query().exact);
    // The normal rule goes unsaid, so a 1.1 page reads as it always has.
    if (rule != FillDeadlineRule::Ch11)
        cap_label += std::string(" — ") + fill_rule_name(rule, FillRuleNameStyle::Long) + " fills";
    std::string subtitle =
        hydra::counted(out.songs, "chart", "charts") + " — " + shown + " — " + cap_label;
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
    out.subtitle = std::move(subtitle);
    out.footer = std::move(footer);
    out.paths = std::move(collected.rows);
    return out;
}

PageSettings PageSettings::of(const Settings& settings) {
    // generate_report lists one profile (chart mode, SP cap and lens), and
    // labels its tiers from the hit window.
    return PageSettings{settings.profile(), settings.hit_window_ms};
}

bool settings_change_touches(const Settings& before, const Settings& after) {
    return PageSettings::of(before) != PageSettings::of(after);
}

}  // namespace hydra::app::report
