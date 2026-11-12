"""Compare the game's score in a press log with Hydra's points for each chord.

Inputs: the press log play_chart.py writes with --log (its columns are
overlay_logs.PRESS_LOG_COLUMNS; load_press_log also reads the older logs
overlay_logs names), the `hydra_replay score --path` JSON the run played, the
route the run pressed through and the run's stop reason. Pure logic: no
process, no game.

The score check (D319 item 1) is compare_scores: a chord passes when the
game's score step for it equals Hydra's points for it. A keyboard press
carries no strike velocity, so the keys route forgives a ghost or accent
chord and the kit route does not. A chord of the score JSON with no row in
the log fails, and so does a run whose stop reason is not the song's end
(stop_check). The log's own `hit` column is the player's and is not read
here.

The report prints the first FAIL with the two numbers, the counts, the final
totals side by side and the spread of the press times against the chart (the
clock at the read before the send minus the chord's ms).

Usage:
    python tools\\ch_probe\\experiments\\score_compare.py <press.csv> <score.json>
        --route kit|keys --stop <RunResult.stop> [--stop-message <text>]

Exits 0 when every check passes, 1 otherwise.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import statistics
import sys
from dataclasses import dataclass
from typing import List, Optional, Sequence, TextIO

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments.overlay_logs import (
    KEYS_PRESS_COLUMNS, KIT_PRESS_COLUMNS, PLAYER_PRESS_COLUMNS, PRESS_COLUMNS_WITHOUT_PLAYER,
    PRESS_COLUMNS_WITHOUT_RETIME, PRESS_COLUMNS_WITHOUT_WALL, PRESS_LOG_COLUMNS,
    RETIME_PRESS_COLUMNS, WALL_PRESS_COLUMNS)
from tools.ch_probe.experiments.play_chart import STOP_DONE

# The route words; game_tools.ROUTE_KEYS and ROUTE_KIT own them, and a test
# pins these to them. Written out because game_tools imports the whole MCP
# side, which this script never needs.
ROUTE_KEYS = "keys"
ROUTE_KIT = "kit"

# The words hydra_replay writes in a note's "dynamic" for a ghost and an
# accent; dynamic_str in src/core/model.cpp owns them.
_DYNAMIC_WORDS = ("ghost", "accent")

# The older press logs overlay_logs names (KEYS_PRESS_COLUMNS,
# PRESS_COLUMNS_WITHOUT_WALL, PRESS_COLUMNS_WITHOUT_PLAYER,
# PRESS_COLUMNS_WITHOUT_RETIME) load too, with the missing columns blank.


@dataclass(frozen=True)
class Press:
    """One press-log row. The kit, wall, player and retime fields keep the
    log's text as written; they are blank for a keys-route run (kit fields),
    for a row they do not apply to (player and retime fields) and for a log
    from before the column existed. The score check reads none of the player
    or retime fields, and not `hit`."""
    index: int
    chart_ms: float
    clock_ms_at_press: float
    score_before: int
    score_after: int
    hit: bool
    down_frame: str = ""
    up_frame: str = ""
    down_ms: str = ""
    up_ms: str = ""
    wall_s_at_read: str = ""
    wall_s_at_reply: str = ""
    scored_between: str = ""
    end_read_ms: str = ""
    probe: str = ""
    retimed: str = ""


@dataclass(frozen=True)
class HydraChord:
    """One chord of the score JSON: its `index`, Hydra's `cum.total` after it,
    and whether any of its notes is a ghost or an accent."""
    index: int
    total: int
    ghost_or_accent: bool


def load_press_log(path: str) -> List[Press]:
    """The press log's rows. Raises ValueError when the header is not
    PRESS_LOG_COLUMNS or one of the older headers this module names."""
    accepted = (PRESS_LOG_COLUMNS, PRESS_COLUMNS_WITHOUT_RETIME, PRESS_COLUMNS_WITHOUT_PLAYER,
                PRESS_COLUMNS_WITHOUT_WALL, KEYS_PRESS_COLUMNS)
    with open(path, encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        if tuple(reader.fieldnames or ()) not in accepted:
            raise ValueError(
                f"{path}: header {reader.fieldnames} is not "
                + " or ".join(str(list(a)) for a in accepted))
        return [Press(int(row["index"]), float(row["chart_ms"]),
                      float(row["clock_ms_at_press"]), int(row["score_before"]),
                      int(row["score_after"]), row["hit"] == "1",
                      **{c: row.get(c) or ""
                         for c in KIT_PRESS_COLUMNS + WALL_PRESS_COLUMNS
                         + PLAYER_PRESS_COLUMNS + RETIME_PRESS_COLUMNS})
                for row in reader]


def chords_from_score(score: dict) -> List[HydraChord]:
    """The chords of a parsed score JSON, in its order."""
    return [HydraChord(c["index"], c["cum"]["total"],
                       any(n["dynamic"] in _DYNAMIC_WORDS for n in c["notes"]))
            for c in score["chords"]]


# --- the score check ------------------------------------------------------------

@dataclass(frozen=True)
class ChordVerdict:
    """The score check at one chord. `status` is "equal" (the step equals
    Hydra's points), "ghost_accent" (it does not, on a keys-route chord with a
    ghost or accent note) or "FAIL". `game` is the game's step and `hydra`
    Hydra's points; `score_after` is the game's score after the chord."""
    index: int
    game: int
    hydra: int
    score_after: int
    status: str
    ghost_or_accent: bool


def compare_scores(presses: Sequence[Press], chords: Sequence[HydraChord],
                   route: str) -> List[ChordVerdict]:
    """One verdict per chord the log has rows for, in log order. Consecutive
    rows with one index are one chord (a split chord has a row per press):
    the game's step is the score after its last row minus the score before
    its first, and Hydra's points are its `cum.total` minus the chord's
    before it in the score JSON. Raises ValueError for a press whose index is
    not in the score JSON and for a route that is neither ROUTE_KEYS nor
    ROUTE_KIT."""
    if route not in (ROUTE_KEYS, ROUTE_KIT):
        raise ValueError(f"route {route!r} is neither {ROUTE_KEYS!r} nor {ROUTE_KIT!r}")
    position = {c.index: i for i, c in enumerate(chords)}
    verdicts: List[ChordVerdict] = []
    k = 0
    while k < len(presses):
        first = presses[k]
        if first.index not in position:
            raise ValueError(f"press for chord index {first.index} is not in the score JSON")
        last = k
        while last + 1 < len(presses) and presses[last + 1].index == first.index:
            last += 1
        i = position[first.index]
        chord = chords[i]
        points = chord.total - (chords[i - 1].total if i > 0 else 0)
        step = presses[last].score_after - first.score_before
        if step == points:
            status = "equal"
        elif route == ROUTE_KEYS and chord.ghost_or_accent:
            status = "ghost_accent"
        else:
            status = "FAIL"
        verdicts.append(ChordVerdict(first.index, step, points, presses[last].score_after,
                                     status, chord.ghost_or_accent))
        k = last + 1
    return verdicts


def stop_check(stop: Optional[str], message: str = "") -> tuple:
    """(passed, the PASS or FAIL line for how the run ended). Only STOP_DONE,
    play_chart's stop for a run that reached the song's end, passes (D319
    item 1); a stop that was not given fails too. `message` is the run's
    RunResult.message, printed beside a failing stop."""
    if stop is None:
        return False, "FAIL run end: the run's stop reason was not given"
    if stop == STOP_DONE:
        return True, "PASS run end: the run reached the song's end"
    why = f": {message}" if message else ""
    return False, f"FAIL run end: the run stopped before the song's end ({stop}){why}"


# --- the press-time spread ----------------------------------------------------------

def press_offsets_ms(presses: Sequence[Press]) -> List[float]:
    """The clock at the read before each send minus the chord's chart ms."""
    return [p.clock_ms_at_press - p.chart_ms for p in presses]


def spread(values: Sequence[float]) -> tuple:
    """(median, 5th percentile, 95th percentile). The percentiles are
    statistics.quantiles' "inclusive" cut points."""
    if len(values) == 1:
        return values[0], values[0], values[0]
    cuts = statistics.quantiles(values, n=20, method="inclusive")
    return statistics.median(values), cuts[0], cuts[-1]


# --- the report ---------------------------------------------------------------------

def report(presses: Sequence[Press], score: dict, out: Optional[TextIO] = None, *,
           route: str, stop: Optional[str], stop_message: str = "") -> bool:
    """Print every check, the run's end first. True when all pass."""
    out = sys.stdout if out is None else out
    stop_ok, stop_line = stop_check(stop, stop_message)
    print(stop_line, file=out)
    if not presses:
        print("FAIL the press log has no rows", file=out)
        return False
    chords = chords_from_score(score)
    verdicts = compare_scores(presses, chords, route)

    if route == ROUTE_KEYS:
        print("Step at each ghost or accent chord (keys route: these may differ):",
              file=out)
        for v in verdicts:
            if v.ghost_or_accent:
                print(f"  chord index {v.index}: game {v.game:+d}, Hydra {v.hydra:+d}",
                      file=out)

    fails = [v for v in verdicts if v.status == "FAIL"]
    counts = {s: sum(1 for v in verdicts if v.status == s)
              for s in ("equal", "ghost_accent", "FAIL")}
    print(f"\nScore step at {len(verdicts)} chords: {counts['equal']} equal to Hydra's "
          f"points, {counts['ghost_accent']} different at a ghost or accent chord on "
          f"the keys route, {counts['FAIL']} failed", file=out)
    if fails:
        f = fails[0]
        print(f"FAIL score step at chord index {f.index}: game {f.game:+d}, "
              f"Hydra {f.hydra:+d}", file=out)
    else:
        print("PASS score step: every chord's step equals Hydra's points", file=out)

    pressed = {v.index for v in verdicts}
    unplayed = [c.index for c in chords if c.index not in pressed]
    if unplayed:
        shown = ", ".join(str(i) for i in unplayed[:10])
        more = f" and {len(unplayed) - 10} more" if len(unplayed) > 10 else ""
        print(f"FAIL chords with no press-log row: {len(unplayed)} of {len(chords)} "
              f"(chord index {shown}{more})", file=out)
    else:
        print(f"PASS every chord has a press-log row ({len(chords)} chords)", file=out)

    last = verdicts[-1]
    print(f"Final: game {last.score_after} after chord index {last.index}; "
          f"Hydra's whole-chart total {score['final']['total']}", file=out)

    median, p5, p95 = spread(press_offsets_ms(presses))
    print(f"\nClock at the read before the send minus chart ms: median {median:+.2f}, "
          f"5th percentile {p5:+.2f}, 95th percentile {p95:+.2f} "
          f"({len(presses)} presses)", file=out)
    return stop_ok and not fails and not unplayed


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Compare a press log's scores with a hydra_replay score JSON.")
    parser.add_argument("press_log", help="the CSV play_chart.py --log wrote")
    parser.add_argument("score_json", help="the hydra_replay score JSON it played")
    parser.add_argument("--route", required=True, choices=(ROUTE_KIT, ROUTE_KEYS),
                        help="the route the run pressed through")
    parser.add_argument("--stop", default=None,
                        help="the run's stop reason (play_chart's RunResult.stop); "
                             "without it the run fails")
    parser.add_argument("--stop-message", default="",
                        help="the run's RunResult.message, printed beside the stop")
    args = parser.parse_args(argv)
    presses = load_press_log(args.press_log)
    with open(args.score_json, encoding="utf-8") as f:
        score = json.load(f)
    ok = report(presses, score, route=args.route, stop=args.stop,
                stop_message=args.stop_message)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
