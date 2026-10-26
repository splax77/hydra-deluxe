// Shared UI state — the C++ port of hydra_app.py's HyAppUserSettings /
// HyAppRecordBook / HyAppState. One AppState is built at startup and handed
// to every view's render function each frame.
//
// Persistence differs from Python on purpose: settings live in their own INI
// next to the executable (not hymisc.INIPATH's format) and the chart library
// table now lives on RecordStore (see store/record_store.h) instead of a
// separate ad hoc scan_library() connection — both were free to redesign,
// same as the record format itself (Phase 4).

#ifndef HYDRA_UI_APP_STATE_H
#define HYDRA_UI_APP_STATE_H

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"
#include "app/dynamics_breakdown.h"
#include "app/path_view.h"
#include "core/rules.h"
#include "store/record_store.h"
#include "ui/column_widths.h"
#include "ui/dm_jobs.h"
#include "ui/generation.h"
#include "ui/library_jobs.h"
#include "ui/library_model.h"
#include "ui/list_scroll.h"  // SelectionSource
#include "ui/report_state.h"  // ReportBuild, ReportOutOfDate
#include "ui/update_job.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace hydra::ui {

class PreviewController;

// The settings struct itself lives in app/config.h (shared with the CLI
// exes).
using Settings = app::Settings;

// A song-panel row that keeps its place while the open song is re-analyzed
// (AppState::view_holds_space): the height it had when it was last drawn with
// content. The row draws that much empty space instead of nothing, so the
// rows under it don't jump up and back. detail::draw_held_row
// (ui/details_parts.h) draws every held row.
struct HeldRow {
    std::optional<float> height;
    // For a frame the row has nothing to draw: the height to keep, or none.
    // When it isn't holding, the row forgets its height, so a stale one never
    // pads it later.
    std::optional<float> keep(bool hold) {
        if (!hold) height.reset();
        return height;
    }
};

// Per-frame UI state of the Song Details modal. Owned here, not as statics in
// the draw code: a static outlives this AppState (the UI test runner builds one
// per test) and a static pointer into `viewed`'s record outlived the record
// itself (1.5.1 SP-cap crash). Everything in here is derived from, and must be
// re-synced against, the AppState fields it mirrors.
struct DetailsViewState {
    // show_details as of the last tick(); its true->false edge runs
    // close_details() once, whatever closed the panel.
    bool prev_open = false;
    // Points into `viewed`'s record; valid only for record_watcher's generation.
    const Path* selected_path = nullptr;
    GenerationWatcher record_watcher;
    // When "Copied!" last flashed after Copy path string; -1 = never.
    double copied_at = -1.0;
    // The Paths tab's built views, kept between frames (app::PathsTabCache).
    app::PathsTabCache paths_tab;
    // The Preview overlay's key for selected_path (app::path_overlay_key),
    // built when the selection or the record changes instead of every frame.
    std::string overlay_key;
    const Path* overlay_key_path = nullptr;
    int overlay_key_generation = -1;
    // The chart file's presence (the "Song file not found" line), as of the
    // last look (AppState::cached_file_check), not every frame: on a sleeping
    // or network drive one look can stall a frame. -1 = look now.
    bool file_ok = true;
    double file_checked_at = -1.0;
    // The headline (render_headline) and the Preview's "Showing" row
    // (render_path_picker), held while the open song is re-analyzed.
    // close_details forgets both.
    HeldRow headline;
    HeldRow path_picker;
};

// Per-frame UI state of the main window's library view. Owned here, not as
// statics in the draw code, for the same reason as DetailsViewState: a static
// outlives this AppState, so the UI test runner's next app inherited the last
// test's search text and dmleaderboards filter.
struct LibraryViewState {
    // The status line's fade. The watcher starts at "already seen" for this
    // app's counter (0), so app startup does not start a fade.
    GenerationWatcher status_watcher{/*seen=*/0};
    double status_shown_at = -1.0;
    // The folder waiting on the "Remove folder?" confirm.
    std::optional<size_t> confirm_remove;
    // The search box's text and whether it was filled from `search` yet.
    // Typing is applied at most every AppState::kSearchThrottleSeconds:
    // `search_pending` holds an edit
    // not applied yet, `search_applied_at` when the last one was.
    char search_buf[256] = "";
    bool search_synced = false;
    bool search_pending = false;
    double search_applied_at = -1.0;
    // Whether the table's sort was read from its header yet. The ImGui
    // context outlives an AppState (the GUI tests build one per test), so a
    // fresh model reads the header's current sort on its first frame.
    bool sort_synced = false;
    // The panel state the table last fitted the columns that make way for the
    // song panel to (render_table); unset = fit them on the table's next frame
    // past its first.
    std::optional<bool> columns_for_panel;
    // The selection (notespath) the table last asked to scroll to, so it asks
    // only when the selected song changes. Whether and how far it then moves
    // is ui/list_scroll.h's call.
    std::string scrolled_to;
    // The dmleaderboards picker's name filter.
    char dm_filter[128] = "";
    // The split beside the song panel. The share itself lives in
    // hydra_ui.ini (library_share() in app_shell.h); these tell a drag apart
    // from a width the split set: the width the library had last frame, and
    // the room and UI scale it had when the split last set its width.
    float library_w = -1.0f;
    float split_set_for_w = -1.0f;
    float split_set_for_px = -1.0f;
    bool panel_was_open = false;
    // Song folders: a folder was added or removed since the dialog opened,
    // so it offers "Scan now".
    bool folders_changed = false;
    // The Analysis settings panel was open at the end of last frame, so an
    // Esc that closed it this frame can be told apart (render_settings_button).
    bool settings_panel_was_open = false;
    // The table's column widths, from the one width rule (ui/column_widths.h):
    // the columns as the rule sees them and their measure (which keeps the UI
    // scale it was taken at). The room the table had this frame is kept too,
    // so a GUI test can hand the rule the same inputs the table did.
    struct ColumnWidths {
        std::vector<ColumnSpec> specs;
        MeasuredWidths measured;
        TableRoom room;
    } column_widths;
};

// The open song as the engine analyzed it on the click (D87 item 1). Nothing
// here is read from stored details: the record, its timing, the length and
// the Dynamics count all come from the click's ViewJob.
struct ViewedSong {
    enum class State {
        None,         // nothing selected, or an error was dismissed
        Analyzing,    // the click's job is running
        Ready,        // `record` holds the engine's result
        Cancelled,    // the panel's Cancel stopped it ("Analysis cancelled.")
        Failed,       // `message` and `error` say why
        FileMissing,  // the chart file is gone; the panel's notice says so
        RulesBroken,  // analysis is off; only the Dynamics count ran
    };
    State state = State::None;
    std::optional<HydraRecord> record;     // set only when Ready
    std::optional<SongTiming> timing;      // the analyzed song's, when Ready
    // The song's length in chart time (app::analysis_song_length), when
    // Ready; empty when the owner gave none.
    std::optional<double> song_length_ms;
    // The best path's summary row (stars), as the click saved or found it.
    store::PathSummary summary;
    // Failed: the plain sentence and its details line (app::plain_error_detail,
    // empty when the sentence already quotes it).
    std::string message;
    std::string error;
    // The Dynamics count, or why it failed. Set in every state the job
    // finished in, RulesBroken included.
    std::optional<app::DynamicsBreakdown> dynamics;
    std::string dynamics_message;
    std::string dynamics_error;

    bool ready() const { return state == State::Ready && record.has_value(); }
};

// One report as the app holds it (D103): the last good result, kept in memory
// so a window opens at once, when it was built, whether it is out of date,
// whether its window is open, and how the latest build ended. A failed or
// cancelled build keeps the last good result. ReportBuild and
// ReportOutOfDate live in ui/report_state.h, shared with the windows.
template <class Result>
struct ReportSlot {
    // Shared, never copied: the window reads the rows where the job left them.
    std::shared_ptr<const Result> result;
    std::chrono::system_clock::time_point built_at{};  // "Built HH:MM"
    ReportOutOfDate out_of_date = ReportOutOfDate::None;
    // When the batch that last marked this report out of date finished, for
    // the window's "(a batch finished at HH:MM)" (D103 item 21). Unset until
    // a batch marks it.
    std::optional<std::chrono::system_clock::time_point> batch_finished;
    bool window_open = false;
    // How the latest finished build ended: None (never built), Ready,
    // Cancelled or Failed. Failed carries the plain sentence and raw text.
    ReportBuild last = ReportBuild::None;
    std::string message;
    std::string error;
    // An out-of-date event that landed while a build was running. That build
    // read the old library or settings, so its result starts out of date.
    ReportOutOfDate out_of_date_during_build = ReportOutOfDate::None;
    // `result` was built because its window opened with nothing in memory,
    // as after a close let the rows go (D103 item 28). The window then keeps
    // the search, timing, Optimal paths only and sort the user left; a result
    // from Refresh or a batch starts them over. The path report only;
    // AppState::collect_path_report sets it.
    bool built_on_open = false;

    // A build stopped by Cancel. The outcome reads Cancelled, and an
    // out-of-date event that landed during the stopped build is dropped with
    // it. The last good result, its message and its out-of-date mark stay.
    // The path report lands here through land_report when its job stops; the
    // comparison calls it at the press, because its job is parked at once.
    void mark_cancelled() {
        last = ReportBuild::Cancelled;
        out_of_date_during_build = ReportOutOfDate::None;
    }
};

using PathReportSlot = ReportSlot<app::report::GeneratedReport>;
using DmReportSlot = ReportSlot<app::dm_report::GeneratedDmReport>;

// What the app reads before it opens the store: the settings, with
// hydra_rules.ini already loaded, and the loader's error if the file was bad.
struct StartupSettings {
    app::Settings settings;
    std::string rules_error;
};

class AppState {
public:
    // Reads the user's settings and rules file, then starts opening the
    // user's database on store_open_job: the window draws while it opens.
    AppState();
    // Injecting form, for tests and harnesses: same startup work as the
    // default constructor, but on a caller-supplied settings struct and store
    // instead of the user's INI and database. The store is ready at once.
    AppState(app::Settings settings, std::unique_ptr<store::RecordStore> store);
    ~AppState();  // out-of-line: PreviewController is only forward-declared here

    Settings settings;
    // Null until the startup open is collected (store_ready). Only the main
    // window reaches it, and run_frame draws that window only once it is
    // ready.
    std::unique_ptr<store::RecordStore> store;

    // The startup open (StoreOpenJob). tick() collects it once it finishes:
    // the store and the library move in, and the job goes.
    std::unique_ptr<StoreOpenJob> store_open_job;
    // Whether the store is open and the library read: the one gate run_frame
    // asks before drawing the main window.
    bool store_ready() const { return store != nullptr; }
    // Whether the startup open failed. The window then has nothing to show:
    // main.cpp shows the startup message box with store_open_error() and
    // Hydra closes (D72 item 1).
    bool store_open_failed() const { return store_open_error_ != nullptr; }
    std::exception_ptr store_open_error() const { return store_open_error_; }
    // The sentence the startup open left to show once, in an information box
    // over the window, or "" when it left none. Today only a set-aside library
    // file leaves one (StoreOpenJob::startup_notice). main.cpp shows it
    // between frames and then calls startup_notice_closed; a test reads it
    // here.
    const std::string& startup_notice() const { return startup_notice_; }
    // The box has closed: forgets the sentence and asks for the scan the
    // fresh library needs (request_scan), so the scan starts after the box.
    // Does nothing when no sentence is waiting.
    void startup_notice_closed();
    // Where the startup open is, for the startup screen. A default progress
    // when no open is running.
    StoreOpenProgress store_open_progress() const;
    // Whether the startup screen shows its words yet: only once the open has
    // run kViewProgressDelaySeconds, so a fast open shows an empty window
    // and then the library.
    bool store_open_shown() const;
    // Blocks until the startup open has finished, then collects it as tick()
    // does. For the test harness only, so a test can start from a ready
    // store; the app never waits on it.
    void wait_store_open();
    // Set at startup when hydra_rules.ini is bad (the loader's message, which
    // names the key). While set, analysis is off: start_batch does nothing
    // and a click counts only the Dynamics (ViewedSong::State::RulesBroken).
    // It clears only on a restart with a fixed file; there is no fallback to
    // the default rules.
    std::string rules_error;
    // What the screens say above rules_error while analysis is off:
    // app::kAnalysisOffSentence (D103 item 24, D114 item 11).
    bool analysis_blocked() const { return !rules_error.empty(); }

    // Library browsing: every scanned chart in memory, with its stored
    // summary, filtered by the search and the status chip and sorted by the
    // table (ui/library_model.h).
    LibraryModel library;
    // The applied search text; empty = no filter. How many charts the
    // library holds is library.rows().size(), read where it is needed.
    std::string search;
    // Applies a search at once (the box throttles its own calls).
    void set_search(std::string text);
    // Re-reads every chart and summary: at startup and after a scan. The open
    // song panel's entry follows its row (follow_selected_row).
    void reload_library();
    // Re-reads every row's summary: after a settings change or a batch step.
    void refresh_library_summaries();
    // Re-reads one chart's summary: after one song's analysis is stored, and
    // for the open chart on each batch refresh. True when its row changed.
    bool refresh_library_row(const std::string& md5);
    // Once per frame, from the library pane: reloads after a scan finishes,
    // and re-reads summaries while a batch runs -- at most once per
    // kBatchRefreshSeconds, and only when the batch stored something since
    // the last look.
    void tick_library(double now);

    // The rows on screen, in order, as indices into library.rows().
    const std::vector<size_t>& library_view_order() const { return library.order(); }
    size_t library_shown_count() const { return library.order().size(); }
    // The row at position `view_index` of library_view_order().
    const LibraryRow& library_row_at(size_t view_index) const {
        return library.rows()[library.order()[view_index]];
    }
    // The rows on screen (library_view_order), in table order: what the
    // Analyze button runs. The rows keep only part of each chart
    // (LibraryChart), so the entries are read from the store; throws when
    // that read fails.
    std::vector<store::ChartLibraryEntry> library_matches() const;

    // Selection / details modal.
    std::optional<store::ChartLibraryEntry> selected;
    bool show_details = false;
    // Why the selection last changed, as the last select() was told. A table
    // hands it to selection_scroll (ui/list_scroll.h) when it moves its view
    // to the selected row.
    SelectionSource last_selection_source = SelectionSource::Elsewhere;
    void select(const store::ChartLibraryEntry& entry, SelectionSource source);
    // A library row's click: reads the row's whole entry from the store and
    // selects it. A failed read says so (set_problem) and selects nothing.
    void select(const LibraryChart& row, SelectionSource source);
    // Whether this library row (a LibraryChart or a store entry) is the
    // selected one, by row_key (ui/library_model.h).
    template <class Row>
    bool is_selected_row(const Row& row) const;

    // Whether the song panel is showing. Code outside the panel reads this,
    // not show_details (which the X, Escape and the tests write).
    bool details_open() const { return show_details; }

    // The rows the library currently shows, in order. Next / previous and
    // the GUI tests walk these. (T12 re-implements the three over its
    // in-memory model and library_view_order(); callers don't change.)
    size_t view_row_count() const;
    const LibraryChart& view_row(size_t i) const;
    store::RecordStatus view_row_status(size_t i) const;

    // Opens the next (delta 1) or previous (delta -1) row of the current
    // view. Never wraps, and does nothing when the open song isn't in the view.
    void select_relative(int delta);
    bool can_select_relative(int delta) const;

    // Everything that must stop when the song panel closes. It runs once, on
    // the panel's open-to-closed edge (tick() watches it), whatever closed
    // it: the X, Escape, the Rescan library button, or a new selection. The
    // Preview stops and lets go of its audio device, and the click's job is
    // cancelled: nothing is saved for a song whose analysis didn't finish
    // (D87 item 11). Safe to call when closed.
    void close_details();

    // A report row's click (D103 item 3): switches the analysis settings to
    // `chartmode` first when it differs, the way the panel's own controls do,
    // then selects the chart with hash `hyhash`. A chart with several copies
    // selects the first in the library's current order. A hash the library
    // doesn't list selects nothing. While a batch locks the analysis settings, a
    // row of another mode selects nothing and the status line says why.
    void select_chart(const std::string& hyhash, const std::string& chartmode);

    // Whether the selected chart's file exists, as of the last look
    // (cached_file_check).
    bool selected_file_ok(double now);
    // How long the UI trusts a cached "this file exists" answer (D54).
    static constexpr double kFileCheckSeconds = 2.0;
    // The one cached file check: calls `look` and keeps its answer when
    // `checked_at` is -1 ("look now") or kFileCheckSeconds old; otherwise
    // returns the kept answer. `checked_at` and `answer` are the caller's
    // pair (the song panel's chart file).
    template <class Look>
    static bool cached_file_check(double& checked_at, bool& answer, double now, Look look) {
        if (checked_at < 0.0 || now - checked_at >= kFileCheckSeconds) {
            answer = look();
            checked_at = now;
        }
        return answer;
    }
    // How long a neutral status line stays (set_status); a problem never
    // fades.
    static constexpr double kStatusFadeSeconds = 6.0;
    // A click's progress box shows only once its job has run this long, so
    // a fast chart shows its paths with no box (D87 item 6).
    static constexpr double kViewProgressDelaySeconds = 0.15;
    // Whether a job `elapsed_s` old has waited out that delay: the one test
    // the click's box (view_progress_shown) and the startup screen
    // (store_open_shown) both ask.
    static bool progress_delay_passed(double elapsed_s) {
        return elapsed_s >= kViewProgressDelaySeconds;
    }
    // How long "Copied!" stays after the path is copied.
    static constexpr double kCopiedSeconds = 2.0;
    // Typing in the library search re-filters at most this often, so a burst
    // of keys on a big library filters a few times rather than once per key.
    static constexpr double kSearchThrottleSeconds = 0.15;
    // A running batch refreshes the library's results at most this often.
    static constexpr double kBatchRefreshSeconds = 1.0;

    // The details modal's own per-frame state (see DetailsViewState above).
    DetailsViewState details_ui;
    // The library view's own per-frame state (see LibraryViewState above).
    LibraryViewState library_ui;

    // The open song as the click's analysis found it (ViewedSong above).
    ViewedSong viewed;
    // Bumped whenever `viewed` changes; invalidates the UI's selection caches.
    Generation record_generation;

    // Asks for the click's analysis of `selected` under the current settings
    // (D87 item 1): a click, a setting change with a song open (D90 item 1)
    // and "Try again" all come here. It bumps view_generation, so an older
    // request's late result is dropped. At most one job runs at a time: when
    // one is still running, it is cancelled and this request waits in
    // view_pending until that job has exited, and only the latest request
    // then runs.
    void start_view();
    // The panel's Cancel: the job stops at its next progress tick, and the
    // panel then shows "Analysis cancelled." (D87 item 11).
    void cancel_view();
    // The panel's Continue under an error: the panel goes back to empty.
    void dismiss_view_error();
    // The click's one job, and the generation of the latest request. A
    // finished job whose generation is not the latest is dropped, never
    // compared by address (memory: analyze-job pointer-reuse bug). The job is
    // held until it exits, even once cancelled: joining it on the UI thread
    // would freeze the window until its next progress tick.
    std::unique_ptr<ViewJob> view_job;
    Generation view_generation;
    // A request that waits for the cancelled view_job to exit; tick() then
    // starts it under the settings and song current at that moment.
    bool view_pending = false;
    // Whether the latest request is still working (its job runs, or it waits
    // in view_pending), and whether its progress box shows yet: only once it
    // has waited kViewProgressDelaySeconds since start_view.
    bool view_running() const;
    bool view_progress_shown() const;
    // Whether the song panel holds its rows' places (HeldRow): the open song
    // is analyzing again after its result was shown, as after a setting
    // change. Which requests count as analyzing again is view_reanalysis_'s
    // to say. Every held row asks here, through detail::draw_held_row.
    bool view_holds_space() const;
    // True once the click's job has been collected and nothing waits: the
    // click has settled. The one spelling tests wait on.
    bool view_settled() const { return !view_job && !view_pending; }

    // 3D Preview (Phase 5). The GUI's shared D3D11 device is injected once at
    // startup (set_render_device, mirroring load_icons); the controller is
    // created lazily on first use -- a session that never opens the Preview tab
    // pays nothing. preview_controller() returns null when no device was set.
    void set_render_device(ID3D11Device* device, ID3D11DeviceContext* context);
    PreviewController* preview_controller();
    std::unique_ptr<PreviewController> preview;

    // Background jobs (at most one of each kind runs at a time).
    std::unique_ptr<ScanJob> scan_job;
    std::unique_ptr<BatchJob> batch_job;
    std::unique_ptr<ReportJob> report_job;

    // True while a batch is running. The analysis settings are locked then: a
    // result is filed under the settings it ran with. A click's analysis
    // does not lock it; a setting change restarts that one (D90 item 2).
    bool batch_running() const;
    // Why the settings are locked: a batch, or nothing. A job finishes on its
    // own thread, so two reads of batch_running() in one frame can disagree;
    // a caller that needs both "locked?" and "by what?" reads this once.
    enum class SettingsLock { None, Batch };
    SettingsLock settings_lock() const;
    bool settings_locked() const { return settings_lock() != SettingsLock::None; }
    // The one sentence for a request a running batch turns away: the status
    // line's, and the greyed Scan and Analyze buttons' tooltip (D51 call 24,
    // D114 item 12).
    static constexpr const char* kBatchRunningStatus = "A batch is running.";
    // Whether a library scan may start now: there are song folders, no scan
    // job is held (its modal is still up until Continue), and no batch is
    // running. start_scan enforces it; the toolbar button reads it.
    bool can_scan() const;
    // Whether a batch may start now, whatever it would cover: hydra_rules.ini
    // is good and no batch is running. start_batch enforces it; the toolbar's
    // Analyze button and the confirm's Start read it. Each screen asks its own
    // "is there anything to analyze?" on top, because they count different
    // things (matched rows, rows still to do).
    bool can_analyze() const;
    // Whether any background job is still working: the startup open, scan, batch, the click's
    // job (a waiting request included), path report, the two leaderboard jobs, and
    // the Preview's jobs. A finished job waiting to be collected counts as
    // done. The parked leaderboard jobs are left out: they were cancelled,
    // and nothing on screen waits on them. The GUI tests' wait-idle waits on
    // this.
    bool any_job_running() const;

    // Once per frame, before any view draws (run_frame). Collects the
    // startup open once it finishes. Owns the panel's
    // closing edge and the click's job: saving its summary, re-identifying
    // an edited chart, and showing its result. `now` is ImGui::GetTime() in
    // the app.
    void tick(double now);

    // Whether this batch run has already kicked off its path report — one
    // report per run, however long the finished strip stays open. The strip
    // shows the report's outcome only once this run started it.
    bool report_started = false;

    // The two reports (D103). Each job hands its result here when
    // update_background_jobs collects it; the windows, the toolbar and the
    // finished strip read these and never the jobs' results.
    PathReportSlot path_report;
    DmReportSlot dm_report;
    // Where each report's latest build stands: Building while its job runs
    // (for the path report, also while it waits on a batch or on the batch
    // confirm before its build), otherwise how the last one ended
    // (ReportSlot::last).
    ReportBuild path_report_build() const;
    ReportBuild dm_report_build() const;

    // Builds the path report under the current settings, analyzing first
    // (plan 2026-10-15, question 8). It plans a batch over every library row;
    // when that plan has rows to run, the batch runs (once the report window
    // asks and the user answers Analyze, when there are more than
    // report_confirm_above rows) and the report builds when it finishes, as
    // after any batch. With nothing to
    // run the build starts at once. A batch already running is waited on
    // instead. Does nothing while a build or a wait is under way. A window's
    // Refresh and Try again, and showing it with nothing built, all come
    // here. Off under a bad hydra_rules.ini, like a batch.
    void request_path_report();
    // The path report window's Cancel. A build: the job finishes its one read
    // of the store, drops the rows (generate_report checks the flag after the
    // read) and the window then shows the cancelled state. A wait: the
    // report's own batch stops as the strip's Stop stops it (a batch the user
    // started is left to the user), and the
    // window shows the cancelled state at once. While the window asks
    // whether to analyze (path_report_awaits_confirm), Cancel is its "no":
    // the confirm closes, nothing runs, and the report builds from the
    // results already stored.
    void cancel_path_report();
    // Opens the path report window: the toolbar's "Open path report", the
    // finished strip's "Open report". With nothing in memory (never built
    // this session, or let go by close_path_report) it requests a build whose
    // result keeps the window's filters (ReportSlot::built_on_open).
    void show_path_report();
    // Closes the path report window (D103 item 28): its X, Esc and Ctrl+W
    // all come here, through the window's close callback. The rows go, a
    // build still running stops as its Cancel stops it and never lands, and
    // the slot starts over as never built, so the next show_path_report
    // builds again. A batch the report waited on keeps running, with its
    // strip; its finish builds the report as any batch's does. A question
    // the window was asking goes with it: nothing runs, and no confirm opens
    // in the main window.
    void close_path_report();
    // Whether the toolbar offers "Open path report" with nothing built: the
    // library has rows and analysis is on, since opening is how a library is
    // analyzed for the report.
    bool path_report_offered() const;
    // More rows to run than this and opening the report asks first, in the
    // report window itself (the user's answer 8, 2026-10-15).
    static constexpr int kReportConfirmAbove = 500;
    // The threshold request_path_report reads: kReportConfirmAbove, lowered
    // only by tests, whose corpus is smaller.
    int report_confirm_above = kReportConfirmAbove;
    // The batch the path report waits on, whose snapshot its window shows;
    // null when it waits on none.
    const BatchJob* path_report_batch() const;
    // Whether the path report window asks whether to analyze: the batch
    // confirm is the report's own, answered by the window's Analyze
    // (start_path_report_batch) or Cancel (cancel_path_report), never by the main
    // window's popup.
    bool path_report_awaits_confirm() const { return report_wait_ == ReportWait::Confirm; }
    // The window's Analyze: runs the batch its question counted, which the
    // report then waits on. Does nothing unless the window asks, or while
    // can_analyze says no.
    void start_path_report_batch();

    // "Compare dmleaderboards user" picker + its two network jobs. The fetch
    // job loads the ladder into dm_users; the report job builds the
    // comparison into dm_report.
    bool dm_picker_open = false;
    // Set by reopen_dm_picker: the picker opens its popup on the main
    // window's next frame (render_dm_picker_modal), since a popup has to be
    // opened from the window that draws it.
    bool dm_picker_popup_pending = false;
    std::vector<net::DmUser> dm_users;
    std::unique_ptr<DmFetchUsersJob> dm_fetch_job;
    std::unique_ptr<DmReportJob> dm_report_job;

    void start_scan();
    void start_batch(bool redo);
    void start_dm_fetch();  // loads the dmleaderboards user list
    // A player picked in the picker (D103 item 8): remembers the player, opens
    // the comparison window and builds the comparison, replacing one still
    // building for another player (D103 item 9).
    void start_dm_report(const std::string& discord_id, const std::string& username);
    // The comparison's Refresh and Try again (D103 item 11): builds it again
    // for the last player picked, under the current settings, when no build
    // is running. Does nothing before any player was picked.
    void request_dm_report();
    // Opens the player picker again: the toolbar's button, and the comparison
    // window's Cancel and "Compare another player..." (D103 item 10). Loads
    // the ladder when this session has none.
    void reopen_dm_picker();
    // The last player picked, whom the comparison window's title names even
    // while that player's scores still fetch.
    const std::string& dm_player_name() const { return dm_player_name_; }

    // The confirm's "Also re-analyze charts that already have a result" box.
    bool batch_redo = false;

    // True while the "Analyze library" confirm shows. open_batch_confirm()
    // plans the batch over the rows it would analyze (the library, or the
    // search's matches) from one store read: one plan that skips the charts
    // with a result and one that redoes them (app::plan_batch). Every number
    // the confirm shows is read from those plans, every copy of a chart
    // counted like the library counts it (D76). start_batch() hands the batch
    // the plan the redo box picks, so the run is the plan the confirm showed
    // (D79).
    bool batch_confirm_pending = false;
    app::BatchPlan batch_plan;
    app::BatchPlan batch_redo_plan;
    // Whether the main window's "Analyze library" popup shows: a confirm is
    // open and it is not the path report's own (path_report_awaits_confirm),
    // which its window asks instead.
    bool batch_confirm_in_main_window() const {
        return batch_confirm_pending && !path_report_awaits_confirm();
    }
    // The confirm's question for the plan `redo` picks: the popup's heading,
    // and the path report window's line while it asks.
    std::string batch_question(bool redo) const;
    // The rows in scope: the redo plan runs every one of them.
    int64_t batch_scope_charts() const { return batch_redo_plan.todo_rows(); }
    // The rows in scope that already have a result: the skip plan's skipped.
    int64_t batch_scope_with_result() const { return batch_plan.skipped; }
    // The toolbar's Analyze. While the path report window asks, the user's
    // own confirm replaces that question, which is answered as its Cancel
    // answers it: the report lists what is stored.
    void open_batch_confirm();
    // The plan a batch started now would run: the redo plan when `redo`.
    const app::BatchPlan& batch_plan_for(bool redo) const {
        return redo ? batch_redo_plan : batch_plan;
    }
    // Closes the confirm without starting, letting go of its plans.
    void close_batch_confirm();

    // Once per frame (run_frame), after tick(): when a batch ends, mark both
    // reports out of date and start its path report (never for a stopped
    // batch); collect each finished report job into its slot, posting the
    // path report's outcome to the status line when the finished strip was
    // dismissed before it landed; let go of cancelled jobs once they finish.
    void update_background_jobs();

    // Leaderboard jobs cancelled while a request was in flight. WinHTTP only
    // checks the cancel flag between reads, so joining one on the spot could
    // freeze the window for up to two minutes. They wait here instead, and
    // update_background_jobs() drops each once it has finished. A path
    // report replaced while it builds waits the same way, since it stops
    // only once its read of the store is done.
    std::vector<std::unique_ptr<DmFetchUsersJob>> parked_dm_fetches;
    std::vector<std::unique_ptr<DmReportJob>> parked_dm_reports;
    std::vector<std::unique_ptr<ReportJob>> parked_reports;
    void cancel_dm_fetch();
    void cancel_dm_report();

    // Set by the details modal's "Rescan library" remedy, the Song folders'
    // "Scan now", startup_notice_closed and the startup open of a library
    // whose scan is out of date: the main window calls start_scan on its next
    // frame (the scan modal belongs to it), which refuses when can_scan says
    // no.
    bool request_scan = false;
    // True while the scan the app asked for on open (request_rescan_on_open)
    // is pending or its dialog is up: the Scanning charts dialog then opens
    // with app::library_rescan_notice. The dialog clears it when it closes.
    bool scan_started_by_open = false;

    // The status line under the toolbar. set_status is news ("Path report
    // saved"): neutral text that fades (kStatusFadeSeconds). set_problem is
    // something the user should act on: orange, and it stays until dismissed.
    // The view times the fade off status_generation changing.
    std::string status_message;
    bool status_is_problem = false;
    Generation status_generation;
    void set_status(std::string message);
    void set_problem(std::string message);
    void dismiss_status();

    // The one way to finish a settings change: write the INI (with a status
    // message when it can't be written — a silent failure made changes look
    // persisted when they weren't) and then refresh whatever the change
    // invalidated.
    //
    // This is the single place that knows which settings change a record's
    // identity — the chart mode, the SP cap, and the lens (the ms limit and
    // the score range a result ran under). Every widget just mutates
    // `settings` and calls this, so none of them can forget a refresh. That
    // forgetting is exactly the bug class here: the 1.5.1 SP-cap crash came
    // from this path, and the Pro Drums / 2x Bass checkboxes used to refresh
    // the library page while leaving `viewed` pointing at the old record.
    void commit_settings();

    // The number boxes' form of commit_settings: the same refresh at once,
    // but the INI waits for flush_settings. A held +/- button changes the
    // value every frame, and each change used to rewrite the file.
    void edit_settings();
    // Writes the INI if an edit_settings change is not saved yet. run_frame
    // calls it once no widget is active, which is when an edit has ended.
    void flush_settings();

    // ---- updates (D259 and D260 in docs/audit/2026-10-03-fix-decisions.md) ----
    // What the version bar shows (UpdateState). Both constructors set it up:
    // the first start after a version change lands in Updated, the leftover
    // download folder goes, and check_updates_on_start runs.
    UpdateState update;
    // The check and the download; at most one of each, collected by tick().
    std::unique_ptr<UpdateCheckJob> update_check_job;
    std::unique_ptr<UpdateDownloadJob> update_download_job;
    // The key a download is checked against: the shipped one, which a test
    // replaces with the test key.
    std::array<uint8_t, 64> update_public_key;
    // The start-up check: starts one when this is the installed exe
    // (app::installed_by_setup), whatever the bar shows. A failure shows in
    // the bar's resting status, not as a phase.
    void check_updates_on_start();
    // Whether a check is running.
    bool update_check_running() const;
    // What the bar says, and with which dot: the one answer the version bar
    // draws (VersionBarStatus).
    VersionBarStatus version_bar_status() const;
    // The release the bar's sentence names, whose notes show on hover: the one
    // offered or being installed during a phase, else the one the start-up
    // check found while the bar says it is available. Null otherwise.
    const app::UpdateRelease* bar_release() const;
    // The bar's update buttons. Update downloads, checks and starts the
    // installer (offered, declined, failed with a release, or the resting
    // bar's Update while VersionBarStatus::update_button is true); Skip this
    // version writes skipped_update_version and puts the buttons away; Later
    // puts them away until the next start and writes nothing.
    void update_now();
    void skip_version();
    void later();
    // The bar's Cancel while downloading: the download stops, its file goes,
    // and the bar offers the release again once the job has stopped.
    void cancel_update();
    // The OK in Updated: offers the release the start-up check found when it
    // is newer and not skipped, otherwise gives the bar back to its resting
    // status.
    void dismiss_update_note();
    // The download's progress for the strip: bytes so far, and the line's
    // "12.3 of 33.2 MB" in decision 4's unit. 0 and empty outside
    // Downloading.
    uint64_t update_received() const;
    std::string update_megabytes_text() const;

private:
    explicit AppState(StartupSettings start);

    // Moves a finished startup open into place: the store and library, or
    // the failure. tick() and wait_store_open() both collect through it.
    void collect_store_open();
    // The opened library's scan is out of date (StoreOpenJob::rescan_needed):
    // asks for a scan (request_scan, scan_started_by_open), or sets the
    // status line's problem when a chart folder cannot be read
    // (app::library_rescan_blocked_notice) and asks for nothing.
    void request_rescan_on_open();
    std::exception_ptr store_open_error_;
    std::string startup_notice_;  // what startup_notice returns

    // The click's job lifecycle on the UI thread: drop a late result, then
    // re-identify an edited chart, save the summary when needed and show the
    // result. tick() calls it every frame, whichever tab shows.
    void update_view_job();
    // Whether the click's job thread is still working, cancelled or not. The
    // one spelling of that test.
    bool view_thread_alive() const;
    // Saves the click's record under `profile` when the chart's stored record
    // is missing, Stale or different (D87 item 2), marks both open reports
    // out of date when it saves (D115 question 4), and returns the summary
    // the record now gives. Throws on a store failure.
    store::PathSummary save_view_summary(const std::string& song_id, const store::Profile& profile,
                                         const app::AnalysisResult& result);
    // The click's one save, for a result or a stored failure: writes
    // `prepared` unless the chart's stored answer already says the same,
    // and marks both open reports out of date when it writes. Throws on a
    // store failure.
    void save_view_record(const store::PreparedRecord& prepared);
    // Starts view_job for the latest request. view_job must be empty.
    void launch_view_job();
    // When the latest request was made (start_view), for view_progress_shown.
    std::chrono::steady_clock::time_point view_requested_at_{};
    // For view_holds_space: the open song's last result landed Ready, so a
    // request for it now is a re-analysis. update_view_job sets it as each
    // result lands; start_view, cancel_view and close_details also clear it.
    bool view_reanalysis_ = false;
    // The row index select_relative would open, if there is one.
    std::optional<size_t> relative_row(int delta) const;
    // Runs one store read on the UI thread. A read that throws puts its
    // sentence in the status line and returns false, and the caller keeps
    // what it showed (D73 item 3). Every read whose answer a screen shows
    // goes through it.
    bool read_store(const std::function<void()>& read);
    // Makes batch_plan and batch_redo_plan over the rows `list_items` gives,
    // under the current settings, in one store read. False when the store
    // failed (read_store). The batch confirm and the path report's opening
    // both plan through here, so the plan a run takes is made one way (D79).
    bool plan_batches(const std::function<std::vector<app::ScanItem>()>& list_items);
    // Re-reads `selected` from its library row after reload_library.
    void follow_selected_row();
    // The library row for the chart `key` (row_key), or null.
    const LibraryChart* library_row_for(const std::string& key) const;
    // The store's entry for the chart `key`, listed under `md5`: the one
    // lookup select and follow_selected_row share. Empty when none, or when
    // the read failed (read_store said so).
    std::optional<store::ChartLibraryEntry> read_entry(const std::string& key,
                                                       const std::string& md5);
    bool batch_finish_seen_ = false;  // update_background_jobs saw this run end

    // Why a path report build started. Batch: the finished batch's own
    // report, whose outcome the strip shows and which "Open automatically"
    // opens. Open: show_path_report found nothing in memory, so its result
    // keeps the window's filters (ReportSlot::built_on_open). Request: the
    // window's Refresh or Try again.
    enum class PathReportCause { Request, Batch, Open };
    // Starts report_job, parking one still running. The report lists the
    // records filed under `run`.
    void launch_path_report(app::BatchRun run, PathReportCause cause);
    // request_path_report's work, for the cause given.
    void request_path_report(PathReportCause cause);
    PathReportCause path_report_cause_ = PathReportCause::Request;
    // What the path report waits on before its build: the batch confirm
    // request_path_report opened, which the report window asks (Confirm),
    // the batch it started, or a batch already
    // running when it was asked. The build that follows carries
    // report_wait_cause_.
    enum class ReportWait { None, Confirm, OwnBatch, Batch };
    ReportWait report_wait_ = ReportWait::None;
    PathReportCause report_wait_cause_ = PathReportCause::Request;
    // update_background_jobs' path report part: settles a wait on the
    // confirm or on a batch.
    void settle_report_wait();
    // The report window's question answered "no": its plans go, nothing
    // runs, and the report builds from what is stored (settle_report_wait).
    void decline_report_batch();
    // Starts dm_report_job for the last player picked. None may be running.
    void launch_dm_report();
    std::string dm_player_id_;
    std::string dm_player_name_;
    // Moves each finished report job's outcome into its slot.
    void collect_path_report();
    void collect_dm_report();
    // An out-of-date event for the reports that read what changed: `path`
    // and `dm` say which. A slot with a result goes out of date; a running
    // build remembers it for its own result. A batch's event passes the time
    // the batch finished, which the slot keeps for its window.
    void mark_reports_out_of_date(
        ReportOutOfDate reason, bool path, bool dm,
        std::optional<std::chrono::system_clock::time_point> batch_finished = std::nullopt);
    ID3D11Device* render_device_ = nullptr;
    ID3D11DeviceContext* render_context_ = nullptr;

    // The settings as of the last commit, so apply_settings can tell an
    // identity change from any other settings edit, and which reports a
    // change reaches.
    app::Settings committed_settings_;

    // An edit_settings change the INI does not have yet.
    bool settings_unsaved_ = false;
    void save_settings();
    // The refresh half of commit_settings.
    void apply_settings();
    // A finished scan replaced the chart table: reloads the rows once, so
    // they match the store again. True when it reloaded. tick_library runs it
    // every frame; a click, previous / next and the batch confirm run it
    // first, so none of them reads rows older than the store.
    bool reload_after_scan();
    // reload_after_scan's memory: whether the current scan's result was read
    // yet. Then tick_library's: the batch's stored count and time at the last
    // summary read.
    bool scan_reloaded_ = true;
    int batch_seen_completed_ = 0;
    double batch_refreshed_at_ = -1.0;

    // The updater's start-up work, which both constructors run.
    void start_updates();
    // Whether the release the last check found is the one Skip this version
    // hid.
    bool found_release_skipped() const;
    // tick()'s part: collects the check and the download, and watches the
    // started installer.
    void tick_updates();
    void collect_update_check();
    void collect_update_download();
    // Puts the strip in `phase` with `text`, the detail line cleared.
    void set_update_phase(UpdateState::Phase phase, std::string text);
    // The strip offering `release` (decision 2).
    void offer_update(const app::UpdateRelease& release);
    // The started installer's "has it ended" check; empty when none runs.
    InstallerEnded installer_ended_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_APP_STATE_H
