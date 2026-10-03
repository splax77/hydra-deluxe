#include "ui/preview_controller.h"

#include <cstdint>
#include <algorithm>
#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "app/config.h"
#include "audio/device.h"
#include "audio/player.h"
#include "ui/preview_load_job.h"

namespace hydra::ui {

namespace {

// The overlay's identity: the path it was built from plus the SP cap the
// meter was scaled to. A cap change must rebuild the scene just like a
// path change, so both live in the one key the three key_ members compare.
std::string overlay_key(const std::string& path_key, int sp_cap) {
    return path_key + "|cap" + std::to_string(sp_cap);
}

}  // namespace

PreviewController::PreviewController(ID3D11Device* device, ID3D11DeviceContext* context)
    : device_(device), context_(context) {}

PreviewController::~PreviewController() { close(); }

void PreviewController::open(const store::ChartLibraryEntry& entry, bool pro,
                             bool bass2x, Difficulty difficulty,
                             const Path* path, const std::string& path_key,
                             int sp_cap, const core::Rules& rules) {
    rules_ = rules;
    if (active_ && open_key_ == entry.md5) {
        // Same chart, same overlay: nothing to do, and nothing built.
        if (sp_cap == sp_cap_ && path_key == requested_path_key_) return;
        path_ = path ? std::optional<Path>(*path) : std::nullopt;
        sp_cap_ = sp_cap;
        requested_path_key_ = path_key;
        path_key_ = overlay_key(path_key, sp_cap);
        // Mid-load the load job is building its own scene; poll() reconciles.
        // Once the song is here, the new overlay builds on a job and poll()
        // swaps it in, which leaves the audio and the playhead alone.
        if (!job_ && song_ && !song_->is_empty()) start_scene_job();
        return;
    }
    close();

    open_key_ = entry.md5;
    active_ = true;
    error_.clear();
    pro_ = pro;
    sp_cap_ = sp_cap;

    path_ = path ? std::optional<Path>(*path) : std::nullopt;
    requested_path_key_ = path_key;
    path_key_ = overlay_key(path_key, sp_cap_);
    job_path_key_ = path_key_;
    job_ = std::make_unique<PreviewLoadJob>(entry, pro, bass2x, difficulty, path_, sp_cap_,
                                            rules);
    job_->start();
}

void PreviewController::start_scene_job() {
    if (scene_job_) {
        scene_job_->cancel();
        retired_scene_jobs_.push_back(std::move(scene_job_));
    }
    render::TrackStateOptions track_opts;
    track_opts.pro = pro_;
    scene_job_ = std::make_unique<PreviewSceneJob>(song_, scene_base_, path_, sp_cap_, rules_,
                                                   path_key_, track_opts);
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
    scene_job_.reset();
    base_job_.reset();
    base_started_ = false;
    retired_scene_jobs_.clear();
    requested_path_key_.clear();
    scene_ = hydra::app::PreviewScene{};
    scene_dirty_ = true;  // the renderer (if kept) must drop the old chart
    pending_track_.reset();  // scene_ is empty now; render() builds its (empty) timeline
    song_.reset();
    scene_base_.reset();
    path_.reset();
    sp_cap_ = kCloneHeroSpCap;
    path_key_.clear();
    job_path_key_.clear();
    scene_path_key_.clear();
    have_frame_ = false;
    active_ = false;
    scrubbing_ = false;
    resume_after_scrub_ = false;
    open_key_.clear();
    error_.clear();
    audio_warning_.clear();
}

void PreviewController::poll() {
    // Replaced overlay builds that have finished can go.
    retired_scene_jobs_.erase(
        std::remove_if(retired_scene_jobs_.begin(), retired_scene_jobs_.end(),
                       [](const std::unique_ptr<PreviewSceneJob>& j) { return j->finished(); }),
        retired_scene_jobs_.end());
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
            error_ = scene_job_->error();
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
    if (!base_started_ && !job_ && !scene_job_ && !scene_base_ && song_ && !song_->is_empty()) {
        render::TrackStateOptions track_opts;
        track_opts.pro = pro_;
        base_job_ = std::make_unique<PreviewBaseJob>(song_, track_opts);
        base_job_->start();
        base_started_ = true;
    }

    if (!job_ || !job_->finished()) return;

    const bool ok = job_->ok();
    if (ok) {
        PreviewLoadJob::Result result = job_->take_result();
        song_ = std::make_shared<const Song>(std::move(result.song));
        scene_ = std::move(result.scene);
        pending_track_ = std::move(result.track_state);
        pending_track_opts_ = result.track_opts;
        scene_path_key_ = job_path_key_;
        scene_dirty_ = true;
        transport_.set_gain(static_cast<float>(volume_pct_) / 100.0f);
        transport_.load(std::make_unique<audio::Playhead>(std::move(result.audio)),
                        scene_.song_length_ms, result.audio_offset_ms);
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
        error_ = job_->error();
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
            render::TrackStateOptions opts;
            opts.pro = pro_;
            // A job built scene_'s timeline on its worker: just move it in.
            // If it was built for another pro-drums setting than the one
            // drawn now, or there is none (the empty scene after close()),
            // build it here as before.
            if (pending_track_ && pending_track_opts_.pro == opts.pro)
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
        if (error_.empty()) error_ = e.what();
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

double PreviewController::length_ms() const { return transport_.length_ms(); }

void PreviewController::seek_ms(double ms) { transport_.seek_ms(ms); }

void PreviewController::jump_ms(double delta_ms) {
    if (!active_ || job_) return;  // nothing loaded yet
    transport_.seek_ms(transport_.now_ms() + delta_ms);
}

void PreviewController::step_ticks(int delta_ticks) {
    if (!active_ || job_) return;
    transport_.pause();
    transport_.seek_ms(hydra::app::step_tick_ms(scene_, transport_.now_ms(),
                                                transport_.length_ms(), delta_ticks));
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
// outlives the chart, remembered for the next one opened.
void PreviewController::set_volume(int percent) {
    volume_pct_ = percent < 0 ? 0 : percent > 100 ? 100 : percent;
    transport_.set_gain(static_cast<float>(volume_pct_) / 100.0f);
}

hydra::app::PreviewTimeBox PreviewController::time_box() const {
    return hydra::app::build_time_box(scene_, transport_.now_ms(),
                                      transport_.length_ms());
}

hydra::app::PreviewScoreBox PreviewController::score_box() const {
    return hydra::app::build_score_box(scene_, transport_.now_ms());
}

hydra::app::PreviewDrainBox PreviewController::drain_box() const {
    return hydra::app::build_drain_box(scene_, transport_.now_ms());
}

const std::vector<double>& PreviewController::scrub_marks() const {
    const double length = transport_.length_ms();
    if (!cache_fresh(scrub_marks_cache_) || scrub_marks_cache_.length_ms != length) {
        scrub_marks_ = hydra::app::build_scrub_marks(scene_, length);
        stamp(scrub_marks_cache_);
        scrub_marks_cache_.length_ms = length;
    }
    return scrub_marks_;
}

hydra::app::PreviewNextActBox PreviewController::next_act_box() const {
    return hydra::app::build_next_act_box(scene_, transport_.now_ms());
}

const std::vector<hydra::app::PreviewNextActBox>& PreviewController::next_act_boxes() const {
    if (!cache_fresh(next_act_boxes_cache_)) {
        next_act_boxes_.clear();
        next_act_boxes_.reserve(scene_.activations.size());
        for (const hydra::app::PreviewActivation& a : scene_.activations)
            next_act_boxes_.push_back(hydra::app::build_next_act_box(scene_, a.ms));
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

const render::PreviewConfig& PreviewController::preview_config() const {
    static const render::PreviewConfig kOnyxDefaults;
    return renderer_ ? renderer_->config() : kOnyxDefaults;
}

double PreviewController::sp_meter_bars() const {
    return hydra::app::sp_meter_bars_at(scene_.sp_meter, transport_.now_ms());
}

int PreviewController::sp_meter_cap() const { return scene_.sp_meter.cap; }

bool PreviewController::sp_meter_has_curve() const {
    return !scene_.sp_meter.segments.empty();
}

}  // namespace hydra::ui
