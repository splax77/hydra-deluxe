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

    uint8_t note = 0;          // note_on / note_off (0..127, clipped as mido)
    uint8_t velocity = 0;      // note_on / note_off (0..127, clipped as mido)

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

// The track names the song parser finds its tracks by (MidiFile::drums_track
// and MidiTrack::is_events). They are also the only names the reader
// recognizes when it picks a track's name (TrackNamePick in midi.cpp, D78), so
// the lookups and that choice share one list.
inline constexpr std::string_view kDrumsTrackName = "PART DRUMS";
inline constexpr std::string_view kEventsTrackName = "EVENTS";
inline constexpr std::string_view kRecognizedTrackNames[] = {kDrumsTrackName,
                                                             kEventsTrackName};

class MidiTrack {
public:
    // Chosen by MidiFile::parse_track (D78).
    std::string name;
    std::vector<Message> messages;

    // Whether the song parser reads practice sections from this track.
    bool is_events() const { return name == kEventsTrackName; }
};

// Which drum-track note messages MidiFile::lean keeps, one table by pitch for
// the messages Message::is_note_on accepts and one for the rest. The song
// parser fills it (load_songbytes_mid in parse/song.cpp).
struct MidiLeanFilter {
    bool note_on[128] = {};
    bool note_off[128] = {};

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
    // `filter` keeps, and an events track (MidiTrack::is_events) its text
    // metas. Every track keeps its name. A dropped message hands its delta
    // on, so every kept message sits at the tick it has in the full read, and
    // the file throws exactly where the full read throws.
    static MidiFile lean(const uint8_t* data, size_t size, const MidiLeanFilter& filter);

    // The track the song parser reads tempo and meter from.
    static constexpr size_t kTimingTrack = 0;

    // The track the song parser reads the drum chart from, or nullptr when
    // the file has none. MidiParser::parse and MidiFile::lean both ask here.
    const MidiTrack* drums_track() const;

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
