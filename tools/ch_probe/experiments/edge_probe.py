"""The window-edge probe: play a chart the way the auto-player does, but
press a few named chords at a lead the caller picks, so a run can find where
the game's hit window really ends and which note a press takes when two notes
on one pad are both in reach.

Design point 8 of docs/superpowers/plans/2026-11-07-player-limits.md says why
it exists: TM-26 and TM-27 on docs/clone-hero/03-timing.md are open, and no
page says which of two same-pad notes in window a press hits. The older
tools/ch_probe/experiments/walk_edges.py (removed;
`git show a081f660:tools/ch_probe/experiments/walk_edges.py`) played the same
chart, but it sent keys to the focused game, which the 2026-10-30 ruling rules
out, and it wrote no result file.

Nothing here times a press. The schedule (play_chart.load_schedule) times
every press; this module only moves a probe chord and its presses so the
schedule's own first press for it fires at the probe's lead, checks that the
move changed nothing else, and puts the chord's real time back on its press
row. The move carries each press's `last_send_ms` with it, so play_chords
never drops a probe press: pressing past the window is the point.

The probes file is a JSON list of {"index": <chord index>, "lead_ms": <ms>}.
A positive lead presses early, a negative one late, as play_chart's leads do.
"""

from __future__ import annotations

import dataclasses
import json
from typing import Dict, List, Sequence, Tuple

from tools.ch_probe.experiments import play_chart

# The press-row column that marks a probe press. It holds the lead the probe
# asked for, so a read of the log can never take a probe row for a plain one.
PROBE_COLUMN = "probe"

# How close press_plan's moved press must land to the asked time. The move is
# one subtraction, so anything past float rounding means another rule timed
# the chord.
_SAME_MS = 1e-6


@dataclasses.dataclass(frozen=True)
class Probe:
    """One probe: the chord's `index` in the score file and the `lead_ms` its
    press gets (positive early, negative late)."""
    index: int
    lead_ms: float


@dataclasses.dataclass(frozen=True)
class ProbeRun:
    """What a probe run needs after the chords are moved: each probe chord's
    asked lead and how far its time was moved, by its `index`."""
    leads: Dict[int, float]
    shifts: Dict[int, float]

    def mark_row(self, row: dict) -> None:
        """Put a probe chord's real time back on its press row, with the lead
        the press really used against it, and mark the row with the asked
        lead in PROBE_COLUMN. Other rows are left as they are."""
        shift = self.shifts.get(row.get("index"))
        if shift is None:
            return
        row["chart_ms"] = row["chart_ms"] + shift
        if "lead_ms" in row:
            row["lead_ms"] = row["lead_ms"] + shift
        row[PROBE_COLUMN] = self.leads[row["index"]]


def load_probes(path: str) -> List[Probe]:
    """The probes in the file at `path`. Raises ValueError for a file that is
    not a non-empty list of {"index", "lead_ms"} entries, or that names one
    chord twice."""
    with open(path, encoding="utf-8") as f:
        raw = json.load(f)
    if not isinstance(raw, list) or not raw:
        raise ValueError(f'{path} holds no probes: it must be a list of '
                         f'{{"index": <chord index>, "lead_ms": <ms>}}')
    probes = []
    for entry in raw:
        index = entry.get("index") if isinstance(entry, dict) else None
        lead = entry.get("lead_ms") if isinstance(entry, dict) else None
        if (type(index) is not int or isinstance(lead, bool)
                or not isinstance(lead, (int, float))):
            raise ValueError(f'{path}: {json.dumps(entry)} is not '
                             f'{{"index": <chord index>, "lead_ms": <ms>}}')
        probes.append(Probe(index, float(lead)))
    twice = sorted({p.index for p in probes if [q.index for q in probes].count(p.index) > 1})
    if twice:
        raise ValueError(f"{path} names chords {', '.join(map(str, twice))} more than once")
    return probes


def _first_presses(chords: Sequence[play_chart.Chord]) -> Dict[int, play_chart.Press]:
    """Each chord's first press in press_plan's firing order, by `index`."""
    first: Dict[int, play_chart.Press] = {}
    for press in play_chart.press_plan(chords):
        first.setdefault(press.index, press)
    return first


def _fire_times(chords: Sequence[play_chart.Chord]) -> Dict[int, List[float]]:
    """Every press time press_plan gives each chord, by `index`."""
    times: Dict[int, List[float]] = {}
    for press in play_chart.press_plan(chords):
        times.setdefault(press.index, []).append(press.at_ms)
    return times


def probe_chords(chords: Sequence[play_chart.Chord],
                 probes: Sequence[Probe]) -> Tuple[List[play_chart.Chord], ProbeRun]:
    """`chords` with each probe chord moved so press_plan presses it at the
    probe's lead, and the ProbeRun that puts the real times back on the rows.
    Raises ValueError for a probe chord not in `chords`, for one with a press
    the schedule times from a Star Power end (Press.timed_from_end) or that
    the move does not land at the probe's lead, and for a move that would
    shift another chord's press."""
    by_index = {chord.index: chord for chord in chords}
    missing = [p.index for p in probes if p.index not in by_index]
    if missing:
        raise ValueError(f"probe chords {', '.join(map(str, missing))} are not in the "
                         f"score file")
    from_end = sorted(p.index for p in probes
                      if any(press.timed_from_end is not None
                             for press in by_index[p.index].presses))
    if from_end:
        raise ValueError(_timed_by_another_rule(from_end))
    first = _first_presses(chords)
    leads = {p.index: p.lead_ms for p in probes}
    # A press fires at its chord's time plus its aim, so moving the chord and
    # its presses by (the first press's lead - the probe's) moves that press
    # to the probe's lead.
    shifts = {p.index: p.lead_ms - first[p.index].lead_ms for p in probes}
    moved = [_moved(chord, shifts[chord.index]) if chord.index in shifts else chord
             for chord in chords]

    before, after = _fire_times(chords), _fire_times(moved)
    wrong = sorted(index for index in after
                   if index in shifts
                   and abs(after[index][0] - (by_index[index].ms - leads[index])) > _SAME_MS)
    if wrong:
        raise ValueError(_timed_by_another_rule(wrong))
    changed = sorted(index for index in before
                     if index not in shifts
                     and (len(before[index]) != len(after.get(index, []))
                          or any(abs(a - b) > _SAME_MS
                                 for a, b in zip(before[index], after[index]))))
    if changed:
        raise ValueError(f"the probes would move the presses of chords "
                         f"{', '.join(map(str, changed))}, which are not probes")
    return moved, ProbeRun(leads, shifts)


def _moved(chord: play_chart.Chord, shift_ms: float) -> play_chart.Chord:
    """`chord` and its presses `shift_ms` earlier: the chord's `ms`, and each
    press's `chart_ms` and `last_send_ms`."""
    return dataclasses.replace(
        chord, ms=chord.ms - shift_ms,
        presses=tuple(dataclasses.replace(p, chart_ms=p.chart_ms - shift_ms,
                                          last_send_ms=p.last_send_ms - shift_ms)
                      for p in chord.presses))


def _timed_by_another_rule(indexes: Sequence[int]) -> str:
    return (f"probe chords {', '.join(map(str, indexes))} are timed by another "
            f"rule, not from their own time (a squeeze is timed from its "
            f"Star Power end), so no probe lead can be given to them")


def probe_lines(probes: Sequence[Probe]) -> List[str]:
    """The reply's line naming each probe and its lead, in the file's order."""
    items = [f"chord {p.index} {abs(p.lead_ms):g} ms {'early' if p.lead_ms >= 0 else 'late'}"
             for p in probes]
    return ["probes: " + ", ".join(items)]


def probe_row_lines(rows: Sequence[dict]) -> List[str]:
    """What a probe run's rows say about its probes: one line per probe row,
    with the asked lead, the lead read off the row (chart_ms minus
    clock_ms_at_press) and the game's score step across the press
    (score_after minus score_before). The line gives no hit or miss verdict:
    score_compare judges a chord by its score step against Hydra's points
    (D319 item 1). Empty when no row is a probe's."""
    lines = []
    for row in rows:
        if row.get(PROBE_COLUMN) in (None, ""):
            continue
        asked = float(row[PROBE_COLUMN])
        read = float(row["chart_ms"]) - float(row["clock_ms_at_press"])
        step = float(row["score_after"]) - float(row["score_before"])
        lines.append(f"probe chord {row['index']}: asked {_lead_words(asked)}, "
                     f"pressed {_lead_words(read)}, score step {step:g}")
    return lines


def _lead_words(lead_ms: float) -> str:
    return f"{abs(lead_ms):.2f} ms {'early' if lead_ms >= 0 else 'late'}"
