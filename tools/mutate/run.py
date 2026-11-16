"""The runner of hydra_mutate: one mapping run, then one fresh process per bug.

The rewriter (rewrite.py) writes the switched copies and mutants.json into
the mutation build folder, and the main session builds hydra_tests from them
with the mutation preset. This script then drives that one build:

    py -I tools/mutate/run.py [--build build-mutation] [--processes N] [--out <run folder>]

1. The mapping run. hydra_tests.exe runs once with HYDRA_MUTANT unset, so
   every switch is off, and with HYDRA_MUTANT_OUT naming a result file. The
   listener (listener.cpp, whose header is the file format's one home) writes
   which switches each test reached. A test that fails here is "red with no
   bug": its verdict is left out of every bug run.
2. The bug runs. For each switch at least one test reaches, a fresh
   hydra_tests.exe starts with HYDRA_MUTANT=<switch> and a doctest source-file
   filter naming only the test files that hold a reaching test (filter_arg).
   A switch noted outside any test, such as in static start-up, is set before
   any test begins, so its run takes every test file. --processes runs that
   many side by side; the default is the machine's physical core count, read
   at run time. Each run has the time limit test_timeout_seconds gives.

What counts is only doctest's own verdict, read from the listener's file. A
test that goes red on a bug the map says it does not reach is an error, not
a catch. A run whose file is missing, cut short, or does not agree with its
exit code is listed as incomplete and never counted.

The runner never builds and never reads compiler output. A missing build,
or one older than its mutants.json, stops it with a message.

What it writes, in the run folder (default <build>/runs/<date-time>):

    map.out               the mapping run's listener file
    map.log               its console output
    map.json              the parsed map: each test, its verdict, the switches
                          it reached, the switches noted outside any test,
                          the run's seconds and exit code
    red_with_no_bug.txt   the tests that failed in the mapping run
    bugs/<n>.out, .log    each bug run's listener file and console output
    bugs/<n>.json         each bug run's result (bug_result says the fields)
    summary.json          the counts, all from the runs above
"""

from __future__ import annotations

import argparse
import concurrent.futures
import datetime
import json
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

MUTANTS_FORMAT = 1  # the mutants.json format rewrite.py writes (its FORMAT)

# hydra_tests' exit codes: doctest's own 0 (all passed) and 1 (some failed).
# Any other code (tests/run_guard.h's refusal, a crash the C runtime ends)
# means the run did not end the way a counted run ends.
EXIT_PASSED = 0
EXIT_FAILED = 1

# The longest command line CreateProcess takes, in characters, counting the
# closing null (the Win32 CreateProcessW documentation, lpCommandLine).
MAX_COMMAND_LINE = 32767


class RunError(RuntimeError):
    pass


def test_timeout_seconds(baseline_seconds: float) -> float:
    """How long a run with one switch on may take before it counts as stuck.

    The user's decision is D82 item 3 in docs/audit/2026-10-03-fix-decisions.md:
    PIT's documented defaults (timeoutFactor, timeoutConst;
    pitest.org/quickstart/commandline), the JVM mutation tool's answer to the
    same question. `baseline_seconds` is how long the same tests took with
    every switch off. tools/mutate/py_mutate.py calls this too.
    """
    return baseline_seconds * 1.25 + 4.0


# ------------------------------------------------------------------ mutants.json


def read_mutants(path: Path) -> dict:
    """mutants.json as rewrite.py wrote it; stops on a missing, partial or other-format file."""
    if not path.is_file():
        raise RunError(f"run.py: no {path}; run tools/mutate/rewrite.py and build first")
    listing = json.loads(path.read_text(encoding="utf-8"))
    if listing.get("format") != MUTANTS_FORMAT:
        raise RunError(f"run.py: {path} is format {listing.get('format')}, not {MUTANTS_FORMAT}")
    if listing.get("complete") is not True:
        raise RunError(f"run.py: {path} is not complete; the rewrite that wrote it stopped part way")
    numbers = [m["number"] for m in listing["mutants"]]
    if len(set(numbers)) != len(numbers):
        raise RunError(f"run.py: {path} lists a switch number twice")
    return listing


# ------------------------------------------------------------------ the listener's file


def unescape_field(text: str) -> str:
    """One field of the listener's file with its escapes (listener.cpp, escape_field) undone."""
    out = []
    i = 0
    while i < len(text):
        c = text[i]
        if c == "\\" and i + 1 < len(text):
            nxt = text[i + 1]
            out.append({"\\": "\\", "t": "\t", "n": "\n", "r": "\r"}.get(nxt, "\\" + nxt))
            i += 2
            continue
        out.append(c)
        i += 1
    return "".join(out)


def _switches(field: str) -> list[int]:
    return [int(x) for x in field.split(",")] if field else []


def test_id(name: str, file: str, line: int) -> str:
    """One test's key: doctest registers a test case by its file, line and name."""
    return f"{file}:{line} {name}"


# Each record kind and its field count, after the kind itself (listener.cpp's header).
_RECORD_FIELDS = {"start": 3, "test": 5, "check": 3, "exception": 2, "outside": 2}


def parse_listener(text: str) -> dict:
    """The listener's file read into tests, checks, exceptions and outside notes.

    Returns {"tests": {id: {"name", "file", "line", "verdict", "switches"}},
    "order": [id, ...], "checks": {id: [[file, line], ...]},
    "exceptions": {id: [message, ...]}, "outside": {when: [switch, ...]},
    "unended": [id, ...], "partial_line": bool}. "unended" holds each test
    that started and has no test record: the one running when the process
    died. "partial_line" is true when the file ends part way through a line.
    """
    tests: dict[str, dict] = {}
    order: list[str] = []
    checks: dict[str, list] = {}
    exceptions: dict[str, list] = {}
    outside: dict[str, list] = {}
    started: dict[str, bool] = {}
    current: Optional[str] = None
    partial = bool(text) and not text.endswith("\n")
    lines = text.split("\n")
    if partial:
        lines = lines[:-1]
    for raw in lines:
        if raw == "":
            continue
        fields = [unescape_field(f) for f in raw.split("\t")]
        kind, rest = fields[0], fields[1:]
        if _RECORD_FIELDS.get(kind) != len(rest):
            raise RunError(f"run.py: a listener line run.py cannot read: {raw!r}")
        if kind == "start":
            current = test_id(rest[0], rest[1], int(rest[2]))
            started[current] = True
        elif kind == "test":
            tid = test_id(rest[0], rest[1], int(rest[2]))
            if tid in tests:
                raise RunError(f"run.py: the listener wrote test {tid} twice")
            if rest[3] not in ("passed", "failed"):
                raise RunError(f"run.py: test {tid} has verdict {rest[3]!r}")
            tests[tid] = {"name": rest[0], "file": rest[1], "line": int(rest[2]),
                          "verdict": rest[3], "switches": _switches(rest[4])}
            order.append(tid)
            started.pop(tid, None)
            current = None
        elif kind == "check":
            if current is None:
                raise RunError(f"run.py: a check record outside any test: {raw!r}")
            checks.setdefault(current, []).append([rest[1], int(rest[2])])
        elif kind == "exception":
            if current is None:
                raise RunError(f"run.py: an exception record outside any test: {raw!r}")
            exceptions.setdefault(current, []).append(rest[1])
        elif kind == "outside":
            outside.setdefault(rest[0], []).extend(_switches(rest[1]))
    return {"tests": tests, "order": order, "checks": checks, "exceptions": exceptions,
            "outside": outside, "unended": sorted(started), "partial_line": partial}


def read_listener(path: Path) -> Optional[dict]:
    """parse_listener on a file, or None when the file is missing."""
    if not path.is_file():
        return None
    return parse_listener(path.read_bytes().decode("utf-8", errors="replace"))


# ------------------------------------------------------------------ the map


def build_map(parsed: dict, listing: dict) -> dict:
    """From the mapping run's parsed file: which tests reach which switch.

    Stops when the run reached a switch mutants.json does not list, because
    then the build does not hold the copies this listing describes.
    """
    listed = {m["number"] for m in listing["mutants"]}
    reach: dict[int, list[str]] = {}
    for tid in parsed["order"]:
        for n in parsed["tests"][tid]["switches"]:
            reach.setdefault(n, []).append(tid)
    outside = sorted({n for ns in parsed["outside"].values() for n in ns})
    unknown = sorted((set(reach) | set(outside)) - listed)
    if unknown:
        raise RunError(f"run.py: the mapping run reached switches {unknown[:20]} that mutants.json does not "
                       "list; the build is not of these copies")
    red = [tid for tid in parsed["order"] if parsed["tests"][tid]["verdict"] == "failed"]
    return {"tests": parsed["tests"], "order": parsed["order"], "reach": reach, "outside": outside,
            "red_with_no_bug": red}


def reached_switches(m: dict) -> list[int]:
    """Every switch at least one test reaches, or that was noted outside any test."""
    return sorted(set(m["reach"]) | set(m["outside"]))


def files_for(n: int, m: dict) -> Optional[list[str]]:
    """The test files a run of switch n takes, or None for every test file.

    A switch noted outside any test is set before any test starts, so no one
    test file can be named for it.
    """
    if n in m["outside"]:
        return None
    return sorted({m["tests"][tid]["file"] for tid in m["reach"].get(n, [])})


def filter_arg(files: Optional[list[str]]) -> list[str]:
    """The doctest arguments that run only these test files, matched whole.

    doctest splits a filter at each comma not escaped with a backslash, and
    reads a doubled backslash as one (parseCommaSepArgs in doctest.h), so
    both are escaped. One -sf argument holds every file, because hydra_tests
    refuses a filter given twice (tests/run_guard.h). Each file is the exact
    path doctest registered, as the listener wrote it; doctest matches it
    with * and ? as wildcards, so a path holding either stops the run.
    """
    if files is None:
        return []
    if not files:
        raise RunError("run.py: a bug run with no test file to run")
    parts = []
    for f in files:
        if "*" in f or "?" in f:
            raise RunError(f"run.py: the test file {f} holds a doctest wildcard, so no filter names it alone")
        parts.append(f.replace("\\", "\\\\").replace(",", "\\,"))
    return ["-sf=" + ",".join(parts)]


def expected_tests(files: Optional[list[str]], m: dict) -> list[str]:
    """The mapping run's tests a run on these files must report."""
    if files is None:
        return list(m["order"])
    wanted = set(files)
    return [tid for tid in m["order"] if m["tests"][tid]["file"] in wanted]


# ------------------------------------------------------------------ one bug's verdict


def bug_result(n: int, m: dict, files: Optional[list[str]], parsed: Optional[dict], exit_code: Optional[int],
               seconds: float, timed_out: bool) -> dict:
    """One bug run's result, from its parsed file and how its process ended.

    "state" is one of: "timed_out" (stopped at the time limit), "incomplete"
    (not counted; "why" says what was wrong), "caught" (a test that reaches
    the bug went red) or "survived". "caught_by" lists each red test that
    reaches the bug, with its failed check lines and exception messages.
    "red_not_reaching" lists each red test the map says does not reach it:
    an error, never a catch. Tests red with no bug are left out of both.
    """
    out = {"number": n, "files": files, "exit_code": exit_code, "seconds": round(seconds, 3),
           "timed_out": timed_out, "state": None, "why": [], "caught_by": [], "red_not_reaching": []}
    if timed_out:
        out["state"] = "timed_out"
        return out
    why = []
    if parsed is None:
        why.append("the run wrote no result file")
    else:
        if parsed["partial_line"]:
            why.append("the result file ends part way through a line")
        if parsed["unended"]:
            why.append("tests started and never ended: " + ", ".join(parsed["unended"]))
        expected = expected_tests(files, m)
        missing = [t for t in expected if t not in parsed["tests"]]
        expected_set = set(expected)
        extra = [t for t in parsed["order"] if t not in expected_set]
        if missing:
            why.append(f"{len(missing)} tests the map runs in these files have no result, first {missing[0]}")
        if extra:
            why.append(f"{len(extra)} tests the map does not hold ran, first {extra[0]}")
        any_red = any(parsed["tests"][t]["verdict"] == "failed" for t in parsed["order"])
        if exit_code not in (EXIT_PASSED, EXIT_FAILED):
            why.append(f"exit code {exit_code}, which is not doctest's own")
        elif (exit_code == EXIT_FAILED) != any_red:
            why.append(f"exit code {exit_code} does not match the file, which has "
                       f"{'a' if any_red else 'no'} failed test")
    if why:
        out["state"] = "incomplete"
        out["why"] = why
        if parsed is not None:
            out["red_in_file"] = [t for t in parsed["order"] if parsed["tests"][t]["verdict"] == "failed"]
        return out
    reachers = set(m["reach"].get(n, []))
    via_outside = n in m["outside"]
    skip = set(m["red_with_no_bug"])
    for tid in parsed["order"]:
        if parsed["tests"][tid]["verdict"] != "failed" or tid in skip:
            continue
        if tid in reachers or via_outside:
            out["caught_by"].append({"test": tid, "checks": parsed["checks"].get(tid, []),
                                     "exceptions": parsed["exceptions"].get(tid, [])})
        else:
            out["red_not_reaching"].append(tid)
    out["state"] = "caught" if out["caught_by"] else "survived"
    return out


# ------------------------------------------------------------------ processes


def physical_cores() -> int:
    """The machine's physical core count, as Windows reports it now."""
    cmd = ["powershell.exe", "-NoProfile", "-NonInteractive", "-Command",
           "(Get-CimInstance Win32_Processor | Measure-Object -Property NumberOfCores -Sum).Sum"]
    try:
        text = subprocess.run(cmd, capture_output=True, text=True, timeout=60, check=True).stdout.strip()
        n = int(text)
    except (OSError, subprocess.SubprocessError, ValueError) as e:
        raise RunError(f"run.py: could not read the physical core count ({e}); give --processes") from e
    if n < 1:
        raise RunError(f"run.py: Windows reported {n} physical cores; give --processes")
    return n


def _kill_tree(p: subprocess.Popen) -> None:
    # hydra_tests starts other programs in some tests, so the whole tree goes.
    subprocess.run(["taskkill", "/T", "/F", "/PID", str(p.pid)], capture_output=True)
    p.wait()


def run_tests(exe: Path, args: list[str], out_file: Path, log_file: Path, cwd: Path,
              mutant: Optional[int], timeout: Optional[float]) -> tuple[Optional[int], float, bool]:
    """One fresh hydra_tests.exe. Returns (exit code, seconds, timed out)."""
    env = dict(os.environ)
    env.pop("HYDRA_MUTANT", None)
    if mutant is not None:
        env["HYDRA_MUTANT"] = str(mutant)
    env["HYDRA_MUTANT_OUT"] = str(out_file)
    cmd = [str(exe)] + args
    if len(subprocess.list2cmdline(cmd)) + 1 > MAX_COMMAND_LINE:
        raise RunError(f"run.py: the command line for {out_file.name} is longer than Windows takes")
    out_file.unlink(missing_ok=True)
    t0 = time.perf_counter()
    with open(log_file, "wb") as log:
        p = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT, env=env, cwd=str(cwd))
        try:
            code = p.wait(timeout=timeout)
            timed_out = False
        except subprocess.TimeoutExpired:
            _kill_tree(p)
            code = None
            timed_out = True
    return code, time.perf_counter() - t0, timed_out


def run_bug(n: int, m: dict, exe: Path, bugs_dir: Path, cwd: Path, limit: float) -> dict:
    files = files_for(n, m)
    out_file = bugs_dir / f"{n}.out"
    code, seconds, timed_out = run_tests(exe, filter_arg(files), out_file, bugs_dir / f"{n}.log", cwd, n, limit)
    result = bug_result(n, m, files, read_listener(out_file), code, seconds, timed_out)
    result["time_limit_seconds"] = round(limit, 3)
    (bugs_dir / f"{n}.json").write_text(json.dumps(result, indent=1) + "\n", encoding="utf-8")
    return result


# ------------------------------------------------------------------ the run


def check_build(build: Path, mutants_path: Path) -> Path:
    exe = build / "Release" / "hydra_tests.exe"
    if not exe.is_file():
        raise RunError(f"run.py: no {exe}; the main session builds it with "
                       "build_cpp.ps1 -Preset mutation -Target hydra_tests")
    if exe.stat().st_mtime < mutants_path.stat().st_mtime:
        raise RunError(f"run.py: {exe} is older than {mutants_path}, so it was not built from these copies; "
                       "build again")
    return exe


def summarize(listing: dict, m: dict, map_seconds: float, map_exit: int, processes: int,
              results: list[dict], wall_seconds: float) -> dict:
    """The counts of one run, every one read from the runs' results."""
    listed = [x["number"] for x in listing["mutants"]]
    reached = reached_switches(m)
    states: dict[str, int] = {}
    for r in results:
        states[r["state"]] = states.get(r["state"], 0) + 1
    return {
        "switches_listed": len(listed),
        "spots_skipped": len(listing["skipped"]),
        "switches_reached": len(reached),
        "switches_reached_by_no_test": sorted(set(listed) - set(reached)),
        "switches_noted_outside_any_test": m["outside"],
        "tests_in_mapping_run": len(m["order"]),
        "red_with_no_bug": len(m["red_with_no_bug"]),
        "mapping_run_seconds": round(map_seconds, 3),
        "mapping_run_exit_code": map_exit,
        "time_limit_seconds": round(test_timeout_seconds(map_seconds), 3),
        "processes": processes,
        "bug_runs": len(results),
        "bug_runs_by_state": states,
        "bug_runs_incomplete": [r["number"] for r in results if r["state"] == "incomplete"],
        "bug_runs_timed_out": [r["number"] for r in results if r["state"] == "timed_out"],
        "errors_red_not_reaching": [{"number": r["number"], "test": t} for r in results
                                    for t in r["red_not_reaching"]],
        "bug_run_seconds_total": round(sum(r["seconds"] for r in results), 3),
        "bug_phase_wall_seconds": round(wall_seconds, 3),
    }


def main(argv=None) -> int:
    root = Path(__file__).resolve().parents[2]
    ap = argparse.ArgumentParser(description="Run the mutation build: one mapping run, then one process per bug.")
    ap.add_argument("--build", type=Path, default=root / "build-mutation",
                    help="the mutation build folder, holding mutants.json and Release/hydra_tests.exe "
                         "(default: build-mutation in the checkout holding this script)")
    ap.add_argument("--processes", type=int, default=None,
                    help="bug runs side by side (default: the physical core count, read at run time)")
    ap.add_argument("--out", type=Path, default=None,
                    help="the run folder, which must not exist yet (default: <build>/runs/<date-time>)")
    ap.add_argument("--map-only", action="store_true", help="stop after the mapping run")
    a = ap.parse_args(argv)
    try:
        return _run(a)
    except RunError as e:
        print(e, file=sys.stderr)
        return 1


def _run(a) -> int:
    build = a.build.resolve()
    mutants_path = build / "mutants.json"
    listing = read_mutants(mutants_path)
    exe = check_build(build, mutants_path)
    if a.processes is not None and a.processes < 1:
        raise RunError("run.py: --processes must be 1 or more")
    processes = a.processes if a.processes is not None else physical_cores()
    run_dir = (a.out or build / "runs" / datetime.datetime.now().strftime("%Y%m%d-%H%M%S")).resolve()
    if run_dir.exists():
        raise RunError(f"run.py: the run folder exists already: {run_dir}")
    run_dir.mkdir(parents=True)
    cwd = build.parent

    print(f"run.py: mapping run of {exe}, every switch off")
    map_out = run_dir / "map.out"
    map_exit, map_seconds, _ = run_tests(exe, [], map_out, run_dir / "map.log", cwd, None, None)
    parsed = read_listener(map_out)
    if parsed is None:
        raise RunError(f"run.py: the mapping run wrote no result file; see {run_dir / 'map.log'}")
    if parsed["partial_line"] or parsed["unended"]:
        raise RunError(f"run.py: the mapping run was cut short; see {run_dir / 'map.log'}")
    m = build_map(parsed, listing)
    any_red = bool(m["red_with_no_bug"])
    if map_exit not in (EXIT_PASSED, EXIT_FAILED) or (map_exit == EXIT_FAILED) != any_red:
        raise RunError(f"run.py: the mapping run ended with exit code {map_exit}, which does not match its "
                       f"{len(m['red_with_no_bug'])} failed tests; see {run_dir / 'map.log'}")
    (run_dir / "map.json").write_text(json.dumps({
        "seconds": round(map_seconds, 3), "exit_code": map_exit,
        "tests": [dict(m["tests"][t], id=t) for t in m["order"]],
        "outside": parsed["outside"],
        "reach": {str(n): m["reach"][n] for n in sorted(m["reach"])},
    }, indent=1) + "\n", encoding="utf-8")
    (run_dir / "red_with_no_bug.txt").write_text("".join(t + "\n" for t in m["red_with_no_bug"]),
                                                  encoding="utf-8")
    reached = reached_switches(m)
    print(f"run.py: mapping run {len(m['order'])} tests in {map_seconds:.1f} s; "
          f"{len(m['red_with_no_bug'])} red with no bug; {len(reached)} of {len(listing['mutants'])} "
          "switches reached")
    for t in m["red_with_no_bug"]:
        print(f"  red with no bug: {t}")

    results: list[dict] = []
    wall = 0.0
    if not a.map_only:
        limit = test_timeout_seconds(map_seconds)
        bugs_dir = run_dir / "bugs"
        bugs_dir.mkdir()
        print(f"run.py: {len(reached)} bug runs, {processes} side by side, time limit {limit:.1f} s each")
        t0 = time.perf_counter()
        with concurrent.futures.ThreadPoolExecutor(max_workers=processes) as pool:
            futures = [pool.submit(run_bug, n, m, exe, bugs_dir, cwd, limit) for n in reached]
            for f in concurrent.futures.as_completed(futures):
                results.append(f.result())
        wall = time.perf_counter() - t0
        results.sort(key=lambda r: r["number"])
    summary = summarize(listing, m, map_seconds, map_exit, processes, results, wall)
    (run_dir / "summary.json").write_text(json.dumps(summary, indent=1) + "\n", encoding="utf-8")
    print(f"run.py: {json.dumps(summary['bug_runs_by_state'])}; "
          f"{len(summary['errors_red_not_reaching'])} red on a bug they do not reach; results in {run_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
