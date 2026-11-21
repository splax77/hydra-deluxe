"""Tests for method_watch.py and start_candidates.json: the start's refusals
and what it sends the agent, the shared-code addresses an "all" class skips,
and the candidate file against the dump's facts and image list. The settings
run on the real candidates file and a cut of the real build's facts. The
refusals before an attach run on the server's real game and Frida tools;
past an attach a fake run_agent plays the agent. No test attaches Frida or
starts the game."""

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

from tools.ch_mcp import build_facts  # noqa: E402
from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import frida_tools as FT  # noqa: E402
from tools.ch_mcp import game_tools as GT  # noqa: E402
from tools.ch_mcp import method_watch as MW  # noqa: E402
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402

BACKUP_LINE = "backup: taken"

# The real 1.1 build's facts (hydra-data/ch-dumps/6142/facts.json), cut to
# what the watch reads: each class start_candidates.json names, whole; for
# each address of theirs another class also runs, the first other class that
# runs it, cut to its methods at that address; and the first two other
# classes with an abstract method, cut to that one. The folder is laid out
# the way build_facts.load_facts reads it, and REAL_6142_GAME is the
# installed game's real version file.
REAL_WATCH_DUMPS = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                                "ch-dumps-6142-method-watch")
REAL_6142_GAME = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ch-dumps-6142", "game")


def real_facts():
    return build_facts.load_facts(REAL_6142_GAME, REAL_WATCH_DUMPS)


def holders(facts):
    """Each code address in `facts`, with the TypeDefIndex of every method
    that runs at it (one entry per method)."""
    held = {}
    for cls in facts["classes"]:
        for m in cls["methods"]:
            held.setdefault(m["rva"], []).append(cls["typedef_index"])
    return held


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


class FakeFrida:
    """FridaTools as MethodWatch uses it: its public attach check."""

    def __init__(self, attached=True):
        self.attached = attached

    def is_attached(self):
        return self.attached


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


class WatchTestBase(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.folder = tmp.name
        self.agent_dir = os.path.join(self.folder, "agent")
        make_built_agent(self.agent_dir, MW.AGENT_NAME)
        self.events = []
        self.game = FakeGame(self.folder, self.events)
        self.watches = []

    def make(self, payloads, *, attached=True, load_facts=real_facts):
        agent = ScriptedAgent(payloads, self.events)
        watch = MW.MethodWatch(self.game, FakeFrida(attached), threading.RLock(),
                               run_agent=agent, wait=quick_wait,
                               agent_dir=self.agent_dir, load_facts=load_facts)
        self.watches.append(watch)
        self.addCleanup(self.stop_all)
        return watch, agent

    def stop_all(self):
        for hook in self.game.hooks:
            hook()


class RealToolsTest(unittest.TestCase):
    """The watch on the server's real game and Frida tools, on a temp folder
    with no game running: nothing is attached, the state a watch meets before
    ch_attach."""

    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        lock = threading.RLock()
        game = GT.GameTools(None, lock, os.path.join(tmp.name, "scratch"))
        self.watch = MW.MethodWatch(game, FT.FridaTools(game, lock), lock)

    def test_the_start_refuses_when_nothing_is_attached(self):
        reply = self.watch.ch_method_watch_start()
        self.assertTrue(reply.startswith(FT.NOT_ATTACHED), reply)
        self.assertFalse(self.watch.running())

    def test_stop_with_no_watch(self):
        # Nothing ran, so the stop names the start tool and lists no calls.
        reply = self.watch.ch_method_watch_stop()
        self.assertIn(MW.MethodWatch.ch_method_watch_start.__name__, reply)
        self.assertNotIn(MW.call_lines([])[0], reply.splitlines())
        self.assertFalse(self.watch.running())


class StartTest(WatchTestBase):
    def test_the_start_sends_each_classes_skips_and_assembly(self):
        # What the agent gets is the settings watch_settings makes for the
        # real candidates and the real build's facts, item by item.
        watch, agent = self.make([])
        watch.ch_method_watch_start()
        watch.ch_method_watch_stop()
        self.assertIsNotNone(agent.start, "the agent was never run")
        self.assertEqual(agent.start["classes"],
                         MW.watch_settings(MW.load_candidates(), real_facts()))

    def test_the_start_refuses_without_the_builds_facts(self):
        # The real loader, on the real game's version file and a dumps
        # folder that holds no facts.
        empty = tempfile.TemporaryDirectory()
        self.addCleanup(empty.cleanup)

        def no_facts():
            return build_facts.load_facts(REAL_6142_GAME, empty.name)
        with self.assertRaises(build_facts.UnknownBuild) as caught:
            no_facts()
        watch, _ = self.make([], load_facts=no_facts)
        reply = watch.ch_method_watch_start()
        self.assertEqual(reply.splitlines()[0], str(caught.exception))
        self.assertFalse(watch.running())
        self.assertEqual(self.events, [])

    def test_a_stale_agent_starts_no_watch(self):
        watch, _ = self.make([])
        with open(os.path.join(self.agent_dir, "menuhooks.ts"), "a", encoding="utf-8") as fh:
            fh.write("// an edit after the build\n")
        refusal = BS.check(os.path.join(self.agent_dir, MW.AGENT_FILE))
        self.assertIsNotNone(refusal, "the edit did not make the build stale")
        self.assertEqual(watch.ch_method_watch_start(), refusal)
        self.assertFalse(watch.running())
        self.assertEqual(self.events, [])


class SettingsTest(unittest.TestCase):
    """watch_settings and the rules under it, on the real candidates file
    and the real build's facts."""

    @classmethod
    def setUpClass(cls):
        cls.candidates = MW.load_candidates()
        cls.facts = real_facts()
        cls.settings = {s["label"]: s
                        for s in MW.watch_settings(cls.candidates, cls.facts)}

    def test_classes_narrows_the_watch_and_an_unknown_name_is_refused(self):
        entries = self.candidates["classes"]
        last = entries[-1]["class"]
        self.assertEqual([s["label"] for s in
                          MW.watch_settings(self.candidates, self.facts, [last])], [last])
        # A real class of the build that the file does not list.
        listed = {e["class"] for e in entries}
        unknown = next(c["name"] for c in self.facts["classes"] if c["name"] not in listed)
        with self.assertRaises(MW.CandidatesError) as caught:
            MW.watch_settings(self.candidates, self.facts, [unknown])
        message = str(caught.exception)
        self.assertIn(unknown, message)
        for e in entries:
            with self.subTest(cls=e["class"]):
                self.assertIn(e["class"], message)

    def test_listed_classes_send_their_rvas_and_by_name_classes_their_names(self):
        kinds = set()
        for entry in self.candidates["classes"]:
            sent = self.settings[entry["class"]]
            kinds.add(entry["hook"])
            with self.subTest(cls=entry["class"]):
                self.assertEqual(sent["hook"], entry["hook"])
                namespace, _, name = sent["full_name"].rpartition(".")
                self.assertEqual((namespace, name),
                                 (entry.get("namespace") or "", entry["class"]))
                listed = [MW.parse_rva(m["rva"]) for m in entry.get("methods", [])]
                named = [n["name"] for n in entry.get("names", [])]
                self.assertEqual(sent["rvas"], listed if entry["hook"] == MW.HOOK_LISTED else [])
                self.assertEqual(sent["names"], named if entry["hook"] == MW.HOOK_BY_NAME else [])
        self.assertEqual(kinds, set(MW.HOOK_KINDS), "the file no longer uses every hook kind")

    def test_shared_addresses_are_those_more_than_one_class_runs(self):
        shared = MW.shared_rvas(self.facts)
        held = holders(self.facts)
        by_two_classes = {rva for rva, owners in held.items()
                          if rva != -1 and len(set(owners)) > 1}
        twice_in_one_class = {rva for rva, owners in held.items()
                              if len(owners) > 1 and len(set(owners)) == 1}
        self.assertTrue(by_two_classes, "the facts hold no address two classes run")
        self.assertTrue(twice_in_one_class, "the facts hold no address one class runs twice")
        self.assertGreater(len(set(held.get(-1, []))), 1,
                           "the facts hold no abstract method in two classes")
        self.assertEqual(shared, by_two_classes)
        self.assertFalse(shared & twice_in_one_class)
        self.assertNotIn(-1, shared)

    def test_an_all_class_skips_its_shared_addresses_and_a_listed_one_does_not(self):
        shared = MW.shared_rvas(self.facts)
        skipping = 0
        for entry in self.candidates["classes"]:
            own = {m["rva"] for m in MW.facts_class(self.facts, entry)["methods"]}
            skips = self.settings[entry["class"]]["skip_rvas"]
            with self.subTest(cls=entry["class"]):
                if entry["hook"] == MW.HOOK_ALL:
                    self.assertEqual(skips, sorted(own & shared))
                    skipping += bool(skips)
                else:
                    self.assertEqual(skips, [])
        self.assertGreater(skipping, 0, "no all class skips an address in this build")

    def test_only_scene_manager_names_an_assembly(self):
        named = 0
        for entry in self.candidates["classes"]:
            with self.subTest(cls=entry["class"]):
                self.assertEqual(self.settings[entry["class"]]["assembly"],
                                 entry.get("assembly"))
            named += "assembly" in entry
        self.assertGreater(named, 0, "the file names no assembly")

    def test_an_all_class_the_facts_lack_is_refused(self):
        entry = next(e for e in self.candidates["classes"] if e["hook"] == MW.HOOK_ALL)
        facts = {"classes": [c for c in self.facts["classes"]
                             if c["name"] != entry["class"]]}
        with self.assertRaises(MW.CandidatesError) as caught:
            MW.watch_settings(self.candidates, facts)
        self.assertIn(MW.full_name(entry), str(caught.exception))

    def test_an_empty_assembly_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "c.json")
            with open(path, "w", encoding="utf-8") as fh:
                json.dump({"classes": [{"class": "X", "hook": "all", "assembly": ""}]}, fh)
            with self.assertRaises(MW.CandidatesError):
                MW.load_candidates(path)

    def test_only_scene_manager_is_a_scene_loader(self):
        marked = {e["class"] for e in self.candidates["classes"] if e.get("scene_load")}
        self.assertTrue(marked, "the file marks no scene loader")
        self.assertEqual(MW.scene_load_classes(self.candidates), marked)

    def test_a_bad_hook_kind_is_refused(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "c.json")
            with open(path, "w", encoding="utf-8") as fh:
                json.dump({"classes": [{"class": "X", "hook": "some"}]}, fh)
            with self.assertRaises(MW.CandidatesError):
                MW.load_candidates(path)

    def test_the_build_script_builds_the_agent_the_watch_runs(self):
        # build_stamp finds the script that writes the agent; that script
        # compiles the agent's source and stamps what it wrote.
        command = BS.build_command(os.path.join(FT.AGENT_DIR, MW.AGENT_FILE))
        with open(os.path.join(FT.AGENT_DIR, BS.PACKAGE_JSON), encoding="utf-8") as fh:
            words = json.load(fh)["scripts"][command.split()[-1]].split()
        self.assertIn(f"{MW.AGENT_NAME}.ts", words)
        stamper = [i for i, w in enumerate(words)
                   if os.path.basename(w) == os.path.basename(BS.__file__)]
        self.assertEqual(len(stamper), 1, words)
        self.assertEqual(words[stamper[0] + 1:], [MW.AGENT_FILE])


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
        return MW.facts_class(self.facts, entry)

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

    def test_the_addresses_each_class_skips_in_this_build(self):
        # On the installed build's whole facts: an "all" class skips only
        # addresses another class also runs, and a listed or by_name class
        # skips none of the methods it hooks.
        held = holders(self.facts)
        skipped = 0
        for s in MW.watch_settings(self.candidates, self.facts):
            entry = next(e for e in self.candidates["classes"] if e["class"] == s["label"])
            index = self.facts_class(entry)["typedef_index"]
            with self.subTest(cls=s["label"]):
                if entry["hook"] != MW.HOOK_ALL:
                    self.assertEqual(s["skip_rvas"], [])
                for rva in s["skip_rvas"]:
                    self.assertTrue(set(held[rva]) - {index}, hex(rva))
            skipped += len(s["skip_rvas"])
        self.assertGreater(skipped, 0, "no class skips an address in this build")

    def test_each_named_assembly_holds_its_class_in_the_dump(self):
        dump = os.path.join(os.path.dirname(build_facts.facts_path(
            self.candidates["game_version"])), "dump.cs")
        if not os.path.isfile(dump):
            self.skipTest(f"{dump} is not on this machine")
        images = dump_images(dump)
        for entry in self.candidates["classes"]:
            if "assembly" not in entry:
                continue
            with self.subTest(cls=entry["class"]):
                index = self.facts_class(entry)["typedef_index"]
                holder = [name for name, first in images if first <= index][-1]
                self.assertEqual(holder, entry["assembly"] + ".dll")


# A dump.cs header line: the image's file name and the TypeDefIndex of its
# first class. The images are listed in TypeDefIndex order.
_IMAGE_LINE = re.compile(r"^// Image \d+: (?P<name>\S+) - (?P<first>\d+)$")


def dump_images(path):
    """The (file name, first TypeDefIndex) of each image dump.cs lists in its
    header, which ends at the first line that is not an image line."""
    images = []
    with open(path, encoding="utf-8") as fh:
        for line in fh:
            m = _IMAGE_LINE.match(line.rstrip("\n"))
            if m is None:
                break
            images.append((m.group("name"), int(m.group("first"))))
    return images


if __name__ == "__main__":
    unittest.main()
