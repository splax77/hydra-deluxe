// PreviewLoadJob: the one background job behind the details modal's 3D
// Preview tab. Read by ui/preview_controller.{h,cpp}, which owns the job,
// pulls its finished scene + mixed audio, and hands the buffer to the audio
// device — nothing else in the UI touches it.

#ifndef HYDRA_UI_PREVIEW_LOAD_JOB_H
#define HYDRA_UI_PREVIEW_LOAD_JOB_H

#include <atomic>
#include <memory>
#include <optional>
#include <string>

#include "app/preview_view.h"
#include "audio/decode.h"
#include "core/model.h"
#include "parse/song.h"
#include "render/track_state.h"
#include "store/record_store.h"
#include "ui/job_base.h"

namespace hydra::ui {

// Prepares a chart for the 3D Preview off the render thread, mirroring
// AnalyzeJob: re-parse the notes and gather audio (resolve_preview_source),
// decode and mix every stem to one buffer, build the PreviewScene, and build
// the highway timeline (render::TrackState) from it. The controller pulls the
// finished scene, timeline and mixed audio, hands the timeline to the
// renderer and the buffer to an audio::Playhead. Parse, decode and the
// timeline are heavy, so none of it runs on-frame.
class PreviewLoadJob : public ResultJobBase {
public:
    PreviewLoadJob(store::ChartLibraryEntry entry, bool pro, bool bass2x,
                   Difficulty difficulty, std::optional<Path> path, int sp_cap,
                   core::Rules rules = core::default_rules());
    ~PreviewLoadJob() { shutdown(); }

    void start();

    struct Result {
        app::PreviewScene scene;
        audio::DecodedAudio mixed;
        // Where chart time 0 sits in `mixed` (never negative: a negative
        // chart offset is padded into the front of `mixed` instead).
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

    // Where the load is, for the Preview tab's loading bar. The steps run in
    // this order; Decoding is the long one on big charts, so it is the only
    // step that reports a count (stems done / stems total). Building is the
    // scene, Highway the timeline built from it.
    enum class Step { Reading, Decoding, Mixing, Building, Highway };
    struct Progress {
        Step step = Step::Reading;
        int stems_done = 0;
        int stems_total = 0;
        // Overall 0..1 estimate: the three fixed steps get a slice each and
        // decoding spreads its slice across the stems.
        float fraction() const;
        // Short label for the current step ("Decoding audio 2/5").
        std::string label() const;
    };
    Progress progress() const;

private:
    void run();

    store::ChartLibraryEntry entry_;
    bool pro_;
    bool bass2x_;
    Difficulty difficulty_;
    std::optional<Path> path_;
    int sp_cap_;  // the record's SP cap, used only to scale the preview's SP meter
    core::Rules rules_;  // copied: the job outlives the caller's settings
    std::optional<Result> result_;

    // Written by the worker, read by the render thread; each field is its own
    // atomic so a torn read can only be one step stale, never garbage.
    std::atomic<Step> step_{Step::Reading};
    std::atomic<int> stems_done_{0};
    std::atomic<int> stems_total_{0};
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
