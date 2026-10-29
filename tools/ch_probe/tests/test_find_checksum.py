"""Tests for the checksum finder (overlay spike, question 2): the hash forms,
the Il2Cpp string decode, the direct search, the pointer walk and the
finder's run.

No game: a real Process over fake memory answers the reads, a fake region
list stands in for VirtualQueryEx, and a fake engine finder stands in for the
live search.
"""

from __future__ import annotations

import functools
import io
import os
import struct
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import engine_finder  # noqa: E402
from tools.ch_probe import il2cpp_strings as S  # noqa: E402
from tools.ch_probe import process  # noqa: E402
from tools.ch_probe.experiments import find_checksum as F  # noqa: E402
from tools.ch_probe.tests import fakes  # noqa: E402

# The MD5 of the NS E star cutoffs notes.mid, equal to the game's checksum for
# that chart in scorestats.json (the spike plan, "What the code does today").
HASH = "234C917654CC7257E2BAFCF7D761304B"

RAW = b"\x23\x4c\x91\x76\x54\xcc\x72\x57\xe2\xba\xfc\xf7\xd7\x61\x30\x4b"
UTF16_UPPER = (b"2\x003\x004\x00C\x009\x001\x007\x006\x00"
               b"5\x004\x00C\x00C\x007\x002\x005\x007\x00"
               b"E\x002\x00B\x00A\x00F\x00C\x00F\x007\x00"
               b"D\x007\x006\x001\x003\x000\x004\x00B\x00")
UTF16_LOWER = (b"2\x003\x004\x00c\x009\x001\x007\x006\x00"
               b"5\x004\x00c\x00c\x007\x002\x005\x007\x00"
               b"e\x002\x00b\x00a\x00f\x00c\x00f\x007\x00"
               b"d\x007\x006\x001\x003\x000\x004\x00b\x00")
ASCII_UPPER = b"234C917654CC7257E2BAFCF7D761304B"
ASCII_LOWER = b"234c917654cc7257e2bafcf7d761304b"

BASE = 0x180000000
ENGINE = 0x10000000
CLOCK = ENGINE + C.OFF_SONG_CLOCK
VERIFY_CONSTS = {C.RVA_CONST_NORMAL_BACK: C.EXPECT_NORMAL_BACK_S,
                 C.RVA_CONST_NORMAL_FRONT: C.EXPECT_NORMAL_FRONT_S}


def string_object(text: str) -> bytes:
    """A System.String object as the game lays it out: a zeroed header, the
    length at +0x10, the UTF-16LE characters from +0x14."""
    return (b"\0" * 0x10 + struct.pack("<i", len(text))
            + text.encode("utf-16-le"))


def object_with(slots: dict) -> bytes:
    """A 0x400-byte object whose 8-byte slots hold the given values."""
    raw = bytearray(0x400)
    for offset, value in slots.items():
        struct.pack_into("<Q", raw, offset, value)
    return bytes(raw)


class RecordingOpener:
    """Stands in for open_process and remembers how it was called."""

    def __init__(self, proc):
        self.proc = proc
        self.calls = []

    def __call__(self, *args, **kwargs):
        self.calls.append((args, kwargs))
        return self.proc


class FakeTime:
    """A clock that moves only when the code under test sleeps."""

    def __init__(self):
        self.t = 1000.0
        self.sleeps = []

    def now(self):
        return self.t

    def sleep(self, seconds):
        self.sleeps.append(seconds)
        self.t += seconds


def run_finder(proc, regions, *, find_engine=None, opener=None, sleep=None):
    out = io.StringIO()
    clock = FakeTime()
    code = F.run(
        ["--hash", HASH],
        open_proc=opener or RecordingOpener(proc),
        find_engine=find_engine or (lambda p, patterns, out: ENGINE),
        regions=regions,
        sleep=sleep or clock.sleep,
        now=clock.now,
        out=out)
    return code, out.getvalue()


class EncodeFormsTest(unittest.TestCase):
    def test_the_five_forms_of_the_known_hash(self):
        forms = S.encode_forms(HASH)
        self.assertEqual(
            [(f.name, f.pattern, f.utf16) for f in forms],
            [("raw bytes", RAW, False),
             ("UTF-16 upper hex", UTF16_UPPER, True),
             ("UTF-16 lower hex", UTF16_LOWER, True),
             ("ASCII upper hex", ASCII_UPPER, False),
             ("ASCII lower hex", ASCII_LOWER, False)])

    def test_lower_case_input_gives_the_same_forms(self):
        self.assertEqual(S.encode_forms(HASH.lower()), S.encode_forms(HASH))

    def test_a_string_that_is_not_a_hash_is_refused(self):
        with self.assertRaises(ValueError):
            S.encode_forms(HASH[:-1])
        with self.assertRaises(ValueError):
            S.encode_forms(HASH[:-1] + "G")


class DecodeStringTest(unittest.TestCase):
    def test_a_string_object_decodes_to_the_hash(self):
        self.assertEqual(S.decode_il2cpp_string(string_object(HASH)), HASH)

    def test_an_absurd_length_is_refused(self):
        raw = b"\0" * 0x10 + struct.pack("<i", 5000) + b"\0" * 64
        self.assertIsNone(S.decode_il2cpp_string(raw))

    def test_a_negative_length_is_refused(self):
        raw = b"\0" * 0x10 + struct.pack("<i", -1) + b"\0" * 64
        self.assertIsNone(S.decode_il2cpp_string(raw))

    def test_fewer_characters_than_the_length_says_is_refused(self):
        self.assertIsNone(S.decode_il2cpp_string(string_object(HASH)[:-2]))


class DirectSearchTest(unittest.TestCase):
    def test_hits_at_region_edges_are_counted_once(self):
        # Two regions side by side: the raw hash ends region one and starts
        # region two. The region walk names region one twice.
        r1, r2 = 0x20000, 0x20040
        data1 = b"\x11" * 0x30 + RAW
        data2 = RAW + b"\x22" * 0x30
        proc, _ = fakes.fake_process({r1: data1, r2: data2}, BASE)
        forms = S.encode_forms(HASH)
        result = F.scan_for_hash(
            proc, forms, regions=lambda p: [(r1, 0x40), (r2, 0x40), (r1, 0x40)])
        self.assertEqual([(h.address, h.form.name) for h in result.hits],
                         [(0x20030, "raw bytes"), (0x20040, "raw bytes")])
        # The bytes before a hit stop at its own region's start.
        self.assertEqual(result.hits[0].before, b"\x11" * 32)
        self.assertEqual(result.hits[1].before, b"")

    def test_an_unreadable_region_is_skipped(self):
        r1 = 0x20000
        proc, _ = fakes.fake_process({r1: ASCII_UPPER}, BASE)
        result = F.scan_for_hash(proc, S.encode_forms(HASH),
                                 regions=lambda p: [(0x90000, 0x100),
                                                    (r1, len(ASCII_UPPER))])
        self.assertEqual(result.regions, 1)
        self.assertEqual(result.bytes_scanned, 32)
        self.assertEqual([(h.address, h.form.name) for h in result.hits],
                         [(r1, "ASCII upper hex")])


class PointerWalkTest(unittest.TestCase):
    def test_a_two_hop_path_prints_its_exact_offsets(self):
        a, b, s = 0x30000, 0x40000, 0x50000
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({0x58: a}),
             a: object_with({0x20: b}),
             b: object_with({0x10: s}),
             s: string_object(HASH)},
            BASE, consts=VERIFY_CONSTS, clocks={CLOCK: [1.0, 1.5, 2.0]})
        code, text = run_finder(proc, lambda p: [(s, len(string_object(HASH)))])
        self.assertEqual(code, 0)
        self.assertIn(
            "engine+0x58 -> +0x20 -> +0x10 = 234C917654CC7257E2BAFCF7D761304B",
            text)
        self.assertIn(f"{s + 0x14:#x}  UTF-16 upper hex", text)

    def test_no_path_within_the_object_cap_says_not_found(self):
        objs = [0x30000, 0x31000, 0x32000]
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({0x8: objs[0], 0x10: objs[1], 0x18: objs[2]}),
             objs[0]: object_with({}), objs[1]: object_with({}),
             objs[2]: object_with({})},
            BASE)
        result = F.walk_pointers(proc, ENGINE, [0x50000], max_objects=2)
        self.assertEqual(result.paths, [])
        self.assertEqual(result.objects_read, 2)
        self.assertTrue(result.hit_cap)

    def test_no_path_still_exits_0_with_the_direct_hits_listed(self):
        s = 0x50000
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({0x8: 0x30000}),
             0x30000: object_with({0x8: ENGINE}),
             s: string_object(HASH)},
            BASE, consts=VERIFY_CONSTS, clocks={CLOCK: [3.0]})
        code, text = run_finder(proc, lambda p: [(s, len(string_object(HASH)))])
        self.assertEqual(code, 0)
        self.assertIn("pointer path not found within 2 hops", text)
        self.assertIn(f"{s + 0x14:#x}  UTF-16 upper hex", text)
        self.assertIn("read 2 objects, took", text)
        self.assertIn("clock frozen", text)


class SongEntryHolderTest(unittest.TestCase):
    def test_a_pointer_to_the_song_entry_names_its_holder(self):
        entry = 0x70000            # the SongEntry; its checksum at +0x38
        globals_obj = 0x80000      # holds the SongEntry at +0x28
        entry_bytes = b"\0" * 0x38 + RAW + b"\0" * 8
        globals_bytes = object_with({0x28: entry})[:0x40]
        # A copy of the pointer one byte off alignment is not a holder.
        stray = b"\0" + struct.pack("<Q", entry) + b"\0" * 7
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({}), entry: entry_bytes,
             globals_obj: globals_bytes, 0x90000: stray},
            BASE, consts=VERIFY_CONSTS, clocks={CLOCK: [1.0, 2.0]})
        regions = lambda p: [(entry, len(entry_bytes)),
                             (globals_obj, len(globals_bytes)),
                             (0x90000, len(stray))]
        code, text = run_finder(proc, regions)
        self.assertEqual(code, 0)
        self.assertIn("rawhit 0x70038 -> SongEntry 0x70000 <- held at 0x80028 "
                      "(candidate GlobalVariables = 0x80000)", text)
        self.assertIn("  1 holders, took", text)

    def test_no_pointer_to_the_song_entry_says_so(self):
        entry = 0x70000
        entry_bytes = b"\0" * 0x38 + RAW + b"\0" * 8
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({}), entry: entry_bytes},
            BASE, consts=VERIFY_CONSTS, clocks={CLOCK: [1.0]})
        code, text = run_finder(proc, lambda p: [(entry, len(entry_bytes))])
        self.assertEqual(code, 0)
        self.assertIn("rawhit 0x70038 -> SongEntry 0x70000 <- no pointer to "
                      "it found", text)
        self.assertIn("  0 holders, took", text)


class MovingMemory:
    """Memory whose region contents change when the finder sleeps between
    its two searches."""

    def __init__(self, cells):
        self.cells = dict(cells)

    def read(self, addr, size):
        raw = self.cells.get(addr)
        if raw is None:
            raise OSError(f"nothing mapped at {addr:#x}")
        return raw[:size]


class RescanTest(unittest.TestCase):
    def test_a_hash_that_moves_between_passes_is_reported_moved(self):
        region = 0x60000
        before = b"\0" * 0x40 + ASCII_LOWER + b"\0" * 0x60
        after = b"\0" * 0x80 + ASCII_LOWER + b"\0" * 0x20
        memory = MovingMemory({ENGINE: object_with({}), region: before,
                               CLOCK: struct.pack("<d", 4.0)})
        proc = process.Process(BASE, memory.read, process.read_only_writer)
        for rva, value in VERIFY_CONSTS.items():
            memory.cells[proc.resolve(rva)] = struct.pack("<d", value)
        sleeps = []

        def sleep(seconds):
            sleeps.append(seconds)
            memory.cells[region] = after

        code, text = run_finder(proc, lambda p: [(region, len(before))],
                                sleep=sleep)
        self.assertEqual(code, 0)
        self.assertEqual(sleeps, [10.0])
        self.assertIn("0 held, 1 moved, 1 new", text)
        self.assertIn(f"moved  {region + 0x40:#x}  ASCII lower hex", text)
        self.assertIn(f"new    {region + 0x80:#x}  ASCII lower hex", text)


class RunTest(unittest.TestCase):
    def test_the_game_is_opened_read_only(self):
        proc, _ = fakes.fake_process({ENGINE: object_with({})}, BASE,
                                     consts=VERIFY_CONSTS, clocks={CLOCK: [1.0]})
        opener = RecordingOpener(proc)
        code, _text = run_finder(proc, lambda p: [], opener=opener)
        self.assertEqual(code, 0)
        self.assertEqual(opener.calls, [((), {"read_only": True})])

    def test_no_playing_song_exits_2_with_the_message(self):
        frozen = 0x20000
        proc, _ = fakes.fake_process(
            None, BASE, consts=VERIFY_CONSTS,
            doubles={frozen + C.OFF_TOTAL_WINDOW: 0.17},
            clocks={frozen + C.OFF_SONG_CLOCK: [3.0]})
        clock = FakeTime()
        finder = functools.partial(
            engine_finder.find_live_engine_capped,
            scan=lambda p, b, f: ([frozen], 0, 0),
            sleep=clock.sleep, now=clock.now)
        scanned = []
        code, text = run_finder(proc, lambda p: scanned.append(p) or [],
                                find_engine=finder)
        self.assertEqual(code, 2)
        self.assertIn(
            "no playing song found in 120 s; start the song and run again",
            text)
        self.assertEqual(scanned, [])


class ClockStateTest(unittest.TestCase):
    def test_the_three_states(self):
        self.assertEqual(F.clock_state(1.0, 1.5), "moving")
        self.assertEqual(F.clock_state(1.0, 1.0), "frozen")
        self.assertEqual(F.clock_state(1.0, None),
                         "stopped (the engine object could not be read)")
        self.assertEqual(F.clock_state(None, 1.0),
                         "stopped (the engine object could not be read)")


class ClockNoteTest(unittest.TestCase):
    def test_the_note_carries_both_clock_readings(self):
        self.assertEqual(F.clock_note(10.0, 10.0),
                         "frozen (song clock 10.00 -> 10.00 s)")
        self.assertEqual(F.clock_note(1.0, 1.5),
                         "moving (song clock 1.00 -> 1.50 s)")

    def test_an_unreadable_clock_has_no_readings(self):
        self.assertEqual(F.clock_note(1.0, None),
                         "stopped (the engine object could not be read)")

    def test_the_report_prints_the_readings_of_a_frozen_pass(self):
        proc, _ = fakes.fake_process(
            {ENGINE: object_with({})}, BASE, consts=VERIFY_CONSTS,
            clocks={CLOCK: [3.0]})
        _code, text = run_finder(proc, lambda p: [])
        self.assertIn("clock frozen (song clock 3.00 -> 3.00 s)", text)


if __name__ == "__main__":
    unittest.main()
