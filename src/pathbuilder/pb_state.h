// The Path Builder's open chart and the player's path: what a chart open
// loads (the settings, the parsed song, Hydra's record and the search graph),
// the worker that loads it, and undo and redo over the player's path. No
// ImGui and no window; pb_window.cpp draws from it.

#ifndef HYDRA_PATHBUILDER_PB_STATE_H
#define HYDRA_PATHBUILDER_PB_STATE_H

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "app/analysis.h"  // AnalysisSettings
#include "app/config.h"
#include "app/job_base.h"
#include "core/model.h"
#include "pathbuilder/pb_coach.h"
#include "search/graph.h"
#include "search/pather.h"  // PinnedWindow
#include "store/record_store.h"  // ChartLibraryEntry

namespace hydra::pathbuilder {

// One chart, opened: everything the builder reads while it shows the chart.
struct OpenChart {
    explicit OpenChart(Song song_in) : song(std::move(song_in)) {}
    // The graph keeps a reference to `song` (ScoreGraph), so an open chart
    // never moves: it lives behind the unique_ptr load_open_chart returns.
    OpenChart(const OpenChart&) = delete;
    OpenChart& operator=(const OpenChart&) = delete;

    std::string chart_path;  // the file opened
    // The settings the chart was read and searched under: Settings::batch_run()
    // of the hydra_settings.ini and hydra_rules.ini read at the open.
    app::AnalysisSettings settings;
    Song song;
    HydraRecord record;
    // The graph every search on this chart runs on (search_graph), built once.
    std::shared_ptr<const ScoreGraph> graph;
    // What PreviewController::open reads for this chart, filled from the file.
    store::ChartLibraryEntry entry;
    // The path with no activation: search_target over no windows, found once
    // at the open. The screen shows it while the player has no window, so
    // the empty start has a scene, a meter curve and a score from a path
    // ("The empty path" in docs/superpowers/plans/2026-10-30-path-builder-redesign.md (removed; git show 307d0b5c:docs/superpowers/plans/2026-10-30-path-builder-redesign.md)).
    // Empty only when the search found no path at all.
    std::optional<Path> empty_path;

    // Whether the chart has a Star Power phrase. Without one there is no
    // path to build (the record then has no activation to offer).
    bool has_star_power() const { return song.sp_phrase_count() > 0; }
};

// Opens the chart at `notespath` under `settings`, on the calling thread:
// parses it as the Preview parses it (app::resolve_preview_song with the
// settings' reading, rules and fill rule), checks it has notes (require_notes,
// whose error the user sees), builds the graph, runs analyze_chart over it and
// finds the empty path.
// Throws what those throw. Setting *cancel stops the search, by
// app::stop_on_cancel's rule.
std::unique_ptr<OpenChart> load_open_chart(const std::string& notespath,
                                           const app::Settings& settings,
                                           const std::atomic<bool>* cancel = nullptr);

// The window's title line for an open chart, as the mockup's title line
// reads (docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html (removed; git show 6804353f:docs/superpowers/specs/2026-10-30-path-builder-mockup/mockup_template.html)):
// the song's name, a dash and its artist, each as
// display_title and display_artist show them. A chart with no usable name
// (display_title's kUnknownTitle) shows its file path instead.
std::string chart_title_line(const OpenChart& chart);

// load_open_chart on a worker thread. Its message() and error() are the plain
// sentence and details line of a failed open (app::plain_error and
// app::plain_error_detail, through ResultJobBase).
class OpenJob : public ui::ResultJobBase {
public:
    OpenJob(std::string notespath, app::Settings settings);
    ~OpenJob();
    // The opened chart, once finished() and ok(); taken once.
    std::unique_ptr<OpenChart> take();
    const std::string& notespath() const { return notespath_; }

private:
    std::string notespath_;
    app::Settings settings_;
    std::unique_ptr<OpenChart> chart_;
};

// The player's own path: the windows they pinned, in act-tick order, and the
// path the engine made of them (empty while there is no window).
struct PlayerPath {
    std::vector<PinnedWindow> windows;
    std::optional<Path> path;
};

// The player's path as a copy of `path`, every window pinned in full
// (pinned_windows), as Paste, Open path... and "Finish the rest for me" make
// it.
PlayerPath player_copy_of(const Path& path);

// Undo and redo over the player's path: every state the path has been in,
// with no cap, and which one is current.
class PathHistory {
public:
    // Starts over at `start`, with nothing to undo or redo.
    void reset(PlayerPath start);
    // Makes `next` the current state; anything that could be redone is gone.
    void edit(PlayerPath next);
    bool can_undo() const { return current_ > 0; }
    bool can_redo() const { return current_ + 1 < states_.size(); }
    // Each returns false and changes nothing when there is nothing to do.
    bool undo();
    bool redo();
    const PlayerPath& current() const { return states_[current_]; }
    // Counts every change of the current state. A reader that keeps
    // something built from current().path compares this, not the path's
    // address, which an edit after an undo reuses.
    uint64_t generation() const { return generation_; }

private:
    std::vector<PlayerPath> states_{PlayerPath{}};
    size_t current_ = 0;
    uint64_t generation_ = 0;
};

// The builder's state between frames: the chart that is open (or being
// opened) and the player's path on it.
class PbState {
public:
    // Starts opening `notespath`. Reads Settings::load() and hydra_rules.ini
    // now, in that order, so a change made in either applies to the next chart
    // opened. A chart still opening is dropped for the new one. Throws
    // SettingsFileError or RulesFileError before anything here changes, and
    // PbApp::open shows it.
    void open(const std::string& notespath);
    // open() with the settings given, for tests and for open().
    void open_with(const std::string& notespath, const app::Settings& settings);
    // Takes in a finished open, and drops replaced opens that have stopped.
    // Call once per frame.
    void poll();
    // An open is running, a replaced one has not stopped yet, or the
    // decision list's searches are running.
    bool busy() const { return job_ != nullptr || !retired_.empty() || decisions_.busy(); }
    // The chart being opened is not in yet.
    bool opening() const { return job_ != nullptr; }
    // The chart being opened, or the one last asked for.
    const std::string& requested_path() const { return requested_; }
    // The Preview's volume in the settings the last open() read; none before
    // the first. The builder never writes it back (plan Q2).
    std::optional<int> preview_volume() const { return preview_volume_; }

    const OpenChart* chart() const { return chart_.get(); }
    // The open chart's search graph, for a search on a worker: the pointer
    // also keeps the chart alive, whose song the graph reads, so a search
    // still running when another chart opens never reads a freed song.
    // Null with no chart open.
    std::shared_ptr<const ScoreGraph> search_graph() const {
        return chart_ ? std::shared_ptr<const ScoreGraph>(chart_, chart_->graph.get()) : nullptr;
    }
    // Why the last open failed: the plain sentence and the details line under
    // it (empty when the sentence already quotes it). Both empty after an open
    // that worked.
    const std::string& error() const { return error_; }
    const std::string& error_detail() const { return error_detail_; }

    PathHistory& history() { return history_; }
    const PathHistory& history() const { return history_; }

    // The decision list the screen reads (pb_coach.h). poll() takes its
    // answers in; a newly opened chart clears it.
    DecisionList& decisions() { return decisions_; }
    const DecisionList& decisions() const { return decisions_; }

private:
    DecisionList decisions_;
    std::string requested_;
    std::optional<int> preview_volume_;
    std::unique_ptr<OpenJob> job_;
    // Opens a newer open replaced, cancelled and still stopping. poll() drops
    // each once it has finished, so the window never waits on one.
    std::vector<std::unique_ptr<OpenJob>> retired_;
    std::shared_ptr<const OpenChart> chart_;
    std::string error_;
    std::string error_detail_;
    PathHistory history_;
};

// What the builder says over a chart with no Star Power phrase.
inline constexpr const char* kNothingToBuildText =
    "This chart has no Star Power phrases, so there is no path to build.";

}  // namespace hydra::pathbuilder

#endif  // HYDRA_PATHBUILDER_PB_STATE_H
