"""Tests for the Clone Hero MCP server: the tool list, the reply cut and its
closing line, ch_status joining both halves, and the .mcp.json entry.

No game and no Frida: the helper is built on a bot_run.Env whose launch
refuses, and no test calls a tool that touches the game.
"""

from __future__ import annotations

import asyncio
import json
import os
import sys
import tempfile
import unittest
from types import SimpleNamespace

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402
from tools.ch_mcp import front as F  # noqa: E402
from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import kit_tools as KT  # noqa: E402
from tools.ch_mcp import server as S  # noqa: E402
from tools.ch_mcp import sp_tools as SP  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS  # noqa: E402

# A real worker: the real server on a real Helper, over stdio, as the front
# runs server.py. argv gives the repo root, the game exe, the documents folder,
# the scratch root and "backup" or "none". Its Env has no registry runner, so a
# backup is the code's own temp-folder form, and nothing it does starts a game.
# Its lines go to stderr, because stdout carries the replies.
WORKER_CHILD = """
import sys
sys.path.insert(0, sys.argv[1])
from tools.ch_mcp import bot_run as B, server as S
env = B.Env(exe=sys.argv[2], documents_dir=sys.argv[3], run_reg=None,
            out=lambda line: print(line, file=sys.stderr))
helper = S.Helper(env=env, scratch_root=sys.argv[4])
if sys.argv[5] == "backup":
    helper.game.ensure_backup()
S.build_server(helper).run("stdio")
"""


def backup_folders(root: str) -> list[str]:
    """Every backup folder game_files.backup made anywhere under `root`."""
    return [os.path.join(top, name) for top, dirs, _files in os.walk(root)
            for name in dirs if name.startswith(game_files.BACKUP_PREFIX)]

# The eighteen tools of the server plan's "The tools" table, then the eleven
# of the Star Power and song start plan's (task T5), then the five kit tools
# of the in-game input plan (task K5). The direct-start plan (task D5)
# registers the changed ch_start and ch_kit_setup and adds no tool. The optimal
# score run plan (task T4) adds ch_run_chart, the reload plan (task R3)
# ch_open_items, the player-limits plan (task T4) ch_edge_probe, and the in-game
# press timing plan (task IG3) ch_kit_frame_probe. The server plan is
# docs/superpowers/plans/2026-10-28-ch-mcp-server.md (removed; git show ec91f8d1:docs/superpowers/plans/2026-10-28-ch-mcp-server.md).
TOOL_NAMES = {
    "ch_run_chart", "ch_open_items", "ch_edge_probe", "ch_kit_frame_probe",
    "ch_start", "ch_stop", "ch_status", "ch_install_chart", "ch_wait_stats",
    "ch_stats", "ch_compare", "ch_attach", "ch_detach", "ch_state",
    "ch_snapshot", "ch_trace_start", "ch_notes", "ch_trace_stop", "ch_press",
    "ch_play_chart", "ch_play_status", "ch_play_stop",
    "ch_sp_watch_start", "ch_sp", "ch_sp_watch_status", "ch_sp_watch_stop",
    "ch_sp_compare", "ch_sp_between", "ch_key", "ch_menu", "ch_open_song",
    "ch_method_watch_start", "ch_method_watch_stop",
    "ch_kit_setup", "ch_kit_press", "ch_kit_status", "ch_kit_watch", "ch_kit_teardown",
}


def _no_launch(cmd):
    raise AssertionError(f"a test launched {cmd}")


class ServerTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.printed = []
        # No registry runner: a backup is the code's own temp-folder form and
        # never reads the user's Rewired key.
        env = bot_run.Env(exe=os.path.join(tmp.name, "game.exe"),
                          documents_dir=os.path.join(tmp.name, "docs"),
                          launch=_no_launch, out=self.printed.append, run_reg=None)
        self.helper = S.Helper(env=env, scratch_root=tmp.name,
                               game_kw={"driver": object()})

    def test_build_server_registers_the_thirty_seven_tools(self):
        tools = asyncio.run(S.build_server(self.helper).list_tools())
        self.assertEqual({t.name for t in tools}, TOOL_NAMES)

    def _open_items(self):
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_open_items", {}))
        return result.content[0].text

    def test_ch_open_items_says_so_when_nothing_is_open(self):
        # The front's word for "nothing" is the one the worker must say.
        self.assertEqual(self._open_items(), F.NOTHING_OPEN)

    def test_ch_open_items_names_an_adopted_backup(self):
        # A real backup, taken the way ch_attach takes one for a game the
        # server did not start.
        self.assertEqual(backup_folders(self.tmp), [])
        self.helper.game.ensure_backup()
        [folder] = backup_folders(self.tmp)
        self.assertTrue(os.path.isfile(os.path.join(folder, game_files.BACKUP_MANIFEST)))
        [line] = self._open_items().splitlines()
        self.assertTrue(line.startswith("adopted backup"), line)
        self.assertTrue(line.endswith(folder), line)

    def _front_reads_a_real_worker(self, take_backup: bool) -> tuple[list, list]:
        """What the front's own reader makes of a real worker's ch_open_items
        reply over the real pipe, and the backup folders under the worker's
        scratch root. The worker is the real server.py code, started by the
        front's own WorkerLink."""
        scratch = os.path.join(self.tmp, "worker scratch")
        os.makedirs(scratch)
        link = F.WorkerLink([sys.executable, "-I", "-c", WORKER_CHILD, _REPO_ROOT,
                             self.helper.env.exe, self.helper.env.documents_dir, scratch,
                             "backup" if take_backup else "none"])
        link.start()
        self.addCleanup(link.stop, F.WORKER_EXIT_CAP_S)
        return F.open_items(link, F.WORKER_START_CAP_S), backup_folders(scratch)

    def test_the_front_reads_the_real_nothing_open_as_nothing_open(self):
        self.assertEqual(self._front_reads_a_real_worker(take_backup=False), ([], []))

    def test_the_front_reads_the_real_adopted_backup_line_as_open(self):
        lines, folders = self._front_reads_a_real_worker(take_backup=True)
        self.assertEqual(len(folders), 1, folders)
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].startswith("adopted backup"), lines)
        self.assertTrue(lines[0].endswith(folders[0]), lines)

    def test_open_items_uses_public_checks(self):
        # A field of another tool read here would be a second copy of the
        # question its owner answers (audit L76).
        import inspect
        source = inspect.getsource(S.Helper.open_items)
        self.assertNotIn("._", source)

    def test_each_public_check_answers_both_ways(self):
        alive = SimpleNamespace(is_alive=lambda: True)
        dead = SimpleNamespace(is_alive=lambda: False)
        h = self.helper
        cases = [
            ("kit.agent_attached", h.kit.agent_attached, h.kit, "_agent", object(), None),
            ("sp.watch_running", h.sp.watch_running, h.sp, "_watch",
             SP._Watch("w.csv", 1.0, thread=alive), SP._Watch("w.csv", 1.0, thread=dead)),
            ("method_watch.running", h.method_watch.running, h.method_watch, "_session",
             SimpleNamespace(ended=lambda: False), SimpleNamespace(ended=lambda: True)),
            ("frida.trace_running", h.frida.trace_running, h.frida, "_trace",
             SimpleNamespace(ended=lambda: False), SimpleNamespace(ended=lambda: True)),
        ]
        for name, check, owner, field, on, off in cases:
            with self.subTest(name):
                setattr(owner, field, None)
                self.assertFalse(check())
                setattr(owner, field, on)
                self.assertTrue(check())
                setattr(owner, field, off)
                self.assertFalse(check())

    def test_a_long_reply_is_cut_at_a_line_with_the_clone_hero_closing_line(self):
        long_text = "\n".join(f"call {i}: " + "x" * 90 for i in range(200))
        self.helper.frida.ch_notes = lambda since: long_text
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_notes", {"since": 0}))
        text = result.content[0].text
        self.assertLessEqual(len(text), REPLY_CAP_CHARS)
        lines = text.splitlines()
        kept = len(lines) - 1
        self.assertTrue(all(line in long_text.splitlines() for line in lines[:-1]))
        self.assertEqual(lines[-1], f"[cut at 12,000 characters: {200 - kept} of 200 lines "
                                    f"dropped; {S.CAP_HINT}]")
        self.assertIn("ch_notes(since=...)", S.CAP_HINT)

    def test_the_trace_texts_say_the_hook_fires_on_hits(self):
        # The texts rest on the Clone Hero page's entry for the traced
        # method; the test fails when the page no longer holds it.
        with open(os.path.join(_REPO_ROOT, "docs", "clone-hero", "07-internals.md"),
                  encoding="utf-8") as f:
            page = f.read()
        self.assertIn("### IN-19 Whether the score method fires only for hit notes", page)
        server = S.build_server(self.helper)
        tools = {t.name: t for t in asyncio.run(server.list_tools())}
        for name, text in (("instructions", server.instructions),
                           ("ch_trace_start", tools["ch_trace_start"].description)):
            with self.subTest(text=name):
                self.assertIn("each note the player hits", text)
                self.assertNotIn("processes", text)

    def test_the_kit_is_wired_into_the_menu_driver_and_the_auto_player(self):
        self.assertIs(self.helper.menu._kit, self.helper.kit)
        # ch_play_chart's route reaches GameTools, and with no kit set up the
        # kit press GameTools asks KitTools for is None.
        server = S.build_server(self.helper)
        result = asyncio.run(server.call_tool("ch_play_chart",
                                              {"score_json": "x.json",
                                               "schedule_json": "y.json", "route": "pads"}))
        # The unknown route is named, with every route GameTools takes.
        text = result.content[0].text
        self.assertTrue(text.startswith('unknown route "pads"'), text)
        for route in GT.ROUTES:
            self.assertIn(route, text.split(";", 1)[1])
        result = asyncio.run(server.call_tool("ch_play_chart",
                                              {"score_json": "x.json",
                                               "schedule_json": "y.json", "route": GT.ROUTE_KIT}))
        # The kit route is refused with the kit's own "not set up" words and
        # names the other route.
        text = result.content[0].text
        self.assertTrue(text.startswith(f"route {GT.ROUTE_KIT}:"), text)
        self.assertIn(KT.NOT_SET_UP, text)
        self.assertIn(f'route="{GT.ROUTE_KEYS}"', text)

    def test_the_play_tools_take_the_arguments_of_the_methods_they_reach(self):
        # Each play tool's schema is the argument list of the GameTools
        # method it calls: the same names, the same defaults, and the same
        # arguments required. An argument the method dropped is gone from the
        # tool too.
        import inspect
        required = object()
        for name in ("ch_run_chart", "ch_play_chart", "ch_edge_probe"):
            with self.subTest(name):
                params = list(inspect.signature(getattr(GT.GameTools, name)).parameters.values())[1:]
                want = {p.name: required if p.default is inspect.Parameter.empty else p.default
                        for p in params}
                got = {arg: spec.get("default", required)
                       for arg, spec in self._schema(name).items()}
                self.assertEqual(got, want)

    def test_ch_edge_probe_passes_its_arguments_through(self):
        seen = []
        self.helper.game.ch_edge_probe = lambda *a: seen.append(a) or "probing"
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_edge_probe", {
            "song_folder": "song", "score_json": "s.json", "schedule_json": "sched.json",
            "probes_json": "p.json"}))
        self.assertEqual(result.content[0].text, "probing")
        self.assertEqual(seen, [("song", "s.json", "sched.json", "p.json")])

    def test_ch_run_chart_passes_its_arguments_through(self):
        seen = []
        self.helper.game.ch_run_chart = lambda *a: seen.append(a) or "ran"
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_run_chart", {
            "song_folder": "song", "score_json": "top.json", "schedule_json": "sched.json",
            "kit_player": "System", "path_index": 3, "probes_json": "p.json"}))
        self.assertEqual(result.content[0].text, "ran")
        self.assertEqual(seen, [("song", "top.json", "sched.json", "System", 3, "p.json")])

    def test_a_leftover_timing_argument_is_dropped_without_a_word(self):
        # A call that still passes timing runs the real tool as if timing
        # were left out: the same reply, and no argument error. With no game
        # open, ch_run_chart reaches its schedule read and ch_play_chart its
        # kit check.
        server = S.build_server(self.helper)
        for name, args in (("ch_run_chart", {"song_folder": "song", "score_json": "top.json",
                                             "schedule_json": "sched.json"}),
                           ("ch_play_chart", {"score_json": "s.json",
                                              "schedule_json": "sched.json"})):
            with self.subTest(name):
                left_out = asyncio.run(server.call_tool(name, args))
                passed = asyncio.run(server.call_tool(name, {**args, "timing": "python"}))
                self.assertFalse(passed.is_error)
                self.assertEqual(passed.content[0].text, left_out.content[0].text)
        self.assertEqual(self.printed, [])

    def test_ch_kit_frame_probe_passes_its_seconds_through(self):
        seen = []
        self.helper.kit.ch_kit_frame_probe = lambda *a: seen.append(a) or "probed"
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_kit_frame_probe",
                                                                   {"seconds": 5}))
        self.assertEqual(result.content[0].text, "probed")
        self.assertEqual(seen, [(5,)])

    def test_the_helper_hands_ch_run_chart_its_three_parts(self):
        parts = self.helper.game._run_parts
        self.assertIs(parts.frida, self.helper.frida)
        self.assertIs(parts.kit, self.helper.kit)
        self.assertIs(parts.sp, self.helper.sp)

    def test_the_kit_reads_the_pause_setting_in_the_env_documents_folder(self):
        docs = self.helper.env.documents_dir
        os.makedirs(docs)
        with open(os.path.join(docs, "settings.ini"), "w", encoding="utf-8") as f:
            f.write("[game]\npause_on_focus_lost = 1\n")
        self.helper.frida.is_attached = lambda: True
        # Refused, naming the file, the setting, the value read there, the
        # run's value and ch_start.
        text = self.helper.kit.ch_kit_setup()
        self.assertTrue(text.startswith("refused:"), text)
        self.assertIn(f"{game_files.SETTINGS} reads {game_files.PAUSE_ON_FOCUS_LOST} = 1", text)
        self.assertIn(f"not {game_files.PAUSE_ON_FOCUS_LOST_RUN_VALUE}", text)
        self.assertIn("ch_start", text)

    def _schema(self, name):
        tools = {t.name: t for t in asyncio.run(S.build_server(self.helper).list_tools())}
        return tools[name].input_schema["properties"]

    def test_ch_start_takes_human_and_passes_it_through(self):
        self.assertEqual(self._schema("ch_start")["human"]["default"], False)
        seen = []
        self.helper.game.ch_start = lambda *a, **kw: seen.append((a, kw)) or "started"
        result = asyncio.run(S.build_server(self.helper).call_tool(
            "ch_start", {"song_folder": "song", "human": True}))
        self.assertEqual(result.content[0].text, "started")
        self.assertEqual(seen, [(("song", bot_run.DEFAULT_PLAYER, None), {"human": True})])

    def test_ch_play_chart_passes_its_arguments_through(self):
        seen = []
        self.helper.game.ch_play_chart = lambda *a: seen.append(a) or "playing"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_play_chart", {"score_json": "s.json",
                                                       "schedule_json": "sched.json"}))
        asyncio.run(server.call_tool("ch_play_chart", {
            "score_json": "s.json", "schedule_json": "sched.json", "log": "p.csv",
            "song_folder": "song", "route": "kit", "path_index": 2}))
        self.assertEqual(seen, [("s.json", "sched.json", None, None, None, 0),
                                ("s.json", "sched.json", "p.csv", "song", "kit", 2)])

    def test_ch_open_song_takes_the_player_and_passes_it_through(self):
        self.assertEqual(self._schema("ch_open_song")["player"]["default"],
                         bot_run.DEFAULT_PLAYER)
        seen = []
        self.helper.menu.ch_open_song = lambda *a: seen.append(a) or "opened"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_open_song", {"song_folder": "song"}))
        asyncio.run(server.call_tool("ch_open_song", {"song_folder": "song",
                                                      "player": "ProDrums,Expert"}))
        self.assertEqual(seen, [("song", bot_run.DEFAULT_PLAYER), ("song", "ProDrums,Expert")])

    def test_ch_sp_compare_passes_the_schedule_through_and_takes_no_dump(self):
        schema = self._schema("ch_sp_compare")
        self.assertNotIn("default", schema["schedule_json"])
        self.assertNotIn("dump_json", schema)
        self.assertNotIn("path_index", schema)
        seen = []
        self.helper.sp.ch_sp_compare = lambda *a: seen.append(a) or "compared"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_sp_compare", {"watch_csv": "w.csv",
                                                       "score_json": "s.json",
                                                       "schedule_json": "sched.json"}))
        asyncio.run(server.call_tool("ch_sp_compare", {
            "watch_csv": "w.csv", "score_json": "s.json", "schedule_json": "sched.json",
            "resolution": 192}))
        width = schema["width"]["default"]
        self.assertEqual(seen, [("w.csv", "s.json", "sched.json", None, None, width),
                                ("w.csv", "s.json", "sched.json", 192, None, width)])

    def test_ch_compare_takes_the_run_settings_and_passes_them_through(self):
        schema = self._schema("ch_compare")
        self.assertEqual(set(schema), {"stats_path", "score_json", "checksum",
                                       "playback_speed", "no_fail", "modifiers"})
        for name in ("checksum", "playback_speed", "no_fail", "modifiers"):
            self.assertIsNone(schema[name]["default"])
        seen = []
        self.helper.game.ch_compare = lambda *a: seen.append(a) or "compared"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_compare", {"stats_path": "st.json",
                                                    "score_json": "s.json"}))
        asyncio.run(server.call_tool("ch_compare", {
            "stats_path": "st.json", "score_json": "s.json", "checksum": "AB",
            "playback_speed": 100, "no_fail": True, "modifiers": ["DoubleKick"]}))
        self.assertEqual(seen, [("st.json", "s.json", None, None, None, None),
                                ("st.json", "s.json", "AB", 100, True, ["DoubleKick"])])

    def test_ch_start_human_with_no_song_folder_is_refused_before_any_launch(self):
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_start", {"human": True}))
        self.assertEqual(result.content[0].text, bot_run.HUMAN_NEEDS_SONG)
        self.assertEqual(self.printed, [])

    def test_ch_kit_setup_takes_player_and_passes_it_through(self):
        self.assertEqual(self._schema("ch_kit_setup")["player"]["default"], None)
        seen = []
        self.helper.kit.ch_kit_setup = lambda player: seen.append(player) or "set up"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_kit_setup", {}))
        asyncio.run(server.call_tool("ch_kit_setup", {"player": "System"}))
        self.assertEqual(seen, [None, "System"])

    def test_ch_kit_press_takes_value_and_passes_it_through(self):
        self.assertEqual(self._schema("ch_kit_press")["value"]["default"], None)
        seen = []
        self.helper.kit.ch_kit_press = lambda *a: seen.append(a) or "pressed"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_kit_press", {"items": "green"}))
        asyncio.run(server.call_tool("ch_kit_press", {"items": "green", "read": "Green",
                                                      "value": 0.748}))
        self.assertEqual(seen, [("green", None, None, None, None, None),
                                ("green", None, None, None, "Green", 0.748)])

    def test_ch_kit_press_says_what_value_does(self):
        tools = {t.name: t for t in asyncio.run(S.build_server(self.helper).list_tools())}
        self.assertIn("value sets", tools["ch_kit_press"].description)

    def test_status_joins_the_game_and_the_frida_lines(self):
        # ch_status is the game half, then the Frida half, each from its own
        # tool, before and after a real backup; after it, the status names
        # the backup folder.
        h = self.helper
        for step in ("no backup", "backup taken"):
            with self.subTest(step):
                if step == "backup taken":
                    h.game.ensure_backup()
                out = h.ch_status().splitlines()
                self.assertEqual(out, h.game.ch_status().splitlines() + h.frida.status_lines())
        [folder] = backup_folders(self.tmp)
        self.assertTrue(any(line.endswith(folder) for line in out), out)
        self.assertEqual(self.printed, [])


class McpJsonTest(unittest.TestCase):
    def test_the_ch_entry_runs_the_front_and_the_worker_without_dash_i(self):
        with open(os.path.join(_REPO_ROOT, ".mcp.json"), encoding="utf-8") as f:
            entry = json.load(f)["mcpServers"]["ch"]
        # The entry runs the front's file; after the "--" the front's main
        # splits on, the worker command runs the server's file with the same
        # launcher.
        args = entry["args"]
        split = args.index("--")
        front_args, worker = args[:split], args[split + 1:]
        self.assertEqual(len(front_args), 1, args)
        self.assertTrue(os.path.samefile(os.path.join(_REPO_ROOT, front_args[0]), F.__file__))
        self.assertEqual(worker[0], entry["command"])
        self.assertTrue(os.path.samefile(os.path.join(_REPO_ROOT, worker[-1]), S.__file__))
        self.assertNotIn("-I", args)


if __name__ == "__main__":
    unittest.main()
