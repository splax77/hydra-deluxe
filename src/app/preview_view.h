// Preview view-model — the note highway as plain data, built here so the
// derivation is testable without a Direct3D frame or an audio device.
//
// The Preview tab renders a chart's notes — at the difficulty and drum mode
// the library's View row selects — as a scrolling 3D highway (see
// docs/adr/0005). This module turns a parsed Song (and, when the
// chart has been analyzed, the selected Path) into a PreviewScene: notes with
// their lane and drum attributes at resolved ms/measure positions, plus the
// spans the highway shades — SP phrases, solos, and activation fills — and the
// path overlay's activation moments. The renderer and the transport UI consume
// this; they compute no timing of their own.
//
// Everything here derives from one SongTiming (the song's own), so every ms in
// a scene shares the engine's timing truth and cannot drift between notes and
// overlay. Follows the path_view.h pattern: resolve the facts once, render dumb.

#ifndef HYDRA_APP_PREVIEW_VIEW_H
#define HYDRA_APP_PREVIEW_VIEW_H

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/stars.h"
#include "core/timing.h"
#include "app/path_view.h"
#include "parse/song.h"

namespace hydra::app {

// The five drum lanes, in KRYBG order, matching core NoteColor. A dedicated
// enum keeps the renderer off the parser's NoteColor (whose values start at 1).
enum class PreviewLane { Kick = 0, Red = 1, Yellow = 2, Blue = 3, Green = 4 };

PreviewLane lane_of(NoteColor color);

// The colour a lane is drawn from: lane_of read backwards, so a renderer
// holding only the lane can still ask the core's colour rules
// (allows_cymbals).
NoteColor color_of(PreviewLane lane);

// One drawn note. A chord at a tick expands to one PreviewNote per struck lane.
struct PreviewNote {
    int64_t tick = 0;
    double ms = 0.0;       // onset, from the song's ms index
    double measure = 0.0;  // decimal measure position (for barlines/labels)
    PreviewLane lane = PreviewLane::Kick;
    bool cymbal = false;       // pro-drums cymbal (Yellow/Blue/Green only)
    bool ghost = false;        // dynamics: quiet
    bool accent = false;       // dynamics: loud
    bool double_kick = false;  // the 2x-kick note
};

// A shaded time span on the highway: SP phrase, solo, or activation fill. Both
// tick and ms endpoints are carried so the renderer can place it by whichever
// axis it scrolls on.
struct PreviewSpan {
    int64_t start_tick = 0;
    int64_t end_tick = 0;
    double start_ms = 0.0;
    double end_ms = 0.0;
};

// What the game would have done with one candidate activation fill, read off
// the path's stored passed-over fills (see build_preview_scene):
//   Hidden  — never shown: not enough SP banked, or SP was already running.
//   Offered — shown and passed over: the path could have activated here.
//   Taken   — the fill the path activates on.
enum class PreviewFillState { Hidden, Offered, Taken };

// One candidate activation fill and what the path did with it. `chord_tick`
// is the chord the fill rides on: the tick an activation and a stored
// passed-over fill name it by. `span` is the window build_preview_base
// draws, which need not end on that chord.
struct PreviewFill {
    PreviewSpan span;
    int64_t chord_tick = 0;
    PreviewFillState state = PreviewFillState::Hidden;
};

// One Clone Hero drum lane (a roll or trill marker): a strip down each pad
// column it lights, over the lane's span. Which pads it lights is
// light_drum_lanes's rule in parse/song.cpp. A kick has no column, so it is never
// one.
struct PreviewDrumLane {
    PreviewSpan span;
    std::vector<PreviewLane> pads;
};

// One activation the selected path takes: where the player deploys banked SP.
struct PreviewActivation {
    int64_t tick = 0;
    double ms = 0.0;
    // The fills the path was shown and passed over before this activation,
    // copied from the record (Activation::skipped_fill_ticks).
    std::vector<int64_t> skipped_fill_ticks;
    // The fill this activation stands on: an index into PreviewScene::fills,
    // set by apply_preview_overlay when it marks that fill Taken (or finds an
    // earlier activation already took it). Empty when the activation's tick
    // ends no fill. The highway's fill-lane colour reads it.
    std::optional<size_t> taken_fill;
    // The deact node: where this activation's SP runs out, read off the
    // record (the search stamps it). has_sp_end is false only for a record
    // written before blob v4, which does not carry the node.
    bool has_sp_end = false;
    int64_t sp_end_tick = 0;
    double sp_end_ms = 0.0;
    // The stretch this activation's Star Power runs over, {ms, sp_end_ms}.
    // Empty without a stored end, or when the end is not after the start (a
    // window of no length is no window). The drain box, the score box and
    // the highway's tinted floor and SP end marks (through the track state's
    // sp_active intervals) all read it; the drain and score boxes ask
    // sp_running_at.
    std::optional<std::pair<double, double>> sp_window() const {
        if (!has_sp_end || !(sp_end_ms > ms)) return std::nullopt;
        return std::make_pair(ms, sp_end_ms);
    }
    // Is this activation's Star Power running with the playhead at `now`
    // (ms)? The one home of that rule (M48), defined after struck_at below.
    // The drain and score boxes ask it through running_activation; the
    // highway's sp_active sweep answers it separately, and test_track_state
    // pins the two together.
    bool sp_running_at(double now) const;
    // Where this window's SP end changed, read off the record: the tick the
    // change takes effect (Activation::refill_tick) and the end after it. The
    // first entry is the activation itself. The gauge draws the window from
    // these alone; a squeezed-out phrase has no entry.
    struct SpEndChange {
        int64_t at_tick = 0;
        int64_t end_tick = 0;
        bool operator==(const SpEndChange& o) const {
            return at_tick == o.at_tick && end_tick == o.end_tick;
        }
        bool operator!=(const SpEndChange& o) const { return !(*this == o); }
    };
    std::vector<SpEndChange> sp_end_changes;
    // Where each bar this activation spends arrived (Activation::bank_rise_ticks).
    std::vector<int64_t> bank_rise_ticks;
    // The activation note's lane (the chord's highest-priority note, Green
    // first): the lane the activated fill lights. Kick when the record has no
    // chord.
    PreviewLane lane = PreviewLane::Kick;
    bool has_lane = false;
    // The activation's position as the Paths tab prints it (format_measure)
    // and its chord, which build_next_act_box names in the Pro Drums
    // setting's words; chord is empty when the record has none.
    std::string measure;
    Chord chord;
    // The activation in path notation with its timings, copied whole from
    // Activation::notationstr_verbose (its one owner) for the next-activation
    // box.
    std::string notation;
};

// A line on the highway: a measure line, a beat line, or the fainter line
// (Onyx line-1/2/3). Where the first two fall is MeasureIndex::walk's, and
// where the third falls is build_beat_events's; it maps the walk's kinds onto
// these.
enum class PreviewBeatKind { MeasureLine, Beat, Half };

struct PreviewBeat {
    int64_t tick = 0;
    double ms = 0.0;
    PreviewBeatKind kind = PreviewBeatKind::Beat;
};

// A tempo change, for the time box's BPM readout.
struct PreviewTempo {
    int64_t tick = 0;
    double ms = 0.0;
    double bpm = 0.0;
};

// A practice section, for the time box's section readout.
struct PreviewSection {
    int64_t tick = 0;
    double ms = 0.0;
    std::string name;
};

// One span of the continuous measure count (MeasureCount), flattened for the
// parts of the Preview that drain by measures: it starts at a time
// signature's own tick and runs at `tpm` ticks per measure. The drawn lines
// read the grid instead (MeasureIndex, through build_beat_events).
struct PreviewMeasureSpan {
    int64_t tick = 0;
    int64_t tpm = 0;
};

// The time signature the time box shows. A scene with a song reads it from
// SongTiming::time_signature_at; these defaults are what a scene built from
// nothing shows.
struct PreviewTimeSig {
    int numerator = kDefaultTimeSigNumerator;  // Song's default (core/song.h)
    int denominator = kDefaultTimeSigDenominator;
};

// One straight stretch of the Star Power meter: the banked bars run linearly
// in ms from start_bars at start_ms to end_bars at end_ms.
//
// SP drains at one bar per two measures — linear in MEASURES, not in ms. Inside
// a single tempo section crossed with a single meter section, measures are
// linear in ms, so cutting the curve at every tempo change, meter change,
// stored SP end step, activation and deact node makes it exactly piecewise
// linear in ms. Nothing here is an approximation.
struct SpMeterSegment {
    double start_ms = 0.0;
    double end_ms = 0.0;
    double start_bars = 0.0;
    double end_bars = 0.0;
};

// The meter over the whole chart: contiguous segments in time order, each
// one's end_ms the next one's start_ms. The meter's jumps — a phrase
// collecting, an activation spending the bank — are discontinuities BETWEEN
// two adjacent segments, not slopes inside one.
struct SpMeterCurve {
    std::vector<SpMeterSegment> segments;
    int cap = kCloneHeroSpCap;  // the ceiling in bars
};

// One step of the running score: what the game's counter reads from the
// moment this chord is hit until the next one. Copied from core/replay.h,
// which is proven equal to the engine's own totals for every corpus path.
struct PreviewScoreStep {
    double ms = 0.0;      // the chord's onset
    int64_t total = 0;    // on-screen total: a solo's bonus lands on its last chord
    int multiplier = 1;   // the game's disc once the chord is hit (ReplayChord::multiplier_shown)
    int multiplier_plain = 1;  // the combo multiplier alone, never doubled (ReplayChord::multiplier_after)
    int combo = 0;        // notes hit so far, this chord included
    // The running score the star counter reads: the score stars count, as
    // core/stars.h says, taken off the replay's running sums (ReplayChord::cum).
    int64_t score_without_solo = 0;
};

// The running score for the path the overlay shows.
//   None        — no path: the chart is not analyzed, so there is no score.
//   Unavailable — the path cannot be replayed faithfully (an activation
//                 without a stored deactivation node or squeeze-out tick),
//                 or the replay's total is not the stored one. The box says
//                 so rather than show a number nobody can vouch for.
//   Ready       — `steps` holds one entry per chord, in time order.
struct PreviewScore {
    enum class State { None, Unavailable, Ready };
    State state = State::None;
    std::vector<PreviewScoreStep> steps;
    // The path's star cutoffs (core/stars.h's star_cutoffs), which the score
    // box's star counter reads. Filled whenever there is a path.
    StarCutoffs cutoffs;
};

// One solo section's notes, as the solo box counts them. Path-free: a Hydra
// path hits every note, so the notes hit at a moment are those of the chords
// struck by then (struck_at).
struct PreviewSoloNotes {
    // One of the solo's chords: its onset, and the solo's notes hit once it
    // is struck, this chord's included. A chord's notes are
    // solo_note_count's (core/scoring.h).
    struct Step {
        double ms = 0.0;
        int notes_hit = 0;
    };
    std::vector<Step> steps;  // in time order, never empty
    int notes = 0;            // the solo's note total: the last step's notes_hit
    int64_t bonus = 0;        // solo_bonus summed over the solo's chords
};

// The whole chart as the Preview draws it. Notes are in tick order. Spans are
// in start order and do not overlap within their own list.
struct PreviewScene {
    std::vector<PreviewNote> notes;
    std::vector<PreviewSpan> sp_phrases;    // from the song's SP-phrase flags
    std::vector<PreviewSpan> solos;         // from the song's solo sections
    // The same solo sections' notes, for the solo box: one entry per entry
    // of `solos`, in the same order.
    std::vector<PreviewSoloNotes> solo_notes;
    std::vector<PreviewDrumLane> drum_lanes;  // from the song's drum lanes
    std::vector<PreviewFill> fills;         // candidate activation-fill windows
    std::vector<PreviewActivation> activations;  // overlay: the path's activations
    // Overlay: the fills a second, "ghost" path activates on, as indices into
    // `fills` in chart order. apply_preview_overlay names them by the rule
    // that names the drawn path's Taken fill. Empty with no ghost path.
    std::vector<size_t> ghost_fills;
    // Overlay: where each bar banked after the path's last window arrived
    // (Path::trailing_bank_ticks). Empty without a path.
    std::vector<int64_t> trailing_bank_ticks;
    std::vector<PreviewBeat> beats;    // measure/beat/half lines, tick order
    std::vector<PreviewTempo> tempos;  // tempo changes, tick order
    std::vector<PreviewSection> sections;  // practice sections, tick order
    std::vector<PreviewMeasureSpan> measure_spans;  // one per time signature, tick order
    // Banked SP over time, for the meter gauge. With a path every value is
    // the record's: the bank steps at each stored bar arrival, and each
    // window drains from the stored SP end changes to the deact node. Without
    // a path there is no record, so it fills one bar per phrase and pins at
    // the cap: that chart-only view has nothing to spend the bank. Empty when
    // the chart has no SP phrase and the path stamps no bar.
    SpMeterCurve sp_meter;
    // The running score the score box reads. None when built without a path.
    PreviewScore score;
    // The song's own timing. The time box asks it for ms->tick, for
    // measure:beat:tick and for the tempo and time signature in force rather
    // than re-deriving any of them from the flattened lists above. Empty only
    // on a default-built PreviewScene (no
    // song to read); build_preview_scene always fills it.
    std::optional<SongTiming> timing;
    // The song's length, as app::song_length_ms answers it (D255), handed to
    // build_preview_base. 0 when the song has none. The SP curve closes here.
    // The Preview's playback, time box, scrubber and gold marks end at the
    // same length (PreviewTransport::length_ms).
    double song_length_ms = 0.0;
    bool has_notes = false;

    // Does the scene have an SP gauge? Yes whenever it was built from a song
    // (it has timing), Star Power or not: a chart with no SP phrase shows an
    // empty gauge. The gauge and the drain box both ask this.
    bool has_sp_gauge() const { return timing.has_value(); }
};

// The last note the highway draws, or null when the scene has none. The one
// answer to "when is the scene's last note": a song with no length ends its
// beat grid past it (build_preview_base). It
// is not the song's length (PreviewScene::song_length_ms, app::song_length_ms).
const PreviewNote* last_drawn_note(const PreviewScene& scene);

// last_drawn_note's onset in ms, or 0 when the scene has no notes.
double last_note_ms(const PreviewScene& scene);

// The highway's lines from the song's timing alone: every line of the
// measure grid (MeasureIndex::walk) from tick 0 to last_tick, each with a Half
// line halfway back to the line before it (none before tick 0). A weak beat
// is drawn as a Beat (D115, the user's answer 8). ms from the timing's own ms
// index.
std::vector<PreviewBeat> build_beat_events(const SongTiming& timing, int64_t last_tick);

// The Preview's time readouts at `now_ms`. `timestamp` is the clock beside the
// scrubber, playhead and where playback ends as "m:ss.mmm / m:ss.mmm". The time box
// over the highway shows the two same points through format_measure, the BPM
// and time signature in force, and the practice section in force.
struct PreviewTimeBox {
    std::string timestamp;     // "0:35.000 / 2:06.253"
    std::string position;      // format_measure at the playhead, "m27.2.450"
    std::string length;        // format_measure where playback ends, "m96.3.240"
    std::string tempo;         // "BPM 191.001 · 4/4"
    std::string section_line;  // "Section chorus_1"; empty when no section is in force
};

PreviewTimeBox build_time_box(const PreviewScene& scene, double now_ms,
                              double length_ms);

// The score box the Preview draws under the time box, at the moment the time
// box shows for `now_ms` and `length_ms` (shown_ms in preview_view.cpp). `score`
// is the total with thousands separators ("12,345"), and `detail` is
// "x<multiplier> · combo <n>", both as the last chord hit left them. The
// multiplier is the replay's. While Star Power runs at the playhead (the
// same test the drain box uses) it is the disc as that chord left it:
// doubled when SP paid the chord. Once SP has ended it is the plain combo
// multiplier, as the game's disc drops at the SP end rather than at the next
// chord. Hidden (`shown` false) when the scene has no path; "Score
// unavailable" with an empty `detail` when the replay could not be trusted.
//
// The star counter reads the same last chord. `stars` is stars_for_score
// (core/stars.h, the rule's home) on that chord's score_without_solo, so a
// solo's bonus raises `score` but never the stars. `next_star` is
// "star <n> in <points>": the next star's number and the points still needed
// for it, with thousands separators. It is empty at kMaxStars. Neither is
// set unless `available`.
struct PreviewScoreBox {
    bool shown = false;
    bool available = false;
    std::string score;
    std::string detail;
    int stars = 0;
    std::string next_star;
};

PreviewScoreBox build_score_box(const PreviewScene& scene, double now_ms, double length_ms);

// Is a note at `note_ms` struck with the playhead at `now_ms`? Yes when it is
// at or before the playhead, so a note exactly on the playhead counts as hit
// and a jump to an activation lands on a struck chord (D48, Q27). The score
// box, the highway's gem flash and PreviewActivation::sp_running_at all ask
// this.
inline bool struck_at(double now_ms, double note_ms) { return note_ms <= now_ms; }

// PreviewActivation::sp_running_at, declared in the struct above: the
// activation's sp_window, entered when its chord is struck (struck_at) and
// left at the window's end.
inline bool PreviewActivation::sp_running_at(double now) const {
    const std::optional<std::pair<double, double>> w = sp_window();
    return w && struck_at(now, w->first) && now < w->second;
}

// The Star Power drain box the Preview draws beside the SP gauge, at the
// moment the time box shows for `now_ms` and `length_ms` (shown_ms). `rate` is how long one bar of SP lasts at the playhead ("1 bar / 4.0 s"):
// kMeasuresPerSpBar measures at the local measure length the song's timing
// gives (SongTiming::ms_per_measure_at). The box is `active` when the path has
// SP running at the playhead (PreviewActivation::sp_running_at).
// Then `detail` is "empties in X s", that stored end minus the playhead.
// Otherwise `detail` is "full meter X s": the meter's cap in bars at that same
// bar time, so it jumps exactly when the rate does. A chart with no Star Power
// never has SP running, so its box always reads that idle form.
// Hidden (`shown` false) when the scene has no SP gauge (has_sp_gauge).
// Nothing here re-derives Star Power: it reads the record and the song's own
// timing.
struct PreviewDrainBox {
    bool shown = false;
    bool active = false;
    std::string header;  // "SP drain" or "SP drain (if activated)"
    std::string rate;    // "1 bar / 4.0 s"
    std::string detail;  // "empties in 7.3 s" or "full meter 16.0 s"
};

PreviewDrainBox build_drain_box(const PreviewScene& scene, double now_ms, double length_ms);

// How long Clone Hero's solo box shows each result after a solo's last note:
// first the end message, then the bonus, then the box hides. Both read the
// game's own code.
inline constexpr double kSoloResultMs = 1500.0;
inline constexpr double kSoloTailMs = 2.0 * kSoloResultMs;

// The percent of a solo's notes the game's box shows with `hit` of `total`
// notes hit: the game's own sum, done in 32-bit float and truncated, so it
// can read one below the exact percent (53 of 100 shows 52). 0 when `total`
// is 0.
int solo_percent(int hit, int total);

// The solo box the Preview draws right of the highway, at the moment the
// time box shows for `now_ms` and `length_ms` (shown_ms). It follows the
// solo whose first chord was struck last (struck_at), so a solo that starts
// while the one before still shows its result takes over. From its first
// chord until its last, `headline` is solo_percent of the notes hit so far
// ("85%") and `detail` is "102 / 120 notes". Once the last chord is struck
// the solo is over: `headline` is "Perfect Solo!" for kSoloResultMs, since a
// Hydra path hits every note, then the bonus ("+12,000") for kSoloResultMs,
// with `detail` empty. Then the box hides until the next solo. Every moment
// is timed off the chords' onsets, so scrubbing shows what playback shows.
// Works with or without a path.
struct PreviewSoloBox {
    bool shown = false;
    std::string headline;
    std::string detail;
};

PreviewSoloBox build_solo_box(const PreviewScene& scene, double now_ms, double length_ms);

// The boxes the scene's solos show at their widest: for each solo, the box
// on its first chord, on its last chord before the end (the most digits),
// its end message and its bonus. The panel reserves the largest of them so
// the overlay's scale holds still through a solo.
std::vector<PreviewSoloBox> build_solo_box_samples(const PreviewScene& scene);

// The playhead `delta_ms` from `now_ms`, for the Preview's step buttons and
// keys. It starts from the moment build_time_box shows for the same `now_ms`
// and `length_ms` (one shared helper computes that moment), so a playhead past
// the end steps from the end. The result is held inside [0, length_ms], so a
// step never lands before 0 or past where playback ends.
double step_shown_ms(double now_ms, double length_ms, double delta_ms);

// Where each activation of the shown path sits on the scrubber: its onset over
// `length_ms` (the song's length, PreviewTransport::length_ms), clamped to
// 0..1, in activation order. Empty with no path or no length.
std::vector<double> build_scrub_marks(const PreviewScene& scene, double length_ms);

// Where the scrubber's thumb sits with the playhead at `now_ms`. It follows
// the playhead and stays parked at `scrub_end_ms` (the scrubber's right edge)
// should the clock run past it between frames.
double scrub_thumb_ms(double now_ms, double scrub_end_ms);

// Is `length_ms` a song length the timeline can use? Only a positive length
// is. song_fraction and the Paths tab's end-measure label both ask it, so the
// marks and the label appear and vanish together. The length each caller
// passes is app::song_length_ms's answer (D75).
bool has_song_length(double length_ms);

// How far into the song `ms` is: its share of a song length, clamped to 0..1.
// No value when has_song_length says the length is unusable. The length each
// caller passes is app::song_length_ms's answer (D75).
std::optional<double> song_fraction(double ms, double length_ms);

// Where "< Act" (direction -1) or "Act >" (+1) moves the playhead from
// `now_ms`: the onset of the nearest activation strictly before or after it.
// Half a millisecond of slack each way, so a playhead parked on an activation
// moves past it. nullopt when there is none that way.
std::optional<double> activation_jump_ms(const PreviewScene& scene, double now_ms,
                                         int direction);

// The box under the SP drain box, top right: the first activation at or after the
// playhead (the same half-millisecond slack), read at the moment the time box
// shows for `now_ms` and `length_ms` (shown_ms), "Next: activation 1 of 3",
// then the activation's notation, then "at m32.1.0 · [Kick - Green cymbal]".
// The chord's notes are named by Chord::rowstr in the words of `pro_drums`,
// the Pro Drums setting: "Red snare" with it on, plain "Red" with it off (D48,
// Q11). Hidden past the last activation and when the scene has no path.
struct PreviewNextActBox {
    bool shown = false;
    std::string header;
    // The activation's PreviewActivation::notation, whose owner is
    // Activation::notationstr_verbose. The Preview draws it as one piece.
    std::string notation;
    std::string detail;
};
PreviewNextActBox build_next_act_box(const PreviewScene& scene, double now_ms, double length_ms,
                                     bool pro_drums);

// The number under the SP gauge: bars banked at `now_ms` over the cap, one
// decimal ("2.5/4"). A curve with no segments reads 0.0 bars ("0.0/4").
std::string sp_meter_readout(const SpMeterCurve& curve, double now_ms);

// One entry of the Preview's "Showing" list: the path's notation, with
// "  (optimal)" on an Optimal path and "  (best all-0)" on the all-0 path.
std::string preview_path_label(const PathButtonView& button);

// Bars banked at `ms`: 0 before the curve begins, its final value after the
// curve ends, and interpolated inside a segment. On a boundary shared by two
// segments the LATER one wins — that is how a collection step or an
// activation's snap reads as an instant jump rather than a ramp.
double sp_meter_bars_at(const SpMeterCurve& curve, double ms);

// Build the scene from a parsed song. `path` may be null (the chart is not
// analyzed yet): then `activations` is empty, every candidate fill reads
// Offered, and everything else is present, so the Preview works for any
// selected chart. When `path` is given, its activations (those carrying a
// timecode) become the overlay, their ms resolved against the song's own
// timing so they line up with the notes exactly, and each candidate fill is
// classified Hidden / Offered / Taken from the activations' stored
// passed-over fills.
//
// `sp_cap` is the SP meter's ceiling in bars — the viewed record's own sp_cap,
// which is 4 for any normal Clone Hero run and differs only on a what-if
// record analyzed at another cap. It scales the meter curve and nothing else;
// no note, span or fill in the scene depends on it.
//
// `rules` prices the running score (the replay reads the backend leeway and
// the squeeze-out rule from it); nothing else in the scene depends on it.
//
// `song_length_ms` is the song's length (app::song_length_ms, D255): it
// becomes song_length_ms, where the SP meter curve closes and the beat lines
// stop (D266). The audio's end is never an input. See build_preview_base for
// the beat lines without a length.
//
// `ghost_path` (may be null) is a second path drawn faintly beside the first:
// it fills ghost_fills and nothing else. Without one the scene is exactly
// what it was before the ghost existed.
PreviewScene build_preview_scene(const Song& song, const Path* path,
                                 int sp_cap = kCloneHeroSpCap,
                                 const core::Rules& rules = core::default_rules(),
                                 std::optional<double> song_length_ms = std::nullopt,
                                 const Path* ghost_path = nullptr);

// The same scene in two halves, so a path change rebuilds only what the path
// changes. build_preview_scene(song, path, cap, rules) is exactly
// apply_preview_overlay(build_preview_base(song), song, path, cap, rules).
//
// The base reads the song alone. It owns these fields: notes, sp_phrases,
// solos, solo_notes, drum_lanes, the fill windows (each fills[i].span), beats, tempos,
// sections, measure_spans, timing, song_length_ms and has_notes.
// Every fill in a base reads Hidden, the field's default.
//
// The overlay owns the rest: activations, every fill's state, ghost_fills,
// score and sp_meter. Those read the path, the ghost path, the SP cap or the
// rules. The SP meter is
// overlay even without a path: its ceiling is the record's cap and its drain
// is the path's activations.
//
// apply_preview_overlay clears whatever overlay `base` already carries before
// it builds the new one, so a scene built for another path works as a base too.
// An empty song gives the empty scene build_preview_scene gives.
//
// The beat lines stop at the song's length, where playback stops (D266): the
// last one sits at or before `song_length_ms`. With no length has_song_length
// accepts, they run two measures past the last note.
PreviewScene build_preview_base(const Song& song,
                                std::optional<double> song_length_ms = std::nullopt);
PreviewScene apply_preview_overlay(PreviewScene base, const Song& song, const Path* path,
                                   int sp_cap = kCloneHeroSpCap,
                                   const core::Rules& rules = core::default_rules(),
                                   const Path* ghost_path = nullptr);

// Identity of the path an overlay was built from: path_identity, the same
// rule the Paths tab's all-0 dedupe reads. Callers that must notice a changed
// selection compare these keys. A null path (no overlay) gives an empty key.
std::string path_overlay_key(const Path* path);

}  // namespace hydra::app

#endif  // HYDRA_APP_PREVIEW_VIEW_H
