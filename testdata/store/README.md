# Real old databases (upgrade fixtures)

These four files are databases written by real Hydra releases. The store tests open them with today's `RecordStore` to prove an old user's file upgrades honestly. Nothing here was written by hand: each file is what an old `hydra_batch` wrote, shrunk with SQLite's `VACUUM INTO`.

The files are read-only fixtures. A test copies one to a temp path before it opens it. Today's store opens a file in WAL mode (write-ahead logging: SQLite keeps new writes in a side file named `-wal`, with an index file named `-shm`), so opening the checked-in file directly would write those two files beside it and change it.

## The four files

| File | Release | Commit | Bytes |
|---|---|---|---|
| `v1.8.4-schema2.db` | `v1.8.4` | `031d21874d687f13097df84a57de077eb02866cb` | 73,728 |
| `v1.8.4-schema2-legacy-fills.db` | `v1.8.4`, run with `--legacy-fills` | `031d21874d687f13097df84a57de077eb02866cb` | 73,728 |
| `v2.0.0-schema3.db` | `v2.0.0` | `5be59569a880696dec1b3f1c6d89660349e75dc4` | 73,728 |
| `v2.1.0-schema4.db` | `v2.1.0` | `bc5828247beb284187e7c48b7d9774cf18c4505c` | 77,824 |

The set is 299,008 bytes, under the 1 MB cap, and each file is under the 256 KB cap. The commit is `git rev-parse <tag>^{commit}`. Each file was made on 2026-10-10.

## The three charts

All three come from the checked-in corpus. Each was copied with its `song.ini` into its own song folder inside one charts folder, and every run read that same folder.

| Chart file | Corpus path (under `testdata/input/common/`) | hyhash |
|---|---|---|
| `.chart` | `IB24/T2/Stone Temple Pilots - Trippin on a Hole in a Paper Heart/notes.chart` | `0b647b1570475f18d463466f2c4d72a7` |
| `.mid` | `Summer Blast _25 Setlist/Tier 2/Dance Gavin Dance - Young Robot/notes.mid` | `0d4fd734fe5f2bf6ff1fdbc4ff55be8f` |
| `.mid` | `Summer Blast _25 Setlist/Tier 2/Thornhill - Limbo/notes.mid` | `bef0759746740f9c1045f64e9a5a2938` |

The plan asked for one `.sng` chart. The corpus holds no `.sng` file, so the set is one `.chart` and two `.mid` instead.

They are the shortest real songs of each kind in the corpus. Every release analysed all three: each run printed "Analyzed 3, skipped 0 already stored, 0 failed". Limbo and Young Robot have no Star Power activations, so their best path has none ("(No activations.)" in the batch output). They are still full result rows with a structure blob.

## How they were made

Each old release's `hydra_batch` was built from its tag, in its own detached checkout of that tag. Their `Release` folders hold no settings file and no `hydra_rules.ini`, so every run used each release's default settings and rules. Every run printed the same settings: chart mode "Expert Pro Drums, 2x Bass", depth "scores 4", SP cap 4 bars, timing cap 10 ms, squeeze window 500 ms. The fill rule was Clone Hero 1.1, or "Clone Hero 1.0 (legacy)" for the `--legacy-fills` run.

All three tags take `--db` and `--legacy-fills` (checked with `git show <tag>:src/cli/batch.cpp`). Each command ran from the release's own `build-cpp\Release` folder, into a fresh database:

```
v1.8.4:         .\hydra_batch.exe --db C:\Users\Patrick\Downloads\Hydra\tf-old\db\v1.8.4.db C:\Users\Patrick\Downloads\Hydra\tf-old\charts
v1.8.4 legacy:  .\hydra_batch.exe --legacy-fills --db C:\Users\Patrick\Downloads\Hydra\tf-old\db\v1.8.4-legacy-fills.db C:\Users\Patrick\Downloads\Hydra\tf-old\charts
v2.0.0:         .\hydra_batch.exe --db C:\Users\Patrick\Downloads\Hydra\tf-old\db\v2.0.0.db C:\Users\Patrick\Downloads\Hydra\tf-old\charts
v2.1.0:         .\hydra_batch.exe --db C:\Users\Patrick\Downloads\Hydra\tf-old\db\v2.1.0.db C:\Users\Patrick\Downloads\Hydra\tf-old\charts
```

Then each database was folded and shrunk into this folder with Python's sqlite3 (`py`):

```
py -I -c "import sqlite3,sys; sqlite3.connect(sys.argv[1]).execute('VACUUM INTO ?', (sys.argv[2],))" <old db> testdata\store\<file>.db
```

`VACUUM INTO` keeps `user_version`. The written file is in rollback-journal mode (`PRAGMA journal_mode` reads `delete`), not WAL.

To regenerate, build the tag, make the charts folder from the three corpus paths above, and run the same commands. Compare by the facts below, not by file hash. Two things can differ on a fresh run: the order the batch's workers finish in decides which chart gets which `result_id` (the four files here already disagree on it), and SQLite's page layout can move.

## Facts the tests pin

Every number below was read from the checked-in file (a copy of it, so nothing wrote beside it) on 2026-10-10.

### Schema and stamps

| | v1.8.4-schema2 | v1.8.4-schema2-legacy-fills | v2.0.0-schema3 | v2.1.0-schema4 |
|---|---|---|---|---|
| `PRAGMA user_version` | 2 | 2 | 3 | **0** |
| `legacy_fills` column | absent | absent | present | present |
| `rules_fp` column | absent | absent | absent | present |
| `stars` column | absent | absent | present | present |
| `structure` column | present | present | present | present |
| results rows | 3 | 3 | 3 | 3 |
| sorted `result_id`s | 1, 2, 3 | 1, 2, 3 | 1, 2, 3 | 1, 2, 3 |
| `hyversion` on every row | `1.8.2` | `1.8.2` | `1.8.2` | `2.1.0+allzero` |
| `chartmode` on every row | Expert Pro Drums, 2x Bass | same | same | same |
| `sp_cap` on every row | 4 | 4 | 4 | 4 |
| `meta` rows | `engine_mode = ch11` | `engine_mode = ch10` | `auto_results_deleted = 1`, `engine_mode = ch11` | `auto_results_deleted = 1`, `engine_mode = ch11` |
| `legacy_fills` per row | (no column) | (no column) | 0, 0, 0 | 0, 0, 0 |
| `charts` rows | 0 | 0 | 0 | 0 |
| `kDetailTablesCountSql` | 5 | 5 | 5 | 5 |

The v2.1.0 file says `user_version` 0, not 4. That is what the real release writes: commit `87a588d5` ("Store SQL says each thing once"), which is in v2.1.0, stopped setting `user_version` at all, and the file was created fresh by that build. A test that tells schema 4 apart should check for the `rules_fp` column, not `user_version`.

The schema 2 files have no `legacy_fills` column; `RecordStore::upgrade_results_key` owns how a row gets one. But today's open keeps none of the v1.8.4 rows: they have a score and no stars, and the copy upgrade leaves such rows out (ADR 0026 owns that rule). So after the open both v1.8.4 files hold 0 results rows, and nothing in them shows which fill rule a row would get.

The `charts` table exists in all four files but holds no rows. `hydra_batch` never fills it; the app's library scan does. So these files cannot pin "the charts rows keep their names". The song names live in `songmeta` instead, which is one of the detail tables the summary-only upgrade drops.

### Results rows by chart

`result_id` is not the same chart in every file. Pin rows by `hyhash`, or use the map below.

| File | result_id 1 | result_id 2 | result_id 3 |
|---|---|---|---|
| v1.8.4-schema2 | Limbo | Trippin | Young Robot |
| v1.8.4-schema2-legacy-fills | Limbo | Young Robot | Trippin |
| v2.0.0-schema3 | Limbo | Trippin | Young Robot |
| v2.1.0-schema4 | Limbo | Young Robot | Trippin |

The stored summary numbers per chart:

| Chart | score | actcount | notecount | pathcount | structure bytes |
|---|---|---|---|---|---|
| Limbo (`bef07597…`) | 315,350 | 0 | 1,375 | 1 | 128 |
| Young Robot (`0d4fd734…`) | 245,800 | 0 | 1,088 | 1 | 115 |
| Trippin (`0b647b15…`), Clone Hero 1.1 fills | 377,735 | 3 | 1,257 | 5 | 495 |
| Trippin (`0b647b15…`), legacy-fills file only | 379,655 | 4 | 1,257 | 5 | 419 |

Every row stores `ms_enabled` 1, `ms_value` 10, `depth_mode` 0 and `depth_value` 4.

### Rules fingerprint

An old row's rules fingerprint sits inside its `structure` blob, at the place `rules_fp_of` in `src/store/record_store.cpp` reads. These are the bytes it reads there.

Every row of every file holds the same 8 bytes, because every run used the default rules:

- bytes in file order: `f24c606966e9d270`
- the same bytes read as a little-endian 64-bit number: `0x70d2e96669604cf2`

The v2.1.0 file's `rules_fp` column already holds exactly those 8 bytes on all three rows. After the upgrade, `rules_fp` on every row the open keeps should equal them (the two v1.8.4 files keep no rows).

The first 4 bytes of the blob (the path format) are `06000000` in the three older files and `07000000` in the v2.1.0 file.

### Detail tables

`kDetailTablesCountSql` (in `tests/old_layout_fixture.h`) counts which of its five named tables or indexes exist. It reads 5 in all four files, not only in v2.1.0. Their row counts are the same in every file:

| Table | rows |
|---|---|
| `paths` | 7 |
| `path_refs` | 7 |
| `songmeta` | 3 |
| `dynamics` | 3 |

`path_refs_by_node` is an index, so it has no rows of its own.

### Song names in `songmeta`

| hyhash | `ref_name` | `ref_artist` | `ref_charter` |
|---|---|---|---|
| `0b647b1570475f18d463466f2c4d72a7` | Trippin' on a Hole in a Paper Heart | Stone Temple Pilots | Advanst |
| `0d4fd734fe5f2bf6ff1fdbc4ff55be8f` | Young Robot | Dance Gavin Dance | Boo |
| `bef0759746740f9c1045f64e9a5a2938` | Limbo | Thornhill | Benimaru |

All four files hold these same three rows.
