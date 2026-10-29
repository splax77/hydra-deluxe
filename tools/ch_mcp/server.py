"""The MCP server that lets an agent drive Clone Hero as text.

Claude Code starts this server (see .mcp.json at the repo root) and calls its
tools. The plan is docs/superpowers/plans/2026-10-28-ch-mcp-server.md. The
game-side tools live in game_tools.py and the Frida tools in frida_tools.py;
this file only builds them around one shared lock, registers the tools, cuts
each reply and puts the user's files back at exit. build_server() is the one
list of the tools.

Run it as:  py tools/ch_mcp/server.py

Do not add -I: the frida module lives in the user's site-packages, which -I
skips. Nothing here prints to stdout, because stdout is the MCP protocol
channel.
"""

from __future__ import annotations

import atexit
import os
import sys
import threading
from typing import Optional

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402
from tools.ch_mcp.frida_tools import FridaTools  # noqa: E402
from tools.ch_mcp.game_tools import GameTools  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS, cap_reply  # noqa: E402

# The closing line of a cut reply: how to ask for the rest.
CAP_HINT = "ch_notes(since=...) reads a long trace in pieces"


def capped(text: str) -> str:
    """A reply cut at REPLY_CAP_CHARS with the Clone Hero closing line."""
    return cap_reply(text, REPLY_CAP_CHARS, hint=CAP_HINT)


def _to_stderr(line: str) -> None:
    print(line, file=sys.stderr)


class Helper:
    """The one lock, the game-side tools and the Frida tools, sharing it.
    `env` and the two keyword dicts pass the tests' fakes through."""

    def __init__(self, env: Optional[bot_run.Env] = None,
                 scratch_root: Optional[str] = None,
                 game_kw: Optional[dict] = None, frida_kw: Optional[dict] = None):
        self.lock = threading.RLock()
        self.env = env if env is not None else bot_run.Env(out=_to_stderr)
        self.game = GameTools(self.env, self.lock, scratch_root, **(game_kw or {}))
        frida_kw = {"now": self.env.now, "sleep": self.env.sleep, **(frida_kw or {})}
        # GameTools has no public accessor for its session folder; the Frida
        # logs go beside its backups and stats copies.
        self.frida = FridaTools(self.game, self.lock, self.game._session_dir, **frida_kw)

    def ch_status(self) -> str:
        with self.lock:
            return "\n".join([self.game.ch_status()] + self.frida.status_lines())

    def shutdown(self) -> str:
        """Question 3 of the plan, through GameTools.shutdown: its stop hooks
        stop the trace and detach first."""
        return self.game.shutdown()


def build_server(helper: Optional[Helper] = None):
    """The server and its tools. This is the one list of the tools."""
    from mcp.server.mcpserver import MCPServer

    h = helper if helper is not None else Helper()
    server = MCPServer(
        "ch",
        instructions=(
            "Drive Clone Hero as text. ch_start launches the game (with a song folder the"
            " bot plays it; with none the game opens at its menu) after backing up the"
            " user's files; ch_stop closes it and puts them back. ch_attach, ch_state and"
            " ch_snapshot read the game through Frida; ch_trace_start, ch_notes and"
            " ch_trace_stop trace each note it processes. ch_press and ch_play_chart press"
            " drum lanes on a song the user started from the menu."
            " See docs/agents/clone-hero-mcp.md."),
    )

    # -- starting, stopping and status (game_tools) ------------------------------

    @server.tool()
    def ch_start(song_folder: str | None = None, player: str = bot_run.DEFAULT_PLAYER,
                 profile: str | None = None) -> str:
        """Back up the user's game files, then start Clone Hero. With a song
        folder the game plays it with a bot from the command line; with none
        it opens at its menu. A run already open is stopped and restored
        first. The reply names the backup folder, the command line, the pid
        and the song length."""
        return capped(h.game.ch_start(song_folder, player, profile))

    @server.tool()
    def ch_stop() -> str:
        """Stop any trace, Frida session and play session, then close the game
        the server started and restore the user's files, one line per file.
        A game the server did not start stays open; its files are restored."""
        return capped(h.game.ch_stop())

    @server.tool()
    def ch_status() -> str:
        """The run, the backup folder, the play session, the last stats copy,
        whether Frida is attached and whether a trace runs."""
        return capped(h.ch_status())

    @server.tool()
    def ch_install_chart(chart_folder: str, name: str | None = None) -> str:
        """Copy one chart folder into the probe song folder with a silent
        audio file, so the game can launch it."""
        return capped(h.game.ch_install_chart(chart_folder, name))

    @server.tool()
    def ch_wait_stats() -> str:
        """Wait for the bot run to write its end-of-song stats and copy them
        into the session folder; the reply names the copy or the cap."""
        return capped(h.game.ch_wait_stats())

    @server.tool()
    def ch_stats(path: str | None = None) -> str:
        """Every field of a stats copy as name: value lines. No path means the
        last copy ch_wait_stats made."""
        return capped(h.game.ch_stats(path))

    @server.tool()
    def ch_compare(stats_path: str, score_json: str) -> str:
        """Compare a stats copy with a hydra_replay score JSON: three lines,
        then pass or fail."""
        return capped(h.game.ch_compare(stats_path, score_json))

    # -- Frida (frida_tools) --------------------------------------------------------

    @server.tool()
    def ch_attach() -> str:
        """Check the installed build has dumped facts, then attach Frida's
        state agent to the running game and show its first read. A game the
        server did not start is backed up first; one it launched gets a few
        seconds to load before the attach."""
        return capped(h.frida.ch_attach())

    @server.tool()
    def ch_detach() -> str:
        """Detach the state agent from the game."""
        return capped(h.frida.ch_detach())

    @server.tool()
    def ch_state() -> str:
        """One fresh read of the game's state: a word such as menu, playing or
        paused, then the fields it was read from."""
        return capped(h.frida.ch_state())

    @server.tool()
    def ch_snapshot() -> str:
        """One fresh read of the game's clock and flags, plus the score after
        the play session's last press and the song clock at that press when a
        play session has run."""
        return capped(h.frida.ch_snapshot())

    @server.tool()
    def ch_trace_start(song_folder: str, score_json: str | None = None,
                       with_dynamics: bool = False) -> str:
        """Start tracing each note the game processes, in the background. The
        notes come from the song folder's manifest, else from score_json.
        with_dynamics=True also hooks the accent and ghost method. Needs
        ch_attach and a started song."""
        return capped(h.frida.ch_trace_start(song_folder, score_json, with_dynamics))

    @server.tool()
    def ch_notes(since: int = 0) -> str:
        """The traced calls from index since on, each next to its nearest note
        with the time difference, then how many sit close to their note. The
        last line names the since to ask for next."""
        return capped(h.frida.ch_notes(since))

    @server.tool()
    def ch_trace_stop() -> str:
        """Stop the trace and show how it ended and how many calls it saw."""
        return capped(h.frida.ch_trace_stop())

    # -- keys and the auto-player (game_tools) ----------------------------------------

    @server.tool()
    def ch_press(lanes: str) -> str:
        """Press one chord of drum lanes, such as "red,kick", after bringing
        the game window to the front. An unknown name is refused with the
        list of lanes."""
        return capped(h.game.ch_press(lanes))

    @server.tool()
    def ch_play_chart(score_json: str, log: str | None = None) -> str:
        """Play a hydra_replay score JSON with key presses in the background.
        Start the song from the game's menu: a song launched with a bot is
        played by the bot. log, when given, is where the press log goes."""
        return capped(h.game.ch_play_chart(score_json, log))

    @server.tool()
    def ch_play_status() -> str:
        """The play session's counts, its stop reason when it ended, and its
        press rows, newest first."""
        return capped(h.game.ch_play_status())

    @server.tool()
    def ch_play_stop() -> str:
        """Tell the play session to stop and show the result so far."""
        return capped(h.game.ch_play_stop())

    return server


def main() -> None:
    helper = Helper()
    # The restore lines go to stderr, which Claude Code keeps in its MCP log.
    atexit.register(lambda: _to_stderr(helper.shutdown()))
    build_server(helper).run("stdio")


if __name__ == "__main__":
    main()
