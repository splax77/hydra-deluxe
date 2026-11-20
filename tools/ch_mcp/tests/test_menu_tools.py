"""Tests for menu_tools: ch_open_song's kit presses, waits, caps and refusals,
the expect words, and the checked-in menu_steps.json.

No game is running. Where a test can, it drives the server's real game,
Frida and kit tools on a temp folder, never attached and never set up, and
feeds the walk the state agent's reads from real Clone Hero runs. The older
walk tests in OpenSongTest still use the fakes below.
"""

from __future__ import annotations

import json
import os
import re
import sys
import tempfile
import threading
import time
import unittest
import urllib.parse

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import kit_tools as KT  # noqa: E402
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
    def test_no_kit_refuses_before_any_read(self):
        tools = self.tools(lambda presses, reads: state(), STEPS, kit=False)
        self.assertEqual(tools.ch_open_song(self.folder), MT.NO_KIT)
        self.assertEqual(self.events, [])
        self.assertEqual(self.frida.reads, 0)

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


def recorded_reads():
    """Every read the state agent sent in RECORDED_STATE_LOGS, in order."""
    return [message["payload"] for run, name in RECORDED_STATE_LOGS
            for message in recorded(run, name) if message.get("type") == "send"]


class RecordedReplies:
    """Stands where FridaTools.read_state is called and answers with the
    recorded reads in turn, starting over after the last. It keeps each read
    it gave and the time.monotonic() it gave it at, the clock MenuTools waits
    on by default."""

    def __init__(self):
        self.reads = recorded_reads()
        self.served = []
        self.times = []

    def read_state(self):
        read = self.reads[len(self.served) % len(self.reads)]
        self.served.append(read)
        self.times.append(time.monotonic())
        return read


class RecordedReadTest(unittest.TestCase):
    """ch_open_song's end check and Online check, run on reads from real
    runs."""

    def setUp(self):
        self.reads = recorded_reads()
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


class RealToolsTest(unittest.TestCase):
    """ch_open_song on the server's real game, Frida and kit tools, on a temp
    folder with no registry export: Frida is never attached and the kit is
    never set up, which is what a run sees before ch_attach and
    ch_kit_setup. Where a walk needs a read, RecordedReplies gives it the
    reads of real runs. MenuTools keeps its own clock."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.root = tmp.name
        self.scratch = os.path.join(self.root, "scratch")
        self.target = os.path.join(self.root, "target")
        self.lock = threading.RLock()
        self.game = GT.GameTools(bot_run.Env(run_reg=None), self.lock, self.scratch)
        self.frida = FT.FridaTools(self.game, self.lock)
        self.kit = KT.KitTools(self.game, self.frida, self.lock)
        self.first = MT.load_steps()["steps"][0]

    def backups(self):
        """The backup folders GameTools.ensure_backup made under the temp
        scratch folder."""
        return [d for _root, dirs, _files in os.walk(self.scratch) for d in dirs
                if d.startswith(game_files.BACKUP_PREFIX)]

    def steps_file(self, steps):
        path = os.path.join(self.root, "steps.json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump({"verified": False, "steps": steps}, f)
        return path

    def recorded(self):
        replies = RecordedReplies()
        if not replies.reads:
            self.fail("the recorded state logs hold no read")
        return replies

    def test_unattached_refuses_before_any_press(self):
        # The reason is the one the real FridaTools gives before ch_attach.
        with self.assertRaises(StateUnread) as caught:
            self.frida.read_state()
        tools = MT.MenuTools(self.game, self.frida, self.lock, kit=self.kit)
        reply = tools.ch_open_song(self.target)
        self.assertIn(f"stopped at step 1 ({self.first['name']}): no read came back: "
                      f"{caught.exception}", reply)
        self.assertIn("kit items pressed so far: none", reply)
        self.assertIn("last read: none", reply)
        self.assertEqual(self.backups(), [])

    def test_a_refused_kit_press_stops_at_once_naming_the_refusal(self):
        # The checked-in walk's first step presses a kit item; the real kit,
        # never set up, refuses it with its own reason.
        if not self.first["items"]:
            self.fail(f"step 1 ({self.first['name']}) of menu_steps.json presses nothing")
        replies = self.recorded()
        tools = MT.MenuTools(self.game, replies, self.lock, kit=self.kit)
        reply = tools.ch_open_song(self.target)
        self.assertIn(f"stopped at step 1 ({self.first['name']}): the kit refused the press "
                      f"of {self.first['items'][0]}, so nothing was pressed: {KT.NOT_SET_UP}",
                      reply)
        self.assertIn("kit items pressed so far: none", reply)
        last = "\n".join("  " + line for line in FT.menu_lines(replies.served[-1][FT.MENU_BLOCK]))
        self.assertIn("last read:\n" + last, reply)
        # One read, the one before the press: the walk did not wait.
        self.assertEqual(len(replies.served), 1)
        # The backup comes before the first press, refused or not.
        self.assertEqual(len(self.backups()), 1)

    def test_a_refused_scroll_press_in_the_search_stops_at_once(self):
        # A walk of one search step with no presses of its own: no recorded
        # read selects the target, so the search presses SEARCH_NEXT_ITEM,
        # and the real kit, never set up, refuses it.
        replies = self.recorded()
        for read in replies.reads:
            if MT.EXPECT[MT.SEARCH_EXPECT](read, self.target):
                self.fail(f"a recorded read selects the target: {read}")
        path = self.steps_file([{"name": "search", "items": [], "expect": MT.SEARCH_EXPECT}])
        tools = MT.MenuTools(self.game, replies, self.lock, kit=self.kit, steps_path=path)
        reply = tools.ch_open_song(self.target)
        self.assertIn(f"stopped at step 1 (search): the kit refused the press of "
                      f"{MT.SEARCH_NEXT_ITEM}, so nothing was pressed: {KT.NOT_SET_UP}", reply)
        self.assertIn("kit items pressed so far: none", reply)
        self.assertEqual(len(replies.served), 1)

    def test_a_player_word_menu_fields_does_not_hold_is_refused_before_any_read(self):
        # The asked words are built from menu_fields.json's own tables: an
        # instrument with no difficulty, an instrument word where the
        # difficulty goes, and an instrument word missing its last letter.
        instruments = list(MT._field_words(MT.INSTRUMENT_KEY).values())
        difficulties = list(MT._field_words(MT.DIFFICULTY_KEY).values())
        instrument, difficulty = instruments[0], difficulties[0]
        self.assertNotIn(instrument, difficulties)
        self.assertNotIn(instrument[:-1], instruments)
        replies = self.recorded()
        tools = MT.MenuTools(self.game, replies, self.lock, kit=self.kit)
        for asked in (instrument, f"{instrument},{instrument}",
                      f"{instrument[:-1]},{difficulty}"):
            with self.subTest(asked=asked):
                with self.assertRaises(ValueError) as caught:
                    MT.player_target(asked)
                reply = tools.ch_open_song(self.target, asked)
                self.assertTrue(reply.startswith("refused:"), reply)
                self.assertIn(str(caught.exception), reply)
        # Refused before the first read, so before the backup and any press.
        self.assertEqual(replies.served, [])
        self.assertEqual(self.backups(), [])

    def test_a_steps_file_with_an_item_the_kit_cannot_press_is_refused_before_any_read(self):
        bad = "lane:" + next(iter(KT.KIT_ELEMENTS))
        self.assertIsNotNone(MT.item_problem(bad))
        path = self.steps_file([{"name": "x", "items": [bad], "expect": self.first["expect"]}])
        with self.assertRaises(MT.StepsInvalid) as caught:
            MT.load_steps(path)
        replies = self.recorded()
        tools = MT.MenuTools(self.game, replies, self.lock, kit=self.kit, steps_path=path)
        reply = tools.ch_open_song(self.target)
        self.assertTrue(reply.startswith("refused:"), reply)
        self.assertIn(str(caught.exception), reply)
        self.assertEqual(replies.served, [])
        self.assertEqual(self.backups(), [])

    def test_a_step_waits_menu_step_cap_s_reading_every_menu_poll_s(self):
        # A walk of one step with no presses, waiting for the checked-in
        # walk's first word, on MenuTools' own clock. No recorded read shows
        # that word, so the step waits out its cap. This takes
        # MENU_STEP_CAP_S of real time.
        word = self.first["expect"]
        replies = self.recorded()
        for read in replies.reads:
            if MT.EXPECT[word](read, self.target):
                self.fail(f"a recorded read shows {word}, so the step would not wait: {read}")
        path = self.steps_file([{"name": "wait", "items": [], "expect": word}])
        tools = MT.MenuTools(self.game, replies, self.lock, kit=self.kit, steps_path=path)
        reply = tools.ch_open_song(self.target)
        self.assertIn(f"stopped at step 1 (wait): {word} did not hold within", reply)
        times = replies.times
        # Read 0 is the one before the step; the step's wait reads the rest.
        self.assertGreaterEqual(len(times), 3, times)
        # The wait lasts the cap, and stops at the first read past it.
        self.assertGreaterEqual(times[-1] - times[0], MT.MENU_STEP_CAP_S)
        self.assertLess(times[-2] - times[1], MT.MENU_STEP_CAP_S)
        for before, after in zip(times, times[1:]):
            self.assertGreaterEqual(after - before, MT.MENU_POLL_S)


class ExpectWordsTest(unittest.TestCase):
    def test_a_real_read_while_a_song_plays_holds_only_the_loaded_song_words(self):
        # In every recorded read the agent found each menu class with no live
        # object, because a song was playing. So of every expect word, the
        # ones that hold are exactly the words of the checked-in walk's steps
        # that do not rest on a menu object (menu_tools marks those that do
        # "unproven"), with the loaded folder as the target.
        reads = recorded_reads()
        if not reads:
            self.fail("the recorded state logs hold no read")
        steps = MT.load_steps()["steps"]
        loaded_words = {step["expect"] for step in steps if not step.get("unproven")}
        if not loaded_words or len(loaded_words) == len(steps):
            self.fail("menu_steps.json marks no step, or every step, unproven")
        for read in reads:
            classes = read[FT.MENU_BLOCK]["classes"]
            live = {name: entry.get("live") for name, entry in classes.items()}
            if any(live.values()):
                self.fail(f"a recorded read has a live menu object: {live}")
            target = MT.loaded_folder(read)
            with self.subTest(loaded=target):
                holding = {word for word, holds in MT.EXPECT.items() if holds(read, target)}
                self.assertEqual(holding, loaded_words)

    def test_only_true_counts_as_active(self):
        for reads in ([False], [None], ["error: x"], []):
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


class NumbersTest(unittest.TestCase):
    def test_the_key_gap_is_input_drivers_not_defined_here(self):
        self.assertIs(MT.MENU_KEY_GAP_S, input_driver.MENU_KEY_GAP_S)
        with open(MT.__file__, encoding="utf-8") as f:
            text = f.read()
        self.assertIsNone(re.search(r"^\s*MENU_KEY_GAP_S\s*=", text, re.M))


class StepsFileTest(unittest.TestCase):
    def setUp(self):
        self.data = MT.load_steps()
        self.steps = self.data["steps"]

    def test_every_source_is_a_web_page_or_a_repo_file_that_exists(self):
        # The test names no repo file itself: a plan the file cites leaves the
        # repo when its work is done, and this check then fails until the
        # citation moves with it.
        sources = self.data["sources"]
        self.assertTrue(sources, "menu_steps.json cites no source")
        for source in sources:
            url = urllib.parse.urlparse(source)
            with self.subTest(source=source):
                if url.scheme:
                    self.assertEqual(url.scheme, "https")
                    self.assertTrue(url.netloc and url.path.strip("/"))
                else:
                    self.assertTrue(os.path.isfile(os.path.join(_REPO_ROOT, source)))

    def test_every_item_is_a_kit_item(self):
        for step in self.steps:
            for item in step["items"]:
                self.assertIsNone(MT.item_problem(item), item)

    def test_the_item_check_takes_only_one_prefixed_kit_name(self):
        # Built from kit_tools' own names and prefix: every kit name with the
        # prefix passes, in any case and with a space after the prefix; a
        # bare name, another prefix, two names in one item, a name the kit
        # lacks and the prefix alone are refused.
        names = list(KT.KIT_ELEMENTS)
        self.assertGreaterEqual(len(names), 2)
        self.assertIsNotNone(MT.item_problem(KT.KIT_PREFIX))
        for name, other in zip(names, names[1:] + names[:1]):
            with self.subTest(name=name):
                self.assertIsNone(MT.item_problem(KT.KIT_PREFIX + name))
                self.assertIsNone(MT.item_problem((KT.KIT_PREFIX + " " + name).upper()))
                unknown = name + "x"
                self.assertNotIn(unknown, KT.KIT_ELEMENTS)
                for bad in (name, "lane:" + name, f"{KT.KIT_PREFIX}{name},{other}",
                            KT.KIT_PREFIX + unknown):
                    self.assertIsNotNone(MT.item_problem(bad), bad)

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

    def test_it_ends_on_a_word_every_real_read_of_a_playing_song_holds(self):
        reads = recorded_reads()
        if not reads:
            self.fail("the recorded state logs hold no read")
        holds = MT.EXPECT[self.steps[-1]["expect"]]
        for read in reads:
            with self.subTest(loaded=MT.loaded_folder(read)):
                self.assertTrue(holds(read, MT.loaded_folder(read)))


if __name__ == "__main__":
    unittest.main()
