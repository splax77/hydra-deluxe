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
from tools.ch_mcp import menu_tools as MT  # noqa: E402
from tools.ch_mcp import trace_hits as TH  # noqa: E402
from tools.ch_mcp.kit_tools import DYNAMIC_GHOST, pad_axis  # noqa: E402
from tools.ch_mcp.tests.test_build_facts import REAL_FACTS  # noqa: E402
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_mcp.tests.test_frida_attach import (  # noqa: E402
    MENU, STATE, FakeFrida, ReplySession, instant_wait)
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
        # Each key the highlighted row is worked out from names a field
        # menu.ts reads. The two row arrays are of a kind _instance_lines puts
        # on the rows line; the cursor and scroll are of neither array kind
        # nor an enum, so they print as numbers on the plain line, where the
        # highlighted row follows the scroll.
        keys = {f["key"]: f.get("kind", FT.ENUM_KIND) for f in self.data["fields"]}
        for key in (FT.CURSOR_KEY, FT.SCROLL_KEY, FT.ROW_NAMES_KEY, FT.ROW_IDS_KEY):
            self.assertIn(key, keys)
        for key in (FT.ROW_NAMES_KEY, FT.ROW_IDS_KEY):
            self.assertIn(keys[key], FT.ARRAY_KINDS, key)
        for key in (FT.CURSOR_KEY, FT.SCROLL_KEY):
            self.assertNotIn(keys[key], FT.ARRAY_KINDS + (FT.ENUM_KIND,), key)

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
        # The menu block of a real state reply (recorded_state's run, mid-song:
        # every menu class found with no live instance, no selected song, and
        # GlobalVariables naming the song folder). Each class gets one line
        # with its counts, in the block's order, and the folder prints as sent.
        menu = recorded_state()[FT.MENU_BLOCK]
        lines = FT.menu_lines(menu)
        classes = list(menu["classes"].items())
        self.assertTrue(classes)
        self.assertEqual([line.split(":")[0] for line in lines[:len(classes)]],
                         [name for name, _ in classes])
        for line, (name, entry) in zip(lines, classes):
            with self.subTest(name=name):
                self.assertTrue(entry["found"])
                self.assertIn(f"{entry['live']} live, {entry['destroyed']} destroyed", line)
        folder = menu["globals"]["currentSongFolder"]
        self.assertTrue(folder)
        self.assertIn(f"GlobalVariables currentSongFolder: {folder}", lines)

    def test_menu_lines_with_no_selected_song(self):
        # The recorded mid-song block has no selected song, and prints one
        # line saying so in place of the folder and chartName lines.
        menu = recorded_state()[FT.MENU_BLOCK]
        self.assertIsNone(menu["selectedSong"])
        lines = FT.menu_lines(menu)
        song = [line for line in lines if line.startswith("selected song")]
        self.assertEqual(song, ["selected song: none"])

    def test_menu_lines_print_the_agents_error(self):
        # The error menu.ts's readMenu sends when the game's assembly is not
        # loaded, built from its own literal and ASSEMBLY_NAME: menu_lines
        # prints it on one menu line and reads nothing else.
        with open(os.path.join(FT.AGENT_DIR, "menu.ts"), encoding="utf-8") as f:
            source = f.read()
        name = re.search(r'const ASSEMBLY_NAME = "(\w+)";', source).group(1)
        template = re.search(r"return \{ error: `\$\{ASSEMBLY_NAME\}([^`]*)` \};",
                             source).group(1)
        error = name + template
        self.assertEqual(FT.menu_lines({"error": error}), [f"menu: {error}"])

    def test_menu_lines_with_no_block(self):
        # A reply with no menu block gives one menu line saying it was not
        # read, never the class, song or GlobalVariables lines.
        reply = recorded_state()
        reply.pop(FT.MENU_BLOCK)
        lines = FT.menu_lines(reply.get(FT.MENU_BLOCK))
        self.assertEqual(len(lines), 1, lines)
        self.assertTrue(lines[0].startswith("menu: not read ("), lines[0])
        self.assertIn("menu block", lines[0])

    def test_ch_menu_prints_the_lines_then_the_state_word(self):
        # The recorded reply (recorded_state's run, mid-song) answers the
        # attach and then ch_menu's one read: ch_menu prints that read's menu
        # block as menu_lines prints it, then the state word for the same read.
        reply = recorded_state()
        tools = self.tools([reply, reply], attached=True, real_time=True)
        self.assertEqual(tools.ch_menu().splitlines(),
                         FT.menu_lines(reply[FT.MENU_BLOCK])
                         + [f"state: {FT.state_words(reply)}"])
        self.assertEqual(FT.state_words(reply), FT.PLAYING)

    def test_ch_menu_before_attach_says_not_attached(self):
        self.assertEqual(self.tools().ch_menu(),
                         f"state: {FT.state_words(None)}; {FT.NOT_ATTACHED}")

    def test_state_leaves_the_menu_block_to_ch_menu(self):
        tools = self.tools([STATE, {**STATE, "menu": MENU_READ}], attached=True)
        out = tools.ch_state()
        self.assertIn("songTime: 3.5", out)
        self.assertNotIn("menu", out)

    def test_read_state_gives_the_song_check_the_loaded_folder(self):
        # ch_run_chart's song check reads the loaded song folder from
        # read_state through menu_tools.loaded_folder. On the recorded reply
        # that is the folder GlobalVariables named in the run's own log line.
        with open(RECORDED_STATE_LOG, encoding="utf-8") as f:
            logged = json.loads(f.readline())["payload"]
        folder = logged[FT.MENU_BLOCK]["globals"]["currentSongFolder"]
        self.assertTrue(folder)
        tools = self.tools([recorded_state(), recorded_state()], attached=True,
                           real_time=True)
        self.assertEqual(MT.loaded_folder(tools.read_state()), folder)

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

    def test_every_page_word_has_a_row_rule(self):
        words = set(FT._enum_words(FT.PAGE_KEY).values())
        self.assertEqual(words, set(FT.ROW_ADDS_SCROLL) | {FT.NO_ROW_PAGE})

    def test_menu_lines_print_why_the_player_screen_was_not_read(self):
        # Until its hook is placed, menu.ts sends the player screen as not
        # found with the reason its playerScreenHook holds, read here from
        # menu.ts itself. In the recorded menu block, that entry stands in for
        # the read one: menu_lines prints the reason on the class's own line.
        with open(os.path.join(FT.AGENT_DIR, "menu.ts"), encoding="utf-8") as f:
            source = f.read()
        screen = re.search(r'const PLAYER_SCREEN = "(\w+)";', source).group(1)
        reason = re.search(r'let playerScreenHook: string \| null = "([^"]+)";',
                           source).group(1)
        menu = recorded_state()[FT.MENU_BLOCK]
        self.assertIn(screen, menu["classes"])
        menu["classes"][screen] = {"found": False, "reason": reason}
        lines = [line for line in FT.menu_lines(menu) if line.startswith(screen)]
        self.assertEqual(lines, [f"{screen}: {reason}"])

    def test_read_state_before_attach_raises_with_the_reason(self):
        # A real game side on a temp folder, with nothing attached: the
        # reason is the not-attached line ch_state prints for the same case.
        lock = threading.RLock()
        game = GT.GameTools(None, lock, os.path.join(self.tmp, "scratch"))
        tools = FT.FridaTools(game, lock)
        with self.assertRaises(FT.StateUnread) as caught:
            tools.read_state()
        self.assertEqual(str(caught.exception), tools.ch_state())
        self.assertEqual(str(caught.exception),
                         f"state: {FT.state_words(None)}; {FT.NOT_ATTACHED}")


class StateWordsTest(unittest.TestCase):
    def test_not_running_wins_over_any_read(self):
        # When the last attach found no game, the word is the not-running one
        # whatever read is passed, the recorded mid-song reply included, and
        # it differs from the word for no read at all.
        word = FT.state_words(None, running=False)
        self.assertEqual(FT.state_words(recorded_state(), running=False), word)
        self.assertNotEqual(word, FT.state_words(None))
        self.assertNotEqual(word, FT.state_words(recorded_state()))

    def test_not_attached_is_what_a_real_game_side_shows_before_attach(self):
        # A real game side on a temp folder, never attached: ch_state's word
        # is state_words' word for no read, which is neither the not-running
        # word nor the recorded reply's word.
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        lock = threading.RLock()
        tools = FT.FridaTools(GT.GameTools(None, lock, os.path.join(tmp.name, "scratch")),
                              lock)
        word = FT.state_words(None)
        self.assertTrue(tools.ch_state().startswith(f"state: {word}; "), tools.ch_state())
        self.assertNotIn(word, {FT.state_words(None, running=False),
                                FT.state_words(recorded_state())})

    def test_playing(self):
        # Every reply the state agent sent in the recorded run was read while
        # the song played (its log holds no other state), and each reads as
        # the playing word.
        with open(RECORDED_STATE_LOG, encoding="utf-8") as f:
            replies = [json.loads(line)["payload"] for line in f if line.strip()]
        self.assertTrue(replies)
        for i, reply in enumerate(replies):
            with self.subTest(reply=i):
                self.assertTrue(reply["isSongPlaying"])
                self.assertEqual(FT.state_words(reply), FT.PLAYING)


class TraceTest(Rig):
    def test_trace_start_refuses_with_no_live_game_manager(self):
        tools = self.tools([STATE, MENU], attached=True)
        out = tools.ch_trace_start(self.song)
        self.assertEqual(out.splitlines()[-1],
                         "no live GameManager: start the song (pause it if you like), "
                         "then call ch_trace_start")
        self.assertEqual(self.traces, [])

    def test_trace_start_before_attach_refuses(self):
        # A real game side on a temp folder, never attached: the refusal is
        # the not-attached line, no trace is built and the game side holds no
        # backup afterwards.
        game = GT.GameTools(None, self.lock, os.path.join(self.tmp, "scratch"))
        tools = FT.FridaTools(game, self.lock, current_build=lambda: VERSION,
                              load_facts=self.load_facts, trace_session=self.make_trace)
        out = tools.ch_trace_start(self.song)
        self.assertEqual(out.splitlines(),
                         [f"state: {FT.state_words(None)}; {FT.NOT_ATTACHED}",
                          "no trace started"])
        self.assertEqual(self.traces, [])
        self.assertEqual(game.open_lines(), [])

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
