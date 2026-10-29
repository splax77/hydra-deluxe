"""Watch the engine's Star Power fields while a song plays, and log them.

Plain version: this script reads the live engine object over and over and
writes a CSV row each time anything it logs changes. The columns come from
overlay_logs.WATCH_COLUMNS; each one except `wall_s` is a field of
engine.Snapshot, so engine.decode_snapshot is the one place that says where a
field lives and how wide it is. The Star Power offsets are not yet confirmed
live (see constants.py), which is what this log is for.

A row is written only on a change, plus one final row when the run ends. So a
paused game shows as two rows far apart in wall time with the same clock.
watch_checks.py and sp_compare.py read the log.

`wall_s` is the computer's clock in seconds since 1970 (time.time), so a row
can be lined up with log_tail.py's wall-clock lines.

The run ends when the song clock stops (live.StoppedCheck: frozen too long,
or jumped back), when the engine can no longer be read, on Ctrl-C, or at
RUN_CAP_S of wall time.

Usage:
    py tools\\ch_probe\\experiments\\sp_watch.py [--out <csv>]

Run it, then start the song. Exit code 2 means no song started inside the
engine search's cap.
"""

from __future__ import annotations

import argparse
import csv
import os
import sys
import time
from dataclasses import dataclass
from typing import Callable, Optional, Sequence, TextIO

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import engine_finder
from tools.ch_probe.engine import EngineModel, Snapshot
from tools.ch_probe.process import open_process
from tools.ch_probe.experiments import live
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS

# Where the log goes when --out is not given.
RESULTS_DIR = os.path.join(os.path.dirname(__file__), "results", "overlay-spike")

# The pause between two reads of the engine (the plan's Task A step 3).
POLL_S = 0.004

# The longest a run lasts, in seconds of wall time (the plan's Task A step 3).
RUN_CAP_S = 240.0

# The one column that is not a Snapshot field: when the row was read.
WALL_COLUMN = "wall_s"

# Exit code for "no song started inside the engine search's cap".
EXIT_NO_SONG = 2


def watch_row(wall_s: float, snap: Snapshot) -> list:
    """One CSV row in WATCH_COLUMNS order: the wall time, then each logged
    Snapshot field under its own name."""
    return [wall_s if col == WALL_COLUMN else getattr(snap, col)
            for col in WATCH_COLUMNS]


def logged_values(snap: Snapshot) -> tuple:
    """The logged fields of a snapshot as the CSV writes them (str, the way
    the csv module writes a value). Two snapshots with equal results would
    write the same row apart from the wall time. Comparing the written text
    also treats a NaN read as equal to itself."""
    return tuple(str(getattr(snap, col)) for col in WATCH_COLUMNS
                 if col != WALL_COLUMN)


@dataclass(frozen=True)
class WatchSummary:
    """What a run did: rows written, the first and last clock read, and why
    it stopped, in words."""

    rows: int
    first_clock_s: float
    last_clock_s: float
    reason: str


def watch(
    snapshot: Callable[[], Snapshot],
    write_row: Callable[[list], None],
    *,
    now: Callable[[], float] = time.time,
    sleep: Callable[[float], None] = time.sleep,
    poll_s: float = POLL_S,
    cap_s: float = RUN_CAP_S,
) -> WatchSummary:
    """Poll `snapshot` every `poll_s` seconds and write a row on each change.

    A row is written for the first read, for every read whose logged fields
    differ from the last row written, and once more at the end for the last
    read when it was not written already. The run stops on the first of: the
    clock stopping (live.StoppedCheck raises), a failed read (OSError),
    Ctrl-C, or `cap_s` seconds of `now()` since the start."""
    start = now()
    first = snapshot()
    t = now()
    stopped = live.StoppedCheck(first.clock_s, t)
    write_row(watch_row(t, first))
    rows = 1
    last_written = logged_values(first)
    latest = (t, first)
    latest_written = True
    reason = ""

    try:
        while True:
            if now() - start >= cap_s:
                reason = f"run cap of {cap_s:g} s reached"
                break
            sleep(poll_s)
            try:
                snap = snapshot()
            except OSError as e:
                reason = f"lost the engine ({e})"
                break
            t = now()
            latest = (t, snap)
            latest_written = False
            try:
                stopped.check(snap.clock_s, t)
            except (live.ClockFrozen, live.ClockJumpedBack) as e:
                reason = str(e)
                break
            values = logged_values(snap)
            if values != last_written:
                write_row(watch_row(t, snap))
                rows += 1
                last_written = values
                latest_written = True
    except KeyboardInterrupt:
        reason = "stopped by Ctrl-C"

    if not latest_written:
        write_row(watch_row(*latest))
        rows += 1
    return WatchSummary(rows, first.clock_s, latest[1].clock_s, reason)


def find_engine_live(proc) -> int:
    """LIVE-ONLY: the live engine's address, through the one capped search
    (engine_finder.find_live_engine_capped) over every window pattern.
    Raises engine_finder.EngineSearchTimeout when no song starts in time."""
    return engine_finder.find_live_engine_capped(
        proc, engine_finder.all_patterns(proc))


def default_out_path() -> str:
    """A fresh file name under RESULTS_DIR, stamped with the local time."""
    return os.path.join(
        RESULTS_DIR, f"sp_watch_{time.strftime('%Y%m%d_%H%M%S')}.csv")


def run(
    argv: Optional[Sequence[str]] = None,
    *,
    opener: Callable[..., object] = open_process,
    find_engine: Callable[[object], int] = find_engine_live,
    now: Callable[[], float] = time.time,
    sleep: Callable[[float], None] = time.sleep,
    out: Optional[TextIO] = None,
) -> int:
    """The script: open the game read-only, check the build, find the live
    engine, watch it, and print what the run did. Returns the exit code.

    The defaults reach the running game (LIVE-ONLY); the tests pass fakes for
    `opener`, `find_engine`, `now` and `sleep`."""
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(
        description="Log the engine's Star Power fields while a song plays.")
    parser.add_argument("--out", help="CSV to write (default: a new file "
                        "under experiments/results/overlay-spike/)")
    args = parser.parse_args(argv)
    out_path = args.out or default_out_path()

    print("Connecting to Clone Hero (read-only)...", file=out)
    proc = opener(read_only=True)
    try:
        proc.verify_targets()
        print("  Waiting for a song to play (start it now)...", file=out)
        try:
            engine_ptr = find_engine(proc)
        except engine_finder.EngineSearchTimeout as e:
            print(f"\n  {e}", file=out)
            return EXIT_NO_SONG
        model = EngineModel(proc)
        model.use_object(engine_ptr)
        print(f"  Engine at {engine_ptr:#x}. Writing {out_path}", file=out)

        parent = os.path.dirname(out_path)
        if parent:
            os.makedirs(parent, exist_ok=True)
        with open(out_path, "w", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(WATCH_COLUMNS)
            summary = watch(model.snapshot, writer.writerow, now=now,
                            sleep=sleep)
    finally:
        proc.close()

    print(f"  Stopped: {summary.reason}.", file=out)
    print(f"  Rows written: {summary.rows}", file=out)
    print(f"  Engine: {engine_ptr:#x}", file=out)
    print(f"  First clock: {summary.first_clock_s:.3f} s, "
          f"last clock: {summary.last_clock_s:.3f} s", file=out)
    return 0


def main() -> None:
    """LIVE-ONLY entry point: run() against the real game."""
    sys.exit(run())


if __name__ == "__main__":
    main()
