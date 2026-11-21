Agent a3c752c45ea335d8b, task T5-CAP-RUNS, session 63c70014-4328-49dc-9205-742235732752.

# The capture runs: one run per kind of claim

The user chose "One run per kind". This file lists the runs. Nothing was run to write it: every command below was checked by reading the tool's own source or help text. The main session runs them, one at a time. A fresh agent per slice then reads each run's output against its records.

Paths: `REPO` is `C:\Users\Patrick\Downloads\Hydra\hydra-test` (the main checkout, where `build-cpp\Release` holds the built tools). `CAP` is `docs/audit/2026-11-14-test-audit/capture/runs` in the audit worktree. Every command is PowerShell, run from the audit worktree unless it says otherwise. `testdata/input` is the checked-in corpus (116 chart files); no run touches the user's library or real database.

Counts of records below come from the capture lists CL1 to CL4, which count by keyword and by hand. They are the scouts' counts, not a fresh read of the records, and a record can sit in two runs. The handoff's 2,800 is a text count of "Investigate, claim not checked only"; the lists total about 4,100 because they also count records that only mention the claim.

## Runs already done: what they cover

`records-engine/` (seven charts, dumped and scheduled at 85 and 60 ms) and `records-engine/window-sweep/sweep.tsv` (all 116 corpus charts, dumped at 85 and 60 ms under two settings sets) already settle one kind of claim: "the early-fill window moves this chart's kept paths, totals or notation". The answer is no, for every corpus chart, under those two settings sets. They serve these records, which need no new run for that claim: the window-WN\* records whose only open claim is "the window can move the result" for a corpus chart (WN1 W1 to W13, WN2, WN3, WN4a, WN4c 3, WN5 and WN6 where the record asks only for the best path string or total at 60 against 85), and the DL5 / DL5b rows that ask for "E at 85, not at 60" on a corpus chart. They do not cover what `dump` leaves out: `e_offset` (read it off the screen's "early fill N ms" line), legacy fills, other caps and depths, difficulty, Pro Drums or 2x Bass changes, the schedule of any chart except the eight, and every screen. Count: about 180 records (the window-WN\* "claim not checked" records, 178 by CL4, and DL5 rows 1 to 4).

## The runs

### R1. Engine record of every corpus chart (dump, schedule, score)

Kind: a record states what the engine finds on a real chart (best path, total, activation ticks, squeeze figures, fills, SP end, chord tables, solo, meter, end ratios).

Command (one loop; `--efill 60` is the default window; `--cap 4 --ms 10 --depth 2` are the GUI tests' settings the sweep also used):

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/runs/R1'; New-Item -ItemType Directory -Force $o | Out-Null; $root='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; Get-ChildItem -Recurse $root -File | Where-Object { $_.Name -match '\.(chart|mid)$' } | Sort-Object FullName | ForEach-Object { $n=$_.FullName.Substring($root.Length+1) -replace '[^\w.-]','_'; & $R dump --chart $_.FullName --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/$n.dump.json" 2>"$o/$n.dump.err"; & $R schedule --chart $_.FullName --path "$o/$n.dump.json" --index 0 --out "$o/$n.schedule.json" 2>"$o/$n.schedule.err"; & $R score --chart $_.FullName --path "$o/$n.dump.json" --index 0 --out "$o/$n.score.json" 2>"$o/$n.score.err" }
```

Checked against: `tools/replay.cpp` usage text (dump takes `--cap --efill --ms --depth --out`; schedule and score take `--path <dump> --index N --out`). A chart whose dump fails leaves a non-empty `.err` file; a chart with no activations still schedules (`flammarker_ingame` did).

Output: `CAP/R1/<chart path with / as _>.{dump,schedule,score}.json` and `.err`.

Serves: every record in CL1, CL2, CL3 and CL4 section 1 whose chart is in `testdata/input` and whose need is "dump, schedule or score at the default or GUI settings". By CL counts: about 120 (CL1 section 1), 85 (CL2 section 1), 190 (CL3 section 1, records `C2`, `C3`, `C5`, `C6`, `C7`, `C11` to `C14`, and the engine-reading records of `D1` to `D12` that say "compared with the tool reading it"), and 120 (CL4 section 1; its charts named in rows for Burnout, White Washed, Believer, Everlong, Feast of Fire, Overrated, Tapped Out, Tapestry, Allies, Sugar, HopH2O, Unbound, Get Ur Freak On, Focus, Epidermis, Pitch Dark, folded_sqin, relabel_case, clamped_sqin_a/b, spent_then_next, the fast-tempo charts, `testdata/preview/notes.mid` excluded, see R9). Total about 515. Burnout's "3- 1 2" 378315, Believer's paths, Beg's E0 and the like are in the outputs by chart name.

Settles each claim: the claim's typed value (path string, total, tick, ms figure, activation count, chord) is found or not in the named chart's `.dump.json` (`paths[*].pathstring`, `total`), `.schedule.json` (`activations[*]`: `act_tick`, `sp_ready_ms`, `deact_ms`, `squeezes`, `sp_end_steps`, `skipped_fill_ticks`; `chords`) or `.score.json` (`sections`, `measure`, `warnings`, solo and dynamics fields). A typed value the engine does not give means the claim fails. A value no dump or schedule field carries (for example `e_offset`, `bank_rise_ticks`, `anchor_room_ms`) is not settled here: it goes to R4 (screen text) or is marked `carry out` by the re-judge agent.

### R2. Engine record under other rules and settings (variants)

Kind: a record states what the engine finds under a rule or setting other than the default: Clone Hero 1.0 fills, Hard, Pro Drums off, 2x Bass off, an early-fill window of 0 or 85, a different cap, legacy fills with ms 0 and tie limit 1.

Command:

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/runs/R2'; New-Item -ItemType Directory -Force $o | Out-Null; $root='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; $v=@{ legacy=@('--legacy-fills'); hard=@('--difficulty','hard'); noprodrums=@('--prodrums','0'); nobass2x=@('--bass2x','0'); efill0=@('--efill','0'); efill85=@('--efill','85'); cap2=@('--cap','2'); cap32=@('--cap','32'); legacyms0=@('--legacy-fills','--ms','0') }; Get-ChildItem -Recurse $root -File | Where-Object { $_.Name -match '\.(chart|mid)$' } | Sort-Object FullName | ForEach-Object { $n=$_.FullName.Substring($root.Length+1) -replace '[^\w.-]','_'; foreach ($k in $v.Keys) { & $R dump --chart $_.FullName --cap 4 --ms 10 --depth 2 --efill 60 @($v[$k]) --out "$o/$n.$k.dump.json" 2>"$o/$n.$k.err" } }
```

Checked against: `tools/replay.cpp`. `settings_from` applies `--difficulty`, `--prodrums`, `--bass2x` for every command, but the `dump` line of the usage text does not list them; if a dump refuses one, the `.err` file says so and R2 falls back to `score` for those three variants. A later flag overrides an earlier one (`--cap 2` after `--cap 4`): the parser keeps the last value (`tools/replay.cpp` lines 835 to 856 assign each flag as it reads it). That is read, not run: if the main session finds otherwise, change `--cap 4` to be omitted for the cap2 and cap32 variants.

Output: `CAP/R2/<chart>.<variant>.dump.json`. 116 charts times 9 variants is 1,044 dumps; the 116-chart sweep did 464 in one run, so this is about two such sweeps.

Serves: CL1 rows "legacy fills", "Hard", "Pro Drums on and off", "2x Bass", "Sugar", "HopH2O", "A chart with CH 1.0 fills" (A13 6, 17 to 23; A14 24; A20 14; A24 4; A26 RU15 to RU20; A39 lines 74 and 114; B1 29 to 31); CL2 rows Acid Romance and Pathfinder `count_dynamics`; CL4 rows WN1 W3 to W5 (Sugar, HopH2O, Unbound), WN4 6 and 32, WN4a 6 and 9, DL4 test_model 3 to 6 (windows 0, 60 and 85), DL5 2 and 3. About 110 records.

Settles each claim: the variant's `.dump.json` holds or lacks the typed path or total; an `.err` file means the flag was refused. A record that asks for a tie limit of 1 or a Path limit: `dump` has no tie-limit flag (not in the usage text), so those records (WN1 W3 to W5, A26 RU15 to RU20, 9 records) are unplaced in the counts below.

### R3. Targeted search and pinned windows

Kind: a record states that a path with named SP ends exists, is kept, or fails (`target`).

Command, for the charts the records name, from R1's dumps:

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/runs/R3'; New-Item -ItemType Directory -Force $o | Out-Null; $in='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\common'; $r1='docs/audit/2026-11-14-test-audit/capture/runs/R1'; $bel="$in\IB24\T1\Smash Mouth - I_m A Believer (The Monkees cover) [highfine]\notes.chart"; $all="$in\Summer Blast _25 Setlist\Tier 3\Mutemath - Allies\notes.chart"; $bk=($bel.Substring('C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\'.Length) -replace '[^\w.-]','_'); $ak=($all.Substring('C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\'.Length) -replace '[^\w.-]','_'); foreach ($i in 0,1,2,3,4) { & $R target --chart $bel --path "$r1/$bk.dump.json" --index $i --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/believer.path$i.target.json" 2>"$o/believer.path$i.err" }; & $R target --chart $all --path "$r1/$ak.dump.json" --index 0 --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/allies.path0.target.json" 2>"$o/allies.path0.err"; & $R target --chart $all --ticks "22272,49920" --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/allies.ticks.target.json" 2>"$o/allies.ticks.err"
```

Checked against: the `target` lines of the usage text (`--path <dump> --index N`, `--ticks "t1,t2"`; give one of the three). The usage text warns that `--ticks` "can run for a very long time" on a long chart. Cap this step at 10 minutes; if Allies' `--ticks` run passes it, stop it and report; the `--path` forms are fast.

Output: `CAP/R3/*.target.json` (fields `realized`, `failed_tick`, `realized_prefix`, `failed_reason`, `pins`, `paths`).

Serves: CL1 row "Charts whose targeted search names activation ticks" (A27 ST1 to ST4, ST6 to ST8, ST10 to ST12; A8 14 to 17), CL4 WN1 W12 and W13, WN2 12, WN6 6, WN4b 25 to 27, 30, 31. About 30 records. Records whose ticks are typed in the record and are not 22272 or 49920 (ST records on fuzz seeds 5 and 199, about 8) need their ticks read from the record by the re-judge agent: unplaced.

Settles each claim: `realized` true or false, `failed_reason`, and `pins` against the typed window ticks.

### R4. Screens: one `hydra_uitest --script` per screen

Kind: a record states the text, colour, count or layout a screen shows on a real chart. One capture per screen, as the user chose. The script files do not exist yet except the eight in `records-screen/`; a script is written by the main session from the verbs below, each of which `docs/agents/ui-testing.md` lists. `--ini` and `--db` start each test from a copy; neither file is opened or changed.

R4a is the retake of the eight `records-screen/` captures with the fixed `moments.py`. It is the only part that needs the existing scripts:

```
$rs='docs/audit/2026-11-14-test-audit/records-screen'; $U='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_uitest.exe'; $o='docs/audit/2026-11-14-test-audit/capture/runs/R4a'; New-Item -ItemType Directory -Force $o | Out-Null; $abs=(Resolve-Path $o).Path -replace '\\','/'; $m=@(@('beg','beg.schedule.json',''), @('believer_path4','believer.path4.schedule.json','--ini corpus_depth4.ini'), @('believer_path0_moments','believer.schedule.json',''), @('pitch_dark_moments','pitch_dark_ratio.schedule.json',''), @('folded_sqin','folded_sqin.schedule.json','--ini fixtures.ini'), @('late_sqin_twice','late_sqin_twice.schedule.json','--ini fixtures.ini'), @('path_reduction_test0','path_reduction_test0.schedule.json','--ini fixtures.ini'), @('flammarker_ingame','flammarker_ingame.schedule.json','--ini fixtures.ini')); foreach ($c in $m) { $head=Get-Content "$rs/$($c[0]).cmd.txt"; $k=($head | Select-String '^type \*\*/##scrub' | Select-Object -First 1).LineNumber-1; $name=$c[0] -replace '_moments$',''; $lines=@($head[0..($k-1)]) + @(py -I "$rs/moments.py" "docs/audit/2026-11-14-test-audit/records-engine/$($c[1])" $c[0] $abs); Set-Content "$o/$($c[0]).cmd.txt" $lines; $a=@('--script',"$o/$($c[0]).cmd.txt"); if ($c[2]) { $a += '--ini'; $a += "$rs/" + ($c[2] -split ' ')[1] }; & $U @a > "$o/$($c[0]).txt" }
```

Checked against: `records-screen/index.md` ("How it was captured"), `moments.py` (its usage line and its print lines), `tests/ui/uitest_main.cpp` (`--script` and `--ini` are options; a script runs alone). The head of each command file is kept up to its first `type **/##scrub` line; the tail is replaced by the fixed `moments.py` output, written into `CAP/R4a` instead of over `records-screen/`. Unchecked: the eight files' heads were not all read to the end; if a file has lines after its last screenshot, they are lost. The two `.ini` files name absolute `chartfolder=` lines for this machine (index.md says so); if `fixtures.ini` points elsewhere, fix that line first. `pitch_dark.cmd.txt` and `believer.cmd.txt` do not use `moments.py` (they use "Act >" jumps), so they are not retaken.

Serves: the records that read the Paths tab, Stars tab and Preview highway on those eight charts: CL1 section 2 rows "Paths tab" (A2, A3, A18; about 55), "Preview score box" (A23 2), "Preview highway" rows for Pitch Dark and Beg (A15, A20; about 25); CL2 "Preview gauge and drain box" for Pitch Dark and Believer (A29, about 18) and "Paths tab" Believer path 4 (A30 19, 27); CL4 Paths-tab and Preview rows for Beg, Believer, Pitch Dark (WN5 7 to 17, DL5 4, about 25). About 125 records.

Settles each claim: the picture is read with the Read tool and the text file with Read: the claim's number, colour or sentence is present at the named moment. The `believer_path4_a2_end_minus_1ms` and `_end` pair that showed the same score (index.md) is re-read at the corrected moments.

R4b to R4k are the other screens, one script each, saved as `CAP/R4<letter>/<screen>.cmd.txt` with its stdout beside it. Common head for every script: `timeout 300`, `click Scan library`, `wait-text Continue`, `click **/Continue`, `wait-idle` (the head of `beg.cmd.txt`). Command for each: `build-cpp\Release\hydra_uitest.exe --script <file> [--ini <file>] > <file>.txt` from `REPO`. A script that needs a settings change, a batch or a window other than `//Hydra` uses the label rows of `docs/agents/ui-testing.md`. Screenshots use `screenshot <png> | <ref>` as in the retake.

| Run | Screen | Chart and path | What to do on it (verbs) | Records (by CL counts) |
|---|---|---|---|---|
| R4b | Library view | the corpus library as the harness builds it (98 charts in `testdata/input`) | `type **/##search | beyonce`, then the other typed queries the records name (`artist:`, `stars:N`, `squeeze<=N`, a quoted phrase, a bad operator, an empty box); `text`; `state`; chips `**/Stale`, `**/Analyzed` after a batch | CL1 A17 1 to 28, A6, A8 (about 70); CL2 A41, A42 (about 30); CL3 Library rows (about 250) |
| R4c | Analyze library confirm, batch strip, finished strip | the corpus | `click Analyze library...`, `text`, set settings through `//$FOCUSED/**/<label>` as the table in ui-testing.md says, `click //Analyze library/Start analyzing`, `text` while running, `wait-idle`, `text` | CL1 A8, A16; CL2 A37 batch-reports; CL4 DL3, DL5 5, DL5 21 to 24 (about 70) |
| R4d | Settings button and Analysis settings panel | none | `click **/###analysissettings`, `text`, `dump //$FOCUSED | earlyfillwindow`; change the window to 0, 85, then `defaults` | CL4 DL4 test_config 8, DL5 6, 21 to 23, 24 (about 8) |
| R4e | Dynamics tab | Acid Romance (2x Bass on and off), Pathfinder at Hard | `open Acid Romance`, `click **/Dynamics`, `text`; set 2x Bass off; again | CL2 A39, B1 29 to 40; CL1 A24 19, 23, A18 25 (about 25) |
| R4f | Path report window | the corpus, filters on | `click Open path report`, `wait-idle`, `window //###pathreport`, `text`, `click **/##timing`; header sort | CL1 A1, A7, A25, A26 RV (about 70); CL2 A46, A47 (about 22); CL4 DL3 11, 32 to 40 (about 10) |
| R4g | Comparison window | the canned player `alice` | `click Compare with dmleaderboards...`, pick the row `**/###111`, `window //###dmreport`, `text` | CL1 A10 (about 35); CL4 A26 RV11 to RV13 (about 3) |
| R4h | Path Builder window | Burnout, White Washed, Set the World on Fire | Path Builder has its own runner: `build-cpp\Release\hydra_pathbuilder_uitest.exe --script <file>` (`tests/pb_ui/pbtest_main.cpp` usage line). Its open-a-song verbs are unchecked here: read `tests/pb_ui/pbtest_main.cpp` and an existing `tests/pb_ui` test for the opening lines | CL1 A19 PB1 to PB10; CL2 A48, A49; CL3 C7 (about 50); CL4 WN6 (about 14) |
| R4i | Startup screen and status line | `--db testdata/store/v2.12.1_library.db` (see R6) | `state`, `text` immediately after launch | CL4 DL2 (see R6) |
| R4j | Fill report page, report windows placement | needs a browser or a real second monitor | not a `hydra_uitest` capture; marked no possible run with these tools | CL1 A7 24 to 47, A14 1 to 22 (about 70) |
| R4k | Preview scene extras (loading bar mid-load, volume, no audio device) | a corpus chart with stems; none ship in `testdata/input` | marked unavailable (see "Unavailable inputs") | CL1 A21, A22 (about 50) |

R4b to R4h are the screens that can be captured on `testdata/input` alone. R4i is R6's. R4j and R4k are listed so no record drops out: no run is named for them.

### R5. A real store made by `hydra_batch` over `testdata/input`, and the settings it reads

Kind: a record states what the store, the settings file or the rules file holds or does on real data (keys, columns, profile, record stamps, "no chart library" sentence, hydra_compare tests, batch count lines).

Command (the library is left alone because folder arguments are given; `--db` names a new file under CAP so the user's `hydra.db` is not touched):

```
$o='docs/audit/2026-11-14-test-audit/capture/runs/R5'; New-Item -ItemType Directory -Force $o | Out-Null; $B='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_batch.exe'; $in='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; & $B --db "$o/hydra.db" $in *> "$o/batch.txt"; & $B --legacy-fills --db "$o/hydra_legacy.db" $in *> "$o/batch_legacy.txt"; Copy-Item 'C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_settings.ini' "$o/hydra_settings.ini.used"
```

Checked against: the head comment and `batch_main` in `src/cli/batch.cpp` (`<folder>` arguments leave the library alone; `--db <path>` picks the file; `--legacy-fills`; without folders or `--whole-library` it refuses). It reads `hydra_settings.ini` beside the exe (`load_tool_settings`), which is the development build's file under `build-cpp\Release`, not the installed app's. The command copies that file next to the output so the settings used are on record. Do not commit the `.db` files (binary); commit `batch.txt`, `batch_legacy.txt` and the settings copy. The `.db` files are read by the re-judge agent with `python -I` and `sqlite3`.

Serves: CL1 section 3 rows "Real hydra_batch database", "Corpus scan rows", "A real hydra_settings.ini from the machine" (A9 1 to 40, A5, A10 27, A14 21, A25 R27, A4-q3 about 15; about 70 records); CL2 section 3 rows "A real new hydra.db written by this build", "A real broken hydra_rules.ini and hydra_settings.ini" (the machine's own file only; broken ones are not on disk), A33 and A34 (about 20); CL3 section 3 rows "Real hydra.db / stored records", "real hydra_settings.ini", "two real hydra_batch databases over testdata/input" (D14 `test_compare_db`, about 20; C8, C9, about 40); CL4 DL1, DL4 test_config, DL5 28, DL5b 9 and 10 (the two stores for `test_compare_db.py`: run R5 twice, with one chart changed, is not possible, see below). About 150 records.

Settles each claim: the store's tables, `user_version`, profile columns and stamps (read with `sqlite3`) against the claim; the console lines against the "Analyzed N, skipped M, F failed" text the records quote; the settings copy against the keys the records say load and save. The `test_compare_db.py` claim of a changed result needs a second store with one result changed; no run names how to change one without editing the user's data, so DL5b 9 and 10 are unplaced.

### R6. Real older library files, opened by Hydra

Kind: a record states what Hydra does when it opens a real library file from an older release.

Command (once per file; `--db` is copied, never opened):

```
$U='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_uitest.exe'; $o='docs/audit/2026-11-14-test-audit/capture/runs/R6'; New-Item -ItemType Directory -Force $o | Out-Null; Set-Content "$o/open.cmd.txt" @('timeout 120','wait-idle','state','text'); foreach ($f in 'v1.8.4-schema2','v1.8.4-schema2-legacy-fills','v2.0.0-schema3','v2.1.0-schema4','v2.12.1_library') { & $U --script "$o/open.cmd.txt" --db "C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\store\$f.db" > "$o/$f.txt" }
```

Checked against: `docs/agents/ui-testing.md` (`--db <file>` starts from a copy; a missing file is an error; `state` prints `store=opening|ready|failed`; startup screen words). `testdata/store/README.md` says the first four files are too old to convert, so the expected output for them is the refusal sentence; the fifth opens.

Serves: CL1 section 3 row "A real stored library file saved by an older build" (A16 18), "Real hydra.db (current and older, v2.1.0 schema 4)" (A9 16, 17, 23; A5 4, 5, 24; A27 SI14); CL2 A32 1 to 7, 9 to 13 (the files that exist: v2.1.0-schema4, v2.12.1), A42 5, 7; CL4 DL2 1, 8, 9 (the v2.12.1 file). About 30 records.

Settles each claim: the first screen text and `state` line against the claimed words and result.

### R7. Hook tests: recorded real calls judged by the hooks

Kind: a hook test states how a hook answers a call; the real call is one a past session made.

Command (the extract holds secrets and must stay in the gitignored `gates\state\replay`; `replay_extract.py` itself refuses a path that is not ignored):

```
python -I C:\Users\Patrick\.claude\gates\tests\replay_extract.py --out C:\Users\Patrick\.claude\gates\state\replay\calls.jsonl --days 14; python -I C:\Users\Patrick\.claude\gates\tests\replay.py C:\Users\Patrick\.claude\gates --calls C:\Users\Patrick\.claude\gates\state\replay\calls.jsonl --out C:\Users\Patrick\.claude\gates\state\replay\report.txt; Copy-Item C:\Users\Patrick\.claude\gates\state\replay\report.txt docs/audit/2026-11-14-test-audit/capture/runs/R7/report.txt
```

Create `CAP/R7` first. Checked against: the two files' docstrings and `--help` (`replay_extract.py --out --days --limit --workers`; `replay.py <gates folder> --calls --out --sample-shell`). The extract reads session transcripts under `~/.claude/projects`, not `state\agents`; the handoff's wording "from the real state under `state\agents\`" does not match the code, which reads `*.jsonl` transcripts. `state\agents` holds one `.json` and one `.handback` file per agent (48 folders). `state\replay` does not exist yet. The report holds counts, rule names and redacted shapes only (its docstring), which is why it is safe to copy into the repo. Do not copy `calls.jsonl`.

Serves: `percall.body` (progress, delete rule, library flag), `review_dispatch.body`, `publish_gate.check`, `bash_windows_rule`. By CL4 section 4: E1 test_percall_key (6) and the shared note for test_docs_numbers, test_bash_windows_rule, test_delete_rule and test_media_rule (about 60 records by the shared note), E2 test_percall_rewrite (10), part of test_progress (the call-budget and refusal cases; about 20 of 48), test_publish (33), E3 test_review_dispatch (32). About 160 records.

Settles each claim: the hook's answer on real recorded calls (counts of allowed and refused by rule in `report.txt`) is compared with what the test says the hook does. A test that says "the hook refuses a delete outside the scratch folder" holds if the report shows refusals of that class on real deletes and no crash.

### R8. Hook tests: the real ref moves

Kind: a hook test states how the ref gate judges real branch moves.

Command:

```
$env:GATES_REFLOG_REPLAY='1'; python -I -m unittest tests.test_review_rules.ReflogReplay -v 2>&1 | Tee-Object docs/audit/2026-11-14-test-audit/capture/runs/R8/reflog_replay.txt
```

Run it from `C:\Users\Patrick\.claude\gates` (so `tests.test_review_rules` imports), then copy the output file in. Create `CAP/R8` first. Checked against: `tests/test_review_rules.py` line 980: the class `ReflogReplay` runs only when `GATES_REFLOG_REPLAY=1`, reads `tests/reflog_moves.json` (294 moves) and the real repos, read only. This is a hook test: an agent may not run it; the main session does.

Serves: CL4 section 4 E3 test_refusals (4), test_ref_judge (19, the reflog-move cases), test_review_rules (110, the `explain_main_move` cases). About 130 records; the transcript cases of test_ref_judge (the typed prompt, tool-result answer, sub-agent prompt; CL4 says this session's transcript has none with `"promptSource":"typed"`) are not served.

Settles each claim: whether the replay's tally shows every historical main move allowed or refused as the test says.

### R9. Preview golden

Kind: a record states what Hydra's frame shows against a saved Onyx screenshot of the same chart.

Command: `build-cpp\Release\hydra_uitest.exe --script <file>` with a script that sets `--ini` to a file whose `chartfolder=` is `C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\preview`, then `open <title>` (the title is unchecked: read `testdata/preview/` for its `song.ini` or notes title), `click **/Preview`, `type **/##scrub | <seconds>`, `screenshot CAP/R9/hydra.png | **/##highway_image`; compare by eye with the saved Onyx screenshot under `testdata/preview/` (unchecked: file name not read). Serves CL1 A21 29, 30; CL4 WN4c 6. About 3 records. The compare against the Onyx picture is by reading both pictures.

## Counts

Records served per run, by the capture lists' counts (a record can sit in two runs; the sum is above the real count of records):

| Run | Records served |
|---|---|
| Already done (records-engine, window sweep) | about 180 |
| R1 corpus engine record | about 515 |
| R2 variants | about 110 |
| R3 targeted search | about 30 |
| R4a retake of eight screens | about 125 |
| R4b to R4h other screens | about 520 (R4b 350, R4c 70, R4d 8, R4e 25, R4f 100, R4g 38, R4h 64) |
| R5 store and settings | about 150 |
| R6 old library files | about 30 |
| R7 hook calls | about 160 |
| R8 ref moves | about 130 |
| R9 preview golden | about 3 |

Records with no possible run with these tools or with this machine's files: about 900. They are:

- Clone Hero game runs: CL3 D1 to D13 sections "Real ch_start / ch_stop / ch_run_chart run", "Real ch_play_chart run", "Recorded real Frida session replies", "machine_recorder recording", "ch_open_song / ch_kit replies", "Saved real reads of live Clone Hero memory", "live run where the game came to the front", "Real sp_compare watch run", "Real engine memory block" and "UNSORTED" (CL3 sections 4 and the last table): about 700 records. They need a live Clone Hero, which is the user's game, and saved runs already exist outside the repo under `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\` (nine dated folders, 2026-10-28 to 2026-11-13, such as `2026-10-31\ls-d1a-run3`, `2026-11-06\ls-o1-run1`, `2026-11-06\ls-d2b-run1`) and `testdata/ch_watch/ls-o2-run1-sp_watch-1.csv`. Those saved runs are real input the re-judge agent can read. They are not a run to take, and no run reads them yet. Whether a saved run answers a given record is for the re-judge agent to say. Anything that needs a new live run, or `allow key run`, goes to the user.
- Hook tests with no replayer in the gates folder: `agent_stop`, `common`, `posture`, `PromptHook`, `stall`, `shell_output`, `ruling_rules`, `session_cards_start`, `cleanup_sweep` (CL4 E1, E2, E4: about 330 records, of which the 7 + 1 + 18 + 35 + 50 + 48 + 44 + 42 = 245 have real recorded state in `state\agent_stop.jsonl`, `state\stall`, `state\posture`, `state\refusals.jsonl`, `state\rulings.jsonl`, `state\reviews.jsonl`, `state\agents\*.json` and `.handback`, but nothing feeds it to the hooks). No existing tool does it. Writing one is a new tool, which is not this scout's to propose; it goes to the user.
- Screens needing a browser, a second monitor, a real GPU, the installed app, DPI changes or a real network (R4j, R4k and the version-bar rows): about 120 (CL1 A7 24 to 47, A14 1 to 22 pages, A17 11, 29, A25 P2, P9, P10; CL3 version bar and taskbar rows).
- Records whose claim only a hand-built chart can test, or that name a golden digest (DL5b 11 to 20): about 20.

Records covered by the dumps already done: about 180, as above.

Records I could not place: about 150. They are the records whose capture the lists give only as "see the record itself": CL3's "UNSORTED" row for D2 to D12 (about 280 of which 150 are Clone Hero game records already counted above under "no possible run"; the rest, about 130, are in D8 to D12 and may sit under R1 or R7), the CL2 rows that name a chart not in the corpus or that depend on typed ticks (about 30), R2's tie-limit records (9), R3's fuzz-seed records (8), R4h's unknown Path Builder opening lines, and DL5b 9 and 10. Treat 150 as the figure, to the nearest 25, until each is read.

## Unavailable inputs

| Input | Where I looked | Records that wait on it |
|---|---|---|
| A real library file from release 2.8.0 (with a charts row, without the failure columns) | `testdata/store/` (README lists v1.8.4, v1.8.4 legacy, v2.0.0, v2.1.0, v2.12.1 only); `C:\Users\Patrick\Downloads\Hydra\` (searched four levels for `hydra.db*` and `hydra_settings.ini*`: only the development build's files turned up) | CL4 DL2 2 and 4; CL2 A32 (2.8.0 row) |
| Real library files from 2.2.0 to 2.7.0 | same; the store README says none is checked in | CL4 DL2 5, 6; CL1 A9; CL2 A32 |
| A real library from a release before the album column | same | CL4 DL2 3 |
| A real 1.6 library file | same | CL2 A32 |
| A real `hydra_rules.ini` from older releases (`fill_length_measures`, 1.9.0 and 2.8.0 values) | same; `build-cpp\Release` holds no rules file | CL1 A26 RU2 to RU6, RU22, A5 21; CL3 C2 617, C5 949 |
| A real older `hydra_settings.ini` (`sp_cap=auto` from 1.8.4, `hit_window_ms=85` older file) | `build-cpp\Release\hydra_settings.ini.backup-2026-10-25` exists but is the development build's file, not a release's | CL1 A9, A5 21, 22, 23 |
| A saved real DMBot `/all-users` and `/user/<id>/scores` response; a saved real GitHub release reply | `testdata/update` exists (not read); the comparison tests use canned replies | CL1 A10 3, 11, A25 P6, A26 RV12; CL2 A40 H21, A47 5 |
| `.sng` containers, `.srb` bundles (`Biology.srb`) | `testdata/` and the Hydra folder (`*.srb`, `*.sng` search found none) | CL3 C3, C11, C12, C14 (about 25) |
| Charts named by records that are not in `testdata/input`: Weltseele, Slander, What's My Age Again?, Dumpweed, Freak, Rescued, Ghost, As I Am, Thrice - Deadbolt, SoundHaven - Triad, Hormone's Echo, Band Like That and Car Bomb (CL3 names `common/IB24/T2/fanclubwallet - Band Like That`; not checked whether it is present) | CL1 and CL2 lists (they say "not found") | CL1 A3 1, A13 9, A4 41; CL2 A30 6 to 8, A35 7, 8, B3 10, 11, B7 13 |
| Charts with real stems in `.ogg`/`.opus` per song (the corpus has only `testdata/audio/sine220.*`) | CL1, CL3 | CL1 A27 SA1, SA2, A22; CL2 A31 SR1 to SR15; CL3 section 3 "Real audio" |
| A real second `hydra_rules.ini`, a damaged one, a truncated copy of a real library | not on disk | CL4 DL1 178, 917, 1288, 1328; CL2 A32, A33 |
| `C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra.db` and `C:\Program Files\Hydra\hydra.db` (the development build's and the installed app's libraries) | `docs/agents/this-machine.md` line 75 | Any record whose claim is about the user's own library or row counts (CL1 A8 22 to 27, 30, 32, 35, 39, library of 12,345 charts; CL2 A42 3, 4, real `hydra_ui.ini` after a Best path sort): **needs the user's yes**. The Library view only, never an analysis (`docs/agents/ui-testing.md`, case 3). |

## What I stopped on

Nothing was stopped by a hook. Three things are unchecked and the main session should read them before running: the Path Builder script's opening lines (R4h, `tests/pb_ui/pbtest_main.cpp`), the last lines of the eight `records-screen` command files after their final screenshot (R4a), and whether `hydra_replay dump` accepts `--difficulty`, `--prodrums` and `--bass2x` (R2; the parser takes any flag for any command, so it should, but the usage text does not list them). The handoff says `replay_extract.py` reads "state under `state\agents`"; the code reads transcripts (R7). I wrote this near the 20-minute mark and did not read each record, only the lists, so the per-run counts are the lists' counts.
