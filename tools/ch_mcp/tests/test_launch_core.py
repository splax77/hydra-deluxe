"""Tests for agent/launch_core.ts, the launch agent's decision: both byte
checks pass before anything is written, the write is read back, the loader's
entry is hooked only after the write, and a difference writes nothing and
hooks nothing.

No game and no Frida. Plain Node runs launch_core.ts (Node erases its type
annotations). Each case goes to a small driver on stdin, and the driver prints
what applyPatch returned and every call it made on the memory. The patch is
the real launch_patch.json, and the memory holds the bytes one real launch
read from the game, with the replies that launch sent as the expected
statuses. The tests skip, naming the reason, when `node` is not on PATH.
"""

from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run as B  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp.tests.test_bot_run import recorded_launch_replies  # noqa: E402

CORE = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", "launch_core.ts")
PATCH_FILE = B.LAUNCH_PATCH_FILE
NODE = shutil.which("node")
# One driver run is a fresh Node start; this is its stop.
NODE_CAP_S = 30

# The driver: reads one case from stdin, runs it against launch_core.ts and
# prints the result as JSON. Its memory maps an RVA's text to the bytes found
# there, and every case fills it with bytes a real run read from the game; a
# write replaces them.
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
            return (memory.get(rva) ?? []).slice(0, n);
        },
        writeBytes(rva, bytes) {
            calls.push(["write", rva, bytes]);
            memory.set(rva, bytes);
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

# The real launch_patch.json, read by bot_run's own reader: the patch the
# launch agent gets in its arm message.
PATCH = B._read_patch(PATCH_FILE)
LOADER = PATCH["loader_rva"]
SITE = PATCH["patch_rva"]

# The replies of one real human launch: testdata/ch_runs/ls-d0-run1, copied
# from hydra-data\ch-mcp-runs\2026-10-31\ls-d0-run1 (test_bot_run reads it).
REPLIES = recorded_launch_replies()
RECORDED_WAITING = next(r for r in REPLIES
                        if r["state"] == "waiting" and "modules" not in r)
RECORDED_PATCHED = next(r for r in REPLIES
                        if r["state"] == "patched" and r["loader_runs"] == 0)
RECORDED_AFTER_LOADER = next(r for r in REPLIES
                             if r["state"] == "patched" and r["loader_runs"] == 1)


def hex_bytes(text: str) -> list[int]:
    """A byte string as the patch and the replies write it, read by
    bot_run's own reader."""
    return list(B._byte_values(text))


def recorded_memory(site: str | None = None) -> dict:
    """The game's memory as the real launch read it: the loader's entry and
    the patch site as the patched reply names them. `site` replaces the patch
    site's bytes with others the same run recorded."""
    return {LOADER: hex_bytes(RECORDED_PATCHED["entry_before"]),
            SITE: hex_bytes(site or RECORDED_PATCHED["before"])}


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
    """applyPatch on the real patch, with the module and the clock the real
    launch's patched reply names."""
    case = {"op": "apply", "patch": PATCH, "module": RECORDED_PATCHED["module"],
            "now": RECORDED_PATCHED["patched_at_ms"],
            "memory": memory if memory is not None else recorded_memory()}
    case.update(extra)
    return run(case)


def kinds(calls: list) -> list[str]:
    return [c[0] for c in calls]


@unittest.skipIf(NODE is None, "node is not on PATH, so launch_core.ts cannot run")
class ApplyPatchTest(unittest.TestCase):
    def test_matching_bytes_write_once_then_hook(self):
        out = apply()
        # The status is the reply the real launch sent once it had patched.
        self.assertEqual(out["status"], RECORDED_PATCHED)
        site_len = len(hex_bytes(PATCH["expect"]))
        self.assertEqual(out["calls"], [
            ["read", LOADER, len(hex_bytes(PATCH["entry_expect"]))],
            ["read", SITE, site_len],
            ["write", SITE, hex_bytes(PATCH["write"])],
            ["read", SITE, site_len],
            ["hook", LOADER],
        ])

    def test_a_patch_site_already_written_writes_nothing_and_hooks_nothing(self):
        # The patch site as the same run's loader found it after the write: a
        # second arm on that game reads these bytes, not the expected ones.
        written = RECORDED_AFTER_LOADER["bytes_at_first_run"]
        out = apply(recorded_memory(site=written))
        status = out["status"]
        self.assertEqual(status["state"], "refused")
        self.assertEqual(kinds(out["calls"]), ["read", "read"])
        self.assertEqual(status["entry_before"], RECORDED_PATCHED["entry_before"])
        self.assertEqual(status["before"], written)
        self.assertIsNone(status["after"])
        self.assertIsNone(status["patched_at_ms"])
        for named in (SITE, written, PATCH["expect"]):
            self.assertIn(named, status["reason"])

    def test_no_status_carries_an_error_key(self):
        # frida_attach ends a run on a reply that its ends_run says ends it,
        # so no status (launch.ts replies with a copy of it) may be one, in
        # any state. Each also has the keys of the real launch's replies.
        cases = {
            "patched": apply()["status"],
            "refused site": apply(recorded_memory(
                site=RECORDED_AFTER_LOADER["bytes_at_first_run"]))["status"],
            "waiting": run({"op": "waiting"})["status"],
        }
        for what, status in cases.items():
            with self.subTest(case=what):
                self.assertIsNone(FA.ends_run(status))
                self.assertEqual(set(status), set(RECORDED_PATCHED))

    def test_the_loader_run_is_counted_and_reads_the_site(self):
        out = apply(loader_runs=1)
        # The status is the reply the real launch sent after the loader ran.
        self.assertEqual(out["status"], RECORDED_AFTER_LOADER)
        # The hook reads the patch site at the loader's run.
        self.assertEqual(out["calls"][-1],
                         ["read", SITE, len(hex_bytes(PATCH["expect"]))])
        self.assertEqual(kinds(out["calls"]),
                         ["read", "read", "write", "read", "hook", "read"])


@unittest.skipIf(NODE is None, "node is not on PATH, so launch_core.ts cannot run")
class StatusAndPatchTest(unittest.TestCase):
    def test_the_status_before_the_module_appears_is_waiting(self):
        # The waiting status the real launch replied with before the module
        # appeared.
        self.assertEqual(run({"op": "waiting"})["status"], RECORDED_WAITING)

    def test_a_missing_field_is_named(self):
        # The fields are the ones readPatch itself returns from the real file.
        fields = list(run({"op": "read_patch", "patch": PATCH})["patch"])
        self.assertTrue(fields)
        for key in fields:
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
        self.assertIn(data["launch"], B.LAUNCH_FORMS)


if __name__ == "__main__":
    unittest.main()
