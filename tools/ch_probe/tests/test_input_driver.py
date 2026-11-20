"""Unit tests for the input driver's pure logic.

These run with no game and no debugger. They test the binding map, the key
table against the game's own saved key map, and that press_chord sends the
right key events.

The live seam (the real SendInput) is replaced with a recorder here. What only
a running Clone Hero can exercise -- that SendInput
actually reaches the game window -- is out of scope for these tests and is
commented as LIVE-ONLY in the module.

Run from the repo root:
    python -m pytest tools/ch_probe/tests/test_input_driver.py -q
    (or, if pytest is absent) python -m unittest tools.ch_probe.tests.test_input_driver
"""

from __future__ import annotations

import os
import sys
import unittest
import xml.etree.ElementTree as ET

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

    def test_every_lane_has_a_short_name(self):
        self.assertEqual(set(LANE_NAMES), set(Lane))
        self.assertEqual(LANE_NAMES[Lane.KICK], "Kick")


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


class TestMenuKeys(unittest.TestCase):
    """The non-lane keys and the typing path the menus take."""

    def test_the_menu_key_table(self):
        self.assertEqual(input_driver.MENU_KEYS, {
            "enter": 0x0D, "escape": 0x1B, "space": 0x20, "backspace": 0x08,
            "tab": 0x09, "up": 0x26, "down": 0x28, "left": 0x25, "right": 0x27})

    def test_the_menu_key_gap_is_the_ruled_value(self):
        # Ruling 3 of the 2026-10-30 ch_mcp Star Power plan.
        self.assertEqual(input_driver.MENU_KEY_GAP_S, 0.25)

    def test_press_key_sends_down_hold_then_up(self):
        driver, presses = _make_driver()
        enter = input_driver.MENU_KEYS["enter"]
        sent = driver.press_key(enter, sleep=lambda s: presses.append(("sleep", s)))
        self.assertEqual(presses, [(enter, False), ("sleep", input_driver.KEY_HOLD_S),
                                   (enter, True)])
        self.assertEqual(sent, enter)

    def test_type_text_sends_unicode_events_with_the_gap_between(self):
        driver = InputDriver()
        events = []
        driver.send_input = lambda event: events.append(
            (event.type, event.u.ki.wVk, event.u.ki.wScan, event.u.ki.dwFlags))
        driver.send_key = lambda vk, key_up: self.fail("type_text pressed a virtual key")
        sleeps = []
        typed = driver.type_text("Wi", gap_s=0.25, sleep=sleeps.append)
        down = input_driver.KEYEVENTF_UNICODE
        up = input_driver.KEYEVENTF_UNICODE | input_driver.KEYEVENTF_KEYUP
        self.assertEqual(input_driver.KEYEVENTF_UNICODE, 0x0004)
        kb = input_driver.INPUT_KEYBOARD
        self.assertEqual(events, [(kb, 0, ord("W"), down), (kb, 0, ord("W"), up),
                                  (kb, 0, ord("i"), down), (kb, 0, ord("i"), up)])
        self.assertEqual(sleeps, [0.25])
        self.assertEqual(typed, 2)

    def test_type_text_sends_a_wide_character_as_its_two_halves(self):
        driver = InputDriver()
        units = []
        driver.send_unicode = lambda unit, key_up: units.append((unit, key_up))
        driver.type_text("\U0001F941", gap_s=0.25, sleep=lambda s: None)
        self.assertEqual(units, [(0xD83E, False), (0xD83E, True),
                                 (0xDD41, False), (0xDD41, True)])

    def test_send_key_still_sends_a_virtual_key_event(self):
        driver = InputDriver()
        events = []
        driver.send_input = lambda event: events.append(
            (event.u.ki.wVk, event.u.ki.wScan, event.u.ki.dwFlags))
        driver.tap(Lane.RED)
        red = DEFAULT_BINDINGS[Lane.RED]
        self.assertEqual(events, [(red, 0, 0), (red, 0, input_driver.KEYEVENTF_KEYUP)])


class TestGameWindow(unittest.TestCase):
    """The one way a runner finds and focuses the game window."""

    def test_find_game_window_asks_for_the_one_title(self):
        asked = []

        def finder(cls, title):
            asked.append((cls, title))
            return 0x1234

        self.assertEqual(input_driver.find_game_window(find=finder), 0x1234)
        self.assertEqual(asked, [(None, C.WINDOW_TITLE)])
        self.assertEqual(input_driver.find_game_window(find=lambda cls, title: None), 0)

    def test_focus_window_skips_a_missing_window(self):
        focused = []
        input_driver.focus_window(0, focus=focused.append)
        self.assertEqual(focused, [])
        input_driver.focus_window(0x1234, focus=focused.append)
        self.assertEqual(focused, [0x1234])


if __name__ == "__main__":
    unittest.main()
