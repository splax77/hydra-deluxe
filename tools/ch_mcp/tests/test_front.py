"""Tests for the front process: forwarding, a worker's death, the caps, ch_reload
and ch_call.

Every test runs the real front as a process and speaks MCP's stdio lines to it.
Where the claim can be shown without the game, the worker is the real one,
tools/ch_mcp/server.py (a reload test runs a copy of its code and edits the
copy, the way a merge does). The rest run this same file with --fake-worker,
which serves a real MCPServer over its pipes. A settings file, read at start
and by ch_open_items, makes the next fake differ from the last. Every read has
a cap, and a fake ends itself after FAKE_LIFE_S, so a hung fake cannot hang the
run.
"""

from __future__ import annotations

import json
import os
import queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
FRONT_PY = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "front.py")
# The worker .mcp.json runs behind the front.
SERVER_PY = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "server.py")

sys.path.insert(0, os.path.dirname(FRONT_PY))
import front as front_module  # noqa: E402
sys.path.remove(os.path.dirname(FRONT_PY))

# The longest any fake lives, and the longest a test waits on one reply or exit.
FAKE_LIFE_S = 60
READ_CAP_S = 30

WORKER_INSTRUCTIONS = "Fake worker instructions."
WORKER_STDERR_LINE = "fake worker line on stderr"
LONG_TEXT = "é" + "x" * 11_998 + "é"
SLOW_EXIT_S = 40
FRONT_TOOL_NAMES = [tool.name for tool in front_module.FRONT_TOOLS]
ADOPTION_LINE = "a previous worker (pid 5) left a backup open: C:/backup; ch_stop restores it"
# The per-request keys that open a 2026-07-28 connection, which has no initialize.
MODERN_ENVELOPE = {
    "io.modelcontextprotocol/protocolVersion": "2026-07-28",
    "io.modelcontextprotocol/clientCapabilities": {},
}


def read_settings(path: str | None) -> dict:
    if path is None or not os.path.exists(path):
        return {}
    with open(path, encoding="utf-8") as lines:
        return json.load(lines)


def fake_worker(mode: str, settings_path: str | None = None) -> None:
    """Serve the fake tools over stdio; `mode` picks how the fake misbehaves, and
    the settings file what this one worker lists and holds open."""
    timer = threading.Timer(FAKE_LIFE_S, os._exit, args=(99,))
    timer.daemon = True
    timer.start()
    settings = read_settings(settings_path)
    if mode == "mute":
        time.sleep(FAKE_LIFE_S)
        return
    if mode == "exits_at_start" or settings.get("exit_at_start"):
        os._exit(3)
    if mode == "raw":
        raw_worker()
        return

    import asyncio
    import atexit

    from mcp.server.mcpserver import MCPServer

    if mode == "slow_exit":
        atexit.register(time.sleep, SLOW_EXIT_S)

    server = MCPServer("fake", instructions=settings.get("instructions", WORKER_INSTRUCTIONS))
    echoes: list[str] = []

    @server.tool()
    def echo_args(text: str, numbers: list[int] | None = None, nested: dict | None = None) -> str:
        """Say back the arguments as JSON."""
        echoes.append(text)
        return json.dumps({"text": text, "numbers": numbers, "nested": nested}, sort_keys=True)

    @server.tool()
    def echo_count() -> str:
        """How many calls reached echo_args in this worker."""
        return str(len(echoes))

    @server.tool()
    def long_text() -> str:
        """A reply of exactly the worker's cut length."""
        return LONG_TEXT

    if not settings.get("drop_fails"):
        @server.tool()
        def fails() -> str:
            """A tool whose reply has isError true."""
            raise ValueError("the fake failed on purpose")

    if settings.get("extra_tool"):
        @server.tool()
        def extra_tool() -> str:
            """A tool only a later worker has."""
            return "extra tool answered"

    @server.tool()
    def ch_state() -> str:
        """The fake's state line."""
        return "fake state"

    @server.tool()
    async def ch_open_items() -> str:
        """The lines in the settings file, or the real tool's word for none."""
        now = read_settings(settings_path)
        if now.get("open_items") == "hang":
            await asyncio.sleep(FAKE_LIFE_S)
        if now.get("open_items") == "fails":
            raise ValueError("the fake failed on purpose")
        if now.get("open_items") == "dies":
            os._exit(9)
        return "\n".join(now.get("open", []) + settings.get("held_from_start", [])) or "nothing open"

    @server.tool(description=settings.get("my_pid_text"))
    def my_pid() -> str:
        """Which process answered."""
        return str(os.getpid())

    @server.tool()
    async def slow(seconds: float) -> str:
        """A reply that comes after `seconds`, without blocking other calls."""
        await asyncio.sleep(seconds)
        return f"slept {seconds}"

    @server.tool()
    def die() -> str:
        """Exit mid-call with code 7."""
        os._exit(7)

    @server.tool()
    def die_after_reply() -> str:
        """Reply, then exit with code 8 while no call is open."""
        threading.Timer(0.2, os._exit, args=(8,)).start()
        return "dying soon"

    print(WORKER_STDERR_LINE, file=sys.stderr, flush=True)
    server.run("stdio")


def raw_worker() -> None:
    """A hand-written worker, so a test controls every byte on its stdout. The
    tool `junk_then_reply` writes lines that are not replies and then replies;
    `close_output` closes stdout, never replies and stays alive."""
    out = sys.stdout.buffer

    def write(message: dict) -> None:
        out.write(json.dumps(message).encode("utf-8") + b"\n")
        out.flush()

    for line in sys.stdin.buffer:
        message = json.loads(line)
        method, number = message.get("method"), message.get("id")
        if method == "initialize":
            write({"jsonrpc": "2.0", "id": number, "result": {
                "protocolVersion": "2025-11-25", "capabilities": {"tools": {}},
                "serverInfo": {"name": "raw", "version": "1"}, "instructions": WORKER_INSTRUCTIONS}})
        elif method == "tools/list":
            schema = {"type": "object", "properties": {}}
            write({"jsonrpc": "2.0", "id": number, "result": {"tools": [
                {"name": name, "description": name, "inputSchema": schema}
                for name in ("junk_then_reply", "close_output", "my_pid")]}})
        elif method == "tools/call":
            name = message["params"]["name"]
            if name == "close_output":
                os.close(1)
                time.sleep(FAKE_LIFE_S)
            if name == "junk_then_reply":
                for junk in (b"not json\n", b"[1]\n", b'{"id": [1]}\n', b"\xff\xfe\n"):
                    out.write(junk)
            text = "after junk" if name == "junk_then_reply" else str(os.getpid())
            write({"jsonrpc": "2.0", "id": number, "result": {"content": [{"type": "text", "text": text}]}})


def worker_command(mode: str, python: str = sys.executable, settings: str | None = None) -> list[str]:
    command = [python, os.path.abspath(__file__), "--fake-worker", mode]
    return command if settings is None else [*command, settings]


class Peer:
    """A process spoken to in MCP stdio lines; every read has a cap."""

    def __init__(self, command: list[str], stderr_path: str, env: dict | None = None) -> None:
        self._stderr_file = open(stderr_path, "wb")
        self.stderr_path = stderr_path
        self.proc = subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self._stderr_file, env=env
        )
        self._lines: queue.Queue = queue.Queue()
        self.notifications: list[str] = []
        self._ids = iter(range(1, 1000))
        threading.Thread(target=self._read, daemon=True).start()

    def _read(self) -> None:
        for line in self.proc.stdout:
            self._lines.put(json.loads(line))

    def send(self, method: str, params: dict) -> int:
        number = next(self._ids)
        self._write({"jsonrpc": "2.0", "id": number, "method": method, "params": params})
        return number

    def _write(self, message: dict) -> None:
        self.proc.stdin.write(json.dumps(message).encode("utf-8") + b"\n")
        self.proc.stdin.flush()

    def reply(self, number: int, held: dict | None = None) -> dict:
        """The reply with this id; other replies go in `held`."""
        held = {} if held is None else held
        deadline = time.monotonic() + READ_CAP_S
        while number not in held:
            message = self._lines.get(timeout=max(0.0, deadline - time.monotonic()))
            if "id" in message:
                held[message["id"]] = message
            else:
                self.notifications.append(message["method"])
        return held.pop(number)

    def ask(self, method: str, params: dict) -> dict:
        return self.reply(self.send(method, params))["result"]

    def hello(self, version: str = "2025-11-25") -> dict:
        result = self.ask("initialize", {
            "protocolVersion": version,
            "capabilities": {},
            "clientInfo": {"name": "test", "version": "1"},
        })
        self._write({"jsonrpc": "2.0", "method": "notifications/initialized"})
        return result

    def call(self, tool: str, arguments: dict | None = None) -> dict:
        return self.ask("tools/call", {"name": tool, "arguments": arguments or {}})

    def text(self, tool: str, arguments: dict | None = None) -> str:
        return self.call(tool, arguments)["content"][0]["text"]

    def close_stdin(self) -> None:
        self.proc.stdin.close()

    def stderr(self) -> str:
        self._stderr_file.flush()
        with open(self.stderr_path, encoding="utf-8", errors="replace") as lines:
            return lines.read()

    def cleanup(self) -> None:
        if self.proc.poll() is None:
            self.proc.kill()
            self.proc.wait(READ_CAP_S)
        self.proc.stdin.close()
        self._stderr_file.close()


class FrontCase(unittest.TestCase):
    def setUp(self) -> None:
        folder = tempfile.TemporaryDirectory(ignore_cleanup_errors=True)
        self.addCleanup(folder.cleanup)
        self.folder = folder.name
        self.settings_path = os.path.join(self.folder, "settings.json")
        self._count = 0

    def write_settings(self, **settings: object) -> None:
        """What the next worker to start, and ch_open_items from now on, read."""
        with open(self.settings_path, "w", encoding="utf-8") as lines:
            json.dump(settings, lines)

    def settings_worker(self, mode: str = "plain", python: str = sys.executable) -> list[str]:
        return worker_command(mode, python, self.settings_path)

    def peer(self, command: list[str]) -> Peer:
        """A process whose temp folder is the test's own. The real worker reads
        game_tools.OPEN_RUN_MARKER under tempfile.gettempdir() at start, so a
        marker a live worker left on this machine never reaches a test."""
        self._count += 1
        env = {**os.environ, "TEMP": self.folder, "TMP": self.folder}
        peer = Peer(command, os.path.join(self.folder, f"stderr{self._count}.txt"), env)
        self.addCleanup(peer.cleanup)
        return peer

    def real_worker(self) -> list[str]:
        return [sys.executable, SERVER_PY]

    def copied_worker(self) -> str:
        """A copy of the real worker's code (tools/ch_mcp, tools/ch_probe and
        tools/hydra_ui_mcp.py, the parts server.py imports) for a test to edit
        between a start and a reload; returns the copy's server.py."""
        tools = os.path.join(self.folder, "code", "tools")
        skip = shutil.ignore_patterns("tests", "results", "__pycache__")
        for part in ("ch_mcp", "ch_probe"):
            shutil.copytree(os.path.join(_REPO_ROOT, "tools", part), os.path.join(tools, part), ignore=skip)
        shutil.copy2(os.path.join(_REPO_ROOT, "tools", "hydra_ui_mcp.py"), tools)
        return os.path.join(tools, "ch_mcp", "server.py")

    def edit_worker(self, server_py: str) -> dict[str, list[str]]:
        """The kind of edit a merge makes to server.py: one tool added, one
        tool's decorator dropped and one tool's description reworded. Returns
        the names for each of the reload reply's added, removed and changed lines."""
        with open(server_py, encoding="utf-8") as source:
            text = source.read()
        edits = [
            ("    return server\n",
             '    @server.tool()\n    def ch_added_at_reload() -> str:\n'
             '        """A tool the edit adds."""\n        return "added at reload"\n\n'
             "    return server\n"),
            ("    @server.tool()\n    def ch_wait_stats(", "    def ch_wait_stats("),
            ('"""Detach the state agent from the game."""', '"""Detach the agent (reworded by the edit)."""'),
        ]
        for old, new in edits:
            self.assertEqual(text.count(old), 1, f"server.py no longer holds the edit's anchor {old!r}")
            text = text.replace(old, new)
        with open(server_py, "w", encoding="utf-8") as source:
            source.write(text)
        return {"added": ["ch_added_at_reload"], "removed": ["ch_wait_stats"], "changed": ["ch_detach"]}

    def front(self, worker: list[str], **caps: float) -> Peer:
        """The real front, with any cap in `caps` shortened for the test."""
        lines = [f"import sys; sys.path.insert(0, {os.path.dirname(FRONT_PY)!r}); import front"]
        lines += [f"front.{name} = {value!r}" for name, value in caps.items()]
        lines.append("sys.exit(front.main(sys.argv[1:]))")
        return self.peer([sys.executable, "-c", "\n".join(lines), "--", *worker])

    def wait_exit(self, peer: Peer) -> float:
        """Seconds from now to the peer's exit; a miss fails the test at its cap."""
        began = time.monotonic()
        peer.proc.wait(READ_CAP_S)
        return time.monotonic() - began


class ForwardingTest(FrontCase):
    def test_tool_list_and_instructions_come_from_the_worker(self) -> None:
        direct = self.peer(self.real_worker())
        worker_hello = direct.hello()
        worker_tools = direct.ask("tools/list", {})["tools"]

        front = self.front(self.real_worker())
        hello = front.hello()
        tools = front.ask("tools/list", {})["tools"]

        # The front lists the worker's tools whole, then its own, and joins its
        # sentence to the worker's instructions.
        self.assertEqual(tools[:len(worker_tools)], worker_tools)
        self.assertEqual(
            tools[len(worker_tools):],
            [tool.model_dump(mode="json", by_alias=True, exclude_none=True) for tool in front_module.FRONT_TOOLS],
        )
        self.assertEqual(hello["instructions"], f"{worker_hello['instructions']} {front_module.FRONT_INSTRUCTIONS}")
        self.assertTrue(hello["capabilities"]["tools"]["listChanged"])

    def test_a_call_comes_back_as_the_worker_sent_it(self) -> None:
        direct = self.peer(self.real_worker())
        direct.hello()
        front = self.front(self.real_worker())
        front.hello()
        missing = os.path.join(self.folder, "no_such_stats.json")

        # A plain reply, a reply naming a failed read, and a call the worker
        # refuses for a missing argument (isError true).
        for tool, args in (("ch_open_items", {}), ("ch_status", {}), ("ch_stats", {"path": missing}),
                           ("ch_install_chart", {})):
            with self.subTest(tool):
                self.assertEqual(front.call(tool, args), direct.call(tool, args))

    def test_arguments_and_long_text_arrive_unchanged(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        arguments = {"text": "é\n\"quoted\"", "numbers": [1, 2], "nested": {"a": [None, True]}}

        echoed = front.call("echo_args", arguments)["content"][0]["text"]
        self.assertEqual(json.loads(echoed), arguments)

        self.assertEqual(front.call("long_text")["content"][0]["text"], LONG_TEXT)
        self.assertEqual(len(LONG_TEXT), 12_000)

    def test_is_error_stays_true(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        result = front.call("fails")
        self.assertTrue(result["isError"])
        self.assertEqual(result["content"][0]["text"], "Error executing tool fails")

class WorkerStartTest(FrontCase):
    def test_a_worker_with_no_reply_within_the_cap_ends_at_it_and_every_reply_says_so(self) -> None:
        # A cap of 0 s ends the wait before the real worker can have answered
        # anything: the deadline has passed by the first look at the reply.
        cap = 0.0
        worker = self.real_worker()
        front = self.front(worker, WORKER_START_CAP_S=cap)
        shown = subprocess.list2cmdline(worker)
        named_cap = f"WORKER_START_CAP_S ({cap} s)"

        hello = front.hello()
        self.assertIn(shown, hello["instructions"])
        self.assertIn(named_cap, hello["instructions"])
        self.assertEqual([t["name"] for t in front.ask("tools/list", {})["tools"]], FRONT_TOOL_NAMES)

        result = front.call("ch_status")
        self.assertTrue(result["isError"])
        self.assertTrue(result["content"][0]["text"].startswith("ch_status: "))
        self.assertIn(shown, result["content"][0]["text"])
        self.assertIn(named_cap, result["content"][0]["text"])
        self.assertIn(named_cap, front.stderr())

    def test_a_worker_that_exits_at_start_names_its_exit_code(self) -> None:
        worker = worker_command("exits_at_start")
        front = self.front(worker)
        hello = front.hello()
        self.assertIn(subprocess.list2cmdline(worker), hello["instructions"])
        self.assertIn("exited with code 3", hello["instructions"])

    def test_a_command_that_cannot_run_is_named(self) -> None:
        worker = [os.path.join(self.folder, "no_such_program.exe")]
        hello = self.front(worker).hello()
        self.assertIn("no_such_program.exe", hello["instructions"])
        self.assertIn("did not run", hello["instructions"])


class WorkerDeathTest(FrontCase):
    def test_an_exit_mid_call_names_the_tool_and_code_and_a_new_worker_answers(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        first_pid = front.call("my_pid")["content"][0]["text"]

        result = front.call("die")

        self.assertTrue(result["isError"])
        text = result["content"][0]["text"]
        self.assertIn("die: the worker exited with code 7", text)
        self.assertIn("started a new worker", text)
        second_pid = front.call("my_pid")["content"][0]["text"]
        self.assertNotEqual(first_pid, second_pid)

    def test_an_exit_between_calls_fails_the_next_call_the_same_way(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        self.assertEqual(front.call("die_after_reply")["content"][0]["text"], "dying soon")
        time.sleep(1.0)

        result = front.call("my_pid")

        self.assertTrue(result["isError"])
        self.assertIn("my_pid: the worker exited with code 8", result["content"][0]["text"])
        self.assertFalse(front.call("my_pid").get("isError"))

    def test_with_nothing_open_the_failed_reply_is_only_the_failure(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        self.assertEqual(len(front.text("die").splitlines()), 1)

    def test_the_new_workers_adoption_line_joins_the_failed_reply(self) -> None:
        self.write_settings(open=[ADOPTION_LINE])
        front = self.front(self.settings_worker())
        front.hello()
        lines = front.text("die").splitlines()
        self.assertIn("die: the worker exited with code 7", lines[0])
        self.assertEqual(lines[1:], [ADOPTION_LINE])

    def test_a_failed_ask_of_the_new_worker_keeps_the_failed_calls_text(self) -> None:
        self.write_settings(open_items="fails")
        front = self.front(self.settings_worker())
        front.hello()
        result = front.call("die")
        self.assertTrue(result["isError"])
        lines = result["content"][0]["text"].splitlines()
        self.assertIn("die: the worker exited with code 7", lines[0])
        self.assertIn("could not read what the new worker holds open", lines[1])

    def test_a_new_worker_that_dies_at_the_ask_keeps_the_failed_calls_text(self) -> None:
        self.write_settings(open_items="dies")
        front = self.front(self.settings_worker())
        front.hello()
        lines = front.text("die").splitlines()
        self.assertIn("die: the worker exited with code 7", lines[0])
        self.assertIn("it exited with code 9", lines[1])

    def test_a_new_worker_that_never_answers_the_ask_ends_at_the_cap(self) -> None:
        self.write_settings(open_items="hang")
        front = self.front(self.settings_worker(), WORKER_START_CAP_S=5.0)
        front.hello()
        lines = front.text("die").splitlines()
        self.assertIn("die: the worker exited with code 7", lines[0])
        self.assertIn("no reply within WORKER_START_CAP_S (5.0 s)", lines[1])


class ReloadTest(FrontCase):
    def reloadable(self, **caps: float) -> Peer:
        front = self.front(self.settings_worker(), **caps)
        front.hello()
        return front

    def test_open_items_refuse_with_every_line_and_the_worker_stays(self) -> None:
        self.write_settings(open=["run: game pid 7, backup folder B", "kit: kit agent attached"])
        front = self.reloadable()
        pid = front.text("my_pid")

        result = front.call("ch_reload")

        self.assertTrue(result["isError"])
        self.assertEqual(
            result["content"][0]["text"],
            "refused: run: game pid 7, backup folder B; kit: kit agent attached; "
            "call ch_stop first, then ch_reload",
        )
        self.assertEqual(front.text("my_pid"), pid)

    def test_a_reload_reports_pids_times_and_the_tool_difference(self) -> None:
        if _REPO_ROOT not in sys.path:
            sys.path.insert(0, _REPO_ROOT)
        from tools.ch_mcp.game_tools import pid_running  # noqa: PLC0415  (its imports are slow for a fake)

        server_py = self.copied_worker()
        front = self.front([sys.executable, server_py])
        hello = front.hello()
        before = front.ask("tools/list", {})["tools"]
        count = len(before) - len(FRONT_TOOL_NAMES)
        names = self.edit_worker(server_py)

        result = front.call("ch_reload")

        self.assertFalse(result.get("isError"))
        lines = result["content"][0]["text"].splitlines()
        pids = re.match(r"^worker process: pid (\d+) stopped, pid (\d+) started", lines[0])
        self.assertIsNotNone(pids, lines[0])
        old_pid, new_pid = (int(pid) for pid in pids.groups())
        self.assertFalse(pid_running(old_pid))
        self.assertTrue(pid_running(new_pid))
        self.assertIn("not the pid in the open-run marker", lines[0])
        self.assertRegex(lines[1], r"^open check: \d+\.\d s$")
        # The real worker exits cleanly when its stdin closes.
        self.assertRegex(lines[2], r"^stop: \d+\.\d s, exit code 0$")
        self.assertRegex(lines[3], r"^start: \d+\.\d s$")
        self.assertIn(f"{count} before, {count} after", lines[4])
        self.assertEqual(lines[5:8], [front_module.names_line(label, names[label])
                                      for label in ("added", "removed", "changed")])
        self.assertEqual(lines[-2:], [f"protocol version with Claude Code: {hello['protocolVersion']}",
                                      "list_changed sent"])
        self.assertIn("notifications/tools/list_changed", front.notifications)
        after = {t["name"]: t for t in front.ask("tools/list", {})["tools"]}
        self.assertIn("ch_added_at_reload", after)
        self.assertNotIn("ch_wait_stats", after)
        self.assertEqual(after["ch_detach"]["description"], "Detach the agent (reworded by the edit).")
        self.assertEqual(front.text("ch_added_at_reload"), "added at reload")

    def test_equal_lists_send_nothing(self) -> None:
        self.write_settings()
        front = self.reloadable()
        text = front.text("ch_reload")
        self.assertIn("added: none\nremoved: none\nchanged: none", text)
        self.assertTrue(text.endswith("list_changed: not sent, lists equal"))
        self.assertNotIn("not declared", text)
        self.assertEqual(front.notifications, [])

    def test_a_connection_on_2026_07_28_says_not_declared_and_sends_nothing(self) -> None:
        version = MODERN_ENVELOPE["io.modelcontextprotocol/protocolVersion"]
        server_py = self.copied_worker()
        front = self.front([sys.executable, server_py])
        # The first reply proves the first worker has loaded its code.
        front.ask("tools/call", {"name": "ch_open_items", "arguments": {}, "_meta": MODERN_ENVELOPE})
        names = self.edit_worker(server_py)
        result = front.ask("tools/call", {"name": "ch_reload", "arguments": {}, "_meta": MODERN_ENVELOPE})
        lines = result["content"][0]["text"].splitlines()
        self.assertIn(front_module.names_line("added", names["added"]), lines)
        self.assertIn(f"protocol version with Claude Code: {version}", lines)
        self.assertEqual(lines[-1], f"list_changed not declared on protocol {version}; use ch_call for a new tool")
        self.assertEqual(front.notifications, [])
        # The reply's way to the new tool works.
        called = front.ask("tools/call", {"name": "ch_call", "_meta": MODERN_ENVELOPE,
                                          "arguments": {"tool": "ch_added_at_reload", "args": {}}})
        self.assertEqual(called["content"][0]["text"], "added at reload")
        # This connection sent no initialize, so it offered no version.
        self.assertIn(f"Claude Code offered no version; this connection runs protocol {version}", front.stderr())

    def test_equal_lists_on_2026_07_28_say_both_lines(self) -> None:
        self.write_settings()
        front = self.front(self.settings_worker())
        result = front.ask("tools/call", {"name": "ch_reload", "arguments": {}, "_meta": MODERN_ENVELOPE})
        lines = result["content"][0]["text"].splitlines()
        self.assertEqual(lines[-2:], [
            "list_changed not declared on protocol 2026-07-28; use ch_call for a new tool",
            "list_changed: not sent, lists equal",
        ])

    def test_an_initialize_offering_2026_07_28_is_answered_as_a_handshake_connection(self) -> None:
        # The mcp package has no 2026-07-28 initialize: it counters with its latest
        # handshake version and declares listChanged, so the front can send.
        self.write_settings()
        front = self.front(self.settings_worker())
        hello = front.hello("2026-07-28")
        self.assertEqual(hello["protocolVersion"], "2025-11-25")
        self.assertTrue(hello["capabilities"]["tools"]["listChanged"])
        self.write_settings(extra_tool=True)
        text = front.text("ch_reload")
        self.assertIn("protocol version with Claude Code: 2025-11-25", text)
        self.assertTrue(text.endswith("list_changed sent"))

    def test_the_protocol_version_is_printed_at_the_handshake(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello("2025-06-18")
        front.call("my_pid")
        self.assertEqual(front.stderr().count("this connection runs protocol 2025-06-18"), 1)
        self.assertIn("Claude Code offered 2025-06-18", front.stderr())

    def test_a_call_in_flight_refuses_the_reload_and_is_named(self) -> None:
        self.write_settings()
        front = self.reloadable()
        pid = front.text("my_pid")
        waiting = front.send("tools/call", {"name": "slow", "arguments": {"seconds": 2.0}})
        time.sleep(0.5)

        result = front.call("ch_reload")

        self.assertTrue(result["isError"])
        self.assertEqual(
            result["content"][0]["text"],
            "refused: a call is in flight: slow; ch_reload after it replies",
        )
        self.assertEqual(front.reply(waiting)["result"]["content"][0]["text"], "slept 2.0")
        self.assertEqual(front.text("my_pid"), pid)

    def test_a_call_in_flight_through_ch_call_is_named_by_its_inner_tool(self) -> None:
        self.write_settings()
        front = self.reloadable()
        waiting = front.send("tools/call", {
            "name": "ch_call", "arguments": {"tool": "slow", "args": {"seconds": 2.0}},
        })
        time.sleep(0.5)
        self.assertIn("a call is in flight: slow;", front.text("ch_reload"))
        front.reply(waiting)

    def test_a_worker_slow_to_exit_is_terminated_at_the_cap_and_the_reply_says_so(self) -> None:
        front = self.front(worker_command("slow_exit"), WORKER_EXIT_CAP_S=1.0)
        front.hello()
        old_pid = front.text("my_pid")

        text = front.text("ch_reload")

        self.assertIn("stop: WORKER_EXIT_CAP_S (1.0 s) ended the wait for pid", text)
        self.assertIn("the front terminated it", text)
        self.assertNotEqual(front.text("my_pid"), old_pid)

    def test_a_worker_that_never_answers_the_check_is_left_running(self) -> None:
        self.write_settings(open_items="hang")
        front = self.reloadable(WORKER_EXIT_CAP_S=1.0)
        pid = front.text("my_pid")
        waiting = front.send("tools/call", {"name": "slow", "arguments": {"seconds": 3.0}})
        time.sleep(0.3)

        result = front.call("ch_reload")

        self.assertTrue(result["isError"])
        text = result["content"][0]["text"]
        self.assertIn("no reply to ch_open_items within WORKER_EXIT_CAP_S (1.0 s)", text)
        self.assertIn("calls in flight: slow", text)
        front.reply(waiting)
        self.assertEqual(front.text("my_pid"), pid)

    def test_a_failed_check_refuses(self) -> None:
        self.write_settings(open_items="fails")
        front = self.reloadable()
        pid = front.text("my_pid")
        result = front.call("ch_reload")
        self.assertTrue(result["isError"])
        self.assertIn("refused: ch_open_items failed", result["content"][0]["text"])
        self.assertEqual(front.text("my_pid"), pid)

    def test_a_worker_dead_before_the_check_is_replaced_and_the_reply_says_so(self) -> None:
        self.write_settings()
        front = self.reloadable()
        self.assertEqual(front.text("die_after_reply"), "dying soon")
        time.sleep(1.0)
        result = front.call("ch_reload")
        self.assertTrue(result["isError"])
        self.assertIn("ch_reload: the worker exited with code 8", result["content"][0]["text"])
        self.assertFalse(front.call("my_pid").get("isError"))

    def test_a_new_worker_that_fails_to_start_is_reported_and_the_next_call_retries(self) -> None:
        self.write_settings()
        front = self.reloadable()
        self.write_settings(exit_at_start=True)

        result = front.call("ch_reload")

        self.assertTrue(result["isError"])
        text = result["content"][0]["text"]
        self.assertRegex(text, r"^stop: \d+\.\d s, exit code 0\n")
        self.assertIn("ch_reload: no new worker started:", text)
        self.assertIn("exited with code 3", text)
        self.write_settings()
        self.assertFalse(front.call("my_pid").get("isError"))

    def test_changed_instructions_are_live_at_the_next_reconnect(self) -> None:
        self.write_settings()
        front = self.reloadable()
        self.assertNotIn("instructions changed", front.text("ch_reload"))
        self.write_settings(instructions="New instructions.")
        self.assertIn("instructions changed; live at the next reconnect", front.text("ch_reload"))

    def test_the_new_workers_adoption_line_joins_the_reload_reply(self) -> None:
        self.write_settings()
        front = self.reloadable()
        self.write_settings(held_from_start=[ADOPTION_LINE])
        lines = front.text("ch_reload").splitlines()
        self.assertIn(ADOPTION_LINE, lines)
        self.assertIn("added: none", lines)

    def test_a_call_after_a_reload_goes_to_the_new_worker_and_a_tool_is_read_from_it(self) -> None:
        self.write_settings()
        front = self.reloadable()
        self.write_settings(extra_tool=True)
        front.call("ch_reload")
        self.assertEqual(front.text("ch_call", {"tool": "extra_tool", "args": {}}), "extra tool answered")


class ChCallTest(FrontCase):
    def test_it_forwards_and_returns_the_tools_reply(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        self.assertEqual(front.call("ch_call", {"tool": "ch_state", "args": {}}), front.call("ch_state"))
        echoed = front.text("ch_call", {"tool": "echo_args", "args": {"text": "é", "numbers": [1]}})
        self.assertEqual(json.loads(echoed), {"text": "é", "numbers": [1], "nested": None})

    def test_an_unknown_name_is_refused_with_the_list(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        result = front.call("ch_call", {"tool": "nope", "args": {}})
        self.assertTrue(result["isError"])
        text = result["content"][0]["text"]
        self.assertIn("the worker has no tool 'nope'", text)
        self.assertIn("ch_state", text)
        self.assertIn("my_pid", text)

    def test_ch_call_and_ch_reload_are_refused_inside_it(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        pid = front.text("my_pid")
        for tool in FRONT_TOOL_NAMES:
            with self.subTest(tool):
                result = front.call("ch_call", {"tool": tool, "args": {"tool": "ch_state", "args": {}}})
                self.assertTrue(result["isError"])
                self.assertEqual(result["content"][0]["text"], f"refused: {tool} does not go through ch_call")
        self.assertEqual(front.text("my_pid"), pid)

    def test_shapes_the_hook_cannot_read_reach_no_worker(self) -> None:
        front = self.front(self.real_worker())
        front.hello()
        path = {"path": os.path.join(self.folder, "no_such_stats.json")}
        calls = (
            {"tool": 7, "args": path},
            {"tool": None, "args": path},
            {"args": path},
            {"tool": "ch_stats"},
            {"tool": "ch_stats", "args": json.dumps(path)},
            {"tool": "ch_stats", "args": [path]},
            {"tool": "ch_stats", "args": None},
        )
        for arguments in calls:
            with self.subTest(arguments):
                refusal = front_module.call_shape_refusal(arguments)
                self.assertIsNotNone(refusal)
                result = front.call("ch_call", arguments)
                # The reply is the front's own refusal, not the worker's
                # "could not read" answer that a forwarded ch_stats gives.
                self.assertTrue(result["isError"])
                self.assertEqual(result["content"][0]["text"], refusal)
        # A shape the front reads goes through, and its reply is the worker's.
        self.assertEqual(front.call("ch_call", {"tool": "ch_stats", "args": path}), front.call("ch_stats", path))

    def test_args_has_no_default_in_the_schema(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        tool = next(t for t in front.ask("tools/list", {})["tools"] if t["name"] == "ch_call")
        owner = next(t for t in front_module.FRONT_TOOLS if t.name == front_module.CALL_TOOL)
        self.assertEqual(tool["inputSchema"], owner.input_schema)
        self.assertIn("args", tool["inputSchema"]["required"])
        self.assertNotIn("default", tool["inputSchema"]["properties"]["args"])

    def test_a_front_whose_first_start_failed_starts_a_worker_for_ch_call(self) -> None:
        self.write_settings(exit_at_start=True)
        front = self.front(self.settings_worker())
        self.assertIn("exited with code 3", front.hello()["instructions"])

        result = front.call("ch_call", {"tool": "ch_state", "args": {}})
        self.assertTrue(result["isError"])
        self.assertIn("ch_call: the worker command", result["content"][0]["text"])

        self.write_settings()
        self.assertEqual(front.text("ch_call", {"tool": "ch_state", "args": {}}), "fake state")


class WorkerOutputTest(FrontCase):
    def test_lines_that_are_not_replies_are_skipped_and_the_call_still_answers(self) -> None:
        front = self.front(worker_command("raw"))
        front.hello()
        self.assertEqual(front.call("junk_then_reply")["content"][0]["text"], "after junk")
        self.assertIn("not JSON-RPC", front.stderr())

    def test_a_worker_that_closes_its_output_fails_the_open_call_and_is_replaced(self) -> None:
        front = self.front(worker_command("raw"))
        front.hello()
        first_pid = front.call("my_pid")["content"][0]["text"]

        result = front.call("close_output")

        self.assertTrue(result["isError"])
        text = result["content"][0]["text"]
        self.assertIn("close_output: the worker closed its output while it still ran", text)
        self.assertIn("started a new worker", text)
        self.assertNotEqual(front.call("my_pid")["content"][0]["text"], first_pid)


class FrontExitTest(FrontCase):
    def test_a_quiet_worker_exits_with_the_front(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        front.close_stdin()
        self.wait_exit(front)
        self.assertEqual(front.proc.returncode, 0)
        self.assertRegex(front.stderr(), r"worker exited in \d+\.\d s")
        self.assertIn(WORKER_STDERR_LINE, front.stderr())

    def test_a_slow_worker_is_terminated_at_the_cap_and_stderr_says_so(self) -> None:
        front = self.front(worker_command("slow_exit"), FRONT_EXIT_CAP_S=1.0)
        front.hello()
        pid = int(front.call("my_pid")["content"][0]["text"])
        front.close_stdin()
        self.assertLess(self.wait_exit(front), SLOW_EXIT_S - 10)
        self.assertIn("FRONT_EXIT_CAP_S (1.0 s) ended the wait", front.stderr())
        self.assertIn(str(pid), front.stderr())

    def test_an_open_call_does_not_hold_the_exit_past_the_cap(self) -> None:
        front = self.front(worker_command("slow_exit"), FRONT_EXIT_CAP_S=1.0)
        front.hello()
        front.send("tools/call", {"name": "slow", "arguments": {"seconds": 50}})
        time.sleep(0.5)
        front.close_stdin()
        self.assertLess(self.wait_exit(front), SLOW_EXIT_S - 10)
        self.assertIn("FRONT_EXIT_CAP_S (1.0 s) ended the wait", front.stderr())


@unittest.skipUnless(shutil.which("py"), "the py launcher is what .mcp.json runs, and it is absent")
class PyLauncherTest(FrontCase):
    """.mcp.json's command is `py`, a launcher with Python as its child, so the
    process the front holds is not the one running the tools."""

    def test_exit_code_and_a_new_worker_pass_through_the_launcher(self) -> None:
        front = self.front(worker_command("plain", python="py"))
        front.hello()
        self.assertIn("exited with code 7", front.call("die")["content"][0]["text"])
        self.assertFalse(front.call("my_pid").get("isError"))

    def test_a_reload_labels_the_launchers_pid_as_the_launchers(self) -> None:
        front = self.front(worker_command("plain", python="py"))
        front.hello()
        python_pid = front.text("my_pid")
        line = front.text("ch_reload").splitlines()[0]
        self.assertRegex(line, r"^worker process \(py launcher\): pid \d+ stopped, pid \d+ started")
        self.assertNotIn(f"pid {python_pid} ", line)

    def test_terminate_at_the_cap_reaches_the_python_behind_the_launcher(self) -> None:
        front = self.front(worker_command("slow_exit", python="py"), FRONT_EXIT_CAP_S=1.0)
        front.hello()
        pid = int(front.call("my_pid")["content"][0]["text"])
        front.close_stdin()
        self.wait_exit(front)
        self.assertIn("FRONT_EXIT_CAP_S (1.0 s) ended the wait", front.stderr())
        # tasklist answers with the process when it still runs, and an INFO line when not.
        deadline = time.monotonic() + 5
        listed = ""
        while time.monotonic() < deadline:
            listed = subprocess.run(
                ["tasklist", "/FI", f"PID eq {pid}", "/NH"],
                capture_output=True, text=True, timeout=READ_CAP_S,
            ).stdout
            if str(pid) not in listed:
                break
            time.sleep(0.2)
        self.assertNotIn(str(pid), listed)


class SourceTest(unittest.TestCase):
    def setUp(self) -> None:
        with open(FRONT_PY, encoding="utf-8") as source:
            self.source = source.read()

    def test_the_front_loads_none_of_the_repos_code_but_its_own(self) -> None:
        # So a ch_reload brings every merged line of the worker's code live:
        # the front, which never restarts, holds none of it.
        probe = (
            f"import json, sys; sys.path.insert(0, {os.path.dirname(FRONT_PY)!r}); import front; "
            "print(json.dumps([getattr(m, '__file__', None) for m in list(sys.modules.values())]))"
        )
        loaded = subprocess.run([sys.executable, "-I", "-c", probe], capture_output=True, text=True,
                                timeout=READ_CAP_S, check=True).stdout
        repo = os.path.normcase(_REPO_ROOT) + os.sep
        from_repo = sorted({os.path.normcase(os.path.abspath(path)) for path in json.loads(loaded)
                            if path and os.path.normcase(os.path.abspath(path)).startswith(repo)})
        self.assertEqual(from_repo, [os.path.normcase(FRONT_PY)])

    def test_the_front_holds_no_reply_cut_and_no_process_group(self) -> None:
        for word in ("REPLY_CAP_CHARS", "win32job", "Job"):
            self.assertNotIn(word, self.source)


if __name__ == "__main__":
    if sys.argv[1:2] == ["--fake-worker"]:
        fake_worker(*sys.argv[2:4])
