"""Tests for experiments/sp_compare.py on a small hand-written run.

The score JSON is eight chords with three phrase ends and one activation
(chord 4, tick 768) that ends on chord 6 (tick 1152). The watcher rows have
the columns sp_watch.py logs. They are built from a short list of events
(phrase rises, the activation and its end) sampled at DEFAULT_TIMES, this
file's own grid and not the watcher's timing. Each test changes one event and
checks the report's lines, pinned as literals.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_sp_compare
"""

from __future__ import annotations

import contextlib
import csv
import io
import json
import math
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
from tools.ch_probe.tests.test_play_chart import _schedule_of  # noqa: E402


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
            "sp_overflow_u32": 0, "round_start_s": 0.0,
            "audio_s": math.nan, "raw_s": math.nan, "corr_s": math.nan,
        })
    return rows


def _write_schedule(folder):
    """SCORE's hand-built schedule (test_play_chart._schedule_of: one plain
    press per chord), written to `folder`; its path."""
    path = os.path.join(folder, "schedule.json")
    with open(path, "w", encoding="utf-8") as f:
        json.dump(_schedule_of(SCORE), f)
    return path


def _score_reach():
    with tempfile.TemporaryDirectory() as folder:
        return sp_compare.press_reach(_write_schedule(folder))


def run(rows, **kwargs):
    kwargs.setdefault("resolution", 192)
    kwargs.setdefault("reach", _score_reach())
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
    def test_a_log_without_the_clock_columns_loads_with_them_nan(self):
        old = [c for c in WATCH_COLUMNS if c not in ("audio_s", "raw_s", "corr_s")]
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "old.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                w = csv.writer(f)
                w.writerow(old)
                w.writerow([7 for _ in old])
            rows = sp_compare.load_watch_rows(path)
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["score"], 7)
        self.assertTrue(all(math.isnan(rows[0][c])
                            for c in ("audio_s", "raw_s", "corr_s")))

    def test_a_saved_log_with_the_older_hit_time_s_header_loads(self):
        old = ["hit_time_s" if c == "round_start_s" else c
               for c in WATCH_COLUMNS if c not in ("audio_s", "raw_s", "corr_s")]
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "saved.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                w = csv.writer(f)
                w.writerow(old)
                w.writerow(range(len(old)))
            rows = sp_compare.load_watch_rows(path)
        self.assertEqual(rows[0]["round_start_s"], old.index("hit_time_s"))
        self.assertNotIn("hit_time_s", rows[0])

    def test_a_log_lacking_another_column_is_still_refused(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "bad.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                csv.writer(f).writerow(("wall_s", "clock_s"))
            with self.assertRaises(ValueError):
                sp_compare.load_watch_rows(path)

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
        # Every run names SCORE's schedule; a file of one folder is enough.
        out = io.StringIO()
        with tempfile.TemporaryDirectory() as folder:
            with contextlib.redirect_stdout(out):
                code = sp_compare.main(argv + ["--schedule", _write_schedule(folder)])
        return code, out.getvalue()

    def test_main_needs_the_schedule(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            with contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    sp_compare.main([csv_path, json_path])

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
        "sp_end_s": 0.0, "sp_overflow_u32": 0, "round_start_s": 0.0,
        "audio_s": math.nan, "raw_s": math.nan, "corr_s": math.nan,
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

    def test_a_flip_goes_to_its_own_chord_not_one_17_ms_after(self):
        # Chord 1 is pressed at 998 ms and chord 2, 17.2 ms later, at 1002.2
        # ms. A flip 1 ms after chord 1 belongs to chord 1; chord 2's earlier
        # press must not take it.
        chords = [{"index": 1, "tick": 100, "ms": 1000.0},
                  {"index": 2, "tick": 200, "ms": 1017.2}]
        reach = {1: sp_compare.PressReach(998.0, 998.0),
                 2: sp_compare.PressReach(1002.2, 1002.2)}
        chord_ms = [c["ms"] for c in chords]
        got = sp_compare._landed_chord(chords, chord_ms, 1001.0, reach)
        self.assertIs(got, chords[0])
        # A flip at 1003 ms is after chord 2's press (1002.2).
        got = sp_compare._landed_chord(chords, chord_ms, 1003.0, reach)
        self.assertIs(got, chords[1])

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


def _fixture(name):
    return os.path.join(_HERE, name)


# Each path's score file and its hydra_replay schedule file.
_MOUTHS = (_fixture("mouths_refine_score.json"), _fixture("mouths_refine_schedule.json"))
_BOMBER = (_fixture("bomber_refine_score.json"), _fixture("bomber_refine_schedule.json"))


def _rounded(reach):
    return (round(reach.earliest_ms, 2), round(reach.latest_ms, 2))


def _fixture_chords(score_path):
    """The fixture file's chords as they are: the phrase-end flag, in_sp and
    multipliers are the refine-prep score file's own."""
    with open(score_path, encoding="utf-8") as f:
        return json.load(f)["chords"]


def _rise_lines(score_path, reach, rows):
    report = sp_compare.Report()
    sp_compare._check_phrase_rises(report, rows, _fixture_chords(score_path),
                                   AMOUNT, MAX, reach)
    return report.lines


class PressReachTest(unittest.TestCase):
    """press_reach takes each chord's press times from the schedule file:
    each press's chord `ms` plus its `aim_ms`. Every reach below is those two
    fields of the fixture, added and rounded to 0.01 ms."""

    def test_mouths_740_reaches_its_late_kick_press(self):
        reach = sp_compare.press_reach(_MOUTHS[1])
        # Chord 740 is at 118425.95080264665 ms. Its cymbals are aimed
        # -22.60600950299704 ms (squeeze_out_inside), its kick
        # 7.39399049700296 ms (squeeze_out_late).
        self.assertEqual(_rounded(reach[740]), (118403.34, 118433.34))

    def test_mouths_740_is_credited_at_its_second_press(self):
        reach = sp_compare.press_reach(_MOUTHS[1])
        rows = [_row(118.300, 0), _row(118.412, 0), _row(118.441, 1536)]
        self.assertIn("PASS phrase rise after chord 740 (tick 48480, 118426.0 ms): "
                      "+1536 (0 -> 1536) at 118441.0 ms, full",
                      _rise_lines(_MOUTHS[0], reach, rows))

    def test_bomber_147_reaches_its_solved_early_press(self):
        reach = sp_compare.press_reach(_BOMBER[1])
        # 147 is an activation and chord 180's squeeze anchor, so the
        # schedule presses it in two groups: the yellow cymbal aimed
        # -62.42936351273587 ms from 27333.333333333332 ms, then the kick
        # 8 ms later (after_ms 8.0).
        self.assertEqual(_rounded(reach[147]), (27270.90, 27278.90))
        # A flip after 147's press but before its ms lands on 147, not on
        # chord 146 83 ms before it.
        chords = _fixture_chords(_BOMBER[0])
        chord_ms = [c["ms"] for c in chords]
        got = sp_compare._landed_chord(chords, chord_ms, 27285.0, reach)
        self.assertEqual(got["index"], 147)

    def test_bomber_180_reaches_its_late_press(self):
        reach = sp_compare.press_reach(_BOMBER[1])
        # Chord 180 is at 32583.333333333336 ms, aimed 38.903969820592465 ms
        # (squeeze_out_late).
        self.assertEqual(_rounded(reach[180]), (32622.24, 32622.24))
        rows = [_row(32.500, 0), _row(32.640, 1536)]
        self.assertIn("PASS phrase rise after chord 180 (tick 18768, 32583.3 ms): "
                      "+1536 (0 -> 1536) at 32640.0 ms, full",
                      _rise_lines(_BOMBER[0], reach, rows))

    def test_senri_with_no_squeeze_reaches_its_one_press(self):
        # Chord 268 (31860.680000000004 ms) ends an activation and is aimed
        # -15.0 ms (sp_end); chord 267 (31759.329000000005 ms) is aimed
        # -2.0 ms (plain). Each has one press.
        reach = sp_compare.press_reach(_fixture("senri_schedule.json"))
        self.assertEqual(_rounded(reach[268]), (31845.68, 31845.68))
        self.assertEqual(_rounded(reach[267]), (31757.33, 31757.33))

    def test_a_schedule_that_refuses_its_path_is_refused(self):
        doc = _schedule_of(SCORE)
        doc["refusals"] = ["chord 3 is a squeeze the schedule could not split, so it is not pressed"]
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "schedule.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump(doc, f)
            with self.assertRaisesRegex(ValueError, "refuses to play this path"):
                sp_compare.press_reach(path)


def _samples(*pairs):
    """Rows holding only the two columns gap_summary reads, from (wall_s,
    clock_s) pairs."""
    return [{"wall_s": wall_s, "clock_s": clock_s} for wall_s, clock_s in pairs]


# The wall_s and clock_s columns of a real run's watch CSV. The copy reads them
# by name, because the other columns are left out.
LS_O2_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_watch", "ls-o2-run1-sp_watch-1.csv")


class GapSummaryTest(unittest.TestCase):
    def test_two_gaps_give_the_count_the_longest_and_the_clocks_around_them(self):
        got = sp_compare.gap_summary(
            _samples((100.0, 1.0), (100.004, 1.004), (100.064, 1.064),
                     (100.068, 1.068), (100.312, 1.312), (100.316, 1.316)), 50)
        self.assertEqual(got.count, 2)
        self.assertAlmostEqual(got.longest_ms, 244.0, places=6)
        self.assertAlmostEqual(got.from_ms, 1004.0, places=6)
        self.assertAlmostEqual(got.to_ms, 1312.0, places=6)

    def test_rows_with_no_gap_over_give_0(self):
        rows = _samples((100.0, 1.0), (100.004, 1.004), (100.008, 1.008))
        self.assertEqual(sp_compare.gap_summary(rows, 50), (0, None, None, None))

    def test_no_rows_and_one_row_give_0(self):
        self.assertEqual(sp_compare.gap_summary([], 50), (0, None, None, None))
        self.assertEqual(sp_compare.gap_summary(_samples((100.0, 1.0)), 50),
                         (0, None, None, None))

    def test_a_gap_of_exactly_the_threshold_does_not_count(self):
        rows = _samples((0.0, 1.0), (0.05, 1.05))
        self.assertEqual(sp_compare.gap_summary(rows, 50), (0, None, None, None))

    def test_the_boundary_holds_at_a_real_wall_time(self):
        # 0.0625 s is exact in binary even on a wall time this large, so this
        # gap is exactly the threshold, with no rounding left in the subtraction.
        rows = _samples((1794062016.0, 1.0), (1794062016.0625, 1.0625))
        self.assertEqual(sp_compare.gap_summary(rows, 62.5).count, 0)
        self.assertEqual(sp_compare.gap_summary(rows, 62.4).count, 1)

    # 1.0625 s is exact in binary, so its clock is exactly 1062.5 ms and these
    # boundary tests have no rounding left in the comparison.

    def test_a_gap_whose_later_row_sits_exactly_at_the_song_end_counts(self):
        rows = _samples((0.0, 1.0), (0.1, 1.0625))
        got = sp_compare.gap_summary(rows, 50, 1062.5)
        self.assertEqual(got.count, 1)
        self.assertAlmostEqual(got.to_ms, 1062.5, places=6)

    def test_a_gap_whose_later_row_is_past_the_song_end_does_not_count(self):
        rows = _samples((0.0, 1.0), (0.1, 1.0625))
        self.assertEqual(sp_compare.gap_summary(rows, 50, 1062.4),
                         (0, None, None, None))

    def test_the_row_after_the_song_end_row_is_left_out(self):
        rows = _samples((0.0, 1.0), (0.1, 1.0625), (0.4, 1.125))
        got = sp_compare.gap_summary(rows, 50, 1062.5)
        self.assertEqual(got.count, 1)
        self.assertAlmostEqual(got.longest_ms, 100.0, places=6)
        self.assertAlmostEqual(got.to_ms, 1062.5, places=6)

    def test_a_gap_with_both_rows_past_the_song_end_does_not_count(self):
        rows = _samples((0.0, 2.0), (0.1, 2.5))
        self.assertEqual(sp_compare.gap_summary(rows, 50, 1000.0),
                         (0, None, None, None))

    def test_a_final_frozen_clock_row_is_left_out(self):
        # The last two rows show one clock, past the song's end, five seconds
        # apart: the shape of a run that ended on a stopped clock.
        rows = _samples((100.0, 4.9), (100.06, 4.96), (100.064, 5.02), (105.1, 5.02))
        got = sp_compare.gap_summary(rows, 50, 5000.0)
        self.assertEqual(got.count, 1)
        self.assertAlmostEqual(got.longest_ms, 60.0, places=3)
        self.assertAlmostEqual(got.to_ms, 4960.0, places=6)

    def test_with_no_song_end_every_row_counts(self):
        rows = _samples((100.0, 4.9), (100.004, 5.02), (105.1, 5.02))
        got = sp_compare.gap_summary(rows, 50, None)
        self.assertEqual(got.count, 1)
        self.assertAlmostEqual(got.longest_ms, 5096.0, places=3)

    @staticmethod
    def _ls_o2_rows():
        with open(LS_O2_CSV, newline="", encoding="utf-8") as f:
            return [{c: float(r[c]) for c in ("wall_s", "clock_s")}
                    for r in csv.DictReader(f)]

    def test_the_real_ls_o2_run_inside_its_song(self):
        # The run folder is hydra-data/ch-mcp-runs/2026-11-07/ls-o2-run1. Its
        # ch_start printed "song length: 62080 ms".
        rows = self._ls_o2_rows()
        self.assertEqual(len(rows), 6240)
        got = sp_compare.gap_summary(rows, 50, 62080)
        self.assertEqual(got.count, 360)
        self.assertAlmostEqual(got.longest_ms, 243.7, places=1)
        self.assertAlmostEqual(got.from_ms, 2146.834, places=3)
        self.assertAlmostEqual(got.to_ms, 40566.2, delta=0.05)

    def test_the_real_ls_o2_run_with_no_song_end(self):
        # The longest gap is the last pair of rows: both show the same clock,
        # because the watch writes its final row once live.StoppedCheck says
        # the clock has stopped.
        got = sp_compare.gap_summary(self._ls_o2_rows(), 50)
        self.assertEqual(got.count, 371)
        self.assertAlmostEqual(got.longest_ms, 5084.784, places=3)
        self.assertAlmostEqual(got.from_ms, 2146.834, places=3)
        self.assertAlmostEqual(got.to_ms, 63700.788, places=3)


if __name__ == "__main__":
    unittest.main()
