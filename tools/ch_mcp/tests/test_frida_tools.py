"""Tests for frida_tools: the attach waits and refusals, the state words, the
engine lines of ch_snapshot, the trace start and its refusals, ch_notes,
ch_trace_stop and the stop hook.

No game and no Frida: the fake frida module and its sessions come from
test_frida_attach, the game side is a fake with the methods FridaTools
calls, the trace is a fake standing in for trace_hits.TraceSession, the
engine's memory is a fake process the real EngineModel reads, and the clock
is a fake that sleeping moves.
"""

from __future__ import annotations

import csv
import json
import os
import re
import struct
import sys
import tempfile
import threading
import time
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_facts as B  # noqa: E402
from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import hydra_end  # noqa: E402
from tools.ch_mcp import trace_hits as TH  # noqa: E402
from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis  # noqa: E402
from tools.ch_mcp.tests.test_build_facts import REAL_FACTS  # noqa: E402
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    MENU, MENU_ONE_DESTROYED, STATE, FakeFrida, ReplySession, instant_wait)
from tools.ch_mcp.tests.test_hydra_end import FAKE  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.engine import SNAPSHOT_SIZE  # noqa: E402
from tools.ch_probe.process import decode_u32  # noqa: E402

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
# One reply the state agent sent in a real Clone Hero run, copied from the log
# ch_attach wrote there: hydra-data/ch-mcp-runs/2026-11-12/ls-g1-run1/state-1.jsonl.
RECORDED_STATE_LOG = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g1-run1",
                                  "state-1.jsonl")


def recorded_state():
    """The recorded reply's payload without its read id, which ReplySession
    gives back as the id of the read it answers."""
    with open(RECORDED_STATE_LOG, encoding="utf-8") as f:
        payload = json.loads(f.readline())["payload"]
    payload.pop("id")
    return payload
# The song end the fake dump of the test song's chart names.
SONG_END_MS = 6000.0


def setUpModule():
    FAKE.start()


def tearDownModule():
    FAKE.stop()


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


ENGINE = 0x10000


class FakeEngineProcess:
    """The game's memory as the real EngineModel reads it: one block at ENGINE
    holding the flags word and the dynamics threshold."""

    def __init__(self, flags=0x1000, threshold=64):
        self.block = bytearray(SNAPSHOT_SIZE)
        struct.pack_into("<I", self.block, C.OFF_FLAGS, flags)
        struct.pack_into("<I", self.block, C.OFF_DYN_THRESHOLD, threshold)
        self.verified = False
        self.closed = False

    def verify_targets(self):
        self.verified = True

    def read(self, addr, size):
        return bytes(self.block[addr - ENGINE:addr - ENGINE + size])

    def read_u32(self, addr):
        return decode_u32(self.read(addr, 4))

    def close(self):
        self.closed = True


class Rig(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.tmp = tmp.name
        self.agent_dir = os.path.join(self.tmp, "agent")
        for name in (FT.STATE_AGENT_NAME, TH.DEFAULT_AGENT_NAME):
            make_built_agent(self.agent_dir, name)
        self.song = os.path.join(self.tmp, "song")
        os.makedirs(self.song)
        with open(TH.manifest_path(self.song), "w", encoding="utf-8") as f:
            json.dump({"notes": [{"index": i, "time_ms": t}
                                 for i, t in enumerate((1000, 2000, 3000))]}, f)
        self.chart = os.path.join(self.song, "notes.chart")
        open(self.chart, "w").close()
        FAKE.set_end(self.chart, SONG_END_MS)
        self.clock = FakeClock()
        self.loads = 0
        self.traces = []
        self.lock = threading.RLock()
        self.proc = FakeEngineProcess()
        self.opens = []

    def open_engine_process(self, **kw):
        self.opens.append(kw)
        return self.proc

    def load_facts(self):
        self.loads += 1
        return FACTS

    def make_trace(self, *args, **kw):
        trace = FakeTrace(*args, **kw)
        self.traces.append(trace)
        return trace

    def tools(self, replies=(), game=None, load_facts=None, attached=False,
              opener=None, find_engine=None, current_build=None, real_time=False):
        """`real_time` leaves FridaTools on its own clock and its own reply
        wait (time.monotonic, time.sleep, frida_attach's wait) instead of the
        fake clock and instant_wait."""
        self.game = game if game is not None else FakeGame()
        self.game.folder = self.tmp
        self.session = ReplySession(list(replies))
        self.frida = FakeFrida(self.session)
        clock = {} if real_time else {"now": self.clock.now, "sleep": self.clock.sleep,
                                      "wait": instant_wait}
        tools = FT.FridaTools(
            self.game, self.lock,
            frida_mod=self.frida,
            current_build=current_build or (lambda: VERSION),
            load_facts=load_facts or self.load_facts,
            trace_session=self.make_trace, agent_dir=self.agent_dir,
            opener=opener or self.open_engine_process,
            find_engine=find_engine or (lambda proc: ENGINE),
            **clock)
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
        # On the real clock (time.monotonic, the clock launched_at is on), the
        # game launched longer ago than ATTACH_DELAY_S.
        reply = recorded_state()
        launched = time.monotonic() - FT.ATTACH_DELAY_S - 1.0
        tools = self.tools([reply], game=FakeGame(launched_at=launched), real_time=True)
        started = time.monotonic()
        out = tools.ch_attach()
        self.assertLess(time.monotonic() - started, FT.ATTACH_DELAY_S, out)
        self.assertNotIn("(ATTACH_DELAY_S)", out)
        self.assertEqual(self.frida.attached_to, [C.PROCESS_NAME])
        self.assertIn(f"state: {FT.state_words(reply)}", out.splitlines())

    def test_an_unknown_build_is_refused_and_nothing_attaches(self):
        # A game folder whose version file names a build no facts folder
        # describes; the real build_facts reads both.
        game_dir = os.path.join(self.tmp, "game")
        version_file = B.version_file(game_dir)
        os.makedirs(os.path.dirname(version_file))
        with open(version_file, "w", encoding="utf-8") as f:
            json.dump({"version": "v1.1.0.6200-final"}, f)
        dumps = os.path.join(self.tmp, "dumps")
        os.makedirs(dumps)
        tools = self.tools([recorded_state()], real_time=True,
                           current_build=lambda: B.current_build(game_dir),
                           load_facts=lambda: B.load_facts(game_dir, dumps))
        out = tools.ch_attach()
        self.assertIn("v1.1.0.6200-final", out)
        self.assertEqual(out.splitlines()[-1], "nothing attached")
        self.assertEqual(self.frida.attached_to, [])
        self.assertEqual(self.game.backups, 0)

    def test_the_facts_load_once_per_build(self):
        tools = self.tools([STATE, STATE])
        tools.ch_attach()
        tools.ch_detach()
        tools.ch_attach()
        self.assertEqual(self.loads, 1)

    def drop_session(self):
        """Frida's own call when the game process ends: the session's
        "detached" handler with the reason Frida gives."""
        self.session.handlers["detached"]("process-terminated", None)

    def test_a_game_that_closes_after_the_attach_is_not_attached_with_frida_reason(self):
        tools = self.tools([recorded_state()], attached=True, real_time=True)
        self.drop_session()
        out = tools.ch_state()
        self.assertTrue(out.startswith(f"state: {FT.state_words(None)} ("), out)
        self.assertIn("process-terminated", out)
        self.assertNotIn(FT.state_words(None, running=False), out)

    def test_is_attached_follows_the_attach_and_a_dropped_session(self):
        tools = self.tools([recorded_state()], real_time=True)
        self.assertFalse(tools.is_attached())
        tools.ch_attach()
        self.assertTrue(tools.is_attached())
        self.drop_session()
        self.assertFalse(tools.is_attached())

    def test_is_attached_is_false_after_a_detach(self):
        tools = self.tools([recorded_state()], attached=True, real_time=True)
        self.assertTrue(tools.is_attached())
        tools.ch_detach()
        self.assertFalse(tools.is_attached())

    def test_attaching_again_after_the_game_dropped_the_session_closes_the_old_log(self):
        reply = recorded_state()
        tools = self.tools([reply, reply], attached=True, real_time=True)
        old = tools._agent
        self.drop_session()
        out = tools.ch_attach()
        self.assertIn(f"state: {FT.state_words(reply)}", out.splitlines())
        self.assertTrue(old._log.closed)
        self.assertIsNot(tools._agent, old)

    def test_a_missing_state_agent_says_how_to_build_it(self):
        # build_stamp.check owns the refusal's words; this checks ch_attach
        # gives that refusal for the file it would load, and attaches nothing.
        path = os.path.join(self.agent_dir, FT.STATE_AGENT_FILE)
        os.remove(path)
        refusal = BS.check(path)
        self.assertIsNotNone(refusal)
        out = self.tools([recorded_state()], real_time=True).ch_attach()
        self.assertEqual(out.splitlines()[-1], refusal)
        self.assertEqual(self.frida.attached_to, [])

    def test_a_stale_stamp_refuses_naming_the_file_and_the_build_command(self):
        # A source edit after the build, then no stamp at all: ch_attach gives
        # build_stamp.check's refusal for each, and attaches nothing.
        path = os.path.join(self.agent_dir, FT.STATE_AGENT_FILE)
        with open(os.path.join(self.agent_dir, "state.ts"), "a", encoding="utf-8") as f:
            f.write("// an edit after the build\n")
        stale = BS.check(path)
        self.assertIsNotNone(stale)
        out = self.tools([recorded_state()], real_time=True).ch_attach()
        self.assertEqual(out.splitlines()[-1], stale)
        self.assertEqual(self.frida.attached_to, [])
        os.remove(BS.stamp_path(path))
        unstamped = BS.check(path)
        self.assertIsNotNone(unstamped)
        self.assertNotEqual(unstamped, stale)
        out = self.tools([recorded_state()], real_time=True).ch_attach()
        self.assertEqual(out.splitlines()[-1], unstamped)
        self.assertEqual(self.frida.attached_to, [])

    def test_a_stale_trace_agent_starts_no_trace(self):
        # ch_trace_start reaches trace_hits.TraceSession, the trace loader.
        path = os.path.join(self.agent_dir, os.path.basename(TH.DEFAULT_AGENT))
        with open(os.path.join(self.agent_dir, "trace.ts"), "a", encoding="utf-8") as f:
            f.write("// an edit after the build\n")
        with self.assertRaises(TH.AgentMissing) as raised:
            TH.TraceSession([{"index": 0, "time_ms": 1000}], "Eng", ["AddPoints"],
                            os.path.join(self.tmp, "t.jsonl"), agent_path=path,
                            song_end_ms=SONG_END_MS)
        self.assertEqual(str(raised.exception),
                         f"{path} is stale: trace.ts changed since it was built; "
                         "run npm run build:trace in tools/ch_mcp/agent first")
        os.remove(BS.stamp_path(path))
        with self.assertRaises(TH.AgentMissing) as raised:
            TH.TraceSession([{"index": 0, "time_ms": 1000}], "Eng", ["AddPoints"],
                            os.path.join(self.tmp, "t.jsonl"), agent_path=path,
                            song_end_ms=SONG_END_MS)
        self.assertIn("has no build stamp", str(raised.exception))
        out = []
        self.assertEqual(TH.main(["--song", self.song, "--mode", TH.MODE_COUNT,
                                  "--engine-class", "Eng", "--agent", path,
                                  "--out", os.path.join(self.tmp, "t.jsonl")], out=out.append), 1)
        self.assertEqual(out, [BS.check(path)])
        make_built_agent(self.agent_dir, TH.DEFAULT_AGENT_NAME)
        session = TH.TraceSession([{"index": 0, "time_ms": 1000}], "Eng", ["AddPoints"],
                                  os.path.join(self.tmp, "t.jsonl"), agent_path=path,
                            song_end_ms=SONG_END_MS)
        self.assertEqual(session.agent_path, path)


class ReadTest(Rig):
    def test_state_before_attach_says_not_attached(self):
        tools = self.tools(real_time=True)
        self.assertEqual(tools.ch_state(), f"state: {FT.state_words(None)}; {FT.NOT_ATTACHED}")
        self.assertEqual(self.frida.attached_to, [])

    def real_game_tools(self, rows=None):
        """A real game_tools.GameTools whose play_snapshot ch_snapshot reads:
        with `rows`, a finished play session holding those press rows; without,
        no play session ever ran."""
        game = GT.GameTools(None, self.lock, os.path.join(self.tmp, "scratch"))
        if rows is not None:
            game._play = GT._PlaySession(score_json="score.json", chord_count=len(rows),
                                         log_path=None, rows=rows)
        return game

    def snapshot_with(self, game):
        tools = self.tools([recorded_state(), recorded_state()], attached=True,
                           real_time=True)
        self.game.play_snapshot = game.play_snapshot
        return tools.ch_snapshot().splitlines()

    def test_snapshot_with_no_play_session_says_the_score_is_not_read(self):
        lines = self.snapshot_with(self.real_game_tools())
        score = [line for line in lines if line.startswith("score")]
        self.assertEqual(len(score), 1, lines)
        self.assertTrue(score[0].startswith("score: not read ("), score[0])
        self.assertIn("no play session", score[0])

    def test_snapshot_shows_the_score_after_the_last_press(self):
        # The rows are a real in-game run's press log
        # (testdata/ch_runs/ls-g2-run1/press-1.csv, copied from
        # hydra-data/ch-mcp-runs/2026-11-12/ls-g2-run1). The line takes the
        # last row's score and clock and counts every row.
        path = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g2-run1", "press-1.csv")
        with open(path, encoding="utf-8", newline="") as f:
            rows = list(csv.DictReader(f))
        self.assertGreater(len(rows), 1)
        self.assertNotEqual(rows[0]["score_after"], rows[-1]["score_after"])
        lines = self.snapshot_with(self.real_game_tools(rows))
        score = [line for line in lines if line.startswith("score after press ")]
        self.assertEqual(len(score), 1, lines)
        self.assertTrue(score[0].startswith(
            f"score after press {len(rows)}: {rows[-1]['score_after']}"), score[0])
        self.assertIn(f"song clock at that press: {rows[-1]['clock_ms_at_press']} ms "
                      "(the play session's clock)", lines)


class SnapshotEngineLinesTest(Rig):
    def snapshot_lines(self, flags=0x1000, threshold=64, state=STATE, **kw):
        self.proc = FakeEngineProcess(flags, threshold)
        tools = self.tools([STATE, state], attached=True, **kw)
        return tools.ch_snapshot().splitlines()

    def test_the_engine_fields_a_real_run_read_print_as_that_run_printed_them(self):
        # The engine lines of a ch_snapshot in a real run
        # (testdata/ch_runs/ls-o1-run1/replies.txt, copied from
        # hydra-data/ch-mcp-runs/2026-11-06/ls-o1-run1/replies.txt). The flags
        # word and threshold that run read from the game give back the lines
        # it printed: the word in hex, the stop bit clear because that word
        # does not hold it, the threshold, and pad_axis's ghost value for it.
        path = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-o1-run1", "replies.txt")
        with open(path, encoding="utf-8") as f:
            recorded = f.read().splitlines()
        start = next(i for i, line in enumerate(recorded) if line.startswith("flags word: "))
        printed = recorded[start:start + 4]
        flags = int(re.match(r"flags word: (0x[0-9a-f]+) ", printed[0]).group(1), 16)
        threshold = int(re.match(r"dynamics threshold: (\d+) ", printed[2]).group(1))
        self.assertFalse(flags & FT.DYNAMICS_STOP_BIT)
        self.assertEqual(FT.engine_lines(flags, threshold), printed)
        self.assertIn(repr(pad_axis(DYNAMIC_GHOST, threshold)), printed[3])

    def test_the_threshold_line_names_the_value_and_the_offsets_constant(self):
        self.assertIn("dynamics threshold: 64 (OFF_DYN_THRESHOLD, +0x2c0)",
                      self.snapshot_lines(threshold=64))

    def test_the_ghost_value_is_the_one_pad_axis_gives_at_full_precision(self):
        lines = self.snapshot_lines(threshold=64)
        self.assertIn(f"ghost axis value: {pad_axis('ghost', 64)!r} (kit_tools.pad_axis)", lines)

    def test_the_search_runs_without_the_servers_lock(self):
        seen = []

        def try_the_lock():
            taken = self.lock.acquire(blocking=False)
            seen.append(taken)
            if taken:
                self.lock.release()

        def finder(proc):
            other = threading.Thread(target=try_the_lock)
            other.start()
            other.join()
            return ENGINE

        self.snapshot_lines(find_engine=finder)
        self.assertEqual(seen, [True])


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

# One player screen as menu.ts sends it, one value per field for its one live
# instance: on the Instrument page of a drum profile, with three rows and the
# third highlighted.
PLAYER_FIELDS = {
    "page": [1], "cursor": [1], "scroll": [1], "controllerType": [2],
    "instrument": [0], "difficulty": [3], "playerIndex": [0], "firstTime": [True],
    "rowCount": [3], "visibleRows": [5], "isDrums": [True], "networkFlag": [False],
    "backLock": [False], "rowNames": [["Guitar", "Drums", "Pro Drums"]],
    "rowInstruments": [[0, 6, 9]],
}


def player_screen(fields, live=1):
    return {**MENU_READ, "classes": {"PlayerSelectionMenu": {
        "found": True, "live": live, "destroyed": 0, "active": [True] * live,
        "fields": fields}}}


class MenuFieldsTest(unittest.TestCase):
    """menu_fields.json parses, and each field and enum it names sits at the
    dump line it gives, in the dump of the build it names. The dump lives
    outside the repo, so without it the dump check is skipped."""

    def setUp(self):
        self.data = FT.menu_fields()

    def enums(self):
        return [f for f in self.data["fields"] if f.get("kind", FT.ENUM_KIND) == FT.ENUM_KIND]

    def test_it_parses_with_one_table_per_enum_field(self):
        keys = [f["key"] for f in self.data["fields"]]
        self.assertEqual(keys, [
            "page", "cursor", "scroll", "controllerType", "instrument", "difficulty",
            "playerIndex", "firstTime", "rowCount", "visibleRows", "isDrums",
            "networkFlag", "backLock", "rowNames", "rowInstruments"])
        self.assertEqual({f["key"]: f.get("kind") for f in self.data["fields"] if "kind" in f}, {
            "cursor": "int", "scroll": "int", "playerIndex": "int", "firstTime": "bool",
            "rowCount": "int", "visibleRows": "int", "isDrums": "bool",
            "networkFlag": "bool", "backLock": "bool", "rowNames": "string_array",
            "rowInstruments": "enum_array"})
        for field in self.data["fields"]:
            with self.subTest(key=field["key"]):
                self.assertEqual(field["class"], "PlayerSelectionMenu")
        for field in self.enums():
            with self.subTest(key=field["key"]):
                for number in field["values"]:
                    int(number)

    def test_the_highlighted_row_keys_are_entries(self):
        keys = {f["key"]: f.get("kind", FT.ENUM_KIND) for f in self.data["fields"]}
        self.assertEqual(keys[FT.CURSOR_KEY], "int")
        self.assertEqual(keys[FT.SCROLL_KEY], "int")
        self.assertEqual(keys[FT.ROW_NAMES_KEY], "string_array")
        self.assertEqual(keys[FT.ROW_IDS_KEY], "enum_array")

    def dump_type(self, field):
        """The type the dump should give the field, from its kind."""
        kind = field.get("kind", FT.ENUM_KIND)
        if kind == FT.ENUM_KIND:
            return field["enum"]
        if kind == "enum_array":
            words = next(f for f in self.data["fields"] if f["key"] == field["words_from"])
            return words["enum"] + "[]"
        return {"int": "int", "bool": "bool", "string_array": "string[]"}[kind]

    def test_each_name_sits_at_its_dump_line(self):
        dump = os.path.join(B.DUMPS_ROOT, self.data["build"], "dump.cs")
        if not os.path.isfile(dump):
            self.skipTest(f"{dump} is not on this machine")
        wanted = {}
        for field in self.data["fields"]:
            wanted[field["class_dump_line"]] = f"class {field['class']} "
            wanted[field["dump_line"]] = f" {field['field']}; "
            if "declared_in" in field:
                wanted[field["declared_in_dump_line"]] = f"class {field['declared_in']} "
            if "enum_dump_line" in field:
                wanted[field["enum_dump_line"]] = f"enum {field['enum']} "
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
            with self.subTest(key=field["key"]):
                # The field's line names the type its kind reads.
                self.assertIn(f" {self.dump_type(field)} {field['field']}; ",
                              found[field["dump_line"]])
                # A field read on a class it does not declare comes from that
                # class's parent, and sits inside the parent's body.
                if "declared_in" in field:
                    self.assertIn(f"class {field['class']} : {field['declared_in']} ",
                                  found[field["class_dump_line"]])
                    self.assertGreater(field["dump_line"], field["declared_in_dump_line"])
                else:
                    self.assertGreater(field["dump_line"], field["class_dump_line"])

    def test_each_enum_word_sits_in_its_enum(self):
        dump = os.path.join(B.DUMPS_ROOT, self.data["build"], "dump.cs")
        if not os.path.isfile(dump):
            self.skipTest(f"{dump} is not on this machine")
        with open(dump, encoding="utf-8") as f:
            lines = f.readlines()
        for field in self.enums():
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
        self.assertEqual(FT.menu_lines(player(9, 3))[:2], [
            "PlayerSelectionMenu: 1 live, 0 destroyed; active: [true]",
            "PlayerSelectionMenu[0]: instrument: 9 (ProDrums); difficulty: 3 (Expert)"])
        line = FT.menu_lines(player(6, 2))[1]
        self.assertIn("instrument: 6 (Drums)", line)
        self.assertIn("difficulty: 2 (Hard)", line)

    def test_player_screen_on_the_instrument_page_with_drum_rows(self):
        self.assertEqual(FT.menu_lines(player_screen(PLAYER_FIELDS))[:3], [
            "PlayerSelectionMenu: 1 live, 0 destroyed; active: [true]",
            "PlayerSelectionMenu[0]: page: 1 (Instrument); cursor: 1; scroll: 1; "
            "highlighted row: 2, name \"Pro Drums\", id 9 (ProDrums); "
            "controllerType: 2 (Drums); instrument: 0 (Guitar); difficulty: 3 (Expert); "
            "playerIndex: 0; firstTime: true; rowCount: 3; visibleRows: 5; "
            "isDrums: true; networkFlag: false; backLock: false",
            "PlayerSelectionMenu[0] rows: rowNames: [\"Guitar\", \"Drums\", \"Pro Drums\"]; "
            "rowInstruments: [0 (Guitar), 6 (Drums), 9 (ProDrums)]"])

    def test_player_screen_page_the_enum_does_not_name(self):
        line = FT.menu_lines(player_screen({**PLAYER_FIELDS, "page": [6]}))[1]
        self.assertTrue(line.startswith(
            "PlayerSelectionMenu[0]: page: 6 (not in menu_fields.json); cursor: 1; "), line)

    def test_player_screen_with_null_row_arrays(self):
        lines = FT.menu_lines(player_screen(
            {**PLAYER_FIELDS, "rowNames": [None], "rowInstruments": [None]}))
        self.assertIn("highlighted row: 2, name none (rowNames is null), "
                      "id none (rowInstruments is null); ", lines[1])
        self.assertEqual(lines[2], "PlayerSelectionMenu[0] rows: "
                         "rowNames: null (no array); rowInstruments: null (no array)")

    def test_player_screen_highlight_index_out_of_range(self):
        line = FT.menu_lines(player_screen({**PLAYER_FIELDS, "scroll": [4]}))[1]
        self.assertIn("scroll: 4; highlighted row: 5, "
                      "name none (index 5 is outside the 3 rowNames), "
                      "id none (index 5 is outside the 3 rowInstruments); ", line)
        line = FT.menu_lines(player_screen({**PLAYER_FIELDS, "cursor": [-3]}))[1]
        self.assertIn("highlighted row: -2, name none (index -2 is outside the 3 rowNames), ",
                      line)

    def test_player_screen_with_an_unread_cursor(self):
        line = FT.menu_lines(player_screen({**PLAYER_FIELDS, "cursor": ["error: gone"]}))[1]
        self.assertIn("cursor: error: gone; scroll: 1; "
                      "highlighted row: not known (cursor was not read); ", line)

    def highlighted(self, **fields):
        line = FT.menu_lines(player_screen({**PLAYER_FIELDS, **fields}))[1]
        return line.split("highlighted row: ")[1].split("; controllerType")[0]

    def test_confirm_and_difficulty_take_the_cursor_alone(self):
        # Page 0 is Confirm and 2 Difficulty: cursor 1 and scroll 1 give row 1.
        for page in (0, 2):
            self.assertEqual(self.highlighted(page=[page]),
                             "1, name \"Drums\", id 6 (Drums)", page)

    def test_instrument_and_modifiers_add_the_scroll(self):
        # Page 1 is Instrument and 3 Modifiers: cursor 1 and scroll 1 give row 2.
        for page in (1, 3):
            self.assertEqual(self.highlighted(page=[page]),
                             "2, name \"Pro Drums\", id 9 (ProDrums)", page)

    def test_an_unread_scroll_matters_only_where_the_scroll_is_added(self):
        self.assertEqual(self.highlighted(page=[0], scroll=["error: gone"]),
                         "1, name \"Drums\", id 6 (Drums)")
        self.assertEqual(self.highlighted(page=[3], scroll=["error: gone"]),
                         "not known (scroll was not read)")

    def test_the_ready_page_highlights_no_row(self):
        self.assertEqual(self.highlighted(page=[4]), "none on the Ready page")

    def test_a_page_with_no_row_rule_or_no_read(self):
        self.assertEqual(self.highlighted(page=[6]), "not known (page 6 has no row rule)")
        self.assertEqual(self.highlighted(page=["error: gone"]),
                         "not known (page was not read)")

    def test_every_page_word_has_a_row_rule(self):
        words = set(FT._enum_words(FT.PAGE_KEY).values())
        self.assertEqual(words, set(FT.ROW_ADDS_SCROLL) | {FT.NO_ROW_PAGE})

    def test_two_player_screens_get_a_line_pair_each(self):
        fields = {key: values * 2 for key, values in PLAYER_FIELDS.items()}
        fields["playerIndex"] = [0, 1]
        lines = FT.menu_lines(player_screen(fields, live=2))
        self.assertTrue(lines[3].startswith("PlayerSelectionMenu[1]: page: 1 (Instrument); "))
        self.assertIn("playerIndex: 1; ", lines[3])
        self.assertTrue(lines[4].startswith("PlayerSelectionMenu[1] rows: "))

    def test_menu_lines_print_why_the_player_screen_was_not_read(self):
        menu = {**MENU_READ, "classes": {"PlayerSelectionMenu": {
            "found": False, "reason": "the player-screen hook has not been placed yet"}}}
        self.assertEqual(FT.menu_lines(menu)[0],
                         "PlayerSelectionMenu: the player-screen hook has not been placed yet")

    def test_menu_lines_print_a_field_read_that_is_not_a_known_number_as_sent(self):
        menu = {**MENU_READ, "classes": {"PlayerSelectionMenu": {
            "found": True, "live": 2, "destroyed": 0, "active": [None, "error: x"],
            "fields": {"instrument": [99, "error: boom"], "difficulty": [-1, None]}}}}
        self.assertEqual(FT.menu_lines(menu)[:3], [
            "PlayerSelectionMenu: 2 live, 0 destroyed; active: [null, \"error: x\"]",
            "PlayerSelectionMenu[0]: instrument: 99 (not in menu_fields.json); difficulty: -1 (None)",
            "PlayerSelectionMenu[1]: instrument: error: boom; difficulty: null"])

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

    def test_trace_start_caps_at_hydras_end_of_the_folders_chart(self):
        tools = self.tools([STATE, STATE], attached=True)
        tools.ch_trace_start(self.song)
        (trace,) = self.traces
        self.assertEqual(trace.kw["song_end_ms"], SONG_END_MS)
        self.assertEqual(FAKE.charts[-1], self.chart)

    def test_a_folder_with_no_chart_starts_no_trace(self):
        os.remove(self.chart)
        tools = self.tools([STATE, STATE], attached=True)
        self.game.backups = 0
        out = tools.ch_trace_start(self.song)
        self.assertEqual(out.splitlines()[-1],
                         f"{self.song} has no notes.chart or notes.mid, so Hydra has no "
                         "song end to cap the trace at; no trace started")
        self.assertEqual(self.traces, [])
        self.assertEqual(self.game.backups, 0)

    def test_a_failed_dump_starts_no_trace(self):
        # A chart the fake has no dump for fails as hydra_replay would.
        os.remove(self.chart)
        open(os.path.join(self.song, "notes.mid"), "w").close()
        tools = self.tools([STATE, STATE], attached=True)
        self.game.backups = 0
        out = tools.ch_trace_start(self.song)
        self.assertIn("hydra_replay dump failed with exit code 1", out)
        self.assertTrue(out.endswith("; no trace started"))
        self.assertEqual(self.traces, [])
        self.assertEqual(self.game.backups, 0)

    def test_the_dump_runs_outside_the_lock(self):
        tools = self.tools([STATE, STATE], attached=True)
        held = []
        real = hydra_end.make_dump

        def dump(chart, path):
            # Another thread can take the lock while the dump runs.
            got = []

            def take():
                got.append(self.lock.acquire(timeout=1.0))
                if got[0]:
                    self.lock.release()

            t = threading.Thread(target=take)
            t.start()
            t.join()
            held.append(not got[0])
            real(chart, path)

        with mock.patch.object(hydra_end, "make_dump", dump):
            tools.ch_trace_start(self.song)
        self.assertEqual(held, [False])
        self.assertEqual(len(self.traces), 1)

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
