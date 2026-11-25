// The game's per-note hit window and the Dynamic Hit Window row
// (core/note_window.h).

#include "doctest.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/model.h"
#include "core/note_window.h"
#include "core/song.h"
#include "record_fixtures.h"  // test::build_fixture_song
#include "corpus_util.h"  // corpus::read_bytes
#include "source_tree.h"  // for_each_source_file, root: the shared walk of the repo

using namespace hydra;
namespace fs = std::filesystem;

namespace {

// A chart at 120 BPM, 192 ticks a beat (2.604 ms a tick), with one Red chord
// on each tick of `ticks` (test::build_fixture_song).
Song red_chords(const std::vector<int64_t>& ticks) {
    std::vector<test::FixtureNote> notes;
    for (int64_t tick : ticks) notes.push_back({tick});
    return test::build_fixture_song(192, 120.0, notes);
}

// A side at the floor and at the cap: half the whole window when both
// half-gaps are held at the front clamp, and when both are held at the back
// clamp (TM-22). The first test checks those two whole windows against the
// values TM-21's page prints.
const double kFloorSideMs = whole_window_ms(2 * kWindowFrontMs) / 2;
const double kCapSideMs = whole_window_ms(2 * kWindowBackMs) / 2;
const doctest::Approx kFloorSide = doctest::Approx(kFloorSideMs);
const doctest::Approx kCapSide = doctest::Approx(kCapSideMs);

// The "### <id>" block of the page docs/clone-hero/03-timing.md, from its
// heading to the next heading.
std::string timing_page_block(const std::string& id) {
    const std::string page =
        corpus::read_bytes((sourcetree::root() / "docs" / "clone-hero" / "03-timing.md").string());
    const std::string heading = "### " + id + " ";
    const size_t at = page.find(heading);
    const std::string missing = "03-timing.md has no block " + id;
    REQUIRE_MESSAGE(at != std::string::npos, missing);
    const size_t end = page.find("\n### ", at + heading.size());
    return page.substr(at, end == std::string::npos ? std::string::npos : end - at);
}

// One worked value a page prints: the whole window `ms`, to `decimals`
// places, at the summed gap `x`.
struct PageValue {
    double ms = 0.0;
    int decimals = 0;
    double x = 0.0;
};

// Every "<ms> ms at x = <x>" in `block`, in page order.
std::vector<PageValue> worked_values(const std::string& block) {
    std::vector<PageValue> out;
    const std::regex value(R"(([0-9]+)\.([0-9]+) ms at x = ([0-9]+))");
    for (std::sregex_iterator it(block.begin(), block.end(), value), stop; it != stop; ++it) {
        const std::smatch& m = *it;
        PageValue v;
        v.ms = std::stod(m[1].str() + "." + m[2].str());
        v.decimals = static_cast<int>(m[2].length());
        v.x = std::stod(m[3].str());
        out.push_back(v);
    }
    return out;
}

// Equal to nine places: for an expected value worked out from the same
// inputs the code is given.
doctest::Approx pinned(double v) { return doctest::Approx(v).epsilon(1e-9); }

// Chords 768 ticks (2,000 ms) apart, so every side is the cap. The
// activation is on 768; a SqIn set the step on 2304 and a SqOut squeezed out
// the chord on 3840. Each scale's two sides differ, so reading the wrong one
// shows.
struct TwoSqueezes {
    Song song;
    Activation act;
};
TwoSqueezes two_squeezes() {
    TwoSqueezes t{red_chords({0, 768, 1536, 2304, 3072, 3840}), Activation{}};
    Activation& a = t.act;
    a.timecode = t.song.timecode(768);
    a.bank_rise_ticks = {0, 768};  // two bars, so the ready point is known
    a.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {2304, 3900, SpEndKind::SqIn}};
    SPSqueeze in{};
    in.kind = SqueezeKind::SqIn;
    in.offset_ms = 20.0;
    in.transfer = TransferScale{3.0, 1.5};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -100.0;
    a.sqinouts = {in, out};
    a.sqout_tick = 3840;
    a.transfer_post = TransferScale{2.0, 4.0};
    return t;
}

}  // namespace

TEST_CASE("note window: the window formula and clamps are pinned") {
    // Every worked value TM-21's evidence line prints, each to the places
    // the page gives it.
    const std::vector<PageValue> values = worked_values(timing_page_block("TM-21"));
    REQUIRE_FALSE(values.empty());
    std::set<double> xs;
    for (const PageValue& v : values) {
        CAPTURE(v.x);
        CAPTURE(v.ms);
        xs.insert(v.x);
        CHECK(std::abs(whole_window_ms(v.x) - v.ms) <= 0.5 * std::pow(10.0, -v.decimals));
    }
    // The page's values include the summed gaps at both clamps, so the floor
    // and the cap of a side above stand on the page.
    CHECK(xs.count(2 * kWindowFrontMs) == 1);
    CHECK(xs.count(2 * kWindowBackMs) == 1);
    // TM-22: a side is held at the floor when both neighbours are close, and
    // at the cap when both are far.
    CHECK(window_side_ms(1.0, 1.0) == kFloorSide);
    CHECK(window_side_ms(10000.0, 10000.0) == kCapSide);
    // TM-19: a missing neighbour, or one at the same time, gives the back;
    // a close one is held at the front; between the clamps it is half the gap.
    CHECK(half_gap_ms(std::nullopt) == kWindowBackMs);
    CHECK(half_gap_ms(0.0) == kWindowBackMs);
    CHECK(half_gap_ms(10.0) == kWindowFrontMs);
    CHECK(half_gap_ms(100.0) == 100.0 / 2);
}

TEST_CASE("note window: the formula's two coefficients are written in one C++ file") {
    // The two coefficients kFormulaLinear and kFormulaSquare, each as its
    // first five significant digits, read from the constant itself. A file
    // holds one when that digit run appears in it with every '.' taken out,
    // so 0.011092437 and 1.1092e-2 both count. kFormulaScale (170) and
    // kFormulaOffsetMs (20) are too common to search for. Only C++ under src/
    // and tools/ counts: tools/ch_probe/SPEC.md and the schedule files under
    // tools/ch_probe/tests/ carry the coefficients as prose and data.
    std::vector<std::string> needles;
    for (const double coefficient : {kFormulaLinear, kFormulaSquare}) {
        char buf[32];
        std::snprintf(buf, sizeof buf, "%.4e", coefficient);  // such as "1.1092e-02"
        const std::string mantissa(buf, std::strchr(buf, 'e'));
        std::string digits;
        for (const char ch : mantissa)
            if (ch != '.') digits += ch;
        REQUIRE(digits.size() == 5);
        needles.push_back(digits);
    }
    std::set<std::string> holders;
    sourcetree::for_each_source_file([&](const fs::path& path, const std::string& rel) {
        if (rel.rfind("src/", 0) != 0 && rel.rfind("tools/", 0) != 0) return;
        const std::string ext = path.extension().string();
        if (ext != ".h" && ext != ".cpp") return;
        std::string text = corpus::read_bytes(path.string());
        text.erase(std::remove(text.begin(), text.end(), '.'), text.end());
        for (const std::string& needle : needles)
            if (text.find(needle) != std::string::npos) holders.insert(rel);
    });
    CHECK(holders == std::set<std::string>{"src/core/note_window.h"});
}

TEST_CASE("note window: chord_times lists the song's chords") {
    const Song song = red_chords({0, 20, 40, 2000});
    const ChordTimes chords = chord_times(song);
    REQUIRE(chords.ticks.size() == 4);
    REQUIRE(chords.ms.size() == 4);
    REQUIRE(chords.sides_ms.size() == 4);
    CHECK(chords.ticks[1] == 20);
    CHECK(chords.ms[3] == song.timecode(2000).ms());
    CHECK(chords.sides_ms[3] == kCapSide);  // far from chord 2, and last
    CHECK(chords.sides_ms[1] == kFloorSide);  // 52.08 ms from both neighbours
    CHECK(chords.at(40) == std::optional<int>(2));
    CHECK_FALSE(chords.at(41).has_value());
    CHECK_FALSE(chords.at(5000).has_value());
}

TEST_CASE("note window: chord_times carries the chord at each place") {
    Song song = red_chords({0, 20, 40});
    song.sequence[1].chord.add_note(NoteColor::Kick);
    const ChordTimes chords = chord_times(song);
    REQUIRE(chords.chords.size() == 3);
    CHECK(chords.chords[0].notationstr() == "[ R   ]");
    CHECK(chords.chords[1].notationstr() == "[KR   ]");
    // One lookup by tick gives both the side and the chord.
    const std::optional<int> c = chords.at(20);
    REQUIRE(c.has_value());
    CHECK(chords.chords[static_cast<size_t>(*c)].notationstr() == "[KR   ]");
}

TEST_CASE("note window: a squeeze's safe sum is the anchor's side through the ratio, plus the squeezed side, less the timing") {
    // Free inputs, each term a different size, so a dropped or swapped term
    // shows.
    const double anchor_side = kCapSideMs;
    const double ratio = 1.033;
    const double squeezed_side = kFloorSideMs;
    const double room = squeeze_safe_ms(anchor_side, ratio, squeezed_side, 0.0);
    CHECK(room == pinned(anchor_side * ratio + squeezed_side));
    const double needed = 30.0;
    CHECK(squeeze_safe_ms(anchor_side, ratio, squeezed_side, needed) == pinned(room - needed));
    // The sum is safe while the timing needs less than the room the two
    // sides give, and short once it needs more.
    CHECK_FALSE(safe_is_short(squeeze_safe_ms(anchor_side, ratio, squeezed_side, room - 1.0)));
    CHECK(safe_is_short(squeeze_safe_ms(anchor_side, ratio, squeezed_side, room + 1.0)));
}

TEST_CASE("note window: exactly 0 is safe, anything below is short") {
    CHECK_FALSE(safe_is_short(0.0));
    CHECK(safe_is_short(std::nextafter(0.0, -1.0)));
    WindowEnds row;
    row.safe_ms = 0.0;
    CHECK_FALSE(row.is_short());
    row.safe_ms = -0.04;
    CHECK(row.is_short());
}

TEST_CASE("note window: each squeeze reads its own side of its own scale") {
    const TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    CHECK(t.act.squeeze_chord_tick(0) == std::optional<int64_t>(2304));
    CHECK(t.act.squeeze_chord_tick(1) == std::optional<int64_t>(3840));
    CHECK_FALSE(t.act.squeeze_chord_tick(2).has_value());

    // The SqOut's anchor is pressed early, so its sum counts the early side
    // of transfer_post, not the late one.
    const SPSqueeze& sqout = t.act.sqinouts[1];
    REQUIRE(squeeze_presses(sqout).anchor == Press::Early);
    const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);
    REQUIRE(out.has_value());
    REQUIRE(out->ends.size() == 2);
    CHECK(out->ends[0].side_ms == kCapSide);
    CHECK(out->ends[1].side_ms == kCapSide);
    const auto out_sum = [&](double ratio) {
        return squeeze_safe_ms(out->ends[0].side_ms, ratio, out->ends[1].side_ms, sqout.difficulty());
    };
    CHECK(out->safe_ms == pinned(out_sum(t.act.transfer_post->early)));
    CHECK(out->safe_ms != doctest::Approx(out_sum(t.act.transfer_post->late)));

    // The SqIn's anchor is pressed late, so its sum counts the late side of
    // its own transfer: not its early side, and never transfer_post's.
    const SPSqueeze& sqin = t.act.sqinouts[0];
    REQUIRE(squeeze_presses(sqin).anchor == Press::Late);
    const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);
    REQUIRE(in.has_value());
    REQUIRE(in->ends.size() == 2);
    CHECK(in->ends[1].side_ms == kCapSide);
    const auto in_sum = [&](double ratio) {
        return squeeze_safe_ms(in->ends[0].side_ms, ratio, in->ends[1].side_ms, sqin.difficulty());
    };
    CHECK(in->safe_ms == pinned(in_sum(sqin.transfer->late)));
    CHECK(in->safe_ms != doctest::Approx(in_sum(sqin.transfer->early)));
    CHECK(in->safe_ms != doctest::Approx(in_sum(t.act.transfer_post->late)));

    // A place past the activation's squeezes has no row.
    CHECK_FALSE(squeeze_window_ends(t.act, 2, chords).has_value());
}

TEST_CASE("note window: a cap-clamped squeeze is anchored on the overfill chord") {
    // The chord on 1536 has a neighbour 20 ticks (52.08 ms) after it, so its
    // side is not the activation chord's.
    const Song song = red_chords({0, 768, 1536, 1556, 3072});
    const ChordTimes chords = chord_times(song);
    Activation act;
    act.timecode = song.timecode(768);
    act.sp_end_steps = {{768, 3000, SpEndKind::Activation}, {1536, 3082, SpEndKind::Clamped}};
    SPSqueeze out{};
    out.kind = SqueezeKind::SqOut;
    out.offset_ms = -26.04;
    act.sqinouts.push_back(out);
    act.sqout_tick = 3072;
    act.transfer_post = TransferScale{1.0, 1.0};

    REQUIRE(act.squeeze_anchor_tick(0) == std::optional<int64_t>(1536));
    const double overfill_side = chords.sides_ms[static_cast<size_t>(*chords.at(1536))];
    const double activation_side = chords.sides_ms[static_cast<size_t>(*chords.at(768))];
    REQUIRE(overfill_side != activation_side);
    // The anchor is always the frontend end.
    const std::optional<WindowEnds> row = squeeze_window_ends(act, 0, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 2);
    CHECK(row->ends[0].role == EndRole::Frontend);
    CHECK(row->ends[0].tick == 1536);
    CHECK(row->ends[0].side_ms == overfill_side);
}

TEST_CASE("note window: a row the record cannot name is left out") {
    const Song song = red_chords({0, 768, 1536, 2304, 3072, 3840});
    const ChordTimes chords = chord_times(song);

    SUBCASE("an unset scale") {
        TwoSqueezes t = two_squeezes();
        t.act.transfer_post.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        t.act.sqinouts[0].transfer.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 0, chords).has_value());
    }
    SUBCASE("a squeeze that names no chord") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick.reset();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
    }
    SUBCASE("a squeeze with no end in the record") {
        TwoSqueezes t = two_squeezes();
        t.act.sp_end_steps.clear();
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        CHECK_FALSE(squeeze_window_ends(t.act, 0, chords).has_value());
    }
    SUBCASE("a tick with no chord on it") {
        TwoSqueezes t = two_squeezes();
        t.act.sqout_tick = 3000;
        CHECK_FALSE(squeeze_window_ends(t.act, 1, chords).has_value());
        t.act.bank_rise_ticks = {0, 3000};  // the bar arrived where no chord sits
        CHECK_FALSE(early_fill_window_end(t.act, chords).has_value());
    }
    SUBCASE("fewer stored bar arrivals than the activation level") {
        TwoSqueezes t = two_squeezes();
        t.act.bank_rise_ticks = {0};
        CHECK_FALSE(early_fill_window_end(t.act, chords).has_value());
    }
}

// ---- the one owners of each note's press and each note's word --------------

namespace {

SPSqueeze squeeze_at(SqueezeKind kind, double offset_ms) {
    SPSqueeze q{};
    q.kind = kind;
    q.offset_ms = offset_ms;
    return q;
}

// A stored scale whose two sides differ, so reading the wrong one shows.
const TransferScale kTwoSidedScale{1.0044, 0.9970};

}  // namespace

TEST_CASE("note window: squeeze_presses follows the sentence, not the kind") {
    // Each line is (kind, offset_ms) -> the way the squeezed note is pressed,
    // the word the squeeze sentence prints. Each kind is pressed both ways,
    // so a pick by kind alone fails.
    struct Case {
        SqueezeKind kind;
        double offset_ms;
        Press squeezed;
    };
    const Case cases[] = {
        // A squeeze in: "more than ... early" while its figure is 0 or more.
        {SqueezeKind::SqIn, 0.001, Press::Early},
        {SqueezeKind::SqIn, 0.0, Press::Early},
        {SqueezeKind::SqIn, -0.001, Press::Late},
        // A squeeze out: its figure is the offset negated.
        {SqueezeKind::SqOut, 0.001, Press::Early},
        {SqueezeKind::SqOut, 0.0, Press::Late},
        {SqueezeKind::SqOut, -0.001, Press::Late},
    };
    for (const Case& c : cases) {
        CAPTURE(c.offset_ms);
        CAPTURE(static_cast<int>(c.kind));
        const SqueezePresses p = squeeze_presses(squeeze_at(c.kind, c.offset_ms));
        CHECK(p.squeezed == c.squeezed);
        CHECK(p.anchor != p.squeezed);
    }
}

TEST_CASE("note window: role_word gives the row's words") {
    CHECK(std::string(role_word(EndRole::Frontend)) == "frontend");
    CHECK(std::string(role_word(EndRole::Backend)) == "backend");
    CHECK(std::string(role_word(EndRole::SpReady)).empty());  // the user's ruling: "No word"
}

TEST_CASE("note window: squeeze_window_ends lists the anchor, then the squeezed note") {
    // two_squeezes' activation, on a song whose squeezed-out chord (3840) has
    // a neighbour 20 ticks after it.
    const TwoSqueezes t = two_squeezes();
    const Song song = red_chords({0, 768, 1536, 2304, 3072, 3840, 3860});
    const ChordTimes chords = chord_times(song);
    const double squeezed_gap_before = song.timecode(3840).ms() - song.timecode(3072).ms();
    const double squeezed_gap_after = song.timecode(3860).ms() - song.timecode(3840).ms();

    // The squeeze out: the anchor (768) is the frontend and the squeezed
    // chord (3840) the backend.
    const std::optional<WindowEnds> out = squeeze_window_ends(t.act, 1, chords);
    REQUIRE(out.has_value());
    REQUIRE(out->ends.size() == 2);
    CHECK(out->ends[0].role == EndRole::Frontend);
    CHECK(out->ends[0].tick == 768);
    CHECK(out->ends[0].side_ms == kCapSide);
    CHECK(out->ends[1].role == EndRole::Backend);
    CHECK(out->ends[1].tick == 3840);
    // The backend's side is its own chord's, from that chord's two gaps; the
    // near neighbour keeps it below the cap.
    CHECK(out->ends[1].side_ms == pinned(window_side_ms(squeezed_gap_before, squeezed_gap_after)));
    CHECK(out->ends[1].side_ms != kCapSide);

    // The squeeze in has a timing, so its squeezed chord (2304) is pressed
    // early. It is still the backend: the row names the anchor (768) the
    // frontend whatever way either note is pressed.
    REQUIRE(squeeze_presses(t.act.sqinouts[0]).squeezed == Press::Early);
    const std::optional<WindowEnds> in = squeeze_window_ends(t.act, 0, chords);
    REQUIRE(in.has_value());
    REQUIRE(in->ends.size() == 2);
    CHECK(in->ends[0].role == EndRole::Frontend);
    CHECK(in->ends[0].tick == 768);
    CHECK(in->ends[1].role == EndRole::Backend);
    CHECK(in->ends[1].tick == 2304);

    CHECK_FALSE(squeeze_window_ends(t.act, 2, chords).has_value());
}

TEST_CASE("note window: the anchor's scale side follows its press") {
    // Every side here is the cap, and both stored scales are kTwoSidedScale.
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    t.act.transfer_post = kTwoSidedScale;
    t.act.sqinouts[0].transfer = kTwoSidedScale;
    const double anchor_side = chords.sides_ms[static_cast<size_t>(*chords.at(768))];

    // Squeeze `k` with its offset set to `offset_ms`: the figure it needs,
    // the way its anchor is pressed, and its row's safe sum.
    struct Row {
        double needed;
        Press anchor;
        double safe;
    };
    const auto row_at = [&](size_t k, double offset_ms) {
        t.act.sqinouts[k].offset_ms = offset_ms;
        const SPSqueeze& q = t.act.sqinouts[k];
        const std::optional<WindowEnds> row = squeeze_window_ends(t.act, k, chords);
        REQUIRE(row.has_value());
        CHECK(row->ends[0].role == EndRole::Frontend);
        CHECK(row->ends[0].tick == 768);  // the anchor, whichever way it is pressed
        return Row{q.difficulty(), squeeze_presses(q).anchor, row->safe_ms};
    };

    // Between an offset that needs a timing and one that needs none, the
    // anchor's press turns round, so the sum moves by the anchor's side times
    // the change of scale side, less the change of the figure needed. A side
    // picked by kind would move it by the figure alone.
    SUBCASE("a squeeze in") {
        const Row timed = row_at(0, 20.0);
        const Row untimed = row_at(0, -20.0);
        REQUIRE(timed.anchor == Press::Late);
        REQUIRE(untimed.anchor == Press::Early);
        CHECK(untimed.safe - timed.safe ==
              doctest::Approx(anchor_side * (kTwoSidedScale.early - kTwoSidedScale.late) -
                              (untimed.needed - timed.needed)));
    }
    SUBCASE("a squeeze out") {
        const Row timed = row_at(1, -100.0);
        const Row untimed = row_at(1, 10.0);
        REQUIRE(timed.anchor == Press::Early);
        REQUIRE(untimed.anchor == Press::Late);
        CHECK(untimed.safe - timed.safe ==
              doctest::Approx(anchor_side * (kTwoSidedScale.late - kTwoSidedScale.early) -
                              (untimed.needed - timed.needed)));
    }
}

TEST_CASE("note window: a squeeze's row keeps the multiplier its sum used") {
    // two_squeezes' squeeze in (0) and squeeze out (1). Each scale's sides
    // differ, so a row that kept one multiplier and summed with another shows.
    const TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    for (size_t k : {size_t{0}, size_t{1}}) {
        CAPTURE(k);
        const SPSqueeze& q = t.act.sqinouts[k];
        const std::optional<WindowEnds> row = squeeze_window_ends(t.act, k, chords);
        REQUIRE(row.has_value());
        REQUIRE(row->ends.size() == 2);
        REQUIRE(row->priced.has_value());
        REQUIRE(row->frontend_effective_ms.has_value());
        // The row keeps the owner's answer for this squeeze.
        const std::optional<PricedScale> owner = squeeze_multiplier(t.act, k);
        REQUIRE(owner.has_value());
        CHECK(row->priced->press == owner->press);
        CHECK(row->priced->ratio == owner->ratio);
        const double front = row->ends[0].side_ms;
        const double back = row->ends[1].side_ms;
        // The sum is priced by the multiplier the row keeps.
        CHECK(row->safe_ms == pinned(squeeze_safe_ms(front, row->priced->ratio, back, q.difficulty())));
        // The Effective figure is the product that sum adds.
        CHECK(*row->frontend_effective_ms == pinned(scaled_anchor_ms(front, row->priced->ratio)));
        CHECK(row->safe_ms == pinned(*row->frontend_effective_ms + back - q.difficulty()));
    }
}

TEST_CASE("note window: an early fill's row has no multiplier") {
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);
    t.act.e_offset = -10.0;  // ready 10 ms after the deadline
    REQUIRE(t.act.is_e_critical());
    const std::optional<WindowEnds> row = early_fill_row(t.act, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 1);
    CHECK(row->ends[0].role == EndRole::SpReady);
    CHECK_FALSE(row->priced.has_value());
    CHECK_FALSE(row->frontend_effective_ms.has_value());
}

TEST_CASE("note window: the early fill end reads the SP-ready note") {
    TwoSqueezes t = two_squeezes();
    const ChordTimes chords = chord_times(t.song);

    t.act.e_offset = -10.0;  // ready 10 ms after the deadline
    REQUIRE(early_fill_difficulty(t.act.e_offset) == 10.0);
    std::optional<WindowEnds> row = early_fill_window_end(t.act, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 1);
    CHECK(row->ends[0].role == EndRole::SpReady);
    CHECK(row->ends[0].tick == 768);
    CHECK(row->ends[0].side_ms == kCapSide);
    CHECK(row->safe_ms == pinned(kCapSideMs - 10.0));

    t.act.e_offset = 10.0;  // time to spare
    REQUIRE(early_fill_difficulty(t.act.e_offset) == -10.0);
    row = early_fill_window_end(t.act, chords);
    REQUIRE(row.has_value());
    REQUIRE(row->ends.size() == 1);
    CHECK(row->ends[0].role == EndRole::SpReady);  // whichever sign the figure has
    CHECK(row->safe_ms == pinned(kCapSideMs + 10.0));
}
