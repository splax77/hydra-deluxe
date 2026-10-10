// Hand-built songs and paths shared by the Preview view-model tests: the
// scene file (test_preview_scene.cpp), the SP meter file
// (test_sp_meter_curve.cpp) and the overlay-box file (test_preview_boxes.cpp)
// all build on these, so the three files read one copy.
#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/preview_view.h"
#include "core/replay.h"
#include "analyzed_chart.h"
#include "parse/song.h"
#include "record_fixtures.h"

namespace hydra::preview_fixtures {

// A tiny 4/4, 120 BPM song built by hand: 480 ticks/quarter, so tick t is at
// t*500/480 ms and a measure is 1920 ticks. Four timestamps exercise every
// note attribute and every shaded-span source.
inline Song make_hand_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;  // tpm_changes[0] defaults to 1920 (4/4)
    song.build_timing();

    auto push = [&](int64_t tick, Chord chord) -> SongTimestamp& {
        SongTimestamp ts;
        ts.timecode = song.timecode(tick);
        ts.chord = std::move(chord);
        song.sequence.push_back(std::move(ts));
        return song.sequence.back();
    };

    // tick 0: a plain Red.
    {
        Chord c;
        c.add_note(NoteColor::Red);
        push(0, c);
    }
    // tick 240: a Yellow cymbal, ghost; start of a solo.
    {
        Chord c;
        c.add_note(NoteColor::Yellow);
        c.apply_cymbal(NoteColor::Yellow);
        c.apply_ghost(NoteColor::Yellow);
        push(240, c).flag_solo = true;
    }
    // tick 480: a Blue accent; still in the solo.
    {
        Chord c;
        c.add_note(NoteColor::Blue);
        c.apply_accent(NoteColor::Blue);
        push(480, c).flag_solo = true;
    }
    // tick 720: a 2x-kick + Green chord; ends an SP phrase that began at tick
    // 240, and sits at the end of an activation fill 480 ticks long.
    {
        Chord c;
        c.add_2x();  // adds the Kick note itself, with is2x set
        c.add_note(NoteColor::Green);
        SongTimestamp& ts = push(720, c);
        test::mark_phrase_end(ts, 720, 480);
        ts.activation_length = 480;
    }
    // The one solo section: the chords at ticks 240 and 480 (sequence
    // positions 1 and 2). The parsers build this list from the flags
    // (find_solo_sections in song.cpp, private there); a hand-built song
    // states it.
    song.solo_sections = {{1, 2}};
    return song;
}

// A song with four candidate activation fills, each 240 ticks long, ending on
// the notes at ticks 480, 960, 1440 and 1920.
inline Song make_fill_song() {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    song.build_timing();
    for (int i = 1; i <= 4; ++i) {
        Chord c;
        c.add_note(NoteColor::Green);
        SongTimestamp ts;
        ts.timecode = song.timecode(480 * i);
        ts.chord = std::move(c);
        ts.activation_length = 240;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// An activation at `tick` that passed over the fills ending at `skipped_fills`.
inline Activation act_at(const Song& song, int64_t tick, std::vector<int64_t> skipped_fills) {
    Activation a;
    a.timecode = song.timecode(tick);
    a.skipped_fill_ticks = std::move(skipped_fills);
    return a;
}

// ---- SP meter fixtures --------------------------------------------------
//
// Same 4/4, 120 BPM grid as make_hand_song: a note every `step` ticks out to
// `last_tick`, with an SP phrase ending on each tick in `phrase_ends`. One
// measure is 1920 ticks = 2000 ms, so one SP bar (two measures) burns 4000 ms
// at this tempo. `extra_bpm` adds tempo changes before the timing is built.
// A phrase can only end on a note, so `step` is there for a fixture that needs
// a phrase off the quarter-note grid. `extra_sigs` adds time signatures (tick ->
// numerator, denominator) the same way.
inline Song make_sp_song(const std::vector<int64_t>& phrase_ends, int64_t last_tick,
                         const std::map<int64_t, double>& extra_bpm = {},
                         int64_t step = 480,
                         const std::map<int64_t, std::pair<int, int>>& extra_sigs = {}) {
    Song song(480);
    song.bpm_changes[0] = 120.0;
    for (const auto& kv : extra_bpm) song.bpm_changes[kv.first] = kv.second;
    for (const auto& [tick, sig] : extra_sigs) apply_timesig(song, tick, sig.first, sig.second);
    song.build_timing();

    for (int64_t t = 0; t <= last_tick; t += step) {
        Chord c;
        c.add_note(NoteColor::Red);
        SongTimestamp ts;
        ts.timecode = song.timecode(t);
        ts.chord = std::move(c);
        if (std::find(phrase_ends.begin(), phrase_ends.end(), t) != phrase_ends.end())
            test::mark_phrase_end(ts, t, step);
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// An activation the engine could have recorded: a timecode, the bars it
// spends, and the SP end the search stamped on it. Nothing derives that end
// any more, so the fixture states it as a literal tick. The window is plain:
// an activation that collects no phrase mid-SP. A fixture that collects one
// states its whole SP-end history itself.
inline Activation sp_act_at(const Song& song, int64_t tick, int sp_meter, int64_t end_tick) {
    Activation a;
    a.timecode = song.timecode(tick);
    test::set_sp_meter(a, sp_meter);
    test::set_skips(a, 0);
    test::set_plain_window(a, end_tick);
    return a;
}

// The engine fixture from test_search.cpp (the "a second clamp in the same
// window replaces clamp_tick" row of "SP cap overfill: where the end lands and
// which note clamped it"), rebuilt with each phrase's start
// tick set so the Preview draws the phrases. 192 ticks per beat, 4/4, 120
// BPM: a measure is 768 ticks and 2000 ms.
inline Song make_overfill_song() {
    return test::build_fixture_song(192, 120.0,
                                    {{0, true},     {768, true},  {2304, false, 384}, {3072, true},
                                     {3840, true},  {4608},       {5376},             {6000},
                                     {6768},        {7500}});
}

// A path whose stored score is what the replay prices it at, so the scene
// trusts it. The corpus case proves the replay against the engine; these
// hand-built fixtures only test the Preview's plumbing.
inline Path priced_path(const Song& song, std::vector<Activation> acts) {
    Path p;
    p.activations = std::move(acts);
    assign_score(p, replay_stored_path(song, p).result.final);
    return p;
}

// Two one-bar activations on make_sp_song's timing (120 BPM, 4/4, 480 ticks a
// beat): tick 1920 is 2000 ms, measure 2; tick 7680 is 8000 ms, measure 5.
// One phrase ends at tick 960 (1000 ms) to fill the first bar. The notes run
// to tick 13440 so the second activation's deact node (tick 11520) is inside.
struct TwoActs {
    Song song = make_sp_song({960}, /*last_tick=*/13440);
    Path path;
    app::PreviewScene scene;
    TwoActs() {
        Activation a = sp_act_at(song, 1920, /*sp_meter=*/1, /*end_tick=*/5760);
        a.bank_rise_ticks = {960};  // the phrase's bar, as the engine stamps it
        a.chord.add_note(NoteColor::Red);
        Activation b = sp_act_at(song, 7680, /*sp_meter=*/1, /*end_tick=*/11520);
        b.chord.add_note(NoteColor::Red);
        path.activations = {a, b};
        scene = app::build_preview_scene(song, &path);
    }
};

}  // namespace hydra::preview_fixtures
