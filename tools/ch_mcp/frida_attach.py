"""Attach Frida to the running game, run one compiled agent, and keep every
message it sends.

`run_agent` is the shared machinery: it attaches, loads the agent, tells it to
start with the cap, writes each message as one JSON line, and detaches in a
`finally` block, so a normal end, the cap, a Python error and Ctrl-C all
detach. Any agent can use it; the command line below runs the clock agent and
prints the clock summary.

`LiveAgent` is the long-lived form for a caller that asks questions between
other work: it stays attached with one request/response agent loaded
(agent/state.ts) and answers each `read` until it is detached.

The safety rules it follows are in docs/superpowers/plans/2026-10-28-ch-mcp-feasibility-tests.md,
"Safety rules for every live step", and docs/agents/clone-hero-mcp.md,
"Safety rules". The agents' by-name and allowed-calls checks live in
tests/test_frida_attach.py.

Run from the repo root, with the game already playing a song. Do not add `-I`:
the frida module lives in the user's site-packages, which `-I` skips.

    py tools/ch_mcp/frida_attach.py --agent tools/ch_mcp/agent/clock.js \\
        --cap 93 --out <scratch>\\clock.jsonl
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import threading
import time
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


# A single long Event.wait cannot be interrupted by Ctrl-C on Windows, so the
# default wait comes back this often. It only lets Ctrl-C through; it changes
# no cap.
_INTERRUPT_SLICE_S = 0.5


def _default_wait(event: threading.Event, timeout_s: float) -> bool:
    """Wait for `event` for at most `timeout_s`, in short slices so Ctrl-C
    reaches the `finally` block in run_agent. True when the event was set."""
    deadline = time.monotonic() + timeout_s
    while True:
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            return event.is_set()
        if event.wait(min(remaining, _INTERRUPT_SLICE_S)):
            return True


def _detached_text(reason: Any) -> str:
    """The line for a session Frida dropped; run_agent and LiveAgent both
    report it."""
    return f"session detached: {reason}"


def _import_frida() -> Any:
    """The frida module, or SystemExit with the advice that fixes a failed
    import."""
    try:
        import frida  # noqa: PLC0415
    except ModuleNotFoundError as e:
        raise SystemExit(f"{e}. The frida module lives in the user's "
                         "site-packages: run this without py -I.") from e
    return frida


def run_agent(agent_path: str, cap_s: float,
              on_message: Callable[[Any], str | None], out_path: str, *,
              start: dict | None = None,
              frida_mod: Any = None,
              wait: Callable[[threading.Event, float], bool] | None = None,
              out: Callable[[str], None] = print) -> RunResult:
    """Run the compiled agent at `agent_path` inside the game for at most
    `cap_s` seconds and write every message to `out_path`, one JSON line each.

    `on_message` sees each payload the agent sends; when it returns an end
    reason, the run ends. `start` holds an agent's own settings: they ride in
    the start message beside its `type` and `cap_s`, which they cannot
    replace. `frida_mod` and `wait` are injectable so the tests run without
    Frida and without sleeping; `wait` defaults to `_default_wait`."""
    if wait is None:
        wait = _default_wait
    if frida_mod is None:
        frida_mod = _import_frida()
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
        finish(END_DETACHED, _detached_text(reason))

    try:
        log = open(out_path, "w", encoding="utf-8")
        session.on("detached", detached)
        script = session.create_script(source)
        script.on("message", handle)
        script.load()
        script.post({**(start or {}), "type": "start", "cap_s": cap_s})
        if not wait(ended, cap_s):
            # End the run here, before the detach below: Frida answers that
            # detach through `detached`, which must not replace the cap end.
            finish(END_CAP)
            out(f"cap of {cap_s:g} s reached before the agent finished; "
                f"{len(result.payloads)} messages seen")
    finally:
        # A session the game already dropped may refuse a second detach. Say
        # so and still close the log; an error already in flight stays the
        # one that surfaces.
        try:
            session.detach()
        except Exception as e:  # noqa: BLE001
            out(f"detach reported: {e}")
        finally:
            with lock:
                if log is not None:
                    log.close()
    return result


class LiveAgentError(Exception):
    """A LiveAgent attach or read that failed. `reason` is one of the END_*
    reasons; the message is the line to show."""

    def __init__(self, reason: str, message: str) -> None:
        super().__init__(message)
        self.reason = reason


class AttachFailed(LiveAgentError):
    """LiveAgent.start found no game, or got no first reply. Nothing stays
    attached."""


class ReadFailed(LiveAgentError):
    """LiveAgent.read got no reply: the cap passed, the session dropped, or
    the agent reported an error."""


@dataclass
class _Pending:
    """The one read waiting for its reply."""
    id: int
    event: threading.Event
    reply: Any = None
    failure: LiveAgentError | None = None


# The reason `gone_reason` holds after the caller's own detach.
_DETACHED_BY_CALLER = "detached by the caller"


class LiveAgent:
    """One attach that stays open, with a request/response agent loaded.

    `start(cap_s)` attaches to the game, loads the agent and returns the
    first read; `read(cap_s)` asks again; `detach()` ends the session. Each
    read posts `{"type": "read", "id": n}` and takes only the reply carrying
    the same id, so a reply that comes after its read gave up is logged and
    dropped, never handed to a later read. Every message the agent sends is
    written to `log_path`, one JSON line each, as run_agent does.

    `alive` is False once the session has dropped or been detached, and
    `gone_reason` then says why. One LiveAgent attaches once; attach again
    with a new one. `frida_mod` and `wait` are injectable so the tests run
    without Frida and without sleeping."""

    def __init__(self, agent_path: str, *, log_path: str,
                 frida_mod: Any = None,
                 wait: Callable[[threading.Event, float], bool] | None = None,
                 out: Callable[[str], None] = print) -> None:
        self.agent_path = agent_path
        self.log_path = log_path
        self._frida_mod = frida_mod
        self._wait = wait if wait is not None else _default_wait
        self._out = out
        # Frida calls the handlers on its own thread; this lock guards the
        # log and every field below.
        self._lock = threading.Lock()
        self._session: Any = None
        self._script: Any = None
        self._log: Any = None
        self._next_id = 0
        self._pending: _Pending | None = None
        self._detach_called = False
        self.alive = False
        self.gone_reason: str | None = None

    def start(self, cap_s: float) -> dict:
        """Attach, load the agent and return its answer to the first read,
        waiting at most `cap_s` seconds for it. Raises AttachFailed with the
        reason, and leaves nothing attached, on any failure."""
        if self._session is not None:
            raise RuntimeError("this LiveAgent has already attached once")
        frida_mod = self._frida_mod
        if frida_mod is None:
            frida_mod = _import_frida()
        with open(self.agent_path, encoding="utf-8") as fh:
            source = fh.read()
        try:
            session = frida_mod.attach(PROCESS_NAME)
        except frida_mod.ProcessNotFoundError:
            raise AttachFailed(
                END_NOT_RUNNING,
                f"{PROCESS_NAME} is not running; nothing attached") from None
        self._session = session
        started = False
        try:
            self._log = open(self.log_path, "w", encoding="utf-8")
            session.on("detached", self._on_detached)
            try:
                script = session.create_script(source)
                script.on("message", self._on_message)
                script.load()
            except Exception as e:  # noqa: BLE001
                raise AttachFailed(
                    END_SCRIPT_ERROR,
                    f"{END_SCRIPT_ERROR}: loading the agent failed: {e}") from e
            with self._lock:
                self._script = script
                if self.gone_reason is None:
                    self.alive = True
            try:
                first = self._request(cap_s, "the first read")
            except ReadFailed as e:
                raise AttachFailed(e.reason, f"{e}; detached") from e
            started = True
            return first
        finally:
            if not started:
                self.detach()

    def read(self, cap_s: float) -> dict:
        """Ask the agent for a fresh read and return its reply, waiting at
        most `cap_s` seconds. A `found: false` reply is data, not a failure.
        Raises ReadFailed naming the cap, the detach reason or the agent's
        error."""
        return self._request(cap_s, "the read")

    def detach(self) -> str:
        """Detach once and return what happened. A second call detaches
        nothing and says the session was already detached."""
        with self._lock:
            if self._detach_called:
                return f"already detached ({self.gone_reason})"
            self._detach_called = True
            dropped = self.gone_reason
            if dropped is None:
                self.gone_reason = _DETACHED_BY_CALLER
            self.alive = False
            session = self._session
        if session is None:
            return "nothing attached"
        line = "detached" if dropped is None else f"already gone: {dropped}"
        try:
            # Outside the lock: Frida answers a detach through _on_detached,
            # which takes the lock itself.
            session.detach()
        except Exception as e:  # noqa: BLE001
            # A session the game already dropped may refuse a second detach.
            self._out(f"detach reported: {e}")
        finally:
            with self._lock:
                if self._log is not None:
                    self._log.close()
        return line

    def _request(self, cap_s: float, what: str) -> dict:
        with self._lock:
            if not self.alive:
                reason = END_DETACHED
                why = self.gone_reason or "not attached"
                raise ReadFailed(reason, f"no session for {what}: {why}")
            self._next_id += 1
            pending = _Pending(id=self._next_id, event=threading.Event())
            self._pending = pending
            script = self._script
        try:
            # Outside the lock: the reply may arrive on Frida's thread
            # before post returns.
            try:
                script.post({"type": "read", "id": pending.id})
            except Exception as e:  # noqa: BLE001
                # The game can drop the session between the alive check and
                # the post; Frida then refuses the post.
                raise ReadFailed(END_DETACHED,
                                 f"posting {what} failed: {e}") from e
            self._wait(pending.event, cap_s)
        finally:
            with self._lock:
                if self._pending is pending:
                    self._pending = None
        if pending.failure is not None:
            raise pending.failure
        if pending.reply is None:
            raise ReadFailed(END_CAP, f"no reply to {what} within the cap "
                                      f"of {cap_s:g} s")
        reply = {k: v for k, v in pending.reply.items() if k != "id"}
        if "error" in reply:
            raise ReadFailed(END_AGENT_ERROR,
                             f"the agent sent an error: {reply['error']}")
        return reply

    def _on_message(self, message: dict, data: Any) -> None:
        with self._lock:
            if self._log is None or self._log.closed:
                return
            self._log.write(json.dumps(message) + "\n")
            self._log.flush()
            pending = self._pending
            if pending is None or pending.event.is_set():
                return
            kind = message.get("type")
            if kind == "send":
                payload = message.get("payload")
                if isinstance(payload, dict) and payload.get("id") == pending.id:
                    pending.reply = payload
                    pending.event.set()
            elif kind == "error":
                pending.failure = ReadFailed(
                    END_SCRIPT_ERROR,
                    f"{END_SCRIPT_ERROR}: {message.get('description')}")
                pending.event.set()

    def _on_detached(self, reason: Any, *_rest: Any) -> None:
        with self._lock:
            if self.gone_reason is None:
                self.gone_reason = _detached_text(reason)
            self.alive = False
            pending = self._pending
            if pending is not None and not pending.event.is_set():
                pending.failure = ReadFailed(END_DETACHED, self.gone_reason)
                pending.event.set()


def summarize_clock(result: RunResult) -> list[str]:
    """The clock run's summary lines: how many samples, the first and last
    `songTime`, how many times `songTime` went backwards (a song restart that
    keeps the same GameManager shows up as one), whether `isSongOver` went
    true, and the error if one came. A sample is a payload carrying
    `songTime`; clock.ts owns its shape."""
    samples = [p for p in result.payloads
               if isinstance(p, dict) and "songTime" in p]
    lines = [f"ended: {result.reason}", f"samples: {len(samples)}"]
    if samples:
        lines.append(f"songTime: first {samples[0]['songTime']}, "
                     f"last {samples[-1]['songTime']}")
    else:
        lines.append("songTime: no samples")
    backwards = sum(1 for a, b in zip(samples, samples[1:])
                    if b["songTime"] < a["songTime"])
    lines.append(f"songTime went backwards: {backwards} times")
    went_over = any(s.get("isSongOver") is True for s in samples)
    lines.append(f"isSongOver went true: {'yes' if went_over else 'no'}")
    if result.error is not None:
        lines.append(f"error: {result.error}")
    return lines


def main(argv: list[str] | None = None, *, frida_mod: Any = None,
         wait: Callable[[threading.Event, float], bool] | None = None,
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
