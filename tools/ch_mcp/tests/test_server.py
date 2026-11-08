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
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import server as S  # noqa: E402
from tools.ch_mcp import sp_tools as SP  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS  # noqa: E402

# The eighteen tools of the server plan's "The tools" table, then the eleven
# of the Star Power and song start plan's (task T5), then the five kit tools
# of the in-game input plan (task K5). The direct-start plan (task D5)
# registers the changed ch_start and ch_kit_setup and adds no tool. The optimal
# score run plan (task T4) adds ch_run_chart, the reload plan (task R3)
# ch_open_items, and the player-limits plan (task T4) ch_edge_probe. The server plan is
# docs/superpowers/plans/2026-10-28-ch-mcp-server.md (removed; git show ec91f8d1:docs/superpowers/plans/2026-10-28-ch-mcp-server.md).
TOOL_NAMES = {
    "ch_run_chart", "ch_open_items", "ch_edge_probe",
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
        self.printed = []
        env = bot_run.Env(exe=os.path.join(tmp.name, "game.exe"),
                          documents_dir=os.path.join(tmp.name, "docs"),
                          locallow_dir=os.path.join(tmp.name, "locallow"),
                          launch=_no_launch, out=self.printed.append)
        self.helper = S.Helper(env=env, scratch_root=tmp.name,
                               game_kw={"driver": object()})

    def test_build_server_registers_the_thirty_seven_tools(self):
        tools = asyncio.run(S.build_server(self.helper).list_tools())
        self.assertEqual({t.name for t in tools}, TOOL_NAMES)

    def _open_items(self):
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_open_items", {}))
        return result.content[0].text

    def test_ch_open_items_says_so_when_nothing_is_open(self):
        self.assertEqual(self._open_items(), "nothing open")

    def test_ch_open_items_names_the_run_with_its_backup_folder(self):
        self.helper.game._run = SimpleNamespace(game=SimpleNamespace(pid=7), backup_dir="the-backup")
        self.assertEqual(self._open_items(), "run: game pid 7, backup folder the-backup")

    def test_ch_open_items_names_an_adopted_backup(self):
        self.helper.game._adopted_backup = "the-backup"
        self.assertEqual(self._open_items(), "adopted backup: the-backup")

    def _front_reads(self):
        """What the front's own reader makes of the real ch_open_items reply."""
        from tools.ch_mcp import front as F
        text = self._open_items()
        message = {"result": {"content": [{"type": "text", "text": text}]}}
        link = SimpleNamespace(request=lambda method, params, cap_s=None: message)
        return F.open_items(link, 1.0)

    def test_the_front_reads_the_real_nothing_open_as_nothing_open(self):
        self.assertEqual(self._front_reads(), [])

    def test_the_front_reads_the_real_adopted_backup_line_as_open(self):
        self.helper.game._adopted_backup = "the-backup"
        self.assertEqual(self._front_reads(), ["adopted backup: the-backup"])

    def test_ch_open_items_lists_each_running_thing_on_its_own_line(self):
        alive = SimpleNamespace(is_alive=lambda: True)
        self.helper.frida._agent = SimpleNamespace(alive=True)
        self.helper.kit._agent = object()
        self.helper.game._play = GT._PlaySession("s.json", 2, None, thread=alive, rows=[{}])
        self.helper.sp._watch = SP._Watch("w.csv", 1.0, thread=alive)
        self.helper.method_watch._session = SimpleNamespace(ended=lambda: False)
        self.helper.frida._trace = SimpleNamespace(ended=lambda: False)
        self.assertEqual(self._open_items().splitlines(), [
            "play session: running, 1 presses so far", "frida: state agent attached",
            "kit: kit agent attached", "star power watch: running", "method watch: running",
            "trace: running"])

    def test_ch_open_items_leaves_out_what_has_ended(self):
        dead = SimpleNamespace(is_alive=lambda: False)
        self.helper.frida._agent = SimpleNamespace(alive=False)
        self.helper.game._play = GT._PlaySession("s.json", 2, None, thread=dead)
        self.helper.sp._watch = SP._Watch("w.csv", 1.0, thread=dead)
        self.helper.method_watch._session = SimpleNamespace(ended=lambda: True)
        self.helper.frida._trace = SimpleNamespace(ended=lambda: True)
        self.assertEqual(self._open_items(), "nothing open")

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
        # Live step L2 found the default hook fires only on hit notes (plan
        # 2026-10-28-ch-mcp-server, "Live results (2026-10-29)"; removed, read
        # it with git show ec91f8d1:docs/superpowers/plans/2026-10-28-ch-mcp-server.md).
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
                                              {"score_json": "x.json", "route": "pads"}))
        self.assertEqual(result.content[0].text, 'unknown route "pads"; routes: kit, keys')
        result = asyncio.run(server.call_tool("ch_play_chart",
                                              {"score_json": "x.json", "route": "kit"}))
        self.assertEqual(result.content[0].text,
                         'route kit: the kit is not set up; call ch_kit_setup first, '
                         'or pass route="keys"')

    def test_ch_run_chart_takes_the_plans_arguments_and_no_lead(self):
        self.assertEqual(set(self._schema("ch_run_chart")),
                         {"song_folder", "score_json", "kit_player", "decline_mode", "dump_json"})

    def test_ch_play_chart_takes_no_lead_either(self):
        self.assertEqual(set(self._schema("ch_play_chart")),
                         {"score_json", "log", "song_folder", "route", "decline_mode",
                          "dump_json", "path_index"})

    def test_ch_edge_probe_takes_its_leads_from_the_probes_file_only(self):
        self.assertEqual(set(self._schema("ch_edge_probe")),
                         {"song_folder", "score_json", "probes_json"})

    def test_ch_edge_probe_passes_its_arguments_through(self):
        seen = []
        self.helper.game.ch_edge_probe = lambda *a: seen.append(a) or "probing"
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_edge_probe", {
            "song_folder": "song", "score_json": "s.json", "probes_json": "p.json"}))
        self.assertEqual(result.content[0].text, "probing")
        self.assertEqual(seen, [("song", "s.json", "p.json")])

    def test_ch_run_chart_passes_its_arguments_through(self):
        seen = []
        self.helper.game.ch_run_chart = lambda *a: seen.append(a) or "ran"
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_run_chart", {
            "song_folder": "song", "score_json": "top.json", "kit_player": "System",
            "decline_mode": "kick_only", "dump_json": "dump.json"}))
        self.assertEqual(result.content[0].text, "ran")
        self.assertEqual(seen, [("song", "top.json", "System", "kick_only", "dump.json")])

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
        self.assertEqual(self.helper.kit.ch_kit_setup(),
                         "refused: settings.ini reads pause_on_focus_lost = 1, not 0; "
                         "start the game with ch_start, which sets it for the run")

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

    def test_ch_play_chart_passes_the_decline_arguments_through(self):
        schema = self._schema("ch_play_chart")
        self.assertEqual(schema["decline_mode"]["default"], None)
        self.assertEqual(schema["dump_json"]["default"], None)
        self.assertEqual(schema["path_index"]["default"], 0)
        seen = []
        self.helper.game.ch_play_chart = lambda *a: seen.append(a) or "playing"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_play_chart", {"score_json": "s.json"}))
        asyncio.run(server.call_tool("ch_play_chart", {
            "score_json": "s.json", "decline_mode": "kick_only",
            "dump_json": "d.json", "path_index": 2}))
        self.assertEqual(seen, [("s.json", None, None, None, None, None, 0),
                                ("s.json", None, None, None, "kick_only", "d.json", 2)])

    def test_ch_sp_compare_passes_the_dump_arguments_through(self):
        schema = self._schema("ch_sp_compare")
        self.assertEqual(schema["dump_json"]["default"], None)
        self.assertEqual(schema["path_index"]["default"], 0)
        seen = []
        self.helper.sp.ch_sp_compare = lambda *a: seen.append(a) or "compared"
        server = S.build_server(self.helper)
        asyncio.run(server.call_tool("ch_sp_compare", {"watch_csv": "w.csv",
                                                       "score_json": "s.json"}))
        asyncio.run(server.call_tool("ch_sp_compare", {
            "watch_csv": "w.csv", "score_json": "s.json", "dump_json": "d.json",
            "path_index": 2}))
        width = schema["width"]["default"]
        self.assertEqual(seen, [("w.csv", "s.json", None, None, width, None, 0),
                                ("w.csv", "s.json", None, None, width, "d.json", 2)])

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
        out = self.helper.ch_status().splitlines()
        self.assertIn("game: no run open, and no backup taken", out)
        self.assertEqual(out[-2:], ["frida: not attached", "trace: none"])
        self.assertEqual(self.printed, [])


class McpJsonTest(unittest.TestCase):
    def test_the_ch_entry_runs_the_front_and_the_worker_without_dash_i(self):
        with open(os.path.join(_REPO_ROOT, ".mcp.json"), encoding="utf-8") as f:
            entry = json.load(f)["mcpServers"]["ch"]
        self.assertEqual(entry["command"], "py")
        self.assertEqual(entry["args"], ["tools/ch_mcp/front.py", "--", "py", "tools/ch_mcp/server.py"])
        self.assertNotIn("-I", entry["args"])


if __name__ == "__main__":
    unittest.main()
