"""Tests for experiments/sp_compare.py on a small hand-written run.

The score JSON is eight chords with three phrase ends and one activation
(chord 4, tick 768) that ends on chord 6 (tick 1152, 4000 ms). The chart's
resolution is 192, so the game's full meter is 5,760 (SP-11). The watcher rows
have the columns sp_watch.py logs. They are built from a short list of events
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
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import play_chart, sp_compare  # noqa: E402
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
    "activations": [{"act_tick": 768, "deact_tick": 1152, "deact_ms": 4000.0}],
}

RESOLUTION = 192
# The game's full meter at RESOLUTION (SP-11: 30 x 192), and a phrase's
# quarter of it; hand-typed so the fixture does not use the code it tests.
FULL_METER = 5760
PHRASE = 1440

# The game credits a chord this long after its ms in the fake run.
CREDIT_MS = 5
DEFAULT_TIMES = range(500, 5001, 10)


def build_rows(times=DEFAULT_TIMES, *, amount0=0, rise_skip=(), act=(3005, 4100),
               act_clears=True, mult_doubles=True, mult_lag=0,
               sp_max=lambda t: FULL_METER, u64_scale=1):
    """Watcher rows for the fake run, one per time in `times` (clock ms).

    A phrase end adds PHRASE to the amount (clamped at sp_max) CREDIT_MS after
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
            amount = min(amount + PHRASE, sp_max(at)) if kind == "rise" else 0
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
    kwargs.setdefault("resolution", RESOLUTION)
    kwargs.setdefault("reach", _score_reach())
    kwargs.setdefault("stop", play_chart.STOP_DONE)
    return sp_compare.compare(rows, SCORE, **kwargs)


MATCHING_REPORT = """\
Summary: 12 PASS, 0 FAIL
Star Power comparison: 451 watcher rows, 8 chords, 1 path activations, width 64
PASS run end: the run reached the song's end
PASS phrase rise after chord 0 (tick 0, 1000.0 ms): +1440 (0 -> 1440) at 1010.0 ms, full
PASS phrase rise after chord 2 (tick 384, 2000.0 ms): +1440 (1440 -> 2880) at 2010.0 ms, full
PASS phrase rise after chord 5 (tick 960, 3500.0 ms): +1440 (0 -> 1440) at 3510.0 ms, full, during Star Power
Full rises: all 1440 ticks; sp_max / 4 = 1440
PASS sp_max 5760; the game's full meter 5760 (resolution 192 x 30); sp_bar_ticks 14400
Activation 1 at clock 3010.0 ms:
PASS   activation 1 multiplier doubles: 4 -> 8 at 3010.0 ms
PASS   activation 1 end time ahead of the clock: end 4100.0 ms, clock 3010.0 ms
PASS   activation 1 flag clears at Hydra's end: Hydra 4000.0 ms, flag 0 at 4100.0 ms
PASS   activation 1 multiplier halves at the end: 8 -> 4 at 4100.0 ms
PASS   activation 1 on the path's chord: chord 4 (tick 768, 3000.0 ms)
PASS   activation 1 end chord: game end 4100.0 ms, the game's last chord inside: chord 6 (tick 1152, 4000.0 ms); Hydra's deactivation: chord 6 (tick 1152, 4000.0 ms), difference +100.0 ms
PASS multiplier at each chord: 8 of 8 chords match (one chord of lag allowed)"""


class MatchingRunTest(unittest.TestCase):
    def test_everything_passes(self):
        report = run(build_rows(), sp_bar_ticks=14400)
        self.assertEqual(report.fails, 0)
        self.assertEqual(report.text(), MATCHING_REPORT)

    def test_summary_is_the_first_line(self):
        self.assertEqual(run(build_rows()).lines[0], "Summary: 12 PASS, 0 FAIL")
        failing = run(build_rows(rise_skip={2}))
        self.assertEqual(failing.lines[0], "Summary: 11 PASS, 1 FAIL")
        self.assertEqual(sum(1 for line in failing.lines if line.startswith("Summary")), 1)


class RunEndTest(unittest.TestCase):
    def test_a_run_that_stopped_before_the_song_end_fails(self):
        report = run(build_rows(), stop=play_chart.STOP_NO_ENGINE,
                     stop_message="no engine after 30 s")
        self.assertIn("FAIL run end: the run stopped before the song's end "
                      "(no engine): no engine after 30 s", report.lines)
        self.assertEqual(report.fails, 1)

    def test_a_run_with_no_stop_reason_fails(self):
        report = run(build_rows(), stop=None)
        self.assertIn("FAIL run end: the run's stop reason was not given", report.lines)
        self.assertEqual(report.fails, 1)


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
        self.assertIn("FAIL   activation 1 end chord: the flag never cleared; Hydra's "
                      "deactivation: chord 6 (tick 1152, 4000.0 ms)", report.lines)
        self.assertEqual(report.fails, 2)

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

    def test_star_power_over_before_hydras_end_fails_three_ways(self):
        # The game's Star Power ends at 3300 ms, 700 ms before Hydra's 4000:
        # the flag clears early, the last chord inside is chord 4 and not
        # chord 6, and chord 5, which Hydra pays in Star Power, gets 4.
        report = run(build_rows(act=(3005, 3300)))
        self.assertIn("FAIL   activation 1 flag clears at Hydra's end: Hydra 4000.0 ms, "
                      "flag 0 at 3300.0 ms", report.lines)
        self.assertIn("FAIL   activation 1 end chord: game end 3300.0 ms, the game's "
                      "last chord inside: chord 4 (tick 768, 3000.0 ms); Hydra's "
                      "deactivation: chord 6 (tick 1152, 4000.0 ms), difference "
                      "-700.0 ms", report.lines)
        self.assertIn("FAIL multiplier at each chord: first mismatch at chord 5 "
                      "(tick 960, 3500.0 ms): game 4, expected 8 (multiplier 4, in "
                      "Star Power); 7 of 8 chords match", report.lines)
        self.assertEqual(report.fails, 3)

    def test_zero_of_zero_fails(self):
        # One row, before any chord's press: no chord is compared.
        report = sp_compare.Report()
        sp_compare._check_multipliers(report, [_row(0.5, 0)], SCORE["chords"],
                                      None, _score_reach())
        self.assertEqual(report.lines[0], "FAIL multiplier at each chord: 0 of 0 "
                                          "chords match (one chord of lag allowed)")
        self.assertEqual(report.fails, 1)

    def test_an_activation_on_at_row_0_is_reported(self):
        # The watch starts at 3010 ms, after the activation at 3005 ms.
        rows = build_rows(range(3010, 5001, 10))
        found = sp_compare.find_activations(rows)
        self.assertEqual([(a.number, a.flip_row, a.drop_row) for a in found],
                         [(1, 0, 109)])
        report = run(rows)
        self.assertIn("FAIL   activation 1 was already on at the watch's first row "
                      "(3010.0 ms); its start was not seen", report.lines)


class PathTest(unittest.TestCase):
    def test_activation_one_fill_early_is_off_the_path(self):
        report = run(build_rows(act=(2505, 3600)))
        self.assertIn("FAIL   activation 1 not on the path's chord: game tick 576, "
                      "path act_tick 768", report.lines)
        for line in (
            "PASS   activation 1 multiplier doubles: 4 -> 8 at 2510.0 ms",
            "PASS   activation 1 end time ahead of the clock: end 3600.0 ms, "
            "clock 2510.0 ms",
            "FAIL   activation 1 flag clears at Hydra's end: Hydra 4000.0 ms, flag 0 "
            "at 3600.0 ms",
            "PASS   activation 1 multiplier halves at the end: 8 -> 4 at 3600.0 ms",
        ):
            self.assertIn(line, report.lines)
        self.assertFalse(any("end chord" in line for line in report.lines))
        self.assertIn("Multiplier not compared from chord 3 on: the game's "
                      "activations part from the path at 2510.0 ms", report.lines)
        self.assertEqual(report.fails, 2)

    def test_flag_a_ms_before_the_chord_lands_on_it(self):
        # The auto-player presses early, so the flag can show on a row whose
        # clock is just before the activation chord's ms.
        times = sorted(set(DEFAULT_TIMES) | {2999})
        report = run(build_rows(times, act=(2999, 4100)))
        self.assertIn("PASS   activation 1 on the path's chord: chord 4 (tick 768, "
                      "3000.0 ms)", report.lines)
        self.assertEqual(report.fails, 0)


# LS-M2's activation 2 (hydra-data/ch-mcp-runs/2026-11-08/ls-m2-run1): the row
# before the flip, the flip, the last row with the flag set and the row it
# clears on, copied from that run's sp_watch-2.csv (data rows 34704, 34705,
# 36289, 36290).
LS_M2_ACT2_CSV = """\
wall_s,clock_s,score,sp_amount_u32,sp_amount_u64,sp_max_u32,sp_max_u64,sp_active_byte,sp_active_u32,sp_mult_byte,sp_mult_u32,sp_end_s,sp_overflow_u32,round_start_s,audio_s,raw_s,corr_s
1794158598.3915508,156.35033088494401,275945,7200,7200,14400,14400,0,65536,4,4,0.0,1,0.0,156.3490625,156.35033088494401,7.236376404762268e-07
1794158598.395959,156.35639098487172,276465,7200,7200,14400,14400,1,65537,8,8,163.47694157434088,0,0.0,156.35510416666668,156.35639098487172,7.236376404762268e-07
1794158605.5179918,163.47676531263005,302915,1,1,14400,14400,1,65537,8,8,163.47694157434088,0,0.0,163.47527083333333,163.47676531263005,7.236376404762268e-07
1794158605.522446,163.4828281124619,302915,0,0,14400,14400,0,65536,4,4,0.0,0,0.0,163.481375,163.4828281124619,7.236376404762268e-07
"""

# The path's activation 2 and six of its chords, copied from the score file of
# the path that run played (hydra-data/ch-mcp-runs/2026-11-08/kick-prep/who):
# chord 609 is the activation, 655 and 656 sit either side of the game's end,
# and 680 is Hydra's deactivation chord.
LS_M2_ACT2 = {"act_tick": 168960, "deact_tick": 180480,
              "deact_ms": 167027.11399999994}
LS_M2_CHORDS = [
    _chord(609, 168960, 156354.29199999996, 4, in_sp=True),
    _chord(654, 176560, 163401.1258333333, 4, in_sp=True),
    _chord(655, 176640, 163474.87199999994, 4, in_sp=True),
    _chord(656, 176880, 163696.11049999995, 4, in_sp=True),
    _chord(679, 180240, 166806.03849999994, 4, in_sp=True),
    _chord(680, 180480, 167027.11399999994, 4, in_sp=True),
]


class LsM2EndTest(unittest.TestCase):
    def test_the_end_check_fails_when_the_last_paid_chord_is_not_hydras_deact_chord(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "watch.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                f.write(LS_M2_ACT2_CSV)
            rows = sp_compare.load_watch_rows(path)
        act = sp_compare._Activation(2, 1, 3)
        # Each chord pressed 2 ms early, the same hand-picked aim as
        # test_play_chart._schedule_of; the run's own schedule is not kept.
        reach = {c["index"]: sp_compare.PressReach(c["ms"] - 2.0, c["ms"] - 2.0)
                 for c in LS_M2_CHORDS}
        report = sp_compare.Report()
        sp_compare._check_activation(report, rows, act, LS_M2_ACT2["deact_ms"])
        sp_compare._check_end_chord(report, rows, act, LS_M2_ACT2, LS_M2_CHORDS, reach)
        # The flag clears 3,544 ms before Hydra's end (the run's figure); the
        # game's stored end is 6 ms earlier still.
        self.assertEqual(report.lines, [
            "PASS   activation 2 multiplier doubles: 4 -> 8 at 156356.4 ms",
            "PASS   activation 2 end time ahead of the clock: end 163476.9 ms, "
            "clock 156356.4 ms",
            "FAIL   activation 2 flag clears at Hydra's end: Hydra 167027.1 ms, "
            "flag 0 at 163482.8 ms",
            "PASS   activation 2 multiplier halves at the end: 8 -> 4 at 163482.8 ms",
            "FAIL   activation 2 end chord: game end 163476.9 ms, the game's last "
            "chord inside: chord 655 (tick 176640, 163474.9 ms); Hydra's "
            "deactivation: chord 680 (tick 180480, 167027.1 ms), difference "
            "-3550.2 ms",
        ])
        self.assertEqual(report.fails, 2)

    def test_a_chord_pressed_exactly_at_the_end_is_inside(self):
        # Hydra pays a chord at the end (paid_by_sp_walk in
        # src/core/backend_value.h); the judge's game side does the same.
        rows = [_row(1.0, 0), _row(1.5, 0, active=1), _row(2.0, 0, active=1),
                _row(2.1, 0)]
        rows[2]["sp_end_s"] = 2.0
        chords = [_chord(1, 100, 1500.0, 4, in_sp=True),
                  _chord(2, 200, 2000.0, 4, in_sp=True),
                  _chord(3, 300, 2500.0, 4)]
        reach = {c["index"]: sp_compare.PressReach(c["ms"], c["ms"]) for c in chords}
        report = sp_compare.Report()
        sp_compare._check_end_chord(report, rows, sp_compare._Activation(1, 1, 3),
                                    {"act_tick": 100, "deact_tick": 200},
                                    chords, reach)
        self.assertEqual(report.fails, 0)
        self.assertIn("the game's last chord inside: chord 2 (tick 200, 2000.0 ms)",
                      report.lines[0])

    def test_hydras_chord_is_the_last_in_sp_one_in_the_window(self):
        # A chord on deact_tick that Hydra does not pay in Star Power is not
        # Hydra's deactivation chord.
        rows = [_row(1.0, 0), _row(1.5, 0, active=1), _row(1.8, 0, active=1),
                _row(1.9, 0)]
        rows[2]["sp_end_s"] = 1.8
        chords = [_chord(1, 100, 1500.0, 4, in_sp=True),
                  _chord(2, 150, 1750.0, 4, in_sp=True),
                  _chord(3, 200, 2000.0, 4)]
        reach = {c["index"]: sp_compare.PressReach(c["ms"], c["ms"]) for c in chords}
        report = sp_compare.Report()
        sp_compare._check_end_chord(report, rows, sp_compare._Activation(1, 1, 3),
                                    {"act_tick": 100, "deact_tick": 200},
                                    chords, reach)
        self.assertEqual(report.fails, 0)
        self.assertIn("Hydra's deactivation: chord 2 (tick 150, 1750.0 ms)",
                      report.lines[0])


# Freak's resolution. Its sp_max, 14,400, is pinned in the test below: the
# plan's F5 acceptance (docs/superpowers/plans/2026-11-08-player-audit-fixes.md)
# and SP-11.
FREAK_RESOLUTION = 480
# Hydra's own full bar at that resolution, 32 x 480 (SP-11), which the game's
# meter is not.
HYDRA_BAR_AT_FREAK_RESOLUTION = 15360


def _sp_max_lines(sp_max, resolution):
    report = sp_compare.Report()
    sp_compare._check_sp_max(report, [_row(1.0, 0, sp_max=sp_max)], MAX,
                             resolution, None)
    return report.lines


class SpMaxTest(unittest.TestCase):
    def test_sp_max_is_checked_against_30_times_resolution(self):
        self.assertEqual(sp_compare.game_full_meter(FREAK_RESOLUTION), 14400)
        self.assertEqual(_sp_max_lines(14400, FREAK_RESOLUTION),
                         ["PASS sp_max 14400; the game's full meter "
                          "14400 (resolution 480 x 30)"])
        self.assertEqual(_sp_max_lines(HYDRA_BAR_AT_FREAK_RESOLUTION, FREAK_RESOLUTION),
                         ["FAIL sp_max 15360; the game's full meter "
                          "14400 (resolution 480 x 30)"])

    def test_changing_sp_max_fails(self):
        # The meter's maximum changes at chord 3's ms.
        changes_ms = SCORE["chords"][3]["ms"]
        report = run(build_rows(sp_max=lambda t: FULL_METER if t < changes_ms else 7000),
                     sp_bar_ticks=14400)
        self.assertIn("FAIL sp_max 5760, 7000; the game's full meter 5760 "
                      "(resolution 192 x 30); sp_bar_ticks 14400", report.lines)

    def test_sp_max_check_refuses_without_a_resolution(self):
        with self.assertRaisesRegex(ValueError, "no chart resolution given"):
            run(build_rows(), resolution=None)

    def test_the_hydra_full_meter_line_follows_question_4(self):
        # D324 item 4 chose "Drop that half": the line shows sp_max and the
        # game's 30 x resolution only.
        report = run(build_rows())
        self.assertNotIn("Hydra full meter", report.text())
        self.assertIn("PASS sp_max 5760; the game's full meter 5760 (resolution "
                      "192 x 30)", report.lines)
        with open(sp_compare.__file__, encoding="utf-8") as f:
            self.assertNotIn("Hydra full meter", f.read())


class CodeTest(unittest.TestCase):
    def test_ms_calls_s_to_ms(self):
        with mock.patch.object(sp_compare.C, "s_to_ms", return_value=-1.0) as s_to_ms:
            self.assertEqual(sp_compare._ms({"clock_s": 1.5}), -1.0)
        s_to_ms.assert_called_once_with(1.5)

    def test_no_ready_copy_remains(self):
        # F6 reads the ready point from the game and Hydra (D323 item 1).
        with open(sp_compare.__file__, encoding="utf-8") as f:
            text = f.read()
        for name in ("at_ready_level", "READY_FRACTION", "SP_ACTIVATION_BARS",
                     "SP_CAP_BARS", "MEASURES_PER_SP_BAR", "QUARTERS_PER_MEASURE",
                     "ready_between", "ReadyWindow", "hydra_full_meter_ticks"):
            self.assertNotIn(name, text)


class TolerancesTest(unittest.TestCase):
    def test_clamped_rise_passes(self):
        report = run(build_rows(amount0=4000))
        self.assertIn("PASS phrase rise after chord 2 (tick 384, 2000.0 ms): +320 "
                      "(5440 -> 5760) at 2010.0 ms, clamped", report.lines)
        self.assertIn("Full rises: all 1440 ticks; sp_max / 4 = 1440", report.lines)
        self.assertEqual(report.fails, 0)

    def test_multiplier_one_chord_late_passes(self):
        report = run(build_rows(mult_lag=1))
        self.assertIn("PASS multiplier at each chord: 8 of 8 chords match (one chord "
                      "of lag allowed)", report.lines)
        self.assertEqual(report.fails, 0)


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
                    sp_compare.main([csv_path, json_path, "--resolution", "192",
                                     "--stop", "done"])

    def test_main_needs_the_resolution(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            with contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    self._main([csv_path, json_path, "--stop", "done"])

    def test_main_reads_the_files_and_exits_0(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            code, text = self._main([csv_path, json_path, "--resolution", "192",
                                     "--sp-bar-ticks", "14400", "--stop", "done"])
        self.assertEqual(code, 0)
        self.assertEqual(text, MATCHING_REPORT + "\n")

    def test_main_exits_1_on_a_fail(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows(rise_skip={2}))
            code, _ = self._main([csv_path, json_path, "--resolution", "192",
                                  "--stop", "done"])
        self.assertEqual(code, 1)

    def test_main_with_no_stop_fails(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows())
            code, text = self._main([csv_path, json_path, "--resolution", "192"])
        self.assertEqual(code, 1)
        self.assertIn("FAIL run end: the run's stop reason was not given\n", text)

    def test_width_defaults_to_64_and_32_reads_the_u32_columns(self):
        with tempfile.TemporaryDirectory() as folder:
            csv_path, json_path = self._write(folder, build_rows(u64_scale=2))
            _, text64 = self._main([csv_path, json_path, "--resolution", "192",
                                    "--stop", "done"])
            _, text32 = self._main([csv_path, json_path, "--resolution", "192",
                                    "--stop", "done", "--width", "32"])
        self.assertIn("FAIL sp_max 11520; the game's full meter 5760 "
                      "(resolution 192 x 30)\n", text64)
        self.assertIn("PASS sp_max 5760; the game's full meter 5760 "
                      "(resolution 192 x 30)\n", text32)

    def test_missing_column_is_an_error(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "watch.csv")
            with open(path, "w", newline="", encoding="utf-8") as f:
                f.write("wall_s,clock_s\n1.0,0.5\n")
            with self.assertRaises(ValueError):
                sp_compare.load_watch_rows(path)


def _row(clock_s, amount, *, active=0, sp_max=FULL_METER):
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
