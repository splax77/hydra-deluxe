# Storage plan, Task 0: the two measurements

Status: done. Both numbers are far under their lines, so wave 1 goes ahead without asking the user again.

## 1. How long the path report's extra analysis pass will take

Estimate: about 2.4 seconds. The line is 15 seconds, so it is far under.

How it was worked out. `hydra_bench --engine "C:\Clone Hero"` runs every chart once, one at a time, and writes nothing. That took 19.14 seconds of wall time on the whole library (18,811 charts, 58 failed to load, the same 58 the batch skips). The batch uses `app::batch_worker_count()` workers (src/app/analysis.cpp). This machine has 16 logical cores, so it asks for 15 and the cap of 8 applies: 8 workers. 19.14 s divided by 8 is 2.39 s.

Two cautions. The 19.14 s includes about 2.5 s of folder scanning and file hashing that the report pass will not repeat; the bench's own phase sums add to 16.64 s (parse 8.136, graph 2.284, analyze 5.801, prepare 0.418). Dividing 16.64 by 8 gives 2.08 s. The 2.4 s is therefore the safe (higher) figure. And the bench run builds a throwaway score graph and prepares a row for every chart, so it is a fair stand-in for "analyse once more", not an exact copy of the report pass. The join measures the real pass.

Where it came from:
- Command (through the bench lock): `pwsh -NoProfile -File tools\bench_run.ps1 -Label "storage-T0-engine" -Script <scratch>\t0\engine.ps1`. The script runs `hydra_bench.exe --engine "C:\Clone Hero"` from a scratch folder holding a copy of the exe and the settings file.
- Bench log line (`~\.claude\hooks\state\bench\bench_log.md`): `08:00:15 storage-T0-engine (exit 0, 19.3 s, compilers busy at start: 0)`, then `charts 18811 failed 58 | parse 8.136s | graph 2.284s | analyze x1 5.801s | prepare 0.418s | hash 8d17f958172bd6f4`.
- Build: hydra_bench from main's working tree at 1fa82b5 (docs-only on top of d63d1d8), `.\build_cpp.ps1 -Target hydra_bench`. It was already up to date.
- Settings: a copy of `C:\Program Files\Hydra\hydra_settings.ini` with the `dm_last_user` line removed. The key names were checked against the key table in src/app/config.cpp. It says Expert (`view_difficulty`), Pro Drums on (`view_prodrums=1`), 2x Bass on (`view_bass2x=1`), depth 4 scores (`depth_value=4`, `depth_mode=0`), Path limit 10 ms (`mslimit_enabled=1`, `mslimit_value=10`), SP cap 4 (`sp_cap=4`), folder `C:\Clone Hero`.

## 2. How long the first open of the new Hydra will take to shrink the file

0.41 seconds, and the file goes from 292,372,480 bytes to 13,033,472 bytes (13.0 MB). The line is 10 seconds, so it is far under, and the file is smaller than the 20 MB estimate.

The steps took: dropping the four tables and the column, 0.310 s; VACUUM, 0.098 s; the WAL checkpoint, 0.001 s. No `-wal` or `-shm` file was left afterwards. Before the upgrade the copy held 19,106 results, 91,196 paths, 91,196 path_refs, 19,099 songmeta and 18,820 dynamics rows. Bench log line: `storage-T0-upgrade`, started 08:02:29, compilers busy at start: 0.

How it was run. A backup-API copy of the installed database is at `<scratch>\t0\real.db`, 292,372,480 bytes (the live file is 292,229,120 bytes plus a 45.6 MB log file; the backup folds the log in). The backup took 0.76 s. The upgrade script is `<scratch>\t0\upgrade.py`, run through `<scratch>\t0\upgrade.ps1`. It copies real.db to real-upgraded.db, then in one transaction drops the paths, path_refs, songmeta and dynamics tables and the results.structure column, runs VACUUM, runs the WAL checkpoint, and prints the time of each step and the file sizes.

`<scratch>` is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\16f8c684-985c-4209-9849-49818baf2140\scratchpad`.

The main session ran it with `pwsh -NoProfile -File tools\bench_run.ps1 -Label "storage-T0-upgrade" -Script <scratch>\t0\upgrade.ps1`. The Task 0 agent couldn't, because the PowerShell hook denies table-dropping commands inside a subagent, where nobody can answer a permission prompt.
