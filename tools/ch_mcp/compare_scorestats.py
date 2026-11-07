"""Compare one bot run's scorestats.json with Hydra's score of the same chart.

    py -I tools/ch_mcp/compare_scorestats.py <scorestats copy> <hydra_replay score JSON>
        [--player-index 0]

The Hydra side is the JSON `hydra_replay score --chart <chart> --out <json>`
writes. Six lines come out, five of them checked:

  total_notes        pass or fail. The game's count against Hydra's gem count
                     and its chord count; the line says which one matches.
  sp_phrases_total   pass or fail. Against Hydra's chords that end an SP phrase.
  score              pass or fail. The game's score against Hydra's final.total.
  accent_notes_hit, ghost_notes_hit
                     pass or fail, one line for the two. Against Hydra's gems
                     of each dynamic.
  sp_activations     pass or fail. Against Hydra's list of activations.
  base_score         reported only. The game's base_score and note_score beside
                     three Hydra numbers, because what the game counts in
                     base_score is one of the things the live test learns.

Every number names the file it came from. The exit code is 0 when every
checked line passes, 1 otherwise.
"""

from __future__ import annotations

import argparse
import json
import sys
from collections import Counter
from dataclasses import dataclass
from typing import Callable, Optional

# The words hydra_replay writes in a gem's "dynamic"; dynamic_str in
# src/core/model.cpp owns them. They are written out here because this file
# runs on its own under py -I, and kit_tools imports game_tools, which imports
# this file.
DYNAMIC_ACCENT = "accent"
DYNAMIC_GHOST = "ghost"


@dataclass(frozen=True)
class HydraCounts:
    gems: int
    chords: int
    sp_phrase_ends: int
    notes: int
    notes_accent_ghost: int
    total: int
    accent_gems: int
    ghost_gems: int
    activations: int


def _load(path: str) -> dict:
    with open(path, encoding="utf-8-sig") as f:
        return json.load(f)


def hydra_counts(score: dict) -> HydraCounts:
    chords = score["chords"]
    final = score["final"]
    dynamics = Counter(n["dynamic"] for c in chords for n in c["notes"])
    return HydraCounts(
        gems=sum(len(c["notes"]) for c in chords),
        chords=len(chords),
        sp_phrase_ends=sum(1 for c in chords if c["is_sp_phrase_end"]),
        notes=int(final["notes"]),
        notes_accent_ghost=int(final["notes"]) + int(final["accent"]) + int(final["ghost"]),
        total=int(final["total"]),
        accent_gems=dynamics[DYNAMIC_ACCENT],
        ghost_gems=dynamics[DYNAMIC_GHOST],
        activations=len(score["activations"]),
    )


def _verdict(ok: bool) -> str:
    return "pass" if ok else "FAIL"


def compare(stats_path: str, hydra_path: str, *, player_index: int = 0,
            out: Callable[[str], None] = print) -> bool:
    """Print the six lines. Returns True when every checked line passes."""
    stats = _load(stats_path)
    players = stats.get("players") or []
    if not 0 <= player_index < len(players):
        raise ValueError(f"{stats_path} has {len(players)} players; "
                         f"player index {player_index} is not one of them")
    p = players[player_index]
    h = hydra_counts(_load(hydra_path))
    game_src = f"({stats_path}, players[{player_index}])"
    hydra_src = f"({hydra_path})"

    total = int(p["total_notes"])
    matches = [name for name, value in (("gems", h.gems), ("chords", h.chords)) if value == total]
    notes_ok = bool(matches)
    out(f"total_notes: {_verdict(notes_ok)}; game {total} {game_src}; "
        f"Hydra gems {h.gems}, chords {h.chords} {hydra_src}; "
        f"matches {' and '.join(matches) if matches else 'neither'}")

    phrases = int(p["sp_phrases_total"])
    sp_ok = phrases == h.sp_phrase_ends
    out(f"sp_phrases_total: {_verdict(sp_ok)}; game {phrases} {game_src}; "
        f"Hydra chords with is_sp_phrase_end {h.sp_phrase_ends} {hydra_src}")

    game_score = int(p["score"])
    score_ok = game_score == h.total
    out(f"score: {_verdict(score_ok)}; game {game_score} {game_src}; "
        f"Hydra final.total {h.total} {hydra_src}")

    accents, ghosts = int(p["accent_notes_hit"]), int(p["ghost_notes_hit"])
    dynamics_ok = accents == h.accent_gems and ghosts == h.ghost_gems
    out(f"accent_notes_hit, ghost_notes_hit: {_verdict(dynamics_ok)}; "
        f"game {accents} accents, {ghosts} ghosts {game_src}; "
        f"Hydra gems with dynamic accent {h.accent_gems}, ghost {h.ghost_gems} {hydra_src}")

    activations = int(p["sp_activations"])
    activations_ok = activations == h.activations
    out(f"sp_activations: {_verdict(activations_ok)}; game {activations} {game_src}; "
        f"Hydra activations {h.activations} {hydra_src}")

    hydra_values = (("final.notes", h.notes),
                    ("final.notes + accent + ghost", h.notes_accent_ghost),
                    ("final.total", h.total))

    def equal_to(value: int) -> str:
        names = [name for name, v in hydra_values if v == value]
        return ", ".join(names) if names else "none"

    base, note = int(p["base_score"]), int(p["note_score"])
    out(f"base_score (reported): game base_score {base}, note_score {note} {game_src}; "
        + ", ".join(f"Hydra {name} {v}" for name, v in hydra_values) + f" {hydra_src}; "
        f"base_score equals {equal_to(base)}; note_score equals {equal_to(note)}")
    return notes_ok and sp_ok and score_ok and dynamics_ok and activations_ok


def main(argv: Optional[list[str]] = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("scorestats", help="a copied scorestats.json")
    ap.add_argument("hydra", help="the hydra_replay score JSON for the same chart")
    ap.add_argument("--player-index", type=int, default=0,
                    help="which entry of the stats file's players list (default 0)")
    args = ap.parse_args(argv)
    return 0 if compare(args.scorestats, args.hydra, player_index=args.player_index) else 1


if __name__ == "__main__":
    sys.exit(main())
