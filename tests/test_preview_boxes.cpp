// Tests for app/preview_view's overlay boxes: the time box, the score box, the
// drain box, the next activation box, the scrub marks and the playhead steps
// the Preview tab draws over the highway. The scene itself has its own file
// (test_preview_scene.cpp), and so does the SP meter (test_sp_meter_curve.cpp);
// the shared hand-built songs live in preview_song_fixtures.h.

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/config.h"  // Settings: the app's default analysis settings
#include "app/preview_view.h"
#include "app/path_view.h"
#include "app/song_length.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/squeeze_rating.h"
#include "core/stars.h"
#include "core/strutil.h"  // ends_with
#include "core/timing.h"  // sp_bars_to_measures
#include "analyzed_chart.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "preview_song_fixtures.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"
#include "ui/preview_panel.h"  // kStepMs, the step the Preview's buttons take

using namespace hydra;
using namespace hydra::app;
using namespace hydra::preview_fixtures;

namespace {

// The detail line's separator: a middle dot, U+00B7, in UTF-8.
const std::string kDot = "\xC2\xB7";

// The overlay boxes at `now_ms` with no end to playback, so shown_ms leaves
// the playhead where it is. The cases that pin what a playhead past the end
// reads call the builders with a length.
constexpr double kNoPlaybackEnd = std::numeric_limits<double>::infinity();
PreviewScoreBox score_box_at(const PreviewScene& sc, double now_ms) {
    return build_score_box(sc, now_ms, kNoPlaybackEnd);
}
PreviewDrainBox drain_box_at(const PreviewScene& sc, double now_ms) {
    return build_drain_box(sc, now_ms, kNoPlaybackEnd);
}
PreviewNextActBox next_act_box_at(const PreviewScene& sc, double now_ms, bool pro_drums) {
    return build_next_act_box(sc, now_ms, kNoPlaybackEnd, pro_drums);
}
PreviewSoloBox solo_box_at(const PreviewScene& sc, double now_ms) {
    return build_solo_box(sc, now_ms, kNoPlaybackEnd);
}

// Two solos close together on a 120 BPM grid at 480 ticks a beat (tick t is
// t*500/480 ms): a chord every 240 ticks from tick 0 to 2400, each a Red, with
// a Kick added on every third chord (ticks 0, 720, 1440, 2160) so a chord's
// notes and the chords differ. The first solo is the chords at ticks 480, 720
// and 960 (500, 750 and 1000 ms: 1 + 2 + 1 notes); the second the chords at
// 1440, 1680 and 1920 (1500, 1750 and 2000 ms: 2 + 1 + 1 notes). The second
// starts 500 ms after the first ends, inside its results.
Song two_solo_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int64_t t = 0; t <= 2400; t += 240) {
        Chord c;
        c.add_note(NoteColor::Red);
        if ((t / 240) % 3 == 0) c.add_note(NoteColor::Kick);
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord = std::move(c);
        ts.flag_solo = (t >= 480 && t <= 960) || (t >= 1440 && t <= 1920);
        song.sequence.push_back(std::move(ts));
    }
    // Sequence positions 2..4 and 6..8, as find_solo_sections would build them.
    song.solo_sections = {{2, 4}, {6, 8}};
    return song;
}

// A time signature written into a chart: from `tick`, numerator/denominator.
struct SigAt {
    int64_t tick;
    int numerator;
    int denominator;
};

// 120 BPM at 480 ticks a quarter, 4/4 from tick 0 then `sigs`, and one red
// note at `note_tick` so the grid reaches past the changes.
Song meter_song(std::initializer_list<SigAt> sigs, int64_t note_tick) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    for (const SigAt& s : sigs) apply_timesig(song, s.tick, s.numerator, s.denominator);
    song.build_timing();
    Chord c;
    c.add_note(NoteColor::Red);
    SongTimestamp ts;
    ts.timecode = song.timecode(note_tick);
    ts.chord = std::move(c);
    song.sequence.push_back(std::move(ts));
    return song;
}

// A corpus chart's song (testdata/input), read by the parser the app uses.
const Song& corpus_song(const std::string& name) {
    const std::string path = corpus::chart_named(name);
    REQUIRE_MESSAGE(!path.empty(), "no corpus chart named " << name);
    return corpus::song(path, ChartReading{});
}

// A position in the shape format_measure (core/timing.h) prints, for an
// expected measure, beat and tick counted from somewhere other than the box.
std::string grid_label(int measure, int beat, int64_t ticks_in) {
    return "m" + std::to_string(measure) + "." + std::to_string(beat) + "." +
           std::to_string(ticks_in);
}

// The signature as the time box's tempo line ends with it.
std::string sig_text(const std::pair<int, int>& sig) {
    return std::to_string(sig.first) + "/" + std::to_string(sig.second);
}

// The drawn measure lines of a scene, in order.
std::vector<PreviewBeat> drawn_measure_lines(const PreviewScene& scene) {
    std::vector<PreviewBeat> lines;
    for (const PreviewBeat& b : scene.beats)
        if (b.kind == PreviewBeatKind::MeasureLine) lines.push_back(b);
    return lines;
}

// A real song's length and one playhead on it: the last note of My Own Summer
// and the chord halfway through its notes, for the step cases.
struct RealPlayhead {
    PreviewScene scene;
    double len = 0.0;
    double chord_ms = 0.0;
};
RealPlayhead real_playhead() {
    const Song& song = corpus_song("Deftones - My Own Summer");
    RealPlayhead p;
    p.scene = build_preview_scene(song, nullptr);
    p.len = last_note_ms(p.scene);
    REQUIRE_FALSE(song.sequence.empty());
    p.chord_ms =
        song.timing().ms_index().at(song.sequence[song.sequence.size() / 2].timecode.ticks());
    // The cases step a whole kStepMs either way from the chord.
    REQUIRE(p.chord_ms - ui::kStepMs > 0.0);
    REQUIRE(p.chord_ms + ui::kStepMs < p.len);
    return p;
}

// The score box's detail line for a multiplier and combo.
std::string score_detail(int multiplier, int combo) {
    return "x" + std::to_string(multiplier) + " " + kDot + " combo " + std::to_string(combo);
}

// The replay chord struck at `ms` (struck_at), or none before the first.
const ReplayChord* struck_chord(const ReplayResult& r, double ms) {
    const ReplayChord* at = nullptr;
    for (const ReplayChord& c : r.chords)
        if (struck_at(ms, c.ms)) at = &c;
    return at;
}

}  // namespace

TEST_CASE("build_time_box: the position counts the drawn grid on a real chart") {
    // Entertain Me changes meter over a hundred times (5/4, 15/16 and back),
    // so its grid restarts often. At every line the highway draws, the box's
    // measure and beat are the count of drawn measure and beat lines so far,
    // a half line sits its distance from the beat line before it, and the
    // signature is the grid's own at that tick.
    const Song& song = corpus_song("Tigran Hamasyan - Entertain Me");
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    REQUIRE_FALSE(scene.beats.empty());
    REQUIRE(scene.beats.front().kind == PreviewBeatKind::MeasureLine);
    const double len = scene.beats.back().ms;

    int measure = 0;
    int beat = 0;
    int64_t beat_tick = 0;
    size_t wrong = 0;
    for (const PreviewBeat& b : scene.beats) {
        if (b.kind == PreviewBeatKind::MeasureLine) {
            ++measure;
            beat = 1;
            beat_tick = b.tick;
        } else if (b.kind == PreviewBeatKind::Beat) {
            ++beat;
            beat_tick = b.tick;
        }
        const PreviewTimeBox box = build_time_box(scene, b.ms, len);
        const std::string want = grid_label(measure, beat, b.tick - beat_tick);
        const std::string want_sig = sig_text(scene.timing->time_signature_at(b.tick));
        if (box.position != want || !ends_with(box.tempo, want_sig)) {
            // Only the first disagreement is printed; the count says how many.
            if (wrong++ == 0) {
                CHECK(box.position == want);
                CHECK_MESSAGE(ends_with(box.tempo, want_sig), box.tempo << " lacks " << want_sig);
            }
        }
    }
    CHECK(wrong == 0);

    // The length line reads where playback ends the way the position reads
    // the playhead, and a playhead past the end shows the end.
    const PreviewTimeBox at_end = build_time_box(scene, len, len);
    CHECK(build_time_box(scene, 0.0, len).length == at_end.position);
    const PreviewTimeBox past = build_time_box(scene, len + 1000.0, len);
    CHECK(past.timestamp == at_end.timestamp);
    CHECK(past.position == at_end.position);
}

TEST_CASE("build_time_box: the end measure runs past the last beat line") {
    // Past the drawn grid the last tempo and meter hold: a playhead a whole
    // number of measures past the last measure line reads that many measures
    // on, on the first beat.
    const Song& song = corpus_song("Deftones - My Own Summer");
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    const std::vector<PreviewBeat> lines = drawn_measure_lines(scene);
    REQUIRE_FALSE(lines.empty());
    const PreviewBeat& last = lines.back();

    const int measures_on = 3;  // a free input: any count that lands past the grid
    const double ms = last.ms + measures_on * scene.timing->ms_per_measure_at(last.tick);
    REQUIRE(ms > scene.beats.back().ms);
    const PreviewTimeBox box = build_time_box(scene, ms, ms);
    CHECK(box.position == grid_label(static_cast<int>(lines.size()) + measures_on, 1, 0));
    CHECK(box.length == box.position);
}

TEST_CASE("build_time_box: the practice section in force") {
    // My Own Summer's chart names its practice sections. The box names the
    // section whose tick is at or before the playhead's, from its own tick on,
    // and nothing before the first.
    const Song& song = corpus_song("Deftones - My Own Summer");
    const std::vector<SongSection>& sections = song.practice_sections;
    REQUIRE(sections.size() >= 2);
    const PreviewScene scene = build_preview_scene(song, nullptr);
    const MsIndex& ms = song.timing().ms_index();

    // The scene carries the chart's sections as parsed.
    REQUIRE(scene.sections.size() == sections.size());
    for (size_t i = 0; i < sections.size(); ++i) {
        CHECK(scene.sections[i].tick == sections[i].tick);
        CHECK(scene.sections[i].name == sections[i].name);
        CHECK(scene.sections[i].ms == doctest::Approx(ms.at(sections[i].tick)));
    }

    const double len = std::max(scene.beats.back().ms, ms.at(sections.back().tick)) + 1.0;
    for (size_t i = 0; i < sections.size(); ++i) {
        const SongSection& s = sections[i];
        CAPTURE(s.name);
        CHECK(build_time_box(scene, ms.at(s.tick), len).section_line == "Section " + s.name);
        if (s.tick == 0) continue;
        const std::string before = i == 0 ? std::string{} : "Section " + sections[i - 1].name;
        CHECK(build_time_box(scene, ms.at(s.tick - 1), len).section_line == before);
    }
}

TEST_CASE("build_time_box: a mid-measure meter change restarts the grid") {
    // No corpus chart writes a meter change inside a measure, so this chart
    // is written here. Clone Hero's grid restarts a measure at such a change
    // (docs/clone-hero/code/6142/grid_restart_20DACA0.md; Hydra's MeasureIndex),
    // and the box and the drawn lines both read that grid.
    const int64_t beat = 480;  // meter_song's ticks a quarter
    const int64_t change = 4 * beat + 2 * beat;  // the 4/4 second measure's third beat
    const int64_t playhead = change + beat + beat / 2;  // half a beat into the 3/4 measure's second beat
    const Song song = meter_song({{change, 3, 4}}, playhead);
    PreviewScene scene = build_preview_scene(song, nullptr);

    // The lines: the first 4/4 measure, the cut-short second, then 3/4 from
    // the change.
    const std::vector<PreviewBeat> lines = drawn_measure_lines(scene);
    REQUIRE(lines.size() >= 4);
    CHECK(lines[0].tick == 0);
    CHECK(lines[1].tick == 4 * beat);
    CHECK(lines[2].tick == change);
    CHECK(lines[3].tick == change + 3 * beat);

    // The box puts the playhead in the measure the third line starts.
    const PreviewTimeBox box =
        build_time_box(scene, song.timing().ms_index().at(playhead), 1e9);
    CHECK(box.position == grid_label(3, 2, beat / 2));
    CHECK(ends_with(box.tempo, sig_text({3, 4})));
}

TEST_CASE("build_time_box: the time signature in force, as the chart wrote it") {
    // Stabwound is the corpus chart that writes 6/8 (its .chart's "TS 6 3").
    // 6/8 and 3/4 measures are the same length, so only the written signature
    // tells them apart. On every 6/8 measure line the box shows 6/8, and the
    // next beat line is an eighth note on (half a quarter), its second beat.
    const Song& song = corpus_song("Necrophagist - Stabwound");
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    const double len = scene.beats.back().ms;
    const int64_t eighth = song.tick_resolution() / 2;

    int six_eight_lines = 0;
    for (size_t i = 0; i < scene.beats.size(); ++i) {
        const PreviewBeat& line = scene.beats[i];
        if (line.kind != PreviewBeatKind::MeasureLine) continue;
        if (scene.timing->time_signature_at(line.tick) != std::pair<int, int>{6, 8}) continue;
        ++six_eight_lines;
        CHECK(ends_with(build_time_box(scene, line.ms, len).tempo, "6/8"));
        size_t j = i + 1;
        while (j < scene.beats.size() && scene.beats[j].kind == PreviewBeatKind::Half) ++j;
        if (j == scene.beats.size() || scene.beats[j].kind != PreviewBeatKind::Beat) continue;
        CHECK(scene.beats[j].tick - line.tick == eighth);
        CHECK(ends_with(build_time_box(scene, scene.beats[j].ms, len).position, ".2.0"));
    }
    // The chart's 6/8 reached the grid: without it nothing above ran.
    CHECK(six_eight_lines > 0);
}

TEST_CASE("step_shown_ms: a step moves the shown moment by exactly its size") {
    // A real song's length and a real chord's moment, and the step the
    // Preview's buttons take.
    const RealPlayhead p = real_playhead();
    CHECK(step_shown_ms(p.chord_ms, p.len, ui::kStepMs) == doctest::Approx(p.chord_ms + ui::kStepMs));
    CHECK(step_shown_ms(p.chord_ms, p.len, -ui::kStepMs) == doctest::Approx(p.chord_ms - ui::kStepMs));
    // A step of 0 lands on the shown moment itself.
    CHECK(step_shown_ms(p.chord_ms, p.len, 0.0) == doctest::Approx(p.chord_ms));
}

TEST_CASE("step_shown_ms: never before 0 or past where playback ends") {
    const RealPlayhead p = real_playhead();
    const double half = ui::kStepMs / 2.0;
    CHECK(step_shown_ms(half, p.len, -ui::kStepMs) == 0.0);
    CHECK(step_shown_ms(0.0, p.len, -ui::kStepMs) == 0.0);
    CHECK(step_shown_ms(p.len - half, p.len, ui::kStepMs) == p.len);
    CHECK(step_shown_ms(p.len, p.len, ui::kStepMs) == p.len);
}

TEST_CASE("step_shown_ms: a playhead outside the song steps from the moment shown") {
    // Past the end the time box shows the end, and the step starts there.
    const RealPlayhead p = real_playhead();
    const double outside = 100.0 * ui::kStepMs;  // a free input: well past either edge
    CHECK(build_time_box(p.scene, p.len + outside, p.len).timestamp ==
          build_time_box(p.scene, p.len, p.len).timestamp);
    CHECK(step_shown_ms(p.len + outside, p.len, -ui::kStepMs) ==
          doctest::Approx(p.len - ui::kStepMs));
    // Before 0 the time box shows 0, and the step starts there.
    CHECK(build_time_box(p.scene, -outside, p.len).timestamp ==
          build_time_box(p.scene, 0.0, p.len).timestamp);
    CHECK(step_shown_ms(-outside, p.len, ui::kStepMs) == doctest::Approx(ui::kStepMs));
}

TEST_CASE("score box: no path hides the box") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(scene.score.state == PreviewScore::State::None);
    CHECK(scene.score.steps.empty());
    CHECK_FALSE(score_box_at(scene, 600.0).shown);
}

TEST_CASE("score box: a solo's bonus lands on its last note") {
    // Hand song: chords at 0, 250, 500 and 750 ms. The solo is the chords at
    // 250 and 500 ms, so its bonus is withheld at 250 and paid at 500.
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == 4);

    const ReplayResult r = replay_path(song, {});
    REQUIRE(r.chords[1].points.solo > 0);
    // replay_path owns the withheld total; 180 was pinned from one run as a
    // fixed oracle beside it.
    CHECK(scene.score.steps[1].total == r.chords[1].cum_onscreen_total);
    CHECK(scene.score.steps[1].total == 180);
    CHECK(scene.score.steps[2].total == r.chords[2].cum.total());
    CHECK(scene.score.steps[3].total == path.totalscore());

    // Between the solo's two chords the box shows the withheld total.
    PreviewScoreBox mid = score_box_at(scene, 400.0);
    CHECK(mid.shown);
    CHECK(mid.available);
    CHECK(mid.score == group_thousands(scene.score.steps[1].total));
    CHECK(mid.detail == "x1 " + kDot + " combo 2");

    // A chord exactly at the playhead counts as hit.
    PreviewScoreBox on = score_box_at(scene, scene.score.steps[2].ms);
    CHECK(on.score == group_thousands(scene.score.steps[2].total));
    CHECK(on.detail == "x1 " + kDot + " combo 3");
}

TEST_CASE("score box: before the first note nothing is hit yet") {
    // The fill song's first note is at tick 480, 500 ms.
    Song song = make_fill_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    PreviewScoreBox box = score_box_at(scene, 100.0);
    CHECK(box.shown);
    CHECK(box.available);
    CHECK(box.score == "0");
    CHECK(box.detail == "x1 " + kDot + " combo 0");
}

TEST_CASE("struck_at: a chord exactly on the playhead counts as hit") {
    // 2500 ms is the activation chord (tick 2400) of the make_sp_song({1920},
    // 9600) score-box case below: a jump to that activation lands on it.
    CHECK(struck_at(2500.0, 2500.0));
    CHECK_FALSE(struck_at(2499.9, 2500.0));
    CHECK(struck_at(2500.1, 2500.0));
}

TEST_CASE("score box: the multiplier is the replay's, doubled on chords Star Power pays") {
    // The analyzed corpus chart's best path, as the engine found it.
    const AnalysisResult& a = corpus::analyzed_by_ten_scores();
    const Path& best = a.record.best_path();
    const PreviewScene scene = build_preview_scene(a.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE_FALSE(scene.activations.empty());

    // Each step carries exactly what the replay says the disc shows.
    const ReplayResult r = replay_stored_path(a.song, best).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    for (size_t i = 0; i < r.chords.size(); ++i) {
        CHECK(scene.score.steps[i].multiplier == r.chords[i].multiplier_shown);
        // And the plain disc, which the box shows once SP has ended.
        CHECK(scene.score.steps[i].multiplier_plain == r.chords[i].multiplier_after);
    }

    // Around each activation's SP end: a millisecond before it the box shows
    // the doubled disc of the chord struck then, which Star Power paid; from
    // the end on it shows that chord's plain disc.
    int ends_checked = 0;
    for (const PreviewActivation& act : scene.activations) {
        if (!act.has_sp_end) continue;
        const double before = act.sp_end_ms - 1.0;
        REQUIRE(before > act.ms);
        const ReplayChord* in = struck_chord(r, before);
        REQUIRE(in != nullptr);
        CHECK(in->in_sp);
        CHECK(in->multiplier_shown == in->multiplier_after * kStarPowerMultiplier);
        CHECK(score_box_at(scene, before).detail ==
              score_detail(in->multiplier_shown, in->combo_after));
        const ReplayChord* at_end = struck_chord(r, act.sp_end_ms);
        REQUIRE(at_end != nullptr);
        CHECK(score_box_at(scene, act.sp_end_ms).detail ==
              score_detail(at_end->multiplier_after, at_end->combo_after));
        ++ends_checked;
    }
    CHECK(ends_checked > 0);
}

TEST_CASE("score box: past the length it reads the moment the time box shows") {
    // The fixture above: 2000 ms is five notes in, 7000 ms fifteen.
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);

    CHECK(build_score_box(scene, 7000.0, /*length_ms=*/2000.0).detail ==
          "x1 " + kDot + " combo 5");
    CHECK(build_score_box(scene, 7000.0, /*length_ms=*/7000.0).detail ==
          "x2 " + kDot + " combo 15");
}

TEST_CASE("score box: a chord exactly on the playhead counts as hit") {
    // The case above: "Act >" from the start lands on the activation chord at
    // 2500 ms, and the box counts that chord as hit and paid (D48, Q27). This
    // case guards the shared rule: make struck_at exclusive and it reads
    // "x1 · combo 5".
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    const std::optional<double> jump = activation_jump_ms(scene, 0.0, +1);
    REQUIRE(jump.has_value());
    CHECK(score_box_at(scene, *jump).detail == "x2 " + kDot + " combo 6");
}

TEST_CASE("score box: a leeway chord past the SP end is paid doubled, the box stays plain") {
    // Every chord, on every path the engine found for the analyzed corpus
    // chart, that the replay paid in Star Power although it lies past its
    // activation's SP end (inside backend_leeway_ms, the counted-backend
    // leeway in core::Rules). The score pays it doubled; the box, at the
    // chord's own instant, shows the plain disc, because SP has ended.
    const AnalysisResult& a = corpus::analyzed_by_ten_scores();
    const double leeway_ms = core::default_rules().backend_leeway_ms;
    int leeway_chords = 0;
    for (const Path& path : a.record.paths) {
        const PreviewScene scene = build_preview_scene(a.song, &path);
        if (scene.score.state != PreviewScore::State::Ready) continue;
        const ReplayResult r = replay_stored_path(a.song, path).result;
        REQUIRE(r.chords.size() == scene.score.steps.size());
        for (size_t i = 0; i < r.chords.size(); ++i) {
            const ReplayChord& c = r.chords[i];
            if (!c.in_sp) continue;
            // The activation whose SP end this chord lies just past.
            const auto past = std::find_if(
                scene.activations.begin(), scene.activations.end(),
                [&](const PreviewActivation& act) {
                    return act.has_sp_end && c.ms > act.sp_end_ms && act.ms < act.sp_end_ms &&
                           c.ms - act.sp_end_ms < leeway_ms;
                });
            if (past == scene.activations.end()) continue;
            ++leeway_chords;
            CHECK(c.multiplier_shown == c.multiplier_after * kStarPowerMultiplier);
            CHECK(scene.score.steps[i].multiplier == c.multiplier_shown);
            CHECK(scene.score.steps[i].multiplier_plain == c.multiplier_after);
            CHECK(score_box_at(scene, c.ms).detail ==
                  score_detail(c.multiplier_after, c.combo_after));
        }
    }
    // The chart's paths must hold such a chord, or nothing above ran.
    CHECK_MESSAGE(leeway_chords > 0,
                  "no path of the analyzed chart has a chord paid past its SP end");
}

TEST_CASE("score box: a path the replay can't reproduce says so") {
    Song song = make_hand_song();

    // The stored score disagrees with the replay by one point.
    Path wrong = priced_path(song, {});
    wrong.notes_points += 1;
    PreviewScene a = build_preview_scene(song, &wrong);
    CHECK(a.score.state == PreviewScore::State::Unavailable);
    PreviewScoreBox box = score_box_at(a, 600.0);
    CHECK(box.shown);
    CHECK_FALSE(box.available);
    CHECK(box.score == "Score unavailable");
    CHECK(box.detail.empty());

    // An activation with no deactivation node yields no window, so the replay
    // can't stand for the path.
    Path old;
    old.activations.push_back(act_at(song, 720, {}));
    PreviewScene b = build_preview_scene(song, &old);
    CHECK(b.score.state == PreviewScore::State::Unavailable);
    CHECK(score_box_at(b, 600.0).score == "Score unavailable");
}

TEST_CASE("score box: the analyzed chart ends on the path's total") {
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
    const Path& best = r.record.best_path();
    PreviewScene scene = build_preview_scene(r.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == r.song.sequence.size());
    CHECK(scene.score.steps.back().total == best.totalscore());
    for (size_t i = 1; i < scene.score.steps.size(); ++i) {
        CHECK(scene.score.steps[i].ms >= scene.score.steps[i - 1].ms);
        CHECK(scene.score.steps[i].combo > scene.score.steps[i - 1].combo);
    }
    PreviewScoreBox end = score_box_at(scene, last_note_ms(scene));
    CHECK(end.score == group_thousands(best.totalscore()));
}

// ---- the score box's star counter ---------------------------------------

TEST_CASE("score box stars: before the first chord no star is earned") {
    // The fill song's first note is at tick 480, 500 ms.
    Song song = make_fill_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    PreviewScoreBox box = score_box_at(scene, 100.0);
    CHECK(box.stars == 0);
    CHECK(box.next_star == "star 1 in " + group_thousands(star_cutoffs(path).cutoffs[0]));
    // Pinned from one run.
    CHECK(box.next_star == "star 1 in 20");
}

TEST_CASE("score box stars: the analyzed chart ends on the path's own star count") {
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
    const Path& best = r.record.best_path();
    PreviewScene scene = build_preview_scene(r.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    PreviewScoreBox end = score_box_at(scene, last_note_ms(scene));
    // path_stars is the number the details header shows.
    CHECK(end.stars == path_stars(best));
    CHECK(end.next_star.empty() == (end.stars == kMaxStars));
    // Pinned from one run: this chart's best path earns all seven, so the
    // next-star line is gone.
    CHECK(end.stars == 7);
    CHECK(end.next_star.empty());
}

TEST_CASE("score box stars: a solo's bonus raises the score but not the stars") {
    // Hand song: chords at 0, 250, 500 and 750 ms. The solo is the chords at
    // 250 and 500 ms, so its whole bonus is paid on the chord at 500 ms. The
    // literals below were pinned from one run. This path's cutoffs: 3 stars
    // at 380, 4 at 760 (star_cutoffs).
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    REQUIRE(path.score_solo == 200);
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.cutoffs.cutoffs[2] == 380);

    // Inside the solo: the bonus is still withheld.
    const PreviewScoreBox before = score_box_at(scene, 400.0);
    CHECK(before.score == "180");
    CHECK(before.stars == 1);
    CHECK(before.next_star == "star 2 in 10");

    // The solo's last chord: the on-screen score jumps by that chord's own
    // points plus the whole bonus, and passes the 3-star cutoff. The stars
    // move by the chord's own points alone, so the box still shows 2 stars.
    const PreviewScoreBox after = score_box_at(scene, 500.0);
    CHECK(after.score == "480");
    CHECK(after.stars == 2);
    CHECK(after.next_star == "star 3 in 100");
    const PreviewScoreStep& s1 = scene.score.steps[1];
    const PreviewScoreStep& s2 = scene.score.steps[2];
    CHECK(s2.total - s1.total ==
          (s2.score_without_solo - s1.score_without_solo) + path.score_solo);

    // At the end the box agrees with the path's own count (path_stars), and
    // the gap is still counted without the bonus.
    const PreviewScoreBox end = score_box_at(scene, 1000.0);
    CHECK(end.score == "580");
    CHECK(end.stars == path_stars(path));
    CHECK(end.stars == 3);
    CHECK(end.next_star == "star 4 in 380");
}

TEST_CASE("score box stars: no score, no stars") {
    Song song = make_hand_song();
    PreviewScoreBox none = score_box_at(build_preview_scene(song, nullptr), 600.0);
    CHECK(none.stars == 0);
    CHECK(none.next_star.empty());

    Path wrong = priced_path(song, {});
    wrong.notes_points += 1;
    PreviewScoreBox unavailable = score_box_at(build_preview_scene(song, &wrong), 600.0);
    REQUIRE(unavailable.shown);
    REQUIRE_FALSE(unavailable.available);
    CHECK(unavailable.stars == 0);
    CHECK(unavailable.next_star.empty());
}

// ---- SP drain box -------------------------------------------------------

TEST_CASE("drain box: idle on a chart with no Star Power") {
    // A default scene was built from no song: no gauge, so no box.
    CHECK_FALSE(drain_box_at(PreviewScene{}, 1000.0).shown);
    // Notes but no SP phrase and no path: the gauge is empty, and the box
    // still shows, in its idle form.
    Song song = make_sp_song({}, /*last_tick=*/3840);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.sp_meter.segments.empty());
    const PreviewDrainBox box = drain_box_at(scene, 1000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.header == "SP drain (if activated)");
    CHECK(box.rate == "1 bar / 4.0 s");
    CHECK(box.detail == "full meter 16.0 s");
}

TEST_CASE("drain box: idle at a steady tempo reads the rate and a full meter") {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    PreviewScene scene = build_preview_scene(song, nullptr);

    PreviewDrainBox box = drain_box_at(scene, 2000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.header == "SP drain (if activated)");
    CHECK(box.rate == "1 bar / 4.0 s");
    // Cap 4: eight measures, 16000 ms.
    CHECK(box.detail == "full meter 16.0 s");
}

TEST_CASE("drain box: the rate switches exactly at a tempo change") {
    // 120 BPM until tick 5760 (6000 ms), then 60 BPM: a bar goes 4 s -> 8 s.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(drain_box_at(scene, 5990.0).rate == "1 bar / 4.0 s");
    CHECK(drain_box_at(scene, 6000.0).rate == "1 bar / 8.0 s");

    // "full meter" is the cap at the bar time in force, so it jumps with the
    // rate, at the change and not before: 4 x 4 s, then 4 x 8 s.
    CHECK(drain_box_at(scene, 2000.0).detail == "full meter 16.0 s");
    CHECK(drain_box_at(scene, 5990.0).detail == "full meter 16.0 s");
    CHECK(drain_box_at(scene, 6000.0).detail == "full meter 32.0 s");
}

TEST_CASE("drain box: past the length it reads the moment the time box shows") {
    // The fixture above. A playhead past where playback ends reads the
    // moment the time box shows (shown_ms), so it lands on the length.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{5760, 60.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    const PreviewDrainBox held = build_drain_box(scene, 6000.0, /*length_ms=*/5990.0);
    CHECK(held.rate == "1 bar / 4.0 s");
    CHECK(held.detail == "full meter 16.0 s");
    const PreviewDrainBox at = build_drain_box(scene, 6000.0, /*length_ms=*/6000.0);
    CHECK(at.rate == "1 bar / 8.0 s");
    CHECK(at.detail == "full meter 32.0 s");
}

TEST_CASE("drain box: a 7/8 section drains faster at the same BPM") {
    // 7/8 from tick 3840 (4000 ms, a barline): 1680 ticks = 1750 ms a
    // measure, so a bar lasts 3500 ms instead of 4000.
    Song song = make_sp_song({960}, /*last_tick=*/13440, {}, 480, {{3840, {7, 8}}});
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(drain_box_at(scene, 2000.0).rate == "1 bar / 4.0 s");
    CHECK(drain_box_at(scene, 4000.0).rate == "1 bar / 3.5 s");
    CHECK(drain_box_at(scene, 4000.0).detail == "full meter 14.0 s");
}

TEST_CASE("drain box: active inside the stored SP window, idle outside it") {
    // Two bars at tick 3840 (4000 ms): the stored end is tick 11520 (12000 ms).
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520)};
    PreviewScene scene = build_preview_scene(song, &path);

    PreviewDrainBox before = drain_box_at(scene, 3999.0);
    CHECK_FALSE(before.active);
    CHECK(before.header == "SP drain (if activated)");

    PreviewDrainBox at = drain_box_at(scene, 4000.0);
    CHECK(at.active);
    CHECK(at.header == "SP drain");
    CHECK(at.rate == "1 bar / 4.0 s");
    CHECK(at.detail == "empties in 8.0 s");
    CHECK(drain_box_at(scene, 9000.0).detail == "empties in 3.0 s");

    // At the stored end SP is over, as the gauge reads that boundary too.
    PreviewDrainBox after = drain_box_at(scene, 12000.0);
    CHECK_FALSE(after.active);
    CHECK(after.detail.rfind("full meter ", 0) == 0);
}

TEST_CASE("drain box: empties in reads the stored end, not a recount") {
    // A phrase collected mid-SP pushes the stored end two measures past the
    // plain act + 4 measures: 16000 ms, not 12000. Only the record knows.
    Song song = make_sp_song({960, 5760}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2, /*end_tick=*/11520);
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);

    PreviewDrainBox box = drain_box_at(scene, 12000.0);
    CHECK(box.active);
    CHECK(box.detail == "empties in 4.0 s");
}

TEST_CASE("drain box: an activation with no stored end stays idle") {
    // The analyzed corpus chart's best path, with its first activation's
    // stored SP end taken away, as a record without SP end steps holds it.
    // Nothing then says SP is running inside that activation.
    const AnalysisResult& a = corpus::analyzed_by_ten_scores();
    const Path& best = a.record.best_path();
    const PreviewScene with_end = build_preview_scene(a.song, &best);
    REQUIRE_FALSE(with_end.activations.empty());
    REQUIRE(with_end.activations[0].has_sp_end);
    // A moment inside the first activation's window, where the full record
    // shows SP draining.
    const double inside = (with_end.activations[0].ms + with_end.activations[0].sp_end_ms) / 2.0;
    REQUIRE(drain_box_at(with_end, inside).active);

    Path no_end = best;
    no_end.activations[0].sp_end_steps.clear();
    const PreviewScene scene = build_preview_scene(a.song, &no_end);
    REQUIRE_FALSE(scene.activations[0].has_sp_end);

    // At that moment the box is idle, and reads what it reads with no path
    // at all: the idle drain line.
    const PreviewScene no_path = build_preview_scene(a.song, nullptr);
    const PreviewDrainBox idle = drain_box_at(no_path, inside);
    REQUIRE(idle.shown);
    REQUIRE_FALSE(idle.active);
    const PreviewDrainBox box = drain_box_at(scene, inside);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.header == idle.header);
    CHECK(box.detail == idle.detail);
}

TEST_CASE("solo_percent: the game's float32 truncation") {
    // 53 / 100 in float is just under 0.53, and times 100 just under 53, so
    // the game shows 52 where whole-number math gives 53.
    CHECK(solo_percent(53, 100) == 52);
    CHECK(solo_percent(0, 120) == 0);
    CHECK(solo_percent(102, 120) == 85);
    CHECK(solo_percent(1, 3) == 33);
    CHECK(solo_percent(120, 120) == 100);
    CHECK(solo_percent(0, 0) == 0);
}

TEST_CASE("solo box: from before the solo to its bonus, timed off the last chord") {
    // Hand song: the solo is the one-note chords at 250 and 500 ms.
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    REQUIRE(path.score_solo == 200);
    const PreviewScene scene = build_preview_scene(song, &path);

    // Before the first chord: hidden.
    CHECK_FALSE(solo_box_at(scene, 249.9).shown);

    // On the first chord and between the two: one of two notes hit.
    for (double t : {250.0, 400.0}) {
        const PreviewSoloBox b = solo_box_at(scene, t);
        CHECK(b.shown);
        CHECK(b.headline == "50%");
        CHECK(b.detail == "1 / 2 notes");
    }

    // On the last chord the solo is over: the end message, no count.
    const PreviewSoloBox end = solo_box_at(scene, 500.0);
    CHECK(end.shown);
    CHECK(end.headline == "Perfect Solo!");
    CHECK(end.detail.empty());
    CHECK(solo_box_at(scene, 500.0 + kSoloResultMs - 0.1).headline == "Perfect Solo!");

    // kSoloResultMs later: the bonus.
    const PreviewSoloBox bonus = solo_box_at(scene, 500.0 + kSoloResultMs);
    CHECK(bonus.shown);
    CHECK(bonus.headline == "+200");
    CHECK(bonus.detail.empty());
    CHECK(solo_box_at(scene, 500.0 + kSoloTailMs - 0.1).headline == "+200");

    // kSoloTailMs after the last chord: hidden.
    CHECK_FALSE(solo_box_at(scene, 500.0 + kSoloTailMs).shown);
}

TEST_CASE("solo box: the count is notes, the bonus thousands-grouped") {
    const Song song = two_solo_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solo_notes.size() == 2);
    CHECK(scene.solo_notes[0].notes == 4);
    CHECK(scene.solo_notes[1].notes == 4);

    // The first solo's middle chord has two notes: three of four hit.
    const PreviewSoloBox mid = solo_box_at(scene, 750.0);
    CHECK(mid.headline == "75%");
    CHECK(mid.detail == "3 / 4 notes");
    CHECK(solo_box_at(scene, 500.0).headline == "25%");
    CHECK(solo_box_at(scene, 500.0).detail == "1 / 4 notes");
}

TEST_CASE("solo box: a solo that starts during the last one's results takes over") {
    // The second solo starts while the first one's results still show. The
    // expected boxes are each solo's own, as build_solo_box_samples gives
    // them: per solo its first chord, its second-to-last chord, its last
    // chord (the end message) and its bonus.
    const Song song = two_solo_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solo_notes.size() == 2);
    const PreviewSoloNotes& first = scene.solo_notes[0];
    const PreviewSoloNotes& second = scene.solo_notes[1];
    REQUIRE(first.steps.size() >= 2);
    REQUIRE(second.steps.size() >= 2);
    const std::vector<PreviewSoloBox> samples = build_solo_box_samples(scene);
    REQUIRE(samples.size() == 8);
    const PreviewSoloBox& first_end = samples[2];
    const PreviewSoloBox& second_start = samples[4];
    const PreviewSoloBox& second_end = samples[6];
    const PreviewSoloBox& second_bonus = samples[7];
    const double second_ms = second.steps.front().ms;
    const double second_last_ms = second.steps.back().ms;
    // The case: the second solo starts inside the first one's results.
    REQUIRE(second_ms < first.steps.back().ms + kSoloTailMs);

    // Just before the second solo starts, the first one's results show.
    const PreviewSoloBox before = solo_box_at(scene, std::nextafter(second_ms, 0.0));
    CHECK(before.headline == first_end.headline);
    // On the second solo's first chord the new solo wins: its own count.
    const PreviewSoloBox next = solo_box_at(scene, second_ms);
    CHECK(next.headline == second_start.headline);
    CHECK(next.detail == second_start.detail);
    CHECK(next.headline != before.headline);
    // It runs its own results from its own last chord.
    CHECK(solo_box_at(scene, second_last_ms).headline == second_end.headline);
    CHECK(solo_box_at(scene, second_last_ms + kSoloResultMs).headline == second_bonus.headline);
    CHECK_FALSE(solo_box_at(scene, second_last_ms + kSoloTailMs).shown);
}

TEST_CASE("solo box: the same with or without a path") {
    const Song song = two_solo_song();
    const Path path = priced_path(song, {});
    // Both solos' bonuses together are the path's solo score.
    CHECK(path.score_solo == 800);
    const PreviewScene bare = build_preview_scene(song, nullptr);
    const PreviewScene with_path = build_preview_scene(song, &path);
    for (double t : {0.0, 500.0, 750.0, 1000.0, 1500.0, 2000.0, 3500.0, 5000.0}) {
        const PreviewSoloBox a = solo_box_at(bare, t);
        const PreviewSoloBox b = solo_box_at(with_path, t);
        CHECK(a.shown == b.shown);
        CHECK(a.headline == b.headline);
        CHECK(a.detail == b.detail);
    }
}

TEST_CASE("solo box: a bonus over a thousand has a separator; none without solos") {
    PreviewScene scene;
    PreviewSoloNotes s;
    s.steps = {{100.0, 60}, {200.0, 120}};
    s.notes = 120;
    s.bonus = 12000;
    scene.solo_notes.push_back(s);
    CHECK(solo_box_at(scene, 100.0).detail == "60 / 120 notes");
    CHECK(solo_box_at(scene, 200.0 + kSoloResultMs).headline == "+12,000");

    CHECK_FALSE(solo_box_at(build_preview_scene(make_fill_song(), nullptr), 600.0).shown);
}

TEST_CASE("solo box: past the length it reads the moment the time box shows") {
    // Playback ends on the hand song's last solo chord: the playhead held
    // there shows the end message, not the hidden box 4 s past it.
    const PreviewScene scene = build_preview_scene(make_hand_song(), nullptr);
    CHECK(build_solo_box(scene, 4500.0, /*length_ms=*/500.0).headline == "Perfect Solo!");
    CHECK_FALSE(build_solo_box(scene, 4500.0, kNoPlaybackEnd).shown);
}

TEST_CASE("solo box samples: each solo's widest boxes") {
    const PreviewScene scene = build_preview_scene(two_solo_song(), nullptr);
    const std::vector<PreviewSoloBox> samples = build_solo_box_samples(scene);
    // Per solo: the first chord, the last before the end, the end message
    // and the bonus.
    REQUIRE(samples.size() == 8);
    CHECK(samples[0].headline == "25%");
    CHECK(samples[1].headline == "75%");
    CHECK(samples[1].detail == "3 / 4 notes");
    CHECK(samples[2].headline == "Perfect Solo!");
    CHECK(samples[3].headline == "+400");
    CHECK(samples[4].headline == "50%");
    for (const PreviewSoloBox& b : samples) CHECK(b.shown);
}

TEST_CASE("scrub marks: each activation's onset over the scrubber's length") {
    TwoActs t;
    std::vector<double> marks = build_scrub_marks(t.scene, 10000.0);
    REQUIRE(marks.size() == 2);
    CHECK(marks[0] == doctest::Approx(0.2));
    CHECK(marks[1] == doctest::Approx(0.8));
    // Clamped into the bar when the length is shorter than the path.
    CHECK(build_scrub_marks(t.scene, 4000.0)[1] == doctest::Approx(1.0));
    CHECK(build_scrub_marks(t.scene, 0.0).empty());
    CHECK(build_scrub_marks(build_preview_scene(t.song, nullptr), 10000.0).empty());
}

TEST_CASE("song_fraction: a clamped share, none without a length") {
    // The scrub marks' numbers: 2000 ms of 10,000 ms, 8000 ms of 4000 ms.
    REQUIRE(song_fraction(2000.0, 10000.0).has_value());
    CHECK(*song_fraction(2000.0, 10000.0) == doctest::Approx(0.2));
    REQUIRE(song_fraction(8000.0, 4000.0).has_value());
    CHECK(*song_fraction(8000.0, 4000.0) == doctest::Approx(1.0));
    CHECK_FALSE(song_fraction(2000.0, 0.0).has_value());
}

TEST_CASE("song_fraction: has_song_length takes only a positive length") {
    CHECK(has_song_length(10000.0));
    CHECK_FALSE(has_song_length(0.0));
    CHECK_FALSE(has_song_length(-1.0));
}

// The audio-tail chart: the last note is at 1000 ms and the audio stops 5 s
// later, at 6000 ms. One activation on the note at tick 1440 (750 ms); one
// bar of SP runs two measures, so its window ends at tick 5280. The scrubber's
// marks and the Paths timeline are both placed over the length they are given.
// This test gives both the chart's audio_end_ms, not app::song_length_ms's
// answer, and the two marks must be the same number.
TEST_CASE("scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes") {
    const test::AudioTailChart c = test::audio_tail_chart();
    Path path;
    path.activations = {sp_act_at(c.song, 1440, /*sp_meter=*/1, /*end_tick=*/5280)};
    const PreviewScene scene = build_preview_scene(c.song, &path, kCloneHeroSpCap,
                                                   core::default_rules(), c.audio_end_ms);
    const std::vector<double> marks = build_scrub_marks(scene, c.audio_end_ms);
    REQUIRE(marks.size() == 1);
    CHECK(marks[0] == doctest::Approx(0.125));

    const ActivationsView view =
        build_activations(path, HydraRecord{}, &c.song.timing(), std::nullopt,
                          core::default_rules(), c.audio_end_ms);
    REQUIRE(view.acts.size() == 1);
    REQUIRE(view.acts[0].song_fraction.has_value());
    CHECK(marks[0] == *view.acts[0].song_fraction);
}

// The scrubber's right end is the number it is given (here the chart's
// audio_end_ms), so past the last note the thumb keeps following the playhead,
// all the way to that end.
TEST_CASE("scrub marks: a playhead past the last note parks the thumb at the right end") {
    const test::AudioTailChart c = test::audio_tail_chart();
    const double end = c.audio_end_ms;
    CHECK(end == doctest::Approx(6000.0));
    CHECK(scrub_thumb_ms(c.audio_end_ms, end) == doctest::Approx(6000.0));
    CHECK(scrub_thumb_ms(3000.0, end) == doctest::Approx(3000.0));  // in the tail
    CHECK(scrub_thumb_ms(500.0, end) == doctest::Approx(500.0));
}

TEST_CASE("activation jumps: nearest activation before or after the playhead") {
    TwoActs t;
    CHECK(activation_jump_ms(t.scene, 0.0, +1) == doctest::Approx(2000.0));
    // Parked on an activation, "next" moves on and "previous" goes back past it.
    CHECK(activation_jump_ms(t.scene, 2000.0, +1) == doctest::Approx(8000.0));
    CHECK(activation_jump_ms(t.scene, 2000.3, +1) == doctest::Approx(8000.0));
    CHECK_FALSE(activation_jump_ms(t.scene, 2000.0, -1).has_value());
    CHECK(activation_jump_ms(t.scene, 5000.0, -1) == doctest::Approx(2000.0));
    CHECK(activation_jump_ms(t.scene, 8000.0, -1) == doctest::Approx(2000.0));
    CHECK_FALSE(activation_jump_ms(t.scene, 8000.0, +1).has_value());
    CHECK_FALSE(activation_jump_ms(build_preview_scene(t.song, nullptr), 0.0, +1).has_value());
}

TEST_CASE("next activation box: the activation at or after the playhead") {
    TwoActs t;
    PreviewNextActBox box = next_act_box_at(t.scene, 0.0, /*pro_drums=*/true);
    CHECK(box.shown);
    CHECK(box.header == "Next: activation 1 of 2");
    CHECK(box.detail == "at m2.1.0 " + kDot + " [Red snare]");
    CHECK(next_act_box_at(t.scene, 2000.0, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, 2001.0, true).header == "Next: activation 2 of 2");
    CHECK(next_act_box_at(t.scene, 2001.0, true).detail == "at m5.1.0 " + kDot + " [Red snare]");
    CHECK_FALSE(next_act_box_at(t.scene, 9000.0, true).shown);
    CHECK_FALSE(next_act_box_at(build_preview_scene(t.song, nullptr), 0.0, true).shown);
}

TEST_CASE("next activation box: past the length it reads the moment the time box shows") {
    TwoActs t;
    CHECK(build_next_act_box(t.scene, 9000.0, /*length_ms=*/2000.0, true).header ==
          "Next: activation 1 of 2");
    CHECK_FALSE(build_next_act_box(t.scene, 9000.0, /*length_ms=*/9000.0, true).shown);
}

TEST_CASE("next activation box: the note names follow the Pro Drums setting") {
    // The Dynamics wording (note_label): with Pro Drums off the red pad is
    // plain "Red", with it on "Red snare" (D48, Q11).
    TwoActs t;
    CHECK(next_act_box_at(t.scene, 0.0, /*pro_drums=*/false).detail ==
          "at m2.1.0 " + kDot + " [Red]");
    CHECK(next_act_box_at(t.scene, 0.0, /*pro_drums=*/true).detail ==
          "at m2.1.0 " + kDot + " [Red snare]");
}

TEST_CASE("next activation box: on an activation it names that one, past it the next") {
    // The analyzed corpus chart's best path. The playhead counts as on an
    // activation for the same short span the "Act <" jump uses: once a jump
    // back would land on activation k, the box names the one after it. Each
    // expected header is the box's own reading with the playhead exactly on
    // that activation.
    const AnalysisResult& a = corpus::analyzed_by_ten_scores();
    const Path& best = a.record.best_path();
    const PreviewScene scene = build_preview_scene(a.song, &best);
    const std::vector<PreviewActivation>& acts = scene.activations;
    REQUIRE_FALSE(acts.empty());

    // Before the song the box names the first.
    CHECK(next_act_box_at(scene, -100.0, true).header ==
          next_act_box_at(scene, acts[0].ms, true).header);

    for (size_t k = 0; k < acts.size(); ++k) {
        // Probe moments just after activation k: free inputs on both sides of
        // the span, nothing here says where it ends.
        for (double d : {0.0, 0.25, 0.5, std::nextafter(0.5, 1.0), 1.0, 2.0}) {
            const double t = acts[k].ms + d;
            CAPTURE(k);
            CAPTURE(d);
            const std::optional<double> back = activation_jump_ms(scene, t, -1);
            const bool past_k = back.has_value() && *back == acts[k].ms;
            const PreviewNextActBox box = next_act_box_at(scene, t, true);
            if (!past_k) {
                CHECK(box.header == next_act_box_at(scene, acts[k].ms, true).header);
            } else if (k + 1 < acts.size()) {
                CHECK(box.header == next_act_box_at(scene, acts[k + 1].ms, true).header);
            } else {
                CHECK_FALSE(box.shown);
            }
        }
    }
}

TEST_CASE("next activation box: the notation line is the activation's own, header and detail unchanged") {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Activation a = sp_act_at(song, 1920, /*sp_meter=*/1, /*end_tick=*/5760);
    a.chord.add_note(NoteColor::Red);
    a.e_offset = 300.0;  // not e-critical
    auto box_for = [&song](const Activation& act) {
        Path path;
        path.activations = {act};
        const PreviewScene scene = build_preview_scene(song, &path);
        REQUIRE(scene.activations.size() == 1);
        CHECK(scene.activations[0].notation == act.notationstr_verbose());
        const PreviewNextActBox box = next_act_box_at(scene, 0.0, /*pro_drums=*/true);
        CHECK(box.notation == act.notationstr_verbose());
        CHECK(box.header == "Next: activation 1 of 1");
        CHECK(box.detail == "at m2.1.0 " + kDot + " [Red snare]");
        // Past the last activation the box, and its notation, are gone.
        CHECK(next_act_box_at(scene, 9000.0, true).notation.empty());
        return box;
    };

    // A plain activation shows its bare notation.
    CHECK(box_for(a).notation == "0");

    // One with a SqIn shows the "+" and its timing.
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 100.0});
    CHECK(box_for(a).notation == "0+ (100.0 ms)");

    // An e-critical one (TwoActs' first) leads with the E and its timing.
    TwoActs t;
    const PreviewNextActBox e = next_act_box_at(t.scene, 0.0, true);
    CHECK(e.notation == "E0 (0.0 ms)");
    CHECK(e.notation == t.path.activations[0].notationstr_verbose());
    CHECK(e.header == "Next: activation 1 of 2");
    CHECK(e.detail == "at m2.1.0 " + kDot + " [Red snare]");
}

namespace {

// A chart whose lookups have many entries to search: four tempos, three
// meters (with the signature the chart wrote) and four sections, at 480
// ticks a beat. Tempo 120 BPM to tick 1920, 90 to 4800, 150 to 7680, then 60.
// 4/4, then 3/4 from tick 3840, then 4/4 again from 6720 (both barlines).
// So ticks 960, 1920, 2880, 3840, 4800, 5760, 6720, 7680, 8640 and 9600 are
// 1000, 2000, 3333.3, 4666.7, 6000, 6800, 7600, 8400, 10400 and 12400 ms.
Song make_lookup_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.bpm_changes[1920] = 90.0;
    song.bpm_changes[4800] = 150.0;
    song.bpm_changes[7680] = 60.0;
    apply_timesig(song, 3840, 3, 4);
    apply_timesig(song, 6720, 4, 4);
    song.practice_sections = {{960, "Intro"}, {2880, "Verse"}, {5760, "Chorus"},
                              {8640, "Outro"}};
    song.build_timing();
    for (int64_t t = 0; t <= 9600; t += 960) {
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord.add_note(NoteColor::Red);
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// The time box at `now` as one line: timestamp | position | length | tempo
// line | section line.
std::string time_box_line(const PreviewScene& scene, double now, double length_ms) {
    const PreviewTimeBox b = build_time_box(scene, now, length_ms);
    return b.timestamp + " | " + b.position + " | " + b.length + " | " + b.tempo + " | " +
           b.section_line;
}

// Built lines against pinned ones, printed as literals on a mismatch
// (record_fixtures.h).
using test::check_lines;

}  // namespace

TEST_CASE("build_time_box: every lookup at its boundaries on a chart of many changes") {
    const Song song = make_lookup_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.tempos.size() == 4);
    REQUIRE(scene.measure_spans.size() == 3);
    REQUIRE(scene.sections.size() == 4);
    const double len = 12400.0;

    // Each change at its own moment and a hair before it. The playhead's
    // tick is rounded to the nearest one, so a section, signature or measure
    // shows from half a tick before its tick: at 90 BPM a tick is 1.389 ms,
    // so 3332.7 ms is tick 2879.54 and shows Verse at m2.3.0, while 3332.6 ms
    // is tick 2879.47, still Intro at m2.2.479. Likewise 999.9 ms is tick
    // 959.9 (Intro) and 4666.0 ms tick 3839.5 (3/4). The tempo is looked up
    // by that same tick, so a hair before 2000 ms the box reads tick 1920
    // (m2.1.0) at the new 90 BPM (D48, Q23). In 3/4 a measure is 1440 ticks: measure 3 starts at 3840,
    // 4 at 5280, so tick 5759 (6799 ms at 150 BPM) is m4.1.479. Past the
    // length the playhead is held at 12400 ms, tick 9600, m6.3.0.
    std::vector<std::string> got;
    for (double now : {-500.0, 0.0, 999.9, 1000.0, std::nextafter(2000.0, -1e300), 2000.0,
                       3332.6, 3332.7, 4666.0, 4666.7, std::nextafter(6000.0, -1e300), 6000.0,
                       6799.0, 6800.0, 7600.0, std::nextafter(8400.0, -1e300), 8400.0, 10400.0,
                       20000.0})
        got.push_back(time_box_line(scene, now, len));
    const std::string d = " " + kDot + " ";
    check_lines(got,
                {"0:00.000 / 0:12.400 | m1.1.0 | m6.3.0 | BPM 120.000" + d + "4/4 | ",
                 "0:00.000 / 0:12.400 | m1.1.0 | m6.3.0 | BPM 120.000" + d + "4/4 | ",
                 "0:01.000 / 0:12.400 | m1.3.0 | m6.3.0 | BPM 120.000" + d + "4/4 | Section Intro",
                 "0:01.000 / 0:12.400 | m1.3.0 | m6.3.0 | BPM 120.000" + d + "4/4 | Section Intro",
                 "0:02.000 / 0:12.400 | m2.1.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:02.000 / 0:12.400 | m2.1.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:03.333 / 0:12.400 | m2.2.479 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Intro",
                 "0:03.333 / 0:12.400 | m2.3.0 | m6.3.0 | BPM 90.000" + d + "4/4 | Section Verse",
                 "0:04.666 / 0:12.400 | m3.1.0 | m6.3.0 | BPM 90.000" + d + "3/4 | Section Verse",
                 "0:04.667 / 0:12.400 | m3.1.0 | m6.3.0 | BPM 90.000" + d + "3/4 | Section Verse",
                 "0:06.000 / 0:12.400 | m3.3.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.000 / 0:12.400 | m3.3.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.799 / 0:12.400 | m4.1.479 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Verse",
                 "0:06.800 / 0:12.400 | m4.2.0 | m6.3.0 | BPM 150.000" + d + "3/4 | Section Chorus",
                 "0:07.600 / 0:12.400 | m5.1.0 | m6.3.0 | BPM 150.000" + d + "4/4 | Section Chorus",
                 "0:08.400 / 0:12.400 | m5.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Chorus",
                 "0:08.400 / 0:12.400 | m5.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Chorus",
                 "0:10.400 / 0:12.400 | m6.1.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Outro",
                 "0:12.400 / 0:12.400 | m6.3.0 | m6.3.0 | BPM 60.000" + d + "4/4 | Section Outro"},
                "time box");

    // A negative length shows zero. A length past the last note keeps
    // counting at the last tempo and meter: 60 BPM from tick 9600 (12400
    // ms), so 15000 ms is tick 10848, 288 into measure 7 (from 10560), and
    // 16400 ms is tick 11520, its third beat.
    check_lines({time_box_line(scene, 500.0, -1.0), time_box_line(scene, 15000.0, 16400.0)},
                {"0:00.000 / 0:00.000 | m1.1.0 | m1.1.0 | BPM 120.000" + d + "4/4 | ",
                 "0:15.000 / 0:16.400 | m7.1.288 | m7.3.0 | BPM 60.000" + d + "4/4 | Section Outro"},
                "time box lengths");

    // Sections out of tick order never reach the time box: both parsers sort
    // them (sort_practice_sections, D27 R7.7), and the parser tests pin that.
}

TEST_CASE("build_time_box: inside the half tick before a tempo change every line shows the new values") {
    // make_lookup_song's tempo goes 120 -> 90 BPM at tick 1920 (2000 ms). A
    // hair before 2000 ms the playhead rounds to tick 1920, so the measure
    // line already reads m2.1.0; the BPM line must read that tick's tempo
    // too, not the old one (D48, Q23).
    const Song song = make_lookup_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    const PreviewTimeBox box = build_time_box(scene, std::nextafter(2000.0, -1e300), 12400.0);
    CHECK(box.position == "m2.1.0");
    CHECK(box.tempo == "BPM 90.000 " + kDot + " 4/4");
}
