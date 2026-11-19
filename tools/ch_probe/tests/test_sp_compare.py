"""Tests for experiments/sp_compare.py.

Most tests read real recordings under testdata/ch_runs (each excerpt names its
source run folder under hydra-data/ch-mcp-runs). The rest use a small
hand-written run for cases no recording shows.

The hand-written score JSON is eight chords with three phrase ends and one activation
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

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import play_chart, sp_compare  # noqa: E402
from tools.ch_probe.experiments.overlay_logs import (  # noqa: E402
    WATCH_CLOCK_COLUMNS, WATCH_COLUMNS, WATCH_OLDER_NAMES)
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


# The first three rows of hydra-data/ch-mcp-runs/2026-11-07/ls-r1-run1's
# sp_watch-1.csv, copied unchanged: a run of another chart, whose meter is
# not Pitch Dark's.
_OTHER_CHART_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-head.csv")


def _sp_max_lines(rows, resolution):
    report = sp_compare.Report()
    sp_compare._check_sp_max(report, rows, MAX, resolution, None)
    return report.lines


class SpMaxTest(unittest.TestCase):
    def test_sp_max_is_checked_against_the_charts_resolution(self):
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        want = sp_compare.game_full_meter(resolution)
        pitch = sp_compare.load_watch_rows(_RISE_CSV)
        other = sp_compare.load_watch_rows(_OTHER_CHART_CSV)
        self.assertNotEqual(other[0][MAX], want)
        for rows, verdict in ((pitch, "PASS"), (other, "FAIL")):
            lines = _sp_max_lines(rows, resolution)
            self.assertEqual(len(lines), 1, lines)
            self.assertTrue(lines[0].startswith(f"{verdict} sp_max {rows[0][MAX]}; "), lines)
            self.assertIn(f"the game's full meter {want} (resolution {resolution} x "
                          f"{sp_compare.GAME_FULL_METER_PER_RESOLUTION})", lines[0])

    def _real_act3(self):
        """The real act3 piece, Hydra's score of its seconds and the path's
        press reach, with the chart's resolution."""
        score, reach = _path_files()
        rows = sp_compare.load_watch_rows(_ACT3_CSV)
        return rows, _within(score, rows), reach, _midi_resolution(_PITCH_DARK_CHART)

    def test_sp_max_check_refuses_without_a_resolution(self):
        rows, score, reach, resolution = self._real_act3()
        with self.assertRaises(ValueError) as caught:
            sp_compare.compare(rows, score, reach=reach, stop=play_chart.STOP_DONE)
        self.assertIn("resolution", str(caught.exception))
        # The same real run with the chart's resolution is compared.
        report = sp_compare.compare(rows, score, resolution=resolution, reach=reach,
                                    stop=play_chart.STOP_DONE)
        self.assertEqual(len(_starting(report, "PASS sp_max ")), 1, report.text())

    def test_the_hydra_full_meter_line_follows_question_4(self):
        # D324 item 4 chose "Drop that half": the line shows sp_max and the
        # game's full meter for the chart's resolution, and nothing more.
        rows, score, reach, resolution = self._real_act3()
        report = sp_compare.compare(rows, score, resolution=resolution, reach=reach,
                                    stop=play_chart.STOP_DONE)
        self.assertNotIn("Hydra full meter", report.text())
        want = sp_compare.game_full_meter(resolution)
        self.assertEqual(_starting(report, "PASS sp_max "),
                         [f"PASS sp_max {rows[0][MAX]}; the game's full meter {want} "
                          f"(resolution {resolution} x "
                          f"{sp_compare.GAME_FULL_METER_PER_RESOLUTION})"],
                         report.text())


# Two more pieces of pitch-dark-ratios-run1's sp_watch-1.csv, rows unchanged.
# sp_watch-1-clamp166.csv holds clock 88.70 s to 88.96 s (data rows 19736 to
# 19793), where a phrase end fills the meter during Star Power.
# sp_watch-1-end269.csv holds clock 103.30 s to 104.08 s (data rows 23003 to
# 23178), around the end of the run's first activation.
_CLAMP_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1", "sp_watch-1-clamp166.csv")
_END_CSV = os.path.join(_RUNS_DIR, "pitch-dark-ratios-run1", "sp_watch-1-end269.csv")


class TolerancesTest(unittest.TestCase):
    """The checks' allowances, on real rows of the run that kept to path 0."""

    @classmethod
    def setUpClass(cls):
        cls.score, cls.reach = _path_files()

    def test_a_rise_cut_short_by_the_full_meter_passes_as_clamped(self):
        rows = sp_compare.load_watch_rows(_CLAMP_CSV)
        chords = _within(self.score, rows)["chords"]
        ends = [c for c in chords if c["is_sp_phrase_end"]]
        self.assertEqual(len(ends), 1, "the clamp piece should hold one phrase end")
        rises = sp_compare.phrase_rises(rows, AMOUNT)
        self.assertEqual(len(rises), 1, rises)
        # The game's meter is full on the row the rise shows on, and the rise
        # is smaller than the quarter of the meter a phrase gives.
        risen = next(r for r in rows if sp_compare._ms(r) == rises[0].ms)
        self.assertEqual(risen[AMOUNT], risen[MAX])
        self.assertLess(rises[0].size, risen[MAX] / sp_compare.PHRASES_PER_FULL_METER)
        report = sp_compare.Report()
        sp_compare._check_phrase_rises(report, rows, chords, AMOUNT, MAX, self.reach)
        self.assertEqual(report.fails, 0, report.text())
        line = _starting(report, f"PASS phrase rise after {sp_compare._chord_label(ends[0])}: ")
        self.assertEqual(len(line), 1, report.text())
        self.assertIn(f"+{rises[0].size} ({rises[0].before} -> {rises[0].after})", line[0])
        self.assertTrue(line[0].endswith(", clamped, during Star Power"), line[0])

    def test_a_multiplier_read_that_shows_the_next_chords_value_passes(self):
        # The flag clears between Hydra's last chord in Star Power and the
        # next chord, so the row read for that last chord (the last one
        # before the next chord's ms) already shows the next chord's value.
        rows = sp_compare.load_watch_rows(_END_CSV)
        chords = _within(self.score, rows)["chords"]
        last_sp = max(i for i, c in enumerate(chords) if c["in_sp"])
        self.assertLess(last_sp + 1, len(chords), "the piece should reach the next chord")
        chord, after = chords[last_sp], chords[last_sp + 1]
        read = [r for r in rows if sp_compare._ms(r) < after["ms"]][-1]
        self.assertNotIn(read["sp_mult_u32"], sp_compare._expected_multipliers(chord))
        self.assertIn(read["sp_mult_u32"], sp_compare._expected_multipliers(after))
        report = sp_compare.Report()
        sp_compare._check_multipliers(report, rows, chords, None, self.reach)
        self.assertEqual(report.fails, 0, report.text())
        self.assertEqual(report.lines[0], f"PASS multiplier at each chord: {len(chords)} of "
                                          f"{len(chords)} chords match (one chord of lag "
                                          f"allowed)")


class LastChordMultiplierTest(unittest.TestCase):
    """The song's last chord has no next chord to read before, so the check
    reads it on the row its points land on, not on the watcher's final row."""

    def test_last_chord_paid_in_star_power_before_it_ends_passes(self):
        # The act3 piece runs past the run's last Star Power, which ends after
        # the song's last chord, so its final row shows the halved multiplier.
        score, reach = _path_files()
        rows = sp_compare.load_watch_rows(_ACT3_CSV)
        chords = _within(score, rows)["chords"]
        last = chords[-1]
        self.assertEqual(last["index"], score["chords"][-1]["index"])
        self.assertTrue(last["in_sp"])
        for row in (rows[-2], rows[-1]):
            self.assertNotIn(row["sp_mult_u32"], sp_compare._expected_multipliers(last))
        report = sp_compare.Report()
        sp_compare._check_multipliers(report, rows, chords, None, reach)
        self.assertEqual(report.fails, 0, report.text())
        self.assertTrue(report.lines[0].startswith("PASS multiplier at each chord: "),
                        report.lines[0])


class FilesTest(unittest.TestCase):
    # The first three rows of hydra-data/ch-mcp-runs/2026-11-06/ls-d2b-run1's
    # sp_watch-1.csv, copied unchanged. The watcher that wrote it logged no
    # clock columns and called round_start_s by an older name.
    _OLD_LOG = os.path.join(_RUNS_DIR, "ls-d2b-run1", "sp_watch-1-head.csv")

    @staticmethod
    def _header_and_cells(path):
        with open(path, newline="", encoding="utf-8") as f:
            reader = csv.reader(f)
            header = next(reader)
            return header, list(reader)

    def test_a_log_without_the_clock_columns_loads_with_them_nan(self):
        header, cells = self._header_and_cells(self._OLD_LOG)
        self.assertFalse(set(WATCH_CLOCK_COLUMNS) & set(header))
        rows = sp_compare.load_watch_rows(self._OLD_LOG)
        self.assertEqual(len(rows), len(cells))
        for row in rows:
            self.assertEqual(set(row), set(WATCH_COLUMNS))
            self.assertTrue(all(math.isnan(row[c]) for c in WATCH_CLOCK_COLUMNS), row)
            self.assertIsInstance(row["score"], int)
            self.assertIsInstance(row["clock_s"], float)
        # The loaded rows feed the checks: their clock reads as ms, in order.
        clocks = [sp_compare._ms(r) for r in rows]
        self.assertEqual(clocks, sorted(clocks))
        self.assertFalse(any(math.isnan(t) for t in clocks))

    def test_a_saved_log_with_the_older_hit_time_s_header_loads(self):
        header, cells = self._header_and_cells(self._OLD_LOG)
        renamed = {old: new for old, new in WATCH_OLDER_NAMES.items() if old in header}
        self.assertTrue(renamed, "the log should use an older column name")
        rows = sp_compare.load_watch_rows(self._OLD_LOG)
        for old, new in renamed.items():
            self.assertNotIn(new, header)
            for row, line in zip(rows, cells):
                self.assertNotIn(old, row)
                self.assertEqual(row[new], float(line[header.index(old)]))

    def test_a_log_lacking_another_column_is_still_refused(self):
        # A machine recorder's samples.csv from ls-p3-run1 (copied from
        # hydra-data/ch-mcp-runs/2026-11-12/ls-p3-run1/machine-1): it has the
        # watcher's wall_s and clock_s but not its Star Power columns.
        path = os.path.join(_RUNS_DIR, "ls-p3-run1", "machine-1", "samples.csv")
        header, _ = self._header_and_cells(path)
        self.assertIn("clock_s", header)
        self.assertNotIn(AMOUNT, header)
        with self.assertRaises(ValueError) as caught:
            sp_compare.load_watch_rows(path)
        message = str(caught.exception)
        self.assertIn(path, message)
        self.assertIn(repr(AMOUNT), message)
        self.assertNotIn(repr("clock_s"), message)

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

    @staticmethod
    def _real_main(csv_path, args, lead_ms=0.0):
        """main on a real Pitch Dark watch piece, with path 0's hydra_replay
        score held to the piece's span (_within) and its schedule; its exit
        code, its printed text, and compare's own report on the same inputs
        (stop and width read from `args` as main reads them)."""
        score, reach = _path_files()
        rows = sp_compare.load_watch_rows(csv_path)
        piece = _within(score, rows, lead_ms)
        resolution = _midi_resolution(_PITCH_DARK_CHART)
        out = io.StringIO()
        with tempfile.TemporaryDirectory() as folder:
            json_path = os.path.join(folder, "score.json")
            with open(json_path, "w", encoding="utf-8") as f:
                json.dump(piece, f)
            with contextlib.redirect_stdout(out):
                code = sp_compare.main(
                    [csv_path, json_path, "--resolution", str(resolution),
                     "--schedule", _replay_file("test_pitch_dark_ratio-schedule.json")]
                    + args)
        return code, out.getvalue(), rows, piece, resolution, reach

    @staticmethod
    def _sp_bar_ticks():
        """sp_bar_ticks from the game's scorestats.json after pitch-dark-run1
        (copied from hydra-data/ch-mcp-runs/2026-11-12/pitch-dark-run1), a run
        of the same chart as the act3 piece."""
        path = os.path.join(_RUNS_DIR, "pitch-dark-run1",
                            "scorestats-CHON_-_Pitch_Dark_B-20261112-230618.json")
        with open(path, encoding="utf-8") as f:
            return json.load(f)["players"][0]["sp_bar_ticks"]

    def test_main_reads_the_files_and_exits_0(self):
        ticks = self._sp_bar_ticks()
        code, text, rows, piece, resolution, reach = self._real_main(
            _ACT3_CSV, ["--sp-bar-ticks", str(ticks), "--stop", play_chart.STOP_DONE])
        report = sp_compare.compare(rows, piece, resolution=resolution, sp_bar_ticks=ticks,
                                    reach=reach, stop=play_chart.STOP_DONE)
        self.assertEqual(report.fails, 0, report.text())
        self.assertEqual(code, 0, text)
        self.assertEqual(text, report.text() + "\n")
        sp_max = _starting(report, "PASS sp_max ")
        self.assertEqual(len(sp_max), 1, report.text())
        self.assertTrue(sp_max[0].endswith(f"; sp_bar_ticks {ticks}"), sp_max[0])

    def test_main_exits_1_on_a_fail(self):
        # pitch-dark-run1's third activation is off the path (see
        # RealRunTest); the chords reach back as far as a flip may land.
        code, text, *_ = self._real_main(_OFF_PATH_CSV, ["--stop", play_chart.STOP_DONE],
                                         sp_compare.LANDING_WINDOW_MS)
        self.assertEqual(code, 1, text)
        self.assertIn("\nFAIL   activation 1 not on the path's chord: game tick ", text)

    def test_main_with_no_stop_fails(self):
        code, text, *_ = self._real_main(_ACT3_CSV, [])
        self.assertEqual(code, 1, text)
        ok, line = stop_check(None)
        self.assertFalse(ok)
        self.assertIn("\n" + line + "\n", text)
        # The act3 piece passes every other check, so the stop is the one fail.
        self.assertEqual(text.count("\nFAIL "), 1, text)

    def test_width_defaults_to_its_owner_and_32_reads_the_same_real_meter(self):
        # Every recorded watch log under hydra-data/ch-mcp-runs holds equal
        # u32 and u64 amount and maximum columns, so both widths give one
        # verdict on a real log; only the header names the width.
        code64, text64, *_ = self._real_main(_ACT3_CSV, ["--stop", play_chart.STOP_DONE])
        code32, text32, *_ = self._real_main(_ACT3_CSV, ["--stop", play_chart.STOP_DONE,
                                                         "--width", "32"])
        self.assertEqual((code64, code32), (0, 0), text64 + text32)
        lines64, lines32 = text64.splitlines(), text32.splitlines()
        self.assertTrue(lines64[1].endswith(f", width {sp_compare.DEFAULT_WIDTH}"), lines64[1])
        self.assertTrue(lines32[1].endswith(", width 32"), lines32[1])
        self.assertEqual(lines64[2:], lines32[2:])


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


# The columns sp_compare reads by default, from their owner.
AMOUNT, MAX = sp_compare.column_names()


class RowChecksTest(unittest.TestCase):
    def test_old_name_is_the_public_finder(self):
        self.assertIs(sp_compare._find_activations, sp_compare.find_activations)

    def test_column_names_follow_the_width(self):
        # Each width main accepts names two columns the watcher logs
        # (WATCH_COLUMNS, the logs' owner), and the two widths name different
        # columns.
        widths = (32, 64)
        names = {w: sp_compare.column_names(w) for w in widths}
        for width, pair in names.items():
            self.assertEqual(len(set(pair)), 2, pair)
            for name in pair:
                self.assertIn(name, WATCH_COLUMNS, (width, name))
        self.assertFalse(set(names[32]) & set(names[64]), names)
        self.assertEqual(sp_compare.column_names(),
                         sp_compare.column_names(sp_compare.DEFAULT_WIDTH))

    def test_is_active_reads_the_flag_byte_only(self):
        # The game's real rows set bits above the flag byte in sp_active_u32:
        # the rise piece (no Star Power) logs 16777216 with the byte 0, and
        # the clamp piece (in Star Power) logs 16777217 with the byte 1.
        off = [r for r in sp_compare.load_watch_rows(_RISE_CSV) if r["sp_active_u32"] != 0]
        on = [r for r in sp_compare.load_watch_rows(_CLAMP_CSV)
              if r["sp_active_u32"] not in (0, 1)]
        self.assertTrue(off, "the rise piece should hold a row with the high bits set")
        self.assertTrue(on, "the clamp piece should hold a row with the high bits set")
        for row in off:
            self.assertEqual(row["sp_active_byte"], 0, row)
            self.assertFalse(sp_compare.is_active(row), row)
        for row in on:
            self.assertEqual(row["sp_active_byte"], 1, row)
            self.assertTrue(sp_compare.is_active(row), row)

    def test_a_flip_goes_to_its_own_chord_not_the_next_ones_press(self):
        # In Hydra's Bomber schedule, chord 147's first press comes 22.9 ms
        # after chord 146's (both read from the file) and before 147's ms.
        # A flip between the two presses belongs to 146; 147's early
        # press must not take it. A flip at 147's first press is 147's.
        reach = sp_compare.press_reach(_BOMBER[1])
        chords = _fixture_chords(_BOMBER[0])
        chord_ms = [c["ms"] for c in chords]
        k = next(i for i, c in enumerate(chords) if c["index"] == 146)
        first, second = chords[k], chords[k + 1]
        self.assertEqual(second["index"], 147)
        lo, hi = reach[146].earliest_ms, reach[147].earliest_ms
        self.assertLess(lo, hi)
        self.assertLess(hi, second["ms"])
        between = (lo + hi) / 2
        self.assertIs(sp_compare._landed_chord(chords, chord_ms, between, reach), first)
        self.assertIs(sp_compare._landed_chord(chords, chord_ms, hi, reach), second)

    def test_phrase_rises_gives_each_step_up(self):
        # The rise piece's meter steps from 3600 to 7200 on its data row
        # 34 (0-based) and nowhere else; the act3 piece's meter only drains in
        # Star Power and holds after it, so it has no rise.
        rows = sp_compare.load_watch_rows(_RISE_CSV)
        rises = sp_compare.phrase_rises(rows, AMOUNT)
        self.assertEqual(rises, [sp_compare.Rise(sp_compare._ms(rows[34]), 3600, 7200)])
        self.assertEqual(rows[33][AMOUNT], 3600)
        self.assertEqual(rows[34][AMOUNT], 7200)
        self.assertEqual(sp_compare.phrase_rises(sp_compare.load_watch_rows(_ACT3_CSV),
                                                 AMOUNT), [])

    def test_phrase_rises_on_three_sizes(self):
        # Three real rises, each read off its piece: a full quarter of the
        # meter (rise piece, 3600 -> 7200), one cut short by the full meter
        # (clamp piece, 10953 -> 14400) and one from 1 on the row Star Power
        # ends (act2 edges piece, 1 -> 3600).
        sizes = [[r.size for r in sp_compare.phrase_rises(sp_compare.load_watch_rows(p),
                                                           AMOUNT)]
                 for p in (_RISE_CSV, _CLAMP_CSV, _EARLY_END_CSV)]
        self.assertEqual(sizes, [[3600], [3447], [3599]])

    def test_flips_between_keeps_only_the_flip_inside(self):
        # The act2 edges piece: row 0 before the flag sets, row 1 the flip,
        # row 2 still set, row 3 the drop.
        rows = sp_compare.load_watch_rows(_EARLY_END_CSV)
        flip, drop = sp_compare._ms(rows[1]), sp_compare._ms(rows[3])
        flips = sp_compare.flips_between(rows, sp_compare._ms(rows[0]), drop)
        self.assertEqual([(a.number, a.flip_row, a.drop_row) for a in flips], [(1, 1, 3)])
        # A window after the flip holds set rows but no flip.
        self.assertEqual(sp_compare.flips_between(rows, math.nextafter(flip, math.inf),
                                                  drop), [])
        self.assertEqual(sp_compare.flips_between(rows, sp_compare._ms(rows[0]),
                                                  math.nextafter(flip, -math.inf)), [])

    def test_flips_between_includes_both_ends(self):
        rows = sp_compare.load_watch_rows(_EARLY_END_CSV)
        flip = sp_compare._ms(rows[1])
        self.assertEqual(len(sp_compare.flips_between(rows, flip, flip)), 1)


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

    # Rows of hydra-data/ch-mcp-runs/2026-11-07/ls-r1-run1's sp_watch-1.csv,
    # copied unchanged (data rows 26287 to 26301, clock 118.39 s to 118.46 s):
    # a run of the same Mouths chart. Its press-1.csv shows chord 740 pressed
    # twice, at clock 118412.5 ms and 118443.1 ms; the flag clears at
    # 118431.0 ms and the meter rises 0 -> 1440 at 118449.2 ms.
    _MOUTHS_RISE_CSV = os.path.join(_RUNS_DIR, "ls-r1-run1", "sp_watch-1-rise740.csv")

    def test_mouths_740_is_credited_at_its_second_press(self):
        reach = sp_compare.press_reach(_MOUTHS[1])
        rows = sp_compare.load_watch_rows(self._MOUTHS_RISE_CSV)
        rises = sp_compare.phrase_rises(rows, AMOUNT)
        self.assertEqual(len(rises), 1, rises)
        rise = rises[0]
        # The rise shows after the chord's ms and after its last press.
        chord = next(c for c in _fixture_chords(_MOUTHS[0]) if c["index"] == 740)
        self.assertTrue(chord["is_sp_phrase_end"])
        self.assertGreater(rise.ms, chord["ms"])
        self.assertGreater(rise.ms, reach[740].latest_ms)
        self.assertLess(rise.ms, max(chord["ms"], reach[740].latest_ms)
                        + sp_compare.EVENT_WINDOW_MS)
        risen = next(r for r in rows if sp_compare._ms(r) == rise.ms)
        self.assertLess(rise.after, risen[MAX])
        self.assertIn(f"PASS phrase rise after {sp_compare._chord_label(chord)}: "
                      f"+{rise.size} ({rise.before} -> {rise.after}) at "
                      f"{sp_compare.format_ms(rise.ms)}, full",
                      _rise_lines(_MOUTHS[0], reach, rows))

    def _assert_reach_spans_presses(self, schedule_path):
        """Every press of the schedule's plan (play_chart.press_plan) lies in
        its chord's reach, and each reach's two ends are presses of that
        chord. The chords' press times, by index."""
        reach = sp_compare.press_reach(schedule_path)
        presses = {}
        for press in play_chart.press_plan(play_chart.load_schedule(schedule_path).chords):
            presses.setdefault(press.index, []).append(press.at_ms)
        self.assertEqual(set(reach), set(presses))
        for index, times in presses.items():
            for at in times:
                self.assertLessEqual(reach[index].earliest_ms, at, index)
                self.assertLessEqual(at, reach[index].latest_ms, index)
            self.assertIn(reach[index].earliest_ms, times, index)
            self.assertIn(reach[index].latest_ms, times, index)
        return reach, presses

    def _chord(self, score_path, index):
        return next(c for c in _fixture_chords(score_path) if c["index"] == index)

    def test_bomber_147_reaches_its_solved_early_press(self):
        # 147 is an activation and chord 180's squeeze anchor, so the
        # schedule presses it early, before the chord's own ms.
        reach, presses = self._assert_reach_spans_presses(_BOMBER[1])
        chords = _fixture_chords(_BOMBER[0])
        chord_ms = [c["ms"] for c in chords]
        at = chord_ms.index(self._chord(_BOMBER[0], 147)["ms"])
        prev = chords[at - 1]
        self.assertLess(reach[147].latest_ms, chords[at]["ms"])
        # A flip after 147's last press but before its ms lands on 147. Chord
        # 146 before it is near enough to take the flip if 147's early press
        # were not read: its first press is before the flip, inside the
        # landing window.
        flip = (reach[147].latest_ms + chords[at]["ms"]) / 2
        self.assertLessEqual(reach[prev["index"]].earliest_ms, flip)
        self.assertLessEqual(flip - prev["ms"], sp_compare.LANDING_WINDOW_MS)
        got = sp_compare._landed_chord(chords, chord_ms, flip, reach)
        self.assertEqual(got["index"], 147)

    def test_bomber_180_reaches_its_late_press(self):
        # Chord 180 has one press, aimed after the chord's own ms. A rise on
        # the press's row is credited; a rise between the chord's ms and the
        # press is not, because the search starts at the press.
        reach, presses = self._assert_reach_spans_presses(_BOMBER[1])
        self.assertEqual(len(presses[180]), 1, presses[180])
        chord = self._chord(_BOMBER[0], 180)
        press_ms = reach[180].latest_ms
        self.assertGreater(press_ms, chord["ms"])
        size = FULL_METER // sp_compare.PHRASES_PER_FULL_METER

        def rise_at(ms):
            clock_s = ms / 1000
            while sp_compare._ms({"clock_s": clock_s}) < ms:
                clock_s = math.nextafter(clock_s, math.inf)
            rows = [_row(chord["ms"] / 1000 - 0.1, 0), _row(clock_s, size)]
            return sp_compare._ms(rows[1]), _rise_lines(_BOMBER[0], reach, rows)

        shown, lines = rise_at(press_ms)
        self.assertIn(f"PASS phrase rise after {sp_compare._chord_label(chord)}: "
                      f"+{size} (0 -> {size}) at {sp_compare.format_ms(shown)}, full",
                      lines)
        _, early = rise_at((chord["ms"] + press_ms) / 2)
        self.assertEqual(len([l for l in early if l.startswith(
            f"FAIL phrase rise after {sp_compare._chord_label(chord)}: missing")]), 1,
            early)

    def test_senri_with_no_squeeze_reaches_its_one_press(self):
        # Chords 267 (plain) and 268 (sp_end) each have one press, so their
        # earliest and latest reach are that press.
        reach, presses = self._assert_reach_spans_presses(_fixture("senri_schedule.json"))
        for index in (267, 268):
            self.assertEqual(len(presses[index]), 1, presses[index])
            self.assertEqual(reach[index], (presses[index][0], presses[index][0]))


def _samples(*pairs):
    """Rows holding only the two columns gap_summary reads, from (wall_s,
    clock_s) pairs."""
    return [{"wall_s": wall_s, "clock_s": clock_s} for wall_s, clock_s in pairs]


# The wall_s and clock_s columns of a real run's watch CSV. The copy reads them
# by name, because the other columns are left out.
LS_O2_CSV = os.path.join(_REPO_ROOT, "testdata", "ch_watch", "ls-o2-run1-sp_watch-1.csv")
# The run folder is hydra-data/ch-mcp-runs/2026-11-07/ls-o2-run1. Its
# replies.txt holds ch_start's "song length: 62080 ms".
LS_O2_SONG_END_MS = 62080


class GapSummaryTest(unittest.TestCase):
    def test_two_gaps_give_the_count_the_longest_and_the_clocks_around_them(self):
        # The real ls-o2 run's first 165 rows. Its first two wall-time gaps
        # over 50 ms are its data rows 162 -> 163 (73.576 ms, from clock
        # 2146.834 ms) and 163 -> 164 (68.144 ms, to clock 2290.694 ms),
        # read off the file (0-based rows).
        got = sp_compare.gap_summary(self._ls_o2_rows()[:165], 50)
        self.assertEqual(got.count, 2)
        self.assertAlmostEqual(got.longest_ms, 73.576, places=3)
        self.assertAlmostEqual(got.from_ms, 2146.834, places=3)
        self.assertAlmostEqual(got.to_ms, 2290.694, places=3)

    def test_rows_with_no_gap_over_give_0(self):
        rows = _samples((100.0, 1.0), (100.004, 1.004), (100.008, 1.008))
        self.assertEqual(sp_compare.gap_summary(rows, 50), (0, None, None, None))

    def test_no_rows_and_one_row_give_0(self):
        self.assertEqual(sp_compare.gap_summary([], 50), (0, None, None, None))
        self.assertEqual(sp_compare.gap_summary(_samples((100.0, 1.0)), 50),
                         (0, None, None, None))

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
        # The real ls-o2 run ended on a stopped clock: its last two rows show
        # one clock, past the song's end, with a wall-time gap over the
        # threshold between them. With the song end that pair adds nothing.
        rows = self._ls_o2_rows()
        last_gap = self._last_gap_ms(rows)
        self.assertGreater(last_gap, 50)
        got = sp_compare.gap_summary(rows, 50, LS_O2_SONG_END_MS)
        self.assertEqual(got, sp_compare.gap_summary(rows[:-1], 50, LS_O2_SONG_END_MS))
        self.assertLess(got.longest_ms, last_gap)
        self.assertLessEqual(got.to_ms, LS_O2_SONG_END_MS)

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

    def _last_gap_ms(self, rows):
        """The wall-time gap of the file's last pair, which must show one
        clock: the watch writes its final row once live.StoppedCheck says the
        clock has stopped."""
        self.assertEqual(rows[-1]["clock_s"], rows[-2]["clock_s"])
        return sp_compare.C.s_to_ms(rows[-1]["wall_s"] - rows[-2]["wall_s"])

    def _assert_a_gap_of_the_file(self, rows, got, song_end_ms):
        """The summary's longest gap and its first gap's start are pairs of
        consecutive rows in the file, over the threshold and inside the song."""
        def inside(i):
            return song_end_ms is None or max(sp_compare._ms(rows[i - 1]),
                                              sp_compare._ms(rows[i])) <= song_end_ms
        def gap(i):
            return sp_compare.C.s_to_ms(rows[i]["wall_s"] - rows[i - 1]["wall_s"])
        self.assertTrue(any(inside(i) and gap(i) == got.longest_ms
                            for i in range(1, len(rows))))
        self.assertTrue(any(inside(i) and gap(i) > 50
                            and sp_compare._ms(rows[i - 1]) == got.from_ms
                            for i in range(1, len(rows))))
        self.assertLessEqual(got.from_ms, got.to_ms)

    def test_the_real_ls_o2_run_inside_its_song(self):
        # Every clock in the file rises or holds, so leaving out the rows past
        # the song end gives the summary the song end gives.
        rows = self._ls_o2_rows()
        self.assertTrue(all(rows[i]["clock_s"] >= rows[i - 1]["clock_s"]
                            for i in range(1, len(rows))))
        inside = [r for r in rows if sp_compare._ms(r) <= LS_O2_SONG_END_MS]
        self.assertLess(len(inside), len(rows), "the file should run past the song end")
        got = sp_compare.gap_summary(rows, 50, LS_O2_SONG_END_MS)
        self.assertGreater(got.count, 0)
        self.assertEqual(got, sp_compare.gap_summary(inside, 50))
        self.assertLessEqual(got.to_ms, LS_O2_SONG_END_MS)
        self._assert_a_gap_of_the_file(rows, got, LS_O2_SONG_END_MS)

    def test_the_real_ls_o2_run_with_no_song_end(self):
        # With no song end the final frozen pair counts, and it is the longest
        # gap: one more gap than the file without its last row, ending on the
        # last row's clock.
        rows = self._ls_o2_rows()
        got = sp_compare.gap_summary(rows, 50)
        before = sp_compare.gap_summary(rows[:-1], 50)
        self.assertEqual(got.count, before.count + 1)
        self.assertEqual(got.longest_ms, self._last_gap_ms(rows))
        self.assertEqual(got.to_ms, sp_compare._ms(rows[-1]))
        self.assertEqual(got.from_ms, before.from_ms)
        self._assert_a_gap_of_the_file(rows, got, None)


if __name__ == "__main__":
    unittest.main()
