"""The column lists of the overlay spike's two CSV logs.

This module is the one home of both lists. The script that writes a log and
every script that reads it import the list from here, so a column added or
renamed in one place reaches all of them and the two sides cannot drift.

WATCH_COLUMNS is the Star Power watcher's log (sp_watch.py): one row per poll
of the live engine. Several fields are logged at more than one width because
the disassembly does not say how wide they are; the write-up picks one.

PRESS_LOG_COLUMNS is the auto-player's press log (play_chart.py --log): one
row per chord it pressed. `index` is the chord's own `index` field in the
`hydra_replay score` JSON, not a running count. KIT_PRESS_COLUMNS are the
kit press's reply (tools/ch_mcp/kit_tools.py); a run that presses keys leaves
them blank. WALL_PRESS_COLUMNS are two times on the watch's `wall_s` clock, so
a row lines up with the watch and a trace: just after the wait for the chord
ended, and after the press returned. PLAYER_PRESS_COLUMNS are what the player
reads around a press: the points the game scored between the last press and
this one (`scored_between`, which play_chart.play_chords works out), the Star
Power end the game stored after an activation's press (`end_read_ms`), and the
lead a probe press asked for (`probe`, edge_probe.py); each is blank on a row
it does not apply to. RETIME_PRESS_COLUMNS hold what the retime after an
end read did to a timed press (`retimed`, the Press.retimed word in
play_chart.py: late, inside or plain), blank on every other row. SEND_COLUMN
says whether play_chart sent the press, declined its chord or dropped it
(play_chart.SENT, DECLINED, DROPPED). TIMING_PRESS_COLUMNS say how long each
step of a message took, in ms, on the first row of each message, so a stall
names its step (PS3 of docs/superpowers/plans/2026-11-12-player-stall-fixes.md);
play_chart.play_chords says what each one times. IN_GAME_PRESS_COLUMNS are
what the kit agent reports about a press it timed on the game's frames
(play_chart.play_in_game says what each holds); they are blank on rows
play_chords wrote, and the timing columns are blank on rows play_in_game
wrote (question 3 of
docs/superpowers/plans/2026-11-12-in-game-press-timing.md (removed; git show b971b235:docs/superpowers/plans/2026-11-12-in-game-press-timing.md)).

Older logs have a `hit` column where SEND_COLUMN is now: the player's own
verdict from any score rise, which D319 item 1 removed, with the send state
in it for presses that were not sent. Logs written before the kit press
existed hold only KEYS_PRESS_COLUMNS, logs written before the wall times
existed hold PRESS_COLUMNS_WITHOUT_WALL, logs written before the player's
columns existed hold PRESS_COLUMNS_WITHOUT_PLAYER, logs written before the
retime column existed hold PRESS_COLUMNS_WITHOUT_RETIME, logs written
before SEND_COLUMN existed hold PRESS_COLUMNS_WITH_HIT, logs written
before the timing columns existed hold PRESS_COLUMNS_BEFORE_TIMING, and logs
written before the in-game columns existed hold PRESS_COLUMNS_BEFORE_IN_GAME.
"""

from __future__ import annotations

WATCH_COLUMNS = (
    "wall_s",
    "clock_s",
    "score",
    "sp_amount_u32",
    "sp_amount_u64",
    "sp_max_u32",
    "sp_max_u64",
    "sp_half_u64",
    "sp_active_byte",
    "sp_active_u32",
    "sp_mult_byte",
    "sp_mult_u32",
    "sp_end_s",
    "sp_overflow_u32",
    "round_start_s",
    "audio_s",
    "raw_s",
    "corr_s",
)

# The watch's columns from the clock object. Logs written before they existed
# lack them, and the two readers (watch_checks.py, sp_compare.py) still load
# those logs, with these columns read as NaN.
WATCH_CLOCK_COLUMNS = ("audio_s", "raw_s", "corr_s")
# The game's own half of the meter (constants.OFF_SP_HALF). Every log written
# before it existed lacks it, the saved ones included, so it is left out of
# WATCH_COLUMNS_WITHOUT_LATE_GROUPS beside the clock columns.
WATCH_HALF_COLUMNS = ("sp_half_u64",)
# WATCH_COLUMNS without the clock and half groups: the columns every watcher
# log has.
WATCH_COLUMNS_WITHOUT_LATE_GROUPS = tuple(
    c for c in WATCH_COLUMNS if c not in WATCH_CLOCK_COLUMNS + WATCH_HALF_COLUMNS)

# Saved logs under experiments/results/ name some columns differently: each key
# is a name such a log uses, and its value the WATCH_COLUMNS name for the same
# field. Both readers pass a log's header through watch_header first.
WATCH_OLDER_NAMES = {"hit_time_s": "round_start_s"}


def watch_header(header) -> tuple:
    """A watcher log's header, with each name in WATCH_OLDER_NAMES replaced by
    its WATCH_COLUMNS name."""
    return tuple(WATCH_OLDER_NAMES.get(c, c) for c in header)

# The older logs' verdict column, and the column that took its place.
HIT_COLUMN = "hit"
SEND_COLUMN = "send"

KEYS_PRESS_COLUMNS = (
    "index",
    "chart_ms",
    "clock_ms_at_press",
    "score_before",
    "score_after",
    HIT_COLUMN,
)

KIT_PRESS_COLUMNS = (
    "down_frame",
    "up_frame",
    "down_ms",
    "up_ms",
)

WALL_PRESS_COLUMNS = (
    "wall_s_at_read",
    "wall_s_at_reply",
)

PLAYER_PRESS_COLUMNS = (
    "scored_between",
    "end_read_ms",
    "probe",
)

RETIME_PRESS_COLUMNS = (
    "retimed",
)

TIMING_PRESS_COLUMNS = (
    "wait_read_max_ms",
    "wait_oversleep_max_ms",
    "press_took_ms",
    "reply_check_took_ms",
    "end_read_took_ms",
    "score_read_took_ms",
    "prev_handoff_took_ms",
)

IN_GAME_PRESS_COLUMNS = (
    "frame_gap_ms",
    "agent_note",
)

PRESS_COLUMNS_WITHOUT_WALL = KEYS_PRESS_COLUMNS + KIT_PRESS_COLUMNS

PRESS_COLUMNS_WITHOUT_PLAYER = PRESS_COLUMNS_WITHOUT_WALL + WALL_PRESS_COLUMNS

PRESS_COLUMNS_WITHOUT_RETIME = PRESS_COLUMNS_WITHOUT_PLAYER + PLAYER_PRESS_COLUMNS

PRESS_COLUMNS_WITH_HIT = PRESS_COLUMNS_WITHOUT_RETIME + RETIME_PRESS_COLUMNS

PRESS_COLUMNS_BEFORE_TIMING = tuple(SEND_COLUMN if c == HIT_COLUMN else c
                                    for c in PRESS_COLUMNS_WITH_HIT)

PRESS_COLUMNS_BEFORE_IN_GAME = PRESS_COLUMNS_BEFORE_TIMING + TIMING_PRESS_COLUMNS

PRESS_LOG_COLUMNS = PRESS_COLUMNS_BEFORE_IN_GAME + IN_GAME_PRESS_COLUMNS
