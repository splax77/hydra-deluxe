"""Tests for build_stamp: the stamp a build writes beside a compiled agent,
the check every loader makes, and the run's record of what it loaded.

No build: each test makes a fake agent folder in a temp folder, with a
package.json, the sources, and a bundle in frida-compile's layout whose
source map names them. `make_built_agent` is shared with the loader tests.
"""

from __future__ import annotations

import json
import os
import shutil
import sys
import tempfile
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_stamp as BS  # noqa: E402

REAL_AGENT_DIR = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent")
REAL_PACKAGE_JSON = os.path.join(REAL_AGENT_DIR, "package.json")
# A real run's prep schedule, written by hydra_replay
# (testdata/ch_runs/2026-11-12-pitch-dark-prep/schedule.json).
REAL_SCHEDULE = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "2026-11-12-pitch-dark-prep",
                             "schedule.json")
# The builds record a real run wrote (testdata/ch_runs/ls-g2-run1/builds-1.json).
REAL_BUILDS_RECORD = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-g2-run1", "builds-1.json")


def real_scripts():
    """The build scripts of the real agent package.json."""
    with open(REAL_PACKAGE_JSON, encoding="utf-8") as f:
        return json.load(f)["scripts"]


def real_agent_js_paths():
    """The built-agent path of each TypeScript source in the real agent
    folder, built or not."""
    return sorted(os.path.join(REAL_AGENT_DIR, os.path.splitext(n)[0] + ".js")
                  for n in os.listdir(REAL_AGENT_DIR) if n.endswith(".ts") and not n.endswith(".d.ts"))


def source_of(js_path):
    """The TypeScript source a built agent is compiled from."""
    return os.path.splitext(os.path.basename(js_path))[0] + ".ts"


def bundle_bytes(assets):
    """A bundle in frida-compile's layout from (name, text) pairs."""
    blobs = [(name, text.encode("utf-8")) for name, text in assets]
    header = "".join(f"{len(blob)} {name}\n" for name, blob in blobs)
    return ("📦\n" + header + "✄\n").encode("utf-8") + "\n✄\n".encode("utf-8").join(
        blob for _, blob in blobs)


def write_bundle(js_path, module_text, sources):
    """Write a fake built agent whose source map names `sources`."""
    name = "/" + os.path.basename(js_path)
    source_map = json.dumps({"version": 3, "sources": sources, "mappings": ""})
    with open(js_path, "wb") as f:
        f.write(bundle_bytes([(name, module_text), (name + ".map", source_map)]))


def make_built_agent(folder, name, sources=None, module_text=None):
    """A fresh build of agent `name` in `folder`: the real package.json, the
    sources (a dict of relative path to text; `<name>.ts` alone by default),
    the bundle naming them plus one package file, and its stamp. Returns
    the `.js` path."""
    os.makedirs(folder, exist_ok=True)
    shutil.copyfile(REAL_PACKAGE_JSON, os.path.join(folder, "package.json"))
    if sources is None:
        sources = {f"{name}.ts": f"// {name}\n"}
    for rel, text in sources.items():
        path = os.path.normpath(os.path.join(folder, rel))
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
    js = os.path.join(folder, f"{name}.js")
    write_bundle(js, module_text if module_text is not None else f"// {name}.ts\nlisten();\n",
                 ["node_modules/frida-il2cpp-bridge/dist/index.ts", *sources])
    BS.write_stamp(js)
    return js


class StampTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.agent = os.path.join(tmp.name, "agent")
        self.sources = {"state.ts": "import './menu.js';\n", "menu.ts": "// menu\n",
                        "../menu_fields.json": "{}\n"}
        self.js = make_built_agent(self.agent, "state", self.sources)

    def edit(self, rel, text):
        with open(os.path.join(self.agent, rel), "w", encoding="utf-8") as f:
            f.write(text)

    def test_a_fresh_build_passes(self):
        self.assertIsNone(BS.check(self.js))

    def test_the_stamp_holds_each_local_source_the_map_names(self):
        stamp = BS.read_stamp(self.js)
        self.assertEqual(sorted(stamp["sources"]),
                         ["../menu_fields.json", "menu.ts", "package.json", "state.ts"])
        self.assertEqual(stamp["js"], "state.js")
        self.assertTrue(os.path.isfile(os.path.join(self.agent, "state.stamp.json")))

    def test_an_edited_import_makes_the_stamp_stale(self):
        # A new import is an edit of the file that holds it.
        self.edit("state.ts", "import './menu.js';\nimport './extra.js';\n")
        self.assertEqual(BS.check(self.js),
                         f"{self.js} is stale: state.ts changed since it was built; "
                         "run npm run build:state in tools/ch_mcp/agent first")

    def test_an_edited_json_the_compile_read_makes_it_stale(self):
        self.edit("../menu_fields.json", '{"x": 1}\n')
        self.assertIn("../menu_fields.json changed since it was built", BS.check(self.js))

    def test_a_changed_package_json_makes_it_stale(self):
        # A moved dependency version changes the bundle without touching a .ts.
        with open(os.path.join(self.agent, "package.json"), "a", encoding="utf-8") as f:
            f.write("\n")
        self.assertIn("package.json changed since it was built", BS.check(self.js))

    def test_a_gone_source_makes_it_stale(self):
        os.remove(os.path.join(self.agent, "menu.ts"))
        self.assertIn("menu.ts, which it was built from, is gone", BS.check(self.js))

    def test_a_missing_stamp_refuses(self):
        os.remove(BS.stamp_path(self.js))
        self.assertEqual(BS.check(self.js),
                         f"{self.js} has no build stamp (state.stamp.json); "
                         "run npm run build:state in tools/ch_mcp/agent first")

    def test_a_missing_js_refuses(self):
        os.remove(self.js)
        self.assertEqual(BS.check(self.js),
                         f"{self.js} not found; run npm run build:state in tools/ch_mcp/agent first")

    def test_a_js_swapped_under_its_stamp_refuses(self):
        write_bundle(self.js, "// other build\n", list(self.sources))
        self.assertIn("is not the file its build stamp was written for", BS.check(self.js))

    def test_a_broken_stamp_refuses(self):
        with open(BS.stamp_path(self.js), "w", encoding="utf-8") as f:
            f.write("{not json")
        self.assertIn("cannot read the build stamp", BS.check(self.js))

    def test_the_build_command_comes_from_package_json(self):
        # Every agent source in the real agent folder: one a script builds
        # gets that script's npm command; a module the agents import gets
        # none. Every script is some agent's command.
        scripts = real_scripts()
        commanded = set()
        for js in real_agent_js_paths():
            with self.subTest(agent=os.path.basename(js)):
                try:
                    command = BS.build_command(js)
                except BS.StampError as e:
                    self.assertIn("no build script writes", str(e))
                    continue
                self.assertTrue(command.startswith("npm run "), command)
                script = command[len("npm run "):]
                self.assertIn(script, scripts)
                self.assertIn(source_of(js), scripts[script])
                commanded.add(script)
        self.assertEqual(commanded, set(scripts))

    def test_each_build_script_writes_the_stamp_after_the_compile(self):
        scripts = real_scripts()
        for js in real_agent_js_paths():
            try:
                script = BS.build_command(js)[len("npm run "):]
            except BS.StampError:
                continue
            with self.subTest(script=script):
                name = os.path.basename(js)
                steps = [step.split() for step in scripts[script].split("&&")]
                compiles = [i for i, words in enumerate(steps)
                            if words[:1] == ["frida-compile"] and name in words
                            and source_of(js) in words]
                stamps = [i for i, words in enumerate(steps)
                          if any(os.path.basename(w) == "build_stamp.py" for w in words)
                          and words[-1:] == [name]]
                self.assertEqual(len(compiles), 1)
                self.assertEqual(len(stamps), 1)
                self.assertLess(compiles[0], stamps[0])

    def test_a_bundle_with_no_source_map_writes_no_stamp(self):
        with open(self.js, "wb") as f:
            f.write(bundle_bytes([("/state.js", "// x\n")]))
        with self.assertRaises(BS.StampError) as raised:
            BS.write_stamp(self.js)
        self.assertIn("holds no source map /state.js.map", str(raised.exception))

    def test_a_file_that_is_no_bundle_writes_no_stamp(self):
        with open(self.js, "w", encoding="utf-8") as f:
            f.write("// plain js\n")
        self.assertEqual(BS.main([self.js]), 1)

    def test_bundle_module_returns_the_script_asset(self):
        self.assertEqual(BS.bundle_module(self.js), "// state.ts\nlisten();\n")


class BuildsRecordTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.agent = os.path.join(tmp.name, "agent")
        self.kit = make_built_agent(self.agent, "kit")
        self.state = make_built_agent(self.agent, "state")

    def test_builds_record_holds_each_stamp_and_the_replay_build(self):
        with open(REAL_SCHEDULE, encoding="utf-8") as f:
            schedule = json.load(f)
        record = BS.builds_record([self.kit, self.state], schedule)
        self.assertEqual([a["file"] for a in record["agents"]], ["kit.js", "state.js"])
        for entry, js in zip(record["agents"], (self.kit, self.state)):
            self.assertEqual(entry["stamp"], BS.read_stamp(js))
            self.assertEqual(entry["digest"], BS.stamp_digest(BS.read_stamp(js)))
            self.assertTrue(entry["fresh"])
        # The record names the hydra_replay that wrote the real schedule,
        # not the stand-in for a schedule with none.
        self.assertEqual(record["hydra_replay_build"], schedule["hydra_replay_build"])
        self.assertNotEqual(record["hydra_replay_build"], BS.NO_REPLAY_BUILD)
        # It holds what a real run's builds record holds, for a fresh agent.
        with open(REAL_BUILDS_RECORD, encoding="utf-8") as f:
            real = json.load(f)
        self.assertEqual(set(record), set(real))
        for entry in record["agents"]:
            self.assertEqual(set(entry), set(real["agents"][0]))

    def test_a_schedule_with_no_build_says_so(self):
        for schedule in ({}, {"hydra_replay_build": ""}, {"hydra_replay_build": None}):
            with self.subTest(schedule=schedule):
                record = BS.builds_record([self.kit], schedule)
                self.assertEqual(record["hydra_replay_build"], BS.NO_REPLAY_BUILD)
                self.assertTrue(record["hydra_replay_build"])

    def test_an_agent_with_no_stamp_says_why(self):
        os.remove(BS.stamp_path(self.state))
        entry = BS.builds_record([self.state], {})["agents"][0]
        self.assertIsNone(entry["stamp"])
        self.assertIn("has no build stamp", entry["why_no_stamp"])
        self.assertFalse(entry["fresh"])


if __name__ == "__main__":
    unittest.main()
