"""Play the chords Hydra analyzed by reading the song clock and sending inputs.

The notes come from a `hydra_replay score` JSON, so the auto-player presses
exactly the chords, pads and times Hydra scored. Make one with
    hydra_replay score --chart <notes.chart or notes.mid> --acts "" --out <dump.json>
(or --path <a dump's JSON> --index N to walk one of its paths; the chords are
the same). A `hydra_replay dump` file does not work here: it lists paths, not
chords. Each entry of the score JSON's "chords" list gives the chord's place
in the chart (`index`), its time in the chart (`ms`, with no Offset or
song.ini delay, which is the song clock the game shows) and its "notes", one
per gem, each with the gem's color and whether it is a cymbal. Whether the 2x
kick notes are in it follows the run's own settings (--bass2x).

The auto-player presses each chord's own pads and nothing else. It has no
Star Power key: on drums the game activates from a press on a fill's last
note (docs/superpowers/plans/2026-10-28-overlay-spike.md, "How Star Power
starts on drums").

While the song plays, live.wait_until polls the game's song clock and fires
each chord PRESS_LEAD_MS early; it also notices the song stopping (clock frozen
or jumped back). live.first_note_index picks where a run that joins mid-song
starts. A jump back (a restart or seek) re-syncs to the new clock position and
starts the counts and the press log over. A run also stops at RUN_CAP_S of
wall time.

The game is opened read-only and the engine is found with
engine_finder.find_live_engine_capped; when no song starts inside its cap the
script prints the finder's message and exits 2.

With --log, the script writes one CSV row per press, with the columns
overlay_logs.PRESS_LOG_COLUMNS names. score_compare.py reads that file.

Usage:
    python tools\\ch_probe\\experiments\\play_chart.py <score.json> [--log <press.csv>]

Run this, then start the song (or start it mid-song).
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import sys
import time
from dataclasses import dataclass, field
from typing import Callable, List, Optional, Sequence, TextIO

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
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS
from tools.ch_probe.experiments.walk_edges import SongClock

# play_chart presses each chord this many ms before its time (D51: unchanged).
PRESS_LEAD_MS = 2.0

# The score is read this long after a press: a dense chart's next chord comes
# sooner than constants.INPUT_SETTLE_MS.
SCORE_READ_DELAY_S = 0.005

# A run stops after this much wall time, counted from the first clock read
# (the overlay spike plan, Task B step 1).
RUN_CAP_S = 240.0

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


@dataclass(frozen=True)
class Chord:
    """One chord to press: its `index` in the score JSON, its time and keys."""
    index: int
    ms: float
    lanes: List[int]


def load_dump_chords(path: str) -> List[Chord]:
    """The file's chords in order, each with its own `index` from the JSON."""
    with open(path, encoding="utf-8") as f:
        dump = json.load(f)
    return [Chord(chord["index"], chord["ms"],
                  [_KEY_FOR_PAD[(gem["color"], gem["cymbal"])]
                   for gem in chord["notes"]])
            for chord in dump["chords"]]


def load_dump_notes(path: str) -> list:
    """The file's chords in order, as (ms, the keys its gems press)."""
    return [(chord.ms, chord.lanes) for chord in load_dump_chords(path)]


# --- one run ------------------------------------------------------------------

class RunCapReached(Exception):
    """The run has used RUN_CAP_S of wall time."""


@dataclass
class RunResult:
    """What one run did. `rows` are the press-log rows, keyed by the names in
    PRESS_LOG_COLUMNS. `stop` says why the run ended: "done" (every chord
    pressed), "past_end" (the clock was past the last chord), "frozen",
    "cap", "interrupted" or "lost" (the process went away)."""
    sent: int = 0
    hit: int = 0
    max_streak: int = 0
    rows: list = field(default_factory=list)
    stop: str = ""


def play_chords(chords: Sequence[Chord], clock, read_score: Callable[[], int],
                press: Callable[[List[int]], object], *,
                now: Callable[[], float] = time.perf_counter,
                sleep: Callable[[float], None] = time.sleep,
                out: Optional[TextIO] = None) -> RunResult:
    """Press every chord from where the song clock is now, and log each press.

    `clock` reads the song clock (live.ReadsSongClock), `read_score` reads the
    engine's score and `press` sends one chord's keys. The live script passes
    the game's; the tests pass fakes. `now` and `sleep` are the wall clock the
    stop checks and the waits use."""
    out = sys.stdout if out is None else out
    notes_ms = [c.ms for c in chords]
    result = RunResult()
    start_s = now()

    def over_cap() -> bool:
        # live.wait_until asks this before every clock read.
        if now() - start_s >= RUN_CAP_S:
            raise RunCapReached(f"run cap of {RUN_CAP_S:g} s reached")
        return False

    raw_s, _est = clock.read()
    stopped = live.StoppedCheck(raw_s, now())
    cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
    if cursor >= len(chords):
        print(f"  Song clock at {raw_s:.2f}s is past the last note; nothing to play.",
              file=out)
        result.stop = "past_end"
        return result
    print(f"  Song clock at {raw_s:.2f}s; starting at note {cursor + 1}/{len(chords)} "
          f"(t={C.ms_to_s(notes_ms[cursor]):.2f}s)", file=out)

    score_before = read_score()
    streak = 0

    print(f"\n  {'#':>4}  {'chart_t':>7}  {'clock':>7}  {'diff_ms':>7}  {'lanes':10}  "
          f"{'result':6}  {'streak':>6}", file=out)

    try:
        while cursor < len(chords):
            chord = chords[cursor]
            try:
                raw_ms, _est_ms = live.wait_until(
                    clock, chord.ms, lead_ms=PRESS_LEAD_MS, stopped=stopped,
                    should_stop=over_cap, now=now, sleep=sleep)
            except live.ClockJumpedBack as e:
                # A restart or seek: rebuild the cursor from the new clock
                # position rather than assuming the song went back to note 0.
                print(f"\n  Song {e}. Re-syncing.", file=out)
                raw_s, _est = clock.read()
                stopped = live.StoppedCheck(raw_s, now())
                cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
                score_before = read_score()
                result = RunResult()
                streak = 0
                if cursor >= len(chords):
                    result.stop = "past_end"
                    print("\n  Stopped.", file=out)
                    return result
                continue

            press(chord.lanes)

            # Check hit through engine.pressed_input_hit.
            sleep(SCORE_READ_DELAY_S)
            score_after = read_score()
            hit = pressed_input_hit(score_before, score_after)

            result.sent += 1
            if hit:
                result.hit += 1
                streak += 1
                result.max_streak = max(result.max_streak, streak)
            else:
                streak = 0
            result.rows.append({
                "index": chord.index,
                "chart_ms": chord.ms,
                "clock_ms_at_press": raw_ms,
                "score_before": score_before,
                "score_after": score_after,
                "hit": int(hit),
            })

            diff_ms = raw_ms - chord.ms
            lane_str = "+".join(LANE_NAMES.get(l, "?") for l in chord.lanes)
            tag = "HIT" if hit else "MISS"
            if result.sent <= 30 or result.sent % 20 == 0 or not hit:
                print(f"  {result.sent:4d}  {C.ms_to_s(chord.ms):7.2f}  "
                      f"{C.ms_to_s(raw_ms):7.2f}  {diff_ms:+7.1f}  {lane_str:10}  "
                      f"{tag:6}  {streak:6}", file=out)

            score_before = score_after
            cursor += 1
        result.stop = "done"

    except live.ClockFrozen as e:
        print(f"  {e}. Song ended or paused?", file=out)
        print("\n  Stopped.", file=out)
        result.stop = "frozen"
    except RunCapReached as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop = "cap"
    except KeyboardInterrupt:
        print("\n  Stopped.", file=out)
        result.stop = "interrupted"
    except OSError as e:
        print(f"\n  Lost process: {e}", file=out)
        result.stop = "lost"
    return result


def write_press_log(path: str, rows: Sequence[dict]) -> None:
    """Write the press log: the PRESS_LOG_COLUMNS header, then one row per
    press."""
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=PRESS_LOG_COLUMNS)
        writer.writeheader()
        writer.writerows(rows)


# --- the live script ------------------------------------------------------------

def _find_engine(proc) -> int:
    """The playing song's engine object, through the capped finder.
    LIVE-ONLY: scans the game's memory."""
    return engine_finder.find_live_engine_capped(
        proc, [engine_finder.normal_pattern(proc)])


def _live_press(driver: InputDriver, hwnd: int) -> Callable[[List[int]], object]:
    """A press that focuses the game window, then sends the chord's keys.
    LIVE-ONLY: sends input to the game."""
    def press(lanes: List[int]) -> object:
        # Keep Clone Hero focused so SendInput reaches it.
        focus_window(hwnd)
        return driver.press_chord(lanes)
    return press


def main(argv: Optional[Sequence[str]] = None, *,
         open_proc: Callable = open_process,
         find_engine: Callable[[object], int] = _find_engine,
         out: Optional[TextIO] = None) -> int:
    """Play a score JSON against the running game. Returns the exit code.

    LIVE-ONLY: opens the game, finds its engine and sends keys. `open_proc`
    and `find_engine` stand in for open_process and the capped finder in
    tests."""
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(
        description="Play a hydra_replay score JSON in Clone Hero.")
    parser.add_argument("score_json", help="a hydra_replay score JSON")
    parser.add_argument("--log", help="write one CSV row per press here")
    args = parser.parse_args(argv)

    chords = load_dump_chords(args.score_json)
    if not chords:
        print(f"  {args.score_json} has no chords to play.", file=out)
        return 0
    print(f"Read {os.path.basename(args.score_json)}: {len(chords)} chords", file=out)
    print(f"  First note at {C.ms_to_s(chords[0].ms):.2f}s, "
          f"last at {C.ms_to_s(chords[-1].ms):.2f}s", file=out)

    print("\nConnecting to Clone Hero...", file=out)
    proc = open_proc(read_only=True)
    try:
        proc.verify_targets()

        print("  Waiting for active engine (start/unpause the song)...", file=out)
        try:
            engine_ptr = find_engine(proc)
        except engine_finder.EngineSearchTimeout as e:
            print(f"\n  {e}", file=out)
            return 2
        engine = EngineModel(proc)
        engine.use_object(engine_ptr)
        print(f"  Engine at {engine_ptr:#x}, song clock = {engine.song_clock():.2f}s",
              file=out)

        ch_hwnd = find_game_window()
        if ch_hwnd:
            print(f"  Clone Hero window handle: {ch_hwnd:#x}", file=out)
        else:
            print("  WARNING: could not find Clone Hero window", file=out)

        # No fill-in between frames: the estimate is the raw clock, which is
        # what play_chart has always fired on.
        clock = SongClock(engine.song_clock, max_fill_s=0.0)
        result = play_chords(chords, clock, engine.score,
                             _live_press(InputDriver(), ch_hwnd), out=out)
    finally:
        proc.close()

    if args.log:
        write_press_log(args.log, result.rows)
        print(f"\n  Press log: {args.log} ({len(result.rows)} rows)", file=out)
    print(f"\n  Notes: {result.sent}, Hits: {result.hit}, "
          f"Misses: {result.sent - result.hit}", file=out)
    print(f"  Hit rate: {result.hit / max(result.sent, 1) * 100:.1f}%", file=out)
    print(f"  Best streak: {result.max_streak}", file=out)
    print("Done.", file=out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
