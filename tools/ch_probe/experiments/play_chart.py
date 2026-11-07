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

# An activation whose squeeze-out the late press alone cannot reach is pressed
# this many ms early, which moves its end earlier by the same amount. The user
# set it: question 3 of the same plan. press_plan says when it is used.
SQOUT_ACT_LEAD_MS = 55.0

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
    on an activation's chord, is the `index` of its squeeze-out chord."""
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


@dataclass(frozen=True)
class Press:
    """One press press_plan works out: the chord's `index` and time
    (`chart_ms`), the `lead_ms` it is pressed by (negative for a late
    press), the keys it sends with their dynamics and axes, whether the
    chord is `declined`, and `checked`, which says the load-time check
    reads it. `end_ms`, set on a squeeze's presses, is the Star Power end the
    press is timed against: the dump's end, less any lead press_plan gave the
    activation."""
    index: int
    chart_ms: float
    lead_ms: float
    lanes: List[int]
    dynamics: List[str]
    axes: Optional[List[float]]
    declined: bool
    checked: bool
    end_ms: Optional[float] = None

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


def press_plan(chords: Sequence[Chord]) -> List[Press]:
    """Every press of `chords`, in the order they fire: the one place that
    works out when each gem is pressed (design point 1 of
    docs/superpowers/plans/2026-11-07-player-refinements.md says why each
    rule is what it is). A squeezed-out chord with more than one gem gives
    two presses. Raises ValueError for a chord two rules would time."""
    by_index = {chord.index: chord for chord in chords}
    sides = window_sides(chords)
    # An activation is pressed early only when the late press of its
    # squeeze-out would otherwise fall past that chord's window. The end then
    # moves earlier by the same amount: the plan's point 4 says why that
    # holds only at one tempo and time signature (r = 1).
    act_leads = {}
    for chord in chords:
        if chord.sqout_index is None:
            continue
        out = by_index[chord.sqout_index]
        if out.squeeze.end_ms + SP_OUT_LAG_MS - out.ms > sides[out.index]:
            act_leads[chord.index] = SQOUT_ACT_LEAD_MS
    presses = []
    for chord in chords:
        presses.extend(_chord_presses(chord, act_leads))
    return sorted(presses, key=lambda p: p.at_ms)


def _chord_presses(chord: Chord, act_leads: Mapping[int, float]) -> List[Press]:
    """The presses of one chord, for press_plan."""
    def press(lead_ms: float, gems: Sequence[int], checked: bool,
              end_ms: Optional[float] = None) -> Press:
        return Press(chord.index, chord.ms, lead_ms, [chord.lanes[i] for i in gems],
                     [chord.dynamics[i] for i in gems],
                     None if chord.axes is None else [chord.axes[i] for i in gems],
                     chord.declined, checked, end_ms)

    every = range(len(chord.lanes))
    squeeze = chord.squeeze
    if chord.index in act_leads:
        if squeeze is not None or chord.ends_sp:
            raise ValueError(f"chord {chord.index} starts an activation pressed early "
                             f"and is also a squeeze or an end; no rule times both")
        return [press(act_leads[chord.index], every, True)]
    if squeeze is not None:
        end_ms = squeeze.end_ms - act_leads.get(squeeze.act_index, 0.0)
        inside_lead_ms = chord.ms - (end_ms - SP_END_LEAD_MS)
        if squeeze.kind == SQUEEZE_IN:
            if inside_lead_ms > PRESS_LEAD_MS:
                return [press(inside_lead_ms, every, True, end_ms)]
            return [press(PRESS_LEAD_MS, every, False, end_ms)]
        # The gem Hydra's FirstNote rule pays outside is note 0 of
        # Chord::note_list(true) (src/core/model.cpp), and the score file
        # lists a chord's notes in that order (src/core/replay.cpp), so it is
        # the file's first gem.
        late = press(chord.ms - (end_ms + SP_OUT_LAG_MS), [0], True, end_ms)
        if len(chord.lanes) == 1:
            return [late]
        return [press(inside_lead_ms, range(1, len(chord.lanes)), True, end_ms), late]
    if chord.ends_sp and not chord.end_squeezed_out:
        return [press(SP_END_LEAD_MS, every, True)]
    return [press(PRESS_LEAD_MS, every, False)]


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
                         f"SQOUT_ACT_LEAD_MS {SQOUT_ACT_LEAD_MS:g} ms)")


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
    top = next((color for color in _TOP_PAD_ORDER
                if any(gem["color"] == color for gem in gems)), None)
    if top is None:
        return []
    if sum(gem["color"] == top for gem in gems) > 1:
        raise ValueError(f"a declined chord holds two {top} gems; which one the game "
                         f"flags is not known")
    return [i for i, gem in enumerate(gems) if gem["color"] != top]


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
    deact_ticks = {act["deact_tick"] for act in score_acts}
    if dump_path is None:
        if any(act.get("sqout_tick", -1) >= 0 for act in score_acts):
            raise ValueError(f"{path} has a squeeze-out, whose timing needs the path's "
                             f"dump: pass dump_json")
        squeezes, sqout_of_act, sqout_deacts = {}, {}, set()
    else:
        squeezes, sqout_of_act, sqout_deacts = _squeezes(
            path, score, _dump_path(path, score, dump_path, path_index))
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
        if declined and (squeeze is not None or sqout_index is not None):
            raise ValueError(f"chord {chord['index']} is a fill the path passes over and "
                             f"also a squeeze or a squeeze-out's activation")
        chords.append(Chord(chord["index"], chord["ms"], [lanes[i] for i in kept],
                            [dynamics[i] for i in kept], ends_sp=tick in deact_ticks,
                            declined=declined, chart_lanes=lanes,
                            end_squeezed_out=tick in sqout_deacts, squeeze=squeeze,
                            sqout_index=sqout_index))
    _refuse_far_presses(chords, press_plan(chords))
    return chords


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
    column holds. A split chord (press_plan) has a row per press, both with
    its `index`. A declined chord's row says DECLINED in `hit`. `sent`, `hit`
    and `max_streak` count presses and leave declined chords out, so a
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
                press: Callable[[List[int], Optional[List[float]]], object], *,
                now: Callable[[], float] = time.perf_counter,
                sleep: Callable[[float], None] = time.sleep,
                wall: Callable[[], float] = time.time,
                on_press: Optional[Callable[[dict], object]] = None,
                should_stop: Optional[Callable[[], bool]] = None,
                out: Optional[TextIO] = None,
                cap_s: float = RUN_CAP_S) -> RunResult:
    """Fire press_plan's presses of `chords` from where the song clock is
    now, in the order they fire, and log each press.

    `clock` reads the song clock (live.ReadsSongClock), `read_score` reads the
    engine's score and `press` sends one chord's keys and their `axes`. When
    `press` returns a dict (the kit press of tools/ch_mcp/kit_tools.py), the
    row also keeps its values that PRESS_LOG_COLUMNS names. The live script passes
    the game's; the tests pass fakes. `now` and `sleep` are the timer the stop
    checks and the waits use. `wall` stamps each row's wall columns: it is the
    watch's clock, because `now` shares no zero with the watch or a trace.
    `on_press`, when given, gets each press-log row as it is added;
    `should_stop`, when given, is asked with the run cap and ends the run as
    "stopped" when it returns True. `cap_s` is the run cap in wall seconds."""
    out = sys.stdout if out is None else out
    presses = press_plan(chords)
    notes_ms = [p.at_ms for p in presses]
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
          f"(t={C.ms_to_s(presses[cursor].chart_ms):.2f}s)", file=out)

    score_before = read_score()
    streak = 0

    print(f"\n  {'#':>4}  {'chart_t':>7}  {'clock':>7}  {'diff_ms':>7}  {'lanes':10}  "
          f"{'result':6}  {'streak':>6}", file=out)

    try:
        while cursor < len(presses):
            # One press of press_plan's: a chord, or one part of a split one.
            chord = presses[cursor]
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
                cursor = live.first_note_index(notes_ms, C.s_to_ms(raw_s))
                score_before = read_score()
                result = RunResult()
                streak = 0
                if cursor >= len(presses):
                    result.stop = "past_end"
                    print("\n  Stopped.", file=out)
                    return result
                continue
            wall_s_at_read = wall()

            # A declined chord in a mode that keeps no gem sends no press.
            pressed = None if chord.declined and not chord.lanes else press(chord.lanes, chord.axes)
            wall_s_at_reply = wall()

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
            row = {
                "index": chord.index,
                "chart_ms": chord.chart_ms,
                "clock_ms_at_press": raw_ms,
                "lead_ms": lead_ms,
                "score_before": score_before,
                "score_after": score_after,
                "hit": DECLINED if chord.declined else int(hit),
                "wall_s_at_read": wall_s_at_read,
                "wall_s_at_reply": wall_s_at_reply,
            }
            if isinstance(pressed, dict):
                # A kit press replies with what the game's frames did; the
                # row keeps the parts the press log has columns for.
                row.update((k, pressed[k]) for k in PRESS_LOG_COLUMNS
                           if k in pressed and k not in row)
            result.rows.append(row)
            if on_press is not None:
                on_press(row)

            diff_ms = raw_ms - chord.chart_ms
            lane_str = "+".join(LANE_NAMES.get(l, "?") for l in chord.lanes)
            tag = DECLINED if chord.declined else "HIT" if hit else "MISS"
            if result.sent <= 30 or result.sent % 20 == 0 or not hit:
                print(f"  {result.sent:4d}  {C.ms_to_s(chord.chart_ms):7.2f}  "
                      f"{C.ms_to_s(raw_ms):7.2f}  {diff_ms:+7.1f}  {lane_str:10}  "
                      f"{tag:6}  {streak:6}", file=out)

            score_before = score_after
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
    not.
    LIVE-ONLY: sends input to the game."""
    def press(lanes: List[int], axes: Optional[List[float]] = None) -> object:
        # Keep Clone Hero focused so SendInput reaches it.
        focus_window(hwnd)
        return driver.press_chord(lanes)
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
                             cap_s=cap_s)
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
