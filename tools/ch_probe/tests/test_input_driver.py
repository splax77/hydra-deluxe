"""Unit tests for the input driver's pure logic.

These run with no game and no debugger. They test the binding map (set then
get), the key table, and that tap and press_chord send the right key events.

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

# Make the repo root importable so `tools.ch_probe...` resolves regardless of
# where the test runner is launched from.
_REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

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


class TestBindings(unittest.TestCase):
    def test_defaults_are_present(self):
        driver = InputDriver()
        # Every lane in the shipped placeholder map is readable.
        for lane, vk in DEFAULT_BINDINGS.items():
            self.assertEqual(driver.get_binding(lane), vk)

    def test_set_binding_round_trips(self):
        driver = InputDriver()
        driver.set_binding(0, 0x51)  # 'Q'
        self.assertEqual(driver.get_binding(0), 0x51)

    def test_set_binding_adds_new_lane(self):
        driver = InputDriver(bindings={})
        driver.set_binding(7, 0x42)
        self.assertEqual(driver.get_binding(7), 0x42)

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


class TestTap(unittest.TestCase):
    def test_tap_sends_down_then_up(self):
        driver, presses = _make_driver()
        vk = driver.get_binding(0)
        driver.tap(0)
        self.assertEqual(presses, [(vk, False), (vk, True)])


class TestKeyTable(unittest.TestCase):
    """One key table. A lane number means the same key everywhere."""

    def test_lanes_follow_the_bind_screen(self):
        keys = {lane: chr(DEFAULT_BINDINGS[lane]) for lane in Lane}
        self.assertEqual(keys, {
            Lane.GREEN: "A", Lane.RED: "S", Lane.YELLOW: "J", Lane.BLUE: "K",
            Lane.KICK: "L", Lane.YELLOW_CYMBAL: "U", Lane.BLUE_CYMBAL: "Y",
            Lane.GREEN_CYMBAL: "T"})

    def test_lane_numbers_are_unchanged(self):
        self.assertEqual([int(lane) for lane in Lane], list(range(8)))
        self.assertEqual(Lane.KICK, 4)

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

    def test_key_hold_is_the_recorded_value(self):
        # D54 records the 3 ms hold as it is.
        self.assertEqual(input_driver.KEY_HOLD_S, 0.003)

    def test_unbound_lane_is_skipped(self):
        driver = InputDriver(bindings={Lane.KICK: 0x4C})
        presses = []
        driver.send_key = lambda vk, key_up: presses.append((vk, key_up))
        driver.press_chord([Lane.GREEN, Lane.KICK], sleep=lambda s: None)
        self.assertEqual(presses, [(0x4C, False), (0x4C, True)])


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
