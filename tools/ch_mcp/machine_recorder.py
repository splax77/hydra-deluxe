"""The machine recorder: a small process of its own that runs beside the game and
writes what the whole machine was doing, so a stall in the worker has a witness
outside the worker.

It reads the process table the way Task Manager does and the game's song clock
the way the Star Power watch does. It needs no administrator rights, starts no
trace and changes nothing on the machine. It writes only under --out.

The worker starts it and tells it two things on its stdin: the `pid <n>` line and
the `engine <hex>` line. Nothing else crosses. It stops on the first of four ends
(the STOP_ names below), and each end leaves its name on the last row of
samples.csv. A run by hand gives --target or --seconds and stdin is left unread,
because a shell's stdin is often closed from the start.

The design is point 2 of docs/superpowers/plans/2026-11-07-ch-run-timing.md, and
tools/ch_mcp/machine_report.py reads the three files.
"""

from __future__ import annotations

import argparse
import csv
import ctypes
import dataclasses
import os
import struct
import sys
import threading
import time
from typing import Any, Callable, Dict, FrozenSet, Iterable, List, NamedTuple, Optional, Tuple

if __package__ in (None, ""):
    # Run as a script (-I leaves the repo off sys.path).
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import process as P  # noqa: E402

# Question 12 of the plan: samples a second.
RATE_PER_S = 50
# Question 18 of the plan: how long past the run's cap the recorder may live.
RECORDER_CAP_EXTRA_S = 60

# The three files, with the columns machine_report.py and recorder_link.py rely on.
SAMPLES_FILE = "samples.csv"
PROCESSES_FILE = "processes.csv"
THREADS_FILE = "threads.csv"
SAMPLES_COLUMNS = ("sample", "wall_s", "sample_ms", "processes", "threads", "ready_threads",
                   "clock_s", "clock_wall_before_s", "clock_wall_after_s", "recorder_pid",
                   "period_ms", "stop_reason", "process_time_s")
PROCESSES_COLUMNS = ("sample", "pid", "parent_pid", "name", "cpu_ms", "read_bytes", "write_bytes",
                     "hard_faults")
THREADS_COLUMNS = ("sample", "pid", "tid", "state", "wait_reason", "reads_as", "switches", "cpu_ms")

STOP_STDIN = "stdin closed"
STOP_PARENT = "parent gone"
STOP_CAP = "cap"
STOP_SECONDS = "seconds"

# The process table: NtQuerySystemInformation, information class 5, on 64-bit
# Windows (row P38 of the plan; LS-T1 proves it on this machine).
SYSTEM_PROCESS_INFORMATION_CLASS = 5
# geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/process.htm
PROCESS_ENTRY_SIZE = 0x100
PROC_NEXT_ENTRY_OFFSET = 0x00
PROC_NUMBER_OF_THREADS = 0x04
# The page gives this one for 32-bit only, and what the count means is unproven
# (row P44). No member before it is pointer-sized, so the plan takes the same 0x10.
PROC_HARD_FAULT_COUNT = 0x10
PROC_USER_TIME = 0x28
PROC_KERNEL_TIME = 0x30
PROC_IMAGE_NAME_LENGTH = 0x38  # ImageName, a UNICODE_STRING: its Length, in bytes
PROC_IMAGE_NAME_BUFFER = 0x40  # and its Buffer, past the structure's padding
PROC_UNIQUE_PROCESS_ID = 0x50
PROC_INHERITED_FROM_PID = 0x58
PROC_READ_TRANSFER_COUNT = 0xE8
PROC_WRITE_TRANSFER_COUNT = 0xF0
# geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/thread.htm
THREAD_ENTRY_SIZE = 0x50
THREAD_KERNEL_TIME = 0x00
THREAD_USER_TIME = 0x08
THREAD_UNIQUE_THREAD = 0x30  # ClientId is at 0x28; the thread id is CLIENT_ID's second member
THREAD_CONTEXT_SWITCHES = 0x40
THREAD_STATE = 0x44
THREAD_WAIT_REASON = 0x48

# The table's times count in units of 100 ns.
TICKS_PER_MS = 10_000

# The one owner of what a thread state number means (row P39). 3 (standby) and 7
# (deferred ready) also wait for a CPU, by the answer to questions 12 to 19.
RUNNING = "running"
READY = "ready"
WAITING = "waiting"
OTHER = "other"
STATE_READS_AS = {2: RUNNING, 1: READY, 3: READY, 7: READY, 5: WAITING}
READY_STATES = tuple(state for state, word in STATE_READS_AS.items() if word == READY)


def reads_as(state: int) -> str:
    """The word a thread state number reads as."""
    return STATE_READS_AS.get(state, OTHER)


def _layout(fields: Iterable[Tuple[int, str]]) -> struct.Struct:
    """One Struct reading (offset, format code) fields, in offset order."""
    text, at = "<", 0
    for offset, code in sorted(fields):
        text += "x" * (offset - at) + code
        at = offset + struct.calcsize("<" + code)
    return struct.Struct(text)


_PROCESS_FIELDS = _layout((
    (PROC_NEXT_ENTRY_OFFSET, "I"), (PROC_NUMBER_OF_THREADS, "I"), (PROC_HARD_FAULT_COUNT, "I"),
    (PROC_USER_TIME, "q"), (PROC_KERNEL_TIME, "q"), (PROC_IMAGE_NAME_LENGTH, "H"),
    (PROC_IMAGE_NAME_BUFFER, "Q"), (PROC_UNIQUE_PROCESS_ID, "Q"), (PROC_INHERITED_FROM_PID, "Q"),
    (PROC_READ_TRANSFER_COUNT, "q"), (PROC_WRITE_TRANSFER_COUNT, "q")))
_THREAD_FIELDS = _layout((
    (THREAD_KERNEL_TIME, "q"), (THREAD_USER_TIME, "q"), (THREAD_UNIQUE_THREAD, "Q"),
    (THREAD_CONTEXT_SWITCHES, "I"), (THREAD_STATE, "I"), (THREAD_WAIT_REASON, "I")))


class Thread(NamedTuple):
    tid: int
    state: int
    wait_reason: int
    switches: int
    cpu_ticks: int


class Process(NamedTuple):
    pid: int
    parent_pid: int
    name: str
    cpu_ticks: int
    read_bytes: int
    write_bytes: int
    hard_faults: int
    threads: Dict[int, Thread]


class Table(NamedTuple):
    processes: Dict[int, Process]
    thread_count: int
    ready_threads: int


class ThreadGain(NamedTuple):
    switches: int
    cpu_ms: float


class ProcessGain(NamedTuple):
    cpu_ms: float
    read_bytes: int
    write_bytes: int
    hard_faults: int
    threads: Dict[int, ThreadGain]


def parse_table(buffer: bytes, base: int = 0, thread_pids: Optional[Iterable[int]] = None) -> Table:
    """The processes and threads in one table. `base` is the address the table
    was read at, which the entries' name pointers count from. Only the pids in
    `thread_pids` get their threads parsed (None means all of them), because the
    whole table's threads at every sample would cost more than the recorder's
    budget (question 15); the thread and ready counts always cover the table."""
    wanted = None if thread_pids is None else frozenset(thread_pids)
    processes: Dict[int, Process] = {}
    thread_count = ready = 0
    at = 0
    while True:
        (next_entry, threads, hard_faults, user, kernel, name_length, name_pointer, pid, parent,
         read_bytes, write_bytes) = _PROCESS_FIELDS.unpack_from(buffer, at)
        first = at + PROCESS_ENTRY_SIZE
        end = first + threads * THREAD_ENTRY_SIZE
        thread_count += threads
        # A slice with a step reads every thread's state byte without a Python
        # loop. States are small, so the low byte of the ULONG is the whole number.
        states = buffer[first + THREAD_STATE:end:THREAD_ENTRY_SIZE]
        ready += sum(states.count(state) for state in READY_STATES)
        name_at = name_pointer - base
        name = buffer[name_at:name_at + name_length].decode("utf-16-le", "replace")
        processes[pid] = Process(
            pid, parent, name, user + kernel, read_bytes, write_bytes, hard_faults,
            _parse_threads(buffer, first, threads) if wanted is None or pid in wanted else {})
        if next_entry == 0:
            return Table(processes, thread_count, ready)
        at += next_entry


def _parse_threads(buffer: bytes, first: int, count: int) -> Dict[int, Thread]:
    threads = {}
    for at in range(first, first + count * THREAD_ENTRY_SIZE, THREAD_ENTRY_SIZE):
        kernel, user, tid, switches, state, wait_reason = _THREAD_FIELDS.unpack_from(buffer, at)
        threads[tid] = Thread(tid, state, wait_reason, switches, user + kernel)
    return threads


def _gain(before: int, after: int) -> int:
    # A pid or tid Windows reused can read lower than its last owner did.
    return max(after - before, 0)


def deltas(previous: Dict[int, Process], current: Dict[int, Process]) -> Dict[int, ProcessGain]:
    """What each process in `current` gained since `previous`. A process or
    thread first seen gains 0, and one that vanished has no entry."""
    gains = {}
    for pid, now in current.items():
        then = previous.get(pid)
        if then is None or then.name != now.name:
            then = now
        threads = {}
        for tid, thread in now.threads.items():
            before = then.threads.get(tid, thread)
            threads[tid] = ThreadGain(_gain(before.switches, thread.switches),
                                      _gain(before.cpu_ticks, thread.cpu_ticks) / TICKS_PER_MS)
        gains[pid] = ProcessGain(_gain(then.cpu_ticks, now.cpu_ticks) / TICKS_PER_MS,
                                 _gain(then.read_bytes, now.read_bytes),
                                 _gain(then.write_bytes, now.write_bytes),
                                 _gain(then.hard_faults, now.hard_faults), threads)
    return gains


@dataclasses.dataclass
class SampleRow:
    """One row of samples.csv; its fields are SAMPLES_COLUMNS. None is an empty cell."""
    sample: int
    wall_s: float
    sample_ms: float
    processes: int
    threads: int
    ready_threads: int
    clock_s: Optional[float]
    clock_wall_before_s: Optional[float]
    clock_wall_after_s: Optional[float]
    recorder_pid: int
    period_ms: float
    stop_reason: Optional[str] = None
    process_time_s: Optional[float] = None


class RecorderFiles:
    """The three CSV files, each with its header on disk from the start. Opening
    refuses a folder that already holds a recording, so none is overwritten."""

    def __init__(self, folder: str, open_file: Callable[..., Any] = open):
        self._files = []
        self._writers = []
        for name, columns in ((PROCESSES_FILE, PROCESSES_COLUMNS), (THREADS_FILE, THREADS_COLUMNS),
                              (SAMPLES_FILE, SAMPLES_COLUMNS)):
            file = open_file(os.path.join(folder, name), "x", newline="", encoding="utf-8")
            self._files.append(file)
            self._writers.append(csv.writer(file, lineterminator="\n"))
            self._writers[-1].writerow(columns)
        self.processes, self.threads, self.samples = self._writers
        self.flush()

    def flush(self) -> None:
        """samples.csv goes last, so a sample it shows has its rows on disk."""
        for file in self._files:
            file.flush()

    def close(self) -> None:
        for file in self._files:
            file.close()


def write_sample(files: RecorderFiles, row: SampleRow, table: Table,
                 gains: Dict[int, ProcessGain], listed: FrozenSet[int]) -> None:
    """Write one sample's rows and flush them together. A process gets a row when
    it gained something or is in `listed`; a thread gets one when it is not
    waiting or its context switches moved."""
    for pid, gain in gains.items():
        process = table.processes[pid]
        if pid in listed or gain.cpu_ms or gain.read_bytes or gain.write_bytes or gain.hard_faults:
            files.processes.writerow((row.sample, pid, process.parent_pid, process.name, gain.cpu_ms,
                                      gain.read_bytes, gain.write_bytes, gain.hard_faults))
        for tid, thread_gain in gain.threads.items():
            thread = process.threads[tid]
            word = reads_as(thread.state)
            if word != WAITING or thread_gain.switches:
                files.threads.writerow((row.sample, pid, tid, thread.state, thread.wait_reason, word,
                                        thread_gain.switches, thread_gain.cpu_ms))
    files.samples.writerow(dataclasses.astuple(row))
    files.flush()


class Stop:
    """Why the recorder is stopping. The first end to arrive is the one named."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self.reason: Optional[str] = None

    def set(self, reason: str) -> None:
        with self._lock:
            if self.reason is None:
                self.reason = reason


class Inputs:
    """What the stdin lines have said so far. Until `target` is known no thread
    rows are written; until `engine` is known no clock is read."""

    def __init__(self, target: Optional[int] = None):
        self.target = target
        self.engine: Optional[int] = None

    def apply(self, line: str) -> None:
        """Take in one stdin line. A line that is neither form is reported, not
        fatal: a bad line must never cost a run its recording."""
        parts = line.split()
        try:
            if len(parts) == 2 and parts[0] == "pid":
                self.target = int(parts[1])
                return
            if len(parts) == 2 and parts[0] == "engine":
                self.engine = int(parts[1], 16)
                return
        except ValueError:
            pass
        print(f"machine recorder: ignored stdin line {line!r}", file=sys.stderr, flush=True)


def read_stdin(lines: Iterable[Any], inputs: Inputs, stop: Stop) -> None:
    """Apply each stdin line until the input ends, which means stop."""
    try:
        for line in lines:
            inputs.apply(line.decode("utf-8", "replace") if isinstance(line, bytes) else line)
    except (OSError, ValueError):
        pass  # A broken pipe says the same as a closed one.
    stop.set(STOP_STDIN)


def watch_parent(wait: Callable[[], None], stop: Stop) -> None:
    """Block on the parent's handle; its return means the parent is gone."""
    wait()
    stop.set(STOP_PARENT)


class ClockReader:
    """Reads the game's song clock. `open_reader` turns a pid into a reader of
    (address, size) bytes. A failed read is None and the next one tries again."""

    def __init__(self, open_reader: Callable[[int], Callable[[int, int], bytes]]):
        self._open_reader = open_reader
        self._reader: Optional[Callable[[int, int], bytes]] = None
        self._pid: Optional[int] = None

    def read(self, pid: int, engine: int) -> Optional[float]:
        try:
            if self._reader is None or self._pid != pid:
                self._reader = self._open_reader(pid)
                self._pid = pid
            data = self._reader(engine + C.OFF_SONG_CLOCK, struct.calcsize("<d"))
            return struct.unpack("<d", data)[0]
        except (OSError, struct.error):
            return None


@dataclasses.dataclass
class Seams:
    """The outside world of the sample loop; a test swaps any of these."""
    read_table: Callable[[], Tuple[bytes, int]]
    open_reader: Callable[[int], Callable[[int, int], bytes]]
    wall: Callable[[], float] = time.time
    monotonic: Callable[[], float] = time.perf_counter
    # time.sleep, not Event.wait: on Windows only the sleep wakes on time at this
    # rate, and a stop is seen at the next tick, which is within one period.
    sleep: Callable[[float], None] = time.sleep
    process_time: Callable[[], float] = time.process_time


def record(files: RecorderFiles, inputs: Inputs, stop: Stop, seams: Seams, *, period_s: float,
           parent_pid: int, recorder_pid: int, cap_s: Optional[float] = None,
           seconds: Optional[float] = None) -> str:
    """Take samples until an end arrives, and return its name. The sample that
    sees the end is the last row, and it carries the end's name."""
    clock = ClockReader(seams.open_reader)
    limits = sorted((limit, name) for limit, name in ((cap_s, STOP_CAP), (seconds, STOP_SECONDS))
                    if limit is not None)
    start = next_tick = seams.monotonic()
    previous: Dict[int, Process] = {}
    number = 0
    while True:
        began = seams.monotonic()
        reason = stop.reason or next((name for limit, name in limits if began - start >= limit),
                                     None)
        number += 1
        wall_s = seams.wall()
        target, engine = inputs.target, inputs.engine
        clock_s = before = after = None
        if target is not None and engine is not None:
            # First in the sample, so the reading sits next to its wall stamps.
            read_before = seams.wall()
            clock_s = clock.read(target, engine)
            if clock_s is not None:
                before, after = read_before, seams.wall()
        buffer, base = seams.read_table()
        listed = frozenset(pid for pid in (target, parent_pid, recorder_pid) if pid is not None)
        table = parse_table(buffer, base, listed if target is not None else ())
        gains = deltas(previous, table.processes)
        previous = table.processes
        # The row holds its own duration, so the file write itself is not in it.
        row = SampleRow(number, wall_s, (seams.monotonic() - began) * 1000, len(table.processes),
                        table.thread_count, table.ready_threads, clock_s, before, after,
                        recorder_pid, period_s * 1000)
        if reason is not None:
            row.stop_reason = reason
            row.process_time_s = seams.process_time()
        write_sample(files, row, table, gains, listed)
        if reason is not None:
            return reason
        # The ticks hold their own pace, and a late sample skips ticks, not bursts.
        next_tick = max(next_tick + period_s, seams.monotonic())
        seams.sleep(max(next_tick - seams.monotonic(), 0))


_STATUS_INFO_LENGTH_MISMATCH = 0xC0000004
_ULONG_MAX = 0xFFFFFFFF


class TableBuffer:
    """The memory the table is read into, kept between samples."""

    def __init__(self) -> None:
        self.raw = ctypes.create_string_buffer(0)


_TABLE_BUFFER = TableBuffer()


def _nt_query() -> Callable[..., int]:
    """NtQuerySystemInformation. LIVE-ONLY."""
    query = ctypes.WinDLL("ntdll").NtQuerySystemInformation
    query.restype = ctypes.c_ulong
    query.argtypes = (ctypes.c_ulong, ctypes.c_void_p, ctypes.c_ulong,
                      ctypes.POINTER(ctypes.c_ulong))
    return query


_QUERY: List[Callable[..., int]] = []


def read_table(held: TableBuffer = _TABLE_BUFFER,
               query: Optional[Callable[..., int]] = None) -> Tuple[bytes, int]:
    """One whole process table and the address it was read at. LIVE-ONLY unless a
    fake `query` is given. A table that outgrew the buffer grows it and asks
    again; the size at least doubles, so the ULONG limit caps the tries."""
    if query is None:
        if not _QUERY:
            _QUERY.append(_nt_query())
        query = _QUERY[0]
    while True:
        size = len(held.raw)
        needed = ctypes.c_ulong(0)
        status = query(SYSTEM_PROCESS_INFORMATION_CLASS, ctypes.addressof(held.raw), size,
                       ctypes.byref(needed))
        if status == 0:
            return ctypes.string_at(held.raw, needed.value), ctypes.addressof(held.raw)
        if status != _STATUS_INFO_LENGTH_MISMATCH:
            raise OSError(f"NtQuerySystemInformation failed with status {status:#x}")
        grown = max(needed.value, size * 2, 1)
        if grown > _ULONG_MAX:
            raise OSError(f"the process table needs {grown} bytes, more than one call can return")
        held.raw = ctypes.create_string_buffer(grown)


def open_game_reader(pid: int) -> Callable[[int, int], bytes]:
    """A reader of the game's memory, with the rights the watch opens it with.
    LIVE-ONLY. open_process finds the game by name, so it does not fit here."""
    handle = _kernel32().OpenProcess(P.access_mask(read_only=True), False, pid)
    if not handle:
        raise OSError(ctypes.get_last_error(), f"OpenProcess failed for pid {pid}")
    return P.make_reader(handle)


_SYNCHRONIZE = 0x00100000
_INFINITE = 0xFFFFFFFF


def _kernel32() -> Any:
    """kernel32 with its handle types declared. LIVE-ONLY. This is a WinDLL of its
    own, so these declarations reach no other module's calls."""
    from ctypes import wintypes  # noqa: PLC0415  (Windows only)
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    k32.OpenProcess.restype = wintypes.HANDLE
    k32.OpenProcess.argtypes = (wintypes.DWORD, wintypes.BOOL, wintypes.DWORD)
    k32.WaitForSingleObject.restype = wintypes.DWORD
    k32.WaitForSingleObject.argtypes = (wintypes.HANDLE, wintypes.DWORD)
    return k32


def open_parent_wait(pid: int) -> Callable[[], None]:
    """Open a SYNCHRONIZE handle on the parent now, and return the call that
    blocks until the parent ends. LIVE-ONLY. The cap of this wait is the
    recorder's own: the thread is a daemon and ends with the process."""
    k32 = _kernel32()
    handle = k32.OpenProcess(_SYNCHRONIZE, False, pid)
    if not handle:
        raise OSError(ctypes.get_last_error(), f"OpenProcess failed for parent pid {pid}")

    def wait() -> None:
        k32.WaitForSingleObject(handle, _INFINITE)

    return wait


def _positive(text: str) -> float:
    value = float(text)
    if not value > 0:
        raise argparse.ArgumentTypeError(f"{text} is not above 0")
    return value


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Record the machine beside a Clone Hero run.")
    parser.add_argument("--out", required=True, help="the folder for the three files")
    parser.add_argument("--parent", required=True, type=int,
                        help="the pid whose end stops the recorder")
    parser.add_argument("--rate", type=_positive, default=RATE_PER_S, help="samples a second")
    caps = parser.add_mutually_exclusive_group()
    caps.add_argument("--cap-s", type=_positive, help="stop after exactly this many seconds")
    caps.add_argument("--run-cap-s", type=_positive,
                      help="the run's cap; the recorder adds RECORDER_CAP_EXTRA_S to it")
    parser.add_argument("--target", type=int, help="a run by hand: the pid the pid line would give")
    parser.add_argument("--seconds", type=_positive, help="a run by hand: how long to record")
    return parser


def main(argv: Optional[List[str]] = None) -> int:
    parser = _parser()
    args = parser.parse_args(argv)
    cap_s = args.cap_s
    if args.run_cap_s is not None:
        cap_s = args.run_cap_s + RECORDER_CAP_EXTRA_S
    if cap_s is None and args.seconds is None:
        parser.error("give --cap-s, --run-cap-s or --seconds: a recorder never runs without an end")

    stop = Stop()
    inputs = Inputs(args.target)
    parent_wait = open_parent_wait(args.parent)
    os.makedirs(args.out, exist_ok=True)
    files = RecorderFiles(args.out)
    # The sampling thread is named so a check can find its rows in threads.csv.
    print(f"machine recorder: pid {os.getpid()}, sampling thread {threading.get_native_id()}, "
          f"folder {args.out}", flush=True)
    threading.Thread(target=watch_parent, args=(parent_wait, stop), daemon=True).start()
    if args.target is None and args.seconds is None:
        # Unbuffered, because a buffered stdin's lock can abort the interpreter's
        # exit while this daemon thread is blocked in a read.
        stdin = os.fdopen(sys.stdin.fileno(), "rb", buffering=0, closefd=False)
        threading.Thread(target=read_stdin, args=(stdin, inputs, stop), daemon=True).start()
    try:
        reason = record(files, inputs, stop, Seams(read_table, open_game_reader),
                        period_s=1 / args.rate, parent_pid=args.parent, recorder_pid=os.getpid(),
                        cap_s=cap_s, seconds=args.seconds)
    finally:
        files.close()
    print(f"machine recorder: stopped, {reason}", flush=True)
    return 0


if __name__ == "__main__":
    sys.exit(main())
