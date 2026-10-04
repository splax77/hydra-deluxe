"""Play the chords Hydra analyzed by reading the song clock and sending inputs.

The notes come from a `hydra_replay dump` JSON, so the auto-player presses
exactly the chords, lanes and times Hydra scored. Make one with
    hydra_replay dump --chart <notes.chart or notes.mid> --db <hydra.db> --out <dump.json>
Each entry of the dump's "chords" list gives the chord's time in the chart
(`ms`, with no Offset or song.ini delay, which is the song clock the game
shows) and the pads it hits (`lanes`, each note's color and whether it is a
cymbal, written by hydra_replay from the C++ chord). Whether the 2x kick
notes are in it follows the dump's own settings.

While the song plays, live.wait_until polls the game's song clock and fires
each chord PRESS_LEAD_MS early; it also notices the song stopping (clock frozen
or jumped back). live.first_note_index picks where a run that joins mid-song
starts. A jump back (a restart or seek) re-syncs to the new clock position.

Usage:
    python tools\\ch_probe\\experiments\\play_chart.py <dump.json>

Run this, then start the song (or start it mid-song).
"""

from __future__ import annotations

import json
import os
import sys
import time

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import engine_finder
from tools.ch_probe.engine import EngineModel
from tools.ch_probe.process import open_process
from tools.ch_probe.input_driver import LANE_NAMES, InputDriver, Lane
from tools.ch_probe.experiments import live
from tools.ch_probe.experiments.walk_edges import SongClock

# play_chart presses each chord this many ms before its time (D51: unchanged).
PRESS_LEAD_MS = 2.0

# The key for each pad a dump chord names in its "lanes" list: hydra_replay
# writes each note's color (Kick, Red, Yellow, Blue, Green) and whether it is
# a cymbal, from the C++ chord. A 2x kick is the kick pad.
_KEY_FOR_PAD = {
    ("Kick", False): Lane.KICK,
    ("Red", False): Lane.RED,
    ("Yellow", False): Lane.YELLOW,
    ("Yellow", True): Lane.YELLOW_CYMBAL,
    ("Blue", False): Lane.BLUE,
    ("Blue", True): Lane.BLUE_CYMBAL,
    ("Green", False): Lane.GREEN,
    ("Green", True): Lane.GREEN_CYMBAL,
}


def load_dump_notes(path: str) -> list:
    """The dump's chords in order, as (ms, lanes); chords with no keys are left out."""
    with open(path, encoding="utf-8") as f:
        dump = json.load(f)
    notes = []
    for chord in dump["chords"]:
        lanes = [_KEY_FOR_PAD[(pad["color"], pad["cymbal"])] for pad in chord["lanes"]]
        if lanes:
            notes.append((chord["ms"], lanes))
    return notes


def main() -> None:
    if len(sys.argv) != 2:
        print("Usage: play_chart.py <hydra_replay dump JSON>")
        return
    dump_path = sys.argv[1]
    notes = load_dump_notes(dump_path)
    if not notes:
        print(f"  {dump_path} has no chords to play.")
        return
    notes_ms = [ms for ms, _lanes in notes]
    print(f"Read {os.path.basename(dump_path)}: {len(notes)} chords")
    print(f"  First note at {notes_ms[0] / 1000:.2f}s, last at {notes_ms[-1] / 1000:.2f}s")

    print("\nConnecting to Clone Hero...")
    proc = open_process()
    proc.verify_targets()

    print("  Waiting for active engine (start/unpause the song)...")
    engine = EngineModel(proc)
    engine.use_object(engine_finder.find_live_engine(
        proc, [engine_finder.normal_pattern(proc)]))
    engine_ptr = engine.object_ptr
    clock_now = engine.song_clock()
    print(f"  Engine at {engine_ptr:#x}, song clock = {clock_now:.2f}s")

    driver = InputDriver()

    # Keep Clone Hero focused so SendInput reaches it
    import ctypes
    user32 = ctypes.windll.user32
    ch_hwnd = user32.FindWindowW(None, "Clone Hero")
    if ch_hwnd:
        print(f"  Clone Hero window handle: {ch_hwnd:#x}")
    else:
        print("  WARNING: could not find Clone Hero window")

    def ensure_focus():
        if ch_hwnd:
            user32.SetForegroundWindow(ch_hwnd)

    def read_score():
        return engine.score()

    # No fill-in between frames: the estimate is the raw clock, which is what
    # play_chart has always fired on.
    clock = SongClock(engine.song_clock, max_fill_s=0.0)

    # Start from wherever the song already is -- no restart required.
    raw_s, _est = clock.read()
    stopped = live.StoppedCheck(raw_s, time.perf_counter())
    cursor = live.first_note_index(notes_ms, raw_s * 1000)
    if cursor >= len(notes):
        print(f"  Song clock at {raw_s:.2f}s is past the last note; nothing to play.")
        proc.close()
        return
    print(f"  Song clock at {raw_s:.2f}s; starting at note {cursor + 1}/{len(notes)} "
          f"(t={notes_ms[cursor] / 1000:.2f}s)")

    score_before = read_score()
    total_sent = 0
    total_hit = 0
    streak = 0
    max_streak = 0

    print(f"\n  {'#':>4}  {'chart_t':>7}  {'clock':>7}  {'diff_ms':>7}  {'lanes':10}  {'result':6}  {'streak':>6}")

    try:
        while cursor < len(notes):
            note_ms, lanes = notes[cursor]
            try:
                raw_ms, _est_ms = live.wait_until(
                    clock, note_ms, lead_ms=PRESS_LEAD_MS, stopped=stopped)
            except live.ClockJumpedBack as e:
                # A restart or seek: rebuild the cursor from the new clock
                # position rather than assuming the song went back to note 0.
                print(f"\n  Song {e}. Re-syncing.")
                raw_s, _est = clock.read()
                stopped = live.StoppedCheck(raw_s, time.perf_counter())
                cursor = live.first_note_index(notes_ms, raw_s * 1000)
                score_before = read_score()
                total_sent = 0; total_hit = 0; streak = 0; max_streak = 0
                if cursor >= len(notes):
                    raise KeyboardInterrupt
                continue

            # Ensure CH has focus before every input
            ensure_focus()

            # Send input for this note's lanes — press all down, hold, release
            driver.press_chord(lanes)

            # Check hit: the game score only rises when a note registers.
            time.sleep(0.005)
            score_after = read_score()
            hit = score_after > score_before

            total_sent += 1
            if hit:
                total_hit += 1
                streak += 1
                if streak > max_streak:
                    max_streak = streak
            else:
                streak = 0

            diff_ms = raw_ms - note_ms

            lane_str = "+".join(LANE_NAMES.get(l, "?") for l in lanes)
            tag = "HIT" if hit else "MISS"

            if total_sent <= 30 or total_sent % 20 == 0 or not hit:
                print(f"  {total_sent:4d}  {note_ms / 1000:7.2f}  {raw_ms / 1000:7.2f}  "
                      f"{diff_ms:+7.1f}  {lane_str:10}  {tag:6}  {streak:6}")

            score_before = score_after
            cursor += 1

    except live.ClockFrozen as e:
        print(f"  {e}. Song ended or paused?")
        print("\n  Stopped.")
    except KeyboardInterrupt:
        print("\n  Stopped.")
    except OSError as e:
        print(f"\n  Lost process: {e}")

    print(f"\n  Notes: {total_sent}, Hits: {total_hit}, Misses: {total_sent - total_hit}")
    print(f"  Hit rate: {total_hit/max(total_sent,1)*100:.1f}%")
    print(f"  Best streak: {max_streak}")

    proc.close()
    print("Done.")


if __name__ == "__main__":
    main()
