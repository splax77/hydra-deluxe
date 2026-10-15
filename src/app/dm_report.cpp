#include "app/dm_report.h"

#include <stdexcept>
#include <unordered_map>

#include "app/config.h"          // Settings::with_known_chartmode, profile_at_clone_hero_cap
#include "app/display_format.h"  // format_percent, percent_steps
#include "app/report.h"  // lacks_chart_library, kNoChartLibrary
#include "core/error_kind.h"
#include "core/model.h"  // counted, group_thousands
#include "parse/song.h"  // display_title, display_artist, display_charter
#include "search/graph.h"  // fill_rule_for, fill_rule_name
#include "store/chart_index.h"

namespace hydra::app::dm_report {

namespace {

// Counts one score into `stats` by its status. Both tally_dm_rows overloads go
// through it.
void count_status(DmReportStats& stats, const DmReportRow& r) {
    ++stats.total;
    ++(stats.*status_words(r.status).count);
}

}  // namespace

const StatusWords& status_words(const std::string& status) {
    for (const StatusWords& w : kStatusWords)
        if (status == w.status) return w;
    return status_words(kStatusNotInLibrary);  // in the table, so this returns
}

std::string why_not_comparable(Difficulty difficulty, int sp_cap, bool legacy_fills,
                               bool note_shuffle) {
    if (difficulty != Difficulty::Expert)
        return "Needs Expert: the leaderboard only has Expert scores.";
    if (!is_clone_hero_cap(sp_cap))
        return "Needs SP cap " + std::to_string(kCloneHeroSpCap) +
               ", Clone Hero's rule: the leaderboard's scores were played under it.";
    // The fill rule the board's scores were played under.
    const FillDeadlineRule board_rule = FillDeadlineRule::Ch11;
    if (fill_rule_for(legacy_fills) != board_rule)
        return std::string("Needs ") + fill_rule_name(board_rule, FillRuleNameStyle::Long) +
               " fills: untick \"1.0 fills\". The leaderboard is played on current Clone Hero.";
    // D104 item 6: a board score never records its modifiers.
    if (note_shuffle)
        return "dmleaderboards scores don't say whether Note Shuffle was on, so they can't be "
               "compared with a shuffled path. Untick Note Shuffle to compare.";
    return std::string();
}

std::vector<DmReportRow> collect_dm_rows(store::RecordStore& store,
                                         const std::vector<net::DmScore>& scores,
                                         const store::Profile& profile) {
    // The Clone Hero rules come from why_not_comparable. Of its four inputs,
    // the cap and the fill rule arrive in `profile`, and the difficulty and
    // Note Shuffle in its chart mode, read back through its owner. A profile
    // at another cap is refused: a what-if cap's score would read as "above
    // optimal" nonsense. The window's rebuild buttons reach here without the
    // toolbar's check, so this call is the gate for them (D114 item 6).
    const Settings mode = Settings{}.with_known_chartmode(profile.chartmode);
    const std::string refused =
        why_not_comparable(mode.difficulty(), profile.cap.exact, profile.lens.legacy_fills != 0,
                           mode.view_noteshuffle);
    if (!refused.empty()) throw KindedError(ErrorKind::AlreadyPlain, refused);
    // The page compares library charts only, as the path report does (D92).
    if (report::lacks_chart_library(store))
        throw KindedError(ErrorKind::AlreadyPlain, report::kNoChartLibrary);

    // Every chart the last scan found, so a score with no current result can
    // say whether analyzing would fix it, and so a joined record has names.
    // The page counts scores, not charts, so the copies go unused (D79).
    const store::ChartIndex library = store.chart_index();

    // One read of every Ready record under the profile, indexed by song id.
    // A result the library doesn't list is left out, as the path report
    // leaves it out (D87 item 4, D92).
    std::unordered_map<std::string, store::RecordRow> by_id;
    for (store::RecordRow& r : store.ready_records(profile))
        if (library.lists(r.song_id)) {
            std::string id = r.song_id;
            by_id.emplace(std::move(id), std::move(r));
        }

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

        auto it = by_id.find(s.identifier);
        const store::RecordRow* rec = it != by_id.end() ? &it->second : nullptr;
        // A joined record's chart is listed, so it has a naming copy.
        const store::ChartLibraryEntry* named = rec ? library.naming(rec->song_id) : nullptr;

        // Identity: the leaderboard's own metadata when it has it, else the
        // joined chart's library names, else whatever the leaderboard sent.
        // Either source goes through the same display owners (D74 item 2).
        const bool board_names = (s.known && !s.song_name.empty()) || !named;
        row.song = display_title(board_names ? s.song_name : named->title);
        row.artist = display_artist(board_names ? s.artist : named->artist);
        row.charter = display_charter(board_names ? s.charter : named->charter);
        if (row.charter.empty() && named) row.charter = display_charter(named->charter);

        // Hydra's optimal is for normal speed only. An off-speed score shows
        // Hydra's numbers when it has them, but is never called under, at or
        // above optimal.
        const bool base = net::is_base_speed(s.speed);
        if (rec && rec->has_scored_best_path()) {
            int64_t opt = *rec->summary().score;
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
            row.status = library.lists(s.identifier) ? kStatusNoCurrentResult : kStatusNotInLibrary;
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
    if (status == kStatusNoCurrentResult || status == kStatusNoPaths ||
        status == kStatusOtherSpeed)
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
    // A status with no tile ("no paths", D62 item 1) still counts in Scores
    // shown.
    std::vector<report::Tile> tiles = {{"Scores shown", group_thousands(stats.total)}};
    for (const StatusWords& w : kStatusWords)
        if (w.tile) tiles.push_back({w.name, group_thousands(stats.*w.count)});
    tiles.push_back({"Avg % of optimal", average});
    tiles.push_back({"Points left on table", group_thousands(points_left)});
    return tiles;
}

std::string counts_phrase(const DmReportStats& stats) {
    std::string out;
    auto add = [&](const StatusWords& w) {
        if (!out.empty()) out += ", ";
        out += hydra::counted(stats.*w.count, w.count_one,
                              w.count_many ? w.count_many : w.count_one);
    };
    // Every status always, then the ones named only when there are any, so
    // every other phrase reads as before (D62).
    for (const StatusWords& w : kStatusWords)
        if (!w.count_when_any) add(w);
    for (const StatusWords& w : kStatusWords)
        if (w.count_when_any && stats.*w.count > 0) add(w);
    return out;
}

GeneratedDmReport generate_dm_report(store::RecordStore& store,
                                     const std::vector<net::DmScore>& scores,
                                     const store::Profile& profile,
                                     const std::string& username) {
    GeneratedDmReport out;
    out.username = username;
    out.chartmode = profile.chartmode;
    const std::string& chartmode = profile.chartmode;
    std::vector<DmReportRow> rows = collect_dm_rows(store, scores, profile);
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
        "Charts with " + status_words(kStatusNoCurrentResult).count_one +
        " are in your library without a result for this mode at SP cap " +
        std::to_string(kCloneHeroSpCap) + ": analyze them, then compare again.";
    out.subtitle = std::move(subtitle);
    out.footer = std::move(footer);
    out.rows = std::move(rows);
    return out;
}

bool settings_change_touches(const Settings& before, const Settings& after) {
    // generate_dm_report lists the one profile the comparison names, which
    // is at Clone Hero's SP cap whatever the cap setting, so the cap reaches
    // no row.
    return before.profile_at_clone_hero_cap() != after.profile_at_clone_hero_cap();
}

}  // namespace hydra::app::dm_report
