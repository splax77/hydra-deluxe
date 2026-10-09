#include "ui/preview_load_job.h"

#include <algorithm>
#include <future>
#include <string>

#include "app/analysis.h"  // chart_changed_since
#include "app/preview_source.h"
#include "app/preview_view.h"
#include "app/song_length.h"   // chart_song_length_ms
#include "audio/song_audio.h"  // map_song_stems, open_song_stems, mix_song_stems
#include "core/model.h"
#include "core/winstr.h"
#include "render/track_state.h"
#include "ui/library_parts.h"  // time_left_text
#include "ui/widgets.h"        // progress_fraction

namespace hydra::ui {

namespace {

// Each step's share of the loading bar, from timing every step on the
// blink-182 Discography chart (the stress case: a 625 MB, 8.6-hour
// song.opus) with a scratch harness on 2026-10-03, measured under load with
// the file already in the OS cache:
//   read + parse notes.mid  55 ms
//   open both stems        569 ms  (the Opus page index is nearly all of it)
//   build the scene         31 ms
//   build the highway       39 ms
// An ordinary 7-stem chart (Kissing the Shadows) splits about the same way:
// parse 10 ms, open audio 57 ms, scene and highway under 1 ms.
constexpr float kReadShare = 0.08f;
constexpr float kOpenShare = 0.82f;
constexpr float kSceneShare = 0.045f;
// The highway gets what is left (about 0.055), so the slices add up to 1.

// Whole megabytes (10^6 bytes, like file sizes), rounded to nearest.
long long whole_mb(uint64_t bytes) {
    return static_cast<long long>((bytes + 500000) / 1000000);
}

}  // namespace

double ByteRateClock::update(double now_s, uint64_t bytes_done, uint64_t bytes_total) {
    if (window_start_s_ < 0.0) {
        window_start_s_ = now_s;
        window_start_bytes_ = bytes_done;
        last_publish_s_ = now_s;
        return published_;
    }
    // At most one new answer a second, each from the last second's bytes.
    if (now_s - last_publish_s_ < 1.0 || now_s - window_start_s_ < 1.0) return published_;
    const double span = now_s - window_start_s_;
    const uint64_t moved =
        bytes_done > window_start_bytes_ ? bytes_done - window_start_bytes_ : 0;
    const double rate = static_cast<double>(moved) / span;
    const uint64_t left = bytes_total > bytes_done ? bytes_total - bytes_done : 0;
    published_ = rate > 0.0 ? static_cast<double>(left) / rate : -1.0;
    window_start_s_ = now_s;
    window_start_bytes_ = bytes_done;
    last_publish_s_ = now_s;
    return published_;
}

PreviewLoadJob::PreviewLoadJob(store::ChartLibraryEntry entry, bool pro, bool bass2x,
                               Difficulty difficulty, std::optional<Path> path, int sp_cap,
                               core::Rules rules, bool noteshuffle)
    : entry_(std::move(entry)),
      pro_(pro),
      bass2x_(bass2x),
      difficulty_(difficulty),
      path_(std::move(path)),
      sp_cap_(sp_cap),
      rules_(std::move(rules)),
      noteshuffle_(noteshuffle) {}

void PreviewLoadJob::start() {
    started_ = std::chrono::steady_clock::now();
    spawn([this] { run(); });
}

std::vector<std::unique_ptr<audio::StemReader>> PreviewLoadJob::open_audio(
    app::SharedBytes container, const std::atomic<bool>& stop) {
    auto keep_going = [this, &stop] { return !is_cancelled() && !stop.load(); };
    // Find the stems. A container's audio entries are copied out of the
    // container bytes here, checking for a cancel between entries.
    std::vector<app::PreviewAudioStem> stems =
        app::resolve_preview_stems(entry_.notespath, container, keep_going);
    container.reset();  // this branch's hold on the container
    if (!keep_going()) throw JobCancelled{};

    // Map every loose stem first, so the bar's byte total is the size of the
    // bytes the readers will walk (MappedFile::size), read once; a container
    // stem's size is its extracted bytes.
    std::vector<std::optional<audio::StemBytes>> stem_bytes =
        audio::map_song_stems(std::move(stems));
    bytes_total_.store(audio::stems_total_bytes(stem_bytes));

    // The song's own opener, with the bar's byte counter as its progress.
    std::vector<std::unique_ptr<audio::StemReader>> readers;
    try {
        readers = audio::open_song_stems(std::move(stem_bytes),
                                         [this, &keep_going](uint64_t done, uint64_t) {
                                             bytes_done_.store(done);
                                             return keep_going();
                                         });
    } catch (const audio::OpenCancelled&) {
        throw JobCancelled{};
    }
    audio_done_.store(true);
    return readers;
}

void PreviewLoadJob::run() {
    run_guarded([this] {
        // The note stream and the stems are never stored, so re-read both.
        // Closing the details window joins this thread on the UI thread, so
        // both branches look at the cancel flag often: the audio branch every
        // 4 MB, this one between steps (the parse itself takes about 0.1 s on
        // the largest chart).
        throw_if_cancelled();
        // A .sng or .srb is read from disk once; both branches share the bytes.
        app::SharedBytes container = app::read_preview_container(read_file_bytes, entry_.notespath);
        throw_if_cancelled();

        // Has the chart changed since it was analyzed (finding 126)? First the
        // rescan's own shortcut: when app::chart_changed_since says no change, the
        // files still have the scan's md5, so nothing is read.
        // Otherwise the file is hashed with the scan's own rule, so the two
        // can never disagree, on a thread of its own beside both branches: a
        // .sng's hash covers all its audio, so in front of the stem open it
        // would add to the load. A hash that is not the record's means the
        // chart changed. An empty hash (an unreadable file) is not a change:
        // the parse error speaks for that. The future waits for the hash if
        // this job throws.
        std::future<bool> changed_check = std::async(std::launch::async, [this] {
            const std::optional<app::ChartNow> now =
                app::chart_changed_since(entry_.notespath, entry_.sig);
            return now && !now->md5.empty() && now->md5 != entry_.md5;
        });

        // Branch (b) on its own thread. If this branch throws, `stop` tells it
        // to give up and the guard waits for it, so it never outlives the job.
        std::atomic<bool> stop{false};
        std::future<std::vector<std::unique_ptr<audio::StemReader>>> audio_branch =
            std::async(std::launch::async,
                       [this, container, &stop] { return open_audio(container, stop); });
        struct JoinGuard {
            std::atomic<bool>& stop;
            std::future<std::vector<std::unique_ptr<audio::StemReader>>>& f;
            ~JoinGuard() {
                stop.store(true);
                if (f.valid()) f.wait();
            }
        } guard{stop, audio_branch};

        // Branch (a) here: the notes.
        app::PreviewSong ps =
            app::resolve_preview_song(entry_.notespath, container, pro_, bass2x_, difficulty_, rules_,
                                      noteshuffle_);
        // The song's length, from the same owner analysis saves through
        // (D75), with the container already in hand. A failed read costs only
        // the length: the scrubber then ends where playback does
        // (app::scrub_end_ms).
        std::optional<double> song_length_ms;
        try {
            song_length_ms = app::chart_song_length_ms(
                app::chart_timing_meta(entry_.timing, entry_.notespath), entry_.notespath, ps.song,
                difficulty_, bass2x_, rules_, container);
        } catch (const std::exception&) {
        }
        container.reset();
        // A chart with no charting at this difficulty would otherwise build an
        // empty scene and the tab would show a blank highway with no reason
        // given. Throwing here surfaces it as "Preview failed: ...", the same
        // wording analysis uses.
        require_notes(ps.song, difficulty_, pro_);
        reading_done_.store(true);
        throw_if_cancelled();

        // Both branches done: mix. get() rethrows the audio branch's cancel.
        std::vector<std::unique_ptr<audio::StemReader>> readers = audio_branch.get();
        const bool chart_changed = changed_check.get();
        throw_if_cancelled();
        // The song's own mix step: what plays, and where its audio ends.
        audio::SongMix song_mix = audio::mix_song_stems(std::move(readers), ps.audio_offset_ms);
        const std::optional<double> audio_end_ms = song_mix.end_chart_ms;

        // The scene and the highway wait for the audio, because the beat
        // lines run to its end (D48, Q25). A changed chart is drawn with no
        // path, as an unanalyzed one is (drawn_path).
        const Path* path = drawn_path(path_, chart_changed);
        app::PreviewScene scene =
            app::build_preview_scene(ps.song, path, sp_cap_, rules_, audio_end_ms, song_length_ms);
        scene_done_.store(true);
        throw_if_cancelled();
        // The highway timeline, built here so the UI thread only uploads it,
        // with the options the controller draws with (track_options).
        const render::TrackStateOptions track_opts = track_options(pro_);
        render::TrackState track_state = render::build_track_state(scene, track_opts);
        highway_done_.store(true);

        result_ = Result{std::move(scene),  std::move(song_mix.mix),  song_mix.audio_offset_ms,
                         audio_end_ms,      song_length_ms,           std::move(ps.song),
                         std::move(track_state), track_opts,          chart_changed};
        return true;
    });
}

PreviewLoadJob::Result PreviewLoadJob::take_result() { return std::move(*result_); }

PreviewLoadJob::Progress PreviewLoadJob::progress() const {
    Progress p;
    if (!reading_done_.load())
        p.step = Step::Reading;
    else if (!audio_done_.load())
        p.step = Step::Opening;
    else if (!scene_done_.load())
        p.step = Step::Building;
    else
        p.step = Step::Highway;
    p.bytes_total = bytes_total_.load();
    p.bytes_done = std::min(bytes_done_.load(), p.bytes_total);
    p.elapsed_s =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - started_).count();
    if (p.step == Step::Opening) {
        std::lock_guard<std::mutex> lock(clock_mutex_);
        p.time_left_s = clock_.update(p.elapsed_s, p.bytes_done, p.bytes_total);
    }
    return p;
}

float PreviewLoadJob::Progress::fraction() const {
    switch (step) {
        case Step::Reading: return 0.0f;
        case Step::Opening:
            // The audio slice's fill is the shared progress bar rule: a zero
            // total (no stems, empty files) reads empty, "nothing reported
            // yet", so the bar stays at the slice's start.
            return kReadShare + kOpenShare * progress_fraction(static_cast<double>(bytes_done),
                                                               static_cast<double>(bytes_total));
        case Step::Building: return kReadShare + kOpenShare;
        case Step::Highway: return kReadShare + kOpenShare + kSceneShare;
    }
    return 0.0f;
}

std::string PreviewLoadJob::Progress::label() const {
    switch (step) {
        case Step::Reading: return "Reading chart";
        case Step::Opening: {
            // Under half a megabyte in all would read "0 of 0 MB"; just the name.
            const long long total = whole_mb(bytes_total);
            if (total <= 0) return "Opening audio";
            const long long done = std::min(whole_mb(std::min(bytes_done, bytes_total)), total);
            return "Opening audio: " + std::to_string(done) + " of " + std::to_string(total) +
                   " MB";
        }
        case Step::Building: return "Building scene";
        case Step::Highway: return "Building highway";
    }
    return "";
}

std::string gated_time_left_text(double elapsed_s, double time_left_s) {
    if (elapsed_s < kTimeLeftAfterSeconds || !(time_left_s >= 0.0)) return "";
    return detail::time_left_text(time_left_s);
}

// Only the Preview's own step check lives here: opening audio is the one step
// with a byte rate. The gate and the words are gated_time_left_text's.
std::string PreviewLoadJob::Progress::time_left_text() const {
    if (step != Step::Opening) return "";
    return gated_time_left_text(elapsed_s, time_left_s);
}

std::shared_ptr<const PreviewSceneBase> build_scene_base(
    const Song& song, render::TrackStateOptions track_opts, std::optional<double> audio_end_ms,
    std::optional<double> song_length_ms, const std::function<void()>& check_cancel) {
    auto built = std::make_shared<PreviewSceneBase>();
    built->scene = app::build_preview_base(song, audio_end_ms, song_length_ms);
    check_cancel();
    built->track_state = render::build_track_state(built->scene, track_opts);
    built->track_opts = track_opts;
    check_cancel();
    return built;
}

PreviewBaseJob::PreviewBaseJob(std::shared_ptr<const Song> song,
                               render::TrackStateOptions track_opts,
                               std::optional<double> audio_end_ms,
                               std::optional<double> song_length_ms)
    : song_(std::move(song)),
      track_opts_(track_opts),
      audio_end_ms_(audio_end_ms),
      song_length_ms_(song_length_ms) {}

void PreviewBaseJob::start() { spawn([this] { run(); }); }

void PreviewBaseJob::run() {
    run_guarded([this] {
        throw_if_cancelled();
        base_ = build_scene_base(*song_, track_opts_, audio_end_ms_, song_length_ms_,
                                 [this] { throw_if_cancelled(); });
        return true;
    });
}

PreviewSceneJob::PreviewSceneJob(std::shared_ptr<const Song> song,
                                 std::shared_ptr<const PreviewSceneBase> base,
                                 std::optional<Path> path, int sp_cap, core::Rules rules,
                                 std::string key, render::TrackStateOptions track_opts,
                                 std::optional<double> audio_end_ms,
                                 std::optional<double> song_length_ms)
    : song_(std::move(song)),
      base_(std::move(base)),
      path_(std::move(path)),
      sp_cap_(sp_cap),
      rules_(std::move(rules)),
      key_(std::move(key)),
      track_opts_(track_opts),
      audio_end_ms_(audio_end_ms),
      song_length_ms_(song_length_ms) {}

void PreviewSceneJob::start() { spawn([this] { run(); }); }

void PreviewSceneJob::run() {
    run_guarded([this] {
        throw_if_cancelled();  // a newer selection already replaced this one
        // The path-free half, once per chart: reuse the controller's, or
        // build it here when there is none yet (or it was drawn with other
        // timeline options).
        std::shared_ptr<const PreviewSceneBase> base = base_;
        if (!base || base->track_opts != track_opts_)
            base = build_scene_base(*song_, track_opts_, audio_end_ms_, song_length_ms_,
                                    [this] { throw_if_cancelled(); });
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
