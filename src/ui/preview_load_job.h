// PreviewLoadJob: the one background job behind the details modal's 3D
// Preview tab. Read by ui/preview_controller.{h,cpp}, which owns the job,
// pulls its finished scene + audio source, and hands the source to the audio
// device — nothing else in the UI touches it.

#ifndef HYDRA_UI_PREVIEW_LOAD_JOB_H
#define HYDRA_UI_PREVIEW_LOAD_JOB_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "app/preview_source.h"
#include "app/preview_view.h"
#include "audio/stem_reader.h"
#include "audio/stream_mix.h"
#include "core/model.h"
#include "parse/song.h"
#include "render/track_state.h"
#include "store/record_store.h"
#include "ui/job_base.h"

namespace hydra::ui {

// The path a Preview scene draws over the notes: none when the chart file
// changed since its record was analyzed, because the record's path belongs to
// the old notes (finding 126). The first load and every later overlay ask here.
inline const Path* drawn_path(const std::optional<Path>& path, bool chart_changed) {
    return path && !chart_changed ? &*path : nullptr;
}

// The one place the Preview's view settings become highway options: the
// pro-drums setting that picked the drum track also picks how the pads draw
// (cymbals, or all toms). The load job, the controller and the tests build
// their options here and compare them whole (finding R7.16).
inline render::TrackStateOptions track_options(bool pro) {
    render::TrackStateOptions opts;
    opts.pro = pro;
    return opts;
}

// Turns "bytes done so far" readings into a time-left estimate for the
// loading bar. The rate is measured over at least one second of the load's
// own progress, and the answer changes at most once a second, so the text
// doesn't flicker.
class ByteRateClock {
public:
    // `now_s` is seconds since the load started. Returns the seconds left at
    // the current rate, or -1 while the rate is not known yet (under a second
    // measured, or no bytes moved in the last second).
    double update(double now_s, uint64_t bytes_done, uint64_t bytes_total);

private:
    double window_start_s_ = -1.0;
    uint64_t window_start_bytes_ = 0;
    double last_publish_s_ = -1.0;
    double published_ = -1.0;
};

// Prepares a chart for the 3D Preview off the render thread, on the same job
// base as ViewJob. Two branches run at once:
//   (a) parse the chart, build the PreviewScene, and build the highway
//       timeline (render::TrackState) from it;
//   (b) find every audio stem and open it (audio::open_song_stems). An
//       open reads the compressed file's layout, not its audio, so even an
//       8-hour stem opens in well under a second.
// Once both are done the readers are mixed by audio::mix_song_stems, the step
// the song's length uses too, into a stream that plays them straight from
// the compressed bytes. The controller pulls the finished
// scene, timeline and mix, hands the timeline to the renderer and the mix to
// an audio::Playhead. None of this runs on-frame.
class PreviewLoadJob : public ResultJobBase {
public:
    PreviewLoadJob(store::ChartLibraryEntry entry, bool pro, bool bass2x,
                   Difficulty difficulty, std::optional<Path> path, int sp_cap,
                   core::Rules rules = core::default_rules());
    ~PreviewLoadJob() { shutdown(); }

    void start();

    struct Result {
        app::PreviewScene scene;
        // Every stem that opened, mixed while it plays (audio::SongMix::mix).
        std::unique_ptr<audio::MixSource> audio;
        // Where chart time 0 sits in `audio` (audio::SongMix::audio_offset_ms).
        double audio_offset_ms = 0.0;
        // Where the audio stops in chart time (audio::SongMix::end_chart_ms).
        // The beat lines run to it, and the controller hands it to every
        // later base build. It is not the song's length.
        std::optional<double> audio_end_ms;
        // The song's length (app::chart_song_length_ms, D75), worked out from
        // the chart's metadata with no audio read. The scene's SP meter
        // closes at it, the scrubber ends at it, and the controller hands it
        // to every later base build. Empty when the owner gives none.
        std::optional<double> song_length_ms;
        // The parsed song the scene was built from. The controller keeps it so
        // a later path selection can rebuild the overlay without re-parsing.
        Song song;
        // The highway timeline, build_track_state(scene, track_opts), built
        // here on the worker so the UI thread only moves it into the
        // renderer. track_opts.pro is the pro-drums setting the job was
        // started with.
        render::TrackState track_state;
        render::TrackStateOptions track_opts;
        // The chart file's hash (app::hash_chart_file, the scan's rule) is not
        // the entry's md5: the chart changed since its record was analyzed.
        // The file is hashed only when app::chart_files_unchanged says no
        // for the entry's sig.
        // The scene was then built with no path, as for an unanalyzed chart.
        bool chart_changed = false;
    };

    // Valid once finished() && ok(); moves the result out (call once).
    Result take_result();

    // Where the load is, for the Preview tab's loading bar. The steps are
    // shown in this order, and the step shown is the first one not yet done.
    // Opening audio runs beside reading the chart; the scene and the highway
    // wait for it, because the beat lines run to the audio's end.
    enum class Step { Reading, Opening, Building, Highway };
    struct Progress {
        Step step = Step::Reading;
        // Compressed audio bytes processed so far, out of all stems' bytes.
        uint64_t bytes_done = 0;
        uint64_t bytes_total = 0;
        // Seconds since the load started.
        double elapsed_s = 0.0;
        // Seconds left at the measured byte rate (ByteRateClock); < 0 while
        // not known.
        double time_left_s = -1.0;
        // Overall 0..1. Each step owns a slice of the bar sized by its
        // measured cost; Opening audio fills its slice by bytes. Never NaN.
        float fraction() const;
        // "Reading chart", "Opening audio: 312 of 625 MB", "Building scene",
        // "Building highway".
        std::string label() const;
        // "about 0:40 left" / "about 3:05 left", or "" when the load is
        // under 3 s old, the rate isn't known, or the step isn't Opening audio.
        std::string time_left_text() const;
    };
    Progress progress() const;

private:
    void run();
    // Branch (b): find and open every stem. Throws JobCancelled when the job
    // is cancelled or `stop` is set (branch (a) failed).
    std::vector<std::unique_ptr<audio::StemReader>> open_audio(app::SharedBytes container,
                                                               const std::atomic<bool>& stop);

    store::ChartLibraryEntry entry_;
    bool pro_;
    bool bass2x_;
    Difficulty difficulty_;
    std::optional<Path> path_;
    int sp_cap_;  // the record's SP cap, used only to scale the preview's SP meter
    core::Rules rules_;  // copied: the job outlives the caller's settings
    std::optional<Result> result_;

    // Written by the workers, read by the render thread; each field is its own
    // atomic so a torn read can only be one step stale, never garbage.
    std::atomic<bool> reading_done_{false};
    std::atomic<bool> audio_done_{false};
    std::atomic<bool> scene_done_{false};
    std::atomic<bool> highway_done_{false};
    std::atomic<uint64_t> bytes_done_{0};
    std::atomic<uint64_t> bytes_total_{0};
    std::chrono::steady_clock::time_point started_{};  // set in start(), then read-only

    // The time-left clock, fed from progress() on the render thread.
    mutable std::mutex clock_mutex_;
    mutable ByteRateClock clock_;
};

// The path-free half of one chart's Preview: the scene without an overlay
// (app::build_preview_base) and the highway timeline built from it under
// `track_opts`. Built once per open chart, then shared read-only between
// scene jobs, like the song.
struct PreviewSceneBase {
    app::PreviewScene scene;
    render::TrackState track_state;
    render::TrackStateOptions track_opts;
};

// Builds a song's PreviewSceneBase. `audio_end_ms` and `song_length_ms` are
// the load's Result fields of those names (app::build_preview_base reads
// both). `check_cancel` runs between the steps and may throw to stop the
// build.
std::shared_ptr<const PreviewSceneBase> build_scene_base(
    const Song& song, render::TrackStateOptions track_opts, std::optional<double> audio_end_ms,
    std::optional<double> song_length_ms, const std::function<void()>& check_cancel);

// Builds the PreviewSceneBase off the UI thread once a chart has loaded, so
// that even the first path change only lays an overlay over it.
class PreviewBaseJob : public ResultJobBase {
public:
    PreviewBaseJob(std::shared_ptr<const Song> song, render::TrackStateOptions track_opts,
                   std::optional<double> audio_end_ms,
                   std::optional<double> song_length_ms = std::nullopt);
    ~PreviewBaseJob() { shutdown(); }

    void start();
    // Valid once finished() && ok().
    std::shared_ptr<const PreviewSceneBase> take_base() { return std::move(base_); }

private:
    void run();

    std::shared_ptr<const Song> song_;
    render::TrackStateOptions track_opts_;
    std::optional<double> audio_end_ms_;
    std::optional<double> song_length_ms_;
    std::shared_ptr<const PreviewSceneBase> base_;
};

// Rebuilds the Preview for a new path overlay off the UI thread. The song and
// the path-free base are shared with the controller, read-only, so nothing is
// re-parsed, the audio is left alone, and only the overlay is built: the
// scene's overlay (app::apply_preview_overlay) and the timeline's overlay
// fields (render::rebuild_overlay_fields) on a copy of the base timeline.
// `base` may be null, or built for other timeline options: then the job
// builds it first and hands it back in Output::base for the next job. `key`
// is the overlay key the scene is built for; `track_opts` are the timeline
// options the controller draws with (its pro-drums setting); `audio_end_ms`
// and `song_length_ms` are the load's Result fields, for a base the job has
// to build.
class PreviewSceneJob : public ResultJobBase {
public:
    PreviewSceneJob(std::shared_ptr<const Song> song,
                    std::shared_ptr<const PreviewSceneBase> base, std::optional<Path> path,
                    int sp_cap, core::Rules rules, std::string key,
                    render::TrackStateOptions track_opts, std::optional<double> audio_end_ms,
                    std::optional<double> song_length_ms = std::nullopt);
    ~PreviewSceneJob() { shutdown(); }

    void start();

    struct Output {
        app::PreviewScene scene;
        // build_track_state(scene, track_opts), built on the worker.
        render::TrackState track_state;
        render::TrackStateOptions track_opts;
        // The base this output was built over: the one the job was given, or
        // the one it had to build.
        std::shared_ptr<const PreviewSceneBase> base;
    };
    // Valid once finished() && ok(); moves the output out (call once).
    Output take_output();
    const std::string& key() const { return key_; }

private:
    void run();

    std::shared_ptr<const Song> song_;
    std::shared_ptr<const PreviewSceneBase> base_;
    std::optional<Path> path_;
    int sp_cap_;
    core::Rules rules_;
    std::string key_;
    render::TrackStateOptions track_opts_;
    std::optional<double> audio_end_ms_;
    std::optional<double> song_length_ms_;
    std::optional<Output> output_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_PREVIEW_LOAD_JOB_H
