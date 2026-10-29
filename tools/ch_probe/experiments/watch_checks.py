"""The two checks on a Star Power watcher log that need no activation.

Plain version: sp_watch.py writes a CSV of the engine's fields while a song
plays. This script reads that CSV (columns from overlay_logs.WATCH_COLUMNS)
and prints two checks, each with a PASS or FAIL line and its numbers:

  * Hit time. Is the field at +0x2E0 the time of the last hit? For each
    chord in the `hydra_replay score` JSON, take the first row near the
    chord's time in which `hit_time_s` changed, and see how far the new value
    sits from the chord. The verdict uses constants.HIT_TIME_MAX_GAP_MS, the
    same limit experiments/watch_window.py uses. This check means something
    only on an auto-player run: the game's bot may hit exactly on the note's
    time, which would match whatever the field holds.
  * Pause. Two rows far apart in wall time whose clock barely moved are a
    pause. The report prints each pause and every other field that changed
    across it (expected: none).

Everything here is pure: it reads files and returns lines, and touches no
process.

Usage:
    py tools\\ch_probe\\experiments\\watch_checks.py <watch.csv> [--score <score.json>]
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import statistics
import sys
from typing import Optional, Sequence

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS

# How far from a chord's time (song clock, ms) a row may be and still be
# searched for that chord's hit-time change (the plan's Task A step 4).
HIT_SEARCH_MS = 50.0

# A pause: two consecutive rows more than PAUSE_MIN_WALL_S apart in wall time
# whose clocks differ by less than PAUSE_MAX_CLOCK_S (the plan's Task A
# step 4).
PAUSE_MIN_WALL_S = 1.0
PAUSE_MAX_CLOCK_S = 0.2


# --- reading the files ---------------------------------------------------------

def parse_value(text: str):
    """A logged value as a number: an int when the text is a whole number
    (so a 64-bit field keeps every bit), else a float."""
    try:
        return int(text)
    except ValueError:
        return float(text)


def load_watch_rows(path: str) -> list[dict]:
    """The watcher log's rows, each a dict keyed by WATCH_COLUMNS. Raises
    ValueError when the header is not WATCH_COLUMNS."""
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        header = tuple(next(reader, ()))
        if header != WATCH_COLUMNS:
            raise ValueError(
                f"{path}: header {header} is not overlay_logs.WATCH_COLUMNS")
        return [dict(zip(WATCH_COLUMNS, map(parse_value, row)))
                for row in reader]


def load_chords(path: str) -> list[dict]:
    """The `chords` list of a `hydra_replay score` JSON."""
    with open(path, encoding="utf-8") as f:
        return json.load(f)["chords"]


# --- the hit-time check ----------------------------------------------------------

def hit_time_check(rows: Sequence[dict], chords: Sequence[dict]) -> list[str]:
    """Whether +0x2E0 (`hit_time_s`) behaves like the time of the last hit.

    For each chord whose `ms` lies inside the log's clock span, the change
    row is the first row within HIT_SEARCH_MS of the chord in which
    `hit_time_s` differs from the row before. A row an earlier chord already
    took is skipped, so two close chords do not share one change. The chord
    counts when the new value sits within C.HIT_TIME_MAX_GAP_MS of the chord.
    PASS when every chord in the span counts."""
    changes = [(C.s_to_ms(cur["clock_s"]), C.s_to_ms(cur["hit_time_s"]), i)
               for i, (prev, cur) in enumerate(zip(rows, rows[1:]), start=1)
               if cur["hit_time_s"] != prev["hit_time_s"]]
    if not rows:
        return ["FAIL hit time (+0x2E0): the log has no rows"]
    first_ms = C.s_to_ms(rows[0]["clock_s"])
    last_ms = C.s_to_ms(rows[-1]["clock_s"])
    in_span = [ch for ch in chords if first_ms <= ch["ms"] <= last_ms]

    taken: set[int] = set()
    counted, gaps, misses = 0, [], []
    for ch in in_span:
        found = None
        for clock_ms, value_ms, row_i in changes:
            if row_i in taken or abs(clock_ms - ch["ms"]) > HIT_SEARCH_MS:
                continue
            found = (value_ms, row_i)
            break
        if found is None:
            misses.append(f"  chord {ch['index']} at {ch['ms']:.1f} ms: "
                          f"no hit-time change within {HIT_SEARCH_MS:g} ms")
            continue
        taken.add(found[1])
        gap = found[0] - ch["ms"]
        gaps.append(gap)
        if abs(gap) <= C.HIT_TIME_MAX_GAP_MS:
            counted += 1
        else:
            misses.append(f"  chord {ch['index']} at {ch['ms']:.1f} ms: "
                          f"hit time {found[0]:.1f} ms, {gap:+.1f} ms away")

    ok = bool(in_span) and counted == len(in_span)
    lines = [f"{'PASS' if ok else 'FAIL'} hit time (+0x2E0): {counted} of "
             f"{len(in_span)} chords in the log have a hit time within "
             f"{C.HIT_TIME_MAX_GAP_MS:g} ms ({len(chords) - len(in_span)} "
             f"chords outside the log's clock span)"]
    if gaps:
        lines.append(f"  distance from the chord: median "
                     f"{statistics.median(gaps):+.1f} ms, largest "
                     f"{max(gaps, key=abs):+.1f} ms")
    lines.extend(misses)
    return lines


# --- the pause check -------------------------------------------------------------

def find_pauses(rows: Sequence[dict]) -> list[tuple[int, int]]:
    """Index pairs (i, i + 1) of consecutive rows that make a pause."""
    return [(i, i + 1) for i, (a, b) in enumerate(zip(rows, rows[1:]))
            if b["wall_s"] - a["wall_s"] > PAUSE_MIN_WALL_S
            and abs(b["clock_s"] - a["clock_s"]) < PAUSE_MAX_CLOCK_S]


def pause_check(rows: Sequence[dict]) -> list[str]:
    """Each pause in the log, its span, and every field other than the wall
    time and the clock that differs across it. A pause with no such field is
    PASS. The last row of a log is the watcher's own final row, so a pause
    ending there is labelled as the end of the log."""
    pauses = find_pauses(rows)
    if not pauses:
        return ["pause: no pause in this log"]
    others = [c for c in WATCH_COLUMNS if c not in ("wall_s", "clock_s")]
    lines = []
    for i, j in pauses:
        a, b = rows[i], rows[j]
        moved = [f"{c} {a[c]} -> {b[c]}" for c in others if a[c] != b[c]]
        end_note = (" (ends at the log's final row, where the watcher stopped)"
                    if j == len(rows) - 1 else "")
        lines.append(
            f"{'FAIL' if moved else 'PASS'} pause: wall {a['wall_s']:.3f} to "
            f"{b['wall_s']:.3f} s ({b['wall_s'] - a['wall_s']:.3f} s), clock "
            f"{a['clock_s']:.3f} to {b['clock_s']:.3f} s{end_note}")
        lines.append("  other fields: " + ("; ".join(moved) if moved
                                           else "none moved"))
    return lines


# --- the script ------------------------------------------------------------------

def run(argv: Optional[Sequence[str]] = None) -> list[str]:
    """Read the files named in `argv` and return the report lines."""
    parser = argparse.ArgumentParser(
        description="Hit-time and pause checks on an sp_watch.py log.")
    parser.add_argument("watch_csv")
    parser.add_argument("--score", help="hydra_replay score JSON for the "
                        "hit-time check (auto-player runs only)")
    args = parser.parse_args(argv)
    rows = load_watch_rows(args.watch_csv)
    lines = [f"{os.path.basename(args.watch_csv)}: {len(rows)} rows"]
    if args.score:
        lines.extend(hit_time_check(rows, load_chords(args.score)))
    else:
        lines.append("hit time (+0x2E0): not checked (no --score given)")
    lines.extend(pause_check(rows))
    return lines


def main() -> None:
    for line in run():
        print(line)


if __name__ == "__main__":
    main()
