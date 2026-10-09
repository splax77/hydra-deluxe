#include "ui/preview_controller.h"

#include <cstdint>
#include <algorithm>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "app/config.h"
#include "app/user_messages.h"
#include "audio/device.h"
#include "audio/player.h"
#include "ui/preview_load_job.h"

namespace hydra::ui {

namespace {

// Between the path part and the cap in an overlay key, typed once for the
// writer and the reader below.
constexpr const char* kCapSeparator = "|cap";

// The overlay's identity: the path it was built from plus the SP cap the
// meter was scaled to. A cap change must rebuild the scene just like a
// path change, so both live in the one key the three key_ members compare.
std::string overlay_key(const std::string& path_key, int sp_cap) {
    return path_key + kCapSeparator + std::to_string(sp_cap);
}

// The path part of an overlay_key: the one place a key is read back. The cap
// is written last, so the last separator ends the path part. Empty for an
// empty key (nothing has loaded).
std::string overlay_key_path_part(const std::string& key) {
    const std::size_t at = key.rfind(kCapSeparator);
    return at == std::string::npos ? std::string() : key.substr(0, at);
}

}  // namespace

PreviewController::PreviewController(ID3D11Device* device, ID3D11DeviceContext* context)
    : device_(device), context_(context), volume_pct_(app::Settings{}.preview_volume) {}

bool PreviewController::base_due() const {
    return !base_started_ && !job_ && !notes_job_ && !scene_job_ && !scene_base_ && song_ &&
           !song_->is_empty();
}

bool PreviewController::busy() const {
    return job_ || notes_job_ || base_job_ || scene_job_ || !retired_scene_jobs_.empty() ||
           !retired_base_jobs_.empty() || !retired_notes_jobs_.empty() || base_due();
}

namespace {

// Moves a running job to `retired` after cancelling it, so poll() drops it
// once its thread ends and the UI thread never waits on it.
template <typename Job>
void retire(std::unique_ptr<Job>& job, std::vector<std::unique_ptr<Job>>& retired) {
    if (!job) return;
    job->cancel();
    retired.push_back(std::move(job));
}

// Drops the retired jobs whose threads have ended.
template <typename Job>
void drop_finished(std::vector<std::unique_ptr<Job>>& retired) {
    retired.erase(std::remove_if(retired.begin(), retired.end(),
                                 [](const std::unique_ptr<Job>& j) { return j->finished(); }),
                  retired.end());
}

}  // namespace

PreviewController::~PreviewController() { close(); }

void PreviewController::open(const store::ChartLibraryEntry& entry, bool pro,
                             bool bass2x, Difficulty difficulty,
                             const Path* path, const std::string& path_key,
                             int sp_cap, const core::Rules& rules, bool noteshuffle) {
    rules_ = rules;
    const PreviewSongKey key{entry.md5, difficulty, pro, bass2x, noteshuffle};
    if (active_ && open_key_ == key) {
        // Same chart, same overlay: nothing to do, and nothing built.
        if (sp_cap == sp_cap_ && path_key == requested_path_key_) return;
        path_ = path ? std::optional<Path>(*path) : std::nullopt;
        sp_cap_ = sp_cap;
        requested_path_key_ = path_key;
        path_key_ = overlay_key(path_key, sp_cap);
        // Mid-load the load or notes job is building its own scene; poll()
        // reconciles. Once the song is here, the new overlay builds on a job
        // and poll() swaps it in, which leaves the audio and the playhead
        // alone.
        if (!job_ && !notes_job_ && song_ && !song_->is_empty()) start_scene_job();
        return;
    }
    // The same chart in another mode, with its audio already loaded: keep the
    // audio and the playhead, and reload only the notes.
    if (active_ && first_load_done_ && !job_ && open_key_.md5 == key.md5) {
        path_ = path ? std::optional<Path>(*path) : std::nullopt;
        sp_cap_ = sp_cap;
        requested_path_key_ = path_key;
        path_key_ = overlay_key(path_key, sp_cap_);
        start_notes_job(entry, key);
        return;
    }
    close();

    open_key_ = key;
    active_ = true;
    error_.clear();
    error_detail_.clear();
    pro_ = pro;
    sp_cap_ = sp_cap;

    path_ = path ? std::optional<Path>(*path) : std::nullopt;
    requested_path_key_ = path_key;
    path_key_ = overlay_key(path_key, sp_cap_);
    job_path_key_ = path_key_;
    job_ = std::make_unique<PreviewLoadJob>(entry, pro, bass2x, difficulty, path_, sp_cap_,
                                            rules, noteshuffle);
    job_->start();
}

void PreviewController::start_notes_job(const store::ChartLibraryEntry& entry,
                                        const PreviewSongKey& key) {
    open_key_ = key;
    // A "no notes in this mode" message goes away when the user switches back.
    error_.clear();
    error_detail_.clear();
    // Nothing built for the old notes may land on the new ones: the overlay,
    // the path-free base and an earlier notes reload are all retired.
    retire(scene_job_, retired_scene_jobs_);
    retire(base_job_, retired_base_jobs_);
    retire(notes_job_, retired_notes_jobs_);
    // The highway on screen is the old notes until the new ones land, so it
    // no longer answers shows_path for any selection.
    scene_path_key_.clear();
    job_path_key_ = path_key_;
    // A changed chart keeps drawing no path (drawn_path, finding 126).
    const Path* drawn = drawn_path(path_, chart_changed_);
    notes_job_ = std::make_unique<PreviewNotesJob>(
        entry, key.pro, key.bass2x, key.difficulty,
        drawn ? std::optional<Path>(*drawn) : std::nullopt, sp_cap_, rules_, key.noteshuffle,
        audio_end_ms_);
    notes_job_->start();
}

void PreviewController::take_notes_job() {
    if (notes_job_->ok()) {
        PreviewNotesJob::Result result = notes_job_->take_result();
        song_ = std::make_shared<const Song>(std::move(result.song));
        song_length_ms_ = result.song_length_ms;
        pro_ = notes_job_->pro();
        scene_ = std::move(result.scene);
        pending_track_ = std::move(result.track_state);
        pending_track_opts_ = result.track_opts;
        scene_path_key_ = job_path_key_;
        scene_dirty_ = true;
        // The path-free base belonged to the old notes; poll() builds the new
        // one as it does after a first load.
        scene_base_.reset();
        base_started_ = false;
        // The audio stays; only where playback ends moves with the last note.
        transport_.set_last_note_ms(hydra::app::last_note_ms(scene_));
    } else {
        error_ = notes_job_->message();
        error_detail_ = notes_job_->error();
        // The panel draws no Play button under an error, so audio left
        // playing would have no way to stop. The playhead stays where it is.
        transport_.pause();
        // Drop the old notes so no later overlay is built on them. The
        // transport stays loaded: switching back reloads only the notes.
        song_.reset();
        song_length_ms_.reset();
        scene_ = hydra::app::PreviewScene{};
        pending_track_.reset();
        scene_dirty_ = true;
        scene_base_.reset();
        base_started_ = false;
    }
    notes_job_.reset();
    job_path_key_.clear();
}

void PreviewController::start_scene_job() {
    retire(scene_job_, retired_scene_jobs_);
    // A changed chart keeps drawing no path (drawn_path, finding 126).
    const Path* drawn = drawn_path(path_, chart_changed_);
    std::optional<Path> path = drawn ? std::optional<Path>(*drawn) : std::nullopt;
    scene_job_ = std::make_unique<PreviewSceneJob>(song_, scene_base_, std::move(path), sp_cap_,
                                                   rules_, path_key_, track_opts(), audio_end_ms_,
                                                   song_length_ms_);
    scene_job_->start();
}

PreviewController::LoadProgress PreviewController::load_progress() const {
    if (!job_) return {};
    PreviewLoadJob::Progress p = job_->progress();
    return {p.fraction(), p.label(), p.time_left_text()};
}

void PreviewController::close() {
    // Stop the device before the audio it pulls from is destroyed.
    audio_device_.reset();
    transport_.unload();
    job_.reset();  // ResultJobBase's shutdown() joins the worker
    notes_job_.reset();
    scene_job_.reset();
    base_job_.reset();
    base_started_ = false;
    first_load_done_ = false;
    retired_scene_jobs_.clear();
    retired_base_jobs_.clear();
    retired_notes_jobs_.clear();
    requested_path_key_.clear();
    scene_ = hydra::app::PreviewScene{};
    scene_dirty_ = true;  // the renderer (if kept) must drop the old chart
    pending_track_.reset();  // scene_ is empty now; render() builds its (empty) timeline
    song_.reset();
    audio_end_ms_.reset();
    song_length_ms_.reset();
    scene_base_.reset();
    path_.reset();
    sp_cap_ = kCloneHeroSpCap;
    path_key_.clear();
    job_path_key_.clear();
    scene_path_key_.clear();
    have_frame_ = false;
    // The targets go (D95 call 4); zero sizes make the next render() resize.
    if (renderer_) renderer_->release_targets();
    rt_w_ = 0;
    rt_h_ = 0;
    active_ = false;
    scrubbing_ = false;
    resume_after_scrub_ = false;
    open_key_ = PreviewSongKey{};
    error_.clear();
    error_detail_.clear();
    audio_warning_.clear();
    chart_changed_ = false;
}

void PreviewController::poll() {
    // Replaced notes, base and overlay builds that have finished can go.
    drop_finished(retired_scene_jobs_);
    drop_finished(retired_base_jobs_);
    drop_finished(retired_notes_jobs_);
    // A new mode's notes are ready: swap them in (or show why not). If the
    // path changed while they were read, its overlay builds next.
    if (notes_job_ && notes_job_->finished()) {
        take_notes_job();
        if (song_ && scene_path_key_ != path_key_) start_scene_job();
    }
    // A new selection's overlay is ready: swap it in.
    if (scene_job_ && scene_job_->finished()) {
        if (scene_job_->ok()) {
            PreviewSceneJob::Output out = scene_job_->take_output();
            scene_base_ = std::move(out.base);  // the next path change reuses it
            scene_ = std::move(out.scene);
            pending_track_ = std::move(out.track_state);
            pending_track_opts_ = out.track_opts;
            scene_path_key_ = scene_job_->key();
            scene_dirty_ = true;
        } else if (error_.empty()) {
            error_ = scene_job_->message();
            error_detail_ = scene_job_->error();
        }
        scene_job_.reset();
    }
    // The path-free base, built once the chart has loaded so even the first
    // path change only lays an overlay over it. A scene job already running
    // without one builds its own, so none is started beside it.
    if (base_job_ && base_job_->finished()) {
        if (base_job_->ok() && !scene_base_) scene_base_ = base_job_->take_base();
        base_job_.reset();
    }
    if (base_due()) {
        base_job_ = std::make_unique<PreviewBaseJob>(song_, track_opts(), audio_end_ms_,
                                                     song_length_ms_);
        base_job_->start();
        base_started_ = true;
    }

    if (!job_ || !job_->finished()) return;

    const bool ok = job_->ok();
    if (ok) {
        PreviewLoadJob::Result result = job_->take_result();
        song_ = std::make_shared<const Song>(std::move(result.song));
        audio_end_ms_ = result.audio_end_ms;
        song_length_ms_ = result.song_length_ms;
        chart_changed_ = result.chart_changed;
        scene_ = std::move(result.scene);
        pending_track_ = std::move(result.track_state);
        pending_track_opts_ = result.track_opts;
        scene_path_key_ = job_path_key_;
        scene_dirty_ = true;
        first_load_done_ = true;
        transport_.set_gain(app::Settings::volume_gain(volume_pct_));
        transport_.load(std::make_unique<audio::Playhead>(std::move(result.audio)),
                        hydra::app::last_note_ms(scene_), result.audio_offset_ms);
        // Open the output device only when there is audio to play; a chart with
        // no locatable stems previews silently (the highway still draws).
        if (transport_.has_audio()) {
            AudioSource source = [this](float* out, int64_t frames) {
                return transport_.read_frames(out, frames);
            };
            try {
                audio_device_ =
                    device_factory_
                        ? device_factory_(transport_.channels(), transport_.sample_rate(),
                                          source)
                        : std::make_unique<audio::PreviewAudioDevice>(
                              transport_.channels(), transport_.sample_rate(), source);
                audio_device_->start();
            } catch (const std::exception& e) {
                // No device: still previewable, just muted. A warning, not
                // error_, which the panel treats as fatal.
                audio_device_.reset();
                audio_warning_ = e.what();
            }
        }
    } else {
        // The job worked out the plain sentence while it still had the
        // exception; the raw text ("StreamMix: ...") is the details line.
        error_ = job_->message();
        error_detail_ = job_->error();
    }
    job_.reset();
    job_path_key_.clear();

    // The Paths tab can change the selection while the load runs, and the job
    // built its scene from the path it was started with.
    if (ok && scene_path_key_ != path_key_) start_scene_job();
}

ID3D11ShaderResourceView* PreviewController::render(int width, int height) {
    if (width <= 0 || height <= 0)
        return have_frame_ && renderer_ ? renderer_->texture_srv() : nullptr;
    try {
        if (!renderer_) {
            // Authored highway art lives beside the exe, staged by the build
            // (or wherever app::set_path_overrides pointed a harness).
            const std::string asset_dir = hydra::app::asset_dir();
            renderer_ =
                std::make_unique<render::PreviewRenderer>(device_, context_, asset_dir);
        }
        if (width != rt_w_ || height != rt_h_) {
            renderer_->resize(width, height);
            rt_w_ = width;
            rt_h_ = height;
        }
        if (scene_dirty_) {
            const render::TrackStateOptions opts = track_opts();
            // A job built scene_'s timeline on its worker: just move it in.
            // If it was built for other options than the ones drawn now, or
            // there is none (the empty scene after close()), build it here as
            // before.
            if (pending_track_ && pending_track_opts_ == opts)
                renderer_->set_scene(scene_, std::move(*pending_track_));
            else
                renderer_->set_scene(scene_, opts);
            pending_track_.reset();
            scene_dirty_ = false;
            ++scene_generation_;  // the per-scene caches built before this are stale
        }
        renderer_->render(transport_.tick());
        have_frame_ = true;
        return renderer_->texture_srv();
    } catch (const std::exception& e) {
        if (error_.empty()) {
            error_ = hydra::app::plain_error(e);
            error_detail_ = hydra::app::plain_error_detail(e);
        }
        return nullptr;
    }
}

// ---- transport controls: forwarded to the Transport --------------------

void PreviewController::play() {
    if (!active_ || job_) return;  // nothing loaded yet
    transport_.play();
}

void PreviewController::pause() { transport_.pause(); }

void PreviewController::toggle() {
    if (transport_.playing())
        pause();
    else
        play();
}

bool PreviewController::playing() const { return transport_.playing(); }

double PreviewController::position_ms() const { return transport_.now_ms(); }

// The scrubber's range: the song's length the load worked out
// (app::chart_song_length_ms, D75), so the slider and its gold marks end where
// the Paths timeline does. A chart with no length falls back to
// playback_end_ms() (app::scrub_end_ms, D70 item 1).
double PreviewController::scrub_end_ms() const {
    return hydra::app::scrub_end_ms(song_length_ms_, playback_end_ms());
}

// Where playback stops: the transport's length, the later of the last note
// and the audio's end. Every reader in this class asks here.
double PreviewController::playback_end_ms() const { return transport_.length_ms(); }

void PreviewController::seek_ms(double ms) { transport_.seek_ms(ms); }

void PreviewController::jump_ms(double delta_ms) {
    if (!active_ || job_) return;  // nothing loaded yet
    transport_.seek_ms(transport_.now_ms() + delta_ms);
}

void PreviewController::step_ticks(int delta_ticks) {
    if (!active_ || job_) return;
    transport_.pause();
    transport_.seek_ms(hydra::app::step_tick_ms(scene_, transport_.now_ms(),
                                                playback_end_ms(), delta_ticks));
}

void PreviewController::set_scrubbing(bool held) {
    if (held == scrubbing_) return;
    scrubbing_ = held;
    if (held) {
        resume_after_scrub_ = transport_.playing();
        if (resume_after_scrub_) transport_.pause();
    } else if (resume_after_scrub_) {
        resume_after_scrub_ = false;
        transport_.play();
    }
}

bool PreviewController::has_audio() const { return transport_.has_audio(); }

// The volume percent lives here, not on the transport: it is a setting that
// outlives the chart, remembered for the next one opened. Its range and its
// percent-to-gain rule are the setting's (app::Settings, finding 72).
void PreviewController::set_volume(int percent) {
    volume_pct_ = app::Settings::clamp(&app::Settings::preview_volume, percent);
    transport_.set_gain(app::Settings::volume_gain(volume_pct_));
}

hydra::app::PreviewTimeBox PreviewController::time_box() const {
    return hydra::app::build_time_box(scene_, transport_.now_ms(), playback_end_ms());
}

hydra::app::PreviewScoreBox PreviewController::score_box() const {
    return hydra::app::build_score_box(scene_, transport_.now_ms(), playback_end_ms());
}

hydra::app::PreviewDrainBox PreviewController::drain_box() const {
    return hydra::app::build_drain_box(scene_, transport_.now_ms(), playback_end_ms());
}

const std::vector<double>& PreviewController::scrub_marks() const {
    const double length = scrub_end_ms();
    if (!cache_fresh(scrub_marks_cache_) || scrub_marks_cache_.length_ms != length) {
        scrub_marks_ = hydra::app::build_scrub_marks(scene_, length);
        stamp(scrub_marks_cache_);
        scrub_marks_cache_.length_ms = length;
    }
    return scrub_marks_;
}

hydra::app::PreviewNextActBox PreviewController::next_act_box() const {
    return hydra::app::build_next_act_box(scene_, transport_.now_ms(), playback_end_ms(), pro_);
}

const std::vector<hydra::app::PreviewNextActBox>& PreviewController::next_act_boxes() const {
    if (!cache_fresh(next_act_boxes_cache_)) {
        next_act_boxes_.clear();
        next_act_boxes_.reserve(scene_.activations.size());
        for (const hydra::app::PreviewActivation& a : scene_.activations)
            next_act_boxes_.push_back(
                hydra::app::build_next_act_box(scene_, a.ms, playback_end_ms(), pro_));
        stamp(next_act_boxes_cache_);
    }
    return next_act_boxes_;
}

std::string PreviewController::sp_meter_readout() const {
    return hydra::app::sp_meter_readout(scene_.sp_meter, transport_.now_ms());
}

bool PreviewController::jump_activation(int direction) {
    if (!active_ || job_) return false;  // nothing loaded yet
    const std::optional<double> to =
        hydra::app::activation_jump_ms(scene_, transport_.now_ms(), direction);
    if (!to) return false;
    transport_.seek_ms(*to);
    return true;
}

bool PreviewController::seek_activation(size_t index) {
    if (!active_ || job_ || index >= scene_.activations.size()) return false;
    transport_.seek_ms(scene_.activations[index].ms);
    return true;
}

// The renderer's json is the one source of the Preview's numbers; there is no
// default-built struct to fall back on (finding 220).
const render::PreviewConfig& PreviewController::preview_config() const {
    if (!renderer_)
        throw std::logic_error(
            "PreviewController::preview_config: no renderer yet (the first render() builds it)");
    return renderer_->config();
}

double PreviewController::sp_meter_bars() const {
    return hydra::app::sp_meter_bars_at(scene_.sp_meter, transport_.now_ms());
}

int PreviewController::sp_meter_cap() const { return scene_.sp_meter.cap; }

bool PreviewController::has_sp_gauge() const { return scene_.has_sp_gauge(); }

bool PreviewController::shows_path(const std::string& path_key) const {
    return !scene_path_key_.empty() && overlay_key_path_part(scene_path_key_) == path_key;
}

render::TrackStateOptions PreviewController::track_opts() const { return track_options(pro_); }

}  // namespace hydra::ui
