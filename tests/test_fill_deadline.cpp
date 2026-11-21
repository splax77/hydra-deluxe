// Tests for the two fill-spawn deadline rules (search/graph.h
// activation_fill_deadline_ms).
//
// A fill only spawns if the SP meter was full by some deadline. Clone Hero 1.1
// puts that deadline kCh11FillLeadBeats beats before the fill starts
// (docs/clone-hero/04-fills.md FL-19, FL-20). Clone Hero 1.0 puts it the
// fill's own length plus a 64th note before the fill starts, that lead held
// between 0.25 s and 10 s (FL-42).
//
// The deadline tests walk every fill of every corpus chart and check each
// deadline against the game's rule on that fill's own ticks and timing.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "search/graph.h"
#include "ui/library_parts.h"  // legacy_fills_help_text
#include "core/model.h"        // fill_start_tick
#include "core/song.h"
#include "corpus_util.h"

using hydra::FillDeadlineRule;
using hydra::FillRuleNameStyle;
using hydra::SongTiming;

namespace {

// FL-42's clamp on the 1.0 lead: "held between 0.25 s and 10 s" (the game's
// code/4045/fill_deadline.cs lines 21 to 29). graph.cpp types the same two
// numbers inside activation_fill_deadline_ms and exposes no constant.
constexpr double kCh10LeadFloorMs = 250.0;
constexpr double kCh10LeadCeilingMs = 10000.0;

// The tolerance for two deadlines computed in a different order of the same
// sums: far above a double's rounding, far below a tick at any corpus tempo.
constexpr double kSameDeadline = 1e-9;

// The open end of a band of leads that has only one bound.
constexpr double kNoBound = std::numeric_limits<double>::infinity();

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

// FL-42's 1.0 lead before its clamp: from a 64th note (the resolution / 16
// ticks) before the fill's start to the fill's end, in real time.
double ch10_unclamped_lead_ms(const SongTiming& t, int64_t fill_end, int64_t fill_length) {
    const double sixty_fourth_ticks = static_cast<double>(t.tick_resolution()) / 16.0;
    const double before_start =
        static_cast<double>(hydra::fill_start_tick(fill_end, fill_length)) - sixty_fourth_ticks;
    return t.ms_index().at(fill_end) - t.ms_index().ms_at_tick_f(before_start);
}

double fill_start_ms(const SongTiming& t, int64_t fill_end, int64_t fill_length) {
    return t.ms_index().at(hydra::fill_start_tick(fill_end, fill_length));
}

// Checks the 1.0 deadline of every corpus fill whose unclamped lead lies in
// [lo, hi]: it is the fill's start less lead(unclamped lead). Returns how
// many fills it checked.
template <typename Lead>
int check_ch10_deadlines(double lo, double hi, Lead lead) {
    int checked = 0;
    for_each_corpus_fill([&](const SongTiming& t, int64_t fill_end, int64_t fill_length) {
        const double unclamped = ch10_unclamped_lead_ms(t, fill_end, fill_length);
        if (unclamped < lo || unclamped > hi) return;
        ++checked;
        CAPTURE(fill_end);
        CAPTURE(fill_length);
        CHECK(ch10(t, fill_end, fill_length) ==
              doctest::Approx(fill_start_ms(t, fill_end, fill_length) - lead(unclamped))
                  .epsilon(kSameDeadline));
    });
    return checked;
}

}  // namespace

TEST_CASE("fill deadline: CH 1.0 is the fill's start less its length and a 64th note") {
    // Every fill whose lead lies inside FL-42's clamp: the lead is the fill's
    // own length plus a 64th note at that fill's tempo.
    const int checked = check_ch10_deadlines(kCh10LeadFloorMs, kCh10LeadCeilingMs,
                                             [](double unclamped) { return unclamped; });
    REQUIRE(checked > 0);
}

TEST_CASE("fill deadline: CH 1.0 lead is held at 250 ms at the least") {
    // The fast-tempo fixtures' fills (a 384-tick fill at 4,000 BPM leads by
    // about 31 ms) fall under FL-42's 0.25 s floor.
    const int checked = check_ch10_deadlines(-kNoBound, kCh10LeadFloorMs,
                                             [](double) { return kCh10LeadFloorMs; });
    REQUIRE(checked > 0);
}

TEST_CASE("fill deadline: CH 1.0 lead is held at 10000 ms at the most") {
    // Chaotrope - Senescence (shortened) has a 4,992-tick fill (26 beats) at
    // 150 BPM, res 192: its unclamped lead is (4992 + 12) ticks at 2.083 ms a
    // tick, about 10,425 ms, over FL-42's 10 s ceiling.
    const int checked = check_ch10_deadlines(kCh10LeadCeilingMs, kNoBound,
                                             [](double) { return kCh10LeadCeilingMs; });
    REQUIRE(checked > 0);
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
    const int64_t sixty_fourth = res / 16;  // whole: every corpus resolution is a multiple of 16
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
