// Step 2, T3 (finding 21, D21): one phrase-end rule for both chart
// formats. A Star Power phrase covers start <= tick < end, as Clone Hero 1.1
// assigns notes to phrases (0x20D2440). The same chords and the same phrase,
// written once as .chart and once as .mid, must flag the same chord.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "chart_text.h"
#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

namespace {

// One red pad every 480 ticks from 480 to 2400, at 480 ticks per beat in both
// formats. The phrase always starts on the chord at 1440.
constexpr int64_t kFirstChord = 480, kLastChord = 2400, kStep = 480;
constexpr int64_t kPhraseStart = 1440;

struct Flags {
    std::vector<int64_t> ticks;   // chords with flag_sp
    std::vector<int64_t> starts;  // their sp_phrase_start
    // Written out: the code is C++17, which has no defaulted ==.
    bool operator==(const Flags& o) const { return ticks == o.ticks && starts == o.starts; }
};

Flags flags_of(const Song& song) {
    Flags f;
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.flag_sp) continue;
        f.ticks.push_back(ts.timecode.ticks());
        f.starts.push_back(ts.sp_phrase_start.value_or(-1));
    }
    return f;
}

Flags chart_flags(int64_t length) {
    std::string lines;
    for (int64_t t = kFirstChord; t <= kLastChord; t += kStep) {
        if (t == kPhraseStart) lines += testchart::line(t, "S 2 " + std::to_string(length));
        lines += testchart::line(t, "N 1 0");
    }
    return flags_of(load_songbytes_chart(
        testchart::chart_bytes(testchart::section("ExpertDrums", lines), 480), true, true));
}

Flags mid_flags(int64_t length) {
    struct Ev {
        int64_t tick;
        int order;  // at one tick: phrase on, phrase off, then the note
        std::vector<uint8_t> bytes;
    };
    std::vector<Ev> evs;
    for (int64_t t = kFirstChord; t <= kLastChord; t += kStep)
        evs.push_back({t, 2, {0x90, 97, 100}});           // Expert red pad
    evs.push_back({kPhraseStart, 0, {0x90, 116, 100}});   // SP phrase on
    evs.push_back({kPhraseStart + length, 1, {0x80, 116, 0}});  // SP phrase off
    std::stable_sort(evs.begin(), evs.end(), [](const Ev& a, const Ev& b) {
        return a.tick != b.tick ? a.tick < b.tick : a.order < b.order;
    });
    std::vector<uint8_t> track =
        testmidi::concat({testmidi::track_name("PART DRUMS"), testmidi::set_tempo()});
    int64_t at = 0;
    for (const Ev& e : evs) {
        const std::vector<uint8_t> d = testmidi::varlen(static_cast<uint32_t>(e.tick - at));
        track.insert(track.end(), d.begin(), d.end());
        track.insert(track.end(), e.bytes.begin(), e.bytes.end());
        at = e.tick;
    }
    const std::vector<uint8_t> eot = testmidi::end_of_track();
    track.insert(track.end(), eot.begin(), eot.end());
    return flags_of(load_songbytes_mid(testmidi::smf(track), true, true));
}

}  // namespace

TEST_CASE("phrase end: a phrase ending on or between chords flags the last chord inside") {
    for (int64_t length : {480, 240}) {  // ends on the chord at 1920; ends at 1680
        CAPTURE(length);
        const Flags want{{1440}, {1440}};
        CHECK(chart_flags(length) == want);
        CHECK(mid_flags(length) == want);
    }
}

TEST_CASE("phrase end: a phrase running past the last note flags that note") {
    // 1440 + 1440 = 2880, a beat after the last chord.
    const Flags want{{2400}, {1440}};
    CHECK(chart_flags(1440) == want);
    CHECK(mid_flags(1440) == want);
}

TEST_CASE("phrase end: a zero-length phrase on a chord flags nothing") {
    CHECK(chart_flags(0) == Flags{});
    CHECK(mid_flags(0) == Flags{});
}
