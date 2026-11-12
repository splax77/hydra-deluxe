"""Tests for the probe-song layout. Pure text/data; no game, no ffmpeg, and no
hydra_replay: the song end comes from test_hydra_end's fake dumps."""

from __future__ import annotations

import os
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.probe_chart import ms_to_ticks, ticks_to_ms  # noqa: E402
from tools.ch_probe.tests.chart_reader import drum_notes  # noqa: E402


def setUpModule():
    FAKE.start()


def tearDownModule():
    FAKE.stop()


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
            # The [Song] and [SyncTrack] lines probe_chart's writer puts out.
            self.assertIn("  Resolution = 480\n", text)
            self.assertIn("  0 = B 125000\n", text)
            self.assertIn('  MusicStream = "song.ogg"\n', text)
        edge = P.chart_text("x", P.edge_walk().notes)
        self.assertIn("[ExpertDrums]\n{\n  3000 = N 0 0\n", edge)

    def test_song_folder_writer_writes_the_three_files(self):
        calls = []
        with tempfile.TemporaryDirectory() as root:
            chart = os.path.join(root, "x", "notes.chart")
            # The fake dump of the chart it writes names a song end of 4,000 ms.
            FAKE.set_end(chart, 4000.0)
            folder, length_ms = P.write_song_folder(
                root, "x", "chart text",
                write_ogg=lambda path, length_ms: calls.append((path, length_ms)))
            self.assertEqual(folder, os.path.join(root, "x"))
            self.assertEqual(length_ms, 4000.0)
            with open(chart, encoding="utf-8") as f:
                self.assertEqual(f.read(), "chart text")
            with open(os.path.join(folder, "song.ini"), encoding="utf-8") as f:
                self.assertNotIn("song_length", f.read())
        # The end is read from a dump of the chart the writer wrote, and the
        # silent audio is exactly that long.
        self.assertEqual(FAKE.charts[-1], chart)
        self.assertEqual(calls, [(os.path.join(root, "x", P.SONG_OGG), 4000.0)])

    def test_song_folder_writer_writes_no_audio_when_hydra_names_no_end(self):
        calls = []
        with tempfile.TemporaryDirectory() as root:
            FAKE.set_end(os.path.join(root, "x", "notes.chart"), None)
            with self.assertRaises(hydra_end.SongEndMissing):
                P.write_song_folder(root, "x", "chart text",
                                    write_ogg=lambda *a: calls.append(a))
        self.assertEqual(calls, [])

    def test_write_song_returns_hydras_end_and_writes_the_manifest(self):
        with tempfile.TemporaryDirectory() as root:
            FAKE.set_end(os.path.join(root, P.EDGE_WALK, "notes.chart"), 123000.0)
            calls = []
            folder, length_ms = P.write_song(root, P.EDGE_WALK, P.edge_walk(),
                                             write_ogg=lambda *a: calls.append(a))
            self.assertEqual(length_ms, 123000.0)
            self.assertEqual(calls, [(os.path.join(folder, P.SONG_OGG), 123000.0)])
            self.assertTrue(os.path.isfile(os.path.join(folder, "manifest.json")))

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
        self.assertIn("delay = 0\n", P.song_ini("x"))


if __name__ == "__main__":
    unittest.main()
