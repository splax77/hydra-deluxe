"""Tests for agent/launch_core.ts, the launch agent's decision: both byte
checks pass before anything is written, the write is read back, the loader's
entry is hooked only after the write, and any difference writes nothing and
hooks nothing.

No game and no Frida. Plain Node runs launch_core.ts (Node erases its type
annotations), with a fake memory standing in for the game's: each case goes
to a small driver on stdin and the driver prints what applyPatch returned and
every call it made on the memory. The tests skip, naming the reason, when
`node` is not on PATH.
"""

from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
CORE = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", "launch_core.ts")
PATCH_FILE = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "launch_patch.json")
NODE = shutil.which("node")
# One driver run is a fresh Node start; this is its stop.
NODE_CAP_S = 30

# The driver: reads one case from stdin, runs it against launch_core.ts and
# prints the result as JSON. The fake memory maps an RVA's text to the bytes
# found there; a write replaces them unless the case says the write does not
# stick.
DRIVER = """
import { readFileSync } from "node:fs";
const core = await import(%(core_url)s);
const c = JSON.parse(readFileSync(0, "utf8"));
const out = {};
if (c.op === "apply") {
    const memory = new Map(Object.entries(c.memory));
    const calls = [];
    let hook = null;
    const io = {
        readBytes(rva, n) {
            calls.push(["read", rva, n]);
            if (c.read_throws) { throw new Error("read refused"); }
            return (memory.get(rva) ?? []).slice(0, n);
        },
        writeBytes(rva, bytes) {
            calls.push(["write", rva, bytes]);
            if (c.write_sticks !== false) { memory.set(rva, bytes); }
        },
        hookEntry(rva, onEnter) {
            calls.push(["hook", rva]);
            hook = onEnter;
        },
    };
    const status = core.applyPatch(core.readPatch(c.patch), io, c.module,
                                   () => c.now);
    for (let i = 0; i < (c.loader_runs ?? 0); i++) { hook(); }
    out.status = status;
    out.calls = calls;
} else if (c.op === "read_patch") {
    try { out.patch = core.readPatch(c.patch); }
    catch (e) { out.error = e.message; }
} else if (c.op === "waiting") {
    out.status = core.waitingStatus();
}
process.stdout.write(JSON.stringify(out));
"""

LOADER = "0x10"
SITE = "0x20"
PATCH = {
    "module": "GameAssembly.dll",
    "loader_rva": LOADER,
    "entry_expect": "40 56 57",
    "patch_rva": SITE,
    "expect": "BA 01 00 00 00",
    "write": "BA 00 00 00 00",
    "defer": False,
}
ENTRY = [0x40, 0x56, 0x57]
SITE_BYTES = [0xBA, 0x01, 0x00, 0x00, 0x00]
WRITTEN = [0xBA, 0x00, 0x00, 0x00, 0x00]
MODULE = {"base": "0x7ff800000000", "size": 4096}
NOW = 1761900000000


def run(case: dict) -> dict:
    driver = DRIVER % {"core_url": json.dumps(pathlib.Path(CORE).as_uri())}
    done = subprocess.run(
        [NODE, "--input-type=module", "-e", driver],
        input=json.dumps(case), capture_output=True, text=True,
        timeout=NODE_CAP_S, check=False)
    if done.returncode != 0:
        raise AssertionError(f"node exited {done.returncode}: {done.stderr}")
    return json.loads(done.stdout)


def apply(memory=None, **extra) -> dict:
    case = {"op": "apply", "patch": PATCH, "module": MODULE, "now": NOW,
            "memory": memory if memory is not None
            else {LOADER: ENTRY, SITE: SITE_BYTES}}
    case.update(extra)
    return run(case)


def kinds(calls: list) -> list[str]:
    return [c[0] for c in calls]


@unittest.skipIf(NODE is None, "node is not on PATH, so launch_core.ts cannot run")
class ApplyPatchTest(unittest.TestCase):
    def test_matching_bytes_write_once_then_hook(self):
        out = apply()
        status = out["status"]
        self.assertEqual(status["state"], "patched")
        self.assertEqual(status["entry_before"], "40 56 57")
        self.assertEqual(status["before"], "BA 01 00 00 00")
        self.assertEqual(status["after"], "BA 00 00 00 00")
        self.assertEqual(status["module"], MODULE)
        self.assertEqual(status["patched_at_ms"], NOW)
        self.assertIsNone(status["reason"])
        self.assertEqual(status["loader_runs"], 0)
        self.assertIsNone(status["bytes_at_first_run"])
        self.assertEqual(out["calls"], [
            ["read", LOADER, 3],
            ["read", SITE, 5],
            ["write", SITE, WRITTEN],
            ["read", SITE, 5],
            ["hook", LOADER],
        ])

    def test_a_differing_entry_writes_nothing_and_hooks_nothing(self):
        out = apply({LOADER: [0x48, 0x89, 0x5C], SITE: SITE_BYTES})
        status = out["status"]
        self.assertEqual(status["state"], "refused")
        self.assertEqual(kinds(out["calls"]), ["read", "read"])
        self.assertEqual(status["entry_before"], "48 89 5C")
        self.assertEqual(status["before"], "BA 01 00 00 00")
        self.assertIsNone(status["after"])
        self.assertIn("48 89 5C", status["reason"])
        self.assertIn("BA 01 00 00 00", status["reason"])
        self.assertIn("nothing was written", status["reason"])

    def test_a_differing_patch_site_writes_nothing_and_hooks_nothing(self):
        out = apply({LOADER: ENTRY, SITE: [0xB9, 0x01, 0x00, 0x00, 0x00]})
        status = out["status"]
        self.assertEqual(status["state"], "refused")
        self.assertEqual(kinds(out["calls"]), ["read", "read"])
        self.assertEqual(status["entry_before"], "40 56 57")
        self.assertEqual(status["before"], "B9 01 00 00 00")
        self.assertIn("40 56 57", status["reason"])
        self.assertIn("B9 01 00 00 00", status["reason"])

    def test_a_short_read_is_a_difference(self):
        out = apply({LOADER: ENTRY[:2], SITE: SITE_BYTES})
        self.assertEqual(out["status"]["state"], "refused")
        self.assertNotIn("write", kinds(out["calls"]))

    def test_a_write_that_does_not_take_is_refused_and_not_hooked(self):
        out = apply(write_sticks=False)
        status = out["status"]
        self.assertEqual(status["state"], "refused")
        self.assertEqual(status["after"], "BA 01 00 00 00")
        self.assertNotIn("hook", kinds(out["calls"]))
        self.assertIsNone(status["patched_at_ms"])

    def test_a_failing_read_is_refused_with_its_error(self):
        out = apply(read_throws=True)
        status = out["status"]
        self.assertEqual(status["state"], "refused")
        self.assertIn("read refused", status["reason"])
        self.assertEqual(kinds(out["calls"]), ["read"])

    def test_no_status_carries_an_error_key(self):
        # frida_attach.LiveAgent raises on a reply with an `error` key, so a
        # status (launch.ts replies with a copy of it) names its reason as
        # `reason` in every state. test_frida_attach feeds these through
        # LiveAgent itself.
        cases = {
            "patched": apply(),
            "refused entry": apply({LOADER: [0x48, 0x89, 0x5C], SITE: SITE_BYTES}),
            "refused write": apply(write_sticks=False),
            "refused read": apply(read_throws=True),
        }
        for what, out in cases.items():
            with self.subTest(case=what):
                self.assertNotIn("error", out["status"])
                self.assertIn("reason", out["status"])
        self.assertNotIn("error", run({"op": "waiting"})["status"])

    def test_the_loader_runs_are_counted_and_the_first_reads_the_site(self):
        out = apply(loader_runs=2)
        status = out["status"]
        self.assertEqual(status["loader_runs"], 2)
        self.assertEqual(status["bytes_at_first_run"], "BA 00 00 00 00")
        # The hook reads the patch site once, at the first run only.
        self.assertEqual(out["calls"][-1], ["read", SITE, 5])
        self.assertEqual(kinds(out["calls"]),
                         ["read", "read", "write", "read", "hook", "read"])


@unittest.skipIf(NODE is None, "node is not on PATH, so launch_core.ts cannot run")
class StatusAndPatchTest(unittest.TestCase):
    def test_the_status_before_the_module_appears_is_waiting(self):
        self.assertEqual(run({"op": "waiting"})["status"], {
            "state": "waiting", "before": None, "after": None,
            "entry_before": None, "module": None, "loader_runs": 0,
            "bytes_at_first_run": None, "patched_at_ms": None, "reason": None})

    def test_a_missing_field_is_named(self):
        for key in ("module", "loader_rva", "entry_expect", "patch_rva",
                    "expect", "write", "defer"):
            with self.subTest(field=key):
                patch = {k: v for k, v in PATCH.items() if k != key}
                out = run({"op": "read_patch", "patch": patch})
                self.assertIn(f'"{key}"', out["error"])

    def test_malformed_values_are_refused(self):
        for key, value in (("loader_rva", "3968D0"), ("patch_rva", "0xZZ"),
                           ("expect", "BA 1 00"), ("write", "BA,00")):
            with self.subTest(field=key):
                out = run({"op": "read_patch", "patch": {**PATCH, key: value}})
                self.assertIn(f'"{key}"', out["error"])

    def test_a_write_of_another_length_is_refused(self):
        out = run({"op": "read_patch",
                   "patch": {**PATCH, "write": "BA 00 00 00"}})
        self.assertIn('"write"', out["error"])
        self.assertIn('"expect"', out["error"])

    def test_no_patch_object_is_refused(self):
        out = run({"op": "read_patch", "patch": None})
        self.assertIn('"patch"', out["error"])

    def test_the_data_file_reads_as_a_patch(self):
        with open(PATCH_FILE, encoding="utf-8") as fh:
            data = json.load(fh)
        out = run({"op": "read_patch", "patch": data})
        self.assertNotIn("error", out)
        self.assertEqual(out["patch"]["module"], data["module"])
        self.assertEqual(data["launch"], "suspended")


if __name__ == "__main__":
    unittest.main()
