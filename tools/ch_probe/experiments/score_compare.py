"""Compare the game's score in a press log with Hydra's running total.

Inputs: the press log play_chart.py writes with --log (its columns are
overlay_logs.PRESS_LOG_COLUMNS) and the `hydra_replay score --path` JSON the
run played. Pure logic: no process, no game.

The score check. For each press, the game's `score_after` is set against
Hydra's `cum.total` at that chord, through the running difference
`game score - Hydra total`. The difference starts at 0. A press whose
difference is unchanged is "equal". A press whose score equals the previous
chord's total plus the running difference is "lagged": the 5 ms read landed
before the engine credited the press, which is not a fault. A keyboard press
carries no strike velocity, so the difference may change on a chord with a
ghost or accent note; it may also change on the chord right after one whose
read lagged, because that is where the change shows. Any other change is a
FAIL, and the new difference becomes the one later chords are held to, so one
fault is reported once. Why ghost and accent chords are exempt is in
docs/superpowers/plans/2026-10-28-overlay-spike.md, "How Star Power starts on
drums".

The report prints the running difference after every ghost or accent chord,
the first FAIL with the two numbers, the counts, the final totals side by
side, the spread of the press times against the chart (clock at the press
minus the chord's ms), and the hit rate, which is a FAIL under MIN_HIT_PERCENT.

Usage:
    python tools\\ch_probe\\experiments\\score_compare.py <press.csv> <score.json>

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

from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS

# A run that hits fewer than this percent of its presses is a FAIL (the
# overlay spike plan, Task B step 3).
MIN_HIT_PERCENT = 98

# The words hydra_replay writes in a note's "dynamic" for a ghost and an
# accent; dynamic_str in src/core/model.cpp owns them.
_DYNAMIC_WORDS = ("ghost", "accent")


@dataclass(frozen=True)
class Press:
    """One press-log row."""
    index: int
    chart_ms: float
    clock_ms_at_press: float
    score_before: int
    score_after: int
    hit: bool


@dataclass(frozen=True)
class HydraChord:
    """One chord of the score JSON: its `index`, Hydra's `cum.total` after it,
    and whether any of its notes is a ghost or an accent."""
    index: int
    total: int
    ghost_or_accent: bool


def load_press_log(path: str) -> List[Press]:
    """The press log's rows. Raises ValueError when the header is not
    PRESS_LOG_COLUMNS."""
    with open(path, encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        if tuple(reader.fieldnames or ()) != PRESS_LOG_COLUMNS:
            raise ValueError(
                f"{path}: header {reader.fieldnames} is not {list(PRESS_LOG_COLUMNS)}")
        return [Press(int(row["index"]), float(row["chart_ms"]),
                      float(row["clock_ms_at_press"]), int(row["score_before"]),
                      int(row["score_after"]), row["hit"] == "1")
                for row in reader]


def chords_from_score(score: dict) -> List[HydraChord]:
    """The chords of a parsed score JSON, in its order."""
    return [HydraChord(c["index"], c["cum"]["total"],
                       any(n["dynamic"] in _DYNAMIC_WORDS for n in c["notes"]))
            for c in score["chords"]]


# --- the score check ------------------------------------------------------------

@dataclass(frozen=True)
class ChordVerdict:
    """The score check at one press. `status` is "equal", "lagged",
    "ghost_accent" (the difference changed where it may) or "FAIL".
    `difference` is the running difference after this press."""
    index: int
    game: int
    hydra: int
    difference: int
    previous_difference: int
    status: str
    ghost_or_accent: bool


def compare_scores(presses: Sequence[Press],
                   chords: Sequence[HydraChord]) -> List[ChordVerdict]:
    """One verdict per press, in log order (the rule is in the module
    docstring). Raises ValueError for a press whose index is not in the
    score JSON."""
    position = {c.index: i for i, c in enumerate(chords)}
    verdicts: List[ChordVerdict] = []
    running = 0
    change_may_show = False   # the last press was a ghost/accent chord read lagged
    for p in presses:
        if p.index not in position:
            raise ValueError(f"press for chord index {p.index} is not in the score JSON")
        i = position[p.index]
        chord = chords[i]
        previous_total = chords[i - 1].total if i > 0 else 0
        difference = p.score_after - chord.total
        before = running
        if difference == running:
            status = "equal"
        elif p.score_after - previous_total == running:
            status = "lagged"
        elif chord.ghost_or_accent or change_may_show:
            status = "ghost_accent"
            running = difference
        else:
            status = "FAIL"
            running = difference
        change_may_show = chord.ghost_or_accent and status == "lagged"
        verdicts.append(ChordVerdict(p.index, p.score_after, chord.total, running,
                                     before, status, chord.ghost_or_accent))
    return verdicts


# --- the press-time spread and the hit rate ---------------------------------------

def press_offsets_ms(presses: Sequence[Press]) -> List[float]:
    """Clock at each press minus the chord's chart ms."""
    return [p.clock_ms_at_press - p.chart_ms for p in presses]


def spread(values: Sequence[float]) -> tuple:
    """(median, 5th percentile, 95th percentile). The percentiles are
    statistics.quantiles' "inclusive" cut points."""
    if len(values) == 1:
        return values[0], values[0], values[0]
    cuts = statistics.quantiles(values, n=20, method="inclusive")
    return statistics.median(values), cuts[0], cuts[-1]


def hit_rate_line(presses: Sequence[Press]) -> tuple:
    """(passed, the PASS or FAIL line for the hit rate)."""
    hits = sum(1 for p in presses if p.hit)
    n = len(presses)
    rate = f"{hits / n * 100:.1f}% ({hits} of {n})"
    if hits * 100 >= MIN_HIT_PERCENT * n:
        return True, f"PASS hit rate {rate}"
    first = next(p for p in presses if not p.hit)
    return False, (f"FAIL hit rate {rate}, under {MIN_HIT_PERCENT}%; first miss: "
                   f"chord index {first.index} at {first.chart_ms:.1f} ms")


# --- the report ---------------------------------------------------------------------

def report(presses: Sequence[Press], score: dict,
           out: Optional[TextIO] = None) -> bool:
    """Print every check. True when all pass."""
    out = sys.stdout if out is None else out
    if not presses:
        print("FAIL the press log has no rows", file=out)
        return False
    verdicts = compare_scores(presses, chords_from_score(score))

    print("Running difference (game - Hydra) after each ghost or accent chord:",
          file=out)
    for v in verdicts:
        if v.ghost_or_accent:
            lag = ", read lagged" if v.status == "lagged" else ""
            print(f"  chord index {v.index}: {v.difference:+d} "
                  f"(was {v.previous_difference:+d}{lag})", file=out)

    fails = [v for v in verdicts if v.status == "FAIL"]
    counts = {s: sum(1 for v in verdicts if v.status == s)
              for s in ("equal", "lagged", "ghost_accent", "FAIL")}
    print(f"\nScore at {len(verdicts)} presses: {counts['equal']} equal, "
          f"{counts['lagged']} lagged, {counts['ghost_accent']} changed at a ghost "
          f"or accent chord, {counts['FAIL']} failed", file=out)
    if fails:
        f = fails[0]
        print(f"FAIL score at chord index {f.index}: game {f.game}, Hydra {f.hydra} "
              f"(difference {f.difference:+d}, was {f.previous_difference:+d})", file=out)
    else:
        print("PASS score: the difference changed only at ghost or accent chords",
              file=out)

    last = verdicts[-1]
    print(f"Final: game {last.game}, Hydra {last.hydra} at chord index {last.index}; "
          f"Hydra's whole-chart total {score['final']['total']}", file=out)

    median, p5, p95 = spread(press_offsets_ms(presses))
    print(f"\nClock at press minus chart ms: median {median:+.2f}, "
          f"5th percentile {p5:+.2f}, 95th percentile {p95:+.2f} "
          f"({len(presses)} presses)", file=out)

    hits_ok, line = hit_rate_line(presses)
    print(line, file=out)
    return hits_ok and not fails


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Compare a press log's scores with a hydra_replay score JSON.")
    parser.add_argument("press_log", help="the CSV play_chart.py --log wrote")
    parser.add_argument("score_json", help="the hydra_replay score JSON it played")
    args = parser.parse_args(argv)
    presses = load_press_log(args.press_log)
    with open(args.score_json, encoding="utf-8") as f:
        score = json.load(f)
    return 0 if report(presses, score) else 1


if __name__ == "__main__":
    sys.exit(main())
