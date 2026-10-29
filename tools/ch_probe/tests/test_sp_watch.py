"""Tests for the Star Power watcher (experiments/sp_watch.py) and the Star
Power fields engine.decode_snapshot reads.

No game: a real Process over fake memory (tests/fakes.py) answers the reads,
a fake wall clock moves only when the code sleeps, and the engine search is
the real capped finder over a fake scan that never finds a song.
"""

from __future__ import annotations

import csv
import dataclasses
import io
import os
import struct
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine  # noqa: E402
from tools.ch_probe import engine_finder as F  # noqa: E402
from tools.ch_probe.experiments import sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

BASE = 0x180000000
OBJ = 0x2_0000_1000

# The build check's two window constants, as the game stores them.
GOOD_CONSTS = {C.RVA_CONST_NORMAL_BACK: 0.085, C.RVA_CONST_NORMAL_FRONT: 0.0375}


class FakeClock:
    """A wall clock that moves only when the code under test sleeps."""

    def __init__(self, start=1000.0, step=None):
        self.t = start
        self.step = step          # when set, every sleep moves this far
        self.sleeps = []

    def now(self):
        return self.t

    def sleep(self, seconds):
        self.sleeps.append(seconds)
        self.t += seconds if self.step is None else self.step


def sp_block():
    """A 0x2E8-byte engine object with every Star Power field set. The
    offsets are written as literals so the test also pins constants.py.
    Each two-width field has bytes past its low 32 bits (or past its low
    byte) set, so each width reads a different number."""
    block = bytearray(0x2E8)
    struct.pack_into("<d", block, 0x100, 12.5)               # song clock
    struct.pack_into("<II", block, 0x68, 7680, 1)            # SP amount
    struct.pack_into("<II", block, 0x1B0, 6144, 2)           # SP maximum
    struct.pack_into("<I", block, 0x42, 0x0101)              # SP active
    struct.pack_into("<I", block, 0x48, 0x0208)              # multiplier
    struct.pack_into("<d", block, 0x120, 61.25)              # SP end time
    struct.pack_into("<I", block, 0x130, 1)                  # overflow
    return bytes(block)


class DecodeSnapshotTest(unittest.TestCase):
    def test_the_block_is_the_snapshot_size(self):
        self.assertEqual(engine.SNAPSHOT_SIZE, 0x2E8)

    def test_every_star_power_field_at_each_width(self):
        snap = engine.decode_snapshot(sp_block())
        self.assertEqual(snap.sp_amount_u32, 7680)
        self.assertEqual(snap.sp_amount_u64, 4294974976)
        self.assertEqual(snap.sp_max_u32, 6144)
        self.assertEqual(snap.sp_max_u64, 8589940736)
        self.assertEqual(snap.sp_active_byte, 1)
        self.assertEqual(snap.sp_active_u32, 257)
        self.assertEqual(snap.sp_mult_byte, 8)
        self.assertEqual(snap.sp_mult_u32, 520)
        self.assertEqual(snap.sp_end_s, 61.25)
        self.assertEqual(snap.sp_overflow_u32, 1)
        self.assertEqual(snap.clock_s, 12.5)

    def test_every_logged_column_but_the_wall_time_is_a_snapshot_field(self):
        names = {f.name for f in dataclasses.fields(engine.Snapshot)}
        self.assertEqual(
            [c for c in WATCH_COLUMNS if c not in names], ["wall_s"])


def snap_at(clock_s, **fields):
    """A snapshot of an all-zero object with its clock and any field set."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    return dataclasses.replace(base, clock_s=clock_s, **fields)


def snapshots(seq):
    """A snapshot callable serving `seq` in order; an exception in the list
    is raised instead of returned."""
    items = list(seq)

    def read():
        item = items.pop(0)
        if isinstance(item, Exception):
            raise item
        return item
    return read


class WatchTest(unittest.TestCase):
    def test_writes_a_row_only_when_a_logged_field_changes(self):
        rows = []
        clock = FakeClock()
        summary = sp_watch.watch(
            snapshots([snap_at(1.0), snap_at(1.0), snap_at(1.01),
                       snap_at(1.01, sp_amount_u32=96),
                       OSError("gone")]),
            rows.append, now=clock.now, sleep=clock.sleep)
        self.assertEqual([r[1] for r in rows], [1.0, 1.01, 1.01])
        self.assertEqual([r[3] for r in rows], [0, 0, 96])
        self.assertEqual(summary.rows, 3)
        self.assertEqual(summary.reason, "lost the engine (gone)")
        self.assertEqual(clock.sleeps, [0.004] * 4)

    def test_a_frozen_clock_ends_the_run_with_a_final_row(self):
        rows = []
        clock = FakeClock()
        summary = sp_watch.watch(lambda: snap_at(3.0), rows.append,
                                 now=clock.now, sleep=clock.sleep)
        self.assertEqual(summary.reason,
                         "clock frozen at 3.00 s for over 5 s (song quit or paused)")
        self.assertEqual(summary.rows, 2)
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0][0], 1000.0)
        self.assertGreater(rows[1][0] - rows[0][0], 5.0)
        self.assertEqual(rows[1][1], 3.0)

    def test_a_clock_that_jumps_back_ends_the_run_with_a_final_row(self):
        rows = []
        clock = FakeClock()
        summary = sp_watch.watch(
            snapshots([snap_at(30.0), snap_at(30.01), snap_at(2.0)]),
            rows.append, now=clock.now, sleep=clock.sleep)
        self.assertEqual(summary.reason,
                         "clock jumped back (30.01 -> 2.00 s)")
        self.assertEqual([r[1] for r in rows], [30.0, 30.01, 2.0])
        self.assertEqual(summary.last_clock_s, 2.0)

    def test_the_wall_clock_cap_ends_a_run_whose_clock_keeps_moving(self):
        rows = []
        clock = FakeClock(step=1.0)
        ticks = iter(range(10_000))
        summary = sp_watch.watch(lambda: snap_at(float(next(ticks))),
                                 rows.append, now=clock.now, sleep=clock.sleep)
        self.assertEqual(summary.reason, "run cap of 240 s reached")
        self.assertEqual(clock.t, 1240.0)
        self.assertEqual(summary.rows, 241)
        self.assertEqual(summary.last_clock_s, 240.0)

    def test_poll_and_cap_literals(self):
        self.assertEqual(sp_watch.POLL_S, 0.004)
        self.assertEqual(sp_watch.RUN_CAP_S, 240.0)


class RunTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.out_csv = os.path.join(self.tmp.name, "watch.csv")
        self.opened = []

    def tearDown(self):
        self.tmp.cleanup()

    def opener_for(self, proc):
        def opener(**kwargs):
            self.opened.append(kwargs)
            return proc
        return opener

    def test_no_song_exits_2_with_the_finder_message(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)
        clock = FakeClock()

        def never_finds(p):
            return F.find_live_engine_capped(
                p, [(b"b", b"f")], scan=lambda *_: ([], 0, 0),
                sleep=clock.sleep, now=clock.now, out=io.StringIO())

        out = io.StringIO()
        code = sp_watch.run(["--out", self.out_csv],
                            opener=self.opener_for(proc),
                            find_engine=never_finds, out=out)
        self.assertEqual(code, 2)
        self.assertIn(
            "no playing song found in 120 s; start the song and run again",
            out.getvalue())
        self.assertGreaterEqual(clock.t - 1000.0, 120.0)
        self.assertFalse(os.path.exists(self.out_csv))

    def test_opens_the_game_read_only(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)

        def no_song(p):
            raise F.EngineSearchTimeout("no song")

        sp_watch.run(["--out", self.out_csv], opener=self.opener_for(proc),
                     find_engine=no_song, out=io.StringIO())
        self.assertEqual(self.opened, [{"read_only": True}])

    def test_a_whole_run_writes_the_header_and_prints_the_summary(self):
        proc, mem = fakes.fake_process({OBJ: sp_block()}, BASE,
                                       consts=GOOD_CONSTS)
        clock = FakeClock()
        out = io.StringIO()
        code = sp_watch.run(["--out", self.out_csv],
                            opener=self.opener_for(proc),
                            find_engine=lambda p: OBJ,
                            now=clock.now, sleep=clock.sleep, out=out)
        self.assertEqual(code, 0)
        with open(self.out_csv, newline="", encoding="utf-8") as f:
            table = list(csv.reader(f))
        self.assertEqual(tuple(table[0]), WATCH_COLUMNS)
        self.assertEqual(len(table), 3)   # header, first row, final row
        self.assertEqual(table[1][1:], ["12.5", "0", "7680", "4294974976",
                                        "6144", "8589940736", "1", "257", "8",
                                        "520", "61.25", "1", "0.0"])
        text = out.getvalue()
        self.assertIn("  Rows written: 2\n", text)
        self.assertIn("  Engine: 0x200001000\n", text)
        self.assertIn("  First clock: 12.500 s, last clock: 12.500 s\n", text)
        self.assertIn("clock frozen at 12.50 s", text)
        self.assertEqual(mem.writes, {})


if __name__ == "__main__":
    unittest.main()
