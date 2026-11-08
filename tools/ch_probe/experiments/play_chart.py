"""Play the chords Hydra analyzed by reading the song clock and sending inputs.

The notes come from a `hydra_replay score` JSON, so the auto-player presses
exactly the chords, pads and times Hydra scored. Make one with
    hydra_replay score --chart <notes.chart or notes.mid> --acts "" --out <dump.json>
(or --path <a dump's JSON> --index N to walk one of its paths; the chords are
the same). A `hydra_replay dump` file does not work here: it lists paths, not
chords. Each entry of the score JSON's "chords" list gives the chord's place
in the chart (`index`), its time in the chart (`ms`, with no Offset or
song.ini delay, which is the song clock the game shows) and its "notes", one
per gem, each with the gem's color, whether it is a cymbal and its `dynamic`.
Whether the 2x kick notes are in it follows the run's own settings (--bass2x).

The auto-player presses each chord's own pads and nothing else. It has no
Star Power key: on drums the game activates from a press on a fill's last
note (docs/clone-hero/05-star-power.md, SP-17).

So a fill the path passes over is declined: declined_chord_indexes reads which
chords those are from a `hydra_replay dump` file, and load_dump_chords keeps
only the gems the decline mode sends (design point 4 of
docs/superpowers/plans/2026-11-06-optimal-score-run.md (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md);
the game's side is FL-31 on docs/clone-hero/04-fills.md).

The kit press gives each pad gem the strength its `dynamic` asks for; a key
has no strength. kit_tools.pad_axis owns the strengths, and with_axes reads the
game's dynamics threshold for them when the chart holds a ghost gem.

While the song plays, live.wait_until polls the game's song clock and fires
each press at the time press_plan gives it (a lead before the chord's time,
or after it for a squeeze-out); it also notices the song stopping (clock frozen
or jumped back). load_dump_chords refuses at load a press the game may not
match to its note. live.first_note_index picks where a run that joins mid-song
starts. A jump back (a restart or seek) re-syncs to the new clock position and
starts the counts and the press log over. A run also stops at its cap of
wall time: RUN_CAP_S, unless the caller passes a longer one.

The game is opened read-only and the engine is found with
engine_finder.find_live_engine_capped; when no song starts inside its cap the
script prints the finder's message and exits 2.

With --log, the script writes one CSV row per press, with the columns
overlay_logs.PRESS_LOG_COLUMNS names. score_compare.py reads that file.

Usage:
    python tools\\ch_probe\\experiments\\play_chart.py <score.json> [--log <press.csv>]
        [--dump <dump.json> [--path-index N]]

Run this, then start the song (or start it mid-song).
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import sys
import time
from dataclasses import dataclass, field, replace
from typing import Callable, List, Mapping, Optional, Sequence, TextIO, Tuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C, engine_finder
from tools.ch_probe.engine import EngineModel, pressed_input_hit
from tools.ch_probe.process import open_process
from tools.ch_probe.input_driver import (
    LANE_NAMES, InputDriver, Lane, find_game_window, focus_window)
from tools.ch_probe.experiments import live
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS
from tools.ch_probe.experiments.walk_edges import SongClock

# play_chart presses each chord this many ms before its time (D51: unchanged).
PRESS_LEAD_MS = 2.0

# A chord that ends Star Power is pressed this many ms before its time, so the
# hit lands in a frame before the end check. The user set it: question 3 of
# docs/superpowers/plans/2026-11-06-optimal-score-run.md (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md).
SP_END_LEAD_MS = 15.0

# A squeezed-out chord's late gem is pressed this many ms after the Star Power
# end, so the hit lands after the frame that ends it. The user set it:
# question 2 of docs/superpowers/plans/2026-11-07-player-refinements.md.
SP_OUT_LAG_MS = 15.0

# An activation pressed early or late for a squeeze is moved this many ms more
# than the squeeze's bare need, and its press must stay this far inside its own
# window. The user set it: question 1 of
# docs/superpowers/plans/2026-11-07-player-limits.md. activation_leads says
# how it is used.
ACT_LEAD_MARGIN_MS = 6.0

# The two ways an activation is moved, as ActLead.direction says them.
EARLY = "early"
LATE = "late"

# A chord whose gems the game would score in an order that costs points is
# pressed in groups, each this many ms after the one before (SC-38 on
# docs/clone-hero/06-scoring.md says why). The user set it ("8 ms"), and said
# to split "Only where it matters": the amendment after LS-R1b run 1 in
# docs/superpowers/plans/2026-11-07-player-refinements.md. _order_groups says
# which chords split and in what order.
CHORD_ORDER_GAP_MS = 8.0

# The dump's words for a squeeze's kind: SPSqueeze::type_name in
# src/core/model.h writes them.
SQUEEZE_IN = "SqIn"
SQUEEZE_OUT = "SqOut"

# Which gems a declined chord sends: _kept_gems owns what each mode keeps.
# Question 4 of the 2026-11-06 plan says which is tried first; question 5 of
# the 2026-11-07 plan added all_but_top_pad.
DECLINE_KICK_ONLY = "kick_only"
DECLINE_NOTHING = "nothing"
DECLINE_ALL_BUT_TOP_PAD = "all_but_top_pad"
DECLINE_MODES = (DECLINE_KICK_ONLY, DECLINE_NOTHING, DECLINE_ALL_BUT_TOP_PAD)

# The pad colors, highest first, in the order FL-09 on
# docs/clone-hero/04-fills.md reads the game's pick of a fill's flagged note.
# Row P22 of the 2026-11-07 plan: unchecked until a run shows it.
_TOP_PAD_ORDER = ("Green", "Blue", "Yellow", "Red")

# What a declined chord's press row and printed line say in place of a result.
DECLINED = "declined"

# What an activation's press row says in `end_read_ms` when the game's Star
# Power was off right after the press, so the plan's times stand.
NO_ACTIVATION_READ = "no activation read"

# The score is read this long after a press: a dense chart's next chord comes
# sooner than constants.INPUT_SETTLE_MS.
SCORE_READ_DELAY_S = 0.005

# A run stops after this much wall time, counted from the first clock read
# (the overlay spike plan, Task B step 1), when its caller gives no cap.
RUN_CAP_S = 240.0

# The key for each gem in a chord's "notes" list, by the gem's color (Kick,
# Red, Yellow, Blue, Green) and whether it is a cymbal, as hydra_replay writes
# them. A 2x kick is the kick pad, and a ghost or accent hits the same pad as
# a plain gem.
_KEY_FOR_PAD = {
    ("Kick", False): Lane.KICK,
    ("Red", False): Lane.RED,
    ("Yellow", False): Lane.YELLOW,
    ("Yellow", True): Lane.YELLOW_CYMBAL,
    ("Blue", False): Lane.BLUE,
    ("Blue", True): Lane.BLUE_CYMBAL,
    ("Green", False): Lane.GREEN,
    ("Green", True): Lane.GREEN_CYMBAL,
}


@dataclass(frozen=True)
class Squeeze:
    """What the dump says about a chord that squeezes at a Star Power end.
    `kind` is SQUEEZE_IN or SQUEEZE_OUT. `end_ms` is the end's time, the
    chord's time less the dump's `offset_ms` (note minus end: `offset_ms` in
    src/core/model.h). `act_index` is the `index` of the chord the
    activation started on."""
    kind: str
    end_ms: float
    act_index: int


# The dump's names for the fields of an activation that say how its end moves
# (tools/replay_json.cpp, paths_json, owns what each means).
_END_SCALE = {EARLY: "end_scale_early", LATE: "end_scale_late"}
_STEADY_ACT = {EARLY: "steady_ms_before_act", LATE: "steady_ms_after_act"}
_STEADY_END = {EARLY: "steady_ms_before_end", LATE: "steady_ms_after_end"}


@dataclass(frozen=True)
class ActEnd:
    """What the files say about where an activation's Star Power ends, kept
    on the activation's chord. `end_ms` is the end Hydra's path reaches (the
    score file's `deact_ms`, or its end chord's time), None when neither file
    gives it. `stored_end_ms` is the end the game stores when the activation
    fires, before any phrase extends it (the time of the dump's
    `nominal_deact_tick`), None when no file gives that time.
    `anchor_index` is the `index` of the chord on the dump's
    `end_anchor_tick`, None when the dump names none. `fields` holds the
    dump's end_scale_* and steady_ms_* values by name, each only when the dump
    wrote the key (a written null stays None)."""
    end_ms: Optional[float]
    stored_end_ms: Optional[float] = None
    anchor_index: Optional[int] = None
    fields: Mapping[str, Optional[float]] = field(default_factory=dict)


@dataclass(frozen=True)
class ActLead:
    """How far one activation is pressed off its chord's time so its squeezes
    land: activation_leads works it out. `act_index` is the activation's
    chord, `squeeze_index` the chord whose need set the lead, `direction`
    EARLY or LATE, `lead_ms` how far, `ratio` the dump's end_scale_* for that
    direction, and `planned_end_ms` the end the game should store once the
    activation fires: the stored end moved by `lead_ms` times `ratio`."""
    act_index: int
    squeeze_index: int
    direction: str
    lead_ms: float
    planned_end_ms: float
    ratio: float

    @property
    def end_shift_ms(self) -> float:
        """How far the end moves: negative for an early activation."""
        moved = self.lead_ms * self.ratio
        return -moved if self.direction == EARLY else moved

    @property
    def press_lead_ms(self) -> float:
        """The activation press's lead as Press.lead_ms holds it."""
        return self.lead_ms if self.direction == EARLY else -self.lead_ms


@dataclass(frozen=True)
class Chord:
    """One chord to press: its `index` in the score JSON, its time, its keys
    and each key's `dynamic`, in the same order. `axes` holds each key's
    strength once with_axes has worked them out. `ends_sp` says the chord's
    tick is an activation's `deact_tick`. `declined` says the path passes this
    fill over, so `lanes` holds only what the decline mode sends.

    The rest is what press_plan reads. `chart_lanes` is every gem's key, the
    gems a declined chord leaves out too (None: the same as `lanes`).
    `end_squeezed_out` says the activation this chord ends has a squeeze-out.
    `squeeze` is set on a chord the dump names as a squeeze. `sqout_index`,
    on an activation's chord, is the `index` of its squeeze-out chord.
    `multipliers` is the score file's per-gem `multiplier` for each key in
    `lanes` (None when the file gives none). `activating_gem`, on an
    activation's chord, is the place in `lanes` of the gem the game flags
    (_flagged_gem). `act_end`, on an activation's chord, is its ActEnd.
    `sp_act_index` is the `index` of the activation chord whose Star Power
    this chord ends, or pays inside when the score file says `in_sp`."""
    index: int
    ms: float
    lanes: List[int]
    dynamics: List[str]
    axes: Optional[List[float]] = None
    ends_sp: bool = False
    declined: bool = False
    chart_lanes: Optional[List[int]] = None
    end_squeezed_out: bool = False
    squeeze: Optional[Squeeze] = None
    sqout_index: Optional[int] = None
    multipliers: Optional[List[int]] = None
    activating_gem: Optional[int] = None
    act_end: Optional[ActEnd] = None
    sp_act_index: Optional[int] = None


@dataclass(frozen=True)
class Press:
    """One press press_plan works out: the chord's `index` and time
    (`chart_ms`), the `lead_ms` it is pressed by (negative for a late
    press), the keys it sends with their dynamics and axes, whether the
    chord is `declined`, and `checked`, which says the load-time check
    reads it. `end_ms`, set on a press timed against a Star Power end, is
    that end: the files' end, moved by any lead activation_leads gave the
    activation, and `end_act_index` is that activation's chord.
    `stored_end_ms`, on the first press of an activation with squeezes, is
    the end the game should store once it fires; play_chords reads the
    game's end after this press and retime_from_end moves the presses timed
    against it. `after_ms`, on a later order group of a chord (_order_groups),
    is how long after its chord's first group of the same message it goes
    down; the kit agent makes that gap inside the game (press_messages says
    which presses share a message). None on every other press."""
    index: int
    chart_ms: float
    lead_ms: float
    lanes: List[int]
    dynamics: List[str]
    axes: Optional[List[float]]
    declined: bool
    checked: bool
    end_ms: Optional[float] = None
    end_act_index: Optional[int] = None
    stored_end_ms: Optional[float] = None
    after_ms: Optional[float] = None

    @property
    def at_ms(self) -> float:
        """The song clock the press fires at."""
        return self.chart_ms - self.lead_ms


def half_gap_ms(gap_ms: Optional[float]) -> float:
    """The game's clamped half-gap to a neighbour `gap_ms` away, in normal
    mode; None means there is no neighbour. TM-18 and TM-19 on
    docs/clone-hero/03-timing.md say what a missing neighbour, or one at the
    same time, gives."""
    if gap_ms is None or gap_ms == 0:
        return C.EXPECT_NORMAL_BACK_MS
    return min(max(gap_ms / 2, C.s_to_ms(C.EXPECT_NORMAL_FRONT_S)),
               C.EXPECT_NORMAL_BACK_MS)


def window_side_ms(summed_ms: float) -> float:
    """How far from a note the game's window reaches on each side, given the
    sum of its two clamped half-gaps: half the normal-mode formula's output
    (TM-20 and TM-21; the constants are constants.WINDOW_FORMULA_NORMAL_*).
    The page leaves open whether the two sides differ (TM-26) and whether the
    hit check enforces this window (TM-27); the player assumes both sides
    are this one."""
    whole_ms = ((summed_ms * C.WINDOW_FORMULA_NORMAL_LINEAR
                 - summed_ms ** 2 * C.WINDOW_FORMULA_NORMAL_SQUARE)
                * C.WINDOW_FORMULA_NORMAL_SCALE - C.WINDOW_FORMULA_NORMAL_OFFSET_MS)
    return whole_ms / 2


def note_window_ms(gap_before_ms: Optional[float], gap_after_ms: Optional[float]) -> float:
    """The game's window, each side, for a note `gap_before_ms` after the
    note before it and `gap_after_ms` before the note after it (None where
    there is none)."""
    return window_side_ms(half_gap_ms(gap_before_ms) + half_gap_ms(gap_after_ms))


def window_sides(chords: Sequence[Chord]) -> dict:
    """Each chord's window, each side, by its `index`: note_window_ms over the
    gaps to the chords either side in `chords`, which are in time order. A
    declined chord counts, because its notes are still in the chart."""
    sides = {}
    for i, chord in enumerate(chords):
        before = chord.ms - chords[i - 1].ms if i > 0 else None
        after = chords[i + 1].ms - chord.ms if i + 1 < len(chords) else None
        sides[chord.index] = note_window_ms(before, after)
    return sides


def activation_leads(chords: Sequence[Chord]) -> List[ActLead]:
    """The lead of each activation of `chords` that one of its squeezes
    needs, in chart order; empty when none does. press_plan reads its leads
    here and nowhere else. Design points 2 and 6 of
    docs/superpowers/plans/2026-11-07-player-limits.md say why: a squeeze-out
    whose late gem would land past its window needs the end earlier, so the
    activation early; a squeeze-in after the end whose press before the end
    would land before its window needs the end later, so the activation late.
    The lead meets the largest need, through the dump's ratio, plus
    ACT_LEAD_MARGIN_MS. Raises ValueError, naming the chords, for an
    activation that needs both, one whose lead does not fit, and one the
    files cannot time (_solve_lead)."""
    sides = window_sides(chords)
    by_index = {chord.index: chord for chord in chords}
    needs: dict = {}
    for chord in chords:
        squeeze = chord.squeeze
        if squeeze is None:
            continue
        if squeeze.kind == SQUEEZE_OUT:
            direction = EARLY
            need_ms = squeeze.end_ms + SP_OUT_LAG_MS - chord.ms - sides[chord.index]
        else:
            direction = LATE
            need_ms = chord.ms + SP_END_LEAD_MS - squeeze.end_ms - sides[chord.index]
        if need_ms > 0:
            needs.setdefault(squeeze.act_index, []).append((direction, need_ms, chord.index))
    leads = [_solve_lead(by_index[act_index], act_needs, sides[act_index])
             for act_index, act_needs in needs.items()]
    return sorted(leads, key=lambda lead: by_index[lead.act_index].ms)


def _solve_lead(act: Chord, needs: Sequence[Tuple[str, float, int]],
                side_ms: float) -> ActLead:
    """The ActLead of activation chord `act` for its squeezes' `needs`, each
    (direction, how far the end must move in ms, the squeeze chord's
    `index`); `side_ms` is the activation chord's own window. Raises
    ValueError in plain words when no lead works."""
    directions = {direction for direction, _need, _index in needs}
    if len(directions) > 1:
        early = next(i for d, _n, i in needs if d == EARLY)
        late = next(i for d, _n, i in needs if d == LATE)
        raise ValueError(f"activation chord {act.index} would have to be pressed early for "
                         f"chord {early} and late for chord {late}; no press does both")
    direction = directions.pop()
    need_ms, squeeze_index = max((need, index) for _d, need, index in needs)
    end = act.act_end if act.act_end is not None else ActEnd(None)
    scale_name = _END_SCALE[direction]
    if scale_name not in end.fields:
        raise ValueError(f"chord {squeeze_index} needs activation chord {act.index} pressed "
                         f"{direction}, and the dump has no {scale_name}: remake it with "
                         f"hydra_replay dump")
    if end.anchor_index != act.index:
        anchor = ("no chord" if end.anchor_index is None
                  else f"chord {end.anchor_index}")
        raise ValueError(f"chord {act.index}'s end is anchored on {anchor}; pressing the "
                         f"activation {direction} does not move it (chord {squeeze_index} "
                         f"needs it moved)")
    ratio = end.fields[scale_name]
    if ratio is None:
        raise ValueError(f"chord {squeeze_index} needs activation chord {act.index} pressed "
                         f"{direction}, and the dump's {scale_name} for it is null")
    if end.stored_end_ms is None:
        raise ValueError(f"chord {squeeze_index} needs activation chord {act.index} pressed "
                         f"{direction}, and no file gives the end the game stores when it "
                         f"fires")
    bare_ms = need_ms / ratio
    lead_ms = bare_ms + ACT_LEAD_MARGIN_MS
    if lead_ms > side_ms - ACT_LEAD_MARGIN_MS:
        raise ValueError(f"chord {squeeze_index} needs activation chord {act.index} pressed "
                         f"{bare_ms:.0f} ms {direction} ({lead_ms:.0f} ms with "
                         f"ACT_LEAD_MARGIN_MS); its window is {side_ms:.0f} ms, and the press "
                         f"stays {ACT_LEAD_MARGIN_MS:g} ms inside it")
    for span_name, span_lead_ms in ((_STEADY_ACT[direction], lead_ms),
                                    (_STEADY_END[direction], lead_ms * ratio)):
        span_ms = end.fields.get(span_name)
        if span_ms is not None and span_lead_ms > span_ms:
            raise ValueError(f"activation chord {act.index} pressed {lead_ms:.2f} ms "
                             f"{direction} for chord {squeeze_index} moves past a tempo or "
                             f"time signature change: {span_name} is {span_ms:.2f} ms")
    lead = ActLead(act.index, squeeze_index, direction, lead_ms, 0.0, ratio)
    return replace(lead, planned_end_ms=end.stored_end_ms + lead.end_shift_ms)


def press_plan(chords: Sequence[Chord]) -> List[Press]:
    """Every press of `chords`, in the order they fire: the one place that
    works out when each gem is pressed (design point 1 of
    docs/superpowers/plans/2026-11-07-player-refinements.md and design points
    2, 3 and 6 of docs/superpowers/plans/2026-11-07-player-limits.md say why
    each rule is what it is). A squeezed-out chord with more than one gem
    gives two presses, and a chord _order_groups splits gives one per group.
    Raises ValueError for a chord two rules would time, and
    activation_leads's for an activation no lead works for."""
    leads = {lead.act_index: lead for lead in activation_leads(chords)}
    squeezed = {chord.squeeze.act_index for chord in chords if chord.squeeze is not None}
    act_ends = {chord.index: chord.act_end for chord in chords if chord.act_end is not None}
    presses = []
    for chord in chords:
        presses.extend(_chord_presses(chord, leads, act_ends, squeezed))
    return sorted(presses, key=lambda p: p.at_ms)


def retime_from_end(presses: Sequence[Press], act_index: int, planned_end_ms: float,
                    read_end_ms: float) -> List[Press]:
    """`presses` in firing order, with every press timed against activation
    chord `act_index`'s end (Press.end_act_index) moved by how far the end the
    game stored, `read_end_ms`, is from `planned_end_ms`. Design point 1 of
    docs/superpowers/plans/2026-11-07-player-limits.md says why: the game
    stamps an activation with the clock of the frame that read the press."""
    delta_ms = read_end_ms - planned_end_ms
    moved = [replace(p, lead_ms=p.lead_ms - delta_ms, end_ms=p.end_ms + delta_ms)
             if p.end_act_index == act_index else p for p in presses]
    return sorted(moved, key=lambda p: p.at_ms)


def press_messages(presses: Sequence[Press]) -> List[Tuple[Press, List[Press]]]:
    """`presses` in firing order as the messages play_chords sends: each a
    first press and the later order groups (Press.after_ms) that go down
    with it in one message, in the order their first presses fire. A later
    group goes with its chord's press that fires `after_ms` before it.
    Raises ValueError for a later group with no such press, which would be a
    bug in press_plan."""
    messages: List[Tuple[Press, List[Press]]] = []
    for press in sorted(presses, key=lambda p: p.at_ms):
        if press.after_ms is None:
            messages.append((press, []))
            continue
        head = next((m for m in reversed(messages) if m[0].index == press.index
                     and abs(m[0].at_ms + press.after_ms - press.at_ms) < _SAME_MS), None)
        if head is None:
            raise ValueError(f"chord {press.index} has an order group {press.after_ms:g} ms "
                             f"after no press of its own")
        head[1].append(press)
    return messages


# How close two press times must be to count as one: press_plan's times are
# sums of the same numbers, so anything past float rounding is another time.
_SAME_MS = 1e-6


def _order_groups(chord: Chord) -> List[List[int]]:
    """The groups `chord` is pressed in, in firing order, each as places in
    its `lanes`: one group for most chords. An activation's chord presses its
    `activating_gem` first; the rest go in groups by `multipliers`, the lower
    multiplier first. The user's "Only where it matters" (the amendment after
    LS-R1b run 1 of the 2026-11-07 plan) is why every other chord stays one
    group."""
    every = list(range(len(chord.lanes)))
    first = [] if chord.activating_gem is None else [chord.activating_gem]
    rest = [i for i in every if i not in first]
    if chord.multipliers is None:
        by_multiplier = [rest]
    else:
        by_multiplier = [[i for i in rest if chord.multipliers[i] == m]
                         for m in sorted({chord.multipliers[i] for i in rest})]
    # A declined chord that sends nothing is still one press, of no keys.
    return [group for group in [first] + by_multiplier if group] or [every]


def _squeeze_out_inside_groups(chord: Chord) -> List[List[int]]:
    """The groups a squeeze-out chord's inside gems are pressed in, in firing
    order, each as places in its `lanes`: every gem but the late one (place
    0, as _timed_presses says), the lower multiplier first. The late gem goes
    after them all, so a late gem the score file pays at a lower multiplier
    than an inside gem is an order no press gives. Design point 5 of
    docs/superpowers/plans/2026-11-07-player-limits.md and the user's answer
    to its question 5 say why that is refused as Hydra's question, not the
    player's."""
    inside = list(range(1, len(chord.lanes)))
    if chord.multipliers is None:
        return [inside]
    late_m = chord.multipliers[0]
    higher = sorted({chord.multipliers[i] for i in inside if chord.multipliers[i] > late_m})
    if higher:
        raise ValueError(f"chord {chord.index} is a squeeze-out whose late gem pays x{late_m} "
                         f"and an inside gem x{higher[-1]}: Hydra assumes an order no press "
                         f"gives, since the late gem goes last; this is a question about "
                         f"Hydra's path for the user, not a player bug")
    return [[i for i in inside if chord.multipliers[i] == m]
            for m in sorted({chord.multipliers[i] for i in inside})]


def _chord_presses(chord: Chord, leads: Mapping[int, ActLead],
                   act_ends: Mapping[int, ActEnd], squeezed: set) -> List[Press]:
    """The presses of one chord, for press_plan: `leads` and `act_ends` by
    activation chord, and `squeezed`, the activation chords with a squeeze."""
    def press(lead_ms: float, gems: Sequence[int], checked: bool,
              end_ms: Optional[float] = None, end_act: Optional[int] = None,
              after_ms: Optional[float] = None) -> Press:
        return Press(chord.index, chord.ms, lead_ms, [chord.lanes[i] for i in gems],
                     [chord.dynamics[i] for i in gems],
                     None if chord.axes is None else [chord.axes[i] for i in gems],
                     chord.declined, checked, end_ms, end_act, after_ms=after_ms)

    def end_shift_ms(act_index: Optional[int]) -> float:
        lead = leads.get(act_index)
        return 0.0 if lead is None else lead.end_shift_ms

    presses = _timed_presses(chord, leads, act_ends, press, end_shift_ms)
    # The first press of an activation with squeezes says what end the game
    # should store, so play_chords reads the game's end after it.
    if chord.index in squeezed:
        stored_ms = (leads[chord.index].planned_end_ms if chord.index in leads
                     else act_ends[chord.index].stored_end_ms
                     if chord.index in act_ends else None)
        if stored_ms is not None:
            presses[0] = replace(presses[0], stored_end_ms=stored_ms)
    return presses


def _timed_presses(chord: Chord, leads: Mapping[int, ActLead],
                   act_ends: Mapping[int, ActEnd], press: Callable[..., Press],
                   end_shift_ms: Callable[[Optional[int]], float]) -> List[Press]:
    """The presses of one chord before _chord_presses marks an activation's:
    `press` builds one, and `end_shift_ms` says how far an activation's lead
    moves its end."""

    # A declined chord's kept gems split by multiplier like any chord's
    # (design point 5 of docs/superpowers/plans/2026-11-07-player-limits.md);
    # it starts no activation, so no gem of it goes first.
    groups = _order_groups(chord)
    squeeze = chord.squeeze

    def in_order(lead_ms: float, checked: bool, end_ms: Optional[float] = None,
                 end_act: Optional[int] = None,
                 order: Optional[List[List[int]]] = None) -> List[Press]:
        # The first group keeps the chord's lead; a later group is a press
        # off the chord's own time, so the load check reads it.
        return [press(lead_ms - k * CHORD_ORDER_GAP_MS, gems, checked or k > 0, end_ms,
                      end_act, k * CHORD_ORDER_GAP_MS if k > 0 else None)
                for k, gems in enumerate(groups if order is None else order)]

    if chord.index in leads:
        if squeeze is not None or chord.ends_sp:
            raise ValueError(f"chord {chord.index} starts an activation pressed "
                             f"{leads[chord.index].direction} and is also a squeeze or an "
                             f"end; no rule times both")
        return in_order(leads[chord.index].press_lead_ms, True)
    if squeeze is not None:
        end_ms = squeeze.end_ms + end_shift_ms(squeeze.act_index)
        inside_lead_ms = chord.ms - (end_ms - SP_END_LEAD_MS)
        if squeeze.kind == SQUEEZE_IN:
            if inside_lead_ms > PRESS_LEAD_MS:
                return in_order(inside_lead_ms, True, end_ms, squeeze.act_index)
            # A plain press is not timed against the end, so the game's end
            # does not move it.
            return in_order(PRESS_LEAD_MS, False, end_ms)
        # The gem Hydra's FirstNote rule pays outside is note 0 of
        # Chord::note_list(true) (src/core/model.cpp), and the score file
        # lists a chord's notes in that order (src/core/replay.cpp), so it is
        # the file's first gem.
        late = press(chord.ms - (end_ms + SP_OUT_LAG_MS), [0], True, end_ms, squeeze.act_index)
        if len(chord.lanes) == 1:
            return [late]
        # The last inside group goes down at the inside time, SP_END_LEAD_MS
        # before the end, so no later group can land close to the end check
        # and pay outside; the earlier groups go down CHORD_ORDER_GAP_MS
        # apart before it.
        inside = _squeeze_out_inside_groups(chord)
        return in_order(inside_lead_ms + (len(inside) - 1) * CHORD_ORDER_GAP_MS, True, end_ms,
                        squeeze.act_index, inside) + [late]
    # Design point 3 of the player-limits plan: an end moved earlier puts the
    # chords before it at risk; one moved later leaves them inside already.
    end_side_shift_ms = min(end_shift_ms(chord.sp_act_index), 0.0)
    if chord.ends_sp and not chord.end_squeezed_out:
        # The chord sits on the end, so its own time is the end the lead
        # moves, and its lead is SP_END_LEAD_MS less that move.
        shift_ms = end_side_shift_ms
        return in_order(SP_END_LEAD_MS - shift_ms, True, chord.ms + shift_ms,
                        chord.sp_act_index)
    if (end_side_shift_ms < 0 and chord.sp_act_index is not None
            and chord.sp_act_index != chord.index and not chord.ends_sp):
        # A chord Hydra pays inside Star Power is pressed SP_END_LEAD_MS
        # before an end a lead moved earlier, when its own plain press would
        # come later than that. An end no lead moved leaves it plain.
        act_end = act_ends.get(chord.sp_act_index)
        if act_end is not None and act_end.end_ms is not None:
            end_ms = act_end.end_ms + end_side_shift_ms
            inside_lead_ms = chord.ms - (end_ms - SP_END_LEAD_MS)
            if inside_lead_ms > PRESS_LEAD_MS:
                return in_order(inside_lead_ms, True, end_ms, chord.sp_act_index)
    return in_order(PRESS_LEAD_MS, False)


def _refuse_far_presses(chords: Sequence[Chord], presses: Sequence[Press]) -> None:
    """Raise ValueError naming every chord with a checked press the game may
    not match to it: one outside its own note's window, or inside the window
    of the nearest other note on one of its pads, before or after (design
    point 5 of the 2026-11-07 plan; the press handler matches a press to a
    note on its own pad). A plain PRESS_LEAD_MS press is not checked: it has
    to work on any chart, and a real chart holds notes on one pad closer than
    a window."""
    sides = window_sides(chords)
    place = {chord.index: i for i, chord in enumerate(chords)}

    def inside(chord: Chord, at_ms: float) -> bool:
        return abs(at_ms - chord.ms) <= sides[chord.index]

    def reaches_a_neighbour(press: Press) -> bool:
        here = place[press.index]
        for lane in press.lanes:
            for side in (range(here - 1, -1, -1), range(here + 1, len(chords))):
                other = next((chords[i] for i in side
                              if lane in (chords[i].chart_lanes or chords[i].lanes)), None)
                if other is not None and inside(other, press.at_ms):
                    return True
        return False

    refused = sorted({press.index for press in presses
                      if press.checked and press.lanes
                      and (not inside(chords[place[press.index]], press.at_ms)
                           or reaches_a_neighbour(press))})
    if refused:
        raise ValueError(f"a press would miss its note or reach another note on its pad "
                         f"for chords {', '.join(map(str, refused))} (SP_END_LEAD_MS "
                         f"{SP_END_LEAD_MS:g} ms, SP_OUT_LAG_MS {SP_OUT_LAG_MS:g} ms, "
                         f"ACT_LEAD_MARGIN_MS {ACT_LEAD_MARGIN_MS:g} ms)")


def _dump_path(score_path: str, score: dict, dump_path: str, path_index: int) -> dict:
    """Path `path_index` of the dump at `dump_path`. Raises ValueError when
    the score file says it was made from another path."""
    with open(dump_path, encoding="utf-8") as f:
        path = json.load(f)["paths"][path_index]
    # A score file made from a path file names the path it read; one made with
    # --acts has no path_source and cannot be checked.
    source_index = score.get("path_source", {}).get("index")
    if source_index is not None and source_index != path_index:
        raise ValueError(f"{score_path} was made from path {source_index}, "
                         f"but path_index is {path_index}")
    return path


def declined_chord_indexes(score_path: str, dump_path: str,
                           path_index: int = 0) -> List[int]:
    """The `index` of each chord in the score file whose tick is a fill the
    dump's path `path_index` passed over. The dump's `skipped_fill_ticks` are
    the only record of those: the score file holds no meter. Raises ValueError
    for a tick the score file has no chord on, which means the two files are
    not of one chart."""
    with open(score_path, encoding="utf-8") as f:
        score = json.load(f)
    path = _dump_path(score_path, score, dump_path, path_index)
    chords = score["chords"]
    skipped = {tick for act in path["activations"] for tick in act["skipped_fill_ticks"]}
    missing = skipped - {chord["tick"] for chord in chords}
    if missing:
        raise ValueError(f"no chord in {score_path} at skipped fill ticks "
                         f"{', '.join(map(str, sorted(missing)))}")
    return [chord["index"] for chord in chords if chord["tick"] in skipped]


def _kept_gems(mode: str, gems: Sequence[Mapping]) -> List[int]:
    """Which of a declined chord's gems (the score file's "notes") it sends
    in decline mode `mode`, by their place in the chord. kick_only sends at
    most one gem, the first kick; all_but_top_pad sends every gem but the
    highest pad, so a chord with no pad gem sends nothing. Raises ValueError
    for a mode not in DECLINE_MODES, and in all_but_top_pad for a chord with
    two gems of its highest pad's color, because which one the game flags is
    not known."""
    if mode not in DECLINE_MODES:
        raise ValueError(f'unknown decline mode "{mode}"; modes: {", ".join(DECLINE_MODES)}')
    if mode == DECLINE_NOTHING:
        return []
    if mode == DECLINE_KICK_ONLY:
        return [i for i, gem in enumerate(gems) if _KEY_FOR_PAD[(gem["color"], gem["cymbal"])]
                == Lane.KICK][:1]
    top = _flagged_gem(gems)
    if top is None:
        return []
    return [i for i in range(len(gems)) if i != top]


def _flagged_gem(gems: Sequence[Mapping]) -> Optional[int]:
    """The place in a chord (the score file's "notes") of the gem the game
    flags: its highest pad by _TOP_PAD_ORDER, or None for a chord with no pad
    gem. all_but_top_pad leaves this gem out, and an activation's chord
    presses it first. Raises ValueError for a chord with two gems of its
    highest pad's color, because which one the game flags is not known."""
    top = next((color for color in _TOP_PAD_ORDER
                if any(gem["color"] == color for gem in gems)), None)
    if top is None:
        return None
    places = [i for i, gem in enumerate(gems) if gem["color"] == top]
    if len(places) > 1:
        raise ValueError(f"a chord holds two {top} gems; which one the game "
                         f"flags is not known")
    return places[0]


def load_dump_chords(path: str, declines: Optional[Mapping[int, str]] = None,
                     dump_path: Optional[str] = None, path_index: int = 0) -> List[Chord]:
    """The score file's chords in order, each with its own `index` from the
    JSON. A chord on an activation's end is marked. `dump_path` and
    `path_index` name the path's `hydra_replay dump` file, whose squeeze data
    press_plan times the squeezes by; a score file with a squeeze-out and no
    dump raises ValueError naming `dump_json`, because the squeeze-out's
    timing needs it. A press the game may not match to its note raises
    _refuse_far_presses's ValueError. `declines` maps a chord's `index` to
    its decline mode. A declined chord stays in the list with its time,
    because its notes are still in the chart for a neighbour's window."""
    declines = {} if declines is None else declines
    with open(path, encoding="utf-8") as f:
        score = json.load(f)
    score_acts = score.get("activations", [])
    act_ticks = {act["act_tick"] for act in score_acts}
    deact_ticks = {act["deact_tick"] for act in score_acts}
    if dump_path is None:
        if any(act.get("sqout_tick", -1) >= 0 for act in score_acts):
            raise ValueError(f"{path} has a squeeze-out, whose timing needs the path's "
                             f"dump: pass dump_json")
        dump_path_entry = None
        squeezes, sqout_of_act, sqout_deacts = {}, {}, set()
    else:
        dump_path_entry = _dump_path(path, score, dump_path, path_index)
        squeezes, sqout_of_act, sqout_deacts = _squeezes(path, score, dump_path_entry)
    act_ends = _act_ends(path, score, dump_path_entry, squeezes)
    sp_acts = _sp_acts(score)
    chords = []
    for chord in score["chords"]:
        gems = chord["notes"]
        lanes = [_KEY_FOR_PAD[(gem["color"], gem["cymbal"])] for gem in gems]
        dynamics = [gem["dynamic"] for gem in gems]
        declined = chord["index"] in declines
        kept = _kept_gems(declines[chord["index"]], gems) if declined else range(len(gems))
        tick = chord.get("tick")
        squeeze = squeezes.get(tick)
        sqout_index = sqout_of_act.get(chord["index"])
        if declined and (squeeze is not None or sqout_index is not None
                         or tick in act_ticks):
            raise ValueError(f"chord {chord['index']} is a fill the path passes over and "
                             f"also a squeeze or an activation")
        # A score file whose gems carry no `multiplier` splits no chord by it.
        multipliers = ([gems[i]["multiplier"] for i in kept]
                       if all("multiplier" in gem for gem in gems) else None)
        activating_gem = None
        if tick in act_ticks:
            try:
                flagged = _flagged_gem(gems)
            except ValueError as e:
                raise ValueError(f"chord {chord['index']} starts an activation: {e}") from e
            # A chord with no pad is all kicks, and its kick is the flagged gem.
            activating_gem = 0 if flagged is None else flagged
        chords.append(Chord(chord["index"], chord["ms"], [lanes[i] for i in kept],
                            [dynamics[i] for i in kept], ends_sp=tick in deact_ticks,
                            declined=declined, chart_lanes=lanes,
                            end_squeezed_out=tick in sqout_deacts, squeeze=squeeze,
                            sqout_index=sqout_index, multipliers=multipliers,
                            activating_gem=activating_gem, act_end=act_ends.get(tick),
                            sp_act_index=sp_acts.get(chord["index"])))
    _refuse_far_presses(chords, press_plan(chords))
    return chords


def _sp_acts(score: dict) -> dict:
    """The `index` of the activation chord for each chord of the score file
    that ends an activation or that the file says is `in_sp`, by the chord's
    `index`: the activation whose ticks hold it."""
    by_tick = {chord.get("tick"): chord for chord in score["chords"]}
    acts = [(act["act_tick"], act["deact_tick"], by_tick[act["act_tick"]]["index"])
            for act in score.get("activations", []) if act["act_tick"] in by_tick]
    out = {}
    for chord in score["chords"]:
        tick = chord.get("tick")
        for act_tick, deact_tick, act_index in acts:
            if tick == deact_tick or (chord.get("in_sp") and act_tick <= tick <= deact_tick):
                out[chord["index"]] = act_index
    return out


def _act_ends(score_path: str, score: dict, path: Optional[dict], squeezes: dict) -> dict:
    """Each activation's ActEnd by its `act_tick`, from the score file and,
    when given, the dump's `path`. Raises ValueError for an anchor tick the
    score file has no chord on."""
    chords = score["chords"]
    by_tick = {chord.get("tick"): chord for chord in chords}
    dump_acts = {act["act_tick"]: act for act in (path or {}).get("activations", [])}
    names = (*_END_SCALE.values(), *_STEADY_ACT.values(), *_STEADY_END.values())
    out = {}
    for act in score.get("activations", []):
        deact = by_tick.get(act["deact_tick"])
        end_ms = act.get("deact_ms", None if deact is None else deact["ms"])
        dump_act = dump_acts.get(act["act_tick"])
        if dump_act is None:
            out[act["act_tick"]] = ActEnd(end_ms)
            continue
        anchor_tick = dump_act.get("end_anchor_tick")
        if anchor_tick is not None and anchor_tick not in by_tick:
            raise ValueError(f"no chord in {score_path} at end_anchor_tick {anchor_tick}")
        out[act["act_tick"]] = ActEnd(
            end_ms, _stored_end_ms(act, dump_act, chords, by_tick, end_ms, squeezes),
            None if anchor_tick is None else by_tick[anchor_tick]["index"],
            {name: dump_act[name] for name in names if name in dump_act})
    return out


def _stored_end_ms(act: Mapping, dump_act: Mapping, chords: Sequence[Mapping],
                   by_tick: Mapping, end_ms: Optional[float],
                   squeezes: Mapping[int, Squeeze]) -> Optional[float]:
    """The time of the end the game stores when activation `act` fires, the
    dump's `nominal_deact_tick`, read off the files with no tempo map: the
    path's end when no phrase extended it, or a chord on that tick, or the
    end the activation's first squeeze is timed against when no phrase was
    collected in its Star Power before that squeeze (the score file's
    `in_sp` and `is_sp_phrase_end`). None when none of those holds."""
    nominal = dump_act.get("nominal_deact_tick")
    if nominal is None:
        return None
    if nominal == act["deact_tick"]:
        return end_ms
    if nominal in by_tick:
        return by_tick[nominal]["ms"]
    sq_ticks = sorted(set(dump_act.get("sqin_ticks", []))
                      | ({dump_act["sqout_tick"]} if dump_act.get("sqout_tick", -1) >= 0
                         else set()))
    if not sq_ticks or sq_ticks[0] not in squeezes:
        return None
    first = sq_ticks[0]
    collected = any(chord.get("in_sp") and chord.get("is_sp_phrase_end")
                    and act["act_tick"] < chord.get("tick") < first for chord in chords)
    return None if collected else squeezes[first].end_ms


def _squeezes(score_path: str, score: dict, path: dict):
    """The dump path's squeezes, read against the score file's chords: each
    squeeze chord's Squeeze by its tick, each squeeze-out chord's `index` by
    its activation chord's `index`, and the deact ticks of the activations
    with a squeeze-out. Raises ValueError when the two files do not describe
    one path, or a squeeze list does not match its ticks."""
    by_tick = {chord.get("tick"): chord for chord in score["chords"]}
    path_deacts = [act["deact_tick"] for act in path["activations"]]
    if path_deacts != [act["deact_tick"] for act in score.get("activations", [])]:
        raise ValueError(f"the dump's path ends its activations at ticks {path_deacts}, "
                         f"which {score_path} does not")

    def chord_at(tick: int) -> dict:
        if tick not in by_tick:
            raise ValueError(f"no chord in {score_path} at squeeze tick {tick}")
        return by_tick[tick]

    squeezes, sqout_of_act, sqout_deacts = {}, {}, set()
    for act in path["activations"]:
        if not act["sqinouts"] and act["sqout_tick"] < 0 and not act["sqin_ticks"]:
            continue
        act_index = chord_at(act["act_tick"])["index"]
        offsets = {kind: [s["offset_ms"] for s in act["sqinouts"] if s["kind"] == kind]
                   for kind in (SQUEEZE_IN, SQUEEZE_OUT)}
        sqout_ticks = [act["sqout_tick"]] if act["sqout_tick"] >= 0 else []
        if (len(act["sqinouts"]) != len(offsets[SQUEEZE_IN]) + len(offsets[SQUEEZE_OUT])
                or len(offsets[SQUEEZE_IN]) != len(act["sqin_ticks"])
                or len(offsets[SQUEEZE_OUT]) != len(sqout_ticks)):
            raise ValueError(f"the activation at tick {act['act_tick']} has sqinouts "
                             f"{act['sqinouts']}, which do not match its sqin_ticks "
                             f"{act['sqin_ticks']} and sqout_tick {act['sqout_tick']}")
        pairs = ([(SQUEEZE_IN, tick, offset)
                  for tick, offset in zip(act["sqin_ticks"], offsets[SQUEEZE_IN])]
                 + [(SQUEEZE_OUT, tick, offset)
                    for tick, offset in zip(sqout_ticks, offsets[SQUEEZE_OUT])])
        for kind, tick, offset in pairs:
            if tick in squeezes:
                raise ValueError(f"two squeezes on the chord at tick {tick}")
            squeezes[tick] = Squeeze(kind, chord_at(tick)["ms"] - offset, act_index)
        for tick in sqout_ticks:
            sqout_of_act[act_index] = chord_at(tick)["index"]
            sqout_deacts.add(act["deact_tick"])
    return squeezes, sqout_of_act, sqout_deacts


def load_score_times(path: str) -> List[Tuple[int, float]]:
    """The score file's chords as (`index`, ms), in order. It reads nothing a
    squeeze-out's dump would add, so a caller that needs only the times (the
    trace's note list) gets them for any score file."""
    with open(path, encoding="utf-8") as f:
        return [(chord["index"], chord["ms"]) for chord in json.load(f)["chords"]]


def load_dump_notes(path: str) -> list:
    """The file's chords in order, as (ms, the keys its gems press)."""
    return [(chord.ms, chord.lanes) for chord in load_dump_chords(path)]


def with_axes(chords: Sequence[Chord], engine) -> Tuple[List[Chord], Optional[int]]:
    """`chords` with each gem's strength from kit_tools.pad_axis, and the
    dynamics threshold read for them. The field is read once, and only when a
    gem is a ghost; otherwise the threshold is None. Every strength is worked
    out here, so a threshold no ghost can use raises pad_axis's ValueError
    before the first press."""
    # Imported here because kit_tools imports game_tools, which imports this
    # module.
    from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis
    threshold = None
    if any(DYNAMIC_GHOST in chord.dynamics for chord in chords):
        threshold = engine.dyn_threshold()
    return [replace(chord, axes=[pad_axis(dynamic, threshold) for dynamic in chord.dynamics])
            for chord in chords], threshold


# --- one run ------------------------------------------------------------------

class RunCapReached(Exception):
    """The run has used its cap of wall time."""


class StopAsked(Exception):
    """The caller's `should_stop` said to end the run."""


# The `stop` of a live run that never found a playing song's engine.
STOP_NO_ENGINE = "no engine"


@dataclass
class RunResult:
    """What one run did. `rows` are the press-log rows, keyed by the names in
    PRESS_LOG_COLUMNS, plus `lead_ms`, the lead the press used, which no log
    column holds. A split chord (press_plan) has a row per press, all with
    its `index`. A declined chord's row says DECLINED in `hit`. `sent`, `hit`
    and `max_streak` count the messages play_chords sends (press_messages:
    a chord's order groups are one) and leave declined chords out, so a
    declined chord is no miss; `declined` counts them. `stop` says why the run ended: "done" (every chord
    pressed), "past_end" (the clock was past the last chord), "frozen",
    "cap", "interrupted", "lost" (the process went away), "stopped" (the
    caller's `should_stop` said so) or STOP_NO_ENGINE (live_play found no
    playing song; `message` holds the finder's words). `dyn_threshold` is the
    dynamics threshold live_play read, None when it read none."""
    sent: int = 0
    hit: int = 0
    declined: int = 0
    max_streak: int = 0
    rows: list = field(default_factory=list)
    stop: str = ""
    message: str = ""
    dyn_threshold: Optional[int] = None


def play_chords(chords: Sequence[Chord], clock, read_score: Callable[[], int],
                press: Callable[..., object], *,
                now: Callable[[], float] = time.perf_counter,
                sleep: Callable[[float], None] = time.sleep,
                wall: Callable[[], float] = time.time,
                on_press: Optional[Callable[[dict], object]] = None,
                should_stop: Optional[Callable[[], bool]] = None,
                out: Optional[TextIO] = None,
                cap_s: float = RUN_CAP_S,
                read_sp_end: Optional[Callable[[], Optional[float]]] = None) -> RunResult:
    """Fire press_plan's presses of `chords` from where the song clock is
    now, in the order they fire, as press_messages groups them, and log a
    row per press.

    `read_sp_end`, when given, reads the game's Star Power end in song
    seconds, None while Star Power is off (EngineModel.sp_end). After a
    message whose first press carries Press.stored_end_ms it is read once:
    the first row's `end_read_ms` holds the end in ms and retime_from_end
    moves the later presses timed against that activation's end, or the row
    says NO_ACTIVATION_READ and the plan stands.

    `clock` reads the song clock (live.ReadsSongClock), `read_score` reads the
    engine's score and `press` sends one chord's keys and their `axes`. A
    chord split in order groups is one call, with `then=` a list of
    (lanes, axes, after_ms) for its later groups; every other press is
    called with the keys and axes alone. When `press` returns a dict (the
    kit press of tools/ch_mcp/kit_tools.py), each row also keeps its values
    that PRESS_LOG_COLUMNS names, from the reply's `groups` entry for its
    group when the reply has them.

    Design point 9 of docs/superpowers/plans/2026-11-07-player-limits.md
    says how a row's score is read: the score is read fresh just before each
    message, its rise since the last row's `score_after` goes in the first
    row's `scored_between`, and that fresh read is every row's
    `score_before`. The score is read once after the reply, so every row of
    a split chord holds the same `score_after` and the same `hit`.

    The live script passes the game's; the tests pass fakes. `now` and
    `sleep` are the timer the stop checks and the waits use. `wall` stamps
    each row's wall columns: it is the watch's clock, because `now` shares no
    zero with the watch or a trace. `on_press`, when given, gets each
    press-log row as it is added; `should_stop`, when given, is asked with
    the run cap and ends the run as "stopped" when it returns True. `cap_s`
    is the run cap in wall seconds."""
    out = sys.stdout if out is None else out
    plan = press_messages(press_plan(chords))
    presses = list(plan)
    notes_ms = [head.at_ms for head, _later in presses]
    result = RunResult()
    start_s = now()

    def over_cap() -> bool:
        # live.wait_until asks this before every clock read.
        if now() - start_s >= cap_s:
            raise RunCapReached(f"run cap of {cap_s:g} s reached")
        if should_stop is not None and should_stop():
            raise StopAsked("asked to stop")
        return False

    raw_s, _est = clock.read()
    stopped = live.StoppedCheck(raw_s, now())
    cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
    if cursor >= len(presses):
        print(f"  Song clock at {raw_s:.2f}s is past the last note; nothing to play.",
              file=out)
        result.stop = "past_end"
        return result
    print(f"  Song clock at {raw_s:.2f}s; starting at press {cursor + 1}/{len(presses)} "
          f"(t={C.ms_to_s(presses[cursor][0].chart_ms):.2f}s)", file=out)

    # The score the last row ended on: what scored_between is counted from.
    last_score = read_score()
    streak = 0

    print(f"\n  {'#':>4}  {'chart_t':>7}  {'clock':>7}  {'diff_ms':>7}  {'lanes':10}  "
          f"{'result':6}  {'streak':>6}", file=out)

    try:
        while cursor < len(presses):
            # One message: a chord, with any later order groups of it, or one
            # part of a squeezed-out chord.
            chord, later = presses[cursor]
            lead_ms = chord.lead_ms
            try:
                raw_ms, _est_ms = live.wait_until(
                    clock, chord.chart_ms, lead_ms=lead_ms, stopped=stopped,
                    should_stop=over_cap, now=now, sleep=sleep)
            except live.ClockJumpedBack as e:
                # A restart or seek: rebuild the cursor from the new clock
                # position rather than assuming the song went back to note 0.
                print(f"\n  Song {e}. Re-syncing.", file=out)
                raw_s, _est = clock.read()
                stopped = live.StoppedCheck(raw_s, now())
                # The song starts over, so a retimed plan does too.
                presses = list(plan)
                notes_ms = [head.at_ms for head, _later in presses]
                cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
                last_score = read_score()
                result = RunResult()
                streak = 0
                if cursor >= len(presses):
                    result.stop = "past_end"
                    print("\n  Stopped.", file=out)
                    return result
                continue
            wall_s_at_read = wall()
            score_before = read_score()

            # A declined chord in a mode that keeps no gem sends no press.
            if chord.declined and not chord.lanes:
                pressed = None
            elif later:
                pressed = press(chord.lanes, chord.axes,
                                then=[(p.lanes, p.axes, p.after_ms) for p in later])
            else:
                pressed = press(chord.lanes, chord.axes)
            wall_s_at_reply = wall()

            end_read = None
            if chord.stored_end_ms is not None and read_sp_end is not None:
                end_s = read_sp_end()
                if end_s is None:
                    end_read = NO_ACTIVATION_READ
                else:
                    end_read = C.s_to_ms(end_s)
                    rest = [p for head, rest_later in presses[cursor + 1:]
                            for p in [head, *rest_later]]
                    presses[cursor + 1:] = press_messages(retime_from_end(
                        rest, chord.index, chord.stored_end_ms, end_read))
                    notes_ms = [head.at_ms for head, _later in presses]

            # Check hit through engine.pressed_input_hit.
            sleep(SCORE_READ_DELAY_S)
            score_after = read_score()
            hit = not chord.declined and pressed_input_hit(score_before, score_after)

            if chord.declined:
                result.declined += 1
            else:
                result.sent += 1
                if hit:
                    result.hit += 1
                    streak += 1
                    result.max_streak = max(result.max_streak, streak)
                else:
                    streak = 0
            # A kit press replies with what the game's frames did, per group
            # for a message with later groups.
            replies = [pressed] if isinstance(pressed, dict) else []
            if replies and later and isinstance(pressed.get("groups"), list):
                replies = pressed["groups"]
            for k, part in enumerate([chord, *later]):
                row = {
                    "index": part.index,
                    "chart_ms": part.chart_ms,
                    # The clock read that sent the message; a later group's
                    # own moment is its reply's down_ms.
                    "clock_ms_at_press": raw_ms,
                    "lead_ms": part.lead_ms,
                    "score_before": score_before,
                    "score_after": score_after,
                    "hit": DECLINED if part.declined else int(hit),
                    "wall_s_at_read": wall_s_at_read,
                    "wall_s_at_reply": wall_s_at_reply,
                }
                if k == 0:
                    row["scored_between"] = score_before - last_score
                    if end_read is not None:
                        row["end_read_ms"] = end_read
                if k < len(replies) and isinstance(replies[k], dict):
                    # The row keeps the parts the press log has columns for.
                    row.update((key, replies[k][key]) for key in PRESS_LOG_COLUMNS
                               if key in replies[k] and key not in row)
                result.rows.append(row)
                if on_press is not None:
                    on_press(row)

            diff_ms = raw_ms - chord.chart_ms
            lane_str = " then ".join("+".join(LANE_NAMES.get(l, "?") for l in part.lanes)
                                     for part in [chord, *later])
            tag = DECLINED if chord.declined else "HIT" if hit else "MISS"
            if result.sent <= 30 or result.sent % 20 == 0 or not hit:
                print(f"  {result.sent:4d}  {C.ms_to_s(chord.chart_ms):7.2f}  "
                      f"{C.ms_to_s(raw_ms):7.2f}  {diff_ms:+7.1f}  {lane_str:10}  "
                      f"{tag:6}  {streak:6}", file=out)

            last_score = score_after
            cursor += 1
        result.stop = "done"

    except live.ClockFrozen as e:
        print(f"  {e}. Song ended or paused?", file=out)
        print("\n  Stopped.", file=out)
        result.stop = "frozen"
    except RunCapReached as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop = "cap"
    except StopAsked as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop = "stopped"
    except KeyboardInterrupt:
        print("\n  Stopped.", file=out)
        result.stop = "interrupted"
    except OSError as e:
        print(f"\n  Lost process: {e}", file=out)
        result.stop = "lost"
    return result


def write_press_log(path: str, rows: Sequence[dict]) -> None:
    """Write the press log: the PRESS_LOG_COLUMNS header, then one row per
    press. A row's other values stay out of the file."""
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = _press_log_writer(f)
        writer.writeheader()
        writer.writerows(rows)


def append_press_row(path: str, row: dict) -> None:
    """Add one row to the press log, with the header first when the file is
    new or empty. The file is closed again, so a worker that dies keeps every
    row so far."""
    with open(path, "a", encoding="utf-8", newline="") as f:
        writer = _press_log_writer(f)
        if f.tell() == 0:
            writer.writeheader()
        writer.writerow(row)


def _press_log_writer(f: TextIO) -> csv.DictWriter:
    # lead_ms stays off the log: a row's clock_ms_at_press already shows it.
    return csv.DictWriter(f, fieldnames=PRESS_LOG_COLUMNS, extrasaction="ignore")


# --- the live script ------------------------------------------------------------

def find_normal_engine(proc, out: Optional[TextIO] = None) -> int:
    """The playing song's engine object, through the capped finder, over the
    normal-mode pattern only: the auto-player does not play a precision-mode
    engine. The finder's progress dots go to `out` (stdout when None).
    LIVE-ONLY: scans the game's memory."""
    return engine_finder.find_live_engine_capped(
        proc, [engine_finder.normal_pattern(proc)], out=out)


def live_press(driver: InputDriver, hwnd: int) -> Callable[..., object]:
    """A press that focuses the game window, then sends the chord's keys. A
    key has no strength, so `axes` goes unused. This key route needs the game
    window in focus and the user's typed `allow key run`; the kit press does
    not. A key route has no frame clock to time a later order group by, so
    each group in `then` is sent once its `after_ms` has passed on this
    process's clock, counted from the first group's send.
    LIVE-ONLY: sends input to the game."""
    def press(lanes: List[int], axes: Optional[List[float]] = None,
              then: Sequence[Tuple[List[int], Optional[List[float]], float]] = ()) -> object:
        # Keep Clone Hero focused so SendInput reaches it.
        focus_window(hwnd)
        start_s = time.perf_counter()
        sent = driver.press_chord(lanes)
        for later_lanes, _axes, after_ms in then:
            time.sleep(max(0.0, start_s + C.ms_to_s(after_ms) - time.perf_counter()))
            driver.press_chord(later_lanes)
        return sent
    return press


def live_play(chords: Sequence[Chord], *,
              open_proc: Callable = open_process,
              find_engine: Optional[Callable[[object], int]] = None,
              driver: Optional[InputDriver] = None,
              press: Optional[Callable[..., object]] = None,
              on_press: Optional[Callable[[dict], object]] = None,
              should_stop: Optional[Callable[[], bool]] = None,
              out: Optional[TextIO] = None,
              cap_s: float = RUN_CAP_S) -> RunResult:
    """Play `chords` against the running game and return what the run did.

    LIVE-ONLY: opens the game read-only, finds its engine and presses each
    chord through `press` when one is given (the kit press, which needs no
    window, so none is looked up), else sends keys through `driver` (a new
    InputDriver when None) to the game window. When no song starts
    inside the finder's cap, the result's `stop` is STOP_NO_ENGINE and its
    `message` is the finder's. with_axes works out each gem's strength once
    the engine is found, on the press route only, and raises its ValueError
    before any press when no ghost strength is possible. `on_press`, `should_stop` and `cap_s` go to
    play_chords. `open_proc` and `find_engine` stand in for open_process and
    the capped finder in tests. With no `find_engine` the real search runs
    and its progress dots go to `out`, so a caller that gives `out` a buffer
    (the `ch` MCP server, whose stdout is its protocol channel) gets no
    stdout from the search."""
    out = sys.stdout if out is None else out
    if find_engine is None:
        def find_engine(proc) -> int:
            return find_normal_engine(proc, out=out)
    print("\nConnecting to Clone Hero...", file=out)
    proc = open_proc(read_only=True)
    try:
        proc.verify_targets()

        print("  Waiting for active engine (start/unpause the song)...", file=out)
        try:
            engine_ptr = find_engine(proc)
        except engine_finder.EngineSearchTimeout as e:
            return RunResult(stop=STOP_NO_ENGINE, message=str(e))
        engine = EngineModel(proc)
        engine.use_object(engine_ptr)
        print(f"  Engine at {engine_ptr:#x}, song clock = {engine.song_clock():.2f}s",
              file=out)
        # A key has no strength, so the keys route reads no threshold.
        threshold = None
        if press is not None:
            chords, threshold = with_axes(chords, engine)
        if threshold is not None:
            print(f"  Dynamics threshold: {threshold}", file=out)

        if press is None:
            ch_hwnd = find_game_window()
            if ch_hwnd:
                print(f"  Clone Hero window handle: {ch_hwnd:#x}", file=out)
            else:
                print("  WARNING: could not find Clone Hero window", file=out)
            driver = InputDriver() if driver is None else driver
            press = live_press(driver, ch_hwnd)

        # No fill-in between frames: the estimate is the raw clock, and the
        # presses fire on that.
        clock = SongClock(engine.song_clock, max_fill_s=0.0)
        result = play_chords(chords, clock, engine.score, press,
                             on_press=on_press, should_stop=should_stop, out=out,
                             cap_s=cap_s, read_sp_end=engine.sp_end)
        result.dyn_threshold = threshold
        return result
    finally:
        proc.close()


def main(argv: Optional[Sequence[str]] = None, *,
         open_proc: Callable = open_process,
         find_engine: Optional[Callable[[object], int]] = None,
         out: Optional[TextIO] = None) -> int:
    """Play a score JSON against the running game. Returns the exit code.

    LIVE-ONLY: the run itself is live_play's. `open_proc` and `find_engine`
    stand in for open_process and the capped finder in tests."""
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(
        description="Play a hydra_replay score JSON in Clone Hero.")
    parser.add_argument("score_json", help="a hydra_replay score JSON")
    parser.add_argument("--log", help="write one CSV row per press here")
    parser.add_argument("--dump", help="the path's hydra_replay dump JSON, which times "
                                       "its squeezes (needed for a squeeze-out)")
    parser.add_argument("--path-index", type=int, default=0,
                        help="the path of --dump the score JSON was made from")
    args = parser.parse_args(argv)

    chords = load_dump_chords(args.score_json, dump_path=args.dump,
                              path_index=args.path_index)
    if not chords:
        print(f"  {args.score_json} has no chords to play.", file=out)
        return 0
    print(f"Read {os.path.basename(args.score_json)}: {len(chords)} chords", file=out)
    print(f"  First note at {C.ms_to_s(chords[0].ms):.2f}s, "
          f"last at {C.ms_to_s(chords[-1].ms):.2f}s", file=out)

    result = live_play(chords, open_proc=open_proc, find_engine=find_engine,
                       out=out)
    if result.stop == STOP_NO_ENGINE:
        print(f"\n  {result.message}", file=out)
        return 2

    if args.log:
        write_press_log(args.log, result.rows)
        print(f"\n  Press log: {args.log} ({len(result.rows)} rows)", file=out)
    print(f"\n  Notes: {result.sent}, Hits: {result.hit}, "
          f"Misses: {result.sent - result.hit}", file=out)
    print(f"  Hit rate: {result.hit / max(result.sent, 1) * 100:.1f}%", file=out)
    print(f"  Best streak: {result.max_streak}", file=out)
    print("Done.", file=out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
