"""Tests for the front process: forwarding, a worker's death and the caps.

Every test runs the real front as a process and speaks MCP's stdio lines to it.
The worker is this same file run with --fake-worker, which serves a real
MCPServer over its pipes. Every read has a cap, and a fake ends itself after
FAKE_LIFE_S, so a hung fake cannot hang the run.
"""

from __future__ import annotations

import ast
import json
import os
import queue
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

# The longest any fake lives, and the longest a test waits on one reply or exit.
FAKE_LIFE_S = 60
READ_CAP_S = 30

WORKER_INSTRUCTIONS = "Fake worker instructions."
WORKER_STDERR_LINE = "fake worker line on stderr"
LONG_TEXT = "é" + "x" * 11_998 + "é"
SLOW_EXIT_S = 40


def fake_worker(mode: str) -> None:
    """Serve the fake tools over stdio; `mode` picks how the fake misbehaves."""
    timer = threading.Timer(FAKE_LIFE_S, os._exit, args=(99,))
    timer.daemon = True
    timer.start()
    if mode == "mute":
        time.sleep(FAKE_LIFE_S)
        return
    if mode == "exits_at_start":
        os._exit(3)

    import asyncio
    import atexit

    from mcp.server.mcpserver import MCPServer

    if mode == "slow_exit":
        atexit.register(time.sleep, SLOW_EXIT_S)

    server = MCPServer("fake", instructions=WORKER_INSTRUCTIONS)

    @server.tool()
    def echo_args(text: str, numbers: list[int] | None = None, nested: dict | None = None) -> str:
        """Say back the arguments as JSON."""
        return json.dumps({"text": text, "numbers": numbers, "nested": nested}, sort_keys=True)

    @server.tool()
    def long_text() -> str:
        """A reply of exactly the worker's cut length."""
        return LONG_TEXT

    @server.tool()
    def fails() -> str:
        """A tool whose reply has isError true."""
        raise ValueError("the fake failed on purpose")

    @server.tool()
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


def worker_command(mode: str, python: str = sys.executable) -> list[str]:
    return [python, os.path.abspath(__file__), "--fake-worker", mode]


class Peer:
    """A process spoken to in MCP stdio lines; every read has a cap."""

    def __init__(self, command: list[str], stderr_path: str) -> None:
        self._stderr_file = open(stderr_path, "wb")
        self.stderr_path = stderr_path
        self.proc = subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=self._stderr_file
        )
        self._lines: queue.Queue = queue.Queue()
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
        return held.pop(number)

    def ask(self, method: str, params: dict) -> dict:
        return self.reply(self.send(method, params))["result"]

    def hello(self) -> dict:
        result = self.ask("initialize", {
            "protocolVersion": "2025-11-25",
            "capabilities": {},
            "clientInfo": {"name": "test", "version": "1"},
        })
        self._write({"jsonrpc": "2.0", "method": "notifications/initialized"})
        return result

    def call(self, tool: str, arguments: dict | None = None) -> dict:
        return self.ask("tools/call", {"name": tool, "arguments": arguments or {}})

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
        self._count = 0

    def peer(self, command: list[str]) -> Peer:
        self._count += 1
        peer = Peer(command, os.path.join(self.folder, f"stderr{self._count}.txt"))
        self.addCleanup(peer.cleanup)
        return peer

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
        direct = self.peer(worker_command("plain"))
        worker_hello = direct.hello()
        worker_tools = direct.ask("tools/list", {})["tools"]

        front = self.front(worker_command("plain"))
        hello = front.hello()
        tools = front.ask("tools/list", {})["tools"]

        self.assertEqual(
            [(t["name"], t["description"], t["inputSchema"], t.get("outputSchema")) for t in tools],
            [(t["name"], t["description"], t["inputSchema"], t.get("outputSchema")) for t in worker_tools],
        )
        self.assertEqual(tools, worker_tools)
        self.assertEqual(worker_hello["instructions"], WORKER_INSTRUCTIONS)
        self.assertTrue(hello["instructions"].startswith(WORKER_INSTRUCTIONS + " "))
        self.assertIn("ch_reload", hello["instructions"])
        self.assertIn("ch_call", hello["instructions"])
        self.assertTrue(hello["capabilities"]["tools"]["listChanged"])

    def test_a_call_comes_back_as_the_worker_sent_it(self) -> None:
        direct = self.peer(worker_command("plain"))
        direct.hello()
        front = self.front(worker_command("plain"))
        front.hello()
        arguments = {"text": "é\n\"quoted\"", "numbers": [1, 2], "nested": {"a": [None, True]}}

        for tool, args in (("echo_args", arguments), ("long_text", {}), ("fails", {})):
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

    def test_replies_find_their_calls_out_of_order(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        slow = front.send("tools/call", {"name": "slow", "arguments": {"seconds": 1.5}})
        quick = front.send("tools/call", {"name": "slow", "arguments": {"seconds": 0.0}})
        held: dict = {}
        self.assertEqual(front.reply(quick, held)["result"]["content"][0]["text"], "slept 0.0")
        self.assertEqual(front.reply(slow, held)["result"]["content"][0]["text"], "slept 1.5")


class WorkerStartTest(FrontCase):
    def test_a_mute_worker_ends_at_the_cap_and_every_reply_says_so(self) -> None:
        worker = worker_command("mute")
        front = self.front(worker, WORKER_START_CAP_S=1.0)
        shown = subprocess.list2cmdline(worker)

        hello = front.hello()
        self.assertIn(shown, hello["instructions"])
        self.assertIn("WORKER_START_CAP_S (1.0 s)", hello["instructions"])
        self.assertEqual(front.ask("tools/list", {})["tools"], [])

        result = front.call("my_pid")
        self.assertTrue(result["isError"])
        self.assertIn(shown, result["content"][0]["text"])
        self.assertIn("WORKER_START_CAP_S (1.0 s)", result["content"][0]["text"])
        self.assertIn("MCP log", result["content"][0]["text"])
        self.assertIn("WORKER_START_CAP_S (1.0 s)", front.stderr())

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

    def test_two_calls_open_at_a_death_start_one_new_worker(self) -> None:
        front = self.front(worker_command("plain"))
        front.hello()
        waiting = front.send("tools/call", {"name": "slow", "arguments": {"seconds": 20}})
        dying = front.send("tools/call", {"name": "die", "arguments": {}})
        held: dict = {}
        for number in (waiting, dying):
            result = front.reply(number, held)["result"]
            self.assertIn("exited with code 7", result["content"][0]["text"])
        self.assertFalse(front.call("my_pid").get("isError"))
        self.assertEqual(front.stderr().count(" started in "), 2)


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

    def test_the_front_imports_only_the_mcp_package_and_the_standard_library(self) -> None:
        roots = set()
        for node in ast.walk(ast.parse(self.source)):
            if isinstance(node, ast.Import):
                roots.update(alias.name.split(".")[0] for alias in node.names)
            elif isinstance(node, ast.ImportFrom):
                roots.add((node.module or "").split(".")[0])
        self.assertEqual(roots - set(sys.stdlib_module_names), {"mcp", "mcp_types"})

    def test_the_front_holds_no_reply_cut_and_no_process_group(self) -> None:
        for word in ("REPLY_CAP_CHARS", "win32job", "Job"):
            self.assertNotIn(word, self.source)

    def test_the_caps_are_the_users_numbers(self) -> None:
        sys.path.insert(0, os.path.dirname(FRONT_PY))
        self.addCleanup(sys.path.remove, os.path.dirname(FRONT_PY))
        import front

        self.assertEqual(
            (front.WORKER_START_CAP_S, front.WORKER_EXIT_CAP_S, front.FRONT_EXIT_CAP_S, front.FRONT_POLL_S),
            (30, 10, 90, 0.5),
        )


if __name__ == "__main__":
    if sys.argv[1:2] == ["--fake-worker"]:
        fake_worker(sys.argv[2])
