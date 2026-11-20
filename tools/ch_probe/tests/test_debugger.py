"""Unit tests for the pure logic in debugger.py.

What we can test with no game running: the breakpoint byte bookkeeping (does
arming write 0xCC and remember the real byte, does disarming put it back), run
on this process's own memory through the real process.py bindings; the
XMM0-to-double decode, the rip-minus-one fix-up, the read masking that hides
0xCC, and the ThreadContext register snapshot, taken from a real suspended
thread of this process.

What we cannot test here: the Win32 event pump (WaitForDebugEvent,
GetThreadContext). It needs a live debuggee and is marked LIVE-ONLY in the
module. attach's setup steps do run here, on a fake Win32. We still confirm the
Debugger class exposes the right method names so the interface contract is met.
"""

from __future__ import annotations

import ctypes
import os
import struct
import threading
import unittest
from unittest import mock

from tools.ch_probe import debugger
from tools.ch_probe.debugger import (
    INT3,
    BreakpointTable,
    Debugger,
    ThreadContext,
    adjust_rip_after_int3,
    decode_xmm0_double,
    mask_breakpoints,
)


class FakeMemory:
    """A stand-in for process memory: a dict of address -> byte.

    Gives the BreakpointTable the same read/write shape the live debugger does,
    so the whole save/restore/re-arm state machine runs with no OS calls.
    """

    def __init__(self, initial: dict[int, int] | None = None) -> None:
        self.cells: dict[int, int] = dict(initial or {})

    def read(self, addr: int, size: int) -> bytes:
        return bytes(self.cells.get(addr + i, 0) for i in range(size))

    def write(self, addr: int, data: bytes) -> None:
        for i, b in enumerate(data):
            self.cells[addr + i] = b


def _own_process_debugger(test: unittest.TestCase) -> Debugger:
    """A Debugger wired to this test's own process the way attach() wires it
    to Clone Hero, minus DebugActiveProcess: a real OpenProcess handle, the
    real process.make_reader/make_writer bindings and the real cache flush.
    Nothing is attached to and nothing is left running; the handle is closed
    when the test ends."""
    dbg = Debugger()
    dbg._win32 = debugger._Win32()
    k32 = dbg._win32.k32
    handle = k32.OpenProcess(debugger.PROCESS_ALL_ACCESS, False, os.getpid())
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    test.addCleanup(k32.CloseHandle, handle)
    dbg._proc_handle = handle
    dbg._read_raw = debugger.process.make_reader(handle)
    dbg._write_raw = debugger.process.make_writer(handle)
    return dbg


_COPY_SIZE = 16  # bytes of real code copied; the tests below use offsets 0-7


def _code_copy(test: unittest.TestCase, dbg: Debugger) -> int:
    """A writable buffer holding a copy of real machine code: the first bytes
    of kernel32's GetTickCount as loaded in this process, read through the
    debugger's own reader. Breakpoints go on the copy, never on code that
    runs. Returns the copy's address."""
    func = ctypes.cast(ctypes.windll.kernel32.GetTickCount, ctypes.c_void_p).value
    code = dbg._raw_read(func, _COPY_SIZE)
    buf = (ctypes.c_ubyte * _COPY_SIZE).from_buffer_copy(code)
    test.addCleanup(lambda: buf)  # keeps the buffer alive for the whole test
    return ctypes.addressof(buf)


class BreakpointTableTests(unittest.TestCase):
    """The table the live debugger builds, writing real process memory
    through process.py's bindings. Each check compares memory with what it
    held before the table touched it."""

    def setUp(self):
        self.dbg = _own_process_debugger(self)
        self.table = self.dbg._table
        self.base = _code_copy(self, self.dbg)
        self.before = ctypes.string_at(self.base, _COPY_SIZE)
        # A copied 0xCC would make every "now holds 0xCC" check below pass
        # without the table writing anything.
        self.assertNotIn(INT3, self.before[:8])

    def memory(self) -> bytes:
        return ctypes.string_at(self.base, _COPY_SIZE)

    def test_arm_writes_int3_and_saves_original(self):
        addr = self.base
        self.table.add(addr)
        self.table.arm(addr)

        self.assertEqual(self.memory()[0], INT3)                     # 0xCC is installed
        self.assertEqual(self.table.original(addr), self.before[0])  # real byte saved
        self.assertTrue(self.table.is_armed(addr))

    def test_disarm_restores_original_byte(self):
        addr = self.base + 1
        self.table.add(addr)
        self.table.arm(addr)
        self.table.disarm(addr)

        self.assertEqual(self.memory(), self.before)  # back to the real byte
        self.assertFalse(self.table.is_armed(addr))

    def test_rearm_after_disarm_saves_no_stale_byte(self):
        # The re-arm must reinstall 0xCC without re-reading (which would now read
        # a stale 0xCC if it read at the wrong moment). Original stays the truth.
        addr = self.base + 2
        self.table.add(addr)
        self.table.arm(addr)
        self.table.disarm(addr)
        self.table.arm(addr)   # re-arm, as the single-step handler does

        self.assertEqual(self.memory()[2], INT3)
        self.assertEqual(self.table.original(addr), self.before[2])

    def test_arm_is_idempotent_and_preserves_original(self):
        addr = self.base + 3
        self.table.add(addr)
        self.table.arm(addr)
        self.table.arm(addr)  # second arm must not overwrite the saved byte with 0xCC

        self.assertEqual(self.table.original(addr), self.before[3])
        self.assertEqual(self.memory()[3], INT3)

    def test_remove_restores_and_forgets(self):
        addr = self.base + 4
        self.table.add(addr)
        self.table.arm(addr)
        self.table.remove(addr)

        self.assertEqual(self.memory(), self.before)  # restored
        self.assertFalse(self.table.has(addr))        # forgotten

    def test_armed_originals_lists_only_armed(self):
        armed, unarmed = self.base + 5, self.base + 6
        self.table.add(armed)
        self.table.add(unarmed)
        self.table.arm(armed)  # arm one, leave the other unarmed

        self.assertEqual(self.table.armed_originals(), {armed: self.before[5]})

    def test_disarm_of_unarmed_is_a_safe_no_op(self):
        # Disarming something that was never armed must not touch memory or throw
        # -- the pump can call disarm defensively.
        addr = self.base + 7
        self.table.add(addr)
        self.table.disarm(addr)  # never armed
        self.assertEqual(self.memory(), self.before)
        self.assertFalse(self.table.is_armed(addr))


class MaskBreakpointsTests(unittest.TestCase):
    def test_masks_installed_cc_back_to_original(self):
        # A read of four bytes where the second byte holds our 0xCC.
        data = bytes([0x48, INT3, 0x89, 0xE5])
        originals = {0x1001: 0x8B}  # the real byte at the patched address
        out = mask_breakpoints(0x1000, data, originals)

        self.assertEqual(out, bytes([0x48, 0x8B, 0x89, 0xE5]))

    def test_ignores_breakpoints_outside_the_range(self):
        data = bytes([0x01, 0x02, 0x03])
        originals = {0x9999: 0xFF}  # far outside the read window
        out = mask_breakpoints(0x1000, data, originals)

        self.assertEqual(out, data)

    def test_no_breakpoints_returns_input(self):
        data = bytes([0xDE, 0xAD])
        self.assertEqual(mask_breakpoints(0x0, data, {}), data)


class Xmm0DecodeTests(unittest.TestCase):
    def test_decodes_known_double_from_low_eight_bytes(self):
        # Pack 89.5 (the "is there a clamp?" number) into the low 8 bytes and
        # fill the high 8 with junk that must be ignored.
        low = struct.pack("<d", 89.5)
        high = b"\xAA" * 8
        self.assertEqual(decode_xmm0_double(low + high), 89.5)

    def test_decodes_the_normal_back_window(self):
        buf = struct.pack("<d", 85.0) + b"\x00" * 8
        self.assertEqual(decode_xmm0_double(buf), 85.0)

    def test_short_buffer_raises(self):
        with self.assertRaises(ValueError):
            decode_xmm0_double(b"\x00\x00\x00")


class AdjustRipTests(unittest.TestCase):
    def test_rip_minus_one_points_at_the_breakpoint(self):
        # The CPU reports rip one past the 0xCC it just ran.
        self.assertEqual(adjust_rip_after_int3(0x1_0000_0041), 0x1_0000_0040)


class ThreadContextTests(unittest.TestCase):
    def test_registers_are_plain_int_attributes(self):
        # A real CONTEXT, read the way _on_our_breakpoint reads one: from a
        # suspended thread of this process. The thread is resumed and joined
        # before the checks run.
        win32 = debugger._Win32()
        k32 = win32.k32
        k32.SuspendThread.argtypes = [ctypes.wintypes.HANDLE]
        k32.SuspendThread.restype = ctypes.wintypes.DWORD
        k32.ResumeThread.argtypes = [ctypes.wintypes.HANDLE]
        k32.ResumeThread.restype = ctypes.wintypes.DWORD
        release = threading.Event()
        thread = threading.Thread(target=release.wait, daemon=True)
        thread.start()
        try:
            handle = k32.OpenThread(debugger.THREAD_ALL_ACCESS, False,
                                    thread.native_id)
            self.assertTrue(handle, ctypes.get_last_error())
            try:
                self.assertNotEqual(k32.SuspendThread(handle), 0xFFFFFFFF)
                try:
                    raw = debugger.CONTEXT()
                    raw.ContextFlags = debugger.CONTEXT_ALL
                    self.assertTrue(k32.GetThreadContext(handle, ctypes.byref(raw)),
                                    ctypes.get_last_error())
                finally:
                    k32.ResumeThread(handle)
            finally:
                k32.CloseHandle(handle)
        finally:
            release.set()
            thread.join(timeout=5)
        self.assertFalse(thread.is_alive())

        ctx = Debugger._context_from_ctx(raw)
        self.assertNotEqual(ctx.rip, 0)  # a suspended thread has a real rip
        for name in ThreadContext._GENERAL:
            # Each register is the CONTEXT field of the same name, as a plain int.
            self.assertIs(type(getattr(ctx, name)), int, name)
            self.assertEqual(getattr(ctx, name), getattr(raw, name.capitalize()), name)

    def test_xmm0_double_reads_the_formula_return(self):
        xmm0 = struct.pack("<d", 37.5) + b"\x00" * 8
        ctx = ThreadContext({"rip": 0}, xmm0)
        self.assertEqual(ctx.xmm0_double(), 37.5)


class DebuggerSurfaceTests(unittest.TestCase):
    """The Win32 pump is LIVE-ONLY, but the class must still expose the method
    names the interface contract names, so construction and attribute presence
    are worth a cheap check."""

    def test_debugger_constructs_without_attaching(self):
        dbg = Debugger()
        for name in ("attach", "set_breakpoint", "clear_breakpoint",
                     "read", "write", "run", "stop"):
            self.assertTrue(callable(getattr(dbg, name)), name)
        self.assertFalse(hasattr(dbg, "set_hw_data_breakpoint"))

    def test_module_exports_the_expected_names(self):
        for name in ("Debugger", "ThreadContext", "BreakpointTable",
                     "decode_xmm0_double", "mask_breakpoints",
                     "adjust_rip_after_int3", "INT3"):
            self.assertTrue(hasattr(debugger, name), name)


def _exception_event(code, *, tid, addr=0):
    """A DEBUG_EVENT carrying one exception, built by hand (no debuggee)."""
    ev = debugger.DEBUG_EVENT()
    ev.dwDebugEventCode = debugger.EXCEPTION_DEBUG_EVENT
    ev.dwThreadId = tid
    rec = ev.u.Exception.ExceptionRecord
    rec.ExceptionCode = code
    rec.ExceptionAddress = addr
    return ev


class KillOnExitTests(unittest.TestCase):
    """attach() must turn Windows' kill-on-exit off right after attaching, so
    a Python crash or Ctrl+C detaches from Clone Hero instead of killing it."""

    def setUp(self):
        self.calls = []
        self.kill_ok = True
        calls, test = self.calls, self

        class FakeK32:
            def DebugActiveProcess(self, pid):
                calls.append(("DebugActiveProcess", pid))
                return 1

            def DebugSetProcessKillOnExit(self, kill):
                calls.append(("DebugSetProcessKillOnExit", kill))
                return 1 if test.kill_ok else 0

            def DebugActiveProcessStop(self, pid):
                calls.append(("DebugActiveProcessStop", pid))
                return 1

            def OpenProcess(self, access, inherit, pid):
                calls.append(("OpenProcess", pid))
                return 0x1234

            def FlushInstructionCache(self, handle, addr, size):
                calls.append(("FlushInstructionCache", handle,
                              getattr(addr, "value", addr), size))
                return 1

        class FakeWin32:
            def __init__(self):
                self.k32 = FakeK32()

        self.k32_type = FakeK32
        self._real_win32 = debugger._Win32
        debugger._Win32 = FakeWin32

    def tearDown(self):
        debugger._Win32 = self._real_win32

    def test_attach_detaches_if_kill_on_exit_cannot_be_turned_off(self):
        self.kill_ok = False
        with self.assertRaises(OSError):
            Debugger().attach(4242)
        self.assertIn(("DebugActiveProcessStop", 4242), self.calls)
        self.assertNotIn(("OpenProcess", 4242), self.calls)

    def test_memory_goes_through_process_bindings(self):
        # The debugger reads and writes through the reader and writer
        # process.py builds over its handle; it adds only the cache flush.
        built, calls = [], self.calls

        def make_reader(handle):
            built.append(("reader", handle))

            def read(addr, size):
                calls.append(("read", addr, size))
                return b"\x90" * size
            return read

        def make_writer(handle):
            built.append(("writer", handle))

            def write(addr, data):
                calls.append(("write", addr, data))
            return write

        with mock.patch.object(debugger.process, "make_reader", make_reader), \
                mock.patch.object(debugger.process, "make_writer", make_writer):
            dbg = Debugger()
            dbg.attach(4242)
        self.assertEqual(built, [("reader", 0x1234), ("writer", 0x1234)])
        self.assertEqual(dbg._raw_read(0x5000, 2), b"\x90\x90")
        dbg._raw_write(0x5000, b"\xCC")
        self.assertEqual(calls[-3:], [
            ("read", 0x5000, 2),
            ("write", 0x5000, b"\xCC"),
            ("FlushInstructionCache", 0x1234, 0x5000, 1),
        ])
        self.assertFalse(hasattr(self.k32_type, "ReadProcessMemory"))


class SafeDetachTests(unittest.TestCase):
    """stop() removes the breakpoints, then answers events still queued for
    them. Those late events must never re-plant a 0xCC or reach the game."""

    def test_single_step_after_stop_does_not_rearm(self):
        dbg = Debugger()
        dbg._pending_rearm[7] = 0x1000   # thread 7 was mid-step; bp since removed
        ev = _exception_event(debugger.EXCEPTION_SINGLE_STEP, tid=7)
        self.assertEqual(dbg._handle_exception(ev), debugger.DBG_CONTINUE)
        self.assertEqual(dbg._pending_rearm, {})
        self.assertFalse(dbg._table.has(0x1000))

    def test_queued_hit_on_a_removed_breakpoint_rewinds_and_continues(self):
        dbg = Debugger()
        dbg._seen_initial = True
        dbg._removed.add(0x1000)
        rewound = []
        dbg._rewind_rip = rewound.append
        ev = _exception_event(debugger.EXCEPTION_BREAKPOINT, tid=7, addr=0x1000)
        self.assertEqual(dbg._handle_exception(ev), debugger.DBG_CONTINUE)
        self.assertEqual(rewound, [7])

    def test_table_lists_every_registered_address(self):
        mem = FakeMemory({0x10: 0x55, 0x20: 0x66})
        table = BreakpointTable(mem.read, mem.write)
        table.add(0x10)
        table.add(0x20)
        table.arm(0x10)
        self.assertEqual(sorted(table.addresses()), [0x10, 0x20])


if __name__ == "__main__":
    unittest.main()
