// Chart parsing.
//
// Turns a .mid/.chart/.sng/.srb file into a Song: a tick-ordered sequence of
// SongTimestamps (a Timecode + Chord + solo/SP/fill flags), plus the tempo and
// meter maps. The MidiParser and ChartParser live in the .cpp; callers use the
// load_songpath_* functions.
//
// Every load_songpath* and load_songbytes_* function takes a ChartReading
// (parse/chart_reading.h): which notes to read. It has no default, so every
// caller says which reading it means.

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
#include "core/song.h"
#include "core/timing.h"
#include "core/winstr.h"  // ByteSource
#include "parse/chart_reading.h"

namespace hydra {

// Difficulty and ChartReading live in parse/chart_reading.h. The Song class,
// its record types and FillDeadlineRule live in core/song.h.

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

// What no_notes_message's sentence says is missing, such as "Hard Pro Drums
// notes"; nullopt for a text that is not that sentence. The one reader of the
// sentence's wording, for a screen that names the missing part on its own.
std::optional<std::string> no_notes_part(const std::string& message);

// A chart with no notes at the asked difficulty. Its message is
// no_notes_message's sentence, and it is still a ChartFileError, so anything
// that catches chart problems catches it. Its kind is NoNotes, which the
// store remembers (remembered_failure); its sentence is shown as written.
class NoNotesError : public ChartFileError {
public:
    NoNotesError(Difficulty difficulty, bool prodrums);
};

// What a song with no usable name is called everywhere it is shown.
inline constexpr const char* kUnknownTitle = "(unknown)";

// The one fallback for a song's name. A blank name (an empty `name =`, a
// missing name key, an empty .sng/.srb name) and the placeholder
// "<unknown title>" both become kUnknownTitle. Library rows from older scans
// still hold those, so every place that shows a stored name reads it through
// this.
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

// The one cleaned charter every screen shows (D266 item 2): tags stripped and
// the ends trimmed. A missing charter reads kUnknownTitle, like a missing
// artist, whether it is empty, only tags or spaces, or the scan's
// kUnknownCharter placeholder. Stored text is unchanged.
std::string display_charter(std::string_view charter);

// The one cleaned album every screen shows: tags stripped and the ends
// trimmed, as for a charter. It has no placeholder, because the scan stores
// none for an album, so a blank album reads empty.
std::string display_album(std::string_view album);

// The placeholders the scan stores for a blank artist or charter. Neither
// reaches the screen: display_artist and display_charter show kUnknownTitle
// for them.
inline constexpr const char* kUnknownArtist = "<unknown artist>";
inline constexpr const char* kUnknownCharter = "<unknown charter>";

// What the scan stores for a blank artist or charter (an empty `artist =`, a
// missing key, an empty .sng or .srb field): the placeholder above.
// discover_charts applies both once, after the rescan cache, so cached rows
// from older scans are covered too. What a screen shows is display_artist's
// and display_charter's question, not this one.
std::string artist_or_unknown(std::string artist);
std::string charter_or_unknown(std::string charter);

// The raw detail a Note Shuffle freeze carries (its sentence is
// plain_error's, for ErrorKind::NoteShuffleFreezes).
inline constexpr const char* kNoteShuffleFreezeDetail =
    "Note Shuffle: Clone Hero's redraw never ends on this chart";

// ChartReading::noteshuffle is the Note Shuffle switch (D104). Off, the chart
// is read as written. On, load_songbytes_mid and load_songbytes_chart finish
// by shuffling the chords (apply_note_shuffle in parse/note_shuffle.h), and
// every other loader ends at one of those two. A chart the game freezes on
// throws KindedError(ErrorKind::NoteShuffleFreezes) instead of returning.
//
// `fills` is the fill rule the song will be scored under: a chart with no
// fills of its own gets the fills that game version generates
// (Song::check_activations). A caller holding a setting converts it with
// search/graph.h's fill_rule_for. No loader has a default for either, so a
// caller can't leave the rule out by accident. Tests name theirs once, in
// tests/load_util.h.
//
// kNotesOnlyFills is for a caller that reads only the chords (their pads,
// times and dynamics), never which chord carries a fill: a fill only marks a
// chord the chart already has, so either game version gives the same chords.
// Anything that reads SongTimestamp::activation_length passes the real rule.
inline constexpr FillDeadlineRule kNotesOnlyFills = FillDeadlineRule::Ch11;

// The [song] section of a song.ini as lower-cased key -> value, with the
// value's leading blanks trimmed. A key seen twice keeps its last value.
// Section and key names match in any case, and `;` and `#` start comments.
// The text is decode_chart_text's (parse/text_decode.h), so every key and
// value is valid UTF-8. Throws std::runtime_error when the file cannot be
// read. The library scan (app::read_metadata_ini), the Preview (delay) and
// the chart loaders (chart_drum_keys_beside) all read song.ini through here.
std::map<std::string, std::string> read_song_ini_keys(const std::string& path);

// The two song.ini keys that change how a drum chart is read: five_lane_drums
// and pro_drums. The drum reader in song.cpp says what each one does
// (midi_drum_chart_flags, ChartParser::parse); a .chart reads only the first.
// Both false is a chart with neither key, or one with no song.ini.
struct ChartDrumKeys {
    bool five_lane_drums = false;
    bool pro_drums = false;
};

// The two keys from a song.ini already read (read_song_ini_keys), or from a
// .sng's metadata pairs (sng_read_metadata), which hold the same keys. Clone
// Hero reads each source's values by its own rule; song.cpp holds one reader
// per source, each naming the game getter it follows.
ChartDrumKeys chart_drum_keys(const std::map<std::string, std::string>& ini);
ChartDrumKeys chart_drum_keys(const std::vector<std::pair<std::string, std::string>>& sng_metadata);

// The two keys from the song.ini in the folder of the notes file at
// `notespath` (find_song_ini). A folder with no song.ini, or one that cannot
// be read, gives neither key. Every loader given a .mid or .chart path reads
// its keys through here, so the GUI's analysis, the Preview and the tools all
// see the same ones.
ChartDrumKeys chart_drum_keys_beside(const std::string& notespath);

Song load_songpath_mid(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills);
Song load_songpath_chart(const std::string& path, const ChartReading& reading,
                         const core::Rules& rules, FillDeadlineRule fills);
Song load_songpath_sng(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills);
Song load_songpath_srb(const std::string& path, const ChartReading& reading,
                       const core::Rules& rules, FillDeadlineRule fills);

// A notes file's bytes and the song.ini keys that go with it. A caller
// holding bytes with no song.ini beside them, such as a test, uses the
// overloads below, which read them with neither key set.
Song load_songbytes_mid(const std::vector<uint8_t>& data, const ChartDrumKeys& keys,
                        const ChartReading& reading, const core::Rules& rules,
                        FillDeadlineRule fills);
Song load_songbytes_chart(const std::vector<uint8_t>& data, const ChartDrumKeys& keys,
                          const ChartReading& reading, const core::Rules& rules,
                          FillDeadlineRule fills);
Song load_songbytes_mid(const std::vector<uint8_t>& data, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills);
Song load_songbytes_chart(const std::vector<uint8_t>& data, const ChartReading& reading,
                          const core::Rules& rules, FillDeadlineRule fills);
// A whole .sng / .srb container's bytes, as read from disk.
Song load_songbytes_sng(const std::vector<uint8_t>& container, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills);
Song load_songbytes_srb(const std::vector<uint8_t>& container, const ChartReading& reading,
                        const core::Rules& rules, FillDeadlineRule fills);

// The file at `path` parsed from `bytes`, its contents already read by the
// caller: dispatch on path's extension like load_songpath, without touching
// the disk. The Preview uses it to read a .sng/.srb once and share the bytes
// between the notes and the audio. Its notes are picked out exactly as
// load_songpath picks them from the file.
Song load_songpath_from_bytes(const std::string& path, const std::vector<uint8_t>& bytes,
                              const ChartReading& reading, const core::Rules& rules,
                              FillDeadlineRule fills);

// Dispatch on the file extension (.mid/.chart/.sng/.srb, case-insensitive).
// A .sng or .srb is read in pieces: its header, then only the notes, never
// the audio and art that make up the rest of the file.
Song load_songpath(const std::string& path, const ChartReading& reading,
                   const core::Rules& rules, FillDeadlineRule fills);

// Throws NoNotesError when `song` has no notes. `difficulty` and `prodrums`
// are the ones the song was loaded with, so the sentence names them. For a
// caller that already holds a loaded song, such as the Preview loader.
void require_notes(const Song& song, Difficulty difficulty, bool prodrums);

// load_songpath, then require_notes: the chart at `path`, or NoNotesError
// when it has no notes under `reading`.
Song load_songpath_with_notes(const std::string& path, const ChartReading& reading,
                              const core::Rules& rules, FillDeadlineRule fills);

// The one dispatch on the extension of `path`, its bytes read from `src`:
// load_songpath passes the file (file_byte_source), load_songpath_from_bytes
// the bytes in hand. A .mid or .chart is read whole; a .sng or .srb only in
// the pieces its notes need. Tests pass a counting source to prove that. A
// .mid or .chart takes its drum keys from the song.ini beside `path`
// (chart_drum_keys_beside), never from `src`.
Song load_songpath_reading(const ByteSource& src, const std::string& path,
                           const ChartReading& reading, const core::Rules& rules,
                           FillDeadlineRule fills);

}  // namespace hydra

#endif  // HYDRA_PARSE_SONG_H
