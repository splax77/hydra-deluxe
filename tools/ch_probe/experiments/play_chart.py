"""Play the chords Hydra analyzed by reading the song clock and sending inputs.

The notes come from a `hydra_replay score` JSON, so the auto-player presses
exactly the chords, pads and times Hydra scored. Make one with
    hydra_replay score --chart <notes.chart or notes.mid> --acts "" --out <dump.json>
(or --path <a dump's JSON> --index N to walk one of its paths; the chords are
the same). A `hydra_replay dump` file does not work here: it lists paths, not
chords. Each entry of the score JSON's "chords" list gives the chord's place
in the chart (`index`), its time in the chart (`ms`, with no Offset or
song.ini delay, which is the song clock the game shows) and its "notes", one
per gem, each with the gem's color, whether it is a cymbal and its `dynamic`.
Whether the 2x kick notes are in it follows the run's own settings (--bass2x).

The auto-player presses each chord's own pads and nothing else. It has no
Star Power key: on drums the game activates from a press on a fill's last
note (docs/clone-hero/05-star-power.md, SP-17).

So a fill the path passes over is declined: declined_chord_indexes reads which
chords those are from a `hydra_replay dump` file, and load_dump_chords keeps
only the gems the decline mode sends (design point 4 of
docs/superpowers/plans/2026-11-06-optimal-score-run.md (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md);
the game's side is FL-31 on docs/clone-hero/04-fills.md).

The kit press gives each pad gem the strength its `dynamic` asks for; a key
has no strength. kit_tools.pad_axis owns the strengths, and with_axes reads the
game's dynamics threshold for them when the chart holds a ghost gem.

While the song plays, live.wait_until polls the game's song clock and fires
each chord early, by SP_END_LEAD_MS on a chord that ends Star Power and by
PRESS_LEAD_MS on every other; it also notices the song stopping (clock frozen
or jumped back). live.first_note_index picks where a run that joins mid-song
starts. A jump back (a restart or seek) re-syncs to the new clock position and
starts the counts and the press log over. A run also stops at its cap of
wall time: RUN_CAP_S, unless the caller passes a longer one.

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
from dataclasses import dataclass, field, replace
from typing import Callable, List, Mapping, Optional, Sequence, TextIO, Tuple

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

# A chord that ends Star Power is pressed this many ms before its time, so the
# hit lands in a frame before the end check. The user set it: question 3 of
# docs/superpowers/plans/2026-11-06-optimal-score-run.md (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md).
SP_END_LEAD_MS = 15.0

# Which gems a declined chord sends: _kept_gems owns what each mode keeps.
# Question 4 of the same plan says which is tried first.
DECLINE_KICK_ONLY = "kick_only"
DECLINE_NOTHING = "nothing"
DECLINE_MODES = (DECLINE_KICK_ONLY, DECLINE_NOTHING)

# What a declined chord's press row and printed line say in place of a result.
DECLINED = "declined"

# The score is read this long after a press: a dense chart's next chord comes
# sooner than constants.INPUT_SETTLE_MS.
SCORE_READ_DELAY_S = 0.005

# A run stops after this much wall time, counted from the first clock read
# (the overlay spike plan, Task B step 1), when its caller gives no cap.
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
    """One chord to press: its `index` in the score JSON, its time, its keys
    and each key's `dynamic`, in the same order. `axes` holds each key's
    strength once with_axes has worked them out. `ends_sp` says the chord's
    tick is an activation's `deact_tick`. `declined` says the path passes this
    fill over, so `lanes` holds only what the decline mode sends."""
    index: int
    ms: float
    lanes: List[int]
    dynamics: List[str]
    axes: Optional[List[float]] = None
    ends_sp: bool = False
    declined: bool = False


def early_side_ms(gap_ms: float) -> float:
    """How far from a note the game's window reaches on the side of a
    neighbour `gap_ms` away, in normal mode (TM-18 and TM-19 on
    docs/clone-hero/03-timing.md)."""
    return min(max(gap_ms / 2, C.s_to_ms(C.EXPECT_NORMAL_FRONT_S)),
               C.EXPECT_NORMAL_BACK_MS)


def lead_limit_ms(gap_ms: float) -> float:
    """A lead must stay below this for a chord `gap_ms` after the chord before
    it. The press must fall inside the chord's own early side, and past the
    earlier chord's late edge, which is the same half-gap."""
    side = early_side_ms(gap_ms)
    return min(side, gap_ms - side)


def _refuse_long_leads(chords: Sequence[Chord]) -> None:
    """Raise ValueError naming every Star Power end chord SP_END_LEAD_MS is too
    long for. Only those chords are checked: PRESS_LEAD_MS has to work on any
    chart, and a real chart holds chords closer than a window's clamp. The
    first chord has no chord before it for a press to reach."""
    refused = [chord.index for before, chord in zip(chords, chords[1:])
               if chord.ends_sp and SP_END_LEAD_MS >= lead_limit_ms(chord.ms - before.ms)]
    if refused:
        raise ValueError(f"SP_END_LEAD_MS of {SP_END_LEAD_MS:g} ms is too long for chords "
                         f"{', '.join(map(str, refused))}: an early press would miss "
                         f"its note or reach the chord before it")


def declined_chord_indexes(score_path: str, dump_path: str,
                           path_index: int = 0) -> List[int]:
    """The `index` of each chord in the score file whose tick is a fill the
    dump's path `path_index` passed over. The dump's `skipped_fill_ticks` are
    the only record of those: the score file holds no meter. Raises ValueError
    for a tick the score file has no chord on, which means the two files are
    not of one chart."""
    with open(dump_path, encoding="utf-8") as f:
        path = json.load(f)["paths"][path_index]
    with open(score_path, encoding="utf-8") as f:
        score = json.load(f)
    chords = score["chords"]
    # A score file made from a path file names the path it read; one made with
    # --acts has no path_source and cannot be checked.
    source_index = score.get("path_source", {}).get("index")
    if source_index is not None and source_index != path_index:
        raise ValueError(f"{score_path} was made from path {source_index}, "
                         f"but path_index is {path_index}")
    skipped = {tick for act in path["activations"] for tick in act["skipped_fill_ticks"]}
    missing = skipped - {chord["tick"] for chord in chords}
    if missing:
        raise ValueError(f"no chord in {score_path} at skipped fill ticks "
                         f"{', '.join(map(str, sorted(missing)))}")
    return [chord["index"] for chord in chords if chord["tick"] in skipped]


def _kept_gems(mode: str, lanes: List[int],
               dynamics: List[str]) -> Tuple[List[int], List[str]]:
    """The lanes and dynamics a declined chord sends in decline mode `mode`:
    at most one gem, the design point's limit. Raises ValueError for a mode
    not in DECLINE_MODES."""
    if mode not in DECLINE_MODES:
        raise ValueError(f'unknown decline mode "{mode}"; modes: {", ".join(DECLINE_MODES)}')
    kept = [i for i, lane in enumerate(lanes) if lane == Lane.KICK][:1]
    if mode == DECLINE_NOTHING:
        kept = []
    return [lanes[i] for i in kept], [dynamics[i] for i in kept]


def load_dump_chords(path: str, declines: Optional[Mapping[int, str]] = None) -> List[Chord]:
    """The file's chords in order, each with its own `index` from the JSON.
    A chord on an activation's end is marked, and a lead too long for such a
    chord raises _refuse_long_leads's ValueError. `declines` maps a chord's
    `index` to its decline mode. A declined chord stays in the list with its
    time, because its note is still in the chart for a neighbour's lead."""
    declines = {} if declines is None else declines
    with open(path, encoding="utf-8") as f:
        dump = json.load(f)
    deact_ticks = {act["deact_tick"] for act in dump.get("activations", [])}
    chords = []
    for chord in dump["chords"]:
        lanes = [_KEY_FOR_PAD[(gem["color"], gem["cymbal"])] for gem in chord["notes"]]
        dynamics = [gem["dynamic"] for gem in chord["notes"]]
        declined = chord["index"] in declines
        if declined:
            lanes, dynamics = _kept_gems(declines[chord["index"]], lanes, dynamics)
        chords.append(Chord(chord["index"], chord["ms"], lanes, dynamics,
                            ends_sp=chord.get("tick") in deact_ticks, declined=declined))
    _refuse_long_leads(chords)
    return chords


def load_dump_notes(path: str) -> list:
    """The file's chords in order, as (ms, the keys its gems press)."""
    return [(chord.ms, chord.lanes) for chord in load_dump_chords(path)]


def with_axes(chords: Sequence[Chord], engine) -> Tuple[List[Chord], Optional[int]]:
    """`chords` with each gem's strength from kit_tools.pad_axis, and the
    dynamics threshold read for them. The field is read once, and only when a
    gem is a ghost; otherwise the threshold is None. Every strength is worked
    out here, so a threshold no ghost can use raises pad_axis's ValueError
    before the first press."""
    # Imported here because kit_tools imports game_tools, which imports this
    # module.
    from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis
    threshold = None
    if any(DYNAMIC_GHOST in chord.dynamics for chord in chords):
        threshold = engine.dyn_threshold()
    return [replace(chord, axes=[pad_axis(dynamic, threshold) for dynamic in chord.dynamics])
            for chord in chords], threshold


# --- one run ------------------------------------------------------------------

class RunCapReached(Exception):
    """The run has used its cap of wall time."""


class StopAsked(Exception):
    """The caller's `should_stop` said to end the run."""


# The `stop` of a live run that never found a playing song's engine.
STOP_NO_ENGINE = "no engine"


@dataclass
class RunResult:
    """What one run did. `rows` are the press-log rows, keyed by the names in
    PRESS_LOG_COLUMNS, plus `lead_ms`, the lead the press used, which no log
    column holds. A declined chord's row says DECLINED in `hit`. `sent`, `hit`
    and `max_streak` leave declined chords out, so a declined chord is no
    miss; `declined` counts them. `stop` says why the run ended: "done" (every chord
    pressed), "past_end" (the clock was past the last chord), "frozen",
    "cap", "interrupted", "lost" (the process went away), "stopped" (the
    caller's `should_stop` said so) or STOP_NO_ENGINE (live_play found no
    playing song; `message` holds the finder's words). `dyn_threshold` is the
    dynamics threshold live_play read, None when it read none."""
    sent: int = 0
    hit: int = 0
    declined: int = 0
    max_streak: int = 0
    rows: list = field(default_factory=list)
    stop: str = ""
    message: str = ""
    dyn_threshold: Optional[int] = None


def play_chords(chords: Sequence[Chord], clock, read_score: Callable[[], int],
                press: Callable[[List[int], Optional[List[float]]], object], *,
                now: Callable[[], float] = time.perf_counter,
                sleep: Callable[[float], None] = time.sleep,
                wall: Callable[[], float] = time.time,
                on_press: Optional[Callable[[dict], object]] = None,
                should_stop: Optional[Callable[[], bool]] = None,
                out: Optional[TextIO] = None,
                cap_s: float = RUN_CAP_S) -> RunResult:
    """Press every chord from where the song clock is now, and log each press.

    `clock` reads the song clock (live.ReadsSongClock), `read_score` reads the
    engine's score and `press` sends one chord's keys and their `axes`. When
    `press` returns a dict (the kit press of tools/ch_mcp/kit_tools.py), the
    row also keeps its values that PRESS_LOG_COLUMNS names. The live script passes
    the game's; the tests pass fakes. `now` and `sleep` are the timer the stop
    checks and the waits use. `wall` stamps each row's wall columns: it is the
    watch's clock, because `now` shares no zero with the watch or a trace.
    `on_press`, when given, gets each press-log row as it is added;
    `should_stop`, when given, is asked with the run cap and ends the run as
    "stopped" when it returns True. `cap_s` is the run cap in wall seconds."""
    out = sys.stdout if out is None else out
    notes_ms = [c.ms for c in chords]
    result = RunResult()
    start_s = now()

    def over_cap() -> bool:
        # live.wait_until asks this before every clock read.
        if now() - start_s >= cap_s:
            raise RunCapReached(f"run cap of {cap_s:g} s reached")
        if should_stop is not None and should_stop():
            raise StopAsked("asked to stop")
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
            lead_ms = SP_END_LEAD_MS if chord.ends_sp else PRESS_LEAD_MS
            try:
                raw_ms, _est_ms = live.wait_until(
                    clock, chord.ms, lead_ms=lead_ms, stopped=stopped,
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
            wall_s_at_read = wall()

            # A declined chord in a mode that keeps no gem sends no press.
            pressed = None if chord.declined and not chord.lanes else press(chord.lanes, chord.axes)
            wall_s_at_reply = wall()

            # Check hit through engine.pressed_input_hit.
            sleep(SCORE_READ_DELAY_S)
            score_after = read_score()
            hit = not chord.declined and pressed_input_hit(score_before, score_after)

            if chord.declined:
                result.declined += 1
            else:
                result.sent += 1
                if hit:
                    result.hit += 1
                    streak += 1
                    result.max_streak = max(result.max_streak, streak)
                else:
                    streak = 0
            row = {
                "index": chord.index,
                "chart_ms": chord.ms,
                "clock_ms_at_press": raw_ms,
                "lead_ms": lead_ms,
                "score_before": score_before,
                "score_after": score_after,
                "hit": DECLINED if chord.declined else int(hit),
                "wall_s_at_read": wall_s_at_read,
                "wall_s_at_reply": wall_s_at_reply,
            }
            if isinstance(pressed, dict):
                # A kit press replies with what the game's frames did; the
                # row keeps the parts the press log has columns for.
                row.update((k, pressed[k]) for k in PRESS_LOG_COLUMNS
                           if k in pressed and k not in row)
            result.rows.append(row)
            if on_press is not None:
                on_press(row)

            diff_ms = raw_ms - chord.ms
            lane_str = "+".join(LANE_NAMES.get(l, "?") for l in chord.lanes)
            tag = DECLINED if chord.declined else "HIT" if hit else "MISS"
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
    except StopAsked as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop = "stopped"
    except KeyboardInterrupt:
        print("\n  Stopped.", file=out)
        result.stop = "interrupted"
    except OSError as e:
        print(f"\n  Lost process: {e}", file=out)
        result.stop = "lost"
    return result


def write_press_log(path: str, rows: Sequence[dict]) -> None:
    """Write the press log: the PRESS_LOG_COLUMNS header, then one row per
    press. A row's other values stay out of the file."""
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = _press_log_writer(f)
        writer.writeheader()
        writer.writerows(rows)


def append_press_row(path: str, row: dict) -> None:
    """Add one row to the press log, with the header first when the file is
    new or empty. The file is closed again, so a worker that dies keeps every
    row so far."""
    with open(path, "a", encoding="utf-8", newline="") as f:
        writer = _press_log_writer(f)
        if f.tell() == 0:
            writer.writeheader()
        writer.writerow(row)


def _press_log_writer(f: TextIO) -> csv.DictWriter:
    # lead_ms stays off the log: a row's clock_ms_at_press already shows it.
    return csv.DictWriter(f, fieldnames=PRESS_LOG_COLUMNS, extrasaction="ignore")


# --- the live script ------------------------------------------------------------

def find_normal_engine(proc, out: Optional[TextIO] = None) -> int:
    """The playing song's engine object, through the capped finder, over the
    normal-mode pattern only: the auto-player does not play a precision-mode
    engine. The finder's progress dots go to `out` (stdout when None).
    LIVE-ONLY: scans the game's memory."""
    return engine_finder.find_live_engine_capped(
        proc, [engine_finder.normal_pattern(proc)], out=out)


def live_press(driver: InputDriver, hwnd: int) -> Callable[..., object]:
    """A press that focuses the game window, then sends the chord's keys. A
    key has no strength, so `axes` goes unused. This key route needs the game
    window in focus and the user's typed `allow key run`; the kit press does
    not.
    LIVE-ONLY: sends input to the game."""
    def press(lanes: List[int], axes: Optional[List[float]] = None) -> object:
        # Keep Clone Hero focused so SendInput reaches it.
        focus_window(hwnd)
        return driver.press_chord(lanes)
    return press


def live_play(chords: Sequence[Chord], *,
              open_proc: Callable = open_process,
              find_engine: Optional[Callable[[object], int]] = None,
              driver: Optional[InputDriver] = None,
              press: Optional[Callable[..., object]] = None,
              on_press: Optional[Callable[[dict], object]] = None,
              should_stop: Optional[Callable[[], bool]] = None,
              out: Optional[TextIO] = None,
              cap_s: float = RUN_CAP_S) -> RunResult:
    """Play `chords` against the running game and return what the run did.

    LIVE-ONLY: opens the game read-only, finds its engine and presses each
    chord through `press` when one is given (the kit press, which needs no
    window, so none is looked up), else sends keys through `driver` (a new
    InputDriver when None) to the game window. When no song starts
    inside the finder's cap, the result's `stop` is STOP_NO_ENGINE and its
    `message` is the finder's. with_axes works out each gem's strength once
    the engine is found, on the press route only, and raises its ValueError
    before any press when no ghost strength is possible. `on_press`, `should_stop` and `cap_s` go to
    play_chords. `open_proc` and `find_engine` stand in for open_process and
    the capped finder in tests. With no `find_engine` the real search runs
    and its progress dots go to `out`, so a caller that gives `out` a buffer
    (the `ch` MCP server, whose stdout is its protocol channel) gets no
    stdout from the search."""
    out = sys.stdout if out is None else out
    if find_engine is None:
        def find_engine(proc) -> int:
            return find_normal_engine(proc, out=out)
    print("\nConnecting to Clone Hero...", file=out)
    proc = open_proc(read_only=True)
    try:
        proc.verify_targets()

        print("  Waiting for active engine (start/unpause the song)...", file=out)
        try:
            engine_ptr = find_engine(proc)
        except engine_finder.EngineSearchTimeout as e:
            return RunResult(stop=STOP_NO_ENGINE, message=str(e))
        engine = EngineModel(proc)
        engine.use_object(engine_ptr)
        print(f"  Engine at {engine_ptr:#x}, song clock = {engine.song_clock():.2f}s",
              file=out)
        # A key has no strength, so the keys route reads no threshold.
        threshold = None
        if press is not None:
            chords, threshold = with_axes(chords, engine)
        if threshold is not None:
            print(f"  Dynamics threshold: {threshold}", file=out)

        if press is None:
            ch_hwnd = find_game_window()
            if ch_hwnd:
                print(f"  Clone Hero window handle: {ch_hwnd:#x}", file=out)
            else:
                print("  WARNING: could not find Clone Hero window", file=out)
            driver = InputDriver() if driver is None else driver
            press = live_press(driver, ch_hwnd)

        # No fill-in between frames: the estimate is the raw clock, and the
        # presses fire on that.
        clock = SongClock(engine.song_clock, max_fill_s=0.0)
        result = play_chords(chords, clock, engine.score, press,
                             on_press=on_press, should_stop=should_stop, out=out,
                             cap_s=cap_s)
        result.dyn_threshold = threshold
        return result
    finally:
        proc.close()


def main(argv: Optional[Sequence[str]] = None, *,
         open_proc: Callable = open_process,
         find_engine: Optional[Callable[[object], int]] = None,
         out: Optional[TextIO] = None) -> int:
    """Play a score JSON against the running game. Returns the exit code.

    LIVE-ONLY: the run itself is live_play's. `open_proc` and `find_engine`
    stand in for open_process and the capped finder in tests."""
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

    result = live_play(chords, open_proc=open_proc, find_engine=find_engine,
                       out=out)
    if result.stop == STOP_NO_ENGINE:
        print(f"\n  {result.message}", file=out)
        return 2

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
