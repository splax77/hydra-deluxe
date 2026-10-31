"""Tests for menu_tools: ch_open_song's steps, waits, caps and refusals, and
the checked-in menu_steps.json.

No game and no Frida: the game side is a fake that records each press, the
Frida side a fake whose read is a function of the presses so far and the
reads so far, and the clock a fake that sleeping moves.
"""

from __future__ import annotations

import json
import os
import re
import sys
import tempfile
import threading
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_mcp import menu_tools as MT  # noqa: E402
from tools.ch_mcp.frida_tools import StateUnread  # noqa: E402
from tools.ch_mcp.game_tools import PressResult  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402
from tools.ch_probe import probe_songs  # noqa: E402

BACKUP = "backup"
SONG_NAME = probe_songs.full_name(probe_songs.WINDOW_MAP)


def state(*, song_select=0, player=0, selected=None, found=False, loaded=False,
          online=(False,)):
    """One read in state.ts's shape, with menu.ts's menu block."""
    return {
        "found": found,
        "songLoaded": loaded,
        "menu": {
            "classes": {
                "MainMenu": {"found": True, "live": 1, "destroyed": 0,
                             "onlineMenuActive": list(online)},
                "SongSelect": {"found": True, "live": song_select, "destroyed": 0,
                               "onlineMenuActive": [False] * song_select},
                "PlayerSelectionMenu": {"found": True, "live": player, "destroyed": 0},
            },
            "selectedSong": None if selected is None else {"folderPath": selected,
                                                           "chartName": "x"},
            "globals": {"found": False, "reason": "no live GlobalVariables.instance"},
        },
    }


class FakeGame:
    """GameTools' three methods MenuTools calls. `events` holds the backup and
    every item pressed, in order. An item in `refuse` is not sent: the call
    returns the refusal GameTools would give, with sent False."""

    def __init__(self, refuse=()):
        self.events = []
        self.refuse = set(refuse)

    def ensure_backup(self):
        self.events.append(BACKUP)
        return "backup folder: X"

    def press_keys(self, items):
        refused = [i for i in items if i in self.refuse]
        if refused:
            return PressResult(False, f'unknown key "{refused[0]}"; valid keys: enter, ...')
        self.events.extend(items)
        return PressResult(True, "keys sent: " + ", ".join(items))

    def press_lanes(self, lanes):
        if MT.LANE_PREFIX + lanes in self.refuse:
            return PressResult(False, "no game window found (Clone Hero is not running)")
        self.events.append(MT.LANE_PREFIX + lanes)
        return PressResult(True, f"pressed {lanes}")

    def keys(self):
        return [e for e in self.events if e != BACKUP]


class FakeFrida:
    """read_state as `script(keys_pressed, reads_so_far)`; None raises
    StateUnread."""

    def __init__(self, game, script):
        self.game = game
        self.script = script
        self.reads = 0

    def read_state(self):
        result = self.script(len(self.game.keys()), self.reads)
        self.reads += 1
        if result is None:
            raise StateUnread("state: not attached; not attached; call ch_attach first")
        return result


class FakeClock:
    def __init__(self):
        self.t = 0.0
        self.slept = []

    def now(self):
        return self.t

    def sleep(self, s):
        self.slept.append(s)
        self.t += s


class MenuToolsCase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = tmp.name
        self.folder = os.path.join(self.root, probe_songs.WINDOW_MAP)
        os.makedirs(self.folder)
        with open(os.path.join(self.folder, game_files.SONG_INI), "w", encoding="utf-8") as f:
            f.write(f"[song]\nname = {SONG_NAME}\n")
        self.other = os.path.join(self.root, "other")
        self.game = FakeGame()
        self.clock = FakeClock()

    def steps_file(self, steps):
        path = os.path.join(self.root, "steps.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump({"verified": False, "steps": steps}, f)
        return path

    def tools(self, script, steps=None):
        path = MT.STEPS_FILE if steps is None else self.steps_file(steps)
        self.frida = FakeFrida(self.game, script)
        return MT.MenuTools(self.game, self.frida, threading.RLock(),
                            now=self.clock.now, sleep=self.clock.sleep, steps_path=path)


# A short file in the checked-in file's shape: three single-key steps, the
# search, and a wait.
STEPS = [
    {"name": "song list", "keys": ["enter"], "expect": "song_select_live"},
    {"name": "search", "keys": ["lane:blue", "text:{name}"],
     "expect": "selected_folder_is_target"},
    {"name": "confirm", "keys": ["lane:green"], "expect": "player_screen_live"},
    {"name": "ready", "keys": ["lane:green"], "expect": "game_manager_live"},
    {"name": "loads", "keys": [], "expect": "song_loaded"},
]


class OpenSongTest(MenuToolsCase):
    def screens(self, target_after_downs=0, never_target=False):
        """The game's reads for STEPS: the song list after Enter; the typed
        name selects the first result, each Down the next; the target is
        result `target_after_downs`; Green opens the player screen, Green
        again loads the song, which reads loaded on the read after."""
        folder = self.folder
        other = self.other

        def script(keys, reads):
            if keys == 0:
                return state()
            if keys < 3:  # enter, lane:blue
                return state(song_select=1, selected=other + "0")
            downs = self.game.keys().count(MT.SEARCH_NEXT_KEY)
            if keys == 3 + downs:  # the typed text, then the Downs
                hit = not never_target and downs == target_after_downs
                return state(song_select=1, selected=folder if hit else f"{other}{downs + 1}")
            if keys == 4 + downs:
                return state(song_select=1, player=1, selected=folder)
            return state(found=True, loaded=self.frida.reads > self.loaded_after)
        self.loaded_after = 10 ** 9
        return script

    def test_a_full_run_prints_one_line_per_step(self):
        script = self.screens()
        tools = self.tools(script, STEPS)
        self.loaded_after = 0
        reply = tools.ch_open_song(self.folder)
        for i, step in enumerate(STEPS, start=1):
            self.assertIn(f"step {i} ({step['name']}): ", reply)
        self.assertIn(f"text:{SONG_NAME}", reply)
        self.assertTrue(reply.endswith(f"done: {self.folder} is loaded"), reply)
        self.assertEqual(self.game.keys(), ["enter", "lane:blue", f"text:{SONG_NAME}",
                                            "lane:green", "lane:green"])

    def test_backup_comes_before_the_first_key(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        tools.ch_open_song(self.folder)
        self.assertEqual(self.game.events[0], BACKUP)
        self.assertEqual(self.game.events.count(BACKUP), 1)

    def test_a_read_that_never_changes_stops_at_the_cap_naming_the_step(self):
        tools = self.tools(lambda keys, reads: state(), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (song list): song_select_live did not hold within "
                      f"{MT.MENU_STEP_CAP_S:g} s (MENU_STEP_CAP_S)", reply)
        self.assertIn("keys sent so far: enter", reply)
        self.assertIn("last read:", reply)
        self.assertIn("  SongSelect: 0 live, 0 destroyed", reply)
        self.assertEqual(self.game.keys(), ["enter"])
        self.assertGreaterEqual(self.clock.t, MT.MENU_STEP_CAP_S)

    def test_online_open_on_the_first_read_refuses_before_any_key(self):
        tools = self.tools(lambda keys, reads: state(online=(True,)), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (song list): refused before its keys: the Online "
                      "menu is on screen", reply)
        self.assertIn("keys sent so far: none", reply)
        self.assertEqual(self.game.events, [])

    def test_an_error_entry_refuses_before_any_key(self):
        tools = self.tools(lambda keys, reads: state(online=("error: boom",)), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn('refused before its keys: MainMenu object 0\'s Online read is '
                      '"error: boom", so the read cannot show the Online menu is closed', reply)
        self.assertEqual(self.game.events, [])

    def test_a_null_entry_is_not_a_refusal(self):
        self.assertIsNone(MT.online_refusal(state(online=(None, False))))

    def test_a_failed_menu_block_refuses(self):
        broken = {"found": False, "menu": {"error": "reading the menus failed: x"}}
        self.assertIn("the menu read failed", MT.online_refusal(broken))
        self.assertIn("no menu block", MT.online_refusal({"found": False}))

    def test_online_open_before_step_3_stops_there_with_the_earlier_keys(self):
        inner = self.screens()
        after_text = []

        def script(keys, reads):
            read = inner(keys, reads)
            if keys == 3:  # after the search's text, the target selected
                after_text.append(reads)
                # The first such read ends the search's wait; the second is
                # the read before step 3.
                if len(after_text) >= 2:
                    read["menu"]["classes"]["MainMenu"]["onlineMenuActive"] = [True]
            return read
        tools = self.tools(script, STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 3 (confirm): refused before its keys: the Online menu "
                      "is on screen", reply)
        self.assertIn(f"keys sent so far: enter, lane:blue, text:{SONG_NAME}", reply)
        self.assertNotIn("lane:green", self.game.keys())

    def test_a_search_that_needs_two_downs_presses_exactly_two(self):
        tools = self.tools(self.screens(target_after_downs=2), STEPS)
        self.loaded_after = 0
        reply = tools.ch_open_song(self.folder)
        self.assertEqual(self.game.keys().count(MT.SEARCH_NEXT_KEY), 2)
        self.assertIn("step 2 (search): 2 Down presses to reach the target", reply)
        self.assertIn("done:", reply)

    def test_a_search_that_never_finds_the_target_stops_at_the_down_cap(self):
        tools = self.tools(self.screens(never_target=True), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertEqual(self.game.keys().count(MT.SEARCH_NEXT_KEY), MT.SEARCH_DOWN_CAP)
        self.assertIn(f"stopped at step 2 (search): the target was not selected after "
                      f"{MT.SEARCH_DOWN_CAP} Down presses (SEARCH_DOWN_CAP)", reply)

    def test_unattached_refuses_before_any_key(self):
        tools = self.tools(lambda keys, reads: None, STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (song list): no read came back: state: not attached",
                      reply)
        self.assertIn("last read: none", reply)
        self.assertEqual(self.game.events, [])

    def test_a_folder_with_no_song_name_refuses(self):
        tools = self.tools(lambda keys, reads: state(), STEPS)
        reply = tools.ch_open_song(self.other)
        self.assertTrue(reply.startswith("refused:"), reply)
        self.assertEqual(self.game.events, [])
        self.assertEqual(self.frida.reads, 0)

    def test_a_step_that_presses_space_refuses_the_file(self):
        tools = self.tools(lambda keys, reads: state(),
                           [{"name": "mapper", "keys": ["space"], "expect": "song_select_live"}])
        reply = tools.ch_open_song(self.folder)
        self.assertIn("presses space, which no step may press (FORBIDDEN_KEYS)", reply)
        self.assertEqual(self.game.events, [])

    def test_a_refused_key_press_stops_at_once_naming_the_refusal(self):
        self.game.refuse.add("enter")
        tools = self.tools(lambda keys, reads: state(), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (song list): the game refused the press of enter, "
                      'so nothing was sent: unknown key "enter"', reply)
        self.assertIn("keys sent so far: none", reply)
        self.assertIn("last read:\n  MainMenu:", reply)
        self.assertEqual(self.frida.reads, 1)
        self.assertLess(self.clock.t, MT.MENU_STEP_CAP_S)

    def test_a_refused_lane_press_stops_at_once_naming_the_refusal(self):
        self.game.refuse.add(MT.LANE_PREFIX + "blue")
        tools = self.tools(self.screens(), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 2 (search): the game refused the press of blue, so "
                      "nothing was sent: no game window found", reply)
        self.assertIn("keys sent so far: enter", reply)
        self.assertNotIn(f"text:{SONG_NAME}", self.game.keys())

    def test_a_refused_down_press_in_the_search_stops_at_once(self):
        self.game.refuse.add(MT.SEARCH_NEXT_KEY)
        tools = self.tools(self.screens(target_after_downs=2), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 2 (search): the game refused the press of down", reply)
        self.assertEqual(self.game.keys().count(MT.SEARCH_NEXT_KEY), 0)

    def test_presses_after_the_first_wait_the_key_gap(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        tools.ch_open_song(self.folder)
        # Five press calls: enter; lane:blue; the text; lane:green; lane:green.
        self.assertEqual(self.clock.slept.count(input_driver.MENU_KEY_GAP_S), 4)

    def test_same_folder_ignores_case_slashes_and_a_trailing_slash(self):
        self.assertTrue(MT.same_folder(self.folder, self.folder.upper().replace("\\", "/") + "/"))
        self.assertFalse(MT.same_folder(self.folder, self.other))
        self.assertFalse(MT.same_folder(None, self.folder))


class NumbersTest(unittest.TestCase):
    def test_the_three_numbers_are_ruling_3s(self):
        self.assertEqual(MT.MENU_STEP_CAP_S, 10)
        self.assertEqual(MT.SEARCH_DOWN_CAP, 10)
        self.assertEqual(MT.MENU_POLL_S, 0.5)

    def test_the_key_gap_is_input_drivers_not_defined_here(self):
        self.assertIs(MT.MENU_KEY_GAP_S, input_driver.MENU_KEY_GAP_S)
        with open(MT.__file__, encoding="utf-8") as f:
            text = f.read()
        self.assertIsNone(re.search(r"^\s*MENU_KEY_GAP_S\s*=", text, re.M))


class StepsFileTest(unittest.TestCase):
    def setUp(self):
        self.data = MT.load_steps()
        self.steps = self.data["steps"]

    def test_it_loads_and_starts_unverified(self):
        self.assertIs(self.data["verified"], False)

    def test_it_names_the_two_manual_pages(self):
        self.assertEqual(len(self.data["sources"]), 2)
        for url in self.data["sources"]:
            self.assertTrue(url.startswith("https://wiki.clonehero.net/"), url)

    def test_no_step_presses_space(self):
        for step in self.steps:
            for key in step["keys"]:
                self.assertNotIn(key.lower(), MT.FORBIDDEN_KEYS, step["name"])

    def test_every_key_item_passes_the_item_check(self):
        for step in self.steps:
            for key in step["keys"]:
                self.assertIsNone(MT.key_item_problem(key), key)

    def test_the_item_check_refuses_what_a_step_may_not_list(self):
        for bad in ("f1", "lane:purple", "lane:green,kick", "lane:", "{name}",
                    "lane:{name}", "Enter,space"):
            self.assertIsNotNone(MT.key_item_problem(bad), bad)
        for good in ("enter", "Down", "lane:Green", "lane: kick", "text:{name}", "text:x,y"):
            self.assertIsNone(MT.key_item_problem(good), good)

    def test_a_file_with_an_unpressable_item_is_refused_whole(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "steps.json")
            for bad in ("lane:green,kick", "f1"):
                with open(path, "w", encoding="utf-8") as f:
                    json.dump({"steps": [{"name": "x", "keys": [bad],
                                          "expect": "song_loaded"}]}, f)
                with self.assertRaises(MT.StepsInvalid) as ctx:
                    MT.load_steps(path)
                self.assertIn("lists a key that cannot be pressed", str(ctx.exception))

    def test_the_object_reads_are_marked_unproven(self):
        for step in self.steps:
            if step["expect"] in ("song_select_live", "player_screen_live"):
                self.assertIs(step.get("unproven"), True, step["name"])

    def test_it_types_the_song_name_and_ends_on_song_loaded(self):
        typed = [k for s in self.steps for k in s["keys"] if MT.NAME_FIELD in k]
        self.assertEqual(len(typed), 1)
        self.assertEqual(self.steps[-1]["expect"], "song_loaded")
        self.assertIn(MT.SEARCH_EXPECT, [s["expect"] for s in self.steps])


if __name__ == "__main__":
    unittest.main()
