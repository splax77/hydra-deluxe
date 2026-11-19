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
#include "app/display_format.h"  // kFactSeparator
#include "app/preview_view.h"
#include "app/path_view.h"
#include "app/song_length.h"
#include "core/model.h"
#include "core/replay.h"
#include "core/scoring.h"  // solo_note_count, solo_bonus
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

// One measure's length in ms at a steady `bpm`, from the chart's own inputs:
// its ticks per measure from tick 0 over its ticks per beat, at 60000/bpm ms
// a beat.
double measure_ms_from_inputs(const Song& song, double bpm) {
    const double beats_per_measure = static_cast<double>(song.tpm_changes.at(0)) /
                                     static_cast<double>(song.tick_resolution());
    return beats_per_measure * 60000.0 / bpm;
}

// A length in ms as the drain box prints it: seconds, one decimal.
std::string drain_seconds(double ms) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.1f", ms / 1000.0);
    return buf;
}

// How long the engine's rule makes `bars` bars of Star Power last from
// `tick`: SongTiming::sp_end_ms, the timing owner's SP end for sp_bars_to_measures
// measures on, minus the start. It adds no measures of its own.
double engine_sp_ms(const Song& song, int64_t tick, int64_t bars) {
    const double from_ms = song.timing().timecode(tick).ms();
    return song.timing().sp_end_ms(from_ms, sp_bars_to_measures(bars)) - from_ms;
}

// The replay chord struck at `ms` (struck_at), or none before the first.
const ReplayChord* struck_chord(const ReplayResult& r, double ms) {
    const ReplayChord* at = nullptr;
    for (const ReplayChord& c : r.chords)
        if (struck_at(ms, c.ms)) at = &c;
    return at;
}

// Whether Star Power's bar from drawn measure line `i` runs inside one tempo
// and one signature: the kMeasuresPerSpBar measures from that line all have
// its signature, and no tempo entry falls strictly inside them. There the
// engine's bar (engine_sp_ms) is the bar in force the drain box reads.
bool steady_bar_from(const Song& song, const std::vector<PreviewBeat>& lines, size_t i) {
    const size_t end = i + static_cast<size_t>(kMeasuresPerSpBar);
    if (end >= lines.size()) return false;
    const SongTiming& timing = song.timing();
    const std::pair<int, int> sig = timing.time_signature_at(lines[i].tick);
    for (size_t k = i + 1; k < end; ++k)
        if (timing.time_signature_at(lines[k].tick) != sig) return false;
    for (const int64_t c : timing.ms_index().change_ticks())
        if (c > lines[i].tick && c < lines[end].tick) return false;
    return true;
}

// Pitch Dark (testdata/input/test_pitch_dark_ratio) analyzed under the app's
// default settings, and the record's path "3+ 2+ 1" drawn on it. Its first
// activation's stored SP end moves twice, by a clamp and then a squeeze-in
// (the Pitch Dark case in test_sp_meter_curve.cpp requires those steps).
struct PitchDarkScene {
    std::string chart;
    const Song* song = nullptr;
    const Path* path = nullptr;
    PreviewScene scene;  // with the path
    PreviewScene bare;   // the same song with no path
};
PitchDarkScene pitch_dark_scene() {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    PitchDarkScene pd;
    pd.chart = corpus::chart_named("test_pitch_dark_ratio");
    REQUIRE_FALSE(pd.chart.empty());
    pd.song = &corpus::song(pd.chart, cfg.reading(), cfg.rules);
    pd.path = test::path_named(corpus::analyzed(pd.chart, cfg).all_paths(), "3+ 2+ 1");
    REQUIRE(pd.path != nullptr);
    pd.scene = build_preview_scene(*pd.song, pd.path, cfg.sp_cap, cfg.rules);
    pd.bare = build_preview_scene(*pd.song, nullptr, cfg.sp_cap, cfg.rules);
    REQUIRE(pd.scene.activations.size() == pd.path->walk_activations().size());
    return pd;
}

// The next activation box's detail line for activation `a` of a record on
// `song`, worked from the record alone: "at", the measure format_measure
// (core/timing.h) gives the activation's tick, the app's fact separator
// (kFactSeparator, app/display_format.h), then the chord in Chord::rowstr's
// words for `pro_drums`. An engine activation always carries the chord hit
// there (an activation node is a chart note, search/engine.cpp), so a record
// activation without one fails here.
std::string record_act_detail(const Song& song, const Activation& a, bool pro_drums) {
    REQUIRE(a.chord.count() > 0);
    return "at " + format_measure(song.timing(), a.timecode.ticks()) + kFactSeparator +
           a.chord.rowstr(pro_drums);
}

// The next activation box's header for activation `i` of `count`.
std::string next_act_header(size_t i, size_t count) {
    return "Next: activation " + std::to_string(i + 1) + " of " + std::to_string(count);
}

// A corpus chart whose drum track names one solo: Snakeskin Boots, whose
// .chart writes "E solo" at tick 53376 and "E soloend" at 59424 in its
// ExpertDrums track, with 31 of the solo's 66 chords holding two or more pads.
const Song& solo_song() {
    const Song& song = corpus_song("Jack Wall - Snakeskin Boots");
    REQUIRE(song.solo_sections.size() == 1);
    return song;
}

// The first moment a box timed off `from` reads as `since` ms on: from +
// since, or the next double up when the sum's rounding leaves now - from
// short of `since`.
double moment_after(double from, double since) {
    double t = from + since;
    while (t - from < since) t = std::nextafter(t, std::numeric_limits<double>::infinity());
    return t;
}

// The notes the engine's scoring counts in `s` (solo_note_count, core/scoring.h).
int section_notes(const Song& song, const SoloSection& s) {
    int notes = 0;
    for (size_t i = s.first; i <= s.last; ++i)
        notes += solo_note_count(song.sequence[i].chord, song.sequence[i].flag_solo);
    return notes;
}

// The bonus the engine's scoring pays over `s` (solo_bonus, core/scoring.h).
int64_t section_bonus(const Song& song, const SoloSection& s) {
    int64_t bonus = 0;
    for (size_t i = s.first; i <= s.last; ++i)
        bonus += solo_bonus(song.sequence[i].chord, song.sequence[i].flag_solo);
    return bonus;
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
    // A real chart that was never analyzed: no path, so no score and no
    // stars, at a playhead halfway through its notes.
    const RealPlayhead p = real_playhead();
    CHECK(p.scene.score.state == PreviewScore::State::None);
    CHECK(p.scene.score.steps.empty());
    const PreviewScoreBox box = score_box_at(p.scene, p.chord_ms);
    CHECK_FALSE(box.shown);
    CHECK(box.stars == 0);
    CHECK(box.next_star.empty());
}

TEST_CASE("score box: a solo's bonus lands on its last note") {
    // Hand song: chords at 0, 250, 500 and 750 ms. The solo is the chords at
    // 250 and 500 ms, so its bonus is withheld at 250 and paid at 500.
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == 4);

    // The replay owns every total, multiplier and combo the box shows.
    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());
    REQUIRE(r.chords[1].points.solo > 0);
    CHECK(scene.score.steps[1].total == r.chords[1].cum_onscreen_total);
    CHECK(scene.score.steps[2].total == r.chords[2].cum.total());
    CHECK(scene.score.steps[3].total == path.totalscore());

    // Between the solo's two chords the box shows the withheld total.
    PreviewScoreBox mid = score_box_at(scene, 400.0);
    CHECK(mid.shown);
    CHECK(mid.available);
    CHECK(mid.score == group_thousands(scene.score.steps[1].total));
    CHECK(mid.detail == score_detail(r.chords[1].multiplier_after, r.chords[1].combo_after));

    // A chord exactly at the playhead counts as hit.
    PreviewScoreBox on = score_box_at(scene, scene.score.steps[2].ms);
    CHECK(on.score == group_thousands(scene.score.steps[2].total));
    CHECK(on.detail == score_detail(r.chords[2].multiplier_after, r.chords[2].combo_after));
}

TEST_CASE("score box: before the first note nothing is hit yet") {
    // A real chart, priced by the replay with no activation. Its drum track's
    // first note is at tick 1248 (notes.chart), so there is room before it.
    const Song& song = corpus_song("Deftones - My Own Summer");
    const Path path = priced_path(song, {});
    const PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE_FALSE(scene.score.steps.empty());
    const double first_chord_ms = scene.score.steps.front().ms;
    REQUIRE(first_chord_ms > 0.0);
    const PreviewScoreBox box = score_box_at(scene, first_chord_ms / 2.0);
    CHECK(box.shown);
    CHECK(box.available);
    // The start of the count: no points, the plain multiplier, no combo.
    CHECK(box.score == group_thousands(0));
    CHECK(box.detail == score_detail(1, 0));
}

TEST_CASE("struck_at: a chord exactly on the playhead counts as hit") {
    // A real chord's time: the chord halfway through My Own Summer. A jump
    // to an activation lands on its chord's exact ms (D48, Q27), so that
    // moment counts the chord as hit; a moment before it does not.
    const RealPlayhead p = real_playhead();
    CHECK(struck_at(p.chord_ms, p.chord_ms));
    CHECK_FALSE(struck_at(p.chord_ms - 0.1, p.chord_ms));
    CHECK(struck_at(p.chord_ms + 0.1, p.chord_ms));
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

TEST_CASE("every overlay box: past the length it reads the moment the time box shows") {
    // The analyzed corpus chart's best path. Playback is cut at each
    // activation's own moment, and the playhead sits a second past it: every
    // box must read exactly what it reads with the playhead on that moment.
    const AnalysisResult& a = corpus::analyzed_by_ten_scores();
    const Path& best = a.record.best_path();
    const PreviewScene scene = build_preview_scene(a.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE_FALSE(scene.activations.empty());
    for (const PreviewActivation& act : scene.activations) {
        const double len = act.ms;
        const double past = len + 1000.0;
        INFO("playback cut at " << len << " ms");

        const PreviewScoreBox s_past = build_score_box(scene, past, len);
        const PreviewScoreBox s_end = score_box_at(scene, len);
        CHECK(s_past.shown == s_end.shown);
        CHECK(s_past.score == s_end.score);
        CHECK(s_past.detail == s_end.detail);
        CHECK(s_past.stars == s_end.stars);
        CHECK(s_past.next_star == s_end.next_star);

        const PreviewDrainBox d_past = build_drain_box(scene, past, len);
        const PreviewDrainBox d_end = drain_box_at(scene, len);
        CHECK(d_past.shown == d_end.shown);
        CHECK(d_past.active == d_end.active);
        CHECK(d_past.header == d_end.header);
        CHECK(d_past.rate == d_end.rate);
        CHECK(d_past.detail == d_end.detail);

        const PreviewSoloBox o_past = build_solo_box(scene, past, len);
        const PreviewSoloBox o_end = solo_box_at(scene, len);
        CHECK(o_past.shown == o_end.shown);
        CHECK(o_past.headline == o_end.headline);
        CHECK(o_past.detail == o_end.detail);

        const PreviewNextActBox n_past = build_next_act_box(scene, past, len, true);
        const PreviewNextActBox n_end = next_act_box_at(scene, len, true);
        CHECK(n_past.shown == n_end.shown);
        CHECK(n_past.header == n_end.header);
        CHECK(n_past.notation == n_end.notation);
        CHECK(n_past.detail == n_end.detail);

        // The box past the length is not the box a second later: the cut
        // holds the playhead on the activation, so the next activation box
        // still names the one it sits on.
        CHECK(n_past.notation == act.notation);
    }
}

TEST_CASE("score box: a path the replay can't reproduce says so") {
    // The analyzed corpus chart's best path, which the replay reproduces;
    // the two broken copies below differ from it in one fact each.
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
    const Path& best = r.record.best_path();
    REQUIRE_FALSE(best.activations.empty());
    const PreviewScene good = build_preview_scene(r.song, &best);
    REQUIRE(good.score.state == PreviewScore::State::Ready);
    const double mid_ms = good.score.steps[good.score.steps.size() / 2].ms;
    const PreviewScoreBox ready = score_box_at(good, mid_ms);
    REQUIRE(ready.available);

    // The stored score disagrees with the replay by one point.
    Path wrong = best;
    wrong.notes_points += 1;
    const PreviewScene a = build_preview_scene(r.song, &wrong);
    CHECK(a.score.state == PreviewScore::State::Unavailable);
    const PreviewScoreBox box = score_box_at(a, mid_ms);
    CHECK(box.shown);
    CHECK_FALSE(box.available);
    // It says so in words, in place of the score the ready box shows, and
    // shows no detail line and no star counter.
    CHECK_FALSE(box.score.empty());
    CHECK(box.score != ready.score);
    CHECK(box.detail.empty());
    CHECK(box.stars == 0);
    CHECK(box.next_star.empty());

    // An activation with no deactivation node yields no window, so the replay
    // can't stand for the path. It reads the same words.
    Path old = best;
    old.activations.front().sp_end_steps.clear();
    const PreviewScene b = build_preview_scene(r.song, &old);
    CHECK(b.score.state == PreviewScore::State::Unavailable);
    const PreviewScoreBox old_box = score_box_at(b, mid_ms);
    CHECK_FALSE(old_box.available);
    CHECK(old_box.score == box.score);
    CHECK(old_box.stars == 0);
    CHECK(old_box.next_star.empty());
}

TEST_CASE("score box: the analyzed chart ends on the path's total") {
    const AnalysisResult& r = corpus::analyzed_by_ten_scores();
    const Path& best = r.record.best_path();
    PreviewScene scene = build_preview_scene(r.song, &best);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    REQUIRE(scene.score.steps.size() == r.song.sequence.size());
    CHECK(scene.score.steps.back().total == best.totalscore());
    for (size_t i = 1; i < scene.score.steps.size(); ++i) {
        INFO("chord " << i << " of " << scene.score.steps.size());
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
}

TEST_CASE("score box stars: a solo's bonus raises the score but not the stars") {
    // Hand song: chords at 0, 250, 500 and 750 ms. The solo is the chords at
    // 250 and 500 ms, so its whole bonus is paid on the chord at 500 ms.
    Song song = make_hand_song();
    Path path = priced_path(song, {});
    REQUIRE(path.score_solo > 0);
    PreviewScene scene = build_preview_scene(song, &path);
    REQUIRE(scene.score.state == PreviewScore::State::Ready);
    const StarCutoffs cut = star_cutoffs(path);
    const ReplayResult r = replay_stored_path(song, path).result;
    REQUIRE(r.chords.size() == scene.score.steps.size());

    // At every chord, and a moment after the last, the box shows the
    // replay's on-screen total, while the star count reads the replay's
    // score without the solo bonus (score_without_solo, core/stars.h).
    for (const ReplayChord& c : r.chords) {
        INFO("chord at " << c.ms << " ms");
        const PreviewScoreBox box = score_box_at(scene, c.ms);
        CHECK(box.score == group_thousands(c.cum_onscreen_total));
        const int64_t counted = score_without_solo(c.cum);
        CHECK(box.stars == stars_for_score(cut, counted));
        if (box.stars < kMaxStars)
            CHECK(box.next_star == "star " + std::to_string(box.stars + 1) + " in " +
                                       group_thousands(cut.cutoffs[box.stars] - counted));
    }

    // The solo's last chord: the on-screen score jumps by that chord's own
    // points plus the whole bonus; the score the stars read moves by the
    // chord's own points alone.
    const PreviewScoreStep& s1 = scene.score.steps[1];
    const PreviewScoreStep& s2 = scene.score.steps[2];
    CHECK(s2.total - s1.total ==
          (s2.score_without_solo - s1.score_without_solo) + path.score_solo);
    // This song's solo bonus lifts the shown score across a star cutoff the
    // star count has not reached, so a box that counted the bonus would
    // show one more star here.
    REQUIRE(stars_for_score(cut, s2.total) > stars_for_score(cut, s2.score_without_solo));
    CHECK(score_box_at(scene, s2.ms).stars == stars_for_score(cut, s2.score_without_solo));

    // At the end the box agrees with the path's own count (path_stars).
    CHECK(score_box_at(scene, 1000.0).stars == path_stars(path));
}

// ---- SP drain box -------------------------------------------------------

TEST_CASE("drain box: idle on a chart with no Star Power") {
    // A default scene was built from no song: no gauge, so no box.
    CHECK_FALSE(drain_box_at(PreviewScene{}, 1000.0).shown);
    // Notes but no SP phrase and no path: the gauge is empty, and the box
    // still shows, in its idle form.
    const double bpm = 120.0;  // make_sp_song's one tempo
    Song song = make_sp_song({}, /*last_tick=*/3840);
    PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.sp_meter.segments.empty());
    const double now_ms = 1000.0;
    const PreviewDrainBox box = drain_box_at(scene, now_ms);
    CHECK(box.shown);
    CHECK_FALSE(box.active);
    CHECK(box.header == "SP drain (if activated)");

    // One bar is kMeasuresPerSpBar measures of the chart's own measure; a
    // full meter is the scene's cap in bars.
    const double bar_ms = static_cast<double>(kMeasuresPerSpBar) * measure_ms_from_inputs(song, bpm);
    const double full_ms = bar_ms * static_cast<double>(scene.sp_meter.cap);
    CHECK(box.rate == "1 bar / " + drain_seconds(bar_ms) + " s");
    CHECK(box.detail == "full meter " + drain_seconds(full_ms) + " s");

    // At one steady tempo the engine's rule, which counts measures on from
    // the playhead, gives the same two lengths.
    const int64_t now_tick = scene.timing->display_tick_at_ms(now_ms);
    CHECK(engine_sp_ms(song, now_tick, 1) == doctest::Approx(bar_ms));
    CHECK(engine_sp_ms(song, now_tick, scene.sp_meter.cap) == doctest::Approx(full_ms));
}

TEST_CASE("drain box: the rate switches exactly at a tempo change") {
    // make_sp_song's 120 BPM, then a slower tempo from a tick on.
    const double bpm_before = 120.0;
    const double bpm_after = 60.0;
    const int64_t change_tick = 5760;
    Song song = make_sp_song({960}, /*last_tick=*/13440, {{change_tick, bpm_after}});
    PreviewScene scene = build_preview_scene(song, nullptr);
    const double change_ms = song.timing().ms_index().at(change_tick);
    const double before_ms = change_ms - 10.0;
    REQUIRE(scene.timing->display_tick_at_ms(before_ms) < change_tick);
    REQUIRE(scene.timing->display_tick_at_ms(change_ms) == change_tick);

    const double cap = static_cast<double>(scene.sp_meter.cap);
    const double bar_before = static_cast<double>(kMeasuresPerSpBar) *
                              measure_ms_from_inputs(song, bpm_before);
    const double bar_after = static_cast<double>(kMeasuresPerSpBar) *
                             measure_ms_from_inputs(song, bpm_after);
    REQUIRE(drain_seconds(bar_before) != drain_seconds(bar_after));

    // The rate reads the tempo in force: the old one just before the change,
    // the new one from the change on.
    CHECK(drain_box_at(scene, before_ms).rate == "1 bar / " + drain_seconds(bar_before) + " s");
    CHECK(drain_box_at(scene, change_ms).rate == "1 bar / " + drain_seconds(bar_after) + " s");

    // "full meter" is the cap at the bar time in force, so it jumps with the
    // rate, at the change and not before.
    CHECK(drain_box_at(scene, 0.0).detail == "full meter " + drain_seconds(bar_before * cap) + " s");
    CHECK(drain_box_at(scene, before_ms).detail ==
          "full meter " + drain_seconds(bar_before * cap) + " s");
    CHECK(drain_box_at(scene, change_ms).detail ==
          "full meter " + drain_seconds(bar_after * cap) + " s");

    // From the change on the tempo stays put, so the engine's rule gives the
    // same lengths there. Just before the change it does not: the engine
    // counts the bar across the change, and the box reads only the tempo in
    // force, so before a change its times are its own estimate.
    CHECK(engine_sp_ms(song, change_tick, 1) == doctest::Approx(bar_after));
    CHECK(engine_sp_ms(song, change_tick, scene.sp_meter.cap) == doctest::Approx(bar_after * cap));
}

TEST_CASE("drain box: a 7/8 section drains faster at the same BPM") {
    // Stabwound writes 7/8 (its .chart's "TS 7 3") between 8/8 sections at
    // 120 BPM (its "B 120000"). Wherever Star Power's bar runs inside one
    // tempo and one signature (steady_bar_from), the box's bar is the
    // engine's (engine_sp_ms), and its full meter is that bar times the cap.
    const Song& song = corpus_song("Necrophagist - Stabwound");
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    const std::vector<PreviewBeat> lines = drawn_measure_lines(scene);
    const double cap = static_cast<double>(scene.sp_meter.cap);

    // The engine's bar at each steady measure line in 7/8 and in 8/8, with
    // the tempo there.
    struct SteadyBar {
        double tempo;
        double bar_ms;
    };
    std::vector<SteadyBar> seven_eight;
    std::vector<SteadyBar> eight_eight;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (!steady_bar_from(song, lines, i)) continue;
        const PreviewBeat& line = lines[i];
        INFO("measure line at tick " << line.tick);
        const double bar_ms = engine_sp_ms(song, line.tick, 1);
        const PreviewDrainBox box = drain_box_at(scene, line.ms);
        CHECK(box.rate == "1 bar / " + drain_seconds(bar_ms) + " s");
        CHECK(box.detail == "full meter " + drain_seconds(bar_ms * cap) + " s");
        const std::pair<int, int> sig = scene.timing->time_signature_at(line.tick);
        const SteadyBar bar{scene.timing->tempo_at(line.tick), bar_ms};
        if (sig == std::pair<int, int>{7, 8}) seven_eight.push_back(bar);
        if (sig == std::pair<int, int>{8, 8}) eight_eight.push_back(bar);
    }

    // At the same tempo a 7/8 bar is shorter than an 8/8 one, so the box
    // reads a faster drain in 7/8. The chart's steady 7/8 measures (from tick
    // 10320) and 8/8 measures (from tick 4560) are both at 120 BPM.
    size_t compared = 0;
    for (const SteadyBar& s : seven_eight) {
        for (const SteadyBar& e : eight_eight) {
            if (s.tempo != e.tempo) continue;
            ++compared;
            CHECK(s.bar_ms < e.bar_ms);
            CHECK(drain_seconds(s.bar_ms) != drain_seconds(e.bar_ms));
        }
    }
    CHECK(compared > 0);
}

TEST_CASE("drain box: active inside the stored SP window, idle outside it") {
    // Pitch Dark's record: each activation's window runs from its own moment
    // to the stored end, the record's Activation::window_end_tick.
    const PitchDarkScene pd = pitch_dark_scene();
    const PreviewScene& scene = pd.scene;
    REQUIRE_FALSE(scene.activations.empty());
    // Is any activation other than `self` running at `ms`?
    auto other_running = [&scene](const PreviewActivation& self, double ms) {
        for (const PreviewActivation& o : scene.activations)
            if (&o != &self && o.sp_running_at(ms)) return true;
        return false;
    };
    for (size_t i = 0; i < scene.activations.size(); ++i) {
        const PreviewActivation& a = scene.activations[i];
        INFO("activation at tick " << a.tick);
        REQUIRE(a.sp_end_tick == pd.path->walk_activations()[i].window_end_tick());
        REQUIRE(a.sp_window().has_value());
        const double start = a.sp_window()->first;
        const double end = a.sp_window()->second;

        // From its own moment the box counts down to the stored end.
        const PreviewDrainBox at = drain_box_at(scene, start);
        CHECK(at.active);
        CHECK(at.header == "SP drain");
        CHECK(at.detail == "empties in " + drain_seconds(end - start) + " s");
        const double mid = (start + end) / 2.0;
        CHECK(drain_box_at(scene, mid).detail == "empties in " + drain_seconds(end - mid) + " s");

        // Just before it, and from the stored end on, it reads idle: what it
        // reads with no path at all.
        for (const double idle_ms : {std::nextafter(start, 0.0), end}) {
            if (other_running(a, idle_ms)) continue;
            const PreviewDrainBox box = drain_box_at(scene, idle_ms);
            const PreviewDrainBox bare = drain_box_at(pd.bare, idle_ms);
            CHECK_FALSE(box.active);
            CHECK(box.header == bare.header);
            CHECK(box.detail == bare.detail);
        }
    }
}

TEST_CASE("drain box: empties in reads the stored end, not a recount") {
    // Pitch Dark's first activation: its stored SP end moves past the first
    // step's end (a clamp, then a squeeze-in). Past that first end the box
    // still runs, and counts down to the last step's end.
    const PitchDarkScene pd = pitch_dark_scene();
    const Activation& rec = pd.path->walk_activations()[0];
    REQUIRE(rec.sp_end_steps.size() >= 2);
    const PreviewActivation& a = pd.scene.activations[0];
    REQUIRE(a.sp_end_tick == rec.window_end_tick());
    const int64_t first_end_tick = rec.sp_end_steps.front().end_tick;
    REQUIRE(first_end_tick < a.sp_end_tick);

    const double first_end_ms = pd.song->timing().ms_index().at(first_end_tick);
    const double now = (first_end_ms + a.sp_end_ms) / 2.0;
    const PreviewDrainBox box = drain_box_at(pd.scene, now);
    CHECK(box.active);
    CHECK(box.detail == "empties in " + drain_seconds(a.sp_end_ms - now) + " s");
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

TEST_CASE("solo_percent: truncated, never above the whole-number percent") {
    // Snakeskin Boots' solo: at every count of notes hit the box shows, the
    // percent is at most the exact share truncated and at most one below it
    // (the game's sum is done in float); all notes read 100, none 0.
    const Song& song = solo_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solo_notes.size() == 1);
    const PreviewSoloNotes& solo = scene.solo_notes[0];
    REQUIRE(solo.notes > 0);
    for (const PreviewSoloNotes::Step& s : solo.steps) {
        INFO(s.notes_hit << " of " << solo.notes << " notes");
        const int whole = s.notes_hit * 100 / solo.notes;
        const int shown = solo_percent(s.notes_hit, solo.notes);
        CHECK(shown <= whole);
        CHECK(shown >= whole - 1);
    }
    CHECK(solo_percent(solo.notes, solo.notes) == 100);
    CHECK(solo_percent(0, solo.notes) == 0);
}

TEST_CASE("solo box: from before the solo to its bonus, timed off the last chord") {
    // Snakeskin Boots' one solo. Before its first chord the box is hidden;
    // on each chord before the last it counts the notes hit so far; from the
    // last chord it shows the end message for kSoloResultMs, then the bonus
    // the engine's scoring pays for the solo for kSoloResultMs, then hides.
    const Song& song = solo_song();
    const SoloSection& sec = song.solo_sections.front();
    REQUIRE(sec.last > sec.first);
    const PreviewScene scene = build_preview_scene(song, nullptr);
    const int notes = section_notes(song, sec);
    const int64_t bonus = section_bonus(song, sec);
    const double first_ms = song.sequence[sec.first].timecode.ms();
    const double last_ms = song.sequence[sec.last].timecode.ms();

    CHECK_FALSE(solo_box_at(scene, std::nextafter(first_ms, 0.0)).shown);

    int hit = 0;
    for (size_t i = sec.first; i < sec.last; ++i) {
        const SongTimestamp& ts = song.sequence[i];
        hit += solo_note_count(ts.chord, ts.flag_solo);
        const PreviewSoloBox b = solo_box_at(scene, ts.timecode.ms());
        INFO("chord " << i << ", " << hit << " of " << notes << " notes");
        CHECK(b.shown);
        CHECK(b.headline == std::to_string(solo_percent(hit, notes)) + "%");
        CHECK(b.detail == group_thousands(hit) + " / " + counted(notes, "note", "notes"));
    }

    // On the last chord the solo is over: the end message, no count.
    const PreviewSoloBox end = solo_box_at(scene, last_ms);
    CHECK(end.shown);
    CHECK(end.headline == "Perfect Solo!");
    CHECK(end.detail.empty());
    // A tenth of a millisecond is a free input: any moment just short of a
    // boundary.
    CHECK(solo_box_at(scene, last_ms + kSoloResultMs - 0.1).headline == "Perfect Solo!");

    // kSoloResultMs later: the bonus.
    const std::string bonus_text = "+" + group_thousands(bonus);
    const PreviewSoloBox paid = solo_box_at(scene, moment_after(last_ms, kSoloResultMs));
    CHECK(paid.shown);
    CHECK(paid.headline == bonus_text);
    CHECK(paid.detail.empty());
    CHECK(solo_box_at(scene, last_ms + kSoloTailMs - 0.1).headline == bonus_text);

    // kSoloTailMs after the last chord: hidden.
    CHECK_FALSE(solo_box_at(scene, moment_after(last_ms, kSoloTailMs)).shown);
}

TEST_CASE("solo box: the count is notes, not chords") {
    // Snakeskin Boots' solo holds more notes than chords (31 of its 66 chords
    // have two or more pads), so a box that counted chords would read lower.
    const Song& song = solo_song();
    const SoloSection& sec = song.solo_sections.front();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solo_notes.size() == 1);
    const int notes = section_notes(song, sec);
    const size_t chords = sec.last - sec.first + 1;
    REQUIRE(static_cast<size_t>(notes) > chords);
    CHECK(scene.solo_notes[0].notes == notes);
    CHECK(scene.solo_notes[0].steps.size() == chords);

    // On the chord before the last, the box counts every note of the chords
    // struck so far, out of the solo's notes.
    REQUIRE(sec.last > sec.first);
    int hit = 0;
    for (size_t i = sec.first; i < sec.last; ++i)
        hit += solo_note_count(song.sequence[i].chord, song.sequence[i].flag_solo);
    const PreviewSoloBox box = solo_box_at(scene, song.sequence[sec.last - 1].timecode.ms());
    CHECK(box.detail == group_thousands(hit) + " / " + counted(notes, "note", "notes"));
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
    const PreviewScene bare = build_preview_scene(song, nullptr);
    const PreviewScene with_path = build_preview_scene(song, &path);
    // The boxes' bonuses together are the path's solo score.
    int64_t bonuses = 0;
    for (const PreviewSoloNotes& s : bare.solo_notes) bonuses += s.bonus;
    CHECK(bonuses == path.score_solo);
    for (double t : {0.0, 500.0, 750.0, 1000.0, 1500.0, 2000.0, 3500.0, 5000.0}) {
        const PreviewSoloBox a = solo_box_at(bare, t);
        const PreviewSoloBox b = solo_box_at(with_path, t);
        CHECK(a.shown == b.shown);
        CHECK(a.headline == b.headline);
        CHECK(a.detail == b.detail);
    }
}

TEST_CASE("solo box: none on a chart without solos") {
    // The fill song names no solo, so at none of its chords is there a box.
    const Song song = make_fill_song();
    REQUIRE(song.solo_sections.empty());
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE_FALSE(song.sequence.empty());
    for (const SongTimestamp& ts : song.sequence)
        CHECK_FALSE(solo_box_at(scene, ts.timecode.ms()).shown);
}

TEST_CASE("solo box samples: each solo's widest boxes") {
    // Per solo: the box on its first chord, on its chord before the last, on
    // its last chord (the end message) and kSoloResultMs on (the bonus), in
    // that order, each the box the playhead shows at that moment.
    const Song& song = solo_song();
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.solo_notes.size() == 1);
    const PreviewSoloNotes& solo = scene.solo_notes[0];
    REQUIRE(solo.steps.size() >= 2);
    const double last_ms = solo.steps.back().ms;
    const std::vector<double> moments = {solo.steps.front().ms,
                                         solo.steps[solo.steps.size() - 2].ms, last_ms,
                                         moment_after(last_ms, kSoloResultMs)};
    const std::vector<PreviewSoloBox> samples = build_solo_box_samples(scene);
    REQUIRE(samples.size() == moments.size());
    for (size_t i = 0; i < moments.size(); ++i) {
        INFO("sample " << i << " at " << moments[i] << " ms");
        const PreviewSoloBox at = solo_box_at(scene, moments[i]);
        CHECK(samples[i].shown);
        CHECK(samples[i].shown == at.shown);
        CHECK(samples[i].headline == at.headline);
        CHECK(samples[i].detail == at.detail);
    }
}

TEST_CASE("scrub marks: each activation's onset over the scrubber's length") {
    // Pitch Dark's record over its length as the app works it out
    // (analysis_song_length). Each mark is its activation's moment over that
    // length; a shorter length clamps the marks past it to the bar's end.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::optional<double> len =
        analysis_song_length(std::nullopt, pd.chart, *pd.song).ms;
    REQUIRE(len.has_value());
    const std::vector<PreviewActivation>& acts = pd.scene.activations;
    REQUIRE(acts.size() >= 2);

    const std::vector<double> marks = build_scrub_marks(pd.scene, *len);
    REQUIRE(marks.size() == acts.size());
    for (size_t i = 0; i < acts.size(); ++i) {
        INFO("activation at tick " << acts[i].tick);
        CHECK(marks[i] == doctest::Approx(acts[i].ms / *len));
    }

    // A length that ends between the first and last activations.
    const double short_len = (acts.front().ms + acts.back().ms) / 2.0;
    const std::vector<double> clamped = build_scrub_marks(pd.scene, short_len);
    REQUIRE(clamped.size() == acts.size());
    CHECK(clamped.front() == doctest::Approx(acts.front().ms / short_len));
    CHECK(clamped.back() == 1.0);

    // No length, or no path: no marks.
    CHECK(build_scrub_marks(pd.scene, 0.0).empty());
    CHECK(build_scrub_marks(pd.bare, *len).empty());
}

TEST_CASE("song_fraction: has_song_length takes only a positive length") {
    // A real chart's length, as the app works it out, is one; no length (0)
    // and a negative one are not.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::optional<double> len =
        analysis_song_length(std::nullopt, pd.chart, *pd.song).ms;
    REQUIRE(len.has_value());
    CHECK(has_song_length(*len));
    CHECK_FALSE(has_song_length(0.0));
    CHECK_FALSE(has_song_length(-*len));
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
    // The case: the audio runs on past the last note.
    REQUIRE(c.audio_end_ms > c.last_note_ms);
    const std::vector<double> marks = build_scrub_marks(scene, c.audio_end_ms);
    REQUIRE(marks.size() == 1);
    REQUIRE(scene.activations.size() == 1);
    // The mark is the activation's moment over the audio's end, not over the
    // last note's.
    CHECK(marks[0] == doctest::Approx(scene.activations[0].ms / c.audio_end_ms));
    CHECK(marks[0] != doctest::Approx(scene.activations[0].ms / c.last_note_ms));

    const ActivationsView view =
        build_activations(path, HydraRecord{}, &c.song.timing(), std::nullopt,
                          core::default_rules(), c.audio_end_ms);
    REQUIRE(view.acts.size() == 1);
    REQUIRE(view.acts[0].song_fraction.has_value());
    CHECK(marks[0] == *view.acts[0].song_fraction);
}

TEST_CASE("scrub marks: a playhead past the last note parks the thumb at the right end") {
    // Pitch Dark's length as the app works it out (analysis_song_length).
    // song_length_ms adds kSongEndPadMs after the last note's end, so the
    // scrubber runs on past the last note. There the thumb follows the
    // playhead; past the scrubber's end it stays parked at the end.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::optional<double> len =
        analysis_song_length(std::nullopt, pd.chart, *pd.song).ms;
    REQUIRE(len.has_value());
    const double last = last_note_ms(pd.bare);
    REQUIRE(*len > last);
    const double tail = (last + *len) / 2.0;
    CHECK(scrub_thumb_ms(last, *len) == last);
    CHECK(scrub_thumb_ms(tail, *len) == tail);
    CHECK(scrub_thumb_ms(*len, *len) == *len);
    CHECK(scrub_thumb_ms(std::nextafter(*len, std::numeric_limits<double>::infinity()), *len) ==
          *len);
    const double past = *len + 1000.0;  // a free input: a second past the end
    CHECK(scrub_thumb_ms(past, *len) == *len);
}

TEST_CASE("activation jumps: nearest activation before or after the playhead") {
    // Pitch Dark's record. Each jump lands on an activation's own moment,
    // the record's tick through the chart's tempo map.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::vector<PreviewActivation>& acts = pd.scene.activations;
    REQUIRE(acts.size() >= 2);
    const MsIndex& ms = pd.song->timing().ms_index();
    std::vector<double> want;
    for (const Activation& a : pd.path->walk_activations()) want.push_back(ms.at(a.timecode.ticks()));
    REQUIRE(want.size() == acts.size());

    // Before the first activation "Act >" goes to it and "< Act" nowhere.
    CHECK(activation_jump_ms(pd.scene, 0.0, +1) == doctest::Approx(want.front()));
    CHECK_FALSE(activation_jump_ms(pd.scene, 0.0, -1).has_value());

    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        CHECK(acts[i].ms == doctest::Approx(want[i]));
        // Parked on activation i, "Act >" moves on to the next one and
        // "< Act" goes back past it; none either way at the ends.
        const std::optional<double> next = activation_jump_ms(pd.scene, acts[i].ms, +1);
        const std::optional<double> prev = activation_jump_ms(pd.scene, acts[i].ms, -1);
        if (i + 1 < acts.size()) {
            REQUIRE(next.has_value());
            CHECK(*next == doctest::Approx(want[i + 1]));
            // Between activation i and the next, each way lands on one of the two.
            const double mid = (acts[i].ms + acts[i + 1].ms) / 2.0;
            CHECK(activation_jump_ms(pd.scene, mid, +1) == doctest::Approx(want[i + 1]));
            CHECK(activation_jump_ms(pd.scene, mid, -1) == doctest::Approx(want[i]));
        } else {
            CHECK_FALSE(next.has_value());
        }
        if (i > 0) {
            REQUIRE(prev.has_value());
            CHECK(*prev == doctest::Approx(want[i - 1]));
        } else {
            CHECK_FALSE(prev.has_value());
        }
    }

    // With no path there is nowhere to jump.
    CHECK_FALSE(activation_jump_ms(pd.bare, 0.0, +1).has_value());
    CHECK_FALSE(activation_jump_ms(pd.bare, last_note_ms(pd.bare), -1).has_value());
}

TEST_CASE("next activation box: the activation at or after the playhead") {
    // Pitch Dark's record. On each activation, and between the one before it
    // and it, the box names it: its place in the path, then the measure and
    // chord the record gives it.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::vector<PreviewActivation>& acts = pd.scene.activations;
    REQUIRE(acts.size() >= 2);
    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        const std::string detail =
            record_act_detail(*pd.song, pd.path->walk_activations()[i], /*pro_drums=*/true);
        const double from = i == 0 ? 0.0 : (acts[i - 1].ms + acts[i].ms) / 2.0;
        for (const double now : {from, acts[i].ms}) {
            CAPTURE(now);
            const PreviewNextActBox box = next_act_box_at(pd.scene, now, /*pro_drums=*/true);
            CHECK(box.shown);
            CHECK(box.header == next_act_header(i, acts.size()));
            CHECK(box.detail == detail);
        }
    }
    // Past the last activation, and with no path, the box is hidden.
    const double past = acts.back().ms + 1000.0;  // a free input: a second past it
    CHECK_FALSE(next_act_box_at(pd.scene, past, true).shown);
    CHECK_FALSE(next_act_box_at(pd.bare, 0.0, true).shown);
}

TEST_CASE("next activation box: the note names follow the Pro Drums setting") {
    // The box names the chord in Chord::rowstr's words for the Pro Drums
    // setting (D48, Q11), with it off and on, on each of Pitch Dark's
    // activations.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::vector<PreviewActivation>& acts = pd.scene.activations;
    REQUIRE_FALSE(acts.empty());
    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        const Activation& rec = pd.path->walk_activations()[i];
        for (const bool pro : {false, true}) {
            CAPTURE(pro);
            CHECK(next_act_box_at(pd.scene, acts[i].ms, pro).detail ==
                  record_act_detail(*pd.song, rec, pro));
        }
    }

    // Pitch Dark's activations land on kicks and cymbals
    // (testdata/replay/test_pitch_dark_ratio-schedule.json), which read the
    // same in both settings. So the record is copied with its first
    // activation's chord swapped for the chart's first chord with a red pad,
    // which the setting renames (note_label, core/model.cpp).
    const Chord* red = nullptr;
    for (const SongTimestamp& ts : pd.song->sequence) {
        if (ts.chord.at(NoteColor::Red).has_value()) {
            red = &ts.chord;
            break;
        }
    }
    REQUIRE(red != nullptr);
    REQUIRE(red->rowstr(false) != red->rowstr(true));
    Path path = *pd.path;
    REQUIRE_FALSE(path.activations.empty());
    path.activations[0].chord = *red;
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    const PreviewScene scene = build_preview_scene(*pd.song, &path, cfg.sp_cap, cfg.rules);
    REQUIRE_FALSE(scene.activations.empty());
    for (const bool pro : {false, true}) {
        CAPTURE(pro);
        CHECK(next_act_box_at(scene, scene.activations[0].ms, pro).detail ==
              record_act_detail(*pd.song, path.activations[0], pro));
    }
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
    // Pitch Dark's record. Each activation's notation, whose one owner is
    // Activation::notationstr_verbose, reaches the scene and the box whole,
    // and the box's header and detail still read the activation's place and
    // the record's measure and chord.
    const PitchDarkScene pd = pitch_dark_scene();
    const std::vector<PreviewActivation>& acts = pd.scene.activations;
    REQUIRE_FALSE(acts.empty());
    for (size_t i = 0; i < acts.size(); ++i) {
        CAPTURE(i);
        const Activation& rec = pd.path->walk_activations()[i];
        CHECK(acts[i].notation == rec.notationstr_verbose());
        const PreviewNextActBox box = next_act_box_at(pd.scene, acts[i].ms, /*pro_drums=*/true);
        CHECK(box.notation == rec.notationstr_verbose());
        CHECK(box.header == next_act_header(i, acts.size()));
        CHECK(box.detail == record_act_detail(*pd.song, rec, true));
    }
    // Past the last activation the box, and its notation, are gone.
    const double past = acts.back().ms + 1000.0;  // a free input: a second past it
    CHECK(next_act_box_at(pd.scene, past, true).notation.empty());
}

TEST_CASE("build_time_box: every line switches at each change, and in the half tick before it, on a real chart") {
    // My Own Summer's chart writes tempo changes, signatures and practice
    // sections. At every tick where a tempo or a section starts, and at every
    // drawn measure line, the box reads that tick from the change's own
    // moment and from the moment just before it, which rounds to the same
    // tick, so all its lines switch together (D48, Q23). One tick earlier it
    // reads the tick before, and its tempo and section lines differ from the
    // change's exactly where the chart's own tempo, signature or section
    // differs.
    const Song& song = corpus_song("Deftones - My Own Summer");
    const PreviewScene scene = build_preview_scene(song, nullptr);
    REQUIRE(scene.timing.has_value());
    const SongTiming& timing = song.timing();
    const MsIndex& ms = timing.ms_index();

    // The name of the chart's section in force at `tick`; empty before the
    // first. Both parsers hand sections over in tick order.
    auto section_at = [&song](int64_t tick) {
        std::string name;
        for (const SongSection& s : song.practice_sections)
            if (s.tick <= tick) name = s.name;
        return name;
    };

    std::set<int64_t> changes;
    for (const int64_t t : ms.change_ticks()) changes.insert(t);
    for (const SongSection& s : song.practice_sections) changes.insert(s.tick);
    for (const PreviewBeat& b : drawn_measure_lines(scene)) changes.insert(b.tick);
    // Tick 0 has no tick before it.
    changes.erase(changes.begin(), changes.upper_bound(0));
    REQUIRE_FALSE(changes.empty());
    const double len = ms.at(*changes.rbegin()) + 1000.0;  // a free input: past the last change

    int tempo_line_moves = 0;
    int section_line_moves = 0;
    for (const int64_t t : changes) {
        CAPTURE(t);
        const double on_ms = ms.at(t);
        const PreviewTimeBox on = build_time_box(scene, on_ms, len);
        const PreviewTimeBox hair = build_time_box(
            scene, std::nextafter(on_ms, -std::numeric_limits<double>::infinity()), len);
        const PreviewTimeBox before = build_time_box(scene, ms.at(t - 1), len);

        CHECK(on.position == format_measure(timing, t));
        CHECK(hair.position == on.position);
        CHECK(hair.tempo == on.tempo);
        CHECK(hair.section_line == on.section_line);
        CHECK(before.position == format_measure(timing, t - 1));

        const bool tempo_moves = timing.tempo_at(t) != timing.tempo_at(t - 1) ||
                                 timing.time_signature_at(t) != timing.time_signature_at(t - 1);
        CHECK((on.tempo != before.tempo) == tempo_moves);
        const bool section_moves = section_at(t) != section_at(t - 1);
        CHECK((on.section_line != before.section_line) == section_moves);
        if (tempo_moves) ++tempo_line_moves;
        if (section_moves) ++section_line_moves;
    }
    // The chart's tempo changes and sections reached the loop; without them
    // it checked only measure lines.
    CHECK(tempo_line_moves > 0);
    CHECK(section_line_moves > 0);

    // A negative length shows zero: the same box as a length of zero.
    const PreviewTimeBox negative = build_time_box(scene, len / 2.0, -len);
    const PreviewTimeBox zero = build_time_box(scene, 0.0, 0.0);
    CHECK(negative.timestamp == zero.timestamp);
    CHECK(negative.position == zero.position);
    CHECK(negative.length == zero.length);
}
