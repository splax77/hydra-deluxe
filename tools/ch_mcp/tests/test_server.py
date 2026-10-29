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

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402
from tools.ch_mcp import server as S  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS  # noqa: E402

# The eighteen tools of the plan's "The tools" table.
TOOL_NAMES = {
    "ch_start", "ch_stop", "ch_status", "ch_install_chart", "ch_wait_stats",
    "ch_stats", "ch_compare", "ch_attach", "ch_detach", "ch_state",
    "ch_snapshot", "ch_trace_start", "ch_notes", "ch_trace_stop", "ch_press",
    "ch_play_chart", "ch_play_status", "ch_play_stop",
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

    def test_build_server_registers_the_eighteen_tools(self):
        tools = asyncio.run(S.build_server(self.helper).list_tools())
        self.assertEqual({t.name for t in tools}, TOOL_NAMES)

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
        # 2026-10-28-ch-mcp-server, "Live results (2026-10-29)").
        server = S.build_server(self.helper)
        tools = {t.name: t for t in asyncio.run(server.list_tools())}
        for name, text in (("instructions", server.instructions),
                           ("ch_trace_start", tools["ch_trace_start"].description)):
            with self.subTest(text=name):
                self.assertIn("each note the player hits", text)
                self.assertNotIn("processes", text)

    def test_status_joins_the_game_and_the_frida_lines(self):
        out = self.helper.ch_status().splitlines()
        self.assertIn("game: no run open, and no backup taken", out)
        self.assertEqual(out[-2:], ["frida: not attached", "trace: none"])
        self.assertEqual(self.printed, [])


class McpJsonTest(unittest.TestCase):
    def test_the_ch_entry_runs_the_server_without_dash_i(self):
        with open(os.path.join(_REPO_ROOT, ".mcp.json"), encoding="utf-8") as f:
            entry = json.load(f)["mcpServers"]["ch"]
        self.assertEqual(entry["command"], "py")
        self.assertEqual(entry["args"], ["tools/ch_mcp/server.py"])
        self.assertNotIn("-I", entry["args"])


if __name__ == "__main__":
    unittest.main()
