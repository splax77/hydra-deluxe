"""The offline stall harness: the real player loop with no game, watched for
a step that runs long (PS4 of
docs/superpowers/plans/2026-11-12-player-stall-fixes.md).

It runs the real play_chart.play_chords on a real schedule file. The song
clock is the wall clock. Each press goes through the checkout's real
LiveAgent and kit_tools.kit_press, to a fake Frida script that answers on a
thread of its own, as Frida does, so the kit log's writing is in the loop.
This loop case (`--timing loop`, the default) is the control: play_chords is
the keys route's code (play_chart.live_play), and no kit run uses it, since a
kit run is timed by the game (play_chart.live_in_game, D342). Each row goes to one of three sinks: `writer` (play_chart.PressLogWriter),
`per-row` (open, append one row and close, on the player's thread) or
`none`. The song is played over and over until the run's cap.

A watchdog thread watches which step the player's thread is in. When one
runs past the threshold it writes every thread's Python stack with
faulthandler to `stall-<n>.txt`, with the step's name and how long it ran.
For the run's first stall only it then attaches `cdb -pv` to this process
and adds what cdb printed, because each attach pauses the process. The step
names are the stems of overlay_logs.TIMING_PRESS_COLUMNS, plus `loop`
(play_chords' own code between two steps) and `score_delay` (the sleep
before the score read after a press, which no column times).

`--timing game` plays each pass with play_chart.play_in_game instead, the
player a kit run uses (D339). Its fake
Frida script is then a fake game: stall_game.mjs, beside this file, a Node
process that runs the kit agent's real frame scheduler on a wall-clock frame
ticker with its own song clock, takes the real `play`, `retime` and
`stop_play` messages and sends the real event batches. The forced block then
lands on play_in_game's thread, the one that takes the agent's events.
play_once runs one pass of either timing and reports what the tests check.

The threshold, the run's length and the hunt's run count default to the
user's numbers in tools/ch_probe/constants.py. `--load` starts a second
process that reads this checkout's testdata and .git folders in a loop, and
stops it on every way out. Everything the run writes goes in its own folder
(`--out`, else a new one under the system's temp folder).

Run, from the checkout's root (with -I, `-m` cannot find `tools`, so the
file is named by its path):
     py -I tools/ch_probe/experiments/stall_harness.py <schedule.json>
     --sink per-row|writer|none [--timing loop|game] [--load]
     [--force-stall-ms MS] [--minutes M] [--threshold-ms MS] [--no-cdb]
     [--hunt-run N] [--out DIR]
"""

from __future__ import annotations

import argparse
import faulthandler
import io
import json
import os
import pathlib
import queue
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from contextlib import contextmanager
from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Sequence, Tuple

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp import kit_tools  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.experiments import live, play_chart  # noqa: E402

SINK_WRITER = "writer"
SINK_PER_ROW = "per-row"
SINK_NONE = "none"
SINKS = (SINK_WRITER, SINK_PER_ROW, SINK_NONE)

# Which player a pass runs: play_chords' outside loop, or play_in_game with
# the fake game.
TIMING_LOOP = "loop"
TIMING_GAME = "game"
TIMINGS = (TIMING_LOOP, TIMING_GAME)

# The fake game's frame rate: the 330 frames a second the in-game timing
# plan gives the game ("about 3 ms at 330 frames a second", its question 2),
# which its task IG4 gives this fake. The plan is
# docs/superpowers/plans/2026-11-12-in-game-press-timing.md (removed; git show b971b235:docs/superpowers/plans/2026-11-12-in-game-press-timing.md).
FAKE_GAME_FPS = 330
_HERE = os.path.dirname(os.path.abspath(__file__))
GAME_DRIVER = os.path.join(_HERE, "stall_game.mjs")
SCHEDULER = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent", "press_schedule.ts")
# The fake game's one message of its own, which kit.ts lacks: how many times
# each seq was written (stall_game.mjs answers it).
FAKE_STATS_TYPE = "fake_stats"
# How long the fake game may take to exit once its stdin closes.
FAKE_GAME_EXIT_CAP_S = 5.0

# The median kit press call of LS-A2 (row P10 of the plan above, and question
# 2's own text): how long the fake Frida script takes to answer a post.
KIT_PRESS_CALL_MEDIAN_MS = 7.6
FAKE_KIT_REPLY_S = C.ms_to_s(KIT_PRESS_CALL_MEDIAN_MS)

# The step names, from the columns that time them.
STEP_WAIT_READ = "wait_read"
STEP_WAIT_OVERSLEEP = "wait_oversleep"
STEP_PRESS = "press"
STEP_REPLY_CHECK = "reply_check"
STEP_END_READ = "end_read"
STEP_SCORE_READ = "score_read"
STEP_HANDOFF = "handoff"
STEP_SCORE_DELAY = "score_delay"
STEP_LOOP = "loop"

# What cdb runs once attached: every thread's stack, then a detach. The
# command is the one docs/agents/this-machine.md ("Debugger: WinDbg and cdb")
# shows working.
CDB_COMMAND = "~*kc 40; qd"

# The PowerShell lookup docs/agents/this-machine.md gives for cdb's folder.
_CDB_LOOKUP = "(Get-AppxPackage -Name Microsoft.WinDbg).InstallLocation"


# --- which step the player is in ----------------------------------------------

class StepTracker:
    """The player thread's current step, for the watchdog. Each step that
    starts bumps `seq`, so the watchdog dumps one step once. A step may be
    allowed time of its own (a sleep's asked length): only time past that
    counts toward a stall."""

    def __init__(self, now: Callable[[], float] = time.perf_counter) -> None:
        self._now = now
        self._lock = threading.Lock()
        self.seq = 0
        self.step: Optional[str] = None
        self.began = 0.0
        self.allow_s = 0.0
        self.thread_ident: Optional[int] = None
        self.last_step: Optional[str] = None

    def enter(self, name: str, allow_s: float = 0.0) -> None:
        with self._lock:
            self.seq += 1
            self.step, self.began, self.allow_s = name, self._now(), allow_s
            self.thread_ident = threading.get_ident()

    def leave(self, then: Optional[str] = STEP_LOOP) -> None:
        """End the current step; the player is in `then` from now (None
        when it left the loop)."""
        with self._lock:
            self.last_step = self.step
            self.seq += 1
            self.step, self.began, self.allow_s = then, self._now(), 0.0

    def current(self) -> Tuple[int, Optional[str], float, float, Optional[int]]:
        with self._lock:
            return self.seq, self.step, self.began, self.allow_s, self.thread_ident

    @contextmanager
    def step_of(self, name: str, allow_s: float = 0.0):
        self.enter(name, allow_s)
        try:
            yield
        finally:
            self.leave()


# --- the watchdog -------------------------------------------------------------

def find_cdb(timeout_s: float) -> Tuple[Optional[str], str]:
    """cdb.exe's path from the lookup in docs/agents/this-machine.md, or
    None and why not."""
    try:
        done = subprocess.run(["powershell", "-NoProfile", "-Command", _CDB_LOOKUP],
                              capture_output=True, text=True, timeout=timeout_s)
    except (OSError, subprocess.SubprocessError) as e:
        return None, f"the WinDbg lookup failed: {e}"
    folder = done.stdout.strip()
    if not folder:
        return None, "Microsoft.WinDbg is not installed (Get-AppxPackage found no package)"
    path = os.path.join(folder, "amd64", "cdb.exe")
    if not os.path.isfile(path):
        return None, f"the WinDbg package holds no {path}"
    return path, ""


def launch_cdb(cdb_path: str, pid: int, timeout_s: float) -> str:
    """Attach cdb to `pid` without stopping it for good, print every
    thread's stack and detach; return what it printed."""
    try:
        done = subprocess.run([cdb_path, "-pv", "-p", str(pid), "-c", CDB_COMMAND],
                              capture_output=True, text=True, timeout=timeout_s)
    except subprocess.TimeoutExpired as e:
        return f"cdb did not finish within {timeout_s:g} s: {e}"
    except OSError as e:
        return f"cdb could not start: {e}"
    return done.stdout + (f"\n[stderr]\n{done.stderr}" if done.stderr else "")


class Watchdog:
    """Watches a StepTracker from a thread of its own and writes
    `stall-<n>.txt` in `folder` for each step that runs past `threshold_ms`.
    `cdb` is (path, why-not): with a path, the first stall also gets one
    cdb attach; with None, the stall file says why there is none. `deadline`
    is the run's end on `now`, which caps a cdb attach."""

    def __init__(self, tracker: StepTracker, folder: str, threshold_ms: float, *,
                 cdb: Tuple[Optional[str], str], deadline: float,
                 launch: Callable[[str, int, float], str] = launch_cdb,
                 now: Callable[[], float] = time.perf_counter) -> None:
        self.tracker, self.folder, self.threshold_ms = tracker, folder, threshold_ms
        self.cdb, self.deadline, self.launch, self.now = cdb, deadline, launch, now
        self.stalls: List[Tuple[str, str, float]] = []
        self._cdb_used = False
        self._dumped_seq = -1
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._body, name="stall watchdog",
                                        daemon=True)

    def start(self) -> None:
        self._thread.start()

    def stop(self) -> None:
        self._stop.set()
        self._thread.join()

    def _body(self) -> None:
        # Four looks per threshold, so a stall is seen before it runs a
        # quarter past the threshold.
        poll_s = C.ms_to_s(self.threshold_ms) / 4
        while not self._stop.wait(poll_s):
            self.check()

    def check(self) -> None:
        seq, step, began, allow_s, ident = self.tracker.current()
        if step is None or seq == self._dumped_seq:
            return
        ran_s = self.now() - began
        if C.s_to_ms(ran_s - allow_s) <= self.threshold_ms:
            return
        self._dumped_seq = seq
        self._dump(seq, step, ran_s, allow_s, ident)

    def _dump(self, seq: int, step: str, ran_s: float, allow_s: float,
              ident: Optional[int]) -> None:
        path = os.path.join(self.folder, f"stall-{len(self.stalls) + 1}.txt")
        names = {t.ident: t.name for t in threading.enumerate()}
        with open(path, "w", encoding="utf-8") as f:
            f.write(f"step: {step}\n")
            f.write(f"running for: {C.s_to_ms(ran_s):.1f} ms when seen")
            if allow_s:
                f.write(f" (asked to sleep {C.s_to_ms(allow_s):.1f} ms)")
            f.write(f"; threshold {self.threshold_ms:g} ms\n")
            f.write(f"player thread: Thread {ident or 0:#018x} "
                    f"({names.get(ident, 'unknown')})\n")
            f.write("threads: " + "; ".join(f"{i:#018x} {n}" for i, n in names.items())
                    + "\n\n")
            f.flush()
            faulthandler.dump_traceback(file=f, all_threads=True)
            f.write("\n")
            f.write(self._cdb_text(seq) + "\n")
        self.stalls.append((path, step, C.s_to_ms(ran_s)))

    def _cdb_text(self, seq: int) -> str:
        path, why_not = self.cdb
        if path is None:
            return f"cdb: none: {why_not}"
        if self._cdb_used:
            return "cdb: not attached again; one attach per run, at its first stall"
        self._cdb_used = True
        began = self.now()
        text = self.launch(path, os.getpid(), max(self.deadline - began, 0.0))
        took_ms = C.s_to_ms(self.now() - began)
        still_on = self.tracker.current()[0] == seq
        on_line = ("the stall was still on when cdb finished, so cdb saw it"
                   if still_on else
                   "the stall had ended by the time cdb finished; cdb's stacks "
                   "may not show it")
        return (f"cdb: the attach took {took_ms:.0f} ms; {on_line}\n"
                f"--- cdb output ---\n{text}")


# --- the fake Frida -----------------------------------------------------------

class _FakeScript:
    """Answers each post on a thread of its own after `reply_s`, through the
    message handler, the way Frida calls LiveAgent._on_message."""

    def __init__(self, reply_s: float) -> None:
        self.reply_s = reply_s
        self.handlers: dict = {}
        self._posts: queue.Queue = queue.Queue()
        self._frame = 0
        self._thread = threading.Thread(target=self._body, name="fake Frida", daemon=True)

    def on(self, kind: str, handler: Callable) -> None:
        self.handlers[kind] = handler

    def load(self) -> None:
        self._thread.start()

    def post(self, message: dict) -> None:
        self._posts.put(message)

    def stop(self) -> None:
        self._posts.put(None)

    def _reply(self, message: dict) -> dict:
        if message.get("type") != "press":
            return {"id": message["id"], "found": True}
        groups = []
        for _group in [message["elements"], *[g["elements"] for g in message.get("then", [])]]:
            self._frame += 1
            groups.append({"down_frame": self._frame,
                           "up_frame": self._frame + message["hold_frames"],
                           "down_ms": 0.0, "up_ms": 0.0})
        reply = {"id": message["id"], **groups[0]}
        if len(groups) > 1:
            reply["groups"] = groups
        return reply

    def _body(self) -> None:
        while True:
            message = self._posts.get()
            if message is None:
                return
            time.sleep(self.reply_s)
            self.handlers["message"]({"type": "send", "payload": self._reply(message)}, None)


class _GameScript:
    """The fake game as a Frida script: stall_game.mjs in a Node process.
    A post goes to its stdin as one JSON line; each line it prints is a
    payload, handed to the message handler on a thread of its own, the way
    Frida calls LiveAgent._on_message. That thread only reads and hands
    over, so the Node process never waits on a slow Python thread, as a game
    never waits on Frida's queue. Node's errors go to `err_path`."""

    def __init__(self, command: Sequence[str], err_path: str) -> None:
        self.command, self.err_path = list(command), err_path
        self.handlers: dict = {}
        self._proc: Optional[subprocess.Popen] = None
        self._err: Any = None
        self._post_lock = threading.Lock()
        self._stopping = False
        self._thread = threading.Thread(target=self._body, name="fake game reader",
                                        daemon=True)

    def on(self, kind: str, handler: Callable) -> None:
        self.handlers[kind] = handler

    def load(self) -> None:
        self._err = open(self.err_path, "w", encoding="utf-8")
        self._proc = subprocess.Popen(self.command, stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE, stderr=self._err,
                                      text=True, encoding="utf-8", bufsize=1)
        self._thread.start()

    def post(self, message: dict) -> None:
        with self._post_lock:
            self._proc.stdin.write(json.dumps(message) + "\n")
            self._proc.stdin.flush()

    def stop(self) -> None:
        if self._proc is None:
            return
        self._stopping = True
        with self._post_lock:
            try:
                self._proc.stdin.close()
            except OSError:
                pass
        try:
            self._proc.wait(FAKE_GAME_EXIT_CAP_S)
        except subprocess.TimeoutExpired:
            self._proc.kill()
            self._proc.wait()
        self._thread.join(FAKE_GAME_EXIT_CAP_S)
        self._err.close()

    def _body(self) -> None:
        for line in self._proc.stdout:
            if line.strip():
                self.handlers["message"]({"type": "send", "payload": json.loads(line)}, None)
        code = self._proc.wait()
        if not self._stopping:
            # A request waiting on a game that died fails at once, naming it.
            self.handlers["message"]({"type": "error", "description":
                                      f"the fake game exited with code {code}; its "
                                      f"errors are in {self.err_path}"}, None)


class _FakeSession:
    def __init__(self, make_script: Callable[[], Any]) -> None:
        self.make_script = make_script
        self.script: Any = None

    def on(self, kind: str, handler: Callable) -> None:
        pass

    def create_script(self, source: str) -> Any:
        self.script = self.make_script()
        return self.script

    def detach(self) -> None:
        if self.script is not None:
            self.script.stop()


class FakeFrida:
    """The frida module LiveAgent is given: attach returns a session whose
    script answers every post."""

    class ProcessNotFoundError(Exception):
        pass

    def __init__(self, reply_s: float = FAKE_KIT_REPLY_S) -> None:
        self.reply_s = reply_s

    def attach(self, target: Any) -> _FakeSession:
        return _FakeSession(lambda: _FakeScript(self.reply_s))


def node_path() -> Optional[str]:
    return shutil.which("node")


def game_command(clock_start_ms: float, node: Optional[str] = None) -> List[str]:
    """The fake game's command line: Node on stall_game.mjs, with the real
    scheduler's file URL, FAKE_GAME_FPS and the song clock's start."""
    node = node or node_path()
    if node is None:
        raise RuntimeError("node is not on PATH, so the fake game cannot run")
    return [node, GAME_DRIVER, pathlib.Path(SCHEDULER).as_uri(), str(FAKE_GAME_FPS),
            repr(float(clock_start_ms))]


class FakeGameFrida(FakeFrida):
    """The frida module for a game-timed pass: attach returns a session
    whose script is the fake game, its song clock begun at
    `clock_start_ms` when the `play` message arrives."""

    def __init__(self, clock_start_ms: float, err_path: str) -> None:
        super().__init__()
        self.clock_start_ms, self.err_path = clock_start_ms, err_path

    def attach(self, target: Any) -> _FakeSession:
        command = game_command(self.clock_start_ms)
        return _FakeSession(lambda: _GameScript(command, self.err_path))


# --- the sinks ----------------------------------------------------------------

class _BlockOnce:
    """--force-stall-ms: the first data row written through
    play_chart._press_log_writer blocks this long, on whichever thread
    writes it."""

    def __init__(self, block_s: float) -> None:
        self.block_s = block_s
        self.done = False
        self._real = play_chart._press_log_writer

    def make(self, f):
        writer = self._real(f)
        outer = self

        class Blocking:
            def writeheader(self):
                return writer.writeheader()

            def writerows(self, rows):
                return writer.writerows(rows)

            def writerow(self, row):
                if not outer.done:
                    outer.done = True
                    time.sleep(outer.block_s)
                return writer.writerow(row)

        return Blocking()


@contextmanager
def _forced_block(block_ms: Optional[float]):
    if block_ms is None:
        yield
        return
    block = _BlockOnce(C.ms_to_s(block_ms))
    play_chart._press_log_writer = block.make
    try:
        yield
    finally:
        play_chart._press_log_writer = block._real


def per_row_append(path: str, row: dict) -> None:
    """The `per-row` sink: open the press log, append one row, close it, on
    the calling thread."""
    with open(path, "a", encoding="utf-8", newline="") as f:
        play_chart._press_log_writer(f).writerow(row)


class _Sink:
    """One pass's press log, through the chosen sink."""

    def __init__(self, kind: str, path: str) -> None:
        self.kind, self.path = kind, path
        self.writer: Optional[play_chart.PressLogWriter] = None
        self.rows: List[dict] = []

    def start(self) -> None:
        if self.kind == SINK_WRITER:
            self.writer = play_chart.PressLogWriter(self.path)
            self.writer.start()
        elif self.kind == SINK_PER_ROW:
            play_chart.write_press_log(self.path, [])

    def add(self, row: dict) -> None:
        self.rows.append(row)
        if self.writer is not None:
            self.writer.add(row)
        elif self.kind == SINK_PER_ROW:
            per_row_append(self.path, row)

    def end(self) -> str:
        if self.kind == SINK_NONE:
            return f"no press log ({len(self.rows)} rows kept in memory)"
        if self.writer is not None:
            cap_s = C.LOG_WRITER_CAP_S
            if not self.writer.stop(cap_s):
                return (f"press log {self.path} may lack its last rows: its writer did "
                        f"not finish within {cap_s:g} s")
            if self.writer.error is not None:
                play_chart.write_press_log(self.path, self.rows)
                return (f"press log {self.path} written whole at the end ({len(self.rows)} "
                        f"rows); the writer failed: {self.writer.error}")
        return f"press log {self.path} ({len(self.rows)} rows)"


# --- the load -----------------------------------------------------------------

def load_folders(repo_root: str = _REPO_ROOT) -> List[str]:
    """The folders the load reads: this checkout's testdata and its .git.
    In a worktree `.git` is a file naming the worktree's git folder, whose
    `commondir` names the shared one, which is the folder read."""
    git = os.path.join(repo_root, ".git")
    if os.path.isfile(git):
        with open(git, encoding="utf-8") as f:
            gitdir = f.read().strip().removeprefix("gitdir:").strip()
        gitdir = os.path.normpath(os.path.join(repo_root, gitdir))
        common = os.path.join(gitdir, "commondir")
        if os.path.isfile(common):
            with open(common, encoding="utf-8") as f:
                gitdir = os.path.normpath(os.path.join(gitdir, f.read().strip()))
        git = gitdir
    return [os.path.join(repo_root, "testdata"), git]


def load_command(folders: Sequence[str]) -> List[str]:
    return [sys.executable, "-I", os.path.abspath(__file__), "--read-loop", *folders]


def read_loop(folders: Sequence[str]) -> None:
    """The load process: read every file under `folders`, over and over,
    until it is killed."""
    while True:
        for folder in folders:
            for root, _dirs, files in os.walk(folder):
                for name in files:
                    try:
                        with open(os.path.join(root, name), "rb") as f:
                            while f.read(io.DEFAULT_BUFFER_SIZE):
                                pass
                    except OSError:
                        pass


# --- one run ------------------------------------------------------------------

class _Clock:
    """The song clock: wall time from the pass's start, begun at `start_s`."""

    def __init__(self, start_s: float, now: Callable[[], float]) -> None:
        self.start_s, self.now, self.t0 = start_s, now, now()

    def read(self) -> Tuple[float, float]:
        raw = self.start_s + self.now() - self.t0
        return raw, raw


def _first_press_s(chords) -> float:
    first = play_chart.press_plan(chords)[0].at_ms
    # Twice the skip window, so play_chords never skips the first press.
    return C.ms_to_s(first - 2 * live.START_LEAD_MS)


def _play_pass(n: int, chords, tracker: StepTracker, sink: _Sink, folder: str,
               cap_s: float, reply_s: float, out: Callable[[str], None]):
    now = time.perf_counter
    agent_path = os.path.join(folder, "fake-agent.js")
    if not os.path.exists(agent_path):
        with open(agent_path, "w", encoding="utf-8") as f:
            f.write("// stall_harness: the fake Frida script answers every post\n")
    agent = FA.LiveAgent(agent_path, log_path=os.path.join(folder, f"kit-{n}.jsonl"),
                         frida_mod=FakeFrida(reply_s), out=out)
    agent.start(kit_tools.READ_CAP_S)
    kit = kit_tools.kit_press(agent)
    real_gap = kit_tools.reply_gap

    def press(lanes, axes=None, then=()):
        with tracker.step_of(STEP_PRESS):
            return kit(lanes, axes, then=then)

    def reply_gap(reply, groups):
        with tracker.step_of(STEP_REPLY_CHECK):
            return real_gap(reply, groups)

    def read_score() -> int:
        with tracker.step_of(STEP_SCORE_READ):
            return 0

    def read_sp_end():
        with tracker.step_of(STEP_END_READ):
            return None

    def sleep(seconds: float) -> None:
        name = (STEP_WAIT_OVERSLEEP if tracker.last_step == STEP_WAIT_READ
                else STEP_SCORE_DELAY)
        with tracker.step_of(name, allow_s=seconds):
            time.sleep(seconds)

    def on_press(row: dict) -> None:
        with tracker.step_of(STEP_HANDOFF):
            sink.add(row)

    clock = _Clock(_first_press_s(chords), now)

    class TimedClock:
        def read(self):
            with tracker.step_of(STEP_WAIT_READ):
                return clock.read()

    sink.start()
    printed = io.StringIO()
    # play_chords imports reply_gap from kit_tools when it starts.
    kit_tools.reply_gap = reply_gap
    try:
        result = play_chart.play_chords(
            chords, TimedClock(), read_score, press, now=now, sleep=sleep,
            on_press=on_press, out=printed, cap_s=cap_s, read_sp_end=read_sp_end, kit=True)
    finally:
        kit_tools.reply_gap = real_gap
        tracker.leave(then=None)
        log_line = sink.end()
        kit_line = agent.detach()
        with open(os.path.join(folder, f"play-{n}.txt"), "w", encoding="utf-8") as f:
            f.write(printed.getvalue())
    return result, log_line, kit_line


@dataclass
class GameReport:
    """What a game-timed pass saw beyond its RunResult. `reported` counts
    the agent's reports of each seq as play_in_game took them (a sent,
    dropped or skipped event each), and `fires` the fake game's writes of
    each seq. `posts` holds each message play_in_game posted, as (type,
    seconds since the pass began). `end_reads` holds when each end read
    returned, on the same clock. `latest_take_ms` is the most any sent
    event waited, from its press's down to play_in_game taking it, both in
    epoch time: a forced block shows there."""
    reported: Dict[int, int] = field(default_factory=dict)
    fires: Dict[int, int] = field(default_factory=dict)
    posts: List[Tuple[str, float]] = field(default_factory=list)
    end_reads: List[float] = field(default_factory=list)
    latest_take_ms: float = 0.0


class _TappedPlayer:
    """A kit_tools.GamePlayer as play_in_game sees it, which also counts
    each report it hands over and notes each post."""

    def __init__(self, player: kit_tools.GamePlayer, report: GameReport,
                 since: Callable[[], float]) -> None:
        self._player, self._report, self._since = player, report, since
        self.events = self

    def get(self, timeout: float) -> dict:
        event = self._player.events.get(timeout=timeout)
        kind = event.get("kind") if isinstance(event, dict) else None
        seqs = (event.get("seqs") or [] if kind == kit_tools.EVENT_SKIPPED
                else [event.get("seq")] if kind in (kit_tools.EVENT_SENT,
                                                    kit_tools.EVENT_DROPPED) else [])
        for seq in seqs:
            self._report.reported[seq] = self._report.reported.get(seq, 0) + 1
        if kind == kit_tools.EVENT_SENT and event.get("down_ms") is not None:
            waited_ms = C.s_to_ms(time.time()) - event["down_ms"]
            self._report.latest_take_ms = max(self._report.latest_take_ms, waited_ms)
        return event

    def post(self, message: dict) -> dict:
        self._report.posts.append((message["type"], self._since()))
        return self._player.post(message)

    def alive(self) -> bool:
        return self._player.alive()


def planned_ends(chords) -> List[float]:
    """The Star Power end each end read of `chords` gets in a game-timed
    pass, in the order the reads come: the end the schedule planned
    (Press.planned_end_ms), as if the game stored just that."""
    return [p.planned_end_ms for p in play_chart.press_plan(chords)
            if p.planned_end_ms is not None]


def _game_pass(n: int, chords, tracker: StepTracker, sink: _Sink, folder: str,
               cap_s: float, out: Callable[[str], None],
               end_read_block_s: Optional[float] = None):
    """One pass of play_chart.play_in_game on the fake game. With
    `end_read_block_s`, the first end read blocks that long before it
    returns, on play_in_game's thread, so its retime goes out that much
    later."""
    now = time.perf_counter
    began = now()
    report = GameReport()
    agent_path = os.path.join(folder, "fake-agent.js")
    if not os.path.exists(agent_path):
        with open(agent_path, "w", encoding="utf-8") as f:
            f.write("// stall_harness: the fake Frida script answers every post\n")
    clock_start_ms = C.s_to_ms(_first_press_s(chords))
    agent = FA.LiveAgent(agent_path, log_path=os.path.join(folder, f"kit-{n}.jsonl"),
                         frida_mod=FakeGameFrida(clock_start_ms,
                                                 os.path.join(folder, f"game-{n}.err")),
                         out=out)
    agent.start(kit_tools.READ_CAP_S)
    player = kit_tools.GamePlayer(agent)
    ends = iter(planned_ends(chords))
    blocks = [end_read_block_s] if end_read_block_s is not None else []

    def read_sp_end() -> Optional[float]:
        with tracker.step_of(STEP_END_READ):
            if blocks:
                time.sleep(blocks.pop())
            end_ms = next(ends, None)
            report.end_reads.append(now() - began)
            return None if end_ms is None else C.ms_to_s(end_ms)

    def on_press(row: dict) -> None:
        with tracker.step_of(STEP_HANDOFF):
            sink.add(row)

    sink.start()
    printed = io.StringIO()
    try:
        result = play_chart.play_in_game(
            chords, _TappedPlayer(player, report, lambda: now() - began),
            clock_addr=1, score_addr=1, now=now, on_press=on_press, out=printed,
            cap_s=cap_s, read_sp_end=read_sp_end)
        stats = agent.request({"type": FAKE_STATS_TYPE}, kit_tools.READ_CAP_S)
        report.fires = {int(seq): count for seq, count in stats["fires"].items()}
    finally:
        tracker.leave(then=None)
        player.close()
        log_line = sink.end()
        kit_line = agent.detach()
        with open(os.path.join(folder, f"play-{n}.txt"), "w", encoding="utf-8") as f:
            f.write(printed.getvalue())
    return result, log_line, kit_line, report


@dataclass
class PassReport:
    """One pass of play_once: its RunResult and its press log's line, the
    retime_missed rows, and for a game-timed pass its GameReport."""
    result: play_chart.RunResult
    log_line: str
    retime_missed: int
    game: Optional[GameReport] = None


def play_once(chords, *, timing: str, sink: str, folder: str, cap_s: float,
              force_stall_ms: Optional[float] = None,
              end_read_block_ms: Optional[float] = None,
              out: Callable[[str], None] = print,
              reply_s: float = FAKE_KIT_REPLY_S) -> PassReport:
    """Play `chords` once with `timing`, through `sink`, with the forced
    block of --force-stall-ms on the first press-log row and, for a
    game-timed pass, `end_read_block_ms` on the first end read."""
    os.makedirs(folder, exist_ok=True)
    tracker = StepTracker()
    press_log = _Sink(sink, os.path.join(folder, "press-1.csv"))
    with _forced_block(force_stall_ms):
        if timing == TIMING_GAME:
            block_s = None if end_read_block_ms is None else C.ms_to_s(end_read_block_ms)
            result, log_line, _kit, game = _game_pass(1, chords, tracker, press_log, folder,
                                                      cap_s, out, block_s)
        else:
            if end_read_block_ms is not None:
                raise ValueError("an end read block needs the game timing, whose end reads "
                                 "post a retime")
            result, log_line, _kit = _play_pass(1, chords, tracker, press_log, folder,
                                                cap_s, reply_s, out)
            game = None
    missed = sum(1 for row in result.rows
                 if row.get("agent_note") == kit_tools.NOTE_RETIME_MISSED)
    return PassReport(result, log_line, missed, game)


def run_harness(chords, *, sink: str, folder: str, threshold_ms: float, minutes: float,
                force_stall_ms: Optional[float] = None, load: bool = False,
                cdb: bool = True, hunt_run: Optional[int] = None,
                out: Callable[[str], None] = print,
                popen: Callable[..., Any] = subprocess.Popen,
                find: Callable[[float], Tuple[Optional[str], str]] = find_cdb,
                launch: Callable[[str, int, float], str] = launch_cdb,
                reply_s: float = FAKE_KIT_REPLY_S, timing: str = TIMING_LOOP) -> int:
    """Play `chords` over and over for `minutes`, through `sink`, with
    `timing`'s player, and say how the run went. Returns the exit code: 0
    when the run reached its cap, 1 when a pass ended any other way."""
    now = time.perf_counter
    cap_s = minutes * 60
    began = now()
    deadline = began + cap_s
    os.makedirs(folder, exist_ok=True)
    out(f"harness folder: {folder}")
    if hunt_run is not None:
        out(f"hunt run {hunt_run} of at most {C.STALL_HUNT_RUNS_MAX}")
    cdb_found = (find(cap_s) if cdb else (None, "skipped (--no-cdb)"))
    tracker = StepTracker(now)
    dog = Watchdog(tracker, folder, threshold_ms, cdb=cdb_found, deadline=deadline,
                   launch=launch, now=now)
    loader = None
    code = 0
    dropped = 0
    try:
        if load:
            folders = load_folders()
            loader = popen(load_command(folders))
            out("load: reading " + " and ".join(folders))
        dog.start()
        with _forced_block(force_stall_ms):
            n = 0
            while True:
                left_s = deadline - now()
                if left_s <= 0:
                    out("cap reached")
                    break
                n += 1
                press_log = _Sink(sink, os.path.join(folder, f"press-{n}.csv"))
                missed = ""
                if timing == TIMING_GAME:
                    result, log_line, kit_line, _game = _game_pass(
                        n, chords, tracker, press_log, folder, left_s, out)
                    missed = (f", {kit_tools.NOTE_RETIME_MISSED} "
                              + str(sum(1 for row in result.rows if row.get("agent_note")
                                        == kit_tools.NOTE_RETIME_MISSED)))
                else:
                    result, log_line, kit_line = _play_pass(
                        n, chords, tracker, press_log, folder, left_s, reply_s, out)
                dropped += result.dropped
                out(f"pass {n}: {result.stop}, sent {result.sent}, dropped {result.dropped}"
                    f"{missed}; {log_line}; kit log: {kit_line}")
                if result.stop == "cap":
                    out("cap reached")
                    break
                if result.stop != play_chart.STOP_DONE:
                    out(f"pass {n} ended early: {result.message}")
                    code = 1
                    break
    finally:
        dog.stop()
        if loader is not None:
            loader.kill()
            loader.wait()
            out("load: stopped")
    out(f"dropped {dropped} chords in all")
    if not dog.stalls:
        out(f"no stall over {threshold_ms:g} ms in {minutes:g} minutes")
    else:
        out(f"{len(dog.stalls)} stalls over {threshold_ms:g} ms in {minutes:g} minutes: "
            + "; ".join(f"{os.path.basename(p)} in {step} ({ms:.0f} ms when seen)"
                        for p, step, ms in dog.stalls))
    return code


def main(argv: Optional[Sequence[str]] = None, *,
         out: Callable[[str], None] = print, **hooks) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if argv[:1] == ["--read-loop"]:
        read_loop(argv[1:])
        return 0
    ap = argparse.ArgumentParser(description="Run the real player loop with no game and "
                                             "dump the stacks when a step runs long.")
    ap.add_argument("schedule", help="a hydra_replay schedule JSON")
    ap.add_argument("--sink", required=True, choices=SINKS)
    ap.add_argument("--timing", choices=TIMINGS, default=TIMING_LOOP)
    ap.add_argument("--minutes", type=float, default=C.STALL_RUN_MINUTES)
    ap.add_argument("--threshold-ms", type=float, default=C.STALL_THRESHOLD_MS)
    ap.add_argument("--force-stall-ms", type=float)
    ap.add_argument("--load", action="store_true")
    ap.add_argument("--no-cdb", action="store_true")
    ap.add_argument("--hunt-run", type=int)
    ap.add_argument("--out")
    args = ap.parse_args(argv)
    if args.minutes <= 0 or args.threshold_ms <= 0:
        ap.error("--minutes and --threshold-ms must be more than 0")
    if args.force_stall_ms is not None and args.sink == SINK_NONE:
        ap.error("--force-stall-ms blocks a sink's write, so it needs --sink writer or per-row")
    if args.hunt_run is not None and not 1 <= args.hunt_run <= C.STALL_HUNT_RUNS_MAX:
        ap.error(f"--hunt-run counts from 1 to {C.STALL_HUNT_RUNS_MAX}, the hunt's "
                 f"most runs (D335 item 3)")
    chords = play_chart.load_schedule(args.schedule).chords
    folder = args.out or tempfile.mkdtemp(prefix="stall-harness-")
    return run_harness(chords, sink=args.sink, folder=folder, threshold_ms=args.threshold_ms,
                       minutes=args.minutes, force_stall_ms=args.force_stall_ms,
                       load=args.load, cdb=not args.no_cdb, hunt_run=args.hunt_run,
                       out=out, timing=args.timing, **hooks)


if __name__ == "__main__":
    sys.exit(main())
