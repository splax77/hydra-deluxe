"""Watch whether the Clone Hero window ever comes to the front during a run.

A live run must leave the user's window in front the whole time (the plan
docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md, "Launch without
focus, and nothing ever shown"). ForegroundWatch reads which window is in
front every FOREGROUND_POLL_S on a daemon thread, from ch_start to ch_stop,
and counts each time the game's window becomes the front one. That count is
the pass line of every live step.

The window lookup is input_driver.find_game_window; this module only reads
which window is in front and never changes it.
"""

from __future__ import annotations

import ctypes
import threading
from typing import Callable, List, Optional, Tuple

# How often the front window is read, in seconds (the user's answer to the
# plan's question 4).
FOREGROUND_POLL_S = 0.1


def foreground_window() -> int:
    """The handle of the window in front, or 0 when there is none."""
    return ctypes.windll.user32.GetForegroundWindow() or 0


def count_line(count: int) -> str:
    """How ch_status and ch_stop say the count."""
    return f"foreground went to the game {count} time{'' if count == 1 else 's'}"


class ForegroundWatch:
    """Counts the times the game's window comes to the front.

    `find_window` returns the game's window handle (0 while it has none),
    `foreground` the handle of the window in front, `now` the time an event
    is stamped with, and `sleep` waits between two polls. `events` holds
    (time, handle) for every change of the front window, the game's or not,
    so a failed step can say when the game came forward and when it left.
    `count` counts only the changes that put the game in front."""

    def __init__(self, find_window: Callable[[], int], foreground: Callable[[], int],
                 now: Callable[[], float], sleep: Callable[[float], None],
                 poll_s: float = FOREGROUND_POLL_S):
        self._find_window = find_window
        self._foreground = foreground
        self._now = now
        self._sleep = sleep
        self.poll_s = poll_s
        self._lock = threading.Lock()
        self._stopped = threading.Event()
        self._thread: Optional[threading.Thread] = None
        self._last: Optional[int] = None
        self._game_in_front = False
        self.count = 0
        self.events: List[Tuple[float, int]] = []
        # The time of each change that put the game in front (one per count),
        # and the time start() ran; line() prints the first minus the second.
        self.game_times: List[float] = []
        self._started_at = 0.0
        # Set when a poll raised; the watch stops there and the line says so.
        self.error: Optional[str] = None

    def start(self) -> None:
        """Start polling on a daemon thread. It ends at stop() or when the
        process exits."""
        self._started_at = self._now()
        self._thread = threading.Thread(target=self._loop, name="ch_foreground_watch",
                                        daemon=True)
        self._thread.start()

    def stop(self) -> None:
        """Stop counting. A poll already under way when this is called
        records nothing, so `count` and `events` are final once it returns.
        Calling it again does nothing."""
        with self._lock:
            self._stopped.set()

    def running(self) -> bool:
        return (self._thread is not None and self._thread.is_alive()
                and not self._stopped.is_set())

    def poll(self) -> None:
        """Read the front window once and record a change. The game counts
        when it is in front now and was not at the last poll, so a window
        found one poll after it came forward still counts once."""
        handle = self._foreground() or 0
        game = self._find_window() or 0
        in_front = bool(game) and handle == game
        with self._lock:
            if self._stopped.is_set():
                return
            came = in_front and not self._game_in_front
            if handle != self._last or came:
                at = self._now()
                if handle != self._last:
                    self._last = handle
                    self.events.append((at, handle))
                if came:
                    self.count += 1
                    self.game_times.append(at)
            self._game_in_front = in_front

    def line(self) -> str:
        """count_line for this watch, with "so far" while it runs and the
        error that stopped it, if one did. When the game came forward, a
        second line lists each time, in seconds since start()."""
        with self._lock:
            count, times = self.count, list(self.game_times)
        text = count_line(count)
        if self.error is not None:
            text = f"{text} before the watch failed: {self.error}"
        elif self.running():
            text += " so far"
        if times:
            at = ", ".join(f"{t - self._started_at:.1f} s" for t in times)
            text += f"\nforeground went to the game at (since the watch started): {at}"
        return text

    def _loop(self) -> None:
        while not self._stopped.is_set():
            try:
                self.poll()
            except Exception as e:  # reported by line()
                self.error = f"{type(e).__name__}: {e}"
                return
            self._sleep(self.poll_s)
