"""The runner of hydra_mutate: one mapping run, then one fresh process per bug.

The rewriter (rewrite.py) writes the switched copies and mutants.json into
the mutation build folder, and the main session builds hydra_tests from them
with the mutation preset. This script then drives that one build:

    py -I tools/mutate/run.py [--build build-mutation] [--processes N] [--out <run folder>]

1. The mapping run. hydra_tests.exe runs once with HYDRA_MUTANT unset, so
   every switch is off, with HYDRA_MUTANT_OUT naming a result file, and
   with doctest's --duration=true, so its console output gives each test's
   own time (parse_durations). The listener (listener.cpp, whose header is
   the file format's one home) writes which switches each test reached. A
   test that fails here is "red with no bug": its verdict is left out of
   every bug run.
2. The bug runs. For each switch at least one test reaches, a fresh
   hydra_tests.exe starts with HYDRA_MUTANT=<switch> and a doctest source-file
   filter naming only the test files that hold a reaching test (filter_arg).
   A switch noted outside any test, such as in static start-up, is set before
   any test begins, so its run takes every test file. --processes runs that
   many side by side; the default is the machine's physical core count, read
   at run time. Each run has the time limit test_timeout_seconds gives for
   its own clean time (clean_seconds), and its verdict is the bug's: a bug
   no test caught in it is survived.

What counts is only doctest's own verdict, read from the listener's file,
and the evidence of a crash in the files and the exit code (find_crash).
sort_red_tests says which red tests are catches and which are errors. A run
whose file is missing, cut short, or does not agree with its exit code, and
that is no proven crash, is listed as incomplete and never counted.

    py -I tools/mutate/run.py --rescore <run folder> [--build build-mutation]

scores a run folder again from its files with the current rules and starts
no process (rescore says what it reads and rewrites).

The runner never builds and never reads compiler output. A missing build,
or one older than its mutants.json, stops it with a message.

What it writes, in the run folder (default <build>/runs/<date-time>):

    map.out               the mapping run's listener file
    map.log               its console output, stdout only
    map.err               its stderr
    map.json              the parsed map: each test, its verdict, its own
                          seconds, the switches it reached, the switches
                          noted outside any test, the run's seconds and
                          exit code
    red_with_no_bug.txt   the tests that failed in the mapping run
    bugs/<n>.out, .log    each bug run's listener file and console output
    bugs/<n>.json         each bug's result (bug_result says the fields)
    summary.json          the counts, all from the runs above
"""

from __future__ import annotations

import argparse
import concurrent.futures
import datetime
import json
import os
import re
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

    The rule is the run's own clean time plus 5 seconds. The user chose it
    in session 60c3a576 ("own time plus 5s"), for this tool in place of the
    PIT formula of D82 item 3 (docs/audit/2026-10-03-fix-decisions.md).
    `baseline_seconds` is how long the same tests took with every switch
    off; clean_seconds says how run.py adds it up. tools/mutate/py_mutate.py
    calls this too, with its own clean time of the modules a bug run loads.
    """
    return baseline_seconds + 5.0


# The summary's words for the rule test_timeout_seconds applies.
TIME_LIMIT_RULE = "each bug run's clean time (clean_seconds) plus 5 s (test_timeout_seconds)"


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
    "outside_at": [[tests ended before it, [switch, ...]], ...],
    "unended": [id, ...], "running_at_end": id or None, "partial_line": bool}.
    "unended" holds each test that started and has no test record: the one
    running when the process died. "running_at_end" is the test whose start
    is the file's last test record, or None when the last one ended.
    "partial_line" is true when the file ends part way through a line.
    """
    tests: dict[str, dict] = {}
    order: list[str] = []
    checks: dict[str, list] = {}
    exceptions: dict[str, list] = {}
    outside: dict[str, list] = {}
    outside_at: list[list] = []
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
            outside_at.append([len(order), _switches(rest[1])])
    return {"tests": tests, "order": order, "checks": checks, "exceptions": exceptions,
            "outside": outside, "outside_at": outside_at, "unended": sorted(started),
            "running_at_end": current, "partial_line": partial}


def read_listener(path: Path) -> Optional[dict]:
    """parse_listener on a file, or None when the file is missing."""
    if not path.is_file():
        return None
    return parse_listener(read_run_file(path).decode("utf-8", errors="replace"))


# How long a run's file may stay locked before run.py gives up on it.
LOCKED_FILE_WAIT_SECONDS = 10.0


def read_run_file(path: Path) -> bytes:
    """A file a test process wrote, read once Windows lets it be opened.

    Right after a process writes a file, another program, such as a virus
    scanner, can hold it open for a moment, and Windows then refuses to open
    it. The read is tried again until LOCKED_FILE_WAIT_SECONDS have passed.
    A file still locked then raises RunError, which makes a bug run
    incomplete (result_from_files) and stops the mapping run.
    """
    deadline = time.monotonic() + LOCKED_FILE_WAIT_SECONDS
    while True:
        try:
            return path.read_bytes()
        except PermissionError as e:
            if time.monotonic() >= deadline:
                raise RunError(f"run.py: {path} stayed locked for {LOCKED_FILE_WAIT_SECONDS:.0f} s: {e}") from e
            time.sleep(0.25)


def read_log(path: Path) -> Optional[str]:
    """A run's console output, or None when the file is missing."""
    if not path.is_file():
        return None
    return read_run_file(path).decode("utf-8", errors="replace")


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


# ------------------------------------------------------------------ test times


# The mapping run's doctest argument that prints each test's own time.
DURATION_ARG = "--duration=true"

# With --duration, doctest's console reporter ends each test with a header
# (its file and line, then "TEST CASE:  <name>") and "<seconds> s: <name>"
# (logTestStart and test_case_end in ConsoleReporter, in
# third_party/doctest/doctest.h). A failing test's header comes earlier, at
# its first failure, with the test's own lines after it.
_DURATION_HEADER = re.compile(r"^(.+)\((\d+)\):$")
_DURATION_TIME = re.compile(r"^(\d+\.\d+) s: (.*)$")


def parse_durations(log_text: str, m: dict) -> dict[str, float]:
    """Each test's own seconds from a --duration run's console output, by test_id.

    A time line names its test, and that name picks the test from the
    listener's tests in the map `m`. The nearest header before the time line
    is read only when two tests share the name, to tell them apart. So a
    test's own output that breaks into a header does not lose its time.
    Stops, naming the line, when a time line's name picks no test, or picks
    two that the header does not tell apart, and when a test has two times:
    a time is then never guessed.
    """
    by_name: dict[str, list[str]] = {}
    for tid in m["order"]:
        by_name.setdefault(m["tests"][tid]["name"], []).append(tid)
    out: dict[str, float] = {}
    header: Optional[tuple[str, int]] = None
    for raw in log_text.splitlines():
        found = _DURATION_HEADER.match(raw)
        if found:
            header = (found.group(1), int(found.group(2)))
            continue
        timed = _DURATION_TIME.match(raw)
        if not timed:
            continue
        name = timed.group(2)
        named = by_name.get(name, [])
        if not named:
            raise RunError(f"run.py: the test time {raw!r} in the console output names no test the listener "
                           "recorded")
        if len(named) > 1:
            told = [tid for tid in named if header is not None and tid == test_id(name, header[0], header[1])]
            if len(told) != 1:
                raise RunError(f"run.py: the test time {raw!r} in the console output names {len(named)} tests, "
                               "and no header just before it says which")
            named = told
        tid = named[0]
        if tid in out:
            raise RunError(f"run.py: the console output gives test {tid} two times")
        out[tid] = float(timed.group(1))
        # A header belongs to one test's time; the next time needs its own.
        header = None
    return out


def add_test_seconds(m: dict, durations: dict[str, float]) -> None:
    """Puts each mapping-run test's own seconds in the map as its "seconds".

    `durations` is parse_durations of the map. Stops when a test the
    listener recorded has no time, so no clean time is ever a guess.
    """
    missing = [tid for tid in m["order"] if tid not in durations]
    if missing:
        raise RunError(f"run.py: the mapping run's console output gives no time for {len(missing)} tests, "
                       f"first {missing[0]}")
    for tid in m["order"]:
        m["tests"][tid]["seconds"] = durations[tid]


def clean_seconds(files: Optional[list[str]], m: dict, map_seconds: float) -> float:
    """How long a run on these test files took with every switch off.

    It is the sum of the mapping run's own times for the tests in these
    files (add_test_seconds). A run of every file is the whole mapping run,
    so its clean time is that run's seconds.
    """
    if files is None:
        return map_seconds
    return sum(m["tests"][tid]["seconds"] for tid in expected_tests(files, m))


# ------------------------------------------------------------------ one bug's verdict


# doctest's console reporter writes this before the message of an exception
# it reports as a crash (is_crash, in ConsoleReporter::test_case_exception in
# third_party/doctest/doctest.h), and never for an exception a test threw.
DOCTEST_CRASH_TAG = "test case CRASHED: "

# An NTSTATUS whose two top bits are set has error severity (Microsoft's
# NTSTATUS documentation, [MS-ERREF] 2.3). A process an unhandled exception
# or a fast fail ends exits with such a code, for example 0xC0000005 or
# 0xC0000409.
NTSTATUS_ERROR = 0xC0000000
# STATUS_CONTROL_C_EXIT: the process was stopped with Ctrl+C, so it did not
# crash.
NTSTATUS_CONTROL_C_EXIT = 0xC000013A

# The exit code of an abort that no handler stops: abort calls _exit(3) when
# Windows error reporting is off (Microsoft's abort documentation, Remarks),
# and doctest turns it off (_set_abort_behavior in FatalConditionHandler, in
# third_party/doctest/doctest.h). The C runtime sets SIGABRT back to its
# default before it calls doctest's handler (Microsoft's signal
# documentation, Remarks), so an abort that comes while the handler is still
# on its way to exit(EXIT_FAILURE), from another thread or from the
# shutdown that exit runs, ends the process with this code after doctest has
# already reported the crash. Only doctest's SIGABRT handler runs with
# SIGABRT at its default, so only its message goes with this code.
EXIT_ABORT = 3
# The message doctest's SIGABRT handler reports (the std::signal call in
# FatalConditionHandler, third_party/doctest/doctest.h).
DOCTEST_SIGABRT_MESSAGE = "SIGABRT - Abort (abnormal termination) signal"


def _doctest_crash_messages(log_text: Optional[str]) -> list[str]:
    """The messages doctest reported as a crash in a run's console output."""
    if not log_text:
        return []
    return [line.split(DOCTEST_CRASH_TAG, 1)[1].strip() for line in log_text.splitlines()
            if DOCTEST_CRASH_TAG in line]


def find_crash(parsed: Optional[dict], exit_code: Optional[int], log_text: Optional[str],
               expected: list[str]) -> Optional[dict]:
    """The crash a run's files and exit code prove, or None.

    Two kinds count, each only on its own evidence. doctest catches an
    access violation, an abort or a terminate (FatalConditionHandler in
    doctest.h): it reports the running test as failed with the crash message,
    writes its console line with DOCTEST_CRASH_TAG and exits with
    EXIT_FAILURE, or, for an abort only, with EXIT_ABORT when a second abort
    overtakes that exit (EXIT_ABORT says how). So that crash needs the exit
    code that goes with its message, the console line, and the file's last
    test failed with that same message; EXIT_ABORT with no console line is
    no crash. Windows ends a crash
    doctest cannot catch, such as a fast fail, with an error NTSTATUS; that
    crash needs such an exit code and a file whose last record is the start
    of the test that was running. Both need a whole file whose tests are the
    run's expected tests in order, up to the crashed one: anything else is a
    file cut short or a run that went wrong some other way.
    """
    if parsed is None or parsed["partial_line"]:
        return None
    order = parsed["order"]
    if exit_code in (EXIT_FAILED, EXIT_ABORT) and not parsed["unended"] and order:
        last = order[-1]
        messages = _doctest_crash_messages(log_text)
        thrown = parsed["exceptions"].get(last, [])
        said = [msg for msg in messages if msg in thrown
                and (exit_code == EXIT_FAILED or msg == DOCTEST_SIGABRT_MESSAGE)]
        if parsed["tests"][last]["verdict"] == "failed" and said and order == expected[:len(order)]:
            return {"test": last, "how": "doctest caught it: " + said[0]}
        return None
    if (isinstance(exit_code, int) and exit_code & NTSTATUS_ERROR == NTSTATUS_ERROR
            and exit_code != NTSTATUS_CONTROL_C_EXIT):
        running = parsed["running_at_end"]
        if running is not None and parsed["unended"] == [running] and order + [running] == \
                expected[:len(order) + 1]:
            return {"test": running, "how": f"the process ended with exit code 0x{exit_code:08X}"}
    return None


def sort_red_tests(n: int, m: dict, parsed: dict, crashed: Optional[str] = None) -> dict:
    """Each red test of one run, sorted by what the run's own file shows.

    "caught_by": the map says the test reaches the bug, the switch was noted
    outside any test in the mapping run, or the test noted the switch in this
    run itself. "caught_through_shared_state": the test did not reach the
    bug, but an earlier test in the same process did, or the switch was noted
    between tests before it, so the bug's effect could reach it through
    anything the process keeps between tests (tests/corpus_util.h, "cached
    corpus work", keeps each corpus chart's parse and analysis for the whole
    run). "red_unexplained": neither; nothing in this process ran the bug
    before or during the test, so its failure has no cause this run shows,
    and it is an error. Tests red with no bug are left out of all three.

    "crashed" names a test the process died inside, which wrote no ending of
    its own (find_crash's second kind); it is sorted as a red test after the
    file's last one.
    """
    reachers = set(m["reach"].get(n, []))
    via_outside = n in m["outside"]
    skip = set(m["red_with_no_bug"])
    noted_before = {at for at, ns in parsed["outside_at"] if n in ns}
    out = {"caught_by": [], "caught_through_shared_state": [], "red_unexplained": []}
    reached_earlier = False
    order = list(parsed["order"])
    if crashed is not None and crashed not in parsed["tests"]:
        order.append(crashed)
    for i, tid in enumerate(order):
        test = parsed["tests"].get(tid, {"verdict": "failed", "switches": []})
        reached_earlier = reached_earlier or i in noted_before
        if test["verdict"] == "failed" and tid not in skip:
            if tid in reachers or via_outside or n in test["switches"]:
                out["caught_by"].append({"test": tid, "checks": parsed["checks"].get(tid, []),
                                         "exceptions": parsed["exceptions"].get(tid, [])})
            elif reached_earlier:
                out["caught_through_shared_state"].append(tid)
            else:
                out["red_unexplained"].append(tid)
        reached_earlier = reached_earlier or n in test["switches"]
    return out


def bug_result(n: int, m: dict, files: Optional[list[str]], parsed: Optional[dict], exit_code: Optional[int],
               seconds: float, timed_out: bool, log_text: Optional[str] = None,
               parse_error: Optional[str] = None) -> dict:
    """One bug run's result, from its parsed file, its console output and how its process ended.

    "state" is one of: "timed_out" (stopped at the time limit; D82 item 3
    counts it as found), "crashed" (the test program crashed: found; "crash"
    names the test that was running and how the crash shows), "incomplete"
    (not counted; "why" says what was wrong), "caught" (a test went red that
    the bug can explain) or "survived". sort_red_tests says the three lists
    of red tests; a crashed run keeps those recorded before the crash, and a
    timed-out one lists its red tests in "red_in_file". "parse_error" is
    read_listener's refusal of the file, when it refused it.
    """
    out = {"number": n, "files": files, "exit_code": exit_code, "seconds": round(seconds, 3),
           "timed_out": timed_out, "state": None, "why": [], "crash": None, "caught_by": [],
           "caught_through_shared_state": [], "red_unexplained": []}
    if timed_out:
        out["state"] = "timed_out"
        if parsed is not None:
            out["red_in_file"] = [t for t in parsed["order"] if parsed["tests"][t]["verdict"] == "failed"]
            out["running_at_limit"] = parsed["running_at_end"]
        return out
    expected = expected_tests(files, m)
    if parse_error is None:
        crash = find_crash(parsed, exit_code, log_text, expected)
        if crash is not None:
            out["state"] = "crashed"
            out["crash"] = crash
            out.update(sort_red_tests(n, m, parsed, crashed=crash["test"]))
            return out
    why = []
    if parse_error is not None:
        why.append(parse_error)
    elif parsed is None:
        why.append("the run wrote no result file")
    else:
        if parsed["partial_line"]:
            why.append("the result file ends part way through a line")
        if parsed["unended"]:
            why.append("tests started and never ended: " + ", ".join(parsed["unended"]))
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
    out.update(sort_red_tests(n, m, parsed))
    found = out["caught_by"] or out["caught_through_shared_state"]
    out["state"] = "caught" if found else "survived"
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
              mutant: Optional[int], timeout: Optional[float],
              err_file: Optional[Path] = None) -> tuple[Optional[int], float, bool]:
    """One fresh hydra_tests.exe. Returns (exit code, seconds, timed out).

    Its stderr goes to `err_file` when one is given, and into the log with
    its stdout when not. Stdout is buffered and stderr is not, so in one
    file a stderr write can land part way through any stdout line.
    """
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
    with open(log_file, "wb") as log, open(err_file or os.devnull, "wb") as err:
        stderr = err if err_file is not None else subprocess.STDOUT
        p = subprocess.Popen(cmd, stdout=log, stderr=stderr, env=env, cwd=str(cwd))
        try:
            code = p.wait(timeout=timeout)
            timed_out = False
        except subprocess.TimeoutExpired:
            _kill_tree(p)
            code = None
            timed_out = True
    return code, time.perf_counter() - t0, timed_out


def result_from_files(n: int, m: dict, files: Optional[list[str]], out_file: Path, log_file: Path,
                      exit_code: Optional[int], seconds: float, timed_out: bool) -> dict:
    """bug_result on a run's two files; a listener file run.py cannot read makes the run incomplete."""
    try:
        parsed, parse_error = read_listener(out_file), None
    except RunError as e:
        parsed, parse_error = None, str(e)
    try:
        log = read_log(log_file)
    except RunError as e:
        log = None
        parse_error = parse_error or str(e)
    return bug_result(n, m, files, parsed, exit_code, seconds, timed_out, log, parse_error)


def run_bug(n: int, m: dict, exe: Path, bugs_dir: Path, cwd: Path, clean: float) -> dict:
    """The bug's run on the test files that hold a reaching test (files_for).

    `clean` is those files' clean time (clean_seconds); the run's limit is
    test_timeout_seconds of it.
    """
    files = files_for(n, m)
    limit = test_timeout_seconds(clean)
    out_file, log_file = bugs_dir / f"{n}.out", bugs_dir / f"{n}.log"
    code, seconds, timed_out = run_tests(exe, filter_arg(files), out_file, log_file, cwd, n, limit)
    result = result_from_files(n, m, files, out_file, log_file, code, seconds, timed_out)
    result["clean_seconds"] = round(clean, 3)
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


def time_limit_record(limits: list[float]) -> dict:
    """The summary's account of the bug runs' time limits: the rule and the lowest and highest limit given."""
    return {"rule": TIME_LIMIT_RULE,
            "lowest_seconds": round(min(limits), 3) if limits else None,
            "highest_seconds": round(max(limits), 3) if limits else None}


def summarize(listing: dict, m: dict, map_seconds: float, map_exit: int, processes: int,
              results: list[dict], wall_seconds: float, time_limit: dict) -> dict:
    """The counts of one run, every one read from the runs' results.

    `time_limit` is the run's account of its limits (time_limit_record).
    """
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
        "time_limit": time_limit,
        "processes": processes,
        "bug_runs": len(results),
        "bug_runs_by_state": states,
        # Caught, crashed and timed out each mean the tests found the bug
        # (D82 item 3 for timed out); bug_result says each state.
        "bugs_found": sum(1 for r in results if r["state"] in ("caught", "crashed", "timed_out")),
        "bug_runs_crashed": [dict(r["crash"], number=r["number"]) for r in results if r["state"] == "crashed"],
        "bug_runs_incomplete": [r["number"] for r in results if r["state"] == "incomplete"],
        "bug_runs_timed_out": [r["number"] for r in results if r["state"] == "timed_out"],
        "bugs_caught_only_through_shared_state": [r["number"] for r in results if r["state"] == "caught"
                                                  and not r["caught_by"]],
        "caught_through_shared_state": [{"number": r["number"], "test": t} for r in results
                                        for t in r["caught_through_shared_state"]],
        "errors_red_unexplained": [{"number": r["number"], "test": t} for r in results
                                   for t in r["red_unexplained"]],
        "bug_run_seconds_total": round(sum(r["seconds"] for r in results), 3),
        "bug_phase_wall_seconds": round(wall_seconds, 3),
    }


def rescore(run_dir: Path, listing: dict) -> list[dict]:
    """Score a run folder again with this run.py, from the files the run left.

    A run scored by an older run.py is brought up to the current rules this
    way without running anything again. The map is read again from map.out.
    Each bugs/<n>.json is rewritten by result_from_files, given that run's
    .out and .log and the exit code, seconds and timed-out flag its old json
    stored, which are the facts the process gave and no file holds. The time
    limit it was run with, and the clean time that limit came from, are
    kept, so the map needs no test times here. A bug whose stored test files
    are not the ones files_for names now stops the re-score, because its run
    did not run what the current rules would. summary.json, which only a finished run
    has, is written again from the new results, keeping the process count,
    wall time and time limits the run had. Returns the new results by number.
    """
    parsed = read_listener(run_dir / "map.out")
    if parsed is None:
        raise RunError(f"run.py: no {run_dir / 'map.out'}; {run_dir} is not a run folder")
    m = build_map(parsed, listing)
    bugs_dir = run_dir / "bugs"
    olds = []
    for path in bugs_dir.glob("*.json"):
        old = json.loads(path.read_text(encoding="utf-8"))
        if path.stem != str(old["number"]):
            raise RunError(f"run.py: {path} holds bug {old['number']}")
        files = files_for(old["number"], m)
        if files != old["files"]:
            raise RunError(f"run.py: bug {old['number']} was run on {old['files']}, but the map now gives "
                           f"{files}; its run cannot be scored again")
        olds.append(old)
    olds.sort(key=lambda o: o["number"])
    results = []
    for old in olds:
        n = old["number"]
        r = result_from_files(n, m, old["files"], bugs_dir / f"{n}.out", bugs_dir / f"{n}.log",
                              old["exit_code"], old["seconds"], old["timed_out"])
        for kept in ("clean_seconds", "time_limit_seconds"):
            if kept in old:
                r[kept] = old[kept]
        (bugs_dir / f"{n}.json").write_text(json.dumps(r, indent=1) + "\n", encoding="utf-8")
        results.append(r)
    summary_path = run_dir / "summary.json"
    if summary_path.is_file():
        old_summary = json.loads(summary_path.read_text(encoding="utf-8"))
        time_limit = old_summary.get("time_limit")
        if time_limit is None:
            # A summary with no time_limit record holds the one limit every
            # bug run of it had.
            time_limit = {"every_bug_run_seconds": old_summary.get("time_limit_seconds")}
        summary = summarize(listing, m, old_summary["mapping_run_seconds"], old_summary["mapping_run_exit_code"],
                            old_summary["processes"], results, old_summary["bug_phase_wall_seconds"], time_limit)
        summary_path.write_text(json.dumps(summary, indent=1) + "\n", encoding="utf-8")
    return results


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
    ap.add_argument("--rescore", type=Path, default=None, metavar="RUN_FOLDER",
                    help="score a finished or stopped run folder again from its files, with the mutants.json "
                         "in --build; starts no process (rescore says how)")
    a = ap.parse_args(argv)
    try:
        if a.rescore is not None:
            if a.out is not None or a.map_only or a.processes is not None:
                raise RunError("run.py: --rescore takes only --build")
            results = rescore(a.rescore.resolve(), read_mutants(a.build.resolve() / "mutants.json"))
            states: dict[str, int] = {}
            for r in results:
                states[r["state"]] = states.get(r["state"], 0) + 1
            print(f"run.py: re-scored {len(results)} bug runs in {a.rescore}: {json.dumps(states)}")
            return 0
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
    map_log = run_dir / "map.log"
    # The tests' stderr writes go to their own file, so they never break
    # into the lines parse_durations reads.
    map_exit, map_seconds, _ = run_tests(exe, [DURATION_ARG], map_out, map_log, cwd, None, None,
                                         run_dir / "map.err")
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
    add_test_seconds(m, parse_durations(read_log(map_log) or "", m))
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
    limits: list[float] = []
    if not a.map_only:
        cleans = {n: clean_seconds(files_for(n, m), m, map_seconds) for n in reached}
        limits = [test_timeout_seconds(c) for c in cleans.values()]
        bugs_dir = run_dir / "bugs"
        bugs_dir.mkdir()
        span = f"{min(limits):.1f} to {max(limits):.1f} s" if limits else "none"
        print(f"run.py: {len(reached)} bug runs, {processes} side by side, time limits {span} "
              "(each run's clean time plus 5 s)")
        t0 = time.perf_counter()
        with concurrent.futures.ThreadPoolExecutor(max_workers=processes) as pool:
            futures = [pool.submit(run_bug, n, m, exe, bugs_dir, cwd, cleans[n]) for n in reached]
            for f in concurrent.futures.as_completed(futures):
                results.append(f.result())
        wall = time.perf_counter() - t0
        results.sort(key=lambda r: r["number"])
    summary = summarize(listing, m, map_seconds, map_exit, processes, results, wall, time_limit_record(limits))
    (run_dir / "summary.json").write_text(json.dumps(summary, indent=1) + "\n", encoding="utf-8")
    print(f"run.py: {json.dumps(summary['bug_runs_by_state'])}; "
          f"{len(summary['caught_through_shared_state'])} red through shared state; "
          f"{len(summary['errors_red_unexplained'])} red with no cause the run shows; results in {run_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
