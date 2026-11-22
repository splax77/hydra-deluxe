"""edge_probe.py presses named chords at a lead the caller picks, through
play_chart's own plan and play loop.

The chord cases read freak_limits_schedule.json, hydra_replay schedule's file
for Freak path 0, through play_chart.load_schedule; the probe chords are picked
from it by their press rule, not by a typed index. The row cases read a real
probe run: testdata/ch_runs/2026-11-07-ls-e-run1 holds its probes.json and
the plain rows 0 to 11 plus every probe row of its press-1.csv, copied from
the LS-E run 1 folder hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1 (an
ch_edge_probe run on the Edge Walk Pair chart). No test opens the game.
"""

from __future__ import annotations

import csv
import json
import os
import re
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import edge_probe, play_chart  # noqa: E402

_FREAK_SCHEDULE = os.path.join(_HERE, "freak_limits_schedule.json")
_PROBE_RUN = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-07-ls-e-run1")
_PROBE_RUN_PROBES = os.path.join(_PROBE_RUN, "probes.json")
_PROBE_RUN_PRESSES = os.path.join(_PROBE_RUN, "press-1.csv")


def _freak_chords():
    return play_chart.load_schedule(_FREAK_SCHEDULE).chords


def _plain(chord):
    return all(press.rule == "plain" for press in chord.presses)


def _from_end(chord):
    return any(press.timed_from_end is not None for press in chord.presses)


def _first_from_end(chords):
    """The first chord whose press the schedule times from a Star Power end."""
    found = [chord for chord in chords if _from_end(chord)]
    if not found:
        raise AssertionError("the Freak schedule has no chord timed from an end")
    return found[0]


def _probe_run_rows():
    with open(_PROBE_RUN_PRESSES, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f))


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

    def _two_plain_chords(self, chords):
        """Freak's first chord and the chord just before its first squeeze,
        each a plain press."""
        squeeze = _first_from_end(chords)
        before = [chord for chord in chords if chord.index < squeeze.index]
        picked = (chords[0], before[-1])
        for chord in picked:
            self.assertTrue(_plain(chord), chord.index)
        return picked

    def _pressed_at(self, moved, chords, probed):
        """The first press time press_plan gives chord `index` after the move,
        with every other chord's press times checked unchanged."""
        at = {}
        for press in play_chart.press_plan(moved):
            at.setdefault(press.index, press.at_ms)
        plain = {}
        for press in play_chart.press_plan(chords):
            plain.setdefault(press.index, press.at_ms)
        self.assertEqual({i: t for i, t in at.items() if i not in probed},
                         {i: t for i, t in plain.items() if i not in probed})
        return at

    # -- the probes file ------------------------------------------------------------

    def test_a_probes_file_sets_the_lead_each_named_chord_is_pressed_at(self):
        chords = _freak_chords()
        first, before_squeeze = self._two_plain_chords(chords)
        # One lead is a whole number, as a hand-written file may give it.
        entries = [{"index": first.index, "lead_ms": 88.72},
                   {"index": before_squeeze.index, "lead_ms": -82}]
        probes = edge_probe.load_probes(self._file("p.json", entries))
        moved, _run = edge_probe.probe_chords(chords, probes)
        by_index = {chord.index: chord for chord in chords}
        at = self._pressed_at(moved, chords, {e["index"] for e in entries})
        for entry in entries:
            with self.subTest(index=entry["index"]):
                self.assertAlmostEqual(at[entry["index"]],
                                       by_index[entry["index"]].ms - entry["lead_ms"])

    def test_a_probes_file_that_is_not_a_list_of_probes_is_refused(self):
        for bad in ([], {"index": 1, "lead_ms": 5}, [{"index": 1}], [{"index": "1", "lead_ms": 5}],
                    [{"index": 1, "lead_ms": True}]):
            with self.subTest(bad=bad):
                with self.assertRaises(ValueError):
                    edge_probe.load_probes(self._file("p.json", bad))

    def test_a_chord_named_twice_is_refused_with_the_file_and_that_chord_named(self):
        path = self._file("p.json", [{"index": 1, "lead_ms": 5}, {"index": 2, "lead_ms": 5},
                                     {"index": 1, "lead_ms": -5}])
        with self.assertRaises(ValueError) as caught:
            edge_probe.load_probes(path)
        message = str(caught.exception)
        self.assertIn(path, message)
        # With the path taken out, the only numbers left are the chords named.
        self.assertEqual(re.findall(r"\d+", message.replace(path, "")), ["1"])

    # -- moving the probe chords ----------------------------------------------------

    def test_a_probe_chord_is_pressed_at_its_lead_and_no_other_press_moves(self):
        chords = _freak_chords()
        early, late = self._two_plain_chords(chords)
        moved, _run = edge_probe.probe_chords(
            chords, [edge_probe.Probe(early.index, 88.72), edge_probe.Probe(late.index, -82.72)])
        at = self._pressed_at(moved, chords, {early.index, late.index})
        self.assertAlmostEqual(at[early.index], early.ms - 88.72)
        self.assertAlmostEqual(at[late.index], late.ms + 82.72)
        # A moved press is a judge target alone: it loses its band and its
        # window, so the player never drops it past the window. Every other
        # press keeps its band and window as the schedule gave them.
        was = {chord.index: chord for chord in chords}
        for chord in moved:
            for now, then in zip(chord.presses, was[chord.index].presses):
                with self.subTest(chord=chord.index):
                    if chord.index in (early.index, late.index):
                        self.assertIsNotNone(then.band)
                        self.assertIsNotNone(then.window_from_ms)
                        self.assertEqual((now.band, now.window_from_ms, now.window_to_ms),
                                         (None, None, None))
                    else:
                        self.assertEqual((now.band, now.window_from_ms, now.window_to_ms),
                                         (then.band, then.window_from_ms, then.window_to_ms))

    def test_a_probe_chord_not_in_the_score_file_is_refused_and_only_it_is_named(self):
        chords = _freak_chords()
        present = chords[0].index
        missing = max(chord.index for chord in chords) + 1
        with self.assertRaises(ValueError) as caught:
            edge_probe.probe_chords(chords, [edge_probe.Probe(present, 10.0),
                                             edge_probe.Probe(missing, 10.0)])
        self.assertEqual(re.findall(r"\d+", str(caught.exception)), [str(missing)])

    def test_a_squeeze_chord_timed_from_its_end_is_refused(self):
        chords = _freak_chords()
        squeeze = _first_from_end(chords)
        with self.assertRaises(ValueError) as caught:
            edge_probe.probe_chords(chords, [edge_probe.Probe(squeeze.index, 10.0)])
        self.assertEqual(str(caught.exception),
                         edge_probe._timed_by_another_rule([squeeze.index]))

    def test_a_plain_chord_of_a_real_schedule_is_pressed_at_its_lead(self):
        chords = _freak_chords()
        _first, chord = self._two_plain_chords(chords)
        moved, _run = edge_probe.probe_chords(chords, [edge_probe.Probe(chord.index, -60.0)])
        at = self._pressed_at(moved, chords, {chord.index})
        self.assertAlmostEqual(at[chord.index], chord.ms + 60.0)

    # -- the rows -------------------------------------------------------------------

    def test_the_probe_row_lines_of_a_real_run_give_one_line_per_probe_row(self):
        rows = _probe_run_rows()
        probe_rows = [row for row in rows if row[edge_probe.PROBE_COLUMN] != ""]
        self.assertTrue(probe_rows and len(probe_rows) < len(rows))
        lines = edge_probe.probe_row_lines(rows)
        self.assertEqual(len(lines), len(probe_rows))
        for row, line in zip(probe_rows, lines):
            with self.subTest(index=row["index"]):
                self.assertEqual(re.findall(r"chord (\d+)", line), [row["index"]])
                asked = float(row[edge_probe.PROBE_COLUMN])
                read = float(row["chart_ms"]) - float(row["clock_ms_at_press"])
                asked_words = edge_probe._lead_words(asked)
                pressed_words = edge_probe._lead_words(read)
                self.assertLess(line.index(asked_words), line.rindex(pressed_words))
                # The run's own verdict column: the game scored nothing for
                # exactly the presses it marked as misses.
                step = float(re.findall(r"-?\d+(?:\.\d+)?", line)[-1])
                self.assertEqual(step == 0, row["hit"] == "0")
        self.assertEqual(edge_probe.probe_row_lines(
            [row for row in rows if row[edge_probe.PROBE_COLUMN] == ""]), [])

    def test_the_probes_line_names_each_probe_of_a_real_file_in_its_order(self):
        probes = edge_probe.load_probes(_PROBE_RUN_PROBES)
        lines = edge_probe.probe_lines(probes)
        self.assertEqual(len(lines), 1)
        named = re.findall(r"chord (\d+) (\d+(?:\.\d+)?) ms (early|late)", lines[0])
        self.assertEqual([(int(i), float(ms), side) for i, ms, side in named],
                         [(p.index, abs(p.lead_ms), "early" if p.lead_ms >= 0 else "late")
                          for p in probes])


if __name__ == "__main__":
    unittest.main()
