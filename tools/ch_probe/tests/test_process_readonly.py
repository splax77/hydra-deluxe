"""Tests for the read-only open: the rights it asks for, and the writer that
refuses every write.

No game: open_process itself is LIVE-ONLY, so these tests drive the two pieces
it is built from, access_mask and read_only_writer, with a real Process over
a hand-made reader for the reads.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_process_readonly
"""

from __future__ import annotations

import os
import struct
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import process  # noqa: E402

BASE = 0x180000000


class AccessMaskTest(unittest.TestCase):
    def test_read_only_asks_for_read_and_query_rights(self):
        self.assertEqual(process.access_mask(True), 0x0410)

    def test_full_open_keeps_the_four_rights(self):
        self.assertEqual(process.access_mask(False), 0x0438)


class ReadOnlyWriterTest(unittest.TestCase):
    def _read_only_process(self):
        """A real Process whose reader serves one double, wired the way
        open_process wires a read-only open."""
        addr = 0x20000
        cells = {addr: struct.pack("<d", 1.5)}

        def reader(a, size):
            return cells[a][:size]

        return process.Process(BASE, reader, process.read_only_writer), addr

    def test_write_raises(self):
        proc, addr = self._read_only_process()
        with self.assertRaises(PermissionError) as caught:
            proc.write(addr, b"\x00" * 8)
        self.assertEqual(str(caught.exception), "process opened read-only")

    def test_reads_work_normally(self):
        proc, addr = self._read_only_process()
        self.assertEqual(proc.read_double(addr), 1.5)


if __name__ == "__main__":
    unittest.main()
