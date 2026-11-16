"""Tests for py_mutate.py, run on the real tools/ folder and the real hooks folder.

    py -I -m unittest discover -s tools/mutate -p test_py_mutate.py

Every check is a property the rewrite must hold on real files; no snippet,
fixture or count is typed here.
"""

import ast
import json
import os
import shutil
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import py_mutate as pm  # noqa: E402

TOOLS = os.path.dirname(HERE)
GATES = os.path.expanduser("~/.claude/gates")
FOLDERS = [TOOLS] + ([GATES] if os.path.isdir(GATES) else [])


def _py_mtimes(folder):
    out = {}
    for root, dirs, names in os.walk(folder):
        dirs[:] = [d for d in dirs if d != "__pycache__"]
        for n in names:
            if n.endswith(".py"):
                p = os.path.join(root, n)
                out[p] = os.stat(p).st_mtime_ns
    return out


class RewriteOnRealFolders(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.mkdtemp(prefix="py_mutate_test_")
        cls.before = {f: _py_mtimes(f) for f in FOLDERS}
        cls.runs = {}
        for i, folder in enumerate(FOLDERS):
            copy = os.path.join(cls.tmp, "copy%d" % i)
            cls.runs[folder] = (copy,) + pm.copy_and_rewrite(folder, copy)
        cls.after = {f: _py_mtimes(f) for f in FOLDERS}

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.tmp, ignore_errors=True)

    def test_some_bugs_are_planted(self):
        for folder, (_, mutants, _) in self.runs.items():
            self.assertTrue(mutants, folder)

    def test_off_side_gives_back_the_original_ast(self):
        for folder, (copy, mutants, skipped) in self.runs.items():
            unparsed = {s["file"] for s in skipped if s["reason"] in pm.FILE_SKIP_REASONS.values()}
            for root, dirs, names in os.walk(copy):
                for n in names:
                    if not n.endswith(".py"):
                        continue
                    rel = os.path.relpath(os.path.join(root, n), copy).replace("\\", "/")
                    if rel in unparsed:
                        continue
                    with self.subTest(file=rel):
                        original = pm._read(os.path.join(folder, rel))
                        rewritten = pm._read(os.path.join(root, n))
                        self.assertTrue(pm.same_ast(pm.unwrap(rewritten), ast.parse(original)))

    def test_on_side_is_exactly_the_one_edit(self):
        for folder, (copy, mutants, _) in self.runs.items():
            texts = {}
            for m in mutants:
                if m["file"] not in texts:
                    texts[m["file"]] = (pm._read(os.path.join(folder, m["file"])),
                                        pm._read(os.path.join(copy, m["file"])))
                original, rewritten = texts[m["file"]]
                with self.subTest(file=m["file"], number=m["number"]):
                    rows = list(pm.KINDS[m["kind"]].values())
                    self.assertIn((m["was"], m["now"]), rows)
                    self.assertEqual(original[m["at"]:m["at"] + len(m["was"])], m["was"])
                    self.assertTrue(m["lo"] <= m["at"] and m["at"] + len(m["was"]) <= m["hi"])
                    one_edit = original[:m["at"]] + m["now"] + original[m["at"] + len(m["was"]):]
                    self.assertTrue(pm.same_ast(pm.unwrap(rewritten, on=m["number"]), ast.parse(one_edit)))
                    self.assertFalse(pm.same_ast(ast.parse(one_edit), ast.parse(original)))

    def test_skips_carry_a_reason_from_the_tables(self):
        reasons = set(pm.SKIP_REASONS.values()) | set(pm.FILE_SKIP_REASONS.values())
        for _, (_, _, skipped) in self.runs.items():
            for s in skipped:
                self.assertIn(s["reason"], reasons)

    def test_numbers_are_unique_and_the_same_on_a_second_rewrite(self):
        for folder, (_, mutants, skipped) in self.runs.items():
            numbers = [m["number"] for m in mutants]
            self.assertEqual(numbers, list(range(1, len(numbers) + 1)))
            again = pm.copy_and_rewrite(folder, os.path.join(self.tmp, "again%d" % FOLDERS.index(folder)))
            self.assertEqual(json.dumps(again[0]), json.dumps(mutants))
            self.assertEqual(json.dumps(again[1]), json.dumps(skipped))

    def test_real_folders_are_never_written(self):
        self.assertEqual(self.before, self.after)
        for folder in FOLDERS:
            self.assertEqual(self.before[folder], _py_mtimes(folder))
        with self.assertRaises(SystemExit):
            pm.copy_and_rewrite(TOOLS, os.path.join(TOOLS, "mutate", "never_here"))
        self.assertFalse(os.path.exists(os.path.join(TOOLS, "mutate", "never_here")))


def _first_test_with_target(folder):
    """The first test file in name order whose target module sits beside it.

    Chosen by name order so nobody picks it (the plan's LS5 rule).
    """
    for n in sorted(os.listdir(folder)):
        if pm.is_test_file(n) and n.endswith(".py"):
            target = n[len("test_"):] if n.startswith("test_") else None
            if target and os.path.isfile(os.path.join(folder, target)):
                return n, target
    return None


class MappingAndOneBugOnARealModule(unittest.TestCase):
    def test_map_then_the_first_reached_bug(self):
        pick = _first_test_with_target(TOOLS)
        if pick is None:
            self.skipTest("no test file in tools/ has its target beside it")
        test_file, target = pick
        tmp = tempfile.mkdtemp(prefix="py_mutate_run_")
        self.addCleanup(shutil.rmtree, tmp, True)
        copy = os.path.join(tmp, "copy")
        mutants, _ = pm.copy_and_rewrite(TOOLS, copy, [target])
        lines, _, timed_out = pm.run_child(copy, os.path.join(tmp, "map.jsonl"), 0, [test_file[:-3]])
        self.assertFalse(timed_out)
        tests, reach, edge, red = pm.build_map(lines)
        self.assertTrue(tests)
        numbers = {m["number"] for m in mutants}
        reached = set().union(*reach.values(), *edge.values())
        self.assertTrue(reached, "the target's own test file reached none of its switches")
        self.assertLessEqual(reached, numbers)
        os.makedirs(os.path.join(tmp, "bugs"))
        n = min(reached)
        load = next(x["seconds"] for x in lines if x["test"] == "before any test")
        bug = pm.run_bug(tmp, copy, n, tests, reach, edge, red, load)
        self.assertEqual(bug["number"], n)
        self.assertEqual(bug["modules"], [test_file[:-3]])
        with open(os.path.join(tmp, "bugs", "%d.jsonl" % n), encoding="utf-8") as f:
            ran = [json.loads(x) for x in f]
        statuses = {x["test"]: x["status"] for x in ran if "status" in x}
        self.assertEqual(set(statuses), {x["test"] for x in tests})
        for c in bug["caught_by"]:
            self.assertIn(statuses[c["test"]], ("failed", "error"))
            self.assertIn(n, reach[c["test"]] | set().union(*edge.values()))
        self.assertEqual(bug["red_not_reaching"], [])


def _absolute_path_imports(folder):
    """Test files that put a literal absolute path on sys.path or load a module from one."""
    found = []
    for root, dirs, names in os.walk(folder):
        dirs[:] = [d for d in dirs if d != "__pycache__"]
        for n in sorted(names):
            if not (n.endswith(".py") and pm.is_test_file(n)):
                continue
            path = os.path.join(root, n)
            try:
                tree = ast.parse(pm._read(path))
            except (SyntaxError, UnicodeDecodeError):
                continue
            for node in ast.walk(tree):
                if not isinstance(node, ast.Call):
                    continue
                f = node.func
                name = f.attr if isinstance(f, ast.Attribute) else getattr(f, "id", "")
                if name not in ("insert", "append", "spec_from_file_location"):
                    continue
                if any(isinstance(a, ast.Constant) and isinstance(a.value, str) and os.path.isabs(a.value)
                       for a in node.args):
                    found.append(os.path.relpath(path, folder))
                    break
    return found


class AbsolutePathImportsReachNoBug(unittest.TestCase):
    def test_a_test_that_imports_the_real_folder_reaches_no_bug(self):
        for folder in FOLDERS:
            cases = _absolute_path_imports(folder)
            if not cases:
                continue
            rel = cases[0]
            tmp = tempfile.mkdtemp(prefix="py_mutate_abs_")
            self.addCleanup(shutil.rmtree, tmp, True)
            copy = os.path.join(tmp, "copy")
            pm.copy_and_rewrite(folder, copy)
            module = rel[:-3].replace("\\", ".").replace("/", ".")
            lines, _, _ = pm.run_child(copy, os.path.join(tmp, "map.jsonl"), 0, [module])
            tests, reach, _, _ = pm.build_map(lines)
            for t in tests:
                self.assertEqual(reach[t["test"]], set(), t["test"])
            return
        self.skipTest("none found: no test file in %s puts a literal absolute path on sys.path"
                      % " or ".join(FOLDERS))


if __name__ == "__main__":
    unittest.main()
