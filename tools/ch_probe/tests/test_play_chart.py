"""play_chart.py plays the chords Hydra analyzed, read from a dump file.

The notes, lanes and times come from a `hydra_replay dump` JSON: each entry of
its "chords" list gives the chord's time (`ms`) and the pads it hits
(`lanes`, written by hydra_replay from the C++ chord: each note's color and
whether it is a cymbal). One gem is one key: pressing a tom and a cymbal key
for one gem is an overhit. Waiting for a note, noticing the song stopped and
picking the first note when a run joins mid-song are live.py's.
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


def _pad(color, cymbal=False):
    return {"color": color, "cymbal": cymbal}


# Only the keys play_chart reads; a real dump carries many more per chord.
# Each "lanes" list is what hydra_replay's lanes_json writes for that chord.
_DUMP = {"chords": [
    {"ms": 0.0, "lanes": [_pad("Yellow")]},                  # yellow tom
    {"ms": 500.0, "lanes": [_pad("Yellow", True)]},          # yellow cymbal
    {"ms": 1000.0, "lanes": [_pad("Blue", True)]},           # blue cymbal
    {"ms": 1500.0, "lanes": [_pad("Green", True)]},          # green cymbal
    {"ms": 2000.0, "lanes": [_pad("Kick")]},                 # 2x kick
    {"ms": 2500.0, "lanes": [_pad("Kick"), _pad("Red")]},    # red + kick
    {"ms": 3000.0, "lanes": [_pad("Green")]},                # green accent
    {"ms": 3500.0, "lanes": []},                             # nothing to press
]}


def _load():
    with tempfile.TemporaryDirectory() as d:
        path = os.path.join(d, "dump.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(_DUMP, f)
        return play_chart.load_dump_notes(path)


class DumpPathTest(unittest.TestCase):
    def test_dump_chords_become_timed_lanes(self):
        self.assertEqual([(ms, list(lanes)) for ms, lanes in _load()], [
            (0.0, [Lane.YELLOW]),
            (500.0, [Lane.YELLOW_CYMBAL]),
            (1000.0, [Lane.BLUE_CYMBAL]),
            (1500.0, [Lane.GREEN_CYMBAL]),
            (2000.0, [Lane.KICK]),             # the 2x kick presses L
            (2500.0, [Lane.KICK, Lane.RED]),
            (3000.0, [Lane.GREEN]),            # an accent is still the green pad
        ])

    def test_chord_codes_are_not_decoded_here(self):
        # The dump says the pads; play_chart never reads the ADR 0015 code.
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
