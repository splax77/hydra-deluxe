Agent id: a56a9b58f1e870b83 (task T5-CLEAN-RUNS, session 63c70014-4328-49dc-9205-742235732752)

# The clean Investigate records, and the real run for each

Plain version. 397 Investigate records hold no fault after TOTALS.md's corrections. Their one open point is "claim not checked". This file names, for each, the run in `capture/RUNS.md` that would check it, or says why no run does. Nothing was run.

How the list was built. I read the 21 files `counts/T5-IR1.md` to `T5-IR21.md` and took every Investigate record whose fault cell is "none". Then I applied TOTALS.md section 3: records it cleared are added (IR3 687; IR11 A17 35 and 39; IR16 B2 PR:99; IR17 nine `test_game_files.py` hand-built-ini rows; IR20 lexer_language and A49 16). TOTALS.md section 6 lists IR1's compare_db rows as unsettled, so IR1 205, 230, 241, 275, 286, 299 are left out, and so are IR21 DL5b 9 and 10, which are the same two tests (`tools/test_compare_db.py:286` and `:299`). IR8's non-Investigate rows are left out (only A3 #20 is clean there). No record on this list is in TOTALS.md's unsettled items 3 and 4 (they hold faults).

Limits you should know. (1) I did not open each of the 397 records. I read the count files' reason for each row, the "Real input needed" and Q3 text at the top of the record files D14, D2, D5, D9, D12, B2, C11, A17 and the Q3 sentences of the others I sampled, and I placed the rest by their record file and test file. A per-record "claim not checked" can name a narrower input than the group line says. The `Need` column is the group's need, not a quote from each record. (2) TOTALS.md names 9 IR17 D3 rows cleared of X9 for hand-built ini text; `test_game_files.py` holds 10 hand-built-ini rows (326, 333, 338, 346, 353, 369, 374, 378, 396, 400). I took 326 to 378 and 400 and left 396, because IR17 also marks it "a split with song.cpp". (3) IR20 lists many clean records only by block, so its rows carry the record number and a test file or a name, not a line. (4) The mark `unplaced` means no run in RUNS.md fits and it is not one of the four marks the brief gives; I did not invent a run for these. Most are Python tool tests or `hydra_precheck` tests whose real input is a repo or a real handoff, which RUNS.md has no run for.

## 1. Counts

| Mark | Records |
|---|---|
| R1 | 40 |
| R2 | 3 |
| R3 | 3 |
| R4a | 22 |
| R4b | 9 |
| R4c | 6 |
| R4d | 1 |
| R4e | 5 |
| R4f | 23 |
| R4h | 17 |
| R5 | 13 |
| R6 | 3 |
| R8 | 10 |
| no run | 54 |
| unavailable input | 16 |
| needs the user's library | 1 |
| unplaced | 171 |
| covered (records-engine dumps already cover it) | 0 |
| total | 397 |

Covered is 0 because every record here asks about a screen, a store, a tool or a chart under settings that the eight `records-engine/` dumps and the window sweep do not carry. I did not mark any record `covered` without reading its dump; the window-sweep answer only helps WN window records, and the clean list holds none that ask only "can the window move the result".

Unavailable input, by input: real `dump.cs` of Clone Hero build 1.1.0.6142 (outside the repo): 6; real `.sng` containers: 4; older `hydra_rules.ini`: 4; real stems: 2.

## 2. The commands

Paths: `REPO` is `C:\Users\Patrick\Downloads\Hydra\hydra-test`; `CAP` is `docs/audit/2026-11-14-test-audit/capture`. Each block is PowerShell, taken from `capture/RUNS.md` and checked there against the tool's source or help text. I re-read none of the tool sources, so the checking is RUNS.md's. Outputs go under `capture/out/<run>/` (RUNS.md used `capture/runs/`; I changed the folder as the brief says, by replacing `capture/runs/R` with `capture/out/R` in each command). Everything reads `testdata/input`, `testdata/store`, `testdata/preview`, or recorded git moves. Nothing touches the user's library or database.

### R1 engine record of every corpus chart (dump, schedule, score) (40 records)

The whole corpus is run because the records name charts only by group (tempo maps, clamped_sqin, game-test charts, tail songs). If the main session wants fewer, filter the `Where-Object` on `.chart|.mid` names to `clamped_sqin|relabel_case|spent_then_next` for A35, to the game-test charts for C5, and leave the rest whole.

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/out/R1'; New-Item -ItemType Directory -Force $o | Out-Null; $root='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; Get-ChildItem -Recurse $root -File | Where-Object { $_.Name -match '\.(chart|mid)$' } | Sort-Object FullName | ForEach-Object { $n=$_.FullName.Substring($root.Length+1) -replace '[^\w.-]','_'; & $R dump --chart $_.FullName --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/$n.dump.json" 2>"$o/$n.dump.err"; & $R schedule --chart $_.FullName --path "$o/$n.dump.json" --index 0 --out "$o/$n.schedule.json" 2>"$o/$n.schedule.err"; & $R score --chart $_.FullName --path "$o/$n.dump.json" --index 0 --out "$o/$n.score.json" 2>"$o/$n.score.err" }
```

Output: `capture/out/R1/`. Serves: A35 5, A35 8, D13 125, A23 2, C5 63, B2 -, B2 -, C11 SL no length no notes, C11 CL clamped window still offers squeeze-out, C11 CL two ends clamped, C11 CL tied clamps, C11 CL plain bar ties ceiling, C11 CL folded variant Clamped to SqIn, C11 CL unclamped window, WN4b 7, B2 -, B7 3, B7 4, B7 5, B7 6, B7 7, B7 8, B7 9, B7 10, B7 12, B7 15, B7 16, B7 17, B7 18, B7 21, A28 1, A28 2, A28 3, A28 8, A28 9, A28 10, A28 16, A28 19, A28 20, A28 22.

### R2 variants (narrowed to the two windows the clean records need) (3 records)

RUNS.md's nine variants are cut to `efill0` and `efill85`. A26 RU20 and RU21 ask for searches with and without a rule: if they mean legacy fills, add `legacy=@('--legacy-fills')` to the hash by hand; the record decides.

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/out/R2'; New-Item -ItemType Directory -Force $o | Out-Null; $root='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; $v=@{ efill0=@('--efill','0'); efill85=@('--efill','85') }; Get-ChildItem -Recurse $root -File | Where-Object { $_.Name -match '\.(chart|mid)$' } | Sort-Object FullName | ForEach-Object { $n=$_.FullName.Substring($root.Length+1) -replace '[^\w.-]','_'; foreach ($k in $v.Keys) { & $R dump --chart $_.FullName --cap 4 --ms 10 --depth 2 --efill 60 @($v[$k]) --out "$o/$n.$k.dump.json" 2>"$o/$n.$k.err" } }
```

Output: `capture/out/R2/`. Serves: D9 D9.md:719, A26 RU20, A26 RU21.

### R3 targeted search (3 records)

Cap at 10 minutes; the Allies `--ticks` step can run very long (usage text). A27 ST10 and ST11 and WN1 W13 name their ticks in their records; RUNS.md's two charts (Believer, Allies) are the ones it checked. If a record names another chart, the re-judge agent adds its line.

```
$R='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_replay.exe'; $o='docs/audit/2026-11-14-test-audit/capture/out/R3'; New-Item -ItemType Directory -Force $o | Out-Null; $in='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\common'; $r1='docs/audit/2026-11-14-test-audit/capture/out/R1'; $bel="$in\IB24\T1\Smash Mouth - I_m A Believer (The Monkees cover) [highfine]\notes.chart"; $all="$in\Summer Blast _25 Setlist\Tier 3\Mutemath - Allies\notes.chart"; $bk=($bel.Substring('C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\'.Length) -replace '[^\w.-]','_'); $ak=($all.Substring('C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input\'.Length) -replace '[^\w.-]','_'); foreach ($i in 0,1,2,3,4) { & $R target --chart $bel --path "$r1/$bk.dump.json" --index $i --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/believer.path$i.target.json" 2>"$o/believer.path$i.err" }; & $R target --chart $all --path "$r1/$ak.dump.json" --index 0 --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/allies.path0.target.json" 2>"$o/allies.path0.err"; & $R target --chart $all --ticks "22272,49920" --cap 4 --ms 10 --depth 2 --efill 60 --out "$o/allies.ticks.target.json" 2>"$o/allies.ticks.err"
```

Output: `capture/out/R3/`. Serves: WN1 W13, A27 ST10, A27 ST11.

### R4a retake of the eight screen captures (Paths tab, Preview) (22 records)

Needs `build-cpp\Release\hydra_uitest.exe` and the eight command files in `records-screen/`. RUNS.md flags it unchecked that the last lines of those files after the final screenshot are dropped.

```
$rs='docs/audit/2026-11-14-test-audit/records-screen'; $U='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_uitest.exe'; $o='docs/audit/2026-11-14-test-audit/capture/out/R4a'; New-Item -ItemType Directory -Force $o | Out-Null; $abs=(Resolve-Path $o).Path -replace '\\','/'; $m=@(@('beg','beg.schedule.json',''), @('believer_path4','believer.path4.schedule.json','--ini corpus_depth4.ini'), @('believer_path0_moments','believer.schedule.json',''), @('pitch_dark_moments','pitch_dark_ratio.schedule.json',''), @('folded_sqin','folded_sqin.schedule.json','--ini fixtures.ini'), @('late_sqin_twice','late_sqin_twice.schedule.json','--ini fixtures.ini'), @('path_reduction_test0','path_reduction_test0.schedule.json','--ini fixtures.ini'), @('flammarker_ingame','flammarker_ingame.schedule.json','--ini fixtures.ini')); foreach ($c in $m) { $head=Get-Content "$rs/$($c[0]).cmd.txt"; $k=($head | Select-String '^type \*\*/##scrub' | Select-Object -First 1).LineNumber-1; $name=$c[0] -replace '_moments$',''; $lines=@($head[0..($k-1)]) + @(py -I "$rs/moments.py" "docs/audit/2026-11-14-test-audit/records-engine/$($c[1])" $c[0] $abs); Set-Content "$o/$($c[0]).cmd.txt" $lines; $a=@('--script',"$o/$($c[0]).cmd.txt"); if ($c[2]) { $a += '--ini'; $a += "$rs/" + ($c[2] -split ' ')[1] }; & $U @a > "$o/$($c[0]).txt" }
```

Output: `capture/out/R4a/`. Serves: A44 7, A20 2, A20 3, A20 7, A20 10, A20 14, A20 15, A20 17, A20 18, A20 20, A18 2, A18 31, A18 33, A18 37, A18 54, A29 13 no SP no path, WN5 2, A45 2, A45 3, A45 7, A45 15, A45 16.

### R4b, R4c, R4d, R4e, R4f, R4h screens

RUNS.md gives no ready command for these. Each is one `hydra_uitest.exe --script <file>` run (Path Builder: `hydra_pathbuilder_uitest.exe --script <file>`), with the common head `timeout 300`, `click Scan library`, `wait-text Continue`, `click **/Continue`, `wait-idle`, and then the verbs in the RUNS.md R4 table. The script files do not exist and a main session writes them from `docs/agents/ui-testing.md`. What each serves from this list:

| Run | Screen | Records |
|---|---|---|
| R4b | Library view (search, theme colours) | A17 16, A17 17, A34 T3 apply_theme shades, A34 T7 hover teal, A41 1, A41 6, A41 13, A41 15, A41 18 |
| R4c | Analyze library confirm, batch and finished strips | DL3 1, DL3 5, DL3 6, DL3 7, DL3 8, DL3 9 |
| R4d | Analysis settings panel | DL4 6 |
| R4e | Dynamics tab (Acid Romance 2x Bass, Pathfinder Hard) | B1 22, B1 27, B1 36, B1 37, B1 38 |
| R4f | Path report window | A7 #1, A7 #2, A7 #3, A7 #4, A7 #5, A7 #6, A7 #7, A7 #8, A7 #9, A7 #10, A7 #11, A7 #43, A7 #47, A47 2, A47 6, A47 7, A25 P1, A25 R25, A25 R29, A25 R30, DL3 32, DL3 38, DL3 39 |
| R4h | Path Builder window (own runner) | A19 PL13, WN7 4, A49 5, A49 26, A38 1, A38 2, A38 3, A38 4, A38 5, A38 6, A38 7, A38 8, A38 9, A38 10, A38 11, A38 12, A38 13 |

Output: `capture/out/R4b/` to `capture/out/R4h/`, each script beside its stdout.

### R5 real store from hydra_batch over testdata/input (13 records)

Do not commit the `.db` files. Settings used are copied beside the output.

```
$o='docs/audit/2026-11-14-test-audit/capture/out/R5'; New-Item -ItemType Directory -Force $o | Out-Null; $B='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_batch.exe'; $in='C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input'; & $B --db "$o/hydra.db" $in *> "$o/batch.txt"; & $B --legacy-fills --db "$o/hydra_legacy.db" $in *> "$o/batch_legacy.txt"; Copy-Item 'C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_settings.ini' "$o/hydra_settings.ini.used"
```

Output: `capture/out/R5/`. Serves: A9 29 whole-number key, A27 SF9, A34 R8 Profile compares, A33 T2 in-memory reads itself, B1 7, WN4b 31, DL1 171, DL1 178, DL1 456, DL1 536, DL1 Profile, A17 35, A17 39.

### R6 older real library files (narrowed to the two files that exist for the clean records) (3 records)

The records need only `v2.1.0-schema4` (A6 28, A32 3) and `v2.12.1_library` (DL2 1). RUNS.md says the older four are too old to convert and answer with the refusal sentence; the clean records do not need them.

```
$U='C:\Users\Patrick\Downloads\Hydra\hydra-test\build-cpp\Release\hydra_uitest.exe'; $o='docs/audit/2026-11-14-test-audit/capture/out/R6'; New-Item -ItemType Directory -Force $o | Out-Null; Set-Content "$o/open.cmd.txt" @('timeout 120','wait-idle','state','text'); foreach ($f in 'v2.1.0-schema4','v2.12.1_library') { & $U --script "$o/open.cmd.txt" --db "C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\store\$f.db" > "$o/$f.txt" }
```

Output: `capture/out/R6/`. Serves: DL2 1, A6 28, A32 3.

### R8 the real ref moves (10 records)

Run from `C:\Users\Patrick\.claude\gates`. Create `capture/out/R8` first. A hook test: the main session runs it, not an agent. The E3 RJ and RR records are real-git judge cases; RUNS.md serves E3 with this run for the `explain_main_move` cases, and I read no further than that to place the RJ and RR rows here.

```
$env:GATES_REFLOG_REPLAY='1'; python -I -m unittest tests.test_review_rules.ReflogReplay -v 2>&1 | Tee-Object docs/audit/2026-11-14-test-audit/capture/out/R8/reflog_replay.txt
```

Output: `capture/out/R8/`. Serves: E3 RJ:147, E3 RJ:153, E3 RJ:221, E3 RJ:232, E3 RJ:285, E3 RR:252, E3 RR:313, E3 RR:324, E3 RR:328, E3 RR:341.

Not served by any command: R7 (hook calls on recorded real transcripts) serves no row of this list, so it is not written out. R9 serves none either.

## 3. The list

Columns: record file, record, test file:line (or name), what the claim needs, run or mark.

| Record file | Record | Test | Need | Run or mark |
|---|---|---|---|---|
| D14 | test_existing_claims_listed | tools/test_check_handoff.py:94 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_missing_claims_fail | tools/test_check_handoff.py:105 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_changed_since_date | tools/test_check_handoff.py:114 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_unchanged_is_not_listed | tools/test_check_handoff.py:125 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_prose_is_ignored | tools/test_check_handoff.py:132 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_other_repo_root_on_line | tools/test_check_handoff.py:138 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_date_sources | tools/test_check_handoff.py:150 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_qualified_function_found_where_written | tools/test_check_handoff.py:183 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_path_outside_the_repo_or_in_the_wrong_case_is_missing | tools/test_check_handoff.py: | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_qualified_method_needs_its_class_in_the_file | tools/test_check_handoff.py:210 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_session_id_is_not_a_commit | tools/test_check_handoff.py:221 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_claims_table_statuses | tools/test_check_handoff.py:234 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_claims_table_only_rows | tools/test_check_handoff.py:260 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_claims_exit_code | tools/test_check_handoff.py:268 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_claims_rows_that_must_not_pass | tools/test_check_handoff.py:278 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_claims_table_with_no_readable_row_fails | tools/test_check_handoff.py:297 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_row_without_outer_pipes_is_read | tools/test_check_handoff.py:303 | a real handoff from docs/handoffs/ run through check_handoff.py, each EXISTS or MISSING line beside git itself | unplaced |
| D14 | test_missing_path_reported | tools/test_check_pointers.py:76 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_hook_names | tools/test_check_pointers.py:83 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_absolute_path | tools/test_check_pointers.py:91 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_sessions_line_skipped | tools/test_check_pointers.py:99 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_exit_code | tools/test_check_pointers.py:107 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_wrong_case_is_broken | tools/test_check_pointers.py:139 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_tail_match_is_never_found | tools/test_check_pointers.py:145 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_docs_read_at_any_depth_and_archive_left_out | tools/test_check_pointers.py:156 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_fenced_block_and_sibling_and_other_repo | tools/test_check_pointers.py:165 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_not_file_names_are_ignored | tools/test_check_pointers.py:173 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_shorthand_names_are_ignored | tools/test_check_pointers.py:181 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_memory_note_shown_by_short_name | tools/test_check_pointers.py:191 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_git_show_fallback_that_opens | tools/test_check_pointers.py:210 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_git_show_fallback_that_does_not_open | tools/test_check_pointers.py:224 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_git_show_fallback_in_hooks_repo | tools/test_check_pointers.py:235 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_quoted_pointer_skipped_only_in_rulings | tools/test_check_pointers.py:244 | the real docs run through check_pointers.py, its broken-pointer list beside git | unplaced |
| D14 | test_a_hand_fix_of_one_of_three_faults... | tools/test_effort_test_faults.py:99 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_no_commits_and_an_empty_report... | tools/test_effort_test_faults.py:110 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_every_fault_caught_passes_the_run | tools/test_effort_test_faults.py:113 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_named_by_file_and_key_line | tools/test_effort_test_faults.py:119 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_named_by_the_line_the_fault_has_at_the_tip | tools/test_effort_test_faults.py:124 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_named_by_the_words_of_the_fault | tools/test_effort_test_faults.py:128 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_named_by_the_planted_text | tools/test_effort_test_faults.py:132 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_a_line_with_no_file_does_not_name_a_fault | tools/test_effort_test_faults.py:141 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_a_paragraph_about_one_fault... | tools/test_effort_test_faults.py:144 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_a_longer_file_name_is_not_the_faults_file | tools/test_effort_test_faults.py:149 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_a_number_inside_a_longer_number... | tools/test_effort_test_faults.py:152 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_fixed_wins_over_named | tools/test_effort_test_faults.py:156 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_fixing_the_planted_copy_counts... | tools/test_effort_test_faults.py:160 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_removing_the_faults_file_counts_as_fixed | tools/test_effort_test_faults.py:169 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_an_edit_away_from_planted_lines... | tools/test_effort_test_faults.py:175 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_neighbouring_changed_lines_are_one_false_finding | tools/test_effort_test_faults.py:181 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_an_edit_on_a_reference_line... | tools/test_effort_test_faults.py:186 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_an_edit_in_a_file_with_no_fault... | tools/test_effort_test_faults.py:190 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_removing_a_whole_multi_line_fault... | tools/test_effort_test_faults.py:194 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_a_line_added_beside_a_planted_line... | tools/test_effort_test_faults.py:199 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_removed_lines_that_look_like_diff_headers... | tools/test_effort_test_faults.py:204 | a real effort-test wave judged by the script, beside the fault key and git | unplaced |
| D14 | test_path_and_quoted_line_pass | tools/test_effort_test_scout.py:45 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_an_absolute_windows_path_passes | tools/test_effort_test_scout.py:49 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_a_wrong_path_fails | tools/test_effort_test_scout.py:53 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_a_wrong_line_fails | tools/test_effort_test_scout.py:57 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_a_longer_file_name_is_not_the_path | tools/test_effort_test_scout.py:60 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_the_match_is_case_exact | tools/test_effort_test_scout.py:64 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_each_answer_file_gets_a_line | tools/test_effort_test_scout.py:69 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_a_missing_answer_file_fails_loudly | tools/test_effort_test_scout.py:84 | a real scout answer file checked by the script against a real key | unplaced |
| D14 | test_each_labelled_file_holds_the_run_the_key_says | tools/test_effort_test_shuffle.py:95 | a real six-branch bundle built by the script | unplaced |
| D14 | test_a_run_name_is_replaced_by_that_runs_label | tools/test_effort_test_shuffle.py:112 | a real six-branch bundle built by the script | unplaced |
| D14 | test_the_order_is_shuffled | tools/test_effort_test_shuffle.py:118 | a real six-branch bundle built by the script | unplaced |
| D14 | test_a_key_in_a_folder_beside_the_bundle_is_allowed | tools/test_effort_test_shuffle.py:145 | a real six-branch bundle built by the script | unplaced |
| D14 | test_an_unknown_branch_writes_nothing | tools/test_effort_test_shuffle.py:156 | a real six-branch bundle built by the script | unplaced |
| D14 | test_a_broken_transcript_line_fails_loudly | tools/test_effort_test_taint.py:216 | a real subagent transcript read by the script | unplaced |
| D5 | 3137 pid no process holds | TG:3138 | a claim about a real process or its exit on the real clock (threads, timing) | no run |
| D5 | 3166 terminated runs no atexit | TG:3169 | a claim about a real process or its exit on the real clock (threads, timing) | no run |
| D5 | 3171 normal exit runs atexit | TG:3174 | a claim about a real process or its exit on the real clock (threads, timing) | no run |
| D2 | 617 | DF:543 | the real dump.cs of Clone Hero build 1.1.0.6142 (outside the repo; docs-facts page checked against it) | unavailable input: dump.cs (/c/ch-dumps absent) |
| D2 | 662 | DF:593 | the real dump.cs of Clone Hero build 1.1.0.6142 (outside the repo; docs-facts page checked against it) | unavailable input: dump.cs (/c/ch-dumps absent) |
| D2 | 742 | DF:743 | the real dump.cs of Clone Hero build 1.1.0.6142 (outside the repo; docs-facts page checked against it) | unavailable input: dump.cs (/c/ch-dumps absent) |
| D2 | 1575 | FA:1061 | real kit.ts and kit_methods.json beside the live game (kit method names) | no run |
| E3 | RJ:147 | RJ:147 | the real ref moves judged by the real judge | R8 |
| E3 | RJ:153 | RJ:153 | the real ref moves judged by the real judge | R8 |
| E3 | RJ:221 | RJ:221 | the real ref moves judged by the real judge | R8 |
| E3 | RJ:232 | RJ:232 | the real ref moves judged by the real judge | R8 |
| E3 | RJ:285 | RJ:285 | the real ref moves judged by the real judge | R8 |
| E3 | RR:252 | RR:252 | the real ref moves judged by the real judge | R8 |
| E3 | RR:313 | RR:313 | the real ref moves judged by the real judge | R8 |
| E3 | RR:324 | RR:324 | the real ref moves judged by the real judge | R8 |
| E3 | RR:328 | RR:328 | the real ref moves judged by the real judge | R8 |
| E3 | RR:341 | RR:341 | the real ref moves judged by the real judge | R8 |
| D9 | D9.md:417 | test_trace_hits.py:336 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:540 | test_cheap_checks.py:73 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:544 | test_cheap_checks.py:79 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:637 | test_debugger.py:125 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:641 | test_debugger.py:133 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:644 | test_debugger.py:140 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:647 | test_debugger.py:146 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:655 | test_debugger.py:157 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:658 | test_debugger.py:163 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:666 | test_debugger.py:175 | what the real game memory / real log folder holds (live game) | no run |
| D9 | D9.md:719 | test_early_fill_walk.py:65 | a corpus chart with early fills dumped at other windows beside the generated chart | R2 |
| A9 | 29 whole-number key | tests/test_config.cpp:764 | the real hydra_settings.ini the tool writes (whole-number key) | R5 |
| A12 | 10 premise maps | tests/test_docs_match_code.cpp:1887 | the real plans against the real shape page (docs check, a text read) | unplaced |
| C4 | 1 CMake reader, made-up file | tests/test_layering.cpp:547 | the real CMake files through the reader | unplaced |
| A16 | 22 startup read by part | tests/test_library_jobs.cpp:1274 | a startup read of the user's real library | needs the user's library |
| D16 | key_delivery_test.py | tools/ch_probe/experiments/key_delivery_test.py:46 | a person reading Notepad / pad flashes on a live game (no assertions) | no run |
| D16 | pad_flash_test.py | tools/ch_probe/experiments/pad_flash_test.py:49 | a person reading Notepad / pad flashes on a live game (no assertions) | no run |
| D11 | 46 | TP:922 wall clock is the watch's | live Clone Hero watch run | no run |
| D11 | 99 | TP:1833 null range | live Clone Hero watch run | no run |
| A11 | 4 | DM:502 what counts as a code name | the real doc pages and code names (docs check, a text read) | unplaced |
| A11 | 6 | DM:1092 which CH lines name Hydra code | the real doc pages and code names (docs check, a text read) | unplaced |
| A35 | 5 | TV:263 clamped_sqin charts | the engine record of the clamped_sqin / twin SP end charts | R1 |
| A35 | 8 | TV:361 twin SP ends | the engine record of the clamped_sqin / twin SP end charts | R1 |
| A44 | 7 | UP:540 paths-long-path | the Paths tab on a real chart at real window sizes | R4a |
| D13 | 125 | tools/ch_probe/tests/test_tick_rate_vs_hydra.py:46 | a hydra_replay dump of a chart beside the tool's own reading | R1 |
| A19 | PL13 | tests/test_path_list.cpp:362 | the Path Builder path list (fold state) on a real chart | R4h |
| A19 | PC5 | tests/test_precheck.cpp:505 | the hydra_precheck unknown-option line from the real tool | unplaced |
| C9 | 1.2 | test_precheck_pick.cpp:84 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.3 | test_precheck_pick.cpp:117 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.4 | test_precheck_pick.cpp:136 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.5 | test_precheck_pick.cpp:158 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.6 | test_precheck_pick.cpp:180 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.7 | test_precheck_pick.cpp:200 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.8 | test_precheck_pick.cpp:223 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.9 | test_precheck_pick.cpp:241 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.10 | test_precheck_pick.cpp:258 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.11 | test_precheck_pick.cpp:277 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.12 | test_precheck_pick.cpp:292 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.17 | test_precheck_pick.cpp:388 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.27 | test_precheck_pick.cpp:442 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.28 | test_precheck_pick.cpp:450 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.29 | test_precheck_pick.cpp:483 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.30 | test_precheck_pick.cpp:492 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.31 | test_precheck_pick.cpp:502 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.32 | test_precheck_pick.cpp:515 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.33 | test_precheck_pick.cpp:529 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.34 | test_precheck_pick.cpp:549 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.35 | test_precheck_pick.cpp:561 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.38 | test_precheck_pick.cpp:623 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.39 | test_precheck_pick.cpp:631 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 1.40 | test_precheck_pick.cpp:640 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 3.1 | test_precheck_pick_lines.cpp:38 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 3.2 | test_precheck_pick_lines.cpp:68 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 3.4 | test_precheck_pick_lines.cpp:103 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.1 | test_precheck_pick_names.cpp:28 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.2 | test_precheck_pick_names.cpp:38 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.3 | test_precheck_pick_names.cpp:49 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.4 | test_precheck_pick_names.cpp:69 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.5 | test_precheck_pick_names.cpp:83 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.6 | test_precheck_pick_names.cpp:95 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.7 | test_precheck_pick_names.cpp:106 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.8 | test_precheck_pick_names.cpp:122 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.9 | test_precheck_pick_names.cpp:134 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.10 | test_precheck_pick_names.cpp:143 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.11 | test_precheck_pick_names.cpp:158 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.12 | test_precheck_pick_names.cpp:167 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C9 | 4.13 | test_precheck_pick_names.cpp:177 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 1.1 | test_precheck_check1.cpp:121 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 1.2 | test_precheck_check1.cpp:139 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 1.3 | test_precheck_check1.cpp:152 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 1.4 | test_precheck_check1.cpp:165 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 3.1 | test_precheck_check3.cpp:105 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 3.3 | test_precheck_check3.cpp:128 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 5.1 | test_precheck_check5.cpp:110 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 5.2 | test_precheck_check5.cpp:127 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 4.1 | test_precheck_git.cpp:83 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 4.2 | test_precheck_git.cpp:116 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| C8 | 4.10 | test_precheck_git.cpp:297 | real repo history / real source text through hydra_precheck (record: claim not checked only) | unplaced |
| D10 | 63 | test_ocr.py:37 empty_is_none | live game screen read by OCR | no run |
| D10 | 68 | test_ocr.py:56 none_input | live game screen read by OCR | no run |
| A17 | 16 | test_library_query.cpp:300 term applies to column | the Library view search on the corpus | R4b |
| A17 | 17 | test_library_query.cpp:313 unknown field names | the Library view search on the corpus | R4b |
| A17 | 40 | test_main.cpp whole file | nothing: the file has no assertion | no run |
| D8 | :422 | tools/ch_mcp/tests/test_method_watch.py:422 | the real game dump / folders the tool reads (live game files) | no run |
| D8 | :426 | tools/ch_mcp/tests/test_method_watch.py:426 | the real game dump / folders the tool reads (live game files) | no run |
| D8 | :435 | tools/ch_mcp/tests/test_method_watch.py:435 | the real game dump / folders the tool reads (live game files) | no run |
| D8 | :453 | tools/ch_mcp/tests/test_method_watch.py:453 | the real game dump / folders the tool reads (live game files) | no run |
| D8 | :133 | tools/ch_mcp/tests/test_press_schedule.py:133 | a real schedule driving the real press scheduler (live kit) | no run |
| D8 | :161 | tools/ch_mcp/tests/test_press_schedule.py:161 | a real schedule driving the real press scheduler (live kit) | no run |
| D8 | :290 | tools/ch_mcp/tests/test_quiet.py:290 | the real game dump / folders the tool reads (live game files) | no run |
| A7 | #1 | tests/test_app_reports.cpp:81 | the Path report window on real corpus charts | R4f |
| A7 | #2 | tests/test_app_reports.cpp:109 | the Path report window on real corpus charts | R4f |
| A7 | #3 | tests/test_app_reports.cpp:123 | the Path report window on real corpus charts | R4f |
| A7 | #4 | tests/test_app_reports.cpp:150 | the Path report window on real corpus charts | R4f |
| A7 | #5 | tests/test_app_reports.cpp:161 | the Path report window on real corpus charts | R4f |
| A7 | #6 | tests/test_app_reports.cpp:185 | the Path report window on real corpus charts | R4f |
| A7 | #7 | tests/test_app_reports.cpp:208 | the Path report window on real corpus charts | R4f |
| A7 | #8 | tests/test_app_reports.cpp:240 | the Path report window on real corpus charts | R4f |
| A7 | #9 | tests/test_app_reports.cpp:267 | the Path report window on real corpus charts | R4f |
| A7 | #10 | tests/test_app_reports.cpp:327 | the Path report window on real corpus charts | R4f |
| A7 | #11 | tests/test_app_reports.cpp:344 | the Path report window on real corpus charts | R4f |
| A7 | #43 | tests/test_app_shell.cpp:401 | the Path report window on real corpus charts | R4f |
| A7 | #47 | tests/test_app_shell.cpp:470 | the Path report window on real corpus charts | R4f |
| D1 | 21 | the_servers_env_backs_up_the_key_with_reg_exe :492 | a live bot run of Clone Hero | no run |
| D1 | 49 | a_song_ini_with_nowhere_for_a_title_is_refused_by_the_writer | a live bot run of Clone Hero | no run |
| D1 | 53 | test_chdis.py dis_prints_the_listing_up_to_the_padding :97 | a live bot run of Clone Hero | no run |
| A47 | 2 | test_open_path :1051 | the report windows on a real run | R4f |
| A47 | 6 | test_reopen :1345 | the report windows on a real run | R4f |
| A47 | 7 | test_reopen_keeps_filters :1419 | the report windows on a real run | R4f |
| A23 | 2 | combo_after :150 | the engine record of a real chart (replay rows) | R1 |
| A20 | 2 | color_of :70 | the Preview highway on a real chart | R4a |
| A20 | 3 | song length handed in :80 | the Preview highway on a real chart | R4a |
| A20 | 7 | solo spans :174 | the Preview highway on a real chart | R4a |
| A20 | 10 | unanalyzed offers every fill :227 | the Preview highway on a real chart | R4a |
| A20 | 14 | 1.0 rule :289 | the Preview highway on a real chart | R4a |
| A20 | 15 | beats from timing :306 | the Preview highway on a real chart | R4a |
| A20 | 17 | 6/8 steps :351 | the Preview highway on a real chart | R4a |
| A20 | 18 | fainter line at a restart :373 | the Preview highway on a real chart | R4a |
| A20 | 20 | beat lines stop at the length :411 | the Preview highway on a real chart | R4a |
| D6 | test_launch_core.py:221 | test_launch_core.py:221 malformed_values_are_refused | the game's real launch patch (live game files) | no run |
| D6 | test_launch_core.py:228 | test_launch_core.py:228 write_of_another_length | the game's real launch patch (live game files) | no run |
| D6 | test_launch_core.py:234 | test_launch_core.py:234 no_patch_object | the game's real launch patch (live game files) | no run |
| A26 | RG3 | test_run_guard.cpp:36 | the run guard's answer on real command lines | unplaced |
| A26 | RG4 | test_run_guard.cpp:43 | the run guard's answer on real command lines | unplaced |
| A26 | RG6 | test_run_guard.cpp:65 | the run guard's answer on real command lines | unplaced |
| A26 | RU2 | test_rules.cpp:77 | a real older hydra_rules.ini (record: claim not checked) | unavailable input: older hydra_rules.ini |
| A26 | RU4 | test_rules.cpp:102 | a real older hydra_rules.ini (record: claim not checked) | unavailable input: older hydra_rules.ini |
| A26 | RU5 | test_rules.cpp:111 | a real older hydra_rules.ini (record: claim not checked) | unavailable input: older hydra_rules.ini |
| A26 | RU6 | test_rules.cpp:140 | a real older hydra_rules.ini (record: claim not checked) | unavailable input: older hydra_rules.ini |
| A26 | RU20 | test_rules.cpp:415 | real corpus charts searched with and without the rule | R2 |
| A26 | RU21 | test_rules.cpp:430 | real corpus charts searched with and without the rule | R2 |
| WN1 | W13 | test_search_target.cpp:420 | a targeted search on a real chart | R3 |
| WN3 | 10 | test_tied_variants.cpp:263 | real tied-variant offsets with a tie limit (no dump flag) | unplaced |
| C5 | 63 | :129 | the engine record of the game-test charts (freeze answer) | R1 |
| A18 | 2 | :43 | the Paths tab on a real chart | R4a |
| A18 | 31 | :941 | the Paths tab on a real chart | R4a |
| A18 | 33 | :962 | the Paths tab on a real chart | R4a |
| A18 | 37 | :1034 | the Paths tab on a real chart | R4a |
| A18 | 54 | :1404 | the Paths tab on a real chart | R4a |
| D12 | - | PC:102 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PC:107 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PC:144 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PC:159 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PS:100 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PS:110 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PS:119 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PS:125 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| D12 | - | PS:146 | a probe chart the tool writes, read by the engine (the generated chart is not in testdata/input) | unplaced |
| B2 | - | MIDI:100 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:118 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:140 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:157 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:168 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:177 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:191 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:201 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:240 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:252 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:261 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:302 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:432 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:465 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:474 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:532 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:561 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:591 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:616 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:647 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:717 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:767 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:800 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | MIDI:909 | a corpus .mid beside an independent reader (mido) or the game; R1 gives only the engine side | unplaced |
| B2 | - | PR:115 | the pathstring text of a real chart from the real writer | R1 |
| B2 | - | PR:135 | the pathstring text of a real chart from the real writer | R1 |
| D3 | - | tools/ch_mcp/tests/test_frida_tools.py:588 each_name_sits_at | the real game dump.cs (the test skips when absent) | unavailable input: dump.cs |
| D3 | - | tools/ch_mcp/tests/test_frida_tools.py:624 each_enum_word_si | the real game dump.cs (the test skips when absent) | unavailable input: dump.cs |
| A25 | P1 | tests/test_report_page.cpp:62 write_report_file swaps the pa | the report page / rows on real charts | R4f |
| A25 | R25 | tests/test_report_rows.cpp:902 generate_report hands back no | the report page / rows on real charts | R4f |
| A25 | R29 | tests/test_report_rows.cpp:962 PageSettings::of differs when | the report page / rows on real charts | R4f |
| A25 | R30 | tests/test_report_rows.cpp:976 settings_change_touches reads | the report page / rows on real charts | R4f |
| A27 | SF9 | tests/test_saved_figure_fill.cpp:341 a result this build sav | a real saved batch result of two corpus charts | R5 |
| A27 | ST10 | tests/test_search_target.cpp:420 free after: fills past the | a targeted search on a real chart with named ticks | R3 |
| A27 | ST11 | tests/test_search_target.cpp:451 a pinned window over the Pa | a targeted search on a real chart with named ticks | R3 |
| A27 | SI3 | tests/test_song_input.cpp:85 a drum track named PART DRUM | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI4 | tests/test_song_input.cpp:99 Chord: a modifier on a missing | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI6 | tests/test_song_input.cpp:130 parse_finite_number reads only | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI7 | tests/test_song_input.cpp:143 .chart: an Offset that is not | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI8 | tests/test_song_input.cpp:160 song.ini: a delay that is not | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI13 | tests/test_song_input.cpp:351 display_title: tags go, words | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| A27 | SI17 | tests/test_song_input.cpp:377 display_charter: tags go | a real song / song.ini of the named kind (corpus holds none for the edge case) | unplaced |
| C11 | sng metadata pairs | SNG:57 | a real .sng container (none in testdata; only a fuzz seed) | unavailable input: real .sng |
| C11 | sng file table | SNG:65 | a real .sng container (none in testdata; only a fuzz seed) | unavailable input: real .sng |
| C11 | pick_notes_file | SNG:312 | a real .sng container (none in testdata; only a fuzz seed) | unavailable input: real .sng |
| C11 | find_song_ini | SNG:328 | a real .sng container (none in testdata; only a fuzz seed) | unavailable input: real .sng |
| C11 | SL no length no notes | SL:269 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL clamped window still offers squeeze-out | CL:114 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL two ends clamped | CL:128 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL tied clamps | CL:168 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL plain bar ties ceiling | CL:210 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL folded variant Clamped to SqIn | CL:284 | the engine record of a real chart (song length / SP end clamp) | R1 |
| C11 | CL unclamped window | CL:322 | the engine record of a real chart (song length / SP end clamp) | R1 |
| A34 | R8 Profile compares | SR:339 | a real store written by hydra_batch | R5 |
| A34 | T3 apply_theme shades | TH:49 | a screen showing the theme colours (any screen capture) | R4b |
| A34 | T7 hover teal | TH:104 | a screen showing the theme colours (any screen capture) | R4b |
| A33 | T2 in-memory reads itself | SRC:130 | a real store written by hydra_batch | R5 |
| A29 | 13 no SP no path | MC:406 | the Preview gauge on a real chart | R4a |
| DL3 | 1 | click.cpp:549 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 5 | click.cpp:636 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 6 | click.cpp:658 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 7 | click.cpp:697 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 8 | click.cpp:708 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 9 | click.cpp:720 | the Analyze library batch screens on the corpus | R4c |
| DL3 | 32 | rep.cpp:952 | the Path report window after a real analysis | R4f |
| DL3 | 38 | rep.cpp:1097 | the Path report window after a real analysis | R4f |
| DL3 | 39 | rep.cpp:272 | the Path report window after a real analysis | R4f |
| B1 | 7 | test_cli.cpp:312 | the real hydra_batch message on a real run | R5 |
| B1 | 21 | test_cli.cpp:710 | the real exe's import table (a file read, no run) | unplaced |
| B1 | 22 | dyn.cpp:82 | the Dynamics tab on a real chart | R4e |
| B1 | 27 | dyn.cpp:249 | the Dynamics tab on a real chart | R4e |
| B1 | 36 | dyn.cpp:365 | the Dynamics tab on a real chart | R4e |
| B1 | 37 | dyn.cpp:373 | the Dynamics tab on a real chart | R4e |
| B1 | 38 | dyn.cpp:381 | the Dynamics tab on a real chart | R4e |
| WN4b | 7 | tests/test_pb_coach.cpp:276 | the engine ends of a real chart | R1 |
| WN4b | 31 | tests/test_saved_figure_fill.cpp:341 | a real saved batch result of corpus charts | R5 |
| WN5 | 2 | tests/ui/uitest_preview.cpp:151 | the Preview on a real chart | R4a |
| WN7 | 4 | tests/test_pb_files.cpp:84 | the Path Builder file save on Burnout | R4h |
| A41 | 1 | uitest_library.cpp:62 | the Library view on the corpus | R4b |
| A41 | 6 | uitest_library.cpp:281 | the Library view on the corpus | R4b |
| A41 | 13 | uitest_library.cpp:706 | the Library view on the corpus | R4b |
| A41 | 15 | uitest_library.cpp:805 | the Library view on the corpus | R4b |
| A41 | 18 | uitest_library.cpp:1015 | the Library view on the corpus | R4b |
| DL1 | 171 | test_store_records.cpp:171 | a real store written by hydra_batch | R5 |
| DL1 | 178 | test_store_records.cpp:178 | a real store written by hydra_batch | R5 |
| DL1 | 456 | test_store_records.cpp:456 | a real store written by hydra_batch | R5 |
| DL1 | 536 | test_store_records.cpp:536 | a real store written by hydra_batch | R5 |
| DL1 | Profile | test_store_results.cpp:339 | a real store written by hydra_batch | R5 |
| DL2 | 1 | test_store_open.cpp:183 | a real older library file opened by Hydra | R6 |
| DL4 | 6 | test_model.cpp:275 | the Analysis settings panel defaults | R4d |
| A45 | 2 | uitest_preview.cpp:90 | the Preview on a real chart | R4a |
| A45 | 3 | uitest_preview.cpp:151 | the Preview on a real chart | R4a |
| A45 | 7 | uitest_preview.cpp:534 | the Preview on a real chart | R4a |
| A45 | 15 | uitest_preview.cpp:849 | the Preview on a real chart | R4a |
| A45 | 16 | uitest_preview.cpp:922 | the Preview on a real chart | R4a |
| D2 | 687 | DF:681 | the real dump.cs of build 1.1.0.6142 (docs-facts names map) | unavailable input: dump.cs (/c/ch-dumps absent) |
| A17 | 35 | test_long_paths | the real settings / report page paths on a long-path run | R5 |
| A17 | 39 | test_long_paths | the real settings / report page paths on a long-path run | R5 |
| B2 | - | PR:99 | the pathstring text of a real chart from the real writer | R1 |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:326 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:333 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:338 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:346 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:353 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:369 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:374 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:378 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| D3 | - | tools/ch_mcp/tests/test_game_files.py:400 | the game's real settings.ini / song.ini beside the tool's edit (live game files) | no run |
| B7 | 3 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 4 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 5 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 6 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 7 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 8 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 9 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 10 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 12 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 15 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 16 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 17 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 18 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| B7 | 21 | tests/test_timing.cpp | the tempo map of a real chart | R1 |
| C14-C15-fuzz | fuzz 1 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15-fuzz | fuzz 2 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15-fuzz | fuzz 3 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15-fuzz | fuzz 4 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15-fuzz | fuzz 5 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15-fuzz | fuzz 6 | fuzz target | nothing: no expected value, bytes only | no run |
| C14-C15 | parse_baseline | single_owner rows | nothing: no assertion or own examples | no run |
| C14-C15 | today_iso | single_owner rows | nothing: no assertion or own examples | no run |
| C14-C15 | copy_entry_name | single_owner rows | nothing: no assertion or own examples | no run |
| C14-C15 | compile_rule/flags_line | single_owner rows | nothing: no assertion or own examples | no run |
| C14-C15 | in_scope | single_owner rows | nothing: no assertion or own examples | no run |
| C14-C15 | lexer_language/skipped_comment_line | scan.cpp:452-476 | real .ps1/.yml files read by the scanner (record asked) | unplaced |
| A49 | 16 | scan.cpp:379-383 | the real rule-file list through select_rule_files | unplaced |
| A49 | 5 | tests/pb_ui/pbtest_window.cpp | Path Builder on a chart with nothing to build | R4h |
| A49 | 26 | tests/pb_ui (main) | the Path Builder argument loop | R4h |
| A6 | 28 | tests/test_app_library.cpp | a real saved v2.1.0 library through the scan notice | R6 |
| A38 | 1 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 2 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 3 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 4 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 5 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 6 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 7 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 8 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 9 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 10 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 11 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 12 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A38 | 13 | run_command (no test case) | the Path Builder command interpreter on a real chart | R4h |
| A28 | 1 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 2 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 3 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 8 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 9 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 10 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 16 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 19 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 20 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A28 | 22 | tests (build_tail_song) | the engine tail figures of a real chart | R1 |
| A31 | SR1 | test_stem_reader.cpp:225 | a real stem file per song (corpus has only sine220) | unavailable input: real stems |
| A31 | SR14 | test_stem_reader.cpp:557 | a real stem file per song (corpus has only sine220) | unavailable input: real stems |
| A32 | 3 | so.cpp:183 | a real older library file classified by the store | R6 |

## 4. What I could not place

The `unplaced` rows (171): the D14 Python tool tests (68), the C8 and C9 precheck tests, D12 probe-chart tests, B2 MIDI tests, A27 song-input tests, A26 run-guard tests, A12, A11, C4, A19 PC5, B1 21, WN3 10, the two A49/C14-C15 scanner rows. They need a tool run on real repo files or real handoffs, or a generated chart the corpus does not hold; RUNS.md has no run for these and I did not write one.

Count check. TOTALS.md read minus faulty gives 406 clean; minus the 6 compare_db rows and the 2 IR21 DL5b rows that are the same two tests gives 398. This list holds 397. I could not find the one missing row without reopening every count file; the count files themselves may differ from TOTALS.md by one (for example IR2 and IR3 prose-listed clean rows).
