"""Play the presses Hydra planned, by reading the song clock and sending inputs.

The presses come from a `hydra_replay schedule` JSON, which says, for every
chord of one path, which gems go down, when, and by when a press must be sent.
Make one with
    hydra_replay schedule --chart <notes.chart or notes.mid> --path <dump.json> --index N --out <schedule.json>
tools/replay_schedule.cpp (schedule_json) owns every field of that file and
every timing number in it. load_schedule reads the file as written and
press_plan returns its presses in firing order; this script works out no
window, lead, end or split of its own.

The auto-player presses each chord's own pads and nothing else. It has no
Star Power key: on drums the game activates from a press on a fill's last
note (docs/clone-hero/05-star-power.md, SP-17). So a fill the path passes over
is declined; the schedule marks its chord `declined` and its press sends the
gems the schedule's `kept_gems` names.

The kit press gives each pad gem the strength its `dynamic` asks for; a key
has no strength. kit_tools.pad_axis owns the strengths, and with_axes reads the
game's dynamics threshold for them when a press holds a ghost gem.

While the song plays, live.wait_until polls the game's song clock and fires
each press at its time; it also notices the song stopping (clock frozen or
jumped back). live.first_note_index picks where a run that joins mid-song
starts. A jump back (a restart or seek) re-syncs to the new clock position and
starts the counts and the press log over. A run also stops at its cap of wall
time: RUN_CAP_S, unless the caller passes a longer one.

The game is opened read-only and the engine is found with
engine_finder.find_live_engine_capped; when no song starts inside its cap the
script prints the finder's message and exits 2. main's docstring says what
every other stop exits with.

With --log, the script writes one CSV row per press, with the columns
overlay_logs.PRESS_LOG_COLUMNS names. score_compare.py reads that file.

Usage:
    python tools\\ch_probe\\experiments\\play_chart.py <schedule.json> [--log <press.csv>]

Run this, then start the song (or start it mid-song).
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import queue
import sys
import threading
import time
from dataclasses import dataclass, field, replace
from typing import Callable, List, Mapping, Optional, Sequence, TextIO, Tuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C, engine_finder
from tools.ch_probe.engine import EngineModel
from tools.ch_probe.process import open_process
from tools.ch_probe.input_driver import (
    LANE_NAMES, InputDriver, Lane, find_game_window, focus_window)
from tools.ch_probe.experiments import live
from tools.ch_probe.experiments.overlay_logs import PRESS_LOG_COLUMNS, SEND_COLUMN
from tools.ch_probe.experiments.live import SongClock

# What the press row and printed line of a press play_chords sent as its plan
# gave it say in the send column (overlay_logs.SEND_COLUMN).
SENT = "sent"

# What a declined chord's press row and printed line say in the send column.
DECLINED = "declined"

# What the press row and printed line of a press play_chords did not send,
# because the game would judge it past its chord's window, say in the send
# column: question 4 of docs/superpowers/plans/2026-11-08-player-freak-fixes.md.
DROPPED = "dropped"

# What an activation's press row says in `end_read_ms` when the game's Star
# Power was off right after the press, so the plan's times stand.
NO_ACTIVATION_READ = "no activation read"

# What a press row says in `retimed` about a press the schedule gave a
# `fallback` (design point 5 of
# docs/superpowers/plans/2026-11-08-replay-press-schedule.md): a squeeze-out's
# late gem kept after the read end, that gem moved in with its chord's last
# inside group (the block's `join_group`), or a squeeze-in pressed plain (the
# block's `plain`). A squeeze-in that keeps its squeeze press leaves it blank.
RETIMED_LATE = "late"
RETIMED_INSIDE = "inside"
RETIMED_PLAIN = "plain"

# The score is read this long after a press: a dense chart's next chord comes
# sooner than constants.INPUT_SETTLE_MS.
SCORE_READ_DELAY_S = 0.005

# A run stops after this much wall time, counted from the first clock read
# (the overlay spike plan, Task B step 1), when its caller gives no cap.
RUN_CAP_S = 240.0

# The key for each gem in a chord's "gems" list, by the gem's color (Kick,
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
class Press:
    """One entry of the schedule's `presses` list, as hydra_replay schedule
    wrote it (schedule_json in tools/replay_schedule.cpp says what each field
    means). `index` is the entry's `chord` and `chart_ms` that chord's `ms`.
    `lanes` and `dynamics` are the keys and dynamics of the entry's `gems`,
    in the same order, and `axes` their strengths once with_axes has worked
    them out. `declined` is the chord's. `planned_end_ms`, on the first press
    of a chord whose `reads` names an activation, is that read's
    `planned_end_ms`: play_chords reads the game's end after it. `retimed`
    is what retime_from_end did with the press's `fallback` (a RETIMED_
    word), None until it ran on a press with one. `seq` is the number
    play_in_game gives the press in the kit agent's schedule, None
    elsewhere; replace() keeps it, so a press retime_from_end moves keeps
    its seq and the agent can tell it from one that already fired. The other
    fields are the entry's own, under the same names."""
    index: int
    chart_ms: float
    aim_ms: float
    lanes: List[int]
    dynamics: List[str]
    axes: Optional[List[float]]
    declined: bool
    last_send_ms: float
    rule: str
    after_ms: Optional[float] = None
    timed_from_end: Optional[int] = None
    end_read_after: Optional[int] = None
    aim_end_ms: Optional[float] = None
    fallback: Optional[Mapping] = None
    planned_end_ms: Optional[float] = None
    retimed: Optional[str] = None
    seq: Optional[int] = None

    @property
    def lead_ms(self) -> float:
        """How far before its chord's time the press fires: the schedule's
        `aim_ms` (negative for an early press) with its sign turned, as
        live.wait_until takes it."""
        return -self.aim_ms

    @property
    def at_ms(self) -> float:
        """The song clock the press fires at."""
        return self.chart_ms + self.aim_ms


@dataclass(frozen=True)
class Chord:
    """One entry of the schedule's `chords` list: its `index`, `tick` and
    `ms`, the keys (`lanes`) and `dynamics` of all its gems in order, whether
    it is `declined` and `ends_sp`, and `presses`, the schedule's presses of
    it in the file's order."""
    index: int
    tick: int
    ms: float
    lanes: List[int]
    dynamics: List[str]
    declined: bool = False
    ends_sp: bool = False
    presses: Tuple[Press, ...] = ()


@dataclass(frozen=True)
class Activation:
    """One entry of the schedule's `activations` list: its `index`, the
    `chord` it starts on, the chord it reads its end after
    (`read_end_after`), and `sp_ready_ms`, when Hydra has the meter ready
    for it (None when the file gives none)."""
    index: int
    chord: Optional[int]
    read_end_after: Optional[int]
    sp_ready_ms: Optional[float]


@dataclass(frozen=True)
class Schedule:
    """What load_schedule reads from a schedule file: its chords in order,
    its activations, and `song_length_ms`, Hydra's song end
    (app::song_length_ms, D255)."""
    chords: List[Chord]
    activations: List[Activation]
    song_length_ms: float


class _Fields:
    """One object of a schedule file, read by key. A missing key raises
    ValueError naming the file, the object and the key, so a file without a
    value the play depends on is refused and never read with a default
    (D318 item 2). `get` is for the keys the file writes only sometimes."""

    def __init__(self, raw, path: str, where: str) -> None:
        if not isinstance(raw, dict):
            raise ValueError(f"{path}: {where} is not an object; make the file again "
                             f"with hydra_replay schedule")
        self._raw, self._path, self._where = raw, path, where

    def __getitem__(self, key: str):
        if key not in self._raw:
            raise ValueError(f"{self._path}: {self._where} has no `{key}`; make the file "
                             f"again with hydra_replay schedule")
        return self._raw[key]

    def get(self, key: str):
        return self._raw.get(key)


def load_schedule(path: str) -> Schedule:
    """The schedule file at `path`, read as written. Raises ValueError,
    quoting the file, when its `refusals` list is not empty or a squeeze in
    it is not `feasible` (question 5 of
    docs/superpowers/plans/2026-11-08-replay-press-schedule.md), and when a
    chord reads the game's end for more than one activation, which no
    play_chords read covers.

    It also raises ValueError, naming the key, for a file without a key it
    reads (D318 item 2), for a null `song_length_ms` (the run's end, D321
    item 2), for an empty chord list (D323 item 2), for a chord with no gems
    (D321 item 3), and for a press that names a chord or gem the file does
    not hold."""
    with open(path, encoding="utf-8") as f:
        doc = _Fields(json.load(f), path, "the file")
    if doc["refusals"]:
        raise ValueError(f"{path} refuses to play this path: " + "; ".join(doc["refusals"]))
    rows = [_Fields(row, path, f"entry {k} of `chords`")
            for k, row in enumerate(doc["chords"])]
    if not rows:
        # The one refusal of a schedule with no chords; every reader of a
        # schedule meets it here, through load_schedule.
        raise ValueError(f"{path} has no chords to play")
    song_length_ms = doc["song_length_ms"]
    if song_length_ms is None:
        raise ValueError(f"{path}: song_length_ms is null, so the file names no song end; "
                         f"make the file again with a hydra_replay that writes one")
    acts = [_Fields(act, path, f"entry {k} of `activations`")
            for k, act in enumerate(doc["activations"])]
    infeasible = []
    for act in acts:
        for k, raw in enumerate(act["squeezes"]):
            q = _Fields(raw, path, f"squeeze {k} of activation {act['index']}")
            if not q["feasible"]:
                infeasible.append(f"chord {q['chord']} (activation {act['index']}, {q['kind']}, "
                                  f"anchor room {q['anchor_room_ms']:g} ms, squeeze room "
                                  f"{q['squeeze_room_ms']:g} ms)")
    if infeasible:
        raise ValueError(f"{path} holds a squeeze the game cannot make at 100% speed: "
                         + "; ".join(infeasible))
    gems_of: dict = {}
    for row in rows:
        gems = row["gems"]
        if not gems:
            raise ValueError(f"{path}: chord {row['index']} has no gems, so no press can "
                             f"play it; make the file again with hydra_replay schedule")
        gems_of[row["index"]] = [_Fields(gem, path, f"gem {g} of chord {row['index']}")
                                 for g, gem in enumerate(gems)]
    by_index = {row["index"]: row for row in rows}
    presses_of: dict = {row["index"]: [] for row in rows}
    for k, raw in enumerate(doc["presses"]):
        entry = _Fields(raw, path, f"entry {k} of `presses`")
        if entry["chord"] not in by_index:
            raise ValueError(f"{path}: entry {k} of `presses` names chord {entry['chord']}, "
                             f"which `chords` does not hold")
        row = by_index[entry["chord"]]
        row_gems = gems_of[row["index"]]
        if any(not isinstance(g, int) or not 0 <= g < len(row_gems) for g in entry["gems"]):
            raise ValueError(f"{path}: entry {k} of `presses` names gems {entry['gems']} of "
                             f"chord {row['index']}, which holds {len(row_gems)}")
        gems = [row_gems[g] for g in entry["gems"]]
        presses_of[row["index"]].append(Press(
            index=row["index"], chart_ms=row["ms"], aim_ms=entry["aim_ms"],
            lanes=[_KEY_FOR_PAD[(gem["color"], gem["cymbal"])] for gem in gems],
            dynamics=[gem["dynamic"] for gem in gems], axes=None,
            declined=row["declined"], last_send_ms=entry["last_send_ms"],
            rule=entry["rule"], after_ms=entry["after_ms"],
            timed_from_end=entry["timed_from_end"],
            end_read_after=entry["end_read_after"], aim_end_ms=entry["aim_end_ms"],
            fallback=entry.get("fallback")))
    chords = []
    for row in rows:
        presses = presses_of[row["index"]]
        reads = row["reads"]
        if len(reads) > 1:
            raise ValueError(f"{path}: chord {row['index']} reads the end of "
                             f"{len(reads)} activations; play_chords reads one")
        if reads and presses:
            first = min(range(len(presses)), key=lambda k: presses[k].at_ms)
            read = _Fields(reads[0], path, f"the end read of chord {row['index']}")
            presses[first] = replace(presses[first], planned_end_ms=read["planned_end_ms"])
        gems = gems_of[row["index"]]
        chords.append(Chord(
            index=row["index"], tick=row["tick"], ms=row["ms"],
            lanes=[_KEY_FOR_PAD[(gem["color"], gem["cymbal"])] for gem in gems],
            dynamics=[gem["dynamic"] for gem in gems],
            declined=row["declined"], ends_sp=row["ends_sp"], presses=tuple(presses)))
    activations = [Activation(act["index"], act["chord"], act["read_end_after"],
                              act["sp_ready_ms"]) for act in acts]
    return Schedule(chords, activations, song_length_ms)


def press_plan(chords: Sequence[Chord]) -> List[Press]:
    """Every press of `chords`, in the order they fire. The schedule worked
    each one out; this only sorts them by the clock they fire at."""
    return sorted((p for chord in chords for p in chord.presses), key=lambda p: p.at_ms)


def retime_from_end(presses: Sequence[Press], read_after: int, read_end_ms: float,
                    chord_lanes: Optional[Mapping[int, Sequence[int]]] = None) -> List[Press]:
    """`presses` in firing order, with every press the schedule times from
    the end read after chord `read_after` (Press.end_read_after) moved by how
    far the end the game stored, `read_end_ms`, is from that press's own
    `aim_end_ms`. Design point 1 of
    docs/superpowers/plans/2026-11-07-player-limits.md says why: the game
    stamps an activation with the clock of the frame that read the press.

    A moved press with a `fallback` then does what the block says (design
    point 5 of docs/superpowers/plans/2026-11-08-replay-press-schedule.md),
    and Press.retimed says which. A read end past `if_end_after_ms` makes
    the late gem `join_group`: it goes down `before_end_ms` before the read
    end, in one press with its chord's group of `join_gems` (indexes into
    the chord's lanes, from `chord_lanes` by chord index) when that names
    any. A read end before `if_end_before_ms` makes the press `plain`, at
    the block's `aim_ms`. A read end equal to a threshold keeps the press.
    Raises ValueError for a moved press with no `aim_end_ms`, a block with
    another action, or a `join_gems` group no press holds, each a bug in
    the schedule."""
    moved = []
    for p in presses:
        if p.end_read_after != read_after:
            moved.append(p)
            continue
        if p.aim_end_ms is None:
            raise ValueError(f"chord {p.index} is timed from the end read after chord "
                             f"{read_after} but has no aim_end_ms")
        delta_ms = read_end_ms - p.aim_end_ms
        moved.append(replace(p, aim_ms=p.aim_ms + delta_ms, aim_end_ms=read_end_ms))
    out: List[Press] = []
    joins: List[Tuple[Press, Mapping]] = []
    for p in moved:
        block = p.fallback
        if p.end_read_after != read_after or block is None:
            out.append(p)
        elif block.get("action") == "join_group":
            if read_end_ms > block["if_end_after_ms"]:
                joins.append((replace(p, aim_ms=read_end_ms - block["before_end_ms"] - p.chart_ms,
                                      retimed=block["retimed"]), block))
            else:
                out.append(replace(p, retimed=RETIMED_LATE))
        elif block.get("action") == "plain":
            if read_end_ms < block["if_end_before_ms"]:
                out.append(replace(p, aim_ms=block["aim_ms"], retimed=block["retimed"]))
            else:
                out.append(p)
        else:
            raise ValueError(f"chord {p.index}'s fallback has action {block.get('action')!r}, "
                             f"which play_chords does not know")
    for late, block in joins:
        if not block["join_gems"]:
            out.append(late)
            continue
        lanes = chord_lanes[late.index] if chord_lanes is not None else None
        want = None if lanes is None else sorted(lanes[g] for g in block["join_gems"])
        k = next((k for k, q in enumerate(out) if q.index == late.index
                  and q.end_read_after == read_after and sorted(q.lanes) == want), None)
        if k is None:
            raise ValueError(f"chord {late.index}'s late gem joins gems {block['join_gems']}, "
                             f"which no press of it holds")
        group = out[k]
        axes = (None if group.axes is None or late.axes is None
                else list(group.axes) + list(late.axes))
        out[k] = replace(group, lanes=list(group.lanes) + list(late.lanes),
                         dynamics=list(group.dynamics) + list(late.dynamics), axes=axes,
                         retimed=late.retimed)
    return sorted(out, key=lambda p: p.at_ms)


def press_messages(presses: Sequence[Press]) -> List[Tuple[Press, List[Press]]]:
    """`presses` in firing order as the messages play_chords sends: each a
    first press and the later order groups (Press.after_ms) that go down
    with it in one message, in the order their first presses fire. A later
    group goes with its chord's press that fires `after_ms` before it.
    Raises ValueError for a later group with no such press, which would be a
    bug in the schedule."""
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


# How close two press times must be to count as one: the schedule's times are
# sums of the same numbers, so anything past float rounding is another time.
_SAME_MS = 1e-6


def load_score_times(path: str) -> List[Tuple[int, float]]:
    """The score file's chords as (`index`, ms), in order. It reads nothing a
    squeeze-out's dump would add, so a caller that needs only the times (the
    trace's note list) gets them for any score file."""
    with open(path, encoding="utf-8") as f:
        return [(chord["index"], chord["ms"]) for chord in json.load(f)["chords"]]


def with_axes(chords: Sequence[Chord], engine) -> Tuple[List[Chord], Optional[int]]:
    """`chords` with each press's gem strengths from kit_tools.pad_axis, and
    the dynamics threshold read for them. The field is read once, and only
    when a pressed gem is a ghost; otherwise the threshold is None. Every
    strength is worked out here, so a threshold no ghost can use raises
    pad_axis's ValueError before the first press."""
    # Imported here because kit_tools imports game_tools, which imports this
    # module.
    from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis
    threshold = None
    if any(DYNAMIC_GHOST in p.dynamics for chord in chords for p in chord.presses):
        threshold = engine.dyn_threshold()
    return [replace(chord, presses=tuple(
                replace(p, axes=[pad_axis(dynamic, threshold) for dynamic in p.dynamics])
                for p in chord.presses))
            for chord in chords], threshold


# --- one run ------------------------------------------------------------------

class RunCapReached(Exception):
    """The run has used its cap of wall time."""


class StopAsked(Exception):
    """The caller's `should_stop` said to end the run."""


class ReplyMissing(Exception):
    """A kit press replied without what the press asked for
    (kit_tools.reply_gap says what); the text names the chord."""


# The `stop` of a run that sent every press: the only stop that reached the
# song's end.
STOP_DONE = "done"

# The `stop` of a live run that never found a playing song's engine.
STOP_NO_ENGINE = "no engine"


@dataclass
class RunResult:
    """What one run did. `rows` are the press-log rows, keyed by the names in
    PRESS_LOG_COLUMNS, plus `lead_ms`, the lead the press used, which no log
    column holds. A split chord (press_plan) has a row per press, all with
    its `index`. Each row's send column (overlay_logs.SEND_COLUMN) says SENT,
    DECLINED for a declined chord, or DROPPED on each row of a message
    play_chords dropped. `sent` counts the messages play_chords sends
    (press_messages: a chord's order groups are one) and leaves declined
    chords and dropped messages out; `declined` and `dropped` count those.
    No count here says whether the game took a press: score_compare judges
    a chord by the game's score step (D319 item 1). `skipped` counts the chords the run passed over where it
    started (start_line says which). `stop` says why the run ended:
    STOP_DONE (every chord pressed), "past_end" (the clock was past the last
    chord), "frozen", "cap", "interrupted", "lost" (the process went away),
    "stopped" (the caller's `should_stop` said so), "reply" (a kit reply
    lacked what its press asked for) or STOP_NO_ENGINE (live_play found no
    playing song). For every stop but STOP_DONE, `message` says why in
    words: the finder's for STOP_NO_ENGINE. `dyn_threshold` is the dynamics
    threshold live_play read, None when it read none."""
    sent: int = 0
    declined: int = 0
    dropped: int = 0
    skipped: int = 0
    rows: list = field(default_factory=list)
    stop: str = ""
    message: str = ""
    dyn_threshold: Optional[int] = None


def start_line(raw_s: float, messages: Sequence[Tuple[Press, List[Press]]],
               cursor: int) -> Tuple[str, int]:
    """The line a run prints where it starts, and how many chords it skips:
    the chords with a press among `messages[:cursor]`, the messages
    live.first_note_index passed over for the clock read `raw_s`. A chord
    counts once however many of its presses were passed over."""
    skipped = len({p.index for head, later in messages[:cursor] for p in [head, *later]})
    chords = f"{skipped} chord{'' if skipped == 1 else 's'}"
    if cursor >= len(messages):
        return (f"  Song clock at {raw_s:.2f}s is past the last note; nothing to play "
                f"(skipped {chords}).", skipped)
    return (f"  Song clock at {raw_s:.2f}s; starting at press {cursor + 1}/{len(messages)} "
            f"(t={C.ms_to_s(messages[cursor][0].chart_ms):.2f}s); skipped {chords} "
            f"less than {live.START_LEAD_MS:g} ms ahead of the clock.", skipped)


class _TimedWait:
    """The clock and the sleep play_chords gives live.wait_until, each timed
    with `now`, so a row can say how long the wait's slowest clock read took
    and how far its worst sleep ran past what it asked. reset starts the
    two over for the next message."""

    def __init__(self, clock, now: Callable[[], float],
                 sleep: Callable[[float], None]) -> None:
        self._clock, self._now, self._sleep = clock, now, sleep
        self.reset()

    def reset(self) -> None:
        self.read_max_ms = 0.0
        self.oversleep_max_ms = 0.0

    def read(self):
        began = self._now()
        value = self._clock.read()
        self.read_max_ms = max(self.read_max_ms, C.s_to_ms(self._now() - began))
        return value

    def sleep(self, seconds: float) -> None:
        began = self._now()
        self._sleep(seconds)
        self.oversleep_max_ms = max(self.oversleep_max_ms,
                                    C.s_to_ms(self._now() - began - seconds))


def play_chords(chords: Sequence[Chord], clock, read_score: Callable[[], int],
                press: Callable[..., object], *,
                now: Callable[[], float] = time.perf_counter,
                sleep: Callable[[float], None] = time.sleep,
                wall: Callable[[], float] = time.time,
                on_press: Optional[Callable[[dict], object]] = None,
                should_stop: Optional[Callable[[], bool]] = None,
                out: Optional[TextIO] = None,
                cap_s: float = RUN_CAP_S,
                read_sp_end: Optional[Callable[[], Optional[float]]] = None,
                kit: bool = False) -> RunResult:
    """Fire press_plan's presses of `chords` from where the song clock is
    now, in the order they fire, as press_messages groups them, and log a
    row per press.

    `read_sp_end`, when given, reads the game's Star Power end in song
    seconds, None while Star Power is off (EngineModel.sp_end). After a
    message whose first press carries Press.planned_end_ms (the first press
    of each chord the schedule's `reads` names, so the activation's chord and
    its `read_end_after` chord each get a read) it is read once: the first
    row's `end_read_ms` holds the end in ms, and retime_from_end moves the
    later presses timed from the end read after that chord and applies their
    fallbacks; or the row says NO_ACTIVATION_READ and the plan stands. A row
    of a press retime_from_end marked says so in `retimed`.

    `clock` reads the song clock (live.ReadsSongClock), `read_score` reads the
    engine's score and `press` sends one chord's keys and their `axes`. A
    chord split in order groups is one call, with `then=` a list of
    (lanes, axes, after_ms) for its later groups; every other press is
    called with the keys and axes alone. With `kit` True, `press` is the kit
    press of tools/ch_mcp/kit_tools.py: each reply must hold what
    kit_tools.reply_gap asks of it, else the run stops at that chord as
    "reply" (D318 item 2), and each row keeps the reply's values that
    PRESS_LOG_COLUMNS names, from the reply's `groups` entry for its group
    when the press sent later groups. A keys press's reply is not read.

    Design point 9 of docs/superpowers/plans/2026-11-07-player-limits.md
    says how a row's score is read: the score is read fresh just before each
    message, its rise since the last row's `score_after` goes in the first
    row's `scored_between`, and that fresh read is every row's
    `score_before`. The score is read once after the reply, so every row of
    a split chord holds the same `score_after`.

    A message read past its first press's Press.last_send_ms is not sent
    (design point 5 of
    docs/superpowers/plans/2026-11-08-player-freak-fixes.md): it reads no end
    and no score after, and its rows say DROPPED in the send column with the
    clock it was dropped at.

    The first row of each message carries overlay_logs.TIMING_PRESS_COLUMNS,
    each in ms on the `now` timer, so a stall names the step it happened in
    (PS3 of docs/superpowers/plans/2026-11-12-player-stall-fixes.md).
    `wait_read_max_ms` is the slowest single clock read of the wait for the
    message, and `wait_oversleep_max_ms` the most a sleep of that wait ran
    past what it asked. `press_took_ms` is the press call, `reply_check_took_ms`
    the kit reply's check, and `end_read_took_ms` the Star Power end read with
    its retime; each is left out when its step did not run. `score_read_took_ms`
    adds up the score read before the press and the one after it, without the
    SCORE_READ_DELAY_S sleep between them. `prev_handoff_took_ms` is how long
    `on_press` took for the message before, since a row is handed over before
    its own hand-off ends; it is left out on the first message and when there
    is no `on_press`.

    The live script passes the game's; the tests pass fakes. `now` and
    `sleep` are the timer the stop checks and the waits use. `wall` stamps
    each row's wall columns: it is the watch's clock, because `now` shares no
    zero with the watch or a trace. `on_press`, when given, gets each
    press-log row as it is added; `should_stop`, when given, is asked with
    the run cap and ends the run as "stopped" when it returns True. `cap_s`
    is the run cap in wall seconds."""
    out = sys.stdout if out is None else out
    reply_gap = None
    if kit:
        # Imported here because kit_tools imports game_tools, which imports
        # this module.
        from tools.ch_mcp.kit_tools import reply_gap
    plan = press_messages(press_plan(chords))
    chord_lanes = {c.index: c.lanes for c in chords}
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
    line, result.skipped = start_line(raw_s, presses, cursor)
    print(line, file=out)
    if cursor >= len(presses):
        result.stop = "past_end"
        result.message = f"the song clock at {raw_s:.2f}s is past the last note"
        return result

    # The score the last row ended on: what scored_between is counted from.
    last_score = read_score()

    print(f"\n  {'#':>4}  {'chart_t':>7}  {'clock':>7}  {'diff_ms':>7}  {'lanes':10}  "
          f"{'send':8}", file=out)

    def took_ms(began: float) -> float:
        return C.s_to_ms(now() - began)

    timed_wait = _TimedWait(clock, now, sleep)
    # How long on_press took for the message before (prev_handoff_took_ms).
    prev_handoff_ms: Optional[float] = None

    try:
        while cursor < len(presses):
            # One message: a chord, with any later order groups of it, or one
            # part of a squeezed-out chord.
            chord, later = presses[cursor]
            lead_ms = chord.lead_ms
            timed_wait.reset()
            try:
                raw_ms, _est_ms = live.wait_until(
                    timed_wait, chord.chart_ms, lead_ms=lead_ms, stopped=stopped,
                    should_stop=over_cap, now=now, sleep=timed_wait.sleep)
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
                line, result.skipped = start_line(raw_s, presses, cursor)
                print(line, file=out)
                if cursor >= len(presses):
                    result.stop = "past_end"
                    result.message = (f"song {e}, and the clock at {raw_s:.2f}s is past "
                                      f"the last note")
                    print("\n  Stopped.", file=out)
                    return result
                continue
            wall_s_at_read = wall()
            began = now()
            score_before = read_score()
            score_read_ms = took_ms(began)

            # A message whose first press the game would judge past its
            # chord's window is not sent, later groups and all: design point 5
            # of docs/superpowers/plans/2026-11-08-player-freak-fixes.md says
            # why raw_ms is the read it is judged from.
            sends = not (chord.declined and not chord.lanes)
            dropped = sends and raw_ms > chord.last_send_ms

            # Each step's time, None while the step has not run.
            steps_ms: dict = {}
            # A declined chord in a mode that keeps no gem sends no press.
            if not sends or dropped:
                pressed = None
            else:
                began = now()
                if later:
                    pressed = press(chord.lanes, chord.axes,
                                    then=[(p.lanes, p.axes, p.after_ms) for p in later])
                else:
                    pressed = press(chord.lanes, chord.axes)
                steps_ms["press_took_ms"] = took_ms(began)
            wall_s_at_reply = wall()
            if reply_gap is not None and sends and not dropped:
                began = now()
                gap = reply_gap(pressed, 1 + len(later))
                steps_ms["reply_check_took_ms"] = took_ms(began)
                if gap is not None:
                    raise ReplyMissing(f"the kit's reply to chord {chord.index}'s press {gap}")

            end_read = None
            if chord.planned_end_ms is not None and read_sp_end is not None and not dropped:
                began = now()
                end_s = read_sp_end()
                if end_s is None:
                    end_read = NO_ACTIVATION_READ
                else:
                    end_read = C.s_to_ms(end_s)
                    rest = [p for head, rest_later in presses[cursor + 1:]
                            for p in [head, *rest_later]]
                    presses[cursor + 1:] = press_messages(retime_from_end(
                        rest, chord.index, end_read, chord_lanes))
                    notes_ms = [head.at_ms for head, _later in presses]
                steps_ms["end_read_took_ms"] = took_ms(began)

            # Nothing was sent for a dropped message, so the fresh read stands.
            if dropped:
                score_after = score_before
            else:
                sleep(SCORE_READ_DELAY_S)
                began = now()
                score_after = read_score()
                score_read_ms += took_ms(began)

            if dropped:
                result.dropped += 1
            elif chord.declined:
                result.declined += 1
            else:
                result.sent += 1
            # A kit press replies with what the game's frames did, per group
            # for a message with later groups; reply_gap checked it holds them.
            replies = ([] if reply_gap is None or pressed is None
                       else pressed["groups"] if later else [pressed])
            handoff_ms = 0.0
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
                    SEND_COLUMN: DROPPED if dropped else DECLINED if part.declined else SENT,
                    "wall_s_at_read": wall_s_at_read,
                    "wall_s_at_reply": wall_s_at_reply,
                }
                if part.retimed is not None:
                    row["retimed"] = part.retimed
                if k == 0:
                    row["scored_between"] = score_before - last_score
                    if end_read is not None:
                        row["end_read_ms"] = end_read
                    row["wait_read_max_ms"] = timed_wait.read_max_ms
                    row["wait_oversleep_max_ms"] = timed_wait.oversleep_max_ms
                    row.update(steps_ms)
                    row["score_read_took_ms"] = score_read_ms
                    if prev_handoff_ms is not None:
                        row["prev_handoff_took_ms"] = prev_handoff_ms
                if k < len(replies):
                    # The row keeps the parts the press log has columns for.
                    row.update((key, replies[k][key]) for key in PRESS_LOG_COLUMNS
                               if key in replies[k] and key not in row)
                result.rows.append(row)
                if on_press is not None:
                    began = now()
                    on_press(row)
                    handoff_ms += took_ms(began)
            if on_press is not None:
                prev_handoff_ms = handoff_ms

            diff_ms = raw_ms - chord.chart_ms
            lane_str = " then ".join("+".join(LANE_NAMES.get(l, "?") for l in part.lanes)
                                     for part in [chord, *later])
            tag = DROPPED if dropped else DECLINED if chord.declined else SENT
            if result.sent <= 30 or result.sent % 20 == 0 or tag != SENT:
                print(f"  {result.sent:4d}  {C.ms_to_s(chord.chart_ms):7.2f}  "
                      f"{C.ms_to_s(raw_ms):7.2f}  {diff_ms:+7.1f}  {lane_str:10}  "
                      f"{tag:8}", file=out)

            last_score = score_after
            cursor += 1
        result.stop = STOP_DONE

    except live.ClockFrozen as e:
        print(f"  {e}. Song ended or paused?", file=out)
        print("\n  Stopped.", file=out)
        result.stop, result.message = "frozen", str(e)
    except RunCapReached as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop, result.message = "cap", str(e)
    except StopAsked as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop, result.message = "stopped", str(e)
    except ReplyMissing as e:
        print(f"\n  Stopped: {e}.", file=out)
        result.stop, result.message = "reply", str(e)
    except KeyboardInterrupt:
        print("\n  Stopped.", file=out)
        result.stop, result.message = "interrupted", "interrupted from the keyboard"
    except OSError as e:
        print(f"\n  Lost process: {e}", file=out)
        result.stop, result.message = "lost", f"lost the game process: {e}"
    return result


# --- in-game timing -------------------------------------------------------------


def engine_addresses(engine) -> Tuple[int, int]:
    """The song clock's and the score's addresses on `engine`, which the
    `play` message hands the kit agent (D339 item 4)."""
    return engine._addr(C.OFF_SONG_CLOCK), engine._addr(C.OFF_SCORE)


def play_in_game(chords: Sequence[Chord], player, *, clock_addr: int, score_addr: int,
                 now: Callable[[], float] = time.perf_counter,
                 on_press: Optional[Callable[[dict], object]] = None,
                 should_stop: Optional[Callable[[], bool]] = None,
                 out: Optional[TextIO] = None,
                 cap_s: float = RUN_CAP_S,
                 read_sp_end: Optional[Callable[[], Optional[float]]] = None) -> RunResult:
    """Hand press_plan's presses of `chords` to the kit agent in one `play`
    message and make a press-log row of each event it reports, as
    play_chords would for the same press (design points 1 to 5 of
    docs/superpowers/plans/2026-11-12-in-game-press-timing.md). The agent
    fires every press on the game's frames; nothing here waits for a press
    time. kit_tools' contract block names every message and event field.

    `player` is a kit_tools.GamePlayer (post, events, alive). A row's
    `clock_ms_at_press` is the clock of the frame the press went down on,
    or was dropped on; `score_before` is the score on that frame and
    `score_after` the agent's read after the release. `wall_s_at_read` and
    `wall_s_at_reply` are the down and up times the agent stamped, in
    seconds. The timing columns stay empty; `frame_gap_ms` and `agent_note`
    hold the event's frame gap and note. `scored_between` is counted as
    play_chords counts it, on the first row of each message.

    After the event of a press carrying Press.planned_end_ms, `read_sp_end`
    is read once, and retime_from_end moves the presses not yet reported;
    the moved ones go to the agent in one `retime` message, each under the
    seq it already had (Press.seq).

    The agent's stop reasons map onto play_chords' stop words: done is
    STOP_DONE, and past_end, stopped, cap and frozen keep their own word;
    jumped_back sends the original plan again and starts the counts over. An
    event this run cannot place (an unknown seq, kind or reason, or a sent
    or dropped event missing a field, or an error the agent reports) ends
    the run as "reply" after a `stop_play`; a lost session ends it as
    "lost". A stop event's `cause` and `lift_error` go in the run's message,
    and a "done" that carries either ends as "reply". `should_stop` is asked
    while no event comes, and a True sends `stop_play`. A `retime_missed`
    note is named in the run's message.

    A retime the agent refuses after its play has ended (the end read was
    slow, so the agent's stop is already queued) changes nothing the agent
    plays: the run reads the queued events as the agent sent them, ends with
    the stop word their stop gives, and names the refused retime in the
    run's message. Any other failure of a retime's post ends the run as
    "reply"."""
    from collections import deque
    from tools.ch_mcp import frida_attach as FA
    from tools.ch_mcp import kit_tools as K
    out = sys.stdout if out is None else out
    chord_lanes = {c.index: c.lanes for c in chords}
    # Each press keeps its seq for the whole run, whatever retime_from_end
    # does to it, so a retime of a press that already fired (its event still
    # unread here) names a seq the agent knows and ignores.
    plan = [replace(p, seq=k) for k, p in enumerate(press_plan(chords))]

    def entries_for(presses: Sequence[Press], context: Sequence[Press] = ()) -> List[dict]:
        # A later group's after_seq is its head's seq, which press_messages
        # finds among `presses` and `context` (the presses already reported).
        head_of = {id(q): head for head, later in press_messages([*context, *presses])
                   for q in later}
        return [K.play_entry(p.seq, p, head_of[id(p)].seq if id(p) in head_of else None)
                for p in presses]

    def start() -> dict:
        player.post(K.play_message(
            entries_for(plan), clock_addr=clock_addr, score_addr=score_addr,
            score_delay_ms=C.s_to_ms(SCORE_READ_DELAY_S), start_lead_ms=live.START_LEAD_MS,
            cap_ms=C.s_to_ms(cap_s), stall_ms=C.s_to_ms(live.STALL_S),
            jump_back_ms=C.s_to_ms(live.JUMP_BACK_S)))
        return {p.seq: p for p in plan}

    by_seq = start()
    every_seq = dict(by_seq)
    result = RunResult()
    reported: set = set()
    missed: List[int] = []
    # Why each refused retime of a play that had ended was refused.
    refused: List[str] = []
    # Events taken off player.events to look for the agent's stop, read
    # before any new one.
    backlog: deque = deque()
    last_score: Optional[int] = None
    stopping_since: Optional[float] = None

    def play_ended() -> bool:
        # The agent sends its stop before it answers any later message, and
        # Frida hands both over in that order, so when a retime is refused
        # because the play ended, its stop is already queued.
        while True:
            try:
                backlog.append(player.events.get_nowait())
            except queue.Empty:
                break
        return any(isinstance(e, dict) and e.get("kind") == K.EVENT_STOP for e in backlog)

    def fail(stop: str, message: str) -> RunResult:
        try:
            player.post(K.stop_play_message())
        except Exception:  # the stop is said in `message` already
            pass
        result.stop, result.message = stop, message
        print(f"\n  Stopped: {message}.", file=out)
        return result

    while True:
        try:
            event = (backlog.popleft() if backlog
                     else player.events.get(timeout=K.KIT_EVENT_WAIT_S))
        except queue.Empty:
            if not player.alive():
                result.stop, result.message = "lost", "lost the kit agent's session"
                return result
            if stopping_since is None and should_stop is not None and should_stop():
                stopping_since = now()
                player.post(K.stop_play_message())
            elif stopping_since is not None and now() - stopping_since > K.KIT_PRESS_CAP_S:
                result.stop = "stopped"
                result.message = (f"asked to stop; the agent sent no stop within "
                                  f"{K.KIT_PRESS_CAP_S:g} s (KIT_PRESS_CAP_S)")
                return result
            continue
        kind = event.get("kind") if isinstance(event, dict) else None
        if kind == K.EVENT_SKIPPED:
            seqs = list(event.get("seqs") or [])
            unknown = [s for s in seqs if s not in by_seq]
            if unknown:
                return fail("reply", f"the kit agent skipped seqs {unknown}, which no entry "
                                     f"holds")
            result.skipped = len({by_seq[s].index for s in seqs})
            reported.update(seqs)
            print(f"  Song clock at {C.ms_to_s(event.get('clock_ms') or 0):.2f}s; "
                  f"skipped {result.skipped} chords less than {live.START_LEAD_MS:g} ms "
                  f"ahead of the clock.", file=out)
            continue
        if kind == K.EVENT_STOP:
            reason = event.get("reason")
            if reason == K.STOP_REASON_JUMPED_BACK:
                print("\n  Song clock jumped back. Re-syncing.", file=out)
                by_seq = start()
                every_seq.update(by_seq)
                result, reported, missed, last_score = RunResult(), set(), [], None
                refused = []
                continue
            words = {K.STOP_REASON_DONE: STOP_DONE, K.STOP_REASON_PAST_END: "past_end",
                     K.STOP_REASON_STOPPED: "stopped", K.STOP_REASON_CAP: "cap",
                     K.STOP_REASON_FROZEN: "frozen"}
            if reason not in words:
                return fail("reply", f"the kit agent stopped for a reason this run does "
                                     f"not know: {reason!r}")
            result.stop = words[reason]
            notes = []
            if result.stop == STOP_DONE:
                lost = [s for s in by_seq if s not in reported]
                if lost:
                    return fail("reply", f"the kit agent ended with no report for seqs "
                                         f"{lost}")
            else:
                notes.append(f"the kit agent stopped: {reason}")
            # What stopped the play from outside the scheduler, and a lift
            # that failed (so a pad may still be down), are never left out.
            cause, lift_error = event.get("cause"), event.get("lift_error")
            if cause:
                notes.append(f"cause: {cause}")
            if lift_error:
                notes.append(f"the agent could not lift its pads, so one may still be down: "
                             f"{lift_error}")
            if (cause or lift_error) and result.stop == STOP_DONE:
                result.stop = "reply"
            if missed:
                notes.append(f"{K.NOTE_RETIME_MISSED} on chords "
                             f"{', '.join(str(every_seq[s].index) for s in missed)}: each fired "
                             f"at its planned time before its retime arrived")
            notes.extend(refused)
            result.message = "; ".join(notes)
            return result
        if kind == K.EVENT_ERROR:
            return fail("reply", f"the kit agent reported an error: {event.get('error')}")
        if kind not in (K.EVENT_SENT, K.EVENT_DROPPED):
            return fail("reply", f"the kit agent sent an event this run does not know: "
                                 f"{event!r}")
        lacking = [f for f in K.PRESS_EVENT_FIELDS if f not in event]
        if lacking:
            return fail("reply", f"the kit agent's {kind} event lacks {', '.join(lacking)}: "
                                 f"{event!r}")
        seq = event["seq"]
        if seq not in every_seq:
            return fail("reply", f"the kit agent reported seq {seq!r}, which no entry holds")
        part = every_seq[seq]
        reported.add(seq)
        dropped = kind == K.EVENT_DROPPED
        send = DROPPED if dropped else DECLINED if part.declined else SENT
        note = event["agent_note"] or ""
        row = {
            "index": part.index, "chart_ms": part.chart_ms,
            "clock_ms_at_press": event["clock_ms_at_press"], "lead_ms": part.lead_ms,
            "score_before": event["score_before"],
            # A press the agent's stop lifted before its score read has none.
            "score_after": (event["score_before"] if event["score_after"] is None
                            else event["score_after"]),
            SEND_COLUMN: send, "agent_note": note,
        }
        if event["frame_gap_ms"] is not None:
            row["frame_gap_ms"] = event["frame_gap_ms"]
        if not dropped:
            row.update({key: event[key] for key in ("down_frame", "up_frame",
                                                    "down_ms", "up_ms")
                        if event[key] is not None})
            row["wall_s_at_read"] = C.ms_to_s(event["down_ms"])
            if event["up_ms"] is not None:
                row["wall_s_at_reply"] = C.ms_to_s(event["up_ms"])
        if part.retimed is not None:
            row["retimed"] = part.retimed
        if part.after_ms is None:
            # The first row of a message, as play_chords counts it.
            row["scored_between"] = (0 if last_score is None
                                     else event["score_before"] - last_score)
            if dropped:
                result.dropped += 1
            elif part.declined:
                result.declined += 1
            else:
                result.sent += 1
        last_score = row["score_after"]
        if note == K.NOTE_RETIME_MISSED:
            missed.append(seq)
        post_failure: Optional[str] = None
        if part.planned_end_ms is not None and read_sp_end is not None and not dropped:
            # Every end read posts a retime of every entry not yet reported,
            # even one that moves nothing: the agent replaces its whole set
            # of unfired entries, and the retime's arrival is what clears
            # retime_missed for this read (IG1's contract).
            end_s = read_sp_end()
            rest = sorted((p for s, p in by_seq.items() if s not in reported),
                          key=lambda p: p.at_ms)
            if end_s is None:
                row["end_read_ms"] = NO_ACTIVATION_READ
                moved = rest
            else:
                row["end_read_ms"] = C.s_to_ms(end_s)
                moved = retime_from_end(rest, part.index, row["end_read_ms"], chord_lanes)
            entries = entries_for(moved, [p for s, p in by_seq.items() if s in reported])
            played_by_seq, played_every_seq = by_seq, dict(every_seq)
            by_seq = {s: p for s, p in by_seq.items() if s in reported}
            by_seq.update({p.seq: p for p in moved})
            every_seq.update({p.seq: p for p in moved})
            try:
                player.post(K.retime_message(part.index, entries))
            except Exception as e:  # each failure is said in the run's message
                if (isinstance(e, FA.ReadFailed) and e.reason == FA.END_AGENT_ERROR
                        and play_ended()):
                    # The agent's play ended on the presses as they were, so
                    # the rows still queued are made from those.
                    by_seq, every_seq = played_by_seq, played_every_seq
                    refused.append(f"the retime after chord {part.index}'s end read was "
                                   f"refused because the kit agent's play had already "
                                   f"ended: {e}")
                else:
                    post_failure = (f"the retime after chord {part.index}'s end read "
                                    f"failed: {e}")
        result.rows.append(row)
        if on_press is not None:
            on_press(row)
        if post_failure is not None:
            return fail("reply", post_failure)
        if send != SENT or result.sent <= 30 or result.sent % 20 == 0 or row["agent_note"]:
            print(f"  {result.sent:4d}  {C.ms_to_s(part.chart_ms):7.2f}  "
                  f"{C.ms_to_s(row['clock_ms_at_press']):7.2f}  {send:8} {note}",
                  file=out)


def write_press_log(path: str, rows: Sequence[dict]) -> None:
    """Write the press log: the PRESS_LOG_COLUMNS header, then one row per
    press. A row's other values stay out of the file."""
    with open(path, "w", encoding="utf-8", newline="") as f:
        writer = _press_log_writer(f)
        writer.writeheader()
        writer.writerows(rows)


class PressLogWriter:
    """The press log of a run as it is made, written on a thread of its own,
    so a write that Windows holds up costs the player's thread nothing (PS2
    of docs/superpowers/plans/2026-11-12-player-stall-fixes.md).

    start opens the file, which starts it over, and writes the header; add
    queues a row and makes no file call. Each row is flushed before the next
    is taken, so a worker that dies keeps every row so far. A write that
    fails is kept in `error` and ends the writing; the rows added after it
    are not written, and the caller writes the whole log at the end."""

    _STOP = object()

    def __init__(self, path: str) -> None:
        self.path = path
        self.error: Optional[Exception] = None
        self._rows: "queue.Queue" = queue.Queue()
        self._thread = threading.Thread(target=self._body, name="press log writer",
                                        daemon=True)

    def start(self) -> None:
        self._thread.start()

    def add(self, row: dict) -> None:
        # A copy, so the row the file gets is the row as it was added.
        self._rows.put(dict(row))

    def stop(self, cap_s: float) -> bool:
        """Ask the writer to finish the rows it holds and close the file,
        and wait up to `cap_s` for it. False when it was still writing."""
        self._rows.put(self._STOP)
        self._thread.join(cap_s)
        return not self._thread.is_alive()

    def _body(self) -> None:
        try:
            with open(self.path, "w", encoding="utf-8", newline="") as f:
                writer = _press_log_writer(f)
                writer.writeheader()
                f.flush()
                while True:
                    row = self._rows.get()
                    if row is self._STOP:
                        return
                    writer.writerow(row)
                    f.flush()
        except Exception as e:  # said by the caller, which writes the log whole
            self.error = e


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
    play_chords, and a given `press` goes as the kit press (`kit`). `open_proc` and `find_engine` stand in for open_process and
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
        kit = press is not None
        if kit:
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
                             cap_s=cap_s, read_sp_end=engine.sp_end, kit=kit)
        result.dyn_threshold = threshold
        return result
    finally:
        proc.close()


def live_in_game(chords: Sequence[Chord], *, player,
                 open_proc: Callable = open_process,
                 find_engine: Optional[Callable[[object], int]] = None,
                 on_press: Optional[Callable[[dict], object]] = None,
                 should_stop: Optional[Callable[[], bool]] = None,
                 out: Optional[TextIO] = None,
                 cap_s: float = RUN_CAP_S) -> RunResult:
    """live_play for in-game timing: open the game read-only, find the
    engine as live_play does, work out the strengths (with_axes), and hand
    the run to play_in_game on `player` (a kit_tools.GamePlayer), with the
    engine's clock and score addresses and its Star Power end read.
    LIVE-ONLY."""
    out = sys.stdout if out is None else out
    if find_engine is None:
        def find_engine(proc) -> int:
            return find_normal_engine(proc, out=out)
    proc = open_proc(read_only=True)
    try:
        proc.verify_targets()
        try:
            engine_ptr = find_engine(proc)
        except engine_finder.EngineSearchTimeout as e:
            return RunResult(stop=STOP_NO_ENGINE, message=str(e))
        engine = EngineModel(proc)
        engine.use_object(engine_ptr)
        chords, threshold = with_axes(chords, engine)
        clock_addr, score_addr = engine_addresses(engine)
        result = play_in_game(chords, player, clock_addr=clock_addr, score_addr=score_addr,
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
    """Play a schedule JSON against the running game. Returns the exit code:
    0 and "Done." only when the run sent every press (STOP_DONE), 2 when no
    playing song was found, and 1 for every other stop and for a schedule
    load_schedule refuses, each printed with its reason.

    LIVE-ONLY: the run itself is live_play's. `open_proc` and `find_engine`
    stand in for open_process and the capped finder in tests."""
    out = sys.stdout if out is None else out
    parser = argparse.ArgumentParser(
        description="Play a hydra_replay schedule JSON in Clone Hero.")
    parser.add_argument("schedule_json", help="a hydra_replay schedule JSON")
    parser.add_argument("--log", help="write one CSV row per press here")
    args = parser.parse_args(argv)

    try:
        chords = load_schedule(args.schedule_json).chords
    except ValueError as e:
        print(f"  Refused: {e}", file=out)
        return 1
    print(f"Read {os.path.basename(args.schedule_json)}: {len(chords)} chords", file=out)
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
    print(f"\n  Sent: {result.sent}, Declined: {result.declined}, "
          f"Dropped: {result.dropped}, Skipped at the start: {result.skipped}", file=out)
    if result.stop != STOP_DONE:
        print(f"Stopped before the song's end ({result.stop}): {result.message}", file=out)
        return 1
    print("Done.", file=out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
