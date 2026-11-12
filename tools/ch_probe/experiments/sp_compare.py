"""Compare the game's live Star Power fields with Hydra's score for one path.

Pure logic: it opens no process. It reads three files from an overlay-spike run:

- the Star Power watcher's CSV (sp_watch.py), whose columns are
  overlay_logs.WATCH_COLUMNS, one row each time a field changed;
- the `hydra_replay score --path <dump.json> --index N` JSON of the path the
  run was meant to play;
- the `hydra_replay schedule` JSON of the same path, which says when the
  auto-player presses each chord (press_reach).

Each chord's `ms` in the JSON is taken as the chord's time on the game's song
clock (`clock_s` in the CSV), so the comparison is the same for an
auto-player run and a run the game's bot played.

It prints one PASS or FAIL line per check, with the numbers:

- phrase rises: each phrase-end chord is followed by a rise in the SP amount;
- sp_max on every row equals the game's full meter for the chart's
  resolution (game_full_meter);
- each activation the game made: the multiplier doubles, an end time ahead of
  the clock is set, and at that end time the flag clears and the multiplier
  halves; and whether it landed on the chord the path activates on;
- the game's multiplier at each chord against Hydra's, up to the first
  activation that parts from the path.

The plan behind every check is "Task S" of
docs/superpowers/plans/2026-10-28-overlay-spike.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-28-overlay-spike.md).

It also owns the row questions the `ch` MCP server asks of a watch in
progress: find_activations, phrase_rises, flips_between and gap_summary.
The server prints their answers and asks nothing itself.

Usage:
    py tools\\ch_probe\\experiments\\sp_compare.py <watch.csv> <score.json>
        [--resolution 192] [--sp-bar-ticks 14400] [--width 64|32]
        --schedule <schedule.json>

Exit code 0 when every check passed, 1 when any printed FAIL.
"""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import math
import os
import sys
from dataclasses import dataclass, field
from typing import NamedTuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C
from tools.ch_probe.experiments.overlay_logs import (
    WATCH_COLUMNS, WATCH_COLUMNS_WITHOUT_CLOCK, watch_header)
from tools.ch_probe.experiments import play_chart
from tools.ch_probe.experiments.score_compare import stop_check

# How far from an event a matching change in the watcher log may sit: a
# phrase end's rise, the multiplier doubling after an activation, the flag
# clearing at the end time. The plan's Task S names this window.
EVENT_WINDOW_MS = 100.0

# How far back from an activation's first flagged row the chord it landed on
# may sit. The plan's Task S names this window.
LANDING_WINDOW_MS = 150.0

# The game credits a chord when the auto-player presses it, which is not at
# the chord's own ms: the path's schedule (tools/replay_schedule.cpp) says when
# each gem is pressed, early or late, and a chord can have two presses. So a
# rise or an activation flag can show on a row whose clock is that far from the
# chord, and the searches reach that far for that chord only. A wider reach for
# every chord could hand a rise to the wrong one of two chords a few ms apart.
class PressReach(NamedTuple):
    """The song clock of a chord's first and last press (press_reach)."""
    earliest_ms: float
    latest_ms: float


def press_reach(schedule_path: str) -> dict:
    """Each chord's PressReach by its `index`, from press_plan over
    play_chart.load_schedule of the path's `hydra_replay schedule` file.
    load_schedule's ValueError, for a schedule that refuses the path, is
    passed on."""
    chords = play_chart.load_schedule(schedule_path).chords
    reach = {}
    for press in play_chart.press_plan(chords):
        lo, hi = reach.get(press.index, (press.at_ms, press.at_ms))
        reach[press.index] = PressReach(min(lo, press.at_ms), max(hi, press.at_ms))
    return reach

# Hydra's Star Power multiplier, copied because this script cannot read C++.
# The owner is kStarPowerMultiplier in src/core/timing.h.
SP_MULTIPLIER = 2

# The game's full meter in units per tick of the chart's resolution: SP-11 in
# docs/clone-hero/05-star-power.md (D319 item 2 checks sp_max against it).
GAME_FULL_METER_PER_RESOLUTION = 30

# Which sp_amount and sp_max columns to read by default. The amount and the
# maximum are 64-bit by item 5 of "Things that surprised me" in
# `git show 5fbe71ae:docs/audit/ch-evidence.md`, and by the Il2Cpp dump
# (results/overlay-spike/il2cpp-fields.md); --width 32 stays for a run that
# says otherwise.
# The flag is read from sp_active_byte because +0x42 is a one-byte bool (the
# bytes after it are other flags), so sp_active_u32 would mix them in.
DEFAULT_WIDTH = 64


def column_names(width: int = DEFAULT_WIDTH) -> tuple:
    """The (amount, maximum) watch-row column names for a width in bits."""
    return f"sp_amount_u{width}", f"sp_max_u{width}"

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
    """The watcher CSV's rows as dicts of numbers, keyed by WATCH_COLUMNS. A
    log written before the clock columns existed loads with them NaN, and a
    saved log's older column names are read through overlay_logs.watch_header."""
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        have = watch_header(reader.fieldnames or ())
        reader.fieldnames = list(have)
        missing = [c for c in WATCH_COLUMNS_WITHOUT_CLOCK if c not in have]
        if missing:
            raise ValueError(f"{path} lacks the watcher columns {missing}")
        return [{c: _number(r[c]) if c in have else math.nan
                 for c in WATCH_COLUMNS} for r in reader]


def load_score(path: str) -> dict:
    """The `hydra_replay score` JSON as written."""
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def game_full_meter(resolution: int) -> int:
    """The game's full Star Power meter for a chart of this resolution (SP-11)."""
    return GAME_FULL_METER_PER_RESOLUTION * resolution


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
    return C.s_to_ms(row["clock_s"])


def format_ms(ms: float) -> str:
    """A time in ms as the report prints it."""
    return f"{ms:.1f} ms"


def _chord_label(chord: dict) -> str:
    return f"chord {chord['index']} (tick {chord['tick']}, {format_ms(chord['ms'])})"


def is_active(row: dict) -> bool:
    """Whether a watch row has the Star Power flag set."""
    return row["sp_active_byte"] != 0


def find_activations(rows: list) -> list:
    """Each run of rows with the flag set, entered from a row with it clear,
    and a run already set at the first row (flip_row 0: the watch started
    after that activation began)."""
    found = []
    i = 0
    while i < len(rows):
        if is_active(rows[i]) and (i == 0 or not is_active(rows[i - 1])):
            drop = None
            for k in range(i + 1, len(rows)):
                if not is_active(rows[k]):
                    drop = k
                    break
            found.append(_Activation(len(found) + 1, i, drop))
            i = drop if drop is not None else len(rows)
        else:
            i += 1
    return found


# A second name for find_activations; only test_sp_compare.py reads it.
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


class GapSummary(NamedTuple):
    """What gap_summary found. With no gap every field but `count` is None."""
    count: int
    longest_ms: float | None
    from_ms: float | None  # the clock of the row before the first gap
    to_ms: float | None    # the clock of the row after the last gap


def gap_summary(rows: list, gap_ms: float,
                song_end_ms: float | None = None) -> GapSummary:
    """The gaps in wall time between consecutive rows that are longer than
    gap_ms. A gap of exactly gap_ms does not count. The watch writes a row
    only on a change, so a gap is a stretch with no new row, which a stalled
    watch and a stalled game both make.

    A gap counts only when both its rows have a clock at or below song_end_ms,
    the song's end (D255). That leaves out the wait after the song, which ends
    on the watch's final row for a stopped clock (sp_watch.watch). With no
    song_end_ms every row counts."""
    gaps = {i: C.s_to_ms(rows[i]["wall_s"] - rows[i - 1]["wall_s"])
            for i in range(1, len(rows))
            if song_end_ms is None
            or max(_ms(rows[i - 1]), _ms(rows[i])) <= song_end_ms}
    over = [i for i, gap in gaps.items() if gap > gap_ms]
    if not over:
        return GapSummary(0, None, None, None)
    return GapSummary(len(over), max(gaps[i] for i in over),
                      _ms(rows[over[0] - 1]), _ms(rows[over[-1]]))


def _check_phrase_rises(report: Report, rows: list, chords: list,
                        amount_col: str, max_col: str, reach: dict) -> None:
    """A rise is looked for from the chord's first press (press_reach) to
    EVENT_WINDOW_MS after the later of its ms and its last press."""
    used_steps = set()
    full_sizes = []
    rise_rows = _rise_rows(rows, amount_col)
    for chord in chords:
        if not chord["is_sp_phrase_end"]:
            continue
        presses = reach[chord["index"]]
        lo = presses.earliest_ms
        hi = max(chord["ms"], presses.latest_ms) + EVENT_WINDOW_MS
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
                         f"missing (no rise from {format_ms(lo)} to {format_ms(hi)})")
            continue
        used_steps.add(step)
        before = rows[step - 1][amount_col]
        after = rows[step][amount_col]
        size = after - before
        kind = "clamped" if after == rows[step][max_col] else "full"
        if kind == "full":
            full_sizes.append(size)
        during = ", during Star Power" if is_active(rows[step - 1]) else ""
        report.check(True, f"phrase rise after {_chord_label(chord)}: "
                     f"+{size} ({before} -> {after}) at {format_ms(_ms(rows[step]))}, "
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
                  resolution: int, sp_bar_ticks) -> None:
    """Every row's sp_max against game_full_meter for the chart's resolution."""
    values = []
    for r in rows:
        if r[max_col] not in values:
            values.append(r[max_col])
    want = game_full_meter(resolution)
    beside = (f"the game's full meter {want} (resolution {resolution} x "
              f"{GAME_FULL_METER_PER_RESOLUTION})")
    if sp_bar_ticks is not None:
        beside += f"; sp_bar_ticks {sp_bar_ticks}"
    shown = ", ".join(str(v) for v in values)
    report.check(values == [want], f"sp_max {shown}; {beside}")


def _landed_chord(chords: list, chord_ms: list, flip_ms: float, reach: dict):
    """The latest chord whose first press (press_reach) is at or before the
    flip, within LANDING_WINDOW_MS of the chord's ms, or None."""
    def earliest(chord: dict) -> float:
        return reach[chord["index"]].earliest_ms
    # No chord after the one the widest early press reaches can qualify.
    widest_lead = max([0.0] + [c["ms"] - earliest(c) for c in chords])
    i = bisect.bisect_right(chord_ms, flip_ms + widest_lead) - 1
    while i >= 0 and earliest(chords[i]) > flip_ms:
        i -= 1
    if i < 0 or flip_ms - chord_ms[i] > LANDING_WINDOW_MS:
        return None
    return chords[i]


def _check_activation(report: Report, rows: list, act: _Activation,
                      hydra_end_ms: float | None) -> None:
    """One activation's checks. `hydra_end_ms` is the path activation's
    `deact_ms`, None when the path has no activation to match it."""
    n = act.number
    flip = rows[act.flip_row]
    flip_ms = _ms(flip)
    window = [r for r in rows[act.flip_row:] if _ms(r) - flip_ms <= EVENT_WINDOW_MS]

    if act.flip_row == 0:
        report.check(False, f"  activation {n} was already on at the watch's first row "
                     f"({format_ms(flip_ms)}); its start was not seen")
        return
    base = rows[act.flip_row - 1]["sp_mult_u32"]
    doubled = next((r for r in window if r["sp_mult_u32"] == base * SP_MULTIPLIER), None)
    if doubled is not None:
        report.check(True, f"  activation {n} multiplier doubles: {base} -> "
                     f"{doubled['sp_mult_u32']} at {format_ms(_ms(doubled))}")
    else:
        seen = sorted({r["sp_mult_u32"] for r in window})
        report.check(False, f"  activation {n} multiplier doubles: {base} before, "
                     f"{seen} within {EVENT_WINDOW_MS:g} ms, expected "
                     f"{base * SP_MULTIPLIER}")

    ahead = next((r for r in window if is_active(r) and r["sp_end_s"] > r["clock_s"]), None)
    if ahead is not None:
        report.check(True, f"  activation {n} end time ahead of the clock: end "
                     f"{format_ms(ahead['sp_end_s'] * 1000)}, clock {format_ms(_ms(ahead))}")
    else:
        report.check(False, f"  activation {n} end time ahead of the clock: end "
                     f"{format_ms(flip['sp_end_s'] * 1000)}, clock {format_ms(flip_ms)}")

    if act.drop_row is None:
        last = rows[-1]
        report.check(False, f"  activation {n} flag clears at the end: still set at "
                     f"the last row ({format_ms(_ms(last))}), end time "
                     f"{format_ms(last['sp_end_s'] * 1000)}")
        return
    last_active = rows[act.drop_row - 1]
    end_ms = last_active["sp_end_s"] * 1000
    if last_active["sp_end_s"] != flip["sp_end_s"]:
        report.note(f"  activation {n} end time moved while active: "
                    f"{format_ms(flip['sp_end_s'] * 1000)} at the start, {format_ms(end_ms)} at the end")
    drop_ms = _ms(rows[act.drop_row])
    if hydra_end_ms is None:
        report.check(False, f"  activation {n} flag clears at Hydra's end: no path "
                     f"activation to compare with; flag 0 at {format_ms(drop_ms)}")
    else:
        report.check(abs(drop_ms - hydra_end_ms) <= EVENT_WINDOW_MS,
                     f"  activation {n} flag clears at Hydra's end: Hydra "
                     f"{format_ms(hydra_end_ms)}, flag 0 at {format_ms(drop_ms)}")

    held = last_active["sp_mult_u32"]
    near_end = [r for r in rows[act.drop_row - 1:]
                if abs(_ms(r) - end_ms) <= EVENT_WINDOW_MS]
    halved = next((r for r in near_end if r["sp_mult_u32"] * SP_MULTIPLIER == held), None)
    if halved is not None:
        report.check(True, f"  activation {n} multiplier halves at the end: {held} -> "
                     f"{halved['sp_mult_u32']} at {format_ms(_ms(halved))}")
    else:
        seen = sorted({r["sp_mult_u32"] for r in near_end})
        report.check(False, f"  activation {n} multiplier halves at the end: {held} "
                     f"before, {seen} within {EVENT_WINDOW_MS:g} ms of the end")


def _check_end_chord(report: Report, rows: list, act: _Activation, want: dict,
                     chords: list, reach: dict) -> None:
    """The last chord the game paid inside this Star Power against Hydra's
    deactivation chord (D319 item 2). The game's is the last chord whose
    first press (press_reach) is at or before the game's end time; Hydra's is
    the last chord its score JSON marks `in_sp` from act_tick to deact_tick."""
    n = act.number
    hydra = [c for c in chords
             if c["in_sp"] and want["act_tick"] <= c["tick"] <= want["deact_tick"]]
    if not hydra:
        report.check(False, f"  activation {n} end chord: Hydra pays no chord inside "
                     f"Star Power from tick {want['act_tick']} to {want['deact_tick']}")
        return
    deact = hydra[-1]
    if act.drop_row is None:
        report.check(False, f"  activation {n} end chord: the flag never cleared; "
                     f"Hydra's deactivation chord {_chord_label(deact)}")
        return
    end_ms = C.s_to_ms(rows[act.drop_row - 1]["sp_end_s"])
    paid = [c for c in chords if reach[c["index"]].earliest_ms <= end_ms]
    game = paid[-1] if paid else None
    game_text = _chord_label(game) if game is not None else "none"
    report.check(game is not None and game["index"] == deact["index"],
                 f"  activation {n} end chord: game end {format_ms(end_ms)}, last chord "
                 f"paid inside {game_text}; Hydra's deactivation chord "
                 f"{_chord_label(deact)}, difference {end_ms - deact['ms']:+.1f} ms")


def _check_activations(report: Report, rows: list, score: dict,
                       reach: dict) -> float | None:
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
        report.note(f"Activation {act.number} at clock {format_ms(flip_ms)}:")
        _check_activation(report, rows, act, want["deact_ms"] if want else None)

        landed = _landed_chord(chords, chord_ms, flip_ms, reach)
        landed_tick = landed["tick"] if landed else None
        if want is not None and landed_tick == want["act_tick"]:
            report.check(True, f"  activation {act.number} on the path's chord: "
                         f"{_chord_label(landed)}")
            _check_end_chord(report, rows, act, want, chords, reach)
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
                       parted_ms, reach: dict) -> None:
    """The game's multiplier after each chord is credited, read on the last
    row before the next chord, against Hydra's for that chord. One chord of
    lag either way is allowed, because a 4 ms poll can land on either side of
    a credit. A chord with no row from its first press (press_reach) on is
    not compared."""
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
        if j < 0 or clocks[j] < reach[chord["index"]].earliest_ms:
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
        report.check(compared > 0, f"multiplier at each chord: {matched} of {compared} "
                     f"chords match (one chord of lag allowed)")
    if skipped_before:
        report.note(f"Multiplier not compared on {skipped_before} chords with no "
                    f"watcher row between them and the next chord")
    if stopped_at is not None:
        report.note(f"Multiplier not compared from chord {stopped_at['index']} on: "
                    f"the game's activations part from the path at {format_ms(parted_ms)}")


def compare(rows: list, score: dict, *, resolution: int | None = None,
            sp_bar_ticks: int | None = None, width: int = DEFAULT_WIDTH,
            reach: dict, stop: str | None = None, stop_message: str = "") -> Report:
    """Every check over the watcher rows and the score JSON, as a Report whose
    first line is the summary. `reach` is press_reach of the same path's
    schedule file; `stop` and `stop_message` are the run's RunResult.stop and
    .message, judged by score_compare.stop_check. Raises ValueError with no
    resolution, because sp_max is checked against it."""
    if resolution is None:
        raise ValueError("no chart resolution given; sp_max is checked against "
                         "the game's full meter for it (SP-11)")
    report = Report()
    _compare_into(report, rows, score, resolution, sp_bar_ticks, width, reach,
                  stop, stop_message)
    report.lines.insert(0, f"Summary: {report.passes} PASS, {report.fails} FAIL")
    return report


def _compare_into(report: Report, rows: list, score: dict, resolution: int,
                  sp_bar_ticks, width: int, reach: dict, stop, stop_message: str) -> None:
    amount_col, max_col = column_names(width)
    chords = score["chords"]
    report.note(f"Star Power comparison: {len(rows)} watcher rows, {len(chords)} "
                f"chords, {len(score['activations'])} path activations, width {width}")
    stop_ok, stop_line = stop_check(stop, stop_message)
    report.check(stop_ok, stop_line[len("PASS "):])
    if not rows:
        report.check(False, "watcher rows: the CSV has none")
        return
    for i in range(1, len(rows)):
        if rows[i]["clock_s"] < rows[i - 1]["clock_s"]:
            report.note(f"Note: the clock went back at row {i + 1} "
                        f"({format_ms(_ms(rows[i - 1]))} -> {format_ms(_ms(rows[i]))}); "
                        f"the checks assume one pass through the song")
            break

    _check_phrase_rises(report, rows, chords, amount_col, max_col, reach)
    _check_sp_max(report, rows, max_col, resolution, sp_bar_ticks)
    parted_ms = _check_activations(report, rows, score, reach)
    _check_multipliers(report, rows, chords, parted_ms, reach)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("watch_csv", help="the sp_watch.py CSV")
    ap.add_argument("score_json", help="hydra_replay score --path JSON of the reference path")
    ap.add_argument("--resolution", type=int, required=True,
                    help="the chart's Resolution from its [Song] section; sp_max "
                         "is checked against the game's full meter for it")
    ap.add_argument("--stop", default=None,
                    help="the run's stop reason (play_chart's RunResult.stop); "
                         "without it the run fails")
    ap.add_argument("--stop-message", default="",
                    help="the run's RunResult.message")
    ap.add_argument("--sp-bar-ticks", type=int, default=None,
                    help="sp_bar_ticks from the game's scorestats.json")
    ap.add_argument("--width", type=int, choices=(32, 64), default=DEFAULT_WIDTH,
                    help="which sp_amount and sp_max columns to read")
    ap.add_argument("--schedule", required=True,
                    help="the same path's hydra_replay schedule JSON, which gives "
                         "each chord's press times")
    args = ap.parse_args(argv)
    report = compare(load_watch_rows(args.watch_csv), load_score(args.score_json),
                     resolution=args.resolution, sp_bar_ticks=args.sp_bar_ticks,
                     width=args.width, reach=press_reach(args.schedule),
                     stop=args.stop, stop_message=args.stop_message)
    print(report.text())
    return 1 if report.fails else 0


if __name__ == "__main__":
    sys.exit(main())
