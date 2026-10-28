"""The column lists of the overlay spike's two CSV logs.

This module is the one home of both lists. The script that writes a log and
every script that reads it import the list from here, so a column added or
renamed in one place reaches all of them and the two sides cannot drift.

WATCH_COLUMNS is the Star Power watcher's log (sp_watch.py): one row per poll
of the live engine. Several fields are logged at more than one width because
the disassembly does not say how wide they are; the write-up picks one.

PRESS_LOG_COLUMNS is the auto-player's press log (play_chart.py --log): one
row per chord it pressed. `index` is the chord's own `index` field in the
`hydra_replay score` JSON, not a running count.
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
    "sp_active_byte",
    "sp_active_u32",
    "sp_mult_byte",
    "sp_mult_u32",
    "sp_end_s",
    "sp_overflow_u32",
    "hit_time_s",
)

PRESS_LOG_COLUMNS = (
    "index",
    "chart_ms",
    "clock_ms_at_press",
    "score_before",
    "score_after",
    "hit",
)
