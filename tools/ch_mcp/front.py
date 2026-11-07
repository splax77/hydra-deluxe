"""The small front process Claude Code talks to; the real server is its child.

The front starts the worker command given after `--`, copies the worker's tool
list and instructions, and forwards every tool call to it. So a worker can be
replaced without Claude Code's connection dropping. The plan is
docs/superpowers/plans/2026-11-06-ch-server-reload.md; its design points 1 to 6
and 9 say why the front is shaped this way.

Run it as:  py tools/ch_mcp/front.py -- py tools/ch_mcp/server.py

The front adds two tools of its own. ch_reload swaps the worker for a fresh one
when nothing is open, and ch_call reaches a worker tool by name.

The front holds no game state and no worker tool text, and it never cuts a
reply: the worker owns all three. It speaks to the worker with its own JSON-RPC lines over
a plain Popen, not the mcp package's stdio client (the plan's question 1 says
why). Nothing here prints to stdout, because stdout is the MCP protocol channel.
"""

from __future__ import annotations

import asyncio
import contextlib
import itertools
import json
import os
import subprocess
import sys
import threading
import time
from typing import Any, Iterator, NamedTuple

from mcp.server.lowlevel import NotificationOptions, Server
from mcp.server.stdio import stdio_server
from mcp.types import CallToolResult, ListToolsResult, NotificationParams, TextContent, Tool
from mcp_types.version import LATEST_HANDSHAKE_VERSION

# How long a new worker gets to answer initialize and tools/list, and then
# ch_open_items (the plan's question 3).
WORKER_START_CAP_S = 30
# How long a reload waits for the old worker's ch_open_items answer, and then for
# its exit (the plan's question 5).
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

RELOAD_TOOL = "ch_reload"
CALL_TOOL = "ch_call"
# The worker's tool that lists what it holds open, and its words for "nothing".
# Any other reply counts as something open, so a changed word refuses a reload.
OPEN_ITEMS_TOOL = "ch_open_items"
NOTHING_OPEN = "nothing open"

# The two tools the front serves itself; every other tool is the worker's.
FRONT_TOOLS = [
    Tool(
        name=RELOAD_TOOL,
        description=(
            "Stop the worker and start a fresh one, so Python code merged since it started "
            "goes live with no reconnect. Refuses while anything is open or another call is "
            "running. The reply gives both process ids, each step's seconds, the tools added "
            "and removed, and whether Claude Code was told the list changed."
        ),
        input_schema={"type": "object", "properties": {}},
    ),
    Tool(
        name=CALL_TOOL,
        description=(
            "Call a worker tool by name, for a tool this session does not list yet or whose "
            "schema changed at a ch_reload. args is required; a call with no arguments "
            "passes {}. The reply is the tool's own."
        ),
        input_schema={
            "type": "object",
            "properties": {
                "tool": {"type": "string", "description": "The worker tool's name."},
                "args": {"type": "object", "description": "The tool's arguments."},
            },
            "required": ["tool", "args"],
        },
    ),
]

# Passed at the handshake and asked again per connection, so the mcp package
# alone decides whether listChanged is declared (design point 6).
LIST_CHANGED_OPTIONS = NotificationOptions(tools_changed=True)


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


def _ended_words(gone: WorkerExited) -> str:
    """What ended a wait, for a reply's text: an exit code, or a closed output."""
    if gone.code is None:
        return "closed its output while it still ran"
    return f"exited with code {gone.code}"


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


class _Pending(dict):
    """A worker's waiting requests by id. `closed` is set once its reader has
    stopped, so a request made after that fails instead of waiting for a reply
    nobody reads."""

    closed = False


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
            pending = _Pending()
            threading.Thread(target=self._read_replies, args=(proc, pending), daemon=True).start()
            try:
                instructions, tools = self._handshake(proc, pending, began + WORKER_START_CAP_S)
            except WorkerExited as gone:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} {_ended_words(gone)} "
                    "before it answered; its stderr is in the MCP log"
                ) from gone
            except TimeoutError as capped:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} gave no reply within "
                    f"WORKER_START_CAP_S ({WORKER_START_CAP_S} s), so the front terminated it; "
                    "its stderr is in the MCP log"
                ) from capped
            except (ValueError, KeyError, TypeError, AttributeError) as refused:
                self._end(proc, 0)
                raise WorkerStartFailed(
                    f"the worker command {self.shown_command} sent a malformed handshake "
                    f"({type(refused).__name__}: {refused}); "
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

    def request(
        self, method: str, params: dict[str, Any], cap_s: float | None = None
    ) -> dict[str, Any]:
        """Ask the current worker, starting one if none runs. The wait ends at the
        reply or the worker's exit (WorkerExited). A forwarded call passes no cap
        (question 4); with one, its end raises TimeoutError."""
        with self.swap_lock:
            if self._proc is None:
                self.start()
            proc, pending = self._proc, self._pending
        deadline = None if cap_s is None else time.monotonic() + cap_s
        return self._ask(proc, pending, method, params, deadline)

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
                if pending.closed and not reply.arrived.is_set():
                    # The reader is gone but the worker may be mid-exit: give it a moment
                    # so the reply can name an exit code.
                    try:
                        proc.wait(FRONT_POLL_S)
                    except subprocess.TimeoutExpired:
                        pass
                    raise WorkerExited(proc)
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
        try:
            with proc.stdout:
                for line in proc.stdout:
                    try:
                        message = json.loads(line)
                        number = message.get("id")
                        reply = pending.get(number)
                    except (ValueError, AttributeError, TypeError):
                        _log(f"worker pid {proc.pid} wrote a line that is not JSON-RPC: {line[:200]!r}")
                        continue
                    if reply is not None:
                        reply.message = message
                        reply.arrived.set()
                    elif "method" in message and number is not None:
                        _log(f"worker pid {proc.pid} asked {message['method']}; the front answers no requests")
        except Exception as failure:
            _log(f"the reader of worker pid {proc.pid} stopped: {type(failure).__name__}: {failure}")
        finally:
            # Output ended or the reader failed: every waiting request now sees `closed`.
            pending.closed = True


def _close_pipe(pipe: Any) -> None:
    try:
        pipe.close()
    except (OSError, ValueError):
        pass


class OpenCalls:
    """The tool calls in flight, by name, so a reload can name what stopped it.

    A call enters before it reaches WorkerLink.request. So one that enters after
    a reload's check waits on swap_lock and goes to the new worker, and none can
    be on its way to the worker being stopped.
    """

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._names: dict[int, str] = {}
        self._ids = itertools.count(1)

    @contextlib.contextmanager
    def held(self, name: str) -> Iterator[int]:
        token = next(self._ids)
        with self._lock:
            self._names[token] = name
        try:
            yield token
        finally:
            with self._lock:
                del self._names[token]

    def others(self, token: int) -> list[str]:
        with self._lock:
            return sorted(name for held, name in self._names.items() if held != token)


class OpenItemsUnread(Exception):
    """ch_open_items answered, but not with a list of what is open."""


class Swapped(NamedTuple):
    """A finished reload: its reply so far, and whether the tool lists differ."""

    lines: list[str]
    lists_differ: bool


def _failed(text: str) -> CallToolResult:
    return CallToolResult(content=[TextContent(type="text", text=text)], is_error=True)


def open_items(link: WorkerLink, cap_s: float) -> list[str]:
    """What the worker holds open, one line each; empty when nothing is. Raises
    OpenItemsUnread, TimeoutError or what WorkerLink.request raises."""
    message = link.request("tools/call", {"name": OPEN_ITEMS_TOOL, "arguments": {}}, cap_s)
    if "result" not in message:
        raise OpenItemsUnread(f"the worker refused the call: {message.get('error')}")
    result = message["result"]
    text = "\n".join(block.get("text", "") for block in result.get("content", []))
    lines = [line for line in text.splitlines() if line.strip()]
    if result.get("isError") or not lines:
        raise OpenItemsUnread(text or "its reply held no text")
    return [] if lines == [NOTHING_OPEN] else lines


def new_worker_lines(link: WorkerLink) -> list[str]:
    """What a worker that just started holds open, such as the backup a dead
    worker left, for the reply of the call that replaced it (design point 3).
    A failed ask becomes a line, so the caller's own text is never lost."""
    if link.pid is None:
        return []
    try:
        return open_items(link, WORKER_START_CAP_S)
    except WorkerExited as gone:
        why = f"it exited with code {gone.code}"
    except TimeoutError:
        why = f"it gave no reply within WORKER_START_CAP_S ({WORKER_START_CAP_S} s)"
    except (WorkerStartFailed, OpenItemsUnread) as failure:
        why = str(failure)
    return [f"the front could not read what the new worker holds open: {why}"]


def worker_died(link: WorkerLink, name: str, gone: WorkerExited) -> CallToolResult:
    """The reply for a call whose worker ended: a new worker starts at once."""
    lines = [f"{name}: the worker {_ended_words(gone)} before it replied; {link.replace(gone.proc)}"]
    text = "\n".join(lines + new_worker_lines(link))
    _log(text)
    return _failed(text)


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
        return worker_died(link, name, gone)
    if "result" not in message:
        return _failed(f"{name}: the worker refused the call: {message.get('error')}")
    try:
        return CallToolResult.model_validate(message["result"])
    except ValueError as malformed:
        return _failed(f"{name}: the worker's reply is not a tool result: {malformed}")


def call_shape_refusal(arguments: dict[str, Any]) -> str | None:
    """Why this ch_call never reaches a worker, whatever the worker lists. The
    hook in percall.py reads the same two fields, so nothing is coerced here."""
    tool = arguments.get("tool")
    if not isinstance(tool, str):
        return f"refused: {CALL_TOOL}'s tool must be a string, and this one is {tool!r}"
    if not isinstance(arguments.get("args"), dict):
        return (
            f"refused: {CALL_TOOL}'s args must be an object, and this one is "
            f"{arguments.get('args')!r}; a call with no arguments passes {{}}"
        )
    if tool in (CALL_TOOL, RELOAD_TOOL):
        return f"refused: {tool} does not go through {CALL_TOOL}"
    return None


def route_call(link: WorkerLink, calls: OpenCalls, name: str, arguments: dict[str, Any] | None) -> CallToolResult:
    """One call other than ch_reload: forwarded, or ch_call's inner call forwarded."""
    if name == CALL_TOOL:
        arguments = arguments or {}
        refusal = call_shape_refusal(arguments)
        if refusal:
            return _failed(refusal)
        name, arguments = arguments["tool"], arguments["args"]
        try:
            # A front whose first start failed lists no worker tool until one runs.
            with link.swap_lock:
                if link.pid is None:
                    link.start()
                listed = [tool.name for tool in link.tools]
        except WorkerStartFailed as failure:
            _log(str(failure))
            return _failed(f"{CALL_TOOL}: {failure}")
        if name not in listed:
            return _failed(f"refused: the worker has no tool {name!r}; its tools: {', '.join(listed)}")
    with calls.held(name):
        return forward(link, name, arguments)


def process_label(link: WorkerLink) -> str:
    """What the pid in a reload's reply is. Popen's pid is whatever the command
    started, so behind `py` it is the launcher's, not the Python in the marker."""
    program = os.path.splitext(os.path.basename(link.command[0]))[0].lower()
    return "worker process (py launcher)" if program == "py" else "worker process"


def reload_refusal(link: WorkerLink, calls: OpenCalls, token: int) -> CallToolResult | None:
    """Why this reload must leave the worker alone, or None when it may go on
    (design point 2). Anything but a clear "nothing open" refuses."""
    try:
        items = open_items(link, WORKER_EXIT_CAP_S)
    except WorkerExited as gone:
        return worker_died(link, RELOAD_TOOL, gone)
    except WorkerStartFailed as failure:
        _log(str(failure))
        return _failed(f"{RELOAD_TOOL}: {failure}")
    except TimeoutError:
        running = ", ".join(calls.others(token)) or "none"
        return _failed(
            f"refused: the worker gave no reply to {OPEN_ITEMS_TOOL} within WORKER_EXIT_CAP_S "
            f"({WORKER_EXIT_CAP_S} s), so the front left it running; calls in flight: {running}"
        )
    except OpenItemsUnread as failure:
        return _failed(f"refused: {OPEN_ITEMS_TOOL} failed, so the front left the worker running: {failure}")
    if items:
        return _failed(f"refused: {'; '.join(items)}; call ch_stop first, then {RELOAD_TOOL}")
    running = calls.others(token)
    if running:
        return _failed(f"refused: a call is in flight: {', '.join(running)}; {RELOAD_TOOL} after it replies")
    return None


def stop_line(stopped: WorkerStop) -> str:
    if stopped.cap_ended:
        return (
            f"stop: WORKER_EXIT_CAP_S ({WORKER_EXIT_CAP_S} s) ended the wait for pid {stopped.pid}; "
            f"the front terminated it (exit code {stopped.code}, {stopped.seconds:.1f} s)"
        )
    return f"stop: {stopped.seconds:.1f} s, exit code {stopped.code}"


def names_line(label: str, names: list[str]) -> str:
    return f"{label}: {', '.join(names) or 'none'}"


def reload_worker(link: WorkerLink, calls: OpenCalls) -> CallToolResult | Swapped:
    """Design point 2. The lock holds new calls back until the new worker runs."""
    with calls.held(RELOAD_TOOL) as token, link.swap_lock:
        began = time.monotonic()
        refusal = reload_refusal(link, calls, token)
        if refusal:
            return refusal
        checked = time.monotonic()
        tools_before, instructions_before = link.tools, link.instructions
        # The check above left a worker running, and the lock keeps it so.
        stopped = link.stop(WORKER_EXIT_CAP_S)
        _log(stop_line(stopped))
        stopped_at = time.monotonic()
        try:
            link.start()
        except WorkerStartFailed as failure:
            _log(str(failure))
            return _failed(
                f"{stop_line(stopped)}\n{RELOAD_TOOL}: no new worker started: {failure}; "
                "the next call tries again"
            )
        before = {tool.name: tool for tool in tools_before}
        after = {tool.name: tool for tool in link.tools}
        added = sorted(after.keys() - before.keys())
        removed = sorted(before.keys() - after.keys())
        changed = sorted(name for name in after.keys() & before.keys() if after[name] != before[name])
        lines = [
            f"{process_label(link)}: pid {stopped.pid} stopped, pid {link.pid} started "
            "(the process the front started, not the pid in the open-run marker)",
            f"open check: {checked - began:.1f} s",
            stop_line(stopped),
            f"start: {time.monotonic() - stopped_at:.1f} s",
            f"worker tools: {len(before)} before, {len(after)} after",
            names_line("added", added),
            names_line("removed", removed),
            names_line("changed", changed),
        ]
        if link.instructions != instructions_before:
            lines.append("instructions changed; live at the next reconnect")
    return Swapped(lines + new_worker_lines(link), bool(added or removed or changed))


async def list_changed_lines(server: Server, ctx: Any, lists_differ: bool) -> list[str]:
    """Tell Claude Code the list changed when the connection allows it, and say
    what happened (design point 6)."""
    version = ctx.protocol_version
    declared = server.get_capabilities(LIST_CHANGED_OPTIONS, protocol_version=version).tools.list_changed
    lines = [] if declared else [f"list_changed not declared on protocol {version}; use ch_call for a new tool"]
    if not lists_differ:
        return [*lines, "list_changed: not sent, lists equal"]
    if not declared:
        return lines
    try:
        await ctx.session.send_tool_list_changed()
    except Exception as error:  # the reload is done, and its reply must still arrive
        return [f"list_changed not sent: {error}; use ch_call for a new tool"]
    return ["list_changed sent"]


def build_front(link: WorkerLink, start_failure: str | None) -> Server:
    """The server Claude Code talks to. After a failed start it still serves, so
    the failure reaches Claude Code as text and each call tries a new worker."""
    calls = OpenCalls()
    announced: list[str] = []

    def note_protocol(ctx: Any) -> None:
        """The version line for the MCP log, once. A connection on 2026-07-28
        has no initialize, so its first request prints it."""
        if not announced:
            announced.append(ctx.protocol_version)
            offered = getattr(ctx.session.client_params, "protocol_version", "no version")
            _log(f"Claude Code offered {offered}; this connection runs protocol {ctx.protocol_version}")

    async def on_initialized(ctx: Any, params: Any) -> None:
        note_protocol(ctx)

    async def on_list_tools(ctx: Any, params: Any) -> ListToolsResult:
        note_protocol(ctx)
        return ListToolsResult(tools=[*link.tools, *FRONT_TOOLS])

    async def on_call_tool(ctx: Any, params: Any) -> CallToolResult:
        note_protocol(ctx)
        # On a thread, so a long call holds up neither other calls nor the exit.
        if params.name != RELOAD_TOOL:
            return await asyncio.to_thread(route_call, link, calls, params.name, params.arguments)
        swapped = await asyncio.to_thread(reload_worker, link, calls)
        if isinstance(swapped, CallToolResult):
            return swapped
        lines = [
            *swapped.lines,
            f"protocol version with Claude Code: {ctx.protocol_version}",
            *await list_changed_lines(front, ctx, swapped.lists_differ),
        ]
        return CallToolResult(content=[TextContent(type="text", text="\n".join(lines))])

    front = Server(
        "ch",
        instructions=f"{link.instructions or start_failure or ''} {FRONT_INSTRUCTIONS}".strip(),
        on_list_tools=on_list_tools,
        on_call_tool=on_call_tool,
    )
    front.add_notification_handler("notifications/initialized", NotificationParams, on_initialized)
    return front


async def serve(link: WorkerLink, server: Server) -> None:
    """Serve until the front's stdin ends, then see the worker out."""
    try:
        async with stdio_server() as (read_stream, write_stream):
            options = server.create_initialization_options(LIST_CHANGED_OPTIONS)
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
