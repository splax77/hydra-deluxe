#include "ui/preview_load_job.h"

#include <algorithm>
#include <cmath>
#include <future>
#include <string>

#include "app/preview_source.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "core/winstr.h"
#include "render/track_state.h"

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
                               core::Rules rules)
    : entry_(std::move(entry)),
      pro_(pro),
      bass2x_(bass2x),
      difficulty_(difficulty),
      path_(std::move(path)),
      sp_cap_(sp_cap),
      rules_(std::move(rules)) {}

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

    // The bar's byte total: a loose file's size on disk, a container stem's
    // extracted bytes.
    std::vector<uint64_t> sizes;
    sizes.reserve(stems.size());
    uint64_t total = 0;
    for (const app::PreviewAudioStem& s : stems) {
        uint64_t n = 0;
        if (s.from_file()) {
            try {
                n = file_size_bytes(s.path);
            } catch (const std::exception&) {
                n = 0;  // the open below fails too and skips this stem
            }
        } else {
            n = s.bytes.size();
        }
        sizes.push_back(n);
        total += n;
    }
    bytes_total_.store(total);

    std::vector<std::unique_ptr<audio::StemReader>> readers;
    uint64_t before = 0;  // bytes of the stems already opened
    for (std::size_t i = 0; i < stems.size(); ++i) {
        app::PreviewAudioStem& s = stems[i];
        const uint64_t size = sizes[i];
        try {
            audio::StemBytes bytes;
            if (s.from_file())
                bytes.mapped = audio::MappedFile::open(s.path);
            else
                bytes.owned = std::move(s.bytes);
            // The Opus index reports every 4 MB; returning false stops it there.
            readers.push_back(audio::open_stem_reader(
                std::move(bytes), [this, &keep_going, before, size](uint64_t done, uint64_t) {
                    bytes_done_.store(before + std::min(done, size));
                    return keep_going();
                }));
        } catch (const audio::OpenCancelled&) {
            throw JobCancelled{};
        } catch (const std::exception&) {
            // A stem that won't open is skipped, so one unreadable or corrupt
            // stem never silences the rest of the chart.
        }
        before += size;
        bytes_done_.store(before);
        if (!keep_going()) throw JobCancelled{};
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

        // Branch (a) here: notes, scene, highway.
        app::PreviewSong ps =
            app::resolve_preview_song(entry_.notespath, container, pro_, bass2x_, difficulty_, rules_);
        container.reset();
        // A chart with no charting at this difficulty would otherwise build an
        // empty scene and the tab would show a blank highway with no reason
        // given. Throwing here surfaces it as "Preview failed: ...", the same
        // wording analysis uses.
        require_notes(ps.song, difficulty_, pro_);
        reading_done_.store(true);
        throw_if_cancelled();
        const Path* path = path_ ? &*path_ : nullptr;
        app::PreviewScene scene = app::build_preview_scene(ps.song, path, sp_cap_, rules_);
        scene_done_.store(true);
        throw_if_cancelled();
        // The highway timeline, built here so the UI thread only uploads it.
        // The pro-drums setting that picked the drum track also picks how the
        // pads draw (cymbals or all toms), as the controller would.
        render::TrackStateOptions track_opts;
        track_opts.pro = pro_;
        render::TrackState track_state = render::build_track_state(scene, track_opts);
        highway_done_.store(true);

        // Both branches done: mix. get() rethrows the audio branch's cancel.
        std::vector<std::unique_ptr<audio::StemReader>> readers = audio_branch.get();
        throw_if_cancelled();
        // The chart sync rule: audio_ms = chart_ms + audio_offset_ms. A
        // negative offset means the chart starts before the audio and the
        // playhead can't seek below 0, so it becomes silence in front of the
        // stems (rounded to whole frames) and the offset becomes 0.
        double offset_ms = ps.audio_offset_ms;
        int64_t front_pad = 0;
        if (offset_ms < 0.0) {
            front_pad = static_cast<int64_t>(std::llround(-offset_ms * kOutRate / 1000.0));
            offset_ms = 0.0;
        }
        auto mix = std::make_unique<audio::StreamMix>(std::move(readers), kOutRate, kOutChannels,
                                                      front_pad);
        result_ = Result{std::move(scene),     std::move(mix),         offset_ms,
                         std::move(ps.song),   std::move(track_state), track_opts};
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
        case Step::Opening: {
            // A zero total (no stems, empty files) stays at the slice's start:
            // never a 0/0 NaN into ImGui::ProgressBar (ImGui issue #7451).
            const float part =
                bytes_total > 0
                    ? static_cast<float>(static_cast<double>(std::min(bytes_done, bytes_total)) /
                                         static_cast<double>(bytes_total))
                    : 0.0f;
            return kReadShare + kOpenShare * part;
        }
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

std::string PreviewLoadJob::Progress::time_left_text() const {
    if (step != Step::Opening || elapsed_s < 3.0 || !(time_left_s >= 0.0)) return "";
    if (time_left_s < 59.5) {
        const long long s = std::max(1LL, static_cast<long long>(std::ceil(time_left_s)));
        return "about " + std::to_string(s) + " s left";
    }
    const long long m = std::max(1LL, std::llround(time_left_s / 60.0));
    return "about " + std::to_string(m) + " min left";
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
