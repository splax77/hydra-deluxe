"""Tests for the checksum finder (overlay spike, question 2): the hash forms,
the direct search, the pointer walk and the song clock notes.

The hash forms run on a real chart and the score file the game wrote after
playing it. The clock notes run on song clock readings recorded from inside
the game. The search and the walk run the finder's real readers
(ReadProcessMemory and VirtualQueryEx, on a read-only handle) over pages this
test process allocates for itself, so no stand-in answers a single read.
"""

from __future__ import annotations

import ctypes
import hashlib
import io
import json
import mmap
import os
import struct
import sys
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import il2cpp_strings as S  # noqa: E402
from tools.ch_probe import process  # noqa: E402
from tools.ch_probe.experiments import find_checksum as F  # noqa: E402

# A real chart and the game's own score file from a run of it. The score file
# was copied from hydra-data/ch-mcp-runs/2026-11-12/pitch-dark-run1.
_PITCH_DARK_NOTES = os.path.join(_REPO_ROOT, "testdata", "input",
                                 "test_pitch_dark_ratio", "notes.mid")
_PITCH_DARK_SCORESTATS = os.path.join(
    _REPO_ROOT, "testdata", "ch_runs", "pitch-dark-run1",
    "scorestats-CHON_-_Pitch_Dark_B-20261112-230618.json")

# The song clock read from inside the game every quarter second over the end
# of a song: moving while it plays, then the same reading twice once the song
# is over. Copied from hydra-data/ch-mcp-runs/2026-10-28/l2/clock.jsonl.
_SONG_OVER_CLOCK = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                                "2026-10-28-l2", "clock-song-over.jsonl")


def _read_scorestats() -> dict:
    with open(_PITCH_DARK_SCORESTATS, encoding="utf-8") as f:
        return json.load(f)


def _notes_md5() -> bytes:
    with open(_PITCH_DARK_NOTES, "rb") as f:
        return hashlib.md5(f.read()).digest()


def _song_over_readings() -> list:
    """The recorded samples, in order, each as its payload dict."""
    with open(_SONG_OVER_CLOCK, encoding="utf-8") as f:
        return [json.loads(line)["payload"] for line in f if line.strip()]


def _moving_and_frozen_pairs():
    """Two consecutive readings while the song plays, and the two last
    readings, which the game gave after the song was over."""
    samples = _song_over_readings()
    playing = [s for s in samples if not s["isSongOver"]]
    moving = (playing[0]["songTime"], playing[1]["songTime"])
    frozen = (samples[-2]["songTime"], samples[-1]["songTime"])
    assert samples[-2]["isSongOver"] and samples[-1]["isSongOver"]
    return moving, frozen


# The checksum the game wrote for the chart it played: the hash the finder
# searches memory for.
HASH = _read_scorestats()["checksum"]

RAW = _notes_md5()
ASCII_UPPER = HASH.upper().encode("ascii")
ASCII_LOWER = HASH.lower().encode("ascii")


# --- this process's own memory, read the way the finder reads the game -------

# VirtualAlloc commits memory a page at a time, so a page is the smallest
# region the region walk can report.
PAGE = mmap.PAGESIZE
_MEM_COMMIT = 0x1000
_MEM_RESERVE = 0x2000
_MEM_RELEASE = 0x8000
_PAGE_READWRITE = 0x04

_k32 = ctypes.WinDLL("kernel32", use_last_error=True)
_k32.VirtualAlloc.restype = ctypes.c_void_p
_k32.VirtualAlloc.argtypes = [ctypes.c_void_p, ctypes.c_size_t,
                              ctypes.c_uint32, ctypes.c_uint32]
_k32.VirtualFree.restype = ctypes.c_int
_k32.VirtualFree.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint32]
_k32.OpenProcess.restype = ctypes.c_void_p
_k32.OpenProcess.argtypes = [ctypes.c_uint32, ctypes.c_int, ctypes.c_uint32]


class OwnMemoryCase(unittest.TestCase):
    """A test that allocates pages in this process and reads them through
    the real reader on a read-only handle to this same process, opened with
    the rights the finder asks for (process.access_mask)."""

    def setUp(self):
        handle = _k32.OpenProcess(process.access_mask(True), False,
                                  os.getpid())
        if not handle:
            raise OSError(ctypes.get_last_error(), "OpenProcess on self failed")
        self.proc = process.Process(0, process.make_reader(handle),
                                    process.read_only_writer,
                                    pid=os.getpid(), handle=handle)
        self.addCleanup(self.proc.close)

    def reserve(self, pages: int) -> int:
        """Reserve `pages` pages with none committed: reading them fails."""
        addr = _k32.VirtualAlloc(None, pages * PAGE, _MEM_RESERVE,
                                 _PAGE_READWRITE)
        if not addr:
            raise OSError(ctypes.get_last_error(), "VirtualAlloc failed")
        self.addCleanup(_k32.VirtualFree, addr, 0, _MEM_RELEASE)
        return addr

    def commit(self, addr: int, pages: int) -> int:
        """Commit `pages` zeroed read-write pages at `addr`."""
        got = _k32.VirtualAlloc(addr, pages * PAGE, _MEM_COMMIT,
                                _PAGE_READWRITE)
        if not got:
            raise OSError(ctypes.get_last_error(), "VirtualAlloc commit failed")
        return got

    def page(self) -> int:
        """One committed page of its own, a region of its own."""
        addr = self.reserve(1)
        return self.commit(addr, 1)

    @staticmethod
    def put(addr: int, data: bytes) -> None:
        ctypes.memmove(addr, data, len(data))

    @staticmethod
    def put_u64(addr: int, value: int) -> None:
        ctypes.memmove(addr, struct.pack("<Q", value), 8)

    @staticmethod
    def put_double(addr: int, value: float) -> None:
        ctypes.memmove(addr, struct.pack("<d", value), 8)


def _hits_in(result, start: int, size: int):
    """The (address, form name) of each hit inside [start, start + size)."""
    return [(h.address, h.form.name) for h in result.hits
            if start <= h.address < start + size]


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


class DirectSearchTest(OwnMemoryCase):
    def test_hits_at_a_regions_two_ends_are_found_and_stop_at_its_start(self):
        # The raw hash fills the first and the last bytes of one page, found
        # by the finder's own walk over this process's regions.
        region = self.page()
        last = region + PAGE - len(RAW)
        self.put(region, RAW)
        self.put(last, RAW)
        self.assertIn((region, PAGE), list(F.live_rw_regions(self.proc)))
        result = F.scan_for_hash(self.proc, S.encode_forms(HASH))
        self.assertEqual(_hits_in(result, region, PAGE),
                         [(region, S.FORM_RAW), (last, S.FORM_RAW)])
        hits = {h.address: h for h in result.hits}
        # A hit at the region's first byte has nothing before it, even when
        # the memory below the region is readable; the last hit carries the
        # region's own bytes before it.
        self.assertEqual(hits[region].before, b"")
        self.assertEqual(hits[last].before,
                         self.proc.read(last - F.CONTEXT_BYTES,
                                        F.CONTEXT_BYTES))

    def test_an_unreadable_region_is_skipped(self):
        # Two pages reserved, only the second committed: reading the first
        # fails in ReadProcessMemory itself.
        reserved = self.reserve(2)
        readable = self.commit(reserved + PAGE, 1)
        self.put(readable, ASCII_UPPER)
        with self.assertRaises(OSError):
            self.proc.read(reserved, PAGE)
        result = F.scan_for_hash(self.proc, S.encode_forms(HASH),
                                 regions=lambda p: [(reserved, PAGE),
                                                    (readable, PAGE)])
        self.assertEqual(result.regions, 1)
        self.assertEqual(result.bytes_scanned, PAGE)
        self.assertEqual(_hits_in(result, readable, PAGE),
                         [(readable, S.FORM_ASCII_UPPER)])


class PointerWalkTest(OwnMemoryCase):
    def _engine_pointing_at(self, objects):
        engine = self.page()
        for slot, obj in enumerate(objects, start=1):
            self.put_u64(engine + slot * F.POINTER_SIZE, obj)
        return engine

    def test_the_walk_stops_at_its_object_cap_and_says_so(self):
        objects = [self.page() for _ in range(3)]
        engine = self._engine_pointing_at(objects)
        # Nothing points at the target, so only the cap can end the walk
        # early. With no cap in the way it reads the engine and each object.
        target = self.page()
        whole = F.walk_pointers(self.proc, engine, [target])
        self.assertEqual(whole.paths, [])
        self.assertEqual(whole.objects_read, 1 + len(objects))
        self.assertFalse(whole.hit_cap)
        cap = len(objects) - 1
        capped = F.walk_pointers(self.proc, engine, [target], max_objects=cap)
        self.assertEqual(capped.paths, [])
        self.assertEqual(capped.objects_read, cap)
        self.assertTrue(capped.hit_cap)

    def test_no_path_says_not_found_within_the_hop_limit(self):
        # A UTF-16 hit whose string object nothing points to.
        string_obj = self.page()
        self.put(string_obj + S.STRING_CHARS_OFFSET,
                 HASH.upper().encode("utf-16-le"))
        hits = [h for h in F.scan_for_hash(
            self.proc, S.encode_forms(HASH),
            regions=lambda p: [(string_obj, PAGE)]).hits if h.form.utf16]
        self.assertEqual([h.address for h in hits],
                         [string_obj + S.STRING_CHARS_OFFSET])
        engine = self._engine_pointing_at([])
        out = io.StringIO()
        result = F._pointer_report(self.proc, engine, hits, time.perf_counter,
                                   out)
        text = out.getvalue()
        self.assertEqual(result.paths, [])
        self.assertIn(f"pointer path not found within {F.MAX_HOPS} hops",
                      text)
        self.assertIn(f"read {result.objects_read} objects, took", text)


class RescanTest(OwnMemoryCase):
    def test_a_hash_that_moves_between_passes_is_reported_moved(self):
        region = self.page()
        forms = S.encode_forms(HASH)

        def search():
            return F.scan_for_hash(self.proc, forms,
                                   regions=lambda p: [(region, PAGE)]).hits

        # One form stays put; the other moves to the next slot along.
        self.put(region, ASCII_UPPER)
        old = region + PAGE // 2
        new = old + len(ASCII_LOWER)
        self.put(old, ASCII_LOWER)
        first = search()
        self.put(old, b"\0" * len(ASCII_LOWER))
        self.put(new, ASCII_LOWER)
        second = search()
        held, moved, appeared = F.compare_passes(first, second)
        self.assertEqual([(h.address, h.form.name) for h in held],
                         [(region, S.FORM_ASCII_UPPER)])
        self.assertEqual([(h.address, h.form.name) for h in moved],
                         [(old, S.FORM_ASCII_LOWER)])
        self.assertEqual([(h.address, h.form.name) for h in appeared],
                         [(new, S.FORM_ASCII_LOWER)])


class ClockStateTest(OwnMemoryCase):
    def test_the_three_states(self):
        moving, frozen = _moving_and_frozen_pairs()
        self.assertEqual(F.clock_state(*moving), F.CLOCK_MOVING)
        self.assertEqual(F.clock_state(*frozen), F.CLOCK_FROZEN)
        # A clock read from memory that cannot be read, on either side.
        unread = F._read_clock(self.proc, self.reserve(1) - C.OFF_SONG_CLOCK)
        self.assertIsNone(unread)
        self.assertEqual(F.clock_state(moving[0], unread), F.CLOCK_STOPPED)
        self.assertEqual(F.clock_state(unread, moving[1]), F.CLOCK_STOPPED)


class ClockNoteTest(OwnMemoryCase):
    def test_the_note_carries_both_clock_readings(self):
        for before, after in _moving_and_frozen_pairs():
            with self.subTest(before=before, after=after):
                note = F.clock_note(before, after)
                self.assertTrue(note.startswith(F.clock_state(before, after)))
                first = note.index(f"{before:.2f}")
                self.assertGreater(note.index(f"{after:.2f}", first + 1),
                                   first)

    def test_an_unreadable_clock_has_no_readings(self):
        moving, _frozen = _moving_and_frozen_pairs()
        unread = F._read_clock(self.proc, self.reserve(1) - C.OFF_SONG_CLOCK)
        self.assertEqual(F.clock_note(moving[0], unread), F.CLOCK_STOPPED)

    def test_the_report_prints_the_readings_of_a_frozen_pass(self):
        # The song clock the game held after the song was over, put where the
        # finder reads it on the engine object.
        _moving, (reading, _same) = _moving_and_frozen_pairs()
        engine = self.page()
        self.put_double(engine + C.OFF_SONG_CLOCK, reading)
        out = io.StringIO()
        F._timed_scan(self.proc, engine, S.encode_forms(HASH),
                      lambda p: [], time.perf_counter, out, "First search")
        self.assertIn(f"clock {F.clock_note(reading, reading)}",
                      out.getvalue())


if __name__ == "__main__":
    unittest.main()
