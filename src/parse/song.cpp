#include "parse/song.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

#include "core/error_kind.h"
#include "core/strutil.h"
#include "core/winstr.h"  // read_file_bytes, file_byte_source, memory_byte_source
#include "parse/midi.h"
#include "parse/note_shuffle.h"
#include "parse/section_names.h"
#include "parse/sng.h"
#include "parse/chart_files.h"
#include "parse/srb.h"
#include "parse/text_decode.h"
#include "parse/timesig.h"

namespace hydra {

// ---- shared helpers -----------------------------------------------------

namespace {

// Whether a text event's body marks a practice section, and if so the name it
// shows. Both questions are answered in parse/section_names.cpp; this is the
// one door both readers use, and each says which reader it is.
bool section_name_of(std::string_view body, SectionReader reader, std::string* name) {
    if (!is_section_marker(body, reader)) return false;
    *name = section_display_name(body);
    return true;
}

// The body of the text event that marks where the song ends (Song::end_event_tick).
constexpr std::string_view kEndEventBody = "end";

// Whether a text event's body, its brackets already taken off, is the end
// event. The match is exact and case-sensitive, as Clone Hero's own string
// compare is. Both readers ask here; each says which brackets it takes off.
bool is_end_event(std::string_view body) { return body == kEndEventBody; }

// The body of the text event that marks the coda. The first one in a chart
// gives the coda tick that place_authored_fills skips fills from.
constexpr std::string_view kCodaEventBody = "coda";

// Whether a text event's body, already trimmed and with its brackets taken
// off, is the coda. The match is exact and case-sensitive, as Clone Hero's own
// compare is (0x21544E0 for a .mid, 0x2151830 for a .chart). Both readers ask
// here; each says what it trims and which brackets it takes off, because the
// game's two readers differ there.
bool is_coda_event(std::string_view body) { return body == kCodaEventBody; }

// Practice sections in tick order, the order Song promises. A .chart's
// [Events] need not be sorted. A .mid's events track (MidiFile::events_track)
// is read in tick order already, but both parsers call this once at the end,
// so the promise has one keeper. The sort is stable, so two sections on one
// tick keep the file's order.
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

// Whether a note can light a drum lane's column. Kicks never do: a lane runs
// down a pad column, and the kick has none.
bool lights_lane_pad(const ChordNote& note) { return note.colortype != NoteColor::Kick; }

// A chord's pads as a drum lane sees them: how many notes it has that light
// a pad (lights_lane_pad), and the lowest and highest of them in Clone Hero's
// note-bit order (the game's lane builder compares lane_bit values).
struct LanePads {
    int count = 0;
    NoteColor lowest = NoteColor::Red;
    NoteColor highest = NoteColor::Red;
};

// Where a note sits in that bit order: the position of its lane bit
// (lane_bit, core/model.h), so the order is read off the game's bit
// table and has no second copy here.
int lane_bit_rank(const ChordNote& note) {
    int rank = 0;
    for (uint32_t bit = lane_bit(note); (bit & 1u) == 0; bit >>= 1) ++rank;
    return rank;
}

LanePads lane_pads(const Chord& chord) {
    LanePads out;
    int low_rank = 0, high_rank = 0;
    for (const ChordNote& note : chord.note_list()) {
        if (!lights_lane_pad(note)) continue;
        const int rank = lane_bit_rank(note);
        if (out.count == 0 || rank < low_rank) {
            low_rank = rank;
            out.lowest = note.colortype;
        }
        if (out.count == 0 || rank > high_rank) {
            high_rank = rank;
            out.highest = note.colortype;
        }
        ++out.count;
    }
    return out;
}

// The chords a drum lane picks its pads from: those from one tick before the
// lane's start up to its end, as [first, last) in `sequence`.
using LaneSpan = std::pair<std::vector<SongTimestamp>::const_iterator,
                           std::vector<SongTimestamp>::const_iterator>;
LaneSpan lane_span(const DrumLane& lane, const std::vector<SongTimestamp>& sequence) {
    const auto first = std::lower_bound(sequence.begin(), sequence.end(), lane.start_tick - 1,
                                        [](const SongTimestamp& ts, int64_t tick) {
                                            return ts.timecode.ticks() < tick;
                                        });
    auto last = first;
    while (last != sequence.end() && last->timecode.ticks() < lane.end_tick) ++last;
    return {first, last};
}

// Clone Hero's pads for one lane (D112): picked from the first chords with a
// pad. Empty when the span has none. A double lane's second pad is picked the
// way the game's lane builder picks it, which NL-55 on
// docs/clone-hero/02-note-list.md records with its listings (D333). The case
// with no later chord is one the listings do not show, so it follows the
// user's choice in D333.
std::vector<NoteColor> game_lane_pads(const DrumLane& lane, const LaneSpan& span,
                                      const std::vector<SongTimestamp>& sequence) {
    auto it = span.first;
    LanePads first;
    for (; it != span.second; ++it) {
        first = lane_pads(it->chord);
        if (first.count > 0) break;
    }
    if (first.count == 0) return {};
    std::vector<NoteColor> pads = {first.lowest};
    if (lane.double_lane) {
        std::optional<NoteColor> second;
        for (auto next = it + 1; next != sequence.end(); ++next) {
            const LanePads p = lane_pads(next->chord);
            if (p.count > 0) {
                second = p.highest;
                break;
            }
        }
        if (second && *second != first.lowest) pads.push_back(*second);
    }
    return pads;
}

// Every pad a note in the span lands on (D254), each colour once. A yellow
// tom and a yellow cymbal share the yellow column, so they light it once.
std::vector<NoteColor> every_lane_pad(const LaneSpan& span) {
    std::vector<NoteColor> pads;
    for (auto it = span.first; it != span.second; ++it) {
        for (const ChordNote& note : it->chord.note_list()) {
            if (!lights_lane_pad(note)) continue;
            if (std::find(pads.begin(), pads.end(), note.colortype) == pads.end())
                pads.push_back(note.colortype);
        }
    }
    return pads;
}

// The one place a drum lane's pads are worked out. The loader tail
// (finish_loaded_song) calls it once, with the lanes the parser recorded at
// the loaded difficulty, after the Note Shuffle switch, and passes that
// switch here.
//
// With Note Shuffle off, a lane follows Clone Hero's rule (game_lane_pads,
// chosen by the user in D112). With it on, a lane lights every pad its
// shuffled notes land on (every_lane_pad). That is a deliberate deviation
// from the game, which still picks from the first notes: the user ruled that
// a shuffled roll looks better with every pad lit (D254). Either way the
// lane sees the shuffled notes, because the game's lane holds references to
// its notes and the shuffle recolours them in place (D112's 2026-10-17
// correction names the listings).
//
// Both modes read the same span (lane_span), and a lane with no pad there is
// dropped. The game's own cut-off for a lane whose notes start late is
// unknown, so none is applied (the user's decision). Returned in lane-start
// order, each lane's pads in red-to-green order with a repeated colour once.
std::vector<DrumLane> light_drum_lanes(std::vector<DrumLane> lanes,
                                       const std::vector<SongTimestamp>& sequence,
                                       bool noteshuffle) {
    std::stable_sort(lanes.begin(), lanes.end(), [](const DrumLane& a, const DrumLane& b) {
        return a.start_tick < b.start_tick;
    });
    std::vector<DrumLane> lit;
    lit.reserve(lanes.size());
    for (DrumLane& lane : lanes) {
        const LaneSpan span = lane_span(lane, sequence);
        lane.pads = noteshuffle ? every_lane_pad(span) : game_lane_pads(lane, span, sequence);
        if (lane.pads.empty()) continue;
        std::sort(lane.pads.begin(), lane.pads.end(), [](NoteColor a, NoteColor b) {
            return static_cast<int>(a) < static_cast<int>(b);
        });
        lit.push_back(std::move(lane));
    }
    return lit;
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

// The disco markers both formats read are matched by hand. The dynamics marker
// is Clone Hero's two exact strings (finding 64). The tests "... disco markers
// match the pinned tables" in test_song_parse.cpp check the disco
// markers through both parsers, with every byte value in every position that
// matters; test_dynamics_tag.cpp pins the dynamics strings.
//
// regex_dot and regex_digit mean what `.` and `\d` mean to std::regex
// (ECMAScript, char). No marker body starts with '[' or ends with ']', so
// peel_brackets is never ambiguous.
bool regex_dot(char c) { return c != '\n' && c != '\r'; }
bool regex_digit(char c) { return c >= '0' && c <= '9'; }

std::string_view peel_brackets(std::string_view s) {
    if (!s.empty() && s.front() == '[') s.remove_prefix(1);
    if (!s.empty() && s.back() == ']') s.remove_suffix(1);
    return s;
}

// The head both disco markers share. `mix_digit` is the parsed difficulty's
// digit (difficulty_chart_codes). A marker naming another difficulty is not a
// marker here: Clone Hero applies each marker only to the difficulty it names,
// in both formats (D19; 0x215C750, 0x213D076, 0x2155050).
bool disco_head(std::string_view s, char mix_digit) {
    return s.size() >= 11 && s.substr(0, 3) == "mix" && regex_dot(s[3]) &&
           s[4] == mix_digit && regex_dot(s[5]) && s.substr(6, 5) == "drums";
}

// Clone Hero 1.1 turns dynamics on only when a drum track text event is
// exactly one of these two strings (String.op_Equality at 0x21557A5 and
// 0x21557BB, on the raw text with no trim). One bracket, extra brackets or
// spaces do nothing (finding 64, D24).
bool is_dynamics_marker(std::string_view s) {
    return s == "ENABLE_CHART_DYNAMICS" || s == "[ENABLE_CHART_DYNAMICS]";
}

// Whether `s` opens the disco section of the difficulty `mix_digit` names.
bool is_disco_on_marker(std::string_view s, char mix_digit) {
    s = peel_brackets(s);
    if (!disco_head(s, mix_digit)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest == "d";
}

// Whether `s` closes the disco section of the difficulty `mix_digit` names.
// "dnoflip" reads as flip off. Clone Hero turns flip on for it (its classifier
// at 0x215CD50 looks only at the 7th character); Hydra keeps it off on purpose
// (D19).
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
//
// A fill the coda skips is never placed. `coda_tick` is the chart's first coda
// event, which each reader finds through is_coda_event. Clone Hero 1.1's track
// finish marks the fills from the coda on (0x215DA70, the setge at 0x215DB6D)
// and the landing step passes over a marked fill (0x20CFF60). Build 1.0 skips
// only a fill on the coda tick (code/4045/fill_generator.cs line 88), and
// FillDeadlineRule::Ch10 follows it (FL-08, FL-39, Q-40). A skipped fill
// still marks the song (Song::coda_skipped_fill), so no fill is generated.
void place_authored_fills(Song& song, const std::vector<AuthoredFill>& fills,
                          double slop_beats, std::optional<int64_t> coda_tick,
                          FillDeadlineRule fills_rule) {
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
        if (coda_tick.has_value()) {
            const bool skipped = fills_rule == FillDeadlineRule::Ch10
                                     ? f.start == *coda_tick
                                     : f.start >= *coda_tick;
            if (skipped) {
                song.coda_skipped_fill = true;
                continue;
            }
        }
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

// One tick's drum notes as the chart wrote them, before the five-lane
// conversion (convert_drum_pads). A chart can write five pads: red, yellow,
// blue, a fourth pad (the game's OrangeDrum, the pad Hydra's chord calls
// green) and a fifth pad (the game's GreenDrum). `four` holds the kick and the
// first four pads, each on its own lane, the fourth pad on the green lane.
// `fifth` holds the fifth pad alone, also on its green lane. Keeping it apart
// means a tick holding both the fourth and the fifth pad never puts two notes
// on one lane before the conversion decides where the fifth pad goes. Each
// holds its pads' ghost, accent and cymbal marks as the reader read them.
struct TickPads {
    Chord four;
    Chord fifth;

    Chord& pads(bool fifth_pad) { return fifth_pad ? fifth : four; }
    int count() const { return four.count() + fifth.count(); }
};

// The facts about a whole drum chart that pick the conversion's branch: Clone
// Hero's track flags +0xA1 and +0xA2 (docs/clone-hero/01-reading-a-chart.md
// CR-18), and the song.ini switch that picks the five-lane branch on its own.
// Each reader works them out once, before its first chord: the .mid reader in
// midi_drum_chart_flags, the .chart reader in ChartParser::parse and
// ChartParser::load_sections.
struct DrumChartFlags {
    bool fifth_pad = false;  // a fifth-pad note on any difficulty
    bool pro_marks = false;  // a Pro Drums tom or cymbal marker
    // song.ini's five_lane_drums (ChartDrumKeys). Probes C and E on
    // v1.1.0.6142 (ch-mcp-runs/2026-11-12/five-lane) show the game takes the
    // five-lane branch whenever it is set, Pro marks or not, in a .chart too.
    bool five_lane_key = false;
};

// The five-lane conversion, Clone Hero's track step 0x215D450
// (docs/audit/note-shuffle/step0/open2-refute/f215D450.asm, rules NL-74 to
// NL-76 on docs/clone-hero/02-note-list.md): the one place a chart's raw pads
// become the chord's four lanes. It runs before the flam step and the disco
// flip, as the game converts at load and does those two later. Only the lane
// and the tom or cymbal mark change, so ghosts and accents stay on their
// notes. A cymbal is kept only where cymbal_counts says it counts, so with
// Pro Drums off a converted chart has none, as any other chart.
//
// The four-lane branch ends with the game's all-toms step (NL-74): on a chart
// with neither flag up every pad is a tom. A .chart reader's pads are toms
// already on such a chart, so the step changes only a .mid's.
Chord convert_drum_pads(const TickPads& raw, const DrumChartFlags& flags, bool pro) {
    Chord chord = raw.four;
    const std::optional<ChordNote>& fifth = raw.fifth.at(NoteColor::Green);
    const bool fourth_pad = chord.at(NoteColor::Green).has_value();
    const auto set_cymbal = [&](NoteColor color, bool cymbal) {
        std::optional<ChordNote>& note = chord.at(color);
        if (!note) return;
        note->cymbaltype = cymbal && cymbal_counts(color, pro) ? NoteCymbalType::Cymbal
                                                               : NoteCymbalType::Normal;
    };
    const bool five_lane = (flags.fifth_pad && !flags.pro_marks) || flags.five_lane_key;
    if (five_lane) {
        // Yellow and the fourth pad are cymbals, red and blue toms.
        set_cymbal(NoteColor::Yellow, true);
        set_cymbal(NoteColor::Blue, false);
        set_cymbal(NoteColor::Green, true);
    } else if (!flags.fifth_pad && !flags.pro_marks) {
        set_cymbal(NoteColor::Yellow, false);
        set_cymbal(NoteColor::Blue, false);
        set_cymbal(NoteColor::Green, false);
    }
    // Otherwise the four-lane branch keeps the first four pads as the reader
    // marked them. In both branches the fifth pad is a tom, on the green lane,
    // or on the blue lane when the fourth pad shares its tick.
    if (!fifth) return chord;
    ChordNote moved = *fifth;
    moved.cymbaltype = NoteCymbalType::Normal;
    moved.colortype = fourth_pad ? NoteColor::Blue : NoteColor::Green;
    if (!chord.at(moved.colortype)) {
        chord.insert_note(moved);
        return chord;
    }
    // A tick that also holds a blue note puts two blue toms on one lane. The
    // chord keeps the chart's own blue note. In the four-lane branch the game
    // gives the fifth pad that blue note's marks, so the two are alike. In the
    // five-lane branch the game keeps both until its track finalizer merges
    // them into one, or-ing their flags (NL-77, Q-14; probe D's 35 notes), so
    // the kept note also takes the fifth pad's ghost or accent, through the
    // chord's one owner of a second mark.
    if (five_lane) {
        if (moved.is_accent()) chord.apply_accent(moved.colortype);
        else if (moved.is_ghost()) chord.apply_ghost(moved.colortype);
    }
    return chord;
}

// Emit the tick's chord as a sequence timestamp, shared by both parsers. The
// raw pads are converted first (convert_drum_pads), by the chart's `flags`.
// apply_flam is true only on the .mid path: MIDI charts carry a flam marker
// that converts the chord, while the .chart format has no such marker, so
// ChartParser always passes false.
// A disco section swaps red and yellow only under Pro Drums. That rule lives
// here, once, for both parsers (finding 250): `pro` is the Pro Drums setting
// and `in_disco` says whether this difficulty's disco section is open.
void emit_chord_timestamp(Song& song, const TickPads& raw, const DrumChartFlags& flags,
                          int64_t tick, bool apply_flam, bool pro, bool in_disco, bool solo) {
    Chord chord = convert_drum_pads(raw, flags, pro);
    if (apply_flam) chord.apply_flam_conversion();
    if (pro && in_disco) chord.apply_disco_flip();
    SongTimestamp ts;
    ts.chord = std::move(chord);
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

namespace {
// The two ends of the no-notes sentence; no_notes_message writes them and
// no_notes_part reads them back.
constexpr std::string_view kNoNotesHead = "No ";
constexpr std::string_view kNoNotesTail = " in this chart.";
}  // namespace

std::string no_notes_message(Difficulty difficulty, bool prodrums) {
    return std::string(kNoNotesHead) + difficulty_name(difficulty) +
           (prodrums ? " Pro Drums" : " Drums") + " notes" + std::string(kNoNotesTail);
}

std::optional<std::string> no_notes_part(const std::string& message) {
    const size_t ends = kNoNotesHead.size() + kNoNotesTail.size();
    if (message.size() <= ends || !starts_with(message, kNoNotesHead) ||
        !ends_with(message, kNoNotesTail))
        return std::nullopt;
    return message.substr(kNoNotesHead.size(), message.size() - ends);
}

NoNotesError::NoNotesError(Difficulty difficulty, bool prodrums)
    : ChartFileError(ErrorKind::NoNotes, no_notes_message(difficulty, prodrums)) {}

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
    return name_or_unknown(std::move(title), kOldTitlePlaceholder);
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

std::string display_charter(std::string_view charter) {
    // The cleaned charter, then the same missing-name test as the artist.
    return name_or_unknown(clean_name(charter), kUnknownCharter);
}

std::string display_album(std::string_view album) { return clean_name(album); }

std::string artist_or_unknown(std::string artist) {
    return artist.empty() ? std::string(kUnknownArtist) : artist;
}

std::string charter_or_unknown(std::string charter) {
    return charter.empty() ? std::string(kUnknownCharter) : charter;
}

// ---- MidiParser ---------------------------------------------------------

namespace {

enum class MPhase { None, Time, Pre, PreDelayed, Notes, PostDelayed };

// What a MIDI message does to the parser, decided once when the message is
// classified and carried out later in its phase. A plain tagged struct, so a
// tick's handlers cost no allocation.
enum class MAct : uint8_t {
    None, Note, Kick, FillStart, StoreFillEnd, SpStart, SpEnd, Tom, Flam,
    Solo, Dynamics, Disco, Tempo, TimeSig, LaneStart, LaneEnd,
};

struct MOp {
    MPhase phase = MPhase::None;
    MAct act = MAct::None;
    NoteColor color = NoteColor::Kick;                  // Note, Tom
    NoteDynamicType dyn = NoteDynamicType::Normal;      // Note, Kick
    NoteCymbalType cymbal = NoteCymbalType::Normal;     // Tom
    bool flag = false;     // Kick: is2x; Flam/Solo/Disco: on; LaneStart/LaneEnd: double
    bool fifth_pad = false;  // Note: the fifth pad (TickPads::fifth)
    int64_t tick = 0;      // FillStart/StoreFillEnd/SpStart/SpEnd/Dynamics/Tempo/TimeSig/Lane*
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

MOp mop_note(NoteColor color, NoteDynamicType dyn, bool fifth_pad = false) {
    MOp op = mop(MPhase::Notes, MAct::Note);
    op.color = color;
    op.dyn = dyn;
    op.fifth_pad = fifth_pad;
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
// Difficulty enum order. All four difficulties share the one drum track
// (MidiFile::drums_track); each owns a block of five pitches starting at its
// kick (kick, then the four pads). Its 2x kick sits one below the kick
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

// A difficulty's pads sit on the pitches just above its kick: red, yellow,
// blue, the fourth pad and, this many above the kick, the fifth pad (TickPads
// names them). Clone Hero's pitch-to-type map says so
// (docs/audit/note-shuffle/step0/open2/midi_type_table.txt).
constexpr int kFifthPadAboveKick = 5;

// Whether `pitch` is the fifth pad of any difficulty in kDifficultyChartCodes.
// One on any difficulty raises the chart's first flag (DrumChartFlags), so it
// is read whichever difficulty is parsed.
constexpr bool is_midi_fifth_pad_pitch(int pitch) {
    for (const auto& d : kDifficultyChartCodes)
        if (pitch == d.kick_pitch + kFifthPadAboveKick) return true;
    return false;
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
// Drum lanes (Song::drum_lanes): a single lane and a double lane, held from
// note-on to note-off. The note-on's velocity says which difficulties show
// it (midi_lane_lowest_difficulty).
constexpr int kSingleLaneMarkerPitch = 126;
constexpr int kDoubleLaneMarkerPitch = 127;
constexpr int kMarkerPitches[] = {
    kSoloMarkerPitch,     kFlamMarkerPitch, kYellowTomMarkerPitch, kBlueTomMarkerPitch,
    kGreenTomMarkerPitch, kSpMarkerPitch,   kFillMarkerPitch,      kSingleLaneMarkerPitch,
    kDoubleLaneMarkerPitch,
};

// The lowest difficulty a .mid drum lane shows on, from its note-on
// velocity: Clone Hero's rule, which the user chose for the Preview's lanes.
// The lane shows on that difficulty and every one above it.
Difficulty midi_lane_lowest_difficulty(int velocity) {
    if (velocity >= 21 && velocity <= 30) return Difficulty::Easy;
    if (velocity >= 31 && velocity <= 40) return Difficulty::Medium;
    if (velocity >= 41 && velocity <= 50) return Difficulty::Hard;
    return Difficulty::Expert;
}

// Whether a lane whose lowest difficulty is `lowest` shows at `difficulty`.
// Difficulty lists the hardest first, so every difficulty at or before
// `lowest` in it is at least as hard.
bool midi_lane_shows_at(Difficulty lowest, Difficulty difficulty) {
    return static_cast<int>(difficulty) <= static_cast<int>(lowest);
}

constexpr int lowest_marker_pitch() {
    int lowest = kMarkerPitches[0];
    for (int pitch : kMarkerPitches)
        if (pitch < lowest) lowest = pitch;
    return lowest;
}
// The solo marker divides the pitches: no marker sits below it and no pad at
// or above it. So no pad pitch is also a marker, and the note-off gate
// (midi_note_is_read) drops every pad's note-off.
static_assert(lowest_marker_pitch() == kSoloMarkerPitch,
              "a marker below the solo marker would change the note-off gate");
constexpr int highest_pad_pitch() {
    int highest = 0;
    for (const auto& d : kDifficultyChartCodes)
        if (d.kick_pitch + kFifthPadAboveKick > highest) highest = d.kick_pitch + kFifthPadAboveKick;
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
// from difficulty_chart_codes. The kick and its pads (kFifthPadAboveKick) are
// the note pitches, and the 2x kick sits one below the kick. The rest are the
// shared markers (is_midi_marker_pitch). A pitch outside this set belongs to
// another difficulty (or to another instrument) and is not read into a chord,
// so Expert's 95 is never read below Expert: Clone Hero reads each 2x kick
// only into its own difficulty (D20; 0x2155050 at 0x21555CD).
bool is_handled_note(int note, int base, int kick2x) {
    if (note >= base && note <= base + kFifthPadAboveKick) return true;
    if (note == kick2x) return true;
    return is_midi_marker_pitch(note);
}

// Whether the MIDI parser reads a note message at this pitch. `on` is
// Message::is_note_on. A note-on is read on any handled pitch, and on any
// difficulty's fifth pad, which midi_drum_chart_flags looks for; only a marker
// is read on a note-off: a pad or 2x kick note-off is dropped (see the
// static_assert beside kMarkerPitches). MidiParser::optype and the lean read's
// filter (parse_mid_lean) both ask here.
bool midi_note_is_read(int pitch, bool on, int base, int kick2x) {
    if (on && is_midi_fifth_pad_pitch(pitch)) return true;
    return on ? is_handled_note(pitch, base, kick2x) : is_midi_marker_pitch(pitch);
}

// The chart's two flags (DrumChartFlags), from the whole drum track before any
// chord is read: a note-on of any difficulty's fifth pad raises the first and
// a tom marker the second, as Clone Hero's .mid drum reader raises them
// (docs/clone-hero/code/6142/mid_drum_reader_2155050.md, lines 124 to 147 and
// 270 of its listing).
//
// The two flags do not start at zero in a .mid: the reader starts them from
// song.ini's five_lane_drums and pro_drums (`keys`), and the track can only
// raise them (mid_drum_reader_2155050.asm lines 80 to 99, CR-18 in
// docs/clone-hero/01-reading-a-chart.md). pro_drums's
// part in that is read from the game's code and not yet seen in a run: no
// probe has played a .mid with a fifth pad and pro_drums = True.
DrumChartFlags midi_drum_chart_flags(const MidiTrack& track, const ChartDrumKeys& keys) {
    DrumChartFlags flags;
    flags.fifth_pad = keys.five_lane_drums;
    flags.pro_marks = keys.pro_drums;
    flags.five_lane_key = keys.five_lane_drums;
    for (const Message& msg : track.messages) {
        if (!msg.is_note_on()) continue;
        if (is_midi_fifth_pad_pitch(msg.note)) flags.fifth_pad = true;
        if (msg.note == kYellowTomMarkerPitch || msg.note == kBlueTomMarkerPitch ||
            msg.note == kGreenTomMarkerPitch)
            flags.pro_marks = true;
    }
    return flags;
}

class MidiParser {
public:
    MidiParser(const core::Rules& rules, FillDeadlineRule fills) : rules_(rules), fills_rule_(fills) {}
    Song parse(const MidiFile& mid, bool pro, bool bass2x, Difficulty difficulty,
               const ChartDrumKeys& keys);

private:
    const core::Rules& rules_;
    FillDeadlineRule fills_rule_;

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
    // A drum lane's note-on and note-off. The lane is recorded whole at its
    // note-off and lit in the loader tail (light_drum_lanes). A note-off
    // with no lane open records nothing, and a lane never closed is dropped,
    // as an open fill is.
    void op_lane_start(int64_t tick, bool double_lane) { lane_start_tick_[double_lane] = tick; }
    void op_lane_end(int64_t tick, bool double_lane) {
        std::optional<int64_t>& start = lane_start_tick_[double_lane];
        if (start) lanes_.push_back({*start, tick, double_lane, {}});
        start.reset();
    }
    // A pad note. The fifth pad takes no tom marker: the conversion makes it
    // a tom whatever the markers say (convert_drum_pads).
    void op_note(NoteColor color, NoteDynamicType dyn, bool fifth_pad) {
        ChordNote& note = pads_.pads(fifth_pad).add_note(color);  // may throw ChartFileError
        note.dynamictype = dynamics_enabled_ ? dyn : NoteDynamicType::Normal;
        // A ghost or accent velocity before the tag: Clone Hero prices it as
        // plain, and the Dynamics tab reports how many there were.
        if (!dynamics_enabled_ && dyn != NoteDynamicType::Normal) ++marks_before_tag_;
        if (!fifth_pad && cymbal_counts(color, mode_pro_))
            note.cymbaltype = flag_cymbals_[static_cast<int>(color) - 1];
    }
    // A kick, a 2x kick when `is2x`. Chord::add_kick merges a 2x kick and a
    // normal kick on one tick into one kick, so a mark before the tag counts
    // once for it, as for any other note.
    void op_kick(NoteDynamicType dyn, bool is2x) {
        pads_.four.add_kick(is2x, dynamics_enabled_ ? dyn : NoteDynamicType::Normal);  // may throw
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
        if (!pads_.four.apply_2x_bass(mode_bass2x_) || !kick_mark_before_tag_) return;
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
    Difficulty difficulty_ = Difficulty::Expert;  // set by parse()
    DrumChartFlags flags_;  // set by parse() from midi_drum_chart_flags

    TickPads pads_;
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
    // The open single [0] and double [1] lane's note-on tick, and every lane
    // closed so far, handed on unlit after pass 2.
    std::array<std::optional<int64_t>, 2> lane_start_tick_{};
    std::vector<DrumLane> lanes_;
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
            // The difficulty's own note pitches come first: base is the kick,
            // the next four are Red/Yellow/Blue/Green (the fourth pad), and
            // the one after them the fifth pad, held apart in TickPads.
            // Clone Hero reads the kick's velocity exactly as it reads a
            // pad's: 127 is an accent, 1 is a ghost, and both score double.
            const NoteDynamicType vel_dyn =
                velocity == 127   ? NoteDynamicType::Accent
                : velocity == 1   ? NoteDynamicType::Ghost
                                  : NoteDynamicType::Normal;
            if (note == base_) return mop_kick(vel_dyn, false);
            if (note > base_ && note < base_ + kFifthPadAboveKick) {
                // base+1 -> Red(2), as 97 -> Red(2) on Expert.
                NoteColor color = static_cast<NoteColor>(note - base_ + 1);
                return mop_note(color, vel_dyn);
            }
            if (note == base_ + kFifthPadAboveKick)
                return mop_note(NoteColor::Green, vel_dyn, /*fifth_pad=*/true);
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
                case kSingleLaneMarkerPitch:
                case kDoubleLaneMarkerPitch: {
                    // A lane this difficulty does not show opens nothing, so
                    // its note-off closes nothing.
                    if (!midi_lane_shows_at(midi_lane_lowest_difficulty(velocity), difficulty_))
                        return {};
                    MOp op = mop_tick(MPhase::Pre, MAct::LaneStart, tick);
                    op.flag = note == kDoubleLaneMarkerPitch;
                    return op;
                }
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
            case kSingleLaneMarkerPitch:
            case kDoubleLaneMarkerPitch: {
                MOp op = mop_tick(MPhase::Pre, MAct::LaneEnd, tick);
                op.flag = note == kDoubleLaneMarkerPitch;
                return op;
            }
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
        case MAct::Note: op_note(op.color, op.dyn, op.fifth_pad); break;
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
        case MAct::LaneStart: op_lane_start(op.tick, op.flag); break;
        case MAct::LaneEnd: op_lane_end(op.tick, op.flag); break;
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
    pads_ = TickPads();
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

    if (pads_.count())
        emit_chord_timestamp(*song_, pads_, flags_, tick, flag_flam_, mode_pro_, flag_disco_,
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
                       Difficulty difficulty, const ChartDrumKeys& keys) {
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;
    base_ = difficulty_base_pitch(difficulty);
    kick2x_pitch_ = difficulty_chart_codes(difficulty).kick2x_pitch();
    mix_digit_ = difficulty_chart_codes(difficulty).mix_digit;
    difficulty_ = difficulty;

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

    // The coda tick, before the drum pass places fills, because the events
    // track may come after the drum track in the file. Clone Hero's events
    // reader (0x21544E0) trims the text and keeps the first coda; a body in
    // brackets loses one pair. Pass 3 reads the same track for other events.
    std::optional<int64_t> coda_tick;
    if (const MidiTrack* const events = mid.events_track()) {
        int64_t at = 0;
        for (const Message& msg : events->messages) {
            at += msg.time;
            if (msg.str_attr != Message::StrAttr::Text) continue;
            std::string_view body = trim_view(msg.str);
            if (body.size() >= 2 && body.front() == '[' && body.back() == ']')
                body = body.substr(1, body.size() - 2);
            if (is_coda_event(body)) {
                coda_tick = at;
                break;
            }
        }
    }

    // Pass 2: the drum track.
    if (const MidiTrack* const drums = mid.drums_track()) {
        const MidiTrack& track = *drums;
        flags_ = midi_drum_chart_flags(track, keys);
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
        lane_start_tick_ = {};
        lanes_.clear();
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
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats, coda_tick, fills_rule_);
        song.dynamics_enabled = dynamics_enabled_;
    }

    // Pass 3: practice sections and the end event, which live on the events
    // track as bracketed text metas. Sections are sorted once at the end; the
    // first end event is kept.
    if (const MidiTrack* const events = mid.events_track()) {
        elapsed = 0;
        for (const Message& msg : events->messages) {
            elapsed += msg.time;
            if (msg.str_attr != Message::StrAttr::Text) continue;
            const std::string& s = msg.str;
            if (s.size() < 2 || s.front() != '[' || s.back() != ']') continue;
            const std::string_view body = std::string_view(s).substr(1, s.size() - 2);
            if (is_end_event(body) && !song.end_event_tick.has_value())
                song.end_event_tick = elapsed;
            std::string name;
            if (section_name_of(body, SectionReader::Mid, &name))
                song.practice_sections.push_back({elapsed, name});
        }
    }
    sort_practice_sections(song.practice_sections);
    song.last_note_end_tick = mid.last_note_end_tick();
    song.solo_sections = find_solo_sections(song.sequence);
    // Unlit: the loader tail picks their pads (finish_loaded_song).
    song.drum_lanes = std::move(lanes_);

    song.check_activations(rules_, fills_rule_);
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
    SpStart, SpEnd, FillStart, Lane,
};

struct COp {
    CPhase phase = CPhase::None;
    CAct act = CAct::None;
    NoteColor color = NoteColor::Kick;  // Note, Accent, Ghost, Cymbal
    bool flag = false;                  // Disco, Solo: on; Lane: double
    bool fifth_pad = false;             // Note, Accent, Ghost: the fifth pad (TickPads::fifth)
    int64_t a = 0;   // Tempo/TimeSig/SpStart/FillStart/Lane: tick; SpEnd: start
    int64_t b = 0;   // SpStart/FillStart/SpEnd/Lane: end tick
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

COp cop_color(CPhase phase, CAct act, NoteColor color, bool fifth_pad = false) {
    COp op = cop(phase, act);
    op.color = color;
    op.fifth_pad = fifth_pad;
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
    ChartParser(const core::Rules& rules, FillDeadlineRule fills) : rules_(rules), fills_rule_(fills) {}
    Song parse(const std::vector<uint8_t>& data, bool pro, bool bass2x,
               Difficulty difficulty, const ChartDrumKeys& keys);

private:
    const core::Rules& rules_;
    FillDeadlineRule fills_rule_;

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
    // An S 65 or S 66 line: the lane is recorded whole and lit in the loader
    // tail (light_drum_lanes).
    void op_lane(int64_t start, int64_t end, bool double_lane) {
        lanes_.push_back({start, end, double_lane, {}});
    }
    void op_sp_start(int64_t start, int64_t end) {
        sp_start_tick_ = start;
        sp_end_tick_ = end;
    }
    void op_sp_end(int64_t start, int64_t end) {
        close_sp_phrase(*song_, start, end);
        sp_end_tick_.reset();
    }
    void op_solo(bool on) { flag_solo_ = on; }
    void op_note(NoteColor color, bool fifth_pad) { pads_.pads(fifth_pad).add_note(color); }
    void op_kick(bool is2x) { pads_.four.add_kick(is2x, NoteDynamicType::Normal); }
    void op_accent(NoteColor color, bool fifth_pad) { pads_.pads(fifth_pad).apply_accent(color); }
    void op_ghost(NoteColor color, bool fifth_pad) { pads_.pads(fifth_pad).apply_ghost(color); }
    void op_cymbal(NoteColor color) { pads_.four.apply_cymbal(color); }

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
    // The events section's first end event (Song::end_event_tick).
    std::optional<int64_t> event_end_tick_;
    // The first coda event of any events section, which place_authored_fills
    // reads. Clone Hero keeps it on the chart, so a later [Events] section
    // never replaces it (0x2151830's null test at 0x21523A7).
    std::optional<int64_t> coda_tick_;
    // The latest note end over every section (Song::last_note_end_tick).
    std::optional<int64_t> last_note_end_tick_;
    // The chart's two flags, from every drum section (load_sections).
    DrumChartFlags flags_;

    TickPads pads_;
    std::vector<COp> ops_;  // one tick's handlers, reused tick to tick
    bool flag_solo_ = false;
    bool flag_disco_ = false;
    std::optional<int64_t> sp_start_tick_;
    std::optional<int64_t> sp_end_tick_;
    std::vector<AuthoredFill> fills_;  // every authored fill, placed after the section
    std::vector<DrumLane> lanes_;      // every drum lane, handed on unlit
};

// A .chart pad: the lane the reader puts it on, and whether it is the fifth
// pad, which TickPads holds apart from the other four.
struct ChartPad {
    NoteColor color;
    bool fifth_pad = false;
};

// The pad of a .chart note number, or nullopt for a number that is not a pad.
// N 4 is the fourth pad and N 5 the fifth (TickPads names them).
std::optional<ChartPad> chart_pad_color(int value) {
    switch (value) {
        case 1: return ChartPad{NoteColor::Red};
        case 2: return ChartPad{NoteColor::Yellow};
        case 3: return ChartPad{NoteColor::Blue};
        case 4: return ChartPad{NoteColor::Green};
        case 5: return ChartPad{NoteColor::Green, true};
        default: return std::nullopt;
    }
}

// A Pro Drums cymbal marker's note number is its pad's number plus this
// (N 66 marks pad 2).
constexpr int kChartCymbalMarkerBase = 64;

// The pad a .chart cymbal marker marks, or nullopt for a number that is not
// one. Which pads can carry a cymbal is allows_cymbals' rule; the fifth pad
// carries none. ChartParser::note_optype reads the marker, and
// ChartParser::load_sections counts it as the chart's Pro Drums mark
// (DrumChartFlags).
std::optional<NoteColor> chart_cymbal_marker_pad(int value) {
    const std::optional<ChartPad> pad = chart_pad_color(value - kChartCymbalMarkerBase);
    if (!pad || pad->fifth_pad || !allows_cymbals(pad->color)) return std::nullopt;
    return pad->color;
}

// Whether a .chart section is a difficulty's drum section, whichever
// difficulty is parsed.
bool is_chart_drum_section(std::string_view name) {
    for (Difficulty d : kAllDifficulties)
        if (name == difficulty_chart_codes(d).chart_section()) return true;
    return false;
}

// The .chart phrase numbers of a single and a double drum lane
// (Song::drum_lanes), written in the difficulty's own drum section.
constexpr int kChartSingleLanePhrase = 65;
constexpr int kChartDoubleLanePhrase = 66;

// Sets `bracket` to the first bracketed run in `line`, brackets included. The
// loop says which '[' and ']' pair up, and the test ".chart: section headers
// are found as the pinned table says" pins it. Returns false when there is none.
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
    // unterminated piece counts only when it is non-empty. The walk reads the
    // file's text as decode_chart_text_view gives it (parse/text_decode.h).
    std::string decoded;
    const std::string_view text = decode_chart_text_view(
        std::string_view(reinterpret_cast<const char*>(data.data()), data.size()), decoded);

    // Every line of every section is classified, so a number the reader
    // refuses throws wherever it sits. What is kept depends on the section:
    // [Song] and [SyncTrack] whole, the drum section as its tick lines,
    // [Events] as its practice sections and end event, any other section
    // nothing; every section also gives its latest note end. A section
    // counts once its '}' is read, and a later section of the same name
    // replaces an earlier one (the end event is the exception: the first one
    // in the file stays).
    enum class Keep { Whole, Drums, Events, Nothing };
    bool in_section = false;
    Keep keep = Keep::Nothing;
    bool in_drum_section = false;  // any difficulty's (is_chart_drum_section)
    std::optional<ChartSection> whole;
    std::vector<ChartLine> drums;
    std::vector<SongSection> events;
    std::optional<int64_t> events_end;
    // Every section's latest note end (an N line's tick plus its sustain,
    // Song::last_note_end_tick), by section name, so a later section of the
    // same name replaces an earlier one here too.
    std::string_view section_name;
    std::optional<int64_t> section_note_end;
    std::unordered_map<std::string_view, std::optional<int64_t>> section_note_ends;
    const auto take_note_end = [&](const ChartLine& l) {
        if (l.kind != LineKind::Note) return;
        const int64_t note_end = l.tick + l.length;
        if (!section_note_end.has_value() || note_end > *section_note_end)
            section_note_end = note_end;
    };
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
                    case Keep::Events:
                        event_sections_ = std::move(events);
                        // First end event over the whole file, as Clone Hero's
                        // setter keeps it: a later [Events] section adds none
                        // once one is set.
                        if (!event_end_tick_.has_value()) event_end_tick_ = events_end;
                        break;
                    case Keep::Nothing: break;
                }
                section_note_ends[section_name] = section_note_end;
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
                    ChartDataEntry entry(lhs, rhs, mix_digit);
                    take_note_end(entry.line);
                    whole->add(std::move(entry));
                } else if (keep == Keep::Events) {
                    std::string_view event;
                    const ChartLine l = classify_chart_line(lhs, rhs, mix_digit, &event);
                    take_note_end(l);
                    std::string name;
                    if (l.kind == LineKind::Event &&
                        section_name_of(event, SectionReader::Chart, &name))
                        events.push_back({l.tick, std::move(name)});
                    // A .chart may write the end event with or without its
                    // brackets; one pair comes off when both are there.
                    std::string_view body = event;
                    if (body.size() >= 2 && body.front() == '[' && body.back() == ']')
                        body = body.substr(1, body.size() - 2);
                    if (l.kind == LineKind::Event && is_end_event(body) && !events_end.has_value())
                        events_end = l.tick;
                    // Clone Hero's [Events] reader trims the quoted text,
                    // then takes off a leading '[' and, when there is one,
                    // the closing ']' (0x2151830 at 0x2151D36 to 0x2151D9C).
                    std::string_view coda_body = trim_view(event);
                    if (!coda_body.empty() && coda_body.front() == '[') {
                        coda_body.remove_prefix(1);
                        if (!coda_body.empty() && coda_body.back() == ']') coda_body.remove_suffix(1);
                    }
                    if (l.kind == LineKind::Event && is_coda_event(coda_body) && !coda_tick_.has_value())
                        coda_tick_ = l.tick;
                } else {
                    const ChartLine l = classify_chart_line(lhs, rhs, mix_digit, nullptr);
                    take_note_end(l);
                    if (keep == Keep::Drums && l.kind != LineKind::Prop) drums.push_back(l);
                    // The chart's two flags come from every difficulty's drum
                    // section, as Clone Hero's .chart loader shares them
                    // across its drum tracks (CR-18): a fifth-pad note raises
                    // the first and a cymbal marker the second.
                    if (in_drum_section && l.kind == LineKind::Note) {
                        const std::optional<ChartPad> pad = chart_pad_color(l.value);
                        if (pad && pad->fifth_pad) flags_.fifth_pad = true;
                        if (chart_cymbal_marker_pad(l.value)) flags_.pro_marks = true;
                    }
                }
            }
        } else {
            std::string_view bracket;
            if (!find_section_header(line, &bracket))
                throw ChartFileError("expected a [section] header");
            const std::string_view name = bracket.substr(1, bracket.size() - 2);
            in_section = true;
            section_name = name;
            section_note_end.reset();
            in_drum_section = is_chart_drum_section(name);
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
                events_end.reset();
            } else {
                keep = Keep::Nothing;
            }
        }
    }
    for (const auto& [name, note_end] : section_note_ends) {
        if (note_end.has_value() && (!last_note_end_tick_.has_value() || *note_end > *last_note_end_tick_))
            last_note_end_tick_ = note_end;
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
            if (e.value == kChartSingleLanePhrase || e.value == kChartDoubleLanePhrase) {
                COp op = cop_span(CPhase::Pre, CAct::Lane, tick, tick + e.length);
                op.flag = e.value == kChartDoubleLanePhrase;
                return op;
            }
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
    if (const std::optional<ChartPad> pad = chart_pad_color(value))
        return cop_color(N, CAct::Note, pad->color, pad->fifth_pad);
    // Whether a cymbal marker counts is cymbal_counts'. A marker that does
    // not count is read and dropped.
    if (const std::optional<NoteColor> pad = chart_cymbal_marker_pad(value)) {
        if (cymbal_counts(*pad, mode_pro_)) return cop_color(M, CAct::Cymbal, *pad);
        return {};
    }
    switch (value) {
        case 0: return cop_flag(N, CAct::Kick, false);
        // The 2x kick, read whatever the 2x Bass setting: it can take a
        // normal kick on its tick with it (D105).
        case 32: return cop_flag(N, CAct::Kick, true);
        case 34: return cop_color(M, CAct::Accent, NoteColor::Red);
        case 35: return cop_color(M, CAct::Accent, NoteColor::Yellow);
        case 36: return cop_color(M, CAct::Accent, NoteColor::Blue);
        case 37: return cop_color(M, CAct::Accent, NoteColor::Green);
        case 38: return cop_color(M, CAct::Accent, NoteColor::Green, /*fifth_pad=*/true);
        case 40: return cop_color(M, CAct::Ghost, NoteColor::Red);
        case 41: return cop_color(M, CAct::Ghost, NoteColor::Yellow);
        case 42: return cop_color(M, CAct::Ghost, NoteColor::Blue);
        case 43: return cop_color(M, CAct::Ghost, NoteColor::Green);
        case 44: return cop_color(M, CAct::Ghost, NoteColor::Green, /*fifth_pad=*/true);
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
        case CAct::Note: op_note(op.color, op.fifth_pad); break;
        case CAct::Kick: op_kick(op.flag); break;
        case CAct::Accent: op_accent(op.color, op.fifth_pad); break;
        case CAct::Ghost: op_ghost(op.color, op.fifth_pad); break;
        case CAct::Cymbal: op_cymbal(op.color); break;
        case CAct::SpStart: op_sp_start(op.a, op.b); break;
        case CAct::SpEnd: op_sp_end(op.a, op.b); break;
        case CAct::FillStart: op_fillstart(op.a, op.b); break;
        case CAct::Lane: op_lane(op.a, op.b, op.flag); break;
    }
}

void ChartParser::push_timestamp(int64_t tick, const ChartLine* first, const ChartLine* last) {
    pads_ = TickPads();

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
    pads_.four.apply_2x_bass(mode_bass2x_);
    run_phase(CPhase::NoteMods);

    // Phrase end: SP.
    if (sp_end_tick_.has_value() && tick >= *sp_end_tick_) {
        const int64_t start = sp_start_tick_.value_or(0);
        ops.insert(ops.begin(), cop_span(CPhase::Pre, CAct::SpEnd, start, *sp_end_tick_));
    }

    run_phase(CPhase::Pre);

    if (pads_.count())
        emit_chord_timestamp(*song_, pads_, flags_, tick, /*apply_flam=*/false, mode_pro_,
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

namespace {

// The chart's Resolution, read from [Song] (null when the chart has none).
// One number rule: std::stoll on the raw text, so leading digits are read and
// trailing junk is ignored. Text it can't read refuses the chart with a plain
// reason; a value at or below 0 is check_timing_maps's to refuse.
int64_t chart_resolution(const ChartSection* song_sec) {
    constexpr ErrorKind kRefused = ErrorKind::ChartTimingRefused;
    auto refuse = [&](const char* why) {
        return ChartFileError(kRefused, std::string(kResolutionRefusalPrefix) + why);
    };
    if (song_sec == nullptr) throw refuse("missing");
    const auto it = song_sec->prop_data.find("Resolution");
    if (it == song_sec->prop_data.end() || it->second.empty()) throw refuse("missing");
    try {
        return std::stoll(it->second.front().property_str);
    } catch (const std::invalid_argument&) {
        throw refuse("not a whole number");
    } catch (const std::out_of_range&) {
        throw refuse("too large");
    }
}

}  // namespace

Song ChartParser::parse(const std::vector<uint8_t>& data, bool pro,
                        bool bass2x, Difficulty difficulty, const ChartDrumKeys& keys) {
    // A disco marker counts only in the difficulty it names, so the reader
    // needs this difficulty's digit before it classifies any line.
    load_sections(data, difficulty_chart_codes(difficulty).mix_digit,
                  difficulty_chart_codes(difficulty).chart_section());
    // A .chart's two flags start at zero whatever song.ini says
    // (chart_loader_2161a80.asm lines 158 and 159, CR-18), so only
    // five_lane_drums reaches the branch here, and pro_drums does not.
    flags_.five_lane_key = keys.five_lane_drums;
    mode_pro_ = pro;
    mode_bass2x_ = bass2x;

    const auto song_it = sections_.find(std::string(kSongSection));
    const ChartSection* song_sec = song_it == sections_.end() ? nullptr : &song_it->second;
    const int64_t tick_resolution = chart_resolution(song_sec);

    Song song(tick_resolution);
    song_ = &song;

    // Offset is a decimal number of seconds, read from its raw text by the one
    // chart-number rule (parse_finite_number), integer or not, so "500ms" or
    // "nan" counts as absent, like Clone Hero's default 0.
    if (auto it = song_sec->prop_data.find("Offset");
        it != song_sec->prop_data.end() && !it->second.empty()) {
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
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats, coda_tick_, fills_rule_);
    }

    // Practice sections, in the file's order, which is not required to be
    // sorted; sort_practice_sections orders them.
    if (event_sections_.has_value()) {
        song.practice_sections = std::move(*event_sections_);
        sort_practice_sections(song.practice_sections);
    }
    song.end_event_tick = event_end_tick_;
    song.last_note_end_tick = last_note_end_tick_;
    song.solo_sections = find_solo_sections(song.sequence);
    // Unlit: the loader tail picks their pads (finish_loaded_song).
    song.drum_lanes = std::move(lanes_);

    // .chart ghosts and accents are explicit per-note flags (N 34-37 accent,
    // N 40-43 ghost), applied unconditionally, so a .chart has no opt-in
    // marker like MIDI's [ENABLE_CHART_DYNAMICS]. Dynamics are always on; the
    // Dynamics tab reads this flag.
    song.dynamics_enabled = true;
    song.check_activations(rules_, fills_rule_);
    return song;
}

}  // namespace

// ---- public loaders -----------------------------------------------------

namespace {

// The chord list outlives the parse (the engine and the Preview read it), and
// each parser sizes it from a count that can run high. So its spare room goes
// back once the parser's own buffers are freed. Every loader ends at
// load_songbytes_mid or load_songbytes_chart, whose tail (finish_loaded_song)
// calls this.
void trim_parsed_song(Song& song) {
    song.sequence.shrink_to_fit();
}

Song parse_mid_lean(const std::vector<uint8_t>& data, bool pro, bool bass2x,
                    Difficulty difficulty, const ChartDrumKeys& keys, const core::Rules& rules,
                    FillDeadlineRule fills) {
    // Only the notes MidiParser reads are decoded (midi_note_is_read).
    const int base = difficulty_base_pitch(difficulty);
    const int kick2x = difficulty_chart_codes(difficulty).kick2x_pitch();
    MidiLeanFilter filter;
    for (int pitch = 0; pitch < kMidiDataValues; ++pitch) {
        filter.note_on[pitch] = midi_note_is_read(pitch, true, base, kick2x);
        filter.note_off[pitch] = midi_note_is_read(pitch, false, base, kick2x);
    }
    const MidiFile mid = MidiFile::lean(data.data(), data.size(), filter);
    return MidiParser(rules, fills).parse(mid, pro, bass2x, difficulty, keys);
}

// The Note Shuffle switch, applied once a parser has finished the song: the
// fill, solo and Star Power passes inside the parsers read only ticks and
// flags, never a pad, so they see the same chords either way. The drum-lane
// pass does read pads, so it runs after this (finish_loaded_song).
void apply_shuffle_switch(Song& song, bool pro, bool noteshuffle) {
    if (!noteshuffle) return;
    if (apply_note_shuffle(song.sequence, pro) == NoteShuffleResult::GameFreezes)
        throw KindedError(ErrorKind::NoteShuffleFreezes, kNoteShuffleFreezeDetail);
}

// The tail both public byte loaders end with, whichever parser read the
// chart. The drum lanes are lit here, after the Note Shuffle switch, because
// the game's lanes hold notes and its shuffle recolours them (see
// light_drum_lanes; D112's 2026-10-17 correction says what was and was not
// read in the game). The switch is passed on too, because with Note Shuffle
// on the lanes deliberately leave the game's rule (D254).
void finish_loaded_song(Song& song, const ChartReading& reading) {
    apply_shuffle_switch(song, reading.prodrums, reading.noteshuffle);
    song.drum_lanes =
        light_drum_lanes(std::move(song.drum_lanes), song.sequence, reading.noteshuffle);
    trim_parsed_song(song);
}

}  // namespace

// The whole file's bytes become text through decode_chart_text
// (parse/text_decode.h), the one rule for chart text encodings.
std::map<std::string, std::string> read_song_ini_keys(const std::string& path) {
    const std::vector<uint8_t> raw = read_file_bytes(path);
    const std::string text = decode_chart_text(raw.data(), raw.size());

    std::map<std::string, std::string> keys;
    bool in_song_section = false;
    size_t pos = 0;
    while (pos <= text.size()) {
        size_t eol = text.find('\n', pos);
        std::string line = text.substr(pos, eol == std::string::npos ? std::string::npos
                                                                      : eol - pos);
        pos = (eol == std::string::npos) ? text.size() + 1 : eol + 1;

        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' ||
                                 line.back() == '\t'))
            line.pop_back();
        size_t a = line.find_first_not_of(" \t");
        if (a == std::string::npos) continue;
        line = line.substr(a);
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            std::string section = to_lower_ascii(line.substr(1, line.size() - 2));
            in_song_section = (section == "song");
            continue;
        }
        if (!in_song_section) continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = to_lower_ascii(line.substr(0, eq));
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
        std::string value = line.substr(eq + 1);
        size_t vb = value.find_first_not_of(" \t");
        value = (vb == std::string::npos) ? std::string() : value.substr(vb);
        keys[key] = value;
    }
    return keys;
}

namespace {

// The whole number `text` spells, read the way .NET's Int32.TryParse reads it
// by default (blanks around it and one leading sign allowed, nothing past 32
// bits), or nothing.
std::optional<int32_t> dotnet_int32(std::string_view text) {
    std::string_view s = trim_view(text);
    bool negative = false;
    if (!s.empty() && (s.front() == '+' || s.front() == '-')) {
        negative = s.front() == '-';
        s.remove_prefix(1);
    }
    if (s.empty()) return std::nullopt;
    int64_t magnitude = 0;
    for (const char c : s) {
        if (c < '0' || c > '9') return std::nullopt;
        magnitude = magnitude * 10 + (c - '0');
        if (magnitude > int64_t{std::numeric_limits<int32_t>::max()} + 1) return std::nullopt;
    }
    const int64_t value = negative ? -magnitude : magnitude;
    if (value > std::numeric_limits<int32_t>::max()) return std::nullopt;
    return static_cast<int32_t>(value);
}

// A song.ini's two drum keys. Clone Hero reads both through one bool getter
// (0x1D4480, called from ini_song_reader_1EFC60.asm lines 77 to 89, CR-18).
// This is that getter's text rule. The game's ini parser trims each value
// before the getter sees it (ini_parse_21695E0.md), so this trims too.
bool ini_drum_key_is_true(std::string_view value) {
    if (const std::optional<int32_t> number = dotnet_int32(value)) return *number != 0;
    return equals_ci(trim_view(value), "true");
}

// A .sng's two drum keys. Clone Hero reads both through its .sng bool getter
// (0x2642C0, called from sng_meta_reader_1EFF90.asm lines 46 to 56), which
// takes .NET's Boolean.TryParse and nothing else. This is that rule.
bool sng_drum_key_is_true(std::string_view value) { return equals_ci(trim_view(value), "true"); }

// The two keys from a key -> value lookup, whatever spells the keys, each
// read by `is_true`, the rule of the getter that reads that source.
template <typename Find>
ChartDrumKeys drum_keys_from(const Find& find, bool (*is_true)(std::string_view)) {
    ChartDrumKeys keys;
    if (const std::optional<std::string_view> v = find("five_lane_drums"))
        keys.five_lane_drums = is_true(*v);
    if (const std::optional<std::string_view> v = find("pro_drums")) keys.pro_drums = is_true(*v);
    return keys;
}

}  // namespace

ChartDrumKeys chart_drum_keys(const std::map<std::string, std::string>& ini) {
    return drum_keys_from(
        [&](std::string_view key) -> std::optional<std::string_view> {
            const auto it = ini.find(std::string(key));
            if (it == ini.end()) return std::nullopt;
            return std::string_view(it->second);
        },
        ini_drum_key_is_true);
}

ChartDrumKeys chart_drum_keys(const std::vector<std::pair<std::string, std::string>>& sng_metadata) {
    return drum_keys_from(
        [&](std::string_view key) -> std::optional<std::string_view> {
            // The last pair of a key wins, as a song.ini's last line does.
            std::optional<std::string_view> found;
            for (const auto& [k, v] : sng_metadata)
                if (equals_ci(k, key)) found = std::string_view(v);
            return found;
        },
        sng_drum_key_is_true);
}

ChartDrumKeys chart_drum_keys_beside(const std::string& notespath) {
    const std::string ini = find_song_ini(parent_folder(notespath));
    if (ini.empty()) return {};
    try {
        return chart_drum_keys(read_song_ini_keys(ini));
    } catch (const std::exception&) {
        return {};  // an unreadable song.ini sets neither key, as a missing one
    }
}

Song load_songbytes_mid(const std::vector<uint8_t>& data, const ChartDrumKeys& keys,
                        const ChartReading& reading, const core::Rules& rules,
                        FillDeadlineRule fills) {
    // The lean file is gone once parse_mid_lean returns.
    Song song = parse_mid_lean(data, reading.prodrums, reading.bass2x, reading.difficulty, keys,
                               rules, fills);
    finish_loaded_song(song, reading);
    return song;
}

Song load_songbytes_chart(const std::vector<uint8_t>& data, const ChartDrumKeys& keys,
                          const ChartReading& reading, const core::Rules& rules,
                          FillDeadlineRule fills) {
    // The parser and its sections are gone by the end of this statement.
    Song song = ChartParser(rules, fills)
                    .parse(data, reading.prodrums, reading.bass2x, reading.difficulty, keys);
    finish_loaded_song(song, reading);
    return song;
}

Song load_songbytes_mid(const std::vector<uint8_t>& data, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills) {
    return load_songbytes_mid(data, ChartDrumKeys{}, reading, rules, fills);
}

Song load_songbytes_chart(const std::vector<uint8_t>& data, const ChartReading& reading,
                          const core::Rules& rules, FillDeadlineRule fills) {
    return load_songbytes_chart(data, ChartDrumKeys{}, reading, rules, fills);
}

Song load_songpath_mid(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills) {
    return load_songbytes_mid(read_file_bytes(path), chart_drum_keys_beside(path), reading, rules,
                              fills);
}

Song load_songpath_chart(const std::string& path, const ChartReading& reading,
                         const core::Rules& rules, FillDeadlineRule fills) {
    return load_songbytes_chart(read_file_bytes(path), chart_drum_keys_beside(path), reading,
                                rules, fills);
}

namespace {

// The one notes reader for each container kind, whether the bytes come from
// disk in pieces or from a buffer in memory (the Preview reads the whole file
// once and shares it; its notes are picked out the same way). A .sng carries
// its song.ini keys in its metadata block, so the drum keys come from there.
Song load_container_sng(const ByteSource& src, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills) {
    const std::vector<uint8_t> head = sng_read_head(src);
    const ChartDrumKeys keys = chart_drum_keys(sng_read_metadata(head));
    const SngNotes notes = sng_read_notes(src, head);
    if (notes.format == ChartFormat::Mid)
        return load_songbytes_mid(notes.bytes, keys, reading, rules, fills);
    return load_songbytes_chart(notes.bytes, keys, reading, rules, fills);
}

Song load_container_srb(const ByteSource& src, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills) {
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
        is_mid = is_midi_header(notebytes.data(), notebytes.size());

    // SrbMetadata holds no song.ini drum keys, so both read as unset here.
    if (is_mid) return load_songbytes_mid(notebytes, reading, rules, fills);
    return load_songbytes_chart(notebytes, reading, rules, fills);
}

}  // namespace

Song load_songbytes_sng(const std::vector<uint8_t>& buf, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills) {
    return load_container_sng(memory_byte_source(buf), reading, rules, fills);
}

Song load_songbytes_srb(const std::vector<uint8_t>& buf, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills) {
    return load_container_srb(memory_byte_source(buf), reading, rules, fills);
}

// A container on disk is read in pieces: its header, then only the notes. The
// rest is audio and art, which the notes never need. ADR 0024 records how large
// a container can be: about 1 GB each for three .sng files in the library.
Song load_songpath_sng(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills) {
    return load_container_sng(file_byte_source(path), reading, rules, fills);
}

Song load_songpath_srb(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills) {
    return load_container_srb(file_byte_source(path), reading, rules, fills);
}

Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                              const ChartReading& reading, const core::Rules& rules, FillDeadlineRule fills) {
    return load_songpath_reading(memory_byte_source(bytes), path, reading, rules, fills);
}

Song load_songpath_reading(const ByteSource& src, const std::string& path,
                           const ChartReading& reading, const core::Rules& rules, FillDeadlineRule fills) {
    switch (chart_format_of(path)) {
        case ChartFormat::Mid:
            return load_songbytes_mid(read_all(src), chart_drum_keys_beside(path), reading, rules,
                                      fills);
        case ChartFormat::Chart:
            return load_songbytes_chart(read_all(src), chart_drum_keys_beside(path), reading,
                                        rules, fills);
        case ChartFormat::Sng: return load_container_sng(src, reading, rules, fills);
        case ChartFormat::Srb: return load_container_srb(src, reading, rules, fills);
        case ChartFormat::None: break;
    }
    throw KindedError(ErrorKind::ChartUnreadable, "unexpected chart type: " + path);
}

Song load_songpath(const std::string& path, const ChartReading& reading,
                   const core::Rules& rules, FillDeadlineRule fills) {
    return load_songpath_reading(file_byte_source(path), path, reading, rules, fills);
}

void require_notes(const Song& song, Difficulty difficulty, bool prodrums) {
    if (song.is_empty()) throw NoNotesError(difficulty, prodrums);
}

Song load_songpath_with_notes(const std::string& path, const ChartReading& reading,
                              const core::Rules& rules, FillDeadlineRule fills) {
    Song song = load_songpath(path, reading, rules, fills);
    require_notes(song, reading.difficulty, reading.prodrums);
    return song;
}

}  // namespace hydra
