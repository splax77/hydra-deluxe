"""Tests for the probe-song layout. Pure text/data; no game, no ffmpeg."""

from __future__ import annotations

import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.probe_chart import ms_to_ticks, ticks_to_ms  # noqa: E402
from tools.ch_probe.tests.chart_reader import drum_notes  # noqa: E402


class ProbeSongsTest(unittest.TestCase):
    def test_one_tick_is_one_ms(self):
        self.assertEqual(ms_to_ticks(1, P.RESOLUTION, P.BPM), 1)
        self.assertEqual(ticks_to_ms(1, P.RESOLUTION, P.BPM), 1.0)

    def test_chart_ticks_match_manifest_times(self):
        for build in (P.window_map, P.edge_walk):
            notes = build().notes
            text = P.chart_text("x", notes)
            read = drum_notes(text)
            ticks = [tick for tick, _note, _sustain in read]
            self.assertEqual(ticks, [n.time_ms for n in notes])
            self.assertEqual(ticks, sorted(set(ticks)))
            # Every note is the kick, with no sustain.
            self.assertEqual({(note, sustain) for _tick, note, sustain in read},
                             {(C.PROBE_CHART_NOTE_KICK, 0)})
            # Bytes from the base's own writer, before the fold to probe_chart.
            self.assertIn("  Resolution = 480\n", text)
            self.assertIn("  0 = B 125000\n", text)
            self.assertIn('  MusicStream = "song.ogg"\n', text)
        edge = P.chart_text("x", P.edge_walk().notes)
        self.assertIn("[ExpertDrums]\n{\n  3000 = N 0 0\n", edge)

    def test_song_folder_writer_writes_the_three_files(self):
        calls = []
        with tempfile.TemporaryDirectory() as root:
            folder = P.write_song_folder(
                root, "x", "chart text", 4000,
                write_ogg=lambda path, length_ms: calls.append((path, length_ms)))
            self.assertEqual(folder, os.path.join(root, "x"))
            with open(os.path.join(folder, "notes.chart"), encoding="utf-8") as f:
                self.assertEqual(f.read(), "chart text")
            with open(os.path.join(folder, "song.ini"), encoding="utf-8") as f:
                self.assertIn("song_length = 4000\n", f.read())
        self.assertEqual(calls, [(os.path.join(root, "x", "song.ogg"), 4000)])
        self.assertEqual(P.song_length_ms(1000), 1000 + P.SILENCE_MS)

    def test_song_names_are_spelled_once(self):
        self.assertEqual(P.WINDOW_MAP, "Window Map")
        self.assertEqual(P.EDGE_WALK, "Edge Walk")
        self.assertEqual(P.NAME_PREFIX, "Hydra Probe - ")

    def test_run_notes_have_the_same_gap_both_sides(self):
        rows = P.manifest("x", P.window_map().notes)["notes"]
        for gap in P.CAP_GAPS_MS + P.FLOOR_GAPS_MS:
            label = "cap" if gap in P.CAP_GAPS_MS else "floor"
            run = [r for r in rows if r["block"] == f"{label}_{gap}" and r["role"] == "run"]
            self.assertEqual(len(run), P.RUN_NOTES)
            for r in run[:-1]:
                self.assertEqual(r["gap_before_ms"], gap)
                self.assertEqual(r["gap_after_ms"], gap)

    def test_uneven_middle_notes(self):
        rows = P.manifest("x", P.window_map().notes)["notes"]
        for before, after in P.UNEVEN_GAPS_MS:
            mid = [r for r in rows if r["block"] == f"uneven_{before}_{after}"
                   and r["role"] == "middle"]
            self.assertEqual(len(mid), 1)
            self.assertEqual((mid[0]["gap_before_ms"], mid[0]["gap_after_ms"]),
                             (before, after))

    def test_blocks_are_separated_by_silence(self):
        rows = P.manifest("x", P.window_map().notes)["notes"]
        for prev, cur in zip(rows, rows[1:]):
            if prev["block"] != cur["block"]:
                self.assertGreaterEqual(cur["gap_before_ms"], P.SILENCE_MS)

    def test_edge_walk_is_isolated(self):
        rows = P.manifest("x", P.edge_walk().notes)["notes"]
        self.assertEqual(len(rows), P.EDGE_WALK_NOTES)
        self.assertTrue(all(r["gap_before_ms"] in (None, P.EDGE_WALK_GAP_MS) for r in rows))

    def test_song_ini_has_no_delay(self):
        self.assertIn("delay = 0\n", P.song_ini("x", 1000))


if __name__ == "__main__":
    unittest.main()
