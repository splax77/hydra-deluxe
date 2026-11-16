"""Split one hydra_mutate run's timed-out bugs by the test that was running when the run was stopped.

    py -I tools/mutate/timeout_split.py <run folder> <repo root> --fast-below <seconds>

A bug run that hits its time limit is stopped part way through a test. Its
bugs/<n>.json then names that test ("running_at_limit") and the tests that
had already gone red in its file ("red_in_file"); run.py's bug_result is the
one owner of both fields, and report.py's load_bugs reads the jsons.

It prints two splits.

The first puts each timed-out bug in one of four groups: it already had a
failing test, or it had none and was stopped in a test that runs faster than
--fast-below seconds clean, or in a slower one, or between tests. A test's
clean time is its "seconds" in the run's map.json, which run.py writes. The
line between fast and slow is an argument, so a reader can compare against
any figure; no line is built in.

The second asks, for each test that was running at a limit, whether its body
reaches one of the shared test waits, and prints the totals on each side and
then one line per test with its bug numbers. The waits are the ones
tests/wait_util.h defines (see wait_names). A test reaches one when its body
names it, or calls by name a function or method that does, to any depth. The
functions searched are those in the test's own file and in every header that
file includes with quotes, to any depth. This is a ceiling: a test that
reaches a wait may have been stuck somewhere else.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import statistics
import sys
from pathlib import Path
from typing import Optional

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import report  # noqa: E402

# The one home of test waits (its own header says so).
WAIT_HOME = "tests/wait_util.h"

CASE = re.compile(r"^\s*(TEST_CASE|TEST_CASE_FIXTURE|DOCTEST_TEST_CASE|SCENARIO)\b")
INCLUDE = re.compile(r'^\s*#include\s+"([^"]+)"', re.M)
# A function-shaped block: a name, its parentheses (one level of nesting
# inside), optional const/noexcept/trailing return type, then its brace.
DEF = re.compile(r"(?<![\w.>])(\w+)\s*\([^;{}()]*(?:\([^()]*\)[^;{}()]*)*\)\s*(?:const\s*)?(?:noexcept\s*)?"
                 r"(?:->\s*[\w:<>]+\s*)?\{")
# Words DEF would read as a function name but which open a statement block.
NOT_FUNCTIONS = {"if", "for", "while", "switch", "catch", "return", "sizeof", "TEST_CASE", "SUBCASE", "CHECK",
                 "REQUIRE"}
# A wait in WAIT_HOME is a function that sleeps between its tries.
SLEEPS = re.compile(r"\bsleep_for\s*\(")

GROUPS = ("red_already", "fast", "slow", "between")
GROUP_WORDS = {
    "red_already": "already had a failing test",
    "fast": "had none and were stopped in a test faster than the line clean",
    "slow": "had none and were stopped in a test at or above the line clean",
    "between": "had none and were stopped between tests",
}


class SplitError(RuntimeError):
    pass


def _calls(name: str, text: str) -> bool:
    return re.search(r"\b" + re.escape(name) + r"\s*\(", text) is not None


class Sources:
    """Reads the repo's test sources and answers which functions reach a wait."""

    def __init__(self, repo: Path):
        self.repo = repo
        self._texts: dict[str, Optional[str]] = {}
        self._defs: dict[str, dict[str, list[str]]] = {}
        self._helpers: dict[str, dict[str, str]] = {}
        self.waits = wait_names(self)

    def text(self, rel: str) -> Optional[str]:
        if rel not in self._texts:
            p = self.repo / rel
            self._texts[rel] = p.read_text(encoding="utf-8", errors="replace") if p.is_file() else None
        return self._texts[rel]

    def defs(self, rel: str) -> dict[str, list[str]]:
        """Each function-shaped block in the file: name -> the bodies of that name."""
        if rel not in self._defs:
            t = self.text(rel) or ""
            out: dict[str, list[str]] = {}
            for mm in DEF.finditer(t):
                name = mm.group(1)
                if name in NOT_FUNCTIONS:
                    continue
                i, depth = mm.end(), 1
                while i < len(t) and depth:
                    depth += t[i] == "{"
                    depth -= t[i] == "}"
                    i += 1
                out.setdefault(name, []).append(t[mm.end():i])
            self._defs[rel] = out
        return self._defs[rel]

    def closure(self, rel: str) -> list[str]:
        """The file and every header it includes with quotes, to any depth, leaving out WAIT_HOME."""
        seen: list[str] = []

        def visit(f: str):
            if f in seen or f == WAIT_HOME or self.text(f) is None:
                return
            seen.append(f)
            for inc in INCLUDE.findall(self.text(f)):
                for cand in ((Path(f).parent / inc).as_posix(), "tests/" + inc):
                    if self.text(cand) is not None:
                        visit(cand)
                        break

        visit(rel)
        return seen

    def helpers(self, rel: str) -> dict[str, str]:
        """Names in the file's closure whose body reaches a wait -> the file that defines it."""
        if rel not in self._helpers:
            bodies: dict[str, list[tuple[str, str]]] = {}
            for f in self.closure(rel):
                for name, bs in self.defs(f).items():
                    bodies.setdefault(name, []).extend((f, b) for b in bs)
            self._helpers[rel] = reaching(bodies, self.waits)
        return self._helpers[rel]

    def body(self, rel: str, line: int) -> str:
        """A test case's text: from its registered line to the next test case."""
        t = self.text(rel)
        if t is None:
            raise SplitError(f"timeout_split.py: no {self.repo / rel}")
        lines = t.split("\n")
        end = next((i for i in range(line, len(lines)) if CASE.match(lines[i])), len(lines))
        return "\n".join(lines[line - 1:end])

    def reaches_wait(self, rel: str, line: int) -> tuple[bool, list[str]]:
        """(the body names a wait itself, the helpers it calls that reach one)."""
        b = self.body(rel, line)
        direct = any(_calls(w, b) for w in self.waits)
        used = sorted(h for h in self.helpers(rel) if _calls(h, b))
        return direct, used


def reaching(bodies: dict[str, list[tuple[str, str]]], targets: set[str]) -> dict[str, str]:
    """Names whose body calls a target, or calls a name that does, to any depth -> its file."""
    hit: dict[str, str] = {}
    changed = True
    while changed:
        changed = False
        for name, bs in bodies.items():
            if name in hit:
                continue
            for f, b in bs:
                if any(_calls(t, b) for t in targets | set(hit)):
                    hit[name] = f
                    changed = True
                    break
    return hit


def wait_names(src: Sources) -> set[str]:
    """The waits WAIT_HOME defines: each function there that sleeps, and each that calls one."""
    defs = src.defs(WAIT_HOME)
    if not defs:
        raise SplitError(f"timeout_split.py: no functions read from {src.repo / WAIT_HOME}")
    sleepers = {n for n, bs in defs.items() if any(SLEEPS.search(b) for b in bs)}
    bodies = {n: [(WAIT_HOME, b) for b in bs] for n, bs in defs.items()}
    return sleepers | set(reaching(bodies, sleepers))


def read_map_tests(run_dir: Path) -> dict[str, dict]:
    """map.json's tests by id: each holds its file, registered line and clean "seconds"."""
    stored = json.loads((run_dir / "map.json").read_text(encoding="utf-8"))
    return {t["id"]: t for t in stored["tests"]}


def timed_out_bugs(run_dir: Path) -> list[dict]:
    bugs = report.load_bugs(run_dir)
    return [bugs[n] for n in sorted(bugs) if bugs[n]["state"] == "timed_out"]


def split_groups(bugs: list[dict], tests: dict[str, dict], fast_below: float) -> dict:
    """Each timed-out bug in one of GROUPS, plus those whose run left no result file to read."""
    out = {g: [] for g in GROUPS}
    out["no_result"] = []
    slow_cleans = []
    for b in bugs:
        if "running_at_limit" not in b:
            out["no_result"].append(b["number"])
            continue
        t = b["running_at_limit"]
        if b.get("red_in_file"):
            out["red_already"].append(b["number"])
        elif t is None:
            out["between"].append(b["number"])
        else:
            if t not in tests or "seconds" not in tests[t]:
                raise SplitError(f"timeout_split.py: bug {b['number']} stopped in {t}, which map.json gives no "
                                 "clean time")
            clean = tests[t]["seconds"]
            if clean < fast_below:
                out["fast"].append(b["number"])
            else:
                out["slow"].append(b["number"])
                slow_cleans.append(clean)
    out["slow_median_clean"] = statistics.median(slow_cleans) if slow_cleans else None
    return out


def wait_split(bugs: list[dict], tests: dict[str, dict], src: Sources) -> dict:
    """Each test running at a limit, on the side "waits" or "no wait", with its bugs."""
    by_test: dict[str, list[dict]] = {}
    for b in bugs:
        if b.get("running_at_limit") is not None:
            by_test.setdefault(b["running_at_limit"], []).append(b)
    rows = []
    totals = {"waits": [0, 0, 0], "no wait": [0, 0, 0]}
    for tid, bs in by_test.items():
        if tid not in tests:
            raise SplitError(f"timeout_split.py: {tid} is not in map.json")
        rel = tests[tid]["file"].replace("\\", "/")
        direct, used = src.reaches_wait(rel, tests[tid]["line"])
        side = "waits" if (direct or used) else "no wait"
        numbers = [b["number"] for b in bs]
        no_red = [b["number"] for b in bs if not b.get("red_in_file")]
        totals[side][0] += 1
        totals[side][1] += len(numbers)
        totals[side][2] += len(no_red)
        rows.append({"test": tid, "file": rel, "line": tests[tid]["line"], "side": side, "direct": direct,
                     "helpers": used, "bugs": numbers, "no_red": no_red})
    rows.sort(key=lambda r: (r["side"] != "waits", r["file"], r["line"]))
    return {"totals": totals, "rows": rows}


def split_text(groups: dict, waits: dict, fast_below: float) -> str:
    total = sum(len(groups[g]) for g in GROUPS) + len(groups["no_result"])
    out = [f"Timed-out bugs: {total}."]
    for g in GROUPS:
        out.append(f"{len(groups[g])} {GROUP_WORDS[g]}" + (f" (line: {fast_below} s)." if g in ("fast", "slow") else "."))
    if groups["no_result"]:
        out.append(f"{len(groups['no_result'])} left no result file, so no test is named: "
                   + ", ".join(map(str, groups["no_result"])))
    if groups["slow_median_clean"] is not None:
        out.append(f"Median clean time of the slow group's tests, per bug: {groups['slow_median_clean']:.2f} s.")
    out.append("")
    for side in ("waits", "no wait"):
        tests, bugs, no_red = waits["totals"][side]
        word = "reach a wait" if side == "waits" else "reach no wait"
        out.append(f"{tests} tests {word}. They hold {bugs} timed-out bugs, {no_red} of them with no failing test.")
    out.append("")
    out.append("side | bugs | with no failing test | test | how it reaches a wait | bug numbers")
    for r in waits["rows"]:
        how = ", ".join((["named in the test"] if r["direct"] else []) + r["helpers"]) or "-"
        out.append(f"{r['side']} | {len(r['bugs'])} | {len(r['no_red'])} | {r['test']} | {how} | "
                   + ", ".join(map(str, r["bugs"])))
    return "\n".join(out) + "\n"


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Split a hydra_mutate run's timed-out bugs by the test running at "
                                             "the limit.")
    ap.add_argument("run_dir", type=Path, help="the run folder run.py wrote")
    ap.add_argument("repo", type=Path, help="the repo root whose tests/ the run was built from")
    ap.add_argument("--fast-below", type=float, required=True,
                    help="a test's clean seconds below which it counts as fast")
    a = ap.parse_args(argv)
    try:
        tests = read_map_tests(a.run_dir)
        bugs = timed_out_bugs(a.run_dir)
        src = Sources(a.repo)
        text = split_text(split_groups(bugs, tests, a.fast_below), wait_split(bugs, tests, src), a.fast_below)
    except (SplitError, report.ReportError) as e:
        print(e, file=sys.stderr)
        return 1
    sys.stdout.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
