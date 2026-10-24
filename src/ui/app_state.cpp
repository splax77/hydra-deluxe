#include "ui/app_state.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <unordered_set>

#include "app/config.h"
#include "app/dm_report.h"  // settings_change_touches
#include "app/report.h"     // settings_change_touches
#include "app/rules_file.h"
#include "app/update_key.h"
#include "app/user_messages.h"
#include "core/app_version.h"
#include "core/model.h"  // counted, has_have, group_thousands
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
        start.rules_error = app::plain_error_detail(e);
    }
    return start;
}

// Whether a chart's stored answer already says what `fresh` would save, so a
// click saves nothing (D87 item 2): the same Ready record, or the same stored
// failure, kind and text.
bool stored_says_same(const store::RecordLookup& found, const store::RecordRow& fresh) {
    if (fresh.failure_kind)
        return found.status == store::RecordStatus::CantAnalyze &&
               found.failure_kind == fresh.failure_kind && found.failure_text == fresh.failure_text;
    return found.status == store::RecordStatus::Ready && found.record == fresh;
}

// The record a click's failed analysis is stored as: app::prepare_failed_chart's
// answer for what the analysis threw. Nothing when it threw nothing.
std::optional<store::PreparedRecord> click_failure_record(const std::string& song_id,
                                                          const store::Profile& profile,
                                                          const std::exception_ptr& thrown) {
    if (!thrown) return std::nullopt;
    try {
        std::rethrow_exception(thrown);
    } catch (const std::exception& e) {
        return app::prepare_failed_chart(song_id, profile, e);
    } catch (...) {
    }
    return std::nullopt;
}

}  // namespace

AppState::AppState() : AppState(load_startup_settings()) {}

// The store opens on its own thread, so the window draws while an old file
// upgrades. A bad rules file gates the store on RulesStamp::none(), so no row
// reads Ready under the defaults the settings still hold.
AppState::AppState(StartupSettings start) : AppState(std::move(start.settings), nullptr) {
    rules_error = std::move(start.rules_error);
    store_open_job = std::make_unique<StoreOpenJob>(
        app::db_path(),
        rules_error.empty() ? core::RulesStamp::of(settings.rules) : core::RulesStamp::none(),
        settings);
    store_open_job->start();
}

AppState::AppState(app::Settings initial_settings,
                   std::unique_ptr<store::RecordStore> initial_store)
    : settings(std::move(initial_settings)),
      store(std::move(initial_store)),
      update_public_key(app::kUpdatePublicKey),
      committed_settings_(settings) {
    // No store: the default constructor's open job brings it, and the
    // library with it (collect_store_open).
    if (store) reload_library();
    start_updates();
}

// Out-of-line so the unique_ptr<PreviewController> can be a forward declaration
// in the header (its destructor needs the full type, which lives here).
AppState::~AppState() { flush_settings(); }  // an edit in progress still lands

StoreOpenProgress AppState::store_open_progress() const {
    return store_open_job ? store_open_job->progress() : StoreOpenProgress{};
}

bool AppState::store_open_shown() const {
    return store_open_job && !store_open_job->finished() &&
           progress_delay_passed(store_open_job->elapsed_s());
}

void AppState::wait_store_open() {
    if (!store_open_job) return;
    store_open_job->wait();
    collect_store_open();
}

void AppState::collect_store_open() {
    if (!store_open_job || !store_open_job->finished()) return;
    const std::unique_ptr<StoreOpenJob> job = std::move(store_open_job);
    if (!job->ok()) {
        store_open_error_ = job->failure();
        return;
    }
    store = job->take_store();
    library = job->take_library();
    library.set_query(search);
    // The library read failed after the store opened: the same problem line
    // a failed reload shows, over an empty library.
    if (!job->library_problem().empty()) set_problem(job->library_problem());
    startup_notice_ = job->startup_notice();
}

void AppState::startup_notice_closed() {
    if (startup_notice_.empty()) return;
    startup_notice_.clear();
    // The fresh library is empty; the chart folders came from the ini, so
    // the scan finds the same songs. start_scan refuses when can_scan says
    // no, as the toolbar's button does.
    request_scan = true;
}

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
    const std::string problem = read_library(*store, settings, library);
    library.set_query(search);
    if (!problem.empty()) set_problem(problem);
    follow_selected_row();
}

// The open song panel follows its library row after a reload (a rescan, or
// reidentify_chart after a click found edited files): its entry is read
// again by row_key, the lookup select uses (D114 item 7). A chart the reload
// dropped keeps the panel as it was.
void AppState::follow_selected_row() {
    if (!selected) return;
    for (const LibraryRow& r : library.rows()) {
        if (!is_selected_row(r.entry)) continue;
        if (std::optional<store::ChartLibraryEntry> entry = read_entry(row_key(r.entry), r.entry.md5))
            selected = std::move(*entry);
        return;
    }
}

const LibraryChart* AppState::library_row_for(const std::string& key) const {
    for (const LibraryRow& r : library.rows())
        if (row_key(r.entry) == key) return &r.entry;
    return nullptr;
}

std::optional<store::ChartLibraryEntry> AppState::read_entry(const std::string& key,
                                                             const std::string& md5) {
    std::vector<store::ChartLibraryEntry> copies;
    if (!read_store([&] { copies = store->list_chart_library_copies(md5); })) return std::nullopt;
    for (store::ChartLibraryEntry& copy : copies)
        if (row_key(copy) == key) return std::move(copy);
    return std::nullopt;
}

void AppState::refresh_library_summaries() {
    read_store([&] { read_library_summaries(*store, settings, library); });
}

bool AppState::refresh_library_row(const std::string& md5) {
    bool changed = false;
    read_store([&] {
        changed = library.set_summary_for(md5, store->lookup_one(md5, settings.profile())) > 0;
    });
    return changed;
}

void AppState::set_search(std::string text) {
    search = std::move(text);
    library.set_query(search);
}

std::vector<store::ChartLibraryEntry> AppState::library_matches() const {
    const std::vector<size_t>& matched = library.order();
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
    // A row the store no longer lists is left out. The confirm reloads after
    // a scan first (reload_after_scan), so that happens only when that reload
    // failed and said so.
    std::vector<store::ChartLibraryEntry> out;
    out.reserve(matched.size());
    for (std::optional<store::ChartLibraryEntry>& e : found)
        if (e) out.push_back(std::move(*e));
    return out;
}

bool AppState::reload_after_scan() {
    if (!scan_job || scan_reloaded_ || !scan_job->snapshot().finished) return false;
    scan_reloaded_ = true;
    reload_library();
    return true;
}

void AppState::tick_library(double now) {
    reload_after_scan();
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
    reload_after_scan();  // the neighbour in the rows the store holds now
    if (std::optional<size_t> i = relative_row(delta)) select(view_row(*i), SelectionSource::Step);
}

void AppState::select(const store::ChartLibraryEntry& entry, SelectionSource source) {
    // The previous chart's panel, torn down the one way: a row click or
    // previous / next swaps the song while the panel stays open.
    close_details();
    selected = entry;
    last_selection_source = source;
    show_details = true;
    start_view();
}

void AppState::select(const LibraryChart& row, SelectionSource source) {
    // Kept by value: a reload replaces the rows `row` may live in.
    const std::string key = row_key(row);
    std::string md5 = row.md5;
    if (reload_after_scan()) {
        // The clicked row as the new scan wrote it, which may be under a new
        // hash. None: the scan removed it, and the click opens nothing.
        const LibraryChart* now = library_row_for(key);
        if (!now) return;
        md5 = now->md5;
    }
    // None: the rows are older than the store only when the reload after a
    // scan failed, and that read said so.
    if (std::optional<store::ChartLibraryEntry> entry = read_entry(key, md5)) select(*entry, source);
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
    // The next song opened is not a re-analysis, and no held height pads it.
    view_reanalysis_ = false;
    details_ui.headline = {};
    details_ui.path_picker = {};
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
    if (!selected) {
        view_reanalysis_ = false;
        return;
    }
    if (!file_exists_utf8(selected->notespath)) {
        viewed.state = ViewedSong::State::FileMissing;
        view_reanalysis_ = false;
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
    view_job = std::make_unique<ViewJob>(*selected, settings.profile(),
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
        view_reanalysis_ = false;
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

bool AppState::view_holds_space() const {
    return viewed.state == ViewedSong::State::Analyzing && view_reanalysis_;
}

bool AppState::view_progress_shown() const {
    return view_running() &&
           progress_delay_passed(
               std::chrono::duration<double>(std::chrono::steady_clock::now() - view_requested_at_)
                   .count());
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

    JobOutcome outcome = job->outcome();
    // The click's one exception to outcome()'s order: a run that finished
    // and was then cancelled is dropped, not shown. That Cancel is the
    // panel's Cancel button (closing the panel drops the run above, by
    // generation), and nothing is saved for a song whose analysis the user
    // stopped, as close_details says (D87 item 11).
    if (outcome == JobOutcome::Done && job->is_cancelled()) outcome = JobOutcome::Cancelled;

    ViewedSong v;
    if (outcome == JobOutcome::Cancelled) {
        v.state = ViewedSong::State::Cancelled;
    } else if (outcome == JobOutcome::Failed) {
        v.state = ViewedSong::State::Failed;
        v.message = job->message();
        v.error = job->error();
    } else {
        ViewOutcome out = job->take_outcome();
        v.dynamics = std::move(out.dynamics);
        v.dynamics_message = std::move(out.dynamics_message);
        v.dynamics_error = std::move(out.dynamics_error);
        const store::Profile profile = job->profile();
        std::string song_id = job->song().md5;
        // Changed chart files: the library row takes the hash and fingerprint
        // they give now, before anything is saved under them. An edited chart
        // gets a new hash (D87 item 3); one only saved again keeps its hash
        // and takes the new fingerprint (D96), when ViewJob::run finds that
        // fingerprint worth saving. The library is read again either way,
        // because the next row click looks its chart up by the row's hash.
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
                v.error = app::plain_error_detail(e);
                viewed = std::move(v);
                view_reanalysis_ = false;
                record_generation.bump();
                return;
            }
            song_id = out.new_md5;
            reload_library();  // the panel's entry follows its row (follow_selected_row)
        }
        if (job->analysis_off()) {
            v.state = ViewedSong::State::RulesBroken;
        } else if (!out.analysis) {
            v.state = ViewedSong::State::Failed;
            v.message = std::move(out.analysis_message);
            v.error = std::move(out.analysis_error);
            // A failure the store remembers is the chart's result, as a
            // batch stores it. The panel shows the failure either way.
            try {
                if (const std::optional<store::PreparedRecord> failure =
                        click_failure_record(song_id, profile, out.analysis_failure))
                    save_view_record(*failure);
            } catch (const std::exception& e) {
                set_problem(app::plain_error(e));
            }
        } else {
            try {
                v.summary = save_view_summary(song_id, profile, *out.analysis);
            } catch (const std::exception& e) {
                set_problem("Analyzed, but saving failed. " + app::plain_error(e));
                // The engine's own summary, as a save would have shown it.
                // record_of skips the profile check, so this shows the
                // analysis even when the save refused it for not fitting.
                v.summary = store::record_of(song_id, out.analysis->record).summary();
            }
            v.state = ViewedSong::State::Ready;
            v.timing = out.analysis->song.timing();
            v.song_length_ms = out.length.ms;
            v.record = std::move(out.analysis->record);
        }
    }
    viewed = std::move(v);
    view_reanalysis_ = viewed.ready();
    record_generation.bump();
}

store::PathSummary AppState::save_view_summary(const std::string& song_id,
                                               const store::Profile& profile,
                                               const app::AnalysisResult& result) {
    const store::PreparedRecord prepared = store::prepare_record(song_id, profile, result.record);
    save_view_record(prepared);
    return prepared.record.summary();
}

void AppState::save_view_record(const store::PreparedRecord& prepared) {
    const std::string& song_id = prepared.record.song_id;
    const store::RecordLookup found = store->lookup_one(song_id, prepared.profile);
    if (stored_says_same(found, prepared.record)) return;  // nothing new to save
    store->save(prepared);
    refresh_library_row(song_id);  // its row's Best path cell and chip
    // Both open reports now read an older library (D115 question 4). No
    // batch finished, so the strip names no time.
    mark_reports_out_of_date(ReportOutOfDate::Library, /*path=*/true, /*dm=*/true, std::nullopt);
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

bool AppState::can_analyze() const { return !analysis_blocked() && !batch_running(); }

bool AppState::any_job_running() const {
    // view_job counts while it runs even when cancelled: tick() still has to
    // collect it (and start a waiting request).
    return (store_open_job && !store_open_job->finished()) ||
           (scan_job && !scan_job->snapshot().finished) || batch_running() || view_pending ||
           view_thread_alive() || (report_job && !report_job->finished()) ||
           (dm_fetch_job && !dm_fetch_job->finished()) ||
           (dm_report_job && !dm_report_job->finished()) || (preview && preview->busy()) ||
           (update_check_job && !update_check_job->finished()) ||
           (update_download_job && !update_download_job->finished());
}

void AppState::tick(double /*now*/) {
    collect_store_open();
    if (!show_details && details_ui.prev_open) close_details();
    details_ui.prev_open = show_details;
    update_view_job();
    tick_updates();
}

void AppState::select_chart(const std::string& hyhash, const std::string& chartmode) {
    if (chartmode != settings.chartmode_key()) {
        // The bar is off during a batch, so a row can't switch it either.
        if (settings_locked()) {
            set_status(kBatchRunningStatus);
            return;
        }
        // Settings::with_chartmode picks the choices, which are then
        // committed as the bar commits. When it finds none, this build can't
        // analyze the row's mode, so the row selects nothing.
        std::optional<Settings> mode = settings.with_chartmode(chartmode);
        if (!mode) return;
        settings = std::move(*mode);
        commit_settings();
    }
    reload_after_scan();  // the copies the store holds now
    // Ids are lowercase hex everywhere (the scan writes them, the store
    // lowercases old ones), so a copy is an exact match.
    const std::vector<LibraryRow>& rows = library.rows();
    auto is_copy = [&](size_t i) { return rows[i].entry.md5 == hyhash; };
    // The first copy the table shows; a copy the search or chip hides is
    // still the chart, so the rest of the library comes after.
    for (size_t i : library.order()) {
        if (is_copy(i)) {
            select(rows[i].entry, SelectionSource::Elsewhere);
            return;
        }
    }
    for (size_t i = 0; i < rows.size(); ++i) {
        if (is_copy(i)) {
            select(rows[i].entry, SelectionSource::Elsewhere);
            return;
        }
    }
}

void AppState::start_scan() {
    if (!can_scan()) {
        // The toolbar's button is off during a batch, but "Scan now" and the
        // panel's "Rescan library" ask from here: say why nothing happened.
        if (batch_running()) set_status(kBatchRunningStatus);
        return;
    }
    scan_job = std::make_unique<ScanJob>(settings.chartfolders, *store);
    scan_reloaded_ = false;
    scan_job->start();
}

void AppState::open_batch_confirm() {
    // Exactly the rows on screen (library_matches). The plans made here are
    // the ones the batch runs (D79), so the confirm, the strip and the
    // finished counts come from one plan. Rows older than the store would
    // count fewer charts than the button's N.
    // The report window's question gives way to the user's own confirm.
    if (path_report_awaits_confirm()) decline_report_batch();
    reload_after_scan();
    if (!plan_batches([&] { return library_matches(); })) {
        // The database failed: the confirm stays closed (D72 item 4).
        close_batch_confirm();
        return;
    }
    batch_confirm_pending = true;
}

bool AppState::plan_batches(const std::function<std::vector<app::ScanItem>()>& list_items) {
    const app::BatchRun run = settings.batch_run();
    return read_store([&] {
        const std::vector<app::ScanItem> items = list_items();
        batch_plan = app::plan_batch(items, app::charts_with_result(*store, run, false));
        batch_redo_plan = app::plan_batch(items, app::charts_with_result(*store, run, true));
    });
}

void AppState::close_batch_confirm() {
    batch_confirm_pending = false;
    batch_plan = {};
    batch_redo_plan = {};
}

std::string AppState::batch_question(bool redo) const {
    // Library rows, each copy of a chart counted, like the library's own
    // counts (D76). What runs is the plan `redo` picks (D79).
    const int64_t with = batch_scope_with_result();
    const int64_t to_run = batch_plan_for(redo).todo_rows();
    if (to_run == 0) return "Every chart here already has a result.";
    if (redo && with > 0)
        return "Analyze " + counted(to_run, "chart", "charts") + ", re-analyzing " +
               group_thousands(with) + " that already " + has_have(with) + " a result?";
    return "Analyze " + counted(to_run, "chart", "charts") + " that " + has_have(to_run) +
           " no result yet?";
}

void AppState::start_batch(bool redo) {
    if (!can_analyze()) return;
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
    batch_seen_completed_ = 0;
    batch_refreshed_at_ = -1.0;
    batch_finish_seen_ = false;
    apply_app_batch_analyzer_for_test(*batch_job);
    batch_job->start();
}

void AppState::update_background_jobs() {
    settle_report_wait();
    if (batch_job && !batch_finish_seen_ && batch_job->snapshot().finished) {
        batch_finish_seen_ = true;
        // tick_library re-reads the summaries once the batch ends. Stopped
        // or not, the run stored results, so both reports in memory now
        // read an older library (D103). Their windows name this moment as
        // the batch's finish time (D103 item 21).
        mark_reports_out_of_date(ReportOutOfDate::Library, /*path=*/true, /*dm=*/true,
                                 std::chrono::system_clock::now());
        // One path report per finished run. A stopped run keeps its results
        // but builds no report: a report of part of the library would read
        // as the whole of it.
        const bool stopped = batch_job->is_cancelled();
        const bool waited = path_report_batch() != nullptr;
        if (!stopped && !report_started) {
            report_started = true;
            // The report lists the records the batch filed: its cap and lens,
            // not whatever the analysis settings hold now, analyzed under the
            // batch's settings. A report that waited on this batch is the
            // window's own build, with the window's cause.
            launch_path_report(batch_job->batch_run(),
                               waited ? report_wait_cause_ : PathReportCause::Batch);
        }
        if (waited) {
            // A wait on a stopped batch ends as a cancelled build: Try again.
            if (stopped) path_report.mark_cancelled();
            report_wait_ = ReportWait::None;
        }
    }

    collect_path_report();
    collect_dm_report();

    // Let go of cancelled jobs once they have returned.
    parked_reports.erase(
        std::remove_if(parked_reports.begin(), parked_reports.end(),
                       [](const std::unique_ptr<ReportJob>& job) { return job->finished(); }),
        parked_reports.end());
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
    // One that already finished has its result: keep it, not the cancel.
    if (dm_report_job->finished()) {
        collect_dm_report();
        return;
    }
    dm_report_job->cancel();
    parked_dm_reports.push_back(std::move(dm_report_job));
    dm_report_job.reset();
    dm_report.mark_cancelled();
}

void AppState::start_dm_fetch() {
    if (dm_fetch_job && !dm_fetch_job->finished()) return;
    dm_users.clear();
    dm_fetch_job = std::make_unique<DmFetchUsersJob>();
    dm_fetch_job->start();
}

void AppState::reopen_dm_picker() {
    dm_picker_open = true;
    dm_picker_popup_pending = true;
    if (dm_users.empty()) start_dm_fetch();
}

void AppState::start_dm_report(const std::string& discord_id, const std::string& username) {
    // A new player replaces one still building (D103 item 9). Parked, never
    // joined here.
    if (dm_report_job && !dm_report_job->finished()) {
        dm_report_job->cancel();
        parked_dm_reports.push_back(std::move(dm_report_job));
    }
    dm_report_job.reset();
    dm_player_id_ = discord_id;
    dm_player_name_ = username;
    // Remember the choice so the picker can pre-select it next time.
    settings.dm_last_user = discord_id;
    commit_settings();
    dm_report.window_open = true;
    launch_dm_report();
}

void AppState::request_dm_report() {
    if (!store || dm_player_id_.empty()) return;
    if (dm_report_job && !dm_report_job->finished()) return;
    collect_dm_report();  // one that finished this frame lands first
    launch_dm_report();
}

void AppState::launch_dm_report() {
    dm_report.out_of_date_during_build = ReportOutOfDate::None;
    dm_report_job = std::make_unique<DmReportJob>(*store, dm_player_id_, dm_player_name_,
                                                  settings.profile_at_clone_hero_cap());
    dm_report_job->start();
}

ReportBuild AppState::path_report_build() const {
    if (report_job && !report_job->finished()) return ReportBuild::Building;
    return report_wait_ != ReportWait::None ? ReportBuild::Building : path_report.last;
}

const BatchJob* AppState::path_report_batch() const {
    const bool on_batch = report_wait_ == ReportWait::OwnBatch || report_wait_ == ReportWait::Batch;
    return on_batch ? batch_job.get() : nullptr;
}

void AppState::settle_report_wait() {
    if (report_wait_ == ReportWait::Confirm && !batch_confirm_pending) {
        // The window's question was answered. Analyze ran the batch, which
        // the report now waits on; Cancel (decline_report_batch) leaves the
        // report to list what is stored.
        if (batch_job && !batch_finish_seen_) {
            report_wait_ = ReportWait::OwnBatch;
        } else {
            report_wait_ = ReportWait::None;
            launch_path_report(settings.batch_run(), report_wait_cause_);
        }
    }
    // The batch was let go before its finish was seen here: nothing says how
    // it ended, so the build counts as cancelled.
    if ((report_wait_ == ReportWait::OwnBatch || report_wait_ == ReportWait::Batch) && !batch_job) {
        report_wait_ = ReportWait::None;
        path_report.mark_cancelled();
    }
}

ReportBuild AppState::dm_report_build() const {
    return dm_report_job && !dm_report_job->finished() ? ReportBuild::Building : dm_report.last;
}

void AppState::request_path_report() { request_path_report(PathReportCause::Request); }

void AppState::request_path_report(PathReportCause cause) {
    if (!store || analysis_blocked()) return;
    if (path_report_build() == ReportBuild::Building) return;
    collect_path_report();  // one that finished this frame lands first
    // A batch still running builds the report when it finishes; the window
    // shows its progress meanwhile.
    if (batch_job && !batch_finish_seen_) {
        report_wait_ = ReportWait::Batch;
        report_wait_cause_ = cause;
        return;
    }
    const app::BatchRun run = settings.batch_run();
    // The user's own confirm is open: the report lists what is stored, as
    // the confirm's plans are the user's to run.
    if (batch_confirm_pending) {
        launch_path_report(run, cause);
        return;
    }
    // Every library row, planned as a batch plans it, so the run's counts
    // and the strip's are one plan's (D79).
    reload_after_scan();
    if (!plan_batches([&] { return store->list_chart_library(0, -1); })) {  // -1 = no limit
        // The database failed and the status line says so; the build reads
        // the store too and shows its own failure in the window.
        close_batch_confirm();
        launch_path_report(run, cause);
        return;
    }
    if (batch_plan.todo_rows() == 0) {
        close_batch_confirm();
        launch_path_report(run, cause);
        return;
    }
    // The plans the window's question counts and start_batch runs. The
    // question is the report's own, so the main window's popup stays shut
    // (batch_confirm_in_main_window).
    batch_confirm_pending = true;
    report_wait_cause_ = cause;
    report_wait_ = ReportWait::Confirm;
    // Few enough just run, as the window's Analyze runs them.
    if (batch_plan.todo_rows() <= report_confirm_above) start_path_report_batch();
}

void AppState::start_path_report_batch() {
    if (!path_report_awaits_confirm()) return;
    // The plan the question counted, never redone. A batch that can't start
    // leaves the window asking, its Analyze greyed.
    start_batch(false);
    settle_report_wait();
}

void AppState::decline_report_batch() {
    close_batch_confirm();
    settle_report_wait();
}

void AppState::launch_path_report(app::BatchRun run, PathReportCause cause) {
    // A build still running read an older library: it stops between charts,
    // and waits parked rather than freezing the window until then.
    if (report_job && !report_job->finished()) {
        report_job->cancel();
        parked_reports.push_back(std::move(report_job));
    }
    report_job = std::make_unique<ReportJob>(*store, std::move(run), settings.hit_window_ms);
    path_report_cause_ = cause;
    path_report.out_of_date_during_build = ReportOutOfDate::None;
    report_job->start();
}

void AppState::cancel_path_report() {
    // While the window asks, its Cancel means "don't analyze".
    if (path_report_awaits_confirm()) {
        decline_report_batch();
        return;
    }
    if (report_job && !report_job->finished()) report_job->cancel();
    if (report_wait_ == ReportWait::None) return;
    // The report's own batch stops as the strip's Stop stops it. A batch the
    // user started is the user's to stop.
    if (report_wait_ == ReportWait::OwnBatch && batch_job) batch_job->stop();
    report_wait_ = ReportWait::None;
    path_report.mark_cancelled();
}

void AppState::show_path_report() {
    path_report.window_open = true;
    if (!path_report.result && path_report_build() == ReportBuild::None)
        request_path_report(PathReportCause::Open);
}

void AppState::close_path_report() {
    // A batch the report waited on keeps running with its strip, and builds
    // the report when it finishes, as any batch does. A question the window
    // was asking goes with it, so no popup takes it up in the main window.
    if (path_report_awaits_confirm()) close_batch_confirm();
    report_wait_ = ReportWait::None;
    // A build still running stops as the window's Cancel stops it. It waits
    // parked, so neither its rows nor its Cancelled land with the window shut;
    // one that finished this frame is dropped the same way.
    cancel_path_report();
    if (report_job) parked_reports.push_back(std::move(report_job));
    // The slot starts over as never built: no rows, no outcome, nothing out
    // of date, so show_path_report builds again.
    path_report = PathReportSlot{};
}

bool AppState::path_report_offered() const {
    return !library.rows().empty() && !analysis_blocked();
}

namespace {

// How a finished report job ends up in its slot. The one spelling both
// collect_* functions share. Returns the job's outcome.
template <class Result, class Job>
ReportBuild land_report(ReportSlot<Result>& slot, const Job& job) {
    const ReportOutOfDate during =
        std::exchange(slot.out_of_date_during_build, ReportOutOfDate::None);
    switch (job.outcome()) {
        case JobOutcome::Done:
            slot.result = job.result();
            slot.built_at = std::chrono::system_clock::now();
            slot.out_of_date = during;
            slot.last = ReportBuild::Ready;
            slot.message.clear();
            slot.error.clear();
            break;
        case JobOutcome::Cancelled:
            slot.mark_cancelled();
            break;
        case JobOutcome::Failed:
            slot.last = ReportBuild::Failed;
            slot.message = job.message();
            slot.error = job.error();
            break;
    }
    return slot.last;
}

}  // namespace

void AppState::collect_path_report() {
    if (!report_job || !report_job->finished()) return;
    const std::unique_ptr<ReportJob> job = std::move(report_job);
    const ReportBuild outcome = land_report(path_report, *job);
    // Only new rows carry a cause; a cancelled or failed build leaves the
    // rows, and what they were built for, as they were.
    if (outcome == ReportBuild::Ready)
        path_report.built_on_open = path_report_cause_ == PathReportCause::Open;
    if (path_report_cause_ != PathReportCause::Batch) return;
    // The batch's own report: "Open automatically" opens it (D103 item 13).
    if (outcome == ReportBuild::Ready && settings.auto_open_report)
        path_report.window_open = true;
    // The finished strip shows the outcome. Dismissed before the report
    // landed, the outcome goes to the status line instead.
    if (batch_job) return;
    if (outcome == ReportBuild::Ready)
        set_status(app::kPathReportReady);
    else if (outcome == ReportBuild::Failed)
        set_problem(std::string(app::kPathReportFailed) + " " + path_report.message);
}

void AppState::collect_dm_report() {
    if (!dm_report_job || !dm_report_job->finished()) return;
    const std::unique_ptr<DmReportJob> job = std::move(dm_report_job);
    land_report(dm_report, *job);
}

void AppState::mark_reports_out_of_date(
    ReportOutOfDate reason, bool path, bool dm,
    std::optional<std::chrono::system_clock::time_point> batch_finished) {
    auto mark = [&](auto& slot, bool building) {
        if (slot.result) slot.out_of_date = reason;
        if (building) slot.out_of_date_during_build = reason;
        if (batch_finished && (slot.result || building)) slot.batch_finished = batch_finished;
    };
    if (path) mark(path_report, path_report_build() == ReportBuild::Building);
    if (dm) mark(dm_report, dm_report_build() == ReportBuild::Building);
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
    const app::Settings& before = committed_settings_;
    if (settings.profile() != before.profile()) {
        // The rows themselves come from the scan and stay; their summaries
        // are read again, and the open song is analyzed again under the new
        // settings, as a click would (D90 item 1).
        refresh_library_summaries();
        if (show_details && selected) start_view();
    }
    // Each report's own settings_change_touches says whether the change
    // reaches what that report read (D103 item 22).
    mark_reports_out_of_date(ReportOutOfDate::Settings,
                       /*path=*/app::report::settings_change_touches(before, settings),
                       /*dm=*/app::dm_report::settings_change_touches(before, settings));
    committed_settings_ = settings;
}

// ---- updates ----------------------------------------------------------------

namespace {

// The version bar's fixed sentences (D259 and D260 in
// docs/audit/2026-10-03-fix-decisions.md). The ones that name a version are
// built with core::named_version.
constexpr const char* kUpdateVerifyingText = "Checking the download...";
constexpr const char* kUpdateLaunchingText = "Starting the installer...";
constexpr const char* kUpdateDeclinedText = "The installer didn't start, so Hydra wasn't updated.";
constexpr const char* kOnlyInstalledChecksText =
    "Only the installed copy of " HYDRA_APP_NAME " checks for updates.";
constexpr const char* kCheckingForUpdatesText = "Checking for updates...";
constexpr const char* kCouldNotCheckText = "Hydra couldn't check for updates when it started.";
constexpr const char* kUpToDateText = "Up to date";
constexpr const char* kSkippedText = "Skipped.";

// "Hydra Deluxe 9.9.9 is available.": the offer's sentence and the resting
// bar's after Later or Skip this version.
std::string available_text(const core::AppVersion& version) {
    return core::named_version(version) + " is available.";
}

// Inno Setup's log, beside the download.
std::string installer_log_path() { return join_folder(update_folder(), "setup.log"); }

}  // namespace

void AppState::start_updates() {
    // The first start after any version change says so once. An empty value
    // is a first start, which has nothing to compare.
    const core::AppVersion running = core::running_version();
    const std::optional<core::AppVersion> last = core::parse_version(settings.last_run_version);
    if (!settings.last_run_version.empty() && last != running)
        set_update_phase(UpdateState::Phase::Updated,
                         "Updated to " + core::named_version(running) + ".");
    if (last != running) {
        settings.last_run_version = core::version_text(running);
        committed_settings_.last_run_version = settings.last_run_version;
        save_settings();
    }
    // The installer a declined start kept goes at the next start.
    try {
        std::error_code ec;
        std::filesystem::remove_all(os_path(update_folder()), ec);
    } catch (const std::exception&) {
        // No temp folder: there is no leftover to remove either.
    }
    check_updates_on_start();
}

void AppState::set_update_phase(UpdateState::Phase phase, std::string text) {
    update.phase = phase;
    update.text = std::move(text);
    update.detail.clear();
}

void AppState::offer_update(const app::UpdateRelease& release) {
    update.release = release;
    set_update_phase(UpdateState::Phase::Offered, available_text(release.version));
}

void AppState::check_updates_on_start() {
    if (update_check_job || !app::installed_by_setup()) return;
    update_check_job = std::make_unique<UpdateCheckJob>();
    update_check_job->start();
}

bool AppState::update_check_running() const { return update_check_job != nullptr; }

bool AppState::found_release_skipped() const {
    return update.found &&
           core::parse_version(settings.skipped_update_version) == update.found->version;
}

VersionBarStatus AppState::version_bar_status() const {
    using Dot = VersionBarStatus::Dot;
    using LastCheck = UpdateState::LastCheck;
    VersionBarStatus status;
    if (update.phase != UpdateState::Phase::None) {
        // The phase's own sentence; the bar adds its buttons and detail line.
        status.dot = update.phase == UpdateState::Phase::Offered ? Dot::Available : Dot::Neutral;
        status.text = update.text;
        return status;
    }
    if (!app::installed_by_setup()) {
        status.dimmed = kOnlyInstalledChecksText;
        return status;
    }
    if (update_check_running()) {
        status.text = kCheckingForUpdatesText;
        return status;
    }
    if (update.last_check == LastCheck::Failed) {
        status.text = kCouldNotCheckText;
        status.dimmed = update.check_error;
        return status;
    }
    if (update.last_check == LastCheck::Newer && update.found) {
        status.dot = Dot::Available;
        status.text = available_text(update.found->version);
        const bool skipped = found_release_skipped();
        if (skipped) status.dimmed = kSkippedText;
        status.update_button = !skipped;
        return status;
    }
    status.dot = Dot::UpToDate;
    status.text = kUpToDateText;
    return status;
}

const app::UpdateRelease* AppState::bar_release() const {
    if (update.phase != UpdateState::Phase::None) return update.release ? &*update.release : nullptr;
    if (version_bar_status().dot == VersionBarStatus::Dot::Available && update.found)
        return &*update.found;
    return nullptr;
}

void AppState::tick_updates() {
    collect_update_check();
    collect_update_download();
    // The installer closes Hydra through Restart Manager once its UAC prompt
    // is accepted. Its process ending while Hydra still runs means it did
    // not update (decision 10); the download stays for Update to try again.
    if (installer_ended_ && installer_ended_()) {
        installer_ended_ = nullptr;
        set_update_phase(UpdateState::Phase::Declined, kUpdateDeclinedText);
    }
}

void AppState::collect_update_check() {
    if (!update_check_job || !update_check_job->finished()) return;
    const std::unique_ptr<UpdateCheckJob> job = std::move(update_check_job);
    if (job->outcome() == JobOutcome::Cancelled) return;
    // What the check found is kept whatever the bar shows, so the Updated
    // note's OK can still offer it (dismiss_update_note).
    if (!job->ok()) {
        // A failed check shows in the resting status, with the plain_error
        // sentence; the next start tries again.
        update.last_check = UpdateState::LastCheck::Failed;
        update.check_error = job->message();
        update.found.reset();
        return;
    }
    const app::UpdateRelease& release = job->release();
    update.check_error.clear();
    if (!core::is_newer_version(release.version, core::running_version())) {
        update.last_check = UpdateState::LastCheck::UpToDate;
        update.found.reset();
        return;
    }
    update.last_check = UpdateState::LastCheck::Newer;
    update.found = release;
    // The offer replaces only the resting status; a skipped release is named
    // there, not offered.
    if (update.phase == UpdateState::Phase::None && !found_release_skipped()) offer_update(release);
}

void AppState::update_now() {
    using Phase = UpdateState::Phase;
    if (update_download_job) return;
    // The resting bar's Update starts the download as the offer's does.
    if (update.phase == Phase::None && version_bar_status().update_button) update.release = update.found;
    if (!update.release) return;
    if (update.phase != Phase::None && update.phase != Phase::Offered &&
        update.phase != Phase::Declined && update.phase != Phase::Failed)
        return;
    // After a declined start the kept download is checked again, not fetched.
    const bool reuse = update.phase == Phase::Declined;
    std::string folder;
    try {
        folder = update_folder();
    } catch (const std::exception& e) {
        set_update_phase(Phase::Failed, app::plain_error(e));
        update.detail = app::plain_error_detail(e);
        return;
    }
    update_download_job =
        std::make_unique<UpdateDownloadJob>(*update.release, folder, update_public_key, reuse);
    set_update_phase(Phase::Downloading, "Downloading " + core::named_version(update.release->version) + "...");
    update_download_job->start();
}

void AppState::collect_update_download() {
    using Phase = UpdateState::Phase;
    if (!update_download_job) return;
    if (!update_download_job->finished()) {
        if (update_download_job->verifying() && update.phase == Phase::Downloading)
            set_update_phase(Phase::Verifying, kUpdateVerifyingText);
        return;
    }
    const std::unique_ptr<UpdateDownloadJob> job = std::move(update_download_job);
    switch (job->outcome()) {
        case JobOutcome::Cancelled:
            // Decision 4: Cancel goes back to the offer.
            offer_update(*update.release);
            return;
        case JobOutcome::Failed:
            set_update_phase(Phase::Failed, job->message());
            update.detail = job->error();
            return;
        case JobOutcome::Done: break;
    }
    set_update_phase(Phase::Launching, kUpdateLaunchingText);
    try {
        installer_ended_ = launch_installer(job->installer_path(), installer_log_path());
    } catch (const std::exception& e) {
        // The user's ruling during T3a: a start that fails is decision 10's
        // state, with Windows' reason under it; the download stays.
        installer_ended_ = nullptr;
        set_update_phase(Phase::Declined, kUpdateDeclinedText);
        update.detail = app::plain_error_detail(e);
    }
}

void AppState::skip_version() {
    if (!update.release || update_download_job || update.phase == UpdateState::Phase::Launching)
        return;
    settings.skipped_update_version = core::version_text(update.release->version);
    committed_settings_.skipped_update_version = settings.skipped_update_version;
    save_settings();
    later();
}

void AppState::later() {
    if (update_download_job || update.phase == UpdateState::Phase::Launching) return;
    update.release.reset();
    set_update_phase(UpdateState::Phase::None, {});
}

void AppState::cancel_update() {
    if (update_download_job) update_download_job->cancel();
}

void AppState::dismiss_update_note() {
    using Phase = UpdateState::Phase;
    if (update.phase != Phase::Updated) return;
    set_update_phase(Phase::None, {});
    if (update.last_check == UpdateState::LastCheck::Newer && update.found &&
        !found_release_skipped())
        offer_update(*update.found);
}

uint64_t AppState::update_received() const {
    if (!update_download_job || update.phase != UpdateState::Phase::Downloading) return 0;
    return update_download_job->received();
}

std::string AppState::update_megabytes_text() const {
    if (update.phase != UpdateState::Phase::Downloading || !update.release) return {};
    char text[64];
    std::snprintf(text, sizeof(text), "%.1f of %.1f MB",
                  static_cast<double>(update_received()) / 1e6,
                  static_cast<double>(update.release->installer_size) / 1e6);
    return text;
}

}  // namespace hydra::ui
