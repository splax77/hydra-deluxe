"""The offline stall harness (tools/ch_probe/experiments/stall_harness.py).
Every run here is a fraction of a minute on a few chords and attaches no
cdb, so the file runs in seconds."""

from __future__ import annotations

import csv
import os
import subprocess
import tempfile
import time
import unittest
from dataclasses import replace
from unittest import mock

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


# A Store package name no machine has, so the real WinDbg lookup finds
# nothing and says why.
_NO_SUCH_PACKAGE = "Microsoft.WinDbg.NoSuchPackage"


class _Run:
    def __init__(self, test: unittest.TestCase) -> None:
        self.tmp = tempfile.TemporaryDirectory()
        test.addCleanup(self.tmp.cleanup)
        self.folder = self.tmp.name
        self.lines: list = []

    def __call__(self, **kw) -> int:
        kw.setdefault("threshold_ms", _THRESHOLD_MS)
        kw.setdefault("minutes", _MINUTES)
        kw.setdefault("cdb", False)
        return SH.run_harness(_chords(), folder=self.folder, out=self.lines.append, **kw)

    def text(self) -> str:
        return "\n".join(self.lines)

    def stall_files(self) -> list:
        return sorted(n for n in os.listdir(self.folder) if n.startswith("stall-"))

    def press_logs(self) -> dict:
        """Each press log the run wrote, by file name, as its rows."""
        logs = {}
        for name in sorted(n for n in os.listdir(self.folder)
                           if n.startswith("press-") and n.endswith(".csv")):
            with open(os.path.join(self.folder, name), encoding="utf-8", newline="") as f:
                logs[name] = list(csv.DictReader(f))
        return logs

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
        self.assertEqual(run(sink=SH.SINK_WRITER, force_stall_ms=_BLOCK_MS, cdb=False), 0)
        self.assertEqual(run.stall_files(), [])
        logs = run.press_logs()
        # The first pass plays every chord; a later pass may end at the cap.
        self.assertEqual([int(row["index"]) for row in logs["press-1.csv"]],
                         [chord.index for chord in _chords()])
        self.assertEqual([(name, row["index"]) for name, rows in logs.items() for row in rows
                          if row[overlay_logs.SEND_COLUMN] == play_chart.DROPPED], [])

    def test_the_cap_ends_the_run(self):
        run = _Run(self)
        minutes = 0.01
        began = time.perf_counter()
        code = SH.main([_SCHEDULE, "--sink", SH.SINK_WRITER, "--minutes", f"{minutes:g}",
                        "--no-cdb", "--out", run.folder], out=run.lines.append)
        took_s = time.perf_counter() - began
        # 0 is the code of a run that went on to its cap; a pass that ended
        # any other way gives 1.
        self.assertEqual(code, 0)
        self.assertGreaterEqual(took_s, minutes * 60)
        # The schedule runs about two minutes, so the cap cut its first pass
        # short and no second pass began.
        logs = run.press_logs()
        self.assertEqual(list(logs), ["press-1.csv"])
        self.assertLess(len(logs["press-1.csv"]),
                        len(play_chart.load_schedule(_SCHEDULE).chords))

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
        self.assertEqual(run.stall_files(), [])
        self.assertIn(f"no stall over {threshold_ms:g} ms in {minutes:g} minutes", run.lines)

    def test_a_missing_cdb_is_said_and_the_run_goes_on(self):
        # The real lookup, run once on a package that is not installed.
        lookup = SH._CDB_LOOKUP.replace("Microsoft.WinDbg", _NO_SUCH_PACKAGE)
        self.assertNotEqual(lookup, SH._CDB_LOOKUP)
        with mock.patch.object(SH, "_CDB_LOOKUP", lookup):
            found = SH.find_cdb(30.0)
        path, why_not = found
        self.assertIsNone(path)
        self.assertTrue(why_not)
        run = _Run(self)
        launched = []
        self.assertEqual(run(sink=SH.SINK_PER_ROW, force_stall_ms=_BLOCK_MS, cdb=True,
                             find=lambda _timeout_s: found,
                             launch=lambda *a: launched.append(a) or ""), 0)
        dump = run.read("stall-1.txt")
        self.assertIn(why_not, dump)
        self.assertIn(SH.per_row_append.__name__, dump)
        self.assertEqual(launched, [])

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

    def test_the_load_process_is_stopped_at_the_end(self):
        # The real load process, started the way the run starts it; each one
        # is kept so the test can see it ended.
        made = []

        def popen(args):
            made.append(subprocess.Popen(args))
            return made[-1]

        def end_all():
            for p in made:
                if p.poll() is None:
                    p.kill()
                    p.wait()

        self.addCleanup(end_all)
        run = _Run(self)
        self.assertEqual(run(sink=SH.SINK_NONE, minutes=0.005, load=True, popen=popen), 0)
        with mock.patch.object(SH, "_play_pass", side_effect=RuntimeError("boom")):
            with self.assertRaises(RuntimeError):
                run(sink=SH.SINK_NONE, load=True, popen=popen)
        self.assertEqual(len(made), 2)
        # The read loop never ends by itself, so a return code means the run
        # stopped it and waited for it.
        self.assertTrue(all(p.returncode is not None for p in made))
        folders = SH.load_folders()
        self.assertEqual(made[0].args[-2:], folders)
        repo = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
        self.assertEqual(folders[0], os.path.join(repo, "testdata"))
        self.assertTrue(os.path.isdir(folders[1]))
        self.assertTrue(os.path.isfile(os.path.join(folders[1], "HEAD")))

    def test_the_step_names_are_the_timing_columns(self):
        steps = (SH.STEP_WAIT_READ, SH.STEP_WAIT_OVERSLEEP, SH.STEP_PRESS,
                 SH.STEP_REPLY_CHECK, SH.STEP_END_READ, SH.STEP_SCORE_READ, SH.STEP_HANDOFF)
        self.assertEqual(len(steps), len(overlay_logs.TIMING_PRESS_COLUMNS))
        for step, column in zip(steps, overlay_logs.TIMING_PRESS_COLUMNS):
            self.assertIn(step, column)


if __name__ == "__main__":
    unittest.main()
