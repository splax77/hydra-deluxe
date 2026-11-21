"""Tests for the read-only open: the rights it asks for, and the writer that
refuses every write.

No game: the open runs open_process(read_only=True) on this test's own
process, so the handle, the module lookup and the ReadProcessMemory reader are
the real ones. Only the search for the game's process by name is pointed at
this process's id.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_process_readonly
"""

from __future__ import annotations

import ctypes
import os
import struct
import sys
import unittest
from unittest import mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import process  # noqa: E402

# Every Windows process has kernel32.dll loaded, so the module lookup finds it
# in this one.
_OWN_MODULE = "kernel32.dll"


class AccessMaskTest(unittest.TestCase):
    def test_read_only_asks_for_read_and_query_rights(self):
        mask = process.access_mask(True)
        self.assertTrue(mask & process._PROCESS_VM_READ, "a read-only open can read memory")
        self.assertTrue(
            mask & process._PROCESS_QUERY_INFORMATION,
            "a read-only open can query memory regions",
        )
        self.assertFalse(mask & process._PROCESS_VM_WRITE, "a read-only open asks no right to write")
        self.assertFalse(
            mask & process._PROCESS_VM_OPERATION,
            "a read-only open asks no right to change memory protection",
        )
        self.assertEqual(
            mask & ~(process._PROCESS_VM_READ | process._PROCESS_QUERY_INFORMATION),
            0,
            "a read-only open asks for no other right",
        )

    def test_full_open_keeps_the_four_rights(self):
        mask = process.access_mask(False)
        for name in (
            "_PROCESS_VM_READ",
            "_PROCESS_VM_WRITE",
            "_PROCESS_VM_OPERATION",
            "_PROCESS_QUERY_INFORMATION",
        ):
            with self.subTest(right=name):
                self.assertTrue(mask & getattr(process, name), f"a full open asks for {name}")


class ReadOnlyOpenTest(unittest.TestCase):
    def setUp(self):
        self.cell = ctypes.c_double(1.5)
        self.addr = ctypes.addressof(self.cell)
        with mock.patch.object(process, "_find_pid_by_name", return_value=os.getpid()), \
                mock.patch.object(process.constants, "MODULE_NAME", _OWN_MODULE):
            self.proc = process.open_process(
                os.path.basename(sys.executable), read_only=True
            )
        self.addCleanup(self.proc.close)

    def test_write_raises(self):
        with self.assertRaises(PermissionError) as caught:
            self.proc.write(self.addr, struct.pack("<d", 9.0))
        # The refusal is read_only_writer's own.
        with self.assertRaises(PermissionError) as owner:
            process.read_only_writer(self.addr, b"")
        self.assertEqual(str(caught.exception), str(owner.exception))
        # Nothing reached memory, and the read-only handle still reads it.
        self.assertEqual(self.cell.value, 1.5)
        self.assertEqual(self.proc.read_double(self.addr), 1.5)


if __name__ == "__main__":
    unittest.main()
