"""The pause check on a Star Power watcher log that needs no activation.

Plain version: sp_watch.py writes a CSV of the engine's fields while a song
plays. This script reads that CSV (columns from overlay_logs.WATCH_COLUMNS)
and prints one check: two rows far apart in wall time whose clock barely moved
are a pause. The report prints each pause and every other field that changed
across it (expected: none).

Everything here is pure: it reads files and returns lines, and touches no
process.

Usage:
    py tools\\ch_probe\\experiments\\watch_checks.py <watch.csv>
"""

from __future__ import annotations

import argparse
import csv
import math
import os
import sys
from typing import Optional, Sequence

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments.overlay_logs import (
    WATCH_CLOCK_COLUMNS, WATCH_COLUMNS, WATCH_HALF_COLUMNS, watch_header)

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


def same_value(a, b) -> bool:
    """Whether two logged values are equal, a NaN read counting as equal to
    itself (sp_watch.py's change test treats it the same way)."""
    return a == b or (a != a and b != b)


# The column groups overlay_logs says a log may lack, each one whole.
_OPTIONAL_GROUPS = (WATCH_CLOCK_COLUMNS, WATCH_HALF_COLUMNS)


def accepted_headers() -> tuple:
    """Every header a watcher log may have: WATCH_COLUMNS with any of the
    groups in _OPTIONAL_GROUPS left out, in WATCH_COLUMNS order."""
    headers = []
    for mask in range(1 << len(_OPTIONAL_GROUPS)):
        left_out = {c for i, group in enumerate(_OPTIONAL_GROUPS)
                    if mask >> i & 1 for c in group}
        headers.append(tuple(c for c in WATCH_COLUMNS if c not in left_out))
    return tuple(headers)


def load_watch_rows(path: str) -> list[dict]:
    """The watcher log's rows, each a dict keyed by WATCH_COLUMNS. A log
    that lacks a group overlay_logs lets it lack (accepted_headers) loads with
    those columns NaN, and a saved log's older column names are read through
    overlay_logs.watch_header. Raises ValueError on any other header."""
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.reader(f)
        header = watch_header(next(reader, ()))
        if header not in accepted_headers():
            raise ValueError(
                f"{path}: header {header} is not overlay_logs.WATCH_COLUMNS")
        blanks = dict.fromkeys(WATCH_COLUMNS, math.nan)
        return [{**blanks, **dict(zip(header, map(parse_value, row)))}
                for row in reader]


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
        moved = [f"{c} {a[c]} -> {b[c]}" for c in others
                 if not same_value(a[c], b[c])]
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
    """Read the file named in `argv` and return the report lines."""
    parser = argparse.ArgumentParser(
        description="The pause check on an sp_watch.py log.")
    parser.add_argument("watch_csv")
    args = parser.parse_args(argv)
    rows = load_watch_rows(args.watch_csv)
    lines = [f"{os.path.basename(args.watch_csv)}: {len(rows)} rows"]
    lines.extend(pause_check(rows))
    return lines


def main() -> None:
    for line in run():
        print(line)


if __name__ == "__main__":
    main()
