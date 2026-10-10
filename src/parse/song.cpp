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

#include "core/error_kind.h"
#include "core/strutil.h"
#include "core/winstr.h"  // read_file_bytes, file_byte_source, memory_byte_source
#include "parse/midi.h"
#include "parse/note_shuffle.h"
#include "parse/sng.h"
#include "parse/chart_files.h"
#include "parse/srb.h"
#include "parse/timesig.h"

namespace hydra {

// The timing maps Hydra can measure time with: a positive resolution, every
// measure at least one tick long, every tempo a positive, finite BPM. Both
// parsers reach this through Song::build_timing, so no chart with a zero,
// negative or infinite measure length reaches the engine (D4, R7.6).
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes) {
    constexpr ErrorKind kRefused = ErrorKind::ChartTimingRefused;
    if (tick_resolution <= 0)
        throw ChartFileError(kRefused, std::string(kResolutionRefusalPrefix) +
                                           std::to_string(tick_resolution) +
                                           ", and it must be above 0");
    for (const auto& [tick, len] : tpm_changes)
        if (len <= 0)
            throw ChartFileError(kRefused, std::string(kTimeSignatureRefusalPrefix) +
                                               std::to_string(tick) + " makes a measure " +
                                               std::to_string(len) + " ticks long");
    for (const auto& [tick, bpm] : bpm_changes) {
        // A .mid tempo of 0 microseconds per beat reads back as an infinite BPM
        // (D12): say so, rather than "not above 0 BPM".
        if (std::isinf(bpm) && bpm > 0.0)
            throw ChartFileError(kRefused, std::string(kTempoRefusalPrefix) +
                                               std::to_string(tick) +
                                               " is infinite (0 microseconds per beat)");
        if (!std::isfinite(bpm) || bpm <= 0.0)
            throw ChartFileError(kRefused, std::string(kTempoRefusalPrefix) +
                                               std::to_string(tick) + " is not above 0 BPM");
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

// Practice sections in tick order, the order Song promises. A .chart's
// [Events] need not be sorted, and a .mid lists each EVENTS track's sections
// in turn, so both parsers call this once at the end. The sort is stable, so
// two sections on one tick keep the file's order.
void sort_practice_sections(std::vector<SongSection>& sections) {
    std::stable_sort(sections.begin(), sections.end(),
                     [](const SongSection& a, const SongSection& b) { return a.tick < b.tick; });
}

// The one place solo sections are found: each unbroken run of solo-flagged
// timestamps in the finished sequence is one section. Both parsers call it
// once their sequence is complete. Two solos with no plain chord between them
// read as one run, the same as the replay and the Preview have always read
// them.
std::vector<SoloSection> find_solo_sections(const std::vector<SongTimestamp>& sequence) {
    std::vector<SoloSection> sections;
    for (size_t i = 0; i < sequence.size(); ++i) {
        if (!sequence[i].flag_solo) continue;
        if (!sections.empty() && sections.back().last + 1 == i)
            sections.back().last = i;
        else
            sections.push_back({i, i});
    }
    return sections;
}

// The chart readers' one integer fast path, for the common case of a plain
// number: reads the leading [-]digits of w, as std::strtol and std::strtoll
// read them, when there are 1 to max_digits of them, and says how many bytes
// it read. Returns false for anything else (a sign of '+', no digits, too
// many digits), and the caller then asks the std function, so that
// function's exact rules still decide every unusual spelling. max_digits
// keeps the value inside the caller's type: 9 digits always fit an int, 18 a
// long long.
bool fast_leading_int(std::string_view w, size_t max_digits, int64_t& out, size_t& used) {
    size_t i = 0;
    const bool neg = !w.empty() && w[0] == '-';
    if (neg) i = 1;
    const size_t first_digit = i;
    int64_t v = 0;
    while (i < w.size() && w[i] >= '0' && w[i] <= '9') {
        if (i - first_digit >= max_digits) return false;
        v = v * 10 + (w[i] - '0');
        ++i;
    }
    if (i == first_digit) return false;
    out = neg ? -v : v;
    used = i;
    return true;
}

// The digit counts above which fast_leading_int hands over to std::stoi and
// std::stoll: the most digits that cannot overflow an int and a long long.
constexpr size_t kFastIntDigits = 9;
constexpr size_t kFastLongLongDigits = 18;

// Whether all of s reads as one std::stoll number, and its value.
bool try_parse_int(std::string_view s, int64_t& out) {
    if (s.empty()) return false;
    int64_t fast = 0;
    size_t used = 0;
    if (fast_leading_int(s, kFastLongLongDigits, fast, used)) {
        // std::stoll stops where the digits stop too, so the number is whole
        // only when they run to the end.
        if (used != s.size()) return false;
        out = fast;
        return true;
    }
    // std::stoll reads nothing from a letter and throws; say no without the
    // throw. Property names ("Resolution", "Name") all land here.
    if (std::isalpha(static_cast<unsigned char>(s[0]))) return false;
    try {
        size_t idx = 0;
        long long v = std::stoll(std::string(s), &idx);
        if (idx == s.size()) {
            out = static_cast<int64_t>(v);
            return true;
        }
    } catch (...) {
    }
    return false;
}

// The disco markers both formats read are matched by hand, each exactly the
// whole-string regex it replaced (named beside it). The dynamics marker is not
// a regex any more: it is Clone Hero's two exact strings (finding 64). The tests
// "... disco markers match the regexes they replaced" in test_song.cpp check
// the disco markers through both parsers, with every byte value in every
// position that matters; test_dynamics_tag.cpp pins the dynamics strings.
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

// Clone Hero 1.1 turns dynamics on only when a PART DRUMS text event is
// exactly one of these two strings (String.op_Equality at 0x21557A5 and
// 0x21557BB, on the raw text with no trim). One bracket, extra brackets or
// spaces do nothing (finding 64, D24).
bool is_dynamics_marker(std::string_view s) {
    return s == "ENABLE_CHART_DYNAMICS" || s == "[ENABLE_CHART_DYNAMICS]";
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

// The parser handlers MIDI and .chart share. Each parser decides when to
// call them (its own event phases); what they do to the Song lives here once.

}  // namespace

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
        throw ChartFileError(ErrorKind::ChartTimingRefused,
                             std::string(kTimeSignatureRefusalPrefix) + std::to_string(tick) +
                                 " has a bottom number that is out of range");
    song.tpm_changes[tick] = song.tick_resolution() * static_cast<int64_t>(numerator) * 4 /
                             static_cast<int64_t>(denominator);
    song.timesig_changes[tick] = {numerator, denominator};
}

namespace {

// One authored activation fill, as the chart wrote it.
struct AuthoredFill {
    int64_t start = 0;
    int64_t end = 0;
};

// A placed fill: the chord at `index` becomes an activation chord whose fill
// began at `starttick`. A chord before the fill start takes nothing, so a
// .chart fill with a negative length never gets a negative one.
void apply_fill_end(Song& song, size_t index, int64_t starttick) {
    SongTimestamp& ts = song.sequence[index];
    if (ts.timecode.ticks() >= starttick)
        ts.activation_length = ts.timecode.ticks() - starttick;
}

// The one owner of authored-fill placement, for both parsers, run once every
// chord is read: each fill is placed on its own, as Clone Hero 1.1 does
// (0x20CFF60 calling 0x5DE030; finding 315, D30). The candidates are the last
// chord at or before the fill end, which must not be before the fill start,
// and the first chord after the end, which must be inside the landing window.
// When both qualify the closer wins, and a tie goes to the later chord. A fill
// with neither is dropped, even when one side has no chord at all (Clone
// Hero's 0x5DE030 takes that side unbounded; D30 does not copy it). A later
// fill that picks the same chord replaces the earlier one's length.
//
// The window is Clone Hero 1.1's: the whole ticks of resolution x slop, plus
// one tick (cvttsd2si truncates at 0x20D0088, then inc adds one at
// 0x20D008D), and a chord exactly at its edge still lands (0x5DE17D; D22).
// The +1 sits on top of the rules value, so a hydra_rules.ini slop of 0 still
// lands a chord one tick late.
void place_authored_fills(Song& song, const std::vector<AuthoredFill>& fills,
                          double slop_beats) {
    if (fills.empty() || song.sequence.empty()) return;
    // The chord list with its ticks. Both parsers emit chords in tick order
    // (.mid by its event times, .chart by D47's one sort in ChartParser), so
    // no sort here.
    std::vector<std::pair<int64_t, size_t>> order;
    order.reserve(song.sequence.size());
    for (size_t i = 0; i < song.sequence.size(); ++i)
        order.emplace_back(song.sequence[i].timecode.ticks(), i);
    const int64_t window_ticks =
        static_cast<int64_t>(song.tick_resolution() * slop_beats) + 1;

    for (const AuthoredFill& f : fills) {
        const auto it = std::upper_bound(
            order.begin(), order.end(), f.end,
            [](int64_t t, const std::pair<int64_t, size_t>& c) { return t < c.first; });
        const std::pair<int64_t, size_t>* after = it != order.end() ? &*it : nullptr;
        const std::pair<int64_t, size_t>* before = it != order.begin() ? &*(it - 1) : nullptr;
        if (before && before->first < f.start) before = nullptr;
        if (after && after->first > f.end + window_ticks) after = nullptr;

        const std::pair<int64_t, size_t>* pick = after ? after : before;
        if (before && after)
            pick = (after->first - f.end) <= (f.end - before->first) ? after : before;
        if (pick) apply_fill_end(song, pick->second, f.start);
    }
}

// The one owner of "which chord awards this SP phrase", for both formats. A
// phrase covers the ticks start <= t < end, as Clone Hero 1.1 assigns notes
// to phrases (0x20D2440), and the last chord inside it gets the phrase. So a
// zero-length phrase, or one with no chord inside, awards nothing, and a
// phrase running past the last note is awarded on that note (finding 21,
// D21). Each parser calls this once no later chord can fall inside the
// phrase, always before that tick's own chord is emitted: .mid at the 116
// note-off; .chart at the first tick at or past the end, and once more after
// the last tick for a phrase still open.
void close_sp_phrase(Song& song, int64_t start_tick, int64_t end_tick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    const int64_t t = last.timecode.ticks();
    if (t >= start_tick && t < end_tick) {
        last.flag_sp = true;
        last.sp_phrase_start = start_tick;
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
    return difficulty_chart_codes(difficulty).name;
}

std::optional<Difficulty> difficulty_from_name(std::string_view name) {
    for (Difficulty d : kAllDifficulties) {
        const std::string_view want = difficulty_name(d);
        if (equals_ci(name, want)) return d;
    }
    return std::nullopt;
}

std::string no_notes_message(Difficulty difficulty, bool prodrums) {
    return std::string("No ") + difficulty_name(difficulty) +
           (prodrums ? " Pro Drums" : " Drums") + " notes in this chart.";
}

NoNotesError::NoNotesError(Difficulty difficulty, bool prodrums)
    : ChartFileError(ErrorKind::AlreadyPlain, no_notes_message(difficulty, prodrums)) {}

namespace {

// The one test for "this name is missing": it is empty or holds the
// placeholder the scan stored for a missing one. Either way it reads
// kUnknownTitle.
std::string name_or_unknown(std::string name, std::string_view placeholder) {
    if (name.empty() || name == placeholder) return kUnknownTitle;
    return name;
}

}  // namespace

std::string title_or_unknown(std::string title) {
    // The placeholder the metadata readers used before kUnknownTitle.
    static constexpr const char* kOldPlaceholder = "<unknown title>";
    return name_or_unknown(std::move(title), kOldPlaceholder);
}

// ---- rich-text tags ---------------------------------------------------------

namespace {

struct RichTag {
    std::string_view name;
    bool takes_value;  // the opening tag is <name=value>
};

constexpr RichTag kRichTags[] = {
    {"color", true}, {"size", true}, {"b", false},   {"i", false},
    {"u", false},    {"s", false},   {"sub", false}, {"sup", false},
};

bool is_ascii_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// The byte length of the rich-text tag that starts at text[at] (a '<'), or 0
// when the text there is not one strip_rich_tags removes.
size_t rich_tag_length(std::string_view text, size_t at) {
    size_t i = at + 1;
    const bool closing = i < text.size() && text[i] == '/';
    if (closing) ++i;
    size_t name_end = i;
    while (name_end < text.size() && is_ascii_alpha(text[name_end])) ++name_end;
    if (name_end == i || name_end >= text.size()) return 0;

    const std::string name = to_lower_ascii(text.substr(i, name_end - i));
    for (const RichTag& tag : kRichTags) {
        if (name != tag.name) continue;
        if (text[name_end] == '>')
            return (closing || !tag.takes_value) ? name_end + 1 - at : 0;
        if (!closing && tag.takes_value && text[name_end] == '=') {
            const size_t close = text.find('>', name_end);
            const size_t reopen = text.find('<', name_end);
            if (close == std::string_view::npos || reopen < close) return 0;
            return close + 1 - at;
        }
        return 0;
    }
    return 0;
}

}  // namespace

std::string strip_rich_tags(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] == '<') {
            if (const size_t len = rich_tag_length(text, i)) {
                i += len;
                continue;
            }
        }
        out.push_back(text[i]);
        ++i;
    }
    return out;
}

namespace {

// How every shown name is cleaned: the tags go and the ends are trimmed.
std::string clean_name(std::string_view text) { return trim(strip_rich_tags(text)); }

}  // namespace

std::string display_title(std::string_view title) {
    return title_or_unknown(clean_name(title));
}

std::string display_artist(std::string_view artist) {
    // display_title's rule, plus the artist placeholder the scan stores.
    return name_or_unknown(display_title(artist), kUnknownArtist);
}

std::string display_charter(std::string_view charter) { return clean_name(charter); }

Song::Song(int64_t resolution) : tick_resolution_(resolution) {
    apply_timesig(*this, 0, kDefaultTimeSigNumerator, kDefaultTimeSigDenominator);
}

std::string artist_or_unknown(std::string artist) {
    return artist.empty() ? std::string(kUnknownArtist) : artist;
}

std::string charter_or_unknown(std::string charter) {
    return charter.empty() ? std::string(kUnknownCharter) : charter;
}

// ---- Song::sp_phrase_count ----------------------------------------------

int Song::sp_phrase_count() const {
    int n = 0;
    for (const SongTimestamp& ts : sequence)
        if (ts.flag_sp) ++n;
    return n;
}

// ---- Song::note_count -----------------------------------------------------

int Song::note_count() const {
    int n = 0;
    for (const SongTimestamp& ts : sequence) n += ts.chord.count();
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

enum class MPhase { None, Time, Pre, PreDelayed, Notes, PostDelayed };

// What a MIDI message does to the parser, decided once when the message is
// classified and carried out later in its phase. A plain tagged struct, so a
// tick's handlers cost no allocation (they used to be std::function closures).
enum class MAct : uint8_t {
    None, Note, Kick, FillStart, StoreFillEnd, SpStart, SpEnd, Tom, Flam,
    Solo, Dynamics, Disco, Tempo, TimeSig,
};

struct MOp {
    MPhase phase = MPhase::None;
    MAct act = MAct::None;
    NoteColor color = NoteColor::Kick;                  // Note, Tom
    NoteDynamicType dyn = NoteDynamicType::Normal;      // Note, Kick
    NoteCymbalType cymbal = NoteCymbalType::Normal;     // Tom
    bool flag = false;     // Kick: is2x; Flam/Solo/Disco: on
    int64_t tick = 0;      // FillStart/StoreFillEnd/SpStart/SpEnd/Dynamics/Tempo/TimeSig
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

MOp mop_note(NoteColor color, NoteDynamicType dyn) {
    MOp op = mop(MPhase::Notes, MAct::Note);
    op.color = color;
    op.dyn = dyn;
    return op;
}

MOp mop_kick(NoteDynamicType dyn, bool is2x) {
    MOp op = mop(MPhase::Notes, MAct::Kick);
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
// the four pads). Its 2x kick sits one below the kick
// (DifficultyChartCodes::kick2x_pitch).
namespace {
constexpr DifficultyChartCodes kDifficultyChartCodes[] = {
    {"Expert", 96, '3'},
    {"Hard", 84, '2'},
    {"Medium", 72, '1'},
    {"Easy", 60, '0'},
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

// The marker pitches every difficulty shares on the drum track, each named
// once. kMarkerPitches is the one list of them: is_handled_note reads it, the
// note-off gate in midi_note_is_read reads it, and both of MidiParser::optype's
// switches use these names as their case labels.
constexpr int kSoloMarkerPitch = 103;
constexpr int kFlamMarkerPitch = 109;
constexpr int kYellowTomMarkerPitch = 110;
constexpr int kBlueTomMarkerPitch = 111;
constexpr int kGreenTomMarkerPitch = 112;
constexpr int kSpMarkerPitch = 116;
constexpr int kFillMarkerPitch = 120;
constexpr int kMarkerPitches[] = {
    kSoloMarkerPitch,     kFlamMarkerPitch, kYellowTomMarkerPitch, kBlueTomMarkerPitch,
    kGreenTomMarkerPitch, kSpMarkerPitch,   kFillMarkerPitch,
};

constexpr int lowest_marker_pitch() {
    int lowest = kMarkerPitches[0];
    for (int pitch : kMarkerPitches)
        if (pitch < lowest) lowest = pitch;
    return lowest;
}
// The note-off gate (midi_note_is_read) matches the gate it replaced (a
// note-off below the solo marker is dropped) only while every non-marker
// pitch is below the solo marker.
static_assert(lowest_marker_pitch() == kSoloMarkerPitch,
              "a marker below the solo marker would change the note-off gate");
constexpr int highest_pad_pitch() {
    int highest = 0;
    for (const auto& d : kDifficultyChartCodes)
        if (d.kick_pitch + 4 > highest) highest = d.kick_pitch + 4;
    return highest;
}
static_assert(highest_pad_pitch() < kSoloMarkerPitch,
              "a pad pitch at or above the solo marker would change the note-off gate");

}  // namespace

bool is_midi_marker_pitch(int pitch) {
    return std::find(std::begin(kMarkerPitches), std::end(kMarkerPitches), pitch) !=
           std::end(kMarkerPitches);
}

namespace {

// `base` is the difficulty's kick pitch and `kick2x` its 2x kick pitch, both
// from difficulty_chart_codes. The five note pitches follow the kick, and the
// 2x kick sits one below it. The rest are the shared markers
// (is_midi_marker_pitch). A pitch outside this set belongs to another
// difficulty (or to another instrument) and is dropped, so Expert's 95 is
// never read below Expert: Clone Hero reads each 2x kick only into its own
// difficulty (D20; 0x2155050 at 0x21555CD).
bool is_handled_note(int note, int base, int kick2x) {
    if (note >= base && note <= base + 4) return true;
    if (note == kick2x) return true;
    return is_midi_marker_pitch(note);
}

// Whether the MIDI parser acts on a note message at this pitch. `on` is
// Message::is_note_on. A note-on acts on any handled pitch, but only a marker
// acts on a note-off: a pad or 2x kick note-off is dropped (see the
// static_assert beside kMarkerPitches). MidiParser::optype and the lean read's
// filter (load_songbytes_mid) both ask here.
bool midi_note_is_read(int pitch, bool on, int base, int kick2x) {
    return on ? is_handled_note(pitch, base, kick2x) : is_midi_marker_pitch(pitch);
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
    void op_enable_dynamics(int64_t tick) {
        if (dynamics_enabled_) return;  // a second tag changes nothing
        dynamics_enabled_ = true;
        if (marks_before_tag_ > 0) {
            song_->dynamics_late_tag_tick = tick;
            song_->dynamics_marks_before_tag = marks_before_tag_;
        }
    }
    void op_disco(bool on) { flag_disco_ = on; }
    void op_tempo(int64_t tick, uint32_t miditempo) {
        song_->bpm_changes[tick] = 60000000.0 / static_cast<double>(miditempo);
    }
    void op_timesig(int64_t tick, int numerator, int denominator) {
        apply_timesig(*song_, tick, numerator, denominator);
    }
    void op_fillstart(int64_t tick) { fill_start_tick_ = tick; }
    // The fill marker's note-off: the fill is recorded whole and placed once
    // every chord is read (place_authored_fills). A note-off with no fill
    // open records nothing.
    void op_store_fillend(int64_t tick) {
        if (fill_start_tick_) fills_.push_back({*fill_start_tick_, tick});
        fill_start_tick_.reset();
    }
    void op_sp_start(int64_t tick) { sp_start_tick_ = tick; }
    void op_sp_end(int64_t end_tick) {
        // A note-off with no phrase open (a stray 116 off) closes nothing.
        if (sp_start_tick_) close_sp_phrase(*song_, *sp_start_tick_, end_tick);
        sp_start_tick_.reset();
    }
    void op_tom(NoteColor color, NoteCymbalType cymbal) {
        flag_cymbals_[static_cast<int>(color) - 1] = cymbal;
    }
    void op_flam(bool enabled) { flag_flam_ = enabled; }
    void op_solo(bool on) { flag_solo_ = on; }
    void op_note(NoteColor color, NoteDynamicType dyn) {
        ChordNote& note = chord_.add_note(color);  // may throw ChartFileError
        note.dynamictype = dynamics_enabled_ ? dyn : NoteDynamicType::Normal;
        // A ghost or accent velocity before the tag: Clone Hero prices it as
        // plain, and the Dynamics tab reports how many there were.
        if (!dynamics_enabled_ && dyn != NoteDynamicType::Normal) ++marks_before_tag_;
        if (allows_cymbals(color) && mode_pro_)
            note.cymbaltype = flag_cymbals_[static_cast<int>(color) - 1];
    }
    // A kick, a 2x kick when `is2x`. Chord::add_kick merges a 2x kick and a
    // normal kick on one tick into one kick, so a mark before the tag counts
    // once for it, as for any other note.
    void op_kick(NoteDynamicType dyn, bool is2x) {
        chord_.add_kick(is2x, dynamics_enabled_ ? dyn : NoteDynamicType::Normal);  // may throw
        if (!dynamics_enabled_ && dyn != NoteDynamicType::Normal && !kick_mark_before_tag_) {
            ++marks_before_tag_;
            kick_mark_before_tag_ = true;
        }
    }
    // Once the tick's notes are read: the 2x Bass setting may remove the kick
    // (Chord::apply_2x_bass). A removed kick is never priced, so a mark it
    // had before the tag stops counting, and comes out of the tag's count too
    // when the tag was read on this tick after it.
    void apply_2x_bass(int64_t tick) {
        if (!chord_.apply_2x_bass(mode_bass2x_) || !kick_mark_before_tag_) return;
        --marks_before_tag_;
        if (song_->dynamics_late_tag_tick != std::optional<int64_t>(tick)) return;
        if (--song_->dynamics_marks_before_tag == 0) song_->dynamics_late_tag_tick.reset();
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
    std::vector<MOp> pre_, pre_delayed_, notes_, post_delayed_;
    bool flag_solo_ = false;
    std::array<NoteCymbalType, Chord::kLanes> flag_cymbals_{};
    bool flag_flam_ = false;
    bool flag_disco_ = false;
    std::optional<int64_t> fill_start_tick_;
    std::vector<AuthoredFill> fills_;  // every authored fill, placed after pass 2
    bool dynamics_enabled_ = false;
    int marks_before_tag_ = 0;  // ghost/accent velocities read before the tag
    bool kick_mark_before_tag_ = false;  // this tick's kick counted in marks_before_tag_
    std::optional<int64_t> sp_start_tick_;
};

MOp MidiParser::optype(const Message& msg, int64_t tick) {
    using MType = Message::Type;
    const bool is_channel = (msg.type == MType::NoteOn || msg.type == MType::NoteOff);

    if (is_channel) {
        const int note = msg.note;
        const bool is_noteon = msg.is_note_on();
        if (!midi_note_is_read(note, is_noteon, base_, kick2x_pitch_)) return {};

        if (is_noteon) {
            const int velocity = msg.velocity;
            // The difficulty's own five pitches come first: base is the kick,
            // the next four are Red/Yellow/Blue/Green.
            // Clone Hero reads the kick's velocity exactly as it reads a
            // pad's: 127 is an accent, 1 is a ghost, and both score double.
            const NoteDynamicType vel_dyn =
                velocity == 127   ? NoteDynamicType::Accent
                : velocity == 1   ? NoteDynamicType::Ghost
                                  : NoteDynamicType::Normal;
            if (note == base_) return mop_kick(vel_dyn, false);
            if (note > base_ && note <= base_ + 4) {
                // base+1 -> Red(2), as 97 -> Red(2) on Expert.
                NoteColor color = static_cast<NoteColor>(note - base_ + 1);
                return mop_note(color, vel_dyn);
            }
            // The difficulty's own 2x kick, read whatever the 2x Bass setting:
            // it can take a normal kick on its tick with it (D105).
            if (note == kick2x_pitch_) return mop_kick(vel_dyn, true);
            switch (note) {
                case kFillMarkerPitch:
                    return mop_tick(MPhase::PostDelayed, MAct::FillStart, tick);
                case kSpMarkerPitch:
                    return mop_tick(sp_start_tick_.has_value() ? MPhase::PreDelayed
                                                               : MPhase::Pre,
                                    MAct::SpStart, tick);
                case kGreenTomMarkerPitch: return mop_tom(NoteColor::Green, NoteCymbalType::Normal);
                case kBlueTomMarkerPitch: return mop_tom(NoteColor::Blue, NoteCymbalType::Normal);
                case kYellowTomMarkerPitch: return mop_tom(NoteColor::Yellow, NoteCymbalType::Normal);
                case kFlamMarkerPitch: return mop_flag(MAct::Flam, true);
                case kSoloMarkerPitch: return mop_flag(MAct::Solo, true);
                default:
                    return {};
            }
        }

        // A note-off.
        switch (note) {
            case kFillMarkerPitch:
                return mop_tick(MPhase::Pre, MAct::StoreFillEnd, tick);
            case kSpMarkerPitch:
                return mop_tick(sp_start_tick_.has_value() ? MPhase::Pre
                                                           : MPhase::PreDelayed,
                                MAct::SpEnd, tick);
            case kGreenTomMarkerPitch: return mop_tom(NoteColor::Green, NoteCymbalType::Cymbal);
            case kBlueTomMarkerPitch: return mop_tom(NoteColor::Blue, NoteCymbalType::Cymbal);
            case kYellowTomMarkerPitch: return mop_tom(NoteColor::Yellow, NoteCymbalType::Cymbal);
            case kFlamMarkerPitch: return mop_flag(MAct::Flam, false);
            case kSoloMarkerPitch:
                // A MIDI solo marker covers ticks up to its note-off, not
                // including it: end the solo before this tick's notes.
                // Pinned by ".mid: the note on the solo marker's note-off
                // tick is outside the solo".
                return mop_flag(MAct::Solo, false);
            default:
                return {};
        }
    }

    // Meta family. Text-attribute metas carry `str`; name-attribute metas do
    // not match Python's `MetaMessage(text=...)` patterns.
    if (msg.str_attr == Message::StrAttr::Text) {
        const std::string& t = msg.str;
        // The tag runs with the notes, in file order, so a note written
        // before it at the same tick stays plain, as in Clone Hero, which
        // reads the flag at each note-on (0x21555F1).
        if (is_dynamics_marker(t)) return mop_tick(MPhase::Notes, MAct::Dynamics, tick);
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
        case MAct::Note: op_note(op.color, op.dyn); break;
        case MAct::Kick: op_kick(op.dyn, op.flag); break;
        case MAct::FillStart: op_fillstart(op.tick); break;
        case MAct::StoreFillEnd: op_store_fillend(op.tick); break;
        case MAct::SpStart: op_sp_start(op.tick); break;
        case MAct::SpEnd: op_sp_end(op.tick); break;
        case MAct::Tom: op_tom(op.color, op.cymbal); break;
        case MAct::Flam: op_flam(op.flag); break;
        case MAct::Solo: op_solo(op.flag); break;
        case MAct::Dynamics: op_enable_dynamics(op.tick); break;
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
    kick_mark_before_tag_ = false;

    pre_.clear();
    pre_delayed_.clear();
    notes_.clear();
    post_delayed_.clear();
    for (const Message* msg : msg_buffer_) {
        MOp op = optype(*msg, tick);
        if (!op.runs()) continue;
        switch (op.phase) {
            case MPhase::Pre: pre_.push_back(op); break;
            case MPhase::PreDelayed: pre_delayed_.push_back(op); break;
            case MPhase::Notes: notes_.push_back(op); break;
            case MPhase::PostDelayed: post_delayed_.push_back(op); break;
            default: break;  // Time / None: not run from push_timestamp.
        }
    }

    run_ops(pre_);
    run_ops(pre_delayed_);
    run_ops(notes_);
    apply_2x_bass(tick);

    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, flag_flam_, mode_pro_, flag_disco_,
                             flag_solo_);

    run_ops(post_delayed_);

    msg_buffer_.clear();
}

// Whether the drum track's timestamp so far ends before `msg`: the one owner
// of where one .mid timestamp gives way to the next.
bool opens_timestamp(const Message& msg) {
    return msg.time != 0;
}

Song MidiParser::parse(const MidiFile& mid, bool pro, bool bass2x,
                       Difficulty difficulty) {
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;
    base_ = difficulty_base_pitch(difficulty);
    kick2x_pitch_ = difficulty_chart_codes(difficulty).kick2x_pitch();
    mix_digit_ = difficulty_chart_codes(difficulty).mix_digit;

    Song song(mid.ticks_per_beat);
    song_ = &song;

    // Pass 1: tempo/time-signature marks from the timing track.
    int64_t elapsed = 0;
    for (const Message& msg : mid.tracks[MidiFile::kTimingTrack].messages) {
        elapsed += msg.time;
        MOp op = optype(msg, elapsed);
        if (op.phase == MPhase::Time && op.runs()) run(op);
    }
    song.build_timing();

    // Pass 2: the drum track.
    if (const MidiTrack* const drums = mid.drums_track()) {
        const MidiTrack& track = *drums;
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
        fill_start_tick_.reset();
        fills_.clear();
        marks_before_tag_ = 0;
        // push_timestamp runs at each tick change and once at the end, and
        // emits at most one chord each time, so that count sizes the sequence
        // once.
        size_t pushes = 1;
        for (const Message& msg : track.messages)
            if (opens_timestamp(msg)) ++pushes;
        song.sequence.reserve(pushes);
        for (const Message& msg : track.messages) {
            if (opens_timestamp(msg)) {
                push_timestamp(elapsed);
                elapsed += msg.time;
            }
            msg_buffer_.push_back(&msg);
        }
        push_timestamp(elapsed);
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats);
        song.dynamics_enabled = dynamics_enabled_;
    }

    // Pass 3: practice sections, which live on their own track(s) as bracketed
    // text metas, sorted once at the end.
    for (const MidiTrack& track : mid.tracks) {
        if (!track.is_events()) continue;
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
    sort_practice_sections(song.practice_sections);
    song.solo_sections = find_solo_sections(song.sequence);

    song.check_activations(rules_);
    return song;
}

}  // namespace

// ---- ChartParser --------------------------------------------------------

namespace {

// What one `key = value` line of a .chart says. Prop is a named property
// (its key is not a tick); None is a tick line that says nothing the readers
// use (an empty value, a word they don't know, or a known word with the wrong
// number of words). Every other kind is one event form.
enum class LineKind : uint8_t {
    Prop, None, TimeSig, Tempo, SoloStart, SoloEnd, DiscoOff, DiscoOn, Event, Note, Phrase,
};

// One classified line, as classify_chart_line reads it: a flat value, so the
// drum section is held as one vector of these.
struct ChartLine {
    int64_t tick = 0;
    LineKind kind = LineKind::None;
    int value = 0;       // Note, Phrase: the N or S number; TimeSig: numerator
    int den = 0;         // TimeSig: denominator
    int64_t length = 0;  // Note, Phrase
    double bpm = 0;      // Tempo
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

// A .chart `TS n` line with no second number: a missing exponent is 2 (a
// quarter note), per the .chart format, so `TS 3` is 3/4. This is a rule about
// how the file spells a written line, not Song's default meter
// (kDefaultTimeSig*), which is the meter before any line is written. The two
// are kept apart on purpose: if Song's default ever changed, `TS 3` in a file
// would still mean 3/4. The exponent still goes through timesig_denominator.
constexpr int kChartTsMissingExponent = 2;

// std::stoi / std::stoll on one word, with their exact acceptance rules
// (leading digits read, trailing junk ignored, throws on no digits). A plain
// number takes the fast path; everything else is the std call itself.
int word_stoi(std::string_view w) {
    int64_t v = 0;
    size_t used = 0;
    if (fast_leading_int(w, kFastIntDigits, v, used)) return static_cast<int>(v);
    return std::stoi(std::string(w));
}
long long word_stoll(std::string_view w) {
    int64_t v = 0;
    size_t used = 0;
    if (fast_leading_int(w, kFastLongLongDigits, v, used)) return v;
    return std::stoll(std::string(w));
}

// The one rule for what a .chart line says. Both sides arrive already
// trimmed. `mix_digit` is the parsed difficulty's disco digit: a disco marker
// naming another difficulty is read as a plain text event, which no drum
// section uses. A number that std::stoi or std::stoll refuses throws their
// exception here, whichever section the line sits in. When `event` is given
// and the line is a generic text event, it gets the event's payload: the
// value with its leading "E" and any surrounding quotes taken off.
ChartLine classify_chart_line(std::string_view keystr, std::string_view valuestr,
                              char mix_digit, std::string_view* event) {
    ChartLine out;
    if (!try_parse_int(keystr, out.tick)) {
        out.kind = LineKind::Prop;
        return out;
    }

    const ChartWords t = split_ws_view(valuestr);
    if (t.count == 0) return out;
    const std::string_view t0 = t.w[0];

    if (t0 == "TS" && t.count == 2) {
        out.kind = LineKind::TimeSig;
        out.value = word_stoi(t.w[1]);
        out.den = timesig_denominator(kChartTsMissingExponent);
    } else if (t0 == "TS" && t.count == 3) {
        out.kind = LineKind::TimeSig;
        out.value = word_stoi(t.w[1]);
        out.den = timesig_denominator(word_stoi(t.w[2]));
    } else if (t0 == "B" && t.count == 2) {
        out.kind = LineKind::Tempo;
        out.bpm = static_cast<double>(word_stoll(t.w[1])) / 1000.0;
    } else if (t0 == "E" && t.count == 2 && t.w[1] == "solo") {
        out.kind = LineKind::SoloStart;
    } else if (t0 == "E" && t.count == 2 && t.w[1] == "soloend") {
        out.kind = LineKind::SoloEnd;
    } else if (t0 == "E" && t.count == 2 && is_disco_off_marker(t.w[1], mix_digit)) {
        out.kind = LineKind::DiscoOff;
    } else if (t0 == "E" && t.count == 2 && is_disco_on_marker(t.w[1], mix_digit)) {
        out.kind = LineKind::DiscoOn;
    } else if (t0 == "E") {
        // Generic text event: no gameplay effect, but [Events] carries the
        // practice-section markers here.
        out.kind = LineKind::Event;
        if (event) {
            std::string_view rest = trim_view(valuestr.substr(1));
            if (rest.size() >= 2 && rest.front() == '"' && rest.back() == '"')
                rest = rest.substr(1, rest.size() - 2);
            *event = rest;
        }
    } else if (t0 == "N" && t.count == 3) {
        out.kind = LineKind::Note;
        out.value = word_stoi(t.w[1]);
        out.length = word_stoll(t.w[2]);
    } else if (t0 == "S" && t.count == 3) {
        out.kind = LineKind::Phrase;
        out.value = word_stoi(t.w[1]);
        out.length = word_stoll(t.w[2]);
    }
    return out;
}

// A line of a section kept whole (ChartParser::load_sections says which): the
// line as classify_chart_line reads it, and for a named property its key and raw
// (trimmed) value. Each reader of a property applies its own one number rule
// (Resolution, Offset).
struct ChartDataEntry {
    ChartLine line;
    std::string key_name;
    std::string property_str;

    ChartDataEntry(std::string_view keystr, std::string_view valuestr, char mix_digit)
        : line(classify_chart_line(keystr, valuestr, mix_digit, nullptr)) {
        if (line.kind == LineKind::Prop) {
            key_name = std::string(keystr);
            property_str = std::string(valuestr);
        }
    }

    bool is_tick_data() const { return line.kind != LineKind::Prop; }
};

// A section kept whole. ChartParser::load_sections says which sections are
// kept this way and which are read into lighter forms.
struct ChartSection {
    std::string name;
    std::vector<int64_t> tick_order;
    std::unordered_map<int64_t, std::vector<ChartDataEntry>> tick_data;
    std::unordered_map<std::string, std::vector<ChartDataEntry>> prop_data;

    // Takes the entry by value and moves it into place: no copy per entry.
    void add(ChartDataEntry e) {
        if (e.is_tick_data()) {
            const int64_t key = e.line.tick;
            auto [it, inserted] = tick_data.try_emplace(key);
            if (inserted) tick_order.push_back(key);
            it->second.push_back(std::move(e));
        } else {
            std::vector<ChartDataEntry>& v = prop_data[e.key_name];
            v.push_back(std::move(e));
        }
    }
};

enum class CPhase { None, Time, Notes, NoteMods, Pre, Post, PostDelayed };

// What a .chart event does, decided when it is classified and carried out in
// its phase: a plain tagged struct, so a tick's handlers cost no allocation.
enum class CAct : uint8_t {
    None, Disco, Tempo, TimeSig, Solo, Note, Kick, Accent, Ghost, Cymbal,
    SpStart, SpEnd, FillStart,
};

struct COp {
    CPhase phase = CPhase::None;
    CAct act = CAct::None;
    NoteColor color = NoteColor::Kick;  // Note, Accent, Ghost, Cymbal
    bool flag = false;                  // Disco, Solo: on
    int64_t a = 0;   // Tempo/TimeSig/SpStart/FillStart: tick; SpEnd: start
    int64_t b = 0;   // SpStart/FillStart/SpEnd: end tick
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

// The .chart sections read by name, besides the difficulty's drum section
// (chart_section in DifficultyChartCodes). ChartParser::load_sections keeps
// them and ChartParser::parse reads them back.
constexpr std::string_view kSongSection = "Song";
constexpr std::string_view kSyncTrackSection = "SyncTrack";
constexpr std::string_view kEventsSection = "Events";

class ChartParser {
public:
    explicit ChartParser(const core::Rules& rules) : rules_(rules) {}
    Song parse(const std::vector<uint8_t>& data, bool pro, bool bass2x,
               Difficulty difficulty);

private:
    const core::Rules& rules_;

    void load_sections(const std::vector<uint8_t>& data, char mix_digit,
                       std::string_view drum_section);
    COp optype(const ChartLine& e, int64_t tick);
    COp note_optype(int value) const;  // optype's answer for an N line
    void run(const COp& op);
    void push_timestamp(int64_t tick, const ChartLine* first, const ChartLine* last);

    void op_disco(bool on) { flag_disco_ = on; }
    void op_tempo(int64_t tick, double bpm) { song_->bpm_changes[tick] = bpm; }
    void op_timesig(int64_t tick, int numerator, int denominator) {
        apply_timesig(*song_, tick, numerator, denominator);
    }
    // An S 64 line: the fill is recorded whole and placed once every chord
    // is read (place_authored_fills).
    void op_fillstart(int64_t start, int64_t end) { fills_.push_back({start, end}); }
    void op_sp_start(int64_t start, int64_t end) {
        sp_start_tick_ = start;
        sp_end_tick_ = end;
    }
    void op_sp_end(int64_t start, int64_t end) {
        close_sp_phrase(*song_, start, end);
        sp_end_tick_.reset();
    }
    void op_solo(bool on) { flag_solo_ = on; }
    void op_note(NoteColor color) { chord_.add_note(color); }
    void op_kick(bool is2x) { chord_.add_kick(is2x, NoteDynamicType::Normal); }
    void op_accent(NoteColor color) { chord_.apply_accent(color); }
    void op_ghost(NoteColor color) { chord_.apply_ghost(color); }
    void op_cymbal(NoteColor color) { chord_.apply_cymbal(color); }

    Song* song_ = nullptr;
    bool mode_pro_ = false;
    bool mode_bass2x_ = false;

    // The sections load_sections keeps whole, by name.
    std::unordered_map<std::string, ChartSection> sections_;
    // The parsed difficulty's drum section as its tick lines in file order,
    // and the events section as its practice sections in file order; each is
    // unset when the file has no closed section of that name.
    std::optional<std::vector<ChartLine>> drum_lines_;
    std::optional<std::vector<SongSection>> event_sections_;

    Chord chord_;
    std::vector<COp> ops_;  // one tick's handlers, reused tick to tick
    bool flag_solo_ = false;
    bool flag_disco_ = false;
    std::optional<int64_t> sp_start_tick_;
    std::optional<int64_t> sp_end_tick_;
    std::vector<AuthoredFill> fills_;  // every authored fill, placed after the section
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

// The line of `text` that starts at `pos`, trimmed, with `pos` moved to the
// start of the next one. ChartParser::load_sections says which lines count.
std::string_view next_chart_line(std::string_view text, size_t& pos) {
    const size_t nl = text.find('\n', pos);
    const size_t stop = nl == std::string_view::npos ? text.size() : nl;
    const std::string_view line = trim_view(text.substr(pos, stop - pos));
    pos = nl == std::string_view::npos ? text.size() : nl + 1;
    return line;
}

// The line that closes a section, for load_sections and the count below.
constexpr std::string_view kSectionCloseLine = "}";

// How many lines a section has from `pos` up to the line that closes it in
// ChartParser::load_sections. It sizes the drum section's line list once; the
// lines load_sections drops make it run high, never low.
size_t lines_to_section_close(std::string_view text, size_t pos) {
    size_t n = 0;
    while (pos < text.size() && next_chart_line(text, pos) != kSectionCloseLine) ++n;
    return n;
}

void ChartParser::load_sections(const std::vector<uint8_t>& data, char mix_digit,
                                std::string_view drum_section) {
    // Walk the file in place, one line per '\n' (a trailing '\r' is removed by
    // the trim). Every line ended by a '\n' counts, empty or not; the last
    // unterminated piece counts only when it is non-empty.
    const std::string_view text(reinterpret_cast<const char*>(data.data()), data.size());

    // Every line of every section is classified, so a number the reader
    // refuses throws wherever it sits. What is kept depends on the section:
    // [Song] and [SyncTrack] whole, the drum section as its tick lines,
    // [Events] as its practice sections, any other section nothing. A section
    // counts once its '}' is read, and a later section of the same name
    // replaces an earlier one.
    enum class Keep { Whole, Drums, Events, Nothing };
    bool in_section = false;
    Keep keep = Keep::Nothing;
    std::optional<ChartSection> whole;
    std::vector<ChartLine> drums;
    std::vector<SongSection> events;
    size_t pos = 0;
    while (pos < text.size()) {
        const std::string_view line = next_chart_line(text, pos);

        if (in_section) {
            if (line == "{") {
                // block open
            } else if (line == kSectionCloseLine) {
                switch (keep) {
                    case Keep::Whole:
                        sections_[whole->name] = std::move(*whole);
                        whole.reset();
                        break;
                    case Keep::Drums: drum_lines_ = std::move(drums); break;
                    case Keep::Events: event_sections_ = std::move(events); break;
                    case Keep::Nothing: break;
                }
                in_section = false;
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
                lhs = trim_view(lhs);
                rhs = trim_view(rhs);
                if (keep == Keep::Whole) {
                    whole->add(ChartDataEntry(lhs, rhs, mix_digit));
                } else if (keep == Keep::Events) {
                    std::string_view event;
                    const ChartLine l = classify_chart_line(lhs, rhs, mix_digit, &event);
                    std::string name;
                    if (l.kind == LineKind::Event && section_name_of(std::string(event), &name))
                        events.push_back({l.tick, std::move(name)});
                } else {
                    const ChartLine l = classify_chart_line(lhs, rhs, mix_digit, nullptr);
                    if (keep == Keep::Drums && l.kind != LineKind::Prop) drums.push_back(l);
                }
            }
        } else {
            std::string_view bracket;
            if (!find_section_header(line, &bracket))
                throw ChartFileError("expected a [section] header");
            const std::string_view name = bracket.substr(1, bracket.size() - 2);
            in_section = true;
            if (name == kSongSection || name == kSyncTrackSection) {
                keep = Keep::Whole;
                whole.emplace();
                whole->name = std::string(name);
            } else if (name == drum_section) {
                keep = Keep::Drums;
                drums.clear();
                drums.reserve(lines_to_section_close(text, pos));
            } else if (name == kEventsSection) {
                keep = Keep::Events;
                events.clear();
            } else {
                keep = Keep::Nothing;
            }
        }
    }
}

COp ChartParser::optype(const ChartLine& e, int64_t tick) {
    switch (e.kind) {
        case LineKind::DiscoOn: return cop_flag(CPhase::Pre, CAct::Disco, true);
        case LineKind::DiscoOff: return cop_flag(CPhase::Pre, CAct::Disco, false);
        case LineKind::Tempo: {
            COp op = cop_span(CPhase::Time, CAct::Tempo, tick, 0);
            op.bpm = e.bpm;
            return op;
        }
        case LineKind::TimeSig: {
            COp op = cop_span(CPhase::Time, CAct::TimeSig, tick, 0);
            op.num = e.value;
            op.den = e.den;
            return op;
        }
        case LineKind::SoloStart: return cop_flag(CPhase::Pre, CAct::Solo, true);
        // A .chart `E soloend` sits on the solo's last note: end the solo
        // after this tick's notes. Pinned by ".chart: the note on the solo
        // end tick is in the solo".
        case LineKind::SoloEnd: return cop_flag(CPhase::Post, CAct::Solo, false);
        case LineKind::Note: return note_optype(e.value);
        case LineKind::Phrase:
            if (e.value == 2) return cop_span(CPhase::Pre, CAct::SpStart, tick, tick + e.length);
            if (e.value == 64)
                return cop_span(CPhase::PostDelayed, CAct::FillStart, tick, tick + e.length);
            return {};
        case LineKind::Prop:
        case LineKind::None:
        case LineKind::Event: return {};
    }
    return {};
}

// An N line's op, by its note number.
COp ChartParser::note_optype(int value) const {
    constexpr CPhase N = CPhase::Notes, M = CPhase::NoteMods;
    switch (value) {
        case 0: return cop_flag(N, CAct::Kick, false);
        case 1: return cop_color(N, CAct::Note, NoteColor::Red);
        case 2: return cop_color(N, CAct::Note, NoteColor::Yellow);
        case 3: return cop_color(N, CAct::Note, NoteColor::Blue);
        case 4: return cop_color(N, CAct::Note, NoteColor::Green);
        // The 2x kick, read whatever the 2x Bass setting: it can take a
        // normal kick on its tick with it (D105).
        case 32: return cop_flag(N, CAct::Kick, true);
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

void ChartParser::run(const COp& op) {
    switch (op.act) {
        case CAct::None: break;
        case CAct::Disco: op_disco(op.flag); break;
        case CAct::Tempo: op_tempo(op.a, op.bpm); break;
        case CAct::TimeSig: op_timesig(op.a, op.num, op.den); break;
        case CAct::Solo: op_solo(op.flag); break;
        case CAct::Note: op_note(op.color); break;
        case CAct::Kick: op_kick(op.flag); break;
        case CAct::Accent: op_accent(op.color); break;
        case CAct::Ghost: op_ghost(op.color); break;
        case CAct::Cymbal: op_cymbal(op.color); break;
        case CAct::SpStart: op_sp_start(op.a, op.b); break;
        case CAct::SpEnd: op_sp_end(op.a, op.b); break;
        case CAct::FillStart: op_fillstart(op.a, op.b); break;
    }
}

void ChartParser::push_timestamp(int64_t tick, const ChartLine* first, const ChartLine* last) {
    chord_ = Chord();

    std::vector<COp>& ops = ops_;
    ops.clear();
    for (const ChartLine* e = first; e != last; ++e) {
        COp op = optype(*e, tick);
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
    chord_.apply_2x_bass(mode_bass2x_);
    run_phase(CPhase::NoteMods);

    // Phrase end: SP.
    if (sp_end_tick_.has_value() && tick >= *sp_end_tick_) {
        const int64_t start = sp_start_tick_.value_or(0);
        ops.insert(ops.begin(), cop_span(CPhase::Pre, CAct::SpEnd, start, *sp_end_tick_));
    }

    run_phase(CPhase::Pre);

    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, /*apply_flam=*/false, mode_pro_,
                             flag_disco_, flag_solo_);

    run_phase(CPhase::Post);
    run_phase(CPhase::PostDelayed);
}

// The line after the last one that shares `p`'s timestamp, in tick-sorted
// drum lines: the one owner of which lines make one .chart timestamp.
const ChartLine* tick_group_end(const ChartLine* p, const ChartLine* end) {
    const ChartLine* q = p + 1;
    while (q != end && q->tick == p->tick) ++q;
    return q;
}

Song ChartParser::parse(const std::vector<uint8_t>& data, bool pro,
                        bool bass2x, Difficulty difficulty) {
    // A disco marker counts only in the difficulty it names, so the reader
    // needs this difficulty's digit before it classifies any line.
    load_sections(data, difficulty_chart_codes(difficulty).mix_digit,
                  difficulty_chart_codes(difficulty).chart_section());
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;

    const ChartSection& song_sec = sections_.at(std::string(kSongSection));
    const ChartDataEntry& res_entry = song_sec.prop_data.at("Resolution").at(0);
    // One number rule for Resolution: std::stoll on the raw text (leading
    // digits read, trailing junk ignored, no digits refuses the chart).
    const int64_t tick_resolution = std::stoll(res_entry.property_str);

    Song song(tick_resolution);
    song_ = &song;

    // Offset is a decimal number of seconds, read from its raw text by the one
    // chart-number rule (parse_finite_number), integer or not, so "500ms" or
    // "nan" counts as absent, like Clone Hero's default 0.
    if (auto it = song_sec.prop_data.find("Offset");
        it != song_sec.prop_data.end() && !it->second.empty()) {
        song.chart_offset_s = parse_finite_number(it->second.at(0).property_str);
    }

    // Map tempo and time signatures from the sync track.
    auto sync_it = sections_.find(std::string(kSyncTrackSection));
    if (sync_it != sections_.end()) {
        const ChartSection& sync = sync_it->second;
        for (int64_t tk : sync.tick_order) {
            for (const ChartDataEntry& e : sync.tick_data.at(tk)) {
                COp op = optype(e.line, tk);
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
    if (drum_lines_.has_value()) {
        // The one place chord order is settled for a .chart (D47): the drum
        // section is read in tick order, whatever order the file wrote its
        // ticks in, so the phrase rule (close_sp_phrase, D21) and the fill
        // rule (place_authored_fills, D30) see the same chords in the same
        // order. The sort is stable, so the lines at one tick keep the file's
        // order; a file already in order is not sorted again.
        std::vector<ChartLine>& lines = *drum_lines_;
        const auto by_tick = [](const ChartLine& a, const ChartLine& b) { return a.tick < b.tick; };
        if (!std::is_sorted(lines.begin(), lines.end(), by_tick))
            std::stable_sort(lines.begin(), lines.end(), by_tick);
        // push_timestamp runs once per tick and emits at most one chord, so
        // the tick count sizes the sequence once.
        const ChartLine* const end = lines.data() + lines.size();
        size_t ticks = 0;
        for (const ChartLine* p = lines.data(); p != end; p = tick_group_end(p, end)) ++ticks;
        song.sequence.reserve(ticks);
        for (const ChartLine* p = lines.data(); p != end;) {
            const ChartLine* q = tick_group_end(p, end);
            push_timestamp(p->tick, p, q);
            p = q;
        }
        // A phrase still open after the last tick runs past the last note.
        // Close it now, so that note awards it, as the 116 note-off does in
        // a .mid.
        if (sp_end_tick_.has_value())
            op_sp_end(sp_start_tick_.value_or(0), *sp_end_tick_);
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats);
    }

    // Practice sections, in the file's order, which is not required to be
    // sorted; sort_practice_sections orders them.
    if (event_sections_.has_value()) {
        song.practice_sections = std::move(*event_sections_);
        sort_practice_sections(song.practice_sections);
    }
    song.solo_sections = find_solo_sections(song.sequence);

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

namespace {

// The chord list outlives the parse (the engine and the Preview read it), and
// each parser sizes it from a count that can run high. So its spare room goes
// back once the parser's own buffers are freed. Every loader ends at
// load_songbytes_mid or load_songbytes_chart, which call this.
void trim_parsed_song(Song& song) {
    song.sequence.shrink_to_fit();
}

Song parse_mid_lean(const std::vector<uint8_t>& data, bool pro, bool bass2x,
                    Difficulty difficulty, const core::Rules& rules) {
    // Only the notes MidiParser acts on are decoded (midi_note_is_read).
    const int base = difficulty_base_pitch(difficulty);
    const int kick2x = difficulty_chart_codes(difficulty).kick2x_pitch();
    MidiLeanFilter filter;
    for (int pitch = 0; pitch < kMidiDataValues; ++pitch) {
        filter.note_on[pitch] = midi_note_is_read(pitch, true, base, kick2x);
        filter.note_off[pitch] = midi_note_is_read(pitch, false, base, kick2x);
    }
    const MidiFile mid = MidiFile::lean(data.data(), data.size(), filter);
    return MidiParser(rules).parse(mid, pro, bass2x, difficulty);
}

// The Note Shuffle switch, applied once a parser has finished the song: the
// fill, solo and Star Power passes inside the parsers read only ticks and
// flags, never a pad, so they see the same chords either way. Both public
// byte loaders end here.
void apply_shuffle_switch(Song& song, bool pro, bool noteshuffle) {
    if (!noteshuffle) return;
    if (apply_note_shuffle(song.sequence, pro) == NoteShuffleResult::GameFreezes)
        throw KindedError(ErrorKind::NoteShuffleFreezes, kNoteShuffleFreezeDetail);
}

}  // namespace

Song load_songbytes_mid(const std::vector<uint8_t>& data, bool pro,
                        bool bass2x, Difficulty difficulty, const core::Rules& rules,
                        bool noteshuffle) {
    // The lean file is gone once parse_mid_lean returns.
    Song song = parse_mid_lean(data, pro, bass2x, difficulty, rules);
    apply_shuffle_switch(song, pro, noteshuffle);
    trim_parsed_song(song);
    return song;
}

Song load_songbytes_chart(const std::vector<uint8_t>& data, bool pro,
                          bool bass2x, Difficulty difficulty, const core::Rules& rules,
                          bool noteshuffle) {
    // The parser and its sections are gone by the end of this statement.
    Song song = ChartParser(rules).parse(data, pro, bass2x, difficulty);
    apply_shuffle_switch(song, pro, noteshuffle);
    trim_parsed_song(song);
    return song;
}

Song load_songpath_mid(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_songbytes_mid(read_file_bytes(path), pro, bass2x, difficulty, rules,
                              noteshuffle);
}

Song load_songpath_chart(const std::string& path, bool pro, bool bass2x,
                         Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_songbytes_chart(read_file_bytes(path), pro, bass2x, difficulty, rules,
                                noteshuffle);
}

namespace {

// The one notes reader for each container kind, whether the bytes come from
// disk in pieces or from a buffer in memory (the Preview reads the whole file
// once and shares it; its notes are picked out the same way).
Song load_container_sng(const ByteSource& src, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    const SngNotes notes = sng_read_notes(src, sng_read_head(src));
    if (notes.format == ChartFormat::Mid)
        return load_songbytes_mid(notes.bytes, pro, bass2x, difficulty, rules, noteshuffle);
    return load_songbytes_chart(notes.bytes, pro, bass2x, difficulty, rules, noteshuffle);
}

Song load_container_srb(const ByteSource& src, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    // Stream 1 (metadata) names the notes file; stream 2 is its bytes.
    // srb_read_metadata reads stream 1 and refuses a source too short for it.
    const SrbMetadataRead read = srb_read_metadata(src);
    const SrbMetadata& md = read.fields;

    std::vector<uint8_t> notebytes =
        srb_inflate_stream_reading(src, read.notes_offset, kSrbMaxStream, nullptr);

    // The notes stream's format comes from its name, by the exact-name rule a
    // .sng entry and a loose folder use (notes_file_format). An .srb's notes
    // are always stream 2, so a name outside that rule is not fatal: the
    // stream's own bytes decide below.
    const ChartFormat named = notes_file_format(md.notes_filename);
    bool is_mid;
    if (named == ChartFormat::Mid)
        is_mid = true;
    else if (named == ChartFormat::Chart)
        is_mid = false;
    else  // Unexpected filename: sniff the payload instead.
        is_mid = notebytes.size() >= 4 && std::memcmp(notebytes.data(), "MThd", 4) == 0;

    if (is_mid) return load_songbytes_mid(notebytes, pro, bass2x, difficulty, rules, noteshuffle);
    return load_songbytes_chart(notebytes, pro, bass2x, difficulty, rules, noteshuffle);
}

}  // namespace

Song load_songbytes_sng(const std::vector<uint8_t>& buf, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_container_sng(memory_byte_source(buf), pro, bass2x, difficulty, rules,
                              noteshuffle);
}

Song load_songbytes_srb(const std::vector<uint8_t>& buf, bool pro, bool bass2x,
                        Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_container_srb(memory_byte_source(buf), pro, bass2x, difficulty, rules,
                              noteshuffle);
}

// A container on disk is read in pieces: its header, then only the notes. The
// rest is audio and art (about 1 GB for the largest .sng in the library), which
// the notes never need.
Song load_songpath_sng(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_container_sng(file_byte_source(path), pro, bass2x, difficulty, rules,
                              noteshuffle);
}

Song load_songpath_srb(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_container_srb(file_byte_source(path), pro, bass2x, difficulty, rules,
                              noteshuffle);
}

Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                              bool pro, bool bass2x, Difficulty difficulty,
                              const core::Rules& rules, bool noteshuffle) {
    return load_songpath_reading(memory_byte_source(bytes), path, pro, bass2x, difficulty,
                                 rules, noteshuffle);
}

Song load_songpath_reading(const ByteSource& src, const std::string& path, bool pro,
                           bool bass2x, Difficulty difficulty, const core::Rules& rules,
                           bool noteshuffle) {
    switch (chart_format_of(path)) {
        case ChartFormat::Mid:
            return load_songbytes_mid(read_all(src), pro, bass2x, difficulty, rules,
                                      noteshuffle);
        case ChartFormat::Chart:
            return load_songbytes_chart(read_all(src), pro, bass2x, difficulty, rules,
                                        noteshuffle);
        case ChartFormat::Sng:
            return load_container_sng(src, pro, bass2x, difficulty, rules, noteshuffle);
        case ChartFormat::Srb:
            return load_container_srb(src, pro, bass2x, difficulty, rules, noteshuffle);
        case ChartFormat::None: break;
    }
    throw KindedError(ErrorKind::ChartUnreadable, "unexpected chart type: " + path);
}

Song load_songpath(const std::string& path, bool pro, bool bass2x,
                   Difficulty difficulty, const core::Rules& rules, bool noteshuffle) {
    return load_songpath_reading(file_byte_source(path), path, pro, bass2x, difficulty, rules,
                                 noteshuffle);
}

void require_notes(const Song& song, Difficulty difficulty, bool prodrums) {
    if (song.is_empty()) throw NoNotesError(difficulty, prodrums);
}

Song load_songpath_with_notes(const std::string& path, bool pro, bool bass2x,
                              Difficulty difficulty, const core::Rules& rules,
                              bool noteshuffle) {
    Song song = load_songpath(path, pro, bass2x, difficulty, rules, noteshuffle);
    require_notes(song, difficulty, pro);
    return song;
}

}  // namespace hydra
