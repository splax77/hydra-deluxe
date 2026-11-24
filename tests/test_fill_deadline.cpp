// Tests for the two fill-spawn deadline rules (search/graph.h
// activation_fill_deadline_ms).
//
// A fill only spawns if the SP meter was full by some deadline. Clone Hero 1.1
// puts that deadline kCh11FillLeadBeats beats before the fill starts
// (docs/clone-hero/04-fills.md FL-19, FL-20). Clone Hero 1.0's rule is FL-42.
//
// The 1.1 test walks every fill of every corpus chart and checks each
// deadline against the game's rule on that fill's own ticks and timing. The
// 1.0 tests are hand-worked cases: each comment gives its inputs, then works
// the game's own steps (docs/clone-hero/code/4045/fill_generator.cs lines 41
// and 69, fill_deadline.cs lines 21 to 29) to the expected value.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "search/graph.h"
#include "ui/library_parts.h"  // legacy_fills_help_text
#include "core/model.h"        // fill_start_tick, fill_e_offset, fill_refuses
#include "core/song.h"
#include "corpus_util.h"

using hydra::FillDeadlineRule;
using hydra::FillRuleNameStyle;
using hydra::SongTiming;

namespace {

// The tolerance between a deadline and the value it must be: far above a
// double's rounding, far below a tick at any corpus tempo.
constexpr double kSameDeadline = 1e-9;

// A flat song: one meter section, one tempo section. 4/4 at `res` ticks per
// quarter note is 4*res ticks per measure.
SongTiming flat_timing(int64_t res, double bpm) {
    std::map<int64_t, int64_t> tpm{{0, 4 * res}};
    std::map<int64_t, double> bpm_map{{0, bpm}};
    return SongTiming(res, tpm, bpm_map, {{0, {4, 4}}});
}

double ch10(const SongTiming& t, int64_t fill_end, int64_t fill_length) {
    return hydra::activation_fill_deadline_ms(t, fill_end, fill_length,
                                              FillDeadlineRule::Ch10);
}

double ch11(const SongTiming& t, int64_t fill_end, int64_t fill_length) {
    return hydra::activation_fill_deadline_ms(t, fill_end, fill_length,
                                              FillDeadlineRule::Ch11);
}

// Calls visit(timing, fill_end, fill_length) for every fill of every corpus
// chart: the activation chord's tick and its activation_length, as the
// graph passes them (ScoreGraph::add_act_edge). The fast-tempo fixtures
// (testdata/input/test_fast_tempo, 2,000 and 4,000 BPM) sit in a folder with
// no song.ini, so the corpus scan never lists them; they are read by path.
template <typename Visit>
void for_each_corpus_fill(Visit&& visit) {
    std::vector<std::string> charts = corpus::chart_paths();
    for (const auto& entry :
         std::filesystem::directory_iterator(corpus::root() + "/test_fast_tempo"))
        charts.push_back(entry.path().string());
    for (const std::string& chart : charts) {
        CAPTURE(chart);
        const hydra::Song& song = corpus::song(chart, hydra::ChartReading{});
        for (const hydra::SongTimestamp& ts : song.sequence)
            if (ts.has_activation())
                visit(song.timing(), ts.timecode.ticks(), *ts.activation_length);
    }
}

}  // namespace

// In these cases a beat is `res` ticks, so at B BPM a tick lasts
// 60000 / (B * res) ms. The game's 64th note is Mathf.RoundToInt(12 * res /
// 192) whole ticks, a half rounding to even (fill_generator.md, F13). Its fill
// length is the seconds between the fill's start tick and the activation
// chord's tick plus that 64th (fill_generator.cs line 69, F9: a float
// difference, no rounding). A fill is refused when it starts before now plus
// the larger of that length and 0.25 s, and is not examined at all while it
// starts more than 10 s ahead (fill_deadline.cs lines 21 to 29). So the
// latest "now" that keeps it is its start less the length held to 0.25 s to
// 10 s.

TEST_CASE("fill deadline: CH 1.0 runs a 64th note past the chord, at the chord's tempo") {
    // Inputs: res 192, 120 BPM up to tick 768 and 60 BPM from it. A fill
    // 384 ticks long ends on its chord at tick 768.
    // Start: tick 768 - 384 = 384, at 384 * 60000 / (120 * 192) = 1000 ms.
    // Chord: tick 768, at 768 * 125 / 48 = 2000 ms, where 60 BPM begins.
    // 64th: RoundToInt(12 * 192 / 192) = 12 ticks, past the chord, so at
    // 60 BPM: 12 * 60000 / (60 * 192) = 62.5 ms. Tick 780 is at 2062.5 ms.
    // Length: 2062.5 - 1000 = 1062.5 ms, inside 250 to 10000.
    // Deadline: 1000 - 1062.5 = -62.5 ms.
    // A 64th measured back from the start, at 120 BPM (31.25 ms), would give
    // -31.25 ms, so this case tells the two apart.
    const int64_t res = 192;
    std::map<int64_t, int64_t> tpm{{0, 4 * res}};
    std::map<int64_t, double> bpm_map{{0, 120.0}, {768, 60.0}};
    SongTiming t(res, tpm, bpm_map, {{0, {4, 4}}});
    CHECK(ch10(t, 768, 384) == doctest::Approx(-62.5).epsilon(kSameDeadline));
}

TEST_CASE("fill deadline: CH 1.0 lead is held at the 0.25 s floor at the least") {
    // Inputs: res 192, 120 BPM (a tick is 60000 / (120 * 192) = 125 / 48 ms).
    // A fill 48 ticks long ends at tick 7680.
    // Start: tick 7632, at 7632 * 125 / 48 = 19875 ms.
    // Length: 48 + 12 (the 64th) = 60 ticks, 156.25 ms, under the 0.25 s
    // floor, so the lead is 250 ms.
    // Deadline: 19875 - 250 = 19625 ms.
    SongTiming t = flat_timing(192, 120.0);
    CHECK(ch10(t, 7680, 48) == doctest::Approx(19625.0).epsilon(kSameDeadline));
}

TEST_CASE("fill deadline: CH 1.0 lead is held at the 10 s ceiling at the most") {
    // Inputs: res 192, 150 BPM (a tick is 60000 / (150 * 192) = 25 / 12 ms).
    // A fill 4,992 ticks (26 beats) long ends at tick 9984.
    // Start: tick 4992, at 26 beats * 400 ms = 10400 ms.
    // Length: 4992 + 12 = 5004 ticks, 10425 ms, over the 10 s ceiling, so
    // the lead is 10000 ms.
    // Deadline: 10400 - 10000 = 400 ms.
    SongTiming t = flat_timing(192, 150.0);
    CHECK(ch10(t, 9984, 4992) == doctest::Approx(400.0).epsilon(kSameDeadline));
}

TEST_CASE("fill deadline: CH 1.0's 64th note is a whole tick, a half rounding to even") {
    // The rounding itself: 12 * res / 192 for res 192, 24, 40, 100 and 480 is
    // 12, 1.5, 2.5, 6.25 and 30; half to even makes them 12, 2, 2, 6 and 30.
    CHECK(hydra::ch10_sixty_fourth_ticks(192) == 12);
    CHECK(hydra::ch10_sixty_fourth_ticks(24) == 2);
    CHECK(hydra::ch10_sixty_fourth_ticks(40) == 2);
    CHECK(hydra::ch10_sixty_fourth_ticks(100) == 6);
    CHECK(hydra::ch10_sixty_fourth_ticks(480) == 30);

    // Through the deadline, res 24: 120 BPM, a tick is 60000 / (120 * 24) =
    // 125 / 6 ms. A fill 22 ticks long ends at tick 96. Start: tick 74.
    // Length: 22 + 2 = 24 ticks, 500 ms, inside the clamp. Deadline: tick
    // 74 - 24 = 50, at 50 * 125 / 6 = 3125 / 3 ms (1041.67). A 1.5-tick 64th
    // would give tick 50.5, a 1-tick one tick 51.
    SongTiming t24 = flat_timing(24, 120.0);
    CHECK(ch10(t24, 96, 22) == doctest::Approx(3125.0 / 3.0).epsilon(kSameDeadline));

    // Res 40, where the half rounds down: 120 BPM, a tick is 60000 /
    // (120 * 40) = 12.5 ms. A fill 38 ticks long ends at tick 160. Start:
    // tick 122, at 1525 ms. Length: 38 + 2 = 40 ticks, 500 ms. Deadline:
    // 1525 - 500 = 1025 ms. Rounding the half up (3 ticks) would give
    // 1012.5 ms.
    SongTiming t40 = flat_timing(40, 120.0);
    CHECK(ch10(t40, 160, 38) == doctest::Approx(1025.0).epsilon(kSameDeadline));
}

TEST_CASE("fill deadline: CH 1.0 never refuses a fill that starts over 10 s after SP is ready") {
    // The ceiling case above: res 192, 150 BPM, a 4,992-tick fill ending at
    // tick 9984 starts at 10400 ms and has its deadline at 400 ms. The game
    // does not examine a fill that starts more than 10 s after now
    // (fill_deadline.cs lines 21 to 24), so SP ready at 399 ms (10,001 ms
    // before the start) keeps the fill. Once the start is 10 s away or less,
    // the fill's 10,425 ms length is more than the time left, so SP ready at
    // 401 ms (9,999 ms before the start) loses it. The window is 0 here: the
    // game's own check, with no early-fill window of Hydra's.
    SongTiming t = flat_timing(192, 150.0);
    const double deadline = ch10(t, 9984, 4992);
    CHECK_FALSE(hydra::fill_refuses(hydra::fill_e_offset(deadline, 399.0), 0.0));
    CHECK(hydra::fill_refuses(hydra::fill_e_offset(deadline, 401.0), 0.0));
}

namespace {

// A flat 4/4 song at `res` ticks per beat and `bpm`, timed at `percent` song
// speed through Song::set_speed_percent, the one owner of the speed.
hydra::Song flat_song_at_speed(int64_t res, double bpm, int percent) {
    hydra::Song song(res);
    song.bpm_changes[0] = bpm;
    song.set_speed_percent(percent);
    song.build_timing();
    return song;
}

// A 1.0 fill's lead: how long before its start, in ms, the meter must be
// full. The start is the fill's start tick read on the same timing.
double ch10_lead(const SongTiming& t, int64_t fill_end, int64_t fill_length) {
    return t.timecode(hydra::fill_start_tick(fill_end, fill_length)).ms() -
           ch10(t, fill_end, fill_length);
}

}  // namespace

TEST_CASE("fill deadline: CH 1.0's floor, ceiling and fill length are played seconds at 50% and 200%") {
    // FL-48 on docs/clone-hero/04-fills.md: the game's deadline compares the
    // speed-divided song clock with fill times counted at the speed, so its
    // 0.25 s floor, 10 s ceiling and fill length are all seconds of played
    // time. Inputs: res 192, 120 BPM, every fill ending at tick 19200, whose
    // 64th note is 12 ticks. At 100% a tick is 125 / 48 ms; at 50% it lasts
    // twice that, at 200% half.
    const int64_t res = 192;
    const int64_t fill_end = 19200;
    const hydra::Song normal = flat_song_at_speed(res, 120.0, hydra::kNormalSpeedPercent);
    const hydra::Song half = flat_song_at_speed(res, 120.0, 50);
    const hydra::Song twice = flat_song_at_speed(res, 120.0, 200);
    auto lead = [&](const hydra::Song& song, int64_t length) {
        return ch10_lead(song.timing(), fill_end, length);
    };

    // Inside the clamp at every speed: a 192-tick fill runs 204 ticks,
    // 531.25 ms at 100%, 1062.5 ms at 50% and 265.625 ms at 200%. The lead
    // is the same ticks in played time.
    for (const hydra::Song* song : {&half, &twice}) {
        CAPTURE(song->speed_percent());
        const double played = lead(*song, 192);
        REQUIRE(played > hydra::kCh10FillLeadFloorMs);
        REQUIRE(played < hydra::kCh10FillLeadCeilingMs);
        CHECK(played * song->timing().speed_ratio() ==
              doctest::Approx(lead(normal, 192)).epsilon(kSameDeadline));
    }

    // The floor is 0.25 s of played time. A 120-tick fill runs 132 ticks:
    // 343.75 ms at 100%, above the floor, and 171.875 ms at 200%, under it.
    CHECK(lead(normal, 120) > hydra::kCh10FillLeadFloorMs);
    CHECK(lead(twice, 120) == doctest::Approx(hydra::kCh10FillLeadFloorMs).epsilon(kSameDeadline));
    // A 60-tick fill runs 72 ticks: 187.5 ms at 100%, under the floor, and
    // 375 ms at 50%, above it.
    CHECK(lead(normal, 60) == doctest::Approx(hydra::kCh10FillLeadFloorMs).epsilon(kSameDeadline));
    CHECK(lead(half, 60) > hydra::kCh10FillLeadFloorMs);

    // The ceiling is 10 s of played time. A 2,292-tick fill runs 2,304
    // ticks: 6,000 ms at 100%, under the ceiling, and 12,000 ms at 50%, over
    // it.
    CHECK(lead(normal, 2292) < hydra::kCh10FillLeadCeilingMs);
    CHECK(lead(half, 2292) == doctest::Approx(hydra::kCh10FillLeadCeilingMs).epsilon(kSameDeadline));
    // A 5,748-tick fill runs 5,760 ticks: 15,000 ms at 100%, over the
    // ceiling, and 7,500 ms at 200%, under it.
    CHECK(lead(normal, 5748) == doctest::Approx(hydra::kCh10FillLeadCeilingMs).epsilon(kSameDeadline));
    CHECK(lead(twice, 5748) < hydra::kCh10FillLeadCeilingMs);
}

TEST_CASE("fill deadline: CH 1.1 is the fill's start less 4 beats") {
    // FL-19: the deadline is kCh11FillLeadBeats quarter notes before the
    // fill's start; FL-20: that lead is the resolution times 4 in every meter.
    int checked = 0;
    for_each_corpus_fill([&](const SongTiming& t, int64_t fill_end, int64_t fill_length) {
        ++checked;
        CAPTURE(fill_end);
        CAPTURE(fill_length);
        const int64_t lead_ticks = hydra::kCh11FillLeadBeats * t.tick_resolution();
        const int64_t deadline_tick = hydra::fill_start_tick(fill_end, fill_length) - lead_ticks;
        CHECK(ch11(t, fill_end, fill_length) ==
              doctest::Approx(t.timecode(deadline_tick).ms()).epsilon(kSameDeadline));
    });
    REQUIRE(checked > 0);
}

TEST_CASE("fill deadline: a fill shorter than 4 beats less a 64th is stricter under 1.1") {
    // 1.1 leads by kCh11FillLeadBeats beats (FL-19); 1.0 by the fill's length
    // plus a 64th note (FL-42). The two leads are equal for a fill of
    // kCh11FillLeadBeats beats less a 64th note. At 120 BPM, res 192, every
    // fill below stays inside FL-42's 0.25 s to 10 s clamp.
    const int64_t res = 192;
    SongTiming t = flat_timing(res, 120.0);
    const int64_t fill_end = 7680;
    const int64_t sixty_fourth = hydra::ch10_sixty_fourth_ticks(res);
    const int64_t crossover = hydra::kCh11FillLeadBeats * res - sixty_fourth;
    const int64_t half_beat = res / 2;
    const int64_t ten_beats = 10 * res;

    CHECK(ch11(t, fill_end, half_beat) < ch10(t, fill_end, half_beat));
    CHECK(ch11(t, fill_end, crossover - 1) < ch10(t, fill_end, crossover - 1));
    CHECK(ch10(t, fill_end, crossover) ==
          doctest::Approx(ch11(t, fill_end, crossover)).epsilon(kSameDeadline));
    CHECK(ch10(t, fill_end, crossover + 1) < ch11(t, fill_end, crossover + 1));
    CHECK(ch10(t, fill_end, ten_beats) < ch11(t, fill_end, ten_beats));
}

TEST_CASE("fill rule names: each rule's names are its own, on the 1.0 fills tooltip too") {
    const FillDeadlineRule rules[] = {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11};
    const std::string tooltip = hydra::ui::detail::legacy_fills_help_text();
    std::vector<std::string> names;
    for (FillDeadlineRule rule : rules) {
        const std::string long_name = hydra::fill_rule_name(rule, FillRuleNameStyle::Long);
        const std::string short_name = hydra::fill_rule_name(rule, FillRuleNameStyle::Short);
        CAPTURE(long_name);
        CAPTURE(short_name);
        CHECK(long_name != short_name);
        // The tooltip names both rules in sentences, so in the long style.
        CHECK(tooltip.find(long_name) != std::string::npos);
        names.push_back(long_name);
        names.push_back(short_name);
    }
    // No name is shared between the two rules or the two styles.
    for (size_t i = 0; i < names.size(); ++i)
        for (size_t j = i + 1; j < names.size(); ++j) CHECK(names[i] != names[j]);
    // D48 (Q13): no rule is called "(legacy)", in its names or on the tooltip.
    names.push_back(tooltip);
    for (const std::string& text : names) {
        CAPTURE(text);
        CHECK(text.find("(legacy)") == std::string::npos);
    }
}

TEST_CASE("fill rule descriptions: each rule's sentence names it and its lead") {
    const std::string tooltip = hydra::ui::detail::legacy_fills_help_text();
    const FillDeadlineRule rules[] = {FillDeadlineRule::Ch10, FillDeadlineRule::Ch11};
    for (FillDeadlineRule rule : rules) {
        const std::string sentence = hydra::fill_rule_description(rule);
        CAPTURE(sentence);
        // The sentence opens on its own rule's name.
        CHECK(sentence.rfind(hydra::fill_rule_name(rule, FillRuleNameStyle::Long), 0) == 0);
        // The 1.0 fills tooltip prints the owner's sentence whole.
        CHECK(tooltip.find(sentence) != std::string::npos);
    }
    // 1.1's lead is the beat count the deadline is computed with (FL-19).
    CHECK(hydra::fill_rule_description(FillDeadlineRule::Ch11)
              .find(std::to_string(hydra::kCh11FillLeadBeats) + " beats") != std::string::npos);
    // 1.0's lead is the fill's own length (FL-42).
    CHECK(hydra::fill_rule_description(FillDeadlineRule::Ch10).find("fill-length") !=
          std::string::npos);
}
