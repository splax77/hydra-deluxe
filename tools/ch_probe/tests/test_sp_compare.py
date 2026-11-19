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
