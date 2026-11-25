"""The mutation gate: of the tests a change added or changed, which caught no planted bug?

    py -I tools/mutate/gate.py cpp <run folder> --range <A..B> --files <test file> ...
        [--repo <checkout>] [--mutants <mutants.json>] [--out <folder>]
    py -I tools/mutate/gate.py py <run folder> --range <A..B> --files <test file> ...
        --target <folder> [--repo <checkout>] [--out <folder>]

`cpp` reads a run folder run.py wrote and report.py can read; `py` reads a
run folder py_mutate.py wrote. --range is the change, as `git diff` takes it,
in the checkout --repo names (default: the checkout holding this script).
--files are the change's test files, as paths in that checkout. For `py`,
--target is the folder py_mutate.py copied (its --target), as a path in the
checkout, so each test module can be found as a file.

Which tests the change added or changed is the one rule this script owns
(changed_cpp_tests, changed_py_modules). A C++ test id in the run is
`file:line name` (run.py's test_id), and a test runs from its own line to the
next test's line in the same file; the last test of a file runs to its end.
A test is changed when an added line of `git diff -U0 <range>` falls inside
it. A Python test id carries no line, so every test of a test module the
range added a line to is judged, and the output says so.

What a test caught is never worked out here. For `cpp` it is the catches
report.py's build_report credits to the test, which are the ones run.py's
bug_result sorted; for `py` it is the "caught" count of py_mutate.py's
summary.txt, with the bugs read from its bugs.json. A bug reached counts
against a test only when its run counted: report.py lists an incomplete run
as "not counted", and a reached bug the run stopped before has no run.

One line is printed per changed test:

    passed: <test>: caught <k> of the planted bugs; it reaches <n>
    refused: <test>: reached <n> planted bugs and caught none; <gate.txt> lists them
    not judged: <test>: <reason>

then one line per given file the gate could not look into, and a last line
with the three counts. The gate exits 1 when any test is refused, 0 when
none is, and 2 when it cannot read its inputs. A test it did not judge never
reads "passed". gate.txt, written into --out (default: the run folder),
holds each judged test's line again with every bug it reaches under it.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Optional

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import report  # noqa: E402
import run  # noqa: E402

PASSED = "passed"
REFUSED = "refused"
NOT_JUDGED = "not judged"

EXIT_NONE_REFUSED = 0
EXIT_REFUSED = 1
EXIT_UNREADABLE = 2

GATE_FILE = "gate.txt"

NOT_IN_RUN = ("this run holds no test from it. hydra_uitest tests are never judged, because the mutation "
              "build builds only hydra_tests")
PY_NOT_IN_RUN = "this run holds no test from this module"
REACHES_NONE ="it reaches no planted bug, so the gate has nothing to say about it"
RED_WITH_NO_BUG = ("it was red with no bug in the mapping run, so it is left out of every bug run and can catch "
                   "nothing")
PY_WHOLE_MODULE = ("Python test ids carry no line, so every test of a test module the range added a line to is "
                   "judged.")


class GateError(RuntimeError):
    pass


# ------------------------------------------------------------------ the change


_HUNK = re.compile(r"^@@ -\d+(?:,\d+)? \+(\d+)(?:,(\d+))? @@")


def added_lines(repo: Path, rng: str, files: list[str]) -> dict[str, set[int]]:
    """Each file's added lines in `git diff -U0 <rng>`, by its path in the checkout with forward slashes."""
    cmd = ["git", "-C", str(repo), "-c", "core.quotePath=false", "diff", "-U0", "--no-color", "--no-ext-diff",
           "--no-renames", "--src-prefix=a/", "--dst-prefix=b/", rng, "--"] + files
    done = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    if done.returncode != 0:
        raise GateError(f"gate.py: git diff {rng} failed in {repo}: {done.stderr.strip()}")
    out: dict[str, set[int]] = {}
    current: Optional[str] = None
    for line in done.stdout.splitlines():
        if line.startswith("+++ "):
            name = line[4:]
            if name == "/dev/null":
                current = None
            elif name.startswith("b/"):
                current = name[2:]
                out.setdefault(current, set())
            else:
                raise GateError(f"gate.py: git diff names a file the gate cannot read: {line!r}")
            continue
        hunk = _HUNK.match(line)
        if hunk and current is not None:
            start = int(hunk.group(1))
            count = int(hunk.group(2)) if hunk.group(2) is not None else 1
            out[current].update(range(start, start + count))
    return out


def _in_checkout(path: str, repo: Path) -> str:
    """A path as the checkout names it: relative to `repo`, with forward slashes."""
    p = path.replace("\\", "/")
    if os.path.isabs(p):
        rel = os.path.relpath(p, str(repo))
        if rel.startswith(".."):
            raise GateError(f"gate.py: {path} is outside the checkout {repo}")
        p = rel.replace("\\", "/")
    return p


def changed_cpp_tests(tests: list[tuple[str, str, int]], added: dict[str, set[int]]) -> list[str]:
    """The test ids an added line falls inside, in the order given.

    `tests` is each test as (id, file in the checkout, line). A test runs
    from its own line to the next test's line in the same file, and the last
    one to the end of the file.
    """
    by_file: dict[str, list[int]] = {}
    for _, f, line in tests:
        by_file.setdefault(f, []).append(line)
    for lines in by_file.values():
        lines.sort()
    out = []
    for tid, f, line in tests:
        later = [ln for ln in by_file[f] if ln > line]
        end = later[0] if later else None
        if any(a >= line and (end is None or a < end) for a in added.get(f, ())):
            out.append(tid)
    return out


def changed_py_modules(files: list[str], target: str, added: dict[str, set[int]]) -> dict[str, Optional[str]]:
    """Each given file's test module, or None when the range added no line to it.

    A module is its path under `target` with dots for slashes and no .py, as
    unittest names it. A file outside `target` stops the gate.
    """
    out: dict[str, Optional[str]] = {}
    top = target.rstrip("/") + "/" if target not in ("", ".") else ""
    for f in files:
        if not f.startswith(top) or not f.endswith(".py"):
            raise GateError(f"gate.py: {f} is not a .py file under the target {target}")
        out[f] = f[len(top):-len(".py")].replace("/", ".") if added.get(f) else None
    return out


# ------------------------------------------------------------------ one test's line


def verdict(caught: list[int], counted: list[int], uncounted: list[int], red: bool) -> tuple[str, str]:
    """One changed test's word and the rest of its line.

    `caught` are the bugs the owner credits to the test, `counted` the
    reached bugs whose run counted, `uncounted` the reached bugs whose run
    did not count or never happened.
    """
    if caught:
        return PASSED, f"caught {len(caught)} of the planted bugs; it reaches {len(counted) + len(uncounted)}"
    if red:
        return NOT_JUDGED, RED_WITH_NO_BUG
    if counted:
        return REFUSED, f"reached {len(counted)} planted bugs and caught none"
    if uncounted:
        return NOT_JUDGED, (f"it reaches {len(uncounted)} planted bugs, and none of them has a run that counted "
                            "(incomplete, or never run because the run stopped first)")
    return NOT_JUDGED, REACHES_NONE


def _finish(rows: list[dict], notes: list[str], out_dir: Path, head: list[str]) -> int:
    """Prints the lines, writes gate.txt, and gives the exit code."""
    gate_file = out_dir / GATE_FILE
    lines, file_lines = list(head), list(head)
    for r in rows:
        text = f"{r['word']}: {r['test']}: {r['rest']}"
        if r["word"] == REFUSED:
            text += f"; {gate_file} lists them"
        lines.append(text)
        if r["word"] != NOT_JUDGED:
            file_lines.append(text)
            file_lines += ["  " + b for b in r["bugs"]]
    lines += notes
    counts = {w: sum(1 for r in rows if r["word"] == w) for w in (REFUSED, PASSED, NOT_JUDGED)}
    lines.append(f"gate: {counts[REFUSED]} refused, {counts[PASSED]} passed, {counts[NOT_JUDGED]} not judged.")
    out_dir.mkdir(parents=True, exist_ok=True)
    gate_file.write_text("\n".join(file_lines + [lines[-1]]) + "\n", encoding="utf-8")
    print("\n".join(lines))
    return EXIT_REFUSED if counts[REFUSED] else EXIT_NONE_REFUSED


# ------------------------------------------------------------------ C++ runs


def gate_cpp(run_dir: Path, mutants: Path, repo: Path, rng: str, files: list[str], out_dir: Path) -> int:
    r = report.build_report(run_dir, mutants)
    m = r["map"]
    uncounted_bugs = set(r["lists"]["incomplete"]) | set(r["not_run"])
    tests = []
    for tid in m["order"]:
        try:
            where = _in_checkout(m["tests"][tid]["file"], repo)
        except GateError:
            where = m["tests"][tid]["file"].replace("\\", "/")  # outside the checkout: no given file can match it
        tests.append((tid, where, m["tests"][tid]["line"]))
    if tests and not any(not os.path.isabs(t[1]) for t in tests):
        raise GateError(f"gate.py: no test of {run_dir} lies in the checkout {repo}; the run was built from another "
                        "checkout, so name that one with --repo")
    wanted = [_in_checkout(f, repo) for f in files]
    in_run = {f for _, f, _ in tests}
    added = added_lines(repo, rng, wanted)
    changed = changed_cpp_tests([t for t in tests if t[1] in wanted], added)
    notes = []
    for f in wanted:
        if f not in in_run:
            notes.append(f"{NOT_JUDGED}: {f}: {NOT_IN_RUN}")
        elif not any(t[1] == f and t[0] in changed for t in tests):
            notes.append(f"unchanged: {f}: no test in it holds a line {rng} added")
    rows = []
    for tid in changed:
        rec = r["tests"][tid]
        caught = rec["caught"]
        counted = [n for n in rec["reaches"] if n not in uncounted_bugs]
        uncounted = [n for n in rec["reaches"] if n in uncounted_bugs]
        word, rest = verdict(caught, counted, uncounted, tid in m["red_with_no_bug"])
        bugs = []
        for n in sorted(set(rec["reaches"]) | set(caught)):
            state = report.STATE_WORDS[r["bugs"][n]["state"]] if n in r["bugs"] else "never run"
            mark = "; caught by this test" if n in caught else ""
            bugs.append(f"{report._bug_line(n, r)}; {state}{mark}")
        rows.append({"test": tid, "word": word, "rest": rest, "bugs": bugs})
    head = [f"Mutation gate on run {run_dir.name}, range {rng}."]
    return _finish(rows, notes, out_dir, head)


# ------------------------------------------------------------------ Python runs


_PER_TEST = re.compile(r"^  (.+): reaches (\d+), caught (\d+)$")
_PER_BUG = re.compile(r"^  #(\d+) ")


def read_py_run(run_dir: Path) -> dict:
    """What py_mutate.py's run folder says of each test and each bug.

    The map is py_mutate's own build_map of map.jsonl; each test's caught
    count is summary.txt's, and bugs.json names which bugs those are. A
    count the two files do not agree on stops the gate.
    """
    import py_mutate  # noqa: E402  (loads run.py and find.py beside it)

    paths = {name: run_dir / name for name in ("map.jsonl", "summary.txt", "bugs.json")}
    for name, p in paths.items():
        if not p.is_file():
            raise GateError(f"gate.py: no {p}; {run_dir} is not a finished py_mutate.py run folder")
    lines = [json.loads(x) for x in paths["map.jsonl"].read_text(encoding="utf-8").splitlines() if x.strip()]
    tests, reach, edge, red = py_mutate.build_map(lines)
    bugs = json.loads(paths["bugs.json"].read_text(encoding="utf-8"))
    summary = paths["summary.txt"].read_text(encoding="utf-8").splitlines()
    counts, bug_lines = {}, {}
    section = None
    for line in summary:
        if not line.startswith("  "):
            section = line
            continue
        if section == "Per test (caught none first):":
            hit = _PER_TEST.match(line)
            if hit:
                counts[hit.group(1)] = (int(hit.group(2)), int(hit.group(3)))
        elif section == "Per bug:":
            hit = _PER_BUG.match(line)
            if hit:
                bug_lines[int(hit.group(1))] = line.strip()
    run_bugs = {b["number"] for b in bugs}
    out = {}
    for x in tests:
        tid = x["test"]
        if tid not in counts:
            raise GateError(f"gate.py: {paths['summary.txt']} has no line for {tid}")
        caught = sorted(b["number"] for b in bugs if any(c["test"] == tid for c in b["caught_by"]))
        if len(caught) != counts[tid][1] or len(reach[tid]) != counts[tid][0]:
            raise GateError(f"gate.py: {paths['summary.txt']} and {paths['bugs.json']} disagree on {tid}")
        out[tid] = {"module": x["module"], "reaches": sorted(reach[tid]), "caught": caught,
                    "red": tid in red}
    return {"tests": out, "order": [x["test"] for x in tests], "run_bugs": run_bugs, "bug_lines": bug_lines}


def gate_py(run_dir: Path, repo: Path, target: str, rng: str, files: list[str], out_dir: Path) -> int:
    pr = read_py_run(run_dir)
    target = _in_checkout(target, repo)
    wanted = [_in_checkout(f, repo) for f in files]
    modules = changed_py_modules(wanted, target, added_lines(repo, rng, wanted))
    in_run = {rec["module"] for rec in pr["tests"].values()}
    notes = []
    picked = set()
    for f, mod in modules.items():
        if mod is None:
            notes.append(f"unchanged: {f}: {rng} added no line to it")
        elif mod not in in_run:
            notes.append(f"{NOT_JUDGED}: {f}: {PY_NOT_IN_RUN}")
        else:
            picked.add(mod)
    rows = []
    for tid in pr["order"]:
        rec = pr["tests"][tid]
        if rec["module"] not in picked:
            continue
        counted = [n for n in rec["reaches"] if n in pr["run_bugs"]]
        uncounted = [n for n in rec["reaches"] if n not in pr["run_bugs"]]
        word, rest = verdict(rec["caught"], counted, uncounted, rec["red"])
        bugs = [pr["bug_lines"].get(n, f"#{n}") for n in rec["reaches"]]
        rows.append({"test": tid, "word": word, "rest": rest, "bugs": bugs})
    head = [f"Mutation gate on Python run {run_dir.name}, range {rng}.", PY_WHOLE_MODULE]
    return _finish(rows, notes, out_dir, head)


# ------------------------------------------------------------------ the command


def main(argv=None) -> int:
    root = Path(__file__).resolve().parents[2]
    ap = argparse.ArgumentParser(description="Refuse a changed test that reaches planted bugs and catches none.")
    ap.add_argument("kind", choices=("cpp", "py"), help="cpp: a run.py run folder; py: a py_mutate.py one")
    ap.add_argument("run_dir", type=Path, help="the run folder")
    ap.add_argument("--range", required=True, dest="rng", help="the change, as git diff takes it, e.g. A..B")
    ap.add_argument("--files", nargs="+", required=True, help="the change's test files, as paths in the checkout")
    ap.add_argument("--repo", type=Path, default=root, help="the checkout (default: the one holding this script)")
    ap.add_argument("--mutants", type=Path, default=None,
                    help="cpp only: the build's mutants.json (default: two folders above the run folder)")
    ap.add_argument("--target", default=None, help="py only: the folder py_mutate.py copied, in the checkout")
    ap.add_argument("--out", type=Path, default=None, help="where gate.txt goes (default: the run folder)")
    a = ap.parse_args(argv)
    run_dir, repo = a.run_dir.resolve(), a.repo.resolve()
    out_dir = a.out.resolve() if a.out else run_dir
    try:
        if a.kind == "cpp":
            if a.target is not None:
                raise GateError("gate.py: --target is for py runs only")
            mutants = (a.mutants or run_dir.parent.parent / "mutants.json").resolve()
            return gate_cpp(run_dir, mutants, repo, a.rng, a.files, out_dir)
        if a.mutants is not None or a.target is None:
            raise GateError("gate.py: a py run takes --target and no --mutants")
        return gate_py(run_dir, repo, a.target, a.rng, a.files, out_dir)
    except (GateError, report.ReportError, run.RunError) as e:
        print(e, file=sys.stderr)
        return EXIT_UNREADABLE


if __name__ == "__main__":
    sys.exit(main())
