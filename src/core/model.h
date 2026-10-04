// Domain model: charts, chords, squeezes, activations and paths.
//
// The note/chord/path/record types the parser fills and the search produces.
// The *string* forms here (Path::pathstring/pathstring_verbose,
// Activation::notationstr) are user-visible and pinned by the tests, so they must
// match Python byte-for-byte: comma-grouped scores, int() truncation on ms, the
// exact +/- squeeze symbols and [KRYBG] slot layout.
//
// Records are stored in the binary format of store/serialize.h. Stored paths
// carry each chord as its Chord::code, read back by Chord::from_code.

#ifndef HYDRA_CORE_MODEL_H
#define HYDRA_CORE_MODEL_H

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "core/backend_value.h"
#include "core/rules.h"
#include "core/timing.h"

namespace hydra {

// Clone Hero's Star Power meter holds this many bars and no more. It is the
// default SP cap; records analyzed at a different cap answer what-if questions
// whose scores are not achievable in game.
inline constexpr int kCloneHeroSpCap = 4;

// A chart file that does not work. Chord and
// the parsers raise it; the parsers swallow it per-op exactly as Python does.
class ChartFileError : public std::runtime_error {
public:
    explicit ChartFileError(const std::string& what)
        : std::runtime_error(what) {}
};

// ---- enums --------------------------------------------------------------

enum class NoteColor { Kick = 1, Red = 2, Yellow = 3, Blue = 4, Green = 5 };
enum class NoteDynamicType { Normal = 1, Ghost = 2, Accent = 3 };
enum class NoteCymbalType { Normal = 1, Cymbal = 2 };

bool allows_cymbals(NoteColor c);
std::string color_str(NoteColor c);         // "Kick"/"Red"/...
std::string dynamic_str(NoteDynamicType t); // "none"/"ghost"/"accent"
std::string color_notationstr(NoteColor c); // "K"/"R"/"Y"/"B"/"G"

// ---- squeeze thresholds -------------------------------------------------
// One home for the ms thresholds that define squeeze semantics. Each used to
// be a repeated literal; the value is the interface, so a change here is a
// deliberate rule change, not a stray edit.

// A squeeze (or early fill) tighter than this many ms counts as
// difficult: it turns on warning colors, and it is the "Normal" floor of the
// report's timing tiers.
constexpr double kDifficultMs = 2.0;

// The backend leeway edge is a user rule now: core::Rules::backend_leeway_ms
// (hydra_rules.ini), read by the engine and BackendSqueeze::summarystr.

// The default per-side hit window (the registrable Clone Hero Pro Drums
// window). The *setting* app::Settings::hit_window_ms starts from this; the
// display-layer defaults below use it so all entry points agree.
constexpr double kDefaultHitWindowMs = 85.0;

// The squeeze horizon in ms. The search graph only looks this far from the SP
// end for a reachable squeeze, and backends within it of the deactivation are
// stored and shown, so nothing the engine collects is trimmed. The Backend
// limit setting narrows the display from here. hydra_batch prints it.
constexpr double kSqueezeWindowMs = 500.0;

// How far a note sits from a Star Power end, in ms: negative before it,
// positive after. Every backend row's offset_ms is this number.
inline double offset_from_sp_end(double note_ms, double sp_end_ms) {
    return note_ms - sp_end_ms;
}

// Is a note close enough to a Star Power end to matter for a squeeze?
// Strictly inside kSqueezeWindowMs on either side; exactly 500 ms away is
// out. The one window check: the search graph, the engine's tail rows, the
// stored rows (Activation::display_backends) and hydra_replay all ask it.
inline bool within_squeeze_window(double offset_from_sp_end_ms) {
    return std::fabs(offset_from_sp_end_ms) < kSqueezeWindowMs;
}

// Early-fill (E) timing window, applied to e_offset in both directions:
// an activation with e_offset < -window is illegal (the fill can't be
// summoned), and one with e_offset < +window is E-critical. 60 since 2026-09
// (was 85 from 1.5.0, +/-70 before that): across 18,773 analyzed charts the
// hardest E0 on any best path was 57.7 ms, so nothing past 60 earns its
// place. Search-load-bearing, so it is a constant, never the hit_window_ms
// setting.
constexpr double kEarlyFillWindowMs = 60.0;

// ---- note value ----------------------------------------------------------
// What one note is worth before any multiplier. ChordNote::basescore and
// category_scores both read these, so the price has one home.
inline constexpr int kNoteBasePoints = 50;
inline constexpr int kCymbalBonusPoints = 15;
// Solo bonus: this many points per note hit inside a solo section.
inline constexpr int kSoloBonusPerNote = 100;

// ---- ChordNote ----------------------------------------------------------

struct ChordNote {
    NoteColor colortype;
    NoteDynamicType dynamictype = NoteDynamicType::Normal;
    NoteCymbalType cymbaltype = NoteCymbalType::Normal;
    bool is2x = false;

    bool operator==(const ChordNote& o) const;
    bool operator!=(const ChordNote& o) const { return !(*this == o); }
    std::string str() const;
    int basescore() const;
    bool is_dynamic() const { return dynamictype != NoteDynamicType::Normal; }
    bool is_accent() const { return dynamictype == NoteDynamicType::Accent; }
    bool is_ghost() const { return dynamictype == NoteDynamicType::Ghost; }
    bool is_cymbal() const { return cymbaltype == NoteCymbalType::Cymbal; }
};

// ---- Chord --------------------------------------------------------------

class Chord {
public:
    Chord() = default;

    // The chord spelled one character per lane, in KRYBG order: "." for an
    // empty lane, otherwise n/g/a for a normal/ghost/accent note, upper case
    // for a cymbal or a 2x kick. Red + yellow cymbal + green cymbal is
    // ".nN.N". Every chord a chart can express has one; nothing is looked up.
    std::string code() const;
    // The reverse of code(). Throws std::out_of_range on a malformed code.
    static Chord from_code(const std::string& code);

    bool operator==(const Chord& o) const;
    bool operator!=(const Chord& o) const { return !(*this == o); }

    // notemap access (KRYBG order), mirroring __getitem__/__setitem__.
    std::optional<ChordNote>& at(NoteColor c);
    const std::optional<ChordNote>& at(NoteColor c) const;

    // Non-empty notes. basesorted uses a *stable* sort on basescore (Python's
    // list.sort is stable), which the scoring tie-break depends on.
    std::vector<ChordNote> notes(bool basesorted = false) const;

    int count() const;
    int hands_count() const;

    std::string rowstr() const;
    std::string notationstr() const;

    void apply_disco_flip();
    void apply_flam_conversion();

    // Adds a fresh normal note of this color; raises ChartFileError if the
    // color is already present. Returns a reference so the caller can set its
    // dynamics/cymbal/2x, as MidiParser.op_note does.
    ChordNote& add_note(NoteColor color);
    void insert_note(const ChordNote& note);
    void add_2x();
    // Each raises ChartFileError when the colour has no note (a stray marker).
    void apply_cymbal(NoteColor color);
    void apply_ghost(NoteColor color);
    void apply_accent(NoteColor color);

    // Highest-priority note (Green→Blue→Yellow→Red→Kick); throws if empty.
    const ChordNote& activation_note() const;

private:
    // Index = color value - 1 (Kick..Green), preserving KRYBG iteration order.
    std::array<std::optional<ChordNote>, 5> notemap_{};
};

// ---- squeezes -----------------------------------------------------------

enum class SqueezeKind { SqIn, SqOut };

// How hard a squeeze is, in ms: a SqIn's offset, or a SqOut's offset negated.
// "-x + 0.0" turns -0.0 into +0.0 so a dead-on SqOut prints "0.0". The engine's
// act_difficulty and SPSqueeze::difficulty both call this.
inline double squeeze_difficulty(bool is_sqin, double offset_ms) {
    return is_sqin ? offset_ms : (-offset_ms + 0.0);
}

// ---- the early-fill window ----------------------------------------------
// Clone Hero's early-fill rule and the E0, stated once around
// kEarlyFillWindowMs. A fill spawns only when SP was ready by the fill's
// deadline, give or take the window. The e_offset is how long before the
// deadline SP became ready (negative: after it). The search's branch_activate
// and its group key (ready_class) ask these; so do Activation's E tests.
inline double fill_e_offset(double deadline_ms, double ready_ms) { return deadline_ms - ready_ms; }
// The fill refuses to spawn: SP became ready more than the window too late.
inline bool fill_refuses(double e_offset) { return e_offset < -kEarlyFillWindowMs; }

// E0: the early fill lands inside its window and nothing was skipped.
// is_e0(e_offset, 0) alone is "E-critical": the fill is inside the window.
inline bool is_e0(double e_offset, int skips) {
    return e_offset < kEarlyFillWindowMs && skips == 0;
}

// How hard an E0 activation's early fill is, in ms.
inline double early_fill_difficulty(double e_offset) { return -e_offset + 0.0; }

// How frontend (activation-hit) timing error transfers to the SP end. SP
// length is measure-based, so hitting the frontend d ms off moves the SP end
// by r*d ms, where r = ms-per-measure at the SP end / ms-per-measure at the
// frontend. The two directions differ when the activation or SP end sits
// exactly on a meter/tempo change: an early (-) hit moves into the section
// before the tick, a late (+) hit into the section at/after it.
struct TransferScale {
    double early = 1.0;  // r- : early (-) hits — difficult SqOuts, free SqIns
    double late = 1.0;   // r+ : late (+) hits — difficult SqIns, free SqOuts,
                         //      backend squeezes
    // Plain == on both doubles, no tolerance: a stored scale equals only the
    // same values (not bit-exact: -0.0 equals 0.0 and NaN equals nothing;
    // real scales are positive and finite, so that never arises).
    bool operator==(const TransferScale& o) const { return early == o.early && late == o.late; }
    bool operator!=(const TransferScale& o) const { return !(*this == o); }
};

// A SqIn (+) or SqOut (-): which way the note is squeezed across the SP end,
// and by how many ms.
struct SPSqueeze {
    SqueezeKind kind;
    double offset_ms = 0.0;

    double offset() const { return offset_ms; }
    double timing() const { return -offset_ms + 0.0; }
    double difficulty() const { return squeeze_difficulty(kind == SqueezeKind::SqIn, offset_ms); }
    const char* symbol() const {
        return kind == SqueezeKind::SqIn ? "+" : "-";
    }
    bool is_difficult() const { return difficulty() > kDifficultMs; }
    // Whether the squeeze is already done with no frontend timing: the one
    // answer the rating and the sentence both read. The SP walk pays a note
    // on the SP end (core::paid_by_sp_walk), so a SqIn there is already in
    // and free, and a SqOut there still has to be hit late (D13).
    bool is_free() const {
        const bool inside_sp = core::paid_by_sp_walk(offset_ms);
        return kind == SqueezeKind::SqIn ? inside_sp : !inside_sp;
    }
    const char* type_name() const {
        return kind == SqueezeKind::SqIn ? "SqIn" : "SqOut";
    }
    std::string description() const;

    // A SqIn's frontend transfer scale: at squeeze_end_tick, measured from
    // squeeze_anchor_tick. Stamped by the search at copy-out through
    // frontend_transfer_scales and stored, so the details view never needs a
    // SongTiming. Unset means the search could not compute it, a bug guard
    // like transfer_post's (D4). A SqOut's is always unset: that is not an
    // unknown, because no reader asks a SqOut for one. Its row is rated at
    // transfer_post.
    std::optional<TransferScale> transfer;
};

struct BackendSqueeze {
    Timecode timecode;
    Chord chord;
    int points = 0;
    int sqout_points = 0;
    std::optional<double> offset_ms;

    bool operator==(const BackendSqueeze& o) const;
    // Rating label. The outer +/-W edges come from the hit window; the inner
    // -10/3/10 edges are absolute (they encode leeway/near-deact semantics,
    // not the window).
    // squeezed_out: this row is the activation's squeezed-out chord
    // (Activation::is_sqout_backend). Only that row reads the SqOut ladder;
    // every other row, phrase chord or not, reads the plain one.
    // leeway_ms: the backend leeway edge (Rules::backend_leeway_ms); a
    // plain row under it rates "Standard".
    std::string summarystr(bool squeezed_out,
                           double hit_window_ms = kDefaultHitWindowMs,
                           double leeway_ms = core::default_rules().backend_leeway_ms) const;
};

// Multiplier squeeze. Construction validates the chord+combo and throws
// std::invalid_argument when it is not a squeezable situation (mirroring the
// ValueError that ScoreGraph.store_multsqueeze catches).
class MultSqueeze {
public:
    MultSqueeze(Chord chord, int combo);
    // Whether a chord hit at this combo is a multiplier squeeze: exactly the
    // cases the constructor accepts, answered without throwing. The graph
    // asks this of every chord, and "no" is the usual answer.
    static bool applies(const Chord& chord, int combo);

    int multiplier() const;     // to_multiplier(combo) + 1
    std::string direction() const;
    int points() const;
    std::string notationstr() const;
    // "Hit X or Y last/first." guidance text, mirroring MultSqueeze.howto.
    std::string howto() const;

    const Chord& chord() const { return chord_; }
    int combo() const { return combo_; }

    bool operator==(const MultSqueeze& o) const {
        return chord_ == o.chord_ && combo_ == o.combo_;
    }

private:
    void validate() const;
    Chord chord_;
    int combo_;
};

// ---- Activation ---------------------------------------------------------

// Why an activation's SP end moved (Activation::sp_end_steps).
//   Activation - the activation itself: the end its banked bars give.
//   Collected  - a phrase collected while active: two measures more.
//   Clamped    - a phrase collected with the meter full: the end pinned to
//                the cap's length past that phrase (ADR 0013).
//   SqIn       - the squeeze-in phrase, early or late.
enum class SpEndKind : uint8_t { Activation = 0, Collected = 1, Clamped = 2, SqIn = 3 };
// The last valid kind: the codec refuses any stored value past it.
constexpr SpEndKind kLastSpEndKind = SpEndKind::SqIn;

// Is this step a squeeze-in? The one statement of that rule: the engine's
// running window (Engine::squeezed_in), the stored list
// (core::sqin_phrase_ticks) and nth_sqin_step all ask it.
inline bool is_sqin_kind(SpEndKind kind) { return kind == SpEndKind::SqIn; }

// Did this step squeeze in the phrase on `tick`?
inline bool is_sqin_step_on(int64_t step_tick, SpEndKind kind, int64_t tick) {
    return step_tick == tick && is_sqin_kind(kind);
}

// One place an activation's SP end moved: `tick` is the note that moved it,
// `end_tick` the SP end in force after it.
struct SpEndStep {
    int64_t tick = 0;
    int64_t end_tick = 0;
    SpEndKind kind = SpEndKind::Activation;
    bool operator==(const SpEndStep& o) const {
        return tick == o.tick && end_tick == o.end_tick && kind == o.kind;
    }
    bool operator!=(const SpEndStep& o) const { return !(*this == o); }
};

// The n-th SqIn step (counting from 0) in a run of SP-end steps: the n-th
// SqIn squeeze owns it, because both lists are kept in time order. `last`
// when the run holds fewer. Activation::squeeze_end_step and the engine's
// folded-variant copy-out (Engine::close_folded_act) both pair them here.
template <class It>
It nth_sqin_step(It first, It last, size_t n) {
    for (; first != last; ++first)
        if (is_sqin_kind(first->kind) && n-- == 0) return first;
    return last;
}

// Which SqIn a squeeze is, counting from 0 in list order: how many SqIn
// squeezes come before position `at`. That rank picks its step
// (nth_sqin_step) and its transfer scale. `is_sqin` says which entries are
// SqIns, so the engine's own squeeze records can be counted too.
// Activation::squeeze_end_step, the engine's folded-variant copy-out and its
// transfer-scale stamp all count here.
template <class It, class IsSqIn>
size_t sqin_rank(It first, It at, IsSqIn is_sqin) {
    size_t n = 0;
    for (; first != at; ++first)
        if (is_sqin(*first)) ++n;
    return n;
}

// Is this squeeze a squeeze-in? For sqin_rank over an activation's sqinouts.
inline bool is_sqin_squeeze(const SPSqueeze& q) { return q.kind == SqueezeKind::SqIn; }

struct Activation {
    // The search sets these on every activation it makes, so they are
    // plain values (since path format 6, docs/adr/0017).
    Timecode timecode;
    Chord chord;
    int frontend_points = 0;
    std::vector<BackendSqueeze> backends;
    std::vector<SPSqueeze> sqinouts;
    double e_offset = 0.0;

    // The deactivation node, the cap's clamp note and the collected phrases
    // are not fields: they are read from sp_end_steps, below.

    // The cap's clamp: when the meter was full and a phrase extended the
    // window, the end sat a fixed distance from that phrase's note, not from
    // the activation. That note is a Clamped step (clamp_tick()).

    // The chart tick of the SP phrase chord this activation squeezed out:
    // the phrase its deact edge offered it when the path took the SqOut
    // branch (core::offered_phrase).
    // Stamped by the search at copy-out (since path format 4, ADR 0014).
    // Unset when the activation did not squeeze out, or on an older record.
    // Nothing re-derives it.
    std::optional<int64_t> sqout_tick;

    // Each phrase chord collected while active is a step of sp_end_steps,
    // a late-SqIn phrase and a cap-clamped phrase included. A squeezed-out
    // phrase has no step (collected_phrase_ticks()).

    // Every place this window's SP end moved, in order, as the search did it.
    // The first step is the activation. A squeezed-out phrase has no step. A
    // tail activation's last end is the end the search tracked. Stamped at
    // copy-out; empty only on a hand-built activation. Every SP-end fact the
    // record holds is read from this list (the accessors below). Nothing
    // re-derives it.
    std::vector<SpEndStep> sp_end_steps;

    // Where each bar this activation spends arrived, since the previous window
    // closed (or the chart began), in order. A phrase hit at the cap gains
    // nothing and is not here. A squeezed-out phrase's bar is here at the
    // later of the previous deact node and that phrase, when the player hits
    // it. Stamped by the search. sp_meter() is its size.
    std::vector<int64_t> bank_rise_ticks;

    // Bars of SP this activation spends: one per stored arrival.
    int sp_meter() const { return static_cast<int>(bank_rise_ticks.size()); }

    // The fills the path was shown and passed over before this activation,
    // in chart order. The search charges a skip only on a fill it could have
    // taken (enough SP, deadline open) and stamps that fill's tick here.
    // Under the 1.0 fill rule these need not be the fills nearest the
    // activation. skips() is its size. The Preview lights exactly these.
    std::vector<int64_t> skipped_fill_ticks;

    // Fills passed over before this activation: one per stored fill.
    int skips() const { return static_cast<int>(skipped_fill_ticks.size()); }

    // Read from sp_end_steps; see each body in model.cpp.
    //   deact_tick()             - the deactivation node D: where this
    //                              window's SP ends, every extension included.
    //   clamp_tick()             - the collecting note the SP cap last pinned
    //                              the end to (ADR 0013); unset if none.
    //   collected_phrase_ticks() - every phrase chord the gauge received while
    //                              active, in chart order, a late-SqIn phrase
    //                              and a clamped phrase included; never a
    //                              squeezed-out phrase.
    std::optional<int64_t> deact_tick() const;           // the last step's end
    std::optional<int64_t> clamp_tick() const;           // the last Clamped step's tick
    std::vector<int64_t> collected_phrase_ticks() const; // every step after the first
    std::optional<int64_t> nominal_end() const;          // the first step's end
    std::optional<size_t> squeeze_end_step(size_t squeeze_index) const;
    std::optional<int64_t> squeeze_end_tick(size_t squeeze_index) const;
    int64_t end_anchor_tick(size_t step_index) const;
    int64_t refill_tick(size_t step_index) const;
    // The note whose timing moves D: end_anchor_tick of the last step.
    std::optional<int64_t> deact_anchor_tick() const;
    // The note whose timing moves the end squeeze k was measured from.
    std::optional<int64_t> squeeze_anchor_tick(size_t squeeze_index) const;

    // The frontend transfer scale at the deact node D, measured from
    // deact_anchor_tick() (the activation, or the cap's collecting note).
    // Computed by the search at copy-out and stored, so the details view
    // never needs a SongTiming. Display-only: difficulty and everything the
    // search, filter and report derive stay raw gap ms. Each SqIn stores the
    // scale at its own end (SPSqueeze::transfer). Unset means the search
    // could not compute it. That is a bug guard, never an expected state
    // (D4): a test proves no fresh record has it.
    std::optional<TransferScale> transfer_post;

    std::string notationstr() const;
    std::string notationstr_verbose() const;
    bool is_e_critical() const;  // inside the early-fill window: is_e0(e_offset, 0)
    bool is_E0() const;
    std::optional<double> e_difficulty(bool verbose = false) const;
    std::optional<double> difficulty() const;
    bool is_difficult() const;

    // Is this backend the note squeezed out of SP? Compares against the
    // sqout_tick the engine stored (path format 4 on, ADR 0014), so no display re-derives it.
    bool is_sqout_backend(const BackendSqueeze& bsq) const;

    // The squeezed-out chord's row, or nullptr when the activation did not
    // squeeze out. The one way to ask "which row, and how far from the SP
    // end": its offset_ms is the SqOut's offset.
    const BackendSqueeze* sqout_row() const;

    // Mark this activation as squeezing out the phrase chord at `tick`. The
    // only writer of a squeeze-out: it stamps sqout_tick, drops every row
    // past the chord (hit after SP ended), and appends the SqOut entry built
    // from the chord's own row. Throws std::logic_error when no row with an
    // offset sits on `tick`. The engine's copy-out and the codec call it.
    void set_sqout(int64_t tick);

    // Backends worth keeping: those near the deactivation, plus whatever note
    // is being squeezed out of SP however far out it lands. The details view
    // shows exactly these, and the path codec stores only these, so a stored
    // record shows the same rows as a fresh one.
    std::vector<BackendSqueeze> display_backends() const;
};

// sp_end_steps and the scales above are stored data only. Everything that
// derives or judges them — transfer_scale_between, frontend_transfer_scales,
// and the display-layer rating built on them — lives in
// core/squeeze_rating.h. activation_deact_tick lives there too, but it
// derives nothing: it just hands back deact_tick().

// A read-only walk over a path's activations: its own, then the variant tail
// it shares with its parent -- the order all_activations() copies them in.
// It holds pointers into the Path and copies nothing, so it is valid only
// while that Path is alive and unchanged.
class ActivationWalk {
public:
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Activation;
        using difference_type = std::ptrdiff_t;
        using pointer = const Activation*;
        using reference = const Activation&;

        iterator(const std::vector<Activation>* own, const std::vector<Activation>* tail,
                 size_t i)
            : own_(own), tail_(tail), i_(i) {}
        reference operator*() const {
            return i_ < own_->size() ? (*own_)[i_] : (*tail_)[i_ - own_->size()];
        }
        pointer operator->() const { return &**this; }
        iterator& operator++() {
            ++i_;
            return *this;
        }
        iterator operator++(int) {
            iterator old = *this;
            ++i_;
            return old;
        }
        bool operator==(const iterator& o) const { return i_ == o.i_; }
        bool operator!=(const iterator& o) const { return i_ != o.i_; }

    private:
        const std::vector<Activation>* own_;
        const std::vector<Activation>* tail_;
        size_t i_;
    };

    ActivationWalk(const std::vector<Activation>& own, const std::vector<Activation>& tail)
        : own_(&own), tail_(&tail) {}

    size_t size() const { return own_->size() + tail_->size(); }
    bool empty() const { return size() == 0; }
    const Activation& operator[](size_t i) const {
        return i < own_->size() ? (*own_)[i] : (*tail_)[i - own_->size()];
    }
    const Activation& front() const { return (*this)[0]; }
    const Activation& back() const { return (*this)[size() - 1]; }
    iterator begin() const { return iterator(own_, tail_, 0); }
    iterator end() const { return iterator(own_, tail_, size()); }

private:
    const std::vector<Activation>* own_;
    const std::vector<Activation>* tail_;
};

// ---- Path ---------------------------------------------------------------

struct Path {
    std::vector<Activation> activations;
    int notecount = 0;
    // Where each bar banked after the path's last window closed arrived, in
    // order, as Activation::bank_rise_ticks. leftover_sp() is its size. A
    // variant's is its own: the engine stores it and the record keeps it per
    // variant (D3, finding 89), so prepare_variants leaves it alone.
    std::vector<int64_t> trailing_bank_ticks;

    // Bars left after the last window: one per stored arrival.
    int leftover_sp() const { return static_cast<int>(trailing_bank_ticks.size()); }

    int64_t score_base = 0;
    int64_t score_combo = 0;
    int64_t score_sp = 0;
    int64_t score_solo = 0;
    int64_t score_accents = 0;
    int64_t score_ghosts = 0;

    std::vector<Path> variants;
    int tied_count = 1;
    std::optional<int> var_point;
    std::vector<Activation> variant_tail;

    // _activations then _variant_tail, as all_activations() yields.
    std::vector<Activation> all_activations() const;
    // The same activations, read in place. Prefer this unless the caller
    // really needs its own copy.
    ActivationWalk walk_activations() const { return ActivationWalk(activations, variant_tail); }
    bool has_activations() const;

    int64_t totalscore() const;
    std::string pathstring() const;
    // The Ctrl+C string: the chart's multiplier squeezes (from the record,
    // HydraRecord::multsqueezes), the verbose activations, the score.
    std::string pathstring_verbose(const std::vector<MultSqueeze>& multsqueezes) const;

    int tied_pathcount() const { return tied_count; }
    int recount_tied_paths();
    void prepare_variants();

    std::optional<double> difficulty() const;
    // True when the hardest squeeze or E0 fill is past kDifficultMs: the
    // warning color's rule, asked of the path instead of re-derived by callers.
    bool is_difficult() const;

    // An "all-0" path: it has activations and every one of them records
    // skips == 0. False for a path with no activations.
    bool is_allzero() const;

    // The chart's base score: every note at 1x with no Star Power (50 a gem,
    // 65 a cymbal, doubled for a ghost or accent). Clone Hero divides by
    // this for the average multiplier and multiplies it for star cutoffs.
    // Every path hits every note, so it is the same on every path.
    int64_t chart_base_score() const;

    // Points per scored note, on average. 0.0 for a path with no scoring
    // notes, instead of dividing by zero.
    double avg_mult() const;
};

// ---- HydraRecord --------------------------------------------------------

struct HydraRecord {
    std::optional<double> ms_limit;
    std::optional<int> sp_cap;
    // Always true since Auto went (2026-09-27): Auto was the only search that
    // could stop before its score settled. Kept because the stored path
    // structure carries it (store/path_codec.cpp); dropping it would change
    // the record format.
    bool sp_cap_converged = true;
    // The fingerprint of the rules the search ran under (stored since path
    // format 4): Rules::fingerprint(). Results Hydra 1.8.4's Auto saved
    // carry Rules::retired_auto_fingerprint() and are deleted when the store
    // opens (RecordStore::delete_auto_results). A record built in memory
    // starts with the default rules' fixed-cap fingerprint, computed once
    // (core::default_stamp), not once per record decoded; analyze_chart
    // stamps the real one. An older blob reads back core::kNoRulesFingerprint,
    // which matches no rules, so it can never pass as current.
    uint64_t rules_fingerprint = core::default_stamp().fixed;
    // True when fills spawned by Clone Hero 1.0's deadline, not 1.1's.
    // analyze_chart sets it. It is not in the stored bytes: the result's row
    // carries the rule in its key (store::Lens::legacy_fills), prepare_row
    // refuses a key that names the other rule, and get_record sets it back
    // from the key that found the row.
    bool legacy_fills = false;
    std::vector<Path> paths;

    // The chart's multiplier squeezes, in chart order. They depend on the
    // combo alone, never on the path, so a record holds one list rather than
    // one per path (docs/adr/0017).
    std::vector<MultSqueeze> multsqueezes;

    // The best all-0 path: the highest-scoring path whose activations all
    // record skips == 0, found under a 0 ms timing limit, plus the tied
    // variations a early fill or a squeeze in/out produces. The main
    // search keeps paths by score band, not by shape, so this path is usually
    // below the band and absent from `paths`. Empty when the search did not run
    // or found nothing. Deliberately NOT part of all_paths(): the reports and
    // the summary columns must keep counting generated paths only.
    std::vector<Path> allzero_paths;

    const Path& best_path() const { return paths.at(0); }
    Path& best_path() { return paths.at(0); }

    // Depth-first traversal of the path tree (root paths + their nested
    // variants), each path before its variants. Pointers into
    // `paths`; valid until the record is modified or moved.
    std::vector<const Path*> all_paths() const;

    // The same traversal over allzero_paths.
    std::vector<const Path*> all_allzero_paths() const;
};

// The traversal all_paths() uses, over any root list (a search's output, for
// one): each path, then its nested variants. Pointers into `roots`.
std::vector<const Path*> flatten_paths(const std::vector<Path>& roots);

// Format an integer with thousands separators, matching Python's `{:,}`.
std::string group_thousands(int64_t n);

}  // namespace hydra

#endif  // HYDRA_CORE_MODEL_H
