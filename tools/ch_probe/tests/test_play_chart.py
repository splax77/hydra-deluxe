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

No test opens the game or sends a key. The run-cap, caller-hook,
scored-between and dropped-press tests, among others, play the Freak fixture
on the song clock and score a real run recorded, cut into
testdata/ch_runs/ls-f1-run1, on the real timer. Other run tests still use
the fakes below.
"""

from __future__ import annotations

import bisect
import csv
import inspect
import io
import json
import math
import os
import re
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
        # The real Freak file loads; the same file with refusals in it does
        # not, and the error names the file and keeps every refusal, not only
        # the first.
        doc = _read_json(_FREAK_SCHEDULE)
        self.assertEqual(doc["refusals"], [])
        play_chart.load_schedule(_FREAK_SCHEDULE)
        doc["refusals"] = ["no chord at skipped fill A", "no chord at skipped fill B"]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "refused_schedule.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump(doc, f)
            with self.assertRaises(ValueError) as caught:
                play_chart.load_schedule(path)
        message = str(caught.exception)
        self.assertIn(path, message)
        for refusal in doc["refusals"]:
            self.assertIn(refusal, message)

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
        # The schedule file's `constants` block is the list of timing numbers
        # tools/replay_schedule.cpp owns. play_chart defines none of them
        # again, by name or by value.
        constants = _read_json(_FREAK_SCHEDULE)["constants"]
        self.assertTrue(constants)
        numeric = {name: value for name, value in vars(play_chart).items()
                   if name.lstrip("_").isupper()
                   if isinstance(value, (int, float)) and not isinstance(value, bool)}
        for key, value in constants.items():
            self.assertNotIn(key.upper(), {n.lstrip("_") for n in numeric}, key)
            for name, own in numeric.items():
                self.assertNotEqual(own, value, f"{name} is {key}'s {value}")

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


# --- a run on a recorded game clock and the real timer ------------------------

# Clock windows cut from the Star Power watcher log (sp_watch-1.csv) of the
# real run hydra-data/ch-mcp-runs/2026-11-07/ls-f1-run1, which played
# freak_limits_schedule.json in Clone Hero. Only the wall time, song clock and
# score columns are kept.
_F1_RUN = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-f1-run1")
_F1_BEFORE_FIRST_CHORD = os.path.join(_F1_RUN, "sp_watch-1-clock-2.9-4.3.csv")
_F1_MID_SONG = os.path.join(_F1_RUN, "sp_watch-1-clock-20.0-22.6.csv")
# The same log's stretch around chord 134, a declined chord that keeps no gem.
_F1_DECLINED = os.path.join(_F1_RUN, "sp_watch-1-clock-30.0-31.6.csv")
# The first 30 kit press replies of the real run
# hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1 (kit-1.jsonl), as kit.ts sent them.
_E_REPLIES = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-e-run1",
                          "kit-1-press-replies.jsonl")


class _RecordedGame:
    """The song clock and score one real run recorded, read back at the
    recording's own pace on the real timer: a read gives the row the watcher
    logged as much wall time after its first row as has passed since this
    object's first read. A read past the last row fails the test at once,
    since the recording cannot say what the game did next. A recording that
    keeps the active byte and stored end columns also gives the Star Power
    end, as _recorded_end gives it for a row."""

    def __init__(self, path: str) -> None:
        with open(path, encoding="utf-8", newline="") as f:
            recorded = list(csv.DictReader(f))
        self._rows = [(float(r["wall_s"]), float(r["clock_s"]), int(r["score"]))
                      for r in recorded]
        self._ends = ([_recorded_end(r) for r in recorded]
                      if recorded and "sp_end_s" in recorded[0] else None)
        self._walls = [row[0] for row in self._rows]
        self._name = os.path.basename(path)
        self._t0 = None

    @property
    def last_clock_s(self) -> float:
        return self._rows[-1][1]

    def _at(self) -> int:
        now = time.perf_counter()
        if self._t0 is None:
            self._t0 = now
        at = self._walls[0] + (now - self._t0)
        if at > self._walls[-1]:
            raise AssertionError(f"the run read past the end of {self._name}")
        return bisect.bisect_right(self._walls, at) - 1

    def _row(self):
        return self._rows[self._at()]

    def read(self):
        clock_s = self._row()[1]
        return clock_s, clock_s

    def read_score(self) -> int:
        return self._row()[2]

    def read_sp_end(self):
        if self._ends is None:
            raise AssertionError(f"{self._name} keeps no Star Power columns")
        return self._ends[self._at()]


def _recorded_end(row):
    """What engine.sp_end reads from the game in the frame a watcher row
    logged: None while the active byte is clear, else the stored end in song
    seconds. The watcher logs both fields from the same engine offsets
    (engine.py's snapshot), and sp_compare.is_active owns which rows have
    the flag set."""
    from tools.ch_probe.experiments import sp_compare
    if not sp_compare.is_active({"sp_active_byte": int(row["sp_active_byte"])}):
        return None
    return float(row["sp_end_s"])


def _recorded_end_ms(path, chart_ms):
    """The game's end, in ms (None with no activation), that the recording at
    `path` holds once its song clock reaches `chart_ms`. It fails at once
    unless every row from there to the recording's end holds the same
    answer, so the moment of the read after the chord does not change it."""
    from tools.ch_probe import constants as C
    with open(path, encoding="utf-8", newline="") as f:
        ends = {_recorded_end(r) for r in csv.DictReader(f)
                if C.s_to_ms(float(r["clock_s"])) >= chart_ms}
    if len(ends) != 1:
        raise AssertionError(f"{os.path.basename(path)} holds {sorted(map(str, ends))} "
                             f"after {chart_ms:g} ms, not one end")
    (end_s,) = ends
    return None if end_s is None else C.s_to_ms(end_s)


def _freak_chords():
    return play_chart.load_schedule(_FREAK_SCHEDULE).chords


def _plan_parts(chords):
    """(index, chart_ms) of every press in the order play_chords sends them."""
    return [(p.index, p.chart_ms)
            for head, later in play_chart.press_messages(play_chart.press_plan(chords))
            for p in (head, *later)]


def _play_recorded(path, sent=None, chords=None, press=None, read_end=False, **kwargs):
    """play_chords over `chords` (the Freak schedule when None) on the
    recording at `path`, with the real timer. The default `press` only notes
    each chord's keys in `sent`, since no key may reach a window from a
    test. With `read_end`, the run reads the game's end from the recording
    too."""
    game = _RecordedGame(path)
    if read_end:
        kwargs["read_sp_end"] = game.read_sp_end
    sent = [] if sent is None else sent
    chords = _freak_chords() if chords is None else chords
    if press is None:
        def press(lanes, _axes=None, then=()):
            sent.append(list(lanes))
    out = io.StringIO()
    began = time.perf_counter()
    result = play_chart.play_chords(
        chords, game, game.read_score, press, out=out, **kwargs)
    return result, sent, out.getvalue(), time.perf_counter() - began, chords


def _pressed_rows(result):
    """The rows of a run's messages that were sent: a message the run
    dropped calls no press, and a run on the real timer may drop one."""
    return [r for r in result.rows if r[play_chart.SEND_COLUMN] != play_chart.DROPPED]


class RunStopsTest(unittest.TestCase):
    def test_play_chords_caps_at_run_cap_s_unless_told(self):
        # What a cap does is the next test's; this is which cap a run gets.
        cap = inspect.signature(play_chart.play_chords).parameters["cap_s"]
        self.assertIs(cap.default, play_chart.RUN_CAP_S)

    def test_a_caller_given_cap_ends_the_run_there(self):
        # The whole recording comes before the Freak schedule's first chord,
        # so a 1 s cap ends the run before any press.
        result, sent, out, took_s, chords = _play_recorded(_F1_BEFORE_FIRST_CHORD, cap_s=1)
        from tools.ch_probe import constants as C
        self.assertGreater(min(c.ms for c in chords),
                           C.s_to_ms(_RecordedGame(_F1_BEFORE_FIRST_CHORD).last_clock_s))
        self.assertEqual(result.stop, "cap")
        self.assertEqual(result.rows, [])
        self.assertEqual(sent, [])
        self.assertGreaterEqual(took_s, 1.0)
        self.assertIn(result.message, out)


class _FakeProc:
    module_base = 0x10000000

    def __init__(self) -> None:
        self.verified = False
        self.closed = False

    def verify_targets(self) -> None:
        self.verified = True

    def close(self) -> None:
        self.closed = True


# --- the pieces a server calls ----------------------------------------------------

class CallerHooksTest(unittest.TestCase):
    """play_chords hands each row to `on_press` and asks `should_stop`, on
    the recorded mid-song clock with a 2 s cap, inside the 2.6 s the
    recording holds."""

    _CAP_S = 2.0

    def _run_with(self, **kwargs):
        return _play_recorded(_F1_MID_SONG, cap_s=self._CAP_S, **kwargs)

    def assert_a_run_of_the_plan(self, rows, chords):
        # The rows are the presses the plan sends, in its order, with none
        # left out between the first and the last.
        parts = _plan_parts(chords)
        got = [(r["index"], r["chart_ms"]) for r in rows]
        self.assertTrue(got)
        starts = [k for k, part in enumerate(parts) if part == got[0]]
        self.assertTrue(any(parts[k:k + len(got)] == got for k in starts), got)

    def test_on_press_sees_every_row_in_order(self):
        seen = []
        result, _sent, _out, _took, chords = self._run_with(on_press=seen.append)
        self.assertEqual(result.stop, "cap")
        self.assertEqual(seen, result.rows)
        self.assert_a_run_of_the_plan(seen, chords)

    def test_should_stop_after_the_second_press_ends_the_run(self):
        sent = []
        result, _sent, _out, _took, chords = self._run_with(
            sent=sent, should_stop=lambda: len(sent) >= 2)
        self.assertEqual(result.stop, "stopped")
        self.assertEqual(len(sent), 2)
        self.assert_a_run_of_the_plan(result.rows, chords)

    def test_should_stop_false_changes_nothing(self):
        result, sent, _out, _took, chords = self._run_with(should_stop=lambda: False)
        self.assertEqual(result.stop, "cap")
        self.assertTrue(sent)
        self.assert_a_run_of_the_plan(result.rows, chords)


class _FakeDriver:
    """Records each chord press into a shared event list."""

    def __init__(self, events) -> None:
        self.events = events

    def press_chord(self, lanes):
        self.events.append(("press", list(lanes)))
        return ["keys"]


class LivePlayTest(unittest.TestCase):
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


# A committed schedule with ghost gems, some pressed beside full-strength ones.
_GHOST_SCHEDULE = os.path.join(_HERE, "bomber_refine_schedule.json")

# The ch_snapshot reply of the real run hydra-data/ch-mcp-runs/2026-11-06/
# ls-o1-run1, which read the engine's dynamics threshold field in Clone Hero.
_O1_SNAPSHOT = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-o1-run1",
                            "replies-snapshot.txt")


class _RecordedThresholdEngine:
    """The dynamics threshold one real run read from the game, handed back
    on each dyn_threshold call, which it counts."""

    def __init__(self) -> None:
        with open(_O1_SNAPSHOT, encoding="utf-8") as f:
            found = [line.split(":", 1)[1].split()[0] for line in f
                     if line.startswith("dynamics threshold:")]
        if len(found) != 1:
            raise AssertionError(f"{_O1_SNAPSHOT} holds {len(found)} threshold lines, not 1")
        self.value = int(found[0])
        self.reads = 0

    def dyn_threshold(self) -> int:
        self.reads += 1
        return self.value


def _presses_of(chords):
    return [p for chord in chords for p in chord.presses]


class DynamicsTest(unittest.TestCase):
    """with_axes gives each gem the strength of its own dynamic, from the
    threshold the game read, and reads that field only for a ghost. The
    ghost chart is _GHOST_SCHEDULE; the threshold is the one a real run
    read."""

    def _ghost_chart_with_axes(self):
        engine = _RecordedThresholdEngine()
        chords, threshold = play_chart.with_axes(
            play_chart.load_schedule(_GHOST_SCHEDULE).chords, engine)
        return chords, threshold, engine

    def test_each_gems_dynamic_sets_the_strength_of_its_own_lane(self):
        # A press that mixes a ghost with a full-strength gem presses the
        # ghost's lane softer than the others in the same message.
        from tools.ch_mcp.kit_tools import DYNAMIC_GHOST
        chords, _threshold, _engine = self._ghost_chart_with_axes()
        mixed = [p for p in _presses_of(chords)
                 if DYNAMIC_GHOST in p.dynamics and set(p.dynamics) != {DYNAMIC_GHOST}]
        self.assertTrue(mixed)
        for p in mixed:
            self.assertEqual(len(p.axes), len(p.lanes))
            ghost = {a for a, d in zip(p.axes, p.dynamics) if d == DYNAMIC_GHOST}
            other = {a for a, d in zip(p.axes, p.dynamics) if d != DYNAMIC_GHOST}
            self.assertLess(max(ghost), min(other), p)

    def test_a_ghost_chart_reads_the_threshold_once_and_returns_it(self):
        from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis
        chords, threshold, engine = self._ghost_chart_with_axes()
        self.assertEqual(engine.reads, 1)
        self.assertEqual(threshold, engine.value)
        ghost_axes = {a for p in _presses_of(chords)
                      for a, d in zip(p.axes, p.dynamics) if d == DYNAMIC_GHOST}
        # Every ghost gets pad_axis's ghost strength for the threshold read.
        self.assertEqual(ghost_axes, {pad_axis(DYNAMIC_GHOST, engine.value)})

    def test_a_chart_with_no_ghost_never_reads_the_threshold(self):
        # The Freak schedule has no ghost gem.
        engine = _RecordedThresholdEngine()
        chords, threshold = play_chart.with_axes(_freak_chords(), engine)
        self.assertEqual(engine.reads, 0)
        self.assertIsNone(threshold)
        presses = _presses_of(chords)
        self.assertTrue(presses)
        for p in presses:
            self.assertEqual(len(p.axes), len(p.lanes))

    def test_each_chord_reaches_the_press_with_its_axes(self):
        # The Freak schedule with its strengths, played on the recorded
        # mid-song clock: each press gets the axes with_axes gave its message.
        chords, _threshold = play_chart.with_axes(_freak_chords(),
                                                  _RecordedThresholdEngine())
        axes_of = {(tuple(p.lanes), p.index): p.axes for p in _presses_of(chords)}
        seen = []

        def press(lanes, axes=None, then=()):
            seen.append((tuple(lanes), axes))

        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_MID_SONG, chords=chords, press=press, cap_s=CallerHooksTest._CAP_S)
        self.assertEqual(result.stop, "cap")
        # Every row not dropped is a press, since no chord in this stretch is
        # a declined chord that keeps no gem.
        sent_rows = _pressed_rows(result)
        self.assertTrue(seen)
        self.assertEqual(len(seen), len(sent_rows))
        for (lanes, axes), row in zip(seen, sent_rows):
            self.assertIsNotNone(axes)
            self.assertEqual(axes, axes_of[(lanes, row["index"])])

    def test_a_ghost_or_accent_kick_gets_the_axis_a_pad_gets(self):
        # T2 of docs/superpowers/plans/2026-11-08-kick-axis-bind.md: the kit
        # presses the kick as an axis, so with_axes must hand a kick the
        # strength it hands a pad of the same dynamic. No committed chart has
        # a ghost or accent kick, so this chart is written by hand.
        dump = {"chords": [
            {"index": 0, "ms": 1000.0, "notes": [_gem("Kick", "ghost"),
                                                 _gem("Yellow", "accent", cymbal=True)]},
            {"index": 1, "ms": 1500.0, "notes": [_gem("Kick", "accent"), _gem("Red", "ghost")]},
        ]}
        engine = _RecordedThresholdEngine()
        chords, threshold = play_chart.with_axes(_load_dump(dump), engine)
        self.assertEqual(threshold, engine.value)
        kick_ghost, pad_accent = chords[0].presses[0].axes
        kick_accent, pad_ghost = chords[1].presses[0].axes
        self.assertEqual(chords[0].lanes, [Lane.KICK, Lane.YELLOW_CYMBAL])
        self.assertEqual(chords[1].lanes, [Lane.KICK, Lane.RED])
        self.assertEqual(kick_ghost, pad_ghost)
        self.assertEqual(kick_accent, pad_accent)
        self.assertLess(kick_ghost, kick_accent)


class PressReplyRowTest(unittest.TestCase):
    """A kit press's reply adds the values the press log has columns for to
    the row; a press that returns keys adds none."""

    def test_a_dict_reply_rides_into_the_row(self):
        # Real kit press replies, one per press in turn, on the recorded
        # mid-song clock: each row keeps its own reply's kit columns, and the
        # press log writes them.
        from tools.ch_probe.experiments.overlay_logs import KIT_PRESS_COLUMNS
        with open(_E_REPLIES, encoding="utf-8") as f:
            replies = [json.loads(line)["payload"] for line in f]
        given = iter(replies)
        returned = []

        def press(lanes, _axes=None, then=()):
            reply = next(given, None)
            if reply is None:
                raise AssertionError(f"the run pressed more often than {_E_REPLIES} replied")
            returned.append(reply)
            return reply

        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_MID_SONG, press=press, cap_s=CallerHooksTest._CAP_S, kit=True)
        self.assertEqual(result.stop, "cap")
        self.assertTrue(returned)
        rows = _pressed_rows(result)
        self.assertEqual(len(rows), len(returned))
        for row, reply in zip(rows, returned):
            self.assertEqual({c: row[c] for c in KIT_PRESS_COLUMNS},
                             {c: reply[c] for c in KIT_PRESS_COLUMNS})
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, rows)
            with open(path, encoding="utf-8", newline="") as f:
                logged = list(csv.DictReader(f))
        self.assertEqual([{c: r[c] for c in KIT_PRESS_COLUMNS} for r in logged],
                         [{c: str(reply[c]) for c in KIT_PRESS_COLUMNS} for reply in returned])

    def test_a_key_press_adds_nothing(self):
        result, _game, _wall = _run({"start_s": 12.0})
        self.assertEqual(set(result.rows[0]), {
            "index", "chart_ms", "clock_ms_at_press", "lead_ms", "score_before",
            "score_after", "send", "scored_between", "wall_s_at_read", "wall_s_at_reply",
            "wait_read_max_ms", "wait_oversleep_max_ms", "press_took_ms",
            "score_read_took_ms"})




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
        # The recorded mid-song clock and the real wall clock: each row's
        # read stamp comes before its press begins, and its reply stamp
        # after the press returns.
        stamps = []

        def press(lanes, _axes=None, then=()):
            before = time.time()
            stamps.append((before, time.time()))

        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_MID_SONG, press=press, cap_s=CallerHooksTest._CAP_S)
        self.assertEqual(result.stop, "cap")
        self.assertTrue(stamps)
        self.assertEqual(len(stamps), len(_pressed_rows(result)))
        for row, (before, after) in zip(_pressed_rows(result), stamps):
            self.assertLessEqual(row["wall_s_at_read"], before)
            self.assertLessEqual(after, row["wall_s_at_reply"])

    def test_the_columns_are_on_the_wall_clocks_scale(self):
        result, _stamps = self._run_with_a_slow_press(_load_chords()[:3])
        for row in result.rows:
            self.assertGreater(row["wall_s_at_read"], _WALL_EPOCH_S)
            self.assertGreater(row["wall_s_at_reply"], row["wall_s_at_read"])

    def test_a_chord_that_sends_no_press_still_gets_both_stamps(self):
        # Chord 134 of the Freak schedule is declined and keeps no gem. On the
        # recorded clock around it, the run sends it no press, and its row
        # still has both wall stamps, read first.
        declined = [p.index for p in _presses_of(_freak_chords()) if p.declined and not p.lanes]
        self.assertIn(134, declined)
        pressed = []

        def press(lanes, _axes=None, then=()):
            pressed.append(list(lanes))

        # The cap ends the run once the recording's clock passes chord 134's
        # last send, which comes before the recording ends.
        from tools.ch_probe import constants as C
        last_send_s = C.ms_to_s(next(p.last_send_ms for p in _presses_of(_freak_chords())
                                     if p.index == 134))
        with open(_F1_DECLINED, encoding="utf-8", newline="") as f:
            recorded = [(float(r["wall_s"]), float(r["clock_s"])) for r in csv.DictReader(f)]
        past = [wall for wall, clock in recorded if clock >= last_send_s]
        self.assertTrue(past)
        cap_s = past[0] - recorded[0][0]
        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_DECLINED, press=press, cap_s=cap_s)
        self.assertEqual(result.stop, "cap")
        rows = _rows_of(result, 134)
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(row[play_chart.SEND_COLUMN], play_chart.DECLINED)
        self.assertNotIn([], pressed)
        self.assertEqual(len(pressed), len(_pressed_rows(result)) - 1)
        self.assertLessEqual(row["wall_s_at_read"], row["wall_s_at_reply"])

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
        # Once the header is on disk, a second handle locks the bytes after
        # it, as another process holding the log can, so Windows refuses the
        # writer's first row. That error is kept, and no row reaches the file.
        import msvcrt
        writer = self._writer()
        self.assertEqual(self._table_once(0), [list(overlay_logs.PRESS_LOG_COLUMNS)])
        with open(self.path, "rb+") as other:
            other.seek(0, os.SEEK_END)
            # Any length past a row's would do; 1 MiB is past them all.
            msvcrt.locking(other.fileno(), msvcrt.LK_NBLCK, 1 << 20)
            try:
                writer.add({"index": 1})
                writer.add({"index": 2})
                self.assertTrue(writer.stop(self.CAP_S))
            finally:
                other.seek(0, os.SEEK_END)
                msvcrt.locking(other.fileno(), msvcrt.LK_UNLCK, 1 << 20)
        self.assertIsInstance(writer.error, OSError)
        self.assertEqual(_table(self.path), [list(overlay_logs.PRESS_LOG_COLUMNS)])

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
        # The Freak schedule on the recorded mid-song clock, where no chord
        # is split: every press call comes without `then`, one per row.
        not_given = object()
        calls = []

        def press(lanes, _axes=None, then=not_given):
            calls.append(then)

        chords = _freak_chords()
        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_MID_SONG, chords=chords, press=press, cap_s=CallerHooksTest._CAP_S)
        self.assertEqual(result.stop, "cap")
        played = {r["index"] for r in result.rows}
        self.assertFalse([later for head, later in play_chart.press_messages(
            play_chart.press_plan(chords)) if later and head.index in played])
        self.assertTrue(calls)
        self.assertEqual(calls, [not_given] * len(_pressed_rows(result)))

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
        # The mouths fixture's plan with one split chord's first press taken
        # out: its later group is left with no press to go down with.
        plan = play_chart.press_plan(play_chart.load_schedule(_MOUTHS_SCHEDULE).chords)
        later = next(p for p in plan if p.after_ms is not None)
        heads = [p for p in plan if p.index == later.index and p.after_ms is None]
        self.assertTrue(heads)
        kept = [p for p in plan if p not in heads]
        with self.assertRaises(ValueError) as caught:
            play_chart.press_messages(kept)
        # The message names the chord and the group's gap, from the press.
        said = str(caught.exception)
        self.assertRegex(said, rf"\b{later.index}\b")
        self.assertRegex(said, rf"\b{re.escape(f'{later.after_ms:g}')} ms\b")
        # The whole plan has a press for every later group, so it raises nothing.
        play_chart.press_messages(plan)


def _cap_until_clock_passes(path, ms):
    """The wall seconds from the recording's first row to its first row whose
    song clock is at or past `ms`: a run cap that ends a run on it there."""
    from tools.ch_probe import constants as C
    with open(path, encoding="utf-8", newline="") as f:
        recorded = [(float(r["wall_s"]), float(r["clock_s"])) for r in csv.DictReader(f)]
    past = [wall for wall, clock in recorded if clock >= C.ms_to_s(ms)]
    if not past:
        raise AssertionError(f"{os.path.basename(path)} never reaches {ms:g} ms")
    return past[0] - recorded[0][0]


# Rows 238 to 241 of the in-game run's press log
# hydra-data/ch-mcp-runs/2026-11-12/ls-g2-run1/press-1.csv: chord 239 was
# declined, and the game's grant for it came before chord 240's press.
_G2_DECLINED_ROWS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g2-run1",
                                 "press-1-rows-238-241.csv")


class ScoredBetweenTest(unittest.TestCase):
    """The points the game scores between presses go in the later row's
    `scored_between`, and the row's own step starts from the fresh read
    (design point 9 of the player-limits plan). The run plays the Freak
    schedule on the recorded clock and score around its declined chord 134,
    and stops once the clock passes chord 135's last send."""

    def _run(self):
        last_send_ms = next(p.last_send_ms for p in _presses_of(_freak_chords())
                            if p.index == 135)
        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_DECLINED, cap_s=_cap_until_clock_passes(_F1_DECLINED, last_send_ms))
        self.assertEqual(result.stop, "cap")
        return result

    def test_a_rise_between_presses_is_the_later_rows_scored_between(self):
        result = self._run()
        firsts = [r for r in result.rows if "scored_between" in r]
        self.assertGreater(len(firsts), 1)
        # Each message's first row counts from the score the one before ended on.
        for before, row in zip(firsts, firsts[1:]):
            self.assertEqual(row["scored_between"], row["score_before"] - before["score_after"])
        # The recording's score rises at clock 31.05 s, after chord 134's
        # reads and before chord 135's, so the rise is 135's.
        (declined,) = _rows_of(result, 134)
        self.assertEqual(declined[play_chart.SEND_COLUMN], play_chart.DECLINED)
        (row,) = _rows_of(result, 135)
        self.assertGreater(row["scored_between"], 0)
        self.assertEqual(row["score_before"], declined["score_after"] + row["scored_between"])

    def test_the_status_credits_the_pending_declined_chord(self):
        from tools.ch_mcp import game_tools
        with open(_G2_DECLINED_ROWS, encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f))
        (declined,) = [r["index"] for r in rows
                       if r[play_chart.SEND_COLUMN] == play_chart.DECLINED]
        (granted,) = [r for r in rows if float(r["scored_between"])]
        (line,) = game_tools.scored_between_lines(rows)
        # The one line names the declined chord, the amount and the chord it
        # came before, in that order.
        amount = f"{float(granted['scored_between']):g}"
        self.assertRegex(line, rf"\b{declined}\b.*\b{amount}\b.*\b{granted['index']}\b")

    def test_the_new_columns_reach_the_press_log(self):
        result = self._run()
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                reader = csv.DictReader(f)
                table = list(reader)
                header = tuple(reader.fieldnames)
        self.assertEqual(header, overlay_logs.PRESS_LOG_COLUMNS)
        self.assertEqual(len(table), len(result.rows))
        # Every value a row holds for a press-log column is in the file, and a
        # column it does not hold is empty there.
        self.assertTrue([r for r in result.rows if "scored_between" in r])
        for row, written in zip(result.rows, table):
            self.assertEqual(written, {c: str(row[c]) if c in row else "" for c in header})


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
        # The Freak plan, with chord 263's end read 3 ms after the end the
        # file planned for it.
        plan = play_chart.press_plan(self._chords())
        (reader,) = [p for p in plan if p.index == 263 and p.planned_end_ms is not None]
        read_ms = reader.planned_end_ms + 3.0
        moved = play_chart.retime_from_end(plan, 263, read_ms)
        self.assertEqual(len(moved), len(plan))
        own = [p for p in moved if p.end_read_after == 263]
        self.assertTrue(own)
        # The presses timed from that read carry it and fire at a new time.
        before = {(p.index, p.after_ms): p for p in plan}
        for p in own:
            self.assertEqual(p.aim_end_ms, read_ms)
            self.assertNotEqual(p.at_ms, before[(p.index, p.after_ms)].at_ms)
        # Every other press, those of the other activations among them, is
        # the plan's own.
        others = [p for p in moved if p.end_read_after != 263]
        self.assertTrue([p for p in others if p.end_read_after is not None])
        self.assertEqual(sorted(others, key=lambda p: p.at_ms),
                         [p for p in plan if p.end_read_after != 263])


class LivePressTest(unittest.TestCase):
    """live_press focuses through input_driver.focus_window."""

    def test_the_focus_comes_from_input_driver(self):
        from tools.ch_probe import input_driver
        self.assertIs(play_chart.focus_window, input_driver.focus_window)


# --- a press past its window is dropped (design point 5 of
# docs/superpowers/plans/2026-11-08-player-freak-fixes.md) -----------------------

# The same log's stretch from clock 33.4 s to 34.2 s, where the game's clock
# held at 33.618 s for 131 ms of wall time and came back at 33.771 s: Freak
# run 1 reached chord 153 late there.
_F1_STALL = os.path.join(_F1_RUN, "sp_watch-1-clock-33.4-34.2.csv")

# Windows of the Star Power watcher logs (sp_watch-1.csv) of two real runs
# in Clone Hero, hydra-data/ch-mcp-runs/2026-11-07/ls-f1-run1 (Freak) and
# 2026-11-07/ls-r1-run1 (Mouths), each around a chord after which the
# schedule reads the game's end. They keep the wall time, song clock, score,
# active byte and stored end columns.
_F1_READ_263 = os.path.join(_F1_RUN, "sp_watch-1-sp-49.6-50.0.csv")
# From before chord 1302 to past chord 1369's last send; the active byte is
# clear throughout.
_F1_NO_READ_1302 = os.path.join(_F1_RUN, "sp_watch-1-sp-154.4-160.9.csv")
_R1_RUN = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-r1-run1")
_R1_READ_340 = os.path.join(_R1_RUN, "sp_watch-1-sp-57.2-57.6.csv")
_R1_READ_704 = os.path.join(_R1_RUN, "sp_watch-1-sp-110.4-110.8.csv")


class FrozenClockDropTest(unittest.TestCase):
    """The game's clock stalls during the wait for chord 153 of the Freak
    schedule, so the run reaches it past its last send. The run plays the
    recording of that stall and stops once the clock passes chord 154's last
    send."""

    _DROPPED_INDEX = 153

    @classmethod
    def setUpClass(cls):
        plan = play_chart.press_plan(_freak_chords())
        (cls.late,) = [p for p in plan if p.index == cls._DROPPED_INDEX]
        (cls.next_press,) = [p for p in plan if p.index == cls._DROPPED_INDEX + 1]
        cls.lanes = {p.index: list(p.lanes) for p in plan}
        cls.result, cls.sent, cls.out, _took, _chords = _play_recorded(
            _F1_STALL, cap_s=_cap_until_clock_passes(_F1_STALL, cls.next_press.last_send_ms))

    def test_the_late_chord_is_dropped_and_the_next_is_sent(self):
        # The case the test stands on: two recorded reads in a row, the first
        # before the press is due and the next past its last send.
        from tools.ch_probe import constants as C
        with open(_F1_STALL, encoding="utf-8", newline="") as f:
            clocks = [C.s_to_ms(float(r["clock_s"])) for r in csv.DictReader(f)]
        self.assertTrue([1 for a, b in zip(clocks, clocks[1:])
                         if a < self.late.at_ms and b > self.late.last_send_ms])
        result = self.result
        self.assertEqual(result.stop, "cap")
        (row,) = _rows_of(result, self.late.index)
        # The read that would send it is past its last send.
        self.assertGreater(row["clock_ms_at_press"], self.late.last_send_ms)
        self.assertEqual(row[play_chart.SEND_COLUMN], play_chart.DROPPED)
        # Nothing was pressed, so no second score read moved the score.
        self.assertEqual(row["score_before"], row["score_after"])
        # Every row but the dropped one sent its own chord's keys, in order.
        self.assertEqual(self.sent, [self.lanes[r["index"]] for r in _pressed_rows(result)])
        (after,) = _rows_of(result, self.next_press.index)
        self.assertEqual(after[play_chart.SEND_COLUMN], play_chart.SENT)

    def test_the_counts_leave_the_drop_out(self):
        result = self.result
        sends = [r[play_chart.SEND_COLUMN] for r in result.rows]
        self.assertEqual((result.sent, result.dropped, result.declined),
                         (sends.count(play_chart.SENT), sends.count(play_chart.DROPPED),
                          sends.count(play_chart.DECLINED)))
        self.assertGreaterEqual(result.dropped, 1)

    def test_the_printed_line_says_dropped(self):
        # A printed line carries each drop, and one is the late chord's: it
        # gives that chord's chart time in seconds.
        from tools.ch_probe import constants as C
        lines = [line for line in self.out.splitlines()
                 if re.search(rf"\b{play_chart.DROPPED}\b", line)]
        self.assertEqual(len(lines), self.result.dropped)
        chart_s = f"{C.ms_to_s(self.late.chart_ms):.2f}"
        self.assertTrue([line for line in lines if chart_s in line.split()], lines)


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
        # In the real run ls-f1-run1 no Star Power was active when the
        # schedule reads the end after chord 1302, and chord 1369's press is
        # timed from that read. The run plays the recording's clock, score
        # and end from before 1302 until the clock passes 1369's last send.
        chords = self._chords()
        late = _press_of(chords, 1369)
        self.assertEqual(late.end_read_after, 1302)
        (reader_ms,) = [c.ms for c in chords if c.index == 1302]
        self.assertIsNone(_recorded_end_ms(_F1_NO_READ_1302, reader_ms))
        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_NO_READ_1302, chords=chords, read_end=True,
            cap_s=_cap_until_clock_passes(_F1_NO_READ_1302, late.last_send_ms))
        self.assertEqual(result.stop, "cap")
        self.assertEqual([r["end_read_ms"] for r in _rows_of(result, 1302) if "end_read_ms" in r],
                         [play_chart.NO_ACTIVATION_READ])
        (row,) = _rows_of(result, 1369)
        self.assertEqual(row["lead_ms"], -late.aim_ms)
        self.assertNotIn("retimed", row)


def _press_of_all(chords, index):
    """Chord `index`'s presses in `chords`, in firing order."""
    return sorted((p for c in chords if c.index == index for p in c.presses),
                  key=lambda p: p.at_ms)


class FallbackTest(unittest.TestCase):
    """A moved press with a `fallback` does what the block says at the
    block's threshold, and its row says so in `retimed` (design point 5 of
    docs/superpowers/plans/2026-11-08-replay-press-schedule.md, and its
    question 3 (a) for the squeeze-in). The fixtures give the thresholds:
    Freak 387's and 1144's `if_end_after_ms`, Mouths 740's (whose late gem
    joins gems 1 and 2) and Mouths 431's `if_end_before_ms`. The reads after
    Freak 263 and Mouths 340 and 704 are the ends real runs recorded."""

    def _run(self, path, overrides):
        chords = play_chart.load_schedule(path).chords
        result, game = _play_schedule(chords, _reads(chords, overrides))
        self.assertEqual(result.stop, "done")
        return chords, result, game

    def test_a_recorded_read_later_than_planned_moves_the_late_gem_by_its_own_end(self):
        # The real run ls-f1-run1's end after Freak chord 263, which chord
        # 387's late gem is timed from: later than the file planned, and
        # before the gem's threshold, so the gem stays late and moves.
        chords = play_chart.load_schedule(_FREAK_SCHEDULE).chords
        late = _press_of(chords, 387)
        self.assertEqual(late.end_read_after, 263)
        (reader_ms,) = [c.ms for c in chords if c.index == 263]
        read_ms = _recorded_end_ms(_F1_READ_263, reader_ms)
        self.assertGreater(read_ms, late.aim_end_ms)
        self.assertLess(read_ms, late.fallback["if_end_after_ms"])
        (moved,) = [p for p in play_chart.retime_from_end(play_chart.press_plan(chords), 263,
                                                          read_ms) if p.index == 387]
        self.assertEqual(moved.retimed, play_chart.RETIMED_LATE)
        self.assertAlmostEqual(moved.at_ms - late.at_ms,
                               play_chart.own_end_now_ms(late, read_ms) - late.own_end_ms,
                               delta=1e-9)
        self.assertGreater(moved.at_ms, late.at_ms)

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

    def test_a_recorded_read_before_the_threshold_keeps_the_late_gem_apart(self):
        # The real run ls-r1-run1's end after Mouths chord 704, which chord
        # 740's late gem is timed from: before the gem's threshold, so it
        # stays late, out of the inside group its fallback names.
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        late = _press_of(chords, 740, "squeeze_out_late")
        inside = _press_of(chords, 740, "squeeze_out_inside")
        self.assertEqual(late.end_read_after, 704)
        (reader_ms,) = [c.ms for c in chords if c.index == 704]
        read_ms = _recorded_end_ms(_R1_READ_704, reader_ms)
        self.assertLess(read_ms, late.fallback["if_end_after_ms"])
        moved = {p.rule: p for p in play_chart.retime_from_end([inside, late], 704, read_ms)}
        self.assertEqual(moved[late.rule].retimed, play_chart.RETIMED_LATE)
        self.assertEqual(moved[late.rule].lanes, late.lanes)
        self.assertAlmostEqual(moved[late.rule].at_ms - late.at_ms,
                               play_chart.own_end_now_ms(late, read_ms) - late.own_end_ms,
                               delta=1e-9)
        self.assertIsNone(moved[inside.rule].retimed)
        self.assertEqual(moved[inside.rule].lanes, inside.lanes)

    def test_a_squeeze_in_with_a_recorded_read_past_its_threshold_stays_timed(self):
        # The real run ls-r1-run1's end after Mouths chord 340, which chord
        # 431's squeeze-in is timed from: not before the threshold its plain
        # fallback names, so it moves by its own end and is not pressed plain.
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        squeeze = _press_of(chords, 431)
        self.assertEqual(squeeze.end_read_after, 340)
        (reader_ms,) = [c.ms for c in chords if c.index == 340]
        read_ms = _recorded_end_ms(_R1_READ_340, reader_ms)
        self.assertGreaterEqual(read_ms, squeeze.fallback["if_end_before_ms"])
        (moved,) = play_chart.retime_from_end([squeeze], 340, read_ms)
        self.assertIsNone(moved.retimed)
        self.assertAlmostEqual(moved.at_ms - squeeze.at_ms,
                               play_chart.own_end_now_ms(squeeze, read_ms) - squeeze.own_end_ms,
                               delta=1e-9)

    def test_a_block_with_another_action_is_refused(self):
        # A test input: an action name the schedule writer never writes.
        action = "drop"
        late = _press_of(play_chart.load_schedule(_FREAK_SCHEDULE).chords, 1144)
        import dataclasses
        odd = dataclasses.replace(late, fallback={**late.fallback, "action": action})
        with self.assertRaises(ValueError) as caught:
            play_chart.retime_from_end([odd], late.end_read_after, late.aim_end_ms)
        # The refusal names the chord and the action it does not know.
        said = str(caught.exception)
        self.assertRegex(said, rf"\b{late.index}\b")
        self.assertIn(repr(action), said)

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
        # The Mouths fixture's squeeze-in, read just past its range's high
        # side. The read is past the block's threshold too, so the threshold
        # alone would leave it timed from the end: only the range sends it
        # plain, at the block's aim.
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        press = _press_of(chords, 431, "squeeze_in")
        block = press.fallback
        read_ms = press.read_range_ms[1] + 0.01
        self.assertEqual(block["action"], "plain")
        self.assertGreater(read_ms, block["if_end_before_ms"])
        (plain,) = play_chart.retime_from_end([press], press.end_read_after, read_ms)
        self.assertEqual((plain.aim_ms, plain.retimed), (block["aim_ms"], play_chart.RETIMED_RANGE))

    def test_a_squeeze_outs_late_gem_outside_joins_its_group(self):
        # The Mouths fixture's squeeze-out at chord 740, read just below its
        # range's low side. The read is below the late gem's threshold, so
        # the threshold alone would leave the gem late: only the range joins
        # it to its group, at the group's time.
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        group = _press_of(chords, 740, "squeeze_out_inside")
        late = _press_of(chords, 740, "squeeze_out_late")
        self.assertEqual(late.fallback["action"], "join_group")
        read_ms = late.read_range_ms[0] - 0.01
        self.assertLess(read_ms, late.fallback["if_end_after_ms"])
        lanes = {c.index: c.lanes for c in chords}
        (joined,) = play_chart.retime_from_end([group, late], late.end_read_after, read_ms,
                                               lanes)
        self.assertEqual(sorted(joined.lanes), sorted(group.lanes + late.lanes))
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
        # The Mouths fixture's press at chord 465 has a different conversion
        # on each side. What the run does with the loaded fields: a read at
        # the file's stored read end puts the press's own end at the file's
        # stored own end, a read either side of it moves the own end at that
        # side's ratio, and a read past the range moves nothing.
        raw = next(e for e in _read_json(_MOUTHS_SCHEDULE)["presses"] if e["chord"] == 465)
        self.assertNotEqual(raw["conversion"]["early"], raw["conversion"]["late"])
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        press = _press_of(chords, 465)
        after = press.end_read_after

        def own_end_after(read_ms):
            (moved,) = play_chart.retime_from_end([press], after, read_ms)
            self.assertIsNone(moved.retimed)
            self.assertAlmostEqual(moved.at_ms - press.at_ms,
                                   moved.own_end_ms - raw["own_end_ms"], delta=1e-9)
            return moved.own_end_ms

        stored = raw["read_stored_end_ms"]
        self.assertAlmostEqual(own_end_after(stored), raw["own_stored_end_ms"], delta=1e-9)
        for side, step_ms in (("early", -10.0), ("late", 10.0)):
            with self.subTest(side):
                moved_ms = own_end_after(stored + step_ms) - raw["own_stored_end_ms"]
                self.assertAlmostEqual(moved_ms, raw["conversion"][side] * step_ms, delta=1e-9)
        (kept,) = play_chart.retime_from_end([press], after, raw["read_range_ms"][1] + 0.01)
        self.assertEqual((kept.at_ms, kept.retimed), (press.at_ms, play_chart.RETIMED_RANGE))
        # A press with no end_read_after is never moved by a read.
        plain = next(p for c in chords for p in c.presses if p.end_read_after is None)
        (unmoved,) = play_chart.retime_from_end([plain], after, stored + 10.0)
        self.assertEqual(unmoved, plain)

    def test_a_null_range_is_read_as_no_range(self):
        doc = self._doc()
        doc["presses"][1]["read_range_ms"] = None
        chords = _write_and_load(doc, play_chart.load_schedule).chords
        press = next(p for c in chords for p in c.presses if p.index == 41)
        self.assertIsNone(press.read_range_ms)

    def _refused(self, edit):
        """The refusal of the Mouths fixture with `edit(entry)` applied to
        its press at chord 465, an end-timed press, and that press's entry
        number in `presses`. The intact fixture loads."""
        play_chart.load_schedule(_MOUTHS_SCHEDULE)
        doc = _read_json(_MOUTHS_SCHEDULE)
        k = next(k for k, e in enumerate(doc["presses"]) if e["chord"] == 465)
        self.assertIsNotNone(doc["presses"][k]["end_read_after"])
        edit(doc["presses"][k])
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        return str(caught.exception), k

    def assert_names(self, message, k, key):
        # The refusal names the press's entry and the field, so the reader
        # knows which value the file lacks.
        self.assertRegex(message, rf"\bentry {k}\b")
        self.assertIn(f"`{key}`", message)

    def test_each_missing_field_is_named(self):
        for key in play_chart.END_FIELDS:
            with self.subTest(key):
                message, k = self._refused(lambda entry: entry.pop(key))
                self.assert_names(message, k, key)

    def test_a_null_number_and_a_one_sided_conversion_are_named(self):
        # read_range_ms is the one field a null is allowed in.
        for key in (name for name in play_chart.END_FIELDS if name != "read_range_ms"):
            with self.subTest(key):
                message, k = self._refused(lambda entry: entry.update({key: None}))
                self.assert_names(message, k, key)
                self.assertRegex(message, r"\bnull\b")
        message, k = self._refused(lambda entry: entry["conversion"].pop("late"))
        self.assert_names(message, k, "late")


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

# The recorded clock from 4.2 s to 5.6 s of the same Freak run's watcher log,
# cut from hydra-data/ch-mcp-runs/2026-11-07/ls-f1-run1/sp_watch-1.csv like the
# clips above. It passes the Mouths schedule's chord 4, a split chord.
_F1_SPLIT_CHORD = os.path.join(_F1_RUN, "sp_watch-1-clock-4.2-5.6.csv")


def _e_replies():
    """The kit's real press replies in _E_REPLIES, in the order it sent them."""
    with open(_E_REPLIES, encoding="utf-8") as f:
        return [json.loads(line)["payload"] for line in f]


def _replying(reply, calls):
    """A kit press that notes each call's lanes in `calls` and answers every
    press with `reply`."""
    def press(lanes, _axes=None, then=()):
        calls.append(list(lanes))
        return reply
    return press


class ReplyStopTest(unittest.TestCase):
    """A kit reply without what its press asked for stops the run at that
    chord (D318 item 2), on a recorded game clock."""

    def test_a_reply_without_groups_stops_the_run_at_that_chord(self):
        # From 4.2 s the first message the Mouths schedule sends is chord 4,
        # split in two groups. The kit's real reply to a one-group press has
        # no `groups` list, so it is not a reply to this press.
        from tools.ch_mcp.kit_tools import reply_gap
        chords = play_chart.load_schedule(_MOUTHS_SCHEDULE).chords
        reply = _e_replies()[0]
        self.assertNotIn("groups", reply)
        start_ms = _RecordedGame(_F1_SPLIT_CHORD).read()[0] * 1000.0
        head, later = next((h, l) for h, l in play_chart.press_messages(
            play_chart.press_plan(chords)) if h.at_ms - start_ms >= live.START_LEAD_MS)
        self.assertTrue(later)
        calls = []
        result, _sent, out, _took, _chords = _play_recorded(
            _F1_SPLIT_CHORD, chords=chords, press=_replying(reply, calls), kit=True,
            cap_s=_cap_until_clock_passes(_F1_SPLIT_CHORD, head.last_send_ms))
        self.assertEqual(result.stop, "reply")
        self.assertRegex(result.message, rf"\bchord {head.index}\b")
        self.assertIn(reply_gap(reply, 1 + len(later)), result.message)
        self.assertIn(result.message, out)
        # No later chord is pressed and the stopped press logs no row.
        self.assertEqual(calls, [head.lanes])
        self.assertEqual(result.rows, [])

    def test_a_reply_that_is_not_a_dict_or_lacks_down_up_stops_the_run(self):
        # The Freak schedule on the recorded mid-song clock, with the kit's
        # real reply cut down or replaced: the first press stops the run,
        # and the stop names the chord whose lanes were pressed.
        from tools.ch_mcp.kit_tools import reply_gap
        whole = _e_replies()[0]
        self.assertIsNone(reply_gap(whole, 1))
        cases = {
            "None": None,
            "a list": [Lane.RED],
            "no down_ms or up_ms": {k: v for k, v in whole.items() if k not in ("down_ms", "up_ms")},
            "no down_frame": {k: v for k, v in whole.items() if k != "down_frame"},
        }
        chords = _freak_chords()
        for what, reply in cases.items():
            with self.subTest(what):
                calls = []
                result, _sent, _out, _took, _chords = _play_recorded(
                    _F1_MID_SONG, press=_replying(reply, calls), kit=True,
                    cap_s=CallerHooksTest._CAP_S)
                self.assertEqual(result.stop, "reply")
                self.assertEqual(len(calls), 1)
                self.assertIn(reply_gap(reply, 1), result.message)
                named = re.search(r"\bchord (\d+)\b", result.message)
                self.assertIsNotNone(named, result.message)
                self.assertIn(calls[0], [p.lanes for p in
                                         _press_of_all(chords, int(named.group(1)))])
        # PressReplyRowTest plays the whole real replies on to the cap.

    def test_a_keys_press_reply_is_not_read(self):
        # The keys route returns the keys it pressed, which is no kit reply;
        # with no kit the run never checks it and plays on to the cap.
        from tools.ch_mcp.kit_tools import reply_gap
        calls = []
        result, _sent, _out, _took, _chords = _play_recorded(
            _F1_MID_SONG, press=_replying([Lane.RED], calls), cap_s=CallerHooksTest._CAP_S)
        self.assertEqual(result.stop, "cap")
        self.assertTrue(calls)
        self.assertIsNotNone(reply_gap([Lane.RED], 1))
        self.assertEqual(len(_pressed_rows(result)), len(calls))


class LoadRefusalTest(unittest.TestCase):
    """load_schedule refuses a file without what the play reads (D318 item
    2, D321 item 3, D323 item 2)."""

    def _refusal(self, doc):
        with self.assertRaises(ValueError) as caught:
            _write_and_load(doc, play_chart.load_schedule)
        return str(caught.exception)

    def _freak_refusal(self, edit):
        """The refusal of the Freak fixture with `edit(doc)` applied. The
        refusal names the file it read."""
        doc = _read_json(_FREAK_SCHEDULE)
        edit(doc)
        message = self._refusal(doc)
        self.assertIn("schedule.json", message)
        return message, doc

    def test_load_schedule_refuses_a_missing_key(self):
        # The Freak fixture with one key taken out: the refusal names the key,
        # and for a chord's key the chord's entry. The plan names a chord's
        # `notes`; a schedule chord holds its gems in `gems` (schedule_json in
        # tools/replay_schedule.cpp), so that is the key refused here.
        for key in ("song_length_ms", "chords"):
            with self.subTest(key):
                message, _doc = self._freak_refusal(lambda doc: doc.pop(key))
                self.assertIn(f"`{key}`", message)
        for key in ("tick", "gems"):
            with self.subTest(key):
                message, _doc = self._freak_refusal(lambda doc: doc["chords"][1].pop(key))
                self.assertIn(f"`{key}`", message)
                self.assertRegex(message, r"\bentry 1 of `chords`")
        message, _doc = self._freak_refusal(lambda doc: doc.update(song_length_ms=None))
        self.assertIn("song_length_ms", message)
        self.assertRegex(message, r"\bnull\b")

    def test_a_chord_with_no_notes_is_refused(self):
        message, doc = self._freak_refusal(lambda doc: doc["chords"][2].update(gems=[]))
        self.assertRegex(message, rf"\bchord {doc['chords'][2]['index']}\b")
        self.assertRegex(message, r"\bno gems\b")

    def test_an_empty_chord_list_is_refused_in_ch_play_charts_words(self):
        doc = _schedule_of({"chords": []})
        self.assertTrue(self._refusal(doc).endswith("schedule.json has no chords to play"))

    def test_a_press_naming_a_missing_chord_or_gem_is_refused(self):
        # A chord number one past the file's last, and a gem one past the
        # chord's last: each refusal names what the press named.
        message, doc = self._freak_refusal(lambda doc: doc["presses"][0].update(
            chord=max(c["index"] for c in doc["chords"]) + 1))
        self.assertRegex(message, rf"\bentry 0 of `presses`")
        self.assertRegex(message, rf"\bchord {doc['presses'][0]['chord']}\b")

        def past_last_gem(doc):
            entry = doc["presses"][0]
            chord = next(c for c in doc["chords"] if c["index"] == entry["chord"])
            entry["gems"] = [len(chord["gems"])]

        message, doc = self._freak_refusal(past_last_gem)
        entry = doc["presses"][0]
        held = len(next(c for c in doc["chords"] if c["index"] == entry["chord"])["gems"])
        self.assertIn(f"gems {entry['gems']}", message)
        self.assertRegex(message, rf"\bchord {entry['chord']}\b")
        self.assertRegex(message, rf"\bholds {held}\b")


class MainExitTest(unittest.TestCase):
    """main exits 0 only when the run reached the song's end, and 1 with the
    reason for every other stop (D319 item 1, D323 item 2). live_play needs
    the running game, so in its place main gets the result of a real
    play_chords run of the chords it read, on a recorded game clock."""

    def _main(self, play, path=_FREAK_SCHEDULE):
        """main on the schedule at `path`, with live_play replaced by
        `play(chords)`; it gives (exit code, printed text, the run's result)."""
        ran = []

        def live_play(chords, **_kw):
            ran.append(play(chords))
            return ran[-1]

        out = io.StringIO()
        with mock.patch.object(play_chart, "live_play", live_play):
            code = play_chart.main([path], out=out)
        return code, out.getvalue(), (ran[0] if ran else None)

    def test_main_exits_nonzero_on_every_stop_but_the_song_end(self):
        cap_s = CallerHooksTest._CAP_S
        no_frame = {k: v for k, v in _e_replies()[0].items() if k != "down_frame"}
        mid_ms = _RecordedGame(_F1_MID_SONG).read()[0] * 1000.0

        def on(path, chords, **kwargs):
            return _play_recorded(path, chords=chords, **kwargs)[0]

        stops = {
            # The whole recording comes before the first chord.
            "cap": lambda chords: on(_F1_BEFORE_FIRST_CHORD, chords, cap_s=1),
            "stopped": lambda chords: on(_F1_MID_SONG, chords, cap_s=cap_s,
                                         should_stop=lambda: True),
            "reply": lambda chords: on(_F1_MID_SONG, chords, cap_s=cap_s, kit=True,
                                       press=_replying(no_frame, [])),
            # Only the chords before the recording's first clock read.
            "past_end": lambda chords: on(_F1_MID_SONG, [c for c in chords if c.ms < mid_ms],
                                          cap_s=cap_s),
        }
        for stop, play in stops.items():
            with self.subTest(stop):
                code, text, result = self._main(play)
                self.assertEqual(result.stop, stop)
                self.assertTrue(result.message)
                self.assertEqual(code, 1, text)
                self.assertIn(f"({stop}): {result.message}", text)

        # A file load_schedule refuses never reaches the run.
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "schedule.json")
            doc = _read_json(_FREAK_SCHEDULE)
            doc["chords"] = []
            with open(path, "w", encoding="utf-8") as f:
                json.dump(doc, f)
            with self.assertRaises(ValueError) as caught:
                play_chart.load_schedule(path)
            code, text, result = self._main(lambda chords: self.fail("the run began"), path)
        self.assertEqual(code, 1)
        self.assertIsNone(result)
        self.assertIn(str(caught.exception), text)

        # The song end: the chords from 0.3 s to 1.5 s after the recording's
        # first read, all pressed inside its 2 s cap.
        def song_end(chords):
            window = [c for c in chords if mid_ms + 300.0 < c.ms < mid_ms + 1500.0]
            self.assertTrue(window)
            return on(_F1_MID_SONG, window, cap_s=cap_s)

        code, text, result = self._main(song_end)
        self.assertEqual(result.stop, play_chart.STOP_DONE)
        self.assertEqual(code, 0, text)
        self.assertNotIn(f"({result.stop})", text)


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
        # The Freak chords before the recorded mid-song clock's first read,
        # played on that clock: the run starts past its last note, plays
        # nothing and counts every chord with a press as skipped.
        game = _RecordedGame(_F1_MID_SONG)
        with open(_F1_MID_SONG, encoding="utf-8", newline="") as f:
            clocks_s = [float(r["clock_s"]) for r in csv.DictReader(f)]
        chords = [c for c in _freak_chords() if c.ms < clocks_s[0] * 1000.0]
        messages = play_chart.press_messages(play_chart.press_plan(chords))
        pressed = {p.index for head, later in messages for p in (head, *later)}
        self.assertTrue(pressed)
        sent = []
        out = io.StringIO()
        result = play_chart.play_chords(chords, game, game.read_score,
                                        lambda lanes, _axes=None, then=(): sent.append(lanes),
                                        out=out)
        self.assertEqual(result.stop, "past_end")
        self.assertEqual(result.skipped, len(pressed))
        self.assertEqual((sent, result.rows), ([], []))
        # The line is start_line's for the clock read the run made, one of
        # the recording's rows.
        lines = {play_chart.start_line(clock_s, messages, len(messages))[0]
                 for clock_s in clocks_s}
        self.assertIn(out.getvalue().splitlines()[0], lines)


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
