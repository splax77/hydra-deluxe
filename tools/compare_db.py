"""Compare two Hydra databases table by table, for "identical results" proofs.

    py tools\\compare_db.py A.db B.db

Prints one line per table, "<table>: N rows compared, M differ", then the
first few differing keys under any table that differs. Exits 1 when any
table differs, 2 on a usage error, 0 when everything matches.

The speedups plan (docs/superpowers/plans/2026-10-06-perf-speedups.md,
"Proving identical results") is the recipe this belongs to.

What it compares, and what it leaves out on purpose:

- Columns come from PRAGMA table_info, and keys from each table's primary key
  or unique index, so this script holds no copy of the store's schema.
- result_id is left out. It is only the order the batch's worker threads
  finished in. A results row is matched by its unique key instead, and a
  path_refs row by its result's unique key in place of the result_id.
- A table with no primary key (charts) is compared row by row in rowid order,
  because the order a scan saved its rows in is part of its answer.

The comparison runs inside SQLite (B is attached to A), so the whole library
never has to fit in Python's memory.
"""

import sqlite3
import sys
from pathlib import Path

TABLES = ["results", "paths", "path_refs", "songmeta", "dynamics", "charts", "meta"]
SHOWN = 5  # differing keys listed per table
RESULT_ID = "result_id"
ROW_NO = "row_no"  # the positional key of a table with no primary key


def ident(name):
    return '"' + name.replace('"', '""') + '"'


def table_exists(con, schema, table):
    row = con.execute(
        f"SELECT 1 FROM {schema}.sqlite_master WHERE type = 'table' AND name = ?", (table,)
    ).fetchone()
    return row is not None


def table_columns(con, schema, table):
    """(column names in table order, primary-key column names in key order)."""
    info = con.execute(f"PRAGMA {schema}.table_info({ident(table)})").fetchall()
    names = [row[1] for row in info]
    pk = [row[1] for row in sorted((r for r in info if r[5] > 0), key=lambda r: r[5])]
    return names, pk


def unique_key(con, schema, table):
    """The columns of the table's first unique index that does not use
    result_id, or None when it has none."""
    for idx in con.execute(f"PRAGMA {schema}.index_list({ident(table)})").fetchall():
        name, unique = idx[1], idx[2]
        if not unique:
            continue
        cols = [r[2] for r in con.execute(f"PRAGMA {schema}.index_info({ident(name)})")]
        if cols and RESULT_ID not in cols:
            return cols
    return None


class Plan:
    """How one table is read on each side: a SELECT per schema whose columns
    are the key columns, then the value columns, then a presence marker."""

    def __init__(self, keys, values, select):
        self.keys = keys
        self.values = values
        self._select = select  # schema name -> SQL text

    def select(self, schema):
        return self._select(schema)


def plan_for(con, table, names, pk):
    if pk == [RESULT_ID]:
        keys = unique_key(con, "main", table)
        if keys is None:
            raise SystemExit(f"{table}: keyed by result_id alone and has no other unique key")
        values = [c for c in names if c != RESULT_ID and c not in keys]
        cols = ", ".join(ident(c) for c in keys + values)
        return Plan(keys, values, lambda s: f"SELECT {cols}, 1 AS present_ FROM {s}.{ident(table)}")

    if RESULT_ID in pk:
        rkeys = unique_key(con, "main", "results")
        if rkeys is None:
            raise SystemExit(f"{table}: refers to results, which has no unique key")
        own_keys = [c for c in pk if c != RESULT_ID]
        keys = ["result_" + c for c in rkeys] + own_keys
        values = [c for c in names if c != RESULT_ID and c not in own_keys]
        rcols = ", ".join(f"r.{ident(c)} AS {ident('result_' + c)}" for c in rkeys)
        tcols = ", ".join(f"t.{ident(c)}" for c in own_keys + values)

        def select(s):
            return (
                f"SELECT {rcols}, {tcols}, 1 AS present_ FROM {s}.{ident(table)} t"
                f" LEFT JOIN {s}.results r ON r.{RESULT_ID} = t.{RESULT_ID}"
            )

        return Plan(keys, values, select)

    if pk:
        values = [c for c in names if c not in pk]
        cols = ", ".join(ident(c) for c in pk + values)
        return Plan(pk, values, lambda s: f"SELECT {cols}, 1 AS present_ FROM {s}.{ident(table)}")

    cols = ", ".join(ident(c) for c in names)
    return Plan(
        [ROW_NO],
        names,
        lambda s: (
            f"SELECT ROW_NUMBER() OVER (ORDER BY rowid) AS {ROW_NO}, {cols}, 1 AS present_"
            f" FROM {s}.{ident(table)}"
        ),
    )


def show(value):
    if isinstance(value, bytes):
        text = value[:16].hex()
        return text + ("..." if len(value) > 16 else "")
    return repr(value)


def compare_table(con, table):
    """Prints the table's line (and its first differing keys); returns True
    when the table matches."""
    in_a = table_exists(con, "main", table)
    in_b = table_exists(con, "b", table)
    if not in_a and not in_b:
        print(f"{table}: 0 rows compared, 0 differ (absent from both)")
        return True
    if in_a != in_b:
        side, other = ("main", "B") if in_a else ("b", "A")
        n = con.execute(f"SELECT COUNT(*) FROM {side}.{ident(table)}").fetchone()[0]
        print(f"{table}: {n} rows compared, {n} differ (table missing from {other})")
        return False

    names_a, pk_a = table_columns(con, "main", table)
    names_b, pk_b = table_columns(con, "b", table)
    if sorted(names_a) != sorted(names_b) or pk_a != pk_b:
        print(f"{table}: 0 rows compared, 1 differ (columns differ)")
        print(f"  A: {', '.join(names_a)}")
        print(f"  B: {', '.join(names_b)}")
        return False

    plan = plan_for(con, table, names_a, pk_a)
    on = " AND ".join(f"x.{ident(k)} IS y.{ident(k)}" for k in plan.keys)
    changed = " OR ".join(f"x.{ident(v)} IS NOT y.{ident(v)}" for v in plan.values)
    where = "y.present_ IS NULL" + (f" OR {changed}" if changed else "")
    key_cols = ", ".join(f"x.{ident(k)}" for k in plan.keys)
    a_sql, b_sql = plan.select("main"), plan.select("b")

    differing = []
    count = 0
    # Rows of A that B lacks or holds differently, then rows only B has.
    for sql in (
        f"SELECT {key_cols} FROM ({a_sql}) x LEFT JOIN ({b_sql}) y ON {on} WHERE {where}",
        f"SELECT {key_cols} FROM ({b_sql}) x LEFT JOIN ({a_sql}) y ON {on}"
        f" WHERE y.present_ IS NULL",
    ):
        for row in con.execute(sql):
            count += 1
            if len(differing) < SHOWN:
                differing.append(row)

    n_a = con.execute(f"SELECT COUNT(*) FROM main.{ident(table)}").fetchone()[0]
    only_b = con.execute(
        f"SELECT COUNT(*) FROM ({b_sql}) x LEFT JOIN ({a_sql}) y ON {on}"
        f" WHERE y.present_ IS NULL"
    ).fetchone()[0]
    print(f"{table}: {n_a + only_b} rows compared, {count} differ")
    for row in differing:
        print("  " + ", ".join(f"{k}={show(v)}" for k, v in zip(plan.keys, row)))
    if count > len(differing):
        print(f"  ...and {count - len(differing)} more")
    return count == 0


def main(argv):
    if len(argv) != 3:
        print("usage: py tools\\compare_db.py A.db B.db", file=sys.stderr)
        return 2
    # Read-only URIs, so a comparison can never write to either database.
    a_uri, b_uri = (Path(p).resolve().as_uri() + "?mode=ro" for p in argv[1:3])
    try:
        con = sqlite3.connect(a_uri, uri=True)
        con.execute("ATTACH DATABASE ? AS b", (b_uri,))
    except sqlite3.Error as e:
        print(f"cannot open the databases: {e}", file=sys.stderr)
        return 2
    all_match = True
    for table in TABLES:
        if not compare_table(con, table):
            all_match = False
    con.close()
    return 0 if all_match else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
