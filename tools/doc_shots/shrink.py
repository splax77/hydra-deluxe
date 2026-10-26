"""Re-saves the doc pictures smaller, with no loss, and holds them to the size limits.

Usage: py -I tools/doc_shots/shrink.py <images folder> [--below <file>=<y> ...]

tools/doc_shots.ps1 runs this after the runner has saved the pictures; see
docs/agents/doc-screenshots.md. A --below crop keeps only the rows from y
down, for a full-frame shot whose top band must not show. Every PNG is saved
again by Pillow with optimize=True, which keeps every pixel. The script prints
each file's size and exits 1 when a file or the folder is over its limit.
"""

import argparse
import sys
from pathlib import Path

from PIL import Image

# The user's limits (docs/superpowers/plans/2026-10-25-doc-screenshots.md,
# "User decisions (2026-10-25)").
MAX_FILE_BYTES = 500 * 1024
MAX_FOLDER_BYTES = 4 * 1024 * 1024


def parse_below(items):
    below = {}
    for item in items:
        name, sep, y = item.partition("=")
        if not sep or not y.isdigit():
            sys.exit(f"shrink: --below needs <file>=<y>, got {item!r}")
        below[name] = int(y)
    return below


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("folder", type=Path)
    parser.add_argument("--below", action="append", default=[])
    args = parser.parse_args()
    below = parse_below(args.below)

    pngs = sorted(p for p in args.folder.iterdir() if p.suffix.lower() == ".png")
    if not pngs:
        sys.exit(f"shrink: no PNG files in {args.folder}")
    unknown = set(below) - {p.name for p in pngs}
    if unknown:
        sys.exit(f"shrink: --below names files that are not there: {sorted(unknown)}")

    failed = False
    total = 0
    for path in pngs:
        with Image.open(path) as img:
            img.load()
            if path.name in below:
                top = below[path.name]
                if not 0 <= top < img.height:
                    sys.exit(f"shrink: --below {path.name}={top} is outside its {img.height} rows")
                img = img.crop((0, top, img.width, img.height))
            img.save(path, format="PNG", optimize=True)
            width, height = img.size
        size = path.stat().st_size
        total += size
        over = size > MAX_FILE_BYTES
        failed |= over
        note = f"  OVER the {MAX_FILE_BYTES // 1024} KB limit" if over else ""
        print(f"size: {path.name} {width}x{height} {size / 1024:.0f} KB{note}")

    over = total > MAX_FOLDER_BYTES
    failed |= over
    note = f"  OVER the {MAX_FOLDER_BYTES // (1024 * 1024)} MB limit" if over else ""
    print(f"total: {len(pngs)} files {total / 1024:.0f} KB{note}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
