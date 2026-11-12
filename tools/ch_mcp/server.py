"""The MCP server that lets an agent drive Clone Hero as text.

Claude Code starts this server (see .mcp.json at the repo root) and calls its
tools. The plans are the first server plan (removed; read it with git show
444631e6:docs/superpowers/plans/2026-10-28-ch-mcp-server.md) and
docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md), and
docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md (removed; git show 6804353f:docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md) for the virtual kit.
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
from typing import List, Optional

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_mcp import bot_run  # noqa: E402
from tools.ch_mcp.frida_tools import FridaTools  # noqa: E402
from tools.ch_mcp.game_tools import GameTools, RunParts  # noqa: E402
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
        self.game.use_run_parts(RunParts(self.frida, self.kit, self.sp))

    def ch_status(self) -> str:
        with self.lock:
            return "\n".join([self.game.ch_status()] + self.frida.status_lines())

    def open_items(self) -> List[str]:
        """One line per thing a restart of this process would lose (the reload
        plan's design point 2). Each tool answers for its own part through a
        public check."""
        with self.lock:
            lines = self.game.open_lines()
            if self.frida.is_attached():
                lines.append("frida: state agent attached")
            if self.kit.agent_attached():
                lines.append("kit: kit agent attached")
            if self.sp.watch_running():
                lines.append("star power watch: running")
            if self.method_watch.running():
                lines.append("method watch: running")
            if self.frida.trace_running():
                lines.append("trace: running")
            return lines

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
            " never brought to the front. ch_start checks the machine is quiet (about 6 s,"
            " and it can refuse), starts a machine recorder beside the run, backs up the user's files and the"
            " Rewired registry key, then launches the game minimized (with a song folder"
            " the bot plays it, or with human=True a human player on the virtual kit;"
            " with none the game opens at its menu); ch_stop closes it"
            " and puts them back. ch_attach, ch_state, ch_snapshot and ch_menu read the"
            " game through Frida; ch_trace_start, ch_notes and ch_trace_stop trace each"
            " note the player hits. Presses and menus go through the virtual kit inside"
            " the game. ch_kit_setup builds the kit after ch_attach and switches Rewired's"
            " keyboard and mouse off for the run, so the user's own keys and mouse reach"
            " no player. ch_kit_press presses the kit's pads, kick, Start and Select, ch_open_song"
            " walks the menus to a song with it (the route for a day human=True breaks),"
            " ch_play_chart plays a chart on it, ch_run_chart does the attach, the setup,"
            " the Star Power watch and the play in one call,"
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
                 profile: str | None = None, human: bool = False) -> str:
        """Check the machine is quiet (about 6 s; a machine that is not quiet
        refuses the launch and the reply says why). Back up the user's game
        files and the Rewired registry key, set settings.ini's
        pause_on_focus_lost line for the run, then start Clone
        Hero minimized and watch whether its window ever comes to the front.
        With a song folder the game plays it with a bot from the command line;
        with none it opens at its menu. human=True with a song folder starts
        the song from the command line for a human player instead: the game
        starts paused, the launch agent changes one byte of the command-line
        loader in memory when GameAssembly.dll appears
        (tools/ch_mcp/launch_patch.json), and the game is let go. human=True
        with no song folder is refused. A run already open is stopped and
        restored first. Every run also starts the machine recorder beside the
        game, after the check and before the launch; a recorder that fails to
        start is one line and the run goes on. The reply names the recorder's
        pid and folder, the worker's pid and its parent's, the backup folder,
        the settings line, the command line, the pid and the song length."""
        return capped(h.game.ch_start(song_folder, player, profile, human=human))

    @server.tool()
    def ch_stop() -> str:
        """Stop the machine recorder and any trace, Frida session, kit and
        play session, then close the game the server started and restore the
        user's files and the registry key, one line each, after the recorder's
        line, the kit teardown's checks and the count of times the game came
        to the front. A game the server did not start stays open; its files
        are restored."""
        return capped(h.game.ch_stop())

    @server.tool()
    def ch_status() -> str:
        """The run, the backup folder, how many times the game came to the
        front, the machine recorder, the play session, the last stats copy,
        whether Frida is attached and whether a trace runs. For a human launch
        it also shows the launch agent's patch status, ending with the line
        "loader ran after patch:" and yes, no or not yet."""
        return capped(h.ch_status())

    @server.tool()
    def ch_open_items() -> str:
        """What the server holds open, one line each: the run and its backup
        folder, an adopted backup, the Frida state agent, the kit, the machine
        recorder while it runs, and a running play session, Star Power watch,
        method watch or trace. With
        none of them it says "nothing open". ch_stop closes them all."""
        return capped("\n".join(h.open_items()) or "nothing open")

    @server.tool()
    def ch_install_chart(chart_folder: str, name: str | None = None) -> str:
        """Copy one chart folder's song.ini and chart (nothing else) into the
        probe song folder, with a silent audio file as long as Hydra's song end
        for the chart and a title marked as a probe, so the game can launch it.
        Refuses a name that is not a plain folder name."""
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
    def ch_compare(stats_path: str, score_json: str, checksum: str | None = None,
                   playback_speed: int | None = None, no_fail: bool | None = None,
                   modifiers: list[str] | None = None) -> str:
        """Compare a stats copy with a hydra_replay score JSON, one line per
        check, then pass or fail. checksum, playback_speed, no_fail and
        modifiers are what the run asked the game for, given all four or none
        (game_tools.GameTools.ch_compare says why)."""
        return capped(h.game.ch_compare(stats_path, score_json, checksum, playback_speed,
                                        no_fail, modifiers))

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
        play session has run. While a song is playing it also searches for
        the engine and prints its flags word, whether bit 0x10 is set, the
        dynamics threshold and the ghost axis value; the search takes time and
        the song keeps moving during it. At a menu or paused it says those
        lines were not read."""
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
        score_json. The trace is capped at Hydra's song end for the song
        folder's chart plus a margin, found by one hydra_replay dump that can
        take up to prep_inputs.REPLAY_CAP_S; a folder with no chart, or a failed dump,
        starts no trace.
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
    def ch_kit_setup(player: str | None = None) -> str:
        """Build the virtual drum kit inside the game: check that settings.ini
        holds the run's pause_on_focus_lost value (ch_start sets it), take
        the backup, attach the kit agent, let the game listen while it is
        not in focus, switch Rewired's keyboard and mouse off for the run so
        the user's own input reaches no player, take the keyboard and mouse
        from every player, build the kit from the game's MIDI Drums
        definition (kit_tools.KIT_DEFINITION_ID, every element an axis, as a
        real MIDI kit's), hand it to its player with the game's own drum map,
        and check the bindings it reads back against kit_tools.KIT_BINDINGS,
        each an axis binding. player names the Rewired player the kit goes
        to; left out, it goes to kit_tools.KIT_PLAYER, and a given name is
        sent as is and named in the reply's first line. A setup that fails,
        leaves the keyboard or the mouse on, or reads other bindings or a
        button binding is torn down again. Needs ch_attach."""
        return capped(h.kit.ch_kit_setup(player))

    @server.tool()
    def ch_kit_press(items: str, hold_frames: int | None = None,
                     rewrite: bool | None = None, on_main: bool | None = None,
                     read: str | None = None, value: float | None = None) -> str:
        """Press one chord of kit items, such as "green,kick" or "start",
        with or without the kit: prefix, and print the frames it went down
        and came up on. hold_frames, rewrite and on_main override the kit's
        hold, rewrite and thread for this press; left out, the kit's own
        values apply. read names game actions, such as "Green", to read back
        on the kit's player during the hold. value sets the axis value of
        this press's pads and kick, from 0 to 1, such as the ghost value
        ch_snapshot prints; left out, they press as plain gems. Start and
        Select always press at full, whatever value says. An unknown name is refused with the list of kit items.
        Needs ch_kit_setup."""
        return capped(h.kit.ch_kit_press(items, hold_frames, rewrite, on_main, read, value))

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
        """Take the kit away: clear its elements, remove it, put back
        every player's keyboard and mouse, switch Rewired's keyboard and
        mouse back to what ch_kit_setup recorded, put back the other settings
        it changed, print each check, and detach the kit agent. ch_stop runs
        it too."""
        return capped(h.kit.ch_kit_teardown())

    # -- the auto-player and the key route (game_tools) --------------------------------

    @server.tool()
    def ch_play_chart(score_json: str, schedule_json: str, log: str | None = None,
                      song_folder: str | None = None, route: str | None = None,
                      path_index: int = 0) -> str:
        """Play a hydra_replay schedule JSON (hydra_replay schedule, one path
        of the chart score_json scores) in the background, pressing each
        chord through the virtual kit inside the game. route is "kit" or
        "keys"; left out, the kit is used once ch_kit_setup has built it, and
        the call is refused until then. "keys" presses keys into the focused
        game window, and a hook refuses it unless the user has typed allow
        key run. Start the song with ch_start(song_folder, human=True), or
        from the menus with ch_open_song: a song launched with a bot is
        played by the bot. log, when given, is where
        the press log goes, row by row as the run goes. song_folder, when given, sets the run cap from
        the schedule's song end (not the folder's); with none the auto-player keeps its own cap.
        path_index is the path of the dump the schedule was made from, as
        hydra_replay schedule's --index; a schedule made from another path,
        or one that does not say, is refused.
        The schedule says which gems each press sends and when, a fill the
        path passes over among them; a schedule that holds a refusal or a
        squeeze the game cannot make is refused in its own words. The reply
        names each declined chord with what it sends, each squeeze chord with
        when its presses fire against the end, each chord pressed early or
        late for a squeeze, and each chord pressed in more than one group, in
        the order its gems are pressed."""
        return capped(h.game.ch_play_chart(score_json, schedule_json, log, song_folder, route,
                                           path_index))

    @server.tool()
    def ch_run_chart(song_folder: str, score_json: str, schedule_json: str,
                     kit_player: str | None = None, path_index: int = 0) -> str:
        """Start a run in one call, so the first press can beat the first
        note: ch_attach, a check that the game has the schedule's song
        loaded, ch_kit_setup on kit_player, the Star Power watch and the
        auto-player on the kit, with one engine search for the watch and the
        player. Call it right after ch_start(song_folder, human=True), with
        kit_player="System" for such a launch. song_folder must be the folder
        of the schedule's chart. score_json, schedule_json and path_index are
        ch_play_chart's. Before the attach it writes builds-<n>.json in the
        session folder, the agent builds and the hydra_replay build the run
        uses. It writes a press log, a press-<n>.csv beside it, one row at
        each press as the run goes, and its reply names both files. The call
        returns at the first press, with one line per part, the time it
        took, the machine recorder's samples line after the watch's,
        ch_play_chart's declined, squeeze and order lines, the builds
        record's line, the press log's line, and the line "first press chord
        index k at ms". A part that fails stops the call there: the reply
        names it and the kit is taken away; a song the schedule is not for
        stops it before the kit setup. Refused while a play session runs.
        Read the run with ch_play_status and ch_sp_watch_status."""
        return capped(h.game.ch_run_chart(song_folder, score_json, schedule_json, kit_player,
                                          path_index))

    @server.tool()
    def ch_edge_probe(song_folder: str, score_json: str, schedule_json: str,
                      probes_json: str) -> str:
        """The window-edge probe: play a hydra_replay schedule JSON in the
        background on the kit, as ch_play_chart does, except that each chord
        the probes file names is pressed at the lead
        it gives, with no window check. probes_json is a JSON list of
        {"index": <chord index>, "lead_ms": <ms>}; a positive lead presses
        early, a negative one late. Each probe's press row is marked in the
        probe column with its asked lead, and ch_play_status adds one line per
        probe row with the asked lead, the lead the row shows and the game's score
        step across the press.
        A probe chord not in the schedule, or one timed from a Star Power
        end rather than its own time, is refused before anything runs. The run cap
        comes from the schedule's song end. The press rows go to a
        press-<n>.csv in the session folder as they happen, and the reply
        names it. Needs ch_kit_setup; refused while a play session runs."""
        return capped(h.game.ch_edge_probe(song_folder, score_json, schedule_json, probes_json))

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
        """The press log's path on the first line when the session has one,
        then the play session's counts, its stop reason when it ended, and its
        press rows, newest first. The counts are of presses; when a chord was
        pressed in two parts, the count line also says how many chords. Rows
        that carry them add a line per activation with its planned and read
        Star Power end, one line crediting the points scored between presses
        to the declined chords before them ("unexplained" when none waits),
        and a line per edge-probe row. A long reply is cut from its end, so the
        file holds every row."""
        return capped(h.game.ch_play_status())

    @server.tool()
    def ch_play_stop() -> str:
        """Tell the play session to stop and show the result so far."""
        return capped(h.game.ch_play_stop())

    # -- starting a song from the menus (menu_tools) ----------------------------------

    @server.tool()
    def ch_open_song(song_folder: str, player: str = bot_run.DEFAULT_PLAYER) -> str:
        """Start the song in song_folder by pressing, through the virtual kit
        inside the game, the kit items menu_steps.json lists, reading the
        menus before every step and refusing while the Online menu may be on
        screen. player is ch_start's "<Instrument>,<Difficulty>": a press on
        the player screen is refused while it shows another instrument or
        difficulty. One line per step, or the step it stopped on with the
        items pressed and the last read; a walk that loads another song, or
        never checks the player screen, ends refused. Needs ch_kit_setup on a
        game open at its menu. ch_start(song_folder, human=True) starts a
        song with no menu; this walk is kept for a day that route breaks."""
        return capped(h.menu.ch_open_song(song_folder, player))

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
        amount from since_ms on, the engine's address, how the watch ended,
        and a line with the gaps over 50 ms between samples, inside the
        song."""
        return capped(h.sp.ch_sp_watch_status(since_ms))

    @server.tool()
    def ch_sp_watch_stop() -> str:
        """Tell the Star Power watch to stop, then show how it ended and the
        CSV."""
        return capped(h.sp.ch_sp_watch_stop())

    @server.tool()
    def ch_sp_compare(watch_csv: str, score_json: str, schedule_json: str,
                      resolution: int | None = None,
                      sp_bar_ticks: int | None = None,
                      width: int = sp_compare.DEFAULT_WIDTH) -> str:
        """Compare a watcher CSV with a hydra_replay score JSON and print
        sp_compare's report, one PASS or FAIL line per check, then one line
        per activation with Hydra's ready point and the first row the game
        was ready. resolution, the chart's Resolution from its [Song]
        section, is needed: without it the compare is refused. The run's
        stop reason is the last play session's, and a running play is
        refused. The checks look for each chord where the auto-player pressed it,
        which schedule_json, the same path's hydra_replay schedule (the file
        ch_play_chart plays), says. A schedule that refuses its path is
        refused with the schedule's words."""
        return capped(h.sp.ch_sp_compare(watch_csv, score_json, schedule_json,
                                         resolution, sp_bar_ticks, width))

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
