# Step 2: Hydra Reads Charts the Way Clone Hero Does — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every rule the chart readers apply has one owner, and where Clone Hero 1.1's own code answers the question, the owner gives Clone Hero's answer. Disco flip and 2x kicks are read per difficulty, a Star Power phrase pays on the notes Clone Hero pays, each authored fill is placed on its own with Clone Hero's landing window, the dynamics tag counts only in Clone Hero's two spellings, and the small parser drifts each get one home. The stored-data stamps move once, at the end, so old saved results and counts are redone.

**Architecture:** The parser in `src/parse/song.cpp` gains one difficulty table (kick pitch, disco digit, 2x pitch), one phrase-end helper and one fill-placement pass, and both the `.mid` and the `.chart` reader use them. Display code keeps reading what the parser stores. Nothing in the search engine changes in the wave. Two engine follow-ups (findings 37 and 304) wait for step 1 to merge, because step 1 rewrites the files they touch.

**Tech Stack:** C++17 (`CMAKE_CXX_STANDARD 17`, CMakeLists.txt line 22), doctest (`hydra_tests`), Dear ImGui test engine (`hydra_uitest`), `hydra_replay` (built with `-Target hydra_replay`; it is EXCLUDE_FROM_ALL), CMake through `build_cpp.ps1`, PowerShell for the corpus comparison, Python 3.14 for the probe tools (T8) and one library pre-filter (T10).

**Spec:** decisions D19-D31 under "Step 2" in `docs/audit/2026-10-03-fix-decisions.md`; the findings they name in `docs/audit/2026-10-03-derivation-audit.md`; the measured briefs and their verifier corrections in session bbf41dc4's scratchpad (`s2-plan/briefs/*.json`, corrections applied below); the integration rulings in `s2-plan/rulings.md`. Appendix A lists every place this plan departs from or adds to those rulings.

**Base branch:** `claude/s1-t3`, commit 1abd188. Every line number in this plan is from that commit unless a task says otherwise. T10 and T11 branch from `main` after step 1 merges, and say so.

## Global Constraints

Each task changes only the functions named for it. `src/parse/song.cpp` is shared by T0 to T6, so each of those tasks quotes the lines it replaces, and a task that needs another task's function lists it as a dependency instead of editing it.

Each task writes its new tests in its own new file, `tests/test_s2_<topic>.cpp`, and adds one line at the end of the explicit test list in `CMakeLists.txt` (the `add_executable(hydra_tests ...)` list, lines 369-429; the new line goes after line 429, `tests/test_long_paths.cpp`). An existing test may change only where the task's rule change breaks what it pins, and the task names each one. T11 is the one exception: its ruling puts its test beside the existing backend-value tests in `tests/test_model.cpp`.

The code is C++17. No defaulted comparison operators, no `std::span`, no `contains()` on maps. Tests that compare structs write `operator==` by hand.

No task bumps a stamp itself. Each lists its stamp need, and T9 makes every stamp change at once. Until T9 is merged, a development build must never open a real `hydra.db`; use scratch databases passed with `--db`.

Commits are staged by file name, never `git add -A`, because other sessions leave edits in this checkout. Every commit carries `Task:`, `Agent:` and `Session:` trailers and the `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>` line. Plain English in every comment, commit message and doc, per CLAUDE.md.

Every task's "done when" includes two checks: the full `hydra_tests` suite passes, and the corpus score check (below) shows exactly the changes the table names for that task, which for most tasks is none.

**User decisions this plan implements (all 2026-10-03):**

- **D19 (findings 11, 250):** "Disco flip is per difficulty: each difficulty reads only its own `[mix N drums...]` markers, in both formats, as Clone Hero does." `drums0dnoflip` stays off. Clone Hero's one-letter on/off rule (A+) is not adopted. The "flip only with Pro Drums" gate moves to one place. Tasks T0, T1.
- **D20 (findings 10, 12, 255):** "Each difficulty reads its own 2x kick: MIDI 59 Easy, 71 Medium, 83 Hard, 95 Expert, and `.chart` N 32 in its own section." The 2x Bass box works at every difficulty. The Dynamics tab shows one kick total in both "All kicks" and Totals. No record-key fallback. Tasks T0, T2.
- **D21 (finding 21):** one phrase-end helper for both parsers, with Clone Hero's rule: start <= tick < end; a zero-length phrase pays nothing; a phrase running past the last note pays on that note. Task T3.
- **D22 (finding 315, the +1):** "The fill landing window is floor(resolution x slop) + 1 ticks", also on top of a user's `hydra_rules.ini` slop. Task T4.
- **D23 (finding 344):** "The written `kResultsStamp` rule names `src/parse`." Step 2 reuses step 1 Task 22's value and does not bump twice. Task T9.
- **D24 (finding 64):** the dynamics tag counts only as `ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]`, and at a shared tick file order decides. A late tag gets the "from <time> on (N earlier markings ignored by Clone Hero)" line. Tasks T5 (code), T9 (stamps).
- **D25 (finding 222):** off-speed leaderboard scores get their own "other speed" status and leave Matched, Above optimal and Points left on table. Task T7.
- **D26 (findings 53, 314, open half of 315):** no live Clone Hero check now. Generated-fill length and 6/8 beats keep today's rule, written down as unverified. Task T9 (record only).
- **D27 (small owners):** 60, 61, 258/319, 331, R7.5, R7.7, 77 as recommended; 141 and 320 recorded only. Tasks T6, T8, T9.
- **D28 (hit-window plan):** steps 1-3 answered from Clone Hero's code and `poll_windows.csv`; steps 4-6 shelved. Task T8.
- **D29 (findings 37, 304, after step 1 merges):** 37: "the SP-end extension during Star Power comes only from `extend_deacts`, which stops at a full meter, as Clone Hero does"; counted after the merge. 304: "a backend note at exactly +3.0 ms stays uncounted (strict); record it and pin it with a test." Tasks T10, T11, T9 (record).
- **D30 (finding 315, Fill B):** "Authored fills are placed one by one after the whole chord list is read, as Clone Hero does (0x20CFF60 calling 0x5DE030). Each fill takes the last chord at or before its end, but not before its start, or the first chord after its end within the slop. A tie goes to the later chord, and a fill with no chord in bounds is dropped." Clone Hero's unbounded one-side fallback (B2) is not adopted. Task T4, in the same task as the +1, since both own fill placement.
- **D31 (step-2 plan wording, Q1-Q5 and the review):** "Approved as the plan proposes." It settles the 2x Bass tooltip at every difficulty, the UserGuide's 2x lines, the "2x kicks: X of Y" line, the late tag stored in milliseconds, the singular, T7's "Other speed" filter, stat row, status-help sentence, dimmed Points-left cell, ", N at other speeds" clause and UserGuide sentence, and T4's `fill_land_slop_beats` UserGuide line. Tasks T2, T4, T5, T7.

---

## What changes, in plain words

Clone Hero reads each difficulty of a drum chart on its own. Hydra didn't, in two places. Below Expert it applied Expert's disco sections, so it flipped hi-hats that the lower difficulty already wrote on yellow. It also let Expert's 2x kicks leak into the Dynamics tab. Both now follow the difficulty being read.

A Star Power phrase in a `.chart` file paid on the wrong notes at its two edges. A phrase of length zero paid a bar it shouldn't, and a phrase that ran past the last note paid nothing. Both parsers now share one helper with Clone Hero's rule.

Fills change in two small ways. A fill lands one tick later, as Clone Hero computes it. And each fill is now placed on its own once every chord is read. Before, Hydra forgot a fill when the next fill started before any chord reached the first fill's end, so a path's skip count could say one fewer fill than the game shows.

The rest is small. A one-bracket dynamics tag no longer turns dynamics on. A handful of parser rules that were written twice get one owner each. The leaderboard page stops comparing scores played at another speed. The probe tools use the measured hit-window cap.

The stamps that guard saved data move once, in T9. Every saved result then reads Stale once (through step 1 Task 22's bump, which this plan shares), and every saved Dynamics count is recounted in the background.

## Answered questions

The decisions above settle every rule. These five were wording and display details the decisions did not spell out. The user answered all five on 2026-10-03 in D31 ("Approved as the plan proposes"), so each recommendation below is now the approved text. D31 also covers the review's wording gaps, listed after Q5.

**Q1. The 2x Bass tooltip and UserGuide wording (T2).** D20 makes the box work at every difficulty but gives no text. The tooltip now shows at every difficulty, Expert included, where today there is none. *Approved (D31):* tooltip "Include the chart's 2x kicks, like Clone Hero's Double Kick." UserGuide line 41: "Include the chart's 2x kicks, like Clone Hero's Double Kick modifier. It works at every difficulty. Each difficulty has its own 2x kicks, though few charts have any below Expert." UserGuide line 168 ends "both All kicks and the totals leave it out."

**Q2. The Dynamics tab's "2x kicks: X of Y kick notes" line (T2).** D20 says "one kick total feeds both All kicks and Totals" but doesn't name this line. *Approved (D31):* it keeps counting every charted kick, as today, because it describes the chart. If it followed the 2x Bass box, with the box off it would read "12 of 441 kick notes", where the 12 are not part of the 441.

**Q3. What T5 stores for a late dynamics tag (T5).** The ruling says "the tag's tick". The Dynamics tab draws from the stored blob and has no tempo map, so a tick can't become "from 4:01" there. *Approved (D31):* the Song keeps the tick (where the parser knows it), and the breakdown and its blob store the tag's time in milliseconds, converted once in `count_dynamics`. It is still two stored facts per difficulty.

**Q4. The singular in the late-tag line (T5).** *Approved (D31):* "1 earlier marking ignored by Clone Hero" for one, through the app's existing `count_label`. No library chart has exactly one today.

**Q5. The leaderboard counts sentence (T7).** D25 adds a status but no wording for the totals. *Approved (D31):* when there are off-speed scores, the finished window and the page subtitle add one clause, for example "Done: 120 matched, 4 above optimal, 0 not analyzed, 3 not in your library, 2 at other speeds." With none, the text stays exactly as today.

**The review's wording gaps, also approved in D31.** The review found visible text that no question above named. D31 approves each one as the plan writes it.

T7's page gets an "Other speed" filter option, worded `Other speed`.

T7's stat row above the table gets an "Other speed" entry with its count.

T7's Status column help gains one sentence: "Other speed: played at a speed other than 100%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared."

T7 dims the Points-left cell on an off-speed row, the same grey as an empty cell.

T7 adds one UserGuide sentence after line 220, quoted in T7 Step 7.

T4 rewrites the UserGuide line for `fill_land_slop_beats`. It now says the +1 tick applies on top of the setting, and that the setting covers only fills written in the chart. The text is in T4 Step 4.

## Task list

| # | Task | Decisions | Branches from | Stored results change? | Stamp need (for T9) |
|---|---|---|---|---|---|
| T0 | One difficulty table | D19, D20 | `claude/s1-t3` | no | none |
| T1 | Disco flip per difficulty | D19 | T0's commit | yes (Hard, Medium, Easy, Pro Drums) | results (shared), dynamics count |
| T2 | 2x kicks per difficulty | D20 | T0's commit | yes (12 library charts, 2x on) | dynamics count (results ride on step 1's bump) |
| T3 | One phrase-end helper | D21 | `claude/s1-t3` | yes (21 library charts) | results (shared) |
| T4 | Fill placement: Clone Hero's extra tick, and each fill on its own | D22, D30 | `claude/s1-t3` | yes (2 library charts' scores; best-path fill counts on 2-5 library charts per difficulty) | results (shared) |
| T5 | Dynamics tag: two spellings, file order | D24 | `claude/s1-t3` | yes (1 library chart) | results (shared), dynamics count, dynamics blob |
| T6 | Small parser owners | D27 | `claude/s1-t3` | no | none |
| T7 | Off-speed leaderboard rows | D25 | `claude/s1-t3` | no | none |
| T8 | Probe constants, hit-window plan | D27, D28 | `claude/s1-t3` | no | none |
| T9 | Stamps and records (last) | D23, D26, D27, D29, D30 | `claude/s1-t3` | the bumps | owns them |
| T10 | Squeeze-out end reads the cap (finding 37) | D29 | `main` after step 1 | yes (count first) | results (shared) |
| T11 | Pin the +3.0 ms leeway edge (finding 304) | D29 | `main` after step 1 | no | none |

## Execution graph

Picture the wave as ten workers on separate benches. Eight can start at once. The other two (T1 and T2) need a part from T0 first, and T0 is a small job. Nobody waits for anybody else's bench; only the final assembly has an order.

```
claude/s1-t3 (1abd188)
 ├─ T0 ──┬─ T1
 │       └─ T2
 ├─ T3   ├─ T4   ├─ T5   ├─ T6   ├─ T7   ├─ T8   └─ T9
 │
 └─ claude/s2-int:  merge T0, T1, T2, T3, T4, T5, T6, T7, T8, then T9
                     then one full build, the full suites, and the corpus check

main ── step 1 merges (with its Task 22) ──┬─ T10 (count shown to the user before it lands)
                                           └─ T11
```

**Before the wave.** Save the two score scripts and record the baseline (next section). It uses binaries already built at 1abd188, so it doesn't wait for any task, and every lane compares against the same files.

**Wave (all in parallel).** Each task runs in its own worktree `.claude/worktrees/s2-tN` on branch `claude/s2-tN`. T0 starts first. T1 and T2 start the moment T0 has a green commit, and branch from it. T3 to T9 start immediately from `claude/s1-t3`. T9 can be written in parallel, because every stamp each task needs is already known from the table above; it only merges last.

```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test
foreach ($n in 0, 3, 4, 5, 6, 7, 8, 9) {
    git worktree add ".claude\worktrees\s2-t$n" -b "claude/s2-t$n" 1abd188
}
# after T0 commits:
git worktree add .claude\worktrees\s2-t1 -b claude/s2-t1 claude/s2-t0
git worktree add .claude\worktrees\s2-t2 -b claude/s2-t2 claude/s2-t0
```

**Merge order into `claude/s2-int`** (created from `claude/s1-t3`): T0, T1, T2, T3, T4, T5, T6, T7, T8, then T9. The integrator resolves textual conflicts, and expects them in five places.

The `CMakeLists.txt` test list gets one line from every task, all at the same spot.

`src/parse/song.cpp` is touched by T0 to T6, each in its own functions. Each task's "Merge notes" names its neighbours, one sentence each. The busiest spots are the `MOp::tick` comment (line 357: T3, T4, T5) and `MidiParser::run` (T3, T4, T5). Next come pass 2 of `MidiParser::parse` (lines 704-713: T4, T5) and the end of `ChartParser::parse` (lines 1229-1251: T3, T4, T6).

`src/app/dynamics_breakdown.cpp` and `.h` and `src/ui/dynamics_tab.cpp` are touched by both T2 (one kick total) and T5 (two new blob fields).

`docs/UserGuide.md` is touched by T2, T4 and T7, on different lines.

`tests/test_dynamics_store.cpp` is touched only by T9.

The integrator builds and runs every suite once, after T9, not after each merge.

**After the wave.** The integrator runs the full proof (below) on `claude/s2-int` and checks the corpus and library numbers against the expected-changes tables.

**Gates before `main`.** Nothing in this plan merges to `main` until three things hold. Step 1 has merged to `main`, with its Task 22 stamp bump, because this plan's results-stamp value is Task 22's. The derive-once review gate (`docs/superpowers/plans/2026-10-03-derive-once-review-gate.md`) is live. The user says yes. `claude/s2-int` then merges onto that `main`; the one expected conflict is `src/store/stored_versions.h`, resolved as T9 Step 6 says.

**T10 and T11.** Both branch from `main` once step 1 has merged, and they can run while the wave runs, because they touch only engine and core files the wave doesn't. T11 merges to `main` after `claude/s2-int`, behind the same review gate and the user's yes. T10 has one more gate: its corpus and library count (T10 Step 7) is shown to the user, and it lands only on a yes after that.

**Shipping together.** D20 ("no key fallback") and D23 ("does not bump twice") both assume step 1 and step 2 ship in the same release. T9 Step 6 checks that no release tag sits between step 1's Task 22 commit and step 2's merge. If one does, the merge stops and the user is asked, as a blocking question, whether to bump `kResultsStamp` a second time (Appendix A lists this as a departure from D23). T10 lands later on its own yes, so T10 Step 9 re-runs the same check when T10 lands and asks the same question if a release already carries the stamp.

## Before the wave: one score script and one baseline

Every lane and the integrator answer "did any score move?" the same way: one script dumps every chart in a list at one difficulty, and a second script prints the charts whose lines differ between two dumps. These two scripts are the only score tools this plan uses. The briefs' own `dumps.ps1` files produced the expected-result files this plan compares against, and `s2-scores.ps1` writes the same line format with the same `hydra_replay` command, so its output compares directly with theirs.

Shared paths, used by every task below:

```powershell
$sp      = "C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\bbf41dc4-b650-434b-a047-ce3abe26e342\scratchpad"
$base    = "$sp\s2-baseline"       # the two scripts and the 1abd188 baseline files
$baseBin = "$sp\s2-disco\base"     # hydra_replay.exe and hydra_batch.exe built at 1abd188 by the disco brief
$PF      = "$sp\s2-phrasefill"     # the phrase/fill brief: chart lists and expected dumps
$FB      = "$sp\s2-fillb"          # the Fill B measurement: chart lists and expected dumps
$SM      = "$sp\s2-small"          # the small-items brief: the Programmed for Battle copies
```

Use `$baseBin`, not the `s1-t3` worktree's own build: that `hydra_replay.exe` is older than the worktree's last commit. Nothing under `$sp` is ever staged. Each lane writes its own outputs under `$sp\s2-tN\`.

Save this as `$base\s2-scores.ps1`:

```powershell
# One line per chart at one difficulty: the chart, how many paths, and every
# path's string and total, in engine order. Each dump analyzes fresh in its own
# copy of an empty scratch database (a shared one gave sporadic FAILED dumps).
# Never a real hydra.db. The line format and the hydra_replay command match
# the briefs' dumps.ps1, so its files compare with theirs.
#   pwsh -File s2-scores.ps1 -Exe <hydra_replay.exe> -Diff hard -Out <file> -Work <folder>
#        [-Bass2x 0] [-Cap 2] [-List <charts.txt>]
param([Parameter(Mandatory = $true)][string]$Exe,
      [Parameter(Mandatory = $true)][ValidateSet('expert', 'hard', 'medium', 'easy')][string]$Diff,
      [Parameter(Mandatory = $true)][string]$Out,
      [Parameter(Mandatory = $true)][string]$Work,
      [string]$Bass2x = "",   # empty: hydra_replay's default, 2x on
      [string]$Cap = "",      # empty: hydra_replay's default, cap 4
      [string]$List = "",     # empty: every chart in the test corpus
      [string]$Corpus = "C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input")
New-Item -ItemType Directory -Force $Work, "$Work\emptyfolder" | Out-Null
$batch = Join-Path (Split-Path $Exe) "hydra_batch.exe"
if (-not (Test-Path "$Work\empty.db")) { & $batch "$Work\emptyfolder" --db "$Work\empty.db" *> $null }
$tag = [IO.Path]::GetFileNameWithoutExtension($Out)
$db = "$Work\db-$tag.db"; $json = "$Work\dump-$tag.json"; $err = "$Work\dump-$tag.err"
$extra = @()
if ($Bass2x) { $extra += @('--bass2x', $Bass2x) }
if ($Cap) { $extra += @('--cap', $Cap) }
$charts = if ($List) { Get-Content $List | Where-Object { $_ -ne "" } } else {
    Get-ChildItem -Recurse $Corpus -File |
        Where-Object { $_.Name -in @('notes.chart', 'notes.mid') -or $_.Extension -eq '.sng' } |
        Sort-Object FullName | ForEach-Object FullName }
$lines = foreach ($c in $charts) {
    $ok = $false
    foreach ($try in 1..3) {
        Remove-Item $json, $db, "$db-wal", "$db-shm" -ErrorAction SilentlyContinue
        Copy-Item "$Work\empty.db" $db -Force
        & $Exe dump --chart $c --db $db --out $json --difficulty $Diff @extra *> $err
        if (Test-Path $json) { $ok = $true; break }
        if (Select-String -Path $err -Pattern 'has no notes' -Quiet) { break }
    }
    if (-not $ok) {
        $why = (Get-Content $err | Select-Object -Last 2) -join ' / '
        "$c`tFAILED`t$why"; continue
    }
    $j = Get-Content $json -Raw | ConvertFrom-Json
    "$c`t$($j.paths.Count)`t" +
        (($j.paths | ForEach-Object { "$($_.pathstring)=$($_.total)" }) -join ' | ')
}
$lines | Set-Content -Encoding utf8 $Out
"$($charts.Count) charts written to $Out"
```

Save this as `$base\s2-compare.ps1`. A FAILED line ends with "(read from a snapshot at ...)", which names a temp file that changes every run, so that clause is dropped before comparing:

```powershell
# Prints the chart of every line that differs between two score files, once
# each. Nothing printed means the two files agree.
#   pwsh -File s2-compare.ps1 -Before <file> -After <file>
param([Parameter(Mandatory = $true)][string]$Before,
      [Parameter(Mandatory = $true)][string]$After)
function Read-Scores([string]$f) {
    # A FAILED line keeps only its reason. The echoed chart path after it
    # repeats column one, and older brief files spelled non-ASCII names in a
    # different code page (black midi - Sugar／Tzu), so it is dropped.
    Get-Content -Encoding utf8 $f | ForEach-Object {
        ($_ -replace ' / \(read from a snapshot at [^)]*\)', '') -replace "(`tFAILED`t[^:`t]*): .*$", '$1'
    }
}
Compare-Object @(Read-Scores $Before) @(Read-Scores $After) |
    ForEach-Object { ($_.InputObject -split "`t")[0] } | Sort-Object -Unique
```

Record the baseline once, one run at a time, never in parallel (one CPU benchmark at a time):

```powershell
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe "$baseBin\hydra_replay.exe" -Diff $d -Out "$base\base-$d-2x1.txt" -Work "$base\work"
}
foreach ($d in 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe "$baseBin\hydra_replay.exe" -Diff $d -Bass2x 0 -Out "$base\base-$d-2x0.txt" -Work "$base\work"
}
# Sanity check: the brief's own 1abd188 dumps agree with this baseline.
foreach ($d in 'expert', 'hard') {
    pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$PF\out_corpus_base_$d.txt" -After "$base\base-$d-2x1.txt"
}
```

Expected: seven files of 97 lines each, and the sanity check prints nothing. If it prints a chart, stop: the baseline doesn't match what the briefs measured against.

A lane's corpus check is then always the same shape. With `$lane` the lane's worktree and `$w = "$sp\s2-tN"`:

```powershell
$exe = "$lane\build-cpp\Release\hydra_replay.exe"
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -Out "$w\tN-$d-2x1.txt" -Work "$w\work"
    "== $d"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$base\base-$d-2x1.txt" -After "$w\tN-$d-2x1.txt"
}
```

Build `hydra_replay` and `hydra_batch` in the lane first (`.\build_cpp.ps1 -Target hydra_replay; .\build_cpp.ps1 -Target hydra_batch`).

## Expected score changes, for the integrator

Everything below was measured on prototype builds against scratch databases, each re-checked by a second agent (briefs `disco`, `kick2x`, `phrasefill`, `small`, and the Fill B measurement, with their verifier corrections applied). Defaults unless stated: Pro Drums on, 2x Bass on, cap 4, 10 ms, depth scores 4.

**Test corpus (97 charts), per difficulty.** After the whole wave, only T1 moves the corpus. Every other task's corpus column is 0 changes at every difficulty, including T4's Fill B half (0 path-list changes at any difficulty).

| Difficulty | Charts whose best score changes | Charts whose best path changes | The charts (base → s2-int) |
|---|---|---|---|
| Expert | 0 | 0 | none |
| Hard | 3 (1 up, 2 down) | 0 | fanclubwallet - Band Like That 287,770 → 297,790; Mike Orlando - Tapped Out 587,470 → 585,370; sungazer - All These People 320,760 → 318,780 |
| Medium | 3 (1 up, 2 down) | 1 | Mike Orlando - Tapped Out 273,255 → 268,615 (new path); fanclubwallet - Band Like That 135,775 → 139,075; sungazer - All These People 138,830 → 137,870 |
| Easy | 4 (1 up, 3 down) | 1 | Mike Orlando - Tapped Out 283,210 → 277,630; fanclubwallet - Band Like That 127,135 → 130,375; sungazer - All These People 114,670 → 113,610 (new path); Venetian Snares - Epidermis 2,210,275 → 2,209,855 |

30 corpus charts have no notes below Expert, so their lower-difficulty lines read FAILED ("has no notes") on both sides and compare equal. On the corpus, most of T1's changes go down; the "mostly up" pattern below is a library result, and at library Easy it is close.

**Library, per task.** These are the per-task measurements. The integrator re-checks the spot charts listed after the table.

| Task | Expert | Hard | Medium | Easy | Notes |
|---|---|---|---|---|---|
| T0 | 0 | 0 | 0 | 0 | refactor only |
| T1 | 0 (Expert keeps digit 3, so it cannot move) | 641 of 658 change (565 up, 76 down), 207 paths | 645 of 661 (525 up, 120 down), 190 paths | 647 of 661 (354 up, 293 down), 161 paths | Pro Drums only; nothing with Pro Drums off |
| T2 | 0 | 8 `.chart` charts up (median +20,107.5, max +39,615); 3 pitch-83 `.mid` charts gain a Hard chart | 1 chart (Cygnus Terminal) gains a one-note chart worth 50 | 0 | only with 2x on; 0 changes with 2x off |
| T3 | 20 down (2,160 to 19,180, median 9,420), 18 paths | 1 down (4,540) | 0 | 0 | `.chart` only |
| T4, the +1 | 2 up | 2 up | 2 up | 2 up | both Thrice (charter Hoph2o) |
| T4, Fill B | 0 scores; best path 3 of 28 | 0 scores; best path 2 of 28 | 0 scores; best path 5 of 28 | 0 scores; best path 5 of 33 | fill counts only, e.g. Dirge Within - Forever the Martyr `1+ 0 2` → `2+ 0 2` |
| T5 | 1 down (13,150) | 0 | 0 | 0 | Programmed for Battle has no notes below Expert |
| T6, T7, T8 | 0 | 0 | 0 | 0 | metadata, the leaderboard page and tools only |
| T9 | 0 | 0 | 0 | 0 | stamps: every saved row is redone once |
| T10 | counted in T10 Step 7 | | | | expected 0 at cap 4; best scores can only rise; path strings can gain + or − |
| T11 | 0 | 0 | 0 | 0 | pins today's rule |

T2's Dynamics side effect, from the kick2x verifier: below Expert, Dynamics Totals now include the 2x row whenever the box is on. In the library that is 11 charts at Hard (8 `.chart` files plus 3 `.mid` files) and 1 at Medium (Cygnus Terminal). In the corpus it is 0.

**Library spot checks** (chart, difficulty, 2x setting, before at 1abd188, after on `claude/s2-int`, owning task). If a spot chart misses, dump it with the single-task builds to see whether two tasks combine on it before calling it a failure.

| Chart file | Diff | 2x | Before | After | Task |
|---|---|---|---|---|---|
| `C:\Clone Hero\songs\Misc Downloads\Genesis - Driving the Last Spike.sng` | hard | 1 | 614,890 | 690,790 | T1 |
| same | expert | 1 | 1,080,770 | 1,080,770 | T1 control |
| `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\TheCourtofaPorcupine\Incubus\Incubus - Anna Molly (Drum Playthrough Coop3r Drumm3r) (2x Bass Pedal)\notes.mid` | easy | 1 | 624,975 | 581,190 | T1 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\sylvexite\Like Moths To Flames\[2021-FULL] Pure Like Porcelain\Like Moths To Flames - Ameliorate [sylvexite]\notes.chart` | hard | 1 | 299,805 | 339,420 | T2 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\Koloxid\Krallice\Energy Chasms\notes.mid` | hard | 1 | "chart has no notes" | 152,950 | T2 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\Advanst\Packs\Advanst's Drum Hero\Tier 5\Muse - Feeling Good\notes.chart` | expert | 1 | 347,935 | 328,755 | T3 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\Advanst\Packs\Advanst's Drum Hero\Bonus\Big Giant Circles feat. C418 - BGC418\notes.chart` | hard | 1 | 207,255 | 202,715 | T3 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\HopH2O\Thrice\Thrice - The Illusion Of Safety\Thrice - So Strange I Remember You\notes.mid` | expert / hard / medium / easy | 1 | 401,340 / 372,065 / 238,760 / 181,775 | 402,060 / 372,525 / 239,220 / 182,235 | T4 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\HopH2O\Thrice\Thrice - The Illusion Of Safety\Thrice - The Illusion Of Safety (Album)\notes.mid` | expert | 1 | 4,386,960 | 4,387,680 | T4 |
| `C:\Clone Hero\songs\synchotic\Sync Charts\Drummer's Monthly Drive\RAT KING\Last Chance to Reason\Level 2\5 - Programmed for Battle\notes.mid` | expert | 1 | 620,730 | 607,580 | T5 |

The full candidate lists, if the integrator wants more than spot checks, are the briefs' own files: `$sp\s2-disco\library_pairs.tsv`, `$sp\s2-kick2x\lib_low_list.json`, `$PF\lib_cand_<diff>.txt` and `$FB\libB_<diff>.txt`.

**Visible changes the user already approved** (D19-D25, D27, D30, D31). Each task's changes, one line each:

T1: lower-difficulty Pro Drums scores, paths, Preview lanes and Dynamics red/yellow rows change on about 640 library charts each.

T2: the 2x Bass box is enabled at every difficulty, and its tooltip ("Include the chart's 2x kicks, like Clone Hero's Double Kick.") now shows at every difficulty, Expert included. Lower-difficulty record keys gain "2x Bass", and the batch settings summary's difficulty label reads, for example, "Hard · 2x Bass" with the box on (T2 Step 8 updates its pin). The Dynamics tab's phantom 2x rows are gone below Expert. The UserGuide's two 2x lines change as Q1 says.

T3: 21 lower best scores, and Preview phrase notes move.

T4: 2 higher scores, and a few best paths whose fill counts change because a fill that used to be forgotten now counts. The UserGuide's `fill_land_slop_beats` line says the +1 tick applies and that it covers only fills written in the chart.

T5: one lower score, and the "from <m:ss> on (N earlier markings ignored by Clone Hero)" line on 3 charts, singular for one marking.

T6: 56 blank Charter cells read `<unknown charter>`, and 3 Preview section names are corrected.

T7: the "other speed" status on the leaderboard page, with an "Other speed" filter option, an "Other speed" stat row, one Status help sentence ("Other speed: played at a speed other than 100%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared."), a dimmed Points-left cell on off-speed rows, the ", N at other speeds" clause when there are any, and one UserGuide sentence.

The wording in Q1-Q5 and the review's wording gaps were approved in D31 (see "Answered questions").

### The integrator's corpus check

Run the lane check from "Before the wave" with `$lane` set to `.claude\worktrees\s2-int` and `$w` to the integrator's scratch folder, at all four difficulties with 2x on, then again at `hard`, `medium` and `easy` with `-Bass2x 0` against `$base\base-$d-2x0.txt`.

Expected with 2x on: the `expert` block prints nothing. The `hard`, `medium` and `easy` blocks print exactly the T1 corpus charts in the first table (three, three and four chart files), and the totals on those lines match the table. Expected with 2x off: no chart outside T1's list, since no corpus chart authors a lower-difficulty 2x kick. T1's charts are expected to change there too, but their 2x-off totals were not measured, so only the chart names are checked. For the library spot checks, put the spot-check paths in a text file and run the same script with `-List`, once with `$baseBin` and once with the integration build.

### The full proof on `claude/s2-int`

```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-int
.\build_cpp.ps1 -Target hydra_tests;  .\build-cpp\Release\hydra_tests.exe
.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4
.\build_cpp.ps1 -Target hydra_replay; .\build-cpp\Release\hydra_replay.exe selfcheck
python -m pytest tools/ch_probe/tests -q
```

Expected: 0 failed in `hydra_tests`; `hydra_uitest` passes (the uitest pins T2 updates included); `selfcheck` prints no FAIL line; the probe suite passes; the corpus check above matches.

## What is NOT in this plan

**Clone Hero's exact disco on/off rule (option A+).** Clone Hero turns flip on exactly when the text after `mix N ` has a 7th character `d`. That also reads `drumsNeasy`, `drumsNeasynokick` and `drumsd` differently from Hydra (it reads `drumsd` as off). It would move 5 chart-and-difficulty pairs, one at Expert. D19 says no. The spaced `.chart` form `E [mix 3 drums0d]` (0 library charts) stays unread for the same reason.

**Clone Hero's unbounded one-side fill landing (option B2).** When a fill has a chord on only one side of its end, Clone Hero's 0x5DE030 takes that chord however far away. D30 says no: Hydra keeps the bound and drops the fill. The Fill B measurement found B1 and B2 give identical results on every chart it dumped. D30's open question (whether Clone Hero carries a search position from one fill to the next) was not traced.

**Any live Clone Hero check.** D26 says no live check now. That leaves generated-fill length with two meter changes between chords (finding 53), whether a beat in 6/8 is a quarter note for the 4-beat fill deadline (314), whether one authored fill turns off generated fills for the whole chart (the open half of 315), and whether Clone Hero trims meta text before comparing the dynamics tag (D24's caveat). T9 writes the first two down as unverified.

**Hit-window plan steps 4-6.** D28 shelves the hit-time field, the edge walk and precision mode. T8 only marks steps 1-3 answered.

**Smaller leftovers the briefs found.** Finding 255 at Expert: a 2x kick and a normal kick on the same tick still keep whichever comes first in the file, because nobody traced how Clone Hero merges them. A `.chart` `S 2` that starts inside an open phrase still replaces it (Clone Hero keeps both; 0 of the 21 phrase candidates have it). A `.chart` modifier on a chord that lacks that colour is skipped like a modifier with no chord at all (T6), but only the second case was traced in Clone Hero. The small brief says Clone Hero maps `.chart` N 34-38 to accent and N 40-44 to ghost, while Hydra maps 34-37 and 40-43, so lane 5 (38 and 44) is dropped; whether that matters for 4-lane drums is unverified. Finding 97 waits on its own plan (step 1's Q3). Finding 345 (the rescan cache has no reader stamp) is unplanned.

**A related engine oddity T10 found, but does not fix.** On charts whose measures are shorter than about 250 ms, a deactivation edge can claim a phrase that the path already collected before reaching that end. The path then gets no normal deactivation there. It is the same fast-measure regime as finding 37, it exists today, and T10 neither causes nor cures it. It needs its own count before anyone decides.

## Prior art

Matching another program's behaviour by reading its code and pinning each rule with a test is characterization testing aimed at a reference implementation ([Characterization test](https://en.wikipedia.org/wiki/Characterization_test)). Moving a version stamp when the data a function produces changes, rather than when the app version changes, is the cache-key pattern ADR 0018 already uses. T10's guard test compares two independent answers to "where does this end go", which is differential testing ([Differential testing](https://en.wikipedia.org/wiki/Differential_testing)).

---

## Tasks

### Task T0: One table says how each difficulty is spelled in a chart file

**Goal:** One table holds each difficulty's kick pitch, 2x-kick pitch and disco digit, and the parser's kick pitch reads from it. Nothing else changes.

**Findings:** groundwork for 11 and 10 (finding 11's owner note: "the parser's single difficulty choice should feed the disco match too").

**Decision:** rulings, T0.

Think of it as the row of a lookup chart pinned above the parser. Today the parser has only one column of that chart (the kick pitch) and the other two answers are scribbled as Expert's values wherever they're used. This task draws the full chart once. The values come from Clone Hero's code: its pitch-to-difficulty map at 0x210D9C0 (58-66 Easy, 70-78 Medium, 82-90 Hard, 94-102 Expert), its 2x-kick test at 0x21555CD (59, 71, 83, 95), and its mix-digit map at 0x210D990 (0 Easy to 3 Expert).

The table lives in `song.cpp` right where `difficulty_base_pitch` is today (line 401). That spot is inside an anonymous namespace (it opens at line 337), and the tests must be able to read the table, so the task closes that namespace around the new public accessor and reopens it. The row type is declared in `song.h` beside `kAllDifficulties`.

**Files.** The row type goes in `src/parse/song.h`, after line 40. The table and its accessor replace `difficulty_base_pitch` in `src/parse/song.cpp`, lines 399-408. The tests go in the new `tests/test_s2_difficulty_table.cpp`, plus one line in `CMakeLists.txt` after line 429 (`tests/test_long_paths.cpp`).

**Acceptance Criteria:**
- [ ] `difficulty_chart_codes(d)` gives 96/95/'3' for Expert, 84/83/'2' for Hard, 72/71/'1' for Medium and 60/59/'0' for Easy.
- [ ] `difficulty_base_pitch` returns the table's kick pitch and has no switch of its own.
- [ ] No chart parses differently: the corpus score files at all four difficulties match the 1abd188 baseline exactly.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="difficulty table*"` → `Status: SUCCESS!`, then the full run.

**Steps:**

- [ ] **Step 1: The failing test.** Create `tests/test_s2_difficulty_table.cpp`:

```cpp
// Step 2, T0: the one table of how each difficulty is spelled in a chart
// file. Every parser rule that depends on the difficulty reads its row.

#include "doctest.h"

#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

TEST_CASE("difficulty table: kick, 2x kick and disco digit for every difficulty") {
    // Clone Hero 1.1: 0x210D9C0 maps pitches 58-66 / 70-78 / 82-90 / 94-102 to
    // Easy / Medium / Hard / Expert; 0x21555CD flags 59, 71, 83 and 95 as
    // DoubleKick; 0x210D990 maps a mix digit 0-3 to Easy..Expert.
    struct Want {
        Difficulty d;
        int kick;
        int kick2x;
        char mix;
    };
    const Want want[] = {{Difficulty::Expert, 96, 95, '3'},
                         {Difficulty::Hard, 84, 83, '2'},
                         {Difficulty::Medium, 72, 71, '1'},
                         {Difficulty::Easy, 60, 59, '0'}};
    REQUIRE(std::size(want) == std::size(kAllDifficulties));
    for (const Want& w : want) {
        const std::string name = difficulty_name(w.d);
        CAPTURE(name);
        const DifficultyChartCodes& c = difficulty_chart_codes(w.d);
        CHECK(c.kick_pitch == w.kick);
        CHECK(c.kick2x_pitch == w.kick2x);
        CHECK(c.mix_digit == w.mix);
        // The 2x kick sits one pitch below the kick at every difficulty.
        CHECK(c.kick2x_pitch == c.kick_pitch - 1);
    }
}

TEST_CASE("difficulty table: the .mid parser reads each difficulty's kick from it") {
    // One kick per difficulty, a beat apart: Expert's 96 at tick 0, Hard's 84
    // at 480, Medium's 72 at 960, Easy's 60 at 1440. A difficulty that read
    // another's kick pitch would load the wrong tick.
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo(),
                                            note_on(96, 100)};
    // Each later kick: a delta of 480 ticks (0x83 0x60), then a note-on.
    for (uint8_t pitch : {uint8_t{84}, uint8_t{72}, uint8_t{60}})
        ev.push_back({0x83, 0x60, 0x90, pitch, 100});
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));

    const std::pair<Difficulty, int64_t> cases[] = {{Difficulty::Expert, 0},
                                                    {Difficulty::Hard, 480},
                                                    {Difficulty::Medium, 960},
                                                    {Difficulty::Easy, 1440}};
    for (const auto& [d, tick] : cases) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const Song song = load_songbytes_mid(mid, true, true, d);
        REQUIRE(song.sequence.size() == 1);
        CHECK(song.sequence[0].timecode.ticks() == tick);
        CHECK(song.sequence[0].chord.at(NoteColor::Kick).has_value());
    }
}
```

Add `    tests/test_s2_difficulty_table.cpp` to the `hydra_tests` list in `CMakeLists.txt`, after line 429 (`    tests/test_long_paths.cpp`).

The second case passes today: it pins the kick column through the parser, so the switch-to-table move can't break it unnoticed. The first case fails to compile until Step 2.

- [ ] **Step 2: Build and watch it fail.** `.\build_cpp.ps1 -Target hydra_tests` stops with "'DifficultyChartCodes': undeclared identifier". That is the expected failure.

- [ ] **Step 3: Declare the row type.** In `src/parse/song.h`, after line 40 (the closing line of `kAllDifficulties`), insert:

```cpp

// How a chart file spells one difficulty's drums. One row per difficulty, held
// in one table in song.cpp; every parser rule that depends on the difficulty
// reads its row instead of keeping its own copy of Expert's values.
struct DifficultyChartCodes {
    // .mid: the kick's pitch. The four pads follow it (kick + 1 is red).
    int kick_pitch;
    // .mid: the 2x kick's pitch, one below the kick. Clone Hero reads 59, 71,
    // 83 and 95, each into its own difficulty (0x2155050 at 0x21555CD).
    int kick2x_pitch;
    // The N in a disco-flip marker `[mix N drums...]`. Clone Hero applies a
    // marker only to the difficulty it names (0x215C750, digit mapped by
    // 0x210D990).
    char mix_digit;
};

// The row for `difficulty`. An out-of-range value reads as Expert, as the
// parsers' old switches did.
const DifficultyChartCodes& difficulty_chart_codes(Difficulty difficulty);
```

- [ ] **Step 4: The table, and the kick pitch reads it.** In `src/parse/song.cpp`, replace lines 399-408:

```cpp
// All four difficulties share the one "PART DRUMS" track; each owns a block of
// five pitches starting here (kick, then the four pads).
int difficulty_base_pitch(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Hard: return 84;
        case Difficulty::Medium: return 72;
        case Difficulty::Easy: return 60;
        default: return 96;
    }
}
```

with:

```cpp
}  // namespace

// The one table of how each difficulty is spelled in a chart file, in
// Difficulty enum order. All four difficulties share the one "PART DRUMS"
// track; each owns a block of five pitches starting at its kick (kick, then
// the four pads), and its 2x kick sits one below the kick.
namespace {
constexpr DifficultyChartCodes kDifficultyChartCodes[] = {
    {96, 95, '3'},  // Expert
    {84, 83, '2'},  // Hard
    {72, 71, '1'},  // Medium
    {60, 59, '0'},  // Easy
};
static_assert(std::size(kDifficultyChartCodes) == std::size(kAllDifficulties));
}  // namespace

const DifficultyChartCodes& difficulty_chart_codes(Difficulty difficulty) {
    const auto i = static_cast<size_t>(difficulty);
    return kDifficultyChartCodes[i < std::size(kDifficultyChartCodes) ? i : 0];
}

namespace {

int difficulty_base_pitch(Difficulty difficulty) {
    return difficulty_chart_codes(difficulty).kick_pitch;
}
```

The anonymous namespace that opened at line 337 is closed before the public accessor and reopened after it, so everything below (`is_handled_note`, `MidiParser`) stays private as today.

- [ ] **Step 5: Run the new cases, then the full suite.** `.\build-cpp\Release\hydra_tests.exe -tc="difficulty table*"`, then `.\build-cpp\Release\hydra_tests.exe`. Both end `Status: SUCCESS!`.

- [ ] **Step 6: The corpus check.** Build everything (`.\build_cpp.ps1`, which builds `hydra_replay` and `hydra_batch` too). Run the lane corpus check from "Before the wave" at all four difficulties, with `$lane` the s2-t0 worktree, `$w = "$sp\s2-t0"` and files `t0-$d-2x1.txt`.

Expected: no output under any heading. T0 is a pure move.

- [ ] **Step 7: Commit** `src/parse/song.h src/parse/song.cpp tests/test_s2_difficulty_table.cpp CMakeLists.txt` by name: "One table spells each difficulty's kick, 2x kick and disco digit".

**Done when:**
- [ ] Both new cases pass, and the full `hydra_tests` suite ends `Status: SUCCESS!`.
- [ ] T0's four 2x-on score files equal the baseline line for line (Step 6 prints nothing).
- [ ] `git diff 1abd188 --stat` lists only the four files above.

**Stamp needs for T9:** none. No chart parses differently.

---

### Task T1: Each difficulty reads only its own disco markers

**Goal:** A `[mix N drums...]` marker opens or closes a disco section only at the difficulty it names, in both `.mid` and `.chart`, and the "flip only under Pro Drums" check is written once.

**Findings:** 11, 250.

**Decision:** D19: "Disco flip is per difficulty: each difficulty reads only its own `[mix N drums...]` markers, in both formats, as Clone Hero does (0x215C750, 0x213D076, 0x2155050). `drums0dnoflip` stays off (Hydra does not copy Clone Hero's bug). Clone Hero's one-letter on/off rule (A+) is not adopted. The 'flip only with Pro Drums' gate moves to one place."

**How it works today.** Disco flip swaps the red and yellow lanes inside a marked section, and the moved red note becomes a cymbal worth 15 points. A chart marks it with a text event like `[mix 3 drums0d]`, where the digit names the difficulty. `disco_head` (song.cpp line 108) accepts only the digit `3`. In `.mid`, all four difficulties share one track, so every difficulty picks up Expert's markers. In `.chart`, each difficulty has its own section, but the `3` rule still applies inside it, so `[HardDrums]` obeys a stray copied `mix_3` marker and ignores its real `mix_2` one. Separately, both parsers pass `mode_pro_ && flag_disco_` into `emit_chord_timestamp` (lines 664-665 and 1168-1169), so the Pro Drums rule is written twice.

**What changes.** The three marker matchers take the parsed difficulty's digit from T0's table, instead of the fixed `3`. A marker that names another difficulty is simply not a marker at this difficulty. For `.mid`, `MidiParser` keeps the digit beside its kick pitch. For `.chart`, lines are classified while the file is read, so `ChartParser::parse` looks up the digit first and hands it to `load_sections`. A `.chart` marker for another difficulty then falls through to the plain-text-event branch, which nothing in a drum section reads. `emit_chord_timestamp` takes the Pro Drums setting and the disco state as two flags and applies the gate itself.

Two things stay as they are, on purpose. `dnoflip` still reads as flip off; Clone Hero turns it on (its classifier at 0x215CD50 looks only at the 7th character), and that bug is not copied. Clone Hero's 7th-character rule as a whole (option A+) is not adopted either, so `drums0easy`-style suffixes keep today's reading.

Expert cannot change: its digit is `3`, exactly as today. The measurements are the check on that, not the proof (0 changes on 97 corpus and 676 library charts).

**Files.** All the code change is in `src/parse/song.cpp`, and each step below quotes the lines it replaces. The three disco matchers (`disco_head`, `is_disco_on_marker`, `is_disco_off_marker`) take the difficulty's digit. `emit_chord_timestamp` takes the Pro Drums gate. On the `.mid` side, `MidiParser` gains one member for the digit, and its `optype`, `push_timestamp` and `parse` pass it along. On the `.chart` side, `ChartDataEntry`, `load_sections` and `ChartParser` carry the digit from the section header to the matchers. The tests go in the new `tests/test_s2_disco.cpp`, plus one line in the `hydra_tests` list in `CMakeLists.txt`.

**Acceptance Criteria:**
- [ ] In `.mid`, `[mix N drums0d]` flips red and yellow only at the difficulty whose digit is N.
- [ ] In `.chart`, a difficulty section obeys only `mix_N` markers with its own N; `[HardDrums]` ignores `mix_3` and obeys `mix_2`.
- [ ] With Pro Drums off nothing flips, in either format, at any difficulty.
- [ ] `[mix N drums0dnoflip]` still closes a disco section.
- [ ] Band Like That at Hard, Medium and Easy shows the same red and yellow counts with Pro Drums on as off; Expert still flips.
- [ ] Corpus: Expert unchanged; at Hard, Medium and Easy exactly the charts the brief measured change, to the brief's numbers.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*disco*,*Band Like That*"` → `Status: SUCCESS!`, then the full run.

**Steps:**

- [ ] **Step 1: The failing tests.** Create `tests/test_s2_disco.cpp`:

```cpp
// Step 2, T1 (D19, findings 11 and 250): each difficulty reads only its own
// disco-flip markers, in both formats, and the Pro Drums gate lives in one
// place.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "midi_util.h"
#include "parse/song.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// Each difficulty's red-pad pitch, mix digit and .chart section, as literals.
struct Diff {
    Difficulty d;
    uint8_t red;
    char digit;
    const char* section;
};
const Diff kDiffs[] = {{Difficulty::Expert, 97, '3', "ExpertDrums"},
                       {Difficulty::Hard, 85, '2', "HardDrums"},
                       {Difficulty::Medium, 73, '1', "MediumDrums"},
                       {Difficulty::Easy, 61, '0', "EasyDrums"}};

// A .mid whose PART DRUMS holds `markers` at tick 0, then a red note for
// every difficulty on that same tick.
std::vector<uint8_t> mid_with(const std::vector<std::string>& markers) {
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo()};
    for (const std::string& m : markers) ev.push_back(text_event(m));
    for (const Diff& x : kDiffs) ev.push_back(note_on(x.red, 100));
    ev.push_back(end_of_track());
    return smf(concat(ev));
}

// A .chart with one difficulty section: `markers` at tick 0, then a red note.
std::vector<uint8_t> chart_with(const char* section, const std::vector<std::string>& markers) {
    std::string s = "[Song]\n{\n  Resolution = 192\n}\n"
                    "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
                    "[" + std::string(section) + "]\n{\n";
    for (const std::string& m : markers) s += "  0 = E " + m + "\n";
    s += "  0 = N 1 0\n}\n";
    return std::vector<uint8_t>(s.begin(), s.end());
}

// The one chord's red note was flipped: a flipped red reads as a yellow
// cymbal (Chord::apply_disco_flip).
bool flipped(const Song& song) {
    REQUIRE(song.sequence.size() == 1);
    return song.sequence[0].chord.at(NoteColor::Yellow).has_value();
}

std::string mid_on(char digit) { return std::string("[mix ") + digit + " drums0d]"; }
std::string chart_on(char digit) { return std::string("mix_") + digit + "_drums0d"; }

}  // namespace

TEST_CASE(".mid: a disco marker flips only the difficulty it names") {
    for (const Diff& marked : kDiffs) {
        const std::vector<uint8_t> mid = mid_with({mid_on(marked.digit)});
        for (const Diff& parsed : kDiffs) {
            const std::string at = std::string(difficulty_name(parsed.d)) + ", marker " +
                                   mid_on(marked.digit);
            CAPTURE(at);
            CHECK(flipped(load_songbytes_mid(mid, true, true, parsed.d)) ==
                  (parsed.d == marked.d));
        }
    }
}

TEST_CASE(".chart: a section obeys only the disco marker naming its own difficulty") {
    for (const Diff& parsed : kDiffs) {
        for (const Diff& marked : kDiffs) {
            const std::string at = std::string(parsed.section) + ", marker " +
                                   chart_on(marked.digit);
            CAPTURE(at);
            const Song song = load_songbytes_chart(
                chart_with(parsed.section, {chart_on(marked.digit)}), true, true, parsed.d);
            CHECK(flipped(song) == (parsed.d == marked.d));
        }
    }
}

TEST_CASE("disco flip needs Pro Drums, in both formats and at every difficulty") {
    for (const Diff& x : kDiffs) {
        const std::string name = difficulty_name(x.d);
        CAPTURE(name);
        const std::vector<uint8_t> mid = mid_with({mid_on(x.digit)});
        const std::vector<uint8_t> chart = chart_with(x.section, {chart_on(x.digit)});
        CHECK(flipped(load_songbytes_mid(mid, true, true, x.d)));
        CHECK(flipped(load_songbytes_chart(chart, true, true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_mid(mid, false, true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_chart(chart, false, true, x.d)));
    }
}

TEST_CASE("disco: dnoflip still closes the section (Hydra's rule, not Clone Hero's)") {
    // Both markers on one tick run in file order: on, then off.
    for (const Diff& x : kDiffs) {
        const std::string name = difficulty_name(x.d);
        CAPTURE(name);
        const std::string mid_off = std::string("[mix ") + x.digit + " drums0dnoflip]";
        const std::string chart_off = std::string("mix_") + x.digit + "_drums0dnoflip";
        CHECK_FALSE(flipped(load_songbytes_mid(mid_with({mid_on(x.digit), mid_off}), true,
                                               true, x.d)));
        CHECK_FALSE(flipped(load_songbytes_chart(
            chart_with(x.section, {chart_on(x.digit), chart_off}), true, true, x.d)));
    }
}

TEST_CASE("disco: Band Like That keeps its lower difficulties' hi-hat on yellow") {
    // This chart marks disco only for Expert ([mix 3 drums0d], 13 sections);
    // Hard, Medium and Easy carry only `[mix N drums0]` at tick 960. So below
    // Expert, Pro Drums must not move a single red or yellow note: the counts
    // equal a parse with Pro Drums off, where no flip ever applies. Before D19,
    // Hard followed Expert's sections and swapped 111 notes each way (finding
    // 11: 151 red and 40 yellow inside the spans, where Hard has 40 red and 151
    // yellow as authored).
    using app::DynamicsRow;
    const std::string path = std::string(HYDRA_INPUT_DIR) +
                             "/common/IB24/T2/fanclubwallet - Band Like That/notes.mid";
    for (Difficulty d : {Difficulty::Hard, Difficulty::Medium, Difficulty::Easy}) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const app::DynamicsBreakdown pro = app::count_dynamics(load_songpath(path, true, true, d));
        const app::DynamicsBreakdown plain =
            app::count_dynamics(load_songpath(path, false, true, d));
        CHECK(pro.row(DynamicsRow::RedSnare).all() == plain.row(DynamicsRow::RedSnare).all());
        CHECK(pro.row(DynamicsRow::YellowCymbal).all() + pro.row(DynamicsRow::YellowTom).all() ==
              plain.row(DynamicsRow::YellowTom).all());
    }
    // Expert still flips inside its own sections.
    const app::DynamicsBreakdown pro =
        app::count_dynamics(load_songpath(path, true, true, Difficulty::Expert));
    const app::DynamicsBreakdown plain =
        app::count_dynamics(load_songpath(path, false, true, Difficulty::Expert));
    CHECK(pro.row(DynamicsRow::RedSnare).all() != plain.row(DynamicsRow::RedSnare).all());
}
```

Add `    tests/test_s2_disco.cpp` to the `hydra_tests` list in `CMakeLists.txt`, after `tests/test_s2_difficulty_table.cpp`.

- [ ] **Step 2: Build and watch them fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*disco*"`. Expected today: the `.mid` case fails for Hard, Medium and Easy (their markers are ignored, and Expert's marker flips them); the `.chart` case fails where a section meets `mix_3` or its own digit below Expert; the Pro Drums case fails its two "flips with Pro Drums" checks below Expert; Band Like That fails at Hard. The dnoflip case passes already (it only pins that nothing regresses).

- [ ] **Step 3: The matchers take the difficulty's digit.** In `src/parse/song.cpp`, replace lines 107-111:

```cpp
// `mix.3.drums`: the 11-byte head both disco markers share.
bool disco_head(std::string_view s) {
    return s.size() >= 11 && s.substr(0, 3) == "mix" && regex_dot(s[3]) && s[4] == '3' &&
           regex_dot(s[5]) && s.substr(6, 5) == "drums";
}
```

with:

```cpp
// `mix.N.drums`: the 11-byte head both disco markers share, where N is the
// parsed difficulty's digit (difficulty_chart_codes). A marker naming another
// difficulty is not a marker here: Clone Hero applies each marker only to the
// difficulty it names, in both formats (D19; 0x215C750, 0x213D076, 0x2155050).
bool disco_head(std::string_view s, char mix_digit) {
    return s.size() >= 11 && s.substr(0, 3) == "mix" && regex_dot(s[3]) &&
           s[4] == mix_digit && regex_dot(s[5]) && s.substr(6, 5) == "drums";
}
```

Replace lines 118-134:

```cpp
// \[?mix.3.drums\d?d\]?
bool is_disco_on_marker(std::string_view s) {
    s = peel_brackets(s);
    if (!disco_head(s)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest == "d";
}

// \[?mix.3.drums\d?(dnoflip)?\]?
bool is_disco_off_marker(std::string_view s) {
    s = peel_brackets(s);
    if (!disco_head(s)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest.empty() || rest == "dnoflip";
}
```

with:

```cpp
// \[?mix.N.drums\d?d\]?, where N is `mix_digit`
bool is_disco_on_marker(std::string_view s, char mix_digit) {
    s = peel_brackets(s);
    if (!disco_head(s, mix_digit)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest == "d";
}

// \[?mix.N.drums\d?(dnoflip)?\]?, where N is `mix_digit`. "dnoflip" reads as
// flip off. Clone Hero turns flip on for it (its classifier at 0x215CD50 looks
// only at the 7th character); Hydra keeps it off on purpose (D19).
bool is_disco_off_marker(std::string_view s, char mix_digit) {
    s = peel_brackets(s);
    if (!disco_head(s, mix_digit)) return false;
    std::string_view rest = s.substr(11);
    if (!rest.empty() && regex_digit(rest.front())) rest.remove_prefix(1);
    return rest.empty() || rest == "dnoflip";
}
```

- [ ] **Step 4: The Pro Drums gate moves into `emit_chord_timestamp`.** Replace lines 191-204:

```cpp
// Emit the buffered chord as a sequence timestamp, shared by both parsers.
// apply_flam is true only on the .mid path: MIDI charts carry a flam marker
// that converts the chord, while the .chart format has no such marker, so
// ChartParser always passes false (existing behavior, now explicit).
void emit_chord_timestamp(Song& song, Chord& chord, int64_t tick,
                          bool apply_flam, bool apply_disco, bool solo) {
    if (apply_flam) chord.apply_flam_conversion();
    if (apply_disco) chord.apply_disco_flip();
```

(the rest of the function, lines 199-204, is unchanged) with:

```cpp
// Emit the buffered chord as a sequence timestamp, shared by both parsers.
// apply_flam is true only on the .mid path: MIDI charts carry a flam marker
// that converts the chord, while the .chart format has no such marker, so
// ChartParser always passes false (existing behavior, now explicit).
// A disco section swaps red and yellow only under Pro Drums. That rule lives
// here, once, for both parsers (finding 250): `pro` is the Pro Drums setting
// and `in_disco` says whether this difficulty's disco section is open.
void emit_chord_timestamp(Song& song, Chord& chord, int64_t tick, bool apply_flam,
                          bool pro, bool in_disco, bool solo) {
    if (apply_flam) chord.apply_flam_conversion();
    if (pro && in_disco) chord.apply_disco_flip();
```

- [ ] **Step 5: `.mid` reads its own digit.** After line 478 (`    int base_ = 96;`), add:

```cpp
    // The parsed difficulty's disco digit: only `[mix N drums...]` markers
    // with this N open or close a disco section here. parse() sets it from
    // difficulty_chart_codes.
    char mix_digit_ = 0;
```

Replace lines 576-577:

```cpp
        if (is_disco_on_marker(t)) return mop_flag(MAct::Disco, true);
        if (is_disco_off_marker(t)) return mop_flag(MAct::Disco, false);
```

with:

```cpp
        if (is_disco_on_marker(t, mix_digit_)) return mop_flag(MAct::Disco, true);
        if (is_disco_off_marker(t, mix_digit_)) return mop_flag(MAct::Disco, false);
```

Replace lines 663-665:

```cpp
    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, flag_flam_,
                             mode_pro_ && flag_disco_, flag_solo_);
```

with:

```cpp
    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, flag_flam_, mode_pro_, flag_disco_,
                             flag_solo_);
```

After line 677 (`    base_ = difficulty_base_pitch(difficulty);`), add:

```cpp
    mix_digit_ = difficulty_chart_codes(difficulty).mix_digit;
```

- [ ] **Step 6: `.chart` reads its own digit.** Replace lines 768-769:

```cpp
    // Both sides arrive already trimmed.
    ChartDataEntry(std::string_view keystr, std::string_view valuestr);
```

with:

```cpp
    // Both sides arrive already trimmed. `mix_digit` is the parsed
    // difficulty's disco digit: a disco marker naming another difficulty is
    // read as a plain text event, which no drum section uses.
    ChartDataEntry(std::string_view keystr, std::string_view valuestr, char mix_digit);
```

Replace line 802:

```cpp
ChartDataEntry::ChartDataEntry(std::string_view keystr, std::string_view valuestr) {
```

with:

```cpp
ChartDataEntry::ChartDataEntry(std::string_view keystr, std::string_view valuestr,
                               char mix_digit) {
```

Replace lines 835-838:

```cpp
    } else if (t0 == "E" && t.count == 2 && is_disco_off_marker(t.w[1])) {
        discoflip_disable = true;
    } else if (t0 == "E" && t.count == 2 && is_disco_on_marker(t.w[1])) {
        discoflip_enable = true;
```

with:

```cpp
    } else if (t0 == "E" && t.count == 2 && is_disco_off_marker(t.w[1], mix_digit)) {
        discoflip_disable = true;
    } else if (t0 == "E" && t.count == 2 && is_disco_on_marker(t.w[1], mix_digit)) {
        discoflip_enable = true;
```

Replace line 932 (`    void load_sections(const std::vector<uint8_t>& data);`) with `    void load_sections(const std::vector<uint8_t>& data, char mix_digit);`, line 999 (`void ChartParser::load_sections(const std::vector<uint8_t>& data) {`) with `void ChartParser::load_sections(const std::vector<uint8_t>& data, char mix_digit) {`, and line 1030 (`                wip->add(ChartDataEntry(trim_view(lhs), trim_view(rhs)));`) with `                wip->add(ChartDataEntry(trim_view(lhs), trim_view(rhs), mix_digit));`.

Replace lines 1167-1169:

```cpp
    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, /*apply_flam=*/false,
                             mode_pro_ && flag_disco_, flag_solo_);
```

with:

```cpp
    if (chord_.count())
        emit_chord_timestamp(*song_, chord_, tick, /*apply_flam=*/false, mode_pro_,
                             flag_disco_, flag_solo_);
```

Replace line 1177 (`    load_sections(data);`) with:

```cpp
    // A disco marker counts only in the difficulty it names, so the reader
    // needs this difficulty's digit before it classifies any line.
    load_sections(data, difficulty_chart_codes(difficulty).mix_digit);
```

- [ ] **Step 7: Run the new cases, the old oracle cases, then the full suite.** `.\build-cpp\Release\hydra_tests.exe -tc="*disco*,*Band Like That*"`. The two old cases in `tests/test_song.cpp`, ".chart: disco markers match the regexes they replaced" and ".mid: disco and dynamics markers match the regexes they replaced", run at Expert with `mix_3` markers, so they pass unchanged. Then `.\build-cpp\Release\hydra_tests.exe` → `Status: SUCCESS!`. No existing test needs editing.

- [ ] **Step 8: The corpus scores.** Build everything (`.\build_cpp.ps1`), then run the corpus check from "Before the wave" with this lane's build at all four difficulties, 2x on:

```powershell
$w = "$sp\s2-t1"
$exe = "C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t1\build-cpp\Release\hydra_replay.exe"
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -Out "$w\t1-$d-2x1.txt" -Work "$w\work"
    "== $d"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$base\base-$d-2x1.txt" -After "$w\t1-$d-2x1.txt"
}
```

Expected: Expert prints no chart. Hard prints exactly `fanclubwallet - Band Like That`, `Mike Orlando - Tapped Out` and `sungazer - All These People`. Medium prints the same three. Easy prints those three plus `Venetian Snares - Epidermis`. Every other chart's disco spans are identical before and after (the brief's scan found no other corpus chart whose spans differ), so its whole line must match.

Then check the changed lines carry the brief's best paths, from the prototype dumps the verifier re-ran:

| Chart | Difficulty | Before (path=score) | After (path=score) |
|---|---|---|---|
| Band Like That | Hard | 1 1 0=287770 | 1 1 0=297790 |
| Band Like That | Medium | 1 4=135775 | 1 4=139075 |
| Band Like That | Easy | 1 1 0=127135 | 1 1 0=130375 |
| Tapped Out | Hard | 1 0 E2- 4 2 0=587470 | 1 0 E2- 4 2 0=585370 |
| Tapped Out | Medium | 1 0 2- 4 4=273255 | 2 4 3 4=268615 |
| Tapped Out | Easy | 1 0 E2 4 4=283210 | 1 0 E2 4 4=277630 |
| All These People | Hard | 1 1 0- 0+=320760 | 1 1 0- 0+=318780 |
| All These People | Medium | 2 1 1=138830 | 2 1 1=137870 |
| All These People | Easy | 1 0 0+ 0+=114670 | 1 1 0- 0+=113610 |
| Epidermis | Easy | 1 0 3 1- 0 3=2210275 | 1 0 3 1- 0 3=2209855 |

`Select-String -Path "$w\t1-hard-2x1.txt" -SimpleMatch "1 1 0=297790"` (and so on for each row) must find the chart's line. The numbers come from `$sp\s2-disco\corpus_scores.tsv`; a mismatch means the lane differs from the measured prototype, and is a stop-and-report, not a fix-forward.

- [ ] **Step 9: Optional library spot check.** With the base and lane `hydra_replay.exe`, dump Genesis - Driving the Last Spike at Hard (path in `$sp\s2-disco\library_pairs.tsv`): 614,890 before, 690,790 after, path unchanged. At Expert it stays 1,080,770. The verifier reproduced all three.

- [ ] **Step 10: Commit** `src/parse/song.cpp tests/test_s2_disco.cpp CMakeLists.txt` by name: "Disco flip reads each difficulty's own markers; one Pro Drums gate".

**Done when:**
- [ ] The five new cases and the two old oracle cases pass; the full `hydra_tests` suite ends `Status: SUCCESS!`.
- [ ] `hydra_uitest --all --jobs 4` passes (no UI pin reads disco below Expert; this confirms it).
- [ ] The corpus score files match Step 8 exactly: Expert unchanged; Hard and Medium change on 3 charts, Easy on 4, with the ten path=score pairs in the table.
- [ ] `git grep -n "'3'" -- src/parse/song.cpp` finds the digit only in T0's table, and `git grep -n "mode_pro_ && flag_disco_" -- src/parse/song.cpp` finds nothing.

**Stamp needs for T9:** `kDynamicsCountStamp` 1 → 2 (the Dynamics tab's red and yellow rows change below Expert), shared with T2 and T5. The results stamp too: stored Hard, Medium and Easy Pro Drums results change under unchanged keys and an unchanged chart hash. Per D19 and D23 that is step 1 Task 22's release value, not a second bump. T9 also records `dnoflip` as a deliberate difference from Clone Hero (48 library charts, 79 chart-and-difficulty pairs).

**Dependencies:** branches from T0 and reads `difficulty_chart_codes`. Textual neighbours, for the integrator: T2 replaces line 478 with two `MidiParser` members and adds its own line after line 677, beside T1's; keep all of them. T5 changes line 575, the line just above T1's 576-577. T3 changes `ChartParser::push_timestamp` lines 1148-1152 and `op_sp_end` (951-954), near but not on T1's lines. T4 deletes the fill-placement blocks in both `push_timestamp` functions (lines 648-659 and 1154-1163), just above T1's `emit_chord_timestamp` calls; keep T1's calls. T6 replaces ChartParser's section sort and pass 3 of `MidiParser::parse`, away from T1's lines.

---

### Task T2: Each difficulty reads its own 2x kick, the box works everywhere, and the Dynamics tab has one kick total

**Goal:** The parser reads a difficulty's own 2x kick (59 Easy, 71 Medium, 83 Hard, 95 Expert; `.chart` `N 32` in its own section), the 2x Bass box applies at every difficulty, and "All kicks" and Totals count kicks by one rule.

**Findings:** 10, 12, 255.

**Decision:** D20: "Each difficulty reads its own 2x kick: MIDI 59 Easy, 71 Medium, 83 Hard, 95 Expert, and `.chart` N 32 in its own section (Clone Hero 0x21555CD, 0x210D9C0; its only gate is the Double Kick modifier at 0x20D32B7, at every difficulty). The 2x Bass box works at every difficulty, like Clone Hero's Double Kick. The Dynamics tab stops reading Expert's 2x kicks below Expert, so the phantom rows and swallowed kicks go away, and one kick total feeds both 'All kicks' and Totals. ... No key fallback: this ships with step 1's results-stamp bump, which re-analyzes everything anyway."

**How it works today.** Scoring and the Dynamics tab disagree about 2x kicks. `Settings::effective_bass2x` (config.cpp line 131) says 2x only on Expert, and the settings bar greys the box out elsewhere. The parser itself has no difficulty rule for 2x: `is_handled_note` accepts pitch 95 at every difficulty (song.cpp line 417), and `MidiParser::optype` reads it whenever 2x is on (527-529). The Dynamics tab always parses with 2x on (`kDynamicsParseBass2x`), so at Hard it reads Expert's 95s as Hard 2x kicks. Where a 95 shares a tick with a Hard kick, the first in the file wins and the other is dropped silently. On Car Bomb - The Sentinel at Hard, 319 real kicks become "2x kicks" that way. Then the tab counts kicks twice, two ways: the "All kicks" row always adds the 2x row (`kicks_total`), but Totals adds it only when 2x Bass is on (`played_total`).

**What changes.** `is_handled_note` and `MidiParser::optype` read the difficulty's own 2x pitch from T0's table, so 95 is just "another difficulty's pitch" at Hard, Medium and Easy. That one change removes the phantom row and the same-tick swallow below Expert. `.chart` already reads `N 32` only from the section it parses, so it needs no change; a new test pins that. `effective_bass2x` drops its Expert clause, the settings bar stops greying the box out, and the tooltip and UserGuide say what the box does now. `DynamicsBreakdown` gets one `kicks_total(bass2x)` that both "All kicks" and Totals read, on top of one shared `DynamicsCounts` addition.

The "2x kicks: X of Y kick notes" line stays a fact about the chart (question Q2, approved in D31): it asks `kicks_total` for every kick (`bass2x = true`), as it shows today. If it followed the setting, with 2x Bass off it would read "12 of 441 kick notes", where the 12 are not part of the 441.

No record-key fallback. A Hard result filed under "Hard Pro Drums, 1x Bass" stays correct, because that key still parses with no 2x kicks. With the box on (the default), the Hard key now ends "2x Bass", so those rows read as not analyzed until step 1's results-stamp bump re-analyzes them.

**Files.** Each step below quotes the lines it replaces.

The parser change is in `src/parse/song.cpp`. `is_handled_note` and `MidiParser::optype` read the difficulty's own 2x pitch, and `MidiParser` gains one member that `parse` fills from T0's table.

The setting change is in `src/app/config.cpp` and `config.h`, where `effective_bass2x` drops its Expert clause. `src/ui/settings_bar.cpp` stops greying the box out and gets the new tooltip.

The one kick total lives in `src/app/dynamics_breakdown.h` and `.cpp`, and `src/ui/dynamics_tab.cpp` reads it. `docs/UserGuide.md` changes its two 2x lines (41 and 168).

The new tests go in `tests/test_s2_kick2x.cpp`, plus one line in `CMakeLists.txt`. Four existing test files pin the old rule and change where they break: `test_config.cpp`, `test_batch_text.cpp`, `test_dynamics_breakdown.cpp` and `ui/uitest_library.cpp`. One new GUI test goes in `ui/uitest_details.cpp`.

**Acceptance Criteria:**
- [ ] A `.mid` with pitches 59, 71, 83 and 95 gives each difficulty exactly its own 2x kick with 2x on, and none with 2x off.
- [ ] At Hard, a 95 on the same tick as a Hard kick leaves one plain Hard kick (finding 255's swallow is gone below Expert).
- [ ] `.chart` `N 32` counts only in the section being read.
- [ ] `effective_bass2x()` equals `view_bass2x` at every difficulty, and `chartmode_key()` reads "Hard Pro Drums, 2x Bass" with the box on.
- [ ] `kicks_total(false)` leaves the 2x row out, `kicks_total(true)` adds it, and `played_total(b)` equals `pads_total()` plus `kicks_total(b)` for both values.
- [ ] Car Bomb - The Sentinel at Hard counts 1,147 kicks, 0 2x kicks and 2,038 notes with 2x on or off.
- [ ] The 2x Bass box is enabled at Hard, and the Dynamics tab for Pathfinder at Hard reads "2x kicks: 0 of 972 kick notes (0%)".
- [ ] Corpus scores unchanged at every difficulty, with 2x on and off.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*2x kick*,*kick total*,*chartmode_key*,batch text*,dynamics_breakdown*"` → `Status: SUCCESS!`, then the full run, then `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --test difficulty --test dynamics --test dynamics-hard`.

**Steps:**

- [ ] **Step 1: The failing tests.** Create `tests/test_s2_kick2x.cpp`:

```cpp
// Step 2, T2 (D20, findings 10, 12 and 255): each difficulty reads its own 2x
// kick, and the Dynamics tab counts kicks by one rule.

#include "doctest.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "core/model.h"
#include "midi_util.h"
#include "parse/song.h"

#ifndef HYDRA_INPUT_DIR
#error "HYDRA_INPUT_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

TEST_CASE(".mid: each difficulty reads its own 2x kick pitch, as Clone Hero does") {
    // Clone Hero (0x2155050 at 0x21555CD) flags 59, 71, 83 and 95 as
    // DoubleKick, each in its own difficulty: one below that difficulty's kick.
    using namespace testmidi;
    std::vector<std::vector<uint8_t>> ev = {track_name("PART DRUMS"), set_tempo(),
                                            note_on(95, 100)};  // tick 0: Expert
    ev.push_back({0x40, 0x90, 83, 100});                       // tick 64: Hard
    ev.push_back({0x40, 0x90, 71, 100});                       // tick 128: Medium
    ev.push_back({0x40, 0x90, 59, 100});                       // tick 192: Easy
    ev.push_back(end_of_track());
    const std::vector<uint8_t> mid = smf(concat(ev));
    const std::pair<Difficulty, int64_t> cases[] = {{Difficulty::Expert, 0},
                                                    {Difficulty::Hard, 64},
                                                    {Difficulty::Medium, 128},
                                                    {Difficulty::Easy, 192}};
    for (const auto& [d, tick] : cases) {
        const std::string name = difficulty_name(d);
        CAPTURE(name);
        const Song on = load_songbytes_mid(mid, true, true, d);
        REQUIRE(on.sequence.size() == 1);
        CHECK(on.sequence[0].timecode.ticks() == tick);
        CHECK(on.sequence[0].chord.at(NoteColor::Kick)->is2x);
        CHECK(load_songbytes_mid(mid, true, false, d).sequence.empty());
    }
}

TEST_CASE(".mid: Expert's 2x kick never swallows a Hard kick on the same tick") {
    // Finding 255: two kicks on one tick keep whichever comes first in the
    // file. With the 95 first, Hard used to lose its real kick to it.
    using namespace testmidi;
    const std::vector<uint8_t> mid = smf(concat(
        {track_name("PART DRUMS"), set_tempo(), note_on(95, 100), note_on(84, 100),
         end_of_track()}));
    const Song hard = load_songbytes_mid(mid, true, /*bass2x=*/true, Difficulty::Hard);
    REQUIRE(hard.sequence.size() == 1);
    REQUIRE(hard.sequence[0].chord.at(NoteColor::Kick).has_value());
    CHECK_FALSE(hard.sequence[0].chord.at(NoteColor::Kick)->is2x);
}

TEST_CASE(".chart: N 32 is a 2x kick only in the section being read") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n  0 = N 32 0\n}\n"
        "[HardDrums]\n{\n  192 = N 1 0\n  384 = N 32 0\n}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song on = load_songbytes_chart(data, true, true, Difficulty::Hard);
    REQUIRE(on.sequence.size() == 2);  // Expert's tick-0 N 32 is not Hard's
    CHECK(on.sequence[1].timecode.ticks() == 384);
    CHECK(on.sequence[1].chord.at(NoteColor::Kick)->is2x);
    CHECK(load_songbytes_chart(data, true, false, Difficulty::Hard).sequence.size() == 1);
}

TEST_CASE("Dynamics: one kick total follows the 2x Bass setting") {
    app::DynamicsBreakdown bd;
    bd.rows[static_cast<size_t>(app::DynamicsRow::RedSnare)] = {1, 2, 3};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick)] = {0, 1, 40};
    bd.rows[static_cast<size_t>(app::DynamicsRow::Kick2x)] = {0, 0, 12};
    CHECK(bd.kicks_total(false).all() == 41);
    CHECK(bd.kicks_total(true).all() == 53);
    for (bool bass2x : {false, true}) {
        CAPTURE(bass2x);
        CHECK(bd.played_total(bass2x).all() ==
              bd.pads_total().all() + bd.kicks_total(bass2x).all());
        CHECK(bd.played_total(bass2x).accent ==
              bd.pads_total().accent + bd.kicks_total(bass2x).accent);
    }
}

TEST_CASE("Dynamics: Car Bomb - The Sentinel at Hard counts Hard's own kicks") {
    // Finding 10's chart. Hard has 1,147 kicks (pitch 84) and no 2x kicks of
    // its own; Expert's 95 shares a tick with 409 of them. Before D20 the
    // Dynamics parse at Hard showed 828 kicks plus 319 "2x kicks" (the 95 came
    // first on those ticks), and Totals said 1,719 instead of 2,038.
    using app::DynamicsRow;
    const std::string path =
        std::string(HYDRA_INPUT_DIR) + "/common/IB24/T7/Car Bomb - The Sentinel/notes.mid";
    const app::DynamicsBreakdown bd = app::count_dynamics(
        load_songpath(path, /*pro=*/true, app::kDynamicsParseBass2x, Difficulty::Hard));
    CHECK(bd.row(DynamicsRow::Kick).all() == 1147);
    CHECK(bd.row(DynamicsRow::Kick2x).all() == 0);
    CHECK(bd.played_total(true).all() == 2038);
    CHECK(bd.played_total(false).all() == 2038);
}
```

Add `    tests/test_s2_kick2x.cpp` to the `hydra_tests` list in `CMakeLists.txt`, after `tests/test_s2_difficulty_table.cpp`.

- [ ] **Step 2: Build and watch them fail.** The build stops first: `kicks_total(bool)` does not exist yet. Comment out the "one kick total" case for one run to see the parser cases fail: the per-difficulty pitch case fails for Hard, Medium and Easy; the same-tick case fails (`is2x` is true, from the 95); Car Bomb shows 828 and 319. The `.chart` case passes already, since `.chart` was already per section. Restore the commented case.

- [ ] **Step 3: The parser reads the difficulty's own 2x pitch.** In `src/parse/song.cpp`, replace lines 410-426:

```cpp
// `base` is the difficulty's kick pitch. The five note pitches follow it; every
// other pitch here is a marker shared by all four difficulties (95 is the 2x
// kick, which only Expert charts carry). A pitch outside this set belongs to
// another difficulty (or to another instrument) and is dropped.
bool is_handled_note(int note, int base) {
    if (note >= base && note <= base + 4) return true;
    switch (note) {
        case 95:
        case 103:
```

(lines 419-426 are unchanged) with:

```cpp
// `base` is the difficulty's kick pitch and `kick2x` its 2x kick pitch, both
// from difficulty_chart_codes. The five note pitches follow the kick, and the
// 2x kick sits one below it. Every other pitch here is a marker shared by all
// four difficulties. A pitch outside this set belongs to another difficulty
// (or to another instrument) and is dropped, so Expert's 95 is never read
// below Expert: Clone Hero reads each 2x kick only into its own difficulty
// (D20; 0x2155050 at 0x21555CD).
bool is_handled_note(int note, int base, int kick2x) {
    if (note >= base && note <= base + 4) return true;
    if (note == kick2x) return true;
    switch (note) {
        case 103:
```

Replace line 478 (`    int base_ = 96;`) with the two members below. Both start at 0, because `parse()` always sets them from T0's table before any note is read. A default of 96 or 95 would be a second copy of T0's Expert row (the same fix T1 makes for its `mix_digit_`).

```cpp
    int base_ = 0;          // set by parse() from difficulty_chart_codes
    int kick2x_pitch_ = 0;  // set by parse() from difficulty_chart_codes
```

Replace line 502 (`        if (!is_handled_note(note, base_)) return {};`) with:

```cpp
        if (!is_handled_note(note, base_, kick2x_pitch_)) return {};
```

Replace lines 526-529:

```cpp
            switch (note) {
                case 95:
                    if (mode_bass2x_) return mop_note(NoteColor::Kick, vel_dyn, true);
                    return {};
```

with:

```cpp
            // The difficulty's own 2x kick, read only with 2x Bass on.
            if (note == kick2x_pitch_) {
                if (mode_bass2x_) return mop_note(NoteColor::Kick, vel_dyn, true);
                return {};
            }
            switch (note) {
```

After line 677 (`    base_ = difficulty_base_pitch(difficulty);`), add:

```cpp
    kick2x_pitch_ = difficulty_chart_codes(difficulty).kick2x_pitch;
```

`ChartParser` needs nothing: its `case 32` at line 1071 already reads only the section `ChartParser::parse` picked at line 1227.

- [ ] **Step 4: 2x Bass applies at every difficulty.** In `src/app/config.cpp`, replace lines 130-138 (line 139, the `return`, stays):

```cpp
bool Settings::effective_bass2x() const {
    return view_bass2x && difficulty() == Difficulty::Expert;
}

std::string Settings::chartmode_key() const {
    std::string prodrums = view_prodrums ? "Pro Drums" : "Drums";
    // Only Expert can be 2x, so every other difficulty's key ends "1x Bass" —
    // and the four Expert keys are byte-for-byte the ones already in the store.
    std::string bass = effective_bass2x() ? "2x Bass" : "1x Bass";
```

with:

```cpp
// 2x Bass applies at every difficulty, as Clone Hero's Double Kick modifier
// does (D20; its only gate, 0x20D32B7, has no difficulty check). Each
// difficulty has its own 2x kicks; the box decides whether they are read.
bool Settings::effective_bass2x() const { return view_bass2x; }

std::string Settings::chartmode_key() const {
    std::string prodrums = view_prodrums ? "Pro Drums" : "Drums";
    // The four Expert keys are byte-for-byte the ones already in the store.
    // Below Expert a key ends "2x Bass" with the box on since D20; results
    // stored before that are re-analyzed by step 1's results-stamp bump.
    std::string bass = effective_bass2x() ? "2x Bass" : "1x Bass";
```

In `src/app/config.h`, replace lines 128-131:

```cpp
    // 2x Bass with the Expert-only rule applied. A second kick pedal is an
    // Expert charting concept, so the other three difficulties always analyze
    // (and preview, and file their records) as 1x.
    bool effective_bass2x() const;
```

with:

```cpp
    // Whether an analysis, the Preview and a record's key read 2x kicks: the
    // 2x Bass box, at every difficulty (D20). Callers ask here rather than
    // reading view_bass2x, so the rule has one owner.
    bool effective_bass2x() const;
```

- [ ] **Step 5: The settings bar.** In `src/ui/settings_bar.cpp`, replace lines 57-71:

```cpp
    // A second kick pedal only exists in Expert charting, so off Expert the
    // box reads unchecked and is disabled; the stored view_bass2x is left
    // alone, so returning to Expert brings the user's own setting back.
    ImGui::SameLine();
    const bool expert = app.settings.difficulty() == Difficulty::Expert;
    bool bass2x_shown = app.settings.effective_bass2x();
    begin_disabled_checkbox(!expert || locked);
    if (ImGui::Checkbox("2x Bass", &bass2x_shown)) {
        app.settings.view_bass2x = bass2x_shown;
        app.commit_settings();
    }
    end_disabled_checkbox(!expert || locked);
    if (!expert && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                                        ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("2x Bass is an Expert-only charting concept.");
```

with:

```cpp
    // 2x Bass works at every difficulty, like Clone Hero's Double Kick (D20):
    // each difficulty has its own 2x kicks.
    ImGui::SameLine();
    bool bass2x_shown = app.settings.effective_bass2x();
    begin_disabled_checkbox(locked);
    if (ImGui::Checkbox("2x Bass", &bass2x_shown)) {
        app.settings.view_bass2x = bass2x_shown;
        app.commit_settings();
    }
    end_disabled_checkbox(locked);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal |
                             ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Include the chart's 2x kicks, like Clone Hero's Double Kick.");
```

The tooltip wording is question Q1, approved in D31. It now shows at every difficulty, Expert included; today it shows only below Expert.

- [ ] **Step 6: One kick total.** In `src/app/dynamics_breakdown.h`, replace lines 24-28:

```cpp
struct DynamicsCounts {
    int ghost = 0, accent = 0, normal = 0;
    int all() const { return ghost + accent + normal; }
    bool has_dynamics() const { return ghost + accent > 0; }
};
```

with:

```cpp
struct DynamicsCounts {
    int ghost = 0, accent = 0, normal = 0;
    int all() const { return ghost + accent + normal; }
    bool has_dynamics() const { return ghost + accent > 0; }
    // Field-by-field addition: the one sum every total below is built from.
    DynamicsCounts& operator+=(const DynamicsCounts& o) {
        ghost += o.ghost;
        accent += o.accent;
        normal += o.normal;
        return *this;
    }
};
```

Replace lines 48-50:

```cpp
    DynamicsCounts pads_total() const;
    DynamicsCounts kicks_total() const;
    DynamicsCounts played_total(bool bass2x) const;
```

with:

```cpp
    DynamicsCounts pads_total() const;
    // The kick notes that count under this 2x Bass setting: the Kick row,
    // plus the 2x kick row when 2x Bass is on. The one answer to "which kicks
    // count" (finding 12): "All kicks" and Totals both read it.
    DynamicsCounts kicks_total(bool bass2x) const;
    // Every note that counts: pads_total() plus kicks_total(bass2x).
    DynamicsCounts played_total(bool bass2x) const;
```

Replace lines 67-69:

```cpp
// The Dynamics tab's background count always parses with 2x kicks kept, so
// the "2x kick" row is known even while the "2x Bass" box is off.
constexpr bool kDynamicsParseBass2x = true;
```

with:

```cpp
// The Dynamics tab's background count always parses with 2x kicks kept, so
// the "2x kick" row is known even while the "2x Bass" box is off. The parser
// reads only the parsed difficulty's own 2x kicks (D20), so below Expert this
// row never holds Expert's.
constexpr bool kDynamicsParseBass2x = true;
```

In `src/app/dynamics_breakdown.cpp`, replace lines 17-50 (`pads_total`, `kicks_total`, `played_total`) with:

```cpp
DynamicsCounts DynamicsBreakdown::pads_total() const {
    DynamicsCounts t;
    for (size_t i = 0; i <= static_cast<size_t>(DynamicsRow::GreenTom); ++i) t += rows[i];
    return t;
}

DynamicsCounts DynamicsBreakdown::kicks_total(bool bass2x) const {
    DynamicsCounts t = row(DynamicsRow::Kick);
    if (bass2x) t += row(DynamicsRow::Kick2x);
    return t;
}

DynamicsCounts DynamicsBreakdown::played_total(bool bass2x) const {
    DynamicsCounts t = pads_total();
    t += kicks_total(bass2x);
    return t;
}
```

- [ ] **Step 7: The tab reads the one total.** In `src/ui/dynamics_tab.cpp`, replace lines 150-155:

```cpp
    {
        const app::DynamicsCounts k2x = bd.row(app::DynamicsRow::Kick2x);
        const app::DynamicsCounts ktot = bd.kicks_total();
        int pct = ktot.all() > 0
                      ? static_cast<int>(100.0 * k2x.all() / ktot.all())
                      : 0;
```

with:

```cpp
    {
        // How many of the chart's kick notes are 2x, counted or not: a fact
        // about the chart, so it asks for every kick.
        const app::DynamicsCounts k2x = bd.row(app::DynamicsRow::Kick2x);
        const app::DynamicsCounts ktot = bd.kicks_total(/*bass2x=*/true);
        int pct = ktot.all() > 0
                      ? static_cast<int>(100.0 * k2x.all() / ktot.all())
                      : 0;
```

Replace line 183 (`            const app::DynamicsCounts ktot = bd.kicks_total();`) with:

```cpp
            // The same kicks Totals counts (finding 12).
            const app::DynamicsCounts ktot = bd.kicks_total(bass2x);
```

- [ ] **Step 8: The pins this rule change breaks.** Each edit below is a pinned expectation D20 changes on purpose.

`tests/test_dynamics_breakdown.cpp` lines 148-149 call `kicks_total()`, which no longer exists. Replace:

```cpp
    // kicks_total = Kick(3) + Kick2x(1)
    CHECK(bd.kicks_total().all() == 4);
```

with:

```cpp
    // kicks_total(true) = Kick(3) + Kick2x(1); kicks_total(false) = Kick(3)
    CHECK(bd.kicks_total(true).all() == 4);
    CHECK(bd.kicks_total(false).all() == 3);
```

`tests/test_config.cpp` lines 144-167 pin "a non-Expert difficulty is always 1x Bass". Replace the whole case with:

```cpp
TEST_CASE("chartmode_key: 2x Bass applies at every difficulty (D20)") {
    Settings s;
    s.view_difficulty = "Hard";
    s.view_prodrums = true;
    s.view_bass2x = true;  // Clone Hero's Double Kick works at every difficulty
    CHECK(s.difficulty() == hydra::Difficulty::Hard);
    CHECK(s.effective_bass2x());
    CHECK(s.chartmode_key() == "Hard Pro Drums, 2x Bass");
    CHECK(s.to_analysis_settings().bass2x);
    CHECK(s.to_analysis_settings().difficulty == hydra::Difficulty::Hard);

    s.view_bass2x = false;
    CHECK_FALSE(s.effective_bass2x());
    CHECK(s.chartmode_key() == "Hard Pro Drums, 1x Bass");
    s.view_prodrums = false;
    CHECK(s.chartmode_key() == "Hard Drums, 1x Bass");
    s.view_difficulty = "Medium";
    CHECK(s.chartmode_key() == "Medium Drums, 1x Bass");
    s.view_bass2x = true;
    CHECK(s.chartmode_key() == "Medium Drums, 2x Bass");
    s.view_difficulty = "Easy";
    CHECK(s.chartmode_key() == "Easy Drums, 2x Bass");

    // Expert's keys don't move (the case above pins all four byte for byte).
    s.view_difficulty = "Expert";
    s.view_prodrums = true;
    CHECK(s.chartmode_key() == "Expert Pro Drums, 2x Bass");
}
```

Line 204 (`        CHECK(r.chartmode_key() == "Hard Pro Drums, 1x Bass");`) becomes:

```cpp
        // view_bass2x defaults on, and applies at Hard too (D20).
        CHECK(r.chartmode_key() == "Hard Pro Drums, 2x Bass");
```

`tests/test_batch_text.cpp` line 35 (`    s.view_difficulty = "Hard";  // 2x Bass is Expert-only, so it drops out`) becomes `    s.view_difficulty = "Hard";  // 2x Bass (still on) applies at Hard too (D20)`, and line 42 (`    CHECK(d.difficulty == "Hard");`) becomes `    CHECK(d.difficulty == "Hard \xC2\xB7 2x Bass");`.

- [ ] **Step 9: Run the doctest suite.** `.\build-cpp\Release\hydra_tests.exe -tc="*2x kick*,*kick total*,*Car Bomb*,*chartmode_key*,batch text*,dynamics_breakdown*"`, then the full `.\build-cpp\Release\hydra_tests.exe` → `Status: SUCCESS!`.

- [ ] **Step 10: The GUI pins.** `tests/ui/uitest_library.cpp`'s `test_difficulty` pins the greyed-out box. Replace lines 33-35:

```cpp
// The settings bar's difficulty dropdown: it drives the chartmode everything
// else is keyed by, and it disables 2x Bass (an Expert-only charting concept)
// without forgetting the user's stored setting.
```

with:

```cpp
// The settings bar's difficulty dropdown: it drives the chartmode everything
// else is keyed by. 2x Bass stays live at every difficulty (D20) and is part
// of the key there too.
```

Replace lines 50-56:

```cpp
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 1x Bass");

    // Visibly disabled, unchecked, and the stored flag is untouched.
    ImGuiTestItemInfo bass = ctx->ItemInfo("2x Bass");
    IM_CHECK((bass.ItemFlags & ImGuiItemFlags_Disabled) != 0);
    IM_CHECK(h.app->settings.view_bass2x);
    IM_CHECK(!h.app->settings.effective_bass2x());
```

with:

```cpp
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 2x Bass");

    // Live at Hard: unticking it changes Hard's key, and ticking it restores it.
    IM_CHECK((ctx->ItemInfo("2x Bass").ItemFlags & ImGuiItemFlags_Disabled) == 0);
    IM_CHECK(h.app->settings.effective_bass2x());
    ctx->ItemClick("2x Bass");
    IM_CHECK(wait_until(ctx, [&] { return !h.app->settings.view_bass2x; }, 5));
    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 1x Bass");
    ctx->ItemClick("2x Bass");
    IM_CHECK(wait_until(ctx, [&] { return h.app->settings.view_bass2x; }, 5));
```

Replace lines 58-60:

```cpp
    // Back on Expert the box is live again, still carrying the user's own
    // setting. (Checked here rather than at the end of the test: the details
    // modal opened below has no close button the harness can address.)
```

with:

```cpp
    // Back on Expert the box still carries the user's own setting. (Checked
    // here rather than at the end of the test: the details modal opened below
    // has no close button the harness can address.)
```

Line 79 (`    IM_CHECK_STR_EQ(h.app->settings.chartmode_key().c_str(), "Hard Pro Drums, 1x Bass");`) becomes the same check with `"Hard Pro Drums, 2x Bass"`. The analysis that follows (Pokemon Theme at Hard) now runs with 2x on; that chart has no Hard `N 32`, so its path does not change.

`tests/ui/uitest_details.cpp` line 253 (`IM_CHECK(h.app->settings.effective_bass2x());`) stays as it is: it runs at Expert, where D20 changes nothing. Instead, add a Hard test after `test_dynamics_stored` (which ends at line 268):

```cpp
// At Hard the Dynamics tab counts Hard's own kicks (findings 10, 12, 255).
// Pathfinder - When The Sunrise Breaks The Darkness has 1,613 Expert 2x kicks
// (pitch 95) and no Hard ones; 139 of its 972 Hard kicks share a tick with a
// 95. Before D20 the tab showed 833 kicks, a phantom row of 1,613 "2x kicks"
// and Totals of 3,287.
void test_dynamics_hard(ImGuiTestContext* ctx) {
    Harness& h = harness(ctx);
    reset_app(h);
    scan_library(ctx);
    if (ctx->IsError()) return;
    h.app->settings.view_difficulty = "Hard";
    h.app->commit_settings();
    IM_CHECK(h.app->settings.effective_bass2x());  // the default, now at Hard too
    open_titled(ctx, "Sunrise Breaks", "When The Sunrise Breaks The Darkness");
    if (ctx->IsError()) return;

    ctx->ItemClick("##DetailsTabs/Dynamics");
    IM_CHECK(wait_until(ctx, [&] { return h.app->dynamics_result.has_value(); }, 60));
    std::string text = visible_text(h);
    IM_CHECK(text.find("2x kicks: 0 of 972 kick notes (0%)") != std::string::npos);
    IM_CHECK(text.find(" of 3,426 (") != std::string::npos);  // Dynamic notes: X of 3,426

    // 2x Bass off: Totals is unchanged, because Hard has no 2x kicks to drop.
    h.app->settings.view_bass2x = false;
    h.app->commit_settings();
    IM_CHECK(wait_until(ctx, [&] {
        return visible_text(h).find("not counted (2x Bass off)") != std::string::npos;
    }, 5));
    IM_CHECK(visible_text(h).find(" of 3,426 (") != std::string::npos);

    // Restore the defaults the next test relies on.
    h.app->settings.view_bass2x = true;
    h.app->settings.view_difficulty = "Expert";
    h.app->commit_settings();
}
```

and add `        {"dynamics-hard", test_dynamics_hard},` to `details_tests()` after line 769 (`        {"dynamics-stored", test_dynamics_stored},`). A test not in `kRunOrder` runs after the listed ones, so `uitest_tests.cpp` needs no line.

Run `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --test difficulty --test dynamics --test dynamics-hard`, then `--all --jobs 4`. On the unchanged build, `dynamics-hard` would fail its first text check (it shows "1,613 of 2,446"); that is the red half of this step.

- [ ] **Step 11: The UserGuide.** The wording is question Q1, approved in D31. In `docs/UserGuide.md`, replace line 41:

> **2x Bass.** Include the chart's 2x kicks. 2x Bass only exists on Expert, so it is greyed out on the other difficulties.

with:

> **2x Bass.** Include the chart's 2x kicks, like Clone Hero's Double Kick modifier. It works at every difficulty. Each difficulty has its own 2x kicks, though few charts have any below Expert.

In line 168, replace the sentence "When 2x Bass is off, the 2x kick row stays visible but greyed out and is left out of the totals." with "When 2x Bass is off, the 2x kick row stays visible but greyed out, and both All kicks and the totals leave it out." The rest of line 168, and line 172, stay.

- [ ] **Step 12: The corpus scores.** Build everything (`.\build_cpp.ps1`), then:

```powershell
$w = "$sp\s2-t2"
$exe = "C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t2\build-cpp\Release\hydra_replay.exe"
$runs = @(@('expert', '1'), @('hard', '1'), @('medium', '1'), @('easy', '1'),
          @('hard', '0'), @('medium', '0'), @('easy', '0'))
foreach ($r in $runs) {
    $d = $r[0]; $x = $r[1]
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -Bass2x $x -Out "$w\t2-$d-2x$x.txt" -Work "$w\work"
    "== $d 2x$x"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$base\base-$d-2x$x.txt" -After "$w\t2-$d-2x$x.txt"
}
```

Expected: no output under any heading. No corpus chart has pitch 59, 71 or 83, or `N 32` below Expert, so turning 2x on below Expert reads nothing new.

- [ ] **Step 13: Optional library spot check.** With 2x on at Hard, dump Like Moths To Flames - Ameliorate [sylvexite] (path in `$sp\s2-kick2x\lib_low_list.json`): 299,805 with the base build, 339,420 with this lane, path unchanged. Krallice - Energy Chasms at Hard: the base build refuses with "chart has no notes"; this lane scores 152,950. The verifier reproduced both.

- [ ] **Step 14: Commit** `src/parse/song.cpp src/app/config.cpp src/app/config.h src/ui/settings_bar.cpp src/app/dynamics_breakdown.h src/app/dynamics_breakdown.cpp src/ui/dynamics_tab.cpp docs/UserGuide.md tests/test_s2_kick2x.cpp tests/test_config.cpp tests/test_batch_text.cpp tests/test_dynamics_breakdown.cpp tests/ui/uitest_library.cpp tests/ui/uitest_details.cpp CMakeLists.txt` by name: "2x kicks per difficulty; 2x Bass at every difficulty; one Dynamics kick total".

**Done when:**
- [ ] The five new cases and the four edited test files pass; the full `hydra_tests` suite ends `Status: SUCCESS!`.
- [ ] `hydra_uitest --all --jobs 4` passes, including `difficulty`, `dynamics` and the new `dynamics-hard`. If the known settings-lock flake fails, it passes when run alone.
- [ ] All seven corpus score files equal the baseline (Step 12 prints nothing).
- [ ] `git grep -n "case 95" -- src/parse/song.cpp` and `git grep -n "Expert-only" -- src/ui/settings_bar.cpp src/app/config.h src/app/config.cpp docs/UserGuide.md` find nothing.
- [ ] `git grep -n "kicks_total()" -- src tests` finds nothing: every caller says which kicks it wants.
- [ ] `git grep -n "= 9[56];" -- src/parse/song.cpp` finds nothing: no parser member holds a copy of Expert's pitches.

**Stamp needs for T9:** `kDynamicsCountStamp` 1 → 2 (lower-difficulty counts lose the phantom row and regain swallowed kicks), shared with T1 and T5. No results-stamp need of its own: a stored "1x Bass" result is still right, and lower-difficulty "2x Bass" keys are new keys. D20 says it ships with step 1's results-stamp bump. T9 also records "2x kicks at every difficulty" in `CONTEXT.md` (line 47 names the 2x Bass setting) and `docs/differences-from-public-hydra.md`.

**Dependencies:** branches from T0 and reads `difficulty_chart_codes`.

**Merge notes (textual neighbours, for the integrator).** Each neighbour gets one sentence.

T1 adds a `MidiParser` member right after line 478, which T2 replaces with two members, and a line after line 677 beside T2's; keep all of them.

T3 edits `MidiParser::optype` at lines 550-553, about twenty lines below T2's 526-529 in the same function; both edits stay.

T5 adds fields to `struct DynamicsBreakdown` (lines 43-51), close to T2's edit at lines 48-50; keep both.

T5 also changes the Dynamics tab's Chart section (lines 211-216), below T2's edits at 150-185.

T5's blob changes in `encode_dynamics` and `decode_dynamics` don't overlap T2's lines.

T4 and T7 also edit `docs/UserGuide.md`, on other lines.

---

### Task T3: One phrase-end helper for both chart formats

**Goal:** One function decides which chord awards a Star Power phrase, and it uses Clone Hero's rule for both `.chart` and `.mid`.

**Findings:** 21 (and 344, which T9 records).

**Decision:** D21: "One phrase-end helper for both parsers, with Clone Hero's rule (0x20D2440): start <= tick < end; a zero-length phrase pays nothing; a phrase running past the last note pays on that note."

**How it works today.** Each parser closes a phrase on its own, then both call `mark_sp_phrase_end` (song.cpp line 182) to flag the last chord. That helper only checks that the chord is at or after the phrase start. It never sees the end tick.

The `.mid` side happens to be right. It closes the phrase at the 116 note-off, before that tick's notes are added, so a chord on the end tick is outside and a zero-length phrase awards nothing.

The `.chart` side has two mistakes. A zero-length phrase on a chord is awarded, because that chord is "at or after the start". And a phrase still open after the last tick is never closed, so its last chord never awards it.

| Phrase (chords at 480 to 2400, phrase starts at 1440) | `.mid` today | `.chart` today | Both after this task |
|---|---|---|---|
| ends on the chord at 1920 | 1440 | 1440 | 1440 |
| ends between chords, at 1680 | 1440 | 1440 | 1440 |
| runs to 2880, past the last chord | 2400 | nothing | 2400 |
| zero length on the chord at 1440 | nothing | 1440 | nothing |

**The change.** `close_sp_phrase(song, start, end)` replaces `mark_sp_phrase_end`. It flags the last chord only when start <= tick < end. Both parsers now pass the end tick. The `.mid` parser calls it at the 116 note-off, as today. The `.chart` parser calls it at the first tick at or past the end, as today, and once more after the last tick for a phrase still open. Think of the phrase as a half-open box: the chord on the closing wall is outside, and an empty box holds nothing.

The `.mid` behaviour can't change. When the 116 note-off runs, every chord already emitted sits before the note-off tick, so "tick < end" always holds there.

**What the user sees.** Measured on prototype `claude/s2-proto-phrasefill` f7e6e84, which this task reproduces. Best scores drop on 20 Expert charts, by 2,160 to 19,180 (largest: Muse - Feeling Good, 347,935 to 328,755). One Hard chart drops: Big Giant Circles feat. C418 - BGC418, 207,255 to 202,715. Best paths change on 18 of the 20 Expert charts. The Preview stops showing a phrase note at those zero-length spots, and the SP gauge follows. Victoria - Modern Value's last chord becomes a phrase note in the Preview, with no score change. The test corpus does not change at any difficulty.

**Not changed here.** A `.chart` S 2 that starts inside an open phrase still replaces it (Clone Hero keeps both). The brief found no candidate chart with this and did not count it library-wide. It stays a known gap.

**Files.** All the code change is in `src/parse/song.cpp`, and each step below quotes the lines it replaces. The shared helper `mark_sp_phrase_end` becomes `close_sp_phrase`. On the `.mid` side, the 116 note-off op carries its tick, `MidiParser::run` passes it on, and `MidiParser::op_sp_end` hands it to the helper. On the `.chart` side, `ChartParser::op_sp_end`, its `run` and the end check in `push_timestamp` pass the end tick, and the end of `ChartParser::parse` closes a phrase still open after the last tick. Two op comments (`MOp::tick` and `COp::b`) say what the tick now means. The tests go in the new `tests/test_s2_phrase_end.cpp`, plus one line in `CMakeLists.txt`.

**Acceptance Criteria:**
- [ ] In both formats, a phrase ending on a chord or between chords flags the chord at 1440.
- [ ] In both formats, a phrase running past the last chord flags the last chord, and that chord's `sp_phrase_start` is 1440.
- [ ] In both formats, a zero-length phrase on a chord flags nothing.
- [ ] `.chart` and `.mid` give the same flags for every case.
- [ ] No other function in song.cpp is touched. `mark_sp_phrase_end` no longer exists.
- [ ] The corpus dump matches the baseline at all four difficulties. The library candidate dumps match the prototype's phrase dumps at all four difficulties.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*phrase end*"` gives `Status: SUCCESS!`, then the full run and the dump checks in "Done when".

**Steps:**

- [ ] **Step 1: Failing tests.** Create `tests/test_s2_phrase_end.cpp`. It writes the same chords and phrase once as `.chart` and once as `.mid` and compares the flags. The `.mid` builder only needs deltas of 0, 240 and 480 ticks, so it writes them as literal bytes rather than copying `put_varlen` from test_song.cpp.

```cpp
// Step 2, T3 (finding 21, D21): one phrase-end rule for both chart
// formats. A Star Power phrase covers start <= tick < end, as Clone Hero 1.1
// assigns notes to phrases (0x20D2440). The same chords and the same phrase,
// written once as .chart and once as .mid, must flag the same chord.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

namespace {

// One red pad every 480 ticks from 480 to 2400, at 480 ticks per beat in both
// formats. The phrase always starts on the chord at 1440.
constexpr int64_t kFirstChord = 480, kLastChord = 2400, kStep = 480;
constexpr int64_t kPhraseStart = 1440;

struct Flags {
    std::vector<int64_t> ticks;   // chords with flag_sp
    std::vector<int64_t> starts;  // their sp_phrase_start
    // Written out: the code is C++17, which has no defaulted ==.
    bool operator==(const Flags& o) const { return ticks == o.ticks && starts == o.starts; }
};

Flags flags_of(const Song& song) {
    Flags f;
    for (const SongTimestamp& ts : song.sequence) {
        if (!ts.flag_sp) continue;
        f.ticks.push_back(ts.timecode.ticks());
        f.starts.push_back(ts.sp_phrase_start.value_or(-1));
    }
    return f;
}

Flags chart_flags(int64_t length) {
    std::string text =
        "[Song]\n{\n  Resolution = 480\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n";
    for (int64_t t = kFirstChord; t <= kLastChord; t += kStep) {
        if (t == kPhraseStart)
            text += "  " + std::to_string(t) + " = S 2 " + std::to_string(length) + "\n";
        text += "  " + std::to_string(t) + " = N 1 0\n";
    }
    text += "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    return flags_of(load_songbytes_chart(data, true, true));
}

// The delta before an event, as MIDI writes it. Only the gaps this file uses.
std::vector<uint8_t> delta_bytes(int64_t gap) {
    switch (gap) {
        case 0: return {0x00};
        case 240: return {0x81, 0x70};
        case 480: return {0x83, 0x60};
    }
    FAIL("delta_bytes: add the encoding for a gap of " << gap);
    return {};
}

Flags mid_flags(int64_t length) {
    struct Ev {
        int64_t tick;
        int order;  // at one tick: phrase on, phrase off, then the note
        std::vector<uint8_t> bytes;
    };
    std::vector<Ev> evs;
    for (int64_t t = kFirstChord; t <= kLastChord; t += kStep)
        evs.push_back({t, 2, {0x90, 97, 100}});           // Expert red pad
    evs.push_back({kPhraseStart, 0, {0x90, 116, 100}});   // SP phrase on
    evs.push_back({kPhraseStart + length, 1, {0x80, 116, 0}});  // SP phrase off
    std::stable_sort(evs.begin(), evs.end(), [](const Ev& a, const Ev& b) {
        return a.tick != b.tick ? a.tick < b.tick : a.order < b.order;
    });
    std::vector<uint8_t> track =
        testmidi::concat({testmidi::track_name("PART DRUMS"), testmidi::set_tempo()});
    int64_t at = 0;
    for (const Ev& e : evs) {
        const std::vector<uint8_t> d = delta_bytes(e.tick - at);
        track.insert(track.end(), d.begin(), d.end());
        track.insert(track.end(), e.bytes.begin(), e.bytes.end());
        at = e.tick;
    }
    const std::vector<uint8_t> eot = testmidi::end_of_track();
    track.insert(track.end(), eot.begin(), eot.end());
    return flags_of(load_songbytes_mid(testmidi::smf(track), true, true));
}

}  // namespace

TEST_CASE("phrase end: a phrase ending on or between chords flags the last chord inside") {
    for (int64_t length : {480, 240}) {  // ends on the chord at 1920; ends at 1680
        CAPTURE(length);
        const Flags want{{1440}, {1440}};
        CHECK(chart_flags(length) == want);
        CHECK(mid_flags(length) == want);
    }
}

TEST_CASE("phrase end: a phrase running past the last note flags that note") {
    // 1440 + 1440 = 2880, a beat after the last chord.
    const Flags want{{2400}, {1440}};
    CHECK(chart_flags(1440) == want);
    CHECK(mid_flags(1440) == want);
}

TEST_CASE("phrase end: a zero-length phrase on a chord flags nothing") {
    CHECK(chart_flags(0) == Flags{});
    CHECK(mid_flags(0) == Flags{});
}
```

Add `    tests/test_s2_phrase_end.cpp` at the end of the `hydra_tests` list in `CMakeLists.txt`, after line 429 (`tests/test_long_paths.cpp`).

- [ ] **Step 2: Build and watch them fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="phrase end*"`. Expect two failures, both on the `.chart` side: the past-the-end case gets no flag, and the zero-length case flags 1440. The `.mid` checks and the first case pass already.

- [ ] **Step 3: The helper.** Replace song.cpp lines 180-189, which today read:

```cpp
// An SP phrase ending: the last chord closes the phrase that began at
// `starttick`, if it lies inside it.
void mark_sp_phrase_end(Song& song, int64_t starttick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    if (last.timecode.ticks() >= starttick) {
        last.flag_sp = true;
        last.sp_phrase_start = starttick;
    }
}
```

with:

```cpp
// The one owner of "which chord awards this SP phrase", for both formats. A
// phrase covers the ticks start <= t < end, as Clone Hero 1.1 assigns notes
// to phrases (0x20D2440), and the last chord inside it gets the phrase. So a
// zero-length phrase, or one with no chord inside, awards nothing, and a
// phrase running past the last note is awarded on that note (finding 21,
// D21). Each parser calls this once no later chord can fall inside the
// phrase, always before that tick's own chord is emitted: .mid at the 116
// note-off; .chart at the first tick at or past the end, and once more after
// the last tick for a phrase still open.
void close_sp_phrase(Song& song, int64_t start_tick, int64_t end_tick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    const int64_t t = last.timecode.ticks();
    if (t >= start_tick && t < end_tick) {
        last.flag_sp = true;
        last.sp_phrase_start = start_tick;
    }
}
```

- [ ] **Step 4: The `.mid` parser passes the note-off tick.** Line 357 today reads `int64_t tick = 0;      // FillStart/StoreFillEnd/SpStart/Tempo/TimeSig;`. Add `SpEnd` to that list. Lines 457-461 become:

```cpp
    void op_sp_end(int64_t end_tick) {
        // A note-off with no phrase open (a stray 116 off) closes nothing.
        if (sp_start_tick_) close_sp_phrase(*song_, *sp_start_tick_, end_tick);
        sp_start_tick_.reset();
    }
```

Lines 550-553, the 116 note-off, carry the tick (the phase choice is unchanged):

```cpp
                case 116:
                    return mop_tick(sp_start_tick_.has_value() ? MPhase::Pre
                                                               : MPhase::PreDelayed,
                                    MAct::SpEnd, tick);
```

Line 601 becomes `case MAct::SpEnd: op_sp_end(op.tick); break;`.

- [ ] **Step 5: The `.chart` parser passes the end and closes after the last tick.** Line 890 today reads `int64_t b = 0;   // SpStart/FillStart: end tick`. Make it `// SpStart/FillStart/SpEnd: end tick`. Lines 951-954 become:

```cpp
    void op_sp_end(int64_t start, int64_t end) {
        close_sp_phrase(*song_, start, end);
        sp_end_tick_.reset();
    }
```

Line 1118 becomes `case CAct::SpEnd: op_sp_end(op.a, op.b); break;`. Line 1151 becomes:

```cpp
        ops.insert(ops.begin(), cop_span(CPhase::Pre, CAct::SpEnd, start, *sp_end_tick_));
```

After the loop at lines 1230-1231 (`for (int64_t tk : ed.tick_order) push_timestamp(tk, ed.tick_data.at(tk));`), still inside the `if (ed_it != sections_.end())` block, add:

```cpp
        // A phrase still open after the last tick runs past the last note.
        // Close it now, so that note awards it, as the 116 note-off does in
        // a .mid.
        if (sp_end_tick_.has_value())
            op_sp_end(sp_start_tick_.value_or(0), *sp_end_tick_);
```

- [ ] **Step 6: Run the new tests, then the full suite.** `.\build-cpp\Release\hydra_tests.exe -tc="phrase end*"`, then `.\build-cpp\Release\hydra_tests.exe`. Both give `Status: SUCCESS!`. The prototype's full run passed with no existing test edited, so none should need editing here. If one fails, stop and report it rather than editing it.

- [ ] **Step 7: Dump checks** (see "Done when").

- [ ] **Step 8: Commit** `src/parse/song.cpp tests/test_s2_phrase_end.cpp CMakeLists.txt` by name, never `git add -A`: "One phrase-end rule for both formats: start <= tick < end (finding 21, D21)", with the `Task: s2-t3`, `Agent:` and `Session:` trailers and the Co-Authored-By line.

**Done when:**
- [ ] `hydra_tests.exe` (full) reports `Status: SUCCESS!` in the worktree.
- [ ] The corpus is unchanged. Run the lane corpus check from "Before the wave" at all four difficulties, with `$lane` the s2-t3 worktree, `$w = "$sp\s2-t3"` and files `t3-$d-2x1.txt`. All four blocks print nothing.
- [ ] The library candidates move exactly as measured. Dump each difficulty's candidate list and compare with the prototype's phrase dumps. All four comparisons print nothing:

```powershell
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -List "$PF\lib_cand_$d.txt" -Out "$w\t3-libcand-$d.txt" -Work "$w\work"
    "== $d"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$PF\out_libcand_phrase_$d.txt" -After "$w\t3-libcand-$d.txt"
}
```

  That pins the 20 Expert drops (Feeling Good 347,935 to 328,755 among them), the one Hard drop (BGC418 207,255 to 202,715), and no change at Medium or Easy. A sample of 150 non-candidate charts showed no change on the prototype; this task does not repeat it.
- [ ] The diff touches only the song.cpp lines the steps quote (the functions named under Files), the new test file and one CMake line.

**Stamp needs (for T9):** the shared results stamp (`kResultsStamp`, step 1 Task 22's value; D23). Not `kDynamicsCountStamp`: `count_dynamics` never reads `flag_sp` or `sp_phrase_start`, so no note count changes. Not `kPathFormatStamp`: the path layout is unchanged.

**Merge notes (textual neighbours, for the integrator).** Each neighbour gets one sentence.

T5 edits the same `MOp::tick` comment (line 357) and the same `MidiParser::run` switch (T3 line 601, T5 line 605); keep both, so the comment lists `SpEnd` and `Dynamics` and each case passes `op.tick`.

T4 also edits that comment (it drops ApplyFill) and `run`; keep both edits.

T4 edits `MidiParser`'s fill ops at lines 450-455, one line above this task's `op_sp_end` at 457-461; keep both.

T4 deletes the fill block right after this task's SP-end check in `ChartParser::push_timestamp` (lines 1154-1163); keep this task's check.

T4 adds its `place_authored_fills` call where this task adds the after-the-last-tick phrase close (after line 1231); keep both, phrase close first.

T2 edits `MidiParser::optype` at lines 526-529, in the same function as this task's 550-553; both edits stay.

T6 closes and reopens the anonymous namespace at lines 153-170, just above this task's helper at 180-189.

---

### Task T4: Fill placement: Clone Hero's extra tick, and each fill placed on its own

**Goal:** An authored fill lands on a chord up to trunc(resolution x slop) + 1 ticks after the fill ends, as Clone Hero computes it, and every authored fill is placed on its own once the chord list is complete, so no fill is forgotten.

**Findings:** 315 (the +1 and Fill B; the rest of 315 is D26).

**Decisions:** D22: "The fill landing window is floor(resolution x slop) + 1 ticks, as Clone Hero computes it (0x20D0088-0x20D008D). The +1 applies on top of a user's `hydra_rules.ini` slop too." D30: "Authored fills are placed one by one after the whole chord list is read, as Clone Hero does (0x20CFF60 calling 0x5DE030). Each fill takes the last chord at or before its end, but not before its start, or the first chord after its end within the slop. A tie goes to the later chord, and a fill with no chord in bounds is dropped." Clone Hero's unbounded one-side fallback (B2) is not adopted.

The two decisions are one task because both own fill placement. The task makes two commits in one worktree: the +1 first, inside today's `fill_lands_on_chord`, then Fill B, which replaces that function with a placement pass that carries the +1 with it. Each commit has its own measured check, so a difference can be traced to one of them.

**How it works today.** When a drum fill ends, Hydra has to pick the chord the fill "lands" on, which becomes the activation chord. `fill_lands_on_chord` (song.cpp line 141), shared by both parsers, takes the next chord if it is within trunc(resolution x 1/32) ticks of the fill end and no farther away than the previous chord. Otherwise the fill goes on the previous chord, or is dropped if that chord is before the fill start. Generated fills never use this window.

Both parsers decide this while they stream through the chart, and each keeps only one pending fill. When a second fill starts before any chord has reached the first fill's end, the first fill's start and end are overwritten, and the first fill is forgotten. Its path skip counts can then say one fewer fill than the game shows.

**What Clone Hero does.** It computes the same window and then adds one tick: it truncates resolution x 0.03125 at 0x20D0088 (`cvttsd2si`) and adds one at 0x20D008D (`inc`). A chord exactly at the edge still lands (0x5DE17D rejects only when the edge is below the chord's tick). So at resolution 480 Hydra allows 15 ticks and Clone Hero allows 16. And it places fills after the whole chord list is read: 0x20CFF60 calls 0x5DE030 once per fill, which compares the last chord at or before the fill end (not before the fill start) with the first chord after it (not past the window), takes the closer, and gives a tie to the later one.

**The change.** Commit 1 adds the one tick inside `fill_lands_on_chord`, with a comment naming the addresses. The rules value stays 1/32, so the `hydra_rules.ini` fingerprint does not move. The +1 sits on top of the rules value, so a user who sets the slop to 0 still gets a one-tick window (D22 says so). The `rules.h` comment and the User Guide line for `fill_land_slop_beats` say so too.

Commit 2 records each authored fill whole (start and end) while the chart is read, and places them all in one pass, `place_authored_fills`, after the last chord. The pass sorts the chord list by tick, then for each fill finds the two candidates and applies D30's rule. The +1 window moves into that pass. The parsers lose their fill-end bookkeeping: the `ApplyFill` and `FillEnd` steps, `fill_end_tick_`, and the placement blocks in both `push_timestamp` functions. Think of it as seating guests after everyone has arrived instead of at the door: nobody's seat is given away because the next guest walked in first.

Fill B also closes the gap the phrasefill verifier found after the +1. When the chord before a fill end is earlier than the fill start, and the next chord is inside the window but farther away than that earlier chord, Hydra dropped the fill and Clone Hero lands it on the next chord. Under D30 the earlier chord isn't a candidate at all, so the next one wins. The library has 0 such cases today.

**What the user sees.** Commit 1, measured on prototype c34ab44 (and the fill-only build c9a3958): two charts change, both Thrice charts by Hoph2o, at every difficulty. In each, the first chord sits 16 ticks after a fill end at resolution 480. So Strange I Remember You goes from 401,340 to 402,060 at Expert, and gains 460 at Hard, Medium and Easy. The Illusion Of Safety (Album) goes from 4,386,960 to 4,387,680 at Expert, and gains 460 at Hard, Medium and Easy. An activation point moves on each.

Commit 2, measured on prototype 36e118a (`claude/s2-proto-fillb`): no best score changes anywhere. The best path's fill counts change on 3 of 28 candidate charts at Expert, 2 of 28 at Hard, 5 of 28 at Medium and 5 of 33 at Easy, for example Dirge Within - Forever the Martyr `1+ 0 2` → `2+ 0 2`, and Third Eye Blind - Never Let You Go '09 `0 E1 0 E1` → `0 E1 E1 E1`. A 150-chart library sample showed no change. The test corpus changes for neither commit.

**Files.** Most of the change is in `src/parse/song.cpp`, and each step below quotes the lines it replaces.

Commit 1 touches one function: `fill_lands_on_chord` and its comment gain the extra tick.

Commit 2 replaces that function with the placement pass. Near the top of the file, `apply_fill_end` becomes three pieces: the `AuthoredFill` record, `apply_fill_end` by chord index, and `place_authored_fills`.

On the `.mid` side, commit 2 removes the fill-end bookkeeping. That means the fill step in `MAct`, the `MOp::tick` comment, the fill ops and members in `MidiParser`, its `run` case, the fill block in its `push_timestamp`, and two lines in pass 2 of `MidiParser::parse`.

On the `.chart` side, commit 2 removes the same bookkeeping. That means the fill step in `CAct`, the `COp::a` comment, the fill ops and members in `ChartParser`, its `run` case and the fill block in its `push_timestamp`. The end of `ChartParser::parse` calls the new pass.

Outside the parser, the `fill_land_slop_beats` comment in `src/core/rules.h` and its line in `docs/UserGuide.md` say the +1 applies. The tests go in the new `tests/test_s2_fill_landing.cpp`, plus one line in `CMakeLists.txt`.

**Acceptance Criteria:**
- [ ] `.chart` at resolution 192: a chord 7 ticks after the fill end takes the fill; 8 ticks after, the earlier chord does.
- [ ] `.mid` at resolution 480: a chord 16 ticks after the fill end takes the fill (the Thrice case); 17 ticks after, the earlier chord does.
- [ ] With `fill_land_slop_beats = 0`, a chord 1 tick after the fill end takes the fill; 2 ticks after, the earlier chord does.
- [ ] In both formats, two fills where the second starts before any chord reaches the first one's end both land.
- [ ] A fill whose earlier chord is before its start lands on the next chord inside the window, even when that chord is farther away.
- [ ] A fill with a chord on only one side of its end keeps that side's bound: a fill before the first note and a fill past the last note are both dropped.
- [ ] Generated fills are untouched (no change to `Song::check_activations`).
- [ ] The corpus matches the baseline at every difficulty after each commit. The library candidates match the measured dumps for each commit.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="fill landing*"` gives `Status: SUCCESS!`, then the full run and the dump checks in "Done when".

**Steps (commit 1, the extra tick):**

- [ ] **Step 1: Failing tests.** Create `tests/test_s2_fill_landing.cpp`. In every case the earlier chord sits inside the fill, a full beat before the fill end, so the "no farther than the previous chord" test never decides, and an unlanded fill still gets a positive length.

```cpp
// Step 2, T4 (finding 315, D22 and D30): an authored fill lands on the next
// chord up to trunc(resolution x slop) + 1 ticks after the fill ends, as Clone
// Hero 1.1 computes it (cvttsd2si at 0x20D0088, inc at 0x20D008D). A chord
// exactly at the window's edge still lands. Each fill is placed on its own
// once every chord is read (0x20CFF60 calling 0x5DE030).

#include "doctest.h"

#include <cstdint>
#include <string>
#include <vector>

#include "core/rules.h"
#include "midi_util.h"
#include "parse/song.h"

using namespace hydra;

namespace {

int64_t activation_tick(const Song& song) {
    for (const SongTimestamp& ts : song.sequence)
        if (ts.has_activation()) return ts.timecode.ticks();
    return -1;
}

// A .chart at `res` ticks per beat: chords at 0 and 3*res, a fill from 2*res
// to 4*res, and one more chord `after` ticks past the fill end. Returns the
// tick of the chord the fill landed on.
int64_t chart_landing(int64_t res, int64_t after,
                      const core::Rules& rules = core::default_rules()) {
    const auto line = [](int64_t tick, const std::string& what) {
        return "  " + std::to_string(tick) + " = " + what + "\n";
    };
    const std::string text =
        "[Song]\n{\n  Resolution = " + std::to_string(res) + "\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n" +
        line(0, "N 1 0") + line(2 * res, "S 64 " + std::to_string(2 * res)) +
        line(3 * res, "N 1 0") + line(4 * res + after, "N 1 0") + "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    return activation_tick(load_songbytes_chart(data, true, true, Difficulty::Expert, rules));
}

// The same shape as a .mid at 480 ticks per beat: chords at 0 and 1440, the
// fill (pitch 120) from 960 to 1920, and one more chord `after` ticks later.
int64_t mid_landing(uint8_t after) {
    REQUIRE(after < 0x80);  // one delta byte
    const std::vector<uint8_t> track = testmidi::concat({
        testmidi::track_name("PART DRUMS"), testmidi::set_tempo(),
        {0x00, 0x90, 97, 100},          // tick 0: red
        {0x87, 0x40, 0x90, 120, 100},   // tick 960: fill starts
        {0x83, 0x60, 0x90, 97, 100},    // tick 1440: red, inside the fill
        {0x83, 0x60, 0x80, 120, 0},     // tick 1920: fill ends
        {after, 0x90, 97, 100},         // tick 1920 + after: red
        testmidi::end_of_track(),
    });
    return activation_tick(load_songbytes_mid(testmidi::smf(track), true, true));
}

}  // namespace

TEST_CASE("fill landing: .chart window is trunc(res/32) + 1 ticks") {
    // Resolution 192: trunc(6.0) + 1 = 7. The fill ends at 768.
    CHECK(chart_landing(192, 6) == 774);
    CHECK(chart_landing(192, 7) == 775);  // the edge still lands
    CHECK(chart_landing(192, 8) == 576);  // too far: the earlier chord takes it
}

TEST_CASE("fill landing: .mid window at resolution 480 is 16 ticks (the Thrice case)") {
    CHECK(mid_landing(15) == 1935);
    CHECK(mid_landing(16) == 1936);
    CHECK(mid_landing(17) == 1440);
}

TEST_CASE("fill landing: the extra tick applies on top of a user's slop of 0") {
    core::Rules rules = core::default_rules();
    rules.fill_land_slop_beats = 0.0;
    CHECK(chart_landing(192, 1, rules) == 769);
    CHECK(chart_landing(192, 2, rules) == 576);
}
```

Add `    tests/test_s2_fill_landing.cpp` at the end of the `hydra_tests` list in `CMakeLists.txt`, after line 429.

- [ ] **Step 2: Build and watch them fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="fill landing*"`. Expect exactly three failures: `chart_landing(192, 7)` gives 576, `mid_landing(16)` gives 1440, and `chart_landing(192, 1, rules)` gives 576.

- [ ] **Step 3: The extra tick.** Replace song.cpp lines 136-149, which today read:

```cpp
// Activation-fill placement heuristic, shared by both parsers: true when the
// fill that ended at `fill_end_tick` lands on the chord being emitted at
// `tick` (so its op must run after the chord emit), false when it belongs to
// an earlier chord (run it before). "Lands on" means the next chord is within
// a 1/32-of-a-beat slop of the fill end and no closer to the previous chord.
bool fill_lands_on_chord(const Song& song, int64_t fill_end_tick, int64_t tick,
                         double slop_beats) {
    std::optional<int64_t> prevchord_dist;
    if (!song.sequence.empty())
        prevchord_dist = fill_end_tick - song.sequence.back().timecode.ticks();
    int64_t nextchord_dist = tick - fill_end_tick;
    return nextchord_dist <= static_cast<int64_t>(song.tick_resolution() * slop_beats) &&
           (!prevchord_dist.has_value() || nextchord_dist <= *prevchord_dist);
}
```

with:

```cpp
// Activation-fill placement, shared by both parsers: true when the fill that
// ended at `fill_end_tick` lands on the chord being emitted at `tick` (so its
// op must run after the chord emit), false when it belongs to an earlier
// chord (run it before). "Lands on" means the next chord is inside the
// landing window and no farther from the fill end than the previous chord; a
// tie goes to the next chord.
//
// The window is Clone Hero 1.1's: the whole ticks of resolution x slop, plus
// one tick (cvttsd2si truncates at 0x20D0088, then inc adds one at
// 0x20D008D), and a chord exactly at its edge still lands (finding 315, D22).
// The +1 sits on top of the rules value, so a hydra_rules.ini slop of 0 still
// lands a chord one tick late.
bool fill_lands_on_chord(const Song& song, int64_t fill_end_tick, int64_t tick,
                         double slop_beats) {
    std::optional<int64_t> prevchord_dist;
    if (!song.sequence.empty())
        prevchord_dist = fill_end_tick - song.sequence.back().timecode.ticks();
    const int64_t nextchord_dist = tick - fill_end_tick;
    const int64_t window_ticks =
        static_cast<int64_t>(song.tick_resolution() * slop_beats) + 1;
    return nextchord_dist <= window_ticks &&
           (!prevchord_dist.has_value() || nextchord_dist <= *prevchord_dist);
}
```

- [ ] **Step 4: Say so where the setting lives.** `rules.h` lines 38-40 today read:

```cpp
    // Authored-fill placement (fill_lands_on_chord, both parsers): how close
    // the next chord must be to the fill end to count as the chord the fill
    // lands on.
```

Replace them with:

```cpp
    // Authored-fill placement (fill_lands_on_chord, both parsers): how close
    // the next chord must be to the fill end to count as the chord the fill
    // lands on, in beats. The parser takes the whole ticks of this and adds
    // one tick, as Clone Hero 1.1 does (finding 315, D22), so 0 still allows
    // a chord one tick late.
```

`docs/UserGuide.md` line 253 today reads: "**`fill_land_slop_beats`** (default `0.03125`, a 32nd of a beat): how close a fill's end must be to a note for the fill to count. This one applies to authored fills too." Replace it with:

```markdown
- **`fill_land_slop_beats`** (default `0.03125`, a 32nd of a beat): how close after a fill's end a note must be for the fill to land on it. Hydra Deluxe adds one tick to this, as Clone Hero does, so even `0` lets a note one tick late take the fill. It applies only to fills written in the chart.
```

The old "applies to authored fills too" was wrong: generated fills never use this window (the phrasefill brief and its verifier both confirmed it). Since the line is being rewritten anyway, it now says what the code does.

- [ ] **Step 5: Run the new tests, then the full suite.** `-tc="fill landing*"`, then the full `hydra_tests.exe`. Both give `Status: SUCCESS!`. `tests/test_rules.cpp` lines 72 and 98 pin the rules value (1/32, and 0.125 from a file), which does not change. The prototype's full run passed with no test edited.

- [ ] **Step 6: Commit 1's dump checks.** Run the lane corpus check from "Before the wave" at all four difficulties (`$lane` the s2-t4 worktree, `$w = "$sp\s2-t4"`, files `t4a-$d-2x1.txt`). All four blocks print nothing. Then the library candidates, compared with the phrasefill brief's fill-only dumps (not its phrase ones):

```powershell
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -List "$PF\lib_cand_$d.txt" -Out "$w\t4a-libcand-$d.txt" -Work "$w\work"
    "== $d"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$PF\out_libcand_fill_$d.txt" -After "$w\t4a-libcand-$d.txt"
}
```

All four blocks print nothing. That pins both Thrice charts' gains at all four difficulties and no change to the 21 phrase candidates.

- [ ] **Step 7: Commit 1** `src/parse/song.cpp src/core/rules.h docs/UserGuide.md tests/test_s2_fill_landing.cpp CMakeLists.txt` by name: "Fill landing window gains Clone Hero's extra tick (finding 315, D22)", with the trailers.

**Steps (commit 2, each fill placed on its own):**

- [ ] **Step 8: Failing tests.** Append to `tests/test_s2_fill_landing.cpp`. The first two are the prototype's cases (36e118a, `tests/test_song.cpp`), moved here with the `.mid` deltas written as literal bytes (240 is `0x81 0x70`, 120 is `0x78`, 100 is `0x64`, 20 is `0x14`, 480 is `0x83 0x60`). The other three pin the window's two sides.

```cpp
// Fill B (D30): fills are placed after every chord is read. A fill whose next
// fill starts before any chord reaches its end used to be forgotten; now both
// land. Resolution 192: the window is 6 + 1 = 7 ticks.
TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.chart)") {
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = N 0 0\n"
        "  100 = S 64 100\n"   // fill A: 100-200
        "  150 = N 1 0\n"      // A's only chord, inside A
        "  180 = S 64 100\n"   // fill B: 180-280, starts before a chord reaches 200
        "  250 = N 2 0\n"      // B's only chord, inside B
        "  400 = N 3 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 4);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 50);
    REQUIRE(song.sequence[2].has_activation());
    CHECK(*song.sequence[2].activation_length == 70);
    CHECK_FALSE(song.sequence[3].has_activation());
}

TEST_CASE("fill landing: a fill is placed even when the next fill starts before its end chord (.mid)") {
    // 480 ticks per beat: the window is 15 + 1 = 16 ticks.
    using namespace testmidi;
    const std::vector<uint8_t> track = concat({
        track_name("PART DRUMS"), set_tempo(),
        note_on(96, 100),                // tick 0: kick
        {0x81, 0x70, 0x90, 120, 100},    // tick 240: fill A on
        {0x78, 0x90, 97, 100},           // tick 360: red, inside A
        {0x78, 0x90, 120, 0},            // tick 480: fill A off
        {0x14, 0x90, 120, 100},          // tick 500: fill B on, no chord since A's end
        {0x64, 0x90, 98, 100},           // tick 600: yellow, inside B
        {0x78, 0x90, 120, 0},            // tick 720: fill B off
        {0x83, 0x60, 0x90, 99, 100},     // tick 1200: blue
        end_of_track(),
    });
    const Song song = load_songbytes_mid(smf(track), true, true);
    REQUIRE(song.sequence.size() == 4);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 120);
    REQUIRE(song.sequence[2].has_activation());
    CHECK(*song.sequence[2].activation_length == 100);
    CHECK_FALSE(song.sequence[3].has_activation());
}

TEST_CASE("fill landing: an earlier chord before the fill start is no candidate") {
    // Fill 100-104. The chord at 99 is closer (5 ticks) but before the fill
    // start, so only the chord at 110 (6 ticks, inside the 7-tick window)
    // can take it. Before D30 the fill was dropped.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  99 = N 1 0\n"
        "  100 = S 64 4\n"
        "  110 = N 2 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 10);
}

TEST_CASE("fill landing: a fill before the first note keeps the window's bound (B2 not adopted)") {
    // Fill 0-50 has no chord before its end, and the first chord after it
    // (192) is far outside the window, so it is dropped. Clone Hero's 0x5DE030
    // would take the 192 chord; D30 keeps the bound.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = S 64 50\n"
        "  192 = N 0 0\n"
        "  300 = S 64 84\n"    // fill 300-384, lands on 384
        "  384 = N 1 0\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK_FALSE(song.sequence[0].has_activation());
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
    CHECK(song.features.empty());  // an authored fill landed: no generated ones
}

TEST_CASE("fill landing: a fill past the last note keeps its start bound (B2 not adopted)") {
    // Fill 500-600 has no chord after it, and the last chord before its end
    // (384) is before its start, so it is dropped. Clone Hero's 0x5DE030 would
    // move 384's fill to it; D30 keeps 384's own fill, 300-384.
    const std::string text =
        "[Song]\n{\n  Resolution = 192\n}\n"
        "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n"
        "[ExpertDrums]\n{\n"
        "  0 = N 0 0\n"
        "  300 = S 64 84\n"
        "  384 = N 1 0\n"
        "  500 = S 64 100\n"
        "}\n";
    const std::vector<uint8_t> data(text.begin(), text.end());
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    REQUIRE(song.sequence[1].has_activation());
    CHECK(*song.sequence[1].activation_length == 84);
}
```

- [ ] **Step 9: Build and watch them fail.** `-tc="fill landing*"`. Expect three failures: both "next fill starts before its end chord" cases (the first fill is forgotten, so `sequence[1]` has no activation) and "an earlier chord before the fill start" (the fill is dropped). The two "keeps the bound" cases pass already; they pin that D30's bounds hold through the rewrite.

- [ ] **Step 10: One placement pass.** In song.cpp, delete lines 136-149 (commit 1's `fill_lands_on_chord` and its comment). Replace lines 171-178, which today read:

```cpp
// A fill ending: the last chord becomes an activation chord whose fill began
// at `starttick`.
void apply_fill_end(Song& song, int64_t starttick) {
    if (song.sequence.empty()) return;
    SongTimestamp& last = song.sequence.back();
    if (last.timecode.ticks() >= starttick)
        last.activation_length = last.timecode.ticks() - starttick;
}
```

with:

```cpp
// One authored activation fill, as the chart wrote it.
struct AuthoredFill {
    int64_t start = 0;
    int64_t end = 0;
};

// A placed fill: the chord at `index` becomes an activation chord whose fill
// began at `starttick`.
void apply_fill_end(Song& song, size_t index, int64_t starttick) {
    SongTimestamp& ts = song.sequence[index];
    ts.activation_length = ts.timecode.ticks() - starttick;
}

// The one owner of authored-fill placement, for both parsers, run once every
// chord is read: each fill is placed on its own, as Clone Hero 1.1 does
// (0x20CFF60 calling 0x5DE030; finding 315, D30). The candidates are the last
// chord at or before the fill end, which must not be before the fill start,
// and the first chord after the end, which must be inside the landing window.
// When both qualify the closer wins, and a tie goes to the later chord. A fill
// with neither is dropped, even when one side has no chord at all (Clone
// Hero's 0x5DE030 takes that side unbounded; D30 does not copy it). A later
// fill that picks the same chord replaces the earlier one's length.
//
// The window is Clone Hero 1.1's: the whole ticks of resolution x slop, plus
// one tick (cvttsd2si truncates at 0x20D0088, then inc adds one at
// 0x20D008D), and a chord exactly at its edge still lands (0x5DE17D; D22).
// The +1 sits on top of the rules value, so a hydra_rules.ini slop of 0 still
// lands a chord one tick late.
void place_authored_fills(Song& song, const std::vector<AuthoredFill>& fills,
                          double slop_beats) {
    if (fills.empty() || song.sequence.empty()) return;
    // The chord list in tick order (a .chart need not be sorted).
    std::vector<std::pair<int64_t, size_t>> order;
    order.reserve(song.sequence.size());
    for (size_t i = 0; i < song.sequence.size(); ++i)
        order.emplace_back(song.sequence[i].timecode.ticks(), i);
    std::stable_sort(order.begin(), order.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
    const int64_t window_ticks =
        static_cast<int64_t>(song.tick_resolution() * slop_beats) + 1;

    for (const AuthoredFill& f : fills) {
        const auto it = std::upper_bound(
            order.begin(), order.end(), f.end,
            [](int64_t t, const std::pair<int64_t, size_t>& c) { return t < c.first; });
        const std::pair<int64_t, size_t>* after = it != order.end() ? &*it : nullptr;
        const std::pair<int64_t, size_t>* before = it != order.begin() ? &*(it - 1) : nullptr;
        if (before && before->first < f.start) before = nullptr;
        if (after && after->first > f.end + window_ticks) after = nullptr;

        const std::pair<int64_t, size_t>* pick = after ? after : before;
        if (before && after)
            pick = (after->first - f.end) <= (f.end - before->first) ? after : before;
        if (pick) apply_fill_end(song, pick->second, f.start);
    }
}
```

The prototype wrote the same choice as a ladder of eight cases with a B2 switch. With B2 off, every case reduces to "drop a candidate that is out of bounds, then take the closer of what is left", which is what this code says. The prototype's measurement is the check that the two agree.

`song.cpp` already includes `<algorithm>` (line 3), and `std::vector` and `std::pair` already appear throughout the file.

- [ ] **Step 11: The `.mid` parser records fills whole.** Line 346 drops `ApplyFill`:

```cpp
    None, Note, FillStart, StoreFillEnd, SpStart, SpEnd, Tom, Flam,
```

Lines 357-358 (the `MOp::tick` comment) become one line, `int64_t tick = 0;      // FillStart/StoreFillEnd/SpStart/Tempo/TimeSig`. Lines 450-455 become:

```cpp
    void op_fillstart(int64_t tick) { fill_start_tick_ = tick; }
    // The fill marker's note-off: the fill is recorded whole and placed once
    // every chord is read (place_authored_fills). A note-off with no fill
    // open records nothing.
    void op_store_fillend(int64_t tick) {
        if (fill_start_tick_) fills_.push_back({*fill_start_tick_, tick});
        fill_start_tick_.reset();
    }
```

Lines 490-491 become:

```cpp
    std::optional<int64_t> fill_start_tick_;
    std::vector<AuthoredFill> fills_;  // every authored fill, placed after pass 2
```

Delete line 599 (`case MAct::ApplyFill: op_apply_fill(op.tick); break;`). Delete lines 648-659 (the "Activation fill placement." block in `push_timestamp`) and the blank line after it. After line 704 (`dynamics_enabled_ = false;`) add:

```cpp
        fill_start_tick_.reset();
        fills_.clear();
```

After line 712 (`push_timestamp(elapsed);`) add:

```cpp
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats);
```

- [ ] **Step 12: The `.chart` parser records fills whole.** Line 881 drops `FillEnd`: `    SpStart, SpEnd, FillStart,`. Line 889 becomes `int64_t a = 0;   // Tempo/TimeSig/SpStart/FillStart: tick; SpEnd: start`. Lines 942-946 become:

```cpp
    // An S 64 line: the fill is recorded whole and placed once every chord
    // is read (place_authored_fills).
    void op_fillstart(int64_t start, int64_t end) { fills_.push_back({start, end}); }
```

Lines 974-975 become `    std::vector<AuthoredFill> fills_;  // every authored fill, placed after the section`. Delete line 1120 (`case CAct::FillEnd: op_fillend(op.a); break;`). Delete lines 1154-1163 (the "Phrase end: activation fill." block) and the blank line after it. After the loop at lines 1230-1231, still inside the `if (ed_it != sections_.end())` block, add:

```cpp
        place_authored_fills(song, fills_, rules_.fill_land_slop_beats);
```

- [ ] **Step 13: The rules comment names the new owner.** In `rules.h`, commit 1's comment begins "Authored-fill placement (fill_lands_on_chord, both parsers)". Change that name to `place_authored_fills`.

- [ ] **Step 14: Run everything.** `-tc="fill landing*"` (all eight cases pass), then the full `hydra_tests.exe`, `Status: SUCCESS!`. The prototype's full run passed with no existing test edited. `git grep -n "fill_lands_on_chord\|ApplyFill\|FillEnd\|fill_end_tick_" -- src` prints nothing.

- [ ] **Step 15: Commit 2's dump checks.** The lane corpus check again, at all four difficulties, files `t4b-$d-2x1.txt`; all four blocks print nothing. Then the Fill B candidates, compared with the B1 prototype's dumps:

```powershell
foreach ($d in 'expert', 'hard', 'medium', 'easy') {
    pwsh -NoProfile -File "$base\s2-scores.ps1" -Exe $exe -Diff $d -List "$FB\libB_$d.txt" -Out "$w\t4b-libB-$d.txt" -Work "$w\work"
    "== $d"; pwsh -NoProfile -File "$base\s2-compare.ps1" -Before "$FB\out_lib_B1_$d.txt" -After "$w\t4b-libB-$d.txt"
}
```

All four blocks print nothing. The B1 prototype is the +1 plus B1 on 1abd188, which is exactly this lane, so its dumps are the expected answer, including the Dirge Within, SexTon, Third Eye Blind, Oak Pantheon, In This Moment and We the Kings path changes. Finally, re-run Step 6's library-candidate loop with files `t4b-libcand-$d.txt`. It may differ from `$PF\out_libcand_fill_$d.txt` only on a chart that is also in `$FB\libB_$d.txt`; any other chart is a stop-and-report.

- [ ] **Step 16: Commit 2** `src/parse/song.cpp src/core/rules.h tests/test_s2_fill_landing.cpp` by name: "Each authored fill is placed on its own once every chord is read (finding 315, D30)", with the trailers.

**Done when:**
- [ ] `hydra_tests.exe` (full) reports `Status: SUCCESS!` in the worktree after each commit.
- [ ] Step 6's checks printed nothing at commit 1, and Step 15's printed nothing at commit 2.
- [ ] The diff from 1abd188 touches only the song.cpp lines the steps quote (the functions named under Files), the rules.h comment, the User Guide line, the new test file and one CMake line.

**Stamp needs (for T9):** the shared results stamp (step 1 Task 22's value; D23). The `hydra_rules.ini` fingerprint does not catch this, because the rules value is unchanged and only the formula and the placement move. Not `kDynamicsCountStamp`: fills never change which notes a chart has.

**Merge notes (textual neighbours, for the integrator).** Each neighbour gets one sentence.

T1 rewrites the disco helpers that end at song.cpp line 134, two lines above this task's first line, so git may report one conflict; keep T1's disco functions, and this task still removes `fill_lands_on_chord`.

T6 closes and reopens the anonymous namespace around `apply_timesig` (lines 153-170), between this task's two hunks at the top; keep T6's namespace lines, with this task's new functions after them.

Because of that T6 move, `AuthoredFill`, `apply_fill_end` and `place_authored_fills` must stay inside the reopened anonymous namespace.

T3 replaces `mark_sp_phrase_end` right after `apply_fill_end` (lines 180-189); keep both.

T3 edits `MidiParser::op_sp_end` at lines 457-461, one line below this task's fill ops at 450-455; keep both.

T3 and T5 also edit the `MOp::tick` comment (line 357) and `MidiParser::run`; the merged comment lists FillStart, StoreFillEnd, SpStart, SpEnd, Dynamics, Tempo and TimeSig, and has no ApplyFill.

T5 adds a `MidiParser` member after line 492, one line below this task's member edit at 490-491; keep both.

T5 adds `marks_before_tag_ = 0;` after line 704, next to this task's two reset lines; keep all three.

In `ChartParser`, T3 rewrites the SP end check just above this task's deleted block (lines 1148-1152); keep T3's check.

T3 adds its after-the-last-tick phrase close at the same spot as this task's `place_authored_fills` call (after line 1231); keep both, phrase close first.

T2 and T7 also edit `docs/UserGuide.md`, on different lines.

---


### Task T5: The dynamics tag in Clone Hero's exact spellings and file order

**Goal:** A `.mid` chart's dynamics tag counts only when Clone Hero would count it, takes effect exactly where Clone Hero's would, and the Dynamics tab says so when it came late.

**Findings:** 64.

**Decision:** D24: "The dynamics tag counts only in Clone Hero's two exact spellings, `ENABLE_CHART_DYNAMICS` and `[ENABLE_CHART_DYNAMICS]` (0x21557A5, 0x21557BB), and at a shared tick file order decides. ... When the tag comes after marked notes, the Dynamics tab says 'Dynamics enabled: from <time> on (N earlier markings ignored by Clone Hero)' (3 library charts); this adds two stored facts per difficulty (`kDynamicsBlobStamp`)."

**How it works today.** In a `.mid`, ghost and accent notes (velocity 1 and 127) only score as such after the chart turns dynamics on with a text event in the PART DRUMS track. Hydra already prices each note by whether the tag has been seen yet (`op_note`, line 469), and already ignores a tag in any other track. Three things differ from Clone Hero.

First, `is_dynamics_marker` (line 114) peels one optional bracket off each end, so `[ENABLE_CHART_DYNAMICS` with no closing bracket counts. Clone Hero compares the raw text to exactly two strings, with no trim (0x21557A5, 0x21557BB, loaded straight from the event at 0x2155689). One library chart spells it that way: Last Chance to Reason - Programmed for Battle.

Second, the tag is a `Pre` step (line 575). Hydra runs every `Pre` step at a tick before that tick's notes, so a note written before the tag at the same tick still gets priced. Clone Hero checks the flag at each note-on as it reads the file (0x21555F1), so file order decides. No library chart has a note before the tag at a shared tick today.

Third, the Dynamics tab says a flat "Dynamics enabled: yes" (dynamics_tab.cpp line 214) even when marked notes came before a late tag and were priced as plain. Three library charts do this at Expert.

**The change.** The tag matches the two exact strings. It moves from the `Pre` step into the `Notes` step, which runs a tick's messages in file order, so a note before the tag at the same tick stays plain. The parser counts the marked notes it reads before the tag. When there is at least one, it keeps the tag's tick and that count on the Song. `count_dynamics` turns them into two stored facts on the breakdown: the tag's time and the count. The tab reads them.

Think of the tag as a light switch halfway down a hallway. Clone Hero only flips it on for the two exact labels, and the notes before the switch stay in the dark. The tab now says where the switch was and how many marked notes it left in the dark.

**Ruling note (question Q3, approved in D31).** The rulings name "the tag's tick" as the stored fact. The Dynamics tab has no tempo map: it draws from the stored breakdown blob, often without parsing the chart. A tick alone can't become "4:01" there. So this draft keeps the tick on the Song, where the parser knows it, and stores the tag's time in milliseconds in the breakdown and its blob, converted once in `count_dynamics` from the Song's own timing. Still two facts per difficulty. D31 approves this reading: the tab stores the time in milliseconds so it can say "from m:ss".

**What the user sees.** The brief's measurements, checked by its verifier:
- Last Chance to Reason - Programmed for Battle (charter RAT KING) drops from 620,730 to 607,580 at Expert: accent points 2,550 to 0, ghost 50 to 0. Its best path stays "(No activations.)". The chart has no notes below Expert, so nothing else moves. Its Dynamics tab now reads "Dynamics enabled: no (markings ignored by Clone Hero)", and its ghost and accent counts become plain. Caveat from D24: this assumes Clone Hero's MIDI reader does not trim the text before 0x2155050.
- Glass Cloud - Lilac (charter AbstractOrigin) writes the tag with spaces around it. Both programs already ignore it. No change.
- Three charts get the new tab line at Expert: Arsonists Get All The Girls - To Playact In Static (142 earlier markings), Pearl Jam - Alive (Rh) (74), and Scarve - Asphyxiate (8). Their scores do not change, because those notes are already priced as plain today.
- Two more charts have a late tag but no marked note before it. They keep "Dynamics enabled: yes".
- The test corpus does not change.

**Wording.** With N marked notes before the tag, the line is "Dynamics enabled: from <m:ss> on (N earlier markings ignored by Clone Hero)". The time uses `format_duration`, the app's one m:ss formatter, which rounds to the nearest second. N uses `count_label`, which groups thousands and says "1 earlier marking" for one. No library chart has N = 1; the singular is question Q4, approved in D31. With no late tag the line is today's "yes", and with no tag at all it is today's "no" line.

**Files.** Each step below quotes the lines it replaces.

The parser change is in `src/parse/song.cpp`. The marker comment and `is_dynamics_marker` name the two exact spellings. The tag's op moves from the `Pre` step to the `Notes` step, and `MidiParser::run` and the `MOp::tick` comment follow. `op_note` counts the marked notes read before the tag, in a new member beside `dynamics_enabled_` that pass 2 of `parse` resets. `op_enable_dynamics` stores the tag's tick and that count on the Song, which gains two fields in `src/parse/song.h`.

The stored facts live in `src/app/dynamics_breakdown.h` and `.cpp`. `count_dynamics` turns the tick into milliseconds, and the blob grows by two fields (its size, layout comment, encoder and decoder).

The display is in the UI. `src/ui/details_parts.h` declares `dynamics_enabled_text`. `src/ui/dynamics_tab.cpp` uses it for the Chart line and includes `ui/library_parts.h` for `format_duration` and `count_label`. That header's opening comment names the Dynamics tab as their second user.

The tests go in the new `tests/test_s2_dynamics_tag.cpp`, plus one line in `CMakeLists.txt`. One existing test file pins the old rule: `tests/test_song.cpp` drops its dynamics regex oracle and says why, and its marker test expects the two exact spellings (line 694's `"[ENABLE_CHART_DYNAMICS"` is the pin that breaks).

**Acceptance Criteria:**
- [ ] Only `ENABLE_CHART_DYNAMICS` and `[ENABLE_CHART_DYNAMICS]` turn dynamics on. One bracket, doubled brackets, spaces or lower case do not.
- [ ] At a shared tick, a note written before the tag is plain and a note written after it is priced.
- [ ] A tag in the EVENTS track does nothing.
- [ ] After a late tag, the Song holds the tag's tick and the count of marked notes before it. With no marked note before the tag, or no tag, it holds neither.
- [ ] The breakdown carries the tag's time in ms and the count, and the blob round-trips both. A blob in the old 110-byte layout reads as missing.
- [ ] The tab line reads as in "Wording" for each case.
- [ ] Programmed for Battle scores 607,580 at Expert. The corpus does not change.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="dynamics tag*"` gives `Status: SUCCESS!`, then the full run, `hydra_uitest --all --jobs 4`, and the checks in "Done when".

**Steps:**

- [ ] **Step 1: Failing tests.** Create `tests/test_s2_dynamics_tag.cpp`. The `.mid` deltas are written as literal bytes, as test_song.cpp does (240 ticks is `0x81 0x70`, 480 is `0x83 0x60`).

```cpp
// Step 2, T5 (finding 64, D24): the .mid dynamics tag counts only in
// Clone Hero 1.1's two exact spellings (0x21557A5, 0x21557BB, no trim), takes
// effect in file order at a shared tick (the flag is read at each note-on,
// 0x21555F1), and a late tag is stored so the Dynamics tab can say how many
// earlier markings Clone Hero ignored.

#include "doctest.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "env_util.h"
#include "midi_util.h"
#include "parse/song.h"
#include "ui/details_parts.h"

using namespace hydra;
using namespace testmidi;

namespace {

// `ev` (a delta-0 event from midi_util.h) moved later by `delta`.
std::vector<uint8_t> after(std::vector<uint8_t> delta, std::vector<uint8_t> ev) {
    ev.erase(ev.begin());
    delta.insert(delta.end(), ev.begin(), ev.end());
    return delta;
}

Song drums(std::vector<std::vector<uint8_t>> events) {
    events.insert(events.begin(), {track_name("PART DRUMS"), set_tempo()});
    events.push_back(end_of_track());
    return load_songbytes_mid(smf(concat(events)), true, true);
}

NoteDynamicType red(const Song& song, size_t i) {
    return song.sequence.at(i).chord.at(NoteColor::Red)->dynamictype;
}

constexpr const char* kTag = "[ENABLE_CHART_DYNAMICS]";

}  // namespace

TEST_CASE("dynamics tag: only Clone Hero's two exact spellings count") {
    for (const char* text : {"ENABLE_CHART_DYNAMICS", "[ENABLE_CHART_DYNAMICS]"}) {
        CAPTURE(text);
        const Song song = drums({text_event(text), note_on(97, 127)});
        CHECK(song.dynamics_enabled);
        CHECK(red(song, 0) == NoteDynamicType::Accent);
    }
    for (const char* text :
         {"[ENABLE_CHART_DYNAMICS", "ENABLE_CHART_DYNAMICS]", "[[ENABLE_CHART_DYNAMICS]]",
          " [ENABLE_CHART_DYNAMICS] ", "[enable_chart_dynamics]", "ENABLE_CHART_DYNAMICS "}) {
        CAPTURE(text);
        const Song song = drums({text_event(text), note_on(97, 127)});
        CHECK_FALSE(song.dynamics_enabled);
        CHECK(red(song, 0) == NoteDynamicType::Normal);
    }
}

TEST_CASE("dynamics tag: at a shared tick, file order decides") {
    SUBCASE("tag first: the note is priced") {
        const Song song = drums({text_event(kTag), note_on(97, 127)});
        CHECK(red(song, 0) == NoteDynamicType::Accent);
        CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
        CHECK(song.dynamics_marks_before_tag == 0);
    }
    SUBCASE("note first: the note stays plain, and counts as an earlier marking") {
        const Song song = drums({note_on(97, 127), text_event(kTag),
                                 after({0x83, 0x60}, note_on(97, 127))});  // tick 480
        CHECK(red(song, 0) == NoteDynamicType::Normal);
        CHECK(red(song, 1) == NoteDynamicType::Accent);
        REQUIRE(song.dynamics_late_tag_tick.has_value());
        CHECK(*song.dynamics_late_tag_tick == 0);
        CHECK(song.dynamics_marks_before_tag == 1);
    }
}

TEST_CASE("dynamics tag: a late tag keeps its tick and the earlier markings") {
    const Song song = drums({
        note_on(97, 127),                         // tick 0: red, accent velocity
        after({0x81, 0x70}, note_on(98, 1)),      // tick 240: yellow, ghost velocity
        after({0x81, 0x70}, text_event(kTag)),    // tick 480: the tag
        after({0x83, 0x60}, note_on(97, 127)),    // tick 960: red, accent velocity
    });
    CHECK(song.dynamics_enabled);
    CHECK(red(song, 0) == NoteDynamicType::Normal);
    CHECK(song.sequence.at(1).chord.at(NoteColor::Yellow)->dynamictype ==
          NoteDynamicType::Normal);
    CHECK(red(song, 2) == NoteDynamicType::Accent);
    REQUIRE(song.dynamics_late_tag_tick.has_value());
    CHECK(*song.dynamics_late_tag_tick == 480);
    CHECK(song.dynamics_marks_before_tag == 2);
}

TEST_CASE("dynamics tag: a late tag after only plain notes is not stored") {
    const Song song = drums({note_on(97, 100), after({0x83, 0x60}, text_event(kTag)),
                             after({0x83, 0x60}, note_on(97, 127))});
    CHECK(song.dynamics_enabled);
    CHECK(red(song, 1) == NoteDynamicType::Accent);
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a second tag changes nothing") {
    const Song song = drums({text_event(kTag), note_on(97, 127),
                             after({0x83, 0x60}, text_event(kTag)),
                             after({0x83, 0x60}, note_on(97, 127))});
    CHECK_FALSE(song.dynamics_late_tag_tick.has_value());
    CHECK(song.dynamics_marks_before_tag == 0);
}

TEST_CASE("dynamics tag: a tag in the EVENTS track does nothing") {
    // Two tracks: EVENTS (with the tempo and the tag), then PART DRUMS.
    const std::vector<uint8_t> events =
        concat({track_name("EVENTS"), set_tempo(), text_event(kTag), end_of_track()});
    const std::vector<uint8_t> part =
        concat({track_name("PART DRUMS"), note_on(97, 127), end_of_track()});
    std::vector<uint8_t> file = {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1, 0, 2, 0x01, 0xE0};
    for (const std::vector<uint8_t>* t : {&events, &part}) {
        file.insert(file.end(), {'M', 'T', 'r', 'k'});
        const uint32_t n = static_cast<uint32_t>(t->size());
        for (int shift = 24; shift >= 0; shift -= 8)
            file.push_back(static_cast<uint8_t>(n >> shift));
        file.insert(file.end(), t->begin(), t->end());
    }
    const Song song = load_songbytes_mid(file, true, true);
    CHECK_FALSE(song.dynamics_enabled);
    CHECK(red(song, 0) == NoteDynamicType::Normal);
}

TEST_CASE("dynamics tag: the breakdown stores the tag's time and the count") {
    const Song song = drums({note_on(97, 127), after({0x83, 0x60}, text_event(kTag)),
                             note_on(97, 127)});  // tag and a priced note at tick 480
    const app::DynamicsBreakdown bd = app::count_dynamics(song);
    REQUIRE(bd.late_tag_ms.has_value());
    CHECK(*bd.late_tag_ms == 500);  // tick 480 is one beat at 120 BPM
    CHECK(bd.marks_before_tag == 1);

    std::vector<uint8_t> blob = app::encode_dynamics(bd);
    const auto back = app::decode_dynamics(blob);
    REQUIRE(back.has_value());
    CHECK(back->late_tag_ms == bd.late_tag_ms);
    CHECK(back->marks_before_tag == 1);

    app::DynamicsBreakdown none;
    const auto none_back = app::decode_dynamics(app::encode_dynamics(none));
    REQUIRE(none_back.has_value());
    CHECK_FALSE(none_back->late_tag_ms.has_value());
    CHECK(none_back->marks_before_tag == 0);

    blob.resize(110);  // the old layout's length: read as missing, then recounted
    CHECK_FALSE(app::decode_dynamics(blob).has_value());
}

TEST_CASE("dynamics tag: the Dynamics tab's Chart line") {
    using ui::detail::dynamics_enabled_text;
    app::DynamicsBreakdown bd;
    CHECK(dynamics_enabled_text(bd) == "Dynamics enabled: no (markings ignored by Clone Hero)");
    bd.dynamics_enabled = true;
    CHECK(dynamics_enabled_text(bd) == "Dynamics enabled: yes");
    bd.late_tag_ms = 241'000;
    bd.marks_before_tag = 142;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (142 earlier markings ignored by Clone Hero)");
    bd.marks_before_tag = 1;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (1 earlier marking ignored by Clone Hero)");
    bd.marks_before_tag = 1234;
    CHECK(dynamics_enabled_text(bd) ==
          "Dynamics enabled: from 4:01 on (1,234 earlier markings ignored by Clone Hero)");
}

// Dev aid, not a pinned case. Set HYDRA_DYNAMICS_TAG_CHARTS to a list of
// notes.mid paths separated by ';', then run -tc="dynamics tag dev aid*" to
// print each chart's Expert Pro Drums Chart line as the tab would show it.
TEST_CASE("dynamics tag dev aid: print the Chart line for real charts") {
    const auto list = read_env("HYDRA_DYNAMICS_TAG_CHARTS");
    if (!list) return;
    size_t from = 0;
    while (from <= list->size()) {
        const size_t to = std::min(list->find(';', from), list->size());
        const std::string path = list->substr(from, to - from);
        from = to + 1;
        if (path.empty()) continue;
        const Song song = load_songpath_mid(path, true, app::kDynamicsParseBass2x);
        const app::DynamicsBreakdown bd = app::count_dynamics(song);
        std::printf("%s\n  %s (tag tick %lld)\n", path.c_str(),
                    ui::detail::dynamics_enabled_text(bd).c_str(),
                    static_cast<long long>(song.dynamics_late_tag_tick.value_or(-1)));
    }
}
```

Add `    tests/test_s2_dynamics_tag.cpp` at the end of the `hydra_tests` list in `CMakeLists.txt`, after line 429.

- [ ] **Step 2: Build and watch it fail to compile.** `.\build_cpp.ps1 -Target hydra_tests`. The new fields and `dynamics_enabled_text` don't exist yet, so the build stops on this file. That is the expected red. To see the behavioural failures before writing the fields, comment out the three cases that use them and run `-tc="dynamics tag*"`: the one-bracket spellings (`[ENABLE_CHART_DYNAMICS` and `ENABLE_CHART_DYNAMICS]`) turn dynamics on, and the note-first subcase prices the note. Then restore the cases.

Some cases already pass on today's code, and that is expected: they pin behaviour Hydra already shares with Clone Hero, so a later change can't lose it. "A tag in the EVENTS track does nothing" passes today, because Hydra already reads the tag only in PART DRUMS. "A second tag changes nothing" passes once the fields exist, because today's tag op already does nothing when dynamics is on. In the spellings case, the two correct spellings and the doubled-bracket, spaced and lower-case forms already behave as the test says; only the two one-bracket forms are red. "A late tag after only plain notes" passes once the fields exist with their empty defaults; it guards against storing a tag that left no marking in the dark. The cases that must start red are the one-bracket spellings, the note-first subcase, the late-tag tick and count, the breakdown and blob case, and the Chart line.

- [ ] **Step 3: The exact spellings.** song.cpp lines 89-97 today begin "The three text markers both formats read, matched by hand. Each one is exactly the whole-string regex it replaced (named beside it)." Change the first two sentences to: "The disco markers both formats read are matched by hand, each exactly the whole-string regex it replaced (named beside it). The dynamics marker is not a regex any more: it is Clone Hero's two exact strings (finding 64)." Keep the rest of the block as is.

Lines 113-116 today read:

```cpp
// \[?ENABLE_CHART_DYNAMICS\]?
bool is_dynamics_marker(std::string_view s) {
    return peel_brackets(s) == "ENABLE_CHART_DYNAMICS";
}
```

Replace them with:

```cpp
// Clone Hero 1.1 turns dynamics on only when a PART DRUMS text event is
// exactly one of these two strings (String.op_Equality at 0x21557A5 and
// 0x21557BB, on the raw text with no trim). One bracket, extra brackets or
// spaces do nothing (finding 64, D24).
bool is_dynamics_marker(std::string_view s) {
    return s == "ENABLE_CHART_DYNAMICS" || s == "[ENABLE_CHART_DYNAMICS]";
}
```

`peel_brackets` stays; the disco markers still use it, and T1 owns them.

- [ ] **Step 4: File order, and counting the earlier markings.** Line 575 today reads `if (is_dynamics_marker(t)) return mop(MPhase::Pre, MAct::Dynamics);`. Replace it with:

```cpp
        // The tag runs with the notes, in file order, so a note written
        // before it at the same tick stays plain, as in Clone Hero, which
        // reads the flag at each note-on (0x21555F1).
        if (is_dynamics_marker(t)) return mop_tick(MPhase::Notes, MAct::Dynamics, tick);
```

Add `Dynamics` to the `MOp::tick` comment at line 357. Line 605 becomes `case MAct::Dynamics: op_enable_dynamics(op.tick); break;`. Line 442, `void op_enable_dynamics() { dynamics_enabled_ = true; }`, becomes:

```cpp
    void op_enable_dynamics(int64_t tick) {
        if (dynamics_enabled_) return;  // a second tag changes nothing
        dynamics_enabled_ = true;
        if (marks_before_tag_ > 0) {
            song_->dynamics_late_tag_tick = tick;
            song_->dynamics_marks_before_tag = marks_before_tag_;
        }
    }
```

Lines 467-473 (`op_note`) gain one line after the note is added, so a note that failed to add (a duplicate) is not counted:

```cpp
    void op_note(NoteColor color, NoteDynamicType dyn, bool is2x) {
        ChordNote& note = chord_.add_note(color);  // may throw ChartFileError
        note.dynamictype = dynamics_enabled_ ? dyn : NoteDynamicType::Normal;
        // A ghost or accent velocity before the tag: Clone Hero prices it as
        // plain, and the Dynamics tab reports how many there were.
        if (!dynamics_enabled_ && dyn != NoteDynamicType::Normal) ++marks_before_tag_;
        if (allows_cymbals(color) && mode_pro_)
            note.cymbaltype = flag_cymbals_[static_cast<int>(color) - 1];
        note.is2x = is2x;
    }
```

After line 492 (`bool dynamics_enabled_ = false;`) add `int marks_before_tag_ = 0;  // ghost/accent velocities read before the tag`. After line 704 (`dynamics_enabled_ = false;`) add `marks_before_tag_ = 0;`.

- [ ] **Step 5: The Song fields.** After song.h line 131 (`bool dynamics_enabled = false;`) add:

```cpp
    // .mid only (finding 64, D24). When the drum track's dynamics tag came
    // after at least one ghost- or accent-velocity note, the tag's tick and
    // how many such notes came before it (Clone Hero prices them as plain).
    // nullopt and 0 when the tag came before every marked note, or there is
    // no tag (dynamics_enabled says which). count_dynamics stores them.
    std::optional<int64_t> dynamics_late_tag_tick;
    int dynamics_marks_before_tag = 0;
```

- [ ] **Step 6: The breakdown's two stored facts.** In `dynamics_breakdown.h`, after line 45 (`bool dynamics_enabled = false;`) add:

```cpp
    // A late .mid dynamics tag (finding 64): its time in chart ms, and how
    // many marked notes came before it. nullopt and 0 when the tag came first
    // or there is none. The time is stored because the Dynamics tab draws
    // from the stored blob and has no tempo map to turn a tick into m:ss.
    std::optional<uint32_t> late_tag_ms;
    int marks_before_tag = 0;
```

Lines 57-59 (the layout comment) become: "Version byte, then dynamics_enabled (1 byte), then the nine rows in DynamicsRow order, each as ghost/accent/normal, then the late tag's ms (0xFFFFFFFF when none) and the count of markings before it. Every number is a little-endian uint32."

In `dynamics_breakdown.cpp`, `count_dynamics` (lines 104-106) gains, after `bd.dynamics_enabled = song.dynamics_enabled;`:

```cpp
    if (song.dynamics_late_tag_tick)
        bd.late_tag_ms = static_cast<uint32_t>(song.timecode(*song.dynamics_late_tag_tick).ms());
    bd.marks_before_tag = song.dynamics_marks_before_tag;
```

Lines 125-127 become:

```cpp
constexpr size_t kRowCount = static_cast<size_t>(DynamicsRow::Count);  // 9
// version(1) + dynamics_enabled(1) + 9 rows * 3 fields * 4 bytes
// + late tag ms(4) + marks before it(4) = 118. The old layout was 110 bytes;
// decode reads it as missing, so the tab recounts.
constexpr size_t kDynamicsBlobSize = 1 + 1 + kRowCount * 3 * 4 + 4 + 4;
constexpr uint32_t kNoLateTag = 0xFFFFFFFF;
```

In `encode_dynamics`, after the row loop (line 154):

```cpp
    write_u32_le(out, b.late_tag_ms.value_or(kNoLateTag));
    write_u32_le(out, static_cast<uint32_t>(b.marks_before_tag));
```

In `decode_dynamics`, after the row loop (line 169):

```cpp
    const uint32_t tag_ms = read_u32_le(p);                        p += 4;
    if (tag_ms != kNoLateTag) b.late_tag_ms = tag_ms;
    b.marks_before_tag = static_cast<int>(read_u32_le(p));
```

The size check at line 159 already rejects a blob shorter than the new size, so a 110-byte blob reads as missing even before T9 bumps the blob stamp.

- [ ] **Step 7: The tab line.** In `src/ui/details_parts.h`, under `// dynamics_tab.cpp.` (line 35), add:

```cpp
// The Chart section's "Dynamics enabled" line for a stored breakdown.
std::string dynamics_enabled_text(const app::DynamicsBreakdown& bd);
```

with `#include <string>` and `#include "app/dynamics_breakdown.h"` at the top. In `dynamics_tab.cpp`, add `#include "ui/library_parts.h"` (for `format_duration` and `count_label`), define the function inside `namespace detail {` before `render_dynamics_panel`:

```cpp
std::string dynamics_enabled_text(const app::DynamicsBreakdown& bd) {
    if (!bd.dynamics_enabled) return "Dynamics enabled: no (markings ignored by Clone Hero)";
    if (!bd.late_tag_ms) return "Dynamics enabled: yes";
    return "Dynamics enabled: from " + format_duration(*bd.late_tag_ms / 1000.0) + " on (" +
           count_label(bd.marks_before_tag, "earlier marking", "earlier markings") +
           " ignored by Clone Hero)";
}
```

and replace lines 213-216:

```cpp
    if (bd.dynamics_enabled)
        ImGui::TextWrapped("Dynamics enabled: yes");
    else
        ImGui::TextWrapped("Dynamics enabled: no (markings ignored by Clone Hero)");
```

with:

```cpp
    ImGui::TextWrapped("%s", dynamics_enabled_text(bd).c_str());
```

`library_parts.h` lines 1-2 say only the library files include it. Add one line there: "The Dynamics tab includes it too, for format_duration and count_label."

- [ ] **Step 8: The pinned regex test.** In `tests/test_song.cpp`, delete `oracle_dynamics()` (lines 590-593). Change the sentence at line 583 to: "The parsers used std::regex for the disco-flip markers and the .chart section header, and still match them exactly as those regexes did. The dynamics tag is now Clone Hero's two exact strings (finding 64)." At line 698, replace `const bool want = std::regex_match(latin1_to_utf8(marker), oracle_dynamics());` with:

```cpp
            // Clone Hero's two exact strings (finding 64, D24).
            const std::string read = latin1_to_utf8(marker);
            const bool want = read == "ENABLE_CHART_DYNAMICS" || read == "[ENABLE_CHART_DYNAMICS]";
```

Leave the disco half of that test alone; T1 owns it.

- [ ] **Step 9: Run everything.** `-tc="dynamics tag*"`, then the full `hydra_tests.exe`, both `Status: SUCCESS!`. Then `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4`. The two existing pins, `uitest_details.cpp:166` ("Dynamics enabled: yes") and `:613` (the "no" line), must still pass unchanged: neither of their charts has a late tag.

- [ ] **Step 10: Commit** `src/parse/song.cpp src/parse/song.h src/app/dynamics_breakdown.h src/app/dynamics_breakdown.cpp src/ui/details_parts.h src/ui/dynamics_tab.cpp src/ui/library_parts.h tests/test_song.cpp tests/test_s2_dynamics_tag.cpp CMakeLists.txt` by name: "Dynamics tag: Clone Hero's exact spellings, file order, late-tag line (finding 64, D24)", with the trailers.

**Done when:**
- [ ] `hydra_tests.exe` (full) reports `Status: SUCCESS!`, and `hydra_uitest --all --jobs 4` passes, in the worktree.
- [ ] The corpus is unchanged: the lane corpus check from "Before the wave" at all four difficulties (`$lane` the s2-t5 worktree, `$w = "$sp\s2-t5"`, files `t5-$d-2x1.txt`) prints nothing.
- [ ] Programmed for Battle now scores what Clone Hero can pay. Write `$SM\charts\pfb_asis\notes.mid` into a one-line list file and dump it at Expert with `$base\s2-scores.ps1 -List`. The line reads `(No activations.)=607580`. The verifier's no-tag dump (`$sp\s2-verify-small\work\c_notag.json`) has the same total, 607,580.
- [ ] The three late-tag charts show the new line with the brief's counts. Run the dev aid:

```powershell
$env:HYDRA_DYNAMICS_TAG_CHARTS = @(
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\YaBoiBenimaru\Arsonists Get All The Girls\Arsonists Get All The Girls - Portals (2009)\Arsonists Get All The Girls - To Playact In Static\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\MMSRhino\Pearl Jam - Alive (Rh)\notes.mid",
  "C:\Clone Hero\songs\synchotic\Sync Charts\BirdmanExe Drive\YaBoiBenimaru\Scarve\Scarve - Irradiant (2004)\Scarve - Asphyxiate\notes.mid"
) -join ';'
.\build-cpp\Release\hydra_tests.exe -tc="dynamics tag dev aid*"
Remove-Item Env:HYDRA_DYNAMICS_TAG_CHARTS
```

  Expect "from <m:ss> on (142 earlier markings ...)" with tag tick 241440, "(74 ...)" with tick 214292, and "(8 ...)" with tick 103680. The counts come from the brief's Python scan, not from Hydra's parser. If Hydra's count differs, report the difference and its cause (for example 2x kicks at pitch 95, or duplicate notes) before landing; do not adjust the code to match the scan.
- [ ] The scores of those three charts do not change (their early marked notes are plain today too). Dump them at Expert with `s2-scores.ps1 -List`, once with this build and once with `$baseBin\hydra_replay.exe`; `s2-compare.ps1` prints nothing.
- [ ] The diff touches only the lines the steps quote, in the files named under Files.

**Stamp needs (for T9):** `kDynamicsCountStamp` (the one-bracket chart's counts change; T9 bumps it once for T1, T2 and T5). `kDynamicsBlobStamp` (the blob gains two fields; 1 to 2). The shared results stamp (one chart's score changes; step 1 Task 22's value). T9 also needs to update `tests/test_dynamics_store.cpp:87-93`: that case stamps a blob "version 2" to prove an unknown version is rejected, and 2 becomes the current stamp. It should use `kDynamicsBlobStamp.written + 1` instead.

**Merge notes (textual neighbours, for the integrator).** Each neighbour gets one sentence.

T3 edits the same `MOp::tick` comment (line 357) and the same `run` switch (its line 601, this task's 605); keep both.

T1 changes the disco lines right below line 575 (`is_disco_on_marker` and `is_disco_off_marker` at 576-577); keep T1's disco lines and this task's tag line.

T4 edits `MidiParser`'s members at lines 490-491, one line above this task's new member after line 492; keep both.

T4 adds two reset lines after line 704, where this task adds `marks_before_tag_ = 0;`; keep all three.

T4 also edits the `MOp::tick` comment and `run`; keep both edits.

T2 edits `dynamics_breakdown.cpp` and `.h` too (`kicks_total`, `played_total`), in different functions from this task, but the header hunks sit close together; keep both.

T2 edits the "All kicks" and Totals lines in `dynamics_tab.cpp` (around lines 150-209), above this task's Chart line at 211-216; keep both.

---

### Task T6: Small parser owners

*Decision: D27. Findings: 331, R7.5, R7.7, 258, 319, 61, 60.*

**Goal:** Six small rules each get one owner. A stray `.chart` modifier is skipped, a malformed offset or delay is absent, `.mid` sections arrive sorted, the default 4/4 is written once, a container's notes entry is found by one name rule, and a blank artist or charter shows the placeholder.

**What you'll see change:** 56 library charts with a blank `charter =` show `<unknown charter>` in the library and, after their next analysis, in reports. Three `.mid` charts with two EVENTS tracks (Rush - Middletown Dreams, Rush - Good News First, Koseki Bijou - Kyoumen no Nami (YUKiRA cover)) now name the right practice section in the Preview time box. A malformed chart that failed with "apply_cymbal: note not present" now loads. A `.chart` `Offset = 500ms` no longer moves the Preview audio by eight minutes. No library chart has either of those last two problems. No score changes anywhere.

**Stamps:** none. Artist and charter are metadata applied after the rescan cache, sections are not stored, and the other rules only touch charts that fail or misread today.

The six owners are independent. They run in one worktree, `.claude/worktrees/s2-t6` on `claude/s2-t6`, branched from `claude/s1-t3`, one commit each, in the order below. All new C++ tests go in one new file, `tests/test_s2_parser_owners.cpp`, with test names starting `s2 owners:`.

**Shared-file note for the integrator.** In `src/parse/song.cpp` this task touches seven spots, and each step quotes its lines. They are the anonymous-namespace boundary around `apply_timesig`, a new helper after `section_name_of`, the area after `title_or_unknown`, MidiParser pass 3, the ChartParser `Offset` block, the ChartParser section sort, and `load_songbytes_srb`.

Its neighbours, one sentence each:

T4 removes `fill_lands_on_chord` (lines 136-149) just above this task's namespace boundary at 153-170; keep both.

T4 replaces `apply_fill_end` (lines 171-178) just below that boundary, and T4's new functions stay inside the reopened anonymous namespace.

T3 edits `mark_sp_phrase_end` (lines 180-189), a few lines further down; keep both.

T4 and T5 edit pass 2 of `MidiParser::parse` (lines 704-713), just above this task's pass 3 at 717-731; keep all of them.

T3, T4 and T1 edit the end of `ChartParser::parse`, near this task's section sort at 1247-1251; keep all of them.

Expect small textual merges at these spots, not logic conflicts.

**Step 0: Set up.** The worktree is `.claude/worktrees/s2-t6` on `claude/s2-t6` (see the execution graph), and the corpus baseline is the shared one from "Before the wave". Create `tests/test_s2_parser_owners.cpp` with the header below, and add `tests/test_s2_parser_owners.cpp` as one line at the end of the `hydra_tests` list in `CMakeLists.txt` (after line 429, `tests/test_long_paths.cpp`).

```cpp
// Step 2, task 6: the small parser owners (decision D27). Each rule below has
// one owner in the code, and each case pins what that owner answers.

#include "doctest.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "app/analysis.h"
#include "app/preview_source.h"
#include "app/preview_view.h"
#include "core/model.h"
#include "core/strutil.h"
#include "corpus_util.h"
#include "midi_util.h"
#include "miniz.h"
#include "parse/song.h"
#include "store/record_store.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

namespace {

// A .chart at 192 ticks a beat, 4/4 and 120 BPM, with `song_extra` lines in
// [Song] and `drums` lines in [ExpertDrums].
std::vector<uint8_t> chart_bytes(const std::string& song_extra, const std::string& drums) {
    const std::string s = "[Song]\n{\n  Resolution = 192\n" + song_extra + "}\n" +
                          "[SyncTrack]\n{\n  0 = TS 4\n  0 = B 120000\n}\n" +
                          "[ExpertDrums]\n{\n" + drums + "}\n";
    return std::vector<uint8_t>(s.begin(), s.end());
}

// A format-1 MIDI file, one MTrk chunk per track, 480 ticks a beat.
std::vector<uint8_t> smf_tracks(const std::vector<std::vector<uint8_t>>& tracks) {
    std::vector<uint8_t> d = {'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1,
                              0, static_cast<uint8_t>(tracks.size()), 0x01, 0xE0};
    for (const std::vector<uint8_t>& t : tracks) {
        d.insert(d.end(), {'M', 'T', 'r', 'k'});
        const uint32_t n = static_cast<uint32_t>(t.size());
        for (int shift : {24, 16, 8, 0}) d.push_back(static_cast<uint8_t>(n >> shift));
        d.insert(d.end(), t.begin(), t.end());
    }
    return d;
}

// A text meta event `vlq` ticks after the previous event (the delta already
// written as MIDI variable-length bytes).
std::vector<uint8_t> text_after(std::vector<uint8_t> vlq, const std::string& text) {
    vlq.insert(vlq.end(), {0xFF, 0x01, static_cast<uint8_t>(text.size())});
    vlq.insert(vlq.end(), text.begin(), text.end());
    return vlq;
}

// Raw deflate, no zlib header: what .srb streams use (as in test_srb.cpp).
std::vector<uint8_t> deflate_raw(const std::vector<uint8_t>& src) {
    size_t out_len = 0;
    void* p = tdefl_compress_mem_to_heap(src.data(), src.size(), &out_len,
                                         TDEFL_DEFAULT_MAX_PROBES);
    REQUIRE(p != nullptr);
    std::vector<uint8_t> out(static_cast<uint8_t*>(p), static_cast<uint8_t*>(p) + out_len);
    mz_free(p);
    return out;
}

void push_str(std::vector<uint8_t>& out, const std::string& s) {
    const uint32_t n = static_cast<uint32_t>(s.size());
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(n >> (8 * i)));
    out.insert(out.end(), s.begin(), s.end());
}

// An .srb whose metadata names its notes stream `notes_name` (layout in
// parse/srb.h): 16 header bytes, the deflated metadata, the deflated notes,
// then one stand-in audio stream.
std::vector<uint8_t> srb_with(const std::string& notes_name, const std::vector<uint8_t>& notes) {
    std::vector<uint8_t> meta = {'4', 'b', '4', 1};
    push_str(meta, notes_name);
    for (const char* s : {"Name", "Artist", "Album", "Genre", "Charter", "2026", "desc"})
        push_str(meta, s);
    for (int i = 0; i < 24; ++i) meta.push_back(static_cast<uint8_t>(i * 7));
    std::vector<uint8_t> out;
    for (int i = 0; i < 12; ++i) out.push_back(static_cast<uint8_t>(0xA0 + i));
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<uint8_t>(i == 0 ? 17 : 0));
    for (const std::vector<uint8_t>& stream :
         {deflate_raw(meta), deflate_raw(notes), deflate_raw(std::vector<uint8_t>(4096, 0x55))})
        out.insert(out.end(), stream.begin(), stream.end());
    return out;
}

}  // namespace
```

The srb helpers are a copy of `test_srb.cpp`'s fixture, because its copy lives in that file's anonymous namespace and the rulings keep this task's tests in their own file.

#### 6a. A `.chart` modifier with no note under it is skipped (finding 331)

Today `Chord::apply_cymbal`, `apply_ghost` and `apply_accent` throw `std::logic_error` when their note is missing (`src/core/model.cpp` lines 262-277). ChartParser runs each tick's ops inside `run_phase`, which catches only `ChartFileError` (`song.cpp` lines 1135-1143). So the logic error escapes and the whole chart fails with "apply_cymbal: note not present". Clone Hero skips such a marker and moves on to the next one (0x215DDB0-0x215DDC3).

The rulings offer two fixes: check for the note in `op_cymbal`, `op_ghost` and `op_accent`, or make the `Chord::apply_*` throw a `ChartFileError`. This task takes the second. The `.chart` parser already owns one rule for a malformed line, "drop that line, keep the chart", and it lives in exactly one place: the catch in `run_phase`. A duplicate note already reaches it the same way, because `Chord::add_note` throws `ChartFileError("Duplicate note.")` (model.cpp line 250). The comment on `ChartFileError` (model.h lines 34-35) already says "Chord and the parsers raise it; the parsers swallow it per-op". Checking in `op_*` would add a second skip rule beside that one. It would also edit `song.cpp` lines 958-960, a file six tasks share, where this choice edits only `model.cpp`. Nothing else calls `apply_*` with a missing note: the only other callers are tests that apply to notes they just added.

One gap stays open. Clone Hero's skip is shown statically for "no note at the marker's tick". The case where a chord sits at that tick but lacks that colour was not traced. Hydra skips both the same way. No library chart hits either case.

- [ ] **Step 1: Failing tests.** Add to `tests/test_s2_parser_owners.cpp`:

```cpp
TEST_CASE("s2 owners: Chord modifiers on a missing note are a chart-file error (331)") {
    Chord c;
    c.add_note(NoteColor::Red);
    CHECK_THROWS_AS(c.apply_cymbal(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_ghost(NoteColor::Yellow), ChartFileError);
    CHECK_THROWS_AS(c.apply_accent(NoteColor::Blue), ChartFileError);
    c.apply_ghost(NoteColor::Red);
    CHECK(c.at(NoteColor::Red)->is_ghost());
}

TEST_CASE("s2 owners: a .chart modifier with no note under it is skipped (331)") {
    // Tick 0: red alone, plus a yellow cymbal, a yellow ghost and a yellow
    // accent marker with no yellow note to change. Tick 384: a cymbal marker
    // with no chord at all. Tick 768: a yellow note whose cymbal marker is real.
    const std::string drums =
        "  0 = N 1 0\n  0 = N 66 0\n  0 = N 41 0\n  0 = N 35 0\n"
        "  384 = N 66 0\n"
        "  768 = N 2 0\n  768 = N 66 0\n";
    const std::vector<uint8_t> data = chart_bytes("", drums);
    REQUIRE_NOTHROW(load_songbytes_chart(data, true, true));
    const Song song = load_songbytes_chart(data, true, true);
    REQUIRE(song.sequence.size() == 2);
    CHECK(song.sequence[0].chord.code() == ".n...");
    CHECK(song.sequence[1].timecode.ticks() == 768);
    CHECK(song.sequence[1].chord.code() == "..N..");
}
```

- [ ] **Step 2: Build and watch both fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="s2 owners: *331*"`. The first fails because `std::logic_error` is not `ChartFileError`. The second fails with "apply_cymbal: note not present".

- [ ] **Step 3: The change.** Replace `model.cpp` lines 262-277:

```cpp
void Chord::apply_cymbal(NoteColor color) {
    // Python asserts the note is present; a modifier without its base note is
    // a malformed chart. Throw rather than deref an empty slot.
    if (!at(color)) throw std::logic_error("apply_cymbal: note not present");
    at(color)->cymbaltype = NoteCymbalType::Cymbal;
}

void Chord::apply_ghost(NoteColor color) {
    if (!at(color)) throw std::logic_error("apply_ghost: note not present");
    at(color)->dynamictype = NoteDynamicType::Ghost;
}

void Chord::apply_accent(NoteColor color) {
    if (!at(color)) throw std::logic_error("apply_accent: note not present");
    at(color)->dynamictype = NoteDynamicType::Accent;
}
```

with:

```cpp
// A cymbal, ghost or accent marker with no note of its colour under it is a
// malformed line, like a duplicate note. It raises ChartFileError, which the
// .chart parser's per-op catch drops, so the rest of the chart loads. Clone
// Hero skips such a marker too (0x215DDB0).
void Chord::apply_cymbal(NoteColor color) {
    if (!at(color)) throw ChartFileError("cymbal marker with no note under it");
    at(color)->cymbaltype = NoteCymbalType::Cymbal;
}

void Chord::apply_ghost(NoteColor color) {
    if (!at(color)) throw ChartFileError("ghost marker with no note under it");
    at(color)->dynamictype = NoteDynamicType::Ghost;
}

void Chord::apply_accent(NoteColor color) {
    if (!at(color)) throw ChartFileError("accent marker with no note under it");
    at(color)->dynamictype = NoteDynamicType::Accent;
}
```

In `model.h` lines 146-154, add one comment line above `void apply_cymbal(NoteColor color);`: `// Each raises ChartFileError when the colour has no note (a stray marker).` If `<stdexcept>` is now unused in `model.cpp`, leave the include; other code in the file uses `std::out_of_range`.

- [ ] **Step 4: Run the full `hydra_tests`.** Expected `Status: SUCCESS!`.

- [ ] **Step 5: Commit** `src/core/model.cpp src/core/model.h tests/test_s2_parser_owners.cpp CMakeLists.txt` by name: "A .chart modifier with no note is skipped, as Clone Hero does (331)".

#### 6b. One strict number helper for `.chart` Offset and song.ini delay (finding R7.5)

Today two readers parse the same kind of number two ways. `ChartParser::parse` reads `Offset` (song.cpp lines 1192-1206) with a strict integer first and then a bare `std::stod`, which keeps the leading number and drops the rest. So `Offset = 500ms` reads as 500 seconds. `parse_delay_ms` (`src/app/preview_source.cpp` lines 128-139) needs the whole text to be a number, so `delay = 250ms` is absent. Both accept `nan`, and a NaN delay wins over the Offset in `preview_audio_offset_ms`.

The fix is one helper in `core/strutil`, `parse_finite_number`. It trims spaces at both ends, allows one leading `+`, and needs the rest to be one finite decimal number. Anything else is absent. It uses `std::from_chars`, which ignores the locale, so a comma-decimal Windows locale can't change the answer. Both readers call it. No library file changes: the scan found 4,330 integer and 9 decimal Offsets and 11,716 integer delays, with no units, NaNs or infinities.

- [ ] **Step 1: Failing tests.**

```cpp
TEST_CASE("s2 owners: parse_finite_number reads only a whole finite number (R7.5)") {
    CHECK(parse_finite_number("0.25") == 0.25);
    CHECK(parse_finite_number(" -250 ") == -250.0);
    CHECK(parse_finite_number("+500") == 500.0);
    CHECK(parse_finite_number("1e3") == 1000.0);
    for (const char* bad : {"", "  ", "500ms", "0.25s", "nan", "NaN", "inf", "-inf",
                            "1e999", "soon", "+-5", "5 5", "0x1F4"}) {
        CAPTURE(bad);
        CHECK_FALSE(parse_finite_number(bad).has_value());
    }
}

TEST_CASE("s2 owners: a .chart Offset that is not a plain number is absent (R7.5)") {
    auto offset = [](const std::string& text) {
        return load_songbytes_chart(chart_bytes("  Offset = " + text + "\n", "  0 = N 1 0\n"),
                                    true, true)
            .chart_offset_s;
    };
    CHECK(offset("0.25") == 0.25);
    CHECK(offset("1") == 1.0);
    CHECK(offset("-0.5") == -0.5);
    CHECK_FALSE(offset("500ms").has_value());  // 500 seconds before this change
    CHECK_FALSE(offset("0.25s").has_value());
    CHECK_FALSE(offset("nan").has_value());
}

TEST_CASE("s2 owners: a song.ini delay that is not a plain number is absent (R7.5)") {
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() /
                         ("hydra_s2_delay_" + std::to_string(GetCurrentProcessId()));
    fs::create_directories(dir);
    const fs::path ini = dir / "song.ini";
    auto delay = [&](const std::string& value) {
        {
            std::ofstream f(ini, std::ios::binary | std::ios::trunc);
            f << "[song]\ndelay = " << value << "\n";
        }
        return app::read_ini_delay_ms(ini.u8string());
    };
    CHECK(delay("1016") == 1016.0);
    CHECK_FALSE(delay("nan").has_value());  // NaN before, and it beat the Offset
    CHECK_FALSE(delay("inf").has_value());
    CHECK_FALSE(delay("250ms").has_value());
    fs::remove_all(dir);
}
```

- [ ] **Step 2: Build and watch them fail.** The first case does not compile (`parse_finite_number` is undeclared). Comment it out for one build to see the Offset case fail on `500ms` and the delay case fail on `nan`, then restore it.

- [ ] **Step 3: The helper.** In `src/core/strutil.h`, add `#include <optional>` beside the other includes and this after `ends_with_ci` (line 29):

```cpp
// The number a chart file's text spells, read one way for every such number
// (.chart Offset, song.ini and .sng delay): spaces at either end are allowed,
// then an optional '+', then one finite decimal number and nothing else.
// Anything else ("500ms", "0.25s", "nan", "inf", "", "0x1F4") is absent.
// Locale-independent.
std::optional<double> parse_finite_number(std::string_view text);
```

In `src/core/strutil.cpp`, add `#include <charconv>`, `#include <cmath>` and `#include <system_error>` after line 1, and this before the closing `}  // namespace hydra` (line 44):

```cpp
std::optional<double> parse_finite_number(std::string_view text) {
    std::string_view s = trim_view(text);
    if (!s.empty() && s.front() == '+') {
        s.remove_prefix(1);
        if (!s.empty() && (s.front() == '+' || s.front() == '-')) return std::nullopt;
    }
    if (s.empty()) return std::nullopt;
    double value = 0.0;
    const char* const last = s.data() + s.size();
    const auto [end, ec] = std::from_chars(s.data(), last, value, std::chars_format::general);
    if (ec != std::errc() || end != last || !std::isfinite(value)) return std::nullopt;
    return value;
}
```

- [ ] **Step 4: ChartParser reads Offset through it.** Replace song.cpp lines 1192-1206:

```cpp
    // Offset is a decimal number of seconds. ChartDataEntry keeps a
    // non-integer value in property_str, so parse that.
    if (auto it = song_sec.prop_data.find("Offset");
        it != song_sec.prop_data.end() && !it->second.empty()) {
        const ChartDataEntry& e = it->second.at(0);
        if (e.property_int) {
            song.chart_offset_s = static_cast<double>(*e.property_int);
        } else if (e.property_str) {
            try {
                song.chart_offset_s = std::stod(*e.property_str);
            } catch (const std::exception&) {
                // An unreadable Offset is treated as absent, like CH's default 0.
            }
        }
    }
```

with:

```cpp
    // Offset is a decimal number of seconds. ChartDataEntry keeps an integer
    // in property_int and anything else in property_str; the rest is read by
    // the one chart-number rule, so "500ms" or "nan" counts as absent, like
    // Clone Hero's default 0.
    if (auto it = song_sec.prop_data.find("Offset");
        it != song_sec.prop_data.end() && !it->second.empty()) {
        const ChartDataEntry& e = it->second.at(0);
        if (e.property_int)
            song.chart_offset_s = static_cast<double>(*e.property_int);
        else if (e.property_str)
            song.chart_offset_s = parse_finite_number(*e.property_str);
    }
```

`song.cpp` already includes `core/strutil.h` (line 13).

- [ ] **Step 5: `parse_delay_ms` reads through it.** Replace preview_source.cpp lines 128-139:

```cpp
// A delay string in milliseconds, or nullopt when it is not wholly a number.
// Shared by song.ini's delay and a .sng's metadata delay.
std::optional<double> parse_delay_ms(const std::string& text) {
    try {
        size_t used = 0;
        const double ms = std::stod(text, &used);
        if (used != text.size()) return std::nullopt;
        return ms;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
```

with:

```cpp
// A delay in milliseconds, or nullopt when the text is not one finite number
// (the one chart-number rule, core/strutil). Shared by song.ini's delay and a
// .sng's metadata delay.
std::optional<double> parse_delay_ms(const std::string& text) {
    return parse_finite_number(text);
}
```

`preview_source.cpp` already includes `core/strutil.h` (line 19).

- [ ] **Step 6: Add the helper's own case to `tests/test_s2_parser_owners.cpp` only** (the first test above). `tests/test_strutil.cpp` is not edited.

- [ ] **Step 7: Run the full `hydra_tests`.** The existing cases ".chart: [Song] Offset is read in seconds" (test_song.cpp line 567) and "read_ini_delay_ms reads song.ini delay in milliseconds" (test_preview_source.cpp line 348) must pass unchanged.

- [ ] **Step 8: Commit** `src/core/strutil.h src/core/strutil.cpp src/parse/song.cpp src/app/preview_source.cpp tests/test_s2_parser_owners.cpp` by name: "One strict number rule for .chart Offset and song.ini delay (R7.5)".

#### 6c. `.mid` practice sections come out sorted (finding R7.7)

`ChartParser::parse` sorts its sections once at the end (song.cpp lines 1247-1251). `MidiParser::parse` pass 3 (lines 717-731) appends each EVENTS track's sections in turn and never sorts. `Song` promises "Practice sections in tick order" (song.h line 136), so `.mid` breaks that promise on a file with two EVENTS tracks. `build_time_box` (`src/app/preview_view.cpp` lines 532-546) works around it: it checks `std::is_sorted` and keeps a front-to-back scan for an unsorted list, which names the wrong section. Three library charts have two EVENTS tracks out of order.

The fix is one sort helper both parsers call. The time box then drops its fallback and always binary-searches.

- [ ] **Step 1: Failing test.**

```cpp
TEST_CASE("s2 owners: .mid practice sections come out in tick order (R7.7)") {
    using namespace testmidi;
    const std::vector<uint8_t> tempo = concat({set_tempo(), end_of_track()});
    const std::vector<uint8_t> drums =
        concat({track_name("PART DRUMS"), note_on(96, 100), end_of_track()});
    // First EVENTS track: Intro at 0, Chorus at 1920 (VLQ 0x8F 0x00).
    const std::vector<uint8_t> events1 =
        concat({track_name("EVENTS"), text_event("[section Intro]"),
                text_after({0x8F, 0x00}, "[section Chorus]"), end_of_track()});
    // Second EVENTS track: Verse at 960 (VLQ 0x87 0x40).
    const std::vector<uint8_t> events2 =
        concat({track_name("EVENTS"), text_after({0x87, 0x40}, "[section Verse]"),
                end_of_track()});
    const Song song = load_songbytes_mid(smf_tracks({tempo, drums, events1, events2}), true, true);

    REQUIRE(song.practice_sections.size() == 3);
    CHECK(song.practice_sections[0].name == "Intro");
    CHECK(song.practice_sections[1].name == "Verse");
    CHECK(song.practice_sections[1].tick == 960);
    CHECK(song.practice_sections[2].name == "Chorus");

    // The time box names the section the playhead is in. At 120 BPM and 480
    // ticks a beat, tick 1000 is 1041.67 ms (Verse) and tick 2000 is 2083.33 ms
    // (Chorus). Before the sort it said Intro, then Verse.
    const app::PreviewScene scene = app::build_preview_scene(song, nullptr);
    CHECK(app::build_time_box(scene, 1041.7, 3000.0).section_line == "Section Verse");
    CHECK(app::build_time_box(scene, 2083.4, 3000.0).section_line == "Section Chorus");
}
```

- [ ] **Step 2: Build and watch it fail** on `practice_sections[1].name == "Verse"` (it reads "Chorus").

- [ ] **Step 3: One sort, both parsers.** In `song.cpp`, add right after `section_name_of` (after line 73):

```cpp
// Practice sections in tick order, the order Song promises. A .chart's
// [Events] need not be sorted, and a .mid lists each EVENTS track's sections
// in turn, so both parsers call this once at the end. The sort is stable, so
// two sections on one tick keep the file's order.
void sort_practice_sections(std::vector<SongSection>& sections) {
    std::stable_sort(sections.begin(), sections.end(),
                     [](const SongSection& a, const SongSection& b) { return a.tick < b.tick; });
}
```

Replace ChartParser's lines 1247-1251:

```cpp
        std::stable_sort(song.practice_sections.begin(),
                         song.practice_sections.end(),
                         [](const SongSection& a, const SongSection& b) {
                             return a.tick < b.tick;
                         });
```

with `        sort_practice_sections(song.practice_sections);`, and change the comment above the walk (lines 1234-1235) to "Practice sections. tick_order follows the file, which is not required to be sorted; sort_practice_sections orders them."

In MidiParser pass 3, after the loop's closing brace (line 731) and before `song.check_activations(rules_);` (line 733), add `    sort_practice_sections(song.practice_sections);`. Change the pass 3 comment (lines 717-718) to "Pass 3: practice sections, which live on their own track(s) as bracketed text metas, sorted once at the end."

- [ ] **Step 4: The time box binary-searches only.** Replace preview_view.cpp lines 532-546:

```cpp
    // Sections are in tick order on every chart but one kind: a MIDI file
    // with more than one EVENTS track lists each track's sections in turn.
    // The answer has always been "the section before the first one past the
    // playhead, front to back", so an out-of-order list keeps that scan and
    // reads exactly as before; a sorted one gets the binary search.
    const std::vector<PreviewSection>& sections = scene.sections;
    const auto by_tick = [](const PreviewSection& a, const PreviewSection& b) {
        return a.tick < b.tick;
    };
    const auto section_past =
        std::is_sorted(sections.begin(), sections.end(), by_tick)
            ? std::upper_bound(sections.begin(), sections.end(), now_tick,
                               [](int64_t v, const PreviewSection& s) { return v < s.tick; })
            : std::find_if(sections.begin(), sections.end(),
                           [now_tick](const PreviewSection& s) { return s.tick > now_tick; });
```

with:

```cpp
    // Both parsers hand sections over in tick order (sort_practice_sections),
    // so a binary search finds the last one at or before the playhead.
    const std::vector<PreviewSection>& sections = scene.sections;
    const auto section_past =
        std::upper_bound(sections.begin(), sections.end(), now_tick,
                         [](int64_t v, const PreviewSection& s) { return v < s.tick; });
```

- [ ] **Step 5: The one existing test that relied on the fallback.** In `tests/test_preview_view.cpp`, the case "preview lookups: searches match the old scans on a busy synthetic chart" rotates the scene's sections out of order and expects the old scan's answer (lines 1869-1876). That input can no longer come from a parser. Replace those eight lines with:

```cpp
        // Sections reach the scene in tick order from both parsers.
        const PreviewScene sorted = build_preview_scene(c.song, &c.path, 4);
        CHECK(std::is_sorted(sorted.sections.begin(), sorted.sections.end(),
                             [](const PreviewSection& a, const PreviewSection& b) {
                                 return a.tick < b.tick;
                             }));
```

- [ ] **Step 6: Run the full `hydra_tests`.**

- [ ] **Step 7: Commit** `src/parse/song.cpp src/app/preview_view.cpp tests/test_preview_view.cpp tests/test_s2_parser_owners.cpp` by name: ".mid practice sections sorted like .chart; time box drops its unsorted scan (R7.7)".

#### 6d. One owner for the default 4/4 (findings 258 and 319)

`apply_timesig` owns the meter rule: ticks per measure = resolution × 4 × num / den, written together with the signature (song.cpp lines 154-169). But Song's constructor writes the opening meter by hand (song.h lines 112-115: `tpm_changes[0] = resolution * 4; timesig_changes[0] = {4, 4};`). The Preview's time box starts from its own `int ts_num = 4, ts_den = 4;` (preview_view.cpp line 520), and `PreviewTimeSig` repeats 4 and 4 as member defaults (preview_view.h lines 141-142). Test fixtures write meters by hand too, sometimes without the signature.

The fix names the default once, in `song.h`, and writes it through `apply_timesig`. `apply_timesig` sits in `song.cpp`'s anonymous namespace today, so fixtures and an inline constructor can't reach it. This step moves it out (two namespace lines, no body change), declares it in `song.h`, and defines Song's constructor in `song.cpp`. `PreviewTimeSig`'s defaults read the constants, and the time box starts from a default `PreviewTimeSig`. So a default-built scene shows Song's default rather than a literal of its own. A scene built from a song always has a tick-0 signature, because every Song now gets one through `apply_timesig`.

This is how the step honours the ruling "build_time_box reads the scene's default". A default-built `PreviewScene` has no signatures at all, so "the scene's default" is its element type's default, which now reads Song's constants. The alternative, seeding `PreviewScene::time_sigs` with one entry, would make `build_preview_scene` clear it before filling and is more moving parts for the same answer.

Output is identical for every chart: the integer math gives `resolution * 4` exactly.

- [ ] **Step 1: Failing test.**

```cpp
TEST_CASE("s2 owners: Song's default meter is written once, through apply_timesig (258, 319)") {
    const Song song(192);
    CHECK(song.tpm_changes == std::map<int64_t, int64_t>{{0, 768}});
    CHECK(song.timesig_changes.at(0) ==
          std::make_pair(kDefaultTimeSigNumerator, kDefaultTimeSigDenominator));

    // Fixtures set a meter and its signature together, through the owner.
    Song fixture(480);
    apply_timesig(fixture, 2880, 3, 4);
    CHECK(fixture.tpm_changes.at(2880) == 1440);
    CHECK(fixture.timesig_changes.at(2880) == std::make_pair(3, 4));

    // A scene built from nothing reads Song's default, not a literal of its own.
    const app::PreviewTimeSig none;
    CHECK(none.numerator == kDefaultTimeSigNumerator);
    CHECK(none.denominator == kDefaultTimeSigDenominator);
}
```

The case above can only fail to compile: with today's constructor the values are the same, so it can't catch a second 4/4 owner coming back. A source scan backs the one-owner promise. It reads every `.cpp` and `.h` file under `src` and `tests`, and fails on any line that writes a meter by hand. The only allowed writer is `apply_timesig`'s own line in `src/parse/song.cpp`. It also fails on a bare `= 4` signature default in `preview_view.h` or `preview_view.cpp`. Add it to the same file. `HYDRA_SOURCE_DIR` is already defined for `hydra_tests` (CMakeLists.txt line 456), and the file's header in Step 0 already includes `<regex>` and `<sstream>` and checks that `HYDRA_SOURCE_DIR` is defined, so nothing else needs adding here.

```cpp
namespace {

std::string read_source(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

}  // namespace

TEST_CASE("s2 owners: no meter is written by hand outside apply_timesig (258, 319)") {
    const std::filesystem::path root = HYDRA_SOURCE_DIR;
    // A hand write into the meter map: `tpm_changes[...] = ...` (not `==`).
    const std::regex meter_write(R"(tpm_changes\[[^\]]*\]\s*=[^=])");
    std::vector<std::string> offenders;
    for (const char* dir : {"src", "tests"}) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root / dir)) {
            const std::string ext = entry.path().extension().string();
            if (ext != ".cpp" && ext != ".h") continue;
            if (entry.path().filename() == "test_s2_parser_owners.cpp") continue;
            std::istringstream lines(read_source(entry.path()));
            std::string line;
            int n = 0;
            while (std::getline(lines, line)) {
                ++n;
                if (!std::regex_search(line, meter_write)) continue;
                const bool owner = entry.path().filename() == "song.cpp" &&
                                   line.find("song.tpm_changes[tick] =") != std::string::npos;
                if (!owner) offenders.push_back(entry.path().string() + ":" + std::to_string(n));
            }
        }
    }
    CHECK_MESSAGE(offenders.empty(), "meters written by hand: " << offenders.size()
                                     << (offenders.empty() ? "" : ", first " + offenders[0]));

    // The Preview's time box keeps no 4/4 literal of its own.
    const std::regex sig_literal(R"((numerator|denominator|ts_num|ts_den)\s*=\s*4\b)");
    for (const char* file : {"src/app/preview_view.h", "src/app/preview_view.cpp"}) {
        CAPTURE(file);
        CHECK_FALSE(std::regex_search(read_source(root / file), sig_literal));
    }
}
```

On today's code this scan finds Song's constructor (`song.h` line 113), the six fixture lines and the hand-set meters Step 5 lists, and the three Preview literals. After Steps 3-5 it finds nothing.

- [ ] **Step 2: Build and watch it fail to compile** (`kDefaultTimeSigNumerator` and `apply_timesig` undeclared in the test). To see the scan go red on its own, comment out the first case for one run.

- [ ] **Step 3: Name the default and export the owner.** In `song.h`, add before `class Song {` (line 110):

```cpp
// The meter a chart has before its first time signature: 4/4. Song's
// constructor writes it through apply_timesig, and the Preview's
// PreviewTimeSig reads it for a scene with no song.
inline constexpr int kDefaultTimeSigNumerator = 4;
inline constexpr int kDefaultTimeSigDenominator = 4;
```

Replace the constructor (lines 112-115):

```cpp
    explicit Song(int64_t resolution) : tick_resolution_(resolution) {
        tpm_changes[0] = resolution * 4;
        timesig_changes[0] = {4, 4};
    }
```

with:

```cpp
    // Starts at the default meter (kDefaultTimeSig*), written by apply_timesig
    // like every other meter.
    explicit Song(int64_t resolution);
```

After the class's closing `};` (line 164), add:

```cpp
// A time signature at `tick`: ticks per measure = resolution * 4 * num / den,
// and the signature itself for display, written together. The one owner of
// the meter rule: both parsers, Song's default and test fixtures call it. A
// numerator of 0 is ignored; a bottom number of 0 or less refuses the chart.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator);
```

In `song.cpp`, close the anonymous namespace just before `apply_timesig`'s comment and reopen it just after its body. That is, insert `}  // namespace` as a new line before line 154 (`// A time signature: ticks per measure = ...`) and `namespace {` as a new line after line 169 (the `}` that ends `apply_timesig`). The body does not change.

After `title_or_unknown` (after line 240), add:

```cpp
Song::Song(int64_t resolution) : tick_resolution_(resolution) {
    apply_timesig(*this, 0, kDefaultTimeSigNumerator, kDefaultTimeSigDenominator);
}
```

- [ ] **Step 4: The Preview reads Song's default.** In `preview_view.h`, replace lines 141-142 (`int numerator = 4;` and `int denominator = 4;`) with:

```cpp
    int numerator = kDefaultTimeSigNumerator;  // Song's default (parse/song.h)
    int denominator = kDefaultTimeSigDenominator;
```

`preview_view.h` already includes `parse/song.h` (line 29). In `preview_view.cpp`, replace lines 518-529:

```cpp
    // The time signature in force at the playhead's tick, as the chart wrote
    // it; 4/4 before any, the chart default.
    int ts_num = 4, ts_den = 4;
    const auto sig_past = std::upper_bound(
        scene.time_sigs.begin(), scene.time_sigs.end(), now_tick,
        [](int64_t v, const PreviewTimeSig& t) { return v < t.tick; });
    if (sig_past != scene.time_sigs.begin()) {
        ts_num = (sig_past - 1)->numerator;
        ts_den = (sig_past - 1)->denominator;
    }
    char buf[64];
    std::snprintf(buf, sizeof buf, "BPM %.3f \xC2\xB7 %d/%d", bpm, ts_num, ts_den);
```

with:

```cpp
    // The time signature in force at the playhead's tick, as the chart wrote
    // it. A scene from a song always has one at tick 0; a scene built from
    // nothing shows PreviewTimeSig's default, which is Song's.
    PreviewTimeSig sig;
    const auto sig_past = std::upper_bound(
        scene.time_sigs.begin(), scene.time_sigs.end(), now_tick,
        [](int64_t v, const PreviewTimeSig& t) { return v < t.tick; });
    if (sig_past != scene.time_sigs.begin()) sig = *(sig_past - 1);
    char buf[64];
    std::snprintf(buf, sizeof buf, "BPM %.3f \xC2\xB7 %d/%d", bpm, sig.numerator,
                  sig.denominator);
```

- [ ] **Step 5: Fixtures write meters through the owner.** These are the existing-test edits the change calls for. Six fixtures write `song.tpm_changes[0] = 768;` right after `Song song(192);`, repeating the default (the audit counted five; there are six). Delete that line at `tests/test_replay.cpp` lines 235, 486 and 1046, `tests/test_preview_view.cpp` line 188, `tests/test_search.cpp` line 393 and `tests/test_rules.cpp` line 43.

Four fixtures set a meter by hand. In `tests/test_preview_view.cpp`, line 462 `song.tpm_changes[2880] = 1440;` becomes `apply_timesig(song, 2880, 3, 4);`. Lines 501-504 become `apply_timesig(song, 1920, 6, 8);` and `apply_timesig(song, 3360, 3, 4);`. Lines 1812-1813 become `apply_timesig(song, t, three ? 3 : 4, 4);`. `make_sp_song`'s last parameter (line 125) changes from `const std::map<int64_t, int64_t>& extra_tpm = {}` to `const std::map<int64_t, std::pair<int, int>>& extra_sigs = {}`. Its line 129 becomes `for (const auto& [tick, sig] : extra_sigs) apply_timesig(song, tick, sig.first, sig.second);`, and the comment at lines 120-121 says "`extra_sigs` adds time signatures (tick -> numerator, denominator) the same way." Its one caller, line 1385, passes `{{3840, {7, 8}}}` in place of `{{3840, 1680}}`.

None of these tests checks the signature, so every expectation stays as written. The fixture at line 462 now carries 3/4 with its meter, which the audit flagged as the one that split them.

- [ ] **Step 6: Run the full `hydra_tests`.** "build_time_box: the time signature in force, as the chart wrote it" (line 495) still reads "BPM 0.000 · 4/4" for an empty scene.

- [ ] **Step 7: Commit** `src/parse/song.h src/parse/song.cpp src/app/preview_view.h src/app/preview_view.cpp tests/test_replay.cpp tests/test_preview_view.cpp tests/test_search.cpp tests/test_rules.cpp tests/test_s2_parser_owners.cpp` by name: "The default 4/4 is written once, through apply_timesig (258, 319)".

#### 6e. One container-entry name rule (finding 61)

A `.sng` finds its notes entry by exact name with `notes_file_format`: `notes.mid` or `notes.chart`, any case (song.cpp line 1298). An `.srb` uses `chart_format_of` on the name's extension instead, then sniffs `MThd` bytes for any other name (lines 1333-1340). So `song.chart` counts as a chart in an `.srb` but not in a `.sng`. `chart_files.cpp` already owns "which file is a chart", so the exact-name rule there becomes the one rule for both containers. An `.srb` keeps its byte sniff as the fallback for a name outside the rule, because its notes are always stream 2 and there is nothing else to pick.

Nothing changes for any library file: all 54 `.sng` hold `notes.mid` or `notes.chart`, and the bundled `.srb` use the standard names (per the audit; not re-read here). The only behaviour change is an `.srb` whose notes name has a chart extension but isn't `notes.*`. Its bytes now decide, instead of the extension.

- [ ] **Step 1: Failing test.**

```cpp
TEST_CASE("s2 owners: a container's notes entry is found by its exact name (61)") {
    using namespace testmidi;
    const std::vector<uint8_t> mid =
        smf(concat({track_name("PART DRUMS"), set_tempo(), note_on(96, 100), end_of_track()}));
    // "song.chart" is not a notes name. Before, the .srb trusted the extension
    // and read these MIDI bytes as .chart text, which failed. Now the name
    // decides nothing, so the stream's bytes do, and it loads as MIDI.
    CHECK(load_songbytes_srb(srb_with("song.chart", mid), true, true).sequence.size() == 1);
    // The exact names decide, in any case.
    CHECK(load_songbytes_srb(srb_with("NOTES.MID", mid), true, true).sequence.size() == 1);
}
```

- [ ] **Step 2: Build and watch it fail** with an exception from the `.chart` parser (no `[Song]` section).

- [ ] **Step 3: The change.** In `song.cpp`, replace line 1333:

```cpp
    const ChartFormat named = chart_format_of(md.notes_filename);
```

with:

```cpp
    // The notes stream's format comes from its name, by the exact-name rule a
    // .sng entry and a loose folder use (notes_file_format). An .srb's notes
    // are always stream 2, so a name outside that rule is not fatal: the
    // stream's own bytes decide below.
    const ChartFormat named = notes_file_format(md.notes_filename);
```

In `src/parse/chart_files.h`, replace the comment on `notes_file_format` (lines 21-22):

```cpp
// A loose folder's notes file by its exact name, any case: "notes.mid" is
// Mid, "notes.chart" is Chart, anything else is None.
```

with:

```cpp
// A notes file by its exact name, any case, wherever it sits: loose in a song
// folder, or as an entry inside a .sng or .srb. "notes.mid" is Mid,
// "notes.chart" is Chart, anything else is None. The one name rule for both.
```

- [ ] **Step 4: Run the full `hydra_tests`.** "srb: an unexpected notes filename falls back to payload sniffing" (test_srb.cpp line 168) and "srb: a wrapped chart parses identically to the loose file" (line 147) pass unchanged.

- [ ] **Step 5: Commit** `src/parse/song.cpp src/parse/chart_files.h tests/test_s2_parser_owners.cpp` by name: "Containers find their notes entry by one name rule (61)".

#### 6f. A blank artist or charter shows the placeholder (finding 60)

Three metadata readers in `src/app/analysis.cpp` each start artist and charter at `"<unknown artist>"` and `"<unknown charter>"`. `read_metadata_ini` (lines 138-152) and `parse_sng_metadata` (lines 167-182) let a blank value overwrite that, so `charter =` shows a blank cell. `parse_srb_metadata` (lines 192-213) keeps the placeholder for a blank. The title already has one owner, `title_or_unknown`, applied once in `discover_charts` after the rescan cache (line 449).

The fix gives artist and charter the same kind of owner. `artist_or_unknown` and `charter_or_unknown` sit beside `title_or_unknown` in `parse/song`, the readers return blank for missing, and `discover_charts` applies both at the same spot as the title. That spot runs after the rescan cache, so a cached row from an older scan is fixed too. The library shows the placeholder on the next scan. Stored song rows (`ref_artist`, `ref_charter`, used by reports and the leaderboard page) take the new text when the chart is next analysed. Step 1's results-stamp bump re-analyses every record, so in practice that is the release.

- [ ] **Step 1: Failing test.**

```cpp
TEST_CASE("s2 owners: a blank artist or charter reads the placeholder (60)") {
    CHECK(artist_or_unknown("") == kUnknownArtist);
    CHECK(charter_or_unknown("") == kUnknownCharter);
    CHECK(artist_or_unknown("X") == "X");
    CHECK(std::string(kUnknownArtist) == "<unknown artist>");
    CHECK(std::string(kUnknownCharter) == "<unknown charter>");

    namespace fs = std::filesystem;
    const std::string chart = corpus::first_chart_with_suffix(".chart");
    REQUIRE(!chart.empty());
    const fs::path root = fs::temp_directory_path() /
                          ("hydra_s2_blank_meta_" + std::to_string(GetCurrentProcessId()));
    fs::remove_all(root);
    // "artist =" and "charter =" present but blank (56 library song.ini files
    // have a blank charter).
    fs::create_directories(root / "blank");
    fs::copy_file(fs::u8path(chart), root / "blank" / "notes.chart");
    {
        std::ofstream ini(root / "blank" / "song.ini", std::ios::binary);
        ini << "[song]\nname = N\nartist =\ncharter =\n";
    }
    // Both keys missing.
    fs::create_directories(root / "missing");
    fs::copy_file(fs::u8path(chart), root / "missing" / "notes.chart");
    {
        std::ofstream ini(root / "missing" / "song.ini", std::ios::binary);
        ini << "[song]\nname = N\n";
    }

    auto [items, errors] = app::discover_charts({root.u8string()});
    CHECK(errors.empty());
    REQUIRE(items.size() == 2);
    for (const app::ScanItem& it : items) {
        CAPTURE(it.notespath);
        CHECK(it.artist == kUnknownArtist);
        CHECK(it.charter == kUnknownCharter);
    }

    // A rescan-cache row from before this change still holds the blanks. The
    // scan reads it through the same fallback.
    store::RecordStore store(":memory:");
    std::vector<store::ChartLibraryEntry> entries;
    for (const app::ScanItem& it : items)
        entries.push_back({it.md5, it.title, "", "", it.notespath, it.rootfolder, it.sig});
    store.rebuild_chart_library(entries);
    store::ChartLibraryCache cache = store.chart_library_cache();
    auto [cached, errors2] = app::discover_charts({root.u8string()}, app::ScanCallbacks{}, &cache);
    CHECK(errors2.empty());
    REQUIRE(cached.size() == 2);
    for (const app::ScanItem& it : cached) {
        CHECK(it.artist == kUnknownArtist);
        CHECK(it.charter == kUnknownCharter);
    }
    fs::remove_all(root);
}
```

- [ ] **Step 2: Build and watch it fail to compile** (`artist_or_unknown` undeclared). With that block commented out for one build, the `blank` folder reads an empty artist and charter.

- [ ] **Step 3: The owners.** In `song.h`, after `title_or_unknown`'s declaration (line 59), add:

```cpp
// What a song with no usable artist or charter shows everywhere it is shown.
inline constexpr const char* kUnknownArtist = "<unknown artist>";
inline constexpr const char* kUnknownCharter = "<unknown charter>";

// The one fallback for a song's artist and charter, beside title_or_unknown:
// a blank value (an empty `artist =`, a missing key, an empty .sng or .srb
// field) becomes the placeholder. discover_charts applies both once, after
// the rescan cache, so cached rows from older scans are covered too.
std::string artist_or_unknown(std::string artist);
std::string charter_or_unknown(std::string charter);
```

In `song.cpp`, after `title_or_unknown` (after line 240, beside 6d's constructor):

```cpp
std::string artist_or_unknown(std::string artist) {
    return artist.empty() ? std::string(kUnknownArtist) : artist;
}

std::string charter_or_unknown(std::string charter) {
    return charter.empty() ? std::string(kUnknownCharter) : charter;
}
```

- [ ] **Step 4: The readers return blank for missing.** In `analysis.cpp`, each reader's three opening lines change. In `read_metadata_ini`, lines 141-144:

```cpp
    // Empty = no usable name; discover_charts applies the one fallback.
    std::string title;
    std::string artist = "<unknown artist>";
    std::string charter = "<unknown charter>";
```

become:

```cpp
    // Empty = missing or blank; discover_charts applies the one fallback for
    // each (title_or_unknown, artist_or_unknown, charter_or_unknown).
    std::string title;
    std::string artist;
    std::string charter;
```

`parse_sng_metadata`'s lines 169-172 and `parse_srb_metadata`'s lines 194-197 change the same way. In `parse_srb_metadata`, lines 205-206 (`if (!md.artist.empty()) artist = md.artist;` and the charter line) become `artist = md.artist;` and `charter = md.charter;`, since a blank now means the same as missing. The comment at line 160 ends "a missing name stays empty and the other keys keep their <unknown> defaults"; it becomes "missing keys stay empty". The comment at lines 189-190 ("Any parse failure degrades to the <unknown> defaults, matching the .sng path") becomes "Any parse failure leaves the fields empty, matching the .sng path".

- [ ] **Step 5: Apply once in `discover_charts`.** Replace lines 446-449:

```cpp
        // The one fallback, whichever source produced the title: a fresh
        // song.ini, .sng or .srb read, or the rescan cache holding an older
        // scan's blank or "<unknown title>".
        r->title = title_or_unknown(std::move(r->title));
```

with:

```cpp
        // The one fallback for each field, whichever source produced it: a
        // fresh song.ini, .sng or .srb read, or the rescan cache holding an
        // older scan's blank or "<unknown title>".
        r->title = title_or_unknown(std::move(r->title));
        r->artist = artist_or_unknown(std::move(r->artist));
        r->charter = charter_or_unknown(std::move(r->charter));
```

- [ ] **Step 6: Run the full `hydra_tests`.** "discover_charts: a song with no usable name reads (unknown)" (test_analysis.cpp line 348) still finds artist "Someone". No corpus `song.ini` has a blank artist or charter (checked with a grep of `testdata/input`), so no corpus-backed test or uitest changes.

- [ ] **Step 7: Commit** `src/parse/song.h src/parse/song.cpp src/app/analysis.cpp tests/test_s2_parser_owners.cpp` by name: "A blank artist or charter shows the placeholder, through one owner (60)".

**Verify (whole task):**

```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t6
.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe
.\build_cpp.ps1 -Target hydra_replay; .\build_cpp.ps1 -Target hydra_batch
```

Then run the lane corpus check from "Before the wave" at all four difficulties (`$lane` this worktree, `$w = "$sp\s2-t6"`, files `t6-$d-2x1.txt`).

**Done when:**
- [ ] The six commits are on `claude/s2-t6`, each staged by name with the `Task:`, `Agent:` and `Session:` trailers and the Co-Authored-By line.
- [ ] Every `s2 owners:` case passes, and the full `hydra_tests` run ends `Status: SUCCESS!`.
- [ ] The lane corpus check prints nothing at any difficulty: all 97 corpus charts score and path exactly as at 1abd188.
- [ ] `git grep -n "\"<unknown artist>\"\|\"<unknown charter>\"" src` shows only `song.h` and the existing comment in `src/app/library_query.h` (line 31), which names the text as an example.
- [ ] `git grep -n "is_sorted" src/app/preview_view.cpp` and `git grep -n "std::stod" src/parse/song.cpp src/app/preview_source.cpp` print nothing.
- [ ] `git grep -n "tpm_changes\[0\] = " src tests` prints nothing.
- [ ] No stamp is bumped; T9 is told "T6: no stamp".

---

### Task T7: Off-speed leaderboard rows get their own status

*Decision: D25. Finding: 222.*

**Goal:** A leaderboard score played at any speed but 100% gets the status "other speed". It stays out of Matched, Above optimal and Points left on table. One base-speed constant replaces the four bare 100s.

**How it works today.** `collect_dm_rows` in `src/app/dm_report.cpp` (lines 198-207) checks the speed only for the "% of opt" column (`if (s.speed == 100 && opt > 0)`, line 202). Every score with a stored result then gets "above optimal" if it beats Hydra's optimal and "matched" otherwise, whatever its speed (line 204). The page counts both statuses, and "Points left on table" sums the matched rows (line 126 of the page script). So a 150% run that beats the 100% optimal reads as "above optimal", and a slower run adds its gap to Points left. Clone Hero keeps a separate leaderboard per speed, and speed changes the hit windows in chart time, so neither comparison means anything. Avg % already skips them, because `pct` is set only at speed 100. The literal 100 appears four times: `DmScore::speed` (`src/net/dmbot_client.h` line 53), `parse_score`'s default (`dmbot_client.cpp` line 271), `DmReportRow::speed` (`dm_report.h` line 36) and line 202.

**The change.** `dmbot_client.h` gets `kBaseSpeedPercent` and `is_base_speed()`, and the four 100s read them. `collect_dm_rows` gives an off-speed row the status "other speed", whatever its score and whether or not Hydra has a result. It still fills Hydra opt and Points left when a result exists, so the row shows the numbers, but the Points-left cell is dimmed. The page gets an "Other speed" filter option, chip colour and stat. `tally_dm_rows` counts it in its own field. Today its last branch would have counted it as "not in library".

**What you'll see change.** Only the dmleaderboards comparison changes. Off-speed rows show "other speed" and drop out of Matched, Above optimal and Points left on table. When a user has off-speed scores, the finished window and the page subtitle add one clause, for example "Done: 120 matched, 4 above optimal, 0 not analyzed, 3 not in your library, 2 at other speeds." A user with no off-speed scores sees exactly today's text. This clause's wording is not in D25. It was question Q5, and the user approved it in D31, together with the rest of this task's visible text: the "Other speed" filter option, the stat row, the Status column's help sentence, the dimmed Points-left cell and the UserGuide sentence. How many of the user's scores are off speed is unknown until the page is built.

**Stamps:** none.

**Files.** The constant and predicate go in `src/net/dmbot_client.h`, read by `src/net/dmbot_client.cpp`. The status, page script, tally and counts sentence change in `src/app/dm_report.cpp` and `dm_report.h`. The chip colour is one CSS rule in `src/app/html_page.cpp`, and the finished-window line is in `src/ui/library_dialogs.cpp`. One UserGuide sentence changes in `docs/UserGuide.md`; T2 edits a different line of that file. Tests go in the new `tests/test_s2_offspeed.cpp`, plus one line in `CMakeLists.txt`.

Worktree: `.claude/worktrees/s2-t7` on `claude/s2-t7`, from `claude/s1-t3`. Its corpus check runs at Expert only, against the shared baseline.

- [ ] **Step 1: Failing tests.** Create `tests/test_s2_offspeed.cpp` and add it to the end of the `hydra_tests` list in `CMakeLists.txt` (after line 429):

```cpp
// Step 2, task 7: off-speed leaderboard scores get their own status (D25,
// finding 222). Clone Hero keeps a leaderboard per speed and Hydra's optimal
// is computed at base speed, so an off-speed score is shown but not compared.

#include "doctest.h"

#include <string>
#include <vector>

#include "app/analysis.h"
#include "app/dm_report.h"
#include "core/model.h"
#include "corpus_util.h"
#include "net/dmbot_client.h"
#include "store/record_store.h"

using namespace hydra;
using app::dm_report::DmReportRow;

namespace {

constexpr const char* kMode = "Expert Pro Drums, 2x Bass";
constexpr const char* kHash = "aa11bb22cc33dd44ee55ff6677889900";

// One analyzed corpus chart stored under kHash at SP cap 4; returns its best
// score. (The same fixture as test_dm_report.cpp's, without the what-if row.)
int64_t fill_store(store::RecordStore& store) {
    app::AnalysisSettings settings;
    settings.depth_value = 0;
    for (const std::string& path : corpus::chart_paths()) {
        try {
            app::AnalysisResult result = app::analyze_chart_file(path, settings);
            if (result.song.is_empty() || result.record.paths.empty()) continue;
            store.add_song(kHash, "Stored Title", "Stored Artist", "Stored Charter", result.song);
            store.add_record(
                store::RecordKey{kHash, kMode, store::CapQuery::at(kCloneHeroSpCap)},
                result.record);
            return result.record.best_path().totalscore();
        } catch (const std::exception&) {
            continue;
        }
    }
    REQUIRE_MESSAGE(false, "no corpus chart analyzed");
    return 0;
}

net::DmScore score_at(const std::string& identifier, int64_t score, int speed) {
    net::DmScore s;
    s.identifier = identifier;
    s.song_name = "Board Title";
    s.score = score;
    s.percent = 100;
    s.speed = speed;
    s.posted = "2026-01-01T00:00:00Z";
    return s;
}

}  // namespace

TEST_CASE("s2 offspeed: one base speed, read everywhere") {
    CHECK(net::kBaseSpeedPercent == 100);
    CHECK(net::is_base_speed(100));
    CHECK_FALSE(net::is_base_speed(150));
    CHECK_FALSE(net::is_base_speed(75));
    CHECK(net::DmScore{}.speed == net::kBaseSpeedPercent);
    CHECK(DmReportRow{}.speed == net::kBaseSpeedPercent);
}

TEST_CASE("s2 offspeed: an off-speed score is 'other speed', with or without a result") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    REQUIRE(optimal > 0);

    const std::vector<net::DmScore> scores = {
        score_at(kHash, optimal + 5, 150),     // would read "above optimal"
        score_at(kHash, optimal - 1000, 75),   // would read "matched"
        score_at("00ff00ff00ff00ff00ff00ff00ff00ff", 123, 150),  // no result at all
        score_at(kHash, optimal - 1000, 100),  // base speed: matched, as today
    };
    const std::vector<DmReportRow> rows =
        app::dm_report::collect_dm_rows(store, scores, kMode, store::Lens{});
    REQUIRE(rows.size() == 4);

    // These strings are load-bearing: the page's filter and chip classes key on them.
    CHECK(rows[0].status == "other speed");
    CHECK(rows[1].status == "other speed");
    CHECK(rows[2].status == "other speed");
    CHECK(rows[3].status == "matched");

    // The numbers still show; only the comparison is withheld.
    CHECK(rows[0].optimal == optimal);
    CHECK(rows[1].delta == 1000);
    CHECK_FALSE(rows[0].pct.has_value());
    CHECK_FALSE(rows[1].pct.has_value());
    REQUIRE(rows[3].pct.has_value());

    const app::dm_report::DmReportStats stats = app::dm_report::tally_dm_rows(rows);
    CHECK(stats.total == 4);
    CHECK(stats.matched == 1);
    CHECK(stats.above_optimal == 0);
    CHECK(stats.other_speed == 3);
    CHECK(stats.not_in_library == 0);
}

TEST_CASE("s2 offspeed: the counts sentence names other speeds only when there are some") {
    app::dm_report::DmReportStats st;
    st.total = 4;
    st.matched = 1;
    st.above_optimal = 1;
    st.not_in_library = 1;
    st.other_speed = 1;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 matched, 1 above optimal, 0 not analyzed, 1 not in your library, "
          "1 at another speed");
    st.other_speed = 0;
    CHECK(app::dm_report::counts_phrase(st) ==
          "1 matched, 1 above optimal, 0 not analyzed, 1 not in your library");
}

TEST_CASE("s2 offspeed: the page filters, colours and counts 'other speed'") {
    store::RecordStore store(":memory:");
    const int64_t optimal = fill_store(store);
    const app::dm_report::GeneratedDmReport report = app::dm_report::generate_dm_report(
        store, {score_at(kHash, optimal - 1, 100), score_at(kHash, optimal + 5, 150),
                score_at(kHash, optimal - 9, 50)},
        kMode, store::Lens{}, "TestUser");
    CHECK(report.stats.other_speed == 2);
    CHECK(report.html.find("TestUser — 3 scores: 1 matched, 0 above optimal, 0 not analyzed, "
                           "0 not in your library, 2 at other speeds") != std::string::npos);
    CHECK(report.html.find("<option value=\"other speed\">Other speed</option>") !=
          std::string::npos);
    CHECK(report.html.find("'other speed':'s-otherspeed'") != std::string::npos);
    CHECK(report.html.find("['Other speed', otherSpeed.length.toLocaleString()]") !=
          std::string::npos);
    CHECK(report.html.find(".s-otherspeed{") != std::string::npos);
}

// The page's stat row counts statuses in JavaScript, and tally_dm_rows counts
// them in C++. The page has to count per filtered view, so it can't just print
// the C++ totals. Instead this case pins that both count the same statuses:
// every status the C++ assigns has its own tally field, its own count in the
// page's stats(), its own filter option and its own chip class, and the page
// counts no status the C++ doesn't know.
TEST_CASE("s2 offspeed: the page counts the same statuses tally_dm_rows counts") {
    const std::vector<std::string> statuses = {"matched", "above optimal", "not analyzed",
                                               "not in library", "other speed"};
    std::vector<DmReportRow> rows;
    for (const std::string& s : statuses) {
        DmReportRow r;
        r.status = s;
        rows.push_back(r);
    }
    const app::dm_report::DmReportStats st = app::dm_report::tally_dm_rows(rows);
    CHECK(st.total == 5);
    CHECK(st.matched == 1);
    CHECK(st.above_optimal == 1);
    CHECK(st.not_analyzed == 1);
    CHECK(st.not_in_library == 1);
    CHECK(st.other_speed == 1);

    store::RecordStore store(":memory:");
    const std::string html =
        app::dm_report::generate_dm_report(store, {}, kMode, store::Lens{}, "TestUser").html;
    for (const std::string& s : statuses) {
        CAPTURE(s);
        CHECK(html.find("rows.filter(r => r.status === '" + s + "')") != std::string::npos);
        CHECK(html.find("<option value=\"" + s + "\">") != std::string::npos);
        CHECK(html.find("'" + s + "':'s-") != std::string::npos);
    }
    // No status counted on the page that the C++ never assigns.
    size_t counted = 0;
    for (size_t at = html.find("rows.filter(r => r.status === '"); at != std::string::npos;
         at = html.find("rows.filter(r => r.status === '", at + 1))
        ++counted;
    CHECK(counted == statuses.size());

    // The help text reads the base speed from kBaseSpeedPercent, not a literal.
    CHECK(html.find("__BASE_SPEED__") == std::string::npos);
    CHECK(html.find("Other speed: played at a speed other than " +
                    std::to_string(net::kBaseSpeedPercent) + "%.") != std::string::npos);
}
```

- [ ] **Step 2: Build and watch it fail to compile** (`kBaseSpeedPercent`, `is_base_speed`, `other_speed` and `counts_phrase` are undeclared).

- [ ] **Step 3: One base speed.** In `dmbot_client.h`, add after `kDefaultApiBase` (line 29):

```cpp
// The playback speed, in percent, that counts as normal play. Clone Hero keeps
// a separate leaderboard per speed, and Hydra's optimal is computed at this
// speed only, so only these scores can be compared with it.
inline constexpr int kBaseSpeedPercent = 100;
inline bool is_base_speed(int speed_percent) { return speed_percent == kBaseSpeedPercent; }
```

Line 53 `int speed = 100;            // playback speed %, 100 = base` becomes `int speed = kBaseSpeedPercent;  // playback speed %`. In `dmbot_client.cpp` line 271, `s.speed = (int)jint(entry, "speed", 100);` becomes `s.speed = (int)jint(entry, "speed", kBaseSpeedPercent);`. In `dm_report.h` line 36, `int speed = 100;` becomes `int speed = net::kBaseSpeedPercent;`. Line 33's comment `// actual/optimal*100, only when speed==100` becomes `// actual/optimal*100, only at base speed`. The status comment at lines 39-41 gains `| "other speed" (played at a speed other than net::kBaseSpeedPercent; shown, never compared)`.

- [ ] **Step 4: The status and the tally.** Replace `dm_report.cpp` lines 198-207:

```cpp
        if (rec && rec->summary.score) {
            int64_t opt = *rec->summary.score;
            row.optimal = opt;
            row.delta = opt - s.score;
            if (s.speed == 100 && opt > 0)
                row.pct = static_cast<double>(s.score) / static_cast<double>(opt) * 100.0;
            row.status = s.score > opt ? "above optimal" : "matched";
        } else {
            row.status = in_library.count(s.identifier) ? "not analyzed" : "not in library";
        }
```

with:

```cpp
        // Hydra's optimal is a base-speed answer, and Clone Hero keeps a
        // leaderboard per speed. An off-speed score shows Hydra's numbers when
        // it has them, but is never called matched or above optimal.
        const bool base = net::is_base_speed(s.speed);
        if (rec && rec->summary.score) {
            int64_t opt = *rec->summary.score;
            row.optimal = opt;
            row.delta = opt - s.score;
            if (base && opt > 0)
                row.pct = static_cast<double>(s.score) / static_cast<double>(opt) * 100.0;
            row.status = s.score > opt ? "above optimal" : "matched";
        } else {
            row.status = in_library.count(s.identifier) ? "not analyzed" : "not in library";
        }
        if (!base) row.status = "other speed";
```

In `dm_report.h`, `DmReportStats` (lines 63-69) gains `int other_speed = 0;   // played off base speed: shown, not compared` after `not_in_library`. After `tally_dm_rows`'s declaration (line 70), add:

```cpp
// "1 matched, 1 above optimal, 0 not analyzed, 1 not in your library", plus
// ", 2 at other speeds" when there are any. The page subtitle and the
// finished window both read it, so the two can't drift.
std::string counts_phrase(const DmReportStats& stats);
```

In `dm_report.cpp`, `tally_dm_rows`'s loop (lines 257-262) becomes:

```cpp
    for (const DmReportRow& r : rows) {
        if (r.status == "matched") ++stats.matched;
        else if (r.status == "above optimal") ++stats.above_optimal;
        else if (r.status == "not analyzed") ++stats.not_analyzed;
        else if (r.status == "other speed") ++stats.other_speed;
        else ++stats.not_in_library;
    }
```

Add after `tally_dm_rows`:

```cpp
std::string counts_phrase(const DmReportStats& stats) {
    std::string out = group_thousands(stats.matched) + " matched, " +
                      group_thousands(stats.above_optimal) + " above optimal, " +
                      group_thousands(stats.not_analyzed) + " not analyzed, " +
                      group_thousands(stats.not_in_library) + " not in your library";
    if (stats.other_speed > 0)
        out += ", " + report::counted(stats.other_speed, "at another speed", "at other speeds");
    return out;
}
```

`generate_dm_report`'s subtitle (lines 276-281) becomes:

```cpp
    std::string subtitle = username + " — " +
                           report::counted(out.stats.total, "score", "scores") + ": " +
                           counts_phrase(out.stats);
```

The existing subtitle test in `test_dm_report.cpp` (line 232) still matches, because its scores are all at base speed.

- [ ] **Step 5: The page.** In `kBody`, after the `not in library` option (line 44), add `      <option value="other speed">Other speed</option>`. In `kPageJs`:

Line 65-66 become:

```js
const STATUS_CLASS = {'matched':'s-matched', 'above optimal':'s-above',
                      'not analyzed':'s-notanalyzed', 'not in library':'s-unmatched',
                      'other speed':'s-otherspeed'};
```

The Points-left cell (line 99) dims for an off-speed row: `const deltaCls = (noDelta || r.status === 'other speed') ? 'num dim' : (r.delta < 0 ? 'num neg' : 'num');`.

The status column's description (line 86) gains a sentence at its end: ` Other speed: played at a speed other than __BASE_SPEED__%. Clone Hero keeps a separate leaderboard per speed, so it is shown but not compared.` The page shows it with the number filled in, "100%", the text D31 approved.

The help text reads the base speed from the constant, because that is cheap here. In the same edit, the two existing help texts that name it change too: "Only for scores played at 100% speed." (line 80) becomes "Only for scores played at __BASE_SPEED__% speed.", and "100% is normal speed." (line 83) becomes "__BASE_SPEED__% is normal speed." `page_template()` (lines 141-144) fills the token once, when the page shell is first built:

```cpp
const std::string& page_template() {
    // The help texts name the base speed through __BASE_SPEED__, so the page
    // reads net::kBaseSpeedPercent instead of repeating 100.
    static const std::string page =
        html::replace_all(html::page_template(kTitle, kBody, kPageJs), "__BASE_SPEED__",
                          std::to_string(net::kBaseSpeedPercent));
    return page;
}
```

The page's visible text is unchanged by the token: it still reads "100%" in each place.

In `stats(rows)`, after `notInLibrary` (line 122) add `    const otherSpeed = rows.filter(r => r.status === 'other speed');`, and after the `'Not in library'` entry (line 132) add `      ['Other speed', otherSpeed.length.toLocaleString()],`. Points left (line 126) already sums only `matched` rows, so off-speed rows leave it with no further edit.

The page's `stats()` mirrors `tally_dm_rows`: the C++ counts the whole list once for the subtitle and the finished window, and the page counts again for whatever rows the filters leave showing. So the two can't share code, and the page can't just print the C++ totals. The last case in Step 1 pins that they count the same statuses, so a status added to one and not the other fails a test. Add a comment above `stats(rows)` in `kPageJs` that says so: `// Mirrors tally_dm_rows in dm_report.cpp; the test "s2 offspeed: the page counts the same statuses" checks the two agree.`

In `html_page.cpp` line 274, append ` .s-otherspeed{color:var(--muted)}` to the status rules. It reads as "shown, not compared", like Not analyzed.

- [ ] **Step 6: The finished window reads the same sentence.** In `src/ui/library_dialogs.cpp`, replace lines 560-566:

```cpp
            const auto& st = job.stats();
            ImGui::TextWrapped("Done: %s matched, %s above optimal, %s not analyzed, %s not in "
                               "your library.",
                               group_thousands(st.matched).c_str(),
                               group_thousands(st.above_optimal).c_str(),
                               group_thousands(st.not_analyzed).c_str(),
                               group_thousands(st.not_in_library).c_str());
```

with:

```cpp
            ImGui::TextWrapped("Done: %s.", app::dm_report::counts_phrase(job.stats()).c_str());
```

If `library_dialogs.cpp` does not already see `app/dm_report.h` through `ui/dm_jobs.h`, include it.

- [ ] **Step 7: UserGuide.** The UserGuide is prose, so it names 100% as a literal; it can't read the constant. In `docs/UserGuide.md`, line 220 ends "...and the ones for songs not in your library." Add after it: "Scores played at a speed other than 100% get their own status, Other speed. Clone Hero keeps a separate leaderboard for each speed, and Hydra Deluxe's optimal is for normal speed, so those rows show Hydra's numbers but don't count as matched or above optimal."

- [ ] **Step 8: Run the full `hydra_tests`, then `hydra_uitest`.** The uitest's canned API posts speed 100 (`tests/ui/uitest_harness.cpp` line 261), so `uitest_batch_reports.cpp`'s `stats().matched == 1` (line 138) holds.

```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t7
.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe
.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4
```

- [ ] **Step 9: Commit** `src/net/dmbot_client.h src/net/dmbot_client.cpp src/app/dm_report.h src/app/dm_report.cpp src/app/html_page.cpp src/ui/library_dialogs.cpp docs/UserGuide.md tests/test_s2_offspeed.cpp CMakeLists.txt` by name: "Off-speed leaderboard scores get their own status (D25)".

**Done when:**
- [ ] Every `s2 offspeed:` case passes, the full `hydra_tests` ends `Status: SUCCESS!`, and every UI test is `[PASS]`.
- [ ] `git grep -n "\b100\b" src/net/dmbot_client.* src/app/dm_report.*` shows no speed literal (the remaining hits are the `*100.0` percent maths; the page's help texts now read `__BASE_SPEED__`).
- [ ] The lane corpus check at Expert prints nothing. This task touches no parser or engine, so that is a formality.
- [ ] The visible texts match what D31 approved: the ", N at other speeds" clause (Q5), the "Other speed" filter option and stat row, the Status help sentence, the dimmed Points-left cell and the UserGuide sentence.
- [ ] No stamp is bumped.

---

### Task T8: The probe tools use the measured cap; the hit-window plan is updated

*Decisions: D27 (finding 77) and D28.*

**Goal:** One place holds Clone Hero's measured whole-window cap (171.4313 ms) and floor (96.2932 ms) with their evidence. The three probe scripts and their test read it. The hit-window plan says steps 1-3 are answered and steps 4-6 are shelved.

**How it works today.** The three scripts judge the same field three ways. `watch_window.py` (lines 45-48) expects `CAP_EXPECT_MS = 171.43` for gaps of `CAP_CHECK_FROM_GAP_MS = 170` or more, within 0.01 ms (line 183). `passive_probe.py` (lines 162-171) judges the "whole window" against `2 * back_ms`: 170 in normal mode, 80 in precision mode. `poll_windows.py` (lines 118-131) calls anything over `2 * back_ms + 0.5` "NO CLAMP". That fires on real data: the committed `results/poll_windows.csv` peaks at 171.431308. The test file repeats 171.43 and 170 as literals (`test_hit_window_scripts.py` lines 47, 81, 89, 101, 103).

**What Clone Hero does.** `0x20F7210` halves each neighbouring gap and holds the half between 37.5 and 85 ms. The two halves are summed and fed to the window formula `0x20DDDA0`. So the formula's input runs from 75 to 170 ms, and its output from 96.2932 to 171.4313 ms. The cap is a clamp on the input, not on the stored window. I re-checked the CSV: 126 rows, maximum 171.431308 (6 rows), minimum 96.293167 (22 rows). Evaluating the formula with the constants in `ch-evidence.md` section 77 gives 171.4313075 at 170 and 96.2931672 at 75. Only normal mode was read; nobody has read precision mode's cap.

**The change.** `constants.py` gains the measured values and the rules derived from them. `watch_window.py` checks the cap and also the floor against them. `passive_probe.py` judges normal mode against the measured cap and its half, and says "cap unknown" in precision mode instead of guessing 2 × 40. `poll_windows.py` moves its verdict into a pure function that reads the measured values, so a real capped reading says "reached the measured cap" instead of "NO CLAMP". `analysis.clamp_verdict` stays the judge for the passive probe and does not change.

**What you'll see change:** only the probe console's verdict lines, and the plan document. Nothing in Hydra.

**Stamps:** none.

Worktree: `.claude/worktrees/s2-t8` on `claude/s2-t8`, from `claude/s1-t3`. Python tests run from the worktree root with `python -m pytest tools/ch_probe/tests -q` (the command in `tools/ch_probe/README.md` line 70).

- [ ] **Step 1: Failing tests.** Create `tools/ch_probe/tests/test_s2_window_constants.py`:

```python
"""Step 2, task 8: the measured hit-window cap and floor have one home.

constants.py holds them with their evidence; the probe scripts read them.
No game needed: the committed poll_windows.csv is the measurement.
"""

from __future__ import annotations

import csv
import os
import sys
import unittest

_HERE = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT = os.path.abspath(os.path.join(_HERE, "..", "..", ".."))
if _REPO_ROOT not in sys.path:
    sys.path.insert(0, _REPO_ROOT)

from tools.ch_probe import constants as C  # noqa: E402
from tools.ch_probe import probe_songs as P  # noqa: E402
from tools.ch_probe.experiments import passive_probe  # noqa: E402
from tools.ch_probe.experiments import poll_windows  # noqa: E402
from tools.ch_probe.experiments import watch_window as WW  # noqa: E402

_CSV = os.path.join(_REPO_ROOT, "tools", "ch_probe", "experiments", "results",
                    "poll_windows.csv")


def measured_windows() -> list[float]:
    with open(_CSV, newline="", encoding="utf-8") as f:
        return [float(r["total_window_ms"]) for r in csv.DictReader(f)]


class MeasuredConstantsTest(unittest.TestCase):
    def test_cap_and_floor_match_the_committed_measurement(self):
        w = measured_windows()
        self.assertLess(abs(max(w) - C.WINDOW_CAP_MS), C.WINDOW_MATCH_TOLERANCE_MS)
        self.assertLess(abs(min(w) - C.WINDOW_FLOOR_MS), C.WINDOW_MATCH_TOLERANCE_MS)

    def test_derived_rules_come_from_the_per_side_constants(self):
        self.assertEqual(C.ONE_SIDE_CAP_MS, C.WINDOW_CAP_MS / 2)
        self.assertEqual(C.CAP_FROM_GAP_MS, 2 * C.EXPECT_NORMAL_BACK_MS)        # 170
        self.assertEqual(C.FLOOR_UP_TO_GAP_MS, 2 * C.EXPECT_NORMAL_FRONT_S * 1000)  # 75


class PassiveEdgesTest(unittest.TestCase):
    def test_normal_mode_is_judged_against_the_measured_cap(self):
        self.assertEqual(passive_probe.clamp_edges(precision=False),
                         [("one side", C.ONE_SIDE_CAP_MS), ("whole window", C.WINDOW_CAP_MS)])

    def test_precision_mode_has_no_measured_cap(self):
        self.assertEqual(passive_probe.clamp_edges(precision=True), [])


class PollVerdictTest(unittest.TestCase):
    def test_the_real_capped_run_reads_as_capped_not_unclamped(self):
        lines = poll_windows.window_verdict(measured_windows(), back_ms=85.0)
        text = "\n".join(lines)
        self.assertNotIn("NO CLAMP", text)
        self.assertIn("reached the measured cap", text)
        self.assertIn("reached the measured floor", text)

    def test_a_reading_past_the_cap_is_no_clamp(self):
        lines = poll_windows.window_verdict([100.0, C.WINDOW_CAP_MS + 1.0], back_ms=85.0)
        self.assertTrue(any("NO CLAMP" in l for l in lines))

    def test_precision_mode_says_the_cap_is_unknown(self):
        lines = poll_windows.window_verdict([60.0, 70.0], back_ms=40.0)
        self.assertTrue(any("no measured cap" in l for l in lines))


class WatchFloorTest(unittest.TestCase):
    def test_floor_runs_are_checked_against_the_measured_floor(self):
        notes = P.manifest("x", P.window_map().notes)["notes"]
        samples = [WW.Sample(0.0, 0.0, 0, 0.0)] + [
            WW.Sample(float(n["time_ms"]),
                      C.WINDOW_FLOOR_MS if (n["gap_after_ms"] or 999) <= 75 else 100.0, 0, 0.0)
            for n in notes]
        lines = WW.window_report(samples, notes)
        self.assertEqual(sum("matches the floor" in l for l in lines), 4)


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run and watch them fail.** `python -m pytest tools/ch_probe/tests/test_s2_window_constants.py -q` fails on the missing `C.WINDOW_CAP_MS`, `clamp_edges` and `window_verdict`.

- [ ] **Step 3: The constants.** In `tools/ch_probe/constants.py`, add after `CONST_MATCH_TOLERANCE_MS` (line 154):

```python
# --- Measured whole-window cap and floor (normal mode) ------------------------
#
# Clone Hero 1.1.0.6142 sets each note's window from both neighbouring gaps.
# 0x20F7210 halves each gap and holds the half between the front and back
# constants (37.5 and 85 ms). The two halves are summed and passed to the
# window formula (RVA_WINDOW_FORMULA, 0x20DDDA0). So the formula's input runs
# from 75 to 170 ms, and its output from 96.2932 to 171.4313 ms: the cap is a
# clamp on the input, not on the stored window. The formula with the DLL's
# constants gives 171.4313075 at 170 and 96.2931672 at 75, and
# experiments/results/poll_windows.csv agrees: it tops out at 171.431308 ms
# (6 rows) and bottoms out at 96.293167 ms (22 rows). Evidence:
# docs/audit/ch-evidence.md, section 77.
# Normal mode only; nobody has read precision mode's cap.
WINDOW_CAP_MS = 171.4313
WINDOW_FLOOR_MS = 96.2932

# Half the whole window: the per-side reach at the widest spacing (85.72 ms,
# not the 85 ms back constant).
ONE_SIDE_CAP_MS = WINDOW_CAP_MS / 2

# A note reads the cap when the gaps on both sides are at least this wide
# (each half-gap at the back constant), and the floor when both are at most
# this narrow (each half-gap at the front constant).
CAP_FROM_GAP_MS = 2 * EXPECT_NORMAL_BACK_MS               # 170
FLOOR_UP_TO_GAP_MS = 2 * EXPECT_NORMAL_FRONT_S * 1000     # 75

# How close a stored reading must be to count as the cap or the floor. The
# measured values agree with these to 0.00001 ms.
WINDOW_MATCH_TOLERANCE_MS = 0.01
```

`2 * EXPECT_NORMAL_FRONT_S * 1000` evaluates to exactly 75.0 in Python (checked), so `gap <= FLOOR_UP_TO_GAP_MS` treats a 75 ms gap as a floor gap.

- [ ] **Step 4: `watch_window.py`.** Delete lines 45-48 (the comment and the two `CAP_*` constants). In `window_report`, replace lines 182-184:

```python
        if gap >= CAP_CHECK_FROM_GAP_MS:
            ok = len(vals) == 1 and abs(vals[0] - CAP_EXPECT_MS) < 0.01
            verdict = "  matches the cap" if ok else f"  DIFFERENT from {CAP_EXPECT_MS}"
```

with:

```python
        if gap >= C.CAP_FROM_GAP_MS:
            verdict = _verdict(vals, C.WINDOW_CAP_MS, "cap")
```

Replace the floor block's line 192:

```python
        lines.append(f"  {gap:4d} ms gap: " + (", ".join(f"{v:.2f}" for v in vals) or "no value"))
```

with:

```python
        text = ", ".join(f"{v:.2f}" for v in vals) or "no value"
        verdict = _verdict(vals, C.WINDOW_FLOOR_MS, "floor") if gap <= C.FLOOR_UP_TO_GAP_MS else ""
        lines.append(f"  {gap:4d} ms gap: {text}{verdict}")
```

Change the floor heading (line 188) to `f"Step 2, the floor: every gap of {C.FLOOR_UP_TO_GAP_MS:.0f} ms or less should read the measured floor."`. It prints "75 ms" as before, but reads the number from the constant beside it rather than repeating it. Add above `window_report`:

```python
def _verdict(vals: list[float], expect_ms: float, name: str) -> str:
    """'  matches the cap' when the run held one value at expect_ms."""
    ok = len(vals) == 1 and abs(vals[0] - expect_ms) < C.WINDOW_MATCH_TOLERANCE_MS
    return f"  matches the {name}" if ok else f"  DIFFERENT from {expect_ms}"
```

- [ ] **Step 5: `passive_probe.py`.** Add above `run_passive_probe`:

```python
def clamp_edges(precision: bool) -> list[tuple[str, float]]:
    """The edges to judge a clamp against, from the measured constants.

    Normal mode: one side (half the whole window) and the whole window.
    Precision mode: none, because nobody has read its cap.
    """
    if precision:
        return []
    return [("one side", constants.ONE_SIDE_CAP_MS), ("whole window", constants.WINDOW_CAP_MS)]
```

Replace lines 162-171:

```python
    # Judge against the edge of the mode actually being probed: precision
    # mode's back window is 40 ms, not normal mode's 85.
    back_ms = (constants.EXPECT_PRECISION_BACK_MS if engine.precision_mode()
               else constants.EXPECT_NORMAL_BACK_MS)
    verdicts = []
    for label, cap_ms in (("one side", back_ms), ("whole window", 2 * back_ms)):
        verdict = analysis.clamp_verdict(rows, cap_ms=cap_ms)
        _print_verdict(verdict, label)
        verdicts.append(verdict)
    return verdicts
```

with:

```python
    # Judge against the measured edges of the mode actually being probed.
    edges = clamp_edges(engine.precision_mode())
    if not edges:
        print("Precision mode: no measured cap yet, so no clamp verdict.")
    verdicts = []
    for label, cap_ms in edges:
        verdict = analysis.clamp_verdict(rows, cap_ms=cap_ms)
        _print_verdict(verdict, label)
        verdicts.append(verdict)
    return verdicts
```

In `_print_verdict` (line 208), `{verdict.cap_ms:.0f}` becomes `{verdict.cap_ms:.2f}`, so the edge prints as 171.43 rather than 171. Update the module docstring's last paragraph (lines 22-25) to end "...the verdict is given against the measured whole window (constants.WINDOW_CAP_MS) and its half."

- [ ] **Step 6: `poll_windows.py`.** Add above `main`:

```python
def window_verdict(windows: list[float], back_ms: float) -> list[str]:
    """Plain-English verdict lines for a run's stored windows, in ms.

    Judged against the measured normal-mode cap and floor (constants.py).
    `back_ms` is the engine's live back constant; when it is not the normal
    85 ms the game is in precision mode, whose cap nobody has read.
    """
    tol = constants.WINDOW_MATCH_TOLERANCE_MS
    w_min, w_max = min(windows), max(windows)
    if abs(back_ms - constants.EXPECT_NORMAL_BACK_MS) > constants.CONST_MATCH_TOLERANCE_MS:
        return [f"  Back window is {back_ms:.1f} ms, not the normal "
                f"{constants.EXPECT_NORMAL_BACK_MS:.0f}: precision mode, "
                "which has no measured cap yet."]
    lines = [f"  Measured cap {constants.WINDOW_CAP_MS} ms, floor {constants.WINDOW_FLOOR_MS} ms."]
    if w_max > constants.WINDOW_CAP_MS + tol:
        lines.append(f"  *** Window EXCEEDED the measured cap: max {w_max:.3f} ms. NO CLAMP. ***")
    elif abs(w_max - constants.WINDOW_CAP_MS) <= tol:
        lines.append("  The window reached the measured cap and never passed it.")
    elif len(set(round(w, 2) for w in windows)) == 1:
        lines.append("  Window never changed: either notes were uniform or no notes hit.")
    else:
        lines.append("  The window stayed below the cap: the song may have had no gaps of "
                     f"{constants.CAP_FROM_GAP_MS:.0f} ms or more.")
    if abs(w_min - constants.WINDOW_FLOOR_MS) <= tol:
        lines.append("  It also reached the measured floor (gaps of "
                     f"{constants.FLOOR_UP_TO_GAP_MS:.0f} ms or less).")
    return lines
```

The floor line says "reached the measured floor", which the test looks for. Every number in these messages (85, 75 and the cap gap) is read from `constants.py`, so the messages can't drift from the constants they describe. Replace lines 120-131 of `main` (from `print(f"  Window range: ...` through the last `print` of the old verdict) with:

```python
    print(f"  Window range: {w_min:.3f} — {w_max:.3f} ms")
    print(f"  Back window (constant): {back_ms:.1f} ms")
    for line in window_verdict(windows, back_ms):
        print(line)
```

Change the module docstring's lines 9-10 to: "Clone Hero caps the whole window at a measured 171.43 ms (constants.WINDOW_CAP_MS). A stored value above that means no cap; values that top out there mean the cap held."

- [ ] **Step 7: The existing test reads the constants.** These are the edits the change requires in `tools/ch_probe/tests/test_hit_window_scripts.py`. Line 41 packs `0.17143` and line 47 expects `171.43`; they become `C.WINDOW_CAP_MS / 1000` and `C.WINDOW_CAP_MS`. Line 81's expected line becomes `[f"   400 ms gap: 400.00  DIFFERENT from {C.WINDOW_CAP_MS}"]`. Line 89 becomes `notes, lambda n: C.WINDOW_CAP_MS if (n["gap_after_ms"] or 0) >= C.CAP_FROM_GAP_MS else (C.WINDOW_FLOOR_MS if (n["gap_after_ms"] or 999) <= C.FLOOR_UP_TO_GAP_MS else 100.0))`. Without the floor branch, line 92's `assertFalse(any("DIFFERENT" in l ...))` would trip on the new floor verdicts, because that fixture fed 100 to the floor runs. Lines 101 and 103 use `C.WINDOW_CAP_MS` in place of `171.43`. Line 83's floor expectation `["    30 ms gap: 30.00"]` gains the new floor verdict: `[f"    30 ms gap: 30.00  DIFFERENT from {C.WINDOW_FLOOR_MS}"]`. That fixture deliberately feeds the gap as the window, so "DIFFERENT" is the right answer there.

One detail matters for every cap and floor check. `steady_value` rounds each reading to two decimals before the verdict sees it (line 131), so a real cap reads 171.43 and a real floor 96.29. Both sit within `WINDOW_MATCH_TOLERANCE_MS` (0.01) of the constants, which is why the tolerance is 0.01 and not tighter.

- [ ] **Step 8: Run the whole probe suite.** `python -m pytest tools/ch_probe/tests -q` → all pass. `test_runners.py`'s passive cases (lines 86-108) still pass: `FakeProcess` reports normal mode, so `clamp_edges(False)` gives two edges.

- [ ] **Step 9: The plan document (D28).** In `docs/superpowers/plans/2026-09-25-hit-window-testing.md`, replace the opening paragraph (lines 3-7) with:

```markdown
This plan picks up from the 2026-09-25 live session. **Status, 2026-10-03
(decision D28):** steps 1-3 are answered from Clone Hero's code and the
committed `poll_windows.csv`, with no live run. Steps 4-6 are shelved. Hydra
keeps the fixed 85 ms per side.

The answers. Step 1: the whole window caps at 171.43 ms. Step 2: the floor
is real, but it reads 96.29 ms, not the 75 this plan expected, because the
clamp sits on the formula's input, not its output. Step 3: the window uses
both gaps; each is halved and held between 37.5 and 85 ms, and the two
halves are summed. The evidence (addresses 0x20F7210 and 0x20DDDA0, the
formula's constants, and the CSV rows) is in
`docs/audit/ch-evidence.md`, section 77.
The values live in `tools/ch_probe/constants.py` (WINDOW_CAP_MS,
WINDOW_FLOOR_MS), which every probe script reads.
```

Add one line under each remaining step heading. Under "### 1–3." (line 41): "*Answered 2026-10-03; see the status at the top. Kept for the record.*" Under "### 4." (line 65), "### 5." (line 76) and "### 6." (line 99): "*Shelved 2026-10-03 (D28). Run only if Hydra's fixed 85 ms per side is questioned again. Step 5 is the one that would test it: the measured cap says the per-side reach is about 85.72 ms.*" Change line 55's "The floor is real if every value under 75 ms reads 75." to "The floor is real if every value at 75 ms or under reads 96.29 (it does; see the top)."

- [ ] **Step 10: Commit** `tools/ch_probe/constants.py tools/ch_probe/experiments/watch_window.py tools/ch_probe/experiments/passive_probe.py tools/ch_probe/experiments/poll_windows.py tools/ch_probe/tests/test_hit_window_scripts.py tools/ch_probe/tests/test_s2_window_constants.py docs/superpowers/plans/2026-09-25-hit-window-testing.md` by name: "Probe tools read the measured 171.43 ms cap and 96.29 ms floor; hit-window plan steps 1-3 answered (77, D28)".

**Done when:**
- [ ] `python -m pytest tools/ch_probe/tests -q` passes in full.
- [ ] `git grep -n "2 \* back_ms\|2\*back_ms\|CAP_EXPECT_MS" tools/ch_probe` prints nothing, and `git grep -n "171\.43" -- "tools/ch_probe/*.py"` shows only `constants.py` and the `poll_windows.py` docstring. No script or test uses either number as a value of its own.
- [ ] The plan file's top paragraph states the D28 status, and steps 4-6 are marked shelved.
- [ ] The full `hydra_tests` passes and the lane corpus check at Expert prints nothing. This task touches no C++, so both are formalities; run them once so the wave's merge has the same evidence for every lane.
- [ ] No stamp is bumped.

---

### Task T9: Stamps and records (last)

**Goal:** The three stored-data stamps step 2 needs move once, the results-stamp rule names the chart readers, and every rule step 2 decided without code is written down where the next reader looks.

**Decisions:** D23 ("The written `kResultsStamp` rule names `src/parse` ... Step 2 reuses step 1 Task 22's value and does not bump twice"); D19, D20 and D24 for the dynamics count stamp; D24 for the blob stamp; D26, D27 (141, 320), D29 (304), D30 and the dnoflip rule for the records.

**Why this task is last.** The stamps say "everything saved before this point is out of date". If they moved before the parser changes landed, a build in between would stamp old-rule counts as current. So T9 is written alongside the wave but merges after T8.

**How the stamps work today.** `src/store/stored_versions.h` holds every stamp (ADR 0018). Three matter here.

`kResultsStamp` (line 46) guards saved analysis results. Its comment (lines 38-42) says to bump it when "the engine (src/search), the scoring (src/core), or what a record holds" changes. The parser isn't on that list, which is finding 344. This plan's parser changes move scores, so without D23 old results would keep reading Ready with old numbers. Step 1's Task 22 already sets this stamp to the release that ships step 1. D23 says step 2 shares that value, so T9 changes the comment and not the value.

`kDynamicsCountStamp` (line 67, value 1) guards saved Dynamics counts. T1, T2 and T5 change what the parser counts, so it goes to 2. A row stamped 1 then reads as missing, and the Dynamics tab recounts it in the background.

`kDynamicsBlobStamp` (line 70, value 1) is the first byte of every saved Dynamics blob. T5 adds two fields to the blob, so it goes to 2.

```cpp
// src/store/stored_versions.h at 1abd188, lines 38-46
// The analysis version. BUMP IT, to the version of the release that ships the
// change, whenever analysis output changes in a way the path format and the
// hydra_rules.ini fingerprint don't already catch: the engine (src/search),
// the scoring (src/core), or what a record holds. Then shrink `accepted` to
// the new stamp alone. A stale result reads Stale and asks for re-analysis.
//
// "1.8.3" is accepted because 1.8.3 stamped its app version on results that
// are identical to 1.8.2's.
inline constexpr StampRule<std::string_view, 2> kResultsStamp{"1.8.2", {"1.8.2", "1.8.3"}};
```

```cpp
// lines 58-70
// ---- Dynamics counts (the dynamics table) ----------------------------------

// How notes are counted. BUMP IT (add 1) whenever count_dynamics, or the
// parser that feeds it, changes what any chart counts: src/parse/song.cpp
// (the .mid, .chart and .sng loaders), src/parse/midi.cpp and
// src/parse/srb.cpp. The hydra_rules.ini fingerprint doesn't apply: the rules
// move activation marks, never which notes a chart has. A stale count reads as
// missing, and the Dynamics tab recounts it in the background.
// 0 = rows saved before the stamp existed. 1 = the first stamp.
inline constexpr StampRule<int, 1> kDynamicsCountStamp{1, {1}};

// The dynamics blob's byte layout. BUMP IT when encode_dynamics changes.
inline constexpr StampRule<uint8_t, 1> kDynamicsBlobStamp{1, {1}};
```

**Files.** Each step below quotes the lines it replaces.

The stamps are in `src/store/stored_versions.h`: the `kResultsStamp` comment changes, and `kDynamicsCountStamp` and `kDynamicsBlobStamp` move to 2. One existing test breaks on purpose: `tests/test_dynamics_store.cpp`'s "version 2 blob" subcase, because once the layout is 2 a blob stamped 2 is current, so the subcase must stamp a layout newer than this build's. The new tests go in `tests/test_s2_stamps.cpp`, plus one line in the `hydra_tests` list in `CMakeLists.txt` (after line 429).

The records are four docs. ADR 0018 (`docs/adr/0018-stored-data-has-one-set-of-version-stamps.md`) changes its `kResultsStamp` bullet and its Consequences paragraph. A new ADR, `docs/adr/0023-chart-rules-follow-clone-hero-code.md`, records the chart rules; use the next free number if step 1 took a different range (its plan names 0021 and 0022). `CONTEXT.md` rewrites "SP phrase", "Fill spawn deadline (CH 1.1)" and "Gem", and gains "Authored fill", "Disco flip", "2x kick", "Backend leeway" and "Generated fill". `docs/differences-from-public-hydra.md` gains one paragraph in "Numbers that can differ".

**Dependencies:** none inside the wave. T5 Step 6 fixes the names of its two blob fields (`late_tag_ms` and `marks_before_tag`), which T9's comment names. At the merge to `main`: step 1 Task 22's value for `kResultsStamp` and its ADR numbers.

**Acceptance Criteria:**
- [ ] `kDynamicsCountStamp` is `{2, {2}}` and `kDynamicsBlobStamp` is `{2, {2}}`, each with a history line saying what step 2 changed; the `static_assert`s compile.
- [ ] The `kResultsStamp` comment names `src/parse`, and its value line is unchanged from the base.
- [ ] A saved Dynamics count stamped 1 reads as missing, and a blob whose first byte is 1 does not decode.
- [ ] ADR 0018, the new ADR, CONTEXT.md and the differences page say what the code now does, in plain English.
- [ ] Every existing test passes, with the one named edit in `tests/test_dynamics_store.cpp`.

**Verify:**
```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t9
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe --test-case="step 2:*,decode_dynamics*,RecordStore dynamics*"
.\build-cpp\Release\hydra_tests.exe
```
Expected: the first run reports all passed; the second reports 0 failed.

**Steps:**

- [ ] **Step 1: Write the failing tests.** Create `tests/test_s2_stamps.cpp`:

```cpp
// Step 2's stamp moves, pinned. Every stamp on stored data lives in
// src/store/stored_versions.h (ADR 0018). Step 2 changed what the parser
// counts (D19 disco, D20 2x kicks, D24 the dynamics tag), so saved Dynamics
// counts from before it must read as missing; T5 added two fields to the
// Dynamics blob, so an old blob must not decode; and D23 says the rule for
// bumping the results stamp names the chart readers.

#include "doctest.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "app/dynamics_breakdown.h"
#include "store/stored_versions.h"

#ifndef HYDRA_SOURCE_DIR
#error "HYDRA_SOURCE_DIR must be defined (see CMakeLists.txt)"
#endif

using namespace hydra;

TEST_CASE("step 2: Dynamics counts saved before step 2 read as missing") {
    CHECK(store::kDynamicsCountStamp.written == 2);
    CHECK_FALSE(store::kDynamicsCountStamp.is_current(1));
    CHECK_FALSE(store::kDynamicsCountStamp.is_current(0));
}

TEST_CASE("step 2: a Dynamics blob in the old layout does not decode") {
    CHECK(store::kDynamicsBlobStamp.written == 2);
    std::vector<uint8_t> blob = app::encode_dynamics(app::DynamicsBreakdown{});
    REQUIRE(app::decode_dynamics(blob).has_value());
    blob[0] = 1;  // the layout before T5's two fields
    CHECK_FALSE(app::decode_dynamics(blob).has_value());
}

// D23: a change under src/parse that alters what a chart reads as must bump
// the results stamp. The rule is a comment, so this reads the comment: the
// block between the "Results" banner and the kResultsStamp line must name
// src/parse, next to src/search and src/core.
TEST_CASE("step 2: the results stamp's bump rule names the chart readers") {
    namespace fs = std::filesystem;
    const fs::path file =
        fs::u8path(HYDRA_SOURCE_DIR) / "src" / "store" / "stored_versions.h";
    std::ifstream in(file);
    REQUIRE(in.good());
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string text = ss.str();

    const size_t stamp = text.find("kResultsStamp{");
    REQUIRE(stamp != std::string::npos);
    const size_t banner = text.rfind("// ---- Results", stamp);
    REQUIRE(banner != std::string::npos);
    const std::string rule = text.substr(banner, stamp - banner);
    CHECK(rule.find("src/search") != std::string::npos);
    CHECK(rule.find("src/core") != std::string::npos);
    CHECK(rule.find("src/parse") != std::string::npos);
}
```

Add `    tests/test_s2_stamps.cpp` to the `hydra_tests` list in `CMakeLists.txt`, after `tests/test_long_paths.cpp` (line 429).

- [ ] **Step 2: Run them and watch them fail.** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe --test-case="step 2:*"`. All three fail: both stamps are still 1, and the comment does not name `src/parse`.

- [ ] **Step 3: Move the two Dynamics stamps.** Replace lines 66-70 of `src/store/stored_versions.h` with the lines below. The blob comment names T5's two fields as T5 Step 6 defines them.

```cpp
// 0 = rows saved before the stamp existed. 1 = the first stamp.
// 2 = step 2: disco flip and 2x kicks are read per difficulty, and the
// dynamics tag counts only in Clone Hero's two spellings, in file order
// (decisions D19, D20 and D24, 2026-10-03).
inline constexpr StampRule<int, 1> kDynamicsCountStamp{2, {2}};

// The dynamics blob's byte layout. BUMP IT when encode_dynamics changes.
// 2 added late_tag_ms (a late dynamics tag's time in ms) and
// marks_before_tag (the marked notes before it), for the "from <time> on"
// line (D24).
inline constexpr StampRule<uint8_t, 1> kDynamicsBlobStamp{2, {2}};
```

- [ ] **Step 4: Name the parser in the results rule.** Replace lines 38-42 with the lines below. Leave line 46, the value, exactly as it is. The value this plan ships with is step 1 Task 22's, and the merge to `main` brings it (Step 6).

```cpp
// The analysis version. BUMP IT, to the version of the release that ships the
// change, whenever analysis output changes in a way the path format and the
// hydra_rules.ini fingerprint don't already catch: the engine (src/search),
// the scoring (src/core), the chart readers (src/parse) when a chart reads
// differently, or what a record holds. Then shrink `accepted` to the new stamp
// alone. A stale result reads Stale and asks for re-analysis. Changes that
// ship in the same release share one bump (decision D23, 2026-10-03).
```

- [ ] **Step 5: Fix the one test the bump breaks.** In `tests/test_dynamics_store.cpp`, the subcase at lines 87-93 stamps a blob "version 2" and expects it to be refused. Version 2 is now this build's layout, so stamp one past it instead:

```cpp
    SUBCASE("a blob from a newer layout") {
        // A valid-length blob stamped with a layout this build doesn't know.
        const DynamicsBreakdown b = make_full_breakdown(true);
        std::vector<uint8_t> blob = encode_dynamics(b);
        blob[0] = static_cast<uint8_t>(kDynamicsBlobStamp.written + 1);
        CHECK_FALSE(decode_dynamics(blob).has_value());
    }
```

The other stamp tests need no edit. `tests/test_app_state.cpp` line 299 already writes `kDynamicsCountStamp.written - 1`, which is now 1 and still stale. `tests/test_dynamics_store.cpp` lines 217-243 read the stamp through `written`. `tests/test_store.cpp` lines 318-319 pin the results values, which T9 does not change.

Run Verify. All tests pass.

- [ ] **Step 6: The results value at the merge to `main`.** This step runs when `claude/s2-int` merges onto `main`, after step 1 has merged. `stored_versions.h` will conflict, because Task 22 changed line 46 and the paragraph about "1.8.3", and T9 changed lines 38-42 and 66-70. Keep Task 22's value line and its `accepted` list. Keep T9's rule comment and both Dynamics stamps. Then check that no release went out between Task 22 and this merge:

```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test
# Run on main before the step-2 merge: the newest commit that changed the
# kResultsStamp value line is step 1's Task 22.
$t22 = git log -n 1 --format=%H -G'kResultsStamp\{' main -- src/store/stored_versions.h
git show --stat --oneline $t22 | Select-Object -First 1   # check it is Task 22's commit
git tag --contains $t22
```

Expected: the first line names Task 22's stamp commit, and `git tag --contains` prints nothing, meaning no release carries Task 22's stamp without step 2. Then step 2 ships in the same release and shares the bump, as D23 says.

If a tag is printed, a release shipped step 1 alone. Then the merge stops, and the integrator asks the user a blocking question before anything lands on `main`: "A release already shipped step 1. Bump the results stamp again, so every saved result reads Stale a second time?" This departs from D23 ("does not bump twice"), and Appendix A lists it. Bumping is still the right fix, because otherwise old results computed with the old chart rules would keep reading Ready. But it is a second user-visible re-analysis, so the user decides first, not after.

On a yes, set `kResultsStamp` to the version in `CMakeLists.txt` line 20 (`project(Hydra VERSION ...)`) with `accepted` holding only it, and update `tests/test_store.cpp` lines 318-320 to that value. On a no, the merge waits until the user says how to proceed.

- [ ] **Step 7: ADR 0018.** Replace the `kResultsStamp` bullet (lines 24-28) and the Consequences paragraph (lines 47-53) so the scope names the parser:

```markdown
- `kResultsStamp`: the analysis version. It changes only when analysis
  output changes in a way the path format and the rules fingerprint don't
  catch: the engine, the scoring, the chart readers, or what a record holds.
  When it changes it is set to the version of the release that ships the
  change, and changes that ship together share one bump.
```

```markdown
The cost is a rule someone has to remember. A change to the engine, the
scoring, the chart readers or what a record holds that leaves the path
format and the rules alone must bump `kResultsStamp`, or old results keep
reading Ready when they are wrong. The chart readers were left off this list
at first; a reader that now reads a chart differently changes the result
just as surely as the engine does (audit finding 344, user decision D23,
2026-10-03). Check it at every release: if anything under `src/search`, the
scoring in `src/core`, the readers in `src/parse` or the record contents
changed since the last bump, bump it and shrink its `accepted` list to the
new stamp alone.
```

- [ ] **Step 8: The new ADR.** Create `docs/adr/0023-chart-rules-follow-clone-hero-code.md`. Check the number first (`git ls-tree --name-only main docs/adr/` after step 1 merges); use the next free one.

```markdown
# Chart rules follow Clone Hero's code, with the exceptions written here

Hydra reads a drum chart to decide which notes exist, which ones are
cymbals, ghosts or 2x kicks, and which ones pay Star Power. Until step 2
those rules came from the original Python port, and a few of them differed
from Clone Hero 1.1 in ways nobody had chosen.

In October 2026 Clone Hero 1.1's GameAssembly.dll was read statically (not
run) and each reading rule was compared with Hydra's. The evidence, with
code addresses, is in
`docs/audit/ch-evidence.md` and the
step-2 briefs.

## The decision

Where Clone Hero's code answers a reading question, Hydra gives the same
answer, through one owner in `src/parse/song.cpp`. The rules that changed:

- Disco flip is read per difficulty. Each difficulty uses only its own
  `[mix N drums...]` markers, in both formats (Clone Hero 0x215C750,
  0x213D076, 0x2155050). The flip happens only with Pro Drums on, and that
  gate is written once.
- A 2x kick is read per difficulty: MIDI 59 Easy, 71 Medium, 83 Hard, 95
  Expert, and `.chart` N 32 in its own section (0x21555CD, 0x210D9C0). The
  2x Bass setting works at every difficulty, like Clone Hero's Double Kick
  modifier, which is its only gate (0x20D32B7).
- A Star Power phrase pays on the last chord with start <= tick < end. A
  zero-length phrase pays nothing, and a phrase that runs past the last note
  pays on that note (0x20D2440).
- An authored fill lands on a chord up to floor(resolution x slop) + 1
  ticks after the fill ends (0x20D0088-0x20D008D). The +1 applies on top of
  a `hydra_rules.ini` slop too.
- Each authored fill is placed on its own once every chord is read
  (0x20CFF60 calling 0x5DE030): on the last chord at or before its end but
  not before its start, or on the first chord after its end inside the
  window. The closer wins, and a tie goes to the later chord. A fill with
  neither is dropped.
- The MIDI dynamics tag counts only as `ENABLE_CHART_DYNAMICS` or
  `[ENABLE_CHART_DYNAMICS]`, compared exactly, and at a shared tick the file
  order decides (0x21557A5, 0x21557BB). This assumes Clone Hero's MIDI
  reader does not trim the text first; that was not checked.
- A `.chart` cymbal, ghost or accent marker with no note at its tick is
  skipped instead of failing the chart (0x215DDB0). The case of a chord at
  that tick without that colour was not traced.

Rules that were already Clone Hero's and are now written down:

- `.chart` ghosts and accents always count, and a `.chart` always reports
  dynamics on (0x213D620, 0x215DBD0). There is no tag in `.chart`.
- `.chart` gameplay events (solo, soloend, disco) match the raw text after
  trimming whitespace. A quoted `E "solo"` is not a solo (0x213CAF0,
  0x20FE510). Practice-section names strip one pair of quotes, because that
  is how the format writes them. There is no shared normaliser for the two,
  on purpose.

## Deliberate differences from Clone Hero

- `drums0dnoflip` turns flip off. Clone Hero's on/off check looks only at
  the 7th character of the text after `mix N `, so it reads `dnoflip` as on
  (0x215CD50). That is a Clone Hero bug, and Hydra does not copy it. It
  affects 48 library charts. For the same reason Hydra keeps its own on/off
  reading of `drumsNeasy`, `drumsNeasynokick` and `drumsd`, which Clone
  Hero's one-character rule reads differently on 5 chart-and-difficulty
  pairs.
- The backend leeway is Hydra's own rule. No such constant was found in the
  engine methods read; the 3 ms is Hydra's own setting. A note less than `backend_leeway_ms` (3 ms) after the SP end scores under
  Star Power; a note exactly that far after it does not (decision D29).
- When a fill has a chord on only one side of its end, Clone Hero's
  0x5DE030 takes that chord however far away. Hydra keeps the window's
  bound and drops the fill (decision D30). No measured chart scores
  differently either way.

## Not verified against Clone Hero

These keep Hydra's current rule and are written down as unchecked (decision
D26). A live Clone Hero session could settle them.

- A generated fill's length, when two or more meter changes fall between two
  chords. Hydra's walk reads the meter one change late there.
- How many ticks make a beat in compound meters like 6/8. Hydra counts a
  beat as a quarter note (the chart's resolution) everywhere, including the
  4-beat fill deadline.
- Whether one authored fill turns off generated fills for the whole chart.
- Whether Clone Hero carries its search position from one fill to the
  next (its fill search takes a start index).

## Consequences

Lower-difficulty Pro Drums scores change on about 640 library charts each,
mostly up, because charters mark disco only on Expert. A few dozen other
charts move for the other rules. Every saved result and every saved
Dynamics count is redone once.

A future Clone Hero version can change any of these. The addresses above
are where to look first.
```

- [ ] **Step 9: CONTEXT.md.** Make these edits, keeping each entry's style.

After "Skip" (line 75), add:

```markdown
**Authored fill**:
A fill written in the chart. Each one is placed on its own once every chord
is read: on the last chord at or before its end but not before its start, or
on the first chord after its end within the landing window
(`fill_land_slop_beats`, plus one tick). The closer wins, a tie goes to the
later chord, and a fill with neither is dropped (docs/adr/0023).
```

Replace the "SP phrase" entry (lines 77-79) with:

```markdown
**SP phrase**:
A chart section that awards a bar of Star Power when hit fully. It covers
the chords with start <= tick < end, and pays on the last of them; a phrase
of length zero pays nothing, and one that runs past the last note pays on
that note (Clone Hero's rule, docs/adr/0023).
_Avoid_: star power section
```

Replace the "Fill spawn deadline (CH 1.1)" entry (lines 114-116) with:

```markdown
**Fill spawn deadline (CH 1.1)**:
The latest your SP meter can fill up and still have a fill appear. Clone Hero
1.1 puts it a flat 4 beats before the fill starts. Hydra's default rule. A
beat here is a quarter note (the chart's resolution) in every meter, 6/8
included; that is not verified against Clone Hero (docs/adr/0023).
```

After "Fill spawn deadline (CH 1.0)" (line 123), add:

```markdown
**Generated fill**:
A fill Hydra places itself on a chart with no authored fills: on the chord
nearest a downbeat, if one sits within half a beat of it, half a measure
long, and at least 4 measures after the last one (hydra_rules.ini can change
these). With two or more meter changes
between two chords, its length reads the earlier meter; that is not
verified against Clone Hero (docs/adr/0023).
```

After "Squeeze rating" (line 151), add:

```markdown
**Backend leeway**:
How long after the SP end a note still scores under Star Power without a
squeeze: less than `backend_leeway_ms` (3 ms by default). A note exactly
3.0 ms after the SP end does not score under SP. Hydra's own rule: no such
constant was found in the Clone Hero engine methods read; the 3 ms is
Hydra's own setting.
```

In "Gem" (lines 170-176), replace the dynamics sentence ("Dynamics are a velocity rule ... (see docs/adr/0012).") with:

```markdown
in an SP phrase. Dynamics are a velocity rule that applies to every lane, kick
included: velocity 1 is a ghost, velocity 127 an accent, and both score double
(see docs/adr/0012). A MIDI chart turns them on with the exact text
`ENABLE_CHART_DYNAMICS` or `[ENABLE_CHART_DYNAMICS]` in its drum track, and
only notes after the tag count; a `.chart` always has them on.
```

After "Lane" (line 180), add:

```markdown
**Disco flip**:
A section where the chart swaps the red and yellow lanes, marked by text
events such as `[mix N drums0d]` (on) and `[mix N drums0]` (off), where N names the
difficulty: 0 Easy, 1 Medium, 2 Hard, 3 Expert. Each difficulty reads only
its own markers, and the flip happens only with Pro Drums on.
`drums0dnoflip` reads as off, unlike Clone Hero (docs/adr/0023).

**2x kick**:
A kick written for a double bass pedal: MIDI 59 Easy, 71 Medium, 83 Hard,
95 Expert, or `N 32` in a `.chart` difficulty section. Each difficulty reads
its own. The 2x Bass setting turns them on at every difficulty, like Clone
Hero's Double Kick modifier.
```

- [ ] **Step 10: The differences page.** In `docs/differences-from-public-hydra.md`, change "There are two reasons." (line 62) to "There are three reasons." and add this paragraph after the squeeze paragraph (line 66):

```markdown
**Chart rules matched to Clone Hero's code.** In 2.x this version read Clone
Hero 1.1's own code and matched how it reads a chart. Below Expert, disco
flip and 2x kicks now follow the difficulty you play, not Expert's markers.
A Star Power phrase pays on the notes Clone Hero pays at its two edges. An
authored fill lands one tick later, and each fill is placed on its own,
so a fill the next one used to hide now counts. A one-bracket dynamics tag no longer
turns dynamics on. This version's parser started as a port of public Hydra
1.x, checked chart by chart. Public Hydra's disco and 2x kick rules were
checked only in its 1.2.0 source, which still reads charts the old way;
whether a newer public version changed that was not checked. The biggest effect is on
Hard, Medium and Easy with Pro Drums, where about 640 charts each score
differently, mostly higher. One Clone Hero rule is deliberately not copied:
`drums0dnoflip` turns disco flip off here, as the marker says.
```

- [ ] **Step 11: Commit.** Two commits, staged by name. First `src/store/stored_versions.h`, `tests/test_s2_stamps.cpp`, `tests/test_dynamics_store.cpp` and `CMakeLists.txt` ("Step 2 stamps: Dynamics count and blob to 2; results rule names src/parse"). Then the four doc files ("Step 2 records: chart rules follow Clone Hero's code (ADR 0023)").

**Done when:**
- [ ] `hydra_tests` reports 0 failed in the `s2-t9` worktree, and again on `claude/s2-int` after every merge.
- [ ] The three "step 2:" tests failed before Steps 3-4 and pass after.
- [ ] `git diff 1abd188 -- src/store/stored_versions.h` shows no change to line 46.
- [ ] The corpus score check prints no line T9 could have caused (T9 changes no analysis).
- [ ] At the merge to `main`, Step 6's tag check printed nothing, or the user was asked before the merge and said yes to the second bump (a blocking question; Appendix A lists it as a departure from D23).

---

### Task T10: The squeeze-out end reads the cap (finding 37)

**Branches from:** `main`, after step 1 has merged. Everything below quotes 1abd188, the last code this plan's drafter could read. Step 1's Tasks 11, 12, 5 and 17 rewrite these functions, so Step 0 re-reads them, and the edits are located by function name and by what the code does.

**Goal:** When a deactivation edge asks "where would this SP end have gone if the phrase before it had been collected?", it gets `extend_deacts`' answer, cap included. A path whose pre-phrase end was a different tick is never offered that squeeze-out.

**Decision:** D29: "37: the SP-end extension during Star Power comes only from `extend_deacts`, which stops at a full meter, as Clone Hero does (0x20F6800); counted after the merge."

**How it works today, in plain words.** Think of each pending SP end as a parking meter's expiry time. When the player hits a phrase during Star Power, every meter gets two more measures, but no meter may run past the cap's ceiling: the phrase's own note plus two measures per bar of cap. `ScoreGraph::extend_deacts` knows both rules.

```cpp
// src/search/graph.cpp at 1abd188, lines 267-276
    Timecode ceiling = plusmeasure(sp_timecode, sp_bars_to_measures(*sp_meter_cap_));
    for (const Timecode& tc : deact_tcs) {
        Timecode ext = plusmeasure(tc, sp_bars_to_measures(1));
        // min(ext, ceiling): ceiling only when it is strictly earlier. When
        // it wins, the meter was full to the cap on this phrase, and from
        // here on the SP end is measured from this collecting note (a tie
        // leaves the previous anchor in charge, like the engine's own min).
        const bool clamped = ceiling.ticks() < ext.ticks();
        out.push_back({tc, clamped ? ceiling : ext, clamped});
    }
```

The graph also keeps the old end D as a node, so a path can squeeze the phrase out and stop at D. At that node, `add_deact_edge` writes down which end a path must have for the squeeze-out to apply. It adds a plain bar and forgets the ceiling:

```cpp
// src/search/graph.cpp at 1abd188, lines 360-365
        if (recent_backend.is_sp && !deact_edge->sqinout_time.has_value()) {
            deact_edge->sqinout_time = recent_backend.timecode;
            deact_edge->sqinout_timing = offset_ms;
            deact_edge->sqout_time = plusmeasure(*deact_edge->sqout_time, sp_bars_to_measures(1));
            deact_edge->sqin_time = plusmeasure(*deact_edge->sqin_time, sp_bars_to_measures(1));
        }
```

The engine then offers the squeeze-out only to a path whose end equals that number:

```cpp
// src/search/engine.cpp at 1abd188, lines 587-592
int32_t Engine::deactivation_type(const EdgeView& e, int64_t sp_end_time) const {
    if (e.sqinout_time != NO_TIME) {
        return sp_end_time == e.sqout_time ? DEACT_SQINOUT : DEACT_NONE;
    }
    return sp_end_time == node(e.dest).tick ? DEACT_NORMAL : DEACT_NONE;
}
```

When the cap clamped, the path's end is the ceiling, the edge expects the plain bar, and they never match. So the squeeze-out path silently vanishes. The audit reproduced it on a synthetic chart with 100 ms measures at cap 2. It needs a phrase more than 2(cap − 1) measures before the end and less than 500 ms before it, so on real charts only very short measures at low what-if caps can reach it.

**The fix, and the trap in it.** `add_deact_edge` asks `extend_deacts` for the moved end, so the two can't disagree. Calling the owner gives exactly the entry the phrase's own advance edge carries, because `extend_deacts` treats each end on its own.

The trap: once ends can clamp, two different pending ends can move to the same ceiling. Say path A's end was D and path B's was D2, both shortly after the phrase. Both collect it and both now end at the ceiling. At node D, the edge expects the ceiling, so B would be offered "squeeze the phrase out and stop at D", which B can't do: without the phrase B stops at D2. With a plain bar this never happened, because D + 2 measures and D2 + 2 measures are different ticks. So the engine also remembers, for a clamped window, which end the clamping phrase moved, and the squeeze-out at D goes only to the path that moved from D.

After step 1's Task 12, the late-SqIn bump no longer lives in `store_new_backend`; `add_deact_edge` claims the one squeeze-out chord through `core::sqout_chord` and handles both sides. The fix covers both sides there. A phrase after the end can never reach the ceiling (the ceiling sits 2 x cap measures after a note that is later than the end), so the late side's answer never changes; it just comes from the same owner.

**Files.** Each step below names the lines it changes, at 1abd188 unless it says otherwise; Step 0 re-reads them on the new `main`.

The graph side is in `src/search/graph.h` and `graph.cpp`. `ScoreGraphEdge` gains a `sqout_clamped` flag, `SpExtension`'s comment says where the end comes from, and `add_deact_edge` reads `extend_deacts`' end (after step 1 Task 12 it is the version that calls `core::sqout_chord`).

The engine side is in `src/search/engine.cpp`. `EdgeView` carries the flag, `Path` remembers which end a clamp moved, and the advance loop and the three `clamp_tick = NO_TIME` resets keep it. `deactivation_type` and its one caller take the path, so the squeeze-out goes only to the path that moved.

The tests go in the new `tests/test_s2_deact_extension.cpp`, plus one line in the `hydra_tests` list in `CMakeLists.txt`. If Step 9 needs its own stamp bump, `src/store/stored_versions.h` and `tests/test_store.cpp` change too, and only on the user's yes.

**Acceptance Criteria:**
- [ ] On the fast fixture at cap 2, the deactivation edge at tick 5376 expects 6528 (the clamped end), not 6912, and says it was clamped.
- [ ] A search at cap 2 lists a path that squeezes out the phrase at 3456 and ends Star Power at 5376.
- [ ] With two activations whose ends 5376 and 5568 both clamp to 6528, the 2304 activation's squeeze-out ends at 5376, the 2496 activation's at 5568, and neither ends at the other's tick.
- [ ] At cap 3 (no clamp) the same fixture gives the same squeeze-out before and after the change.
- [ ] Every existing test passes; the cap-4 corpus score file is byte-identical to `main`'s.
- [ ] The user has seen the Step 7 count and said yes.

**Verify:**
```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t10
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe --test-case="finding 37:*"
.\build-cpp\Release\hydra_tests.exe
.\build_cpp.ps1 -Target hydra_replay; .\build-cpp\Release\hydra_replay.exe selfcheck
```
Expected: the first run reports all passed; the full run 0 failed; `selfcheck` no FAIL line.

**Steps:**

- [ ] **Step 0: Re-read the code as step 1 left it.** On the new `main`, read `ScoreGraph::add_deact_edge`, `ScoreGraph::extend_deacts`, `Engine::deactivation_type`, `Engine::branch_deactivate`, the advance loop that walks `sp_times`, and the engine's `Path` struct. Note three things before editing. First, whether `Path` still has `clamp_tick` (step 1 Task 5 may track the window's history as steps instead; if so, "the clamping phrase" is the last Clamped step's tick and "the end it moved from" is the end of the step before it). Second, the names `Activation` now uses for the SP end and the squeezed-out chord (step 1's tests use `act.deact_tick()` and `act.sqout_tick`). Third, whether a search with `depth_value` above 0 returns losing paths, which the tests below rely on. Write the three answers in the task's notes, and adjust the code below to them without changing what it does.

- [ ] **Step 1: Write the failing tests.** Create `tests/test_s2_deact_extension.cpp`:

```cpp
// Finding 37 (decision D29): when a phrase is collected during Star Power,
// the SP end moves by extend_deacts' rule, which stops at the cap's ceiling,
// as Clone Hero does (0x20F6800). The deactivation edge's squeeze-out must
// read that same answer, and must not hand a clamped squeeze-out to a path
// whose end before the phrase was a different tick.

#include "doctest.h"

#include <cstdint>
#include <optional>
#include <set>
#include <vector>

#include "core/model.h"
#include "parse/song.h"
#include "search/engine.h"
#include "search/graph.h"

using namespace hydra;

namespace {

struct FastNote {
    int64_t tick;
    bool sp_phrase = false;
    bool activation = false;
};

// 4/4 at 2400 BPM with 192 ticks per beat: a measure is 768 ticks and
// 100 ms, so an SP bar (two measures) is 200 ms and fits inside the 500 ms
// squeeze window. Only then can the cap clamp an end a squeeze-out still
// reaches. Built by hand, like test_search.cpp's build_tail_song.
Song build_fast_song(const std::vector<FastNote>& notes) {
    Song song(192);  // the constructor sets 4/4: 768 ticks per measure
    song.bpm_changes[0] = 2400.0;
    song.build_timing();
    for (const FastNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.sp_phrase;
        if (n.activation) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// Two phrases bank 2 bars; the activation at 2304 ends 4 measures later at
// 5376. The phrase at 3456 is collected mid-SP, 250 ms before 5376. A plain
// bar would move the end to 5376 + 1536 = 6912, but at cap 2 the ceiling is
// 3456 + 4 * 768 = 6528, which is earlier, so the end clamps to 6528.
const std::vector<FastNote> kOneActivation = {
    {0, true}, {768, true}, {2304, false, true}, {3456, true},
    {4608}, {6000}, {6528}, {7000}, {7680}};

// The same, plus a second fill at 2496. Activating there instead ends at
// 5568, also inside the window after 3456, and that end clamps to 6528 too.
const std::vector<FastNote> kTwoActivations = {
    {0, true}, {768, true}, {2304, false, true}, {2496, false, true},
    {3456, true}, {4608}, {6000}, {6528}, {7000}, {7680}};

// The deactivation edge whose node sits at `tick`, walking the SP track the
// way test_search.cpp's sqout_chord test does.
const ScoreGraphEdge* deact_edge_at(const ScoreGraph& graph, int64_t tick) {
    const ScoreGraphNode* sp = nullptr;
    for (const ScoreGraphNode* b = graph.start(); b && !sp;
         b = b->adv_edge ? b->adv_edge->dest : nullptr)
        if (b->branch_edge) sp = b->branch_edge->dest;
    for (; sp; sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
        const ScoreGraphEdge* e = sp->branch_edge;
        if (e && e->dest->timecode.ticks() == tick) return e;
    }
    return nullptr;
}

struct SqOutSeen {
    int64_t act_tick;
    int64_t end_tick;
};

// Every activation, in every returned path and its tied variants, that
// squeezed out the phrase at `phrase_tick`: where it activated and where its
// Star Power ended.
std::vector<SqOutSeen> sqouts_of(const std::vector<Path>& paths, int64_t phrase_tick) {
    std::vector<SqOutSeen> out;
    auto scan = [&](const Path& p) {
        for (const Activation& act : p.all_activations()) {
            if (!act.sqout_tick || *act.sqout_tick != phrase_tick) continue;
            const std::optional<int64_t> end = act.deact_tick();
            REQUIRE(end.has_value());
            out.push_back({act.timecode.ticks(), *end});
        }
    };
    for (const Path& p : paths) {
        scan(p);
        for (const Path& v : p.variants) scan(v);
    }
    return out;
}

EngineOptions keep_losers() {
    EngineOptions o;
    o.depth_mode = DepthMode::Scores;
    o.depth_value = 50;  // keep the squeeze-out even though it scores lower
    return o;
}

}  // namespace

TEST_CASE("finding 37: the squeeze-out edge expects extend_deacts' clamped end") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const ScoreGraphEdge* e = deact_edge_at(graph, 5376);
    REQUIRE(e != nullptr);
    REQUIRE(e->sqinout_time.has_value());
    CHECK(e->sqinout_time->ticks() == 3456);
    // min(5376 + 1536, 3456 + 4 * 768) = 6528, and the ceiling won.
    REQUIRE(e->sqout_time.has_value());
    CHECK(e->sqout_time->ticks() == 6528);  // was 6912
    CHECK(e->sqin_time->ticks() == 6528);
    CHECK(e->sqout_clamped);
}

TEST_CASE("finding 37: a clamped window still offers its squeeze-out") {
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 2);
    const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, keep_losers()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) {
        CHECK(s.act_tick == 2304);
        CHECK(s.end_tick == 5376);
    }
}

TEST_CASE("finding 37: two ends clamped to one tick each keep their own squeeze-out") {
    const Song song = build_fast_song(kTwoActivations);
    const ScoreGraph graph(song, 2);
    for (int64_t act : {int64_t{2304}, int64_t{2496}}) {
        // A targeted search: only this activation exists, so no other path
        // can crowd it out of its group, and a wrong squeeze-out would show.
        EngineOptions o = keep_losers();
        o.target_act_ticks = std::vector<int64_t>{act};
        const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, o), 3456);
        const int64_t own_end = act == 2304 ? 5376 : 5568;
        INFO("activation at " << act);
        REQUIRE_FALSE(seen.empty());
        for (const SqOutSeen& s : seen) {
            CHECK(s.act_tick == act);
            CHECK(s.end_tick == own_end);  // never the other activation's end
        }
    }
}

TEST_CASE("finding 37: an unclamped window is unchanged") {
    // At cap 3 the ceiling is 3456 + 6 * 768 = 8064, later than 6912, so
    // nothing clamps and the plain bar is extend_deacts' answer too.
    const Song song = build_fast_song(kOneActivation);
    const ScoreGraph graph(song, 3);
    const ScoreGraphEdge* e = deact_edge_at(graph, 5376);
    REQUIRE(e != nullptr);
    REQUIRE(e->sqout_time.has_value());
    CHECK(e->sqout_time->ticks() == 6912);
    CHECK_FALSE(e->sqout_clamped);
    const std::vector<SqOutSeen> seen = sqouts_of(run_search(graph, keep_losers()), 3456);
    REQUIRE_FALSE(seen.empty());
    for (const SqOutSeen& s : seen) CHECK(s.end_tick == 5376);
}
```

Add `    tests/test_s2_deact_extension.cpp` to the `hydra_tests` list in `CMakeLists.txt`.

- [ ] **Step 2: Run them and watch them fail.** `.\build_cpp.ps1 -Target hydra_tests` fails to compile, because `ScoreGraphEdge` has no `sqout_clamped`. Add the field (Step 3's first block) and run `--test-case="finding 37:*"` again. The first case fails with 6912 instead of 6528. The second and third fail at `REQUIRE_FALSE(seen.empty())`, because no squeeze-out exists at cap 2. The fourth passes; it is the control.

- [ ] **Step 3: The edge reads the owner.** In `src/search/graph.h`, after `sqin_time` (line 100):

```cpp
    // True when the SP cap's ceiling, not the plain extension, set
    // sqout_time: the claimed phrase filled the meter. A clamped end is the
    // same tick whichever end the phrase moved, so the engine also checks
    // that the path's end before that phrase was this edge's node
    // (finding 37).
    bool sqout_clamped = false;
```

Extend `SpExtension`'s comment (lines 54-57) with one sentence: "The deactivation edge asks extend_deacts for the same answer when it prices a squeeze-out, so the two never disagree (finding 37)."

In `src/search/graph.cpp`, `add_deact_edge`: after step 1 Task 12 the claim reads as below (Task 12's plan, Step 5). Replace its two `plusmeasure(end, sp_bars_to_measures(1))` calls with `extend_deacts`' answer:

```cpp
    // The one phrase chord this SP end can squeeze out (core/sqout_chord.h).
    // Where collecting it moves this end is extend_deacts' answer, the same
    // one the phrase's own advance edge carries, cap included (finding 37).
    // On or before the end, both branches' ends move. After the end, only a
    // late SqIn reaches it, so only the SqIn end moves; a phrase after the
    // end can never reach the ceiling, so that side is never clamped.
    if (const SongTimestamp* c = core::sqout_chord(song_, end)) {
        deact_edge->sqinout_time = c->timecode;
        deact_edge->sqinout_timing = offset_from_sp_end(c->timecode.ms(), end.ms());
        const DeactExtension moved = extend_deacts({end}, c->timecode).front();
        deact_edge->sqin_time = moved.to;
        if (c->timecode.ticks() <= end.ticks()) {
            deact_edge->sqout_time = moved.to;
            deact_edge->sqout_clamped = moved.clamped;
        } else {
            deact_edge->late_sqin_count = 1;
        }
    }
```

If Step 0 found the claim still split between `add_deact_edge` and `store_new_backend` (Task 12 not merged), make the same replacement in both places: in `add_deact_edge` (1abd188 lines 363-364) and in `store_new_backend`'s late-SqIn bump (line 246), each with `extend_deacts({dest}, phrase).front()`.

- [ ] **Step 4: The engine remembers which end a clamp moved.** In `src/search/engine.cpp`, give `EdgeView` the flag (after line 57) and fill it where the view is built (after line 151):

```cpp
    bool sqout_clamped;
```
```cpp
        v.sqout_clamped = o->sqout_clamped;
```

Give `Path` the moved-from end, after `clamp_tick` (line 210):

```cpp
    // When clamp_tick is set: the SP end the clamping phrase moved, before it
    // pinned the window to the ceiling. Only a path that moved from a node's
    // tick may squeeze that phrase out there (finding 37). NO_TIME otherwise.
    int64_t clamp_from;
```

Set it in the advance loop (lines 497-505), keeping the old end before it is overwritten:

```cpp
                const auto& emap = eo->sp_times[(size_t)i].second;
                auto mit = emap.find(sp_end_time);
                if (mit == emap.end()) {
                    p.node = NODE_BROKEN;
                    return;
                }
                if (mit->second.clamped) {
                    p.clamp_tick = eo->sp_times[(size_t)i].first.ticks();
                    p.clamp_from = sp_end_time;
                }
                sp_end_time = mit->second.to_tick;
```

Add `c.clamp_from = NO_TIME;` beside `c.clamp_tick = NO_TIME;` at lines 576 and 604, and `root.clamp_from = NO_TIME;` beside line 1042.

- [ ] **Step 5: The squeeze-out goes only to the path that moved from here.** Change `deactivation_type` to take the path (declaration at line 377, definition at lines 587-592, caller at line 655):

```cpp
    int32_t deactivation_type(const EdgeView& e, const Path& p) const;
```
```cpp
int32_t Engine::deactivation_type(const EdgeView& e, const Path& p) const {
    if (e.sqinout_time != NO_TIME) {
        if (p.sp_end_time != e.sqout_time) return DEACT_NONE;
        // A clamped end is the ceiling whichever end the phrase moved. Only
        // the path whose end before that phrase was this node can squeeze it
        // out here; any other path would stop at an end it never had
        // (finding 37).
        if (e.sqout_clamped &&
            (p.clamp_tick != e.sqinout_time || p.clamp_from != node(e.dest).tick))
            return DEACT_NONE;
        return DEACT_SQINOUT;
    }
    return p.sp_end_time == node(e.dest).tick ? DEACT_NORMAL : DEACT_NONE;
}
```
```cpp
    const int32_t deact_type = deactivation_type(e, p);
```

The guard returns `DEACT_NONE` when it refuses, which is exactly what such a path got before this task, so it removes nothing that exists today.

Leave the group key (lines 888-897) alone. Two paths with the same clamped end but different `clamp_from` still share a group, and the lower-scoring one can be pruned before its own squeeze-out node. That loses a squeeze-out today's code never offered either, so it is no regression; put one sentence about it in the group key's comment.

- [ ] **Step 6: Run Verify.** All four "finding 37:" cases pass, the full suite passes, and `selfcheck` is clean. Then run the cap-4 corpus check from the header at all four difficulties, `main`'s build against this one. Expected: no difference at cap 4. A clamp at cap 4 needs a phrase more than 6 measures before the end and under 500 ms before it, so measures under about 83 ms.

- [ ] **Step 7: Count, and show the user before landing.** First the corpus at the what-if caps, with `s2-scores.ps1 -Cap 2` and `-Cap 3` at all four difficulties, `main`'s build against this one.

Then the library. A window can only change when two measures fit in 500 ms at cap 2 (four at cap 3, six at cap 4), so only charts with a measure shorter than 250 ms can move. This pre-filter lists them. It reads only the tempo map and time signatures, and it uses a lower bound on the shortest measure (shortest measure in ticks at the fastest tempo), so it can list extra charts but never misses one. Containers (`.sng`, `.srb`) are listed unconditionally.

```python
# short_measures.py: list charts that could have a measure under LIMIT ms.
#   python short_measures.py "C:\Clone Hero" 250 > candidates.txt
import os, struct, sys

def vlq(b, i):
    v = 0
    while True:
        c = b[i]; i += 1
        v = (v << 7) | (c & 0x7F)
        if c < 0x80:
            return v, i

def mid_bound_ms(path):
    b = open(path, 'rb').read()
    if b[:4] != b'MThd':
        return None
    hlen = struct.unpack('>I', b[4:8])[0]
    div = struct.unpack('>H', b[12:14])[0]
    if div & 0x8000 or div == 0:
        return None
    i = 8 + hlen
    if b[i:i + 4] != b'MTrk':
        return None
    end = i + 8 + struct.unpack('>I', b[i + 4:i + 8])[0]
    i += 8
    tempos, measures, status = [500000], [4 * div], 0
    while i < end:
        _, i = vlq(b, i)
        c = b[i]
        if c == 0xFF:
            typ = b[i + 1]
            n, i = vlq(b, i + 2)
            data = b[i:i + n]; i += n
            if typ == 0x51 and n == 3 and int.from_bytes(data, 'big') > 0:
                tempos.append(int.from_bytes(data, 'big'))
            elif typ == 0x58 and n >= 2 and data[0] > 0:
                measures.append(data[0] * 4 * div // (2 ** data[1]))
            elif typ == 0x2F:
                break
        elif c in (0xF0, 0xF7):
            n, i = vlq(b, i + 1); i += n
        else:
            if c & 0x80:
                status = c; i += 1
            i += 1 if (status & 0xF0) in (0xC0, 0xD0) else 2
    # Hydra reads tempo and time signatures from the first track only.
    return min(measures) * min(tempos) / div / 1000.0

def chart_bound_ms(path):
    res, bpms, measures, section = 192, [], [], ''
    for raw in open(path, encoding='utf-8-sig', errors='replace'):
        line = raw.strip()
        if line.startswith('['):
            section = line
        elif section == '[Song]' and line.startswith('Resolution'):
            res = int(line.split('=')[1].strip())
        elif section == '[SyncTrack]' and '=' in line:
            parts = line.split('=')[1].split()
            if parts[0] == 'B' and int(parts[1]) > 0:
                bpms.append(int(parts[1]) / 1000.0)
            elif parts[0] == 'TS' and int(parts[1]) > 0:
                den = 2 ** int(parts[2]) if len(parts) > 2 else 4
                measures.append(int(parts[1]) * 4 * res // den)
    measures = measures or [4 * res]
    fastest = max(bpms) if bpms else 120.0
    return min(measures) * (60000.0 / fastest) / res

root, limit = sys.argv[1], float(sys.argv[2])
for d, _, files in os.walk(root):
    for f in files:
        p = os.path.join(d, f)
        low = f.lower()
        try:
            if low.endswith('.sng') or low.endswith('.srb'):
                print(p)
            elif low == 'notes.mid':
                ms = mid_bound_ms(p)
                if ms is None or ms < limit:
                    print(p)
            elif low == 'notes.chart':
                if chart_bound_ms(p) < limit:
                    print(p)
        except Exception:
            print(p)  # unreadable here: let Hydra decide
```

Dump the candidates with `s2-scores.ps1 -List candidates.txt` at caps 2, 3 and 4 and all four difficulties, with `main`'s build and this one, one benchmark at a time. Report, per cap and difficulty: how many charts change best score (up and down, largest and median change), how many change best path, and how many change any path string. Name the five largest changes, with names taken from each chart's `song.ini`.

Tell the user, before landing, what they will see. Best scores can only stay or rise, because the change adds squeeze-out options and removes none. Path strings on clamped windows can gain a `+` (the phrase squeezed in, as unclamped windows already show) and new `−` paths can appear. If the count shows any best score falling, stop: that contradicts the reasoning above and needs a look before anything lands.

- [ ] **Step 8: Commit**, staged by name: `src/search/graph.h`, `src/search/graph.cpp`, `src/search/engine.cpp`, `tests/test_s2_deact_extension.cpp`, `CMakeLists.txt`. Message: "Squeeze-out edge reads extend_deacts' end, cap included (finding 37)". The results-stamp need is handled in Step 9, which T10 owns.

- [ ] **Step 9: T10's own stamp check, when it lands.** T10 lands on its own yes, after its count. That can be after `claude/s2-int` merged, or even after a release, so T9 Step 6's one check at the s2-int merge can't cover it. T10 owns this check itself.

Just before T10 merges to `main`, re-run T9 Step 6's tag check on `main`, the same commands as written there. It finds the newest commit that changed the `kResultsStamp` value line and runs `git tag --contains` on it.

If no tag is printed, no release carries the current results stamp yet. T10 then ships in that same release and shares its bump. No stamp edit is needed.

If a tag is printed, a release already carries step 1 Task 22's stamp (or T9 Step 6's second bump). Then T10 needs a new `kResultsStamp`, because its results differ from the shipped ones under the same keys. Stop and ask the user before landing: "A release already shipped this results stamp. T10 changes saved results, so it needs its own bump, and every saved result reads Stale once more. Bump it?" On a yes, set `kResultsStamp` to the version in `CMakeLists.txt` line 20 with `accepted` holding only it, update `tests/test_store.cpp`'s stamp pin to match, and commit `src/store/stored_versions.h` and `tests/test_store.cpp` by name with T10. On a no, T10 waits.

**Done when:**
- [ ] The four "finding 37:" cases pass, and the first three failed before Steps 3-5.
- [ ] Step 9's tag check ran just before landing, and either printed nothing or the user said yes to T10's own stamp bump.
- [ ] `hydra_tests` 0 failed; `selfcheck` clean.
- [ ] The cap-4 corpus check shows no difference at any difficulty.
- [ ] The Step 7 count is written up in plain English, no best score falls, and the user said yes to landing it.

---

### Task T11: Pin the +3.0 ms leeway edge (finding 304)

**Branches from:** `main`, after step 1 has merged (step 1 Task 15 edits `src/core/backend_value.h`).

**Goal:** The strict edge of the backend leeway is a recorded decision, and a test that names that decision fails if anyone makes it inclusive.

**Decision:** D29: "304: a backend note at exactly +3.0 ms stays uncounted (strict); record it and pin it with a test."

**How it works today.** One function answers "does this note still score under SP without a squeeze?":

```cpp
// src/core/backend_value.h at 1abd188, lines 32-36
// Counted under Star Power with no squeeze: at or before the SP end, or
// less than the leeway (Rules::backend_leeway_ms) after it.
inline bool counted_without_squeeze(double offset_ms, double leeway_ms) {
    return paid_by_sp_walk(offset_ms) || offset_ms < leeway_ms;
}
```

The `<` makes the edge strict: 2.999 ms after the SP end counts, 3.0 ms does not. The value side is already pinned. The case "backend_row_value: every engine case" in `tests/test_model.cpp` checks a row worth 460 at +2.999 ms and 0 at +3.0 ms (lines 333-334). What is missing is a test that names the decision, checks the rule itself, and checks the details table's label, which asks the same rule (`BackendSqueeze::summarystr`, `src/core/model.cpp` line 324). The `Rules::backend_leeway_ms` comment (`src/core/rules.h` line 27, "A backend note this close after the SP end still scores under SP") doesn't say which way the edge falls.

**Files.** The new test case goes in `tests/test_model.cpp`, after "backend_row_value: every engine case" (which ends at line 367 at 1abd188). Two comments change to say the edge is strict: the one on `backend_leeway_ms` in `src/core/rules.h` (line 27), and the one on `counted_without_squeeze` in `src/core/backend_value.h` (lines 32-33).

There is no new test file. The ruling for this task puts the pin beside the existing backend-value tests, which is where a reader looking for this rule will look.

**Acceptance Criteria:**
- [ ] A test named for D29 checks +2.999 ms counted and +3.0 ms not, through `counted_without_squeeze` and through the row label.
- [ ] Flipping `<` to `<=` makes that test fail (checked once, then reverted).
- [ ] Both comments say the edge is strict and cite D29.
- [ ] No score changes: the cap-4 corpus score file is byte-identical to `main`'s.

**Verify:**
```powershell
cd C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\s2-t11
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe --test-case="backend leeway*,backend_row_value*"
.\build-cpp\Release\hydra_tests.exe
```
Expected: all passed, then 0 failed.

**Steps:**

- [ ] **Step 0: Re-read after step 1.** On the new `main`, read `src/core/backend_value.h` and `BackendSqueeze::summarystr` in `src/core/model.cpp`. Step 1 Task 13 removed each row's `is_sp` flag, so `summarystr` may now take the activation, or a flag for "this row is the squeezed-out chord". Adjust the label lines below to call it the way a non-squeezed-out row is labelled now. Keep both offsets and both expected labels.

- [ ] **Step 1: Write the test.** In `tests/test_model.cpp`, after the "backend_row_value: every engine case" case:

```cpp
// D29 (finding 304): the backend leeway's edge is strict. A note less than
// the leeway after the SP end still scores under Star Power; a note exactly
// the leeway after it does not. No such constant was found in the engine
// methods read (no 0.003 in the 126 Clone Hero engine methods read); the
// 3 ms is Hydra's own setting
// (Rules::backend_leeway_ms), so the edge is Hydra's call, and this is it.
TEST_CASE("backend leeway: +2.999 ms is counted, exactly +3.0 ms is not (D29)") {
    const double lw = core::default_rules().backend_leeway_ms;
    REQUIRE(lw == 3.0);
    CHECK(core::counted_without_squeeze(2.999, lw));
    CHECK_FALSE(core::counted_without_squeeze(3.0, lw));

    // The details table labels a row by the same rule.
    BackendSqueeze row;
    row.is_sp = false;
    row.offset_ms = 2.999;
    CHECK(row.summarystr(85.0, lw) == "Standard");
    row.offset_ms = 3.0;
    CHECK(row.summarystr(85.0, lw) == "Hard (uncounted)");
}
```

- [ ] **Step 2: Run it.** It passes at once, because this pins today's rule. Prove it can fail: change `offset_ms < leeway_ms` to `offset_ms <= leeway_ms` in `counted_without_squeeze`, rebuild, run `--test-case="backend leeway*"`, and see both the `CHECK_FALSE` and the "Hard (uncounted)" check fail (as does line 334 of the existing case). Revert the change. Do not commit the flipped line.

- [ ] **Step 3: Say it in the comments.** In `src/core/rules.h`, replace line 27 with:

```cpp
    // A backend note less than this many ms after the SP end still scores
    // under SP. Exactly this far after it does not: the edge is strict
    // (decision D29, 2026-10-03).
```

In `src/core/backend_value.h`, replace lines 32-33 with:

```cpp
// Counted under Star Power with no squeeze: at or before the SP end, or
// less than the leeway (Rules::backend_leeway_ms) after it. Exactly the
// leeway after it is not counted; the edge is strict (decision D29).
```

- [ ] **Step 4: Run Verify,** then the cap-4 corpus check from the header (Expert is enough here; comments and a test change no result). Expected: identical.

- [ ] **Step 5: Commit**, staged by name: `tests/test_model.cpp`, `src/core/rules.h`, `src/core/backend_value.h`. Message: "Pin the strict +3.0 ms backend leeway edge (D29, finding 304)". No stamp.

**Done when:**
- [ ] The new case passes, and it failed with the edge flipped to `<=`.
- [ ] `hydra_tests` 0 failed.
- [ ] The corpus score file is identical to `main`'s.
- [ ] CONTEXT.md's "Backend leeway" entry (written by T9) and both code comments say the same thing.

---

## Appendix A: where this plan departs from or adds to the integration rulings

The rulings in `s2-plan/rulings.md` bind every task. Where the code forced a choice they left open, or where a ruling looked wrong, the plan follows the ruling as closely as the code allows and says so here. Nothing below changes a user decision.

**T0's table sits where the ruling put it, with one namespace detail.** "Next to `difficulty_base_pitch` (line 401)" is inside the anonymous namespace that opens at line 337, and the tests need a public lookup. T0 closes and reopens the namespace around the public accessor. Beside `difficulty_name` (line 208), which is already public, would be tidier.

**T2 leaves `uitest_details.cpp` line 253 alone.** The ruling lists it among the pins to update, but that check runs at Expert, where D20 changes nothing. T2 adds a new `dynamics-hard` GUI test instead.

**T2's "2x kicks: X of Y" line and its wording were open (Q1, Q2), and D31 approved them.** The ruling's "one kick total" doesn't name that line, and D20 gives no tooltip or UserGuide text. D31 approves the plan's wording, the tooltip at every difficulty, and the line counting every charted kick.

**T5 stores the late tag's time in milliseconds, not its tick (Q3, approved in D31).** The Dynamics tab draws from the stored blob and has no tempo map. The tick stays on the Song. It is still two stored facts per difficulty. T5's singular "1 earlier marking" is Q4, also approved in D31.

**T6 picks the `ChartFileError` fix for finding 331.** The ruling offered two fixes. The parser already drops a duplicate note through the same per-op catch, so the skip rule keeps one owner, and the change stays out of `song.cpp`.

**T6 exports `apply_timesig`.** "Fixtures use `apply_timesig`" needs it out of the anonymous namespace and declared in `song.h`, and Song's constructor moves into `song.cpp` to call it. "build_time_box reads the scene's default" is met through `PreviewTimeSig`'s member defaults, which read new `kDefaultTimeSig*` constants: a default-built scene has no signatures at all. T6's comment on those constants does not claim Clone Hero agrees, because nobody checked.

**T6's fixture count is six, not five.** Finding 258 says five fixtures repeat `tpm_changes[0] = 768`; there are three in `test_replay.cpp` and one each in `test_preview_view.cpp`, `test_search.cpp` and `test_rules.cpp`.

**T7 needs more than the ruling names.** `tally_dm_rows`' last branch would silently count off-speed rows as "not in library", so the tally gains an `other_speed` count, and one `counts_phrase` helper feeds the page subtitle and the finished window so they can't drift. Its new clause is Q5, and its other visible texts (the filter option, stat row, help sentence, dimmed cell and UserGuide sentence) were the review's wording gaps; D31 approves all of them. The page's JavaScript counts mirror `tally_dm_rows`, and a T7 test checks they count the same statuses.

**T9 Step 6 may bump the results stamp a second time, which departs from D23.** D23 says step 2 "does not bump twice", assuming step 1 and step 2 ship in one release. If a release tag already holds step 1 Task 22's stamp at the merge to `main`, a second bump is the only way to stop old results reading Ready. That is a second user-visible re-analysis, so it is a blocking question to the user at the merge, not a note afterwards. T10 Step 9 asks the same question for T10 if a release already carries the current stamp when T10 lands.

**T9 names T5's blob fields directly.** T5 Step 6 fixes them as `late_tag_ms` and `marks_before_tag`, so T9 doesn't have to wait for T5's diff to write its stamp comment.

**T9 records Fill B (D30).** The ruling's T9 list predates D30. T9's ADR, CONTEXT entry and differences page say each authored fill is placed on its own, and the ADR lists Clone Hero's unbounded one-side landing as a deliberate difference.

**Two records the briefs suggested are left out.** The small brief says "no time signature means 4/4" and "`.chart` `TS n` means n/4" should be recorded, but no Clone Hero evidence for either was gathered, so the ADR would have nothing to cite; T6's code comment states the default instead. "An off-speed leaderboard score is shown but not compared" (D25) is written in the UserGuide by T7, which is where a reader of that page looks.

**T10 needs a guard the ruling doesn't mention.** Making `add_deact_edge` read `extend_deacts`' answer is not enough on its own: two pending ends can clamp to the same ceiling, and the engine would offer one path a squeeze-out that stops at the other's end. T10 adds a `sqout_clamped` flag, the engine's `clamp_from`, and a test for the two-ends case. Path strings on clamped windows can gain a `+` even where scores don't change; the user sees that with the count.

**T10 duplicates a ten-line song builder.** `build_tail_song` lives in `test_search.cpp`'s anonymous namespace, so the rule "own test file" means a copy.

**T11 follows its own ruling over the general one.** Its test goes beside the existing backend-value tests in `tests/test_model.cpp`, as its ruling says, not in a new `test_s2_*` file. Most of the pin already exists there (lines 333-334); T11 adds a test named for D29, a direct check of `counted_without_squeeze`, a row-label check and two comments.

**Score tools have one owner.** The drafts used four different score scripts. The plan keeps one, `s2-scores.ps1`, plus `s2-compare.ps1`, defined in "Before the wave", and one baseline recorded there. Every task's corpus check and every comparison with a brief's expected dumps goes through them.

## Appendix B: facts the drafters could not verify

These are inherited from the briefs or worked out by reading, not by running. Each task's "done when" turns the important ones into a stop-and-report check rather than a fix-forward.

**Nothing was compiled or run by the drafters.** Every expected first failure is worked out by hand from the code at 1abd188.

**T2's Dynamics numbers.** Pathfinder at Hard, Totals 3,426 (T2's GUI test), comes from the kick2x brief's `dyncount` run, which its verifier reproduced. A raw scan confirms only 972 Hard kicks, 1,613 Expert 95s and 139 shared ticks. `dyncount` exists only on `claude/s2-proto-kick2x` (commit c08d813), not at 1abd188, so T2's own tests carry those numbers. Car Bomb at Hard, 2,038 notes, comes from finding 10's table.

**T1's lower-difficulty counts.** Band Like That at Medium and Easy is expected to show equal red and yellow counts with Pro Drums on and off. That is inferred from a marker scan; only Hard has measured counts.

**T5's late-tag counts.** The counts (142, 74, 8) come from the brief's Python scan, not Hydra's parser. The "4:01" in T5's tests is a fixture value.

**Clone Hero readings not re-checked.** The readings at 0x2155050 (no trim before the dynamics tag compare) and 0x20D2440 (the phrase rule) were not re-disassembled. Finding 331's colour-mismatch case was not traced in Clone Hero.

**Toolchain.** MSVC's `std::from_chars` for `double` needs VS 2019 16.4 or later.

**Inherited from the audit.** The bundled `.srb` files' notes names (`notes.*`) come from the audit. How many of the user's leaderboard scores are off speed is unknown.

**Public Hydra.** Public Hydra's disco and 2x-kick rules were checked only in the local 1.2.0 source. So the differences page names 1.2.0 and says newer versions were not checked.

**Unmeasured corpus totals.** The corpus totals with 2x Bass off for T1's charts were never measured, so the integrator compares only chart names there.

**T10 and T11 after step 1.** T10's library count and pre-filter are written but not run. Step 1's code after it merges was not read, so T10 and T11 each start with a Step 0 that re-reads it.
