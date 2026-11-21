"""Unit tests for engine.py, run with no game and no debugger.

The whole point of the engine model taking a ProcessHandle through its
constructor is that we can hand it a fake here. The fake memory returns
whatever double or dword we told it to for a given address. The score-rise
check runs on a real run's press log instead, and the engine class's field
types come from the real 1.1 build's class dump.

The process is a real Process from the shared builder in fakes.py, so the
address maths and the decoding are production's.

These tests import engine.py as a top-level module. To make that work without a
package root, we put the ch_probe directory itself on sys.path first, so
`import engine` and `import constants` resolve.
"""

import json
import os
import sys
import unittest

# Put the ch_probe directory (parent of this tests/ dir) on the path so the
# sibling modules import as top-level names.
_CH_PROBE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _CH_PROBE_DIR not in sys.path:
    sys.path.insert(0, _CH_PROBE_DIR)

import constants as C  # noqa: E402
import engine  # noqa: E402

# The repo root, so the press log loader and testdata/ resolve.
_REPO_ROOT = os.path.dirname(os.path.dirname(_CH_PROBE_DIR))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_facts  # noqa: E402
from tools.ch_probe.experiments import score_compare  # noqa: E402

try:
    from . import fakes  # noqa: E402
except ImportError:  # pragma: no cover - run as a script from this folder
    import fakes  # type: ignore[no-redef]  # noqa: E402


# --- helpers -----------------------------------------------------------------

OBJ = 0x1234000  # a made-up but fixed engine object address
MODULE_BASE = 0x7FF000000000  # a made-up but fixed GameAssembly.dll base


def make_engine(doubles=None, dwords=None, consts=None, memory=None):
    """An EngineModel over a real Process on fake memory, with no engine
    object adopted yet. The FakeMemory records every read."""
    proc, mem = fakes.fake_process(memory, MODULE_BASE, doubles=doubles,
                                   dwords=dwords, consts=consts)
    return engine.EngineModel(proc), proc, mem


# --- tests: offset arithmetic ------------------------------------------------


# The real 1.1 build's facts cut to its drum engine class, copied from
# hydra-data/ch-dumps/6142/facts.json; test_frida_tools.py reads the same
# folder. The dump lists only the fields the sealed class itself adds (IN-09 on
# docs/clone-hero/07-internals.md), so offsets of the generic base, such as
# the song clock's, are not in it.
_REAL_6142_DUMPS = os.path.join(_REPO_ROOT, "testdata", "ch_runs",
                                "ch-dumps-6142")


def real_engine_fields():
    """The drum engine class's constructor RVA and its instance fields as
    {offset: type}, from the real 6142 dump."""
    facts = build_facts.load_facts(os.path.join(_REAL_6142_DUMPS, "game"),
                                   _REAL_6142_DUMPS)
    name, ctor_rva = build_facts.engine_class(facts)
    cls = build_facts._one_class(facts, name)
    return ctor_rva, {f["offset"]: f["type"] for f in cls["fields"]
                      if not f["static"]}


class TestEngineClassFields(unittest.TestCase):
    def test_the_dump_is_the_class_whose_constructor_constants_names(self):
        ctor_rva, _ = real_engine_fields()
        self.assertEqual(ctor_rva, C.RVA_DRUMS_ENGINE_CTOR)

    def test_round_start_is_a_double_field_of_the_engine(self):
        _, fields = real_engine_fields()
        self.assertEqual(fields.get(C.OFF_ROUND_START), "double")

    def test_dyn_threshold_is_a_uint_field_of_the_engine(self):
        _, fields = real_engine_fields()
        self.assertEqual(fields.get(C.OFF_DYN_THRESHOLD), "uint")


class TestFieldReads(unittest.TestCase):
    def test_reads_before_use_object_raise(self):
        eng, proc, mem = make_engine(doubles={OBJ + C.OFF_SONG_CLOCK: 3.5})
        with self.assertRaises(RuntimeError):
            eng.song_clock()
        self.assertEqual(mem.reads, [])


# --- tests: precision-mode bit -----------------------------------------------


class TestPrecisionMode(unittest.TestCase):
    def test_is_precision_reads_the_one_bit(self):
        # The game tests bit 0x1000 of the flags dword (IN-10 and TM-28 on
        # docs/clone-hero/07-internals.md and 03-timing.md), and no other bit.
        self.assertTrue(engine.is_precision(0x1000))
        self.assertFalse(engine.is_precision(0x0FFF))
        self.assertTrue(engine.is_precision(0xFFFFFFFF))
        self.assertTrue(engine.is_precision(0x0FF1 | 0x1000))
        self.assertFalse(engine.is_precision(0xFFFFFFFF & ~0x1000))
        self.assertFalse(engine.is_precision(0))


# --- tests: did the score rise -----------------------------------------------


# A real run's press log and the game's own score file for that play, copied
# from hydra-data/ch-mcp-runs/2026-11-07/ls-e-run1. The log has one row per
# note pressed (121), and four presses were sent outside the hit window.
_LS_E_RUN = os.path.join(_REPO_ROOT, "testdata", "ch_runs", "ls-e-run1")
_LS_E_PRESS_LOG = os.path.join(_LS_E_RUN, "press-1.csv")
_LS_E_SCORESTATS = os.path.join(_LS_E_RUN,
                                "scorestats-Edge_Walk_Pair-20261107-233205.json")


class TestPressedInputHit(unittest.TestCase):
    def test_score_rises_count_the_notes_the_game_says_were_hit(self):
        # The expected counts are Clone Hero's own, from its scorestats file:
        # pressed_input_hit over the log's real score pairs must call exactly
        # the game's notes_hit presses hits and the rest misses.
        presses = score_compare.load_press_log(_LS_E_PRESS_LOG)
        with open(_LS_E_SCORESTATS, encoding="utf-8") as f:
            player = json.load(f)["players"][0]
        self.assertEqual(len(presses), player["total_notes"])
        hits = sum(engine.pressed_input_hit(p.score_before, p.score_after)
                   for p in presses)
        self.assertEqual(hits, player["notes_hit"])
        self.assertEqual(len(presses) - hits,
                         player["total_notes"] - player["notes_hit"])


# --- tests: constants --------------------------------------------------------


def every_rdata_rva():
    """Every .rdata constant constants.py names: each RVA_CONST_*,
    RVA_FORMULA_* and RVA_HITCHECK_* value, and each value of the formula
    tables. A constant added there is expected without editing this list."""
    rvas = []
    for name in sorted(vars(C)):
        if not name.startswith(("RVA_CONST_", "RVA_FORMULA_", "RVA_HITCHECK_")):
            continue
        value = getattr(C, name)
        rvas += list(value.values()) if isinstance(value, dict) else [value]
    return rvas


class TestConstants(unittest.TestCase):
    def test_reads_every_expected_rva(self):
        rvas = every_rdata_rva()
        eng, proc, mem = make_engine(consts={rva: 0.0 for rva in rvas})
        eng.constants()
        addrs_read = [addr for addr, _ in mem.reads]
        for rva in rvas:
            self.assertIn(proc.resolve(rva), addrs_read,
                          msg="constants() did not read RVA 0x%X" % rva)


# --- tests: song clock (live-only seam) --------------------------------------


class TestSongClock(unittest.TestCase):
    def test_reads_the_song_clock_field(self):
        doubles = {OBJ + C.OFF_SONG_CLOCK: 3.5}
        eng, proc, mem = make_engine(doubles=doubles)
        eng.use_object(OBJ)
        self.assertEqual(eng.song_clock(), 3.5)


class TestFoundObject(unittest.TestCase):
    def test_dyn_threshold_reads_its_field(self):
        eng, proc, mem = make_engine(dwords={OBJ + C.OFF_DYN_THRESHOLD: 64})
        eng.use_object(OBJ)
        self.assertEqual(eng.dyn_threshold(), 64)


if __name__ == "__main__":
    unittest.main()
