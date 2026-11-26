"""Tests for agent/frame_dry_run.py, the offline dry run of the kit agent's
frame picker (task T5 of the 5000% speed-run plan, acceptance line 1).

Each case cuts the checked-in schedule testdata/ch_runs/efl-prep/schedule.json
down to its first three chords, which the test writes as input, and runs the
script's own message builder and picker driver on it. The tests skip, naming
the reason, when `node` is not on PATH.
"""

from __future__ import annotations

import csv
import json
import os
import shutil
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp.agent import frame_dry_run as D  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402

SOURCE = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "efl-prep", "schedule.json")
NODE = shutil.which("node")
# The cases' frame step: any step far smaller than a press's window will do.
STEP_MS = 3.0


@unittest.skipIf(NODE is None, "node is not on PATH, so press_schedule.ts cannot run")
class FrameDryRunTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.dir)
        with open(SOURCE, encoding="utf-8") as f:
            doc = json.load(f)
        keep = {c["index"] for c in doc["chords"][:3]}
        doc["chords"] = [c for c in doc["chords"] if c["index"] in keep]
        doc["presses"] = [p for p in doc["presses"] if p["chord"] in keep]
        self.schedule = os.path.join(self.dir, "schedule.json")
        with open(self.schedule, "w", encoding="utf-8") as f:
            json.dump(doc, f)
        self.play = D.play_message(self.schedule)
        self.assertEqual(len(self.play["entries"]), 3)

    def test_three_presses_on_an_even_list_are_all_sent(self):
        t = D.run_frames(self.play, even_step_ms=STEP_MS)
        self.assertEqual((t["sent"], t["dropped"], t["skipped"]), (3, 0, 0))
        self.assertEqual(t["no_frame_in_band_seqs"], [])
        self.assertEqual(t["stop"], "done")

    def test_one_frame_longer_than_a_window_across_it_drops_exactly_that_press(self):
        middle = sorted(self.play["entries"], key=lambda e: e["at_ms"])[1]
        front, back = middle["window_from_ms"], middle["window_to_ms"]
        # Even frames from the start clock to well past the last press, with
        # the frames from ten steps before the middle press's window to ten
        # steps after it left out: one frame step spans the whole window.
        start = D.start_clock_ms(self.play)
        end = max(e["window_to_ms"] for e in self.play["entries"]) + 2000.0
        clocks, k = [], 0
        while start + k * STEP_MS < end:
            clock = start + k * STEP_MS
            if not front - 10 * STEP_MS < clock < back + 10 * STEP_MS:
                clocks.append(clock)
            k += 1
        self.assertGreater(min(c for c in clocks if c > front) - max(c for c in clocks if c < front),
                           back - front)
        path = os.path.join(self.dir, "frame_clock.csv")
        with open(path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(play_chart.FRAME_CLOCK_COLUMNS)
            for n, clock in enumerate(clocks):
                w.writerow([100 + n, clock])
        t = D.run_frames(self.play, csv_path=path)
        self.assertEqual([d["seq"] for d in t["dropped_seqs"]], [middle["seq"]])
        self.assertEqual((t["sent"], t["dropped"]), (2, 1))


def _run_folder() -> str:
    """hydra-data's 2026-11-22 run folder, found beside this checkout or one
    of its parents; "" when there is none."""
    here = _REPO_ROOT
    while True:
        folder = os.path.join(here, "hydra-data", "ch-mcp-runs", "2026-11-22")
        if os.path.isdir(folder):
            return folder
        up = os.path.dirname(here)
        if up == here:
            return ""
        here = up


RUNS = _run_folder()


@unittest.skipIf(NODE is None, "node is not on PATH, so press_schedule.ts cannot run")
@unittest.skipIf(RUNS == "", "hydra-data's 2026-11-22 run folder is not beside this checkout")
class RealRunTest(unittest.TestCase):
    """The picker, fed the frames LS-M16 run 2 logged, does with the run's
    schedule what run 2's own press log says it did."""

    def test_run_two_frames_send_every_press_on_the_logged_frame(self):
        schedule = os.path.join(RUNS, "ls-m16-prep", "schedule.json")
        run = os.path.join(RUNS, "ls-m16-run2")
        play = D.play_message(schedule)
        t = D.run_frames(play, csv_path=os.path.join(run, "frame_clock-1.csv"))
        self.assertEqual((t["sent"], t["dropped"], t["skipped"]), (len(play["entries"]), 0, 0))
        self.assertEqual(t["no_frame_in_band_seqs"], [])
        mine = {}
        for index, frame in t["sent_frames"]:
            mine.setdefault(index, []).append(frame)
        with open(os.path.join(run, "press-1.csv"), newline="", encoding="utf-8") as f:
            rows = list(csv.DictReader(f))
        self.assertEqual(len(rows), len(play["entries"]))
        # A chord Hydra declines is still pressed, so every row has a frame.
        differ = {}
        for row in rows:
            index, logged = int(row["index"]), int(row["down_frame"])
            if logged not in mine.get(index, []):
                differ[index] = (logged, mine.get(index))
        # The dry run reads no Star Power end, so no retime reaches the picker
        # and a press waiting on one is timed as planned; run 2 timed index
        # 1504 (it waits on chord 1192's end) one frame earlier. Every other
        # press went down on the frame the log says.
        self.assertEqual(sorted(differ), [1504], differ)
        waits = [e for e in play["entries"] if e["index"] == 1504]
        self.assertEqual([e["end_read_after"] for e in waits], [1192])
        self.assertEqual(differ[1504], (801383, [801384]))


if __name__ == "__main__":
    unittest.main()
