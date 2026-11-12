"""Count Star Power phrases that overlap or share a start, per drum track,
in every .chart and .mid under one folder. Read-only."""
import os
import re
import sys

import mido

ROOT = sys.argv[1]
S_LINE = re.compile(r"^\s*(\d+)\s*=\s*S\s+2\s+(\d+)")


def overlaps(spans):
    spans = sorted(spans)
    same_start = sum(1 for a, b in zip(spans, spans[1:]) if a[0] == b[0])
    inside = 0
    furthest = -1
    for s, e in spans:
        if s < furthest:
            inside += 1
        furthest = max(furthest, e)
    return same_start, inside


def chart_tracks(path):
    out, name, spans = {}, None, []
    for line in open(path, encoding="utf-8-sig", errors="replace"):
        line = line.strip()
        if line.startswith("["):
            if name and spans:
                out[name] = spans
            name, spans = line, []
        elif name and "Drums" in name:
            m = S_LINE.match(line)
            if m:
                t, ln = int(m.group(1)), int(m.group(2))
                spans.append((t, t + ln))
    if name and spans:
        out[name] = spans
    return out


def mid_tracks(path):
    out = {}
    mf = mido.MidiFile(path)
    for tr in mf.tracks:
        tname = next((m.name for m in tr if m.type == "track_name"), "")
        if "DRUM" not in tname.upper():
            continue
        t, open_starts, spans = 0, [], []
        for m in tr:
            t += m.time
            if m.type in ("note_on", "note_off") and m.note == 116:
                on = m.type == "note_on" and m.velocity > 0
                if on:
                    open_starts.append(t)
                elif open_starts:
                    spans.append((open_starts.pop(0), t))
        if spans:
            out[tname] = spans
    return out


files = hits = track_count = phrase_count = 0
for dp, _, fs in os.walk(ROOT):
    for f in fs:
        p = os.path.join(dp, f)
        try:
            if f.endswith(".chart"):
                tracks = chart_tracks(p)
            elif f.endswith(".mid"):
                tracks = mid_tracks(p)
            else:
                continue
        except Exception as e:
            print(f"UNREADABLE {p}: {e}")
            continue
        files += 1
        track_count += len(tracks)
        phrase_count += sum(len(s) for s in tracks.values())
        for tn, spans in tracks.items():
            same, inside = overlaps(spans)
            if same or inside:
                hits += 1
                print(f"{os.path.relpath(p, ROOT)} | {tn} | phrases {len(spans)} | same start {same} | starts inside another {inside}")
print(f"files read {files}; drum tracks with phrases {track_count}; phrases {phrase_count}; drum tracks with an overlap {hits}")
