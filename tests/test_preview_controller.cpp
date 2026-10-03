// Tests for the details window's two background loads and the Preview
// controller's no-audio fallback. The loads are real threads on real corpus
// charts; nothing here opens a window or a real audio device.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/preview_view.h"
#include "audio/device.h"
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "render/track_state.h"
#include "store/record_store.h"
#include "ui/dynamics_load_job.h"
#include "ui/preview_controller.h"
#include "ui/preview_load_job.h"

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using hydra::Difficulty;
using hydra::store::ChartLibraryEntry;
using hydra::ui::DynamicsLoadJob;
using hydra::ui::PreviewController;
using hydra::ui::PreviewLoadJob;

namespace {

// Wait up to 60 s for a job's worker to finish.
template <class Job>
void wait_finished(const Job& job) {
    for (int i = 0; i < 1200 && !job.finished(); ++i) Sleep(50);
    REQUIRE(job.finished());
}

ChartLibraryEntry entry_for(const std::string& notespath) {
    ChartLibraryEntry e;
    e.md5 = "prevctl";
    e.title = "Preview controller test";
    e.notespath = notespath;
    return e;
}

void copy_file_utf8(const std::string& from, const std::string& to) {
    std::vector<uint8_t> bytes = hydra::read_file_bytes(from);
    std::FILE* f = hydra::fopen_utf8(to, L"wb");
    REQUIRE(f != nullptr);
    if (!bytes.empty()) std::fwrite(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
}

// A chart folder that has audio: a corpus .chart plus the test sine as
// song.ogg. The GUI test library has no audio at all, so the no-device path
// can only be reached here.
std::string chart_with_audio() {
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);
    std::wstring dir = std::wstring(tmp) + L"hydra_prevctl_" +
                       std::to_wstring(GetCurrentProcessId());
    CreateDirectoryW(dir.c_str(), nullptr);
    const std::string d = hydra::wide_to_utf8(dir);
    copy_file_utf8(corpus::first_chart_with_suffix(".chart"), d + "\\notes.chart");
    copy_file_utf8(std::string(HYDRA_TESTDATA_DIR) + "/audio/sine220.ogg", d + "\\song.ogg");
    return d + "\\notes.chart";
}

// Every field of two timelines, instant by instant.
void check_same_track(const hydra::render::TrackState& got,
                      const hydra::render::TrackState& want) {
    const auto& g = got.instants();
    const auto& w = want.instants();
    REQUIRE(g.size() == w.size());
    for (size_t i = 0; i < g.size(); ++i) {
        CAPTURE(i);
        CHECK(g[i].t == w[i].t);
        REQUIRE(g[i].notes.size() == w[i].notes.size());
        for (size_t n = 0; n < g[i].notes.size(); ++n) {
            CHECK(g[i].notes[n].kick == w[i].notes[n].kick);
            CHECK(g[i].notes[n].pad == w[i].notes[n].pad);
            CHECK(g[i].notes[n].cymbal == w[i].notes[n].cymbal);
            CHECK(g[i].notes[n].velocity == w[i].notes[n].velocity);
        }
        CHECK(g[i].overdrive == w[i].overdrive);
        CHECK(g[i].solo == w[i].solo);
        CHECK(g[i].fill == w[i].fill);
        CHECK(g[i].fill_taken == w[i].fill_taken);
        CHECK(g[i].sp_active == w[i].sp_active);
        CHECK(g[i].fill_lane == w[i].fill_lane);
        CHECK(g[i].fill_lane_pad == w[i].fill_lane_pad);
        CHECK(g[i].beat == w[i].beat);
    }
}

// A corpus chart Hydra finds at least one path on, and its best path.
struct AnalyzedChart {
    std::string chart;
    hydra::Path best;
};
AnalyzedChart first_chart_with_a_path() {
    using namespace hydra;
    using namespace hydra::app;
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 2;
    settings.ms_filter = 10.0;
    for (const std::string& p : corpus::chart_paths()) {
        try {
            AnalysisResult r = analyze_chart_file(p, settings);
            if (!r.record.paths.empty()) return {p, r.record.best_path()};
        } catch (const std::exception&) {
        }
    }
    FAIL("no corpus chart has a path");
    return {};
}

}  // namespace

// The load job builds the highway timeline on its worker so the UI thread
// only moves it into the renderer. It must be exactly the timeline the
// renderer would have built from the job's scene, for the pro-drums setting
// the job was started with.
TEST_CASE("the Preview load builds the highway timeline on its worker") {
    const AnalyzedChart a = first_chart_with_a_path();
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        PreviewLoadJob job(entry_for(a.chart), pro, true, Difficulty::Expert, a.best, 4);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        PreviewLoadJob::Result r = job.take_result();
        CHECK(r.track_opts.pro == pro);
        CHECK_FALSE(r.track_state.instants().empty());
        hydra::render::TrackStateOptions opts;
        opts.pro = pro;
        check_same_track(r.track_state, hydra::render::build_track_state(r.scene, opts));
    }
}

// The same for a path change: the overlay job hands back the timeline built
// from its own scene with the options it was given. The first job builds the
// path-free base itself; a second one given that base only lays the overlay
// over it, and gets the same scene and timeline.
TEST_CASE("the Preview overlay job builds the highway timeline on its worker") {
    const AnalyzedChart a = first_chart_with_a_path();
    PreviewLoadJob load(entry_for(a.chart), true, true, Difficulty::Expert, std::nullopt, 4);
    load.start();
    wait_finished(load);
    REQUIRE(load.ok());
    auto song = std::make_shared<const hydra::Song>(load.take_result().song);
    std::shared_ptr<const hydra::ui::PreviewSceneBase> pro_base;
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        hydra::render::TrackStateOptions opts;
        opts.pro = pro;
        // A base built for the other pro setting is not reused.
        hydra::ui::PreviewSceneJob job(song, pro ? nullptr : pro_base, a.best, 4,
                                       hydra::core::default_rules(), "key", opts);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        hydra::ui::PreviewSceneJob::Output out = job.take_output();
        CHECK(out.track_opts.pro == pro);
        CHECK_FALSE(out.scene.activations.empty());  // the path's overlay is in
        check_same_track(out.track_state, hydra::render::build_track_state(out.scene, opts));
        REQUIRE(out.base != nullptr);
        CHECK(out.base != pro_base);
        CHECK(out.base->track_opts.pro == pro);
        CHECK(out.base->scene.activations.empty());
        if (pro) pro_base = out.base;

        // The next path change: the base is shared, not rebuilt.
        hydra::ui::PreviewSceneJob again(song, out.base, a.best, 4,
                                         hydra::core::default_rules(), "key", opts);
        again.start();
        wait_finished(again);
        REQUIRE(again.ok());
        hydra::ui::PreviewSceneJob::Output out2 = again.take_output();
        CHECK(out2.base == out.base);
        CHECK(out2.scene.activations.size() == out.scene.activations.size());
        check_same_track(out2.track_state, out.track_state);
    }
}

// The controller warms the base after a load with this job: the path-free
// scene and the timeline built from it, for the options it was given.
TEST_CASE("the Preview base job builds the path-free scene and timeline") {
    const AnalyzedChart a = first_chart_with_a_path();
    PreviewLoadJob load(entry_for(a.chart), true, true, Difficulty::Expert, std::nullopt, 4);
    load.start();
    wait_finished(load);
    REQUIRE(load.ok());
    auto song = std::make_shared<const hydra::Song>(load.take_result().song);
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        hydra::render::TrackStateOptions opts;
        opts.pro = pro;
        hydra::ui::PreviewBaseJob job(song, opts);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        std::shared_ptr<const hydra::ui::PreviewSceneBase> base = job.take_base();
        REQUIRE(base != nullptr);
        CHECK(base->track_opts.pro == pro);
        const hydra::app::PreviewScene want = hydra::app::build_preview_base(*song);
        CHECK(base->scene.notes.size() == want.notes.size());
        CHECK(base->scene.fills.size() == want.fills.size());
        CHECK(base->scene.activations.empty());
        CHECK(base->scene.sp_meter.segments.empty());
        check_same_track(base->track_state, hydra::render::build_track_state(want, opts));
    }
}

// Closing the details window joins the load's thread on the UI thread. A
// job that ignored its cancel flag ran its whole parse, decode and mix first.
TEST_CASE("a cancelled Preview load stops before decoding") {
    PreviewLoadJob job(entry_for(corpus::first_chart_with_suffix(".chart")), true, true,
                       Difficulty::Expert, std::nullopt, 4);
    job.cancel();  // the window closed before the worker got going
    job.start();
    wait_finished(job);
    CHECK_FALSE(job.ok());
    CHECK(job.error() == "cancelled");
    CHECK(job.progress().step == PreviewLoadJob::Step::Reading);
}

TEST_CASE("a cancelled Dynamics load stops before counting") {
    DynamicsLoadJob job(entry_for(corpus::first_chart_with_suffix(".chart")), true,
                        Difficulty::Expert);
    job.cancel();
    job.start();
    wait_finished(job);
    CHECK_FALSE(job.ok());
    CHECK(job.error() == "cancelled");
}

// No output device is a warning, not a failure: the chart loads, the clock
// plays, and only the sound is missing.
TEST_CASE("with no audio device the Preview still loads, muted, with a warning") {
    PreviewController pc(nullptr, nullptr);
    pc.set_audio_device_factory(
        [](int, int, PreviewController::AudioSource)
            -> std::unique_ptr<hydra::audio::PreviewAudioDevice> {
            throw std::runtime_error("PreviewAudioDevice: ma_device_init failed");
        });
    pc.open(entry_for(chart_with_audio()), true, true, Difficulty::Expert, nullptr, "", 4);
    for (int i = 0; i < 1200 && pc.loading(); ++i) {
        pc.poll();
        Sleep(50);
    }
    REQUIRE_FALSE(pc.loading());

    CHECK(pc.has_audio());        // the stem decoded; only the device failed
    CHECK_FALSE(pc.has_error());  // so no "Preview failed"
    CHECK(pc.audio_warning() == "PreviewAudioDevice: ma_device_init failed");
    CHECK(pc.length_ms() > 0.0);  // the scene is there to draw
    pc.play();
    CHECK(pc.playing());          // the clock runs without a device

    pc.close();
    CHECK(pc.audio_warning().empty());
}

// Picking another path with the Preview open used to rebuild the scene inside
// open(), on the UI thread. It now builds on a job and lands on a later poll().
TEST_CASE("switching paths builds the new overlay off the UI thread") {
    using namespace hydra;
    using namespace hydra::app;
    AnalysisSettings settings;
    settings.depth_mode = DepthMode::Scores;
    settings.depth_value = 2;
    settings.ms_filter = 10.0;
    std::string chart;
    std::optional<AnalysisResult> analyzed;
    for (const std::string& p : corpus::chart_paths()) {
        try {
            AnalysisResult r = analyze_chart_file(p, settings);
            if (!r.record.paths.empty()) {
                chart = p;
                analyzed.emplace(std::move(r));
                break;
            }
        } catch (const std::exception&) {
        }
    }
    REQUIRE(analyzed.has_value());
    const Path& best = analyzed->record.best_path();
    const std::string best_key = path_overlay_key(&best);

    PreviewController pc(nullptr, nullptr);
    pc.open(entry_for(chart), true, true, Difficulty::Expert, nullptr, "", 4);
    for (int i = 0; i < 1200 && pc.loading(); ++i) {
        pc.poll();
        Sleep(50);
    }
    REQUIRE_FALSE(pc.loading());
    const std::string before = pc.overlay_path_key();

    // open() returns at once: the old overlay is still up, and nothing reloads.
    pc.open(entry_for(chart), true, true, Difficulty::Expert, &best, best_key, 4);
    CHECK(pc.overlay_path_key() == before);
    CHECK_FALSE(pc.loading());

    // The new overlay lands on a later poll. A failed build sets the error,
    // and then nothing more will land, so stop waiting.
    for (int i = 0; i < 1200 && pc.overlay_path_key().rfind(best_key, 0) != 0 && !pc.has_error();
         ++i) {
        pc.poll();
        Sleep(10);
    }
    CHECK_FALSE(pc.has_error());
    CHECK(pc.overlay_path_key().rfind(best_key, 0) == 0);
}
