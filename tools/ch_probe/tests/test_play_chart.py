"""play_chart.py plays the chords Hydra analyzed, read from a hydra_replay file.

The chords and times come from a `hydra_replay score` JSON: each entry of its
"chords" list gives the chord's time (`ms`) and its "notes", one per gem, each
with the gem's color and whether it is a cymbal. One gem is one key: pressing
a tom and a cymbal key for one gem is an overhit. Waiting for a note, noticing
the song stopped and picking the first note when a run joins mid-song are
live.py's.
"""

from __future__ import annotations

import json
import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments import live, play_chart  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402


# Eight chord blocks copied whole from a real run of
#   hydra_replay score --chart "testdata/input/common/IB24/T2/Area 11 - Knightmare Frame/notes.mid" --acts ""
# (Expert Pro Drums, 2x Bass), one per kind of pad: yellow cymbal, the 2x
# kick, kick + yellow cymbal + green cymbal, kick + green cymbal, yellow tom,
# green tom, blue cymbal, and a red accent.
_DUMP = {"chords": json.loads(r"""[
{"chord_code": "..N..", "combo_before": 53, "cum": {"accent": 0, "base": 2715, "combo": 5295, "ghost": 0, "solo": 0, "sp": 0, "total": 8010}, "cum_onscreen_total": 8010, "in_sp": false, "index": 38, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 0, "decimal": 10.0, "measure": 10, "tick": 0}, "ms": 13043.48, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Yellow", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "base": 65, "combo": 195, "ghost": 0, "solo": 0, "sp": 0}, "tick": 19200},
{"chord_code": "N....", "combo_before": 56, "cum": {"accent": 0, "base": 2880, "combo": 5790, "ghost": 0, "solo": 0, "sp": 0, "total": 8670}, "cum_onscreen_total": 8670, "in_sp": false, "index": 41, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 1, "decimal": 10.4375, "measure": 10, "tick": 360}, "ms": 13614.132249999999, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "base": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 20040},
{"chord_code": "n.N.N", "combo_before": 63, "cum": {"accent": 0, "base": 3360, "combo": 7230, "ghost": 0, "solo": 0, "sp": 0, "total": 10590}, "cum_onscreen_total": 10590, "in_sp": false, "index": 45, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 3, "decimal": 10.875, "measure": 10, "tick": 240}, "ms": 14184.7845, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}, {"color": "Yellow", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}, {"color": "Green", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "base": 180, "combo": 540, "ghost": 0, "solo": 0, "sp": 0}, "tick": 20880},
{"chord_code": "n...N", "combo_before": 130, "cum": {"accent": 0, "base": 7020, "combo": 18210, "ghost": 0, "solo": 0, "sp": 0, "total": 25230}, "cum_onscreen_total": 25230, "in_sp": false, "index": 91, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 3, "decimal": 16.875, "measure": 16, "tick": 240}, "ms": 22010.872499999998, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Kick", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}, {"color": "Green", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "base": 115, "combo": 345, "ghost": 0, "solo": 0, "sp": 0}, "tick": 32400},
{"chord_code": "..n..", "combo_before": 136, "cum": {"accent": 0, "base": 7270, "combo": 18960, "ghost": 0, "solo": 0, "sp": 0, "total": 26230}, "cum_onscreen_total": 26230, "in_sp": false, "index": 95, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 1, "decimal": 17.375, "measure": 17, "tick": 240}, "ms": 22663.046499999997, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Yellow", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "base": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 33360},
{"chord_code": "....n", "combo_before": 139, "cum": {"accent": 0, "base": 7420, "combo": 19410, "ghost": 0, "solo": 0, "sp": 0, "total": 26830}, "cum_onscreen_total": 26830, "in_sp": false, "index": 97, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 2, "decimal": 17.625, "measure": 17, "tick": 240}, "ms": 22989.133499999996, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Green", "cymbal": false, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 200}], "points": {"accent": 0, "base": 50, "combo": 150, "ghost": 0, "solo": 0, "sp": 0}, "tick": 33840},
{"chord_code": "...N.", "combo_before": 595, "cum": {"accent": 0, "base": 32620, "combo": 95010, "ghost": 0, "solo": 0, "sp": 0, "total": 127630}, "cum_onscreen_total": 127630, "in_sp": false, "index": 400, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 0, "decimal": 60.0, "measure": 60, "tick": 0}, "ms": 78260.88, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Blue", "cymbal": true, "dynamic": "none", "dynamics_bonus": 0, "multiplier": 4, "sp_points": 260}], "points": {"accent": 0, "base": 65, "combo": 195, "ghost": 0, "solo": 0, "sp": 0}, "tick": 115200},
{"chord_code": ".a...", "combo_before": 846, "cum": {"accent": 50, "base": 46325, "combo": 136275, "ghost": 0, "solo": 0, "sp": 0, "total": 182650}, "cum_onscreen_total": 182650, "in_sp": false, "index": 573, "is_fill": false, "is_solo": false, "is_sp_phrase_end": false, "measure": {"beat": 1, "decimal": 83.25, "measure": 83, "tick": 0}, "ms": 108586.97099999999, "multiplier": 4, "multiplier_after": 4, "notes": [{"color": "Red", "cymbal": false, "dynamic": "accent", "dynamics_bonus": 200, "multiplier": 4, "sp_points": 400}], "points": {"accent": 50, "base": 50, "combo": 300, "ghost": 0, "solo": 0, "sp": 0}, "tick": 159840}
]""")}


def _load():
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "dump.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(_DUMP, f)
        return play_chart.load_dump_notes(path)


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


if __name__ == "__main__":
    unittest.main()
