// Smoke + unit tests for parse/midi.
//
// Smoke: every .mid in the corpus must read without throwing and produce a
// sane track/event structure. Unit: the edge cases the corpus may not contain
// but a user's library might.

#include "doctest.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "core/strutil.h"
#include "core/winstr.h"
#include "parse/chart_files.h"
#include "parse/midi.h"
#include "parse/section_names.h"
#include "parse/song.h"
#include "corpus_util.h"
#include "midi_util.h"

namespace {

using corpus::json;

// Absolute-tick event view, built as JSON so a unit case can diff a parsed
// file against a literal expected stream in one CHECK.
json event_view(const hydra::MidiFile& mid) {
    json view = json::array();
    for (const auto& track : mid.tracks) {
        int64_t tick = 0;
        json events = json::array();
        for (const auto& m : track.messages) {
            using T = hydra::Message::Type;
            tick += m.time;
            const std::string name = hydra::message_type_name(m.type);
            const int note = m.note, velocity = m.velocity;
            if (m.type == T::NoteOn || m.type == T::NoteOff) {
                events.push_back({tick, name, note, velocity});
            } else if (m.type == T::SetTempo) {
                events.push_back({tick, name, m.tempo});
            } else if (m.type == T::TimeSignature) {
                events.push_back({tick, name, m.numerator, m.denominator});
            } else if (m.str_attr == hydra::Message::StrAttr::Text) {
                events.push_back({tick, name, "text", m.str});
            } else if (m.str_attr == hydra::Message::StrAttr::Name) {
                events.push_back({tick, name, "name", m.str});
            }
        }
        view.push_back(std::move(events));
    }
    return view;
}

// smf() lives in midi_util.h, shared with the other tests that build files.
using testmidi::smf;

// The detail the full reader refuses `file` with. The case fails when the
// file is read, or refused as anything but an unreadable chart.
std::string midi_refusal(const std::vector<uint8_t>& file) {
    try {
        const hydra::MidiFile mid(file);
    } catch (const hydra::MidiError& e) {
        CHECK(e.kind() == hydra::ErrorKind::ChartUnreadable);
        return e.what();
    }
    FAIL("the file was read, not refused");
    return {};
}

// The four-letter chunk tag at byte `at` of a file the test writer made.
std::string chunk_tag(const std::vector<uint8_t>& file, size_t at) {
    REQUIRE(file.size() >= at + 4);
    return std::string(file.begin() + static_cast<std::ptrdiff_t>(at),
                       file.begin() + static_cast<std::ptrdiff_t>(at + 4));
}

// The header chunk's tag opens the file.
constexpr size_t kHeaderTagAt = 0;

// Where the first track chunk's tag sits: past the header chunk's tag and
// length field (8 bytes) and the length that field holds.
size_t first_track_tag_at(const std::vector<uint8_t>& file) {
    REQUIRE(file.size() >= 8);
    return 8 + ((size_t{file[4]} << 24) | (size_t{file[5]} << 16) | (size_t{file[6]} << 8) |
                size_t{file[7]});
}

// The longest length four length bytes hold: four 7-bit groups, far longer
// than any track in these tests.
constexpr uint32_t kLongestFourByteLength = (uint32_t{1} << (7 * 4)) - 1;

// A filter that keeps note-ons of 96 and note-offs of 116 only.
hydra::MidiLeanFilter lean_test_filter() {
    hydra::MidiLeanFilter f;
    f.note_on[96] = true;
    f.note_off[116] = true;
    return f;
}

}  // namespace

TEST_CASE("midi: every corpus .mid reads through the lean reader and has a drum track") {
    // The lean reader is the one the song parser reads with, and drums_track
    // owns which track holds the drums (this corpus is all drum charts).
    size_t mids = 0;
    for (const std::string& path : corpus::chart_paths()) {
        if (hydra::chart_format_of(path) != hydra::ChartFormat::Mid) continue;

        ++mids;
        const std::vector<uint8_t> bytes = hydra::read_file_bytes(path);
        const hydra::MidiFile mid =
            hydra::MidiFile::lean(bytes.data(), bytes.size(), lean_test_filter());
        CHECK_MESSAGE(mid.drums_track() != nullptr, path << ": no drum track");
    }
    REQUIRE(mids > 0);
    MESSAGE("midi smoke: " << mids << " files");
}

// from_file reads through read_file_bytes, so a missing file fails as a
// missing song file, with the words cannot_open_file_text gives for its path.
TEST_CASE("midi: from_file on a missing path fails with the file owner's error") {
    const std::string missing = corpus::first_chart_with_suffix(".mid") + ".missing";
    std::optional<hydra::ErrorKind> kind;
    std::string what;
    try {
        hydra::MidiFile::from_file(missing);
    } catch (const hydra::KindedError& e) {
        kind = e.kind();
        what = e.what();
    }
    CHECK(kind == hydra::ErrorKind::SongFileMissing);
    CHECK(what == hydra::cannot_open_file_text(missing));
}

TEST_CASE("midi: running status and zero-velocity note_on") {
    // note_on vel 0 is how most charts spell note_off; consecutive same-status
    // messages exercise running status (the status byte omitted).
    std::vector<uint8_t> track = {
        0x00, 0x90, 0x60, 0x64,   // note_on note 96 vel 100
        0x78, 0x60, 0x00,         // +120, running status, note 96 vel 0
        0x00, 0x61, 0x5A,         // +0, running status, note 97 vel 90
        0x00, 0xFF, 0x2F, 0x00,   // end of track
    };
    hydra::MidiFile mid(smf(track));
    json expected = json::array({ json::array({
        json::array({0, "note_on", 96, 100}),
        json::array({120, "note_on", 96, 0}),
        json::array({120, "note_on", 97, 90}),
    }) });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: running status survives a meta event") {
    // Rock Band rip MIDIs omit the status byte on the channel event right
    // after a meta event. The SMF spec forbids the pattern, but mido reads it:
    // only channel statuses become the running status; 0xFF never does. If a
    // meta byte clobbered it, every note after the first [mix ...] text event
    // would be consumed as meta garbage and the drum track would parse empty.
    std::vector<uint8_t> track = {
        0x00, 0x90, 0x60, 0x64,                    // note_on note 96 vel 100
        0x30, 0xFF, 0x01, 0x03, 'm', 'i', 'x',     // +48 text "mix"
        0x30, 0x61, 0x5A,                          // +48, running status through
                                                   // the meta: note 97 vel 90
        0x00, 0xFF, 0x2F, 0x00,                    // end of track
    };
    hydra::MidiFile mid(smf(track));
    json expected = json::array({ json::array({
        json::array({0, "note_on", 96, 100}),
        json::array({48, "text", "text", "mix"}),
        json::array({96, "note_on", 97, 90}),
    }) });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: a recognized track_name is not displaced by a later name") {
    // Some drum charts carry extra 0x03 metas mid-track ("Drums" after
    // "PART DRUMS"); if the last one won, the song parser's drum name test
    // (is_drums_track_name) would skip the whole track.
    std::vector<uint8_t> track = {
        0x00, 0xFF, 0x03, 0x0A, 'P', 'A', 'R', 'T', ' ', 'D', 'R', 'U', 'M', 'S',
        0x00, 0x90, 0x60, 0x64,                    // note_on note 96 vel 100
        0x00, 0xFF, 0x03, 0x05, 'D', 'r', 'u', 'm', 's',
        0x00, 0xFF, 0x2F, 0x00,                    // end of track
    };
    hydra::MidiFile mid(smf(track));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "PART DRUMS");
}

// D78: Hormone's Echo names its drum track "notes" and then "PART DRUMS",
// both at tick 0. Clone Hero reads the drums; so must Hydra.
TEST_CASE("midi: a recognized track_name wins over an earlier unrecognized one") {
    using namespace testmidi;
    hydra::MidiFile mid(smf(concat({track_name("notes"), track_name("PART DRUMS"),
                                    note_on(96, 100), end_of_track()})));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "PART DRUMS");
}

// Hydra keeps names at any tick (D78 item 3, which also records that YARG
// reads only tick 0), so a recognized name after tick 0 still wins over an
// unrecognized one at tick 0.
TEST_CASE("midi: a recognized track_name after tick 0 still wins") {
    using namespace testmidi;
    hydra::MidiFile mid(smf(concat({track_name("notes"), note_on(96, 100),
                                    after(480, track_name("PART DRUMS")),
                                    end_of_track()})));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "PART DRUMS");
}

TEST_CASE("midi: the first of two recognized track_names wins") {
    using namespace testmidi;
    hydra::MidiFile drums_then_events(smf(concat(
        {track_name("PART DRUMS"), track_name("EVENTS"), note_on(96, 100), end_of_track()})));
    REQUIRE(drums_then_events.tracks.size() == 1);
    CHECK(drums_then_events.tracks[0].name == "PART DRUMS");

    hydra::MidiFile events_then_bass(smf(concat(
        {track_name("EVENTS"), text_event("[section intro]"),
         after(480, track_name("PART BASS")), end_of_track()})));
    REQUIRE(events_then_bass.tracks.size() == 1);
    CHECK(events_then_bass.tracks[0].name == "EVENTS");
}

TEST_CASE("midi: a track with no recognized track_name keeps its first name") {
    using namespace testmidi;
    hydra::MidiFile mid(smf(concat({track_name("notes"), track_name("Drums"),
                                    note_on(96, 100), end_of_track()})));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "notes");
}

// ---- the drum track's name (is_drums_track_name, D252) ----

TEST_CASE("midi: a track named by either drum name, in any case, is the drum track") {
    using namespace testmidi;
    for (const std::string name : {"PART DRUMS", "PART DRUM", "part drums", "Part Drum"}) {
        hydra::MidiFile mid(smf_drums(concat({track_name(name), note_on(96, 100), end_of_track()})));
        INFO(name);
        CHECK(mid.drums_track() == &mid.tracks[1]);
    }
    // No trim and no other spelling: Clone Hero compares the whole lowered name.
    for (const std::string name : {"PART DRUMS ", " PART DRUM", "PART DRUMS2", "DRUMS"}) {
        hydra::MidiFile mid(smf_drums(concat({track_name(name), note_on(96, 100), end_of_track()})));
        INFO(name);
        CHECK(mid.drums_track() == nullptr);
    }
}

namespace {

// A tempo track, then two drum tracks named `first` and `second`. Each holds
// one Expert kick, at velocity 11 in the first and 22 in the second, so a
// test can tell which one was read.
std::vector<uint8_t> two_drum_tracks(const std::string& first, const std::string& second) {
    using namespace testmidi;
    return smf_tracks({concat({set_tempo(), end_of_track()}),
                       concat({track_name(first), note_on(96, 11), end_of_track()}),
                       concat({track_name(second), note_on(96, 22), end_of_track()})});
}

// The velocity of the drum track's first note-on, or -1 with no drum track or
// no note-on in it.
int drum_velocity(const hydra::MidiFile& mid) {
    const hydra::MidiTrack* const drums = mid.drums_track();
    if (drums == nullptr) return -1;
    for (const hydra::Message& m : drums->messages)
        if (m.is_note_on()) return m.velocity;
    return -1;
}

}  // namespace

TEST_CASE("midi: of two drum tracks, the first in file order is the drum track") {
    const hydra::MidiFile drum_first(two_drum_tracks("PART DRUM", "PART DRUMS"));
    REQUIRE(drum_first.drums_track() == &drum_first.tracks[1]);
    CHECK(drum_first.drums_track()->name == "PART DRUM");
    CHECK(drum_velocity(drum_first) == 11);

    const hydra::MidiFile drums_first(two_drum_tracks("PART DRUMS", "PART DRUM"));
    REQUIRE(drums_first.drums_track() == &drums_first.tracks[1]);
    CHECK(drums_first.drums_track()->name == "PART DRUMS");
    CHECK(drum_velocity(drums_first) == 11);
}

TEST_CASE("midi: a drum track name wins over an earlier unrecognized one in any case") {
    using namespace testmidi;
    hydra::MidiFile mid(smf_drums(concat({track_name("notes"), track_name("PART DRUM"),
                                          note_on(96, 100), end_of_track()})));
    REQUIRE(mid.tracks.size() == 2);
    CHECK(mid.tracks[1].name == "PART DRUM");
    CHECK(mid.drums_track() == &mid.tracks[1]);
}

TEST_CASE("midi: sysex and skipped metas do not lose time") {
    std::vector<uint8_t> track = {
        0x00, 0x90, 0x60, 0x64,               // note_on note 96 vel 100
        0x30, 0xF0, 0x04, 0x01, 0x02, 0x03, 0xF7,  // +48 sysex (skipped)
        0x30, 0xFF, 0x06, 0x03, 'm', 'i', 'x',     // +48 marker "mix"
        0x30, 0x80, 0x60, 0x00,               // +48 note_off note 96
        0x00, 0xFF, 0x2F, 0x00,               // end of track
    };
    hydra::MidiFile mid(smf(track));
    json expected = json::array({ json::array({
        json::array({0, "note_on", 96, 100}),
        json::array({96, "marker", "text", "mix"}),   // sysex delta rolled in
        json::array({144, "note_off", 96, 0}),
    }) });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: each text event is decoded on its own, UTF-8 first, else Windows-1252") {
    using namespace testmidi;
    // A Windows-1252 track name ("Szamár", á = 0xE1), a UTF-8 text event
    // ("Café"), and a Windows-1252 one with curly quotes (0x93, 0x94).
    const std::vector<uint8_t> track =
        concat({track_name("Szam\xE1r"), text_event("Caf\xC3\xA9"), text_event("\x93hi\x94"),
                end_of_track()});
    const hydra::MidiFile mid(smf(track));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "Szam\xC3\xA1r");
    std::vector<std::string> texts;
    for (const hydra::Message& m : mid.tracks[0].messages)
        if (m.str_attr == hydra::Message::StrAttr::Text) texts.push_back(m.str);
    CHECK(texts == std::vector<std::string>{"Caf\xC3\xA9", "\xE2\x80\x9Chi\xE2\x80\x9D"});
}

TEST_CASE("midi: each string meta gets its type, its own name and attribute") {
    // One meta of every string-carrying type, 0x01..0x09. 0x08 (program
    // name) has no Message::Type, so the reader drops it.
    std::vector<uint8_t> track;
    for (uint8_t t = 0x01; t <= 0x09; ++t) {
        const uint8_t ev[] = {0x00, 0xFF, t, 0x01, 'x'};
        track.insert(track.end(), std::begin(ev), std::end(ev));
    }
    const uint8_t eot[] = {0x00, 0xFF, 0x2F, 0x00};
    track.insert(track.end(), std::begin(eot), std::end(eot));
    hydra::MidiFile mid(smf(track));
    REQUIRE(mid.tracks.size() == 1);

    using T = hydra::Message::Type;
    using A = hydra::Message::StrAttr;
    struct Want { T type; A attr; };
    const Want want[] = {
        {T::Text, A::Text},
        {T::Copyright, A::Text},
        {T::TrackName, A::Name},
        {T::InstrumentName, A::Name},
        {T::Lyrics, A::Text},
        {T::Marker, A::Text},
        {T::CueMarker, A::Text},
        {T::DeviceName, A::Name},
    };
    const auto& msgs = mid.tracks[0].messages;
    REQUIRE(msgs.size() == std::size(want));
    for (size_t i = 0; i < msgs.size(); ++i) {
        CHECK(msgs[i].type == want[i].type);
        CHECK(msgs[i].str_attr == want[i].attr);
        CHECK(msgs[i].str == "x");
    }

    // message_type_name spells every type the reader emits, each its own
    // way, and none falls through to the spelling it gives a value outside
    // Message::Type.
    const std::string outside = hydra::message_type_name(static_cast<T>(0xFF));
    std::vector<std::string> names;
    for (T type : {T::NoteOn, T::NoteOff, T::SetTempo, T::TimeSignature})
        names.push_back(hydra::message_type_name(type));
    for (const Want& w : want) names.push_back(hydra::message_type_name(w.type));
    for (size_t i = 0; i < names.size(); ++i) {
        CHECK_MESSAGE(!names[i].empty(), i);
        CHECK_MESSAGE(names[i] != outside, names[i]);
        for (size_t j = i + 1; j < names.size(); ++j) CHECK_MESSAGE(names[i] != names[j], names[i]);
    }
}

TEST_CASE("midi: non-MIDI input is rejected") {
    // Junk with no header tag is refused as an unreadable chart, and the
    // detail names the tag it lacks, as the test writer spells it.
    std::vector<uint8_t> junk(40, 'x');
    const std::string detail = midi_refusal(junk);
    CHECK_MESSAGE(detail.find(chunk_tag(testmidi::smf_tracks({}), kHeaderTagAt)) !=
                      std::string::npos,
                  detail);
}

TEST_CASE("midi: SMPTE division is rejected") {
    // A negative division byte pair signals SMPTE timing. The file holds a
    // valid track and reads before the change, so only the SMPTE rule can
    // refuse it, and its detail is its own, not the missing-header or
    // missing-track one.
    std::vector<uint8_t> d = testmidi::smf(testmidi::end_of_track());
    CHECK_NOTHROW((hydra::MidiFile(d)));
    d[12] = 0xE8;  // the header's division bytes: div < 0
    d[13] = 0x00;
    const std::string detail = midi_refusal(d);
    CHECK(detail != midi_refusal(std::vector<uint8_t>(40, 'x')));
    CHECK(detail != midi_refusal(testmidi::smf_tracks({})));
}

TEST_CASE("midi: a five-byte delta accumulates past 32 bits") {
    std::vector<uint8_t> track = {
        0x90, 0x80, 0x80, 0x80, 0x00,  // a five-byte delta (malformed)
        0x90, 0x60, 0x64,              // note_on note 96 vel 100
        0x00, 0xFF, 0x2F, 0x00,        // end of track
    };
    // A delta carries 7 bits a byte, first byte highest: the first byte's
    // bits (0x90 & 0x7F) sit above the four 7-bit groups after it.
    const int64_t delta = int64_t{track[0] & 0x7F} << (7 * 4);
    hydra::MidiFile mid(smf(track));
    json expected = json::array({ json::array({
        json::array({delta, "note_on", 96, 100}),
    }) });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: a message length past the reader's cap refuses the file") {
    // A text meta whose length is 2^32. Before the cap, the length wrapped to
    // 0 and the note_on after it was read as a real event.
    std::vector<uint8_t> meta = {
        0x00, 0xFF, 0x01, 0x90, 0x80, 0x80, 0x80, 0x00,  // text meta, length 2^32
        0x00, 0x90, 0x60, 0x64,                          // note_on note 96 vel 100
        0x00, 0xFF, 0x2F, 0x00,                          // end of track
    };
    CHECK_THROWS_AS((hydra::MidiFile(smf(meta))), hydra::MidiError);

    // A sysex with the longest length four delta bytes hold refuses the
    // file, and the refusal names that length.
    const std::vector<uint8_t> sysex =
        testmidi::concat({std::vector<uint8_t>{0x00, 0xF0}, testmidi::varlen(kLongestFourByteLength),
                          testmidi::end_of_track()});
    const std::string detail = midi_refusal(smf(sysex));
    CHECK_MESSAGE(detail.find(std::to_string(kLongestFourByteLength)) != std::string::npos, detail);

    // A text meta whose length runs a little past the end of its track is
    // not refused: the payload is clamped to the track.
    const std::vector<uint8_t> past_end =
        testmidi::concat({std::vector<uint8_t>{0x00, 0xFF, 0x01}, testmidi::varlen(100),
                          std::vector<uint8_t>{'a', 'b'}});
    CHECK_NOTHROW((hydra::MidiFile(smf(past_end))));
}

// ---- MidiFile::lean (speedups task P1) ----

namespace {

std::vector<std::string> track_names(const hydra::MidiFile& mid) {
    std::vector<std::string> names;
    for (const auto& t : mid.tracks) names.push_back(t.name);
    return names;
}

}  // namespace

TEST_CASE("midi: the lean reader keeps only what each track's role reads, at the same ticks") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        // The first track: its tempo is kept; its text and note are not.
        concat({track_name("tempo"), set_tempo(400000), text_event("t0 text"),
                after(10, note_on(96, 100)), end_of_track()}),
        // The first drum track: its text metas, and the notes the filter names.
        concat({track_name("notes"), text_event("[mix 3 drums0d]"), after(5, note_on(97, 100)),
                after(5, note_on(96, 100)), after(20, note_off(96)), after(30, note_off(116)),
                after(0, track_name("PART DRUMS")), after(7, note_on(96, 1)), end_of_track()}),
        // A second drum track and a guitar track: names only.
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        concat({track_name("PART GUITAR"), text_event("g"), end_of_track()}),
        // EVENTS: its text metas only.
        concat({track_name("EVENTS"), after(3, note_on(96, 100)), after(4, text_event("[section A]")),
                end_of_track()}),
    });
    const hydra::MidiFile mid = hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter());
    CHECK(track_names(mid) ==
          std::vector<std::string>{"tempo", "PART DRUMS", "PART DRUMS", "PART GUITAR", "EVENTS"});
    const json expected = json::array({
        json::array({json::array({0, "set_tempo", 400000})}),
        json::array({json::array({0, "text", "text", "[mix 3 drums0d]"}),
                     json::array({10, "note_on", 96, 100}),
                     json::array({60, "note_off", 116, 0}),
                     json::array({67, "note_on", 96, 1})}),
        json::array(),
        json::array(),
        json::array({json::array({7, "text", "text", "[section A]"})}),
    });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: the lean reader keeps the notes of the first drum track by either name") {
    const std::vector<uint8_t> file = two_drum_tracks("Part Drum", "PART DRUMS");
    const hydra::MidiFile mid = hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter());
    REQUIRE(mid.tracks.size() == 3);
    CHECK(drum_velocity(mid) == 11);
    CHECK(mid.tracks[1].messages.size() == 1);
    CHECK(mid.tracks[2].messages.empty());
}

TEST_CASE("midi: the lean reader sizes each track's messages once") {
    // Each track holds exactly what it keeps, with no spare room from growing
    // one message at a time. The kept counts (1, 7, 5) are ones a vector
    // grown by push_back would overshoot.
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> drums = {track_name("PART DRUMS"),
                                               text_event("[mix 3 drums0d]")};
    for (int i = 0; i < 6; ++i) {
        drums.push_back(after(10, note_on(96, 100)));
        drums.push_back(after(0, note_on(97, 100)));  // dropped by the filter
    }
    drums.push_back(end_of_track());
    std::vector<std::vector<uint8_t>> events = {track_name("EVENTS")};
    for (int i = 0; i < 5; ++i) events.push_back(after(3, text_event("[section A]")));
    events.push_back(end_of_track());
    const std::vector<uint8_t> file = smf_tracks({
        concat({track_name("tempo"), set_tempo(400000), end_of_track()}),
        concat(drums),
        concat(events),
    });
    const hydra::MidiFile mid = hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter());
    REQUIRE(mid.tracks.size() == 3);
    CHECK(mid.tracks[0].messages.size() == 1);
    CHECK(mid.tracks[1].messages.size() == 7);
    CHECK(mid.tracks[2].messages.size() == 5);
    for (const hydra::MidiTrack& t : mid.tracks)
        CHECK_MESSAGE(t.messages.capacity() == t.messages.size(), t.name);
}

TEST_CASE("midi: the lean reader refuses a file where the full reader does") {
    using namespace testmidi;
    // An over-cap meta in a track the lean reader keeps nothing from still
    // refuses the file, with the full reader's words, which name the length.
    const std::vector<uint8_t> oversize =
        concat({std::vector<uint8_t>{0x00, 0xFF, 0x01}, varlen(kLongestFourByteLength),
                std::vector<uint8_t>{'x'}});
    const std::vector<uint8_t> file = smf_tracks({
        concat({track_name("tempo"), set_tempo(), end_of_track()}),
        concat({track_name("PART GUITAR"), oversize}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
    });
    const std::string full = midi_refusal(file);
    CHECK_MESSAGE(full.find(std::to_string(kLongestFourByteLength)) != std::string::npos, full);
    CHECK_THROWS_WITH_AS(hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter()),
                         full.c_str(), hydra::MidiError);

    // A file that opens with another chunk tag is refused by the lean reader
    // in the full reader's words, which name the header tag it lacks.
    const std::vector<uint8_t> not_midi = {'R', 'I', 'F', 'F', 0, 0, 0, 6, 0, 0, 0, 1, 0, 96};
    const std::string not_midi_full = midi_refusal(not_midi);
    CHECK_MESSAGE(not_midi_full.find(chunk_tag(smf_tracks({}), kHeaderTagAt)) != std::string::npos,
                  not_midi_full);
    CHECK_THROWS_WITH_AS(hydra::MidiFile::lean(not_midi.data(), not_midi.size(), lean_test_filter()),
                         not_midi_full.c_str(), hydra::MidiError);
}

TEST_CASE("midi: is_midi_header reads the first four bytes only") {
    // The header comes from the test writer (testmidi::smf_tracks), so the
    // tag is not spelled a second time here.
    std::vector<uint8_t> file = testmidi::smf_tracks({});
    CHECK(hydra::is_midi_header(file.data(), 4));
    CHECK(hydra::is_midi_header(file.data(), file.size()));
    CHECK_FALSE(hydra::is_midi_header(file.data(), 3));  // too short to hold the tag
    CHECK_FALSE(hydra::is_midi_header(file.data(), 0));
    file[3] ^= 0x20;  // the tag's last letter, in the other case
    CHECK_FALSE(hydra::is_midi_header(file.data(), file.size()));
}

TEST_CASE("midi: a file with no track chunk is refused by both readers") {
    // A bare header, and a header whose length field points past the end of
    // the file (the fuzzer's crash, testdata/fuzz/mid/), so the chunk walk
    // finds no MTrk. The song parser reads the timing track by index, so both
    // readers must refuse such a file, in the same words, which name the
    // track tag the walk found none of.
    using namespace testmidi;
    const std::vector<uint8_t> one_track = smf(end_of_track());
    const std::string track_tag = chunk_tag(one_track, first_track_tag_at(one_track));
    const std::vector<uint8_t> bare = smf_tracks({});
    std::vector<uint8_t> long_header = one_track;
    long_header[5] = 2;  // header length 0x00020006, as in the crash file
    for (const std::vector<uint8_t>& file : {bare, long_header}) {
        const std::string full = midi_refusal(file);
        CHECK_MESSAGE(full.find(track_tag) != std::string::npos, full);
        CHECK_THROWS_WITH_AS(hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter()),
                             full.c_str(), hydra::MidiError);
    }
}

TEST_CASE("midi: a track header cut short by the end of the file ends the chunk walk") {
    // The fuzzer's crash (testdata/fuzz/srb/mtrk-tag-at-end.srb): an MTrk tag
    // in the last 4 to 7 bytes, with no room for its length field. Both
    // readers keep the tracks before it and stop there.
    using namespace testmidi;
    const std::vector<uint8_t> whole = smf(end_of_track());
    const std::vector<uint8_t> next_tag = smf_tracks({{}, {}});
    for (size_t extra = 4; extra < 8; ++extra) {
        std::vector<uint8_t> file = whole;
        file.insert(file.end(), next_tag.end() - 8, next_tag.end() - 8 + extra);
        CAPTURE(extra);
        CHECK(hydra::MidiFile(file).tracks.size() == 1);
        CHECK(hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter()).tracks.size() == 1);
    }
}

// ---- which tracks the song parser reads (drums_track, events_track, D253) ----

namespace {

// `file` as the full reader reads it, then as the lean reader does.
std::vector<hydra::MidiFile> both_reads(const std::vector<uint8_t>& file) {
    std::vector<hydra::MidiFile> reads;
    reads.push_back(hydra::MidiFile(file));
    reads.push_back(hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter()));
    return reads;
}

}  // namespace

TEST_CASE("midi: a drum track on track 0 is not the drum track, so the chart has no notes") {
    using namespace testmidi;
    const std::vector<uint8_t> file =
        smf(concat({track_name("PART DRUMS"), set_tempo(), note_on(96, 100), end_of_track()}));
    for (const hydra::MidiFile& read : both_reads(file)) CHECK(read.drums_track() == nullptr);
    const hydra::Song song = hydra::load_songbytes_mid(file, hydra::ChartReading{});
    CHECK(song.sequence.empty());
    CHECK_THROWS_AS(hydra::require_notes(song, hydra::Difficulty::Expert, true),
                    hydra::NoNotesError);
}

TEST_CASE("midi: with drum tracks on track 0 and track 1, track 1 is read") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        concat({track_name("PART DRUMS"), set_tempo(), note_on(96, 11), end_of_track()}),
        concat({track_name("PART DRUMS"), after(480, note_on(96, 22)), end_of_track()}),
    });
    for (const hydra::MidiFile& read : both_reads(file)) {
        REQUIRE(read.drums_track() == &read.tracks[1]);
        CHECK(drum_velocity(read) == 22);
    }
    // Track 0's kick is at tick 0 and track 1's at tick 480, so the chord's
    // tick says which track the song read.
    const hydra::Song song = hydra::load_songbytes_mid(file, hydra::ChartReading{});
    REQUIRE(song.sequence.size() == 1);
    CHECK(song.sequence[0].timecode.ticks() == 480);
}

TEST_CASE("midi: the events track is named events in any case, and track 0 can be it") {
    using namespace testmidi;
    const std::vector<uint8_t> drums =
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()});
    for (const std::string name : {"EVENTS", "Events", "events"}) {
        INFO(name);
        // An unrecognized name first: the events name still wins the pick (D78).
        const std::vector<uint8_t> file = smf_tracks({
            concat({track_name("notes"), track_name(name), set_tempo(),
                    text_event("[section Intro]"), end_of_track()}),
            drums,
        });
        for (const hydra::MidiFile& read : both_reads(file)) {
            CHECK(read.tracks[0].name == name);
            CHECK(read.events_track() == &read.tracks[0]);
        }
        const hydra::Song song = hydra::load_songbytes_mid(file, hydra::ChartReading{});
        REQUIRE(song.practice_sections.size() == 1);
        CHECK(song.practice_sections[0].name == "Intro");
    }
    for (const std::string name : {"EVENTS ", "EVENT", "PART EVENTS"}) {
        INFO(name);
        const std::vector<uint8_t> file = smf_tracks({
            concat({track_name(name), set_tempo(), text_event("[section Intro]"), end_of_track()}),
            drums,
        });
        for (const hydra::MidiFile& read : both_reads(file)) CHECK(read.events_track() == nullptr);
        CHECK(hydra::load_songbytes_mid(file, hydra::ChartReading{}).practice_sections.empty());
    }
}

TEST_CASE("midi: of two events tracks, only the first one's sections count") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("events"), text_event("[section First]"), end_of_track()}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        concat({track_name("EVENTS"), after(960, text_event("[section Second]")), end_of_track()}),
    });
    const std::vector<hydra::MidiFile> reads = both_reads(file);
    for (const hydra::MidiFile& read : reads) CHECK(read.events_track() == &read.tracks[1]);
    // The lean read keeps no text from the second one.
    CHECK(reads[1].tracks[3].messages.empty());
    const hydra::Song song = hydra::load_songbytes_mid(file, hydra::ChartReading{});
    REQUIRE(song.practice_sections.size() == 1);
    CHECK(song.practice_sections[0].name == "First");
}

// Which "[section" texts the .mid reader makes a section from. The rule is
// is_section_marker's (parse/section_names.h), which the reader calls.
namespace {

// A .mid whose events track holds "[section Intro]" at tick 0 and `second` at
// tick 960.
std::vector<uint8_t> two_section_mid(const std::string& second) {
    using namespace testmidi;
    return smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        concat({track_name("EVENTS"), text_event("[section Intro]"),
                after(960, text_event(second)), end_of_track()}),
    });
}

}  // namespace

// The .mid reader drops `second_body` (a "[section" body the section-marker
// owner refuses for the .mid reader but takes for the .chart reader) and keeps
// only the Intro before it.
static void check_mid_drops_section(const std::string& second_body) {
    REQUIRE(hydra::is_section_marker(second_body, hydra::SectionReader::Chart));
    REQUIRE_FALSE(hydra::is_section_marker(second_body, hydra::SectionReader::Mid));
    const hydra::Song song =
        hydra::load_songbytes_mid(two_section_mid("[" + second_body + "]"), hydra::ChartReading{});
    REQUIRE(song.practice_sections.size() == 1);
    CHECK(song.practice_sections[0].tick == 0);
    CHECK(song.practice_sections[0].name == hydra::section_display_name("section Intro"));
}

TEST_CASE("midi: [section _ENDOFSONG] is not a section") {
    check_mid_drops_section("section _ENDOFSONG");
}

TEST_CASE("midi: a section text holding _ENDOFSONG in its middle is not a section") {
    check_mid_drops_section("section Outro_ENDOFSONG_x");
}

TEST_CASE("midi: [section _endofsong] in lower case stays a section") {
    // The game's search is ordinal, so case counts
    // (docs/clone-hero/code/6142/string_contains_197F670.md).
    const hydra::Song song =
        hydra::load_songbytes_mid(two_section_mid("[section _endofsong]"), hydra::ChartReading{});
    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[1].tick == 960);
    // The shown name is section_display_name's (CR-28).
    CHECK(song.practice_sections[1].name == hydra::section_display_name("section _endofsong"));
}

TEST_CASE("midi: [prc__ENDOFSONG] stays a section") {
    const hydra::Song song =
        hydra::load_songbytes_mid(two_section_mid("[prc__ENDOFSONG]"), hydra::ChartReading{});
    REQUIRE(song.practice_sections.size() == 2);
    CHECK(song.practice_sections[1].tick == 960);
    // The shown name is section_display_name's (CR-28).
    CHECK(song.practice_sections[1].name == hydra::section_display_name("prc__ENDOFSONG"));
}

// ---- the end event (Song::end_event_tick) ---------------------------------

TEST_CASE("midi: [end] on the events track is the end event, first one wins") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        // An [end] on the drum track, earlier than any on the events track.
        concat({track_name("PART DRUMS"), text_event("[end]"), note_on(96, 100), end_of_track()}),
        // Ticks 10, 20, 30 and 40: the wrong case, no brackets, then two [end]s.
        concat({track_name("EVENTS"), after(10, text_event("[END]")), after(10, text_event("end")),
                after(10, text_event("[end]")), after(10, text_event("[end]")), end_of_track()}),
    });
    const hydra::Song song = hydra::load_songbytes_mid(file, hydra::ChartReading{});
    CHECK(song.end_event_tick == std::optional<int64_t>(30));

    // An events track with no [end] leaves it unset.
    const std::vector<uint8_t> none = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        concat({track_name("EVENTS"), after(10, text_event("[section Intro]")), end_of_track()}),
    });
    CHECK_FALSE(hydra::load_songbytes_mid(none, hydra::ChartReading{}).end_event_tick.has_value());
}

TEST_CASE("midi: the lean read keeps the end event") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
        concat({track_name("EVENTS"), after(1920, text_event("[end]")), end_of_track()}),
    });
    // The song parser reads through the lean reader, so the song's end event
    // is the lean read's answer, in the full reading and the notes-only one.
    const hydra::Song full = hydra::load_songbytes_mid(file, hydra::ChartReading{});
    CHECK(full.end_event_tick == std::optional<int64_t>(1920));
    const hydra::Song notes_only = hydra::load_songbytes_mid(file, hydra::ChartReading{},
                                                             hydra::core::default_rules(),
                                                             hydra::kNotesOnlyFills);
    CHECK(notes_only.end_event_tick == std::optional<int64_t>(1920));
}

// ---- the last note end (Song::last_note_end_tick) -------------------------

TEST_CASE("midi: a bass sustain past the last drum note is the last note end; BEAT, VENUE and EVENTS don't count") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        // The last drum note ends at tick 10, and a pitch the lean filter
        // drops (70) ends at tick 480. A Big Rock Ending marker (120) held to
        // tick 3000 is not a note.
        concat({track_name("PART DRUMS"), note_on(96, 100), after(10, note_off(96)),
                after(10, note_on(70, 100)), after(460, note_off(70)),
                note_on(120, 100), after(2520, note_off(120)), end_of_track()}),
        // The bass note starts before the drums end and is held to tick 2000.
        // A solo marker (103) and a Star Power marker (116) run on to 4000.
        concat({track_name("PART BASS"), after(5, note_on(60, 100)), after(1995, note_off(60)),
                note_on(103, 100), note_on(116, 100), after(2000, note_off(103)), note_off(116),
                end_of_track()}),
        concat({track_name("BEAT"), after(5000, note_on(40, 100)), after(10, note_off(40)), end_of_track()}),
        concat({track_name("VENUE"), after(6000, note_on(40, 100)), end_of_track()}),
        concat({track_name("EVENTS"), after(7000, note_on(40, 100)), text_event("[end]"),
                end_of_track()}),
    });
    for (const hydra::MidiFile& read : both_reads(file)) {
        CHECK(read.tracks[1].last_note_tick == std::optional<int64_t>(480));
        CHECK(read.tracks[3].last_low_note_tick == std::optional<int64_t>(5010));
        CHECK(read.last_note_end_tick() == std::optional<int64_t>(2000));
    }
    CHECK(hydra::load_songbytes_mid(file, hydra::ChartReading{}).last_note_end_tick ==
          std::optional<int64_t>(2000));
    // The notes-only reading gives the same tick.
    CHECK(hydra::load_songbytes_mid(file, hydra::ChartReading{}, hydra::core::default_rules(),
                                    hydra::kNotesOnlyFills)
              .last_note_end_tick == std::optional<int64_t>(2000));
}

TEST_CASE("midi: the last note end reads each instrument once, from track 1, by instrument name") {
    using namespace testmidi;
    const std::vector<uint8_t> file = smf_tracks({
        // Track 0 is never an instrument, whatever its name.
        concat({track_name("PART GUITAR"), set_tempo(), after(9000, note_on(60, 100)), end_of_track()}),
        concat({track_name("part drum"), after(100, note_on(96, 100)), end_of_track()}),
        concat({track_name("Part Guitar"), after(300, note_on(60, 100)), after(10, note_off(60)),
                end_of_track()}),
        // A second drum track, by the other drum name, and a second guitar.
        concat({track_name("PART DRUMS"), after(8000, note_on(96, 100)), end_of_track()}),
        concat({track_name("PART GUITAR"), after(8500, note_on(60, 100)), end_of_track()}),
        // Names Clone Hero does not read as an instrument.
        concat({track_name("PART REAL_GUITAR"), after(7000, note_on(60, 100)), end_of_track()}),
        concat({track_name("PART DRUMS_2X"), after(7500, note_on(96, 100)), end_of_track()}),
    });
    for (const hydra::MidiFile& read : both_reads(file))
        CHECK(read.last_note_end_tick() == std::optional<int64_t>(310));
    CHECK(hydra::load_songbytes_mid(file, hydra::ChartReading{}).last_note_end_tick ==
          std::optional<int64_t>(310));

    CHECK(hydra::is_instrument_track_name("PART KEYS"));
    CHECK(hydra::is_instrument_track_name("Part Drums"));
    CHECK_FALSE(hydra::is_instrument_track_name("BEAT"));
    CHECK_FALSE(hydra::is_instrument_track_name("VENUE"));
    CHECK_FALSE(hydra::is_instrument_track_name("EVENTS"));

    // Below the gems: guitar animation (50) is not a note, a vocal note (50)
    // is. Pitches 101 and 102 count; 103 does not.
    const std::vector<uint8_t> low = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART GUITAR"), after(100, note_on(60, 100)), after(100, note_off(60)),
                note_on(50, 100), after(900, note_off(50)), end_of_track()}),
        concat({track_name("PART VOCALS"), after(150, note_on(50, 100)), after(50, note_off(50)),
                end_of_track()}),
    });
    for (const hydra::MidiFile& read : both_reads(low))
        CHECK(read.last_note_end_tick() == std::optional<int64_t>(200));
    const std::vector<uint8_t> vocal_last = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART VOCALS"), after(100, note_on(60, 100)), after(100, note_off(60)),
                note_on(40, 100), after(300, note_off(40)), end_of_track()}),
    });
    for (const hydra::MidiFile& read : both_reads(vocal_last))
        CHECK(read.last_note_end_tick() == std::optional<int64_t>(500));
    CHECK(hydra::is_gem_pitch(58));
    CHECK(hydra::is_gem_pitch(102));
    CHECK_FALSE(hydra::is_gem_pitch(57));
    CHECK_FALSE(hydra::is_gem_pitch(103));
    CHECK(hydra::is_low_vocal_pitch(36));
    CHECK_FALSE(hydra::is_low_vocal_pitch(35));
    CHECK_FALSE(hydra::is_low_vocal_pitch(58));

    // No instrument note at all: unset.
    const std::vector<uint8_t> empty = smf_tracks({
        concat({set_tempo(), end_of_track()}),
        concat({track_name("PART DRUMS"), end_of_track()}),
        concat({track_name("BEAT"), note_on(12, 100), end_of_track()}),
    });
    CHECK_FALSE(hydra::load_songbytes_mid(empty, hydra::ChartReading{}).last_note_end_tick.has_value());
}

// ---- drum lanes (Song::drum_lanes) ----------------------------------------

namespace {

// One drum track: a lane note `pitch` held at `velocity` from tick 0 to tick
// 240, over a red at tick 0 on Expert (97), Hard (85), Medium (73) and Easy
// (61), and a yellow at tick 120 on each.
std::vector<uint8_t> lane_mid(uint8_t pitch, uint8_t velocity) {
    using namespace testmidi;
    return smf_drums(concat({
        track_name("PART DRUMS"),
        note_on(pitch, velocity),
        note_on(97, 100),
        note_on(85, 100),
        note_on(73, 100),
        note_on(61, 100),
        after(120, note_on(98, 100)),
        note_on(86, 100),
        note_on(74, 100),
        note_on(62, 100),
        after(120, note_off(pitch)),
        end_of_track(),
    }));
}

size_t lanes_at(const std::vector<uint8_t>& mid, hydra::Difficulty difficulty) {
    return hydra::load_songbytes_mid(mid, hydra::ChartReading{difficulty}).drum_lanes.size();
}

}  // namespace

TEST_CASE("mid: a lane's velocity picks the lowest difficulty that shows it") {
    // Every note-on velocity a lane can carry. Each one shows the lane on
    // one lowest difficulty and every harder one (kAllDifficulties lists the
    // hardest first), never on a gap between; Expert shows every lane; and
    // each difficulty is the lowest for some velocity, each below Expert for
    // one unbroken band of them (Expert takes every velocity outside them).
    const size_t n = std::size(hydra::kAllDifficulties);
    std::vector<size_t> lowest_of;  // by velocity - 1: index of the lowest
    for (int velocity = 1; velocity < hydra::kMidiDataValues; ++velocity) {
        const std::vector<uint8_t> mid = lane_mid(126, static_cast<uint8_t>(velocity));
        size_t shown = 0;
        for (size_t i = 0; i < n; ++i) {
            const size_t lanes = lanes_at(mid, hydra::kAllDifficulties[i]);
            CHECK_MESSAGE(lanes <= 1, velocity);
            // Shown here only if shown at every harder difficulty.
            if (lanes == 1) {
                CHECK_MESSAGE(shown == i, velocity);
                ++shown;
            }
        }
        CHECK_MESSAGE(shown >= 1, velocity);
        lowest_of.push_back(shown == 0 ? size_t{0} : shown - 1);
    }
    for (size_t d = 0; d < n; ++d) {
        size_t first = lowest_of.size(), last = 0, count = 0;
        for (size_t v = 0; v < lowest_of.size(); ++v) {
            if (lowest_of[v] != d) continue;
            first = (std::min)(first, v);
            last = v;
            ++count;
        }
        CHECK_MESSAGE(count > 0, "no velocity is lowest on difficulty index " << d);
        if (count > 0 && d > 0) CHECK_MESSAGE(last - first + 1 == count, "difficulty index " << d);
    }
}

TEST_CASE("mid: 126 is a single lane and 127 a double, note-on to note-off") {
    const hydra::Song single = hydra::load_songbytes_mid(
        lane_mid(126, 96), hydra::ChartReading{hydra::Difficulty::Expert});
    REQUIRE(single.drum_lanes.size() == 1);
    CHECK(single.drum_lanes[0].start_tick == 0);
    CHECK(single.drum_lanes[0].end_tick == 240);
    CHECK_FALSE(single.drum_lanes[0].double_lane);
    CHECK(single.drum_lanes[0].pads == std::vector<hydra::NoteColor>{hydra::NoteColor::Red});

    // Red alone, then yellow: the double lane lights both.
    const hydra::Song dbl = hydra::load_songbytes_mid(
        lane_mid(127, 96), hydra::ChartReading{hydra::Difficulty::Expert});
    REQUIRE(dbl.drum_lanes.size() == 1);
    CHECK(dbl.drum_lanes[0].end_tick == 240);
    CHECK(dbl.drum_lanes[0].double_lane);
    CHECK(dbl.drum_lanes[0].pads ==
          std::vector<hydra::NoteColor>{hydra::NoteColor::Red, hydra::NoteColor::Yellow});
}
