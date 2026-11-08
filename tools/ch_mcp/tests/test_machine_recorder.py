"""The machine recorder: the table parser on one table captured from Windows, the
gains, the files, the loop on fakes, and short real runs of the module as its own
process."""

from __future__ import annotations

import csv
import ctypes
import os
import queue
import struct
import subprocess
import sys
import tempfile
import threading
import time
import unittest
import unittest.mock

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

from tools.ch_mcp import machine_recorder as M  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402

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
# numbers below come from that read, and the thread counts from Windows'
# Get-Process at the same moment, not from the parser.
SAMPLE_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "recorder_sample.bin")
SAMPLE_BASE = 2170157575808  # the address the table was read at
SAMPLE_SIZE = 41_744
SYSTEM_AT = 4464  # where the second entry, System, starts
IDLE_THREADS = 48  # Get-Process -Id 0
SYSTEM_THREADS = 458  # Get-Process -Id 4
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


TWO_PROCESSES = [
    {"pid": 7, "parent": 3, "name": "game.exe", "user": 100_000, "kernel": 50_000, "read": 11,
     "write": 22, "faults": 33,
     "threads": [(71, 10_000, 20_000, 5, 2, 0), (72, 30_000, 40_000, 6, 5, 6)]},
    {"pid": 8, "parent": 7, "name": "other.exe", "user": 1, "kernel": 2, "read": 3, "write": 4,
     "faults": 5, "threads": [(81, 1, 2, 3, 1, 0)]},
]


def layout_of(fields):
    """The parser's struct layout for a {name: (offset, code)} dict."""
    return M._layout(fields.values())


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
        return M.parse_table(self.sample if sample is None else sample, SAMPLE_BASE, **options)

    def test_the_sample_is_the_table_that_was_captured(self):
        self.assertEqual(len(self.sample), SAMPLE_SIZE)
        # NextEntryOffset (byte 0 of an entry) and NumberOfThreads (byte 4), by hand.
        self.assertEqual(struct.unpack_from("<II", self.sample, 0), (SYSTEM_AT, IDLE_THREADS))
        self.assertEqual(struct.unpack_from("<II", self.sample, SYSTEM_AT), (0, SYSTEM_THREADS))
        # UniqueProcessId (byte 0x50) and InheritedFromUniqueProcessId (byte 0x58).
        self.assertEqual(struct.unpack_from("<QQ", self.sample, 0x50), (0, 0))
        self.assertEqual(struct.unpack_from("<QQ", self.sample, SYSTEM_AT + 0x50), (4, 0))
        # System's ImageName: Length (byte 0x38) and Buffer (byte 0x40).
        length, = struct.unpack_from("<H", self.sample, SYSTEM_AT + 0x38)
        pointer, = struct.unpack_from("<Q", self.sample, SYSTEM_AT + 0x40)
        self.assertEqual((length, pointer - SAMPLE_BASE), (12, 41728))
        self.assertEqual(self.sample[41728:41740].decode("utf-16-le"), "System")
        for index, at, tid, state, reason, switches, ticks in SYSTEM_THREAD_ROWS:
            with self.subTest(thread=index):
                self.assertEqual(at, SYSTEM_AT + 0x100 + index * 0x50)
                kernel, user = struct.unpack_from("<qq", self.sample, at)
                self.assertEqual(struct.unpack_from("<QQ", self.sample, at + 0x28), (4, tid))
                self.assertEqual(struct.unpack_from("<III", self.sample, at + 0x40),
                                 (switches, state, reason))
                self.assertEqual(kernel + user, ticks)

    def test_the_parser_reads_the_captured_table(self):
        table = self.parse()
        self.assertEqual(list(table.processes), [0, 4])
        self.assertEqual(table.thread_count, IDLE_THREADS + SYSTEM_THREADS)
        self.assertEqual(table.thread_count, 506)
        # Idle's threads are left out of the run queue, and every System thread waits.
        self.assertEqual(table.ready_threads, 0)
        idle, system = table.processes[0], table.processes[4]
        self.assertEqual((idle.pid, idle.parent_pid, idle.name, idle.cpu_ticks, idle.read_bytes,
                          idle.write_bytes, idle.hard_faults),
                         (0, 0, "", 8_630_397_031_250, 0, 0, 0))
        self.assertEqual((system.pid, system.parent_pid, system.name, system.cpu_ticks,
                          system.read_bytes, system.write_bytes, system.hard_faults),
                         (4, 0, "System", 97_909_843_750, 121_967_916, 2_250_372_155, 124))
        # System's thread ids are all distinct, so its dict holds every thread.
        self.assertEqual(len(system.threads), SYSTEM_THREADS)
        for index, at, tid, state, reason, switches, ticks in SYSTEM_THREAD_ROWS:
            with self.subTest(thread=index):
                self.assertEqual(system.threads[tid], M.Thread(tid, state, reason, switches, ticks))
        self.assertEqual(idle.threads[32], M.Thread(32, 3, 0, 101_772, 625_000))
        # Idle holds 48 entries, 16 of them (one per processor here) with thread id
        # 0, so its dict, keyed by thread id, keeps 33.
        self.assertEqual(len(idle.threads), 33)

    def test_the_parser_layout_is_windows_layout(self):
        self.assertEqual(M._PROCESS_FIELDS.format, layout_of(PROCESS_LAYOUT).format)
        self.assertEqual(M._THREAD_FIELDS.format, layout_of(THREAD_LAYOUT).format)
        self.assertEqual((M.PROCESS_ENTRY_SIZE, M.THREAD_ENTRY_SIZE, M.THREAD_STATE),
                         (PROCESS_SIZE, THREAD_SIZE, 0x44))

    def test_an_offset_moved_by_4_reads_a_wrong_table(self):
        # The parser reads the entries through _PROCESS_FIELDS and _THREAD_FIELDS,
        # so a wrong offset is one of those layouts built with the offset moved.
        moved_parent = dict(PROCESS_LAYOUT, parent=(0x58 + 4, "Q"))
        with unittest.mock.patch.object(M, "_PROCESS_FIELDS", layout_of(moved_parent)):
            table = self.parse()
        # The read takes the high half of the parent pid and HandleCount (byte 0x60).
        self.assertEqual(table.processes[4].parent_pid, 7509 << 32)
        self.assertNotEqual(table.processes[4].parent_pid, 0)
        moved_tid = dict(THREAD_LAYOUT, tid=(0x30 + 4, "Q"))
        with unittest.mock.patch.object(M, "_THREAD_FIELDS", layout_of(moved_tid)):
            table = self.parse()
        self.assertNotIn(33572, table.processes[4].threads)
        self.assertNotIn(12, table.processes[4].threads)

    def test_names_count_from_the_address_the_table_was_read_at(self):
        self.assertEqual(self.parse().processes[4].name, "System")
        moved = M.parse_table(self.sample, SAMPLE_BASE - 2)
        self.assertNotEqual(moved.processes[4].name, "System")

    def test_an_unnamed_process_reads_as_empty(self):
        self.assertEqual(self.parse().processes[0].name, "")

    def test_the_filter_keeps_counts_for_the_whole_table(self):
        table = self.parse(thread_pids=[4])
        self.assertEqual(len(table.processes[4].threads), SYSTEM_THREADS)
        self.assertEqual(table.processes[0].threads, {})
        self.assertEqual((table.thread_count, table.ready_threads), (506, 0))

    def test_an_empty_filter_parses_no_threads(self):
        table = self.parse(thread_pids=())
        self.assertEqual((table.processes[0].threads, table.processes[4].threads), ({}, {}))
        self.assertEqual(table.thread_count, 506)

    def test_every_ready_state_counts_toward_the_run_queue(self):
        table = self.parse(with_system_states(self.sample, (1, 2, 3, 4, 5, 6, 7)))
        self.assertEqual(table.ready_threads, 3)

    def test_the_idle_process_adds_nothing_to_the_run_queue(self):
        # Idle's own entries hold 18 in state 3 (standby), byte 0x44 of each.
        states = [struct.unpack_from("<I", self.sample, 0x100 + n * 0x50 + 0x44)[0]
                  for n in range(IDLE_THREADS)]
        self.assertEqual(states.count(3), 18)
        self.assertEqual(self.parse().ready_threads, 0)

    def test_each_ready_state_of_a_real_process_counts_beside_the_idle_process(self):
        for state in (1, 3, 7):
            with self.subTest(state=state):
                table = self.parse(with_system_states(self.sample, (state,)))
                self.assertEqual(table.ready_threads, 1)

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
        pointer, = struct.unpack_from("<Q", second, 0x40)
        moved = bytearray(second)
        struct.pack_into("<Q", moved, 0x40, pointer + len(padded))
        table = M.parse_table(bytes(padded) + bytes(moved))
        self.assertEqual(table.processes[8].name, "other.exe")


class ReadsAs(unittest.TestCase):
    def test_each_state_number(self):
        words = {state: M.reads_as(state) for state in range(10)}
        self.assertEqual(words, {0: "other", 1: "ready", 2: "running", 3: "ready", 4: "other",
                                 5: "waiting", 6: "other", 7: "ready", 8: "other", 9: "other"})


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

    def test_a_process_first_seen_gains_nothing(self):
        current = M.parse_table(fake_table(TWO_PROCESSES)).processes
        gains = M.deltas({}, current)
        self.assertEqual(gains[7][:4], (0, 0, 0, 0))
        self.assertEqual(gains[7].threads[71], M.ThreadGain(0, 0.0))

    def test_a_thread_first_seen_gains_nothing(self):
        before = M.parse_table(fake_table(TWO_PROCESSES), thread_pids=()).processes
        after = M.parse_table(fake_table(TWO_PROCESSES)).processes
        self.assertEqual(M.deltas(before, after)[7].threads[72], M.ThreadGain(0, 0.0))

    def test_a_reused_pid_gains_nothing(self):
        before = M.parse_table(fake_table(TWO_PROCESSES)).processes
        reborn = [dict(TWO_PROCESSES[0], name="new.exe", user=999_999_999)]
        after = M.parse_table(fake_table(reborn)).processes
        self.assertEqual(M.deltas(before, after)[7].cpu_ms, 0)


def read_csv(folder, name):
    with open(os.path.join(folder, name), newline="", encoding="utf-8") as file:
        return list(csv.reader(file))


def read_rows(folder, name):
    with open(os.path.join(folder, name), newline="", encoding="utf-8") as file:
        return list(csv.DictReader(file))


def sample_row(number=1):
    return M.SampleRow(number, 1000.5, 2.5, 2, 3, 1, None, None, None, RECORDER, 20)


class WriteSample(unittest.TestCase):
    def setUp(self):
        self._dir = tempfile.TemporaryDirectory()
        self.addCleanup(self._dir.cleanup)
        self.folder = self._dir.name
        self.files = M.RecorderFiles(self.folder)
        self.addCleanup(self.files.close)

    def write(self, processes, gains, listed=frozenset()):
        table = M.parse_table(fake_table(processes))
        M.write_sample(self.files, sample_row(), table, gains, listed)

    def test_the_columns_are_pinned_with_one_header_per_file(self):
        self.write(TWO_PROCESSES, {}, frozenset())
        self.write(TWO_PROCESSES, {}, frozenset())
        expected = {
            "samples.csv": "sample,wall_s,sample_ms,processes,threads,ready_threads,clock_s,"
                           "clock_wall_before_s,clock_wall_after_s,recorder_pid,period_ms,"
                           "stop_reason,process_time_s",
            "processes.csv": "sample,pid,parent_pid,name,cpu_ms,read_bytes,write_bytes,hard_faults",
            "threads.csv": "sample,pid,tid,state,wait_reason,reads_as,switches,cpu_ms",
        }
        for name, header in expected.items():
            rows = read_csv(self.folder, name)
            self.assertEqual(",".join(rows[0]), header)
            self.assertEqual(sum(row == rows[0] for row in rows), 1)
        self.assertEqual(read_csv(self.folder, "samples.csv")[1],
                         ["1", "1000.5", "2.5", "2", "3", "1", "", "", "", "901", "20", "", ""])

    def test_a_process_row_needs_a_gain_or_a_listed_pid(self):
        quiet = M.ProcessGain(0.0, 0, 0, 0, {})
        for field, value in (("cpu_ms", 1.5), ("read_bytes", 1), ("write_bytes", 1),
                             ("hard_faults", 1)):
            with self.subTest(field):
                gain = quiet._replace(**{field: value})
                folder = tempfile.mkdtemp(dir=self.folder)
                files = M.RecorderFiles(folder)
                table = M.parse_table(fake_table(TWO_PROCESSES), thread_pids=())
                M.write_sample(files, sample_row(), table, {7: gain, 8: quiet}, frozenset())
                files.close()
                self.assertEqual([row["pid"] for row in read_rows(folder, "processes.csv")], ["7"])
        table = M.parse_table(fake_table(TWO_PROCESSES), thread_pids=())
        M.write_sample(self.files, sample_row(), table, {7: quiet, 8: quiet}, frozenset({8}))
        rows = read_csv(self.folder, "processes.csv")
        self.assertEqual(rows[1:], [["1", "8", "7", "other.exe", "0.0", "0", "0", "0"]])

    def test_a_thread_row_needs_a_non_waiting_state_or_moved_switches(self):
        threads = [(1, 0, 0, 0, 5, 6), (2, 0, 0, 0, 5, 6), (3, 0, 0, 0, 2, 0), (4, 0, 0, 0, 1, 0),
                   (5, 0, 0, 0, 7, 0), (6, 0, 0, 0, 4, 0)]
        processes = [{"pid": 7, "name": "game.exe", "threads": threads}]
        moved = M.ThreadGain(2, 0.5)
        still = M.ThreadGain(0, 0.0)
        gain = M.ProcessGain(0.0, 0, 0, 0, {1: still, 2: moved, 3: still, 4: still, 5: still,
                                            6: still})
        self.write(processes, {7: gain}, frozenset({7}))
        rows = read_rows(self.folder, "threads.csv")
        self.assertEqual([(row["tid"], row["state"], row["reads_as"]) for row in rows],
                         [("2", "5", "waiting"), ("3", "2", "running"), ("4", "1", "ready"),
                          ("5", "7", "ready"), ("6", "4", "other")])
        self.assertEqual((rows[0]["wait_reason"], rows[0]["switches"], rows[0]["cpu_ms"]),
                         ("6", "2", "0.5"))

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
    def lines_at(self, loop, script):
        """A stdin fed by the real reader thread: `script` maps a sample to the
        line sent after it, and the sleep waits until the thread has taken it."""
        lines = queue.Queue()
        taken = threading.Semaphore(0)

        def feed():
            while True:
                line = lines.get()
                if line is None:
                    return
                yield line

        class Counting(M.Inputs):
            def apply(self, line):
                super().apply(line)
                taken.release()

        loop.inputs = Counting()
        thread = threading.Thread(target=M.read_stdin, args=(feed(), loop.inputs, loop.stop),
                                  daemon=True)
        thread.start()

        def on_sleep(sample):
            if sample in script:
                lines.put(script[sample])
                self.assertTrue(taken.acquire(timeout=SUBPROCESS_CAP_S))

        loop.on_sleep = on_sleep
        return lines

    def test_stdin_lines_switch_the_threads_and_the_clock_on_at_their_samples(self):
        reads = []

        def reader(address, size):
            reads.append((address, size))
            return struct.pack("<d", 1.5)

        tables = lambda sample: [dict(TWO_PROCESSES[0]),  # noqa: E731
                                 dict(TWO_PROCESSES[1], pid=PARENT), dict(TWO_PROCESSES[1],
                                                                          pid=RECORDER)]
        loop = FakeLoop(self.folder, tables=tables, open_reader=lambda pid: reader)
        self.lines_at(loop, {2: "pid 7\n", 4: "engine 0x1000\n"})
        reason = loop.run(cap_s=PERIOD_S * 5)
        self.assertEqual(reason, "cap")
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual([row["clock_s"] for row in samples], ["", "", "", "", "1.5", "1.5"])
        self.assertEqual(samples[3]["clock_wall_before_s"], "")
        self.assertEqual(samples[3]["clock_wall_after_s"], "")
        self.assertNotEqual(samples[4]["clock_wall_before_s"], "")
        self.assertNotEqual(samples[4]["clock_wall_after_s"], "")
        self.assertEqual(reads[0], (0x1000 + C.OFF_SONG_CLOCK, 8))
        threads = read_rows(self.folder, "threads.csv")
        self.assertEqual(min(int(row["sample"]) for row in threads), 3)
        self.assertEqual({row["pid"] for row in threads}, {"7", str(PARENT), str(RECORDER)})

    def test_the_game_the_worker_and_the_recorder_always_get_rows(self):
        tables = lambda sample: [dict(TWO_PROCESSES[0]),  # noqa: E731
                                 dict(TWO_PROCESSES[1], pid=PARENT), dict(TWO_PROCESSES[1],
                                                                          pid=RECORDER),
                                 dict(TWO_PROCESSES[1], pid=55)]
        loop = FakeLoop(self.folder, tables=tables, inputs=M.Inputs(7))
        loop.run(cap_s=PERIOD_S * 2)
        rows = [row for row in read_rows(self.folder, "processes.csv") if row["sample"] != "1"]
        self.assertEqual({row["pid"] for row in rows}, {"7", str(PARENT), str(RECORDER)})

    def test_until_the_pid_line_no_process_is_left_out_of_the_gains(self):
        def tables(sample):
            return [dict(TWO_PROCESSES[1], pid=55, user=sample * 10_000)]

        loop = FakeLoop(self.folder, tables=tables)
        loop.run(cap_s=PERIOD_S)
        rows = read_rows(self.folder, "processes.csv")
        self.assertEqual([(row["sample"], row["pid"], row["cpu_ms"]) for row in rows],
                         [("1", "55", "0.0"), ("2", "55", "1.0")])
        self.assertEqual(read_rows(self.folder, "threads.csv"), [])

    def test_the_first_sample_lists_every_process_and_later_ones_keep_the_filter(self):
        idle = [dict(TWO_PROCESSES[1], pid=pid, parent=pid - 1) for pid in (40, 41, 42)]
        loop = FakeLoop(self.folder, tables=lambda sample: idle)
        loop.run(cap_s=PERIOD_S * 2)
        rows = read_rows(self.folder, "processes.csv")
        self.assertEqual([(row["sample"], row["pid"], row["parent_pid"]) for row in rows],
                         [("1", "40", "39"), ("1", "41", "40"), ("1", "42", "41")])

    def test_the_before_stamp_is_the_samples_own_wall_stamp(self):
        # A wall clock that moves 1 s at every call, so a second stamp would show.
        loop = FakeLoop(self.folder, inputs=M.Inputs(7),
                        open_reader=lambda pid: lambda address, size: struct.pack("<d", 2.5))
        loop.inputs.engine = 0x1000
        calls = []

        def wall():
            calls.append(1)
            return float(len(calls))

        seams = M.Seams(loop.read_table, loop.open_reader, wall=wall, monotonic=lambda: loop.time,
                        sleep=loop.sleep, process_time=lambda: 1.25)
        files = M.RecorderFiles(self.folder)
        try:
            M.record(files, loop.inputs, loop.stop, seams, period_s=PERIOD_S, parent_pid=PARENT,
                     recorder_pid=RECORDER, cap_s=PERIOD_S)
        finally:
            files.close()
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual([(row["wall_s"], row["clock_wall_before_s"], row["clock_wall_after_s"])
                          for row in samples], [("1.0", "1.0", "2.0"), ("3.0", "3.0", "4.0")])

    def test_a_short_clock_read_leaves_the_column_empty(self):
        loop = FakeLoop(self.folder, inputs=M.Inputs(7),
                        open_reader=lambda pid: lambda address, size: b"\0\0\0\0")
        loop.inputs.engine = 0x1000
        loop.run(cap_s=PERIOD_S)
        self.assertEqual({row["clock_s"] for row in read_rows(self.folder, "samples.csv")}, {""})

    def test_a_closed_stdin_stops_within_one_period(self):
        seen = {}
        loop = FakeLoop(self.folder)

        def close(sample):
            if sample == 3:
                M.read_stdin([], loop.inputs, loop.stop)
                seen["at"] = loop.time

        loop.on_sleep = close
        loop.on_table = lambda sample: seen.setdefault("last", {}).update({sample: loop.time})
        self.assertEqual(loop.run(), "stdin closed")
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual(len(samples), 4)
        self.assertEqual(samples[-1]["stop_reason"], "stdin closed")
        self.assertLessEqual(seen["last"][4] - seen["at"], PERIOD_S)

    def test_a_signalled_parent_handle_stops_within_one_period(self):
        seen = {}
        signalled = threading.Event()
        loop = FakeLoop(self.folder)
        watcher = threading.Thread(target=M.watch_parent, args=(signalled.wait, loop.stop),
                                   daemon=True)
        watcher.start()

        def die(sample):
            if sample == 3:
                signalled.set()
                watcher.join(SUBPROCESS_CAP_S)
                seen["at"] = loop.time

        loop.on_sleep = die
        loop.on_table = lambda sample: seen.update({sample: loop.time})
        self.assertEqual(loop.run(), "parent gone")
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual(samples[-1]["stop_reason"], "parent gone")
        self.assertLessEqual(seen[4] - seen["at"], PERIOD_S)

    def test_the_cap_stops_it_and_the_last_row_names_it(self):
        loop = FakeLoop(self.folder)
        self.assertEqual(loop.run(cap_s=PERIOD_S * 3), "cap")
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual([row["stop_reason"] for row in samples], ["", "", "", "cap"])

    def test_seconds_stops_it_and_the_earlier_of_two_ends_is_named(self):
        loop = FakeLoop(self.folder)
        self.assertEqual(loop.run(cap_s=PERIOD_S * 3, seconds=PERIOD_S * 2), "seconds")

    def test_the_first_end_to_arrive_is_kept(self):
        stop = M.Stop()
        stop.set("parent gone")
        stop.set("stdin closed")
        self.assertEqual(stop.reason, "parent gone")

    def test_each_row_carries_the_pid_and_period_and_only_the_last_the_process_time(self):
        loop = FakeLoop(self.folder)
        loop.run(cap_s=PERIOD_S * 2)
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual({row["recorder_pid"] for row in samples}, {str(RECORDER)})
        self.assertEqual({row["period_ms"] for row in samples}, {"15.625"})
        self.assertEqual([row["sample"] for row in samples], ["1", "2", "3"])
        self.assertEqual([row["process_time_s"] for row in samples], ["", "", "1.25"])

    def test_a_clock_read_that_raises_leaves_the_column_empty_and_goes_on(self):
        calls = []

        def reader(address, size):
            calls.append(address)
            if len(calls) == 2:
                raise OSError("the game went away")
            return struct.pack("<d", 2.5)

        inputs = M.Inputs(7)
        inputs.engine = 0x1000
        loop = FakeLoop(self.folder, inputs=inputs, open_reader=lambda pid: reader)
        loop.run(cap_s=PERIOD_S * 2)
        samples = read_rows(self.folder, "samples.csv")
        self.assertEqual([row["clock_s"] for row in samples], ["2.5", "", "2.5"])
        self.assertEqual(samples[1]["clock_wall_before_s"], "")
        self.assertEqual(samples[1]["clock_wall_after_s"], "")

    def test_a_game_that_cannot_be_opened_leaves_the_column_empty_and_goes_on(self):
        def refuse(pid):
            raise OSError("no such process")

        inputs = M.Inputs(7)
        inputs.engine = 0x1000
        loop = FakeLoop(self.folder, inputs=inputs, open_reader=refuse)
        self.assertEqual(loop.run(cap_s=PERIOD_S), "cap")
        self.assertEqual([row["clock_s"] for row in read_rows(self.folder, "samples.csv")],
                         ["", ""])

    def test_a_short_clock_read_leaves_the_column_empty(self):
        inputs = M.Inputs(7)
        inputs.engine = 0x1000
        loop = FakeLoop(self.folder, inputs=inputs, open_reader=lambda pid: lambda a, s: b"\0")
        loop.run(cap_s=PERIOD_S)
        self.assertEqual([row["clock_s"] for row in read_rows(self.folder, "samples.csv")],
                         ["", ""])

    def test_a_late_sample_skips_ticks_and_does_not_burst(self):
        waits = []
        loop = FakeLoop(self.folder)
        real_table = loop.read_table

        def slow_second_table():
            result = real_table()
            if loop.sample == 2:
                loop.time += PERIOD_S * 5
            return result

        loop.read_table = slow_second_table
        real_sleep = loop.sleep
        loop.sleep = lambda seconds: (waits.append(seconds), real_sleep(seconds))
        loop.run(cap_s=PERIOD_S * 8)
        self.assertEqual(waits[1], 0)
        self.assertAlmostEqual(waits[2], PERIOD_S)

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


class FakeFile:
    """A file that holds writes until flush, as a buffered file does."""

    def __init__(self, disk, path):
        self.disk = disk
        self.path = path
        self.waiting = ""
        disk[path] = ""

    def write(self, text):
        self.waiting += text
        return len(text)

    def flush(self):
        self.disk[self.path] += self.waiting
        self.waiting = ""

    def close(self):
        self.flush()


class OnDisk(Folder):
    def test_each_samples_rows_are_on_disk_before_the_next_sample(self):
        disk = {}
        files = M.RecorderFiles("x", open_file=lambda path, *a, **k: FakeFile(disk, path))
        seen = []

        def tables(sample):
            seen.append({os.path.basename(path): text.count("\n") for path, text in disk.items()})
            return [dict(TWO_PROCESSES[0], user=100_000 + sample * 10_000)]

        inputs = M.Inputs(7)
        FakeLoop(self.folder, tables=tables, inputs=inputs).run(files, cap_s=PERIOD_S * 2)
        # Header only, then one more row in samples.csv and processes.csv per sample.
        self.assertEqual([view["samples.csv"] for view in seen], [1, 2, 3])
        self.assertEqual([view["processes.csv"] for view in seen], [1, 2, 3])
        self.assertEqual([view["threads.csv"] for view in seen], [1, 2, 3])


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
        with self.assertRaisesRegex(OSError, "0xc0000022"):
            M.read_table(M.TableBuffer(), lambda *args: 0xC0000022)

    def test_a_table_past_the_ulong_limit_stops_the_growing(self):
        def query(information_class, address, size, needed):
            needed._obj.value = 0xFFFFFFFF
            return M._STATUS_INFO_LENGTH_MISMATCH

        held = M.TableBuffer()
        held.raw = ctypes.create_string_buffer(1)
        with unittest.mock.patch.object(M.ctypes, "create_string_buffer",
                                        lambda size: type("Big", (), {"__len__": lambda s: size})()):
            with unittest.mock.patch.object(M.ctypes, "addressof", lambda raw: 0):
                with self.assertRaisesRegex(OSError, "more than one call"):
                    M.read_table(held, query)


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

    def test_the_numbers_the_plan_fixes(self):
        self.assertEqual((M.RATE_PER_S, M.RECORDER_CAP_EXTRA_S), (50, 60))

    def test_the_rate_defaults_to_the_one_constant(self):
        self.assertEqual(self.run_main("--cap-s", "9")["period_s"], 0.02)

    def test_cap_s_is_the_exact_cap(self):
        self.assertEqual(self.run_main("--cap-s", "9")["cap_s"], 9)

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
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], "stdin closed")

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
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], "cap")

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
        self.assertEqual(read_rows(out, "samples.csv")[-1]["stop_reason"], "parent gone")

    def test_a_parent_that_cannot_be_opened_fails_loudly(self):
        # A finished child's pid, which no live process holds any more.
        child = subprocess.run([sys.executable, "-I", "-c", "import os; print(os.getpid())"],
                               capture_output=True, text=True, timeout=SUBPROCESS_CAP_S, check=True)
        _, process = self.start("--cap-s", "1", parent=int(child.stdout))
        _, stderr = process.communicate(timeout=SUBPROCESS_CAP_S)
        self.assertNotEqual(process.returncode, 0)
        self.assertIn("OpenProcess failed for parent pid", stderr)


class ClockRange(Folder):
    def test_a_nonfinite_clock_is_no_clock_and_a_negative_one_is_kept(self):
        # The game's clock is below zero through every song's lead-in (TM-11), so a
        # negative read is a real clock (D325).
        for value, written in ((float("nan"), ""), (float("inf"), ""), (float("-inf"), ""),
                               (-1.0, "-1.0"), (12.5, "12.5")):
            with self.subTest(value=value):
                folder = os.path.join(self.folder, repr(value))
                os.mkdir(folder)
                inputs = M.Inputs(7)
                inputs.engine = 0x1000
                raw = struct.pack("<d", value)
                loop = FakeLoop(folder, inputs=inputs,
                                open_reader=lambda pid, raw=raw: lambda address, size: raw)
                loop.run(cap_s=PERIOD_S)
                samples = read_rows(folder, "samples.csv")
                self.assertEqual([row["clock_s"] for row in samples], [written, written])
                stamped = written != ""
                self.assertEqual([row["clock_wall_before_s"] != "" for row in samples],
                                 [stamped, stamped])


if __name__ == "__main__":
    unittest.main()
