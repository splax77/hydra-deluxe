"""Tests for compare_dumps (the comparison of two builds' facts files).

Each test builds two small facts objects by hand, in the shape the README
describes, so no real dump is read.
"""

from __future__ import annotations

import io
import json
import os
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import compare_dumps as C  # noqa: E402

SCR_X = "ʺʺʸʶʿʵ"
SCR_Y = "ʿʺʸʺʼʴ"
SCR_Z = "ʾʸʸʻʷʸ"
SCR_F1 = "ʳʴʵ"
SCR_F2 = "ʶʷʸ"
SCR_F3 = "ʹʺʻ"


def field(name, offset, type_="double", static=False):
    return {"name": name, "type": type_, "offset": offset, "static": static}


def method(name, rva=0x1000):
    return {"name": name, "return": "void", "params": [], "static": False, "rva": rva}


def cls(name, fields=(), methods=(), namespace="", readable=None):
    return {
        "name": name,
        "namespace": namespace,
        "kind": "class",
        "typedef_index": 1,
        "readable": name.isascii() if readable is None else readable,
        "base": None,
        "interfaces": [],
        "fields": list(fields),
        "methods": list(methods),
    }


def facts(version, *classes):
    return {"game_version": version, "dump_file": "dump.cs", "classes": list(classes), "signatures": {}}


def scrambled_pair():
    """Two scrambled classes, the same in both builds."""
    return [
        cls(SCR_X, [field(SCR_F1, 0x10, "int")], [method(SCR_F2)]),
        cls(SCR_Y, [field(SCR_F1, 0x10, "bool")], []),
    ]


class TestReadableClasses(unittest.TestCase):
    def test_a_moved_offset_is_reported(self):
        old = facts("A", cls("GameManager", [field("songTime", 0x70), field("isSongOver", 0x91, "bool")]))
        new = facts("B", cls("GameManager", [field("songTime", 0x78), field("isSongOver", 0x91, "bool")]))
        lines = C.compare(old, new)
        self.assertIn("field offsets: 2 fields compared, 1 moved, 0 not comparable (offset null on a side)", lines)
        self.assertIn("  GameManager.songTime: 0x70 -> 0x78", lines)
        self.assertNotIn("  GameManager.isSongOver: 0x91 -> 0x91", lines)

    def test_a_missing_class_is_listed_on_its_side(self):
        old = facts("A", cls("GameManager"), cls("OldOnly", namespace="CloneHero"))
        new = facts("B", cls("GameManager"), cls("NewOnly"))
        lines = C.compare(old, new)
        i = lines.index("readable classes only in A: 1")
        self.assertEqual(lines[i + 1], "  CloneHero.OldOnly")
        j = lines.index("readable classes only in B: 1")
        self.assertEqual(lines[j + 1], "  NewOnly")
        self.assertIn(
            "readable classes in both: 1 (0 appear more than once in a file, so their fields are not compared)",
            lines,
        )

    def test_classes_match_by_namespace_and_name_not_by_file(self):
        # The same class from differently named source files still matches;
        # the same name in another namespace does not.
        old = facts("A", cls("GameManager", [field("songTime", 0x70)]), cls("Note", namespace="Old"))
        new = facts("B", cls("GameManager", [field("songTime", 0x70)]), cls("Note", namespace="New"))
        new["dump_file"] = "Assembly-CSharp.dll"
        lines = C.compare(old, new)
        self.assertIn("field offsets: 1 fields compared, 0 moved, 0 not comparable (offset null on a side)", lines)
        self.assertIn("  Old.Note", lines)
        self.assertIn("  New.Note", lines)

    def test_a_null_offset_is_not_comparable_and_never_moved(self):
        old = facts("A", cls("GameManager", [field("songTime", 0x70), field("isPaused", 0x90, "bool")]))
        new = facts("B", cls("GameManager", [field("songTime", None), field("isPaused", None, "bool")]))
        lines = C.compare(old, new)
        self.assertIn("field offsets: 0 fields compared, 0 moved, 2 not comparable (offset null on a side)", lines)
        self.assertFalse(any(line.startswith("  GameManager.") for line in lines))

    def test_a_class_printed_twice_in_a_file_is_not_compared(self):
        old = facts("A", cls("<Module>", [field("x", 0x0)]), cls("<Module>", [field("x", 0x8)]))
        new = facts("B", cls("<Module>", [field("x", 0x10)]))
        lines = C.compare(old, new)
        self.assertIn(
            "readable classes in both: 1 (1 appear more than once in a file, so their fields are not compared)",
            lines,
        )
        self.assertIn("field offsets: 0 fields compared, 0 moved, 0 not comparable (offset null on a side)", lines)


class TestMemberNames(unittest.TestCase):
    def test_scrambled_field_names_turning_readable_are_counted(self):
        old = facts(
            "1.0",
            cls(
                "GameManager",
                [field(SCR_F1, None), field(SCR_F2, None), field(SCR_F3, None)],
                [method(SCR_F1, None), method("Update", None)],
            ),
        )
        new = facts(
            "1.1",
            cls(
                "GameManager",
                [field(SCR_F1, 0x10), field("songTime", 0x70), field("isSongOver", 0x91)],
                [method(SCR_F1), method("Update")],
            ),
        )
        lines = C.compare(old, new)
        self.assertIn(
            "member names in readable classes: 1 classes changed; "
            "field names went scrambled to readable in 1, readable to scrambled in 0, other count changes in 0; "
            "method names went scrambled to readable in 0, readable to scrambled in 0, other count changes in 0",
            lines,
        )
        self.assertIn(
            "  GameManager: fields scrambled 3 -> 1, readable 0 -> 2; methods scrambled 1 -> 1, readable 1 -> 1",
            lines,
        )
        # SCR_F1 is in both, but its 1.0 offset is null.
        self.assertIn("field offsets: 0 fields compared, 0 moved, 1 not comparable (offset null on a side)", lines)

    def test_readable_names_turning_scrambled_are_counted_the_other_way(self):
        old = facts("A", cls("GameManager", [], [method("Update"), method("Start")]))
        new = facts("B", cls("GameManager", [], [method(SCR_F1), method(SCR_F2)]))
        lines = C.compare(old, new)
        self.assertIn(
            "member names in readable classes: 1 classes changed; "
            "field names went scrambled to readable in 0, readable to scrambled in 0, other count changes in 0; "
            "method names went scrambled to readable in 0, readable to scrambled in 1, other count changes in 0",
            lines,
        )

    def test_unchanged_member_names_list_no_class(self):
        old = facts("A", cls("GameManager", [field("songTime", 0x70)]))
        new = facts("B", cls("GameManager", [field("songTime", 0x70)]))
        lines = C.compare(old, new)
        self.assertIn(
            "member names in readable classes: 0 classes changed; "
            "field names went scrambled to readable in 0, readable to scrambled in 0, other count changes in 0; "
            "method names went scrambled to readable in 0, readable to scrambled in 0, other count changes in 0",
            lines,
        )
        self.assertFalse(any(line.startswith("  GameManager:") for line in lines))


class TestScrambledNames(unittest.TestCase):
    def test_kept_names_end_with_names_stable(self):
        lines = C.compare(facts("A", *scrambled_pair()), facts("B", *scrambled_pair()))
        self.assertEqual(lines[-1], "names stable: 2 of 2 scrambled classes keep their names")
        self.assertEqual(
            lines[-2],
            "scrambled classes: 2 names in A, 2 in B; same name and shape in both: 2; "
            "same name, other shape: 0; only in A: 0; only in B: 0",
        )

    def test_a_renamed_class_ends_with_names_reshuffled(self):
        x, y = scrambled_pair()
        renamed = dict(y, name=SCR_Z)
        lines = C.compare(facts("A", x, y), facts("B", x, renamed))
        self.assertEqual(lines[-1], "names reshuffled: only 1 of 2 scrambled names survive")
        self.assertEqual(
            lines[-2],
            "scrambled classes: 2 names in A, 2 in B; same name and shape in both: 1; "
            "same name, other shape: 0; only in A: 1; only in B: 1",
        )

    def test_a_kept_name_with_another_shape_does_not_survive(self):
        x, y = scrambled_pair()
        y_more_methods = dict(y, methods=[method(SCR_F3)])
        y_other_type = dict(y, fields=[field(SCR_F1, 0x10, "int")])
        for changed in (y_more_methods, y_other_type):
            lines = C.compare(facts("A", x, y), facts("B", x, changed))
            self.assertEqual(lines[-1], "names reshuffled: only 1 of 2 scrambled names survive")
            self.assertIn("same name, other shape: 1", lines[-2])

    def test_field_offsets_and_rvas_are_not_part_of_the_shape(self):
        # A Mono build writes null offsets and RVAs; the name still survives.
        x, y = scrambled_pair()
        x_null = dict(x, fields=[field(SCR_F1, None, "int")], methods=[method(SCR_F2, None)])
        lines = C.compare(facts("A", x, y), facts("B", x_null, y))
        self.assertEqual(lines[-1], "names stable: 2 of 2 scrambled classes keep their names")

    def test_a_name_printed_twice_survives_when_every_copy_matches(self):
        x, y = scrambled_pair()
        twin = dict(x, fields=[])
        lines = C.compare(facts("A", x, twin, y), facts("B", twin, x, y))
        self.assertEqual(lines[-1], "names stable: 2 of 2 scrambled classes keep their names")
        lines = C.compare(facts("A", x, twin, y), facts("B", x, y))
        self.assertEqual(lines[-1], "names reshuffled: only 1 of 2 scrambled names survive")


class TestCommandLine(unittest.TestCase):
    def _write(self, folder, name, data):
        path = os.path.join(folder, name)
        with open(path, "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False)
        return path

    def test_prints_the_report_and_ends_with_the_verdict(self):
        with tempfile.TemporaryDirectory() as tmp:
            a = self._write(tmp, "a.json", facts("1.1.0.6142-final", *scrambled_pair()))
            b = self._write(tmp, "b.json", facts("1.1.0.6142-final", *scrambled_pair()))
            out = io.StringIO()
            with redirect_stdout(out):
                code = C.main([a, b])
        self.assertEqual(code, 0)
        printed = out.getvalue().splitlines()
        self.assertEqual(printed[0], f"A file: {a}")
        self.assertEqual(printed[1], f"B file: {b}")
        self.assertEqual(printed[2], "A: 1.1.0.6142-final (2 classes)")
        self.assertEqual(printed[-1], "names stable: 2 of 2 scrambled classes keep their names")

    def test_a_missing_file_fails_loudly(self):
        with tempfile.TemporaryDirectory() as tmp:
            a = self._write(tmp, "a.json", facts("A"))
            missing = os.path.join(tmp, "absent.json")
            err = io.StringIO()
            with redirect_stdout(io.StringIO()), redirect_stderr(err):
                code = C.main([a, missing])
        self.assertEqual(code, 1)
        self.assertTrue(err.getvalue().startswith(f"compare_dumps: {missing}: "))

    def test_a_file_without_classes_fails_loudly(self):
        with tempfile.TemporaryDirectory() as tmp:
            a = self._write(tmp, "a.json", facts("A"))
            b = self._write(tmp, "b.json", {"game_version": "B"})
            err = io.StringIO()
            with redirect_stdout(io.StringIO()), redirect_stderr(err):
                code = C.main([a, b])
        self.assertEqual(code, 1)
        self.assertEqual(err.getvalue(), f"compare_dumps: {b}: no 'classes' list\n")


if __name__ == "__main__":
    unittest.main()
