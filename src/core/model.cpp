#include "core/model.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include "core/backend_value.h"

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

// ---- ChordNote ----------------------------------------------------------

bool ChordNote::operator==(const ChordNote& o) const {
    return colortype == o.colortype && dynamictype == o.dynamictype &&
           cymbaltype == o.cymbaltype && is2x == o.is2x;
}

std::string ChordNote::str() const {
    std::string cym;
    if (allows_cymbals(colortype))
        cym = cymbaltype == NoteCymbalType::Cymbal ? "Cym" : "Tom";

    // Every modifier goes in one parenthesis, dynamic first, so a ghost 2x
    // kick reads "Kick (Ghost, 2x)".
    // Every lane carries dynamics, the kick included (ADR 0012).
    std::vector<std::string> mods;
    switch (dynamictype) {
        case NoteDynamicType::Normal: break;
        case NoteDynamicType::Ghost: mods.push_back("Ghost"); break;
        case NoteDynamicType::Accent: mods.push_back("Accent"); break;
    }
    if (is2x) mods.push_back("2x");

    std::string mod;
    for (size_t i = 0; i < mods.size(); ++i)
        mod += (i == 0 ? " (" : ", ") + mods[i];
    if (!mod.empty()) mod += ")";

    return color_str(colortype) + cym + mod;
}

int ChordNote::basescore() const {
    int points = kNoteBasePoints + (is_cymbal() ? kCymbalBonusPoints : 0);
    if (is_dynamic()) points *= 2;
    return points;
}

// ---- Chord: code --------------------------------------------------------

namespace {

// A lane's "upper case" flag: a cymbal on yellow/blue/green, 2x on the kick.
// Red has neither, so a red note is always lower case.
bool lane_flag(const ChordNote& note) {
    return note.colortype == NoteColor::Kick ? note.is2x : note.is_cymbal();
}

bool lane_allows_flag(NoteColor c) {
    return c == NoteColor::Kick || allows_cymbals(c);
}

}  // namespace

std::string Chord::code() const {
    std::string out(5, '.');
    for (int i = 0; i < 5; ++i) {
        if (!notemap_[i].has_value()) continue;
        const ChordNote& note = *notemap_[i];
        // A flag this lane cannot carry (a red cymbal, a 2x pad, a kick
        // cymbal) never comes out of the parsers; spelling it anyway would
        // read back as a different chord.
        const bool stray_flag = note.colortype == NoteColor::Kick
                                    ? note.is_cymbal()
                                    : note.is2x || (!allows_cymbals(note.colortype) &&
                                                    note.is_cymbal());
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

Chord Chord::from_code(const std::string& code) {
    if (code.size() != 5) throw std::out_of_range("unknown chord code: " + code);

    Chord chord;
    for (int i = 0; i < 5; ++i) {
        const char ch = code[i];
        if (ch == '.') continue;
        const NoteColor color = static_cast<NoteColor>(i + 1);
        const bool flag = ch >= 'A' && ch <= 'Z';
        const char lower = flag ? static_cast<char>(ch - 'A' + 'a') : ch;

        ChordNote note{color};
        switch (lower) {
            case 'n': note.dynamictype = NoteDynamicType::Normal; break;
            case 'g': note.dynamictype = NoteDynamicType::Ghost; break;
            case 'a': note.dynamictype = NoteDynamicType::Accent; break;
            default: throw std::out_of_range("unknown chord code: " + code);
        }
        if (flag) {
            if (!lane_allows_flag(color))
                throw std::out_of_range("unknown chord code: " + code);
            if (color == NoteColor::Kick) note.is2x = true;
            else note.cymbaltype = NoteCymbalType::Cymbal;
        }
        chord.insert_note(note);
    }
    return chord;
}

// ---- Chord: rest --------------------------------------------------------

bool Chord::operator==(const Chord& o) const {
    for (int i = 0; i < 5; ++i) {
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

std::vector<ChordNote> Chord::notes(bool basesorted) const {
    std::vector<ChordNote> out;
    for (const auto& slot : notemap_)
        if (slot.has_value()) out.push_back(*slot);
    if (basesorted) {
        std::stable_sort(out.begin(), out.end(),
                         [](const ChordNote& a, const ChordNote& b) {
                             return a.basescore() < b.basescore();
                         });
    }
    return out;
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

std::string Chord::rowstr() const {
    std::vector<ChordNote> ns = notes();
    std::string inner;
    for (size_t i = 0; i < ns.size(); ++i) {
        if (i) inner += " - ";
        inner += ns[i].str();
    }
    return "[" + inner + "]";
}

std::string Chord::notationstr() const {
    std::string krybg = "[";
    const NoteColor order[5] = {NoteColor::Kick, NoteColor::Red,
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

void Chord::add_2x() {
    add_note(NoteColor::Kick);
    at(NoteColor::Kick)->is2x = true;
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
    const NoteColor order[5] = {NoteColor::Green, NoteColor::Blue,
                                NoteColor::Yellow, NoteColor::Red,
                                NoteColor::Kick};
    for (NoteColor c : order) {
        const std::optional<ChordNote>& slot = at(c);
        if (slot.has_value()) return *slot;
    }
    throw std::runtime_error("activation_note on empty chord");
}

// ---- SPSqueeze ----------------------------------------------------------

std::string SPSqueeze::description() const {
    char buf[96];
    if (kind == SqueezeKind::SqIn)
        std::snprintf(buf, sizeof(buf),
                      "SqIn: Note timing must be earlier than %.1fms.", timing());
    else
        std::snprintf(buf, sizeof(buf),
                      "SqOut: Note timing must be later than %.1fms.", timing());
    return buf;
}

// ---- BackendSqueeze -----------------------------------------------------

bool BackendSqueeze::operator==(const BackendSqueeze& o) const {
    return timecode == o.timecode && chord == o.chord && points == o.points &&
           sqout_points == o.sqout_points && offset_ms == o.offset_ms;
}

std::string BackendSqueeze::summarystr(bool squeezed_out, double hit_window_ms,
                                       double leeway_ms) const {
    double off = offset_ms.value_or(0.0);
    const double w = hit_window_ms;
    if (squeezed_out) {
        if (off < -w) return "Insane SqOut";
        if (off < -10) return "Hard SqOut";
        if (off < 10) return "Standard SqOut";
        if (off < w) return "Easy SqOut";
        return "Free SqOut";
    }
    if (off < -w) return "Free";
    if (off < -10) return "Easy";
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

// A multiplier squeeze is a chord whose notes straddle a to_multiplier step
// (combos 10, 20, 30): combo_ is the combo before the chord, and note i scores
// at to_multiplier(combo_ + i). For 2- and 3-note chords, the combo set below
// plus the mod-10 rule (the last note lands on the step or one past it)
// accept exactly the straddling chords; the test "MultSqueeze accepts exactly
// the 2- and 3-note chords that straddle a multiplier step" checks this
// against to_multiplier. For 4-note chords only combos 7, 17 and 27 are
// accepted, and 5-note chords never are, although both also straddle from
// other combos. Why the set stops there is not recorded; it is kept fixed.
bool MultSqueeze::applies(const Chord& chord, int combo) {
    switch (combo) {
        case 7: case 8: case 17: case 18: case 27: case 28: break;
        default: return false;
    }
    const int mod = (chord.count() + combo) % 10;
    if (mod != 0 && mod != 1) return false;

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

int MultSqueeze::multiplier() const { return to_multiplier(combo_) + 1; }

std::string MultSqueeze::direction() const {
    return (combo_ % 10 == 7) ? "high" : "low";
}

int MultSqueeze::points() const {
    std::vector<ChordNote> order = chord_.notes(true);
    return order.back().basescore() - order.front().basescore();
}

std::string MultSqueeze::notationstr() const {
    return std::to_string(multiplier()) + "x";
}

std::string MultSqueeze::howto() const {
    // Ports MultSqueeze.guide_chords + .howto: every "edge" note (the note(s)
    // tied for the highest/lowest basescore, per direction()) is a single-note
    // chord that alone accomplishes the squeeze when hit last/first.
    std::vector<ChordNote> ordered = chord_.notes(/*basesorted=*/true);
    bool high = direction() == "high";
    // A 3-note chord splits two and one across the step. When only one end
    // holds a single note, placing that note settles the squeeze from either
    // side of the step, so name it: Kick + two cymbals is "Hit [Kick] first."
    // whether the kick is the one left behind or a cymbal is the one carried
    // over. With three different values the direction decides, as above.
    if (ordered.size() == 3) {
        const int lo = ordered[0].basescore();
        const int mid = ordered[1].basescore();
        const int hi = ordered[2].basescore();
        if (lo != mid && mid == hi) high = false;
        if (lo == mid && mid != hi) high = true;
    }
    int edge_score = high ? ordered.back().basescore() : ordered.front().basescore();

    std::string joined;
    for (const ChordNote& note : ordered) {
        if (note.basescore() != edge_score) continue;
        Chord edge;
        edge.insert_note(note);
        if (!joined.empty()) joined += " or ";
        joined += edge.rowstr();
    }
    return "Hit " + joined + (high ? " last." : " first.");
}

// ---- Activation ---------------------------------------------------------

// Inside the early-fill window, skipped fills or not: is_e0 with none skipped.
bool Activation::is_e_critical() const { return is_e0(e_offset, 0); }

bool Activation::is_E0() const { return is_e0(e_offset, skips()); }

std::optional<double> Activation::e_difficulty(bool verbose) const {
    if (is_E0() || verbose) return early_fill_difficulty(e_offset);
    return std::nullopt;
}

std::optional<double> Activation::difficulty() const {
    std::vector<double> diffs;
    for (const SPSqueeze& sq : sqinouts) diffs.push_back(sq.difficulty());
    if (auto e = e_difficulty()) diffs.push_back(*e);
    if (diffs.empty()) return std::nullopt;
    return *std::max_element(diffs.begin(), diffs.end());
}

bool Activation::is_difficult() const {
    if (auto e = e_difficulty(); e && *e > kDifficultMs) return true;
    for (const SPSqueeze& sq : sqinouts)
        if (sq.is_difficult()) return true;
    return false;
}

std::string Activation::notationstr() const {
    std::string e = is_e_critical() ? "E" : "";
    std::string syms;
    for (const SPSqueeze& sq : sqinouts) syms += sq.symbol();
    return e + std::to_string(skips()) + syms;
}

std::string Activation::notationstr_verbose() const {
    std::vector<std::string> timings;
    if (is_e_critical())
        timings.push_back(
            std::to_string(static_cast<long long>(*e_difficulty(true))) + " ms");
    for (const SPSqueeze& sq : sqinouts)
        timings.push_back(
            std::to_string(static_cast<long long>(sq.difficulty())) + " ms");

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
    std::vector<Activation> out;
    out.reserve(activations.size() + variant_tail.size());
    out.insert(out.end(), activations.begin(), activations.end());
    out.insert(out.end(), variant_tail.begin(), variant_tail.end());
    return out;
}

bool Path::has_activations() const {
    return !activations.empty() || !variant_tail.empty();
}

int64_t Path::totalscore() const {
    return score_base + score_combo + score_sp + score_solo + score_accents +
           score_ghosts;
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
            s += std::to_string(multsqueezes[i].multiplier()) + "x";
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

bool Path::is_difficult() const {
    const std::optional<double> d = difficulty();
    return d && *d > kDifficultMs;
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
    if (!row || !row->offset_ms) {
        sqout_tick.reset();
        throw std::logic_error("set_sqout: no backend row with an offset on tick " +
                               std::to_string(tick));
    }
    sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, *row->offset_ms});
}

std::vector<BackendSqueeze> Activation::display_backends() const {
    // Nothing past a squeezed-out note can be a backend: the sqout note is hit
    // after SP has ended, and every later note is hit after that one. The
    // engine's copy-out already drops those rows; this keeps the display
    // honest for any list that still holds one. Chart order is tick order.
    auto is_beyond_sqout = [this](const BackendSqueeze& bsq) {
        return sqout_tick.has_value() && bsq.timecode.ticks() > *sqout_tick;
    };

    std::vector<BackendSqueeze> out;
    for (const BackendSqueeze& bsq : backends) {
        if (is_beyond_sqout(bsq)) continue;
        if (within_squeeze_window(bsq.offset_ms.value_or(0.0)) || is_sqout_backend(bsq))
            out.push_back(bsq);
    }
    return out;
}

bool Path::is_allzero() const {
    const ActivationWalk acts = walk_activations();
    if (acts.empty()) return false;
    for (const Activation& act : acts)
        if (act.skips() != 0) return false;
    return true;
}

int64_t Path::chart_base_score() const {
    return score_base + score_ghosts + score_accents;
}

double Path::avg_mult() const {
    int64_t multscore = totalscore() - score_solo;
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

}  // namespace hydra
