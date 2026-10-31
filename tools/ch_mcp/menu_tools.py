"""The Clone Hero MCP server's menu driver: ch_open_song starts a song by
pressing the menu keys menu_steps.json lists, reading the game after every
press to see where it is.

The plan is docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md,
task T6. Each question has one owner, and this module calls it:
game_tools.GameTools.press_keys sends menu keys and typed text, and
GameTools.ch_press presses a drum lane; frida_tools.FridaTools.read_state is
the read, agent/menu.ts sends the menu block in it, and frida_tools.menu_lines
and state_words print it; game_files.read_song_ini gives the name the search
types. This module owns the step file's format, the five expect words, the
Online refusal, and three of the user's ruling-3 numbers. The fourth, the gap
between presses, is input_driver's.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server hands in the one RLock it shares with the other
tools.
"""

from __future__ import annotations

import json
import os
import time
from typing import Callable, Dict, List, Optional, Sequence, Tuple

from tools.ch_mcp import game_files
from tools.ch_mcp.frida_tools import MENU_BLOCK, StateUnread, menu_lines, state_words
from tools.ch_mcp.game_tools import TEXT_PREFIX
from tools.ch_probe.input_driver import MENU_KEY_GAP_S, MENU_KEYS, Lane

# The user's ruling 3 in the plan above, 2026-10-30.
# How long a step waits, after its last key, for its expect word to hold.
MENU_STEP_CAP_S = 10.0
# How many Down presses the search step makes looking for the target song.
SEARCH_DOWN_CAP = 10
# How often a step reads the game while it waits.
MENU_POLL_S = 0.5

STEPS_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "menu_steps.json")

# In a step's keys, the text replaced by the song's name from its song.ini.
NAME_FIELD = "{name}"
# In a step's keys, the prefix that marks a drum lane, pressed through
# GameTools.ch_press; every other item goes to GameTools.press_keys as it is.
LANE_PREFIX = "lane:"
# The key the search step presses to move to the next result.
SEARCH_NEXT_KEY = "down"
# Keys no step may press. The manual's Keyboard and Gamepads page says Space
# opens the control mapper from the main menu.
FORBIDDEN_KEYS = ("space",)

# The menu classes the expect words look at, by the name agent/menu.ts sends.
SONG_SELECT = "SongSelect"
PLAYER_SCREEN = "PlayerSelectionMenu"


class StepsInvalid(Exception):
    """menu_steps.json is missing, unreadable or breaks the format load_steps
    checks; the text says which."""


def same_folder(a: Optional[str], b: Optional[str]) -> bool:
    """Whether two folder paths name the same folder, ignoring case, slash
    direction and a trailing slash."""
    if not a or not b:
        return False
    return os.path.normcase(os.path.normpath(a)) == os.path.normcase(os.path.normpath(b))


def _menu(state: dict) -> dict:
    menu = state.get(MENU_BLOCK)
    return menu if isinstance(menu, dict) else {}


def _class_live(state: dict, name: str) -> bool:
    entry = (_menu(state).get("classes") or {}).get(name) or {}
    return bool(entry.get("found")) and (entry.get("live") or 0) > 0


def selected_folder(state: dict) -> Optional[str]:
    """The folder of the song the first live SongSelect has selected, as
    agent/menu.ts sent it, or None."""
    song = _menu(state).get("selectedSong")
    return song.get("folderPath") if isinstance(song, dict) else None


# The expect words a step may name, each a test of one read against the
# target folder. song_select_live and player_screen_live take a live object
# for a menu on screen; menu_steps.json marks the steps that rest on that
# "unproven" until live step LS1 shows what the reads do on each screen.
EXPECT: Dict[str, Callable[[dict, str], bool]] = {
    "song_select_live": lambda state, target: _class_live(state, SONG_SELECT),
    "selected_folder_is_target": lambda state, target: same_folder(selected_folder(state), target),
    "player_screen_live": lambda state, target: _class_live(state, PLAYER_SCREEN),
    "game_manager_live": lambda state, target: bool(state.get("found")),
    "song_loaded": lambda state, target: bool(state.get("found")) and bool(state.get("songLoaded")),
}
# The step that waits for this word is the search step: it hunts with Down.
SEARCH_EXPECT = "selected_folder_is_target"


def online_refusal(state: dict) -> Optional[str]:
    """Why this read does not show the Online menu closed, or None when it
    does. Each onlineMenuActive entry agent/menu.ts sends is false (closed),
    null (no live Online object), true (open) or an error text (the call
    threw); only false and null show it closed. A menu block that is missing
    or failed shows nothing."""
    menu = state.get(MENU_BLOCK)
    if not isinstance(menu, dict):
        return "the read has no menu block, so it cannot show the Online menu is closed"
    if "error" in menu:
        return (f"the menu read failed ({menu['error']}), so it cannot show the Online "
                "menu is closed")
    for name, entry in (menu.get("classes") or {}).items():
        for i, value in enumerate((entry or {}).get("onlineMenuActive") or []):
            if value is True:
                return f"the Online menu is on screen ({name} object {i} reads onlineMenuActive true)"
            if value is not None and value is not False:
                return (f"{name} object {i}'s Online read is {json.dumps(value)}, so the read "
                        "cannot show the Online menu is closed")
    return None


def key_item_problem(item: str) -> Optional[str]:
    """Why `item` is not a key a step may list, or None when it is. A step
    lists a MENU_KEYS name, `text:<string>`, or `lane:<name>` with exactly one
    Lane name. Only a text item may hold NAME_FIELD, so the song's name is
    typed and never pressed or parsed as a key."""
    if item.startswith(TEXT_PREFIX):
        return None
    if NAME_FIELD in item:
        return f"{NAME_FIELD} is allowed only in a {TEXT_PREFIX} item"
    if item.startswith(LANE_PREFIX):
        lane = item[len(LANE_PREFIX):].strip().upper()
        if lane not in Lane.__members__:
            return f"{item!r} is not {LANE_PREFIX} and one lane name"
        return None
    if item.strip().lower() not in MENU_KEYS:
        return f"{item!r} is not a MENU_KEYS name"
    return None


def load_steps(path: str = STEPS_FILE) -> dict:
    """The step file, checked: a list of steps, each with a name, a list of
    keys that pass key_item_problem and an expect word from EXPECT, and no key
    in FORBIDDEN_KEYS. Raises StepsInvalid naming what is wrong."""
    try:
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
    except (OSError, ValueError) as e:
        raise StepsInvalid(f"could not read {path}: {e}") from e
    steps = data.get("steps") if isinstance(data, dict) else None
    if not isinstance(steps, list) or not steps:
        raise StepsInvalid(f"{path} has no steps list")
    for i, step in enumerate(steps, start=1):
        if not isinstance(step, dict) or not isinstance(step.get("name"), str):
            raise StepsInvalid(f"{path}: step {i} has no name")
        keys = step.get("keys")
        if not isinstance(keys, list) or not all(isinstance(k, str) for k in keys):
            raise StepsInvalid(f"{path}: step {i} ({step['name']}) has no keys list")
        bad = [k for k in keys if k.strip().lower() in FORBIDDEN_KEYS]
        if bad:
            raise StepsInvalid(f"{path}: step {i} ({step['name']}) presses {bad[0]}, which "
                               "no step may press (FORBIDDEN_KEYS)")
        for item in keys:
            problem = key_item_problem(item)
            if problem is not None:
                raise StepsInvalid(f"{path}: step {i} ({step['name']}) lists a key that "
                                   f"cannot be pressed: {problem}")
        if step.get("expect") not in EXPECT:
            raise StepsInvalid(f"{path}: step {i} ({step['name']}) expects "
                               f"{step.get('expect')!r}; the words are {', '.join(EXPECT)}")
    return data


class _Stopped(Exception):
    """One ch_open_song run stopped; the text is why."""

    def __init__(self, why: str, state: Optional[dict]):
        super().__init__(why)
        self.state = state


class MenuTools:
    """ch_open_song and the state of one run of it.

    `game` is the server's game_tools.GameTools (ensure_backup, press_keys,
    ch_press); `frida` is its frida_tools.FridaTools (read_state). `now` and
    `sleep` are the clock the waits use, and `steps_path` the step file, so
    the tests swap each for a fake. `lock` is the server's one RLock."""

    def __init__(self, game, frida, lock, *,
                 now: Callable[[], float] = time.monotonic,
                 sleep: Callable[[float], None] = time.sleep,
                 steps_path: str = STEPS_FILE):
        self._game = game
        self._frida = frida
        self._lock = lock
        self._now = now
        self._sleep = sleep
        self._steps_path = steps_path
        self._pressed = False
        self._sent: List[str] = []
        self._lines: List[str] = []
        # The newest read that came back in this run, for a stop that has none of its own.
        self._last_read: Optional[dict] = None

    def ch_open_song(self, song_folder: str) -> str:
        """Start the song in `song_folder` from the menus: for each step of
        menu_steps.json, read the game and refuse while the Online menu may
        be open, press the step's keys, then read every MENU_POLL_S until its
        expect word holds or MENU_STEP_CAP_S passes. One line per step, or
        the step it stopped on and the last read."""
        with self._lock:
            self._pressed = False
            self._sent = []
            self._lines = []
            self._last_read = None
            try:
                steps = load_steps(self._steps_path)["steps"]
            except StepsInvalid as e:
                return f"refused: {e}; nothing pressed"
            name = game_files.read_song_ini(song_folder).get("name")
            if not name:
                return (f"refused: {song_folder} has no {game_files.SONG_INI} with a name, "
                        "so the search has nothing to type; nothing pressed")
            for index, step in enumerate(steps, start=1):
                label = f"step {index} ({step['name']})"
                state: Optional[dict] = None
                try:
                    state = self._read()
                    refusal = online_refusal(state)
                    if refusal is not None:
                        raise _Stopped(f"refused before its keys: {refusal}", state)
                    if index == 1:
                        backup = self._game.ensure_backup()
                        if backup:
                            self._lines.append(backup)
                    state = self._run_step(label, step, name, song_folder, state)
                except _Stopped as e:
                    return self._stopped(label, str(e), e.state)
            self._lines.append(f"done: {song_folder} is loaded")
            return "\n".join(self._lines)

    # -- inside -------------------------------------------------------------------

    def _run_step(self, label: str, step: dict, name: str, target: str, before: dict) -> dict:
        """Press one step's keys and wait for its expect word; the search
        step then hunts with Down. Returns the read that showed the word."""
        expect = step["expect"]
        holds = EXPECT[expect]
        items = [item.replace(NAME_FIELD, name) for item in step["keys"]]
        started = self._now()
        self._press(items)
        if expect == SEARCH_EXPECT:
            state = self._search(label, target, items, selected_folder(before))
        else:
            ok, state = self._wait(label, lambda s: holds(s, target))
            if not ok:
                raise _Stopped(f"{expect} did not hold within {MENU_STEP_CAP_S:g} s "
                               "(MENU_STEP_CAP_S) after its last key", state)
        keys = ", ".join(items) or "no keys"
        self._lines.append(f"{label}: {keys}; {expect} after {self._now() - started:.1f} s")
        return state

    def _search(self, label: str, target: str, items: Sequence[str],
                before: Optional[str]) -> dict:
        """After the typed name, wait for the selection to reach the target
        or move; then press Down, at most SEARCH_DOWN_CAP times, each time
        waiting for the selection to move, until the target is selected."""
        def is_target(s: dict) -> bool:
            return EXPECT[SEARCH_EXPECT](s, target)

        _ok, state = self._wait(label, lambda s: is_target(s) or selected_folder(s) != before)
        downs = 0
        while not is_target(state):
            if downs == SEARCH_DOWN_CAP:
                raise _Stopped(f"the target was not selected after {SEARCH_DOWN_CAP} Down "
                               "presses (SEARCH_DOWN_CAP)", state)
            was = selected_folder(state)
            self._press([SEARCH_NEXT_KEY])
            downs += 1
            moved, state = self._wait(label, lambda s: is_target(s) or selected_folder(s) != was)
            if not moved:
                raise _Stopped(f"Down press {downs} did not move the selection within "
                               f"{MENU_STEP_CAP_S:g} s (MENU_STEP_CAP_S)", state)
        if downs:
            self._lines.append(f"{label}: {downs} Down presses to reach the target")
        return state

    def _press(self, items: Sequence[str]) -> None:
        """Send `items` in order: each run of key and text items in one
        press_keys call, each lane item through press_lanes. Every press call
        after the run's first waits MENU_KEY_GAP_S first. The game's replies
        go in the reply, indented. A press the game refuses (its result says
        sent False) stops the run at once, naming the refusal."""
        groups: List[Tuple[str, List[str]]] = []
        for item in items:
            if item.startswith(LANE_PREFIX):
                groups.append(("lane", [item[len(LANE_PREFIX):]]))
            elif groups and groups[-1][0] == "keys":
                groups[-1][1].append(item)
            else:
                groups.append(("keys", [item]))
        for kind, group in groups:
            if self._pressed:
                self._sleep(MENU_KEY_GAP_S)
            self._pressed = True
            result = self._game.press_lanes(group[0]) if kind == "lane" \
                else self._game.press_keys(group)
            if not result.sent:
                raise _Stopped(f"the game refused the press of {', '.join(group)}, so "
                               f"nothing was sent: {result.text}", self._last_read)
            self._lines.extend("  " + line for line in result.text.splitlines() if line)
            self._sent.extend([LANE_PREFIX + group[0]] if kind == "lane" else group)

    def _wait(self, label: str, holds: Callable[[dict], bool]) -> Tuple[bool, dict]:
        """Read every MENU_POLL_S until `holds` is true of a read or
        MENU_STEP_CAP_S has passed since this call. Stops the run on a read
        that fails or shows the Online menu not closed."""
        deadline = self._now() + MENU_STEP_CAP_S
        while True:
            self._sleep(MENU_POLL_S)
            state = self._read()
            refusal = online_refusal(state)
            if refusal is not None:
                raise _Stopped(f"after its keys: {refusal}", state)
            if holds(state):
                return True, state
            if self._now() >= deadline:
                return False, state

    def _read(self) -> dict:
        try:
            self._last_read = self._frida.read_state()
            return self._last_read
        except StateUnread as e:
            raise _Stopped(f"no read came back: {e}", None) from e

    def _stopped(self, label: str, why: str, state: Optional[dict]) -> str:
        lines = list(self._lines)
        lines.append(f"stopped at {label}: {why}")
        lines.append("keys sent so far: " + (", ".join(self._sent) or "none"))
        if state is None:
            lines.append("last read: none")
        else:
            lines.append("last read:")
            lines.extend("  " + line for line in menu_lines(state.get(MENU_BLOCK)))
            lines.append(f"  state: {state_words(state)}")
        return "\n".join(lines)
