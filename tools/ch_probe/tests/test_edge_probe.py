"""edge_probe.py presses named chords at a lead the caller picks, through
play_chart's own plan and play loop.

The chords are made-up kicks a second apart, as on the Edge Walk probe chart,
loaded through play_chart.load_dump_chords. A run is played by
play_chart.play_chords on a fake clock and score, so no test opens the game.
"""

from __future__ import annotations

import io
import json
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import edge_probe, play_chart  # noqa: E402

KICK = {"color": "Kick", "cymbal": False, "dynamic": "none"}
# Chords 0 to 4, a second apart from 3 s.
SCORE = {"chords": [{"index": i, "ms": 3000.0 + 1000.0 * i, "notes": [KICK]} for i in range(5)]}


class _Wall:
    def __init__(self):
        self.t = 0.0

    def now(self):
        return self.t

    def sleep(self, seconds):
        self.t += seconds


class _Clock:
    """A song clock at 2 s plus the wall time; each read costs 1 ms."""

    def __init__(self, wall):
        self.wall = wall

    def read(self):
        self.wall.t += 0.001
        return 2.0 + self.wall.t, 2.0 + self.wall.t


class _Game:
    def __init__(self):
        self.score = 0

    def press(self, lanes, _axes=None):
        self.score += 50

    def read_score(self):
        return self.score


class EdgeProbeTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name

    def _file(self, name, value):
        path = os.path.join(self.tmp, name)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(value, f)
        return path

    def _chords(self, score=SCORE):
        return play_chart.load_dump_chords(self._file("score.json", score))

    # -- the probes file ------------------------------------------------------------

    def test_the_probes_file_is_a_list_of_index_and_lead(self):
        path = self._file("p.json", [{"index": 1, "lead_ms": 88.72}, {"index": 3, "lead_ms": -82}])
        self.assertEqual(edge_probe.load_probes(path),
                         [edge_probe.Probe(1, 88.72), edge_probe.Probe(3, -82.0)])

    def test_a_probes_file_that_is_not_a_list_of_probes_is_refused(self):
        for bad in ([], {"index": 1, "lead_ms": 5}, [{"index": 1}], [{"index": "1", "lead_ms": 5}],
                    [{"index": 1, "lead_ms": True}]):
            with self.subTest(bad=bad):
                with self.assertRaises(ValueError):
                    edge_probe.load_probes(self._file("p.json", bad))

    def test_a_chord_named_twice_is_refused(self):
        path = self._file("p.json", [{"index": 1, "lead_ms": 5}, {"index": 1, "lead_ms": -5}])
        with self.assertRaisesRegex(ValueError, "names chords 1 more than once"):
            edge_probe.load_probes(path)

    # -- moving the probe chords ----------------------------------------------------

    def test_a_probe_chord_is_pressed_at_its_lead_and_no_other_press_moves(self):
        chords = self._chords()
        moved, _run = edge_probe.probe_chords(
            chords, [edge_probe.Probe(1, 88.72), edge_probe.Probe(3, -82.72)])
        at = {p.index: p.at_ms for p in play_chart.press_plan(moved)}
        plain = {p.index: p.at_ms for p in play_chart.press_plan(chords)}
        self.assertAlmostEqual(at[1], 4000.0 - 88.72)
        self.assertAlmostEqual(at[3], 6000.0 + 82.72)
        self.assertEqual({i: at[i] for i in (0, 2, 4)}, {i: plain[i] for i in (0, 2, 4)})

    def test_a_probe_chord_not_in_the_score_file_is_refused(self):
        with self.assertRaisesRegex(ValueError, "probe chords 9 are not in the score file"):
            edge_probe.probe_chords(self._chords(), [edge_probe.Probe(9, 10.0)])

    def test_a_squeeze_chord_timed_from_its_end_is_refused(self):
        # Chord 1 squeezes in at an end 10 ms before it, so press_plan times
        # its press from the end, not from the chord's own time. The end is
        # close enough that the press lands in the chord's window with the
        # activation pressed on time, so activation_leads asks for no lead
        # and the refusal is the probe's own.
        chords = self._chords()
        chords[1] = play_chart.Chord(1, 4000.0, chords[1].lanes, chords[1].dynamics,
                                     squeeze=play_chart.Squeeze(play_chart.SQUEEZE_IN, 3990.0, 0))
        self.assertEqual(play_chart.activation_leads(chords), [])
        with self.assertRaisesRegex(ValueError, "probe chords 1 are timed by another rule"):
            edge_probe.probe_chords(chords, [edge_probe.Probe(1, 10.0)])

    # -- the rows -------------------------------------------------------------------

    def test_a_run_presses_the_probes_at_their_leads_and_marks_only_their_rows(self):
        moved, run = edge_probe.probe_chords(
            self._chords(), [edge_probe.Probe(1, 88.72), edge_probe.Probe(3, -82.72)])
        wall, game = _Wall(), _Game()
        result = play_chart.play_chords(moved, _Clock(wall), game.read_score, game.press,
                                        now=wall.now, sleep=wall.sleep, on_press=run.mark_row,
                                        out=io.StringIO())
        self.assertEqual(result.stop, "done")
        rows = {row["index"]: row for row in result.rows}
        self.assertEqual({i: row["chart_ms"] for i, row in rows.items()},
                         {i: 3000.0 + 1000.0 * i for i in range(5)})
        self.assertEqual({i: row.get(edge_probe.PROBE_COLUMN) for i, row in rows.items()},
                         {0: None, 1: 88.72, 2: None, 3: -82.72, 4: None})
        for index, asked in ((1, 88.72), (3, -82.72), (0, play_chart.PRESS_LEAD_MS)):
            with self.subTest(index=index):
                read = rows[index]["chart_ms"] - rows[index]["clock_ms_at_press"]
                # The fake clock moves 1 ms a read, and a press fires at the
                # first read at or past its time.
                self.assertLessEqual(read, asked)
                self.assertGreater(read, asked - 1.5)
                self.assertAlmostEqual(rows[index]["lead_ms"], asked)

    def test_the_probe_row_lines_give_asked_and_pressed_leads_and_the_hit(self):
        rows = [{"index": 0, "chart_ms": 3000.0, "clock_ms_at_press": 2998.0, "hit": 1},
                {"index": 1, "chart_ms": 4000.0, "clock_ms_at_press": 3911.5, "hit": 1,
                 "probe": 88.72},
                {"index": 3, "chart_ms": 6000.0, "clock_ms_at_press": 6082.0, "hit": 0,
                 "probe": -82.72}]
        self.assertEqual(edge_probe.probe_row_lines(rows), [
            "probe chord 1: asked 88.72 ms early, pressed 88.50 ms early, hit",
            "probe chord 3: asked 82.72 ms late, pressed 82.00 ms late, miss"])

    def test_the_probes_line_names_each_probe_in_the_files_order(self):
        self.assertEqual(edge_probe.probe_lines([edge_probe.Probe(3, -82.72),
                                                 edge_probe.Probe(1, 88.72)]),
                         ["probes: chord 3 82.72 ms late, chord 1 88.72 ms early"])


if __name__ == "__main__":
    unittest.main()
