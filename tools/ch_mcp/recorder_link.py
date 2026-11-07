"""The worker's end of the machine recorder (tools/ch_mcp/machine_recorder.py):
start it, tell it the game's pid and the engine's address, read how far it has
got, and end it. The run timing plan's design point 2 says why it works this
way; the recorder itself owns its sample rate and its own cap.

A recorder never harms a run, so nothing here waits except stop(), and a write
that fails comes back as a line for the reply, not as an exception. The tools
call this class holding the server's lock, so it takes no lock of its own.
"""

from __future__ import annotations

import os
import subprocess
import sys
import threading
from typing import Callable, List, Optional

from tools.ch_mcp import machine_recorder

# How long stop() waits after closing the recorder's stdin, and again after
# terminating it (the run timing plan's question 18).
RECORDER_EXIT_CAP_S = 2

RECORDER_SCRIPT = os.path.abspath(machine_recorder.__file__)
# Each recording's folder under the session folder, numbered from 1.
FOLDER_PREFIX = "machine-"
# Where the recorder's stdout and stderr go, in its folder.
LOG_NAME = "recorder.log"

_LINE_PREFIX = "machine recorder: "


def start_failed_line(error: Exception) -> str:
    """What a reply says about a recorder that did not start."""
    return (f"{_LINE_PREFIX}not started ({type(error).__name__}: {error}); "
            "the run goes on with no recording")


def _close_pipe(pipe) -> None:
    try:
        pipe.close()
    except (OSError, ValueError):
        pass


def samples_text(path: str) -> str:
    """How far the samples file at `path` has got, with no wait. The piece after
    the last newline is dropped, because the recorder may be writing it now."""
    try:
        with open(path, "rb") as f:
            rows = f.read().split(b"\n")[:-1]
    except FileNotFoundError:
        return f"no samples yet in {path}"
    except OSError as e:
        return f"could not read {path}: {e}"
    if len(rows) < 2:
        return f"no samples yet in {path}"
    header, last = (row.decode("ascii", "replace").strip().split(",") for row in (rows[0], rows[-1]))
    try:
        wall = last[header.index(machine_recorder.WALL_COLUMN)]
    except (ValueError, IndexError):
        return (f"{len(rows) - 1} samples so far, but no {machine_recorder.WALL_COLUMN} "
                f"in the last row of {path}")
    return f"{len(rows) - 1} samples so far, last at {wall}"


class RecorderLink:
    """One recorder at a time, for the life of the worker. `popen` is the seam
    the tests swap for a fake."""

    def __init__(self, popen: Callable[..., subprocess.Popen] = subprocess.Popen):
        self._popen = popen
        self._proc: Optional[subprocess.Popen] = None
        self._folder: Optional[str] = None
        self._count = 0
        # What became of the last recorder, once no process is held.
        self._said: Optional[str] = None

    def start(self, session_dir: str, worker_pid: int, cap_s: float) -> List[str]:
        """Start a recorder on a new folder under `session_dir`, not waiting
        for it, with `cap_s` as the run cap it adds its own margin to. One
        still running is stopped first. Returns the lines for the reply, and
        raises what the start raised."""
        lines: List[str] = []
        stopped = self.stop()
        if stopped is not None:
            lines.append(stopped)
        self._said = None
        # The recorder opens its files create-only, so the folder must be new.
        while True:
            self._count += 1
            folder = os.path.join(session_dir, f"{FOLDER_PREFIX}{self._count}")
            if not os.path.exists(folder):
                break
        # sys.executable leaves no py launcher between the worker and the
        # recorder, so --parent is the recorder's real parent. The pipe and the
        # log keep it off the worker's own stdin and stdout, for the reason
        # bot_run._popen_game gives.
        command = [sys.executable, "-I", RECORDER_SCRIPT, "--out", folder,
                   "--parent", str(worker_pid), "--run-cap-s", str(cap_s)]
        try:
            os.makedirs(folder)
            with open(os.path.join(folder, LOG_NAME), "wb") as log:
                self._proc = self._popen(command, stdin=subprocess.PIPE, stdout=log,
                                         stderr=subprocess.STDOUT,
                                         creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        except Exception as e:
            self._said = start_failed_line(e)
            raise
        self._folder = folder
        lines.append(f"{_LINE_PREFIX}started, pid {self._proc.pid}, folder {folder}")
        return lines

    def send(self, line: str) -> Optional[str]:
        """Write `line` to the recorder's stdin. Returns None, or the line for
        the reply when the write failed. With no recorder there is nothing to
        tell, and start's own line has said why."""
        if self._proc is None:
            return None
        try:
            self._proc.stdin.write(f"{line}\n".encode("ascii"))
            self._proc.stdin.flush()
        except (OSError, ValueError) as e:
            return f'{_LINE_PREFIX}"{line}" not sent ({type(e).__name__}: {e}); the run goes on'
        return None

    def running(self) -> bool:
        return self._proc is not None and self._proc.poll() is None

    def stop(self) -> Optional[str]:
        """End the recorder: close its stdin on a thread, because a close waits
        behind a write a hung reader never takes, wait RECORDER_EXIT_CAP_S,
        then terminate it. Returns the line for the reply, or None when there
        was no recorder to stop."""
        proc, self._proc = self._proc, None
        if proc is None:
            return None
        exited = proc.poll() is not None
        said = self._exited_line(proc) if exited else None
        threading.Thread(target=_close_pipe, args=(proc.stdin,), daemon=True).start()
        self._said = said if exited else self._wait_out(proc)
        return self._said

    def line(self) -> str:
        """The recorder's state, for ch_status and open_lines."""
        if self._proc is None:
            return self._said or f"{_LINE_PREFIX}none"
        if self.running():
            return f"{_LINE_PREFIX}running, pid {self._proc.pid}"
        return self._exited_line(self._proc)

    def samples_line(self) -> str:
        """How far the recording has got, read from its file with no wait.
        With no recorder held, what became of the last one, or that none
        was ever started, as on a game the server did not start."""
        if self._proc is None:
            return self._said or f"{_LINE_PREFIX}none; this run is not recorded"
        text = samples_text(os.path.join(self._folder, machine_recorder.SAMPLES_FILE))
        return f"{_LINE_PREFIX}{text}" if self.running() else f"{self.line()}; {text}"

    def _exited_line(self, proc: subprocess.Popen) -> str:
        return (f"{_LINE_PREFIX}exited by itself, pid {proc.pid}, exit code {proc.returncode}, "
                f"folder {self._folder}, log {os.path.join(self._folder, LOG_NAME)}")

    def _wait_out(self, proc: subprocess.Popen) -> str:
        """Wait for `proc` to end after its stdin closed, terminating it at the cap."""
        try:
            proc.wait(RECORDER_EXIT_CAP_S)
            return (f"{_LINE_PREFIX}stopped, pid {proc.pid}, exit code {proc.returncode}, "
                    f"folder {self._folder}")
        except subprocess.TimeoutExpired:
            pass
        late = (f"{_LINE_PREFIX}pid {proc.pid} did not exit within RECORDER_EXIT_CAP_S "
                f"({RECORDER_EXIT_CAP_S} s) of its stdin closing")
        try:
            proc.terminate()
            proc.wait(RECORDER_EXIT_CAP_S)
        except subprocess.TimeoutExpired:
            return f"{late}; terminated, but it still runs {RECORDER_EXIT_CAP_S} s later"
        except OSError as e:
            return f"{late}; terminating it failed ({type(e).__name__}: {e})"
        return f"{late}; terminated, exit code {proc.returncode}, folder {self._folder}"
