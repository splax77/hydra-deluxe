// MIDI reader — the C++ port of hydra/hymidi.py.
//
// A purpose-built reader for exactly what the chart parser consumes: it pulls
// the file into one byte buffer and walks it with an index, emitting only the
// events hysong can act on. Events it cannot match are dropped and their delta
// rolled into the next emitted event, so absolute tick totals are preserved.
//
// Shape mirrors hymidi so the parser port (Phase 2) maps across directly:
//     MidiFile     ticks_per_beat, tracks
//     MidiTrack    name, messages
//     Message      channel note_on/note_off, and the metas hysong reads
//
// The load-bearing meta-string split is preserved: text-metas carry a string
// under StrAttr::Text, name-metas under StrAttr::Name, and 0x08 carries neither
// — hysong matches MetaMessage(text=...), which must not fire on a track name.

#ifndef HYDRA_PARSE_MIDI_H
#define HYDRA_PARSE_MIDI_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "core/error_kind.h"

namespace hydra {

// Thrown for input that is not a usable MIDI file (mirrors hymidi's ValueError).
class MidiError : public KindedError {
public:
    explicit MidiError(const std::string& what) : KindedError(ErrorKind::ChartUnreadable, what) {}
};

// Whether `size` bytes at `data` start with a MIDI file's header tag. The one
// home of that test: the reader's header check and the .srb notes sniff
// (parse/song.cpp) both ask here.
bool is_midi_header(const uint8_t* data, size_t size);

// How many values a MIDI data byte holds: the MIDI file format keeps its top
// bit clear.
inline constexpr int kMidiDataValues = 128;

// One event. A single struct covers channel and meta messages, matching the
// dynamic shape hymidi produces; only the fields relevant to `type` are set.
struct Message {
    // Every kind of message the reader emits. The names mido gives them are
    // spelled by message_type_name().
    enum class Type : uint8_t {
        NoteOn, NoteOff, SetTempo, TimeSignature,
        // Metas whose string is mido's `text` attribute.
        Text, Copyright, Lyrics, Marker, CueMarker,
        // Metas whose string is mido's `name` attribute.
        TrackName, InstrumentName, DeviceName,
    };

    // Which string attribute a meta carries, if any. Text-metas and name-metas
    // are kept distinct because hysong pattern-matches on `text` specifically.
    enum class StrAttr : uint8_t { None, Text, Name };

    int64_t time = 0;          // delta ticks since the previous emitted event.

    Type type = Type::NoteOn;
    StrAttr str_attr = StrAttr::None;

    uint8_t note = 0;          // note_on / note_off (clipped as mido, clip_data_byte)
    uint8_t velocity = 0;      // note_on / note_off (clipped as mido, clip_data_byte)

    uint32_t tempo = 0;        // set_tempo (microseconds per quarter note)

    int numerator = 0;         // time_signature
    int denominator = 0;       // time_signature (already 2**b, as in mido;
                               // 0 when b is past 30, see parse/timesig.h)

    // Text/name payload, UTF-8 (latin-1 decoded). Only text and name metas
    // fill it; every other message leaves it empty, so it never allocates.
    std::string str;

    // Whether this note message starts a note. Any other note message ends
    // one, as in mido and Clone Hero.
    bool is_note_on() const { return type == Type::NoteOn && velocity > 0; }
};

// mido's name for a message type ("note_on", "set_tempo", "track_name", ...).
const char* message_type_name(Message::Type type);

// The track names the song parser finds its tracks by. is_drums_track_name
// and is_events_track_name are the only tests of them: MidiFile::drums_track
// and MidiFile::events_track ask them, and so does the reader when it picks a
// track's name (TrackNamePick in midi.cpp, D78), so the lookups and that
// choice always agree.
//
// The names are spelled as Clone Hero 1.1 spells them.
inline constexpr std::string_view kDrumsTrackNames[] = {"part drums", "part drum"};
inline constexpr std::string_view kEventsTrackName = "events";

// Whether `name` is a drum track name (kDrumsTrackNames) in any ASCII case.
// Case is ignored because Clone Hero 1.1 lowercases each track name
// (ToLowerInvariant at 0x2155FDF) before it compares it (0x2156134 and
// 0x21564A8), with no trim, so the game plays "PART DRUM" and "Part Drums"
// alike (D252). ASCII lowering gives the same answer for these names: track
// names are decoded as latin-1, and no non-ASCII character lowercases to one
// of their letters.
bool is_drums_track_name(std::string_view name);

// Whether `name` is the events track name (kEventsTrackName) in any ASCII
// case. Case is ignored because Clone Hero 1.1 compares each track name with
// it using OrdinalIgnoreCase (String.Equals with comparison type 5 at
// 0x2155F30 and 0x2155F36), so the game reads "EVENTS" and "Events" alike
// (D253). ASCII case folding gives the same answer for this name: track names
// are decoded as latin-1, and no non-ASCII character there changes case to
// one of its letters.
bool is_events_track_name(std::string_view name);

// The instrument track names Clone Hero 1.1 holds besides the drum names
// (kDrumsTrackNames): its string literals list them beside "part drum" and
// "part drums". Only MidiFile::last_note_end_tick reads them, through
// is_instrument_track_name. BEAT, VENUE and EVENTS are not instruments.
inline constexpr std::string_view kVocalsTrackName = "part vocals";
inline constexpr std::string_view kOtherInstrumentTrackNames[] = {
    "part bass",   "part bass ghl",   "part guitar", "part guitar coop", "part guitar coop ghl",
    "part guitar ghl", "part keys",   "part keys ghl", "part rhythm", "part rhythm ghl",
    kVocalsTrackName,
};

// Whether `name` is one of kOtherInstrumentTrackNames, in any ASCII case (as
// is_drums_track_name).
bool is_other_instrument_track_name(std::string_view name);

// Whether `name` is an instrument track name, a drum name or one of
// kOtherInstrumentTrackNames.
bool is_instrument_track_name(std::string_view name);

// Whether a note-on or note-off pitch on an instrument track is a gem or a
// vocal note above the guitar and drum range. The layouts live in
// GuitarGame_ChartFormats, docs/Chart-File-Formats/mid-format/Tracks:
// 5-Fret-Guitar.md, 6-Fret-Guitar.md and Drums.md put the lowest gem at 58
// (6-fret Easy Open) and the first marker (solo) at 103; the markers above
// it (Star Power, versus phrase, tom, fill, Big Rock Ending, tremolo and
// trill) are not notes. The pitches below 58 on a guitar track are animation,
// not notes.
inline bool is_gem_pitch(int pitch) { return pitch >= 58 && pitch < 103; }

// Whether a pitch is below the gems and still a note on a vocals track
// (Vocals.md puts the lowest vocal note at 36). Only a vocals track reads it
// (MidiFile::last_note_end_tick); on any other track these pitches are
// animation or range shifts.
inline bool is_low_vocal_pitch(int pitch) { return pitch >= 36 && pitch < 58; }

class MidiTrack {
public:
    // Chosen by MidiFile::parse_track (D78).
    std::string name;
    std::vector<Message> messages;
    // The tick of the track's last note-on or note-off of a gem pitch
    // (is_gem_pitch), whether or not the read kept it, or nullopt when the
    // track has none. last_low_note_tick is the same for is_low_vocal_pitch.
    // Both readers fill both, so MidiFile::last_note_end_tick reads the same
    // values from either.
    std::optional<int64_t> last_note_tick;
    std::optional<int64_t> last_low_note_tick;
};

// Which drum-track note messages MidiFile::lean keeps, one table by pitch for
// the messages Message::is_note_on accepts and one for the rest. The song
// parser fills it (load_songbytes_mid in parse/song.cpp).
struct MidiLeanFilter {
    bool note_on[kMidiDataValues] = {};
    bool note_off[kMidiDataValues] = {};

    bool keeps(const Message& msg) const {
        return (msg.is_note_on() ? note_on : note_off)[msg.note];
    }
};

class MidiFile {
public:
    // Parse from raw file bytes. Throws MidiError on invalid input.
    explicit MidiFile(const std::vector<uint8_t>& data);
    MidiFile(const uint8_t* data, size_t size);

    // The file as the song parser reads it, holding only what it reads from
    // each track's role: the timing track (kTimingTrack) keeps its tempo and
    // meter, the drum track (drums_track) its text metas and the notes
    // `filter` keeps, and the events track (events_track) its text metas.
    // Every track keeps its name, last_note_tick and last_low_note_tick. A dropped message hands its delta
    // on, so every kept message sits at the tick it has in the full read, and
    // the file throws exactly where the full read throws.
    static MidiFile lean(const uint8_t* data, size_t size, const MidiLeanFilter& filter);

    // The track the song parser reads tempo and meter from.
    static constexpr size_t kTimingTrack = 0;

    // The first track Clone Hero 1.1 reads as an instrument. Its instrument
    // loop starts here (mov r15d, 1 at 0x2155F8D), so the game never plays
    // track 0, and a single-track file has no drums in it (D253).
    static constexpr size_t kFirstInstrumentTrack = 1;

    // The track the song parser reads the drum chart from, or nullptr when
    // the file has none: the first track from kFirstInstrumentTrack, in file
    // order, whose name is_drums_track_name accepts. Clone Hero 1.1 keeps the
    // first too: its set of instruments already read (HashSet Add at
    // 0x21564D7) skips any later drum track, whichever drum name it has.
    // MidiParser::parse and MidiFile::lean both ask here.
    const MidiTrack* drums_track() const;

    // The track the song parser reads practice sections from, or nullptr
    // when the file has none: the first track from track 0, in file order,
    // whose name is_events_track_name accepts. Clone Hero 1.1 reads that one
    // track and no other: its search starts at track 0 (xor ebx, ebx at
    // 0x2155EE4) and leaves on the first match (0x2155F3D) (D253).
    // MidiParser::parse and MidiFile::lean both ask here.
    const MidiTrack* events_track() const;

    // The latest tick any note ends on over the instrument tracks (the song
    // parser's Song::last_note_end_tick), or nullopt when they hold no note.
    // A note ends at its note-off, so the latest note message on a track is
    // its latest end (a note left open counts at its note-on). A track counts
    // from kFirstInstrumentTrack when is_instrument_track_name accepts its
    // name and no earlier track holds the same instrument: Clone Hero 1.1
    // reads each instrument once (see drums_track), and the drum names are
    // one instrument, so the drums count through drums_track.
    std::optional<int64_t> last_note_end_tick() const;

    // Read and parse a file from disk. A file that cannot be read throws
    // read_file_bytes' error; one that is not a MIDI file throws MidiError.
    static MidiFile from_file(const std::string& path);

    int ticks_per_beat = 0;
    int format = 0;
    std::vector<MidiTrack> tracks;

private:
    MidiFile() = default;
    void parse(const uint8_t* data, size_t size);
    MidiTrack parse_track(const uint8_t* data, size_t pos, size_t end);
};

}  // namespace hydra

#endif  // HYDRA_PARSE_MIDI_H
