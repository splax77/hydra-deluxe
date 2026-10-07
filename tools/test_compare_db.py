"""Tests for tools/compare_db.py, the whole-library "identical results" check.

Each test builds two tiny databases by hand and runs the script on them the
way a task does, reading its printed lines and its exit code. The tables
below are a cut-down stand-in for the store's, enough to give compare_db the
shapes it has to handle: a table keyed by result_id, a table that refers to
result_id, a plain keyed table and the scan-order charts table.
"""

import sqlite3
import subprocess
import sys
from pathlib import Path

SCRIPT = Path(__file__).with_name("compare_db.py")

SCHEMA = """
CREATE TABLE results (
  result_id INTEGER PRIMARY KEY, hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,
  sp_cap INTEGER NOT NULL, bestpath TEXT NOT NULL, structure BLOB NOT NULL,
  score INTEGER,
  UNIQUE (hyhash, chartmode, sp_cap));
CREATE TABLE paths (
  hyhash TEXT NOT NULL, chartmode TEXT NOT NULL, phash TEXT NOT NULL,
  payload BLOB NOT NULL, PRIMARY KEY (hyhash, chartmode, phash));
CREATE TABLE path_refs (
  result_id INTEGER NOT NULL, hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,
  phash TEXT NOT NULL, PRIMARY KEY (result_id, phash));
CREATE TABLE charts (md5 TEXT, name TEXT, path TEXT);
CREATE TABLE meta (key TEXT PRIMARY KEY, value TEXT);
"""

# Two charts' results, each with one path node.
RESULTS = [
    ("aaaa", "drums", 4, "best-a", b"\x01\x02", 1000),
    ("bbbb", "drums", 4, "best-b", b"\x03\x04", 2000),
]


def make_db(path, order, payload_b=b"\x10\x11\x12"):
    """Write a tiny store. `order` lists RESULTS indexes in the order they are
    inserted, so the result_id each chart gets depends on it, the way worker
    threads finishing in another order change it in a real batch."""
    con = sqlite3.connect(path)
    con.executescript(SCHEMA)
    for i in order:
        hyhash, mode, cap, best, structure, score = RESULTS[i]
        cur = con.execute(
            "INSERT INTO results (hyhash, chartmode, sp_cap, bestpath, structure, score)"
            " VALUES (?, ?, ?, ?, ?, ?)",
            (hyhash, mode, cap, best, structure, score),
        )
        payload = payload_b if hyhash == "bbbb" else b"\x20\x21"
        con.execute(
            "INSERT INTO paths VALUES (?, ?, ?, ?)", (hyhash, mode, "p-" + hyhash, payload)
        )
        con.execute(
            "INSERT INTO path_refs VALUES (?, ?, ?, ?)",
            (cur.lastrowid, hyhash, mode, "p-" + hyhash),
        )
    con.executemany(
        "INSERT INTO charts VALUES (?, ?, ?)",
        [("aaaa", "Song A", "a/notes.chart"), ("bbbb", "Song B", "b/notes.mid")],
    )
    con.execute("INSERT INTO meta VALUES ('version', '1')")
    con.commit()
    con.close()


# The summary-only layout: results without structure, and no detail tables.
NEW_SCHEMA = """
CREATE TABLE results (
  result_id INTEGER PRIMARY KEY, hyhash TEXT NOT NULL, chartmode TEXT NOT NULL,
  sp_cap INTEGER NOT NULL, bestpath TEXT NOT NULL, score INTEGER,
  UNIQUE (hyhash, chartmode, sp_cap));
CREATE TABLE charts (md5 TEXT, name TEXT, path TEXT);
CREATE TABLE meta (key TEXT PRIMARY KEY, value TEXT);
"""

# The detail tables an old file also held, beyond paths and path_refs.
OLD_DETAIL_SCHEMA = """
CREATE TABLE songmeta (hyhash TEXT PRIMARY KEY, song_length_ms INTEGER);
CREATE TABLE dynamics (hyhash TEXT PRIMARY KEY, ghosts INTEGER);
INSERT INTO songmeta VALUES ('aaaa', 120000);
INSERT INTO dynamics VALUES ('aaaa', 7);
"""


def make_old_db(path, order):
    """An old-layout store: make_db's tables plus songmeta and dynamics."""
    make_db(path, order)
    con = sqlite3.connect(path)
    con.executescript(OLD_DETAIL_SCHEMA)
    con.commit()
    con.close()


def make_new_db(path, order, score_b=2000):
    """A summary-only store holding the same summaries as make_db's, with
    chart bbbb's score set to score_b."""
    con = sqlite3.connect(path)
    con.executescript(NEW_SCHEMA)
    for i in order:
        hyhash, mode, cap, best, _structure, score = RESULTS[i]
        if hyhash == "bbbb":
            score = score_b
        con.execute(
            "INSERT INTO results (hyhash, chartmode, sp_cap, bestpath, score)"
            " VALUES (?, ?, ?, ?, ?)",
            (hyhash, mode, cap, best, score),
        )
    con.executemany(
        "INSERT INTO charts VALUES (?, ?, ?)",
        [("aaaa", "Song A", "a/notes.chart"), ("bbbb", "Song B", "b/notes.mid")],
    )
    con.execute("INSERT INTO meta VALUES ('version', '1')")
    con.commit()
    con.close()


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


def test_result_id_order_alone_compares_clean(tmp_path):
    a, b = tmp_path / "A.db", tmp_path / "B.db"
    make_db(a, [0, 1])
    make_db(b, [1, 0])  # same rows, result_ids swapped
    code, out = run_compare(a, b)
    assert code == 0, out
    assert table_line(out, "results") == "results: 2 rows compared, 0 differ"
    assert table_line(out, "path_refs") == "path_refs: 2 rows compared, 0 differ"
    assert table_line(out, "paths") == "paths: 2 rows compared, 0 differ"
    assert table_line(out, "charts") == "charts: 2 rows compared, 0 differ"
    assert table_line(out, "meta") == "meta: 1 rows compared, 0 differ"


def test_tables_print_in_the_fixed_order(tmp_path):
    a, b = tmp_path / "A.db", tmp_path / "B.db"
    make_db(a, [0, 1])
    make_db(b, [0, 1])
    _, out = run_compare(a, b)
    names = [line.split(":")[0] for line in out.splitlines() if not line.startswith(" ")]
    assert names == ["results", "paths", "path_refs", "songmeta", "dynamics", "charts", "meta"]


def test_one_changed_payload_byte_is_reported(tmp_path):
    a, b = tmp_path / "A.db", tmp_path / "B.db"
    make_db(a, [0, 1])
    make_db(b, [0, 1], payload_b=b"\x10\x11\x13")
    code, out = run_compare(a, b)
    assert code == 1, out
    assert table_line(out, "paths") == "paths: 2 rows compared, 1 differ"
    assert "bbbb" in out
    assert table_line(out, "results") == "results: 2 rows compared, 0 differ"


def test_chart_scan_order_is_part_of_the_answer(tmp_path):
    a, b = tmp_path / "A.db", tmp_path / "B.db"
    make_db(a, [0, 1])
    make_db(b, [0, 1])
    con = sqlite3.connect(b)
    con.execute("DELETE FROM charts")
    con.executemany(
        "INSERT INTO charts VALUES (?, ?, ?)",
        [("bbbb", "Song B", "b/notes.mid"), ("aaaa", "Song A", "a/notes.chart")],
    )
    con.commit()
    con.close()
    code, out = run_compare(a, b)
    assert code == 1, out
    assert table_line(out, "charts") == "charts: 2 rows compared, 2 differ"


def test_summary_only_old_layout_matches_new_layout(tmp_path):
    a, b = tmp_path / "old.db", tmp_path / "new.db"
    make_old_db(a, [0, 1])
    make_new_db(b, [1, 0])  # result_ids swapped too
    code, out = run_compare(a, b, "--summary-only")
    assert code == 0, out
    assert table_line(out, "results") == "results: 2 rows compared, 0 differ"
    assert table_line(out, "charts") == "charts: 2 rows compared, 0 differ"
    assert table_line(out, "meta") == "meta: 1 rows compared, 0 differ"
    assert table_line(out, "skipped tables") == (
        "skipped tables: dynamics, path_refs, paths, songmeta"
    )
    assert table_line(out, "skipped results columns") == (
        "skipped results columns: result_id, structure"
    )


def test_summary_only_reports_one_changed_score(tmp_path):
    a, b = tmp_path / "old.db", tmp_path / "new.db"
    make_old_db(a, [0, 1])
    make_new_db(b, [0, 1], score_b=2001)
    code, out = run_compare(a, b, "--summary-only")
    assert code == 1, out
    assert table_line(out, "results") == "results: 2 rows compared, 1 differ"
    assert "hyhash='bbbb'" in out


if __name__ == "__main__":
    import pytest

    sys.exit(pytest.main([__file__, "-q"]))
