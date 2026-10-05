// PreviewController — owns the live 3D Preview for one open chart.
//
// The details modal's Preview tab drives this. It ties together an async
// PreviewLoadJob (re-parse + decode + build scene), the offscreen
// PreviewRenderer (the Onyx port, docs/adr/0008), a PreviewTransport over the
// mixed audio, and a PreviewAudioDevice that plays it. The Transport owns the
// play/pause/seek rules and the master display clock, as in Onyx: the highway
// is drawn at transport_.tick() each frame, and on play the audio is seeked to
// that time and then follows. A chart with no audio still plays and scrubs.
//
// One controller lives on AppState, created on first use with the GUI's shared
// D3D11 device. It is opened for the selected chart, torn down when the modal
// or selection changes, and only renders + decodes while the Preview tab shows
// — so a library browse never spins up a GPU scene or an audio device.

#ifndef HYDRA_UI_PREVIEW_CONTROLLER_H
#define HYDRA_UI_PREVIEW_CONTROLLER_H

#include <memory>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "app/preview_view.h"
#include "core/model.h"  // Path
#include "parse/song.h"
#include "render/preview_renderer.h"
#include "store/record_store.h"  // ChartLibraryEntry
#include "ui/preview_transport.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

namespace hydra::audio {
class PreviewAudioDevice;
}  // namespace hydra::audio

namespace hydra::ui {

class PreviewLoadJob;
class PreviewSceneJob;
class PreviewBaseJob;
struct PreviewSceneBase;

// What the Preview calls the same song: one chart (its md5) at one
// difficulty, with Pro Drums and 2x Bass on or off. These are the inputs
// Settings::to_analysis_settings reads to pick the notes, so another
// difficulty, Pro Drums or 2x Bass is another song and reloads its notes
// (D48, Q22).
struct PreviewSongKey {
    std::string md5;
    Difficulty difficulty = Difficulty::Expert;
    bool pro = false;
    bool bass2x = false;

    bool operator==(const PreviewSongKey& o) const {
        return md5 == o.md5 && difficulty == o.difficulty && pro == o.pro &&
               bass2x == o.bass2x;
    }
    bool operator!=(const PreviewSongKey& o) const { return !(*this == o); }
};

class PreviewController {
public:
    PreviewController(ID3D11Device* device, ID3D11DeviceContext* context);
    ~PreviewController();

    PreviewController(const PreviewController&) = delete;
    PreviewController& operator=(const PreviewController&) = delete;

    // (Re)start the preview for `entry`. Called every frame the Preview tab is
    // shown. `path` (may be null) supplies the path overlay; it is copied, so
    // the caller's Path need not outlive the call. `path_key` is
    // app::path_overlay_key(path), which the caller builds once per selection
    // (it is too heavy to build per frame). Already open for the same song
    // (PreviewSongKey: chart, difficulty, Pro Drums and 2x Bass), path key and
    // SP cap: a no-op. Another song is a fresh load. Same song, different
    // path or cap: the new
    // overlay is built on a background job off the retained song and swapped
    // in by a later poll() — no re-parse, no audio re-decode, playback
    // position untouched; the old overlay stays up until then.
    void open(const store::ChartLibraryEntry& entry, bool pro, bool bass2x,
              Difficulty difficulty, const Path* path, const std::string& path_key,
              int sp_cap, const core::Rules& rules = core::default_rules());
    // Stop audio, drop the scene/transport, and join the load thread. Keeps the
    // renderer for reuse. Safe to call when nothing is open.
    void close();

    bool active() const { return active_; }
    // The drawn overlay's identity: the path it was built from
    // (app::path_overlay_key) plus the SP cap its meter was scaled to, in the
    // one string open() compares. Empty until something has loaded. Treat the
    // exact spelling as opaque: a test may compare two of these whole, but
    // "is this my path" is shows_path's to answer.
    const std::string& overlay_path_key() const { return scene_path_key_; }
    // The drawn overlay was built from `path_key` (app::path_overlay_key),
    // whatever SP cap its meter was scaled to. False until something has
    // loaded.
    bool shows_path(const std::string& path_key) const;

    // Advance the async load; once finished, build the transport + audio device.
    // Call once per frame while the Preview tab is shown.
    void poll();

    // Bars of SP banked at the transport's current time, and the meter's
    // ceiling — what the panel's gauge draws.
    double sp_meter_bars() const;
    int sp_meter_cap() const;
    // The scene has a gauge to draw (app::PreviewScene::has_sp_gauge, which
    // the drain box asks too).
    bool has_sp_gauge() const;

    // The first load is still running: what the panel's progress bar means.
    bool loading() const { return job_ != nullptr; }
    // Any Preview job is still running or waiting for poll() to take it in:
    // the first load, the base build (or one poll() is about to start), the
    // overlay build, or a replaced overlay build still finishing. False
    // means every Preview thread is done (finding 109).
    bool busy() const;
    // Only meaningful while loading(); the load's current step and how far
    // through the audio it is.
    struct LoadProgress {
        float fraction = 0.0f;  // 0..1 estimate
        std::string label;      // "Opening audio: 312 of 625 MB"
        std::string detail;     // time left ("about 0:40 left"), or ""
    };
    LoadProgress load_progress() const;
    bool has_error() const { return !error_.empty(); }
    const std::string& error() const { return error_; }

    // Set when the audio output device would not open. Not an error: the
    // chart still loads, draws and plays on the clock, just muted. The panel
    // shows one warning line and keeps drawing.
    bool has_audio_warning() const { return !audio_warning_.empty(); }
    const std::string& audio_warning() const { return audio_warning_; }

    // The chart file is not the one its record was analyzed from: its hash
    // (app::hash_chart_file, the scan's rule) differs from the entry's md5.
    // The Preview then draws no path, overlay or score, as for an unanalyzed
    // chart, and the panel shows one warning line (D51 call 18). Cleared by
    // close().
    bool chart_changed() const { return chart_changed_; }

    // What opens the output device. Empty (the default) opens the real one; a
    // test installs a factory that throws, standing in for a PC with no audio
    // device.
    using AudioSource = std::function<int64_t(float* out, int64_t frames)>;
    using AudioDeviceFactory = std::function<std::unique_ptr<hydra::audio::PreviewAudioDevice>(
        int channels, int sample_rate, AudioSource source)>;
    void set_audio_device_factory(AudioDeviceFactory factory) {
        device_factory_ = std::move(factory);
    }

    // Draw the highway at the current playhead into a width x height offscreen
    // target; returns its shader-resource view for ImGui::Image (null until the
    // first successful render). Resizes the target when the region changes.
    ID3D11ShaderResourceView* render(int width, int height);

    // Transport controls, driven by the display clock; audio (when any)
    // follows. Safe before the chart has loaded (nothing to move yet).
    void play();
    void pause();
    void toggle();
    bool playing() const;
    double position_ms() const;
    // Where the scrubber ends: the last note (app::scrub_end_ms, D50 item 4),
    // or playback_end_ms() for a chart with no notes. Not where playback
    // stops.
    double scrub_end_ms() const;
    // Where playback stops: the later of the last note and the audio's end
    // (PreviewTransport::length_ms). Play, the clock and jumps run to here.
    double playback_end_ms() const;
    void seek_ms(double ms);
    // Move the playhead by `delta_ms` (the -5s/+5s buttons, Left/Right).
    // Playing stays playing; the transport stops it at 0 and at
    // playback_end_ms().
    void jump_ms(double delta_ms);
    // Pause, then move the playhead `delta_ticks` chart ticks from the tick
    // the time box shows (the < 5 Ticks / 5 Ticks > buttons, comma and period).
    // A step of 0 snaps onto the displayed tick.
    void step_ticks(int delta_ticks);
    bool has_audio() const;

    // The scrubber is being held (mouse down on it). As in Onyx, playback
    // pauses for the hold and resumes on release if it was playing; the
    // highway still follows the seeks. Call every frame with the slider's
    // active state. Without this the clock kept running under the held
    // slider and the audio was re-seeked to the held time every frame —
    // heard as a buzz.
    void set_scrubbing(bool held);

    // Playback volume in percent, clamped to the preview_volume setting's
    // range (app::Settings::clamp); applied to the audio as it is served, and
    // remembered for the next chart opened. Starts at the setting's default.
    void set_volume(int percent);
    int volume_percent() const { return volume_pct_; }

    // The time box the panel draws over the highway (Onyx's top-left text).
    hydra::app::PreviewTimeBox time_box() const;

    // The score box the panel draws under the time box.
    hydra::app::PreviewScoreBox score_box() const;

    // The SP drain box the panel draws beside the gauge.
    hydra::app::PreviewDrainBox drain_box() const;

    // The drawn path's activations on the scrubber, as fractions of
    // scrub_end_ms() (app::build_scrub_marks). Empty until a path's scene is in.
    // Built once per scene and length, then cached (see SceneCache below);
    // the reference holds until the next call.
    const std::vector<double>& scrub_marks() const;

    // The box at the highway's bottom-left (app::build_next_act_box).
    hydra::app::PreviewNextActBox next_act_box() const;

    // Every box that one shows over the drawn path, one per activation: what
    // it reads with the playhead on each activation in turn. The panel fits
    // the text scale to all of them at once, so it holds still during play.
    // Built once per scene, then cached, like scrub_marks().
    const std::vector<hydra::app::PreviewNextActBox>& next_act_boxes() const;

    // The number under the SP gauge, "2.5/4" (app::sp_meter_readout).
    std::string sp_meter_readout() const;

    // Move the playhead to the previous (-1) or next (+1) activation of the
    // drawn path (app::activation_jump_ms). Playing stays playing, as with
    // jump_ms. False when there is none that way or nothing is loaded.
    bool jump_activation(int direction);

    // Move the playhead to activation `index` (0-based) of the drawn path,
    // for "Show in Preview". False when nothing is loaded or the drawn path
    // has no such activation.
    bool seek_activation(size_t index);

    // The text overlays' scale the panel last drew at (1 = full size). The
    // panel sets it each frame; only the GUI test reads it back.
    void set_overlay_scale(float scale) { overlay_scale_ = scale; }
    float overlay_scale() const { return overlay_scale_; }

    // The Preview's look, as read from 3d-config.json by the renderer. There
    // is no second source: only the first render() builds the renderer, and
    // asking before that throws std::logic_error.
    const render::PreviewConfig& preview_config() const;

private:
    // What the panel reads every frame but only changes with the scene:
    // the scrub marks (which also move with the song length) and the
    // next-activation boxes. Every change to scene_ sets scene_dirty_, and
    // render() is the one place that clears it, counting a new scene
    // generation as it does. So a cache is good only if it was built while
    // the flag was clear and in the current generation. One built while the
    // flag is set is never trusted, which keeps it right even before the
    // first render() (or in a test that never renders): it is rebuilt on
    // every call until render() takes the scene in.
    struct SceneCache {
        bool trusted = false;
        uint64_t generation = 0;
        double length_ms = 0.0;  // scrub marks only
    };
    bool cache_fresh(const SceneCache& c) const {
        return c.trusted && !scene_dirty_ && c.generation == scene_generation_;
    }
    void stamp(SceneCache& c) const {
        c.trusted = !scene_dirty_;
        c.generation = scene_generation_;
    }
    uint64_t scene_generation_ = 0;  // bumped by render() as it takes in a new scene_
    mutable SceneCache scrub_marks_cache_;
    mutable std::vector<double> scrub_marks_;
    mutable SceneCache next_act_boxes_cache_;
    mutable std::vector<hydra::app::PreviewNextActBox> next_act_boxes_;

    float overlay_scale_ = 1.0f;
    ID3D11Device* device_;
    ID3D11DeviceContext* context_;

    std::unique_ptr<render::PreviewRenderer> renderer_;  // created on first render
    int rt_w_ = 0;
    int rt_h_ = 0;
    bool have_frame_ = false;

    std::unique_ptr<PreviewLoadJob> job_;
    hydra::app::PreviewScene scene_;
    bool scene_dirty_ = true;  // scene_ changed since the renderer last saw it
    bool pro_ = true;          // the pro-drums view setting the chart was opened with
    // The highway options drawn with: track_options(pro_).
    render::TrackStateOptions track_opts() const;
    // The highway timeline a job built from scene_ on its worker, waiting for
    // render() to move it into the renderer, plus the options it was built
    // with. Set together with scene_ whenever a job's scene lands; empty
    // otherwise (then render() builds the timeline itself, as for the empty
    // scene after close()). Dropped after the upload.
    std::optional<render::TrackState> pending_track_;
    render::TrackStateOptions pending_track_opts_;
    int sp_cap_ = kCloneHeroSpCap;  // the SP meter's ceiling the scene was built with
    // The rules the running score is priced under: the user's
    // hydra_rules.ini, as the panel passes it to every open().
    core::Rules rules_ = core::default_rules();

    // The path overlay, kept swappable without touching the audio: the parsed
    // song the load produced, the overlay the panel last asked for, and the one
    // scene_ was actually built with. Each key is the path's overlay key plus
    // the SP cap (see overlay_key() in the .cpp), so a cap change rebuilds the
    // scene the same way a path change does. The two keys differ only while a
    // load is in flight and the Paths tab (or the cap) changed the selection
    // under it; poll() closes the gap.
    std::shared_ptr<const Song> song_;  // shared read-only with scene jobs
    // Where the song's audio stops in chart time (the load's
    // Result::audio_end_ms), kept with the song so every base built for it
    // runs the beat lines to the same end. Empty without audio.
    std::optional<double> audio_end_ms_;
    // The song's path-free scene and timeline, shared read-only with scene
    // jobs so a path change builds only the overlay. poll() starts base_job_
    // to build it once the load has landed (a scene job that runs first
    // builds its own and hands it back); dropped with the song.
    std::shared_ptr<const PreviewSceneBase> scene_base_;
    std::unique_ptr<PreviewBaseJob> base_job_;
    bool base_started_ = false;  // base_job_ ran once for this chart
    // The next poll() starts base_job_: the song is here, nothing else is
    // building a base, and none was built or started yet.
    bool base_due() const;
    std::optional<Path> path_;
    std::string requested_path_key_;  // the path half of path_key_, as open() got it
    std::string path_key_;        // key of path_ + sp_cap_
    // The overlay being built for a new selection, and replaced ones still
    // finishing (dropped by poll() once done, so replacing one never joins
    // its thread on the UI thread).
    std::unique_ptr<PreviewSceneJob> scene_job_;
    std::vector<std::unique_ptr<PreviewSceneJob>> retired_scene_jobs_;
    void start_scene_job();
    std::string job_path_key_;    // key the in-flight job was started with
    std::string scene_path_key_;  // key scene_'s overlay was built from

    int volume_pct_;  // starts at app::Settings' preview_volume default
    bool chart_changed_ = false;  // see chart_changed()
    bool scrubbing_ = false;
    bool resume_after_scrub_ = false;

    // Play, pause, seek, the master clock and the audio that follows it.
    // Declared before audio_device_ so the device (whose callback pulls from
    // the transport) is destroyed first.
    PreviewTransport transport_;
    std::unique_ptr<hydra::audio::PreviewAudioDevice> audio_device_;

    bool active_ = false;
    PreviewSongKey open_key_;  // the song open() last loaded
    std::string error_;
    std::string audio_warning_;
    AudioDeviceFactory device_factory_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_PREVIEW_CONTROLLER_H
