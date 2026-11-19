"""The spot finder: where in a C++ file hydra_mutate can plant a bug.

A spot is one place where one small change makes a planted bug, such as `<`
becoming `<=`. The finder reads a C++ source file, asks
`hydra_precheck --code-mask` (codelex, the repo's one lexer) which characters
are code, and applies the fixed kinds in KINDS below. No judgment picks a
spot: the same file always gives the same numbered list.

Each spot is either planted or skipped. A planted spot carries the bounds of
the smallest whole expression around it, so the rewriter (rewrite.py, task M2)
can wrap that expression in `HYDRA_MUT(n, <original>, <changed>)` without
changing what binds to what. A spot the walk cannot bound safely is skipped,
never guessed, with a reason key from SKIP_REASONS.

    py -I tools/mutate/find.py src/core/stars.cpp [more files...]
    py -I tools/mutate/find.py --precheck <hydra_precheck.exe> --root <repo> <files>

The default precheck is the main checkout's build-cpp/Release/hydra_precheck.exe
(main_checkout() finds it); ask the main session to build the hydra_precheck
target if it is missing. Only C++ files are read; Python has its own half in
py_mutate.py (task M6).

The spot format (FORMAT 1). Kept stable: the rewriter reads it.

The command prints one JSON object:

    {"format": 1,
     "kinds": {<kind>: <words>, ...},            from KINDS
     "skip_reasons": {<reason>: <words>, ...},   from SKIP_REASONS
     "files": [{"file": <path>, "spots": [<spot>, ...]}, ...]}

`file` is the path relative to --root (default: the checkout holding this
script) with forward slashes, or the absolute path when the file lies outside
it. Each spot is an object with these keys:

    number      1, 2, 3 ... in file order, counting planted and skipped spots
                alike. Numbers are per file; the rewriter gives the global
                switch numbers.
    line        1-based line of the spot's first character.
    offset      where the changed text starts, counted in characters of the
                file decoded as UTF-8 with no newline translation (Python's
                open(path, encoding="utf-8", newline="")).
    length      how many characters the changed text covers.
    kind        a key of KINDS.
    original    the file's text at [offset, offset + length).
    replacement the text that replaces it when the bug is switched on.
    change      the change in words, for the report.
    expr_start  start offset of the enclosing whole expression; the spot lies
                inside [expr_start, expr_end). null when skipped.
    expr_end    end offset (exclusive) of that expression. null when skipped.
                The expression never starts or ends in whitespace.
    lhs_start   for a planted compare spot, the bounds of its two operands:
    lhs_end     [lhs_start, lhs_end) is the left one and [rhs_start, rhs_end)
    rhs_start   the right one, found by walking out from the operator by C++
    rhs_end     precedence (_File.operands). Both lie inside the expression
                and never start or end in whitespace. null for every other
                spot. The rewriter writes a compare spot's change as a call to
                a helper in hydra_mutant.h on these two operands, because a
                type may define == but not != (or < but not >=).
    skip        null for a planted spot, otherwise a key of SKIP_REASONS.

find_spots(path) returns the same spots as Spot objects for Python callers;
Spot.as_json() is one entry of the list above.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Optional

FORMAT = 1

# The kinds of bug, one table. Each kind maps the token it matches to the
# token it becomes. The first four are the brief's (session 88ea48bb plan,
# "What the tool does", step 1); plus_one is the old probe's off-by-one edit.
# drop_not writes `(bool)` in place of `!`, not nothing, so the expression
# keeps its type: `!ptr` is a bool and `ptr` is not, and the two sides of
# HYDRA_MUT must have one type. A cast binds like `!`, so nothing regroups.
KINDS = {
    "compare": {
        "words": "flip a comparison",
        "swap": {"<": "<=", "<=": "<", ">": ">=", ">=": ">", "==": "!=", "!=": "=="},
    },
    "and_or": {
        "words": "swap and and or",
        "swap": {"&&": "||", "||": "&&"},
    },
    "drop_not": {
        "words": "drop a not",
        "swap": {"!": "(bool)"},
    },
    "plus_one": {
        "words": "a binary + 1 becomes - 1",
        "swap": {"+": "-"},
    },
    "return_bool": {
        "words": "flip a returned true or false",
        "swap": {"true": "false", "false": "true"},
    },
}

# Why a spot was skipped, one table. Every skipped spot carries one key.
SKIP_REASONS = {
    "preprocessor": "on a preprocessor line, or its expression spans one",
    "operator_name": "the name of an overloaded operator, as in operator==",
    "template_bracket": "a < or > without a space on both sides, likely a template bracket",
    "unspaced_and_or": "&& or || without a space on both sides, likely a reference declarator",
    "brace": "a brace inside or right beside the expression (a lambda or a braced initializer)",
    "case": "in a case label, which needs a constant",
    "template": "inside a template, which may need a constant",
    "constexpr": "inside constexpr code, which needs a constant",
    "consteval": "inside consteval code, which needs a constant",
    "static_assert": "inside a static_assert, which needs a constant",
    "requires": "inside a requires clause, which needs a constant",
    "noexcept": "inside a noexcept condition, which needs a constant",
    "enum": "inside an enum, whose values need a constant",
    "array_size": "in an array size, which needs a constant",
    "const_init": "in the initializer of a const, which may be used as a constant",
    "bit_field": "in a bit-field width, which needs a constant",
    "no_bound": "the walk reached the start or end of the file without a bound",
    "operand_unknown": "a comparison's side holds a token the operand walk cannot place, or comes out empty",
    "chained_compare": "a comparison of the same level sits to its left, so its left side is itself a comparison",
}

CPP_SUFFIXES = {".cpp", ".cc", ".cxx", ".h", ".hpp", ".hh", ".hxx", ".inl"}

# Words and punctuation of code that codelex has already masked. This only
# splits code into tokens; codelex alone decides what is a comment or a string.
_TOKEN = re.compile(
    r"(?P<word>[A-Za-z_]\w*)"
    r"|(?P<num>\.?\d(?:[eEpP][+-]|['\w.])*)"
    r"|(?P<punct><=>|<<=|>>=|->\*|\.\.\.|::|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||"
    r"\+=|-=|\*=|/=|%=|&=|\|=|\^=|\.\*|##|[^\s\w])"
)

# A preprocessor line with its backslash continuations; the old probe's rule.
_PREPROCESSOR = re.compile(r"(?m)^[ \t]*#(?:[^\n]*\\\r?\n)*[^\n]*")

_ASSIGN = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="}
_STOP_PUNCT = {",", ";", "?", ":"} | _ASSIGN
_STOP_WORDS = {"return", "throw", "co_return", "co_yield"}
_ANGLE = {"<", ">", "<<", ">>", "<=", ">=", "<=>"}
# Keywords whose presence in a spot's statement means it may need a constant.
_STATEMENT_KEYWORDS = ("template", "constexpr", "consteval", "static_assert", "requires", "noexcept", "enum")
# Keywords whose presence in the head of an enclosing brace block means the
# whole block may need constants.
_BLOCK_KEYWORDS = ("template", "constexpr", "consteval", "enum")
# The kind each matched token belongs to when it is an operator's own name.
_OPERATOR_KIND = {tok: kind for kind in ("compare", "and_or", "drop_not") for tok in KINDS[kind]["swap"]}
# Words that may sit right before `name[` without making it a declaration.
_NOT_A_TYPE = {"return", "throw", "co_return", "co_yield", "else", "do", "delete", "new", "case"}

# The operand walk (_File.operands). C++ precedence, from tightest down, is
# the standard's [expr] grammar: postfix and unary, .* ->*, * / %, + -,
# << >>, <=>, then < <= > >=, then == !=, then & ^ | && || ?: and the
# assignments and comma. Each token met at bracket depth 0 is put in one of
# these groups; a token in none of them skips the spot, never a guess.
_EQUALITY = {"==", "!="}
_RELATIONAL = {"<", "<=", ">", ">="}
# Looser than every comparison, so a side ends there. `&` is not here: it is
# binary and looser, or unary and part of the side (_File.amp_is_binary).
_LOOSER = {"&&", "||", "^", "|", "?", ":", ",", ";"} | _ASSIGN
# Tighter than every comparison, so part of a side. The quotes are what the
# code mask leaves of a string or character literal.
_TIGHTER_PUNCT = {".", "->", "::", "+", "-", "*", "/", "%", "<<", ">>", "<=>", "!", "~",
                  "++", "--", ".*", "->*", '"', "'"}
# Words that cannot sit inside a comparison's side in code that compiles as
# an expression; meeting one means the walk has left the expression.
_NOT_IN_OPERAND = {"else", "do", "if", "while", "for", "switch", "case", "default", "goto", "break",
                   "continue", "typedef", "using", "namespace", "template", "typename", "class",
                   "struct", "union", "enum", "public", "private", "protected", "operator", "auto",
                   "const", "volatile", "static", "extern", "inline", "constexpr", "consteval",
                   "constinit", "mutable", "register", "thread_local", "friend", "virtual",
                   "explicit", "asm", "concept", "requires", "static_assert"}
# Words that end an operand, as a name or literal does; any other word before
# `&` makes it unary.
_KEYWORD_BEFORE_UNARY = _STOP_WORDS | _NOT_IN_OPERAND | {"new", "delete", "sizeof", "alignof",
                                                         "co_await", "noexcept", "decltype"}


@dataclass
class Spot:
    number: int
    line: int
    offset: int
    length: int
    kind: str
    original: str
    replacement: str
    change: str
    expr_start: Optional[int]
    expr_end: Optional[int]
    skip: Optional[str]
    lhs_start: Optional[int] = None
    lhs_end: Optional[int] = None
    rhs_start: Optional[int] = None
    rhs_end: Optional[int] = None

    def as_json(self) -> dict:
        return asdict(self)


@dataclass
class _Tok:
    kind: str  # "word", "num" or "punct"
    text: str
    start: int
    end: int
    pp: bool  # on a preprocessor line


class FinderError(RuntimeError):
    pass


def main_checkout() -> Path:
    """The main checkout of the repo holding this script (a worktree's parent repo)."""
    here = Path(__file__).resolve().parent
    out = subprocess.run(
        ["git", "rev-parse", "--path-format=absolute", "--git-common-dir"],
        cwd=here, capture_output=True, text=True,
    )
    if out.returncode != 0:
        raise FinderError(f"find.py: git rev-parse failed: {out.stderr.strip()}")
    return Path(out.stdout.strip()).parent


def default_precheck() -> Path:
    return main_checkout() / "build-cpp" / "Release" / "hydra_precheck.exe"


def read_source(path: Path) -> str:
    with open(path, encoding="utf-8", newline="") as f:
        return f.read()


def code_mask(path: Path, precheck: Path) -> str:
    """hydra_precheck --code-mask's answer for `path`, checked to line up with the source."""
    if not precheck.is_file():
        raise FinderError(f"find.py: hydra_precheck.exe is missing at {precheck}. "
                          "Ask the main session to build the hydra_precheck target.")
    out = subprocess.run([str(precheck), "--code-mask", str(path)], capture_output=True)
    if out.returncode != 0:
        raise FinderError(f"find.py: hydra_precheck --code-mask failed on {path} "
                          f"(exit {out.returncode}): {out.stderr.decode('utf-8', 'replace').strip()}")
    masked = out.stdout.decode("utf-8")
    text = read_source(path)
    if len(masked) != len(text):
        raise FinderError(f"find.py: hydra_precheck --code-mask gave {len(masked)} characters for the "
                          f"{len(text)}-character {path}, so offsets would not line up")
    return masked


def _tokens(mask: str) -> list[_Tok]:
    pp_ranges = [(m.start(), m.end()) for m in _PREPROCESSOR.finditer(mask)]
    toks: list[_Tok] = []
    r = 0
    for m in _TOKEN.finditer(mask):
        while r < len(pp_ranges) and pp_ranges[r][1] <= m.start():
            r += 1
        pp = r < len(pp_ranges) and pp_ranges[r][0] <= m.start() < pp_ranges[r][1]
        toks.append(_Tok(m.lastgroup, m.group(), m.start(), m.end(), pp))
    return toks


def _spaced(mask: str, tok: _Tok) -> bool:
    return (tok.start > 0 and mask[tok.start - 1].isspace()
            and tok.end < len(mask) and mask[tok.end].isspace())


class _File:
    def __init__(self, text: str, mask: str):
        self.text = text
        self.mask = mask
        self.toks = _tokens(mask)
        # For each token, the index of the innermost unclosed `{` before it, or -1.
        self.block = [-1] * len(self.toks)
        self.block_parent: dict[int, int] = {}
        stack: list[int] = []
        for i, t in enumerate(self.toks):
            self.block[i] = stack[-1] if stack else -1
            if t.text == "{":
                self.block_parent[i] = stack[-1] if stack else -1
                stack.append(i)
            elif t.text == "}" and stack:
                stack.pop()
        self._head_words: dict[int, set[str]] = {}

    def is_angle_unspaced(self, i: int) -> bool:
        t = self.toks[i]
        return t.text in _ANGLE and not _spaced(self.mask, t)

    # -- the bounding walk ------------------------------------------------

    def walk_left(self, i: int) -> tuple[Optional[int], Optional[str]]:
        """Index of the first token of the expression holding token i, or a skip reason."""
        depth = 0
        j = i - 1
        while j >= 0:
            t = self.toks[j]
            if t.pp:
                return None, "preprocessor"
            if self.is_angle_unspaced(j):
                return None, "template_bracket"
            x = t.text
            if x in (")", "]"):
                depth += 1
            elif x in ("(", "["):
                if depth == 0:
                    return j + 1, None
                depth -= 1
            elif x == "}":
                return None, "brace"
            elif x == "{":
                if depth == 0:
                    return j + 1, None
                return None, "brace"
            elif depth == 0 and t.kind == "word" and x == "case":
                return None, "case"
            elif depth == 0 and ((t.kind == "punct" and x in _STOP_PUNCT)
                                 or (t.kind == "word" and x in _STOP_WORDS)):
                return j + 1, None
            j -= 1
        return None, "no_bound"

    def walk_right(self, i: int) -> tuple[Optional[int], Optional[str]]:
        """Index of the last token of the expression holding token i, or a skip reason."""
        depth = 0
        j = i + 1
        while j < len(self.toks):
            t = self.toks[j]
            if t.pp:
                return None, "preprocessor"
            if self.is_angle_unspaced(j):
                return None, "template_bracket"
            x = t.text
            if x in ("(", "["):
                depth += 1
            elif x in (")", "]"):
                if depth == 0:
                    return j - 1, None
                depth -= 1
            elif x == "{":
                return None, "brace"
            elif x == "}":
                if depth == 0:
                    return j - 1, None
                return None, "brace"
            elif depth == 0 and t.kind == "punct" and x in _STOP_PUNCT:
                return j - 1, None
            j += 1
        return None, "no_bound"

    # -- the operand walk, for compare spots ----------------------------------

    def amp_is_binary(self, j: int) -> Optional[bool]:
        """Whether the `&` at index j is binary (True) or unary (False); None when neither is sure."""
        prev = self.toks[j - 1] if j > 0 else None
        if prev is not None and prev.text in ('"', "'", "++", "--"):
            return None  # a literal's quote or a postfix step may end an operand or not
        ends_operand = prev is not None and (
            (prev.kind == "word" and prev.text not in _KEYWORD_BEFORE_UNARY)
            or prev.kind == "num" or prev.text in (")", "]"))
        if not ends_operand:
            return False
        if _spaced(self.mask, self.toks[j]):
            return True
        return None  # `x&y` or `(T)&x`: binary and a cast look alike

    def _side_token(self, j: int, op: str, left: bool) -> str:
        """For the token at index j, at bracket depth 0 beside the comparison op:
        "in" (part of the side), "stop" (binds looser, so the side ends before it),
        "chained" (the same level, met on the left) or "unknown"."""
        t = self.toks[j]
        x = t.text
        if t.kind == "num":
            return "in"
        if t.kind == "word":
            if x in _STOP_WORDS:
                return "stop"
            return "unknown" if x in _NOT_IN_OPERAND else "in"
        if x in _LOOSER:
            return "stop"
        if x == "&":
            binary = self.amp_is_binary(j)
            return "unknown" if binary is None else ("stop" if binary else "in")
        if x in _EQUALITY or x in _RELATIONAL:
            if (x in _EQUALITY) != (op in _EQUALITY):
                # Relational binds tighter than equality.
                return "in" if x in _RELATIONAL else "stop"
            # The same level groups left to right: on the right the side ends
            # here, on the left the whole comparison before it would be the side.
            return "chained" if left else "stop"
        if x in _TIGHTER_PUNCT:
            return "in"
        return "unknown"

    def operands(self, i: int, first: int, last: int):
        """The two operands of the comparison at token i, inside the expression
        [first, last], as ((lhs first, lhs last), (rhs first, rhs last)) token
        indices; or (None, skip reason)."""
        op = self.toks[i].text
        sides = []
        for left in (True, False):
            depth = 0
            step = -1 if left else 1
            opens, closes = (("(", "["), (")", "]")) if not left else ((")", "]"), ("(", "["))
            j = i + step
            while first <= j <= last:
                x = self.toks[j].text
                if x in ("{", "}"):
                    return None, "brace"
                if x in opens:
                    depth += 1
                elif x in closes:
                    if depth == 0:
                        break
                    depth -= 1
                elif depth == 0:
                    c = self._side_token(j, op, left)
                    if c == "stop":
                        break
                    if c != "in":
                        return None, "chained_compare" if c == "chained" else "operand_unknown"
                j += step
            side = (j + 1, i - 1) if left else (i + 1, j - 1)
            if side[0] > side[1]:
                return None, "operand_unknown"
            sides.append(side)
        return (sides[0], sides[1]), None

    # -- the context checks -----------------------------------------------

    def statement_start(self, i: int) -> int:
        """Index of the first token of the statement holding token i."""
        j = i - 1
        while j >= 0 and self.toks[j].text not in (";", "{", "}"):
            j -= 1
        return j + 1

    def head_words(self, brace: int) -> set[str]:
        """The words in the head of the block opened by the `{` at index brace."""
        if brace in self._head_words:
            return self._head_words[brace]
        words: set[str] = set()
        j = brace - 1
        while j >= 0:
            t = self.toks[j]
            if t.text in (";", "{"):
                break
            if t.text == "}":
                # A braced member initializer (`a_{1}, b_(2) {`) belongs to the
                # head; any other `}` ends the previous block.
                nxt = self.toks[j + 1].text if j + 1 < len(self.toks) else ""
                if nxt not in (",", "{", ")"):
                    break
                level = 0
                while j >= 0:
                    if self.toks[j].text == "}":
                        level += 1
                    elif self.toks[j].text == "{":
                        level -= 1
                        if level == 0:
                            break
                    j -= 1
            elif t.kind == "word":
                words.add(t.text)
            j -= 1
        self._head_words[brace] = words
        return words

    def context_skip(self, i: int, first: int) -> Optional[str]:
        """A reason the spot at token i, in the expression starting at token first, needs a constant."""
        s = self.statement_start(i)
        prefix = self.toks[s:i]
        prefix_words = {t.text for t in prefix if t.kind == "word"}
        for k in _STATEMENT_KEYWORDS:
            if k in prefix_words:
                return k
        b = self.block[i]
        innermost = b
        while b >= 0:
            words = self.head_words(b)
            for k in _BLOCK_KEYWORDS:
                if k in words:
                    return k
            b = self.block_parent[b]
        if "const" in prefix_words and any(t.text == "=" for t in prefix):
            return "const_init"
        # Unclosed `[` openers in the statement before the spot: array sizes?
        stack: list[int] = []
        for j in range(s, i):
            x = self.toks[j].text
            if x in ("(", "["):
                stack.append(j)
            elif x in (")", "]") and stack:
                stack.pop()
        for j in stack:
            if self.toks[j].text == "[" and self._declares_array(j):
                return "array_size"
        if first > 0 and self.toks[first - 1].text == ":" and innermost >= 0:
            if self.head_words(innermost) & {"struct", "class", "union"} and self.block[first - 1] == innermost:
                return "bit_field"
        return None

    def _declares_array(self, j: int) -> bool:
        """Whether the `[` at index j follows `type name`, as an array declaration does."""
        k = j - 1
        while k >= 0 and self.toks[k].text == "]":
            level = 0
            while k >= 0:
                if self.toks[k].text == "]":
                    level += 1
                elif self.toks[k].text == "[":
                    level -= 1
                    if level == 0:
                        break
                k -= 1
            k -= 1
        if k < 0 or self.toks[k].kind != "word":
            return False
        k -= 1
        if k >= 0 and self.toks[k].text in ("*", "&", "&&"):
            k -= 1
        return k >= 0 and self.toks[k].kind == "word" and self.toks[k].text not in _NOT_A_TYPE

    # -- the kinds --------------------------------------------------------

    def candidates(self):
        """(token index, kind, unspaced skip reason or None) for each token a kind matches."""
        toks = self.toks
        for i, t in enumerate(toks):
            named = i > 0 and toks[i - 1].text == "operator"
            if t.kind == "punct" and named and t.text in _OPERATOR_KIND:
                yield i, _OPERATOR_KIND[t.text], "operator_name"
            elif t.kind == "punct" and t.text in KINDS["compare"]["swap"]:
                angle = t.text in _ANGLE
                yield i, "compare", "template_bracket" if angle and not _spaced(self.mask, t) else None
            elif t.kind == "punct" and t.text in KINDS["and_or"]["swap"]:
                yield i, "and_or", None if _spaced(self.mask, t) else "unspaced_and_or"
            elif t.kind == "punct" and t.text == "!":
                yield i, "drop_not", None
            elif (t.kind == "punct" and t.text == "+" and i + 1 < len(toks)
                  and toks[i + 1].kind == "num" and toks[i + 1].text == "1" and i > 0
                  and (toks[i - 1].kind in ("word", "num") or toks[i - 1].text in (")", "]"))
                  and toks[i - 1].text not in ("return", "case")):
                yield i, "plus_one", None
            elif (t.kind == "word" and t.text in KINDS["return_bool"]["swap"] and i > 0 and i + 1 < len(toks)
                  and toks[i - 1].text == "return" and toks[i + 1].text == ";"):
                yield i, "return_bool", None


def _line_of(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def spots_in(text: str, mask: str) -> list[Spot]:
    """The numbered spots of a source text, given its code mask."""
    f = _File(text, mask)
    spots: list[Spot] = []
    for i, kind, reason in f.candidates():
        t = f.toks[i]
        first = last = None
        if t.pp:
            reason = "preprocessor"
        if reason is None:
            first, reason = f.walk_left(i)
        if reason is None:
            last, reason = f.walk_right(i)
        if reason is None and first > i:
            reason = "no_bound"  # the spot itself would be the bound; never happens for a matched kind
        if reason is None:
            reason = f.context_skip(i, first)
        sides = None
        if reason is None and kind == "compare":
            sides, reason = f.operands(i, first, last)
        original = text[t.start:t.end]
        replacement = KINDS[kind]["swap"][t.text]
        planted = reason is None
        bounds = {}
        if planted and sides is not None:
            (l0, l1), (r0, r1) = sides
            bounds = dict(lhs_start=f.toks[l0].start, lhs_end=f.toks[l1].end,
                          rhs_start=f.toks[r0].start, rhs_end=f.toks[r1].end)
        spots.append(Spot(
            number=len(spots) + 1,
            line=_line_of(text, t.start),
            offset=t.start,
            length=t.end - t.start,
            kind=kind,
            original=original,
            replacement=replacement,
            change=f"`{original}` becomes `{replacement}`",
            expr_start=f.toks[first].start if planted else None,
            expr_end=f.toks[last].end if planted else None,
            skip=reason,
            **bounds,
        ))
    return spots


def find_spots(path, precheck: Optional[Path] = None) -> list[Spot]:
    """The numbered spots of the C++ file at path."""
    path = Path(path)
    if path.suffix.lower() not in CPP_SUFFIXES:
        raise FinderError(f"find.py: reads C++ files only, not {path}")
    mask = code_mask(path, Path(precheck) if precheck else default_precheck())
    return spots_in(read_source(path), mask)


def _shown_path(path: Path, root: Path) -> str:
    p = path.resolve()
    try:
        return p.relative_to(root.resolve()).as_posix()
    except ValueError:
        return p.as_posix()


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="List the spots hydra_mutate can plant a bug in.")
    ap.add_argument("files", nargs="+", type=Path)
    ap.add_argument("--precheck", type=Path, default=None,
                    help="hydra_precheck.exe (default: the main checkout's build-cpp/Release copy)")
    ap.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2],
                    help="paths in the output are relative to this folder")
    a = ap.parse_args(argv)
    precheck = a.precheck or default_precheck()
    try:
        files = [{"file": _shown_path(p, a.root), "spots": [s.as_json() for s in find_spots(p, precheck)]}
                 for p in a.files]
    except FinderError as e:
        print(e, file=sys.stderr)
        return 1
    out = {
        "format": FORMAT,
        "kinds": {k: v["words"] for k, v in KINDS.items()},
        "skip_reasons": SKIP_REASONS,
        "files": files,
    }
    sys.stdout.write(json.dumps(out, indent=1) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
