"""The report of one hydra_mutate run, written as text files in its run folder.

    py -I tools/mutate/report.py <run folder> [--mutants <mutants.json>] [--out <folder>]

It reads what run.py left in the run folder and the build's mutants.json
(by default the one two folders up, where run.py's default run folder
sits). It never works out a verdict: each bug's state is the one its
bugs/<n>.json holds, and run.py's bug_result is the one owner of scoring.
A run scored by an older run.py is brought up to date first with
run.py --rescore. The map, which test reaches which bug, is read from the
run's map.out by run.py's own build_map.

It writes three files (into --out when given, else the run folder):

    report.txt   the limits, the counts and the lists of bugs by state
    bugs.txt     each bug: file, line, the change, its verdict and every
                 test that went red in its run, with the checks that failed
    tests.txt    each test: how many bugs it reaches and how many it
                 caught, the tests that caught none first

Every count comes from build_report's one result, so report.txt, bugs.txt
and tests.txt cannot disagree.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from pathlib import Path
from typing import Optional

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import run  # noqa: E402

# What each state of bug_result is called in the report. Each verdict comes
# from its bug's one run (the user's answer of session 872cc6f3, "One run,
# say so"), so the words say so.
STATE_WORDS = {
    "caught": "caught in its run",
    "survived": "survived its run",
    "crashed": "crashed in its run",
    "timed_out": "timed out in its run (counted as caught)",
    "incomplete": "incomplete: its run is not counted",
}

# The lists of report.txt, in order: each title and the state its bugs hold.
STATE_LISTS = [
    ("Survivors: bugs reached but caught by no test", "survived"),
    ("Timed-out bugs, counted as caught but listed apart", "timed_out"),
    ("Crashed bugs", "crashed"),
    ("Incomplete runs, not counted", "incomplete"),
    ("Caught bugs", "caught"),
]

OPENING = [
    "Limits of this report.",
    "A test that caught nothing has shown no way to fail against these bugs. That is a question to look at, "
    "not proof the test is empty.",
    "A check that sits after an earlier failing REQUIRE in the same test may never have run.",
    "Each verdict comes from one run of its bug, so it reads \"caught in its run\" or \"survived its run\". "
    "A bug in threaded code, or one that reads past the end of memory, may land either way in another run.",
    "No tool can say whether a survivor is an equivalent bug, one that changes nothing a user could see.",
    "A test that went red in the mapping run, with every switch off, is red with no bug and is left out "
    "of every bug.",
]


# How bugs.txt names each kind of catch, before the test's name.
HOW_RED = "red"
HOW_SHARED = "red through state an earlier test left"
HOW_CRASHED = "crashed while it ran"
CATCH_HOWS = (HOW_RED, HOW_SHARED, HOW_CRASHED)


class ReportError(RuntimeError):
    pass


def catches(bug: dict) -> list[dict]:
    """The tests a bug's json credits with catching it, each once, and how.

    These are the red tests bug_result sorted as catches, and the test that
    was running when the run crashed. Red tests with no cause the run shows
    are errors, not catches; a timed-out or incomplete run sorts none.
    """
    out: list[dict] = []
    seen: set[str] = set()

    def add(test: str, how: str, checks=(), exceptions=()):
        if test not in seen:
            seen.add(test)
            out.append({"test": test, "how": how, "checks": list(checks), "exceptions": list(exceptions)})

    for c in bug["caught_by"]:
        add(c["test"], HOW_RED, c["checks"], c["exceptions"])
    for t in bug["caught_through_shared_state"]:
        add(t, HOW_SHARED)
    if bug["crash"] is not None:
        add(bug["crash"]["test"], HOW_CRASHED)
    return out


def load_bugs(run_dir: Path) -> dict[int, dict]:
    bugs: dict[int, dict] = {}
    for path in (run_dir / "bugs").glob("*.json"):
        bug = json.loads(path.read_text(encoding="utf-8"))
        if path.stem != str(bug["number"]):
            raise ReportError(f"report.py: {path} holds bug {bug['number']}")
        if bug["state"] not in STATE_WORDS:
            raise ReportError(f"report.py: bug {bug['number']} has state {bug['state']!r}, which this report "
                              "has no words for; score the run again with run.py --rescore")
        bugs[bug["number"]] = bug
    return bugs


def build_report(run_dir: Path, mutants_path: Path) -> dict:
    """Everything the three files say, read from the run folder and mutants.json."""
    listing = run.read_mutants(mutants_path)
    parsed = run.read_listener(run_dir / "map.out")
    if parsed is None:
        raise ReportError(f"report.py: no {run_dir / 'map.out'}; {run_dir} is not a run folder")
    m = run.build_map(parsed, listing)
    red_file = run_dir / "red_with_no_bug.txt"
    red_with_no_bug = [t for t in red_file.read_text(encoding="utf-8").splitlines() if t]
    if red_with_no_bug != m["red_with_no_bug"]:
        raise ReportError(f"report.py: {red_file} does not match the mapping run in map.out")
    mutants = {x["number"]: x for x in listing["mutants"]}
    bugs = load_bugs(run_dir)
    unknown = sorted(set(bugs) - set(mutants))
    if unknown:
        raise ReportError(f"report.py: bug runs {unknown} are not in {mutants_path}")
    reached = run.reached_switches(m)

    per_bug = {n: catches(b) for n, b in bugs.items()}
    per_test = {tid: {"reaches": sorted(set(m["tests"][tid]["switches"])), "caught": []} for tid in m["order"]}
    for n in sorted(per_bug):
        for c in per_bug[n]:
            if c["test"] not in per_test:
                raise ReportError(f"report.py: bug {n} credits {c['test']}, which the mapping run does not hold")
            per_test[c["test"]]["caught"].append(n)

    lists = {state: sorted(n for n, b in bugs.items() if b["state"] == state) for _, state in STATE_LISTS}
    return {
        "run": run_dir.name,
        "listing": listing,
        "mutants": mutants,
        "map": m,
        "bugs": bugs,
        "catches": per_bug,
        "tests": per_test,
        "lists": lists,
        "not_reached": sorted(set(mutants) - set(reached)),
        "not_run": sorted(set(reached) - set(bugs)),
        "errors": [(n, t) for n in sorted(bugs) for t in bugs[n]["red_unexplained"]],
        "catch_total": sum(len(c) for c in per_bug.values()),
    }


def _where(mut: dict) -> str:
    return f"{mut['file']} line {mut['line']}, {mut['change']}"


def _bug_line(n: int, r: dict) -> str:
    return f"bug {n}: {_where(r['mutants'][n])}"


def zero_first(r: dict) -> list[str]:
    """The tests in tests.txt's order: fewest catches first, then most bugs reached."""
    return sorted(r["tests"], key=lambda t: (len(r["tests"][t]["caught"]), -len(r["tests"][t]["reaches"]), t))


def report_text(r: dict) -> str:
    listing, m = r["listing"], r["map"]
    out = [f"Mutation report for run {r['run']}.", ""] + OPENING + [""]
    states = ", ".join(f"{s.replace('_', ' ')} {len(r['lists'][s])}" for _, s in STATE_LISTS)
    zero = [t for t in r["tests"] if not r["tests"][t]["caught"]]
    out += [
        "Counts.",
        f"Bugs planted: {len(r['mutants'])}. Spots skipped: {len(listing['skipped'])}.",
        f"Bugs some test reaches: {len(r['mutants']) - len(r['not_reached'])}. "
        f"Bugs no test reaches: {len(r['not_reached'])}.",
        f"Bug runs: {len(r['bugs'])}: {states}.",
        f"Bugs reached but never run, because the run stopped first: {len(r['not_run'])}.",
        f"Tests in the mapping run: {len(m['order'])}. Red with no bug: {len(m['red_with_no_bug'])}. "
        f"Tests that caught no bug: {len(zero)}.",
        f"Catches, each a test and a bug it caught: {r['catch_total']}. bugs.txt and tests.txt count "
        "the same catches.",
        f"Red tests with no cause the run shows (errors): {len(r['errors'])}.",
        "",
    ]
    for title, state in STATE_LISTS:
        out.append(f"== {title} ({len(r['lists'][state])})")
        for n in r["lists"][state]:
            line = _bug_line(n, r)
            bug = r["bugs"][n]
            if state == "crashed":
                line += f"; {bug['crash']['how']}, while {bug['crash']['test']} ran"
            elif state == "incomplete":
                line += "; " + "; ".join(bug["why"])
            out.append(line)
        out.append("")
    out.append(f"== Bugs no test reaches ({len(r['not_reached'])})")
    out += [_bug_line(n, r) for n in r["not_reached"]] + [""]
    out.append(f"== Bugs reached but never run, because the run stopped first ({len(r['not_run'])})")
    out += [_bug_line(n, r) for n in r["not_run"]] + [""]
    out.append(f"== Red tests with no cause the run shows, errors ({len(r['errors'])})")
    out += [f"bug {n}: {t}" for n, t in r["errors"]] + [""]
    out.append(f"== Tests red with no bug, in the mapping run ({len(m['red_with_no_bug'])})")
    out += list(m["red_with_no_bug"]) + [""]
    reasons = listing.get("skip_reasons", {})
    out.append(f"== Spots skipped ({len(listing['skipped'])})")
    for s in listing["skipped"]:
        out.append(f"{s['file']} line {s['line']}, {s['change']}: {reasons.get(s['reason'], s['reason'])}")
    return "\n".join(out) + "\n"


def bugs_text(r: dict) -> str:
    out = ["Each bug run: its verdict and every test that went red in it.", ""]
    for n in sorted(r["bugs"]):
        bug = r["bugs"][n]
        out.append(_bug_line(n, r))
        out.append(f"  {STATE_WORDS[bug['state']]}, {bug['seconds']} s")
        if bug["state"] == "crashed":
            out.append(f"  crash: {bug['crash']['how']}, while {bug['crash']['test']} ran")
        for why in bug["why"]:
            out.append(f"  why: {why}")
        out.append(f"  catches: {len(r['catches'][n])}")
        for c in r["catches"][n]:
            out.append(f"  {c['how']}: {c['test']}")
            # A check in a loop fails once per pass, so each line is given once with its count.
            times: dict[str, int] = {}
            for f, line in c["checks"]:
                times[f"{f}:{line}"] = times.get(f"{f}:{line}", 0) + 1
            for where, k in times.items():
                out.append(f"    failed check at {where}" + (f" ({k} times)" if k > 1 else ""))
            for e in c["exceptions"]:
                out.append(f"    exception: {e}")
        for t in bug["red_unexplained"]:
            out.append(f"  red with no cause the run shows (an error): {t}")
        for t in bug.get("red_in_file", []):
            out.append(f"  red in its file, not sorted because the run {bug['state'].replace('_', ' ')}: {t}")
        if bug.get("running_at_limit"):
            out.append(f"  running at the time limit: {bug['running_at_limit']}")
        out.append("")
    return "\n".join(out)


def tests_text(r: dict) -> str:
    order = zero_first(r)
    zero = [t for t in order if not r["tests"][t]["caught"]]
    some = [t for t in order if r["tests"][t]["caught"]]
    out = ["Each test: how many bugs it caught and how many it reaches in the mapping run.",
           "A test can catch a bug it does not reach, through state an earlier test left (bugs.txt says which).",
           ""]

    def row(t):
        rec = r["tests"][t]
        return f"caught {len(rec['caught'])} | reaches {len(rec['reaches'])} | {t}"

    out.append(f"== Tests that caught no bug ({len(zero)})")
    out += [row(t) for t in zero] + [""]
    out.append(f"== Tests that caught at least one bug ({len(some)})")
    for t in some:
        out.append(row(t))
        out.append("  bugs: " + ", ".join(str(n) for n in r["tests"][t]["caught"]))
    return "\n".join(out) + "\n"


def write_report(run_dir: Path, mutants_path: Path, out_dir: Optional[Path] = None) -> dict:
    r = build_report(run_dir, mutants_path)
    out_dir = out_dir or run_dir
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "report.txt").write_text(report_text(r), encoding="utf-8")
    (out_dir / "bugs.txt").write_text(bugs_text(r), encoding="utf-8")
    (out_dir / "tests.txt").write_text(tests_text(r), encoding="utf-8")
    return r


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="Write the report of one hydra_mutate run.")
    ap.add_argument("run_dir", type=Path, help="the run folder run.py wrote")
    ap.add_argument("--mutants", type=Path, default=None,
                    help="the build's mutants.json (default: two folders above the run folder)")
    ap.add_argument("--out", type=Path, default=None, help="where the report goes (default: the run folder)")
    a = ap.parse_args(argv)
    run_dir = a.run_dir.resolve()
    mutants = (a.mutants or run_dir.parent.parent / "mutants.json").resolve()
    try:
        r = write_report(run_dir, mutants, a.out.resolve() if a.out else None)
    except (ReportError, run.RunError) as e:
        print(e, file=sys.stderr)
        return 1
    print(f"report.py: {len(r['bugs'])} bug runs, {r['catch_total']} catches; report in {a.out or run_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
