// See preview_view.h. Everything here reads the song's own SongTiming, so the
// notes and the path overlay share one ms truth and never drift apart.

#include "app/preview_view.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <exception>
#include <optional>
#include <tuple>

#include "app/config.h"          // Settings: the sp_cap setting's floor
#include "app/display_format.h"  // clock_str, kFactSeparator
#include "core/replay.h"
#include "core/scoring.h"  // solo_bonus, solo_note_count

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

// Searches lane_of rather than keeping a second table that could drift.
NoteColor color_of(PreviewLane lane) {
    for (NoteColor c : {NoteColor::Kick, NoteColor::Red, NoteColor::Yellow, NoteColor::Blue,
                        NoteColor::Green})
        if (lane_of(c) == lane) return c;
    return NoteColor::Kick;  // unreachable; every lane comes from a colour
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

// The Preview's drawn length of a generated fill, in measures of Clone Hero's
// grid at the fill's chord, as the user chose for how the Preview shows a
// generated fill. Only the drawing reads it; the engine's fill length
// (SongTimestamp::activation_length) is left as the generator set it.
constexpr double kGeneratedFillDrawnMeasures = 0.5;

// Where the Preview starts drawing the fill that ends on `ts`. An authored
// fill, and a Clone Hero 1.0 generated fill (already that half measure long,
// by place_fills_ch10), draw from fill_start_tick. A Clone Hero 1.1 generated
// fill starts on its measure line, often at its own chord, so it draws
// kGeneratedFillDrawnMeasures of the grid measure in force at the chord
// (MeasureIndex) before the chord instead.
int64_t drawn_fill_start_tick(const Song& song, const SongTimestamp& ts) {
    const int64_t chord = ts.timecode.ticks();
    if (song.generated_fills != FillDeadlineRule::Ch11)
        return fill_start_tick(chord, *ts.activation_length);
    const MeasureIndex& grid = song.timing().measure_index();
    const int64_t measure = grid.length_at(grid.span_at(chord));
    return chord - static_cast<int64_t>(static_cast<double>(measure) * kGeneratedFillDrawnMeasures);
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

// The meter's ceiling for a cap: the cap, raised to the smallest the sp_cap
// setting allows. That minimum is typed once, in the settings key table
// (finding 139); both meters below ask here.
int sp_meter_cap_of(int sp_cap) { return Settings::clamp(&Settings::sp_cap, sp_cap); }

// The SP meter for a chart with no path: no record, so nothing spends the
// bank. It fills one bar at each phrase's last note and pins at the cap
// (CONTEXT.md, "SP meter gauge").
SpMeterCurve build_unanalyzed_sp_meter_curve(const PreviewScene& scene, int sp_cap) {
    SpMeterCurve curve;
    curve.cap = sp_meter_cap_of(sp_cap);
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
    curve.cap = sp_meter_cap_of(sp_cap);
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
        // Nothing stamped (a hand-built activation): no window to draw. The
        // changes and has_sp_end both come from the record's SP-end steps, so
        // the list being empty is the one check.
        if (act.sp_end_changes.empty()) continue;

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
        // The meter changes are the continuous count's span starts, because
        // bars_left reads that count. They carry only ticks; their ms comes
        // from the same index every other time in the scene does.
        const std::vector<PreviewMeasureSpan>& spans = scene.measure_spans;
        while (next_meter < spans.size() && spans[next_meter].tick <= act.tick) ++next_meter;
        for (size_t i = next_meter; i < spans.size() && spans[i].tick < act.sp_end_tick; ++i)
            cuts.push_back({spans[i].tick, timing.ms_index().at(spans[i].tick)});
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
// only ms, cum_onscreen_total, cum, multiplier_shown, multiplier_after and
// combo_after off each row, plus result.final through faithful(); nothing
// else in this file reads a replay row.
PreviewScore build_score(const Song& song, const Path* path, const core::Rules& rules) {
    PreviewScore score;
    if (path == nullptr) return score;  // None
    score.state = PreviewScore::State::Unavailable;
    score.cutoffs = star_cutoffs(*path);
    try {
        ReplayOptions options;
        options.scores_only = true;
        const PathReplay pr = replay_stored_path(song, *path, rules, options);
        if (!pr.faithful()) return score;
        score.steps.reserve(pr.result.chords.size());
        for (const ReplayChord& c : pr.result.chords)
            score.steps.push_back({c.ms, c.cum_onscreen_total, c.multiplier_shown,
                                   c.multiplier_after, c.combo_after,
                                   score_without_solo(c.cum)});
    } catch (const std::exception&) {
        score.steps.clear();
        return score;  // Unavailable
    }
    score.state = PreviewScore::State::Ready;
    return score;
}

// The activation whose Star Power is running at `now`, or nullptr. Each
// activation answers through PreviewActivation::sp_running_at. The drain box
// and the score box both ask this, so they cannot disagree.
const PreviewActivation* running_activation(const PreviewScene& scene, double now) {
    for (const PreviewActivation& a : scene.activations)
        if (a.sp_running_at(now)) return &a;
    return nullptr;
}

}  // namespace

const PreviewNote* last_drawn_note(const PreviewScene& scene) {
    return scene.notes.empty() ? nullptr : &scene.notes.back();
}

double last_note_ms(const PreviewScene& scene) {
    const PreviewNote* last = last_drawn_note(scene);
    return last ? last->ms : 0.0;
}

PreviewScene build_preview_scene(const Song& song, const Path* path, int sp_cap,
                                 const core::Rules& rules, std::optional<double> audio_end_ms,
                                 std::optional<double> song_length_ms) {
    return apply_preview_overlay(build_preview_base(song, audio_end_ms, song_length_ms), song,
                                 path, sp_cap, rules);
}

PreviewScene build_preview_base(const Song& song, std::optional<double> audio_end_ms,
                                std::optional<double> song_length_ms) {
    PreviewScene scene;
    if (song.is_empty()) return scene;

    for (const SongTimestamp& ts : song.sequence) {
        for (const ChordNote& n : ts.chord.note_list())
            scene.notes.push_back(note_from(ts, n));

        // SP phrase: the end note carries the start tick the parser kept.
        if (ts.flag_sp && ts.sp_phrase_start.has_value())
            scene.sp_phrases.push_back(
                span_from_ticks(song, *ts.sp_phrase_start, ts.timecode.ticks()));

        // Activation fill: the window runs between this note and where
        // drawn_fill_start_tick says the drawing starts, whichever comes
        // first. An authored fill whose chord lands just before its measure
        // line has a start after the chord.
        if (ts.activation_length.has_value()) {
            const int64_t chord = ts.timecode.ticks();
            const int64_t start = drawn_fill_start_tick(song, ts);
            PreviewFill f;
            f.span = span_from_ticks(song, std::min(start, chord), std::max(start, chord));
            f.chord_tick = chord;
            scene.fills.push_back(f);  // state is filled in from the path below
        }
    }
    // Solo spans: one per section the song names (Song::solo_sections), from
    // its first chord to its last. Beside each, the notes the solo box
    // counts (solo_note_count) and the bonus (solo_bonus).
    for (const SoloSection& s : song.solo_sections) {
        scene.solos.push_back(span_from_ticks(song, song.sequence[s.first].timecode.ticks(),
                                              song.sequence[s.last].timecode.ticks()));
        PreviewSoloNotes notes;
        for (size_t i = s.first; i <= s.last; ++i) {
            const SongTimestamp& ts = song.sequence[i];
            notes.bonus += solo_bonus(ts.chord, ts.flag_solo);
            notes.notes += solo_note_count(ts.chord, ts.flag_solo);
            notes.steps.push_back({ts.timecode.ms(), notes.notes});
        }
        scene.solo_notes.push_back(std::move(notes));
    }
    // Drum lanes: each one's span as the chart wrote it, lighting the pads
    // the loader chose (Song::drum_lanes; the rule is light_drum_lanes).
    for (const DrumLane& l : song.drum_lanes) {
        PreviewDrumLane d;
        d.span = span_from_ticks(song, l.start_tick, l.end_tick);
        for (NoteColor c : l.pads) d.pads.push_back(lane_of(c));
        scene.drum_lanes.push_back(std::move(d));
    }

    scene.has_notes = !scene.notes.empty();
    // The song's length is app::song_length_ms's answer (D75); the load hands
    // it in. None given, no length.
    scene.song_length_ms = song_length_ms.value_or(0.0);

    // The beat grid runs to the end of the audio, so lines keep scrolling
    // while music plays past the last note (D48, Q25). The end is the tick a
    // playhead there shows, the same rule the time box uses. Without the
    // audio's end it runs two measures past the last drawn note
    // (last_drawn_note), counted in ticks because beat lines sit on measures
    // and beats.
    const SongTiming& timing = song.timing();
    scene.timing = timing;  // the time box names ticks with the engine's math
    if (const PreviewNote* last = last_drawn_note(scene)) {
        const int64_t last_tick = last->tick;
        int64_t grid_end = last_tick;
        if (audio_end_ms.has_value()) {
            grid_end = std::max(last_tick, timing.display_tick_at_ms(*audio_end_ms));
        } else {
            const MeasureIndex& mi = timing.measure_index();
            grid_end = last_tick + 2 * mi.length_at(mi.span_at(last_tick));
        }
        scene.beats = build_beat_events(timing, grid_end);
    }
    for (const auto& kv : song.bpm_changes) {
        PreviewTempo t;
        t.tick = kv.first;
        t.ms = timing.ms_index().at(kv.first);
        t.bpm = kv.second;
        scene.tempos.push_back(t);
    }
    {
        const MeasureCount& mc = timing.measure_count();
        for (int i = 0; i < mc.count(); ++i) {
            if (mc.tpm_at(i) <= 0) continue;
            scene.measure_spans.push_back({mc.start_at(i), mc.tpm_at(i)});
        }
    }
    // A section marker can sit past the last note. That is fine: the ms index
    // covers every tick (MsIndex::at keeps the last tempo's slope), and
    // timecode(t).ms() reads that same index.
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
            if (std::optional<int64_t> d = a.deact_tick()) {
                pa.has_sp_end = true;
                pa.sp_end_tick = *d;
                pa.sp_end_ms = timing.ms_index().at(*d);
            }
            if (a.chord.count() > 0) {
                pa.has_lane = true;
                pa.lane = lane_of(a.chord.activation_note().colortype);
                pa.chord = a.chord;
            }
            pa.measure = format_measure(timing, pa.tick);
            pa.notation = a.notationstr_verbose();
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
        // A taken fill stays taken: a tied variant's later activation can list
        // a fill the variant activated on, because after the fold it carries
        // its leader's passed-over fills (finding 97). Each activation keeps
        // the index of the fill it stands on (taken_fill), whether it marked
        // that fill or an earlier activation did; the highway reads it.
        std::vector<PreviewFill>& fills = scene.fills;
        size_t next_fill = 0;
        // Marks the fill whose chord is `tick` and returns its index, or
        // nothing when no fill rides on that chord.
        auto mark = [&](int64_t tick, PreviewFillState state) -> std::optional<size_t> {
            while (next_fill < fills.size() && fills[next_fill].chord_tick < tick) ++next_fill;
            if (next_fill >= fills.size() || fills[next_fill].chord_tick != tick)
                return std::nullopt;
            if (fills[next_fill].state != PreviewFillState::Taken) fills[next_fill].state = state;
            return next_fill;
        };
        for (PreviewActivation& a : scene.activations) {
            for (int64_t t : a.skipped_fill_ticks) mark(t, PreviewFillState::Offered);
            a.taken_fill = mark(a.tick, PreviewFillState::Taken);
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
    std::vector<PreviewBeat> out;
    // Onyx draws a fainter line halfway between two lines, so each line after
    // the first gets one half its gap back (the step it ends, which is the
    // span before's step at a restart). Nothing is drawn before tick 0.
    std::optional<int64_t> prev;
    timing.measure_index().walk(last_tick, [&](int64_t tick, MeasureIndex::Line line) {
        if (prev) out.push_back({tick - (tick - *prev) / 2, 0.0, PreviewBeatKind::Half});
        out.push_back({tick, 0.0,
                       line == MeasureIndex::Line::Measure ? PreviewBeatKind::MeasureLine
                                                           : PreviewBeatKind::Beat});
        prev = tick;
    });
    for (PreviewBeat& b : out) b.ms = timing.ms_index().at(b.tick);
    return out;
}

namespace {

// Where playback ends, as the time box shows it: never negative.
double shown_length(double length_ms) { return length_ms < 0.0 ? 0.0 : length_ms; }

// The moment the time box shows: the playhead held inside [0, length].
// step_shown_ms starts from the same moment, so a step moves the displayed
// time by exactly its size, and the score, drain and next-activation boxes
// read it too, so every overlay box describes the same moment.
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
    const int64_t now_tick = scene.timing ? scene.timing->display_tick_at_ms(now) : 0;
    const int64_t end_tick = scene.timing ? scene.timing->display_tick_at_ms(len) : 0;
    box.position = scene.timing ? format_measure(*scene.timing, now_tick) : first_measure_label();
    box.length = scene.timing ? format_measure(*scene.timing, end_tick) : first_measure_label();

    // Every line below asks the same rounded tick, so the tempo, time
    // signature, section and measure all switch together, even in the half
    // tick before a change. The time signature is the grid's, so it switches
    // where the measure lines restart. A scene built from nothing shows BPM 0
    // and PreviewTimeSig's default, which is Song's.
    double bpm = 0.0;
    PreviewTimeSig sig;
    if (scene.timing) {
        bpm = scene.timing->tempo_at(now_tick);
        std::tie(sig.numerator, sig.denominator) = scene.timing->time_signature_at(now_tick);
    }
    char buf[64];
    std::snprintf(buf, sizeof buf, "BPM %.3f%s%d/%d", bpm, kFactSeparator, sig.numerator,
                  sig.denominator);
    box.tempo = buf;

    // Both parsers hand sections over in tick order (sort_practice_sections).
    const std::vector<PreviewSection>& sections = scene.sections;
    const auto section = last_at_or_before(sections.begin(), sections.end(), now_tick,
                                           [](const PreviewSection& s) { return s.tick; });
    if (section != sections.end() && !section->name.empty())
        box.section_line = "Section " + section->name;
    return box;
}

PreviewScoreBox build_score_box(const PreviewScene& scene, double now_ms, double length_ms) {
    PreviewScoreBox box;
    const double now = shown_ms(now_ms, length_ms);
    if (scene.score.state == PreviewScore::State::None) return box;
    box.shown = true;
    if (scene.score.state == PreviewScore::State::Unavailable) {
        box.score = "Score unavailable";
        return box;
    }
    box.available = true;

    // The last chord struck at the playhead (struck_at, which the highway's
    // flash asks too). Before the first chord nothing is hit yet: 0, x1,
    // combo 0.
    const std::vector<PreviewScoreStep>& steps = scene.score.steps;
    auto it = std::partition_point(steps.begin(), steps.end(), [now](const PreviewScoreStep& s) {
        return struck_at(now, s.ms);
    });
    PreviewScoreStep at;
    if (it != steps.begin()) at = *(it - 1);

    // The game's disc drops to the plain multiplier the moment SP ends, not
    // at the next chord, so the doubled value shows only while SP runs.
    const int shown =
        running_activation(scene, now) != nullptr ? at.multiplier : at.multiplier_plain;

    box.score = group_thousands(at.total);
    char buf[64];
    std::snprintf(buf, sizeof buf, "x%d%scombo %d", shown, kFactSeparator, at.combo);
    box.detail = buf;

    // The star counter, on the score stars count (core/stars.h).
    const StarCutoffs& cut = scene.score.cutoffs;
    box.stars = stars_for_score(cut, at.score_without_solo);
    if (box.stars < kMaxStars)
        box.next_star = "star " + std::to_string(box.stars + 1) + " in " +
                        group_thousands(cut.cutoffs[box.stars] - at.score_without_solo);
    return box;
}

PreviewDrainBox build_drain_box(const PreviewScene& scene, double now_ms, double length_ms) {
    PreviewDrainBox box;
    if (!scene.has_sp_gauge()) return box;
    box.shown = true;

    const SongTiming& timing = *scene.timing;
    const double now = shown_ms(now_ms, length_ms);
    const int64_t now_tick = timing.display_tick_at_ms(now);

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

int solo_percent(int hit, int total) {
    if (total <= 0) return 0;
    // Each step in float, as the game does it: the quotient is rounded to
    // float before the multiply, and the product before the truncation.
    const float share = static_cast<float>(hit) / static_cast<float>(total);
    const float percent = share * 100.0f;
    return static_cast<int>(percent);
}

namespace {

// One solo's box with the playhead at `now`, which is at or after the solo's
// first chord: the running percent and count until the last chord is struck,
// then the end message, then the bonus, then nothing.
PreviewSoloBox solo_box_of(const PreviewSoloNotes& solo, double now) {
    PreviewSoloBox box;
    const double last_ms = solo.steps.back().ms;
    if (struck_at(now, last_ms)) {
        const double since = now - last_ms;
        if (since >= kSoloTailMs) return box;
        box.shown = true;
        box.headline = since < kSoloResultMs ? "Perfect Solo!" : "+" + group_thousands(solo.bonus);
        return box;
    }
    // The last chord struck at the playhead.
    const auto it = std::partition_point(
        solo.steps.begin(), solo.steps.end(),
        [now](const PreviewSoloNotes::Step& s) { return struck_at(now, s.ms); });
    const int hit = it == solo.steps.begin() ? 0 : (it - 1)->notes_hit;
    box.shown = true;
    box.headline = std::to_string(solo_percent(hit, solo.notes)) + "%";
    box.detail = group_thousands(hit) + " / " + counted(solo.notes, "note", "notes");
    return box;
}

}  // namespace

PreviewSoloBox build_solo_box(const PreviewScene& scene, double now_ms, double length_ms) {
    const double now = shown_ms(now_ms, length_ms);
    // The solo whose first chord was struck last. Solos come in time order.
    const std::vector<PreviewSoloNotes>& solos = scene.solo_notes;
    const auto after = std::partition_point(
        solos.begin(), solos.end(),
        [now](const PreviewSoloNotes& s) { return struck_at(now, s.steps.front().ms); });
    if (after == solos.begin()) return {};
    return solo_box_of(*(after - 1), now);
}

std::vector<PreviewSoloBox> build_solo_box_samples(const PreviewScene& scene) {
    std::vector<PreviewSoloBox> out;
    for (const PreviewSoloNotes& s : scene.solo_notes) {
        const double first_ms = s.steps.front().ms;
        const double last_ms = s.steps.back().ms;
        out.push_back(solo_box_of(s, first_ms));
        if (s.steps.size() >= 2) out.push_back(solo_box_of(s, s.steps[s.steps.size() - 2].ms));
        out.push_back(solo_box_of(s, last_ms));
        out.push_back(solo_box_of(s, last_ms + kSoloResultMs));
    }
    return out;
}

double step_shown_ms(double now_ms, double length_ms, double delta_ms) {
    return shown_ms(shown_ms(now_ms, length_ms) + delta_ms, length_ms);
}

bool has_song_length(double length_ms) { return length_ms > 0.0; }

std::optional<double> song_fraction(double ms, double length_ms) {
    if (!has_song_length(length_ms)) return std::nullopt;
    return std::clamp(ms / length_ms, 0.0, 1.0);
}

std::vector<double> build_scrub_marks(const PreviewScene& scene, double length_ms) {
    // song_fraction alone decides whether the length is usable; with no
    // usable length it gives nothing, so there are no marks.
    std::vector<double> marks;
    marks.reserve(scene.activations.size());
    for (const PreviewActivation& a : scene.activations)
        if (const std::optional<double> at = song_fraction(a.ms, length_ms)) marks.push_back(*at);
    return marks;
}

double scrub_thumb_ms(double now_ms, double scrub_end_ms) {
    // With no usable edge there is nothing to park at: the thumb follows the
    // playhead, as it always did.
    return has_song_length(scrub_end_ms) ? std::min(now_ms, scrub_end_ms) : now_ms;
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

PreviewNextActBox build_next_act_box(const PreviewScene& scene, double now_ms, double length_ms,
                                     bool pro_drums) {
    PreviewNextActBox box;
    // The first activation not yet behind the playhead. The activations are
    // in time order, so a binary search finds it.
    const double now = shown_ms(now_ms, length_ms);
    const std::vector<PreviewActivation>& acts = scene.activations;
    const auto next = std::lower_bound(
        acts.begin(), acts.end(), now - kOnActivationMs,
        [](const PreviewActivation& a, double v) { return a.ms < v; });
    if (next == acts.end()) return box;
    const size_t i = static_cast<size_t>(next - acts.begin());
    box.shown = true;
    box.header = "Next: activation " + std::to_string(i + 1) + " of " + std::to_string(acts.size());
    box.notation = next->notation;
    box.detail = "at " + next->measure;
    if (next->chord.count() > 0) box.detail += kFactSeparator + next->chord.rowstr(pro_drums);
    return box;
}

std::string sp_meter_readout(const SpMeterCurve& curve, double now_ms) {
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
    return path_identity(*path);
}

}  // namespace hydra::app
