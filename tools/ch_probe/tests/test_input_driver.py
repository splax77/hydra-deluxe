"""Unit tests for the input driver's pure logic.

These run with no game and no debugger. They test the binding map, the key
table against the game's own saved key map, and that press_chord sends the
right key events.

The key codes and SendInput flags are checked against Windows' own names for
them, and the game-window lookup runs for real, read only. The live seam (the
real SendInput) is replaced with a recorder here. What only
a running Clone Hero can exercise -- that SendInput
actually reaches the game window -- is out of scope for these tests and is
commented as LIVE-ONLY in the module.

Run from the repo root:
    python -m pytest tools/ch_probe/tests/test_input_driver.py -q
    (or, if pytest is absent) python -m unittest tools.ch_probe.tests.test_input_driver
"""

from __future__ import annotations

import ctypes
import os
import sys
import unittest
import xml.etree.ElementTree as ET
from ctypes import wintypes

# Windows' own names for its key codes and SendInput flags: pywin32's copy of
# the Win32 headers. The mcp package that CI installs requires pywin32.
import win32con

# Make the repo root importable so `tools.ch_probe...` resolves regardless of
# where the test runner is launched from.
_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import kit_tools as K
from tools.ch_probe import constants as C
from tools.ch_probe import input_driver
from tools.ch_probe.input_driver import DEFAULT_BINDINGS, LANE_NAMES, InputDriver, Lane


def _make_driver():
    """An InputDriver whose key press is recorded, not really sent.

    Returns (driver, presses) where `presses` is a list of (vk, key_up).
    """
    driver = InputDriver()
    presses = []
    # Replace the LIVE-ONLY SendInput seam with a recorder. tap() still runs its
    # real down/up logic on top of this.
    driver.send_key = lambda vk, key_up: presses.append((vk, key_up))
    return driver, presses


# Player1's drums keyboard map as the game saved it in its Rewired registry
# key: the header, the key's line and that one value, copied verbatim from
# hydra-data\ch-mcp-runs\2026-11-12\ls-g1-run1\backup-20261112-182956\rewired.reg,
# the export ch_start took before that run.
RECORDED_DRUMS_KEYBOARD_MAP = os.path.join(
    _REPO_ROOT, "testdata", "ch_runs", "ls-g1-run1", "rewired-key",
    "player1-drums-keyboard-map.reg")
_REWIRED_NS = "{http://guavaman.com/rewired}"


def _recorded_drums_keyboard_map():
    """(value name, {Rewired action id: Unity key code}) of the one hex value
    in RECORDED_DRUMS_KEYBOARD_MAP, its enabled key bindings only."""
    with open(RECORDED_DRUMS_KEYBOARD_MAP, encoding="utf-16") as f:
        text = f.read().replace("\\\n  ", "")
    values = [line for line in text.splitlines() if "=hex:" in line]
    assert len(values) == 1, values
    name, data = values[0].split("=hex:", 1)
    xml = bytes(int(b, 16) for b in data.split(",")).rstrip(b"\0").decode("utf-8")
    root = ET.fromstring(xml.split("?>", 1)[1])
    keys = {}
    for binding in root.iter(_REWIRED_NS + "ActionElementMap"):
        if binding.findtext(_REWIRED_NS + "enabled") == "true":
            keys[int(binding.findtext(_REWIRED_NS + "actionId"))] = int(
                binding.findtext(_REWIRED_NS + "keyboardKeyCode"))
    return name.strip('"'), keys


def _lane_action(lane):
    """The game's Rewired action for a lane, through the kit's own tables."""
    return K.KIT_BINDINGS[K.lane_element(lane).id][0]


def _virtual_key(unity_key):
    """The Windows virtual key of a letter key in Unity's KeyCode numbering,
    which numbers a letter by its lower-case character
    (https://docs.unity3d.com/ScriptReference/KeyCode.html); Windows numbers
    it by its upper-case one."""
    letter = chr(unity_key)
    assert letter.isascii() and letter.isalpha(), unity_key
    return ord(letter.upper())


class TestBindings(unittest.TestCase):
    def test_defaults_are_present(self):
        driver = InputDriver()
        # Every lane in DEFAULT_BINDINGS is readable.
        for lane, vk in DEFAULT_BINDINGS.items():
            self.assertEqual(driver.get_binding(lane), vk)

    def test_missing_lane_raises(self):
        driver = InputDriver(bindings={})
        with self.assertRaises(KeyError):
            driver.get_binding(99)

    def test_constructor_copies_defaults(self):
        # Overriding one driver's lane must not leak into the module defaults.
        driver = InputDriver()
        original = DEFAULT_BINDINGS[0]
        driver.set_binding(0, 0x99)
        self.assertEqual(DEFAULT_BINDINGS[0], original)


class TestKeyTable(unittest.TestCase):
    """One key table. A lane number means the same key everywhere."""

    def test_every_lane_presses_the_key_the_game_saved_for_it(self):
        name, keys = _recorded_drums_keyboard_map()
        # The value is the game's Gameplay / Drums map, the one the kit uses too.
        self.assertIn(f"|categoryId={K.KIT_MAP_CATEGORY_ID}|layoutId={K.KIT_MAP_LAYOUT_ID}|",
                      name)
        for lane in Lane:
            self.assertIn(_lane_action(lane), keys, lane.name)
            self.assertEqual(DEFAULT_BINDINGS[lane], _virtual_key(keys[_lane_action(lane)]),
                             lane.name)

    def test_every_lane_has_its_own_key_and_its_own_kit_pad(self):
        # A lane number is how press_chord finds a key and the kit finds a pad.
        self.assertEqual(set(DEFAULT_BINDINGS), set(Lane))
        self.assertEqual(len(set(DEFAULT_BINDINGS.values())), len(Lane))
        actions = [_lane_action(lane) for lane in Lane]
        self.assertEqual(len(set(actions)), len(Lane))

    def test_every_lane_has_its_own_short_name(self):
        # A printed press log names each lane by LANE_NAMES, so two lanes must
        # never print the same name, and none may print blank.
        self.assertEqual(set(LANE_NAMES), set(Lane))
        names = [LANE_NAMES[lane] for lane in Lane]
        self.assertTrue(all(names), names)
        self.assertEqual(len(set(names)), len(Lane), names)


class TestPressChord(unittest.TestCase):
    """The press play_chart proved at the game: all down, hold, all up."""

    def test_all_down_then_all_up(self):
        driver, presses = _make_driver()
        sent = driver.press_chord([Lane.RED, Lane.KICK],
                                  sleep=lambda s: presses.append(("sleep", s)))
        red, kick = DEFAULT_BINDINGS[Lane.RED], DEFAULT_BINDINGS[Lane.KICK]
        self.assertEqual(presses, [(red, False), (kick, False),
                                   ("sleep", input_driver.KEY_HOLD_S),
                                   (red, True), (kick, True)])
        self.assertEqual(sent, [red, kick])

    def test_a_chord_goes_out_as_virtual_key_events(self):
        driver = InputDriver()
        events = []
        driver.send_input = lambda event: events.append(
            (event.type, event.u.ki.wVk, event.u.ki.dwFlags))
        driver.press_chord([Lane.RED], sleep=lambda s: None)
        red = DEFAULT_BINDINGS[Lane.RED]
        self.assertEqual([(kind, vk) for kind, vk, _ in events],
                         [(win32con.INPUT_KEYBOARD, red)] * 2)
        (_, _, down), (_, _, up) = events
        # A virtual-key event, never a typed character; down first, then up.
        self.assertFalse(down & win32con.KEYEVENTF_UNICODE)
        self.assertFalse(up & win32con.KEYEVENTF_UNICODE)
        self.assertFalse(down & win32con.KEYEVENTF_KEYUP)
        self.assertTrue(up & win32con.KEYEVENTF_KEYUP)


class TestMenuKeys(unittest.TestCase):
    """The non-lane keys the menus take."""

    def test_the_menu_key_table(self):
        # Each name's code is Windows' virtual key of that name.
        vk_names = {"enter": "VK_RETURN", "escape": "VK_ESCAPE", "space": "VK_SPACE",
                    "backspace": "VK_BACK", "tab": "VK_TAB", "up": "VK_UP",
                    "down": "VK_DOWN", "left": "VK_LEFT", "right": "VK_RIGHT"}
        self.assertEqual(set(input_driver.MENU_KEYS), set(vk_names))
        for name, vk_name in vk_names.items():
            self.assertEqual(input_driver.MENU_KEYS[name], getattr(win32con, vk_name), name)

    def test_the_sendinput_numbers_are_windows_own(self):
        for name in ("INPUT_KEYBOARD", "KEYEVENTF_KEYUP", "KEYEVENTF_UNICODE"):
            self.assertEqual(getattr(input_driver, name), getattr(win32con, name), name)

    def test_press_key_sends_down_hold_then_up(self):
        driver, presses = _make_driver()
        enter = input_driver.MENU_KEYS["enter"]
        sent = driver.press_key(enter, sleep=lambda s: presses.append(("sleep", s)))
        self.assertEqual(presses, [(enter, False), ("sleep", input_driver.KEY_HOLD_S),
                                   (enter, True)])
        self.assertEqual(sent, enter)


class TestGameWindow(unittest.TestCase):
    """The one way a runner finds and focuses the game window."""

    def test_find_game_window_finds_the_window_with_the_games_title(self):
        # The real lookup on this desktop. It only reads window titles: it
        # sends nothing, and it works with or without the game open. The
        # expected answer comes from listing every top-level window's title,
        # which FindWindowW matches without regard to case.
        user32 = ctypes.WinDLL("user32", use_last_error=True)
        user32.GetWindowTextLengthW.argtypes = [wintypes.HWND]
        user32.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
        enum_proc = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
        titled = set()

        def visit(hwnd, _):
            size = user32.GetWindowTextLengthW(hwnd) + 1
            text = ctypes.create_unicode_buffer(size)
            user32.GetWindowTextW(hwnd, text, size)
            if text.value.casefold() == C.WINDOW_TITLE.casefold():
                titled.add(hwnd)
            return True

        self.assertTrue(user32.EnumWindows(enum_proc(visit), 0), ctypes.get_last_error())
        found = input_driver.find_game_window()
        if titled:
            self.assertIn(found, titled)
        else:
            self.assertEqual(found, 0)

    def test_focus_window_skips_a_missing_window(self):
        # No window is handle 0, and focusing it is never tried.
        input_driver.focus_window(0, focus=lambda hwnd: self.fail(f"focused {hwnd}"))


if __name__ == "__main__":
    unittest.main()
