"""Shared live-game pieces for the hit-window experiments.

watch_window.py, walk_edges.py and active_probe.py read the same engine fields,
load a probe song's manifest, and ask the same small questions while a song
plays. They share this file so the runners agree on the answers:

  * when the song clock has reached a target time (wait_until),
  * whether the song has stopped: clock frozen or jumped back (StoppedCheck),
  * which note to start at when a run joins mid-song (first_note_index),
  * how far from the note a hit landed (hit_offset_ms),
  * whether a stored window value changed between two reads (window_changed).

Every offset comes from constants.py. Finding the engine is
engine_finder.find_live_engine with engine_finder.all_patterns, which also
finds an engine in precision mode.
"""

from __future__ import annotations

import json
import os
import sys
import time
from typing import Callable, Optional, Protocol, Sequence

_REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "..")
)
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C
from tools.ch_probe import engine as _engine
# The snapshot lives in engine.py; these names stay here until the runners
# call engine.py directly.
from tools.ch_probe.engine import SNAPSHOT_SIZE, Snapshot  # noqa: F401

PROBE_ROOT = r"C:\Clone Hero\songs\Hydra Probe"


def decode_snapshot(raw: bytes) -> Snapshot:
    """engine.decode_snapshot, under the name the runners use."""
    return _engine.decode_snapshot(raw)


def read_snapshot(proc, engine: int) -> Snapshot:
    """One snapshot of the engine object at `engine`, through
    EngineModel.snapshot."""
    model = _engine.EngineModel(proc)
    model.use_object(engine)
    return model.snapshot()


def load_manifest(song_dir: str) -> dict:
    with open(os.path.join(song_dir, "manifest.json"), encoding="utf-8") as f:
        return json.load(f)


# --- Has the song stopped? ----------------------------------------------------

# The song clock not moving for this long means the song was quit or paused.
STALL_S = 5.0

# The song clock falling back by more than this means the song restarted (or
# the player seeked). The edge runners stop on it; play_chart re-syncs.
JUMP_BACK_S = 1.0


class ClockFrozen(RuntimeError):
    """The song clock has not moved for more than STALL_S seconds."""


class ClockJumpedBack(RuntimeError):
    """The song clock fell back by more than JUMP_BACK_S seconds."""


class StoppedCheck:
    """Watches successive raw song-clock reads and says when the song stopped.

    Make one per run and feed it every raw read. check() raises ClockJumpedBack
    or ClockFrozen, and otherwise says whether the clock moved since last time.
    """

    def __init__(self, raw_s: float, now_s: float) -> None:
        self.last_raw_s = raw_s
        self.last_move_s = now_s

    def check(self, raw_s: float, now_s: float) -> bool:
        """True when the clock moved since the last read, False when it held."""
        if raw_s < self.last_raw_s - JUMP_BACK_S:
            raise ClockJumpedBack(
                f"clock jumped back ({self.last_raw_s:.2f} -> {raw_s:.2f} s)")
        if raw_s != self.last_raw_s:
            self.last_raw_s, self.last_move_s = raw_s, now_s
            return True
        if now_s - self.last_move_s > STALL_S:
            raise ClockFrozen(
                f"clock frozen at {raw_s:.2f} s for over {STALL_S:.0f} s"
                " (song quit or paused)")
        return False


# --- Wait for a song time -----------------------------------------------------

class ReadsSongClock(Protocol):
    def read(self) -> tuple[float, float]:
        """(raw, estimate), both song seconds."""
        ...


# Sleep while the target is further ahead than this; inside it, spin. Every
# read is a chance to catch a frame change promptly, which is what keeps the
# clock estimate fresh.
_SPIN_WITHIN_S = 0.04
_WAKE_BEFORE_S = 0.03
_MAX_SLEEP_S = 0.5


def wait_until(clock: ReadsSongClock, target_ms: float, *, lead_ms: float = 0.0,
               stopped: Optional[StoppedCheck] = None,
               should_stop: Optional[Callable[[], bool]] = None,
               now: Callable[[], float] = time.perf_counter,
               sleep: Callable[[float], None] = time.sleep) -> tuple[float, float]:
    """Poll the song clock until its estimate reaches target_ms - lead_ms.

    Returns (raw, estimate) in song ms at that read. `stopped` carries the
    "has the song stopped" state across calls; pass the same one for a whole
    run so a jump back between two waits is still caught. Raises ClockFrozen
    or ClockJumpedBack when the song stops, and RuntimeError("stopped") when
    `should_stop` says so.
    """
    fire_s = C.ms_to_s(target_ms - lead_ms)
    while True:
        if should_stop is not None and should_stop():
            raise RuntimeError("stopped")
        raw, est = clock.read()
        t = now()
        if stopped is None:
            stopped = StoppedCheck(raw, t)
        else:
            stopped.check(raw, t)
        ahead = fire_s - est
        if ahead <= 0:
            return C.s_to_ms(raw), C.s_to_ms(est)
        if ahead > _SPIN_WITHIN_S:
            sleep(min(ahead - _WAKE_BEFORE_S, _MAX_SLEEP_S))


# --- Where a run starts, and where a hit landed -------------------------------

# A run that joins mid-song skips every note less than this far ahead of the
# clock, so the first press is never already late.
START_LEAD_MS = 150.0


def first_note_index(notes_ms: Sequence[float], clock_ms: float) -> int:
    """Index of the first note at least START_LEAD_MS ahead of clock_ms.

    `notes_ms` is in song order. Returns len(notes_ms) when none is.
    """
    i = 0
    while i < len(notes_ms) and notes_ms[i] < clock_ms + START_LEAD_MS:
        i += 1
    return i


def hit_offset_ms(note_ms: float, sent_ms: float, hit_time_before_s: float,
                  hit_time_after_s: float) -> tuple[float, bool]:
    """How far from the note a press landed, in ms (+ is late).

    The engine's stored hit time (+0x2e0) is the better number, so it wins when
    the press changed it. Otherwise it falls back to the estimated send time.
    Returns (offset_ms, True when the engine's hit time gave it).
    """
    if hit_time_after_s != hit_time_before_s:
        return C.s_to_ms(hit_time_after_s) - note_ms, True
    return sent_ms - note_ms, False


def window_changed(a_ms: float, b_ms: float) -> bool:
    """Whether two reads of a stored window (or hit time) differ for real."""
    return abs(a_ms - b_ms) > C.WINDOW_CHANGE_TOLERANCE_MS
