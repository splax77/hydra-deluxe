"""Tests for the Early Fill Walk probe song's layout. Pure text and data: no
game, no ffmpeg and no hydra_replay. The layout's plan is "Which chart" and
"T1" in docs/superpowers/plans/2026-11-12-early-fill-live-check.md; the
numbers pinned here are that plan's."""

from __future__ import annotations

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

# One special-phrase line of [ExpertDrums], stripped: `<tick> = S <code> <length>`.
_S_LINE = re.compile(r"^(\d+)\s*=\s*S\s+(\d+)\s+(\d+)$")

# Four quarter notes at 480 ticks each: the reach of the fill deadline past
# the tick Star Power comes ready (docs/clone-hero/04-fills.md, FL-19).
DEADLINE_TICKS = 1920


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

    def test_five_cases_from_the_plan(self):
        self.assertEqual(P.EFW_CASES,
                         (("A", -30), ("B", 30), ("C", 60), ("D", 70), ("E", 78)))
        # Two phrases, two fills and four notes per case.
        self.assertEqual(len(self.phrases), 10)
        self.assertEqual(len(self.fills), 10)
        self.assertEqual(len(self.notes), 20)

    def test_every_note_is_a_lone_kick(self):
        read = drum_notes(self.text)
        self.assertEqual({(note, sustain) for _t, note, sustain in read},
                         {(C.PROBE_CHART_NOTE_KICK, 0)})
        self.assertEqual(self.notes, sorted(set(self.notes)))
        self.assertEqual(self.notes, [n.time_ms for n in self.t.notes])

    def test_each_phrase_holds_exactly_its_one_note(self):
        # SP-01: start <= tick < start + length.
        for start, length in self.phrases:
            inside = [n for n in self.notes if start <= n < start + length]
            self.assertEqual(inside, [start])
            self.assertEqual(length, 10)
        # The phrase line comes before the note on their shared tick.
        self.assertIn("[ExpertDrums]\n{\n  20000 = S 2 10\n  20000 = N 0 0\n", self.text)

    def test_each_fill_ends_on_a_lone_kick(self):
        # FL-03 and FL-05: a kick exactly at the fill's end is its chord.
        for start, length in self.fills:
            self.assertEqual(length, 480)
            self.assertIn(start + length, self.notes)
            inside = [n for n in self.notes if start <= n <= start + length]
            self.assertEqual(inside, [start + length])

    def test_case_fill_deadline_sits_the_case_distance_before_sp_ready(self):
        sp_ready = [start for start, _len in self.phrases[1::2]]
        case_fills = [start for start, _len in self.fills[0::2]]
        for (label, distance), ready, fill_start in zip(P.EFW_CASES, sp_ready, case_fills):
            with self.subTest(case=label):
                self.assertEqual(fill_start - DEADLINE_TICKS, ready - distance)

    def test_case_distances_pinned(self):
        # The same check as above with the plan's literal distances, so a
        # change to the table cannot pass silently.
        sp_ready = [start for start, _len in self.phrases[1::2]]
        case_fills = [start for start, _len in self.fills[0::2]]
        got = [ready - (start - DEADLINE_TICKS) for ready, start in zip(sp_ready, case_fills)]
        self.assertEqual(got, [-30, 30, 60, 70, 78])

    def test_sp_ready_notes_are_a_second_from_both_neighbours(self):
        sp_ready = [start for start, _len in self.phrases[1::2]]
        for ready in sp_ready:
            i = self.notes.index(ready)
            self.assertGreater(i, 0)
            self.assertLess(i, len(self.notes) - 1)
            self.assertGreaterEqual(ready - self.notes[i - 1], 1000)
            self.assertGreaterEqual(self.notes[i + 1] - ready, 1000)

    def test_lead_in_and_rest_between_cases(self):
        self.assertEqual(self.notes[0], 20000)
        case_firsts = [start for start, _len in self.phrases[0::2]]
        flush_lasts = [start + length for start, length in self.fills[1::2]]
        self.assertEqual(case_firsts[0], 20000)
        for prev_flush, nxt in zip(flush_lasts, case_firsts[1:]):
            self.assertGreaterEqual(nxt - prev_flush, 12000)

    def test_flush_fill_last_note_is_six_seconds_after_the_case_fill(self):
        case_lasts = [start + length for start, length in self.fills[0::2]]
        flush_lasts = [start + length for start, length in self.fills[1::2]]
        for case_last, flush_last in zip(case_lasts, flush_lasts):
            self.assertEqual(flush_last - case_last, 6000)

    def test_manifest_cases_match_the_chart(self):
        cases = P.manifest(P.full_name(P.EARLY_FILL_WALK), self.t.notes,
                           self.t.cases)["cases"]
        self.assertEqual([(c["case"], c["distance_ms"]) for c in cases], list(P.EFW_CASES))
        for i, c in enumerate(cases):
            with self.subTest(case=c["case"]):
                fill_start, fill_len = self.fills[2 * i]
                flush_start, flush_len = self.fills[2 * i + 1]
                self.assertEqual(c["sp_ready_ms"], self.phrases[2 * i + 1][0])
                self.assertEqual(c["fill_start_ms"], fill_start)
                self.assertEqual(c["fill_deadline_ms"], fill_start - DEADLINE_TICKS)
                self.assertEqual(c["fill_last_note_ms"], fill_start + fill_len)
                self.assertEqual(c["flush_last_note_ms"], flush_start + flush_len)

    def test_manifest_first_case_pinned(self):
        case_a = self.t.cases[0]
        self.assertEqual(case_a, {
            "case": "A", "distance_ms": -30, "sp_ready_ms": 22000,
            "fill_start_ms": 23950, "fill_deadline_ms": 22030,
            "fill_last_note_ms": 24430, "flush_last_note_ms": 30430,
        })

    def test_the_song_is_listed(self):
        self.assertEqual(P.EARLY_FILL_WALK, "Early Fill Walk")
        self.assertIn((P.EARLY_FILL_WALK, P.early_fill_walk), P.SONGS)


if __name__ == "__main__":
    unittest.main()
