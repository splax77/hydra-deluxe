"""Give every seam hit its kind of flaw, join each test to its old record,
and apply the user's answers.

    py -I tools/blind_copy/kinds.py <copy> --seams <seams.tsv> --coverage <coverage.tsv>
        --matched <matched.tsv> --out <folder> [--repo <repo> --at <commit>]
        [--answers <answers.tsv>]

<copy> is the blind copy's root, the folder that holds code/. The rule this
script follows is "How a script gives each hit its kind" in the script-first
recheck plan (docs/superpowers/plans/2026-11-20-script-first-recheck.md). The
script is the one owner of reading a replacement's body. Which lines are hits,
and which test owns each, is grade_seams.grade(), read here through the
seams.tsv that find_seams.py writes. Which record is flagged is
match_records.py, read through matched.tsv.

It writes to <folder>:
  hits.tsv    one row per hit and what it was read through: the hook, the
              declared type read, the body lines read, the outcome, any mark
              and a note. A hit line has more than one row when it is a
              wrapper (one row per hit inside the wrapper's definition) or a
              helper's parameter (one row per caller).
  joined.tsv  one row per test with a hit: its record state, its result after
              the answers, its kinds, its newest record's lines 9 and 11
              quoted whole, and whether its file differs between the base
              commit and --at.
  kinds.md    one section per outcome, with its counts and its full list of
              tests; marked and unsorted bodies are listed by hit.
and prints the hit lines read, the rows written, the rows per outcome, the
hit lines per outcome and the marked rows. No count stops it.

answers.tsv has the columns `kind`, `answer` and `exceptions`. The answer is
one of ANSWERS. The exceptions are test names, separated by " | ", for which
the kind counts as having the other answer.
"""

import argparse
import collections
import csv
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import find_seams as fs  # noqa: E402
import grade_seams as gs  # noqa: E402
import match_records as mr  # noqa: E402

FAKE = "fake data"
WAIT = "forced wait"
FAIL = "forced failure"
MIDSTEP = "test code run mid-step"
KINDS = (FAKE, WAIT, FAIL, MIDSTEP)
NOTHING = "replaces nothing"
UNSORTED = "unsorted"
OUTCOMES = KINDS + (NOTHING, UNSORTED)

CONTRADICTED = "contradicted"
STANDS = "the record stands"
NOT_ANSWERED = "not answered"
ANSWERS = (CONTRADICTED, STANDS, NOT_ANSWERED)
NO_RECORD = "no record"
RECORD_STANDS = "stand-in found, record stands"
OPEN = "open"
BASE = "539c6de0"
SEP = " | "

DOWNLOADER = "set_downloader"
WAIT_CALL = re.compile(r"\bwait_\w*\s*\(|\bsleep_for\s*\(|\.wait\s*\(")
THROW = re.compile(r"\bthrow\b")
RETURN = re.compile(r"\breturn\b([^;]*);")
CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
KEYWORDS = {"if", "for", "while", "switch", "return", "throw", "sizeof", "catch", "decltype",
            "static_cast", "const_cast", "reinterpret_cast", "dynamic_cast", "alignof", "noexcept"}
MEMBER_DECL = re.compile(r"^\s*(?:const\s+)?(?:std::optional\s*<\s*ScopedHook\s*>|ScopedHook)\s+(\w+)\s*;")
HOOK_FN_HEAD = re.compile(r"^\s*(?:static\s+|inline\s+)*ScopedHook\s+(\w+)\s*\(")
HOOK_FROM_HELPER = re.compile(r"ScopedHook\s+\w+\s*=\s*([\w:]+)\s*\(")
WRAPPER = re.compile(r"^\s*(?:const\s+)?([A-Z]\w*)\s+\w+\s*(?:[;({]|$)")
USING = re.compile(r"\busing\s+(\w+)\s*=\s*([^;]+);")
OPEN_BR, CLOSE_BR = "([{", ")]}"
MAX_DEPTH = 6


class File:
    """One copy file's code with comments and literals blanked
    (grade_seams.code_marks), as one string with line offsets."""

    def __init__(self, path, text):
        self.path = path
        _, lines = gs.code_marks(text)
        self.lines = lines
        self.code = "\n".join(lines)
        self.starts = [0]
        for ln in lines[:-1]:
            self.starts.append(self.starts[-1] + len(ln) + 1)
        self._units = None
        self._text = text

    def line_of(self, pos):
        lo, hi = 0, len(self.starts) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if self.starts[mid] <= pos:
                lo = mid
            else:
                hi = mid - 1
        return lo + 1

    def units(self):
        if self._units is None:
            self._units = gs.top_units(self.path, self._text)
        return self._units

    def unit_at(self, line):
        for u in self.units():
            if u.start <= line <= u.end:
                return u
        return None


def close_of(s, i):
    """The index of the bracket that closes s[i], or -1."""
    stack = []
    for j in range(i, len(s)):
        c = s[j]
        if c in OPEN_BR:
            stack.append(CLOSE_BR[OPEN_BR.index(c)])
        elif c in CLOSE_BR:
            if not stack or stack.pop() != c:
                return -1
            if not stack:
                return j
    return -1


def open_before(s, i):
    """The index of the unclosed bracket that holds position i, or -1."""
    depth = 0
    for j in range(i - 1, -1, -1):
        c = s[j]
        if c in CLOSE_BR:
            depth += 1
        elif c in OPEN_BR:
            if depth == 0:
                return j
            depth -= 1
    return -1


def split_top(s, a, b):
    """Top-level comma spans of s[a:b]."""
    spans, depth, start = [], 0, a
    for j in range(a, b):
        c = s[j]
        if c in OPEN_BR or (c == "<" and False):
            depth += 1
        elif c in CLOSE_BR:
            depth -= 1
        elif c == "," and depth == 0:
            spans.append((start, j))
            start = j + 1
    if s[start:b].strip() or spans:
        spans.append((start, b))
    return spans


def angle_split(s):
    """Top-level comma parts of a declaration's parameter text, where < >
    also nest."""
    parts, depth, cur = [], 0, ""
    for c in s:
        if c in "(<[{":
            depth += 1
        elif c in ")>]}":
            depth -= 1
        if c == "," and depth == 0:
            parts.append(cur)
            cur = ""
        else:
            cur += c
    if cur.strip():
        parts.append(cur)
    return [p.strip() for p in parts]


def strip_move(e):
    m = re.fullmatch(r"\s*std::(?:move|forward\s*<[^>]*>)\s*\((.*)\)\s*", e, re.S)
    return m.group(1).strip() if m else e.strip()


def param_type(p):
    p = p.split("=", 1)[0].strip()
    m = re.fullmatch(r"(.*?[\w>&*\s])\s*\b([A-Za-z_]\w*)", p, re.S)
    if m and m.group(1).strip() and not m.group(1).strip().endswith("::") and \
            m.group(1).strip() not in ("const", "unsigned", "signed", "long", "short"):
        return m.group(1).strip()
    return p


class Row:
    def __init__(self, hit, via="", hook="", decl="", body="", outcome=UNSORTED, mark="", note=""):
        self.hit, self.via, self.hook, self.decl = hit, via, hook, decl
        self.body, self.outcome, self.mark, self.note = body, outcome, mark, note

    def copy(self, hit, via):
        return Row(hit, via, self.hook, self.decl, self.body, self.outcome, self.mark, self.note)


class Sorter:
    def __init__(self, copy, coverage, hit_lines):
        self.copy = copy
        self.coverage = coverage            # name -> "src/...:line"
        self.hit_lines = hit_lines          # set of (file, line)
        self.files = {}
        self._aliases = None
        self._headers = None

    # --- reading the copy ---
    def file(self, path):
        if path not in self.files:
            real = os.path.join(self.copy, "code", *path.split("/"))
            if not os.path.isfile(real):
                return None
            with open(real, encoding="utf-8", errors="replace") as f:
                self.files[path] = File(path, f.read())
        return self.files[path]

    def walk(self, root, suffixes):
        base = os.path.join(self.copy, "code", root)
        out = []
        for d, dirs, names in os.walk(base):
            dirs.sort()
            for n in sorted(names):
                if n.lower().endswith(suffixes):
                    out.append(os.path.relpath(os.path.join(d, n), os.path.join(self.copy, "code")).replace("\\", "/"))
        return out

    def aliases(self):
        if self._aliases is None:
            self._aliases = {}
            for p in self.walk("src", (".h", ".hpp")):
                for m in USING.finditer(self.file(p).code):
                    self._aliases.setdefault(m.group(1), " ".join(m.group(2).split()))
        return self._aliases

    def test_headers(self):
        if self._headers is None:
            self._headers = self.walk("tests", (".h", ".hpp"))
        return self._headers

    def paired(self, path):
        stem, ext = os.path.splitext(path)
        other = [".cpp"] if ext in (".h", ".hpp") else [".h", ".hpp"]
        return [stem + e for e in other if self.file(stem + e)]

    def find_units(self, name, near):
        """Definitions named `name` with a body, in `near`, its paired file
        and the tests/ headers."""
        out = []
        # unit_name() takes the head's first call, which for a return type
        # such as std::function<void()> is `void`, so the head is searched.
        head = re.compile(r"(?<![\w.>:])" + re.escape(name) + r"\s*\(|^\s*(?:struct|class)\s+" + re.escape(name) + r"\b")
        for p in dict.fromkeys([near] + self.paired(near) + self.test_headers()):
            f = self.file(p)
            if not f:
                continue
            for u in f.units():
                if "{" in u.code and (u.name == name or head.search(u.code.split("{", 1)[0].split(";", 1)[0])):
                    out.append((f, u))
        return out

    # --- step 1: the hook's declaration ---
    def declaration(self, hook):
        """(params text or None, returns void, declared type text)."""
        at = self.coverage.get(hook)
        if not at:
            return None
        path, line = at.rsplit(":", 1)
        f = self.file(path)
        if not f:
            return None
        pos = f.starts[int(line) - 1]
        m = re.compile(r"\b" + re.escape(hook) + r"\s*\(").search(f.code, pos)
        if not m:
            return None
        o = m.end() - 1
        c = close_of(f.code, o)
        if c < 0:
            return None
        params = " ".join(f.code[o + 1:c].split())
        head = f.code[f.starts[int(line) - 1]:m.start()]
        is_void = re.search(r"\bvoid\b", head) is not None
        return params, is_void

    def function_type(self, t):
        base = re.sub(r"\bconst\b|[&*]", "", t).strip()
        if base.startswith("std::function"):
            return True, base
        name = base.split("::")[-1].strip()
        seen = set()
        while name in self.aliases() and name not in seen:
            seen.add(name)
            base = self.aliases()[name]
            if base.startswith("std::function"):
                return True, f"{t} = {base}"
            name = base.split("::")[-1].strip()
        return False, base if base == t else f"{t} = {base}"

    # --- the whole rule for one hit line ---
    def sort(self, path, line, depth=0, via=""):
        hit = f"{path}:{line}"
        f = self.file(path)
        if f is None or depth > MAX_DEPTH:
            return [Row(hit, via, note="file not in the copy" if f is None else "followed too far")]
        text = f.lines[line - 1]
        tok = None
        for m in fs.WORD.finditer(text):
            if fs.SEAM_WORD.fullmatch(m.group(0)):
                tok = m
                break
        if tok is None:
            return self.sort_no_hook(f, line, depth, via)
        hook = tok.group(0)
        pos = f.starts[line - 1] + tok.start()
        after = re.compile(r"\s*").match(f.code, pos + len(hook)).end()
        nxt = f.code[after:after + 1]
        if nxt == "(":
            c = close_of(f.code, after)
            args = split_top(f.code, after + 1, c) if c > 0 else []
            repl = args[0] if args else None
        else:
            repl = self.wrapped_replacement(f, pos)
            if repl is None:
                return [Row(hit, via, hook=hook, note="the hook word is passed as an argument, not called")]
        return self.sort_hook(f, hit, via, hook, repl, depth)

    def wrapped_replacement(self, f, pos):
        """For a hook given to ScopedHook, emplace or a ScopedHook member's
        initialiser as its first argument: the span of the next argument."""
        o = open_before(f.code, pos)
        if o < 0 or f.code[o] not in "({":
            return None
        c = close_of(f.code, o)
        args = split_top(f.code, o + 1, c) if c > 0 else []
        if not args or not (args[0][0] <= pos < args[0][1]):
            return None
        before = f.code[:o].rstrip()
        m = re.search(r"([A-Za-z_]\w*)$", before)
        if not m:
            return None
        ident, ahead = m.group(1), before[:m.start()].rstrip()
        ok = ident in ("ScopedHook", "emplace") or ahead.endswith("ScopedHook")
        if not ok:
            decl = re.compile(r"(?:std::optional\s*<\s*ScopedHook\s*>|ScopedHook)\s+" + re.escape(ident) + r"\s*;")
            ok = any(decl.search(g.code) for g in [f] + [self.file(p) for p in self.paired(f.path)])
        if not ok:
            return None
        return args[1] if len(args) > 1 else (c, c)

    def sort_hook(self, f, hit, via, hook, repl, depth):
        decl = self.declaration(hook)
        if decl is None:
            return [Row(hit, via, hook=hook, note="coverage.tsv gives no declaration that can be read")]
        params, is_void = decl
        parts = angle_split(params)
        first = param_type(parts[0]) if parts else ""
        shown = first or "(no parameter)"
        r = Row(hit, via, hook=hook, decl=shown)
        if not parts and not is_void:
            r.outcome, r.note = NOTHING, "a getter"
            return [r]
        if hook.startswith("fail_"):
            r.outcome, r.note = FAIL, "a fail_ switch"
            return [r]
        rtext = f.code[repl[0]:repl[1]].strip() if repl else ""
        if rtext == "{}":
            r.outcome, r.note = NOTHING, "the hook is cleared with {}"
            return [r]
        is_fn, resolved = self.function_type(first) if first else (False, "")
        r.decl = resolved or shown
        if not is_fn:
            r.outcome, r.note = FAKE, "a plain value"
            return [r]
        return self.sort_replacement(f, repl, r, depth)

    # --- step 3: the replacement's body ---
    def sort_replacement(self, f, span, r, depth):
        e = f.code[span[0]:span[1]]
        lead = len(e) - len(e.lstrip())
        e = e.strip()
        inner = strip_move(e)
        start = span[0] + lead + (e.find(inner) if inner != e else 0)
        if inner.startswith("["):
            return [self.sort_lambda(f, start, r)]
        m = re.fullmatch(r"([\w:]+)\s*\((.*)\)", inner, re.S)
        if m:
            name = m.group(1).split("::")[-1]
            found = self.find_units(name, f.path)
            for g, u in found:
                k = g.code.find("return", g.starts[u.start - 1])
                end = g.starts[u.end - 1] + len(g.lines[u.end - 1])
                rm = re.compile(r"\breturn\s*\[").search(g.code, g.starts[u.start - 1], end)
                if rm:
                    return [self.sort_lambda(g, rm.end() - 1, r)]
                del k
            r.note = f"the helper {name} has no definition that returns a lambda in reach" if not found \
                else f"the helper {name} returns no lambda"
            return [r]
        if re.fullmatch(r"[A-Za-z_]\w*", inner):
            return self.per_caller(f, f.line_of(start), inner, r, depth)
        r.note = "the replacement is neither a lambda, a helper call nor a parameter: " + " ".join(inner.split())[:80]
        return [r]

    def per_caller(self, f, line, name, r, depth):
        u = f.unit_at(line)
        if u is None or not u.name:
            r.note = f"{name} is not a parameter of a function around it"
            return [r]
        o = u.code.find("(")
        c = close_of(u.code, o) if o >= 0 else -1
        params = angle_split(u.code[o + 1:c]) if c > 0 else []
        idx = next((i for i, p in enumerate(params)
                    if re.search(r"\b" + re.escape(name) + r"\s*(?:=.*)?$", p, re.S)), None)
        if idx is None:
            r.note = f"{name} is a variable, not a parameter of {u.name}"
            return [r]
        rows = []
        call = re.compile(r"\b" + re.escape(u.name) + r"\s*\(")
        for p in self.walk("tests", tuple(gs_suffixes())):
            g = self.file(p)
            for m in call.finditer(g.code):
                ln = g.line_of(m.start())
                if p == f.path and u.start <= ln <= u.end and ln == u.start:
                    continue
                if p == f.path and ln == u.start:
                    continue
                cc = close_of(g.code, m.end() - 1)
                args = split_top(g.code, m.end(), cc) if cc > 0 else []
                if idx >= len(args):
                    continue
                rr = Row(r.hit, f"{p}:{ln}", r.hook, r.decl)
                rows += self.sort_replacement(g, args[idx], rr, depth + 1)
        if not rows:
            r.note = f"{name} is a parameter of {u.name}, which no test calls"
            return [r]
        return rows

    def sort_lambda(self, f, at, r):
        cap_close = close_of(f.code, at)
        captures = f.code[at + 1:cap_close]
        j = cap_close + 1
        ws = re.compile(r"\s*")
        j = ws.match(f.code, j).end()
        params = []
        if f.code[j:j + 1] == "(":
            pc = close_of(f.code, j)
            params = [re.findall(r"[A-Za-z_]\w*", p)[-1] for p in angle_split(f.code[j + 1:pc])
                      if re.findall(r"[A-Za-z_]\w*", p)]
            j = pc + 1
        b = f.code.find("{", j)
        bc = close_of(f.code, b)
        if b < 0 or bc < 0:
            r.note = "the lambda's body cannot be matched"
            return r
        body = f.code[b + 1:bc]
        r.body = f"{f.path}:{f.line_of(b)}-{f.line_of(bc)}"
        cap_names = set(re.findall(r"[A-Za-z_]\w*", re.sub(r"=\s*[^,]*", "", captures)))
        default_cap = bool(re.search(r"(?:^|,)\s*[&=]\s*(?:,|$)", captures))
        defined = {u.name for u in f.units()}
        classify(body, params, cap_names, default_cap, defined, r)
        return r

    # --- step 4: lines with no hook word ---
    def sort_no_hook(self, f, line, depth, via):
        hit = f"{f.path}:{line}"
        text = f.lines[line - 1]
        if "selected->" in text and re.search(r"selected->\w+\s*=[^=]", text):
            return [Row(hit, via, hook="selected->", outcome=FAKE, note="a selected-> assignment")]
        m = MEMBER_DECL.match(text)
        if m:
            return self.member(f, line, m.group(1), depth, via)
        m = HOOK_FN_HEAD.match(text)
        if m and "=" not in text.split("(")[0]:
            return self.from_hook_helper(f, line, m.group(1), f.path, depth, via)
        m = HOOK_FROM_HELPER.search(self.statement(f, line))
        if m:
            return self.from_hook_helper(f, line, m.group(1).split("::")[-1], f.path, depth, via)
        if re.search(r"\bScopedHook\b", text):
            st_start = f.starts[line - 1]
            st = self.statement(f, line)
            for mm in fs.WORD.finditer(st):
                if fs.SEAM_WORD.fullmatch(mm.group(0)):
                    target = f.line_of(st_start + mm.start())
                    return [x.copy(hit, via or x.hit) for x in self.sort(f.path, target, depth + 1)]
            return [Row(hit, via, note="a ScopedHook statement with no hook word")]
        m = WRAPPER.match(text)
        if m:
            return self.wrapper(f, line, m.group(1), depth, via)
        return [Row(hit, via, note="no hook word and no shape the rule names")]

    def statement(self, f, line):
        s = f.starts[line - 1]
        depth = 0
        for j in range(s, len(f.code)):
            c = f.code[j]
            if c in OPEN_BR:
                depth += 1
            elif c in CLOSE_BR:
                depth -= 1
                if depth < 0:
                    return f.code[s:j]
            elif c == ";" and depth == 0:
                return f.code[s:j]
        return f.code[s:]

    def member(self, f, line, name, depth, via):
        hit = f"{f.path}:{line}"
        init = re.compile(r"\b" + re.escape(name) + r"\s*(?:\.emplace\s*\(|[({])")
        u = f.unit_at(line)
        places = []
        if u is not None:
            places.append((f, f.starts[u.start - 1], f.starts[u.end - 1] + len(f.lines[u.end - 1]), u.name))
        for p in self.paired(f.path):
            g = self.file(p)
            for gu in g.units():
                if u is not None and gu.name == u.name and re.search(r"\b" + re.escape(u.name) + r"\s*::", gu.code):
                    places.append((g, g.starts[gu.start - 1], g.starts[gu.end - 1] + len(g.lines[gu.end - 1]), gu.name))
        for g, a, b, _ in places:
            for m in init.finditer(g.code, a, b):
                ln = g.line_of(m.start())
                if g is f and ln == line:
                    continue
                o = m.end() - 1
                c = close_of(g.code, o)
                args = split_top(g.code, o + 1, c) if c > 0 else []
                if not args:
                    continue
                first = g.code[args[0][0]:args[0][1]].strip()
                word = first.split("::")[-1]
                if not fs.SEAM_WORD.fullmatch(word) or word not in self.coverage:
                    return [Row(hit, via or f"{g.path}:{ln}",
                                note=f"the member's initialiser gives it {first}, not a hook word")]
                target = g.line_of(args[0][0] + g.code[args[0][0]:args[0][1]].find(word))
                return [x.copy(hit, via or x.hit) for x in self.sort(g.path, target, depth + 1)]
        return [Row(hit, via, note=f"no initialiser of the member {name} in its class")]

    def from_hook_helper(self, f, line, name, near, depth, via):
        hit = f"{f.path}:{line}"
        for g, u in self.find_units(name, near):
            a = g.starts[u.start - 1]
            b = g.starts[u.end - 1] + len(g.lines[u.end - 1])
            m = re.compile(r"\breturn\s+ScopedHook\s*[({]").search(g.code, a, b)
            if not m:
                continue
            c = close_of(g.code, m.end() - 1)
            args = split_top(g.code, m.end(), c) if c > 0 else []
            if not args:
                continue
            w = g.code[args[0][0]:args[0][1]].strip().split("::")[-1]
            if fs.SEAM_WORD.fullmatch(w):
                target = g.line_of(args[0][0] + g.code[args[0][0]:args[0][1]].find(w))
                return [x.copy(hit, via or x.hit) for x in self.sort(g.path, target, depth + 1)]
        return [Row(hit, via, note=f"the helper {name} returns no ScopedHook of a hook word")]

    def wrapper(self, f, line, name, depth, via):
        hit = f"{f.path}:{line}"
        rows, seen = [], set()
        for g, u in self.find_units(name, f.path):
            ranges = [(g, u.start, u.end)]
            for p in self.paired(g.path):
                h = self.file(p)
                for hu in h.units():
                    if re.search(r"\b" + re.escape(name) + r"\s*::", hu.code.split("{", 1)[0]):
                        ranges.append((h, hu.start, hu.end))
            for h, a, b in ranges:
                for (hp, hn) in sorted(self.hit_lines):
                    if hp == h.path and a <= hn <= b and (hp, hn) != (f.path, line):
                        for x in self.sort(hp, hn, depth + 1):
                            key = (x.hit if not x.via else x.via, x.outcome, x.body)
                            if key in seen:
                                continue
                            seen.add(key)
                            rows.append(x.copy(hit, via or x.via or x.hit))
            if rows:
                break
        if not rows:
            return [Row(hit, via, note=f"{name} is no wrapper with a hit inside its definition")]
        return rows


def gs_suffixes():
    import facts_format as ff
    return ff.CODE_SUFFIXES


def classify(body, params, cap_names, default_cap, defined, r):
    """Step 3's five lines, in order, with the marks."""
    text = " ".join(body.split())
    # A lambda inside the body, such as a wait's condition, is not the
    # replacement's own code: its returns and calls are left out.
    body = blank_nested(body)
    stmts = [s.strip() for s in split_statements(body) if s.strip()]
    only_throw = len(stmts) == 1 and stmts[0].startswith("throw")
    rets = [x.strip() for x in RETURN.findall(body)]
    real, captured, other = [], [], []
    for e in rets:
        e2 = strip_move(e)
        m = re.fullmatch(r"(hydra::[\w:]+)\s*\((.*)\)", e2, re.S)
        if m:
            args = [strip_move(a) for a in angle_split(m.group(2))]
            if all(a in params for a in args):
                real.append(e)
                continue
        m = re.fullmatch(r"([A-Za-z_]\w*)\s*\(.*\)", e2, re.S)
        if m and (m.group(1) in cap_names or (default_cap and m.group(1) not in defined)):
            captured.append(e)
            continue
        other.append(e)
    waits = WAIT_CALL.search(body) is not None
    throws = THROW.search(body) is not None
    line2 = bool(other) or (r.hook == DOWNLOADER and not only_throw)
    others = sorted({m.group(1) for m in CALL.finditer(body) if is_plain_call(body, m)
                     and not WAIT_CALL.match(body, m.start()) and m.group(1) not in KEYWORDS
                     and not m.group(1).startswith(("wait_", "sleep_for"))})
    real_calls = {re.match(r"hydra::[\w:]*?(\w+)\s*\(", strip_move(e)).group(1) for e in real}
    others = [o for o in others if o not in real_calls]
    marks = []
    if only_throw:
        r.outcome, r.note = FAIL, "the body is one throw"
    elif captured:
        r.outcome, r.note = UNSORTED, "it returns the answer of a name the lambda captured: return " + captured[0]
    elif line2:
        r.outcome = FAKE
        r.note = ("a downloader whose body does more than throw" if not other
                  else "it returns a value that is not the real function's answer: return " + other[0])
        if waits:
            marks.append("waits, then hands back a test value")
        if throws:
            marks.append("can also throw")
    elif waits:
        r.outcome, r.note = WAIT, "it waits"
        if real:
            r.note = "it waits, then returns the real function's answer"
        if throws:
            marks.append("waits, then throws")
        if others:
            marks.append("waits and also calls " + ", ".join(others))
    elif len(stmts) == 1 and real:
        r.outcome, r.note = NOTHING, "it only returns the real function's answer"
    elif real:
        r.outcome, r.note = UNSORTED, "it returns the real function's answer and does more, with no wait"
    elif throws:
        r.outcome, r.note = UNSORTED, "it throws and does more, with no wait"
    else:
        r.outcome, r.note = MIDSTEP, "a void body that neither waits nor throws"
    r.mark = "; ".join(marks)
    del text


def is_plain_call(s, m):
    """A free call: not a member, not qualified and not a declaration."""
    k = m.start() - 1
    while k >= 0 and s[k] in " \t\n":
        k -= 1
    if k < 0:
        return True
    if s[k] in ".:>" or (s[k] == ">" and k > 0 and s[k - 1] == "-"):
        return False
    if s[k].isalnum() or s[k] == "_":
        word = re.search(r"(\w+)$", s[:k + 1]).group(1)
        return word in ("return", "throw", "else", "do")
    return True


def blank_nested(body):
    """The body with each nested lambda's braces emptied."""
    s = list(body)
    i = 0
    while True:
        i = body.find("[", i)
        if i < 0:
            return "".join(s)
        c = close_of(body, i)
        if c < 0:
            return "".join(s)
        j = re.compile(r"\s*").match(body, c + 1).end()
        if body[j:j + 1] == "(":
            pc = close_of(body, j)
            j = pc + 1 if pc > 0 else j
        m = re.compile(r"\s*(?:mutable\s*)?(?:->\s*[\w:<>,\s&*]+?)?\s*\{").match(body, j)
        if m:
            b = m.end() - 1
            bc = close_of(body, b)
            if bc > 0:
                for k in range(b + 1, bc):
                    if s[k] != "\n":
                        s[k] = " "
                i = bc
                continue
        i = c + 1


def split_statements(body):
    out, depth, cur = [], 0, ""
    for c in body:
        if c in OPEN_BR:
            depth += 1
        elif c in CLOSE_BR:
            depth -= 1
        if c == ";" and depth == 0:
            out.append(cur)
            cur = ""
        elif c == "}" and depth == 0:
            out.append(cur + c)
            cur = ""
        else:
            cur += c
    if cur.strip():
        out.append(cur)
    return out


# --- reading and writing tables ---
def read_tsv(path):
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f, delimiter="\t", quoting=csv.QUOTE_NONE))


def write_tsv(path, header, rows):
    fs.write_tsv(path, header, [[cell(c) for c in r] for r in rows])


def cell(c):
    return " ".join(str(c).replace("\t", " ").split("\n")) if c is not None else ""


HIT_COLUMNS = ("hit", "via", "hook", "declared type", "body read", "outcome", "mark", "note", "status")
JOINED_COLUMNS = ("test", "test file", "test line", "state", "result", "kinds", "outcomes", "hits",
                  "records", "line 9", "line 11", "file changed since " + BASE, "status")


def sort_all(copy, seams, coverage):
    lines = sorted({(r["hit_file"], int(r["hit_line"])) for r in seams})
    cov = {c["name"]: c["declared_at"] for c in coverage}
    s = Sorter(copy, cov, set(lines))
    rows = []
    for p, n in lines:
        rows += s.sort(p, n)
    return lines, rows


def changed_files(repo, base, at):
    if not repo or not at:
        return None
    out = subprocess.run(["git", "-C", repo, "diff", "--name-only", base, at],
                         capture_output=True, text=True)
    if out.returncode != 0:
        raise RuntimeError(f"git diff {base} {at} failed: {out.stderr.strip()}")
    return set(out.stdout.split())


def read_answers(path):
    out = {}
    for a in read_tsv(path):
        k, ans = a["kind"].strip(), a["answer"].strip()
        if k not in KINDS:
            raise ValueError(f"answers.tsv names a kind that is not one of the four: {k!r}")
        if ans not in ANSWERS:
            raise ValueError(f"answers.tsv gives {k} an answer that is not one of {ANSWERS}: {ans!r}")
        names = {x.strip() for x in (a.get("exceptions") or "").split(SEP.strip()) if x.strip()}
        out[k] = (ans, names)
    return out


def answer_for(kind, test, answers):
    ans, names = answers.get(kind, (NOT_ANSWERED, set()))
    if test in names and ans != NOT_ANSWERED:
        return STANDS if ans == CONTRADICTED else CONTRADICTED
    return ans


def result(state, kinds, test, answers):
    """A flagged test's result through the answers; a replaces-nothing or
    unsorted hit never moves it."""
    if state != mr.FLAGGED or not answers:
        return state
    got = [answer_for(k, test, answers) for k in kinds if k in KINDS]
    if any(a == CONTRADICTED for a in got):
        return CONTRADICTED
    if got and all(a == STANDS for a in got):
        return RECORD_STANDS
    return state


def join(seams, rows, matched, changed, answers=None, base=BASE):
    by_hit = collections.defaultdict(list)
    for r in rows:
        by_hit[r.hit].append(r)
    tests = collections.OrderedDict()
    for s in seams:
        if s["test_case"] != "yes":
            continue
        key = (s["test"], s["test_file"])
        t = tests.setdefault(key, {"line": s["test_line"], "hits": []})
        h = f"{s['hit_file']}:{s['hit_line']}"
        if h not in t["hits"]:
            t["hits"].append(h)
    recs = collections.defaultdict(list)
    for m in matched:
        if m["test name"]:
            recs[(m["test name"], m["test file"].removeprefix("code/"))].append(m)
    out = []
    for (name, file), t in tests.items():
        outcomes = []
        for h in t["hits"]:
            for r in by_hit.get(h, []):
                if r.outcome not in outcomes:
                    outcomes.append(r.outcome)
        kinds = [k for k in KINDS if k in outcomes]
        mine = recs.get((name, file), [])
        newest = [m for m in mine if m["newest copy"] in ("yes", "tie")] or mine
        state = newest[0]["test state"] if newest else NO_RECORD
        res = result(state, kinds, name, answers)
        diff = "" if changed is None else ("yes" if file in changed else "no")
        out.append([name, file, t["line"], state, res, SEP.join(kinds),
                    SEP.join(o for o in OUTCOMES if o in outcomes), SEP.join(t["hits"]),
                    SEP.join(m["record"] for m in newest),
                    " || ".join(m["line 9"] for m in newest), " || ".join(m["line 11"] for m in newest),
                    diff, OPEN])
    return out


def kinds_md(rows, joined):
    out = ["# Kinds of flaw", ""]
    for o in OUTCOMES:
        mine = [r for r in rows if r.outcome == o]
        tests = [j for j in joined if o in j[6].split(SEP)]
        flagged = [j for j in tests if j[3] == mr.FLAGGED]
        out += [f"## {o}", "", f"hit rows: {len(mine)}",
                f"hit lines: {len({r.hit for r in mine})}",
                f"tests: {len(tests)}", f"flagged records: {len(flagged)}", ""]
        for j in tests:
            out.append(f"- test: {j[0]} | {j[1]}:{j[2]} | {j[3]}")
        listed = [r for r in mine if o == UNSORTED or r.mark]
        if listed:
            out.append("")
        for r in listed:
            out.append(f"- hit: {r.hit}" + (f" via {r.via}" if r.via else "") +
                       f" | body: {r.body or 'none read'} | " + (f"mark: {r.mark} | " if r.mark else "") +
                       f"note: {r.note}")
        out.append("")
    return "\n".join(out)


def summary(lines, rows):
    out = [f"hit lines read: {len(lines)}", f"rows written: {len(rows)}"]
    for o in OUTCOMES:
        out.append(f"outcome: {o} | rows: {sum(1 for r in rows if r.outcome == o)} | "
                   f"hit lines: {len({r.hit for r in rows if r.outcome == o})}")
    out.append(f"hit lines with more than one outcome: "
               f"{sum(1 for h in {r.hit for r in rows} if len({x.outcome for x in rows if x.hit == h}) > 1)}")
    out.append(f"marked rows: {sum(1 for r in rows if r.mark)}")
    return "\n".join(out) + "\n"


def write_outputs(out, rows, joined):
    os.makedirs(out, exist_ok=True)
    write_tsv(os.path.join(out, "hits.tsv"), HIT_COLUMNS,
              [[r.hit, r.via, r.hook, r.decl, r.body, r.outcome, r.mark, r.note, OPEN] for r in rows])
    write_tsv(os.path.join(out, "joined.tsv"), JOINED_COLUMNS, joined)
    with open(os.path.join(out, "kinds.md"), "w", encoding="utf-8", newline="\n") as f:
        f.write(kinds_md(rows, joined))


def run(copy, seams_path, coverage_path, matched_path, out, repo=None, at=None, answers_path=None,
        base=BASE, say=None):
    say = say or sys.stdout.write
    seams = read_tsv(seams_path)
    coverage = read_tsv(coverage_path)
    matched = read_tsv(matched_path)
    lines, rows = sort_all(copy, seams, coverage)
    answers = read_answers(answers_path) if answers_path else None
    joined = join(seams, rows, matched, changed_files(repo, base, at), answers, base)
    write_outputs(out, rows, joined)
    say(summary(lines, rows))
    if answers:
        c = collections.Counter(j[4] for j in joined)
        say("results: " + ", ".join(f"{k} {c[k]}" for k in sorted(c)) + "\n")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    ap.add_argument("copy")
    ap.add_argument("--seams", required=True)
    ap.add_argument("--coverage", required=True)
    ap.add_argument("--matched", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--repo")
    ap.add_argument("--at", help="the commit each test file is compared with, against --base")
    ap.add_argument("--base", default=BASE)
    ap.add_argument("--answers")
    a = ap.parse_args(argv)
    try:
        return run(a.copy, a.seams, a.coverage, a.matched, a.out, a.repo, a.at, a.answers, a.base)
    except (ValueError, RuntimeError, OSError) as e:
        sys.stderr.write(f"kinds: {e}\n")
        return 2


if __name__ == "__main__":
    sys.exit(main())
