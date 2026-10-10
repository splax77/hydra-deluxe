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
#include "core/timing.h"  // sp_bars_to_measures
#include "analyzed_chart.h"
#include "corpus_util.h"
#include "parse/song.h"
#include "preview_song_fixtures.h"
#include "record_fixtures.h"
#include "search/engine.h"
#include "search/graph.h"

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

}  // namespace

TEST_CASE("build_time_box: timestamp, measure, tempo") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);

    // 1300 ms is tick 1248: bar 1, third beat, 288 ticks into it. 5000 ms is
    // tick 4800: the third bar's third beat, exactly on the line.
    PreviewTimeBox box = build_time_box(scene, 1300.0, 5000.0);
    CHECK(box.timestamp == "0:01.300 / 0:05.000");
    CHECK(box.position == "m1.3.288");
    CHECK(box.length == "m3.3.0");
    CHECK(box.tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(box.section_line.empty());

    CHECK(build_time_box(scene, 0.0, 5000.0).position == "m1.1.0");

    PreviewTimeBox later = build_time_box(scene, 64000.0, 64000.0);
    CHECK(later.timestamp == "1:04.000 / 1:04.000");

    // The playhead is clamped to the length.
    CHECK(build_time_box(scene, 9999.0, 5000.0).timestamp == "0:05.000 / 0:05.000");
}

TEST_CASE("build_time_box: the timestamp rounds 59,999.6 ms to 1:00.000") {
    // The clock rounds to the whole ms before it splits off the minutes, so
    // it never reads 0:60.000 (D48, Q20).
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    CHECK(build_time_box(scene, 59999.6, 64000.0).timestamp == "1:00.000 / 1:04.000");
}

TEST_CASE("build_time_box: the end measure runs past the last beat line") {
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    // The beat grid stops at tick 4320, but the last tempo and meter hold
    // forever: 30 s is tick 28800, which is bar 16 on the nose.
    REQUIRE(scene.beats.back().tick == 4320);
    PreviewTimeBox box = build_time_box(scene, 30000.0, 30000.0);
    CHECK(box.position == "m16.1.0");
    CHECK(box.length == "m16.1.0");
    CHECK(box.timestamp == "0:30.000 / 0:30.000");
}

TEST_CASE("build_time_box: the practice section in force") {
    Song song = make_hand_song();
    song.practice_sections.push_back({480, "Verse 1"});
    song.practice_sections.push_back({2400, "Chorus"});
    PreviewScene scene = build_preview_scene(song, nullptr);

    REQUIRE(scene.sections.size() == 2);
    CHECK(scene.sections[0].tick == 480);
    CHECK(scene.sections[0].ms == doctest::Approx(500.0));
    CHECK(scene.sections[0].name == "Verse 1");
    // Past the last note (tick 720), where the ms index has no entry.
    CHECK(scene.sections[1].ms == doctest::Approx(2500.0));

    const double len = 10000.0;
    CHECK(build_time_box(scene, 0.0, len).section_line.empty());
    CHECK(build_time_box(scene, 400.0, len).section_line.empty());
    CHECK(build_time_box(scene, 500.0, len).section_line == "Section Verse 1");
    CHECK(build_time_box(scene, 2000.0, len).section_line == "Section Verse 1");
    CHECK(build_time_box(scene, 2500.0, len).section_line == "Section Chorus");
    CHECK(build_time_box(scene, 9000.0, len).section_line == "Section Chorus");
}

TEST_CASE("build_time_box: a mid-measure meter change follows the engine") {
    // 4/4 from tick 0, 3/4 from tick 2880 — beat 3 of the second bar, not a
    // barline. A section's bars count from the last barline at or before its
    // first tick (tick 1920 here), so the bar lines run 0, 1920, 3360, 4800 and
    // the second bar is cut short. The box must name ticks the way the engine
    // does rather than by a count of its own.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    apply_timesig(song, 2880, 3, 4);
    song.build_timing();
    {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(3600);
        ts.chord = std::move(c);
        song.sequence.push_back(std::move(ts));
    }
    PreviewScene scene = build_preview_scene(song, nullptr);

    // 120 BPM at 480 ticks/quarter: tick 3600 is 3750 ms, past the change.
    const Timecode tc = song.timing().timecode(3600);
    const int64_t* mbt = tc.measure_beats_ticks();
    CHECK(mbt[0] + 1 == 3);  // the engine's measure, 1-based for display
    CHECK(mbt[1] + 1 == 1);
    CHECK(mbt[2] == 240);

    PreviewTimeBox box = build_time_box(scene, 3750.0, 3750.0);
    CHECK(box.position == "m3.1.240");

    // ...and that measure is the one the drawn bar lines put tick 3600 in: the
    // third line is at 3360 and the fourth at 4800.
    std::vector<int64_t> bars;
    for (const PreviewBeat& b : scene.beats)
        if (b.kind == PreviewBeatKind::Bar) bars.push_back(b.tick);
    REQUIRE(bars.size() >= 4);
    CHECK(bars[1] == 1920);
    CHECK(bars[2] == 3360);
    CHECK(bars[3] == 4800);
}

TEST_CASE("build_time_box: the time signature in force, as the chart wrote it") {
    // 120 BPM, 480 ticks per quarter. 6/8 from tick 1920 (2000 ms), then 3/4
    // from tick 3360 (3500 ms). Both are 1440 ticks a measure, so only the
    // stored signature tells them apart.
    Song song(480);
    song.bpm_changes[0] = 120.0;
    apply_timesig(song, 1920, 6, 8);
    apply_timesig(song, 3360, 3, 4);
    song.build_timing();
    {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(4800);
        ts.chord = std::move(c);
        song.sequence.push_back(std::move(ts));
    }
    PreviewScene scene = build_preview_scene(song, nullptr);

    CHECK(build_time_box(scene, 0.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(build_time_box(scene, 1999.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 4/4");
    CHECK(build_time_box(scene, 2000.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 6/8");
    CHECK(build_time_box(scene, 3500.0, 5000.0).tempo == "BPM 120.000 " + kDot + " 3/4");
    // A scene built from nothing reads the chart default.
    CHECK(build_time_box(PreviewScene{}, 0.0, 0.0).tempo == "BPM 0.000 " + kDot + " 4/4");
    CHECK(build_time_box(PreviewScene{}, 0.0, 0.0).position == "m1.1.0");
}

TEST_CASE("step_tick_ms: one tick from the tick the time box shows") {
    // 120 BPM, 480 ticks per quarter: a tick is 500/480 ms.
    Song song = make_hand_song();
    PreviewScene scene = build_preview_scene(song, nullptr);
    const MsIndex& ms = song.timing().ms_index();

    // 1300 ms is the time box's own example: tick 1248, "m1.3.288".
    const double fwd = step_tick_ms(scene, 1300.0, 5000.0, 1);
    const double back = step_tick_ms(scene, 1300.0, 5000.0, -1);
    CHECK(fwd == doctest::Approx(ms.at(1249)));
    CHECK(back == doctest::Approx(ms.at(1247)));
    CHECK(build_time_box(scene, fwd, 5000.0).position == "m1.3.289");
    CHECK(build_time_box(scene, back, 5000.0).position == "m1.3.287");

    // Between ticks it steps from the rounded tick: 1300.6 ms rounds to 1249.
    CHECK(step_tick_ms(scene, 1300.6, 5000.0, 1) == doctest::Approx(ms.at(1250)));

    // Never before tick 0.
    CHECK(step_tick_ms(scene, 0.0, 5000.0, -1) == doctest::Approx(0.0));
    CHECK(step_tick_ms(scene, 0.5, 5000.0, -3) == doctest::Approx(0.0));

    // Past the end the time box shows the end (5000 ms, tick 4800), and the
    // step starts there too.
    CHECK(build_time_box(scene, 6000.0, 5000.0).position == "m3.3.0");
    CHECK(step_tick_ms(scene, 6000.0, 5000.0, -1) == doctest::Approx(ms.at(4799)));
}

TEST_CASE("step_tick_ms: a tempo change moves the tick length with it") {
    // 120 BPM until tick 1920 (2000 ms), then 240 BPM: a tick shrinks from
    // 500/480 ms to 250/480 ms.
    Song song = make_sp_song({}, 3840, {{1920, 240.0}});
    PreviewScene scene = build_preview_scene(song, nullptr);
    const MsIndex& ms = song.timing().ms_index();
    REQUIRE(ms.at(1920) == doctest::Approx(2000.0));

    CHECK(step_tick_ms(scene, 2000.0, 5000.0, 1) == doctest::Approx(ms.at(1921)));
    CHECK(step_tick_ms(scene, 2000.0, 5000.0, -1) == doctest::Approx(ms.at(1919)));
    CHECK(step_tick_ms(scene, 2000.0, 5000.0, 1) - 2000.0 == doctest::Approx(250.0 / 480.0));
    CHECK(2000.0 - step_tick_ms(scene, 2000.0, 5000.0, -1) == doctest::Approx(500.0 / 480.0));
}

TEST_CASE("step_tick_ms: a scene with no song leaves the time alone") {
    PreviewScene empty;
    CHECK(step_tick_ms(empty, 1234.5, 5000.0, 1) == doctest::Approx(1234.5));
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
    // replay_path owns the withheld total; 180 was pinned from one run on
    // 2026-10-05 as a fixed oracle beside it.
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
    // A Red note every 500 ms (tick 480 steps). One bar of SP activated at
    // tick 2400 (2500 ms) runs two measures, to tick 6240 (6500 ms).
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.activations.size() == 1);
    REQUIRE(scene.activations[0].has_sp_end);
    CHECK(scene.activations[0].sp_end_ms == doctest::Approx(6500.0));

    // Each step carries exactly what the replay says the disc shows.
    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    for (size_t i = 0; i < r.chords.size(); ++i) {
        CHECK(scene.score.steps[i].multiplier == r.chords[i].multiplier_shown);
        // And the plain disc, which the box shows once SP has ended.
        CHECK(scene.score.steps[i].multiplier_plain == r.chords[i].multiplier_after);
    }

    // The SP end itself: the chord on it (6500 ms) was paid doubled, but from
    // this instant on the box is plain. A hair earlier it is still doubled.
    CHECK(score_box_at(scene, 6499.0).detail == "x4 " + kDot + " combo 13");
    CHECK(score_box_at(scene, 6500.0).detail == "x2 " + kDot + " combo 14");
    CHECK(score_box_at(scene, scene.activations[0].sp_end_ms).detail ==
          "x2 " + kDot + " combo 14");

    // 2000 ms: five notes hit, before the activation.
    CHECK(score_box_at(scene, 2000.0).detail == "x1 " + kDot + " combo 5");
    // 2500 ms: the activation chord is hit and paid, so x1 shows as x2.
    CHECK(score_box_at(scene, 2500.0).detail == "x2 " + kDot + " combo 6");
    CHECK(score_box_at(scene, 3000.0).detail == "x2 " + kDot + " combo 7");
    // 6600 ms: Star Power has ended, so the disc is plain even though the
    // last chord hit (6500 ms, on the deactivation node) was paid doubled.
    CHECK(score_box_at(scene, 6600.0).detail == "x2 " + kDot + " combo 14");
    // 7000 ms: the first chord Star Power doesn't pay: the plain x2 of combo 15.
    CHECK(score_box_at(scene, 7000.0).detail == "x2 " + kDot + " combo 15");
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
    // 2500 ms, and the box counts that chord as hit and paid (D48, Q27). The
    // box already counted it before struck_at existed, so this case guards the
    // shared rule rather than a fix: make struck_at exclusive and it reads
    // "x1 · combo 5". The highway's case is the one the fix turned green.
    Song song = make_sp_song({1920}, 9600);
    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    const std::optional<double> jump = activation_jump_ms(scene, 0.0, +1);
    REQUIRE(jump.has_value());
    CHECK(score_box_at(scene, *jump).detail == "x2 " + kDot + " combo 6");
}

// User decision D11 (docs/audit/2026-10-03-fix-decisions.md): a chord just past
// the SP end that Hydra's leeway still pays doubled is paid doubled in the
// score, but the box reads plain from the SP end on.
TEST_CASE("score box: a leeway chord past the SP end is paid doubled, the box stays plain") {
    // Same song as above, plus one extra chord 1 tick (about 1 ms) after the
    // SP end at tick 6240, inside the 3 ms leeway.
    Song song = make_sp_song({1920}, 9600);
    const int64_t leeway_tick = 6241;
    const auto at = std::find_if(song.sequence.begin(), song.sequence.end(),
                                 [&](const SongTimestamp& ts) {
                                     return ts.timecode.ticks() > 6240;
                                 });
    REQUIRE(at != song.sequence.end());
    SongTimestamp extra;
    extra.timecode = song.timecode(leeway_tick);
    extra.chord.add_note(NoteColor::Red);
    song.sequence.insert(at, std::move(extra));

    Path path = priced_path(song, {sp_act_at(song, 2400, 1, 6240)});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.activations.size() == 1);
    const double sp_end = scene.activations[0].sp_end_ms;
    CHECK(sp_end == doctest::Approx(6500.0));

    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    size_t idx = r.chords.size();
    for (size_t i = 0; i < r.chords.size(); ++i)
        if (r.chords[i].tick == leeway_tick) idx = i;
    REQUIRE(idx < r.chords.size());
    const ReplayChord& leeway = r.chords[idx];

    // The replay paid it doubled, a hair after the SP end.
    CHECK(leeway.ms > sp_end);
    CHECK(leeway.ms - sp_end < core::default_rules().backend_leeway_ms);
    CHECK(leeway.in_sp);
    CHECK(leeway.multiplier_shown == leeway.multiplier_after * kStarPowerMultiplier);
    CHECK(scene.score.steps[idx].multiplier == leeway.multiplier_shown);
    CHECK(scene.score.steps[idx].multiplier_plain == leeway.multiplier_after);

    // The box, at its own instant, reads plain.
    const PreviewScoreBox box = score_box_at(scene, leeway.ms);
    CHECK(box.detail == "x" + std::to_string(leeway.multiplier_after) + " " + kDot +
                            " combo " + std::to_string(leeway.combo_after));
}

TEST_CASE("score box: a path the replay can't reproduce says so") {
    Song song = make_hand_song();

    // The stored score disagrees with the replay by one point.
    Path wrong = priced_path(song, {});
    wrong.score_base += 1;
    PreviewScene a = build_preview_scene(song, &wrong);
    CHECK(a.score.state == PreviewScore::State::Unavailable);
    PreviewScoreBox box = score_box_at(a, 600.0);
    CHECK(box.shown);
    CHECK_FALSE(box.available);
    CHECK(box.score == "Score unavailable");
    CHECK(box.detail.empty());

    // An activation with no deactivation node (a record from before blob v4)
    // yields no window, so the replay can't stand for the path.
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

// ---- SP drain box -------------------------------------------------------

TEST_CASE("drain box: hidden without an SP gauge") {
    CHECK_FALSE(drain_box_at(PreviewScene{}, 1000.0).shown);
    // Notes but no SP phrase and no path: no gauge, so no box.
    Song song = make_sp_song({}, /*last_tick=*/3840);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.sp_meter.segments.empty());
    CHECK_FALSE(drain_box_at(scene, 1000.0).shown);
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
    // A pre-v4 record: no deact node, so nothing says SP is running.
    Song song = make_sp_song({960, 1920}, /*last_tick=*/9600);
    Path path;
    path.activations = {act_at(song, 3840, /*skipped_fills=*/{})};
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE_FALSE(scene.activations[0].has_sp_end);

    PreviewDrainBox box = drain_box_at(scene, 5000.0);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.detail.rfind("full meter ", 0) == 0);
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
// bar of SP runs two measures, so its window ends at tick 5280. The scrubber
// is built the way the Preview builds it (its right edge from scrub_end_ms
// over the song's length) and the Paths timeline over the same length
// (app::song_length_ms, D75; this chart's stated length is its 6000 ms), and
// the two marks must be the same number.
TEST_CASE("scrub marks: an activation sits at the Paths timeline's fraction when the audio outlasts the notes") {
    const test::AudioTailChart c = test::audio_tail_chart();
    Path path;
    path.activations = {sp_act_at(c.song, 1440, /*sp_meter=*/1, /*end_tick=*/5280)};
    const PreviewScene scene = build_preview_scene(c.song, &path, kCloneHeroSpCap,
                                                   core::default_rules(), c.audio_end_ms);
    const std::vector<double> marks =
        build_scrub_marks(scene, scrub_end_ms(c.audio_end_ms, c.audio_end_ms));
    REQUIRE(marks.size() == 1);
    CHECK(marks[0] == doctest::Approx(0.125));

    const ActivationsView view =
        build_activations(path, HydraRecord{}, &c.song.timing(), Settings{}.hit_window_ms,
                          std::nullopt, core::default_rules(), c.audio_end_ms);
    REQUIRE(view.acts.size() == 1);
    REQUIRE(view.acts[0].song_fraction.has_value());
    CHECK(marks[0] == *view.acts[0].song_fraction);
}

// The scrubber's right end is the song's length (D75; this chart states
// 6000 ms), so past the last note the thumb keeps following the playhead, all
// the way to that end.
TEST_CASE("scrub marks: a playhead past the last note parks the thumb at the right end") {
    const test::AudioTailChart c = test::audio_tail_chart();
    const double end = scrub_end_ms(c.audio_end_ms, c.audio_end_ms);
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

TEST_CASE("next activation box: on an activation within half a millisecond, and no chord") {
    TwoActs t;
    // The playhead counts as on an activation up to half a millisecond past
    // it: at 2000.5 ms the box still names activation 1, a hair later 2.
    CHECK(next_act_box_at(t.scene, -100.0, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, 2000.5, true).header == "Next: activation 1 of 2");
    CHECK(next_act_box_at(t.scene, std::nextafter(2000.5, 1e300), true).header ==
          "Next: activation 2 of 2");
    CHECK(next_act_box_at(t.scene, 8000.5, true).header == "Next: activation 2 of 2");
    CHECK_FALSE(next_act_box_at(t.scene, std::nextafter(8000.5, 1e300), true).shown);

    // An activation with no chord names only its measure.
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    path.activations = {sp_act_at(song, 1920, /*sp_meter=*/1, /*end_tick=*/5760)};
    const PreviewNextActBox box = next_act_box_at(build_preview_scene(song, &path), 0.0, true);
    CHECK(box.header == "Next: activation 1 of 1");
    CHECK(box.detail == "at m2.1.0");
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
    REQUIRE(scene.time_sigs.size() == 3);
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
