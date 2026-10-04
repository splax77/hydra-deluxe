// Step 2, T5 (finding 64, D24): the .mid dynamics tag counts only in
// Clone Hero 1.1's two exact spellings (0x21557A5, 0x21557BB, no trim), takes
// effect in file order at a shared tick (the flag is read at each note-on,
// 0x21555F1), and a late tag is stored so the Dynamics tab can say how many
// earlier markings Clone Hero ignored.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "env_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "ui/details_parts.h"

using namespace hydra;
using namespace testmidi;

namespace {

// `ev` (a delta-0 event from midi_util.h) moved later by `delta`.
std::vector<uint8_t> after(std::vector<uint8_t> delta, std::vector<uint8_t> ev) {
    ev.erase(ev.begin());
    delta.insert(delta.end(), ev.begin(), ev.end());
    return delta;
}

Song drums(std::vector<std::vector<uint8_t>> events) {
    events.insert(events.begin(), {track_name("PART DRUMS"), set_tempo()});
    events.push_back(end_of_track());
    return load_songbytes_mid(smf(concat(events)), true, true);
}

NoteDynamicType red(const Song& song, size_t i) {
    return song.sequence.at(i).chord.at(NoteColor::Red)->dynamictype;
}

constexpr const char* kTag = "[ENABLE_CHART_DYNAMICS]";

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
                                 after({0x83, 0x60}, note_on(97, 127))});  // tick 480
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
        after({0x81, 0x70}, note_on(98, 1)),      // tick 240: yellow, ghost velocity
        after({0x81, 0x70}, text_event(kTag)),    // tick 480: the tag
        after({0x83, 0x60}, note_on(97, 127)),    // tick 960: red, accent velocity
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
    const Song song = drums({note_on(97, 100), after({0x83, 0x60}, text_event(kTag)),
                             after({0x83, 0x60}, note_on(97, 127))});
    CHECK(song.dynamics_enabled);
    CHECK(red(song, 1) == NoteDynamicType::Accent);
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a second tag changes nothing") {
    const Song song = drums({text_event(kTag), note_on(97, 127),
                             after({0x83, 0x60}, text_event(kTag)),
                             after({0x83, 0x60}, note_on(97, 127))});
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a tag in the EVENTS track does nothing") {
    // Two tracks: EVENTS (with the tempo and the tag), then PART DRUMS.
    const std::vector<uint8_t> events =
        concat({track_name("EVENTS"), set_tempo(), text_event(kTag), end_of_track()});
    const std::vector<uint8_t> part =
        concat({track_name("PART DRUMS"), note_on(97, 127), end_of_track()});
    std::vector<uint8_t> file = {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 2, 0x01, 0xE0};
    for (const std::vector<uint8_t>* t : {&events, &part}) {
        file.insert(file.end(), {'M', 'T', 'r', 'k'});
        const uint32_t n = static_cast<uint32_t>(t->size());
        for (int shift = 24; shift >= 0; shift -= 8)
            file.push_back(static_cast<uint8_t>(n >> shift));
        file.insert(file.end(), t->begin(), t->end());
    }
    const Song song = load_songbytes_mid(file, true, true);
    CHECK_FALSE(song.dynamics_enabled);
    CHECK(red(song, 0) == NoteDynamicType::Normal);
}

TEST_CASE("dynamics tag: the breakdown stores the tag's time and the count") {
    const Song song = drums({note_on(97, 127), after({0x83, 0x60}, text_event(kTag)),
                             note_on(97, 127)});  // tag and a priced note at tick 480
    const app::DynamicsBreakdown bd = app::count_dynamics(song);
    REQUIRE(bd.late_tag_ms.has_value());
    CHECK(*bd.late_tag_ms == 500);  // tick 480 is one beat at 120 BPM
    CHECK(bd.marks_before_tag == 1);

    std::vector<uint8_t> blob = app::encode_dynamics(bd);
    const auto back = app::decode_dynamics(blob);
    REQUIRE(back.has_value());
    CHECK(back->late_tag_ms == bd.late_tag_ms);
    CHECK(back->marks_before_tag == 1);

    app::DynamicsBreakdown none;
    const auto none_back = app::decode_dynamics(app::encode_dynamics(none));
    REQUIRE(none_back.has_value());
    CHECK_FALSE(none_back->late_tag_ms.has_value());
    CHECK(none_back->marks_before_tag == 0);

    blob.resize(110);  // the old layout's length: read as missing, then recounted
    CHECK_FALSE(app::decode_dynamics(blob).has_value());
}

TEST_CASE("dynamics tag: the Dynamics tab's Chart line") {
    using ui::detail::dynamics_enabled_text;
    app::DynamicsBreakdown bd;
    CHECK(dynamics_enabled_text(bd) == "Dynamics enabled: no (markings ignored by Clone Hero)");
    bd.dynamics_enabled = true;
    CHECK(dynamics_enabled_text(bd) == "Dynamics enabled: yes");
    bd.late_tag_ms = 241'000;
    bd.marks_before_tag = 142;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (142 earlier markings ignored by Clone Hero)");
    bd.marks_before_tag = 1;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (1 earlier marking ignored by Clone Hero)");
    bd.marks_before_tag = 1234;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (1,234 earlier markings ignored by Clone Hero)");
}

// Dev aid, not a pinned case. Set HYDRA_DYNAMICS_TAG_CHARTS to a list of
// notes.mid paths separated by ';', then run -tc="dynamics tag dev aid*" to
// print each chart's Expert Pro Drums Chart line as the tab would show it.
TEST_CASE("dynamics tag dev aid: print the Chart line for real charts") {
    const auto list = read_env("HYDRA_DYNAMICS_TAG_CHARTS");
    if (!list) return;
    size_t from = 0;
    while (from <= list->size()) {
        const size_t to = std::min(list->find(';', from), list->size());
        const std::string path = list->substr(from, to - from);
        from = to + 1;
        if (path.empty()) continue;
        const Song song = load_songpath_mid(path, true, app::kDynamicsParseBass2x);
        const app::DynamicsBreakdown bd = app::count_dynamics(song);
        std::printf("%s\n  %s (tag tick %lld)\n", path.c_str(),
                    ui::detail::dynamics_enabled_text(bd).c_str(),
                    static_cast<long long>(song.dynamics_late_tag_tick.value_or(-1)));
    }
}
