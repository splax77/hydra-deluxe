"""play_chart.py plays the presses of a `hydra_replay schedule` file.

load_schedule reads the file as written: which gems each press sends, when,
and by when it must be sent. tools/replay_schedule.cpp owns every timing
number in it, so these tests check what play_chart does with the file and
never work a timing out again. The schedules come from two places. The
committed `*_schedule.json` fixtures beside this file are real files, written
by hydra_replay schedule from real charts. _schedule_of builds a small file by
hand, with hand-picked times, for a case no fixture shows. Waiting for a
press, noticing the song stopped and picking the first press when a run joins
mid-song are live.py's.

A run is tested on fakes for the song clock, the score and the keys, so no
test opens the game or sends a key: the press log's rows, the frozen-clock
stop, the wall-time cap, and main()'s read-only open and capped engine search.
"""

from __future__ import annotations

import contextlib
import csv
import inspect
import io
import json
import math
import os
import statistics
import sys
import tempfile
import time
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import engine_finder  # noqa: E402
from tools.ch_probe.experiments import live, overlay_logs, play_chart  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402


# Eight chord blocks copied whole from a real run of
#   hydra_replay score --chart "testdata/input/common/IB24/T2/Area 11 - Knightmare Frame/notes.mid" --acts ""
# (Expert Pro Drums, 2x Bass), one per kind of pad: yellow cymbal, the 2x
# kick, kick + yellow cymbal + green cymbal, kick + green cymbal, yellow tom,
# green tom, blue cymbal, and a red accent. Each block's "measure" object is
# written by hand in the shape the tool prints ("label" and "decimal"); the
# label is the measure, beat and tick as the screen writes them.
_DUMP = {"chords": json.loads(r"""[
{"chord_code": "..N..", "combo_before": 53, "cum": {"accent": 0, "notes": 2715, "combo": 5295, "ghost": 0, "solo": 0, "sp": 0, "total": 8010}, "cum_onscreen_total": 8010, "in_sp": false, "index": 38, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 10.0, "label": "m11.1.0"}, "ms": 13043.48, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Yellow", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "notes": 65, "combo": 195, "ghost": 0, "solo": 0, "sp": 0}, "tick": 19200},
{"chord_code": "N....", "combo_before": 56, "cum": {"accent": 0, "notes": 2880, "combo": 5790, "ghost": 0, "solo": 0, "sp": 0, "total": 8670}, "cum_onscreen_total": 8670, "in_sp": false, "index": 41, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 10.4375, "label": "m11.2.360"}, "ms": 13614.132249999999, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "notes": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 20040},
{"chord_code": "n.N.N", "combo_before": 63, "cum": {"accent": 0, "notes": 3360, "combo": 7230, "ghost": 0, "solo": 0, "sp": 0, "total": 10590}, "cum_onscreen_total": 10590, "in_sp": false, "index": 45, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 10.875, "label": "m11.4.240"}, "ms": 14184.7845, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}, {"color": "Yellow", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}, {"color": "Green", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "notes": 180, "combo": 540, "ghost": 0, "solo": 0, "sp": 0}, "tick": 20880},
{"chord_code": "n...N", "combo_before": 130, "cum": {"accent": 0, "notes": 7020, "combo": 18210, "ghost": 0, "solo": 0, "sp": 0, "total": 25230}, "cum_onscreen_total": 25230, "in_sp": false, "index": 91, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 16.875, "label": "m17.4.240"}, "ms": 22010.872499999998, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}, {"color": "Green", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "notes": 115, "combo": 345, "ghost": 0, "solo": 0, "sp": 0}, "tick": 32400},
{"chord_code": "..n..", "combo_before": 136, "cum": {"accent": 0, "notes": 7270, "combo": 18960, "ghost": 0, "solo": 0, "sp": 0, "total": 26230}, "cum_onscreen_total": 26230, "in_sp": false, "index": 95, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 17.375, "label": "m18.2.240"}, "ms": 22663.046499999997, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Yellow", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "notes": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 33360},
{"chord_code": "....n", "combo_before": 139, "cum": {"accent": 0, "notes": 7420, "combo": 19410, "ghost": 0, "solo": 0, "sp": 0, "total": 26830}, "cum_onscreen_total": 26830, "in_sp": false, "index": 97, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 17.625, "label": "m18.3.240"}, "ms": 22989.133499999996, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Green", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "notes": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 33840},
{"chord_code": "...N.", "combo_before": 595, "cum": {"accent": 0, "notes": 32620, "combo": 95010, "ghost": 0, "solo": 0, "sp": 0, "total": 127630}, "cum_onscreen_total": 127630, "in_sp": false, "index": 400, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 60.0, "label": "m61.1.0"}, "ms": 78260.88, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Blue", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "notes": 65, "combo": 195, "ghost": 0, "solo": 0, "sp": 0}, "tick": 115200},
{"chord_code": ".a...", "combo_before": 846, "cum": {"accent": 50, "notes": 46325, "combo": 136275, "ghost": 0, "solo": 0, "sp": 0, "total": 182650}, "cum_onscreen_total": 182650, "in_sp": false, "index": 573, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"decimal": 83.25, "label": "m84.2.0"}, "ms": 108586.97099999999, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Red", "cymbal": false, "dynamic": "accent", "dynamics_bonus": 200, "multiplier": 4, "sp_points": 400}], "points": {"accent": 50, "notes": 50, "combo": 300, "ghost": 0, "solo": 0, "sp": 0}, "tick": 159840}
]""")}


def _schedule_of(score):
    """A hand-built schedule file for the chords of a score-shaped dict: one
    plain press per chord at a hand-picked aim of 2 ms early, with a
    hand-picked last send 50 ms after the chord, and a hand-picked song end
    of 200,000 ms. These are test inputs, not the schedule's rules;
    tools/replay_schedule.cpp owns those."""
    chords, presses = [], []
    for chord in score["chords"]:
        gems = [dict(gem, place=k) for k, gem in enumerate(chord["notes"])]
        chords.append({"index": chord["index"], "tick": chord.get("tick", 0),
                       "ms": chord["ms"], "side_ms": 50.0, "gems": gems,
                       "ends_sp": False, "declined": False, "paid_by": [],
                       "reads": [], "refused": False})
        presses.append({"chord": chord["index"], "gems": list(range(len(gems))),
                        "aim_ms": -2.0, "after_ms": None, "rule": "plain",
                        "timed_from_end": None, "end_read_after": None,
                        "aim_end_ms": None, "room_ms": 48.0,
                        "last_send_ms": chord["ms"] + 50.0})
    return {"song_length_ms": 200000.0, "activations": [], "chords": chords,
            "presses": presses, "refusals": []}


def _write_and_load(doc, loader):
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "schedule.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(doc, f)
        return loader(path)


def _load_chords(score=None):
    return _write_and_load(_schedule_of(_DUMP if score is None else score),
                           play_chart.load_schedule).chords


def _load():
    """(ms, keys) of each chord of _DUMP, in order."""
    return [(chord.ms, chord.lanes) for chord in _load_chords()]


def _read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def _unit_end_fields(aim_end_ms):
    """play_chart.END_FIELDS for a press whose read end is its own end, as
    hand-picked test inputs: every end at `aim_end_ms`, conversion 1 on both
    sides and no range. With these retime_from_end moves a press one for one,
    as it did before the fields existed."""
    return {"own_end_ms": aim_end_ms, "read_stored_end_ms": aim_end_ms,
            "own_stored_end_ms": aim_end_ms, "conversion": {"early": 1.0, "late": 1.0},
            "read_range_ms": None}


_FREAK_SCHEDULE = os.path.join(_HERE, "freak_limits_schedule.json")


class LoadScheduleTest(unittest.TestCase):
    """load_schedule reads hydra_replay schedule's file as written (design
    point 7 of docs/superpowers/plans/2026-11-08-replay-press-schedule.md)."""

    def test_load_schedule_gives_one_press_per_group(self):
        doc = _read_json(_FREAK_SCHEDULE)
        schedule = play_chart.load_schedule(_FREAK_SCHEDULE)
        loaded = [p for chord in schedule.chords for p in chord.presses]
        self.assertEqual(len(loaded), len(doc["presses"]))
        by_chord = {}
        for entry in doc["presses"]:
            by_chord.setdefault(entry["chord"], []).append(entry)
        for chord in schedule.chords:
            entries = by_chord.get(chord.index, [])
            self.assertEqual(len(chord.presses), len(entries), chord.index)
            for press, entry in zip(chord.presses, entries):
                self.assertEqual(press.aim_ms, entry["aim_ms"])
                self.assertEqual(press.after_ms, entry["after_ms"])
                self.assertEqual(press.last_send_ms, entry["last_send_ms"])
                self.assertEqual(press.fallback, entry.get("fallback"))
                self.assertEqual(press.timed_from_end, entry["timed_from_end"])
                self.assertEqual(press.end_read_after, entry["end_read_after"])
                self.assertEqual(press.aim_end_ms, entry["aim_end_ms"])
                self.assertEqual(press.lanes, [chord.lanes[g] for g in entry["gems"]])

    def test_freak_squeeze_out_press_is_the_files(self):
        # Literals read from freak_limits_schedule.json: chord 387's late
        # press and its fallback, and chord 263's anchor press.
        chords = {c.index: c for c in play_chart.load_schedule(_FREAK_SCHEDULE).chords}
        (late,) = chords[387].presses
        self.assertEqual(late.rule, "squeeze_out_late")
        self.assertEqual(late.aim_ms, 24.265838069011807)
        self.assertEqual(late.last_send_ms, 60595.849704541964)
        self.assertEqual(late.end_read_after, 263)
        self.assertEqual(late.fallback["if_end_after_ms"], 60580.849704541964)
        self.assertEqual(late.fallback["action"], "join_group")
        (anchor,) = chords[263].presses
        self.assertEqual(anchor.aim_ms, -21.824207380682793)
        # The end is read after the anchor; its planned end is the read's.
        self.assertEqual(anchor.planned_end_ms, 60554.52732830211)

    def test_a_declined_chord_sends_its_kept_gems(self):
        # Freak chord 51 is a skipped fill: kick and blue tom, kept_gems [0].
        chords = {c.index: c for c in play_chart.load_schedule(_FREAK_SCHEDULE).chords}
        chord = chords[51]
        self.assertTrue(chord.declined)
        self.assertEqual(chord.lanes, [Lane.KICK, Lane.BLUE])
        self.assertEqual([p.lanes for p in chord.presses], [[Lane.KICK]])
        self.assertTrue(all(p.declined for p in chord.presses))

    def test_the_file_level_values_are_read(self):
        doc = _read_json(_FREAK_SCHEDULE)
        schedule = play_chart.load_schedule(_FREAK_SCHEDULE)
        self.assertEqual(schedule.song_length_ms, doc["song_length_ms"])
        self.assertEqual(len(schedule.activations), len(doc["activations"]))
        for act, entry in zip(schedule.activations, doc["activations"]):
            self.assertEqual(act.sp_ready_ms, entry["sp_ready_ms"])
            self.assertEqual(act.read_end_after, entry["read_end_after"])

    def test_press_plan_returns_the_presses_in_firing_order(self):
        chords = play_chart.load_schedule(_FREAK_SCHEDULE).chords
        plan = play_chart.press_plan(chords)
        self.assertEqual(sorted(plan, key=lambda p: p.at_ms), plan)
        self.assertEqual(len(plan), len(_read_json(_FREAK_SCHEDULE)["presses"]))

    def test_a_refusal_in_the_file_stops_the_load_quoting_it(self):
        doc = _schedule_of(_DUMP)
        doc["refusals"] = ["chord 41 is a made-up refusal"]
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        self.assertIn("chord 41 is a made-up refusal", str(caught.exception))

    def test_an_infeasible_squeeze_stops_the_load_naming_it(self):
        doc = _schedule_of(_DUMP)
        doc["activations"] = [{"index": 0, "chord": 38, "read_end_after": 38,
                               "sp_ready_ms": None, "squeezes": [
                                   {"chord": 91, "kind": "SqOut", "feasible": False,
                                    "anchor_room_ms": -1.5, "squeeze_room_ms": -2.5}]}]
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        # The refusal names the squeeze's chord; its wording is the loader's.
        self.assertIn("chord 91", str(caught.exception))

    def test_no_deleted_name_remains(self):
        # Design point 8 of the replay press-schedule plan, with DECLINE_MODES
        # added by docs/superpowers/plans/2026-11-08-player-audit-fixes.md.
        deleted = (
            "PRESS_LEAD_MS", "SP_END_LEAD_MS", "SP_OUT_LAG_MS", "ACT_LEAD_MARGIN_MS",
            "FRAME_MS", "CHORD_ORDER_GAP_MS", "EARLY", "LATE", "SQUEEZE_IN",
            "SQUEEZE_OUT", "_TOP_PAD_ORDER", "_END_SCALE", "_STEADY_ACT", "_STEADY_END",
            "Squeeze", "ActEnd", "ActLead", "half_gap_ms", "window_side_ms",
            "note_window_ms", "window_sides", "activation_leads", "_solve_lead",
            "_order_groups", "_squeeze_out_inside_groups", "_chord_presses",
            "_timed_presses", "_refuse_far_presses", "_dump_path",
            "declined_chord_indexes", "_kept_gems", "_flagged_gem", "load_dump_chords",
            "_sp_acts", "_act_ends", "_stored_end_ms", "_squeezes", "load_dump_notes",
            "DECLINE_KICK_ONLY", "DECLINE_NOTHING", "DECLINE_ALL_BUT_TOP_PAD",
            "DECLINE_MODES", "WINDOW_FORMULA_NORMAL_LINEAR",
            "WINDOW_FORMULA_NORMAL_SQUARE", "WINDOW_FORMULA_NORMAL_SCALE",
            "WINDOW_FORMULA_NORMAL_OFFSET_MS")
        import re
        from tools.ch_probe import constants
        for module in (play_chart, constants):
            with open(module.__file__, encoding="utf-8") as f:
                text = f.read()
            for name in deleted:
                self.assertIsNone(re.search(r"\b" + re.escape(name) + r"\b", text),
                                  f"{name} in {os.path.basename(module.__file__)}")

    def test_no_timing_constant_is_defined_in_python(self):
        numeric = sorted(name for name, value in vars(play_chart).items()
                         if name.isupper() or name.lstrip("_").isupper()
                         if isinstance(value, (int, float)) and not isinstance(value, bool))
        self.assertEqual(numeric, ["RUN_CAP_S", "SCORE_READ_DELAY_S", "_SAME_MS"])
        self.assertEqual([name for name in vars(play_chart) if name.endswith("_MS")],
                         ["_SAME_MS"])

    def test_score_times_are_the_chords_index_and_ms(self):
        # What the trace's note list reads from a score file.
        self.assertEqual(_write_and_load(_DUMP, play_chart.load_score_times),
                         [(c.index, c.ms) for c in _load_chords()])

    def test_chord_codes_are_not_decoded_here(self):
        # The notes say the pads; play_chart never reads the ADR 0015 code.
        self.assertFalse(hasattr(play_chart, "lanes_for_code"))


class SharedPiecesTest(unittest.TestCase):
    """play_chart runs on the tracked modules, not private copies."""

    def test_no_private_copies_are_left(self):
        for name in ("find_active_engine", "OFF_SONG_CLOCK", "OFF_SCORE",
                     "NOTE_TO_LANE", "scan_for_engine",
                     "parse_chart", "parse_midi", "_read_vlq",
                     "midi_notes_to_lanes", "chart_notes_to_lanes",
                     "ticks_to_seconds", "TempoEvent", "CHART_DRUM_NOTES",
                     "DRUM_NOTES", "SYNOVIAL_DIR",
                     "wait_until", "first_note_index", "STALL_S"):
            self.assertFalse(hasattr(play_chart, name), name)

    def test_lane_names_come_from_the_key_table(self):
        from tools.ch_probe import input_driver
        self.assertIs(play_chart.LANE_NAMES, input_driver.LANE_NAMES)

    def test_window_helper_comes_from_input_driver(self):
        from tools.ch_probe import input_driver
        self.assertIs(play_chart.find_game_window, input_driver.find_game_window)

    def test_waiting_and_the_start_cursor_come_from_live(self):
        self.assertIs(play_chart.live, live)
        self.assertIs(play_chart.live.wait_until, live.wait_until)
        self.assertIs(play_chart.live.first_note_index, live.first_note_index)

    def test_songclock_lives_in_live(self):
        self.assertIs(play_chart.SongClock, live.SongClock)


# --- a run against fakes: no process, no keys -----------------------------------

class _FakeTime:
    """The wall clock: sleep moves it on, and so does every song-clock read."""

    def __init__(self) -> None:
        self.t = 0.0

    def now(self) -> float:
        return self.t

    def sleep(self, seconds: float) -> None:
        self.t += seconds


class _FakeSongClock:
    """The game's song clock: `start_s` plus `rate` song seconds per wall
    second, held at `freeze_at_s` once it gets there. Each read costs 1 ms of
    wall time, so a spin-wait moves on."""

    def __init__(self, wall: _FakeTime, start_s: float, rate: float = 1.0,
                 freeze_at_s=None) -> None:
        self.wall, self.start_s, self.rate = wall, start_s, rate
        self.freeze_at_s = freeze_at_s

    def read(self):
        self.wall.t += 0.001
        song_s = self.start_s + self.rate * self.wall.t
        if self.freeze_at_s is not None:
            song_s = min(song_s, self.freeze_at_s)
        return song_s, song_s


class _FakeGame:
    """The score: 100 points per group pressed, except the presses numbered
    in `misses` (1-based). A split chord's later groups (`then`) are one
    press with the first; `messages` keeps each call's groups."""

    def __init__(self, misses=()) -> None:
        self.score = 0
        self.presses = []
        self.messages = []
        self.misses = set(misses)

    def press(self, lanes, _axes=None, then=()) -> None:
        self.presses.append(list(lanes))
        self.messages.append([list(lanes)] + [list(group) for group, _axes, _after in then])
        if len(self.presses) not in self.misses:
            self.score += 100 * (1 + len(then))

    def read_score(self) -> int:
        return self.score


def _run(clock_kwargs, misses=()):
    wall = _FakeTime()
    game = _FakeGame(misses)
    result = play_chart.play_chords(
        _load_chords(),
        _FakeSongClock(wall, **clock_kwargs), game.read_score, game.press,
        now=wall.now, sleep=wall.sleep, out=io.StringIO())
    return result, game, wall


class PressLogTest(unittest.TestCase):
    def test_one_row_per_press_with_the_press_log_columns(self):
        result, game, _wall = _run({"start_s": 12.0}, misses=(3,))
        self.assertEqual(result.stop, "done")
        self.assertEqual(result.sent, 8)
        # D319 item 1: the player counts no hits or streaks from a score rise.
        self.assertFalse(hasattr(result, "hit"))
        self.assertFalse(hasattr(result, "max_streak"))
        self.assertEqual(game.presses, [list(lanes) for _ms, lanes in _load()])

        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                table = list(csv.reader(f))

        self.assertEqual(tuple(table[0]), overlay_logs.PRESS_LOG_COLUMNS)
        body = table[1:]
        col = {name: i for i, name in enumerate(table[0])}
        self.assertEqual([r[col["index"]] for r in body],
                         ["38", "41", "45", "91", "95", "97", "400", "573"])
        self.assertEqual([r[col["chart_ms"]] for r in body], [
            "13043.48", "13614.132249999999", "14184.7845", "22010.872499999998",
            "22663.046499999997", "22989.133499999996", "78260.88",
            "108586.97099999999"])
        # The third press scored nothing, and its row still says only that it
        # was sent: the scores are the game's, and no column judges them.
        self.assertEqual([(r[col["score_before"]], r[col["score_after"]], r[col["send"]])
                          for r in body], [
            ("0", "100", "sent"), ("100", "200", "sent"), ("200", "200", "sent"),
            ("200", "300", "sent"), ("300", "400", "sent"), ("400", "500", "sent"),
            ("500", "600", "sent"), ("600", "700", "sent")])
        # The clock the wait returned: inside _schedule_of's 2 ms aim, before
        # the chord.
        for r in body:
            early_ms = float(r[col["chart_ms"]]) - float(r[col["clock_ms_at_press"]])
            self.assertGreater(early_ms, 0.0)
            self.assertLessEqual(early_ms, 2.0)


class RunStopsTest(unittest.TestCase):
    def test_a_frozen_clock_ends_the_run(self):
        # The clock holds at 20 s, between the third chord and the fourth.
        result, _game, wall = _run({"start_s": 12.0, "freeze_at_s": 20.0})
        self.assertEqual(result.stop, "frozen")
        self.assertEqual([r["index"] for r in result.rows], [38, 41, 45])
        self.assertLess(wall.t, play_chart.RUN_CAP_S)

    def test_the_wall_cap_ends_the_run(self):
        # A song clock running at a quarter speed reaches the chord at 78.26 s
        # only after 265 s of wall time, past the cap; it never freezes.
        result, _game, wall = _run({"start_s": 12.0, "rate": 0.25})
        self.assertEqual(result.stop, "cap")
        self.assertEqual([r["index"] for r in result.rows], [38, 41, 45, 91, 95, 97])
        self.assertGreaterEqual(wall.t, play_chart.RUN_CAP_S)
        self.assertLess(wall.t, play_chart.RUN_CAP_S + 1.0)

    def test_a_caller_given_cap_ends_the_run_there(self):
        # The first chord is at 13.04 s on a clock that starts at 12 s, so a
        # 1 s cap ends the run before any press.
        wall = _FakeTime()
        game = _FakeGame()
        out = io.StringIO()
        result = play_chart.play_chords(
            _load_chords(),
            _FakeSongClock(wall, start_s=12.0), game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, out=out, cap_s=1)
        self.assertEqual(result.stop, "cap")
        self.assertEqual(result.rows, [])
        self.assertGreaterEqual(wall.t, 1.0)
        self.assertLess(wall.t, 2.0)
        self.assertIn("run cap of 1 s reached", out.getvalue())


class _FakeProc:
    module_base = 0x10000000

    def __init__(self) -> None:
        self.verified = False
        self.closed = False

    def verify_targets(self) -> None:
        self.verified = True

    def close(self) -> None:
        self.closed = True


class LiveStartTest(unittest.TestCase):
    """main() up to the engine search, on a fake process and a finder that
    never finds a playing song."""

    def _main_with_no_song(self):
        proc = _FakeProc()
        opens = []

        def opener(**kwargs):
            opens.append(kwargs)
            return proc

        wall = _FakeTime()

        def never_finds(p):
            return engine_finder.find_live_engine_capped(
                p, [(b"\x00" * 8, b"\x01" * 8)],
                scan=lambda _proc, _back, _front: ([], 0, 0),
                sleep=wall.sleep, now=wall.now, out=io.StringIO())

        out = io.StringIO()
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "schedule.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump(_schedule_of(_DUMP), f)
            code = play_chart.main([path, "--log", os.path.join(d, "press.csv")],
                                   open_proc=opener, find_engine=never_finds,
                                   out=out)
            log_written = os.path.exists(os.path.join(d, "press.csv"))
        return code, out.getvalue(), proc, opens, log_written, never_finds

    def test_no_song_exits_2_with_the_finder_message(self):
        code, text, proc, _opens, log_written, never_finds = self._main_with_no_song()
        self.assertEqual(code, 2)
        # The message main prints is the one the finder itself raises.
        with self.assertRaises(engine_finder.EngineSearchTimeout) as caught:
            never_finds(_FakeProc())
        self.assertIn(str(caught.exception), text)
        self.assertTrue(proc.closed)
        self.assertFalse(log_written)

    def test_the_game_is_opened_read_only(self):
        _code, _text, proc, opens, _log, _finder = self._main_with_no_song()
        self.assertEqual(opens, [{"read_only": True}])
        self.assertTrue(proc.verified)


# --- the pieces a server calls ----------------------------------------------------

class CallerHooksTest(unittest.TestCase):
    """play_chords hands each row to `on_press` and asks `should_stop`."""

    def _run_with(self, on_press=None, should_stop=None):
        wall = _FakeTime()
        game = _FakeGame()
        result = play_chart.play_chords(
            _load_chords(),
            _FakeSongClock(wall, start_s=12.0), game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, on_press=on_press,
            should_stop=should_stop, out=io.StringIO())
        return result, game

    def test_on_press_sees_every_row_in_order(self):
        seen = []
        result, _game = self._run_with(on_press=seen.append)
        self.assertEqual(result.stop, "done")
        self.assertEqual([r["index"] for r in seen],
                         [38, 41, 45, 91, 95, 97, 400, 573])
        self.assertEqual(seen, result.rows)

    def test_should_stop_after_the_second_chord_ends_the_run(self):
        seen = []
        result, game = self._run_with(on_press=seen.append,
                                      should_stop=lambda: len(seen) >= 2)
        self.assertEqual(result.stop, "stopped")
        self.assertEqual([r["index"] for r in result.rows], [38, 41])
        self.assertEqual(len(game.presses), 2)

    def test_a_stop_leaves_no_key_held_down(self):
        # The real InputDriver press, with its key events recorded: the stop
        # is asked only between presses, so every key that went down is up.
        from tools.ch_probe.input_driver import InputDriver
        events = []
        driver = InputDriver()
        driver.send_key = lambda vk, key_up: events.append((vk, key_up))
        wall = _FakeTime()
        game = _FakeGame()
        seen = []
        result = play_chart.play_chords(
            _load_chords(),
            _FakeSongClock(wall, start_s=12.0), game.read_score,
            play_chart.live_press(driver, 0), now=wall.now, sleep=wall.sleep,
            on_press=seen.append, should_stop=lambda: len(seen) >= 3,
            out=io.StringIO())
        self.assertEqual(result.stop, "stopped")
        self.assertEqual(len(seen), 3)
        down = [vk for vk, key_up in events if not key_up]
        up = [vk for vk, key_up in events if key_up]
        self.assertEqual(len(down), 1 + 1 + 3)  # yellow cymbal, kick, chord of 3
        self.assertEqual(sorted(down), sorted(up))
        self.assertTrue(events[-1][1])

    def test_should_stop_false_changes_nothing(self):
        result, game = self._run_with(should_stop=lambda: False)
        self.assertEqual(result.stop, "done")
        self.assertEqual(len(game.presses), 8)


class _FakeDriver:
    """Records each chord press into a shared event list."""

    def __init__(self, events) -> None:
        self.events = events

    def press_chord(self, lanes):
        self.events.append(("press", list(lanes)))
        return ["keys"]


class LivePlayTest(unittest.TestCase):
    def test_no_engine_returns_the_finder_message_and_closes_the_game(self):
        proc = _FakeProc()
        opens = []

        def opener(**kwargs):
            opens.append(kwargs)
            return proc

        def times_out(_proc):
            raise engine_finder.EngineSearchTimeout("the fake finder gave up")

        events = []
        result = play_chart.live_play(
            _load_chords(), open_proc=opener,
            find_engine=times_out, driver=_FakeDriver(events),
            out=io.StringIO())
        self.assertEqual(result.stop, play_chart.STOP_NO_ENGINE)
        self.assertEqual(result.message, "the fake finder gave up")
        self.assertEqual(result.rows, [])
        self.assertEqual(opens, [{"read_only": True}])
        self.assertTrue(proc.verified)
        self.assertTrue(proc.closed)
        self.assertEqual(events, [])

    def test_the_default_engine_search_prints_its_dots_to_out_only(self):
        # A caller that gives live_play a buffer (the ch MCP server) must get
        # no stdout from the search: its stdout is the protocol channel.
        out, stdout = io.StringIO(), io.StringIO()
        with fakes.searching_for_no_song(engine_finder),                 contextlib.redirect_stdout(stdout):
            result = play_chart.live_play(
                _load_chords(),
                open_proc=lambda **kw: _FakeProc(), driver=_FakeDriver([]),
                out=out)
        self.assertEqual(result.stop, play_chart.STOP_NO_ENGINE)
        self.assertEqual(stdout.getvalue(), "")
        # The search's retry marks (engine_finder owns them) land in the
        # caller's buffer.
        self.assertIn(".", out.getvalue())

    def _cap_seen_by_play_chords(self, **live_kwargs):
        from unittest import mock

        class _FakeEngine:
            def __init__(self, _proc):
                pass

            def use_object(self, _ptr):
                pass

            def song_clock(self):
                return 0.0

            def score(self):
                return 0

            def sp_end(self):
                return None

        seen = {}

        def fake_play_chords(*args, **kwargs):
            seen.update(kwargs)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _FakeEngine), \
                mock.patch.object(play_chart, "find_game_window", lambda: 0), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load_chords(), open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, driver=_FakeDriver([]),
                out=io.StringIO(), **live_kwargs)
        return seen["cap_s"]

    def test_the_cap_reaches_play_chords(self):
        self.assertEqual(self._cap_seen_by_play_chords(cap_s=320.504), 320.504)
        self.assertEqual(self._cap_seen_by_play_chords(), play_chart.RUN_CAP_S)

    def test_live_play_takes_no_press_and_passes_no_kit(self):
        # live_play is the keys route only (D342): no kit press goes in, and
        # play_chords gets no `kit`.
        self.assertNotIn("press", inspect.signature(play_chart.live_play).parameters)
        seen = {}

        def fake_play_chords(*args, **kwargs):
            seen.update(kwargs)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine), \
                mock.patch.object(play_chart, "find_game_window", lambda: 0), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load_chords(), open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, driver=_FakeDriver([]),
                out=io.StringIO())
        self.assertIn("cap_s", seen)
        self.assertNotIn("kit", seen)

    def test_live_play_sends_each_chords_keys_through_the_driver(self):
        chords = _load_chords()
        events, windows = [], []

        def fake_play_chords(chords, clock, read_score, press, **kwargs):
            for chord in chords:
                press(chord.lanes)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine), \
                mock.patch.object(play_chart, "find_game_window", lambda: 0x77), \
                mock.patch.object(play_chart, "focus_window", windows.append), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            result = play_chart.live_play(
                chords, open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, driver=_FakeDriver(events),
                out=io.StringIO())
        self.assertEqual(result.stop, "done")
        self.assertEqual(events, [("press", list(c.lanes)) for c in chords])
        self.assertEqual(windows, [0x77] * len(chords))

    def test_only_the_harness_calls_play_chords_with_a_kit(self):
        # The stall harness's loop case is the one caller that hands
        # play_chords a kit press (its control); neither the player module nor
        # the ch MCP server's game tools does.
        import ast
        from tools.ch_mcp import game_tools
        for module in (play_chart, game_tools):
            with open(module.__file__, encoding="utf-8") as f:
                tree = ast.parse(f.read())
            for node in ast.walk(tree):
                if not isinstance(node, ast.Call):
                    continue
                name = getattr(node.func, "id", None) or getattr(node.func, "attr", None)
                if name == "play_chords":
                    self.assertNotIn("kit", [k.arg for k in node.keywords],
                                     f"{module.__name__} line {node.lineno}")


def _gem(color, dynamic, cymbal=False):
    return {"color": color, "cymbal": cymbal, "dynamic": dynamic}


# One chord of each strength in one press, then a plain chord. The words are
# the ones hydra_replay writes.
_DYNAMICS_DUMP = {"chords": [
    {"index": 3, "ms": 1000.0, "notes": [
        _gem("Red", "ghost"), _gem("Yellow", "accent", cymbal=True), _gem("Kick", "none")]},
    {"index": 4, "ms": 1500.0, "notes": [_gem("Green", "none")]},
]}


def _load_dump(dump):
    return _load_chords(dump)


class _ThresholdEngine:
    """An engine fake that counts its dynamics threshold reads."""

    threshold = 0
    reads = 0

    def __init__(self, _proc=None):
        pass

    def use_object(self, _ptr):
        pass

    def song_clock(self):
        return 0.0

    def score(self):
        return 0

    def sp_end(self):
        return None

    def dyn_threshold(self):
        type(self).reads += 1
        return type(self).threshold


class DynamicsTest(unittest.TestCase):
    """Each gem keeps its dynamic, and the kit route (live_in_game) reads the
    threshold for the ghosts it must press."""

    def setUp(self):
        _ThresholdEngine.reads = 0

    def _live_in_game(self, chords, threshold):
        from unittest import mock
        _ThresholdEngine.threshold = threshold
        seen, proc = [], _FakeProc()

        def fake_play_in_game(chords, player, **kwargs):
            seen.extend(chords)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine), \
                mock.patch.object(play_chart, "engine_addresses", lambda _engine: (0, 0)), \
                mock.patch.object(play_chart, "play_in_game", fake_play_in_game):
            try:
                result = play_chart.live_in_game(
                    chords, player=object(), open_proc=lambda **kw: proc,
                    find_engine=lambda _proc: 0x1000, out=io.StringIO())
            finally:
                self.assertTrue(proc.closed)
        return result, seen

    def test_chords_carry_each_gems_dynamic_beside_its_lane(self):
        chords = _load_dump(_DYNAMICS_DUMP)
        self.assertEqual(chords[0].lanes, [Lane.RED, Lane.YELLOW_CYMBAL, Lane.KICK])
        self.assertEqual(chords[0].dynamics, ["ghost", "accent", "none"])
        self.assertEqual(chords[1].dynamics, ["none"])

    def test_a_ghost_chart_reads_the_threshold_once_and_returns_it(self):
        from tools.ch_mcp.kit_tools import pad_axis
        result, seen = self._live_in_game(_load_dump(_DYNAMICS_DUMP), 64)
        self.assertEqual(_ThresholdEngine.reads, 1)
        self.assertEqual(result.dyn_threshold, 64)
        self.assertEqual(seen[0].presses[0].axes, [pad_axis("ghost", 64),
                                                   pad_axis("accent", 64), pad_axis("none", 64)])
        self.assertEqual(seen[1].presses[0].axes, [pad_axis("none", 64)])

    def test_a_ghost_chart_with_a_threshold_of_0_raises_naming_the_field(self):
        with self.assertRaises(ValueError) as caught:
            self._live_in_game(_load_dump(_DYNAMICS_DUMP), 0)
        # The field's name is read from constants, the offsets' one owner.
        from tools.ch_probe import constants
        field = [name for name, value in vars(constants).items()
                 if name.startswith("OFF_") and value is constants.OFF_DYN_THRESHOLD]
        self.assertTrue(field)
        self.assertTrue(any(name in str(caught.exception) for name in field),
                        str(caught.exception))

    def test_the_keys_route_reads_no_threshold_even_for_a_ghost_chart(self):
        from unittest import mock
        _ThresholdEngine.threshold = 0
        proc, out = _FakeProc(), io.StringIO()
        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine),                 mock.patch.object(play_chart, "play_chords",
                                  lambda *a, **k: play_chart.RunResult(stop="done")),                 mock.patch.object(play_chart, "find_game_window", lambda: 0),                 mock.patch.object(play_chart, "InputDriver", lambda: object()):
            result = play_chart.live_play(
                _load_dump(_DYNAMICS_DUMP), open_proc=lambda **kw: proc,
                find_engine=lambda _proc: 0x1000, out=out)
        self.assertEqual(result.stop, "done")
        self.assertEqual(_ThresholdEngine.reads, 0)
        self.assertIsNone(result.dyn_threshold)

    def test_a_chart_with_no_ghost_never_reads_the_threshold(self):
        result, seen = self._live_in_game(_load_chords(), 0)
        self.assertEqual(result.stop, "done")
        self.assertEqual(_ThresholdEngine.reads, 0)
        self.assertIsNone(result.dyn_threshold)
        self.assertEqual(seen[-1].dynamics, ["accent"])
        self.assertEqual(len(seen), 8)

    def test_each_chord_reaches_the_press_with_its_axes(self):
        wall = _FakeTime()
        game = _FakeGame()
        seen = []

        def press(lanes, axes):
            game.press(lanes)
            seen.append(axes)

        _ThresholdEngine.threshold = 64
        chords, _threshold = play_chart.with_axes(_load_dump(_DYNAMICS_DUMP), _ThresholdEngine())
        play_chart.play_chords(chords, _FakeSongClock(wall, start_s=0.5), game.read_score,
                               press, now=wall.now, sleep=wall.sleep, out=io.StringIO())
        self.assertEqual(seen, [chords[0].presses[0].axes, chords[1].presses[0].axes])

    def test_a_ghost_or_accent_kick_gets_the_axis_a_pad_gets(self):
        # T2 of docs/superpowers/plans/2026-11-08-kick-axis-bind.md: the kit
        # presses the kick as an axis, so with_axes must hand a kick its
        # pad_axis strength exactly as it hands a pad one.
        from tools.ch_mcp.kit_tools import pad_axis
        _ThresholdEngine.threshold = 64
        dump = {"chords": [
            {"index": 0, "ms": 1000.0, "notes": [_gem("Kick", "ghost")]},
            {"index": 1, "ms": 1500.0, "notes": [_gem("Kick", "accent"), _gem("Red", "ghost")]},
        ]}
        chords, threshold = play_chart.with_axes(_load_dump(dump), _ThresholdEngine())
        self.assertEqual(threshold, 64)
        self.assertEqual(chords[0].lanes, [Lane.KICK])
        self.assertEqual(chords[0].presses[0].axes, [pad_axis("ghost", 64)])
        self.assertLess(chords[0].presses[0].axes[0], 1.0)
        self.assertEqual(chords[1].lanes, [Lane.KICK, Lane.RED])
        self.assertEqual(chords[1].presses[0].axes, [pad_axis("accent", 64),
                                                     pad_axis("ghost", 64)])
        self.assertNotEqual(pad_axis("ghost", 64), pad_axis("accent", 64))


class PressReplyRowTest(unittest.TestCase):
    """A kit press's reply adds the values the press log has columns for to
    the row; a press that returns keys adds none."""

    def test_a_dict_reply_rides_into_the_row(self):
        wall = _FakeTime()
        game = _FakeGame()
        down_ms = fakes.KIT_EPOCH_MS + 7

        def press(lanes, _axes):
            game.press(lanes)
            return fakes.kit_reply([(40, 42, down_ms, down_ms + 12)], rewrite=True)

        result = play_chart.play_chords(
            _load_chords()[:1], _FakeSongClock(wall, start_s=12.0),
            game.read_score, press, now=wall.now, sleep=wall.sleep, out=io.StringIO(),
            kit=True)
        row = result.rows[0]
        self.assertEqual(row, {
            "index": 38, "chart_ms": 13043.48, "clock_ms_at_press": row["clock_ms_at_press"],
            "lead_ms": 2.0, "score_before": 0, "score_after": 100, "send": "sent",
            "scored_between": 0,
            "down_frame": 40, "up_frame": 42, "down_ms": down_ms, "up_ms": down_ms + 12,
            "wall_s_at_read": row["wall_s_at_read"], "wall_s_at_reply": row["wall_s_at_reply"],
            **{c: row[c] for c in _STEP_COLUMNS_OF_A_KIT_PRESS}})

    def test_a_key_press_adds_nothing(self):
        result, _game, _wall = _run({"start_s": 12.0})
        self.assertEqual(set(result.rows[0]), {
            "index", "chart_ms", "clock_ms_at_press", "lead_ms", "score_before",
            "score_after", "send", "scored_between", "wall_s_at_read", "wall_s_at_reply",
            "wait_read_max_ms", "wait_oversleep_max_ms", "press_took_ms",
            "score_read_took_ms"})


# The timing columns on the first row of a run's first kit press that reads
# no end: every step but the end read, and no hand-off before it.
_STEP_COLUMNS_OF_A_KIT_PRESS = (
    "wait_read_max_ms", "wait_oversleep_max_ms", "press_took_ms", "reply_check_took_ms",
    "score_read_took_ms")


# A wall clock far from the fake `now`, as time.time is from time.perf_counter.
_WALL_EPOCH_S = 1_700_000_000.0
_SLOW_PRESS_S = 0.25


class WallColumnsTest(unittest.TestCase):
    """Each row's two wall times come from `wall`, not from `now`."""

    def _run_with_a_slow_press(self, chords):
        fake = _FakeTime()
        game = _FakeGame()
        stamps = []

        def wall():
            return _WALL_EPOCH_S + fake.t

        def press(lanes, _axes):
            before = wall()
            fake.t += _SLOW_PRESS_S
            game.press(lanes)
            stamps.append((before, wall()))

        result = play_chart.play_chords(
            chords, _FakeSongClock(fake, start_s=12.0), game.read_score, press,
            now=fake.now, sleep=fake.sleep, wall=wall, out=io.StringIO())
        return result, stamps

    def test_the_columns_are_the_wall_times_around_the_press(self):
        result, stamps = self._run_with_a_slow_press(_load_chords()[:3])
        self.assertEqual(len(stamps), 3)
        self.assertEqual([(r["wall_s_at_read"], r["wall_s_at_reply"]) for r in result.rows],
                         stamps)

    def test_the_columns_are_on_the_wall_clocks_scale(self):
        result, _stamps = self._run_with_a_slow_press(_load_chords()[:3])
        for row in result.rows:
            self.assertGreater(row["wall_s_at_read"], _WALL_EPOCH_S)
            self.assertGreater(row["wall_s_at_reply"], row["wall_s_at_read"])

    def test_a_chord_that_sends_no_press_stamps_one_moment_twice(self):
        # A declined chord whose press keeps no gem, as the schedule writes it.
        doc = _schedule_of({"chords": _DUMP["chords"][:1]})
        doc["chords"][0]["declined"] = True
        doc["presses"][0]["gems"] = []
        declined = _write_and_load(doc, play_chart.load_schedule).chords
        result, stamps = self._run_with_a_slow_press(declined)
        self.assertEqual(stamps, [])
        self.assertEqual(result.rows[0]["wall_s_at_read"], result.rows[0]["wall_s_at_reply"])

    def test_the_wall_clock_is_the_watchs_and_now_stays_the_timer(self):
        parameters = inspect.signature(play_chart.play_chords).parameters
        self.assertIs(parameters["wall"].default, time.time)
        self.assertIs(parameters["now"].default, time.perf_counter)


def _table(path):
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.reader(f))


class PressLogWriterTest(unittest.TestCase):
    """PressLogWriter keeps the log on disk row by row, as a run makes it,
    from a thread of its own."""

    # How long a test waits on the writer's thread before it fails instead
    # of hanging.
    CAP_S = 5.0

    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.path = os.path.join(folder.name, "press.csv")

    def _writer(self):
        writer = play_chart.PressLogWriter(self.path)
        writer.start()
        self.addCleanup(writer.stop, self.CAP_S)
        return writer

    def _table_once(self, count):
        """The log's table once it holds a header and `count` rows, read
        while the writer is open; fails after CAP_S."""
        deadline = time.monotonic() + self.CAP_S
        while True:
            table = _table(self.path) if os.path.exists(self.path) else []
            if len(table) >= 1 + count or time.monotonic() > deadline:
                return table
            time.sleep(0.005)

    def test_the_writer_builds_its_rows_through_press_log_writer(self):
        rows = [{"index": 1, "lead_ms": 2.0, "send": "sent"}, {"index": 2, "send": "dropped"}]
        real = play_chart._press_log_writer
        with mock.patch.object(play_chart, "_press_log_writer", side_effect=real) as built:
            writer = self._writer()
            for row in rows:
                writer.add(row)
            self.assertTrue(writer.stop(self.CAP_S))
        self.assertEqual(built.call_count, 1)
        whole = self.path + ".whole"
        play_chart.write_press_log(whole, rows)
        self.assertEqual(_table(self.path), _table(whole))

    def test_every_row_is_on_disk_before_the_writer_closes(self):
        fake = _FakeTime()
        game = _FakeGame()
        writer = self._writer()
        result = play_chart.play_chords(
            _load_chords()[:3], _FakeSongClock(fake, start_s=12.0),
            game.read_score, game.press, now=fake.now, sleep=fake.sleep,
            on_press=writer.add, out=io.StringIO())
        self.assertEqual(result.stop, "done")
        # A second handle, with the writer still open.
        table = self._table_once(3)
        self.assertEqual(tuple(table[0]), overlay_logs.PRESS_LOG_COLUMNS)
        self.assertEqual([r[0] for r in table[1:]], ["38", "41", "45"])
        self.assertTrue(writer.stop(self.CAP_S))

    def test_a_start_writes_the_file_over_with_the_header(self):
        with open(self.path, "w", encoding="utf-8") as f:
            f.write("an older run's rows\n")
        writer = self._writer()
        self.assertTrue(writer.stop(self.CAP_S))
        self.assertEqual(_table(self.path), [list(overlay_logs.PRESS_LOG_COLUMNS)])

    def test_a_failed_write_is_kept_and_ends_the_writing(self):
        real = play_chart._press_log_writer

        class Failing:
            def __init__(self, f):
                self._real = real(f)
                self.writeheader = self._real.writeheader

            def writerow(self, row):
                raise PermissionError("held open")

        with mock.patch.object(play_chart, "_press_log_writer", new=Failing):
            writer = self._writer()
            writer.add({"index": 1})
            writer.add({"index": 2})
            self.assertTrue(writer.stop(self.CAP_S))
        self.assertIsInstance(writer.error, PermissionError)
        self.assertEqual([r[0] for r in _table(self.path)], ["index"])

    def test_a_log_nowhere_writable_is_kept_as_the_error(self):
        writer = play_chart.PressLogWriter(os.path.join(self.path, "no such folder", "x.csv"))
        writer.start()
        writer.add({"index": 1})
        self.assertTrue(writer.stop(self.CAP_S))
        self.assertIsInstance(writer.error, OSError)

    def test_a_row_costs_the_adding_thread_less_than_the_score_read_sleep(self):
        # The median, so one scan of the file by another process fails nothing.
        row = {column: 1 for column in overlay_logs.PRESS_LOG_COLUMNS}
        writer = self._writer()
        took = []
        for _ in range(100):
            began = time.perf_counter()
            writer.add(row)
            took.append(time.perf_counter() - began)
        self.assertLess(statistics.median(took), play_chart.SCORE_READ_DELAY_S)
        self.assertTrue(writer.stop(self.CAP_S))
        self.assertEqual(len(_table(self.path)), 101)


_MOUTHS_SCHEDULE = os.path.join(_HERE, "mouths_refine_schedule.json")


def _play_schedule(chords, read_sp_end=None):
    """A run of `chords` on the fakes, from a second before the first chord."""
    wall = _FakeTime()
    game = _FakeGame()
    result = play_chart.play_chords(
        chords, _FakeSongClock(wall, start_s=chords[0].ms / 1000 - 1.0), game.read_score,
        game.press, now=wall.now, sleep=wall.sleep, out=io.StringIO(), cap_s=600.0,
        read_sp_end=read_sp_end)
    return result, game


def _rows_of(result, index):
    return [row for row in result.rows if row["index"] == index]


def _split_chords():
    """Chord 7, kick and green cymbal, split in two order groups by hand: the
    kick 2 ms early, the cymbal 8 ms after it. Hand-picked test inputs."""
    doc = _schedule_of({"chords": [{"index": 7, "ms": 1000.0, "notes": [
        _gem("Kick", "none"), _gem("Green", "none", cymbal=True)]}]})
    first = doc["presses"][0]
    doc["presses"] = [dict(first, gems=[0]),
                      dict(first, gems=[1], aim_ms=6.0, after_ms=8.0)]
    return _write_and_load(doc, play_chart.load_schedule).chords


class _SplitReplyPress:
    """A kit press whose agent replies per group, as kit.ts does for a press
    with later groups; it keeps each call's arguments."""

    def __init__(self) -> None:
        self.calls = []

    def __call__(self, lanes, axes=None, then=()):
        self.calls.append((list(lanes), list(then)))
        return fakes.kit_reply(
            [(500 + 3 * k, 502 + 3 * k, _SPLIT_DOWN_MS + 9 * k, _SPLIT_DOWN_MS + 6 + 9 * k)
             for k in range(1 + len(then))],
            after_ms=[0.0] + [after for _lanes, _axes, after in then])


# The first group's down on the agent's clock in _SplitReplyPress's replies.
_SPLIT_DOWN_MS = fakes.KIT_EPOCH_MS + 9000


class SplitSendTest(unittest.TestCase):
    """play_chords sends a split chord as one message and writes a row per
    group (design points 4 and 9 of the player-limits plan)."""

    def _run(self, chords, scores, press):
        wall = _FakeTime()
        reads = iter(scores)
        result = play_chart.play_chords(
            chords, _FakeSongClock(wall, start_s=0.5), lambda: next(reads), press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), kit=True)
        self.assertEqual(result.stop, "done")
        return result

    def test_a_two_group_reply_gives_two_rows_with_their_own_frames_and_one_score(self):
        press = _SplitReplyPress()
        # The start read, the fresh read before the press, the read after it.
        result = self._run(_split_chords(), [0, 0, 250], press)
        self.assertEqual(press.calls, [([Lane.KICK], [([Lane.GREEN_CYMBAL], None, 8.0)])])
        rows = result.rows
        self.assertEqual([(r["index"], r["down_frame"], r["up_frame"], r["down_ms"], r["up_ms"])
                          for r in rows],
                         [(7, 500, 502, 1762600009000, 1762600009006),
                          (7, 503, 505, 1762600009009, 1762600009015)])
        self.assertEqual([(r["score_before"], r["score_after"], r["send"]) for r in rows],
                         [(0, 250, "sent"), (0, 250, "sent")])
        self.assertEqual([round(r["lead_ms"], 2) for r in rows], [2.0, -6.0])
        self.assertEqual(rows[0]["clock_ms_at_press"], rows[1]["clock_ms_at_press"])
        self.assertEqual(rows[0]["scored_between"], 0)
        self.assertNotIn("scored_between", rows[1])
        self.assertEqual(result.sent, 1)

    def test_a_chord_of_one_group_is_sent_with_no_then(self):
        press = _SplitReplyPress()
        chords = _load_chords({"chords": [
            {"index": 7, "ms": 1000.0, "notes": [_gem("Kick", "none"), _gem("Red", "none")]}]})
        result = self._run(chords, [0, 0, 100], press)
        self.assertEqual(press.calls, [([Lane.KICK, Lane.RED], [])])
        self.assertEqual(len(result.rows), 1)

    def test_the_mouths_fixture_sends_one_message_per_split_chord(self):
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        result, game = _play_schedule(chords)
        self.assertEqual(result.stop, "done")
        self.assertEqual(result.dropped, 0)
        self.assertTrue(any(len(groups) == 2 for groups in game.messages))
        plan = play_chart.press_messages(play_chart.press_plan(chords))
        sent = [m for m in plan if m[0].lanes or not m[0].declined]
        self.assertEqual(len(game.messages), len(sent))
        self.assertEqual(sum(1 + len(later) for _head, later in plan), len(result.rows))
        rows_4 = _rows_of(result, 4)
        self.assertEqual([round(r["lead_ms"], 2) for r in rows_4], [2.0, -6.0])
        self.assertEqual(len({r["score_after"] for r in rows_4}), 1)

    def test_a_later_group_with_no_first_press_is_a_bug_said_by_name(self):
        lone = play_chart.Press(index=3, chart_ms=1000.0, aim_ms=6.0, lanes=[Lane.RED],
                                dynamics=["none"], axes=None, declined=False,
                                last_send_ms=1050.0, rule="plain", after_ms=8.0)
        with self.assertRaises(ValueError) as caught:
            play_chart.press_messages([lone])
        self.assertIn("chord 3 has an order group 8 ms after no press of its own",
                      str(caught.exception))


class ScoredBetweenTest(unittest.TestCase):
    """The points the game scores between presses go in the later row's
    `scored_between`, and the row's own step starts from the fresh read
    (design point 9 of the player-limits plan)."""

    def _chords(self, declined):
        doc = _schedule_of({"chords": [
            {"index": 854, "ms": 1000.0, "notes": [_gem("Kick", "none"), _gem("Green", "none")]},
            {"index": 855, "ms": 1200.0, "notes": [_gem("Red", "none")]}]})
        if declined:
            # A declined chord whose press keeps no gem, as the schedule writes it.
            doc["chords"][0]["declined"] = True
            doc["presses"][0]["gems"] = []
        return _write_and_load(doc, play_chart.load_schedule).chords

    def _run(self, declined):
        wall = _FakeTime()
        # Start; 854's fresh read and its read after; 855's fresh read, which
        # holds 854's grant of 200, and its read after.
        reads = iter([0, 0, 0 if declined else 50, 200, 400])
        return play_chart.play_chords(
            self._chords(declined), _FakeSongClock(wall, start_s=0.5), lambda: next(reads),
            lambda lanes, axes=None: None, now=wall.now, sleep=wall.sleep, out=io.StringIO())

    def test_a_rise_between_presses_is_the_later_rows_scored_between(self):
        result = self._run(declined=True)
        self.assertEqual([(r["index"], r["scored_between"], r["score_before"], r["score_after"])
                          for r in result.rows], [(854, 0, 0, 0), (855, 200, 200, 400)])
        self.assertEqual([r["send"] for r in result.rows],
                         [play_chart.DECLINED, play_chart.SENT])

    def test_the_status_credits_the_pending_declined_chord(self):
        from tools.ch_mcp import game_tools
        result = self._run(declined=True)
        self.assertEqual(game_tools.scored_between_lines(result.rows),
                         ["scored between presses: chord 854 granted 200 before chord 855"])

    def test_a_rise_with_no_declined_chord_is_unexplained(self):
        from tools.ch_mcp import game_tools
        result = self._run(declined=False)
        self.assertEqual([r["scored_between"] for r in result.rows], [0, 150])
        self.assertEqual(game_tools.scored_between_lines(result.rows),
                         ["scored between presses: 150 unexplained before chord 855"])

    def test_the_new_columns_reach_the_press_log(self):
        result = self._run(declined=True)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                table = list(csv.DictReader(f))
        self.assertEqual([r["scored_between"] for r in table], ["0", "200"])
        self.assertEqual([r["end_read_ms"] for r in table], ["", ""])
        for column in ("scored_between", "end_read_ms", "probe"):
            self.assertIn(column, overlay_logs.PRESS_LOG_COLUMNS)


class ReadEndTest(unittest.TestCase):
    """play_chords reads the game's end after each chord whose `reads` names
    an activation, and retime_from_end moves the presses the schedule times
    from that read (their `end_read_after`), on the Freak fixture schedule
    (design point 1 of the player-limits plan)."""

    def _chords(self):
        return play_chart.load_schedule(_FREAK_SCHEDULE).chords

    def test_the_presses_timed_from_a_read_are_the_files(self):
        # Chord, the chord its end is read after, and aim_ms, as the file writes them.
        self.assertEqual([(p.index, p.end_read_after, p.aim_ms)
                          for p in play_chart.press_plan(self._chords())
                          if p.end_read_after is not None],
                         [(387, 263, 24.265838069011807), (750, 633, -15.0),
                          (1144, 1062, 39.55059269245248), (1369, 1302, -15.0),
                          (1573, 1512, -15.0)])

    def test_an_end_3_ms_later_moves_the_squeeze_presses_by_3(self):
        from tools.ch_probe import constants as C
        chords = self._chords()
        planned = [p.planned_end_ms for p in play_chart.press_plan(chords)
                   if p.planned_end_ms is not None]
        later = iter([C.ms_to_s(ms + 3.0) for ms in planned])
        result, _game = _play_schedule(chords, lambda: next(later))
        self.assertEqual(result.stop, "done")
        # Each chord the file's `reads` lists holds one read; no other row does.
        self.assertEqual([r["index"] for r in result.rows if "end_read_ms" in r],
                         [232, 263, 633, 1062, 1302, 1512])
        # The file's planned_end_ms of chord 263's read is 60554.53.
        self.assertAlmostEqual(_rows_of(result, 263)[0]["end_read_ms"], 60557.53, delta=0.01)
        # Each press timed from a read moves by the read minus its own
        # aim_end_ms. The squeeze presses 387 and 1144 aim at the read's
        # planned end, so they fire 3 ms later: aims of 24.27 and 39.55 ms
        # become leads of -27.27 and -42.55. The sp_end presses 750, 1369 and
        # 1573 aim at an end 1 ms before their read's planned end, so they
        # move 4 ms: aims of -15 ms become leads of 11.
        self.assertEqual([[round(r["lead_ms"], 2) for r in _rows_of(result, index)]
                          for index in (387, 750, 1144, 1369, 1573)],
                         [[-27.27], [11.0], [-42.55], [11.0], [11.0]])
        # A row not timed from a read keeps the file's aim of -2 ms.
        self.assertEqual([round(r["lead_ms"], 2) for r in _rows_of(result, 388)], [2.0])

    def test_a_clear_active_byte_leaves_the_plan_and_says_so(self):
        result, _game = _play_schedule(self._chords(), lambda: None)
        self.assertEqual(_rows_of(result, 263)[0]["end_read_ms"], play_chart.NO_ACTIVATION_READ)
        self.assertEqual(play_chart.NO_ACTIVATION_READ, "no activation read")
        self.assertEqual([round(r["lead_ms"], 2) for r in _rows_of(result, 387)], [-24.27])

    def test_retime_from_end_moves_only_that_activations_presses(self):
        def press(index, chart_ms, read_after=None):
            return play_chart.Press(
                index=index, chart_ms=chart_ms, aim_ms=0.0, lanes=[Lane.RED],
                dynamics=["none"], axes=None, declined=False, last_send_ms=chart_ms + 50.0,
                rule="plain", end_read_after=read_after,
                aim_end_ms=None if read_after is None else 1000.0,
                **({} if read_after is None else _unit_end_fields(1000.0)))
        plan = [press(1, 990.0, 7), press(2, 991.0), press(3, 1200.0, 8)]
        moved = play_chart.retime_from_end(plan, 7, 1002.0)
        self.assertEqual([(p.index, p.at_ms, p.aim_end_ms) for p in moved],
                         [(2, 991.0, None), (1, 992.0, 1002.0), (3, 1200.0, 1000.0)])


class SpEndTest(unittest.TestCase):
    """EngineModel.sp_end reads the stored end and the active byte in one read."""

    def _engine(self, active):
        import struct
        from tools.ch_probe import constants as C
        from tools.ch_probe.engine import EngineModel
        engine_ptr = 0x5000_0000
        block = bytearray(C.OFF_SP_END_TIME + 8 - C.OFF_SP_ACTIVE)
        block[0] = active
        struct.pack_into("<d", block, C.OFF_SP_END_TIME - C.OFF_SP_ACTIVE, 32.61592)
        proc, _seen = fakes.fake_process({engine_ptr + C.OFF_SP_ACTIVE: bytes(block)},
                                         0x1_4000_0000)
        engine = EngineModel(proc)
        engine.use_object(engine_ptr)
        return engine

    def test_the_end_in_song_seconds_while_active(self):
        self.assertEqual(self._engine(1).sp_end(), 32.61592)

    def test_none_while_the_byte_is_clear(self):
        self.assertIsNone(self._engine(0).sp_end())

    def test_live_play_hands_play_chords_the_engines_reader(self):
        from unittest import mock
        seen = {}

        def fake_play_chords(chords, clock, read_score, press, **kwargs):
            seen.update(kwargs)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine), \
                mock.patch.object(play_chart, "find_game_window", lambda: 0), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load_chords(), open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, driver=_FakeDriver([]),
                out=io.StringIO())
        self.assertEqual(seen["read_sp_end"].__func__, _ThresholdEngine.sp_end)


class LivePressTest(unittest.TestCase):
    """live_press focuses through input_driver.focus_window, then presses."""

    def _press(self, hwnd):
        from unittest import mock
        from tools.ch_probe import input_driver
        events = []

        def focus(h):
            # The real rule, with a recorder in place of SetForegroundWindow.
            input_driver.focus_window(
                h, focus=lambda w: events.append(("focus", w)))

        with mock.patch.object(play_chart, "focus_window", focus):
            sent = play_chart.live_press(_FakeDriver(events), hwnd)(
                [Lane.RED, Lane.KICK])
        return events, sent

    def test_focuses_the_window_then_presses(self):
        events, sent = self._press(0x1234)
        self.assertEqual(events, [("focus", 0x1234),
                                  ("press", [Lane.RED, Lane.KICK])])
        self.assertEqual(sent, ["keys"])

    def test_no_window_presses_without_focusing(self):
        events, _sent = self._press(0)
        self.assertEqual(events, [("press", [Lane.RED, Lane.KICK])])

    def test_the_focus_comes_from_input_driver(self):
        from tools.ch_probe import input_driver
        self.assertIs(play_chart.focus_window, input_driver.focus_window)


# --- a press past its window is dropped (design point 5 of
# docs/superpowers/plans/2026-11-08-player-freak-fixes.md) -----------------------

class _JumpingSongClock(_FakeSongClock):
    """_FakeSongClock that jumps `jump_s` song seconds ahead once it reaches
    `jump_at_s`, as the game's clock did after its stall in Freak run 1."""

    def __init__(self, wall: _FakeTime, start_s: float, jump_at_s: float,
                 jump_s: float) -> None:
        super().__init__(wall, start_s)
        self.jump_at_s, self.jump_s = jump_at_s, jump_s

    def read(self):
        song_s, _est = super().read()
        if song_s >= self.jump_at_s:
            song_s += self.jump_s
        return song_s, song_s


class FrozenClockDropTest(unittest.TestCase):
    """The game stalls during the wait for chord 41, so the run reaches it
    about 107 ms late, as Freak run 1 reached chord 153."""

    def _run(self):
        chords = _load_chords()
        wall = _FakeTime()
        game = _FakeGame()
        # Chord 41 fires at 13,612.13 ms; the clock jumps 131 ms 22 ms before.
        clock = _JumpingSongClock(wall, start_s=12.0, jump_at_s=13.590, jump_s=0.131)
        result = play_chart.play_chords(
            chords, clock, game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO())
        return chords, result, game

    def test_the_late_chord_is_dropped_and_the_next_is_sent(self):
        chords, result, game = self._run()
        self.assertEqual(result.stop, "done")
        (row,) = _rows_of(result, 41)
        late_ms = row["clock_ms_at_press"] - row["chart_ms"]
        self.assertGreater(late_ms, 100.0)
        self.assertLess(late_ms, 110.0)
        # The precondition: the read is past the press's last send.
        self.assertGreater(row["clock_ms_at_press"], chords[1].presses[0].last_send_ms)
        self.assertEqual(row["send"], "dropped")
        self.assertEqual(row["score_before"], row["score_after"])
        for column in ("down_frame", "up_frame", "down_ms", "up_ms", "end_read_ms"):
            self.assertNotIn(column, row)
        # Chord 41 is the lone kick: every press but its own was sent.
        expected = [list(lanes) for ms, lanes in _load() if ms != chords[1].ms]
        self.assertEqual(game.presses, expected)
        self.assertEqual(_rows_of(result, 45)[0]["send"], "sent")

    def test_the_counts_leave_the_drop_out(self):
        _chords, result, _game = self._run()
        self.assertEqual((result.sent, result.dropped, result.declined), (7, 1, 0))

    def test_the_printed_line_says_dropped(self):
        chords = _load_chords()
        wall = _FakeTime()
        game = _FakeGame()
        out = io.StringIO()
        play_chart.play_chords(
            chords, _JumpingSongClock(wall, 12.0, 13.590, 0.131), game.read_score,
            game.press, now=wall.now, sleep=wall.sleep, out=out)
        self.assertIn("dropped", out.getvalue())


class _ScriptedSongClock:
    """A song clock that reads `values_s` in turn and then holds the last;
    each read costs 1 ms of wall time."""

    def __init__(self, wall: _FakeTime, values_s) -> None:
        self.wall, self.values_s = wall, list(values_s)

    def read(self):
        self.wall.t += 0.001
        song_s = self.values_s.pop(0) if len(self.values_s) > 1 else self.values_s[0]
        return song_s, song_s


def _clock_s_at(bound_ms):
    """A clock read, in song seconds, whose ms is `bound_ms` exactly in
    floats: the boundary of the drop check."""
    import math
    from tools.ch_probe import constants as C
    raw_s = C.ms_to_s(bound_ms)
    for _ in range(256):
        ms = C.s_to_ms(raw_s)
        if ms == bound_ms:
            return raw_s
        raw_s = math.nextafter(raw_s, math.inf if ms < bound_ms else -math.inf)
    raise AssertionError(f"no clock read gives exactly {bound_ms} ms")


class DropBoundaryTest(unittest.TestCase):
    """play_chords sends a message read exactly at its first press's
    last_send_ms and drops one read 0.01 ms past it, with every group of it
    (design point 5 of the player-freak-fixes plan). The hand-built
    schedules put last_send_ms 50 ms after the chord."""

    def _run(self, chords, past_ms, press):
        from tools.ch_probe import constants as C
        wall = _FakeTime()
        bound_ms = chords[0].presses[0].last_send_ms
        raw_s = _clock_s_at(bound_ms)
        if past_ms:
            raw_s = C.ms_to_s(C.s_to_ms(raw_s) + past_ms)
        scores = iter([0, 0, 100])
        result = play_chart.play_chords(
            chords, _ScriptedSongClock(wall, [0.5, raw_s]), lambda: next(scores), press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), kit=True)
        self.assertEqual(result.stop, "done")
        return result

    def _kick(self):
        return _load_chords({"chords": [{"index": 7, "ms": 1000.0,
                                         "notes": [_gem("Kick", "none")]}]})

    def test_read_at_the_last_send_is_sent(self):
        press = _SplitReplyPress()
        result = self._run(self._kick(), 0.0, press)
        self.assertEqual(press.calls, [([Lane.KICK], [])])
        self.assertEqual([r["send"] for r in result.rows], ["sent"])
        self.assertEqual((result.sent, result.dropped), (1, 0))

    def test_read_0_01_ms_past_the_last_send_is_dropped(self):
        press = _SplitReplyPress()
        result = self._run(self._kick(), 0.01, press)
        self.assertEqual(press.calls, [])
        self.assertEqual([r["send"] for r in result.rows], ["dropped"])
        self.assertEqual((result.sent, result.dropped), (0, 1))

    def test_a_split_chord_past_its_last_send_sends_nothing_and_drops_every_group(self):
        press = _SplitReplyPress()
        chords = _split_chords()
        (head, later), = play_chart.press_messages(play_chart.press_plan(chords))
        self.assertEqual(len(later), 1)
        result = self._run(chords, 10.0, press)
        self.assertEqual(press.calls, [])
        self.assertEqual([(r["index"], r["send"]) for r in result.rows],
                         [(7, "dropped"), (7, "dropped")])
        self.assertEqual((result.sent, result.dropped), (0, 1))


def _press_of(chords, index, rule=None):
    """The one press of chord `index` (with `rule`, when given) in `chords`."""
    found = [p for c in chords if c.index == index for p in c.presses
             if rule is None or p.rule == rule]
    assert len(found) == 1, (index, rule, found)
    return found[0]


def _reads(chords, overrides):
    """A read_sp_end for a run of `chords`: each read gives the planned end
    of the chord it is read after, in seconds, except the chords in
    `overrides`, whose read gives that many ms instead (a float, read as
    exactly that ms by _clock_s_at)."""
    planned = [p for p in play_chart.press_plan(chords) if p.planned_end_ms is not None]
    ends = [overrides.get(p.index, p.planned_end_ms) for p in planned]
    values = iter([_clock_s_at(ms) for ms in ends])
    return lambda: next(values)


class ReadAfterAnchorTest(unittest.TestCase):
    """play_chords reads the game's end after each chord the schedule's
    `reads` names, the activation's own and its `read_end_after` chord, and
    retime_from_end moves each press timed from a read by how far its own
    end moves, own_end_now_ms of that read minus the press's `own_end_ms`
    (design points 4 and 7 of
    docs/superpowers/plans/2026-11-08-replay-press-schedule.md and design
    point 3 of
    `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`)).
    On the Freak
    fixture, activation 0 starts on chord 232 and is read again after 263;
    387's late press is timed from the read after 263."""

    def _chords(self):
        return play_chart.load_schedule(_FREAK_SCHEDULE).chords

    def _run(self, overrides):
        chords = self._chords()
        result, game = _play_schedule(chords, _reads(chords, overrides))
        self.assertEqual(result.stop, "done")
        return result

    def test_the_end_is_read_after_the_anchor_chord_not_only_the_activation(self):
        chords = self._chords()
        late = _press_of(chords, 387)
        self.assertEqual((late.end_read_after, late.aim_end_ms), (263, 60554.52732830211))
        # 232's read comes back 5 ms late and 263's 2 ms before 387's aim_end_ms.
        result = self._run({232: 57933.88726341159 + 5.0, 263: late.aim_end_ms - 2.0})
        reads = {r["index"]: r["end_read_ms"] for r in result.rows if "end_read_ms" in r}
        self.assertEqual(sorted(reads), [232, 263, 633, 1062, 1302, 1512])
        self.assertAlmostEqual(reads[232], 57938.89, delta=0.01)
        self.assertAlmostEqual(reads[263], 60552.53, delta=0.01)
        # 387 fires 2 ms earlier: its file aim of 24.27 ms late becomes 22.27.
        # Had only 232's read counted, it would have moved 5 ms later.
        self.assertEqual([round(r["lead_ms"], 2) for r in _rows_of(result, 387)], [-22.27])
        # 232's read moves nothing: 263's presses keep the file's aims.
        self.assertEqual([round(r["lead_ms"], 2) for r in _rows_of(result, 263)],
                         [round(-p.aim_ms, 2) for p in _press_of_all(chords, 263)])

    def test_a_read_equal_to_aim_end_moves_nothing(self):
        late = _press_of(self._chords(), 387)
        result = self._run({263: late.aim_end_ms})
        self.assertEqual([r["lead_ms"] for r in _rows_of(result, 387)], [-late.aim_ms])

    def test_each_press_moves_by_its_own_end(self):
        # Mouths chords 431 (a squeeze-in) and 465 (an sp_end press) are both
        # timed from the read after 340, each with its own end fields in the
        # file: one read moves each by how far its own end moves, the
        # production own_end_now_ms minus the file's own_end_ms (design point
        # 3 of
        # `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`)).
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        squeeze, sp_end = _press_of(chords, 431), _press_of(chords, 465)
        self.assertEqual((squeeze.end_read_after, sp_end.end_read_after), (340, 340))
        read_ms = 68985.0
        moved = {p.index: p for p in play_chart.retime_from_end([squeeze, sp_end], 340, read_ms)}
        shifts = {}
        for press in (squeeze, sp_end):
            shifts[press.index] = play_chart.own_end_now_ms(press, read_ms) - press.own_end_ms
            self.assertAlmostEqual(moved[press.index].aim_ms - press.aim_ms,
                                   shifts[press.index], delta=1e-9)
        # Each press moved by its own end, not the other press's.
        self.assertNotAlmostEqual(shifts[431], shifts[465], delta=1e-6)

    def test_no_activation_read_leaves_the_plan(self):
        result, _game = _play_schedule(self._chords(), lambda: None)
        self.assertEqual(_rows_of(result, 263)[0]["end_read_ms"], play_chart.NO_ACTIVATION_READ)
        late = _press_of(self._chords(), 387)
        self.assertEqual([r["lead_ms"] for r in _rows_of(result, 387)], [-late.aim_ms])
        self.assertNotIn("retimed", _rows_of(result, 387)[0])


def _press_of_all(chords, index):
    """Chord `index`'s presses in `chords`, in firing order."""
    return sorted((p for c in chords if c.index == index for p in c.presses),
                  key=lambda p: p.at_ms)


class FallbackTest(unittest.TestCase):
    """A moved press with a `fallback` does what the block says at the
    block's threshold, and its row says so in `retimed` (design point 5 of
    docs/superpowers/plans/2026-11-08-replay-press-schedule.md, and its
    question 3 (a) for the squeeze-in). The fixtures give the thresholds:
    Freak 1144's `if_end_after_ms`, Mouths 740's (whose late gem joins gems
    1 and 2) and Mouths 431's `if_end_before_ms`."""

    def _run(self, path, overrides):
        chords = play_chart.load_schedule(path).chords
        result, game = _play_schedule(chords, _reads(chords, overrides))
        self.assertEqual(result.stop, "done")
        return chords, result, game

    def test_a_read_end_later_than_planned_moves_the_late_gem_by_the_same_ms(self):
        late = _press_of(play_chart.load_schedule(_FREAK_SCHEDULE).chords, 1144)
        _chords, result, _game = self._run(_FREAK_SCHEDULE, {1062: late.aim_end_ms + 3.0})
        (row,) = _rows_of(result, 1144)
        self.assertAlmostEqual(row["lead_ms"], -(late.aim_ms + 3.0), delta=1e-6)
        self.assertEqual(row["retimed"], play_chart.RETIMED_LATE)

    def test_a_read_end_past_the_threshold_joins_the_inside_group(self):
        late = _press_of(play_chart.load_schedule(_FREAK_SCHEDULE).chords, 1144)
        bound = late.fallback["if_end_after_ms"]
        self.assertAlmostEqual(bound, 141607.563330396, delta=1e-6)
        self.assertEqual(late.fallback["join_gems"], [])
        _c, past, _g = self._run(_FREAK_SCHEDULE, {1062: bound + 0.01})
        (row,) = _rows_of(past, 1144)
        self.assertEqual(row["retimed"], play_chart.RETIMED_INSIDE)
        # With no inside group the gem goes down 15 ms before its own end,
        # which here is the read end (conversion 1).
        self.assertAlmostEqual(row["lead_ms"], late.chart_ms - (bound + 0.01 - 15.0), delta=1e-6)
        _c, before, _g = self._run(_FREAK_SCHEDULE, {1062: bound - 0.01})
        (row,) = _rows_of(before, 1144)
        self.assertEqual(row["retimed"], play_chart.RETIMED_LATE)
        self.assertAlmostEqual(row["lead_ms"], -(late.aim_ms + bound - 0.01 - late.aim_end_ms),
                               delta=1e-6)

    def test_a_read_end_exactly_at_the_threshold_keeps_the_late_press(self):
        late = _press_of(play_chart.load_schedule(_FREAK_SCHEDULE).chords, 1144)
        _c, result, _g = self._run(_FREAK_SCHEDULE, {1062: late.fallback["if_end_after_ms"]})
        self.assertEqual(_rows_of(result, 1144)[0]["retimed"], play_chart.RETIMED_LATE)

    def test_the_late_gem_goes_down_with_its_chords_last_inside_group(self):
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        late = _press_of(chords, 740, "squeeze_out_late")
        inside = _press_of(chords, 740, "squeeze_out_inside")
        self.assertEqual(late.fallback["join_gems"], [1, 2])
        bound = late.fallback["if_end_after_ms"]
        _c, result, game = self._run(_MOUTHS_SCHEDULE, {704: bound + 0.01})
        (row,) = _rows_of(result, 740)
        self.assertEqual(row["retimed"], play_chart.RETIMED_INSIDE)
        self.assertIn(sorted(inside.lanes + late.lanes), [sorted(m[0]) for m in game.messages])
        _c, kept, game = self._run(_MOUTHS_SCHEDULE, {704: bound - 0.01})
        self.assertEqual([r.get("retimed") for r in _rows_of(kept, 740)],
                         [None, play_chart.RETIMED_LATE])

    def test_a_squeeze_in_read_end_before_the_threshold_presses_plain(self):
        squeeze = _press_of(play_chart.load_schedule(_MOUTHS_SCHEDULE).chords, 431)
        self.assertEqual(squeeze.fallback["action"], "plain")
        bound = squeeze.fallback["if_end_before_ms"]
        _c, early, _g = self._run(_MOUTHS_SCHEDULE, {340: bound - 0.01})
        (row,) = _rows_of(early, 431)
        self.assertEqual(row["retimed"], play_chart.RETIMED_PLAIN)
        self.assertEqual(row["lead_ms"], -squeeze.fallback["aim_ms"])
        _c, at, _g = self._run(_MOUTHS_SCHEDULE, {340: bound})
        (row,) = _rows_of(at, 431)
        self.assertNotIn("retimed", row)
        self.assertAlmostEqual(row["lead_ms"], -(squeeze.aim_ms + bound - squeeze.aim_end_ms),
                               delta=1e-6)

    def test_a_press_past_last_send_is_dropped(self):
        # Freak chord 388, a plain press, alone; DropBoundaryTest's run reads
        # the clock exactly at its last_send_ms, or 0.01 ms past it.
        chords = [c for c in play_chart.load_schedule(_FREAK_SCHEDULE).chords if c.index == 388]
        sent = DropBoundaryTest._run(self, chords, 0.0, _SplitReplyPress())
        self.assertEqual([r["send"] for r in sent.rows], [play_chart.SENT])
        dropped = DropBoundaryTest._run(self, chords, 0.01, _SplitReplyPress())
        self.assertEqual([r["send"] for r in dropped.rows], [play_chart.DROPPED])

    def test_a_block_with_another_action_is_refused(self):
        late = _press_of(play_chart.load_schedule(_FREAK_SCHEDULE).chords, 1144)
        import dataclasses
        odd = dataclasses.replace(late, fallback={**late.fallback, "action": "drop"})
        with self.assertRaisesRegex(ValueError, "'drop'"):
            play_chart.retime_from_end([odd], 1062, late.aim_end_ms)


def _timed_press(index=5, chart_ms=3000.0, aim_ms=-15.0, lanes=(Lane.RED,), read_after=4,
                 **fields):
    """A hand-built press timed from the read after chord `read_after`, with
    hand-picked test inputs. `fields` gives its END_FIELDS, `aim_end_ms` and
    `fallback`."""
    return play_chart.Press(
        index=index, chart_ms=chart_ms, aim_ms=aim_ms, lanes=list(lanes),
        dynamics=["none"] * len(lanes), axes=None, declined=False,
        last_send_ms=chart_ms + 50.0, rule="plain", end_read_after=read_after, **fields)


class ConversionRetimeTest(unittest.TestCase):
    """retime_from_end moves a press by how far its own end moved, through
    the file's conversion, never one for one with the read (design points 3
    and 5 of
    `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`))."""

    def test_pitch_dark_chord_644_moves_by_the_conversion(self):
        # The plan's Pitch Dark numbers: the stored ends of steps 2 and 1,
        # chord 644's planned own end, its conversion, the read planned after
        # chord 466, and the game's read in the 2026-11-12 run. The chord's
        # time and aim are hand-picked.
        press = _timed_press(index=644, chart_ms=161389.776, aim_ms=-18.316, read_after=466,
                             aim_end_ms=158262.531, own_end_ms=161386.46,
                             read_stored_end_ms=158224.757, own_stored_end_ms=161356.443,
                             conversion={"early": 0.79467, "late": 0.79467},
                             read_range_ms=None)
        (moved,) = play_chart.retime_from_end([press], 466, 158268.988)
        # 5.13 ms later; one for one with the read would be 6.46 against the
        # planned read, or 14.2 against the read priced at the borrowed ratio.
        self.assertAlmostEqual(moved.at_ms - press.at_ms, 5.13, delta=0.01)
        self.assertAlmostEqual(moved.own_end_ms, 161391.59, delta=0.01)
        self.assertEqual(moved.aim_end_ms, 158268.988)
        self.assertIsNone(moved.retimed)

    def _sided(self, **more):
        return _timed_press(aim_end_ms=1000.0, own_end_ms=2000.0, read_stored_end_ms=1000.0,
                            own_stored_end_ms=2000.0, conversion={"early": 0.5, "late": 2.0},
                            read_range_ms=None, **more)

    def test_a_read_before_the_stored_end_takes_early_and_after_takes_late(self):
        press = self._sided()
        (early,) = play_chart.retime_from_end([press], 4, 990.0)
        self.assertAlmostEqual(early.at_ms - press.at_ms, -5.0, delta=1e-9)
        (late,) = play_chart.retime_from_end([press], 4, 1010.0)
        self.assertAlmostEqual(late.at_ms - press.at_ms, 20.0, delta=1e-9)
        (at,) = play_chart.retime_from_end([press], 4, 1000.0)
        self.assertEqual(at.at_ms, press.at_ms)

    def test_the_late_gem_joins_before_the_moved_own_end(self):
        # Conversion 0.5 and a read 10 ms late: the own end moves 5 ms, so
        # the gem goes down 15 ms before 2,005, at 1,990, not 1,995.
        late = _timed_press(chart_ms=1990.0, aim_ms=20.0, aim_end_ms=1000.0,
                            own_end_ms=2000.0, read_stored_end_ms=1000.0,
                            own_stored_end_ms=2000.0, conversion={"early": 0.5, "late": 0.5},
                            read_range_ms=None,
                            fallback={"action": "join_group", "if_end_after_ms": 1005.0,
                                      "before_end_ms": 15.0, "join_gems": [],
                                      "retimed": play_chart.RETIMED_INSIDE})
        (moved,) = play_chart.retime_from_end([late], 4, 1010.0)
        self.assertEqual(moved.retimed, play_chart.RETIMED_INSIDE)
        self.assertAlmostEqual(moved.at_ms, 1990.0, delta=1e-9)

    def test_a_press_missing_a_field_is_refused(self):
        press = _timed_press(aim_end_ms=1000.0, own_end_ms=1000.0, read_stored_end_ms=1000.0,
                             conversion={"early": 1.0, "late": 1.0})
        with self.assertRaisesRegex(ValueError, "own_stored_end_ms"):
            play_chart.retime_from_end([press], 4, 1001.0)


class ReadRangeTest(unittest.TestCase):
    """A read inside a press's `read_range_ms` moves it by the sum; one
    outside takes its written fallback, or leaves it where the schedule put
    it, and says `range` (design point 5 of
    `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`)).
    Every number is a hand-picked test input."""

    RANGE = (990.0, 1010.0)

    def _press(self, **more):
        fields = dict(aim_end_ms=1000.0, own_end_ms=1000.0, read_stored_end_ms=1000.0,
                      own_stored_end_ms=1000.0, conversion={"early": 1.0, "late": 1.0},
                      read_range_ms=self.RANGE)
        fields.update(more)
        return _timed_press(**fields)

    def test_a_read_just_inside_moves_by_the_sum(self):
        press = self._press()
        for read_ms in self.RANGE:
            with self.subTest(read_ms):
                (moved,) = play_chart.retime_from_end([press], 4, read_ms)
                self.assertAlmostEqual(moved.at_ms - press.at_ms, read_ms - 1000.0, delta=1e-9)
                self.assertIsNone(moved.retimed)

    def test_a_null_side_is_open(self):
        for low, high, read_ms in ((None, 1010.0, -1e9), (990.0, None, 1e9)):
            with self.subTest(low=low, high=high):
                press = self._press(read_range_ms=(low, high))
                (moved,) = play_chart.retime_from_end([press], 4, read_ms)
                self.assertIsNone(moved.retimed)
                (kept,) = play_chart.retime_from_end(
                    [press], 4, 1010.01 if high is not None else 989.99)
                self.assertEqual(kept.retimed, play_chart.RETIMED_RANGE)

    def test_a_press_with_no_fallback_stays_unmoved(self):
        press = self._press()
        for read_ms in (self.RANGE[0] - 0.01, self.RANGE[1] + 0.01):
            with self.subTest(read_ms):
                (kept,) = play_chart.retime_from_end([press], 4, read_ms)
                self.assertEqual(kept.at_ms, press.at_ms)
                self.assertEqual(kept.retimed, play_chart.RETIMED_RANGE)

    def test_a_squeeze_in_outside_is_pressed_plain(self):
        # The threshold is far below the read, so only the range sends it plain.
        press = self._press(fallback={"action": "plain", "if_end_before_ms": 900.0,
                                      "aim_ms": -2.0, "retimed": play_chart.RETIMED_PLAIN})
        (plain,) = play_chart.retime_from_end([press], 4, self.RANGE[1] + 0.01)
        self.assertEqual((plain.aim_ms, plain.retimed), (-2.0, play_chart.RETIMED_RANGE))

    def test_a_squeeze_outs_late_gem_outside_joins_its_group(self):
        # The threshold is far above the read, so only the range joins it.
        group = self._press(aim_ms=-15.0, lanes=(Lane.RED,))
        late = self._press(aim_ms=20.0, lanes=(Lane.BLUE,),
                           fallback={"action": "join_group", "if_end_after_ms": 5000.0,
                                     "before_end_ms": 15.0, "join_gems": [0],
                                     "retimed": play_chart.RETIMED_INSIDE})
        (joined,) = play_chart.retime_from_end([group, late], 4, self.RANGE[0] - 0.01,
                                               {5: [Lane.RED, Lane.BLUE]})
        self.assertEqual(joined.lanes, [Lane.RED, Lane.BLUE])
        self.assertEqual(joined.at_ms, group.at_ms)
        self.assertEqual(joined.retimed, play_chart.RETIMED_RANGE)


class EndFieldsLoadTest(unittest.TestCase):
    """load_schedule refuses a press with an `end_read_after` that lacks a
    field of play_chart.END_FIELDS, naming it, so a file older than the
    fields is never played (task T3 of
    `docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md` (removed; `git show 48401572:docs/superpowers/plans/2026-11-13-schedule-reads-hydra-ratios.md`))."""

    def _doc(self):
        doc = _schedule_of(_DUMP)
        doc["presses"][1].update({"end_read_after": 38, "aim_end_ms": 13000.0,
                                  "own_end_ms": 13010.0, "read_stored_end_ms": 12990.0,
                                  "own_stored_end_ms": 13005.0,
                                  "conversion": {"early": 0.5, "late": 2.0},
                                  "read_range_ms": [12900.0, 13100.0]})
        return doc

    def test_the_fields_are_read(self):
        chords = _write_and_load(self._doc(), play_chart.load_schedule).chords
        press = next(p for c in chords for p in c.presses if p.index == 41)
        self.assertEqual((press.own_end_ms, press.read_stored_end_ms, press.own_stored_end_ms,
                          press.conversion, press.read_range_ms),
                         (13010.0, 12990.0, 13005.0, {"early": 0.5, "late": 2.0},
                          (12900.0, 13100.0)))
        # A press with no end_read_after carries none of them.
        plain = next(p for c in chords for p in c.presses if p.index == 38)
        self.assertIsNone(plain.own_end_ms)

    def test_a_null_range_is_read_as_no_range(self):
        doc = self._doc()
        doc["presses"][1]["read_range_ms"] = None
        chords = _write_and_load(doc, play_chart.load_schedule).chords
        press = next(p for c in chords for p in c.presses if p.index == 41)
        self.assertIsNone(press.read_range_ms)

    def test_each_missing_field_is_named(self):
        for key in play_chart.END_FIELDS:
            with self.subTest(key):
                doc = self._doc()
                del doc["presses"][1][key]
                with self.assertRaises(ValueError) as caught:
                    _write_and_load(doc, play_chart.load_schedule)
                self.assertIn(f"entry 1 of `presses` has no `{key}`", str(caught.exception))

    def test_a_null_number_and_a_one_sided_conversion_are_named(self):
        for key in ("own_end_ms", "read_stored_end_ms", "own_stored_end_ms", "conversion"):
            with self.subTest(key):
                doc = self._doc()
                doc["presses"][1][key] = None
                with self.assertRaises(ValueError) as caught:
                    _write_and_load(doc, play_chart.load_schedule)
                self.assertIn(f"null `{key}`", str(caught.exception))
        doc = self._doc()
        del doc["presses"][1]["conversion"]["late"]
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        self.assertIn("has no `late`", str(caught.exception))


class RetimedColumnTest(unittest.TestCase):
    """play_chords puts `retimed` on the rows of the presses retime_from_end
    marked, and on no other row."""

    def test_only_the_marked_rows_carry_it(self):
        chords = play_chart.load_schedule(_FREAK_SCHEDULE).chords
        late = _press_of(chords, 1144)
        result, _game = _play_schedule(
            chords, _reads(chords, {1062: late.fallback["if_end_after_ms"] + 0.01}))
        marked = {r["index"]: r["retimed"] for r in result.rows if "retimed" in r}
        self.assertEqual(marked, {387: play_chart.RETIMED_LATE, 1144: play_chart.RETIMED_INSIDE})

    def test_the_words_are_the_ones_the_schedule_writes(self):
        # Every "retimed" word a fallback in a hydra_replay schedule file
        # carries is one of play_chart's words.
        words = {play_chart.RETIMED_LATE, play_chart.RETIMED_INSIDE,
                 play_chart.RETIMED_PLAIN, play_chart.RETIMED_RANGE}
        written = set()
        for path in (_FREAK_SCHEDULE, _MOUTHS_SCHEDULE):
            for entry in _read_json(path)["presses"]:
                if entry.get("fallback") and "retimed" in entry["fallback"]:
                    written.add(entry["fallback"]["retimed"])
        self.assertTrue(written)
        self.assertLessEqual(written, words)


# --- what is missing stops the run (task F4 of
# docs/superpowers/plans/2026-11-08-player-audit-fixes.md) -------------------------

def _split_then_plain_chords():
    """Chord 7 split in two order groups as _split_chords has it, then chord
    8, a lone red a second later. Hand-picked test inputs."""
    doc = _schedule_of({"chords": [
        {"index": 7, "ms": 1000.0, "notes": [_gem("Kick", "none"),
                                             _gem("Green", "none", cymbal=True)]},
        {"index": 8, "ms": 2000.0, "notes": [_gem("Red", "none")]}]})
    first = doc["presses"][0]
    doc["presses"] = [dict(first, gems=[0]), dict(first, gems=[1], aim_ms=6.0, after_ms=8.0),
                      doc["presses"][1]]
    return _write_and_load(doc, play_chart.load_schedule).chords


class _ReplyPress:
    """A kit press whose agent sends `reply` to every press; it keeps each
    call's lanes."""

    def __init__(self, reply) -> None:
        self.reply, self.calls = reply, []

    def __call__(self, lanes, axes=None, then=()):
        self.calls.append(list(lanes))
        return self.reply


def _kit_run(chords, press, start_s=0.5):
    wall = _FakeTime()
    game = _FakeGame()
    out = io.StringIO()
    result = play_chart.play_chords(
        chords, _FakeSongClock(wall, start_s=start_s), game.read_score, press,
        now=wall.now, sleep=wall.sleep, out=out, kit=True)
    return result, out.getvalue()


class ReplyStopTest(unittest.TestCase):
    """A kit reply without what its press asked for stops the run at that
    chord (D318 item 2)."""

    def test_a_reply_without_groups_stops_the_run_at_that_chord(self):
        press = _ReplyPress({})
        result, text = _kit_run(_split_then_plain_chords(), press)
        self.assertEqual(result.stop, "reply")
        self.assertIn("chord 7", result.message)
        self.assertIn("`groups`", result.message)
        self.assertIn(result.message, text)
        # Chord 8 is never pressed and the stopped press logs no row.
        self.assertEqual(press.calls, [[Lane.KICK]])
        self.assertEqual(result.rows, [])
        # A first group's frames with no `groups` list stops it the same way.
        one = fakes.kit_reply([(500, 502, _SPLIT_DOWN_MS, _SPLIT_DOWN_MS + 6)])
        result, _text = _kit_run(_split_then_plain_chords(), _ReplyPress(one))
        self.assertEqual((result.stop, result.rows), ("reply", []))
        self.assertEqual(result.message, "the kit's reply to chord 7's press has no `groups` "
                                         "list, though the press sent 2 groups")

    def test_a_reply_that_is_not_a_dict_or_lacks_down_up_stops_the_run(self):
        whole = fakes.kit_reply([(500, 502, _SPLIT_DOWN_MS, _SPLIT_DOWN_MS + 6)])
        cases = {
            "None": None,
            "a list": [Lane.RED],
            "no down_ms or up_ms": {k: v for k, v in whole.items() if k not in ("down_ms", "up_ms")},
            "no down_frame": {k: v for k, v in whole.items() if k != "down_frame"},
        }
        for what, reply in cases.items():
            with self.subTest(what):
                press = _ReplyPress(reply)
                result, _text = _kit_run(_load_chords()[:2], press, start_s=12.0)
                self.assertEqual(result.stop, "reply")
                self.assertTrue(result.message.startswith("the kit's reply to chord 38's press "),
                                result.message)
                self.assertEqual(len(press.calls), 1)
        self.assertEqual(
            _kit_run(_load_chords()[:2], _ReplyPress(cases["no down_ms or up_ms"]),
                     start_s=12.0)[0].message,
            "the kit's reply to chord 38's press has no down_ms, up_ms")
        # The whole reply plays on.
        result, _text = _kit_run(_load_chords()[:2], _ReplyPress(whole), start_s=12.0)
        self.assertEqual((result.stop, len(result.rows)), ("done", 2))

    def test_a_keys_press_reply_is_not_read(self):
        # The keys route returns the keys it pressed; no kit check applies.
        result, _game, _wall = _run({"start_s": 12.0})
        self.assertEqual(result.stop, "done")


class LoadRefusalTest(unittest.TestCase):
    """load_schedule refuses a file without what the play reads (D318 item
    2, D321 item 3, D323 item 2)."""

    def _refusal(self, doc):
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        return str(caught.exception)

    def test_load_schedule_refuses_a_missing_key(self):
        # The plan names a chord's `notes`; a schedule chord holds its gems in
        # `gems` (schedule_json in tools/replay_schedule.cpp), so that is the
        # key refused here.
        for key in ("song_length_ms", "chords"):
            with self.subTest(key):
                doc = _schedule_of(_DUMP)
                del doc[key]
                self.assertIn(f"the file has no `{key}`", self._refusal(doc))
        for key in ("tick", "gems"):
            with self.subTest(key):
                doc = _schedule_of(_DUMP)
                del doc["chords"][1][key]
                self.assertIn(f"entry 1 of `chords` has no `{key}`", self._refusal(doc))
        doc = _schedule_of(_DUMP)
        doc["song_length_ms"] = None
        self.assertIn("song_length_ms is null", self._refusal(doc))

    def test_a_chord_with_no_notes_is_refused(self):
        doc = _schedule_of(_DUMP)
        doc["chords"][2]["gems"] = []
        self.assertIn("chord 45 has no gems", self._refusal(doc))

    def test_an_empty_chord_list_is_refused_in_ch_play_charts_words(self):
        doc = _schedule_of({"chords": []})
        self.assertTrue(self._refusal(doc).endswith("schedule.json has no chords to play"))

    def test_a_press_naming_a_missing_chord_or_gem_is_refused(self):
        doc = _schedule_of(_DUMP)
        doc["presses"][0]["chord"] = 9999
        self.assertIn("names chord 9999", self._refusal(doc))
        doc = _schedule_of(_DUMP)
        doc["presses"][0]["gems"] = [3]
        self.assertIn("names gems [3] of chord 38, which holds 1", self._refusal(doc))


class MainExitTest(unittest.TestCase):
    """main exits 0 with "Done." only when the run reached the song's end,
    and 1 with the reason for every other stop (D319 item 1, D323 item 2)."""

    def _main(self, play, doc=None):
        """main on `doc` (_schedule_of(_DUMP) when None), with live_play
        replaced by `play(chords, out)`, a real play_chords run on fakes."""
        from unittest import mock
        out = io.StringIO()
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "schedule.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump(_schedule_of(_DUMP) if doc is None else doc, f)
            with mock.patch.object(play_chart, "live_play",
                                   lambda chords, **kw: play(chords, kw["out"])):
                code = play_chart.main([path], out=out)
        return code, out.getvalue()

    @staticmethod
    def _play(clock_of, press=None, **kwargs):
        def play(chords, out):
            wall = _FakeTime()
            game = _FakeGame()
            return play_chart.play_chords(
                chords, clock_of(wall), game.read_score, press or game.press,
                now=wall.now, sleep=wall.sleep, out=out, **kwargs)
        return play

    def test_main_exits_nonzero_on_every_stop_but_the_song_end(self):
        def at(start_s, **kw):
            return lambda wall: _FakeSongClock(wall, start_s=start_s, **kw)

        stops = {
            # RunCapReached: a 1 s cap before the first chord at 13.04 s.
            "cap": (self._play(at(12.0), cap_s=1), "run cap of 1 s reached"),
            # StopAsked.
            "stopped": (self._play(at(12.0), should_stop=lambda: True), "asked to stop"),
            # ClockFrozen: the clock holds at 20 s, between chords 45 and 91.
            "frozen": (self._play(at(12.0, freeze_at_s=20.0)), "clock frozen at 20.00 s"),
            # ClockJumpedBack: the clock falls from 13.05 s to 11.95 s and the
            # re-sync reads it past the last chord.
            "past_end": (self._play(lambda wall: _ScriptedSongClock(
                wall, [12.0, 13.05, 11.95, 200.0])),
                "song clock jumped back (13.05 -> 11.95 s), and the clock at 200.00s "
                "is past the last note"),
            # The reply stop.
            "reply": (self._play(at(12.0), press=_ReplyPress({}), kit=True),
                      "the kit's reply to chord 38's press has no down_frame"),
        }
        for stop, (play, reason) in stops.items():
            with self.subTest(stop):
                code, text = self._main(play)
                self.assertEqual(code, 1, text)
                self.assertIn(f"Stopped before the song's end ({stop}): ", text)
                self.assertIn(reason, text)
                self.assertNotIn("Done.", text)
        # The empty list never reaches the run.
        code, text = self._main(self._play(at(12.0)), _schedule_of({"chords": []}))
        self.assertEqual(code, 1)
        self.assertIn("has no chords to play", text)
        self.assertNotIn("Done.", text)
        # The song end.
        code, text = self._main(self._play(at(12.0)))
        self.assertEqual(code, 0, text)
        self.assertTrue(text.endswith("Done.\n"), text)
        self.assertNotIn("Stopped", text)

    def test_main_prints_the_notes_of_a_run_that_ends_done(self):
        note = "a retime was refused after the play ended"
        code, text = self._main(lambda chords, out: play_chart.RunResult(
            stop=play_chart.STOP_DONE, message=note))
        self.assertEqual(code, 0, text)
        self.assertIn(note, text)
        self.assertTrue(text.endswith("Done.\n"), text)


class StartLineTest(unittest.TestCase):
    """The start line counts the chords the run passes over where it starts
    (D321 item 3, L13)."""

    def _text(self, clock_of):
        wall = _FakeTime()
        game = _FakeGame()
        out = io.StringIO()
        result = play_chart.play_chords(
            _load_chords(), clock_of(wall), game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, out=out)
        return result, out.getvalue().splitlines()

    # Pinned from one run of _DUMP's schedule: chord 38 fires at 13,041.48 ms,
    # 91.48 ms after a clock read of 12.95 s.
    SKIP_ONE = ("  Song clock at 12.95s; starting at press 2/8 (t=13.61s); skipped 1 chord "
                "less than 150 ms ahead of the clock.")
    SKIP_NONE = ("  Song clock at 12.00s; starting at press 1/8 (t=13.04s); skipped 0 chords "
                 "less than 150 ms ahead of the clock.")

    def test_the_start_line_counts_skipped_chords(self):
        result, lines = self._text(lambda wall: _FakeSongClock(wall, start_s=12.95))
        self.assertEqual(lines[0], self.SKIP_ONE)
        self.assertEqual(result.skipped, 1)
        result, lines = self._text(lambda wall: _FakeSongClock(wall, start_s=12.0))
        self.assertEqual(lines[0], self.SKIP_NONE)
        self.assertEqual(result.skipped, 0)
        # The restart form: chord 38 is pressed from 12 s, the clock falls
        # back to 11.95 s and the re-sync reads 12.95 s, so chord 38 is
        # passed over again.
        result, lines = self._text(lambda wall: _ScriptedSongClock(
            wall, [12.0, 13.05, 11.95, 12.95]))
        self.assertEqual(lines[0], self.SKIP_NONE)
        self.assertIn(self.SKIP_ONE, lines)
        self.assertEqual(result.skipped, 1)

    def test_a_clock_past_the_last_note_counts_every_chord(self):
        result, lines = self._text(lambda wall: _FakeSongClock(wall, start_s=200.0))
        self.assertEqual(result.stop, "past_end")
        self.assertEqual(lines[0], "  Song clock at 200.00s is past the last note; nothing to "
                                   "play (skipped 8 chords).")
        self.assertEqual(result.skipped, 8)


class FakeReplyShapeTest(unittest.TestCase):
    """The kit press fakes send the reply kit.ts sends (D318 item 2, L65)."""

    @staticmethod
    def _kit_ts_reply_keys():
        """The keys of kit.ts's press reply literal, and of its `groups`
        entries, read from the source."""
        path = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", "kit.ts")
        with open(path, encoding="utf-8") as f:
            text = f.read()
        start = text.index("const reply: {")

        def keys(opening, closing):
            # An object literal's keys, `key: value` or the shorthand `key`.
            begin = text.index(opening, start) + len(opening)
            body = text[begin:text.index(closing, begin)]
            return {part.split(":")[0].strip() for part in body.split(",") if part.strip()}

        return keys("} = {", "};"), keys("groups.map((g) => ({", "}));")

    def test_fakes_send_the_real_reply_shape(self):
        from tools.ch_mcp.kit_tools import reply_gap
        top, group = self._kit_ts_reply_keys()
        self.assertTrue(set(overlay_logs.KIT_PRESS_COLUMNS) <= top & group)
        press = _SplitReplyPress()
        replies = [(press([Lane.KICK]), 1),
                   (press([Lane.KICK], None, [([Lane.GREEN_CYMBAL], None, 8.0)]), 2),
                   (fakes.kit_reply([(1, 3, 10, 20)] * 3), 3)]
        for reply, groups in replies:
            with self.subTest(groups=groups):
                self.assertIsNone(reply_gap(reply, groups))
                self.assertEqual(set(reply) - {"groups"}, top)
                self.assertEqual("groups" in reply, groups > 1)
                for entry in reply.get("groups", []):
                    self.assertEqual(set(entry), group)


# The fake stall of StepTimesTest, and the most any other step of the same
# row may show beside it: PS3's acceptance line in
# docs/superpowers/plans/2026-11-12-player-stall-fixes.md.
_STALL_S = 0.3
_STALL_MS = 300.0
_QUIET_MS = 20.0


class _StallOnce:
    """`call`, whose `nth` call (1-based) first moves `wall` on by
    _STALL_S; every other call passes straight through."""

    def __init__(self, wall: _FakeTime, call, nth: int = 1) -> None:
        self.wall, self.call, self.nth = wall, call, nth
        self.calls = 0

    def __call__(self, *args, **kwargs):
        self.calls += 1
        if self.calls == self.nth:
            self.wall.t += _STALL_S
        return self.call(*args, **kwargs)


class _ClockOf:
    """A song clock whose read is `read`."""

    def __init__(self, read) -> None:
        self.read = read


# Which part of StepTimesTest's run each timing column times.
_PART_OF_COLUMN = {
    "wait_read_max_ms": "clock",
    "wait_oversleep_max_ms": "sleep",
    "press_took_ms": "press",
    "reply_check_took_ms": "reply_gap",
    "end_read_took_ms": "sp_end",
    "score_read_took_ms": "score",
    "prev_handoff_took_ms": "on_press",
}


class StepTimesTest(unittest.TestCase):
    """The first row of each message says how long each step took, so a
    stall names its step (PS3 of
    docs/superpowers/plans/2026-11-12-player-stall-fixes.md)."""

    def _run(self, stalled_part):
        """A kit run of the Freak fixture on the fakes, with every part of
        a message the player times, and `stalled_part` stalling once."""
        from tools.ch_mcp import kit_tools
        chords = play_chart.load_schedule(_FREAK_SCHEDULE).chords
        wall = _FakeTime()
        song = _FakeSongClock(wall, start_s=chords[0].ms / 1000 - 1.0)
        parts = {"clock": song.read, "sleep": wall.sleep, "press": _SplitReplyPress(),
                 "reply_gap": kit_tools.reply_gap, "sp_end": _reads(chords, {}),
                 "score": lambda: 0, "on_press": lambda row: None}
        # The run's start reads the song clock and the score once before the
        # first message, so the first message's own reads are the second.
        nth = {"clock": 2, "score": 2}.get(stalled_part, 1)
        parts[stalled_part] = _StallOnce(wall, parts[stalled_part], nth)
        with mock.patch.object(kit_tools, "reply_gap", parts["reply_gap"]):
            return play_chart.play_chords(
                chords, _ClockOf(parts["clock"]), parts["score"], parts["press"],
                now=wall.now, sleep=parts["sleep"], out=io.StringIO(), cap_s=600.0,
                read_sp_end=parts["sp_end"], on_press=parts["on_press"], kit=True)

    def test_a_stall_in_each_step_shows_in_that_steps_column(self):
        self.assertEqual(set(_PART_OF_COLUMN), set(overlay_logs.TIMING_PRESS_COLUMNS))
        for column in overlay_logs.TIMING_PRESS_COLUMNS:
            with self.subTest(column=column):
                result = self._run(_PART_OF_COLUMN[column])
                self.assertEqual(result.stop, "done")
                # The end read and the hand-off show on a later message's row.
                row = next(r for r in result.rows if column in r)
                # isclose's own default tolerance: the fake times are float sums.
                self.assertTrue(row[column] >= _STALL_MS or math.isclose(row[column], _STALL_MS),
                                row[column])
                for other in overlay_logs.TIMING_PRESS_COLUMNS:
                    if other != column:
                        self.assertLess(row.get(other, 0.0), _QUIET_MS, other)

    def test_a_dropped_row_carries_its_wait_times(self):
        wall = _FakeTime()
        game = _FakeGame()
        song = _FakeSongClock(wall, start_s=12.0)
        # After chord 38's hand-off, the next clock read takes 2 s, which is
        # past the last send of chords 41 and 45 (13614 and 14185 ms).
        jump_s = []

        def read():
            if jump_s:
                wall.t += jump_s.pop()
            return song.read()

        def on_press(row):
            if row["index"] == 38:
                jump_s.append(2.0)

        result = play_chart.play_chords(
            _load_chords(), _ClockOf(read), game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), on_press=on_press)
        (row,) = _rows_of(result, 41)
        self.assertEqual(row["send"], play_chart.DROPPED)
        self.assertGreaterEqual(row["wait_read_max_ms"], 2000.0)
        for column in ("wait_oversleep_max_ms", "prev_handoff_took_ms", "score_read_took_ms"):
            self.assertIn(column, row)
        self.assertNotIn("press_took_ms", row)
        self.assertNotIn("reply_check_took_ms", row)

    def test_only_the_first_row_of_a_split_chord_carries_the_times(self):
        wall = _FakeTime()
        result = play_chart.play_chords(
            _split_chords(), _FakeSongClock(wall, start_s=0.5), lambda: 0, _SplitReplyPress(),
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), kit=True,
            on_press=lambda row: None)
        first, second = result.rows
        timing = set(overlay_logs.TIMING_PRESS_COLUMNS)
        # The run's one message reads no end and follows no hand-off.
        self.assertEqual(timing & set(first),
                         timing - {"end_read_took_ms", "prev_handoff_took_ms"})
        self.assertEqual(timing & set(second), set())

    def test_the_timing_columns_reach_the_press_log_on_the_first_row_only(self):
        wall = _FakeTime()
        result = play_chart.play_chords(
            _split_chords(), _FakeSongClock(wall, start_s=0.5), lambda: 0, _SplitReplyPress(),
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), kit=True)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                first, second = list(csv.DictReader(f))
        self.assertNotEqual(first["press_took_ms"], "")
        self.assertEqual(first["end_read_took_ms"], "")
        self.assertEqual([second[c] for c in overlay_logs.TIMING_PRESS_COLUMNS],
                         [""] * len(overlay_logs.TIMING_PRESS_COLUMNS))


if __name__ == "__main__":
    unittest.main()
