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
from tools.ch_probe.experiments.score_compare import stop_check  # noqa: E402
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
            "sp_half_u64": math.nan,
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


# --- Two real runs of one corpus chart -----------------------------------------
#
# Both runs had the auto-player play path 0 of CHON - Pitch Dark (B), whose
# notes.mid is byte for byte the corpus chart
# testdata/input/test_pitch_dark_ratio/notes.mid. Pieces of each run's
# sp_watch-1.csv are copied, rows unchanged, under testdata/ch_runs.
#
# hydra-data/ch-mcp-runs/2026-11-13/pitch-dark-ratios-run1 kept to the path.
# Its sp_watch-1-act3.csv holds clock 188.4 s to 195.6 s, the run's third
# activation from before its flip to after its end; sp_watch-1-rise776.csv
# holds clock 179.2 s to 179.6 s, around the meter's rise after the song's
# last phrase end.
#
# hydra-data/ch-mcp-runs/2026-11-12/pitch-dark-run1 did not: its second
# Star Power ended about 3 s before Hydra's end, and its third activation
# came on another chord. Each *-edges.csv holds four rows of one activation,
# the row before the flag sets, the row it sets on, the last row with it set
# and the row it clears on (data rows 29270, 29271, 33961 and 33962 for the
# second; 37338, 37339, 38941 and 38942 for the third).
_RUNS_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs")
_ACT3_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1", "sp_watch-1-act3.csv")
_RISE_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1", "sp_watch-1-rise776.csv")
_EARLY_END_CSV = os.path.join(_RUNS_DIR, "pitch-dark-run1", "sp_watch-1-act2-edges.csv")
_OFF_PATH_CSV = os.path.join(_RUNS_DIR, "pitch-dark-run1", "sp_watch-1-act3-edges.csv")
_PITCH_DARK_CHART = os.path.join(_REPO_ROOT, "testdata", "input",
                                 "test_pitch_dark_ratio", "notes.mid")

# Hydra's own files for that chart, in testdata/replay. From the repo root,
#   py -I tools/ch_probe/prep_inputs.py
#       --chart testdata/input/test_pitch_dark_ratio/notes.mid --out testdata/replay
# writes the dump and path 0's score and schedule, the path both runs played.
# The runs' own schedules were made by older builds; the press times here
# come from the schedule this checkout's hydra_replay makes.
_REPLAY_DIR = os.path.join(_REPO_ROOT, "testdata", "replay")
_REPLAY_HOW = ("make it as the comment above _REPLAY_DIR in "
               "tools/ch_probe/tests/test_sp_compare.py says")


def _replay_file(name):
    """A file in testdata/replay; a missing one fails the test at once."""
    path = os.path.join(_REPLAY_DIR, name)
    if not os.path.isfile(path):
        raise AssertionError(f"{path} is missing; {_REPLAY_HOW}")
    return path


def _path_files():
    """(score, press_reach) of path 0's hydra_replay files for the chart."""
    score = sp_compare.load_score(_replay_file("test_pitch_dark_ratio-score.json"))
    reach = sp_compare.press_reach(_replay_file("test_pitch_dark_ratio-schedule.json"))
    return score, reach


def _midi_resolution(path):
    """The chart's resolution: the division field (ticks per quarter note) of
    its Standard MIDI File header, bytes 12-13 of the MThd chunk."""
    with open(path, "rb") as f:
        head = f.read(14)
    if head[:4] != b"MThd":
        raise AssertionError(f"{path} does not start with a MIDI header")
    return int.from_bytes(head[12:14], "big")


def _within(score, rows, lead_ms=0.0):
    """The score with only the chords and path activations inside the clock
    span of a watch piece; chords reach lead_ms further back. The piece is a
    few seconds of the run, so the comparison is held to Hydra's chords in
    those seconds; no chord or activation is changed."""
    lo, hi = sp_compare._ms(rows[0]), sp_compare._ms(rows[-1])
    return dict(score,
                chords=[c for c in score["chords"] if lo - lead_ms <= c["ms"] <= hi],
                activations=[a for a in score["activations"]
                             if lo <= a["act_ms"] and a["deact_ms"] <= hi])


def _starting(report, text):
    """The report's lines that start with `text`."""
    return [line for line in report.lines if line.startswith(text)]


class RealRunTest(unittest.TestCase):
    """sp_compare on the real pitch-dark-run1 watch pieces, against Hydra's
    score and schedule of the path the run played."""

    @classmethod
    def setUpClass(cls):
        cls.score, cls.reach = _path_files()
        cls.resolution = _midi_resolution(_PITCH_DARK_CHART)
        cls.act3 = sp_compare.load_watch_rows(_ACT3_CSV)
        cls.rise = sp_compare.load_watch_rows(_RISE_CSV)
        cls.act3_score = _within(cls.score, cls.act3)
        cls.rise_score = _within(cls.score, cls.rise)

    def compare(self, rows, score, *, reach=None, stop=play_chart.STOP_DONE, **kwargs):
        return sp_compare.compare(rows, score, resolution=self.resolution,
                                  reach=reach or self.reach, stop=stop, **kwargs)

    def _activation(self):
        """The path activation inside the act3 piece and its chord."""
        acts = self.act3_score["activations"]
        self.assertEqual(len(acts), 1, "the act3 piece should hold one path activation")
        act = acts[0]
        chord = next(c for c in self.act3_score["chords"] if c["tick"] == act["act_tick"])
        return act, chord

    def _hydra_deact_chord(self, act):
        """The last chord Hydra pays in Star Power from act_tick to deact_tick."""
        return [c for c in self.act3_score["chords"]
                if c["in_sp"] and act["act_tick"] <= c["tick"] <= act["deact_tick"]][-1]

    def _first_flag_row(self, rows):
        return next(i for i, r in enumerate(rows) if sp_compare.is_active(r))

    def test_the_runs_last_activation_passes_every_check(self):
        report = self.compare(self.act3, self.act3_score)
        self.assertEqual(report.fails, 0, report.text())
        act, chord = self._activation()
        flip = self.act3[self._first_flag_row(self.act3)]
        self.assertIn(f"Activation 1 at clock {sp_compare.format_ms(sp_compare._ms(flip))}:",
                      report.lines)
        self.assertIn("PASS   activation 1 on the path's chord: "
                      + sp_compare._chord_label(chord), report.lines)
        end = _starting(report, "PASS   activation 1 end chord:")
        self.assertEqual(len(end), 1, report.text())
        self.assertIn("Hydra's deactivation: "
                      + sp_compare._chord_label(self._hydra_deact_chord(act)), end[0])
        sp_max = _starting(report, "PASS sp_max ")
        self.assertEqual(len(sp_max), 1, report.text())
        self.assertIn(str(sp_compare.game_full_meter(self.resolution)), sp_max[0])
        self.assertEqual(len(_starting(report, "PASS multiplier at each chord:")), 1)

    def test_the_last_phrase_rise_is_found_on_the_row_the_game_shows_it(self):
        ends = [c for c in self.rise_score["chords"] if c["is_sp_phrase_end"]]
        self.assertEqual(len(ends), 1, "the rise piece should hold one phrase end")
        rises = sp_compare.phrase_rises(self.rise, AMOUNT)
        self.assertTrue(rises, "the rise piece should hold the game's rise")
        report = self.compare(self.rise, self.rise_score)
        line = _starting(report, f"PASS phrase rise after {sp_compare._chord_label(ends[0])}: ")
        self.assertEqual(len(line), 1, report.text())
        self.assertIn(f"at {sp_compare.format_ms(rises[0].ms)}", line[0])

    def _rise_cut_before_the_rise(self):
        """The rise piece up to the row before the game's rise: a watch that
        stopped just before the meter rose."""
        first_rise = next(i for i in range(1, len(self.rise))
                          if self.rise[i][AMOUNT] > self.rise[i - 1][AMOUNT])
        return self.rise[:first_rise]

    def test_summary_is_the_first_line(self):
        passing = self.compare(self.act3, self.act3_score)
        failing = self.compare(self._rise_cut_before_the_rise(), self.rise_score)
        self.assertGreater(failing.fails, 0)
        for report in (passing, failing):
            passes = len(_starting(report, "PASS "))
            fails = len(_starting(report, "FAIL "))
            self.assertEqual((report.passes, report.fails), (passes, fails))
            self.assertEqual(report.lines[0], f"Summary: {passes} PASS, {fails} FAIL")
            self.assertEqual(len(_starting(report, "Summary")), 1)

    def test_a_run_that_stopped_before_the_song_end_fails(self):
        done = self.compare(self.act3, self.act3_score)
        stopped = self.compare(self.act3, self.act3_score, stop=play_chart.STOP_NO_ENGINE)
        ok, line = stop_check(play_chart.STOP_NO_ENGINE)
        self.assertFalse(ok)
        self.assertIn(line, stopped.lines)
        self.assertIn(play_chart.STOP_NO_ENGINE, line)
        self.assertEqual(stopped.fails, done.fails + 1)

    def test_a_run_with_no_stop_reason_fails(self):
        done = self.compare(self.act3, self.act3_score)
        report = self.compare(self.act3, self.act3_score, stop=None)
        ok, line = stop_check(None)
        self.assertFalse(ok)
        self.assertIn(line, report.lines)
        self.assertEqual(report.fails, done.fails + 1)

    def test_missing_rise_fails(self):
        rows = self._rise_cut_before_the_rise()
        end = next(c for c in self.rise_score["chords"] if c["is_sp_phrase_end"])
        report = self.compare(rows, self.rise_score)
        line = _starting(report, f"FAIL phrase rise after {sp_compare._chord_label(end)}: "
                                 f"missing")
        self.assertEqual(len(line), 1, report.text())
        # The search starts at the chord's first press (press_reach).
        self.assertIn(f"from {sp_compare.format_ms(self.reach[end['index']].earliest_ms)}",
                      line[0])

    def test_flag_never_sets_fails(self):
        # The act3 piece up to the row before the flag sets: a watch that
        # stopped before the game activated.
        rows = self.act3[:self._first_flag_row(self.act3)]
        act, chord = self._activation()
        report = self.compare(rows, self.act3_score)
        self.assertIn(f"FAIL path activation 1 (act_tick {act['act_tick']}): the game "
                      f"made no activation", report.lines)
        parted = _starting(report, "Multiplier not compared from chord ")
        self.assertEqual(len(parted), 1, report.text())
        self.assertTrue(parted[0].endswith(
            f"the game's activations part from the path at {sp_compare.format_ms(chord['ms'])}"),
            parted[0])

    def test_end_never_clears_fails(self):
        # The act3 piece up to the last row with the flag set: a watch that
        # stopped inside the activation.
        flip = self._first_flag_row(self.act3)
        drop = next(i for i in range(flip, len(self.act3))
                    if not sp_compare.is_active(self.act3[i]))
        rows = self.act3[:drop]
        act, _ = self._activation()
        report = self.compare(rows, self.act3_score)
        self.assertEqual(len(_starting(
            report, "FAIL   activation 1 flag clears at the end: still set at the last "
                    f"row ({sp_compare.format_ms(sp_compare._ms(rows[-1]))})")), 1,
            report.text())
        self.assertIn("FAIL   activation 1 end chord: the flag never cleared; Hydra's "
                      "deactivation: " + sp_compare._chord_label(self._hydra_deact_chord(act)),
                      report.lines)

    def test_zero_of_zero_fails(self):
        # The piece's first row comes before the first chord's first press, so
        # no chord has a row to be read on.
        chords = self.act3_score["chords"]
        self.assertLess(sp_compare._ms(self.act3[0]),
                        self.reach[chords[0]["index"]].earliest_ms)
        report = sp_compare.Report()
        sp_compare._check_multipliers(report, self.act3[:1], chords, None, self.reach)
        self.assertEqual(report.fails, 1)
        self.assertTrue(report.lines[0].startswith(
            "FAIL multiplier at each chord: 0 of 0 chords match"), report.lines[0])
        self.assertIn(f"Multiplier not compared on {len(chords)} chords", report.lines[1])

    def test_an_activation_on_at_row_0_is_reported(self):
        # The act3 piece from the row the flag sets on: a watch that started
        # after the activation began.
        rows = self.act3[self._first_flag_row(self.act3):]
        found = sp_compare.find_activations(rows)
        self.assertEqual([(a.number, a.flip_row) for a in found], [(1, 0)])
        drop = found[0].drop_row
        self.assertTrue(sp_compare.is_active(rows[drop - 1]))
        self.assertFalse(sp_compare.is_active(rows[drop]))
        report = self.compare(rows, self.act3_score)
        self.assertIn("FAIL   activation 1 was already on at the watch's first row "
                      f"({sp_compare.format_ms(sp_compare._ms(rows[0]))}); its start was "
                      "not seen", report.lines)

    def test_star_power_that_ends_before_hydras_end_fails(self):
        # pitch-dark-run1's second activation, on path 0's second activation
        # chord: the flag clears about 3 s before Hydra's deact_ms.
        rows = sp_compare.load_watch_rows(_EARLY_END_CSV)
        act = sp_compare.find_activations(rows)
        self.assertEqual([(a.flip_row, a.drop_row) for a in act], [(1, 3)])
        want = self.score["activations"][1]
        report = sp_compare.Report()
        sp_compare._check_activation(report, rows, act[0], want["deact_ms"])
        sp_compare._check_end_chord(report, rows, act[0], want, self.score["chords"],
                                    self.reach)
        drop_ms = sp_compare._ms(rows[3])
        self.assertLess(drop_ms, want["deact_ms"] - sp_compare.EVENT_WINDOW_MS)
        self.assertIn(f"FAIL   activation 1 flag clears at Hydra's end: Hydra "
                      f"{sp_compare.format_ms(want['deact_ms'])}, flag 0 at "
                      f"{sp_compare.format_ms(drop_ms)}", report.lines)
        hydra = [c for c in self.score["chords"] if c["in_sp"]
                 and want["act_tick"] <= c["tick"] <= want["deact_tick"]][-1]
        end = _starting(report, "FAIL   activation 1 end chord: game end ")
        self.assertEqual(len(end), 1, report.text())
        self.assertIn("Hydra's deactivation: " + sp_compare._chord_label(hydra), end[0])
        self.assertNotIn("the game's last chord inside: " + sp_compare._chord_label(hydra),
                         end[0])

    def test_an_activation_on_a_chord_the_path_does_not_activate_on_is_off_the_path(self):
        # pitch-dark-run1's third activation came between path 0's second and
        # third: inside these rows' span the path makes no activation.
        # Its first row is the one before the flip, so the chords reach back
        # as far as a flip may land (LANDING_WINDOW_MS).
        rows = sp_compare.load_watch_rows(_OFF_PATH_CSV)
        score = _within(self.score, rows, sp_compare.LANDING_WINDOW_MS)
        self.assertEqual(score["activations"], [])
        report = self.compare(rows, score)
        off = _starting(report, "FAIL   activation 1 not on the path's chord: game tick ")
        self.assertEqual(len(off), 1, report.text())
        self.assertTrue(off[0].endswith(", path act_tick none"), off[0])
        game_tick = int(off[0].split("game tick ")[1].split(",")[0])
        self.assertIn(game_tick, [c["tick"] for c in self.score["chords"]])
        self.assertNotIn(game_tick, [a["act_tick"] for a in self.score["activations"]])
        self.assertFalse(_starting(report, "PASS   activation 1 end chord"))
        self.assertFalse(_starting(report, "FAIL   activation 1 end chord"))


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


def _mult_rows(points):
    """Hand-written watcher rows from (clock_s, score, sp_mult_u32) triples;
    only the fields the multiplier check reads vary."""
    rows = []
    for clock_s, score, mult in points:
        row = _row(clock_s, 0)
        row.update(score=score, sp_mult_byte=mult, sp_mult_u32=mult)
        rows.append(row)
    return rows


def _last_chord_check(last_in_sp, points):
    """_check_multipliers over three chords whose last one (2500 ms) is in
    Star Power or not, each pressed at its own ms; the report's lines."""
    chords = [_chord(0, 0, 1000.0, 4), _chord(1, 480, 2000.0, 4, in_sp=True),
              _chord(2, 720, 2500.0, 4, in_sp=last_in_sp)]
    reach = {c["index"]: sp_compare.PressReach(c["ms"], c["ms"]) for c in chords}
    report = sp_compare.Report()
    sp_compare._check_multipliers(report, _mult_rows(points), chords, None, reach)
    return report.lines


class LastChordMultiplierTest(unittest.TestCase):
    """The song's last chord has no next chord to read before, so the check
    reads it on the row its points land on, not on the watcher's final row."""

    def test_last_chord_paid_in_star_power_before_it_ends_passes(self):
        # Star Power doubles 4 to 8 at 1.8 s; the last chord's points land at
        # 2.505 s at 8x; Star Power ends at 3.0 s and the watch runs on to
        # 3.5 s, so the final row shows 4.
        lines = _last_chord_check(True, [
            (0.9, 0, 4), (1.005, 100, 4), (1.8, 100, 8), (2.005, 300, 8),
            (2.505, 500, 8), (3.0, 500, 4), (3.5, 500, 4)])
        self.assertEqual(lines, ["PASS multiplier at each chord: 3 of 3 chords "
                                 "match (one chord of lag allowed)"])

    def test_last_chord_out_of_star_power_passes(self):
        # Star Power ends at 2.2 s, before the last chord; its points land at
        # 2.505 s at 4x, and the rows after the song show 1.
        lines = _last_chord_check(False, [
            (0.9, 0, 4), (1.005, 100, 4), (1.8, 100, 8), (2.005, 300, 8),
            (2.2, 300, 4), (2.505, 400, 4), (3.0, 400, 1), (3.5, 400, 1)])
        self.assertEqual(lines, ["PASS multiplier at each chord: 3 of 3 chords "
                                 "match (one chord of lag allowed)"])

    def test_last_chord_with_no_score_rise_is_read_on_the_final_row(self):
        # The score never rises after 2500 ms, so the check falls back to the
        # row before the final one, which shows 4 after Star Power ended.
        lines = _last_chord_check(True, [
            (0.9, 0, 4), (1.005, 100, 4), (1.8, 100, 8), (2.005, 300, 8),
            (2.505, 300, 8), (3.0, 300, 4), (3.5, 300, 4)])
        self.assertEqual(lines, ["FAIL multiplier at each chord: first mismatch at "
                                 "chord 2 (tick 720, 2500.0 ms): game 4, expected 8 "
                                 "(multiplier 4, in Star Power); 2 of 3 chords match"])


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
        "sp_max_u32": sp_max, "sp_max_u64": sp_max, "sp_half_u64": math.nan,
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
_MOUTHS = (_fixture("mouths_refine_score.json"),
           _fixture("mouths_refine_schedule.json"))
_BOMBER = (_fixture("bomber_refine_score.json"),
           _fixture("bomber_refine_schedule.json"))


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
