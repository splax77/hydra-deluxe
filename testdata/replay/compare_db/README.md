# Record-layout databases for tools/test_compare_db.py

These three files are what today's `hydra_batch` writes. `tools/test_compare_db.py` compares them with `tools/compare_db.py`, to check the record tables (`profiles`, `records`, `record_paths`) on real data. Nothing here is written by hand.

| File | What it holds |
|---|---|
| `profiles-default-only.db` | The three charts under the default settings only |
| `profiles-default-first.db` | The same, then the same charts again with `--legacy-fills` |
| `profiles-legacy-first.db` | The two runs in the other order |

So the last two files hold the same two profiles, numbered the other way round. The first file lacks the legacy-fills profile and its records.

## The charts

The same three corpus charts `testdata/store/README.md` uses, under `testdata/input/common/`:

- `IB24/T2/Stone Temple Pilots - Trippin on a Hole in a Paper Heart`
- `Summer Blast _25 Setlist/Tier 2/Dance Gavin Dance - Young Robot`
- `Summer Blast _25 Setlist/Tier 2/Thornhill - Limbo`

## How they are made

Run `hydra_batch.exe` from a folder that holds no `hydra_settings.ini`, no `hydra_rules.ini` and no `hydra.db`, so every run uses the default settings and rules. `<T>`, `<Y>` and `<L>` are the three song folders above, and `<scratch>` is any empty folder.

```
hydra_batch.exe --db <scratch>\a.db <T> <Y> <L>
py -I -c "import sqlite3,sys; sqlite3.connect(sys.argv[1]).execute('VACUUM INTO ?', (sys.argv[2],))" <scratch>\a.db testdata\replay\compare_db\profiles-default-only.db
hydra_batch.exe --legacy-fills --db <scratch>\a.db <T> <Y> <L>
py -I -c "import sqlite3,sys; sqlite3.connect(sys.argv[1]).execute('VACUUM INTO ?', (sys.argv[2],))" <scratch>\a.db testdata\replay\compare_db\profiles-default-first.db
hydra_batch.exe --legacy-fills --db <scratch>\b.db <T> <Y> <L>
hydra_batch.exe --db <scratch>\b.db <T> <Y> <L>
py -I -c "import sqlite3,sys; sqlite3.connect(sys.argv[1]).execute('VACUUM INTO ?', (sys.argv[2],))" <scratch>\b.db testdata\replay\compare_db\profiles-legacy-first.db
```

`VACUUM INTO` writes a single file in rollback-journal mode, so a read-only open writes nothing beside it. Regenerating gives the same rows, though not the same file bytes.
