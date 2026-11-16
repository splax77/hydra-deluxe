"""Tests for find.py, the spot finder, on every real C++ file under src/.

    py -I -m unittest discover -s tools/mutate -p test_find.py

(`-I` keeps the current folder off the import path, so the dotted form
`py -I -m unittest tools/mutate/test_find.py` cannot import the file.)

The finder runs on the repo's own sources, never on made-up snippets. Each
test checks a property every spot must hold. The skip-rule test searches src/
for real lines each rule exists for, then tests/ and tools/ when src/ has
none, and fails if the finder plants a bug there. The finder needs the main checkout's hydra_precheck.exe (find.py's
default_precheck); without it every test fails, loudly.
"""

import re
import subprocess
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import find  # noqa: E402

REPO = Path(__file__).resolve().parents[2]

_BINARY = r"(?:==|!=|<=|>=|<|>|&&|\|\|)"

# For each skip reason, a search for real src/ lines the reason exists for.
# The named group `tok` is the token a kind matches. Each search is the plain
# shape of the construct, not the finder's rule; the test then asks the
# finder about that token. None: no search can find a real line (said why).
SEARCHES = {
    "preprocessor": re.compile(r"(?m)^[ \t]*#[ \t]*(?:el)?if\b[^\n]*?(?P<tok>" + _BINARY + r") "),
    "operator_name": re.compile(r"\boperator ?(?P<tok>==|!=|<=|>=|<|>|!)\s*\("),
    "template_bracket": re.compile(r"\w(?P<tok><)\w"),
    "unspaced_and_or": re.compile(r"\w(?P<tok>&&) \w"),
    "brace": re.compile(r"(?P<tok>!)\w+\(\[&\]\s*\{"),
    "case": re.compile(r"(?m)^[ \t]*case [^:\n]*? (?P<tok>\+|" + _BINARY + r") "),
    "template": re.compile(r"(?m)^[ \t]*template\s*<[^;{]*\{[^{}]*? (?P<tok>==|!=) "),
    "constexpr": re.compile(r"\bconstexpr\b[^;{}]*(?:\{[^{}]*?)? (?P<tok>" + _BINARY + r") "),
    "consteval": re.compile(r"\bconsteval\b[^;{}]*? (?P<tok>" + _BINARY + r") "),
    "static_assert": re.compile(r"\bstatic_assert\([^;]*? (?P<tok>" + _BINARY + r") "),
    "requires": re.compile(r"\brequires\b[^;{}]*? (?P<tok>&&|\|\|) "),
    "noexcept": re.compile(r"\bnoexcept\([^;{})]*? (?P<tok>" + _BINARY + r") "),
    "enum": re.compile(r"\benum\b[^;{]*\{[^{}]*?\w (?P<tok>\+) 1\b"),
    "array_size": re.compile(
        r"\b(?!return\b|throw\b|else\b|new\b|delete\b|case\b|do\b)[A-Za-z_]\w* \w+\[[^\]\n]*?\w (?P<tok>\+) 1\b"),
    "const_init": re.compile(r"\bconst\b[^;=(){}]*= [^;{}]*? (?P<tok>==|!=|&&|\|\|) "),
    "bit_field": re.compile(r"(?m)^[ \t]*[\w:]+ \w+ : \w+ (?P<tok>\+) 1;"),
    "no_bound": None,  # every statement in a file that compiles ends in ; or a bracket
    # A comparison whose right side starts with a cast of an address, `(T)&x`.
    "operand_unknown": re.compile(r"(?P<tok>==|!=|<=|>=|<|>) \(\w+\)&\w"),
    # A second equality right after a first, as in `a == b != c`.
    "chained_compare": re.compile(r"[\w)\]] (?:==|!=) [\w.]+ (?P<tok>==|!=) "),
}


def tracked_cpp(*folders):
    globs = [f"{d}/*{ext}" for d in folders for ext in (".cpp", ".h")]
    out = subprocess.run(["git", "ls-files", *globs], cwd=REPO, capture_output=True, text=True, check=True)
    return [REPO / f for f in out.stdout.split()]


def src_files():
    return tracked_cpp("src")


class FinderOnRealFiles(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.precheck = find.default_precheck()
        cls.files = {}
        for p in src_files():
            cls.files[p] = (find.read_source(p), find.code_mask(p, cls.precheck), find.find_spots(p, cls.precheck))
        if not cls.files:
            raise AssertionError("git ls-files found no C++ files under src/")

    def each_spot(self):
        for p, (text, mask, spots) in self.files.items():
            for s in spots:
                yield p, text, mask, s

    def test_every_spot_is_in_code(self):
        for p, text, mask, s in self.each_spot():
            where = f"{p.name}:{s.line} spot {s.number}"
            self.assertEqual(text[s.offset:s.offset + s.length], s.original, where)
            self.assertEqual(mask[s.offset:s.offset + s.length], s.original,
                             f"{where}: the code mask blanks part of the spot, so it is not code")
            self.assertTrue(s.original.strip(), where)

    def test_every_change_comes_from_the_kinds_table(self):
        for p, text, mask, s in self.each_spot():
            self.assertIn(s.kind, find.KINDS, f"{p.name}:{s.line}")
            self.assertEqual(find.KINDS[s.kind]["swap"].get(s.original), s.replacement, f"{p.name}:{s.line}")

    def test_planted_expressions_are_balanced_and_hold_the_spot(self):
        pairs = {")": "(", "]": "[", "}": "{"}
        for p, text, mask, s in self.each_spot():
            if s.skip is not None:
                continue
            where = f"{p.name}:{s.line} spot {s.number}"
            self.assertLessEqual(s.expr_start, s.offset, where)
            self.assertGreaterEqual(s.expr_end, s.offset + s.length, where)
            expr = mask[s.expr_start:s.expr_end]
            self.assertEqual(expr, expr.strip(), f"{where}: the expression starts or ends in blank space")
            stack = []
            for ch in expr:
                if ch in "([{":
                    stack.append(ch)
                elif ch in pairs:
                    self.assertTrue(stack and stack.pop() == pairs[ch], f"{where}: unbalanced [{expr}]")
            self.assertEqual(stack, [], f"{where}: unbalanced [{expr}]")

    def test_planted_compare_operands_are_balanced_and_inside_the_expression(self):
        pairs = {")": "(", "]": "["}
        planted = 0
        for p, text, mask, s in self.each_spot():
            where = f"{p.name}:{s.line} spot {s.number}"
            bounds = (s.lhs_start, s.lhs_end, s.rhs_start, s.rhs_end)
            if s.skip is not None or s.kind != "compare":
                self.assertEqual(bounds, (None,) * 4, f"{where}: operand bounds on a spot that is no planted compare")
                continue
            planted += 1
            self.assertTrue(s.expr_start <= s.lhs_start < s.lhs_end <= s.offset, where)
            self.assertTrue(s.offset + s.length <= s.rhs_start < s.rhs_end <= s.expr_end, where)
            self.assertEqual(mask[s.lhs_end:s.offset].strip(), "", f"{where}: code between the left side and the operator")
            self.assertEqual(mask[s.offset + s.length:s.rhs_start].strip(), "",
                             f"{where}: code between the operator and the right side")
            for a, b in ((s.lhs_start, s.lhs_end), (s.rhs_start, s.rhs_end)):
                side = mask[a:b]
                self.assertTrue(side.strip(), f"{where}: an empty side")
                self.assertEqual(side, side.strip(), f"{where}: a side starts or ends in blank space")
                stack = []
                for ch in side:
                    if ch in "([{":
                        stack.append(ch)
                    elif ch in pairs:
                        self.assertTrue(stack and stack.pop() == pairs[ch], f"{where}: unbalanced side [{side}]")
                self.assertEqual(stack, [], f"{where}: unbalanced side [{side}]")
        self.assertGreater(planted, 0, "no compare spot under src/ was planted")

    def test_no_planted_spot_touches_a_preprocessor_line(self):
        for p, (text, mask, spots) in self.files.items():
            lines = mask.split("\n")
            directive = set()
            for n, line in enumerate(lines, 1):
                if line.lstrip().startswith("#") or (n > 1 and n - 1 in directive and lines[n - 2].rstrip().endswith("\\")):
                    directive.add(n)
            for s in spots:
                if s.skip is not None:
                    continue
                first = text.count("\n", 0, s.expr_start) + 1
                last = text.count("\n", 0, s.expr_end) + 1
                for n in range(first, last + 1):
                    self.assertNotIn(n, directive, f"{p.name}:{n} holds planted spot {s.number}")

    def test_every_spot_is_planted_or_has_a_reason(self):
        for p, text, mask, s in self.each_spot():
            where = f"{p.name}:{s.line} spot {s.number}"
            if s.skip is None:
                self.assertIsNotNone(s.expr_start, where)
                self.assertIsNotNone(s.expr_end, where)
            else:
                self.assertIn(s.skip, find.SKIP_REASONS, where)
                self.assertIsNone(s.expr_start, where)
                self.assertIsNone(s.expr_end, where)

    def test_numbers_run_in_file_order(self):
        for p, (text, mask, spots) in self.files.items():
            self.assertEqual([s.number for s in spots], list(range(1, len(spots) + 1)), p.name)
            self.assertEqual([s.offset for s in spots], sorted(s.offset for s in spots), p.name)

    def test_same_file_twice_gives_the_same_list(self):
        for p, (text, mask, spots) in self.files.items():
            again = find.find_spots(p, self.precheck)
            self.assertEqual([s.as_json() for s in again], [s.as_json() for s in spots], p.name)

    _more = None

    @classmethod
    def more_files(cls):
        """The C++ files under tests/ and tools/, read only when src/ lacks a rule's shape."""
        if cls._more is None:
            cls._more = {}
            for p in tracked_cpp("tests", "tools"):
                cls._more[p] = (find.read_source(p), find.code_mask(p, cls.precheck),
                                find.find_spots(p, cls.precheck))
        return cls._more

    @staticmethod
    def search_hits(search, files):
        """(file, line, the finder's spot at the token or None) for each code match of search."""
        hits = []
        for p, (text, mask, spots) in files.items():
            at = {s.offset: s for s in spots}
            for m in search.finditer(text):
                off = m.start("tok")
                if mask[off:m.end("tok")] != m.group("tok"):
                    continue  # the match lies in a comment or a string
                hits.append((p, text.count("\n", 0, off) + 1, at.get(off)))
        return hits

    def test_every_skip_rule_skips_its_real_lines(self):
        self.assertEqual(set(SEARCHES), set(find.SKIP_REASONS),
                         "each skip reason needs a search for its real lines in SEARCHES")
        for reason, search in SEARCHES.items():
            with self.subTest(reason=reason):
                if search is None:
                    self.skipTest(f"{reason}: no real line can be searched for")
                hits = self.search_hits(search, self.files)
                if not hits:
                    hits = self.search_hits(search, self.more_files())
                if not hits:
                    self.skipTest(f"{reason}: no real line in src/, tests/ or tools/ has this shape")
                for p, line, spot in hits:
                    self.assertIsNotNone(spot, f"{p.name}:{line}: the finder lists no spot at the token")
                    self.assertIsNotNone(spot.skip, f"{p.name}:{line}: planted spot {spot.number} where "
                                                    f"the {reason} rule should skip it")
                self.assertIn(reason, {spot.skip for _, _, spot in hits},
                              f"{reason}: none of its {len(hits)} real lines was skipped for this reason")


if __name__ == "__main__":
    unittest.main()
