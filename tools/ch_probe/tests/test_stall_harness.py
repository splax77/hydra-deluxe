"""The offline stall harness (PS4 of
docs/superpowers/plans/2026-11-12-player-stall-fixes.md). Every run here is
a fraction of a minute on a few chords, with no cdb and no real load
process, so the file runs in seconds."""

from __future__ import annotations

import builtins
import os
import tempfile
import time
import unittest
from dataclasses import replace
from unittest import mock

from tools.ch_mcp import frida_attach as FA
from tools.ch_mcp import kit_tools
from tools.ch_probe import constants as C
from tools.ch_probe.experiments import live, overlay_logs, play_chart
from tools.ch_probe.experiments import stall_harness as SH

_HERE = os.path.dirname(os.path.abspath(__file__))
_SCHEDULE = os.path.join(_HERE, "edge_walk_pair_schedule.json")

# The tests' own sizes, far from the run's real numbers so a busy machine
# cannot flip them: a forced block well past a threshold well past noise.
# The threshold is a multiple of the user's own, so it stays above the real
# one if that moves.
_THRESHOLD_MS = 4 * C.STALL_THRESHOLD_MS
_BLOCK_MS = 400.0
_CHORDS = 4
_SPACING_MS = 100.0
# Two passes of the chords above: a pass runs from twice the start lead
# before its first press to its last chord.
_MINUTES = 2 * C.ms_to_s(2 * live.START_LEAD_MS + _CHORDS * _SPACING_MS) / 60


def _chords():
    """The first chords of a real schedule, moved 100 ms apart, each press
    keeping its own room to its last send."""
    chords = play_chart.load_schedule(_SCHEDULE).chords[:_CHORDS]
    base = chords[0].ms
    out = []
    for k, chord in enumerate(chords):
        ms = base + k * _SPACING_MS
        presses = tuple(replace(p, chart_ms=ms, last_send_ms=p.last_send_ms - chord.ms + ms)
                        for p in chord.presses)
        out.append(replace(chord, ms=ms, presses=presses))
    return out


def _no_cdb(_timeout_s):
    return None, "Microsoft.WinDbg is not installed"


class _Run:
    def __init__(self, test: unittest.TestCase) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        test.addCleanup(self.tmp.cleanup)
        self.folder = self.tmp.name
        self.lines: list = []

    def __call__(self, **kw) -> int:
        kw.setdefault("threshold_ms", _THRESHOLD_MS)
        kw.setdefault("minutes", _MINUTES)
        kw.setdefault("find", _no_cdb)
        return SH.run_harness(_chords(), folder=self.folder, out=self.lines.append, **kw)

    def text(self) -> str:
        return "\n".join(self.lines)

    def stall_files(self) -> list:
        return sorted(n for n in os.listdir(self.folder) if n.startswith("stall-"))

    def read(self, name: str) -> str:
        with open(os.path.join(self.folder, name), encoding="utf-8") as f:
            return f.read()


class StallHarnessTest(unittest.TestCase):
    def test_a_forced_stall_writes_a_dump_that_names_the_sink(self):
        run = _Run(self)
        self.assertEqual(run(sink=SH.SINK_PER_ROW, force_stall_ms=_BLOCK_MS), 0)
        self.assertIn("stall-1.txt", run.stall_files())
        dump = run.read("stall-1.txt")
        self.assertIn(f"step: {SH.STEP_HANDOFF}", dump)
        self.assertIn(SH.per_row_append.__name__, dump)
        self.assertNotIn("dropped 0 chords in all", run.text())

    def test_the_writer_sink_drops_nothing_under_a_forced_stall(self):
        run = _Run(self)
        self.assertEqual(run(sink=SH.SINK_WRITER, force_stall_ms=_BLOCK_MS), 0)
        self.assertEqual(run.stall_files(), [])
        self.assertIn("dropped 0 chords in all", run.text())
        self.assertIn(f"press-1.csv ({_CHORDS} rows)", run.text())

    def test_the_cap_ends_the_run_and_says_so(self):
        run = _Run(self)
        code = SH.main([_SCHEDULE, "--sink", SH.SINK_NONE, "--minutes", "0.01",
                        "--no-cdb", "--out", run.folder], out=run.lines.append)
        self.assertEqual(code, 0)
        self.assertIn("cap reached", run.lines)
        # No --threshold-ms given: the run uses the user's number.
        self.assertTrue(any(f"over {C.STALL_THRESHOLD_MS:g} ms in 0.01 minutes" in line
                            for line in run.lines))

    def test_the_defaults_are_the_users_numbers(self):
        seen = {}

        def fake_run(chords, **kw):
            seen.update(kw)
            return 0

        with mock.patch.object(SH, "run_harness", fake_run):
            SH.main([_SCHEDULE, "--sink", SH.SINK_NONE, "--no-cdb"])
        self.assertEqual(seen["threshold_ms"], C.STALL_THRESHOLD_MS)
        self.assertEqual(seen["minutes"], C.STALL_RUN_MINUTES)

    def test_a_hunt_run_past_the_most_is_refused(self):
        with self.assertRaises(SystemExit), mock.patch("sys.stderr"):
            SH.main([_SCHEDULE, "--sink", SH.SINK_NONE,
                     "--hunt-run", str(C.STALL_HUNT_RUNS_MAX + 1)])

    def test_a_clean_run_says_no_stall(self):
        run = _Run(self)
        threshold_ms, minutes = 500.0, 0.01
        self.assertEqual(run(sink=SH.SINK_NONE, threshold_ms=threshold_ms, minutes=minutes), 0)
        self.assertIn(f"no stall over 500 ms in 0.01 minutes", run.lines)

    def test_a_missing_cdb_is_said_and_the_run_goes_on(self):
        run = _Run(self)
        launched = []
        self.assertEqual(run(sink=SH.SINK_PER_ROW, force_stall_ms=_BLOCK_MS, cdb=True,
                             launch=lambda *a: launched.append(a) or ""), 0)
        dump = run.read("stall-1.txt")
        self.assertIn("cdb: none: Microsoft.WinDbg is not installed", dump)
        self.assertIn(SH.per_row_append.__name__, dump)
        self.assertEqual(launched, [])

    def test_a_second_stall_in_one_run_starts_no_second_cdb(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        clock = [100.0]
        tracker = SH.StepTracker(now=lambda: clock[0])
        launched = []
        dog = SH.Watchdog(tracker, tmp.name, _THRESHOLD_MS, cdb=("cdb.exe", ""),
                          deadline=clock[0] + 60,
                          launch=lambda *a: launched.append(a) or "fake cdb stacks",
                          now=lambda: clock[0])
        for step in (SH.STEP_HANDOFF, SH.STEP_PRESS):
            tracker.enter(step)
            clock[0] += C.ms_to_s(2 * _THRESHOLD_MS)
            dog.check()
            dog.check()
            tracker.leave()
        self.assertEqual(len(launched), 1)
        self.assertEqual([step for _path, step, _ms in dog.stalls],
                         [SH.STEP_HANDOFF, SH.STEP_PRESS])
        for n in (1, 2):
            with open(os.path.join(tmp.name, f"stall-{n}.txt"), encoding="utf-8") as f:
                dump = f.read()
            self.assertIn("most recent call first", dump)
        with open(os.path.join(tmp.name, "stall-1.txt"), encoding="utf-8") as f:
            self.assertIn("fake cdb stacks", f.read())

    def test_a_sleep_counts_only_past_what_it_asked(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        clock = [0.0]
        tracker = SH.StepTracker(now=lambda: clock[0])
        dog = SH.Watchdog(tracker, tmp.name, _THRESHOLD_MS, cdb=(None, "test"),
                          deadline=60.0, now=lambda: clock[0])
        tracker.enter(SH.STEP_WAIT_OVERSLEEP, allow_s=0.5)
        clock[0] += 0.5 + C.ms_to_s(_THRESHOLD_MS / 2)
        dog.check()
        self.assertEqual(dog.stalls, [])
        clock[0] += C.ms_to_s(_THRESHOLD_MS)
        dog.check()
        self.assertEqual([step for _p, step, _ms in dog.stalls], [SH.STEP_WAIT_OVERSLEEP])

    def test_a_blocked_agent_log_write_does_not_stall_the_press(self):
        # PS5 moved the kit log's write to LiveAgent's writer thread, so a
        # block there must leave the press, and every other step, on time.
        real_open = builtins.open
        blocked = []

        class BlockingLog:
            def __init__(self, f):
                self._f = f

            def write(self, text):
                if not blocked:
                    blocked.append(True)
                    time.sleep(C.ms_to_s(_BLOCK_MS))
                return self._f.write(text)

            def __getattr__(self, name):
                return getattr(self._f, name)

        def fake_open(path, mode="r", *a, **kw):
            f = real_open(path, mode, *a, **kw)
            return BlockingLog(f) if str(path).endswith(".jsonl") and "w" in mode else f

        run = _Run(self)
        with mock.patch.object(FA, "open", fake_open, create=True):
            self.assertEqual(run(sink=SH.SINK_WRITER), 0)
        self.assertEqual(blocked, [True])
        self.assertEqual(run.stall_files(), [])
        self.assertIn("dropped 0 chords in all", run.text())
        with open(os.path.join(run.folder, "kit-1.jsonl"), encoding="utf-8") as f:
            # The first read, then one reply per chord.
            self.assertEqual(len(f.read().splitlines()), 1 + _CHORDS)

    def test_the_load_process_is_stopped_at_the_end(self):
        made = []

        class FakeLoad:
            def __init__(self, args):
                self.args, self.killed, self.waited = args, False, False
                made.append(self)

            def kill(self):
                self.killed = True

            def wait(self):
                self.waited = True

        run = _Run(self)
        self.assertEqual(run(sink=SH.SINK_NONE, minutes=0.005, load=True, popen=FakeLoad), 0)
        self.assertIn("cap reached", run.lines)
        with mock.patch.object(SH, "_play_pass", side_effect=RuntimeError("boom")):
            with self.assertRaises(RuntimeError):
                run(sink=SH.SINK_NONE, load=True, popen=FakeLoad)
        self.assertEqual(len(made), 2)
        self.assertTrue(all(p.killed and p.waited for p in made))
        folders = SH.load_folders()
        self.assertEqual(made[0].args[-2:], folders)
        repo = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
        self.assertEqual(folders[0], os.path.join(repo, "testdata"))
        self.assertTrue(os.path.isdir(folders[1]))
        self.assertTrue(os.path.isfile(os.path.join(folders[1], "HEAD")))
        self.assertEqual(os.path.basename(folders[1]), ".git")

    def test_the_step_names_are_the_timing_columns(self):
        steps = (SH.STEP_WAIT_READ, SH.STEP_WAIT_OVERSLEEP, SH.STEP_PRESS,
                 SH.STEP_REPLY_CHECK, SH.STEP_END_READ, SH.STEP_SCORE_READ, SH.STEP_HANDOFF)
        for step, column in zip(steps, overlay_logs.TIMING_PRESS_COLUMNS):
            self.assertIn(step, column)


# --- IG4: a Python stall drops nothing when the game times the presses -------
#
# These cases play cuts of the schedule LS-P3 played, on the fake game
# (stall_game.mjs), and skip when that file or `node` is missing.

_LS_P3_SCHEDULE = os.path.join(
    r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep",
    "The Who - Won't Get Fooled Again (O)-schedule.json")
# The block PS1 forced on the old loop (PS1 of
# docs/superpowers/plans/2026-11-12-player-stall-fixes.md).
_PS1_BLOCK_MS = 900.0
# Chords 285 to 300: about 4 s of chords roughly 220 ms apart, so the old
# loop's block after the first one outlasts the next chords' last sends. The
# cut holds chord 292's end read, so its retime goes out too.
_DENSE_CUT = (285, 300)
# The end read IG0 found closest to the first press it moves (7.09 s before
# it), and that press's chord.
_READ_CHORD, _MOVED_CHORD = 1218, 1261
# The late-retime case's own construction: the read chord is moved to go
# down this long before the first press its read moves, well inside the
# block, since no read on file is closer than 7 s.
_READ_LEAD_MS = 400.0
# The chords of the moved chord's neighbourhood the late-retime cut keeps,
# before and after it. The cut runs on past the block, so the play is still
# running when the late retime arrives: the agent refuses a retime once its
# play has ended.
_AROUND_MS = (1500.0, 1500.0)
_PASS_CAP_S = 60.0


def _ls_p3_chords():
    return play_chart.load_schedule(_LS_P3_SCHEDULE).chords


def _dense_cut():
    first, last = _DENSE_CUT
    return [c for c in _ls_p3_chords() if first <= c.index <= last]


def _late_retime_cut():
    """The moved chord with its neighbours, and the read chord moved in
    time (its chord, presses and last sends together) so its first press
    goes down _READ_LEAD_MS before the first press its read moves."""
    chords = _ls_p3_chords()
    by = {c.index: c for c in chords}
    moved_at = min(p.at_ms for c in chords for p in c.presses
                   if p.end_read_after == _READ_CHORD)
    read = by[_READ_CHORD]
    shift = moved_at - _READ_LEAD_MS - min(p.at_ms for p in read.presses)
    read = replace(read, ms=read.ms + shift, presses=tuple(
        replace(p, chart_ms=p.chart_ms + shift, last_send_ms=p.last_send_ms + shift)
        for p in read.presses))
    before, after = _AROUND_MS
    near = [c for c in chords if c.index != _READ_CHORD
            and by[_MOVED_CHORD].ms - before <= c.ms <= by[_MOVED_CHORD].ms + after]
    return sorted([read, *near], key=lambda c: c.ms)


@unittest.skipUnless(SH.node_path() is not None, "node is not on PATH, so the fake game "
                                                 "cannot run")
@unittest.skipUnless(os.path.isfile(_LS_P3_SCHEDULE), f"LS-P3's schedule is not on this "
                                                      f"machine: {_LS_P3_SCHEDULE}")
class InGameStallTest(unittest.TestCase):
    def play(self, chords, **kw):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        kw.setdefault("sink", SH.SINK_PER_ROW)
        began = time.perf_counter()
        report = SH.play_once(chords, folder=tmp.name, cap_s=_PASS_CAP_S,
                              out=lambda _line: None, **kw)
        self.took_s = time.perf_counter() - began
        return report

    def assert_every_seq_once(self, chords, game):
        seqs = set(range(len(play_chart.press_plan(chords))))
        self.assertEqual(game.reported, {s: 1 for s in seqs})
        self.assertEqual(set(game.fires.values()), {1})
        self.assertLessEqual(set(game.fires), seqs)

    def test_a_900_ms_block_drops_nothing_when_the_game_times_the_presses(self):
        chords = _dense_cut()
        run = self.play(chords, timing=SH.TIMING_GAME, force_stall_ms=_PS1_BLOCK_MS)
        result = run.result
        self.assertEqual(result.stop, play_chart.STOP_DONE, result.message)
        self.assertEqual(result.dropped, 0)
        self.assertEqual([r for r in result.rows
                          if r[overlay_logs.SEND_COLUMN] == play_chart.DROPPED], [])
        self.assertEqual(run.retime_missed, 0)
        self.assertEqual(result.sent, len(chords))
        self.assert_every_seq_once(chords, run.game)
        # The block landed while presses were still to come: some press's
        # report waited most of it.
        self.assertGreater(run.game.latest_take_ms, _PS1_BLOCK_MS / 2)
        self.assertIn(kit_tools.RETIME_TYPE, [kind for kind, _t in run.game.posts])

    def test_the_same_900_ms_block_drops_chords_from_the_outside_loop(self):
        run = self.play(_dense_cut(), timing=SH.TIMING_LOOP, force_stall_ms=_PS1_BLOCK_MS)
        self.assertEqual(run.result.stop, play_chart.STOP_DONE, run.result.message)
        self.assertGreaterEqual(run.result.dropped, 1)

    def test_a_retime_900_ms_late_fires_no_press_twice_and_says_retime_missed(self):
        chords = _late_retime_cut()
        run = self.play(chords, timing=SH.TIMING_GAME, end_read_block_ms=_PS1_BLOCK_MS)
        result = run.result
        self.assertEqual(result.stop, play_chart.STOP_DONE, result.message)
        self.assertEqual(result.dropped, 0)
        self.assert_every_seq_once(chords, run.game)
        missed = sorted({r["index"] for r in result.rows
                         if r["agent_note"] == kit_tools.NOTE_RETIME_MISSED})
        self.assertEqual(missed, [_MOVED_CHORD])
        self.assertEqual(run.retime_missed, sum(
            1 for p in play_chart.press_plan(chords) if p.end_read_after == _READ_CHORD))
        self.assertIn(f"{kit_tools.NOTE_RETIME_MISSED} on chords {_MOVED_CHORD}",
                      result.message)
        # One end read, and its one retime, which reached the agent only after
        # the moved press had fired (the retime_missed rows above).
        self.assertEqual(len(run.game.end_reads), 1)
        self.assertEqual([kind for kind, _t in run.game.posts].count(kit_tools.RETIME_TYPE), 1)


if __name__ == "__main__":
    unittest.main()
