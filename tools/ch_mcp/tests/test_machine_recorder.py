"""The machine recorder: the table parser on one table captured from Windows, the
gains, the files, the loop on fakes, and short real runs of the module as its own
process."""

from __future__ import annotations

import csv
import ctypes
import dataclasses
import math
import mmap
import os
import struct
import subprocess
import sys
import tempfile
import threading
import time
import typing
import unittest
import unittest.mock

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

from tools.ch_mcp import machine_recorder as M  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import process as P  # noqa: E402

SCRIPT = os.path.join(ROOT, "tools", "ch_mcp", "machine_recorder.py")
PERIOD_S = 1 / 64  # exact in binary, so the fake clock adds up without rounding
PARENT = 900
RECORDER = 901
SUBPROCESS_CAP_S = 30


# Windows' 64-bit layout of one process entry and one thread entry, written here
# as numbers from geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/
# (process.htm and thread.htm), never taken from the parser: each field is
# (offset, struct format code). CapturedTable checks the parser against them and
# against bytes Windows wrote.
PROCESS_SIZE = 0x100
PROCESS_LAYOUT = {
    "next": (0x00, "I"), "threads": (0x04, "I"), "faults": (0x10, "I"), "user": (0x28, "q"),
    "kernel": (0x30, "q"), "name_length": (0x38, "H"), "name_buffer": (0x40, "Q"),
    "pid": (0x50, "Q"), "parent": (0x58, "Q"), "read": (0xE8, "q"), "write": (0xF0, "q"),
}
THREAD_SIZE = 0x50
THREAD_LAYOUT = {
    "kernel": (0x00, "q"), "user": (0x08, "q"), "tid": (0x30, "Q"), "switches": (0x40, "I"),
    "state": (0x44, "I"), "wait_reason": (0x48, "I"),
}


def fake_table(processes, base=0):
    """Table bytes built from the literal layout above, for the tests of the gains,
    the files and the loop. Each process is a dict of pid, parent, name, user,
    kernel, read, write, faults and threads; each thread is (tid, kernel, user,
    switches, state, wait_reason). No test of the parser's offsets reads it."""
    def put(entry, layout, field, at, value):
        offset, code = layout[field]
        struct.pack_into("<" + code, entry, at + offset, value)

    chunks = []
    at = 0
    sizes = [PROCESS_SIZE + len(process["threads"]) * THREAD_SIZE
             + len(process["name"].encode("utf-16-le")) for process in processes]
    for index, process in enumerate(processes):
        name = process["name"].encode("utf-16-le")
        threads = process["threads"]
        name_at = PROCESS_SIZE + len(threads) * THREAD_SIZE
        entry = bytearray(sizes[index])
        last = index == len(processes) - 1
        for field, value in (("next", 0 if last else sizes[index]), ("threads", len(threads)),
                             ("faults", process.get("faults", 0)), ("user", process.get("user", 0)),
                             ("kernel", process.get("kernel", 0)), ("name_length", len(name)),
                             ("name_buffer", base + at + name_at), ("pid", process["pid"]),
                             ("parent", process.get("parent", 0)), ("read", process.get("read", 0)),
                             ("write", process.get("write", 0))):
            put(entry, PROCESS_LAYOUT, field, 0, value)
        for number, (tid, kernel, user, switches, state, reason) in enumerate(threads):
            first = PROCESS_SIZE + number * THREAD_SIZE
            for field, value in (("kernel", kernel), ("user", user), ("tid", tid),
                                 ("switches", switches), ("state", state),
                                 ("wait_reason", reason)):
                put(entry, THREAD_LAYOUT, field, first, value)
        entry[name_at:] = name
        chunks.append(bytes(entry))
        at += sizes[index]
    return b"".join(chunks)


# One real table: NtQuerySystemInformation class 5, read on this machine with
# machine_recorder.read_table on 2026-11-08 and trimmed to its first two entries
# (the second entry's NextEntryOffset set to 0, no other byte changed). The
# offsets below were read from that file by hand, never from the parser.
SAMPLE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "recorder_sample.bin")
SYSTEM_AT = 4464  # where the second entry, System, starts
SYSTEM_NAME_AT = 41728  # where System's name text starts in the sample
# System's threads: (index, byte where the entry starts, tid, state, wait reason,
# context switches, kernel + user time in 100 ns ticks), read by hand.
SYSTEM_THREAD_ROWS = (
    (0, 4720, 12, 5, 0, 43, 0),
    (1, 4800, 16, 5, 0, 3995, 8_906_250),
    (457, 41280, 33572, 5, 15, 55747, 20_937_500),
)


def read_sample():
    with open(SAMPLE_PATH, "rb") as file:
        return file.read()


def thread_entries(sample, at, pid):
    """How many thread entries follow the process entry at `at` and name `pid` in
    their ClientId (byte 0x28 of a thread entry), counted by hand from the bytes."""
    count = 0
    while P.decode_u64(sample[at + 0x100 + count * 0x50 + 0x28:][:8]) == pid:
        count += 1
    return count


# Each process's thread count, counted from the thread entries Windows wrote
# after its entry; CapturedTable checks NumberOfThreads (byte 4) agrees.
IDLE_THREADS = thread_entries(read_sample(), 0, 0)
SYSTEM_THREADS = thread_entries(read_sample(), SYSTEM_AT, 4)


def sample_base(sample):
    """The address the table was read at: System's ImageName Buffer (byte 0x40 of its
    entry, read by hand) is an address in that read, and the name text sits at
    SYSTEM_NAME_AT in the file."""
    return P.decode_u64(sample[SYSTEM_AT + 0x40:SYSTEM_AT + 0x48]) - SYSTEM_NAME_AT


TWO_PROCESSES = [
    {"pid": 7, "parent": 3, "name": "game.exe", "user": 100_000, "kernel": 50_000, "read": 11,
     "write": 22, "faults": 33,
     "threads": [(71, 10_000, 20_000, 5, 2, 0), (72, 30_000, 40_000, 6, 5, 6)]},
    {"pid": 8, "parent": 7, "name": "other.exe", "user": 1, "kernel": 2, "read": 3, "write": 4,
     "faults": 5, "threads": [(81, 1, 2, 3, 1, 0)]},
]


def with_system_states(sample, states):
    """The sample with System's first threads set to `states`, by literal offsets."""
    changed = bytearray(sample)
    for index, state in enumerate(states):
        struct.pack_into("<I", changed, SYSTEM_AT + 0x100 + index * 0x50 + 0x44, state)
    return bytes(changed)


class CapturedTable(unittest.TestCase):
    """The parser on bytes Windows wrote. Every expected value is a literal."""

    def setUp(self):
        self.sample = read_sample()

    def parse(self, sample=None, **options):
        return M.parse_table(self.sample if sample is None else sample,
                             sample_base(self.sample), **options)

    def u32(self, at):
        return P.decode_u32(self.sample[at:at + 4])

    def u64(self, at):
        return P.decode_u64(self.sample[at:at + 8])

    def test_the_sample_is_the_table_that_was_captured(self):
        # NextEntryOffset (byte 0 of an entry) and NumberOfThreads (byte 4), by hand.
        # Each NumberOfThreads equals the thread entries that name the process.
        self.assertGreater(IDLE_THREADS, 0)
        self.assertGreater(SYSTEM_THREADS, 0)
        self.assertEqual((self.u32(0), self.u32(4)), (SYSTEM_AT, IDLE_THREADS))
        self.assertEqual((self.u32(SYSTEM_AT), self.u32(SYSTEM_AT + 4)), (0, SYSTEM_THREADS))
        # UniqueProcessId (byte 0x50) and InheritedFromUniqueProcessId (byte 0x58).
        self.assertEqual((self.u64(0x50), self.u64(0x58)), (0, 0))
        self.assertEqual((self.u64(SYSTEM_AT + 0x50), self.u64(SYSTEM_AT + 0x58)), (4, 0))
        # System's ImageName: Length, a USHORT (byte 0x38), and Buffer (byte 0x40).
        length = int.from_bytes(self.sample[SYSTEM_AT + 0x38:SYSTEM_AT + 0x3A], "little")
        self.assertEqual(length, 12)
        self.assertEqual(self.sample[SYSTEM_NAME_AT:SYSTEM_NAME_AT + length].decode("utf-16-le"),
                         "System")
        # System is the last entry, and its name text is the last thing in it, so
        # the file ends within the 8-byte alignment after that text.
        name_end = SYSTEM_NAME_AT + length
        self.assertLessEqual(name_end, len(self.sample))
        self.assertLess(len(self.sample), name_end + 8)
        for index, at, tid, state, reason, switches, ticks in SYSTEM_THREAD_ROWS:
            with self.subTest(thread=index):
                self.assertEqual(at, SYSTEM_AT + 0x100 + index * 0x50)
                # KernelTime (byte 0), UserTime (8), ClientId (0x28: pid, then tid at
                # 0x30), ContextSwitches (0x40), ThreadState (0x44), WaitReason (0x48).
                self.assertEqual((self.u64(at + 0x28), self.u64(at + 0x30)), (4, tid))
                self.assertEqual((self.u32(at + 0x40), self.u32(at + 0x44), self.u32(at + 0x48)),
                                 (switches, state, reason))
                self.assertEqual(self.u64(at) + self.u64(at + 8), ticks)

    def by_hand(self, at, layout, field):
        """One field of the entry at `at`, read at the literal layout above."""
        offset, code = layout[field]
        raw = self.sample[at + offset:at + offset + struct.calcsize("<" + code)]
        if code == "H":
            return int.from_bytes(raw, "little")
        # The sample's times and counts are all non-negative, so q reads as Q.
        return (P.decode_u32 if code == "I" else P.decode_u64)(raw)

    def process_by_hand(self, at):
        """The entry's process fields, with no thread rows, read by hand."""
        field = lambda name: self.by_hand(at, PROCESS_LAYOUT, name)  # noqa: E731
        name_at = field("name_buffer") - sample_base(self.sample) if field("name_length") else 0
        name = self.sample[name_at:name_at + field("name_length")].decode("utf-16-le")
        return (field("pid"), field("parent"), name, field("user") + field("kernel"),
                field("read"), field("write"), field("faults"))

    def threads_by_hand(self, at, count):
        rows = []
        for number in range(count):
            first = at + PROCESS_SIZE + number * THREAD_SIZE
            field = lambda name: self.by_hand(first, THREAD_LAYOUT, name)  # noqa: E731
            rows.append(M.Thread(field("tid"), field("state"), field("wait_reason"),
                                 field("switches"), field("kernel") + field("user")))
        return rows

    def test_the_parser_reads_the_captured_table(self):
        table = self.parse()
        self.assertEqual(list(table.processes), [0, 4])
        self.assertEqual(table.thread_count, IDLE_THREADS + SYSTEM_THREADS)
        idle, system = table.processes[0], table.processes[4]
        for process, at in ((idle, 0), (system, SYSTEM_AT)):
            with self.subTest(pid=process.pid):
                self.assertEqual(process[:-1], self.process_by_hand(at))
        self.assertEqual((idle.name, system.name), ("", "System"))
        # System's thread ids are all distinct, so its dict holds every thread.
        system_rows = self.threads_by_hand(SYSTEM_AT, SYSTEM_THREADS)
        self.assertEqual(system.threads, {row.tid: row for row in system_rows})
        self.assertEqual(len(system.threads), SYSTEM_THREADS)
        for index, at, tid, state, reason, switches, ticks in SYSTEM_THREAD_ROWS:
            with self.subTest(thread=index):
                self.assertEqual(system.threads[tid], M.Thread(tid, state, reason, switches, ticks))
        # Some of Idle's entries share thread id 0 (one per processor), so its dict,
        # keyed by thread id, keeps one per distinct id; each unshared id holds its
        # own entry.
        idle_rows = self.threads_by_hand(0, IDLE_THREADS)
        tids = [row.tid for row in idle_rows]
        self.assertLess(len(set(tids)), IDLE_THREADS)
        self.assertEqual(set(idle.threads), set(tids))
        for row in idle_rows:
            if tids.count(row.tid) == 1:
                with self.subTest(idle_thread=row.tid):
                    self.assertEqual(idle.threads[row.tid], row)
        # Idle's threads are left out of the run queue, and every System thread waits.
        self.assertEqual(table.ready_threads,
                         sum(M.reads_as(row.state) == M.READY for row in system_rows))

    def test_a_live_table_holds_this_process_as_windows_reports_it(self):
        # A table Windows writes now, read by the recorder's own reader. The pid,
        # parent pid, image name and thread id come from the OS, so each of the
        # parser's offsets and entry sizes is checked against what Windows says.
        buffer, base = M.read_table(M.TableBuffer())
        table = M.parse_table(buffer, base, thread_pids=[os.getpid()])
        me = table.processes[os.getpid()]
        self.assertEqual(me.parent_pid, os.getppid())
        self.assertEqual(me.name.lower(), os.path.basename(sys.executable).lower())
        tid = threading.get_native_id()
        self.assertIn(tid, me.threads)
        # This thread made the call that read the table, so it was on a processor.
        self.assertEqual(M.reads_as(me.threads[tid].state), M.RUNNING)
        self.assertGreater(me.cpu_ticks, 0)

    def test_names_count_from_the_address_the_table_was_read_at(self):
        self.assertEqual(self.parse().processes[4].name, "System")
        moved = M.parse_table(self.sample, sample_base(self.sample) - 2)
        self.assertNotEqual(moved.processes[4].name, "System")

    def test_the_filter_keeps_counts_for_the_whole_table(self):
        whole = self.parse()
        table = self.parse(thread_pids=[4])
        self.assertEqual(len(table.processes[4].threads), SYSTEM_THREADS)
        self.assertEqual(table.processes[0].threads, {})
        self.assertEqual((table.thread_count, table.ready_threads),
                         (IDLE_THREADS + SYSTEM_THREADS, whole.ready_threads))

    def test_an_empty_filter_parses_no_threads(self):
        table = self.parse(thread_pids=())
        self.assertEqual((table.processes[0].threads, table.processes[4].threads), ({}, {}))
        self.assertEqual(table.thread_count, IDLE_THREADS + SYSTEM_THREADS)

    def test_every_ready_state_counts_toward_the_run_queue(self):
        table = self.parse(with_system_states(self.sample, (1, 2, 3, 4, 5, 6, 7)))
        self.assertEqual(table.ready_threads, 3)

    def test_the_idle_process_adds_nothing_to_the_run_queue(self):
        # Some of Idle's own entries hold a state that reads as ready (byte 0x44 of
        # each, by hand), yet the run queue counts only System's ready threads.
        def ready(at, count):
            return sum(M.reads_as(self.u32(at + 0x100 + n * 0x50 + 0x44)) == M.READY
                       for n in range(count))

        self.assertGreater(ready(0, IDLE_THREADS), 0)
        self.assertEqual(self.parse().ready_threads, ready(SYSTEM_AT, SYSTEM_THREADS))

    def test_each_ready_state_of_a_real_process_counts_beside_the_idle_process(self):
        # Every System thread in the sample waits, so one thread set to a state
        # adds one to the run queue exactly when that state reads as ready.
        self.assertTrue(M.READY_STATES)
        for state in range(max(M.STATE_READS_AS) + 2):
            with self.subTest(state=state):
                table = self.parse(with_system_states(self.sample, (state,)))
                self.assertEqual(table.ready_threads, int(M.reads_as(state) == M.READY))

    def test_no_test_builds_bytes_from_the_parser_offsets(self):
        with open(os.path.abspath(__file__), encoding="utf-8") as file:
            text = file.read()
        for name in ("M." + "PROC_", "M." + "THREAD_KERNEL", "M." + "THREAD_USER",
                     "M." + "THREAD_UNIQUE", "M." + "THREAD_CONTEXT", "M." + "THREAD_WAIT",
                     "fixture" + "_table"):
            with self.subTest(name=name):
                self.assertNotIn(name, text)


class FakeTable(unittest.TestCase):
    """The literal-layout fake the loop tests use, and what only a fake can show."""

    def test_the_fake_reads_back(self):
        table = M.parse_table(fake_table(TWO_PROCESSES))
        self.assertEqual(list(table.processes), [7, 8])
        self.assertEqual((table.thread_count, table.ready_threads), (3, 1))
        game = table.processes[7]
        self.assertEqual((game.name, game.parent_pid, game.cpu_ticks, game.read_bytes,
                          game.write_bytes, game.hard_faults),
                         ("game.exe", 3, 150_000, 11, 22, 33))
        self.assertEqual(game.threads, {
            71: M.Thread(71, 2, 0, 5, 30_000),
            72: M.Thread(72, 5, 6, 6, 70_000),
        })

    def test_the_next_entry_offset_finds_the_next_process(self):
        # Windows may pad an entry; only NextEntryOffset says where the next one is.
        first = fake_table(TWO_PROCESSES[:1])
        second = fake_table(TWO_PROCESSES[1:])
        padded = bytearray(first + b"\0" * 8)
        struct.pack_into("<I", padded, 0x00, len(padded))
        pointer = P.decode_u64(second[0x40:0x48])
        moved = bytearray(second)
        struct.pack_into("<Q", moved, 0x40, pointer + len(padded))
        table = M.parse_table(bytes(padded) + bytes(moved))
        self.assertEqual(table.processes[8].name, "other.exe")


class ReadsAs(unittest.TestCase):
    def test_each_state_number_reads_as_its_word_and_any_other_reads_as_other(self):
        for state, word in M.STATE_READS_AS.items():
            with self.subTest(state=state):
                self.assertEqual(M.reads_as(state), word)
        self.assertEqual(M.reads_as(max(M.STATE_READS_AS) + 1), M.OTHER)

    def test_the_captured_system_threads_read_as_waiting(self):
        # No System thread in the captured table is in the run queue (its ready
        # count is 0), and every one of them reads as waiting.
        system = M.parse_table(read_sample(), sample_base(read_sample())).processes[4]
        self.assertEqual({M.reads_as(thread.state) for thread in system.threads.values()},
                         {M.WAITING})


class Deltas(unittest.TestCase):
    def test_gains_and_a_vanished_process(self):
        before = M.parse_table(fake_table(TWO_PROCESSES)).processes
        later = [dict(TWO_PROCESSES[0], user=256_250, read=111, write=222, faults=34,
                      threads=[(71, 10_000, 30_000, 9, 2, 0), (72, 30_000, 40_000, 6, 5, 6)])]
        after = M.parse_table(fake_table(later)).processes
        gains = M.deltas(before, after)
        self.assertEqual(list(gains), [7])
        self.assertEqual(gains[7].cpu_ms, 15.625)
        self.assertEqual((gains[7].read_bytes, gains[7].write_bytes, gains[7].hard_faults),
                         (100, 200, 1))
        self.assertEqual(gains[7].threads, {71: M.ThreadGain(4, 1.0), 72: M.ThreadGain(0, 0.0)})

    # The three tests below read the table Windows wrote (recorder_sample.bin).

    def captured(self, **options):
        sample = read_sample()
        return M.parse_table(sample, sample_base(sample), **options).processes

    def assert_nothing_gained(self, gain):
        self.assertEqual(gain[:4], (0, 0, 0, 0))
        self.assertEqual({thread.switches for thread in gain.threads.values()} - {0}, set())
        self.assertEqual({thread.cpu_ms for thread in gain.threads.values()} - {0}, set())

    def test_a_process_first_seen_gains_nothing(self):
        current = self.captured()
        gains = M.deltas({}, current)
        self.assertEqual(gains.keys(), current.keys())
        for pid, gain in gains.items():
            with self.subTest(pid=pid):
                # Both captured processes have run, so a gain from zero would show.
                self.assertGreater(current[pid].cpu_ticks, 0)
                self.assertEqual(gain.threads.keys(), current[pid].threads.keys())
                self.assert_nothing_gained(gain)

    def test_a_thread_first_seen_gains_nothing(self):
        before = self.captured(thread_pids=())
        after = self.captured()
        system = M.deltas(before, after)[4]
        self.assertEqual(len(system.threads), len(after[4].threads))
        self.assert_nothing_gained(system)

    def test_a_reused_pid_gains_nothing(self):
        # One pid held first by System and then by a process of another name with
        # more CPU time: the Idle process, both as Windows wrote them. A CPU gain
        # worked across the two would be above zero.
        captured = self.captured()
        system, idle = captured[4], captured[0]
        self.assertGreater(idle.cpu_ticks, system.cpu_ticks)
        gains = M.deltas({4: system}, {4: idle})
        self.assert_nothing_gained(gains[4])


def read_csv(folder, name):
    with open(os.path.join(folder, name), newline="", encoding="utf-8") as file:
        return list(csv.reader(file))


def read_rows(folder, name):
    with open(os.path.join(folder, name), newline="", encoding="utf-8") as file:
        return list(csv.DictReader(file))


# Six lines of samples.csv from a recording of a real run: the header, the first
# sample, two samples with a song clock and the last two, copied unchanged from
# hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1/machine-1/samples.csv.
REAL_SAMPLES = os.path.join(ROOT, "testdata", "ch_runs", "ls-p3-run1", "machine-1",
                            "samples.csv")


def real_sample_rows():
    """The recorded rows as SampleRows, each cell turned into its field's type."""
    hints = typing.get_type_hints(M.SampleRow)
    rows = []
    with open(REAL_SAMPLES, newline="", encoding="utf-8") as file:
        for record in csv.DictReader(file):
            values = {}
            for field in dataclasses.fields(M.SampleRow):
                kind = next(kind for kind in typing.get_args(hints[field.name]) or
                            (hints[field.name],) if kind is not type(None))
                text = record[field.name]
                values[field.name] = None if text == "" else kind(text)
            rows.append(M.SampleRow(**values))
    return rows


def captured_table(**options):
    sample = read_sample()
    return M.parse_table(sample, sample_base(sample), **options)


def live_table(held, **options):
    """A table Windows writes now, read and parsed by the recorder's own code."""
    return M.parse_table(*M.read_table(held), **options)


def burn_cpu(seconds):
    """Spin until this process has used `seconds` of CPU, so its next table shows
    a gain. The spin always uses CPU, so the cap only guards a stopped clock."""
    start, cap = time.process_time(), time.perf_counter() + SUBPROCESS_CAP_S
    while time.process_time() - start < seconds:
        if time.perf_counter() > cap:
            raise AssertionError("this process's CPU time did not move")


class WriteSample(unittest.TestCase):
    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.addCleanup(self._dir.cleanup)
        self.folder = self._dir.name
        self.files = M.RecorderFiles(self.folder)
        self.addCleanup(self.files.close)
        self.row = real_sample_rows()[0]

    def test_the_columns_are_pinned_with_one_header_per_file(self):
        # The recorded rows written again give back the recorded file byte for
        # byte: one header, each cell in the form the recorder wrote in that run,
        # and an empty cell for each missing value. The copy is read with universal
        # newlines, because git may check it out with CRLF line ends.
        with open(REAL_SAMPLES, encoding="utf-8") as file:
            recorded = file.read()
        table = captured_table(thread_pids=())
        nothing_moved = M.deltas(table.processes, table.processes)
        for row in real_sample_rows():
            M.write_sample(self.files, row, table, nothing_moved, frozenset())
        self.files.close()
        with open(os.path.join(self.folder, M.SAMPLES_FILE), newline="", encoding="utf-8") as file:
            self.assertEqual(file.read(), recorded)
        self.assertEqual(read_csv(self.folder, M.SAMPLES_FILE)[0], list(M.SAMPLES_COLUMNS))
        for name, columns in ((M.PROCESSES_FILE, M.PROCESSES_COLUMNS),
                              (M.THREADS_FILE, M.THREADS_COLUMNS)):
            with self.subTest(name):
                self.assertEqual(read_csv(self.folder, name), [list(columns)])

    def test_a_process_row_needs_a_gain_or_a_listed_pid(self):
        # Two live tables with CPU spent between them: this process gains, and the
        # processes that gained nothing get no row unless they are listed.
        held = M.TableBuffer()
        before = live_table(held, thread_pids=())
        burn_cpu(0.05)
        after = live_table(held, thread_pids=())
        gains = M.deltas(before.processes, after.processes)
        moved = {pid for pid, gain in gains.items() if gain[:4] != (0, 0, 0, 0)}
        quiet = sorted(set(gains) - moved)
        self.assertIn(os.getpid(), moved)
        self.assertTrue(quiet, "every process on the machine gained between two reads")
        listed = quiet[0]
        M.write_sample(self.files, self.row, after, gains, frozenset({listed}))
        rows = {int(row["pid"]): row for row in read_rows(self.folder, M.PROCESSES_FILE)}
        self.assertEqual(set(rows), moved | {listed})
        process = after.processes[listed]
        self.assertEqual(rows[listed], {
            "sample": str(self.row.sample), "pid": str(listed),
            "parent_pid": str(process.parent_pid), "name": process.name, "cpu_ms": "0.0",
            "read_bytes": "0", "write_bytes": "0", "hard_faults": "0"})
        self.assertGreater(float(rows[os.getpid()]["cpu_ms"]), 0)

    def test_a_thread_row_needs_a_non_waiting_state_or_moved_switches(self):
        # The captured table with nothing moved: only the threads in a state that
        # does not read as waiting get rows. Every one of System's threads waits.
        table = captured_table()
        M.write_sample(self.files, self.row, table, M.deltas(table.processes, table.processes),
                       frozenset())
        written = [(int(row["pid"]), int(row["tid"]), row["reads_as"])
                   for row in read_rows(self.folder, M.THREADS_FILE)]
        expected = [(pid, tid, M.reads_as(thread.state))
                    for pid, process in table.processes.items()
                    for tid, thread in process.threads.items()
                    if M.reads_as(thread.state) != M.WAITING]
        self.assertTrue(expected)
        self.assertEqual(written, expected)
        self.assertNotIn(4, {pid for pid, _, _ in written})

        # Live: a thread of this process that wakes every millisecond is caught
        # waiting with its switches moved, and that alone gives it a row.
        stop = threading.Event()

        def tick():
            while not stop.wait(0.001):
                pass

        ticker = threading.Thread(target=tick, daemon=True)
        ticker.start()
        self.addCleanup(stop.set)
        held = M.TableBuffer()
        for attempt in range(5):
            before = live_table(held, thread_pids=[os.getpid()])
            time.sleep(0.05)
            after = live_table(held, thread_pids=[os.getpid()])
            gains = M.deltas(before.processes, after.processes)
            ticked = gains[os.getpid()].threads.get(ticker.native_id)
            state = after.processes[os.getpid()].threads[ticker.native_id].state
            if ticked and ticked.switches and M.reads_as(state) == M.WAITING:
                break
        else:
            self.fail("in 5 tries the ticking thread was never caught waiting after a switch")
        folder = tempfile.mkdtemp(dir=self.folder)
        files = M.RecorderFiles(folder)
        M.write_sample(files, self.row, after, {os.getpid(): gains[os.getpid()]}, frozenset())
        files.close()
        rows = {int(row["tid"]): row for row in read_rows(folder, M.THREADS_FILE)}
        mine = after.processes[os.getpid()].threads
        self.assertEqual(set(rows), {tid for tid, gain in gains[os.getpid()].threads.items()
                                     if gain.switches or M.reads_as(mine[tid].state) != M.WAITING})
        self.assertEqual((rows[ticker.native_id]["reads_as"], rows[ticker.native_id]["switches"]),
                         (M.WAITING, str(ticked.switches)))

    def test_a_folder_with_a_recording_is_refused(self):
        with self.assertRaises(FileExistsError):
            M.RecorderFiles(self.folder)


class FakeLoop:
    """The sample loop on a fake clock. `on_sleep(sample)` runs after each sample,
    and `after` samples with no stop the run is a test failure."""

    def __init__(self, folder, tables=None, inputs=None, open_reader=None, on_table=None,
                 on_sleep=None, after=50):
        self.folder = folder
        self.tables = tables or (lambda sample: TWO_PROCESSES)
        self.inputs = inputs or M.Inputs()
        self.stop = M.Stop()
        self.time = 100.0
        self.sample = 0
        self.on_sleep = on_sleep
        self.on_table = on_table
        self.after = after
        self.open_reader = open_reader or (lambda pid: self.fail("no clock expected"))

    def fail(self, text):
        raise AssertionError(text)

    def read_table(self):
        self.sample += 1
        if self.on_table:
            self.on_table(self.sample)
        return fake_table(self.tables(self.sample)), 0

    def sleep(self, seconds):
        self.time += seconds
        if self.on_sleep:
            self.on_sleep(self.sample)
        if self.sample > self.after:
            self.stop.set("test ran out")

    def run(self, files=None, **limits):
        files = files or M.RecorderFiles(self.folder)
        seams = M.Seams(self.read_table, self.open_reader, wall=lambda: self.time + 1000,
                        monotonic=lambda: self.time, sleep=self.sleep,
                        process_time=lambda: 1.25)
        try:
            return M.record(files, self.inputs, self.stop, seams, period_s=PERIOD_S,
                            parent_pid=PARENT, recorder_pid=RECORDER, **limits)
        finally:
            files.close()


class Folder(unittest.TestCase):
    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.addCleanup(self._dir.cleanup)
        self.folder = self._dir.name


class Loop(Folder):
    def test_until_the_pid_line_no_process_is_left_out_of_the_gains(self):
        def tables(sample):
            return [dict(TWO_PROCESSES[1], pid=55, user=sample * 10_000)]

        loop = FakeLoop(self.folder, tables=tables)
        loop.run(cap_s=PERIOD_S)
        rows = read_rows(self.folder, "processes.csv")
        self.assertEqual([(row["sample"], row["pid"], row["cpu_ms"]) for row in rows],
                         [("1", "55", "0.0"), ("2", "55", "1.0")])
        self.assertEqual(read_rows(self.folder, "threads.csv"), [])

    def test_the_first_end_to_arrive_is_kept(self):
        stop = M.Stop()
        stop.set(M.STOP_PARENT)
        stop.set(M.STOP_STDIN)
        self.assertEqual(stop.reason, M.STOP_PARENT)

    def test_each_row_carries_the_pid_and_period_and_only_the_last_the_process_time(self):
        loop = FakeLoop(self.folder)
        loop.run(cap_s=PERIOD_S * 2)
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual({row["recorder_pid"] for row in samples}, {str(RECORDER)})
        self.assertEqual({row["period_ms"] for row in samples}, {"15.625"})
        self.assertEqual([row["sample"] for row in samples], ["1", "2", "3"])
        self.assertEqual([row["process_time_s"] for row in samples], ["", "", "1.25"])

    def test_a_bad_stdin_line_is_ignored(self):
        inputs = M.Inputs()
        for line in ("nonsense\n", "pid x\n", "engine zz\n", "pid\n", ""):
            with unittest.mock.patch("sys.stderr"):
                inputs.apply(line)
        self.assertEqual((inputs.target, inputs.engine), (None, None))

    def test_stdin_bytes_and_text_both_read(self):
        inputs = M.Inputs()
        M.read_stdin([b"pid 7\n", "engine 1f\n"], inputs, M.Stop())
        self.assertEqual((inputs.target, inputs.engine), (7, 0x1f))


class ReadTable(unittest.TestCase):
    def test_a_table_that_outgrows_the_buffer_grows_it_and_keeps_it(self):
        sizes = []
        table = b"table bytes"

        def query(information_class, address, size, needed):
            self.assertEqual(information_class, 5)
            sizes.append(size)
            # Each answer falls short of the table, as when it grows between two calls.
            if len(sizes) == 1:
                needed._obj.value = 4
                return M._STATUS_INFO_LENGTH_MISMATCH
            if size < len(table):
                needed._obj.value = len(table) - 1
                return M._STATUS_INFO_LENGTH_MISMATCH
            ctypes.memmove(address, table, len(table))
            needed._obj.value = len(table)
            return 0

        held = M.TableBuffer()
        data, base = M.read_table(held, query)
        self.assertEqual(data, table)
        self.assertEqual(base, ctypes.addressof(held.raw))
        self.assertEqual(sizes, [0, 4, 10, 20])
        M.read_table(held, query)
        self.assertEqual(sizes[-1], 20)

    def test_another_failure_is_loud(self):
        # Windows cannot write the table into read-only memory, so the real query
        # fails with a status of its own, which the recorder must name.
        memory = Memory(self, TABLE_PAGES, PAGE_READONLY)
        held = M.TableBuffer()
        held.raw = (ctypes.c_char * memory.size).from_address(memory.base)
        needed = ctypes.c_ulong(0)
        status = M._nt_query()(M.SYSTEM_PROCESS_INFORMATION_CLASS, memory.base, memory.size,
                               ctypes.byref(needed))
        self.assertNotIn(status, (0, M._STATUS_INFO_LENGTH_MISMATCH))
        with self.assertRaises(OSError) as caught:
            M.read_table(held)
        self.assertIn(f"{status:#x}", str(caught.exception))
        # Not taken for a short buffer: the buffer was never grown.
        self.assertEqual(ctypes.addressof(held.raw), memory.base)


class CommandLine(Folder):
    def run_main(self, *args):
        with unittest.mock.patch.object(M, "open_parent_wait", lambda pid: lambda: None), \
                unittest.mock.patch.object(M, "record", return_value="cap") as record, \
                unittest.mock.patch("sys.stdin", unittest.mock.Mock(
                    fileno=unittest.mock.Mock(return_value=0))), \
                unittest.mock.patch.object(M.os, "fdopen", return_value=[]), \
                unittest.mock.patch("builtins.print"):
            self.assertEqual(M.main(["--out", self.folder, "--parent", "5", *args]), 0)
        return record.call_args.kwargs

    def test_a_real_run_sampled_at_the_default_rate(self):
        # recorder_link starts the recorder with no --rate, so the run recorded in
        # REAL_SAMPLES took its samples at RATE_PER_S: each row says so, and two
        # rows in a row are one period apart.
        period_s = 1 / M.RATE_PER_S
        rows = read_rows(os.path.dirname(REAL_SAMPLES), M.SAMPLES_FILE)
        self.assertEqual({float(row["period_ms"]) for row in rows}, {C.s_to_ms(period_s)})
        by_number = {int(row["sample"]): float(row["wall_s"]) for row in rows}
        pairs = [(by_number[n], by_number[n + 1]) for n in by_number if n + 1 in by_number]
        self.assertTrue(pairs)
        for before, after in pairs:
            self.assertAlmostEqual(after - before, period_s, delta=period_s / 4)

    def test_the_rate_defaults_to_the_one_constant(self):
        self.assertEqual(self.run_main("--cap-s", "9")["period_s"], 0.02)

    def test_run_cap_s_adds_the_extra(self):
        self.assertEqual(self.run_main("--run-cap-s", "100")["cap_s"], 160)

    def test_two_caps_are_an_error(self):
        with self.assertRaises(SystemExit) as caught, unittest.mock.patch("sys.stderr"):
            M.main(["--out", self.folder, "--parent", "5", "--cap-s", "1", "--run-cap-s", "1"])
        self.assertEqual(caught.exception.code, 2)

    def test_no_end_at_all_is_an_error(self):
        with self.assertRaises(SystemExit) as caught, unittest.mock.patch("sys.stderr"):
            M.main(["--out", self.folder, "--parent", "5"])
        self.assertEqual(caught.exception.code, 2)

    def test_a_rate_of_zero_is_an_error(self):
        with self.assertRaises(SystemExit) as caught, unittest.mock.patch("sys.stderr"):
            M.main(["--out", self.folder, "--parent", "5", "--cap-s", "1", "--rate", "0"])
        self.assertEqual(caught.exception.code, 2)


class Imports(unittest.TestCase):
    def test_it_imports_neither_the_game_tools_nor_the_watch(self):
        code = ("import sys; sys.path.insert(0, sys.argv[1]); "
                "import tools.ch_mcp.machine_recorder; "
                "print([m for m in sys.modules if m.endswith(('game_tools', 'sp_tools'))])")
        out = subprocess.run([sys.executable, "-I", "-c", code, ROOT], capture_output=True,
                             text=True, timeout=SUBPROCESS_CAP_S, check=True)
        self.assertEqual(out.stdout.strip(), "[]")


@unittest.skipUnless(sys.platform == "win32", "the process table is Windows' own")
class RealRun(Folder):
    def start(self, *args, parent=None, stdin=subprocess.DEVNULL):
        out = os.path.join(self.folder, "recording")
        # Started from another folder with -I, the shape the worker starts it in.
        return out, subprocess.Popen(
            [sys.executable, "-I", SCRIPT, "--out", out, "--parent",
             str(parent or os.getpid()), *args],
            cwd=self.folder, stdin=stdin, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True)

    def test_a_one_second_run_on_this_process(self):
        before = time.time()
        out, process = self.start("--target", str(os.getpid()), "--seconds", "1")
        stdout, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        after = time.time()
        self.assertEqual(process.returncode, 0, stderr)
        first = stdout.splitlines()[0]
        self.assertIn(f"pid {process.pid},", first)
        self.assertIn(out, first)
        sampling_thread = first.split("sampling thread ")[1].split(",")[0]

        samples = read_rows(out, "samples.csv")
        self.assertGreaterEqual(len(samples), 20)
        self.assertEqual(samples[-1]["stop_reason"], "seconds")
        self.assertGreater(float(samples[-1]["process_time_s"]), 0)
        self.assertTrue(all(before <= float(row["wall_s"]) <= after for row in samples))
        self.assertEqual({row["recorder_pid"] for row in samples}, {str(process.pid)})
        self.assertGreater(int(samples[0]["processes"]), 1)

        processes = read_rows(out, "processes.csv")
        names = {row["pid"]: row["name"] for row in processes}
        self.assertIn(str(os.getpid()), names)
        self.assertIn(str(process.pid), names)
        self.assertEqual(names[str(process.pid)].lower(), os.path.basename(sys.executable).lower())
        parents = {row["pid"]: row["parent_pid"] for row in processes}
        self.assertEqual(parents[str(process.pid)], str(os.getpid()))

        running = {row["sample"] for row in read_rows(out, "threads.csv")
                   if row["tid"] == sampling_thread and row["reads_as"] == "running"}
        self.assertGreaterEqual(len(running) / len(samples), 0.95)

    def test_a_closed_stdin_ends_the_real_process(self):
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), stdin=subprocess.PIPE)
        process.stdin.write(f"pid {os.getpid()}\n")
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], M.STOP_STDIN)

    def test_a_cap_ends_the_real_process_with_its_stdin_still_open(self):
        out, process = self.start("--cap-s", "0.2", stdin=subprocess.PIPE)
        try:
            process.wait(SUBPROCESS_CAP_S)
            stderr = process.stderr.read()
        finally:
            process.stdin.close()
            process.stdout.close()
            process.stderr.close()
        self.assertEqual((process.returncode, stderr), (0, ""))
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], M.STOP_CAP)

    def test_a_dead_parent_ends_the_real_process(self):
        parent = subprocess.Popen([sys.executable, "-I", "-c", "input()"], stdin=subprocess.PIPE)
        self.addCleanup(parent.stdin.close)
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), "--target", str(os.getpid()),
                                  parent=parent.pid)
        self.assertIn("machine recorder", process.stdout.readline())
        parent.terminate()
        parent.wait(SUBPROCESS_CAP_S)
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], M.STOP_PARENT)

    def test_a_parent_that_cannot_be_opened_fails_loudly(self):
        gone = finished_pid()
        # The owner's own error for that pid, raised here in this process.
        with self.assertRaises(OSError) as owner:
            M.open_parent_wait(gone)
        self.assertIn(str(gone), str(owner.exception))
        out, process = self.start("--cap-s", "1", parent=gone)
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertNotEqual(process.returncode, 0)
        self.assertIn(str(owner.exception), stderr)
        # It fails before it makes a recording folder.
        self.assertFalse(os.path.exists(out))


def complete_rows(folder, name):
    """The rows of a file the recorder is still writing, less a last line it has
    not finished."""
    path = os.path.join(folder, name)
    if not os.path.exists(path):
        return []
    return [row for row in read_rows(folder, name) if None not in row.values()]


def idle_child():
    """A Python process that says it is ready, then blocks on its stdin and uses
    no CPU until the test closes it."""
    child = subprocess.Popen(
        [sys.executable, "-I", "-c", "import sys; print('ready', flush=True); sys.stdin.read()"],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    if child.stdout.readline().strip() != "ready":
        child.kill()
        raise AssertionError("the idle child did not start")
    return child


class RealInputs(Folder):
    """What the stdin lines and the listed pids do, on the real recorder process
    reading the real process table and the real memory of this process."""

    def start(self, *args, **options):
        out, process = RealRun.start(self, *args, **options)
        self.addCleanup(lambda: process.poll() is None and process.kill())
        return out, process

    def wait_for(self, process, folder, what, ready):
        """Read samples.csv until `ready(rows)` holds. The recorder ending first, or
        the cap passing, fails at once with what was awaited."""
        deadline = time.monotonic() + SUBPROCESS_CAP_S
        while True:
            if process.poll() is not None:
                self.fail(f"the recorder ended before {what}: {process.stderr.read()}")
            rows = complete_rows(folder, M.SAMPLES_FILE)
            if ready(rows):
                return int(rows[-1]["sample"])
            if time.monotonic() > deadline:
                self.fail(f"no {what} within {SUBPROCESS_CAP_S} s")
            time.sleep(1 / M.RATE_PER_S)

    def clock_run(self):
        """A recorder told by stdin first which pid to watch (this one) and then
        where its engine is: an address whose song clock offset holds a double in
        this process. Returns the files' rows, the last sample before each line
        was sent, and the recorder's pid."""
        clock = ctypes.c_double(1.5)
        engine = ctypes.addressof(clock) - C.OFF_SONG_CLOCK
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), stdin=subprocess.PIPE)
        self.assertIn("machine recorder", process.stdout.readline())
        before_pid = self.wait_for(process, out, "two samples", lambda rows: len(rows) >= 2)
        process.stdin.write(f"pid {os.getpid()}\n")
        process.stdin.flush()
        self.wait_for(process, out, "a thread row",
                      lambda rows: complete_rows(out, M.THREADS_FILE))
        before_engine = self.wait_for(process, out, "a sample after the thread rows",
                                      lambda rows: True)
        process.stdin.write(f"engine {engine:#x}\n")
        process.stdin.flush()
        self.wait_for(process, out, "a song clock",
                      lambda rows: any(row["clock_s"] for row in rows))
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        return (read_rows(out, M.SAMPLES_FILE), read_rows(out, M.THREADS_FILE), before_pid,
                before_engine, process.pid, clock.value)

    def test_stdin_lines_switch_the_threads_and_the_clock_on_at_their_samples(self):
        samples, threads, before_pid, before_engine, recorder, clock = self.clock_run()
        self.assertEqual(samples[-1]["stop_reason"], M.STOP_STDIN)
        self.assertGreater(min(int(row["sample"]) for row in threads), before_pid)
        # The pid line names this process, which is also the recorder's parent.
        self.assertEqual({int(row["pid"]) for row in threads}, {os.getpid(), recorder})
        early = [row for row in samples if int(row["sample"]) <= before_engine]
        self.assertEqual({(row["clock_s"], row["clock_wall_before_s"], row["clock_wall_after_s"])
                          for row in early}, {("", "", "")})
        clocked = [row for row in samples if row["clock_s"]]
        self.assertTrue(clocked)
        self.assertEqual({float(row["clock_s"]) for row in clocked}, {clock})
        # Once the engine is known every later sample reads the clock.
        first = int(clocked[0]["sample"])
        self.assertEqual([row["sample"] for row in clocked],
                         [row["sample"] for row in samples if int(row["sample"]) >= first])

    def test_the_before_stamp_is_the_samples_own_wall_stamp(self):
        # A live run, and the recording of a real run (REAL_SAMPLES): each sample
        # with a clock carries its own wall stamp as the before stamp, and the
        # after stamp is taken later.
        samples = self.clock_run()[0]
        with open(REAL_SAMPLES, newline="", encoding="utf-8") as file:
            recorded = list(csv.DictReader(file))
        for source, rows in (("live", samples), ("recorded", recorded)):
            clocked = [row for row in rows if row["clock_s"]]
            with self.subTest(source):
                self.assertTrue(clocked)
                for row in clocked:
                    self.assertEqual(row["clock_wall_before_s"], row["wall_s"])
                    self.assertGreaterEqual(float(row["clock_wall_after_s"]),
                                            float(row["clock_wall_before_s"]))

    def listed_run(self):
        """A one-second recorder run with this process as its target and an idle
        child as its parent, beside a second idle child nobody lists."""
        parent, bystander = idle_child(), idle_child()
        for child in (parent, bystander):
            self.addCleanup(child.stdout.close)
            self.addCleanup(child.wait, SUBPROCESS_CAP_S)
            self.addCleanup(child.stdin.close)
        out, process = self.start("--seconds", "1", "--target", str(os.getpid()),
                                  parent=parent.pid)
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        by_sample = {}
        for row in read_rows(out, M.PROCESSES_FILE):
            by_sample.setdefault(int(row["sample"]), {})[int(row["pid"])] = row
        samples = read_rows(out, M.SAMPLES_FILE)
        self.assertGreaterEqual(len(samples), 3)
        return samples, by_sample, parent.pid, bystander.pid, process.pid

    def test_the_game_the_worker_and_the_recorder_always_get_rows(self):
        samples, by_sample, parent, _, recorder = self.listed_run()
        for row in samples[1:]:
            with self.subTest(sample=row["sample"]):
                self.assertLessEqual({os.getpid(), parent, recorder},
                                     set(by_sample.get(int(row["sample"]), {})))
        # The idle parent gains nothing, so only its listing gives it its rows.
        self.assertEqual({by_sample[int(row["sample"])][parent]["cpu_ms"] for row in samples[1:]},
                         {"0.0"})

    def test_the_first_sample_lists_every_process_and_later_ones_keep_the_filter(self):
        samples, by_sample, _, bystander, _ = self.listed_run()
        first = by_sample[1]
        self.assertEqual(len(first), int(samples[0]["processes"]))
        self.assertEqual(int(first[bystander]["parent_pid"]), os.getpid())
        later = {pid for number, rows in by_sample.items() if number > 1 for pid in rows}
        self.assertNotIn(bystander, later)
        self.assertLess(max(len(rows) for number, rows in by_sample.items() if number > 1),
                        len(first))


def finished_pid():
    """The pid of a child that has ended, which no live process holds any more."""
    child = subprocess.run([sys.executable, "-I", "-c", "import os; print(os.getpid())"],
                           capture_output=True, text=True, timeout=SUBPROCESS_CAP_S, check=True)
    return int(child.stdout)


# Windows' memory constants, from the VirtualAlloc and memory-protection pages
# of the Win32 documentation.
MEM_COMMIT_AND_RESERVE = 0x3000
MEM_RELEASE = 0x8000
PAGE_NOACCESS = 0x01
PAGE_READONLY = 0x02
PAGE_READWRITE = 0x04
# 16 MiB of pages: more than this machine's whole process table, so the query
# writes into the memory rather than asking for a bigger buffer.
TABLE_PAGES = (16 << 20) // mmap.PAGESIZE


class Memory:
    """Pages of this process's own memory whose access a test shuts and opens, so
    the real recorder's reads of them really fail and really succeed."""

    def __init__(self, case, pages, protect=PAGE_READWRITE):
        from ctypes import wintypes  # noqa: PLC0415  (Windows only)
        self._k32 = ctypes.WinDLL("kernel32", use_last_error=True)
        self._k32.VirtualAlloc.restype = ctypes.c_void_p
        self._k32.VirtualAlloc.argtypes = (ctypes.c_void_p, ctypes.c_size_t, wintypes.DWORD,
                                           wintypes.DWORD)
        self._k32.VirtualProtect.restype = wintypes.BOOL
        self._k32.VirtualProtect.argtypes = (ctypes.c_void_p, ctypes.c_size_t, wintypes.DWORD,
                                             ctypes.POINTER(wintypes.DWORD))
        self._k32.VirtualFree.restype = wintypes.BOOL
        self._k32.VirtualFree.argtypes = (ctypes.c_void_p, ctypes.c_size_t, wintypes.DWORD)
        self.size = pages * mmap.PAGESIZE
        self.base = self._k32.VirtualAlloc(None, self.size, MEM_COMMIT_AND_RESERVE, protect)
        if not self.base:
            raise OSError(ctypes.get_last_error(), "VirtualAlloc failed")
        case.addCleanup(self._k32.VirtualFree, self.base, 0, MEM_RELEASE)

    def protect(self, page, protect):
        old = ctypes.c_ulong(0)
        if not self._k32.VirtualProtect(self.base + page * mmap.PAGESIZE, mmap.PAGESIZE, protect,
                                        ctypes.byref(old)):
            raise OSError(ctypes.get_last_error(), "VirtualProtect failed")


class RealEnds(Folder):
    """How each end stops the real recorder process, and what it does when the
    clock it is told to read cannot be read."""

    start = RealInputs.start
    wait_for = RealInputs.wait_for

    def assert_stopped_at_the_next_sample(self, samples, reason, ended_at):
        """Only the last row names the end. The sample under way when the end came
        began before it; one more may begin before the recorder has seen it, and
        the one after that is the last."""
        self.assertEqual(samples[-1]["stop_reason"], reason)
        self.assertEqual({row["stop_reason"] for row in samples[:-1]}, {""})
        after = [row["sample"] for row in samples if float(row["wall_s"]) >= ended_at]
        self.assertLessEqual(len(after), 2, f"samples begun after the end: {after}")

    def test_a_closed_stdin_stops_it_at_the_next_sample(self):
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), stdin=subprocess.PIPE)
        self.assertIn("machine recorder", process.stdout.readline())
        self.wait_for(process, out, "two samples", lambda rows: len(rows) >= 2)
        closed_at = time.time()
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)  # closes its stdin
        self.assertEqual(process.returncode, 0, stderr)
        self.assert_stopped_at_the_next_sample(read_rows(out, M.SAMPLES_FILE), M.STOP_STDIN,
                                               closed_at)

    def test_a_parent_that_ends_stops_it_at_the_next_sample(self):
        parent = idle_child()
        self.addCleanup(parent.stdout.close)
        self.addCleanup(parent.stdin.close)
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), "--target", str(os.getpid()),
                                  parent=parent.pid)
        self.assertIn("machine recorder", process.stdout.readline())
        self.wait_for(process, out, "two samples", lambda rows: len(rows) >= 2)
        parent.terminate()
        parent.wait(SUBPROCESS_CAP_S)
        gone_at = time.time()
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        self.assert_stopped_at_the_next_sample(read_rows(out, M.SAMPLES_FILE), M.STOP_PARENT,
                                               gone_at)

    def ended_by(self, *args):
        out, process = self.start("--target", str(os.getpid()), *args)
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        return read_rows(out, M.SAMPLES_FILE)

    def assert_the_first_sample_past_the_end_is_the_last(self, samples, reason, end_s):
        # The recorder times its end on its own clock and stamps rows on the wall
        # clock; one period covers the difference.
        period_s = 1 / M.RATE_PER_S
        self.assertEqual([row["stop_reason"] for row in samples],
                         [""] * (len(samples) - 1) + [reason])
        first, *_, before_last, last = (float(row["wall_s"]) for row in samples)
        self.assertGreater(last - first, end_s - period_s)
        self.assertLess(before_last - first, end_s + period_s)

    def test_the_cap_stops_it_when_it_comes_before_seconds(self):
        cap_s = 1
        samples = self.ended_by("--cap-s", str(cap_s), "--seconds", str(SUBPROCESS_CAP_S))
        self.assert_the_first_sample_past_the_end_is_the_last(samples, M.STOP_CAP, cap_s)

    def test_cap_s_is_the_exact_cap(self):
        # The real process with --cap-s alone ends one cap after it starts, not
        # RECORDER_CAP_EXTRA_S later as --run-cap-s does.
        cap_s = 1
        samples = self.ended_by("--cap-s", str(cap_s))
        self.assert_the_first_sample_past_the_end_is_the_last(samples, M.STOP_CAP, cap_s)

    def test_seconds_stops_it_when_they_come_before_the_cap(self):
        seconds = 1
        samples = self.ended_by("--cap-s", str(SUBPROCESS_CAP_S), "--seconds", str(seconds))
        self.assert_the_first_sample_past_the_end_is_the_last(samples, M.STOP_SECONDS, seconds)

    def test_a_sample_in_samples_csv_has_its_process_rows_on_disk(self):
        # Read while the recorder runs: samples.csv first, then processes.csv. The
        # target, the parent and the recorder get a process row in every sample,
        # so each sample samples.csv shows must already be in processes.csv.
        out, process = self.start("--target", str(os.getpid()), "--seconds", "1")
        deadline = time.monotonic() + SUBPROCESS_CAP_S
        shown = set()
        while process.poll() is None:
            if time.monotonic() > deadline:
                self.fail(f"the recorder did not end within {SUBPROCESS_CAP_S} s")
            samples = {int(row["sample"]) for row in complete_rows(out, M.SAMPLES_FILE)}
            written = {int(row["sample"]) for row in complete_rows(out, M.PROCESSES_FILE)}
            self.assertLessEqual(samples, written)
            shown.add(len(samples))
            time.sleep(1 / M.RATE_PER_S / 2)
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        self.assertGreaterEqual(len(shown - {0}), 3, "the files were seen growing fewer than 3 times")

    def told_to_read(self, pid, clock_at):
        """A recorder told by stdin to read the song clock of `pid` at `clock_at`."""
        out, process = self.start("--cap-s", str(SUBPROCESS_CAP_S), stdin=subprocess.PIPE)
        self.assertIn("machine recorder", process.stdout.readline())
        process.stdin.write(f"pid {pid}\nengine {clock_at - C.OFF_SONG_CLOCK:#x}\n")
        process.stdin.flush()
        return out, process

    def finish(self, out, process):
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertEqual(process.returncode, 0, stderr)
        return read_rows(out, M.SAMPLES_FILE)

    def assert_unread_rows_are_empty(self, rows):
        self.assertTrue(rows)
        self.assertEqual({(row["clock_s"], row["clock_wall_before_s"], row["clock_wall_after_s"])
                          for row in rows}, {("", "", "")})

    def wait_for_clock(self, process, out, what, read, after=0):
        """Wait for a sample past `after` whose clock was read (`read`) or not."""
        return self.wait_for(process, out, what, lambda rows: any(
            int(row["sample"]) > after and bool(row["clock_s"]) == read for row in rows))

    def test_a_clock_read_that_fails_leaves_the_column_empty_and_goes_on(self):
        memory = Memory(self, 1)
        ctypes.c_double.from_address(memory.base).value = 2.5
        out, process = self.told_to_read(os.getpid(), memory.base)
        self.wait_for_clock(process, out, "a song clock", True)
        memory.protect(0, PAGE_NOACCESS)
        shut = self.wait_for(process, out, "a sample", lambda rows: rows)
        failed = self.wait_for_clock(process, out, "a failed read", False, shut)
        memory.protect(0, PAGE_READWRITE)
        self.wait_for_clock(process, out, "a song clock again", True, failed)
        samples = self.finish(out, process)
        clocked = [int(row["sample"]) for row in samples if row["clock_s"]]
        self.assertEqual({float(row["clock_s"]) for row in samples if row["clock_s"]}, {2.5})
        unread = [row for row in samples if clocked[0] < int(row["sample"]) < clocked[-1]
                  and not row["clock_s"]]
        self.assert_unread_rows_are_empty(unread)

    def test_a_game_that_cannot_be_opened_leaves_the_column_empty_and_goes_on(self):
        clock = ctypes.c_double(2.5)
        out, process = self.told_to_read(finished_pid(), ctypes.addressof(clock))
        sent = self.wait_for(process, out, "a sample", lambda rows: rows)
        self.wait_for(process, out, "five more samples",
                      lambda rows: int(rows[-1]["sample"]) >= sent + 5)
        process.stdin.write(f"pid {os.getpid()}\n")
        process.stdin.flush()
        self.wait_for_clock(process, out, "a song clock from this process", True)
        samples = self.finish(out, process)
        first = min(int(row["sample"]) for row in samples if row["clock_s"])
        unread = [row for row in samples if sent < int(row["sample"]) < first]
        self.assertGreaterEqual(len(unread), 5)
        self.assert_unread_rows_are_empty(unread)
        self.assertEqual({float(row["clock_s"]) for row in samples if row["clock_s"]}, {2.5})

    def test_a_short_clock_read_leaves_the_column_empty(self):
        # The clock's 8 bytes straddle two pages; shutting the second leaves a
        # read that can copy only the first half.
        memory = Memory(self, 2)
        clock_at = memory.base + mmap.PAGESIZE - M.CLOCK_BYTES // 2
        ctypes.memmove(clock_at, struct.pack("<d", 2.5), M.CLOCK_BYTES)
        out, process = self.told_to_read(os.getpid(), clock_at)
        self.wait_for_clock(process, out, "a song clock", True)
        memory.protect(1, PAGE_NOACCESS)
        shut = self.wait_for(process, out, "a sample", lambda rows: rows)
        self.wait_for_clock(process, out, "a short read", False, shut)
        samples = self.finish(out, process)
        last_read = max(int(row["sample"]) for row in samples if row["clock_s"])
        self.assertEqual({float(row["clock_s"]) for row in samples if row["clock_s"]}, {2.5})
        self.assert_unread_rows_are_empty(
            [row for row in samples if int(row["sample"]) > last_read])


# Nine rows of samples.csv around a late sample, copied unchanged from
# hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1/machine-1/samples.csv: sample 108
# took 50.6 ms, more than two 20 ms periods.
LATE_SAMPLES = os.path.join(ROOT, "testdata", "ch_runs", "ls-e-run1", "machine-1",
                            "samples.csv")


class RecordedPace(unittest.TestCase):
    def test_a_late_sample_skips_ticks_and_does_not_burst(self):
        # After a late sample the next one starts at once, and the one after it a
        # whole period later: the missed ticks are skipped, not caught up in a
        # burst of samples with no wait between them.
        rows = read_rows(os.path.dirname(LATE_SAMPLES), M.SAMPLES_FILE)
        walls = [float(row["wall_s"]) for row in rows]
        late = [i for i, row in enumerate(rows[:-2])
                if float(row["sample_ms"]) > 2 * float(row["period_ms"])]
        self.assertTrue(late)
        for i in late:
            period_s = C.ms_to_s(float(rows[i]["period_ms"]))
            with self.subTest(sample=rows[i]["sample"]):
                self.assertLess(walls[i + 1] - walls[i] - C.ms_to_s(float(rows[i]["sample_ms"])),
                                period_s / 4)
                self.assertAlmostEqual(walls[i + 2] - walls[i + 1], period_s, delta=period_s / 4)


# Nine rows of samples.csv where Clone Hero's song clock crosses zero, copied
# unchanged from hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/machine-1/samples.csv
# (samples 876-884): the game's clock reads below zero through the lead-in.
LEAD_IN_SAMPLES = os.path.join(ROOT, "testdata", "ch_runs", "ls-t3-run-b",
                               "machine-1-samples-876-884", "samples.csv")


def lead_in_clocks():
    """The song clocks the recorder wrote in LEAD_IN_SAMPLES, in sample order."""
    rows = read_rows(os.path.dirname(LEAD_IN_SAMPLES), M.SAMPLES_FILE)
    return [float(row["clock_s"]) for row in rows if row["clock_s"]]


class RecordedClock(unittest.TestCase):
    def test_a_real_lead_in_reads_below_zero_and_the_recorder_kept_it(self):
        clocks = lead_in_clocks()
        self.assertLess(clocks[0], 0)
        self.assertGreater(clocks[-1], 0)
        self.assertEqual(clocks, sorted(clocks))


class ClockRange(Folder):
    start = RealInputs.start
    wait_for = RealInputs.wait_for
    told_to_read = RealEnds.told_to_read
    finish = RealEnds.finish
    def wait_for_clock(self, process, out, what, read, after=0):
        """RealEnds.wait_for_clock, which also fails at once on a written clock no
        song clock could hold: the empty column it waits for then never comes."""
        def ready(rows):
            written = [row["clock_s"] for row in rows if row["clock_s"]]
            bad = [text for text in written if not math.isfinite(float(text))]
            if bad:
                self.fail(f"the recorder wrote clock {bad[0]}")
            return any(int(row["sample"]) > after and bool(row["clock_s"]) == read
                       for row in rows)
        return self.wait_for(process, out, what, ready)

    def test_a_nonfinite_clock_is_no_clock_and_a_negative_one_is_kept(self):
        # The real recorder reads a double in this process's memory: first the
        # game's own lead-in clock, then each value no clock can hold, each time
        # followed by the lead-in clock again.
        lead_in = lead_in_clocks()[0]
        clock = ctypes.c_double(lead_in)
        out, process = self.told_to_read(os.getpid(), ctypes.addressof(clock))
        last = self.wait_for_clock(process, out, "the lead-in clock", True)
        for value in (float("nan"), float("inf"), float("-inf")):
            clock.value = value
            last = self.wait_for_clock(process, out, f"a sample with no clock for {value}",
                                       False, last)
            clock.value = lead_in
            last = self.wait_for_clock(process, out, "the lead-in clock again", True, last)
        samples = self.finish(out, process)
        self.assertEqual({float(row["clock_s"]) for row in samples if row["clock_s"]}, {lead_in})
        # A sample with no clock carries no clock stamps.
        for row in samples:
            with self.subTest(sample=row["sample"]):
                self.assertEqual(row["clock_wall_before_s"] == "", row["clock_s"] == "")
                self.assertEqual(row["clock_wall_after_s"] == "", row["clock_s"] == "")


if __name__ == "__main__":
    unittest.main()
