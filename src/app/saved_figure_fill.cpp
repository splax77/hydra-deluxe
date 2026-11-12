#include "app/saved_figure_fill.h"

#include <atomic>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"  // chart_changed_since, AnalysisSettings
#include "app/work_pool.h"
#include "core/error_kind.h"
#include "core/model.h"  // engine_hardest_ms
#include "parse/song.h"
#include "search/graph.h"  // fill_rule_for
#include "search/pather.h"  // search_graph
#include "search/pathstring_read.h"

namespace hydra::app {

namespace {

using store::HardestFigure;
using store::MissingHardestRow;

// Whether `replayed` is the path `row` was saved from: its facts read the way
// the store writes a row (store::record_of), compared with the row's own.
bool replay_matches(const MissingHardestRow& row, const Path& replayed) {
    HydraRecord one;
    one.paths.push_back(replayed);
    const store::PathRow facts = store::record_of(row.song_id, one).paths.front();
    return facts.path == row.path && facts.score == row.score &&
           facts.sqin_count == row.sqin_count && facts.sqout_count == row.sqout_count;
}

// Whether the chart file the rows name is still the one they were saved
// from: the rescan's own answer (chart_changed_since), as the click asks it.
// A file that hashes to the row's id again was only saved again.
bool chart_still_there(const MissingHardestRow& row) {
    const std::optional<ChartNow> now = chart_changed_since(row.notespath, row.sig);
    if (!now) return true;
    return !now->md5.empty() && !now->differs_from(row.song_id);
}

// The figures one profile row's rows of a chart get.
struct ProfileFigures {
    int64_t profile_id = 0;
    std::vector<HardestFigure> figures;
};

// The figures for one chart's rows, `first` to `last`, read off `song` on its
// one `graph`, one entry per profile row. Each row gets a figure only when its
// replay matches it; a row whose text the reader refuses gets none. Throws
// anything else `replay` or the comparison throws.
std::vector<ProfileFigures> replayed_figures(const Song& song, const ScoreGraph& graph,
                                             const PathReplay& replay,
                                             std::vector<MissingHardestRow>::const_iterator first,
                                             std::vector<MissingHardestRow>::const_iterator last) {
    // The rows come sorted by profile row, then rank (rows_missing_hardest),
    // so each profile row's rows sit together.
    std::vector<ProfileFigures> out;
    for (auto it = first; it != last; ++it) {
        if (out.empty() || out.back().profile_id != it->profile_id)
            out.push_back(ProfileFigures{it->profile_id, {}});
        std::optional<Path> replayed;
        try {
            replayed = replay(song, graph, it->path);
        } catch (const KindedError& e) {
            if (e.kind() != ErrorKind::PathTextRefused) throw;
            continue;
        }
        if (!replay_matches(*it, *replayed)) continue;
        if (const std::optional<double> figure = engine_hardest_ms(*replayed))
            out.back().figures.push_back(HardestFigure{it->rank, *figure});
    }
    return out;
}

// One chart's figures, found on a worker: the chart's place in the fill's
// list, and its figures profile row by profile row. A skipped chart has none.
struct ChartFigures {
    size_t chart = 0;
    std::vector<ProfileFigures> found;
};

// The figures for one chart's rows, `first` to `last`. Runs on a pool worker,
// so it never throws (run_work_pool). The chart is read, its graph built once
// (search_graph) and every row replayed on that graph before any figure is
// kept, so a chart that throws there keeps all its rows as they are.
ChartFigures chart_figures(size_t chart, const AnalysisSettings& settings,
                           const PathReplay& replay,
                           std::vector<MissingHardestRow>::const_iterator first,
                           std::vector<MissingHardestRow>::const_iterator last) {
    ChartFigures out;
    out.chart = chart;
    try {
        if (!chart_still_there(*first)) return out;
        const Song song = load_songpath(first->notespath, settings.reading(), settings.rules,
                                        fill_rule_for(settings.legacy_fills));
        const ScoreGraph graph = search_graph(song, settings);
        out.found = replayed_figures(song, graph, replay, first, last);
    } catch (...) {
        // A chart whose files can't be checked or read, or whose replay throws,
        // keeps its rows empty, as a gone one does. The fault is the song's,
        // not the store's, so the fill goes on with the other charts.
    }
    return out;
}

}  // namespace

bool fill_saved_hardest(store::RecordStore& store, const Settings& settings,
                        const SavedFigureProgress& progress, const PathReplay& replay,
                        int worker_count) {
    const store::Profile profile = settings.profile();
    if (store.hardest_figure_filled(profile)) return true;

    const std::vector<MissingHardestRow> rows = store.rows_missing_hardest(profile);
    // Where each chart's rows start: they come sorted by chart.
    std::vector<size_t> starts;
    for (size_t i = 0; i < rows.size(); ++i)
        if (i == 0 || rows[i].song_id != rows[i - 1].song_id) starts.push_back(i);
    starts.push_back(rows.size());
    const auto row_at = [&](size_t i) { return rows.begin() + static_cast<std::ptrdiff_t>(i); };

    const AnalysisSettings analysis = settings.to_analysis_settings();
    const uint64_t charts = starts.size() - 1;
    if (charts > 0 && progress && !progress(0, charts)) return false;

    // What runs on the workers, what runs here, and what a stop leaves: the
    // header, fill_saved_hardest.
    std::atomic<bool> stop{false};
    uint64_t done = 0;
    run_work_pool<ChartFigures>(
        static_cast<size_t>(charts), worker_count, &stop,
        [&](size_t c) {
            return chart_figures(c, analysis, replay, row_at(starts[c]), row_at(starts[c + 1]));
        },
        [&](ChartFigures&& chart) {
            if (stop.load()) return;
            // A failed write is the store's fault, not the song's: it throws on,
            // out of the fill.
            const std::string& song_id = rows[starts[chart.chart]].song_id;
            for (const ProfileFigures& p : chart.found)
                if (!p.figures.empty()) store.set_hardest(song_id, p.profile_id, p.figures);
            ++done;
            // The last chart's count goes out after the marker, below.
            if (done < charts && progress && !progress(done, charts)) stop.store(true);
        });
    if (done < charts) return false;
    store.mark_hardest_figure_filled(profile);
    // Every chart is done, so the answer has nothing left to stop.
    if (progress && charts > 0) progress(charts, charts);
    return true;
}

}  // namespace hydra::app
