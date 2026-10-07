// Chart parsing — the C++ port of hydra/hysong.py.
//
// Turns a .mid/.chart/.sng/.srb file into a Song: a tick-ordered sequence of
// SongTimestamps (a Timecode + Chord + solo/SP/fill flags), plus the tempo and
// meter maps. The MidiParser and ChartParser live in the .cpp; callers use the
// load_songpath_* functions.
//
// Every load_* function takes the difficulty to read and defaults to Expert.

#ifndef HYDRA_PARSE_SONG_H
#define HYDRA_PARSE_SONG_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/rules.h"
#include "core/timing.h"
#include "core/winstr.h"  // ByteSource

namespace hydra {

// Which charted difficulty to read. One chart carries all four: a .mid keeps
// them in the single "PART DRUMS" track at four pitch bases, a .chart keeps
// them in four named sections. Expert is the default everywhere, so passing
// nothing reads exactly what Hydra always read.
enum class Difficulty { Expert, Hard, Medium, Easy };

// "Expert" / "Hard" / "Medium" / "Easy" — the name used in the chartmode key
// and the user-facing error strings. It reads the difficulty's row in the one
// table (difficulty_chart_codes), so it shares that table's fallback.
const char* difficulty_name(Difficulty difficulty);

// Every difficulty, in enum order. UI lists and name lookups walk this
// instead of keeping their own copy of the four names.
inline constexpr Difficulty kAllDifficulties[] = {Difficulty::Expert, Difficulty::Hard,
                                                  Difficulty::Medium, Difficulty::Easy};

// How a chart file spells one difficulty's drums. One row per difficulty, held
// in one table in song.cpp; every parser rule that depends on the difficulty
// reads its row instead of keeping its own copy of Expert's values.
struct DifficultyChartCodes {
    // "Expert" / "Hard" / "Medium" / "Easy" (difficulty_name returns it).
    const char* name;
    // .mid: the kick's pitch. The four pads follow it (kick + 1 is red).
    int kick_pitch;
    // The N in a disco-flip marker `[mix N drums...]`. Clone Hero applies a
    // marker only to the difficulty it names (0x215C750, digit mapped by
    // 0x210D990).
    char mix_digit;

    // .mid: the 2x kick's pitch, always one below the kick. Clone Hero reads
    // 59, 71, 83 and 95, each into its own difficulty (0x2155050 at
    // 0x21555CD). Worked out here, the one place, instead of stored.
    constexpr int kick2x_pitch() const { return kick_pitch - 1; }

    // .chart: the difficulty's drum section, the name plus "Drums"
    // ("ExpertDrums", "HardDrums", ...).
    std::string chart_section() const { return std::string(name) + "Drums"; }
};

// The row for `difficulty`. An out-of-range value reads as Expert, as the
// parsers' old switches did.
const DifficultyChartCodes& difficulty_chart_codes(Difficulty difficulty);

// True for a .mid drum-track pitch the parser reads as a marker shared by
// every difficulty rather than as a note. The one list is the marker table in
// song.cpp (kMarkerPitches); the parser reads it, and this exports it.
bool is_midi_marker_pitch(int pitch);

// The difficulty whose name matches `name` in any case ("hard", "HARD" and
// "Hard" all give Hard). nullopt for anything else. The settings INI and
// hydra_replay's --difficulty both read names through this.
std::optional<Difficulty> difficulty_from_name(std::string_view name);

// "No Hard Pro Drums notes in this chart." The one wording for a chart that
// lacks the requested difficulty, used by analysis and the Preview loader.
std::string no_notes_message(Difficulty difficulty, bool prodrums);

// A chart with no notes at the asked difficulty. Its message is
// no_notes_message's sentence, and it is still a ChartFileError, so anything
// that catches chart problems catches it. Its kind is AlreadyPlain: the
// sentence is shown as written.
class NoNotesError : public ChartFileError {
public:
    NoNotesError(Difficulty difficulty, bool prodrums);
};

// What a song with no usable name is called everywhere it is shown.
inline constexpr const char* kUnknownTitle = "(unknown)";

// The one fallback for a song's name. A blank name (an empty `name =`, a
// missing name key, an empty .sng/.srb name) and the "<unknown title>" the
// metadata readers wrote before this existed both become kUnknownTitle.
// Library rows from older scans still hold those, so every
// place that shows a stored name reads it through this.
std::string title_or_unknown(std::string title);

// Removes Clone Hero rich-text tags: <color=...>, </color>, <b>, </b>, <i>,
// </i>, <size=...>, </size>, <u>, </u>, <s>, </s>, <sub>, </sub>, <sup>,
// </sup>, case-insensitive. Anything else in angle brackets is kept, including
// <color> with no value, a tag that never closes, and "<unknown artist>".
// The library search reads it as app::strip_rich_tags (app/library_query.h).
std::string strip_rich_tags(std::string_view text);

// The one cleaned song title every screen shows (D48, Q14): the rich-text
// tags stripped, spaces at either end trimmed, then title_or_unknown's
// fallback, so a title made only of tags or spaces reads kUnknownTitle.
std::string display_title(std::string_view title);

// The one cleaned artist every screen shows (D50 item 5, D56 item 2): tags
// stripped and the ends trimmed, as for a title. A missing artist reads
// kUnknownTitle, whether it is empty, only tags or spaces, or the scan's
// kUnknownArtist placeholder. Stored text is unchanged.
std::string display_artist(std::string_view artist);

// The one cleaned charter every screen shows: tags stripped and the ends
// trimmed. It has no fallback, so a charter made only of tags reads empty
// and the scan's kUnknownCharter placeholder shows as it is.
std::string display_charter(std::string_view charter);

// The placeholders the scan stores for a blank artist or charter. The stored
// artist placeholder never reaches the screen: display_artist shows
// kUnknownTitle for it. The charter placeholder shows as it is.
inline constexpr const char* kUnknownArtist = "<unknown artist>";
inline constexpr const char* kUnknownCharter = "<unknown charter>";

// What the scan stores for a blank artist or charter (an empty `artist =`, a
// missing key, an empty .sng or .srb field): the placeholder above.
// discover_charts applies both once, after the rescan cache, so cached rows
// from older scans are covered too. What a screen shows is display_artist's
// and display_charter's question, not this one.
std::string artist_or_unknown(std::string artist);
std::string charter_or_unknown(std::string charter);

// A timecode paired with a chord and gameplay modifiers, mirroring
// hysong.SongTimestamp.
struct SongTimestamp {
    Timecode timecode;
    Chord chord;
    bool flag_solo = false;
    bool flag_sp = false;
    std::optional<int64_t> activation_length;
    // On the note that ends an SP phrase (flag_sp == true), the tick the phrase
    // began at. The parser otherwise discards the phrase start once the end
    // flag is set; the Preview needs the whole span to shade the phrase, so it
    // is kept here. nullopt on every other note. Not part of the search or the
    // stored record — a display-only addition.
    std::optional<int64_t> sp_phrase_start;

    bool has_activation() const { return activation_length.has_value(); }
};

// A practice section ("Verse 2B", "Chorus 1"), from the chart's own section
// markers. Display only: nothing in the search or the stored record reads these.
struct SongSection {
    int64_t tick = 0;
    std::string name;
};

// One solo section: first and last positions (both inclusive) into
// Song::sequence. What makes a section: find_solo_sections in song.cpp.
struct SoloSection {
    size_t first = 0;
    size_t last = 0;
};

// How a refused-timing message starts. check_timing_maps and apply_timesig
// in song.cpp build their messages from these; the error's kind,
// ChartTimingRefused, is what picks its sentence.
inline constexpr std::string_view kResolutionRefusalPrefix = "the chart's resolution is ";
inline constexpr std::string_view kTimeSignatureRefusalPrefix = "the time signature at tick ";
inline constexpr std::string_view kTempoRefusalPrefix = "the tempo at tick ";

// Throws ChartFileError, naming the tick, unless every measure in these maps
// lasts a positive, finite time: resolution above 0, every measure at least
// one tick long, every tempo a finite BPM above 0 (a .mid tempo of 0
// microseconds per beat is named infinite). Song::build_timing calls it
// first, so both parsers pass through it.
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes);

// The meter a chart has before its first time signature: 4/4. Song's
// constructor writes it through apply_timesig, and the Preview's
// PreviewTimeSig reads it for a scene with no song.
inline constexpr int kDefaultTimeSigNumerator = 4;
inline constexpr int kDefaultTimeSigDenominator = 4;

// A parsed chart: the timestamp sequence plus the tempo/meter maps it was built
// from. Timing is snapshotted once the maps are complete (build_timing), which
// mirrors Python building timecodes only after the whole tempo track is read.
class Song {
public:
    // Starts at the default meter (kDefaultTimeSig*), written by apply_timesig
    // like every other meter.
    explicit Song(int64_t resolution);

    int64_t tick_resolution() const { return tick_resolution_; }

    // Tick-keyed meter (ticks per measure) and tempo (BPM) maps, filled during
    // parsing. std::map keeps them sorted, which the timing indexes rely on.
    std::map<int64_t, int64_t> tpm_changes;
    std::map<int64_t, double> bpm_changes;
    // The chart's own time signatures, tick -> (numerator, denominator),
    // recorded by the same parser call that writes tpm_changes. Display only
    // (the Preview's time box): the engine reads tpm_changes, and a length
    // alone cannot tell 6/8 from 3/4.
    std::map<int64_t, std::pair<int, int>> timesig_changes;

    std::vector<SongTimestamp> sequence;
    std::vector<std::string> features;
    bool dynamics_enabled = false;
    // .mid only (finding 64, D24). When the drum track's dynamics tag came
    // after at least one ghost- or accent-velocity note, the tag's tick and
    // how many such notes came before it (Clone Hero prices them as plain).
    // nullopt and 0 when the tag came before every marked note, or there is
    // no tag (dynamics_enabled says which). count_dynamics stores them.
    std::optional<int64_t> dynamics_late_tag_tick;
    int dynamics_marks_before_tag = 0;
    // .chart [Song] Offset, in seconds, when the file sets one. Only the
    // Preview reads it (to line the audio up); scoring works in chart time.
    std::optional<double> chart_offset_s;

    // Practice sections in tick order. A section marker can sit past the last
    // note, so these ticks are not bounded by the sequence.
    std::vector<SongSection> practice_sections;

    // Solo sections in song order. Both parsers fill it once the sequence is
    // finished; see find_solo_sections in song.cpp. Display and replay only;
    // nothing stores it.
    std::vector<SoloSection> solo_sections;

    // Snapshot the timing indexes from the current maps. Called once the tempo
    // track has been fully mapped and before any timecode is made. Timing
    // that can't measure time is refused here (check_timing_maps).
    void build_timing() {
        check_timing_maps(tick_resolution_, tpm_changes, bpm_changes);
        timing_.emplace(tick_resolution_, tpm_changes, bpm_changes);
    }
    const SongTiming& timing() const { return *timing_; }
    Timecode timecode(int64_t ticks) const { return timing_->timecode(ticks); }
    Timecode start_time() const { return timing_->timecode(0); }

    bool is_empty() const { return sequence.empty(); }

    // How many SP phrases the chart offers. No run can bank more bars than
    // this, so it's the natural clamp on how tall a search graph is worth
    // building.
    int sp_phrase_count() const;

    // How many notes the chart has, at the difficulty and drum options it was
    // loaded with: every chord's Chord::count(), added up. The one answer to
    // the chart's note total. The engine stores it on every path (the Notes
    // column), and the Dynamics tab's Totals are pinned against it by test.
    int note_count() const;

    // If the chart has no drum fills, synthesize them like Clone Hero would.
    void check_activations(const core::Rules& rules = core::default_rules());

private:
    int64_t tick_resolution_;
    std::optional<SongTiming> timing_;
};

// A time signature at `tick`: ticks per measure = resolution * 4 * num / den,
// and the signature itself for display, written together. The one owner of
// the meter rule: both parsers, Song's default and test fixtures call it. A
// numerator of 0 is ignored; a bottom number of 0 or less refuses the chart.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator);

Song load_songpath_mid(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty = Difficulty::Expert,
                       const core::Rules& rules = core::default_rules());
Song load_songpath_chart(const std::string& path, bool pro, bool bass2x,
                         Difficulty difficulty = Difficulty::Expert,
                         const core::Rules& rules = core::default_rules());
Song load_songpath_sng(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty = Difficulty::Expert,
                       const core::Rules& rules = core::default_rules());
Song load_songpath_srb(const std::string& path, bool pro, bool bass2x,
                       Difficulty difficulty = Difficulty::Expert,
                       const core::Rules& rules = core::default_rules());

Song load_songbytes_mid(const std::vector<uint8_t>& data, bool pro, bool bass2x,
                        Difficulty difficulty = Difficulty::Expert,
                        const core::Rules& rules = core::default_rules());
Song load_songbytes_chart(const std::vector<uint8_t>& data, bool pro, bool bass2x,
                          Difficulty difficulty = Difficulty::Expert,
                          const core::Rules& rules = core::default_rules());
// A whole .sng / .srb container's bytes, as read from disk.
Song load_songbytes_sng(const std::vector<uint8_t>& container, bool pro, bool bass2x,
                        Difficulty difficulty = Difficulty::Expert,
                        const core::Rules& rules = core::default_rules());
Song load_songbytes_srb(const std::vector<uint8_t>& container, bool pro, bool bass2x,
                        Difficulty difficulty = Difficulty::Expert,
                        const core::Rules& rules = core::default_rules());

// The file at `path` parsed from `bytes`, its contents already read by the
// caller: dispatch on path's extension like load_songpath, without touching
// the disk. The Preview uses it to read a .sng/.srb once and share the bytes
// between the notes and the audio. Its notes are picked out exactly as
// load_songpath picks them from the file.
Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                              bool pro, bool bass2x,
                              Difficulty difficulty = Difficulty::Expert,
                              const core::Rules& rules = core::default_rules());

// Dispatch on the file extension (.mid/.chart/.sng/.srb, case-insensitive).
// A .sng or .srb is read in pieces: its header, then only the notes, never
// the audio and art that make up the rest of the file.
Song load_songpath(const std::string& path, bool pro, bool bass2x,
                   Difficulty difficulty = Difficulty::Expert,
                   const core::Rules& rules = core::default_rules());

// Throws NoNotesError when `song` has no notes. `difficulty` and `prodrums`
// are the ones the song was loaded with, so the sentence names them. For a
// caller that already holds a loaded song, such as the Preview loader.
void require_notes(const Song& song, Difficulty difficulty, bool prodrums);

// load_songpath, then require_notes: the chart at `path`, or NoNotesError
// when it has no notes at `difficulty`.
Song load_songpath_with_notes(const std::string& path, bool pro, bool bass2x,
                              Difficulty difficulty = Difficulty::Expert,
                              const core::Rules& rules = core::default_rules());

// The one dispatch on the extension of `path`, its bytes read from `src`:
// load_songpath passes the file (file_byte_source), load_songpath_from_bytes
// the bytes in hand. A .mid or .chart is read whole; a .sng or .srb only in
// the pieces its notes need. Tests pass a counting source to prove that.
Song load_songpath_reading(const ByteSource& src, const std::string& path, bool pro,
                           bool bass2x, Difficulty difficulty = Difficulty::Expert,
                           const core::Rules& rules = core::default_rules());

}  // namespace hydra

#endif  // HYDRA_PARSE_SONG_H
