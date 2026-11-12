"""SP-14: list every Star Power activation in the run folders' Star Power watch CSVs.

For each CSV under the given run folders, find each row where sp_active_byte goes
from 0 to 1 (an activation). Print the meter on that first active row, the meter
on the row before it, the full bar (sp_max_u32) and the meter as a count of
quarter bars (sp_max / 4). A meter that is a whole number of quarters prints
"whole"; anything else prints the remainder in meter units.

Three more columns test whether a short first active row is drain that happened
between the activation and the sample: shortfall (meter before minus meter on
the first active row), gap_ms (song clock between the two rows) and drain_ms
(how long the game's own drain, measured over the next 50 active rows, takes to
remove that shortfall). drain_ms at or below gap_ms fits drain inside the gap.

Run from anywhere:
  python -I sp14_first_active_rows.py <runs root> 2026-11-06 2026-11-07 [more folders]
"""
import csv
import pathlib
import sys


def main() -> None:
    root = pathlib.Path(sys.argv[1])
    folders = sys.argv[2:]
    print("csv\tactivation\tclock_s\tmeter_before\tmeter_first_active\tsp_max\tquarters\tremainder"
          "\tshortfall\tgap_ms\tdrain_ms")
    for folder in folders:
        for path in sorted((root / folder).rglob("*sp_watch*.csv")):
            rel = path.relative_to(root).as_posix()
            n = 0
            prev = None
            with path.open(newline="") as f:
                rows = list(csv.DictReader(f))
            for i, row in enumerate(rows):
                active = int(row["sp_active_byte"])
                if prev is not None and int(prev["sp_active_byte"]) == 0 and active == 1:
                    n += 1
                    meter = int(row["sp_amount_u32"])
                    full = int(row["sp_max_u32"])
                    quarter = full // 4
                    q, r = divmod(meter, quarter) if quarter else (0, meter)
                    gap_ms = (float(row["clock_s"]) - float(prev["clock_s"])) * 1000
                    shortfall = int(prev["sp_amount_u32"]) - meter
                    # Drain rate from this row to the active row about 50 rows on.
                    j = min(i + 50, len(rows) - 1)
                    later = rows[j]
                    span_ms = (float(later["clock_s"]) - float(row["clock_s"])) * 1000
                    drained = meter - int(later["sp_amount_u32"])
                    est_ms = shortfall * span_ms / drained if drained > 0 else float("nan")
                    print(f"{rel}\t{n}\t{float(row['clock_s']):.4f}\t{prev['sp_amount_u32']}\t"
                          f"{meter}\t{full}\t{q}\t{'whole' if r == 0 else r}\t"
                          f"{shortfall}\t{gap_ms:.2f}\t{est_ms:.2f}")
                prev = row
            if n == 0:
                print(f"{rel}\t0\t-\t-\t-\t-\t-\tno activation")


if __name__ == "__main__":
    main()
