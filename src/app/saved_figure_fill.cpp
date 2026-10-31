#include "app/saved_figure_fill.h"

#include <cstddef>
#include <exception>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"  // chart_changed_since, AnalysisSettings
#include "core/error_kind.h"
#include "core/model.h"  // engine_hardest_ms
#include "parse/song.h"
#include "search/graph.h"  // fill_rule_for
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

// The figures for one chart's rows, `first` to `last`, written profile row by
// profile row. Each row keeps its empty figure unless its replay matches it.
void fill_chart(store::RecordStore& store, const AnalysisSettings& settings,
                std::vector<MissingHardestRow>::const_iterator first,
                std::vector<MissingHardestRow>::const_iterator last) {
    if (!chart_still_there(*first)) return;
    std::optional<Song> song;
    try {
        song.emplace(load_songpath(first->notespath, settings.reading(), settings.rules,
                                   fill_rule_for(settings.legacy_fills)));
    } catch (const std::exception&) {
        return;  // a chart that no longer reads keeps its rows, as a gone one does
    }

    // The rows come sorted by profile row, then rank (rows_missing_hardest),
    // so each profile row's figures are written together.
    std::vector<HardestFigure> figures;
    int64_t profile_id = first->profile_id;
    auto write = [&] {
        if (!figures.empty()) store.set_hardest(first->song_id, profile_id, figures);
        figures.clear();
    };
    for (auto it = first; it != last; ++it) {
        if (it->profile_id != profile_id) {
            write();
            profile_id = it->profile_id;
        }
        std::optional<Path> replayed;
        try {
            replayed = paths_from_pathstring(*song, settings, it->path);
        } catch (const KindedError& e) {
            if (e.kind() != ErrorKind::PathTextRefused) throw;
            continue;
        }
        if (!replay_matches(*it, *replayed)) continue;
        if (const std::optional<double> figure = engine_hardest_ms(*replayed))
            figures.push_back(HardestFigure{it->rank, *figure});
    }
    write();
}

}  // namespace

bool fill_saved_hardest(store::RecordStore& store, const Settings& settings,
                        const SavedFigureProgress& progress) {
    const store::Profile profile = settings.profile();
    if (store.hardest_figure_filled(profile)) return true;

    const std::vector<MissingHardestRow> rows = store.rows_missing_hardest(profile);
    // Where each chart's rows start: they come sorted by chart.
    std::vector<size_t> starts;
    for (size_t i = 0; i < rows.size(); ++i)
        if (i == 0 || rows[i].song_id != rows[i - 1].song_id) starts.push_back(i);
    starts.push_back(rows.size());

    const AnalysisSettings analysis = settings.to_analysis_settings();
    const uint64_t charts = starts.size() - 1;
    for (uint64_t c = 0; c < charts; ++c) {
        if (progress && !progress(c, charts)) return false;
        fill_chart(store, analysis, rows.begin() + static_cast<std::ptrdiff_t>(starts[c]),
                   rows.begin() + static_cast<std::ptrdiff_t>(starts[c + 1]));
    }
    store.mark_hardest_figure_filled(profile);
    // Every chart is done, so the answer has nothing left to stop.
    if (progress && charts > 0) progress(charts, charts);
    return true;
}

}  // namespace hydra::app
