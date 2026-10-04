// Hand-built songs and record fixtures shared by the test files.
//
// Two kinds of thing live here. The songs are small charts built to give the
// search one exact shape (a late squeeze-in, an early squeeze-out, a clamp).
// The fixture helpers are the one place a test writes a stored fact onto a
// hand-built Activation or Path. A test never assigns those fields itself, so
// when a stored fact changes shape only the helper's body changes.
#ifndef HYDRA_TESTS_RECORD_FIXTURES_H
#define HYDRA_TESTS_RECORD_FIXTURES_H

#include <algorithm>
#include <cstdint>
#include <map>
#include <vector>

#include "core/model.h"
#include "parse/song.h"
#include "search/engine.h"

namespace hydra::test {

// ---- songs ----------------------------------------------------------------

struct FixtureNote {
    int64_t tick;
    bool phrase = false;
    int64_t fill_length = 0;  // 0 = no fill ends here
};

// One red note per entry, flat tempo and 4/4.
inline Song build_fixture_song(int resolution, double bpm, const std::vector<FixtureNote>& notes) {
    Song song(resolution);
    song.bpm_changes[0] = bpm;
    song.build_timing();
    for (const FixtureNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        if (n.phrase) {
            ts.flag_sp = true;
            ts.sp_phrase_start = n.tick >= resolution ? n.tick - resolution : 0;
        }
        if (n.fill_length > 0) ts.activation_length = n.fill_length;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// A note on every beat from 0 to `last`, with the phrase ends and fills
// given. 240 BPM in 4/4 at 480 ticks a beat: a measure is 1920 ticks and
// 1000 ms, and one SP bar (two measures) burns 2000 ms. A note every 250 ms.
inline Song beat_song(const std::vector<int64_t>& phrases,
                      const std::vector<int64_t>& fills, int64_t last) {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= last; t += 480) {
        FixtureNote n{t};
        n.phrase = std::find(phrases.begin(), phrases.end(), t) != phrases.end();
        if (std::find(fills.begin(), fills.end(), t) != fills.end()) n.fill_length = 960;
        notes.push_back(n);
    }
    return build_fixture_song(480, 240.0, notes);
}

// The Epidermis shape (finding 5). Phrases end at 480 and 1920, so two bars
// are banked by 1000 ms. The fill at 5760 (3000 ms) activates, and two bars
// run four measures, so the plain SP end is 13440 (7000 ms). The third
// phrase ends at 13920 (7250 ms), a quarter measure after that end. The
// player squeezes it in by hitting it early. The engine extends SP one bar
// from the old end, to 17280 (9000 ms), so at the phrase the meter holds
// 1 - 0.25 / 2 = 0.875 bars.
inline Song make_late_sqin_song() {
    return beat_song({480, 1920, 13920}, {5760}, 21120);
}

// An early squeeze-out: the third phrase ends at 12960 (6750 ms), inside the
// first window. Hit late, it lands after SP ends and banks one bar instead.
// The fourth phrase, at 14400 (7500 ms), makes two bars for the second fill
// at 17280 (9000 ms). Search it with target ticks {5760, 17280}: only the
// squeeze-out path can take that second fill.
inline Song make_early_sqout_song() {
    return beat_song({480, 1920, 12960, 14400}, {5760, 17280}, 26880);
}

// Finding 52's case, at 120 BPM with 480 ticks a beat (tick t is t*500/480
// ms). Phrases end at 960 (1000 ms) and 18144 (18900 ms): SP is ready at
// 18900 ms. Fill A ends at 19200 (20000 ms) and is 480 ticks (500 ms) long.
// Fill B ends at 24960 (26000 ms) and is 3456 ticks (3600 ms) long. Fill C
// ends at 28800 (30000 ms). Under the 1.0 rule A's deadline is 18968.75 ms
// and B's is 18768.75 ms: A is shown and passed over, B never spawns. Search
// it with FillDeadlineRule::Ch10 and target ticks {28800}.
inline Song make_ch10_fill_song() {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= 38400; t += 960) {
        FixtureNote n{t};
        n.phrase = t == 960;
        if (t == 19200 || t == 28800) n.fill_length = 480;
        if (t == 24960) n.fill_length = 3456;
        notes.push_back(n);
    }
    notes.push_back({18144, true, 0});
    std::sort(notes.begin(), notes.end(),
              [](const FixtureNote& a, const FixtureNote& b) { return a.tick < b.tick; });
    return build_fixture_song(480, 120.0, notes);
}

// A note for the 192-ticks-a-beat songs below: a phrase end, a fill end
// (384 ticks long), or a plain note.
struct TailNote {
    int64_t tick;
    bool sp_phrase = false;
    bool activation = false;
};

// 192 ticks a beat, 768 a measure, with the tempo map given.
inline Song build_tempo_song(const std::vector<TailNote>& notes,
                             const std::map<int64_t, double>& bpm) {
    Song song(192);
    song.tpm_changes[0] = 768;
    for (const auto& kv : bpm) song.bpm_changes[kv.first] = kv.second;
    song.build_timing();
    for (const TailNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.sp_phrase;
        if (n.activation) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// build_tempo_song at a flat 120 BPM.
inline Song build_tail_song(const std::vector<TailNote>& notes) {
    return build_tempo_song(notes, {{0, 120.0}});
}

// The SqIn shape finding 27 is about. Two phrases bank 2 bars; the fill at
// 2304 activates, so SP ends at X = 2304 + 4 measures = 5376. The phrase at
// 5280 sits 250 ms before X: collecting it is the SqIn, and moves the end to
// 6912. The phrase at 6144 is then collected mid-SP and moves it to 8448.
// The tempo halves at 6000, so X (120 BPM) and D (60 BPM) differ, and the old
// "D minus two measures" rebuild (6912, 60 BPM) is wrong.
inline Song sqin_then_collect_song() {
    return build_tempo_song({{0, true},  {768, true},  {1536},       {2304, false, true},
                             {3072},     {3840},       {4608},       {5280, true},
                             {5376},     {6144, true}, {6912},       {7680},
                             {8448},     {8544}},
                            {{0, 120.0}, {6000, 60.0}});
}

// The clamp shape of finding 28, at an SP cap of 2. The meter is full when
// the activation at 2304 starts, so collecting the phrase at 3072 clamps the
// end to plusmeasure(3072, 4) = 6144 instead of 6912. The tempo halves at
// 2700, between the activation and the collecting note.
inline Song clamp_song() {
    return build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                             {3072, true}, {3840}, {4608}, {5376}, {6144}, {6912}},
                            {{0, 120.0}, {2700, 60.0}});
}

// Keep every path, not only the best score (EngineOptions' default depth is
// 0), so a SqOut or SqIn branch the fixture creates is in the output even
// when it is not optimal.
inline EngineOptions wide_search() {
    EngineOptions o;
    o.depth_mode = DepthMode::Points;
    o.depth_value = 1000000;
    return o;
}

// The first activation, over every output path, that matches `pred`. It
// walks the paths in place (walk_activations copies nothing), so the pointer
// stays valid as long as `paths` does.
template <typename Pred>
const Activation* find_act(const std::vector<Path>& paths, Pred pred) {
    for (const Path& p : paths)
        for (const Activation& a : p.walk_activations())
            if (pred(a)) return &a;
    return nullptr;
}

// ---- fixture helpers --------------------------------------------------------
//
// Every hand-built record in the tests writes its stored facts through these.

// A window that collected nothing: SP ends where the banked bars put it.
inline void set_plain_window(Activation& a, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation}};
    a.deact_tick = end_tick;
    a.clamp_tick.reset();
    a.collected_phrase_ticks.clear();
}

// A window the cap clamped at `clamp_tick`, ending at `end_tick`. A display
// test that states only those two facts gets a plain end equal to the final
// one; nothing it checks reads the plain end.
inline void set_clamped_window(Activation& a, int64_t clamp_tick, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation},
                      {clamp_tick, end_tick, SpEndKind::Clamped}};
    a.deact_tick = end_tick;
    a.clamp_tick = clamp_tick;
    a.collected_phrase_ticks = {clamp_tick};
}

// The bars of SP an activation spends, for a test that needs only the count.
inline void set_sp_meter(Activation& a, int bars) {
    a.sp_meter = bars;
}

// The fills an activation passed over before it, for a test that needs only
// the count.
inline void set_skips(Activation& a, int skips) {
    a.skips = skips;
}

// The bars banked after a path's last window, for a test that needs only the
// count.
inline void set_leftover(Path& p, int bars) {
    p.leftover_sp = bars;
}

// The stored transfer scales. `squeeze` is the scale the SqIn lines read,
// `post` the one the backend rows read. Call it after the activation's
// squeezes are pushed, so a squeeze that stores its own scale can take it.
inline void set_transfer(Activation& a, TransferScale squeeze, TransferScale post) {
    a.transfer_pre = squeeze;
    a.transfer_post = post;
}

// One stored scale for the SqIn lines and the backend rows alike.
inline void set_transfer(Activation& a, TransferScale scale) {
    set_transfer(a, scale, scale);
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_RECORD_FIXTURES_H
