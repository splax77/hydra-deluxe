"""Tests for tools/hydra_ui_mcp.py, the MCP server that drives Hydra's GUI.

Two layers. Every test of what a command does runs against the real
build-cpp\\Release\\hydra_uitest.exe --live (RealExeLayer), because Hydra owns
every command rule and the helper only carries messages. The stand-in,
tools/fake_hydra_live.py, is kept for what the real exe cannot be made to do
(hang, exit, close its pipe, answer with 50,000 characters) and for the
window-mode command line and snapshot, which must not open a real window
(TransportLayer). The real layer skips itself until hydra_uitest.exe lists
--live in its usage line.

No test reads or writes the user's library folder; the snapshot tests use a
fake one in a temporary folder.

Run:  py -I tools/test_hydra_ui_mcp.py
"""

import asyncio
import ctypes
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import unittest
from ctypes import wintypes
from datetime import datetime
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import fake_hydra_live as fake  # noqa: E402
import hydra_ui_mcp as hm  # noqa: E402

FAKE = HERE / "fake_hydra_live.py"
WM_CLOSE = 0x0010  # winuser.h
WM_KEYDOWN = 0x0100  # winuser.h
WM_KEYUP = 0x0101  # winuser.h
VK_ESCAPE = 0x001B  # winuser.h: the Win32 key code of Escape, not a Hydra number
# Long enough that the test engine reads the held Esc as an abort request;
# the engine's own hold time is in imgui_te_engine.cpp (the "holding ESC" block).
ESC_HOLD_S = 1.0
HELPER_SCRIPT = HERE / "hydra_ui_mcp.py"

TOOL_NAMES = {
    "hydra_start", "hydra_stop", "hydra_reset", "hydra_open_song", "hydra_click", "hydra_type",
    "hydra_check", "hydra_key", "hydra_window", "hydra_wait_idle", "hydra_wait_text",
    "hydra_text", "hydra_state", "hydra_dump", "hydra_rect", "hydra_screenshot", "hydra_run",
}


def fake_launcher(exe, args):
    # sys.executable directly, not the py launcher, so the launched process's
    # pid is the stand-in's own.
    return [sys.executable, "-I", str(FAKE), "--as", exe.name] + list(args)


_real_realapp_record = hm.realapp_record


def _refuse_the_real_repo(session, repo):
    # A Helper built without repo= would name this checkout and so write the
    # real repo's record file; that fails the test instead.
    assert Path(repo).resolve() != hm.REPO, "a test let a record reach the real repo's file"
    return _real_realapp_record(session, repo)


def setUpModule():
    hm.realapp_record = _refuse_the_real_repo


def tearDownModule():
    hm.realapp_record = _real_realapp_record


def git(repo, *args):
    return subprocess.run(["git", "-C", str(repo), *args], capture_output=True, text=True,
                          check=True).stdout.strip()


def make_repo(folder):
    """A fresh git repo with one commit, standing in for the checkout a
    window session's record names, so no test writes the real repo's
    record file."""
    folder.mkdir(parents=True)
    git(folder, "init", "-q")
    (folder / "tracked.txt").write_text("one\n")
    git(folder, "add", "tracked.txt")
    git(folder, "-c", "user.name=test", "-c", "user.email=test@example.invalid",
        "commit", "-q", "-m", "fixture")
    return folder


def realapp_lines(repo):
    """The records in `repo`'s real-app file, oldest first; [] when there is
    none. A repo make_repo made keeps its git files in .git."""
    path = repo / ".git" / hm.REALAPP_FILE
    if not path.is_file():
        return []
    return [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines()]


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


class TransportLayer(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="hydra_mcp_test_")
        self.root = Path(self.tmp.name)
        self.exe_dir = self.root / "Release"
        self.exe_dir.mkdir()
        for exe in (hm.HEADLESS_EXE, hm.WINDOW_EXE):
            (self.exe_dir / exe).write_bytes(b"")
        self.processes = []
        self.repo = make_repo(self.root / "repo")
        self.helper = self.new_helper()

    def new_helper(self, **kw):
        args = dict(exe_dir=self.exe_dir, launcher=fake_launcher, temp_root=self.root,
                    process_lister=lambda: self.processes, repo=self.repo)
        args.update(kw)
        return hm.Helper(**args)

    def tearDown(self):
        self.helper.shutdown()
        self.tmp.cleanup()

    # -- the wire -------------------------------------------------------------

    def test_the_hello_line_gives_the_timeout_log_and_crash_dump(self):
        out = self.helper.start("headless")
        s = self.helper.session
        self.assertIn("started a headless session", out)
        self.assertEqual(s.hello["pid"], s.proc.pid)
        self.assertEqual(s.timeout, fake.SESSION_TIMEOUT_S)
        self.assertIn(f"wait cap: {fake.SESSION_TIMEOUT_S} s", out)
        # An empty log means stdout, which the helper captures itself.
        self.assertEqual(s.hello["log"], "")
        self.assertEqual(s.log_path, s.scratch / "stdout.log")
        self.assertEqual(s.crash_dump, Path(s.hello["crash_dump"]))

    def test_a_hello_line_without_a_timeout_ends_the_session(self):
        self.helper._read_line = lambda sess, wait_s: (
            '{"ok":true,"hello":{"mode":"headless","pid":1,"log":"","crash_dump":""}}')
        out = self.helper.start("headless")
        self.assertIn("was not a hello line with a timeout, a log and a crash_dump", out)
        self.assertIsNone(self.helper.session)

    def test_a_hello_line_without_a_log_ends_the_session(self):
        self.helper._read_line = lambda sess, wait_s: '{"ok":true,"hello":{"timeout":30}}'
        out = self.helper.start("headless")
        self.assertIn("was not a hello line with a timeout, a log and a crash_dump", out)
        self.assertIsNone(self.helper.session)

    def test_the_line_goes_exactly_as_given_and_the_text_comes_back_as_given(self):
        self.helper.start("headless")
        # The stand-in echoes the line it got as its reply's text.
        for line in ("type Search |  two blanks  ", "  leading", "text | x"):
            self.assertEqual(self.helper.send(line), line)

    def test_a_reply_that_is_not_an_object_is_reported(self):
        self.helper.start("headless")
        self.helper._read_line = lambda sess, wait_s: "5"
        ok, text, _ = self.helper.command("state")
        self.assertFalse(ok)
        self.assertIn("not a JSON reply", text)

    def test_a_reply_without_a_timeout_is_reported(self):
        self.helper.start("headless")
        self.helper._read_line = lambda sess, wait_s: '{"ok":true,"text":"ok"}'
        ok, text, _ = self.helper.command("state")
        self.assertFalse(ok)
        self.assertIn("not a JSON reply", text)
        self.assertEqual(self.helper.session.timeout, fake.SESSION_TIMEOUT_S)

    def test_line_break_is_refused_before_sending(self):
        self.helper.start("headless")
        out = self.helper.send("type Search | a\nquit")
        self.assertTrue(out.startswith("refused: one command is one line"), out)
        self.assertIsNotNone(self.helper.session)
        self.assertEqual(self.helper.send("state"), "state")

    def test_run_splits_a_script_on_line_breaks_only(self):
        self.helper.start("headless")
        sent = []
        real = self.helper.command
        self.helper.command = lambda line: (sent.append(line), real(line))[1]
        # ASCII separators only: the stand-in logs each line to a console
        # that may not encode the Unicode ones; test_a_cut_reply_keeps_a_line_whole
        # covers those.
        self.helper.run("type **/##search | a\x0cb\x0bc\x1cd\x1de\x1ef\r\nstate\rtext\n")
        self.assertEqual(sent, ["type **/##search | a\x0cb\x0bc\x1cd\x1de\x1ef", "state", "text"])

    def test_a_cut_reply_keeps_a_line_whole(self):
        out = hm.cap_reply("a\x0cb\x85c d e\n" + "c" * 2000 + "\n", cap=200)
        self.assertEqual(out, "a\x0cb\x85c d e\n[cut at 200 characters: 1 of 2 lines dropped;"
                              " contains= narrows hydra_text and hydra_dump]")

    # -- the four transport failures ----------------------------------------

    def test_hanging_hydra_is_reported_and_killed(self):
        helper = self.new_helper(reply_margin_s=1)
        try:
            helper.start("headless")
            proc = helper.session.proc
            helper.session.timeout = 0  # as if a reply had reported 0
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

    def test_exiting_hydra_is_reported_with_code_log_tail_and_dump(self):
        self.helper.start("headless")
        dump = self.helper.session.crash_dump
        out = self.helper.send("fake-exit 3 dump")
        self.assertTrue(out.startswith("Hydra exited with code 3."), out)
        self.assertIn("fake: exiting with code 3", out)
        self.assertIn(f"crash dump: {dump}", out)
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

    def test_a_cut_reply_ends_with_the_given_hint(self):
        out = hm.cap_reply("a\n" + "b" * 2000 + "\n", cap=200, hint="ask for less")
        self.assertEqual(out, "a\n[cut at 200 characters: 1 of 2 lines dropped; ask for less]")

    def test_one_long_line_is_cut_too(self):
        out = hm.cap_reply("y" * 50_000, cap=1000)
        self.assertLessEqual(len(out), 1000)
        self.assertTrue(out.endswith("1 of 1 lines dropped; contains= narrows hydra_text and hydra_dump]"))

    def test_missing_exe_names_the_build_command(self):
        (self.exe_dir / hm.HEADLESS_EXE).unlink()
        out = self.helper.start("headless")
        self.assertIn(r".\build_cpp.ps1 -Target hydra_uitest", out)
        self.assertIsNone(self.helper.session)

    # -- the snapshot and window mode ---------------------------------------

    def test_an_unreadable_process_list_refuses_the_snapshot_and_leaves_no_folder(self):
        lib = self.root / "lib"
        make_library(lib)
        helper = self.new_helper(process_lister=lambda: (_ for _ in ()).throw(OSError("no list")))
        out = helper.start("window", library_dir=str(lib))
        self.assertTrue(out.startswith("refused: no list"), out)
        self.assertEqual(list(self.root.glob("hydra_live_*")), [])

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

    def test_a_crash_report_quotes_the_window_log_and_never_an_old_one(self):
        lib = self.root / "lib"
        make_library(lib)
        stale = self.exe_dir / "hydra_uitest.log"
        stale.write_text("old run: this line is stale\n")
        self.helper.start("window", library_dir=str(lib))
        log = self.helper.session.log_path
        self.assertEqual(Path(self.helper.session.hello["log"]), log)
        self.assertTrue(log.is_relative_to(self.helper.session.scratch))
        out = self.helper.send("fake-exit 3")
        self.assertTrue(out.startswith("Hydra exited with code 3."), out)
        self.assertIn(f"last lines of {log}:", out)
        self.assertIn("  fake: exiting with code 3", out)
        self.assertNotIn("this line is stale", out)

    def test_window_command_line_never_names_the_library_folder(self):
        lib = self.root / "Library Folder Q7"
        make_library(lib)
        out = self.helper.start("window", library_dir=str(lib))
        self.assertIn("started a window session", out)
        self.assertIn("hydra.db: ", out)
        s = self.helper.session
        argv = s.argv
        for flag in ("--live", "--db", "--ini", "--ui-ini", "--uitest-log"):
            self.assertIn(flag, argv)
        # The pipe's full name, which Hydra opens as given.
        self.assertEqual(argv[argv.index("--pipe") + 1], s.pipe_name)
        self.assertTrue(s.pipe_name.startswith("\\\\.\\pipe\\"), s.pipe_name)
        for arg in argv:
            self.assertNotIn(os.path.normcase(str(lib)), os.path.normcase(arg))
            self.assertNotIn("Library Folder Q7", arg)
        for flag in ("--db", "--uitest-log"):
            self.assertTrue(Path(argv[argv.index(flag) + 1]).is_relative_to(s.scratch), flag)
        self.assertEqual(self.helper.stop().splitlines()[0], "Hydra exited with code 0")

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

    # -- the real-app record --------------------------------------------------

    def start_window(self):
        lib = self.root / "lib"
        make_library(lib)
        out = self.helper.start("window", library_dir=str(lib))
        self.assertIn("started a window session", out)
        return self.helper.session

    def shoot(self, name):
        """A screenshot through the stand-in, which only echoes: its echo of
        the line is turned into the reply Hydra's screenshot verb gives."""
        real = self.helper._read_line

        def as_hydra(s, wait_s):
            msg = json.loads(real(s, wait_s))
            msg["text"] = f"screenshot: {s.scratch / name}\n"
            return json.dumps(msg)

        self.helper._read_line = as_hydra
        try:
            return self.helper.send(f"screenshot {name}")
        finally:
            del self.helper._read_line

    def head(self):
        return git(self.repo, "rev-parse", "HEAD")

    def test_window_stop_writes_realapp_record(self):
        self.start_window()
        self.helper.send("state")
        self.helper.send("text")
        self.assertTrue(self.shoot("shot.png").startswith("screenshot: "))
        self.helper.stop()
        [rec] = realapp_lines(self.repo)
        self.assertEqual(sorted(rec), sorted(
            ["kind", "commit", "dirty", "commands", "shots", "started", "stopped"]))
        self.assertEqual(rec["kind"], "realapp")
        self.assertEqual(rec["commit"], self.head())
        self.assertIs(rec["dirty"], False)
        # Every line Hydra answered counts, the screenshot too.
        self.assertEqual(rec["commands"], 3)
        self.assertEqual(rec["shots"], 1)
        started = datetime.fromisoformat(rec["started"])
        stopped = datetime.fromisoformat(rec["stopped"])
        self.assertIsNotNone(started.tzinfo)
        self.assertLessEqual(started, stopped)

    def test_a_failed_screenshot_is_no_shot(self):
        self.start_window()
        # The stand-in's echo is not Hydra's screenshot reply.
        self.helper.send("screenshot shot.png")
        self.helper.stop()
        [rec] = realapp_lines(self.repo)
        self.assertEqual((rec["commands"], rec["shots"]), (1, 0))

    def test_headless_writes_no_record(self):
        self.helper.start("headless")
        self.helper.send("state")
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")
        self.assertEqual(realapp_lines(self.repo), [])

    def test_dirty_tree_is_recorded(self):
        (self.repo / "tracked.txt").write_text("changed\n")
        self.start_window()
        self.helper.send("state")
        self.helper.stop()
        [rec] = realapp_lines(self.repo)
        self.assertIs(rec["dirty"], True)
        self.assertEqual(rec["commit"], self.head())

    def test_an_untracked_file_makes_the_tree_dirty(self):
        (self.repo / "new.txt").write_text("new\n")
        self.start_window()
        self.helper.stop()
        [rec] = realapp_lines(self.repo)
        self.assertIs(rec["dirty"], True)

    def test_user_close_writes_record(self):
        self.start_window()
        self.helper.send("state")
        # The stand-in exits 0 with no quit from the helper, as the window
        # does when the user closes it; the next reply notices.
        out = self.helper.send("fake-exit 0")
        self.assertTrue(out.startswith("Hydra's window closed and Hydra exited with code 0"), out)
        [rec] = realapp_lines(self.repo)
        self.assertEqual(rec["commands"], 1)
        self.assertIn(f"real-app record for commit {self.head()} added to", out)

    def test_crash_writes_no_record(self):
        self.start_window()
        self.helper.send("state")
        out = self.helper.send("fake-exit 3")
        self.assertTrue(out.startswith("Hydra exited with code 3."), out)
        self.assertIn("no real-app record written", out)
        self.assertEqual(realapp_lines(self.repo), [])

    def test_a_killed_window_writes_no_record(self):
        self.helper = self.new_helper(reply_margin_s=1)
        self.start_window()
        self.helper.session.timeout = 0  # as if a reply had reported 0
        out = self.helper.send("fake-hang")
        self.assertTrue(out.startswith("Hydra did not answer within 1 s"), out)
        self.assertIn("no real-app record written", out)
        self.assertEqual(realapp_lines(self.repo), [])

    def test_keep_window_writes_record(self):
        s = self.start_window()
        self.helper.send("state")
        out = self.helper.stop(keep_window=True)
        self.assertTrue(out.startswith("left the window open"), out)
        [rec] = realapp_lines(self.repo)
        self.assertEqual(rec["commands"], 1)
        s.pipe.close()
        self.assertEqual(s.proc.wait(timeout=hm.REPLY_MARGIN_S), 0)

    def test_a_new_start_writes_the_old_sessions_record(self):
        self.start_window()
        self.helper.send("state")
        out = self.helper.start("headless")
        self.assertIn("stopped the running session first: Hydra exited with code 0", out)
        self.assertEqual(len(realapp_lines(self.repo)), 1)

    def test_helper_exit_writes_record(self):
        self.start_window()
        self.helper.send("state")
        self.helper.shutdown()
        [rec] = realapp_lines(self.repo)
        self.assertEqual(rec["commands"], 1)

    def test_helper_exit_after_a_crash_writes_no_record(self):
        s = self.start_window()
        self.helper.send("state")
        # The window died with a nonzero code and no tool call has noticed.
        s.proc.kill()
        s.proc.wait()
        self.helper.shutdown()
        self.assertEqual(realapp_lines(self.repo), [])

    def test_a_record_for_the_real_repo_is_refused(self):
        # The guard raises before the real writer runs, so nothing is written.
        with self.assertRaises(AssertionError):
            hm.realapp_record(object(), hm.REPO)

    def test_stop_reply_names_record(self):
        self.start_window()
        out = self.helper.stop()
        path = self.repo / ".git" / hm.REALAPP_FILE
        self.assertEqual(out, f"Hydra exited with code 0\nreal-app record for commit {self.head()}"
                              f" added to {path}")

    def test_no_repo_says_no_record_and_still_stops(self):
        self.helper = self.new_helper(repo=self.root / "not a repo")
        self.start_window()
        out = self.helper.stop()
        self.assertTrue(out.startswith("Hydra exited with code 0\nno real-app record written: git "), out)
        self.assertIsNone(self.helper.session)


class SdkClient(unittest.TestCase):
    def test_the_sdk_client_lists_every_tool(self):
        from mcp import Client, StdioServerParameters

        async def names():
            params = StdioServerParameters(command=sys.executable, args=["-I", str(HELPER_SCRIPT)])
            async with Client(params) as client:
                result = await client.list_tools()
                return {t.name for t in result.tools}

        self.assertEqual(asyncio.run(names()), TOOL_NAMES)

    def test_open_song_sends_open_and_the_title(self):
        sent = []
        real_send = hm.HELPER.send
        hm.HELPER.send = lambda line: sent.append(line) or "ok"
        try:
            asyncio.run(hm.build_server().call_tool("hydra_open_song", {"title": "AC/DC #1"}))
        finally:
            hm.HELPER.send = real_send
        self.assertEqual(sent, ["open AC/DC #1"])


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

# Pinned from the real exe's replies on 2026-10-14. The engine's log lines
# start with a frame number, which varies, so a failure is pinned by its first
# line and the engine's message.
UNKNOWN_BUTTON = "failed: click No such button\n"
UNKNOWN_BUTTON_LOG = "Error 'Unable to locate item: '//Hydra/No such button' (0x00000000)'"


@unittest.skipIf(NO_LIVE is not None, NO_LIVE or "")
class RealExeLayer(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="hydra_mcp_real_")
        self.repo = make_repo(Path(self.tmp.name) / "repo")
        self.helper = hm.Helper(temp_root=self.tmp.name, process_lister=lambda: [], repo=self.repo)

    def tearDown(self):
        self.helper.shutdown()
        self.tmp.cleanup()

    def scan(self):
        # The smoke script's lines up to its first "expect-text Burnout": a
        # scan of the corpus. The script goes on to reset and quit, which
        # would leave no library and no session to check.
        lines = SMOKE.read_text(encoding="utf-8").splitlines()
        upto = [ln.strip() for ln in lines].index("expect-text Burnout") + 1
        out = self.helper.run("\n".join(lines[:upto]))
        self.assertNotIn("stopped at line", out)
        self.assertIn("charts=97", self.helper.send("state"))

    def burnout_row(self, text):
        # The library row for the smoke script's Burnout, read from Hydra's own
        # text reply: the GUI owns the columns, so the test never retypes them.
        rows = [ln + "\n" for ln in text.split("\n") if ln.startswith("| Burnout |")]
        self.assertEqual(len(rows), 1, text)
        return rows[0]

    # -- the session --------------------------------------------------------

    def test_the_real_hello_line(self):
        self.helper.start("headless")
        s = self.helper.session
        self.assertEqual(sorted(s.hello), ["crash_dump", "log", "mode", "pid", "scratch", "timeout"])
        self.assertEqual(s.hello["mode"], "headless")
        self.assertEqual(s.hello["pid"], s.proc.pid)
        self.assertEqual(s.timeout, 30)
        self.assertTrue(Path(s.hello["scratch"]).is_dir())
        # hydra_uitest logs to stdout, which the helper captures.
        self.assertEqual(s.hello["log"], "")
        self.assertEqual(s.log_path, s.scratch / "stdout.log")
        self.assertEqual(s.crash_dump.name, "crash.dmp")
        self.assertEqual(s.crash_dump.parent, Path(s.hello["scratch"]))

    def test_first_tool_call_starts_headless(self):
        out = self.helper.send("state")
        self.assertIn("started a headless session", out)
        self.assertIn("store=ready", out)

    def test_headless_session(self):
        self.helper.start("headless")
        self.assertIn("store=ready", self.helper.send("state"))
        self.scan()
        self.assertEqual(self.helper.send("reset"), "ok")
        self.assertIn("charts=0", self.helper.send("state"))
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")

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

    # -- Hydra's rules, carried as given ------------------------------------

    def test_strict_numbers_are_refused_by_hydra(self):
        self.helper.start("headless")
        for arg in ("90s", "abc", ""):
            self.assertEqual(
                self.helper.send(f"timeout {arg}"),
                f'timeout needs a number of seconds, 0 or more, and nothing else; got "{arg}"\n')
        self.assertEqual(self.helper.session.timeout, 30)

    def test_the_wait_cap_and_the_timeout_every_reply_carries(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("wait 31"),
                         "wait 31 is longer than the 30 s timeout in force; raise the timeout first\n")
        self.assertEqual(self.helper.send("timeout 5"), "ok")
        self.assertEqual(self.helper.session.timeout, 5)
        self.assertEqual(self.helper.send("reset"), "ok")
        self.assertEqual(self.helper.session.timeout, 5)

    def test_one_off_timeouts_are_judged_by_hydra_and_put_back(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.with_timeout("wait-idle", 5), "ok")
        self.assertEqual(self.helper.session.timeout, 30)
        self.assertEqual(self.helper.with_timeout("wait-text never on screen", 0),
                         '"never on screen" did not appear within 0 s\n')
        self.assertEqual(self.helper.session.timeout, 30)
        for bad, shown in ((-1, "-1"), (float("inf"), "inf"), (float("nan"), "nan")):
            self.assertEqual(
                self.helper.with_timeout("wait-idle", bad),
                f'timeout needs a number of seconds, 0 or more, and nothing else; got "{shown}"\n')
        self.assertEqual(self.helper.session.timeout, 30)
        # The value put back is the one Hydra reported, whatever it was.
        self.assertEqual(self.helper.send("timeout 12.5"), "ok")
        self.assertEqual(self.helper.with_timeout("wait-idle", 5), "ok")
        self.assertEqual(self.helper.session.timeout, 12.5)

    def test_a_long_timeout_is_put_back_exactly(self):
        self.helper.start("headless")
        # More digits than printf's default keeps: the reply must carry them all.
        self.assertEqual(self.helper.send("timeout 12.345678901"), "ok")
        self.assertEqual(self.helper.session.timeout, 12.345678901)
        self.assertEqual(self.helper.with_timeout("wait-idle", 5), "ok")
        self.assertEqual(self.helper.session.timeout, 12.345678901)

    def test_text_and_dump_filters_are_hydras(self):
        self.helper.start("headless")
        self.scan()
        row = self.burnout_row(self.helper.send("text"))
        self.assertEqual(self.helper.send(hm.filtered("text", "Burnout")), row)
        self.assertEqual(self.helper.send(hm.filtered("text", "zzqx-none")),
                         '(no line holding "zzqx-none")\n')
        dump = ('window "Hydra" id=0xBAEC0FDA pos=(0,0) size=(1280,800)\n'
                '  "Scan library" id=0xA6E9877D rect=(150,58 81x24)\n')
        self.assertEqual(self.helper.send(hm.filtered("dump Hydra", "Scan library")), dump)
        # A plain dump is every window: Hydra's default, not the helper's.
        self.assertEqual(self.helper.send(hm.filtered("dump", "Scan library")), dump)

    def test_every_text_reply_ends_lines_in_a_bare_newline(self):
        self.helper.start("headless")
        self.scan()
        text = self.helper.send("text")
        self.assertNotIn("\r", text)
        self.burnout_row(text)  # the scanned library still lists Burnout, once
        self.assertEqual(self.helper.send("text |"), text)

    def test_rect_on_a_missing_ref_fails(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("rect No such thing"), 'rect found no item "No such thing"\n')

    def test_a_bare_bar_clears_the_field(self):
        self.helper.start("headless")
        self.scan()
        no_match = hm.filtered("text", "Nothing matches")
        self.assertEqual(self.helper.send("type **/##search | zzqx"), "ok")
        self.assertEqual(self.helper.send(no_match), "Nothing matches.\n")
        # The bar with nothing after it, as a client that trims blanks sends it.
        self.assertEqual(self.helper.send("type **/##search |"), "ok")
        self.assertEqual(self.helper.send(no_match), '(no line holding "Nothing matches")\n')
        # And the helper's own clear, which keeps the trailing blank.
        self.assertEqual(self.helper.send("type **/##search | zzqx"), "ok")
        self.assertEqual(self.helper.send("type **/##search | "), "ok")
        self.assertEqual(self.helper.send(no_match), '(no line holding "Nothing matches")\n')

    def test_failure_carries_hydras_text_and_the_session_survives(self):
        self.helper.start("headless")
        out = self.helper.send("click No such button")
        self.assertTrue(out.startswith(UNKNOWN_BUTTON), out)
        self.assertIn(UNKNOWN_BUTTON_LOG, out)
        self.assertEqual(self.helper.send("frob"), 'unknown verb "frob"\n')
        self.assertIn("store=ready", self.helper.send("state"))

    def test_run_sends_every_line_and_stops_at_the_first_failure(self):
        self.helper.start("headless")
        out = self.helper.run("state\n\n# a comment\nclick No such button\nstate")
        self.assertIn("> \nok\n> # a comment\nok\n> click No such button\n" + UNKNOWN_BUTTON, out)
        self.assertIn(UNKNOWN_BUTTON_LOG, out)
        self.assertEqual(out.count("> state"), 1)
        self.assertTrue(out.endswith("stopped at line 4 of 5"), out)

    def test_a_run_after_a_command_carries_one_line_number_the_helpers(self):
        self.helper.start("headless")
        self.assertIn("store=ready", self.helper.send("state"))
        out = self.helper.run("click No such button")
        self.assertEqual(re.findall(r"line \d+", out), ["line 1"], out)
        self.assertTrue(out.endswith("stopped at line 1 of 1"), out)

    def test_screenshot_is_saved_under_the_name_given(self):
        self.helper.start("headless")
        scratch = self.helper.session.scratch
        for name in ("shot.png", "bare"):
            self.assertEqual(self.helper.send(f"screenshot {name}"), f"screenshot: {scratch / name}\n")
            self.assertEqual((scratch / name).read_bytes()[:8], b"\x89PNG\r\n\x1a\n")
        self.assertFalse((scratch / "bare.png").exists())

    # -- the vendored engine's Hydra patches and the deep dump --------------
    # THIRD_PARTY_NOTICES.txt lists the patches; the literals are real replies.

    def test_a_focused_wildcard_ref_finds_the_popups_checkbox(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("click **/###analysissettings"), "ok")
        self.assertEqual(self.helper.send("rect //$FOCUSED/**/##bass2x"),
                         '"##bass2x" id=0x357E5949 rect=(723,176 24x24) checked\n')

    def test_a_failed_wildcard_check_leaves_wildcard_clicks_working(self):
        self.helper.start("headless")
        out = self.helper.send("check **/###nosuchitem")
        self.assertTrue(out.startswith("failed: check **/###nosuchitem\n"), out)
        self.assertIn("Unable to locate item: '//Hydra/**/###nosuchitem'", out)
        self.assertEqual(self.helper.send("click **/###analysissettings"), "ok")

    def test_dump_shows_the_whole_label(self):
        self.helper.start("headless")
        self.assertEqual(
            self.helper.send(hm.filtered("dump Hydra", "###analysissettings")),
            'window "Hydra" id=0xBAEC0FDA pos=(0,0) size=(1280,800)\n'
            '  "Analysis settings: Score range 2 scores###analysissettings"'
            ' id=0xB7DC11A6 rect=(611,58 257x24)\n')

    def test_a_popup_dump_lists_the_checkboxes_inside_its_tables(self):
        self.helper.start("headless")
        self.assertEqual(self.helper.send("click **/###analysissettings"), "ok")
        self.assertEqual(
            self.helper.send(hm.filtered("dump ##Popup_", "##bass2x")),
            'window "##Popup_2cd77690" id=0x0E54559D pos=(611,82) size=(319,365) popup\n'
            '    "##bass2x" id=0x357E5949 rect=(723,176 24x24) checked\n')

    def window_seed(self):
        """A library folder for a window session, and self.helper a fresh
        helper to start it with. The seed is the headless session's scratch
        folder after a scan, copied once that session has closed its
        database. hydra_uitest deletes the folder on exit unless told
        --keep-temp, so this session gets it. (tearDown shuts down whichever
        helper self.helper is by then.)"""
        self.helper = hm.Helper(temp_root=self.tmp.name, process_lister=lambda: [],
                                launcher=keep_temp_launcher, repo=self.repo)
        self.helper.start("headless")
        self.scan()
        engine = self.helper.session.engine_scratch()
        self.assertEqual(self.helper.stop(), "Hydra exited with code 0")
        self.helper = hm.Helper(temp_root=self.tmp.name, process_lister=lambda: [], repo=self.repo)
        lib = Path(self.tmp.name) / "seed library"
        lib.mkdir()
        try:
            for name in hm.SNAPSHOT_FILES:
                if (engine / name).is_file():
                    shutil.copy2(engine / name, lib / name)
        finally:
            shutil.rmtree(engine, ignore_errors=True)
        self.assertTrue((lib / "hydra.db").is_file())
        return lib

    @unittest.skipUnless(os.environ.get("HYDRA_LIVE_WINDOW_TEST") == "1",
                         "opens a window; set HYDRA_LIVE_WINDOW_TEST=1")
    def test_closing_the_window_ends_hydra(self):
        lib = self.window_seed()
        # Left open by keep_window, the helper still holds the pipe, so only
        # the close can end the live test.
        self.helper.start("window", library_dir=str(lib))
        proc = self.helper.session.proc
        self.assertIn("left the window open", self.helper.stop(keep_window=True))
        post_close(proc.pid)
        self.assertEqual(proc.wait(timeout=hm.REPLY_MARGIN_S), 0)
        # Attached: the close ends Hydra too, and the next command reports it
        # as a plain stop.
        self.helper.start("window", library_dir=str(lib))
        proc = self.helper.session.proc
        post_close(proc.pid)
        self.assertEqual(proc.wait(timeout=hm.REPLY_MARGIN_S), 0)
        out = self.helper.send("state")
        self.assertTrue(out.startswith("Hydra's window closed and Hydra exited with code 0:"), out)

    @unittest.skipUnless(os.environ.get("HYDRA_LIVE_WINDOW_TEST") == "1",
                         "opens a window; set HYDRA_LIVE_WINDOW_TEST=1")
    def test_holding_esc_in_the_window_leaves_the_session_working(self):
        lib = self.window_seed()
        self.helper.start("window", library_dir=str(lib))
        self.assertIn("store=ready", self.helper.send("state"))
        # The person watching holds Esc (to close a popup, say). The test
        # engine reads a held host Esc as "abort the test".
        post_key(self.helper.session.proc.pid, VK_ESCAPE, held_s=ESC_HOLD_S)
        out = self.helper.send("state")
        self.assertIn("store=ready", out)
        self.assertIn("store=ready", self.helper.send("state"))
        self.assertEqual(self.helper.stop().splitlines()[0], "Hydra exited with code 0")

    @unittest.skipUnless(os.environ.get("HYDRA_LIVE_WINDOW_TEST") == "1",
                         "opens a window; set HYDRA_LIVE_WINDOW_TEST=1")
    def test_window_session_on_a_seed_from_headless(self):
        lib = self.window_seed()

        out = self.helper.start("window", library_dir=str(lib))
        self.assertIn("started a window session", out)
        s = self.helper.session
        self.assertEqual(s.hello.get("mode"), "window")
        # Hydra.exe logs where the helper pointed it, and writes no crash dump.
        self.assertEqual(Path(s.hello["log"]), s.scratch / hm.WINDOW_LOG)
        self.assertEqual(s.hello["crash_dump"], "")
        self.assertIsNone(s.crash_dump)
        self.assertIn("charts=97", self.helper.send("state"))
        window_engine = s.engine_scratch()
        self.assertTrue(window_engine.is_dir())
        self.assertEqual(self.helper.stop().splitlines()[0], "Hydra exited with code 0")
        # Live mode deletes its scratch copy of the library on exit.
        self.assertFalse(window_engine.exists())


def visible_windows(pid):
    """The visible windows of process `pid`; the test fails when it has none."""
    user32 = ctypes.WinDLL("user32", use_last_error=True)
    found = []

    @ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    def each(hwnd, _):
        owner = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if owner.value == pid and user32.IsWindowVisible(hwnd):
            found.append(hwnd)
        return True

    user32.EnumWindows(each, 0)
    if not found:
        raise AssertionError(f"process {pid} shows no window")
    return user32, found


def post_close(pid):
    """Ask each visible window of process `pid` to close (WM_CLOSE), as the
    user's click on the close button does."""
    user32, found = visible_windows(pid)
    for hwnd in found:
        user32.PostMessageW(hwnd, WM_CLOSE, 0, 0)


def post_key(pid, vk, held_s):
    """Press key `vk` in the window of process `pid`, hold it `held_s`
    seconds and let go, as the user's keyboard does (the window gets the
    messages the keyboard sends it)."""
    user32, found = visible_windows(pid)
    scan = user32.MapVirtualKeyW(vk, 0)
    for hwnd in found:
        user32.PostMessageW(hwnd, WM_KEYDOWN, vk, 1 | scan << 16)
    time.sleep(held_s)
    for hwnd in found:
        user32.PostMessageW(hwnd, WM_KEYUP, vk, 1 | scan << 16 | 3 << 30)


def keep_temp_launcher(exe, args):
    """The helper's launch, plus --keep-temp, so the engine's scratch folder
    outlives the process (the window test seeds from it)."""
    return hm.default_launcher(exe, list(args) + ["--keep-temp"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
