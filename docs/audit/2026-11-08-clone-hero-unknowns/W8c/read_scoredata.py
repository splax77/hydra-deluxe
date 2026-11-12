"""Read a Clone Hero scoredata.bin (build 6142) in the layout the game's writer uses.

The layout comes from three listings under docs/clone-hero/code/6142/:
score_store_write_2F57C0.asm (file header), score_chart_write_2167B0.asm (one
chart record) and score_entry_write_2BAB60.asm (one instrument's entry). Their
.md notes say which listing line writes which field. This script only checks
that a real file parses to its last byte in that layout; it is not a reader
anything else uses.

It prints the structure, never the chart hash or the score, so the output
can be committed without naming the player's songs.

Run:  py -I docs/audit/2026-11-08-clone-hero-unknowns/W8c/read_scoredata.py <scoredata.bin> [...]
"""

import struct
import sys

INSTRUMENTS = ["Guitar", "Bass", "Rhythm", "GuitarCoop", "SixFretGuitar", "SixFretBass",
               "Drums", "Keys", "Band", "ProDrums", "SixFretRhythm", "SixFretGuitarCoop",
               "Crowd", "Vocals", "SixFretKeys", "None"]
DIFFICULTIES = {-1: "None", 0: "Easy", 1: "Medium", 2: "Hard", 3: "Expert"}


def read(path):
    data = open(path, "rb").read()
    pos = 0

    def take(fmt):
        nonlocal pos
        values = struct.unpack_from("<" + fmt, data, pos)
        pos += struct.calcsize("<" + fmt)
        return values

    version, charts = take("ii")
    print(f"{path}: {len(data)} bytes, version {version}, {charts} chart records")
    for c in range(charts):
        take("16s")  # chart hash, not printed
        entries, play_count, after = take("BHB")
        print(f"  chart {c}: {entries} entries, play count {play_count}, byte after play count {after}")
        for _ in range(entries):
            inst, diff, b14, flag8, short_a, b_c, mods, _score = take("hbBBhBIi")
            name = INSTRUMENTS[inst] if 0 <= inst < len(INSTRUMENTS) else inst
            print(f"    {name} {DIFFICULTIES.get(diff, diff)}: byte+0x14 {b14}, bool+0x8 {flag8}, "
                  f"short+0xA {short_a}, byte+0xC {b_c}, modifiers {mods:#x}")
    print(f"  parsed {pos} of {len(data)} bytes")


if __name__ == "__main__":
    for p in sys.argv[1:]:
        read(p)
