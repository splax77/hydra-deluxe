r"""Compare two Hydra databases table by table, for "identical results" proofs.

    py tools\compare_db.py A.db B.db

Prints one line per table, "<table>: N rows compared, M differ", then the
first few differing keys under any table that differs. Exits 1 when any
table differs, 2 on a usage error, 0 when everything matches.

Proving identical results (the recipe this script belongs to)
-------------------------------------------------------------

This recipe comes from the speedups plan,
docs/superpowers/plans/2026-10-06-perf-speedups.md (removed; git show 444631e6:docs/superpowers/plans/2026-10-06-perf-speedups.md),
section "Proving identical results". Its commands and rules follow, minus the
list of tables (the script reads them itself) and a warning about hash numbers
in old notes. Two facts changed after the plan was written, and they come
first, because they override it.

First, one run is the whole correctness check. A fresh-database hydra_batch
run, compared table by table with this script against the baseline's fresh
database, goes through the scan, the parse, the dynamics count, the engine and
the writer, so it covers every stored table. Extra engine and parse hashes,
other-difficulty parses, rescans and real-database re-analyses only re-check
the same output; below, those are the per-chart hashes, the scan comparison
and the `--redo` run used as a check. The `--redo` run still times a speedup.
The user called them "completely unnecessary" (docs/handoffs/2026-10-06-perf-speedups-wave2-handoff.md (removed; git show 444631e6:docs/handoffs/2026-10-06-perf-speedups-wave2-handoff.md),
"Read this first: how the user wants checks run").

Second, hydra_batch given no chart folder now refuses to run unless it also
gets --whole-library (src/cli/batch_scope.h owns that rule). A whole-library
run needs the user's explicit permission in chat first (CLAUDE.md, agent
rule 8). That covers every run below over the user's library: the
fresh-database run, the `--redo` run and the hydra_bench runs over
`C:\Clone Hero`. Each happens only when the user allows it. Without that, run
the same compare on the checked-in corpus by naming testdata\input as the
chart folder.

A speedup counts only if the output is identical.

The exes. Two sets: the baseline and the task's. The main session builds the
baseline once at dispatch, from the exact commit the wave forks from, in a
detached throwaway worktree through `tools/build_slot.ps1`, and copies
`hydra_batch.exe` and `hydra_bench.exe` to
`C:\Users\Patrick\.claude\hooks\state\bench\baseline-<short hash>\`, so five
agents do not each cold-build a baseline. The task builds its own
`hydra_batch` and `hydra_bench` by `-Target` in its worktree (`hydra_bench` is
EXCLUDE_FROM_ALL, so a plain build never rebuilds it).

The settings. `hydra_batch` and `hydra_bench` read `hydra_settings.ini` from
the folder they run in. Both exe folders get a copy of the installed
`C:\Program Files\Hydra\hydra_settings.ini` with its `dm_last_user` line
removed, and the brief has the agent confirm, by reading the ini against the
key table in `src/app/config.cpp`, that it says Expert, Pro Drums on, 2x Bass
on, depth 4 scores, Path limit 10 ms, SP cap 4 and the folder `C:\Clone Hero`.
Never guess a key name.

Fresh-database run. Through the lock, `hydra_batch.exe --db <scratch>\A.db`
with no folder argument (D79 item 1: with no folder arguments the tool scans
the ini's folders and saves the library, so the `charts` table is written
too), once per exe set. Then `py tools\compare_db.py A.db B.db`. It compares
every content column of every table, leaving out the per-file ids listed
below; `charts` is compared row by row in insert order, because a scan's
order is part of its answer (D63: the first-listed copy names a duplicate).
The expected line for each table is "N rows compared, 0 differ". Any
difference is a finding to report, never something to fix by loosening the
comparison.

Per-chart hashes. `hydra_bench --engine "C:\Clone Hero" --out rows.txt`
prints one hash over every prepared row (structure, nodes, best path,
summary) and writes a per-chart line; `hydra_bench --parse "C:\Clone Hero"
--out parse.tsv` does the same for the whole parsed song plus its dynamics
blob, and records a failure's exception type and message. Both are
single-threaded. Run baseline and task through the lock and compare the
printed hash and the per-chart files. Always A against B from the same
harness build.

Scan comparison. `hydra_bench --scan "C:\Clone Hero" --db <scratch>\scanA.db`
for each exe set, then `py tools\compare_db.py scanA.db scanB.db`, which
already compares `charts` in insert order. A second run on the same database
exercises the rescan cache; compare that too.

Real database with `--redo`. Timing of analysis runs on a copy of the user's
real GUI database, never only on a fresh one, because a fresh database once
hid a 69-second slowdown. Hydra is usually running, so the copy is taken
through SQLite's backup API, which reads a consistent snapshot while the live
connection keeps writing, and leaves a single-file copy with no `-wal`:
`py -c "import sqlite3; s = sqlite3.connect(r'file:C:/Program Files/Hydra/hydra.db?mode=ro', uri=True); d = sqlite3.connect(r'<scratch>\real.db'); s.backup(d); d.close(); s.close()"`.
Keep that copy pristine and copy it again for each run. Then
`hydra_batch.exe --db <copy> --redo` through the lock.

The lock. `pwsh -NoProfile -File tools\bench_run.ps1 -Label "<task>-<what>"
-Script <your timing .ps1>`. It waits for the lock and for compilers and other
Hydra processes to quiet down, runs the script, and logs the run with the
compilers-busy count. Call it with the PowerShell tool's timeout at 600000 ms
and keep each timing script under five minutes. Exit code 3 means the lock
stayed busy: do other work and retry.

What it compares, and what it leaves out on purpose:

- Columns come from PRAGMA table_info, and keys from each table's primary key
  or unique index, so this script holds no copy of the store's schema.
- result_id is left out. It is only the order the batch's worker threads
  finished in. A results row is matched by its unique key instead, and a
  path_refs row by its result's unique key in place of the result_id.
- A profile's id is left out the same way, because each file numbers its
  profiles in the order it first saw them. A profiles row is matched by its
  unique key, and a records or record_paths row by its profile's unique key in
  place of the profile_id. OWN_IDS and REFERENCES list these ids.
- A table with no primary key (charts) is compared row by row in rowid order,
  because the order a scan saved its rows in is part of its answer.

The comparison runs inside SQLite (B is attached to A), so the whole library
never has to fit in Python's memory.

    py tools\compare_db.py --summary-only baseline.db new.db

--summary-only compares only what a summary-only store keeps (ADR 0026,
docs/adr/0026-the-store-keeps-summaries-the-engine-gives-details.md), so a
baseline from before that change still compares against a newer file.
results is compared on the columns both files share, less SUMMARY_LEFT_OUT,
with rows matched by its unique key. The other tables in SUMMARY_TABLES are
compared as above. It prints the tables and results columns it skipped.
"""

import sqlite3
import sys
from pathlib import Path

TABLES = [
    "results", "paths", "path_refs", "songmeta", "dynamics", "charts", "meta",
    "profiles", "records", "record_paths",
]
SUMMARY_ONLY = "--summary-only"
# What a summary-only store keeps.
SUMMARY_TABLES = ["results", "charts", "meta", "profiles", "records", "record_paths"]
SUMMARY_LEFT_OUT = ["result_id", "structure"]  # results columns never compared there
SHOWN = 5  # differing keys listed per table
RESULT_ID = "result_id"
ROW_NO = "row_no"  # the positional key of a table with no primary key
# Each table whose primary key is a per-file id, and that id's column.
OWN_IDS = {"results": RESULT_ID, "profiles": "id"}
# Each column that holds another table's per-file id, and that table. A row is
# matched by the referred row's unique key, its columns named with this
# column's prefix (result_id -> result_hyhash).
REFERENCES = {RESULT_ID: "results", "profile_id": "profiles"}


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
    """The columns of the table's first unique index that does not use its
    per-file id (OWN_IDS, result_id by default), or None when it has none."""
    own_id = OWN_IDS.get(table, RESULT_ID)
    for idx in con.execute(f"PRAGMA {schema}.index_list({ident(table)})").fetchall():
        name, unique = idx[1], idx[2]
        if not unique:
            continue
        cols = [r[2] for r in con.execute(f"PRAGMA {schema}.index_info({ident(name)})")]
        if cols and own_id not in cols:
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
    own_id = OWN_IDS.get(table, RESULT_ID)
    if pk == [own_id]:
        keys = unique_key(con, "main", table)
        if keys is None:
            raise SystemExit(f"{table}: keyed by {own_id} alone and has no other unique key")
        values = [c for c in names if c != own_id and c not in keys]
        cols = ", ".join(ident(c) for c in keys + values)
        return Plan(keys, values, lambda s: f"SELECT {cols}, 1 AS present_ FROM {s}.{ident(table)}")

    refs = [c for c in pk if c in REFERENCES]
    if refs:
        keys, joined, joins = [], [], []
        for n, ref in enumerate(refs):
            target = REFERENCES[ref]
            rkeys = unique_key(con, "main", target)
            if rkeys is None:
                raise SystemExit(f"{table}: refers to {target}, which has no unique key")
            prefix = ref.removesuffix("_id") + "_"
            alias = f"r{n}"
            keys += [prefix + c for c in rkeys]
            joined += [f"{alias}.{ident(c)} AS {ident(prefix + c)}" for c in rkeys]
            joins.append((alias, target, ref))
        own_keys = [c for c in pk if c not in refs]
        keys += own_keys
        values = [c for c in names if c not in refs and c not in own_keys]
        rcols = ", ".join(joined)
        tcols = ", ".join(f"t.{ident(c)}" for c in own_keys + values)

        def select(s):
            join_text = "".join(
                f" LEFT JOIN {s}.{ident(target)} {alias}"
                f" ON {alias}.{ident(OWN_IDS[target])} = t.{ident(ref)}"
                for alias, target, ref in joins
            )
            return f"SELECT {rcols}, {tcols}, 1 AS present_ FROM {s}.{ident(table)} t{join_text}"

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

    return compare_rows(con, table, plan_for(con, table, names_a, pk_a))


def compare_rows(con, table, plan):
    """Prints the table's line (and its first differing keys) for rows read by
    `plan` on each side; returns True when the table matches."""
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


def table_names(con, schema):
    rows = con.execute(
        f"SELECT name FROM {schema}.sqlite_master"
        " WHERE type = 'table' AND name NOT LIKE 'sqlite\\_%' ESCAPE '\\'"
    ).fetchall()
    return {row[0] for row in rows}


def compare_summary_results(con):
    """The results table under --summary-only: the columns both files share,
    less SUMMARY_LEFT_OUT, with rows matched by the unique key."""
    table = "results"
    if not (table_exists(con, "main", table) and table_exists(con, "b", table)):
        return compare_table(con, table)  # reports the side it is missing from
    keys_a, keys_b = unique_key(con, "main", table), unique_key(con, "b", table)
    if keys_a is None or keys_a != keys_b:
        print(f"{table}: 0 rows compared, 1 differ (unique keys differ)")
        print(f"  A: {', '.join(keys_a or [])}")
        print(f"  B: {', '.join(keys_b or [])}")
        return False
    names_a, _ = table_columns(con, "main", table)
    names_b, _ = table_columns(con, "b", table)
    shared = [c for c in names_a if c in names_b and c not in SUMMARY_LEFT_OUT]
    skipped = sorted((set(names_a) | set(names_b)) - set(shared))
    print(f"skipped {table} columns: {', '.join(skipped) or 'none'}")
    values = [c for c in shared if c not in keys_a]
    cols = ", ".join(ident(c) for c in keys_a + values)
    plan = Plan(keys_a, values, lambda s: f"SELECT {cols}, 1 AS present_ FROM {s}.{ident(table)}")
    return compare_rows(con, table, plan)


def compare_summary(con):
    """--summary-only: the tables in SUMMARY_TABLES; names every other table."""
    others = (table_names(con, "main") | table_names(con, "b")) - set(SUMMARY_TABLES)
    print(f"skipped tables: {', '.join(sorted(others)) or 'none'}")
    all_match = compare_summary_results(con)
    for table in SUMMARY_TABLES:
        if table != "results" and not compare_table(con, table):
            all_match = False
    return all_match


def main(argv):
    flags = [a for a in argv[1:] if a.startswith("--")]
    paths = [a for a in argv[1:] if not a.startswith("--")]
    if len(paths) != 2 or any(f != SUMMARY_ONLY for f in flags):
        print(f"usage: py tools\\compare_db.py [{SUMMARY_ONLY}] A.db B.db", file=sys.stderr)
        return 2
    # Read-only URIs, so a comparison can never write to either database.
    a_uri, b_uri = (Path(p).resolve().as_uri() + "?mode=ro" for p in paths)
    try:
        con = sqlite3.connect(a_uri, uri=True)
        con.execute("ATTACH DATABASE ? AS b", (b_uri,))
    except sqlite3.Error as e:
        print(f"cannot open the databases: {e}", file=sys.stderr)
        return 2
    if flags:
        all_match = compare_summary(con)
    else:
        all_match = True
        for table in TABLES:
            if not compare_table(con, table):
                all_match = False
    con.close()
    return 0 if all_match else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
