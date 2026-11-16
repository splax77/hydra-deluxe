"""The Python half of hydra_mutate: plant every bug at once, switch each on alone.

It copies a folder of Python code (tools/, or the hooks folder) into a run
folder, rewrites the copy so that each bug spot sits behind its own numbered
switch, and runs the folder's own unittest files against the copy. One run
with every switch off records which test reaches which switch. Then each
reached switch is turned on alone, in a fresh process, and only the test
modules holding a test that reaches it are run. The real folder is only
read, never written.

    py -I tools/mutate/py_mutate.py --target C:/Users/Patrick/.claude/gates --out <run folder>
    py -I tools/mutate/py_mutate.py --target tools --out <run folder> --mutate agent_cost.py --tests test_agent_cost

A wrapped spot reads `((changed) if _hm_switch(17) else (original))`. Python's
own parser (`ast`) finds the spots, so comments and strings never match. The
bug kinds are find.py's KINDS, read through AST_TOKEN and SPELLING below. A
spot the rewriter cannot place safely is skipped and listed with one of
SKIP_REASONS, never guessed.

What a test "caught" is decided only by unittest's own result for it in the
run with that one switch on.
"""

import argparse
import ast
import bisect
import concurrent.futures
import fnmatch
import importlib.util
import inspect
import json
import os
import re
import shutil
import subprocess
import sys
import time
import traceback
import unittest

SKIP_REASONS = {
    "fstring": "inside an f-string or t-string, whose inner positions are not plain source",
    "pattern": "inside a match pattern, which takes no expression",
    "token": "the operator is not found exactly once between its operands",
}

FILE_SKIP_REASONS = {
    "parse": "the file does not parse as Python, or is not UTF-8",
}

# The name the rewritten code calls, and the module that holds the switches.
SWITCH_NAME = "_hm_switch"
RUNTIME_MODULE = "hydra_mutant"

# The switches' runtime, the same in every process: HYDRA_MUTANT names the
# one live switch (none when unset), as in tools/mutate/hydra_mutant.h. A
# process started by a test (a script run through subprocess) writes the
# switches it reached to the file HYDRA_MUTANT_REACH names when it exits, so
# the runner can count them for the test that started it.
RUNTIME_SRC = """
import atexit, os, sys, types
m = types.ModuleType('hydra_mutant')
m.active = int(os.environ.get('HYDRA_MUTANT') or 0)
m.reached = set()
m.in_runner = False
def switch(n, _m=m):
    _m.reached.add(n)
    return n == _m.active
m.switch = switch
def _write(_m=m):
    p = os.environ.get('HYDRA_MUTANT_REACH')
    if p and _m.reached and not _m.in_runner:
        with open(p, 'a', encoding='utf-8') as f:
            f.write(' '.join(str(n) for n in sorted(_m.reached)) + '\\n')
atexit.register(_write)
sys.modules['hydra_mutant'] = m
"""

PRELUDE = ("{name} = (__import__('sys').modules.get({mod!r}) or exec({src!r}, {{}}) "
           "or __import__('sys').modules[{mod!r}]).switch").format(
               name=SWITCH_NAME, mod=RUNTIME_MODULE, src=RUNTIME_SRC)


def test_file_pattern():
    """unittest's own default discovery pattern, so "a test file" has one home."""
    return inspect.signature(unittest.TestLoader.discover).parameters["pattern"].default


def is_test_file(name):
    return fnmatch.fnmatch(name, test_file_pattern())


def _load_sibling(file_name, module_name):
    """A module beside this one, loaded under a name no target folder's own
    modules can share, so a test in the copy that imports a module named
    `run` or `find` never gets this one."""
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), file_name)
    spec = importlib.util.spec_from_file_location(module_name, path)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


# run.py is the home of the time limit.
_runner = _load_sibling("run.py", "hydra_mutate_run")

# find.py's KINDS is the one home of the bug kinds: their names, and the C++
# token each one changes into which. This module adds only Python's view of
# those tokens, in AST_TOKEN and SPELLING below.
KINDS = _load_sibling("find.py", "hydra_mutate_find").KINDS

# For each kind, the AST class (or, for return_bool, the constant) that holds
# each of find.py's tokens in Python.
AST_TOKEN = {
    "compare": {ast.Lt: "<", ast.LtE: "<=", ast.Gt: ">", ast.GtE: ">=",
                ast.Eq: "==", ast.NotEq: "!="},
    "and_or": {ast.And: "&&", ast.Or: "||"},
    "drop_not": {ast.Not: "!"},
    "plus_one": {ast.Add: "+"},
    "return_bool": {True: "true", False: "false"},
}

# Python's text for a find.py token that Python spells differently; a token
# spelled the same in both has no entry. find.py writes `(bool)` for a dropped
# `!` only so the C++ expression keeps its type; Python needs no such cast, so
# a dropped `not` leaves nothing.
SPELLING = {"&&": "and", "||": "or", "!": "not", "(bool)": "",
            "true": "True", "false": "False"}


def spell(token):
    """Python's text for one of find.py's tokens."""
    return SPELLING.get(token, token)


def python_swap(kind, key):
    """(was, now): the Python text of a spot of `kind` whose AST class or
    constant is `key`, and the text it becomes, both from find.py's swap."""
    token = AST_TOKEN[kind][key]
    return spell(token), spell(KINDS[kind]["swap"][token])


# ------------------------------------------------------------------ finding


class Source:
    """A file's text with a map from ast (line, UTF-8 byte column) to offset."""

    def __init__(self, text):
        self.text = text
        self.starts = [0]
        for m in re.finditer(r"\r\n|\r|\n", text):
            self.starts.append(m.end())

    def offset(self, lineno, col_bytes):
        start = self.starts[lineno - 1]
        end = self.starts[lineno] if lineno < len(self.starts) else len(self.text)
        line = self.text[start:end]
        return start + len(line.encode("utf-8")[:col_bytes].decode("utf-8"))

    def span(self, node):
        return (self.offset(node.lineno, node.col_offset),
                self.offset(node.end_lineno, node.end_col_offset))

    def line_of(self, offset):
        return bisect.bisect_right(self.starts, offset)


def _mask_comments(gap):
    """The gap between two operands with comments blanked, same length.

    A gap holds no string (a string would be an operand node), so a `#`
    there always starts a comment.
    """
    return re.sub(r"#[^\r\n]*", lambda m: " " * len(m.group(0)), gap)


def _find_token(src, lo, hi, token):
    """The offset of `token`, alone, in text[lo:hi], or None if not exactly once."""
    gap = _mask_comments(src.text[lo:hi])
    if token.isalpha():
        pat = r"(?<![\w])" + re.escape(token) + r"(?![\w])"
    else:
        # The gap's only operator is this one, but `<` must not match `<=`.
        pat = r"(?<![<>=!])" + re.escape(token) + r"(?![<>=])"
    found = [m.start() for m in re.finditer(pat, gap)]
    return lo + found[0] if len(found) == 1 else None


def _parents(tree):
    up = {}
    for node in ast.walk(tree):
        for child in ast.iter_child_nodes(node):
            up[child] = node
    return up


def _skip_reason(node, up):
    """A SKIP_REASONS key when the node sits where a wrapper cannot go."""
    fstring_types = tuple(t for t in (getattr(ast, "JoinedStr", None),
                                      getattr(ast, "TemplateStr", None)) if t)
    pattern_type = getattr(ast, "pattern", None)
    n = node
    while n in up:
        n = up[n]
        if isinstance(n, fstring_types):
            return "fstring"
        if pattern_type and isinstance(n, pattern_type):
            return "pattern"
    return None


def find_spots(text):
    """Every bug spot in one file's text, in file order.

    Each spot: kind, lo/hi (the whole expression the wrapper replaces), at
    (the token's offset), was and now (the token's text before and after),
    line. A spot that cannot be placed has `skip` set to a SKIP_REASONS key.
    Raises SyntaxError when the text does not parse.
    """
    tree = ast.parse(text)
    src = Source(text)
    up = _parents(tree)
    spots = []

    def add(kind, node, at_lo, at_hi, table_key):
        was, now = python_swap(kind, table_key)
        lo, hi = src.span(node)
        at = _find_token(src, at_lo, at_hi, was) if kind != "return_bool" else lo
        skip = _skip_reason(node, up)
        if skip is None and at is None:
            skip = "token"
        spots.append({"kind": kind, "lo": lo, "hi": hi, "at": at, "was": was, "now": now,
                      "line": src.line_of(at if at is not None else lo), "skip": skip})

    for node in ast.walk(tree):
        if isinstance(node, ast.Compare):
            left = node.left
            for op, right in zip(node.ops, node.comparators):
                if type(op) in AST_TOKEN["compare"]:
                    add("compare", node, src.span(left)[1], src.span(right)[0], type(op))
                left = right
        elif isinstance(node, ast.BoolOp):
            for a, b in zip(node.values, node.values[1:]):
                add("and_or", node, src.span(a)[1], src.span(b)[0], type(node.op))
        elif isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.Not):
            add("drop_not", node, src.span(node)[0], src.span(node.operand)[0], ast.Not)
        elif isinstance(node, ast.BinOp) and isinstance(node.op, ast.Add) \
                and isinstance(node.right, ast.Constant) and type(node.right.value) is int \
                and node.right.value == 1:
            add("plus_one", node, src.span(node.left)[1], src.span(node.right)[0], ast.Add)
        elif isinstance(node, ast.Return) and isinstance(node.value, ast.Constant) \
                and type(node.value.value) is bool:
            add("return_bool", node.value, 0, 0, node.value.value)
    spots.sort(key=lambda s: (s["lo"], -s["hi"], s["at"] if s["at"] is not None else -1))
    return spots


# ------------------------------------------------------------------ rewriting


def _edited(text, lo, hi, spot):
    """text[lo:hi] with one spot's token edit applied, and no wrappers."""
    a = spot["at"] - lo
    piece = text[lo:hi]
    return piece[:a] + spot["now"] + piece[a + len(spot["was"]):]


def _inside(inner, outer):
    if inner is outer:
        return False
    if (inner["lo"], inner["hi"]) == (outer["lo"], outer["hi"]):
        return inner["number"] < outer["number"]
    return outer["lo"] <= inner["lo"] and inner["hi"] <= outer["hi"]


def _render(text, lo, hi, spots):
    """text[lo:hi] with every spot in it wrapped, outermost first."""
    out = []
    pos = lo
    for s in sorted(spots, key=lambda s: (s["lo"], -s["hi"], -s["number"])):
        if s["lo"] < pos:
            continue
        out.append(text[pos:s["lo"]])
        inner = [t for t in spots if _inside(t, s)]
        off = _render(text, s["lo"], s["hi"], inner)
        on = _edited(text, s["lo"], s["hi"], s)
        out.append("(({}) if {}({}) else ({}))".format(on, SWITCH_NAME, s["number"], off))
        pos = s["hi"]
    out.append(text[pos:hi])
    return "".join(out)


def _prelude_at(text):
    """Where the prelude goes, and the text placed there.

    After the docstring and `from __future__` lines, joined with `;` so no
    line number moves; before the first statement on its own line when there
    are none.
    """
    tree = ast.parse(text)
    src = Source(text)
    lead = None
    for i, st in enumerate(tree.body):
        doc = i == 0 and isinstance(st, ast.Expr) and isinstance(st.value, ast.Constant) \
            and isinstance(st.value.value, str)
        future = isinstance(st, ast.ImportFrom) and st.module == "__future__"
        if not (doc or future):
            break
        lead = st
    if lead is not None:
        return src.span(lead)[1], "; " + PRELUDE
    first = tree.body[0]
    line = min([first.lineno] + [d.lineno for d in getattr(first, "decorator_list", [])])
    return src.starts[line - 1], PRELUDE + "\n"


def rewrite_text(text, spots):
    """The wrapped text for the numbered, unskipped spots of one file."""
    live = [s for s in spots if s.get("number")]
    if not live:
        return text
    body = _render(text, 0, len(text), live)
    # The prelude's place is found in the original; wrappers sit only inside
    # expressions, so the text before that place is unchanged.
    at, what = _prelude_at(text)
    assert body[:at] == text[:at]
    return body[:at] + what + body[at:]


def _read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8")


def _write(path, text):
    with open(path, "wb") as f:
        f.write(text.encode("utf-8"))


def copy_and_rewrite(target, copy_dir, mutate=None):
    """Copy `target` to `copy_dir` and rewrite the copy's non-test .py files.

    `mutate` limits the rewrite to these paths, relative to the target.
    Returns (mutants, skipped). The target is only read.
    """
    target = os.path.abspath(target)
    copy_dir = os.path.abspath(copy_dir)
    if os.path.commonpath([target, copy_dir]) == target:
        raise SystemExit("py_mutate: the run folder %s is inside the target %s" % (copy_dir, target))
    shutil.copytree(target, copy_dir, ignore=shutil.ignore_patterns("__pycache__", ".git"))
    files = []
    for root, dirs, names in os.walk(copy_dir):
        dirs.sort()
        for name in sorted(names):
            if name.endswith(".py") and not is_test_file(name):
                files.append(os.path.relpath(os.path.join(root, name), copy_dir).replace("\\", "/"))
    if mutate is not None:
        wanted = {m.replace("\\", "/") for m in mutate}
        missing = wanted - set(files)
        if missing:
            raise SystemExit("py_mutate: not a non-test .py file in the target: %s" % ", ".join(sorted(missing)))
        files = [f for f in files if f in wanted]
    mutants, skipped = [], []
    number = 0
    for rel in files:
        path = os.path.join(copy_dir, rel)
        try:
            text = _read(path)
            spots = find_spots(text)
        except (SyntaxError, ValueError, UnicodeDecodeError) as e:
            skipped.append({"file": rel, "line": 0, "reason": FILE_SKIP_REASONS["parse"], "detail": str(e)})
            continue
        for s in spots:
            if s["skip"]:
                skipped.append({"file": rel, "line": s["line"], "reason": SKIP_REASONS[s["skip"]]})
                continue
            number += 1
            s["number"] = number
            mutants.append({"number": number, "file": rel, "line": s["line"], "kind": s["kind"],
                            "change": "`%s` becomes `%s`" % (s["was"], s["now"]) if s["now"]
                            else "`%s` is dropped" % s["was"],
                            "lo": s["lo"], "hi": s["hi"], "at": s["at"], "was": s["was"], "now": s["now"]})
        new = rewrite_text(text, spots)
        if new != text:
            _write(path, new)
    return mutants, skipped


# ------------------------------------------------------------------ undoing (for checks)


class _Unwrap(ast.NodeTransformer):
    """Takes each wrapper's "off" side, or the "on" side for switch `on`."""

    def __init__(self, on=0):
        self.on = on

    def visit_IfExp(self, node):
        t = node.test
        if isinstance(t, ast.Call) and isinstance(t.func, ast.Name) and t.func.id == SWITCH_NAME \
                and len(t.args) == 1 and isinstance(t.args[0], ast.Constant):
            side = node.body if t.args[0].value == self.on else node.orelse
            return self.visit(side)
        return self.generic_visit(node)


def unwrap(text, on=0):
    """The AST of rewritten text with the prelude gone and one side of each wrapper taken."""
    tree = ast.parse(text)
    tree.body = [st for st in tree.body
                 if not (isinstance(st, ast.Assign) and len(st.targets) == 1
                         and isinstance(st.targets[0], ast.Name) and st.targets[0].id == SWITCH_NAME)]
    return _Unwrap(on).visit(tree)


def same_ast(a, b):
    return ast.dump(a, include_attributes=False) == ast.dump(b, include_attributes=False)


# ------------------------------------------------------------------ the child (one test process)


class _Result(unittest.TestResult):
    """Records each test's outcome, its failed lines, and the switches reached."""

    def __init__(self, take, out):
        super().__init__()
        self.take = take
        self.out = out
        self.status = {}
        self.where = {}
        self.t0 = 0.0

    def startTest(self, test):
        super().startTest(test)
        self.t0 = time.perf_counter()

    def _fail(self, test, err, kind):
        tid = test.id()
        if self.status.get(tid) != "error":
            self.status[tid] = kind
        mod = sys.modules.get(type(test).__module__)
        mfile = os.path.normcase(os.path.abspath(getattr(mod, "__file__", "") or ""))
        lines = [fr.lineno for fr in traceback.extract_tb(err[2])
                 if os.path.normcase(os.path.abspath(fr.filename)) == mfile]
        if lines:
            self.where.setdefault(tid, []).append(lines[-1])

    def addFailure(self, test, err):
        super().addFailure(test, err)
        self._fail(test, err, "failed")

    def addError(self, test, err):
        super().addError(test, err)
        self._fail(test, err, "error")

    def addSubTest(self, test, subtest, err):
        super().addSubTest(test, subtest, err)
        if err is not None:
            self._fail(test, err, "error" if not issubclass(err[0], test.failureException) else "failed")

    def addUnexpectedSuccess(self, test):
        super().addUnexpectedSuccess(test)
        self.status[test.id()] = "failed"

    def addSkip(self, test, reason):
        super().addSkip(test, reason)
        self.status.setdefault(test.id(), "skipped")

    def stopTest(self, test):
        super().stopTest(test)
        tid = test.id()
        self.out.write(json.dumps({
            "test": tid, "module": type(test).__module__,
            "status": self.status.get(tid, "passed"),
            "lines": self.where.get(tid, []),
            "seconds": round(time.perf_counter() - self.t0, 4),
            "reached": sorted(self.take())}) + "\n")
        self.out.flush()


def child_main(top, names, out_path):
    exec(RUNTIME_SRC, {})
    hm = sys.modules[RUNTIME_MODULE]
    hm.in_runner = True
    reach_file = out_path + ".reach"
    os.environ["HYDRA_MUTANT_REACH"] = reach_file

    def take():
        got = set(hm.reached)
        hm.reached.clear()
        if os.path.exists(reach_file):
            with open(reach_file, encoding="utf-8") as f:
                for line in f:
                    got.update(int(x) for x in line.split())
            os.remove(reach_file)
        return got

    os.chdir(top)
    sys.path.insert(0, top)
    loader = unittest.TestLoader()
    t0 = time.perf_counter()
    if names:
        suite = unittest.TestSuite(loader.loadTestsFromName(n) for n in names)
    else:
        suite = loader.discover(top, top_level_dir=top)
    with open(out_path, "w", encoding="utf-8") as out:
        out.write(json.dumps({"test": "before any test", "reached": sorted(take()),
                              "seconds": round(time.perf_counter() - t0, 4),
                              "load_errors": [str(e) for e in loader.errors]}) + "\n")
        result = _Result(take, out)
        suite.run(result)
        out.write(json.dumps({"test": "after all tests", "reached": sorted(take())}) + "\n")


# ------------------------------------------------------------------ the runner


def _kill_tree(p):
    if os.name == "nt":
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(p.pid)], capture_output=True)
    else:
        p.kill()
    p.wait()


def run_child(copy_dir, out_path, mutant=0, names=None, timeout=None):
    """One fresh test process on the copy. Returns (lines, seconds, timed_out)."""
    env = dict(os.environ)
    env.pop("HYDRA_MUTANT", None)
    if mutant:
        env["HYDRA_MUTANT"] = str(mutant)
    cmd = [sys.executable, "-I", os.path.abspath(__file__), "--child", copy_dir, out_path]
    cmd += list(names or [])
    t0 = time.perf_counter()
    with open(out_path + ".log", "wb") as log:
        p = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT, env=env, cwd=copy_dir)
        try:
            p.wait(timeout=timeout)
            timed_out = False
        except subprocess.TimeoutExpired:
            _kill_tree(p)
            timed_out = True
    seconds = time.perf_counter() - t0
    lines = []
    if os.path.exists(out_path):
        with open(out_path, encoding="utf-8") as f:
            lines = [json.loads(x) for x in f if x.strip()]
    if not timed_out and not lines:
        raise SystemExit("py_mutate: the test process wrote nothing; see %s.log" % out_path)
    return lines, seconds, timed_out


def build_map(lines):
    """From the mapping run's lines: tests, the switches each reached, the red ones."""
    tests = [x for x in lines if "status" in x]
    edge = {x["test"]: set(x["reached"]) for x in lines if "status" not in x}
    reach = {x["test"]: set(x["reached"]) for x in tests}
    red = sorted(x["test"] for x in tests if x["status"] in ("failed", "error"))
    return tests, reach, edge, red


def modules_for(n, tests, reach, edge):
    """The test modules to run for switch n: those holding a test that reaches it.

    A switch reached only outside a test (while modules load, or after the
    last test) cannot be tied to one module, so every module is run.
    """
    mods = sorted({x["module"] for x in tests if n in reach[x["test"]]})
    if any(n in s for s in edge.values()):
        mods = sorted({x["module"] for x in tests})
    return mods


def run_bug(run_dir, copy_dir, n, tests, reach, edge, red, load_seconds):
    mods = modules_for(n, tests, reach, edge)
    baseline = load_seconds + sum(x["seconds"] for x in tests if x["module"] in mods)
    out = os.path.join(run_dir, "bugs", "%d.jsonl" % n)
    lines, seconds, timed_out = run_child(copy_dir, out, n, mods, _runner.test_timeout_seconds(baseline))
    reachers = {t for t, s in reach.items() if n in s}
    reds = [x for x in lines if x.get("status") in ("failed", "error") and x["test"] not in red]
    caught = [x for x in reds if x["test"] in reachers or any(n in s for s in edge.values())]
    stray = [x for x in reds if x not in caught]
    return {"number": n, "modules": mods, "seconds": round(seconds, 3), "timed_out": timed_out,
            "caught_by": [{"test": x["test"], "lines": x["lines"]} for x in caught],
            "red_not_reaching": [x["test"] for x in stray]}


def write_summary(run_dir, mutants, skipped, tests, reach, edge, red, bugs):
    by_n = {b["number"]: b for b in bugs}
    reached_any = set().union(*reach.values(), *edge.values()) if (reach or edge) else set()
    lines = [
        "A test that caught nothing has shown no way to fail against these bugs;",
        "that is a question to look at, not proof the test is empty.",
        "A check after an earlier failing assertion in the same test may never have run.",
        "",
        "Per bug:",
    ]
    for m in mutants:
        b = by_n.get(m["number"])
        if m["number"] not in reached_any:
            state = "reached by no test"
        elif b is None:
            state = "not run"
        elif b["timed_out"]:
            state = "timed out (counted as caught)"
        elif b["caught_by"]:
            state = "caught by " + ", ".join("%s (lines %s)" % (c["test"], c["lines"]) for c in b["caught_by"])
        else:
            state = "survived"
        lines.append("  #%d %s:%d %s, %s: %s" % (m["number"], m["file"], m["line"], m["kind"], m["change"], state))
    catches = {x["test"]: 0 for x in tests}
    for b in bugs:
        for c in b["caught_by"]:
            catches[c["test"]] = catches.get(c["test"], 0) + 1
    per_test = sorted(catches, key=lambda t: (catches[t] != 0, t))
    lines += ["", "Per test (caught none first):"]
    for t in per_test:
        lines.append("  %s: reaches %d, caught %d" % (t, len(reach.get(t, ())), catches[t]))
    lines += ["", "Total catches, per bug: %d; per test: %d" % (
        sum(len(b["caught_by"]) for b in bugs), sum(catches.values()))]
    lines += ["", "Red with no bug (left out of every bug run):"] + ["  " + t for t in red]
    errors = [(b["number"], t) for b in bugs for t in b["red_not_reaching"]]
    lines += ["", "Errors: red on a bug the map says it does not reach:"]
    lines += ["  #%d %s" % e for e in errors]
    lines += ["", "Skipped spots:"] + ["  %s:%d %s" % (s["file"], s["line"], s["reason"]) for s in skipped]
    _write(os.path.join(run_dir, "summary.txt"), "\n".join(lines) + "\n")


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if argv and argv[0] == "--child":
        child_main(argv[1], argv[3:], argv[2])
        return 0
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--target", required=True, help="the folder to copy and rewrite")
    ap.add_argument("--out", required=True, help="the run folder; must not exist yet")
    ap.add_argument("--mutate", nargs="*", help="only these .py files, relative to the target")
    ap.add_argument("--tests", nargs="*", help="only these test modules (dotted names), not discovery")
    ap.add_argument("--processes", type=int, default=1, help="bug runs side by side")
    ap.add_argument("--map-only", action="store_true", help="stop after the mapping run")
    a = ap.parse_args(argv)
    run_dir = os.path.abspath(a.out)
    if os.path.exists(run_dir):
        raise SystemExit("py_mutate: the run folder exists already: %s" % run_dir)
    copy_dir = os.path.join(run_dir, "copy")
    mutants, skipped = copy_and_rewrite(a.target, copy_dir, a.mutate)
    _write(os.path.join(run_dir, "mutants.json"), json.dumps(mutants, indent=1))
    _write(os.path.join(run_dir, "skipped.txt"),
           "".join("%s:%d %s\n" % (s["file"], s["line"], s["reason"]) for s in skipped))
    lines, seconds, _ = run_child(copy_dir, os.path.join(run_dir, "map.jsonl"), 0, a.tests)
    tests, reach, edge, red = build_map(lines)
    print("py_mutate: %d bugs, %d skipped; mapping run %d tests in %.1f s, %d red with no bug"
          % (len(mutants), len(skipped), len(tests), seconds, len(red)))
    bugs = []
    if not a.map_only:
        os.makedirs(os.path.join(run_dir, "bugs"))
        load_seconds = next(x["seconds"] for x in lines if x["test"] == "before any test")
        live = [m["number"] for m in mutants if modules_for(m["number"], tests, reach, edge)]
        with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, a.processes)) as pool:
            bugs = list(pool.map(lambda n: run_bug(run_dir, copy_dir, n, tests, reach, edge, red,
                                                   load_seconds), live))
        _write(os.path.join(run_dir, "bugs.json"), json.dumps(bugs, indent=1))
    write_summary(run_dir, mutants, skipped, tests, reach, edge, red, bugs)
    print("py_mutate: report in %s" % os.path.join(run_dir, "summary.txt"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
