"""play_chart.py plays the chords Hydra analyzed, read from a hydra_replay file.

The chords and times come from a `hydra_replay score` JSON: each entry of its
"chords" list gives the chord's time (`ms`) and its "notes", one per gem, each
with the gem's color and whether it is a cymbal. One gem is one key: pressing
a tom and a cymbal key for one gem is an overhit. Waiting for a note, noticing
the song stopped and picking the first note when a run joins mid-song are
live.py's.

A run is tested on fakes for the song clock, the score and the keys, so no
test opens the game or sends a key: the press log's rows, the frozen-clock
stop, the wall-time cap, and main()'s read-only open and capped engine search.
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

from tools.ch_probe import engine_finder  # noqa: E402
from tools.ch_probe.experiments import live, overlay_logs, play_chart  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402


# Eight chord blocks copied whole from a real run of
#   hydra_replay score --chart "testdata/input/common/IB24/T2/Area 11 - Knightmare Frame/notes.mid" --acts ""
# (Expert Pro Drums, 2x Bass), one per kind of pad: yellow cymbal, the 2x
# kick, kick + yellow cymbal + green cymbal, kick + green cymbal, yellow tom,
# green tom, blue cymbal, and a red accent. Each block's "measure" object is
# rewritten to the shape the tool prints now ("label" and "decimal"); the
# labels are the old measure, beat and tick written as the screen writes them.
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


def _load(loader=None):
    loader = play_chart.load_dump_notes if loader is None else loader
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "dump.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(_DUMP, f)
        return loader(path)


class DumpPathTest(unittest.TestCase):
    def test_dump_chords_become_timed_lanes(self):
        self.assertEqual([(ms, list(lanes)) for ms, lanes in _load()], [
            (13043.48, [Lane.YELLOW_CYMBAL]),
            (13614.132249999999, [Lane.KICK]),  # the 2x kick presses L
            (14184.7845, [Lane.KICK, Lane.YELLOW_CYMBAL, Lane.GREEN_CYMBAL]),
            (22010.872499999998, [Lane.KICK, Lane.GREEN_CYMBAL]),
            (22663.046499999997, [Lane.YELLOW]),
            (22989.133499999996, [Lane.GREEN]),
            (78260.88, [Lane.BLUE_CYMBAL]),
            (108586.97099999999, [Lane.RED]),  # an accent is still the red pad
        ])

    def test_dump_chords_keep_their_index(self):
        chords = _load(play_chart.load_dump_chords)
        self.assertEqual([c.index for c in chords],
                         [38, 41, 45, 91, 95, 97, 400, 573])
        # The same times and keys load_dump_notes gives.
        self.assertEqual([(c.ms, c.lanes) for c in chords], _load())

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

    def test_press_lead_is_two_ms(self):
        self.assertEqual(play_chart.PRESS_LEAD_MS, 2.0)

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
    """The score: 100 points per press, except the presses numbered in
    `misses` (1-based)."""

    def __init__(self, misses=()) -> None:
        self.score = 0
        self.presses = []
        self.misses = set(misses)

    def press(self, lanes, _axes=None) -> None:
        self.presses.append(list(lanes))
        if len(self.presses) not in self.misses:
            self.score += 100

    def read_score(self) -> int:
        return self.score


def _run(clock_kwargs, misses=()):
    wall = _FakeTime()
    game = _FakeGame(misses)
    result = play_chart.play_chords(
        _load(play_chart.load_dump_chords),
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
        # The clock the wait returned: inside the press lead, before the chord.
        for r in body:
            early_ms = float(r[col["chart_ms"]]) - float(r[col["clock_ms_at_press"]])
            self.assertGreater(early_ms, 0.0)
            self.assertLessEqual(early_ms, play_chart.PRESS_LEAD_MS)


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
            _load(play_chart.load_dump_chords),
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
            path = os.path.join(d, "score.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump(_DUMP, f)
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
            _load(play_chart.load_dump_chords),
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
            _load(play_chart.load_dump_chords),
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
            _load(play_chart.load_dump_chords), open_proc=opener,
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
                _load(play_chart.load_dump_chords),
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

        seen = {}

        def fake_play_chords(*args, **kwargs):
            seen.update(kwargs)
            return play_chart.RunResult(stop="done")

        with mock.patch.object(play_chart, "EngineModel", _FakeEngine), \
                mock.patch.object(play_chart, "find_game_window", lambda: 0), \
                mock.patch.object(play_chart, "play_chords", fake_play_chords):
            play_chart.live_play(
                _load(play_chart.load_dump_chords), open_proc=lambda **kw: _FakeProc(),
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
                _load(play_chart.load_dump_chords), open_proc=lambda **kw: _FakeProc(),
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
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "dump.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(dump, f)
        return play_chart.load_dump_chords(path)


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
        self.assertEqual(seen[0].axes, [pad_axis("ghost", 64), pad_axis("accent", 64),
                                        pad_axis("none", 64)])
        self.assertEqual(seen[1].axes, [pad_axis("none", 64)])

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
        result, seen, text = self._live_play(_load(play_chart.load_dump_chords), 0)
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
        self.assertEqual(seen, [chords[0].axes, chords[1].axes])


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
            _load(play_chart.load_dump_chords)[:1], _FakeSongClock(wall, start_s=12.0),
            game.read_score, press, now=wall.now, sleep=wall.sleep, out=io.StringIO())
        self.assertEqual(result.rows[0], {
            "index": 38, "chart_ms": 13043.48, "clock_ms_at_press": result.rows[0]["clock_ms_at_press"],
            "lead_ms": 2.0, "score_before": 0, "score_after": 100, "hit": 1,
            "down_frame": 40, "up_frame": 42, "down_ms": 7, "up_ms": 19})

    def test_a_key_press_adds_nothing(self):
        result, _game, _wall = _run({"start_s": 12.0})
        self.assertEqual(set(result.rows[0]), {
            "index", "chart_ms", "clock_ms_at_press", "lead_ms", "score_before",
            "score_after", "hit"})


# The 8 end chords of Senri's top path and the chord before each, with the
# path's activations, cut from the `hydra_replay score` file of that path. Each
# chord keeps only its index, tick, ms and the three fields of each gem
# load_dump_chords reads.
_SENRI_END_CHORDS = os.path.join(_HERE, "senri_end_chords.json")
_END_CHORD_INDEXES = [268, 566, 819, 1019, 1649, 1843, 2044, 2320]


def _load_senri(lead_ms=play_chart.SP_END_LEAD_MS):
    from unittest import mock
    with mock.patch.object(play_chart, "SP_END_LEAD_MS", lead_ms):
        return play_chart.load_dump_chords(_SENRI_END_CHORDS)


class SpEndLeadTest(unittest.TestCase):
    """A chord on an activation's end is pressed SP_END_LEAD_MS early, and a
    lead its gap to the chord before cannot hold is refused at load."""

    def _gaps(self):
        chords = _load_senri()
        return {chord.index: chord.ms - before.ms
                for before, chord in zip(chords, chords[1:]) if chord.ends_sp}

    def test_the_lead_is_the_number_the_user_set(self):
        self.assertEqual(play_chart.SP_END_LEAD_MS, 15.0)

    def test_the_chords_on_the_deact_ticks_are_marked(self):
        chords = _load_senri()
        self.assertEqual(len(chords), 16)
        self.assertEqual([c.index for c in chords if c.ends_sp], _END_CHORD_INDEXES)

    def test_a_file_with_no_activations_marks_nothing(self):
        self.assertFalse(any(c.ends_sp for c in _load(play_chart.load_dump_chords)))

    def test_a_fake_clock_run_presses_end_chords_early_and_names_the_lead(self):
        wall = _FakeTime()
        game = _FakeGame()
        chords = _load_senri()
        result = play_chart.play_chords(
            chords, _FakeSongClock(wall, start_s=31.0), game.read_score, game.press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), cap_s=300.0)
        self.assertEqual(result.stop, "done")
        self.assertEqual(len(result.rows), 16)
        for row in result.rows:
            want = 15.0 if row["index"] in _END_CHORD_INDEXES else 2.0
            self.assertEqual(row["lead_ms"], want, row["index"])
            # The fake clock moves 1 ms a read, so a press fires inside that
            # read of its lead.
            early_ms = row["chart_ms"] - row["clock_ms_at_press"]
            self.assertLessEqual(early_ms, want, row["index"])
            self.assertGreater(early_ms, want - 1.5, row["index"])

    def test_a_row_with_a_lead_still_writes_the_press_log(self):
        result, _game, _wall = _run({"start_s": 12.0})
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                self.assertEqual(tuple(next(csv.reader(f))), overlay_logs.PRESS_LOG_COLUMNS)

    def test_the_smallest_margin_is_chord_2320s(self):
        gaps = self._gaps()
        self.assertEqual(round(gaps[2320], 1), 68.0)
        self.assertEqual(play_chart.early_side_ms(gaps[2320]), 37.5)
        limits = {i: play_chart.lead_limit_ms(gap) for i, gap in gaps.items()}
        self.assertEqual(round(limits[2320], 1), 30.5)
        self.assertEqual(min(limits, key=limits.get), 2320)

    def test_chord_566s_early_side(self):
        self.assertEqual(round(play_chart.early_side_ms(self._gaps()[566]), 1), 38.8)

    def test_the_clamps_hold_at_both_ends(self):
        self.assertEqual(play_chart.early_side_ms(75.0), 37.5)
        self.assertEqual(play_chart.early_side_ms(170.0), 85.0)
        self.assertEqual(play_chart.early_side_ms(400.0), 85.0)

    def test_a_lead_of_15_loads_on_all_8(self):
        self.assertEqual(len(_load_senri(15.0)), 16)

    def test_a_lead_of_35_is_refused_on_chord_2320(self):
        with self.assertRaises(ValueError) as caught:
            _load_senri(35.0)
        self.assertIn("SP_END_LEAD_MS of 35 ms is too long for chords 2320:",
                      str(caught.exception))

    def test_a_lead_of_40_is_refused_on_566_and_2320_only(self):
        with self.assertRaises(ValueError) as caught:
            _load_senri(40.0)
        self.assertIn("SP_END_LEAD_MS of 40 ms is too long for chords 566, 2320:",
                      str(caught.exception))

    def test_a_lead_equal_to_the_limit_is_refused(self):
        # A lead must be below the limit, so the limit itself fails.
        limit = play_chart.lead_limit_ms(self._gaps()[2320])
        with self.assertRaises(ValueError):
            _load_senri(limit)

    def test_chords_closer_than_the_clamp_load_when_neither_ends_sp(self):
        dump = {"activations": [], "chords": [
            {"index": 0, "tick": 0, "ms": 1000.0, "notes": [_gem("Red", "none")]},
            {"index": 1, "tick": 10, "ms": 1017.241, "notes": [_gem("Green", "none")]}]}
        self.assertEqual(len(_load_dump(dump)), 2)


# Cut from a real target pair of Senri (hydra_replay score and dump, path 0): all
# eight activations, each with the fills it passed over, and five chords. Chord
# 405 is the fill the path passes over. Chords 348 and 458 are fills it does not
# list, and 404 and 406 are the chords either side of 405.
_FILL_SCORE = {
    "activations": [
        {"act_tick": 26880, "deact_tick": 36960}, {"act_tick": 61200, "deact_tick": 74640},
        {"act_tick": 95520, "deact_tick": 107040}, {"act_tick": 126240, "deact_tick": 132960},
        {"act_tick": 193200, "deact_tick": 210000}, {"act_tick": 226800, "deact_tick": 233520},
        {"act_tick": 247920, "deact_tick": 259440}, {"act_tick": 284400, "deact_tick": 294960}],
    "chords": [
        {"index": 348, "ms": 40567.958, "tick": 47040, "is_fill": True,
         "notes": [_gem("Kick", "none"), _gem("Green", "none", cymbal=True)]},
        {"index": 404, "ms": 46878.286, "tick": 54360, "is_fill": False,
         "notes": [_gem("Red", "ghost")]},
        {"index": 405, "ms": 46981.734, "tick": 54480, "is_fill": True,
         "notes": [_gem("Kick", "none"), _gem("Green", "none", cymbal=True)]},
        {"index": 406, "ms": 47395.526, "tick": 54960, "is_fill": False,
         "notes": [_gem("Red", "none")]},
        {"index": 458, "ms": 52774.821999999986, "tick": 61200, "is_fill": True,
         "notes": [_gem("Kick", "none"), _gem("Green", "none", cymbal=True)]},
    ]}
_FILL_DUMP = {"paths": [{"index": 0, "activations": [
    {"act_tick": 26880, "skipped_fill_ticks": []},
    {"act_tick": 61200, "skipped_fill_ticks": [54480]},
    {"act_tick": 95520, "skipped_fill_ticks": []},
    {"act_tick": 126240, "skipped_fill_ticks": []},
    {"act_tick": 193200, "skipped_fill_ticks": []},
    {"act_tick": 226800, "skipped_fill_ticks": []},
    {"act_tick": 247920, "skipped_fill_ticks": []},
    {"act_tick": 284400, "skipped_fill_ticks": []}]}]}


def _with_chord_changes(index, **changes):
    """_FILL_SCORE with the fields of the chord at `index` replaced."""
    return {**_FILL_SCORE, "chords": [{**c, **changes} if c["index"] == index else c
                                      for c in _FILL_SCORE["chords"]]}


def _declined_list(score=_FILL_SCORE, dump=_FILL_DUMP, *args):
    with tempfile.TemporaryDirectory() as d:
        score_path, dump_path = os.path.join(d, "score.json"), os.path.join(d, "dump.json")
        for path, content in ((score_path, score), (dump_path, dump)):
            with open(path, "w", encoding="utf-8") as f:
                json.dump(content, f)
        return play_chart.declined_chord_indexes(score_path, dump_path, *args)


def _load_declining(mode, score=_FILL_SCORE):
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "score.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(score, f)
        return play_chart.load_dump_chords(path, {405: mode})


def _play(chords, misses=()):
    wall = _FakeTime()
    game = _FakeGame(misses)
    out = io.StringIO()
    result = play_chart.play_chords(
        chords, _FakeSongClock(wall, start_s=40.0), game.read_score, game.press,
        now=wall.now, sleep=wall.sleep, out=out)
    return result, game, out.getvalue()


class DeclineListTest(unittest.TestCase):
    """The list of chords to decline, from the path's own record."""

    def test_the_list_from_the_dump_and_score_file_is_chord_405(self):
        self.assertEqual(_declined_list(), [405])

    def test_path_index_picks_the_path(self):
        # A second path, written for this test, passes over chord 348 instead.
        dump = {"paths": _FILL_DUMP["paths"] + [
            {"index": 1, "activations": [{"skipped_fill_ticks": [47040]}]}]}
        self.assertEqual(_declined_list(_FILL_SCORE, dump, 1), [348])
        self.assertEqual(_declined_list(_FILL_SCORE, dump), [405])

    def test_a_path_that_passes_over_nothing_declines_nothing(self):
        dump = {"paths": [{"activations": [{"skipped_fill_ticks": []}]}]}
        self.assertEqual(_declined_list(_FILL_SCORE, dump), [])

    def test_a_tick_with_no_chord_is_refused_by_name(self):
        dump = {"paths": [{"activations": [{"skipped_fill_ticks": [54481]}]}]}
        with self.assertRaises(ValueError) as caught:
            _declined_list(_FILL_SCORE, dump)
        self.assertIn("skipped fill ticks 54481", str(caught.exception))


class DeclineModeTest(unittest.TestCase):
    """What a declined chord sends, and how the run counts it."""

    def test_kick_only_sends_the_kick_lane_alone(self):
        chords = _load_declining(play_chart.DECLINE_KICK_ONLY)
        chord = chords[2]
        self.assertEqual((chord.index, chord.lanes, chord.dynamics, chord.declined),
                         (405, [Lane.KICK], ["none"], True))
        _result, game, _text = _play(chords)
        self.assertEqual(game.presses, [[Lane.KICK, Lane.GREEN_CYMBAL], [Lane.RED], [Lane.KICK],
                                        [Lane.RED], [Lane.KICK, Lane.GREEN_CYMBAL]])

    def test_kick_only_gives_one_strength_for_its_one_lane(self):
        _ThresholdEngine.threshold = 64
        chords, _threshold = play_chart.with_axes(
            _load_declining(play_chart.DECLINE_KICK_ONLY), _ThresholdEngine())
        self.assertEqual(len(chords[2].axes), 1)

    def test_kick_only_sends_one_gem_when_the_chord_has_two_kicks(self):
        score = _with_chord_changes(405, notes=[_gem("Kick", "none"), _gem("Kick", "none"),
                                                _gem("Green", "none", cymbal=True)])
        self.assertEqual(_load_declining(play_chart.DECLINE_KICK_ONLY, score)[2].lanes,
                         [Lane.KICK])

    def test_nothing_sends_no_press_and_the_row_says_declined(self):
        chords = _load_declining(play_chart.DECLINE_NOTHING)
        self.assertEqual(chords[2].lanes, [])
        result, game, text = _play(chords)
        self.assertEqual(game.presses, [[Lane.KICK, Lane.GREEN_CYMBAL], [Lane.RED], [Lane.RED],
                                        [Lane.KICK, Lane.GREEN_CYMBAL]])
        self.assertEqual([(r["index"], r["hit"]) for r in result.rows],
                         [(348, 1), (404, 1), (405, "declined"), (406, 1), (458, 1)])
        self.assertIn("declined", text)

    def test_the_press_log_says_declined_in_the_hit_column(self):
        result, _game, _text = _play(_load_declining(play_chart.DECLINE_NOTHING))
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "press.csv")
            play_chart.write_press_log(path, result.rows)
            with open(path, encoding="utf-8", newline="") as f:
                rows = list(csv.DictReader(f))
        self.assertEqual(rows[2]["hit"], "declined")

    def test_a_declined_chord_is_neither_a_hit_nor_a_miss(self):
        for mode in play_chart.DECLINE_MODES:
            result, _game, _text = _play(_load_declining(mode))
            self.assertEqual((result.sent, result.hit, result.declined, result.stop),
                             (4, 4, 1, "done"), mode)

    def test_a_kick_that_scores_does_not_count_as_a_hit(self):
        result, game, _text = _play(_load_declining(play_chart.DECLINE_KICK_ONLY))
        self.assertEqual(game.score, 500)
        self.assertEqual(result.hit, 4)

    def test_a_declined_chord_leaves_the_streak_alone(self):
        # Press 3 is the declined kick, and it scores nothing.
        result, _game, _text = _play(_load_declining(play_chart.DECLINE_KICK_ONLY), misses=(3,))
        self.assertEqual((result.sent, result.hit, result.max_streak), (4, 4, 4))

    def test_an_unknown_mode_raises(self):
        with self.assertRaises(ValueError) as caught:
            _load_declining("both")
        self.assertEqual(str(caught.exception),
                         'unknown decline mode "both"; modes: kick_only, nothing')

    def test_with_no_declines_every_chord_is_pressed_as_before(self):
        chords = _load_dump(_FILL_SCORE)
        self.assertEqual([c.declined for c in chords], [False] * 5)
        result, game, text = _play(chords)
        self.assertEqual(game.presses[2], [Lane.KICK, Lane.GREEN_CYMBAL])
        self.assertEqual((result.sent, result.hit, result.declined), (5, 5, 0))
        self.assertEqual([r["hit"] for r in result.rows], [1] * 5)
        self.assertNotIn("declined", text)


class DeclineAndLeadTest(unittest.TestCase):
    """A declined chord keeps what load_dump_chords and the lead rule do."""

    def test_no_chord_of_the_fixture_is_both_declined_and_an_end(self):
        self.assertEqual([c.ends_sp for c in _load_declining(play_chart.DECLINE_KICK_ONLY)],
                         [False] * 5)

    def test_a_declined_chord_is_still_the_chord_before_for_the_lead_rule(self):
        # Chord 406 ends Star Power 20 ms after chord 405. A press 15 ms early
        # is refused whether or not 405 sends anything: its note is still there.
        score = _with_chord_changes(406, ms=46981.734 + 20.0, tick=36960)
        for mode in play_chart.DECLINE_MODES:
            with self.assertRaises(ValueError) as caught:
                _load_declining(mode, score)
            self.assertIn("too long for chords 406", str(caught.exception))

    def test_a_declined_chord_on_an_end_keeps_the_end_lead(self):
        score = _with_chord_changes(405, tick=36960)
        chords = _load_declining(play_chart.DECLINE_KICK_ONLY, score)
        self.assertTrue(chords[2].ends_sp and chords[2].declined)
        result, _game, _text = _play(chords)
        self.assertEqual(result.rows[2]["lead_ms"], play_chart.SP_END_LEAD_MS)
        self.assertEqual(result.rows[3]["lead_ms"], play_chart.PRESS_LEAD_MS)


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


if __name__ == "__main__":
    unittest.main()
