"""Tests for watch_checks.py: the +0x2E0 and pause checks on a Star Power
watcher log. The tests read real watcher logs under testdata/ch_runs and
Hydra's score chords for the same runs; no game."""

from __future__ import annotations

import csv
import math
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.experiments import watch_checks as W  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import (  # noqa: E402
    WATCH_CLOCK_COLUMNS, WATCH_COLUMNS, WATCH_COLUMNS_WITHOUT_LATE_GROUPS,
    WATCH_HALF_COLUMNS)


_RUNS_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs")

# The +0x2E0 tests read a piece of a run on The Who - Won't Get Fooled Again
# (O), whose notes.mid has one drum lane (note 126) starting at tick 548960,
# chord 2108. The piece is the header and lines 110320 to 110945 of
# hydra-data\ch-mcp-runs\2026-11-11\ls-a2-run1\sp_watch-1.csv, rows
# unchanged: clock 507.009 s to 509.841 s, from just before chord 2107 to
# just after chord 2133, the lane's last. Across the whole log +0x2E0 changes
# only in these rows, once at each of the lane's 26 chords. The chords are
# indexes 2106 to 2134 of the hydra_replay score JSON made for the run, copied
# unchanged from hydra-data\ch-mcp-runs\2026-11-11\ls-a2-prep.
LANE_CSV = os.path.join(_RUNS_DIR, "ls-a2-run1", "sp_watch-1-lane2108.csv")
LANE_CHORDS_JSON = os.path.join(_RUNS_DIR, "ls-a2-prep",
                                "score-chords-2106-2134.json")
LANE_FIRST, LANE_LAST = 2108, 2133

# A piece of a run of a chart with no drum lane: pitch-dark-ratios-run1's
# sp_watch-1-act3.csv (the comment above its name in test_sp_compare.py says
# where it comes from), with Hydra's score of the same chart.
NO_LANE_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1",
                           "sp_watch-1-act3.csv")
NO_LANE_SCORE_JSON = os.path.join(_REPO_ROOT, "testdata", "replay",
                                  "test_pitch_dark_ratio-score.json")


def lane_chords(first=None, last=None):
    """The copied score chords, those from `first` to `last` when given."""
    return [c for c in W.load_chords(LANE_CHORDS_JSON)
            if (first is None or c["index"] >= first)
            and (last is None or c["index"] <= last)]


def chords_in_span(rows, chords):
    """The chords whose time lies inside the rows' clock span."""
    first_ms, last_ms = (C.s_to_ms(rows[0]["clock_s"]),
                         C.s_to_ms(rows[-1]["clock_s"]))
    return [c for c in chords if first_ms <= c["ms"] <= last_ms]


class RoundStartCheckTest(unittest.TestCase):
    """The field at +0x2E0 is the start of a drum lane's round, written when a
    hit completes one (docs/clone-hero/07-internals.md, IN-12). It is not a
    hit time, so on a real log it changes at a lane's chords and nowhere
    else."""

    def test_each_chord_of_a_real_lane_takes_its_own_change(self):
        rows = W.load_watch_rows(LANE_CSV)
        lane = lane_chords(LANE_FIRST, LANE_LAST)
        self.assertEqual(len(lane), LANE_LAST - LANE_FIRST + 1)
        lines = W.round_start_check(rows, lane)
        self.assertEqual(
            lines[0],
            f"PASS +0x2E0: {len(lane)} of {len(lane)} chords in the log have "
            f"a +0x2E0 value within {C.ROUND_START_MAX_GAP_MS:g} ms (0 chords "
            f"outside the log's clock span)")
        # The piece holds one change per lane chord and no more, so a PASS on
        # every chord means each took a change of its own.
        changes = [cur for prev, cur in zip(rows, rows[1:])
                   if not W.same_value(cur["round_start_s"],
                                       prev["round_start_s"])]
        self.assertEqual(len(changes), len(lane))

    def test_a_chord_before_the_lane_finds_no_change(self):
        rows = W.load_watch_rows(LANE_CSV)
        in_span = chords_in_span(rows, lane_chords())
        before = [c for c in in_span if c["index"] < LANE_FIRST]
        self.assertEqual(len(before), 1)
        lines = W.round_start_check(rows, lane_chords())
        self.assertTrue(lines[0].startswith(
            f"FAIL +0x2E0: {len(in_span) - 1} of {len(in_span)} "), lines[0])
        self.assertEqual(lines[-1],
                         f"  chord {before[0]['index']} at "
                         f"{before[0]['ms']:.1f} ms: no +0x2E0 change within "
                         f"{W.HIT_SEARCH_MS:g} ms")

    def test_chords_outside_the_log_are_not_judged(self):
        rows = W.load_watch_rows(LANE_CSV)
        chords = lane_chords()
        outside = [c for c in chords if c not in chords_in_span(rows, chords)]
        self.assertEqual([c["index"] for c in outside],
                         [LANE_FIRST - 2, LANE_LAST + 1])
        self.assertIn(f"({len(outside)} chords outside the log's clock span)",
                      W.round_start_check(rows, chords)[0])

    def test_a_chart_with_no_lane_never_changes_the_field(self):
        rows = W.load_watch_rows(NO_LANE_CSV)
        self.assertTrue(all(r["round_start_s"] == 0.0 for r in rows))
        chords = W.load_chords(NO_LANE_SCORE_JSON)
        in_span = chords_in_span(rows, chords)
        self.assertTrue(in_span)
        lines = W.round_start_check(rows, chords)
        self.assertTrue(lines[0].startswith(
            f"FAIL +0x2E0: 0 of {len(in_span)} "), lines[0])
        self.assertEqual(lines[1:],
                         [f"  chord {c['index']} at {c['ms']:.1f} ms: no "
                          f"+0x2E0 change within {W.HIT_SEARCH_MS:g} ms"
                          for c in in_span])


# The end of a real Star Power watch with every column: the last three reads
# before the song ended, and the final row the watcher wrote once the clock
# had held. Copied from lines 1 and 50242-50245 of
# hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-run1\sp_watch-1.csv.
FROZEN_END_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1",
                              "sp_watch-1-frozen-end.csv")

# The end of a real watch written before the half column existed: the last
# two reads and the final row. Copied unchanged from lines 1 and 50641-50643
# of hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\sp_watch-1.csv.
NO_HALF_END_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-tail.csv")

# The first three rows of a real watch written before the clock and half
# columns existed, which also calls round_start_s by its older name. Copied
# unchanged from hydra-data\ch-mcp-runs\2026-11-06\ls-d2b-run1\sp_watch-1.csv.
NO_CLOCK_HEAD_CSV = os.path.join(_RUNS_DIR, "ls-d2b-run1", "sp_watch-1-head.csv")

# The first three rows of a real watch written before the half column existed.
# Copied unchanged from hydra-data\ch-mcp-runs\2026-11-07\ls-r1-run1\sp_watch-1.csv.
NO_HALF_HEAD_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-head.csv")

# Seven rows of a real watch with every column, from
# hydra-data\ch-mcp-runs\2026-11-12\ls-g3-run1\sp_watch-1.csv (lines 2 and
# 20842-20847).
FULL_ROWS_CSV = os.path.join(_RUNS_DIR, "ls-g3-run1", "sp_watch-1.csv")


class PauseCheckTest(unittest.TestCase):
    def test_the_held_clock_at_a_real_logs_end_is_one_pause_with_its_span(self):
        rows = W.load_watch_rows(FROZEN_END_CSV)
        a, b = rows[-2], rows[-1]
        self.assertGreater(b["wall_s"] - a["wall_s"], W.PAUSE_MIN_WALL_S)
        self.assertEqual(W.find_pauses(rows), [(len(rows) - 2, len(rows) - 1)])
        lines = W.pause_check(rows)
        self.assertEqual(len(lines), 2)
        self.assertTrue(lines[0].startswith("PASS pause: "), lines[0])
        for value in (a["wall_s"], b["wall_s"], a["clock_s"], b["clock_s"]):
            self.assertIn(f"{value:.3f}", lines[0])
        self.assertEqual(lines[1], "  other fields: none moved")

    def test_a_pause_into_the_final_row_says_so(self):
        rows = W.load_watch_rows(FROZEN_END_CSV)
        end_note = "(ends at the log's final row, where the watcher stopped)"
        self.assertTrue(W.pause_check(rows)[0].endswith(end_note))
        # Without the watcher's final row the same reads hold no pause.
        self.assertEqual(W.pause_check(rows[:-1]), ["pause: no pause in this log"])

    def test_a_nan_read_held_across_the_pause_is_not_a_move(self):
        # This log has no half column, so sp_half_u64 loads NaN in every row,
        # on both sides of the watcher's final pause.
        rows = W.load_watch_rows(NO_HALF_END_CSV)
        self.assertTrue(all(math.isnan(r["sp_half_u64"]) for r in rows[-2:]))
        lines = W.pause_check(rows)
        self.assertTrue(lines[0].startswith("PASS pause: "), lines[0])
        self.assertEqual(lines[1], "  other fields: none moved")


class FilesTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        self.tmp.cleanup()

    def write_csv(self, header, rows):
        path = os.path.join(self.tmp.name, "watch.csv")
        with open(path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(header)
            w.writerows(rows)
        return path

    @staticmethod
    def header_and_cells(path):
        with open(path, newline="", encoding="utf-8") as f:
            reader = csv.reader(f)
            return next(reader), list(reader)

    def test_whole_number_cells_load_as_ints_with_every_digit(self):
        # sp_watch.py writes a 64-bit field as its whole number; a float would
        # drop low bits once a value passes 2**53, so such cells load as ints.
        header, cells = self.header_and_cells(FULL_ROWS_CSV)
        self.assertEqual(tuple(header), WATCH_COLUMNS)
        got = W.load_watch_rows(FULL_ROWS_CSV)
        self.assertEqual(len(got), len(cells))
        for loaded, texts in zip(got, cells):
            for column, text in zip(header, texts):
                value = loaded[column]
                if "." in text or "e" in text:
                    self.assertIsInstance(value, float, column)
                    self.assertEqual(value, float(text), column)
                else:
                    self.assertIsInstance(value, int, column)
                    self.assertEqual(str(value), text, column)
        for column in ("sp_amount_u64", "sp_max_u64", "sp_half_u64"):
            self.assertIsInstance(got[0][column], int, column)

    def test_a_log_written_before_the_half_column_loads_with_it_nan(self):
        header, _ = self.header_and_cells(NO_HALF_HEAD_CSV)
        self.assertNotIn("sp_half_u64", header)
        got = W.load_watch_rows(NO_HALF_HEAD_CSV)
        self.assertEqual(set(got[0]), set(WATCH_COLUMNS))
        self.assertTrue(all(math.isnan(r["sp_half_u64"]) for r in got))
        # The clock columns the log did write keep their reads.
        self.assertFalse(any(math.isnan(r[c]) for r in got
                             for c in WATCH_CLOCK_COLUMNS))
        self.assertEqual(W.pause_check(got), ["pause: no pause in this log"])

    def test_a_log_without_the_clock_or_half_columns_loads(self):
        header, _ = self.header_and_cells(NO_CLOCK_HEAD_CSV)
        self.assertFalse(set(header) & set(WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS))
        got = W.load_watch_rows(NO_CLOCK_HEAD_CSV)
        self.assertTrue(all(math.isnan(r[c]) for r in got
                            for c in WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS))
        self.assertFalse(any(math.isnan(r[c]) for r in got
                             for c in WATCH_COLUMNS_WITHOUT_LATE_GROUPS))

    def test_a_header_missing_only_part_of_a_group_is_refused(self):
        # A real full log with only raw_s, one of the three clock columns,
        # taken out of its header and rows.
        header, cells = self.header_and_cells(FULL_ROWS_CSV)
        drop = header.index("raw_s")
        path = self.write_csv(header[:drop] + header[drop + 1:],
                              [c[:drop] + c[drop + 1:] for c in cells])
        with self.assertRaises(ValueError):
            W.load_watch_rows(path)

    def test_a_saved_log_with_the_older_hit_time_s_header_loads(self):
        # The real ls-d2b log calls round_start_s by its older name.
        header, cells = self.header_and_cells(NO_CLOCK_HEAD_CSV)
        self.assertIn("hit_time_s", header)
        self.assertNotIn("round_start_s", header)
        got = W.load_watch_rows(NO_CLOCK_HEAD_CSV)
        old = header.index("hit_time_s")
        for loaded, texts in zip(got, cells):
            self.assertEqual(loaded["round_start_s"], W.parse_value(texts[old]))
            self.assertNotIn("hit_time_s", loaded)

    def test_run_reads_both_files(self):
        lines = W.run([LANE_CSV, "--score", LANE_CHORDS_JSON])
        rows = W.load_watch_rows(LANE_CSV)
        self.assertEqual(lines[0],
                         f"{os.path.basename(LANE_CSV)}: {len(rows)} rows")
        self.assertEqual(lines[1:],
                         W.round_start_check(rows, lane_chords())
                         + W.pause_check(rows))
        self.assertEqual(lines[-1], "pause: no pause in this log")


if __name__ == "__main__":
    unittest.main()
