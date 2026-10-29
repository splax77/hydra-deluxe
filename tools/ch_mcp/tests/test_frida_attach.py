"""Tests for frida_attach: the attach runner detaches on every path, the clock
summary is right, and the Frida agents contain nothing that writes to the game.

No game and no Frida: a fake frida module hands back a fake session whose
script delivers hand-written messages, and a fake wait returns at once.
"""

from __future__ import annotations

import io
import json
import os
import re
import sys
import tempfile
import threading
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_probe.constants import PROCESS_NAME  # noqa: E402

AGENT_DIR = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent")

# ---- the no-writes check ------------------------------------------------
# The one list of what an agent script may never contain, each with why.
# Task D's tests call no_write_findings over trace.ts with this same list.
NO_WRITE_PATTERNS = (
    (r"Memory\.write", "writes game memory"),
    (r"\.write[A-Z]\w*\(", "writes game memory through a pointer"),
    (r"Memory\.(patchCode|protect|copy)\b",
     "changes game memory, code or page rights"),
    (r"\.value\s*(?:\+\+|--|(?:[-+*/%&|^]|<<|>>>?|\*\*|&&|\|\||\?\?)?=(?![=>]))",
     "sets a field"),
    (r"\.invoke\(", "calls a game method"),
    (r"new\s+NativeFunction\b", "calls a game function"),
    (r"\.implementation\s*=(?!=)", "replaces a game method"),
    (r"Interceptor\.replace", "replaces a game function"),
    (r"\bretval\.replace\(", "changes a return value"),
    (r"\bargs\[[^\]]+\]\s*=(?![=>])", "changes a call argument"),
    (r"\b(?:\w+\.)?(?:context|ctx)\.\w+\s*=(?![=>])", "changes a register"),
    (r"add\(0x", "reads by offset, not by field name"),
)


def no_write_findings(text: str) -> list[tuple[int, str, str]]:
    """Every line of an agent script that matches NO_WRITE_PATTERNS, as
    (line number, line, why)."""
    found = []
    for n, line in enumerate(text.splitlines(), start=1):
        for pattern, why in NO_WRITE_PATTERNS:
            if re.search(pattern, line):
                found.append((n, line.strip(), why))
    return found


def agent_sources() -> list[str]:
    """The TypeScript agents beside package.json (not node_modules)."""
    return sorted(os.path.join(AGENT_DIR, name)
                  for name in os.listdir(AGENT_DIR) if name.endswith(".ts"))


# ---- fakes ----------------------------------------------------------------

class FakeProcessNotFound(Exception):
    pass


class FakeScript:
    def __init__(self, session):
        self.session = session
        self.handlers = {}
        self.posted = []

    def on(self, event, handler):
        self.handlers[event] = handler

    def load(self):
        if self.session.load_raises is not None:
            raise self.session.load_raises

    def post(self, message):
        self.posted.append(message)
        for m in self.session.messages:
            self.handlers["message"](m, None)


class FakeSession:
    def __init__(self, messages, load_raises=None):
        self.messages = messages
        self.load_raises = load_raises
        self.handlers = {}
        self.detach_calls = 0
        self.script = None
        self.source = None

    def on(self, event, handler):
        self.handlers[event] = handler

    def create_script(self, source):
        self.source = source
        self.script = FakeScript(self)
        return self.script

    def detach(self):
        # Real Frida answers the runner's own detach by calling the session's
        # "detached" handler with this reason.
        self.detach_calls += 1
        handler = self.handlers.get("detached")
        if handler is not None:
            handler("application-requested", None)


class FakeFrida:
    ProcessNotFoundError = FakeProcessNotFound

    def __init__(self, session=None):
        self.session = session
        self.attached_to = []

    def attach(self, name):
        self.attached_to.append(name)
        if self.session is None:
            raise FakeProcessNotFound(f"unable to find process with name '{name}'")
        return self.session


def instant_wait(event, timeout_s):
    """Returns at once: True when the run already ended, False as if the cap
    had passed."""
    return event.is_set()


def sent(payload):
    return {"type": "send", "payload": payload}


def sample(song_time, over=False, playing=True):
    return {"t": 1000, "songTime": song_time, "isSongOver": over,
            "isSongPlaying": playing, "isPaused": False, "instances": 1}


class Workspace(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.agent = os.path.join(self.tmp.name, "agent.js")
        with open(self.agent, "w", encoding="utf-8") as fh:
            fh.write("// fake agent\n")
        self.out_path = os.path.join(self.tmp.name, "out.jsonl")
        self.lines = []

    def run_agent(self, frida, cap_s=93.0):
        return FA.run_agent(self.agent, cap_s, FA.ends_run, self.out_path,
                            frida_mod=frida, wait=instant_wait,
                            out=self.lines.append)

    def written(self):
        with open(self.out_path, encoding="utf-8") as fh:
            return [json.loads(line) for line in fh]


# ---- the runner -----------------------------------------------------------

class DetachTest(Workspace):
    def test_detaches_after_a_normal_end(self):
        msgs = [sent(sample(1.0)), sent(sample(1.25)),
                sent({"done": "cap", "samples": 2})]
        session = FakeSession(msgs)
        result = self.run_agent(FakeFrida(session))
        self.assertEqual(result.reason, FA.END_DONE)
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.written(), msgs)
        self.assertEqual(session.script.posted,
                         [{"type": "start", "cap_s": 93.0}])
        self.assertEqual(session.source, "// fake agent\n")
        self.assertEqual(self.lines, [])

    def test_start_settings_ride_in_the_start_message(self):
        session = FakeSession([sent({"done": "cap"})])
        FA.run_agent(self.agent, 93.0, FA.ends_run, self.out_path,
                     start={"mode": "count", "type": "other", "cap_s": 1},
                     frida_mod=FakeFrida(session), wait=instant_wait,
                     out=self.lines.append)
        # The runner's own `type` and `cap_s` win over the agent's settings.
        self.assertEqual(session.script.posted,
                         [{"type": "start", "cap_s": 93.0, "mode": "count"}])

    def test_detaches_after_the_cap(self):
        session = FakeSession([sent(sample(1.0))])
        result = self.run_agent(FakeFrida(session))
        self.assertEqual(result.reason, FA.END_CAP)
        self.assertIsNone(result.error)
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.lines, [
            "cap of 93 s reached before the agent finished; 1 messages seen"])

    def test_detaches_after_a_thrown_error(self):
        session = FakeSession([], load_raises=RuntimeError("bad script"))
        with self.assertRaises(RuntimeError):
            self.run_agent(FakeFrida(session))
        self.assertEqual(session.detach_calls, 1)

    def test_detaches_after_ctrl_c(self):
        session = FakeSession([], load_raises=KeyboardInterrupt())
        with self.assertRaises(KeyboardInterrupt):
            self.run_agent(FakeFrida(session))
        self.assertEqual(session.detach_calls, 1)

    def test_attaches_to_the_game_by_process_name(self):
        frida = FakeFrida(FakeSession([sent({"done": "cap"})]))
        self.run_agent(frida)
        self.assertEqual(frida.attached_to, [PROCESS_NAME])


class EndTest(Workspace):
    def test_an_agent_error_ends_the_run_and_is_kept(self):
        session = FakeSession([sent({"error": "GameManager not found"}),
                               sent(sample(9.0))])
        result = self.run_agent(FakeFrida(session))
        self.assertEqual(result.reason, FA.END_AGENT_ERROR)
        self.assertEqual(result.error, "GameManager not found")
        self.assertEqual(session.detach_calls, 1)

    def test_a_script_exception_ends_the_run(self):
        session = FakeSession([{"type": "error",
                                "description": "TypeError: boom"}])
        result = self.run_agent(FakeFrida(session))
        self.assertEqual(result.reason, FA.END_SCRIPT_ERROR)
        self.assertEqual(result.error, "TypeError: boom")

    def test_no_message_is_written_after_the_run_ends(self):
        session = FakeSession([sent({"done": "cap"})])
        self.run_agent(FakeFrida(session))
        session.script.handlers["message"](sent(sample(5.0)), None)
        self.assertEqual(self.written(), [sent({"done": "cap"})])

    def test_the_game_closing_ends_the_run(self):
        session = FakeSession([])
        frida = FakeFrida(session)

        def wait(event, timeout_s):
            session.handlers["detached"]("process-terminated", None)
            return event.is_set()

        result = FA.run_agent(self.agent, 93.0, FA.ends_run, self.out_path,
                              frida_mod=frida, wait=wait,
                              out=self.lines.append)
        self.assertEqual(result.reason, FA.END_DETACHED)
        self.assertEqual(result.error, "session detached: process-terminated")
        self.assertEqual(session.detach_calls, 1)


class DroppedSessionDetachTest(Workspace):
    def test_a_detach_that_raises_is_reported_and_the_log_still_closes(self):
        session = FakeSession([sent(sample(1.0))])

        def refuse():
            session.detach_calls += 1
            raise RuntimeError("session is gone")

        session.detach = refuse
        result = self.run_agent(FakeFrida(session))
        self.assertEqual(session.detach_calls, 1)
        self.assertIn("detach reported: session is gone", self.lines)
        self.assertEqual(result.reason, FA.END_CAP)
        # The log was closed: it reads back whole and accepts no late write.
        self.assertEqual(self.written(), [sent(sample(1.0))])
        session.script.handlers["message"](sent(sample(2.0)), None)
        self.assertEqual(self.written(), [sent(sample(1.0))])


class DefaultWaitTest(unittest.TestCase):
    def test_returns_true_at_once_when_the_event_is_set(self):
        event = threading.Event()
        event.set()
        self.assertTrue(FA._default_wait(event, 30.0))

    def test_returns_false_when_the_cap_passes(self):
        self.assertFalse(FA._default_wait(threading.Event(), 0.05))

    def test_wakes_when_the_event_is_set_during_the_wait(self):
        event = threading.Event()
        threading.Timer(0.05, event.set).start()
        self.assertTrue(FA._default_wait(event, 30.0))


class NotRunningTest(Workspace):
    def test_not_running_says_so_and_stops(self):
        result = self.run_agent(FakeFrida(None))
        self.assertEqual(result.reason, FA.END_NOT_RUNNING)
        self.assertEqual(self.lines,
                         ["Clone Hero.exe is not running; nothing attached"])
        self.assertFalse(os.path.exists(self.out_path))

    def test_the_command_line_exits_1_when_not_running(self):
        code = FA.main(["--agent", self.agent, "--cap", "93",
                        "--out", self.out_path],
                       frida_mod=FakeFrida(None), wait=instant_wait,
                       out=self.lines.append)
        self.assertEqual(code, 1)
        self.assertEqual(self.lines,
                         ["Clone Hero.exe is not running; nothing attached"])


# ---- the long-lived agent -----------------------------------------------------

class ReplyScript(FakeScript):
    """Answers each read with the session's next reply. A dict goes back as
    the agent would send it, carrying the read's id; a list is delivered
    message by message as written; None sends nothing, as an agent that
    never answers."""

    def post(self, message):
        self.posted.append(message)
        if not self.session.replies:
            return
        reply = self.session.replies.pop(0)
        if reply is None:
            return
        if isinstance(reply, list):
            for m in reply:
                self.handlers["message"](m, None)
            return
        self.handlers["message"](sent({**reply, "id": message["id"]}), None)


class ReplySession(FakeSession):
    def __init__(self, replies, load_raises=None):
        super().__init__([], load_raises)
        self.replies = list(replies)

    def create_script(self, source):
        self.source = source
        self.script = ReplyScript(self)
        return self.script


# What state.ts sends, by the field names it reads.
STATE = {"found": True, "instances": 1, "songTime": 3.5, "isPaused": False,
         "isSongOver": False, "isSongPlaying": True, "songLoaded": True}
MENU = {"found": False, "reason": "no live GameManager"}


class LiveAgentTest(Workspace):
    def live(self, frida, wait=instant_wait):
        agent = FA.LiveAgent(self.agent, log_path=self.out_path,
                             frida_mod=frida, wait=wait, out=self.lines.append)
        # Runs before the temp folder's cleanup, so Windows can delete the
        # log; detach is safe to call again.
        self.addCleanup(agent.detach)
        return agent

    def started(self, replies):
        session = ReplySession(replies)
        agent = self.live(FakeFrida(session))
        agent.start(30.0)
        return agent, session

    def test_start_posts_a_read_and_returns_its_reply(self):
        session = ReplySession([STATE])
        frida = FakeFrida(session)
        agent = self.live(frida)
        self.assertEqual(agent.start(30.0), STATE)
        self.assertEqual(frida.attached_to, [PROCESS_NAME])
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1}])
        self.assertEqual(session.source, "// fake agent\n")
        self.assertTrue(agent.alive)
        self.assertEqual(session.detach_calls, 0)
        self.assertEqual(self.written(), [sent({**STATE, "id": 1})])

    def test_start_on_a_game_that_is_not_running_attaches_nothing(self):
        agent = self.live(FakeFrida(None))
        with self.assertRaises(FA.AttachFailed) as caught:
            agent.start(30.0)
        self.assertEqual(str(caught.exception),
                         "Clone Hero.exe is not running; nothing attached")
        self.assertEqual(caught.exception.reason, FA.END_NOT_RUNNING)
        self.assertFalse(agent.alive)
        self.assertFalse(os.path.exists(self.out_path))
        self.assertEqual(agent.detach(), "nothing attached")

    def test_start_with_no_first_reply_raises_on_the_cap_and_detaches_once(self):
        session = ReplySession([None])
        agent = self.live(FakeFrida(session))
        with self.assertRaises(FA.AttachFailed) as caught:
            agent.start(30.0)
        self.assertEqual(str(caught.exception),
                         "no reply to the first read within the cap of 30 s; "
                         "detached")
        self.assertEqual(caught.exception.reason, FA.END_CAP)
        self.assertEqual(session.detach_calls, 1)
        self.assertFalse(agent.alive)
        self.assertEqual(agent.detach(),
                         "already detached (detached by the caller)")
        self.assertEqual(session.detach_calls, 1)

    def test_start_whose_agent_fails_to_load_detaches_once(self):
        session = ReplySession([], load_raises=RuntimeError("bad script"))
        agent = self.live(FakeFrida(session))
        with self.assertRaises(FA.AttachFailed) as caught:
            agent.start(30.0)
        self.assertEqual(str(caught.exception),
                         "script error: loading the agent failed: bad script")
        self.assertEqual(caught.exception.reason, FA.END_SCRIPT_ERROR)
        self.assertEqual(session.detach_calls, 1)

    def test_start_detaches_on_ctrl_c(self):
        session = ReplySession([], load_raises=KeyboardInterrupt())
        agent = self.live(FakeFrida(session))
        with self.assertRaises(KeyboardInterrupt):
            agent.start(30.0)
        self.assertEqual(session.detach_calls, 1)

    def test_read_returns_the_next_payload_and_the_log_holds_it(self):
        later = {**STATE, "songTime": 4.0}
        agent, session = self.started([STATE, later])
        self.assertEqual(agent.read(5.0), later)
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1},
                                                 {"type": "read", "id": 2}])
        self.assertEqual(self.written(), [sent({**STATE, "id": 1}),
                                          sent({**later, "id": 2})])

    def test_no_live_game_manager_is_returned_as_data(self):
        agent, _ = self.started([STATE, MENU])
        self.assertEqual(agent.read(5.0), MENU)
        self.assertTrue(agent.alive)

    def test_read_with_no_reply_names_the_cap(self):
        agent, _ = self.started([STATE, None])
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "no reply to the read within the cap of 5 s")
        self.assertEqual(caught.exception.reason, FA.END_CAP)

    def test_a_late_reply_is_not_taken_by_the_next_read(self):
        stale = {**STATE, "songTime": 4.0, "id": 2}
        fresh = {**STATE, "songTime": 9.0, "id": 3}
        agent, _ = self.started([STATE, None, [sent(stale), sent(fresh)]])
        with self.assertRaises(FA.ReadFailed):
            agent.read(5.0)
        self.assertEqual(agent.read(5.0)["songTime"], 9.0)
        # Both still reach the log.
        self.assertEqual(self.written()[-2:], [sent(stale), sent(fresh)])

    def test_read_after_the_game_closes_names_the_reason(self):
        agent, session = self.started([STATE, STATE])
        session.handlers["detached"]("process-terminated", None)
        self.assertFalse(agent.alive)
        self.assertEqual(agent.gone_reason,
                         "session detached: process-terminated")
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "no session for the read: "
                         "session detached: process-terminated")
        self.assertEqual(caught.exception.reason, FA.END_DETACHED)
        self.assertEqual(len(session.script.posted), 1)

    def test_a_detach_during_a_read_ends_that_read(self):
        session = ReplySession([STATE, None])
        calls = []

        def wait(event, timeout_s):
            calls.append(timeout_s)
            if len(calls) == 2:
                session.handlers["detached"]("process-terminated", None)
            return event.is_set()

        agent = self.live(FakeFrida(session), wait=wait)
        agent.start(30.0)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "session detached: process-terminated")
        self.assertEqual(caught.exception.reason, FA.END_DETACHED)

    def test_an_agent_error_fails_the_read(self):
        agent, _ = self.started(
            [STATE, {"error": "reading GameManager failed: boom"}])
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "the agent sent an error: "
                         "reading GameManager failed: boom")
        self.assertEqual(caught.exception.reason, FA.END_AGENT_ERROR)

    def test_a_script_exception_fails_the_read(self):
        agent, _ = self.started(
            [STATE, [{"type": "error", "description": "TypeError: boom"}]])
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception), "script error: TypeError: boom")
        self.assertEqual(caught.exception.reason, FA.END_SCRIPT_ERROR)

    def test_detach_twice_detaches_once_and_reports_the_second(self):
        agent, session = self.started([STATE])
        self.assertEqual(agent.detach(), "detached")
        self.assertEqual(agent.detach(),
                         "already detached (detached by the caller)")
        self.assertEqual(session.detach_calls, 1)
        self.assertFalse(agent.alive)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "no session for the read: detached by the caller")

    def test_detach_after_the_game_closed_says_it_was_gone(self):
        agent, session = self.started([STATE])
        session.handlers["detached"]("process-terminated", None)
        self.assertEqual(agent.detach(),
                         "already gone: session detached: process-terminated")
        self.assertEqual(session.detach_calls, 1)

    def test_a_detach_that_raises_is_reported_and_the_log_closes(self):
        agent, session = self.started([STATE])

        def refuse():
            session.detach_calls += 1
            raise RuntimeError("session is gone")

        session.detach = refuse
        self.assertEqual(agent.detach(), "detached")
        self.assertEqual(self.lines, ["detach reported: session is gone"])
        session.script.handlers["message"](sent(MENU), None)
        self.assertEqual(self.written(), [sent({**STATE, "id": 1})])

    def test_read_before_start_says_nothing_is_attached(self):
        agent = self.live(FakeFrida(ReplySession([STATE])))
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(str(caught.exception),
                         "no session for the read: not attached")


# ---- the clock summary ------------------------------------------------------

class SummaryTest(unittest.TestCase):
    def test_summary_of_a_song_that_ends(self):
        result = FA.RunResult(reason=FA.END_DONE, cap_s=93.0, payloads=[
            sample(0.5), sample(30.0), sample(62.1, over=True,
                                               playing=False),
            {"done": "cap", "samples": 3}])
        self.assertEqual(FA.summarize_clock(result), [
            "ended: done",
            "samples: 3",
            "songTime: first 0.5, last 62.1",
            "songTime went backwards: 0 times",
            "isSongOver went true: yes",
        ])

    def test_summary_counts_a_clock_that_went_backwards(self):
        result = FA.RunResult(reason=FA.END_CAP, cap_s=93.0, payloads=[
            sample(10.0), sample(10.25), sample(-3.0), sample(-2.75),
            sample(5.0), sample(4.5)])
        lines = FA.summarize_clock(result)
        self.assertIn("songTime went backwards: 2 times", lines)

    def test_summary_with_an_error_and_no_samples(self):
        result = FA.RunResult(reason=FA.END_AGENT_ERROR, cap_s=93.0,
                              payloads=[{"error": "no live GameManager"}],
                              error="no live GameManager")
        self.assertEqual(FA.summarize_clock(result), [
            "ended: error",
            "samples: 0",
            "songTime: no samples",
            "songTime went backwards: 0 times",
            "isSongOver went true: no",
            "error: no live GameManager",
        ])

    def test_the_command_line_prints_the_summary(self):
        with tempfile.TemporaryDirectory() as tmp:
            agent = os.path.join(tmp, "clock.js")
            with open(agent, "w", encoding="utf-8") as fh:
                fh.write("// fake\n")
            out_path = os.path.join(tmp, "clock.jsonl")
            session = FakeSession([sent(sample(1.0)), sent(sample(2.0)),
                                   sent({"done": "cap", "samples": 2})])
            lines = []
            code = FA.main(["--agent", agent, "--cap", "93",
                            "--out", out_path],
                           frida_mod=FakeFrida(session), wait=instant_wait,
                           out=lines.append)
        self.assertEqual(code, 0)
        self.assertEqual(lines, [
            "ended: done",
            "samples: 2",
            "songTime: first 1.0, last 2.0",
            "songTime went backwards: 0 times",
            "isSongOver went true: no",
            f"messages written to {out_path}",
        ])
        self.assertEqual(session.detach_calls, 1)

    def test_the_command_line_ends_at_the_cap_with_no_error(self):
        # The runner's own detach after the cap must not turn the cap end
        # into a "session detached" error.
        with tempfile.TemporaryDirectory() as tmp:
            agent = os.path.join(tmp, "clock.js")
            with open(agent, "w", encoding="utf-8") as fh:
                fh.write("// fake\n")
            out_path = os.path.join(tmp, "clock.jsonl")
            session = FakeSession([sent(sample(1.0)), sent(sample(2.0))])
            lines = []
            code = FA.main(["--agent", agent, "--cap", "93",
                            "--out", out_path],
                           frida_mod=FakeFrida(session), wait=instant_wait,
                           out=lines.append)
        self.assertEqual(code, 0)
        self.assertEqual(lines, [
            "cap of 93 s reached before the agent finished; 2 messages seen",
            "ended: cap",
            "samples: 2",
            "songTime: first 1.0, last 2.0",
            "songTime went backwards: 0 times",
            "isSongOver went true: no",
            f"messages written to {out_path}",
        ])
        self.assertEqual(session.detach_calls, 1)

    def test_the_command_line_refuses_a_zero_cap(self):
        with self.assertRaises(SystemExit):
            with unittest.mock.patch("sys.stderr", io.StringIO()):
                FA.main(["--agent", "a.js", "--cap", "0", "--out", "o"],
                        frida_mod=FakeFrida(None), wait=instant_wait,
                        out=lambda s: None)


# ---- the agents ---------------------------------------------------------------

class NoWritesTest(unittest.TestCase):
    def test_the_check_catches_every_pattern(self):
        # Without this, a broken pattern would let every agent pass.
        bad = [
            "Memory.writeDouble(p, 1);",
            "p.writeU8(1);",
            "Memory.protect(p, 4096, 'rwx');",
            "gm.field('songTime').value = 0;",
            "gm.method('Stop').invoke();",
            "const f = new NativeFunction(p, 'void', []);",
            "klass.method('Hit').implementation = function () {};",
            "Interceptor.replace(p, cb);",
            "gm.handle.add(0x70).readDouble();",
            "Memory.copy(dst, src, 8);",
            "gm.field('count').value += 1;",
            "gm.field('count').value++;",
            "gm.field('flag').value ??= true;",
            "retval.replace(ptr(0));",
            "args[0] = ptr(0);",
            "args[slot] = ptr(0);",
            "this.context.rax = ptr(0);",
            "ctx.xmm0 = buf;",
        ]
        for line in bad:
            with self.subTest(line=line):
                self.assertTrue(no_write_findings(line))

    def test_reads_and_comparisons_pass(self):
        ok = [
            "const t = gm.field<number>('songTime').value;",
            "if (f.value === true) {}",
            "if (f.value == 1) {}",
            "Interceptor.attach(p, { onEnter() {} });",
            "log.write('x');",
            "if (f.value >= 1 || f.value <= 0 || f.value != 2) {}",
            "const a = args[0]; if (args[1] == null) {}",
            "const p = args[slot]; return args[slot] === null;",
            "const buf = xmmOf(ctx, slot); if (ctx.xmm0 == undefined) {}",
            "return (_c, a) => a[slot].toInt32();",
            "const r = retval.toInt32() + this.context.rax.toInt32();",
            "const b = Memory.alloc(8); const s = Memory.scan(p, 8, pat, cb);",
        ]
        for line in ok:
            with self.subTest(line=line):
                self.assertEqual(no_write_findings(line), [])

    def test_no_agent_writes_to_the_game(self):
        sources = agent_sources()
        self.assertIn(os.path.join(AGENT_DIR, "clock.ts"), sources)
        self.assertIn(os.path.join(AGENT_DIR, "state.ts"), sources)
        for path in sources:
            with self.subTest(agent=os.path.basename(path)):
                with open(path, encoding="utf-8") as fh:
                    self.assertEqual(no_write_findings(fh.read()), [])


class BuildScriptsTest(unittest.TestCase):
    def test_build_scripts_end_by_themselves(self):
        # A watch build never exits, so it would have no cap.
        with open(os.path.join(AGENT_DIR, "package.json"),
                  encoding="utf-8") as fh:
            scripts = json.load(fh)["scripts"]
        self.assertEqual(sorted(scripts),
                         ["build:clock", "build:state", "build:trace"])
        for name, command in scripts.items():
            with self.subTest(script=name):
                words = command.split()
                self.assertNotIn("-w", words)
                self.assertNotIn("--watch", words)


if __name__ == "__main__":
    unittest.main()
