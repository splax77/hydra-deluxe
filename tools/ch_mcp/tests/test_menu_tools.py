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
from tools.ch_mcp.tests.test_frida_attach import recorded  # noqa: E402
from tools.ch_probe import input_driver  # noqa: E402

BACKUP = "backup"
MENUS = ("PlayerProfileMenu", "MainMenu", "SongSelect", "PlayerSelectionMenu")
ONLINE_HOLDERS = ("MainMenu", "SongSelect")


# The player screen's instrument and difficulty numbers the fake sends by
# default: 6 is Drums and 3 is Expert in menu_fields.json, which is
# bot_run.DEFAULT_PLAYER.
DRUMS, EXPERT = 6, 3


def state(*, screen=None, selected=None, found=False, loaded=False, online=(False,),
          current=None, live=1, player=(DRUMS, EXPERT)):
    """One read in state.ts's shape, with menu.ts's menu block: every menu
    class has `live` live instances, and only `screen`'s reads active true.
    The player screen's fields read the instrument and difficulty numbers
    `player` gives, for every live instance."""
    classes = {}
    for name in MENUS:
        entry = {"found": True, "live": live, "destroyed": 0,
                 "active": [name == screen] * live}
        if name in ONLINE_HOLDERS:
            entry["onlineMenuActive"] = list(online) if name == "MainMenu" else [False] * live
        if name == "PlayerSelectionMenu":
            entry["fields"] = {"instrument": [player[0]] * live,
                               "difficulty": [player[1]] * live}
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
    """read_state returns what `script` gives for the count of presses and the
    count of reads so far; None raises StateUnread."""

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

    def test_backup_comes_before_the_first_press(self):
        tools = self.tools(self.screens(), STEPS)
        self.loaded_after = 0
        tools.ch_open_song(self.folder)
        self.assertEqual(self.events[0], BACKUP)
        self.assertEqual(self.events.count(BACKUP), 1)

    def _player_script(self, player):
        """The checked-in walk, with the player screen reading `player`."""
        def script(presses, reads):
            if presses < 4:
                return state(screen=(None, "PlayerProfileMenu", "MainMenu", "SongSelect")[presses],
                             selected=self.other)
            if presses == 4:
                return state(screen="PlayerSelectionMenu", selected=self.other, player=player)
            return state(found=True, loaded=True, current=self.folder)
        return script

    def test_open_song_refuses_a_player_word_menu_fields_does_not_hold(self):
        tools = self.tools(self._player_script((DRUMS, EXPERT)))
        for asked in ("Drums", "Drums,Brutal", "Drum,Expert"):
            with self.subTest(asked=asked):
                reply = tools.ch_open_song(self.folder, asked)
                self.assertTrue(reply.startswith("refused: player "), reply)
                self.assertTrue(reply.endswith("; nothing pressed"), reply)
        self.assertEqual(self.kit.presses(), [])

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

    def test_same_folder_sees_a_junction_as_its_target(self):
        import _winapi
        os.mkdir(self.folder)
        link = os.path.join(self.root, "link")
        _winapi.CreateJunction(self.folder, link)
        self.addCleanup(os.rmdir, link)
        self.assertTrue(MT.same_folder(link, self.folder))
        self.assertFalse(MT.same_folder(link, self.other))


# The state agent's replies from four real Clone Hero runs, each taken while a
# song played, copied from the logs ch_attach wrote under
# hydra-data/ch-mcp-runs: 2026-11-06/ls-d1b-run1/state-1.jsonl,
# 2026-11-06/ls-d2b-run1/state-4.jsonl, 2026-11-12/ls-g1-run1/state-1.jsonl
# and 2026-11-12/ls-g2-run1/state-2.jsonl. A read taken while a song plays is
# the read a walk ends on.
RECORDED_STATE_LOGS = (("2026-11-06-ls-d1b-run1", "state-1.jsonl"),
                       ("2026-11-06-ls-d2b-run1", "state-4.jsonl"),
                       ("ls-g1-run1", "state-1.jsonl"),
                       ("ls-g2-run1", "state-2.jsonl"))


class RecordedReadTest(unittest.TestCase):
    """ch_open_song's end check and Online check, run on reads from real
    runs."""

    def setUp(self):
        self.reads = [message["payload"] for run, name in RECORDED_STATE_LOGS
                      for message in recorded(run, name) if message.get("type") == "send"]
        if not self.reads:
            self.fail("the recorded state logs hold no read")
        for read in self.reads:
            if not MT.loaded_folder(read):
                self.fail(f"a recorded read names no loaded song: {read}")

    def test_a_walk_that_ends_on_another_songs_folder_is_refused_naming_both(self):
        # D318 item 4: the walk that loads another song ends refused, naming
        # both folders. The target here is the song another real run loaded.
        folders = {MT.loaded_folder(read) for read in self.reads}
        if len(folders) < 2:
            self.fail(f"the recorded reads load only {folders}, so no read ends on another "
                      "song's folder")
        for read in self.reads:
            loaded = MT.loaded_folder(read)
            target = next(f for f in sorted(folders) if not MT.same_folder(f, loaded))
            with self.subTest(loaded=loaded, target=target):
                refusal = MT.MenuTools._end_refusal(read, target, MT.DEFAULT_PLAYER, True)
                self.assertIsNotNone(refusal)
                self.assertTrue(refusal.startswith("refused:"), refusal)
                self.assertIn(json.dumps(loaded), refusal)
                self.assertIn(target, refusal)
                self.assertIsNone(MT.MenuTools._end_refusal(read, loaded, MT.DEFAULT_PLAYER,
                                                            True))

    def test_a_walk_that_never_checked_the_player_screen_is_refused_naming_the_player(self):
        # The right song loaded, but no press was made on the player screen
        # after its check, so the instrument and difficulty went unchecked.
        for read in self.reads:
            loaded = MT.loaded_folder(read)
            for player in (MT.DEFAULT_PLAYER, "ProDrums,Expert"):
                with self.subTest(loaded=loaded, player=player):
                    refusal = MT.MenuTools._end_refusal(read, loaded, player, False)
                    self.assertIsNotNone(refusal)
                    self.assertTrue(refusal.startswith("refused:"), refusal)
                    self.assertIn(player, refusal)
                    self.assertIsNone(MT.MenuTools._end_refusal(read, loaded, player, True))

    def test_a_real_read_with_no_online_object_is_not_a_refusal(self):
        # Each real read sends onlineMenuActive for MainMenu and SongSelect,
        # with no entry while the song plays: nothing shows the Online menu
        # open, so the walk is not refused.
        for read in self.reads:
            classes = read["menu"]["classes"]
            self.assertTrue(any("onlineMenuActive" in entry for entry in classes.values()))
            with self.subTest(loaded=MT.loaded_folder(read)):
                self.assertIsNone(MT.online_refusal(read))


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

    def test_it_names_the_two_manual_pages_and_only_repo_files_that_exist(self):
        # The test names no repo file itself: a plan the file cites leaves the
        # repo when its work is done, and this check then fails until the
        # citation moves with it.
        wiki = [s for s in self.data["sources"] if s.startswith("https://wiki.clonehero.net/")]
        self.assertEqual(len(wiki), 2)
        for source in self.data["sources"]:
            if source in wiki:
                continue
            self.assertTrue(os.path.isfile(os.path.join(_REPO_ROOT, source)), source)

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
