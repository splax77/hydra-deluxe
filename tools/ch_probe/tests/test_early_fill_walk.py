"""Tests for the Early Fill Walk probe song's layout. Pure text and data: no
game, no ffmpeg and no hydra_replay run. Every expected number is read from
probe_songs' EFW_ table, the generator's own layout constants.

Which note each Star Power phrase holds and which chord each fill lands on is
Hydra's engine's answer, not the test's: the engine tests read Hydra's score
JSON for the real Early Fill Walk chart from the 2026-11-12 efl-prep run."""

from __future__ import annotations

import json
import os
import re
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.tests.chart_reader import drum_notes, drum_ticks  # noqa: E402

# The run folder copied from hydra-data/ch-mcp-runs/2026-11-12/efl-prep.
# "Early Fill Walk/notes.chart" is the chart that run wrote with
# `py -m tools.ch_probe.probe_songs --out <run folder>`; score.json is
# hydra_replay's score of that chart (path 1 of the run's dump.json).
_EFL_DIR = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "efl-prep")
_EFL_CHART = os.path.join(_EFL_DIR, "Early Fill Walk", "notes.chart")
_EFL_SCORE = os.path.join(_EFL_DIR, "score.json")

# One special-phrase line of [ExpertDrums], stripped: `<tick> = S <code> <length>`.
_S_LINE = re.compile(r"^(\d+)\s*=\s*S\s+(\d+)\s+(\d+)$")


def _ms(ticks: float) -> float:
    """Chart ticks in ms, at the song's own resolution and tempo."""
    return P.ticks_to_ms(ticks, P.RESOLUTION, P.BPM)


def _specials(text: str, code: int) -> list[tuple[int, int]]:
    """Every `S <code>` line of [ExpertDrums] as (tick, length), in file order."""
    out = []
    in_drums = False
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "[ExpertDrums]":
            in_drums = True
            continue
        if in_drums:
            if stripped == "}":
                break
            m = _S_LINE.match(stripped)
            if m and int(m.group(2)) == code:
                out.append((int(m.group(1)), int(m.group(3))))
    return out


class EarlyFillWalkTest(unittest.TestCase):
    def setUp(self):
        self.t = P.early_fill_walk()
        self.text = P.chart_text(P.full_name(P.EARLY_FILL_WALK), self.t.notes,
                                 self.t.phrases, self.t.fills)
        self.notes = drum_ticks(self.text)
        self.phrases = _specials(self.text, 2)
        self.fills = _specials(self.text, 64)

    def test_each_case_writes_two_phrases_two_fills_and_four_notes(self):
        cases = len(P.EFW_CASES)
        self.assertGreater(cases, 0)
        self.assertEqual(len(self.phrases), 2 * cases)
        self.assertEqual(len(self.fills), 2 * cases)
        self.assertEqual(len(self.notes), 4 * cases)

    def test_every_note_is_a_lone_kick(self):
        read = drum_notes(self.text)
        self.assertEqual({(note, sustain) for _t, note, sustain in read},
                         {(C.PROBE_CHART_NOTE_KICK, 0)})
        self.assertEqual(self.notes, sorted(set(self.notes)))
        self.assertEqual(self.notes, [n.time_ms for n in self.t.notes])

    def test_each_phrase_line_comes_before_the_note_it_starts_on(self):
        lines = [line.strip() for line in self.text.splitlines()]
        for start, length in self.phrases:
            with self.subTest(tick=start):
                phrase_line = lines.index(f"{start} = S 2 {length}")
                note_line = lines.index(f"{start} = N {C.PROBE_CHART_NOTE_KICK} 0")
                self.assertLess(phrase_line, note_line)

    def test_case_fill_deadline_sits_the_case_distance_before_sp_ready(self):
        deadline_ms = _ms(P.EFW_DEADLINE_BEATS * P.RESOLUTION)
        sp_ready = [start for start, _len in self.phrases[1::2]]
        case_fills = [start for start, _len in self.fills[0::2]]
        self.assertEqual(len(case_fills), len(P.EFW_CASES))
        for (label, distance), ready, fill_start in zip(P.EFW_CASES, sp_ready, case_fills):
            with self.subTest(case=label):
                self.assertEqual(_ms(fill_start) - deadline_ms, _ms(ready) - distance)

    def test_sp_ready_note_follows_its_phrase_note_and_precedes_its_fill(self):
        # The note before each SP-ready note is its case's first phrase note,
        # the phrase gap earlier; the note after it is the case fill's end.
        sp_ready = [start for start, _len in self.phrases[1::2]]
        first_phrases = [start for start, _len in self.phrases[0::2]]
        case_fill_ends = [start + length for start, length in self.fills[0::2]]
        for ready, phrase, fill_end in zip(sp_ready, first_phrases, case_fill_ends):
            with self.subTest(tick=ready):
                i = self.notes.index(ready)
                self.assertEqual(self.notes[i - 1], phrase)
                self.assertEqual(self.notes[i + 1], fill_end)
                self.assertEqual(_ms(ready) - _ms(phrase), P.EFW_PHRASE_GAP_MS)

    def test_lead_in_and_rest_between_cases(self):
        self.assertEqual(_ms(self.notes[0]), P.EFW_LEAD_IN_MS)
        case_firsts = [start for start, _len in self.phrases[0::2]]
        flush_lasts = [start + length for start, length in self.fills[1::2]]
        self.assertEqual(case_firsts[0], self.notes[0])
        for prev_flush, nxt in zip(flush_lasts, case_firsts[1:]):
            self.assertEqual(_ms(nxt) - _ms(prev_flush), P.EFW_REST_MS)

    def test_flush_fill_last_note_comes_the_flush_gap_after_the_case_fill(self):
        case_lasts = [start + length for start, length in self.fills[0::2]]
        flush_lasts = [start + length for start, length in self.fills[1::2]]
        self.assertEqual(len(flush_lasts), len(P.EFW_CASES))
        for case_last, flush_last in zip(case_lasts, flush_lasts):
            self.assertEqual(_ms(flush_last) - _ms(case_last), P.EFW_FLUSH_AFTER_MS)

    def test_manifest_cases_match_the_chart(self):
        deadline_ms = _ms(P.EFW_DEADLINE_BEATS * P.RESOLUTION)
        cases = P.manifest(P.full_name(P.EARLY_FILL_WALK), self.t.notes,
                           self.t.cases)["cases"]
        self.assertEqual([(c["case"], c["distance_ms"]) for c in cases], list(P.EFW_CASES))
        for i, c in enumerate(cases):
            with self.subTest(case=c["case"]):
                fill_start, fill_len = self.fills[2 * i]
                flush_start, flush_len = self.fills[2 * i + 1]
                self.assertEqual(c["sp_ready_ms"], _ms(self.phrases[2 * i + 1][0]))
                self.assertEqual(c["fill_start_ms"], _ms(fill_start))
                self.assertEqual(c["fill_deadline_ms"], _ms(fill_start) - deadline_ms)
                self.assertEqual(c["fill_last_note_ms"], _ms(fill_start + fill_len))
                self.assertEqual(c["flush_last_note_ms"], _ms(flush_start + flush_len))

    def test_the_song_is_listed(self):
        self.assertIn((P.EARLY_FILL_WALK, P.early_fill_walk), P.SONGS)


class EarlyFillWalkEngineTest(unittest.TestCase):
    """Hydra's engine read of the real Early Fill Walk chart, against the
    layout the generator writes today."""

    @classmethod
    def setUpClass(cls):
        with open(_EFL_CHART, encoding="utf-8") as f:
            cls.recorded = f.read()
        with open(_EFL_SCORE, encoding="utf-8") as f:
            cls.chords = json.load(f)["chords"]
        t = P.early_fill_walk()
        cls.text = P.chart_text(P.full_name(P.EARLY_FILL_WALK), t.notes,
                                t.phrases, t.fills)
        cls.phrases = _specials(cls.text, 2)
        cls.fills = _specials(cls.text, 64)

    def test_the_recorded_chart_is_the_generators_chart(self):
        # The engine facts below are about the recorded chart. If the
        # generator's layout changes, this fails: write the song again with
        # probe_songs and score it again with hydra_replay.
        self.assertEqual(self.recorded, self.text)

    def test_engine_chord_times_match_the_charts_tempo(self):
        self.assertEqual(len(self.chords), len(drum_ticks(self.recorded)))
        for chord in self.chords:
            with self.subTest(tick=chord["tick"]):
                self.assertEqual(chord["ms"], _ms(chord["tick"]))

    def test_each_phrase_ends_on_the_one_note_it_starts_on(self):
        # A phrase that starts on a note and has its last chord there holds
        # that one note only.
        ends = [c["tick"] for c in self.chords if c["is_sp_phrase_end"]]
        self.assertEqual(ends, [start for start, _len in self.phrases])

    def test_each_fill_lands_on_the_kick_at_its_end(self):
        landed = [(c["fill_start"], c["tick"]) for c in self.chords if c["is_fill"]]
        self.assertEqual(landed, [(start, start + length) for start, length in self.fills])


if __name__ == "__main__":
    unittest.main()
