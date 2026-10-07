#include "ui/app_state.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "app/config.h"
#include "app/rules_file.h"
#include "app/report_files.h"
#include "app/user_messages.h"
#include "core/winstr.h"
#include "parse/song.h"  // display_title
#include "ui/preview_controller.h"

namespace hydra::ui {

namespace {

// Settings plus hydra_rules.ini. A bad file is not fatal: the app still
// opens so the user can read the error, but analysis stays off.
StartupSettings load_startup_settings() {
    StartupSettings start{Settings::load(), {}};
    try {
        start.settings.rules = app::load_rules_file(app::default_rules_path());
    } catch (const app::RulesFileError& e) {
        start.rules_error = e.what();
    }
    return start;
}

}  // namespace

AppState::AppState() : AppState(load_startup_settings()) {}

// A bad rules file gates the store on RulesStamp::none(), so no row reads
// Ready under the defaults the settings still hold.
AppState::AppState(StartupSettings start)
    : AppState(start.settings,
               app::open_store(app::db_path(),
                               start.rules_error.empty()
                                   ? core::RulesStamp::of(start.settings.rules)
                                   : core::RulesStamp::none())) {
    rules_error = std::move(start.rules_error);
}

AppState::AppState(app::Settings initial_settings,
                   std::unique_ptr<store::RecordStore> initial_store)
    : settings(std::move(initial_settings)),
      store(std::move(initial_store)),
      committed_chartmode_(settings.chartmode_key()),
      committed_cap_(settings.cap_query()),
      committed_lens_(settings.lens()) {
    reload_library();
}

// Out-of-line so the unique_ptr<PreviewController> can be a forward declaration
// in the header (its destructor needs the full type, which lives here).
AppState::~AppState() { flush_settings(); }  // an edit in progress still lands

void AppState::set_render_device(ID3D11Device* device, ID3D11DeviceContext* context) {
    render_device_ = device;
    render_context_ = context;
}

PreviewController* AppState::preview_controller() {
    if (!preview && render_device_)
        preview = std::make_unique<PreviewController>(render_device_, render_context_);
    return preview.get();
}

bool AppState::read_store(const std::function<void()>& read) {
    try {
        read();
        return true;
    } catch (const std::exception& e) {
        set_problem(app::plain_error(e));
        return false;
    }
}

void AppState::reload_library() {
    // The whole scan in one read. Every chart is needed anyway: the chips
    // count them and the search filters them in memory.
    if (!read_store([&] { library.set_charts(store->list_chart_library(0, -1)); }))  // -1 = no limit
        return;
    library.set_query(search);
    refresh_library_summaries();
}

void AppState::refresh_library_summaries() {
    // One store call for the whole library (T7 splits it into chunks), on
    // this thread, never per row per frame: the batch workers share the
    // store's lock.
    read_store([&] {
        library.set_summaries(store->get_summaries(library.hashes(), settings.chartmode_key(),
                                                   settings.cap_query(), settings.lens()));
    });
}

bool AppState::refresh_library_row(const std::string& md5) {
    bool changed = false;
    read_store([&] {
        changed = library.set_summary_for(md5, store->get_summary(settings.record_key(md5))) > 0;
    });
    return changed;
}

void AppState::set_search(std::string text) {
    search = std::move(text);
    library.set_query(search);
}

std::vector<store::ChartLibraryEntry> AppState::library_matches() const {
    const std::vector<size_t> matched = library.matches();
    // Each matched row's place in the answer, by row_key.
    std::unordered_map<std::string_view, size_t> place;
    place.reserve(matched.size());
    for (size_t k = 0; k < matched.size(); ++k)
        place.emplace(row_key(library.rows()[matched[k]].entry), k);
    std::vector<std::optional<store::ChartLibraryEntry>> found(matched.size());
    for (store::ChartLibraryEntry& e : store->list_chart_library(0, -1)) {  // -1 = no limit
        const auto it = place.find(row_key(e));
        if (it != place.end()) found[it->second] = std::move(e);
    }
    // A row the store no longer lists (a scan replaced the table since the
    // rows were read) is left out; the reload that follows drops its row.
    std::vector<store::ChartLibraryEntry> out;
    out.reserve(matched.size());
    for (std::optional<store::ChartLibraryEntry>& e : found)
        if (e) out.push_back(std::move(*e));
    return out;
}

void AppState::tick_library(double now) {
    // A finished scan replaced the chart table: read it once.
    if (scan_job && !scan_reloaded_ && scan_job->snapshot().finished) {
        scan_reloaded_ = true;
        reload_library();
    }
    // A batch stores results on its own threads. Re-read the summaries at
    // most once per kBatchRefreshSeconds, only when it stored something since
    // the last read,
    // and once more when it ends so the last results show.
    if (batch_job) {
        const BatchJob::Snapshot snap = batch_job->snapshot();
        if (snap.completed != batch_seen_completed_ &&
            (snap.finished || now - batch_refreshed_at_ >= kBatchRefreshSeconds)) {
            batch_seen_completed_ = snap.completed;
            batch_refreshed_at_ = now;
            // The open song's panel shows the engine's own result, which a
            // batch under the same settings finds again, so only the rows are
            // read (D87 item 1).
            refresh_library_summaries();
        }
    }
}

// The rows the library table shows, in its current order and filter.
size_t AppState::view_row_count() const { return library_shown_count(); }

const LibraryChart& AppState::view_row(size_t i) const {
    return library_row_at(i).entry;
}

store::RecordStatus AppState::view_row_status(size_t i) const {
    return library_row_at(i).status;
}

std::optional<size_t> AppState::relative_row(int delta) const {
    if (!selected || delta == 0) return std::nullopt;
    const size_t n = view_row_count();
    for (size_t i = 0; i < n; ++i) {
        if (!is_selected_row(view_row(i))) continue;
        const long long j = static_cast<long long>(i) + delta;
        if (j < 0 || j >= static_cast<long long>(n)) return std::nullopt;
        return static_cast<size_t>(j);
    }
    return std::nullopt;
}

bool AppState::can_select_relative(int delta) const { return relative_row(delta).has_value(); }

void AppState::select_relative(int delta) {
    if (std::optional<size_t> i = relative_row(delta)) select(view_row(*i));
}

void AppState::select(const store::ChartLibraryEntry& entry) {
    // The previous chart's panel, torn down the one way: a row click or
    // previous / next swaps the song while the panel stays open.
    close_details();
    selected = entry;
    show_details = true;
    start_view();
}

void AppState::select(const LibraryChart& row) {
    std::vector<store::ChartLibraryEntry> copies;
    if (!read_store([&] { copies = store->list_chart_library_copies(row.md5); })) return;
    for (const store::ChartLibraryEntry& copy : copies) {
        if (row_key(copy) == row_key(row)) {
            select(copy);
            return;
        }
    }
    // None: a scan replaced the table since the rows were read, and the
    // reload that follows redraws the table.
}

void AppState::close_details() {
    show_details = false;
    // The audio device must stop, and the GPU and decode work must not keep
    // running behind a hidden panel.
    if (preview) preview->close();
    // A click's analysis that hasn't finished is cancelled, and its result,
    // even one that finished this frame, is dropped by generation: nothing is
    // saved for it (D87 item 11).
    if (view_job) view_job->cancel();
    view_pending = false;
    view_generation.bump();
    if (viewed.state == ViewedSong::State::Analyzing) {
        viewed = ViewedSong{};
        record_generation.bump();
    }
    // The next open looks at the chart file at once.
    details_ui.file_checked_at = -1.0;
}

void AppState::start_view() {
    view_generation.bump();
    view_requested_at_ = std::chrono::steady_clock::now();
    viewed = ViewedSong{};
    record_generation.bump();
    // A finished job not yet collected is an older request's: let it go.
    if (view_job && view_job->finished()) view_job.reset();
    if (view_job) view_job->cancel();
    view_pending = false;
    if (!selected) return;
    if (!file_exists_utf8(selected->notespath)) {
        viewed.state = ViewedSong::State::FileMissing;
        return;
    }
    viewed.state = ViewedSong::State::Analyzing;
    // One job at a time: this request waits for the cancelled one to exit.
    if (view_job)
        view_pending = true;
    else
        launch_view_job();
}

void AppState::launch_view_job() {
    view_job = std::make_unique<ViewJob>(*selected, settings.record_key(selected->md5),
                                         settings.to_analysis_settings(), analysis_blocked(),
                                         view_generation.n);
    view_job->start();
}

void AppState::cancel_view() {
    if (view_pending) {
        // The waiting request never started; the job still running is an
        // older request's, and its result is dropped by generation.
        view_pending = false;
        viewed = ViewedSong{};
        viewed.state = ViewedSong::State::Cancelled;
        record_generation.bump();
        return;
    }
    if (view_thread_alive()) view_job->cancel();
}

bool AppState::view_thread_alive() const { return view_job && !view_job->finished(); }

void AppState::dismiss_view_error() {
    if (viewed.state != ViewedSong::State::Failed) return;
    viewed = ViewedSong{};
    record_generation.bump();
}

bool AppState::view_running() const {
    return view_pending ||
           (view_thread_alive() && view_job->generation() == view_generation.n);
}

bool AppState::view_progress_shown() const {
    return view_running() &&
           std::chrono::duration<double>(std::chrono::steady_clock::now() - view_requested_at_)
                   .count() >= kViewProgressDelaySeconds;
}

void AppState::update_view_job() {
    if (!view_job || !view_job->finished()) return;
    const std::unique_ptr<ViewJob> job = std::move(view_job);
    // The cancelled job has exited, so the latest request starts now.
    if (view_pending) {
        view_pending = false;
        launch_view_job();
        return;
    }
    // A late result from an older request: dropped by generation.
    if (job->generation() != view_generation.n) return;

    ViewedSong v;
    if (job->is_cancelled()) {
        v.state = ViewedSong::State::Cancelled;
    } else if (!job->ok()) {
        v.state = ViewedSong::State::Failed;
        v.message = job->message();
        v.error = job->error();
    } else {
        ViewOutcome out = job->take_outcome();
        v.dynamics = std::move(out.dynamics);
        v.dynamics_message = std::move(out.dynamics_message);
        v.dynamics_error = std::move(out.dynamics_error);
        store::RecordKey key = job->key();
        // An edited chart: its library row takes the new hash before anything
        // is saved under it (D87 item 3).
        if (out.files_changed) {
            const std::string& path = job->song().notespath;
            try {
                store->reidentify_chart(path, out.new_md5, out.new_sig);
            } catch (const std::exception& e) {
                // Fail loudly and save nothing: a summary under a hash no
                // library row has would be an orphan (ruling 15).
                v = ViewedSong{};
                v.state = ViewedSong::State::Failed;
                v.message = app::plain_error(e);
                v.error = e.what();
                viewed = std::move(v);
                record_generation.bump();
                return;
            }
            key.hyhash = out.new_md5;
            if (is_selected_row(job->song())) {
                selected->md5 = out.new_md5;
                selected->sig = out.new_sig;
            }
            reload_library();
        }
        if (job->analysis_off()) {
            v.state = ViewedSong::State::RulesBroken;
        } else if (!out.analysis) {
            v.state = ViewedSong::State::Failed;
            v.message = std::move(out.analysis_message);
            v.error = std::move(out.analysis_error);
        } else {
            try {
                v.summary = save_view_summary(key, *out.analysis);
            } catch (const std::exception& e) {
                set_problem("Analyzed, but saving failed. " + app::plain_error(e));
                v.summary = store::summarize_record(out.analysis->record);
            }
            v.state = ViewedSong::State::Ready;
            v.timing = out.analysis->song.timing();
            v.song_length_ms = out.length.ms;
            v.record = std::move(out.analysis->record);
        }
    }
    viewed = std::move(v);
    record_generation.bump();
}

store::PathSummary AppState::save_view_summary(const store::RecordKey& key,
                                               const app::AnalysisResult& result) {
    const store::PreparedRow row = store::prepare_row(key, result.record);
    const std::vector<store::SummaryLookup> found =
        store->get_summaries({key.hyhash}, key.chartmode, key.cap, key.lens);
    const bool ready = !found.empty() && found.front().status == store::RecordStatus::Ready;
    if (ready && found.front().summary == row.summary && found.front().bestpath == row.bestpath)
        return row.summary;  // the row already says what the engine says
    store->save_analysis(row);
    refresh_library_row(key.hyhash);  // its row's Best path cell and chip
    return row.summary;
}

bool AppState::selected_file_ok(double now) {
    if (!selected) return false;
    return cached_file_check(details_ui.file_checked_at, details_ui.file_ok, now,
                             [this] { return file_exists_utf8(selected->notespath); });
}

bool AppState::batch_running() const { return batch_job && !batch_job->snapshot().finished; }

AppState::SettingsLock AppState::settings_lock() const {
    return batch_running() ? SettingsLock::Batch : SettingsLock::None;
}

template <class Row>
bool AppState::is_selected_row(const Row& row) const {
    return selected && row_key(row) == row_key(*selected);
}
template bool AppState::is_selected_row(const store::ChartLibraryEntry& row) const;
template bool AppState::is_selected_row(const LibraryChart& row) const;

bool AppState::can_scan() const {
    return !settings.chartfolders.empty() && !scan_job && !batch_running();
}

bool AppState::any_job_running() const {
    // view_job counts while it runs even when cancelled: tick() still has to
    // collect it (and start a waiting request).
    return (scan_job && !scan_job->snapshot().finished) || batch_running() || view_pending ||
           view_thread_alive() || (report_job && !report_job->finished()) ||
           (dm_fetch_job && !dm_fetch_job->finished()) ||
           (dm_report_job && !dm_report_job->finished()) || (preview && preview->busy());
}

void AppState::tick(double /*now*/) {
    if (!show_details && details_ui.prev_open) close_details();
    details_ui.prev_open = show_details;
    update_view_job();
}

bool AppState::report_file_shown(double now) {
    return cached_file_check(library_ui.report_checked_at, library_ui.report_exists, now,
                             [] { return app::report_file_exists(); });
}

void AppState::start_scan() {
    if (!can_scan()) {
        // The toolbar's button is off during a batch, but "Scan now" and the
        // panel's "Rescan library" ask from here: say why nothing happened.
        if (batch_running()) set_status("A batch is running.");
        return;
    }
    scan_job = std::make_unique<ScanJob>(settings.chartfolders, *store);
    scan_reloaded_ = false;
    scan_job->start();
}

void AppState::open_batch_confirm() {
    // Exactly the rows the library's search matches. The plans made here are
    // the ones the batch runs (D79), so the confirm, the strip and the
    // finished counts come from one plan.
    std::vector<app::ScanItem> items;
    const app::BatchRun run = settings.batch_run();
    std::unordered_set<std::string> with_result;
    if (!read_store([&] {
            const std::vector<store::ChartLibraryEntry> scope = library_matches();
            items.reserve(scope.size());
            for (const store::ChartLibraryEntry& e : scope) items.push_back(scan_item_of(e));
            with_result = app::charts_with_result(*store, run, false);
        })) {
        // The database failed: the confirm stays closed (D72 item 4).
        close_batch_confirm();
        return;
    }
    batch_plan = app::plan_batch(items, with_result);
    batch_redo_plan = app::plan_batch(items, app::charts_with_result(*store, run, true));
    batch_confirm_pending = true;
}

void AppState::close_batch_confirm() {
    batch_confirm_pending = false;
    batch_plan = {};
    batch_redo_plan = {};
}

void AppState::start_batch(bool redo) {
    if (analysis_blocked()) return;
    if (batch_running()) return;
    // A direct call (a test) has no confirm open: plan here. When the store
    // can't answer, the confirm stays closed and nothing starts.
    if (!batch_confirm_pending) open_batch_confirm();
    if (!batch_confirm_pending) return;
    // Exactly the plan the confirm counted -- not a search string that SQL
    // would match differently from the library's own search, and not a
    // second store read.
    batch_job = std::make_unique<BatchJob>(redo ? std::move(batch_redo_plan) : std::move(batch_plan),
                                           settings.batch_run(), *store);
    close_batch_confirm();
    report_started = false;
    report_outcome_shown = false;
    batch_seen_completed_ = 0;
    batch_refreshed_at_ = -1.0;
    batch_finish_seen_ = false;
    apply_app_batch_analyzer_for_test(*batch_job);
    batch_job->start();
}

void AppState::update_background_jobs() {
    if (batch_job && !batch_finish_seen_ && batch_job->snapshot().finished) {
        batch_finish_seen_ = true;
        // tick_library re-reads the summaries once the batch ends.
        // One path report per finished run. A stopped run keeps its results
        // but builds no report: a report of part of the library would read
        // as the whole of it.
        if (!batch_job->is_cancelled() && !report_started) {
            report_started = true;
            // The report lists the records the batch filed: its cap and lens,
            // not whatever the settings bar holds now, analyzed under the
            // batch's settings, reusing the charts the batch just analyzed
            // (D87 item 5). Opening the page and its timing bands only shape
            // the page, so they stay live.
            const app::BatchRun& run = batch_job->batch_run();
            report_job = std::make_unique<ReportJob>(*store, run.cap_query(), run.lens,
                                                     settings.auto_open_report,
                                                     settings.hit_window_ms, run,
                                                     batch_job->take_report_seed());
            report_job->start();
        }
    }

    // The finished strip shows the report's outcome. Dismissed before the
    // report landed, the outcome goes to the status line instead.
    if (!batch_job && report_job && report_job->finished()) {
        library_ui.report_checked_at = -1.0;  // a new report: look at once
        if (!report_outcome_shown && !report_job->is_cancelled()) {
            const std::string where = report_job->saved_path().u8string();
            if (!report_job->ok())
                set_problem("The path report could not be built. " + report_job->message());
            else if (!report_job->open_problem().empty())
                set_problem("Path report saved to " + where + ". " + report_job->open_problem());
            else
                set_status("Path report saved to " + where + ".");
        }
        report_job.reset();
    }

    // Let go of cancelled leaderboard jobs once their request has returned.
    parked_dm_fetches.erase(
        std::remove_if(parked_dm_fetches.begin(), parked_dm_fetches.end(),
                       [](const std::unique_ptr<DmFetchUsersJob>& job) { return job->finished(); }),
        parked_dm_fetches.end());
    parked_dm_reports.erase(
        std::remove_if(parked_dm_reports.begin(), parked_dm_reports.end(),
                       [](const std::unique_ptr<DmReportJob>& job) { return job->finished(); }),
        parked_dm_reports.end());
}

void AppState::cancel_dm_fetch() {
    if (!dm_fetch_job) return;
    dm_fetch_job->cancel();
    if (!dm_fetch_job->finished()) parked_dm_fetches.push_back(std::move(dm_fetch_job));
    dm_fetch_job.reset();
}

void AppState::cancel_dm_report() {
    if (!dm_report_job) return;
    dm_report_job->cancel();
    if (!dm_report_job->finished()) parked_dm_reports.push_back(std::move(dm_report_job));
    dm_report_job.reset();
}

void AppState::start_dm_fetch() {
    if (dm_fetch_job && !dm_fetch_job->finished()) return;
    dm_users.clear();
    dm_fetch_job = std::make_unique<DmFetchUsersJob>();
    dm_fetch_job->start();
}

void AppState::start_dm_report(const std::string& discord_id, const std::string& username) {
    if (dm_report_job && !dm_report_job->finished()) return;
    dm_report_job = std::make_unique<DmReportJob>(*store, discord_id, username,
                                                  settings.chartmode_key(),
                                                  settings.lens(),
                                                  settings.auto_open_report);
    // Remember the choice so the picker can pre-select it next time.
    settings.dm_last_user = discord_id;
    commit_settings();
    library_ui.dm_opened_by_click = false;
    dm_report_job->start();
}

void AppState::set_status(std::string message) {
    status_message = std::move(message);
    status_is_problem = false;
    status_generation.bump();
}

void AppState::set_problem(std::string message) {
    status_message = std::move(message);
    status_is_problem = true;
    status_generation.bump();
}

void AppState::dismiss_status() {
    status_message.clear();
    status_is_problem = false;
    status_generation.bump();
}

void AppState::commit_settings() {
    save_settings();
    apply_settings();
}

void AppState::edit_settings() {
    settings_unsaved_ = true;
    apply_settings();
}

void AppState::flush_settings() {
    if (settings_unsaved_) save_settings();
}

void AppState::save_settings() {
    settings_unsaved_ = false;
    if (!settings.save())
        set_problem("Settings could not be saved — " + app::ini_path() +
                   " is not writable.");
}

void AppState::apply_settings() {
    // Which record a chart shows is (chart, chart mode, SP cap, lens). When
    // any of the last three moves, every cached lookup is answering the old
    // question and has to be re-asked.
    std::string chartmode = settings.chartmode_key();
    store::CapQuery cap = settings.cap_query();
    store::Lens lens = settings.lens();
    if (chartmode != committed_chartmode_ || cap != committed_cap_ || lens != committed_lens_) {
        // The rows themselves come from the scan and stay; their summaries
        // are read again, and the open song is analyzed again under the new
        // settings, as a click would (D90 item 1).
        refresh_library_summaries();
        if (show_details && selected) start_view();
    }
    committed_chartmode_ = std::move(chartmode);
    committed_cap_ = cap;
    committed_lens_ = lens;
}

}  // namespace hydra::ui
