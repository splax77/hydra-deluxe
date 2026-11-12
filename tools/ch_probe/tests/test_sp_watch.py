"""Tests for the Star Power watcher (experiments/sp_watch.py) and the Star
Power fields engine.decode_snapshot reads.

No game: a real Process over fake memory (tests/fakes.py) answers the reads,
a fake wall clock moves only when the code sleeps, and the engine search is
the real capped finder over a fake scan that never finds a song.
"""

from __future__ import annotations

import contextlib
import csv
import dataclasses
import io
import math
import os
import struct
import sys
import tempfile
import time
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

    def test_the_watch_writes_the_half_column(self):
        # The game's half sits at +0x1B8 (SP-16), written as a literal so the
        # test also pins constants.OFF_SP_HALF.
        block = bytearray(sp_block())
        struct.pack_into("<Q", block, 0x1B8, 7200)
        snap = engine.decode_snapshot(bytes(block))
        self.assertEqual(snap.sp_half_u64, 7200)
        row = dict(zip(WATCH_COLUMNS, sp_watch.watch_row(5.0, snap)))
        self.assertEqual(row["sp_half_u64"], 7200)
        self.assertEqual(row["sp_max_u64"], 8589940736)


PLAYER = 0x3_0000_2000
MANAGER = 0x3_0000_3000
CLOCK = 0x3_0000_4000


def clock_memory(*, listener=PLAYER, manager=MANAGER, clock=CLOCK):
    """Fake memory holding the walk from the engine to the clock object, with
    known values at the clock's fields. The offsets are written as literals
    so the test also pins constants.py. The correction is a float in the game
    (dump.cs), and 0.25 is exact at that width."""
    block = bytearray(sp_block())
    struct.pack_into("<Q", block, 0x1A0, listener)
    player = bytearray(0x28)
    struct.pack_into("<Q", player, 0x20, manager)
    game = bytearray(0xB8)
    struct.pack_into("<Q", game, 0xB0, clock)
    sync = bytearray(0x40)
    struct.pack_into("<f", sync, 0x18, 0.25)
    struct.pack_into("<d", sync, 0x20, 12.75)
    struct.pack_into("<d", sync, 0x38, 12.5)
    return {OBJ: bytes(block), PLAYER + 0x20: bytes(player[0x20:]),
            MANAGER + 0xB0: bytes(game[0xB0:]), CLOCK: bytes(sync)}


def model_on(memory):
    """An EngineModel on `memory` and the fake memory that records its reads."""
    proc, mem = fakes.fake_process(memory, BASE, consts=GOOD_CONSTS)
    model = engine.EngineModel(proc)
    model.use_object(OBJ)
    return model, mem


class ClockColumnsTest(unittest.TestCase):
    def test_the_walk_reads_the_three_clock_fields(self):
        snap = model_on(clock_memory())[0].snapshot()
        self.assertEqual((snap.audio_s, snap.raw_s, snap.corr_s),
                         (12.75, 12.5, 0.25))

    def test_a_sample_costs_three_reads_past_the_engine_block(self):
        model, mem = model_on(clock_memory())
        model.snapshot()
        self.assertEqual(mem.reads, [(OBJ, 0x2E8), (PLAYER + 0x20, 8),
                                     (MANAGER + 0xB0, 8), (CLOCK, 0x40)])

    def test_the_walk_writes_nothing(self):
        model, mem = model_on(clock_memory())
        model.snapshot()
        self.assertEqual(mem.writes, {})

    def assert_blank(self, snap):
        self.assertTrue(all(math.isnan(v) for v in
                            (snap.audio_s, snap.raw_s, snap.corr_s)))
        self.assertEqual(snap.clock_s, 12.5)

    def test_a_null_at_any_hop_leaves_the_columns_blank(self):
        for hop in ("listener", "manager", "clock"):
            with self.subTest(hop=hop):
                self.assert_blank(
                    model_on(clock_memory(**{hop: 0}))[0].snapshot())

    def test_a_failed_read_past_the_engine_leaves_the_columns_blank(self):
        for gone in (PLAYER + 0x20, MANAGER + 0xB0, CLOCK):
            with self.subTest(gone=hex(gone)):
                memory = clock_memory()
                del memory[gone]
                self.assert_blank(model_on(memory)[0].snapshot())

    def test_a_failed_engine_read_still_raises(self):
        memory = clock_memory()
        del memory[OBJ]
        with self.assertRaises(OSError):
            model_on(memory)[0].snapshot()

    def test_decoding_the_engine_block_alone_leaves_the_columns_blank(self):
        snap = engine.decode_snapshot(sp_block())
        self.assertTrue(math.isnan(snap.audio_s))

    def test_a_blank_clock_does_not_end_the_watch(self):
        memory = clock_memory()
        del memory[CLOCK]
        model, _ = model_on(memory)
        clock = FakeClock()
        rows = []
        summary = sp_watch.watch(model.snapshot, rows.append, now=clock.now,
                                 sleep=clock.sleep)
        self.assertEqual(
            summary.reason,
            "clock frozen at 12.50 s for over 5 s (song quit or paused)")
        self.assertEqual(summary.rows, 2)

    def test_the_three_reads_cost_under_a_tenth_of_the_poll(self):
        samples = 1000
        with_clock = model_on(clock_memory())[0]
        without = model_on({OBJ: sp_block()})[0]

        def seconds_per_sample(model):
            start = time.perf_counter()
            for _ in range(samples):
                model.snapshot()
            return (time.perf_counter() - start) / samples

        # The approved test bound: 10 percent of the poll (the plan's T4).
        extra = seconds_per_sample(with_clock) - seconds_per_sample(without)
        self.assertLess(extra, 0.10 * sp_watch.POLL_S)


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

    def test_should_stop_ends_the_run_after_two_polls(self):
        rows = []
        clock = FakeClock()
        asked = []

        def should_stop():
            asked.append(True)
            return len(asked) > 2

        summary = sp_watch.watch(
            snapshots([snap_at(1.0), snap_at(1.01), snap_at(1.02)]),
            rows.append, now=clock.now, sleep=clock.sleep,
            should_stop=should_stop)
        self.assertEqual(summary.reason, "stopped")
        self.assertEqual(clock.sleeps, [0.004] * 2)
        self.assertEqual([r[1] for r in rows], [1.0, 1.01, 1.02])
        self.assertEqual(summary.rows, 3)

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

    def test_the_default_search_prints_its_dots_to_out_only(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)
        out, stdout = io.StringIO(), io.StringIO()
        with fakes.searching_for_no_song(F), contextlib.redirect_stdout(stdout):
            code = sp_watch.run(["--out", self.out_csv],
                                opener=self.opener_for(proc), out=out)
        self.assertEqual(code, 2)
        self.assertEqual(stdout.getvalue(), "")
        self.assertIn("....", out.getvalue())

    def test_find_engine_live_sends_its_dots_to_the_out_it_is_given(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)
        quiet, stdout = io.StringIO(), io.StringIO()
        with fakes.searching_for_no_song(F), contextlib.redirect_stdout(stdout):
            with self.assertRaises(F.EngineSearchTimeout):
                sp_watch.find_engine_live(proc, out=quiet)
            self.assertEqual(stdout.getvalue(), "")
            with self.assertRaises(F.EngineSearchTimeout):
                sp_watch.find_engine_live(proc)
        self.assertIn("....", quiet.getvalue())
        self.assertIn("....", stdout.getvalue())

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
                                        "6144", "8589940736", "0", "1", "257", "8",
                                        "520", "61.25", "1", "0.0",
                                        "nan", "nan", "nan"])
        text = out.getvalue()
        self.assertIn("  Rows written: 2\n", text)
        self.assertIn("  Engine: 0x200001000\n", text)
        self.assertIn("  First clock: 12.500 s, last clock: 12.500 s\n", text)
        self.assertIn("clock frozen at 12.50 s", text)
        self.assertEqual(mem.writes, {})

    def test_a_whole_run_logs_the_clock_columns_in_every_row(self):
        proc, mem = fakes.fake_process(clock_memory(), BASE, consts=GOOD_CONSTS)
        clock = FakeClock()
        code = sp_watch.run(["--out", self.out_csv],
                            opener=self.opener_for(proc),
                            find_engine=lambda p: OBJ,
                            now=clock.now, sleep=clock.sleep, out=io.StringIO())
        self.assertEqual(code, 0)
        with open(self.out_csv, newline="", encoding="utf-8") as f:
            table = list(csv.reader(f))
        self.assertEqual(table[0][-3:], ["audio_s", "raw_s", "corr_s"])
        self.assertEqual(len(table), 3)   # header, first row, final row
        for row in table[1:]:
            self.assertEqual(row[-3:], ["12.75", "12.5", "0.25"])
        self.assertEqual(mem.writes, {})


if __name__ == "__main__":
    unittest.main()
