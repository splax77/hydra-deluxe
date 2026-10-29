"""The MCP server that lets Claude drive Hydra's GUI one command at a time.

Claude Code starts this helper (see .mcp.json at the repo root) and calls its
tools. The helper launches a dev build of hydra_uitest.exe (headless) or
Hydra.exe (the real window), connects to its live channel, sends each tool
call as one command line and returns the reply's text as Hydra wrote it. The
design and the wire protocol are in "T8: one owner for every command rule" in
docs/superpowers/plans/2026-10-14-hydra-mcp-server.md.

Run it as:  py -I tools/hydra_ui_mcp.py

The helper only carries messages. Hydra owns every command rule: what a line
means, which numbers it takes, the wording of a reply, the filters, the
timeout in force and where it logs. The helper owns the transport (launch,
the pipe name, stop), the caps on every reply, the reports for a dead or
silent Hydra, the tool list, the line numbers of a hydra_run script, and the
snapshot of the user's library (snapshot_library, the only code that knows
that folder's path). What it never does: build anything, or name the library
folder on a command line.
"""

import atexit
import ctypes
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from ctypes import wintypes
from datetime import datetime
from pathlib import Path

# The user's answers to the plan's open questions 4 and 5 (2026-10-14).
# This is the one home of these three numbers.
REPLY_MARGIN_S = 15.0  # how much longer than the timeout in force to wait for a reply
HELLO_TIMEOUT_S = 60.0  # how long hydra_start waits for the hello line
REPLY_CAP_CHARS = 12_000  # the longest reply Claude is handed

# The timeout in force is not kept here: the hello line and every reply carry
# it (its owner is CommandSession in tests/ui/uitest_commands.h).

# Plumbing granularity, not behaviour: how many log lines a crash report
# quotes, and how often the helper retries opening the pipe while the
# launched exe is still making it (bounded by HELLO_TIMEOUT_S).
LOG_TAIL_LINES = 20
CONNECT_RETRY_S = 0.05

DEFAULT_LIBRARY_DIR = r"C:\Program Files\Hydra"
LIBRARY_DIR_ENV = "HYDRA_LIBRARY_DIR"

REPO = Path(__file__).resolve().parent.parent
EXE_DIR = REPO / "build-cpp" / "Release"
HEADLESS_EXE = "hydra_uitest.exe"
WINDOW_EXE = "Hydra.exe"
BUILD_TARGETS = {HEADLESS_EXE: "hydra_uitest", WINDOW_EXE: "Hydra"}
# The file the helper asks Hydra.exe to log to (--uitest-log), in its own
# scratch folder. hydra_uitest logs to stdout, which the helper captures.
WINDOW_LOG = "window.log"

# What a snapshot copies, in order. Only hydra.db must exist.
SNAPSHOT_FILES = ("hydra.db", "hydra.db-wal", "hydra.db-shm", "hydra_settings.ini", "hydra_ui.ini")
CLOSE_HYDRA_FIRST = "Close Hydra first: a database it has open is not a clean copy."
NO_SESSION = "no session; call hydra_start"


class SnapshotRefused(Exception):
    pass


class HydraGone(Exception):
    """The session ended under us; the message is the report for Claude."""


# ---------------------------------------------------------------------------
# The running-Hydra check and the snapshot.


def running_processes():
    """Every running process as (image name, full exe path). The path is None
    when Windows will not say. Only Hydra.exe's path is looked up, since that
    is the only one anyone asks about."""
    k32 = ctypes.WinDLL("kernel32", use_last_error=True)

    class PROCESSENTRY32W(ctypes.Structure):
        _fields_ = [
            ("dwSize", wintypes.DWORD), ("cntUsage", wintypes.DWORD),
            ("th32ProcessID", wintypes.DWORD), ("th32DefaultHeapID", ctypes.c_void_p),
            ("th32ModuleID", wintypes.DWORD), ("cntThreads", wintypes.DWORD),
            ("th32ParentProcessID", wintypes.DWORD), ("pcPriClassBase", ctypes.c_long),
            ("dwFlags", wintypes.DWORD), ("szExeFile", ctypes.c_wchar * 260),
        ]

    k32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
    k32.CreateToolhelp32Snapshot.argtypes = [wintypes.DWORD, wintypes.DWORD]
    k32.Process32FirstW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]
    k32.Process32NextW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]
    k32.OpenProcess.restype = wintypes.HANDLE
    k32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    k32.QueryFullProcessImageNameW.argtypes = [
        wintypes.HANDLE, wintypes.DWORD, wintypes.LPWSTR, ctypes.POINTER(wintypes.DWORD)]
    k32.CloseHandle.argtypes = [wintypes.HANDLE]
    TH32CS_SNAPPROCESS = 0x2
    PROCESS_QUERY_LIMITED_INFORMATION = 0x1000

    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    if snap in (None, wintypes.HANDLE(-1).value):
        raise OSError(ctypes.get_last_error(), "CreateToolhelp32Snapshot failed")
    out = []
    try:
        entry = PROCESSENTRY32W()
        entry.dwSize = ctypes.sizeof(PROCESSENTRY32W)
        more = k32.Process32FirstW(snap, ctypes.byref(entry))
        while more:
            name = entry.szExeFile
            path = None
            if name.lower() == WINDOW_EXE.lower():
                h = k32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, entry.th32ProcessID)
                if h:
                    buf = ctypes.create_unicode_buffer(32768)
                    size = wintypes.DWORD(len(buf))
                    if k32.QueryFullProcessImageNameW(h, 0, buf, ctypes.byref(size)):
                        path = buf.value
                    k32.CloseHandle(h)
            out.append((name, path))
            more = k32.Process32NextW(snap, ctypes.byref(entry))
    finally:
        k32.CloseHandle(snap)
    return out


def _same_folder(a, b):
    # realpath first, so a junction, a symbolic link or an 8.3 short name
    # cannot make one folder look like two.
    def key(p):
        return os.path.normcase(os.path.realpath(p))
    return key(a) == key(b)


def hydra_running_from(library_dir, processes):
    """The Hydra.exe processes in `processes` that run from `library_dir`.
    A Hydra.exe whose path Windows would not give counts too, because it
    might be that one."""
    hits = []
    for name, path in processes:
        if name.lower() != WINDOW_EXE.lower():
            continue
        if path is None:
            hits.append(f"{WINDOW_EXE} (Windows did not say which folder it runs from)")
        elif _same_folder(os.path.dirname(path), library_dir):
            hits.append(path)
    return hits


def snapshot_library(library_dir, dest, processes):
    """Copy the user's library files into `dest` and return (copied, report).

    This is the only place that knows the library folder's path: `library_dir`
    if given, else the HYDRA_LIBRARY_DIR variable, else the installed folder.
    It only reads that folder. `copied` maps each copied file's name to its
    path inside `dest`; `report` lists what was copied, with sizes and
    modification times, for the reply. Raises SnapshotRefused when a Hydra.exe
    runs from the folder or the database is missing."""
    lib = Path(library_dir or os.environ.get(LIBRARY_DIR_ENV) or DEFAULT_LIBRARY_DIR)
    running = hydra_running_from(lib, processes)
    if running:
        raise SnapshotRefused(f"{CLOSE_HYDRA_FIRST} Running: {'; '.join(running)}")
    if not (lib / SNAPSHOT_FILES[0]).is_file():
        raise SnapshotRefused(f"no {SNAPSHOT_FILES[0]} in {lib}")
    dest.mkdir(parents=True, exist_ok=True)
    copied = {}
    lines = [f"snapshot of {lib}, taken {datetime.now():%Y-%m-%d %H:%M:%S}:"]
    for name in SNAPSHOT_FILES:
        src = lib / name
        if not src.is_file():
            lines.append(f"  {name}: not present")
            continue
        shutil.copy2(src, dest / name)
        st = src.stat()
        stamp = datetime.fromtimestamp(st.st_mtime)
        lines.append(f"  {name}: {st.st_size:,} bytes, modified {stamp:%Y-%m-%d %H:%M:%S}")
        copied[name] = dest / name
    return copied, "\n".join(lines)


# ---------------------------------------------------------------------------
# Reply shaping: only the cap. The text inside is Hydra's.


def split_lines(text):
    """`text` cut into lines at its line breaks and nothing else, a last empty
    piece dropped as str.splitlines drops it. The pipe frames commands and
    replies by line break, and str.splitlines also cuts at form feeds, the
    separator characters and Unicode line separators, which a line may hold."""
    lines = text.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    if lines[-1] == "":
        lines.pop()
    return lines


CAP_HINT = "contains= narrows hydra_text and hydra_dump"


def cap_reply(text, cap=REPLY_CAP_CHARS, hint=CAP_HINT):
    """Cut `text` to at most `cap` characters at a line boundary, with a last
    line saying how many lines were dropped and ending with `hint`, which
    says how to ask for the rest. Another server passes its own hint."""
    if len(text) <= cap:
        return text
    lines = split_lines(text)
    total = len(lines)

    def note(dropped):
        return f"[cut at {cap:,} characters: {dropped} of {total} lines dropped; {hint}]"

    room = cap - len(note(total)) - 1
    kept, size = [], 0
    for line in lines:
        if size + len(line) + 1 > room:
            break
        kept.append(line)
        size += len(line) + 1
    if not kept:
        # One line longer than the cap: keep its start and count it dropped.
        return lines[0][:room] + "\n" + note(total)
    return "\n".join(kept) + "\n" + note(total - len(kept))


def describe_exit(code):
    if code is not None and code > 0x7FFFFFFF:
        return f"{code} (0x{code:08X})"
    return str(code)


def seconds_field(msg, key):
    """msg[key] as a float when it is a JSON number, else None."""
    v = msg.get(key)
    if isinstance(v, bool) or not isinstance(v, (int, float)):
        return None
    return float(v)


# ---------------------------------------------------------------------------
# One live session.


class Session:
    def __init__(self, mode, proc, argv, scratch, pipe_name, stdout_path, log_path, keep):
        self.mode = mode
        self.proc = proc
        self.argv = argv
        self.scratch = scratch  # the helper's own folder: snapshot, shots, output
        self.pipe_name = pipe_name
        self.stdout_path = stdout_path  # the child's stdout and stderr
        # Where the process's output goes. Until the hello line names it, the
        # file the helper itself pointed the output at.
        self.log_path = log_path
        self.crash_dump = None  # the hello line's crash_dump; None when it names none
        self.keep = keep
        self.pipe = None
        self.buf = b""
        self.hello = {}
        self.timeout = None  # seconds; the last value Hydra reported

    def engine_scratch(self):
        s = self.hello.get("scratch")
        return Path(s) if s else None


def default_launcher(exe, args):
    return [str(exe)] + list(args)


class Helper:
    """Everything the tools do, with the outside world passed in so the tests
    can swap the exes for the stand-in and the process list for a fake."""

    def __init__(self, exe_dir=EXE_DIR, launcher=default_launcher, temp_root=None,
                 process_lister=running_processes, reply_margin_s=REPLY_MARGIN_S,
                 hello_timeout_s=HELLO_TIMEOUT_S):
        self.exe_dir = Path(exe_dir)
        self.launcher = launcher
        self.temp_root = Path(temp_root) if temp_root else Path(tempfile.gettempdir())
        self.process_lister = process_lister
        self.reply_margin_s = reply_margin_s
        self.hello_timeout_s = hello_timeout_s
        self.session = None
        self.ever_started = False
        self.detached = []  # sessions left open by hydra_stop(keep_window=True)
        self.count = 0
        self.lock = threading.RLock()

    # -- starting and stopping ------------------------------------------------

    def start(self, mode, library_dir=None, keep=False):
        with self.lock:
            if mode not in ("headless", "window"):
                return f'unknown mode "{mode}"; use "headless" or "window"'
            notes = []
            if self.session is not None:
                notes.append("stopped the running session first: " + self._stop_session(False))
            self.ever_started = True
            exe = self.exe_dir / (HEADLESS_EXE if mode == "headless" else WINDOW_EXE)
            if not exe.is_file():
                return "\n".join(notes + [
                    f"{exe.name} is not built (looked in {self.exe_dir}). Build it from the repo"
                    f" root with: .\\build_cpp.ps1 -Target {BUILD_TARGETS[exe.name]}"])
            self.count += 1
            tag = f"{os.getpid()}_{self.count}"
            scratch = self.temp_root / f"hydra_live_{tag}"
            if scratch.exists():
                shutil.rmtree(scratch)
            # The pipe's full name: Hydra opens exactly this and never builds one.
            pipe_name = rf"\\.\pipe\hydra-live-{tag}"
            args = ["--live", "--pipe", pipe_name]
            if mode == "window":
                try:
                    copied, report = snapshot_library(library_dir, scratch, self.process_lister())
                except (SnapshotRefused, OSError) as e:
                    # An unreadable process list or a failed copy refuses too:
                    # no snapshot is better than a half one.
                    shutil.rmtree(scratch, ignore_errors=True)
                    return "\n".join(notes + [f"refused: {e}"])
                notes.append(report)
                args += ["--db", str(copied["hydra.db"])]
                if "hydra_settings.ini" in copied:
                    args += ["--ini", str(copied["hydra_settings.ini"])]
                if "hydra_ui.ini" in copied:
                    args += ["--ui-ini", str(copied["hydra_ui.ini"])]
            scratch.mkdir(parents=True, exist_ok=True)
            args += ["--shots", str(scratch)]
            # The child's stdout and stderr go to a file of the helper's own.
            # Hydra.exe has no console, so it is pointed at a log in the same
            # fresh folder: a log there can only be this run's, even if the
            # process dies before its hello line names it.
            stdout_path = scratch / "stdout.log"
            log_path = stdout_path
            if mode == "window":
                log_path = scratch / WINDOW_LOG
                args += ["--uitest-log", str(log_path)]
            argv = self.launcher(exe, args)
            # The child must never inherit this helper's stdin and stdout:
            # they are the MCP channel to Claude Code.
            out = open(stdout_path, "wb")
            try:
                proc = subprocess.Popen(
                    argv, stdin=subprocess.DEVNULL, stdout=out, stderr=subprocess.STDOUT,
                    creationflags=subprocess.CREATE_NO_WINDOW)
            finally:
                out.close()
            s = Session(mode, proc, argv, scratch, pipe_name, stdout_path, log_path, keep)
            self.session = s
            try:
                self._connect(s)
            except HydraGone as e:
                self.session = None
                return "\n".join(notes + [str(e)])
            h = s.hello
            lines = [f"started a {h.get('mode', mode)} session",
                     f"  pid: {h.get('pid', proc.pid)}",
                     f"  engine scratch: {h.get('scratch', '?')}",
                     f"  wait cap: {s.timeout:g} s, unless a hydra_wait_idle or hydra_wait_text call gives its own",
                     f"  log: {s.log_path}",
                     f"  helper scratch: {scratch}" + (" (kept after stop)" if keep else "")]
            return "\n".join(lines + notes)

    def _connect(self, s):
        deadline = time.monotonic() + self.hello_timeout_s
        while True:
            try:
                s.pipe = open(s.pipe_name, "r+b", buffering=0)
                break
            except OSError:
                if s.proc.poll() is not None:
                    raise HydraGone(self._kill_report(s, exited=True))
                if time.monotonic() >= deadline:
                    raise HydraGone(self._kill_report(
                        s, silent=f"Hydra did not open its live channel within {self.hello_timeout_s:g} s"))
                time.sleep(CONNECT_RETRY_S)
        line = self._read_line(s, max(0.0, deadline - time.monotonic()))
        if line is None:
            raise HydraGone(self._kill_report(
                s, silent=f"Hydra did not send its hello line within {self.hello_timeout_s:g} s"))
        try:
            msg = json.loads(line)
            hello = msg["hello"]
            if not msg.get("ok") or not isinstance(hello, dict):
                raise ValueError
            timeout = seconds_field(hello, "timeout")
            log, crash_dump = hello["log"], hello["crash_dump"]
            if timeout is None or not isinstance(log, str) or not isinstance(crash_dump, str):
                raise ValueError
        except (ValueError, KeyError, TypeError):
            raise HydraGone(self._kill_report(
                s, silent="Hydra's first line was not a hello line with a timeout, a log and a"
                          f" crash_dump: {line[:200]!r}"))
        s.hello = hello
        s.timeout = timeout
        # An empty log means stdout, which the helper captures itself; an
        # empty crash_dump means nothing writes one.
        s.log_path = Path(log) if log else s.stdout_path
        s.crash_dump = Path(crash_dump) if crash_dump else None

    def stop(self, keep_window=False):
        with self.lock:
            if self.session is None:
                return "no session is running"
            return self._stop_session(keep_window)

    def _stop_session(self, keep_window):
        s = self.session
        self.session = None
        if keep_window and s.mode == "window":
            # The pipe stays open: the live test ends on disconnect, so the
            # window lives until the user closes it or this helper exits.
            self.detached.append(s)
            return f"left the window open (pid {s.proc.pid}); its files stay in {s.scratch}"
        try:
            s.pipe.write(b"quit\n")
        except (OSError, ValueError, AttributeError):
            pass
        try:
            code = s.proc.wait(timeout=self.reply_margin_s)
            msg = f"Hydra exited with code {describe_exit(code)}"
        except subprocess.TimeoutExpired:
            s.proc.kill()
            s.proc.wait()
            msg = f"Hydra did not exit within {self.reply_margin_s:g} s of quit, so the helper killed it"
        self._close(s)
        return msg + self._cleanup(s)

    def _close(self, s):
        if s.pipe is not None:
            try:
                s.pipe.close()
            except OSError:
                pass

    def _cleanup(self, s):
        if s.keep:
            return f"; kept {s.scratch}"
        try:
            shutil.rmtree(s.scratch)
        except FileNotFoundError:
            pass
        except OSError as e:
            return f"; could not delete {s.scratch}: {e}"
        return ""

    def shutdown(self):
        """Called at exit: never leave a Hydra behind."""
        with self.lock:
            if self.session is not None:
                s = self.session
                self.session = None
                if s.proc.poll() is None:
                    s.proc.kill()
                    s.proc.wait()
                self._close(s)
                self._cleanup(s)

    # -- talking to Hydra -----------------------------------------------------

    def _read_line(self, s, wait_s):
        """One reply line, or None when none came within wait_s. Raises
        HydraGone when the pipe closed. The read runs on a worker thread so
        the wait can be capped; the pipe is only read while no write is due,
        because Windows serialises synchronous reads and writes on one handle."""
        result = {}

        def work():
            try:
                while b"\n" not in s.buf:
                    chunk = s.pipe.read(65536)
                    if not chunk:
                        result["eof"] = True
                        return
                    s.buf += chunk
                line, s.buf = s.buf.split(b"\n", 1)
                result["line"] = line.decode("utf-8", errors="replace")
            except (OSError, ValueError):
                result["eof"] = True

        t = threading.Thread(target=work, daemon=True)
        t.start()
        t.join(wait_s)
        if t.is_alive():
            return None
        if "eof" in result:
            raise HydraGone(self._kill_report(s, exited=True))
        return result["line"]

    def _kill_report(self, s, exited=False, silent=None):
        """End the session and say why: an exit code and log tail, or a
        silence that the helper ended by killing the process."""
        if self.session is s:
            self.session = None
        if silent is not None and s.proc.poll() is None:
            s.proc.kill()
        lingered = False
        try:
            code = s.proc.wait(timeout=self.reply_margin_s)
        except subprocess.TimeoutExpired:
            s.proc.kill()
            code = s.proc.wait()
            lingered = True
        self._close(s)
        if silent is not None:
            lines = [f"{silent}, so the helper killed it. The session is stopped; call hydra_start."]
        elif lingered:
            lines = [f"Hydra closed its live channel but kept running for {self.reply_margin_s:g} s,"
                     f" so the helper killed it (exit code {describe_exit(code)})."
                     " The session is stopped; call hydra_start."]
        elif code == 0 and s.mode == "window":
            # The helper did not send quit, so a clean exit of the window is
            # the user closing it, or a quit line a tool call passed on: a
            # plain stop, not a crash. The helper does not read the lines it
            # passes on, so the message does not say which.
            lines = ["Hydra's window closed and Hydra exited with code 0: the user closed it,"
                     " or a quit command was sent. The session is stopped; call hydra_start."]
        else:
            lines = [f"Hydra exited with code {describe_exit(code)}. The session is stopped;"
                     " call hydra_start."]
        tail = self._log_tail(s)
        if tail is None:
            lines.append(f"no log written: {s.log_path} does not exist")
        elif tail:
            lines.append(f"last lines of {s.log_path}:")
            lines += ["  " + t for t in tail]
        if s.crash_dump is not None and s.crash_dump.is_file():
            lines.append(f"crash dump: {s.crash_dump}")
        # The helper's scratch folder stays after a crash or a kill, so the log
        # can be read; the SessionStart cleanup sweep removes it later.
        lines.append(f"files kept in {s.scratch}")
        return "\n".join(lines)

    def _log_tail(self, s):
        """The log's last lines, or None when the file does not exist."""
        try:
            text = s.log_path.read_text(encoding="utf-8", errors="replace")
        except FileNotFoundError:
            return None
        except OSError:
            return []
        return text.splitlines()[-LOG_TAIL_LINES:]

    def _session(self):
        """The live session, starting a headless one if none was ever started."""
        if self.session is not None:
            return self.session, ""
        if self.ever_started:
            raise HydraGone(NO_SESSION)
        started = self.start("headless")
        if self.session is None:
            raise HydraGone(started)
        return self.session, f"({started})\n"

    def command(self, line):
        """Send one line exactly as given; return (ok, text, prefix). text is
        the reply's, untouched."""
        if "\n" in line or "\r" in line:
            # Transport: the pipe frames commands by line break, so a line
            # holding one would arrive as two commands with two replies.
            return False, "refused: one command is one line; this one holds a line break", ""
        s, prefix = self._session()
        # No command outlasts the timeout in force (CommandSession), so that
        # plus the margin is the longest any reply can take.
        wait_s = s.timeout + self.reply_margin_s
        try:
            s.pipe.write((line + "\n").encode("utf-8"))
        except (OSError, ValueError):
            raise HydraGone(self._kill_report(s, exited=True))
        reply = self._read_line(s, wait_s)
        if reply is None:
            raise HydraGone(self._kill_report(s, silent=f"Hydra did not answer within {wait_s:g} s"))
        try:
            msg = json.loads(reply)
            if not isinstance(msg, dict) or not isinstance(msg.get("text"), str):
                raise ValueError
            timeout = seconds_field(msg, "timeout")
            if timeout is None:
                raise ValueError
        except ValueError:
            return False, f"Hydra sent a line that is not a JSON reply: {reply[:200]!r}", prefix
        s.timeout = timeout
        return bool(msg.get("ok")), msg["text"], prefix

    def send(self, line):
        """One command as a tool reply: Hydra's text, capped."""
        with self.lock:
            try:
                ok, text, prefix = self.command(line)
            except HydraGone as e:
                return cap_reply(str(e))
            return cap_reply(prefix + text)

    def with_timeout(self, line, timeout):
        """Run `line` under a one-off timeout, then put back the one Hydra
        reported before it. Hydra judges the number."""
        with self.lock:
            if timeout is None:
                return self.send(line)
            try:
                s, prefix = self._session()
                old = s.timeout
                ok, text, _ = self.command(f"timeout {timeout!r}")
                if ok:
                    ok, text, _ = self.command(line)
                    if self.session is s:
                        # repr gives the shortest text that reads back as the
                        # same number, so the old value comes back exactly.
                        self.command(f"timeout {old!r}")
            except HydraGone as e:
                return cap_reply(str(e))
            return cap_reply(prefix + text)

    def run(self, script):
        with self.lock:
            # Every line goes to Hydra, blank and comment lines too: which
            # lines are no command is the live channel's call (command_line in
            # tests/ui/uitest_script.cpp), and it answers each of them. Hydra's
            # live replies carry no line number, so "stopped at line N" is the
            # only one, counting the script's own lines.
            lines = split_lines(script)
            out = []
            for i, line in enumerate(lines, 1):
                try:
                    ok, text, prefix = self.command(line)
                except HydraGone as e:
                    out += [f"> {line}", str(e)]
                    break
                if prefix:
                    out.append(prefix.rstrip("\n"))
                out += [f"> {line}", text.rstrip("\n")]
                if not ok:
                    out.append(f"stopped at line {i} of {len(lines)}")
                    break
            return cap_reply("\n".join(out) if out else "no lines to run")


def filtered(line, contains):
    """`line`, plus the bar and `contains` when one is given. What the bar
    means is Hydra's (split_bar in tests/ui/uitest_commands.cpp)."""
    return line if contains is None else f"{line} | {contains}"


# ---------------------------------------------------------------------------
# The tools Claude sees. Each returns plain text.

HELPER = Helper()


def build_server():
    from mcp.server.mcpserver import MCPServer

    server = MCPServer(
        "hydra",
        instructions=(
            "Drive Hydra's GUI by widget label. Start with hydra_start('headless') for the"
            " test corpus, or hydra_start('window') for the real window on a snapshot copy"
            " of the user's library. Read the screen with hydra_text, hydra_dump and"
            " hydra_state. To open a song, use hydra_open_song with its exact title,"
            " not a click on its row. Refs below the toolbar need **/ (the search box"
            " is **/##search)."
            " See docs/agents/ui-testing.md."),
    )

    @server.tool()
    def hydra_start(mode: str, library_dir: str | None = None, keep: bool = False) -> str:
        """Start a session; stops any session already running. mode is "headless"
        (hydra_uitest on the test corpus, no window) or "window" (the real Hydra
        window on a fresh snapshot copy of the library folder: library_dir, else
        HYDRA_LIBRARY_DIR, else DEFAULT_LIBRARY_DIR in tools/hydra_ui_mcp.py).
        keep=True keeps the helper's scratch folder, screenshots included,
        after hydra_stop."""
        return HELPER.start(mode, library_dir, keep)

    @server.tool()
    def hydra_stop(keep_window: bool = False) -> str:
        """End the session: send quit, wait for exit, kill if needed. In window
        mode, keep_window=True leaves the window open for the user to look at."""
        return HELPER.stop(keep_window)

    @server.tool()
    def hydra_reset() -> str:
        """A fresh library in the same session (window mode: back to the snapshot)."""
        return HELPER.send("reset")

    @server.tool()
    def hydra_open_song(title: str) -> str:
        """Open the song with exactly this title: the way to open a song. Hydra
        types the title into the library search, then opens the row whose
        title equals it; when none does, the reply says what the search shows."""
        return HELPER.send(f"open {title}")

    @server.tool()
    def hydra_click(ref: str) -> str:
        """Click the widget with this label (relative to the current window).
        Widgets below the toolbar, such as the library and the song panel, need
        **/ in front (**/Preview). A bare label that finds nothing is tried
        once as **/<label>, and the reply names the ref used; it is refused
        when several widgets carry the label. To open a song,
        use hydra_open_song."""
        return HELPER.send(f"click {ref}")

    @server.tool()
    def hydra_type(ref: str, text: str) -> str:
        """Type text into the input with this label, kept exactly. An empty
        text clears the field. Inputs below the toolbar need **/ in front: the
        library search is **/##search. A bare label that finds nothing is
        tried once as **/<label>, and the reply names the ref used; it is
        refused when several widgets carry the label."""
        return HELPER.send(f"type {ref} | {text}")

    @server.tool()
    def hydra_check(ref: str, on: bool) -> str:
        """Tick (on=True) or untick (on=False) the checkbox with this label."""
        return HELPER.send(f"{'check' if on else 'uncheck'} {ref}")

    @server.tool()
    def hydra_key(keys: str) -> str:
        """Press a key or chord, such as Escape, Enter, Ctrl+W or Down."""
        return HELPER.send(f"key {keys}")

    @server.tool()
    def hydra_window(ref: str) -> str:
        """Set the window that later labels are relative to, such as //Hydra."""
        return HELPER.send(f"window {ref}")

    @server.tool()
    def hydra_wait_idle(timeout: float | None = None) -> str:
        """Wait until no background job runs. timeout (seconds) applies to this call only."""
        return HELPER.with_timeout("wait-idle", timeout)

    @server.tool()
    def hydra_wait_text(text: str, timeout: float | None = None) -> str:
        """Wait until this text is on screen. timeout (seconds) applies to this call only."""
        return HELPER.with_timeout(f"wait-text {text}", timeout)

    @server.tool()
    def hydra_text(contains: str | None = None) -> str:
        """The screen as text; with contains, only the lines holding that substring."""
        return HELPER.send(filtered("text", contains))

    @server.tool()
    def hydra_state() -> str:
        """The app's state block: store, chart counts, search, shown rows."""
        return HELPER.send("state")

    @server.tool()
    def hydra_dump(window: str | None = None, contains: str | None = None) -> str:
        """The widget tree with rects, of the windows whose name holds window
        (every window when it is not given); with contains, only matching item
        lines plus their window line."""
        return HELPER.send(filtered("dump" if window is None else f"dump {window}", contains))

    @server.tool()
    def hydra_rect(ref: str) -> str:
        """One widget's line in the dump's shape."""
        return HELPER.send(f"rect {ref}")

    @server.tool()
    def hydra_screenshot(name: str) -> str:
        """Save a PNG of the screen under exactly this file name, so give it
        the .png yourself. The reply is "screenshot: <path>". A file in the
        helper's scratch folder goes when hydra_stop deletes that folder (see
        keep on hydra_start)."""
        return HELPER.send(f"screenshot {name}")

    @server.tool()
    def hydra_run(script: str) -> str:
        """Several command lines at once, one per line, stopping at the first
        failure, which names its line in the script."""
        return HELPER.run(script)

    return server


def main():
    atexit.register(HELPER.shutdown)
    build_server().run("stdio")


if __name__ == "__main__":
    main()
