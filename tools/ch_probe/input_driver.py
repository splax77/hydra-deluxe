"""The input driver: fire drum-lane keystrokes at the running game.

What this does, in one line: it presses and releases a keyboard key for a
drum lane. It also presses the non-lane keys in MENU_KEYS and types text,
for driving the game's menus. Waiting for the song clock to reach a note is not its job; the
runners do that with experiments/live.wait_until, then call press_chord.

Why the timing can be sloppy. Windows cannot deliver a keystroke at an exact
millisecond -- the scheduler jitters by several ms, which is huge next to an
85 ms hit window. That would be fatal if we needed the input to be precise.
It is not. The game stores its own hit time and the runners read that number
back, so what we record is what the engine measured, not what we intended.
We only have to land the key NEAR the note edge.

One seam here can only be exercised by a live game, and is marked LIVE-ONLY
in the code: send_input, the real ctypes SendInput call. It needs a real
desktop and the game window in focus. Unit tests monkeypatch it, or send_key
above it.

Everything else -- the key tables and the press logic -- is plain Python and
is unit-tested with fakes.
"""

from __future__ import annotations

import ctypes
import time
from ctypes import wintypes
from enum import IntEnum
from typing import Callable, Dict, Iterable, List, Optional

try:
    from . import constants as C
except ImportError:  # pragma: no cover - top-level import, ch_probe on sys.path
    import constants as C  # type: ignore[no-redef]

# How long press_chord holds a chord's keys down: the 3 ms hold play_chart
# proved at the game, kept as it is (phase 6, task J3-8).
KEY_HOLD_S = 0.003


# --- The one key table ---------------------------------------------------------


class Lane(IntEnum):
    """One input lane per bound key. The number means the same thing everywhere
    in ch_probe: 0 is the green pad, 4 is the kick. A .chart numbers its notes
    differently (note 0 is the kick), so chart code says "note", never "lane"."""

    GREEN = 0
    RED = 1
    YELLOW = 2
    BLUE = 3
    KICK = 4
    YELLOW_CYMBAL = 5
    BLUE_CYMBAL = 6
    GREEN_CYMBAL = 7


# Windows virtual-key codes, read off the game's own Controller Remap screen
# (Options -> Controls, Player1 drums) on 2026-09-25:
#   Green=A  Red=S  Yellow=J  Blue=K  Orange/Kick=L  2X Kick=O
#   Yellow Cymbal=U  Blue Cymbal=Y  Green Cymbal=T
# ch_probe presses L for the 2x kick too (either pedal hits either kick), so
# O has no lane here. play_chart.py hits notes with these. A different install may rebind them;
# set_binding() overrides one lane.
DEFAULT_BINDINGS: Dict[int, int] = {
    Lane.GREEN: 0x41,          # 'A'
    Lane.RED: 0x53,            # 'S'
    Lane.YELLOW: 0x4A,         # 'J'
    Lane.BLUE: 0x4B,           # 'K'
    Lane.KICK: 0x4C,           # 'L' (orange)
    Lane.YELLOW_CYMBAL: 0x55,  # 'U'
    Lane.BLUE_CYMBAL: 0x59,    # 'Y'
    Lane.GREEN_CYMBAL: 0x54,   # 'T'
}

# Short names for printed logs (moved from play_chart.py).
LANE_NAMES: Dict[int, str] = {
    Lane.GREEN: "Grn", Lane.RED: "Red", Lane.YELLOW: "Yel", Lane.BLUE: "Blu",
    Lane.KICK: "Kick", Lane.YELLOW_CYMBAL: "YCym", Lane.BLUE_CYMBAL: "BCym",
    Lane.GREEN_CYMBAL: "GCym",
}


# --- The menu keys ---------------------------------------------------------------
#
# The one table of keys that are not drum lanes, by the name a caller types
# (the MCP server's ch_key). The values are Windows virtual-key codes. The
# menu meanings come from the manual's "Keyboard and Gamepads" page
# (https://wiki.clonehero.net/books/guitars-drums-controllers/page/keyboard-and-gamepads),
# read 2026-10-30; a key that page gives no menu meaning says so.
MENU_KEYS: Dict[str, int] = {
    "enter": 0x0D,      # VK_RETURN: Start/Pause on that page
    "escape": 0x1B,     # VK_ESCAPE: no menu meaning on that page
    "space": 0x20,      # VK_SPACE: opens the control mapper from the main menu, per that page
    "backspace": 0x08,  # VK_BACK: no menu meaning on that page
    "tab": 0x09,        # VK_TAB: no menu meaning on that page
    "up": 0x26,         # VK_UP: Strum Up on that page
    "down": 0x28,       # VK_DOWN: Strum Down on that page
    "left": 0x25,       # VK_LEFT: no menu meaning on that page
    "right": 0x27,      # VK_RIGHT: no menu meaning on that page
}

# The wait between two menu keys, and between two typed characters (the
# user's ruling 3 in docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md).
MENU_KEY_GAP_S = 0.25


# --- Win32 SendInput plumbing (used only by the live seam) -------------------
#
# The INPUT structure Windows expects for SendInput. We only ever send keyboard
# events, but the struct's size must match what Windows knows (40 bytes on
# x64), so the union is padded out to the size of the largest member.

INPUT_KEYBOARD = 1
KEYEVENTF_KEYUP = 0x0002
# The event carries a character in wScan instead of a key in wVk: the way
# SendInput types text whatever the keyboard layout.
KEYEVENTF_UNICODE = 0x0004

# Pointer-sized unsigned integer, used for the dwExtraInfo field.
ULONG_PTR = ctypes.c_size_t


class _KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", wintypes.WORD),
        ("wScan", wintypes.WORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ULONG_PTR),
    ]


class _INPUTUNION(ctypes.Union):
    # Pad to 32 bytes so sizeof(INPUT) matches the real MOUSEINPUT-sized union.
    _fields_ = [("ki", _KEYBDINPUT), ("_pad", ctypes.c_byte * 32)]


class _INPUT(ctypes.Structure):
    _fields_ = [("type", wintypes.DWORD), ("u", _INPUTUNION)]


def _key_input(vk: int, scan: int, flags: int) -> _INPUT:
    """One keyboard INPUT: a virtual key in `vk`, or a character in `scan`
    when `flags` holds KEYEVENTF_UNICODE."""
    event = _INPUT()
    event.type = INPUT_KEYBOARD
    event.u.ki = _KEYBDINPUT(wVk=vk, wScan=scan, dwFlags=flags, time=0, dwExtraInfo=0)
    return event


class InputDriver:
    """Send drum-lane keystrokes.

    Implements the InputDriver Protocol in interfaces.py.
    """

    def __init__(self, bindings: Optional[Dict[int, int]] = None) -> None:
        # A copy of the defaults so callers can override lanes one at a time
        # without mutating the shared module-level dict.
        self._bindings: Dict[int, int] = dict(
            DEFAULT_BINDINGS if bindings is None else bindings
        )

    # -- bindings -------------------------------------------------------------

    def set_binding(self, lane: int, vk: int) -> None:
        """Map a drum lane to a Windows virtual-key code.

        Use this when an install binds a lane to a different key than the
        DEFAULT_BINDINGS above (read off the bind screen on 2026-09-25).
        """
        self._bindings[lane] = vk

    def get_binding(self, lane: int) -> int:
        """Return the virtual-key code currently mapped to `lane`.

        Raises KeyError if the lane has no binding, so a missing lane fails
        loudly rather than pressing the wrong key.
        """
        return self._bindings[lane]

    # -- pressing keys --------------------------------------------------------

    def tap(self, lane: int) -> None:
        """Press then release the key for `lane` (key-down, key-up)."""
        vk = self.get_binding(lane)
        self.send_key(vk, key_up=False)
        self.send_key(vk, key_up=True)

    def press_chord(self, lanes: Iterable[int], *, hold_s: float = KEY_HOLD_S,
                    sleep: Callable[[float], None] = time.sleep) -> List[int]:
        """Press every lane's key down, hold `hold_s`, then release them all.

        This is the press play_chart.py proved at the game: a chord's keys go
        down together so the game sees one chord. A lane with no binding is
        skipped, not guessed. Returns the keys pressed.
        """
        vks: List[int] = []
        for lane in lanes:
            try:
                vk = self.get_binding(lane)
            except KeyError:
                continue
            vks.append(vk)
            self.send_key(vk, key_up=False)
        sleep(hold_s)
        for vk in vks:
            self.send_key(vk, key_up=True)
        return vks

    def press_key(self, vk: int, *, hold_s: float = KEY_HOLD_S,
                  sleep: Callable[[float], None] = time.sleep) -> int:
        """Press one key (a MENU_KEYS code, say): down, hold `hold_s`, up.
        Returns the key pressed."""
        self.send_key(vk, key_up=False)
        sleep(hold_s)
        self.send_key(vk, key_up=True)
        return vk

    def type_text(self, text: str, *, gap_s: float,
                  sleep: Callable[[float], None] = time.sleep) -> int:
        """Type `text` one character at a time as Unicode key events, waiting
        `gap_s` between two characters. A character outside the 16-bit range
        goes as its two UTF-16 halves. Returns the characters typed."""
        for i, char in enumerate(text):
            if i:
                sleep(gap_s)
            data = char.encode("utf-16-le")
            for unit in (int.from_bytes(data[j:j + 2], "little")
                         for j in range(0, len(data), 2)):
                self.send_unicode(unit, key_up=False)
                self.send_unicode(unit, key_up=True)
        return len(text)

    def send_key(self, vk: int, key_up: bool) -> None:
        """One key event for the virtual key `vk`, through send_input.
        Tests replace this method with a recorder."""
        self.send_input(_key_input(vk, 0, KEYEVENTF_KEYUP if key_up else 0))

    def send_unicode(self, unit: int, key_up: bool) -> None:
        """One typed-character event for the UTF-16 code unit `unit`, through
        send_input."""
        flags = KEYEVENTF_UNICODE | (KEYEVENTF_KEYUP if key_up else 0)
        self.send_input(_key_input(0, unit, flags))

    def send_input(self, event: "_INPUT") -> None:
        """LIVE-ONLY seam: one real keyboard event via Win32 SendInput.

        This is the only place that touches the OS. It needs a real desktop and
        the game window in focus, so it cannot run under unit tests -- tests
        replace this method (or send_key) with a recorder. Everything above it
        is pure logic.
        """
        sent = ctypes.windll.user32.SendInput(
            1, ctypes.byref(event), ctypes.sizeof(_INPUT)
        )
        if sent != 1:
            err = ctypes.get_last_error()
            raise OSError(f"SendInput failed (returned {sent}, GetLastError={err})")


# --- The game window ------------------------------------------------------------
#
# A key only reaches the game while its window has focus. Every runner finds
# the window and focuses it through these two; what a runner does when there
# is no window (warn and go on, or stop) stays its own choice.


def find_game_window(find: Optional[Callable[[Optional[str], str], Optional[int]]] = None) -> int:
    """The game window's handle, or 0 when no window has the game's title.
    `find` takes (class, title) and stands in for user32's window lookup in
    tests.
    LIVE-ONLY by default: it asks the real desktop."""
    if find is None:
        find = ctypes.windll.user32.FindWindowW
    return find(None, C.WINDOW_TITLE) or 0


def focus_window(hwnd: int, focus: Optional[Callable[[int], object]] = None) -> None:
    """Bring the window `hwnd` to the front; a 0 handle (no window) does
    nothing. `focus` stands in for user32.SetForegroundWindow in tests."""
    if not hwnd:
        return
    if focus is None:
        focus = ctypes.windll.user32.SetForegroundWindow
    focus(hwnd)
