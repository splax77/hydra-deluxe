// PreviewLoadJob: the one background job behind the details modal's 3D
// Preview tab. Read by ui/preview_controller.{h,cpp}, which owns the job,
// pulls its finished scene + audio source, and hands the source to the audio
// device — nothing else in the UI touches it.

#ifndef HYDRA_UI_PREVIEW_LOAD_JOB_H
#define HYDRA_UI_PREVIEW_LOAD_JOB_H

#include <atomic>
#include <chrono>
#include <cstdint>
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

// Prepares a chart for the 3D Preview off the render thread, mirroring
// AnalyzeJob. Two branches run at once:
//   (a) parse the chart, build the PreviewScene, and build the highway
//       timeline (render::TrackState) from it;
//   (b) find every audio stem and open it with audio::open_stem_reader. An
//       open reads the compressed file's layout, not its audio, so even an
//       8-hour stem opens in well under a second.
// Once both are done the readers go into one audio::StreamMix, which plays
// them straight from the compressed bytes. The controller pulls the finished
// scene, timeline and mix, hands the timeline to the renderer and the mix to
// an audio::Playhead. None of this runs on-frame.
class PreviewLoadJob : public ResultJobBase {
public:
    PreviewLoadJob(store::ChartLibraryEntry entry, bool pro, bool bass2x,
                   Difficulty difficulty, std::optional<Path> path, int sp_cap,
                   core::Rules rules = core::default_rules());
    ~PreviewLoadJob() { shutdown(); }

    void start();

    // The output format of the mix: what the audio device plays.
    static constexpr int kOutRate = 48000;
    static constexpr int kOutChannels = 2;

    struct Result {
        app::PreviewScene scene;
        // Every stem that opened, mixed while it plays. A negative chart
        // offset is silence in front of the stems (StreamMix's front pad).
        std::unique_ptr<audio::MixSource> audio;
        // Where chart time 0 sits in `audio` (never negative: a negative
        // chart offset is padded into the front of `audio` instead).
        double audio_offset_ms = 0.0;
        // The parsed song the scene was built from. The controller keeps it so
        // a later path selection can rebuild the overlay without re-parsing.
        Song song;
        // The highway timeline, build_track_state(scene, track_opts), built
        // here on the worker so the UI thread only moves it into the
        // renderer. track_opts.pro is the pro-drums setting the job was
        // started with.
        render::TrackState track_state;
        render::TrackStateOptions track_opts;
    };

    // Valid once finished() && ok(); moves the result out (call once).
    Result take_result();

    // Where the load is, for the Preview tab's loading bar. The steps are
    // shown in this order. Opening audio runs beside the other three, so the
    // step shown is the first one not yet done.
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
        // "about 40 s left" / "about 3 min left", or "" when the load is
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

// Rebuilds the Preview scene for a new path overlay off the UI thread, and the
// highway timeline from it. The song is shared with the controller, read-only,
// so nothing is re-parsed and the audio is left alone. `key` is the overlay
// key the scene is built for; `track_opts` are the timeline options the
// controller draws with (its pro-drums setting).
class PreviewSceneJob : public ResultJobBase {
public:
    PreviewSceneJob(std::shared_ptr<const Song> song, std::optional<Path> path, int sp_cap,
                    core::Rules rules, std::string key,
                    render::TrackStateOptions track_opts);
    ~PreviewSceneJob() { shutdown(); }

    void start();

    struct Output {
        app::PreviewScene scene;
        // build_track_state(scene, track_opts), built on the worker.
        render::TrackState track_state;
        render::TrackStateOptions track_opts;
    };
    // Valid once finished() && ok(); moves the output out (call once).
    Output take_output();
    const std::string& key() const { return key_; }

private:
    void run();

    std::shared_ptr<const Song> song_;
    std::optional<Path> path_;
    int sp_cap_;
    core::Rules rules_;
    std::string key_;
    render::TrackStateOptions track_opts_;
    std::optional<Output> output_;
};

}  // namespace hydra::ui

#endif  // HYDRA_UI_PREVIEW_LOAD_JOB_H
