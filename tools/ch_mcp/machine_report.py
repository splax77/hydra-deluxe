"""The report over one machine recorder folder: who had the CPU, which threads
waited for one, and whether the game's clock kept up with wall time. Read-only.

    py -I tools/ch_mcp/machine_report.py <folder> <game pid> [--worker <pid>]
        [--from <wall s>] [--to <wall s>]

docs/superpowers/plans/2026-11-07-ch-run-timing.md, design point 2 ("The clock's
excess" and "The reader") and the guard at the top of its T6 section, says what
each section means. machine_recorder.py owns the three files and the `reads_as`
word; this module only reads them.

A sample's amounts cover the slice since the sample before it, so a window holds
every sample stamped inside it and the slices those samples end. All times stay
exact fractions of the text in the files, so a value on a threshold stays on it.
"""
from __future__ import annotations

import argparse
import csv
import os
import statistics
import sys
from collections import defaultdict
from dataclasses import dataclass
from fractions import Fraction
from typing import Dict, Iterable, List, Optional, Sequence, Set, Tuple

if __package__ in (None, ""):
    # Run as a script (py -I leaves the repo off sys.path).
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

from tools.ch_mcp import game_files, machine_recorder, quiet  # noqa: E402
from tools.ch_mcp.sp_tools import GAP_MS  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.experiments.sp_compare import gap_summary  # noqa: E402

# Question 14 of the plan: how many sample periods a clock excess must pass, by
# size, to count as long.
LONG_STEP_FACTOR = 2
# Question 16 of the plan: how many consecutive ready samples make a stretch.
READY_STRETCH_MIN = 2
# Question 2 of the plan and quiet.py's factors list: listed even at zero.
ALWAYS_LISTED_NAMES = quiet.COMPILER_NAMES + quiet.REPORTED_NAMES
# Design point 2 of the plan: how many long excesses are listed, and which
# percentile of the clock's steps is printed.
LONGEST_LISTED = 10
STEP_PERCENTILE = 99

# The `key = value` lines of settings.ini that hold the game's frame cap. They
# sit in [video], so game_files.get_game_setting, which reads [game], cannot
# read them.
SETTINGS_KEYS = ("framerate", "vsync")

# machine_recorder.py owns the three files, their columns and the `reads_as` words.
SAMPLES_FILE = machine_recorder.SAMPLES_FILE
PROCESSES_FILE = machine_recorder.PROCESSES_FILE
THREADS_FILE = machine_recorder.THREADS_FILE
SAMPLE_COLUMNS = machine_recorder.SAMPLES_COLUMNS
PROCESS_COLUMNS = machine_recorder.PROCESSES_COLUMNS
THREAD_COLUMNS = machine_recorder.THREADS_COLUMNS
READS_READY = machine_recorder.READY
READS_RUNNING = machine_recorder.RUNNING
# Empty on an ordinary row of samples.csv, so an empty one is no half row.
SAMPLE_OPTIONAL = ("clock_s", "clock_wall_before_s", "clock_wall_after_s", "stop_reason",
                   "process_time_s")
# The Idle process has no name, so an empty one is no half row either.
PROCESS_OPTIONAL = ("name",)

NO_ROWS_NAME = "(no rows)"
NO_NAME = "(no name)"


class Refused(Exception):
    """The report cannot go on. The message says why, for the reader."""


@dataclass(frozen=True)
class Sample:
    number: int
    wall_s: Fraction
    clock_s: Optional[Fraction]
    clock_wall_before_s: Optional[Fraction]
    clock_wall_after_s: Optional[Fraction]
    recorder_pid: int
    period_ms: Fraction
    stop_reason: str
    process_time_s: str


@dataclass(frozen=True)
class ProcessRow:
    sample: int
    pid: int
    parent_pid: int
    name: str
    cpu_ms: Fraction
    read_bytes: Fraction
    write_bytes: Fraction
    hard_faults: Fraction


@dataclass(frozen=True)
class ThreadRow:
    sample: int
    pid: int
    tid: int
    reads_as: str


@dataclass(frozen=True)
class Recording:
    samples: List[Sample]
    processes: List[ProcessRow]
    threads: List[ThreadRow]
    half_rows: int

    @property
    def period_ms(self) -> Fraction:
        return self.samples[0].period_ms


@dataclass(frozen=True)
class Window:
    samples: List[Sample]
    slices: Set[int]
    span_ms: Fraction


@dataclass(frozen=True)
class ClockPair:
    """Two samples next to each other in the clock section."""
    earlier: Sample
    later: Sample

    @property
    def step_ms(self) -> Fraction:
        return _ms(self.later.clock_s - self.earlier.clock_s)

    @property
    def wall_gap_ms(self) -> Fraction:
        return _ms(self.later.wall_s - self.earlier.wall_s)

    @property
    def excess_ms(self) -> Fraction:
        return self.step_ms - self.wall_gap_ms


def _ms(seconds: Fraction) -> Fraction:
    return seconds * Fraction(C.MS_PER_S)


def _optional(text: Optional[str]) -> Optional[Fraction]:
    return Fraction(text) if text else None


def _read_rows(folder: str, file_name: str, columns: Sequence[str],
               optional: Sequence[str] = ()) -> Tuple[List[dict], int]:
    """The whole rows of one file, and how many rows a terminated recorder left
    half written. Refuses a file that is missing or lacks a column."""
    path = os.path.join(folder, file_name)
    required = [c for c in columns if c not in optional]
    try:
        with open(path, newline="", encoding="utf-8", errors="replace") as f:
            reader = csv.DictReader(f)
            missing = [c for c in columns if c not in (reader.fieldnames or ())]
            if missing:
                raise Refused(f"refused: {path} has no column {', '.join(missing)}")
            rows = list(reader)
    except OSError as e:
        raise Refused(f"refused: could not read {path} ({e})")
    # A row cut short has None past the cut; an empty cell is whole when its column may be.
    whole = [row for row in rows if all(row[c] is not None for c in columns)
             and all(row[c] != "" for c in required)]
    return whole, len(rows) - len(whole)


def _parsed(file_name: str, rows: Iterable[dict], make) -> list:
    try:
        return [make(row) for row in rows]
    except ValueError as e:
        raise Refused(f"refused: {file_name} holds a row that cannot be read ({e})")


def _sample(row: dict) -> Sample:
    if row["clock_s"] and not (row["clock_wall_before_s"] and row["clock_wall_after_s"]):
        raise ValueError(f"sample {row['sample']} has a clock reading with no wall stamps")
    return Sample(int(row["sample"]), Fraction(row["wall_s"]), _optional(row["clock_s"]),
                  _optional(row["clock_wall_before_s"]), _optional(row["clock_wall_after_s"]),
                  int(row["recorder_pid"]), Fraction(row["period_ms"]),
                  row["stop_reason"] or "", row["process_time_s"] or "")


def _process(row: dict) -> ProcessRow:
    return ProcessRow(int(row["sample"]), int(row["pid"]), int(row["parent_pid"]), row["name"],
                      Fraction(row["cpu_ms"]), Fraction(row["read_bytes"]),
                      Fraction(row["write_bytes"]), Fraction(row["hard_faults"]))


def _thread(row: dict) -> ThreadRow:
    return ThreadRow(int(row["sample"]), int(row["pid"]), int(row["tid"]), row["reads_as"])


def read_recording(folder: str) -> Recording:
    sample_rows, half_samples = _read_rows(folder, SAMPLES_FILE, SAMPLE_COLUMNS, SAMPLE_OPTIONAL)
    process_rows, half_processes = _read_rows(folder, PROCESSES_FILE, PROCESS_COLUMNS,
                                              PROCESS_OPTIONAL)
    thread_rows, half_threads = _read_rows(folder, THREADS_FILE, THREAD_COLUMNS)
    return Recording(_parsed(SAMPLES_FILE, sample_rows, _sample),
                     _parsed(PROCESSES_FILE, process_rows, _process),
                     _parsed(THREADS_FILE, thread_rows, _thread),
                     half_samples + half_processes + half_threads)


def window_of(samples: Sequence[Sample], start: Optional[Fraction],
              end: Optional[Fraction]) -> Window:
    """The samples stamped from `start` to `end`, both included, and the slices
    they end. The first sample of a recording ends no slice."""
    inside = [s for s in samples
              if (start is None or s.wall_s >= start) and (end is None or s.wall_s <= end)]
    numbers = {s.number for s in inside}
    slices: Set[int] = set()
    span_ms = Fraction(0)
    for earlier, later in zip(samples, samples[1:]):
        if later.number in numbers:
            slices.add(later.number)
            span_ms += _ms(later.wall_s - earlier.wall_s)
    return Window(inside, slices, span_ms)


def _gained(rows: Iterable[ProcessRow], slices: Set[int],
            field: str) -> Dict[Tuple[int, str], Fraction]:
    """What each process, as (pid, name), gained of `field` over these slices."""
    totals: Dict[Tuple[int, str], Fraction] = defaultdict(Fraction)
    for row in rows:
        if row.sample in slices:
            totals[(row.pid, row.name)] += getattr(row, field)
    return totals


def _share(cpu_ms: Fraction, span_ms: Fraction) -> float:
    return float(cpu_ms / span_ms * 100)


def _wall(seconds: Fraction) -> str:
    return f"{float(seconds):.3f}"


def _settings_lines(documents_dir: str) -> List[str]:
    path = os.path.join(documents_dir, game_files.SETTINGS)
    try:
        with open(path, encoding="utf-8-sig", errors="replace") as f:
            lines = [line.strip() for line in f]
    except OSError as e:
        return [f"{game_files.SETTINGS}: could not read {path} ({e})"]
    out = []
    for key in SETTINGS_KEYS:
        found = [line for line in lines if line.split("=", 1)[0].strip().lower() == key
                 and "=" in line]
        out += [f"{game_files.SETTINGS}: {line}" for line in found] or [
            f"{game_files.SETTINGS}: no {key} line"]
    return out


def _recording_lines(rec: Recording, folder: str, whole: Window) -> List[str]:
    samples = rec.samples
    lines = [f"recording: {len(samples)} samples, from wall {_wall(samples[0].wall_s)} "
             f"to {_wall(samples[-1].wall_s)}"]
    last = samples[-1]
    if last.stop_reason:
        lines.append(f"recorder stopped: {last.stop_reason}, "
                     f"its own process time {last.process_time_s} s")
    else:
        lines.append("recorder stopped: not said, the last row of samples.csv is missing, "
                     "so the recorder was probably terminated")
    if rec.half_rows:
        lines.append(f"left out {rec.half_rows} half-written rows")

    gaps = [_ms(b.wall_s - a.wall_s) for a, b in zip(samples, samples[1:])]
    over = gap_summary([{"wall_s": s.wall_s, "clock_s": 0} for s in samples], GAP_MS).count
    lines.append(f"gaps between samples: median {float(statistics.median(gaps)):.1f} ms, "
                 f"worst {float(max(gaps)):.1f} ms, {over} over {GAP_MS} ms "
                 "(the recorder's own stalls)")

    pid = samples[0].recorder_pid
    cpu = sum(ms for key, ms in _gained(rec.processes, whole.slices, "cpu_ms").items()
              if key[0] == pid)
    wrote = sum(n for key, n in _gained(rec.processes, whole.slices, "write_bytes").items()
                if key[0] == pid)
    size = sum(os.path.getsize(os.path.join(folder, name))
               for name in (SAMPLES_FILE, PROCESSES_FILE, THREADS_FILE))
    lines.append(f"recorder cost: pid {pid}, {_share(cpu, whole.span_ms):.1f}% of one core "
                 "over the recording")
    lines.append(f"recorder cost: wrote {int(wrote)} bytes, its three files hold {size} bytes")
    return lines


def _worker_lines(rec: Recording, worker: int) -> Tuple[List[str], Optional[int]]:
    """The worker's parent and that parent's parent, and the front's pid when
    the table's parent pids reach it."""
    table: Dict[int, ProcessRow] = {}
    for row in rec.processes:
        table.setdefault(row.pid, row)
    if worker not in table:
        return [f"worker {worker}: no row in {PROCESSES_FILE}, so the front is not known"], None
    launcher = table[worker].parent_pid
    if launcher not in table:
        return [f"worker {worker}: its parent is pid {launcher}, which has no row in "
                f"{PROCESSES_FILE}, so the front is not known"], None
    front = table[launcher].parent_pid
    front_name = _shown(table[front].name) if front in table else NO_ROWS_NAME
    return [f"worker {worker}: its parent is pid {launcher} {_shown(table[launcher].name)}",
            f"front: pid {front} {front_name}, the parent of pid {launcher}"], front


def _stem(name: str) -> str:
    return os.path.splitext(name)[0].lower()


_REPORTED_LOWER = frozenset(name.lower() for name in quiet.REPORTED_NAMES)


def _always_listed(name: str) -> bool:
    """Whether a process name is one of ALWAYS_LISTED_NAMES, by quiet.py's own
    answers for a compiler and for a reported name."""
    return quiet.is_compiler(name) or name.lower() in _REPORTED_LOWER


def _shown(name: str) -> str:
    return name or NO_NAME


def _cpu_lines(rec: Recording, whole: Window, window: Window, windowed: bool,
               roles: Dict[int, List[str]]) -> List[str]:
    whole_ms = _gained(rec.processes, whole.slices, "cpu_ms")
    window_ms = _gained(rec.processes, window.slices, "cpu_ms")


    listed = {key for key, ms in whole_ms.items() if ms > 0}
    listed |= {(row.pid, row.name) for row in rec.processes
               if row.pid in roles or _always_listed(row.name)}
    listed |= {(pid, NO_ROWS_NAME) for pid in roles
               if not any(key[0] == pid for key in listed)}
    seen = {_stem(key[1]) for key in listed}
    # A name no process had: nothing to give a pid to.
    listed |= {(None, name) for name in ALWAYS_LISTED_NAMES if _stem(name) not in seen}

    shown_ms = window_ms if windowed else whole_ms
    order = sorted(listed, key=lambda key: (-shown_ms.get(key, 0), key[1].lower(), key[0] or 0))
    title = "CPU share of one core, whole recording"
    lines = [title + (" then the window:" if windowed else ":")]
    for key in order:
        pid, name = key
        shares = f"{_share(whole_ms.get(key, Fraction(0)), whole.span_ms):6.1f}%"
        if windowed:
            shares += f" {_share(window_ms.get(key, Fraction(0)), window.span_ms):6.1f}%"
        who = f"pid {pid} {_shown(name)}" if pid is not None else f"pid - {name}"
        label = f" ({' and '.join(roles[pid])})" if pid in roles else ""
        lines.append(f"  {shares}  {who}{label}")
    return lines


def ready_stretches(ready_samples: Iterable[int]) -> List[List[int]]:
    """The runs of consecutive sample numbers long enough to be a stretch."""
    runs: List[List[int]] = []
    for number in sorted(ready_samples):
        if runs and runs[-1][-1] + 1 == number:
            runs[-1].append(number)
        else:
            runs.append([number])
    return [run for run in runs if len(run) >= READY_STRETCH_MIN]


def _stretch_text(stretches: List[List[int]], walls: Dict[int, Fraction],
                  by_sample: Dict[int, List[ProcessRow]], slices: Set[int]) -> str:
    if not stretches:
        return "no ready stretch"
    longest = max(stretches, key=lambda run: walls[run[-1]] - walls[run[0]])
    rows = [row for number in longest for row in by_sample.get(number, ())]
    gained = _gained(rows, slices, "cpu_ms")
    who = [f"pid {pid} {_shown(name)} {float(ms):.1f} ms"
           for (pid, name), ms in sorted(gained.items(), key=lambda kv: (-kv[1], kv[0]))
           if ms > 0]
    return (f"longest ready stretch {float(_ms(walls[longest[-1]] - walls[longest[0]])):.1f} ms "
            f"from wall {_wall(walls[longest[0]])}, CPU gained in those samples: "
            f"{', '.join(who) or 'by nobody'}")


def _thread_lines(rec: Recording, whole: Window, window: Window, pids: Sequence[int],
                  roles: Dict[int, List[str]]) -> List[str]:
    numbers = {s.number for s in window.samples}
    walls = {s.number: s.wall_s for s in rec.samples}
    by_sample: Dict[int, List[ProcessRow]] = defaultdict(list)
    for row in rec.processes:
        by_sample[row.sample].append(row)
    lines = [f"threads in the window, out of {len(numbers)} samples:"]
    for pid in pids:
        label = " and ".join(roles[pid])
        rows = [row for row in rec.threads if row.pid == pid and row.sample in numbers]
        if not rows:
            lines.append(f"  {label} pid {pid}: no thread rows in the window")
        for tid in sorted({row.tid for row in rows}):
            mine = [row for row in rows if row.tid == tid]
            running = sum(row.reads_as == READS_RUNNING for row in mine)
            ready = [row.sample for row in mine if row.reads_as == READS_READY]
            stretch = _stretch_text(ready_stretches(ready), walls, by_sample, whole.slices)
            lines.append(f"  {label} pid {pid} thread {tid}: running {running}, ready "
                         f"{len(ready)}, {stretch}")
    return lines


def _read_too_long(sample: Sample, period_ms: Fraction) -> bool:
    """Whether the guard at the top of the plan's T6 section drops this sample."""
    return _ms(sample.clock_wall_after_s - sample.clock_wall_before_s) > period_ms


def clock_pairs(samples: Sequence[Sample],
                period_ms: Fraction) -> Tuple[List[ClockPair], List[Sample]]:
    """The pairs the clock section reads, and the samples the guard dropped. A
    dropped sample is out of the section, so its neighbours pair up; a sample
    with no clock ends the run, since no reading joins the two sides."""
    pairs: List[ClockPair] = []
    dropped: List[Sample] = []
    earlier: Optional[Sample] = None
    for sample in samples:
        if sample.clock_s is None:
            earlier = None
        elif _read_too_long(sample, period_ms):
            dropped.append(sample)
        else:
            if earlier is not None:
                pairs.append(ClockPair(earlier, sample))
            earlier = sample
    return pairs, dropped


def _lead_line(kept: Sequence[Sample]) -> str:
    """The clock's lead over wall time against its median offset, the way the
    plan's row P36 measured it."""
    offsets = [(_ms(s.clock_s - s.wall_s), s) for s in kept]
    middle = statistics.median(offset for offset, _ in offsets)
    high, high_at = max(offsets, key=lambda item: item[0])
    low, low_at = min(offsets, key=lambda item: item[0])
    return (f"clock lead over wall time, against its median offset: "
            f"largest {float(high - middle):+.1f} ms at wall {_wall(high_at.wall_s)}, "
            f"lowest {float(low - middle):+.1f} ms at wall {_wall(low_at.wall_s)}")


def _clock_lines(rec: Recording, window: Window) -> List[str]:
    if all(s.clock_s is None for s in rec.samples):
        return ["no clock in this recording"]
    numbers = {s.number for s in window.samples}
    period = rec.period_ms
    all_pairs, all_dropped = clock_pairs(rec.samples, period)
    dropped = [s for s in all_dropped if s.number in numbers]
    gone = {s.number for s in all_dropped}
    kept = [s for s in window.samples if s.clock_s is not None and s.number not in gone]
    pairs = [pair for pair in all_pairs if pair.later.number in numbers]

    lines = [f"clock reads dropped, their two stamps over one period ({float(period):g} ms) "
             f"apart: {len(dropped)}"]
    if not kept:
        return lines + ["no clock readings left in the window"]
    lines.append(_lead_line(kept))
    if not pairs:
        return lines + ["clock steps: none in the window"]

    steps = [pair.step_ms for pair in pairs]
    percentile = statistics.quantiles(steps, n=100, method="inclusive")[STEP_PERCENTILE - 1]
    lines.append(f"clock steps: count {len(steps)}, median {float(statistics.median(steps)):.1f} "
                 f"ms, {STEP_PERCENTILE}th percentile {float(percentile):.1f} ms")

    limit = LONG_STEP_FACTOR * period
    long_pairs = [pair for pair in pairs if abs(pair.excess_ms) > limit]
    lines.append(f"clock steps with an excess over {float(limit):g} ms by size: {len(long_pairs)}")
    if long_pairs:
        lines.append("largest of those, as detail (one step's excess can be the measurement; "
                     "the lead above judges the game):")
    else:
        lines.append("no clock step has an excess that long, so none is listed")
    for pair in sorted(long_pairs, key=lambda p: -abs(p.excess_ms))[:LONGEST_LISTED]:
        lines.append(f"  wall {_wall(pair.later.wall_s)}: step {float(pair.step_ms):.1f} ms, "
                     f"wall gap {float(pair.wall_gap_ms):.1f} ms, "
                     f"excess {float(pair.excess_ms):+.1f} ms")
    return lines


def _nonzero_by_process(gains: Dict[Tuple[int, str], Fraction]) -> List[Tuple[int, str]]:
    return sorted((key for key, n in gains.items() if n > 0), key=lambda key: (-gains[key], key))


def _bytes_lines(rec: Recording, window: Window) -> List[str]:
    read = _gained(rec.processes, window.slices, "read_bytes")
    wrote = _gained(rec.processes, window.slices, "write_bytes")
    both = {key: read.get(key, 0) + wrote.get(key, 0) for key in set(read) | set(wrote)}
    lines = ["bytes by process in the window:"]
    for key in _nonzero_by_process(both):
        lines.append(f"  pid {key[0]} {_shown(key[1])}: read {int(read.get(key, 0))}, "
                     f"wrote {int(wrote.get(key, 0))}")
    return lines if len(lines) > 1 else lines + ["  none"]


def _fault_lines(rec: Recording, window: Window) -> List[str]:
    faults = _gained(rec.processes, window.slices, "hard_faults")
    # Row P44 of the plan: this column's field is not confirmed yet.
    lines = ["hard faults by process in the window (meaning unchecked):"]
    for key in _nonzero_by_process(faults):
        lines.append(f"  pid {key[0]} {_shown(key[1])}: {int(faults[key])}")
    return lines if len(lines) > 1 else lines + ["  none"]


def report_lines(folder: str, pid: int, worker: Optional[int] = None,
                 start: Optional[Fraction] = None, end: Optional[Fraction] = None,
                 documents_dir: str = game_files.DOCUMENTS_DIR) -> List[str]:
    """The report, line by line. Raises Refused when it cannot go on."""
    rec = read_recording(folder)
    thread_samples = sum(row.pid == pid for row in rec.threads)
    first = f"the recording holds {thread_samples} thread samples for pid {pid}"
    if not thread_samples:
        raise Refused(f"{first}\nrefused: no thread rows for pid {pid}, "
                      "so this recording is not of that process")
    if len(rec.samples) < 2:
        raise Refused(f"{first}\nrefused: {SAMPLES_FILE} holds {len(rec.samples)} whole samples, "
                      "and a slice needs two")

    whole = window_of(rec.samples, None, None)
    window = window_of(rec.samples, start, end)
    if not window.slices:
        raise Refused(f"{first}\nrefused: no slice ends in that window; the recording runs from "
                      f"wall {_wall(rec.samples[0].wall_s)} to {_wall(rec.samples[-1].wall_s)}")
    windowed = start is not None or end is not None

    lines = [first] + _settings_lines(documents_dir)
    lines += _recording_lines(rec, folder, whole)

    roles: Dict[int, List[str]] = {}
    roles.setdefault(pid, []).append("game")
    if worker is not None:
        roles.setdefault(worker, []).append("worker")
        worker_lines, front = _worker_lines(rec, worker)
        lines += worker_lines
        if front is not None:
            roles.setdefault(front, []).append("front")
    recorder = rec.samples[0].recorder_pid
    roles.setdefault(recorder, []).append("recorder")

    if windowed:
        lines.append(f"window: wall {_wall(window.samples[0].wall_s)} to "
                     f"{_wall(window.samples[-1].wall_s)}, {float(window.span_ms):.1f} ms of slices")
    else:
        lines.append("window: the whole recording")
    lines += _cpu_lines(rec, whole, window, windowed, roles)
    thread_pids = list(dict.fromkeys(p for p in (pid, worker, recorder) if p is not None))
    lines += _thread_lines(rec, whole, window, thread_pids, roles)
    lines += _clock_lines(rec, window)
    lines += _bytes_lines(rec, window)
    lines += _fault_lines(rec, window)
    return lines


def _wall_time(text: str) -> Fraction:
    try:
        return Fraction(text)
    except ValueError:
        raise argparse.ArgumentTypeError(f"{text!r} is not a wall time in seconds")


def main(argv: Optional[List[str]] = None,
         documents_dir: str = game_files.DOCUMENTS_DIR) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("folder", help="the recorder's folder")
    ap.add_argument("pid", type=int, help="the game's pid")
    ap.add_argument("--worker", type=int,
                    help="the worker's pid; names its parent and that parent's parent, the front")
    ap.add_argument("--from", dest="start", type=_wall_time, help="the window's first wall time")
    ap.add_argument("--to", dest="end", type=_wall_time, help="the window's last wall time")
    args = ap.parse_args(argv)
    try:
        lines = report_lines(args.folder, args.pid, args.worker, args.start, args.end,
                             documents_dir)
    except Refused as e:
        print(e)
        return 1
    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    sys.exit(main())
