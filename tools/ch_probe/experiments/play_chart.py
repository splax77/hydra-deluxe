"""Play the chords Hydra analyzed by reading the song clock and sending inputs.

The notes come from a `hydra_replay score` JSON, so the auto-player presses
exactly the chords, pads and times Hydra scored. Make one with
    hydra_replay score --chart <notes.chart or notes.mid> --acts "" --out <dump.json>
(or --path <a dump's JSON> to walk one of its paths; the chords are the same).
Each entry of its "chords" list gives the chord's time in the chart (`ms`,
with no Offset or song.ini delay, which is the song clock the game shows) and
its "notes", one per gem, each with the gem's color and whether it is a
cymbal. Whether the 2x kick notes are in it follows the run's own settings
(--bass2x).

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

from tools.ch_probe import constants as C, engine_finder
from tools.ch_probe.engine import EngineModel, pressed_input_hit
from tools.ch_probe.process import open_process
from tools.ch_probe.input_driver import (
    LANE_NAMES, InputDriver, Lane, find_game_window, focus_window)
from tools.ch_probe.experiments import live
from tools.ch_probe.experiments.walk_edges import SongClock

# play_chart presses each chord this many ms before its time (D51: unchanged).
PRESS_LEAD_MS = 2.0

# The key for each gem in a chord's "notes" list, by the gem's color (Kick,
# Red, Yellow, Blue, Green) and whether it is a cymbal, as hydra_replay writes
# them. A 2x kick is the kick pad, and a ghost or accent hits the same pad as
# a plain gem.
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
    """The file's chords in order, as (ms, the keys its gems press)."""
    with open(path, encoding="utf-8") as f:
        dump = json.load(f)
    return [(chord["ms"],
             [_KEY_FOR_PAD[(gem["color"], gem["cymbal"])] for gem in chord["notes"]])
            for chord in dump["chords"]]


def main() -> None:
    if len(sys.argv) != 2:
        print("Usage: play_chart.py <hydra_replay score JSON>")
        return
    dump_path = sys.argv[1]
    notes = load_dump_notes(dump_path)
    if not notes:
        print(f"  {dump_path} has no chords to play.")
        return
    notes_ms = [ms for ms, _lanes in notes]
    print(f"Read {os.path.basename(dump_path)}: {len(notes)} chords")
    print(f"  First note at {C.ms_to_s(notes_ms[0]):.2f}s, last at {C.ms_to_s(notes_ms[-1]):.2f}s")

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
    ch_hwnd = find_game_window()
    if ch_hwnd:
        print(f"  Clone Hero window handle: {ch_hwnd:#x}")
    else:
        print("  WARNING: could not find Clone Hero window")

    def ensure_focus():
        focus_window(ch_hwnd)

    def read_score():
        return engine.score()

    # No fill-in between frames: the estimate is the raw clock, which is what
    # play_chart has always fired on.
    clock = SongClock(engine.song_clock, max_fill_s=0.0)

    # Start from wherever the song already is -- no restart required.
    raw_s, _est = clock.read()
    stopped = live.StoppedCheck(raw_s, time.perf_counter())
    cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
    if cursor >= len(notes):
        print(f"  Song clock at {raw_s:.2f}s is past the last note; nothing to play.")
        proc.close()
        return
    print(f"  Song clock at {raw_s:.2f}s; starting at note {cursor + 1}/{len(notes)} "
          f"(t={C.ms_to_s(notes_ms[cursor]):.2f}s)")

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
                cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
                score_before = read_score()
                total_sent = 0; total_hit = 0; streak = 0; max_streak = 0
                if cursor >= len(notes):
                    raise KeyboardInterrupt
                continue

            # Ensure CH has focus before every input
            ensure_focus()

            # Send input for this note's lanes — press all down, hold, release
            driver.press_chord(lanes)

            # Check hit through engine.pressed_input_hit. Read 5 ms after the
            # press, not after constants.INPUT_SETTLE_MS: a dense chart's next
            # chord comes sooner than that settle.
            time.sleep(0.005)
            score_after = read_score()
            hit = pressed_input_hit(score_before, score_after)

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
                print(f"  {total_sent:4d}  {C.ms_to_s(note_ms):7.2f}  {C.ms_to_s(raw_ms):7.2f}  "
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
