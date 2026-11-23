"""Tests for frida_attach: the attach runner detaches on every path, the clock
summary is right, the Frida agents read fields by name and never by raw
offset, and every game method an agent calls is named in that agent's own
ALLOWED_CALLS list.

No game and no Frida: a stand-in frida module hands back a session whose
script delivers messages. The runner tests deliver messages real agents sent
from inside the game, copied under testdata/ch_runs (see `recorded`).
"""

from __future__ import annotations

import csv
import io
import json
import os
import re
import sys
import tempfile
import threading
import time
import unittest
import unittest.mock

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import build_stamp as BS  # noqa: E402
from tools.ch_mcp import frida_attach as FA  # noqa: E402
from tools.ch_mcp.tests.test_build_stamp import make_built_agent  # noqa: E402
from tools.ch_probe.constants import PROCESS_NAME  # noqa: E402

AGENT_DIR = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "agent")

# ---- the by-name check ----------------------------------------------------
# The one list of what an agent script may never contain, each with why.
# Writes and method calls are allowed (the user's rulings 2 and 5 of
# 2026-10-30, D272); reading by raw offset is not. test_trace_hits.py calls
# by_name_only_findings over trace.ts with this same list.
BY_NAME_ONLY_PATTERNS = (
    (r"add\(0x", "reads by offset, not by field name"),
)


def by_name_only_findings(text: str) -> list[tuple[int, str, str]]:
    """Every line of an agent script that matches BY_NAME_ONLY_PATTERNS, as
    (line number, line, why)."""
    found = []
    for n, line in enumerate(text.splitlines(), start=1):
        for pattern, why in BY_NAME_ONLY_PATTERNS:
            if re.search(pattern, line):
                found.append((n, line.strip(), why))
    return found


# ---- the allowed-calls check ----------------------------------------------
# Every game method an agent calls is named in that file's own ALLOWED_CALLS
# literal. A method is found by name with `.method(...)` or `.tryMethod(...)`,
# and each such lookup must name a literal from the file's list. Each
# `.invoke(`, and each `new NativeFunction` (built on a method's
# virtualAddress), must then be on such a lookup, so a call can reach only a
# method the file listed; `_traced_name` says what "on a lookup" covers. A
# file with no list may look up nothing, so it may call nothing. kit.ts calls
# LoadMap through a NativeFunction because .invoke cannot call it
# (docs/clone-hero/07-internals.md, IN-76). LISTED_AGENTS are the files
# allowed a list, and each one's list is pinned by a test below: menu.ts's to
# its names, kit.ts's to KIT_METHODS.
# The check reads source text, not a parsed program, so it has limits. It
# does not skip comments or strings: a lookup written in a comment counts as
# a source of its name (the variable's other sources must still be lookups),
# and an `.invoke(` in a comment is a finding. Functions are found by the
# `function` keyword only, so an arrow function is no scope of its own, and
# a name is judged over the whole enclosing function. It cannot see a
# function handed on by reference (`list.forEach(f)`, `g = f`) or a call made
# as `m.invoke.call(...)`. Only those two could hide a call, and no line in
# menu.ts or kit.ts does either today: every function that takes a method is
# called by name, and each real `.invoke(` and the NativeFunction trace to a
# literal lookup. A name set by destructuring or `for ... of` has no
# lookup source, so it is a finding, not a miss.
MENU_AGENT = "menu.ts"
KIT_AGENT = "kit.ts"
LISTED_AGENTS = (MENU_AGENT, KIT_AGENT)
KIT_METHODS = os.path.join(_REPO_ROOT, "tools", "ch_mcp", "kit_methods.json")
# A method looked up by name, with an optional type argument; group 2 is the
# name when it is a string literal.
_METHOD_LOOKUP = re.compile(r"\.(?:method|tryMethod)\s*(?:<[^>]*>)?\s*\(\s*(?:([\"'])(\w+)\1)?")
_INVOKE = re.compile(r"(?:\.\s*invoke|\[\s*[\"']invoke[\"']\s*\])\s*\(")
_NATIVE_FUNCTION = re.compile(r"new\s+NativeFunction\b")
_ALLOWED_LIST = re.compile(r"ALLOWED_CALLS\s*=\s*\[([^\]]*)\]")
# An expression that ends in a lookup by a literal name, with any
# `.overload(...)` after it. Before the lookup it may hold only names, dots,
# calls and type arguments, so no operator can pick some other value. Group 2
# is the looked-up name.
_LOOKUP_CHAIN = re.compile(
    r"\s*\w+(?:\s*\([^()]*\))?(?:\s*\.\s*\w+(?:\s*<[^<>]*>)?(?:\s*\([^()]*\))?)*?"
    r"\s*\.\s*(?:method|tryMethod)\s*(?:<[^<>]*>)?\s*\(\s*([\"'])(\w+)\1[^()]*\)"
    r"(?:\s*\.\s*overload\s*\([^()]*\))*\s*")
_FUNCTION = re.compile(r"\bfunction\s+(\w+)\s*(?:<[^()]*?>)?\s*\(")


def allowed_calls(agent_text: str) -> list[str]:
    """The names in one agent's ALLOWED_CALLS literal, or [] for none."""
    match = _ALLOWED_LIST.search(agent_text)
    if match is None:
        return []
    return re.findall(r"[\"'](\w+)[\"']", match.group(1))


def _close(text: str, start: int, open_ch: str, close_ch: str) -> int:
    """The index just past the bracket that closes the one at `start`, or
    len(text) when none does."""
    depth = 0
    for i in range(start, len(text)):
        if text[i] == open_ch:
            depth += 1
        elif text[i] == close_ch:
            depth -= 1
            if depth == 0:
                return i + 1
    return len(text)


def _enclosing_function(text: str, at: int):
    """The innermost `function` declaration whose body holds `at`, as (its
    name, its parameter text, the body's start, the body's end), or None at
    the top level."""
    best = None
    for m in _FUNCTION.finditer(text, 0, at):
        params_end = _close(text, m.end() - 1, "(", ")")
        body = text.find("{", params_end)
        if body == -1 or body > at:
            continue
        end = _close(text, body, "{", "}")
        if at < end:
            best = (m.group(1), text[m.end():params_end - 1], body, end)
    return best


def _statement_end(text: str, start: int) -> int:
    """The index of the `;` that ends the statement running from `start`,
    outside any bracket, or len(text)."""
    depth = 0
    for i in range(start, len(text)):
        c = text[i]
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
            if depth < 0:
                return i
        elif c == ";" and depth == 0:
            return i
    return len(text)


def _traced_name(text: str, at: int) -> str | None:
    """The literal name the value ending at `at` was looked up by, or None
    when it was not looked up by a literal name. The value counts as looked
    up when it is itself a lookup chain (`_LOOKUP_CHAIN`), or when it is a
    plain name and every source of that name is one: each place its own
    function sets it, and, when it is that function's first parameter, each
    call of the function in the file. A name with no source is not looked
    up."""
    head = text[max(0, at - 400):at]
    direct = re.search(_LOOKUP_CHAIN.pattern + r"$", head)
    if direct is not None:
        return direct.group(2)
    var = re.search(r"(?<![\w.$])(\w+)\s*$", head)
    if var is None:
        return None
    name = var.group(1)
    scope = _enclosing_function(text, at)
    lo, hi = (scope[2], scope[3]) if scope else (0, len(text))
    names = []
    for m in re.finditer(rf"(?<![\w.$]){name}\s*(?::[^=;()]*)?=(?![=>])",
                         text[lo:hi]):
        rhs_start = lo + m.end()
        chain = _LOOKUP_CHAIN.fullmatch(text[rhs_start:_statement_end(text, rhs_start)])
        if chain is None:
            return None
        names.append(chain.group(2))
    if scope is not None and re.match(rf"\s*{name}\b\s*[:,)]?", scope[1]):
        func = scope[0]
        for m in re.finditer(rf"(?<![\w.$]){func}\s*(?:<[^<>()]*>)?\s*\(", text):
            if text[:m.start()].rstrip().endswith("function"):
                continue
            arg = text[m.end():_close(text, m.end() - 1, "(", ")") - 1]
            chain = _LOOKUP_CHAIN.fullmatch(arg)
            if chain is None:
                return None
            names.append(chain.group(2))
    return names[0] if names else None


def call_findings(texts: dict[str, str]) -> list[str]:
    """Every call in `texts` (agent file name -> its text) that the file's
    own allowed list does not cover: a method looked up by a name outside
    that file's ALLOWED_CALLS or by a name that is not a literal, an
    `.invoke(` on anything but such a lookup, or a NativeFunction built on
    anything but such a lookup's virtualAddress."""
    found = []
    for name, text in sorted(texts.items()):
        allowed = set(allowed_calls(text))
        if _ALLOWED_LIST.search(text) and name not in LISTED_AGENTS:
            found.append(f"{name}: holds an ALLOWED_CALLS list, and only "
                         f"{', '.join(LISTED_AGENTS)} may")
        lines = text.splitlines()
        for n, line in enumerate(lines, start=1):
            for m in _METHOD_LOOKUP.finditer(line):
                if m.group(2) is None:
                    found.append(f"{name}:{n}: a method looked up by a name "
                                 f"that is not a literal: {line.strip()}")
                elif m.group(2) not in allowed:
                    found.append(f"{name}:{n}: {m.group(2)} is not in "
                                 f"{name}'s ALLOWED_CALLS")
        # A lookup by an unlisted name is already a finding above, so these
        # ask only whether each call is on a lookup by a literal name.
        for m in _INVOKE.finditer(text):
            if _traced_name(text, m.start()) is None:
                n = text.count("\n", 0, m.start()) + 1
                found.append(f"{name}:{n}: .invoke( on a method not looked up "
                             f"by a literal name: {lines[n - 1].strip()}")
        for m in _NATIVE_FUNCTION.finditer(text):
            n = text.count("\n", 0, m.start()) + 1
            address = re.match(r"\s*\(\s*(\w+)\s*\.\s*virtualAddress\s*,", text[m.end():])
            if address is None or _traced_name(
                    text, m.end() + address.start(1) + len(address.group(1))) is None:
                found.append(f"{name}:{n}: calls a game function: {lines[n - 1].strip()}")
    return found


def kit_methods_findings(kit_text: str, data: dict) -> list[str]:
    """Every difference between kit.ts's ALLOWED_CALLS and the method names
    in kit_methods.json (`data`), each way, plus any name either one lists
    twice."""
    listed = allowed_calls(kit_text)
    named = [m["name"] for m in data["methods"]]
    found = []
    for name in sorted(set(listed) - set(named)):
        found.append(f"{name} is in {KIT_AGENT}'s ALLOWED_CALLS but not in "
                     "kit_methods.json")
    for name in sorted(set(named) - set(listed)):
        found.append(f"{name} is in kit_methods.json but not in "
                     f"{KIT_AGENT}'s ALLOWED_CALLS")
    for what, names in ((f"{KIT_AGENT}'s ALLOWED_CALLS", listed),
                        ("kit_methods.json", named)):
        for name in sorted({n for n in names if names.count(n) > 1}):
            found.append(f"{name} is listed twice in {what}")
    return found


def load_kit_methods() -> dict:
    with open(KIT_METHODS, encoding="utf-8") as fh:
        return json.load(fh)


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


# Messages real Frida agents sent from inside Clone Hero, each file a few lines
# copied from one run's log under hydra-data/ch-mcp-runs; the test that reads
# a file names its source folder.
CH_RUNS = os.path.join(_REPO_ROOT, "testdata", "ch_runs")


def recorded(run: str, name: str) -> list[dict]:
    """The messages in testdata/ch_runs/<run>/<name>, one JSON line each, as
    the agent's log kept them."""
    with open(os.path.join(CH_RUNS, run, name), encoding="utf-8") as fh:
        return [json.loads(line) for line in fh]


def recorded_game_process() -> tuple[int, str]:
    """The game's pid and process name in a real run: the pid from the line
    ch_start replied with (hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/
    replies.txt, line 9), and the name the machine recorder's process list
    gives that pid (the same folder's machine-1/processes.csv)."""
    with open(os.path.join(CH_RUNS, "ls-t3-run-b", "replies.txt"),
              encoding="utf-8") as fh:
        pid = int(re.search(r"\bgame process id (\d+)\.", fh.read()).group(1))
    path = os.path.join(CH_RUNS, "ls-t3-run-b", "machine-1", "processes.csv")
    with open(path, encoding="utf-8", newline="") as fh:
        names = {row["name"] for row in csv.DictReader(fh)
                 if int(row["pid"]) == pid}
    if len(names) != 1:
        raise AssertionError(f"pid {pid} has names {names} in {path}")
    return pid, names.pop()


# The cap in frida_attach.py's usage line (`--cap 93`): a run that is meant to
# end on its own gets this long, and no test here waits on it.
RUN_CAP_S = 93.0
# A cap short enough that the real wait reaches it at once. frida_attach's
# command line takes any cap above 0 (`--cap must be more than 0 seconds`).
SHORT_CAP_S = 0.05


class Workspace(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.agent = os.path.join(self.tmp.name, "agent.js")
        with open(self.agent, "w", encoding="utf-8") as fh:
            fh.write("// fake agent\n")
        self.out_path = os.path.join(self.tmp.name, "out.jsonl")
        self.lines = []

    def run_agent(self, frida, cap_s=RUN_CAP_S, wait=instant_wait):
        return FA.run_agent(self.agent, cap_s, FA.ends_run, self.out_path,
                            frida_mod=frida, wait=wait,
                            out=self.lines.append)

    def written(self):
        with open(self.out_path, encoding="utf-8") as fh:
            return [json.loads(line) for line in fh]


# ---- the runner -----------------------------------------------------------

class DetachTest(Workspace):
    def test_detaches_after_a_normal_end(self):
        # A real trace agent run: one status message, then its done message
        # (hydra-data/ch-mcp-runs/2026-10-28/l3/trace-ew-count.jsonl).
        msgs = recorded("2026-10-28-l3", "trace-ew-count.jsonl")
        status, done = (m["payload"] for m in msgs)
        # The status message ends no run and the done message does, by the
        # agents' shared end signal.
        self.assertIsNone(FA.ends_run(status))
        self.assertEqual(FA.ends_run(done), FA.END_DONE)
        session = FakeSession(msgs)
        result = self.run_agent(FakeFrida(session), wait=FA._default_wait)
        self.assertEqual(result.reason, FA.END_DONE)
        self.assertIsNone(result.error)
        self.assertEqual(result.payloads, [status, done])
        self.assertEqual(session.detach_calls, 1)
        # The log keeps every message the agent sent, in order.
        self.assertEqual(self.written(), msgs)
        self.assertEqual(session.script.posted,
                         [{"type": "start", "cap_s": RUN_CAP_S}])
        with open(self.agent, encoding="utf-8") as fh:
            self.assertEqual(session.source, fh.read())
        self.assertEqual(self.lines, [])

    def test_detaches_after_the_cap(self):
        # One real clock sample (hydra-data/ch-mcp-runs/2026-10-28/l2/
        # clock.jsonl). A sample ends no run, so the real wait runs to the cap.
        first = recorded("2026-10-28-l2", "clock.jsonl")[:1]
        session = FakeSession(first)
        result = self.run_agent(FakeFrida(session), cap_s=SHORT_CAP_S,
                                wait=FA._default_wait)
        self.assertEqual(result.reason, FA.END_CAP)
        self.assertIsNone(result.error)
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.written(), first)
        # One line, naming the cap that passed and how many messages came.
        self.assertEqual(len(self.lines), 1)
        self.assertRegex(self.lines[0], rf"\bcap of {SHORT_CAP_S:g} s\b")
        self.assertRegex(self.lines[0], r"\b1 message seen\b")

    def test_attaches_to_the_game_by_process_name(self):
        # The name the running game had in a real run, read from that run's
        # own record rather than from the constant the runner uses; the
        # agent's messages are a real trace run's (hydra-data/ch-mcp-runs/
        # 2026-10-28/l3/trace-ew-count.jsonl).
        frida = FakeFrida(FakeSession(recorded("2026-10-28-l3", "trace-ew-count.jsonl")))
        self.run_agent(frida)
        self.assertEqual(frida.attached_to, [recorded_game_process()[1]])


class EndTest(Workspace):
    def test_an_agent_error_ends_the_run_and_is_kept(self):
        # A real kit agent's error reply (hydra-data/ch-mcp-runs/2026-10-31/
        # ls-d1a-run2/kit-1.jsonl), then a real clock sample (2026-10-28/l2/
        # clock.jsonl) that comes after the run has ended.
        error_msgs = recorded("2026-10-31-ls-d1a-run2", "kit-1.jsonl")
        late = recorded("2026-10-28-l2", "clock.jsonl")[:1]
        session = FakeSession(error_msgs + late)
        result = self.run_agent(FakeFrida(session), wait=FA._default_wait)
        self.assertEqual(result.reason, FA.END_AGENT_ERROR)
        # The agent's own error text is kept, and the later sample does not
        # replace it.
        self.assertEqual(result.error, error_msgs[0]["payload"]["error"])
        self.assertEqual(session.detach_calls, 1)

    def test_a_script_exception_ends_the_run(self):
        # A real script error: the kit agent's JavaScript threw inside the
        # game (hydra-data/ch-mcp-runs/2026-10-31/ls-d1a-run1/kit-1.jsonl).
        thrown = recorded("2026-10-31-ls-d1a-run1", "kit-1.jsonl")
        session = FakeSession(thrown)
        result = self.run_agent(FakeFrida(session), wait=FA._default_wait)
        self.assertEqual(result.reason, FA.END_SCRIPT_ERROR)
        self.assertEqual(result.error, thrown[0]["description"])
        self.assertEqual(result.payloads, [])
        self.assertEqual(session.detach_calls, 1)

    def test_no_message_is_written_after_the_run_ends(self):
        # A real trace run that ends on its done message (hydra-data/
        # ch-mcp-runs/2026-10-28/l3/trace-ew-count.jsonl), and a real clock
        # sample (2026-10-28/l2/clock.jsonl) that arrives after it.
        msgs = recorded("2026-10-28-l3", "trace-ew-count.jsonl")
        late = recorded("2026-10-28-l2", "clock.jsonl")[0]
        session = FakeSession(msgs)
        result = self.run_agent(FakeFrida(session), wait=FA._default_wait)
        session.script.handlers["message"](late, None)
        self.assertEqual(self.written(), msgs)
        self.assertEqual(result.payloads, [m["payload"] for m in msgs])


class DefaultWaitTest(unittest.TestCase):
    def wait_within(self, event, bound_s):
        """Run the real wait on `event` with the long run cap (RUN_CAP_S) in
        a thread, and give it `bound_s` to return. Fails at the bound rather
        than sitting out the cap, so a wait that ignores the event fails in
        `bound_s`, not in RUN_CAP_S. Returns what the wait returned."""
        result = []
        waiter = threading.Thread(
            target=lambda: result.append(FA._default_wait(event, RUN_CAP_S)),
            daemon=True)
        waiter.start()
        waiter.join(bound_s)
        self.assertFalse(waiter.is_alive(),
                         f"the wait was still running {bound_s:g} s in, "
                         f"with a cap of {RUN_CAP_S:g} s")
        return result[0]

    def test_returns_true_at_once_when_the_event_is_set(self):
        # "At once": before the wait's first slice (_INTERRUPT_SLICE_S) ends.
        event = threading.Event()
        event.set()
        self.assertTrue(self.wait_within(event, FA._INTERRUPT_SLICE_S))

    def test_returns_false_when_the_cap_passes(self):
        self.assertFalse(FA._default_wait(threading.Event(), 0.05))

    def test_wakes_when_the_event_is_set_during_the_wait(self):
        # The event is set SHORT_CAP_S into the wait; the wait returns within
        # one slice of that, far inside the run cap it was given.
        event = threading.Event()
        threading.Timer(SHORT_CAP_S, event.set).start()
        self.assertTrue(self.wait_within(event, SHORT_CAP_S + FA._INTERRUPT_SLICE_S))


class AgentRefusalTest(Workspace):
    def test_the_command_line_refuses_a_stale_or_missing_agent_before_attaching(self):
        folder = os.path.join(self.tmp.name, "built")
        agent = make_built_agent(folder, "clock")
        with open(os.path.join(folder, "clock.ts"), "a", encoding="utf-8") as fh:
            fh.write("// an edit after the build\n")
        # Anything that reaches for Frida fails the test, so a refusal that
        # comes after the attach cannot pass.
        no_frida = object()
        stale = BS.check(agent)
        self.assertIsNotNone(stale)
        code = FA.main(["--agent", agent, "--cap", "93", "--out", self.out_path],
                       frida_mod=no_frida, out=self.lines.append)
        self.assertEqual(code, 1)
        # build_stamp owns the wording; the line names the source that changed.
        self.assertEqual(self.lines, [stale])
        self.assertIn("clock.ts", self.lines[0])
        os.remove(agent)
        self.lines.clear()
        missing = BS.check(agent)
        self.assertIsNotNone(missing)
        self.assertNotEqual(missing, stale)
        code = FA.main(["--agent", agent, "--cap", "93", "--out", self.out_path],
                       frida_mod=no_frida, out=self.lines.append)
        self.assertEqual(code, 1)
        self.assertEqual(self.lines, [missing])
        self.assertIn(agent, self.lines[0])
        self.assertFalse(os.path.exists(self.out_path))


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
STATE = {"found": True, "instances": 1, "destroyed": 0, "songTime": 3.5, "isPaused": False,
         "isSongOver": False, "isSongPlaying": True, "songLoaded": True}
MENU = {"found": False, "reason": "no live GameManager", "destroyed": 0}
# A read that found no live GameManager and skipped one destroyed one.
MENU_ONE_DESTROYED = {**MENU, "destroyed": 1}


def settle(agent, cap_s=5.0):
    """Wait until the agent's log writer has taken every queued line. The
    writer runs on its own thread, so a test reads the log only after this
    (or after detach, which waits for it)."""
    lines = agent._log_queue
    if lines is None:
        return
    deadline = time.monotonic() + cap_s
    while lines.unfinished_tasks:
        if time.monotonic() > deadline:
            raise AssertionError(f"the log writer still had lines after {cap_s} s")
        time.sleep(0.005)


class LiveAgentTest(Workspace):
    def setUp(self):
        super().setUp()
        self.agents = []

    def live(self, frida, wait=instant_wait):
        agent = FA.LiveAgent(self.agent, log_path=self.out_path,
                             frida_mod=frida, wait=wait, out=self.lines.append)
        # Runs before the temp folder's cleanup, so Windows can delete the
        # log; detach is safe to call again.
        self.addCleanup(agent.detach)
        self.agents.append(agent)
        return agent

    def written(self):
        for agent in self.agents:
            settle(agent)
        return super().written()

    def raw_lines(self):
        """The log's lines as text, read through a second handle."""
        with open(self.out_path, encoding="utf-8") as fh:
            return fh.readlines()

    def test_start_posts_a_read_and_returns_its_reply(self):
        # The state agent's real answer to the first read of a run, carrying
        # id 1 (hydra-data/ch-mcp-runs/2026-11-06/ls-d2b-run1/state-4.jsonl).
        reply = recorded("2026-11-06-ls-d2b-run1", "state-4.jsonl")
        session = ReplySession([reply])
        frida = FakeFrida(session)
        agent = self.live(frida, wait=FA._default_wait)
        state = agent.start(30.0)
        self.assertEqual(frida.attached_to, [PROCESS_NAME])
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1}])
        # The caller gets the agent's payload without the id the read gave it.
        self.assertNotIn("id", state)
        self.assertEqual({**state, "id": 1}, reply[0]["payload"])
        with open(self.agent, encoding="utf-8") as fh:
            self.assertEqual(session.source, fh.read())
        self.assertTrue(agent.alive)
        self.assertEqual(session.detach_calls, 0)
        # The log keeps the message as the agent sent it, id and all.
        self.assertEqual(self.written(), reply)

    def test_a_payload_with_no_id_goes_to_the_listener_and_the_log(self):
        # The state agent's real first reply (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d2b-run1/state-4.jsonl) and one real play_events
        # message from the kit agent, which carries no id (2026-11-12/
        # ls-g3-run1/kit-1.jsonl).
        reply = recorded("2026-11-06-ls-d2b-run1", "state-4.jsonl")
        event_msg = recorded("2026-11-12-ls-g3-run1", "kit-1.jsonl")[0]
        self.assertNotIn("id", event_msg["payload"])
        session = ReplySession([reply])
        agent = self.live(FakeFrida(session), wait=FA._default_wait)
        agent.start(30.0)
        heard = []
        agent.listen(heard.append)
        session.script.handlers["message"](event_msg, None)
        # A message with an id is a reply, never the listener's, even when
        # no read is waiting for it.
        session.script.handlers["message"](reply[0], None)
        self.assertEqual(heard, [event_msg["payload"]])
        agent.listen(None)
        session.script.handlers["message"](event_msg, None)
        self.assertEqual(heard, [event_msg["payload"]])
        # Both copies of the event reach the log, heard or not.
        self.assertEqual(self.written().count(event_msg), 2)

    def test_binary_data_reaches_the_listener_beside_its_payload_but_not_the_log(self):
        # A frame_clock message as kit.ts's sendFrameClock sends it: a
        # payload with no id and binary data (send(message, data)).
        reply = recorded("2026-11-06-ls-d2b-run1", "state-4.jsonl")
        session = ReplySession([reply])
        agent = self.live(FakeFrida(session), wait=FA._default_wait)
        agent.start(30.0)
        heard = []
        agent.listen(heard.append)
        payload = {"type": "frame_clock", "count": 1, "layout": "x"}
        message = {"type": "send", "payload": payload}
        data = bytes(range(12))
        session.script.handlers["message"](message, data)
        self.assertEqual(heard, [{**payload, FA.PAYLOAD_DATA: data}])
        # The payload the agent sent is left as it was.
        self.assertNotIn(FA.PAYLOAD_DATA, payload)
        self.assertEqual(self.written()[-1], message)

    def real_started(self, run, name):
        """A LiveAgent started, with the real wait, on a recorded log's first
        message, delivered as the agent sent it. Returns the agent, its
        session and every message in the log, for the test to deliver."""
        msgs = recorded(run, name)
        session = ReplySession([msgs[:1]])
        agent = self.live(FakeFrida(session), wait=FA._default_wait)
        agent.start(30.0)
        return agent, session, msgs

    def test_start_with_no_first_reply_raises_on_the_cap_and_detaches_once(self):
        # The agent sends nothing, so the real wait runs to the cap.
        session = ReplySession([None])
        agent = self.live(FakeFrida(session), wait=FA._default_wait)
        with self.assertRaises(FA.AttachFailed) as caught:
            agent.start(SHORT_CAP_S)
        self.assertEqual(caught.exception.reason, FA.END_CAP)
        # The line names what got no reply, the cap that passed, and that
        # nothing stays attached.
        text = str(caught.exception)
        self.assertRegex(text, r"\bfirst read\b")
        self.assertRegex(text, rf"\bcap of {SHORT_CAP_S:g} s\b")
        self.assertRegex(text, r"\bdetached$")
        self.assertEqual(session.detach_calls, 1)
        self.assertFalse(agent.alive)
        self.assertEqual(agent.gone_reason, FA._DETACHED_BY_CALLER)
        # A second detach detaches nothing and names why the first happened.
        self.assertIn(FA._DETACHED_BY_CALLER, agent.detach())
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(self.written(), [])

    def test_read_returns_the_next_payload_and_the_log_holds_it(self):
        # Three state reads in a row from one real run, ids 1 to 3
        # (hydra-data/ch-mcp-runs/2026-11-06/ls-d1b-run1/state-1.jsonl).
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        second = msgs[1]["payload"]
        session.replies.append([msgs[1]])
        state = agent.read(5.0)
        # The read takes the next id, and returns that reply without its id.
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1},
                                                 {"type": "read", "id": 2}])
        self.assertEqual(second["id"], 2)
        self.assertNotIn("id", state)
        self.assertEqual({**state, "id": 2}, second)
        self.assertNotEqual(state["songTime"], msgs[0]["payload"]["songTime"])
        self.assertEqual(self.written(), msgs[:2])

    def test_read_with_no_reply_names_the_cap(self):
        # The state agent's real first reply (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d2b-run1/state-4.jsonl); the next read gets nothing,
        # so the real wait runs to the cap.
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        session.replies.append(None)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(SHORT_CAP_S)
        self.assertEqual(caught.exception.reason, FA.END_CAP)
        text = str(caught.exception)
        self.assertRegex(text, r"\bthe read\b")
        self.assertNotRegex(text, r"\bfirst read\b")
        self.assertRegex(text, rf"\bcap of {SHORT_CAP_S:g} s$")
        # A read that got no reply leaves the session up.
        self.assertTrue(agent.alive)

    def test_a_late_reply_is_not_taken_by_the_next_read(self):
        # Real state replies with ids 1 to 3 (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d1b-run1/state-1.jsonl). Read 2 gets nothing in time;
        # its reply then arrives just before read 3's.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        stale, fresh = msgs[1], msgs[2]
        session.replies += [None, [stale, fresh]]
        with self.assertRaises(FA.ReadFailed):
            agent.read(SHORT_CAP_S)
        state = agent.read(5.0)
        self.assertEqual(session.script.posted[-1], {"type": "read", "id": 3})
        self.assertEqual({**state, "id": 3}, fresh["payload"])
        self.assertNotEqual(state["songTime"], stale["payload"]["songTime"])
        # Both still reach the log, in the order they came.
        self.assertEqual(self.written(), msgs)

    def test_request_posts_any_message_with_an_id(self):
        # A real kit agent's status reply (id 1) and its setup reply (id 2)
        # (hydra-data/ch-mcp-runs/2026-11-06/ls-d2b-run1/kit-2.jsonl).
        agent, session, msgs = self.real_started("2026-11-06-ls-d2b-run1",
                                                 "kit-2.jsonl")
        setup = msgs[1]["payload"]
        session.replies.append([msgs[1]])
        message = {"type": "setup", "tag": "hydra-kit"}
        reply = agent.request(message, 10.0)
        # The request goes out whole, with the next id added and nothing else.
        self.assertEqual(session.script.posted[-1], {**message, "id": 2})
        self.assertEqual(setup["id"], 2)
        self.assertNotIn("id", reply)
        self.assertEqual({**reply, "id": 2}, setup)
        self.assertEqual(self.written(), msgs)

    def test_a_request_with_no_reply_names_its_type_and_the_cap(self):
        # The state agent's real first reply (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d2b-run1/state-4.jsonl); the request gets nothing,
        # so the real wait runs to the cap.
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        session.replies.append(None)
        message = {"type": "press"}
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.request(message, SHORT_CAP_S)
        self.assertEqual(caught.exception.reason, FA.END_CAP)
        text = str(caught.exception)
        self.assertRegex(text, rf"\b{message['type']} request\b")
        self.assertRegex(text, rf"\bcap of {SHORT_CAP_S:g} s$")

    def test_a_request_that_brings_its_own_id_or_no_type_is_refused(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d2b-run1/state-4.jsonl).
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        for bad in ({"type": "setup", "id": 7}, {"tag": "hydra-kit"},
                    {"type": 3}, ["setup"]):
            with self.subTest(message=bad):
                with self.assertRaises(ValueError):
                    agent.request(bad, 1.0)
        # Nothing went out past start's own read, and the session is still up.
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1}])
        self.assertTrue(agent.alive)

    def test_an_agent_error_fails_the_read(self):
        # A real kit agent's status reply (id 1), then its error reply to the
        # next request (id 2) (hydra-data/ch-mcp-runs/2026-10-31/ls-d1a-run2/
        # kit-1.jsonl).
        agent, session, msgs = self.real_started("2026-10-31-ls-d1a-run2",
                                                 "kit-1-from-start.jsonl")
        error = msgs[1]["payload"]
        session.replies.append([msgs[1]])
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(caught.exception.reason, FA.END_AGENT_ERROR)
        # The line carries the agent's own error text.
        self.assertIn(error["error"], str(caught.exception))
        # An error ends the read, not the session.
        self.assertTrue(agent.alive)
        self.assertEqual(session.detach_calls, 0)

    def test_an_agent_error_carries_its_whole_reply(self):
        # The same real kit run: its error reply carries a fault record
        # beside the error text (hydra-data/ch-mcp-runs/2026-10-31/
        # ls-d1a-run2/kit-1.jsonl).
        agent, session, msgs = self.real_started("2026-10-31-ls-d1a-run2",
                                                 "kit-1-from-start.jsonl")
        error = msgs[1]["payload"]
        self.assertGreater(set(error) - {"id", "error"}, set())
        session.replies.append([msgs[1]])
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        # The caller gets every key the agent sent, all but the read's id.
        reply = caught.exception.reply
        self.assertNotIn("id", reply)
        self.assertEqual({**reply, "id": error["id"]}, error)

    def test_a_failure_with_no_reply_carries_none(self):
        # A read that got nothing in time, then one that ended on a real
        # script error (hydra-data/ch-mcp-runs/2026-10-31/ls-d1a-run1/
        # kit-1.jsonl): neither has an agent reply to carry.
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        thrown = recorded("2026-10-31-ls-d1a-run1", "kit-1.jsonl")
        session.replies += [None, thrown]
        for cap_s, reason in ((SHORT_CAP_S, FA.END_CAP),
                              (5.0, FA.END_SCRIPT_ERROR)):
            with self.subTest(reason=reason):
                with self.assertRaises(FA.ReadFailed) as caught:
                    agent.read(cap_s)
                self.assertEqual(caught.exception.reason, reason)
                self.assertIsNone(caught.exception.reply)

    def test_a_script_exception_fails_the_read(self):
        # A real kit agent's first reply (hydra-data/ch-mcp-runs/2026-10-31/
        # ls-d1a-run2/kit-1.jsonl), then the error the same agent's
        # JavaScript threw inside the game in the run before
        # (2026-10-31/ls-d1a-run1/kit-1.jsonl).
        agent, session, _ = self.real_started("2026-10-31-ls-d1a-run2",
                                              "kit-1-from-start.jsonl")
        thrown = recorded("2026-10-31-ls-d1a-run1", "kit-1.jsonl")
        session.replies.append(thrown)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(caught.exception.reason, FA.END_SCRIPT_ERROR)
        # The line says it was a script error and carries the game's text.
        text = str(caught.exception)
        self.assertTrue(text.startswith(FA.END_SCRIPT_ERROR), text)
        self.assertIn(thrown[0]["description"], text)
        # A script error ends the read, not the session.
        self.assertTrue(agent.alive)
        self.assertEqual(session.detach_calls, 0)

    def test_detach_twice_detaches_once_and_reports_the_second(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d2b-run1/state-4.jsonl).
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        first = agent.detach()
        second = agent.detach()
        self.assertNotEqual(first, second)
        self.assertNotIn("already", first)
        # The second detach says nothing was left and names why the first
        # one happened.
        self.assertRegex(second, r"^already detached\b")
        self.assertIn(FA._DETACHED_BY_CALLER, second)
        self.assertEqual(session.detach_calls, 1)
        self.assertFalse(agent.alive)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(caught.exception.reason, FA.END_DETACHED)
        text = str(caught.exception)
        self.assertRegex(text, r"\bthe read\b")
        self.assertIn(FA._DETACHED_BY_CALLER, text)
        # The read posted nothing past start's own.
        self.assertEqual(session.script.posted, [{"type": "read", "id": 1}])

    def test_detach_after_the_game_closed_says_it_was_gone(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d2b-run1/state-4.jsonl). Frida then
        # reports the drop with its reason for a game that closed,
        # "process-terminated" (one of Frida's session detach reasons).
        agent, session, _ = self.real_started("2026-11-06-ls-d2b-run1",
                                              "state-4.jsonl")
        session.handlers["detached"]("process-terminated", None)
        self.assertFalse(agent.alive)
        dropped = FA._detached_text("process-terminated")
        self.assertEqual(agent.gone_reason, dropped)
        line = agent.detach()
        self.assertRegex(line, r"^already gone\b")
        self.assertIn(dropped, line)
        self.assertNotIn(FA._DETACHED_BY_CALLER, line)
        # The caller's detach still asks Frida once, and the reason stays the
        # game's.
        self.assertEqual(session.detach_calls, 1)
        self.assertEqual(agent.gone_reason, dropped)

    def test_a_detach_that_raises_is_reported_and_the_log_closes(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d1b-run1/state-1.jsonl); its second
        # reply then arrives after the detach.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        # What Frida raises when a session the game dropped is detached.
        refusal = RuntimeError("session is gone")

        def refuse():
            session.detach_calls += 1
            raise refusal

        session.detach = refuse
        line = agent.detach()
        self.assertNotIn("already", line)
        self.assertFalse(agent.alive)
        # One line says the detach reported a fault, with Frida's own text.
        self.assertEqual(len(self.lines), 1)
        self.assertRegex(self.lines[0], r"^detach reported\b")
        self.assertIn(str(refusal), self.lines[0])
        # The log closed anyway: the later reply is not written.
        session.script.handlers["message"](msgs[1], None)
        self.assertEqual(self.written(), msgs[:1])

    def real_game_pid(self):
        """The pid Clone Hero had in a real run, from the machine recorder's
        process list (hydra-data/ch-mcp-runs/2026-11-07/ls-t3-run-b/
        machine-1/processes.csv)."""
        path = os.path.join(CH_RUNS, "ls-t3-run-b", "machine-1", "processes.csv")
        with open(path, encoding="utf-8", newline="") as fh:
            pids = {int(row["pid"]) for row in csv.DictReader(fh)
                    if row["name"] == PROCESS_NAME}
        self.assertEqual(len(pids), 1, pids)
        return pids.pop()

    def test_a_pid_attaches_to_that_process(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d2b-run1/state-4.jsonl).
        pid = self.real_game_pid()
        reply = recorded("2026-11-06-ls-d2b-run1", "state-4.jsonl")
        session = ReplySession([reply])
        frida = FakeFrida(session)
        agent = FA.LiveAgent(self.agent, log_path=self.out_path, pid=pid,
                             frida_mod=frida, wait=FA._default_wait,
                             out=self.lines.append)
        self.addCleanup(agent.detach)
        state = agent.start(30.0)
        # The attach goes to that process, not to the game by its name.
        self.assertEqual(len(frida.attached_to), 1)
        self.assertNotEqual(frida.attached_to[0], PROCESS_NAME)
        self.assertEqual(frida.attached_to[0], pid)
        self.assertTrue(agent.alive)
        self.assertEqual({**state, "id": 1}, reply[0]["payload"])

    def test_a_pid_that_is_not_running_names_the_pid(self):
        # The pid the game had in a real run, now long gone, so Frida finds
        # no such process.
        pid = self.real_game_pid()
        failures = {}
        for target in (pid, None):
            frida = FakeFrida(None)
            agent = FA.LiveAgent(self.agent, log_path=self.out_path, pid=target,
                                 frida_mod=frida, wait=FA._default_wait,
                                 out=self.lines.append)
            with self.assertRaises(FA.AttachFailed) as caught:
                agent.start(30.0)
            self.assertEqual(caught.exception.reason, FA.END_NOT_RUNNING)
            self.assertEqual(frida.attached_to,
                             [PROCESS_NAME if target is None else pid])
            failures[target] = str(caught.exception)
        # The line with a pid is the line without one, with the pid named
        # where the game's name stood.
        by_name = failures[None]
        self.assertTrue(by_name.startswith(PROCESS_NAME), by_name)
        rest = by_name[len(PROCESS_NAME):]
        by_pid = failures[pid]
        self.assertTrue(by_pid.endswith(rest), by_pid)
        self.assertRegex(by_pid[:-len(rest)], rf"\b{pid}$")
        self.assertNotIn(PROCESS_NAME, by_pid)
        # Nothing attached, so no log was opened.
        self.assertFalse(os.path.exists(self.out_path))

    def blocked_writes(self, agent):
        """Make every later write to the agent's log wait until the returned
        event is set. The cleanup sets it, before the detach that `live`
        registered, so a failed test still lets the writer finish."""
        release = threading.Event()
        log = agent._log
        real_write = log.write

        def blocked_write(text):
            release.wait(10.0)
            return real_write(text)

        log.write = blocked_write
        self.addCleanup(release.set)
        return release

    def test_a_blocked_log_write_does_not_hold_up_the_reply(self):
        # Two real state replies, ids 1 and 2 (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d1b-run1/state-1.jsonl). The second answers a read
        # made while every log write is held.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        settle(agent)
        release = self.blocked_writes(agent)
        session.replies.append([msgs[1]])
        replies = []
        asker = threading.Thread(target=lambda: replies.append(agent.read(5.0)),
                                 daemon=True)
        asker.start()
        asker.join(2.0)
        answered = not asker.is_alive()
        # The log still waits on the held write, so the reply did not.
        self.assertEqual(len(self.raw_lines()), 1)
        release.set()
        asker.join(10.0)
        self.assertTrue(answered, "the reply waited for the blocked log write")
        self.assertEqual(len(replies), 1)
        self.assertEqual({**replies[0], "id": msgs[1]["payload"]["id"]},
                         msgs[1]["payload"])
        settle(agent)
        self.assertEqual([json.loads(line) for line in self.raw_lines()], msgs[:2])

    def test_every_message_reaches_the_log_in_order_once(self):
        # Started on the state agent's real first reply (hydra-data/
        # ch-mcp-runs/2026-11-06/ls-d1b-run1/state-1.jsonl), then the first
        # 200 real clock samples of a run (2026-10-28/l2/clock.jsonl, lines
        # 1 to 200), sent one after another while the writer runs.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        samples = recorded("2026-10-28-l2", "clock-first-200.jsonl")
        for msg in samples:
            session.script.handlers["message"](msg, None)
        self.assertEqual(agent.detach(), "detached")
        # One line per message: the start's reply, then each sample once, in
        # the order sent.
        lines = self.raw_lines()
        self.assertEqual(len(lines), 1 + len(samples))
        self.assertEqual([json.loads(line) for line in lines], msgs[:1] + samples)

    def test_every_line_is_on_disk_before_detach(self):
        # Three real state replies (hydra-data/ch-mcp-runs/2026-11-06/
        # ls-d1b-run1/state-1.jsonl): the first answers the start, the other
        # two arrive while the agent stays attached.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        for msg in msgs[1:]:
            session.script.handlers["message"](msg, None)
        settle(agent)
        self.assertTrue(agent.alive)
        # A second handle reads every message from disk, in arrival order,
        # with no detach yet.
        self.assertEqual([json.loads(line) for line in self.raw_lines()], msgs)

    def test_detach_waits_for_a_slow_writer(self):
        # Three real state replies (hydra-data/ch-mcp-runs/2026-11-06/
        # ls-d1b-run1/state-1.jsonl): the first answers the start, the other
        # two arrive while every write is slow. All the slow writes together
        # take less than the writer's cap, so the detach waits for them all.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        slow_s = SHORT_CAP_S
        self.assertLess(len(msgs) * slow_s, FA.LOG_WRITER_CAP_S)
        log = agent._log
        real_write = log.write

        def slow_write(text):
            time.sleep(slow_s)
            return real_write(text)

        log.write = slow_write
        for msg in msgs[1:]:
            session.script.handlers["message"](msg, None)
        self.assertEqual(agent.detach(), "detached")
        self.assertEqual([json.loads(line) for line in self.raw_lines()], msgs)

    def test_detach_says_so_when_the_writer_is_held_past_the_cap(self):
        # Two real state replies (hydra-data/ch-mcp-runs/2026-11-06/
        # ls-d1b-run1/state-1.jsonl). The second is sent while every write
        # is held, and the writer's cap is cut to SHORT_CAP_S.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        settle(agent)
        release = self.blocked_writes(agent)
        session.script.handlers["message"](msgs[1], None)
        with unittest.mock.patch.object(FA, "LOG_WRITER_CAP_S", SHORT_CAP_S):
            line = agent.detach()
        # The detach returns at the cap, with a line saying the log may lack
        # its last lines and naming the cap that passed.
        self.assertRegex(line, r"^detached; ")
        self.assertIn("the log may lack its last lines", line)
        self.assertRegex(line, rf"\b{SHORT_CAP_S:g} s after the detach$")
        # Once released, the writer still writes the held line and ends.
        release.set()
        agent._writer.join(10.0)
        self.assertFalse(agent._writer.is_alive())
        self.assertEqual([json.loads(l) for l in self.raw_lines()], msgs[:2])

    def test_a_failed_log_write_stops_the_log_and_not_the_reply(self):
        # Three real state replies, ids 1 to 3 (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d1b-run1/state-1.jsonl). After the first, the log's
        # file is closed under the writer, so its next write really fails.
        agent, session, msgs = self.real_started("2026-11-06-ls-d1b-run1",
                                                 "state-1.jsonl")
        settle(agent)
        agent._log.close()
        try:
            agent._log.write("")
        except ValueError as e:
            failure = str(e)
        else:
            self.fail("a closed file took a write")
        session.replies += [[msgs[1]], [msgs[2]]]
        for msg in msgs[1:]:
            state = agent.read(5.0)
            self.assertEqual({**state, "id": msg["payload"]["id"]}, msg["payload"])
        self.assertTrue(agent.alive)
        detached = agent.detach()
        self.assertNotIn("may lack", detached)
        # One line, once: it names the log, the file's own error and that
        # the log stops while the requests go on.
        self.assertEqual(len(self.lines), 1, self.lines)
        line = self.lines[0]
        self.assertIn(self.out_path, line)
        self.assertIn(failure, line)
        self.assertRegex(line, r"\bthe log stops\b")
        self.assertRegex(line, r"\brequests go on$")
        self.assertEqual(self.written(), msgs[:1])

    def test_read_before_start_says_nothing_is_attached(self):
        # The state agent's real first reply (hydra-data/ch-mcp-runs/
        # 2026-11-06/ls-d2b-run1/state-4.jsonl) waits for a start that
        # never comes.
        session = ReplySession([recorded("2026-11-06-ls-d2b-run1", "state-4.jsonl")])
        frida = FakeFrida(session)
        agent = self.live(frida, wait=FA._default_wait)
        with self.assertRaises(FA.ReadFailed) as caught:
            agent.read(5.0)
        self.assertEqual(caught.exception.reason, FA.END_DETACHED)
        text = str(caught.exception)
        self.assertRegex(text, r"\bthe read\b")
        self.assertRegex(text, r"\bnot attached$")
        self.assertNotIn(FA._DETACHED_BY_CALLER, text)
        # The read reached for nothing: no attach and no script.
        self.assertEqual(frida.attached_to, [])
        self.assertIsNone(session.script)
        self.assertFalse(agent.alive)


# ---- the clock summary ------------------------------------------------------

def song_over_window() -> list[dict]:
    """Seven real clock samples around the end of a song, the last three
    after isSongOver turned true (hydra-data/ch-mcp-runs/2026-10-28/l2/
    clock.jsonl, lines 238 to 244)."""
    return recorded("2026-10-28-l2", "clock-song-over.jsonl")


class SummaryTest(Workspace):
    def summary_checks(self, lines, payloads, reason):
        """The summary lines against what the payloads themselves hold: the
        end reason, how many samples, the first and last songTime, how often
        it went back, and whether isSongOver turned true."""
        times = [p["songTime"] for p in payloads]
        back = sum(1 for a, b in zip(times, times[1:]) if b < a)
        over = any(p["isSongOver"] for p in payloads)
        self.assertRegex(lines[0], rf"^ended: {re.escape(reason)}$")
        self.assertRegex(lines[1], rf"^samples: {len(payloads)}$")
        self.assertRegex(lines[2], rf"\bfirst {re.escape(str(times[0]))}\b")
        self.assertRegex(lines[2], rf"\blast {re.escape(str(times[-1]))}$")
        self.assertRegex(lines[3], rf"\bbackwards: {back} times$")
        self.assertRegex(lines[4], rf"\bisSongOver went true: {'yes' if over else 'no'}$")

    def test_summary_of_a_song_that_ends(self):
        # The real samples, as run_agent collects them; samples end no run,
        # so the real wait runs to a short cap.
        msgs = song_over_window()
        payloads = [m["payload"] for m in msgs]
        # The window holds the song's end and no step back.
        self.assertFalse(payloads[0]["isSongOver"])
        self.assertTrue(payloads[-1]["isSongOver"])
        result = self.run_agent(FakeFrida(FakeSession(msgs)), cap_s=SHORT_CAP_S,
                                wait=FA._default_wait)
        self.assertEqual(result.payloads, payloads)
        lines = FA.summarize_clock(result)
        self.assertEqual(len(lines), 5, lines)
        self.summary_checks(lines, payloads, FA.END_CAP)
        self.assertRegex(lines[4], r"\byes$")

    def test_summary_with_an_error_and_no_samples(self):
        # A real agent error reply, the run's only message (hydra-data/
        # ch-mcp-runs/2026-10-31/ls-d1a-run2/kit-1.jsonl). It carries no
        # songTime, so the run has no samples.
        msgs = recorded("2026-10-31-ls-d1a-run2", "kit-1.jsonl")
        self.assertNotIn("songTime", msgs[0]["payload"])
        result = self.run_agent(FakeFrida(FakeSession(msgs)), wait=FA._default_wait)
        self.assertEqual(result.reason, FA.END_AGENT_ERROR)
        lines = FA.summarize_clock(result)
        self.assertEqual(len(lines), 6, lines)
        self.assertRegex(lines[0], rf"^ended: {re.escape(FA.END_AGENT_ERROR)}$")
        self.assertRegex(lines[1], r"^samples: 0$")
        self.assertRegex(lines[2], r"\bno samples$")
        self.assertRegex(lines[3], r"\bbackwards: 0 times$")
        self.assertRegex(lines[4], r"\bwent true: no$")
        # The last line is the agent's own error text.
        self.assertRegex(lines[5], r"^error: ")
        self.assertTrue(lines[5].endswith(msgs[0]["payload"]["error"]), lines[5])

    def test_the_command_line_prints_the_summary(self):
        # The real samples around a song's end (song_over_window), through
        # the command line with the real wait and a short cap.
        msgs = song_over_window()
        payloads = [m["payload"] for m in msgs]
        agent = make_built_agent(os.path.join(self.tmp.name, "built"), "clock")
        session = FakeSession(msgs)
        code = FA.main(["--agent", agent, "--cap", f"{SHORT_CAP_S:g}",
                        "--out", self.out_path],
                       frida_mod=FakeFrida(session), wait=FA._default_wait,
                       out=self.lines.append)
        self.assertEqual(code, 0)
        # The runner's cap line, then the summary, then where the messages
        # went; the file holds every one of them.
        self.assertEqual(len(self.lines), 7, self.lines)
        self.summary_checks(self.lines[1:6], payloads, FA.END_CAP)
        self.assertRegex(self.lines[6], rf"\bwritten to {re.escape(self.out_path)}$")
        self.assertEqual(self.written(), msgs)
        self.assertEqual(session.detach_calls, 1)

    def test_the_command_line_ends_at_the_cap_with_no_error(self):
        # The runner's own detach after the cap must not turn the cap end
        # into a "session detached" error. Two real clock samples (hydra-data/
        # ch-mcp-runs/2026-10-28/l2/clock.jsonl), and the real wait runs to
        # a short cap.
        msgs = recorded("2026-10-28-l2", "clock.jsonl")
        agent = make_built_agent(os.path.join(self.tmp.name, "built"), "clock")
        session = FakeSession(msgs)
        code = FA.main(["--agent", agent, "--cap", f"{SHORT_CAP_S:g}",
                        "--out", self.out_path],
                       frida_mod=FakeFrida(session), wait=FA._default_wait,
                       out=self.lines.append)
        self.assertEqual(code, 0)
        # The first line names the cap and how many messages came.
        self.assertRegex(self.lines[0], rf"\bcap of {SHORT_CAP_S:g} s\b")
        self.assertGreater(len(msgs), 1)
        self.assertRegex(self.lines[0], rf"\b{len(msgs)} messages seen\b")
        self.summary_checks(self.lines[1:6], [m["payload"] for m in msgs],
                            FA.END_CAP)
        # No error line, and no word of a detach.
        self.assertEqual(len(self.lines), 7, self.lines)
        self.assertFalse([line for line in self.lines if line.startswith("error")])
        self.assertFalse([line for line in self.lines if "detached" in line])
        self.assertEqual(session.detach_calls, 1)

    def test_the_command_line_refuses_a_zero_cap(self):
        # A real built agent and an out path that would both be taken, so
        # the cap is the only thing the command line can refuse.
        agent = make_built_agent(os.path.join(self.tmp.name, "built"), "clock")
        frida = FakeFrida(None)
        stderr = io.StringIO()
        with self.assertRaises(SystemExit) as caught:
            with unittest.mock.patch("sys.stderr", stderr):
                FA.main(["--agent", agent, "--cap", "0", "--out", self.out_path],
                        frida_mod=frida, wait=instant_wait,
                        out=self.lines.append)
        self.assertNotEqual(caught.exception.code, 0)
        # The refusal names the cap, and nothing was attached or written.
        self.assertRegex(stderr.getvalue(), r"\berror: --cap\b")
        self.assertEqual(frida.attached_to, [])
        self.assertFalse(os.path.exists(self.out_path))


# ---- the agents ---------------------------------------------------------------

class ByNameOnlyTest(unittest.TestCase):
    def test_the_check_catches_an_offset_read_added_to_a_real_agent(self):
        # Without this, a broken pattern would let every agent pass. The real
        # clock.ts passes; the same text with one offset read added fails at
        # that line.
        with open(os.path.join(AGENT_DIR, "clock.ts"), encoding="utf-8") as fh:
            clock = fh.read()
        bad = "const v = gm.handle.add(0x70).readDouble();"
        found = by_name_only_findings(clock + bad + "\n")
        self.assertEqual([(n, line) for n, line, _ in found],
                         [(len(clock.splitlines()) + 1, bad)])

    def test_no_agent_reads_by_offset(self):
        sources = agent_sources()
        self.assertIn(os.path.join(AGENT_DIR, "clock.ts"), sources)
        self.assertIn(os.path.join(AGENT_DIR, "state.ts"), sources)
        self.assertIn(os.path.join(AGENT_DIR, MENU_AGENT), sources)
        self.assertIn(os.path.join(AGENT_DIR, KIT_AGENT), sources)
        for path in sources:
            with self.subTest(agent=os.path.basename(path)):
                with open(path, encoding="utf-8") as fh:
                    self.assertEqual(by_name_only_findings(fh.read()), [])


class AllowedCallsTest(unittest.TestCase):
    @staticmethod
    def real_texts():
        texts = {}
        for path in agent_sources():
            with open(path, encoding="utf-8") as fh:
                texts[os.path.basename(path)] = fh.read()
        return texts

    def test_agent_calls_only_named_methods(self):
        self.assertEqual(call_findings(self.real_texts()), [])

    def test_only_the_listed_agents_hold_a_list(self):
        held = sorted(name for name, text in self.real_texts().items()
                      if _ALLOWED_LIST.search(text))
        self.assertEqual(held, sorted(LISTED_AGENTS))

    def test_a_list_in_an_unlisted_file_fails(self):
        texts = self.real_texts()
        texts["clock.ts"] += 'export const ALLOWED_CALLS = ["SetActive"];\n'
        findings = call_findings(texts)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith("clock.ts:"), findings[0])
        self.assertIn("ALLOWED_CALLS", findings[0])

    def test_a_method_outside_the_files_list_fails(self):
        texts = self.real_texts()
        added_at = len(texts["clock.ts"].splitlines()) + 1
        texts["clock.ts"] += 'go.method("SetActive");\n'
        findings = call_findings(texts)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith(f"clock.ts:{added_at}: SetActive"),
                        findings[0])

    def test_a_name_on_another_files_list_fails(self):
        # Each file's list covers that file only: a name from menu.ts's real
        # list, called in clock.ts, fails.
        texts = self.real_texts()
        name = allowed_calls(texts[MENU_AGENT])[0]
        added_at = len(texts["clock.ts"].splitlines()) + 1
        texts["clock.ts"] += f'go.method("{name}");\n'
        findings = call_findings(texts)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith(f"clock.ts:{added_at}: {name}"),
                        findings[0])

    def test_a_kit_list_name_the_data_file_lacks_fails(self):
        data = load_kit_methods()
        gone = data["methods"][0]["name"]
        data = {"methods": data["methods"][1:]}
        findings = kit_methods_findings(self.real_texts()[KIT_AGENT], data)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith(f"{gone} is in {KIT_AGENT}'s"),
                        findings[0])

    def test_a_data_file_name_the_kit_list_lacks_fails(self):
        data = load_kit_methods()
        gone = allowed_calls(self.real_texts()[KIT_AGENT])[0]
        kit = self.real_texts()[KIT_AGENT].replace(f'"{gone}"', '"x_placeholder"', 1)
        findings = kit_methods_findings(kit, data)
        self.assertTrue(any(f.startswith(f"{gone} is in kit_methods.json")
                            for f in findings), findings)

    def test_kit_calls_match_the_data_file(self):
        data = load_kit_methods()
        kit = self.real_texts()[KIT_AGENT]
        self.assertEqual(kit_methods_findings(kit, data), [])
        for entry in data["methods"]:
            with self.subTest(method=entry["name"]):
                self.assertIsInstance(entry["dump_line"], int)
                self.assertTrue(entry["image"])
                self.assertTrue(entry["class"])
                self.assertIn(f" {entry['name']}(", entry["signature"])

    def test_kit_load_map_passes_the_method_pointer(self):
        # docs/clone-hero/07-internals.md, IN-76: the LoadMap stub writes the
        # slot of its hidden method pointer, which the bridge's .invoke does
        # not pass for a non-generic method. The call is a NativeFunction
        # in kit.ts; it ends with the overload's own handle and is no .invoke.
        # Each check matches on names and punctuation only, so laying the
        # source out another way changes nothing.
        kit = self.real_texts()[KIT_AGENT]
        lookup = re.search(r"const\s+(\w+)\s*=\s*\w+\.method\(\s*\"LoadMap\"\s*,"
                           r"\s*5\s*\)([^;]*);", kit)
        self.assertIsNotNone(lookup, "no LoadMap lookup in kit.ts")
        method, chain = lookup.group(1), lookup.group(2)
        self.assertNotIn(".invoke(", chain)
        # A NativeFunction is on that method's own code address, and takes
        # the bridge's types with one more pointer at the end.
        native = re.search(rf"const\s+(\w+)\s*=\s*new\s+NativeFunction\(\s*"
                           rf"{method}\.virtualAddress\s*,[^,]*,\s*(\w+)\s*\)", kit)
        self.assertIsNotNone(native, f"no NativeFunction on {method}.virtualAddress")
        call, types = native.group(1), native.group(2)
        self.assertRegex(kit, rf"const\s+{types}\b[^=]*=\s*\[\s*\"pointer\"\s*,\s*"
                              rf"\.\.\.{method}\.parameters\.map\([^;]*\)\s*,\s*"
                              rf"\"pointer\"\s*\]\s*;")
        # The call passes the method's own handle as its last argument.
        self.assertRegex(kit, rf"\b{call}\(\s*\w+\.handle\s*,[^;]*,\s*"
                              rf"{method}\.handle\s*\)\s*;")

    def one_finding(self, texts, file, added_at, *parts):
        # The checker gives exactly one finding, on the added line, and it
        # holds each of `parts`.
        findings = call_findings(texts)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith(f"{file}:{added_at}:"), findings[0])
        for part in parts:
            self.assertIn(part, findings[0])

    def test_an_invoke_on_a_handle_from_elsewhere_in_kit_ts_fails(self):
        # The real kit.ts with an `.invoke(` added on a handle no lookup made.
        kit = self.real_texts()[KIT_AGENT]
        added = "const m = Il2Cpp.domain.assembly(GAME); m.invoke();"
        texts = {KIT_AGENT: kit + added + "\n"}
        self.one_finding(texts, KIT_AGENT, len(kit.splitlines()) + 1,
                         ".invoke( on a method not looked up", added)

    def test_an_invoke_on_a_set_handle_in_kit_ts_fails(self):
        # A name set from a real lookup, then set again from elsewhere, is
        # no longer known to hold a listed method.
        kit = self.real_texts()[KIT_AGENT]
        added = ('function stray(o: Il2Cpp.Object) { let m = o.method("get_Count"); '
                 'm = other; m.invoke(); }')
        texts = {KIT_AGENT: kit + added + "\n"}
        self.one_finding(texts, KIT_AGENT, len(kit.splitlines()) + 1,
                         ".invoke( on a method not looked up")

    def test_an_invoke_in_a_second_file_fails(self):
        # The real agents, with one `.invoke(` on a lookup added to clock.ts,
        # which holds no ALLOWED_CALLS list and so may look up nothing. The
        # one finding is the lookup's unlisted name.
        texts = self.real_texts()
        self.assertEqual(allowed_calls(texts["clock.ts"]), [])
        added_at = len(texts["clock.ts"].splitlines()) + 1
        texts["clock.ts"] += 'go.method("SetActive", 1).invoke(true);\n'
        self.one_finding(texts, "clock.ts", added_at, "SetActive is not in")

    def test_an_invoke_spelled_with_brackets_in_kit_ts_fails(self):
        # `m["invoke"](...)` is the same call as `m.invoke(...)`.
        kit = self.real_texts()[KIT_AGENT]
        added = 'const m = Il2Cpp.domain.assembly(GAME); m["invoke"]();'
        texts = {KIT_AGENT: kit + added + "\n"}
        self.one_finding(texts, KIT_AGENT, len(kit.splitlines()) + 1,
                         ".invoke( on a method not looked up", added)

    def test_a_bare_invoke_in_a_second_file_fails(self):
        texts = self.real_texts()
        added_at = len(texts["clock.ts"].splitlines()) + 1
        texts["clock.ts"] += "m.invoke();\n"
        self.one_finding(texts, "clock.ts", added_at,
                         ".invoke( on a method not looked up")

    def test_a_second_invoke_in_menu_ts_fails(self):
        # The real menu.ts, with its one `.invoke(` site (on the parameter of
        # its `call` helper) handed a method that no lookup made, from a new
        # line: the finding is on the `.invoke(` line, since `call` can no
        # longer promise a listed method.
        menu = self.real_texts()[MENU_AGENT]
        invoke_at = next(n for n, l in enumerate(menu.splitlines(), start=1)
                         if _INVOKE.search(l))
        texts = {MENU_AGENT: menu + "const stray = call(other);\n"}
        self.one_finding(texts, MENU_AGENT, invoke_at, ".invoke( on a method not looked up")

    def test_a_listed_name_passes_in_menu_ts(self):
        # The same added call, on a lookup of a name menu.ts lists, passes.
        menu = self.real_texts()[MENU_AGENT]
        name = allowed_calls(menu)[0]
        texts = {MENU_AGENT: menu + f'const ok = call(o.method("{name}"));\n'}
        self.assertEqual(call_findings(texts), [])

    def test_an_unlisted_name_in_menu_ts_fails(self):
        menu = self.real_texts()[MENU_AGENT]
        texts = {MENU_AGENT: menu + 'const stray = call(o.method("SetActive"));\n'}
        self.one_finding(texts, MENU_AGENT, len(menu.splitlines()) + 1,
                         f"SetActive is not in {MENU_AGENT}'s ALLOWED_CALLS")

    def test_a_lookup_by_a_name_that_is_not_a_literal_fails(self):
        # The real agents, with a lookup by a variable added to clock.ts.
        texts = self.real_texts()
        added_at = len(texts["clock.ts"].splitlines()) + 1
        added = "o.tryMethod(name);"
        texts["clock.ts"] += added + "\n"
        self.one_finding(texts, "clock.ts", added_at,
                         "a method looked up by a name that is not a literal", added)

    def test_a_native_function_fails(self):
        # The real agents, with a NativeFunction on an address no lookup gave
        # added to clock.ts.
        texts = self.real_texts()
        added_at = len(texts["clock.ts"].splitlines()) + 1
        added = "const f = new NativeFunction(p, 'void', []);"
        texts["clock.ts"] += added + "\n"
        self.one_finding(texts, "clock.ts", added_at, "calls a game function", added)

    def test_a_second_native_function_in_kit_ts_fails(self):
        # The real kit.ts with a NativeFunction added at the end on an
        # address no lookup gave: the one finding names kit.ts, the added
        # line's number and the line itself.
        kit = self.real_texts()[KIT_AGENT]
        line = "const f = new NativeFunction(p.virtualAddress, 'void', []);"
        texts = {KIT_AGENT: kit + line + "\n"}
        added_at = len(texts[KIT_AGENT].splitlines())
        findings = call_findings(texts)
        self.assertEqual(len(findings), 1, findings)
        self.assertTrue(findings[0].startswith(f"{KIT_AGENT}:{added_at}:"), findings[0])
        self.assertTrue(findings[0].endswith(line), findings[0])


# ---- kit.ts's play hook -----------------------------------------------------
# The play messages hook GameManager.Update. These checks read kit.ts as text.

_TIMING_START = "// ---- in-game timing"
_TIMING_END = "// ---- end of in-game timing"
_STRING = re.compile(r"([\"'`])(?:\\.|(?!\1).)*\1")
_NUMBER = re.compile(r"\b\d+(?:\.\d+)?\b")
# The one number the in-game block may hold, and the line it sits on: the
# unit change from the clock's seconds to the scheduler's ms. 0 is allowed
# anywhere, for comparisons and the hook's first argument.
_UNIT_LINE = "const MS_PER_S = 1000;"


def timing_block(kit_text: str) -> str:
    """kit.ts's in-game timing block, its two marker lines left out."""
    lines = kit_text.splitlines()
    start = next(i for i, l in enumerate(lines) if l.startswith(_TIMING_START))
    end = next(i for i, l in enumerate(lines) if l.startswith(_TIMING_END))
    return "\n".join(lines[start + 1:end])


def own_number_findings(block: str) -> list[str]:
    """Every number literal in `block`'s code (comments and strings left
    out) other than 0 and the unit line's own."""
    found = []
    for line in block.splitlines():
        code = _STRING.sub("", line.split("//", 1)[0]).strip()
        for token in _NUMBER.findall(code):
            if token != "0" and code != _UNIT_LINE:
                found.append(f"holds the number {token}: {line.strip()}")
    return found


def play_waits_findings(kit_text: str) -> list[str]:
    """Why kit.ts's play case would wait for its schedule before replying:
    a play case that does not reply with startPlay through settled, or a
    startPlay that is async, awaits, sleeps or chains a promise."""
    found = []
    if 'case "play":\n            return settled(() => startPlay(m));' not in kit_text:
        found.append('the play case does not reply with settled(() => startPlay(m))')
    if re.search(r"async\s+function\s+startPlay\b", kit_text):
        found.append("startPlay is async")
    body = top_functions(kit_text).get("startPlay")
    if body is None:
        found.append("no function startPlay")
        return found
    for word in ("await", "sleep(", ".then(", "Promise"):
        if word in body:
            found.append(f"startPlay holds {word}")
    return found


def stop_path_findings(kit_text: str) -> list[str]:
    """Every place a play can stop that does not reach the one function that
    takes the hook out and lifts every pad (unhook): a function that steps or
    stops the scheduler without applyStep, an applyStep that does not call
    endPlay, an endPlay, teardown or frameProbe that does not call unhook, an
    unhook that does not detach and write the lift, and a detach or a hook
    placed anywhere else."""
    fns = top_functions(kit_text)
    found = []
    for name, body in sorted(fns.items()):
        stops = "schedule.step(" in body or "schedule.stop(" in body
        if stops and name != "applyStep" and "applyStep(" not in body:
            found.append(f"{name} steps or stops the scheduler without applyStep")
        if ".detach()" in body and name != "unhook":
            found.append(f"{name} detaches a hook outside unhook")
        if "Interceptor.attach(" in body and name != "placeHook":
            found.append(f"{name} places a hook outside placeHook")
    for name, needs in (("applyStep", "endPlay("), ("endPlay", "unhook("),
                        ("teardown", "unhook("), ("frameProbe", "unhook("),
                        ("unhook", ".detach()"), ("unhook", "write(kit, pads, true)")):
        if needs not in fns.get(name, ""):
            found.append(f"{name} does not call {needs}")
    outside = "\n".join(body for name, body in fns.items())
    if kit_text.count(".detach()") != outside.count(".detach()"):
        found.append("a detach sits outside every function")
    return found


class KitPlayHookTest(unittest.TestCase):
    @staticmethod
    def kit() -> str:
        with open(os.path.join(AGENT_DIR, KIT_AGENT), encoding="utf-8") as fh:
            return fh.read()

    def test_kit_hooks_game_manager_update_once(self):
        kit = self.kit()
        self.assertIn('.class(GAME_MANAGER).method("Update", 0)', kit)
        self.assertIn("Update", allowed_calls(kit))
        # The data file lists Update once, on GameManager; its dump line is
        # the data file's own value, checked by test_kit_calls_match_the_data_file.
        update = [e for e in load_kit_methods()["methods"] if e["name"] == "Update"]
        self.assertEqual([e["class"] for e in update], ["GameManager"])
        self.assertEqual(kit.count("Interceptor.attach("), 1)
        self.assertIn('import { PressSchedule } from "./press_schedule.js";', kit)

    def test_kit_play_holds_no_number_of_its_own(self):
        kit = self.kit()
        block = timing_block(kit)
        self.assertEqual(own_number_findings(block), [])
        self.assertIn(_UNIT_LINE, block)

    def test_a_number_of_its_own_fails(self):
        # The real timing block with one real kit.ts line from outside it
        # added: setup's LoadMap lookup, which names its 5 parameters.
        kit = self.kit()
        added = next(l for l in kit.splitlines() if '.method("LoadMap", 5)' in l)
        findings = own_number_findings(timing_block(kit) + "\n" + added + "\n")
        # One finding, naming the number and the line that holds it.
        self.assertEqual(len(findings), 1, findings)
        self.assertRegex(findings[0], r"\b5\b")
        self.assertIn(added.strip(), findings[0])

    # A small kit text with the play case, startPlay and every stop path the
    # two checks read, each laid out the way kit.ts lays it out. The bad cases
    # below edit this text, not the real kit.ts, so a reformat of kit.ts
    # cannot turn an edit into one that changes nothing.
    PLAY_KIT = (
        'function answer(m) {\n'
        '    switch (m.type) {\n'
        '        case "play":\n'
        '            return settled(() => startPlay(m));\n'
        '    }\n'
        '}\n'
        '\n'
        'function startPlay(m) {\n'
        '    play = p;\n'
        '    return { started: true };\n'
        '}\n')
    STOP_KIT = (
        'function placeHook(kit) {\n'
        '    hook = Interceptor.attach(update.virtualAddress, {});\n'
        '}\n'
        '\n'
        'function unhook(kit) {\n'
        '    hook?.detach();\n'
        '    write(kit, pads, true);\n'
        '}\n'
        '\n'
        'function applyStep(p, step, now) {\n'
        '    endPlay(p);\n'
        '}\n'
        '\n'
        'function endPlay(p) {\n'
        '    try {\n'
        '        unhook(p.kit);\n'
        '    } finally {\n'
        '        play = null;\n'
        '    }\n'
        '}\n'
        '\n'
        'function stopPlay(p) {\n'
        '    applyStep(p, p.schedule.stop(Date.now()), null);\n'
        '    return { stopped: true };\n'
        '}\n'
        '\n'
        'function teardown() {\n'
        '    unhook(kit);\n'
        '}\n'
        '\n'
        'function frameProbe() {\n'
        '    unhook(kit);\n'
        '}\n')

    def edited(self, text, old, new):
        # `text` with its one `old` replaced by `new`; fails if `old` is not there.
        self.assertEqual(text.count(old), 1, old)
        return text.replace(old, new)

    def test_a_play_that_waits_fails(self):
        self.assertEqual(play_waits_findings(self.PLAY_KIT), [])
        kit = self.edited(self.PLAY_KIT, "function startPlay(", "async function startPlay(")
        self.assertEqual(play_waits_findings(kit), ["startPlay is async"])
        kit = self.edited(self.PLAY_KIT, "    play = p;\n", "    play = p;\n    await sleep(1);\n")
        self.assertEqual(play_waits_findings(kit),
                         ["startPlay holds await", "startPlay holds sleep("])

    def test_a_stop_that_skips_unhook_fails(self):
        self.assertEqual(stop_path_findings(self.STOP_KIT), [])
        bad = self.edited(self.STOP_KIT,
                          "    applyStep(p, p.schedule.stop(Date.now()), null);\n",
                          "    p.schedule.stop(Date.now());\n")
        self.assertEqual(stop_path_findings(bad),
                         ["stopPlay steps or stops the scheduler without applyStep"])
        bad = self.edited(self.STOP_KIT, "        unhook(p.kit);\n", "        hook?.detach();\n")
        self.assertEqual(stop_path_findings(bad), [
            "endPlay detaches a hook outside unhook", "endPlay does not call unhook("])


class DestroyedSkipTest(unittest.TestCase):
    def read(self, name):
        with open(os.path.join(AGENT_DIR, name), encoding="utf-8") as fh:
            return fh.read()

    def test_only_gamemanager_ts_names_the_native_object_field(self):
        # One owner for "is this Unity object destroyed": gamemanager.ts. The
        # other agents never name the field; menu.ts imports isDestroyed.
        for path in agent_sources():
            name = os.path.basename(path)
            if name == "gamemanager.ts":
                continue
            with self.subTest(agent=name):
                self.assertNotIn("m_CachedPtr", self.read(name))
        self.assertIn("export function isDestroyed(", self.read("gamemanager.ts"))
        self.assertRegex(self.read("menu.ts"),
                         r'(?m)^import\s*\{[^}]*\bisDestroyed\b[^}]*\}\s*from\s*"\./gamemanager\.js"')

    def test_the_state_agent_sends_the_destroyed_count_in_both_replies(self):
        # Every object readState returns, found or not, holds a `destroyed`
        # key, written out or as a shorthand.
        body = top_functions(self.read("state.ts"))["readState"]
        self.assertIn("lookUpGameManager()", body)
        replies = [body[m.end() - 1:_close(body, m.end() - 1, "{", "}")]
                   for m in re.finditer(r"\breturn\s*\{", body)]
        self.assertEqual(len(replies), 2, replies)
        for reply in replies:
            with self.subTest(reply=reply[:60]):
                self.assertRegex(reply, r"[{,]\s*destroyed\s*[:,}]")


LAUNCH_AGENT = "launch.ts"
LAUNCH_CORE = "launch_core.ts"
# A top-level function and its body, up to the first line that starts with `}`.
_TOP_FUNCTION = re.compile(r"^(?:async\s+)?function\s+(\w+)\s*\(.*?^\}",
                           re.MULTILINE | re.DOTALL)
# A number written in hex, as an address would be.
_HEX_LITERAL = re.compile(r"\b0x[0-9A-Fa-f]+\b")


def top_functions(text: str) -> dict[str, str]:
    """Each top-level function of an agent file, by name -> its whole text."""
    return {m.group(1): m.group(0) for m in _TOP_FUNCTION.finditer(text)}


def patch_findings(text: str) -> list[str]:
    """Why `text`, a launch agent, could change the game's code outside the
    module observer: the observer and the patch must each appear once, the
    patch must sit in one top-level function, and that function must be
    called from the observer's callback, from its setImmediate branch, and
    from nowhere else."""
    found = []
    for what in ("Process.attachModuleObserver(", "Memory.patchCode("):
        n = text.count(what)
        if n != 1:
            found.append(f"{n} {what} sites, expected 1")
    if found:
        return found
    holders = [name for name, body in top_functions(text).items()
               if "Memory.patchCode(" in body]
    if len(holders) != 1:
        return [f"Memory.patchCode( sits in {len(holders)} top-level "
                "functions, expected 1"]
    patcher = holders[0]
    observer_at = text.index("Process.attachModuleObserver(")
    observer = text[observer_at:text.index("\n    });", observer_at)]
    calls = re.findall(rf"\b{patcher}\(", text)
    deferred = re.findall(rf"setImmediate\(\(\)\s*=>\s*{patcher}\(", observer)
    # The definition, the deferred call and the direct call.
    if len(calls) != 3:
        found.append(f"{patcher}( appears {len(calls)} times, expected its "
                     "definition and the observer's two calls")
    if len(re.findall(rf"\b{patcher}\(", observer)) != 2 or len(deferred) != 1:
        found.append(f"the observer's callback does not call {patcher} once "
                     "directly and once through setImmediate")
    return found


class LaunchAgentTest(unittest.TestCase):
    def read(self, name):
        with open(os.path.join(AGENT_DIR, name), encoding="utf-8") as fh:
            return fh.read()

    def test_the_launch_files_are_scanned_with_the_other_agents(self):
        # The by-name and allowed-calls tests above read every agent_sources
        # file, so these two get them too. Neither holds an ALLOWED_CALLS
        # list, so neither may look up or call a game method.
        names = [os.path.basename(p) for p in agent_sources()]
        self.assertIn(LAUNCH_AGENT, names)
        self.assertIn(LAUNCH_CORE, names)
        self.assertEqual(allowed_calls(self.read(LAUNCH_AGENT)), [])
        self.assertEqual(allowed_calls(self.read(LAUNCH_CORE)), [])

    def test_neither_launch_file_holds_an_address(self):
        # Every RVA arrives in the arm message, from launch_patch.json.
        for name in (LAUNCH_AGENT, LAUNCH_CORE):
            with self.subTest(agent=name):
                self.assertEqual(_HEX_LITERAL.findall(self.read(name)), [])

    def test_the_check_catches_a_patch_outside_the_observer(self):
        good = self.read(LAUNCH_AGENT)
        self.assertEqual(patch_findings(good), [])
        deferred = "setImmediate(() => patchModule(patch, module));"
        self.assertEqual(good.count(deferred), 1, deferred)
        calls = "patchModule( appears {} times, expected its definition and the observer's two calls"
        observer = ("the observer's callback does not call patchModule once "
                    "directly and once through setImmediate")
        bad = {
            "a second patch": (good + "Memory.patchCode(p, 1, () => {});\n",
                               ["2 Memory.patchCode( sites, expected 1"]),
            "a second observer": (good + "Process.attachModuleObserver({});\n",
                                  ["2 Process.attachModuleObserver( sites, expected 1"]),
            "a patch at load": (good + "patchModule(armed!, Process.getModuleByName(\"x\"));\n",
                                [calls.format(4)]),
            "no deferred branch": (good.replace(deferred, "return;"),
                                   [calls.format(2), observer]),
        }
        for what, (text, findings) in bad.items():
            with self.subTest(case=what):
                self.assertEqual(patch_findings(text), findings)


class LaunchStatusRepliesTest(Workspace):
    def test_every_status_the_agent_sent_comes_back_whole(self):
        # launch.ts's replies from one real human launch, ids 1 to 6
        # (hydra-data/ch-mcp-runs/2026-10-31/ls-d0-run1): the start's idle,
        # the arm reply naming the modules, a waiting status, the patched
        # status and two statuses after the loader ran. LiveAgent raises on
        # an `error` key, so a status that carried one would fail every poll
        # of bot_run.launch_human.
        msgs = recorded("ls-d0-run1", "launch-20261031-141438.jsonl")
        session = ReplySession([[m] for m in msgs])
        agent = FA.LiveAgent(self.agent, log_path=self.out_path,
                             frida_mod=FakeFrida(session), wait=FA._default_wait,
                             out=self.lines.append)
        self.addCleanup(agent.detach)
        replies = [agent.start(30.0), agent.request({"type": "arm"}, 5.0)]
        replies += [agent.request({"type": "status"}, 5.0) for _ in msgs[2:]]
        # Each reply is the agent's payload whole, less the id the request gave.
        for reply, msg in zip(replies, msgs):
            with self.subTest(id=msg["payload"]["id"]):
                self.assertNotIn("id", reply)
                self.assertEqual({**reply, "id": msg["payload"]["id"]}, msg["payload"])
        self.assertEqual(len(replies), len(msgs))
        self.assertTrue(agent.alive)
        self.assertEqual(self.lines, [])


class LaunchErrorKeyTest(unittest.TestCase):
    @staticmethod
    def code(name):
        # The agent's text with its comment lines left out.
        with open(os.path.join(AGENT_DIR, name), encoding="utf-8") as fh:
            return "\n".join(l for l in fh.read().splitlines()
                             if not l.lstrip().startswith("//"))

    def test_launch_ts_sends_an_error_key_only_for_a_fault(self):
        # LiveAgent raises on an `error` key, so launch.ts builds one only
        # inside a catch block, and launch_core.ts builds none.
        code = self.code(LAUNCH_AGENT)
        sites = [m.start() for m in re.finditer(r"\berror\s*:", code)]
        self.assertNotEqual(sites, [])
        catches = [(m.end() - 1, _close(code, m.end() - 1, "{", "}"))
                   for m in re.finditer(r"\bcatch\s*\([^)]*\)\s*\{", code)]
        for at in sites:
            with self.subTest(line=code.count("\n", 0, at) + 1):
                self.assertTrue(any(lo < at < hi for lo, hi in catches))
        self.assertNotRegex(self.code(LAUNCH_CORE), r"\berror\s*:")


class BuildScriptsTest(unittest.TestCase):
    def test_build_scripts_end_by_themselves(self):
        # A watch build never exits, so it would have no cap.
        with open(os.path.join(AGENT_DIR, "package.json"),
                  encoding="utf-8") as fh:
            scripts = json.load(fh)["scripts"]
        # Every agent no other agent imports is one the loaders start, so
        # each has a build script, and no script builds anything else.
        imported = set()
        for path in agent_sources():
            with open(path, encoding="utf-8") as fh:
                imported |= {m + ".ts" for m in re.findall(r'from\s+"\./(\w+)\.js"', fh.read())}
        entries = sorted(os.path.basename(p) for p in agent_sources()
                         if os.path.basename(p) not in imported)
        built = sorted(t for command in scripts.values()
                       for t in re.findall(r"\b(\w+\.ts)\b", command))
        self.assertNotEqual(entries, [])
        self.assertEqual(built, entries)
        for name, command in scripts.items():
            with self.subTest(script=name):
                words = command.split()
                self.assertNotIn("-w", words)
                self.assertNotIn("--watch", words)


if __name__ == "__main__":
    unittest.main()
