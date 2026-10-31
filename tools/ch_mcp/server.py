"""The MCP server that lets an agent drive Clone Hero as text.

Claude Code starts this server (see .mcp.json at the repo root) and calls its
tools. The plans are docs/superpowers/plans/2026-10-28-ch-mcp-server.md and
docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md, and
docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md for the virtual kit.
The game-side tools live in game_tools.py, the Frida tools in frida_tools.py,
the Star Power tools in sp_tools.py, the virtual kit in kit_tools.py, the menu
driver in menu_tools.py and the method watch in method_watch.py; this file only
builds them around one shared lock, registers the tools, cuts each reply and
puts the user's files back at exit. build_server() is the one list of the
tools.

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
from tools.ch_mcp.kit_tools import KitTools, read_pause_setting  # noqa: E402
from tools.ch_mcp.menu_tools import MenuTools  # noqa: E402
from tools.ch_mcp.method_watch import METHOD_WATCH_CAP_S, MethodWatch  # noqa: E402
from tools.ch_mcp.sp_tools import SpTools  # noqa: E402
from tools.ch_probe.experiments import sp_compare  # noqa: E402
from tools.hydra_ui_mcp import REPLY_CAP_CHARS, cap_reply  # noqa: E402

# The closing line of a cut reply: how to ask for the rest.
CAP_HINT = "ch_notes(since=...) reads a long trace in pieces"


def capped(text: str) -> str:
    """A reply cut at REPLY_CAP_CHARS with the Clone Hero closing line."""
    return cap_reply(text, REPLY_CAP_CHARS, hint=CAP_HINT)


def _to_stderr(line: str) -> None:
    print(line, file=sys.stderr)


class Helper:
    """The one lock and every tool object sharing it: the game-side tools,
    the Frida tools, the Star Power tools, the virtual kit, the menu driver
    and the method watch. `env` and the keyword dicts pass the tests' fakes
    through."""

    def __init__(self, env: Optional[bot_run.Env] = None,
                 scratch_root: Optional[str] = None,
                 game_kw: Optional[dict] = None, frida_kw: Optional[dict] = None,
                 sp_kw: Optional[dict] = None, kit_kw: Optional[dict] = None,
                 menu_kw: Optional[dict] = None, watch_kw: Optional[dict] = None):
        self.lock = threading.RLock()
        self.env = env if env is not None else bot_run.Env(out=_to_stderr)
        self.game = GameTools(self.env, self.lock, scratch_root, **(game_kw or {}))
        frida_kw = {"now": self.env.now, "sleep": self.env.sleep, **(frida_kw or {})}
        self.frida = FridaTools(self.game, self.lock, **frida_kw)
        # SpTools keeps its own `now`, because its CSV's wall_s column is on
        # the clock sp_watch's docstring names, not on env's.
        sp_kw = {"sleep": self.env.sleep, **(sp_kw or {})}
        self.sp = SpTools(self.game, self.lock, **sp_kw)
        # KitTools hands its press to GameTools itself (use_kit_press). It
        # reads the pause setting in env's documents folder, the same
        # settings.ini ch_start writes the run's value into.
        documents_dir = self.env.documents_dir
        kit_kw = {"now": self.env.now, "sleep": self.env.sleep,
                  "read_settings": lambda: read_pause_setting(documents_dir),
                  **(kit_kw or {})}
        self.kit = KitTools(self.game, self.frida, self.lock, **kit_kw)
        menu_kw = {"now": self.env.now, "sleep": self.env.sleep, **(menu_kw or {})}
        self.menu = MenuTools(self.game, self.frida, self.lock, kit=self.kit, **menu_kw)
        watch_kw = {"now": self.env.now, **(watch_kw or {})}
        self.method_watch = MethodWatch(self.game, self.frida, self.lock, **watch_kw)

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
            "Drive Clone Hero as text, with nobody at the keyboard and the game window"
            " never brought to the front. ch_start backs up the user's files and the"
            " Rewired registry key, then launches the game minimized (with a song folder"
            " the bot plays it; with none the game opens at its menu); ch_stop closes it"
            " and puts them back. ch_attach, ch_state, ch_snapshot and ch_menu read the"
            " game through Frida; ch_trace_start, ch_notes and ch_trace_stop trace each"
            " note the player hits. Presses and menus go through the virtual kit inside"
            " the game. ch_kit_setup builds the kit after ch_attach and switches Rewired's"
            " keyboard and mouse off for the run, so the user's own keys and mouse reach"
            " no player. ch_kit_press presses the kit's pads and buttons, ch_open_song"
            " walks the menus to a song with it, ch_play_chart plays a chart on it,"
            " ch_kit_status and ch_kit_watch read it, and ch_kit_teardown takes it away and"
            " switches the keyboard and mouse back (ch_stop does too). ch_key, ch_press and"
            " ch_play_chart with route keys press keys into the focused window, and a"
            " hook refuses them unless the user has typed allow key run."
            " ch_sp and the ch_sp_watch tools read Star Power."
            " See docs/agents/clone-hero-mcp.md."),
    )

    # -- starting, stopping and status (game_tools) ------------------------------

    @server.tool()
    def ch_start(song_folder: str | None = None, player: str = bot_run.DEFAULT_PLAYER,
                 profile: str | None = None) -> str:
        """Back up the user's game files and the Rewired registry key, set
        settings.ini's pause_on_focus_lost line for the run, then start Clone
        Hero minimized and watch whether its window ever comes to the front.
        With a song folder the game plays it with a bot from the command line;
        with none it opens at its menu. A run already open is stopped and
        restored first. The reply names the backup folder, the settings line,
        the command line, the pid and the song length."""
        return capped(h.game.ch_start(song_folder, player, profile))

    @server.tool()
    def ch_stop() -> str:
        """Stop any trace, Frida session, kit and play session, then close the
        game the server started and restore the user's files and the registry
        key, one line each, after the kit teardown's checks and the count of
        times the game came to the front. A game the server did not start
        stays open; its files are restored."""
        return capped(h.game.ch_stop())

    @server.tool()
    def ch_status() -> str:
        """The run, the backup folder, how many times the game came to the
        front, the play session, the last stats copy, whether Frida is
        attached and whether a trace runs."""
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
    def ch_menu() -> str:
        """One fresh read of the menus: how many objects of each menu class
        are live, whether each one's Online menu is on screen
        (onlineMenuActive), each player screen's page, cursor, highlighted
        row, controller type and the other fields menu_fields.json names,
        the selected song's folder and name, the game mode, the player
        count, and the live-GameManager word. Needs ch_attach."""
        return capped(h.frida.ch_menu())

    @server.tool()
    def ch_trace_start(song_folder: str, score_json: str | None = None,
                       with_dynamics: bool = False) -> str:
        """Start tracing, in the background, each note the player hits: the
        default hook fires once per hit note and never for a missed note or
        an overhit. The notes come from the song folder's manifest, else from
        score_json.
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

    # -- the virtual kit (kit_tools) ------------------------------------------------------

    @server.tool()
    def ch_kit_setup() -> str:
        """Build the virtual drum kit inside the game: check that settings.ini
        holds the run's pause_on_focus_lost value (ch_start sets it), take
        the backup, attach the kit agent, let the game listen while it is
        not in focus, switch Rewired's keyboard and mouse off for the run so
        the user's own input reaches no player, take the keyboard and mouse
        from every player, hand the kit to its player with the game's own
        drum map, and check the bindings it reads back against
        kit_tools.KIT_BINDINGS. A setup that fails, leaves the keyboard or
        the mouse on, or reads other bindings is torn down again. Needs
        ch_attach."""
        return capped(h.kit.ch_kit_setup())

    @server.tool()
    def ch_kit_press(items: str, hold_frames: int | None = None,
                     rewrite: bool | None = None, on_main: bool | None = None,
                     read: str | None = None) -> str:
        """Press one chord of kit items, such as "green,kick" or "start",
        with or without the kit: prefix, and print the frames it went down
        and came up on. hold_frames, rewrite and on_main override the kit's
        hold, rewrite and thread for this press; left out, the kit's own
        values apply. read names game actions, such as "Green", to read back
        on the kit's player during the hold. An unknown name is refused with
        the list of kit items. Needs ch_kit_setup."""
        return capped(h.kit.ch_kit_press(items, hold_frames, rewrite, on_main, read))

    @server.tool()
    def ch_kit_status() -> str:
        """The kit agent's status: the frame count and the frame rate,
        whether the game has focus, whether the game listens while it is not
        in focus, whether Rewired's keyboard and mouse are switched on, the
        kit, each player's keyboard and mouse flags, then how many times the
        game came to the front."""
        return capped(h.kit.ch_kit_status())

    @server.tool()
    def ch_kit_watch(seconds: float) -> str:
        """For seconds, read every Rewired player and the controllers for any
        button held, and say whether any key or button reached a player; a
        true read is printed as a failure with its frame and reader. A watch
        longer than kit_tools.KIT_WATCH_MAX_S is refused, and so is one while
        a play session runs. Needs ch_kit_setup."""
        return capped(h.kit.ch_kit_watch(seconds))

    @server.tool()
    def ch_kit_teardown() -> str:
        """Take the kit away: clear its pads and buttons, remove it, put back
        every player's keyboard and mouse, switch Rewired's keyboard and
        mouse back to what ch_kit_setup recorded, put back the other settings
        it changed, print each check, and detach the kit agent. ch_stop runs
        it too."""
        return capped(h.kit.ch_kit_teardown())

    # -- the auto-player and the key route (game_tools) --------------------------------

    @server.tool()
    def ch_play_chart(score_json: str, log: str | None = None,
                      song_folder: str | None = None, route: str | None = None) -> str:
        """Play a hydra_replay score JSON in the background, pressing each
        chord through the virtual kit inside the game. route is "kit" or
        "keys"; left out, the kit is used once ch_kit_setup has built it, and
        the call is refused until then. "keys" presses keys into the focused
        game window, and a hook refuses it unless the user has typed allow
        key run. Start the song from the menus with ch_open_song: a song
        launched with a bot is played by the bot. log, when given, is where
        the press log goes. song_folder, when given, sets the run cap from
        the song's length; with none the auto-player keeps its own cap."""
        return capped(h.game.ch_play_chart(score_json, log, song_folder, route))

    @server.tool()
    def ch_press(lanes: str) -> str:
        """The key route: press one chord of drum lanes, such as "red,kick",
        as keys after bringing the game window to the front. A hook refuses
        it unless the user has typed allow key run; ch_kit_press is the
        route that needs no window. An unknown name is refused with the list
        of lanes."""
        return capped(h.game.ch_press(lanes))

    @server.tool()
    def ch_key(keys: str) -> str:
        """The key route: press menu keys in order after bringing the game
        window to the front: a comma list of key names such as "enter,down",
        with an optional last item text:<string> that types the string (a
        comma in it is typed, not split). A hook refuses it unless the user
        has typed allow key run. An unknown name is refused with the list of
        keys."""
        return capped(h.game.ch_key(keys))

    @server.tool()
    def ch_play_status() -> str:
        """The play session's counts, its stop reason when it ended, and its
        press rows, newest first."""
        return capped(h.game.ch_play_status())

    @server.tool()
    def ch_play_stop() -> str:
        """Tell the play session to stop and show the result so far."""
        return capped(h.game.ch_play_stop())

    # -- starting a song from the menus (menu_tools) ----------------------------------

    @server.tool()
    def ch_open_song(song_folder: str) -> str:
        """Start the song in song_folder by pressing, through the virtual kit
        inside the game, the kit items menu_steps.json lists, reading the
        menus before every step and refusing while the Online menu may be on
        screen. One line per step, or the step it stopped on with the items
        pressed and the last read. Needs ch_kit_setup on a game open at its
        menu."""
        return capped(h.menu.ch_open_song(song_folder))

    # -- Star Power (sp_tools) ---------------------------------------------------------

    @server.tool()
    def ch_sp_watch_start(song_folder: str | None = None, out: str | None = None) -> str:
        """Back up the user's files, then watch the engine's Star Power fields
        in the background: wait for a moving clock, then log every change to
        a CSV until the song stops, the run cap or ch_sp_watch_stop.
        song_folder, when given, sets the run cap from the song's length. out
        is the CSV; none means a new file in the session folder."""
        return capped(h.sp.ch_sp_watch_start(song_folder, out))

    @server.tool()
    def ch_sp() -> str:
        """The latest Star Power read the watcher made: the engine clock, the
        amount and its maximum, whether the amount is at the ready level,
        the active flag, the end time, the multiplier and the overflow flag.
        It never reads the game itself."""
        return capped(h.sp.ch_sp())

    @server.tool()
    def ch_sp_watch_status(since_ms: float = 0) -> str:
        """The watch's rows so far, every activation and every rise of the
        amount from since_ms on, the engine's address, and how the watch
        ended."""
        return capped(h.sp.ch_sp_watch_status(since_ms))

    @server.tool()
    def ch_sp_watch_stop() -> str:
        """Tell the Star Power watch to stop, then show how it ended and the
        CSV."""
        return capped(h.sp.ch_sp_watch_stop())

    @server.tool()
    def ch_sp_compare(watch_csv: str, score_json: str, resolution: int | None = None,
                      sp_bar_ticks: int | None = None,
                      width: int = sp_compare.DEFAULT_WIDTH) -> str:
        """Compare a watcher CSV with a hydra_replay score JSON and print
        sp_compare's report, one PASS or FAIL line per check. resolution,
        when given, also prints Hydra's full meter beside the game's."""
        return capped(h.sp.ch_sp_compare(watch_csv, score_json, resolution,
                                         sp_bar_ticks, width))

    @server.tool()
    def ch_sp_between(start_ms: float, end_ms: float) -> str:
        """Over the watch's rows: each activation whose flip lies between
        start_ms and end_ms, the highest amount read there, and the first
        read there at the ready level with the flag clear."""
        return capped(h.sp.ch_sp_between(start_ms, end_ms))

    # -- the method watch (method_watch) ------------------------------------------------

    @server.tool()
    def ch_method_watch_start(classes: list[str] | None = None,
                              cap_s: float = METHOD_WATCH_CAP_S) -> str:
        """Hook the menu and song-start methods start_candidates.json names
        (or only those of the named classes) for at most cap_s seconds, and
        say what was hooked and what could not be found. Needs ch_attach."""
        return capped(h.method_watch.ch_method_watch_start(classes, cap_s))

    @server.tool()
    def ch_method_watch_stop() -> str:
        """End the method watch, then name the call just before each scene
        load and list every call in order, the list last."""
        return capped(h.method_watch.ch_method_watch_stop())

    return server


def main() -> None:
    helper = Helper()
    # The restore lines go to stderr, which Claude Code keeps in its MCP log.
    atexit.register(lambda: _to_stderr(helper.shutdown()))
    build_server(helper).run("stdio")


if __name__ == "__main__":
    main()
