// Smoke + unit tests for parse/midi.
//
// Smoke: every .mid in the corpus must read without throwing and produce a
// sane track/event structure. Unit: the edge cases the corpus may not contain
// but a user's library might.

#include "doctest.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include "core/strutil.h"
#include "parse/chart_files.h"
#include "parse/midi.h"
#include "corpus_util.h"
#include "leak_check.h"
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

// smf() now lives in midi_util.h so test_song.cpp can build files too.
using testmidi::smf;

}  // namespace

TEST_CASE("midi: every corpus .mid reads with a sane structure") {
    hydra::test::leak_checked([&] {
        size_t mids = 0;
        for (const std::string& path : corpus::chart_paths()) {
            if (hydra::chart_format_of(path) != hydra::ChartFormat::Mid) continue;

            hydra::MidiFile mid = hydra::MidiFile::from_file(path);
            ++mids;

            CHECK_MESSAGE(mid.ticks_per_beat > 0, path << ": ticks_per_beat");
            CHECK_MESSAGE(!mid.tracks.empty(), path << ": no tracks");

            // Delta times never run backwards, and the drum track exists by name
            // (this corpus is all drum charts).
            bool has_drums = false;
            bool deltas_ok = true;
            for (const auto& t : mid.tracks) {
                if (t.name == "PART DRUMS") has_drums = true;
                for (const auto& m : t.messages)
                    if (m.time < 0) deltas_ok = false;
            }
            CHECK_MESSAGE(deltas_ok, path << ": negative delta");
            CHECK_MESSAGE(has_drums, path << ": no PART DRUMS track");
        }
        REQUIRE(mids > 0);
        MESSAGE("midi smoke: " << mids << " files");
    });
}

// D54: from_file reads through read_file_bytes, so a missing file fails with
// that owner's own words.
TEST_CASE("midi: from_file on a missing path fails with the file owner's error") {
    const std::string missing = corpus::first_chart_with_suffix(".mid") + ".missing";
    std::string what;
    try {
        hydra::MidiFile::from_file(missing);
    } catch (const std::exception& e) {
        what = e.what();
    }
    CHECK_MESSAGE(what.rfind("cannot open file: ", 0) == 0, what);
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
    // "PART DRUMS"); if the last one won, the song parser's exact name match
    // would skip the whole track.
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

// Hydra keeps names at any tick (YARG reads only tick 0), so a recognized
// name after tick 0 still wins over an unrecognized one at tick 0.
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

TEST_CASE("midi: track_name is not exposed as text") {
    // hysong matches MetaMessage(text=...) for disco/dynamics markers; a
    // track_name must carry `name`, not `text`, or every title would be offered
    // to those regexes.
    std::vector<uint8_t> track = {
        0x00, 0xFF, 0x03, 0x0A, 'P', 'A', 'R', 'T', ' ', 'D', 'R', 'U', 'M', 'S',
        0x00, 0x90, 0x60, 0x64,
        0x00, 0xFF, 0x2F, 0x00,
    };
    hydra::MidiFile mid(smf(track));
    REQUIRE(mid.tracks.size() == 1);
    CHECK(mid.tracks[0].name == "PART DRUMS");

    bool found = false;
    for (const auto& m : mid.tracks[0].messages) {
        if (m.type == hydra::Message::Type::TrackName) {
            found = true;
            CHECK(m.str_attr == hydra::Message::StrAttr::Name);
            CHECK(m.str_attr != hydra::Message::StrAttr::Text);
        }
    }
    CHECK(found);
}

TEST_CASE("midi: each string meta gets its type, mido name and attribute") {
    // One meta of every string-carrying type, 0x01..0x09. 0x08 (program
    // name) is unknown to mido, so it is dropped.
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
    struct Want { T type; const char* name; A attr; };
    const Want want[] = {
        {T::Text, "text", A::Text},
        {T::Copyright, "copyright", A::Text},
        {T::TrackName, "track_name", A::Name},
        {T::InstrumentName, "instrument_name", A::Name},
        {T::Lyrics, "lyrics", A::Text},
        {T::Marker, "marker", A::Text},
        {T::CueMarker, "cue_marker", A::Text},
        {T::DeviceName, "device_name", A::Name},
    };
    const auto& msgs = mid.tracks[0].messages;
    REQUIRE(msgs.size() == std::size(want));
    for (size_t i = 0; i < msgs.size(); ++i) {
        CHECK(msgs[i].type == want[i].type);
        CHECK(std::string(hydra::message_type_name(msgs[i].type)) == want[i].name);
        CHECK(msgs[i].str_attr == want[i].attr);
        CHECK(msgs[i].str == "x");
    }
    CHECK(std::string(hydra::message_type_name(T::NoteOn)) == "note_on");
    CHECK(std::string(hydra::message_type_name(T::NoteOff)) == "note_off");
    CHECK(std::string(hydra::message_type_name(T::SetTempo)) == "set_tempo");
    CHECK(std::string(hydra::message_type_name(T::TimeSignature)) == "time_signature");
}

TEST_CASE("midi: non-MIDI input is rejected") {
    std::vector<uint8_t> junk(40, 'x');
    CHECK_THROWS_AS((hydra::MidiFile(junk)), hydra::MidiError);
}

TEST_CASE("midi: SMPTE division is rejected") {
    // A negative division byte pair signals SMPTE timing.
    std::vector<uint8_t> d = {
        'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0xE8, 0x00,  // div < 0
    };
    CHECK_THROWS_AS((hydra::MidiFile(d)), hydra::MidiError);
}

TEST_CASE("midi: a five-byte delta accumulates past 32 bits, as mido does") {
    std::vector<uint8_t> track = {
        0x90, 0x80, 0x80, 0x80, 0x00,  // delta 2^32 (malformed: five bytes)
        0x90, 0x60, 0x64,              // note_on note 96 vel 100
        0x00, 0xFF, 0x2F, 0x00,        // end of track
    };
    hydra::MidiFile mid(smf(track));
    json expected = json::array({ json::array({
        json::array({int64_t{1} << 32, "note_on", 96, 100}),
    }) });
    CHECK(event_view(mid) == expected);
}

TEST_CASE("midi: a message longer than mido's 1,000,000-byte cap refuses the file, as mido does") {
    // A text meta whose length is 2^32. Before the cap, the length wrapped to
    // 0 and the note_on after it was read as a real event.
    std::vector<uint8_t> meta = {
        0x00, 0xFF, 0x01, 0x90, 0x80, 0x80, 0x80, 0x00,  // text meta, length 2^32
        0x00, 0x90, 0x60, 0x64,                          // note_on note 96 vel 100
        0x00, 0xFF, 0x2F, 0x00,                          // end of track
    };
    CHECK_THROWS_AS((hydra::MidiFile(smf(meta))), hydra::MidiError);

    // A sysex one byte over the cap: 1,000,001 = 0xBD 0x84 0x41.
    std::vector<uint8_t> sysex = {
        0x00, 0xF0, 0xBD, 0x84, 0x41,
        0x00, 0xFF, 0x2F, 0x00,
    };
    CHECK_THROWS_AS((hydra::MidiFile(smf(sysex))), hydra::MidiError);

    // Exactly at the cap (1,000,000 = 0xBD 0x84 0x40) is not refused. The
    // track is shorter than that, so the payload is clamped as today.
    std::vector<uint8_t> at_cap = {
        0x00, 0xFF, 0x01, 0xBD, 0x84, 0x40, 'a', 'b',
    };
    CHECK_NOTHROW((hydra::MidiFile(smf(at_cap))));
}

// ---- MidiFile::lean (speedups task P1) ----

namespace {

// A filter that keeps note-ons of 96 and note-offs of 116 only.
hydra::MidiLeanFilter lean_test_filter() {
    hydra::MidiLeanFilter f;
    f.note_on[96] = true;
    f.note_off[116] = true;
    return f;
}

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
    // refuses the file, with the full reader's words.
    const std::vector<uint8_t> oversize = {0x00, 0xFF, 0x01, 0xBD, 0x84, 0x41, 'x'};
    const std::vector<uint8_t> file = smf_tracks({
        concat({track_name("tempo"), set_tempo(), end_of_track()}),
        concat({track_name("PART GUITAR"), oversize}),
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()}),
    });
    std::string full, lean;
    try {
        hydra::MidiFile m(file);
    } catch (const hydra::MidiError& e) {
        full = e.what();
    }
    try {
        hydra::MidiFile::lean(file.data(), file.size(), lean_test_filter());
    } catch (const hydra::MidiError& e) {
        lean = e.what();
    }
    CHECK(full == "Message length 1000001 exceeds maximum length 1000000");
    CHECK(lean == full);

    const std::vector<uint8_t> not_midi = {'R', 'I', 'F', 'F', 0, 0, 0, 6, 0, 0, 0, 1, 0, 96};
    CHECK_THROWS_WITH_AS(hydra::MidiFile::lean(not_midi.data(), not_midi.size(), lean_test_filter()),
                         "not a MIDI file: missing MThd header", hydra::MidiError);
}
