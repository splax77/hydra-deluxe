"""Tests for tools/compare_db.py, the whole-library "identical results" check.

Every database these tests compare was written by a real Hydra build from
charts in the checked-in corpus. Each test runs the script on two of them the
way a task does, and reads its printed lines and its exit code.

The old-layout pair is in testdata/store; its README.md says which release
and command wrote each file. The two files hold the same three charts, run
once with Clone Hero 1.1 fills and once with --legacy-fills, so one chart's
analysis differs and the others' do not. The record-layout files are in
testdata/replay/compare_db; its README.md gives the hydra_batch commands that
write them.

An expected count is worked out here by reading both files with plain
sqlite3, row by row, never by re-running compare_db's own SQL.
"""

import re
import sqlite3
import subprocess
import sys
from pathlib import Path

import pytest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import compare_db  # noqa: E402

SCRIPT = HERE / "compare_db.py"
REPO = HERE.parent
STORE = REPO / "testdata" / "store"
CH11 = STORE / "v1.8.4-schema2.db"
LEGACY = STORE / "v1.8.4-schema2-legacy-fills.db"

RECORD_FILES = REPO / "testdata" / "replay" / "compare_db"
BOTH_DEFAULT_FIRST = RECORD_FILES / "profiles-default-first.db"
BOTH_LEGACY_FIRST = RECORD_FILES / "profiles-legacy-first.db"
DEFAULT_ONLY = RECORD_FILES / "profiles-default-only.db"


def run_compare(a, b, *flags):
    proc = subprocess.run(
        [sys.executable, str(SCRIPT), *flags, str(a), str(b)],
        capture_output=True,
        text=True,
    )
    return proc.returncode, proc.stdout


def table_line(stdout, table):
    for line in stdout.splitlines():
        if line.startswith(table + ":"):
            return line
    raise AssertionError(f"no line for {table} in:\n{stdout}")


def counts(stdout, table):
    """(rows compared, rows that differ) from a table's line."""
    line = table_line(stdout, table)
    m = re.match(re.escape(table) + r": (\d+) rows compared, (\d+) differ", line)
    assert m, line
    return int(m.group(1)), int(m.group(2))


def listed_keys(stdout, table):
    """The differing keys printed under a table's line."""
    lines = stdout.splitlines()
    start = lines.index(table_line(stdout, table)) + 1
    keys = []
    for line in lines[start:]:
        if not line.startswith("  "):
            break
        if not line.startswith("  ..."):
            keys.append(line)
    return keys


def read(db, sql, params=()):
    """Rows from a checked-in file, opened read-only so nothing writes beside it."""
    assert Path(db).exists(), f"{db} is missing; its folder's README.md says how it is made"
    con = sqlite3.connect(Path(db).resolve().as_uri() + "?mode=ro", uri=True)
    try:
        return con.execute(sql, params).fetchall()
    finally:
        con.close()


def tables(db):
    return {
        name
        for (name,) in read(
            db,
            "SELECT name FROM sqlite_master WHERE type = 'table'"
            " AND name NOT LIKE 'sqlite\\_%' ESCAPE '\\'",
        )
    }


def columns(db, table):
    return [row[1] for row in read(db, f"PRAGMA table_info({compare_db.ident(table)})")]


def keyed(db, sql, n_keys):
    """{first n_keys columns: the rest} for each row sql reads; fails on a
    repeated key, so a key that does not name one row is caught here."""
    rows = read(db, sql)
    out = {tuple(r[:n_keys]): tuple(r[n_keys:]) for r in rows}
    assert len(out) == len(rows), f"{sql} does not key one row each in {db}"
    return out


def differing(a, b):
    """Keys whose values differ between two keyed() reads, or that one lacks."""
    return {k for k in a.keys() | b.keys() if a.get(k) != b.get(k)}


def results_by_chart(db, left_out):
    """Each chart's results row, without the columns in left_out. Both files
    hold one row per chart, under one set of settings."""
    cols = [c for c in columns(db, "results") if c not in left_out and c != "hyhash"]
    sql = "SELECT hyhash, " + ", ".join(compare_db.ident(c) for c in cols) + " FROM results"
    return keyed(db, sql, 1)


def names_key(stdout, table, column, value):
    """True when a key printed under the table names column=value."""
    text = f"{column}={compare_db.show(value)}"
    return any(text in key for key in listed_keys(stdout, table))


def test_result_id_order_alone_compares_clean():
    # A chart whose analysis is the same in both files but whose result_id is
    # not: the batch's workers finished in another order.
    ids_a = dict(read(CH11, "SELECT hyhash, result_id FROM results"))
    ids_b = dict(read(LEGACY, "SELECT hyhash, result_id FROM results"))
    own_id = compare_db.OWN_IDS["results"]
    rows_a = results_by_chart(CH11, [own_id])
    rows_b = results_by_chart(LEGACY, [own_id])
    changed = differing(rows_a, rows_b)
    renumbered = {h for h in ids_a if ids_a[h] != ids_b.get(h) and (h,) not in changed}
    assert renumbered, "the two files hold no chart that only changed its result_id"

    # Each path_refs row, named by its result's chart in place of the result_id.
    refs_sql = (
        "SELECT r.hyhash, p.phash, p.hyhash, p.chartmode FROM path_refs p"
        f" JOIN results r ON r.{own_id} = p.{own_id}"
    )
    refs_changed = differing(keyed(CH11, refs_sql, 2), keyed(LEGACY, refs_sql, 2))

    code, out = run_compare(CH11, LEGACY)
    assert code == 1, out  # the chart whose fills differ is a real difference
    assert counts(out, "results") == (len(rows_a.keys() | rows_b.keys()), len(changed)), out
    assert counts(out, "path_refs")[1] == len(refs_changed), out
    for h in renumbered:
        assert not names_key(out, "results", "hyhash", h), out
        assert not names_key(out, "path_refs", "result_hyhash", h), out


def test_every_table_a_real_store_holds_is_compared_once():
    # Every real store generation checked in, old layouts and the record layout.
    held = set().union(*(tables(db) for db in sorted(STORE.glob("*.db"))))
    assert held <= set(compare_db.TABLES), sorted(held - set(compare_db.TABLES))

    _, out = run_compare(CH11, LEGACY)
    names = [line.split(":")[0] for line in out.splitlines() if not line.startswith(" ")]
    assert sorted(names) == sorted(compare_db.TABLES), out
    assert len(set(names)) == len(names), out


def test_one_changed_payload_is_reported():
    sql = "SELECT hyhash, chartmode, phash, payload FROM paths"
    changed = differing(keyed(CH11, sql, 3), keyed(LEGACY, sql, 3))
    assert changed, "the two files hold the same path payloads"
    charts_changed = {h for h, _mode, _phash in changed}

    code, out = run_compare(CH11, LEGACY)
    assert code == 1, out
    assert counts(out, "paths")[1] == len(changed), out
    for key in listed_keys(out, "paths"):
        assert any(f"hyhash={compare_db.show(h)}" in key for h in charts_changed), key


def test_summary_only_leaves_out_detail_tables_and_columns():
    detail = (tables(CH11) | tables(LEGACY)) - set(compare_db.SUMMARY_TABLES)
    assert detail, "neither file holds a table the summary-only compare leaves out"

    _, plain = run_compare(CH11, LEGACY)
    code, out = run_compare(CH11, LEGACY, compare_db.SUMMARY_ONLY)
    skipped = table_line(out, "skipped tables").split(": ", 1)[1].split(", ")
    assert set(skipped) == detail, out
    for table in detail:
        table_line(plain, table)  # the plain compare does read it
        assert not any(line.startswith(table + ":") for line in out.splitlines()), out
    left_out = table_line(out, "skipped results columns").split(": ", 1)[1].split(", ")
    assert set(compare_db.SUMMARY_LEFT_OUT) <= set(left_out), out


def test_summary_only_reports_a_changed_score():
    left_out = compare_db.SUMMARY_LEFT_OUT
    assert columns(CH11, "results") == columns(LEGACY, "results")
    changed = {h for (h,) in differing(
        results_by_chart(CH11, left_out), results_by_chart(LEGACY, left_out))}
    scores_a = dict(read(CH11, "SELECT hyhash, score FROM results"))
    scores_b = dict(read(LEGACY, "SELECT hyhash, score FROM results"))
    rescored = {h for h in scores_a if scores_a[h] != scores_b.get(h)}
    assert rescored, "no chart's score differs between the two files"

    code, out = run_compare(CH11, LEGACY, compare_db.SUMMARY_ONLY)
    assert code == 1, out
    assert counts(out, "results")[1] == len(changed), out
    for h in rescored:
        assert names_key(out, "results", "hyhash", h), out


def profiles_by_settings(db):
    """{every profiles column but its id: that id}."""
    own_id = compare_db.OWN_IDS["profiles"]
    cols = [c for c in columns(db, "profiles") if c != own_id]
    sql = "SELECT " + ", ".join(compare_db.ident(c) for c in cols) + f", {own_id} FROM profiles"
    return {k: v for k, (v,) in keyed(db, sql, len(cols)).items()}


@pytest.mark.parametrize("flags", [[], [compare_db.SUMMARY_ONLY]])
def test_profile_id_order_alone_compares_clean(flags):
    ids_a = profiles_by_settings(BOTH_DEFAULT_FIRST)
    ids_b = profiles_by_settings(BOTH_LEGACY_FIRST)
    assert ids_a.keys() == ids_b.keys(), "the two files hold different profiles"
    assert ids_a != ids_b, "the two files number their profiles the same way"

    code, out = run_compare(BOTH_DEFAULT_FIRST, BOTH_LEGACY_FIRST, *flags)
    assert code == 0, out
    for table in ("profiles", "records", "record_paths"):
        assert counts(out, table)[1] == 0, out


def test_records_of_a_profile_one_file_lacks_are_reported():
    own_id = compare_db.OWN_IDS["profiles"]
    only_in_a = profiles_by_settings(BOTH_DEFAULT_FIRST).keys() - profiles_by_settings(
        DEFAULT_ONLY).keys()
    assert only_in_a, "the default-only file holds every profile the other file does"
    lacking = [profiles_by_settings(BOTH_DEFAULT_FIRST)[k] for k in only_in_a]
    marks = ", ".join("?" * len(lacking))
    record_songs = [s for (s,) in read(
        BOTH_DEFAULT_FIRST, f"SELECT song_id FROM records WHERE profile_id IN ({marks})", lacking)]
    songs = set(record_songs)
    (n_paths,) = read(
        BOTH_DEFAULT_FIRST, f"SELECT COUNT(*) FROM record_paths WHERE profile_id IN ({marks})",
        lacking)[0]
    assert songs, "the profile only one file holds has no records"
    legacy = read(
        BOTH_DEFAULT_FIRST, f"SELECT legacy_fills FROM profiles WHERE {own_id} IN ({marks})",
        lacking)

    code, out = run_compare(BOTH_DEFAULT_FIRST, DEFAULT_ONLY)
    assert code == 1, out
    assert counts(out, "profiles")[1] == len(only_in_a), out
    assert counts(out, "records")[1] == len(record_songs), out
    assert counts(out, "record_paths")[1] == n_paths, out
    for key in listed_keys(out, "records"):
        assert any(f"song_id={compare_db.show(s)}" in key for s in songs), key
        assert any(f"legacy_fills={compare_db.show(v)}" in key for (v,) in legacy), key


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
