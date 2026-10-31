"""Tests for experiments/sp_compare.py on a small hand-written run.

The score JSON is eight chords with three phrase ends and one activation
(chord 4, tick 768) that ends on chord 6 (tick 1152). The watcher rows are
built from a short list of events (phrase rises, the activation and its end)
sampled every 10 ms, the way sp_watch.py would log them. Each test changes
one event and checks the report's lines, pinned as literals.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_sp_compare
"""

from __future__ import annotations

import contextlib
import csv
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

from tools.ch_probe.experiments import sp_compare  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS  # noqa: E402


def _chord(index, tick, ms, multiplier, *, in_sp=False, phrase_end=False, fill=False):
    return {"index": index, "tick": tick, "ms": ms, "multiplier": multiplier,
            "multiplier_after": multiplier, "in_sp": in_sp,
            "is_sp_phrase_end": phrase_end, "is_fill": fill,
            "notes": [{"color": "Kick", "cymbal": False, "dynamic": "none"}]}


SCORE = {
    "chords": [
        _chord(0, 0, 1000.0, 1, phrase_end=True),
        _chord(1, 192, 1500.0, 1),
        _chord(2, 384, 2000.0, 4, phrase_end=True),
        _chord(3, 576, 2500.0, 4, fill=True),
        _chord(4, 768, 3000.0, 4, in_sp=True, fill=True),
        _chord(5, 960, 3500.0, 4, in_sp=True, phrase_end=True),
        _chord(6, 1152, 4000.0, 4, in_sp=True),
        _chord(7, 1344, 4500.0, 4),
    ],
    "activations": [{"act_tick": 768, "deact_tick": 1152}],
}

# The game credits a chord this long after its ms in the fake run.
CREDIT_MS = 5
DEFAULT_TIMES = range(500, 5001, 10)


def build_rows(times=DEFAULT_TIMES, *, amount0=0, rise_skip=(), act=(3005, 4100),
               act_clears=True, mult_doubles=True, mult_lag=0,
               sp_max=lambda t: 6144, u64_scale=1):
    """Watcher rows for the fake run, one per time in `times` (clock ms).

    A phrase end adds 1536 to the amount (clamped at sp_max) CREDIT_MS after
    its chord; an activation empties the meter, sets the flag, doubles the
    multiplier and sets the end time act[1]; the flag clears at act[1]."""
    chords = SCORE["chords"]
    events = [(c["ms"] + CREDIT_MS, "rise") for c in chords
              if c["is_sp_phrase_end"] and c["index"] not in rise_skip]
    if act is not None:
        events.append((act[0], "activate"))
    events.sort()
    rows = []
    for t in times:
        amount = amount0
        for at, kind in events:
            if at > t:
                break
            amount = min(amount + 1536, sp_max(at)) if kind == "rise" else 0
        credited = [c for c in chords if c["ms"] + CREDIT_MS <= t]
        base = credited[-1 - mult_lag]["multiplier"] if len(credited) > mult_lag else 1
        active = act is not None and t >= act[0] and (not act_clears or t < act[1])
        mult = base * (2 if active and mult_doubles else 1)
        rows.append({
            "wall_s": 100 + t / 1000, "clock_s": t / 1000, "score": 0,
            "sp_amount_u32": amount, "sp_amount_u64": amount * u64_scale,
            "sp_max_u32": sp_max(t), "sp_max_u64": sp_max(t) * u64_scale,
            "sp_active_byte": int(active), "sp_active_u32": int(active),
            "sp_mult_byte": mult, "sp_mult_u32": mult,
            "sp_end_s": act[1] / 1000 if act is not None and t >= act[0] else 0.0,
            "sp_overflow_u32": 0, "hit_time_s": 0.0,
        })
    return rows


def run(rows, **kwargs):
    kwargs.setdefault("resolution", 192)
    return sp_compare.compare(rows, SCORE, **kwargs)


MATCHING_REPORT = """\
Star Power comparison: 451 watcher rows, 8 chords, 1 path activations, width 64
PASS phrase rise after chord 0 (tick 0, 1000.0 ms): +1536 (0 -> 1536) at 1010.0 ms, full
PASS phrase rise after chord 2 (tick 384, 2000.0 ms): +1536 (1536 -> 3072) at 2010.0 ms, full
PASS phrase rise after chord 5 (tick 960, 3500.0 ms): +1536 (0 -> 1536) at 3510.0 ms, full, during Star Power
Full rises: all 1536 ticks; sp_max / 4 = 1536
PASS sp_max constant: 6144; Hydra full meter 6144 (resolution 192 x 32); sp_bar_ticks 14400
Activation 1 at clock 3010.0 ms:
PASS   activation 1 multiplier doubles: 4 -> 8 at 3010.0 ms
PASS   activation 1 end time ahead of the clock: end 4100.0 ms, clock 3010.0 ms
PASS   activation 1 flag clears at the end: end 4100.0 ms, flag 0 at 4100.0 ms
PASS   activation 1 multiplier halves at the end: 8 -> 4 at 4100.0 ms
PASS   activation 1 on the path's chord: chord 4 (tick 768, 3000.0 ms); game end 4100.0 ms, Hydra deactivation 4000.0 ms (chord 6, tick 1152), difference +100.0 ms
PASS multiplier at each chord: 8 of 8 chords match (one chord of lag allowed)
Summary: 10 PASS, 0 FAIL"""


class MatchingRunTest(unittest.TestCase):
    def test_everything_passes(self):
        report = run(build_rows(), sp_bar_ticks=14400)
        self.assertEqual(report.fails, 0)
        self.assertEqual(report.text(), MATCHING_REPORT)


class FailingRunTest(unittest.TestCase):
    def test_missing_rise_fails(self):
        report = run(build_rows(rise_skip={2}))
        self.assertIn("FAIL phrase rise after chord 2 (tick 384, 2000.0 ms): "
                      "missing (no rise from 1998.0 ms to 2100.0 ms)", report.lines)
        self.assertEqual(report.fails, 1)

    def test_flag_never_sets_fails(self):
        report = run(build_rows(act=None))
        self.assertIn("FAIL path activation 1 (act_tick 768): the game made no "
                      "activation", report.lines)
        self.assertIn("Multiplier not compared from chord 3 on: the game's "
                      "activations part from the path at 3000.0 ms", report.lines)
        self.assertEqual(report.fails, 1)

    def test_end_never_clears_fails(self):
        report = run(build_rows(act_clears=False))
        self.assertIn("FAIL   activation 1 flag clears at the end: still set at the "
                      "last row (5000.0 ms), end time 4100.0 ms", report.lines)
        self.assertEqual(report.fails, 1)

    def test_multiplier_that_stays_4_fails(self):
        report = run(build_rows(mult_doubles=False))
        self.assertIn("FAIL   activation 1 multiplier doubles: 4 before, [4] within "
                      "100 ms, expected 8", report.lines)
        self.assertIn("FAIL   activation 1 multiplier halves at the end: 4 before, "
                      "[4] within 100 ms of the end", report.lines)
        self.assertIn("FAIL multiplier at each chord: first mismatch at chord 5 "
                      "(tick 960, 3500.0 ms): game 4, expected 8 (multiplier 4, in "
                      "Star Power); 7 of 8 chords match", report.lines)
        self.assertEqual(report.fails, 3)

    def test_multiplier_4_on_an_in_sp_chord_fails(self):
        # The activation itself behaves, but Star Power is over by chord 5,
        # which Hydra pays in Star Power at multiplier 4.
        report = run(build_rows(act=(3005, 3300)))
        self.assertIn("FAIL multiplier at each chord: first mismatch at chord 5 "
                      "(tick 960, 3500.0 ms): game 4, expected 8 (multiplier 4, in "
                      "Star Power); 7 of 8 chords match", report.lines)
        self.assertEqual(report.fails, 1)


class PathTest(unittest.TestCase):
    def test_activation_one_fill_early_is_off_the_path_but_fields_pass(self):
        report = run(build_rows(act=(2505, 3600)))
        self.assertIn("FAIL   activation 1 not on the path's chord: game tick 576, "
                      "path act_tick 768", report.lines)
        for line in (
            "PASS   activation 1 multiplier doubles: 4 -> 8 at 2510.0 ms",
            "PASS   activation 1 end time ahead of the clock: end 3600.0 ms, "
            "clock 2510.0 ms",
            "PASS   activation 1 flag clears at the end: end 3600.0 ms, flag 0 at "
            "3600.0 ms",
            "PASS   activation 1 multiplier halves at the end: 8 -> 4 at 3600.0 ms",
        ):
            self.assertIn(line, report.lines)
        self.assertIn("Multiplier not compared from chord 3 on: the game's "
                      "activations part from the path at 2510.0 ms", report.lines)
        self.assertEqual(report.fails, 1)

    def test_flag_a_ms_before_the_chord_lands_on_it(self):
        # The auto-player presses early, so the flag can show on a row whose
        # clock is just before the activation chord's ms.
        times = sorted(set(DEFAULT_TIMES) | {2999})
        report = run(build_rows(times, act=(2999, 4100)))
        self.assertIn("PASS   activation 1 on the path's chord: chord 4 (tick 768, "
                      "3000.0 ms); game end 4100.0 ms, Hydra deactivation 4000.0 ms "
                      "(chord 6, tick 1152), difference +100.0 ms", report.lines)
        self.assertEqual(report.fails, 0)


class TolerancesTest(unittest.TestCase):
    def test_clamped_rise_passes(self):
        report = run(build_rows(amount0=4000))
        self.assertIn("PASS phrase rise after chord 2 (tick 384, 2000.0 ms): +608 "
                      "(5536 -> 6144) at 2010.0 ms, clamped", report.lines)
        self.assertIn("Full rises: all 1536 ticks; sp_max / 4 = 1536", report.lines)
        self.assertEqual(report.fails, 0)

    def test_multiplier_one_chord_late_passes(self):
        report = run(build_rows(mult_lag=1))
        self.assertIn("PASS multiplier at each chord: 8 of 8 chords match (one chord "
                      "of lag allowed)", report.lines)
        self.assertEqual(report.fails, 0)

    def test_changing_sp_max_fails(self):
        report = run(build_rows(sp_max=lambda t: 6144 if t < 2500 else 7000),
                     sp_bar_ticks=14400)
        self.assertIn("FAIL sp_max not constant: values 6144, 7000; Hydra full meter "
                      "6144 (resolution 192 x 32); sp_bar_ticks 14400", report.lines)

    def test_hydra_full_meter(self):
        self.assertEqual(sp_compare.hydra_full_meter_ticks(192), 6144)


class FilesTest(unittest.TestCase):
    def _write(self, folder, rows):
        csv_path = os.path.join(folder, "watch.csv")
        with open(csv_path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(WATCH_COLUMNS)
            for r in rows:
                w.writerow([r[c] for c in WATCH_COLUMNS])
        json_path = os.path.join(folder, "score.json")
        with open(json_path, "w", encoding="utf-8") as f:
            json.dump(SCORE, f)
        return csv_path, json_path

    def _main(self, argv):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = sp_compare.main(argv)
        return code, out.getvalue()

    def test_main_reads_the_files_and_exits_0(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            code, text = self._main([csv_path, json_path, "--resolution", "192",
                                     "--sp-bar-ticks", "14400"])
        self.assertEqual(code, 0)
        self.assertEqual(text, MATCHING_REPORT + "\n")

    def test_main_exits_1_on_a_fail(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows(rise_skip={2}))
            code, _ = self._main([csv_path, json_path, "--resolution", "192"])
        self.assertEqual(code, 1)

    def test_width_defaults_to_64_and_32_reads_the_u32_columns(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows(u64_scale=2))
            _, text64 = self._main([csv_path, json_path, "--resolution", "192"])
            _, text32 = self._main([csv_path, json_path, "--resolution", "192",
                                    "--width", "32"])
        self.assertIn("PASS sp_max constant: 12288; Hydra full meter 6144 "
                      "(resolution 192 x 32)\n", text64)
        self.assertIn("PASS sp_max constant: 6144; Hydra full meter 6144 "
                      "(resolution 192 x 32)\n", text32)

    def test_missing_column_is_an_error(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "watch.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                f.write("wall_s,clock_s\n1.0,0.5\n")
            with self.assertRaises(ValueError):
                sp_compare.load_watch_rows(path)

    def test_main_runs_with_no_resolution(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            code, text = self._main([csv_path, json_path])
        self.assertEqual(code, 0)
        self.assertIn("PASS sp_max constant: 6144; Hydra full meter: not computed "
                      "(no resolution given)\n", text)


NO_RESOLUTION_LINE = ("PASS sp_max constant: 6144; Hydra full meter: not computed "
                      "(no resolution given); sp_bar_ticks 14400")
RESOLUTION_LINE = ("PASS sp_max constant: 6144; Hydra full meter 6144 (resolution "
                   "192 x 32); sp_bar_ticks 14400")


class NoResolutionTest(unittest.TestCase):
    def test_sp_max_line_says_not_computed_and_the_rest_is_unchanged(self):
        report = run(build_rows(), resolution=None, sp_bar_ticks=14400)
        self.assertEqual(report.text(),
                         MATCHING_REPORT.replace(RESOLUTION_LINE, NO_RESOLUTION_LINE))
        self.assertEqual(report.fails, 0)


def _row(clock_s, amount, *, active=0, sp_max=6144):
    """One hand-written watcher row; only the fields the row checks read vary."""
    return {
        "wall_s": 100.0 + clock_s, "clock_s": clock_s, "score": 0,
        "sp_amount_u32": amount, "sp_amount_u64": amount,
        "sp_max_u32": sp_max, "sp_max_u64": sp_max,
        "sp_active_byte": active, "sp_active_u32": active,
        "sp_mult_byte": 1, "sp_mult_u32": 1,
        "sp_end_s": 0.0, "sp_overflow_u32": 0, "hit_time_s": 0.0,
    }


AMOUNT = "sp_amount_u64"
MAX = "sp_max_u64"


class RowChecksTest(unittest.TestCase):
    def test_old_name_is_the_public_finder(self):
        self.assertIs(sp_compare._find_activations, sp_compare.find_activations)

    def test_column_names_follow_the_width(self):
        self.assertEqual(sp_compare.column_names(), (AMOUNT, MAX))
        self.assertEqual(sp_compare.column_names(32),
                         ("sp_amount_u32", "sp_max_u32"))

    def test_is_active_reads_the_flag_byte_only(self):
        row = _row(1.0, 0, active=1)
        self.assertTrue(sp_compare.is_active(row))
        row["sp_active_u32"] = 257
        row["sp_active_byte"] = 0
        self.assertFalse(sp_compare.is_active(row))

    def test_ready_fraction_is_half_a_meter(self):
        self.assertEqual(sp_compare.READY_FRACTION, 0.5)

    def test_phrase_rises_gives_each_step_up(self):
        rows = [_row(1.0, 0), _row(1.5, 1536), _row(2.0, 1536), _row(2.5, 3072),
                _row(3.0, 0, active=1), _row(3.5, 1536, active=1)]
        rises = sp_compare.phrase_rises(rows, AMOUNT)
        self.assertEqual(rises, [(1500.0, 0, 1536), (2500.0, 1536, 3072),
                                 (3500.0, 0, 1536)])
        self.assertEqual([r.size for r in rises], [1536, 1536, 1536])

    def test_phrase_rises_on_three_sizes(self):
        rows = [_row(1.0, 0), _row(1.5, 1536), _row(2.0, 4608), _row(2.5, 6144)]
        sizes = [r.size for r in sp_compare.phrase_rises(rows, AMOUNT)]
        self.assertEqual(sizes, [1536, 3072, 1536])

    def test_flips_between_keeps_only_the_flip_inside(self):
        rows = [_row(1.0, 3072), _row(2.0, 0, active=1), _row(2.5, 0),
                _row(4.0, 3072), _row(5.0, 0, active=1), _row(6.0, 0)]
        flips = sp_compare.flips_between(rows, 1500.0, 3000.0)
        self.assertEqual([(a.number, a.flip_row, a.drop_row) for a in flips],
                         [(1, 1, 2)])
        self.assertEqual(sp_compare.flips_between(rows, 2500.0, 4500.0), [])

    def test_flips_between_includes_both_ends(self):
        rows = [_row(1.0, 3072), _row(2.0, 0, active=1), _row(2.5, 0)]
        self.assertEqual(len(sp_compare.flips_between(rows, 2000.0, 2000.0)), 1)

    def test_ready_between_finds_a_row_at_exactly_half(self):
        rows = [_row(1.0, 1536), _row(2.0, 3072), _row(3.0, 4608)]
        got = sp_compare.ready_between(rows, 1500.0, 3500.0, AMOUNT, MAX)
        self.assertEqual(got.ready_ms, 2000.0)
        self.assertEqual(got.highest, 4608)
        self.assertEqual(got.sp_max, 6144)
        self.assertEqual(got.rows, 2)

    def test_ready_between_finds_none_one_unit_below_half(self):
        rows = [_row(1.0, 1536), _row(2.0, 3071)]
        got = sp_compare.ready_between(rows, 500.0, 2500.0, AMOUNT, MAX)
        self.assertIsNone(got.ready_row)
        self.assertIsNone(got.ready_ms)
        self.assertEqual(got.highest, 3071)

    def test_ready_between_finds_none_at_half_with_the_flag_set(self):
        rows = [_row(1.0, 1536), _row(2.0, 3072, active=1)]
        got = sp_compare.ready_between(rows, 500.0, 2500.0, AMOUNT, MAX)
        self.assertIsNone(got.ready_row)
        self.assertEqual(got.highest, 3072)

    def test_ready_between_never_calls_an_empty_meter_ready(self):
        rows = [_row(1.0, 0, sp_max=0), _row(2.0, 0, sp_max=0)]
        got = sp_compare.ready_between(rows, 500.0, 2500.0, AMOUNT, MAX)
        self.assertIsNone(got.ready_row)
        self.assertEqual((got.rows, got.highest, got.sp_max), (2, 0, 0))

    def test_ready_between_ignores_rows_outside_the_window(self):
        rows = [_row(1.0, 6144), _row(2.0, 1536), _row(3.0, 6144)]
        got = sp_compare.ready_between(rows, 1500.0, 2500.0, AMOUNT, MAX)
        self.assertIsNone(got.ready_row)
        self.assertEqual(got.highest, 1536)

    def test_ready_between_on_an_empty_window(self):
        got = sp_compare.ready_between([_row(1.0, 3072)], 2000.0, 3000.0, AMOUNT, MAX)
        self.assertEqual((got.rows, got.highest, got.sp_max, got.ready_row),
                         (0, None, None, None))


if __name__ == "__main__":
    unittest.main()
