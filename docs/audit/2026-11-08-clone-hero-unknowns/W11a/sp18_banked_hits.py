"""SP-18: count hits that landed with Star Power banked and did not activate it.

For each Star Power watch CSV under the given run folders, a "banked hit" is a
row where the score rose since the row before (a note was hit), Star Power was
off on the row before and is still off on this row, and the meter on the row
before held at least half the full bar (sp_max_u32 / 2), so the game could
activate (SP-16). Each such row is a hit that could have activated Star Power
and did not. The script also prints how many activations the file shows (rows
where sp_active_byte goes from 0 to 1) for comparison.

Run from anywhere:
  python -I sp18_banked_hits.py <runs root> <folder> [more folders]
"""
import csv
import pathlib
import sys


def main() -> None:
    root = pathlib.Path(sys.argv[1])
    print("csv\tbanked_hits\tactivations\tfirst_banked_hit_clock_s")
    for folder in sys.argv[2:]:
        for path in sorted((root / folder).rglob("*sp_watch*.csv")):
            rel = path.relative_to(root).as_posix()
            banked = 0
            acts = 0
            first = "-"
            prev = None
            with path.open(newline="") as f:
                for row in csv.DictReader(f):
                    if prev is not None:
                        was_off = int(prev["sp_active_byte"]) == 0
                        now_on = int(row["sp_active_byte"]) == 1
                        if was_off and now_on:
                            acts += 1
                        half = int(prev["sp_max_u32"]) // 2
                        if (was_off and not now_on and int(row["score"]) > int(prev["score"])
                                and int(prev["sp_amount_u32"]) >= half > 0):
                            banked += 1
                            if first == "-":
                                first = f"{float(row['clock_s']):.4f}"
                    prev = row
            print(f"{rel}\t{banked}\t{acts}\t{first}")


if __name__ == "__main__":
    main()
