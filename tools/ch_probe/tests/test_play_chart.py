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
import os
import statistics
import sys
import tempfile
import time
import unittest

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
    hand-picked last send 50 ms after the chord. These are test inputs, not
    the schedule's rules; tools/replay_schedule.cpp owns those."""
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
    return {"song_length_ms": None, "activations": [], "chords": chords,
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


_FREAK_SCHEDULE = os.path.join(_HERE, "freak_limits_schedule.json")


def _read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


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
        schedule = play_chart.load_schedule(_FREAK_SCHEDULE)
        self.assertEqual(schedule.song_length_ms, 193934.0)
        self.assertEqual(schedule.activations[0].sp_ready_ms, 17314.681543154737)
        self.assertEqual(schedule.activations[0].read_end_after, 263)

    def test_press_plan_returns_the_presses_in_firing_order(self):
        chords = play_chart.load_schedule(_FREAK_SCHEDULE).chords
        plan = play_chart.press_plan(chords)
        self.assertEqual(sorted(plan, key=lambda p: p.at_ms), plan)
        self.assertEqual(len(plan), 1609)

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
        self.assertIn("chord 91", str(caught.exception))
        self.assertIn("cannot make", str(caught.exception))

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
        from tools.ch_probe.experiments import walk_edges
        self.assertIs(play_chart.live, live)
        self.assertIs(play_chart.live.wait_until, live.wait_until)
        self.assertIs(play_chart.live.first_note_index, live.first_note_index)
        self.assertIs(play_chart.SongClock, walk_edges.SongClock)


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
        self.assertEqual((result.sent, result.hit, result.max_streak), (8, 7, 5))
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
        self.assertEqual([(r[col["score_before"]], r[col["score_after"]], r[col["hit"]])
                          for r in body], [
            ("0", "100", "1"), ("100", "200", "1"), ("200", "200", "0"),
            ("200", "300", "1"), ("300", "400", "1"), ("400", "500", "1"),
            ("500", "600", "1"), ("600", "700", "1")])
        # The clock the wait returned: inside _schedule_of's 2 ms aim, before
        # the chord.
        for r in body:
            early_ms = float(r[col["chart_ms"]]) - float(r[col["clock_ms_at_press"]])
            self.assertGreater(early_ms, 0.0)
            self.assertLessEqual(early_ms, 2.0)


class RunStopsTest(unittest.TestCase):
    def test_run_cap_is_240_s(self):
        self.assertEqual(play_chart.RUN_CAP_S, 240.0)

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
        return code, out.getvalue(), proc, opens, log_written

    def test_no_song_exits_2_with_the_finder_message(self):
        code, text, proc, _opens, log_written = self._main_with_no_song()
        self.assertEqual(code, 2)
        self.assertIn("no playing song found in 120 s; start the song and run again",
                      text)
        self.assertTrue(proc.closed)
        self.assertFalse(log_written)

    def test_the_game_is_opened_read_only(self):
        _code, _text, proc, opens, _log = self._main_with_no_song()
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
        self.assertEqual(play_chart.STOP_NO_ENGINE, "no engine")
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
        self.assertIn("....", out.getvalue())

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

    def test_a_given_press_is_used_and_no_window_is_looked_up(self):
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

        seen = []

        def fake_play_chords(chords, clock, read_score, press, **kwargs):
            seen.append(press)
            return play_chart.RunResult(stop="done")

        def no_window():
            raise AssertionError("find_game_window was called on the press route")

        def kit_press(lanes, _axes):
            return {}

        with mock.patch.object(play_chart, "EngineModel", _FakeEngine), \
                mock.patch.object(play_chart, "find_game_window", no_window), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load_chords(), open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, press=kit_press, out=io.StringIO())
        self.assertEqual(seen, [kit_press])


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
    """Each gem keeps its dynamic, and live_play reads the threshold for the
    ghosts it must press."""

    def setUp(self):
        _ThresholdEngine.reads = 0

    def _live_play(self, chords, threshold):
        from unittest import mock
        _ThresholdEngine.threshold = threshold
        seen, proc, out = [], _FakeProc(), io.StringIO()

        def fake_play_chords(chords, clock, read_score, press, **kwargs):
            seen.extend(chords)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _ThresholdEngine), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            try:
                result = play_chart.live_play(
                    chords, open_proc=lambda **kw: proc, find_engine=lambda _proc: 0x1000,
                    press=lambda lanes, axes: {}, out=out)
            finally:
                self.assertTrue(proc.closed)
        return result, seen, out.getvalue()

    def test_chords_carry_each_gems_dynamic_beside_its_lane(self):
        chords = _load_dump(_DYNAMICS_DUMP)
        self.assertEqual(chords[0].lanes, [Lane.RED, Lane.YELLOW_CYMBAL, Lane.KICK])
        self.assertEqual(chords[0].dynamics, ["ghost", "accent", "none"])
        self.assertEqual(chords[1].dynamics, ["none"])

    def test_a_ghost_chart_reads_the_threshold_once_and_prints_it(self):
        from tools.ch_mcp.kit_tools import pad_axis
        result, seen, text = self._live_play(_load_dump(_DYNAMICS_DUMP), 64)
        self.assertEqual(_ThresholdEngine.reads, 1)
        self.assertEqual(result.dyn_threshold, 64)
        self.assertIn("  Dynamics threshold: 64\n", text)
        self.assertEqual(seen[0].presses[0].axes, [pad_axis("ghost", 64),
                                                   pad_axis("accent", 64), pad_axis("none", 64)])
        self.assertEqual(seen[1].presses[0].axes, [pad_axis("none", 64)])

    def test_a_ghost_chart_with_a_threshold_of_0_raises_naming_the_field(self):
        with self.assertRaises(ValueError) as caught:
            self._live_play(_load_dump(_DYNAMICS_DUMP), 0)
        self.assertIn("OFF_DYN_THRESHOLD", str(caught.exception))

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
        result, seen, text = self._live_play(_load_chords(), 0)
        self.assertEqual(result.stop, "done")
        self.assertEqual(_ThresholdEngine.reads, 0)
        self.assertIsNone(result.dyn_threshold)
        self.assertNotIn("threshold", text)
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
    """A press that returns a dict (the kit press) adds the values the press
    log has columns for to the row; a press that returns keys adds none."""

    def test_a_dict_reply_rides_into_the_row(self):
        wall = _FakeTime()
        game = _FakeGame()

        def press(lanes, _axes):
            game.press(lanes)
            return {"down_frame": 40, "up_frame": 42, "down_ms": 7, "up_ms": 19,
                    "rewrite": True, "hit": "not this"}

        result = play_chart.play_chords(
            _load_chords()[:1], _FakeSongClock(wall, start_s=12.0),
            game.read_score, press, now=wall.now, sleep=wall.sleep, out=io.StringIO())
        row = result.rows[0]
        self.assertEqual(row, {
            "index": 38, "chart_ms": 13043.48, "clock_ms_at_press": row["clock_ms_at_press"],
            "lead_ms": 2.0, "score_before": 0, "score_after": 100, "hit": 1,
            "scored_between": 0,
            "down_frame": 40, "up_frame": 42, "down_ms": 7, "up_ms": 19,
            "wall_s_at_read": row["wall_s_at_read"], "wall_s_at_reply": row["wall_s_at_reply"]})

    def test_a_key_press_adds_nothing(self):
        result, _game, _wall = _run({"start_s": 12.0})
        self.assertEqual(set(result.rows[0]), {
            "index", "chart_ms", "clock_ms_at_press", "lead_ms", "score_before",
            "score_after", "hit", "scored_between", "wall_s_at_read", "wall_s_at_reply"})


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


class AppendPressRowTest(unittest.TestCase):
    """append_press_row keeps the log on disk row by row, as a run makes it."""

    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.path = os.path.join(folder.name, "press.csv")

    def test_the_file_grows_with_each_press_of_a_run(self):
        seen = []

        def on_press(row):
            play_chart.append_press_row(self.path, row)
            seen.append(len(_table(self.path)) - 1)

        fake = _FakeTime()
        game = _FakeGame()
        result = play_chart.play_chords(
            _load_chords()[:3], _FakeSongClock(fake, start_s=12.0),
            game.read_score, game.press, now=fake.now, sleep=fake.sleep,
            on_press=on_press, out=io.StringIO())
        self.assertEqual(result.stop, "done")
        self.assertEqual(seen, [1, 2, 3])
        table = _table(self.path)
        self.assertEqual(tuple(table[0]), overlay_logs.PRESS_LOG_COLUMNS)
        self.assertEqual([r[0] for r in table[1:]], ["38", "41", "45"])

    def test_a_file_with_its_header_gets_no_second_one(self):
        play_chart.write_press_log(self.path, [])
        play_chart.append_press_row(self.path, {"index": 7})
        self.assertEqual([r[0] for r in _table(self.path)], ["index", "7"])

    def test_it_writes_what_write_press_log_writes(self):
        rows = [{"index": 1, "lead_ms": 2.0, "hit": 1}, {"index": 2, "hit": 0}]
        for row in rows:
            play_chart.append_press_row(self.path, row)
        whole = self.path + ".whole"
        play_chart.write_press_log(whole, rows)
        self.assertEqual(_table(self.path), _table(whole))

    def test_an_append_costs_less_than_the_score_read_sleep(self):
        # The median, so one scan of the file by another process fails nothing.
        row = {column: 1 for column in overlay_logs.PRESS_LOG_COLUMNS}
        took = []
        for _ in range(100):
            began = time.perf_counter()
            play_chart.append_press_row(self.path, row)
            took.append(time.perf_counter() - began)
        self.assertLess(statistics.median(took), play_chart.SCORE_READ_DELAY_S)


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
        groups = [{"down_frame": 500 + 3 * k, "up_frame": 502 + 3 * k,
                   "down_ms": 9000 + 9 * k, "up_ms": 9006 + 9 * k} for k in range(1 + len(then))]
        reply = {**groups[0], "hold_frames": 2}
        if then:
            reply["groups"] = groups
        return reply


class SplitSendTest(unittest.TestCase):
    """play_chords sends a split chord as one message and writes a row per
    group (design points 4 and 9 of the player-limits plan)."""

    def _run(self, chords, scores, press):
        wall = _FakeTime()
        reads = iter(scores)
        result = play_chart.play_chords(
            chords, _FakeSongClock(wall, start_s=0.5), lambda: next(reads), press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO())
        self.assertEqual(result.stop, "done")
        return result

    def test_a_two_group_reply_gives_two_rows_with_their_own_frames_and_one_hit(self):
        press = _SplitReplyPress()
        # The start read, the fresh read before the press, the read after it.
        result = self._run(_split_chords(), [0, 0, 250], press)
        self.assertEqual(press.calls, [([Lane.KICK], [([Lane.GREEN_CYMBAL], None, 8.0)])])
        rows = result.rows
        self.assertEqual([(r["index"], r["down_frame"], r["up_frame"], r["down_ms"], r["up_ms"])
                          for r in rows], [(7, 500, 502, 9000, 9006), (7, 503, 505, 9009, 9015)])
        self.assertEqual([(r["score_before"], r["score_after"], r["hit"]) for r in rows],
                         [(0, 250, 1), (0, 250, 1)])
        self.assertEqual([round(r["lead_ms"], 2) for r in rows], [2.0, -6.0])
        self.assertEqual(rows[0]["clock_ms_at_press"], rows[1]["clock_ms_at_press"])
        self.assertEqual(rows[0]["scored_between"], 0)
        self.assertNotIn("scored_between", rows[1])
        self.assertEqual((result.sent, result.hit), (1, 1))

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
        self.assertEqual(result.rows[1]["hit"], 1)

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
        # Each press timed from a read fires 3 ms later: the file's aims of
        # 24.27, -15, 39.55, -15 and -15 ms become leads of -27.27, 12, -42.55,
        # 12 and 12 ms.
        self.assertEqual([[round(r["lead_ms"], 2) for r in _rows_of(result, index)]
                          for index in (387, 750, 1144, 1369, 1573)],
                         [[-27.27], [12.0], [-42.55], [12.0], [12.0]])
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
                aim_end_ms=None if read_after is None else 1000.0)
        plan = [press(1, 990.0, 7), press(2, 991.0), press(3, 1200.0, 8)]
        moved = play_chart.retime_from_end(plan, 7, 1000.0, 1002.0)
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
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load_chords(), open_proc=lambda **kw: _FakeProc(),
                find_engine=lambda _proc: 0x1000, press=lambda lanes, axes: {},
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

    def test_the_late_chord_is_dropped_and_the_next_is_pressed_and_hit(self):
        chords, result, game = self._run()
        self.assertEqual(result.stop, "done")
        (row,) = _rows_of(result, 41)
        late_ms = row["clock_ms_at_press"] - row["chart_ms"]
        self.assertGreater(late_ms, 100.0)
        self.assertLess(late_ms, 110.0)
        # The precondition: the read is past the press's last send.
        self.assertGreater(row["clock_ms_at_press"], chords[1].presses[0].last_send_ms)
        self.assertEqual(row["hit"], "dropped")
        self.assertEqual(row["score_before"], row["score_after"])
        for column in ("down_frame", "up_frame", "down_ms", "up_ms", "end_read_ms"):
            self.assertNotIn(column, row)
        # Chord 41 is the lone kick: every press but its own was sent.
        expected = [list(lanes) for ms, lanes in _load() if ms != chords[1].ms]
        self.assertEqual(game.presses, expected)
        self.assertEqual(_rows_of(result, 45)[0]["hit"], 1)

    def test_the_counts_leave_the_drop_out_and_the_streak_starts_over(self):
        _chords, result, _game = self._run()
        self.assertEqual((result.sent, result.hit, result.dropped, result.declined),
                         (7, 7, 1, 0))
        # 38 hits, 41 is dropped, the six after it hit: a streak that did not
        # start over would reach 7.
        self.assertEqual(result.max_streak, 6)

    def test_the_printed_line_says_dropped(self):
        chords = _load_chords()
        wall = _FakeTime()
        game = _FakeGame()
        out = io.StringIO()
        play_chart.play_chords(
            chords, _JumpingSongClock(wall, 12.0, 13.590, 0.131), game.read_score,
            game.press, now=wall.now, sleep=wall.sleep, out=out)
        self.assertIn("dropped", out.getvalue())


if __name__ == "__main__":
    unittest.main()
