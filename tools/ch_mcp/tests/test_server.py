"""Tests for the Clone Hero MCP server: the tool list, the reply cut and its
closing line, ch_status joining both halves, and the .mcp.json entry.

No game and no Frida: the helper is built on a bot_run.Env whose launch
refuses, and no test calls a tool that touches the game.
"""

from __future__ import annotations

import asyncio
import inspect
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
from tools.ch_mcp import front as F  # noqa: E402
from tools.ch_mcp import game_files  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import kit_tools as KT  # noqa: E402
from tools.ch_mcp import server as S  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS  # noqa: E402
# A real Star Power watch piece of Pitch Dark's run, the chart, and Hydra's
# files for it in testdata/replay.
from tools.ch_probe.tests.test_sp_compare import (  # noqa: E402
    _PITCH_DARK_CHART, _RISE_CSV, _midi_resolution, _replay_file)

# The scorestats.json the game wrote after a real Pitch Dark run, copied from
# hydra-data\ch-mcp-runs\2026-11-12\pitch-dark-run1.
_PITCH_DARK_STATS = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "pitch-dark-run1",
                                 "scorestats-CHON_-_Pitch_Dark_B-20261112-230618.json")

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

    def _tool_methods(self) -> dict:
        """Each ch_ method of the Helper and of the tool parts it holds, by
        name: the methods the server's tools call. The Helper's own method
        comes first where a part has one of the same name (ch_status)."""
        h = self.helper
        found = {}
        for owner in [h] + [v for k, v in vars(h).items() if not k.startswith("_")]:
            for name in dir(type(owner)):
                if name.startswith("ch_") and callable(getattr(owner, name)):
                    found.setdefault(name, getattr(owner, name))
        return found

    def test_build_server_registers_a_tool_for_each_ch_method_of_its_parts(self):
        names = {t.name for t in asyncio.run(S.build_server(self.helper).list_tools())}
        methods = self._tool_methods()
        self.assertTrue(methods)
        # Every ch_ method of a part is a tool, and every tool but one calls
        # the method of its own name. ch_open_items answers from
        # Helper.open_items, which test_ch_open_items_* read.
        self.assertEqual(set(methods) - names, set())
        self.assertEqual(names - set(methods), {"ch_open_items"})
        self.assertTrue(callable(self.helper.open_items))

    def _reaches(self, tool: str, method, cases: list) -> None:
        """The tool's schema names the method's arguments, with the method's
        defaults, and each case's arguments give the reply the method itself
        gives them, capped as the server caps it. The cases' replies all
        differ, so each argument a case changes is one the method reads."""
        params = inspect.signature(method).parameters
        schema = self._schema(tool)
        self.assertEqual(set(schema), set(params))
        for arg, spec in schema.items():
            if "default" in spec:
                self.assertEqual(spec["default"], params[arg].default, arg)
        server = S.build_server(self.helper)
        replies = []
        for args in cases:
            with self.subTest(tool=tool, args=args):
                want = S.capped(method(**args))
                got = asyncio.run(server.call_tool(tool, args)).content[0].text
                self.assertEqual(got, want)
                replies.append(got)
        self.assertEqual(len(set(replies)), len(cases), replies)

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

    def test_a_long_reply_is_cut_at_a_line_with_the_clone_hero_closing_line(self):
        long_text = "\n".join(f"call {i}: " + "x" * 90 for i in range(200))
        self.helper.frida.ch_notes = lambda since: long_text
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_notes", {"since": 0}))
        text = result.content[0].text
        self.assertLessEqual(len(text), REPLY_CAP_CHARS)
        lines = text.splitlines()
        kept = len(lines) - 1
        self.assertTrue(all(line in long_text.splitlines() for line in lines[:-1]))
        self.assertEqual(lines[-1], f"[cut at {REPLY_CAP_CHARS:,} characters: "
                                    f"{200 - kept} of 200 lines dropped; {S.CAP_HINT}]")
        self.assertIn("ch_notes(since=...)", S.CAP_HINT)

    def test_the_trace_texts_say_the_hook_fires_on_hits(self):
        # The texts rest on IN-19 in docs/clone-hero/07-internals.md, the
        # Clone Hero page's entry for the traced method.
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

    def _pitch_dark_schedule(self) -> tuple[str, str, int]:
        """Hydra's score and schedule of Pitch Dark's path 0 in testdata/replay
        (test_sp_compare's _replay_file says how they are made), and the path
        index the schedule says it was made from."""
        schedule = _replay_file("test_pitch_dark_ratio-schedule.json")
        index = GT.schedule_doc(schedule)["path_source"]["index"]
        return _replay_file("test_pitch_dark_ratio-score.json"), schedule, index

    def test_ch_run_chart_hands_its_schedule_and_path_to_the_run(self):
        # With no game open, the real run stops at the schedule's checks:
        # another path than the schedule's is refused, and the schedule's own
        # path goes on to the chart check, which this schedule
        # fails because it names its chart by a relative path. song_folder,
        # kit_player and probes_json are read only after those checks.
        score, schedule, index = self._pitch_dark_schedule()
        base = {"song_folder": os.path.dirname(_PITCH_DARK_CHART), "score_json": score,
                "schedule_json": schedule}
        self._reaches("ch_run_chart", self.helper.game.ch_run_chart, [
            {**base, "path_index": index}, {**base, "path_index": index + 1}])

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

    def test_ch_kit_frame_probe_hands_its_seconds_to_the_kit(self):
        # Seconds over the kit's cap are refused before any kit is needed,
        # naming the seconds given.
        cap = KT.FRAME_PROBE_SECONDS
        self._reaches("ch_kit_frame_probe", self.helper.kit.ch_kit_frame_probe,
                      [{"seconds": cap * 2}, {"seconds": cap * 3}])

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

    def test_ch_start_hands_human_to_the_game(self):
        # human with no song folder is refused before the quiet check; the
        # same call without human would go on to check the machine.
        self._reaches("ch_start", self.helper.game.ch_start, [{"human": True}])
        self.assertEqual(self.printed, [])

    def test_ch_play_chart_hands_its_route_and_path_to_the_game(self):
        # With no kit set up, no route and the kit route are refused with
        # different words, and the keys route reaches the schedule's path
        # check. log and song_folder are read only once a game runs.
        score, schedule, index = self._pitch_dark_schedule()
        base = {"score_json": score, "schedule_json": schedule}
        self._reaches("ch_play_chart", self.helper.game.ch_play_chart, [
            base,
            {**base, "route": GT.ROUTE_KIT},
            {**base, "route": GT.ROUTE_KEYS, "path_index": index + 1},
            {**base, "route": GT.ROUTE_KEYS, "path_index": index + 2},
        ])

    def test_ch_open_song_hands_its_player_to_the_menu_driver(self):
        # A player with only one of its two words is refused before any
        # press, naming the player given.
        song = os.path.dirname(_PITCH_DARK_CHART)
        instrument, difficulty = bot_run.DEFAULT_PLAYER.split(",")
        self._reaches("ch_open_song", self.helper.menu.ch_open_song, [
            {"song_folder": song, "player": instrument},
            {"song_folder": song, "player": difficulty},
        ])

    def test_ch_sp_compare_hands_the_recorded_files_and_resolution_to_the_compare(self):
        # A real watch piece against Hydra's files for its chart: refused
        # with no resolution, compared with the chart's own.
        score, schedule, _index = self._pitch_dark_schedule()
        base = {"watch_csv": _RISE_CSV, "score_json": score, "schedule_json": schedule}
        self._reaches("ch_sp_compare", self.helper.sp.ch_sp_compare, [
            base, {**base, "resolution": _midi_resolution(_PITCH_DARK_CHART)}])

    def test_ch_compare_hands_the_run_settings_to_the_compare(self):
        # The game's real scorestats.json against Hydra's score, with and
        # without the run's settings as that file holds them.
        with open(_PITCH_DARK_STATS, encoding="utf-8") as f:
            stats = json.load(f)
        [player] = stats["players"]
        score = _replay_file("test_pitch_dark_ratio-score.json")
        base = {"stats_path": _PITCH_DARK_STATS, "score_json": score}
        self._reaches("ch_compare", self.helper.game.ch_compare, [base, {
            **base, "checksum": stats["checksum"], "playback_speed": stats["playback_speed"],
            "no_fail": player["no_fail"], "modifiers": player["modifiers"]}])

    def test_ch_start_human_with_no_song_folder_is_refused_before_any_launch(self):
        result = asyncio.run(S.build_server(self.helper).call_tool("ch_start", {"human": True}))
        self.assertEqual(result.content[0].text, bot_run.HUMAN_NEEDS_SONG)
        self.assertEqual(self.printed, [])

    def test_ch_kit_setup_hands_its_player_to_the_kit(self):
        # With no game attached, setup is refused; a given player is named
        # first in the reply.
        self._reaches("ch_kit_setup", self.helper.kit.ch_kit_setup,
                      [{}, {"player": KT.KIT_PLAYER}])

    def test_ch_kit_press_hands_its_hold_frames_and_value_to_the_kit(self):
        # A hold and a value out of range are refused before any kit is
        # needed, each naming its own argument; a press within range then
        # needs the kit.
        item = KT.FRAME_PROBE_ITEM
        self._reaches("ch_kit_press", self.helper.kit.ch_kit_press, [
            {"items": item}, {"items": item, "hold_frames": 0},
            {"items": item, "value": 2.0}])

    def test_ch_kit_press_names_each_of_its_arguments_in_its_description(self):
        tool = {t.name: t for t in asyncio.run(S.build_server(self.helper).list_tools())
                }["ch_kit_press"]
        args = tool.input_schema["properties"]
        self.assertTrue(args)
        for arg in args:
            with self.subTest(arg):
                self.assertIn(arg, tool.description)

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
