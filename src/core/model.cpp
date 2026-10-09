#include "core/model.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <limits>

#include "core/backend_value.h"
#include "core/scoring.h"  // category_scores, multsqueeze_gain
#include "core/stars.h"    // score_without_solo

namespace hydra {

// ---- enums --------------------------------------------------------------

bool allows_cymbals(NoteColor c) {
    return c == NoteColor::Yellow || c == NoteColor::Blue ||
           c == NoteColor::Green;
}

std::string color_str(NoteColor c) {
    switch (c) {
        case NoteColor::Kick: return "Kick";
        case NoteColor::Red: return "Red";
        case NoteColor::Yellow: return "Yellow";
        case NoteColor::Blue: return "Blue";
        case NoteColor::Green: return "Green";
    }
    return "";
}

std::string dynamic_str(NoteDynamicType t) {
    switch (t) {
        case NoteDynamicType::Normal: return "none";
        case NoteDynamicType::Ghost: return "ghost";
        case NoteDynamicType::Accent: return "accent";
    }
    return "none";
}

std::string dynamic_label(NoteDynamicType t) {
    if (t == NoteDynamicType::Normal) return "";
    std::string word = dynamic_str(t);
    word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
    return word;
}

std::string color_notationstr(NoteColor c) {
    switch (c) {
        case NoteColor::Kick: return "K";
        case NoteColor::Red: return "R";
        case NoteColor::Yellow: return "Y";
        case NoteColor::Blue: return "B";
        case NoteColor::Green: return "G";
    }
    return "";
}

std::string note_label(const ChordNote& note, bool pro) {
    if (note.colortype == NoteColor::Kick) return lane_flag(note) ? "2x kick" : "Kick";
    const std::string colour = color_str(note.colortype);
    if (!allows_cymbals(note.colortype)) return pro ? colour + " snare" : colour;  // red
    if (note.is_cymbal()) return colour + " cymbal";
    return pro ? colour + " tom" : colour;
}

// ---- ChordNote ----------------------------------------------------------

bool ChordNote::operator==(const ChordNote& o) const {
    return colortype == o.colortype && dynamictype == o.dynamictype &&
           cymbaltype == o.cymbaltype && is2x == o.is2x;
}

// The note's name (note_label, with the Pro Drums setting `pro`), then a
// ghost or accent in parentheses, so a ghost 2x kick reads "2x kick (Ghost)".
// Every lane carries dynamics, the kick included (ADR 0012).
std::string ChordNote::str(bool pro) const {
    const std::string word = dynamic_label(dynamictype);
    return note_label(*this, pro) + (word.empty() ? "" : " (" + word + ")");
}

int ChordNote::basescore() const {
    int points = kNoteBasePoints + (is_cymbal() ? kCymbalBonusPoints : 0);
    if (is_dynamic()) points *= 2;
    return points;
}

// ---- Chord: code --------------------------------------------------------

bool lane_allows_flag(NoteColor c) {
    return c == NoteColor::Kick || allows_cymbals(c);
}

void set_lane_flag(ChordNote& note) {
    if (!lane_allows_flag(note.colortype))
        throw std::logic_error("set_lane_flag: " + color_str(note.colortype) +
                               " has no flag");
    if (note.colortype == NoteColor::Kick) note.is2x = true;
    else note.cymbaltype = NoteCymbalType::Cymbal;
}

bool lane_flag(const ChordNote& note) {
    return note.colortype == NoteColor::Kick ? note.is2x : note.is_cymbal();
}

std::string Chord::code() const {
    std::string out(kLanes, '.');
    for (size_t i = 0; i < kLanes; ++i) {
        if (!notemap_[i].has_value()) continue;
        const ChordNote& note = *notemap_[i];
        // A flag this lane cannot carry (a red cymbal, a 2x pad, a kick
        // cymbal) never comes out of the parsers; spelling it anyway would
        // read back as a different chord. The note must be its plain self or
        // its lane's flagged self (set_lane_flag), nothing else.
        ChordNote plain = note;
        plain.is2x = false;
        plain.cymbaltype = NoteCymbalType::Normal;
        bool stray_flag = note != plain;
        if (stray_flag && lane_allows_flag(note.colortype)) {
            ChordNote flagged = plain;
            set_lane_flag(flagged);
            stray_flag = note != flagged;
        }
        if (stray_flag)
            throw std::logic_error("chord note has a flag its lane cannot carry");
        char ch = note.dynamictype == NoteDynamicType::Ghost    ? 'g'
                  : note.dynamictype == NoteDynamicType::Accent ? 'a'
                                                                : 'n';
        if (lane_flag(note)) ch = static_cast<char>(ch - 'a' + 'A');
        out[i] = ch;
    }
    return out;
}

// ---- Chord: rest --------------------------------------------------------

bool Chord::operator==(const Chord& o) const {
    for (size_t i = 0; i < kLanes; ++i) {
        if (notemap_[i].has_value() != o.notemap_[i].has_value()) return false;
        if (notemap_[i].has_value() && !(*notemap_[i] == *o.notemap_[i]))
            return false;
    }
    return true;
}

std::optional<ChordNote>& Chord::at(NoteColor c) {
    return notemap_[static_cast<int>(c) - 1];
}

const std::optional<ChordNote>& Chord::at(NoteColor c) const {
    return notemap_[static_cast<int>(c) - 1];
}

Chord::NoteList Chord::note_list(bool basesorted) const {
    NoteList out;
    for (const auto& slot : notemap_)
        if (slot.has_value()) out.notes_[out.count_++] = *slot;
    if (!basesorted) return out;
    // An insertion sort moves a note only past higher scores, so ties keep
    // lane order: the stable sort the header promises, with no heap.
    for (size_t i = 1; i < out.count_; ++i) {
        const ChordNote x = out.notes_[i];
        const int bx = x.basescore();
        size_t j = i;
        while (j > 0 && bx < out.notes_[j - 1].basescore()) {
            out.notes_[j] = out.notes_[j - 1];
            --j;
        }
        out.notes_[j] = x;
    }
    return out;
}

std::vector<ChordNote> Chord::notes(bool basesorted) const {
    const NoteList list = note_list(basesorted);
    return std::vector<ChordNote>(list.begin(), list.end());
}

int Chord::count() const {
    int n = 0;
    for (const auto& slot : notemap_)
        if (slot.has_value()) ++n;
    return n;
}

int Chord::hands_count() const {
    return at(NoteColor::Kick).has_value() ? count() - 1 : count();
}

std::string Chord::rowstr(bool pro) const {
    std::vector<ChordNote> ns = notes();
    std::string inner;
    for (size_t i = 0; i < ns.size(); ++i) {
        if (i) inner += " - ";
        inner += ns[i].str(pro);
    }
    return "[" + inner + "]";
}

std::string Chord::notationstr() const {
    std::string krybg = "[";
    const NoteColor order[kLanes] = {NoteColor::Kick, NoteColor::Red,
                                NoteColor::Yellow, NoteColor::Blue,
                                NoteColor::Green};
    for (NoteColor c : order)
        krybg += at(c).has_value() ? color_notationstr(c) : " ";
    return krybg + "]";
}

void Chord::apply_disco_flip() {
    std::optional<ChordNote> red = at(NoteColor::Red);
    std::optional<ChordNote> yellow = at(NoteColor::Yellow);

    if (red) {
        red->cymbaltype = NoteCymbalType::Cymbal;
        red->colortype = NoteColor::Yellow;
    }
    if (yellow) {
        yellow->cymbaltype = NoteCymbalType::Normal;
        yellow->colortype = NoteColor::Red;
    }

    at(NoteColor::Red) = yellow;
    at(NoteColor::Yellow) = red;
}

void Chord::apply_flam_conversion() {
    if (hands_count() != 1) return;
    ChordNote flam = activation_note();
    switch (flam.colortype) {
        case NoteColor::Red: flam.colortype = NoteColor::Yellow; break;
        case NoteColor::Yellow: flam.colortype = NoteColor::Blue; break;
        case NoteColor::Blue: flam.colortype = NoteColor::Green; break;
        case NoteColor::Green: flam.colortype = NoteColor::Blue; break;
        default: break;  // Kick: no conversion, as the Python match has no case.
    }
    insert_note(flam);
}

ChordNote& Chord::add_note(NoteColor color) {
    std::optional<ChordNote>& slot = at(color);
    if (slot.has_value()) throw ChartFileError("Duplicate note.");
    slot = ChordNote{color};
    return *slot;
}

void Chord::insert_note(const ChordNote& note) { at(note.colortype) = note; }

// Clone Hero's readers keep both kicks; its track finalizer then ORs their
// flags onto one of them and deletes the other (0x215B8C0, 0x5DB0C0), so the
// kick that stays carries the 2x mark and both kicks' ghost or accent marks.
ChordNote& Chord::add_kick(bool is2x, NoteDynamicType dyn) {
    std::optional<ChordNote>& kick = at(NoteColor::Kick);
    if (!kick.has_value()) {
        ChordNote& added = add_note(NoteColor::Kick);
        added.dynamictype = dyn;
        if (is2x) set_lane_flag(added);
        return added;
    }
    // Two kicks of one kind: add_note raises the duplicate.
    if (lane_flag(*kick) == is2x) return add_note(NoteColor::Kick);
    set_lane_flag(*kick);
    // One dynamictype can't hold a ghost and an accent together, and what the
    // game scores that as is an open user question (D105), so a kick that
    // already has a mark keeps it, as the first kick did before D105.
    if (!kick->is_dynamic()) kick->dynamictype = dyn;
    return *kick;
}

void Chord::add_2x() { add_kick(true, NoteDynamicType::Normal); }

bool Chord::apply_2x_bass(bool bass2x) {
    std::optional<ChordNote>& kick = at(NoteColor::Kick);
    if (bass2x || !kick.has_value() || !lane_flag(*kick)) return false;
    kick.reset();
    return true;
}

// A cymbal, ghost or accent marker with no note of its colour under it is a
// malformed line, like a duplicate note. It raises ChartFileError, which the
// .chart parser's per-op catch drops, so the rest of the chart loads. Clone
// Hero skips such a marker too (0x215DDB0).
void Chord::apply_cymbal(NoteColor color) {
    if (!at(color)) throw ChartFileError("cymbal marker with no note under it");
    at(color)->cymbaltype = NoteCymbalType::Cymbal;
}

void Chord::apply_ghost(NoteColor color) {
    if (!at(color)) throw ChartFileError("ghost marker with no note under it");
    at(color)->dynamictype = NoteDynamicType::Ghost;
}

void Chord::apply_accent(NoteColor color) {
    if (!at(color)) throw ChartFileError("accent marker with no note under it");
    at(color)->dynamictype = NoteDynamicType::Accent;
}

const ChordNote& Chord::activation_note() const {
    const NoteColor order[kLanes] = {NoteColor::Green, NoteColor::Blue,
                                NoteColor::Yellow, NoteColor::Red,
                                NoteColor::Kick};
    for (NoteColor c : order) {
        const std::optional<ChordNote>& slot = at(c);
        if (slot.has_value()) return *slot;
    }
    throw std::runtime_error("activation_note on empty chord");
}

// ---- SPSqueeze ----------------------------------------------------------

std::optional<SqueezeKind> squeeze_kind_from_name(std::string_view name) {
    for (SqueezeKind kind : {SqueezeKind::SqIn, SqueezeKind::SqOut})
        if (name == SPSqueeze{kind, 0.0}.type_name()) return kind;
    return std::nullopt;
}

// ---- BackendSqueeze -----------------------------------------------------

bool BackendSqueeze::operator==(const BackendSqueeze& o) const {
    return timecode == o.timecode && chord == o.chord && points == o.points &&
           sqout_points == o.sqout_points && offset_ms == o.offset_ms;
}

double BackendSqueeze::offset() const {
    if (!offset_ms)
        throw std::logic_error("backend row on tick " + std::to_string(timecode.ticks()) +
                               " has no offset");
    return *offset_ms;
}

std::string BackendSqueeze::summarystr(bool squeezed_out, double hit_window_ms,
                                       double leeway_ms) const {
    const double off = offset();
    const double w = hit_window_ms;
    const double band = kBackendInnerBandMs;
    if (squeezed_out) {
        // A row the engine does not count says so, as the plain rows do
        // (finding 33): the same test, the same tag.
        const char* tag = core::counted_without_squeeze(off, leeway_ms) ? "" : " (uncounted)";
        const char* label = off < -w     ? "Insane SqOut"
                            : off < -band ? "Hard SqOut"
                            : off < band  ? "Standard SqOut"
                            : off < w     ? "Easy SqOut"
                                          : "Free SqOut";
        return std::string(label) + tag;
    }
    if (off < -w) return "Free";
    if (off < -band) return "Easy";
    // Counted by the engine with no squeeze: the same edge it prices with.
    if (core::counted_without_squeeze(off, leeway_ms)) return "Standard";
    if (off < w) return "Hard (uncounted)";
    return "Insane (uncounted)";
}

// ---- MultSqueeze --------------------------------------------------------

MultSqueeze::MultSqueeze(Chord chord, int combo)
    : chord_(std::move(chord)), combo_(combo) {
    validate();
}

// Every chord of any size whose notes straddle a to_multiplier step counts
// (D51 call 3, with the user's note that 4- and 5-note chords are valid in
// Clone Hero, so Hydra must get them right).
bool MultSqueeze::applies(const Chord& chord, int combo) {
    if (!(to_multiplier(combo + 1) < to_multiplier(combo + chord.count()))) return false;

    // A chord whose notes are all worth the same has nothing to squeeze.
    const std::vector<ChordNote> notes = chord.notes();
    for (const ChordNote& n : notes)
        if (n.basescore() != notes[0].basescore()) return true;
    return false;
}

void MultSqueeze::validate() const {
    if (!applies(chord_, combo_))
        throw std::invalid_argument("not a multiplier squeeze at this combo");
}

// The multiplier the last note is paid at when the chord is hit in the best
// order, read from the payout.
int MultSqueeze::multiplier() const { return category_scores(chord_, combo_).multiplier_after; }

namespace {

// The notes of a multiplier-squeeze chord the player has to place, read from
// the payouts: `first` must be hit before the step, `last` past it.
struct SqueezeAdvice {
    std::vector<ChordNote> first;
    std::vector<ChordNote> last;
};

SqueezeAdvice squeeze_advice(const Chord& chord, int combo) {
    // In the best order (cheapest first) the notes past the step are the
    // ones paid at a higher multiplier than the first note.
    std::vector<CategoryScores> per_note;
    category_scores(chord, combo, &per_note);
    const std::vector<ChordNote> best = chord.notes(true);
    size_t step = 1;
    while (per_note[step].multiplier == per_note[0].multiplier) ++step;
    const int dearest_before = best[step - 1].basescore();
    const int cheapest_past = best[step].basescore();

    // A note cheaper than every note past the step has to stay before it, and
    // a note dearer than every note before the step has to cross it. A note
    // worth the value the two sides share can go either way.
    std::vector<ChordNote> must_stay, must_cross;
    for (const ChordNote& note : best) {
        if (note.basescore() < cheapest_past) must_stay.push_back(note);
        if (note.basescore() > dearest_before) must_cross.push_back(note);
    }
    SqueezeAdvice advice;
    if (must_cross.empty()) {
        advice.first = must_stay;
    } else if (must_stay.empty()) {
        advice.last = must_cross;
    } else if (dearest_before == cheapest_past) {
        // Tied notes fill the middle, so both ends need naming.
        advice.first = must_stay;
        advice.last = must_cross;
    } else if (must_stay.size() == 1) {
        // One note alone before the step: placing it settles the order.
        advice.first = must_stay;
    } else {
        // Otherwise name every note that crosses (D51 call 3).
        advice.last = must_cross;
    }
    return advice;
}

// "[YellowCym] and [BlueCym]": each note as its own one-note chord, named in
// the Pro Drums setting's words (Chord::rowstr).
std::string joined_notes(const std::vector<ChordNote>& notes, bool pro) {
    std::string joined;
    for (const ChordNote& note : notes) {
        Chord one;
        one.insert_note(note);
        if (!joined.empty()) joined += " and ";
        joined += one.rowstr(pro);
    }
    return joined;
}

}  // namespace

// "high" when the advice names notes to hit last, "low" when it names only
// notes to hit first.
std::string MultSqueeze::direction() const {
    return squeeze_advice(chord_, combo_).last.empty() ? "low" : "high";
}

int MultSqueeze::points() const { return multsqueeze_gain(chord_, combo_); }

std::string MultSqueeze::notationstr() const {
    return std::to_string(multiplier()) + "x";
}

std::string MultSqueeze::howto(bool pro) const {
    const SqueezeAdvice advice = squeeze_advice(chord_, combo_);
    if (advice.last.empty()) return "Hit " + joined_notes(advice.first, pro) + " first.";
    if (advice.first.empty()) return "Hit " + joined_notes(advice.last, pro) + " last.";
    return "Hit " + joined_notes(advice.first, pro) + " first and " + joined_notes(advice.last, pro) +
           " last.";
}

// ---- Activation ---------------------------------------------------------

// Inside the early-fill window, skipped fills or not: is_e0 with none skipped.
bool Activation::is_e_critical() const { return is_e0(e_offset, 0); }

bool Activation::is_E0() const { return is_e0(e_offset, skips()); }

std::optional<double> Activation::e_difficulty(bool verbose) const {
    if (is_E0() || verbose) return early_fill_difficulty(e_offset);
    return std::nullopt;
}

namespace {

// hardest() and badge_timing() in one place; they differ only in whether a
// free squeeze takes part.
std::optional<HardestTiming> hardest_part(const Activation& act, bool free_squeezes) {
    std::optional<HardestTiming> best;
    // Squeezes first, in list order, and only a strictly larger value takes
    // over: so a tie keeps the first squeeze, and the fill below must beat
    // every squeeze to be named.
    for (const SPSqueeze& sq : act.sqinouts) {
        if (!free_squeezes && sq.is_free()) continue;
        const double d = sq.difficulty();
        if (!best || d > best->ms)
            best = HardestTiming{sq.kind == SqueezeKind::SqIn ? TimingPart::SqueezeIn
                                                              : TimingPart::SqueezeOut,
                                 d};
    }
    // Either early fill below counts only when it needs timing.
    const bool fill_needs_timing = early_fill_needs_timing(act.e_offset);
    if (const std::optional<double> e = act.e_difficulty(); e && fill_needs_timing) {
        if (!best || *e > best->ms) best = HardestTiming{TimingPart::EarlyFill, *e};
    }
    // The optional early fill of an E activation that skipped fills (D48 Q10).
    if (!best && fill_needs_timing && act.is_e_critical())
        best = HardestTiming{TimingPart::EarlyFill, *act.e_difficulty(/*verbose=*/true)};
    return best;
}

}  // namespace

std::optional<HardestTiming> Activation::hardest() const {
    // Only timings that need hitting take part (D51 call 4, D13): a free
    // squeeze (SPSqueeze::is_free) and an early fill with time to spare
    // (early_fill_needs_timing) never name the hardest part.
    return hardest_part(*this, /*free_squeezes=*/false);
}

std::optional<HardestTiming> Activation::badge_timing() const {
    // A timing that needs hitting always names the badge. Only when there is
    // none does a free squeeze get it (D80); fills still follow hardest()'s
    // rules, so this fallback only ever finds a free squeeze.
    if (std::optional<HardestTiming> h = hardest()) return h;
    return hardest_part(*this, /*free_squeezes=*/true);
}

std::optional<double> Activation::difficulty() const {
    const std::optional<HardestTiming> h = hardest();
    if (!h) return std::nullopt;
    // An optional early fill is never required, so it is not a difficulty.
    if (h->part == TimingPart::EarlyFill && !is_E0()) return std::nullopt;
    return h->ms;
}

// D51 call 4 and D13: hardest() only names a timing that needs hitting, so
// difficulty() is empty exactly when there is nothing to time.
bool Activation::needs_timing() const { return difficulty().has_value(); }

bool Activation::within_ms_limit(double limit_ms) const {
    for (const SPSqueeze& sq : sqinouts)
        if (!sq.within_limit(limit_ms)) return false;
    return fill_within_limit(e_offset, skips(), limit_ms);
}

bool Activation::is_difficult() const {
    const std::optional<double> d = difficulty();
    return d && past_difficult_floor(*d);
}

std::string Activation::notationstr() const {
    std::string e = is_e_critical() ? "E" : "";
    std::string syms;
    for (const SPSqueeze& sq : sqinouts) syms += sq.symbol();
    return e + std::to_string(skips()) + syms;
}

std::string Activation::notationstr_verbose() const {
    std::vector<std::string> timings;
    if (is_e_critical()) timings.push_back(format_ms_whole(*e_difficulty(true)));
    for (const SPSqueeze& sq : sqinouts) timings.push_back(format_ms_whole(sq.difficulty()));

    if (timings.empty()) return notationstr();

    std::string joined;
    for (size_t i = 0; i < timings.size(); ++i) {
        if (i) joined += "/";
        joined += timings[i];
    }
    return notationstr() + " (" + joined + ")";
}

// ---- Path ---------------------------------------------------------------

std::vector<Activation> Path::all_activations() const {
    const ActivationWalk walk = walk_activations();
    return std::vector<Activation>(walk.begin(), walk.end());
}

bool Path::has_activations() const { return !walk_activations().empty(); }

int64_t Path::totalscore() const {
    return score_total(score_base, score_combo, score_sp, score_solo, score_accents,
                       score_ghosts);
}

std::string Path::pathstring() const {
    if (!has_activations()) return "(No activations.)";
    const ActivationWalk acts = walk_activations();
    std::string out;
    for (size_t i = 0; i < acts.size(); ++i) {
        if (i) out += " ";
        out += acts[i].notationstr();
    }
    return out;
}

std::string Path::pathstring_verbose(const std::vector<MultSqueeze>& multsqueezes) const {
    std::vector<std::string> sections;

    if (!multsqueezes.empty()) {
        std::string s;
        for (size_t i = 0; i < multsqueezes.size(); ++i) {
            if (i) s += ", ";
            s += multsqueezes[i].notationstr();
        }
        sections.push_back(s);
    } else {
        sections.push_back("(No mult squeezes.)");
    }

    if (has_activations()) {
        const ActivationWalk acts = walk_activations();
        std::string s;
        for (size_t i = 0; i < acts.size(); ++i) {
            if (i) s += " ";
            s += acts[i].notationstr_verbose();
        }
        sections.push_back(s);
    } else {
        sections.push_back("(No activations.)");
    }

    sections.push_back("Score: " + group_thousands(totalscore()));

    std::string out;
    for (size_t i = 0; i < sections.size(); ++i) {
        if (i) out += " | ";
        out += sections[i];
    }
    return out;
}

std::string path_identity(const Path& path) {
    // "score|tick>deact tick ..." with "-" for an activation that has no
    // deact tick. Only compared, so the form just has to be unambiguous.
    std::string out = std::to_string(path.totalscore()) + "|";
    for (const Activation& act : path.walk_activations()) {
        const std::optional<int64_t> deact = act.deact_tick();
        out += std::to_string(act.timecode.ticks()) + ">" +
               (deact ? std::to_string(*deact) : std::string("-")) + " ";
    }
    return out;
}

int Path::recount_tied_paths() {
    tied_count = 1;
    for (Path& v : variants) {
        v.recount_tied_paths();
        tied_count += v.tied_count;
    }
    return tied_count;
}

void Path::prepare_variants() {
    const ActivationWalk mine = walk_activations();
    for (Path& v : variants) {
        const size_t vp = static_cast<size_t>(v.var_point.value_or(0));
        v.variant_tail.clear();
        for (size_t i = vp; i < mine.size(); ++i) v.variant_tail.push_back(mine[i]);
        v.score_base = score_base;
        v.score_combo = score_combo;
        v.score_sp = score_sp;
        v.score_solo = score_solo;
        v.score_accents = score_accents;
        v.score_ghosts = score_ghosts;
        v.notecount = notecount;
        // trailing_bank_ticks is the variant's own: the engine stores it and
        // the record keeps it per variant (D3, finding 89).
        v.prepare_variants();
    }
}

std::optional<double> Path::difficulty() const {
    std::optional<double> best;
    for (const Activation& act : walk_activations()) {
        if (auto d = act.difficulty()) {
            if (!best || *d > *best) best = d;
        }
    }
    return best;
}

// The same activations difficulty() walks (D51 call 4).
bool Path::needs_timing() const {
    for (const Activation& act : walk_activations())
        if (act.needs_timing()) return true;
    return false;
}

bool Path::within_ms_limit(double limit_ms) const {
    for (const Activation& act : walk_activations())
        if (!act.within_ms_limit(limit_ms)) return false;
    return true;
}

bool Path::is_difficult() const {
    const std::optional<double> d = difficulty();
    return d && past_difficult_floor(*d);
}

// ---- the SP-end history's readers -----------------------------------------
// Each one reads sp_end_steps and nothing else (R1). An empty list (a
// hand-built activation) gives "unset", never a guess.

std::optional<int64_t> Activation::deact_tick() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return sp_end_steps.back().end_tick;
}

namespace {

// Which note did the SP cap last pin, at or before step `last`? The tick of
// the latest Clamped step up to there, else unset. clamp_tick (the whole
// history) and end_anchor_tick (up to one step) both ask it.
std::optional<int64_t> last_clamp_tick(const std::vector<SpEndStep>& steps, size_t last) {
    for (size_t s = std::min(last + 1, steps.size()); s-- > 0;)
        if (is_clamp_kind(steps[s].kind)) return steps[s].tick;
    return std::nullopt;
}

}  // namespace

std::optional<int64_t> Activation::clamp_tick() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return last_clamp_tick(sp_end_steps, sp_end_steps.size() - 1);
}

std::vector<int64_t> Activation::collected_phrase_ticks() const {
    std::vector<int64_t> out;
    for (size_t k = 1; k < sp_end_steps.size(); ++k) out.push_back(sp_end_steps[k].tick);
    return out;
}

std::optional<int64_t> Activation::nominal_end() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return sp_end_steps.front().end_tick;
}

std::optional<size_t> Activation::squeeze_end_step(size_t squeeze_index) const {
    if (squeeze_index >= sqinouts.size() || sp_end_steps.empty()) return std::nullopt;
    if (sqinouts[squeeze_index].kind == SqueezeKind::SqOut) return sp_end_steps.size() - 1;
    // The k-th SqIn squeeze is the k-th SqIn step (nth_sqin_step). Its end
    // was measured from the step before it.
    const size_t k = sqin_rank(sqinouts.begin(), sqinouts.begin() + squeeze_index, is_sqin_squeeze);
    const auto step = nth_sqin_step(sp_end_steps.begin() + 1, sp_end_steps.end(), k);
    if (step == sp_end_steps.end()) return std::nullopt;
    return static_cast<size_t>(step - sp_end_steps.begin()) - 1;
}

std::optional<int64_t> Activation::squeeze_end_tick(size_t squeeze_index) const {
    const std::optional<size_t> s = squeeze_end_step(squeeze_index);
    if (!s) return std::nullopt;
    return sp_end_steps[*s].end_tick;
}

// D1: an end is measured from the note whose timing moves it, the latest
// clamp at or before the step that set it, else the activation.
int64_t Activation::end_anchor_tick(size_t step_index) const {
    return last_clamp_tick(sp_end_steps, step_index).value_or(timecode.ticks());
}

int64_t Activation::refill_tick(size_t step_index) const {
    const SpEndStep& s = sp_end_steps.at(step_index);
    if (step_index == 0 || !is_sqin_kind(s.kind)) return s.tick;
    // A late squeeze-in: the phrase sits past the end in force, and the
    // player hits it early, so the bar arrives at that end.
    return std::min(s.tick, sp_end_steps[step_index - 1].end_tick);
}

// The note whose timing moves the final SP end D: the latest Clamped step,
// else the activation (D1). Unset when the history is empty (old records).
std::optional<int64_t> Activation::deact_anchor_tick() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return end_anchor_tick(sp_end_steps.size() - 1);
}

// The note whose timing moves the SP end squeeze k was measured from.
// Unset together with squeeze_end_tick(k).
std::optional<int64_t> Activation::squeeze_anchor_tick(size_t squeeze_index) const {
    const std::optional<size_t> s = squeeze_end_step(squeeze_index);
    if (!s) return std::nullopt;
    return end_anchor_tick(*s);
}

// The engine stamps the squeezed-out chord's tick at copy-out (since path
// format 4, ADR 0014). A record without it is Stale and is never guessed at.
bool Activation::is_sqout_backend(const BackendSqueeze& bsq) const {
    return core::sqout_position(bsq.timecode.ticks(), sqout_tick) == core::SqOutPosition::Exact;
}

const BackendSqueeze* Activation::sqout_row() const {
    if (!sqout_tick) return nullptr;
    for (const BackendSqueeze& b : backends)
        if (is_sqout_backend(b)) return &b;
    return nullptr;
}

void Activation::set_sqout(int64_t tick) {
    // Rows past the squeezed-out chord are hit after SP ended: drop them.
    backends.erase(std::remove_if(backends.begin(), backends.end(),
                                  [tick](const BackendSqueeze& b) {
                                      return core::sqout_position(b.timecode.ticks(), tick) ==
                                             core::SqOutPosition::After;
                                  }),
                   backends.end());
    sqout_tick = tick;
    const BackendSqueeze* row = sqout_row();
    // No row on the tick, or a row with no offset (BackendSqueeze::offset
    // throws): undo the stamp and fail loudly.
    try {
        if (!row)
            throw std::logic_error("set_sqout: no backend row on tick " + std::to_string(tick));
        sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, row->offset()});
    } catch (const std::logic_error&) {
        sqout_tick.reset();
        throw;
    }
}

std::vector<BackendSqueeze> Activation::display_backends() const {
    // Nothing past a squeezed-out note can be a backend: the sqout note is hit
    // after SP has ended, and every later note is hit after that one. The
    // engine's copy-out already drops those rows; this keeps the display
    // honest for any list that still holds one. core::sqout_position says
    // which rows are past it.
    std::vector<BackendSqueeze> out;
    for (const BackendSqueeze& bsq : backends) {
        if (core::sqout_position(bsq.timecode.ticks(), sqout_tick) == core::SqOutPosition::After)
            continue;
        if (within_squeeze_window(bsq.offset()) || is_sqout_backend(bsq))
            out.push_back(bsq);
    }
    return out;
}

bool Path::is_allzero() const {
    const ActivationWalk acts = walk_activations();
    if (acts.empty()) return false;
    for (const Activation& act : acts)
        if (!allzero_activation(act.e_offset, act.skips())) return false;
    return true;
}

int64_t Path::chart_base_score() const {
    return score_base + score_ghosts + score_accents;
}

double Path::avg_mult() const {
    const int64_t multscore = score_without_solo(*this);
    int64_t basescore = chart_base_score();
    if (basescore == 0) return 0.0;
    return static_cast<double>(multscore) / static_cast<double>(basescore);
}

// ---- HydraRecord ---------------------------------------------------------

// Depth-first walk over a root list and its nested variants, each path
// before its variants. Shared by all_paths() and all_allzero_paths()
// so both traversals stay identical.
std::vector<const Path*> flatten_paths(const std::vector<Path>& roots) {
    std::vector<const Path*> out;
    std::vector<const Path*> queue(roots.size());
    for (size_t i = 0; i < roots.size(); ++i) queue[i] = &roots[i];

    while (!queue.empty()) {
        const Path* p = queue.front();
        queue.erase(queue.begin());
        std::vector<const Path*> variants(p->variants.size());
        for (size_t i = 0; i < p->variants.size(); ++i) variants[i] = &p->variants[i];
        queue.insert(queue.begin(), variants.begin(), variants.end());
        out.push_back(p);
    }
    return out;
}

std::vector<const Path*> HydraRecord::all_paths() const {
    return flatten_paths(paths);
}

std::vector<const Path*> HydraRecord::all_allzero_paths() const {
    return flatten_paths(allzero_paths);
}

bool HydraRecord::is_optimal(const Path& path) const {
    return !paths.empty() && path.totalscore() == best_path().totalscore();
}

// ---- helpers ------------------------------------------------------------

std::string group_thousands(int64_t n) {
    bool neg = n < 0;
    // Build the magnitude without overflowing on INT64_MIN.
    uint64_t v = neg ? (~static_cast<uint64_t>(n) + 1ULL)
                     : static_cast<uint64_t>(n);
    std::string digits = std::to_string(v);

    std::string out;
    int cnt = 0;
    for (int i = static_cast<int>(digits.size()) - 1; i >= 0; --i) {
        out.push_back(digits[static_cast<size_t>(i)]);
        if (++cnt % 3 == 0 && i != 0) out.push_back(',');
    }
    std::reverse(out.begin(), out.end());
    if (neg) out = "-" + out;
    return out;
}

std::string counted(int64_t n, const std::string& one, const std::string& many) {
    return group_thousands(n) + " " + (n == 1 ? one : many);
}

const char* has_have(int64_t n) {
    return n == 1 ? "has" : "have";
}

std::string format_ms_whole(double ms) {
    return std::to_string(std::lround(ms)) + " ms";
}

}  // namespace hydra
