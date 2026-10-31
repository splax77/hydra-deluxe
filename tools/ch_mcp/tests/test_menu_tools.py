"""Tests for menu_tools: ch_open_song's kit presses, waits, caps and refusals,
the expect words, and the checked-in menu_steps.json.

No game and no Frida: the game side is a fake that records the backup, the
kit a fake that records each press, the Frida side a fake whose read is a
function of the presses so far and the reads so far, and the clock a fake
that sleeping moves.
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

from tools.ch_mcp import menu_tools as MT  # noqa: E402
from tools.ch_mcp.frida_tools import StateUnread  # noqa: E402
from tools.ch_mcp.game_tools import PressResult  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402

BACKUP = "backup"
MENUS = ("PlayerProfileMenu", "MainMenu", "SongSelect", "PlayerSelectionMenu")
ONLINE_HOLDERS = ("MainMenu", "SongSelect")


def state(*, screen=None, selected=None, found=False, loaded=False, online=(False,),
          current=None, live=1):
    """One read in state.ts's shape, with menu.ts's menu block: every menu
    class has `live` live instances, and only `screen`'s reads active true."""
    classes = {}
    for name in MENUS:
        entry = {"found": True, "live": live, "destroyed": 0,
                 "active": [name == screen] * live}
        if name in ONLINE_HOLDERS:
            entry["onlineMenuActive"] = list(online) if name == "MainMenu" else [False] * live
        classes[name] = entry
    return {
        "found": found,
        "songLoaded": loaded,
        "menu": {
            "classes": classes,
            "selectedSong": None if selected is None else {"folderPath": selected,
                                                           "chartName": "x"},
            "globals": {"found": True, "currentSongFolder": current, "gameMode": 0,
                        "playerCount": 1},
        },
    }


class FakeGame:
    """GameTools' one method MenuTools calls. `events` is shared with the
    kit, so it holds the backup and every press, in order."""

    def __init__(self, events):
        self.events = events

    def ensure_backup(self):
        self.events.append(BACKUP)
        return "backup folder: X"


class FakeKit:
    """KitTools.press_items. An item in `refuse` is not pressed: the call
    returns the refusal KitTools would give, with sent False."""

    def __init__(self, events, refuse=()):
        self.events = events
        self.refuse = set(refuse)
        self.calls = []

    def press_items(self, items, hold_frames=None):
        self.calls.append(list(items))
        refused = [i for i in items if i in self.refuse]
        if refused:
            return PressResult(False, "the kit is not set up; call ch_kit_setup first")
        self.events.extend(items)
        return PressResult(True, f"pressed {', '.join(items)} for 2 frames")

    def presses(self):
        return [e for e in self.events if e != BACKUP]


class FakeFrida:
    """read_state as `script(presses_so_far, reads_so_far)`; None raises
    StateUnread."""

    def __init__(self, kit, script):
        self.kit = kit
        self.script = script
        self.reads = 0

    def read_state(self):
        result = self.script(len(self.kit.presses()), self.reads)
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
        self.folder = os.path.join(self.root, "target")
        self.other = os.path.join(self.root, "other")
        self.events = []
        self.game = FakeGame(self.events)
        self.kit = FakeKit(self.events)
        self.clock = FakeClock()

    def steps_file(self, steps):
        path = os.path.join(self.root, "steps.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump({"verified": False, "steps": steps}, f)
        return path

    def tools(self, script, steps=None, kit=True):
        path = MT.STEPS_FILE if steps is None else self.steps_file(steps)
        self.frida = FakeFrida(self.kit, script)
        return MT.MenuTools(self.game, self.frida, threading.RLock(),
                            kit=self.kit if kit else None,
                            now=self.clock.now, sleep=self.clock.sleep, steps_path=path)


# A file in the checked-in file's shape, plus a search step between the song
# list and the player screen.
STEPS = [
    {"name": "profile screen", "items": ["kit:start"], "expect": "profile_screen_active"},
    {"name": "pick the profile", "items": ["kit:green"], "expect": "main_menu_active"},
    {"name": "song list", "items": ["kit:green"], "expect": "song_select_active"},
    {"name": "search", "items": [], "expect": "selected_folder_is_target"},
    {"name": "confirm", "items": ["kit:green"], "expect": "player_screen_active"},
    {"name": "ready", "items": ["kit:green"], "expect": "game_manager_live"},
    {"name": "loads", "items": [], "expect": "song_loaded"},
]


class OpenSongTest(MenuToolsCase):
    def screens(self, target_after_scrolls=0, never_target=False, loads=None):
        """The game's reads for STEPS: Start shows the profile screen, Green
        the main menu, Green the song list; the selected song is result
        `target_after_scrolls`, each scroll press the next; Green opens the
        player screen, Green again loads the song `loads` (the target when
        None), which reads loaded on the read after."""
        folder = self.folder
        other = self.other
        loads = folder if loads is None else loads

        def script(presses, reads):
            if presses < 3:
                return state(screen=(None, "PlayerProfileMenu", "MainMenu")[presses])
            scrolls = self.kit.presses().count(MT.SEARCH_NEXT_ITEM)
            if presses == 3 + scrolls:
                hit = not never_target and scrolls == target_after_scrolls
                return state(screen="SongSelect", selected=folder if hit else f"{other}{scrolls}")
            if presses == 4 + scrolls:
                return state(screen="PlayerSelectionMenu", selected=folder)
            return state(found=True, loaded=self.frida.reads > self.loaded_after, current=loads)
        self.loaded_after = 10 ** 9
        return script

    def test_a_full_run_prints_one_line_per_step(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        reply = tools.ch_open_song(self.folder)
        for i, step in enumerate(STEPS, start=1):
            self.assertIn(f"step {i} ({step['name']}): ", reply)
        self.assertIn("  pressed kit:start for 2 frames", reply)
        self.assertTrue(reply.endswith(f"done: {self.folder} is loaded"), reply)
        self.assertEqual(self.kit.presses(), ["kit:start", "kit:green", "kit:green",
                                              "kit:green", "kit:green"])
        # One kit press per item.
        self.assertEqual(self.kit.calls, [[p] for p in self.kit.presses()])

    def test_the_checked_in_walk_reaches_song_loaded_pressing_only_kit_items(self):
        def script(presses, reads):
            if presses < 4:
                return state(screen=(None, "PlayerProfileMenu", "MainMenu", "SongSelect")[presses],
                             selected=self.other)
            if presses == 4:
                return state(screen="PlayerSelectionMenu", selected=self.other)
            return state(found=True, loaded=True, current=self.folder)
        reply = self.tools(script).ch_open_song(self.folder)
        self.assertTrue(reply.endswith(f"done: {self.folder} is loaded"), reply)
        self.assertEqual(self.events[0], BACKUP)
        self.assertTrue(self.kit.presses())
        for item in self.kit.presses():
            self.assertIsNone(MT.item_problem(item), item)

    def test_backup_comes_before_the_first_press(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        tools.ch_open_song(self.folder)
        self.assertEqual(self.events[0], BACKUP)
        self.assertEqual(self.events.count(BACKUP), 1)

    def test_another_song_loaded_is_named_in_the_last_line(self):
        tools = self.tools(self.screens(loads=self.other), STEPS)
        self.loaded_after = 0
        reply = tools.ch_open_song(self.folder)
        self.assertTrue(reply.endswith(
            f"done: a song is loaded, but its folder is {json.dumps(self.other)}, not the "
            f"target {self.folder}"), reply)

    def test_a_read_that_never_changes_stops_at_the_cap_naming_the_step(self):
        tools = self.tools(lambda presses, reads: state(), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (profile screen): profile_screen_active did not hold "
                      f"within {MT.MENU_STEP_CAP_S:g} s (MENU_STEP_CAP_S)", reply)
        self.assertIn("kit items pressed so far: kit:start", reply)
        self.assertIn("last read:", reply)
        self.assertIn("  PlayerProfileMenu: 1 live, 0 destroyed; active: [false]", reply)
        self.assertEqual(self.kit.presses(), ["kit:start"])
        self.assertGreaterEqual(self.clock.t, MT.MENU_STEP_CAP_S)

    def test_online_open_on_the_first_read_refuses_before_any_press(self):
        tools = self.tools(lambda presses, reads: state(online=(True,)), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (profile screen): refused before its presses: the "
                      "Online menu is on screen", reply)
        self.assertIn("kit items pressed so far: none", reply)
        self.assertEqual(self.events, [])

    def test_online_open_before_step_2_stops_there_with_the_earlier_press(self):
        inner = self.screens()
        after_start = []

        def script(presses, reads):
            read = inner(presses, reads)
            if presses == 1:
                after_start.append(reads)
                # The first such read ends step 1's wait; the second is the
                # read before step 2.
                if len(after_start) >= 2:
                    read["menu"]["classes"]["MainMenu"]["onlineMenuActive"] = [True]
            return read
        tools = self.tools(script, STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 2 (pick the profile): refused before its presses: the "
                      "Online menu is on screen", reply)
        self.assertIn("kit items pressed so far: kit:start\n", reply)
        self.assertEqual(self.kit.presses(), ["kit:start"])

    def test_an_error_entry_refuses_before_any_press(self):
        tools = self.tools(lambda presses, reads: state(online=("error: boom",)), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn('refused before its presses: MainMenu object 0\'s Online read is '
                      '"error: boom", so the read cannot show the Online menu is closed', reply)
        self.assertEqual(self.events, [])

    def test_a_null_entry_is_not_a_refusal(self):
        self.assertIsNone(MT.online_refusal(state(online=(None, False))))

    def test_a_failed_menu_block_refuses(self):
        broken = {"found": False, "menu": {"error": "reading the menus failed: x"}}
        self.assertIn("the menu read failed", MT.online_refusal(broken))
        self.assertIn("no menu block", MT.online_refusal({"found": False}))
        self.assertIn("no classes", MT.online_refusal({"menu": {"selectedSong": None}}))
        odd = state()
        odd["menu"]["classes"]["MainMenu"]["onlineMenuActive"] = True
        self.assertIn("not a list", MT.online_refusal(odd))

    def test_a_search_that_needs_two_scrolls_presses_exactly_two(self):
        tools = self.tools(self.screens(target_after_scrolls=2), STEPS)
        self.loaded_after = 0
        reply = tools.ch_open_song(self.folder)
        self.assertEqual(self.kit.presses().count(MT.SEARCH_NEXT_ITEM), 2)
        self.assertIn(f"step 4 (search): 2 {MT.SEARCH_NEXT_ITEM} presses to reach the target",
                      reply)
        self.assertIn("done:", reply)

    def test_a_search_that_never_finds_the_target_stops_at_the_scroll_cap(self):
        tools = self.tools(self.screens(never_target=True), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertEqual(self.kit.presses().count(MT.SEARCH_NEXT_ITEM), MT.SEARCH_DOWN_CAP)
        self.assertIn(f"stopped at step 4 (search): the target was not selected after "
                      f"{MT.SEARCH_DOWN_CAP} {MT.SEARCH_NEXT_ITEM} presses (SEARCH_DOWN_CAP)",
                      reply)

    def test_unattached_refuses_before_any_press(self):
        tools = self.tools(lambda presses, reads: None, STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (profile screen): no read came back: state: not "
                      "attached", reply)
        self.assertIn("last read: none", reply)
        self.assertEqual(self.events, [])

    def test_no_kit_refuses_before_any_read(self):
        tools = self.tools(lambda presses, reads: state(), STEPS, kit=False)
        self.assertEqual(tools.ch_open_song(self.folder), MT.NO_KIT)
        self.assertEqual(self.events, [])
        self.assertEqual(self.frida.reads, 0)

    def test_a_refused_kit_press_stops_at_once_naming_the_refusal(self):
        self.kit.refuse.add("kit:start")
        tools = self.tools(lambda presses, reads: state(), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn("stopped at step 1 (profile screen): the kit refused the press of "
                      "kit:start, so nothing was pressed: the kit is not set up", reply)
        self.assertIn("kit items pressed so far: none", reply)
        self.assertIn("last read:\n  PlayerProfileMenu:", reply)
        self.assertEqual(self.frida.reads, 1)
        self.assertLess(self.clock.t, MT.MENU_STEP_CAP_S)

    def test_a_refused_scroll_press_in_the_search_stops_at_once(self):
        self.kit.refuse.add(MT.SEARCH_NEXT_ITEM)
        tools = self.tools(self.screens(target_after_scrolls=2), STEPS)
        reply = tools.ch_open_song(self.folder)
        self.assertIn(f"stopped at step 4 (search): the kit refused the press of "
                      f"{MT.SEARCH_NEXT_ITEM}", reply)
        self.assertEqual(self.kit.presses().count(MT.SEARCH_NEXT_ITEM), 0)

    def test_presses_after_the_first_wait_the_key_gap(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        tools.ch_open_song(self.folder)
        # Five presses: start, then green four times.
        self.assertEqual(self.clock.slept.count(input_driver.MENU_KEY_GAP_S), 4)

    def test_same_folder_ignores_case_slashes_and_a_trailing_slash(self):
        self.assertTrue(MT.same_folder(self.folder, self.folder.upper().replace("\\", "/") + "/"))
        self.assertFalse(MT.same_folder(self.folder, self.other))
        self.assertFalse(MT.same_folder(None, self.folder))


class ExpectWordsTest(unittest.TestCase):
    WORDS = {"profile_screen_active": "PlayerProfileMenu",
             "main_menu_active": "MainMenu",
             "song_select_active": "SongSelect",
             "player_screen_active": "PlayerSelectionMenu"}

    def test_each_active_word_holds_when_its_menu_reads_active_true(self):
        for word, name in self.WORDS.items():
            with self.subTest(word=word):
                self.assertTrue(MT.EXPECT[word](state(screen=name), "t"))
                for other in MENUS:
                    if other != name:
                        self.assertFalse(MT.EXPECT[word](state(screen=other), "t"))

    def test_one_active_instance_among_several_is_enough(self):
        read = state()
        read["menu"]["classes"]["SongSelect"]["active"] = [False, None, True]
        self.assertTrue(MT.EXPECT["song_select_active"](read, "t"))

    def test_only_true_counts_as_active(self):
        for reads in ([False], [None], ["error: x"], [], [1], "true"):
            read = state()
            read["menu"]["classes"]["MainMenu"]["active"] = reads
            with self.subTest(reads=reads):
                self.assertFalse(MT.EXPECT["main_menu_active"](read, "t"))
        missing = state(screen="MainMenu")
        del missing["menu"]["classes"]["MainMenu"]["active"]
        self.assertFalse(MT.EXPECT["main_menu_active"](missing, "t"))
        unfound = state(screen="MainMenu")
        unfound["menu"]["classes"]["MainMenu"]["found"] = False
        self.assertFalse(MT.EXPECT["main_menu_active"](unfound, "t"))
        self.assertFalse(MT.EXPECT["main_menu_active"]({"found": False}, "t"))

    def test_the_old_words_stay(self):
        for word in ("song_select_live", "selected_folder_is_target", "player_screen_live",
                     "game_manager_live", "song_loaded"):
            self.assertIn(word, MT.EXPECT)
        self.assertTrue(MT.EXPECT["song_select_live"](state(), "t"))
        self.assertFalse(MT.EXPECT["song_select_live"](state(live=0), "t"))


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

    def test_it_names_the_two_manual_pages_and_the_plan(self):
        wiki = [s for s in self.data["sources"] if s.startswith("https://wiki.clonehero.net/")]
        self.assertEqual(len(wiki), 2)
        self.assertIn("docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md",
                      self.data["sources"])

    def test_every_item_is_a_kit_item(self):
        for step in self.steps:
            for item in step["items"]:
                self.assertIsNone(MT.item_problem(item), item)

    def test_the_item_check_refuses_what_a_step_may_not_list(self):
        for bad in ("enter", "space", "down", "lane:green", "text:{name}", "text:x",
                    "kit:purple", "kit:green,kick", "kit:", "green"):
            self.assertIsNotNone(MT.item_problem(bad), bad)
        for good in ("kit:green", "kit:Start", "KIT: kick", "kit:yellow_cymbal"):
            self.assertIsNone(MT.item_problem(good), good)

    def test_a_lane_or_key_item_says_the_kit_route_presses_no_keys(self):
        for bad in ("lane:green", "enter", "text:x"):
            self.assertIn("the kit route presses no keys", MT.item_problem(bad), bad)

    def test_a_file_with_a_lane_or_key_item_is_refused_whole(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "steps.json")
            for bad in ("lane:green", "enter"):
                with open(path, "w", encoding="utf-8") as f:
                    json.dump({"steps": [
                        {"name": "ok", "items": ["kit:start"], "expect": "song_loaded"},
                        {"name": "x", "items": [bad], "expect": "song_loaded"}]}, f)
                with self.subTest(item=bad), self.assertRaises(MT.StepsInvalid) as ctx:
                    MT.load_steps(path)
                self.assertIn("step 2 (x) lists an item that cannot be pressed", str(ctx.exception))
                self.assertIn("the kit route presses no keys", str(ctx.exception))

    def test_a_file_in_the_old_keys_shape_is_refused(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "steps.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump({"steps": [{"name": "x", "keys": ["kit:start"],
                                      "expect": "song_loaded"}]}, f)
            with self.assertRaises(MT.StepsInvalid) as ctx:
                MT.load_steps(path)
            self.assertIn("has no items list", str(ctx.exception))

    def test_a_refused_file_presses_nothing(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = os.path.join(tmp, "steps.json")
            with open(path, "w", encoding="utf-8") as f:
                json.dump({"steps": [{"name": "x", "items": ["lane:green"],
                                      "expect": "song_loaded"}]}, f)
            events = []
            kit = FakeKit(events)
            frida = FakeFrida(kit, lambda presses, reads: state())
            tools = MT.MenuTools(FakeGame(events), frida, threading.RLock(), kit=kit,
                                 steps_path=path)
            reply = tools.ch_open_song("C:\\Songs\\x")
            self.assertTrue(reply.startswith("refused:"), reply)
            self.assertTrue(reply.endswith("; nothing pressed"), reply)
            self.assertEqual(events, [])
            self.assertEqual(frida.reads, 0)

    def test_the_object_reads_are_marked_unproven(self):
        for step in self.steps:
            if step["expect"].endswith(("_live", "_active")) and step["expect"] != "game_manager_live":
                self.assertIs(step.get("unproven"), True, step["name"])

    def test_it_ends_on_song_loaded(self):
        self.assertEqual(self.steps[-1]["expect"], "song_loaded")


if __name__ == "__main__":
    unittest.main()
