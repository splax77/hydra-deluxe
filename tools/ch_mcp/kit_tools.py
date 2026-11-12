"""The Clone Hero MCP server's kit tools: a virtual drum kit inside the game.

The kit agent (agent/kit.ts, built to kit.js) builds one Rewired custom
controller from the game's own drum definition (KIT_DEFINITION_ID), hands it
to a player with the game's drum map (KIT_MAP_CATEGORY_ID, KIT_MAP_LAYOUT_ID),
presses its pads, kick, Start and Select on request and takes it away again. It sends no key presses and never touches the window, so the game can
stay minimized and unfocused; while the kit is set up, the Rewired devices in
KIT_DEVICES are switched off so the user's own keys and mouse reach no player
(the plan
docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md), "One agent for the
kit" and "The tools"). Placeholder controllers made before it put the kit on
KIT_REQUIRED_ID, the one id the game grades hit strength for.

The agent holds none of the kit's numbers. This module owns them: the
definition, the tag, the player, the elements, the bindings the game's map must
give, the hold, the caps and the axis value (the plan's owner table). The
agent reads the map back and this module refuses to go on when it differs.

`kit_press` is the auto-player's press on the kit route. play_chart.play_chords
still owns when a chord is pressed; the kit press only delivers it, and its
reply rides into the press row.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server hands in the one RLock it shares with the other
tools; every tool holds it. The kit agent answers one request at a time, so
every request also holds this module's own request lock, which the play
thread takes too (the play thread never takes the server's lock).
"""

from __future__ import annotations

import dataclasses
import math
import os
import threading
import time
from typing import Any, Callable, Dict, List, Optional, Sequence, Tuple, Union

from tools.ch_mcp import build_stamp
from tools.ch_mcp import frida_attach as FA
from tools.ch_mcp import game_files
from tools.ch_mcp.frida_tools import AGENT_DIR, ATTACH_CAP_S, NOT_ATTACHED, READ_CAP_S
from tools.ch_mcp.game_tools import PressResult
from tools.ch_probe.constants import ms_to_s, s_to_ms
from tools.ch_probe.experiments.overlay_logs import KIT_PRESS_COLUMNS
from tools.ch_probe.input_driver import Lane

KIT_AGENT_NAME = "kit"
KIT_AGENT_FILE = f"{KIT_AGENT_NAME}.js"

# The Rewired controller definition the kit is built from: the one the game's
# MIDIInputManager builds a real MIDI drum kit from, so the kit's kick is an
# axis like a real kit's. The game ships it in Clone Hero_Data\level1 with ten
# elements, all axes and no buttons, and a map at KIT_MAP_CATEGORY_ID and
# KIT_MAP_LAYOUT_ID that binds them to the actions in KIT_BINDINGS (rows P7
# and P9 of docs/superpowers/plans/2026-11-08-kick-axis-bind.md).
KIT_DEFINITION_ID = 1
KIT_DEFINITION_NAME = "MIDI Drums"
# The map setup loads onto the kit's player, by its map category ("Gameplay")
# and layout ("Drums") in the game's Rewired setup. The fact's home is block
# IN-76 of docs/clone-hero/07-internals.md.
KIT_MAP_CATEGORY_ID = 1
KIT_MAP_LAYOUT_ID = 2
# The tag the kit is created with and found by (the plan's owner table).
KIT_TAG = "hydra-kit"
# The id the kit must hold: the literal the game's velocity gate passes to
# ContainsController (docs/clone-hero/code/6142/lane_velocity_337E10.asm line
# 53). Only a custom controller with this id has its hit strength read.
KIT_REQUIRED_ID = 6
# How many placeholder controllers setup creates before the kit, from the same
# literal: ids count up from zero in a launch (block IN-81 of
# docs/clone-hero/07-internals.md), so this many use up every id below it.
KIT_PLACEHOLDERS = KIT_REQUIRED_ID
# Why a second setup in one launch cannot reach KIT_REQUIRED_ID, and what does.
# Destroying a controller does not move the game's id counter (IN-81).
NEW_LAUNCH_NOTE = ("only a new launch of the game puts the kit back on id "
                   f"{KIT_REQUIRED_ID}: a teardown does not reset the game's id "
                   "counter, so a setup after one gets a higher id "
                   "(docs/clone-hero/07-internals.md, IN-81); ch_stop, then ch_start")
KIT_ALREADY_SET_UP = ("the kit is already set up. ch_kit_teardown takes it away, but "
                      "a setup after that in the same launch is refused, because the "
                      f"kit would not be on id {KIT_REQUIRED_ID}; "
                      "start the game again (ch_stop, then ch_start) to set it up again")
# The tag of a placeholder is this text plus its index. It must not start with
# KIT_TAG, so a placeholder can never be taken for the kit.
KIT_PLACEHOLDER_TAG = "hydra-placeholder-"
# The Rewired player the kit is handed to when ch_kit_setup names none. The
# user kept Player1 (docs/agents/rulings/clone-hero.md, "Keep Player1"); a
# human launch passes player="System" to ch_kit_setup.
KIT_PLAYER = "Player1"

# How long a press holds, counted in the game's frames (the user's answer to
# the plan's question 2, 2026-10-30).
KIT_HOLD_FRAMES = 2
# How long setup and teardown get to answer (question 3): a handful of calls
# on the game's main thread.
KIT_SETUP_CAP_S = 10
# How long one press gets to answer (question 3): a press replies once its
# hold has passed, so a missing reply means the agent is gone or the game froze.
KIT_PRESS_CAP_S = 1
# Whether a press writes its values again on every tick of its hold. One
# write is enough: at live step LS-K1, presses of green held 1, 2 and 3
# frames with rewrite off each read "Green on" from the frame after the down
# write until the up frame (hold 2: "frame 3220: Green off", "frame 3221:
# Green on", up at frame 3222).
KIT_REWRITE = False
# Whether a press runs on the game's main thread. Frida's own thread is
# enough: LS-K1's three presses above ran with "main thread off".
KIT_PRESS_ON_MAIN = False
# The longest watch ch_kit_watch accepts, in seconds (the user's answer to
# the plan's K7 question, 2026-10-30, "Cap at 60 s (Recommended)"). A longer
# one is refused, naming this cap.
KIT_WATCH_MAX_S = 60.0
# ch_kit_status counts frames over this long to give the frame rate (the
# plan's "Where the schedule runs").
KIT_RATE_WINDOW_S = 1.0
# How long play_chart.play_in_game waits on the agent's event queue before it
# looks at should_stop and the agent's session again. It times no press: the
# agent fires every press on the game's frames, and only a stop asked for
# while no event comes waits on it.
KIT_EVENT_WAIT_S = 0.05

# The Rewired devices kit.ts switches off for the run, by the name its setup
# and status replies key each one's read-back with (the user's rulings of
# 2026-10-30: the keyboard, "Switch it off (Recommended)", and the mouse,
# "Switch it off too (Recommended)").
KIT_DEVICES = ("keyboard", "mouse")

# The prefix a menu step names a kit item with (menu_steps.json's kit: items).
KIT_PREFIX = "kit:"


@dataclasses.dataclass(frozen=True)
class KitElement:
    """One element of the kit: its id and name on the definition, and the
    axis value it is always pressed at. Every element is an axis. A
    `strength` of None means the element takes the press's value (the pads
    and the kick); a number means it is always written at that value."""
    id: int
    name: str
    strength: Optional[float]


# The one table of what can be pressed on the kit, by the name a caller uses.
# The pad and kick names are the auto-player's lanes (input_driver.Lane,
# lower-cased), so a chord's lanes are found here by name. Element ids and
# names are KIT_DEFINITION_ID's (row P7 of
# docs/superpowers/plans/2026-11-08-kick-axis-bind.md). Start and Select are
# written at full, as the game's MIDI pump writes a real kit's StartPad and
# SelectPad (row P6 there).
KIT_ELEMENTS: Dict[str, KitElement] = {
    "red": KitElement(0, "Snare", None),
    "yellow": KitElement(1, "Tom1", None),
    "blue": KitElement(2, "Tom2", None),
    "green": KitElement(3, "Tom3", None),
    "yellow_cymbal": KitElement(4, "Cymbal1", None),
    "blue_cymbal": KitElement(5, "Cymbal2", None),
    "green_cymbal": KitElement(6, "Cymbal3", None),
    "start": KitElement(18, "StartPad", 1.0),
    "select": KitElement(19, "SelectPad", 1.0),
    "kick": KitElement(7, "Kick", None),
}

# The bindings the game's drum map must give the kit: element id to the
# game action's id and name. The action ids are the game's action table; the
# agent sends each binding's action id, and the check compares those. The
# names are for the reply only.
KIT_BINDINGS: Dict[int, Tuple[int, str]] = {
    0: (1, "Red"),
    1: (2, "Yellow"),
    2: (3, "Blue"),
    3: (0, "Green"),
    4: (37, "YellowCymbal"),
    5: (38, "BlueCymbal"),
    6: (39, "GreenCymbal"),
    7: (4, "Orange"),
    18: (16, "Start"),
    19: (6, "Select"),
}

# The numbers of Rewired's ControllerElementType, which the setup reply gives
# each binding as its element_type (dump.cs of build 6142, lines 290899 to
# 290906).
ELEMENT_TYPE_AXIS = 0
ELEMENT_TYPE_BUTTON = 1
# The elements whose bindings' element type the setup check reads. Every
# element is checked, and each must be an axis, because KIT_DEFINITION_ID
# holds no buttons.
KIT_TYPE_CHECKED = tuple(KIT_ELEMENTS)

NOT_SET_UP = "the kit is not set up; call ch_kit_setup first"
PLAY_RUNNING = "a play session is running; call ch_play_stop first"


class KitRefused(Exception):
    """A kit request that was refused before anything was sent; the text
    says why."""


def lane_element(lane: int) -> KitElement:
    """The kit element that presses one of the auto-player's lanes."""
    return KIT_ELEMENTS[Lane(lane).name.lower()]


def item_names() -> str:
    return ", ".join(KIT_ELEMENTS)


def parse_items(items: Union[str, Sequence[str]]) -> List[str]:
    """A comma list or sequence of kit names, each with or without the
    KIT_PREFIX, as KIT_ELEMENTS keys in order, each once. Raises KitRefused
    naming an unknown name, or when there is none."""
    raw = items.split(",") if isinstance(items, str) else list(items)
    names: List[str] = []
    for item in raw:
        name = item.strip().lower()
        if name.startswith(KIT_PREFIX):
            name = name[len(KIT_PREFIX):].strip()
        if not name:
            continue
        if name not in KIT_ELEMENTS:
            raise KitRefused(f'unknown kit item "{item.strip()}"; kit items: {item_names()}')
        if name not in names:
            names.append(name)
    if not names:
        raise KitRefused(f"no kit items given; kit items: {item_names()}")
    return names


def placeholder_tags() -> List[str]:
    """The tag of each placeholder controller, in the order setup makes
    them."""
    return [f"{KIT_PLACEHOLDER_TAG}{i}" for i in range(KIT_PLACEHOLDERS)]


def setup_message(player: Any) -> dict:
    """The setup message, handing the kit to `player` with the map at
    KIT_MAP_CATEGORY_ID and KIT_MAP_LAYOUT_ID, after the placeholders. The
    name goes to the agent as given: the agent knows the game's players and
    refuses a name none of them has."""
    return {"type": "setup", "tag": KIT_TAG, "definition_id": KIT_DEFINITION_ID,
            "player": player, "category_id": KIT_MAP_CATEGORY_ID,
            "layout_id": KIT_MAP_LAYOUT_ID, "placeholder_tags": placeholder_tags()}


def teardown_message() -> dict:
    """The teardown message. It carries the tags so an agent that holds no
    setup record can still find the kit and the placeholders."""
    return {"type": "teardown", "tag": KIT_TAG, "placeholder_tags": placeholder_tags()}


def given_player_line(player: Any) -> str:
    """The first line of ch_kit_setup's reply when the caller named the
    player."""
    return f'kit player: "{player}" as given (KIT_PLAYER is {KIT_PLAYER})'


# The words hydra_replay writes in a gem's "dynamic"; dynamic_str in
# src/core/model.cpp owns them.
DYNAMIC_NONE = "none"
DYNAMIC_GHOST = "ghost"
DYNAMIC_ACCENT = "accent"


# The strengths copy the game's own bot and MIDI handler, from the Evidence
# line of SC-13 in docs/clone-hero/06-scoring.md: the bot's velocity for an
# accent and for a ghost, and the handler's turning of a velocity into an axis
# value. A plain gem presses at the accent's velocity. By SC-13 the game reads
# the velocity of every pressed lane and marks a hit at or above the threshold
# accent, so it marks such a plain gem's hit accent too; whether that mark
# pays anything is the question SC-13's Ruling line names (Q-51).
def pad_axis(dynamic: str, threshold: Optional[int] = None) -> float:
    """The axis value that presses a gem of this `dynamic`, on a pad or the
    kick, the one home of the strengths. Only a ghost needs `threshold`, the engine's dynamics
    threshold (constants.OFF_DYN_THRESHOLD). Raises ValueError for a word
    hydra_replay does not write, and for a ghost with a `threshold` no
    velocity can answer."""
    top_velocity = 127

    def axis(velocity: int) -> float:
        return 0.5 + velocity / (2 * top_velocity)

    if dynamic in (DYNAMIC_NONE, DYNAMIC_ACCENT):
        return axis(top_velocity)
    if dynamic != DYNAMIC_GHOST:
        raise ValueError(f'unknown gem dynamic "{dynamic}"; make the score file again '
                         f"with hydra_replay score")
    if threshold is None or not 1 <= threshold <= top_velocity:
        raise ValueError(f"no ghost press is possible: the engine's dynamics threshold "
                         f"(OFF_DYN_THRESHOLD) reads {threshold}, and it must be 1 to "
                         f"{top_velocity}; start the song, read the field again, and "
                         f"report the number if it stays outside that range")
    return axis(threshold - 1)


def press_message(elements: Sequence[KitElement], hold_frames: int = KIT_HOLD_FRAMES, *,
                  rewrite: bool = KIT_REWRITE, on_main: bool = KIT_PRESS_ON_MAIN,
                  read: Sequence[str] = (),
                  values: Optional[Sequence[float]] = None,
                  then: Sequence[Tuple[Sequence[KitElement], Optional[Sequence[float]],
                                       float]] = ()) -> dict:
    """The press message for `elements`, in order. Every element is written
    as an axis value. `values` gives each element's value, one per
    element, and an element with a `strength` ignores its own and is written
    at its strength; None presses the pads and the kick as plain gems. `read`
    names the game actions the
    agent reads back on the kit's player during the hold; the message carries
    it only when it names some.

    `then` holds a chord's later order groups, each (elements, values,
    after_ms): the agent puts each group down in the first frame at or after
    `after_ms` from the first group's down, and holds each group
    `hold_frames` frames from its own down (design point 4 of
    docs/superpowers/plans/2026-11-07-player-limits.md). The message carries
    `then` only when it holds some. Raises ValueError for an element twice
    in one group, which would send one press for two gems (D321 item 3),
    for an element in two groups, since one group's release would lift the
    other's, and for an `after_ms` that is not a number above 0."""
    def element_list(group: Sequence[KitElement],
                     group_values: Optional[Sequence[float]]) -> List[dict]:
        if group_values is None:
            group_values = [pad_axis(DYNAMIC_NONE)] * len(group)
        twice = sorted({e.name for e in group if list(group).count(e) > 1})
        if twice:
            raise ValueError(f"kit element {', '.join(twice)} is twice in one group of a "
                             f"press; each gem needs its own element")
        return [{"id": e.id, "value": value if e.strength is None else e.strength}
                for e, value in zip(group, group_values, strict=True)]

    message = {"type": "press", "elements": element_list(elements, values),
               "hold_frames": hold_frames, "cap_s": KIT_PRESS_CAP_S,
               "rewrite": rewrite, "on_main": on_main}
    if read:
        message["read"] = list(read)
    if then:
        used = {e.id for e in elements}
        later = []
        for group, group_values, after_ms in then:
            if (isinstance(after_ms, bool) or not isinstance(after_ms, (int, float))
                    or not after_ms > 0):
                raise ValueError(f"a later group's after_ms is a number above 0, not {after_ms!r}")
            twice = sorted(used & {e.id for e in group})
            if twice:
                raise ValueError(f"kit elements {', '.join(map(str, twice))} are in two groups "
                                 f"of one press")
            used |= {e.id for e in group}
            later.append({"elements": element_list(group, group_values),
                          "after_ms": float(after_ms)})
        message["then"] = later
    return message


def reply_gap(reply: Any, groups: int) -> Optional[str]:
    """What a kit press's reply lacks, as words that follow "the kit's reply
    to the press", or None when it holds what a run reads from it: the press
    log's kit columns (overlay_logs.KIT_PRESS_COLUMNS), and, for a press of
    more than one group (`groups` counts them), a `groups` list with an entry
    per group holding the same columns. kit.ts's press handler writes that
    shape; a reply without it stops the run (D318 item 2)."""
    if not isinstance(reply, dict):
        return f"is not an object but {reply!r}"
    if groups > 1:
        parts = reply.get("groups")
        if not isinstance(parts, list):
            return f"has no `groups` list, though the press sent {groups} groups"
        if len(parts) != groups:
            return f"has {len(parts)} `groups` entries for the {groups} groups the press sent"
        for k, part in enumerate(parts):
            missing = ([key for key in KIT_PRESS_COLUMNS if key not in part]
                       if isinstance(part, dict) else list(KIT_PRESS_COLUMNS))
            if missing:
                return f"has no {', '.join(missing)} in `groups` entry {k}"
    missing = [key for key in KIT_PRESS_COLUMNS if key not in reply]
    if missing:
        return f"has no {', '.join(missing)}"
    return None


def parse_actions(read: Union[None, str, Sequence[str]]) -> List[str]:
    """A comma list or sequence of game action names for a press to read
    back, each once, in order; [] for None. The names go to the game as
    given, because the game's GetButton is what knows them. Raises
    KitRefused for a name that is not text."""
    if read is None:
        return []
    raw = read.split(",") if isinstance(read, str) else list(read)
    names: List[str] = []
    for item in raw:
        if not isinstance(item, str):
            raise KitRefused(f"read takes game action names, not {item!r}")
        name = item.strip()
        if name and name not in names:
            names.append(name)
    return names


def watch_message(seconds: float) -> dict:
    """The watch message: read every player for `seconds`, and give up at
    the cap the request waits for."""
    return {"type": "watch", "seconds": seconds, "cap_s": watch_cap_s(seconds)}


def watch_cap_s(seconds: float) -> float:
    """How long a watch of `seconds` gets to answer: the watch itself plus
    KIT_PRESS_CAP_S (the plan, task K7)."""
    return seconds + KIT_PRESS_CAP_S


def seen_lines(reply: dict) -> List[str]:
    """A press reply's read-back, one line per change, or nothing when the
    press asked for no read."""
    if "seen" not in reply:
        return []
    lines = [f"read back on the kit's player: {', '.join(reply.get('read') or [])}"]
    seen = reply.get("seen") or []
    lines.extend(f"  frame {s.get('frame')}: {s.get('action')} {_flag_text(s.get('value'))}"
                 for s in seen)
    if not seen:
        lines.append("  no reads came back")
    return lines


def watch_lines(reply: dict) -> List[str]:
    """The watch reply as lines: what was sampled, then either that no key
    reached a player or a failure line with each true read listed. Only a
    reply that sampled at least one tick on at least one reader and counted
    no true read says no key reached a player; a reply that sampled nothing
    is a failure too, so a watch that read nothing never reads as a pass."""
    readers = reply.get("readers") or []
    ticks = reply.get("ticks")
    lines = [f"watched {reply.get('seconds')} s: {ticks} ticks, frames "
             f"{reply.get('first_frame')} to {reply.get('last_frame')}, readers: "
             f"{', '.join(readers)}"]
    count = reply.get("true_count")
    reads = reply.get("reads") or []
    if count == 0 and not reads:
        if not readers or not isinstance(ticks, int) or ticks < 1:
            lines.append("FAILED: the watch sampled nothing, so it proves nothing")
        else:
            lines.append("no key reached a player")
        return lines
    lines.append(f"FAILED: a key reached a player: {count} true reads; "
                 f"the first {len(reads)} that turned true:")
    lines.extend(f"  at {r.get('ms')} ms, frame {r.get('frame')}: {r.get('who')} "
                 "GetAnyButton true" for r in reads)
    return lines


def map_problems(maps: Any) -> List[str]:
    """Why the setup reply's maps do not hold exactly one enabled map at
    KIT_MAP_CATEGORY_ID and KIT_MAP_LAYOUT_ID: a line naming the ids, then
    one line per map that came back. Empty when they do."""
    want = (f"expected one enabled map at category {KIT_MAP_CATEGORY_ID}, "
            f"layout {KIT_MAP_LAYOUT_ID}")
    if not isinstance(maps, list) or not maps:
        return [f"{want}; the setup reply holds no map for the kit"]
    hits = [m for m in maps if m.get("enabled") is True
            and m.get("category_id") == KIT_MAP_CATEGORY_ID
            and m.get("layout_id") == KIT_MAP_LAYOUT_ID]
    if len(hits) == 1:
        return []
    return [f"{want}; the setup reply holds {len(hits)} such maps among {len(maps)}:"] + [
        f"  map: category {m.get('category_id')}, layout {m.get('layout_id')}, "
        f"{'enabled' if m.get('enabled') else 'disabled'}" for m in maps]


def binding_problems(maps: Any) -> List[str]:
    """How the setup reply's maps differ from KIT_BINDINGS, one line per
    element; empty when they match. Only bindings in enabled maps count,
    because a disabled map's bindings never fire. A KIT_TYPE_CHECKED
    element's bindings must also all be axis bindings (_type_problem)."""
    if not isinstance(maps, list) or not maps:
        return ["the setup reply holds no map for the kit"]
    got: Dict[int, set] = {}
    types: Dict[int, set] = {}
    for m in maps:
        if not m.get("enabled"):
            continue
        for b in m.get("bindings") or []:
            got.setdefault(b.get("element_id"), set()).add(b.get("action_id"))
            types.setdefault(b.get("element_id"), set()).add(b.get("element_type"))
    problems: List[str] = []
    names = {e.id: e.name for e in KIT_ELEMENTS.values()}
    checked = {KIT_ELEMENTS[n].id for n in KIT_TYPE_CHECKED}
    for element_id, (action_id, action_name) in KIT_BINDINGS.items():
        actions = got.pop(element_id, set())
        label = f"{names.get(element_id, '?')} (element {element_id})"
        if actions != {action_id}:
            found = ", ".join(str(a) for a in sorted(actions, key=str)) or "none"
            problems.append(f"{label}: expected action {action_id} ({action_name}), "
                            f"the map gives {found}")
        elif element_id in checked:
            problem = _type_problem(label, types.get(element_id, set()))
            if problem:
                problems.append(problem)
    for element_id, actions in sorted(got.items(), key=lambda kv: str(kv[0])):
        problems.append(f"element {element_id}: not in KIT_BINDINGS, the map gives "
                        + ", ".join(str(a) for a in sorted(actions, key=str)))
    return problems


def _type_text(element_type: Any) -> str:
    kind = {ELEMENT_TYPE_AXIS: "axis", ELEMENT_TYPE_BUTTON: "button"}.get(element_type)
    return f"{element_type} ({kind})" if kind else str(element_type)


def _type_problem(label: str, found: set) -> Optional[str]:
    """Why the element types of one element's bindings are not all
    ELEMENT_TYPE_AXIS, or None when they are."""
    if found == {ELEMENT_TYPE_AXIS}:
        return None
    if ELEMENT_TYPE_BUTTON in found:
        given = ", ".join(_type_text(t) for t in sorted(found, key=str))
        return (f"{label}: the map binds it as a button (element type {given}), but the "
                f"kit is built from definition {KIT_DEFINITION_ID} ({KIT_DEFINITION_NAME}), "
                f"whose elements are all axes, so a button binding is another "
                f"definition's map")
    given = ", ".join(_type_text(t) for t in sorted(found, key=str)) or "none"
    return f"{label}: expected element type {_type_text(ELEMENT_TYPE_AXIS)}, the map gives {given}"


def _flag_text(value: Any) -> str:
    return "on" if value is True else "off" if value is False else str(value)


def device_label(device: str) -> str:
    """The Rewired property a KIT_DEVICES device is read and set through, as
    the lines print it."""
    return f"{device.capitalize()}.enabled"


def devices_problem(reply: dict) -> Optional[str]:
    """Why the setup reply does not show every KIT_DEVICES device switched
    off, or None when it does. Only a read-back of false passes, so a reply
    with no read-back is refused too."""
    on = [f"Rewired's {d} ({device_label(d)}) reads {_flag_text(reply.get(f'{d}_now'))}"
          for d in KIT_DEVICES if reply.get(f"{d}_now") is not False]
    if not on:
        return None
    return (f"refused: {' and '.join(on)} after setup, not off, so the user's own "
            "input would still reach the game")


def placeholder_ids(reply: dict) -> str:
    """The setup reply's placeholder ids as text, or "none"."""
    return ", ".join(str(p.get("id")) for p in reply.get("placeholders") or []) or "none"


def kit_id_problem(reply: dict) -> Optional[str]:
    """Why the setup reply's kit does not hold KIT_REQUIRED_ID, naming the
    ids the placeholders and the kit got, or None when it does."""
    kit_id = (reply.get("kit") or {}).get("id")
    if kit_id == KIT_REQUIRED_ID:
        return None
    return (f"refused: the kit is controller {kit_id}, not KIT_REQUIRED_ID "
            f"{KIT_REQUIRED_ID}; placeholder ids: {placeholder_ids(reply)}\n"
            f"{NEW_LAUNCH_NOTE}")


def setup_lines(reply: dict) -> List[str]:
    """The setup reply as lines: the switch, each KIT_DEVICES device, each
    player's keyboard and mouse flags before and now, the placeholders, the
    kit, and each map with its bindings."""
    lines = [f"ignoreInputWhenAppNotInFocus: was {_flag_text(reply.get('switch_was'))}, "
             f"now {_flag_text(reply.get('switch_now'))}"]
    lines.extend(f"{device_label(d)}: was {_flag_text(reply.get(f'{d}_was'))}, "
                 f"now {_flag_text(reply.get(f'{d}_now'))}" for d in KIT_DEVICES)
    now = {p.get("id"): p for p in reply.get("players_now") or []}
    for p in reply.get("players_was") or []:
        n = now.get(p.get("id"), {})
        lines.append(f"player {p.get('id')} {p.get('name')}: keyboard was "
                     f"{_flag_text(p.get('has_keyboard'))}, now {_flag_text(n.get('has_keyboard'))}; "
                     f"mouse was {_flag_text(p.get('has_mouse'))}, now {_flag_text(n.get('has_mouse'))}")
    lines.append(f"placeholders: controllers {placeholder_ids(reply)} "
                 f"({reply.get('placeholders_created')} created)")
    kit = reply.get("kit") or {}
    lines.append(f"kit: controller {kit.get('id')}, definition {KIT_DEFINITION_ID} "
                 f"({KIT_DEFINITION_NAME}), tag {kit.get('tag')}, "
                 f"{'created' if kit.get('created') else 'found already made'}, "
                 f"on {kit.get('player')}")
    if not reply.get("first_setup", True):
        lines.append("an earlier setup in this agent recorded the flags above; "
                     "teardown puts those back")
    for m in reply.get("maps") or []:
        bindings = m.get("bindings") or []
        lines.append(f"map: category {m.get('category_id')}, layout {m.get('layout_id')}, "
                     f"{'enabled' if m.get('enabled') else 'disabled'}, "
                     f"{len(bindings)} bindings")
        lines.extend(f"  {b.get('element_name')} (element {b.get('element_id')}) -> "
                     f"{b.get('action_name')} (action {b.get('action_id')})" for b in bindings)
    return lines


def teardown_lines(reply: dict) -> List[str]:
    """The teardown reply's checks, one line each. A failed check reads the
    way a failed file restore does ("... failed on <what>: ...")."""
    lines: List[str] = []
    for c in reply.get("checks") or []:
        if c.get("ok"):
            lines.append(f"kit teardown: {c.get('what')}: ok ({c.get('detail')})")
        else:
            lines.append(f"kit teardown failed on {c.get('what')}: {c.get('detail')}")
    lines.append("kit teardown: all checks ok" if reply.get("all_ok")
                 else "kit teardown: NOT all checks ok; see the lines above")
    return lines


# The order fault_lines prints the x64 general registers in. Any other
# register the agent sends follows, by name.
FAULT_REGISTERS = ("rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
                   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rip")


def fault_lines(error: FA.LiveAgentError) -> List[str]:
    """The native fault a failed kit request's error reply carries, as kit.ts's
    faultOf sends it: where it faulted, the memory access, the registers and
    the JS stack. No lines when the reply holds no fault."""
    reply = getattr(error, "reply", None)
    fault = reply.get("fault") if isinstance(reply, dict) else None
    if not isinstance(fault, dict):
        return []
    where = (f"{fault.get('module')}+0x{fault.get('offset')} (0x{fault.get('address')})"
             if fault.get("module") is not None
             else f"0x{fault.get('address')} (in no module)")
    access = (f"{fault.get('operation')} of 0x{fault.get('memory_address')}"
              if fault.get("operation") is not None else "no memory access recorded")
    lines = [f"fault: {fault.get('type')} at {where}, {access}"]
    regs = fault.get("registers") if isinstance(fault.get("registers"), dict) else {}
    names = [n for n in FAULT_REGISTERS if n in regs]
    names += sorted(n for n in regs if n not in FAULT_REGISTERS)
    lines.append("registers: " + (" ".join(f"{n}=0x{regs[n]}" for n in names) or "none sent"))
    stack = fault.get("stack") if isinstance(fault.get("stack"), list) else []
    if stack:
        lines.append("stack:")
        lines.extend(f"  {str(s).strip()}" for s in stack if str(s).strip())
    return lines


def kit_press(agent: FA.LiveAgent, *, request_lock: Optional[threading.Lock] = None,
              hold_frames: int = KIT_HOLD_FRAMES) -> Callable[..., dict]:
    """The auto-player's press on the kit route: a callable that sends one
    chord's lanes at their `axes` to the kit agent and returns its reply,
    which play_chords checks with reply_gap and keeps in the press row. Axes of None press every pad as
    a plain gem. `then` holds a split chord's later groups as play_chords
    gives them, each (lanes, axes, after_ms), and goes in the same message
    (press_message), so the agent replies once, with a `groups` entry per
    group. It waits at most KIT_PRESS_CAP_S and raises
    frida_attach.ReadFailed when no reply comes, which ends the run. A reply
    that carries a native fault puts fault_lines under the error's message,
    since the run reports only that message."""
    lock = request_lock if request_lock is not None else threading.Lock()

    def press(lanes: Sequence[int], axes: Optional[Sequence[float]] = None,
              then: Sequence[Tuple[Sequence[int], Optional[Sequence[float]], float]] = ()
              ) -> dict:
        message = press_message(
            [lane_element(lane) for lane in lanes], hold_frames, values=axes,
            then=[([lane_element(lane) for lane in group], group_axes, after_ms)
                  for group, group_axes, after_ms in then])
        with lock:
            try:
                return agent.request(message, KIT_PRESS_CAP_S)
            except FA.ReadFailed as e:
                lines = fault_lines(e)
                if not lines:
                    raise
                raise FA.ReadFailed(e.reason, "\n".join([str(e), *lines]), e.reply) from e
    return press


# --- in-game timing: the contract with the kit agent ---------------------------
#
# Design points 1, 3 and 5 of
# docs/superpowers/plans/2026-11-12-in-game-press-timing.md. This block is the
# Python home of every name the play messages and the agent's events use.
# The names match the contract block at the top of
# tools/ch_mcp/agent/press_schedule.ts (IG1).
#
# Messages Python posts (each a request: the agent replies at once with the id):
#   play        entries, hold_frames, start_lead_ms, score_delay_ms, cap_ms,
#               stall_ms, jump_back_ms, and clock_addr and score_addr for
#               kit.ts alone
#   retime      read_after, entries   (REPLACES the whole set of entries not
#               yet fired; one goes after every end read, even one that moves
#               nothing, because its arrival clears retime_missed for it)
#   stop_play   (no fields)
#   frame_probe seconds, clock_addr, elements, read, hold_frames, cap_s
# An entry: seq, index (the chord), at_ms, last_send_ms, elements (each id and
# value, as a press message's), after_ms and after_seq (both null on a head),
# end_read_after (null when the entry is timed from no end read).
#
# The agent's events, sent with no id: either one event payload with a
# `kind`, or {"type": "play_events", "events": [event, ...]} for a frame's batch
# (kit.ts, task IG2, picks which). Event kinds (the event's `kind`):
#   skipped  seqs, count, clock_ms   (once, on the first frame)
#   sent     seq, index, clock_ms_at_press, down_frame, up_frame, down_ms,
#            up_ms, score_before, score_after (null when play stopped first),
#            frame_gap_ms (null on the first frame), agent_note
#   dropped  the same fields, the down and up ones null
#   stop     reason, frame, clock_ms, wall_ms, cause (null, or what stopped
#            the play from outside the scheduler), lift_error (null unless
#            lifting the pads failed)   (always last)
# agent_note is "" or retime_missed. A batch may also carry `error` (the
# agent's own failure outside any play step, with no stop after it): GamePlayer
# turns it into an EVENT_ERROR event.
PLAY_TYPE = "play"
RETIME_TYPE = "retime"
STOP_PLAY_TYPE = "stop_play"
FRAME_PROBE_TYPE = "frame_probe"
PLAY_EVENTS_TYPE = "play_events"
EVENT_SKIPPED = "skipped"
EVENT_SENT = "sent"
EVENT_DROPPED = "dropped"
EVENT_STOP = "stop"
# Made here from a batch's `error`; the agent sends no event of this kind.
EVENT_ERROR = "agent_error"
NOTE_RETIME_MISSED = "retime_missed"
STOP_REASON_DONE = "done"
STOP_REASON_PAST_END = "past_end"
STOP_REASON_STOPPED = "stopped"
STOP_REASON_CAP = "cap"
STOP_REASON_FROZEN = "frozen"
STOP_REASON_JUMPED_BACK = "jumped_back"
STOP_REASONS = (STOP_REASON_DONE, STOP_REASON_PAST_END, STOP_REASON_STOPPED,
                STOP_REASON_CAP, STOP_REASON_FROZEN, STOP_REASON_JUMPED_BACK)
# The fields every sent or dropped event must hold.
PRESS_EVENT_FIELDS = ("seq", "index", "clock_ms_at_press", "down_frame", "up_frame",
                      "down_ms", "up_ms", "score_before", "score_after",
                      "frame_gap_ms", "agent_note")
# The pad the frame probe presses once (design point 2's probe, task IG2).
FRAME_PROBE_ITEM = "green"
# The longest frame probe ch_kit_frame_probe accepts: the 5 s the user
# accepted for LS-G1 (D339 item 3).
FRAME_PROBE_SECONDS = 5.0
# LS-G1's frame-rate floor, in frames a second: the probe passes only at or
# above it (D339 item 3).
FRAME_PROBE_MIN_FPS = 322.2
# The file the probe's whole reply is saved to in the session folder, with
# a number that makes it new.
FRAME_PROBE_FILE_PREFIX = "frame_probe-"


def play_entry(seq: int, press: Any, after_seq: Optional[int]) -> dict:
    """One `play` or `retime` entry for a play_chart.Press: its `seq`, its
    chord, its fire time (Press.at_ms, with no offset: D339 item 5), its
    last send time, its kit elements at its `axes`, and for a later order
    group its `after_ms` and the seq of the press it follows."""
    elements = press_message([lane_element(lane) for lane in press.lanes],
                             values=press.axes)["elements"]
    return {"seq": seq, "index": press.index, "at_ms": press.at_ms,
            "last_send_ms": press.last_send_ms, "elements": elements,
            "after_ms": press.after_ms if after_seq is not None else None,
            "after_seq": after_seq, "end_read_after": press.end_read_after}


def play_message(entries: Sequence[dict], *, clock_addr: int, score_addr: int,
                 score_delay_ms: float, start_lead_ms: float, cap_ms: float,
                 stall_ms: float, jump_back_ms: float,
                 hold_frames: int = KIT_HOLD_FRAMES) -> dict:
    """The `play` message: the whole schedule, and every number the agent
    plays it by. The caller passes its own numbers (the score delay, the
    start lead, the run cap and the stop checks' stall and jump back);
    this module adds the hold."""
    return {"type": PLAY_TYPE, "entries": list(entries), "hold_frames": hold_frames,
            "clock_addr": clock_addr, "score_addr": score_addr,
            "score_delay_ms": score_delay_ms, "start_lead_ms": start_lead_ms,
            "cap_ms": cap_ms, "stall_ms": stall_ms, "jump_back_ms": jump_back_ms}


def retime_message(read_after: int, entries: Sequence[dict]) -> dict:
    """The `retime` message after the end read after chord `read_after`:
    every entry not yet reported, as retime_from_end left them, which the
    agent puts in place of every entry it has not fired."""
    return {"type": RETIME_TYPE, "read_after": read_after, "entries": list(entries)}


def stop_play_message() -> dict:
    return {"type": STOP_PLAY_TYPE}


def frame_probe_action() -> str:
    """The game action FRAME_PROBE_ITEM's pad is bound to (KIT_BINDINGS):
    the one the probe reads back each frame."""
    return KIT_BINDINGS[KIT_ELEMENTS[FRAME_PROBE_ITEM].id][1]


def frame_probe_message(seconds: float, clock_addr: int) -> dict:
    """The `frame_probe` message for LS-G1: log every frame for `seconds`,
    pressing FRAME_PROBE_ITEM once for the kit's hold. `read` names that
    pad's own game action (KIT_BINDINGS), so each frame's row also holds
    that action's read-back beside the any-button one."""
    element = KIT_ELEMENTS[FRAME_PROBE_ITEM]
    return {"type": FRAME_PROBE_TYPE, "seconds": seconds, "clock_addr": clock_addr,
            "elements": press_message([element])["elements"],
            "read": [frame_probe_action()],
            "hold_frames": KIT_HOLD_FRAMES, "cap_s": watch_cap_s(seconds)}


class GamePlayer:
    """The kit agent as play_chart.play_in_game uses it: post the play
    messages, and take the agent's events from a queue that Frida's thread
    fills, so a slow reader loses none. `events` holds each event in the
    order the agent sent it."""

    def __init__(self, agent: FA.LiveAgent,
                 request_lock: Optional[threading.Lock] = None) -> None:
        import queue
        self._agent = agent
        self._lock = request_lock if request_lock is not None else threading.Lock()
        self.events: "queue.Queue[dict]" = queue.Queue()
        agent.listen(self._take)

    def _take(self, payload: dict) -> None:
        if payload.get("type") == PLAY_EVENTS_TYPE:
            for event in payload.get("events") or []:
                self.events.put(event)
            if payload.get("error"):
                self.events.put({"kind": EVENT_ERROR, "error": payload["error"]})
        elif "kind" in payload:
            self.events.put(payload)

    def alive(self) -> bool:
        return self._agent.alive

    def post(self, message: dict) -> dict:
        """Post one play message and return the agent's reply, waiting at
        most KIT_PRESS_CAP_S. Raises frida_attach.ReadFailed as kit_press
        does."""
        with self._lock:
            return self._agent.request(message, KIT_PRESS_CAP_S)

    def close(self) -> None:
        self._agent.listen(None)


def find_clock_addr() -> int:
    """The playing song's engine clock address, through the run's engine
    search (game_tools.find_run_engine) on the game opened read-only.
    LIVE-ONLY."""
    from tools.ch_mcp.game_tools import find_run_engine
    from tools.ch_probe.engine import EngineModel
    from tools.ch_probe.experiments.play_chart import engine_addresses
    from tools.ch_probe.process import open_process
    proc = open_process(read_only=True)
    try:
        engine = EngineModel(proc)
        engine.use_object(find_run_engine(proc))
        return engine_addresses(engine)[0]
    finally:
        proc.close()


def _is_number(value: Any) -> bool:
    return isinstance(value, (int, float)) and not isinstance(value, bool)


def _first_pressed(rows: Sequence[dict], down_frame: int,
                   pressed: Callable[[dict], bool]) -> Optional[int]:
    """The frame of the first row at or after `down_frame` that `pressed`
    says read pressed, or None."""
    for row in rows:
        frame = row.get("frame")
        if _is_number(frame) and frame >= down_frame and pressed(row):
            return frame
    return None


def save_frame_probe(reply: dict, folder: str) -> str:
    """Save the probe's whole reply, its rows with it, as JSON to a new
    FRAME_PROBE_FILE_PREFIX file in `folder`, and return the line that says
    where it went (or why it was not saved)."""
    import json
    n = 1
    while os.path.exists(os.path.join(folder, f"{FRAME_PROBE_FILE_PREFIX}{n}.json")):
        n += 1
    path = os.path.join(folder, f"{FRAME_PROBE_FILE_PREFIX}{n}.json")
    try:
        with open(path, "x", encoding="utf-8") as f:
            json.dump(reply, f)
    except (OSError, TypeError, ValueError) as e:
        return f"rows not saved: {type(e).__name__}: {e}"
    return f"rows saved to {path}"


def frame_probe_lines(reply: dict, clock_addr: int, saved_line: str) -> List[str]:
    """The frame probe's reply judged against LS-G1's pass line
    (docs/superpowers/plans/2026-11-12-in-game-press-timing.md): the
    frame steps, the frame rate beside FRAME_PROBE_MIN_FPS, the frame the
    press read back on, the gap between the engine clock and songTime,
    hook_out and failure, then one PASS or FAIL line naming each part that
    failed. kit.ts's frame_probe owns what the reply holds; `saved_line`
    is save_frame_probe's line."""
    import statistics
    lines = [f"frame probe on the engine clock at {clock_addr:#x}"]
    failed: List[str] = []
    rows = [row for row in reply.get("frames") or [] if isinstance(row, dict)]
    frames = [row.get("frame") for row in rows]
    if rows and all(_is_number(f) for f in frames):
        lines.append(f"rows: {len(rows)}, frames {frames[0]} to {frames[-1]}; {saved_line}")
    else:
        lines.append(f"rows: {len(rows)}; {saved_line}")

    # The hook ran on every frame: each row's frame is one above the last.
    breaks = [(a, b) for a, b in zip(frames, frames[1:])
              if not (_is_number(a) and _is_number(b) and b - a == 1)]
    steps = max(len(frames) - 1, 0)
    if len(rows) < 2:
        lines.append(f"frame steps: none to check in {len(rows)} rows")
        failed.append("too few rows to check the frame steps")
    elif breaks:
        if len(breaks) == 1:
            where = f"frame {breaks[0][0]} to {breaks[0][1]}"
        else:
            where = (f"first frame {breaks[0][0]} to {breaks[0][1]}, "
                     f"last frame {breaks[-1][0]} to {breaks[-1][1]}")
        lines.append(f"frame steps: {len(breaks)} of {steps} did not rise by exactly 1 "
                     f"({where})")
        failed.append(f"frame steps ({len(breaks)} of {steps} did not rise by exactly 1)")
    else:
        lines.append(f"frame steps: all {steps} rose by exactly 1")

    # The frame rate: the frames the game advanced over the rows' wall span.
    walls = [row.get("wall_ms") for row in rows]
    rate = None
    if (len(rows) >= 2 and _is_number(frames[0]) and _is_number(frames[-1])
            and _is_number(walls[0]) and _is_number(walls[-1]) and walls[-1] > walls[0]):
        advanced = frames[-1] - frames[0]
        span_ms = walls[-1] - walls[0]
        rate = advanced / ms_to_s(span_ms)
        lines.append(f"frame rate: {rate:.1f} frames a second ({advanced} frames in "
                     f"{span_ms} ms of wall time); floor {FRAME_PROBE_MIN_FPS} "
                     f"(FRAME_PROBE_MIN_FPS, D339 item 3)")
        if rate < FRAME_PROBE_MIN_FPS:
            failed.append(f"frame rate ({rate:.1f} is under {FRAME_PROBE_MIN_FPS})")
    else:
        lines.append(f"frame rate: not measurable; floor {FRAME_PROBE_MIN_FPS} "
                     f"(FRAME_PROBE_MIN_FPS, D339 item 3)")
        failed.append("frame rate (not measurable)")

    # The one press: the frame it went down on and the first frame each read
    # saw it pressed.
    down = reply.get("down_frame")
    action = frame_probe_action()
    names = reply.get("read") or []
    if not _is_number(down):
        lines.append(f"press: never written (down_frame {down}, up_frame "
                     f"{reply.get('up_frame')})")
        failed.append("read-back (the press was never written)")
    else:
        lines.append(f"press: down on frame {down}, up on frame {reply.get('up_frame')}")
        if action in names:
            at = names.index(action)

            def own(row: dict) -> bool:
                read = row.get("read")
                return isinstance(read, list) and len(read) > at and read[at] is True

            own_frame = _first_pressed(rows, down, own)
        else:
            own_frame = None
        any_frame = _first_pressed(rows, down, lambda row: row.get("any_button") is True)
        for label, frame in ((f"{action} read-back", own_frame), ("any_button", any_frame)):
            if frame is None:
                lines.append(f"{label}: never read pressed")
            else:
                lines.append(f"{label}: first read pressed on frame {frame} "
                             f"(frames after the down frame: {frame - down})")
        if own_frame is None:
            failed.append(f"read-back ({action} never read pressed)")

    # The engine clock against songTime, on each frame that holds both.
    gaps = [s_to_ms(row["engine_clock_s"] - row["song_time_s"]) for row in rows
            if _is_number(row.get("engine_clock_s")) and _is_number(row.get("song_time_s"))]
    if gaps:
        lines.append(f"engine clock minus songTime over {len(gaps)} frames: smallest "
                     f"{min(gaps):.3f} ms, median {statistics.median(gaps):.3f} ms, "
                     f"largest {max(gaps):.3f} ms")
    else:
        lines.append("engine clock minus songTime: no frame holds both")

    hook_out = reply.get("hook_out")
    lines.append(f"hook_out: {hook_out}")
    if hook_out is not True:
        failed.append("hook (still in after the probe)")
    failure = reply.get("failure")
    lines.append(f"failure: {'none' if failure is None else failure}")
    if failure is not None:
        failed.append(f"fault ({failure})")
    lines.append(f"managers: {reply.get('managers')}; probe wall time: "
                 f"{reply.get('wall_ms')} ms")

    if failed:
        lines.append("FAIL against LS-G1's pass line: " + "; ".join(failed))
    else:
        lines.append("PASS against LS-G1's pass line: every frame stepped by 1, no fault, "
                     f"the rate is at or above {FRAME_PROBE_MIN_FPS}, the press read back "
                     "as pressed, and the hook is out")
    return lines


def read_pause_setting(documents_dir: str = game_files.DOCUMENTS_DIR) -> str:
    """The value of settings.ini's pause_on_focus_lost line, through
    game_files' own lookup of a [game] line. Raises
    game_files.SettingNotFound or OSError as that lookup does."""
    return game_files.get_game_setting(os.path.join(documents_dir, game_files.SETTINGS),
                                       game_files.PAUSE_ON_FOCUS_LOST)


class KitTools:
    """The five kit tools, press_items for the menu driver, kit_press for the
    auto-player, and the kit agent between them.

    `game` is the server's game_tools.GameTools: this class takes its backup,
    session folder, play snapshot and foreground line, registers a stop hook
    that tears the kit down, and hands it the kit press. `frida` is the
    server's frida_tools.FridaTools; ch_kit_setup needs its attach. Every
    outside seam is an argument so the tests swap each for a fake:
    `agent_dir`, `frida_mod` (None imports frida at the first attach), `wait`
    (how a request waits for its reply), `now` and `sleep` (the clock the
    frame rate is timed on), `read_settings` (the live settings.ini's
    pause_on_focus_lost value) and `find_clock` (the playing song's clock
    address for the frame probe; find_clock_addr)."""

    def __init__(self, game, frida, lock, *,
                 agent_dir: str = AGENT_DIR,
                 frida_mod: Any = None,
                 wait: Optional[Callable] = None,
                 now: Callable[[], float] = time.monotonic,
                 sleep: Callable[[float], None] = time.sleep,
                 read_settings: Callable[[], str] = read_pause_setting,
                 find_clock: Optional[Callable[[], int]] = None):
        self._game = game
        self._find_clock = find_clock if find_clock is not None else find_clock_addr
        self._frida = frida
        self._lock = lock
        self._agent_dir = agent_dir
        self._frida_mod = frida_mod
        self._wait = wait
        self._now = now
        self._sleep = sleep
        self._read_settings = read_settings
        # One request at a time to the kit agent, from a tool or the play
        # thread.
        self._request_lock = threading.Lock()
        self._agent: Optional[FA.LiveAgent] = None
        # True once setup answered with the expected bindings.
        self._ready = False
        self._count = 0
        # Lines the live agent says outside a reply (a refused detach).
        self._said: List[str] = []
        game.add_stop_hook(self._stop_hook)
        game.use_kit_press(self.kit_press)

    # -- the tools ------------------------------------------------------------------

    def ch_kit_setup(self, player: Optional[str] = None) -> str:
        """Check the attach and settings.ini, take the backup, attach the kit
        agent, send setup, check with devices_problem that the Rewired
        devices read off, check with kit_id_problem that the kit holds
        KIT_REQUIRED_ID, check with map_problems that the map it read back is
        the one setup_message names, and check that map's bindings against
        KIT_BINDINGS. A setup that fails, leaves a device on, gets another
        id, reads another map or reads other bindings is torn down again, so
        nothing is left half done.

        `player` names the Rewired player the kit is handed to; None hands
        it to KIT_PLAYER. A given name is sent as is, and the reply's first
        line is given_player_line."""
        text = self._setup(KIT_PLAYER if player is None else player)
        if player is None:
            return text
        return f"{given_player_line(player)}\n{text}"

    def _setup(self, player: Any) -> str:
        """ch_kit_setup's work, handing the kit to `player`."""
        with self._lock:
            if not self._frida.is_attached():
                return NOT_ATTACHED
            if self._ready and self._agent is not None and self._agent.alive:
                return KIT_ALREADY_SET_UP
            key = game_files.PAUSE_ON_FOCUS_LOST
            want = game_files.PAUSE_ON_FOCUS_LOST_RUN_VALUE
            try:
                value = self._read_settings()
            except (game_files.SettingNotFound, OSError) as e:
                return (f"refused: could not read {key} in {game_files.SETTINGS} ({e}); "
                        f"start the game with ch_start, which sets it to {want}")
            if value.strip() != want:
                return (f"refused: {game_files.SETTINGS} reads {key} = {value.strip()}, "
                        f"not {want}; start the game with ch_start, which sets it for the run")
            lines = [line for line in (self._game.ensure_backup(),) if line]
            path = os.path.join(self._agent_dir, KIT_AGENT_FILE)
            refusal = build_stamp.check(path)
            if refusal is not None:
                lines.append(refusal)
                return self._reply(lines)
            if self._agent is not None:
                # A session the game dropped: detach closes its log file.
                self._agent.detach()
                self._agent = None
            self._count += 1
            log = os.path.join(self._game.session_dir(), f"kit-{self._count}.jsonl")
            agent = FA.LiveAgent(path, log_path=log, frida_mod=self._frida_mod,
                                 wait=self._wait, out=self._said.append)
            try:
                agent.start(ATTACH_CAP_S)
            except FA.AttachFailed as e:
                lines.append(f"kit agent: {e}{self._cap_note(e, 'ATTACH_CAP_S')}")
                return self._reply(lines)
            except SystemExit as e:  # frida_attach's advice for a failed import
                lines.append(str(e))
                return self._reply(lines)
            self._agent = agent
            lines.append(f"kit agent attached; log: {log}")
            try:
                with self._request_lock:
                    reply = agent.request(setup_message(player), KIT_SETUP_CAP_S)
            except FA.ReadFailed as e:
                lines.append(f"setup failed: {e}{self._cap_note(e, 'KIT_SETUP_CAP_S')}")
                lines.extend(fault_lines(e))
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            lines.extend(setup_lines(reply))
            devices = devices_problem(reply)
            if devices is not None:
                lines.append(devices)
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            wrong_id = kit_id_problem(reply)
            if wrong_id is not None:
                lines.append(wrong_id)
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            map_wrong = map_problems(reply.get("maps"))
            if map_wrong:
                lines.append(f"refused: the kit's map is not the one at category "
                             f"{KIT_MAP_CATEGORY_ID}, layout {KIT_MAP_LAYOUT_ID}:")
                lines.extend(f"  {p}" for p in map_wrong)
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            problems = binding_problems(reply.get("maps"))
            if problems:
                lines.append("refused: the kit's bindings differ from KIT_BINDINGS:")
                lines.extend(f"  {p}" for p in problems)
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            self._ready = True
            lines.append(f"bindings: all {len(KIT_BINDINGS)} match KIT_BINDINGS")
            return self._reply(lines)

    def ch_kit_press(self, items: Union[str, Sequence[str]],
                     hold_frames: Optional[int] = None, rewrite: Optional[bool] = None,
                     on_main: Optional[bool] = None,
                     read: Union[None, str, Sequence[str]] = None,
                     value: Optional[float] = None) -> str:
        """Press one chord of kit items, such as "green,kick" or "start", and
        print the frames it went down and came up on. press_items does the
        work and says what each argument does."""
        return self.press_items(items, hold_frames, rewrite=rewrite, on_main=on_main,
                                read=read, value=value).text

    def press_items(self, items: Union[str, Sequence[str]],
                    hold_frames: Optional[int] = None, *, rewrite: Optional[bool] = None,
                    on_main: Optional[bool] = None,
                    read: Union[None, str, Sequence[str]] = None,
                    value: Optional[float] = None) -> PressResult:
        """Press one chord of kit items (KIT_ELEMENTS names, with or without
        KIT_PREFIX), held `hold_frames` frames (KIT_HOLD_FRAMES when None).
        `rewrite` and `on_main` set the press's rewrite and thread
        (KIT_REWRITE and KIT_PRESS_ON_MAIN when None). `read` names game
        actions, such as "Green", that the agent reads back on the kit's
        player during the hold; the text then lists each change it saw.
        `value` is the axis value of this press's pads and kick, from 0 to 1;
        None presses them as plain gems. Start and Select always press at
        their KitElement strength, whatever `value` says. The result says
        whether the press went out, for the menu driver."""
        with self._lock:
            try:
                names = parse_items(items)
                actions = parse_actions(read)
            except KitRefused as e:
                return PressResult(False, str(e))
            hold = KIT_HOLD_FRAMES if hold_frames is None else hold_frames
            if isinstance(hold, bool) or not isinstance(hold, int) or hold < 1:
                return PressResult(False, f"hold_frames is a whole number of 1 or more, "
                                          f"not {hold_frames!r}")
            for what, flag in (("rewrite", rewrite), ("on_main", on_main)):
                if flag is not None and not isinstance(flag, bool):
                    return PressResult(False, f"{what} is true, false or left out, "
                                              f"not {flag!r}")
            if value is not None and (isinstance(value, bool)
                                      or not isinstance(value, (int, float))
                                      or not 0 <= value <= 1):
                return PressResult(False, f"value is a number from 0 to 1 or left out, "
                                          f"not {value!r}")
            agent = self._ready_agent()
            if agent is None:
                return PressResult(False, NOT_SET_UP)
            if self._play_running():
                return PressResult(False, PLAY_RUNNING)
            elements = [KIT_ELEMENTS[n] for n in names]
            message = press_message(
                elements, hold,
                rewrite=KIT_REWRITE if rewrite is None else rewrite,
                on_main=KIT_PRESS_ON_MAIN if on_main is None else on_main,
                read=actions,
                values=None if value is None else [value] * len(elements))
            try:
                with self._request_lock:
                    reply = agent.request(message, KIT_PRESS_CAP_S)
            except FA.ReadFailed as e:
                return PressResult(False, self._reply(
                    [f"press failed: {e}{self._cap_note(e, 'KIT_PRESS_CAP_S')}",
                     *fault_lines(e)]))
            gap = reply_gap(reply, 1)
            if gap is not None:
                return PressResult(False, self._reply(
                    [f"press failed: the kit's reply to the press {gap}"]))
            down, up = reply["down_frame"], reply["up_frame"]
            frames = up - down if isinstance(down, int) and isinstance(up, int) else "?"
            down_ms, up_ms = reply["down_ms"], reply["up_ms"]
            held = (f"{up_ms - down_ms:g} ms" if isinstance(down_ms, (int, float))
                    and isinstance(up_ms, (int, float)) else "? ms")
            lines = [
                f"pressed {', '.join(names)} (elements "
                f"{', '.join(str(e.id) for e in elements)}) for {hold} frames: down at frame "
                f"{down}, up at frame {up} ({frames} frames apart, {held} by the agent's clock)"]
            if rewrite is not None or on_main is not None:
                lines.append(f"rewrite {_flag_text(reply.get('rewrite'))}, main thread "
                             f"{_flag_text(reply.get('on_main'))} (as the agent ran it)")
            if value is not None:
                lines.append(f"pads and kick pressed at value {value}")
            lines.extend(seen_lines(reply))
            return PressResult(True, self._reply(lines))

    def ch_kit_watch(self, seconds: float) -> str:
        """For `seconds`, have the kit agent read GetAnyButton on every
        Rewired player and on its controllers, and print whether any read
        came true: a true read means a key or button reached a player. It
        is refused while a play session runs, because the kit's own presses
        read true; the server's lock keeps a press from running meanwhile.
        A `seconds` above KIT_WATCH_MAX_S is refused."""
        with self._lock:
            if (isinstance(seconds, bool) or not isinstance(seconds, (int, float))
                    or not math.isfinite(seconds) or not seconds > 0):
                return f"seconds is a number above 0, not {seconds!r}"
            if seconds > KIT_WATCH_MAX_S:
                return (f"seconds is at most {KIT_WATCH_MAX_S:g} (KIT_WATCH_MAX_S), "
                        f"not {seconds!r}")
            agent = self._ready_agent()
            if agent is None:
                return NOT_SET_UP
            if self._play_running():
                return PLAY_RUNNING
            cap = watch_cap_s(seconds)
            try:
                with self._request_lock:
                    reply = agent.request(watch_message(seconds), cap)
            except FA.ReadFailed as e:
                note = " (seconds plus KIT_PRESS_CAP_S)" if e.reason == FA.END_CAP else ""
                return self._reply([f"watch failed: {e}{note}", *fault_lines(e)])
            return self._reply(watch_lines(reply))

    def ch_kit_status(self) -> str:
        """The kit agent's status: the frame count, the frame rate over
        KIT_RATE_WINDOW_S, whether the game has focus, the switch, each
        KIT_DEVICES device, the kit, each player's keyboard and mouse flags,
        then the foreground count."""
        with self._lock:
            lines: List[str] = []
            agent = self._agent
            if agent is None or not agent.alive:
                why = f" ({agent.gone_reason})" if agent is not None else ""
                lines.append(f"kit agent: not attached{why}; call ch_kit_setup")
            else:
                try:
                    first, t0 = self._status(agent), self._now()
                    self._sleep(KIT_RATE_WINDOW_S)
                    second, t1 = self._status(agent), self._now()
                except FA.ReadFailed as e:
                    lines.append(f"status failed: {e}{self._cap_note(e, 'READ_CAP_S')}")
                    lines.extend(fault_lines(e))
                else:
                    lines.extend(self._status_lines(first, second, t1 - t0))
            fg = self._game.foreground_line()
            lines.append(fg if fg else "foreground: no watch (the server did not start the game)")
            return self._reply(lines)

    def ch_kit_teardown(self) -> str:
        """Clear the kit's elements, remove the kit, destroy it and every
        placeholder, give every player back the keyboard and mouse flags
        setup recorded, put the Rewired devices and the switch back, print
        each check, and detach the kit agent."""
        with self._lock:
            if self._play_running():
                return PLAY_RUNNING
            if self._agent is None:
                return "no kit agent attached; nothing to tear down"
            return self._reply(self._teardown_and_detach())

    def kit_press(self) -> Optional[Callable[..., dict]]:
        """The auto-player's kit press, or None when the kit is not set up."""
        with self._lock:
            agent = self._ready_agent()
            if agent is None:
                return None
            return kit_press(agent, request_lock=self._request_lock)

    def game_player(self) -> Optional[GamePlayer]:
        """The kit agent for play_chart.play_in_game, or None when the kit
        is not set up."""
        with self._lock:
            agent = self._ready_agent()
            if agent is None:
                return None
            return GamePlayer(agent, self._request_lock)

    def ch_kit_frame_probe(self, seconds: float = FRAME_PROBE_SECONDS) -> str:
        """LS-G1's frame probe: find the playing song's engine, then have the
        kit agent log each frame for `seconds` (the frame number, the engine
        clock, songTime and the wall time), press FRAME_PROBE_ITEM once and
        read it back each frame, save its reply to the session folder
        (save_frame_probe) and print frame_probe_lines' report. A `seconds`
        above FRAME_PROBE_SECONDS is refused, and so is a probe while a play
        session runs."""
        with self._lock:
            if (isinstance(seconds, bool) or not isinstance(seconds, (int, float))
                    or not math.isfinite(seconds) or not seconds > 0):
                return f"seconds is a number above 0, not {seconds!r}"
            if seconds > FRAME_PROBE_SECONDS:
                return (f"seconds is at most {FRAME_PROBE_SECONDS:g} (FRAME_PROBE_SECONDS), "
                        f"not {seconds!r}")
            agent = self._ready_agent()
            if agent is None:
                return NOT_SET_UP
            if self._play_running():
                return PLAY_RUNNING
            try:
                clock_addr = self._find_clock()
            except Exception as e:  # the finder's own words go to the caller
                return f"frame probe: no engine found: {type(e).__name__}: {e}"
            message = frame_probe_message(seconds, clock_addr)
            try:
                with self._request_lock:
                    reply = agent.request(message, message["cap_s"])
            except FA.ReadFailed as e:
                note = " (seconds plus KIT_PRESS_CAP_S)" if e.reason == FA.END_CAP else ""
                return self._reply([f"frame probe failed: {e}{note}", *fault_lines(e)])
            saved_line = save_frame_probe(reply, self._game.session_dir())
            return self._reply(frame_probe_lines(reply, clock_addr, saved_line))

    def agent_attached(self) -> bool:
        """Whether a kit agent is held, alive or not: the one ch_kit_teardown
        detaches. The server's open_items asks this."""
        with self._lock:
            return self._agent is not None

    # -- inside -----------------------------------------------------------------------

    def _ready_agent(self) -> Optional[FA.LiveAgent]:
        agent = self._agent
        if not self._ready or agent is None or not agent.alive:
            return None
        return agent

    def _play_running(self) -> bool:
        play = self._game.play_snapshot()
        return play is not None and play["stop"] is None

    def _status(self, agent: FA.LiveAgent) -> dict:
        with self._request_lock:
            return agent.request({"type": "status", "tag": KIT_TAG}, READ_CAP_S)

    @staticmethod
    def _status_lines(first: dict, second: dict, seconds: float) -> List[str]:
        lines = [f"frame: {second.get('frame')}"]
        f0, f1 = first.get("frame"), second.get("frame")
        if isinstance(f0, int) and isinstance(f1, int) and seconds > 0:
            lines.append(f"frame rate: {(f1 - f0) / seconds:.1f} frames a second "
                         f"({f1 - f0} frames in {seconds:.2f} s)")
        else:
            lines.append("frame rate: not known (no two frame counts)")
        lines.append(f"game focused: {_flag_text(second.get('focused'))}")
        lines.append(f"ignoreInputWhenAppNotInFocus: {_flag_text(second.get('switch_now'))}")
        lines.extend(f"{device_label(d)}: {_flag_text(second.get(f'{d}_now'))}"
                     for d in KIT_DEVICES)
        if second.get("kit_exists"):
            lines.append(f"kit: controller {second.get('kit_id')}, tag {second.get('kit_tag')}")
        else:
            lines.append(f"kit: none tagged {second.get('kit_tag')}")
        lines.extend(f"player {p.get('id')} {p.get('name')}: keyboard "
                     f"{_flag_text(p.get('has_keyboard'))}, mouse {_flag_text(p.get('has_mouse'))}"
                     for p in second.get("players") or [])
        return lines

    def _teardown_and_detach(self) -> List[str]:
        """Send teardown, print its checks, then detach the kit agent."""
        lines: List[str] = []
        agent = self._agent
        self._ready = False
        if agent is None:
            return lines
        try:
            with self._request_lock:
                reply = agent.request(teardown_message(), KIT_SETUP_CAP_S)
        except FA.ReadFailed as e:
            lines.append(f"kit teardown failed on the request: {e}"
                         f"{self._cap_note(e, 'KIT_SETUP_CAP_S')}")
            lines.extend(fault_lines(e))
        else:
            lines.extend(teardown_lines(reply))
        lines.append("kit agent: " + agent.detach())
        self._agent = None
        return lines

    def _stop_hook(self) -> Optional[str]:
        """game_tools' stop hook: tear the kit down, then detach its agent."""
        with self._lock:
            lines = self._teardown_and_detach() if self._agent is not None else []
            lines.extend(self._said)
            self._said.clear()
            return "\n".join(lines) or None

    @staticmethod
    def _cap_note(error: FA.LiveAgentError, name: str) -> str:
        return f" ({name})" if error.reason == FA.END_CAP else ""

    def _reply(self, lines: List[str]) -> str:
        """`lines`, then anything the live agent said meanwhile."""
        lines = lines + self._said
        self._said.clear()
        return "\n".join(lines)

    def setup_for_run(self, player: Optional[str] = None) -> Tuple[Optional[Callable[..., dict]], str]:
        """ch_kit_setup for another tool (game_tools' ch_run_chart): the kit
        press and the setup's reply. The press is None when this call built no
        kit, which covers a kit already set up: its refusal leaves that kit as
        it is, and a failed setup has torn its own kit down."""
        with self._lock:
            was_ready = self._ready_agent() is not None
            text = self.ch_kit_setup(player)
            return (None if was_ready else self.kit_press()), text
