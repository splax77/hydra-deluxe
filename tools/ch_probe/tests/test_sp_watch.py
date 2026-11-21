"""Tests for the Star Power watcher (experiments/sp_watch.py) and the Star
Power fields engine.decode_snapshot reads.

No game. The watch's row, its frozen-clock end, its wall-time cap and its
should_stop run on the real clock over reads played back from real watch CSVs
under testdata/ch_runs. The read-only open goes through the real opener and
the real process list. The row-on-change test runs on the real clock over
hand-built snapshots. The two progress-dot tests use a real
Process over fake memory (tests/fakes.py) and the real capped engine finder
over a fake scan that never finds a song.
"""

from __future__ import annotations

import contextlib
import dataclasses
import io
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
from tools.ch_probe import process as P  # noqa: E402
from tools.ch_probe.experiments import live, sp_compare, sp_watch  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

# The end of a real Star Power watch: the last three reads before the song
# ended, and the final row the watch wrote once the clock had held for over
# live.STALL_S. Copied from lines 1 and 50242-50245 of
# hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-run1\sp_watch-1.csv.
FROZEN_END_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                              "pitch-dark-ratios-run1", "sp_watch-1-frozen-end.csv")

# A real watch's reads while a song played, every row a change and the clock
# never going back: the same run's sp_watch-1-act3.csv, whose source
# test_sp_compare.py names.
ACT3_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                        "pitch-dark-ratios-run1", "sp_watch-1-act3.csv")


def recorded_snapshots(path):
    """Each row of a recorded watch CSV as the Snapshot the watch read then:
    every field the watch logs, as sp_compare.load_watch_rows reads it, and
    the fields it does not log left zero."""
    base = engine.decode_snapshot(bytes(engine.SNAPSHOT_SIZE))
    names = {f.name for f in dataclasses.fields(engine.Snapshot)}
    return [dataclasses.replace(base, **{c: v for c, v in row.items() if c in names})
            for row in sp_compare.load_watch_rows(path)]


BASE = 0x180000000

# The build check's two window constants, as the game stores them.
GOOD_CONSTS = {C.RVA_CONST_NORMAL_BACK: 0.085, C.RVA_CONST_NORMAL_FRONT: 0.0375}


# The values sp_block packs. The low parts are the game's own, from the
# first row of the recorded act3 watch; the active flag is set, as in Star
# Power. Each two-width field also has bytes past its low 32 bits (or past
# its low byte) set, so each width reads a different number.
_RECORDED = sp_compare.load_watch_rows(ACT3_CSV)[0]
CLOCK_S = _RECORDED["clock_s"]
AMOUNT_LO, AMOUNT_HI = _RECORDED["sp_amount_u32"], 1
MAX_LO, MAX_HI = _RECORDED["sp_max_u32"], 2
ACTIVE_BYTE, ACTIVE_NEXT_BYTE = 1, 1
MULT_BYTE, MULT_NEXT_BYTE = _RECORDED["sp_mult_byte"], 2
END_S = 61.25
OVERFLOW = _RECORDED["sp_overflow_u32"]


def sp_block():
    """A 0x2E8-byte engine object with every Star Power field set. The
    offsets are written as literals so the test also pins constants.py."""
    block = bytearray(0x2E8)
    struct.pack_into("<d", block, 0x100, CLOCK_S)                    # song clock
    struct.pack_into("<II", block, 0x68, AMOUNT_LO, AMOUNT_HI)       # SP amount
    struct.pack_into("<II", block, 0x1B0, MAX_LO, MAX_HI)            # SP maximum
    struct.pack_into("<BB", block, 0x42, ACTIVE_BYTE, ACTIVE_NEXT_BYTE)  # SP active
    struct.pack_into("<BB", block, 0x48, MULT_BYTE, MULT_NEXT_BYTE)  # multiplier
    struct.pack_into("<d", block, 0x120, END_S)                      # SP end time
    struct.pack_into("<I", block, 0x130, OVERFLOW)                   # overflow
    return bytes(block)


class DecodeSnapshotTest(unittest.TestCase):
    def test_every_star_power_field_at_each_width(self):
        # A 64-bit read takes the high word too; a 32-bit read of a flag
        # takes the byte after it; a byte read takes the low byte only.
        snap = engine.decode_snapshot(sp_block())
        self.assertEqual(snap.sp_amount_u32, AMOUNT_LO)
        self.assertEqual(snap.sp_amount_u64, AMOUNT_LO + (AMOUNT_HI << 32))
        self.assertEqual(snap.sp_max_u32, MAX_LO)
        self.assertEqual(snap.sp_max_u64, MAX_LO + (MAX_HI << 32))
        self.assertEqual(snap.sp_active_byte, ACTIVE_BYTE)
        self.assertEqual(snap.sp_active_u32, ACTIVE_BYTE + (ACTIVE_NEXT_BYTE << 8))
        self.assertEqual(snap.sp_mult_byte, MULT_BYTE)
        self.assertEqual(snap.sp_mult_u32, MULT_BYTE + (MULT_NEXT_BYTE << 8))
        self.assertEqual(snap.sp_end_s, END_S)
        self.assertEqual(snap.sp_overflow_u32, OVERFLOW)
        self.assertEqual(snap.clock_s, CLOCK_S)

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
        # The watch runs on the real clock; the sleep it asks for is recorded
        # and then slept.
        rows, slept = [], []

        def sleep(seconds):
            slept.append(seconds)
            time.sleep(seconds)

        summary = sp_watch.watch(
            snapshots([snap_at(1.0), snap_at(1.0), snap_at(1.01),
                       snap_at(1.01, sp_amount_u32=96),
                       OSError("gone")]),
            rows.append, sleep=sleep)
        written = [dict(zip(WATCH_COLUMNS, r)) for r in rows]
        self.assertEqual([r["clock_s"] for r in written], [1.0, 1.01, 1.01])
        self.assertEqual([r["sp_amount_u32"] for r in written], [0, 0, 96])
        self.assertEqual(summary.rows, 3)
        self.assertEqual(summary.reason, "lost the engine (gone)")
        self.assertEqual(slept, [sp_watch.POLL_S] * 4)

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
        # The watch runs on the real clock over real reads whose clock never
        # stops, so only the cap can end it. Ten polls is enough to show the
        # cap waits for its time; the recorded piece holds far more reads than
        # ten polls take, and the test fails at once if they run out.
        cap_s = 10 * sp_watch.POLL_S
        reads = recorded_snapshots(ACT3_CSV)
        served, serve = serve_in_order(self, reads)
        rows = []
        start = time.time()
        summary = sp_watch.watch(serve, rows.append, cap_s=cap_s)
        self.assertGreaterEqual(time.time() - start, cap_s)
        self.assertIn(f"{cap_s:g} s", summary.reason)
        # Every read was a change, so each one is a row, and nothing was lost.
        self.assertEqual(summary.rows, len(served))
        self.assert_one_row_per_read(rows, served)
        self.assertEqual(summary.last_clock_s, served[-1].clock_s)

    def test_should_stop_ends_the_run_after_two_polls(self):
        # should_stop says yes on its third ask. It is asked before each poll,
        # so the run reads three times, sleeps twice and stops.
        asked, slept = [], []

        def should_stop():
            asked.append(True)
            return len(asked) > 2

        def sleep(seconds):
            slept.append(seconds)
            time.sleep(seconds)

        served, serve = serve_in_order(self, recorded_snapshots(ACT3_CSV))
        rows = []
        summary = sp_watch.watch(serve, rows.append, sleep=sleep,
                                 should_stop=should_stop)
        self.assertEqual(summary.reason, sp_watch.STOPPED_REASON)
        self.assertEqual(slept, [sp_watch.POLL_S] * 2)
        self.assertEqual(len(served), 3)
        self.assert_one_row_per_read(rows, served)
        self.assertEqual(summary.rows, 3)

    def assert_one_row_per_read(self, rows, served):
        """Each written row is sp_watch.watch_row of the read it was written
        from, at the wall time the row holds, one row per read in order."""
        wall = WATCH_COLUMNS.index(sp_watch.WALL_COLUMN)
        self.assertEqual(len(rows), len(served))
        self.assertEqual(rows, [sp_watch.watch_row(r[wall], s)
                                for r, s in zip(rows, served)])


def serve_in_order(test, reads):
    """A snapshot callable serving `reads` in order, and the list of reads it
    has served. Running out fails the test at once."""
    pending = list(reads)
    served = []

    def serve():
        if not pending:
            test.fail(f"the watch read all {len(served)} recorded reads")
        served.append(pending.pop(0))
        return served[-1]
    return served, serve


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

    def finder_dots(self, proc):
        """What the engine search prints on its own when no song plays,
        inside fakes.searching_for_no_song."""
        dots = io.StringIO()
        with self.assertRaises(F.EngineSearchTimeout):
            F.find_live_engine_capped(proc, F.all_patterns(proc), out=dots)
        self.assertNotEqual(dots.getvalue(), "")
        return dots.getvalue()

    def test_the_default_search_prints_its_dots_to_out_only(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)
        out, stdout = io.StringIO(), io.StringIO()
        with fakes.searching_for_no_song(F), contextlib.redirect_stdout(stdout):
            dots = self.finder_dots(proc)
            code = sp_watch.run(["--out", self.out_csv],
                                opener=self.opener_for(proc), out=out)
        self.assertEqual(code, sp_watch.EXIT_NO_SONG)
        self.assertEqual(stdout.getvalue(), "")
        self.assertIn(dots, out.getvalue())

    def test_find_engine_live_sends_its_dots_to_the_out_it_is_given(self):
        proc, _ = fakes.fake_process(None, BASE, consts=GOOD_CONSTS)
        quiet, stdout = io.StringIO(), io.StringIO()
        with fakes.searching_for_no_song(F), contextlib.redirect_stdout(stdout):
            dots = self.finder_dots(proc)
            with self.assertRaises(F.EngineSearchTimeout):
                sp_watch.find_engine_live(proc, out=quiet)
            self.assertEqual(stdout.getvalue(), "")
            with self.assertRaises(F.EngineSearchTimeout):
                sp_watch.find_engine_live(proc)
        self.assertEqual(quiet.getvalue(), dots)
        self.assertEqual(stdout.getvalue(), dots)

    def test_opens_the_game_read_only(self):
        # The real opener, asked for a process by the name of the game's
        # module: no process is named after a DLL, so it finds none in the
        # real process list and the run stops before any read.
        def opener(**kwargs):
            self.opened.append(kwargs)
            return P.open_process(C.MODULE_NAME, **kwargs)

        with self.assertRaises(P.ProcessNotFoundError):
            sp_watch.run(["--out", self.out_csv], opener=opener,
                         out=io.StringIO())
        self.assertEqual(self.opened, [{"read_only": True}])
        self.assertFalse(os.path.exists(self.out_csv))


if __name__ == "__main__":
    unittest.main()
