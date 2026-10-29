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

Run from the repo root, with the game already playing the song:

    py -I tools/ch_mcp/trace_hits.py --mode count \\
        --song "C:\\Clone Hero\\songs\\Hydra Probe\\Window Map" \\
        --engine-class <name> --out <scratch>\\trace-count.jsonl
    py -I tools/ch_mcp/trace_hits.py --mode args --method <name> \\
        --song "C:\\Clone Hero\\songs\\Hydra Probe\\Window Map" \\
        --engine-class <name> --out <scratch>\\trace-args.jsonl
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import threading
from dataclasses import dataclass
from typing import Any, Callable

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe.probe_songs import song_length_ms  # noqa: E402

DEFAULT_AGENT = os.path.join(_HERE, "agent", "trace.js")

MODE_COUNT = "count"
MODE_ARGS = "args"

# The trace's cap is the song's length plus this margin (plan 2026-10-28,
# question 5: the same cap as the clock run).
TRACE_CAP_MARGIN_S = 30.0


# ---- the song -----------------------------------------------------------------

def load_manifest(song_folder: str) -> tuple[str, list[dict]]:
    """The manifest path and its notes, each with `index` and `time_ms`
    (probe_songs.manifest owns the shape)."""
    path = os.path.join(song_folder, "manifest.json")
    with open(path, encoding="utf-8") as fh:
        return path, json.load(fh)["notes"]


def trace_cap_s(notes: list[dict]) -> float:
    """The song's length (probe_songs.song_length_ms) plus the margin."""
    return C.ms_to_s(song_length_ms(notes[-1]["time_ms"])) + TRACE_CAP_MARGIN_S


# ---- passing the settings to the agent --------------------------------------
# run_agent posts {"type": "start", "cap_s": ...} itself. These wrappers add
# the trace settings to that one message and pass everything else through.

class _Script:
    def __init__(self, script: Any, settings: dict):
        self._script = script
        self._settings = settings

    def on(self, *a: Any) -> Any:
        return self._script.on(*a)

    def load(self) -> Any:
        return self._script.load()

    def post(self, message: Any, *a: Any) -> Any:
        if isinstance(message, dict) and message.get("type") == "start":
            message = {**message, **self._settings}
        return self._script.post(message, *a)


class _Session:
    def __init__(self, session: Any, settings: dict):
        self._session = session
        self._settings = settings

    def on(self, *a: Any) -> Any:
        return self._session.on(*a)

    def create_script(self, source: str) -> _Script:
        return _Script(self._session.create_script(source), self._settings)

    def detach(self) -> Any:
        return self._session.detach()


class StartWith:
    """A frida module stand-in whose scripts get `settings` in their start
    message."""

    def __init__(self, frida_mod: Any, settings: dict):
        self._frida = frida_mod
        self._settings = settings
        self.ProcessNotFoundError = frida_mod.ProcessNotFoundError

    def attach(self, name: str) -> _Session:
        return _Session(self._frida.attach(name), self._settings)


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


def _payloads_with(result: FA.RunResult, key: str) -> list[dict]:
    return [p for p in result.payloads if isinstance(p, dict) and key in p]


def ended_line(result: FA.RunResult) -> str:
    """How the run ended, plus the agent's own reason when it sent one
    ("song over", or "cap" when it stopped just before the runner's cap)."""
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
        out(f"{args.agent} not found; run npm run build:trace in "
            f"tools/ch_mcp/agent first")
        return 1

    manifest_path, notes = load_manifest(args.song)
    cap_s = trace_cap_s(notes)
    settings = {
        "mode": args.mode,
        "engine_class": args.engine_class,
        "engine_ctor_rva": args.engine_ctor_rva,
        "hit_time_offset": C.OFF_HIT_TIME,
        "methods": args.method,
    }
    if frida_mod is None:
        import frida as frida_mod  # noqa: PLC0415
    # run_agent's own wait unless a test passes one.
    waits = {} if wait is None else {"wait": wait}
    result = FA.run_agent(args.agent, cap_s, FA.ends_run, args.out,
                          frida_mod=StartWith(frida_mod, settings),
                          out=out, **waits)
    if result.reason == FA.END_NOT_RUNNING:
        return 1
    report = count_report if args.mode == MODE_COUNT else args_report
    for line in report(result, manifest_path, notes):
        out(line)
    out(f"messages written to {args.out}")
    return 1 if result.error is not None else 0


if __name__ == "__main__":
    sys.exit(main())
