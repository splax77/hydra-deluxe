"""Tests for method_watch.py and start_candidates.json: the start's refusals
and backup, the stop reply's call list and scene-load line, the calls log, and
the candidate file against the dump's facts. A fake run_agent plays the agent:
no test attaches Frida or starts the game."""

from __future__ import annotations

import json
import os
import sys
import tempfile
import threading
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_facts  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import method_watch as MW  # noqa: E402

BACKUP_LINE = "backup: taken"


def quick_wait(event, timeout_s):
    # A real wait, cut short so a test never sits out a cap.
    return event.wait(min(timeout_s, 0.05))


class FakeGame:
    """The three game_tools.GameTools methods MethodWatch calls."""

    def __init__(self, folder, events):
        self.folder = folder
        self.events = events
        self.hooks = []

    def session_dir(self):
        return self.folder

    def ensure_backup(self):
        self.events.append("backup")
        return BACKUP_LINE

    def add_stop_hook(self, hook):
        self.hooks.append(hook)


class FakeLiveAgent:
    def __init__(self, alive):
        self.alive = alive


class FakeFrida:
    """FridaTools as the default attach check reads it."""

    def __init__(self, agent=None):
        self._agent = agent


class ScriptedAgent:
    """Plays menuhooks.js for run_agent: sends the scripted payloads, then
    waits the way run_agent does until the watch is stopped or capped."""

    def __init__(self, payloads, events):
        self.payloads = payloads
        self.events = events
        self.start = None
        self.cap_s = None
        self.agent_path = None

    def __call__(self, agent_path, cap_s, on_message, out_path, *, start,
                 frida_mod, wait, out):
        self.events.append("hooks")
        self.agent_path = agent_path
        self.start = start
        self.cap_s = cap_s
        with open(out_path, "w", encoding="utf-8") as fh:
            for p in self.payloads:
                fh.write(json.dumps({"type": "send", "payload": p}) + "\n")
        for p in self.payloads:
            reason = on_message(p)
            if reason is not None:
                return FA.RunResult(reason=reason, cap_s=cap_s,
                                    payloads=list(self.payloads),
                                    error=p.get("error"))
        ended = threading.Event()
        if not wait(ended, cap_s):
            return FA.RunResult(reason=FA.END_CAP, cap_s=cap_s,
                                payloads=list(self.payloads))
        return FA.RunResult(reason=FA.END_CAP, cap_s=cap_s, payloads=list(self.payloads))


# What menuhooks.ts sends: the hook summary, then one message per call.
SUMMARY = {"hooked": 3, "classes": [{"class": "GlobalVariables", "hooked": 1},
                                    {"class": "PlayerSelectionCanvas", "hooked": 1},
                                    {"class": "SceneManager", "hooked": 1}],
           "missing": [], "no_code": [], "not_hooked": []}
M3_CALL = {"call": "ʺʵʸʸʽʶʾˀʶʺʺ", "class": "GlobalVariables", "names": ["ʺʵʸʸʽʶʾˀʶʺʺ"],
           "rva": "0x235ad0",
           "args": ["C:\\Clone Hero\\songs\\Hydra Probe\\Window Map", "ˁʻʽʷʽʻʾʵʷˀʶ",
                    "ref string", "ref string", "ref string"],
           "songTime": None, "t_ms": 1000}
M4_CALL = {"call": "ʽʶʼˀʷʺʳʻʶˁʾ", "class": "PlayerSelectionCanvas", "names": ["ʽʶʼˀʷʺʳʻʶˁʾ"],
           "rva": "0xd0790", "args": [], "songTime": None, "t_ms": 1250}
LOAD_CALL = {"call": "LoadScene", "class": "SceneManager", "names": ["LoadScene"],
             "rva": "0x289c250", "args": ["Gameplay"], "songTime": None, "t_ms": 1300}


class WatchTestBase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.folder = tmp.name
        self.agent_dir = os.path.join(self.folder, "agent")
        os.mkdir(self.agent_dir)
        with open(os.path.join(self.agent_dir, MW.AGENT_FILE), "w", encoding="utf-8") as fh:
            fh.write("// compiled agent stand-in\n")
        self.events = []
        self.game = FakeGame(self.folder, self.events)
        self.watches = []

    def make(self, payloads, *, frida=None, attached=True):
        agent = ScriptedAgent(payloads, self.events)
        kw = {}
        if frida is None:
            frida = FakeFrida()
            kw["attached"] = lambda: attached
        watch = MW.MethodWatch(self.game, frida, threading.RLock(),
                               run_agent=agent, wait=quick_wait,
                               agent_dir=self.agent_dir, **kw)
        self.watches.append(watch)
        self.addCleanup(self.stop_all)
        return watch, agent

    def stop_all(self):
        for hook in self.game.hooks:
            hook()


class StartTest(WatchTestBase):
    def test_the_start_refuses_when_nothing_is_attached(self):
        watch, agent = self.make([SUMMARY], frida=FakeFrida(agent=None))
        reply = watch.ch_method_watch_start()
        self.assertEqual(reply, f"{FT.NOT_ATTACHED}; no method watch started")
        self.assertEqual(self.events, [])

    def test_the_start_refuses_a_dropped_attach(self):
        watch, _ = self.make([SUMMARY], frida=FakeFrida(agent=FakeLiveAgent(alive=False)))
        self.assertIn(FT.NOT_ATTACHED, watch.ch_method_watch_start())
        self.assertEqual(self.events, [])

    def test_a_live_attach_lets_the_start_through(self):
        watch, _ = self.make([SUMMARY], frida=FakeFrida(agent=FakeLiveAgent(alive=True)))
        self.assertIn("hooked: 3 code addresses", watch.ch_method_watch_start())

    def test_the_backup_comes_before_the_hooks(self):
        watch, _ = self.make([SUMMARY])
        reply = watch.ch_method_watch_start()
        self.assertEqual(self.events, ["backup", "hooks"])
        self.assertEqual(reply.splitlines()[0], BACKUP_LINE)

    def test_the_start_refuses_while_a_watch_runs(self):
        watch, _ = self.make([SUMMARY])
        watch.ch_method_watch_start()
        self.assertEqual(watch.ch_method_watch_start(),
                         "a method watch is already running; call ch_method_watch_stop first")
        self.assertEqual(self.events, ["backup", "hooks"])

    def test_the_start_names_a_missing_class_and_rva(self):
        summary = dict(SUMMARY, missing=["class CliGameLauncher found in 0 images",
                                         "GlobalVariables has no method at RVA 0x235ad0"])
        watch, _ = self.make([summary])
        lines = watch.ch_method_watch_start().splitlines()
        self.assertIn("not found: class CliGameLauncher found in 0 images", lines)
        self.assertIn("not found: GlobalVariables has no method at RVA 0x235ad0", lines)

    def test_the_start_sends_the_candidate_classes_and_the_cap(self):
        watch, agent = self.make([SUMMARY])
        watch.ch_method_watch_start()
        labels = [c["label"] for c in agent.start["classes"]]
        candidates = MW.load_candidates()
        self.assertEqual(labels, [c["class"] for c in candidates["classes"]])
        self.assertEqual(agent.cap_s, 120.0)
        self.assertEqual(agent.agent_path, os.path.join(self.agent_dir, "menuhooks.js"))

    def test_classes_narrows_the_watch_and_an_unknown_name_is_refused(self):
        watch, agent = self.make([SUMMARY])
        watch.ch_method_watch_start(classes=["SceneManager"])
        self.assertEqual([c["label"] for c in agent.start["classes"]], ["SceneManager"])
        watch.ch_method_watch_stop()
        reply = watch.ch_method_watch_start(classes=["Nope"])
        self.assertTrue(reply.startswith("not in start_candidates.json: Nope;"), reply)
        self.assertIn("no method watch started", reply)

    def test_an_agent_error_ends_the_watch_and_says_why(self):
        watch, _ = self.make([{"error": "no method of the watched classes could be hooked",
                               "classes": [], "missing": ["class X found in 0 images"]}])
        lines = watch.ch_method_watch_start().splitlines()
        self.assertIn("nothing hooked: no method of the watched classes could be hooked", lines)
        self.assertIn("not found: class X found in 0 images", lines)
        self.assertIn("ended: error; error: no method of the watched classes could be hooked",
                      lines)

    def test_a_missing_agent_says_how_to_build_it(self):
        watch, _ = self.make([SUMMARY])
        os.remove(os.path.join(self.agent_dir, MW.AGENT_FILE))
        reply = watch.ch_method_watch_start()
        self.assertIn("npm run build:menuhooks", reply)
        self.assertEqual(self.events, [])


class StopTest(WatchTestBase):
    def test_stop_with_no_watch(self):
        watch, _ = self.make([SUMMARY])
        self.assertEqual(watch.ch_method_watch_stop(),
                         "no method watch has run; call ch_method_watch_start")

    def test_the_stop_lists_the_calls_in_order_and_names_the_one_before_the_load(self):
        watch, _ = self.make([SUMMARY, M3_CALL, M4_CALL, LOAD_CALL])
        watch.ch_method_watch_start()
        lines = watch.ch_method_watch_stop().splitlines()
        self.assertEqual(lines[0], "ended: stopped")
        start = lines.index("calls: 3")
        self.assertEqual(lines[start + 1:start + 4], [
            '  1. GlobalVariables ʺʵʸʸʽʶʾˀʶʺʺ (0x235AD0) args '
            '["C:\\\\Clone Hero\\\\songs\\\\Hydra Probe\\\\Window Map", "ˁʻʽʷʽʻʾʵʷˀʶ", '
            '"ref string", "ref string", "ref string"]; songTime null; +0 ms',
            "  2. PlayerSelectionCanvas ʽʶʼˀʷʺʳʻʶˁʾ (0xD0790) args []; songTime null; +250 ms",
            '  3. SceneManager LoadScene (0x289C250) args ["Gameplay"]; songTime null; +300 ms',
        ])
        self.assertIn('  call 3: SceneManager LoadScene (0x289C250) scene "Gameplay"; '
                      "last before it: call 2, PlayerSelectionCanvas ʽʶʼˀʷʺʳʻʶˁʾ (0xD0790), "
                      "args []", lines)

    def test_a_load_that_calls_another_load_names_the_call_before_both(self):
        inner = dict(LOAD_CALL, call="LoadSceneAsync", names=["LoadSceneAsync"],
                     rva="0x289c0e0", t_ms=1301)
        watch, _ = self.make([SUMMARY, M4_CALL, LOAD_CALL, inner])
        watch.ch_method_watch_start()
        lines = watch.ch_method_watch_stop().splitlines()
        self.assertIn('  call 3: SceneManager LoadSceneAsync (0x289C0E0) scene "Gameplay"; '
                      "last before it: call 1, PlayerSelectionCanvas ʽʶʼˀʷʺʳʻʶˁʾ (0xD0790), "
                      "args []", lines)

    def test_no_load_is_said(self):
        watch, _ = self.make([SUMMARY, M4_CALL])
        watch.ch_method_watch_start()
        self.assertIn("scene loads: none seen", watch.ch_method_watch_stop().splitlines())

    def test_the_calls_log_holds_one_line_per_call(self):
        watch, _ = self.make([SUMMARY, M3_CALL, M4_CALL, LOAD_CALL])
        watch.ch_method_watch_start()
        watch.ch_method_watch_stop()
        path = os.path.join(self.folder, "method_watch-1.jsonl")
        with open(path, encoding="utf-8") as fh:
            rows = [json.loads(line) for line in fh]
        self.assertEqual(rows, [M3_CALL, M4_CALL, LOAD_CALL])

    def test_the_game_stop_ends_a_running_watch(self):
        watch, _ = self.make([SUMMARY, M4_CALL])
        watch.ch_method_watch_start()
        self.assertEqual(self.game.hooks[0](), "method watch: ended: stopped, 1 calls")
        self.assertIsNone(self.game.hooks[0]())


class SettingsTest(unittest.TestCase):
    def test_listed_classes_send_their_rvas_and_by_name_classes_their_names(self):
        settings = {s["label"]: s for s in MW.watch_settings(MW.load_candidates())}
        self.assertEqual(settings["GlobalVariables"]["rvas"], [0x235AD0])
        self.assertEqual(settings["CliGameLauncher"]["rvas"], [0x395850, 0x398D30, 0x398180])
        self.assertEqual(settings["SceneManager"]["names"], ["LoadScene", "LoadSceneAsync"])
        self.assertEqual(settings["SceneManager"]["full_name"],
                         "UnityEngine.SceneManagement.SceneManager")
        for label in ("SongSelect", "PlayerSelectionCanvas", "PlayerSelectionMenu"):
            with self.subTest(label=label):
                self.assertEqual(settings[label]["hook"], "all")
                self.assertEqual(settings[label]["rvas"], [])

    def test_only_scene_manager_is_a_scene_loader(self):
        self.assertEqual(MW.scene_load_classes(MW.load_candidates()), {"SceneManager"})

    def test_a_bad_hook_kind_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "c.json")
            with open(path, "w", encoding="utf-8") as fh:
                json.dump({"classes": [{"class": "X", "hook": "some"}]}, fh)
            with self.assertRaises(MW.CandidatesError):
                MW.load_candidates(path)

    def test_the_build_script_builds_the_agent_the_watch_runs(self):
        with open(os.path.join(FT.AGENT_DIR, "package.json"), encoding="utf-8") as fh:
            scripts = json.load(fh)["scripts"]
        self.assertEqual(scripts[f"build:{MW.AGENT_NAME}"],
                         f"frida-compile -o {MW.AGENT_FILE} {MW.AGENT_NAME}.ts")


class CandidatesAgainstFactsTest(unittest.TestCase):
    """Every RVA and name in start_candidates.json is in the dump's facts for
    its class. The facts are not checked in; build_facts owns where they are,
    and the test skips where they are absent (CI has no dump)."""

    @classmethod
    def setUpClass(cls):
        cls.candidates = MW.load_candidates()
        path = build_facts.facts_path(cls.candidates["game_version"])
        if not os.path.isfile(path):
            raise unittest.SkipTest(f"no facts file at {path} (the dump is not on this "
                                    "machine)")
        with open(path, encoding="utf-8") as fh:
            cls.facts = json.load(fh)

    def facts_class(self, entry):
        found = [c for c in self.facts["classes"]
                 if c.get("name") == entry["class"]
                 and (c.get("namespace") or "") == (entry.get("namespace") or "")]
        self.assertEqual(len(found), 1, entry["class"])
        return found[0]

    def test_the_facts_describe_the_files_version(self):
        self.assertTrue(build_facts.same_build(self.facts["game_version"],
                                               self.candidates["game_version"]))

    def test_every_rva_is_a_method_of_its_class_with_its_name(self):
        for entry in self.candidates["classes"]:
            by_rva = {m.get("rva"): m["name"] for m in self.facts_class(entry)["methods"]}
            for m in entry.get("methods", []):
                with self.subTest(cls=entry["class"], rva=m["rva"]):
                    rva = MW.parse_rva(m["rva"])
                    self.assertIn(rva, by_rva)
                    self.assertEqual(by_rva[rva], m["name"])

    def test_every_name_is_a_method_of_its_class(self):
        for entry in self.candidates["classes"]:
            names = {m["name"] for m in self.facts_class(entry)["methods"]}
            for n in entry.get("names", []):
                with self.subTest(cls=entry["class"], name=n["name"]):
                    self.assertIn(n["name"], names)


if __name__ == "__main__":
    unittest.main()
