// Tiny MIDI builders shared by the parser tests.
//
// A unit case that wants to pin one parsing rule needs a real .mid byte
// stream, not a corpus file. smf() wraps a hand-written track into the
// smallest legal file the readers accept.

#ifndef HYDRA_TESTS_MIDI_UTIL_H
#define HYDRA_TESTS_MIDI_UTIL_H

#include <cstdint>
#include <string>
#include <vector>

namespace testmidi {

// The one writer of MThd/MTrk chunks in the tests (audit finding 286).
// A file at 480 ticks a beat: format 0 for one track, format 1 for several,
// each track's raw bytes (delta+event stream) wrapped in an MTrk chunk.
inline std::vector<uint8_t> smf_tracks(const std::vector<std::vector<uint8_t>>& tracks) {
    const uint8_t format = tracks.size() == 1 ? 0 : 1;
    const uint16_t count = static_cast<uint16_t>(tracks.size());
    std::vector<uint8_t> d = {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, format,
                              static_cast<uint8_t>(count >> 8), static_cast<uint8_t>(count),
                              0x01, 0xE0};  // div=480
    for (const std::vector<uint8_t>& track : tracks) {
        d.insert(d.end(), {'M', 'T', 'r', 'k'});
        const uint32_t n = static_cast<uint32_t>(track.size());
        for (int shift : {24, 16, 8, 0}) d.push_back(static_cast<uint8_t>(n >> shift));
        d.insert(d.end(), track.begin(), track.end());
    }
    return d;
}

// Assemble a minimal single-track SMF (format 0, 480 tpqn) from raw track
// bytes (delta+event stream, without the MTrk header).
inline std::vector<uint8_t> smf(const std::vector<uint8_t>& track) {
    return smf_tracks({track});
}

// A delta time as MIDI writes it: variable-length, 7 bits a byte, most
// significant first, every byte but the last with its top bit set.
inline std::vector<uint8_t> varlen(uint32_t ticks) {
    std::vector<uint8_t> out = {static_cast<uint8_t>(ticks & 0x7F)};
    while (ticks >>= 7) out.insert(out.begin(), static_cast<uint8_t>(0x80 | (ticks & 0x7F)));
    return out;
}

// `ev`, a delta-0 event from the builders below, moved `ticks` later than the
// event before it: its one-byte zero delta replaced by varlen(ticks).
inline std::vector<uint8_t> after(uint32_t ticks, const std::vector<uint8_t>& ev) {
    std::vector<uint8_t> out = varlen(ticks);
    out.insert(out.end(), ev.begin() + 1, ev.end());
    return out;
}

// A delta-0 meta track-name event, so the song parser can find "PART DRUMS".
inline std::vector<uint8_t> track_name(const std::string& name) {
    std::vector<uint8_t> ev = {0x00, 0xFF, 0x03,
                               static_cast<uint8_t>(name.size())};
    ev.insert(ev.end(), name.begin(), name.end());
    return ev;
}

// A delta-0 meta text event, for [ENABLE_CHART_DYNAMICS] and friends.
inline std::vector<uint8_t> text_event(const std::string& text) {
    std::vector<uint8_t> ev = {0x00, 0xFF, 0x01,
                               static_cast<uint8_t>(text.size())};
    ev.insert(ev.end(), text.begin(), text.end());
    return ev;
}

// A delta-0 set_tempo meta. The song parser needs a tick-0 tempo before it can
// build a bpm map, so every hand-built drum track starts with one.
inline std::vector<uint8_t> set_tempo(uint32_t usec_per_beat = 500000) {
    return {0x00, 0xFF, 0x51, 0x03,
            static_cast<uint8_t>(usec_per_beat >> 16),
            static_cast<uint8_t>(usec_per_beat >> 8),
            static_cast<uint8_t>(usec_per_beat)};
}

// A delta-0 note_on with an explicit status byte on channel 0.
inline std::vector<uint8_t> note_on(uint8_t note, uint8_t velocity) {
    return {0x00, 0x90, note, velocity};
}

// End of track, delta 0.
inline std::vector<uint8_t> end_of_track() {
    return {0x00, 0xFF, 0x2F, 0x00};
}

// Concatenate event chunks into one track byte stream.
inline std::vector<uint8_t> concat(
    const std::vector<std::vector<uint8_t>>& parts) {
    std::vector<uint8_t> out;
    for (const auto& p : parts) out.insert(out.end(), p.begin(), p.end());
    return out;
}

}  // namespace testmidi

#endif  // HYDRA_TESTS_MIDI_UTIL_H
