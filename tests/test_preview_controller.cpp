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
#include "app/config.h"  // Settings: the Preview volume's owner
#include "chart_text.h"
#include "app/preview_view.h"
#include "audio/device.h"
#include "audio_chart_fixtures.h"
#include "audio_util.h"  // fixture_path
#include "core/winstr.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "render/track_state.h"
#include "store/record_store.h"
#include "temp_util.h"
#include "app/config.h"
#include "ui/library_jobs.h"  // ViewJob
#include "ui/preview_controller.h"
#include "ui/preview_load_job.h"
#include "wait_util.h"
#include "warp_util.h"

#ifndef HYDRA_TESTDATA_DIR
#error "HYDRA_TESTDATA_DIR must be defined (see CMakeLists.txt)"
#endif

using hydra::Difficulty;
using hydra::store::ChartLibraryEntry;
using hydra::ui::ViewJob;
using hydra::ui::PreviewController;
using hydra::ui::PreviewLoadJob;
using testwait::wait_until;

namespace {

// Waits for a job's worker to finish.
template <class Job>
void wait_finished(const Job& job) {
    wait_until([&] { return job.finished(); }, "the load job to finish");
}

// Polls until the first load has landed (loading()).
void wait_loaded(PreviewController& pc) {
    wait_until(
        [&] {
            pc.poll();
            return !pc.loading();
        },
        "the Preview's first load to land");
}

// Polls until every Preview thread is done (busy(), finding 109).
void settle(PreviewController& pc) {
    wait_until(
        [&] {
            pc.poll();
            return !pc.busy();
        },
        "every Preview thread to finish");
}

// Polls until the overlay drawn is `path_key`'s (shows_path), or a failed
// build set the error and nothing more will land.
void wait_for_path(PreviewController& pc, const std::string& path_key) {
    wait_until(
        [&] {
            pc.poll();
            return pc.shows_path(path_key) || pc.has_error();
        },
        "the overlay for " + path_key + " to land");
}

// An audio device factory that fails as a PC with no device would, so
// nothing real opens.
PreviewController::AudioDeviceFactory no_device_factory() {
    return [](int, int, PreviewController::AudioSource)
               -> std::unique_ptr<hydra::audio::PreviewAudioDevice> {
        throw std::runtime_error("no device in tests");
    };
}

// The entry the scan would make (corpus::scanned_entry), without its
// fingerprint: its md5 is the file's own hash, so the Preview hashes the file,
// takes the chart as unchanged and draws the path it is given.
ChartLibraryEntry entry_for(const std::string& notespath) {
    ChartLibraryEntry e = corpus::scanned_entry(notespath);
    e.sig.clear();
    return e;
}

// The chart folders with audio (chart_with_audio, short_chart_with_long_audio)
// and their helpers live in audio_chart_fixtures.h.
using audiochart::chart_with_audio;
using audiochart::copy_file_utf8;
using audiochart::short_chart_with_long_audio;

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

// The settings every analysis in this file runs with.
hydra::app::AnalysisSettings analysis_settings() {
    hydra::app::AnalysisSettings settings;
    settings.depth_mode = hydra::DepthMode::Scores;
    settings.depth_value = 2;
    settings.ms_filter = 10.0;
    return settings;
}

// `chart` analyzed with analysis_settings(); the test stops unless Hydra
// found a path on it.
hydra::app::AnalysisResult analyze_with_paths(const std::string& chart) {
    hydra::app::AnalysisResult analyzed = hydra::app::analyze_chart_file(chart, analysis_settings());
    REQUIRE_FALSE(analyzed.record.paths.empty());
    return analyzed;
}

// A corpus chart Hydra finds at least one path on, and its best path.
struct AnalyzedChart {
    std::string chart;
    hydra::Path best;
};
AnalyzedChart first_chart_with_a_path() {
    const corpus::ChartWithPaths found = corpus::first_chart_with_paths(analysis_settings());
    return {found.chart, found.result.record.best_path()};
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
        const hydra::render::TrackStateOptions opts = hydra::ui::track_options(pro);
        CHECK(r.track_opts == opts);
        CHECK_FALSE(r.track_state.instants().empty());
        check_same_track(r.track_state, hydra::render::build_track_state(r.scene, opts));
    }
}

// The beat lines keep scrolling while the music plays past the last note
// (D48, Q25): the load builds its scene once the audio's length is known, and
// the grid ends on the beat a playhead at the audio's end shows, 5 s in, tick
// 9600, not two measures past the last note.
TEST_CASE("the Preview load runs the beat lines to the end of the audio") {
    const std::string notes = short_chart_with_long_audio("prevctl_grid_len");
    audiochart::write_text_file(hydra::parent_folder(notes) + "\\song.ini",
                                "[song]\nsong_length = 3000\n");
    PreviewLoadJob job(entry_for(notes), true, true, Difficulty::Expert, std::nullopt, 4);
    job.start();
    wait_finished(job);
    REQUIRE(job.ok());
    PreviewLoadJob::Result r = job.take_result();
    // Where the audio stops in chart time, which the controller keeps for
    // every later base build: the sine's five seconds.
    REQUIRE(r.audio_end_ms.has_value());
    CHECK(*r.audio_end_ms == doctest::Approx(5000.0));
    // The SP meter curve closes at the song's length, the one analysis saves
    // for this chart (D75), not at the audio's end.
    CHECK(r.scene.song_length_ms == 3000.0);
    CHECK(hydra::app::analysis_song_length(std::nullopt, notes, r.song, {}).ms == 3000.0);
    REQUIRE_FALSE(r.scene.beats.empty());
    CHECK(r.scene.beats.back().tick == 9600);
    CHECK(r.scene.beats.back().ms == doctest::Approx(5000.0));
    // The timeline the renderer draws is built from that same scene.
    check_same_track(r.track_state, hydra::render::build_track_state(r.scene, r.track_opts));
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
        const hydra::render::TrackStateOptions opts = hydra::ui::track_options(pro);
        // A base built for the other pro setting is not reused. This chart
        // has no audio, so there is no audio end.
        hydra::ui::PreviewSceneJob job(song, pro ? nullptr : pro_base, a.best, 4,
                                       hydra::core::default_rules(), "key", opts, std::nullopt);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        hydra::ui::PreviewSceneJob::Output out = job.take_output();
        CHECK(out.track_opts == opts);
        CHECK_FALSE(out.scene.activations.empty());  // the path's overlay is in
        check_same_track(out.track_state, hydra::render::build_track_state(out.scene, opts));
        REQUIRE(out.base != nullptr);
        CHECK(out.base != pro_base);
        CHECK(out.base->track_opts == opts);
        CHECK(out.base->scene.activations.empty());
        if (pro) pro_base = out.base;

        // The next path change: the base is shared, not rebuilt.
        hydra::ui::PreviewSceneJob again(song, out.base, a.best, 4,
                                         hydra::core::default_rules(), "key", opts, std::nullopt);
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
// scene and the timeline built from it, for the options and the audio end it
// was given. A scene job that has no base yet builds one the same way, so
// the beat lines still reach the audio's end after a path change.
TEST_CASE("the Preview base job builds the path-free scene and timeline") {
    PreviewLoadJob load(entry_for(short_chart_with_long_audio()), true, true, Difficulty::Expert,
                        std::nullopt, 4);
    load.start();
    wait_finished(load);
    REQUIRE(load.ok());
    PreviewLoadJob::Result r = load.take_result();
    REQUIRE(r.audio_end_ms.has_value());
    auto song = std::make_shared<const hydra::Song>(std::move(r.song));
    for (bool pro : {true, false}) {
        CAPTURE(pro);
        const hydra::render::TrackStateOptions opts = hydra::ui::track_options(pro);
        hydra::ui::PreviewBaseJob job(song, opts, r.audio_end_ms);
        job.start();
        wait_finished(job);
        REQUIRE(job.ok());
        std::shared_ptr<const hydra::ui::PreviewSceneBase> base = job.take_base();
        REQUIRE(base != nullptr);
        CHECK(base->track_opts == opts);
        const hydra::app::PreviewScene want = hydra::app::build_preview_base(*song, r.audio_end_ms);
        CHECK(base->scene.notes.size() == want.notes.size());
        CHECK(base->scene.fills.size() == want.fills.size());
        CHECK(base->scene.activations.empty());
        CHECK(base->scene.sp_meter.segments.empty());
        REQUIRE_FALSE(base->scene.beats.empty());
        CHECK(base->scene.beats.back().tick == 9600);
        check_same_track(base->track_state, hydra::render::build_track_state(want, opts));

        hydra::ui::PreviewSceneJob scene_job(song, nullptr, std::nullopt, 4,
                                             hydra::core::default_rules(), "key", opts,
                                             r.audio_end_ms);
        scene_job.start();
        wait_finished(scene_job);
        REQUIRE(scene_job.ok());
        const hydra::ui::PreviewSceneJob::Output out = scene_job.take_output();
        REQUIRE(out.base != nullptr);
        REQUIRE_FALSE(out.base->scene.beats.empty());
        CHECK(out.base->scene.beats.back().tick == 9600);
        REQUIRE_FALSE(out.scene.beats.empty());
        CHECK(out.scene.beats.back().tick == 9600);
    }
}

// The Preview's view settings become highway options in one place
// (finding R7.16), and the jobs compare the options whole.
TEST_CASE("track_options: one builder, compared whole") {
    using hydra::ui::track_options;
    CHECK(track_options(true) == track_options(true));
    CHECK(track_options(true) != track_options(false));
    CHECK(track_options(true).pro);
    CHECK_FALSE(track_options(false).pro);
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

// The click's job counts the Dynamics too (D87 item 1); a cancel before it
// got going stops it before any parse.
TEST_CASE("a cancelled click job stops before counting") {
    const hydra::store::ChartLibraryEntry entry =
        entry_for(corpus::first_chart_with_suffix(".chart"));
    const hydra::app::Settings settings;
    ViewJob job(entry, settings.record_key(entry.md5), settings.to_analysis_settings(),
                /*analysis_off=*/false, /*generation=*/0);
    job.cancel();
    job.start();
    wait_finished(job);
    CHECK_FALSE(job.ok());
    CHECK(job.is_cancelled());
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
    wait_loaded(pc);

    CHECK(pc.has_audio());        // the stem decoded; only the device failed
    CHECK_FALSE(pc.has_error());  // so no "Preview failed"
    CHECK(pc.audio_warning() == "PreviewAudioDevice: ma_device_init failed");
    CHECK(pc.scrub_end_ms() > 0.0);  // the scene is there to draw
    pc.play();
    CHECK(pc.playing());          // the clock runs without a device

    pc.close();
    CHECK(pc.audio_warning().empty());
}

// A device that opens but won't start takes the same path as one that won't
// open (D109): the chart loads, the clock plays, the device is let go and
// the warning names the step that failed.
TEST_CASE("with an audio device that won't start the Preview still loads, muted, with a warning") {
    // A headless device (nothing real opens) whose start() is forced to
    // fail; the guard puts both switches back when the case ends.
    struct SwitchRestore {
        bool headless;
        bool start_fails;
        ~SwitchRestore() {
            hydra::audio::set_start_fails(start_fails);
            hydra::audio::set_headless(headless);
        }
    } restore{hydra::audio::set_headless(true), hydra::audio::set_start_fails(true)};

    PreviewController pc(nullptr, nullptr);
    pc.open(entry_for(chart_with_audio()), true, true, Difficulty::Expert, nullptr, "", 4);
    wait_loaded(pc);

    CHECK(pc.has_audio());        // the stem decoded; only the device failed
    CHECK_FALSE(pc.has_error());  // so no "Preview failed"
    CHECK_FALSE(pc.has_audio_device());
    CHECK(pc.audio_warning() == "PreviewAudioDevice: ma_device_start failed");
    CHECK(pc.scrub_end_ms() > 0.0);  // the scene is there to draw
    pc.play();
    CHECK(pc.playing());          // the clock runs without a device

    pc.close();
    CHECK(pc.audio_warning().empty());
}

// The Preview's look is the renderer's 3d-config.json and nothing else
// (finding 220): there is no default-built struct behind it. Only the first
// render builds the renderer, so asking before then fails loudly.
TEST_CASE("preview_config: asked before the first render it fails loudly") {
    PreviewController pc(nullptr, nullptr);
    CHECK_THROWS_AS(pc.preview_config(), std::logic_error);
}

// Finding 193: a missing Preview file reads its kind's sentence, the one D51
// call 25 chose, and the raw text stays for the details line.
TEST_CASE("a Preview asset failure reads Reinstall Hydra, with the raw text as its detail") {
    const hydra::app::PathOverrides previous = hydra::app::path_overrides();
    hydra::app::PathOverrides overrides = previous;
    overrides.asset_dir = testtemp::temp_path("no_preview_assets", "");
    hydra::app::set_path_overrides(overrides);
    {
        PreviewController pc(nullptr, nullptr);
        CHECK(pc.render(64, 64) == nullptr);
        CHECK(pc.error() ==
              "Some of Hydra's Preview files are missing. Reinstall Hydra to restore them.");
        CHECK(pc.error_detail().rfind("PreviewRenderer: missing", 0) == 0);
    }
    hydra::app::set_path_overrides(previous);
}

namespace {

// short_chart_with_long_audio's chart (last note at 100 ms, the sine runs
// 5 s) with a song.ini stating a 3 s length.
std::string long_audio_chart_stating_3s(const std::string& tag) {
    const std::string notes = short_chart_with_long_audio(tag);
    audiochart::write_text_file(hydra::parent_folder(notes) + "\\song.ini",
                                "[song]\nsong_length = 3000\n");
    return notes;
}

// A controller with no audio device, opened on `notes` and loaded.
void open_and_load(PreviewController& pc, const std::string& notes) {
    pc.set_audio_device_factory(no_device_factory());
    pc.open(entry_for(notes), true, true, Difficulty::Expert, nullptr, "", 4);
    wait_loaded(pc);
    REQUIRE(pc.has_audio());
}

}  // namespace

// D95 call 4: closing the Preview frees its GPU render targets, and the next
// open draws again.
TEST_CASE("closing the Preview frees its render targets; reopening draws again (WARP)") {
    Microsoft::WRL::ComPtr<ID3D11Device> dev;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx;
    REQUIRE(warp::make_device(dev, ctx));
    const hydra::app::PathOverrides previous = hydra::app::path_overrides();
    hydra::app::PathOverrides overrides = previous;
    overrides.asset_dir = HYDRA_ASSET_DIR;
    hydra::app::set_path_overrides(overrides);
    {
        const std::string notes = short_chart_with_long_audio("prevctl_release");
        PreviewController pc(dev.Get(), ctx.Get());
        open_and_load(pc, notes);
        ID3D11ShaderResourceView* srv = pc.render(64, 64);
        REQUIRE(srv != nullptr);

        // Hold the shown texture: once freed, this is its only reference.
        Microsoft::WRL::ComPtr<ID3D11Resource> shown;
        srv->GetResource(&shown);
        pc.close();
        shown->AddRef();
        CHECK(shown->Release() == 1);  // Release returns the count left

        open_and_load(pc, notes);
        CHECK(pc.render(64, 64) != nullptr);
    }
    hydra::app::set_path_overrides(previous);
}

// The scrubber ends at the song's length (D75), here the 3 s song.ini
// states, not the audio's 5 s end. The slider's range is scrub_end_ms(), so a
// drag to its right end seeks there, and the thumb follows the playhead.
TEST_CASE("scrub marks: the Preview's scrubber ends at the song's length") {
    PreviewController pc(nullptr, nullptr);
    open_and_load(pc, long_audio_chart_stating_3s("prevctl_scrub_len"));

    CHECK(pc.scrub_end_ms() == doctest::Approx(3000.0));  // the slider's right end
    pc.seek_ms(pc.scrub_end_ms());                         // a drag to that end
    CHECK(pc.position_ms() == doctest::Approx(3000.0));

    pc.seek_ms(2100.0);  // in the tail after the last note
    CHECK(hydra::app::scrub_thumb_ms(pc.position_ms(), pc.scrub_end_ms()) ==
          doctest::Approx(2100.0));
}

// Playback still runs to the audio's end (D48), past the song's length: a
// jump past the end stops there. Same chart: length 3 s, the sine runs 5 s.
TEST_CASE("a jump past the end of the Preview stops at the audio's end") {
    PreviewController pc(nullptr, nullptr);
    open_and_load(pc, long_audio_chart_stating_3s("prevctl_jump_len"));

    CHECK(pc.playback_end_ms() == doctest::Approx(5000.0));
    CHECK(pc.scrub_end_ms() == doctest::Approx(3000.0));
    pc.jump_ms(60000.0);
    CHECK(pc.position_ms() == doctest::Approx(5000.0));
}

// Picking another path with the Preview open used to rebuild the scene inside
// open(), on the UI thread. It now builds on a job and lands on a later poll().
TEST_CASE("switching paths builds the new overlay off the UI thread") {
    using namespace hydra;
    using namespace hydra::app;
    const corpus::ChartWithPaths found = corpus::first_chart_with_paths(analysis_settings());
    const std::string& chart = found.chart;
    const Path& best = found.result.record.best_path();
    const std::string best_key = path_overlay_key(&best);

    PreviewController pc(nullptr, nullptr);
    pc.open(entry_for(chart), true, true, Difficulty::Expert, nullptr, "", 4);
    wait_loaded(pc);
    const std::string before = pc.overlay_path_key();

    // open() returns at once: the old overlay is still up, and nothing reloads.
    pc.open(entry_for(chart), true, true, Difficulty::Expert, &best, best_key, 4);
    CHECK(pc.overlay_path_key() == before);
    CHECK_FALSE(pc.loading());

    // The new overlay lands on a later poll.
    wait_for_path(pc, best_key);
    CHECK_FALSE(pc.has_error());
    CHECK(pc.shows_path(best_key));
}

// "Is the drawn overlay the selected path's?" is the controller's to answer
// (finding 340): the same path at another SP cap still is, before and after
// the rebuild for the new cap lands; another path is not.
TEST_CASE("shows_path: the same path at another cap, and a different path") {
    using namespace hydra;
    using namespace hydra::app;
    const std::string chart = chart_with_audio();
    const AnalysisResult analyzed = analyze_with_paths(chart);
    const Path& best = analyzed.record.best_path();
    const std::string best_key = path_overlay_key(&best);
    // Another path's key: the best one's with its last character changed.
    std::string other_key = best_key;
    REQUIRE_FALSE(other_key.empty());
    other_key.back() = other_key.back() == 'x' ? 'y' : 'x';

    PreviewController pc(nullptr, nullptr);
    pc.set_audio_device_factory(no_device_factory());
    const ChartLibraryEntry entry = entry_for(chart);
    pc.open(entry, true, true, Difficulty::Expert, &best, best_key, 4);
    wait_for_path(pc, best_key);
    REQUIRE_FALSE(pc.has_error());
    CHECK(pc.shows_path(best_key));
    CHECK_FALSE(pc.shows_path(other_key));
    CHECK_FALSE(pc.shows_path(""));
    const std::string at_cap4 = pc.overlay_path_key();

    // The same path at cap 1: the cap-4 overlay stays up until the rebuild
    // lands, and both are this path's.
    pc.open(entry, true, true, Difficulty::Expert, &best, best_key, 1);
    CHECK(pc.overlay_path_key() == at_cap4);
    CHECK(pc.shows_path(best_key));
    settle(pc);
    REQUIRE_FALSE(pc.has_error());
    CHECK(pc.overlay_path_key() != at_cap4);  // the cap-1 overlay is in
    CHECK(pc.shows_path(best_key));
    CHECK_FALSE(pc.shows_path(other_key));
}

namespace {

// no_device_factory, counting its calls: how many times the Preview opened
// its audio for output.
PreviewController::AudioDeviceFactory counting_factory(int& calls) {
    return [&calls, fail = no_device_factory()](int channels, int sample_rate,
                                               PreviewController::AudioSource source) {
        ++calls;
        return fail(channels, sample_rate, std::move(source));
    };
}

// A chart with Expert and Hard drums, the last note at 2 s (tick 3840 at 600
// BPM, 192 ticks a beat), and 5 s of audio.
std::string expert_and_hard_chart_with_audio(const std::string& tag) {
    const std::string dir = testtemp::temp_dir(tag);
    using namespace testchart;
    const std::string notes = line(0, "N 1 0") + line(1920, "N 2 0") + line(3840, "N 1 0");
    audiochart::write_text_file(
        dir + "\\notes.chart",
        chart_text(section("ExpertDrums", notes) + section("HardDrums", notes), 192, "",
                   line(0, "TS 4") + line(0, "B 600000")));
    copy_file_utf8(testaudio::fixture_path("sine220.ogg"), dir + "\\song.ogg");
    return dir + "\\notes.chart";
}

}  // namespace

// With a chart open on the Preview tab, a mode change used to keep the old
// mode's notes: open() compared only the md5. Each of difficulty, Pro Drums,
// 2x Bass and Note Shuffle picks another song (D48, Q22; D104 item 1). Once
// the first load is done, only the notes reload: the audio is not opened
// again, the playhead stays where it was, and playing stays playing.
TEST_CASE("a mode change on the open chart reloads only the notes and keeps the playhead") {
    PreviewController pc(nullptr, nullptr);
    int device_calls = 0;
    pc.set_audio_device_factory(counting_factory(device_calls));
    const ChartLibraryEntry entry = entry_for(expert_and_hard_chart_with_audio("prevctl_modes"));
    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    settle(pc);
    REQUIRE_FALSE(pc.has_error());
    REQUIRE(pc.has_audio());
    REQUIRE(device_calls == 1);

    // The same chart in the same mode is a no-op: nothing reloads.
    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    CHECK_FALSE(pc.busy());

    struct Mode {
        std::string what;
        bool pro;
        bool bass2x;
        Difficulty difficulty;
        bool noteshuffle;
    };
    const Mode modes[] = {
        {"difficulty", true, false, Difficulty::Hard, false},
        {"Pro Drums", false, false, Difficulty::Hard, false},
        {"2x Bass", false, true, Difficulty::Hard, false},
        {"Note Shuffle", false, true, Difficulty::Hard, true},
    };
    for (const Mode& m : modes) {
        CAPTURE(m.what);
        // Paused: the playhead stays exactly where it was.
        pc.pause();
        pc.seek_ms(1500.0);
        pc.open(entry, m.pro, m.bass2x, m.difficulty, nullptr, "", 4, hydra::core::default_rules(),
                m.noteshuffle);
        CHECK_FALSE(pc.loading());  // the old highway stays up, no loading bar
        CHECK(pc.busy());           // the notes job runs
        settle(pc);
        INFO(pc.error_detail());
        CHECK_FALSE(pc.has_error());
        CHECK_FALSE(pc.playing());
        CHECK(pc.position_ms() == doctest::Approx(1500.0));
        CHECK(pc.has_audio());
    }

    // Playing: it keeps playing through a mode change, from about where it was.
    pc.seek_ms(1500.0);
    pc.play();
    REQUIRE(pc.playing());
    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    settle(pc);
    CHECK_FALSE(pc.has_error());
    CHECK(pc.playing());
    CHECK(pc.position_ms() >= 1500.0);
    pc.pause();

    // Five mode changes, and the audio was opened only by the first load.
    CHECK(device_calls == 1);
}

// A mode the chart has no notes in fails as the first load would ("Preview
// failed: ..."), but keeps the audio and the spot: the Preview pauses where
// it is. Switching back clears the error and lands on the same spot, paused.
TEST_CASE("a mode with no notes pauses the Preview in place; switching back clears it") {
    PreviewController pc(nullptr, nullptr);
    int device_calls = 0;
    pc.set_audio_device_factory(counting_factory(device_calls));
    // Expert notes only, and 5 s of audio.
    const ChartLibraryEntry entry = entry_for(short_chart_with_long_audio("prevctl_nonotes"));
    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    settle(pc);
    REQUIRE_FALSE(pc.has_error());
    REQUIRE(pc.has_audio());

    pc.seek_ms(400.0);
    pc.play();
    pc.open(entry, true, false, Difficulty::Hard, nullptr, "", 4);
    settle(pc);
    CHECK(pc.has_error());
    CHECK_FALSE(pc.playing());
    CHECK(pc.has_audio());
    const double stopped_at = pc.position_ms();
    CHECK(stopped_at >= 400.0);

    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    CHECK_FALSE(pc.has_error());  // cleared as the reload starts
    settle(pc);
    CHECK_FALSE(pc.has_error());
    CHECK_FALSE(pc.playing());
    CHECK(pc.position_ms() == doctest::Approx(stopped_at));
    CHECK(device_calls == 1);
}

// Two mode changes in a row: the second one's notes are the ones drawn, and
// the first one's never land. The chart has no audio, so where playback ends
// is each mode's last note: Expert's at tick 960, Hard's at 1920, Medium's at
// 2880, at 600 BPM and 192 ticks a beat (100 ms a beat).
TEST_CASE("a second mode change during a notes reload wins") {
    const std::string dir = testtemp::temp_dir("prevctl_twomodes");
    using namespace testchart;
    audiochart::write_text_file(
        dir + "\\notes.chart",
        chart_text(section("ExpertDrums", line(0, "N 1 0") + line(960, "N 1 0")) +
                       section("HardDrums", line(0, "N 1 0") + line(1920, "N 1 0")) +
                       section("MediumDrums", line(0, "N 1 0") + line(2880, "N 1 0")),
                   192, "", line(0, "TS 4") + line(0, "B 600000")));
    const ChartLibraryEntry entry = entry_for(dir + "\\notes.chart");

    PreviewController pc(nullptr, nullptr);
    pc.open(entry, true, false, Difficulty::Expert, nullptr, "", 4);
    settle(pc);
    REQUIRE_FALSE(pc.has_error());
    REQUIRE(pc.playback_end_ms() == doctest::Approx(500.0));

    pc.open(entry, true, false, Difficulty::Hard, nullptr, "", 4);
    pc.open(entry, true, false, Difficulty::Medium, nullptr, "", 4);
    wait_until(
        [&] {
            if (!pc.busy()) return true;
            pc.poll();
            CHECK(pc.playback_end_ms() != doctest::Approx(1000.0));  // Hard's never lands
            return false;
        },
        "every Preview thread to finish");
    CHECK_FALSE(pc.has_error());
    CHECK(pc.playback_end_ms() == doctest::Approx(1500.0));
}

// An edited chart that was not rescanned used to draw its record's path on
// the new notes (finding 126). The load hashes the file the way the scan
// does; a hash that is not the record's hides the path, and the panel says
// so (D51 call 18).
TEST_CASE("the Preview hides the path when the chart file changed since its record") {
    using namespace hydra;
    using namespace hydra::app;
    const std::string chart = chart_with_audio();
    const AnalysisResult analyzed = analyze_with_paths(chart);
    const Path& best = analyzed.record.best_path();
    const std::string best_key = path_overlay_key(&best);

    for (const bool changed : {true, false}) {
        CAPTURE(changed);
        ChartLibraryEntry entry = entry_for(chart);
        if (changed) entry.md5 = "0123456789abcdef0123456789abcdef";
        PreviewController pc(nullptr, nullptr);
        pc.set_audio_device_factory(no_device_factory());
        pc.open(entry, true, true, Difficulty::Expert, &best, best_key, 4);
        wait_loaded(pc);
        REQUIRE_FALSE(pc.has_error());
        CHECK(pc.chart_changed() == changed);
        CHECK(pc.scrub_marks().empty() == changed);
    }
}

// A 1 GB .sng costs seconds to hash, so the changed-chart check asks the
// rescan's own shortcut first: files app::chart_changed_since finds
// unchanged keep the scan's md5 and are not read again. The
// entry's md5 here is deliberately wrong; only a re-hash could notice that.
TEST_CASE("the Preview trusts the scan's fingerprint and does not re-hash an unchanged chart") {
    using namespace hydra;
    const std::string dir = testtemp::temp_dir("prevctl_sig");
    copy_file_utf8(corpus::first_chart_with_suffix(".chart"), dir + "\\notes.chart");
    std::FILE* ini = fopen_utf8(dir + "\\song.ini", L"wb");
    REQUIRE(ini != nullptr);
    std::fputs("[song]\nname = Fingerprint test\n", ini);
    std::fclose(ini);

    // The fingerprint and md5 come from the scan itself.
    auto [items, errors] = app::discover_charts({dir});
    REQUIRE(items.size() == 1);
    const ChartLibraryEntry scanned = app::to_library_entry(items[0]);
    CHECK_FALSE(app::chart_changed_since(scanned.notespath, scanned.sig).has_value());
    CHECK(app::chart_changed_since(scanned.notespath, "").has_value());

    // The library list hands the Preview the same fingerprint the scan stored.
    store::RecordStore db(":memory:");
    db.rebuild_chart_library({scanned});
    const std::vector<ChartLibraryEntry> listed = db.list_chart_library(0, -1);
    REQUIRE(listed.size() == 1);
    CHECK(listed[0].sig == scanned.sig);

    // {fingerprint, expected chart_changed}: the scan's own fingerprint skips
    // the hash, so the wrong md5 goes unnoticed; a stale one or none hashes.
    const std::pair<std::string, bool> cases[] = {
        {scanned.sig, false}, {scanned.sig + ":stale", true}, {"", true}};
    for (const auto& [sig, changed] : cases) {
        CAPTURE(sig);
        ChartLibraryEntry entry = listed[0];
        entry.md5 = "0123456789abcdef0123456789abcdef";
        entry.sig = sig;
        PreviewController pc(nullptr, nullptr);
        pc.set_audio_device_factory(no_device_factory());
        pc.open(entry, true, true, Difficulty::Expert, nullptr, "", 4);
        wait_loaded(pc);
        REQUIRE_FALSE(pc.has_error());
        CHECK(pc.chart_changed() == changed);
    }
}

// wait-idle in the GUI harness means every Preview thread is done, not only
// the first load: the base job and the overlay jobs count too (finding 109).
TEST_CASE("busy covers the overlay and base jobs, not only the first load") {
    const AnalyzedChart a = first_chart_with_a_path();
    const std::string best_key = hydra::app::path_overlay_key(&a.best);
    PreviewController pc(nullptr, nullptr);
    pc.open(entry_for(a.chart), true, true, Difficulty::Expert, nullptr, "", 4);
    wait_loaded(pc);

    pc.open(entry_for(a.chart), true, true, Difficulty::Expert, &a.best, best_key, 4);
    CHECK(pc.busy());
    CHECK_FALSE(pc.loading());
    settle(pc);
    CHECK_FALSE(pc.has_error());
    CHECK(pc.shows_path(best_key));
}

// The Preview's volume is the setting's: its default and its range
// come from app::Settings, the owner (finding 72).
TEST_CASE("the Preview volume is the settings owner's: default and clamp") {
    PreviewController pc(nullptr, nullptr);
    CHECK(pc.volume_percent() == hydra::app::Settings{}.preview_volume);
    pc.set_volume(150);
    CHECK(pc.volume_percent() == 100);
    pc.set_volume(-5);
    CHECK(pc.volume_percent() == 0);
}

TEST_CASE("the Preview's song key: another difficulty, Pro Drums or 2x Bass makes a different song") {
    using hydra::ui::PreviewSongKey;
    const PreviewSongKey key{"prevctl", Difficulty::Expert, /*pro=*/true, /*bass2x=*/false};
    const PreviewSongKey same{"prevctl", Difficulty::Expert, /*pro=*/true, /*bass2x=*/false};
    CHECK(key == same);
    CHECK_FALSE(key != same);

    PreviewSongKey hard = key;
    hard.difficulty = Difficulty::Hard;
    CHECK(key != hard);
    CHECK_FALSE(key == hard);

    PreviewSongKey plain = key;
    plain.pro = false;
    CHECK(key != plain);
    CHECK_FALSE(key == plain);

    PreviewSongKey bass = key;
    bass.bass2x = true;
    CHECK(key != bass);
    CHECK_FALSE(key == bass);
}
