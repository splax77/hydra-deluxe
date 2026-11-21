// Finding 64, D24: the .mid dynamics tag counts only in
// Clone Hero 1.1's two exact spellings (0x21557A5, 0x21557BB, no trim), takes
// effect in file order at a shared tick (the flag is read at each note-on,
// 0x21555F1), and a late tag is stored so the Dynamics tab can say how many
// earlier markings Clone Hero ignored.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "app/display_format.h"
#include "app/dynamics_breakdown.h"
#include "load_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "ui/details_parts.h"

using namespace hydra;
using namespace testmidi;

namespace {

Song drums(std::vector<std::vector<uint8_t>> events) {
    events.insert(events.begin(), track_name("PART DRUMS"));
    events.push_back(end_of_track());
    return load_songbytes_mid(smf_drums(concat(events)), ChartReading{});
}

NoteDynamicType red(const Song& song, size_t i) {
    return song.sequence.at(i).chord.at(NoteColor::Red)->dynamictype;
}

constexpr const char* kTag = "[ENABLE_CHART_DYNAMICS]";

// A drum chart with `marks` accented reds one beat apart, then the tag a beat
// after the last, then a priced red at the tag's tick: a late tag with
// `marks` earlier markings.
Song late_tag_song(int marks) {
    const uint32_t beat = 480;  // one beat: smf_tracks writes 480 ticks per beat
    std::vector<std::vector<uint8_t>> events;
    for (int i = 0; i < marks; ++i)
        events.push_back(i == 0 ? note_on(97, 127) : after(beat, note_on(97, 127)));
    events.push_back(after(beat, text_event(kTag)));
    events.push_back(note_on(97, 127));
    return drums(std::move(events));
}

// The song's late tag time in whole ms, read through its own tempo map.
uint32_t late_tag_ms(const Song& song) {
    return static_cast<uint32_t>(song.timecode(*song.dynamics_late_tag_tick).ms());
}

}  // namespace

TEST_CASE("dynamics tag: only Clone Hero's two exact spellings count") {
    for (const char* text : {"ENABLE_CHART_DYNAMICS", "[ENABLE_CHART_DYNAMICS]"}) {
        CAPTURE(text);
        const Song song = drums({text_event(text), note_on(97, 127)});
        CHECK(song.dynamics_enabled);
        CHECK(red(song, 0) == NoteDynamicType::Accent);
    }
    for (const char* text :
         {"[ENABLE_CHART_DYNAMICS", "ENABLE_CHART_DYNAMICS]", "[[ENABLE_CHART_DYNAMICS]]",
          " [ENABLE_CHART_DYNAMICS] ", "[enable_chart_dynamics]", "ENABLE_CHART_DYNAMICS "}) {
        CAPTURE(text);
        const Song song = drums({text_event(text), note_on(97, 127)});
        CHECK_FALSE(song.dynamics_enabled);
        CHECK(red(song, 0) == NoteDynamicType::Normal);
    }
}

TEST_CASE("dynamics tag: at a shared tick, file order decides") {
    SUBCASE("tag first: the note is priced") {
        const Song song = drums({text_event(kTag), note_on(97, 127)});
        CHECK(red(song, 0) == NoteDynamicType::Accent);
        CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
        CHECK(song.dynamics_marks_before_tag == 0);
    }
    SUBCASE("note first: the note stays plain, and counts as an earlier marking") {
        const Song song = drums({note_on(97, 127), text_event(kTag),
                                 after(480,note_on(97, 127))});  // tick 480
        CHECK(red(song, 0) == NoteDynamicType::Normal);
        CHECK(red(song, 1) == NoteDynamicType::Accent);
        REQUIRE(song.dynamics_late_tag_tick.has_value());
        CHECK(*song.dynamics_late_tag_tick == 0);
        CHECK(song.dynamics_marks_before_tag == 1);
    }
}

TEST_CASE("dynamics tag: a late tag keeps its tick and the earlier markings") {
    const Song song = drums({
        note_on(97, 127),                         // tick 0: red, accent velocity
        after(240,note_on(98, 1)),      // tick 240: yellow, ghost velocity
        after(240,text_event(kTag)),    // tick 480: the tag
        after(480,note_on(97, 127)),    // tick 960: red, accent velocity
    });
    CHECK(song.dynamics_enabled);
    CHECK(red(song, 0) == NoteDynamicType::Normal);
    CHECK(song.sequence.at(1).chord.at(NoteColor::Yellow)->dynamictype ==
          NoteDynamicType::Normal);
    CHECK(red(song, 2) == NoteDynamicType::Accent);
    REQUIRE(song.dynamics_late_tag_tick.has_value());
    CHECK(*song.dynamics_late_tag_tick == 480);
    CHECK(song.dynamics_marks_before_tag == 2);
}

TEST_CASE("dynamics tag: a late tag after only plain notes is not stored") {
    const Song song = drums({note_on(97, 100), after(480,text_event(kTag)),
                             after(480,note_on(97, 127))});
    CHECK(song.dynamics_enabled);
    CHECK(red(song, 1) == NoteDynamicType::Accent);
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a second tag changes nothing") {
    const Song song = drums({text_event(kTag), note_on(97, 127),
                             after(480,text_event(kTag)),
                             after(480,note_on(97, 127))});
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a tag in the EVENTS track does nothing") {
    // Two tracks: EVENTS (with the tempo and the tag), then PART DRUMS.
    const std::vector<uint8_t> events =
        concat({track_name("EVENTS"), set_tempo(), text_event(kTag), end_of_track()});
    const std::vector<uint8_t> part =
        concat({track_name("PART DRUMS"), note_on(97, 127), end_of_track()});
    const Song song = load_songbytes_mid(smf_tracks({events, part}), ChartReading{});
    CHECK_FALSE(song.dynamics_enabled);
    CHECK(red(song, 0) == NoteDynamicType::Normal);
}

TEST_CASE("dynamics tag: the breakdown carries the tag's time and the count") {
    const Song song = late_tag_song(1);
    REQUIRE(song.dynamics_late_tag_tick.has_value());
    REQUIRE(song.dynamics_marks_before_tag == 1);
    const app::DynamicsBreakdown bd = app::count_dynamics(song);
    REQUIRE(bd.late_tag_ms.has_value());
    // The time is the song's own tag tick read through its tempo map, in ms,
    // so a tick passed through unconverted fails.
    CHECK(*bd.late_tag_ms == late_tag_ms(song));
    CHECK(static_cast<int64_t>(*bd.late_tag_ms) != *song.dynamics_late_tag_tick);
    CHECK(bd.marks_before_tag == song.dynamics_marks_before_tag);

    // A chart whose tag comes before any marking stores no late tag.
    const app::DynamicsBreakdown clean = app::count_dynamics(drums({text_event(kTag), note_on(97, 127)}));
    CHECK(clean.dynamics_enabled);
    CHECK_FALSE(clean.late_tag_ms.has_value());
    CHECK(clean.marks_before_tag == 0);
}

TEST_CASE("dynamics tag: the Dynamics tab's Chart line") {
    using ui::detail::dynamics_enabled_text;
    CHECK(dynamics_enabled_text(app::count_dynamics(drums({note_on(97, 127)}))) ==
          "Dynamics enabled: no (markings ignored by Clone Hero)");
    CHECK(dynamics_enabled_text(app::count_dynamics(drums({text_event(kTag), note_on(97, 127)}))) ==
          "Dynamics enabled: yes");

    // A late tag: the line names the song's own tag time and how many
    // markings came before it, one in the singular and a thousand-plus with
    // a comma.
    struct LateCase {
        int marks;
        const char* count_text;
    };
    for (const LateCase& c : {LateCase{1, "1 earlier marking"}, LateCase{1234, "1,234 earlier markings"}}) {
        CAPTURE(c.marks);
        const Song song = late_tag_song(c.marks);
        REQUIRE(song.dynamics_marks_before_tag == c.marks);
        const std::string time_text = app::format_duration(late_tag_ms(song) / 1000.0);
        const std::string expected = "Dynamics enabled: from " + time_text + " on (" +
                                     std::string(c.count_text) + " ignored by Clone Hero)";
        CHECK(dynamics_enabled_text(app::count_dynamics(song)) == expected);
    }
}
