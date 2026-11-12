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
from tools.ch_probe import constants as C
from tools.ch_probe.experiments import overlay_logs, play_chart
from tools.ch_probe.experiments import stall_harness as SH

_HERE = os.path.dirname(os.path.abspath(__file__))
_SCHEDULE = os.path.join(_HERE, "edge_walk_pair_schedule.json")

# The tests' own sizes, far from the run's real numbers so a busy machine
# cannot flip them: a forced block well past a threshold well past noise.
_THRESHOLD_MS = 150.0
_BLOCK_MS = 400.0
_CHORDS = 4
_SPACING_MS = 100.0
# Long enough for one pass of the chords above, and a little more.
_MINUTES = 0.02


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


if __name__ == "__main__":
    unittest.main()
