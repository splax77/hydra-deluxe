"""The probe tests' one reader of .chart drum notes.

The probe writes .chart text (probe_chart.chart_text); the tests read it back
to check what landed where. Both test files read through here, so they agree
on which lines are notes: the note lines inside [ExpertDrums], in file order.

Imported as tools.ch_probe.tests.chart_reader from the repo root.
"""

from __future__ import annotations

import re

# One note line, stripped: `<tick> = N <note> <sustain>`.
_NOTE_LINE = re.compile(r"^(\d+)\s*=\s*N\s+(\d+)\s+(\d+)$")


def drum_notes(text: str) -> list[tuple[int, int, int]]:
    """Every note line of the [ExpertDrums] section as (tick, note, sustain),
    in file order. Lines in other sections are not drum notes."""
    notes: list[tuple[int, int, int]] = []
    in_drums = False
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "[ExpertDrums]":
            in_drums = True
            continue
        if in_drums:
            if stripped == "}":
                break
            match = _NOTE_LINE.match(stripped)
            if match:
                notes.append((int(match.group(1)), int(match.group(2)),
                              int(match.group(3))))
    return notes


def drum_ticks(text: str) -> list[int]:
    """The ticks of drum_notes, in file order."""
    return [tick for tick, _note, _sustain in drum_notes(text)]
