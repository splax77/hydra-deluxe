"""Time what an overlay would pay to find the engine and to read it.

Plain version: an overlay has to find the game's engine object once per song
(a scan of the game's memory) and then read it many times a second. This
script prices both while a song plays. It finds the engine FIND_RUNS times in a
row and checks every find lands on the same address, then times READ_COUNT
whole-snapshot reads and READ_COUNT single 4-byte reads. For each it prints
the fastest, the median and the slowest time in ms, and it prints how many
memory regions and bytes each scan walked.

It opens the game read-only and never sends it input. Run it only at the
sitting's step 5, never beside the auto-player: the scans would add noise to
the clock timings the other runs measure.

Usage (from the repo root, with a song playing):
    py tools\\ch_probe\\experiments\\poll_cost.py
"""

from __future__ import annotations

import os
import statistics
import sys
import time
from typing import Callable, List, Optional, Sequence, TextIO, Tuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C, engine_finder
from tools.ch_probe.engine import EngineModel
from tools.ch_probe.process import open_process

# How many engine finds and how many reads are timed (the plan's Task D
# step 3).
FIND_RUNS = 5
READ_COUNT = 1000

# The exit code when no song was playing within the finder's cap.
EXIT_NO_SONG = 2

Readers = Tuple[Callable[[], object], Callable[[], object]]


def spread_ms(durations_s: Sequence[float]) -> Tuple[float, float, float]:
    """The fastest, median and slowest of a list of durations, in ms."""
    ms = [C.s_to_ms(d) for d in durations_s]
    return min(ms), statistics.median(ms), max(ms)


def timing_line(name: str, durations_s: Sequence[float]) -> str:
    """One report line: what was timed, and its min, median and max in ms."""
    low, mid, high = spread_ms(durations_s)
    return (f"{name}: min {low:.3f} ms, median {mid:.3f} ms, "
            f"max {high:.3f} ms ({len(durations_s)} runs)")


def time_each(fn: Callable[[], object], count: int,
              perf: Callable[[], float]) -> Tuple[List[float], List[object]]:
    """Call `fn` `count` times; return how long each call took (seconds, by
    `perf`) and what each returned."""
    durations: List[float] = []
    results: List[object] = []
    for _ in range(count):
        t0 = perf()
        results.append(fn())
        durations.append(perf() - t0)
    return durations, results


def measure(
    find_once: Callable[[], int],
    make_readers: Callable[[int], Readers],
    scans: List[Tuple[int, int]],
    *,
    perf: Callable[[], float] = time.perf_counter,
    out: Optional[TextIO] = None,
    find_runs: int = FIND_RUNS,
    read_count: int = READ_COUNT,
) -> int:
    """Time `find_runs` engine finds, then `read_count` snapshot reads and
    `read_count` single reads of the engine the last find returned, and print
    the report. `scans` is filled by find_once with (regions, bytes) for every
    memory scan it ran. `make_readers(address)` returns the snapshot read and
    the single read to time. Returns the exit code: 0, or EXIT_NO_SONG when a
    find hit its cap (its message is printed)."""
    out = sys.stdout if out is None else out
    try:
        find_times, addresses = time_each(find_once, find_runs, perf)
    except engine_finder.EngineSearchTimeout as e:
        out.write(f"\n{e}\n")
        return EXIT_NO_SONG
    out.write("\n")

    distinct = list(dict.fromkeys(addresses))
    if len(distinct) > 1:
        out.write("DISAGREE: the engine finds returned different addresses: "
                  + ", ".join(f"{a:#x}" for a in addresses) + "\n")
    else:
        out.write(f"All {find_runs} engine finds agree: {distinct[0]:#x}\n")
    out.write(timing_line("engine find", find_times) + "\n")
    if scans:
        regions = [r for r, _ in scans]
        sizes = [b for _, b in scans]
        out.write(f"memory scans: {len(scans)} in all; regions per scan "
                  f"{min(regions)} to {max(regions)}; bytes per scan "
                  f"{min(sizes):,} to {max(sizes):,}\n")

    snapshot_read, single_read = make_readers(addresses[-1])
    snap_times, _ = time_each(snapshot_read, read_count, perf)
    out.write(timing_line("snapshot() read", snap_times) + "\n")
    single_times, _ = time_each(single_read, read_count, perf)
    out.write(timing_line("read_u32 read", single_times) + "\n")
    return 0


def main(
    *,
    opener: Callable = open_process,
    scan: Callable = engine_finder.scan_for_engine,
    sleep: Callable[[float], None] = time.sleep,
    now: Callable[[], float] = time.monotonic,
    perf: Callable[[], float] = time.perf_counter,
    out: Optional[TextIO] = None,
) -> int:
    """Open the game read-only, check the build, and run measure() on the
    live engine. The keyword arguments are the seams a test fakes.

    LIVE-ONLY with its defaults: it opens Clone Hero and scans its memory."""
    out = sys.stdout if out is None else out
    proc = opener(read_only=True)
    try:
        proc.verify_targets()
        patterns = [engine_finder.normal_pattern(proc)]
        scans: List[Tuple[int, int]] = []

        def recording_scan(p, back, front):
            hits, regions, size = scan(p, back, front)
            scans.append((regions, size))
            return hits, regions, size

        def find_once() -> int:
            return engine_finder.find_live_engine_capped(
                proc, patterns, scan=recording_scan, sleep=sleep, now=now,
                out=out)

        def make_readers(address: int) -> Readers:
            engine = EngineModel(proc)
            engine.use_object(address)
            return engine.snapshot, lambda: proc.read_u32(address + C.OFF_SCORE)

        out.write(f"Finding the engine {FIND_RUNS} times "
                  "(start the song if it is not playing)...\n")
        out.flush()
        try:
            return measure(find_once, make_readers, scans, perf=perf, out=out)
        except OSError as e:
            out.write(f"\nLost the process: {e}\n")
            return 1
    finally:
        proc.close()


if __name__ == "__main__":
    sys.exit(main())
