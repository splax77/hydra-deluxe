"""Tests for the checksum finder (overlay spike, question 2): the hash forms,
the direct search, the pointer walk and the finder's run.

The hash forms run on a real chart and the score file the game wrote after
playing it. The rest has no game: a real Process over fake memory answers the
reads, a fake region list stands in for VirtualQueryEx, and a fake engine
finder stands in for the live search.
"""

from __future__ import annotations

import functools
import hashlib
import io
import json
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

# A real chart and the game's own score file from a run of it. The score file
# was copied from hydra-data/ch-mcp-runs/2026-11-12/pitch-dark-run1.
_PITCH_DARK_NOTES = os.path.join(_REPO_ROOT, "testdata", "input",
                                 "test_pitch_dark_ratio", "notes.mid")
_PITCH_DARK_SCORESTATS = os.path.join(
    _REPO_ROOT, "testdata", "ch_runs", "pitch-dark-run1",
    "scorestats-CHON_-_Pitch_Dark_B-20261112-230618.json")


def _read_scorestats() -> dict:
    with open(_PITCH_DARK_SCORESTATS, encoding="utf-8") as f:
        return json.load(f)


def _notes_md5() -> bytes:
    with open(_PITCH_DARK_NOTES, "rb") as f:
        return hashlib.md5(f.read()).digest()


# The checksum the game wrote for the chart it played: the hash the finder
# searches memory for.
HASH = _read_scorestats()["checksum"]

RAW = _notes_md5()
ASCII_UPPER = HASH.upper().encode("ascii")
ASCII_LOWER = HASH.lower().encode("ascii")

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
    def test_the_raw_form_is_the_md5_of_the_chart_the_game_played(self):
        # The game's checksum for a folder chart is the MD5 of its notes file
        # (docs/clone-hero/01-reading-a-chart.md, CR-10), so the raw pattern
        # the finder searches for must be those digest bytes.
        raw = [f.pattern for f in S.encode_forms(HASH) if f.name == S.FORM_RAW]
        self.assertEqual(raw, [_notes_md5()])

    def test_each_form_reads_back_as_the_game_checksum(self):
        forms = {f.name: f for f in S.encode_forms(HASH)}
        self.assertEqual(set(forms), {S.FORM_RAW, S.FORM_UTF16_UPPER,
                                      S.FORM_UTF16_LOWER, S.FORM_ASCII_UPPER,
                                      S.FORM_ASCII_LOWER})
        self.assertEqual(forms[S.FORM_RAW].pattern.hex().upper(),
                         HASH.upper())
        self.assertEqual(
            forms[S.FORM_UTF16_UPPER].pattern.decode("utf-16-le"),
            HASH.upper())
        self.assertEqual(
            forms[S.FORM_UTF16_LOWER].pattern.decode("utf-16-le"),
            HASH.lower())
        self.assertEqual(forms[S.FORM_ASCII_UPPER].pattern.decode("ascii"),
                         HASH.upper())
        self.assertEqual(forms[S.FORM_ASCII_LOWER].pattern.decode("ascii"),
                         HASH.lower())
        # Only the UTF-16 forms can be a System.String's characters, which
        # the pointer walk follows.
        self.assertEqual({name for name, f in forms.items() if f.utf16},
                         {S.FORM_UTF16_UPPER, S.FORM_UTF16_LOWER})

    def test_lower_case_input_gives_the_same_forms(self):
        self.assertEqual(S.encode_forms(HASH.lower()), S.encode_forms(HASH))

    def test_a_string_that_is_not_a_hash_is_refused(self):
        stats = _read_scorestats()
        # The game's checksum one digit short, and another text field of the
        # same score file.
        for text in (stats["checksum"][:-1], stats["song_name"]):
            with self.subTest(text=text):
                with self.assertRaises(ValueError):
                    S.encode_forms(text)


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
