#include "parse/midi.h"

#include <algorithm>

#include "core/winstr.h"  // read_file_bytes
#include "parse/timesig.h"

namespace hydra {
namespace {

using MType = Message::Type;

// mido splits the string-valued metas across two attribute names, and the
// split is load-bearing (see the header). These are the meta types that carry
// their string as `text`.
bool text_meta(int meta_type, MType* type) {
    switch (meta_type) {
        case 0x01: *type = MType::Text; return true;
        case 0x02: *type = MType::Copyright; return true;
        case 0x05: *type = MType::Lyrics; return true;
        case 0x06: *type = MType::Marker; return true;
        case 0x07: *type = MType::CueMarker; return true;
        default: return false;
    }
}

// These carry their string as `name`. 0x08 is deliberately absent: mido has no
// spec for it, so it arrives as an unknown meta with neither attribute.
bool name_meta(int meta_type, MType* type) {
    switch (meta_type) {
        case 0x03: *type = MType::TrackName; return true;
        case 0x04: *type = MType::InstrumentName; return true;
        case 0x09: *type = MType::DeviceName; return true;
        default: return false;
    }
}

// How many data bytes follow each channel status, by high nibble.
int channel_data_len(int high) {
    switch (high) {
        case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: return 2;
        case 0xC0: case 0xD0: return 1;
        default: return -1;
    }
}

uint32_t be32(const uint8_t* d, size_t at) {
    return (uint32_t(d[at]) << 24) | (uint32_t(d[at + 1]) << 16) |
           (uint32_t(d[at + 2]) << 8) | uint32_t(d[at + 3]);
}

int16_t be16(const uint8_t* d, size_t at) {
    return int16_t((uint16_t(d[at]) << 8) | uint16_t(d[at + 1]));
}

// mido decodes every meta string as latin-1 (its _charset): byte -> codepoint,
// which never raises and round-trips each byte. The result is stored as UTF-8
// so string comparisons behave the same regardless of codepage.
std::string decode_latin1(const uint8_t* p, size_t len) {
    std::string out;
    out.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        uint8_t b = p[i];
        if (b < 0x80) {
            out.push_back(static_cast<char>(b));
        } else {
            // Two-byte UTF-8 encoding of codepoint b (0x80..0xFF).
            out.push_back(static_cast<char>(0xC0 | (b >> 6)));
            out.push_back(static_cast<char>(0x80 | (b & 0x3F)));
        }
    }
    return out;
}

// A variable-length quantity, 7 bits per byte. Advances pos. Accumulates in
// 64 bits so a malformed 5+-byte quantity reads as mido reads it (Python
// integers do not wrap) instead of wrapping at 32 bits.
uint64_t read_varlen(const uint8_t* data, size_t& pos, size_t end) {
    uint64_t value = 0;
    while (pos < end) {
        uint8_t b = data[pos++];
        value = (value << 7) | (b & 0x7F);
        if (!(b & 0x80)) break;
    }
    return value;
}

// mido's MAX_MESSAGE_LENGTH. mido's read_bytes raises for any meta or sysex
// message longer than this, and nothing catches it, so mido refuses the whole
// file. Hydra follows mido on broken files (user decision 27, 2026-09-24).
// The cap also keeps `pos += length` from wrapping the read position.
constexpr uint64_t kMaxMessageLength = 1000000;

// A meta or sysex length. Throws MidiError, with mido's wording, past the cap.
uint64_t read_message_length(const uint8_t* data, size_t& pos, size_t end) {
    const uint64_t length = read_varlen(data, pos, end);
    if (length > kMaxMessageLength)
        throw MidiError("Message length " + std::to_string(length) +
                        " exceeds maximum length " + std::to_string(kMaxMessageLength));
    return length;
}

// Build the meta events hysong can act on; returns false to skip the rest.
bool meta_message(int meta_type, const uint8_t* payload, size_t len,
                  int64_t time, Message* out) {
    if (meta_type == 0x51 && len == 3) {
        out->type = MType::SetTempo;
        out->time = time;
        out->tempo = (uint32_t(payload[0]) << 16) |
                     (uint32_t(payload[1]) << 8) | uint32_t(payload[2]);
        return true;
    }

    if (meta_type == 0x58 && len >= 2) {
        out->type = MType::TimeSignature;
        out->time = time;
        out->numerator = payload[0];
        // Stored as a power of two, as mido. An exponent past 30 has no int
        // power, so it reads as 0, which the song parser refuses.
        out->denominator = timesig_denominator(payload[1]);
        return true;
    }

    MType type;
    if (text_meta(meta_type, &type)) {
        out->type = type;
        out->time = time;
        out->str = decode_latin1(payload, len);
        out->str_attr = Message::StrAttr::Text;
        return true;
    }
    if (name_meta(meta_type, &type)) {
        out->type = type;
        out->time = time;
        out->str = decode_latin1(payload, len);
        out->str_attr = Message::StrAttr::Name;
        return true;
    }

    return false;
}

}  // namespace

const char* message_type_name(Message::Type type) {
    switch (type) {
        case MType::NoteOn: return "note_on";
        case MType::NoteOff: return "note_off";
        case MType::SetTempo: return "set_tempo";
        case MType::TimeSignature: return "time_signature";
        case MType::Text: return "text";
        case MType::Copyright: return "copyright";
        case MType::Lyrics: return "lyrics";
        case MType::Marker: return "marker";
        case MType::CueMarker: return "cue_marker";
        case MType::TrackName: return "track_name";
        case MType::InstrumentName: return "instrument_name";
        case MType::DeviceName: return "device_name";
    }
    return "unknown";
}

MidiFile::MidiFile(const std::vector<uint8_t>& data) {
    parse(data.data(), data.size());
}

MidiFile::MidiFile(const uint8_t* data, size_t size) {
    parse(data, size);
}

MidiFile MidiFile::from_file(const std::string& path) {
    // read_file_bytes owns reading a whole file (UTF-8 paths, long paths,
    // files past 2 GB) and its "cannot open file: " error (D54).
    return MidiFile(read_file_bytes(path));
}

void MidiFile::parse(const uint8_t* data, size_t size) {
    if (size < 14 || data[0] != 'M' || data[1] != 'T' ||
        data[2] != 'h' || data[3] != 'd') {
        throw MidiError("not a MIDI file: missing MThd header");
    }

    uint32_t header_len = be32(data, 4);
    format = be16(data, 8);
    // ntracks = be16(data, 10);  // not needed: chunks are walked directly.
    int16_t division = be16(data, 12);

    if (division < 0) {
        // SMPTE timing. Charts are all ticks-per-beat, and treating an SMPTE
        // division as one would silently misplace every note.
        throw MidiError("SMPTE time division is not supported");
    }
    ticks_per_beat = division;

    size_t pos = 8 + header_len;
    while (pos < size) {
        if (pos + 4 > size ||
            data[pos] != 'M' || data[pos + 1] != 'T' ||
            data[pos + 2] != 'r' || data[pos + 3] != 'k') {
            // Unknown chunk: the length field still tells us how to skip.
            if (pos + 8 > size) break;
            pos += 8 + be32(data, pos + 4);
            continue;
        }

        uint32_t chunk_len = be32(data, pos + 4);
        size_t start = pos + 8;
        size_t end = std::min(start + chunk_len, size);
        tracks.push_back(parse_track(data, start, end));
        pos = start + chunk_len;
    }
}

MidiTrack MidiFile::parse_track(const uint8_t* data, size_t pos, size_t end) {
    MidiTrack track;
    // A channel event with running status is three bytes (delta, two data
    // bytes), the smallest message the reader emits, so bytes / 3 is a cheap
    // upper estimate that saves the vector's repeated regrowth on huge tracks.
    track.messages.reserve((end - pos) / 3);

    // Ticks accumulated since the last emitted message. Skipped events hand
    // their delta to whatever comes next, so absolute time is preserved.
    int64_t pending = 0;
    int running = 0;
    bool named = false;

    while (pos < end) {
        pending += static_cast<int64_t>(read_varlen(data, pos, end));  // delta time

        if (pos >= end) break;

        uint8_t b = data[pos];
        int status;
        if (b & 0x80) {
            status = b;
            ++pos;
            // A meta event must not become the running status (mido excludes
            // only 0xFF). Rock Band rips use running status for the channel
            // event right after a meta event, so 0xFF must not clobber it.
            if (b != 0xFF) running = b;
        } else if (running) {
            status = running;
        } else {
            // Running status with nothing to run from: malformed past here.
            break;
        }

        if (status == 0xFF) {
            if (pos >= end) break;
            int meta_type = data[pos++];
            const uint64_t length = read_message_length(data, pos, end);
            const uint8_t* payload = data + pos;
            size_t avail = end - pos;
            size_t plen = std::min<size_t>(length, avail);
            pos += length;

            Message msg;
            if (meta_message(meta_type, payload, plen, pending, &msg)) {
                // mido's MidiTrack.name is the FIRST track_name meta; some
                // charts carry extra 0x03 metas mid-track (e.g. "Drums" after
                // "PART DRUMS"), and the last one must not win.
                if (meta_type == 0x03 && !named) {
                    track.name = msg.str;
                    named = true;
                }
                track.messages.push_back(std::move(msg));
                pending = 0;
            }
            continue;
        }

        if (status == 0xF0 || status == 0xF7) {
            const uint64_t length = read_message_length(data, pos, end);
            pos += length;
            continue;
        }

        int high = status & 0xF0;
        int nbytes = channel_data_len(high);
        if (nbytes < 0) {
            // System-common byte we do not model; no length to resync on.
            break;
        }

        if (pos + static_cast<size_t>(nbytes) > end) break;
        uint8_t d1 = data[pos];
        uint8_t d2 = nbytes > 1 ? data[pos + 1] : 0;
        pos += nbytes;

        if (high == 0x90 || high == 0x80) {
            // clip=True in mido: data bytes are clamped, not rejected.
            Message& msg = track.messages.emplace_back();
            msg.type = (high == 0x90) ? MType::NoteOn : MType::NoteOff;
            msg.note = d1 < 128 ? d1 : uint8_t{127};
            msg.velocity = d2 < 128 ? d2 : uint8_t{127};
            msg.time = pending;
            pending = 0;
        }
    }

    return track;
}

}  // namespace hydra
