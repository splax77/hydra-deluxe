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

import fake_hydra_live as fake  # noqa: E402
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
        self.assertEqual(s.timeout, fake.HELLO_TIMEOUT_S)
        self.assertIn(f"wait cap: {fake.HELLO_TIMEOUT_S} s", out)

    def test_a_hello_line_without_a_timeout_ends_the_session(self):
        self.helper._read_line = lambda sess, wait_s: '{"ok":true,"hello":{"mode":"headless","pid":1}}'
        out = self.helper.start("headless")
        self.assertIn("was not a hello line with a timeout", out)
        self.assertIsNone(self.helper.session)

    def test_the_helper_follows_its_own_timeouts_and_a_reset_keeps_them(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("timeout 5"), "ok")
        self.assertEqual(self.helper.session.timeout, 5)
        self.assertEqual(self.helper.send("reset"), "ok")
        self.assertEqual(self.helper.session.timeout, 5)

    def test_screenshot_is_saved_under_the_name_given(self):
        self.helper.start("headless")
        scratch = self.helper.session.scratch
        for name in ("shot.png", "bare"):
            self.assertEqual(self.helper.send(f"screenshot {name}"), f"screenshot: {scratch / name}")
            self.assertTrue((scratch / name).is_file())
        self.assertFalse((scratch / "bare.png").exists())

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
        self.assertTrue(out.startswith("error: !! failed at line 1: click missing button"), out)
        self.assertIn("Unable to locate item", out)
        out = self.helper.send("wait-text never here")
        self.assertTrue(out.startswith("error: failed: wait-text never here\n"), out)
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
        self.assertEqual(self.helper.session.timeout, fake.HELLO_TIMEOUT_S)

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

    def test_run_sends_every_line_and_stops_at_the_first_failure(self):
        out = self.helper.run("click Scan library\n# a comment\n\nclick missing one\nstate\n")
        self.assertIn("> click Scan library\nok\n> # a comment\nok\n> \nok\n", out)
        self.assertIn("> click missing one\nerror: !! failed at line 4: click missing one", out)
        self.assertNotIn("> state", out)
        self.assertTrue(out.endswith("stopped at line 4 of 5"), out)

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
        self.assertEqual(self.helper.session.timeout, fake.HELLO_TIMEOUT_S)

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

    def test_a_junction_to_the_folder_counts_as_the_folder(self):
        lib = self.root / "lib"
        make_library(lib)
        link = self.root / "lib junction"
        r = subprocess.run(["cmd", "/c", "mklink", "/J", str(link), str(lib)],
                           capture_output=True, text=True)
        if r.returncode != 0:
            self.skipTest(f"Windows would not make a junction here: {r.stdout}{r.stderr}")
        try:
            # Hydra runs from the real folder; the user named the junction.
            procs = [("Hydra.exe", str(lib / "Hydra.exe"))]
            self.assertEqual(hm.hydra_running_from(link, procs), [str(lib / "Hydra.exe")])
            # And the other way round.
            procs = [("Hydra.exe", str(link / "Hydra.exe"))]
            self.assertEqual(len(hm.hydra_running_from(lib, procs)), 1)
        finally:
            os.rmdir(link)  # removes the junction only, never what it points at

    def test_a_short_name_for_the_folder_counts_as_the_folder(self):
        lib = self.root / "Library Folder With A Long Name"
        make_library(lib)
        import ctypes
        buf = ctypes.create_unicode_buffer(32768)
        n = ctypes.windll.kernel32.GetShortPathNameW(str(lib), buf, len(buf))
        short = buf.value if n else ""
        if not short or os.path.basename(short) == lib.name:
            self.skipTest("this volume gives no 8.3 short name for a folder in temp")
        procs = [("Hydra.exe", str(lib / "Hydra.exe"))]
        self.assertEqual(len(hm.hydra_running_from(short, procs)), 1)
        procs = [("Hydra.exe", os.path.join(short, "Hydra.exe"))]
        self.assertEqual(len(hm.hydra_running_from(lib, procs)), 1)

    def test_a_crash_report_never_quotes_an_old_runs_log(self):
        lib = self.root / "lib"
        make_library(lib)
        stale = self.exe_dir / "hydra_uitest.log"
        stale.write_text("old run: this line is stale\n")
        self.helper.start("window", library_dir=str(lib))
        out = self.helper.send("fake-exit 3")
        self.assertTrue(out.startswith("Hydra exited with code 3."), out)
        self.assertIn(f"no log written: {stale} is unchanged since launch", out)
        self.assertNotIn("this line is stale", out)

    def test_a_crash_report_quotes_a_log_written_after_launch(self):
        lib = self.root / "lib"
        make_library(lib)
        log = self.exe_dir / "hydra_uitest.log"
        log.write_text("old run: this line is stale\n")
        self.helper.start("window", library_dir=str(lib))
        # What Hydra.exe does when it opens its log: start it afresh.
        log.write_text("this run: fresh line\n")
        out = self.helper.send("fake-exit 3")
        self.assertIn(f"last lines of {log}:\n  this run: fresh line", out)
        self.assertNotIn("this line is stale", out)

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

    def test_the_real_hello_line(self):
        self.helper.start("headless")
        s = self.helper.session
        # Pinned from the real exe's first line on 2026-10-14.
        self.assertEqual(sorted(s.hello), ["mode", "pid", "scratch", "timeout"])
        self.assertEqual(s.hello["mode"], "headless")
        self.assertEqual(s.hello["pid"], s.proc.pid)
        self.assertEqual(s.timeout, 30)
        self.assertTrue(Path(s.hello["scratch"]).is_dir())

    def test_real_replies_for_a_failure_a_skipped_line_and_a_screenshot(self):
        self.helper.start("headless")
        out = self.helper.run("state\n\n# a comment\nclick No such button\nstate")
        self.assertIn("> \nok\n> # a comment\nok\n", out)
        self.assertIn("> click No such button\nerror: !! failed at line 4: click No such button\n", out)
        self.assertIn("Unable to locate item", out)
        self.assertTrue(out.endswith("stopped at line 4 of 5"), out)
        # The interpreter's own error, from a wait that gives up.
        out = self.helper.with_timeout("wait-text never on screen", 0)
        self.assertTrue(out.startswith("error: failed: wait-text never on screen\n"), out)
        self.assertIn('did not appear within 0s', out)
        scratch = self.helper.session.scratch
        for name in ("shot.png", "bare"):
            self.assertEqual(self.helper.send(f"screenshot {name}"), f"screenshot: {scratch / name}")
            self.assertEqual((scratch / name).read_bytes()[:8], b"\x89PNG\r\n\x1a\n")
        self.assertFalse((scratch / "bare.png").exists())

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
