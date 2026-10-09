#include "app/dm_report.h"

#include <stdexcept>
#include <unordered_map>

#include "app/config.h"          // Settings::chartmode_key, Settings::lens
#include "app/display_format.h"  // format_percent, percent_steps
#include "app/report.h"  // records_by_hash, library_copies_by_hash
#include "core/error_kind.h"
#include "core/model.h"  // counted, group_thousands
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "search/graph.h"  // fill_rule_name

namespace hydra::app::dm_report {

namespace {

// Counts one score into `stats` by its status. Both tally_dm_rows overloads go
// through it.
void count_status(DmReportStats& stats, const DmReportRow& r) {
    ++stats.total;
    if (r.status == kStatusUnderOptimal) ++stats.under_optimal;
    else if (r.status == kStatusAtOptimal) ++stats.at_optimal;
    else if (r.status == kStatusAboveOptimal) ++stats.above_optimal;
    else if (r.status == kStatusNotAnalyzed) ++stats.not_analyzed;
    else if (r.status == kStatusNoPaths) ++stats.no_paths;
    else if (r.status == kStatusOtherSpeed) ++stats.other_speed;
    else ++stats.not_in_library;
}

}  // namespace

std::string why_not_comparable(Difficulty difficulty, int sp_cap, bool legacy_fills,
                               bool note_shuffle) {
    if (difficulty != Difficulty::Expert)
        return "Needs Expert: the leaderboard only has Expert scores.";
    if (sp_cap != kCloneHeroSpCap)
        return "Needs SP cap " + std::to_string(kCloneHeroSpCap) +
               ", Clone Hero's rule: the leaderboard's scores were played under it.";
    if (legacy_fills)
        return std::string("Needs ") +
               fill_rule_name(FillDeadlineRule::Ch11, FillRuleNameStyle::Long) +
               " fills: untick \"1.0 fills\". The leaderboard is played on current Clone Hero.";
    // D104 item 6: a board score never records its modifiers.
    if (note_shuffle)
        return "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be "
               "compared with a shuffled path.";
    return std::string();
}

std::vector<DmReportRow> collect_dm_rows(store::RecordStore& store,
                                         const std::vector<net::DmScore>& scores,
                                         const std::string& chartmode,
                                         const store::Lens& lens) {
    // The Clone Hero rules come from why_not_comparable. Of its four inputs,
    // the fill rule arrives in `lens` and Note Shuffle in `chartmode`, read
    // back through its owner. The cap is forced to Clone Hero's below, and the
    // difficulty is folded into `chartmode`, so the caller's settings take
    // that rule to the same owner (the library toolbar asks it for all four
    // before a comparison can start).
    const std::optional<Settings> mode = Settings{}.with_chartmode(chartmode);
    const std::string refused =
        why_not_comparable(Difficulty::Expert, kCloneHeroSpCap, lens.legacy_fills != 0,
                           mode && mode->view_noteshuffle);
    if (!refused.empty()) throw KindedError(ErrorKind::AlreadyPlain, refused);
    // The page compares library charts only, as the path report does (D92).
    if (report::lacks_chart_library(store))
        throw KindedError(ErrorKind::AlreadyPlain, report::kNoChartLibrary);

    // One query for every stored record in this chartmode, indexed by hash.
    // Only records at Clone Hero's cap: a what-if cap's score would read as
    // "above optimal" nonsense.
    const std::unordered_map<std::string, store::RecordListing> by_hash =
        report::records_by_hash(store, chartmode, store::CapQuery::at(kCloneHeroSpCap), lens);

    // Every chart the last scan found, keyed like the leaderboard join, so a
    // score with no current result can say whether analyzing would fix it.
    // The page counts scores, not charts, so the copies go unused (D79).
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
            if (base && opt > 0) row.pct_h = percent_steps(s.score, opt, kPercentDecimals);
            const bool above = s.score > opt;
            row.status = above            ? kStatusAboveOptimal
                         : s.score == opt ? kStatusAtOptimal
                                          : kStatusUnderOptimal;
            // Kept apart from the status, which an off-speed score overwrites
            // below; the page's "+N over" reads it (D64).
            row.above_optimal = above;
        } else if (rec) {
            // A Ready result whose analysis kept no path (D51 call 11).
            row.status = kStatusNoPaths;
        } else {
            row.status = report::library_lists(library, s.identifier) ? kStatusNotAnalyzed
                                                                       : kStatusNotInLibrary;
        }
        if (!base) row.status = kStatusOtherSpeed;
        rows.push_back(std::move(row));
    }
    return rows;
}

DmReportStats tally_dm_rows(const std::vector<DmReportRow>& rows) {
    DmReportStats stats;
    for (const DmReportRow& r : rows) count_status(stats, r);
    return stats;
}

DmReportStats tally_dm_rows(const std::vector<DmReportRow>& rows,
                            const std::vector<size_t>& shown) {
    DmReportStats stats;
    for (size_t i : shown) count_status(stats, rows[i]);
    return stats;
}

report::ChipToken status_token(const std::string& status) {
    using report::ChipToken;
    if (status == kStatusUnderOptimal || status == kStatusAtOptimal) return ChipToken::t0;
    if (status == kStatusAboveOptimal) return ChipToken::t1;
    if (status == kStatusNotAnalyzed || status == kStatusNoPaths || status == kStatusOtherSpeed)
        return ChipToken::muted;
    // kStatusNotInLibrary, and a status the page has no class for.
    return ChipToken::tn;
}

std::vector<report::Tile> dm_tiles(const std::vector<DmReportRow>& rows,
                                   const std::vector<size_t>& shown) {
    const DmReportStats stats = tally_dm_rows(rows, shown);
    int64_t pct_sum = 0;
    int64_t pct_rows = 0;
    int64_t points_left = 0;
    for (size_t i : shown) {
        const DmReportRow& r = rows[i];
        if (r.pct_h) {
            pct_sum += *r.pct_h;
            ++pct_rows;
        }
        // Only a score under optimal leaves points on the table, and its
        // delta is the points it left.
        if (r.status == kStatusUnderOptimal && r.delta) points_left += *r.delta;
    }
    // The mean of the cells' percents. Each pct_h is in format_percent's
    // steps, so the sum over pct_rows rows of 100% each, in those steps, is
    // the mean as format_percent writes it: the same rounding as a cell, so
    // one row's tile reads exactly its cell.
    std::string average = report::kDash;
    if (pct_rows > 0)
        average = format_percent(
            pct_sum, pct_rows * percent_steps(1, 1, kPercentDecimals), kPercentDecimals);
    // A "no paths" row has no tile of its own (D62 item 1); Scores shown counts
    // it.
    return {
        {"Scores shown", group_thousands(stats.total)},
        {"Under optimal", group_thousands(stats.under_optimal)},
        {"At optimal", group_thousands(stats.at_optimal)},
        {"Above optimal", group_thousands(stats.above_optimal)},
        {"Not analyzed", group_thousands(stats.not_analyzed)},
        {"Not in library", group_thousands(stats.not_in_library)},
        {"Other speed", group_thousands(stats.other_speed)},
        {"Avg % of optimal", average},
        {"Points left on table", group_thousands(points_left)},
    };
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
    out.username = username;
    out.chartmode = chartmode;
    std::vector<DmReportRow> rows = collect_dm_rows(store, scores, chartmode, lens);
    out.stats = tally_dm_rows(rows);
    if (rows.empty()) return out;

    // Every score, whatever the filters show; dm_tiles counts the rows that
    // pass them (D103 item 27).
    std::string subtitle =
        username + " — " + hydra::counted(out.stats.total, "score", "scores");
    std::string footer =
        "Actual scores from dmleaderboards.com against Hydra's optimal for " + chartmode +
        ". Above-optimal scores are expected — Hydra's optimal excludes several score "
        "backends, and older Clone Hero versions allowed fills that are impossible now. "
        "Not analyzed charts are in your library without a current result for this mode "
        "at SP cap " + std::to_string(kCloneHeroSpCap) + ": analyze them, then compare again.";
    out.subtitle = std::move(subtitle);
    out.footer = std::move(footer);
    out.rows = std::move(rows);
    return out;
}

bool settings_change_touches(const Settings& before, const Settings& after) {
    // generate_dm_report is asked for one chart mode and one lens. Its results
    // are always at Clone Hero's SP cap, so the cap setting reaches no row.
    return before.chartmode_key() != after.chartmode_key() || before.lens() != after.lens();
}

}  // namespace hydra::app::dm_report
