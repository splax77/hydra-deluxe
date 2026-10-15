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
#include "app/user_messages.h"  // plain_error
#include "core/error_kind.h"
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

LibraryLookup library_lookup(store::RecordStore& store, const store::Profile& profile,
                             const store::ChartIndex& library) {
    LibraryLookup out;
    const std::vector<std::string>& ids = library.song_ids();
    std::vector<store::RecordLookup> found = store.lookup(ids, profile);
    for (size_t i = 0; i < ids.size(); ++i) {
        store::RecordLookup& l = found[i];
        switch (l.status) {
            case store::RecordStatus::Ready:
                out.ready.emplace(ids[i], std::move(l.record));
                break;
            case store::RecordStatus::CantAnalyze: {
                // The library lists the chart, so it has a naming copy.
                const store::ChartLibraryEntry& naming = *library.naming(ids[i]);
                out.cant_analyze.push_back(CantAnalyzeChart{
                    ids[i], display_title(naming.title), display_artist(naming.artist),
                    *l.failure_kind, std::move(l.failure_text)});
                break;
            }
            case store::RecordStatus::NotAnalyzed:
            case store::RecordStatus::Stale:
                out.no_result.push_back(ids[i]);
                break;
        }
    }
    return out;
}

namespace {

// How many library rows the charts `ids` count as. Every chart count on the
// page asks store::ChartIndex::count (D76, D77), through ids_of or here.
int64_t charts_counted(const store::ChartIndex& index, const std::vector<std::string>& ids) {
    return index.count(ids);
}

std::vector<std::string> ids_of(const std::vector<CantAnalyzeChart>& charts) {
    std::vector<std::string> ids;
    ids.reserve(charts.size());
    for (const CantAnalyzeChart& c : charts) ids.push_back(c.song_id);
    return ids;
}

// The reason a stored failure gives, worded by app::plain_error, the one
// owner of failure sentences, from the stored kind and text.
std::string failure_sentence(const CantAnalyzeChart& chart) {
    return plain_error(KindedError(chart.kind, chart.text));
}

// The can't-analyze charts grouped by their sentence, each group in the
// order its reason first appears in the library, its charts in library
// order.
std::vector<CantAnalyzeGroup> group_by_reason(const std::vector<CantAnalyzeChart>& charts,
                                              const store::ChartIndex& library) {
    std::vector<CantAnalyzeGroup> groups;
    for (const CantAnalyzeChart& c : charts) {
        std::string sentence = failure_sentence(c);
        auto it = std::find_if(groups.begin(), groups.end(), [&](const CantAnalyzeGroup& g) {
            return g.sentence == sentence;
        });
        if (it == groups.end()) {
            groups.push_back(CantAnalyzeGroup{std::move(sentence), 0, {}});
            it = groups.end() - 1;
        }
        it->charts.push_back(c);
    }
    for (CantAnalyzeGroup& g : groups) g.count = charts_counted(library, ids_of(g.charts));
    return groups;
}

// The words after the count when every can't-analyze chart shares one
// reason, read off that reason's sentence; empty when its kind has no short
// form, and the line falls back to the generic words. A NoNotes sentence is
// NoNotesError's own text (no_notes_message in parse/song.cpp), "No <part>
// notes in this chart.", so the short form is "no <part> notes".
std::string shared_reason_words(ErrorKind kind, const std::string& sentence) {
    static const std::string kHead = "No ";
    static const std::string kTail = " in this chart.";
    if (kind != ErrorKind::NoNotes) return std::string();
    if (sentence.size() <= kHead.size() + kTail.size() || !hydra::starts_with(sentence, kHead) ||
        !hydra::ends_with(sentence, kTail))
        return std::string();
    return "no " + sentence.substr(kHead.size(), sentence.size() - kHead.size() - kTail.size());
}

// The coverage line for the can't-analyze charts (the user's answer 5,
// 2026-10-15): the shared reason when there is one with a short form, else
// the generic words. Empty when there are none.
std::string cant_analyze_line(const std::vector<CantAnalyzeGroup>& groups, int64_t count) {
    if (count == 0) return std::string();
    if (groups.size() == 1) {
        const std::string words =
            shared_reason_words(groups.front().charts.front().kind, groups.front().sentence);
        if (!words.empty()) return hydra::counted(count, "chart has", "charts have") + " " + words;
    }
    return hydra::counted(count, "chart", "charts") + " can't be analyzed";
}

// The coverage line for the charts with no result. Empty when there are none.
std::string no_result_line(int64_t count) {
    if (count == 0) return std::string();
    return hydra::counted(count, "chart has", "charts have") + " no result under these settings";
}

// Whether the store holds a record under any profile. The one place report.cpp
// reads RecordStore::record_count().
bool holds_results(store::RecordStore& store) { return store.record_count() > 0; }

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
    LibraryLookup found = library_lookup(store, options.run->profile, out.library);
    // Built once for the whole report, not once per row.
    const std::vector<TimingTier> tiers = timing_tiers(options.hit_window_ms);
    // The library's scan order, so the page's rows come out the same way
    // every time.
    for (const std::string& id : out.library.song_ids()) {
        const auto it = found.ready.find(id);
        if (it == found.ready.end()) continue;
        place_record(out.rows, it->second, options.max_paths, *out.library.naming(id), tiers);
    }
    out.cant_analyze = std::move(found.cant_analyze);
    out.no_result = std::move(found.no_result);
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
        {"Charts shown", group_thousands(library.count(ids_of(rows, shown))) + " of " +
                             group_thousands(charts_counted(library, library.song_ids()))},
        {"Paths shown", group_thousands(static_cast<int64_t>(shown.size()))},
        // The Hardest ms column's own text (format_ms).
        {"Hardest ms", hardest ? format_ms(*hardest->ms) : std::string(kDash)},
        {"Past " + beyond_edge_text(hit_window_ms) + " ms", group_thousands(past_edge)},
        // Written without thousands separators, as the old page wrote it.
        {"Highest skip", std::to_string(highest_skip)},
    };
}

std::string nothing_under_settings(const std::string& chartmode, int cap,
                                   const std::string& fills) {
    return std::string(kNothingUnderSettings) + " (" + chartmode + ", SP cap " +
           group_thousands(cap) + (fills.empty() ? "" : ", " + fills) +
           "). Analyze with these settings, or change them.";
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
    const store::Profile& profile = options.run->profile;
    const FillDeadlineRule rule = fill_rule_for(profile.lens.legacy_fills);
    out.rows = static_cast<int64_t>(rows.size());
    // The page's coverage: every library chart is shown, can't be analyzed,
    // or has no result (the user's answer 5, 2026-10-15).
    out.library_charts = charts_counted(out.library, out.library.song_ids());
    out.cant_analyze_charts = charts_counted(out.library, ids_of(collected.cant_analyze));
    out.cant_analyze = group_by_reason(collected.cant_analyze, out.library);
    out.no_result_charts = charts_counted(out.library, collected.no_result);
    out.cant_analyze_line = cant_analyze_line(out.cant_analyze, out.cant_analyze_charts);
    out.no_result_line = no_result_line(out.no_result_charts);
    if (rows.empty()) {
        // Nothing listed. A stored failure is a result, so the coverage lines
        // say why; otherwise the database is empty, or it holds results
        // under other settings (or only stale ones), and the user needs to
        // hear which (finding 105).
        const bool any_results = holds_results(store);
        if (!collected.cant_analyze.empty()) {
            out.empty_reason = EmptyReason::NothingToShow;
            out.why_empty = kNothingToShow;
        } else if (!any_results) {
            out.empty_reason = EmptyReason::NothingStored;
        } else if (lacks_chart_library(store, any_results)) {
            // The report covers library charts only (D87 item 4).
            out.empty_reason = EmptyReason::NoLibrary;
            out.why_empty = kNoChartLibrary;
        } else {
            out.empty_reason = EmptyReason::NothingUnderSettings;
            out.why_empty = nothing_under_settings(
                profile.chartmode, profile.cap.exact,
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
    std::string cap_label = "SP cap " + bars_text(profile.cap.exact);
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
