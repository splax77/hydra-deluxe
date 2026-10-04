"""The input driver: fire drum-lane keystrokes at the running game.

What this does, in one line: it presses and releases a keyboard key for a
drum lane. Waiting for the song clock to reach a note is not its job; the
runners do that with experiments/live.wait_until, then call press_chord.

Why the timing can be sloppy. Windows cannot deliver a keystroke at an exact
millisecond -- the scheduler jitters by several ms, which is huge next to an
85 ms hit window. That would be fatal if we needed the input to be precise.
It is not. The game stores its own hit time and the runners read that number
back, so what we record is what the engine measured, not what we intended.
We only have to land the key NEAR the note edge.

One seam here can only be exercised by a live game, and is marked LIVE-ONLY
in the code: send_key, the real ctypes SendInput call. It needs a real
desktop and the game window in focus. Unit tests monkeypatch it.

Everything else -- the binding map and the press logic -- is plain Python and
is unit-tested with fakes.
"""

from __future__ import annotations

import ctypes
import time
from ctypes import wintypes
from enum import IntEnum
from typing import Callable, Dict, Iterable, List, Optional


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


# --- Win32 SendInput plumbing (used only by the live seam) -------------------
#
# The INPUT structure Windows expects for SendInput. We only ever send keyboard
# events, but the struct's size must match what Windows knows (40 bytes on
# x64), so the union is padded out to the size of the largest member.

INPUT_KEYBOARD = 1
KEYEVENTF_KEYUP = 0x0002

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

    def press_chord(self, lanes: Iterable[int], *, hold_s: float = 0.003,
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

    def send_key(self, vk: int, key_up: bool) -> None:
        """LIVE-ONLY seam: one real key event via Win32 SendInput.

        This is the only place that touches the OS. It needs a real desktop and
        the game window in focus, so it cannot run under unit tests -- tests
        replace this method with a recorder. Everything above it is pure logic.
        """
        flags = KEYEVENTF_KEYUP if key_up else 0
        event = _INPUT()
        event.type = INPUT_KEYBOARD
        event.u.ki = _KEYBDINPUT(
            wVk=vk,
            wScan=0,
            dwFlags=flags,
            time=0,
            dwExtraInfo=0,
        )
        sent = ctypes.windll.user32.SendInput(
            1, ctypes.byref(event), ctypes.sizeof(_INPUT)
        )
        if sent != 1:
            err = ctypes.get_last_error()
            raise OSError(f"SendInput failed (returned {sent}, GetLastError={err})")
