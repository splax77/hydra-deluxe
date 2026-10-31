"""The Clone Hero MCP server's Frida tools: attach to the game, read its
state, and trace the notes the player hits.

Each tool is a thin call into the module that owns the work (the plan
docs/superpowers/plans/2026-10-28-ch-mcp-server.md, "The tools" and "Who owns
which question"): frida_attach.LiveAgent for the attach and the reads,
build_facts for the build check and the hook names, ch_probe.constants for
the hook addresses, and trace_hits for the trace and its line-up. This module
adds only the session state between tool calls (the live agent, the trace,
the facts loaded once per build), the state words, which it owns, and the
menu lines, which print agent/menu.ts's block as sent.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel. The server (server.py) hands in the one RLock it shares
with game_tools.GameTools; every tool holds it.
"""

from __future__ import annotations

import functools
import json
import os
import time
from typing import Any, Callable, Dict, List, Optional

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

# The key of state.ts's reply that holds menu.ts's block.
MENU_BLOCK = "menu"
# The menu fields menu.ts reads and their enum words; menu.ts bundles the same
# file at build time.
MENU_FIELDS_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "menu_fields.json")


class StateUnread(Exception):
    """FridaTools.read_state found no attached agent or the read failed; the
    text is the reason, as ch_state would print it."""


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
    """Every GameManager field the agent sent, in its order, as `name:
    value`. The menu block is left out: ch_menu prints it."""
    return [f"{key}: {_value_text(value)}" for key, value in snapshot.items()
            if key != MENU_BLOCK]


@functools.lru_cache(maxsize=None)
def menu_fields() -> dict:
    """menu_fields.json, read once: the fields agent/menu.ts reads, each with
    its enum table."""
    with open(MENU_FIELDS_FILE, encoding="utf-8") as f:
        return json.load(f)


# menu_fields.json's kinds that this file prints differently; an entry with no
# kind is an enum (menu_fields.json's "about" says what each kind means).
ENUM_KIND = "enum"
ARRAY_KINDS = ("string_array", "enum_array")
# The keys the highlighted row is worked out from: the BaseMenu cursor and
# scroll, and the two row arrays it indexes. highlighted_row_text is the one
# home of that rule; the agent sends only the raw values.
CURSOR_KEY = "cursor"
SCROLL_KEY = "scroll"
ROW_NAMES_KEY = "rowNames"
ROW_IDS_KEY = "rowInstruments"


def _entry(key: str) -> Optional[dict]:
    for field in menu_fields()["fields"]:
        if field["key"] == key:
            return field
    return None


def _kind(key: str) -> str:
    entry = _entry(key)
    return ENUM_KIND if entry is None else entry.get("kind", ENUM_KIND)


def _enum_words(key: str) -> Optional[Dict[str, str]]:
    """The word table for an enum entry, or for an enum_array the table of
    the entry its words_from names."""
    entry = _entry(key)
    if entry is None:
        return None
    if "words_from" in entry:
        return _enum_words(entry["words_from"])
    return entry.get("values")


def _is_number(value: Any) -> bool:
    return isinstance(value, int) and not isinstance(value, bool)


def _number_text(key: str, value: Any) -> str:
    """One enum number with its word from menu_fields.json's table; anything
    else as sent."""
    if not _is_number(value):
        return _value_text(value)
    words = _enum_words(key) or {}
    return f"{value} ({words.get(str(value), 'not in menu_fields.json')})"


def _one_value_text(key: str, value: Any) -> str:
    """One instance's value of one field, the way its kind prints."""
    kind = _kind(key)
    if kind in ARRAY_KINDS:
        if value is None:
            return "null (no array)"
        if not isinstance(value, list):
            return _value_text(value)
        if kind == "string_array":
            return _value_text(value)
        return "[" + ", ".join(_number_text(key, v) for v in value) + "]"
    if kind == ENUM_KIND:
        return _number_text(key, value)
    return _value_text(value)


def _row_text(key: str, rows: Any, index: int) -> str:
    """One row array's element at `index`, or plain text saying why there is
    none."""
    if rows is None:
        return f"none ({key} is null)"
    if not isinstance(rows, list):
        return _value_text(rows)
    if not 0 <= index < len(rows):
        return f"none (index {index} is outside the {len(rows)} {key})"
    value = rows[index]
    if _kind(key) == "string_array":
        # Quoted, so a row name never reads as one of these messages.
        return json.dumps(value, ensure_ascii=False)
    return _number_text(key, value)


def highlighted_row_text(values: Dict[str, Any]) -> str:
    """The highlighted row of one player screen, from one instance's raw
    values by key: its index is cursor + scroll, then that row's name and
    instrument id. The one home of which row is highlighted."""
    cursor, scroll = values.get(CURSOR_KEY), values.get(SCROLL_KEY)
    if not (_is_number(cursor) and _is_number(scroll)):
        return "not known (cursor or scroll was not read)"
    index = cursor + scroll
    return (f"{index}, name {_row_text(ROW_NAMES_KEY, values.get(ROW_NAMES_KEY), index)}, "
            f"id {_row_text(ROW_IDS_KEY, values.get(ROW_IDS_KEY), index)}")


def _nth(values: Any, i: int) -> Any:
    """Instance i's value of one field: the agent sends a list with one value
    per live instance. Anything that is not a list is printed as sent for
    every instance."""
    if not isinstance(values, list):
        return values
    return values[i] if i < len(values) else "not sent"


def _instance_lines(name: str, fields: dict, count: int) -> List[str]:
    """One menu class's fields as lines, per live instance: the plain fields
    on one line in menu_fields.json's order with the highlighted row right
    after scroll, then the arrays on a `rows` line. Fields the agent sent that
    menu_fields.json does not name come last, as sent."""
    listed = [f["key"] for f in menu_fields()["fields"] if f["key"] in fields]
    keys = listed + [k for k in fields if k not in listed]
    lines = []
    for i in range(count):
        values = {k: _nth(fields[k], i) for k in keys}
        plain, rows = [], []
        for key in keys:
            text = f"{key}: {_one_value_text(key, values[key])}"
            if _kind(key) in ARRAY_KINDS:
                rows.append(text)
                continue
            plain.append(text)
            if key == SCROLL_KEY:
                plain.append(f"highlighted row: {highlighted_row_text(values)}")
        lines.append(f"{name}[{i}]: " + "; ".join(plain))
        if rows:
            lines.append(f"{name}[{i}] rows: " + "; ".join(rows))
    return lines


def menu_lines(menu: Optional[dict]) -> List[str]:
    """state.ts's menu block as lines: each menu class with its live and
    destroyed counts, each live instance's active read, and for the menus that
    hold one each live instance's onlineMenuActive read; then, for a class
    with fields, each live instance's fields (_instance_lines); then the
    selected song and GlobalVariables' fields. The agent's values are printed
    as sent, plus the highlighted row (highlighted_row_text); this names no
    menu state of its own."""
    if menu is None:
        return ["menu: not read (the agent sent no menu block)"]
    if "error" in menu:
        return [f"menu: {menu['error']}"]
    lines: List[str] = []
    for name, entry in menu.get("classes", {}).items():
        if not entry.get("found"):
            lines.append(f"{name}: {entry.get('reason')}")
            continue
        line = f"{name}: {entry['live']} live, {entry['destroyed']} destroyed"
        if "active" in entry:
            line += f"; active: {_value_text(entry['active'])}"
        if "onlineMenuActive" in entry:
            line += f"; onlineMenuActive: {_value_text(entry['onlineMenuActive'])}"
        lines.append(line)
        fields = entry.get("fields")
        if isinstance(fields, dict):
            lines.extend(_instance_lines(name, fields, entry["live"]))
    song = menu.get("selectedSong")
    if song is None:
        lines.append("selected song: none")
    else:
        lines.append(f"selected song folder: {_value_text(song.get('folderPath'))}")
        lines.append(f"selected song chartName: {_value_text(song.get('chartName'))}")
    globals_ = menu.get("globals", {})
    if not globals_.get("found"):
        lines.append(f"GlobalVariables: {globals_.get('reason')}")
    else:
        for key in ("currentSongFolder", "gameMode", "playerCount"):
            lines.append(f"GlobalVariables {key}: {_value_text(globals_.get(key))}")
    return lines


class FridaTools:
    """The eight Frida tools, read_state for menu_tools, and the session
    state between them.

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

    def ch_menu(self) -> str:
        """One fresh read, printed as the menu block (menu_lines), then the
        live-GameManager word from the same read."""
        with self._lock:
            lines: List[str] = []
            snapshot = self._read(lines)
            if snapshot is None:
                return self._reply(lines)
            lines.extend(menu_lines(snapshot.get(MENU_BLOCK)))
            lines.append(f"state: {state_words(snapshot)}")
            return self._reply(lines)

    def is_attached(self) -> bool:
        """Whether the state agent is attached and its session is alive, for
        tools that need an attach before they start (method_watch)."""
        with self._lock:
            return self._agent is not None and self._agent.alive

    def read_state(self) -> dict:
        """One fresh read as the raw dict state.ts sent, menu block included,
        for menu_tools. Raises StateUnread with the reason when no read came
        back."""
        with self._lock:
            lines: List[str] = []
            snapshot = self._read(lines)
            if snapshot is None:
                raise StateUnread(self._reply(lines))
            return snapshot

    # -- the trace ----------------------------------------------------------------------

    def ch_trace_start(self, song_folder: str, score_json: Optional[str] = None,
                       with_dynamics: bool = False) -> str:
        """Hook the engine method that runs once per hit note (and the
        dynamics one with with_dynamics=True) in a background trace, after
        one state read shows a live GameManager."""
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
