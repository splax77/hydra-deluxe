"""Count the per-player and per-section keys of every scorestats*.json under hydra-data/ch-mcp-runs.

Run: py docs/audit/2026-11-08-clone-hero-unknowns/main/sc37_scorestats_keys.py
Writes to stdout only.
"""
import json, pathlib, collections
root = pathlib.Path(r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs")
res = collections.Counter()
bydir = collections.Counter()
clean = collections.Counter()
for p in sorted(root.rglob("scorestats*.json")):
    try:
        d = json.load(open(p, encoding="utf-8"))
    except Exception as e:
        print("bad", p, e)
        continue
    players = d.get("players") or d.get("player_stats") or []
    if not players:
        for k, v in d.items():
            if isinstance(v, list) and v and isinstance(v[0], dict):
                players = v
                break
    bydir[p.parent.parent.name + "/" + p.parent.name] += 1
    for pl in players:
        res[(len(pl), len(pl.get("section_stats", [{}])[0]) if pl.get("section_stats") else 0)] += 1
        clean[pl.get("clean_play_bonus")] += 1
print(dict(bydir))
print("(per-player keys, section keys) ->", dict(res))
print("clean_play_bonus values", dict(clean))
