"""Tests for the probe-song layout. No game, no ffmpeg and no hydra_replay run:
the song end comes from Hydra's real dump of the Window Map chart
(testdata/replay/window_map-dump.json), and the charts are checked against
the probe song folders earlier runs wrote (testdata/ch_runs)."""

from __future__ import annotations

import json
import os
import sys
import tempfile
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_files, hydra_end  # noqa: E402
from tools.ch_mcp.tests.test_hydra_end import FAKE, _REAL_MAKE_DUMP  # noqa: E402
from tools.ch_mcp.tests.test_trace_hits import (  # noqa: E402
    CH_RUNS, WINDOW_MAP_CHART, window_map_dump, window_map_dumps)
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import prep_inputs  # noqa: E402
from tools.ch_probe import probe_chart  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.probe_chart import ms_to_ticks, ticks_to_ms  # noqa: E402
from tools.ch_probe.tests.chart_reader import drum_notes, drum_ticks  # noqa: E402

# The Edge Walk song folder probe_songs wrote for the early fill live check's
# prep (hydra-data/ch-mcp-runs/2026-11-12/efl-prep/Edge Walk): its chart and
# song.ini. The Window Map chart is the one the 2026-10-28 l3 runs played
# (WINDOW_MAP_CHART); Hydra's dump of it is test_trace_hits' window_map_dump.
EDGE_WALK_SONG = os.path.join(CH_RUNS, "2026-11-12-efl-prep", P.EDGE_WALK)
RECORDED_CHARTS = {
    P.WINDOW_MAP: WINDOW_MAP_CHART,
    P.EDGE_WALK: os.path.join(EDGE_WALK_SONG, "notes.chart"),
}


def read_text(path: str) -> str:
    with open(path, encoding="utf-8", newline="") as f:
        return f.read()


def song_chart_text(name: str) -> str:
    """The notes.chart text probe_songs writes for the song `name`."""
    t = dict(P.SONGS)[name]()
    return P.chart_text(P.full_name(name), t.notes, t.phrases, t.fills)


def last_note_ms(name: str) -> float:
    return max(n.time_ms for n in dict(P.SONGS)[name]().notes)


# FAKE stays started as a guard: a dump no test sets up fails the way a
# failed hydra_replay dump does, so no test reaches a real hydra_replay.
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
            # The [Song] and [SyncTrack] sections probe_chart's writer makes
            # for this song's resolution, tempo and audio file.
            self.assertIn(probe_chart._song_section("x", P.RESOLUTION, P.SONG_OGG), text)
            self.assertIn(probe_chart._sync_track_section(P.BPM), text)
            # The first note comes after the lead-in.
            self.assertEqual(ticks[0], ms_to_ticks(P.LEAD_IN_MS, P.RESOLUTION, P.BPM))

    def test_song_folder_writer_writes_the_three_files(self):
        # The Window Map chart, whose real hydra_replay dump names its end.
        dump = window_map_dump(self)
        calls, made = [], []
        with tempfile.TemporaryDirectory() as root:
            chart = os.path.join(root, P.WINDOW_MAP, "notes.chart")
            with window_map_dumps(self, made):
                folder, length_ms = P.write_song_folder(
                    root, P.WINDOW_MAP, song_chart_text(P.WINDOW_MAP),
                    write_ogg=lambda path, length_ms: calls.append((path, length_ms)))
            self.assertEqual(folder, os.path.join(root, P.WINDOW_MAP))
            # The chart written is byte for byte the one the dump was made of.
            self.assertEqual(read_text(chart), read_text(WINDOW_MAP_CHART))
            ini = game_files.read_song_ini(folder)
            self.assertEqual(ini["name"], P.full_name(P.WINDOW_MAP))
            self.assertNotIn("song_length", ini)
        # The end is Hydra's, read from a dump of the chart the writer wrote.
        # It is no earlier than the last note, and the silent audio is
        # exactly that long.
        self.assertEqual(made, [chart])
        self.assertEqual(length_ms, hydra_end.song_length_ms(dump))
        self.assertGreaterEqual(length_ms, last_note_ms(P.WINDOW_MAP))
        self.assertEqual(calls, [(os.path.join(folder, P.SONG_OGG), length_ms)])

    def test_song_folder_writer_writes_no_audio_when_hydra_gives_no_end(self):
        # hydra_end's own make_dump, run with an exe that is not there, so the
        # real runner fails before any hydra_replay could start and Hydra
        # gives no end for the chart.
        calls = []
        with tempfile.TemporaryDirectory() as root:
            missing_exe = os.path.join(root, "no-such-hydra_replay.exe")
            with unittest.mock.patch.object(hydra_end, "make_dump", _REAL_MAKE_DUMP), \
                    unittest.mock.patch.object(prep_inputs, "DEFAULT_EXE", missing_exe):
                with self.assertRaises(hydra_end.SongEndMissing) as caught:
                    P.write_song_folder(root, P.WINDOW_MAP, song_chart_text(P.WINDOW_MAP),
                                        write_ogg=lambda *a: calls.append(a))
            self.assertIsInstance(caught.exception.__cause__, prep_inputs.ReplayFailed)
            self.assertFalse(os.path.exists(os.path.join(root, P.WINDOW_MAP, P.SONG_OGG)))
        self.assertEqual(calls, [])

    def test_write_song_returns_hydras_end_and_writes_the_manifest(self):
        dump = window_map_dump(self)
        with tempfile.TemporaryDirectory() as root:
            calls, made = [], []
            with window_map_dumps(self, made):
                folder, length_ms = P.write_song(root, P.WINDOW_MAP, P.window_map(),
                                                 write_ogg=lambda *a: calls.append(a))
            self.assertEqual(read_text(os.path.join(folder, "notes.chart")),
                             read_text(WINDOW_MAP_CHART))
            self.assertEqual(length_ms, hydra_end.song_length_ms(dump))
            self.assertEqual(calls, [(os.path.join(folder, P.SONG_OGG), length_ms)])
            # The manifest names the song and lists the chart's notes by time.
            with open(os.path.join(folder, "manifest.json"), encoding="utf-8") as f:
                written = json.load(f)
        chart_ms = [ticks_to_ms(tick, P.RESOLUTION, P.BPM)
                    for tick in drum_ticks(read_text(WINDOW_MAP_CHART))]
        self.assertEqual([r["time_ms"] for r in written["notes"]], chart_ms)
        self.assertEqual(written["song"], P.full_name(P.WINDOW_MAP))

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

    def test_window_map_and_edge_walk_charts_are_the_ones_earlier_runs_wrote(self):
        # The l3 traces played the recorded Window Map chart and Hydra's dump
        # was made of it; the Edge Walk chart is the one written for the early
        # fill check (RECORDED_CHARTS). A chart written today that differs
        # would not match those recordings, so each must be its file byte for
        # byte, with no Star Power phrase.
        for name, recorded in RECORDED_CHARTS.items():
            text = song_chart_text(name)
            self.assertEqual(text, read_text(recorded), name)
            self.assertNotIn(" = S ", text)

    def test_window_map_and_edge_walk_manifests_have_no_cases(self):
        for build in (P.window_map, P.edge_walk):
            t = build()
            self.assertNotIn("cases", P.manifest("x", t.notes, t.cases))

    def test_song_ini_has_no_delay(self):
        # The song.ini probe_songs writes is the one an earlier run wrote for
        # Edge Walk, and read the way the tools read a song folder, its delay
        # shifts the audio by no ms, so a chart tick is the audio's ms.
        self.assertEqual(P.song_ini(P.full_name(P.EDGE_WALK)),
                         read_text(os.path.join(EDGE_WALK_SONG, "song.ini")))
        self.assertEqual(float(game_files.read_song_ini(EDGE_WALK_SONG)["delay"]), 0.0)


if __name__ == "__main__":
    unittest.main()
