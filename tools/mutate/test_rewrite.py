"""Tests for rewrite.py, the rewriter, on every real C++ file under src/.

    py -I -m unittest discover -s tools/mutate -p test_rewrite.py

The rewriter runs on the repo's own sources, never on made-up snippets, and
writes into a temporary folder. Each test checks a property every copy must
hold: every switch off reads back as the original byte for byte, one switch on
reads back as the original with only that spot's change (for a compare spot,
the helper call on its two operands), no copy lands in
src/, and the switch numbers are unique and the same on a second run. The
copies are read back through hydra_precheck's code mask, so the main
checkout's hydra_precheck.exe (find.py's default_precheck) must exist;
without it every test fails, loudly.
"""

import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import find  # noqa: E402
import rewrite  # noqa: E402

REPO = Path(__file__).resolve().parents[2]


def tracked(*patterns):
    out = subprocess.run(["git", "ls-files", *patterns], cwd=REPO, capture_output=True, text=True, check=True)
    return out.stdout.split()


SOURCES = tracked("src/*.cpp")
HEADERS = tracked("src/*.h")


class RewriteSrc(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = Path(tempfile.mkdtemp(prefix="hydra_rewrite_test_"))
        cls.out = cls.tmp / "first"
        cls.listing = rewrite.rewrite(SOURCES + HEADERS, cls.out, REPO)
        cls.spots = {rel: find.find_spots(REPO / rel) for rel in SOURCES + HEADERS}

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def pieces(self, rel):
        copy = self.out / "mutated" / rel
        return rewrite.parse_copy(rewrite.read_text(copy), find.code_mask(copy, find.default_precheck()))

    def test_every_switch_off_gives_the_original_bytes(self):
        self.assertTrue(self.listing["copies"], "no file under src/ got a copy")
        for rel in self.listing["copies"]:
            with self.subTest(file=rel):
                off = rewrite.take_sides(self.pieces(rel))
                self.assertEqual(off.encode("utf-8"), (REPO / rel).read_bytes())

    def test_one_switch_on_differs_only_by_its_spots_change(self):
        pieces = {rel: self.pieces(rel) for rel in self.listing["copies"]}
        for m in self.listing["mutants"]:
            spot = next(s for s in self.spots[m["file"]] if s.number == m["spot"])
            with self.subTest(switch=m["number"], file=m["file"], line=m["line"]):
                self.assertIsNone(spot.skip)
                self.assertEqual(spot.replacement, find.KINDS[spot.kind]["swap"][spot.original])
                self.assertTrue(spot.expr_start <= spot.offset < spot.offset + spot.length <= spot.expr_end)
                original = (REPO / m["file"]).read_bytes().decode("utf-8")
                on = rewrite.take_sides(pieces[m["file"]], m["number"])
                if spot.kind != "compare":
                    self.assertEqual(on, original[:spot.offset] + spot.replacement + original[spot.offset + spot.length:])
                    continue
                # A compare spot: the two operands, untouched, handed to the
                # helper for the flipped operator; the rest byte for byte.
                self.assertTrue(spot.expr_start <= spot.lhs_start < spot.lhs_end <= spot.offset)
                self.assertTrue(spot.offset + spot.length <= spot.rhs_start < spot.rhs_end <= spot.expr_end)
                head, tail = original[:spot.lhs_start], original[spot.rhs_end:]
                self.assertTrue(on.startswith(head) and on.endswith(tail))
                call = on[len(head):len(on) - len(tail)]
                lhs = original[spot.lhs_start:spot.lhs_end]
                rhs = original[spot.rhs_start:spot.rhs_end]
                self.assertEqual(call, f"::hydra_mutant::{rewrite.HELPERS[spot.replacement]}<{m['number']}>"
                                       f"(({lhs}), ({rhs}))")

    def test_every_helper_is_defined_in_the_switch_header(self):
        header = (REPO / "tools" / "mutate" / "hydra_mutant.h").read_text(encoding="utf-8")
        self.assertEqual(sorted(rewrite.HELPERS), sorted(find.KINDS["compare"]["swap"].values()))
        for op, name in rewrite.HELPERS.items():
            with self.subTest(op=op):
                self.assertRegex(header, r"\bbool " + name + r"\(const A& a, const B& b\)")
        used = {m["kind"] for m in self.listing["mutants"]}
        self.assertIn("compare", used, "no compare spot under src/ became a switch")

    def test_no_copy_lands_in_src(self):
        for rel in self.listing["copies"]:
            copy = (self.out / "mutated" / rel).resolve()
            self.assertTrue(copy.is_file())
            self.assertFalse(rewrite._inside(copy, REPO / "src"), copy)
        with self.assertRaises(rewrite.RewriteError):
            rewrite.rewrite(SOURCES[:1], REPO / "src" / "mutation_out_never_written", REPO)
        self.assertFalse((REPO / "src" / "mutation_out_never_written").exists())

    def test_numbers_are_unique_and_stable(self):
        numbers = [m["number"] for m in self.listing["mutants"]]
        self.assertEqual(numbers, list(range(1, len(numbers) + 1)))
        second_out = self.tmp / "second"
        # Given in the other order: the numbers follow the paths, not the arguments.
        second = rewrite.rewrite(list(reversed(SOURCES + HEADERS)), second_out, REPO)
        self.assertEqual(second["mutants"], self.listing["mutants"])
        self.assertEqual(second["skipped"], self.listing["skipped"])
        for rel in self.listing["copies"]:
            self.assertEqual((second_out / "mutated" / rel).read_bytes(), (self.out / "mutated" / rel).read_bytes())

    def test_every_spot_is_a_switch_or_skipped_once(self):
        seen = {}
        for m in self.listing["mutants"]:
            seen.setdefault((m["file"], m["spot"]), []).append("switch")
        for s in self.listing["skipped"]:
            seen.setdefault((s["file"], s["spot"]), []).append(s["reason"])
            self.assertIn(s["reason"], self.listing["skip_reasons"])
        for rel, spots in self.spots.items():
            for s in spots:
                self.assertEqual(len(seen.get((rel, s.number), [])), 1, f"{rel} spot {s.number}")
                if s.skip is not None:
                    self.assertEqual(seen[(rel, s.number)], [s.skip])

    def test_headers_get_no_copy_and_their_spots_are_skipped(self):
        for rel in self.listing["copies"]:
            self.assertIn(Path(rel).suffix, rewrite.COPY_SUFFIXES)
        planted_in_headers = 0
        for rel in HEADERS:
            self.assertFalse((self.out / "mutated" / rel).exists())
            for s in self.spots[rel]:
                if s.skip is None:
                    planted_in_headers += 1
                    self.assertIn({"file": rel, "line": s.line, "kind": s.kind, "change": s.change,
                                   "spot": s.number, "reason": "not_copied"}, self.listing["skipped"])
        self.assertGreater(planted_in_headers, 0, "no header in src/ has a spot the finder would plant")

    def test_nested_and_shared_expressions_occur(self):
        # The round-trip tests above only prove the nesting rule if real
        # files hold such spots; this says they do.
        nested = shared = 0
        for rel in self.listing["copies"]:
            for w in rewrite.wrappers(self.pieces(rel)):
                inner = list(rewrite.wrappers(w.off))
                if inner:
                    nested += 1
        for rel in SOURCES:
            planted = [s for s in self.spots[rel] if s.skip is None]
            bounds = [(s.expr_start, s.expr_end) for s in planted]
            shared += len(bounds) - len(set(bounds))
        self.assertGreater(nested, 0)
        self.assertGreater(shared, 0)

    def test_copy_lines_hold_the_switch(self):
        for m in self.listing["mutants"]:
            lines = rewrite.read_text(self.out / "mutated" / m["copy"]).split("\n")
            first, last = m["copy_lines"]
            span = "\n".join(lines[first - 1:last])
            self.assertIn(f"{rewrite.MACRO}({m['number']}, (", span)

    def test_a_second_run_clears_the_first_runs_copies(self):
        out = self.tmp / "rerun"
        rewrite.rewrite(SOURCES, out, REPO)
        listing = rewrite.rewrite(SOURCES[:1], out, REPO)
        present = sorted(p.relative_to(out / "mutated").as_posix()
                         for p in (out / "mutated").rglob("*") if p.is_file())
        self.assertEqual(present, listing["copies"])

    def test_a_file_no_run_listed_stops_the_run(self):
        out = self.tmp / "foreign"
        stranger = out / "mutated" / SOURCES[0]
        stranger.parent.mkdir(parents=True)
        shutil.copyfile(REPO / SOURCES[0], stranger)
        with self.assertRaises(rewrite.RewriteError):
            rewrite.rewrite(SOURCES[:1], out, REPO)
        self.assertEqual(stranger.read_bytes(), (REPO / SOURCES[0]).read_bytes())


if __name__ == "__main__":
    unittest.main()
