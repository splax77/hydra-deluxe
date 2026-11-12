// The one-time fill of the saved hardest figure. An older build left a saved
// path's figure empty where this build saves the engine's own figure
// (store::PathRow::hardest_ms). The store says which rows those are and
// writes what it is given; it reads no chart. This file does the reading: it
// replays each row's saved path text on its chart and hands the engine's
// figure back to the store. The startup open runs it once per profile
// (ui::StoreOpenJob).

#ifndef HYDRA_APP_SAVED_FIGURE_FILL_H
#define HYDRA_APP_SAVED_FIGURE_FILL_H

#include <cstdint>
#include <functional>
#include <string>

#include "app/analysis.h"  // batch_worker_count
#include "app/config.h"
#include "search/pathstring_read.h"
#include "store/record_store.h"

namespace hydra::app {

// Charts done out of charts found. Called on the calling thread before the
// first chart, after each chart's figures are written, and once more after
// the last. Returning false stops the fill: no chart not yet started is read,
// and no chart that finishes after the answer is written.
using SavedFigureProgress = std::function<bool(uint64_t charts_done, uint64_t charts_total)>;

// Fills in the saved hardest figure of every row
// store::RecordStore::rows_missing_hardest finds under `settings`' profile,
// then records that the fill has run for that profile. Returns at once when
// it already has (RecordStore::hardest_figure_filled), and calls `progress`
// only when there is a chart to read.
//
// The charts are read and replayed on `worker_count` threads of the batch's
// pool (run_work_pool, app/work_pool.h), and each chart's figures are written
// together on the calling thread as that chart finishes, so charts finish in
// any order. `worker_count` must be at least 1, as run_work_pool asks.
//
// Chart by chart, a chart whose files changed since the scan, or are gone
// (chart_changed_since), keeps its rows as they are. Otherwise the chart is
// read once under the settings' reading, rules and fill rule, its search graph
// is built once (search_graph), and each row's path text is replayed on that
// graph (`replay`, below). A replay is used only
// when its text, score and squeeze counts, read the way the store writes them
// (store::record_of), equal the row's. The figure written is the replayed
// path's engine_hardest_ms. A row whose text the reader refuses, or whose
// replay differs, keeps its empty figure.
//
// A chart that no longer reads, or whose replay or comparison with a row
// throws anything else, is skipped: every one of its rows keeps its empty
// figure, and the fill goes on with the other charts. The marker is still
// set at the end, so a skipped chart is not tried again.
//
// Returns true when the fill has run for the profile, false when `progress`
// answered stop. A stop leaves the marker unset and the charts already
// written as they are, so the next open goes on with the rows still empty.
// Throws what the store throws, such as a failed write of a figure or of the
// marker; that stops the fill with the marker unset.
//
// `replay` is how a row's path text is replayed on its chart's graph. The app
// passes none and gets paths_from_pathstring_on_graph; a test passes one that
// throws for a chart it picks, or that notes the graph it is handed. It runs
// on the workers, so it must be safe to call from several threads at once.
// The app passes no `worker_count` either and gets the batch's own count; a
// test passes one to compare counts.
using PathReplay =
    std::function<Path(const Song& song, const ScoreGraph& graph, const std::string& text)>;
bool fill_saved_hardest(store::RecordStore& store, const Settings& settings,
                        const SavedFigureProgress& progress,
                        const PathReplay& replay = paths_from_pathstring_on_graph,
                        int worker_count = batch_worker_count());

}  // namespace hydra::app

#endif  // HYDRA_APP_SAVED_FIGURE_FILL_H
