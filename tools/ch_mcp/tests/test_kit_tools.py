"""Tests for kit_tools, the virtual drum kit's tools.

No game and no Frida: the fake frida module and its scripted replies come
from test_frida_attach, the game side and the Frida tools are fakes with the
methods KitTools calls, and the clock is a fake that sleeping moves.
"""

from __future__ import annotations

import csv
import io
import os
import sys
import tempfile
import threading
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import kit_tools as K  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    FakeFrida, ReplySession, instant_wait)
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests.test_play_chart import _FakeSongClock, _FakeTime  # noqa: E402

BACKUP_LINE = "backup folder: X (taken now)"

# What kit.ts's status sends, by the field names it reads.
STATUS = {"frame": 1000, "focused": False, "switch_now": True, "setup_recorded": False,
          "kit_tag": "hydra-kit", "kit_exists": False, "kit_id": None,
          "players": [{"id": 0, "name": "Player1", "has_keyboard": True, "has_mouse": False}]}

# The ten bindings the probe read off the game's default map for the kit.
BINDINGS = [
    {"element_id": 0, "element_name": "Snare", "action_id": 1, "action_name": "Red"},
    {"element_id": 1, "element_name": "Tom1", "action_id": 2, "action_name": "Yellow"},
    {"element_id": 2, "element_name": "Tom2", "action_id": 3, "action_name": "Blue"},
    {"element_id": 3, "element_name": "Tom3", "action_id": 0, "action_name": "Green"},
    {"element_id": 4, "element_name": "Cymbal1", "action_id": 37, "action_name": "YellowCymbal"},
    {"element_id": 5, "element_name": "Cymbal2", "action_id": 38, "action_name": "BlueCymbal"},
    {"element_id": 6, "element_name": "Cymbal3", "action_id": 39, "action_name": "GreenCymbal"},
    {"element_id": 20, "element_name": "Start", "action_id": 16, "action_name": "Start"},
    {"element_id": 21, "element_name": "Select", "action_id": 6, "action_name": "Select"},
    {"element_id": 22, "element_name": "Kick", "action_id": 4, "action_name": "Orange"},
]


def setup_reply(bindings=BINDINGS):
    return {
        "first_setup": True, "switch_was": True, "switch_now": False,
        "players_was": [
            {"id": 9999999, "name": "System", "has_keyboard": True, "has_mouse": True},
            {"id": 0, "name": "Player1", "has_keyboard": True, "has_mouse": False}],
        "players_now": [
            {"id": 9999999, "name": "System", "has_keyboard": False, "has_mouse": False},
            {"id": 0, "name": "Player1", "has_keyboard": False, "has_mouse": False}],
        "kit": {"id": 7, "tag": "hydra-kit", "created": True, "player": "Player1"},
        "maps": [{"category_id": 1, "layout_id": 2, "enabled": True, "bindings": list(bindings)}],
    }


PRESS_REPLY = {"down_frame": 1200, "up_frame": 1202, "down_ms": 5000, "up_ms": 5013,
               "hold_frames": 2, "rewrite": True, "on_main": False}

TEARDOWN_OK = {"checks": [
    {"what": "elements cleared", "ok": True, "detail": "2 cleared"},
    {"what": "kit gone", "ok": True, "detail": "no controller tagged hydra-kit"},
    {"what": "switch", "ok": True, "detail": "ignoreInputWhenAppNotInFocus true (was true)"},
], "all_ok": True}

TEARDOWN_MISMATCH = {"checks": [
    {"what": "kit gone", "ok": True, "detail": "no controller tagged hydra-kit"},
    {"what": "player 0 flags", "ok": False,
     "detail": "Player1: keyboard false (was true), mouse false (was false)"},
], "all_ok": False}


class FakeGame:
    """The game_tools.GameTools methods KitTools calls."""

    def __init__(self, events, folder):
        self.events = events
        self.folder = folder
        self.hooks = []
        self.kit_source = None
        self.play = None
        self.fg = "foreground went to the game 0 times so far"

    def add_stop_hook(self, hook):
        self.hooks.append(hook)

    def use_kit_press(self, source):
        self.kit_source = source

    def ensure_backup(self):
        self.events.append("backup")
        return BACKUP_LINE

    def session_dir(self):
        return self.folder

    def play_snapshot(self):
        return self.play

    def foreground_line(self):
        return self.fg


class FakeFridaTools:
    def __init__(self, attached=True):
        self.attached = attached

    def is_attached(self):
        return self.attached


class RecordingFrida(FakeFrida):
    def __init__(self, session, events):
        super().__init__(session)
        self.events = events

    def attach(self, name):
        self.events.append("attach")
        return super().attach(name)


class FakeClock:
    def __init__(self):
        self.t = 0.0

    def now(self):
        return self.t

    def sleep(self, s):
        self.t += s


class Rig(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.agent_dir = os.path.join(self.tmp, "agent")
        os.makedirs(self.agent_dir)
        with open(os.path.join(self.agent_dir, K.KIT_AGENT_FILE), "w", encoding="utf-8") as f:
            f.write("// fake kit agent\n")
        self.events = []
        self.clock = FakeClock()

    def tools(self, replies=(), attached=True, setting="0"):
        self.game = FakeGame(self.events, self.tmp)
        self.session = ReplySession(list(replies))
        self.frida = RecordingFrida(self.session, self.events)

        def read_settings():
            if isinstance(setting, Exception):
                raise setting
            return setting

        tools = K.KitTools(self.game, FakeFridaTools(attached), threading.RLock(),
                           agent_dir=self.agent_dir, frida_mod=self.frida,
                           wait=instant_wait, now=self.clock.now, sleep=self.clock.sleep,
                           read_settings=read_settings)
        # Detach before the temp folder goes, so Windows can delete the log.
        self.addCleanup(lambda: tools._agent is not None and tools._agent.detach())
        return tools

    def posted(self):
        return [{k: v for k, v in m.items() if k != "id"} for m in self.session.script.posted]

    def set_up(self, more=()):
        tools = self.tools([STATUS, setup_reply(), *more])
        reply = tools.ch_kit_setup()
        self.assertIn("bindings: all 10 match KIT_BINDINGS", reply)
        return tools


class ConstantsTest(unittest.TestCase):
    def test_the_users_answers_and_the_probe_values_are_pinned(self):
        self.assertEqual(K.KIT_HOLD_FRAMES, 2)
        self.assertEqual(K.KIT_SETUP_CAP_S, 10)
        self.assertEqual(K.KIT_PRESS_CAP_S, 1)
        self.assertEqual(K.KIT_AXIS_VALUE, 1.0)
        self.assertEqual(K.KIT_WATCH_MAX_S, 60.0)
        self.assertEqual(K.KIT_DEFINITION_ID, 6)
        self.assertEqual(K.KIT_TAG, "hydra-kit")
        self.assertEqual(K.KIT_PLAYER, "Player1")

    def test_every_auto_player_lane_has_a_kit_element(self):
        self.assertEqual({lane: K.lane_element(lane).id for lane in Lane}, {
            Lane.GREEN: 3, Lane.RED: 0, Lane.YELLOW: 1, Lane.BLUE: 2, Lane.KICK: 22,
            Lane.YELLOW_CYMBAL: 4, Lane.BLUE_CYMBAL: 5, Lane.GREEN_CYMBAL: 6})

    def test_items_take_the_kit_prefix_and_refuse_unknown_names(self):
        self.assertEqual(K.parse_items("kit:start, Green,kick,green"), ["start", "green", "kick"])
        with self.assertRaises(K.KitRefused) as cm:
            K.parse_items("purple")
        self.assertTrue(str(cm.exception).startswith('unknown kit item "purple"; kit items: red,'))
        with self.assertRaises(K.KitRefused):
            K.parse_items(" , ")


class SetupTest(Rig):
    def test_refused_without_ch_attach(self):
        tools = self.tools(attached=False)
        self.assertEqual(tools.ch_kit_setup(), FT.NOT_ATTACHED)
        self.assertEqual(self.events, [])

    def test_refused_when_the_settings_line_is_1(self):
        reply = self.tools(setting="1").ch_kit_setup()
        self.assertEqual(reply, "refused: settings.ini reads pause_on_focus_lost = 1, not 0; "
                                "start the game with ch_start, which sets it for the run")
        self.assertEqual(self.events, [])

    def test_refused_when_the_settings_line_is_missing(self):
        reply = self.tools(setting=G.SettingNotFound("no `pause_on_focus_lost` line")).ch_kit_setup()
        self.assertTrue(reply.startswith("refused: could not read pause_on_focus_lost"))
        self.assertIn("ch_start", reply)
        self.assertEqual(self.events, [])

    def test_backup_then_attach_then_setup_and_the_flags_are_printed(self):
        tools = self.tools([STATUS, setup_reply()])
        lines = tools.ch_kit_setup().splitlines()
        self.assertEqual(self.events, ["backup", "attach"])
        self.assertEqual(self.posted(), [
            {"type": "read"},
            {"type": "setup", "tag": "hydra-kit", "definition_id": 6, "player": "Player1"}])
        self.assertEqual(lines[0], BACKUP_LINE)
        self.assertIn("ignoreInputWhenAppNotInFocus: was on, now off", lines)
        self.assertIn("player 9999999 System: keyboard was on, now off; mouse was on, now off", lines)
        self.assertIn("player 0 Player1: keyboard was on, now off; mouse was off, now off", lines)
        self.assertIn("kit: controller 7, tag hydra-kit, created, on Player1", lines)
        self.assertIn("map: category 1, layout 2, enabled, 10 bindings", lines)
        self.assertIn("  Kick (element 22) -> Orange (action 4)", lines)
        self.assertEqual(lines[-1], "bindings: all 10 match KIT_BINDINGS")
        self.assertIsNotNone(self.game.kit_source())

    def test_a_map_that_differs_is_refused_naming_the_element_and_torn_down(self):
        wrong = [b if b["element_id"] != 22 else {**b, "action_id": 0, "action_name": "Green"}
                 for b in BINDINGS]
        tools = self.tools([STATUS, setup_reply(wrong), TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("refused: the kit's bindings differ from KIT_BINDINGS:", lines)
        self.assertIn("  Kick (element 22): expected action 4 (Orange), the map gives 0", lines)
        self.assertEqual(self.posted()[-1], {"type": "teardown", "tag": "hydra-kit"})
        self.assertIn("kit teardown: all checks ok", lines)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())

    def test_a_disabled_map_or_a_missing_binding_is_a_difference(self):
        reply = setup_reply()
        reply["maps"][0]["enabled"] = False
        self.assertEqual(len(K.binding_problems(reply["maps"])), 10)
        missing = K.binding_problems(setup_reply(BINDINGS[1:])["maps"])
        self.assertEqual(missing, ["Snare (element 0): expected action 1 (Red), the map gives none"])
        self.assertEqual(K.binding_problems([]), ["the setup reply holds no map for the kit"])

    def test_a_setup_with_no_reply_ends_at_its_cap_and_tears_down(self):
        tools = self.tools([STATUS, None, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("setup failed: no reply to the setup request within the cap of 10 s "
                      "(KIT_SETUP_CAP_S)", lines)
        self.assertEqual(self.posted()[-1]["type"], "teardown")
        self.assertEqual(lines[-1], "kit agent: detached")


class PressTest(Rig):
    def test_green_and_kick_send_elements_3_and_22_with_the_hold(self):
        tools = self.set_up([PRESS_REPLY])
        reply = tools.ch_kit_press("green,kick")
        self.assertEqual(self.posted()[-1], {
            "type": "press", "elements": [{"id": 3, "value": 1.0}, {"id": 22, "value": True}],
            "hold_frames": 2, "cap_s": 1, "rewrite": True, "on_main": False})
        self.assertEqual(reply, "pressed green, kick (elements 3, 22) for 2 frames: down at frame "
                                "1200, up at frame 1202 (2 frames apart, 13 ms by the agent's clock)")

    def test_a_given_hold_is_sent(self):
        tools = self.set_up([PRESS_REPLY])
        self.assertTrue(tools.press_items(["kit:start"], hold_frames=5).sent)
        self.assertEqual(self.posted()[-1]["hold_frames"], 5)
        self.assertEqual(self.posted()[-1]["elements"], [{"id": 20, "value": True}])

    def test_a_reply_that_never_comes_ends_at_the_press_cap(self):
        tools = self.set_up([None])
        result = tools.press_items("red")
        self.assertFalse(result.sent)
        self.assertEqual(result.text, "press failed: no reply to the press request within the "
                                      "cap of 1 s (KIT_PRESS_CAP_S)")

    def test_refused_before_setup_and_while_a_play_session_runs(self):
        tools = self.tools()
        self.assertEqual(tools.ch_kit_press("red"), K.NOT_SET_UP)
        tools = self.set_up()
        self.game.play = {"last_row": None, "press_count": 0, "stop": None}
        self.assertEqual(tools.ch_kit_press("red"), K.PLAY_RUNNING)
        self.assertEqual(tools.ch_kit_press("red", hold_frames=0),
                         "hold_frames is a whole number of 1 or more, not 0")


class PressOptionsTest(Rig):
    def test_a_press_with_read_sends_the_names_and_prints_seen(self):
        reply = {**PRESS_REPLY, "read": ["Green"], "seen": [
            {"frame": 1200, "action": "Green", "value": False},
            {"frame": 1201, "action": "Green", "value": True},
            {"frame": 1202, "action": "Green", "value": False}]}
        tools = self.set_up([reply])
        lines = tools.ch_kit_press("green", read="Green").splitlines()
        self.assertEqual(self.posted()[-1], {
            "type": "press", "elements": [{"id": 3, "value": 1.0}], "hold_frames": 2,
            "cap_s": 1, "rewrite": True, "on_main": False, "read": ["Green"]})
        self.assertEqual(lines[1:], [
            "read back on the kit's player: Green",
            "  frame 1200: Green off",
            "  frame 1201: Green on",
            "  frame 1202: Green off"])

    def test_read_takes_a_list_each_name_once_and_refuses_a_name_that_is_not_text(self):
        tools = self.set_up([{**PRESS_REPLY, "read": ["Green", "Orange"], "seen": []}])
        lines = tools.ch_kit_press("green", read=["Green", " Orange", "Green"]).splitlines()
        self.assertEqual(self.posted()[-1]["read"], ["Green", "Orange"])
        self.assertEqual(lines[-2:], ["read back on the kit's player: Green, Orange",
                                      "  no reads came back"])
        self.assertEqual(tools.ch_kit_press("green", read=[3]),
                         "read takes game action names, not 3")

    def test_none_sends_the_constants_and_a_given_value_overrides_them(self):
        tools = self.set_up([PRESS_REPLY, {**PRESS_REPLY, "rewrite": False, "on_main": True}])
        reply = tools.ch_kit_press("green")
        self.assertEqual((self.posted()[-1]["rewrite"], self.posted()[-1]["on_main"]),
                         (K.KIT_REWRITE, K.KIT_PRESS_ON_MAIN))
        self.assertNotIn("read", self.posted()[-1])
        self.assertEqual(len(reply.splitlines()), 1)
        lines = tools.ch_kit_press("green", hold_frames=1, rewrite=False,
                                   on_main=True).splitlines()
        self.assertEqual((self.posted()[-1]["rewrite"], self.posted()[-1]["on_main"],
                          self.posted()[-1]["hold_frames"]), (False, True, 1))
        self.assertEqual(lines[1], "rewrite off, main thread on (as the agent ran it)")

    def test_rewrite_and_on_main_take_only_true_or_false(self):
        tools = self.set_up()
        self.assertEqual(tools.ch_kit_press("green", rewrite="no"),
                         "rewrite is true, false or left out, not 'no'")
        self.assertEqual(tools.ch_kit_press("green", on_main=1),
                         "on_main is true, false or left out, not 1")


WATCH_QUIET = {"seconds": 20, "ticks": 3100, "first_frame": 5000, "last_frame": 8200,
               "readers": ["System", "Player1", "controllers"], "true_count": 0, "reads": []}


class WatchTest(Rig):
    def test_a_watch_with_no_true_read_says_no_key_reached_a_player(self):
        tools = self.set_up([WATCH_QUIET])
        lines = tools.ch_kit_watch(20).splitlines()
        self.assertEqual(self.posted()[-1], {"type": "watch", "seconds": 20, "cap_s": 21})
        self.assertEqual(lines, [
            "watched 20 s: 3100 ticks, frames 5000 to 8200, readers: System, Player1, controllers",
            "no key reached a player"])

    def test_a_true_read_is_a_failure_naming_the_frame_and_the_reader(self):
        loud = {**WATCH_QUIET, "true_count": 7,
                "reads": [{"ms": 4210, "frame": 5672, "who": "System"},
                          {"ms": 9000, "frame": 6440, "who": "controllers"}]}
        tools = self.set_up([loud])
        lines = tools.ch_kit_watch(20).splitlines()
        self.assertEqual(lines[1:], [
            "FAILED: a key reached a player: 7 true reads; the first 2 that turned true:",
            "  at 4210 ms, frame 5672: System GetAnyButton true",
            "  at 9000 ms, frame 6440: controllers GetAnyButton true"])

    def test_refused_while_a_play_session_runs_before_setup_and_on_a_bad_length(self):
        tools = self.tools()
        self.assertEqual(tools.ch_kit_watch(20), K.NOT_SET_UP)
        tools = self.set_up()
        sent = len(self.posted())
        self.game.play = {"last_row": None, "press_count": 0, "stop": None}
        self.assertEqual(tools.ch_kit_watch(20), K.PLAY_RUNNING)
        self.game.play = None
        for bad in (0, -1, True, "20", float("inf")):
            self.assertEqual(tools.ch_kit_watch(bad), f"seconds is a number above 0, not {bad!r}")
        self.assertEqual(len(self.posted()), sent)

    def test_a_watch_that_sampled_nothing_is_a_failure_not_a_pass(self):
        for empty in ({**WATCH_QUIET, "ticks": 0}, {**WATCH_QUIET, "readers": []},
                      {k: v for k, v in WATCH_QUIET.items() if k != "ticks"}):
            tools = self.set_up([empty])
            lines = tools.ch_kit_watch(20).splitlines()
            self.assertEqual(lines[-1], "FAILED: the watch sampled nothing, so it proves nothing")

    def test_a_reply_with_listed_reads_fails_even_if_its_count_says_zero(self):
        odd = {**WATCH_QUIET, "reads": [{"ms": 5, "frame": 5001, "who": "Player1"}]}
        tools = self.set_up([odd])
        lines = tools.ch_kit_watch(20).splitlines()
        self.assertTrue(lines[1].startswith("FAILED: a key reached a player"))

    def test_a_watch_is_allowed_at_the_cap_and_refused_just_above_it(self):
        tools = self.set_up([{**WATCH_QUIET, "seconds": 60}])
        sent = len(self.posted())
        lines = tools.ch_kit_watch(60).splitlines()
        self.assertEqual(self.posted()[-1], {"type": "watch", "seconds": 60, "cap_s": 61})
        self.assertEqual(lines[-1], "no key reached a player")
        self.assertEqual(len(self.posted()), sent + 1)
        self.assertEqual(tools.ch_kit_watch(60.001),
                         "seconds is at most 60 (KIT_WATCH_MAX_S), not 60.001")
        self.assertEqual(tools.ch_kit_watch(3600),
                         "seconds is at most 60 (KIT_WATCH_MAX_S), not 3600")
        self.assertEqual(len(self.posted()), sent + 1)

    def test_a_watch_with_no_reply_ends_at_its_cap(self):
        tools = self.set_up([None])
        self.assertEqual(tools.ch_kit_watch(2.5),
                         "watch failed: no reply to the watch request within the cap of 3.5 s "
                         "(seconds plus KIT_PRESS_CAP_S)")


class KitPressTest(Rig):
    def test_the_kit_press_reply_rides_into_the_press_row_and_the_log(self):
        tools = self.set_up([PRESS_REPLY])
        press = self.game.kit_source()
        wall = _FakeTime()
        chords = [play_chart.Chord(7, 1000.0, [Lane.GREEN, Lane.KICK])]
        score = iter([0, 50])
        result = play_chart.play_chords(
            chords, _FakeSongClock(wall, start_s=0.5), lambda: next(score), press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO())
        self.assertEqual(result.stop, "done")
        self.assertEqual(self.posted()[-1]["elements"],
                         [{"id": 3, "value": 1.0}, {"id": 22, "value": True}])
        row = result.rows[0]
        self.assertEqual((row["down_frame"], row["up_frame"], row["down_ms"], row["up_ms"]),
                         (1200, 1202, 5000, 5013))
        self.assertNotIn("rewrite", row)
        path = os.path.join(self.tmp, "press.csv")
        play_chart.write_press_log(path, result.rows)
        with open(path, encoding="utf-8", newline="") as f:
            table = list(csv.DictReader(f))
        self.assertEqual((table[0]["down_frame"], table[0]["up_ms"]), ("1200", "5013"))
        self.assertIsNotNone(tools.kit_press())

    def test_no_kit_press_before_setup(self):
        self.tools()
        self.assertIsNone(self.game.kit_source())


class StatusTest(Rig):
    def test_the_frame_rate_comes_from_two_reads_one_second_apart(self):
        later = {**STATUS, "frame": 1160, "kit_exists": True, "kit_id": 7,
                 "players": [{"id": 0, "name": "Player1", "has_keyboard": False,
                              "has_mouse": False}]}
        tools = self.set_up([STATUS, later])
        lines = tools.ch_kit_status().splitlines()
        self.assertEqual(self.posted()[-2:], [{"type": "status", "tag": "hydra-kit"}] * 2)
        self.assertEqual(lines, [
            "frame: 1160",
            "frame rate: 160.0 frames a second (160 frames in 1.00 s)",
            "game focused: off",
            "ignoreInputWhenAppNotInFocus: on",
            "kit: controller 7, tag hydra-kit",
            "player 0 Player1: keyboard off, mouse off",
            "foreground went to the game 0 times so far"])

    def test_no_agent_says_so_and_still_prints_the_foreground_line(self):
        tools = self.tools()
        self.game.fg = None
        self.assertEqual(tools.ch_kit_status().splitlines(), [
            "kit agent: not attached; call ch_kit_setup",
            "foreground: no watch (the server did not start the game)"])


class TeardownTest(Rig):
    def test_teardown_prints_each_check_and_detaches(self):
        tools = self.set_up([TEARDOWN_OK])
        lines = tools.ch_kit_teardown().splitlines()
        self.assertEqual(self.posted()[-1], {"type": "teardown", "tag": "hydra-kit"})
        self.assertEqual(lines, [
            "kit teardown: elements cleared: ok (2 cleared)",
            "kit teardown: kit gone: ok (no controller tagged hydra-kit)",
            "kit teardown: switch: ok (ignoreInputWhenAppNotInFocus true (was true))",
            "kit teardown: all checks ok",
            "kit agent: detached"])
        self.assertEqual(self.session.detach_calls, 1)
        self.assertIsNone(self.game.kit_source())

    def test_a_mismatch_is_printed_as_a_failure(self):
        tools = self.set_up([TEARDOWN_MISMATCH])
        lines = tools.ch_kit_teardown().splitlines()
        self.assertIn("kit teardown failed on player 0 flags: Player1: keyboard false "
                      "(was true), mouse false (was false)", lines)
        self.assertIn("kit teardown: NOT all checks ok; see the lines above", lines)

    def test_refused_while_a_play_session_runs_and_with_nothing_attached(self):
        tools = self.tools()
        self.assertEqual(tools.ch_kit_teardown(), "no kit agent attached; nothing to tear down")
        self.game.play = {"last_row": None, "press_count": 3, "stop": None}
        self.assertEqual(tools.ch_kit_teardown(), K.PLAY_RUNNING)

    def test_the_stop_hook_tears_down_then_detaches(self):
        tools = self.set_up([TEARDOWN_OK])
        self.assertEqual(len(self.game.hooks), 1)
        said = self.game.hooks[0]().splitlines()
        self.assertEqual(self.posted()[-1]["type"], "teardown")
        self.assertEqual(said[-2:], ["kit teardown: all checks ok", "kit agent: detached"])
        self.assertEqual(self.session.detach_calls, 1)
        self.assertIsNone(tools._agent)
        self.assertIsNone(self.game.hooks[0]())


class ReadPauseSettingTest(unittest.TestCase):
    def test_reads_the_game_sections_line(self):
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, G.SETTINGS), "wb") as f:
                f.write(b"[audio]\r\npause_on_focus_lost = 7\r\n[game]\r\n"
                        b"pause_on_focus_lost = 0\r\nnote_speed = 7\r\n")
            self.assertEqual(K.read_pause_setting(d), "0")


if __name__ == "__main__":
    unittest.main()
