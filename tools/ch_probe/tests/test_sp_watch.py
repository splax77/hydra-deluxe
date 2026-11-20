"""Tests for the Star Power watcher (experiments/sp_watch.py) and the Star
Power fields engine.decode_snapshot reads.

No game. The watch's row and its frozen-clock end run on the real clock over
reads played back from a real watch CSV under testdata/ch_runs. The other
tests still use a real Process over fake memory (tests/fakes.py), a fake wall
clock that moves only when the code sleeps, and the real capped engine
finder over a fake scan that never finds a song.
"""

from __future__ import annotations

import contextlib
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
from tools.ch_probe.experiments import live, sp_compare, sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

# The end of a real Star Power watch: the last three reads before the song
# ended, and the final row the watch wrote once the clock had held for over
# live.STALL_S. Copied from lines 1 and 50242-50245 of
# hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-run1\sp_watch-1.csv.
FROZEN_END_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                              "pitch-dark-ratios-run1", "sp_watch-1-frozen-end.csv")


def recorded_snapshots(path):
    """Each row of a recorded watch CSV as the Snapshot the watch read then:
    every field the watch logs, as sp_compare.load_watch_rows reads it, and
    the fields it does not log left zero."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    names = {f.name for f in dataclasses.fields(engine.Snapshot)}
    return [dataclasses.replace(base, **{c: v for c, v in row.items() if c in names})
            for row in sp_compare.load_watch_rows(path)]


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

    def test_a_row_writes_every_column_of_the_recorded_read(self):
        # Each recorded row, read back as the Snapshot the watch read then,
        # writes the same row again, the game's own half column among them.
        recorded = sp_compare.load_watch_rows(FROZEN_END_CSV)
        for want, snap in zip(recorded, recorded_snapshots(FROZEN_END_CSV)):
            row = dict(zip(WATCH_COLUMNS,
                           sp_watch.watch_row(want[sp_watch.WALL_COLUMN], snap)))
            self.assertEqual(row, want)
            self.assertGreater(row["sp_half_u64"], 0)


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
    def test_a_sample_costs_three_reads_past_the_engine_block(self):
        model, mem = model_on(clock_memory())
        model.snapshot()
        self.assertEqual(mem.reads, [(OBJ, 0x2E8), (PLAYER + 0x20, 8),
                                     (MANAGER + 0xB0, 8), (CLOCK, 0x40)])


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
        # The recorded reads before the run's final row, then the last of them
        # again and again, as a game whose song has ended answers. The watch
        # runs on the real clock. Its cap ends the run, and the test fails, if
        # the freeze is never seen.
        recorded = sp_compare.load_watch_rows(FROZEN_END_CSV)
        reads = recorded_snapshots(FROZEN_END_CSV)[:-1]
        served = []

        def serve():
            served.append(True)
            return reads[min(len(served), len(reads)) - 1]

        rows = []
        summary = sp_watch.watch(serve, rows.append, cap_s=2 * live.STALL_S)
        last = reads[-1].clock_s
        with self.assertRaises(live.ClockFrozen) as frozen:
            live.StoppedCheck(last, 0.0).check(last, 2 * live.STALL_S)
        self.assertEqual(summary.reason, str(frozen.exception))
        self.assertEqual(summary.rows, len(recorded))
        written = [dict(zip(WATCH_COLUMNS, r)) for r in rows]

        def logged(table):
            return [{c: v for c, v in r.items() if c != sp_watch.WALL_COLUMN}
                    for r in table]
        self.assertEqual(logged(written), logged(recorded))
        # The run ends the way the recorded run ended: a final row with the
        # last clock, more than STALL_S of wall time after the row before it.
        for table in (written, recorded):
            wall = [r[sp_watch.WALL_COLUMN] for r in table]
            self.assertGreater(wall[-1] - wall[-2], live.STALL_S)
            self.assertEqual(table[-1]["clock_s"], last)

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
