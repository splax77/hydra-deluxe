"""Attach Frida to the running game, run one compiled agent, and keep every
message it sends.

`run_agent` is the shared machinery: it attaches, loads the agent, tells it to
start with the cap, writes each message as one JSON line, and detaches in a
`finally` block, so a normal end, the cap, a Python error and Ctrl-C all
detach. Any agent can use it; the command line below runs the clock agent and
prints the clock summary.

It reads and traces only. The safety rules it follows are in
docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md, "Safety rules
for every live step". The agents' no-writes check lives in
tests/test_frida_attach.py.

Run from the repo root, with the game already playing a song:

    py -I tools/ch_mcp/frida_attach.py --agent tools/ch_mcp/agent/clock.js \\
        --cap 93 --out <scratch>\\clock.jsonl
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import threading
from dataclasses import dataclass, field
from typing import Any, Callable

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe.constants import PROCESS_NAME  # noqa: E402

# How a run ended. RunResult.reason holds one of these.
END_DONE = "done"            # the agent said it was finished
END_AGENT_ERROR = "error"    # the agent sent an error and stopped
END_SCRIPT_ERROR = "script error"  # the agent's JavaScript threw
END_CAP = "cap"              # the cap passed before the agent finished
END_DETACHED = "detached"    # the game closed or Frida dropped the session
END_NOT_RUNNING = "not running"


@dataclass
class RunResult:
    reason: str
    cap_s: float
    payloads: list = field(default_factory=list)
    error: str | None = None


def ends_run(payload: Any) -> str | None:
    """The agents' shared end signal: a payload with an `error` key or a
    `done` key ends the run. Returns the end reason, or None to keep going."""
    if isinstance(payload, dict):
        if "error" in payload:
            return END_AGENT_ERROR
        if "done" in payload:
            return END_DONE
    return None


def _default_wait(event: threading.Event, timeout_s: float) -> bool:
    return event.wait(timeout_s)


def run_agent(agent_path: str, cap_s: float,
              on_message: Callable[[Any], str | None], out_path: str, *,
              frida_mod: Any = None,
              wait: Callable[[threading.Event, float], bool] = _default_wait,
              out: Callable[[str], None] = print) -> RunResult:
    """Run the compiled agent at `agent_path` inside the game for at most
    `cap_s` seconds and write every message to `out_path`, one JSON line each.

    `on_message` sees each payload the agent sends; when it returns an end
    reason, the run ends. `frida_mod` and `wait` are injectable so the tests
    run without Frida and without sleeping."""
    if frida_mod is None:
        import frida as frida_mod  # noqa: PLC0415
    with open(agent_path, encoding="utf-8") as fh:
        source = fh.read()

    try:
        session = frida_mod.attach(PROCESS_NAME)
    except frida_mod.ProcessNotFoundError:
        out(f"{PROCESS_NAME} is not running; nothing attached")
        return RunResult(reason=END_NOT_RUNNING, cap_s=cap_s)

    result = RunResult(reason=END_CAP, cap_s=cap_s)
    ended = threading.Event()
    lock = threading.Lock()

    def finish(reason: str, error: str | None = None) -> None:
        if not ended.is_set():
            result.reason = reason
            if error is not None:
                result.error = error
            ended.set()

    log = None

    def handle(message: dict, data: Any) -> None:
        # Frida calls this on its own thread; the lock keeps a late message
        # from writing while the log closes.
        with lock:
            if log is None or log.closed:
                return
            log.write(json.dumps(message) + "\n")
            log.flush()
        kind = message.get("type")
        if kind == "send":
            payload = message.get("payload")
            result.payloads.append(payload)
            reason = on_message(payload)
            if reason is not None:
                error = payload.get("error") if isinstance(payload, dict) else None
                finish(reason, error)
        elif kind == "error":
            finish(END_SCRIPT_ERROR, message.get("description"))

    def detached(reason: Any, *_rest: Any) -> None:
        finish(END_DETACHED, f"session detached: {reason}")

    try:
        log = open(out_path, "w", encoding="utf-8")
        session.on("detached", detached)
        script = session.create_script(source)
        script.on("message", handle)
        script.load()
        script.post({"type": "start", "cap_s": cap_s})
        if not wait(ended, cap_s):
            out(f"cap of {cap_s:g} s reached before the agent finished; "
                f"{len(result.payloads)} messages seen")
    finally:
        session.detach()
        with lock:
            if log is not None:
                log.close()
    return result


def summarize_clock(result: RunResult) -> list[str]:
    """The clock run's summary lines: how many samples, the first and last
    `songTime`, whether `isSongOver` went true, and the error if one came.
    A sample is a payload carrying `songTime`; clock.ts owns its shape."""
    samples = [p for p in result.payloads
               if isinstance(p, dict) and "songTime" in p]
    lines = [f"ended: {result.reason}", f"samples: {len(samples)}"]
    if samples:
        lines.append(f"songTime: first {samples[0]['songTime']}, "
                     f"last {samples[-1]['songTime']}")
    else:
        lines.append("songTime: no samples")
    went_over = any(s.get("isSongOver") is True for s in samples)
    lines.append(f"isSongOver went true: {'yes' if went_over else 'no'}")
    if result.error is not None:
        lines.append(f"error: {result.error}")
    return lines


def main(argv: list[str] | None = None, *, frida_mod: Any = None,
         wait: Callable[[threading.Event, float], bool] = _default_wait,
         out: Callable[[str], None] = print) -> int:
    ap = argparse.ArgumentParser(
        description="Attach to the running game, run the clock agent, and "
                    "write its samples as JSON lines.")
    ap.add_argument("--agent", required=True, help="the compiled agent .js")
    ap.add_argument("--cap", required=True, type=float,
                    help="stop after this many seconds")
    ap.add_argument("--out", required=True, help="the JSON-lines output file")
    args = ap.parse_args(argv)
    if args.cap <= 0:
        ap.error("--cap must be more than 0 seconds")

    result = run_agent(args.agent, args.cap, ends_run, args.out,
                       frida_mod=frida_mod, wait=wait, out=out)
    if result.reason == END_NOT_RUNNING:
        return 1
    for line in summarize_clock(result):
        out(line)
    out(f"messages written to {args.out}")
    return 1 if result.error is not None else 0


if __name__ == "__main__":
    sys.exit(main())
