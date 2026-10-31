"""The Clone Hero MCP server's Star Power tools: watch the engine's Star
Power fields while a song plays, and answer questions about the rows.

The read lives in ch_probe and nothing here reads memory (the plan
docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md, "Where
the Star Power read lives, and who calls it"). engine.decode_snapshot says
where each field is, EngineModel.snapshot reads it, and sp_watch.watch polls
it and decides when a run stops. sp_compare owns every question about the
rows: activations, rises, flips inside a window and the ready level. This
module adds only the watch thread, the rows and latest read it keeps in
memory, the CSV it writes as it goes, and sp_lines, the one place a Snapshot
becomes text.

The watch runs in a daemon thread that never takes the server's lock, so a
tool holding the lock never waits on it. ch_sp prints the latest read the
thread made, so it never touches the game from a second thread and never
waits on the engine search.

Every tool returns plain text and never prints: an MCP server's stdout is its
protocol channel.
"""

from __future__ import annotations

import csv
import dataclasses
import os
import threading
import time
from typing import Callable, List, Optional

from tools.ch_mcp import bot_run
from tools.ch_mcp.frida_tools import READ_CAP_S
from tools.ch_probe import constants as C
from tools.ch_probe import engine_finder
from tools.ch_probe.engine import EngineModel, Snapshot
from tools.ch_probe.experiments import sp_compare, sp_watch
from tools.ch_probe.experiments.overlay_logs import WATCH_COLUMNS
from tools.ch_probe.process import open_process

# The amount and maximum columns sp_compare.compare reads at its default
# width; the server reads the same ones.
AMOUNT_COL = f"sp_amount_u{sp_compare.DEFAULT_WIDTH}"
MAX_COL = f"sp_max_u{sp_compare.DEFAULT_WIDTH}"

CSV_PREFIX = "sp_watch-"

NO_WATCH = "no watcher has run; call ch_sp_watch_start"
NO_READ = "no engine read yet (the watcher is waiting for a moving clock)"


def _ms_text(s: float) -> str:
    """Seconds on the game's clock as ms text."""
    return f"{C.s_to_ms(s):.1f} ms"


def _row(snap: Snapshot) -> dict:
    """A snapshot as a row in the shape sp_compare reads: the watch row's
    columns, with no wall time."""
    return dict(zip(WATCH_COLUMNS, sp_watch.watch_row(None, snap)))


def sp_lines(snap: Snapshot) -> List[str]:
    """One read of the Star Power fields as lines: the engine clock, the
    amount and the maximum, the amount as a share of the maximum and whether
    it is at the ready level (sp_compare.at_ready_level), the flag, the end
    time and how far ahead of the clock it is, the multiplier and the
    overflow flag."""
    row = _row(snap)
    amount, sp_max = row[AMOUNT_COL], row[MAX_COL]
    lines = [f"engine clock: {_ms_text(snap.clock_s)}",
             f"amount: {amount} ({AMOUNT_COL})",
             f"max: {sp_max} ({MAX_COL})"]
    if sp_max > 0:
        ready = sp_compare.at_ready_level(amount, sp_max)
        lines.append(f"share of max: {amount / sp_max:.3f}")
        lines.append(f"ready: {'yes, at or over' if ready else 'no, below'} "
                     f"{sp_compare.READY_FRACTION:g} of max (sp_compare.READY_FRACTION)")
    else:
        lines.append("share of max: none (max is 0)")
        lines.append("ready: unknown (max is 0)")
    on = sp_compare._active(row)
    lines.append(f"active flag: {row['sp_active_byte']} ({'on' if on else 'off'})")
    lines.append(f"end time: {_ms_text(snap.sp_end_s)}, "
                 f"{_ms_text(snap.sp_end_s - snap.clock_s)} ahead of the clock")
    lines.append(f"multiplier: {row['sp_mult_u32']}")
    lines.append(f"overflow flag: {row['sp_overflow_u32']}")
    return lines


@dataclasses.dataclass
class _Watch:
    """One ch_sp_watch_start run. The thread appends to `rows` (dicts keyed
    by WATCH_COLUMNS, as sp_compare.load_watch_rows gives them) and sets
    `latest` on every read; `summary` or `error` is set when it ends."""
    csv_path: str
    cap_s: float
    thread: Optional[threading.Thread] = None
    stop: threading.Event = dataclasses.field(default_factory=threading.Event)
    rows: list = dataclasses.field(default_factory=list)
    latest: Optional[Snapshot] = None
    engine: Optional[int] = None
    summary: Optional[sp_watch.WatchSummary] = None
    # Why the run ended without a summary: the engine search's message.
    no_engine: Optional[str] = None
    error: Optional[str] = None
    csv_written: bool = False

    def running(self) -> bool:
        return self.thread is not None and self.thread.is_alive()

    def ended_line(self) -> str:
        if self.running():
            return f"running: {len(self.rows)} rows so far"
        if self.error is not None:
            return f"ended: failed: {self.error}"
        if self.no_engine is not None:
            return f"ended: {self.no_engine}"
        if self.summary is not None:
            s = self.summary
            return (f"ended: {s.reason}; rows written {s.rows}; first clock "
                    f"{_ms_text(s.first_clock_s)}, last clock {_ms_text(s.last_clock_s)}")
        return "ended: no result"

    def csv_line(self) -> str:
        if self.csv_written:
            return f"csv: {self.csv_path}"
        return f"csv: {self.csv_path} (not written: the engine was never found)"


def _engine_reader(proc, engine_ptr: int) -> Callable[[], Snapshot]:
    """The live engine's snapshot read. LIVE-ONLY: reads the game's memory
    through EngineModel."""
    model = EngineModel(proc)
    model.use_object(engine_ptr)
    return model.snapshot


class SpTools:
    """The six Star Power tools and the watch between them.

    `game` is the server's game_tools.GameTools (the tests pass a fake with
    ensure_backup, session_dir, add_stop_hook and run_cap_s). Every outside
    seam is an argument, so the tests swap each for a fake: `opener`
    (open_process), `find_engine` (sp_watch's capped engine search), `reader`
    (turns the open process and the engine's address into a snapshot read),
    `watch` (sp_watch.watch), and `now` and `sleep` (the watch's wall clock,
    which the CSV's wall_s column is on). `lock` is the server's one RLock."""

    def __init__(self, game, lock, *,
                 opener: Callable[..., object] = open_process,
                 find_engine: Callable[[object], int] = sp_watch.find_engine_live,
                 reader: Callable[[object, int], Callable[[], Snapshot]] = _engine_reader,
                 watch: Callable[..., sp_watch.WatchSummary] = sp_watch.watch,
                 now: Callable[[], float] = time.time,
                 sleep: Callable[[float], None] = time.sleep):
        self._game = game
        self._lock = lock
        self._opener = opener
        self._find_engine = find_engine
        self._reader = reader
        self._watch_fn = watch
        self._now = now
        self._sleep = sleep
        self._watch: Optional[_Watch] = None
        self._count = 0
        game.add_stop_hook(self._stop_hook)

    # -- the watch ------------------------------------------------------------------

    def ch_sp_watch_start(self, song_folder: Optional[str] = None,
                          out: Optional[str] = None) -> str:
        """Take the backup, then start the watch thread: open the game
        read-only, check the build, find the engine and watch it until the
        song stops, the run cap, or ch_sp_watch_stop. `out` is the CSV; a
        relative path goes in the session folder, and none means a new file
        there."""
        with self._lock:
            if self._watch is not None and self._watch.running():
                return "a Star Power watch is already running; call ch_sp_watch_stop first"
            if song_folder is None:
                cap_s = sp_watch.RUN_CAP_S
                cap_line = f"run cap: {cap_s:g} s (sp_watch.RUN_CAP_S; no song folder given)"
            else:
                try:
                    cap_s = self._game.run_cap_s(song_folder)
                except (OSError, ValueError, bot_run.SongLengthMissing) as e:
                    return (f"no watch started: could not pick the run cap for "
                            f"{song_folder}: {type(e).__name__}: {e}")
                cap_line = f"run cap: {cap_s:g} s (game_tools.run_cap_s for {song_folder})"
            lines = [line for line in (self._game.ensure_backup(),) if line]
            self._count += 1
            if out:
                csv_path = out if os.path.isabs(out) else os.path.join(self._game.session_dir(), out)
            else:
                csv_path = os.path.join(self._game.session_dir(),
                                        f"{CSV_PREFIX}{self._count}.csv")
            w = _Watch(csv_path=csv_path, cap_s=cap_s)
            w.thread = threading.Thread(target=self._body, args=(w,),
                                        name="ch_sp_watch", daemon=True)
            self._watch = w
            w.thread.start()
            lines.append("watching: waiting for a moving clock (cap "
                         f"{engine_finder.ENGINE_SEARCH_CAP_S:g} s, "
                         "engine_finder.ENGINE_SEARCH_CAP_S)")
            lines.append(f"csv: {csv_path}")
            lines.append(cap_line)
            return "\n".join(lines)

    def ch_sp(self) -> str:
        """The latest read the watcher made, as sp_lines, then the watch's
        state. It never reads the game itself."""
        with self._lock:
            w = self._watch
            if w is None:
                return NO_WATCH
            snap = w.latest
            if snap is None:
                lines = [NO_READ]
            else:
                lines = sp_lines(snap)
            lines.append("watcher " + w.ended_line())
            return "\n".join(lines)

    def ch_sp_watch_status(self, since_ms: float = 0) -> str:
        """The rows so far, each activation and each rise of the amount from
        since_ms on (sp_compare.find_activations and phrase_rises), the
        engine's address, and how the watch ended."""
        with self._lock:
            w = self._watch
            if w is None:
                return NO_WATCH
            rows = list(w.rows)
            lines = [f"watcher {w.ended_line()}", w.csv_line(),
                     f"rows: {len(rows)}",
                     "engine: " + (f"{w.engine:#x}" if w.engine is not None
                                   else "not found yet")]
            acts = [a for a in sp_compare.find_activations(rows)
                    if C.s_to_ms(rows[a.flip_row]["clock_s"]) >= since_ms]
            lines.append(f"activations from {since_ms:g} ms: {len(acts)}")
            lines.extend(_activation_line(rows, a) for a in acts)
            rises = [r for r in sp_compare.phrase_rises(rows, AMOUNT_COL) if r.ms >= since_ms]
            lines.append(f"rises of {AMOUNT_COL} from {since_ms:g} ms: {len(rises)}")
            lines.extend(f"  at {r.ms:.1f} ms: +{r.size} ({r.before} -> {r.after})"
                         for r in rises)
            return "\n".join(lines)

    def ch_sp_watch_stop(self) -> str:
        """Tell the watch to stop at its next poll, wait at most READ_CAP_S
        for it, then show how it ended and the CSV."""
        with self._lock:
            w = self._watch
            if w is None:
                return NO_WATCH
            return "\n".join(self._stop_lines(w))

    # -- questions about rows -------------------------------------------------------

    def ch_sp_compare(self, watch_csv: str, score_json: str,
                      resolution: Optional[int] = None,
                      sp_bar_ticks: Optional[int] = None,
                      width: int = sp_compare.DEFAULT_WIDTH) -> str:
        """sp_compare.compare's report for a watcher CSV against a
        hydra_replay score JSON."""
        with self._lock:
            try:
                rows = sp_compare.load_watch_rows(watch_csv)
                score = sp_compare.load_score(score_json)
                report = sp_compare.compare(rows, score, resolution=resolution,
                                            sp_bar_ticks=sp_bar_ticks, width=width)
            except (OSError, ValueError, KeyError, TypeError) as e:
                return f"compare failed: {type(e).__name__}: {e}"
            return report.text()

    def ch_sp_between(self, start_ms: float, end_ms: float) -> str:
        """Over the rows in memory: each activation whose flip lies in the
        window (sp_compare.flips_between), the highest amount read there, and
        the first ready row with the flag clear (sp_compare.ready_between)."""
        with self._lock:
            w = self._watch
            if w is None:
                return NO_WATCH
            rows = list(w.rows)
            lines = [f"window: {start_ms:.1f} ms to {end_ms:.1f} ms, over {len(rows)} rows "
                     f"(watcher {w.ended_line()})"]
            flips = sp_compare.flips_between(rows, start_ms, end_ms)
            if flips:
                lines.append(f"activations inside: {len(flips)}")
                lines.extend(_activation_line(rows, a) for a in flips)
            else:
                lines.append("no activation inside")
            ready = sp_compare.ready_between(rows, start_ms, end_ms, AMOUNT_COL, MAX_COL)
            if ready.rows == 0:
                lines.append("no rows inside the window")
                return "\n".join(lines)
            lines.append(f"rows inside: {ready.rows}; highest {AMOUNT_COL}: {ready.highest}; "
                         f"{MAX_COL}: {ready.sp_max}")
            if ready.ready_row is None:
                lines.append("no row inside at the ready level with the flag clear")
            else:
                lines.append(f"first row at the ready level with the flag clear: "
                             f"{ready.ready_ms:.1f} ms ({AMOUNT_COL} "
                             f"{ready.ready_row[AMOUNT_COL]})")
            return "\n".join(lines)

    # -- inside ---------------------------------------------------------------------

    def _body(self, w: _Watch) -> None:
        """The watch thread. It never takes the lock."""
        try:
            proc = self._opener(read_only=True)
        except Exception as e:  # reported by the status tools
            w.error = f"could not open the game: {type(e).__name__}: {e}"
            return
        try:
            proc.verify_targets()
            try:
                engine_ptr = self._find_engine(proc)
            except engine_finder.EngineSearchTimeout as e:
                w.no_engine = str(e)
                return
            w.engine = engine_ptr
            take_snapshot = self._reader(proc, engine_ptr)

            def snapshot() -> Snapshot:
                snap = take_snapshot()
                w.latest = snap
                return snap

            parent = os.path.dirname(w.csv_path)
            if parent:
                os.makedirs(parent, exist_ok=True)
            with open(w.csv_path, "w", newline="", encoding="utf-8") as f:
                writer = csv.writer(f)
                writer.writerow(WATCH_COLUMNS)
                w.csv_written = True

                def write_row(row: list) -> None:
                    writer.writerow(row)
                    f.flush()
                    w.rows.append(dict(zip(WATCH_COLUMNS, row)))

                w.summary = self._watch_fn(snapshot, write_row, now=self._now,
                                           sleep=self._sleep, cap_s=w.cap_s,
                                           should_stop=w.stop.is_set)
        except Exception as e:  # reported by the status tools
            w.error = f"{type(e).__name__}: {e}"
        finally:
            proc.close()

    @staticmethod
    def _stop_lines(w: _Watch) -> List[str]:
        w.stop.set()
        if w.thread is not None:
            w.thread.join(READ_CAP_S)
        if w.running():
            return [f"the Star Power watcher was still running after {READ_CAP_S:g} s "
                    "(READ_CAP_S); it ends with the server", w.csv_line()]
        return [f"watcher {w.ended_line()}", w.csv_line()]

    def _stop_hook(self) -> Optional[str]:
        """game_tools' stop hook: stop a running watch."""
        with self._lock:
            w = self._watch
            if w is None or not w.running():
                return None
            return "sp watch: " + "; ".join(self._stop_lines(w))


def _activation_line(rows: list, act) -> str:
    """One activation from sp_compare.find_activations, as the rows show it:
    the flip clock, the multiplier on the row before and on the flip row, the
    end time on the flip row, and when the flag cleared."""
    flip = rows[act.flip_row]
    before = rows[act.flip_row - 1]
    if act.drop_row is None:
        cleared = "still set at the last row"
    else:
        cleared = f"cleared at {_ms_text(rows[act.drop_row]['clock_s'])}"
    return (f"  activation {act.number} at {_ms_text(flip['clock_s'])}: multiplier "
            f"{before['sp_mult_u32']} before, {flip['sp_mult_u32']} on the flip row; "
            f"end time {_ms_text(flip['sp_end_s'])}; flag {cleared}")
