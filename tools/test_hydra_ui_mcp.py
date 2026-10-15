"""Tests for tools/hydra_ui_mcp.py, the MCP server that drives Hydra's GUI.

Two layers. The first needs no build: tools/fake_hydra_live.py stands in for
the exes, making the same pipe and speaking the same JSON lines, and can be
told to hang, exit or answer with 50,000 characters. The second drives the
real build-cpp\\Release exes and skips itself until hydra_uitest.exe lists
--live in its usage line (the live channel, task T2 of
docs/superpowers/plans/2026-10-14-hydra-mcp-server.md).

No test reads or writes the user's library folder; the snapshot tests use a
fake one in a temporary folder.

Run:  py -I tools/test_hydra_ui_mcp.py
"""

import asyncio
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import hydra_ui_mcp as hm  # noqa: E402

FAKE = HERE / "fake_hydra_live.py"
HELPER_SCRIPT = HERE / "hydra_ui_mcp.py"

TOOL_NAMES = {
    "hydra_start", "hydra_stop", "hydra_reset", "hydra_click", "hydra_type",
    "hydra_check", "hydra_key", "hydra_window", "hydra_wait_idle", "hydra_wait_text",
    "hydra_text", "hydra_state", "hydra_dump", "hydra_rect", "hydra_screenshot", "hydra_run",
}


def fake_launcher(exe, args):
    # sys.executable directly, not the py launcher, so the launched process's
    # pid is the stand-in's own and the pipe name matches.
    return [sys.executable, "-I", str(FAKE), "--as", exe.name] + list(args)


def make_library(folder, with_wal=True):
    """A fake library folder with an old, fixed modification time on every file."""
    folder.mkdir(parents=True)
    names = list(hm.SNAPSHOT_FILES) if with_wal else [
        n for n in hm.SNAPSHOT_FILES if not n.startswith("hydra.db-")]
    for i, name in enumerate(names):
        p = folder / name
        p.write_bytes(f"{name} contents {i}".encode() * 10)
        os.utime(p, ns=(1_600_000_000_000_000_000, 1_600_000_000_000_000_000 + i))
    return names


class FakeLayer(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="hydra_mcp_test_")
        self.root = Path(self.tmp.name)
        self.exe_dir = self.root / "Release"
        self.exe_dir.mkdir()
        for exe in (hm.HEADLESS_EXE, hm.WINDOW_EXE):
            (self.exe_dir / exe).write_bytes(b"")
        self.processes = []
        self.helper = self.new_helper()

    def new_helper(self, **kw):
        args = dict(exe_dir=self.exe_dir, launcher=fake_launcher, temp_root=self.root,
                    process_lister=lambda: self.processes)
        args.update(kw)
        return hm.Helper(**args)

    def tearDown(self):
        self.helper.shutdown()
        self.tmp.cleanup()

    # -- the session --------------------------------------------------------

    def test_hello_line_is_parsed(self):
        out = self.helper.start("headless")
        s = self.helper.session
        self.assertIn("started a headless session", out)
        self.assertIn(f"pid: {s.proc.pid}", out)
        self.assertEqual(s.hello["pid"], s.proc.pid)
        self.assertIn("engine scratch: ", out)

    def test_one_tool_call_sends_one_line_and_returns_the_reply(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("click Scan library"), "ok")
        self.assertIn("charts=97", self.helper.send("state"))
        self.assertIn("> click Scan library", self.helper.session.log_path.read_text())

    def test_first_tool_call_starts_headless(self):
        out = self.helper.send("state")
        self.assertIn("started a headless session", out)
        self.assertIn("store=ready", out)

    def test_failure_carries_the_error_and_log_and_the_session_survives(self):
        self.helper.start("headless")
        out = self.helper.send("click missing button")
        self.assertTrue(out.startswith("failed: click failed"), out)
        self.assertIn("Unable to locate item", out)
        self.assertEqual(self.helper.send("click Scan library"), "ok")

    def test_line_break_is_refused_before_sending(self):
        self.helper.start("headless")
        out = self.helper.send("type Search | a\nquit")
        self.assertTrue(out.startswith("refused: one command is one line"), out)
        self.assertIsNotNone(self.helper.session)
        self.assertEqual(self.helper.send("state").count("store=ready"), 1)

    def test_hanging_hydra_is_reported_and_killed(self):
        helper = self.new_helper(reply_margin_s=1)
        try:
            helper.start("headless")
            proc = helper.session.proc
            self.assertEqual(helper.send("timeout 0"), "ok")
            out = helper.send("fake-hang")
            self.assertTrue(out.startswith("Hydra did not answer within 1 s"), out)
            self.assertIsNotNone(proc.poll())
            self.assertIsNone(helper.session)
            self.assertEqual(helper.send("state"), hm.NO_SESSION)
        finally:
            helper.shutdown()

    def test_a_closed_channel_with_a_live_process_is_reported_as_killed(self):
        helper = self.new_helper(reply_margin_s=1)
        try:
            helper.start("headless")
            proc = helper.session.proc
            out = helper.send("fake-close")
            self.assertTrue(out.startswith("Hydra closed its live channel but kept running for 1 s"), out)
            self.assertNotIn("Hydra exited with code", out)
            self.assertIsNotNone(proc.poll())
            self.assertIsNone(helper.session)
        finally:
            helper.shutdown()

    def test_bad_timeouts_are_refused_before_anything_is_sent(self):
        self.helper.start("headless")
        for bad in (-1, float("inf"), float("nan")):
            self.assertTrue(self.helper.with_timeout("wait-idle", bad).startswith("refused: timeout"), bad)
        self.assertEqual(self.helper.session.timeout, hm.ENGINE_DEFAULT_TIMEOUT_S)

    def test_a_reply_that_is_not_an_object_is_reported(self):
        self.helper.start("headless")
        self.helper._read_line = lambda sess, wait_s: "5"
        ok, text, _ = self.helper.command("state")
        self.assertFalse(ok)
        self.assertIn("not a JSON reply", text)

    def test_an_unreadable_process_list_refuses_the_snapshot_and_leaves_no_folder(self):
        lib = self.root / "lib"
        make_library(lib)
        helper = self.new_helper(process_lister=lambda: (_ for _ in ()).throw(OSError("no list")))
        out = helper.start("window", library_dir=str(lib))
        self.assertTrue(out.startswith("refused: no list"), out)
        self.assertEqual(list(self.root.glob("hydra_live_*")), [])

    def test_exiting_hydra_is_reported_with_code_log_tail_and_dump(self):
        self.helper.start("headless")
        out = self.helper.send("fake-exit 3 dump")
        self.assertTrue(out.startswith("Hydra exited with code 3."), out)
        self.assertIn("fake: exiting with code 3", out)
        self.assertIn("crash.dmp", out)
        self.assertEqual(self.helper.send("click x"), hm.NO_SESSION)

    def test_long_reply_is_cut_at_the_cap(self):
        self.helper.start("headless")
        out = self.helper.send("fake-big")
        self.assertLessEqual(len(out), hm.REPLY_CAP_CHARS)
        last = out.splitlines()[-1]
        kept = len(out.splitlines()) - 1
        self.assertEqual(
            last, f"[cut at 12,000 characters: {1000 - kept} of 1000 lines dropped;"
                  " contains= narrows hydra_text and hydra_dump]")

    def test_one_long_line_is_cut_too(self):
        out = hm.cap_reply("y" * 50_000, cap=1000)
        self.assertLessEqual(len(out), 1000)
        self.assertTrue(out.endswith("1 of 1 lines dropped; contains= narrows hydra_text and hydra_dump]"))

    def test_run_stops_at_the_first_failure(self):
        out = self.helper.run("click Scan library\n# a comment\nclick missing one\nstate\n")
        self.assertIn("> click Scan library\nok", out)
        self.assertIn("> click missing one\nfailed: click failed", out)
        self.assertNotIn("> state", out)
        self.assertTrue(out.endswith("stopped at line 2 of 3"), out)

    def test_text_and_dump_narrow_with_contains(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("text", lambda t: hm.filter_text(t, "Bass")), "2x Bass")
        out = self.helper.send("dump Hydra", lambda t: hm.filter_dump(t, "2x Bass"))
        self.assertEqual(out.splitlines(), [
            'window "Hydra" id=0x1 pos=(0,0) size=(1280,800)',
            '  "2x Bass" id=0x3 rect=(140,40 80x22) checkable',
            'window "Settings" id=0x5 pos=(100,100) size=(400,300)',
            '  "2x Bass##s" id=0x6 rect=(110,140 80x22) checkable',
        ])

    def test_one_off_timeout_is_put_back(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.with_timeout("wait-idle", 5), "ok")
        self.assertEqual(self.helper.session.timeout, hm.ENGINE_DEFAULT_TIMEOUT_S)

    def test_stop_exits_zero_and_deletes_the_scratch(self):
        self.helper.start("headless")
        scratch = self.helper.session.scratch
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")
        self.assertFalse(scratch.exists())
        self.assertEqual(self.helper.send("state"), hm.NO_SESSION)

    def test_keep_keeps_the_scratch(self):
        self.helper.start("headless", keep=True)
        scratch = self.helper.session.scratch
        self.assertEqual(self.helper.stop(), f"Hydra exited with code 0; kept {scratch}")
        self.assertTrue(scratch.is_dir())

    def test_start_again_stops_the_old_session(self):
        self.helper.start("headless")
        old = self.helper.session.proc
        out = self.helper.start("headless")
        self.assertIn("stopped the running session first: Hydra exited with code 0", out)
        self.assertEqual(old.poll(), 0)

    def test_missing_exe_names_the_build_command(self):
        (self.exe_dir / hm.HEADLESS_EXE).unlink()
        out = self.helper.start("headless")
        self.assertIn(r".\build_cpp.ps1 -Target hydra_uitest", out)
        self.assertIsNone(self.helper.session)

    # -- the snapshot and window mode ---------------------------------------

    def test_snapshot_copies_the_five_files_and_leaves_the_originals_alone(self):
        lib = self.root / "Fake Library"
        make_library(lib)
        before = {n: ((lib / n).read_bytes(), (lib / n).stat().st_mtime_ns) for n in hm.SNAPSHOT_FILES}
        dest = self.root / "snap"
        copied, report = hm.snapshot_library(str(lib), dest, [])
        self.assertEqual(sorted(copied), sorted(hm.SNAPSHOT_FILES))
        for n in hm.SNAPSHOT_FILES:
            self.assertEqual((dest / n).read_bytes(), before[n][0])
            self.assertEqual(((lib / n).read_bytes(), (lib / n).stat().st_mtime_ns), before[n])
            self.assertIn(f"  {n}: ", report)

    def test_snapshot_without_wal_says_what_was_absent(self):
        lib = self.root / "lib"
        make_library(lib, with_wal=False)
        copied, report = hm.snapshot_library(str(lib), self.root / "snap", [])
        self.assertNotIn("hydra.db-wal", copied)
        self.assertIn("  hydra.db-wal: not present", report)

    def test_running_hydra_from_the_folder_refuses_the_snapshot(self):
        lib = self.root / "lib"
        make_library(lib)
        self.processes = [("explorer.exe", None), ("Hydra.exe", str(lib / "Hydra.exe"))]
        out = self.helper.start("window", library_dir=str(lib))
        self.assertTrue(out.startswith("refused: " + hm.CLOSE_HYDRA_FIRST), out)
        self.assertIsNone(self.helper.session)
        self.assertEqual(list(self.root.glob("hydra_live_*")), [])

    def test_hydra_from_another_folder_does_not_count(self):
        lib = self.root / "lib"
        procs = [("HYDRA.EXE", str(self.root / "other" / "Hydra.exe")), ("Hydra.exe.bak", None)]
        self.assertEqual(hm.hydra_running_from(lib, procs), [])
        self.assertEqual(len(hm.hydra_running_from(lib, [("hydra.exe", None)])), 1)

    def test_window_command_line_never_names_the_library_folder(self):
        lib = self.root / "Library Folder Q7"
        make_library(lib)
        out = self.helper.start("window", library_dir=str(lib))
        self.assertIn("started a window session", out)
        self.assertIn("hydra.db: ", out)
        argv = self.helper.session.argv
        self.assertIn("--db", argv)
        self.assertIn("--ini", argv)
        self.assertIn("--ui-ini", argv)
        for arg in argv:
            self.assertNotIn(os.path.normcase(str(lib)), os.path.normcase(arg))
            self.assertNotIn("Library Folder Q7", arg)
        db = Path(argv[argv.index("--db") + 1])
        self.assertTrue(db.is_relative_to(self.helper.session.scratch))
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")

    def test_keep_window_leaves_the_process_running(self):
        lib = self.root / "lib"
        make_library(lib)
        self.helper.start("window", library_dir=str(lib))
        proc = self.helper.session.proc
        out = self.helper.stop(keep_window=True)
        self.assertTrue(out.startswith("left the window open"), out)
        self.assertIsNone(proc.poll())
        self.helper.detached[0].pipe.close()
        self.assertEqual(proc.wait(timeout=hm.REPLY_MARGIN_S), 0)


class SdkClient(unittest.TestCase):
    def test_the_sdk_client_lists_every_tool(self):
        from mcp import Client, StdioServerParameters

        async def names():
            params = StdioServerParameters(command=sys.executable, args=["-I", str(HELPER_SCRIPT)])
            async with Client(params) as client:
                result = await client.list_tools()
                return {t.name for t in result.tools}

        self.assertEqual(asyncio.run(names()), TOOL_NAMES)


def why_no_live():
    """None when the real layer can run, else why it skips. The marker is
    --live in the usage line hydra_uitest prints for --help."""
    exe = hm.EXE_DIR / hm.HEADLESS_EXE
    if not exe.is_file():
        return f"{exe} is not built"
    r = subprocess.run([str(exe), "--help"], capture_output=True, text=True,
                       timeout=hm.HELLO_TIMEOUT_S)
    if "--live" not in (r.stdout + r.stderr):
        return "hydra_uitest.exe has no --live yet (task T2)"
    return None


SMOKE = hm.REPO / "tests" / "ui" / "scripts" / "smoke.txt"
NO_LIVE = why_no_live()


@unittest.skipIf(NO_LIVE is not None, NO_LIVE or "")
class RealExeLayer(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="hydra_mcp_real_")
        self.helper = hm.Helper(temp_root=self.tmp.name, process_lister=lambda: [])

    def tearDown(self):
        self.helper.shutdown()
        self.tmp.cleanup()

    def scan(self):
        # The smoke script's lines up to its reset: the script goes on to reset
        # and quit, which would leave no library and no session to check.
        lines = SMOKE.read_text(encoding="utf-8").splitlines()
        before_reset = lines[:[ln.strip() for ln in lines].index("reset")]
        out = self.helper.run("\n".join(before_reset))
        self.assertNotIn("stopped at line", out)
        self.assertIn("charts=97", self.helper.send("state"))

    def test_headless_session(self):
        self.helper.start("headless")
        self.assertIn("store=ready", self.helper.send("state"))
        self.scan()
        self.assertEqual(self.helper.send("reset"), "ok")
        self.assertIn("charts=0", self.helper.send("state"))
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")

    @unittest.skipUnless(os.environ.get("HYDRA_LIVE_WINDOW_TEST") == "1",
                         "opens a window; set HYDRA_LIVE_WINDOW_TEST=1")
    def test_window_session_on_a_seed_from_headless(self):
        self.helper.start("headless", keep=True)
        self.scan()
        engine = self.helper.session.engine_scratch()
        lib = Path(self.tmp.name) / "seed library"
        lib.mkdir()
        self.helper.stop()
        for name in hm.SNAPSHOT_FILES:
            if (engine / name).is_file():
                shutil.copy2(engine / name, lib / name)
        out = self.helper.start("window", library_dir=str(lib))
        self.assertIn("started a window session", out)
        self.assertIn("charts=97", self.helper.send("state"))
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")


if __name__ == "__main__":
    unittest.main(verbosity=2)
