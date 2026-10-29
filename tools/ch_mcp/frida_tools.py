"""The Clone Hero MCP server's Frida tools: attach to the game, read its
state, and trace the notes it processes.

Each tool is a thin call into the module that owns the work (the plan
docs/superpowers/plans/2026-10-28-ch-mcp-server.md, "The tools" and "Who owns
which question"): frida_attach.LiveAgent for the attach and the reads,
build_facts for the build check and the hook names, ch_probe.constants for
the hook addresses, and trace_hits for the trace and its line-up. This module
adds only the session state between tool calls (the live agent, the trace,
the facts loaded once per build) and the state words, which it owns.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server (server.py) hands in the one RLock it shares
with game_tools.GameTools; every tool holds it.
"""

from __future__ import annotations

import json
import os
import time
from typing import Any, Callable, List, Optional

from tools.ch_mcp import build_facts
from tools.ch_mcp import frida_attach as FA
from tools.ch_mcp import trace_hits as TH
from tools.ch_probe import constants as C

# The three Frida caps of the plan's question 5, approved by the user on
# 2026-10-29.
# How long after the server launched the game ch_attach waits before it
# attaches (5a): a command-line launch has not built its song before then.
ATTACH_DELAY_S = 5.0
# How long Frida gets to attach, load the state agent and answer its first
# read (5b).
ATTACH_CAP_S = 30.0
# How long one state read gets to come back (5c). Each read walks the whole
# heap; live step L1 times it.
READ_CAP_S = 5.0

AGENT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "agent")
STATE_AGENT_NAME = "state"
STATE_AGENT_FILE = f"{STATE_AGENT_NAME}.js"

# ch_trace_start's refusal when the state read finds no live GameManager.
NO_GAME_MANAGER = ("no live GameManager: start the song (pause it if you like), "
                   "then call ch_trace_start")

NOT_ATTACHED = "not attached; call ch_attach first"


def state_words(snapshot: Optional[dict], *, running: bool = True) -> str:
    """The one word (or short phrase) for the game's state. `snapshot` is a
    state.ts reply, or None when no agent is attached; `running` is False
    when the last attach found no game process. The agent never names a
    state: this is the one home of these words and their order."""
    if not running:
        return "not running"
    if snapshot is None:
        return "not attached"
    if not snapshot.get("found"):
        return "menu (no live GameManager)"
    if not snapshot.get("songLoaded"):
        return "loading"
    if snapshot.get("isSongOver"):
        return "song over"
    if snapshot.get("isPaused"):
        return "paused"
    if snapshot.get("isSongPlaying"):
        return "playing"
    return "loaded, not started"


def _value_text(value: Any) -> str:
    return value if isinstance(value, str) else json.dumps(value, ensure_ascii=False)


def _field_lines(snapshot: dict) -> List[str]:
    """Every field the agent sent, in its order, as `name: value`."""
    return [f"{key}: {_value_text(value)}" for key, value in snapshot.items()]


class FridaTools:
    """The seven Frida tools and the session state between them.

    `game` is the server's game_tools.GameTools (the tests pass a fake with
    the same five methods); its stop hook, registered here, stops the trace
    and detaches, and its session_dir() is the folder the agent logs go in.
    Every outside seam is an argument, so the
    tests swap each for a fake: `frida_mod` (None imports frida when the
    first attach needs it), `wait` (how a read waits for its reply),
    `current_build` and `load_facts` (build_facts'), `trace_session`
    (trace_hits.TraceSession), `agent_dir`, `now` and `sleep` (the clock
    `game.launched_at` is on)."""

    def __init__(self, game, lock, *,
                 frida_mod: Any = None,
                 wait: Optional[Callable] = None,
                 current_build: Callable[[], str] = build_facts.current_build,
                 load_facts: Callable[[], dict] = build_facts.load_facts,
                 trace_session: Callable[..., Any] = TH.TraceSession,
                 agent_dir: str = AGENT_DIR,
                 now: Callable[[], float] = time.monotonic,
                 sleep: Callable[[float], None] = time.sleep):
        self._game = game
        self._lock = lock
        self._frida_mod = frida_mod
        self._wait = wait
        self._current_build = current_build
        self._load_facts = load_facts
        self._trace_session = trace_session
        self._agent_dir = agent_dir
        self._now = now
        self._sleep = sleep

        self._agent: Optional[FA.LiveAgent] = None
        self._not_running = False
        # The facts of one build, loaded once and kept: (version, facts).
        self._facts: Optional[tuple] = None
        self._trace = None
        self._count = 0
        # Lines the live agent says outside a reply (a refused detach).
        self._said: List[str] = []
        game.add_stop_hook(self._stop_hook)

    # -- attaching and reading ------------------------------------------------------

    def ch_attach(self) -> str:
        """Check the build, take the backup for a game the server did not
        start, wait out ATTACH_DELAY_S after a launch, then attach the state
        agent and show its first read."""
        with self._lock:
            lines: List[str] = []
            if self._agent is not None and self._agent.alive:
                return "already attached; call ch_detach first to attach again"
            try:
                version, _ = self._facts_now()
            except build_facts.BuildFactsError as e:
                return f"{e}\nnothing attached"
            lines.append(f"build: {version} (facts: {build_facts.facts_path(version)})")
            path = os.path.join(self._agent_dir, STATE_AGENT_FILE)
            if not os.path.isfile(path):
                lines.append(TH.agent_missing_line(path, STATE_AGENT_NAME))
                return "\n".join(lines)
            launched = self._game.launched_at()
            if launched is None:
                backup = self._game.ensure_backup()
                if backup:
                    lines.append(backup)
            else:
                remaining = ATTACH_DELAY_S - (self._now() - launched)
                if remaining > 0:
                    lines.append(f"waited {remaining:.1f} s, so the attach comes "
                                 f"{ATTACH_DELAY_S:g} s after the launch (ATTACH_DELAY_S)")
                    self._sleep(remaining)
            if self._agent is not None:
                # A session the game dropped: detach closes its log file.
                self._agent.detach()
                self._agent = None
            self._count += 1
            log = os.path.join(self._game.session_dir(), f"state-{self._count}.jsonl")
            agent = FA.LiveAgent(path, log_path=log, frida_mod=self._frida_mod,
                                 wait=self._wait, out=self._said.append)
            started = self._now()
            try:
                first = agent.start(ATTACH_CAP_S)
            except FA.AttachFailed as e:
                self._not_running = e.reason == FA.END_NOT_RUNNING
                lines.append(str(e) + self._cap_note(e, "ATTACH_CAP_S"))
                return self._reply(lines)
            except SystemExit as e:  # frida_attach's advice for a failed import
                lines.append(str(e))
                return self._reply(lines)
            self._not_running = False
            self._agent = agent
            lines.append(f"attached in {self._now() - started:.1f} s; log: {log}")
            lines.append(f"state: {state_words(first)}")
            lines.extend(_field_lines(first))
            return self._reply(lines)

    def ch_detach(self) -> str:
        """Detach the state agent. A trace keeps its own session; ch_trace_stop
        ends it."""
        with self._lock:
            if self._agent is None:
                return "nothing attached"
            line = self._agent.detach()
            self._agent = None
            return self._reply([line])

    def ch_state(self) -> str:
        """One fresh read: the state word, then the fields it was read from."""
        with self._lock:
            lines: List[str] = []
            snapshot = self._read(lines)
            if snapshot is None:
                return self._reply(lines)
            lines.insert(0, f"state: {state_words(snapshot)}")
            lines.extend(_field_lines(snapshot))
            return self._reply(lines)

    def ch_snapshot(self) -> str:
        """One fresh read of GameManager's fields, then the score after the
        play session's last press and the song clock at that press, so the
        two clocks sit side by side."""
        with self._lock:
            lines: List[str] = []
            snapshot = self._read(lines)
            if snapshot is None:
                return self._reply(lines)
            lines.extend(_field_lines(snapshot))
            song_time = snapshot.get("songTime")
            if isinstance(song_time, (int, float)):
                lines.append(f"songTime in ms: {C.s_to_ms(song_time):.1f} (GameManager's clock)")
            lines.extend(self._score_lines())
            return self._reply(lines)

    # -- the trace ----------------------------------------------------------------------

    def ch_trace_start(self, song_folder: str, score_json: Optional[str] = None,
                       with_dynamics: bool = False) -> str:
        """Hook the per-note engine method (and the dynamics one with
        with_dynamics=True) in a background trace, after one state read shows
        a live GameManager."""
        with self._lock:
            if self._trace is not None and not self._trace.ended():
                return "a trace is already running; call ch_trace_stop first"
            try:
                _, facts = self._facts_now()
                engine, ctor_rva = build_facts.engine_class(facts)
                rvas = list(C.PER_NOTE_DEFAULT_RVAS)
                if with_dynamics:
                    rvas.append(C.RVA_NOTE_DYNAMICS)
                methods = build_facts.method_names_by_rva(facts, engine, rvas)
            except build_facts.BuildFactsError as e:
                return f"{e}\nno trace started"
            lines: List[str] = []
            snapshot = self._read(lines)
            if snapshot is None:
                lines.append("no trace started")
                return self._reply(lines)
            if not snapshot.get("found"):
                lines.append(NO_GAME_MANAGER)
                return self._reply(lines)
            try:
                notes_path, notes = TH.song_notes(song_folder, score_json)
            except TH.NotesMissing as e:
                lines.append(str(e))
                return self._reply(lines)
            except (OSError, ValueError, KeyError, TypeError) as e:
                lines.append(f"could not read the notes: {type(e).__name__}: {e}")
                return self._reply(lines)
            if not notes:
                lines.append(f"no notes in {notes_path}; no trace started")
                return self._reply(lines)
            self._count += 1
            out_path = os.path.join(self._game.session_dir(), f"trace-{self._count}.jsonl")
            agent_path = os.path.join(self._agent_dir, os.path.basename(TH.DEFAULT_AGENT))
            try:
                session = self._trace_session(notes, engine, methods, out_path,
                                              engine_ctor_rva=ctor_rva,
                                              agent_path=agent_path,
                                              frida_mod=self._frida_mod)
            except TH.AgentMissing as e:
                lines.append(str(e))
                return self._reply(lines)
            # The hooks are the first touch of the game this call makes, so the
            # backup comes after every refusal above and just before them.
            # ensure_backup knows whether a run or an earlier call holds one.
            backup = self._game.ensure_backup()
            if backup:
                lines.append(backup)
            self._trace = session.start()
            lines.append(f"trace started; log: {out_path}")
            lines.append(f"engine class: {engine}, constructor RVA 0x{ctor_rva:X}; "
                         "trace.ts checks its shape and ch_notes shows the result")
            lines.append("hooked methods: " + ", ".join(
                f"{name} (0x{rva:X})" for name, rva in zip(methods, rvas)))
            lines.append(f"notes: {len(notes)} from {notes_path}")
            lines.append(f"song cap: {session.settings['song_cap_s']:g} s of song played; "
                         f"wall cap: {session.cap_s:g} s")
            return self._reply(lines)

    def ch_notes(self, since: int = 0) -> str:
        """The trace's calls from index `since` on, each next to its nearest
        note (trace_hits.line_up), then how many sit within
        EXPECT_NORMAL_BACK_MS of it."""
        with self._lock:
            session = self._trace
            if session is None:
                return "no trace has run; call ch_trace_start"
            since = max(0, since)
            lined = session.lined_up(since)
            total = since + len(lined)
            lines = list(TH.engine_lines(self._trace_result(session)))
            for i, x in enumerate(lined, start=since):
                lines.append(f"call {i}: {x.call['call']} at {x.call_ms:.1f} ms; "
                             f"nearest note {x.note_index} at {x.note_ms} ms, "
                             f"diff {x.diff_ms:+.1f} ms")
            limit = C.EXPECT_NORMAL_BACK_MS
            lines.append(f"calls within {limit:g} ms of their nearest note "
                         f"(EXPECT_NORMAL_BACK_MS): {TH.count_within(lined, limit)} "
                         f"of {len(lined)}")
            lines.append(session.ended_line())
            lines.extend(session.lines)
            lines.append(f"next: ch_notes(since={total})")
            return self._reply(lines)

    def ch_trace_stop(self) -> str:
        """Stop the trace, waiting at most READ_CAP_S for it to detach, and
        show how it ended and how many calls it saw."""
        with self._lock:
            if self._trace is None:
                return "no trace has run"
            return self._reply(self._trace_stop_lines(self._trace))

    # -- for the server -------------------------------------------------------------

    def status_lines(self) -> List[str]:
        """The Frida half of ch_status: attached or not, and the trace."""
        with self._lock:
            lines: List[str] = []
            agent = self._agent
            if agent is not None and agent.alive:
                lines.append(f"frida: attached (log: {agent.log_path})")
            elif agent is not None:
                lines.append(f"frida: not attached ({agent.gone_reason})")
            else:
                lines.append("frida: not attached")
            trace = self._trace
            if trace is None:
                lines.append("trace: none")
            elif trace.ended():
                lines.append(f"trace: {trace.ended_line()}, {len(trace.calls())} calls")
            else:
                lines.append(f"trace: running, {len(trace.calls())} calls so far")
            return lines

    # -- inside -------------------------------------------------------------------------

    def _facts_now(self) -> tuple:
        """The installed build's version and its facts, loaded once per build
        and kept. Raises build_facts.BuildFactsError when there are none."""
        version = self._current_build()
        if self._facts is None or self._facts[0] != version:
            self._facts = (version, self._load_facts())
        return self._facts

    def _read(self, lines: List[str]) -> Optional[dict]:
        """One state read, or None with the reason in `lines`."""
        agent = self._agent
        if agent is None or not agent.alive:
            word = state_words(None, running=not self._not_running)
            # "not running" is what the last attach found; the game may have
            # started since, so the line says whose answer it is.
            if agent is not None:
                why = f" ({agent.gone_reason})"
            elif self._not_running:
                why = " (found by the last ch_attach)"
            else:
                why = ""
            lines.append(f"state: {word}{why}; {NOT_ATTACHED}")
            return None
        try:
            return agent.read(READ_CAP_S)
        except FA.ReadFailed as e:
            lines.append(f"read failed: {e}{self._cap_note(e, 'READ_CAP_S')}")
            if not agent.alive:
                lines.append(f"state: {state_words(None)}")
            return None

    @staticmethod
    def _cap_note(error: FA.LiveAgentError, name: str) -> str:
        return f" ({name})" if error.reason == FA.END_CAP else ""

    def _score_lines(self) -> List[str]:
        """The score after the play session's last press (the plan's
        question 7); its owner is the play session's press row."""
        play = self._game.play_snapshot()
        if play is None:
            return ["score: not read (no play session has found the engine)"]
        row = play["last_row"]
        ended = "" if play["stop"] is None else f" (the play session ended: {play['stop']})"
        if row is None:
            return [f"score: no press yet in the play session{ended}"]
        return [f"score after press {play['press_count']}: {row['score_after']}{ended}",
                f"song clock at that press: {row['clock_ms_at_press']} ms "
                "(the play session's clock)"]

    @staticmethod
    def _trace_result(session) -> FA.RunResult:
        """The finished run, or the payloads so far in a run's shape, for
        trace_hits' readers."""
        if session.result is not None:
            return session.result
        return FA.RunResult(reason="running", cap_s=session.cap_s,
                            payloads=session.payloads())

    @staticmethod
    def _trace_stop_lines(session) -> List[str]:
        if not session.ended() and not session.stop(READ_CAP_S):
            return [f"the trace thread was still running after {READ_CAP_S:g} s "
                    "(READ_CAP_S); it ends with the server"]
        return [session.ended_line(), f"calls: {len(session.calls())}"] + list(session.lines)

    def _stop_hook(self) -> Optional[str]:
        """game_tools' stop hook: stop the trace, then detach the agent."""
        with self._lock:
            lines: List[str] = []
            if self._trace is not None and not self._trace.ended():
                lines.append("trace: " + "; ".join(self._trace_stop_lines(self._trace)))
            if self._agent is not None:
                lines.append("frida: " + self._agent.detach())
                self._agent = None
            lines.extend(self._said)
            self._said.clear()
            return "\n".join(lines) or None

    def _reply(self, lines: List[str]) -> str:
        """`lines`, then anything the live agent said meanwhile."""
        lines = lines + self._said
        self._said.clear()
        return "\n".join(lines)
