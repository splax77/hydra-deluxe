"""The meaning layer: turn raw addresses into named facts about the engine.

Everything above this module talks about "the total window" or "precision
mode", never about byte offsets or RVAs. This module is the only place in the
probe that knows an engine field lives at object_ptr + 0x20. It gets its
addresses from constants.py and its plumbing (memory reads, breakpoints) from a
ProcessHandle and a Debugger passed in.

Why the dependency injection: this module never imports debugger.py, nor
process.py's Process class or its Win32 backend. It only depends on the
ProcessHandle and Debugger Protocols in interfaces.py -- the shapes, not the
concrete classes. That is what lets the unit tests drive it with fake objects
and no running game. The one thing it takes from process.py is the pure byte
decoders, so a snapshot block decodes exactly as a single-field read does.
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from typing import TYPE_CHECKING, Optional

try:
    # Normal case: imported as part of the ch_probe package.
    from . import constants as C
    from .process import decode_double, decode_f32, decode_u32, decode_u64
except ImportError:  # pragma: no cover - exercised only by direct-script runs
    # Fallback: the module's own directory is on sys.path and it is imported as
    # a top-level module (how the unit tests load it without a package root).
    import constants as C  # type: ignore[no-redef]
    from process import (  # type: ignore[no-redef]
        decode_double, decode_f32, decode_u32, decode_u64)

if TYPE_CHECKING:
    # Type hints only. These are Protocols (duck-typed shapes), not concrete
    # classes, so importing them creates no link to process.py or debugger.py.
    from .interfaces import Debugger, ProcessHandle, ThreadContext


def is_precision(flags: int) -> bool:
    """True when the flags dword has the PrecisionMode bit set. A clear bit
    means normal mode; no other bit matters."""
    return (flags & C.PRECISION_MODE_BIT) != 0


def pressed_input_hit(score_before: int, score_after: int) -> bool:
    """Whether a press hit its note: the score rises only on a hit, proven
    by play_chart."""
    return score_after > score_before


# One read covers every watched field, so a sample is a consistent snapshot.
# The double at +0x2e0 is the furthest field out.
SNAPSHOT_SIZE = C.OFF_HIT_TIME + 8

# One read covers the clock object's fields, so the three belong together. The
# raw clock is the furthest out.
CLOCK_BLOCK_SIZE = C.CLOCK_OFF_RAW + 8


@dataclass(frozen=True)
class Snapshot:
    """The watched engine fields, all from one read, and the clock object's.

    The Star Power fields come after the first five. Their offsets are not yet
    confirmed live (see constants.py, which also cites the evidence for each
    field's width). Some are read at two widths so the live log can confirm
    that evidence. Their names match the columns in
    experiments/overlay_logs.py.

    The last three come from the clock object, not the engine, and are NaN
    when the walk to it found nothing (EngineModel.clock_block). That walk is
    not yet confirmed live, so nothing may depend on them."""

    window_ms: float
    clock_s: float
    score: int
    hit_time_s: float
    flags: int
    sp_amount_u32: int
    sp_amount_u64: int
    sp_max_u32: int
    sp_max_u64: int
    sp_active_byte: int
    sp_active_u32: int
    sp_mult_byte: int
    sp_mult_u32: int
    sp_end_s: float
    sp_overflow_u32: int
    audio_s: float = math.nan
    raw_s: float = math.nan
    corr_s: float = math.nan

    @property
    def precision(self) -> bool:
        return is_precision(self.flags)


def decode_clock_block(raw: Optional[bytes]) -> dict:
    """The Snapshot's clock fields from CLOCK_BLOCK_SIZE bytes of the clock
    object. With no bytes they keep their NaN defaults."""
    if raw is None:
        return {}
    return {
        "audio_s": decode_double(raw[C.CLOCK_OFF_AUDIO:C.CLOCK_OFF_AUDIO + 8]),
        "raw_s": decode_double(raw[C.CLOCK_OFF_RAW:C.CLOCK_OFF_RAW + 8]),
        "corr_s": decode_f32(raw[C.CLOCK_OFF_CORR:C.CLOCK_OFF_CORR + 4]),
    }


def decode_snapshot(raw: bytes, clock_raw: Optional[bytes] = None) -> Snapshot:
    """Turn SNAPSHOT_SIZE bytes read from the engine object into a Snapshot,
    each field at its offset from constants.py. `clock_raw` is the clock
    object's block, when the caller has one."""
    def dbl(off: int) -> float:
        return decode_double(raw[off:off + 8])

    def u32(off: int) -> int:
        return decode_u32(raw[off:off + 4])

    def u64(off: int) -> int:
        return decode_u64(raw[off:off + 8])

    def byte(off: int) -> int:
        return raw[off]

    return Snapshot(
        window_ms=C.s_to_ms(dbl(C.OFF_TOTAL_WINDOW)),
        clock_s=dbl(C.OFF_SONG_CLOCK),
        score=u32(C.OFF_SCORE),
        hit_time_s=dbl(C.OFF_HIT_TIME),
        flags=u32(C.OFF_FLAGS),
        sp_amount_u32=u32(C.OFF_SP_AMOUNT),
        sp_amount_u64=u64(C.OFF_SP_AMOUNT),
        sp_max_u32=u32(C.OFF_SP_MAX),
        sp_max_u64=u64(C.OFF_SP_MAX),
        sp_active_byte=byte(C.OFF_SP_ACTIVE),
        sp_active_u32=u32(C.OFF_SP_ACTIVE),
        sp_mult_byte=byte(C.OFF_SP_MULTIPLIER),
        sp_mult_u32=u32(C.OFF_SP_MULTIPLIER),
        sp_end_s=dbl(C.OFF_SP_END_TIME),
        sp_overflow_u32=u32(C.OFF_SP_OVERFLOW),
        **decode_clock_block(clock_raw),
    )


class EngineModel:
    """Reads named facts off the live DrumsEngine object.

    Build it with a ProcessHandle (memory reads) and a Debugger (breakpoints).
    Call capture_object() once to learn where the engine object is, then use the
    reader methods.
    """

    def __init__(self, process: "ProcessHandle",
                 debugger: Optional["Debugger"] = None) -> None:
        self._process = process
        # Only capture_object() needs a debugger. A runner that finds the
        # engine by memory scan (engine_finder) passes none.
        self._debugger = debugger
        # Filled in by capture_object(). None until then.
        self.object_ptr: Optional[int] = None

    # --- capture -------------------------------------------------------------

    def capture_object(self) -> int:
        """Learn the live engine object's address by catching its constructor.

        On entry to the DrumsEngine constructor the object pointer sits in rcx
        (x64 fastcall / IL2CPP calling convention). So: put a breakpoint on the
        constructor, and when it fires read rcx and remember it.

        LIVE-ONLY SEAM: with a real debugger, setting the breakpoint only arms
        it; the callback fires later when the game constructs the engine, which
        is why we then pump run() until the pointer shows up. The fake debugger
        in the tests fires the callback the moment the breakpoint is set, so the
        pointer is already captured and run() is never called.
        """
        if self._debugger is None:
            raise RuntimeError(
                "capture_object needs a debugger; use use_object() with a "
                "pointer from engine_finder instead")
        addr = self._process.resolve(C.RVA_DRUMS_ENGINE_CTOR)

        def _on_ctor(debugger: "Debugger", ctx: "ThreadContext") -> None:
            self.object_ptr = ctx.rcx

        self._debugger.set_breakpoint(addr, _on_ctor)

        if self.object_ptr is None:
            # Real path: wait for the game to actually build the engine.
            self._debugger.run(until=lambda: self.object_ptr is not None)

        if self.object_ptr is None:
            raise RuntimeError(
                "constructor breakpoint never fired; engine object not captured"
            )
        return self.object_ptr

    def use_object(self, object_ptr: int) -> int:
        """Adopt an engine object found another way: the memory scan in
        engine_finder, the route play_chart.py proved live. Returns it."""
        self.object_ptr = object_ptr
        return object_ptr

    # --- per-object reads ----------------------------------------------------

    def _addr(self, offset: int) -> int:
        """Absolute address of a field, given its byte offset off the object."""
        if self.object_ptr is None:
            raise RuntimeError(
                "no engine object captured yet; call capture_object() first"
            )
        return self.object_ptr + offset

    def total_window(self) -> float:
        """The total search window (seconds). This is the field the passive probe
        watches: set to back*2 at construction, then overwritten per note by the
        formula's caller."""
        return self._process.read_double(self._addr(C.OFF_TOTAL_WINDOW))

    def back_window(self) -> float:
        """Back-window constant (seconds): the max per-side, ~0.085 in normal mode."""
        return self._process.read_double(self._addr(C.OFF_BACK_WINDOW))

    def front_window(self) -> float:
        """Front-window constant (seconds): the min per-side, ~0.0375 in normal mode."""
        return self._process.read_double(self._addr(C.OFF_FRONT_WINDOW))

    def hit_time(self) -> float:
        """The double at +0x2e0, in seconds. It is not a hit time, whatever this
        method's name says: docs/clone-hero/03-timing.md, TM-14, says what the
        engine writes there and when."""
        return self._process.read_double(self._addr(C.OFF_HIT_TIME))

    def score(self) -> int:
        """The game score. It rises only when a note is hit, which is how
        play_chart.py tells a hit from a miss."""
        return self._process.read_u32(self._addr(C.OFF_SCORE))

    def dyn_threshold(self) -> int:
        """The dynamics threshold a hit's velocity is compared with
        (docs/clone-hero/06-scoring.md, SC-13)."""
        return self._process.read_u32(self._addr(C.OFF_DYN_THRESHOLD))

    def note_count(self) -> int:
        """Note count the processing loop uses."""
        return self._process.read_u32(self._addr(C.OFF_NOTE_COUNT))

    def precision_mode(self) -> bool:
        """True when the PrecisionMode flag bit is set. The flags field is a
        dword; is_precision tests the bit."""
        return is_precision(self._process.read_u32(self._addr(C.OFF_FLAGS)))

    def snapshot(self) -> Snapshot:
        """Every watched engine field from one read of the object, so the
        values belong to the same frame, and the clock object's fields from
        clock_block. decode_snapshot does the decoding. A failed read of the
        engine raises OSError; the clock's can only leave its fields NaN."""
        raw = self._process.read(self._addr(0), SNAPSHOT_SIZE)
        return decode_snapshot(raw, self.clock_block(raw))

    def clock_block(self, engine_raw: bytes) -> Optional[bytes]:
        """The clock object's block, walked from the engine block already
        read: listener, then its GameManager, then the clock. Walked on every
        sample, so a clock the game replaces is never read stale.

        None when a pointer on the way is null or a read fails. The watch ends
        on an OSError, and losing these three columns must not end it. The
        walk's hops are docs/clone-hero/03-timing.md, TM-47."""
        off = C.OFF_ENGINE_LISTENER
        player = decode_u64(engine_raw[off:off + 8])
        try:
            manager = player and self._process.read_u64(player + C.PLAYER_OFF_GM)
            clock = manager and self._process.read_u64(manager + C.GM_OFF_CLOCK)
            if not clock:
                return None
            return self._process.read(clock, CLOCK_BLOCK_SIZE)
        except OSError:
            return None

    def song_clock(self) -> float:
        """Current song time in seconds, from +0x100. Proven live on
        2026-09-25: play_chart.py plays whole songs off this field. The game
        writes it once per frame, so a read can be a few ms stale."""
        return self._process.read_double(self._addr(C.OFF_SONG_CLOCK))

    # --- global constants ----------------------------------------------------

    def constants(self) -> dict:
        """Read every .rdata window/formula constant into a flat dict of floats.

        These live at fixed spots in the game's read-only data, so they don't
        need a captured object -- just the module base, which the ProcessHandle
        already knows. Keys are readable names; values are the decoded doubles.
        """
        read = self._process.read_const_double
        out = {
            # Per-side window constants, both modes.
            C.CONST_KEY_NORMAL_BACK: read(C.RVA_CONST_NORMAL_BACK),
            C.CONST_KEY_NORMAL_FRONT: read(C.RVA_CONST_NORMAL_FRONT),
            C.CONST_KEY_PRECISION_BACK: read(C.RVA_CONST_PRECISION_BACK),
            C.CONST_KEY_PRECISION_FRONT: read(C.RVA_CONST_PRECISION_FRONT),
            # Shared divisor and exponent.
            C.CONST_KEY_DIVISOR: read(C.RVA_FORMULA_DIVISOR),
            C.CONST_KEY_EXPONENT: read(C.RVA_FORMULA_EXPONENT),
            # Threshold used in the hit-check comparison.
            C.CONST_KEY_HITCHECK_THRESHOLD: read(C.RVA_HITCHECK_THRESHOLD),
        }
        # Normal-branch formula constants, prefixed so the names stay clear.
        for name, rva in C.RVA_FORMULA_NORMAL.items():
            out[C.CONST_KEY_PREFIX_NORMAL + name] = read(rva)
        # Precision-branch formula constants.
        for name, rva in C.RVA_FORMULA_PRECISION.items():
            out[C.CONST_KEY_PREFIX_PRECISION + name] = read(rva)
        return out
