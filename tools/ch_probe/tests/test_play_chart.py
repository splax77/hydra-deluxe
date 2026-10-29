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

    def press(self, lanes) -> None:
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


if __name__ == "__main__":
    unittest.main()
