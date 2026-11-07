"""The small front process Claude Code talks to; the real server is its child.

The front starts the worker command given after `--`, copies the worker's tool
list and instructions, and forwards every tool call to it. So a worker can be
replaced without Claude Code's connection dropping. The plan is
docs/superpowers/plans/2026-11-06-ch-server-reload.md; its design points 1, 3,
5 and 6 say why the front is shaped this way.

Run it as:  py tools/ch_mcp/front.py -- py tools/ch_mcp/server.py

The front holds no game state and no tool text, and it never cuts a reply: the
worker owns all three. It speaks to the worker with its own JSON-RPC lines over
a plain Popen, not the mcp package's stdio client (the plan's question 1 says
why). Nothing here prints to stdout, because stdout is the MCP protocol channel.
"""

from __future__ import annotations

import asyncio
import itertools
import json
import subprocess
import sys
import threading
import time
from typing import Any, NamedTuple

from mcp.server.lowlevel import NotificationOptions, Server
from mcp.server.stdio import stdio_server
from mcp.types import CallToolResult, ListToolsResult, TextContent, Tool
from mcp_types.version import LATEST_HANDSHAKE_VERSION

# How long a new worker gets to answer initialize and tools/list (the plan's question 3).
WORKER_START_CAP_S = 30
# How long a reload's stop waits for the worker to exit (the plan's question 5).
WORKER_EXIT_CAP_S = 10
# How long the front's own exit waits for the worker to exit (the plan's question 5).
FRONT_EXIT_CAP_S = 90
# How often a wait looks at whether the worker is still alive (the plan's question 4).
FRONT_POLL_S = 0.5

# The sentence the front adds to the worker's instructions (design point 1).
FRONT_INSTRUCTIONS = (
    "ch_reload swaps in a fresh worker so merged code goes live, and "
    "ch_call(tool, args) reaches a worker tool this session does not list yet."
)


def _log(line: str) -> None:
    """One line for Claude Code's MCP log."""
    print(f"ch front: {line}", file=sys.stderr, flush=True)


class WorkerStartFailed(Exception):
    """The worker did not start; the text names the command and what ended it."""


class WorkerExited(Exception):
    """The worker process exited before it replied."""

    def __init__(self, proc: subprocess.Popen) -> None:
        super().__init__(f"exit code {proc.returncode}")
        self.proc = proc
        self.code = proc.returncode


class WorkerStop(NamedTuple):
    """How one worker ended."""

    pid: int
    seconds: float
    code: int | None
    cap_ended: bool


class _Reply:
    """One request waiting for its reply."""

    def __init__(self) -> None:
        self.arrived = threading.Event()
        self.message: dict[str, Any] = {}


class WorkerLink:
    """One worker process at a time, and the JSON-RPC lines to it.

    start and stop swap the process, under swap_lock; request asks whichever
    worker is current. The instructions and tools are the current worker's.
    """

    def __init__(self, command: list[str]) -> None:
        self.command = command
        self.instructions: str | None = None
        self.tools: list[Tool] = []
        self.swap_lock = threading.RLock()
        self._proc: subprocess.Popen | None = None
        self._pending: dict[int, _Reply] = {}
        self._ids = itertools.count(1)
        self._write_lock = threading.Lock()
        self._closed = False

    @property
    def shown_command(self) -> str:
        return subprocess.list2cmdline(self.command)

    @property
    def pid(self) -> int | None:
        return None if self._proc is None else self._proc.pid

    def start(self) -> None:
        """Start a worker and read its instructions and tools, or raise WorkerStartFailed."""
        with self.swap_lock:
            if self._closed:
                raise WorkerStartFailed("the front is exiting, so it starts no worker")
            began = time.monotonic()
            try:
                proc = subprocess.Popen(
                    self.command,
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE,
                    creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
                )
            except OSError as error:
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} did not run: {error}"
                ) from error
            pending: dict[int, _Reply] = {}
            threading.Thread(target=self._read_replies, args=(proc, pending), daemon=True).start()
            try:
                instructions, tools = self._handshake(proc, pending, began + WORKER_START_CAP_S)
            except WorkerExited as gone:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} exited with code {gone.code} "
                    "before it answered; its stderr is in the MCP log"
                ) from gone
            except TimeoutError as capped:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} gave no reply within "
                    f"WORKER_START_CAP_S ({WORKER_START_CAP_S} s), so the front terminated it; "
                    "its stderr is in the MCP log"
                ) from capped
            except ValueError as refused:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} {refused}; "
                    "its stderr is in the MCP log"
                ) from refused
            self._proc, self._pending = proc, pending
            self.instructions, self.tools = instructions, tools
            _log(f"worker pid {proc.pid} started in {time.monotonic() - began:.1f} s")

    def _handshake(
        self, proc: subprocess.Popen, pending: dict[int, _Reply], deadline: float
    ) -> tuple[str | None, list[Tool]]:
        """The worker's instructions and tools, read over its first three messages."""
        hello = self._ask(proc, pending, "initialize", {
            "protocolVersion": LATEST_HANDSHAKE_VERSION,
            "capabilities": {},
            "clientInfo": {"name": "ch-front", "version": "1"},
        }, deadline)
        if "result" not in hello:
            raise ValueError(f"refused initialize: {hello.get('error')}")
        self._send(proc, {"jsonrpc": "2.0", "method": "notifications/initialized"})
        listed = self._ask(proc, pending, "tools/list", {}, deadline)
        if "result" not in listed:
            raise ValueError(f"refused tools/list: {listed.get('error')}")
        if listed["result"].get("nextCursor"):
            raise ValueError("listed its tools in pages, which the front does not read")
        tools = [Tool.model_validate(tool) for tool in listed["result"]["tools"]]
        return hello["result"].get("instructions"), tools

    def stop(self, cap_s: float) -> WorkerStop | None:
        """Close the worker's stdin and wait up to cap_s; None when no worker runs."""
        with self.swap_lock:
            proc, self._proc = self._proc, None
            return None if proc is None else self._end(proc, cap_s)

    def _end(self, proc: subprocess.Popen, cap_s: float) -> WorkerStop:
        """End one process: its stdin closes, then cap_s, then terminate."""
        began = time.monotonic()
        # On a thread, because a close waits behind a write a hung worker never reads.
        threading.Thread(target=_close_pipe, args=(proc.stdin,), daemon=True).start()
        cap_ended = False
        try:
            proc.wait(cap_s)
        except subprocess.TimeoutExpired:
            cap_ended = True
            proc.terminate()
            try:
                proc.wait(WORKER_EXIT_CAP_S)
            except subprocess.TimeoutExpired:
                _log(f"worker pid {proc.pid} still runs {WORKER_EXIT_CAP_S} s after terminate")
        return WorkerStop(proc.pid, time.monotonic() - began, proc.returncode, cap_ended)

    def close(self) -> None:
        """The front's own exit: no more workers, and the current one gets FRONT_EXIT_CAP_S."""
        with self.swap_lock:
            self._closed = True
            stopped = self.stop(FRONT_EXIT_CAP_S)
        if stopped is None:
            return
        if stopped.cap_ended:
            _log(
                f"FRONT_EXIT_CAP_S ({FRONT_EXIT_CAP_S} s) ended the wait for worker pid "
                f"{stopped.pid}; the front terminated it (exit code {stopped.code})"
            )
        else:
            _log(f"worker exited in {stopped.seconds:.1f} s")

    def replace(self, proc: subprocess.Popen) -> str:
        """Start a new worker after `proc` exited, once however many calls saw it.
        Returns the words for the failed call's reply."""
        with self.swap_lock:
            if self._closed:
                return "the front is exiting, so it started no new worker"
            if self._proc is proc:
                self._end(proc, 0)
                self._proc = None
                try:
                    self.start()
                except WorkerStartFailed as failure:
                    _log(str(failure))
                    return f"the front could not start a new worker: {failure}"
            if self._proc is None:
                return "no worker is running; the next call tries to start one"
            return f"the front has started a new worker (pid {self._proc.pid})"

    def request(self, method: str, params: dict[str, Any]) -> dict[str, Any]:
        """Ask the current worker, starting one if none runs. The wait ends at the
        reply or the worker's exit (WorkerExited), with no fixed cap (question 4)."""
        with self.swap_lock:
            if self._proc is None:
                self.start()
            proc, pending = self._proc, self._pending
        return self._ask(proc, pending, method, params)

    def _ask(
        self,
        proc: subprocess.Popen,
        pending: dict[int, _Reply],
        method: str,
        params: dict[str, Any],
        deadline: float | None = None,
    ) -> dict[str, Any]:
        """Send one request and wait; raises TimeoutError only when given a deadline."""
        number = next(self._ids)
        reply = pending[number] = _Reply()
        self._send(proc, {"jsonrpc": "2.0", "id": number, "method": method, "params": params})
        try:
            while True:
                wait_s = FRONT_POLL_S
                if deadline is not None:
                    wait_s = max(0.0, min(wait_s, deadline - time.monotonic()))
                if reply.arrived.wait(wait_s):
                    return reply.message
                if proc.poll() is not None:
                    # A reply written just before the exit may still be on its way.
                    if reply.arrived.wait(FRONT_POLL_S):
                        return reply.message
                    raise WorkerExited(proc)
                if deadline is not None and time.monotonic() >= deadline:
                    raise TimeoutError(method)
        finally:
            del pending[number]

    def _send(self, proc: subprocess.Popen, message: dict[str, Any]) -> None:
        """One JSON object on one line. A broken pipe is not raised: the caller's
        wait ends at the worker's exit, which is the one place that is reported."""
        line = json.dumps(message).encode("utf-8") + b"\n"
        try:
            with self._write_lock:
                proc.stdin.write(line)
                proc.stdin.flush()
        except (OSError, ValueError):
            pass

    def _read_replies(self, proc: subprocess.Popen, pending: dict[int, _Reply]) -> None:
        """The one reader of a worker's stdout; it hands each reply to its request.
        The worker's notifications are dropped: no tool it serves sends one the
        client needs."""
        with proc.stdout:
            for line in proc.stdout:
                try:
                    message = json.loads(line)
                    number = message.get("id")
                except (ValueError, AttributeError):
                    _log(f"worker pid {proc.pid} wrote a line that is not JSON-RPC: {line[:200]!r}")
                    continue
                reply = pending.get(number)
                if reply is not None:
                    reply.message = message
                    reply.arrived.set()
                elif "method" in message and number is not None:
                    _log(f"worker pid {proc.pid} asked {message['method']}; the front answers no requests")


def _close_pipe(pipe: Any) -> None:
    try:
        pipe.close()
    except (OSError, ValueError):
        pass


def _failed(text: str) -> CallToolResult:
    return CallToolResult(content=[TextContent(type="text", text=text)], is_error=True)


def forward(link: WorkerLink, name: str, arguments: dict[str, Any] | None) -> CallToolResult:
    """One tool call through the worker, its result returned whole."""
    params: dict[str, Any] = {"name": name}
    if arguments is not None:
        params["arguments"] = arguments
    try:
        message = link.request("tools/call", params)
    except WorkerStartFailed as failure:
        _log(str(failure))
        return _failed(f"{name}: {failure}")
    except WorkerExited as gone:
        text = f"{name}: the worker exited with code {gone.code} before it replied; {link.replace(gone.proc)}"
        _log(text)
        return _failed(text)
    if "result" not in message:
        return _failed(f"{name}: the worker refused the call: {message.get('error')}")
    return CallToolResult.model_validate(message["result"])


def build_front(link: WorkerLink, start_failure: str | None) -> Server:
    """The server Claude Code talks to. After a failed start it still serves, so
    the failure reaches Claude Code as text and each call tries a new worker."""

    async def on_list_tools(ctx: Any, params: Any) -> ListToolsResult:
        return ListToolsResult(tools=link.tools)

    async def on_call_tool(ctx: Any, params: Any) -> CallToolResult:
        # On a thread, so a long call holds up neither other calls nor the exit.
        return await asyncio.to_thread(forward, link, params.name, params.arguments)

    return Server(
        "ch",
        instructions=f"{link.instructions or start_failure or ''} {FRONT_INSTRUCTIONS}".strip(),
        on_list_tools=on_list_tools,
        on_call_tool=on_call_tool,
    )


async def serve(link: WorkerLink, server: Server) -> None:
    """Serve until the front's stdin ends, then see the worker out."""
    try:
        async with stdio_server() as (read_stream, write_stream):
            options = server.create_initialization_options(NotificationOptions(tools_changed=True))
            await server.run(read_stream, write_stream, options)
    finally:
        link.close()


def main(argv: list[str] | None = None) -> int:
    argv = sys.argv[1:] if argv is None else argv
    command = argv[argv.index("--") + 1:] if "--" in argv else []
    if not command:
        _log("usage: front.py -- <worker command>")
        return 2
    link = WorkerLink(command)
    start_failure = None
    try:
        link.start()
    except WorkerStartFailed as failure:
        start_failure = str(failure)
        _log(start_failure)
    asyncio.run(serve(link, build_front(link, start_failure)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
