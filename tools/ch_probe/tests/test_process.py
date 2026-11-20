"""Unit tests for the process/address layer.

These test the parts that do not need a running game: the byte decoding, the
RVA-to-address math, the typed reads and writes, and the build-drift check.

The reads and writes run through the real backend (process.make_reader and
make_writer, so ReadProcessMemory and WriteProcessMemory) on a handle to this
test's own process. The bytes they read are real game bytes: a slice of
.rdata copied out of the installed game's GameAssembly.dll, laid into a page
this test allocates at the spot its RVAs resolve to. Clone Hero is never
started or touched, and nothing is left running.

What is not tested here: open_process(), _find_pid_by_name() and
_find_module_base() look the game up by name, so they need it running.

Run from the repo root with:
    py -m unittest tools.ch_probe.tests.test_process
"""

from __future__ import annotations

import os
import struct
import unittest

from .. import constants, interfaces, process
from .fakes import fake_process
from .test_find_checksum import _REPO_ROOT, _k32, OwnMemoryCase, PAGE

# The .rdata doubles around the normal and precision window constants, one
# 8-byte slot per line as "<rva> <bytes in hex>". Copied from GameAssembly.dll
# in the installed game folder (tools/ch_mcp/game_files.py, GAME_DIR), whose
# Clone Hero_Data/version.json reads v1.1.0.6142-final, the build
# constants.py was made from. .rdata is not relocated, so a loaded game holds
# these same bytes at module base + RVA.
_RDATA_SLOTS = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                            "gameassembly-6142-rdata", "window-constants.txt")
_SLOT = 8  # bytes per double


def _rdata_slots():
    """The slice's first RVA and its bytes. Fails if the slots are not one
    unbroken run, since the tests place them as one block."""
    with open(_RDATA_SLOTS, encoding="ascii") as f:
        rows = [line.split() for line in f if line.strip()]
    first = int(rows[0][0], 16)
    for i, (rva, _) in enumerate(rows):
        if int(rva, 16) != first + _SLOT * i:
            raise ValueError(f"{_RDATA_SLOTS}: slot {rva} breaks the run")
    return first, b"".join(bytes.fromhex(raw) for _, raw in rows)


FIRST_RVA, GAME_BYTES = _rdata_slots()


def _game_double(rva: int) -> float:
    """The double the game stores at `rva`, decoded by the probe's decoder."""
    off = rva - FIRST_RVA
    return process.decode_double(GAME_BYTES[off:off + _SLOT])


class GameBytesCase(OwnMemoryCase):
    """A page of this process holding the real .rdata slice, read through the
    real reader. game() gives a Process whose module base puts each RVA in
    the slice on its own bytes, so resolve() and the reads run as they do on
    the live game."""

    def setUp(self):
        super().setUp()
        self.addr = self.page()
        self.put(self.addr, GAME_BYTES)

    def game(self, shift: int = 0) -> process.Process:
        """A Process over the slice. `shift` moves the module base by that
        many bytes, the way a wrong base would."""
        return process.Process(self.addr - FIRST_RVA + shift,
                               process.make_reader(self.proc.handle),
                               process.read_only_writer)


class DecodeHelpersTest(unittest.TestCase):
    """The pure byte-to-number decoders."""

    def test_decode_double_endianness(self):
        # Feed the exact bytes for 37.5 and make sure we read little-endian,
        # not big-endian. The reversed bytes would decode to a wildly different
        # number, so a byte-order bug can't hide.
        raw = struct.pack("<d", 37.5)
        self.assertEqual(process.decode_double(raw), 37.5)
        self.assertNotEqual(process.decode_double(raw[::-1]), 37.5)

    def test_decode_double_wrong_length(self):
        with self.assertRaises(ValueError):
            process.decode_double(b"\x00\x00\x00\x00")

    def test_decode_f32_known_pattern(self):
        # 100.0 as a little-endian IEEE float is 00 00 c8 42.
        self.assertEqual(process.decode_f32(bytes([0x00, 0x00, 0xC8, 0x42])), 100.0)
        self.assertNotEqual(process.decode_f32(bytes([0x42, 0xC8, 0x00, 0x00])), 100.0)

    def test_decode_f32_wrong_length(self):
        with self.assertRaises(ValueError):
            process.decode_f32(bytes(8))

    def test_decode_u32_known_pattern(self):
        self.assertEqual(process.decode_u32(b"\x00\x10\x00\x00"), 0x1000)
        self.assertEqual(process.decode_u32(b"\xff\xff\xff\xff"), 0xFFFFFFFF)

    def test_decode_u32_wrong_length(self):
        with self.assertRaises(ValueError):
            process.decode_u32(b"\x00\x00")

    def test_decode_u64_known_pattern(self):
        self.assertEqual(process.decode_u64(struct.pack("<Q", 0x1122334455667788)),
                         0x1122334455667788)

    def test_decode_u64_wrong_length(self):
        with self.assertRaises(ValueError):
            process.decode_u64(b"\x00" * 4)


class ResolveMathTest(unittest.TestCase):
    """Process.resolve turns an RVA into a live address; pinned by literals."""

    def test_resolve_adds_base(self):
        proc, _ = fake_process({}, module_base=0x140000000)
        self.assertEqual(proc.resolve(0x20DDDA0), 0x140000000 + 0x20DDDA0)

    def test_resolve_zero_rva(self):
        proc, _ = fake_process({}, module_base=0x7FF000000000)
        self.assertEqual(proc.resolve(0), 0x7FF000000000)


class TypedReadTest(GameBytesCase):
    """The typed reads pull the right number of bytes from real memory and
    decode them."""

    def test_read_double_reads_each_game_slot(self):
        for off in range(0, len(GAME_BYTES), _SLOT):
            with self.subTest(rva=hex(FIRST_RVA + off)):
                self.assertEqual(self.proc.read_double(self.addr + off),
                                 process.decode_double(GAME_BYTES[off:off + _SLOT]))

    def test_read_u32_reads_each_game_word(self):
        for off in range(0, len(GAME_BYTES), 4):
            with self.subTest(off=off):
                self.assertEqual(self.proc.read_u32(self.addr + off),
                                 process.decode_u32(GAME_BYTES[off:off + 4]))

    def test_read_const_double_resolves_then_reads(self):
        # read_const_double takes an RVA, resolves it, then reads the double:
        # the game's two normal constants come out as constants.py expects.
        game = self.game()
        self.assertEqual(game.read_const_double(constants.RVA_CONST_NORMAL_BACK),
                         constants.EXPECT_NORMAL_BACK_S)
        self.assertEqual(game.read_const_double(constants.RVA_CONST_NORMAL_FRONT),
                         constants.EXPECT_NORMAL_FRONT_S)

    def test_read_past_readable_memory_raises(self):
        # A double that starts 4 bytes before the end of a committed page runs
        # into a page that is reserved but not committed. The read must fail
        # rather than hand back the 4 bytes it could reach.
        addr = self.reserve(2)
        self.commit(addr, 1)
        with self.assertRaises(OSError):
            self.proc.read(addr + PAGE - 4, _SLOT)


class WriteTest(GameBytesCase):
    """Process.write puts bytes into the process's memory through the real
    WriteProcessMemory binding."""

    def test_written_constants_pass_the_build_check(self):
        handle = _k32.OpenProcess(process.access_mask(False), False, os.getpid())
        if not handle:
            self.fail("OpenProcess on this process with write rights failed")
        blank = self.page()
        proc = process.Process(blank - FIRST_RVA, process.make_reader(handle),
                               process.make_writer(handle), handle=handle)
        self.addCleanup(proc.close)
        # A zeroed page holds no constants, so the check refuses it.
        with self.assertRaises(process.BuildMismatchError):
            proc.verify_targets()
        for rva in (constants.RVA_CONST_NORMAL_BACK, constants.RVA_CONST_NORMAL_FRONT):
            off = rva - FIRST_RVA
            proc.write(proc.resolve(rva), GAME_BYTES[off:off + _SLOT])
        # The game's two slots, written in, are what the check now reads.
        self.assertIsNone(proc.verify_targets())


class CheckNormalConstantsTest(unittest.TestCase):
    """The pure pass/fail decision behind verify_targets. The game stores the
    constants in seconds, so the check takes seconds."""

    TOL_S = constants.ms_to_s(constants.CONST_MATCH_TOLERANCE_MS)

    def test_game_values_pass(self):
        # The two doubles the installed game stores at the normal RVAs.
        process.check_normal_constants(
            _game_double(constants.RVA_CONST_NORMAL_BACK),
            _game_double(constants.RVA_CONST_NORMAL_FRONT))

    def test_within_tolerance_passes(self):
        # A read a hair under the tolerance is still accepted.
        process.check_normal_constants(
            constants.EXPECT_NORMAL_BACK_S + self.TOL_S * 0.9,
            constants.EXPECT_NORMAL_FRONT_S - self.TOL_S * 0.9)

    def test_back_off_by_one_ms_fails(self):
        one_ms_s = constants.ms_to_s(1.0)
        self.assertGreater(one_ms_s, self.TOL_S)
        with self.assertRaises(process.BuildMismatchError):
            process.check_normal_constants(
                constants.EXPECT_NORMAL_BACK_S - one_ms_s,
                constants.EXPECT_NORMAL_FRONT_S)

    def test_front_reading_the_precision_back_fails(self):
        # A front read that lands on the game's precision-mode back constant.
        precision_back_s = _game_double(constants.RVA_CONST_PRECISION_BACK)
        self.assertGreater(abs(precision_back_s - constants.EXPECT_NORMAL_FRONT_S),
                           self.TOL_S)
        with self.assertRaises(process.BuildMismatchError):
            process.check_normal_constants(constants.EXPECT_NORMAL_BACK_S,
                                           precision_back_s)

    def test_just_past_tolerance_fails(self):
        with self.assertRaises(process.BuildMismatchError):
            process.check_normal_constants(
                constants.EXPECT_NORMAL_BACK_S + self.TOL_S * 2,
                constants.EXPECT_NORMAL_FRONT_S)

    def test_values_in_ms_are_refused(self):
        # The check reads seconds: a build-drift guard that accepted ms would be wrong.
        with self.assertRaises(process.BuildMismatchError):
            process.check_normal_constants(
                constants.s_to_ms(constants.EXPECT_NORMAL_BACK_S),
                constants.s_to_ms(constants.EXPECT_NORMAL_FRONT_S))


class VerifyTargetsTest(GameBytesCase):
    """verify_targets reads the two constants from memory, then applies the
    check."""

    def _assert_drift_caught(self, game: process.Process, rva: int,
                             expect_s: float) -> None:
        read_s = game.read_const_double(rva)
        self.assertGreater(abs(read_s - expect_s), CheckNormalConstantsTest.TOL_S)
        with self.assertRaises(process.BuildMismatchError):
            game.verify_targets()

    def test_good_build_passes(self):
        self.assertIsNone(self.game().verify_targets())

    def test_base_one_slot_late_raises(self):
        # One double past the normal back constant holds a different number.
        self._assert_drift_caught(self.game(shift=_SLOT),
                                  constants.RVA_CONST_NORMAL_BACK,
                                  constants.EXPECT_NORMAL_BACK_S)

    def test_precision_pair_raises(self):
        # A base that lands each normal read on its precision-mode constant.
        shift = constants.RVA_CONST_PRECISION_FRONT - constants.RVA_CONST_NORMAL_FRONT
        self.assertEqual(
            shift, constants.RVA_CONST_PRECISION_BACK - constants.RVA_CONST_NORMAL_BACK)
        self._assert_drift_caught(self.game(shift=shift),
                                  constants.RVA_CONST_NORMAL_FRONT,
                                  constants.EXPECT_NORMAL_FRONT_S)


class ProtocolShapeTest(OwnMemoryCase):
    """Process must satisfy the ProcessHandle Protocol so the other modules can
    rely on its method names."""

    def test_process_is_a_process_handle(self):
        self.assertIsInstance(self.proc, interfaces.ProcessHandle)


if __name__ == "__main__":
    unittest.main()
