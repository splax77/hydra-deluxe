"""Tests for kit_tools, the virtual drum kit's tools.

No game and no Frida: the fake frida module and its scripted replies come
from test_frida_attach, the game side and the Frida tools are fakes with the
methods KitTools calls, and the clock is a fake that sleeping moves.
"""

from __future__ import annotations

import csv
import io
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import game_files as G  # noqa: E402
from tools.ch_mcp import kit_tools as K  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    FakeFrida, ReplySession, instant_wait)
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_probe.experiments import play_chart  # noqa: E402
from tools.ch_probe.input_driver import Lane  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402
from tools.ch_probe.tests.test_play_chart import _FakeSongClock, _FakeTime  # noqa: E402

BACKUP_LINE = "backup folder: X (taken now)"

# What kit.ts's status sends, by the field names it reads.
STATUS = {"frame": 1000, "focused": False, "switch_now": True, "keyboard_now": True,
          "mouse_now": True, "setup_recorded": False,
          "kit_tag": "hydra-kit", "kit_exists": False, "kit_id": None,
          "players": [{"id": 0, "name": "Player1", "has_keyboard": True, "has_mouse": False}]}

# The kit is built from the game's MIDI Drums definition (definition 1), so
# its kick is element 7, Start and Select are elements 18 and 19, and every
# element is an axis pressed with a number, never True; the tests that once
# named the kick as button element 22 use these numbers now
# (docs/superpowers/plans/2026-11-08-kick-axis-bind.md, task T1).
#
# The ten bindings of the game's map 2 for definition 1, as level1 ships
# them (row P9 of that plan), every one an axis binding.
BINDINGS = [
    {"element_id": 0, "element_name": "Snare", "action_id": 1, "action_name": "Red"},
    {"element_id": 1, "element_name": "Tom1", "action_id": 2, "action_name": "Yellow"},
    {"element_id": 2, "element_name": "Tom2", "action_id": 3, "action_name": "Blue"},
    {"element_id": 3, "element_name": "Tom3", "action_id": 0, "action_name": "Green"},
    {"element_id": 4, "element_name": "Cymbal1", "action_id": 37, "action_name": "YellowCymbal"},
    {"element_id": 5, "element_name": "Cymbal2", "action_id": 38, "action_name": "BlueCymbal"},
    {"element_id": 6, "element_name": "Cymbal3", "action_id": 39, "action_name": "GreenCymbal"},
    {"element_id": 7, "element_name": "Kick", "action_id": 4, "action_name": "Orange"},
    {"element_id": 18, "element_name": "StartPad", "action_id": 16, "action_name": "Start"},
    {"element_id": 19, "element_name": "SelectPad", "action_id": 6, "action_name": "Select"},
]
BINDINGS = [{**b, "element_type": 0} for b in BINDINGS]

# The map ls-d2b-run1 read back for a kit built from the SLInputDrums
# definition: the same pads, but Start 20, Select 21 and Kick 22 bound as
# buttons (row P10 of the plan).
OLD_BINDINGS = BINDINGS[:7] + [
    {"element_id": 20, "element_name": "Start", "action_id": 16, "action_name": "Start",
     "element_type": 1},
    {"element_id": 21, "element_name": "Select", "action_id": 6, "action_name": "Select",
     "element_type": 1},
    {"element_id": 22, "element_name": "Kick", "action_id": 4, "action_name": "Orange",
     "element_type": 1},
]


# The six placeholder tags, as the setup and teardown messages carry them.
PLACEHOLDER_TAGS = [f"hydra-placeholder-{i}" for i in range(6)]
TEARDOWN_MESSAGE = {"type": "teardown", "tag": "hydra-kit",
                    "placeholder_tags": PLACEHOLDER_TAGS}


def setup_reply(bindings=BINDINGS, kit_id=6):
    return {
        "first_setup": True, "switch_was": True, "switch_now": False,
        "keyboard_was": True, "keyboard_now": False, "mouse_was": True, "mouse_now": False,
        "players_was": [
            {"id": 9999999, "name": "System", "has_keyboard": True, "has_mouse": True},
            {"id": 0, "name": "Player1", "has_keyboard": True, "has_mouse": False}],
        "players_now": [
            {"id": 9999999, "name": "System", "has_keyboard": False, "has_mouse": False},
            {"id": 0, "name": "Player1", "has_keyboard": False, "has_mouse": False}],
        "placeholders": [{"id": i, "tag": tag} for i, tag in enumerate(PLACEHOLDER_TAGS)],
        "placeholders_created": 6,
        "kit": {"id": kit_id, "tag": "hydra-kit", "created": True, "player": "Player1"},
        "maps": [{"category_id": 1, "layout_id": 2, "enabled": True, "bindings": list(bindings)}],
    }


# A setup error reply carrying a native fault, in the shape kit.ts's faultOf
# sends: hex as bare digits, registers in no particular order.
SETUP_FAULT = {
    "error": "setup failed: Error: access violation accessing 0x58",
    "fault": {
        "type": "access-violation", "address": "7ffb21f557f2",
        "module": "GameAssembly.dll", "offset": "1f557f2",
        "operation": "read", "memory_address": "58",
        "registers": {"pc": "7ffb21f557f2", "sp": "aff0", "rip": "7ffb21f557f2",
                      "rsp": "aff0", "rax": "0", "rcx": "18", "rdx": "2", "rbx": "1c2a0",
                      "rbp": "0", "rsi": "0", "rdi": "0", "r8": "0", "r9": "0", "r10": "0",
                      "r11": "0", "r12": "0", "r13": "0", "r14": "0", "r15": "0"},
        "stack": ["Error: access violation accessing 0x58", "    at invokeRaw (method.ts:234)",
                  "    at setup (kit.ts:308)"],
    },
}

# Press replies in kit.ts's shape (tools.ch_probe.tests.fakes.kit_reply):
# one group, and a split press's two.
PRESS_REPLY = fakes.kit_reply([(1200, 1202, fakes.KIT_EPOCH_MS + 5000,
                                fakes.KIT_EPOCH_MS + 5013)])
SPLIT_PRESS_REPLY = fakes.kit_reply([(1200, 1202, fakes.KIT_EPOCH_MS + 5000,
                                      fakes.KIT_EPOCH_MS + 5013),
                                     (1203, 1205, fakes.KIT_EPOCH_MS + 5008,
                                      fakes.KIT_EPOCH_MS + 5021)])

TEARDOWN_OK = {"checks": [
    {"what": "elements cleared", "ok": True, "detail": "2 cleared"},
    {"what": "kit gone", "ok": True, "detail": "no controller tagged hydra-kit"},
    {"what": "keyboard", "ok": True, "detail": "Keyboard.enabled true (was true)"},
    {"what": "mouse", "ok": True, "detail": "Mouse.enabled true (was true)"},
    {"what": "switch", "ok": True, "detail": "ignoreInputWhenAppNotInFocus true (was true)"},
], "all_ok": True}

TEARDOWN_MISMATCH = {"checks": [
    {"what": "kit gone", "ok": True, "detail": "no controller tagged hydra-kit"},
    {"what": "player 0 flags", "ok": False,
     "detail": "Player1: keyboard false (was true), mouse false (was false)"},
    {"what": "keyboard", "ok": False, "detail": "Keyboard.enabled false (was true)"},
    {"what": "mouse", "ok": True, "detail": "Mouse.enabled true (was true)"},
], "all_ok": False}

# Only the mouse check fails: each device is its own step.
TEARDOWN_MOUSE_MISMATCH = {"checks": [
    {"what": "keyboard", "ok": True, "detail": "Keyboard.enabled true (was true)"},
    {"what": "mouse", "ok": False, "detail": "Mouse.enabled false (was true)"},
    {"what": "switch", "ok": True, "detail": "ignoreInputWhenAppNotInFocus true (was true)"},
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
        make_built_agent(self.agent_dir, K.KIT_AGENT_NAME)
        self.events = []
        self.clock = FakeClock()

    # The engine clock address the frame probe tests' finder hands back.
    CLOCK_ADDR = 0x2_0000_1100

    def find_clock(self):
        self.events.append("find clock")
        return self.CLOCK_ADDR

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
                           read_settings=read_settings, find_clock=self.find_clock)
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
        self.assertEqual(K.KIT_WATCH_MAX_S, 60.0)
        self.assertEqual(K.KIT_DEFINITION_ID, 1)
        self.assertEqual(K.KIT_DEFINITION_NAME, "MIDI Drums")
        self.assertEqual(K.KIT_TAG, "hydra-kit")
        self.assertEqual(K.KIT_PLAYER, "Player1")

    def test_the_drum_maps_category_and_layout_are_pinned(self):
        # Page 07, IN-76: category 1 ("Gameplay"), layout 2 ("Drums").
        self.assertEqual(K.KIT_MAP_CATEGORY_ID, 1)
        self.assertEqual(K.KIT_MAP_LAYOUT_ID, 2)

    def test_the_setup_message_carries_the_maps_ids(self):
        self.assertEqual(K.setup_message("System"), {
            "type": "setup", "tag": K.KIT_TAG, "definition_id": 1, "player": "System",
            "category_id": 1, "layout_id": 2, "placeholder_tags": PLACEHOLDER_TAGS})

    def test_every_element_is_an_axis_of_definition_1(self):
        self.assertEqual({n: (e.id, e.name, e.strength) for n, e in K.KIT_ELEMENTS.items()}, {
            "red": (0, "Snare", None), "yellow": (1, "Tom1", None),
            "blue": (2, "Tom2", None), "green": (3, "Tom3", None),
            "yellow_cymbal": (4, "Cymbal1", None), "blue_cymbal": (5, "Cymbal2", None),
            "green_cymbal": (6, "Cymbal3", None), "kick": (7, "Kick", None),
            "start": (18, "StartPad", 1.0), "select": (19, "SelectPad", 1.0)})
        self.assertEqual({k: v[0] for k, v in K.KIT_BINDINGS.items()}, {
            0: 1, 1: 2, 2: 3, 3: 0, 4: 37, 5: 38, 6: 39, 7: 4, 18: 16, 19: 6})
        self.assertEqual((K.ELEMENT_TYPE_AXIS, K.ELEMENT_TYPE_BUTTON), (0, 1))
        self.assertEqual(set(K.KIT_TYPE_CHECKED), set(K.KIT_ELEMENTS))

    def test_the_gates_id_and_the_placeholder_count_are_pinned(self):
        # The literal the gate passes: lane_velocity_337E10.asm line 53.
        self.assertEqual(K.KIT_REQUIRED_ID, 6)
        self.assertEqual(K.KIT_PLACEHOLDERS, 6)

    def test_no_placeholder_tag_equals_or_starts_with_the_kits_tag(self):
        tags = K.placeholder_tags()
        self.assertEqual(len(set(tags)), K.KIT_PLACEHOLDERS)
        for tag in tags:
            self.assertFalse(tag.startswith(K.KIT_TAG), tag)

    def test_every_auto_player_lane_has_a_kit_element(self):
        self.assertEqual({lane: K.lane_element(lane).id for lane in Lane}, {
            Lane.GREEN: 3, Lane.RED: 0, Lane.YELLOW: 1, Lane.BLUE: 2, Lane.KICK: 7,
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

    def test_a_stale_or_missing_kit_agent_is_refused_before_any_attach(self):
        path = os.path.join(self.agent_dir, K.KIT_AGENT_FILE)
        with open(os.path.join(self.agent_dir, "kit.ts"), "a", encoding="utf-8") as f:
            f.write("// an edit after the build\n")
        reply = self.tools([STATUS, setup_reply()]).ch_kit_setup()
        self.assertEqual(reply.splitlines()[-1],
                         f"{path} is stale: kit.ts changed since it was built; "
                         "run npm run build:kit in tools/ch_mcp/agent first")
        os.remove(BS.stamp_path(path))
        reply = self.tools([STATUS, setup_reply()]).ch_kit_setup()
        self.assertEqual(reply.splitlines()[-1], BS.check(path))
        self.assertIn("has no build stamp", reply)
        self.assertNotIn("attach", self.events)

    def test_backup_then_attach_then_setup_and_the_flags_are_printed(self):
        tools = self.tools([STATUS, setup_reply()])
        lines = tools.ch_kit_setup().splitlines()
        self.assertEqual(self.events, ["backup", "attach"])
        self.assertEqual(self.posted(), [
            {"type": "read"},
            {"type": "setup", "tag": "hydra-kit", "definition_id": 1, "player": "Player1",
             "category_id": 1, "layout_id": 2, "placeholder_tags": PLACEHOLDER_TAGS}])
        self.assertEqual(lines[0], BACKUP_LINE)
        self.assertIn("ignoreInputWhenAppNotInFocus: was on, now off", lines)
        self.assertIn("Keyboard.enabled: was on, now off", lines)
        self.assertIn("Mouse.enabled: was on, now off", lines)
        self.assertIn("player 9999999 System: keyboard was on, now off; mouse was on, now off", lines)
        self.assertIn("player 0 Player1: keyboard was on, now off; mouse was off, now off", lines)
        self.assertIn("kit: controller 6, definition 1 (MIDI Drums), tag hydra-kit, created, "
                      "on Player1", lines)
        self.assertIn("map: category 1, layout 2, enabled, 10 bindings", lines)
        self.assertIn("  Kick (element 7) -> Orange (action 4)", lines)
        self.assertEqual(lines[-1], "bindings: all 10 match KIT_BINDINGS")
        self.assertIsNotNone(self.game.kit_source())

    def test_a_map_that_differs_is_refused_naming_the_element_and_torn_down(self):
        wrong = [b if b["element_id"] != 7 else {**b, "action_id": 0, "action_name": "Green"}
                 for b in BINDINGS]
        tools = self.tools([STATUS, setup_reply(wrong), TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("refused: the kit's bindings differ from KIT_BINDINGS:", lines)
        self.assertIn("  Kick (element 7): expected action 4 (Orange), the map gives 0", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIn("kit teardown: all checks ok", lines)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())

    def test_a_keyboard_left_on_is_refused_and_torn_down(self):
        reply = setup_reply()
        reply["keyboard_now"] = True
        tools = self.tools([STATUS, reply, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("Keyboard.enabled: was on, now on", lines)
        self.assertIn("refused: Rewired's keyboard (Keyboard.enabled) reads on after setup, "
                      "not off, so the user's own input would still reach the game", lines)
        self.assertNotIn("bindings: all 10 match KIT_BINDINGS", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIn("kit teardown: keyboard: ok (Keyboard.enabled true (was true))", lines)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())

    def test_a_mouse_left_on_is_refused_and_torn_down(self):
        reply = setup_reply()
        reply["mouse_now"] = True
        tools = self.tools([STATUS, reply, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("Keyboard.enabled: was on, now off", lines)
        self.assertIn("Mouse.enabled: was on, now on", lines)
        self.assertIn("refused: Rewired's mouse (Mouse.enabled) reads on after setup, "
                      "not off, so the user's own input would still reach the game", lines)
        self.assertNotIn("bindings: all 10 match KIT_BINDINGS", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIn("kit teardown: mouse: ok (Mouse.enabled true (was true))", lines)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())

    def test_both_left_on_name_both_in_one_refusal(self):
        reply = {**setup_reply(), "keyboard_now": True, "mouse_now": True}
        self.assertEqual(K.devices_problem(reply),
                         "refused: Rewired's keyboard (Keyboard.enabled) reads on and Rewired's "
                         "mouse (Mouse.enabled) reads on after setup, not off, so the user's own "
                         "input would still reach the game")

    def test_a_reply_with_no_keyboard_or_mouse_read_back_is_refused(self):
        reply = setup_reply()
        del reply["keyboard_now"]
        self.assertEqual(K.devices_problem(reply),
                         "refused: Rewired's keyboard (Keyboard.enabled) reads None after setup, "
                         "not off, so the user's own input would still reach the game")
        reply = setup_reply()
        del reply["mouse_now"]
        self.assertEqual(K.devices_problem(reply),
                         "refused: Rewired's mouse (Mouse.enabled) reads None after setup, "
                         "not off, so the user's own input would still reach the game")
        self.assertIsNone(K.devices_problem(setup_reply()))

    def test_a_setup_reply_missing_mouse_now_is_refused_and_torn_down(self):
        reply = setup_reply()
        del reply["mouse_now"]
        tools = self.tools([STATUS, reply, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("Mouse.enabled: was on, now None", lines)
        self.assertIn("refused: Rewired's mouse (Mouse.enabled) reads None after setup, "
                      "not off, so the user's own input would still reach the game", lines)
        self.assertNotIn("bindings: all 10 match KIT_BINDINGS", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIsNone(self.game.kit_source())

    def test_a_disabled_map_or_a_missing_binding_is_a_difference(self):
        reply = setup_reply()
        reply["maps"][0]["enabled"] = False
        self.assertEqual(len(K.binding_problems(reply["maps"])), 10)
        missing = K.binding_problems(setup_reply(BINDINGS[1:])["maps"])
        self.assertEqual(missing, ["Snare (element 0): expected action 1 (Red), the map gives none"])
        self.assertEqual(K.binding_problems([]), ["the setup reply holds no map for the kit"])

    def test_the_definition_1_map_passes_both_checks(self):
        maps = setup_reply()["maps"]
        self.assertEqual(sorted(b["element_id"] for b in maps[0]["bindings"]),
                         [0, 1, 2, 3, 4, 5, 6, 7, 18, 19])
        self.assertEqual({b["element_type"] for b in maps[0]["bindings"]}, {0})
        self.assertEqual(K.map_problems(maps), [])
        self.assertEqual(K.binding_problems(maps), [])

    def test_the_ls_d2b_run1_map_is_refused_naming_kick_7_and_element_22(self):
        problems = K.binding_problems(setup_reply(OLD_BINDINGS)["maps"])
        self.assertIn("Kick (element 7): expected action 4 (Orange), the map gives none", problems)
        self.assertIn("element 22: not in KIT_BINDINGS, the map gives 4", problems)
        self.assertEqual(problems, [
            "Kick (element 7): expected action 4 (Orange), the map gives none",
            "StartPad (element 18): expected action 16 (Start), the map gives none",
            "SelectPad (element 19): expected action 6 (Select), the map gives none",
            "element 20: not in KIT_BINDINGS, the map gives 16",
            "element 21: not in KIT_BINDINGS, the map gives 6",
            "element 22: not in KIT_BINDINGS, the map gives 4"])

    def test_a_button_binding_on_the_kick_is_refused_naming_button_and_definition_1(self):
        button = [{**b, "element_type": 1} if b["element_id"] == 7 else b for b in BINDINGS]
        tools = self.tools([STATUS, setup_reply(button), TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("refused: the kit's bindings differ from KIT_BINDINGS:", lines)
        self.assertIn(
            "  Kick (element 7): the map binds it as a button (element type 1 (button)), but "
            "the kit is built from definition 1 (MIDI Drums), whose elements are all axes, so "
            "a button binding is another definition's map", lines)
        self.assertNotIn("bindings: all 10 match KIT_BINDINGS", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIsNone(self.game.kit_source())

    def test_every_element_is_type_checked_not_only_the_kick(self):
        button = [{**b, "element_type": 1} if b["element_id"] in (0, 18) else b
                  for b in BINDINGS]
        problems = K.binding_problems(setup_reply(button)["maps"])
        self.assertEqual([p.split(":")[0] for p in problems],
                         ["Snare (element 0)", "StartPad (element 18)"])
        self.assertTrue(all("binds it as a button" in p for p in problems))

    def test_a_binding_with_no_element_type_is_refused(self):
        untyped = [{k: v for k, v in b.items() if k != "element_type"} if b["element_id"] == 7
                   else b for b in BINDINGS]
        self.assertEqual(K.binding_problems(setup_reply(untyped)["maps"]), [
            "Kick (element 7): expected element type 0 (axis), the map gives None"])

    def test_an_element_bound_as_both_kinds_is_refused_as_a_button(self):
        both = BINDINGS + [{**BINDINGS[7], "element_type": 1}]
        self.assertEqual(K.binding_problems(setup_reply(both)["maps"]), [
            "Kick (element 7): the map binds it as a button (element type 0 (axis), "
            "1 (button)), but the kit is built from definition 1 (MIDI Drums), whose "
            "elements are all axes, so a button binding is another definition's map"])

    def test_a_setup_with_no_reply_ends_at_its_cap_and_tears_down(self):
        tools = self.tools([STATUS, None, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("setup failed: no reply to the setup request within the cap of 10 s "
                      "(KIT_SETUP_CAP_S)", lines)
        self.assertEqual(self.posted()[-1]["type"], "teardown")
        self.assertEqual(lines[-1], "kit agent: detached")


class SetupForRunTest(Rig):
    def test_a_setup_that_passes_gives_the_press_and_the_reply(self):
        tools = self.tools([STATUS, setup_reply()])
        press, text = tools.setup_for_run("System")
        self.assertTrue(callable(press))
        self.assertEqual(self.posted()[1]["player"], "System")
        self.assertIn("bindings: all 10 match KIT_BINDINGS", text)

    def test_a_setup_that_is_refused_gives_no_press(self):
        press, text = self.tools(attached=False).setup_for_run("System")
        self.assertIsNone(press)
        self.assertEqual(text, f'{K.given_player_line("System")}\n{FT.NOT_ATTACHED}')

    def test_a_setup_on_the_wrong_id_is_torn_down_and_gives_no_press(self):
        tools = self.tools([STATUS, setup_reply(kit_id=7), TEARDOWN_OK])
        press, text = tools.setup_for_run("System")
        self.assertIsNone(press)
        self.assertIn("refused: the kit is controller 7", text)
        self.assertEqual(self.posted()[-1]["type"], "teardown")

    def test_a_kit_already_set_up_gives_no_press_and_stays(self):
        tools = self.set_up()
        press, text = tools.setup_for_run("System")
        self.assertIsNone(press)
        self.assertIn(K.KIT_ALREADY_SET_UP, text)
        self.assertIsNotNone(tools.kit_press())


class MapProblemsTest(Rig):
    def drum_map(self, **changes):
        return {**setup_reply()["maps"][0], **changes}

    def test_no_map_names_the_ids(self):
        # LS-D1 run 1's reply on System: "maps": [].
        self.assertEqual(K.map_problems([]), [
            "expected one enabled map at category 1, layout 2; "
            "the setup reply holds no map for the kit"])

    def test_a_map_at_layout_0_is_refused_naming_both(self):
        self.assertEqual(K.map_problems([self.drum_map(layout_id=0)]), [
            "expected one enabled map at category 1, layout 2; "
            "the setup reply holds 0 such maps among 1:",
            "  map: category 1, layout 0, enabled"])

    def test_the_right_map_disabled_is_refused(self):
        self.assertEqual(K.map_problems([self.drum_map(enabled=False)]), [
            "expected one enabled map at category 1, layout 2; "
            "the setup reply holds 0 such maps among 1:",
            "  map: category 1, layout 2, disabled"])

    def test_two_enabled_maps_at_the_ids_are_refused(self):
        self.assertEqual(K.map_problems([self.drum_map(), self.drum_map()]), [
            "expected one enabled map at category 1, layout 2; "
            "the setup reply holds 2 such maps among 2:",
            "  map: category 1, layout 2, enabled",
            "  map: category 1, layout 2, enabled"])

    def test_the_drum_map_passes(self):
        self.assertEqual(K.map_problems(setup_reply()["maps"]), [])

    def test_setup_on_system_with_no_map_is_refused_and_torn_down(self):
        reply = setup_reply()
        reply["kit"] = {**reply["kit"], "player": "System"}
        reply["maps"] = []
        tools = self.tools([STATUS, reply, TEARDOWN_OK])
        lines = tools.ch_kit_setup(player="System").splitlines()
        at = lines.index("refused: the kit's map is not the one at category 1, layout 2:")
        self.assertEqual(lines[at + 1], "  expected one enabled map at category 1, layout 2; "
                                        "the setup reply holds no map for the kit")
        self.assertNotIn("refused: the kit's bindings differ from KIT_BINDINGS:", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIn("kit teardown: all checks ok", lines)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())


class SetupPlayerTest(Rig):
    def setup_posted(self):
        return [m for m in self.posted() if m["type"] == "setup"]

    def test_no_player_sends_player1(self):
        tools = self.tools([STATUS, setup_reply()])
        lines = tools.ch_kit_setup().splitlines()
        self.assertEqual(self.setup_posted(), [
            {"type": "setup", "tag": "hydra-kit", "definition_id": 1, "player": "Player1",
             "category_id": 1, "layout_id": 2, "placeholder_tags": PLACEHOLDER_TAGS}])
        self.assertEqual(lines[0], BACKUP_LINE)

    def test_a_given_player_is_sent_as_is_and_named_first(self):
        reply = setup_reply()
        reply["kit"] = {**reply["kit"], "player": "System"}
        tools = self.tools([STATUS, reply])
        lines = tools.ch_kit_setup(player="System").splitlines()
        self.assertEqual(self.setup_posted(), [
            {"type": "setup", "tag": "hydra-kit", "definition_id": 1, "player": "System",
             "category_id": 1, "layout_id": 2, "placeholder_tags": PLACEHOLDER_TAGS}])
        self.assertEqual(lines[0], 'kit player: "System" as given (KIT_PLAYER is Player1)')
        self.assertEqual(lines[1], BACKUP_LINE)
        self.assertIn("kit: controller 6, definition 1 (MIDI Drums), tag hydra-kit, created, "
                      "on System", lines)
        self.assertEqual(lines[-1], "bindings: all 10 match KIT_BINDINGS")

    def test_a_name_no_player_has_is_printed_as_the_agents_refusal(self):
        error = {"error": "setup failed: Error: no player named Player9; players: System, Player1"}
        tools = self.tools([STATUS, error, TEARDOWN_OK])
        lines = tools.ch_kit_setup(player="Player9").splitlines()
        self.assertEqual(self.setup_posted()[0]["player"], "Player9")
        self.assertEqual(lines[0], 'kit player: "Player9" as given (KIT_PLAYER is Player1)')
        self.assertIn("setup failed: the agent sent an error: setup failed: Error: no player "
                      "named Player9; players: System, Player1", lines)
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertEqual(lines[-1], "kit agent: detached")
        self.assertIsNone(self.game.kit_source())

    def test_a_faulted_setup_prints_the_fault_then_tears_down(self):
        tools = self.tools([STATUS, SETUP_FAULT, TEARDOWN_OK])
        lines = tools.ch_kit_setup(player="System").splitlines()
        at = lines.index("setup failed: the agent sent an error: setup failed: "
                         "Error: access violation accessing 0x58")
        self.assertEqual(lines[at + 1:at + 8], [
            "fault: access-violation at GameAssembly.dll+0x1f557f2 (0x7ffb21f557f2), "
            "read of 0x58",
            "registers: rax=0x0 rbx=0x1c2a0 rcx=0x18 rdx=0x2 rsi=0x0 rdi=0x0 rbp=0x0 "
            "rsp=0xaff0 r8=0x0 r9=0x0 r10=0x0 r11=0x0 r12=0x0 r13=0x0 r14=0x0 r15=0x0 "
            "rip=0x7ffb21f557f2 pc=0x7ffb21f557f2 sp=0xaff0",
            "stack:",
            "  Error: access violation accessing 0x58",
            "  at invokeRaw (method.ts:234)",
            "  at setup (kit.ts:308)",
            "kit teardown: elements cleared: ok (2 cleared)",
        ])
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertIn("kit teardown: all checks ok", lines)
        self.assertEqual(lines[-1], "kit agent: detached")

    def test_a_fault_outside_every_module_prints_a_readable_line(self):
        fault = {**SETUP_FAULT["fault"], "module": None, "offset": None,
                 "address": "58", "operation": None, "memory_address": None,
                 "registers": {}, "stack": []}
        tools = self.tools([STATUS, {**SETUP_FAULT, "fault": fault}, TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("fault: access-violation at 0x58 (in no module), "
                      "no memory access recorded", lines)
        self.assertIn("registers: none sent", lines)
        self.assertNotIn("stack:", lines)
        self.assertIn("kit teardown: all checks ok", lines)

    def test_an_error_with_no_fault_prints_no_fault_lines(self):
        error = {"error": "setup failed: Error: no player named Player9; players: System, Player1"}
        tools = self.tools([STATUS, error, TEARDOWN_OK])
        lines = tools.ch_kit_setup(player="Player9").splitlines()
        at = lines.index("setup failed: the agent sent an error: setup failed: Error: no player "
                         "named Player9; players: System, Player1")
        self.assertEqual(lines[at + 1], "kit teardown: elements cleared: ok (2 cleared)")
        self.assertFalse([line for line in lines if line.startswith(("fault:", "registers:"))])

    def test_a_refusal_before_the_attach_still_names_the_given_player(self):
        tools = self.tools(attached=False)
        self.assertEqual(tools.ch_kit_setup(player="System"),
                         f'kit player: "System" as given (KIT_PLAYER is Player1)\n{FT.NOT_ATTACHED}')
        self.assertEqual(self.events, [])


class PressTest(Rig):
    def test_green_and_kick_send_elements_3_and_7_with_the_hold(self):
        tools = self.set_up([PRESS_REPLY])
        reply = tools.ch_kit_press("green,kick")
        self.assertEqual(self.posted()[-1], {
            "type": "press", "elements": [{"id": 3, "value": 1.0}, {"id": 7, "value": 1.0}],
            "hold_frames": 2, "cap_s": 1, "rewrite": False, "on_main": False})
        # True equals 1.0 in Python, so the kick's type is read on its own:
        # kit.ts writes a number with its axis write and true with its button
        # write.
        self.assertIs(type(self.posted()[-1]["elements"][1]["value"]), float)
        self.assertEqual(reply, "pressed green, kick (elements 3, 7) for 2 frames: down at frame "
                                "1200, up at frame 1202 (2 frames apart, 13 ms by the agent's clock)")

    def test_a_given_hold_is_sent(self):
        tools = self.set_up([PRESS_REPLY])
        self.assertTrue(tools.press_items(["kit:start"], hold_frames=5).sent)
        self.assertEqual(self.posted()[-1]["hold_frames"], 5)
        self.assertEqual(self.posted()[-1]["elements"], [{"id": 18, "value": 1.0}])
        self.assertIs(type(self.posted()[-1]["elements"][0]["value"]), float)

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
            "cap_s": 1, "rewrite": False, "on_main": False, "read": ["Green"]})
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
        tools = self.set_up([PRESS_REPLY, {**PRESS_REPLY, "rewrite": True, "on_main": True}])
        reply = tools.ch_kit_press("green")
        self.assertEqual((self.posted()[-1]["rewrite"], self.posted()[-1]["on_main"]),
                         (K.KIT_REWRITE, K.KIT_PRESS_ON_MAIN))
        self.assertNotIn("read", self.posted()[-1])
        self.assertEqual(len(reply.splitlines()), 1)
        lines = tools.ch_kit_press("green", hold_frames=1, rewrite=True,
                                   on_main=True).splitlines()
        self.assertEqual((self.posted()[-1]["rewrite"], self.posted()[-1]["on_main"],
                          self.posted()[-1]["hold_frames"]), (True, True, 1))
        self.assertEqual(lines[1], "rewrite on, main thread on (as the agent ran it)")

    def test_rewrite_and_on_main_take_only_true_or_false(self):
        tools = self.set_up()
        self.assertEqual(tools.ch_kit_press("green", rewrite="no"),
                         "rewrite is true, false or left out, not 'no'")
        self.assertEqual(tools.ch_kit_press("green", on_main=1),
                         "on_main is true, false or left out, not 1")


class PadAxisTest(unittest.TestCase):
    """pad_axis against the byte the game makes of an axis, in
    code/6142/lane_velocity_337E10.asm."""

    @staticmethod
    def byte(axis):
        return round((axis - 0.5) * 254)

    def test_an_accent_and_a_plain_gem_press_at_full_strength_for_any_threshold(self):
        for threshold in (1, 64, 127):
            self.assertEqual(K.pad_axis("accent", threshold), 1.0)
            self.assertEqual(K.pad_axis("none", threshold), 1.0)

    def test_a_plain_or_accent_gem_needs_no_threshold(self):
        self.assertEqual(K.pad_axis("accent"), 1.0)
        self.assertEqual(K.pad_axis("none"), 1.0)

    def test_a_ghost_makes_a_byte_of_the_threshold_minus_one(self):
        self.assertEqual(self.byte(K.pad_axis("ghost", 1)), 0)
        self.assertEqual(self.byte(K.pad_axis("ghost", 64)), 63)
        self.assertEqual(self.byte(K.pad_axis("ghost", 127)), 126)

    def test_a_threshold_with_no_ghost_strength_is_refused_naming_the_field(self):
        for threshold in (0, 128, None):
            with self.assertRaises(ValueError) as caught:
                K.pad_axis("ghost", threshold)
            self.assertIn("OFF_DYN_THRESHOLD", str(caught.exception))
            self.assertIn(str(threshold), str(caught.exception))

    def test_a_word_hydra_replay_never_writes_is_refused(self):
        with self.assertRaises(ValueError) as caught:
            K.pad_axis("loud", 64)
        self.assertIn('"loud"', str(caught.exception))


class StrengthTest(Rig):
    def test_a_mixed_chord_carries_one_strength_per_lane_the_kick_included(self):
        ghost, accent = K.pad_axis("ghost", 64), K.pad_axis("accent", 64)
        message = K.press_message(
            [K.lane_element(Lane.RED), K.lane_element(Lane.YELLOW_CYMBAL),
             K.lane_element(Lane.KICK)],
            values=[accent, accent, ghost])
        self.assertEqual(message["elements"], [{"id": 0, "value": accent},
                                               {"id": 4, "value": accent},
                                               {"id": 7, "value": ghost}])
        self.assertNotEqual(ghost, accent)

    def test_a_red_ghost_and_a_kick_ghost_give_elements_0_and_7_at_the_ghost_value(self):
        ghost = K.pad_axis("ghost", 100)
        message = K.press_message([K.lane_element(Lane.RED), K.lane_element(Lane.KICK)],
                                  values=[ghost, ghost])
        self.assertEqual(message["elements"], [{"id": 0, "value": ghost},
                                               {"id": 7, "value": ghost}])

    def test_a_kick_with_no_value_gives_element_7_at_the_plain_value(self):
        message = K.press_message([K.lane_element(Lane.KICK)])
        self.assertEqual(message["elements"], [{"id": 7, "value": K.pad_axis("none")}])
        self.assertEqual(K.pad_axis("none"), 1.0)

    def test_start_and_select_press_at_full_whatever_value_is_given(self):
        message = K.press_message([K.KIT_ELEMENTS["start"], K.KIT_ELEMENTS["select"]],
                                  values=[0.6, 0.6])
        self.assertEqual(message["elements"], [{"id": 18, "value": 1.0},
                                               {"id": 19, "value": 1.0}])

    def test_no_press_message_writes_true(self):
        # Every element alone and all of them together, with no values, a
        # given value and each end, in the first group and in a later one.
        groups = [[e] for e in K.KIT_ELEMENTS.values()] + [list(K.KIT_ELEMENTS.values())]
        for group in groups:
            for values in (None, [0.6] * len(group), [0] * len(group), [1] * len(group)):
                for message in (K.press_message(group, values=values),
                                K.press_message([], then=[(group, values, 8.0)])):
                    written = message["elements"] + [
                        e for g in message.get("then", []) for e in g["elements"]]
                    self.assertTrue(written)
                    for element in written:
                        self.assertNotIsInstance(element["value"], bool, (group, values))

    def test_a_press_with_no_values_presses_pads_as_plain_gems(self):
        message = K.press_message([K.KIT_ELEMENTS["red"]])
        self.assertEqual(message["elements"], [{"id": 0, "value": K.pad_axis("none")}])

    def test_a_list_of_values_of_another_length_is_an_error(self):
        with self.assertRaises(ValueError):
            K.press_message([K.KIT_ELEMENTS["red"]], values=[0.6, 0.7])

    def test_two_groups_carry_then_with_after_ms_8(self):
        message = K.press_message(
            [K.lane_element(Lane.KICK)], 2,
            then=[([K.lane_element(Lane.GREEN_CYMBAL)], [0.6], 8.0)])
        self.assertEqual(message["elements"], [{"id": 7, "value": K.pad_axis("none")}])
        self.assertEqual(message["then"], [{"elements": [{"id": K.lane_element(
            Lane.GREEN_CYMBAL).id, "value": 0.6}], "after_ms": 8.0}])
        self.assertNotIn("then", K.press_message([K.lane_element(Lane.KICK)]))

    def test_a_bad_later_group_is_refused_before_anything_is_sent(self):
        red = K.lane_element(Lane.RED)
        with self.assertRaisesRegex(ValueError, "are in two groups of one press"):
            K.press_message([red], then=[([red], None, 8.0)])
        for bad in (0, -1.0, True, "8"):
            with self.assertRaisesRegex(ValueError, "after_ms is a number above 0"):
                K.press_message([red], then=[([K.lane_element(Lane.KICK)], None, bad)])

    def test_the_kit_refuses_a_repeated_element(self):
        red = K.lane_element(Lane.RED)
        for message in (lambda: K.press_message([red, K.lane_element(Lane.KICK), red]),
                        lambda: K.press_message([K.lane_element(Lane.KICK)],
                                                then=[([red, red], None, 8.0)])):
            with self.assertRaises(ValueError) as caught:
                message()
            self.assertIn("kit element Snare is twice in one group", str(caught.exception))
        tools = self.set_up([PRESS_REPLY])
        press = self.game.kit_source()
        sent = len(self.posted())
        with self.assertRaises(ValueError):
            press([Lane.RED, Lane.RED])
        self.assertEqual(len(self.posted()), sent)
        self.assertIsNotNone(tools)

    def test_the_fakes_send_the_real_reply_shape(self):
        # test_play_chart's FakeReplyShapeTest checks kit_reply against kit.ts.
        self.assertIsNone(K.reply_gap(PRESS_REPLY, 1))
        self.assertIsNone(K.reply_gap(SPLIT_PRESS_REPLY, 2))
        self.assertEqual(K.reply_gap(PRESS_REPLY, 2),
                         "has no `groups` list, though the press sent 2 groups")

    def test_ch_kit_press_fails_on_a_reply_without_its_frames(self):
        tools = self.set_up([{k: v for k, v in PRESS_REPLY.items() if k != "up_frame"}])
        self.assertIn("press failed: the kit's reply to the press has no up_frame",
                      tools.ch_kit_press("red"))

    def test_the_kit_press_sends_a_split_chord_in_one_message(self):
        self.set_up([SPLIT_PRESS_REPLY])
        press = self.game.kit_source()
        sent = len(self.posted())
        press([Lane.KICK], None, then=[([Lane.GREEN_CYMBAL], None, 8.0)])
        self.assertEqual(len(self.posted()), sent + 1)
        self.assertEqual(self.posted()[-1]["then"][0]["after_ms"], 8.0)

    def test_the_kit_press_sends_the_chords_axes(self):
        self.set_up([PRESS_REPLY])
        press = self.game.kit_source()
        ghost = K.pad_axis("ghost", 64)
        press([Lane.RED, Lane.KICK], [K.pad_axis("none", 64), ghost])
        self.assertEqual(self.posted()[-1]["elements"],
                         [{"id": 0, "value": K.pad_axis("none", 64)}, {"id": 7, "value": ghost}])

    def test_ch_kit_press_sends_a_given_value_to_the_pads_and_kick_and_full_to_start(self):
        tools = self.set_up([PRESS_REPLY])
        lines = tools.ch_kit_press("red,kick,start", value=0.6).splitlines()
        self.assertEqual(self.posted()[-1]["elements"],
                         [{"id": 0, "value": 0.6}, {"id": 7, "value": 0.6},
                          {"id": 18, "value": 1.0}])
        self.assertEqual(lines[1], "pads and kick pressed at value 0.6")

    def test_a_value_at_either_end_is_sent(self):
        tools = self.set_up([PRESS_REPLY, PRESS_REPLY])
        for value in (0, 1):
            self.assertTrue(tools.press_items("red", value=value).sent)
            self.assertEqual(self.posted()[-1]["elements"], [{"id": 0, "value": value}])

    def test_a_value_outside_0_to_1_is_refused_and_nothing_is_sent(self):
        tools = self.set_up()
        sent = len(self.posted())
        for bad in (1.5, -0.1, float("nan"), True, "0.6"):
            self.assertEqual(tools.ch_kit_press("red", value=bad),
                             f"value is a number from 0 to 1 or left out, not {bad!r}")
        self.assertEqual(len(self.posted()), sent)


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


class FrameProbeTest(Rig):
    """ch_kit_frame_probe, LS-G1's probe (task IG3 of
    docs/superpowers/plans/2026-11-12-in-game-press-timing.md)."""

    PROBE_REPLY = {"frames": [{"frame": 1}, {"frame": 2}, {"frame": 3}], "pressed_frame": 2}

    def test_the_probe_cap_is_the_users_five_seconds(self):
        # D339 item 3: the frame probe of LS-G1 runs 5 s.
        self.assertEqual(K.FRAME_PROBE_SECONDS, 5.0)

    def test_the_probe_sends_frame_probe_with_the_engines_clock_address(self):
        tools = self.set_up([self.PROBE_REPLY])
        lines = tools.ch_kit_frame_probe(5).splitlines()
        self.assertEqual(self.posted()[-1], {
            "type": "frame_probe", "seconds": 5, "clock_addr": self.CLOCK_ADDR,
            "elements": [{"id": 3, "value": 1.0}], "read": ["Green"], "hold_frames": 2,
            "cap_s": 6})
        self.assertEqual(lines, [f"frame probe on the engine clock at {self.CLOCK_ADDR:#x}",
                                 "frames: 3 entries", "pressed_frame: 2"])
        self.assertEqual(self.events.count("find clock"), 1)

    def test_more_than_five_seconds_is_refused_before_anything_is_sent(self):
        tools = self.set_up()
        sent = len(self.posted())
        self.assertEqual(tools.ch_kit_frame_probe(5.001),
                         "seconds is at most 5 (FRAME_PROBE_SECONDS), not 5.001")
        self.assertEqual(tools.ch_kit_frame_probe(60),
                         "seconds is at most 5 (FRAME_PROBE_SECONDS), not 60")
        for bad in (0, -1, True, "5", float("nan")):
            self.assertEqual(tools.ch_kit_frame_probe(bad),
                             f"seconds is a number above 0, not {bad!r}")
        self.assertEqual(len(self.posted()), sent)
        self.assertNotIn("find clock", self.events)

    def test_refused_before_setup_and_while_a_play_session_runs(self):
        tools = self.tools()
        self.assertEqual(tools.ch_kit_frame_probe(5), K.NOT_SET_UP)
        tools = self.set_up()
        sent = len(self.posted())
        self.game.play = {"last_row": None, "press_count": 0, "stop": None}
        self.assertEqual(tools.ch_kit_frame_probe(5), K.PLAY_RUNNING)
        self.assertEqual(len(self.posted()), sent)
        self.assertNotIn("find clock", self.events)

    def test_no_engine_found_is_said_and_nothing_is_sent(self):
        tools = self.set_up()
        sent = len(self.posted())

        def no_engine():
            raise RuntimeError("no song is playing")

        tools._find_clock = no_engine
        self.assertEqual(tools.ch_kit_frame_probe(5),
                         "frame probe: no engine found: RuntimeError: no song is playing")
        self.assertEqual(len(self.posted()), sent)

    def test_a_probe_with_no_reply_ends_at_its_cap(self):
        tools = self.set_up([None])
        self.assertEqual(tools.ch_kit_frame_probe(2.5),
                         "frame probe failed: no reply to the frame_probe request within the "
                         "cap of 3.5 s (seconds plus KIT_PRESS_CAP_S)")


class FakeListenAgent:
    """The frida_attach.LiveAgent parts GamePlayer uses: listen, alive and
    request."""

    def __init__(self):
        self.listener = None
        self.listened = []
        self.alive = True
        self.requests = []

    def listen(self, listener):
        self.listened.append(listener)
        self.listener = listener

    def request(self, message, cap_s):
        self.requests.append((message, cap_s))
        return {"ok": True}


class GamePlayerTest(unittest.TestCase):
    def test_events_queue_in_the_order_the_agent_sent_them(self):
        agent = FakeListenAgent()
        player = K.GamePlayer(agent)
        first = {"kind": K.EVENT_SKIPPED, "seqs": [], "count": 0, "clock_ms": 0.0}
        batch = [{"kind": K.EVENT_SENT, "seq": 0}, {"kind": K.EVENT_DROPPED, "seq": 1}]
        last = {"kind": K.EVENT_STOP, "reason": K.STOP_REASON_DONE}
        agent.listener(first)
        agent.listener({"type": K.PLAY_EVENTS_TYPE, "events": batch})
        agent.listener({"type": "log", "line": "not an event"})
        agent.listener(last)
        got = []
        while not player.events.empty():
            got.append(player.events.get_nowait())
        self.assertEqual(got, [first, *batch, last])

    def test_a_batch_with_an_error_queues_the_error_after_its_events(self):
        agent = FakeListenAgent()
        player = K.GamePlayer(agent)
        agent.listener({"type": K.PLAY_EVENTS_TYPE, "events": [],
                        "error": "the stall check failed: boom"})
        self.assertEqual(player.events.get_nowait(),
                         {"kind": K.EVENT_ERROR, "error": "the stall check failed: boom"})
        self.assertTrue(player.events.empty())

    def test_post_waits_the_press_cap_and_close_stops_the_listener(self):
        agent = FakeListenAgent()
        player = K.GamePlayer(agent)
        message = K.stop_play_message()
        self.assertEqual(player.post(message), {"ok": True})
        self.assertEqual(agent.requests, [(message, K.KIT_PRESS_CAP_S)])
        self.assertTrue(player.alive())
        agent.alive = False
        self.assertFalse(player.alive())
        player.close()
        self.assertIsNone(agent.listened[-1])


class GamePlayerFromKitTest(Rig):
    def test_the_kit_tools_player_is_none_until_setup_and_then_listens(self):
        self.assertIsNone(self.tools().game_player())
        tools = self.set_up()
        player = tools.game_player()
        self.assertIsInstance(player, K.GamePlayer)
        self.assertIsNotNone(tools._agent._listener)
        player.close()
        self.assertIsNone(tools._agent._listener)


class KitPressTest(Rig):
    def test_the_kit_press_reply_rides_into_the_press_row_and_the_log(self):
        tools = self.set_up([PRESS_REPLY])
        press = self.game.kit_source()
        wall = _FakeTime()
        lanes, dynamics = [Lane.GREEN, Lane.KICK], ["none", "none"]
        chords = [play_chart.Chord(7, 0, 1000.0, lanes, dynamics, presses=(play_chart.Press(
            index=7, chart_ms=1000.0, aim_ms=-2.0, lanes=lanes, dynamics=dynamics, axes=None,
            declined=False, last_send_ms=1050.0, rule="plain"),))]
        # The start read, the fresh read before the press, the read after it.
        score = iter([0, 0, 50])
        result = play_chart.play_chords(
            chords, _FakeSongClock(wall, start_s=0.5), lambda: next(score), press,
            now=wall.now, sleep=wall.sleep, out=io.StringIO(), kit=True)
        self.assertEqual(result.stop, "done")
        self.assertEqual(self.posted()[-1]["elements"],
                         [{"id": 3, "value": 1.0}, {"id": 7, "value": 1.0}])
        row = result.rows[0]
        self.assertEqual((row["down_frame"], row["up_frame"], row["down_ms"], row["up_ms"]),
                         (1200, 1202, 1762600005000, 1762600005013))
        self.assertNotIn("rewrite", row)
        path = os.path.join(self.tmp, "press.csv")
        play_chart.write_press_log(path, result.rows)
        with open(path, encoding="utf-8", newline="") as f:
            table = list(csv.DictReader(f))
        self.assertEqual((table[0]["down_frame"], table[0]["up_ms"]), ("1200", "1762600005013"))
        self.assertIsNotNone(tools.kit_press())

    def test_a_faulted_kit_press_carries_the_fault_in_its_error(self):
        self.set_up([{**SETUP_FAULT, "error": "press failed: Error: access violation accessing 0x58"}])
        press = self.game.kit_source()
        with self.assertRaises(K.FA.ReadFailed) as caught:
            press([Lane.GREEN])
        lines = str(caught.exception).splitlines()
        self.assertEqual(lines[0], "the agent sent an error: press failed: "
                                   "Error: access violation accessing 0x58")
        self.assertEqual(lines[1], "fault: access-violation at GameAssembly.dll+0x1f557f2 "
                                   "(0x7ffb21f557f2), read of 0x58")
        self.assertEqual(caught.exception.reason, K.FA.END_AGENT_ERROR)
        self.assertEqual(caught.exception.reply["fault"]["type"], "access-violation")

    def test_a_kit_press_error_with_no_fault_is_raised_unchanged(self):
        self.set_up([{"error": "press failed: boom"}])
        with self.assertRaises(K.FA.ReadFailed) as caught:
            self.game.kit_source()([Lane.GREEN])
        self.assertEqual(str(caught.exception), "the agent sent an error: press failed: boom")

    def test_no_kit_press_before_setup(self):
        self.tools()
        self.assertIsNone(self.game.kit_source())


class StatusTest(Rig):
    def test_the_frame_rate_comes_from_two_reads_one_second_apart(self):
        later = {**STATUS, "frame": 1160, "keyboard_now": False, "mouse_now": False,
                 "kit_exists": True, "kit_id": 7,
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
            "Keyboard.enabled: off",
            "Mouse.enabled: off",
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
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
        self.assertEqual(lines, [
            "kit teardown: elements cleared: ok (2 cleared)",
            "kit teardown: kit gone: ok (no controller tagged hydra-kit)",
            "kit teardown: keyboard: ok (Keyboard.enabled true (was true))",
            "kit teardown: mouse: ok (Mouse.enabled true (was true))",
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
        self.assertIn("kit teardown failed on keyboard: Keyboard.enabled false (was true)", lines)
        self.assertIn("kit teardown: mouse: ok (Mouse.enabled true (was true))", lines)
        self.assertIn("kit teardown: NOT all checks ok; see the lines above", lines)

    def test_a_mouse_mismatch_is_printed_as_a_failure(self):
        tools = self.set_up([TEARDOWN_MOUSE_MISMATCH])
        lines = tools.ch_kit_teardown().splitlines()
        self.assertEqual(lines, [
            "kit teardown: keyboard: ok (Keyboard.enabled true (was true))",
            "kit teardown failed on mouse: Mouse.enabled false (was true)",
            "kit teardown: switch: ok (ignoreInputWhenAppNotInFocus true (was true))",
            "kit teardown: NOT all checks ok; see the lines above",
            "kit agent: detached"])

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


class KitIdTest(Rig):
    """The kit must hold the id the gate asks for; any other is refused."""

    def test_a_kit_at_id_6_passes_and_the_reply_names_all_seven_ids(self):
        tools = self.tools([STATUS, setup_reply()])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("placeholders: controllers 0, 1, 2, 3, 4, 5 (6 created)", lines)
        self.assertIn("kit: controller 6, definition 1 (MIDI Drums), tag hydra-kit, created, "
                      "on Player1", lines)
        self.assertEqual(lines[-1], "bindings: all 10 match KIT_BINDINGS")
        self.assertIsNotNone(self.game.kit_source())

    def test_a_kit_at_any_other_id_is_refused_with_the_ids_and_torn_down(self):
        for kit_id in (0, 7):
            with self.subTest(kit_id=kit_id):
                tools = self.tools([STATUS, setup_reply(kit_id=kit_id), TEARDOWN_OK])
                lines = tools.ch_kit_setup().splitlines()
                self.assertIn(f"refused: the kit is controller {kit_id}, not "
                              "KIT_REQUIRED_ID 6; placeholder ids: 0, 1, 2, 3, 4, 5", lines)
                self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)
                self.assertIn("kit teardown: all checks ok", lines)
                self.assertEqual(lines[-1], "kit agent: detached")
                self.assertIsNone(self.game.kit_source())

    def test_a_reply_with_no_kit_id_is_refused_too(self):
        reply = setup_reply()
        del reply["kit"]["id"]
        tools = self.tools([STATUS, reply, TEARDOWN_OK])
        self.assertIn("refused: the kit is controller None, not KIT_REQUIRED_ID 6",
                      tools.ch_kit_setup())
        self.assertEqual(self.posted()[-1], TEARDOWN_MESSAGE)

    def test_the_id_refusal_says_a_new_launch_puts_the_kit_back_on_id_6(self):
        tools = self.tools([STATUS, setup_reply(kit_id=7), TEARDOWN_OK])
        lines = tools.ch_kit_setup().splitlines()
        self.assertIn("only a new launch of the game puts the kit back on id 6: a "
                      "teardown does not reset the game's id counter, so a setup after "
                      "one gets a higher id (docs/clone-hero/07-internals.md, IN-81); "
                      "ch_stop, then ch_start", lines)

    def test_a_setup_on_a_kit_already_set_up_does_not_advise_a_teardown_and_setup(self):
        tools = self.set_up()
        posted = len(self.posted())
        said = tools.ch_kit_setup()
        self.assertEqual(said, "the kit is already set up. ch_kit_teardown takes it "
                         "away, but a setup after that in the same launch is refused, "
                         "because the kit would not be on id 6; start the game again "
                         "(ch_stop, then ch_start) to set it up again")
        self.assertNotIn("call ch_kit_teardown first", said)
        self.assertEqual(len(self.posted()), posted)


# The fake game for KitAgentTest. It reads one case from stdin, loads the
# built kit.js's own code (built_kit_module) with a fake il2cpp bridge, posts
# each message the way Frida would, and prints the replies, every create,
# destroy and AddController call, and the controllers left in the fake game.
# Ids come from a counter that destroying never moves, as the game's does
# (IN-81).
AGENT_DRIVER = """
import { readFileSync } from "node:fs";
const c = JSON.parse(readFileSync(0, "utf8"));
const calls = [];
const live = [];
let nextId = c.first_id ?? 0;
let creates = 0;
// With a case's frame_ms the frame count moves on one frame per frame_ms of
// Node's clock, and every kit write is kept with its frame and time.
const t0 = Date.now();
const frame = () => 1000 + (c.frame_ms ? Math.floor((Date.now() - t0) / c.frame_ms) : 0);
const writes = [];
const keep = (kind) => (id, value) => {
    writes.push([id, value, frame(), Date.now() - t0, kind]);
};
function controller(tag) {
    const made = obj({SetAxisValueById: keep("axis"), SetButtonValueById: keep("button")},
                     {id: nextId++});
    made.tag = tag;
    live.push(made);
    return made;
}
function obj(methods, fields = {}) {
    return {
        handle: 1,
        isNull: () => false,
        method(name) {
            if (!(name in methods)) { throw new Error(`the fake has no method ${name}`); }
            const m = {
                invoke: (...args) => methods[name](...args), overload: () => m,
                virtualAddress: {isNull: () => false}, parameters: [],
                returnType: {fridaAlias: "void"}, handle: 2,
            };
            return m;
        },
        field: (name) => ({value: fields[name]}),
    };
}
const list = (xs) => obj({get_Count: () => xs.length, get_Item: (i) => xs[i]});
const flag = (get, set, value) => {
    const state = {value};
    return {[get]: () => state.value, [set]: (v) => { state.value = v; }};
};
const device = () => obj(flag("get_enabled", "set_enabled", true));
const devices = {keyboard: device(), mouse: device()};
function player(id, name) {
    const maps = obj({LoadMap() {}, GetMaps: () => list([])});
    const pc = obj({
        ...flag("get_hasKeyboard", "set_hasKeyboard", true),
        ...flag("get_hasMouse", "set_hasMouse", false),
        AddController(ctl) { calls.push(["add", ctl.field("id").value, name]); },
        RemoveController(ctl) { calls.push(["remove", ctl.field("id").value, name]); },
    }, {maps});
    return obj({get_id: () => id, get_name: () => ({isNull: () => false, content: name})},
               {controllers: pc});
}
const players = [player(9999999, "System"), player(0, "Player1")];
const helper = obj({
    get_Keyboard: () => devices.keyboard,
    get_Mouse: () => devices.mouse,
    GetFirstCustomControllerWithTag: (s) => live.find((x) => x.tag === s.content) ?? null,
    CreateCustomController(definition, s) {
        creates += 1;
        calls.push(["create", definition, s.content]);
        if (creates === c.fail_on_create) { throw new Error("create failed"); }
        return controller(s.content);
    },
    DestroyCustomController(ctl) {
        const id = ctl.field("id").value;
        calls.push(["destroy", id]);
        if ((c.stuck ?? []).includes(id)) { return false; }
        live.splice(live.indexOf(ctl), 1);
        return true;
    },
});
const cfg = obj(flag("get_ignoreInputWhenAppNotInFocus",
                     "set_ignoreInputWhenAppNotInFocus", true));
const classes = {
    "Rewired.ReInput": obj({
        get_configuration: () => cfg, get_controllers: () => helper,
        get_players: () => obj({get_AllPlayers: () => list(players)}),
    }),
    "Rewired.ControllerType": obj({}, {Custom: 20}),
    "UnityEngine.Time": obj({get_frameCount: frame}),
    "UnityEngine.Application": obj({get_isFocused: () => false}),
};
for (const tag of c.already_there ?? []) { controller(tag); }
let handler = null;
let answered = null;
globalThis.Il2Cpp = {
    domain: {assembly: () => ({image: {class: (name) => classes[name]}})},
    string: (content) => ({content, isNull: () => false}),
    perform: async (fn) => fn(),
    toFridaValue: (v) => v,
    ValueType: class {},
};
globalThis.NativeFunction = function () { return () => {}; };
globalThis.NativePointer = class {};
globalThis.recv = (fn) => { handler = fn; };
globalThis.send = (reply) => { answered(reply); };
await import(%(kit_url)s);
const replies = [];
for (const [i, m] of c.messages.entries()) {
    for (let tries = 0; handler === null && tries < 1000; tries++) {
        await new Promise((go) => setImmediate(go));
    }
    const fn = handler;
    handler = null;
    replies.push(await new Promise((go) => { answered = go; fn({...m, id: i}); }));
}
console.log(JSON.stringify({replies, calls, writes, live: live.map((x) => x.tag)}));
"""
NODE = shutil.which("node")
# One driver run is a fresh Node start; this is its stop.
NODE_CAP_S = 30
# The built kit agent the game loads; the Node tests run this file, so they
# test what a run would load.
KIT_JS = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", K.KIT_AGENT_FILE)
# esbuild's comment line that opens kit.ts's own code in the bundle. What
# comes before it is the il2cpp bridge, which needs a real Frida; the driver
# sets the global the bridge defines.
KIT_CODE_MARKER = "// kit.ts\n"


def built_kit_module(js_path):
    """kit.ts's code as the built bundle holds it, the bridge left out."""
    text = BS.bundle_module(js_path)
    at = text.find(KIT_CODE_MARKER)
    if at < 0:
        raise BS.StampError(f"{js_path}: no {KIT_CODE_MARKER.strip()!r} line in the bundle")
    return text[at:]


def kit_skip_reason(js_path):
    """Why the Node tests cannot run the built file, or None when they can."""
    if NODE is None:
        return "node is not on PATH"
    refusal = BS.check(js_path)
    if refusal is not None:
        return f"the built kit agent may not run: {refusal}"
    return None


KIT_SKIP = kit_skip_reason(KIT_JS)


class KitBuiltFileTest(unittest.TestCase):
    """Which file KitAgentTest runs, and why it skips."""

    def test_the_kit_tests_run_the_built_file(self):
        self.assertEqual(os.path.basename(KIT_JS), "kit.js")
        with tempfile.TemporaryDirectory() as d:
            folder = os.path.join(d, "agent")
            js = make_built_agent(folder, "kit", module_text="var bridge = 1;\n"
                                  "// kit.ts\nvar ALLOWED_CALLS = [];\n")
            self.assertEqual(built_kit_module(js), "// kit.ts\nvar ALLOWED_CALLS = [];\n")
            if NODE is not None:
                self.assertIsNone(kit_skip_reason(js))
            with open(os.path.join(folder, "kit.ts"), "a", encoding="utf-8") as f:
                f.write("// an edit after the build\n")
            if NODE is not None:
                self.assertEqual(kit_skip_reason(js),
                                 f"the built kit agent may not run: {js} is stale: kit.ts "
                                 "changed since it was built; run npm run build:kit in "
                                 "tools/ch_mcp/agent first")
            os.remove(js)
            if NODE is not None:
                self.assertEqual(kit_skip_reason(js),
                                 f"the built kit agent may not run: {js} not found; run "
                                 "npm run build:kit in tools/ch_mcp/agent first")


@unittest.skipIf(KIT_SKIP is not None, str(KIT_SKIP))
class KitAgentTest(unittest.TestCase):
    """The built kit.js, run in Node against a fake game."""

    def run_agent(self, messages, **case):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        agent = os.path.join(tmp.name, "kit.mjs")
        with open(agent, "w", encoding="utf-8") as f:
            f.write(built_kit_module(KIT_JS))
        driver = os.path.join(tmp.name, "driver.mjs")
        with open(driver, "w", encoding="utf-8") as f:
            f.write(AGENT_DRIVER % {"kit_url": json.dumps(pathlib.Path(agent).as_uri())})
        done = subprocess.run([NODE, driver], input=json.dumps({"messages": messages, **case}),
                              capture_output=True, text=True, timeout=NODE_CAP_S)
        self.assertEqual(done.returncode, 0, done.stderr)
        return json.loads(done.stdout.splitlines()[-1])

    @staticmethod
    def of(out, kind):
        return [c[1:] for c in out["calls"] if c[0] == kind]

    @staticmethod
    def destroyed_checks(reply):
        return [c for c in reply["checks"] if c["what"].endswith(" destroyed")]

    def test_setup_makes_six_placeholders_then_the_kit_at_id_6(self):
        out = self.run_agent([K.setup_message("System")])
        reply = out["replies"][0]
        self.assertEqual(self.of(out, "create"),
                         [[1, tag] for tag in PLACEHOLDER_TAGS] + [[1, "hydra-kit"]])
        self.assertEqual(reply["placeholders"],
                         [{"id": i, "tag": tag} for i, tag in enumerate(PLACEHOLDER_TAGS)])
        self.assertEqual(reply["placeholders_created"], 6)
        self.assertEqual(reply["kit"]["id"], 6)
        self.assertIsNone(K.kit_id_problem(reply))

    def test_only_the_kit_is_handed_to_a_player(self):
        out = self.run_agent([K.setup_message("System")])
        self.assertEqual(self.of(out, "add"), [[6, "System"]])

    def test_teardown_destroys_all_seven_and_checks_each(self):
        out = self.run_agent([K.setup_message("System"), K.teardown_message()])
        reply = out["replies"][1]
        self.assertEqual(sorted(c[0] for c in self.of(out, "destroy")), list(range(7)))
        checks = self.destroyed_checks(reply)
        self.assertEqual(len(checks), 7)
        self.assertTrue(all(c["ok"] for c in checks), checks)
        self.assertEqual(out["live"], [])
        self.assertTrue(reply["all_ok"], reply)
        lines = K.teardown_lines(reply)
        self.assertEqual(len([s for s in lines if " destroyed: ok" in s]), 7)
        self.assertIn("kit teardown: hydra-placeholder-2 destroyed: ok "
                      "(controller 2: DestroyCustomController returned true)", lines)
        self.assertIn("kit teardown: kit gone: ok (no controller tagged hydra-kit or "
                      "a placeholder tag)", lines)

    def test_kit_gone_fails_when_a_placeholder_tag_remains(self):
        out = self.run_agent([K.setup_message("System"), K.teardown_message()], stuck=[3])
        reply = out["replies"][1]
        self.assertEqual(out["live"], ["hydra-placeholder-3"])
        self.assertFalse(reply["all_ok"])
        lines = K.teardown_lines(reply)
        self.assertIn("kit teardown failed on hydra-placeholder-3 destroyed: "
                      "controller 3: DestroyCustomController returned false", lines)
        self.assertIn("kit teardown failed on kit gone: controller 3 still tagged "
                      "hydra-placeholder-3", lines)

    def test_kit_gone_fails_when_the_kit_remains(self):
        out = self.run_agent([K.setup_message("System"), K.teardown_message()], stuck=[6])
        self.assertIn("kit teardown failed on kit gone: controller 6 still tagged hydra-kit",
                      K.teardown_lines(out["replies"][1]))

    def test_a_setup_that_throws_after_the_third_placeholder_leaves_three_to_destroy(self):
        out = self.run_agent([K.setup_message("System"), K.teardown_message()], fail_on_create=4)
        self.assertIn("create failed", out["replies"][0]["error"])
        reply = out["replies"][1]
        self.assertEqual(sorted(c[0] for c in self.of(out, "destroy")), [0, 1, 2])
        self.assertEqual(len(self.destroyed_checks(reply)), 3)
        self.assertEqual(out["live"], [])
        self.assertTrue(reply["all_ok"], reply)

    def test_a_setup_that_finds_the_kit_creates_nothing(self):
        out = self.run_agent([K.setup_message("System")],
                             already_there=[*PLACEHOLDER_TAGS, "hydra-kit"])
        reply = out["replies"][0]
        self.assertEqual(self.of(out, "create"), [])
        self.assertFalse(reply["kit"]["created"])
        self.assertEqual(reply["placeholders_created"], 0)
        self.assertEqual([p["id"] for p in reply["placeholders"]], list(range(6)))

    def test_a_second_setup_in_one_agent_creates_nothing_and_teardown_destroys_seven(self):
        out = self.run_agent([K.setup_message("System"), K.setup_message("System"),
                              K.teardown_message()])
        self.assertEqual(len(self.of(out, "create")), 7)
        self.assertEqual(len(out["replies"][1]["placeholders"]), 6)
        self.assertEqual(len(self.destroyed_checks(out["replies"][2])), 7)
        self.assertEqual(out["live"], [])

    def test_a_setup_after_a_throw_keeps_the_placeholders_it_finds(self):
        out = self.run_agent([K.setup_message("System"), K.setup_message("System")],
                             fail_on_create=4)
        self.assertEqual(self.of(out, "create").count([1, "hydra-placeholder-0"]), 1)
        self.assertEqual(out["replies"][1]["placeholders_created"], 3)
        self.assertEqual(len(out["replies"][1]["placeholders"]), 6)

    def test_a_later_group_goes_down_at_or_after_its_gap_and_each_holds_its_own_frames(self):
        # Design point 4 of docs/superpowers/plans/2026-11-07-player-limits.md,
        # question 2 (a): the gap is a floor. The fake frame is 3 ms.
        message = K.press_message([K.KIT_ELEMENTS["red"]], 2,
                                  then=[([K.KIT_ELEMENTS["kick"]], None, 8.0)])
        out = self.run_agent([K.setup_message("System"), message], frame_ms=3)
        reply = out["replies"][1]
        self.assertNotIn("error", reply)
        first, second = reply["groups"]
        self.assertEqual((reply["down_frame"], reply["down_ms"], reply["up_frame"]),
                         (first["down_frame"], first["down_ms"], first["up_frame"]))
        self.assertGreaterEqual(second["down_ms"] - first["down_ms"], 8)
        self.assertGreater(second["down_frame"], first["down_frame"])
        for group in (first, second):
            self.assertGreaterEqual(group["up_frame"] - group["down_frame"], 2)
        self.assertEqual(second["after_ms"], 8.0)
        writes = out["writes"]
        # Each element's first write is its down. Red's release may come
        # before the kick's down, so the writes are read per element.
        first_writes = {}
        for w in writes:
            first_writes.setdefault(w[0], w[1])
        self.assertEqual(first_writes, {0: K.pad_axis("none"), 7: K.pad_axis("none")})
        self.assertEqual(writes[0][:2], [0, K.pad_axis("none")])
        # Every element goes through the agent's axis write, down and up.
        self.assertEqual({w[4] for w in writes}, {"axis"})
        # Every element's last write is its release.
        last = {w[0]: w[1] for w in writes}
        self.assertEqual(last, {0: 0, 7: 0})

    def test_a_press_with_no_later_groups_replies_with_no_groups(self):
        out = self.run_agent([K.setup_message("System"),
                              K.press_message([K.KIT_ELEMENTS["red"]], 2)], frame_ms=3)
        reply = out["replies"][1]
        self.assertNotIn("groups", reply)
        self.assertGreaterEqual(reply["up_frame"] - reply["down_frame"], 2)

    def test_a_later_group_without_a_gap_is_an_error(self):
        message = {**K.press_message([K.KIT_ELEMENTS["red"]], 2),
                   "then": [{"elements": [{"id": 7, "value": 1.0}]}]}
        out = self.run_agent([K.setup_message("System"), message], frame_ms=3)
        self.assertIn('a press "then" group needs an "after_ms" above 0',
                      out["replies"][1]["error"])

    def test_a_teardown_with_no_setup_record_finds_the_controllers_by_tag(self):
        out = self.run_agent([K.teardown_message()],
                             already_there=[*PLACEHOLDER_TAGS, "hydra-kit"])
        self.assertEqual(len(self.destroyed_checks(out["replies"][0])), 7)
        self.assertEqual(out["live"], [])


class KitIdLiteralTest(unittest.TestCase):
    """The kit's id is read from the object; no code compares it with 0."""

    # A comparison of the kit's id with a bare 0, either way round, and a
    # truth test that would read id 0 as "no kit".
    KIT_ID = r"""(?:kitId\([^)]*\)|\bkit_id\b|\bkit\.id\b|kit\.get\(["']id["']\))"""
    COMPARE = r"(?:===|!==|==|!=|<=|>=|<|>)"
    ZERO_AGAINST_ID = re.compile(
        rf"{KIT_ID}\s*{COMPARE}\s*0\b|\b0\s*{COMPARE}\s*{KIT_ID}|\bnot\s+{KIT_ID}|!{KIT_ID}")

    def test_the_pattern_catches_each_form(self):
        for line in ("kitId(kit) === 0", "0 != kit_id", 'kit.get("id") == 0', "not kit_id",
                     "!kitId(kit)"):
            self.assertIsNotNone(self.ZERO_AGAINST_ID.search(line), line)
        self.assertIsNone(self.ZERO_AGAINST_ID.search("kitId(kit) === wanted"))

    def test_no_literal_0_is_compared_with_the_kits_id(self):
        for rel in (("agent", "kit.ts"), ("kit_tools.py",)):
            path = os.path.join(_REPO_ROOT, "tools", "ch_mcp", *rel)
            with open(path, encoding="utf-8") as f:
                found = [line for line in f if self.ZERO_AGAINST_ID.search(line)]
            self.assertEqual(found, [], path)


class ReadPauseSettingTest(unittest.TestCase):
    def test_reads_the_game_sections_line(self):
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, G.SETTINGS), "wb") as f:
                f.write(b"[audio]\r\npause_on_focus_lost = 7\r\n[game]\r\n"
                        b"pause_on_focus_lost = 0\r\nnote_speed = 7\r\n")
            self.assertEqual(K.read_pause_setting(d), "0")


if __name__ == "__main__":
    unittest.main()
