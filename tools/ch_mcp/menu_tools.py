"""The Clone Hero MCP server's menu driver: ch_open_song walks the menus to a
song by pressing the virtual kit's pads and buttons that menu_steps.json
lists, reading the game after every press to see where it is. Nothing it does
touches the keyboard or the window.

The plan is docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md), task K4
("Menus without seeing"); it replaces the key presses of task T6 of
docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md). Each
question has one owner, and this module calls it: kit_tools.KitTools.press_items
presses a kit item and kit_tools.KIT_ELEMENTS names them;
game_tools.GameTools.ensure_backup takes the backup;
frida_tools.FridaTools.read_state is the read, agent/menu.ts sends the menu
block in it, and frida_tools.menu_lines and state_words print it. This module
owns the step file's format, the expect words, the Online refusal, and three of
the user's ruling-3 numbers. The fourth, the gap between presses, is
input_driver's.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server hands in the one RLock it shares with the other
tools, and the kit tools the presses go through.
"""

from __future__ import annotations

import json
import os
import time
from typing import Callable, Dict, List, Optional, Sequence, Tuple

from tools.ch_mcp.bot_run import DEFAULT_PLAYER
from tools.ch_mcp.frida_tools import MENU_BLOCK, StateUnread, menu_fields, menu_lines, state_words
from tools.ch_mcp.kit_tools import KIT_PREFIX, KitRefused, parse_items
from tools.ch_probe.input_driver import MENU_KEY_GAP_S

# The user's ruling 3 in the plan above, 2026-10-30.
# How long a step waits, after its last press, for its expect word to hold.
MENU_STEP_CAP_S = 10.0
# How many scroll presses the search step makes looking for the target song.
SEARCH_DOWN_CAP = 10
# How often a step reads the game while it waits.
MENU_POLL_S = 0.5

STEPS_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "menu_steps.json")

# The kit item the search step presses to move to the next song. No manual
# page says which pads scroll: the plan only notes that the user's
# settings.ini has drum_pad_scrolling = 1, and How to Play says the Blue fret
# opens the search tool. So Blue is a guess: live step LS-K3, which was to
# check it, was struck before it got that far (menu_steps.json's "about" says
# so). The checked-in menu_steps.json has no search step.
SEARCH_NEXT_ITEM = KIT_PREFIX + "blue"

# The menu classes the expect words look at, by the name agent/menu.ts sends.
PROFILE_SCREEN = "PlayerProfileMenu"
MAIN_MENU = "MainMenu"
SONG_SELECT = "SongSelect"
PLAYER_SCREEN = "PlayerSelectionMenu"

NO_KIT = ("refused: the menu driver has no kit tools to press with (the server hands "
          "them in); nothing pressed")


class StepsInvalid(Exception):
    """menu_steps.json is missing, unreadable or breaks the format load_steps
    checks; the text says which."""


def same_folder(a: Optional[str], b: Optional[str]) -> bool:
    """Whether two folder paths name the same folder, ignoring case, slash
    direction and a trailing slash. Both are resolved first, so a junction, a
    symbolic link or an 8.3 short name cannot make one folder look like two."""
    if not a or not b:
        return False
    return os.path.normcase(os.path.realpath(a)) == os.path.normcase(os.path.realpath(b))


def _menu(state: dict) -> dict:
    menu = state.get(MENU_BLOCK)
    return menu if isinstance(menu, dict) else {}


def _class_entry(state: dict, name: str) -> dict:
    entry = (_menu(state).get("classes") or {}).get(name)
    return entry if isinstance(entry, dict) else {}


def _class_live(state: dict, name: str) -> bool:
    entry = _class_entry(state, name)
    return bool(entry.get("found")) and (entry.get("live") or 0) > 0


def _class_active(state: dict, name: str) -> bool:
    """Whether some live instance of the menu class reads `active` true, as
    agent/menu.ts sent it. Null, false and an error text are not true."""
    entry = _class_entry(state, name)
    reads = entry.get("active")
    return bool(entry.get("found")) and isinstance(reads, list) and any(r is True for r in reads)


def selected_folder(state: dict) -> Optional[str]:
    """The folder of the song the first live SongSelect has selected, as
    agent/menu.ts sent it, or None."""
    song = _menu(state).get("selectedSong")
    return song.get("folderPath") if isinstance(song, dict) else None


def loaded_folder(state: dict) -> Optional[str]:
    """GlobalVariables' current song folder, as agent/menu.ts sent it, or
    None."""
    globals_ = _menu(state).get("globals")
    return globals_.get("currentSongFolder") if isinstance(globals_, dict) else None


# The expect words a step may name, each a test of one read against the
# target folder. The _live words take a live object for a menu on screen; the
# _active words take a live object whose GameObject is active. menu_steps.json
# marks the steps that rest on either "unproven". Live step LS-K3 was to settle
# those marks and set the file's "verified" true; its two runs stopped on the
# player screen and it was struck, so the marks and the false flag stand. What
# those runs read is on Clone Hero page 07 (IN-79).
EXPECT: Dict[str, Callable[[dict, str], bool]] = {
    "song_select_live": lambda state, target: _class_live(state, SONG_SELECT),
    "selected_folder_is_target": lambda state, target: same_folder(selected_folder(state), target),
    "player_screen_live": lambda state, target: _class_live(state, PLAYER_SCREEN),
    "game_manager_live": lambda state, target: bool(state.get("found")),
    "song_loaded": lambda state, target: bool(state.get("found")) and bool(state.get("songLoaded")),
    "profile_screen_active": lambda state, target: _class_active(state, PROFILE_SCREEN),
    "main_menu_active": lambda state, target: _class_active(state, MAIN_MENU),
    "song_select_active": lambda state, target: _class_active(state, SONG_SELECT),
    "player_screen_active": lambda state, target: _class_active(state, PLAYER_SCREEN),
}
# The step that waits for this word is the search step: it scrolls with
# SEARCH_NEXT_ITEM.
SEARCH_EXPECT = "selected_folder_is_target"


# The two player-screen fields the instrument and difficulty check reads, by
# their menu_fields.json keys.
INSTRUMENT_KEY = "instrument"
DIFFICULTY_KEY = "difficulty"


def _field_words(key: str) -> Dict[str, str]:
    """menu_fields.json's word table for the enum field `key`, by number."""
    for field in menu_fields()["fields"]:
        if field["key"] == key:
            return field["values"]
    raise KeyError(f"menu_fields.json has no {key} field")


def player_target(player: str) -> Tuple[str, str]:
    """The instrument and difficulty words `player` names, in ch_start's
    "<Instrument>,<Difficulty>" form. Raises ValueError when it is not two
    words that menu_fields.json's tables hold."""
    parts = [part.strip() for part in player.split(",")]
    if len(parts) != 2:
        raise ValueError(f"player {player!r} is not <Instrument>,<Difficulty>")
    for key, word in zip((INSTRUMENT_KEY, DIFFICULTY_KEY), parts):
        words = _field_words(key)
        if word not in words.values():
            raise ValueError(f"player {player!r}: {word!r} is not a {key} word in "
                             f"menu_fields.json ({', '.join(words.values())})")
    return parts[0], parts[1]


def player_screen_refusal(state: dict, instrument: str, difficulty: str) -> Optional[str]:
    """Why this read does not show every live player screen set to
    `instrument` and `difficulty`, or None when it does. The numbers are the
    agent's PlayerSelectionMenu fields, read as words through
    menu_fields.json's tables."""
    fields = _class_entry(state, PLAYER_SCREEN).get("fields")
    if not isinstance(fields, dict):
        return (f"the read has no {PLAYER_SCREEN} fields, so it cannot show the instrument "
                "and difficulty")
    for key, want in ((INSTRUMENT_KEY, instrument), (DIFFICULTY_KEY, difficulty)):
        reads = fields.get(key)
        if not isinstance(reads, list) or not reads:
            return (f"{PLAYER_SCREEN}'s {key} read is {json.dumps(reads)}, so the read "
                    f"cannot show the {key} is {want}")
        words = _field_words(key)
        for i, value in enumerate(reads):
            word = (words.get(str(value)) if isinstance(value, int)
                    and not isinstance(value, bool) else None)
            if word != want:
                return (f"{PLAYER_SCREEN} object {i} reads {key} {json.dumps(value)}"
                        + (f" ({word})" if word is not None else "")
                        + f", not {want}")
    return None


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
    classes = menu.get("classes")
    if not isinstance(classes, dict):
        return "the menu block has no classes, so it cannot show the Online menu is closed"
    for name, entry in classes.items():
        reads = (entry or {}).get("onlineMenuActive") or []
        if not isinstance(reads, list):
            return (f"{name}'s Online read is {json.dumps(reads)}, not a list, so it cannot "
                    "show the Online menu is closed")
        for i, value in enumerate(reads):
            if value is True:
                return f"the Online menu is on screen ({name} object {i} reads onlineMenuActive true)"
            if value is not None and value is not False:
                return (f"{name} object {i}'s Online read is {json.dumps(value)}, so the read "
                        "cannot show the Online menu is closed")
    return None


def item_problem(item: str) -> Optional[str]:
    """Why `item` is not an item a step may list, or None when it is. A step
    lists only kit items: KIT_PREFIX and one name kit_tools.parse_items
    accepts. Anything else (a key name, a lane, typed text) is refused,
    because the kit route presses no keys."""
    if not item.strip().lower().startswith(KIT_PREFIX):
        return f"{item!r} is not a {KIT_PREFIX} item, and the kit route presses no keys"
    try:
        parse_items([item])
    except KitRefused as e:
        return str(e)
    return None


def load_steps(path: str = STEPS_FILE) -> dict:
    """The step file, checked: a list of steps, each with a name, a list of
    items that pass item_problem and an expect word from EXPECT. Raises
    StepsInvalid naming what is wrong."""
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
        items = step.get("items")
        if not isinstance(items, list) or not all(isinstance(k, str) for k in items):
            raise StepsInvalid(f"{path}: step {i} ({step['name']}) has no items list")
        for item in items:
            problem = item_problem(item)
            if problem is not None:
                raise StepsInvalid(f"{path}: step {i} ({step['name']}) lists an item that "
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

    `game` is the server's game_tools.GameTools (ensure_backup); `frida` is
    its frida_tools.FridaTools (read_state); `kit` is its kit_tools.KitTools
    (press_items), and with none every run is refused. `now` and `sleep` are
    the clock the waits use, and `steps_path` the step file, so the tests swap
    each for a fake. `lock` is the server's one RLock."""

    def __init__(self, game, frida, lock, *, kit=None,
                 now: Callable[[], float] = time.monotonic,
                 sleep: Callable[[float], None] = time.sleep,
                 steps_path: str = STEPS_FILE):
        self._game = game
        self._frida = frida
        self._kit = kit
        self._lock = lock
        self._now = now
        self._sleep = sleep
        self._steps_path = steps_path
        self._pressed = False
        self._sent: List[str] = []
        self._lines: List[str] = []
        # The newest read that came back in this run, for a stop that has none of its own.
        self._last_read: Optional[dict] = None

    def ch_open_song(self, song_folder: str, player: str = DEFAULT_PLAYER) -> str:
        """Walk the menus toward the song in `song_folder`, to play it as
        `player` (ch_start's "<Instrument>,<Difficulty>"): for each step of
        menu_steps.json, read the game and refuse while the Online menu may
        be open or, on the player screen, while player_screen_refusal finds
        another instrument or difficulty; press the step's kit items, then
        read every MENU_POLL_S until its expect word holds or MENU_STEP_CAP_S
        passes. One line per step and a last line naming the song the game
        loaded, or the step it stopped on and the last read. A walk that ends
        on another song's folder, or that never read the player screen
        before a press, is a refusal too (D318 item 4)."""
        with self._lock:
            self._pressed = False
            self._sent = []
            self._lines = []
            self._last_read = None
            try:
                steps = load_steps(self._steps_path)["steps"]
            except StepsInvalid as e:
                return f"refused: {e}; nothing pressed"
            try:
                instrument, difficulty = player_target(player)
            except ValueError as e:
                return f"refused: {e}; nothing pressed"
            if self._kit is None:
                return NO_KIT
            state: Optional[dict] = None
            player_checked = False
            label = ""
            for index, step in enumerate(steps, start=1):
                label = f"step {index} ({step['name']})"
                try:
                    state = self._read()
                    refusal = online_refusal(state)
                    if refusal is not None:
                        raise _Stopped(f"refused before its presses: {refusal}", state)
                    if step["items"] and _class_active(state, PLAYER_SCREEN):
                        refusal = player_screen_refusal(state, instrument, difficulty)
                        if refusal is not None:
                            raise _Stopped(f"refused before its presses: {refusal}", state)
                        player_checked = True
                    if index == 1:
                        backup = self._game.ensure_backup()
                        if backup:
                            self._lines.append(backup)
                    state = self._run_step(label, step, song_folder, state)
                except _Stopped as e:
                    return self._stopped(label, str(e), e.state)
            refusal = self._end_refusal(state or {}, song_folder, player, player_checked)
            if refusal is not None:
                return self._stopped(label, refusal, state)
            self._lines.append(f"done: {song_folder} is loaded, as {instrument} on {difficulty}")
            return "\n".join(self._lines)

    # -- inside -------------------------------------------------------------------

    @staticmethod
    def _end_refusal(state: dict, target: str, player: str, player_checked: bool) -> Optional[str]:
        """Why a walk whose steps all held still did not open the target as
        `player`: the song GlobalVariables says is loaded is not the target,
        or no press was made on the player screen after its check. None when
        neither."""
        loaded = loaded_folder(state)
        if not same_folder(loaded, target):
            return (f"refused: a song is loaded, but its folder is {json.dumps(loaded)}, not "
                    f"the target {target}")
        if not player_checked:
            return (f"refused: no step pressed on the player screen, so its instrument and "
                    f"difficulty were never checked against {player}")
        return None

    def _run_step(self, label: str, step: dict, target: str, before: dict) -> dict:
        """Press one step's items and wait for its expect word; the search
        step then scrolls. Returns the read that showed the word."""
        expect = step["expect"]
        holds = EXPECT[expect]
        items = list(step["items"])
        started = self._now()
        self._press(items)
        if expect == SEARCH_EXPECT:
            state = self._search(label, target, before, bool(items))
        else:
            ok, state = self._wait(label, lambda s: holds(s, target))
            if not ok:
                raise _Stopped(f"{expect} did not hold within {MENU_STEP_CAP_S:g} s "
                               "(MENU_STEP_CAP_S) after its last press", state)
        pressed = ", ".join(items) or "no presses"
        self._lines.append(f"{label}: {pressed}; {expect} after {self._now() - started:.1f} s")
        return state

    def _search(self, label: str, target: str, before: dict, pressed: bool) -> dict:
        """After the step's own presses, if it had any, wait for the selection
        to reach the target or move; then press SEARCH_NEXT_ITEM, at most
        SEARCH_DOWN_CAP times, each time waiting for the selection to move,
        until the target is selected. `before` is the read before the step."""
        def is_target(s: dict) -> bool:
            return EXPECT[SEARCH_EXPECT](s, target)

        state = before
        if pressed:
            was = selected_folder(before)
            _ok, state = self._wait(label, lambda s: is_target(s) or selected_folder(s) != was)
        scrolls = 0
        while not is_target(state):
            if scrolls == SEARCH_DOWN_CAP:
                raise _Stopped(f"the target was not selected after {SEARCH_DOWN_CAP} "
                               f"{SEARCH_NEXT_ITEM} presses (SEARCH_DOWN_CAP)", state)
            was = selected_folder(state)
            self._press([SEARCH_NEXT_ITEM])
            scrolls += 1
            moved, state = self._wait(label, lambda s: is_target(s) or selected_folder(s) != was)
            if not moved:
                raise _Stopped(f"{SEARCH_NEXT_ITEM} press {scrolls} did not move the selection "
                               f"within {MENU_STEP_CAP_S:g} s (MENU_STEP_CAP_S)", state)
        if scrolls:
            self._lines.append(f"{label}: {scrolls} {SEARCH_NEXT_ITEM} presses to reach the "
                               "target")
        return state

    def _press(self, items: Sequence[str]) -> None:
        """Press `items` in order, one kit press each through
        KitTools.press_items. Every press after the run's first waits
        MENU_KEY_GAP_S first. The kit's replies go in the reply, indented. A
        press the kit refuses (its result says sent False) stops the run at
        once, naming the refusal."""
        for item in items:
            if self._pressed:
                self._sleep(MENU_KEY_GAP_S)
            self._pressed = True
            result = self._kit.press_items([item])
            if not result.sent:
                raise _Stopped(f"the kit refused the press of {item}, so nothing was "
                               f"pressed: {result.text}", self._last_read)
            self._lines.extend("  " + line for line in result.text.splitlines() if line)
            self._sent.append(item)

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
                raise _Stopped(f"after its presses: {refusal}", state)
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
        lines.append("kit items pressed so far: " + (", ".join(self._sent) or "none"))
        if state is None:
            lines.append("last read: none")
        else:
            lines.append("last read:")
            lines.extend("  " + line for line in menu_lines(state.get(MENU_BLOCK)))
            lines.append(f"  state: {state_words(state)}")
        return "\n".join(lines)
