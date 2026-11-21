"""Tests for the probe-chart generator.

All of this runs with no game and no debugger. We generate chart text, parse it
back with a small reader, and check the numbers. That covers the only thing this
module has to get right: notes land the correct number of ticks apart, the
sections appear in order, and no two pairs overlap.

Written to run under pytest (`python -m pytest <path> -q`). It also runs under
plain unittest (`python -m unittest`) because every check is a unittest assert.
"""

from __future__ import annotations

import json
import os
import sys
import unittest

# Make the package importable when this file is run directly (the repo root is
# three levels up from tools/ch_probe/tests/).
_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.probe_chart import (  # noqa: E402
    DEFAULT_SPACINGS_MS,
    build_probe_chart_text,
    chart_text,
    generate_probe_chart,
    ms_to_ticks,
    probe_note_ticks,
    ticks_to_ms,
)
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_chart, probe_songs  # noqa: E402
# The round-trip half: the one test reader of the notes the generator wrote.
from tools.ch_probe.tests.chart_reader import drum_notes, drum_ticks  # noqa: E402


class TestMsToTicks(unittest.TestCase):
    def test_quarter_note_worth_of_ms(self):
        # At 120 bpm a quarter note is 500 ms and must be exactly `resolution`
        # ticks, whatever the resolution.
        for res in (192, 480, 96):
            self.assertEqual(ms_to_ticks(500, res, 120.0), res)

    def test_one_second_at_various_bpm(self):
        for res, bpm in ((480, 120.0), (192, 200.0)):
            quarter_notes_per_second = bpm / 60
            self.assertEqual(ms_to_ticks(1000, res, bpm),
                             quarter_notes_per_second * res,
                             f"one second at {res} ticks per quarter note, {bpm} bpm")

    def test_rounds_to_nearest(self):
        # One input whose exact tick count has a fraction above a half and one
        # below, so rounding down and rounding up each fail one of them.
        res, bpm = 192, 120.0
        quarter_note_ms = 60000 / bpm
        for ms in (30, 100):
            exact = ms / quarter_note_ms * res
            self.assertNotEqual(exact, int(exact), f"{ms} ms lands on a whole tick")
            self.assertLessEqual(abs(ms_to_ticks(ms, res, bpm) - exact), 0.5,
                                 f"{ms} ms is {exact} ticks exactly")

    def test_ticks_to_ms_inverts_ms_to_ticks(self):
        self.assertEqual(ticks_to_ms(1, 480, 125.0), 1.0)
        self.assertEqual(ticks_to_ms(960, 480, 120.0), 1000.0)
        for res in (192, 480, 96):
            self.assertEqual(ticks_to_ms(ms_to_ticks(500, res, 120.0), res, 120.0), 500.0)


# A probe chart Clone Hero loaded and played to the end in run 2026-10-28-l3
# (its trace-wm-count.txt says "song over"), and that Hydra scored for
# testdata/replay/window_map-score.json. Its manifest.json is the one
# probe_songs wrote beside it: the song name, resolution and tempo.
_PLAYED_SONG_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-10-28-l3",
                                probe_songs.WINDOW_MAP)


def _played_chart() -> tuple[str, dict]:
    """The played chart's text, byte for byte, and its manifest."""
    with open(os.path.join(_PLAYED_SONG_DIR, "notes.chart"), encoding="utf-8",
              newline="") as handle:
        text = handle.read()
    with open(os.path.join(_PLAYED_SONG_DIR, "manifest.json"), encoding="utf-8") as handle:
        manifest = json.load(handle)
    return text, manifest


def _played_chart_inputs(text: str, manifest: dict) -> dict:
    """The inputs chart_text needs to write the played chart again: its notes
    as the chart holds them, and the name, resolution and tempo of its run."""
    notes = drum_notes(text)
    kinds = {kind for _tick, kind, _sustain in notes}
    if len(kinds) != 1:
        raise AssertionError(f"the played chart holds note kinds {sorted(kinds)}")
    return dict(name=manifest["song"], note_ticks=[tick for tick, _k, _s in notes],
                resolution=manifest["resolution"], bpm=manifest["bpm"],
                note=kinds.pop())


class TestChartTextIsAChartTheGamePlayed(unittest.TestCase):
    def test_writes_the_played_chart_line_for_line(self):
        # The sections, their order, the Resolution and tempo lines, the
        # MusicStream line and every brace: any change to one makes this text
        # differ from a chart the game and Hydra both read.
        played, manifest = _played_chart()
        written = chart_text(**_played_chart_inputs(played, manifest),
                             music_stream=probe_songs.SONG_OGG)
        self.assertEqual(written.splitlines(), played.splitlines())
        self.assertEqual(written, played)

    def test_no_stream_drops_only_the_stream_line(self):
        played, manifest = _played_chart()
        inputs = _played_chart_inputs(played, manifest)
        with_stream = chart_text(**inputs, music_stream=probe_songs.SONG_OGG).splitlines()
        without = chart_text(**inputs, music_stream=None).splitlines()
        dropped = [line for line in with_stream if line not in without]
        self.assertEqual(len(dropped), 1, f"lines dropped: {dropped}")
        self.assertIn(probe_songs.SONG_OGG, dropped[0])
        self.assertEqual([line for line in with_stream if line != dropped[0]], without)


class TestPairsAndSpacing(unittest.TestCase):
    def test_two_notes_per_spacing(self):
        spacings = [50, 100, 200]
        ticks = drum_ticks(build_probe_chart_text(spacings, resolution=192, bpm=120.0))
        self.assertEqual(len(ticks), 2 * len(spacings))

    def test_each_pair_is_the_right_delta_apart(self):
        spacings = list(DEFAULT_SPACINGS_MS)
        res, bpm = 192, 120.0
        ticks = drum_ticks(
            build_probe_chart_text(spacings, resolution=res, bpm=bpm)
        )
        for idx, ms in enumerate(spacings):
            first = ticks[2 * idx]
            second = ticks[2 * idx + 1]
            self.assertEqual(
                second - first,
                ms_to_ticks(ms, res, bpm),
                f"spacing {ms} ms produced the wrong tick delta",
            )

    def test_pairs_do_not_overlap(self):
        # Every note must sit strictly after the previous one, and the silence
        # between one pair's second note and the next pair's first note must be
        # wide enough that the game gives both of them the full window:
        # CAP_FROM_GAP_MS, the gap at which a note's window reaches its cap.
        spacings = list(DEFAULT_SPACINGS_MS)
        res, bpm = 192, 120.0
        ticks = drum_ticks(
            build_probe_chart_text(spacings, resolution=res, bpm=bpm)
        )
        self.assertEqual(ticks, sorted(ticks))
        # Between-pair gap = ticks[2] - ticks[1], ticks[4] - ticks[3], ...
        for idx in range(1, len(spacings)):
            prev_second = ticks[2 * idx - 1]
            this_first = ticks[2 * idx]
            silence_ms = ticks_to_ms(this_first - prev_second, res, bpm)
            self.assertGreaterEqual(
                silence_ms,
                C.CAP_FROM_GAP_MS,
                f"pair {idx} starts {silence_ms} ms after pair {idx - 1} ends, "
                "so the game narrows their windows",
            )

    def test_ticks_are_strictly_increasing(self):
        ticks = drum_ticks(build_probe_chart_text([300, 30, 211], resolution=192, bpm=120.0))
        for a, b in zip(ticks, ticks[1:]):
            self.assertLess(a, b)

    def test_probe_note_ticks_are_the_written_ticks(self):
        res, bpm = 480, 125.0
        whole_note = 4 * res  # four quarter notes of `res` ticks each
        lead_in = probe_chart._LEAD_IN_WHOLE_NOTES * whole_note
        pad = probe_chart._PAD_WHOLE_NOTES * whole_note
        first_gap = ms_to_ticks(211, res, bpm)
        second_gap = ms_to_ticks(30, res, bpm)
        ticks = probe_note_ticks([211, 30], resolution=res, bpm=bpm)
        self.assertEqual(ticks, [
            lead_in,
            lead_in + first_gap,
            lead_in + first_gap + pad,
            lead_in + first_gap + pad + second_gap,
        ])
        text = build_probe_chart_text([211, 30], resolution=res, bpm=bpm)
        self.assertEqual(drum_ticks(text), ticks)


class TestFileWrite(unittest.TestCase):
    def test_generate_writes_a_readable_chart(self):
        import tempfile

        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "probe.chart")
            generate_probe_chart([100, 200], path, resolution=192, bpm=120.0)
            with open(path, "r", encoding="utf-8") as handle:
                text = handle.read()
            ticks = drum_ticks(text)
            self.assertEqual(len(ticks), 4)
            self.assertEqual(ticks[1] - ticks[0], ms_to_ticks(100, 192, 120.0))
            self.assertEqual(ticks[3] - ticks[2], ms_to_ticks(200, 192, 120.0))


if __name__ == "__main__":
    unittest.main()
