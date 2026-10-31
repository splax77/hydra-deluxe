"""The Clone Hero MCP server's kit tools: a virtual drum kit inside the game.

The kit agent (agent/kit.ts, built to kit.js) builds one Rewired custom
controller from the game's own drum definition, hands it to a player with the
game's default map, presses its pads and buttons on request and takes it away
again. It sends no key presses and never touches the window, so the game can
stay minimized and unfocused; while the kit is set up, the Rewired devices in
KIT_DEVICES are switched off so the user's own keys and mouse reach no player
(the plan
docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md, "One agent for the
kit" and "The tools").

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

from tools.ch_mcp import frida_attach as FA
from tools.ch_mcp import game_files
from tools.ch_mcp import trace_hits as TH
from tools.ch_mcp.frida_tools import AGENT_DIR, ATTACH_CAP_S, NOT_ATTACHED, READ_CAP_S
from tools.ch_mcp.game_tools import PressResult
from tools.ch_probe.input_driver import Lane

KIT_AGENT_NAME = "kit"
KIT_AGENT_FILE = f"{KIT_AGENT_NAME}.js"

# The Rewired controller definition the kit is built from, "SLInputDrums"
# (the probe's q4_out.json and the plan's probe facts).
KIT_DEFINITION_ID = 6
# The tag the kit is created with and found by (the plan's owner table).
KIT_TAG = "hydra-kit"
# The Rewired player the kit is handed to when ch_kit_setup names none (the
# probe: Players are Player1 to Player4). Live step LS-D1 of the plan
# docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md decides whether it
# becomes the System player.
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
# The value a pad press writes to its axis: the probe's value (q8_pad.js).
# LS-K4's compare says whether the game grades it as a velocity.
KIT_AXIS_VALUE = 1.0
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

# The Rewired devices kit.ts switches off for the run, by the name its setup
# and status replies key each one's read-back with (the user's rulings of
# 2026-10-30: the keyboard, "Switch it off (Recommended)", and the mouse,
# "Switch it off too (Recommended)").
KIT_DEVICES = ("keyboard", "mouse")

# The prefix a menu step names a kit item with (menu_steps.json's kit: items).
KIT_PREFIX = "kit:"


@dataclasses.dataclass(frozen=True)
class KitElement:
    """One element of the kit: its id and name on the definition, and
    whether it is a button (pressed with true) or a pad axis (pressed with
    KIT_AXIS_VALUE)."""
    id: int
    name: str
    button: bool


# The one table of what can be pressed on the kit, by the name a caller uses.
# The pad names are the auto-player's lanes (input_driver.Lane, lower-cased),
# so a chord's lanes are found here by name. Element ids and names are the
# definition's (q4_out.json, "def6_elements").
KIT_ELEMENTS: Dict[str, KitElement] = {
    "red": KitElement(0, "Snare", False),
    "yellow": KitElement(1, "Tom1", False),
    "blue": KitElement(2, "Tom2", False),
    "green": KitElement(3, "Tom3", False),
    "yellow_cymbal": KitElement(4, "Cymbal1", False),
    "blue_cymbal": KitElement(5, "Cymbal2", False),
    "green_cymbal": KitElement(6, "Cymbal3", False),
    "start": KitElement(20, "Start", True),
    "select": KitElement(21, "Select", True),
    "kick": KitElement(22, "Kick", True),
}

# The bindings the game's default map must give the kit: element id to the
# game action's id and name. The action ids are the game's action table
# (q4_out.json, "actions"); the agent sends each binding's action id, and the
# check compares those. The names are for the reply only.
KIT_BINDINGS: Dict[int, Tuple[int, str]] = {
    0: (1, "Red"),
    1: (2, "Yellow"),
    2: (3, "Blue"),
    3: (0, "Green"),
    4: (37, "YellowCymbal"),
    5: (38, "BlueCymbal"),
    6: (39, "GreenCymbal"),
    20: (16, "Start"),
    21: (6, "Select"),
    22: (4, "Orange"),
}

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


def setup_message(player: Any) -> dict:
    """The setup message, handing the kit to `player`. The name goes to the
    agent as given: the agent knows the game's players and refuses a name
    none of them has."""
    return {"type": "setup", "tag": KIT_TAG, "definition_id": KIT_DEFINITION_ID,
            "player": player}


def given_player_line(player: Any) -> str:
    """The first line of ch_kit_setup's reply when the caller named the
    player."""
    return f'kit player: "{player}" as given (KIT_PLAYER is {KIT_PLAYER})'


def press_message(elements: Sequence[KitElement], hold_frames: int = KIT_HOLD_FRAMES, *,
                  rewrite: bool = KIT_REWRITE, on_main: bool = KIT_PRESS_ON_MAIN,
                  read: Sequence[str] = ()) -> dict:
    """The press message for `elements`, each once, in order. `read` names
    the game actions the agent reads back on the kit's player during the
    hold; the message carries it only when it names some."""
    seen: List[KitElement] = []
    for e in elements:
        if e not in seen:
            seen.append(e)
    message = {"type": "press",
               "elements": [{"id": e.id, "value": True if e.button else KIT_AXIS_VALUE}
                            for e in seen],
               "hold_frames": hold_frames, "cap_s": KIT_PRESS_CAP_S,
               "rewrite": rewrite, "on_main": on_main}
    if read:
        message["read"] = list(read)
    return message


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


def binding_problems(maps: Any) -> List[str]:
    """How the setup reply's maps differ from KIT_BINDINGS, one line per
    element; empty when they match. Only bindings in enabled maps count,
    because a disabled map's bindings never fire."""
    if not isinstance(maps, list) or not maps:
        return ["the setup reply holds no map for the kit"]
    got: Dict[int, set] = {}
    for m in maps:
        if not m.get("enabled"):
            continue
        for b in m.get("bindings") or []:
            got.setdefault(b.get("element_id"), set()).add(b.get("action_id"))
    problems: List[str] = []
    names = {e.id: e.name for e in KIT_ELEMENTS.values()}
    for element_id, (action_id, action_name) in KIT_BINDINGS.items():
        actions = got.pop(element_id, set())
        label = f"{names.get(element_id, '?')} (element {element_id})"
        if actions != {action_id}:
            found = ", ".join(str(a) for a in sorted(actions, key=str)) or "none"
            problems.append(f"{label}: expected action {action_id} ({action_name}), "
                            f"the map gives {found}")
    for element_id, actions in sorted(got.items(), key=lambda kv: str(kv[0])):
        problems.append(f"element {element_id}: not in KIT_BINDINGS, the map gives "
                        + ", ".join(str(a) for a in sorted(actions, key=str)))
    return problems


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


def setup_lines(reply: dict) -> List[str]:
    """The setup reply as lines: the switch, each KIT_DEVICES device, each
    player's keyboard and mouse flags before and now, the kit, and each map
    with its bindings."""
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
    kit = reply.get("kit") or {}
    lines.append(f"kit: controller {kit.get('id')}, tag {kit.get('tag')}, "
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


def kit_press(agent: FA.LiveAgent, *, request_lock: Optional[threading.Lock] = None,
              hold_frames: int = KIT_HOLD_FRAMES) -> Callable[[Sequence[int]], dict]:
    """The auto-player's press on the kit route: a callable that sends one
    chord's lanes to the kit agent and returns its reply, which play_chords
    keeps in the press row. It waits at most KIT_PRESS_CAP_S and raises
    frida_attach.ReadFailed when no reply comes, which ends the run."""
    lock = request_lock if request_lock is not None else threading.Lock()

    def press(lanes: Sequence[int]) -> dict:
        message = press_message([lane_element(lane) for lane in lanes], hold_frames)
        with lock:
            return agent.request(message, KIT_PRESS_CAP_S)
    return press


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
    frame rate is timed on) and `read_settings` (the live settings.ini's
    pause_on_focus_lost value)."""

    def __init__(self, game, frida, lock, *,
                 agent_dir: str = AGENT_DIR,
                 frida_mod: Any = None,
                 wait: Optional[Callable] = None,
                 now: Callable[[], float] = time.monotonic,
                 sleep: Callable[[float], None] = time.sleep,
                 read_settings: Callable[[], str] = read_pause_setting):
        self._game = game
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
        devices read off, and check the bindings it read back against
        KIT_BINDINGS. A setup that fails, leaves a device on or reads other
        bindings is torn down again, so nothing is left half done.

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
                return "the kit is already set up; call ch_kit_teardown first to set it up again"
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
            if not os.path.isfile(path):
                lines.append(TH.agent_missing_line(path, KIT_AGENT_NAME))
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
                lines.extend(self._teardown_and_detach())
                return self._reply(lines)
            lines.extend(setup_lines(reply))
            devices = devices_problem(reply)
            if devices is not None:
                lines.append(devices)
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
                     read: Union[None, str, Sequence[str]] = None) -> str:
        """Press one chord of kit items, such as "green,kick" or "start", and
        print the frames it went down and came up on. press_items does the
        work and says what each argument does."""
        return self.press_items(items, hold_frames, rewrite=rewrite, on_main=on_main,
                                read=read).text

    def press_items(self, items: Union[str, Sequence[str]],
                    hold_frames: Optional[int] = None, *, rewrite: Optional[bool] = None,
                    on_main: Optional[bool] = None,
                    read: Union[None, str, Sequence[str]] = None) -> PressResult:
        """Press one chord of kit items (KIT_ELEMENTS names, with or without
        KIT_PREFIX), held `hold_frames` frames (KIT_HOLD_FRAMES when None).
        `rewrite` and `on_main` set the press's rewrite and thread
        (KIT_REWRITE and KIT_PRESS_ON_MAIN when None). `read` names game
        actions, such as "Green", that the agent reads back on the kit's
        player during the hold; the text then lists each change it saw. The
        result says whether the press went out, for the menu driver."""
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
            for what, value in (("rewrite", rewrite), ("on_main", on_main)):
                if value is not None and not isinstance(value, bool):
                    return PressResult(False, f"{what} is true, false or left out, "
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
                read=actions)
            try:
                with self._request_lock:
                    reply = agent.request(message, KIT_PRESS_CAP_S)
            except FA.ReadFailed as e:
                return PressResult(False, self._reply(
                    [f"press failed: {e}{self._cap_note(e, 'KIT_PRESS_CAP_S')}"]))
            down, up = reply.get("down_frame"), reply.get("up_frame")
            frames = up - down if isinstance(down, int) and isinstance(up, int) else "?"
            down_ms, up_ms = reply.get("down_ms"), reply.get("up_ms")
            held = (f"{up_ms - down_ms:g} ms" if isinstance(down_ms, (int, float))
                    and isinstance(up_ms, (int, float)) else "? ms")
            lines = [
                f"pressed {', '.join(names)} (elements "
                f"{', '.join(str(e.id) for e in elements)}) for {hold} frames: down at frame "
                f"{down}, up at frame {up} ({frames} frames apart, {held} by the agent's clock)"]
            if rewrite is not None or on_main is not None:
                lines.append(f"rewrite {_flag_text(reply.get('rewrite'))}, main thread "
                             f"{_flag_text(reply.get('on_main'))} (as the agent ran it)")
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
                return self._reply([f"watch failed: {e}{note}"])
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
                else:
                    lines.extend(self._status_lines(first, second, t1 - t0))
            fg = self._game.foreground_line()
            lines.append(fg if fg else "foreground: no watch (the server did not start the game)")
            return self._reply(lines)

    def ch_kit_teardown(self) -> str:
        """Clear the kit's elements, remove the kit, give every player back
        the keyboard and mouse flags setup recorded, put the Rewired devices
        and the switch back, print each check, and detach the kit agent."""
        with self._lock:
            if self._play_running():
                return PLAY_RUNNING
            if self._agent is None:
                return "no kit agent attached; nothing to tear down"
            return self._reply(self._teardown_and_detach())

    def kit_press(self) -> Optional[Callable[[Sequence[int]], dict]]:
        """The auto-player's kit press, or None when the kit is not set up."""
        with self._lock:
            agent = self._ready_agent()
            if agent is None:
                return None
            return kit_press(agent, request_lock=self._request_lock)

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
                reply = agent.request({"type": "teardown", "tag": KIT_TAG}, KIT_SETUP_CAP_S)
        except FA.ReadFailed as e:
            lines.append(f"kit teardown failed on the request: {e}"
                         f"{self._cap_note(e, 'KIT_SETUP_CAP_S')}")
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
