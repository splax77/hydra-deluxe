#include "ui/preview_load_job.h"

#include <algorithm>
#include <string>

#include "app/preview_source.h"
#include "app/preview_view.h"
#include "audio/mixer.h"
#include "core/model.h"
#include "render/track_state.h"

namespace hydra::ui {

PreviewLoadJob::PreviewLoadJob(store::ChartLibraryEntry entry, bool pro, bool bass2x,
                               Difficulty difficulty, std::optional<Path> path, int sp_cap,
                               core::Rules rules)
    : entry_(std::move(entry)),
      pro_(pro),
      bass2x_(bass2x),
      difficulty_(difficulty),
      path_(std::move(path)),
      sp_cap_(sp_cap),
      rules_(std::move(rules)) {}

void PreviewLoadJob::start() { spawn([this] { run(); }); }

void PreviewLoadJob::run() {
    run_guarded([this] {
        // Re-parse the chart and locate its audio (the note stream and stems
        // are never stored), then decode + mix to one 48 kHz stereo buffer.
        // Closing the details window joins this thread on the UI thread, so
        // the job looks at its cancel flag between steps and between stems.
        step_.store(Step::Reading);
        throw_if_cancelled();
        app::PreviewSource source =
            app::resolve_preview_source(entry_.notespath, pro_, bass2x_, difficulty_, rules_);
        // A chart with no charting at this difficulty would otherwise build an
        // empty scene and the tab would show a blank highway with no reason
        // given. Throwing here surfaces it as "Preview failed: ...", the same
        // wording analysis uses.
        if (source.song.is_empty())
            throw ChartFileError(no_notes_message(difficulty_, pro_));
        throw_if_cancelled();
        step_.store(Step::Decoding);
        audio::DecodedAudio mixed = audio::decode_and_mix(
            source.stems, /*out_rate=*/48000, /*out_channels=*/2, [this](int done, int total) {
                stems_total_.store(total);
                stems_done_.store(done);
                if (done == total) step_.store(Step::Mixing);
                // Called before the first stem and after each one, outside the
                // decoder's per-stem catch, so a cancel stops the load here.
                throw_if_cancelled();
            });
        throw_if_cancelled();
        double offset_ms = source.audio_offset_ms;
        if (offset_ms < 0.0) {
            audio::pad_front_ms(mixed, -offset_ms);
            offset_ms = 0.0;
        }
        step_.store(Step::Building);
        const Path* path = path_ ? &*path_ : nullptr;
        app::PreviewScene scene = app::build_preview_scene(source.song, path, sp_cap_, rules_);
        throw_if_cancelled();
        // The highway timeline, built here so the UI thread only uploads it.
        // The pro-drums setting that picked the drum track also picks how the
        // pads draw (cymbals or all toms), as the controller would.
        step_.store(Step::Highway);
        render::TrackStateOptions track_opts;
        track_opts.pro = pro_;
        render::TrackState track_state = render::build_track_state(scene, track_opts);
        result_ = Result{std::move(scene),       std::move(mixed),       offset_ms,
                         std::move(source.song), std::move(track_state), track_opts};
        return true;
    });
}

PreviewLoadJob::Result PreviewLoadJob::take_result() { return std::move(*result_); }

PreviewLoadJob::Progress PreviewLoadJob::progress() const {
    Progress p;
    p.step = step_.load();
    p.stems_done = stems_done_.load();
    p.stems_total = stems_total_.load();
    return p;
}

float PreviewLoadJob::Progress::fraction() const {
    // Reading 0-10%, decoding 10-85%, mixing 85-95%, building the scene
    // 95-98%, building the highway timeline 98-100%.
    switch (step) {
        case Step::Reading: return 0.0f;
        case Step::Decoding: {
            float per_stem = stems_total > 0
                                 ? static_cast<float>(stems_done) / static_cast<float>(stems_total)
                                 : 0.0f;
            return 0.10f + 0.75f * per_stem;
        }
        case Step::Mixing: return 0.85f;
        case Step::Building: return 0.95f;
        case Step::Highway: return 0.98f;
    }
    return 0.0f;
}

std::string PreviewLoadJob::Progress::label() const {
    switch (step) {
        case Step::Reading: return "Reading chart";
        case Step::Decoding:
            return "Decoding audio " + std::to_string(std::min(stems_done + 1, stems_total)) +
                   "/" + std::to_string(stems_total);
        case Step::Mixing: return "Mixing audio";
        case Step::Building: return "Building scene";
        case Step::Highway: return "Building highway";
    }
    return "";
}

std::shared_ptr<const PreviewSceneBase> build_scene_base(
    const Song& song, render::TrackStateOptions track_opts,
    const std::function<void()>& check_cancel) {
    auto built = std::make_shared<PreviewSceneBase>();
    built->scene = app::build_preview_base(song);
    check_cancel();
    built->track_state = render::build_track_state(built->scene, track_opts);
    built->track_opts = track_opts;
    check_cancel();
    return built;
}

PreviewBaseJob::PreviewBaseJob(std::shared_ptr<const Song> song,
                               render::TrackStateOptions track_opts)
    : song_(std::move(song)), track_opts_(track_opts) {}

void PreviewBaseJob::start() { spawn([this] { run(); }); }

void PreviewBaseJob::run() {
    run_guarded([this] {
        throw_if_cancelled();
        base_ = build_scene_base(*song_, track_opts_, [this] { throw_if_cancelled(); });
        return true;
    });
}

PreviewSceneJob::PreviewSceneJob(std::shared_ptr<const Song> song,
                                 std::shared_ptr<const PreviewSceneBase> base,
                                 std::optional<Path> path, int sp_cap, core::Rules rules,
                                 std::string key, render::TrackStateOptions track_opts)
    : song_(std::move(song)),
      base_(std::move(base)),
      path_(std::move(path)),
      sp_cap_(sp_cap),
      rules_(std::move(rules)),
      key_(std::move(key)),
      track_opts_(track_opts) {}

void PreviewSceneJob::start() { spawn([this] { run(); }); }

void PreviewSceneJob::run() {
    run_guarded([this] {
        throw_if_cancelled();  // a newer selection already replaced this one
        // The path-free half, once per chart: reuse the controller's, or
        // build it here when there is none yet (or it was drawn with other
        // timeline options).
        std::shared_ptr<const PreviewSceneBase> base = base_;
        if (!base || base->track_opts.pro != track_opts_.pro)
            base = build_scene_base(*song_, track_opts_, [this] { throw_if_cancelled(); });
        // Only the overlay is built per path: the scene's, then the
        // timeline's on a copy of the base timeline, so the swap on the UI
        // thread is only a move.
        app::PreviewScene scene = app::apply_preview_overlay(
            base->scene, *song_, path_ ? &*path_ : nullptr, sp_cap_, rules_);
        throw_if_cancelled();
        render::TrackState track_state = base->track_state;
        render::rebuild_overlay_fields(track_state, scene);
        output_ = Output{std::move(scene), std::move(track_state), track_opts_, std::move(base)};
        return true;
    });
}

PreviewSceneJob::Output PreviewSceneJob::take_output() { return std::move(*output_); }

}  // namespace hydra::ui
