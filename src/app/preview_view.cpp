// See preview_view.h. Everything here reads the song's own SongTiming, so the
// notes and the path overlay share one ms truth and never drift apart.

#include "app/preview_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <exception>
#include <optional>

#include "core/squeeze_rating.h"
#include "core/replay.h"

namespace hydra::app {

PreviewLane lane_of(NoteColor color) {
    switch (color) {
        case NoteColor::Kick:   return PreviewLane::Kick;
        case NoteColor::Red:    return PreviewLane::Red;
        case NoteColor::Yellow: return PreviewLane::Yellow;
        case NoteColor::Blue:   return PreviewLane::Blue;
        case NoteColor::Green:  return PreviewLane::Green;
    }
    return PreviewLane::Kick;  // unreachable; NoteColor is a closed set
}

namespace {

PreviewNote note_from(const SongTimestamp& ts, const ChordNote& n) {
    PreviewNote pn;
    pn.tick = ts.timecode.ticks();
    pn.ms = ts.timecode.ms();
    pn.measure = ts.timecode.measures_decimal();
    pn.lane = lane_of(n.colortype);
    pn.cymbal = n.is_cymbal();
    pn.ghost = n.is_ghost();
    pn.accent = n.is_accent();
    pn.double_kick = n.is2x;
    pn.solo = ts.flag_solo;
    return pn;
}

PreviewSpan span_from_ticks(const Song& song, int64_t start, int64_t end) {
    PreviewSpan s;
    s.start_tick = start;
    s.end_tick = end;
    s.start_ms = song.timecode(start).ms();
    s.end_ms = song.timecode(end).ms();
    return s;
}

void push_segment(SpMeterCurve& curve, double start_ms, double end_ms,
                  double start_bars, double end_bars) {
    if (end_ms <= start_ms) return;  // a zero-width stretch draws nothing
    curve.segments.push_back({start_ms, end_ms, start_bars, end_bars});
}

// The flat last segment. Always emitted, even at zero width: it is what
// sp_meter_bars_at reads back as the value after the curve ends.
void close_curve(SpMeterCurve& curve, double cursor_ms, double song_length_ms, double bank) {
    curve.segments.push_back({cursor_ms, std::max(cursor_ms, song_length_ms), bank, bank});
}

// The SP meter for a chart with no path: no record, so nothing spends the
// bank. It fills one bar at each phrase's last note and pins at the cap
// (CONTEXT.md, "SP meter gauge").
SpMeterCurve build_unanalyzed_sp_meter_curve(const PreviewScene& scene, int sp_cap) {
    SpMeterCurve curve;
    curve.cap = sp_cap < 1 ? 1 : sp_cap;
    if (scene.sp_phrases.empty()) return curve;
    const double cap = static_cast<double>(curve.cap);
    double bank = 0.0;
    double cursor_ms = 0.0;
    for (const PreviewSpan& p : scene.sp_phrases) {
        push_segment(curve, cursor_ms, p.end_ms, bank, bank);
        cursor_ms = std::max(cursor_ms, p.end_ms);
        bank = std::min(bank + 1.0, cap);
    }
    close_curve(curve, cursor_ms, scene.song_length_ms, bank);
    return curve;
}

// The SP meter along a path (see SpMeterCurve), drawn only from what the
// record stamped: each bar's arrival between windows (bank_rise_ticks,
// trailing_bank_ticks), and where each window's SP end changed, up to its
// deact node. It counts no phrase and applies no cap; the engine already did
// both. Every comparison is in ticks; ms only places the drawing.
SpMeterCurve build_sp_meter_curve(const PreviewScene& scene, const SongTiming& timing,
                                  int sp_cap) {
    SpMeterCurve curve;
    curve.cap = sp_cap < 1 ? 1 : sp_cap;
    if (scene.activations.empty() && scene.trailing_bank_ticks.empty()) return curve;

    double bank = 0.0;
    double cursor_ms = 0.0;
    // Flat, then one bar up at each stamped arrival.
    auto bank_bars = [&](const std::vector<int64_t>& rises) {
        for (int64_t t : rises) {
            const double ms = timing.ms_index().at(t);
            push_segment(curve, cursor_ms, ms, bank, bank);
            cursor_ms = std::max(cursor_ms, ms);
            bank += 1.0;
        }
    };
    // Bars left at `tick` while SP ends at `end_tick`: the measures between,
    // two to a bar. Never below empty, though a fresh record never asks for a
    // tick past the end in force.
    auto bars_left = [&](int64_t tick, int64_t end_tick) {
        const double measures = timing.measures_at_tick_f(static_cast<double>(end_tick)) -
                                timing.measures_at_tick_f(static_cast<double>(tick));
        return std::max(0.0, measures / static_cast<double>(kMeasuresPerSpBar));
    };

    // Tempos and meters are sorted by tick, and the activations come in path
    // order, which is time order. So one moving index per list finds the
    // first change past each activation note, instead of rescanning both
    // lists for every activation.
    size_t next_tempo = 0;
    size_t next_meter = 0;
    for (const PreviewActivation& act : scene.activations) {
        bank_bars(act.bank_rise_ticks);
        push_segment(curve, cursor_ms, act.ms, bank, bank);
        cursor_ms = std::max(cursor_ms, act.ms);
        bank = 0.0;  // the activation spends the bank
        // Nothing stamped (a hand-built activation): no window to draw.
        if (act.sp_end_changes.empty() || !act.has_sp_end) continue;

        // Inside the window: the measures left until the SP end in force, two
        // to a bar. Cut where the record says the end changed and at each
        // tempo and meter change, where measures stop being linear in ms.
        struct Cut {
            int64_t tick;
            double ms;
        };
        std::vector<Cut> cuts;
        for (size_t k = 1; k < act.sp_end_changes.size(); ++k) {
            const int64_t t = act.sp_end_changes[k].at_tick;
            cuts.push_back({t, timing.ms_index().at(t)});
        }
        // The tempo and meter changes strictly inside the window. Each index
        // first steps past the changes at or before this activation note; the
        // window's own changes are read from there without moving it, so a
        // window running past the next activation still sees them.
        const std::vector<PreviewTempo>& tempos = scene.tempos;
        while (next_tempo < tempos.size() && tempos[next_tempo].tick <= act.tick) ++next_tempo;
        for (size_t i = next_tempo; i < tempos.size() && tempos[i].tick < act.sp_end_tick; ++i)
            cuts.push_back({tempos[i].tick, tempos[i].ms});
        // Meters carry only ticks; their ms comes from the same index every
        // other time in the scene does.
        const std::vector<PreviewMeter>& meters = scene.meters;
        while (next_meter < meters.size() && meters[next_meter].tick <= act.tick) ++next_meter;
        for (size_t i = next_meter; i < meters.size() && meters[i].tick < act.sp_end_tick; ++i)
            cuts.push_back({meters[i].tick, timing.ms_index().at(meters[i].tick)});
        std::stable_sort(cuts.begin(), cuts.end(),
                         [](const Cut& a, const Cut& b) { return a.tick < b.tick; });

        // At a change the segment before it ends on the old end's value, and
        // the next starts on the new one's: that is the step. A change at the
        // end in force (the record's refill tick for a late phrase) ends the
        // first segment at empty and starts the next at a bar.
        size_t step = 0;  // the change in force
        int64_t prev_tick = act.tick;
        double prev_ms = act.ms;
        for (const Cut& cut : cuts) {
            const int64_t end_tick = act.sp_end_changes[step].end_tick;
            push_segment(curve, prev_ms, cut.ms, bars_left(prev_tick, end_tick),
                         bars_left(cut.tick, end_tick));
            while (step + 1 < act.sp_end_changes.size() &&
                   act.sp_end_changes[step + 1].at_tick <= cut.tick)
                ++step;
            prev_tick = cut.tick;
            prev_ms = cut.ms;
        }
        // The last stretch runs to the deact node, the last change's end:
        // empty exactly there.
        const int64_t end_tick = act.sp_end_changes[step].end_tick;
        push_segment(curve, prev_ms, act.sp_end_ms, bars_left(prev_tick, end_tick),
                     bars_left(act.sp_end_tick, end_tick));
        cursor_ms = std::max(cursor_ms, act.sp_end_ms);
    }

    // Bars banked after the last window, then the final value holds to the
    // end of the chart.
    bank_bars(scene.trailing_bank_ticks);
    close_curve(curve, cursor_ms, scene.song_length_ms, bank);
    return curve;
}

// The running score, from the same replay hydra_replay uses. Trusted only
// when replay_stored_path says the replay stands for the path.
//
// Replayed scores-only: the rows' chord_code and notes stay empty. This reads
// only ms, cum_onscreen_total, multiplier_shown, multiplier_after and
// combo_after off each row, plus result.final through faithful(); nothing
// else in this file reads a replay row.
PreviewScore build_score(const Song& song, const Path* path, const core::Rules& rules) {
    PreviewScore score;
    if (path == nullptr) return score;  // None
    score.state = PreviewScore::State::Unavailable;
    try {
        ReplayOptions options;
        options.scores_only = true;
        const PathReplay pr = replay_stored_path(song, *path, rules, options);
        if (!pr.faithful()) return score;
        score.steps.reserve(pr.result.chords.size());
        for (const ReplayChord& c : pr.result.chords)
            score.steps.push_back({c.ms, c.cum_onscreen_total, c.multiplier_shown,
                                   c.multiplier_after, c.combo_after});
    } catch (const std::exception&) {
        score.steps.clear();
        return score;  // Unavailable
    }
    score.state = PreviewScore::State::Ready;
    return score;
}

// The activation whose Star Power is running at `now`, or nullptr. Running
// from the activation chord up to, not including, the SP end. The drain box
// and the score box both ask this, so they cannot disagree. A record with no
// deact node cannot say, so it never counts as running.
const PreviewActivation* running_activation(const PreviewScene& scene, double now) {
    for (const PreviewActivation& a : scene.activations)
        if (a.has_sp_end && a.ms <= now && now < a.sp_end_ms) return &a;
    return nullptr;
}

}  // namespace

PreviewScene build_preview_scene(const Song& song, const Path* path, int sp_cap,
                                 const core::Rules& rules) {
    return apply_preview_overlay(build_preview_base(song), song, path, sp_cap, rules);
}

PreviewScene build_preview_base(const Song& song) {
    PreviewScene scene;
    if (song.is_empty()) return scene;

    bool in_solo = false;
    int64_t solo_start = 0;
    int64_t solo_last = 0;

    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& n : ts.chord.notes())
            scene.notes.push_back(note_from(ts, n));

        // Solo: coalesce a run of flagged timestamps into one span.
        if (ts.flag_solo) {
            if (!in_solo) {
                in_solo = true;
                solo_start = ts.timecode.ticks();
            }
            solo_last = ts.timecode.ticks();
        } else if (in_solo) {
            scene.solos.push_back(span_from_ticks(song, solo_start, solo_last));
            in_solo = false;
        }

        // SP phrase: the end note carries the start tick the parser kept.
        if (ts.flag_sp && ts.sp_phrase_start.has_value())
            scene.sp_phrases.push_back(
                span_from_ticks(song, *ts.sp_phrase_start, ts.timecode.ticks()));

        // Activation fill: the window ends at this note, `activation_length`
        // ticks wide.
        if (ts.activation_length.has_value()) {
            int64_t end = ts.timecode.ticks();
            PreviewFill f;
            f.span = span_from_ticks(song, end - *ts.activation_length, end);
            scene.fills.push_back(f);  // state is filled in from the path below
        }
    }
    if (in_solo)
        scene.solos.push_back(span_from_ticks(song, solo_start, solo_last));

    scene.has_notes = !scene.notes.empty();
    if (scene.has_notes) scene.song_length_ms = scene.notes.back().ms;

    // The beat grid runs two measures past the last note so lines keep
    // scrolling through the look-ahead after the chart ends.
    const SongTiming& timing = song.timing();
    scene.timing = timing;  // the time box names ticks with the engine's math
    scene.tick_resolution = timing.tick_resolution();
    if (scene.has_notes) {
        int64_t last_tick = scene.notes.back().tick;
        const MeasureIndex& mi = timing.measure_index();
        int64_t tpm = mi.tpm_at(mi.section_at(last_tick));
        scene.beats = build_beat_events(timing, last_tick + 2 * tpm);
    }
    for (const auto& kv : song.bpm_changes) {
        PreviewTempo t;
        t.tick = kv.first;
        t.ms = timing.ms_index().at(kv.first);
        t.bpm = kv.second;
        scene.tempos.push_back(t);
    }
    {
        const MeasureIndex& mi = timing.measure_index();
        for (int i = 0; i < mi.count(); ++i) {
            if (mi.tpm_at(i) <= 0) continue;
            scene.meters.push_back({mi.keys_at(i), mi.tpm_at(i), mi.starts_at(i)});
        }
    }
    for (const auto& [tick, sig] : song.timesig_changes)
        scene.time_sigs.push_back({tick, sig.first, sig.second});
    // A section marker can sit past the last note, where the ms index does not
    // reach; the timing's own timecode extrapolates instead.
    for (const SongSection& s : song.practice_sections)
        scene.sections.push_back({s.tick, song.timecode(s.tick).ms(), s.name});
    return scene;
}

PreviewScene apply_preview_overlay(PreviewScene scene, const Song& song, const Path* path,
                                   int sp_cap, const core::Rules& rules) {
    // Drop any overlay the base already carries; the fields go back to what a
    // fresh base holds.
    scene.activations.clear();
    scene.trailing_bank_ticks.clear();
    for (PreviewFill& f : scene.fills) f.state = PreviewFillState::Hidden;
    scene.score = PreviewScore{};
    scene.sp_meter = SpMeterCurve{};
    if (song.is_empty()) return scene;
    const SongTiming& timing = song.timing();

    // Overlay: the path's activations, ms resolved against the song's timing.
    if (path != nullptr) {
        for (const Activation& a : path->walk_activations()) {
            PreviewActivation pa;
            pa.tick = a.timecode.ticks();
            pa.ms = song.timecode(pa.tick).ms();
            pa.skipped_fill_ticks = a.skipped_fill_ticks;
            pa.bank_rise_ticks = a.bank_rise_ticks;
            for (size_t k = 0; k < a.sp_end_steps.size(); ++k)
                pa.sp_end_changes.push_back({a.refill_tick(k), a.sp_end_steps[k].end_tick});
            if (std::optional<int64_t> d = activation_deact_tick(a)) {
                pa.has_sp_end = true;
                pa.sp_end_tick = *d;
                pa.sp_end_ms = timing.ms_index().at(*d);
            }
            if (a.chord.count() > 0) {
                pa.has_lane = true;
                pa.lane = lane_of(a.chord.activation_note().colortype);
                pa.chord = a.chord.rowstr();
            }
            pa.measure = format_measure(timing, pa.tick);
            scene.activations.push_back(pa);
        }
        scene.trailing_bank_ticks = path->trailing_bank_ticks;
    }

    // Which fills the game would have shown. Without a path we cannot know, so
    // every candidate is drawn as offered.
    if (path == nullptr) {
        for (PreviewFill& f : scene.fills) f.state = PreviewFillState::Offered;
    } else {
        // Which fills the game showed is engine truth: each activation lists
        // the fills its path passed over, and its own fill is the taken one.
        // Every other fill stays hidden, including those after the last
        // activation, which the engine records nothing about. Fills and the
        // stored ticks are both in chart order, so one index walks them.
        std::vector<PreviewFill>& fills = scene.fills;
        size_t next_fill = 0;
        auto mark = [&](int64_t tick, PreviewFillState state) {
            while (next_fill < fills.size() && fills[next_fill].span.end_tick < tick) ++next_fill;
            if (next_fill < fills.size() && fills[next_fill].span.end_tick == tick)
                fills[next_fill].state = state;
        };
        for (const PreviewActivation& a : scene.activations) {
            for (int64_t t : a.skipped_fill_ticks) mark(t, PreviewFillState::Offered);
            mark(a.tick, PreviewFillState::Taken);
        }
    }

    // The SP meter, built last. With a path it reads the record's facts
    // gathered above; without one, the chart's phrases.
    scene.score = build_score(song, path, rules);
    scene.sp_meter = path != nullptr ? build_sp_meter_curve(scene, timing, sp_cap)
                                     : build_unanalyzed_sp_meter_curve(scene, sp_cap);
    return scene;
}

double sp_meter_bars_at(const SpMeterCurve& curve, double ms) {
    if (curve.segments.empty()) return 0.0;
    if (ms < curve.segments.front().start_ms) return 0.0;
    if (ms >= curve.segments.back().end_ms) return curve.segments.back().end_bars;
    // The last segment that has started: on a boundary two segments share, that
    // is the later one, which is what makes a step read as a step.
    auto it = std::upper_bound(curve.segments.begin(), curve.segments.end(), ms,
                               [](double v, const SpMeterSegment& s) { return v < s.start_ms; });
    const SpMeterSegment& s = *(it - 1);
    const double span = s.end_ms - s.start_ms;
    if (span <= 0.0) return s.end_bars;
    const double t = (ms - s.start_ms) / span;
    return s.start_bars + (s.end_bars - s.start_bars) * t;
}

std::vector<PreviewBeat> build_beat_events(const SongTiming& timing, int64_t last_tick) {
    std::vector<PreviewBeat> lines;
    const MeasureIndex& mi = timing.measure_index();
    const int64_t tick_r = timing.tick_resolution();
    for (int i = 0; i < mi.count(); ++i) {
        const int64_t section_start = mi.keys_at(i);
        const int64_t tpm = mi.tpm_at(i);
        if (tpm <= 0) continue;
        // The section's bars count from its last barline at or before its
        // first tick (a mid-measure meter change restarts the measure there).
        // Bars before the section's own start belong to the section before.
        const bool last_section = i + 1 >= mi.count();
        const int64_t section_end = last_section ? last_tick : mi.keys_at(i + 1);
        for (int64_t bar = mi.starts_at(i);; bar += tpm) {
            if (last_section ? bar > section_end : bar >= section_end) break;
            if (bar >= section_start) lines.push_back({bar, 0.0, PreviewBeatKind::Bar});
            for (int64_t beat = bar + tick_r; beat < bar + tpm; beat += tick_r) {
                if (last_section ? beat > section_end : beat >= section_end) break;
                if (beat >= section_start) lines.push_back({beat, 0.0, PreviewBeatKind::Beat});
            }
        }
    }
    // Onyx draws a fainter half-beat line half a beat before every bar/beat
    // line (all our lines are a full beat apart, so every one qualifies);
    // nothing is drawn before tick 0.
    std::vector<PreviewBeat> out;
    out.reserve(lines.size() * 2);
    const int64_t half = tick_r / 2;
    for (const PreviewBeat& l : lines) {
        if (l.tick - half >= 0) out.push_back({l.tick - half, 0.0, PreviewBeatKind::Half});
        out.push_back(l);
    }
    for (PreviewBeat& b : out) b.ms = timing.ms_index().at(b.tick);
    return out;
}

namespace {

std::string clock_str(double ms) {
    double secs = ms / 1000.0;
    int minutes = static_cast<int>(secs / 60.0);
    double rem = secs - minutes * 60.0;
    char buf[48];
    std::snprintf(buf, sizeof buf, "%d:%06.3f", minutes, rem);
    return buf;
}

// The tick at `ms`, from the song's own ms index. The last tempo section's
// slope holds forever after it, so this keeps counting past the end of the
// chart. Rounded to the nearest tick and never negative.
int64_t tick_at(const SongTiming& timing, double ms) {
    const int64_t tick = std::llround(timing.ms_index().tick_at_ms(ms));
    return tick < 0 ? 0 : tick;
}

// The song length the time box shows: never negative.
double shown_length(double length_ms) { return length_ms < 0.0 ? 0.0 : length_ms; }

// The moment the time box shows: the playhead held inside [0, length].
// step_tick_ms starts from the same moment, so a step moves the displayed
// tick by exactly its size.
double shown_ms(double now_ms, double length_ms) {
    const double len = shown_length(length_ms);
    return now_ms < 0.0 ? 0.0 : (now_ms > len ? len : now_ms);
}

}  // namespace

PreviewTimeBox build_time_box(const PreviewScene& scene, double now_ms,
                              double length_ms) {
    PreviewTimeBox box;

    const double len = shown_length(length_ms);
    const double now = shown_ms(now_ms, length_ms);

    box.timestamp = clock_str(now) + " / " + clock_str(len);

    // A default-built scene carries no song and so no timing: it reads as tick
    // 0 at measure 1, beat 1, exactly as it always has.
    const int64_t now_tick = scene.timing ? tick_at(*scene.timing, now) : 0;
    const int64_t end_tick = scene.timing ? tick_at(*scene.timing, len) : 0;
    box.position = scene.timing ? format_measure(*scene.timing, now_tick) : "m1.1.0";
    box.length = scene.timing ? format_measure(*scene.timing, end_tick) : "m1.1.0";

    // Each lookup below wants the last entry at or before the playhead, which
    // is the one just before the first entry past it. Tempos and time
    // signatures come from the song's tick-keyed maps, so they are sorted by
    // tick (and the tempos by ms too), and a binary search finds that entry.

    // The tempo in force: the last change at or before now (the opening tempo
    // before any change).
    double bpm = scene.tempos.empty() ? 0.0 : scene.tempos.front().bpm;
    const auto tempo_past = std::upper_bound(
        scene.tempos.begin(), scene.tempos.end(), now,
        [](double v, const PreviewTempo& t) { return v < t.ms; });
    if (tempo_past != scene.tempos.begin()) bpm = (tempo_past - 1)->bpm;
    // The time signature in force at the playhead's tick, as the chart wrote
    // it; 4/4 before any, the chart default.
    int ts_num = 4, ts_den = 4;
    const auto sig_past = std::upper_bound(
        scene.time_sigs.begin(), scene.time_sigs.end(), now_tick,
        [](int64_t v, const PreviewTimeSig& t) { return v < t.tick; });
    if (sig_past != scene.time_sigs.begin()) {
        ts_num = (sig_past - 1)->numerator;
        ts_den = (sig_past - 1)->denominator;
    }
    char buf[64];
    std::snprintf(buf, sizeof buf, "BPM %.3f \xC2\xB7 %d/%d", bpm, ts_num, ts_den);
    box.tempo = buf;

    // Sections are in tick order on every chart but one kind: a MIDI file
    // with more than one EVENTS track lists each track's sections in turn.
    // The answer has always been "the section before the first one past the
    // playhead, front to back", so an out-of-order list keeps that scan and
    // reads exactly as before; a sorted one gets the binary search.
    const std::vector<PreviewSection>& sections = scene.sections;
    const auto by_tick = [](const PreviewSection& a, const PreviewSection& b) {
        return a.tick < b.tick;
    };
    const auto section_past =
        std::is_sorted(sections.begin(), sections.end(), by_tick)
            ? std::upper_bound(sections.begin(), sections.end(), now_tick,
                               [](int64_t v, const PreviewSection& s) { return v < s.tick; })
            : std::find_if(sections.begin(), sections.end(),
                           [now_tick](const PreviewSection& s) { return s.tick > now_tick; });
    if (section_past != sections.begin() && !(section_past - 1)->name.empty())
        box.section_line = "Section " + (section_past - 1)->name;
    return box;
}

PreviewScoreBox build_score_box(const PreviewScene& scene, double now_ms) {
    PreviewScoreBox box;
    if (scene.score.state == PreviewScore::State::None) return box;
    box.shown = true;
    if (scene.score.state == PreviewScore::State::Unavailable) {
        box.score = "Score unavailable";
        return box;
    }
    box.available = true;

    // The last chord at or before the playhead. Before the first chord
    // nothing is hit yet: 0, x1, combo 0.
    const std::vector<PreviewScoreStep>& steps = scene.score.steps;
    auto it = std::upper_bound(steps.begin(), steps.end(), now_ms,
                               [](double v, const PreviewScoreStep& s) { return v < s.ms; });
    PreviewScoreStep at;
    if (it != steps.begin()) at = *(it - 1);

    // The game's disc drops to the plain multiplier the moment SP ends, not
    // at the next chord, so the doubled value shows only while SP runs.
    const int shown =
        running_activation(scene, now_ms) != nullptr ? at.multiplier : at.multiplier_plain;

    box.score = group_thousands(at.total);
    char buf[64];
    std::snprintf(buf, sizeof buf, "x%d \xC2\xB7 combo %d", shown, at.combo);
    box.detail = buf;
    return box;
}

PreviewDrainBox build_drain_box(const PreviewScene& scene, double now_ms) {
    PreviewDrainBox box;
    if (!scene.timing || scene.sp_meter.segments.empty()) return box;
    box.shown = true;

    const SongTiming& timing = *scene.timing;
    const double now = now_ms < 0.0 ? 0.0 : now_ms;
    const int64_t now_tick = tick_at(timing, now);

    // The rule's own constant at the local measure length: on a tempo or
    // meter change the new section is read, as the time box's BPM line does.
    char buf[64];
    const double bar_ms =
        static_cast<double>(kMeasuresPerSpBar) * timing.ms_per_measure_at(now_tick);
    std::snprintf(buf, sizeof buf, "1 bar / %.1f s", bar_ms / 1000.0);
    box.rate = buf;

    // SP is running when the playhead sits in an activation's stored window.
    // A record with no deact node cannot say, so it reads idle. The score box
    // asks the same helper.
    const PreviewActivation* running = running_activation(scene, now);

    if (running != nullptr) {
        box.active = true;
        box.header = "SP drain";
        std::snprintf(buf, sizeof buf, "empties in %.1f s",
                      (running->sp_end_ms - now) / 1000.0);
    } else {
        box.header = "SP drain (if activated)";
        // The cap at the bar time in force here, so it jumps exactly when the
        // rate line does rather than blending in the sections ahead.
        std::snprintf(buf, sizeof buf, "full meter %.1f s",
                      bar_ms * static_cast<double>(scene.sp_meter.cap) / 1000.0);
    }
    box.detail = buf;
    return box;
}

double step_tick_ms(const PreviewScene& scene, double now_ms, double length_ms,
                    int delta_ticks) {
    if (!scene.timing) return now_ms;
    int64_t target = tick_at(*scene.timing, shown_ms(now_ms, length_ms)) + delta_ticks;
    if (target < 0) target = 0;
    return scene.timing->ms_index().at(target);
}

std::vector<double> build_scrub_marks(const PreviewScene& scene, double length_ms) {
    std::vector<double> marks;
    if (length_ms <= 0.0) return marks;
    marks.reserve(scene.activations.size());
    for (const PreviewActivation& a : scene.activations)
        marks.push_back(std::clamp(a.ms / length_ms, 0.0, 1.0));
    return marks;
}

namespace {
// How far from an activation the playhead may sit and still count as on it.
constexpr double kOnActivationMs = 0.5;
}  // namespace

std::optional<double> activation_jump_ms(const PreviewScene& scene, double now_ms,
                                         int direction) {
    if (direction > 0) {
        for (const PreviewActivation& a : scene.activations)
            if (a.ms > now_ms + kOnActivationMs) return a.ms;
        return std::nullopt;
    }
    for (auto it = scene.activations.rbegin(); it != scene.activations.rend(); ++it)
        if (it->ms < now_ms - kOnActivationMs) return it->ms;
    return std::nullopt;
}

PreviewNextActBox build_next_act_box(const PreviewScene& scene, double now_ms) {
    PreviewNextActBox box;
    // The first activation not yet behind the playhead. The activations are
    // in time order, so a binary search finds it.
    const std::vector<PreviewActivation>& acts = scene.activations;
    const auto next = std::lower_bound(
        acts.begin(), acts.end(), now_ms - kOnActivationMs,
        [](const PreviewActivation& a, double v) { return a.ms < v; });
    if (next == acts.end()) return box;
    const size_t i = static_cast<size_t>(next - acts.begin());
    box.shown = true;
    box.header = "Next: activation " + std::to_string(i + 1) + " of " + std::to_string(acts.size());
    box.detail = "at " + next->measure;
    if (!next->chord.empty()) box.detail += " \xC2\xB7 " + next->chord;
    return box;
}

std::string sp_meter_readout(const SpMeterCurve& curve, double now_ms) {
    if (curve.segments.empty()) return {};
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.1f/%d", sp_meter_bars_at(curve, now_ms), curve.cap);
    return buf;
}

std::string preview_path_label(const PathButtonView& button) {
    switch (button.group) {
        case PathButtonView::Group::Optimal: return button.notation + "  (optimal)";
        case PathButtonView::Group::AllZero: return button.notation + "  (best all-0)";
        case PathButtonView::Group::Within:  break;
    }
    return button.notation;
}

std::string path_overlay_key(const Path* path) {
    if (path == nullptr) return {};
    // The key only has to tell paths of one chart apart (the controller
    // compares it after matching the chart), and a chart's multiplier
    // squeezes are the same for every path, so they are left out.
    return path->pathstring_verbose({}) + "|" + std::to_string(path->totalscore());
}

}  // namespace hydra::app
