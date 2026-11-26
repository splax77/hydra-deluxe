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
        // The speed's memory: `slots` maps an RVA to the pointer stored
        // there, `words` maps "pointer+offset" to what is stored there, and
        // the setter stores into `set_result ?? value` at the object's value.
        readSlot(rva) {
            calls.push(["slot", rva]);
            return c.slots[rva] ?? "0";
        },
        readPointer(at, offset) {
            calls.push(["pointer", at, offset]);
            return c.words[`${at}+${offset}`] ?? "0";
        },
        readInt(at, offset) {
            calls.push(["int", at, offset]);
            return c.words[`${at}+${offset}`];
        },
        call(fn, args) {
            calls.push(["call", fn.name, fn.rva, args]);
            c.words[`${args[0]}+${c.value_at}`] = c.set_result ?? args[1];
        },
    };
    const speed = core.readSpeed(c.patch, c.song_speed);
    const status = core.applyPatch(core.readPatch(c.patch), io, c.module,
                                   () => c.now, speed);
    for (let i = 0; i < (c.loader_runs ?? 0); i++) { hook(); }
    out.status = status;
    out.calls = calls;
} else if (c.op === "read_speed") {
    try { out.speed = core.readSpeed(c.patch, c.song_speed); }
    catch (e) { out.error = e.message; }
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


SPEED = PATCH["song_speed"]
SETTER = next(c for c in PATCH["calls"] if c["name"] == "int_setting_set")
# Made-up pointers for the class, its statics and the song_speed object.
KLASS, STATICS, OBJECT = "0x1000", "0x2000", "0x3000"
# The speed the plan arms, inside the patch's own limits.
ARMED = SPEED["max"]


def speed_memory(min_=None, max_=None, before=None, null=None) -> dict:
    """The speed's part of an apply case: the setting found by the patch's
    own RVA and offsets, holding the patch's own limits unless told
    otherwise. `null` names the one pointer that reads as null: "slot",
    "statics" or "object"."""
    slots = {SPEED["typeinfo_rva"]: "0" if null == "slot" else KLASS}
    words = {
        f"{KLASS}+{SPEED['statics_offset']}": "0" if null == "statics" else STATICS,
        f"{STATICS}+{SPEED['object_offset']}": "0" if null == "object" else OBJECT,
        f"{OBJECT}+{SPEED['min_offset']}": SPEED["min"] if min_ is None else min_,
        f"{OBJECT}+{SPEED['max_offset']}": SPEED["max"] if max_ is None else max_,
        f"{OBJECT}+{SPEED['value_offset']}": SPEED["min"] if before is None else before,
    }
    return {"slots": slots, "words": words, "value_at": SPEED["value_offset"]}


def speed_calls(calls: list) -> list:
    """The calls after the patch site's read at the loader's first run."""
    first_run = [i for i, c in enumerate(calls) if c[0] == "hook"][0] + 2
    return calls[first_run:]


@unittest.skipIf(NODE is None, "node is not on PATH, so launch_core.ts cannot run")
class SpeedTest(unittest.TestCase):
    def test_the_speed_is_found_checked_set_once_and_read_back(self):
        out = apply(song_speed=ARMED, loader_runs=1, **speed_memory())
        self.assertEqual(speed_calls(out["calls"]), [
            ["slot", SPEED["typeinfo_rva"]],
            ["pointer", KLASS, SPEED["statics_offset"]],
            ["pointer", STATICS, SPEED["object_offset"]],
            ["int", OBJECT, SPEED["min_offset"]],
            ["int", OBJECT, SPEED["max_offset"]],
            ["int", OBJECT, SPEED["value_offset"]],
            ["call", SETTER["name"], SETTER["rva"], [OBJECT, ARMED, "0"]],
            ["int", OBJECT, SPEED["value_offset"]],
        ])
        status = out["status"]
        self.assertEqual(status["state"], "patched")
        self.assertEqual(status["speed"], {
            "value": ARMED, "state": "set", "object": OBJECT,
            "min": SPEED["min"], "max": SPEED["max"],
            "before": SPEED["min"], "after": ARMED})

    def test_no_speed_armed_reads_and_calls_nothing(self):
        out = apply(loader_runs=1, **speed_memory())
        self.assertEqual(kinds(out["calls"]),
                         ["read", "read", "write", "read", "hook", "read"])
        self.assertNotIn("speed", out["status"])

    def test_before_the_loader_runs_nothing_is_called(self):
        out = apply(song_speed=ARMED, **speed_memory())
        self.assertEqual(kinds(out["calls"]), ["read", "read", "write", "read", "hook"])
        self.assertEqual(out["status"]["speed"]["state"], "armed")

    def test_each_failure_point_refuses_with_no_call(self):
        cases = {
            "a null slot": (speed_memory(null="slot"), SPEED["typeinfo_rva"]),
            "null statics": (speed_memory(null="statics"), SPEED["statics_offset"]),
            "a null object": (speed_memory(null="object"), SPEED["object_offset"]),
            "another minimum": (speed_memory(min_=SPEED["min"] + 1), str(SPEED["min"] + 1)),
            "another maximum": (speed_memory(max_=SPEED["max"] - 1), str(SPEED["max"] - 1)),
        }
        for what, (memory, named) in cases.items():
            with self.subTest(case=what):
                out = apply(song_speed=ARMED, loader_runs=1, **memory)
                status = out["status"]
                self.assertEqual(status["state"], "refused")
                self.assertEqual(status["speed"]["state"], "refused")
                self.assertNotIn("call", kinds(out["calls"]))
                self.assertIn(named, status["reason"])

    def test_a_read_back_that_differs_refuses(self):
        out = apply(song_speed=ARMED, loader_runs=1, set_result=SPEED["min"],
                    **speed_memory())
        status = out["status"]
        self.assertEqual(kinds(out["calls"]).count("call"), 1)
        self.assertEqual(status["state"], "refused")
        self.assertEqual(status["speed"]["after"], SPEED["min"])
        self.assertIn(str(ARMED), status["reason"])

    def test_the_setter_is_called_on_the_first_run_only(self):
        out = apply(song_speed=ARMED, loader_runs=2, **speed_memory())
        self.assertEqual(kinds(out["calls"]).count("call"), 1)
        self.assertEqual(out["status"]["loader_runs"], 2)
        self.assertEqual(out["status"]["speed"]["state"], "set")

    def test_a_speed_outside_the_patch_limits_is_refused_at_the_arm(self):
        for value in (SPEED["min"] - 1, SPEED["max"] + 1, 1.5, "5000", True):
            with self.subTest(value=value):
                out = run({"op": "read_speed", "patch": PATCH, "song_speed": value})
                self.assertIn("song speed", out["error"])
        self.assertIsNone(run({"op": "read_speed", "patch": PATCH,
                               "song_speed": None})["speed"])

    def test_a_missing_speed_field_is_named(self):
        for key in ("typeinfo_rva", "statics_offset", "object_offset", "value_offset",
                    "min_offset", "max_offset", "min", "max"):
            with self.subTest(field=key):
                block = {k: v for k, v in SPEED.items() if k != key}
                out = run({"op": "read_speed", "patch": {**PATCH, "song_speed": block},
                           "song_speed": ARMED})
                self.assertIn(f'"{key}"', out["error"])
        out = run({"op": "read_speed", "patch": {**PATCH, "calls": []}, "song_speed": ARMED})
        self.assertIn(SETTER["name"], out["error"])


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
