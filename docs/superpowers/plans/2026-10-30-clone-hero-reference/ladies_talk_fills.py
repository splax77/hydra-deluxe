"""Count written fills (MIDI notes 120-124) and solos (103) on the drum track of one notes.mid.

Usage: py -I ladies_talk_fills.py <path to notes.mid>
Reads the file by hand (no mido). Prints the drum track name, ticks per beat,
each fill's start/end tick and measure (4/4 assumed from the time signatures
seen), and the solo spans.
"""
import struct
import sys


def read_varlen(data, i):
    v = 0
    while True:
        b = data[i]
        i += 1
        v = (v << 7) | (b & 0x7F)
        if not b & 0x80:
            return v, i


def parse(path):
    data = open(path, "rb").read()
    assert data[:4] == b"MThd"
    fmt, ntracks, div = struct.unpack(">HHH", data[8:14])
    i = 14
    tracks = []
    for _ in range(ntracks):
        assert data[i:i + 4] == b"MTrk"
        ln = struct.unpack(">I", data[i + 4:i + 8])[0]
        tracks.append(data[i + 8:i + 8 + ln])
        i += 8 + ln
    return div, tracks


def events(track):
    i = 0
    tick = 0
    status = 0
    name = None
    out = []
    timesigs = []
    while i < len(track):
        d, i = read_varlen(track, i)
        tick += d
        b = track[i]
        if b == 0xFF:
            typ = track[i + 1]
            ln, j = read_varlen(track, i + 2)
            body = track[j:j + ln]
            if typ == 0x03 and name is None:
                name = body.decode("latin1")
            if typ == 0x58:
                timesigs.append((tick, body[0], 2 ** body[1]))
            if typ == 0x01:
                out.append((tick, "text", body.decode("latin1")))
            i = j + ln
            continue
        if b in (0xF0, 0xF7):
            ln, j = read_varlen(track, i + 1)
            i = j + ln
            continue
        if b & 0x80:
            status = b
            i += 1
        kind = status & 0xF0
        if kind in (0x80, 0x90, 0xA0, 0xB0, 0xE0):
            d1, d2 = track[i], track[i + 1]
            i += 2
            if kind == 0x90 and d2 > 0:
                out.append((tick, "on", d1))
            elif kind == 0x80 or (kind == 0x90 and d2 == 0):
                out.append((tick, "off", d1))
        elif kind in (0xC0, 0xD0):
            i += 1
        else:
            raise SystemExit(f"unknown status {status:#x} at {i}")
    return name, out, timesigs


def main():
    div, tracks = parse(sys.argv[1])
    print("ticks per beat:", div)
    allsigs = []
    for t in tracks:
        name, evs, sigs = events(t)
        allsigs += sigs
        if name and name.lower().strip() in ("part drums", "part drum"):
            fills = {}
            spans = []
            solos = []
            opened = {}
            for tick, kind, n in evs:
                if kind == "on" and 120 <= n <= 124 or (kind == "on" and n == 103):
                    opened[n] = tick
                if kind == "off" and n in opened:
                    if n == 103:
                        solos.append((opened.pop(n), tick))
                    else:
                        spans.append((n, opened.pop(n), tick))
            lanes = {}
            for n, a, b in spans:
                lanes.setdefault((a, b), []).append(n)
            print("drum track:", name)
            print("fills (start tick, end tick, lanes):", len(lanes))
            for (a, b), ns in sorted(lanes.items()):
                print(f"  {a:>7} {b:>7}  measure {a // (div * 4) + 1:>4} to {b // (div * 4) + 1:>4}  lanes {sorted(ns)}")
            print("solos (start, end, measures):")
            for a, b in solos:
                print(f"  {a:>7} {b:>7}  measure {a // (div * 4) + 1} to {b // (div * 4) + 1}")
    sigs = sorted(set(allsigs))
    print("time signatures (tick, num, den):", sigs[:10], "..." if len(sigs) > 10 else "")


main()
