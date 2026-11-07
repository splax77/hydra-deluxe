"""The method watch: hook the menu and song-start methods that
start_candidates.json names, log every call, and show which call came just
before each scene load (the plan
docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md, task
T7 and live step LS-M).

`MethodWatch` holds the two tools, ch_method_watch_start and
ch_method_watch_stop. The start runs agent/menuhooks.js through
frida_attach.run_agent in a background thread, the way
trace_hits.TraceSession runs the trace agent, and replies with what was
hooked and what could not be found. The stop ends the run and lists every
call in order.

Who owns what: start_candidates.json owns which classes and methods are
watched; menuhooks.ts finds them on the live classes and reads each call's
arguments; this module turns the file into the agent's settings and the calls
into reply lines. This module also owns which code addresses an "all" class
leaves unhooked (`shared_rvas`), because it has the dump's facts for every
class and the live agent sees only the classes it watches. The build check is
the attach's (frida_tools.FridaTools), so the watch refuses until something is
attached.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel.
"""

from __future__ import annotations

import json
import os
import threading
import time
from typing import Any, Callable, Iterable, List, Optional

from tools.ch_mcp import build_facts
from tools.ch_mcp import frida_attach as FA
from tools.ch_mcp import frida_tools as FT
from tools.ch_mcp import trace_hits as TH

_HERE = os.path.dirname(os.path.abspath(__file__))

CANDIDATES_FILE = os.path.join(_HERE, "start_candidates.json")

AGENT_NAME = "menuhooks"
AGENT_FILE = f"{AGENT_NAME}.js"

# The watch's default wall-clock cap: the plan's ch_method_watch_start
# (cap_s=120) and live step LS-M ("cap 120 s").
METHOD_WATCH_CAP_S = 120.0

# How a run ended when ch_method_watch_stop ended it, beside frida_attach's
# END_* reasons.
END_STOPPED = "stopped"
END_FAILED = "failed"

# The hook kinds start_candidates.json may give a class; its "hook_kinds"
# says what each means, and menuhooks.ts acts on them.
HOOK_ALL = "all"
HOOK_LISTED = "listed"
HOOK_BY_NAME = "by_name"
HOOK_KINDS = (HOOK_ALL, HOOK_LISTED, HOOK_BY_NAME)


class CandidatesError(Exception):
    """start_candidates.json is missing, unreadable or malformed. The message
    is the line to show."""


# ---- the candidate file ----------------------------------------------------------

def parse_rva(text: str) -> int:
    """An RVA as the candidate file writes it, a hex string like "0x8D090"."""
    if not isinstance(text, str) or not text.lower().startswith("0x"):
        raise CandidatesError(f"an RVA must be a hex string like 0x8D090, not {text!r}")
    return int(text, 16)


def rva_text(rva: int | str) -> str:
    """One way to print an RVA, whether it came from the file or the agent."""
    value = rva if isinstance(rva, int) else int(rva, 16)
    return f"0x{value:X}"


def load_candidates(path: str = CANDIDATES_FILE) -> dict:
    """The candidate file, checked for the shape this module and menuhooks.ts
    read. Raises CandidatesError naming what is wrong."""
    try:
        with open(path, encoding="utf-8") as fh:
            data = json.load(fh)
    except (OSError, ValueError) as e:
        raise CandidatesError(f"cannot read {path}: {type(e).__name__}: {e}") from e
    classes = data.get("classes") if isinstance(data, dict) else None
    if not isinstance(classes, list) or not classes:
        raise CandidatesError(f"{path} holds no classes list")
    for c in classes:
        name = c.get("class")
        if not isinstance(name, str) or not name:
            raise CandidatesError(f"{path}: a class entry has no class name")
        if c.get("hook") not in HOOK_KINDS:
            raise CandidatesError(f"{path}: {name} has hook {c.get('hook')!r}, "
                                  f"not one of {', '.join(HOOK_KINDS)}")
        if "assembly" in c and (not isinstance(c["assembly"], str) or not c["assembly"]):
            raise CandidatesError(f"{path}: {name} has assembly {c['assembly']!r}, "
                                  "not an assembly name")
        for m in c.get("methods", []):
            parse_rva(m.get("rva"))
        if c["hook"] == HOOK_LISTED and not c.get("methods"):
            raise CandidatesError(f"{path}: {name} is listed but lists no methods")
        if c["hook"] == HOOK_BY_NAME and not c.get("names"):
            raise CandidatesError(f"{path}: {name} is by_name but names no methods")
    return data


def full_name(entry: dict) -> str:
    """The class's namespace and name, as an image looks it up."""
    namespace = entry.get("namespace") or ""
    return f"{namespace}.{entry['class']}" if namespace else entry["class"]


def shared_rvas(facts: dict) -> frozenset[int]:
    """The code addresses that methods of more than one class run at, from
    the dump's facts (build_facts.load_facts). The compiler folds identical
    bodies into one, so a hook on such an address fires for every class that
    shares it and buries the watched class's own calls (PlayerSelectionCanvas
    .ctor at 0x66940 in build 6142, for one). A class is told apart by its
    TypeDefIndex, so two methods of one class at one address are not shared.
    The addresses of generic method instantiations are left out, because the
    facts do not carry them (dump_facts.parse_dump keeps them apart)."""
    owners: dict[int, set[int]] = {}
    for cls in facts["classes"]:
        for m in cls["methods"]:
            if m["rva"] != -1:  # an abstract method has no code
                owners.setdefault(m["rva"], set()).add(cls["typedef_index"])
    return frozenset(rva for rva, held in owners.items() if len(held) > 1)


def facts_class(facts: dict, entry: dict) -> dict:
    """The one class of the facts that a candidate entry names, by name and
    namespace. Raises CandidatesError when there is not exactly one."""
    found = [c for c in facts["classes"]
             if c["name"] == entry["class"]
             and (c.get("namespace") or "") == (entry.get("namespace") or "")]
    if len(found) != 1:
        raise CandidatesError(f"{full_name(entry)} matched {len(found)} classes in the "
                              "dump's facts")
    return found[0]


def skip_rvas(facts: dict, entry: dict, shared: frozenset[int]) -> List[int]:
    """The addresses an entry's class leaves unhooked: for an "all" class,
    each of its own addresses in `shared`. A listed or by_name class names
    its methods, so none of them is skipped."""
    if entry["hook"] != HOOK_ALL:
        return []
    return sorted({m["rva"] for m in facts_class(facts, entry)["methods"]} & shared)


def watch_settings(candidates: dict, facts: dict,
                   classes: Optional[Iterable[str]] = None) -> List[dict]:
    """The class list menuhooks.ts gets in its start message: every class in
    the file, or only those `classes` names, each with the addresses it skips
    in the dump's `facts`. Raises CandidatesError on a name the file does not
    hold, or an "all" class the facts do not hold once."""
    entries = candidates["classes"]
    if classes is not None:
        wanted = list(classes)
        known = [e["class"] for e in entries]
        unknown = [n for n in wanted if n not in known]
        if unknown:
            raise CandidatesError(f"not in start_candidates.json: {', '.join(unknown)}; "
                                  f"the classes are {', '.join(known)}")
        entries = [e for e in entries if e["class"] in wanted]
    shared = shared_rvas(facts)
    return [{
        "label": e["class"],
        "full_name": full_name(e),
        "assembly": e.get("assembly"),
        "hook": e["hook"],
        "rvas": ([parse_rva(m["rva"]) for m in e.get("methods", [])]
                 if e["hook"] == HOOK_LISTED else []),
        "names": ([n["name"] for n in e.get("names", [])]
                  if e["hook"] == HOOK_BY_NAME else []),
        "skip_rvas": skip_rvas(facts, e, shared),
    } for e in entries]


def scene_load_classes(candidates: dict) -> set[str]:
    """The classes the file marks as scene loaders."""
    return {e["class"] for e in candidates["classes"] if e.get("scene_load")}


# ---- reply lines ---------------------------------------------------------------------

def _args_text(args: Any) -> str:
    return json.dumps(args, ensure_ascii=False)


def call_label(call: dict) -> str:
    return f"{call.get('class')} {' / '.join(call.get('names', []))} ({rva_text(call['rva'])})"


def start_lines(summary: dict) -> List[str]:
    """The agent's hook summary as reply lines: the count, each class's count,
    every class, RVA or name it could not find, and every address it left
    unhooked because other classes share its code."""
    lines = []
    if "error" in summary:
        lines.append(f"nothing hooked: {summary['error']}")
    else:
        lines.append(f"hooked: {summary.get('hooked', 0)} code addresses")
    for c in summary.get("classes", []):
        lines.append(f"  {c['class']}: {c['hooked']}")
    for line in summary.get("missing", []):
        lines.append(f"not found: {line}")
    for line in summary.get("no_code", []):
        lines.append(f"no code: {line}")
    for line in summary.get("not_hooked", []):
        lines.append(f"not hooked: {line}")
    for s in summary.get("skipped", []):
        lines.append(f"skipped (code shared with other classes): {s['class']} "
                     f"{' / '.join(s['names'])} {rva_text(s['rva'])}")
    return lines


def call_lines(calls: List[dict]) -> List[str]:
    """Every call in order, with its wall time since the first call."""
    if not calls:
        return ["calls: none"]
    first = calls[0].get("t_ms", 0)
    lines = [f"calls: {len(calls)}"]
    for i, call in enumerate(calls, start=1):
        lines.append(f"  {i}. {call_label(call)} args {_args_text(call.get('args', []))}; "
                     f"songTime {_args_text(call.get('songTime'))}; "
                     f"+{call.get('t_ms', 0) - first:.0f} ms")
    return lines


def scene_load_lines(calls: List[dict], scene_classes: set[str]) -> List[str]:
    """For each scene-load call, its scene argument and the last call before
    it that was not itself a scene load (one load can call another)."""
    loads = [i for i, c in enumerate(calls) if c.get("class") in scene_classes]
    if not loads:
        return ["scene loads: none seen"]
    lines = ["scene loads:"]
    for i in loads:
        call = calls[i]
        args = call.get("args", [])
        scene = _args_text(args[0]) if args else "no argument"
        before = next((j for j in range(i - 1, -1, -1)
                       if calls[j].get("class") not in scene_classes), None)
        if before is None:
            prior = "no other call before it"
        else:
            prior = (f"last before it: call {before + 1}, {call_label(calls[before])}, "
                     f"args {_args_text(calls[before].get('args', []))}")
        lines.append(f"  call {i + 1}: {call_label(call)} scene {scene}; {prior}")
    return lines


# ---- the run --------------------------------------------------------------------------

class WatchSession:
    """One menuhooks run in a background thread. Every call it sees goes to
    `calls_path`, one JSON line each; run_agent logs every message the agent
    sends to `agent_log_path`. The stop mirrors trace_hits.TraceSession's:
    run_agent's wait returns when stop() is called, so run_agent detaches,
    and that removes every hook."""

    def __init__(self, agent_path: str, settings: List[dict], cap_s: float,
                 calls_path: str, agent_log_path: str, *,
                 run_agent: Callable[..., FA.RunResult] = FA.run_agent,
                 frida_mod: Any = None,
                 wait: Callable[[threading.Event, float], bool] = FA._default_wait,
                 now: Callable[[], float] = time.monotonic) -> None:
        self.agent_path = agent_path
        self.settings = settings
        self.cap_s = cap_s
        self.calls_path = calls_path
        self.agent_log_path = agent_log_path
        # The runner's own lines (the cap, a refused detach). Kept, not
        # printed: an MCP server's stdout is its protocol channel.
        self.lines: List[str] = []
        self.result: Optional[FA.RunResult] = None
        self.summary: Optional[dict] = None
        self.summary_seen = threading.Event()
        self._run_agent = run_agent
        self._frida_mod = frida_mod
        self._wait = wait
        self._now = now
        self._calls: List[dict] = []
        self._lock = threading.Lock()
        self._log: Any = None
        self._stop = threading.Event()
        self._stopped = False
        self._done = threading.Event()
        self._thread = threading.Thread(target=self._run, name="method-watch",
                                        daemon=True)

    def start(self) -> "WatchSession":
        self._log = open(self.calls_path, "w", encoding="utf-8")
        self._thread.start()
        return self

    def _on_message(self, payload: Any) -> Optional[str]:
        if isinstance(payload, dict):
            if "call" in payload:
                with self._lock:
                    self._calls.append(payload)
                    if self._log is not None and not self._log.closed:
                        self._log.write(json.dumps(payload, ensure_ascii=False) + "\n")
                        self._log.flush()
            elif "hooked" in payload or "error" in payload:
                if self.summary is None:
                    self.summary = payload
                    # An error ends the run, and _run sets the event once the
                    # result is in, so the start reply can say how it ended.
                    if "hooked" in payload:
                        self.summary_seen.set()
        return FA.ends_run(payload)

    def _run_wait(self, ended: threading.Event, cap_s: float) -> bool:
        """run_agent's wait: True when the run ended or stop() was called,
        False when `cap_s` passed first (trace_hits.TraceSession's shape)."""
        deadline = self._now() + cap_s
        while True:
            if ended.is_set():
                return True
            if self._stop.is_set():
                self._stopped = True
                ended.set()
                return True
            remaining = deadline - self._now()
            if remaining <= 0:
                return False
            self._wait(self._stop, min(remaining, FA._INTERRUPT_SLICE_S))

    def _run(self) -> None:
        try:
            result = self._run_agent(self.agent_path, self.cap_s, self._on_message,
                                     self.agent_log_path,
                                     start={"classes": self.settings},
                                     frida_mod=self._frida_mod,
                                     wait=self._run_wait, out=self.lines.append)
            if self._stopped and result.reason == FA.END_CAP:
                result.reason = END_STOPPED
        except BaseException as e:  # noqa: BLE001 - a thread has no caller
            result = FA.RunResult(reason=END_FAILED, cap_s=self.cap_s,
                                  error=f"{type(e).__name__}: {e}")
        finally:
            with self._lock:
                if self._log is not None:
                    self._log.close()
        self.result = result
        self._done.set()
        # A run that ended before its summary came must not leave the start
        # waiting for it.
        self.summary_seen.set()

    def stop(self, cap_s: float) -> bool:
        """Ask the run to end and wait at most `cap_s` for it to detach. True
        when the run has ended."""
        self._stop.set()
        if self._thread.is_alive():
            self._thread.join(cap_s)
        return self.ended()

    def ended(self) -> bool:
        return self._done.is_set()

    def calls(self) -> List[dict]:
        with self._lock:
            return list(self._calls)

    def ended_line(self) -> str:
        if self.result is None:
            return "ended: not yet; the watch is still running"
        line = f"ended: {self.result.reason}"
        if self.result.reason == FA.END_CAP:
            line += f" (the watch's cap of {self.cap_s:g} s)"
        if self.result.error is not None:
            line += f"; error: {self.result.error}"
        return line


class MethodWatch:
    """ch_method_watch_start and ch_method_watch_stop, and the one watch
    between them.

    `game` is the server's game_tools.GameTools (the tests pass a fake): the
    watch takes its backup, writes its logs in its session folder, and stops
    with it through a stop hook. `frida` is the server's FridaTools; the
    watch refuses until it is attached. `load_facts` (build_facts') gives the
    installed build's facts, which shared_rvas reads at each start. Every
    other seam is an argument the tests swap for a fake."""

    def __init__(self, game, frida, lock, *,
                 now: Callable[[], float] = time.monotonic,
                 run_agent: Callable[..., FA.RunResult] = FA.run_agent,
                 frida_mod: Any = None,
                 wait: Callable[[threading.Event, float], bool] = FA._default_wait,
                 agent_dir: str = FT.AGENT_DIR,
                 candidates_path: str = CANDIDATES_FILE,
                 load_facts: Callable[[], dict] = build_facts.load_facts) -> None:
        self._game = game
        self._frida = frida
        self._lock = lock
        self._load_facts = load_facts
        self._now = now
        self._run_agent = run_agent
        self._frida_mod = frida_mod
        self._wait = wait
        self._agent_dir = agent_dir
        self._candidates_path = candidates_path
        self._session: Optional[WatchSession] = None
        self._scene_classes: set[str] = set()
        self._count = 0
        game.add_stop_hook(self._stop_hook)

    def ch_method_watch_start(self, classes: Optional[List[str]] = None,
                              cap_s: float = METHOD_WATCH_CAP_S) -> str:
        """Hook every method start_candidates.json names (or only those of
        `classes`) for at most `cap_s` seconds, and say what was hooked and
        what could not be found."""
        with self._lock:
            if self._session is not None and not self._session.ended():
                return "a method watch is already running; call ch_method_watch_stop first"
            if not self._frida.is_attached():
                return f"{FT.NOT_ATTACHED}; no method watch started"
            if cap_s <= 0:
                return f"cap_s must be more than 0 seconds, not {cap_s:g}"
            try:
                candidates = load_candidates(self._candidates_path)
                settings = watch_settings(candidates, self._load_facts(), classes)
            except (CandidatesError, build_facts.BuildFactsError) as e:
                return f"{e}\nno method watch started"
            agent_path = os.path.join(self._agent_dir, AGENT_FILE)
            if not os.path.isfile(agent_path):
                return TH.agent_missing_line(agent_path, AGENT_NAME)
            lines: List[str] = []
            # The hooks are the first touch of the game, so the backup comes
            # after every refusal above and just before them.
            backup = self._game.ensure_backup()
            if backup:
                lines.append(backup)
            self._count += 1
            folder = self._game.session_dir()
            calls_path = os.path.join(folder, f"method_watch-{self._count}.jsonl")
            agent_log = os.path.join(folder, f"method_watch-{self._count}-agent.jsonl")
            self._scene_classes = scene_load_classes(candidates)
            session = WatchSession(agent_path, settings, cap_s, calls_path, agent_log,
                                   run_agent=self._run_agent, frida_mod=self._frida_mod,
                                   wait=self._wait, now=self._now)
            self._session = session.start()
            lines.append(f"method watch started on {len(settings)} classes: "
                         + ", ".join(s["label"] for s in settings))
            lines.append(f"calls log: {calls_path}")
            lines.append(f"agent log: {agent_log}")
            lines.append(f"cap: {cap_s:g} s of wall clock")
            self._wait(session.summary_seen, FT.ATTACH_CAP_S)
            if session.summary is not None:
                lines.extend(start_lines(session.summary))
            elif not session.ended():
                lines.append(f"no hook summary within {FT.ATTACH_CAP_S:g} s (ATTACH_CAP_S); "
                             "the watch keeps running and ch_method_watch_stop ends it")
            if session.ended():
                lines.append(session.ended_line())
                lines.extend(session.lines)
            return "\n".join(lines)

    def ch_method_watch_stop(self) -> str:
        """End the watch, waiting at most READ_CAP_S for it to detach, then
        name the call before each scene load and list every call in order.
        The call list goes last, so a cut reply keeps the scene loads."""
        with self._lock:
            session = self._session
            if session is None:
                return "no method watch has run; call ch_method_watch_start"
            return "\n".join(self._stop_lines(session))

    def _stop_lines(self, session: WatchSession) -> List[str]:
        lines: List[str] = []
        if not session.ended() and not session.stop(FT.READ_CAP_S):
            lines.append(f"the watch thread was still running after {FT.READ_CAP_S:g} s "
                         "(READ_CAP_S); it ends with the server")
        else:
            lines.append(session.ended_line())
        if session.summary is not None:
            lines.extend(start_lines(session.summary))
        calls = session.calls()
        lines.extend(scene_load_lines(calls, self._scene_classes))
        lines.append(f"calls log: {session.calls_path}")
        lines.extend(session.lines)
        lines.extend(call_lines(calls))
        return lines

    def _stop_hook(self) -> Optional[str]:
        """game_tools' stop hook: end a running watch."""
        with self._lock:
            session = self._session
            if session is None or session.ended():
                return None
            if session.stop(FT.READ_CAP_S):
                return f"method watch: {session.ended_line()}, {len(session.calls())} calls"
            return (f"method watch: still running after {FT.READ_CAP_S:g} s "
                    "(READ_CAP_S); it ends with the server")
