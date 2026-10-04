#include "parse/song.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include "core/strutil.h"
#include "core/winstr.h"  // read_file_bytes
#include "parse/midi.h"
#include "parse/sng.h"
#include "parse/chart_files.h"
#include "parse/srb.h"
#include "parse/timesig.h"

namespace hydra {

bool is_timing_refusal(std::string_view what) {
    for (std::string_view prefix :
         {kResolutionRefusalPrefix, kTimeSignatureRefusalPrefix, kTempoRefusalPrefix})
        if (what.substr(0, prefix.size()) == prefix) return true;
    return false;
}

// The timing maps Hydra can measure time with: a positive resolution, every
// measure at least one tick long, every tempo a positive, finite BPM. Both
// parsers reach this through Song::build_timing, so no chart with a zero,
// negative or infinite measure length reaches the engine (D4, R7.6).
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes) {
    if (tick_resolution <= 0)
        throw ChartFileError(std::string(kResolutionRefusalPrefix) +
                             std::to_string(tick_resolution) + ", and it must be above 0");
    for (const auto& [tick, len] : tpm_changes)
        if (len <= 0)
            throw ChartFileError(std::string(kTimeSignatureRefusalPrefix) + std::to_string(tick) +
                                 " makes a measure " + std::to_string(len) + " ticks long");
    for (const auto& [tick, bpm] : bpm_changes) {
        // A .mid tempo of 0 microseconds per beat reads back as an infinite BPM
        // (D12): say so, rather than "not above 0 BPM".
        if (std::isinf(bpm) && bpm > 0.0)
            throw ChartFileError(std::string(kTempoRefusalPrefix) + std::to_string(tick) +
                                 " is infinite (0 microseconds per beat)");
        if (!std::isfinite(bpm) || bpm <= 0.0)
            throw ChartFileError(std::string(kTempoRefusalPrefix) + std::to_string(tick) +
                                 " is not above 0 BPM");
    }
}

// ---- shared helpers -----------------------------------------------------

namespace {

// The name inside a practice-section marker body, which both formats spell one
// of two ways: "section Verse 2B" (Clone Hero) or "prc_verse_2b" (Rock Band).
// Nothing else is a section.
bool section_name_of(const std::string& body, std::string* name) {
    if (body.rfind("section ", 0) == 0) {
        *name = body.substr(8);
        return true;
    }
    if (body.rfind("prc_", 0) == 0) {
        *name = body.substr(4);
        return true;
    }
    return false;
}

bool try_parse_int(const std::string& s, int64_t& out) {
    if (s.empty()) return false;
    try {
        size_t idx = 0;
        long long v = std::stoll(s, &idx);
        if (idx == s.size()) {
            out = static_cast<int64_t>(v);
            return true;
        }
    } catch (...) {
    }
    return false;
}

// The three text markers both formats read, matched by hand. Each one is
// exactly the whole-string regex it replaced (named beside it). The tests
// "... markers match the regexes they replaced" in test_song.cpp check that
// through both parsers, with every byte value in every position that matters.
//
// What the regex pieces meant, as std::regex (ECMAScript, char) reads them:
// `.` is any byte except '\n' and '\r'; `\d` is an ASCII digit; `\[?` and
// `\]?` are one optional bracket at each end. No marker body starts with '['
// or ends with ']', so peeling one bracket off each end is never ambiguous.
bool regex_dot(char c) { return c != '\n' && c != '\r'; }
bool regex_digit(char c) { return c >= '0' && c <= '9'; }

std::string_view peel_brackets(std::string_view s) {
    if (!s.empty() && s.front() == '[') s.remove_prefix(1);
    if (!s.empty() && s.back() == ']') s.remove_suffix(1);
    return s;
}

// `mix.N.drums`: the 11-byte head both disco markers share, where N is the
// parsed difficulty's digit (difficulty_chart_codes). A marker naming another
// difficulty is not a marker here: Clone Hero applies each marker only to the
// difficulty it names, in both formats (D19; 0x215C750, 0x213D076, 0x2155050).
bool disco_head(std::string_view s, char mix_digit) {
    return s.size() >= 11 && s.substr(0, 3) == "mix" && regex_dot(s[3]) &&
           s[4] == mix_digit && regex_dot(s[5]) && s.substr(6, 5) == "drums";
}

// \[?ENABLE_CHART_DYNAMICS\]?
bool is_dynamics_marker(std::string_view s) {
    return peel_brackets(s) == "ENABLE_CHART_DYNAMICS";
}

// \[?mix.N.drums\d?d\]?, where N is `mix_digit`
bool is_disco_on_marker(std::string_view s, char mix_digit) {
    s = peel_brackets(s);
    if (!disco_head(s, mix_digit)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest == "d";
}

// \[?mix.N.drums\d?(dnoflip)?\]?, where N is `mix_digit`. "dnoflip" reads as
// flip off. Clone Hero turns flip on for it (its classifier at 0x215CD50 looks
// only at the 7th character); Hydra keeps it off on purpose (D19).
bool is_disco_off_marker(std::string_view s, char mix_digit) {
    s = peel_brackets(s);
    if (!disco_head(s, mix_digit)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest.empty() || rest == "dnoflip";
}

// Activation-fill placement heuristic, shared by both parsers: true when the
// fill that ended at `fill_end_tick` lands on the chord being emitted at
// `tick` (so its op must run after the chord emit), false when it belongs to
// an earlier chord (run it before). "Lands on" means the next chord is within
// a 1/32-of-a-beat slop of the fill end and no closer to the previous chord.
bool fill_lands_on_chord(const Song& song, int64_t fill_end_tick, int64_t tick,
                         double slop_beats) {
    std::optional<int64_t> prevchord_dist;
    if (!song.sequence.empty())
        prevchord_dist = fill_end_tick - song.sequence.back().timecode.ticks();
    int64_t nextchord_dist = tick - fill_end_tick;
    return nextchord_dist <= static_cast<int64_t>(song.tick_resolution() * slop_beats) &&
           (!prevchord_dist.has_value() || nextchord_dist <= *prevchord_dist);
}

// The parser handlers MIDI and .chart share. Each parser decides when to
// call them (its own event phases); what they do to the Song lives here once.

// A time signature: ticks per measure = resolution * 4 * num / den. The
// signature itself is kept too, for display. A numerator of 0 names no meter,
// so the line is ignored and the previous meter stays, in both formats
// (finding 100). A bottom number the readers couldn't hold (0, from
// timesig_denominator) refuses the chart here, where the tick is known.
// Anything else that makes a measure last no time is refused by
// check_timing_maps.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator) {
    if (numerator == 0) return;
    if (denominator <= 0)
        throw ChartFileError(std::string(kTimeSignatureRefusalPrefix) + std::to_string(tick) +
                             " has a bottom number that is out of range");
    song.tpm_changes[tick] = song.tick_resolution() * static_cast<int64_t>(numerator) * 4 /
                             static_cast<int64_t>(denominator);
    song.timesig_changes[tick] = {numerator, denominator};
}

// A fill ending: the last chord becomes an activation chord whose fill began
// at `starttick`.
void apply_fill_end(Song& song, int64_t starttick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    if (last.timecode.ticks() >= starttick)
        last.activation_length = last.timecode.ticks() - starttick;
}

// An SP phrase ending: the last chord closes the phrase that began at
// `starttick`, if it lies inside it.
void mark_sp_phrase_end(Song& song, int64_t starttick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    if (last.timecode.ticks() >= starttick) {
        last.flag_sp = true;
        last.sp_phrase_start = starttick;
    }
}

// Emit the buffered chord as a sequence timestamp, shared by both parsers.
// apply_flam is true only on the .mid path: MIDI charts carry a flam marker
// that converts the chord, while the .chart format has no such marker, so
// ChartParser always passes false (existing behavior, now explicit).
// A disco section swaps red and yellow only under Pro Drums. That rule lives
// here, once, for both parsers (finding 250): `pro` is the Pro Drums setting
// and `in_disco` says whether this difficulty's disco section is open.
void emit_chord_timestamp(Song& song, Chord& chord, int64_t tick, bool apply_flam,
                          bool pro, bool in_disco, bool solo) {
    if (apply_flam) chord.apply_flam_conversion();
    if (pro && in_disco) chord.apply_disco_flip();
    SongTimestamp ts;
    ts.chord = chord;
    ts.timecode = song.timecode(tick);
    ts.flag_solo = solo;
    song.sequence.push_back(std::move(ts));
}

}  // namespace

const char* difficulty_name(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Hard: return "Hard";
        case Difficulty::Medium: return "Medium";
        case Difficulty::Easy: return "Easy";
        default: return "Expert";
    }
}

std::optional<Difficulty> difficulty_from_name(std::string_view name) {
    for (Difficulty d : kAllDifficulties) {
        const std::string_view want = difficulty_name(d);
        if (name.size() != want.size()) continue;
        if (std::equal(name.begin(), name.end(), want.begin(), [](char a, char b) {
                return std::tolower(static_cast<unsigned char>(a)) ==
                       std::tolower(static_cast<unsigned char>(b));
            }))
            return d;
    }
    return std::nullopt;
}

std::string no_notes_message(Difficulty difficulty, bool prodrums) {
    return std::string("No ") + difficulty_name(difficulty) +
           (prodrums ? " Pro Drums" : " Drums") + " notes in this chart.";
}

std::string title_or_unknown(std::string title) {
    // The placeholder the metadata readers used before kUnknownTitle.
    static constexpr const char* kOldPlaceholder = "<unknown title>";
    if (title.empty() || title == kOldPlaceholder) return kUnknownTitle;
    return title;
}

// ---- Song::sp_phrase_count ----------------------------------------------

int Song::sp_phrase_count() const {
    int n = 0;
    for (const SongTimestamp& ts : sequence)
        if (ts.flag_sp) ++n;
    return n;
}

// ---- Song::check_activations -------------------------------------------

void Song::check_activations(const core::Rules& rules) {
    for (const SongTimestamp& ts : sequence)
        if (ts.has_activation()) return;  // chart already has fills.

    features.push_back("Auto-Generated Fills");

    struct Cell {
        int64_t tick;                 // the downbeat tick
        std::optional<size_t> best;   // index into sequence of the closest chord
        int64_t bestdist = 0;
        int64_t pre_tpm = 0;
    };
    // std::map keeps measures sorted, which equals Python's dict insertion
    // order here: measures are only ever added at or above the current max.
    std::map<int64_t, Cell> measuremap;

    // SongIter: walk the sequence, advancing at most one tpm/bpm mark each step.
    std::vector<int64_t> tpm_keys, bpm_keys;
    for (const auto& kv : tpm_changes) tpm_keys.push_back(kv.first);
    for (const auto& kv : bpm_changes) bpm_keys.push_back(kv.first);
    size_t tpm_pos = 0, bpm_pos = 0;
    int64_t tpm = tpm_keys.empty() ? 0 : tpm_changes[tpm_keys[tpm_pos++]];
    if (!bpm_keys.empty()) bpm_pos++;  // bpm value itself is unused here

    Timecode downbeat_ref = start_time();

    for (size_t i = 0; i < sequence.size(); ++i) {
        SongTimestamp& ts = sequence[i];
        int64_t tick = ts.timecode.ticks();

        int64_t pre_tpm = tpm;
        if (tpm_pos < tpm_keys.size()) {
            if (tick == tpm_keys[tpm_pos]) {
                tpm = tpm_changes[tpm_keys[tpm_pos]];
                ++tpm_pos;
            } else if (tick > tpm_keys[tpm_pos]) {
                tpm = tpm_changes[tpm_keys[tpm_pos]];
                pre_tpm = tpm;
                ++tpm_pos;
            }
        }
        if (bpm_pos < bpm_keys.size() && tick >= bpm_keys[bpm_pos]) ++bpm_pos;

        int64_t m0 = ts.timecode.measure_beats_ticks()[0];
        for (int64_t measure : {m0 + 1, m0 + 2}) {
            auto it = measuremap.find(measure);
            if (it == measuremap.end()) {
                downbeat_ref = timing().plusmeasure(
                    downbeat_ref,
                    measure - (downbeat_ref.measure_beats_ticks()[0] + 1));
                Cell c;
                c.tick = downbeat_ref.ticks();
                it = measuremap.emplace(measure, c).first;
            }
            Cell& cell = it->second;
            int64_t dist = std::llabs(cell.tick - tick);
            if (!cell.best.has_value() || dist <= cell.bestdist) {
                cell.best = i;
                cell.bestdist = dist;
                cell.pre_tpm = pre_tpm;
            }
        }
    }

    const int64_t cooldown_measures = rules.fill_cooldown_measures;
    const int64_t max_distance =
        static_cast<int64_t>(tick_resolution_ * rules.fill_max_distance_beats);
    std::optional<int64_t> last_act_measure;
    for (auto& kv : measuremap) {
        int64_t measure = kv.first;
        Cell& cell = kv.second;
        if (last_act_measure.has_value() &&
            measure < *last_act_measure + cooldown_measures)
            continue;
        if (cell.best.has_value() && cell.bestdist <= max_distance) {
            sequence[*cell.best].activation_length =
                static_cast<int64_t>(cell.pre_tpm * rules.fill_length_measures);
            last_act_measure = measure;
        }
    }
}

// ---- MidiParser ---------------------------------------------------------

namespace {

enum class MPhase { None, Time, Pre, PreDelayed, Notes, Post, PostDelayed,
                    PreTimestamp };

// What a MIDI message does to the parser, decided once when the message is
// classified and carried out later in its phase. A plain tagged struct, so a
// tick's handlers cost no allocation (they used to be std::function closures).
enum class MAct : uint8_t {
    None, Note, FillStart, StoreFillEnd, ApplyFill, SpStart, SpEnd, Tom, Flam,
    Solo, Dynamics, Disco, Tempo, TimeSig,
};

struct MOp {
    MPhase phase = MPhase::None;
    MAct act = MAct::None;
    NoteColor color = NoteColor::Kick;                  // Note, Tom
    NoteDynamicType dyn = NoteDynamicType::Normal;      // Note
    NoteCymbalType cymbal = NoteCymbalType::Normal;     // Tom
    bool flag = false;     // Note: is2x; Flam/Solo/Disco: on
    int64_t tick = 0;      // FillStart/StoreFillEnd/SpStart/Tempo/TimeSig;
                           // ApplyFill: the fill's start tick
    uint32_t tempo = 0;    // Tempo
    int num = 0, den = 0;  // TimeSig

    bool runs() const { return act != MAct::None; }
};

MOp mop(MPhase phase, MAct act) {
    MOp op;
    op.phase = phase;
    op.act = act;
    return op;
}

MOp mop_note(NoteColor color, NoteDynamicType dyn, bool is2x) {
    MOp op = mop(MPhase::Notes, MAct::Note);
    op.color = color;
    op.dyn = dyn;
    op.flag = is2x;
    return op;
}

MOp mop_tick(MPhase phase, MAct act, int64_t tick) {
    MOp op = mop(phase, act);
    op.tick = tick;
    return op;
}

MOp mop_flag(MAct act, bool on) {
    MOp op = mop(MPhase::Pre, act);
    op.flag = on;
    return op;
}

MOp mop_tom(NoteColor color, NoteCymbalType cymbal) {
    MOp op = mop(MPhase::Pre, MAct::Tom);
    op.color = color;
    op.cymbal = cymbal;
    return op;
}

}  // namespace

// The one table of how each difficulty is spelled in a chart file, in
// Difficulty enum order. All four difficulties share the one "PART DRUMS"
// track; each owns a block of five pitches starting at its kick (kick, then
// the four pads), and its 2x kick sits one below the kick.
namespace {
constexpr DifficultyChartCodes kDifficultyChartCodes[] = {
    {96, 95, '3'},  // Expert
    {84, 83, '2'},  // Hard
    {72, 71, '1'},  // Medium
    {60, 59, '0'},  // Easy
};
static_assert(std::size(kDifficultyChartCodes) == std::size(kAllDifficulties));
}  // namespace

const DifficultyChartCodes& difficulty_chart_codes(Difficulty difficulty) {
    const auto i = static_cast<size_t>(difficulty);
    return kDifficultyChartCodes[i < std::size(kDifficultyChartCodes) ? i : 0];
}

namespace {

int difficulty_base_pitch(Difficulty difficulty) {
    return difficulty_chart_codes(difficulty).kick_pitch;
}

// `base` is the difficulty's kick pitch and `kick2x` its 2x kick pitch, both
// from difficulty_chart_codes. The five note pitches follow the kick, and the
// 2x kick sits one below it. Every other pitch here is a marker shared by all
// four difficulties. A pitch outside this set belongs to another difficulty
// (or to another instrument) and is dropped, so Expert's 95 is never read
// below Expert: Clone Hero reads each 2x kick only into its own difficulty
// (D20; 0x2155050 at 0x21555CD).
bool is_handled_note(int note, int base, int kick2x) {
    if (note >= base && note <= base + 4) return true;
    if (note == kick2x) return true;
    switch (note) {
        case 103:
        case 109: case 110: case 111: case 112:
        case 116:
        case 120:
            return true;
        default:
            return false;
    }
}

class MidiParser {
public:
    explicit MidiParser(const core::Rules& rules) : rules_(rules) {}
    Song parse(const MidiFile& mid, bool pro, bool bass2x, Difficulty difficulty);

private:
    const core::Rules& rules_;

    MOp optype(const Message& msg, int64_t tick);
    void push_timestamp(int64_t tick);
    void run(const MOp& op);
    void run_ops(const std::vector<MOp>& ops);

    // op_* handlers
    void op_enable_dynamics() { dynamics_enabled_ = true; }
    void op_disco(bool on) { flag_disco_ = on; }
    void op_tempo(int64_t tick, uint32_t miditempo) {
        song_->bpm_changes[tick] = 60000000.0 / static_cast<double>(miditempo);
    }
    void op_timesig(int64_t tick, int numerator, int denominator) {
        apply_timesig(*song_, tick, numerator, denominator);
    }
    void op_fillstart(int64_t tick) {
        fill_start_tick_ = tick;
        fill_end_tick_.reset();
    }
    void op_store_fillend(int64_t tick) { fill_end_tick_ = tick; }
    void op_apply_fill(int64_t starttick) { apply_fill_end(*song_, starttick); }
    void op_sp_start(int64_t tick) { sp_start_tick_ = tick; }
    void op_sp_end() {
        // A note-off with no phrase open (a stray 116 off) closes nothing.
        if (sp_start_tick_) mark_sp_phrase_end(*song_, *sp_start_tick_);
        sp_start_tick_.reset();
    }
    void op_tom(NoteColor color, NoteCymbalType cymbal) {
        flag_cymbals_[static_cast<int>(color) - 1] = cymbal;
    }
    void op_flam(bool enabled) { flag_flam_ = enabled; }
    void op_solo(bool on) { flag_solo_ = on; }
    void op_note(NoteColor color, NoteDynamicType dyn, bool is2x) {
        ChordNote& note = chord_.add_note(color);  // may throw ChartFileError
        note.dynamictype = dynamics_enabled_ ? dyn : NoteDynamicType::Normal;
        if (allows_cymbals(color) && mode_pro_)
            note.cymbaltype = flag_cymbals_[static_cast<int>(color) - 1];
        note.is2x = is2x;
    }

    Song* song_ = nullptr;
    bool mode_pro_ = false;
    bool mode_bass2x_ = false;
    int base_ = 0;          // set by parse() from difficulty_chart_codes
    int kick2x_pitch_ = 0;  // set by parse() from difficulty_chart_codes
    // The parsed difficulty's disco digit: only `[mix N drums...]` markers
    // with this N open or close a disco section here. parse() sets it from
    // difficulty_chart_codes.
    char mix_digit_ = 0;

    Chord chord_;
    std::vector<const Message*> msg_buffer_;
    // One bucket per phase that push_timestamp runs, reused tick to tick so
    // their storage is allocated once per parse, not once per tick.
    std::vector<MOp> pre_, pre_delayed_, notes_, pre_timestamp_, post_,
        post_delayed_;
    bool flag_solo_ = false;
    std::array<NoteCymbalType, 5> flag_cymbals_{};
    bool flag_flam_ = false;
    bool flag_disco_ = false;
    std::optional<int64_t> fill_start_tick_;
    std::optional<int64_t> fill_end_tick_;
    bool dynamics_enabled_ = false;
    std::optional<int64_t> sp_start_tick_;
};

MOp MidiParser::optype(const Message& msg, int64_t tick) {
    using MType = Message::Type;
    const bool is_channel = (msg.type == MType::NoteOn || msg.type == MType::NoteOff);

    if (is_channel) {
        int note = msg.note;
        if (!is_handled_note(note, base_, kick2x_pitch_)) return {};

        int velocity = msg.velocity;
        bool is_noteon = (msg.type == MType::NoteOn && velocity > 0);
        bool is_noteoff =
            (msg.type == MType::NoteOff || (msg.type == MType::NoteOn && velocity == 0));

        if (is_noteoff && note < 103) return {};

        if (is_noteon) {
            // The difficulty's own five pitches come first: base is the kick,
            // the next four are Red/Yellow/Blue/Green.
            // Clone Hero reads the kick's velocity exactly as it reads a
            // pad's: 127 is an accent, 1 is a ghost, and both score double.
            const NoteDynamicType vel_dyn =
                velocity == 127   ? NoteDynamicType::Accent
                : velocity == 1   ? NoteDynamicType::Ghost
                                  : NoteDynamicType::Normal;
            if (note == base_) return mop_note(NoteColor::Kick, vel_dyn, false);
            if (note > base_ && note <= base_ + 4) {
                // base+1 -> Red(2), as 97 -> Red(2) on Expert.
                NoteColor color = static_cast<NoteColor>(note - base_ + 1);
                return mop_note(color, vel_dyn, false);
            }
            // The difficulty's own 2x kick, read only with 2x Bass on.
            if (note == kick2x_pitch_) {
                if (mode_bass2x_) return mop_note(NoteColor::Kick, vel_dyn, true);
                return {};
            }
            switch (note) {
                case 120:
                    return mop_tick(MPhase::PostDelayed, MAct::FillStart, tick);
                case 116:
                    return mop_tick(sp_start_tick_.has_value() ? MPhase::PreDelayed
                                                               : MPhase::Pre,
                                    MAct::SpStart, tick);
                case 112: return mop_tom(NoteColor::Green, NoteCymbalType::Normal);
                case 111: return mop_tom(NoteColor::Blue, NoteCymbalType::Normal);
                case 110: return mop_tom(NoteColor::Yellow, NoteCymbalType::Normal);
                case 109: return mop_flag(MAct::Flam, true);
                case 103: return mop_flag(MAct::Solo, true);
                default:
                    return {};
            }
        }

        if (is_noteoff) {
            switch (note) {
                case 120:
                    return mop_tick(MPhase::Pre, MAct::StoreFillEnd, tick);
                case 116:
                    return mop(sp_start_tick_.has_value() ? MPhase::Pre
                                                          : MPhase::PreDelayed,
                               MAct::SpEnd);
                case 112: return mop_tom(NoteColor::Green, NoteCymbalType::Cymbal);
                case 111: return mop_tom(NoteColor::Blue, NoteCymbalType::Cymbal);
                case 110: return mop_tom(NoteColor::Yellow, NoteCymbalType::Cymbal);
                case 109: return mop_flag(MAct::Flam, false);
                case 103:
                    // A MIDI solo marker covers ticks up to its note-off, not
                    // including it: end the solo before this tick's notes.
                    // Pinned by ".mid: the note on the solo marker's note-off
                    // tick is outside the solo".
                    return mop_flag(MAct::Solo, false);
                default:
                    return {};
            }
        }
        return {};
    }

    // Meta family. Text-attribute metas carry `str`; name-attribute metas do
    // not match Python's `MetaMessage(text=...)` patterns.
    if (msg.str_attr == Message::StrAttr::Text) {
        const std::string& t = msg.str;
        if (is_dynamics_marker(t)) return mop(MPhase::Pre, MAct::Dynamics);
        if (is_disco_on_marker(t, mix_digit_)) return mop_flag(MAct::Disco, true);
        if (is_disco_off_marker(t, mix_digit_)) return mop_flag(MAct::Disco, false);
    }
    if (msg.type == MType::SetTempo) {
        MOp op = mop_tick(MPhase::Time, MAct::Tempo, tick);
        op.tempo = msg.tempo;
        return op;
    }
    if (msg.type == MType::TimeSignature) {
        MOp op = mop_tick(MPhase::Time, MAct::TimeSig, tick);
        op.num = msg.numerator;
        op.den = msg.denominator;
        return op;
    }
    return {};
}

void MidiParser::run(const MOp& op) {
    switch (op.act) {
        case MAct::None: break;
        case MAct::Note: op_note(op.color, op.dyn, op.flag); break;
        case MAct::FillStart: op_fillstart(op.tick); break;
        case MAct::StoreFillEnd: op_store_fillend(op.tick); break;
        case MAct::ApplyFill: op_apply_fill(op.tick); break;
        case MAct::SpStart: op_sp_start(op.tick); break;
        case MAct::SpEnd: op_sp_end(); break;
        case MAct::Tom: op_tom(op.color, op.cymbal); break;
        case MAct::Flam: op_flam(op.flag); break;
        case MAct::Solo: op_solo(op.flag); break;
        case MAct::Dynamics: op_enable_dynamics(); break;
        case MAct::Disco: op_disco(op.flag); break;
        case MAct::Tempo: op_tempo(op.tick, op.tempo); break;
        case MAct::TimeSig: op_timesig(op.tick, op.num, op.den); break;
    }
}

void MidiParser::run_ops(const std::vector<MOp>& ops) {
    for (const MOp& op : ops) {
        try {
            run(op);
        } catch (const ChartFileError&) {
        }
    }
}

void MidiParser::push_timestamp(int64_t tick) {
    chord_ = Chord();

    pre_.clear();
    pre_delayed_.clear();
    notes_.clear();
    pre_timestamp_.clear();
    post_.clear();
    post_delayed_.clear();
    for (const Message* msg : msg_buffer_) {
        MOp op = optype(*msg, tick);
        if (!op.runs()) continue;
        switch (op.phase) {
            case MPhase::Pre: pre_.push_back(op); break;
            case MPhase::PreDelayed: pre_delayed_.push_back(op); break;
            case MPhase::Notes: notes_.push_back(op); break;
            case MPhase::Post: post_.push_back(op); break;
            case MPhase::PostDelayed: post_delayed_.push_back(op); break;
            case MPhase::PreTimestamp: pre_timestamp_.push_back(op); break;
            default: break;  // Time / None: not run from push_timestamp.
        }
    }

    run_ops(pre_);
    run_ops(pre_delayed_);
    run_ops(notes_);

    // Activation fill placement.
    if (chord_.count() && fill_end_tick_.has_value() &&
        tick >= *fill_end_tick_) {
        MOp fill_op = mop_tick(MPhase::None, MAct::ApplyFill, *fill_start_tick_);

        if (fill_lands_on_chord(*song_, *fill_end_tick_, tick, rules_.fill_land_slop_beats))
            post_.push_back(fill_op);
        else
            pre_timestamp_.push_back(fill_op);
        fill_start_tick_.reset();
        fill_end_tick_.reset();
    }

    run_ops(pre_timestamp_);

    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, flag_flam_, mode_pro_, flag_disco_,
                             flag_solo_);

    run_ops(post_);
    run_ops(post_delayed_);

    msg_buffer_.clear();
}

Song MidiParser::parse(const MidiFile& mid, bool pro, bool bass2x,
                       Difficulty difficulty) {
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;
    base_ = difficulty_base_pitch(difficulty);
    kick2x_pitch_ = difficulty_chart_codes(difficulty).kick2x_pitch;
    mix_digit_ = difficulty_chart_codes(difficulty).mix_digit;

    Song song(mid.ticks_per_beat);
    song_ = &song;

    // Pass 1: tempo/time-signature marks from the first track.
    int64_t elapsed = 0;
    for (const Message& msg : mid.tracks[0].messages) {
        elapsed += msg.time;
        MOp op = optype(msg, elapsed);
        if (op.phase == MPhase::Time && op.runs()) run(op);
    }
    song.build_timing();

    // Pass 2: the drum track.
    for (const MidiTrack& track : mid.tracks) {
        if (track.name != "PART DRUMS") continue;
        elapsed = 0;
        msg_buffer_.clear();
        flag_solo_ = false;
        flag_disco_ = false;
        flag_cymbals_[static_cast<int>(NoteColor::Green) - 1] =
            NoteCymbalType::Cymbal;
        flag_cymbals_[static_cast<int>(NoteColor::Blue) - 1] =
            NoteCymbalType::Cymbal;
        flag_cymbals_[static_cast<int>(NoteColor::Yellow) - 1] =
            NoteCymbalType::Cymbal;
        dynamics_enabled_ = false;
        for (const Message& msg : track.messages) {
            if (msg.time != 0) {
                push_timestamp(elapsed);
                elapsed += msg.time;
            }
            msg_buffer_.push_back(&msg);
        }
        push_timestamp(elapsed);
        song.dynamics_enabled = dynamics_enabled_;
        break;
    }

    // Pass 3: practice sections, which live on their own track as bracketed
    // text metas.
    for (const MidiTrack& track : mid.tracks) {
        if (track.name != "EVENTS") continue;
        elapsed = 0;
        for (const Message& msg : track.messages) {
            elapsed += msg.time;
            if (msg.str_attr != Message::StrAttr::Text) continue;
            const std::string& s = msg.str;
            if (s.size() < 2 || s.front() != '[' || s.back() != ']') continue;
            std::string name;
            if (section_name_of(s.substr(1, s.size() - 2), &name))
                song.practice_sections.push_back({elapsed, name});
        }
    }

    song.check_activations(rules_);
    return song;
}

}  // namespace

// ---- ChartParser --------------------------------------------------------

namespace {

struct ChartDataEntry {
    std::optional<int64_t> key_tick;
    std::optional<std::string> key_name;

    std::optional<int64_t> property_int;
    std::optional<std::string> property_str;

    std::optional<int> ts_numerator;
    std::optional<int> ts_denominator;
    std::optional<double> tempo_bpm;

    bool solo_start = false;
    bool solo_end = false;
    bool discoflip_enable = false;
    bool discoflip_disable = false;

    // A generic text event's payload: the value with its leading "E" and any
    // surrounding quotes taken off. Only the [Events] walk reads it.
    std::optional<std::string> event_text;

    std::optional<int> notevalue;
    std::optional<int64_t> notelength;
    std::optional<int> phrasevalue;
    std::optional<int64_t> phraselength;

    // Both sides arrive already trimmed. `mix_digit` is the parsed
    // difficulty's disco digit: a disco marker naming another difficulty is
    // read as a plain text event, which no drum section uses.
    ChartDataEntry(std::string_view keystr, std::string_view valuestr, char mix_digit);

    bool is_tick_data() const { return key_tick.has_value(); }
};

// The whitespace-separated words of an event value, as views into it. Only
// the first four are kept (no event reads further); `count` is the full count,
// which the event forms check exactly.
struct ChartWords {
    std::array<std::string_view, 4> w{};
    size_t count = 0;
};

ChartWords split_ws_view(std::string_view s) {
    ChartWords out;
    size_t i = 0, n = s.size();
    while (i < n) {
        while (i < n && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        size_t start = i;
        while (i < n && !std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i > start) {
            if (out.count < out.w.size()) out.w[out.count] = s.substr(start, i - start);
            ++out.count;
        }
    }
    return out;
}

// std::stoi / std::stoll on one word, with their exact acceptance rules
// (leading digits read, trailing junk ignored, throws on no digits).
int word_stoi(std::string_view w) { return std::stoi(std::string(w)); }
long long word_stoll(std::string_view w) { return std::stoll(std::string(w)); }

ChartDataEntry::ChartDataEntry(std::string_view keystr, std::string_view valuestr,
                               char mix_digit) {
    int64_t k;
    if (try_parse_int(std::string(keystr), k))
        key_tick = k;
    else
        key_name = std::string(keystr);

    if (!key_tick.has_value()) {
        int64_t iv;
        std::string value(valuestr);
        if (try_parse_int(value, iv))
            property_int = iv;
        else
            property_str = std::move(value);
        return;
    }

    const ChartWords t = split_ws_view(valuestr);
    if (t.count == 0) return;
    const std::string_view t0 = t.w[0];

    if (t0 == "TS" && t.count == 2) {
        ts_numerator = word_stoi(t.w[1]);
        ts_denominator = 4;
    } else if (t0 == "TS" && t.count == 3) {
        ts_numerator = word_stoi(t.w[1]);
        ts_denominator = timesig_denominator(word_stoi(t.w[2]));
    } else if (t0 == "B" && t.count == 2) {
        tempo_bpm = static_cast<double>(word_stoll(t.w[1])) / 1000.0;
    } else if (t0 == "E" && t.count == 2 && t.w[1] == "solo") {
        solo_start = true;
    } else if (t0 == "E" && t.count == 2 && t.w[1] == "soloend") {
        solo_end = true;
    } else if (t0 == "E" && t.count == 2 && is_disco_off_marker(t.w[1], mix_digit)) {
        discoflip_disable = true;
    } else if (t0 == "E" && t.count == 2 && is_disco_on_marker(t.w[1], mix_digit)) {
        discoflip_enable = true;
    } else if (t0 == "E") {
        // Generic text event: no gameplay effect, but [Events] carries the
        // practice-section markers here.
        std::string_view rest = trim_view(valuestr.substr(1));
        if (rest.size() >= 2 && rest.front() == '"' && rest.back() == '"')
            rest = rest.substr(1, rest.size() - 2);
        event_text = std::string(rest);
    } else if (t0 == "N" && t.count == 3) {
        notevalue = word_stoi(t.w[1]);
        notelength = word_stoll(t.w[2]);
    } else if (t0 == "S" && t.count == 3) {
        phrasevalue = word_stoi(t.w[1]);
        phraselength = word_stoll(t.w[2]);
    }
}

struct ChartSection {
    std::string name;
    std::vector<int64_t> tick_order;
    std::unordered_map<int64_t, std::vector<ChartDataEntry>> tick_data;
    std::unordered_map<std::string, std::vector<ChartDataEntry>> prop_data;

    // Takes the entry by value and moves it into place: no copy per entry.
    void add(ChartDataEntry e) {
        if (e.is_tick_data()) {
            const int64_t key = *e.key_tick;
            auto [it, inserted] = tick_data.try_emplace(key);
            if (inserted) tick_order.push_back(key);
            it->second.push_back(std::move(e));
        } else {
            std::vector<ChartDataEntry>& v = prop_data[*e.key_name];
            v.push_back(std::move(e));
        }
    }
};

enum class CPhase { None, Time, Notes, NoteMods, Pre, Post, PostDelayed };

// What a .chart event does, decided when it is classified and carried out in
// its phase: a plain tagged struct, so a tick's handlers cost no allocation.
enum class CAct : uint8_t {
    None, Disco, Tempo, TimeSig, Solo, Note, TwoX, Accent, Ghost, Cymbal,
    SpStart, SpEnd, FillStart, FillEnd,
};

struct COp {
    CPhase phase = CPhase::None;
    CAct act = CAct::None;
    NoteColor color = NoteColor::Kick;  // Note, Accent, Ghost, Cymbal
    bool flag = false;                  // Disco, Solo: on
    int64_t a = 0;   // Tempo/TimeSig/SpStart/FillStart: tick; SpEnd/FillEnd: start
    int64_t b = 0;   // SpStart/FillStart: end tick
    double bpm = 0;  // Tempo
    int num = 0, den = 0;  // TimeSig

    bool runs() const { return act != CAct::None; }
};

COp cop(CPhase phase, CAct act) {
    COp op;
    op.phase = phase;
    op.act = act;
    return op;
}

COp cop_color(CPhase phase, CAct act, NoteColor color) {
    COp op = cop(phase, act);
    op.color = color;
    return op;
}

COp cop_flag(CPhase phase, CAct act, bool on) {
    COp op = cop(phase, act);
    op.flag = on;
    return op;
}

COp cop_span(CPhase phase, CAct act, int64_t a, int64_t b) {
    COp op = cop(phase, act);
    op.a = a;
    op.b = b;
    return op;
}

class ChartParser {
public:
    explicit ChartParser(const core::Rules& rules) : rules_(rules) {}
    Song parse(const std::vector<uint8_t>& data, bool pro, bool bass2x,
               Difficulty difficulty);

private:
    const core::Rules& rules_;

    void load_sections(const std::vector<uint8_t>& data, char mix_digit);
    COp optype(const ChartDataEntry& e, int64_t tick);
    void run(const COp& op);
    void push_timestamp(int64_t tick, const std::vector<ChartDataEntry>& entries);

    void op_disco(bool on) { flag_disco_ = on; }
    void op_tempo(int64_t tick, double bpm) { song_->bpm_changes[tick] = bpm; }
    void op_timesig(int64_t tick, int numerator, int denominator) {
        apply_timesig(*song_, tick, numerator, denominator);
    }
    void op_fillstart(int64_t start, int64_t end) {
        fill_start_tick_ = start;
        fill_end_tick_ = end;
    }
    void op_fillend(int64_t starttick) { apply_fill_end(*song_, starttick); }
    void op_sp_start(int64_t start, int64_t end) {
        sp_start_tick_ = start;
        sp_end_tick_ = end;
    }
    void op_sp_end(int64_t starttick) {
        mark_sp_phrase_end(*song_, starttick);
        sp_end_tick_.reset();
    }
    void op_solo(bool on) { flag_solo_ = on; }
    void op_note(NoteColor color) { chord_.add_note(color); }
    void op_2x() { chord_.add_2x(); }
    void op_accent(NoteColor color) { chord_.apply_accent(color); }
    void op_ghost(NoteColor color) { chord_.apply_ghost(color); }
    void op_cymbal(NoteColor color) { chord_.apply_cymbal(color); }

    Song* song_ = nullptr;
    bool mode_pro_ = false;
    bool mode_bass2x_ = false;

    std::unordered_map<std::string, ChartSection> sections_;

    Chord chord_;
    std::vector<COp> ops_;  // one tick's handlers, reused tick to tick
    bool flag_solo_ = false;
    bool flag_disco_ = false;
    std::optional<int64_t> sp_start_tick_;
    std::optional<int64_t> sp_end_tick_;
    std::optional<int64_t> fill_start_tick_;
    std::optional<int64_t> fill_end_tick_;
};

// The first match of the regex `\[.*\]` searched in `line`, as std::regex finds
// it: the leftmost '[' that some later ']' closes with only regex_dot bytes
// between, closed by the last such ']' (`.*` is greedy). Returns false when
// there is none.
bool find_section_header(std::string_view line, std::string_view* bracket) {
    size_t i = line.find('[');
    while (i != std::string_view::npos) {
        size_t close = std::string_view::npos;
        size_t k = i + 1;
        for (; k < line.size() && regex_dot(line[k]); ++k)
            if (line[k] == ']') close = k;
        if (close != std::string_view::npos) {
            *bracket = line.substr(i, close - i + 1);
            return true;
        }
        // Every '[' before k ends its run at k too, with no ']' in it.
        i = line.find('[', k);
    }
    return false;
}

void ChartParser::load_sections(const std::vector<uint8_t>& data, char mix_digit) {
    // Walk the file in place, one line per '\n' (a trailing '\r' is removed by
    // the trim). Every line ended by a '\n' counts, empty or not; the last
    // unterminated piece counts only when it is non-empty.
    const std::string_view text(reinterpret_cast<const char*>(data.data()), data.size());

    std::optional<ChartSection> wip;
    size_t pos = 0;
    while (pos < text.size()) {
        const size_t nl = text.find('\n', pos);
        const size_t stop = nl == std::string_view::npos ? text.size() : nl;
        const std::string_view line = trim_view(text.substr(pos, stop - pos));
        pos = nl == std::string_view::npos ? text.size() : nl + 1;

        if (wip.has_value()) {
            if (line == "{") {
                // block open
            } else if (line == "}") {
                sections_[wip->name] = std::move(*wip);
                wip.reset();
            } else {
                // The key is what precedes the first '='; the value is what
                // lies between the first '=' and the next one (or line end).
                const size_t eq = line.find('=');
                std::string_view lhs = line.substr(0, eq), rhs;
                if (eq != std::string_view::npos) {
                    const size_t eq2 = line.find('=', eq + 1);
                    rhs = line.substr(eq + 1, eq2 == std::string_view::npos
                                                  ? std::string_view::npos
                                                  : eq2 - eq - 1);
                }
                wip->add(ChartDataEntry(trim_view(lhs), trim_view(rhs), mix_digit));
            }
        } else {
            std::string_view bracket;
            if (!find_section_header(line, &bracket))
                throw ChartFileError("expected a [section] header");
            ChartSection s;
            s.name = std::string(bracket.substr(1, bracket.size() - 2));
            wip = std::move(s);
        }
    }
}

COp ChartParser::optype(const ChartDataEntry& e, int64_t tick) {
    if (e.discoflip_enable) return cop_flag(CPhase::Pre, CAct::Disco, true);
    if (e.discoflip_disable) return cop_flag(CPhase::Pre, CAct::Disco, false);
    if (e.tempo_bpm.has_value()) {
        COp op = cop_span(CPhase::Time, CAct::Tempo, tick, 0);
        op.bpm = *e.tempo_bpm;
        return op;
    }
    if (e.ts_numerator.has_value()) {
        COp op = cop_span(CPhase::Time, CAct::TimeSig, tick, 0);
        op.num = *e.ts_numerator;
        op.den = *e.ts_denominator;
        return op;
    }
    if (e.solo_start) return cop_flag(CPhase::Pre, CAct::Solo, true);
    // A .chart `E soloend` sits on the solo's last note: end the solo after
    // this tick's notes. Pinned by ".chart: the note on the solo end tick is
    // in the solo".
    if (e.solo_end) return cop_flag(CPhase::Post, CAct::Solo, false);

    if (e.notevalue.has_value()) {
        constexpr CPhase N = CPhase::Notes, M = CPhase::NoteMods;
        switch (*e.notevalue) {
            case 0: return cop_color(N, CAct::Note, NoteColor::Kick);
            case 1: return cop_color(N, CAct::Note, NoteColor::Red);
            case 2: return cop_color(N, CAct::Note, NoteColor::Yellow);
            case 3: return cop_color(N, CAct::Note, NoteColor::Blue);
            case 4: return cop_color(N, CAct::Note, NoteColor::Green);
            case 32:
                if (mode_bass2x_) return cop(N, CAct::TwoX);
                return {};
            case 34: return cop_color(M, CAct::Accent, NoteColor::Red);
            case 35: return cop_color(M, CAct::Accent, NoteColor::Yellow);
            case 36: return cop_color(M, CAct::Accent, NoteColor::Blue);
            case 37: return cop_color(M, CAct::Accent, NoteColor::Green);
            case 40: return cop_color(M, CAct::Ghost, NoteColor::Red);
            case 41: return cop_color(M, CAct::Ghost, NoteColor::Yellow);
            case 42: return cop_color(M, CAct::Ghost, NoteColor::Blue);
            case 43: return cop_color(M, CAct::Ghost, NoteColor::Green);
            case 66:
                if (mode_pro_) return cop_color(M, CAct::Cymbal, NoteColor::Yellow);
                return {};
            case 67:
                if (mode_pro_) return cop_color(M, CAct::Cymbal, NoteColor::Blue);
                return {};
            case 68:
                if (mode_pro_) return cop_color(M, CAct::Cymbal, NoteColor::Green);
                return {};
            default: return {};
        }
    }

    if (e.phrasevalue.has_value()) {
        if (*e.phrasevalue == 2)
            return cop_span(CPhase::Pre, CAct::SpStart, tick, tick + *e.phraselength);
        if (*e.phrasevalue == 64)
            return cop_span(CPhase::PostDelayed, CAct::FillStart, tick,
                            tick + *e.phraselength);
    }
    return {};
}

void ChartParser::run(const COp& op) {
    switch (op.act) {
        case CAct::None: break;
        case CAct::Disco: op_disco(op.flag); break;
        case CAct::Tempo: op_tempo(op.a, op.bpm); break;
        case CAct::TimeSig: op_timesig(op.a, op.num, op.den); break;
        case CAct::Solo: op_solo(op.flag); break;
        case CAct::Note: op_note(op.color); break;
        case CAct::TwoX: op_2x(); break;
        case CAct::Accent: op_accent(op.color); break;
        case CAct::Ghost: op_ghost(op.color); break;
        case CAct::Cymbal: op_cymbal(op.color); break;
        case CAct::SpStart: op_sp_start(op.a, op.b); break;
        case CAct::SpEnd: op_sp_end(op.a); break;
        case CAct::FillStart: op_fillstart(op.a, op.b); break;
        case CAct::FillEnd: op_fillend(op.a); break;
    }
}

void ChartParser::push_timestamp(int64_t tick,
                                 const std::vector<ChartDataEntry>& entries) {
    chord_ = Chord();

    std::vector<COp>& ops = ops_;
    ops.clear();
    for (const ChartDataEntry& e : entries) {
        COp op = optype(e, tick);
        if (op.runs()) ops.push_back(op);
    }

    auto run_phase = [this, &ops](CPhase phase) {
        for (const COp& op : ops) {
            if (op.phase != phase) continue;
            try {
                run(op);
            } catch (const ChartFileError&) {
            }
        }
    };

    run_phase(CPhase::Notes);
    run_phase(CPhase::NoteMods);

    // Phrase end: SP.
    if (sp_end_tick_.has_value() && tick >= *sp_end_tick_) {
        const int64_t start = sp_start_tick_.value_or(0);
        ops.insert(ops.begin(), cop_span(CPhase::Pre, CAct::SpEnd, start, 0));
    }

    // Phrase end: activation fill.
    if (chord_.count() && fill_end_tick_.has_value() &&
        tick >= *fill_end_tick_) {
        CPhase order = fill_lands_on_chord(*song_, *fill_end_tick_, tick, rules_.fill_land_slop_beats)
                           ? CPhase::Post
                           : CPhase::Pre;
        ops.push_back(cop_span(order, CAct::FillEnd, *fill_start_tick_, 0));
        fill_start_tick_.reset();
        fill_end_tick_.reset();
    }

    run_phase(CPhase::Pre);

    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, /*apply_flam=*/false, mode_pro_,
                             flag_disco_, flag_solo_);

    run_phase(CPhase::Post);
    run_phase(CPhase::PostDelayed);
}

Song ChartParser::parse(const std::vector<uint8_t>& data, bool pro,
                        bool bass2x, Difficulty difficulty) {
    // A disco marker counts only in the difficulty it names, so the reader
    // needs this difficulty's digit before it classifies any line.
    load_sections(data, difficulty_chart_codes(difficulty).mix_digit);
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;

    const ChartSection& song_sec = sections_.at("Song");
    const ChartDataEntry& res_entry = song_sec.prop_data.at("Resolution").at(0);
    int64_t tick_resolution;
    if (res_entry.property_int.has_value())
        tick_resolution = *res_entry.property_int;
    else
        tick_resolution = std::stoll(*res_entry.property_str);

    Song song(tick_resolution);
    song_ = &song;

    // Offset is a decimal number of seconds. ChartDataEntry keeps a
    // non-integer value in property_str, so parse that.
    if (auto it = song_sec.prop_data.find("Offset");
        it != song_sec.prop_data.end() && !it->second.empty()) {
        const ChartDataEntry& e = it->second.at(0);
        if (e.property_int) {
            song.chart_offset_s = static_cast<double>(*e.property_int);
        } else if (e.property_str) {
            try {
                song.chart_offset_s = std::stod(*e.property_str);
            } catch (const std::exception&) {
                // An unreadable Offset is treated as absent, like CH's default 0.
            }
        }
    }

    // Map tempo and time signatures from the sync track.
    auto sync_it = sections_.find("SyncTrack");
    if (sync_it != sections_.end()) {
        const ChartSection& sync = sync_it->second;
        for (int64_t tk : sync.tick_order) {
            for (const ChartDataEntry& e : sync.tick_data.at(tk)) {
                COp op = optype(e, tk);
                if (op.phase == CPhase::Time && op.runs()) run(op);
            }
        }
    }
    song.build_timing();

    flag_solo_ = false;
    flag_disco_ = false;

    // Each difficulty is its own section ("ExpertDrums", "HardDrums", ...);
    // everything inside one — notes, dynamics, cymbals, SP, fills, solos —
    // follows for free. A chart missing the section parses as an empty song.
    auto ed_it = sections_.find(std::string(difficulty_name(difficulty)) + "Drums");
    if (ed_it != sections_.end()) {
        const ChartSection& ed = ed_it->second;
        for (int64_t tk : ed.tick_order)
            push_timestamp(tk, ed.tick_data.at(tk));
    }

    // Practice sections. tick_order follows the file, which is not required to
    // be sorted, so sort once at the end.
    auto ev_it = sections_.find("Events");
    if (ev_it != sections_.end()) {
        const ChartSection& ev = ev_it->second;
        for (int64_t tk : ev.tick_order) {
            for (const ChartDataEntry& e : ev.tick_data.at(tk)) {
                if (!e.event_text.has_value()) continue;
                std::string name;
                if (section_name_of(*e.event_text, &name))
                    song.practice_sections.push_back({tk, name});
            }
        }
        std::stable_sort(song.practice_sections.begin(),
                         song.practice_sections.end(),
                         [](const SongSection& a, const SongSection& b) {
                             return a.tick < b.tick;
                         });
    }

    // .chart ghosts and accents are explicit per-note flags (N 34-37 accent,
    // N 40-43 ghost), applied unconditionally, so a .chart has no opt-in
    // marker like MIDI's [ENABLE_CHART_DYNAMICS]. Dynamics are always on; the
    // Dynamics tab reads this flag.
    song.dynamics_enabled = true;
    song.check_activations(rules_);
    return song;
}

}  // namespace

// ---- public loaders -----------------------------------------------------

Song load_songbytes_mid(const std::vector<uint8_t>& data, bool pro,
                        bool bass2x, Difficulty difficulty, const core::Rules& rules) {
    MidiFile mid(data);
    return MidiParser(rules).parse(mid, pro, bass2x, difficulty);
}

Song load_songbytes_chart(const std::vector<uint8_t>& data, bool pro,
                          bool bass2x, Difficulty difficulty, const core::Rules& rules) {
    return ChartParser(rules).parse(data, pro, bass2x, difficulty);
}

Song load_songpath_mid(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules) {
    MidiFile mid = MidiFile::from_file(path);
    return MidiParser(rules).parse(mid, pro, bass2x, difficulty);
}

Song load_songpath_chart(const std::string& path, bool pro, bool bass2x,
                         Difficulty difficulty, const core::Rules& rules) {
    std::vector<uint8_t> data = read_file_bytes(path);
    return ChartParser(rules).parse(data, pro, bass2x, difficulty);
}

Song load_songbytes_sng(const std::vector<uint8_t>& buf, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules) {
    // A notes.mid wins over a notes.chart; among .chart entries the last one
    // listed wins (the order this loader has always used).
    const std::vector<SngFileEntry> entries = sng_read_file_table(buf);
    const SngFileEntry* notes = nullptr;
    ChartFormat format = ChartFormat::None;
    for (const SngFileEntry& e : entries) {
        const ChartFormat f = notes_file_format(e.name);
        if (f == ChartFormat::Mid) {
            notes = &e;
            format = f;
            break;
        }
        if (f == ChartFormat::Chart) {
            notes = &e;
            format = f;
        }
    }
    if (!notes) throw std::runtime_error("No chart files found in SNG file.");

    std::optional<std::vector<uint8_t>> notebytes = sng_decode_file(buf, *notes);
    if (!notebytes) throw std::runtime_error("Truncated SNG file.");
    if (format == ChartFormat::Mid)
        return load_songbytes_mid(*notebytes, pro, bass2x, difficulty, rules);
    return load_songbytes_chart(*notebytes, pro, bass2x, difficulty, rules);
}

Song load_songbytes_srb(const std::vector<uint8_t>& buf, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules) {
    if (buf.size() <= kSrbHeaderSize)
        throw std::runtime_error("Truncated SRB file.");

    // Stream 1 (metadata) names the notes file; stream 2 is its bytes.
    size_t notes_offset = 0;
    std::vector<uint8_t> meta = srb_inflate_stream(
        buf.data(), buf.size(), kSrbHeaderSize, kSrbMaxMetadata, &notes_offset);
    SrbMetadata md;
    srb_parse_metadata(meta, md);

    std::vector<uint8_t> notebytes = srb_inflate_stream(
        buf.data(), buf.size(), notes_offset, kSrbMaxStream, nullptr);

    const ChartFormat named = chart_format_of(md.notes_filename);
    bool is_mid;
    if (named == ChartFormat::Mid)
        is_mid = true;
    else if (named == ChartFormat::Chart)
        is_mid = false;
    else  // Unexpected filename: sniff the payload instead.
        is_mid = notebytes.size() >= 4 && std::memcmp(notebytes.data(), "MThd", 4) == 0;

    if (is_mid) return load_songbytes_mid(notebytes, pro, bass2x, difficulty, rules);
    return load_songbytes_chart(notebytes, pro, bass2x, difficulty, rules);
}

// A container is read from disk once and parsed from those bytes, so the
// Preview can hand the same buffer to its audio extractor (see
// load_songpath_from_bytes).
Song load_songpath_sng(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules) {
    return load_songbytes_sng(read_file_bytes(path), pro, bass2x, difficulty, rules);
}

Song load_songpath_srb(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules) {
    return load_songbytes_srb(read_file_bytes(path), pro, bass2x, difficulty, rules);
}

Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                              bool pro, bool bass2x, Difficulty difficulty,
                              const core::Rules& rules) {
    switch (chart_format_of(path)) {
        case ChartFormat::Mid: return load_songbytes_mid(bytes, pro, bass2x, difficulty, rules);
        case ChartFormat::Chart: return load_songbytes_chart(bytes, pro, bass2x, difficulty, rules);
        case ChartFormat::Sng: return load_songbytes_sng(bytes, pro, bass2x, difficulty, rules);
        case ChartFormat::Srb: return load_songbytes_srb(bytes, pro, bass2x, difficulty, rules);
        case ChartFormat::None: break;
    }
    throw std::runtime_error("unexpected chart type: " + path);
}

Song load_songpath(const std::string& path, bool pro, bool bass2x,
                   Difficulty difficulty, const core::Rules& rules) {
    switch (chart_format_of(path)) {
        case ChartFormat::Mid: return load_songpath_mid(path, pro, bass2x, difficulty, rules);
        case ChartFormat::Chart: return load_songpath_chart(path, pro, bass2x, difficulty, rules);
        case ChartFormat::Sng:
        case ChartFormat::Srb:
            return load_songpath_from_bytes(path, read_file_bytes(path), pro, bass2x,
                                            difficulty, rules);
        case ChartFormat::None: break;
    }
    throw std::runtime_error("unexpected chart type: " + path);
}

}  // namespace hydra
