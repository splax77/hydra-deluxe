"""Compare the game's live Star Power fields with Hydra's score for one path.

Pure logic: it opens no process. It reads two files from an overlay-spike run:

- the Star Power watcher's CSV (sp_watch.py), whose columns are
  overlay_logs.WATCH_COLUMNS, one row each time a field changed;
- the `hydra_replay score --path <dump.json> --index N` JSON of the path the
  run was meant to play.

Each chord's `ms` in the JSON is taken as the chord's time on the game's song
clock (`clock_s` in the CSV), so the comparison is the same for an
auto-player run and a run the game's bot played.

It prints one PASS or FAIL line per check, with the numbers:

- phrase rises: each phrase-end chord is followed by a rise in the SP amount;
- sp_max stays constant, printed beside Hydra's full meter for the chart;
- each activation the game made: the multiplier doubles, an end time ahead of
  the clock is set, and at that end time the flag clears and the multiplier
  halves; and whether it landed on the chord the path activates on;
- the game's multiplier at each chord against Hydra's, up to the first
  activation that parts from the path.

The plan behind every check is "Task S" in
docs/superpowers/plans/2026-10-28-overlay-spike.md.

It also owns the row questions the `ch` MCP server asks of a watch in
progress: find_activations, phrase_rises, flips_between, ready_between and
at_ready_level. The server prints their answers and asks nothing itself.

Usage:
    py tools\\ch_probe\\experiments\\sp_compare.py <watch.csv> <score.json>
        [--resolution 192] [--sp-bar-ticks 14400] [--width 64|32]

Exit code 0 when every check passed, 1 when any printed FAIL.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import os
import sys
from dataclasses import dataclass, field
from typing import NamedTuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS
from tools.ch_probe.experiments.play_chart import PRESS_LEAD_MS

# How far from an event a matching change in the watcher log may sit: a
# phrase end's rise, the multiplier doubling after an activation, the flag
# clearing at the end time. The plan's Task S names this window.
EVENT_WINDOW_MS = 100.0

# How far back from an activation's first flagged row the chord it landed on
# may sit. The plan's Task S names this window.
LANDING_WINDOW_MS = 150.0

# The game can credit a chord up to PRESS_LEAD_MS before the chord's own ms,
# because the auto-player presses that early (play_chart.py owns the lead).
# So a rise or an activation flag can show on a row whose clock is that much
# before the chord, and both searches reach back that far.
EARLY_CREDIT_MS = PRESS_LEAD_MS

# Hydra's own constants, copied because this script cannot read C++. The
# owners are kStarPowerMultiplier in src/core/timing.h, kMeasuresPerSpBar in
# src/core/timing.h, kCloneHeroSpCap in src/core/model.h and
# kSpActivationBars in src/core/timing.h. The meter is counted in measures;
# QUARTERS_PER_MEASURE is the 4/4 measure the plan's tick figure assumes.
SP_MULTIPLIER = 2
MEASURES_PER_SP_BAR = 2
SP_CAP_BARS = 4
SP_ACTIVATION_BARS = 2
QUARTERS_PER_MEASURE = 4

# The share of a full meter an activation needs, from the two copies above.
# at_ready_level is the one place that compares an amount with it.
READY_FRACTION = SP_ACTIVATION_BARS / SP_CAP_BARS

# Which sp_amount and sp_max columns to read by default. The amount and the
# maximum are 64-bit by docs/audit/ch-evidence.md ("Things that surprised me",
# item 5) and the Il2Cpp dump; --width 32 stays for a run that says otherwise.
# The flag is read from sp_active_byte because +0x42 is a one-byte bool (the
# bytes after it are other flags), so sp_active_u32 would mix them in.
DEFAULT_WIDTH = 64

# The plan names four phrases as the expected fill of the meter, so a full
# rise is printed beside sp_max divided by this.
PHRASES_PER_FULL_METER = 4


# --------------------------------------------------------------------------
# Inputs


def _number(text: str):
    """A CSV cell as an int when it is a whole number, else a float."""
    try:
        return int(text)
    except ValueError:
        return float(text)


def load_watch_rows(path: str) -> list:
    """The watcher CSV's rows as dicts of numbers, keyed by WATCH_COLUMNS."""
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        missing = [c for c in WATCH_COLUMNS if c not in (reader.fieldnames or ())]
        if missing:
            raise ValueError(f"{path} lacks the watcher columns {missing}")
        return [{c: _number(r[c]) for c in WATCH_COLUMNS} for r in reader]


def load_score(path: str) -> dict:
    """The `hydra_replay score` JSON as written."""
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def hydra_full_meter_ticks(resolution: int) -> int:
    """Hydra's full SP meter in chart ticks on a 4/4 stretch."""
    return resolution * QUARTERS_PER_MEASURE * MEASURES_PER_SP_BAR * SP_CAP_BARS


# --------------------------------------------------------------------------
# The comparison


@dataclass
class Report:
    lines: list = field(default_factory=list)
    passes: int = 0
    fails: int = 0

    def check(self, ok: bool, text: str) -> None:
        self.lines.append(("PASS " if ok else "FAIL ") + text)
        if ok:
            self.passes += 1
        else:
            self.fails += 1

    def note(self, text: str) -> None:
        self.lines.append(text)

    def text(self) -> str:
        return "\n".join(self.lines)


@dataclass
class _Activation:
    number: int          # 1-based, in the order the game made them
    flip_row: int        # first row with the flag set
    drop_row: int | None  # first row after it with the flag clear, if any


def _ms(row: dict) -> float:
    return row["clock_s"] * 1000.0


def _fmt(ms: float) -> str:
    return f"{ms:.1f} ms"


def _chord_label(chord: dict) -> str:
    return f"chord {chord['index']} (tick {chord['tick']}, {_fmt(chord['ms'])})"


def _active(row: dict) -> bool:
    return row["sp_active_byte"] != 0


def find_activations(rows: list) -> list:
    """Each run of rows with the flag set, entered from a row with it clear."""
    found = []
    i = 1
    while i < len(rows):
        if _active(rows[i]) and not _active(rows[i - 1]):
            drop = None
            for k in range(i + 1, len(rows)):
                if not _active(rows[k]):
                    drop = k
                    break
            found.append(_Activation(len(found) + 1, i, drop))
            i = drop if drop is not None else len(rows)
        else:
            i += 1
    return found


# The name the comparison used before find_activations was public.
_find_activations = find_activations


class Rise(NamedTuple):
    """One rise of the Star Power amount between two consecutive rows."""
    ms: float      # the clock of the row the new amount shows on
    before: int
    after: int

    @property
    def size(self) -> int:
        return self.after - self.before


def _rise_rows(rows: list, amount_col: str) -> list:
    """The index of every row whose amount is above the row before it. This is
    the one place that says what a rise is."""
    return [i for i in range(1, len(rows))
            if rows[i][amount_col] > rows[i - 1][amount_col]]


def phrase_rises(rows: list, amount_col: str) -> list:
    """Every step up of the amount from one row to the next, as Rise tuples."""
    return [Rise(_ms(rows[i]), rows[i - 1][amount_col], rows[i][amount_col])
            for i in _rise_rows(rows, amount_col)]


def _inside(row: dict, start_ms: float, end_ms: float) -> bool:
    """Whether a row's clock lies in the window, both ends included."""
    return start_ms <= _ms(row) <= end_ms


def flips_between(rows: list, start_ms: float, end_ms: float) -> list:
    """The activations (find_activations) whose flip row's clock lies in the
    window."""
    return [a for a in find_activations(rows)
            if _inside(rows[a.flip_row], start_ms, end_ms)]


def at_ready_level(amount, sp_max) -> bool:
    """Whether an amount is enough for an activation on a meter of sp_max. A
    meter that holds nothing (sp_max 0, a read before the game set it up) is
    never ready."""
    return sp_max > 0 and amount >= READY_FRACTION * sp_max


@dataclass
class ReadyWindow:
    """What ready_between read inside one window."""
    rows: int                  # how many rows lie in the window
    highest: int | None        # the highest amount read there
    sp_max: int | None         # the maximum read on the window's first row
    ready_row: dict | None     # the first ready row with the flag clear

    @property
    def ready_ms(self) -> float | None:
        return _ms(self.ready_row) if self.ready_row is not None else None


def ready_between(rows: list, start_ms: float, end_ms: float,
                  amount_col: str, max_col: str) -> ReadyWindow:
    """The highest amount inside the window, and the first row inside it whose
    amount is at the ready level (at_ready_level, against that row's own
    maximum) while the flag is clear. The flag matters because the game
    offers no fill while Star Power is already on."""
    inside = [r for r in rows if _inside(r, start_ms, end_ms)]
    if not inside:
        return ReadyWindow(0, None, None, None)
    ready = next((r for r in inside
                  if not _active(r) and at_ready_level(r[amount_col], r[max_col])),
                 None)
    return ReadyWindow(len(inside), max(r[amount_col] for r in inside),
                       inside[0][max_col], ready)


def _check_phrase_rises(report: Report, rows: list, chords: list,
                        amount_col: str, max_col: str) -> None:
    used_steps = set()
    full_sizes = []
    rise_rows = _rise_rows(rows, amount_col)
    for chord in chords:
        if not chord["is_sp_phrase_end"]:
            continue
        lo = chord["ms"] - EARLY_CREDIT_MS
        hi = chord["ms"] + EVENT_WINDOW_MS
        step = None
        for i in rise_rows:
            t = _ms(rows[i])
            if t < lo or i in used_steps:
                continue
            if t > hi:
                break
            step = i
            break
        if step is None:
            report.check(False, f"phrase rise after {_chord_label(chord)}: "
                         f"missing (no rise from {_fmt(lo)} to {_fmt(hi)})")
            continue
        used_steps.add(step)
        before = rows[step - 1][amount_col]
        after = rows[step][amount_col]
        size = after - before
        kind = "clamped" if after == rows[step][max_col] else "full"
        if kind == "full":
            full_sizes.append(size)
        during = ", during Star Power" if _active(rows[step - 1]) else ""
        report.check(True, f"phrase rise after {_chord_label(chord)}: "
                     f"+{size} ({before} -> {after}) at {_fmt(_ms(rows[step]))}, "
                     f"{kind}{during}")
    sp_max = rows[0][max_col]
    quarter = sp_max / PHRASES_PER_FULL_METER
    if not full_sizes:
        report.note(f"Full rises: none; sp_max / {PHRASES_PER_FULL_METER} = {quarter:g}")
    elif len(set(full_sizes)) == 1:
        report.note(f"Full rises: all {full_sizes[0]} ticks; "
                    f"sp_max / {PHRASES_PER_FULL_METER} = {quarter:g}")
    else:
        sizes = ", ".join(str(s) for s in sorted(set(full_sizes)))
        report.note(f"Full rises: not all the same, sizes {sizes}; "
                    f"sp_max / {PHRASES_PER_FULL_METER} = {quarter:g}")


def _check_sp_max(report: Report, rows: list, max_col: str,
                  resolution: int | None, sp_bar_ticks) -> None:
    values = []
    for r in rows:
        if r[max_col] not in values:
            values.append(r[max_col])
    if resolution is None:
        beside = "Hydra full meter: not computed (no resolution given)"
    else:
        meter = hydra_full_meter_ticks(resolution)
        beside = f"Hydra full meter {meter} (resolution {resolution} x 32)"
    if sp_bar_ticks is not None:
        beside += f"; sp_bar_ticks {sp_bar_ticks}"
    if len(values) == 1:
        report.check(True, f"sp_max constant: {values[0]}; {beside}")
    else:
        shown = ", ".join(str(v) for v in values)
        report.check(False, f"sp_max not constant: values {shown}; {beside}")


def _landed_chord(chords: list, chord_ms: list, flip_ms: float):
    """The latest chord at or before the flip (allowing the early credit),
    within LANDING_WINDOW_MS, or None."""
    i = bisect.bisect_right(chord_ms, flip_ms + EARLY_CREDIT_MS) - 1
    if i < 0 or flip_ms - chord_ms[i] > LANDING_WINDOW_MS:
        return None
    return chords[i]


def _check_activation(report: Report, rows: list, act: _Activation) -> None:
    n = act.number
    flip = rows[act.flip_row]
    flip_ms = _ms(flip)
    window = [r for r in rows[act.flip_row:] if _ms(r) - flip_ms <= EVENT_WINDOW_MS]

    base = rows[act.flip_row - 1]["sp_mult_u32"]
    doubled = next((r for r in window if r["sp_mult_u32"] == base * SP_MULTIPLIER), None)
    if doubled is not None:
        report.check(True, f"  activation {n} multiplier doubles: {base} -> "
                     f"{doubled['sp_mult_u32']} at {_fmt(_ms(doubled))}")
    else:
        seen = sorted({r["sp_mult_u32"] for r in window})
        report.check(False, f"  activation {n} multiplier doubles: {base} before, "
                     f"{seen} within {EVENT_WINDOW_MS:g} ms, expected "
                     f"{base * SP_MULTIPLIER}")

    ahead = next((r for r in window if _active(r) and r["sp_end_s"] > r["clock_s"]), None)
    if ahead is not None:
        report.check(True, f"  activation {n} end time ahead of the clock: end "
                     f"{_fmt(ahead['sp_end_s'] * 1000)}, clock {_fmt(_ms(ahead))}")
    else:
        report.check(False, f"  activation {n} end time ahead of the clock: end "
                     f"{_fmt(flip['sp_end_s'] * 1000)}, clock {_fmt(flip_ms)}")

    if act.drop_row is None:
        last = rows[-1]
        report.check(False, f"  activation {n} flag clears at the end: still set at "
                     f"the last row ({_fmt(_ms(last))}), end time "
                     f"{_fmt(last['sp_end_s'] * 1000)}")
        return
    last_active = rows[act.drop_row - 1]
    end_ms = last_active["sp_end_s"] * 1000
    if last_active["sp_end_s"] != flip["sp_end_s"]:
        report.note(f"  activation {n} end time moved while active: "
                    f"{_fmt(flip['sp_end_s'] * 1000)} at the start, {_fmt(end_ms)} at the end")
    drop_ms = _ms(rows[act.drop_row])
    report.check(abs(drop_ms - end_ms) <= EVENT_WINDOW_MS,
                 f"  activation {n} flag clears at the end: end {_fmt(end_ms)}, "
                 f"flag 0 at {_fmt(drop_ms)}")

    held = last_active["sp_mult_u32"]
    near_end = [r for r in rows[act.drop_row - 1:]
                if abs(_ms(r) - end_ms) <= EVENT_WINDOW_MS]
    halved = next((r for r in near_end if r["sp_mult_u32"] * SP_MULTIPLIER == held), None)
    if halved is not None:
        report.check(True, f"  activation {n} multiplier halves at the end: {held} -> "
                     f"{halved['sp_mult_u32']} at {_fmt(_ms(halved))}")
    else:
        seen = sorted({r["sp_mult_u32"] for r in near_end})
        report.check(False, f"  activation {n} multiplier halves at the end: {held} "
                     f"before, {seen} within {EVENT_WINDOW_MS:g} ms of the end")


def _check_activations(report: Report, rows: list, score: dict) -> float | None:
    """The per-activation checks. Returns the clock ms at which the game's
    activations first part from the path's, or None when they never do."""
    chords = score["chords"]
    chord_ms = [c["ms"] for c in chords]
    by_tick = {c["tick"]: c for c in chords}
    path = score["activations"]
    game = find_activations(rows)

    parted_ms = None
    for k in range(max(len(game), len(path))):
        want = path[k] if k < len(path) else None
        want_chord = by_tick.get(want["act_tick"]) if want else None
        want_tick = want["act_tick"] if want else "none"

        if k >= len(game):
            report.check(False, f"path activation {k + 1} (act_tick {want_tick}): "
                         f"the game made no activation")
            if parted_ms is None and want_chord is not None:
                parted_ms = want_chord["ms"]
            continue

        act = game[k]
        flip_ms = _ms(rows[act.flip_row])
        report.note(f"Activation {act.number} at clock {_fmt(flip_ms)}:")
        _check_activation(report, rows, act)

        landed = _landed_chord(chords, chord_ms, flip_ms)
        landed_tick = landed["tick"] if landed else None
        if want is not None and landed_tick == want["act_tick"]:
            deact = by_tick.get(want["deact_tick"])
            text = (f"  activation {act.number} on the path's chord: "
                    f"{_chord_label(landed)}")
            if act.drop_row is not None and deact is not None:
                end_ms = rows[act.drop_row - 1]["sp_end_s"] * 1000
                text += (f"; game end {_fmt(end_ms)}, Hydra deactivation "
                         f"{_fmt(deact['ms'])} (chord {deact['index']}, tick "
                         f"{deact['tick']}), difference {end_ms - deact['ms']:+.1f} ms")
            elif deact is None:
                text += f"; no chord at deact_tick {want['deact_tick']}"
            report.check(True, text)
            continue

        landed_text = (f"tick {landed_tick}" if landed is not None
                       else f"no chord within {LANDING_WINDOW_MS:g} ms")
        report.check(False, f"  activation {act.number} not on the path's chord: "
                     f"game {landed_text}, path act_tick {want_tick}")
        if parted_ms is None:
            candidates = [flip_ms]
            if want_chord is not None:
                candidates.append(want_chord["ms"])
            parted_ms = min(candidates)
    return parted_ms


def _expected_multipliers(chord: dict) -> set:
    factor = SP_MULTIPLIER if chord["in_sp"] else 1
    return {chord["multiplier"] * factor, chord["multiplier_after"] * factor}


def _check_multipliers(report: Report, rows: list, chords: list,
                       parted_ms) -> None:
    """The game's multiplier after each chord is credited, read on the last
    row before the next chord, against Hydra's for that chord. One chord of
    lag either way is allowed, because a 4 ms poll can land on either side of
    a credit."""
    clocks = [_ms(r) for r in rows]
    compared = matched = 0
    first_fail = None
    fails = 0
    skipped_before = 0
    stopped_at = None
    for i, chord in enumerate(chords):
        sample_ms = chords[i + 1]["ms"] if i + 1 < len(chords) else clocks[-1]
        if parted_ms is not None and sample_ms >= parted_ms:
            stopped_at = chord
            break
        j = bisect.bisect_left(clocks, sample_ms) - 1
        if j < 0 or clocks[j] < chord["ms"] - EARLY_CREDIT_MS:
            skipped_before += 1
            continue
        game = rows[j]["sp_mult_u32"]
        accepted = set()
        for n in (i - 1, i, i + 1):
            if 0 <= n < len(chords):
                accepted |= _expected_multipliers(chords[n])
        compared += 1
        if game in accepted:
            matched += 1
            continue
        fails += 1
        if first_fail is None:
            state = "in Star Power" if chord["in_sp"] else "not in Star Power"
            want = chord["multiplier"] * (SP_MULTIPLIER if chord["in_sp"] else 1)
            first_fail = (f"{_chord_label(chord)}: game {game}, expected {want} "
                          f"(multiplier {chord['multiplier']}, {state})")
    if fails:
        report.check(False, f"multiplier at each chord: first mismatch at {first_fail}; "
                     f"{matched} of {compared} chords match")
    else:
        report.check(True, f"multiplier at each chord: {matched} of {compared} "
                     f"chords match (one chord of lag allowed)")
    if skipped_before:
        report.note(f"Multiplier not compared on {skipped_before} chords with no "
                    f"watcher row between them and the next chord")
    if stopped_at is not None:
        report.note(f"Multiplier not compared from chord {stopped_at['index']} on: "
                    f"the game's activations part from the path at {_fmt(parted_ms)}")


def compare(rows: list, score: dict, *, resolution: int | None = None,
            sp_bar_ticks: int | None = None, width: int = DEFAULT_WIDTH) -> Report:
    """Every check over the watcher rows and the score JSON, as a Report.
    With no resolution, Hydra's full meter is left out of the sp_max line."""
    amount_col = f"sp_amount_u{width}"
    max_col = f"sp_max_u{width}"
    chords = score["chords"]
    report = Report()
    report.note(f"Star Power comparison: {len(rows)} watcher rows, {len(chords)} "
                f"chords, {len(score['activations'])} path activations, width {width}")
    if not rows:
        report.check(False, "watcher rows: the CSV has none")
        return report
    for i in range(1, len(rows)):
        if rows[i]["clock_s"] < rows[i - 1]["clock_s"]:
            report.note(f"Note: the clock went back at row {i + 1} "
                        f"({_fmt(_ms(rows[i - 1]))} -> {_fmt(_ms(rows[i]))}); "
                        f"the checks assume one pass through the song")
            break

    _check_phrase_rises(report, rows, chords, amount_col, max_col)
    _check_sp_max(report, rows, max_col, resolution, sp_bar_ticks)
    parted_ms = _check_activations(report, rows, score)
    _check_multipliers(report, rows, chords, parted_ms)
    report.note(f"Summary: {report.passes} PASS, {report.fails} FAIL")
    return report


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("watch_csv", help="the sp_watch.py CSV")
    ap.add_argument("score_json", help="hydra_replay score --path JSON of the reference path")
    ap.add_argument("--resolution", type=int, default=None,
                    help="the chart's Resolution from its [Song] section; "
                         "without it Hydra's full meter is not computed")
    ap.add_argument("--sp-bar-ticks", type=int, default=None,
                    help="sp_bar_ticks from the game's scorestats.json")
    ap.add_argument("--width", type=int, choices=(32, 64), default=DEFAULT_WIDTH,
                    help="which sp_amount and sp_max columns to read")
    args = ap.parse_args(argv)
    report = compare(load_watch_rows(args.watch_csv), load_score(args.score_json),
                     resolution=args.resolution, sp_bar_ticks=args.sp_bar_ticks,
                     width=args.width)
    print(report.text())
    return 1 if report.fails else 0


if __name__ == "__main__":
    sys.exit(main())
