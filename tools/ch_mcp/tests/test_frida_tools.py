"""Tests for frida_tools: the attach waits and refusals, the state words, the
trace start and its refusals, ch_notes, ch_trace_stop and the stop hook.

No game and no Frida: the fake frida module and its sessions come from
test_frida_attach, the game side is a fake with the four methods FridaTools
calls, the trace is a fake standing in for trace_hits.TraceSession, and the
clock is a fake that sleeping moves.
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

from tools.ch_mcp import build_facts as B  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import trace_hits as TH  # noqa: E402
from tools.ch_mcp.tests.test_build_facts import REAL_FACTS  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    MENU, MENU_ONE_DESTROYED, STATE, FakeFrida, ReplySession, instant_wait)
from tools.ch_probe import constants as C  # noqa: E402

VERSION = "v1.1.0.6142-final"
FACTS = {
    "game_version": "1.1.0.6142-final",
    "signatures": {"drums_engine": {"name": "Eng", "ctor_rva": C.RVA_DRUMS_ENGINE_CTOR}},
    "classes": [{"name": "Eng", "methods": [
        {"name": "AddPoints", "rva": C.RVA_NOTE_SCORE_ADD},
        {"name": "Dynamics", "rva": C.RVA_NOTE_DYNAMICS},
        {"name": "Other", "rva": 0x1234},
    ]}],
}
BACKUP_LINE = "backup folder: X (taken now)"


class FakeGame:
    """game_tools.GameTools' five methods FridaTools calls."""

    def __init__(self, launched_at=None):
        self.launched = launched_at
        self.backups = 0
        self.hooks = []
        self.play = None
        self.folder = None

    def session_dir(self):
        return self.folder

    def ensure_backup(self):
        self.backups += 1
        return BACKUP_LINE if self.backups == 1 else ""

    def launched_at(self):
        return self.launched

    def add_stop_hook(self, hook):
        self.hooks.append(hook)

    def play_snapshot(self):
        return self.play


class FakeClock:
    def __init__(self, t=0.0):
        self.t = t
        self.slept = []

    def now(self):
        return self.t

    def sleep(self, s):
        self.slept.append(s)
        self.t += s


class FakeTrace:
    """Stands in for trace_hits.TraceSession: records how it was built and
    hands back the calls a test gives it."""

    def __init__(self, notes, engine_class, methods, out_path, **kw):
        self.notes = notes
        self.engine_class = engine_class
        self.methods = methods
        self.out_path = out_path
        self.kw = kw
        self.settings = {"song_cap_s": 36.0}
        self.cap_s = 156.0
        self.lines = []
        self.result = None
        self.call_list = []
        self.stop_caps = []
        self.ends_on_stop = True

    def start(self):
        return self

    def ended(self):
        return self.result is not None

    def stop(self, cap_s):
        self.stop_caps.append(cap_s)
        if self.ends_on_stop:
            self.result = FA.RunResult(reason=TH.END_STOPPED, cap_s=self.cap_s)
        return self.ended()

    def payloads(self):
        return list(self.call_list)

    def calls(self):
        return list(self.call_list)

    def lined_up(self, since=0):
        return TH.line_up(self.call_list[since:], self.notes)

    def ended_line(self):
        return "ended: stopped" if self.ended() else "ended: not yet"


def call(song_time_s):
    return {"call": "AddPoints", "names": ["AddPoints"], "songTime": song_time_s, "args": []}


class Rig(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.agent_dir = os.path.join(self.tmp, "agent")
        os.makedirs(self.agent_dir)
        for name in (FT.STATE_AGENT_FILE, os.path.basename(TH.DEFAULT_AGENT)):
            with open(os.path.join(self.agent_dir, name), "w", encoding="utf-8") as f:
                f.write("// fake agent\n")
        self.song = os.path.join(self.tmp, "song")
        os.makedirs(self.song)
        with open(TH.manifest_path(self.song), "w", encoding="utf-8") as f:
            json.dump({"notes": [{"index": i, "time_ms": t}
                                 for i, t in enumerate((1000, 2000, 3000))]}, f)
        self.clock = FakeClock()
        self.loads = 0
        self.traces = []

    def load_facts(self):
        self.loads += 1
        return FACTS

    def make_trace(self, *args, **kw):
        trace = FakeTrace(*args, **kw)
        self.traces.append(trace)
        return trace

    def tools(self, replies=(), game=None, load_facts=None, attached=False):
        self.game = game if game is not None else FakeGame()
        self.game.folder = self.tmp
        self.session = ReplySession(list(replies))
        self.frida = FakeFrida(self.session)
        tools = FT.FridaTools(
            self.game, threading.RLock(),
            frida_mod=self.frida, wait=instant_wait,
            current_build=lambda: VERSION,
            load_facts=load_facts or self.load_facts,
            trace_session=self.make_trace, agent_dir=self.agent_dir,
            now=self.clock.now, sleep=self.clock.sleep)
        # Detach before the temp folder goes, so Windows can delete the log.
        self.addCleanup(lambda: tools._agent is not None and tools._agent.detach())
        if attached:
            tools.ch_attach()
        return tools


class AttachTest(Rig):
    def test_attach_before_the_delay_waits_the_rest_then_attaches(self):
        self.clock.t = 12.0
        tools = self.tools([STATE], game=FakeGame(launched_at=10.0))
        out = tools.ch_attach()
        self.assertEqual(self.clock.slept, [3.0])
        self.assertEqual(self.frida.attached_to, [C.PROCESS_NAME])
        self.assertIn("waited 3.0 s, so the attach comes 5 s after the launch (ATTACH_DELAY_S)", out)
        self.assertIn("state: playing", out)
        self.assertIn("songTime: 3.5", out)
        self.assertEqual(self.game.backups, 0)

    def test_attach_after_the_delay_does_not_wait(self):
        self.clock.t = 20.0
        tools = self.tools([STATE], game=FakeGame(launched_at=10.0))
        tools.ch_attach()
        self.assertEqual(self.clock.slept, [])
        self.assertEqual(self.frida.attached_to, [C.PROCESS_NAME])

    def test_an_unknown_build_is_refused_and_nothing_attaches(self):
        refusal = B.UnknownBuild("v1.1.0.6200-final", "no 6200 folder")

        def unknown():
            raise refusal

        tools = self.tools([STATE], load_facts=unknown)
        out = tools.ch_attach()
        self.assertEqual(out, f"{refusal}\nnothing attached")
        self.assertEqual(self.frida.attached_to, [])
        self.assertEqual(self.game.backups, 0)

    def test_an_adopted_game_gets_its_backup_first(self):
        tools = self.tools([STATE])
        out = tools.ch_attach()
        self.assertEqual(self.game.backups, 1)
        self.assertIn(BACKUP_LINE, out)
        self.assertEqual(self.clock.slept, [])

    def test_the_facts_load_once_per_build(self):
        tools = self.tools([STATE, STATE])
        tools.ch_attach()
        tools.ch_detach()
        tools.ch_attach()
        self.assertEqual(self.loads, 1)

    def test_an_attach_with_no_first_reply_names_its_cap(self):
        tools = self.tools([None])
        out = tools.ch_attach()
        self.assertIn(f"within the cap of {FT.ATTACH_CAP_S:g} s; detached (ATTACH_CAP_S)", out)
        self.assertEqual(self.session.detach_calls, 1)

    def test_a_game_that_is_not_running_says_so(self):
        tools = self.tools()
        self.frida.session = None
        out = tools.ch_attach()
        self.assertIn(f"{C.PROCESS_NAME} is not running", out)
        self.assertIn("state: not running (found by the last ch_attach)", tools.ch_state())

    def test_a_game_that_closes_after_the_attach_is_not_attached_with_frida_reason(self):
        tools = self.tools([STATE], attached=True)
        tools._agent._on_detached("process-terminated")
        out = tools.ch_state()
        self.assertIn("state: not attached (session detached: process-terminated)", out)
        self.assertNotIn("not running", out)

    def test_is_attached_follows_the_attach_and_a_dropped_session(self):
        tools = self.tools([STATE])
        self.assertFalse(tools.is_attached())
        tools.ch_attach()
        self.assertTrue(tools.is_attached())
        tools._agent._on_detached("process-terminated")
        self.assertFalse(tools.is_attached())

    def test_is_attached_is_false_after_a_detach(self):
        tools = self.tools([STATE], attached=True)
        tools.ch_detach()
        self.assertFalse(tools.is_attached())

    def test_attaching_again_after_the_game_dropped_the_session_closes_the_old_log(self):
        tools = self.tools([STATE, STATE], attached=True)
        old = tools._agent
        old._on_detached("process-terminated")
        out = tools.ch_attach()
        self.assertIn("state: playing", out)
        self.assertTrue(old._log.closed)
        self.assertIsNot(tools._agent, old)

    def test_a_missing_state_agent_says_how_to_build_it(self):
        os.remove(os.path.join(self.agent_dir, FT.STATE_AGENT_FILE))
        out = self.tools([STATE]).ch_attach()
        path = os.path.join(self.agent_dir, FT.STATE_AGENT_FILE)
        self.assertEqual(out.splitlines()[-1],
                         TH.agent_missing_line(path, FT.STATE_AGENT_NAME))
        self.assertIn("npm run build:state", out)
        self.assertEqual(self.frida.attached_to, [])


class ReadTest(Rig):
    def test_a_read_with_no_reply_names_its_cap(self):
        tools = self.tools([STATE, None], attached=True)
        out = tools.ch_state()
        self.assertIn(f"within the cap of {FT.READ_CAP_S:g} s (READ_CAP_S)", out)

    def test_state_shows_the_destroyed_count_it_was_read_with(self):
        tools = self.tools([STATE, MENU_ONE_DESTROYED], attached=True)
        lines = tools.ch_state().splitlines()
        self.assertEqual(lines[0], "state: menu (no live GameManager)")
        self.assertIn("found: false", lines)
        self.assertIn("destroyed: 1", lines)

    def test_state_before_attach_says_not_attached(self):
        self.assertEqual(self.tools().ch_state(), f"state: not attached; {FT.NOT_ATTACHED}")

    def test_snapshot_with_no_play_session_says_the_score_is_not_read(self):
        tools = self.tools([STATE, STATE], attached=True)
        out = tools.ch_snapshot()
        self.assertIn("songTime: 3.5", out)
        self.assertIn("score: not read (no play session has found the engine)", out)

    def test_snapshot_shows_the_score_after_the_last_press(self):
        tools = self.tools([STATE, STATE], attached=True)
        self.game.play = {"last_row": {"score_after": 150, "clock_ms_at_press": 2998.0},
                          "press_count": 2, "stop": None}
        out = tools.ch_snapshot()
        self.assertIn("score after press 2: 150", out)
        self.assertIn("song clock at that press: 2998.0 ms", out)


# A hand-written menu block in the shape agent/menu.ts sends.
MENU_READ = {
    "classes": {
        "MainMenu": {"found": True, "live": 1, "destroyed": 0,
                     "onlineMenuActive": [False]},
        "SongSelect": {"found": True, "live": 1, "destroyed": 2,
                       "onlineMenuActive": [True]},
        "PlayerSelectionMenu": {"found": False, "reason": "PlayerSelectionMenu not found"},
    },
    "selectedSong": {"folderPath": "C:\\Songs\\Window Map", "chartName": "notes.chart"},
    "globals": {"found": True, "currentSongFolder": None, "gameMode": 0, "playerCount": 1},
}
MENU_LINES = [
    "MainMenu: 1 live, 0 destroyed; onlineMenuActive: [false]",
    "SongSelect: 1 live, 2 destroyed; onlineMenuActive: [true]",
    "PlayerSelectionMenu: PlayerSelectionMenu not found",
    "selected song folder: C:\\Songs\\Window Map",
    "selected song chartName: notes.chart",
    "GlobalVariables currentSongFolder: null",
    "GlobalVariables gameMode: 0",
    "GlobalVariables playerCount: 1",
]


class MenuFieldsTest(unittest.TestCase):
    """menu_fields.json parses, and each field and enum it names sits at the
    dump line it gives, in the dump of the build it names. The dump lives
    outside the repo, so without it the dump check is skipped."""

    def setUp(self):
        self.data = FT.menu_fields()

    def test_it_parses_with_one_table_per_field(self):
        keys = [f["key"] for f in self.data["fields"]]
        self.assertEqual(keys, ["instrument", "difficulty"])
        for field in self.data["fields"]:
            with self.subTest(key=field["key"]):
                self.assertEqual(field["class"], "PlayerSelectionMenu")
                for number in field["values"]:
                    int(number)

    def test_each_name_sits_at_its_dump_line(self):
        dump = os.path.join(B.DUMPS_ROOT, self.data["build"], "dump.cs")
        if not os.path.isfile(dump):
            self.skipTest(f"{dump} is not on this machine")
        wanted = {}
        for field in self.data["fields"]:
            wanted[field["class_dump_line"]] = f"class {field['class']} "
            wanted[field["dump_line"]] = f" {field['field']}; "
            wanted[field["enum_dump_line"]] = f"public enum {field['enum']} "
        found = {}
        with open(dump, encoding="utf-8") as f:
            for n, line in enumerate(f, start=1):
                if n in wanted:
                    found[n] = line
                if len(found) == len(wanted):
                    break
        for n, text in wanted.items():
            with self.subTest(line=n):
                self.assertIn(text, found.get(n, ""))
        for field in self.data["fields"]:
            # The field's line names the enum type it holds.
            self.assertIn(f" {field['enum']} {field['field']}; ", found[field["dump_line"]])

    def test_each_enum_word_sits_in_its_enum(self):
        dump = os.path.join(B.DUMPS_ROOT, self.data["build"], "dump.cs")
        if not os.path.isfile(dump):
            self.skipTest(f"{dump} is not on this machine")
        with open(dump, encoding="utf-8") as f:
            lines = f.readlines()
        for field in self.data["fields"]:
            start = field["enum_dump_line"] - 1
            end = lines.index("}\n", start)
            body = "".join(lines[start:end])
            consts = dict((value, word) for word, value in re.findall(
                rf"public const {re.escape(field['enum'])} (\w+) = (-?\d+);", body))
            with self.subTest(key=field["key"]):
                self.assertEqual(field["values"], consts)


# The player screen's per-frame method that agent/menu.ts hooks, as its
# literal there.
_FRAME_RVA = re.compile(r"PLAYER_SCREEN_FRAME_RVA = (0x[0-9A-Fa-f]+);")


class PlayerScreenHookTest(unittest.TestCase):
    """The method menu.ts hooks to record the player screens is an instance
    method of PlayerSelectionMenu, in the facts of the build menu_fields.json
    names, and no other class runs its code (a shared body would record other
    classes' objects as player screens). Skipped without the facts."""

    def test_the_hooked_method_is_the_player_screens_own(self):
        with open(os.path.join(FT.AGENT_DIR, "menu.ts"), encoding="utf-8") as f:
            match = _FRAME_RVA.search(f.read())
        self.assertIsNotNone(match, "menu.ts names no PLAYER_SCREEN_FRAME_RVA")
        rva = int(match.group(1), 16)
        path = os.path.join(B.DUMPS_ROOT, FT.menu_fields()["build"], B.FACTS_FILE)
        if not os.path.isfile(path):
            self.skipTest(f"{path} is not on this machine")
        with open(path, encoding="utf-8") as f:
            facts = json.load(f)
        owners = [c for c in facts["classes"]
                  if any(m["rva"] == rva for m in c["methods"])]
        self.assertEqual([c["name"] for c in owners], ["PlayerSelectionMenu"])
        methods = [m for m in owners[0]["methods"] if m["rva"] == rva]
        self.assertEqual([(m["name"], m["static"], m["params"]) for m in methods],
                         [("ʸʳʼʹʸʽʾˀʴʼʷ", False, [])])


class MenuTest(Rig):
    def test_menu_lines_print_the_block_as_sent(self):
        self.assertEqual(FT.menu_lines(MENU_READ), MENU_LINES)

    def test_menu_lines_with_no_song_and_no_globals(self):
        menu = {**MENU_READ, "selectedSong": None,
                "globals": {"found": False, "reason": "no live GlobalVariables.instance"}}
        lines = FT.menu_lines(menu)
        self.assertIn("selected song: none", lines)
        self.assertEqual(lines[-1], "GlobalVariables: no live GlobalVariables.instance")

    def test_menu_lines_print_the_agents_error(self):
        self.assertEqual(FT.menu_lines({"error": "reading the menus failed: x"}),
                         ["menu: reading the menus failed: x"])

    def test_menu_lines_with_no_block(self):
        self.assertEqual(FT.menu_lines(None),
                         ["menu: not read (the agent sent no menu block)"])

    def test_ch_menu_prints_the_lines_then_the_state_word(self):
        tools = self.tools([STATE, {**MENU, "menu": MENU_READ}], attached=True)
        self.assertEqual(tools.ch_menu().splitlines(),
                         MENU_LINES + ["state: menu (no live GameManager)"])

    def test_ch_menu_before_attach_says_not_attached(self):
        self.assertEqual(self.tools().ch_menu(), f"state: not attached; {FT.NOT_ATTACHED}")

    def test_state_leaves_the_menu_block_to_ch_menu(self):
        tools = self.tools([STATE, {**STATE, "menu": MENU_READ}], attached=True)
        out = tools.ch_state()
        self.assertIn("songTime: 3.5", out)
        self.assertNotIn("menu", out)

    def test_read_state_returns_the_agents_dict(self):
        reply = {**STATE, "menu": MENU_READ}
        tools = self.tools([STATE, reply], attached=True)
        self.assertEqual(tools.read_state(), reply)

    def test_menu_lines_print_active_and_the_player_screen_words(self):
        def player(instrument, difficulty):
            return {**MENU_READ, "classes": {"PlayerSelectionMenu": {
                "found": True, "live": 1, "destroyed": 0, "active": [True],
                "fields": {"instrument": [instrument], "difficulty": [difficulty]}}}}
        self.assertEqual(FT.menu_lines(player(9, 3))[0],
                         "PlayerSelectionMenu: 1 live, 0 destroyed; active: [true]; "
                         "instrument: [9 (ProDrums)]; difficulty: [3 (Expert)]")
        line = FT.menu_lines(player(6, 2))[0]
        self.assertIn("instrument: [6 (Drums)]", line)
        self.assertIn("difficulty: [2 (Hard)]", line)

    def test_menu_lines_print_why_the_player_screen_was_not_read(self):
        menu = {**MENU_READ, "classes": {"PlayerSelectionMenu": {
            "found": False, "reason": "the player-screen hook has not been placed yet"}}}
        self.assertEqual(FT.menu_lines(menu)[0],
                         "PlayerSelectionMenu: the player-screen hook has not been placed yet")

    def test_menu_lines_print_a_field_read_that_is_not_a_known_number_as_sent(self):
        menu = {**MENU_READ, "classes": {"PlayerSelectionMenu": {
            "found": True, "live": 2, "destroyed": 0, "active": [None, "error: x"],
            "fields": {"instrument": [99, "error: boom"], "difficulty": [-1, None]}}}}
        self.assertEqual(FT.menu_lines(menu)[0],
                         "PlayerSelectionMenu: 2 live, 0 destroyed; active: [null, \"error: x\"]; "
                         "instrument: [99 (not in menu_fields.json), error: boom]; "
                         "difficulty: [-1 (None), null]")

    def test_read_state_before_attach_raises_with_the_reason(self):
        with self.assertRaises(FT.StateUnread) as caught:
            self.tools().read_state()
        self.assertEqual(str(caught.exception), f"state: not attached; {FT.NOT_ATTACHED}")


class StateWordsTest(unittest.TestCase):
    def state(self, **fields):
        return {**STATE, **fields}

    def test_not_running(self):
        self.assertEqual(FT.state_words(None, running=False), "not running")

    def test_not_attached(self):
        self.assertEqual(FT.state_words(None), "not attached")

    def test_menu_for_found_false(self):
        self.assertEqual(FT.state_words(MENU), "menu (no live GameManager)")

    def test_menu_when_the_only_game_manager_was_destroyed(self):
        self.assertEqual(FT.state_words(MENU_ONE_DESTROYED), "menu (no live GameManager)")

    def test_a_live_game_manager_beside_a_destroyed_one_keeps_its_state(self):
        self.assertEqual(FT.state_words(self.state(destroyed=1)), "playing")

    def test_loading(self):
        self.assertEqual(FT.state_words(self.state(songLoaded=False)), "loading")

    def test_song_over(self):
        self.assertEqual(FT.state_words(self.state(isSongOver=True)), "song over")

    def test_paused(self):
        self.assertEqual(FT.state_words(self.state(isPaused=True)), "paused")

    def test_playing(self):
        self.assertEqual(FT.state_words(STATE), "playing")

    def test_loaded_not_started(self):
        self.assertEqual(FT.state_words(self.state(isSongPlaying=False)), "loaded, not started")


class TraceTest(Rig):
    def test_trace_start_refuses_with_no_live_game_manager(self):
        tools = self.tools([STATE, MENU], attached=True)
        out = tools.ch_trace_start(self.song)
        self.assertEqual(out.splitlines()[-1],
                         "no live GameManager: start the song (pause it if you like), "
                         "then call ch_trace_start")
        self.assertEqual(self.traces, [])

    def test_trace_start_before_attach_refuses(self):
        tools = self.tools()
        out = tools.ch_trace_start(self.song)
        self.assertIn(FT.NOT_ATTACHED, out)
        self.assertEqual(self.traces, [])
        self.assertEqual(self.game.backups, 0)

    def test_trace_start_takes_the_backup_just_before_it_hooks(self):
        tools = self.tools([STATE, STATE], attached=True)
        self.game.backups = 0  # as if the game side held no backup yet
        lines = tools.ch_trace_start(self.song).splitlines()
        self.assertEqual(self.game.backups, 1)
        self.assertEqual(lines[0], BACKUP_LINE)
        self.assertTrue(lines[1].startswith("trace started"))

    def test_a_refused_trace_start_takes_no_backup(self):
        tools = self.tools([STATE, MENU], attached=True)
        self.game.backups = 0
        tools.ch_trace_start(self.song)
        self.assertEqual(self.game.backups, 0)

    def test_trace_start_hooks_the_default_method_from_the_facts(self):
        tools = self.tools([STATE, STATE], attached=True)
        out = tools.ch_trace_start(self.song)
        (trace,) = self.traces
        self.assertEqual(trace.engine_class, "Eng")
        self.assertEqual(trace.methods, ["AddPoints"])
        self.assertEqual(trace.kw["engine_ctor_rva"], C.RVA_DRUMS_ENGINE_CTOR)
        self.assertIn(f"AddPoints (0x{C.RVA_NOTE_SCORE_ADD:X})", out)

    def test_with_dynamics_hooks_both_methods(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song, with_dynamics=True)
        self.assertEqual(self.traces[0].methods, ["AddPoints", "Dynamics"])

    def test_notes_from_since_are_lined_up(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song)
        self.traces[0].call_list = [call(1.0), call(2.1), call(2.95)]
        out = tools.ch_notes(since=1).splitlines()
        self.assertIn("call 1: AddPoints at 2100.0 ms; nearest note 1 at 2000 ms, diff +100.0 ms", out)
        self.assertIn("call 2: AddPoints at 2950.0 ms; nearest note 2 at 3000 ms, diff -50.0 ms", out)
        self.assertNotIn("call 0", "\n".join(out))
        self.assertIn(f"calls within {C.EXPECT_NORMAL_BACK_MS:g} ms of their nearest note "
                      "(EXPECT_NORMAL_BACK_MS): 1 of 2", out)
        self.assertEqual(out[-1], "next: ch_notes(since=3)")

    def test_trace_stop_stops_and_prints_the_ended_line(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song)
        out = tools.ch_trace_stop().splitlines()
        self.assertEqual(self.traces[0].stop_caps, [FT.READ_CAP_S])
        self.assertEqual(out[:2], ["ended: stopped", "calls: 0"])

    def test_a_trace_that_does_not_stop_in_time_says_so(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song)
        self.traces[0].ends_on_stop = False
        self.assertEqual(tools.ch_trace_stop(),
                         f"the trace thread was still running after {FT.READ_CAP_S:g} s "
                         "(READ_CAP_S); it ends with the server")

    def test_the_stop_hook_stops_the_trace_and_detaches_once(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song)
        (hook,) = self.game.hooks
        said = hook()
        self.assertEqual(self.traces[0].stop_caps, [FT.READ_CAP_S])
        self.assertEqual(self.session.detach_calls, 1)
        self.assertIn("frida: detached", said)
        hook()
        self.assertEqual(self.session.detach_calls, 1)
        self.assertEqual(self.traces[0].stop_caps, [FT.READ_CAP_S])


class RealFactsTest(unittest.TestCase):
    def test_both_hook_rvas_name_one_engine_method_each(self):
        if not os.path.isfile(REAL_FACTS):
            self.skipTest(f"no real facts file at {REAL_FACTS}")
        with open(REAL_FACTS, encoding="utf-8") as f:
            facts = json.load(f)
        engine, _ = B.engine_class(facts)
        names = B.method_names_by_rva(facts, engine, [C.RVA_NOTE_SCORE_ADD, C.RVA_NOTE_DYNAMICS])
        self.assertEqual(len(names), 2)


if __name__ == "__main__":
    unittest.main()
