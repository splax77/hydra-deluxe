"""Trace the drum engine's methods while a probe song plays, and find the one
that runs once per note (plan 2026-10-28, Task D and live step L3).

It runs agent/trace.js through frida_attach.run_agent, then reads the song's
manifest.json. Count mode prints the methods whose call count equals the
song's note count, the ones off by one, and the whole table. Args mode traces
only the methods named with --method and lines each call up with the nearest
manifest note.

It reads and traces only; the safety rules are frida_attach's. The engine
class is named on the command line: its obfuscated name, from the dump, and
its constructor RVA (default ch_probe.constants.RVA_DRUMS_ENGINE_CTOR), which
the agent checks along with the rest of the class's shape.

`TraceSession` runs the same args-mode trace in a background thread, so a
caller such as the MCP server can read the calls while the song plays and stop
the trace early (plan 2026-10-28-ch-mcp-server, task T4). Its notes come from
`song_notes`: a probe song's manifest, or a `hydra_replay score` JSON.

Run from the repo root, with the game already playing the song. Do not add
`-I`: the frida module lives in the user's site-packages, which `-I` skips.

    py tools/ch_mcp/trace_hits.py --mode count \\
        --song "C:\\Clone Hero\\songs\\Hydra Probe\\Window Map" \\
        --engine-class <name> --out <scratch>\\trace-count.jsonl
    py tools/ch_mcp/trace_hits.py --mode args --method <name> \\
        --song "C:\\Clone Hero\\songs\\Hydra Probe\\Window Map" \\
        --engine-class <name> --out <scratch>\\trace-args.jsonl
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import threading
import time
from dataclasses import dataclass
from typing import Any, Callable

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.experiments.play_chart import load_dump_chords  # noqa: E402
from tools.ch_probe.probe_songs import song_length_ms  # noqa: E402

DEFAULT_AGENT_NAME = "trace"
DEFAULT_AGENT = os.path.join(_HERE, "agent", f"{DEFAULT_AGENT_NAME}.js")

MODE_COUNT = "count"
MODE_ARGS = "args"

# How a TraceSession ended, beside frida_attach's END_* reasons.
END_STOPPED = "stopped"  # the session's own stop() ended the run
END_FAILED = "failed"    # run_agent raised instead of returning a result

# The trace's song cap is the song's length plus this margin (plan 2026-10-28,
# question 5: the same cap as the clock run).
TRACE_CAP_MARGIN_S = 30.0

# The wall-clock cap is the song cap plus this room, for a song paused or not
# yet started (approved by the user on 2026-10-28).
WALL_CAP_PAUSE_S = 120.0


# ---- the song -----------------------------------------------------------------

class NotesMissing(Exception):
    """Neither a manifest nor a score JSON gave the song's notes."""


def manifest_path(song_folder: str) -> str:
    """Where a probe song keeps its manifest."""
    return os.path.join(song_folder, "manifest.json")


def load_manifest(song_folder: str) -> tuple[str, list[dict]]:
    """The manifest path and its notes, each with `index` and `time_ms`
    (probe_songs.manifest owns the shape)."""
    path = manifest_path(song_folder)
    with open(path, encoding="utf-8") as fh:
        return path, json.load(fh)["notes"]


def notes_from_score(score_json_path: str) -> list[dict]:
    """A `hydra_replay score` JSON's chords as notes in the manifest's shape,
    one per chord. play_chart.load_dump_chords owns reading the file."""
    return [{"index": chord.index, "time_ms": chord.ms}
            for chord in load_dump_chords(score_json_path)]


def song_notes(song_folder: str,
               score_json: str | None = None) -> tuple[str, list[dict]]:
    """The notes a trace lines its calls up with, and the file they came
    from: the song folder's manifest when it has one, else the score JSON."""
    manifest = manifest_path(song_folder)
    if os.path.isfile(manifest):
        return load_manifest(song_folder)
    if score_json is not None and os.path.isfile(score_json):
        return score_json, notes_from_score(score_json)
    score = (f"no score JSON at {score_json}" if score_json is not None
             else "no score JSON was given")
    raise NotesMissing(f"no notes to line the calls up with: no manifest at "
                       f"{manifest}, and {score}")


def trace_cap_s(notes: list[dict]) -> float:
    """The song cap: the song's length (probe_songs.song_length_ms) plus the
    margin. The agent counts it in song time played, so pauses do not use it
    up."""
    return C.ms_to_s(song_length_ms(notes[-1]["time_ms"])) + TRACE_CAP_MARGIN_S


def wall_cap_s(notes: list[dict]) -> float:
    """The wall-clock cap: the song cap plus WALL_CAP_PAUSE_S. The runner
    detaches on it, so a song that never starts or stays paused still ends."""
    return trace_cap_s(notes) + WALL_CAP_PAUSE_S


# ---- reading the results ---------------------------------------------------------

def entry_label(entry: dict) -> str:
    return f"{' / '.join(entry['names'])} ({entry['owner']} {entry['rva']})"


def once_per_note(entries: list[dict], note_count: int) -> tuple[list[dict], list[dict]]:
    """The entries called exactly `note_count` times, and those called one
    more or one fewer."""
    exact = [e for e in entries if e["count"] == note_count]
    off_by_one = [e for e in entries if abs(e["count"] - note_count) == 1]
    return exact, off_by_one


@dataclass
class LinedUp:
    call: dict
    call_ms: float
    note_index: int
    note_ms: float
    diff_ms: float  # the call's time minus the note's


def line_up(calls: list[dict], notes: list[dict]) -> list[LinedUp]:
    """Each call next to the manifest note nearest its songTime. On a tie the
    earlier note wins."""
    out = []
    for call in calls:
        call_ms = C.s_to_ms(call["songTime"])
        best = min(notes, key=lambda n: abs(call_ms - n["time_ms"]))
        out.append(LinedUp(call, call_ms, best["index"], best["time_ms"],
                           call_ms - best["time_ms"]))
    return out


def count_within(lined: list[LinedUp], limit_ms: float) -> int:
    """How many calls sit within `limit_ms` of their nearest note, either
    side, the limit itself included."""
    return sum(1 for x in lined if abs(x.diff_ms) <= limit_ms)


def _with_key(payloads: list, key: str) -> list[dict]:
    return [p for p in payloads if isinstance(p, dict) and key in p]


def _payloads_with(result: FA.RunResult, key: str) -> list[dict]:
    return _with_key(result.payloads, key)


def ended_line(result: FA.RunResult) -> str:
    """How the run ended, plus the agent's own reason when it sent one:
    "song over", "song cap" when the song time played reached the song cap,
    or "cap" when it stopped just before the runner's wall-clock cap."""
    done = _payloads_with(result, "done")
    said = f" (agent: {done[-1]['done']})" if done else ""
    return f"ended: {result.reason}{said}"


def engine_lines(result: FA.RunResult) -> list[str]:
    found = _payloads_with(result, "engine")
    if not found:
        return ["engine: not reported"]
    e = found[0]["engine"]
    shape = ", ".join(f"{k} {'yes' if ok else 'no'}"
                      for k, ok in e["shape"].items())
    return [f"engine: {e['name']} in {e['image']}, base {e['parent']}",
            f"shape: {shape}"]


def count_report(result: FA.RunResult, manifest_path: str,
                 notes: list[dict]) -> list[str]:
    lines = [ended_line(result)] + engine_lines(result)
    n = len(notes)
    lines.append(f"notes in {manifest_path}: {n}")
    tables = _payloads_with(result, "methods")
    if not tables:
        lines.append("table: none received")
    else:
        table = tables[-1]
        entries = table["methods"]
        lines.append(f"hooked: {len(entries)} code addresses, "
                     f"{table['calls']} calls; no code: "
                     f"{len(table['no_code'])} methods; not hooked: "
                     f"{len(table['not_hooked'])}")
        for failure in table["not_hooked"]:
            lines.append(f"  not hooked: {failure}")
        exact, off = once_per_note(entries, n)
        lines.append(f"once per note (count {n}): "
                     + ("; ".join(entry_label(e) for e in exact) or "none"))
        lines.append(f"off by one (count {n - 1} or {n + 1}): "
                     + ("; ".join(entry_label(e) for e in off) or "none"))
        lines.append("all methods by count:")
        for e in sorted(entries, key=lambda e: -e["count"]):
            lines.append(f"  {e['count']:>7}  {entry_label(e)}  "
                         f"({', '.join(e['params'])})")
    if result.error is not None:
        lines.append(f"error: {result.error}")
    return lines


def args_report(result: FA.RunResult, manifest_path: str,
                notes: list[dict]) -> list[str]:
    lines = [ended_line(result)] + engine_lines(result)
    lines.append(f"notes in {manifest_path}: {len(notes)}")
    calls = _payloads_with(result, "call")
    lines.append(f"calls: {len(calls)}")
    lined = line_up(calls, notes)
    for i, x in enumerate(lined, start=1):
        lines.append(f"  call {i}: {x.call['call']} at {x.call_ms:.1f} ms, "
                     f"args {json.dumps(x.call['args'], ensure_ascii=False)}; "
                     f"nearest note {x.note_index} at {x.note_ms} ms, "
                     f"diff {x.diff_ms:+.1f} ms")
    limit = C.EXPECT_NORMAL_BACK_MS
    lines.append(f"calls within {limit:g} ms of their nearest note "
                 f"(EXPECT_NORMAL_BACK_MS): {count_within(lined, limit)} "
                 f"of {len(lined)}")
    if result.error is not None:
        lines.append(f"error: {result.error}")
    return lines


# ---- starting a trace ----------------------------------------------------------------

def trace_settings(notes: list[dict], mode: str, engine_class: str,
                   engine_ctor_rva: int, methods: list[str]) -> dict:
    """The settings trace.ts gets in its start message, beside the runner's
    own `type` and `cap_s`."""
    return {
        "song_cap_s": trace_cap_s(notes),
        "mode": mode,
        "engine_class": engine_class,
        "engine_ctor_rva": engine_ctor_rva,
        "hit_time_offset": C.OFF_HIT_TIME,
        "methods": list(methods),
    }


def agent_missing_line(agent_path: str, agent_name: str = DEFAULT_AGENT_NAME) -> str:
    """What to say when a compiled agent is not there. `agent_name` is the
    agent's name in tools/ch_mcp/agent/package.json (build:<name>); the
    trace agent's by default. frida_tools says it for the state agent too."""
    return (f"{agent_path} not found; run npm run build:{agent_name} in "
            f"tools/ch_mcp/agent first")


class AgentMissing(Exception):
    """The compiled trace agent is not built; the message says how to build
    it."""


class TraceSession:
    """One args-mode trace in a background thread: the same run `main` makes,
    but the caller reads the calls while the song plays and may stop it.

    The thread runs frida_attach.run_agent with a wait that returns when the
    run ends on its own or when stop() is called, so stop() makes run_agent
    detach in its own `finally`. The wall cap (wall_cap_s) still ends a run
    nobody stops. `frida_mod`, `wait` and `now` are injectable so the tests
    run without Frida and without sleeping; `wait(event, timeout_s)` has
    run_agent's shape and defaults to its wait."""

    def __init__(self, notes: list[dict], engine_class: str,
                 methods: list[str], out_path: str, *,
                 engine_ctor_rva: int = C.RVA_DRUMS_ENGINE_CTOR,
                 agent_path: str = DEFAULT_AGENT,
                 frida_mod: Any = None,
                 wait: Callable[[threading.Event, float], bool] | None = None,
                 now: Callable[[], float] = time.monotonic,
                 out: Callable[[str], None] | None = None) -> None:
        if not methods:
            raise ValueError("a trace session needs at least one method")
        if not os.path.isfile(agent_path):
            raise AgentMissing(agent_missing_line(agent_path))
        self.notes = notes
        self.out_path = out_path
        self.agent_path = agent_path
        self.cap_s = wall_cap_s(notes)
        self.settings = trace_settings(notes, MODE_ARGS, engine_class,
                                       engine_ctor_rva, methods)
        # The runner's own lines (the cap, a refused detach, not running).
        # They are kept here, not printed: an MCP server's stdout is its
        # protocol channel.
        self.lines: list[str] = []
        self.result: FA.RunResult | None = None
        self._frida_mod = frida_mod
        self._wait = wait if wait is not None else FA._default_wait
        self._now = now
        self._out = out if out is not None else self.lines.append
        self._payloads: list = []
        self._payload_lock = threading.Lock()
        self._stop = threading.Event()
        self._stopped = False
        self._done = threading.Event()
        self._thread = threading.Thread(target=self._run, name="trace-session",
                                        daemon=True)

    def start(self) -> "TraceSession":
        self._thread.start()
        return self

    def _on_message(self, payload: Any) -> str | None:
        with self._payload_lock:
            self._payloads.append(payload)
        return FA.ends_run(payload)

    def _run_wait(self, ended: threading.Event, cap_s: float) -> bool:
        """run_agent's wait: True when the run ended or stop() was called,
        False when `cap_s` passed first. It sleeps on the stop event in the
        runner's own slices, so an end the agent sends is seen within one
        slice."""
        deadline = self._now() + cap_s
        while True:
            if ended.is_set():
                return True
            if self._stop.is_set():
                # End the run before run_agent's detach, as its cap branch
                # does: Frida answers that detach through `detached`, which
                # must not become the run's end. The reason stays run_agent's
                # default, and _run records the stop.
                self._stopped = True
                ended.set()
                return True
            remaining = deadline - self._now()
            if remaining <= 0:
                return False
            self._wait(self._stop, min(remaining, FA._INTERRUPT_SLICE_S))

    def _run(self) -> None:
        try:
            result = FA.run_agent(self.agent_path, self.cap_s,
                                  self._on_message, self.out_path,
                                  start=self.settings,
                                  frida_mod=self._frida_mod,
                                  wait=self._run_wait, out=self._out)
            # Only a run that nothing else ended is "stopped": an agent end or
            # a detach that landed first keeps its own reason. run_agent
            # leaves END_CAP in place when its wait returns without a finish.
            if self._stopped and result.reason == FA.END_CAP:
                result.reason = END_STOPPED
        except BaseException as e:  # noqa: BLE001 - a thread has no caller
            result = FA.RunResult(reason=END_FAILED, cap_s=self.cap_s,
                                  payloads=self.payloads(),
                                  error=f"{type(e).__name__}: {e}")
        self.result = result
        self._done.set()

    def stop(self, cap_s: float) -> bool:
        """Ask the run to end and wait at most `cap_s` for it to detach.
        True when the run has ended."""
        self._stop.set()
        if self._thread.is_alive():
            self._thread.join(cap_s)
        return self.ended()

    def ended(self) -> bool:
        return self._done.is_set()

    def payloads(self) -> list:
        """Every payload the agent has sent so far, in order (a copy)."""
        with self._payload_lock:
            return list(self._payloads)

    def calls(self) -> list[dict]:
        """The `call` payloads seen so far, in order."""
        return _with_key(self.payloads(), "call")

    def lined_up(self, since: int = 0) -> list[LinedUp]:
        """line_up on the calls from index `since` on."""
        return line_up(self.calls()[since:], self.notes)

    def ended_line(self) -> str:
        """ended_line for the finished run, "ended: stopped" when stop()
        ended it."""
        if self.result is None:
            return "ended: not yet; the trace is still running"
        return ended_line(self.result)


# ---- the command line ----------------------------------------------------------------

def main(argv: list[str] | None = None, *, frida_mod: Any = None,
         wait: Callable[[threading.Event, float], bool] | None = None,
         out: Callable[[str], None] = print) -> int:
    ap = argparse.ArgumentParser(
        description="Trace the drum engine's methods during a probe song and "
                    "find the one that runs once per note.")
    ap.add_argument("--mode", required=True, choices=[MODE_COUNT, MODE_ARGS])
    ap.add_argument("--song", required=True,
                    help="the probe song folder holding manifest.json")
    ap.add_argument("--engine-class", required=True,
                    help="the engine class's name in the dump")
    ap.add_argument("--engine-ctor-rva", type=lambda s: int(s, 0),
                    default=C.RVA_DRUMS_ENGINE_CTOR,
                    help="the engine constructor's RVA, checked by the agent")
    ap.add_argument("--method", action="append", default=[],
                    help="args mode: a method to trace (repeatable)")
    ap.add_argument("--agent", default=DEFAULT_AGENT,
                    help="the compiled trace agent")
    ap.add_argument("--out", required=True, help="the JSON-lines output file")
    args = ap.parse_args(argv)
    if args.mode == MODE_ARGS and not args.method:
        ap.error("--mode args needs at least one --method")
    if args.mode == MODE_COUNT and args.method:
        ap.error("--method is for --mode args; count mode traces every method")
    if not os.path.isfile(args.agent):
        out(agent_missing_line(args.agent))
        return 1

    notes_path, notes = load_manifest(args.song)
    settings = trace_settings(notes, args.mode, args.engine_class,
                              args.engine_ctor_rva, args.method)
    result = FA.run_agent(args.agent, wall_cap_s(notes), FA.ends_run,
                          args.out, start=settings, frida_mod=frida_mod,
                          wait=wait, out=out)
    if result.reason == FA.END_NOT_RUNNING:
        return 1
    report = count_report if args.mode == MODE_COUNT else args_report
    for line in report(result, notes_path, notes):
        out(line)
    out(f"messages written to {args.out}")
    return 1 if result.error is not None else 0


if __name__ == "__main__":
    sys.exit(main())
