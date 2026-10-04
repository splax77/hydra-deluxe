# Step 1: The Engine Stores What the Displays Guess — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers-extended-cc:subagent-driven-development (recommended) or superpowers-extended-cc:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every fact the displays need about a Star Power window is stored once by the engine and read everywhere else, so no screen, tool or test rebuilds it.

**Architecture:** Each activation stores one history of how its SP end moved: the activation, then every collected phrase, cap clamp and SqIn, each with the end it set. The old stored SP end, clamp note and collected phrases become read-only views of that history. The transfer scales, the squeeze-out, the tied variants and the Preview gauge all read stored facts through one accessor each. Scores, path strings and the list of paths do not change.

**Tech Stack:** C++20, doctest (`hydra_tests`), Dear ImGui test engine (`hydra_uitest`), `hydra_replay` (built with `-Target hydra_replay`, it is EXCLUDE_FROM_ALL), CMake through `build_cpp.ps1`, PowerShell scripts for the corpus comparison.

**Spec:** `docs/audit/2026-10-03-fix-decisions.md` (decisions D1-D5), the findings in `docs/audit/2026-10-03-derivation-audit.md`, and the triage rows in `docs/audit/2026-10-03-fix-triage.md`. The existing plan `docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md` runs first, as written.

## Global Constraints

No score, path string or list of paths changes for any chart. The one possible exception is question Q2 below, which waits on the user. Every engine task's Verify compares against the baseline captured in Task 4, and a single difference fails the task.

Each fact is stored once. Where a new list holds the items a stored count counts, the list is stored and the count becomes an accessor. Display code, tools and tests read stored facts through the accessor that owns them and never rebuild them. Test fixtures state literal ticks rather than recomputing engine values.

Tasks add fields to the record without bumping a version stamp. Task 22 bumps `kPathFormatStamp` to 7 and `kResultsStamp` to the release version once, for the whole plan. Until Task 22 lands, a development build must never open a real `hydra.db`. Use scratch databases only, passed with `--db`.

Commits are staged by file name, never `git add -A`, because other sessions leave edits in this checkout. Every commit carries `Task:`, `Agent:` and `Session:` trailers and the Co-Authored-By line. Plain English in every comment, commit message and doc, per CLAUDE.md.

**User decisions (already made, 2026-10-03):**
- D1 (finding 28): "When the SP cap clamps a window, the transfer scale is measured from the collecting note, not the activation." Finding 27 is fixed in the same change, under one stamp bump.
- D2 (finding 32): "The replay keeps its own chord-by-chord walk as a cross-check", asks the shared rule whether SP paid a chord, and the xN disc doubles only when SP actually paid something for it.
- D3 (findings 90, 89): a tied variant folded while SP runs takes the leader's closed activation from the fold point and keeps its own activation note. The search does not change. Each variant stores its own leftover SP.
- D4 (finding 332): the record may hold "unknown" for a transfer scale, but "there should never be a situation when the scale cant be computed". Every way of reaching it is closed at its source, and a test proves no fresh record stores it.
- D5 (R7.10, finding 30): the 500 ms backend window is measured from the SP end for every activation, through one shared check.
- The existing plan's seven decisions (its header) stand.
- Q1 (2026-10-03): timing that makes a measure or beat last zero, negative or infinite time refuses the chart with a plain error naming the tick. A time signature with a top number of 0 is ignored in both formats.
- Q2 (2026-10-03): a tied variant that takes its leader's closed activation also takes the closing SqIn or SqOut. Its path string may gain the + or −. This is the plan's only allowed path-string change.
- Q3 (2026-10-03): finding 97 is out of this plan. It gets its own plan after a corpus count.
- Q4 (2026-10-03): on a late squeeze-in the SP gauge refills at the old SP end.
- Q5-Q10: the recommended answers below apply unless the user overrides them before execution.

---

## What is wrong today, in plain words

Think of a Star Power window as a parking meter. The engine knows every coin that went in: the activation, each phrase collected while SP ran, each time the cap clamped the meter, each squeeze-in. But the record only keeps the time the meter finally ran out. So every screen that needs the story in between has to guess it, and the guesses go wrong in the corners.

The transfer scale rebuilds "the SP end before the SqIn" by stepping back two measures (finding 27). It measures from the activation even when the cap pinned the end to a later note (finding 28). It quietly stores x1.00 when it can't work a scale out (finding 332). The Preview gauge re-simulates the meter and shows a full bar after a late SqIn where the engine has 0.875 (finding 5). It also counts squeezed-out phrases on its own (147). The replay decides for itself which chords SP paid (32). The details table offers a SqOut on phrase notes the engine never squeezes out (29). The squeeze-out is stored twice (127, 128). A tied path folded into its leader while SP runs keeps a stale snapshot of its window (90), and every tied path shows its leader's leftover SP (89). The 500 ms window is written four times with two anchors (30, R7.10). The Preview guesses which fills the path skipped (52).

The fix is one rule applied everywhere. The engine writes down what it already knows, once, and everything else reads it. Each activation stores one history of how its SP end moved. The old stored SP end, clamp note and collected phrases become read-only views of that history, so they can't drift from it. Three stored counts (bars banked, bars left over, fills skipped) become the lengths of the lists that hold the actual ticks. The squeeze-out, the window check and "did SP pay this chord" each get one owner.

## What you'll see change

On a cap-clamped activation where the tempo or meter differs between the activation and the collecting note, the scale line and eff. figures now come from the collecting note (D1). On an activation with a SqIn, the "at the SqIn's SP end" clause is measured from the end the search actually used. Both change only where a tempo or meter change sits between the two notes. Flat-tempo charts look the same.

The Preview score box shows the plain multiplier on a squeezed-out chord that SP paid nothing for (D2). The running total doesn't change.

A tied path folded while SP was running shows its real last activation: SP end, overfill note, collected phrases, backend rows. The Preview shows a score for it instead of "Score unavailable" (D3). Every tied path shows its own leftover SP. On Don Broco - Actors at depth 40, path 30 says 3 bars instead of 1.

On a late squeeze-in the SP gauge refills at the old SP end and reads 0.875 of a bar at the phrase ("0.9/4" instead of "1.0/4"), if you say yes to Q4.

A chart whose timing makes a measure or beat last zero or negative time shows a plain error instead of analysing with time running backwards or crashing, if you say yes to Q1. No chart in `testdata` has such a line.

Every saved result needs one re-analysis after this ships, because the record format changes.

Scores, path strings and the list of paths do not change, except where you allow it in Q2.

## Questions to answer before execution

These are the calls D1-D5 don't cover. Each has a recommendation. The first four are weighty.

**Q1. Timing lines that make a measure or beat last zero, negative or infinite time: refuse the chart, or ignore the line?** This settles round 7's negative-tempo question (R7.6) and finding 100. D4 needs one or the other, because such timing makes the transfer scale impossible to compute. *Recommended: refuse the chart with a plain error naming the tick.* The one exception is a time signature with a top number of 0, which is ignored in both formats, as `.chart` already does. If you choose "ignore", those charts load with the bad line dropped and score as a chart the file doesn't describe. Clone Hero's behaviour for these lines is unknown. Either way, every chart in `testdata` loads and scores as today.

**Q2. When a tied variant takes its leader's closed activation (D3), does it also take the SqIn or SqOut the leader made when it closed the window?** *Recommended: yes.* The squeeze is part of the same closing. A search that prices that path alone gives '0 1+' and '0 1-' on the test chart. If yes, those variants get the + or − in their path string, copy string and report row. This is the plan's only allowed change to a path string. None is expected on the corpus. If no, no string changes, but those variants show an SP end reached through a squeeze they don't list. Scores and the path list don't change either way.

**Q3. Finding 97: a tied variant shows its leader's skip count and E mark. How should it be fixed?** This is a different mechanism from D3. It happens at folds between windows, where the two paths have different fill histories. *Recommended: take it out of this plan and give it its own small plan after a corpus count.* The drafter's preferred fix (record the follower's skip state at the fold) can mean dropping a rare variant, which is a path-list change. The other options are adding skip state to the fold key (slower search, path lists change) or documenting it. Leaving it out changes nothing else in this plan.

**Q4. On a late squeeze-in, where does the SP gauge refill?** *Recommended: at the old SP end.* The player hits that phrase early, so SP never stops, and the gauge then reads 0.875 at the phrase, matching the engine. The alternative leaves the bar empty until the phrase. Nothing is stored either way.

**Q5. Wording when one activation has two or more SqIns with different scales.** *Recommended: one numbered clause per SqIn* ("x0.97 (early) at SqIn 1's SP end; x0.95 (early) at SqIn 2's SP end"). With one SqIn it reads as today. No activation in `testdata` has two.

**Q6. Colour of the "Transfer scale unknown." note.** *Recommended: orange,* because D4 calls it a guard that shows a bug. It should never appear.

**Q7. A phrase-chord row the path did not squeeze out shows a "... SqOut" rating. Should it get the plain rating instead?** *Recommended: yes,* since no path can squeeze it out. It only shows in a rare corner (SP outlasting the song at very high tempo). If no, Task 14 is dropped.

**Q8. When SP ends, should the Preview score box drop to the plain multiplier right away, instead of holding the doubled value until the next chord?** *Recommended: yes, after checking one FC video* that the game's disc drops too. It would then agree with the drain box and highway. If no, Task 16 is dropped.

**Q9. Under the 1.0 fill rule, should the offered fill the Preview lights follow the engine's stored list?** *Recommended: yes.* Under the default 1.1 rule nothing changes. If no, only Task 21 is dropped.

**Q10. Should the gauge keep filling to the cap on charts with no analysis?** *Recommended: yes,* as CONTEXT.md already documents.

## The stored record after this plan

Each activation gains one history list, `sp_end_steps`. Each step is a tick, the SP end it set, and its kind: Activation, Collected, Clamped or SqIn. The old fields `deact_tick`, `clamp_tick` and `collected_phrase_ticks` stop being stored. They become read-only views of the history with the same names, so every reader keeps compiling. New views answer the plain SP end (`nominal_end`), the end a squeeze was measured from (`squeeze_end_tick`), the note that moves an end (`end_anchor_tick`) and where the gauge refills (`refill_tick`).

Three stored counts become list lengths. `bank_rise_ticks` (the ticks where the bank gained a bar) replaces `sp_meter`. `trailing_bank_ticks` (bars banked after the last window) replaces `leftover_sp`, and each tied variant stores its own. `skipped_fill_ticks` replaces `skips`.

The transfer scales lose `transfer_pre`. Each SqIn stores its own scale instead, so several SqIns are exact, and `transfer_post` can hold "unknown". The squeeze-out is stored once, as `sqout_tick` plus its row's offset. The duplicate SqOut entry, the squeeze kind byte and each row's `is_sp` flag go.

Task 22 bumps `kPathFormatStamp` to 7 and `kResultsStamp` to the release version, once.

## Task order

The labels in brackets are the draft labels the task text still uses when it points at another task. Appendix A gives the rulings (R1-R5) those texts cite.

| # | Task | Draft label | Findings | Stored results change? |
|---|---|---|---|---|
| 1 | Run the one-squeeze-rating plan | existing plan | 24, 25, 26, 137, 144, 303 | no |
| 2 | One "decided by the frontend" question | A1 | 259 | no |
| 3 | Timing that can't measure time stops at load | A5 | R7.6, 100, D4 | no (charts that load today) |
| 4 | Baseline at HEAD | R3 step 2, B piece, C1 | all | no |
| 5 | One SP-end history per activation | D1 (with A2, A6) | 27, 156, 5 | layout only |
| 6 | The bank list replaces two counts | D2 | 5, 147, 89 | layout only |
| 7 | The fill list replaces the skip count | D3 | 52 | layout only |
| 8 | Transfer scales read the history | A3 | 27, 28 | yes |
| 9 | "Unknown" is a value the record can hold | A4 | 332 | layout only |
| 10 | Prove no fresh record stores "unknown" | A7 | 332 | no |
| 11 | One window check and one offset rule | C2 | 30, R7.10, 149 | no |
| 12 | One rule for which chord can be squeezed out | C3 | 145 | no |
| 13 | The squeeze-out is stored once | C4 | 127, 128 | layout only |
| 14 | Row labels read the stored squeeze-out (Q7) | C5 | 29 | layout only |
| 15 | The replay asks the shared "paid by SP" rule | C6 | 32 | no |
| 16 | Score box drops the doubled disc at SP end (Q8) | C7 | 1 | no |
| 17 | A variant takes its leader's steps after the fold | B1 | 90 | yes |
| 18 | Each tied variant keeps its own bank | B2 | 89 | yes |
| 19 | The tied-variant guard | B3 | 89, 90 | no |
| 20 | The SP gauge draws only stored facts | D4 | 5, 147, 159 | no |
| 21 | Offered fills read the stored list (Q9) | D5 | 52 | no |
| 22 | Stamps, docs and the final proof | R3 step 9, A8, C8 | all | the bump |

Tasks 1 to 3 change nothing in the engine. Task 4 must run before any engine change. From Task 5 on, every task compares against Task 4's baseline, and any difference it doesn't name fails it.

### Parallel schedule (added 2026-10-03 during execution)

The tasks run in parallel worktrees, not in the table's order. A planner checked every task's files and hand-offs against the code at 02976c1. Its waves:

| Wave | Runs together (→ means one after the other in one worktree) |
|---|---|
| 0 | Task 1 → Task 2; Task 3; Task 4 |
| 1 | Task 5 (merges alone, first); Task 11 (on top of Task 3); Task 15; Task 16 (after the FC video check) |
| 2 | Task 6 → Task 7; Task 8; Task 12; Task 17 |
| 3 | Task 18 → Task 19; Task 20 → Task 21; Task 9 → Task 10 → Task 13; Task 14 |
| 4 | Task 22 |

Each wave branches from an integration branch that holds every earlier wave. Merge order inside a wave: Task 5 first in wave 1; Task 6→7 first in wave 2; Task 18→19 before Task 20→21 in wave 3, since Task 20's corpus check needs Task 18's per-variant banks.

The planner found five problems in the task text. Each lane's brief carries the fix:

1. Task 5's `record_fixtures.h` routes every fixture write of `sp_meter`, `skips`, `leftover_sp`, `deact_tick`, `clamp_tick` and the transfer scales through helpers that write the old fields at first. Tasks 6 to 9 then change only a helper's body. Task 5 does not add `set_bank` (its field arrives in Task 6).
2. Tasks 8 and 10 call the shared songs with the `test::` prefix Task 5 introduces.
3. Every lane runs `scores.ps1 -Repo <its worktree>` and writes its outputs and scratch database to its own scratch folder, never into `.superpowers/sdd/...`.
4. Task 17's Step 5 (widening Task 6's bank-order check to variants) moves to Task 18. Before Task 18, a variant folded between windows still carries its leader's bank ticks.
5. Task 21's fill comparison runs on root paths only. Tied variants still carry their leader's skip list until finding 97 gets its own plan (Q3).

Task 6's benchmark compares against `bench_head_02976c1.exe` in the Task 4 folder. It runs once, at the end of the Task 6→7 lane, while nothing else builds.

## Not in this plan

Finding 37 (the squeeze-out edge ignores the SP cap) moves to step 2. Clone Hero's own code clamps Star Power at the cap during an activation, so fixing Hydra to match can change scores, and this plan changes none. The evidence is in `docs/audit/ch-evidence.md`. Finding 97 waits on Q3. The other step-2 parser drifts, including disco flip at lower difficulties, get their own plan.

## Prior art

Storing a fact where it is produced, instead of rebuilding it downstream, is the "single source of truth" principle; ADR 0011 is this repo's version of it. The baseline comparisons follow characterization testing: capture what the code does today, then require every change to reproduce it except where it names a difference ([Characterization test](https://en.wikipedia.org/wiki/Characterization_test)). The variant oracle, a search that prices one path alone, is differential testing: two independent computations of the same answer, compared ([Differential testing](https://en.wikipedia.org/wiki/Differential_testing)).

---

## Tasks

### Task 1: Run the one-squeeze-rating plan

*Draft label: existing plan.*

**Goal:** The one-squeeze-rating plan's three tasks land as written, so every note near an SP end is rated by one rule before the engine changes.

**Findings:** 24, 25, 26, 137, 144 (rating half), 303.

**Decision:** that plan's seven header decisions.

That plan is already designed and its decisions are made. A check against HEAD 02976c1 found every line reference and code excerpt still matching. None of its files changed after 2026-09-29. Two small range slips are harmless. Its Task 1 names the row tooltip as `path_view.cpp` lines 295-305, and its Task 2 as 296-306. The block is 295-307, and the `snprintf` inside it is 300-305. Either range finds it. Task 8 later replaces `Activation::transfer_pre` with a scale per SqIn. Task 8 edits that plan's new test cases that set `act.transfer_pre`, so run that plan unchanged.

**Files:** as listed in `docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md`, Tasks 1-3.

**Acceptance Criteria:**
- [ ] That plan's Tasks 1 and 2 are committed, each with its own acceptance criteria met.
- [ ] Its Task 3 probe output on Sinner's Vengeance is quoted in the report to the user.
- [ ] Full `hydra_tests` and `hydra_uitest --all --jobs 4` pass.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → `Status: SUCCESS!` and every UI test `[PASS]`.

**Steps:**
- [ ] **Step 1:** Execute Tasks 1, 2 and 3 of `docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md` exactly as written, with their own commits.
- [ ] **Step 2:** Run Verify.

### Task 2: One "decided by the frontend" question

*Draft label: A1.*

**Goal:** The cap-clamped flag asks one named predicate, and the header comment says what is true.

**Findings:** 259.

**Decision:** none needed. Nothing on screen changes.

After the existing plan, the cap-clamped loop is the only place left that asks whether the frontend decides a row. The rule still has no name. The comment above `cap_clamped` says the cap rows are "the rows rate_activation judges". That stopped being true when 4f5fcc1 added plain rows inside SP. This task names the rule and fixes the comment.

**Files:**
- Modify: `src/core/squeeze_rating.h:120-125` (the `cap_clamped` comment) and the rating-pieces section (new declaration).
- Modify: `src/core/squeeze_rating.cpp:174-192` (the cap loop).
- Test: `tests/test_squeeze_rating.cpp`, new case after `"rate_activation: cap_clamped flag"` (line 720 at HEAD).

**Acceptance Criteria:**
- [ ] `is_frontend_decided` is true for a squeezed-out row and for an uncounted row. It is false for a counted row and for a row with no offset.
- [ ] `grep -n "counted_without_squeeze" src/core/squeeze_rating.cpp` shows it only inside `is_frontend_decided` and the row loop's `inside` test.
- [ ] Existing `cap_clamped` cases pass unchanged.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*cap_clamped*,is_frontend_decided*"` → `Status: SUCCESS!`

**Steps:**

- [ ] **Step 1: Failing test.**

```cpp
TEST_CASE("is_frontend_decided: squeezed out, or not counted without a squeeze") {
    const double leeway = core::default_rules().backend_leeway_ms;
    BackendRating counted;
    counted.row.offset_ms = -40.0;
    CHECK_FALSE(is_frontend_decided(counted, leeway));

    BackendRating uncounted;
    uncounted.row.offset_ms = leeway + 5.0;
    CHECK(is_frontend_decided(uncounted, leeway));

    BackendRating squeezed;
    squeezed.row.offset_ms = -40.0;
    squeezed.squeezed_out = true;
    CHECK(is_frontend_decided(squeezed, leeway));

    BackendRating no_offset;
    CHECK_FALSE(is_frontend_decided(no_offset, leeway));
}
```

- [ ] **Step 2: Build and watch it fail** on the missing `is_frontend_decided`.

- [ ] **Step 3: Declare it** in `squeeze_rating.h`, after `BackendRating`:

```cpp
// Whether the frontend decides this row: it was squeezed out, or the engine
// does not count it without a squeeze (at or past the leeway). The overfill
// warning fires only for an activation that has such a row or a SqIn/SqOut.
bool is_frontend_decided(const BackendRating& row, double backend_leeway_ms);
```

Replace the `cap_clamped` comment (lines 120-124) with:

```cpp
    // True when the SP cap clamped this activation's window AND the frontend
    // decides at least one of its squeezes: any SqIn/SqOut, or any row
    // is_frontend_decided accepts. Drives the overfill warning.
```

- [ ] **Step 4: Define and use it** in `squeeze_rating.cpp`:

```cpp
bool is_frontend_decided(const BackendRating& row, double backend_leeway_ms) {
    return row.squeezed_out ||
           (row.row.offset_ms &&
            !core::counted_without_squeeze(*row.row.offset_ms, backend_leeway_ms));
}
```

```cpp
    if (act.clamp_tick.has_value()) {
        bool decided = !act.sqinouts.empty();
        for (const BackendRating& br : out.backends)
            decided = decided || is_frontend_decided(br, backend_leeway_ms);
        out.cap_clamped = decided;
    }
```

(After R1 lands, `act.clamp_tick.has_value()` is spelled `act.clamp_tick().has_value()`. That is R1's mechanical rename, not this task's.)

- [ ] **Step 5: Run the full `hydra_tests`.** Expected `Status: SUCCESS!`.

- [ ] **Step 6: Commit** `src/core/squeeze_rating.h src/core/squeeze_rating.cpp tests/test_squeeze_rating.cpp` by name: "Name the frontend-decided row rule once", with the `Task:`, `Agent:` and `Session:` trailers and the Co-Authored-By line.

### Task 3: Timing that can't measure time stops at load

*Draft label: A5.*

**Goal:** No chart reaches the engine with a measure or beat lasting zero, negative or infinite time.

**Findings:** 332 (its source), with R7.6 and 100 where D4 needs them.

**Decision:** D4: "a zero or negative measure length can't reach the engine (this ties to R7.6, negative `.chart` tempos)". The refuse-or-ignore choice per line is Q1.

A measure's length in ms is ticks-per-measure divided by ticks-per-second. Every way that length can be zero, negative or not a number comes from the chart's timing lines. The table shows each one, the input that reaches it today, and how this task closes it.

| What goes wrong | Input today | Effect today | Closed by |
|---|---|---|---|
| tempo 0 | `.chart` `B 0` | measure length infinite; scale NaN or 0, stored silently | refuse (Q1) |
| tempo below 0 | `.chart` `B -60000` (R7.6) | time runs backwards; front side returns nothing | refuse (Q1) |
| tempo infinite | `.mid` set_tempo of 0 µs | `60000000 / 0`; ms stops advancing | refuse (Q1) |
| numerator 0 | `.mid` TS 0/x (finding 100) | divide-by-zero crash | ignore the line, as `.chart` does (Q1) |
| numerator below 0 | `.chart` `TS -3` (passes the `!= 0` check at song.cpp 1007) | negative measures | refuse (Q1) |
| measure truncates to 0 ticks | `TS 1 12` (den 4096) at resolution 192, either format | `192*4*1/4096 = 0`; crash | refuse (Q1) |
| denominator shift overflow | `.chart` `TS 4 40`; `.mid` exponent byte ≥ 31 | `1 << n` undefined behaviour | refuse (Q1) |
| resolution 0 | `.chart` `Resolution = 0`; `.mid` division 0 | 0/0 NaN; crash in `Timecode` | refuse (Q1) |
| resolution below 0 | `.chart` `Resolution = -192` (a negative `.mid` division is already refused as SMPTE) | signs cancel into nonsense | refuse (Q1) |

One rule owns "is this timing usable", and both parsers reach it through `Song::build_timing`. The zero-numerator rule moves into `apply_timesig`, the handler both parsers already share, so `.chart`'s own `!= 0` check goes away.

**Files:**
- Create: `src/parse/timesig.h` (the denominator-exponent helper, header-only).
- Modify: `src/parse/song.h:119-123` (`build_timing` calls the check; `check_timing_maps` declared).
- Modify: `src/parse/song.cpp:119-125` (`apply_timesig`; new `check_timing_maps`), `:784` (`.chart` denominator), `:1007` (drop `.chart`'s own `!= 0`).
- Modify: `src/parse/midi.cpp:118` (`.mid` denominator).
- Test: `tests/test_song.cpp`, new cases after line 349.

**Acceptance Criteria:**
- [ ] A `.chart` with `B 0`, `B -60000`, `TS -3`, `TS 1 12` at resolution 192, `TS 4 40` or `Resolution = 0` throws `ChartFileError`.
- [ ] A `.mid` with a 0 µs tempo or a denominator exponent of 40 throws `ChartFileError`. With a TS numerator of 0 it loads and keeps the 4/4 default.
- [ ] A `.chart` with `TS 0` loads exactly as today.
- [ ] Every `testdata` chart still loads (existing corpus tests).
- [ ] `kDynamicsCountStamp` is not bumped, because no chart that loads today counts differently.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*timing line*"` → `Status: SUCCESS!`, then the full run.

**Steps:**

- [ ] **Step 1: Failing tests.** In `tests/test_song.cpp`, after line 349 (`midi_util.h` is already included there):

```cpp
namespace {
std::vector<uint8_t> chart_with(const std::string& resolution, const std::string& sync) {
    std::string s = "[Song]\n{\n  Resolution = " + resolution + "\n}\n"
                    "[SyncTrack]\n{\n" + sync + "}\n"
                    "[ExpertDrums]\n{\n  0 = N 0 0\n  768 = N 1 0\n}\n";
    return std::vector<uint8_t>(s.begin(), s.end());
}
}  // namespace

TEST_CASE(".chart: a timing line that can't measure time is refused") {
    const std::string ok = "  0 = TS 4\n  0 = B 120000\n";
    CHECK_NOTHROW(load_songbytes_chart(chart_with("192", ok), true, true));
    for (const char* bad : {"  0 = TS 4\n  0 = B 0\n",
                            "  0 = TS 4\n  0 = B 120000\n  384 = B -60000\n",
                            "  0 = TS -3\n  0 = B 120000\n",
                            "  0 = TS 1 12\n  0 = B 120000\n",
                            "  0 = TS 4 40\n  0 = B 120000\n"}) {
        CAPTURE(bad);
        CHECK_THROWS_AS(load_songbytes_chart(chart_with("192", bad), true, true),
                        ChartFileError);
    }
    CHECK_THROWS_AS(load_songbytes_chart(chart_with("0", ok), true, true), ChartFileError);
}

TEST_CASE(".chart: a TS 0 timing line is still ignored") {
    Song song = load_songbytes_chart(
        chart_with("192", "  0 = TS 4\n  0 = B 120000\n  768 = TS 0\n"), true, true);
    CHECK(song.tpm_changes.size() == 1);
    CHECK(song.tpm_changes.at(0) == 768);
}

TEST_CASE(".mid: a timing line that can't measure time") {
    auto track = [](std::vector<uint8_t> timing) {
        return testmidi::smf(testmidi::concat({testmidi::track_name("PART DRUMS"),
                                               testmidi::set_tempo(), timing,
                                               testmidi::note_on(96, 100),
                                               testmidi::end_of_track()}));
    };
    // TS 0/4: ignored, the 4/4 default stays (finding 100's crash).
    Song song = load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x00, 0x02, 0x18, 0x08}),
                                   true, true);
    CHECK(song.tpm_changes.at(0) == 480 * 4);
    // A 0 us tempo would be an infinite BPM: refused.
    CHECK_THROWS_AS(load_songbytes_mid(track(testmidi::set_tempo(0)), true, true),
                    ChartFileError);
    // A denominator exponent of 40: refused, never shifted.
    CHECK_THROWS_AS(
        load_songbytes_mid(track({0x00, 0xFF, 0x58, 0x04, 0x04, 40, 0x18, 0x08}), true, true),
        ChartFileError);
}
```

- [ ] **Step 2: Build and watch them fail.** The `.mid` TS 0 case crashes the test binary today with a divide by zero. Run it alone (`-tc=".mid: a timing line*"`) to see that, then continue.

- [ ] **Step 3: The exponent helper.** Create `src/parse/timesig.h`:

```cpp
// A time signature's denominator is stored as a power of two in both chart
// formats. One rule turns the exponent into the denominator, so neither
// parser shifts by an out-of-range amount.
#ifndef HYDRA_PARSE_TIMESIG_H
#define HYDRA_PARSE_TIMESIG_H

#include <string>

#include "core/model.h"

namespace hydra {

inline int timesig_denominator(int exponent) {
    if (exponent < 0 || exponent > 30)
        throw ChartFileError("time signature denominator 2^" + std::to_string(exponent) +
                             " is out of range");
    return 1 << exponent;
}

}  // namespace hydra

#endif
```

`.chart` (song.cpp line 784) becomes `ts_denominator = timesig_denominator(word_stoi(t.w[2]));`. `.mid` (midi.cpp line 118) becomes `out->denominator = timesig_denominator(payload[1]);`.

- [ ] **Step 4: `apply_timesig` owns the zero numerator.** Replace song.cpp lines 119-125:

```cpp
// A time signature: ticks per measure = resolution * 4 * num / den. The
// signature itself is kept too, for display. A numerator of 0 names no meter,
// so the line is ignored and the previous meter stays, in both formats
// (finding 100). Anything else that makes a measure last no time is refused
// by check_timing_maps.
void apply_timesig(Song& song, int64_t tick, int numerator, int denominator) {
    if (numerator == 0) return;
    song.tpm_changes[tick] = song.tick_resolution() * static_cast<int64_t>(numerator) * 4 /
                             static_cast<int64_t>(denominator);
    song.timesig_changes[tick] = {numerator, denominator};
}
```

Then drop `&& *e.ts_numerator != 0` from `ChartParser::optype` (line 1007).

- [ ] **Step 5: One usability check.** In `song.cpp`:

```cpp
// The timing maps Hydra can measure time with: a positive resolution, every
// measure at least one tick long, every tempo a positive, finite BPM. Both
// parsers reach this through Song::build_timing, so no chart with a zero,
// negative or infinite measure length reaches the engine (D4, R7.6).
void check_timing_maps(int64_t tick_resolution,
                       const std::map<int64_t, int64_t>& tpm_changes,
                       const std::map<int64_t, double>& bpm_changes) {
    if (tick_resolution <= 0)
        throw ChartFileError("the chart's resolution is " + std::to_string(tick_resolution) +
                             "; it must be above 0");
    for (const auto& [tick, len] : tpm_changes)
        if (len <= 0)
            throw ChartFileError("the time signature at tick " + std::to_string(tick) +
                                 " makes a measure " + std::to_string(len) + " ticks long");
    for (const auto& [tick, bpm] : bpm_changes)
        if (!std::isfinite(bpm) || bpm <= 0.0)
            throw ChartFileError("the tempo at tick " + std::to_string(tick) +
                                 " is not above 0 BPM");
}
```

Declare it in `song.h`, and call it first in `build_timing`:

```cpp
    void build_timing() {
        check_timing_maps(tick_resolution_, tpm_changes, bpm_changes);
        timing_.emplace(tick_resolution_, tpm_changes, bpm_changes);
    }
```

- [ ] **Step 6: Check the error wording.** `tests/test_user_messages.cpp` pins how errors read. Run `plain_error` on one new message and confirm it reads as a plain sentence. Add a case if it doesn't.

- [ ] **Step 7: Run the full `hydra_tests`.** Expected `Status: SUCCESS!`. A grep at HEAD found no `B 0`, negative `B`, `TS 0`, negative `TS`, large-exponent `TS` or `Resolution = 0` in any `testdata` `.chart`, so the corpus tests should not move.

- [ ] **Step 8: Commit** `src/parse/timesig.h src/parse/song.h src/parse/song.cpp src/parse/midi.cpp tests/test_song.cpp` by name: "Refuse timing that can't measure time; TS 0 ignored in both formats".

### Task 4: Baseline at HEAD

*Draft label: R3 step 2 (B piece, C1).*

**Goal:** Before any engine change, capture three records of what HEAD does on the corpus, so every later engine task can prove it moved only what it names.

**Findings:** all of this plan's engine findings (this is their proof).

**Decision:** R3 step 2 and R4 (Appendix A).

This is a characterization test. It records what the code does today, so a later run can be compared line by line. There are three pieces, because each sees something the others can't. Part D's `scores.ps1` lists every path's string and total on all 97 corpus charts. Part B's `tied_compare.ps1` adds the six score parts and each activation's facts, plus an oracle that prices one path alone. Part C's printout adds every squeeze offset and backend row, read back through the codec.

The scripts live in `C:\Users\Patrick\Downloads\Hydra\hydra-test\.superpowers\sdd\2026-10-03-step1-engine-facts\`, called `$w` below. That folder is not git-ignored in this repo, so never stage it. Baseline files go in `$w\baseline\`. If you build in a worktree, pass `-Repo <worktree path>` to `scores.ps1`.

**Files:**
- Modify: `tests/test_path_codec.cpp` (the Part C printout, piece 3; the only repo change).
- Create (not committed): `$w\baseline\scores-HEAD.txt`, `$w\baseline\hydra-baseline-squeeze-facts.txt`, and Part B's `$w\partB\base_corpus`, `base_fix2` and `base_fix4` folders.

**Acceptance Criteria:**
- [ ] `scores-HEAD.txt` is identical to the copy captured during planning, `$w\partD\baseline-02976c1.txt`.
- [ ] Each Part B baseline compared with itself prints `0 list, 0 path, 0 fact differences, 0 oracle disagreements`.
- [ ] `hydra-baseline-squeeze-facts.txt` holds one block per corpus path, and the normal `hydra_tests` run skips the printing case.
- [ ] All three were captured from a build of HEAD 02976c1, plus Tasks 1-3, which change no engine output, before Task 5 starts.

**Verify:**
```powershell
$w = 'C:\Users\Patrick\Downloads\Hydra\hydra-test\.superpowers\sdd\2026-10-03-step1-engine-facts'
pwsh -NoProfile -File "$w\partD\scores.ps1" -Out "$w\baseline\scores-HEAD.txt"
Compare-Object (Get-Content "$w\partD\baseline-02976c1.txt") (Get-Content "$w\baseline\scores-HEAD.txt")
# expect: no output
```
Then the Part B and Part C Verify blocks below.

**Steps:**
- [ ] **Step 1: Scores list (Part D piece).** Run the Verify block above. It must print nothing.
- [ ] **Step 2: Variant facts (Part B piece).** The text below is Part B's drafted piece. In it, `$s` means `$w\partB`.

*Added 2026-10-03 during execution:* the Part B and Part C pieces were never pasted here. They are the "B piece" of `$w\drafts\draft_B2.md` and Task C1 of `$w\drafts\draft_C2.md` (copied from the planning session's scratchpad). Task 4 ran from them on 2026-10-03 (commit c3895bf on `claude/s1-t4`). The exact capture commands are in `$w\progress.md`. Later tasks compare against `$w\baseline\` and `$w\partB\base_corpus`, `base_fix2` and `base_fix4`, with `$w\partB\replay_head.exe` as the 02976c1 oracle. `scores.ps1` now takes `-Corpus` (default: the main checkout's `testdata\input`) and `-Repo` only picks the binaries.

### Task 5: One SP-end history per activation

*Draft label: D1 (merges A2, A6).*

**Goal:** Each activation stores `sp_end_steps` once. `deact_tick()`, `clamp_tick()`, `collected_phrase_ticks()`, `nominal_end()`, the measured-from end of each squeeze, the anchor of any end, and the gauge's refill point are all read from it.

**Findings:** 27 and the SqIn half of 28 (from A2), 156 (from A6), 5 and 159 (the stored fact the gauge needs).

**Decision:** D1 governs the anchor: "the transfer scale is measured from the collecting note (the stored `clamp_tick`), not the activation". The anchor accessor below is that rule, written once. D4 governs the guard: "A test must prove that no fresh record stores 'unknown'". Here that means no fresh activation has an empty history. R1 governs the shape.

Why this task: think of the SP end as a parking meter. Today the record keeps when the meter finally ran out (`deact_tick`), the last time the cap topped it up (`clamp_tick`), and the coins put in while it ran (`collected_phrase_ticks`). Part A wanted two more of the same: the time on the meter before a squeeze-in, and the time it would have had with no coins. Each is a different slice of one receipt. The engine already prints that receipt step by step. D1 stores the receipt and lets every reader read it.

**The history, step by step.** Each step is `{tick, end_tick, kind}`. `tick` is the note that caused it. `end_tick` is the SP end in force after it. The engine moves the end in exactly four places, one kind each. I checked `advance`, `branch_activate`, `branch_deactivate`, `create_deactivated_path` and the tail path. No other place moves the end, so no fifth kind is needed.

| Kind | Where the engine moves the end | `tick` | `end_tick` |
|---|---|---|---|
| `Activation` | `branch_activate`, `c.sp_end_time = aiet_val` (line 575) | the activation chord | the plain end: activation plus two measures per bar |
| `Collected` | `advance`, `sp_end_time = mit->second.to_tick` (line 503), not clamped | the phrase chord | old end plus two measures |
| `Clamped` | the same line, when `mit->second.clamped` (line 504) | the phrase chord | the phrase plus two measures per cap bar |
| `SqIn` | early: the phrase was collected in `advance`; late: `branch_deactivate`, `p.sp_end_time = e.sqin_time` (line 674) | the SqIn phrase chord, early or late | the end after the squeeze-in |

Two rules keep the list exact. A squeezed-out phrase has no step: `create_deactivated_path` drops every step at or after the squeezed-out phrase, the way it already trims the collected list (line 614). A buffered late-SqIn phrase gets no second step when `advance` reaches it, because its step was written at the deact node.

An early squeeze-in needs one extra move. Its phrase is collected in `advance` before the engine knows whether the path will squeeze in or out at the deact node. Both branches share that chain node. So when `branch_deactivate` takes the SqIn branch, it copies the chain from that phrase's step onward and relabels the step `SqIn`. Chain nodes are never edited in place.

A clamped phrase can never be a squeeze-in or squeeze-out phrase. A clamp leaves the path's SP end off the edge's `sqout_time`, so `deactivation_type` returns `DEACT_NONE` (engine.cpp lines 587-592; Part A's A2 case 4). So trimming a squeeze-out never removes a `Clamped` step.

**What each old field becomes.** All accessors live on `Activation` in `core/model.h`, implemented in `core/model.cpp`.

- `std::optional<int64_t> deact_tick() const` is the last step's `end_tick`, unset when the list is empty. It equals today's field on every path: a plain deactivation needs `sp_end_time == dest.tick` (line 591), a squeeze-out's trimmed list ends on the end before its phrase, which is the deact node, and a tail activation ends on `final_sp_end`, which is the path's `sp_end_time`.
- `std::optional<int64_t> clamp_tick() const` is the tick of the last `Clamped` step.
- `std::vector<int64_t> collected_phrase_ticks() const` is the tick of every step after the first, in order. That is every phrase collected while active, including the squeeze-in phrase, as today.
- `std::optional<int64_t> nominal_end() const` is the first step's `end_tick`. That is A6's `nominal_deact_tick`, the same value the graph built in `add_act_edge`.
- `std::optional<size_t> squeeze_end_step(size_t squeeze_index) const` is the index of the step whose `end_tick` the squeeze `sqinouts[squeeze_index]` was measured from. For the k-th SqIn in `sqinouts` it is the step just before the k-th `SqIn` step. For the SqOut it is the last step. SqIns are pushed in time order in both lists, so the k-th matches the k-th.
- `std::optional<int64_t> squeeze_end_tick(size_t squeeze_index) const` is that step's `end_tick`. This is A2's `SPSqueeze::end_tick`.
- `int64_t end_anchor_tick(size_t step_index) const` is the tick of the latest `Clamped` step at or before `step_index`, or the activation tick if none. This is A2's anchor rule (its four cases hold, see the tests). `end_anchor_tick(*squeeze_end_step(i))` replaces A2's `SPSqueeze::anchor_tick`. `end_anchor_tick(sp_end_steps.size() - 1)` is the final end's anchor, D1's rule.
- `int64_t refill_tick(size_t step_index) const` is where step `step_index`'s new end takes effect: its own tick, or for a late `SqIn` step that sits past the end in force, that end (`sp_end_steps[step_index - 1].end_tick`). The player hits a late squeeze-in phrase early, so the bar arrives at the old SP end and SP never stops. R1 says this is read from the list, never stored. This accessor is the one place that reads it.

**No field had to stay.** R1 asks for the reader check before relying on the list. Every reader in `src/` and `tools/` today reads one of the three fields:

| Reader | Reads | Same value from the accessor because |
|---|---|---|
| `squeeze_rating.cpp:22-23` `activation_deact_tick` | `deact_tick` | the deact equality above |
| `squeeze_rating.cpp:28, 40` `frontend_transfer_scales` | `deact_tick` | same (Part A's A3 rewrites this function on top of the accessors) |
| `squeeze_rating.cpp:178` | `clamp_tick` | the last `Clamped` step is the last clamp the path took (`p.clamp_tick`, line 505) |
| `core/replay.cpp:216, 220` `windows_for_path` | `deact_tick` | deact equality |
| `app/path_view.cpp:265-267` "SP overfilled at" | `clamp_tick` | clamp equality |
| `app/preview_view.cpp:140-147, 346` gauge | `collected_phrase_ticks` | every collected phrase gets exactly one step (D4 removes this reader anyway) |
| `tools/replay_json.cpp:85, 91` `paths_json` | `deact_tick` | deact equality |
| `tools/replay.cpp:775` `check_chart` | `deact_tick` | deact equality |
| `engine.cpp:1272` `frontend_transfer_scales(act, ...)` at copy-out | `deact_tick` | it runs after the history is assigned |

`tools/replay_json.cpp:87` and `tools/replay.cpp:768-770` also recompute the plain end with `plusmeasure`; they now read `nominal_end()` (A6, finding 156). The JSON key `nominal_deact_tick` stays. The reasoning is checked by Step 4's test, which compares the accessors with the old stored fields on every corpus activation before the fields are removed.

**Files:**
- Modify: `src/core/model.h:265-334` (`SpEndKind`, `SpEndStep`; the three fields at 281, 288 and 302 become accessors), `src/core/model.cpp` (accessor bodies).
- Modify: `src/search/engine.cpp:159-236` (Act, Path, Variant gain `end_tail`; new `EndNode`; OutAct gains `end_begin`/`end_end` and drops `clamp_tick` and `col_begin`/`col_end`), `329-368` (helpers), `460-528` (advance), `531-585` (branch_activate), `595-618` (create_deactivated_path), `648-678` (branch_deactivate), `760-773` (reduce_group), `941-1031` (emit), `1034-1046` (run), `1159-1276` (rebuild), `1318-1335` (run_search). The `cols_` chain, `Path::clamp_tick` and `Variant::clamp_tick`/`col_tail` are removed once Step 6 lands; the history carries both.
- Modify: `src/store/path_codec.cpp:135-139, 176-181` (the history replaces the three fields).
- Modify: `src/core/squeeze_rating.cpp:22-40, 178`, `src/core/replay.cpp:216-220`, `src/app/path_view.cpp:265-267`, `src/app/preview_view.cpp:346-351`, `tools/replay_json.cpp:84-93`, `tools/replay.cpp:767-776` (member reads become calls; tools read `nominal_end()`).
- Create: `tests/record_fixtures.h` (songs and fixture helpers shared by the test files).
- Test: `tests/test_search.cpp` (new cases after line 874; edit 876-891), `tests/test_path_codec.cpp`, and the fixture writes in `tests/test_preview_view.cpp:157, 756-757, 867-868, 948-949, 1423-1424`, `tests/test_path_view.cpp:350-351`, `tests/test_squeeze_rating.cpp:34, 56, 66, 97, 135, 160, 174, 201, 265, 310, 620, 731, 740, 762` (or wherever Part A's plan and A3 left them).

**Acceptance Criteria:**
- [ ] Late squeeze-in song: history {5760→13440 Activation, 13920→17280 SqIn}; `refill_tick(1) == 13440`; `deact_tick() == 17280`; `collected_phrase_ticks() == {13920}`; `squeeze_end_tick(0) == 13440`.
- [ ] Part A's SqIn-then-collect song: the SqIn's `squeeze_end_tick == 5376` and anchor 2304, while `deact_tick() == 8448`.
- [ ] Cap-2 overfill song: history {2304→5376 Activation, 3072→6144 Clamped, 3840→6912 Clamped}; `clamp_tick() == 3840`.
- [ ] Early squeeze-out song: the first activation's history is {5760→13440 Activation} only.
- [ ] Before the old fields are removed: on every corpus path (roots, variants, all-0 paths), `deact_tick()`, `clamp_tick()` and `collected_phrase_ticks()` equal the stored fields, and `nominal_end()` equals `plusmeasure(activation, 2 × sp_meter)`.
- [ ] Every fresh corpus activation has a non-empty history that starts on its own note and ends on its deact node.
- [ ] A codec round trip keeps the history, kinds included.
- [ ] `scores.ps1` output equals the baseline; `hydra_replay dump` on Allister - Overrated still prints `nominal_deact_tick` 82560 and 167040 for path 0.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="SP end history*,path codec*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
.\build_cpp.ps1 -Target hydra_replay
$s = "C:\Users\Patrick\Downloads\Hydra\hydra-test\.superpowers\sdd\2026-10-03-step1-engine-facts\partD"
pwsh -NoProfile -File "$s\scores.ps1" -Out "$s\after-D1.txt"
Compare-Object (Get-Content "$s\baseline-02976c1.txt") (Get-Content "$s\after-D1.txt")
# expect: no output
.\build-cpp\Release\hydra_replay.exe dump --chart "testdata\input\common\IB24\T1\Allister - Overrated\notes.mid" --db "$s\work\empty.db" --pretty
# expect: path 0's activations show "nominal_deact_tick": 82560 and 167040
```

**Steps:**

- [ ] **Step 1: Shared fixtures.** Create `tests/record_fixtures.h`. The three songs are the ones from Appendix C (Part D's first draft) Step 1, unchanged: `make_late_sqin_song()` (240 BPM, phrases 480, 1920, 13920, fill 5760), `make_early_sqout_song()` (phrases 480, 1920, 12960, 14400, fills 5760 and 17280), `make_ch10_fill_song()` (120 BPM, phrases 960 and 18144, fills 19200, 24960, 28800). Add the fixture helpers every hand-built activation now uses:

```cpp
// A window that collected nothing: SP ends where the banked bars put it.
inline void set_plain_window(Activation& a, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation}};
}

// A window the cap clamped at `clamp_tick`, ending at `end_tick`. A display
// test that states only those two facts gets a plain end equal to the final
// one; nothing it checks reads the plain end.
inline void set_clamped_window(Activation& a, int64_t clamp_tick, int64_t end_tick) {
    a.sp_end_steps = {{a.timecode.ticks(), end_tick, SpEndKind::Activation},
                      {clamp_tick, end_tick, SpEndKind::Clamped}};
}

// The bank an activation spends, for a test that needs only the count: one
// arrival per bar on the ticks just before the activation. Only the count
// (sp_meter()) matters to such a test.
inline void set_bank(Activation& a, int bars) {
    a.bank_rise_ticks.clear();
    for (int k = bars; k > 0; --k) a.bank_rise_ticks.push_back(a.timecode.ticks() - k);
}
```

(`set_bank` and the fill helper arrive with D2 and D3; they are listed here so the header has one home.) Move Part A's `build_tempo_song`, `sqin_then_collect_song`, `clamp_song`, `wide_search` and `find_act` (Appendix D) into this header too, so D1's tests and A3's share them.

- [ ] **Step 2: Failing tests.** Append to `tests/test_search.cpp` (include `record_fixtures.h`):

```cpp
TEST_CASE("SP end history: a late squeeze-in is a SqIn step on its phrase") {
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {5760, 13440, SpEndKind::Activation}, {13920, 17280, SpEndKind::SqIn}}));
    // The bar arrives at the old end: the player hits the phrase early.
    CHECK(act.refill_tick(1) == 13440);
    CHECK(act.deact_tick() == std::optional<int64_t>(17280));
    CHECK(act.nominal_end() == std::optional<int64_t>(13440));
    CHECK((act.collected_phrase_ticks() == std::vector<int64_t>{13920}));
    REQUIRE(act.sqinouts.size() == 1);
    CHECK(act.sqinouts[0].offset_ms == doctest::Approx(250.0));
    CHECK(act.squeeze_end_tick(0) == std::optional<int64_t>(13440));
}

TEST_CASE("SP end history: an early squeeze-in measures from the end before it") {
    // Part A's A2 case. The SqIn phrase at 5280 sits 250 ms before X = 5376.
    Song song = test::sqin_then_collect_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const Activation* act = test::find_act(paths, [](const Activation& a) {
        for (const SPSqueeze& s : a.sqinouts)
            if (s.kind == SqueezeKind::SqIn) return true;
        return false;
    });
    REQUIRE(act != nullptr);
    CHECK((act->sp_end_steps == std::vector<SpEndStep>{
               {2304, 5376, SpEndKind::Activation},
               {5280, 6912, SpEndKind::SqIn},
               {6144, 8448, SpEndKind::Collected}}));
    CHECK(act->deact_tick() == std::optional<int64_t>(8448));
    REQUIRE(act->sqinouts.front().kind == SqueezeKind::SqIn);
    CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(5376));  // X, not 8448 - 2 measures
    REQUIRE(act->squeeze_end_step(0).has_value());
    CHECK(act->end_anchor_tick(*act->squeeze_end_step(0)) == 2304);   // no clamp: the activation
    CHECK(act->sqinouts.front().offset_ms == doctest::Approx(-250.0).epsilon(1e-9));
}

TEST_CASE("SP end history: a squeeze-out measures from the deact node") {
    Song song = test::sqin_then_collect_song();
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, test::wide_search());
    const Activation* act = test::find_act(paths, [](const Activation& a) {
        for (const SPSqueeze& s : a.sqinouts)
            if (s.kind == SqueezeKind::SqOut) return true;
        return false;
    });
    REQUIRE(act != nullptr);
    for (size_t i = 0; i < act->sqinouts.size(); ++i) {
        if (act->sqinouts[i].kind != SqueezeKind::SqOut) continue;
        CHECK(act->squeeze_end_tick(i) == act->deact_tick());
        CHECK(act->end_anchor_tick(*act->squeeze_end_step(i)) ==
              act->clamp_tick().value_or(act->timecode.ticks()));
    }
}

TEST_CASE("SP end history: clamps are steps, and the anchor follows them") {
    Song song = build_tail_song({{0, true, false}, {768, true, false},
                                 {2304, false, true}, {3072, true, false},
                                 {3840, true, false}, {4608}, {5376}, {6000},
                                 {6768}, {7500}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Activation& act = paths.front().activations.front();
    CHECK((act.sp_end_steps == std::vector<SpEndStep>{
               {2304, 5376, SpEndKind::Activation},
               {3072, 6144, SpEndKind::Clamped},
               {3840, 6912, SpEndKind::Clamped}}));
    CHECK(act.clamp_tick() == std::optional<int64_t>(3840));
    CHECK(act.end_anchor_tick(0) == 2304);
    CHECK(act.end_anchor_tick(1) == 3072);
    CHECK(act.end_anchor_tick(2) == 3840);
}

TEST_CASE("SP end history: a squeezed-out phrase leaves no step") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 2);
    const Activation& first = paths.front().activations[0];
    CHECK(first.sqout_tick == std::optional<int64_t>(12960));
    CHECK((first.sp_end_steps ==
           std::vector<SpEndStep>{{5760, 13440, SpEndKind::Activation}}));
    CHECK(first.collected_phrase_ticks().empty());
}
```

And the corpus case, in two stages. Stage one (Step 4) compares the accessors with the old fields; it is written against temporary accessor names, because the old fields still own the real names:

```cpp
// R1's reader check: before the stored fields go, every corpus activation's
// history must give exactly what they hold.
TEST_CASE("SP end history: equals the stored fields on every corpus record") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    int acts = 0;
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            for (const Activation& act : p->walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                ++acts;
                REQUIRE_FALSE(act.sp_end_steps.empty());
                CHECK(act.steps_deact_tick() == act.deact_tick);
                CHECK(act.steps_clamp_tick() == act.clamp_tick);
                CHECK(act.steps_collected_phrase_ticks() == act.collected_phrase_ticks);
                CHECK(act.nominal_end() ==
                      song.timing().plusmeasure(act.timecode, sp_bars_to_measures(act.sp_meter)).ticks());
            }
        }
    }
    CHECK(acts > 1000);
}
```

Stage two (Step 6) rewrites it as the lasting invariant check, once the fields are gone:

```cpp
TEST_CASE("SP end history: every corpus activation is consistent") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            for (const Activation& act : p->walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                // D4's condition: no fresh record leaves the history out.
                REQUIRE_FALSE(act.sp_end_steps.empty());
                CHECK(act.sp_end_steps.front().kind == SpEndKind::Activation);
                CHECK(act.sp_end_steps.front().tick == act.timecode.ticks());
                for (size_t k = 1; k < act.sp_end_steps.size(); ++k) {
                    const SpEndStep& prev = act.sp_end_steps[k - 1];
                    const SpEndStep& s = act.sp_end_steps[k];
                    CHECK(s.kind != SpEndKind::Activation);
                    CHECK(s.tick > prev.tick);
                    // SP never runs dry inside a window: every step takes
                    // effect at or before the end in force.
                    CHECK(act.refill_tick(k) <= prev.end_tick);
                    CHECK(act.refill_tick(k) <= s.tick);
                }
            }
        }
    }
}
```

In `tests/test_path_codec.cpp` add:

```cpp
TEST_CASE("path codec: a node keeps the SP-end history") {
    Activation act;
    act.timecode = Timecode::raw(2304);
    act.sp_end_steps = {{2304, 5376, SpEndKind::Activation},
                        {3072, 6144, SpEndKind::Clamped},
                        {5280, 6912, SpEndKind::SqIn},
                        {6144, 8448, SpEndKind::Collected}};
    Path path;
    path.activations.push_back(act);
    const Path back = store::decode_path_node(store::encode_path_node(path));
    REQUIRE(back.activations.size() == 1);
    CHECK(back.activations.front().sp_end_steps == act.sp_end_steps);
    CHECK(back.activations.front().deact_tick() == std::optional<int64_t>(8448));
    CHECK(back.activations.front().clamp_tick() == std::optional<int64_t>(3072));
}
```

Replace the existing "path codec: encode/decode a path node keeps clamp_tick" (`test_search.cpp:876-891`) with `test::set_clamped_window(act, 3072, 6144);` and `CHECK(decoded.activations.front().clamp_tick() == std::optional<int64_t>(3072));`.

- [ ] **Step 3: Build and watch them fail.** The build fails: `SpEndKind`, `sp_end_steps` and the accessors do not exist.

- [ ] **Step 4: Add the history beside the old fields, and prove them equal.** In `src/core/model.h`, before `Activation`:

```cpp
// Why an activation's SP end moved (Activation::sp_end_steps).
//   Activation - the activation itself: the end its banked bars give.
//   Collected  - a phrase collected while active: two measures more.
//   Clamped    - a phrase collected with the meter full: the end pinned to
//                the cap's length past that phrase (ADR 0013).
//   SqIn       - the squeeze-in phrase, early or late.
enum class SpEndKind : uint8_t { Activation = 0, Collected = 1, Clamped = 2, SqIn = 3 };

// One place an activation's SP end moved: `tick` is the note that moved it,
// `end_tick` the SP end in force after it.
struct SpEndStep {
    int64_t tick = 0;
    int64_t end_tick = 0;
    SpEndKind kind = SpEndKind::Activation;
    bool operator==(const SpEndStep&) const = default;
};
```

In `Activation`, add the field next to the three it will replace:

```cpp
    // Every place this window's SP end moved, in order, as the search did it.
    // The first step is the activation. A squeezed-out phrase has no step. A
    // tail activation's last end is the end the search tracked. Stamped at
    // copy-out; empty only on a hand-built activation. Every SP-end fact the
    // record holds is read from this list (the accessors below). Nothing
    // re-derives it.
    std::vector<SpEndStep> sp_end_steps;
```

For this stage only, add the accessors as `steps_deact_tick()`, `steps_clamp_tick()` and `steps_collected_phrase_ticks()`, plus `nominal_end()`, `squeeze_end_step()`, `squeeze_end_tick()`, `end_anchor_tick()` and `refill_tick()` under their final names. Their bodies in `src/core/model.cpp`:

```cpp
std::optional<int64_t> Activation::deact_tick() const {  // steps_deact_tick in Step 4
    if (sp_end_steps.empty()) return std::nullopt;
    return sp_end_steps.back().end_tick;
}

std::optional<int64_t> Activation::clamp_tick() const {  // steps_clamp_tick in Step 4
    for (auto it = sp_end_steps.rbegin(); it != sp_end_steps.rend(); ++it)
        if (it->kind == SpEndKind::Clamped) return it->tick;
    return std::nullopt;
}

std::vector<int64_t> Activation::collected_phrase_ticks() const {  // steps_... in Step 4
    std::vector<int64_t> out;
    for (size_t k = 1; k < sp_end_steps.size(); ++k) out.push_back(sp_end_steps[k].tick);
    return out;
}

std::optional<int64_t> Activation::nominal_end() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return sp_end_steps.front().end_tick;
}

std::optional<size_t> Activation::squeeze_end_step(size_t squeeze_index) const {
    if (squeeze_index >= sqinouts.size() || sp_end_steps.empty()) return std::nullopt;
    if (sqinouts[squeeze_index].kind == SqueezeKind::SqOut) return sp_end_steps.size() - 1;
    // The k-th SqIn squeeze is the k-th SqIn step: both are kept in time order.
    size_t k = 0;
    for (size_t i = 0; i < squeeze_index; ++i)
        if (sqinouts[i].kind == SqueezeKind::SqIn) ++k;
    for (size_t s = 1; s < sp_end_steps.size(); ++s)
        if (sp_end_steps[s].kind == SpEndKind::SqIn && k-- == 0) return s - 1;
    return std::nullopt;
}

std::optional<int64_t> Activation::squeeze_end_tick(size_t squeeze_index) const {
    const std::optional<size_t> s = squeeze_end_step(squeeze_index);
    if (!s) return std::nullopt;
    return sp_end_steps[*s].end_tick;
}

int64_t Activation::end_anchor_tick(size_t step_index) const {
    for (size_t s = std::min(step_index + 1, sp_end_steps.size()); s-- > 0;)
        if (sp_end_steps[s].kind == SpEndKind::Clamped) return sp_end_steps[s].tick;
    return timecode.ticks();
}

int64_t Activation::refill_tick(size_t step_index) const {
    const SpEndStep& s = sp_end_steps.at(step_index);
    if (step_index == 0 || s.kind != SpEndKind::SqIn) return s.tick;
    // A late squeeze-in: the phrase sits past the end in force, and the
    // player hits it early, so the bar arrives at that end.
    return std::min(s.tick, sp_end_steps[step_index - 1].end_tick);
}
```

In `src/search/engine.cpp`, give `Act`, `Path` and `Variant` an `int32_t end_tail;` ("the SP-end steps so far, an index into ends_, or -1"), and add after `ColNode`:

```cpp
// One SP-end step of the running activation, linked to the one before.
struct EndNode {
    int32_t prev;
    int64_t tick;
    int64_t end;
    SpEndKind kind;
};
```

Add to the Engine class, after `trim_cols`:

```cpp
    int32_t push_end(int32_t prev, int64_t tick, int64_t end, SpEndKind kind) {
        ends_.push_back(EndNode{prev, tick, end, kind});
        return (int32_t)ends_.size() - 1;
    }
    // The steps with every one at or after `tick` dropped: a squeeze-out
    // gives its phrase back, and with it that phrase's step.
    int32_t trim_ends(int32_t tail, int64_t tick) const {
        while (tail >= 0 && ends_[(size_t)tail].tick >= tick) tail = ends_[(size_t)tail].prev;
        return tail;
    }
    // The chain with the step at `tick` relabelled SqIn. Nodes are shared
    // between paths, so the steps from it on are copied, never edited.
    int32_t relabel_sqin(int32_t tail, int64_t tick) {
        std::vector<EndNode> after;
        int32_t t = tail;
        while (t >= 0 && ends_[(size_t)t].tick > tick) {
            after.push_back(ends_[(size_t)t]);
            t = ends_[(size_t)t].prev;
        }
        if (t < 0 || ends_[(size_t)t].tick != tick) return tail;  // the corpus test catches this
        const EndNode found = ends_[(size_t)t];
        int32_t out = push_end(found.prev, found.tick, found.end, SpEndKind::SqIn);
        for (size_t k = after.size(); k-- > 0;)
            out = push_end(out, after[k].tick, after[k].end, after[k].kind);
        return out;
    }
    void emit_ends(int32_t tail, int32_t* begin, int32_t* end) {
        end_scratch_.clear();
        for (int32_t s = tail; s >= 0; s = ends_[(size_t)s].prev) end_scratch_.push_back(s);
        *begin = (int32_t)out_ends_.size();
        for (size_t k = end_scratch_.size(); k-- > 0;) {
            const EndNode& e = ends_[(size_t)end_scratch_[k]];
            out_ends_.push_back(SpEndStep{e.tick, e.end, e.kind});
        }
        *end = (int32_t)out_ends_.size();
    }
```

with members `std::vector<EndNode> ends_;`, `std::vector<SpEndStep> out_ends_;`, `std::vector<int32_t> end_scratch_;`, the accessor `out_ends()`, and `int32_t end_begin, end_end;` on `OutAct`. Then push the steps where the end moves:

- `branch_activate`, after `c.col_tail = -1;` (line 577): `c.end_tail = push_end(-1, n.tick, aiet_val, SpEndKind::Activation);`
- `advance`, after `sp_end_time = mit->second.to_tick;` (line 503):
  ```cpp
                  p.end_tail = push_end(p.end_tail, eo->sp_times[(size_t)i].first.ticks(),
                                        sp_end_time,
                                        mit->second.clamped ? SpEndKind::Clamped
                                                            : SpEndKind::Collected);
  ```
  The buffered branch (`continue` at line 495) pushes nothing.
- `create_deactivated_path`: `c.end_tail = -1;` with the other resets, and in the Act block `a.end_tail = is_sq_out ? trim_ends(p.end_tail, e.sqinout_time) : p.end_tail;`
- `branch_deactivate`, before `p.sp_end_time = e.sqin_time;` (line 674):
  ```cpp
      // The squeeze-in's step. An early one's phrase is already a Collected
      // step (advance collected it before this branch point): relabel it. A
      // late one's phrase comes after this node: its step is written here,
      // on the phrase, and advance skips it as buffered.
      if (e.sqinout_time <= n.tick)
          p.end_tail = relabel_sqin(p.end_tail, e.sqinout_time);
      else
          p.end_tail = push_end(p.end_tail, e.sqinout_time, e.sqin_time, SpEndKind::SqIn);
  ```
- `reduce_group`: `v.end_tail = p.end_tail;`. `run`: `root.end_tail = -1;`. `new_act`: `a.end_tail = -1;`.
- `emit_acts` gains an `int32_t end_tail` parameter; per Act `emit_ends(a.end_tail, &oa.end_begin, &oa.end_end);`, and in the still-running block `emit_ends(end_tail, &last.end_begin, &last.end_end);`. `emit_variant` passes `var.end_tail`, `emit_path` passes `p.end_tail`.
- `rebuild` gains `const std::vector<SpEndStep>& out_ends`; after the collected assign (line 1264): `act.sp_end_steps.assign(out_ends.begin() + oa.end_begin, out_ends.begin() + oa.end_end);`. `run_search` passes `engine.out_ends()`.

In `path_codec.cpp`, write the history after `collected_phrase_ticks` and read it back:

```cpp
    w.u32(static_cast<uint32_t>(act.sp_end_steps.size()));
    for (const SpEndStep& s : act.sp_end_steps) {
        w.i64(s.tick);
        w.i64(s.end_tick);
        w.u8(static_cast<uint8_t>(s.kind));
    }
```

```cpp
    const uint32_t nsteps = r.u32();
    act.sp_end_steps.reserve(nsteps);
    for (uint32_t i = 0; i < nsteps; ++i) {
        SpEndStep s;
        s.tick = r.i64();
        s.end_tick = r.i64();
        const uint8_t kind = r.u8();
        if (kind > static_cast<uint8_t>(SpEndKind::SqIn))
            throw SerializeError("unknown SP-end step kind");
        s.kind = static_cast<SpEndKind>(kind);
        act.sp_end_steps.push_back(s);
    }
```

Run `hydra_tests.exe -tc="SP end history*"`. Every case passes, including "equals the stored fields on every corpus record". If that case fails anywhere, stop. Report which reader would get a different value. Under R1 that field then stays, with the reason written down.

- [ ] **Step 5: Run the score proof.** The Verify block's `scores.ps1` comparison prints nothing.

- [ ] **Step 6: Remove the three stored fields.** Delete `deact_tick` (model.h:281), `clamp_tick` (288) and `collected_phrase_ticks` (302) from `Activation`. Rename `steps_deact_tick`, `steps_clamp_tick` and `steps_collected_phrase_ticks` to `deact_tick`, `clamp_tick` and `collected_phrase_ticks`. Their declarations:

```cpp
    // Read from sp_end_steps; see each body in model.cpp.
    std::optional<int64_t> deact_tick() const;           // the last step's end
    std::optional<int64_t> clamp_tick() const;           // the last Clamped step's tick
    std::vector<int64_t> collected_phrase_ticks() const; // every step after the first
    std::optional<int64_t> nominal_end() const;          // the first step's end
    std::optional<size_t> squeeze_end_step(size_t squeeze_index) const;
    std::optional<int64_t> squeeze_end_tick(size_t squeeze_index) const;
    int64_t end_anchor_tick(size_t step_index) const;
    int64_t refill_tick(size_t step_index) const;
```

In the codec, drop the `opt_i64(deact_tick)`, `opt_i64(clamp_tick)` and collected-list writes and reads (lines 135-139, 176-181). In the engine, delete what only fed them: the `deact_tick` and `clamp_tick` stamps in `rebuild` (1243-1260), the `collected_phrase_ticks` assign (1263-1264), the `cols_` chain and `push_col`/`trim_cols`/`emit_cols`, `col_tail` on Act, Path and Variant, `clamp_tick` on Act, Path and Variant (the history's `Clamped` steps hold it), and `col_begin`/`col_end`/`clamp_tick` on `OutAct`. The `final_sp_end` field stays: `rebuild` still needs it for the tail backends' offsets (line 1201).

Then fix every reader the compiler names. Each is mechanical, a member read becoming a call: `act.deact_tick` → `act.deact_tick()`, `act.clamp_tick` → `act.clamp_tick()`. In `tools/replay_json.cpp:86-87` and `tools/replay.cpp:768-770`, delete the `nominal` computation and read `act.nominal_end().value_or(-1)`.

Fixture writes become histories, through the helpers from Step 1:
- `test_preview_view.cpp:157` (`sp_act_at`): `test::set_plain_window(a, <the stated end>)`. Use the literal ends from A6 (Appendix E).
- `test_preview_view.cpp:756-757`, `867-868`, `1423-1424`: `a.sp_end_steps = {{3840, 11520, SpEndKind::Activation}, {5760, 15360, SpEndKind::Collected}};`
- `test_preview_view.cpp:948-949`: `act.sp_end_steps = {{act_tick, act_tick + 8 * 1920, SpEndKind::Activation}, {phrase_tick, deact, SpEndKind::Collected}};`
- `test_path_view.cpp:350-351`: `test::set_clamped_window(act, 960, 6144);`
- `test_squeeze_rating.cpp`: each `act.deact_tick = X;` becomes `test::set_plain_window(act, X);`, and each line pair that also sets `clamp_tick = C` becomes `test::set_clamped_window(act, C, X);`. A case that set `clamp_tick` alone (731, 740, 762) gets `set_clamped_window(act, 3072, <the end that case already used>)`.

Rewrite Step 2's stage-one corpus case as the stage-two case shown there.

- [ ] **Step 7: Run everything.** Run the Verify block in full.

- [ ] **Step 8: Commit.** Stage by name: `src/core/model.h src/core/model.cpp src/search/engine.cpp src/store/path_codec.cpp src/core/squeeze_rating.cpp src/core/replay.cpp src/app/path_view.cpp src/app/preview_view.cpp tools/replay_json.cpp tools/replay.cpp tests/record_fixtures.h tests/test_search.cpp tests/test_path_codec.cpp tests/test_preview_view.cpp tests/test_path_view.cpp tests/test_squeeze_rating.cpp`. Commit "One SP-end history per activation; the end, clamp and collected phrases read from it".

### Task 6: The bank list replaces `sp_meter` and `leftover_sp`

*Draft label: D2.*

**Goal:** Each activation stores where each bar it spends arrived, each path stores where each leftover bar arrived, and `sp_meter()` and `leftover_sp()` are those lists' sizes.

**Findings:** 147.

**Decision:** R2: "Where a new list holds exactly the items a stored count counts, the list is stored and the count becomes an accessor." Standing rule otherwise.

Why this task: between windows the gauge copies the engine's banking rule (one bar per phrase, stop at the cap, a squeeze-out banks one). The engine decides each of those bars in `Engine::advance` and `Engine::create_deactivated_path`. Storing where each bar arrived lets the gauge just step there. And the count of bars is then the list's length, so it is no longer stored twice.

The list follows the engine's bank (`p.sp`) exactly. A phrase that raises the bank adds its tick. A phrase hit at the cap adds nothing. A squeeze-out adds one entry when the window closes, at the later of the deact node and the squeezed-out phrase, which is when the player hits it. After a late squeeze-out the engine gives that bar back on the next base edge if the phrase is not on it, and adds it again on the phrase's own edge. The list pops and pushes in step, so it ends with one entry.

**Is each count exactly its list's size?** Yes, by construction, and Step 3 checks it on every record before the counts go. `Act.sp_meter` is `p.sp` at activation (line 568), and the list is kept equal to `p.sp` at every change of `p.sp`: `advance` (line 514), `branch_activate` (`c.sp = 0`, list reset), `create_deactivated_path` (`c.sp` is 0 or 1, list reset or one entry). Nothing else writes `p.sp`. `leftover_sp` is `p.sp` at the end (line 1023), so it is the trailing list's size. A path whose last activation outlasts the chart ends active with `p.sp == 0` and an empty list. Variants copy both from their root today (`prepare_variants`), so they stay equal; Part B gives each variant its own (finding 89).

**Files:**
- Modify: `src/core/model.h:271` (`sp_meter` becomes `int sp_meter() const`), `404` (`leftover_sp` becomes `int leftover_sp() const`), new fields; `src/core/model.cpp:540-556` (`prepare_variants` copies `trailing_bank_ticks` instead of `leftover_sp`).
- Modify: `src/search/engine.cpp` (Act and Path gain `bank_tail`; pool `banks_`; `out_ticks_` shared tick output; `OutAct`/`OutPath` gain ranges and drop `sp_meter`/`leftover_sp`; `advance` 510-525; `branch_activate` 561-583; `create_deactivated_path` 600-605; `emit_acts`; `emit_path` 1014-1031; `run`; `rebuild` 1179, 1191).
- Modify: `src/store/path_codec.cpp:108, 147` (no more `i32 sp_meter`), `196, 207` (no more `i32 leftover_sp`; the trailing list instead).
- Modify readers: `src/app/path_view.cpp:202-203, 355-357`, `src/app/preview_view.cpp:112-168, 344`, `tools/replay_json.cpp:94`, `tools/replay.cpp:776`. Each becomes a call.
- Test: `tests/test_search.cpp`, `tests/test_path_codec.cpp`; fixture writes of `sp_meter` and `leftover_sp` in `tests/test_model.cpp`, `test_path_view.cpp`, `test_preview_view.cpp`, `test_squeeze_rating.cpp` (the 47 count writes across the four files, `skips` included) go through `test::set_bank` and `test::set_leftover`.

**Acceptance Criteria:**
- [ ] Early squeeze-out song: the second activation's list is {13440, 14400}; the first's is {480, 1920}; the trailing list is empty.
- [ ] Before the counts are removed: on every corpus path (roots, variants, all-0), each activation's list size equals the stored `sp_meter` and each path's trailing size equals `leftover_sp`.
- [ ] On every corpus root, each list is ascending and inside (previous deact node, activation note]; after a squeeze-out the next list starts at the later of the deact node and the squeezed-out phrase.
- [ ] A round trip keeps both lists.
- [ ] `scores.ps1` equals the baseline. `hydra_bench testdata\input` is within 5% of HEAD.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="Bank*,SP end history*,path codec*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
.\build_cpp.ps1 -Target hydra_replay
pwsh -NoProfile -File "$s\scores.ps1" -Out "$s\after-D2.txt"
Compare-Object (Get-Content "$s\baseline-02976c1.txt") (Get-Content "$s\after-D2.txt")
# expect: no output
# One benchmark at a time. Time the HEAD exe first (copy it aside before rebuilding):
.\build_cpp.ps1 -Target hydra_bench
.\build-cpp\Release\hydra_bench.exe testdata\input
# expect: total time within 5% of HEAD's
```

**Steps:**

- [ ] **Step 1: Failing tests.** Append to `tests/test_search.cpp`:

```cpp
TEST_CASE("Bank: a squeezed-out bar arrives at the deact node") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    const Path& path = paths.front();
    REQUIRE(path.activations.size() == 2);
    CHECK((path.activations[0].bank_rise_ticks == std::vector<int64_t>{480, 1920}));
    // The phrase at 12960 was hit late, just after SP ended at 13440.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
    CHECK(path.trailing_bank_ticks.empty());
}

// R2's check, stage one: the lists hold exactly what the counts count.
TEST_CASE("Bank: the lists match the stored counts on every corpus record") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all) {
            CAPTURE(chart);
            CHECK(p->trailing_bank_ticks.size() == static_cast<size_t>(p->leftover_sp));
            for (const Activation& act : p->walk_activations())
                CHECK(act.bank_rise_ticks.size() == static_cast<size_t>(act.sp_meter));
        }
    }
}

// The lasting order checks, on root paths. A variant's tail activations
// are its leader's; their order against the variant's own windows is Part
// B's job, and Part B extends this case to variants.
TEST_CASE("Bank: every corpus root banks in order") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        for (const Path& root : corpus::analyzed(chart, cfg).paths) {
            const Activation* prev = nullptr;
            for (const Activation& act : root.walk_activations()) {
                CAPTURE(chart);
                CAPTURE(act.timecode.ticks());
                const int64_t floor = prev ? *prev->deact_tick() : std::numeric_limits<int64_t>::min();
                for (size_t k = 0; k < act.bank_rise_ticks.size(); ++k) {
                    CHECK(act.bank_rise_ticks[k] >= floor);
                    CHECK(act.bank_rise_ticks[k] <= act.timecode.ticks());
                    if (k > 0) CHECK(act.bank_rise_ticks[k] >= act.bank_rise_ticks[k - 1]);
                }
                if (prev && prev->sqout_tick) {
                    REQUIRE_FALSE(act.bank_rise_ticks.empty());
                    CHECK(act.bank_rise_ticks.front() ==
                          std::max(*prev->deact_tick(), *prev->sqout_tick));
                }
                prev = &act;
            }
        }
    }
}
```

In `tests/test_path_codec.cpp`, the node case "path codec: a node keeps bank_rise_ticks" and the root case "path codec: a root keeps trailing_bank_ticks" as in Appendix C (Part D's first draft) D2 Step 1, with the root case setting only `trailing_bank_ticks = {111, 222}` and checking `back.paths.front().leftover_sp() == 2` as well.

- [ ] **Step 2: Build and watch them fail** on the missing fields.

- [ ] **Step 3: Add the lists beside the counts, and prove them equal.** Fields (model.h), after `sp_end_steps` and after `leftover_sp`:

```cpp
    // Where each bar this activation spends arrived, since the previous window
    // closed (or the chart began), in order. A phrase hit at the cap gains
    // nothing and is not here. A squeezed-out phrase's bar is here at the
    // later of the previous deact node and that phrase, when the player hits
    // it. Stamped by the search. sp_meter() is its size.
    std::vector<int64_t> bank_rise_ticks;
```

```cpp
    // The same after the path's last window closed. leftover_sp() is its
    // size. A variant copies its root's (prepare_variants).
    std::vector<int64_t> trailing_bank_ticks;
```

Engine (`engine.cpp`): `int32_t bank_tail;` on `Act` and `Path`; pool `std::vector<ColNode> banks_;` (the tick-chain node, reused); shared output `std::vector<int64_t> out_ticks_;` with accessor `out_ticks()`; `int32_t bank_begin, bank_end;` on `OutAct` and `OutPath`. A generic emitter and pusher (D1 removed `emit_cols`, so this is the only tick-chain emitter):

```cpp
    int32_t push_tick(std::vector<ColNode>& pool, int32_t prev, int64_t tick) {
        pool.push_back(ColNode{prev, tick});
        return (int32_t)pool.size() - 1;
    }
    void emit_ticks(const std::vector<ColNode>& pool, int32_t tail, int32_t* begin, int32_t* end) {
        tick_scratch_.clear();
        for (int32_t c = tail; c >= 0; c = pool[(size_t)c].prev) tick_scratch_.push_back(c);
        *begin = (int32_t)out_ticks_.size();
        for (size_t k = tick_scratch_.size(); k-- > 0;)
            out_ticks_.push_back(pool[(size_t)tick_scratch_[k]].tick);
        *end = (int32_t)out_ticks_.size();
    }
```

`advance`, inactive branch, after `p.sp = sp;` (line 514):

```cpp
        // Keep the banked bars in step with p.sp. A gain is a phrase past the
        // buffered ones (a squeeze-out already paid for those). A loss happens
        // only when a buffered phrase is not on this edge: the engine hands the
        // squeeze-out's bar back here and the phrase's own edge adds it again.
        for (int32_t k = sp; k < old_sp; ++k) p.bank_tail = banks_[(size_t)p.bank_tail].prev;
        for (int32_t k = 0; k < sp - old_sp; ++k)
            p.bank_tail = push_tick(banks_, p.bank_tail,
                                    eo->sp_times[(size_t)(buffered + k)].first.ticks());
```

`branch_activate`, right after `c.act_tail = new_act(...)`: `acts_[(size_t)c.act_tail].bank_tail = p.bank_tail;` then `c.bank_tail = -1;`. `create_deactivated_path`, with the resets:

```cpp
    // A squeeze-out banks one bar when the player hits the phrase: just after
    // SP ends for an early phrase, on its own tick for a late one.
    c.bank_tail = is_sq_out ? push_tick(banks_, -1, std::max(node(e.dest).tick, e.sqinout_time))
                            : -1;
```

`new_act` and `run`: `bank_tail = -1`. `emit_acts`: `emit_ticks(banks_, a.bank_tail, &oa.bank_begin, &oa.bank_end);`. `emit_path`, before `emit_acts`: `emit_ticks(banks_, p.bank_tail, &op.bank_begin, &op.bank_end);`. `emit_variant` leaves its zeroed `OutPath` range empty. `rebuild` gains `out_ticks`, and assigns `act.bank_rise_ticks` from the activation range and, for every path, `path.trailing_bank_ticks` from the path range. In `prepare_variants` add `v.trailing_bank_ticks = trailing_bank_ticks;` next to the `leftover_sp` copy. The codec writes and reads both lists (activation: after the history; root totals: after `leftover_sp`), each as a u32 count and i64 ticks.

Run "Bank: the lists match the stored counts on every corpus record". If any count differs from its list anywhere, stop. Under R2 both stay, and the task writes down why they are two facts.

- [ ] **Step 4: The counts become accessors.** Delete `int sp_meter` (model.h:271) and `int leftover_sp` (404). Declare:

```cpp
    // Bars of SP this activation spends: one per stored arrival.
    int sp_meter() const { return static_cast<int>(bank_rise_ticks.size()); }
```

```cpp
    // Bars left after the last window: one per stored arrival.
    int leftover_sp() const { return static_cast<int>(trailing_bank_ticks.size()); }
```

Drop the `i32 sp_meter` (path_codec.cpp:108, 147) and `i32 leftover_sp` (196, 207) from the codec. Drop `sp_meter` from `OutAct` and `leftover_sp` from `OutPath`; `Act::sp_meter` goes too (it was only copied out). In `prepare_variants` remove the `leftover_sp` copy (the trailing copy carries it). Fix each reader the compiler names: `act.sp_meter` → `act.sp_meter()`, `path.leftover_sp` → `path.leftover_sp()`. Fixture writes become `test::set_bank(a, n)` (record_fixtures.h) and, for paths, a `test::set_leftover(p, n)` written the same way (n placeholder ticks before the chart's last note, with the same "only the count matters" comment). Fixtures that are about where bars arrive (the Preview gauge cases) state real ticks instead (D4). Delete stage one's equality case.

- [ ] **Step 5: Run everything, the score proof and the benchmark.** If the benchmark is more than 5% slower, stop and report the numbers.

- [ ] **Step 6: Commit.** Stage by name. Commit "Bank arrivals stored; sp_meter and leftover_sp are their sizes".

### Task 7: The passed-over fill list replaces `skips`

*Draft label: D3.*

**Goal:** Each activation stores the ticks of the fills its path was shown and passed over, and `skips()` is that list's size.

**Findings:** 52.

**Decision:** R2. The visible effect is Q9.

Why this task: `Engine::branch_activate` charges a skip only on a fill that was really offered (enough SP, deadline open), and knows which fill that was (`n.tick`). The record keeps only the count, so the Preview guesses which fills. Storing the ticks makes the count their length.

**Is `skips` exactly the list's size?** Yes, by construction. `Act.skips` is `p.currentskips` at activation (line 568). The list gains one tick exactly where `currentskips += 1` (line 579) and is reset exactly where `currentskips = 0` (line 563). Nothing else writes either. Step 3 checks it on every record first. The engine keeps its own `Act::skips` int for `act_difficulty`'s E0 test (line 690); that is engine-internal bookkeeping, not stored, and the record's `skips()` reads the list.

**Files:**
- Modify: `src/core/model.h:268` (`skips` becomes `int skips() const`) and the new field; `src/core/model.cpp:417, 443, 602` (reads become calls).
- Modify: `src/search/engine.cpp` (Path and Act gain `skip_tail`; pool `fills_`; `OutAct` gains `skip_begin`/`skip_end` and drops `skips`; `branch_activate` 561-583; `new_act`; `run`; `emit_acts`; `rebuild` 1186).
- Modify: `src/store/path_codec.cpp:105, 144`; readers `src/store/record_store.cpp:479`, `src/app/preview_view.cpp:345`, `tools/replay_json.cpp:95`, `tools/replay.cpp:777`.
- Test: `tests/test_search.cpp`, `tests/test_path_codec.cpp`, count writes via `test::set_skips`.

**Acceptance Criteria:**
- [ ] 1.0-rule fill song: the activation at 28800 stores {19200}, and `skips() == 1`.
- [ ] Before the count is removed: on every corpus path, the list size equals the stored `skips`.
- [ ] On every corpus root, each list is ascending, after the previous activation, before its own, and names real fills.
- [ ] A round trip keeps the list. `scores.ps1` equals the baseline.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="Skipped fills*,path codec*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
.\build_cpp.ps1 -Target hydra_replay
pwsh -NoProfile -File "$s\scores.ps1" -Out "$s\after-D3.txt"
Compare-Object (Get-Content "$s\baseline-02976c1.txt") (Get-Content "$s\after-D3.txt")
# expect: no output
```

**Steps:**

- [ ] **Step 1: Failing tests.** Append to `tests/test_search.cpp`:

```cpp
TEST_CASE("Skipped fills: the 1.0 rule's offered fill is the one stored") {
    // Fill A (19200) was shown and passed over; fill B (24960) has the earlier
    // 1.0 deadline and never spawned. The nearest-n guess would name B.
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    REQUIRE(paths.front().activations.size() == 1);
    const Activation& act = paths.front().activations.front();
    CHECK((act.skipped_fill_ticks == std::vector<int64_t>{19200}));
}

TEST_CASE("Skipped fills: the list matches the stored count on every corpus record") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(chart, cfg);
        std::vector<const Path*> all = rec.all_paths();
        for (const Path* p : rec.all_allzero_paths()) all.push_back(p);
        for (const Path* p : all)
            for (const Activation& act : p->walk_activations()) {
                CAPTURE(chart);
                CHECK(act.skipped_fill_ticks.size() == static_cast<size_t>(act.skips));
            }
    }
}

TEST_CASE("Skipped fills: every corpus root passes over real fills in order") {
    const AnalysisSettings cfg = Settings().to_analysis_settings();
    for (const std::string& chart : corpus::chart_paths()) {
        const Song& song = corpus::song(chart, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        for (const Path& root : corpus::analyzed(chart, cfg).paths) {
            const Activation* prev = nullptr;
            for (const Activation& act : root.walk_activations()) {
                CAPTURE(chart);
                for (size_t k = 0; k < act.skipped_fill_ticks.size(); ++k) {
                    const int64_t t = act.skipped_fill_ticks[k];
                    CHECK(t < act.timecode.ticks());
                    if (prev) CHECK(t > prev->timecode.ticks());
                    if (k > 0) CHECK(t > act.skipped_fill_ticks[k - 1]);
                    CHECK(std::any_of(song.sequence.begin(), song.sequence.end(),
                                      [t](const SongTimestamp& ts) {
                                          return ts.timecode.ticks() == t &&
                                                 ts.activation_length.has_value();
                                      }));
                }
                prev = &act;
            }
        }
    }
}
```

And "path codec: a node keeps skipped_fill_ticks" in `test_path_codec.cpp`, the same shape as D2's node case.

- [ ] **Step 2: Build and watch them fail.**

- [ ] **Step 3: Add the list beside the count, and prove them equal.** Field (model.h, after `bank_rise_ticks`):

```cpp
    // The fills the path was shown and passed over before this activation,
    // in chart order. The search charges a skip only on a fill it could have
    // taken (enough SP, deadline open) and stamps that fill's tick here.
    // Under the 1.0 fill rule these need not be the fills nearest the
    // activation. skips() is its size. The Preview lights exactly these.
    std::vector<int64_t> skipped_fill_ticks;
```

Engine: `int32_t skip_tail;` on `Act` and `Path`, pool `std::vector<ColNode> fills_;`, `int32_t skip_begin, skip_end;` on `OutAct`. In `branch_activate`, next to D2's line: `acts_[(size_t)c.act_tail].skip_tail = p.skip_tail;` and `c.skip_tail = -1;`; after `p.currentskips += 1;`:

```cpp
    // The fill just passed over, for the record: the Preview lights it.
    p.skip_tail = push_tick(fills_, p.skip_tail, n.tick);
```

`new_act`, `run`: `skip_tail = -1`. `emit_acts`: `emit_ticks(fills_, a.skip_tail, &oa.skip_begin, &oa.skip_end);`. `rebuild`: assign `act.skipped_fill_ticks` from that range. Codec: a u32 count and i64 ticks after `bank_rise_ticks`. Run the equality case; if it fails anywhere, stop and keep both under R2 with the reason.

- [ ] **Step 4: The count becomes an accessor.** Delete `int skips` (model.h:268); declare `int skips() const { return static_cast<int>(skipped_fill_ticks.size()); }`. Drop `i32 skips` from the codec (105, 144) and `skips` from `OutAct`. Fix readers the compiler names (`model.cpp:417, 443, 602`, `record_store.cpp:479`, `preview_view.cpp:345`, the two tools). Fixture writes become `test::set_skips(a, n)` (record_fixtures.h: n placeholder ticks just before the activation, "only the count matters"), except the Preview fill cases, which state real fill ticks (D5). Delete the equality case.

- [ ] **Step 5: Run everything and the score proof.**

- [ ] **Step 6: Commit.** Stage by name. Commit "Passed-over fills stored; skips is their count".

### Task 8: Transfer scales read the SP-end history

*Draft label: A3.*

**Goal:** `frontend_transfer_scales` measures `post` from D's anchor to D, and each SqIn's scale from its own end's anchor to its own end. Nothing steps back two measures.

**Findings:** 27, 28 (and the fixture half of 156 in `test_squeeze_rating.cpp`).

**Decision:** D1: "the transfer scale is measured from the collecting note (the stored `clamp_tick`), not the activation ... Finding 27's fix (the pre-SqIn SP end) is in the same function, so both go in together under one bump."

R1 stores every end the search used and the note that set it. This task makes the transfer scales read them. `transfer_pre` (one scale per activation, rebuilt from D) is replaced by one stored scale per SqIn. Several SqIns in one activation then become exact. None exists in the 105 `testdata` charts today: 0 of 158 SqIn activations among 1,611, cap 4, no ms filter, from `hydra_replay dump` at HEAD. But the engine can make one.

**Files:**
- Modify: `src/core/model.h` (`SPSqueeze::transfer` added; `Activation::transfer_pre` removed; `deact_anchor_tick` and `squeeze_anchor_tick` declared beside Task 5's accessors), `src/core/model.cpp` (their definitions).
- Modify: `src/core/squeeze_rating.h:34-70` (`ActTransferScales`, comments), `src/core/squeeze_rating.cpp:10-61` (`transfer_scale_between`, `frontend_transfer_scales`), and the plan's `rate_activation` SqIn loop.
- Modify: `src/search/engine.cpp:1266-1275` (stamping, after R1's step stamping).
- Modify: `src/store/path_codec.cpp:130-134, 171-175` (drop `transfer_pre`; a scale per SqIn).
- Modify: `src/app/path_view.cpp` (the plan's scale-line block).
- Test: `tests/test_model.cpp` (accessors), `tests/test_squeeze_rating.cpp:18-357` and the plan's new cases, `tests/test_path_view.cpp:289, 301, 308, 319, 327, 793, 846` and the plan's new case, `tests/test_search.cpp` (new cases after line 420; corpus case 174-257), `tests/test_store.cpp:140-160, 200-255`.

**Acceptance Criteria:**
- [ ] The accessors give the four SqIn-plus-clamp cases the anchors listed in Appendix B.
- [ ] Clamp song: `transfer_post` is 1.0 both ways (collecting note to D, both at 60 BPM). The old activation anchor gave 2.0.
- [ ] SqIn song: the SqIn's `transfer` is 1.0 both ways (2304 to 5376, both at 120 BPM). `transfer_post` is 2.0. The old rebuild gave the SqIn 2.0.
- [ ] A SqOut stores no `transfer`.
- [ ] `grep -n "transfer_pre\|scales.pre\|scales->pre\|sp_bars_to_measures(1)" src/core/squeeze_rating.cpp src/core/model.h` finds nothing.
- [ ] Paths and scores equal the R3 step-2 baseline. The full `hydra_tests` and `hydra_uitest --all --jobs 4` pass.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe; .\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` → `Status: SUCCESS!` and every UI test `[PASS]`. Then run the step-2 baseline compare → no differences.

**Steps:**

**Accessor names (reconciled when the plan was joined).** Task 5 already defines `squeeze_end_step(size_t)`, `squeeze_end_tick(size_t)` and `end_anchor_tick(size_t step)` on `Activation`. Part A's draft called the last one `anchor_of_step`; it is the same rule, and this plan uses Task 5's name. This task adds only the two accessors Task 5 lacks: `deact_anchor_tick()` and `squeeze_anchor_tick(size_t)`. Both are built on Task 5's accessors, so the anchor rule and the "which step was this squeeze measured from" rule each stay in one place.

- [ ] **Step 1: Failing accessor tests.** In `tests/test_model.cpp`:

```cpp
TEST_CASE("Activation: each end's anchor and each squeeze's end, from the steps") {
    using K = SpEndKind;
    Activation a;
    a.timecode = Timecode::raw(2304);

    // Case 3 of the SqIn-plus-clamp table: clamp at C1, SqIn, clamp at C2.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {3072, 6144, K::Clamped},     // C1 pins X
                      {6100, 7680, K::SqIn},        // early SqIn: X = 6144
                      {6912, 9984, K::Clamped}};    // C2 pins D
    a.sqinouts = {SPSqueeze{SqueezeKind::SqIn, -50.0}};
    CHECK(a.end_anchor_tick(0) == 2304);
    CHECK(a.end_anchor_tick(1) == 3072);
    CHECK(a.end_anchor_tick(3) == 6912);
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(6144));
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(3072));  // C1
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(6912));     // C2

    // Case 2: SqIn, then a clamp. The SqIn's end was the activation's.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},        // late SqIn: X = 5376
                      {6144, 12288, K::Clamped}};
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(5376));
    CHECK(a.squeeze_anchor_tick(0) == std::optional<int64_t>(2304));
    CHECK(a.deact_anchor_tick() == std::optional<int64_t>(6144));

    // A SqOut is measured from D, with D's anchor.
    a.sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, -20.0});
    CHECK(a.squeeze_end_tick(1) == a.deact_tick());
    CHECK(a.squeeze_anchor_tick(1) == a.deact_anchor_tick());

    // Two SqIns map to the two SqIn steps in order.
    a.sp_end_steps = {{2304, 5376, K::Activation},
                      {5400, 6912, K::SqIn},
                      {6950, 8448, K::SqIn}};
    a.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 24.0}, SPSqueeze{SqueezeKind::SqIn, 38.0}};
    CHECK(a.squeeze_end_tick(0) == std::optional<int64_t>(5376));
    CHECK(a.squeeze_end_tick(1) == std::optional<int64_t>(6912));

    // An old record (no steps) answers nothing.
    Activation old;
    old.sqinouts = {SPSqueeze{SqueezeKind::SqIn, 5.0}};
    CHECK_FALSE(old.squeeze_end_tick(0).has_value());
    CHECK_FALSE(old.deact_anchor_tick().has_value());
}
```

In `tests/test_search.cpp`, inside the anonymous namespace after `build_tail_song` (line 406), add these fixtures if R1's task has not already added equivalents:

```cpp
// build_tail_song with a tempo map: 192 ticks per beat, 768 per measure.
Song build_tempo_song(const std::vector<TailNote>& notes,
                      const std::map<int64_t, double>& bpm) {
    Song song(192);
    song.tpm_changes[0] = 768;
    for (const auto& kv : bpm) song.bpm_changes[kv.first] = kv.second;
    song.build_timing();
    for (const TailNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.sp_phrase;
        if (n.activation) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// Finding 27's shape. Two phrases bank 2 bars; the fill at 2304 activates,
// so SP ends at X = 2304 + 4 measures = 5376. The phrase at 5280 sits 250 ms
// before X: collecting it is the SqIn, and moves the end to 6912. The phrase
// at 6144 is then collected mid-SP and moves it to 8448. The tempo halves at
// 6000, so X (120 BPM) and D (60 BPM) differ, and the old "D minus two
// measures" rebuild (6912, 60 BPM) is wrong.
Song sqin_then_collect_song() {
    return build_tempo_song({{0, true},  {768, true},  {1536},  {2304, false, true},
                             {3072},     {3840},       {4608},  {5280, true},
                             {5376},     {6144, true}, {6912},  {7680},
                             {8448},     {8544}},
                            {{0, 120.0}, {6000, 60.0}});
}

// Finding 28's shape, at an SP cap of 2. The meter is full when the
// activation at 2304 starts, so collecting the phrase at 3072 clamps the end
// to plusmeasure(3072, 4) = 6144 instead of 6912. The tempo halves at 2700,
// between the activation and the collecting note.
Song clamp_song() {
    return build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                             {3072, true}, {3840}, {4608}, {5376}, {6144}, {6912}},
                            {{0, 120.0}, {2700, 60.0}});
}

// Keep every path, not only the best score (EngineOptions' default depth is
// 0), so a SqIn or SqOut branch the fixture creates is in the output even
// when it is not optimal.
EngineOptions wide_search() {
    EngineOptions o;
    o.depth_mode = DepthMode::Points;
    o.depth_value = 1000000;
    return o;
}

// The first activation, over every output path, that matches `pred`.
template <typename Pred>
const Activation* find_act(const std::vector<Path>& paths, Pred pred) {
    for (const Path& p : paths)
        for (const Activation& a : p.all_activations())
            if (pred(a)) return &a;
    return nullptr;
}
```

The fixture ticks were worked out by hand from `plusmeasure` on a flat 768-tick measure. They have not been run. If a `REQUIRE(act != nullptr)` fails, the fixture did not produce its shape: stop and report it. Do not loosen the check.

Then the engine-level cases:

```cpp
TEST_CASE("search: scales anchor on the collecting note and the SqIn's own end") {
    {
        Song song = clamp_song();
        ScoreGraph graph(song, 2);
        std::vector<Path> paths = run_search(graph, wide_search());
        const Activation* act = find_act(
            paths, [](const Activation& a) { return a.timecode.ticks() == 2304; });
        REQUIRE(act != nullptr);
        CHECK(act->clamp_tick() == std::optional<int64_t>(3072));
        CHECK(act->deact_tick() == std::optional<int64_t>(6144));
        // 3072 and 6144 are both in the 60 BPM section: x1.00, not x2.00.
        CHECK(act->transfer_post.late == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(act->transfer_post.early == doctest::Approx(1.0).epsilon(1e-12));
    }
    {
        Song song = sqin_then_collect_song();
        ScoreGraph graph(song, 4);
        std::vector<Path> paths = run_search(graph, wide_search());
        const Activation* act = find_act(paths, [](const Activation& a) {
            return !a.sqinouts.empty() && a.sqinouts.front().kind == SqueezeKind::SqIn;
        });
        REQUIRE(act != nullptr);
        CHECK(act->deact_tick() == std::optional<int64_t>(8448));
        CHECK(act->squeeze_end_tick(0) == std::optional<int64_t>(5376));   // X
        CHECK(act->squeeze_anchor_tick(0) == std::optional<int64_t>(2304));
        // 2304 -> 5376, both 120 BPM.
        CHECK(act->sqinouts.front().transfer.late == doctest::Approx(1.0).epsilon(1e-12));
        CHECK(act->sqinouts.front().transfer.early == doctest::Approx(1.0).epsilon(1e-12));
        // 2304 (120) -> 8448 (60): measures last twice as long at D.
        CHECK(act->transfer_post.late == doctest::Approx(2.0).epsilon(1e-12));
    }
}
```

In `tests/test_squeeze_rating.cpp`, replace `"frontend_transfer_scales: a SqIn splits the two ends"` (lines 81-119) with stored-step cases, using literal ticks only:

```cpp
TEST_CASE("frontend_transfer_scales: each SqIn reads its own stored end") {
    using K = SpEndKind;
    // Flat 4/4 at 480: a measure is 1920 ticks. The tempo changes at 9600.
    std::map<int64_t, int64_t> tpm{{0, 1920}};
    std::map<int64_t, double> bpm{{0, 120.0}, {9600, 150.0}};
    SongTiming st(480, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(0);
    act.sp_meter = 2;
    act.sp_end_steps = {{0, 7680, K::Activation}, {7600, 11520, K::SqIn}};
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 5.0});

    auto scales = frontend_transfer_scales(act, st);
    REQUIRE(scales.has_value());
    REQUIRE(scales->sqins.size() == 1);
    CHECK(scales->sqins[0].late == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(scales->sqins[0].early == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(scales->post.late == doctest::Approx(0.8).epsilon(1e-9));
    CHECK(scales->post.early == doctest::Approx(0.8).epsilon(1e-9));

    // Nothing is stepped back from D any more: a phrase collected after the
    // SqIn moves D, and the SqIn's scale does not change.
    act.sp_end_steps.push_back({13000, 15360, K::Collected});
    auto later = frontend_transfer_scales(act, st);
    REQUIRE(later.has_value());
    CHECK(later->sqins[0].late == doctest::Approx(1.0).epsilon(1e-12));

    // A SqIn with no SqIn step (an old record) is not guessed at.
    act.sp_end_steps = {{0, 11520, K::Activation}};
    CHECK_FALSE(frontend_transfer_scales(act, st).has_value());
}

TEST_CASE("frontend_transfer_scales: a clamp moves the front anchor (D1)") {
    using K = SpEndKind;
    // 120 BPM until 2700, then 60 (finding 28's worked example, at 192 tpq).
    std::map<int64_t, int64_t> tpm{{0, 768}};
    std::map<int64_t, double> bpm{{0, 120.0}, {2700, 60.0}};
    SongTiming st(192, tpm, bpm);

    Activation act;
    act.timecode = st.timecode(2304);
    act.sp_end_steps = {{2304, 6144, K::Activation}};
    auto unclamped = frontend_transfer_scales(act, st);
    REQUIRE(unclamped.has_value());
    CHECK(unclamped->post.late == doctest::Approx(2.0).epsilon(1e-12));

    act.sp_end_steps = {{2304, 5376, K::Activation}, {3072, 6144, K::Clamped}};
    auto clamped = frontend_transfer_scales(act, st);
    REQUIRE(clamped.has_value());
    CHECK(clamped->post.late == doctest::Approx(1.0).epsilon(1e-12));
    CHECK(clamped->post.early == doctest::Approx(1.0).epsilon(1e-12));
}
```

Rewrite the other `frontend_transfer_scales` cases at lines 18-79, 121-179, 181-257 and 280-357 on the same footing. Each fixture states its steps with literal ticks and drops its `plusmeasure` rebuild (finding 156). The ends are 21840 (line 34), 23520 (line 56, which is 21840 plus two 7/16 measures of 840), 21840 (line 66), 12960 (line 135), 9600 (line 160), 7680 (line 174) and 69120 (line 201). Where a case has no SqIn, its checks read `scales->post` instead of `scales->pre`. The SqIn sub-cases at lines 48-60 and 347-356 add a `SqIn` step and check `scales->sqins[0]`. The `plusmeasure` cross-checks at lines 37, 137 and 202 stay, because they pin the literal rather than rebuild it. If R1's task already converted `act.deact_tick = X` into steps, keep its conversion and only remove what is left of the rebuilds.

In the existing plan's new cases and in `tests/test_path_view.cpp`, a fixture that sets `act.transfer_pre = X` and adds a SqIn sets `sq.transfer = X` on that SqIn instead. A fixture that sets `transfer_pre = transfer_post` drops the line.

- [ ] **Step 2: Build and watch them fail** on the missing accessors, `scales->sqins` and `SPSqueeze::transfer`.

- [ ] **Step 3: The two new accessors.** Declare them in `model.h` beside Task 5's, with these comments, and define them in `model.cpp`. Task 5's `squeeze_end_step` already returns the step before a SqIn's own step for a SqIn, and the last step for the SqOut, so neither function repeats that rule.

```cpp
// The note whose timing moves the final SP end D: the latest Clamped step,
// else the activation (D1). Unset when the history is empty (old records).
std::optional<int64_t> Activation::deact_anchor_tick() const {
    if (sp_end_steps.empty()) return std::nullopt;
    return end_anchor_tick(sp_end_steps.size() - 1);
}

// The note whose timing moves the SP end squeeze k was measured from.
// Unset together with squeeze_end_tick(k).
std::optional<int64_t> Activation::squeeze_anchor_tick(size_t k) const {
    const std::optional<size_t> s = squeeze_end_step(k);
    if (!s) return std::nullopt;
    return end_anchor_tick(*s);
}
```

- [ ] **Step 4: The model.** Add to `SPSqueeze`:

```cpp
    // A SqIn's frontend transfer scale: at squeeze_end_tick, measured from
    // squeeze_anchor_tick. Stamped by the search at copy-out through
    // frontend_transfer_scales and stored, so the details view never needs a
    // SongTiming. A SqOut stores none: its row is rated at transfer_post.
    TransferScale transfer;
```

Delete `Activation::transfer_pre`. Rewrite the comment above `transfer_post` (model.h lines 304-313):

```cpp
    // The frontend transfer scale at the deact node D, measured from
    // deact_anchor_tick() (the activation, or the cap's collecting note).
    // Computed by the search at copy-out and stored, so the details view
    // never needs a SongTiming. Display-only: difficulty and everything the
    // search, filter and report derive stay raw gap ms. Each SqIn stores the
    // scale at its own end (SPSqueeze::transfer).
    TransferScale transfer_post;
```

- [ ] **Step 5: The function.** In `squeeze_rating.h`, replace `ActTransferScales` and its comment (lines 36-47):

```cpp
// An activation's transfer scales, all from stored ticks: `post` at the
// deact node, and one per SqIn (in sqinouts order) at the end that SqIn's
// offset was measured from. Nothing is stepped back from D.
struct ActTransferScales {
    TransferScale post;
    std::vector<TransferScale> sqins;
};
```

In `squeeze_rating.cpp`, rename `transfer_scale_between`'s first parameter to `anchor_tick`, and make it refuse any measure length that is not a positive finite number on either side:

```cpp
std::optional<TransferScale> transfer_scale_between(int64_t anchor_tick,
                                                    int64_t end_tick,
                                                    const SongTiming& timing) {
    const double front_late = timing.ms_per_measure_at(anchor_tick);
    const double front_early = timing.ms_per_measure_at(anchor_tick - 1);
    const double end_late = timing.ms_per_measure_at(end_tick);
    const double end_early = timing.ms_per_measure_at(end_tick - 1);
    for (double m : {front_late, front_early, end_late, end_early})
        if (!std::isfinite(m) || m <= 0.0) return std::nullopt;
    return TransferScale{end_early / front_early, end_late / front_late};
}

std::optional<ActTransferScales> frontend_transfer_scales(const Activation& act,
                                                          const SongTiming& timing) {
    const std::optional<int64_t> d = act.deact_tick();
    const std::optional<int64_t> d_anchor = act.deact_anchor_tick();
    if (!d || !d_anchor) return std::nullopt;
    const std::optional<TransferScale> post = transfer_scale_between(*d_anchor, *d, timing);
    if (!post) return std::nullopt;
    ActTransferScales out;
    out.post = *post;
    for (size_t k = 0; k < act.sqinouts.size(); ++k) {
        if (act.sqinouts[k].kind != SqueezeKind::SqIn) continue;
        const std::optional<int64_t> end = act.squeeze_end_tick(k);
        const std::optional<int64_t> anchor = act.squeeze_anchor_tick(k);
        if (!end || !anchor) return std::nullopt;
        const std::optional<TransferScale> s = transfer_scale_between(*anchor, *end, timing);
        if (!s) return std::nullopt;
        out.sqins.push_back(*s);
    }
    return out;
}
```

The end side now also refuses a zero, negative or NaN measure length. Today such a scale is stored silently.

Rewrite the header comment of `frontend_transfer_scales` (lines 63-68) to: "The activation's transfer scales, from the stored SP-end steps only: `post` from deact_anchor_tick() to deact_tick(), each SqIn from squeeze_anchor_tick(k) to squeeze_end_tick(k). The engine stamps the stored scales through this function at copy-out. nullopt when a tick is missing or a measure length is not a positive finite number." Update module comment line 17 to match.

- [ ] **Step 6: Copy-out.** Replace engine.cpp lines 1266-1275. It runs after R1's task stamps `sp_end_steps`:

```cpp
            // Stamp the frontend transfer scales through the one function
            // that computes them, from the SP-end steps just stamped.
            if (auto scales = frontend_transfer_scales(act, timing)) {
                act.transfer_post = scales->post;
                size_t j = 0;
                for (SPSqueeze& sq : act.sqinouts)
                    if (sq.kind == SqueezeKind::SqIn) sq.transfer = scales->sqins[j++];
            }
```

Task A4 replaces the silent fall-through with an explicit unknown.

- [ ] **Step 7: The codec.** Drop the two `transfer_pre` doubles from `write_activation` / `read_activation` (lines 131-132, 172-173). After each squeeze's `offset_ms`, a SqIn writes `w.f64(sq.transfer.early); w.f64(sq.transfer.late);`. A SqOut writes nothing more. Read in the same order. Do not touch `kPathFormatStamp`; the final task bumps it.

- [ ] **Step 8: The rating.** In the plan's `rate_activation`:

```cpp
    out.scales.post = act.transfer_post;
    out.scales.sqins.clear();
    for (const SPSqueeze& sq : act.sqinouts)
        if (sq.kind == SqueezeKind::SqIn) out.scales.sqins.push_back(sq.transfer);
```

The SqIn loop rates each SqIn against its own scale:

```cpp
    size_t j = 0;
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind == SqueezeKind::SqOut) {
            out.note_effective_ms.push_back(std::nullopt);
            continue;
        }
        const NoteRating n = rate_note(sq.offset_ms, core::paid_by_sp_walk(sq.offset_ms),
                                       out.scales.sqins[j++], hit_window_ms);
        out.scale_governs |= n.effective_ms.has_value();
        out.note_effective_ms.push_back(n.effective_ms);
    }
```

- [ ] **Step 9: The scale line.** In the plan's Task 2 Step 4 block in `path_view.cpp`, replace `const std::string pre_part = sides(rate.scales.pre);` and its clause. The new code adds one clause per SqIn whose scale prints differently from the SP end's. With one SqIn this prints exactly what the plan prints. The wording for two or more is Q5; with the recommended answer:

```cpp
        std::vector<std::string> sqin_parts;
        for (const TransferScale& s : rate.scales.sqins) {
            const std::string part = sides(s);
            if (!part.empty() && part != post_part) sqin_parts.push_back(part);
        }
        std::string clauses;
        if (!post_part.empty()) clauses = post_part + " at the SP end";
        for (size_t i = 0; i < sqin_parts.size(); ++i) {
            if (!clauses.empty()) clauses += "; ";
            clauses += sqin_parts[i] +
                       (sqin_parts.size() == 1
                            ? std::string(" at the SqIn's SP end")
                            : " at SqIn " + std::to_string(i + 1) + "'s SP end");
        }
```

- [ ] **Step 10: Corpus tests.** In `tests/test_search.cpp` (lines 198-217) and `tests/test_store.cpp` (lines 153-154, 238-243), compare `scales->post` with `act.transfer_post`, and each `scales->sqins[j]` with the j-th SqIn's `transfer`. Drop the `transfer_pre` comparisons. Also mend the broken comment at `test_search.cpp` lines 96-99, whose sentence was cut in half by the cases inserted after it: move it above `"stored transfer scales match the display-layer recomputation"` (line 174) and join it with the stray line 173.

- [ ] **Step 11: Run everything** (Verify above), including the baseline compare. A case that pinned an old rebuilt `pre` value is now wrong by design. Update it to the stored-end value, name it in the commit message, and tell the user.

- [ ] **Step 12: Commit** by name: `src/core/model.h src/core/model.cpp src/core/squeeze_rating.h src/core/squeeze_rating.cpp src/search/engine.cpp src/store/path_codec.cpp src/app/path_view.cpp tests/test_model.cpp tests/test_squeeze_rating.cpp tests/test_path_view.cpp tests/test_search.cpp tests/test_store.cpp` with the message "Transfer scales from the SP-end steps; the clamp note anchors (D1)".

### Task 9: "Unknown" is a value the record can hold

*Draft label: A4.*

**Goal:** When a scale can't be computed, the record says "unknown" and the details view says so, instead of showing x1.00.

**Findings:** 332.

**Decision:** D4: "The record may hold an explicit 'unknown' transfer scale instead of a silent x1.00, and the details view then shows a short 'transfer scale unknown' note and no eff. figures ... The 'unknown' value is a guard that shows a bug, not an expected state."

Today the stored scales are plain doubles that default to 1.0, so "could not compute" and "flat tempo" look the same. ADR 0011's "No fallback" section already says the scales should "come back unset". This task makes them optional, all or nothing per activation.

**Files:**
- Modify: `src/core/model.h` (`transfer_post` and `SPSqueeze::transfer` become `std::optional<TransferScale>`).
- Modify: `src/search/engine.cpp` (A3's stamping block, comment only).
- Modify: `src/store/path_codec.cpp` (a presence byte before each scale).
- Modify: `src/core/squeeze_rating.h` / `.cpp` (`ActivationRating::scales` becomes optional; new `stored_transfer_scales`).
- Modify: `src/app/path_view.cpp` (the note).
- Test: `tests/test_squeeze_rating.cpp`, `tests/test_path_view.cpp`, `tests/test_path_codec.cpp`.

**Acceptance Criteria:**
- [ ] An activation with no `transfer_post` rates with `scales` unset. It has no row figure, no SqIn figure, and `scale_governs` is false.
- [ ] Its details row shows `Transfer scale unknown.` and no `eff.` anywhere.
- [ ] A codec round trip keeps unknown as unknown, and a known x1.00 as a known x1.00.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*unknown*"` → `Status: SUCCESS!`, then the full run.

**Steps:**

- [ ] **Step 1: Failing tests.** In `tests/test_squeeze_rating.cpp`:

```cpp
TEST_CASE("rate_activation: an unknown scale rates nothing") {
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;
    // transfer_post left unknown, as copy-out writes it when it can't compute.
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 50.0});
    BackendSqueeze row;
    row.offset_ms = 40.0;
    act.backends.push_back(row);

    ActivationRating r = rate_activation(act, 85.0);
    CHECK_FALSE(r.scales.has_value());
    REQUIRE(r.backends.size() == 1);
    CHECK_FALSE(r.backends[0].note.effective_ms.has_value());
    REQUIRE(r.note_effective_ms.size() == 1);
    CHECK_FALSE(r.note_effective_ms[0].has_value());
    CHECK_FALSE(r.scale_governs);
}
```

In `tests/test_path_view.cpp`, next to the plan's near-1 case:

```cpp
TEST_CASE("build_activations: an unknown scale says so and shows no eff.") {
    HydraRecord rec;
    Activation act;
    act.skips = 0;
    act.e_offset = 300.0;
    BackendSqueeze row;
    row.offset_ms = -40.0;
    act.backends.push_back(row);
    Path p;
    p.activations.push_back(act);

    ActivationsView v = build_activations(p, rec, nullptr, 85.0);
    REQUIRE(v.acts.size() == 1);
    CHECK(v.acts[0].scale_warning == "Transfer scale unknown.");
    CHECK(v.acts[0].scale_warn);  // Q6: recommended orange
    REQUIRE(v.acts[0].backends.size() == 1);
    CHECK(v.acts[0].backends[0].rating.find("eff.") == std::string::npos);
}
```

In `tests/test_path_codec.cpp`:

```cpp
TEST_CASE("path codec: an unknown transfer scale stays unknown") {
    Path p;
    Activation unknown;
    Activation flat;
    flat.transfer_post = TransferScale{1.0, 1.0};
    SPSqueeze s{SqueezeKind::SqIn, 5.0};
    s.transfer = TransferScale{1.0, 1.0};
    flat.sqinouts.push_back(s);
    p.activations = {unknown, flat};
    Path back = store::decode_path_node(store::encode_path_node(p));
    CHECK_FALSE(back.activations[0].transfer_post.has_value());
    REQUIRE(back.activations[1].transfer_post.has_value());
    CHECK(back.activations[1].transfer_post->late == 1.0);
    REQUIRE(back.activations[1].sqinouts[0].transfer.has_value());
}
```

- [ ] **Step 2: Build and watch them fail.**

- [ ] **Step 3: Optional types.** `Activation::transfer_post` and `SPSqueeze::transfer` become `std::optional<TransferScale>`. Add to the `transfer_post` comment: "Unset means the search could not compute it. That is a bug guard, never an expected state (D4): a test proves no fresh record has it." A SqOut's `transfer` is always unset; it is not an unknown, because no reader asks a SqOut for one.

- [ ] **Step 4: Copy-out.** A3's block stays. With optional fields, "not computed" now leaves every scale unset instead of 1.0. Replace its comment with: "When the scales can't be computed, every one stays unknown, never a silent x1.00 (D4). No fresh record reaches this: see 'no fresh record stores an unknown transfer scale'."

- [ ] **Step 5: The codec.** Write `transfer_post` and each SqIn's `transfer` as a presence byte (`w.boolean(x.has_value())`), followed by the two doubles only when present. This is how the codec already writes `opt_f64` and `opt_i64`. A SqOut still writes no scale and no byte.

- [ ] **Step 6: The rating.** `ActivationRating::scales` becomes `std::optional<ActTransferScales>`. One helper reads the record, all or nothing:

```cpp
// The scales the search stored, or nothing when any one is unknown.
std::optional<ActTransferScales> stored_transfer_scales(const Activation& act) {
    if (!act.transfer_post) return std::nullopt;
    ActTransferScales s;
    s.post = *act.transfer_post;
    for (const SPSqueeze& sq : act.sqinouts) {
        if (sq.kind != SqueezeKind::SqIn) continue;
        if (!sq.transfer) return std::nullopt;
        s.sqins.push_back(*sq.transfer);
    }
    return s;
}
```

`rate_activation` sets `out.scales = stored_transfer_scales(act)`. The row loop calls `rate_note` only when `bsq.offset_ms && out.scales`. The SqIn loop pushes `std::nullopt` when `!out.scales`.

- [ ] **Step 7: The display.** At the top of the plan's scale-line block in `path_view.cpp`:

```cpp
        if (!rate.scales) {
            // D4: a guard that shows a bug. A test proves fresh records never
            // reach it.
            av.scale_warning = "Transfer scale unknown.";
            av.scale_warn = true;
        } else {
            // ... the plan's block, reading rate.scales->post and
            // rate.scales->sqins ...
        }
```

The row tooltip and the eff. suffix already depend on `br.note.effective_ms`, which is unset here.

- [ ] **Step 8: Run the full `hydra_tests` and `hydra_uitest --all --jobs 4`.** Change compile sites that read `.transfer_post.late` to `->late` after a `REQUIRE(...has_value())`.

- [ ] **Step 9: Commit** by name: "Unknown transfer scale is explicit (D4)".

### Task 10: Prove no fresh record stores "unknown"

*Draft label: A7.*

**Goal:** A test fails if any freshly analyzed activation lacks a scale or any SP-end fact the scales are read from.

**Findings:** 332.

**Decision:** D4: "A test must prove that no fresh record stores 'unknown'."

This turns D4's condition into a test. The test runs the engine on hand-built songs that cover every shape that sets a scale. Then it repeats the check over every `testdata` chart through a real store round trip. Two ways of reaching "unknown" are not parser questions, and the test covers both. The first is an empty step list. That needs an activation with no deactivation edge and no tracked SP end, which cannot happen today: `branch_activate` refuses a `NO_TIME` end (engine.cpp 556-559), and advance and the SqIn branch only replace it with real ticks. The second is a SqIn with no SqIn step. That cannot happen if R1 keeps guarantee 1. Part B's fold change (D3) must keep both true.

**Files:**
- Modify: `tests/corpus_util.h` (one shared check, so both test files read one copy).
- Test: `tests/test_search.cpp` (new case after A3's; corpus case lines 174-257), `tests/test_store.cpp:200-255`.

**Acceptance Criteria:**
- [ ] The constructed-songs case visits at least one SqIn, one SqOut and one clamp, and finds no unknown.
- [ ] The corpus case asserts the following for each activation. `deact_tick()`, `nominal_end()` and `transfer_post` are set. Every SqIn has `squeeze_end_tick`, `squeeze_anchor_tick` and `transfer`. Every scale is finite and above 0.
- [ ] The store round trip asserts the same after reading back.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="*unknown transfer scale*,stored transfer scales*"` → `Status: SUCCESS!`, with nonzero activation counts in the MESSAGE lines.

**Steps:**

- [ ] **Step 1: The check, once.** In `tests/corpus_util.h`:

```cpp
// Empty when the activation carries every stored fact its transfer scales
// need; otherwise what is missing. D4: nothing here may be missing on a fresh
// record.
inline std::string unknown_scale_reason(const hydra::Activation& a) {
    auto good = [](const hydra::TransferScale& s) {
        return std::isfinite(s.early) && std::isfinite(s.late) && s.early > 0.0 &&
               s.late > 0.0;
    };
    if (!a.deact_tick()) return "no SP-end steps";
    if (!a.nominal_end()) return "no nominal end";
    if (!a.transfer_post) return "transfer_post unknown";
    if (!good(*a.transfer_post)) return "transfer_post not a positive finite number";
    for (size_t k = 0; k < a.sqinouts.size(); ++k) {
        if (a.sqinouts[k].kind != hydra::SqueezeKind::SqIn) continue;
        if (!a.squeeze_end_tick(k) || !a.squeeze_anchor_tick(k))
            return "SqIn without its SqIn step";
        if (!a.sqinouts[k].transfer) return "SqIn transfer unknown";
        if (!good(*a.sqinouts[k].transfer)) return "SqIn transfer not a positive finite number";
    }
    return {};
}
```

- [ ] **Step 2: Constructed songs.** In `tests/test_search.cpp`:

```cpp
TEST_CASE("search: no fresh record stores an unknown transfer scale (constructed songs)") {
    struct Case { const char* name; Song song; int cap; };
    std::vector<Case> cases;
    cases.push_back({"SP past the last note",
                     build_tail_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                      {3072}, {3840}, {4608}, {5136}, {5280}}), 4});
    cases.push_back({"mid-SP phrase, SP past the end",
                     build_tail_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                      {3072}, {3840, true}, {4608}, {5376}, {6144},
                                      {6720}, {6816}}), 4});
    cases.push_back({"SqIn then a collection", sqin_then_collect_song(), 4});
    cases.push_back({"cap clamp", clamp_song(), 2});
    cases.push_back({"tempo change on the activation tick",
                     build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                                       {3072}, {3840}, {4608}, {5376}, {6144}},
                                      {{0, 98.0}, {2304, 97.5}, {5376, 110.0}}), 4});

    int acts = 0, sqins = 0, sqouts = 0, clamps = 0;
    for (const Case& c : cases) {
        ScoreGraph graph(c.song, c.cap);
        for (const Path& p : run_search(graph, wide_search())) {
            for (const Activation& a : p.all_activations()) {
                ++acts;
                if (a.clamp_tick()) ++clamps;
                for (const SPSqueeze& s : a.sqinouts)
                    (s.kind == SqueezeKind::SqIn ? sqins : sqouts) += 1;
                const std::string why = corpus::unknown_scale_reason(a);
                CHECK_MESSAGE(why.empty(), c.name << ": activation at tick "
                                                  << a.timecode.ticks() << ": " << why);
            }
        }
    }
    // The cases really cover the shapes they claim.
    CHECK(acts > 0);
    CHECK(sqins > 0);
    CHECK(sqouts > 0);
    CHECK(clamps > 0);
    MESSAGE("checked " << acts << " activations");
}
```

- [ ] **Step 3: Corpus.** In `"stored transfer scales match the display-layer recomputation"` (line 174), replace the "missing deact_tick" and "non-positive" blocks (lines 213-225) with:

```cpp
                if (const std::string why = corpus::unknown_scale_reason(act); !why.empty()) {
                    d = why;
                    break;
                }
```

Add the same check to every read-back activation in `tests/test_store.cpp`'s round-trip case (line 200).

- [ ] **Step 4: Run them.** They should pass with nonzero counts. If one fails, that is a real source D4 says must be closed. Stop and report it; do not relax the check.

- [ ] **Step 5: Commit** `tests/corpus_util.h tests/test_search.cpp tests/test_store.cpp` by name: "Prove no fresh record stores an unknown transfer scale (D4)".

### Task 11: One window check and one offset rule

*Draft label: C2.*

Six places decide "is this note close enough to the SP end?". Four compare against `kSqueezeWindowMs` in slightly different forms, and two type 500 by hand. Tail rows are the one place they disagree. The graph keeps the song's trailing notes within 500 ms of the last note. The engine copies all of them into the activation. Only the display then drops the ones 500 ms or more from the real SP end. D5 says to measure tail rows from the SP end, like every other row, with one shared check.

The fix is two tiny functions beside `kSqueezeWindowMs`. One says how far a note sits from an SP end. The other says whether that distance is inside the window. Every place calls them, and the engine filters tail rows when it copies them. The stored record does not change, because the codec already wrote only `display_backends()`, which applied the same rule. Finding 149 (the offset subtraction written in five places) falls out of the same edits, so it is folded in here.

One caveat: the graph's two checks are signed today ("head minus note"), and the new check uses the absolute distance. They give the same answer whenever time rises with tick. The load guard (A5, R3 step 1) stops any timing that makes a measure or beat zero, negative or infinite, so time always rises with tick by the time this task runs. A5 is this part's prerequisite.

HEAD line numbers below are from 02976c1. By R3 step 6, the SP-end history task has edited `rebuild` (it assigns `sp_end_steps` near HEAD line 1264) and `path_codec.cpp`, and the transfer-scale tasks have edited `squeeze_rating.cpp`. The functions named here keep their shape, so locate each edit by its function name and the quoted code.

**Goal:** One `within_squeeze_window` and one `offset_from_sp_end` in `core/model.h`, used by the graph, the engine's tail rows, `display_backends`, and `hydra_replay`; the Backend limit box and the help text print the constant.
**Findings:** 30, R7.10, 149
**Decision:** D5: "For an activation whose SP outlasts the song, the 500 ms backend window is measured from the SP end, the same as every other activation. ... The engine's tail collection and the display share one 'inside the squeeze window' check next to `kSqueezeWindowMs`. Nothing on screen or in stored records changes, because the display already filtered by this rule before saving."
**Files:**
- Modify: `src/core/model.h` (constants block, lines 71-75; add `#include <cmath>` near line 15)
- Modify: `src/core/model.cpp` (`display_backends`, lines 579-596)
- Modify: `src/search/graph.cpp` (SqOut check line 131; `store_new_backend` lines 234-239; `is_recent_to_head` lines 280-282; `add_deact_edge` lines 353-358)
- Modify: `src/search/graph.h` (remove `head_time_offset`, lines 156-158; comment on `tail_backends`, lines 121-124)
- Modify: `src/search/engine.cpp` (`rebuild` tail rows, lines 1196-1206)
- Modify: `src/core/replay.cpp` (`sqout_candidates` line 34; `replay_path` offset lines 149-151; `resolve_sqout_note` lines 248-249 and 267, 282)
- Modify: `src/ui/paths_tab.cpp` (clamp, line 503)
- Modify: `tools/replay.cpp` (help text line 112 and the `printf` close at line 135)
- Test: `tests/test_search.cpp` (after line 554), `tests/test_squeeze_rating.cpp` (lines 689-718 and a new case after it)

**Acceptance Criteria:**
- [ ] The tail fixture's last activation holds only the row at tick 5280 in memory, not the graph's row at 5136.
- [ ] On every corpus chart, each tail activation's rows equal what the old display filter kept, and each deact-edge activation's rows are all inside the window or the squeezed-out row.
- [ ] A source scan finds no comparison against `kSqueezeWindowMs` outside `core/model.h`, and no typed 500 in the clamp or the help text.
- [ ] The test at `tests/test_squeeze_rating.cpp` line 689 keeps its name (it was false for tail rows; now it is true) and gains the exact-500 boundary.
- [ ] Every existing test passes.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe --test-case="*tail rows*,*squeeze window*,display_backends*,SP past the last note*"` reports all passed, then `.\build-cpp\Release\hydra_tests.exe` reports 0 failed.

**Steps:**
- [ ] Step 0: Confirm the starting point. Rerun C1's printer on the code as it stands at R3 step 6, and compare it with `hydra-baseline-squeeze-facts.txt` using the Compare-Object line from Task C8. It must print nothing. Steps 3-5 promise not to move any printed fact, and their own Verify steps check the same baseline. If anything differs, stop: Part C would otherwise inherit someone else's change.
- [ ] Step 1: Write the failing tests. In `tests/test_search.cpp`, after line 554:

```cpp
// D5: a tail row is kept on the same window as every other row, measured
// from this activation's own SP end. The graph keeps 5136 (375 ms before the
// last note), but it sits 625 ms before the SP end at 5376, so the engine
// must not copy it into the activation.
TEST_CASE("SP past the last note: tail rows use the 500 ms window from the SP end") {
    Song song = build_tail_song({{0, true, false}, {768, true, false}, {1536},
                                 {2304, false, true}, {3072}, {3840}, {4608},
                                 {5136}, {5280}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> paths = run_search(graph, EngineOptions{});
    const Activation& act = last_act(paths);

    bool graph_kept_5136 = false;
    for (const BackendSqueeze& b : graph.tail_backends())
        if (b.timecode.ticks() == 5136) graph_kept_5136 = true;
    REQUIRE(graph_kept_5136);

    REQUIRE(act.backends.size() == 1);
    CHECK(act.backends[0].timecode.ticks() == 5280);
    // Nothing is left for the display to drop.
    CHECK(act.display_backends() == act.backends);
}

// D5 moves where tail rows are filtered, not which rows survive. The old
// rule is written out here: every graph tail note, measured from the SP end,
// kept when it is strictly less than 500 ms away.
TEST_CASE("tail rows: every corpus activation keeps exactly the rows the old display kept") {
    int tails = 0, edges = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4);
        const int64_t last_tick = song.sequence.back().timecode.ticks();
        for (const Path& p : run_search(graph, EngineOptions{DepthMode::Scores, 4})) {
            for (const Activation& act : p.all_activations()) {
                // R1: the SP end is the last SP-end step's end_tick.
                const std::optional<int64_t> end_tick = act.deact_tick();
                if (!end_tick) continue;
                if (*end_tick <= last_tick) {
                    // A deactivation edge: its rows were gathered inside the window.
                    ++edges;
                    for (const BackendSqueeze& b : act.backends)
                        CHECK((std::fabs(*b.offset_ms) < 500.0 || act.is_sqout_backend(b)));
                    continue;
                }
                ++tails;
                const double end_ms = song.timing().ms_index().at(*end_tick);
                std::vector<BackendSqueeze> want;
                for (const BackendSqueeze& b : graph.tail_backends()) {
                    BackendSqueeze copy = b;
                    copy.offset_ms = b.timecode.ms() - end_ms;
                    if (std::fabs(*copy.offset_ms) < 500.0) want.push_back(copy);
                }
                CHECK(act.display_backends() == want);  // what is stored and shown: unchanged
                CHECK(act.backends == want);            // and now nothing extra in memory
            }
        }
    }
    CHECK(edges > 0);
    CHECK(tails > 0);
}
```

In `tests/test_squeeze_rating.cpp`, extend the case at line 689 with the boundary, and add the source scan after it. The scan follows the pattern of the `win32_path` scan in `tests/test_long_paths.cpp` (lines 272-330); add `<filesystem>`, `<fstream>` and `<regex>` to the includes.

```cpp
    // Exactly 500 ms away is outside, on both sides.
    Activation edge;
    for (double off : {-kSqueezeWindowMs, kSqueezeWindowMs}) {
        BackendSqueeze row;
        row.timecode = st.timecode(0);
        row.offset_ms = off;
        edge.backends.push_back(row);
    }
    CHECK(edge.display_backends().empty());
    CHECK(within_squeeze_window(kSqueezeWindowMs - 0.001));
    CHECK_FALSE(within_squeeze_window(-kSqueezeWindowMs));
    CHECK(offset_from_sp_end(1000.0, 1250.0) == -250.0);
}

// The one-place rule, checked: only core/model.h compares against the
// squeeze window, and nothing types its value by hand.
TEST_CASE("the squeeze window is compared in one place and never typed as 500") {
    namespace fs = std::filesystem;
    const fs::path root = fs::u8path(HYDRA_SOURCE_DIR);
    const std::regex compare(R"([<>]=?\s*kSqueezeWindowMs|kSqueezeWindowMs\s*[<>])");
    std::vector<std::string> problems;
    for (const char* sub : {"src", "tools"}) {
        for (const fs::directory_entry& e : fs::recursive_directory_iterator(root / sub)) {
            const fs::path ext = e.path().extension();
            if (ext != ".cpp" && ext != ".h") continue;
            const std::string rel = fs::relative(e.path(), root).generic_u8string();
            std::ifstream in(e.path());
            std::string line;
            int lineno = 0;
            while (std::getline(in, line)) {
                ++lineno;
                const std::string where = rel + ":" + std::to_string(lineno) + ": ";
                if (rel != "src/core/model.h" && std::regex_search(line, compare))
                    problems.push_back(where + line);
                // Only the Backend limit clamp: the Path limit's own -500..500
                // clamp (settings_bar.cpp line 144) is a different setting.
                if (line.find("backendlimit_value, 0, 500") != std::string::npos ||
                    line.find("within 500 ms") != std::string::npos)
                    problems.push_back(where + line);
            }
        }
    }
    INFO(problems.size() << " problem lines; first: "
                         << (problems.empty() ? std::string() : problems.front()));
    CHECK(problems.empty());
}
```

- [ ] Step 2: Run the tests. The tail fixture fails (5136 is still in memory), `act.backends == want` fails on the corpus, and the scan names `graph.cpp:131`, `graph.cpp:281`, `model.cpp:591`, `replay.cpp:34`, `paths_tab.cpp:503` and `tools/replay.cpp:112`. The new functions do not compile yet.
- [ ] Step 3: Add the two functions in `src/core/model.h`, right after `kSqueezeWindowMs` (line 75), and `#include <cmath>`:

```cpp
// How far a note sits from a Star Power end, in ms: negative before it,
// positive after. Every backend row's offset_ms is this number.
inline double offset_from_sp_end(double note_ms, double sp_end_ms) {
    return note_ms - sp_end_ms;
}

// Is a note close enough to a Star Power end to matter for a squeeze?
// Strictly inside kSqueezeWindowMs on either side; exactly 500 ms away is
// out. The one window check: the search graph, the engine's tail rows, the
// stored rows (Activation::display_backends) and hydra_replay all ask it.
inline bool within_squeeze_window(double offset_from_sp_end_ms) {
    return std::fabs(offset_from_sp_end_ms) < kSqueezeWindowMs;
}
```

- [ ] Step 4: Call them everywhere. In `graph.cpp` line 131: `if (within_squeeze_window(offset_from_sp_end(timestamp.timecode.ms(), kv.second.ms())))`. In `is_recent_to_head` (line 281): `return within_squeeze_window(offset_from_sp_end(tc.ms(), head_time_.ms()));` with a comment that the distance is symmetric, so it serves both an edge's end and a note. Delete `head_time_offset` from `graph.h`. In `store_new_backend` and `add_deact_edge`, write the offset as `offset_from_sp_end(note.ms(), dest.ms())`. In `model.cpp` line 591: `if (within_squeeze_window(bsq.offset_ms.value_or(0.0)) || is_sqout_backend(bsq))`. In `replay.cpp` line 34: `if (ts.flag_sp && within_squeeze_window(offset_from_sp_end(ts.timecode.ms(), d_ms)))`, and use `offset_from_sp_end` in `replay_path` (lines 150-151, keeping its clamp to 0 for chords on or before D) and in `resolve_sqout_note`.
- [ ] Step 5: Filter tail rows in `engine.cpp` `rebuild` (lines 1201-1206):

```cpp
                const double end_ms = timing.ms_index().at(oa.final_sp_end);
                for (const BackendSqueeze& b : tail_backends) {
                    const double off = offset_from_sp_end(b.timecode.ms(), end_ms);
                    // D5: the same window as every other row, from this
                    // activation's own SP end, not from the song's last note.
                    if (!within_squeeze_window(off)) continue;
                    BackendSqueeze copy = b;
                    copy.offset_ms = off;
                    act.backends.push_back(copy);
                }
```

Update the `tail_backends()` comment in `graph.h` (lines 121-124) to say the engine narrows these to the window around each activation's own SP end.
- [ ] Step 6: Print the constant. `paths_tab.cpp` line 503: `std::clamp(app.settings.backendlimit_value, 0, static_cast<int>(kSqueezeWindowMs))`. `tools/replay.cpp` line 112: replace "within 500 ms" with "within %.0f ms", and close the `printf` at line 135 with `..., kSqueezeWindowMs);`. The string has no other conversions; `%%TEMP%%` is already escaped.
- [ ] Step 7: Run Verify. Commit, staging each file by name.

### Task 12: One rule for which chord an SP end can squeeze out

*Draft label: C3.*

The graph decides which phrase chord a deactivation can squeeze out, but in two halves. `add_deact_edge` takes the first phrase chord at or before the end, and `store_new_backend` takes the first one after it, if there was none before. `hydra_replay` re-derives the same choice with its own scan in `sqout_candidates` (finding 145). They agree today only because the halves happen to line up with chart order.

The fix is one function, `core::sqout_chord`, that returns the first phrase chord in chart order inside the window around an SP end. The graph calls it once per deactivation edge, and the replay calls it to resolve a typed offset and to word its warning. The proof is in the order of the steps. The test that compares every graph edge's claimed chord with `sqout_chord` is written and run while the graph still uses its old two-half logic. Passing there proves the new function gives the old answer on the whole corpus. The graph is then switched over, and the same test must stay green.

The function lives in a new header in `core`, because the replay (in `core`) may not include `search`, and it needs the chart.

**Goal:** `core::sqout_chord(song, sp_end)` is the one owner of the claimed chord; the graph and `hydra_replay` both call it.
**Findings:** 145 (and the graph half of 29)
**Decision:** none needed (no result changes; the corpus test proves it)
**Files:**
- Create: `src/core/sqout_chord.h` (header-only)
- Modify: `src/search/graph.cpp` (`store_new_backend` lines 241-247 deleted; `add_deact_edge` lines 346-370)
- Modify: `src/core/replay.cpp` (comment lines 25-28; `resolve_sqout_note` lines 269-271; `ambiguous_window_warnings` lines 298-302)
- Modify: `src/core/replay.h` (line 66: "user decision 23" becomes "plan decision 20 of 2026-09-24")
- Test: `tests/test_replay.cpp` (new case after line 617; comment at line 560), `tests/test_search.cpp` (new case after Task C2's)

**Acceptance Criteria:**
- [ ] `sqout_chord` returns the first phrase chord strictly inside the window, in chart order, or nothing.
- [ ] On every corpus chart, every deactivation edge claims exactly `sqout_chord`'s chord, with the same offset bit for bit, before and after the graph is switched.
- [ ] The existing refusal and warning tests (`tests/test_replay.cpp` lines 525-617) pass unchanged.
- [ ] `hydra_replay selfcheck` passes.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe --test-case="sqout_chord*,graph: every deactivation edge*,a typed squeeze-out*,the squeeze-out warning*"` all pass; `.\build_cpp.ps1 -Target hydra_replay; .\build-cpp\Release\hydra_replay.exe selfcheck` reports no FAIL lines.

**Steps:**
- [ ] Step 1: Write the failing tests. In `tests/test_replay.cpp` after line 617 (it uses `song_with` from lines 484-498):

```cpp
// The one rule for which phrase chord an SP end can squeeze out.
TEST_CASE("sqout_chord: the first phrase chord strictly inside the window, in chart order") {
    const Song two = song_with({{0, false}, {768, false}, {2928, true},
                                {3036, true}, {3072, false}});
    const SongTimestamp* c = core::sqout_chord(two, two.timecode(3072));
    REQUIRE(c != nullptr);
    CHECK(c->timecode.ticks() == 2928);  // 375 ms before D comes before 93.75 ms

    // Exactly 500 ms before D is outside.
    const Song edge = song_with({{0, false}, {2880, true}, {3072, false}});
    CHECK(core::sqout_chord(edge, edge.timecode(3072)) == nullptr);

    // Only a phrase chord after D: that one.
    const Song after = song_with({{0, false}, {3072, false}, {3073, true}});
    REQUIRE(core::sqout_chord(after, after.timecode(3072)) != nullptr);
    CHECK(core::sqout_chord(after, after.timecode(3072))->timecode.ticks() == 3073);

    // An SP end between two chords, and no phrase chord at all.
    CHECK(core::sqout_chord(two, two.timecode(3000))->timecode.ticks() == 2928);
    const Song bare = song_with({{0, false}, {3072, false}});
    CHECK(core::sqout_chord(bare, bare.timecode(3072)) == nullptr);
}
```

In `tests/test_search.cpp` (it already includes `search/graph.h`; add `"core/sqout_chord.h"` and `<set>`):

```cpp
// The graph claims, for every deactivation edge, exactly the chord
// core::sqout_chord names. Run before the graph calls it, this proves the
// function is the graph's old rule; after, it guards against drift.
TEST_CASE("graph: every deactivation edge claims the chord sqout_chord names") {
    int edges = 0, claimed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        const ScoreGraph graph(song, 4);

        // The SP track is one chain; it starts at the first activation's node.
        const ScoreGraphNode* sp = nullptr;
        for (const ScoreGraphNode* b = graph.start(); b && !sp;
             b = b->adv_edge ? b->adv_edge->dest : nullptr)
            if (b->branch_edge) sp = b->branch_edge->dest;

        std::set<const ScoreGraphEdge*> seen;
        for (; sp; sp = sp->adv_edge ? sp->adv_edge->dest : nullptr) {
            const ScoreGraphEdge* e = sp->branch_edge;
            if (!e || !seen.insert(e).second) continue;
            ++edges;
            const Timecode& end = e->dest->timecode;
            const SongTimestamp* c = core::sqout_chord(song, end);
            CHECK(e->sqinout_time.has_value() == (c != nullptr));
            if (!c || !e->sqinout_time) continue;
            ++claimed;
            CHECK(e->sqinout_time->ticks() == c->timecode.ticks());
            CHECK(*e->sqinout_timing == c->timecode.ms() - end.ms());  // bit for bit
        }
    }
    CHECK(edges > 0);
    CHECK(claimed > 0);
}
```

- [ ] Step 2: Run them. Both fail to compile: `core/sqout_chord.h` does not exist.
- [ ] Step 3: Create `src/core/sqout_chord.h`:

```cpp
// Which phrase chord a Star Power end can squeeze out: one rule, asked by
// the search graph (it claims this chord for its deactivation edge) and by
// hydra_replay (it resolves a typed offset and words its warning with it).

#ifndef HYDRA_CORE_SQOUT_CHORD_H
#define HYDRA_CORE_SQOUT_CHORD_H

#include <algorithm>
#include <iterator>

#include "core/model.h"
#include "parse/song.h"

namespace hydra::core {

// The first SP phrase chord, in chart order, strictly inside the squeeze
// window around `sp_end`. nullptr when there is none. Chords are in tick
// order and time rises with tick, so the window is one run of chords: find
// the first chord at or after the end, step back to the run's start, then
// take the run's first phrase chord.
inline const SongTimestamp* sqout_chord(const Song& song, const Timecode& sp_end) {
    const std::vector<SongTimestamp>& seq = song.sequence;
    auto inside = [&](const SongTimestamp& ts) {
        return within_squeeze_window(offset_from_sp_end(ts.timecode.ms(), sp_end.ms()));
    };
    auto it = std::lower_bound(seq.begin(), seq.end(), sp_end.ticks(),
                               [](const SongTimestamp& ts, int64_t t) {
                                   return ts.timecode.ticks() < t;
                               });
    while (it != seq.begin() && inside(*std::prev(it))) --it;
    for (; it != seq.end() && inside(*it); ++it)
        if (it->flag_sp) return &*it;
    return nullptr;
}

}  // namespace hydra::core

#endif  // HYDRA_CORE_SQOUT_CHORD_H
```

- [ ] Step 4: Run the tests. Both now pass while the graph still uses its old logic. This run is the proof that the function is the graph's rule. Save its output line in the commit message.
- [ ] Step 5: Switch the graph. Delete the claim in `store_new_backend` (lines 241-247), so it only copies rows. Make `add_deact_edge` claim once:

```cpp
void ScoreGraph::add_deact_edge() {
    ScoreGraphEdge* deact_edge = new_edge();
    deact_edge->dest = base_track_head_;
    const Timecode& end = deact_edge->dest->timecode;

    deact_edge->sqout_time = end;
    deact_edge->sqin_time = end;

    for (const BackendSqueeze& recent_backend : recent_backends_) {
        BackendSqueeze copy = recent_backend;
        copy.offset_ms = offset_from_sp_end(recent_backend.timecode.ms(), end.ms());
        deact_edge->backends.push_back(copy);
    }

    // The one phrase chord this SP end can squeeze out (core/sqout_chord.h).
    // On or before the end, it moves both branches' ends one bar. After the
    // end, only a late SqIn reaches it, so only the SqIn end moves.
    if (const SongTimestamp* c = core::sqout_chord(song_, end)) {
        deact_edge->sqinout_time = c->timecode;
        deact_edge->sqinout_timing = offset_from_sp_end(c->timecode.ms(), end.ms());
        deact_edge->sqin_time = plusmeasure(end, sp_bars_to_measures(1));
        if (c->timecode.ticks() <= end.ticks())
            deact_edge->sqout_time = plusmeasure(end, sp_bars_to_measures(1));
        else
            deact_edge->late_sqin_count = 1;
    }

    recent_deact_edges_.push_back(deact_edge);
    sp_track_head_->branch_edge = deact_edge;
}
```

This sets the same five fields the two old halves set. The old after-the-end half also only ever ran once per edge, so `late_sqin_count` was never more than 1.
- [ ] Step 6: Switch the replay. In `resolve_sqout_note`, the engine's chord is `core::sqout_chord(song, song.timecode(w.deact_tick))` instead of `cands.front()`. `sqout_candidates` stays only to find the chord nearest a typed offset, which is the replay's own question. In `ambiguous_window_warnings`, take the chord from `core::sqout_chord` and drop the candidate list. Fix the comment at `replay.cpp` lines 25-28 and the mis-citation at `replay.h` line 66 and `tests/test_replay.cpp` line 560.
- [ ] Step 7: Run Verify and the full `hydra_tests`. Commit by name.

### Task 13: The record stores the squeeze-out once

*Draft label: C4.*

Two facts are stored twice (findings 127 and 128). "Did this activation squeeze out?" is stored as `sqout_tick` and again as a SqOut entry in the squeeze list. "By how many ms?" is stored as the SqOut entry's offset and again as the squeezed-out row's `offset_ms`. They come from one number in the engine today, so they agree. ADR 0014 already makes `sqout_tick` the record's truth.

The fix has one writer and one reader. `Activation::set_sqout(tick)` is the only thing that marks a squeeze-out. It stamps the tick, drops the rows past it, and builds the SqOut entry from that row's own offset. The engine's copy-out and the codec's decoder both call it. `Activation::sqout_row()` is the one accessor readers use to ask "which row, and how far?". The codec stops writing the SqOut entry. It stores the tick, and the offset already rides in the row.

This changes the stored layout, so the plan's final task bumps `kPathFormatStamp` for it. The in-memory list keeps its SqOut entry, so the path string ("-"), `difficulty()`, the SqOut sentence and the report read it exactly as before. The `squeeze_sentences` reader (finding 127) needs no change: its two numbers are now built from one stored value by one function, so they cannot split.

`sqout_tick` stays a stored field. R1's SP-end history does not hold it, because a squeezed-out phrase has no step (R1). It is the one stored answer to "which chord was squeezed out". The SP end it was measured from is `deact_tick()`, the last step's `end_tick`.

**Codec order against the SP-end history task.** Both tasks edit `write_activation` and `read_activation` in `src/store/path_codec.cpp`. The history task (R3 step 3) lands first. It removes the stored `deact_tick` and `clamp_tick` (HEAD lines 135-136 and 176-177) and the `collected_phrase_ticks` list (HEAD lines 138-139 and 179-181), and it writes `sp_end_steps` at the end of the activation. The bank and fill tasks (R3 step 4) then add their lists after it. C4 runs at step 6 and rebases on all of that. Its edits touch only two places. One is the squeeze-list block (HEAD lines 124-128 and 163-169), which none of those tasks edits. The other is the line that reads `sqout_tick` (HEAD lines 137 and 178), which the history task keeps. In `read_activation`, the `set_sqout` call goes after the last field is read, the end of the function, so the rows, `sqout_tick` and the steps are all in place. `set_sqout` reads only `backends` and `sqout_tick`, never the steps. Removing `is_sp` in C5 touches only the row block (HEAD lines 120 and 158), which no other task edits. Part B (R3 step 7) comes after C and rebases on these edits.

**Goal:** `sqout_tick` plus the row's `offset_ms` are the only stored form of a squeeze-out; `set_sqout` writes it and `sqout_row` reads it.
**Findings:** 127, 128
**Decision:** none needed (ADR 0014 already names `sqout_tick` as the truth; no visible change)
**Files:**
- Modify: `src/core/model.h` (`Activation`, lines 290-333)
- Modify: `src/core/model.cpp` (next to `is_sqout_backend`, lines 573-577)
- Modify: `src/search/engine.cpp` (`rebuild`, lines 1208-1241)
- Modify: `src/store/path_codec.cpp` (`write_activation` lines 124-128, `read_activation` lines 163-178)
- Modify: `src/core/replay.cpp` (`windows_for_path`, lines 213-230), `src/core/replay.h` (comment lines 200-208)
- Test: `tests/test_search.cpp`, `tests/test_model.cpp`, `tests/test_path_codec.cpp`
- Hand-off (not edited here): the ADR 0014 amendment text, for the final task (R3 step 9)

**Acceptance Criteria:**
- [ ] On every corpus path, an activation has at most one SqOut entry, it is last, `sqout_tick` is set exactly when it exists, and its offset equals its row's `offset_ms` bit for bit (this passes before the change too, and is what the codec relies on).
- [ ] A squeeze-out adds 8 bytes to a node payload (the tick), not 17.
- [ ] Encoding an activation whose SqOut entry and tick disagree throws `std::logic_error`.
- [ ] `path codec: a rebuilt record flattens to the same bytes` (test_path_codec.cpp line 104) passes.

**Verify:** `.\build-cpp\Release\hydra_tests.exe --test-case="squeeze-out:*,Activation: set_sqout*,path codec:*,windows read from a path JSON*"` all pass, then the full `hydra_tests` reports 0 failed.

**Steps:**
- [ ] Step 1: Write the corpus invariant test in `tests/test_search.cpp`. It passes today; it pins what the codec change relies on.

```cpp
TEST_CASE("squeeze-out: one SqOut, last, and its offset is its row's, on every corpus path") {
    int sqouts = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, true, true);
        if (song.is_empty()) continue;
        ScoreGraph graph(song, 4);
        for (const Path& p : run_search(graph, EngineOptions{DepthMode::Scores, 4})) {
            for (const Activation& act : p.all_activations()) {
                int n = 0;
                for (size_t i = 0; i < act.sqinouts.size(); ++i) {
                    if (act.sqinouts[i].kind != SqueezeKind::SqOut) continue;
                    ++n;
                    CHECK(i + 1 == act.sqinouts.size());  // always the last squeeze
                }
                CHECK(n <= 1);
                CHECK(act.sqout_tick.has_value() == (n == 1));
                if (n != 1 || !act.sqout_tick) continue;
                ++sqouts;
                const BackendSqueeze* row = nullptr;
                for (const BackendSqueeze& b : act.backends)
                    if (b.timecode.ticks() == *act.sqout_tick) row = &b;
                REQUIRE(row != nullptr);
                REQUIRE(row->offset_ms.has_value());
                CHECK(*row->offset_ms == act.sqinouts.back().offset_ms);  // exact
            }
        }
    }
    CHECK(sqouts > 0);
}
```

- [ ] Step 2: Write the failing unit tests. In `tests/test_model.cpp`:

```cpp
TEST_CASE("Activation: set_sqout stamps the tick, trims later rows, builds the SqOut from its row") {
    Activation act;
    const std::tuple<int64_t, double> rows[] = {{100, -40.0}, {200, -12.5}, {300, 30.0}};
    for (const auto& [tick, off] : rows) {
        BackendSqueeze b;
        b.timecode = Timecode::raw(tick);
        b.offset_ms = off;
        act.backends.push_back(b);
    }
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 7.0});

    act.set_sqout(200);
    CHECK(act.sqout_tick == std::optional<int64_t>(200));
    REQUIRE(act.backends.size() == 2);  // the row past the squeezed-out chord is gone
    REQUIRE(act.sqinouts.size() == 2);
    CHECK(act.sqinouts[1].kind == SqueezeKind::SqOut);
    CHECK(act.sqinouts[1].offset_ms == -12.5);  // read off the row, not typed twice
    REQUIRE(act.sqout_row() != nullptr);
    CHECK(act.sqout_row()->timecode.ticks() == 200);

    Activation none;
    CHECK(none.sqout_row() == nullptr);
    CHECK_THROWS_AS(none.set_sqout(200), std::logic_error);  // no row on that tick
}
```

In `tests/test_path_codec.cpp`:

```cpp
TEST_CASE("path codec: a squeeze-out is stored once, as its tick") {
    Activation act;
    act.timecode = Timecode::raw(0);
    BackendSqueeze row;
    row.timecode = Timecode::raw(3036);
    row.offset_ms = -93.75;
    row.points = 460;
    row.sqout_points = 260;
    act.backends.push_back(row);
    act.sqinouts.push_back(SPSqueeze{SqueezeKind::SqIn, 12.5});

    Activation plain = act;  // the same activation, not squeezed out
    act.set_sqout(3036);

    Path with, without;
    with.activations.push_back(act);
    without.activations.push_back(plain);
    const std::vector<uint8_t> a = encode_path_node(with);
    const std::vector<uint8_t> b = encode_path_node(without);
    CHECK(a.size() - b.size() == 8);  // the tick, and no second copy of the offset

    const Path back = decode_path_node(a);
    const Activation& got = back.activations.front();
    CHECK(got.sqout_tick == std::optional<int64_t>(3036));
    REQUIRE(got.sqinouts.size() == 2);
    CHECK(got.sqinouts[0].kind == SqueezeKind::SqIn);
    CHECK(got.sqinouts[0].offset_ms == 12.5);
    CHECK(got.sqinouts[1].kind == SqueezeKind::SqOut);
    CHECK(got.sqinouts[1].offset_ms == -93.75);

    // The writer refuses a squeeze-out it cannot store as one fact.
    Activation no_tick = act;
    no_tick.sqout_tick.reset();
    Path p1;
    p1.activations.push_back(no_tick);
    CHECK_THROWS_AS(encode_path_node(p1), std::logic_error);
    Activation drift = act;
    drift.sqinouts.back().offset_ms = -90.0;
    Path p2;
    p2.activations.push_back(drift);
    CHECK_THROWS_AS(encode_path_node(p2), std::logic_error);
}
```

- [ ] Step 3: Run them. `set_sqout` and `sqout_row` do not compile; after stubbing, the 8-byte check reads 17.
- [ ] Step 4: Add the two members in `model.h` (beside `is_sqout_backend`, line 327) and implement them in `model.cpp`:

```cpp
    // The squeezed-out chord's row, or nullptr when the activation did not
    // squeeze out. The one way to ask "which row, and how far from the SP
    // end": its offset_ms is the SqOut's offset.
    const BackendSqueeze* sqout_row() const;

    // Mark this activation as squeezing out the phrase chord at `tick`. The
    // only writer of a squeeze-out: it stamps sqout_tick, drops every row
    // past the chord (hit after SP ended), and appends the SqOut entry built
    // from the chord's own row. Throws std::logic_error when no row with an
    // offset sits on `tick`. The engine's copy-out and the codec call it.
    void set_sqout(int64_t tick);
```

```cpp
const BackendSqueeze* Activation::sqout_row() const {
    if (!sqout_tick) return nullptr;
    for (const BackendSqueeze& b : backends)
        if (b.timecode.ticks() == *sqout_tick) return &b;
    return nullptr;
}

void Activation::set_sqout(int64_t tick) {
    backends.erase(std::remove_if(backends.begin(), backends.end(),
                                  [tick](const BackendSqueeze& b) {
                                      return b.timecode.ticks() > tick;
                                  }),
                   backends.end());
    sqout_tick = tick;
    const BackendSqueeze* row = sqout_row();
    if (!row || !row->offset_ms) {
        sqout_tick.reset();
        throw std::logic_error("set_sqout: no backend row with an offset on tick " +
                               std::to_string(tick));
    }
    sqinouts.push_back(SPSqueeze{SqueezeKind::SqOut, *row->offset_ms});
}
```

- [ ] Step 5: Use it in `engine.cpp` `rebuild`. The squeeze loop (lines 1209-1217) pushes SqIns only and notes `took_sqout`. The block at lines 1227-1241 becomes:

```cpp
            if (took_sqout && oa.deact_edge >= 0) {
                const std::optional<Timecode>& sqout_at =
                    en.edges[(size_t)oa.deact_edge]->sqinout_time;
                if (sqout_at.has_value()) act.set_sqout(sqout_at->ticks());
            }
```

The corpus test from Step 1 showed the SqOut is always last, so the list order is unchanged.
- [ ] Step 6: Change the codec. `write_activation` first checks the activation is one fact: a SqOut entry exists exactly when `sqout_row()` is set, and its offset equals the row's; otherwise it throws `std::logic_error("write_activation: squeeze-out stored twice and disagreeing")`. It then writes the SqIn count and each SqIn's `f64` offset, with no kind byte. `read_activation` reads that list as SqIns. After it reads `sqout_tick`, it moves the value into a local, resets the field, and calls `act.set_sqout(t)`. A `std::logic_error` from that call is rethrown as the same `std::runtime_error` a malformed node raises.
- [ ] Step 7: Make `windows_for_path` read the accessor:

```cpp
        if (const BackendSqueeze* row = act.sqout_row()) {
            w.sqout_tick = row->timecode.ticks();
            w.sqout_offset_ms = row->offset_ms;
        }
```

Its old skip for "a SqOut with no tick" goes, because a decoded record can no longer hold one (an older layout is rejected; see `path codec: a node in an older layout is rejected`, test_path_codec.cpp line 262). Update the `windows_for_path` comment in `replay.h` (lines 200-208). The `hydra_replay` dump JSON keeps both fields; it is written from the one in-memory activation.
- [ ] Step 8: Hand this text to the final task (R3 step 9), which writes the ADR amendments in one pass. Amendment to ADR 0014: "2026-10: the squeeze-out is stored once. The node stores `sqout_tick`. The SqOut's offset is its row's `offset_ms`. The squeeze list stores SqIn offsets only. `Activation::set_sqout` is the one writer, and `Activation::sqout_row` the one reader. The SP end the offset was measured from is `deact_tick()`, over the SP-end history (R1)."
- [ ] Step 9: Run Verify. Commit by name.

### Task 14: Row labels read the stored squeeze-out (only if Q7 is yes)

*Draft label: C5.*

`BackendSqueeze::summarystr` gives every phrase-chord row a "... SqOut" label (finding 29). Only the squeezed-out row, the one `sqout_tick` names, is a squeeze-out. After Task C3, the graph no longer reads the row's `is_sp` flag either, so this label is its last reader.

On engine-made records the mislabel almost never reaches the screen. A deactivation edge always squeezes out its first phrase chord, and the engine drops every row after it. A collected phrase in a tail moves the SP end at least two measures past itself, so it falls outside the window unless two measures last under 500 ms (very fast tempo or a very short meter). The fix still matters for the one-owner rule, and it is the one rare visible change in this part, so it waits on Q7.

The fix: `summarystr` takes "is this the squeezed-out row?" from its caller, and the caller asks `Activation::is_sqout_backend`, which reads `sqout_tick`. The `is_sp` field is then read by nobody, so it is removed from `BackendSqueeze` and from the stored row.

**Goal:** The SqOut ladder applies only to the row `sqout_tick` names; every other row uses the plain ladder; `BackendSqueeze::is_sp` is gone.
**Findings:** 29
**Decision:** none of D1-D5; Q7 below
**Files:**
- Modify: `src/core/model.h` (`BackendSqueeze`, lines 202-218), `src/core/model.cpp` (lines 305-327)
- Modify: `src/app/path_view.cpp` (line 322)
- Modify: `src/search/graph.cpp` (line 230), `src/store/path_codec.cpp` (lines 120 and 158)
- Test: `tests/test_rules.cpp` (lines 155-161), `tests/test_squeeze_rating.cpp` (lines 340, 776, 782, 790), `tests/test_path_view.cpp` (lines 443, 493-517, 816)

**Acceptance Criteria:**
- [ ] A row that is not squeezed out reads the plain ladder whatever chord it is.
- [ ] The squeezed-out row reads the SqOut ladder exactly as today ("Insane SqOut <-- squeezed out (-260)" on Burnout still shows in `hydra_uitest`).
- [ ] No source line names `BackendSqueeze::is_sp`.

**Verify:** `.\build-cpp\Release\hydra_tests.exe --test-case="*summarystr*,build_activations*,rules: the leeway*,rate_activation*"` all pass; `.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4` reports all passed.

**Steps:**
- [ ] Step 1: Write the failing test in `tests/test_rules.cpp`, replacing lines 155-161:

```cpp
TEST_CASE("rules: the leeway moves the Standard edge of a backend rating") {
    BackendSqueeze b;
    b.offset_ms = 4.0;
    CHECK(b.summarystr(false, kDefaultHitWindowMs) == "Hard (uncounted)");
    CHECK(b.summarystr(false, kDefaultHitWindowMs, 5.0) == "Standard");
}

// Finding 29: the SqOut ladder is for the row the activation squeezed out
// (Activation::sqout_tick), not for every phrase chord.
TEST_CASE("BackendSqueeze::summarystr: the SqOut ladder is for the squeezed-out row only") {
    BackendSqueeze b;
    b.offset_ms = -50.0;
    CHECK(b.summarystr(true, 85.0) == "Hard SqOut");
    CHECK(b.summarystr(false, 85.0) == "Easy");
    b.offset_ms = 150.0;
    CHECK(b.summarystr(true, 85.0) == "Free SqOut");
    CHECK(b.summarystr(false, 85.0) == "Insane (uncounted)");
}
```

In `tests/test_path_view.cpp`, delete `phrase.is_sp = true;` (line 498) and change line 517 to `CHECK(b[4].rating == "Insane (uncounted)");`, with the comment at line 493 reading "A phrase chord past the leeway that this path did not squeeze out: a plain row." Delete `is_sp` at lines 443 and 816 and at `tests/test_squeeze_rating.cpp` line 340. Add `false,` as the first argument at `tests/test_squeeze_rating.cpp` lines 776, 782 and 790.
- [ ] Step 2: Run them; they do not compile (`summarystr` has no bool parameter).
- [ ] Step 3: Change `summarystr` to `std::string summarystr(bool squeezed_out, double hit_window_ms = kDefaultHitWindowMs, double leeway_ms = core::default_rules().backend_leeway_ms) const;`, with `if (squeezed_out)` in place of `if (is_sp)` at `model.cpp` line 314. Document it: "squeezed_out: this row is the activation's squeezed-out chord (Activation::is_sqout_backend)."
- [ ] Step 4: At `path_view.cpp` line 322: `row.rating = bsq.summarystr(br.squeezed_out, W, rules.backend_leeway_ms);`. `br.squeezed_out` already comes from `act.is_sqout_backend(bsq)` (`squeeze_rating.cpp` line 105).
- [ ] Step 5: Remove `is_sp` from `BackendSqueeze` (model.h line 207), from `operator==` (model.cpp line 307), from `store_new_backend` (graph.cpp line 230), and from the codec (path_codec.cpp lines 120 and 158).
- [ ] Step 6: Run Verify. Commit by name.

### Task 15: The replay asks "did Star Power pay this chord?"

*Draft label: C6.*

`replay_path` decides "inside SP" on its own (finding 32). Its gate repeats the first two lines of `backend_row_value`, and it counts a chord as "in SP" even when the pricing pays 0. That is exactly the squeezed-out one-note chord under the first-note rule, so the Preview disc shows x2 on a chord SP paid nothing for.

D2 keeps the replay's walk as a cross-check on the engine's totals. It changes only the yes/no: the replay asks one named helper in `core/backend_value.h`, built from `backend_row_value`. The helper also replaces the duplicated gate. The open-window walk still drops a window once it stops paying, which is safe: after a squeezed-out chord, every later chord is past it and pays nothing.

The test file keeps a verbatim copy of the old every-window walk (`reference_replay_path`, `tests/test_replay.cpp` lines 790-901) to prove the open-window walk. Its yes/no line moves to the same helper, behind a flag, so one new corpus test can show the before and after side by side.

**Should `PathReplay::faithful` compare per-chord SP membership too?** Not worth it. After this task, the yes/no and the points come from one function, so they cannot split. The only failure left is the walk paying a different set of chords with equal totals. Catching that needs per-chord facts from the engine. The engine stores none (it sums along graph edges), so it would mean a new stored field on every record for a check that has never failed. The corpus test below does the useful part in tests: it proves the only chords whose yes/no moved are squeezed-out chords SP paid nothing.

**Goal:** `row.in_sp` is true exactly when some window's SP actually paid the chord something, through `core::paid_by_sp`.
**Findings:** 32 (and the D2 half of 1)
**Decision:** D2: "The replay keeps its own chord-by-chord walk as a cross-check on the engine's totals. For 'did Star Power pay this chord?' it asks the shared rule in `core/backend_value.h` instead of deciding it itself. The Preview's xN disc doubles only when SP actually paid something for the chord. So a squeezed-out chord that SP pays nothing shows the plain multiplier, and a partly paid one still shows doubled. No stored change."
**Files:**
- Modify: `src/core/backend_value.h` (after `backend_row_value`, line 45)
- Modify: `src/core/replay.cpp` (window loop, lines 137-168)
- Modify: `src/core/replay.h` (the `in_sp` and `multiplier_shown` comments, lines 152-156)
- Test: `tests/test_replay.cpp` (new cases after line 263; `reference_replay_path` lines 790-901)

**Acceptance Criteria:**
- [ ] A one-note phrase chord squeezed out under the first-note rule reads `in_sp == false` and shows the plain multiplier; a two-note one still reads doubled.
- [ ] On every corpus path, every chord's points and the final totals are unchanged, and every chord whose `in_sp` changed is a squeezed-out chord with 0 SP points.
- [ ] The three open-window equivalence tests (lines 1012-1109) pass.

**Verify:** `.\build-cpp\Release\hydra_tests.exe --test-case="paid_by_sp*,replay:*,D2:*,a squeezed-out chord*,score box:*"` all pass; `hydra_replay selfcheck` reports no FAIL lines.

**Steps:**
- [ ] Step 1: Write the failing tests in `tests/test_replay.cpp` after line 263:

```cpp
TEST_CASE("paid_by_sp: yes exactly when the row's SP value is above zero") {
    using core::SqOutPosition;
    CHECK(core::paid_by_sp(-10.0, 100, 0, SqOutPosition::NoSqOut, 3.0));
    CHECK_FALSE(core::paid_by_sp(-10.0, 100, 0, SqOutPosition::Exact, 3.0));
    CHECK(core::paid_by_sp(-10.0, 100, 50, SqOutPosition::Exact, 3.0));
    CHECK_FALSE(core::paid_by_sp(-10.0, 100, 50, SqOutPosition::After, 3.0));
    CHECK(core::paid_by_sp(2.0, 100, 50, SqOutPosition::NoSqOut, 3.0));        // in the leeway
    CHECK_FALSE(core::paid_by_sp(4.0, 100, 50, SqOutPosition::NoSqOut, 3.0));  // past it
}

// D2: the disc doubles only when Star Power paid the chord something.
TEST_CASE("replay: a squeezed-out chord SP pays nothing shows the plain multiplier") {
    // 120 BPM, 192 ticks per beat: 96 ticks are 250 ms.
    auto build = [](bool two_notes) {
        Song song(192);
        song.tpm_changes[0] = 768;
        song.bpm_changes[0] = 120.0;
        song.build_timing();
        for (int64_t tick : {0, 768, 1536, 2304, 2976, 3072}) {
            SongTimestamp ts;
            ts.timecode = song.timecode(tick);
            ts.chord.add_note(NoteColor::Red);
            if (tick != 2976 || two_notes) ts.chord.add_note(NoteColor::Yellow);
            ts.flag_sp = tick == 2976;
            song.sequence.push_back(ts);
        }
        return song;
    };
    ReplayWindow w;
    w.act_tick = 0;
    w.deact_tick = 3072;
    w.sqout_tick = 2976;  // a phrase chord 250 ms before the SP end

    const Song one = build(false);
    const ReplayResult r = replay_path(one, {w});
    REQUIRE(r.chords.size() == 6);
    CHECK(r.chords[3].in_sp);  // an ordinary chord inside the window: doubled
    CHECK(r.chords[3].multiplier_shown == r.chords[3].multiplier_after * kStarPowerMultiplier);
    CHECK(r.chords[4].points.sp == 0);  // first-note rule: one note loses all its doubling
    CHECK_FALSE(r.chords[4].in_sp);
    CHECK(r.chords[4].multiplier_shown == r.chords[4].multiplier_after);

    // Two notes: only the first note's share is lost, so SP still paid it.
    const Song two = build(true);
    const ReplayResult r2 = replay_path(two, {w});
    CHECK(r2.chords[4].points.sp > 0);
    CHECK(r2.chords[4].in_sp);
    CHECK(r2.chords[4].multiplier_shown == r2.chords[4].multiplier_after * kStarPowerMultiplier);
}
```

Give `reference_replay_path` a fourth parameter `bool d2_in_sp = true`. In its window loop, keep the old gate and `++sp_claims`, and also count `if (core::paid_by_sp(offset, sg.sp, sg.sqout_sp(), pos, rules.backend_leeway_ms)) ++sp_paid;`. Set `row.in_sp = d2_in_sp ? sp_paid > 0 : sp_claims > 0;`. Then add the scope test after line 1039:

```cpp
// D2 moves only the yes/no on a squeezed-out chord SP paid nothing. Every
// chord's points, and so every score, stays as it was.
TEST_CASE("D2: only a squeezed-out chord SP pays nothing loses its doubled disc") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int changed = 0;
    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        for (const Path* p : corpus::analyzed(path, cfg).all_paths()) {
            const std::vector<ReplayWindow> wl = windows_for_path(*p);
            const ReplayResult before = reference_replay_path(song, wl, cfg.rules, false);
            const ReplayResult after = replay_path(song, wl, cfg.rules);
            REQUIRE(before.chords.size() == after.chords.size());
            CHECK(before.final == after.final);
            for (size_t i = 0; i < after.chords.size(); ++i) {
                const ReplayChord& b = before.chords[i];
                const ReplayChord& a = after.chords[i];
                CHECK(a.points == b.points);
                if (a.in_sp == b.in_sp) continue;
                ++changed;
                CHECK(b.in_sp);
                CHECK(a.points.sp == 0);
                bool squeezed_out = false;
                for (const ReplayWindow& w : wl)
                    if (w.sqout_tick == a.tick) squeezed_out = true;
                CHECK(squeezed_out);
                CHECK(a.multiplier_shown == a.multiplier_after);
            }
        }
    }
    MESSAGE(changed << " corpus chords now show the plain multiplier");
}
```

- [ ] Step 2: Run them. `paid_by_sp` does not exist; after a stub, the one-note case reads `in_sp == true`.
- [ ] Step 3: Add the helper to `core/backend_value.h`:

```cpp
// Did Star Power pay this row anything on this path? The one yes/no for
// "inside SP", built from the price above. The replay's xN disc doubles
// exactly when it is true (decision D2): a squeezed-out chord whose
// sqout_points are 0 reads no, a partly paid one reads yes.
inline bool paid_by_sp(double offset_ms, int points, int sqout_points,
                       SqOutPosition pos, double leeway_ms) {
    return backend_row_value(offset_ms, points, sqout_points, pos, leeway_ms) > 0;
}
```

- [ ] Step 4: In `replay_path`, replace the `pays` gate (lines 154-162) with the helper:

```cpp
            const bool paid = core::paid_by_sp(offset, sg.sp, sg.sqout_sp(), pos,
                                               rules.backend_leeway_ms);
            if (paid) {
                ++sp_claims;
                sp_points += core::backend_row_value(offset, sg.sp, sg.sqout_sp(), pos,
                                                     rules.backend_leeway_ms);
            }
            // Keep the window unless it is past its deactivation node and paid
            // nothing here (see `open` above).
            if (paid || !past_deact) open[kept++] = open[k];
```

Rows that pay 0 added 0 before, so `sp_points` is unchanged. Add a third bullet to the "leaves for good" comment (lines 81-91): a squeezed-out chord that pays 0 also ends the window, and every later chord is past it. Rename `sp_claims` to `sp_paid` and update the `in_sp` comments in `replay.h`.
- [ ] Step 5: Run Verify and the full `hydra_tests`. The Preview test at `tests/test_preview_view.cpp` lines 1276-1303 passes unchanged (its path has no squeeze-out). Commit by name.

### Task 16: The score box drops the doubled disc when SP ends (only if Q8 is yes)

*Draft label: C7.*

Finding 1: between the SP end and the next chord, the drain box says "SP drain (if activated)" while the score box still shows the doubled multiplier. D2 decides which chords are paid. It does not decide what the disc shows between chords, so this needs your answer first.

If yes, the score box shows the doubled value only while the playhead sits inside an SP window, and the drain box asks the same question through one helper. Each step keeps both multipliers, so the box can pick.

**Goal:** The score box and the drain box ask one "is SP running at this time?" helper.
**Findings:** 1
**Decision:** Q8 below
**Files:**
- Modify: `src/app/preview_view.h` (`PreviewScoreStep`, lines 172-177), `src/app/preview_view.cpp` (`build_score` line 233, `build_score_box` lines 552-575, `build_drain_box` lines 596-602)
- Test: `tests/test_preview_view.cpp` (lines 1298-1300)

**Acceptance Criteria:**
- [ ] At 6600 ms in the fixture at line 1276, the box reads "x2 · combo 14", not "x4".
- [ ] Inside the window the box still reads doubled ("x2 · combo 7" at 3000 ms).

**Verify:** `.\build-cpp\Release\hydra_tests.exe --test-case="score box:*,drain box:*"` all pass.

**Steps:**
- [ ] Step 1: Change the test at lines 1298-1300 to the new answer:

```cpp
    // 6600 ms: Star Power has ended, so the disc is plain even though the
    // last chord hit (6500 ms, on the deactivation node) was paid doubled.
    CHECK(build_score_box(scene, 6600.0).detail == "x2 " + kDot + " combo 14");
```

- [ ] Step 2: Run it; it fails ("x4").
- [ ] Step 3: Add `int multiplier_plain = 1;` to `PreviewScoreStep` (the combo multiplier, `ReplayChord::multiplier_after`), and fill it in `build_score`. Add one file-local helper in `preview_view.cpp`, used by both boxes:

```cpp
// The activation whose Star Power is running at `now`, or nullptr. Running
// from the activation chord up to, not including, the SP end. The drain box
// and the score box both ask this, so they cannot disagree.
const PreviewActivation* running_activation(const PreviewScene& scene, double now) {
    for (const PreviewActivation& a : scene.activations)
        if (a.has_sp_end && a.ms <= now && now < a.sp_end_ms) return &a;
    return nullptr;
}
```

In `build_score_box`: `const int shown = running_activation(scene, now_ms) ? at.multiplier : at.multiplier_plain;`, and print `shown`. In `build_drain_box`, replace the loop at lines 596-602 with `const PreviewActivation* running = running_activation(scene, now);`.
- [ ] Step 4: Run Verify. Commit by name.

### Task 17: A variant folded mid-SP takes its leader's steps after the fold

*Draft label: B1.*

**Goal:** A variant folded while SP runs stores its own SP end steps up to the fold and its leader's after it, plus its leader's deactivation and, if Q2 is yes, the closing squeeze. Its activation note and everything before the fold stay its own.

**Findings:** 90

**Decision:** D3: "A tied variant folded into its leader while SP is still running takes the leader's closed activation from the fold point: its SP end, clamp note, collected phrases and backend rows. It keeps its own activation note. Why: from the fold point their futures are identical, so the leader's closed activation is the variant's real one. The search does not change." R3 step 7: "A variant's own steps up to the fold, then its leader's steps after it." Q2 decides the closing squeeze; the code below assumes yes.

**Why this is exact.** The search folds two SP-running paths only when they stand on the same SP node with the same SP end and nothing buffered (`reduce_iteration_paths`, HEAD 886-897). From then on, every engine event depends only on that SP end and the notes ahead: each phrase's extension or clamp (`advance`, keyed by `sp_end_time`), the deactivation and its SqIn/SqOut choice (`branch_deactivate`), and the deact edge's backend rows. Inside SP both paths also hold no bank, no skips and no SP-ready time, because `branch_activate` resets them. So every step the leader adds after the fold is a step the variant would have added.

With R1 this gets simpler than the first draft. `deact_tick()`, `clamp_tick()` and `collected_phrase_ticks()` are accessors over `sp_end_steps`. So the variant needs only the right step list, and all three come out right on their own. In particular the clamp rule takes care of itself: the variant's own `Clamped` steps before the fold, then the leader's after it, and `clamp_tick()` reads the last one.

The split point is the fold node's tick. Every step either path holds at the fold has a tick at or before that node: an activation step sits on its own chord, and a collected or clamped step sits on a phrase already crossed. The one step R1 places later than the moment it is pushed, a late SqIn's step on its phrase chord, is pushed while the path is buffered, and the search never folds a buffered path. So the variant's steps are all its own, and the leader's steps with a tick after the fold node are the shared future. `close_folded_act` checks that invariant and throws if it ever fails.

A closing SqOut gives back the squeezed-out phrase and every step at or after it (D1's `trim_ends` in `create_deactivated_path`). The same trim is applied to the variant's own steps, which is what the engine would have done had it carried the variant on.

What stays the variant's own: activation note, the bank and skips it activated with, its early-fill offset, the squeezes its window had at the fold, and its steps up to the fold. The transfer scales are recomputed in `rebuild` from the variant's own activation note and steps, never copied from the leader.

**Files:**
- Modify: `src/search/engine.cpp`: `Variant` (HEAD 186-195), the helpers beside `act_count` (369-371), the emit declarations (387-390), `reduce_group`'s fold block (760-773), `emit_variant` and `emit_path` (993-1031).
- Test: `tests/test_search.cpp`: helpers inside the anonymous namespace after `last_act` (HEAD 418); tests appended at the end of the file.

**Acceptance Criteria:**
- [ ] On the audit chart (cap 2), variant '0 1' stores steps {11520→14592 Activation, 11904→14976 Clamped, 13056→16128 Clamped}, so `deact_tick()` is 16128, `clamp_tick()` 13056 and `collected_phrase_ticks()` {11904, 13056}. Its backend rows equal its leader's.
- [ ] On the squeeze chart, the variant under '1+' ends {…, 13056→16128 Clamped, 16224→17664 SqIn} with one SqIn. The variant under '1-' ends on 13056→16128 with one SqOut and `sqout_tick` 16224, and its backend rows equal its leader's.
- [ ] `replay_stored_path(...).faithful()` holds for every path on both charts.
- [ ] The Part B baseline compare after B1 prints exactly the expected fixture lines (Step 7) and `0 list, 0 path` on the corpus, with `oracle OK` on any `FACT` line.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="tied variants: a variant folded mid-SP*"
# expect: [doctest] test cases: 2 | 2 passed
```
Then the compare in Step 7.

**Steps:**

- [ ] **Step 1: Write the failing tests.** Add `#include "core/replay.h"` and `#include <tuple>` to `tests/test_search.cpp`. Add inside the anonymous namespace, after `last_act`:

```cpp
// The audit's constructed chart for finding 90 ("midsp"), 120 BPM 4/4, run at
// cap 2. '1' activates at 10752; '0 1' activates at 4608, then at 11520. The
// phrase at 11904 clamps both windows to end at 14976, on the same SP node
// with the same score, so the search folds '0 1' into '1' there. The phrase
// at 13056 then clamps the window again, to 16128: after the fold.
//
// with_squeeze adds four notes. The phrase at 11328 lands in '1''s SP but
// before '0 1' activates, so only the leader collects it; the note at 6144
// keeps the two tied. The note at 16032 and the phrase at 16224 sit 250 ms
// either side of the SP end at 16128, so the leader splits there into '1+'
// (SqIn) and '1-' (SqOut), and the variant hangs off both.
std::vector<TailNote> midsp_notes(bool with_squeeze) {
    std::vector<TailNote> n;
    for (int64_t t = 0; t < 3072; t += 96) n.push_back({t, t == 768 || t == 2304, false});
    n.push_back({4608, false, true});
    if (with_squeeze) n.push_back({6144});
    n.push_back({8448, true, false});
    n.push_back({9216, true, false});
    n.push_back({10752, false, true});
    if (with_squeeze) n.push_back({11328, true, false});
    n.push_back({11520, false, true});
    n.push_back({11904, true, false});
    n.push_back({12672, false, true});
    n.push_back({13056, true, false});
    n.push_back({15360});
    if (with_squeeze) {
        n.push_back({16032});
        n.push_back({16224, true, false});
    }
    n.push_back({16896});
    n.push_back({17664});
    n.push_back({18432});
    return n;
}

void collect_paths(const Path& p, std::vector<const Path*>& out) {
    out.push_back(&p);
    for (const Path& v : p.variants) collect_paths(v, out);
}
std::vector<const Path*> every_path(const std::vector<Path>& roots) {
    std::vector<const Path*> out;
    for (const Path& r : roots) collect_paths(r, out);
    return out;
}
const Path* root_named(const std::vector<Path>& roots, const std::string& s) {
    for (const Path& r : roots)
        if (r.pathstring() == s) return &r;
    return nullptr;
}
// The leader's variant whose own last activation is at `tick`.
const Path* variant_at(const Path& leader, int64_t tick) {
    for (const Path& v : leader.variants)
        if (!v.activations.empty() && v.activations.back().timecode.ticks() == tick)
            return &v;
    return nullptr;
}
using Step = std::tuple<int64_t, int64_t, SpEndKind>;
std::vector<Step> steps_of(const Activation& a) {
    std::vector<Step> out;
    for (const SpEndStep& s : a.sp_end_steps) out.emplace_back(s.tick, s.end_tick, s.kind);
    return out;
}
```

Append at the end of the file:

```cpp
// ---- Tied variants keep their own state (decision D3) ------------------

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's steps after the fold") {
    const Song song = build_tail_song(midsp_notes(false));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* leader = root_named(roots, "1");
    REQUIRE(leader != nullptr);
    const Path* variant = variant_at(*leader, 11520);
    REQUIRE(variant != nullptr);
    CHECK(variant->totalscore() == leader->totalscore());

    const Activation& mine = variant->activations.back();
    const Activation& lead = leader->activations.back();
    CHECK(steps_of(lead) == std::vector<Step>{{10752, 13824, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    // Its own activation step and its own clamp at the fold, then the
    // leader's clamp at 13056. Today the list stops at the fold: 14976.
    CHECK(steps_of(mine) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                              {11904, 14976, SpEndKind::Clamped},
                                              {13056, 16128, SpEndKind::Clamped}});
    CHECK(mine.deact_tick() == std::optional<int64_t>(16128));
    CHECK(mine.clamp_tick() == std::optional<int64_t>(13056));
    CHECK(mine.collected_phrase_ticks() == std::vector<int64_t>{11904, 13056});
    // The real deactivation's rows, not the song's last notes.
    CHECK(mine.backends == lead.backends);
    CHECK(mine.sqinouts.empty());
    CHECK(variant->pathstring() == "0 1");

    for (const Path* p : every_path(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}

TEST_CASE("tied variants: a variant folded mid-SP takes its leader's closing SqIn or SqOut") {
    const Song song = build_tail_song(midsp_notes(true));
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 6});

    const Path* in_lead = root_named(roots, "1+");
    const Path* out_lead = root_named(roots, "1-");
    REQUIRE(in_lead != nullptr);
    REQUIRE(out_lead != nullptr);
    const Path* in_var = variant_at(*in_lead, 11520);
    const Path* out_var = variant_at(*out_lead, 11520);
    REQUIRE(in_var != nullptr);
    REQUIRE(out_var != nullptr);

    // The leader collected 11328 in SP. The variant did not: it banked that
    // phrase before activating at 11520, so the step is the leader's alone.
    CHECK(in_lead->activations.back().collected_phrase_ticks() ==
          std::vector<int64_t>{11328, 11904, 13056, 16224});

    // The SqIn side: a late SqIn on the phrase at 16224 moves the end to 17664.
    const Activation& a = in_var->activations.back();
    CHECK(steps_of(a) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped},
                                           {16224, 17664, SpEndKind::SqIn}});
    REQUIRE(a.sqinouts.size() == 1);
    CHECK(a.sqinouts[0].kind == SqueezeKind::SqIn);
    CHECK(in_var->pathstring() == "0 1+");

    // The SqOut side: SP ends at 16128 and the phrase at 16224 has no step.
    const Activation& b = out_var->activations.back();
    CHECK(steps_of(b) == std::vector<Step>{{11520, 14592, SpEndKind::Activation},
                                           {11904, 14976, SpEndKind::Clamped},
                                           {13056, 16128, SpEndKind::Clamped}});
    REQUIRE(b.sqinouts.size() == 1);
    CHECK(b.sqinouts[0].kind == SqueezeKind::SqOut);
    CHECK(b.sqout_tick == std::optional<int64_t>(16224));
    CHECK_FALSE(b.backends.empty());  // 16032 and the squeezed-out 16224
    CHECK(b.backends == out_lead->activations.back().backends);
    CHECK(out_var->pathstring() == "0 1-");

    for (const Path* p : every_path(roots))
        CHECK_MESSAGE(replay_stored_path(song, *p).faithful(), p->pathstring());
}
```

These shapes were checked against today's binary on the `.chart` twins (`hydra_replay dump` at cap 2). Today the first chart lists '1' 7150 with variant '0 1' 7150, SP end 14976, which replays to 6950. The second lists '1+' 8950 and '1-' 8350, each with a variant '0 1' ending at 14976, replaying to 7950. `hydra_replay target --ticks 4608,11520 --cap 2` on the second chart prices the path alone as '0 1+' 8950 ending 17664 with a SqIn, and '0 1-' 8350 ending 16128 with a SqOut at 16224. The step lists are worked out from the cap-2 rule (`extend_deacts`) and R1's step definition; the R1 task's own tests pin the leader's list.

- [ ] **Step 2: Run them and see them fail.** Expected: the variant's step list stops at 11904→14976 (and today's tail rows replace the real backends), and `faithful()` fails for the variants.

- [ ] **Step 3: Record the fold on the variant.** In `Variant`, beside D1's `end_tail`:

```cpp
    // Folded while its last activation's SP was still running. From the fold
    // on, the leader's closing of that window is the variant's own (D3).
    bool open_sp;
    // The chart tick of the node both paths stood on at the fold.
    int64_t fold_tick;
    // How many squeezes the leader's running window held at the fold. Any
    // later ones happened after it, on both paths.
    int32_t fold_sq_count;
```

Beside `act_count`:

```cpp
    int32_t sq_count(int32_t sq_tail) const {
        int32_t n = 0;
        for (int32_t s = sq_tail; s >= 0; s = sqs_[(size_t)s].prev) ++n;
        return n;
    }
```

In `reduce_group`, after D1's `v.end_tail = p.end_tail;`:

```cpp
            // Both paths stand on the same node. On an SP node the window is
            // still open, and the leader will close it for both.
            v.open_sp = p.node >= 0 && node(p.node).is_sp && leader.act_tail >= 0;
            v.fold_tick = p.node >= 0 ? node(p.node).tick : NO_TIME;
            v.fold_sq_count =
                v.open_sp ? sq_count(acts_[(size_t)leader.act_tail].sq_tail) : 0;
```

Nothing else in `reduce_group` or `reduce_iteration_paths` changes: same key, same survivors.

- [ ] **Step 4: Close the window at copy-out.** Declare `void close_folded_act(int32_t own, int32_t lead, const Variant& var);` and give `emit_variant` a `const std::vector<int32_t>& parent_walk` parameter. Add:

```cpp
// D3: the variant was folded while its last activation's SP was running. From
// the fold node on, both paths met the same notes with the same SP end, so the
// leader's closing of that window is the variant's: its later SP end steps,
// its deactivation and (Q2) its closing squeeze. Before the fold the variant
// keeps its own: activation, bank, skips, early-fill offset, squeezes, steps.
// `own` and `lead` index out_acts_. `lead` is the same window on the path the
// variant folded into, already closed.
void Engine::close_folded_act(int32_t own_i, int32_t lead_i, const Variant& var) {
    // Work on copies and write back once. Each entry is copied to a local
    // before its push, because a push can reallocate the vector it reads.
    const OutAct lead = out_acts_[(size_t)lead_i];
    OutAct own = out_acts_[(size_t)own_i];

    own.deact_edge = lead.deact_edge;
    own.final_sp_end = lead.final_sp_end;  // drop if R1 reads it off the last step

    const int32_t sq_begin = (int32_t)out_sqs_.size();
    for (int32_t k = own.sq_begin; k < own.sq_end; ++k) {
        const OutSq s = out_sqs_[(size_t)k];
        out_sqs_.push_back(s);
    }
    for (int32_t k = lead.sq_begin + var.fold_sq_count; k < lead.sq_end; ++k) {
        const OutSq s = out_sqs_[(size_t)k];
        out_sqs_.push_back(s);
    }
    own.sq_begin = sq_begin;
    own.sq_end = (int32_t)out_sqs_.size();

    // A closing SqOut gives back its phrase and every step at or after it,
    // the trim create_deactivated_path makes (D1's trim_ends). Read the
    // squeezed-out chord from the one owner Part C leaves; at HEAD that is
    // the deact edge's sqinout_time.
    int64_t give_back = NO_TIME;
    for (int32_t k = own.sq_begin; k < own.sq_end; ++k)
        if (out_sqs_[(size_t)k].kind == SQ_OUT && own.deact_edge >= 0)
            give_back = edge(own.deact_edge).sqinout_time;

    const int32_t end_begin = (int32_t)out_ends_.size();
    for (int32_t k = own.end_begin; k < own.end_end; ++k) {
        const SpEndStep s = out_ends_[(size_t)k];
        if (s.tick > var.fold_tick)
            throw std::logic_error("a folded variant holds a step past its fold");
        if (give_back == NO_TIME || s.tick < give_back) out_ends_.push_back(s);
    }
    for (int32_t k = lead.end_begin; k < lead.end_end; ++k) {
        const SpEndStep s = out_ends_[(size_t)k];
        if (s.tick > var.fold_tick) out_ends_.push_back(s);
    }
    own.end_begin = end_begin;
    own.end_end = (int32_t)out_ends_.size();

    out_acts_[(size_t)own_i] = own;
}
```

In `emit_variant`, after its `emit_acts(...)` call:

```cpp
        if (var.open_sp) {
            if (op.act_end <= op.act_begin || var.var_point < 1 ||
                (size_t)var.var_point > parent_walk.size())
                throw std::logic_error("a variant folded mid-SP has no window to close");
            close_folded_act(op.act_end - 1, parent_walk[(size_t)var.var_point - 1], var);
        }
```

and replace its recursive call with:

```cpp
        // This variant's activations as its own variants read them: its own,
        // then its parent's from its var_point on (prepare_variants' order).
        std::vector<int32_t> walk;
        for (int32_t j = op.act_begin; j < op.act_end; ++j) walk.push_back(j);
        for (size_t j = (size_t)op.var_point; j < parent_walk.size(); ++j)
            walk.push_back(parent_walk[j]);
        emit_variant(var.var_head, depth + 1, walk);
```

In `emit_path`, after `out_paths_.push_back(op);`:

```cpp
    std::vector<int32_t> walk;
    for (int32_t j = op.act_begin; j < op.act_end; ++j) walk.push_back(j);
    emit_variant(p.var_head, 1, walk);
```

Why index `var_point - 1` is the right window: at the fold the leader held `var_point` activations and its newest was the open one. In the leader's final list that same window, now closed, sits at index `var_point - 1`. A nested variant reads it through its parent's walk, which already holds the parent's own closed window. `rebuild` needs no change: a set `deact_edge` already gives the backend rows, the SqOut trim and `sqout_tick`, and R1's accessors read the new steps.

- [ ] **Step 5: Extend Part D's corpus check to variants.** D1's corpus case "SP end steps: every corpus activation is consistent" and D2's bank checks loop over root paths only (Appendix C (Part D's first draft), "Notes for Part B"). Change their loop from the roots to every path (`rec.all_paths()`, reading `walk_activations()`).

- [ ] **Step 6: Run the tests.** Expected: `2 | 2 passed`, the widened D1/D2 corpus cases pass, and the whole `hydra_tests.exe` run passes.

- [ ] **Step 7: Run the Part B compare.** Build `hydra_replay`, dump into `after_B1` folders as in the baseline task's Step 4, and compare with `-Exe` set to the new binary. Expected on `base_fix2`: three `PATH` lines, all on midspE: `'0 1 | 8950 | …' -> '0 1+ | 8950 | …'` and `'0 1 | 8350 | …' -> '0 1- | 8350 | …'` at depth 4, and the `0 1-` one again at depth 40. Five `FACT` lines: midsp '0 1' at both depths (14976 → 16128), and the three midspE variants. Every one prints `oracle OK`. Summary: `compared 3 charts, 43 paths: 0 list, 3 path, 5 fact differences, 0 oracle disagreements`. On `base_corpus`: `0 list, 0 path`, and any `FACT` line must be a variant with `oracle OK`. On `base_fix4`: all zero. With Q2 = no, the `PATH` lines disappear and the midspE `FACT` lines print `oracle DISAGREES`, because the lone search does take the squeeze.

- [ ] **Step 8: Commit.** Stage by name: `git add src/search/engine.cpp tests/test_search.cpp`, plus the D corpus test file if Step 5 touched another file. Message "Tied variant folded mid-SP takes its leader's steps after the fold (D3)", with the plan's trailers.

### Task 18: Each tied variant keeps its own bank

*Draft label: B2.*

**Goal:** Every variant stores its own `trailing_bank_ticks` (so its own `leftover_sp()`), and a variant folded between windows stores its own `bank_rise_ticks` on its next activation.

**Findings:** 89, plus Part D's note on `bank_rise_ticks` for variants.

**Decision:** D3: "Finding 89 is fixed alongside: each variant stores its own leftover SP, and which paths are listed stays as it is." R2: `trailing_bank_ticks` replaces `leftover_sp`, and `bank_rise_ticks` replaces `sp_meter`. R3 step 7: "Its own bank and fill lists."

**Why, case by case.** A variant's bank list is the bars it banked since its last window closed, oldest first. Whether that differs from its leader's depends on where the fold happened.

- Folded after both finished the song. The search groups every finished path under one key, whatever it holds (HEAD 896). So the two banks can differ in size and ticks. The variant's trailing list is simply its own bank at the fold.
- Folded while SP was running. Both banks are empty then (`branch_activate` resets the bank). Everything banked later is shared, so the variant's lists are its leader's. B1 already covers the window itself.
- Folded between windows. Both hold the same meter, so the same number of bars, m. But they may have banked them at different ticks. The leader's next list (the bank on its next activation, or its trailing list if it never activates again) starts with its own m bars and continues with bars banked after the fold. After the fold the list only grows: a bar is given back only on a buffered squeeze-out edge, and the search never folds a buffered path. So the variant's list is its own m bars, then the leader's from position m on. Counting, not ticks, marks the split here: a squeezed-out bar sits at the later of the deact node and its phrase, which can be past the fold node.

That last case touches an activation the variant does not store today. Its next activation is the leader's, read from the parent at load (`prepare_variants`). So when the leader activated again after the fold, the engine stores the variant's own copy of that one activation, with the variant's bank ticks, as the variant's last own activation, and the variant's `var_point` moves one later. Its path string, score and place in the list do not change; only that activation's banked-bar ticks become the variant's. After that activation both paths' banks reset, so later activations stay shared.

`skipped_fill_ticks` on that copy stays the leader's for now. It names the fills behind `skips()`, and whether a variant gets its own skip state is Q3 (finding 97). If you choose Q3 (a), this copy is exactly where the variant's own skip list and early-fill offset get stamped.

The record keeps the variant's trailing list in the structure blob, next to its `var_point`, because a node payload holds activations only (ADR 0017). The personalised activation is an ordinary node activation and needs no new field.

**Files:**
- Modify: `src/search/engine.cpp`: `Variant`, `reduce_group`'s fold block, `emit_variant`, `emit_path`, a new `splice_bank` helper beside D2's `emit_ticks`.
- Modify: `src/core/model.cpp` `prepare_variants` (HEAD 540-556); `src/core/model.h` comment on `trailing_bank_ticks`.
- Modify: `src/store/path_codec.cpp` `write_tree_entry` / `read_tree_entry` (HEAD 212-251) and the comments at 185-187 and 376-378; `src/store/path_codec.h` 17-22.
- Test: `tests/test_search.cpp` (after B1's tests); `tests/test_path_codec.cpp` (new case at the end; comment at HEAD 125-126).

**Acceptance Criteria:**
- [ ] Tied-bank song (cap 4): the path activating at 2304 stores trailing {5760} (`leftover_sp()` 1), the one at 3072 stores none, whichever is the variant, also after a store round trip.
- [ ] Between-windows song (cap 2): of the two paths tied at 950, the one that first activated at 2304 stores `bank_rise_ticks` {5760, 8448} on its activation at 10752, and the other {8448, 9216}. Both path strings are unchanged ('0 0', '1 0'), and both replay faithfully.
- [ ] Don Broco - Actors (testdata, cap 4, depth 40, 10 ms): the paths at 233,100 store `leftover_sp()` 1 ('7', first activation 150720) and 3 ('3', 82560).
- [ ] A variant's trailing list set apart from its parent's survives `flatten_record`/`rebuild_record`, and no node hash moves.
- [ ] The Part B compare prints the same totals as after B1 (`0 list`, B1's PATH and FACT lines only, all `oracle OK`).

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="tied variants: *bank*,tied variants: Don Broco*,path codec: each variant*"
# expect: [doctest] test cases: 4 | 4 passed
```
Then the Part B compare as in B1 Step 7, expecting the same summary lines.

**Steps:**

- [ ] **Step 1: Write the failing tests.** In `tests/test_search.cpp` after B1's tests:

```cpp
TEST_CASE("tied variants: a variant that finished the song keeps its own bank") {
    // Cap 4. Two phrases fill 2 bars, then two fills. '0' activates at 2304;
    // its SP ends at 5376, so the phrase at 5760 is banked: 1 bar left. '1'
    // activates at 3072; its SP still runs at 5760, so that phrase extends it
    // instead: nothing left. Each doubles two notes, so both score 400, and
    // the search folds one into the other once both finish.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448}});
    ScoreGraph graph(song, 4);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 3});

    auto check_banks = [](const std::vector<const Path*>& all) {
        int tied = 0;
        for (const Path* p : all) {
            if (p->totalscore() != 400) continue;
            ++tied;
            REQUIRE(p->walk_activations().size() == 1);
            const int64_t at = p->walk_activations().front().timecode.ticks();
            const std::vector<int64_t> want =
                at == 2304 ? std::vector<int64_t>{5760} : std::vector<int64_t>{};
            CHECK_MESSAGE(p->trailing_bank_ticks == want, "activation at " << at);
            CHECK(p->leftover_sp() == static_cast<int>(want.size()));
        }
        CHECK(tied == 2);
    };
    const std::vector<const Path*> all = every_path(roots);
    int variants = 0;
    for (const Path* p : all) variants += p->var_point.has_value() ? 1 : 0;
    REQUIRE(variants == 1);  // the two ties are one root and its variant
    check_banks(all);

    HydraRecord rec;
    rec.paths = roots;
    const HydraRecord back = store::rebuild_record(store::flatten_record(rec));
    check_banks(back.all_paths());
}

TEST_CASE("tied variants: a variant folded between windows keeps its own banked bars") {
    // Cap 2. '0 0' activates at 2304 (SP to 5376), then banks 5760 and 8448;
    // the phrase at 9216 finds the meter full. '1 0' activates at 3072 (SP to
    // 7680, extended by 5760), then banks 8448 and 9216. At 9216 both hold
    // 2 bars with the same score, outside SP, so the search folds them. Both
    // then activate at 10752 with 2 bars, banked at different ticks.
    const Song song = build_tail_song({{0, true, false}, {768, true, false},
                                       {2304, false, true}, {3072, false, true},
                                       {5760, true, false}, {8448, true, false},
                                       {9216, true, false}, {10752, false, true},
                                       {11520}, {12288}, {16128}});
    ScoreGraph graph(song, 2);
    const std::vector<Path> roots = run_search(graph, EngineOptions{DepthMode::Scores, 4});

    int tied = 0;
    for (const Path* p : every_path(roots)) {
        if (p->totalscore() != 950) continue;
        ++tied;
        const ActivationWalk acts = p->walk_activations();
        REQUIRE(acts.size() == 2);
        CHECK(acts[1].timecode.ticks() == 10752);
        const bool first_at_2304 = acts[0].timecode.ticks() == 2304;
        CHECK(p->pathstring() == (first_at_2304 ? "0 0" : "1 0"));
        CHECK(acts[1].bank_rise_ticks ==
              (first_at_2304 ? std::vector<int64_t>{5760, 8448}
                             : std::vector<int64_t>{8448, 9216}));
        CHECK(acts[1].sp_meter() == 2);
        CHECK(replay_stored_path(song, *p).faithful());
    }
    CHECK(tied == 2);
}

TEST_CASE("tied variants: Don Broco - Actors at depth 40 shows each tied path's own bank") {
    // Audit finding 89: paths 29 '7' and 30 '3' tie at 233,100. '7' ends the
    // song with 1 bar of SP, '3' with 3. Today both read 1.
    std::string chart;
    for (const std::string& p : corpus::chart_paths())
        if (p.find("Don Broco - Actors") != std::string::npos) chart = p;
    REQUIRE_FALSE(chart.empty());

    SearchSettings s;
    s.sp_cap = 4;
    s.depth_mode = DepthMode::Scores;
    s.depth_value = 40;
    s.ms_filter = 10.0;
    const HydraRecord& rec = corpus::analyzed(chart, s);

    int seen = 0;
    for (const Path* p : rec.all_paths()) {
        if (p->totalscore() != 233100) continue;
        const int64_t first = p->walk_activations().front().timecode.ticks();
        if (first == 150720) { CHECK(p->leftover_sp() == 1); ++seen; }
        if (first == 82560) { CHECK(p->leftover_sp() == 3); ++seen; }
    }
    CHECK(seen == 2);
}
```

In `tests/test_path_codec.cpp`, at the end:

```cpp
// A variant's bank can differ from its parent's (decision D3, finding 89), so
// the structure stores its trailing list next to its var_point. It is a
// path total, not an activation fact, so no node payload moves.
TEST_CASE("path codec: each variant's own trailing bank rides in the structure") {
    HydraRecord rec = fixture().record;
    size_t root = 0;
    while (root < rec.paths.size() && rec.paths[root].variants.empty()) ++root;
    REQUIRE(root < rec.paths.size());
    Path& v = rec.paths[root].variants.front();
    v.trailing_bank_ticks = {111, 222, 333};

    const FlatRecord before = flatten_record(fixture().record);
    const FlatRecord after = flatten_record(rec);
    REQUIRE(after.nodes.size() == before.nodes.size());
    for (size_t i = 0; i < after.nodes.size(); ++i)
        CHECK(after.nodes[i].hash == before.nodes[i].hash);

    const HydraRecord back = rebuild_record(after);
    CHECK(back.paths[root].variants.front().trailing_bank_ticks ==
          std::vector<int64_t>{111, 222, 333});
    CHECK(back.paths[root].trailing_bank_ticks == rec.paths[root].trailing_bank_ticks);
}
```

The three hand-built songs were checked as `.chart` twins with today's binary. The finished one: '1' and '0' both 400, SP ends 7680 and 5376. The between-windows one: '1 0' and '0 0' both 950, both second activations at 10752 with 2 bars. Don Broco at depth 40: index 29 '7' (150720) and 30 '3' (82560), both 233,100.

- [ ] **Step 2: Run them and see them fail.** Expected: the finished variant shows its leader's trailing list; the between-windows variant shows its leader's {8448, 9216}; Don Broco reports 1 for 82560; the codec case gets the parent's list back.

- [ ] **Step 3: Record the bank at the fold.** In `Variant`:

```cpp
    // The path had finished the song when it folded.
    bool finished;
    // Its banked bars at the fold (an index into banks_, or -1). For a finished
    // path this is its whole trailing list; between windows, its first m bars.
    int32_t bank_tail;
```

In `reduce_group`, with B1's lines: `v.finished = p.node < 0;` and `v.bank_tail = p.bank_tail;`.

- [ ] **Step 4: Splice the lists at copy-out.** Beside D2's `emit_ticks`:

```cpp
    // The variant's own banked bars (its chain at the fold, m of them), then
    // the leader's list from position m on. Both held m bars at the fold, and
    // the leader's list only grew after it, so its first m are its own past.
    void splice_bank(int32_t own_tail, int32_t lead_begin, int32_t lead_end,
                     int32_t* begin, int32_t* end) {
        col_scratch_.clear();
        for (int32_t c = own_tail; c >= 0; c = banks_[(size_t)c].prev) col_scratch_.push_back(c);
        const int32_t m = (int32_t)col_scratch_.size();
        if (lead_end - lead_begin < m)
            throw std::logic_error("a folded variant banked more bars than its leader");
        *begin = (int32_t)out_ticks_.size();
        for (size_t k = col_scratch_.size(); k-- > 0;)
            out_ticks_.push_back(banks_[(size_t)col_scratch_[k]].tick);
        for (int32_t k = lead_begin + m; k < lead_end; ++k) {
            const int64_t t = out_ticks_[(size_t)k];
            out_ticks_.push_back(t);
        }
        *end = (int32_t)out_ticks_.size();
    }
```

Give `emit_variant` two more parameters, `int32_t parent_trail_begin, int32_t parent_trail_end` (the parent's trailing range in `out_ticks_`). In its body, after B1's `close_folded_act` block and before `out_paths_.push_back(op);`:

```cpp
        if (var.finished) {
            emit_ticks(banks_, var.bank_tail, out_ticks_, &op.bank_begin, &op.bank_end);
        } else if (var.open_sp) {
            op.bank_begin = parent_trail_begin;
            op.bank_end = parent_trail_end;
        } else if ((size_t)var.var_point < parent_walk.size()) {
            // Folded between windows, and the leader activated again. That
            // activation banked the variant's own bars up to the fold, so the
            // variant stores its own copy of it and reads its parent from the
            // one after.
            OutAct next = out_acts_[(size_t)parent_walk[(size_t)var.var_point]];
            splice_bank(var.bank_tail, next.bank_begin, next.bank_end,
                        &next.bank_begin, &next.bank_end);
            out_acts_.push_back(next);
            op.act_end = (int32_t)out_acts_.size();
            op.var_point = var.var_point + 1;
            op.bank_begin = parent_trail_begin;
            op.bank_end = parent_trail_end;
        } else {
            // Folded between windows; the leader never activated again.
            splice_bank(var.bank_tail, parent_trail_begin, parent_trail_end,
                        &op.bank_begin, &op.bank_end);
        }
```

The copy lands right after the variant's own activations in `out_acts_` (nothing else is pushed there in between), so `[act_begin, act_end)` stays one range. B1's walk already reads `op.var_point`, so nested variants see the copy. The recursive call passes `op.bank_begin, op.bank_end`. `emit_path` passes `op.bank_begin, op.bank_end` from D2's root trailing range. `rebuild` already copies `op.bank_*` into `trailing_bank_ticks` for every path, and `op.var_point` into `var_point`.

- [ ] **Step 5: Stop copying it in the model.** In `prepare_variants`, delete D2's `v.trailing_bank_ticks = trailing_bank_ticks;` (and `v.leftover_sp = leftover_sp;`, if R2's task left it), and write in its place: `// trailing_bank_ticks is the variant's own: the engine stores it and the record keeps it per variant (D3, finding 89).` Reword the field's comment in `model.h` the same way.

- [ ] **Step 6: Store it in the structure.** In `write_tree_entry`:

```cpp
    w.u32(static_cast<uint32_t>(path.variants.size()));
    for (const Path& v : path.variants) {
        w.opt_i32(v.var_point);
        // The variant's own banked bars at the song's end. Its score totals
        // and note count equal its parent's and prepare_variants copies them;
        // this list can differ.
        w.u32(static_cast<uint32_t>(v.trailing_bank_ticks.size()));
        for (int64_t t : v.trailing_bank_ticks) w.i64(t);
        write_tree_entry(w, v, flat, seen);
    }
```

In `read_tree_entry`:

```cpp
    for (uint32_t i = 0; i < nvar; ++i) {
        std::optional<int> var_point = r.opt_i32();
        std::vector<int64_t> trailing(r.u32());
        for (int64_t& t : trailing) t = r.i64();
        Path variant = read_tree_entry(r, lookup);
        variant.var_point = var_point;
        variant.trailing_bank_ticks = std::move(trailing);
        path.variants.push_back(std::move(variant));
    }
```

Reword the comments at HEAD 185-187, 376-378, `path_codec.h` 17-22 and `test_path_codec.cpp` 125-126: a variant's trailing bank is stored with its tree entry; its other totals are copied from its parent. No stamp bump here; the final task bumps `kPathFormatStamp`, and no dev build before it may open a real `hydra.db`.

- [ ] **Step 7: Run the tests and the compare.** Expected: `4 | 4 passed`, the whole suite green (including "path codec: a rebuilt record flattens to the same bytes"), and the Part B compare unchanged from B1's summary lines. The between-windows fixture adds no `FACT` line: the compare's facts do not include banked-bar ticks, and its strings and scores do not move.

- [ ] **Step 8: Commit.** Stage by name: `git add src/search/engine.cpp src/core/model.cpp src/core/model.h src/store/path_codec.cpp src/store/path_codec.h tests/test_search.cpp tests/test_path_codec.cpp`. Message "Each tied variant keeps its own bank (D3, finding 89)", with the plan's trailers.

### Task 19: The tied-variant guard and its ADR text

*Draft label: B3.*

**Goal:** A corpus test pins that every variant stores what a search pricing it alone stores, and the ADRs record the rule.

**Findings:** 89, 90

**Decision:** D3 (quoted in B1 and B2).

**Why.** The fixtures prove the mechanism on purpose-built charts. The guard catches any later case the fixtures do not shape, against an oracle that shares no fold: `search_target` re-prices a variant's exact activation ticks, and a root of that search was never folded. Skips, early-fill offsets and skipped fills are left out of the comparison, because they are finding 97 (Q3).

**Files:**
- Test: `tests/test_replay.cpp` (new case after "targeted search reproduces every corpus path", which ends at HEAD 182).
- Create: the next free ADR in `docs/adr/` (the final task numbers the plan's ADRs): "A tied variant's facts are its own".
- Modify: `docs/adr/0017-record-format-stores-chart-facts-once.md` lines 18-21. Per R3 step 9, the ADR text and the 0017 amendment are written in the final task; this task writes the words for it.

**Acceptance Criteria:**
- [ ] The guard compares more than zero variants against a lone search and finds no difference.
- [ ] The ADR text is ready for the final task.

**Verify:** `hydra_tests.exe -tc="every tied variant*"` → `1 passed`, and its message names how many variants it compared.

**Steps:**

- [ ] **Step 1: Write the guard.** In `tests/test_replay.cpp` after line 182:

```cpp
// Decision D3: a tied variant is stored as a branch of its leader, but its
// facts are its own. Price each variant alone with a targeted search and
// require the same per-activation facts and the same banks. Only a root of
// that search is an oracle: a root was never folded. Skips, the early-fill
// offset and the skipped fills are not compared; they are finding 97.
TEST_CASE("every tied variant stores what a search pricing it alone stores") {
    const app::AnalysisSettings cfg = app::Settings().to_analysis_settings();
    int variants = 0, compared = 0;

    for (const std::string& path : corpus::chart_paths()) {
        const Song& song = corpus::song(path, cfg.prodrums, cfg.bass2x, cfg.difficulty);
        if (song.is_empty()) continue;
        const HydraRecord& rec = corpus::analyzed(path, cfg);

        for (const Path* p : rec.all_paths()) {
            if (!p->var_point) continue;
            ++variants;
            const std::vector<Activation> want = p->all_activations();
            std::vector<int64_t> ticks;
            for (const Activation& a : want) ticks.push_back(a.timecode.ticks());

            const std::vector<Path> alone = search_target(song, cfg, ticks);
            const Path* match = nullptr;
            for (const Path& q : alone) {
                if (q.totalscore() != p->totalscore()) continue;
                const std::vector<Activation> qa = q.all_activations();
                bool same = qa.size() == want.size();
                for (size_t i = 0; same && i < qa.size(); ++i) {
                    same = qa[i].sqinouts.size() == want[i].sqinouts.size();
                    for (size_t k = 0; same && k < qa[i].sqinouts.size(); ++k)
                        same = qa[i].sqinouts[k].kind == want[i].sqinouts[k].kind;
                }
                if (same) { match = &q; break; }
            }
            if (!match) continue;
            ++compared;

            INFO(path << " [" << p->pathstring() << "]");
            CHECK(match->trailing_bank_ticks == p->trailing_bank_ticks);
            const std::vector<Activation> got = match->all_activations();
            for (size_t i = 0; i < got.size(); ++i) {
                CHECK(got[i].sp_end_steps == want[i].sp_end_steps);
                CHECK(got[i].bank_rise_ticks == want[i].bank_rise_ticks);
                CHECK(got[i].sqout_tick == want[i].sqout_tick);
                CHECK(got[i].display_backends() == want[i].display_backends());
            }
        }
    }
    CHECK(variants > 0);
    CHECK(compared > 0);
    MESSAGE("compared " << compared << " of " << variants << " variants against a lone search");
}
```

(`sp_end_steps` equality needs `SpEndStep::operator==`; if R1's task did not define it, compare through the `steps_of` shape from B1.)

- [ ] **Step 2: Run it.** It is expected to pass at once, since B1 and B2 have landed. It is the regression guard; the fixtures in B1 and B2 are the red tests. The corpus at default settings replays all 3,453 paths with no mismatch today, so it may hold few mid-SP folds.

- [ ] **Step 3: Write the ADR text for the final task.** In plain words: a tied variant is stored as a branch of its leader. Before the fold every fact is the variant's own. After the fold the leader's facts are the variant's, because the search folds only paths whose futures are identical. So a variant folded while SP runs keeps its own SP end steps up to the fold and takes its leader's after it, with its deactivation and (if Q2 is yes) its closing squeeze. A variant keeps its own banked bars: its whole trailing list when it folded after the song ended, and its own first bars on its next activation when it folded between windows. That activation is then stored as the variant's own, and `var_point` points past it. The trailing list sits next to `var_point` in the structure blob. Not decided here: a variant's skip state (finding 97). For ADR 0017 lines 18-21: "A variant's score totals and note count are not stored: `Path::prepare_variants` copies them from the parent on every load. Its banked bars are its own and are stored with it (ADR 00xx)."

- [ ] **Step 4: Commit.** `git add tests/test_replay.cpp`. Message "Guard: every tied variant matches a lone search (D3)", with the plan's trailers.

### Task 20: The SP gauge draws only stored facts

*Draft label: D4.*

**Goal:** With a path, `build_sp_meter_curve` reads `bank_rise_ticks`, `trailing_bank_ticks`, `sp_end_steps` through `refill_tick()`, and `deact_tick()`, and nothing else; CONTEXT.md's entry stops contradicting itself.

**Findings:** 5, 147, 159.

**Decision:** Q4 (the late squeeze-in drawing; the code below is answer (b)). R1: "Where the gauge refills on a late SqIn ... is the previous step's `end_tick`. It is read from the list, never stored." The gauge reads it through `Activation::refill_tick`.

Why this task: the record now holds every value the gauge shows. Between windows it steps one bar at each stored arrival. Inside a window it shows the measures left until the SP end in force, two to a bar, cutting at each step's refill tick and at each tempo and meter change. Every comparison is in ticks. Without a path there is no record, so the chart-only gauge stays as it is (Q10), in its own function.

**Files:**
- Modify: `src/app/preview_view.h:75-103` (PreviewActivation: `sp_meter` and `collected_phrase_ticks` go; `sp_end_changes` and `bank_rise_ticks` come), `148-152` and `194-223` (PreviewScene gains `trailing_bank_ticks`; comments).
- Modify: `src/app/preview_view.cpp:54-213` (the gauge), `327-360` (overlay), `400-403`.
- Modify: `CONTEXT.md:206-214`.
- Test: `tests/test_preview_view.cpp` (new cases after 1128; edits at 832-856, 894-922, 1065-1083, scene_difference 635-645, the old-scan comparison 1630-1740 and 1857-1902).

**Acceptance Criteria:**
- [ ] Late squeeze-in song: the gauge reads 0 just before 7000 ms, 1.0 at 7000 ms, 0.875 at 7250 ms (readout "0.9/4"), 0.5 at 8000 ms, 0 at 9000 ms.
- [ ] Early squeeze-out song: no step at 6750 ms; the squeezed-out bar arrives at 7000 ms; 2.0 at 7500 ms.
- [ ] A stored arrival with no phrase there, and a stored step with no collected phrase, both move the gauge: it follows the record.
- [ ] On every corpus path the gauge's value equals the old scan's at every boundary and 300 random times, outside late squeeze-in windows; the no-path gauge equals it everywhere.
- [ ] `build_sp_meter_curve`'s body names none of `sp_phrases`, `collected_phrase_ticks`, `sp_meter()`, `sp_bars_to_measures`, `std::min(`, `SqIn`.
- [ ] CONTEXT.md's entry describes the record-read gauge.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="sp meter*,drain box*,base + overlay*,preview lookups*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
```

**Steps:**

- [ ] **Step 1: Failing tests.** The five cases from Appendix C (Part D's first draft) D4 Step 1, with these changes. "a late squeeze-in refills at the old SP end" is unchanged in its gauge checks (the engine builds the record). "a squeezed-out bar arrives at the deact node" is unchanged. "the bank between windows is the record's" states its fixture as:

```cpp
    Activation act = sp_act_at(song, 3840);       // set_plain_window(a, 11520) inside
    act.bank_rise_ticks = {960, 1920};            // two bars; no phrase at 1920
    path.activations = {act};
    path.trailing_bank_ticks = {12480};           // a bar after the window; no phrase there
```

"the drain follows the stored steps, not the collected list" states:

```cpp
    Activation act = sp_act_at(song, 3840);
    act.bank_rise_ticks = {960, 1920};
    act.sp_end_steps = {{3840, 11520, SpEndKind::Activation},
                        {7680, 15360, SpEndKind::Collected}};  // no phrase ends at 7680
```

The source-scan case bans `"sp_phrases", "collected_phrase_ticks", "sp_meter()", "sp_bars_to_measures", "std::min(", "SqIn"`. The last one keeps the late-SqIn rule in `refill_tick`.

Add one more case, for the refill rule's home:

```cpp
TEST_CASE("refill_tick: a late squeeze-in's bar arrives at the old end") {
    Activation a;
    a.timecode = Timecode::raw(5760);
    a.sp_end_steps = {{5760, 13440, SpEndKind::Activation},
                      {12000, 15360, SpEndKind::Collected},
                      {15840, 19200, SpEndKind::SqIn}};
    CHECK(a.refill_tick(0) == 5760);
    CHECK(a.refill_tick(1) == 12000);
    CHECK(a.refill_tick(2) == 15360);  // past the end in force: the old end
}
```

(It lives in `tests/test_model.cpp`.)

`sp_act_at` (lines 152-160) becomes `sp_act_at(const Song& song, int64_t tick)` with the end stated by A6's literal at each call, through `test::set_plain_window`; the activation's bars come from `bank_rise_ticks`, which each gauge case states as real ticks. "an activation snaps to the recorded bars" (832-856) is deleted: "the bank between windows is the record's" replaces it. "a squeezed-out phrase does not bank mid-drain" (894-922) sets `path.trailing_bank_ticks = {11520};`. "an activation with no recorded bars" (1065-1083) becomes "an activation the record stamped nothing on draws nothing" and expects 0.0 at 3999, 4000, 5999 and 6000 ms.

The old-scan comparison (lines 1729-1740) becomes the by-value comparison from Appendix C (Part D's first draft) D4 Step 3, with two changes. A window counts as a late squeeze-in when any step `k` has `a.refill_tick(k) < a.sp_end_steps[k].tick`, read off the Path's own activations. And `old_scan::sp_meter_curve` reads each activation's bars and collected ticks off the Path (`act.sp_meter()`, `act.collected_phrase_ticks()`), passed in as a parameter, because `PreviewActivation` no longer carries them. The corpus no-path call compares too; the busy synthetic chart does not compare the gauge.

- [ ] **Step 2: Build and watch them fail.** The late squeeze-in case then fails at 7000 ms (about 0.118, wants 1.0) and 7250 ms (1.0, wants 0.875).

- [ ] **Step 3: Carry the facts into the scene.** In `preview_view.h`, `PreviewActivation` loses `sp_meter` (79) and `collected_phrase_ticks` (88-93) and gains:

```cpp
    // Where this window's SP end changed, read off the record: the tick the
    // change takes effect (Activation::refill_tick) and the end after it. The
    // first entry is the activation itself.
    struct SpEndChange {
        int64_t at_tick = 0;
        int64_t end_tick = 0;
        bool operator==(const SpEndChange&) const = default;
    };
    std::vector<SpEndChange> sp_end_changes;
    // Where each bar this activation spends arrived (Activation::bank_rise_ticks).
    std::vector<int64_t> bank_rise_ticks;
```

`PreviewScene` gains `std::vector<int64_t> trailing_bank_ticks;` (overlay; cleared with the others). In `apply_preview_overlay`:

```cpp
            pa.bank_rise_ticks = a.bank_rise_ticks;
            for (size_t k = 0; k < a.sp_end_steps.size(); ++k)
                pa.sp_end_changes.push_back({a.refill_tick(k), a.sp_end_steps[k].end_tick});
            if (std::optional<int64_t> d = a.deact_tick()) { /* has_sp_end, as today */ }
```

then `scene.trailing_bank_ticks = path->trailing_bank_ticks;` and the builder choice (`path != nullptr ? build_sp_meter_curve(...) : build_unanalyzed_sp_meter_curve(...)`).

- [ ] **Step 4: Rewrite the gauge.** Replace lines 54-213 with the code in Appendix C (Part D's first draft) D4 Step 6, with the window's cuts and step lookup reading `act.sp_end_changes`:

```cpp
        // Inside the window: the measures left until the SP end in force, two
        // to a bar. Cut where the record says the end changed and at each
        // tempo and meter change, where measures stop being linear in ms.
        for (size_t k = 1; k < act.sp_end_changes.size(); ++k) {
            const int64_t t = act.sp_end_changes[k].at_tick;
            cuts.push_back({t, timing.ms_index().at(t)});
        }
        ...
        size_t step = 0;  // the change in force
        for (const Cut& cut : cuts) {
            const int64_t end_tick = act.sp_end_changes[step].end_tick;
            push_segment(curve, prev_ms, cut.ms, bars_left(prev_tick, end_tick),
                         bars_left(cut.tick, end_tick));
            while (step + 1 < act.sp_end_changes.size() &&
                   act.sp_end_changes[step + 1].at_tick <= cut.tick)
                ++step;
            prev_tick = cut.tick;
            prev_ms = cut.ms;
        }
```

and the "nothing stamped" guard reading `act.sp_end_changes.empty() || !act.has_sp_end`. At the late squeeze-in's old end the cut's segment ends at 0 (end in force reached), the change takes effect there, and the next segment starts at one bar.

- [ ] **Step 5: CONTEXT.md.** Replace lines 206-214 with the entry in Appendix C (Part D's first draft) D4 Step 7.

- [ ] **Step 6: Run everything.** If any corpus value outside a late squeeze-in window differs from the old scan, stop: it is an unplanned visible change, and it goes back to the user.

- [ ] **Step 7: Commit.** Stage `src/app/preview_view.h src/app/preview_view.cpp CONTEXT.md tests/test_preview_view.cpp tests/test_model.cpp` by name. Commit "Preview SP gauge draws only stored engine facts".

### Task 21: The Preview's offered fills read the stored list (only if Q9 is yes)

*Draft label: D5.*

**Goal:** The Preview lights as offered exactly `Activation::skipped_fill_ticks`.

**Findings:** 52.

**Decision:** Q9.

Why this task: the nearest-n walk (`apply_preview_overlay`, lines 367-397) guesses from the count. D3 stored the answer, and the count is now its size. Under 1.1 the guess and the answer agree, so only 1.0 records change.

**Files:** `src/app/preview_view.h:75-103`, `src/app/preview_view.cpp:345, 362-398`, `CONTEXT.md:196-204`, `tests/test_preview_view.cpp`, all as listed in Appendix C's first-draft Task D5.

**Acceptance Criteria:**
- [ ] On the 1.0-rule fill song, fill A (19200) is Offered, fill B (24960) Hidden, fill C (28800) Taken.
- [ ] The existing fill cases give the same states as today, read from `skipped_fill_ticks`.
- [ ] On every corpus path (default 1.1 rule) the fill states equal the old nearest-n scan's.
- [ ] CONTEXT.md's Path overlay entry says offered fills come from the stored list.

**Verify:** `.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe -tc="build_preview_scene*,base + overlay*,preview lookups*"; .\build-cpp\Release\hydra_tests.exe` → `Status: SUCCESS!` both times.

**Steps:** as Appendix C (Part D's first draft) Task D5, with two changes. `PreviewActivation::skips` (line 80) becomes `skipped_fill_ticks`, copied from `a.skipped_fill_ticks`. And `act_at(song, tick, {fills})` (test_preview_view.cpp:106-111) sets only `a.skipped_fill_ticks`; there is no `skips` to write. `old_scan::fill_states` reads `a.skipped_fill_ticks.size()`. The new test is "build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged" from that draft.

### Task 22: Stamps, docs and the final proof

*Draft label: R3 step 9, A8, C8.*

**Goal:** The record format and results stamps move once for the whole plan, every doc says what the code now does, and the whole corpus proves nothing moved that the plan didn't name.

**Findings:** 48 (ADR 0011's story), 91's doc half, plus the doc halves of every finding above.

**Decision:** R3 step 9 (Appendix A); the global constraint "Task 22 bumps both stamps once".

The engine tasks changed the stored layout and some stored values without touching a stamp, so no dev build could open a real `hydra.db` in between. This task makes it safe again. Old records will read Stale and ask for one re-analysis, which is ADR 0018's rule. The docs come last, so they describe the finished code once.

**Files:**
- Modify: `src/store/stored_versions.h:46` (`kResultsStamp` to the version of the release that ships this plan, read from the version header at execution time, with `accepted` shrunk to it alone) and `:56` (`kPathFormatStamp` to `{7, {7}}`, with the history comment extended: "7 stored the SP-end history and the bank and fill lists, stored the squeeze-out once, and gave each SqIn its own transfer scale (ADR 0021)").
- Create: `docs/adr/0021-the-sp-end-history-is-stored.md` (R1, R2, Part A's four points and the refused-timing rule) and `docs/adr/0022-a-tied-variants-facts-are-its-own.md` (Task 19's text).
- Modify: `docs/adr/0011-*.md` (the "No fallback" sentence and a correcting note for finding 48: backend rows also exist before the SP end), `docs/adr/0013-*.md` (`clamp_tick` is now read from the history), `docs/adr/0014-*.md` (Part C's amendment text, Task 13 Step 8), `docs/adr/0017-*.md` lines 18-21 (Task 19's amendment).
- Modify: `CONTEXT.md` (the "Transfer scale" entry, lines ~129-132; the SP gauge and Path overlay entries if Tasks 20-21 did not already), `docs/UserGuide.md:126`, `docs/cap-clamped-squeeze-frontend-anchor.md` (status and the "Where Hydra prices it wrong" section).
- Test: none new. The three baselines and every suite.

**Acceptance Criteria:**
- [ ] `stored_versions.h` holds the new stamps, and its `static_assert`s compile.
- [ ] A record written before this task reads Stale in the app, checked with `hydra_uitest` on a scratch database built by the HEAD binary.
- [ ] The scores baseline compare prints only the path-string changes Q2 allowed, or nothing.
- [ ] The Part B compare prints no `oracle DISAGREES` line. Every `FACT` line it prints belongs to a variant Task 17 or 18 names.
- [ ] The Part C printout compare prints nothing (Part C's C8).
- [ ] `hydra_tests`, `hydra_uitest --all --jobs 4` and `hydra_replay selfcheck` all pass.
- [ ] `grep -rn "sp_end_shift_ms\|required_frontend_ms\|exact_even_split_ms\|transfer_pre" docs src` finds nothing, except history notes that say the name was removed.

**Verify:**
```powershell
$w = 'C:\Users\Patrick\Downloads\Hydra\hydra-test\.superpowers\sdd\2026-10-03-step1-engine-facts'
.\build_cpp.ps1 -Target hydra_tests; .\build-cpp\Release\hydra_tests.exe
.\build_cpp.ps1 -Target hydra_uitest; .\build-cpp\Release\hydra_uitest.exe --all --jobs 4
.\build_cpp.ps1 -Target hydra_replay; .\build-cpp\Release\hydra_replay.exe selfcheck
pwsh -NoProfile -File "$w\partD\scores.ps1" -Out "$w\baseline\scores-final.txt"
Compare-Object (Get-Content "$w\baseline\scores-HEAD.txt") (Get-Content "$w\baseline\scores-final.txt")
& "$w\partB\tied_compare.ps1" -Before "$w\partB\base_corpus" -After "$w\partB\final_corpus" -Exe .\build-cpp\Release\hydra_replay.exe
.\build-cpp\Release\hydra_tests.exe --test-case="print the corpus squeeze facts" --no-skip > "$w\baseline\squeeze-facts-final.txt"
Compare-Object (Get-Content "$w\baseline\hydra-baseline-squeeze-facts.txt") (Get-Content "$w\baseline\squeeze-facts-final.txt")
```
Expected: every suite passes. The two `Compare-Object` calls print nothing, apart from Q2's allowed string changes in the first. The Part B compare prints only named `FACT` lines and no oracle disagreement. Dump `final_corpus` first with `tied_compare.ps1 -Exe <new hydra_replay> -Out "$w\partB\final_corpus"`.

**Steps:**
- [ ] **Step 1: Stamps.** Edit `stored_versions.h` as listed. Build `hydra_tests` and run it.
- [ ] **Step 2: Stale check.** Build a scratch database with the HEAD binary kept in Task 4 (`$w\partB\replay_head.exe`, or a HEAD `hydra_batch` copy) on two `testdata` charts. Open it with the new `hydra_uitest` through `--db`, and check that the library rows read Stale. Never open the user's own `hydra.db`.
- [ ] **Step 3: ADRs.** Write ADR 0021 in plain English. It covers the history list and its four kinds, and that `deact_tick`, `clamp_tick` and `collected_phrase_ticks` are now views of it. It covers the three lists that replace counts (R2), and each SqIn's stored scale replacing `transfer_pre`. It also says the transfer scale is measured from the note that moves the SP end (D1), that "unknown" is a guard no fresh record reaches (D4), and that timing which can't measure time is refused at load (Q1's answer). Write ADR 0022 from Task 19's text. Add the amendments to 0011, 0013, 0014 and 0017. Use the next free numbers if 0021 or 0022 is taken by then.
- [ ] **Step 4: CONTEXT.md, User Guide, cap doc.** Make the edits Part A listed:
  - CONTEXT.md "Transfer scale": "measured from the note whose timing moves the SP end: the activation, or the cap's collecting note".
  - UserGuide line 126: replace "The squeeze numbers are unaffected; only the note you would move has changed." with "The scale line and the eff. figures are measured from that note."
  - Cap doc: change the status to "warning shipped; transfer scales re-anchored". Replace the section naming the deleted `sp_end_shift_ms`, `required_frontend_ms` and `exact_even_split_ms` with two sentences saying the SP-end history now anchors the scales.
- [ ] **Step 5: Final proof.** Run Verify. If any compare prints a difference no task named, stop and report the first differing line. Do not adjust a baseline.
- [ ] **Step 6: Commit** the stamp change and the docs as two commits, staged by name.

## Appendix A: integration rulings R1-R5

The task texts cite these rulings. They were written while joining the four drafts so that each fact is stored once.

The four drafts each made sound local designs, but together they store several facts twice. Part D's `sp_end_steps` ends on `deact_tick`. Its `bank_rise_ticks`, `trailing_bank_ticks` and `skipped_fill_ticks` have the same size as the stored `sp_meter`, `leftover_sp` and `skips`. Part A's per-squeeze `end_tick` and `nominal_deact_tick` are entries of Part D's step list. The user's rule is one stored fact, one place. These rulings fix that.

### R1. One stored SP-end history per activation

Each activation stores one list, `sp_end_steps`. Each step is `SpEndStep{int64_t tick; int64_t end_tick; SpEndKind kind;}`, where `kind` is one of `Activation`, `Collected`, `Clamped` or `SqIn`. `tick` is the note that caused the step: the activation chord for the first step, or the collecting phrase chord for the others. For a late SqIn that is the SqIn phrase chord, not the old SP end. `end_tick` is the SP end in force after the step. A squeezed-out phrase has no step. An activation whose SP outlasts the chart ends on the end the search tracked. If another cause moves the end (check `Engine::advance`, `branch_activate`, `branch_deactivate`, `create_deactivated_path` and the tail path), add a kind for it and say so.

The list replaces three stored fields: `deact_tick`, `clamp_tick` and `collected_phrase_ticks`. Each becomes a const accessor on `Activation`, keeping its name and return type so the change is mechanical and compiler-checked:
- `deact_tick()` is the last step's `end_tick` (unset when the list is empty, which only an old record can be).
- `clamp_tick()` is the tick of the last `Clamped` step (unset if none).
- `collected_phrase_ticks()` is the ticks of every non-`Activation` step, in order.

Facts Part A needs become accessors over the same list, in `core/model.h` / `model.cpp` beside the others:
- `nominal_end()` is the first step's `end_tick` (Part A's `nominal_deact_tick`, finding 156).
- The end a squeeze's offset was measured from: for a SqIn, the `end_tick` of the step before its `SqIn` step; for the SqOut, `deact_tick()`. Part A decides the accessor's name and signature, and how a SqIn `SPSqueeze` finds its step. Part C stores nothing new here.
- The anchor of an end is the tick of the latest `Clamped` step at or before the step that set that end, or the activation tick if none (D1).
- Where the gauge refills on a late SqIn (Part D's Q1, recommended "at the old SP end") is the previous step's `end_tick`. It is read from the list, never stored.

Before relying on R1, the drafter who owns it must check that every current reader of `collected_phrase_ticks` and `clamp_tick` gets the same values from the accessor on every corpus record. If some reader needs something the list can't give, say what and why, and keep that one field with the reason written down.

### R2. A list replaces the count it counts

Where a new list holds exactly the items a stored count counts, the list is stored and the count becomes an accessor (`size()`). That covers `bank_rise_ticks` → `sp_meter`, `trailing_bank_ticks` → `leftover_sp`, and `skipped_fill_ticks` → `skips`. If a count is not exactly the list's size on every record (for example a fractional or capped value), keep both, and write in the task why they are two different facts.

### R3. Task order for the whole plan

0. The existing plan `2026-10-03-one-squeeze-rating-rule.md` (Tasks 1-3), then A1. Display only.
1. A5, the load guard. Timing that makes a measure or beat zero, negative or infinite is stopped at load. Its "refuse or ignore" choice is a user question, but the guard lands first, because the window predicate and every tick/ms lookup assume time rises with tick.
2. One baseline task, at HEAD before any engine change. It captures three things: Part D's `scores.ps1` path and score list, Part B's per-variant fact dump and compare script, and Part C's C1 squeeze-facts printout. Every engine task after it compares against the baseline.
3. The SP-end history (R1). This merges A2, A6 and D1, owned by Part D's drafter.
4. D2 and D3: the bank and fill lists under R2.
5. A3, A4 and A7: transfer scales read the history (D1, D4); "unknown" value; the proof.
6. C2 to C6, plus C7 if the user says yes.
7. B1 to B3: tied variants. A variant's own steps up to the fold, then its leader's steps after it. Its own bank and fill lists.
8. D4 and D5: the Preview reads the stored facts.
9. The final task: bump `kPathFormatStamp` to 7 and `kResultsStamp` to the release version once, then docs (A8's docs, the new ADRs, the ADR 0011, 0013, 0014 and 0017 amendments, CONTEXT.md), the full baseline comparison and every test suite.

Finding 37 is not in this plan. The Clone Hero evidence says the game clamps SP at the cap during Star Power, which would change scores on some charts, and step 1 changes no scores. It goes to step 2.

### R4. Scores and path strings

No task in this plan may change a score, a path string or the list of paths, with one possible exception. Part B's Q1 (a mid-SP variant takes its leader's closing SqIn/SqOut, which changes those variants' strings) waits on the user. Every engine task's Verify compares against the R3 step-2 baseline.

### R5. Writing

Same as the brief: plain English, real test code, tests first, HEAD line numbers, trailers on every commit, staged by name. Where a revised task now depends on another part's accessor, name the accessor exactly as R1 defines it.


In these rulings, "Part B's Q1" is this plan's Q2 and "Part D's Q1" is Q4.

## Appendix B: accessors and hand-offs between parts

Part A's list of what it reads from the history (Task 8 uses Part D's names; see the note at the top of Task 8):


R1 stores `std::vector<SpEndStep> sp_end_steps` on `Activation`, with `SpEndStep{int64_t tick; int64_t end_tick; SpEndKind kind;}` and kinds `Activation`, `Collected`, `Clamped`, `SqIn`. Part A reads these accessors. The first three are R1's, named exactly as R1 defines them. The last four are Part A's, over the same list, in `core/model.h` / `model.cpp` beside R1's.

```cpp
// R1 (Part D's task defines them):
std::optional<int64_t> deact_tick() const;   // last step's end_tick
std::optional<int64_t> clamp_tick() const;   // tick of the last Clamped step
std::optional<int64_t> nominal_end() const;  // first step's end_tick

// Part A's, over the same list:

// The note whose timing moves the SP end that step `step` set: the tick of
// the latest Clamped step at or before it, else the activation tick
// (timecode.ticks()). D1. Precondition: step < sp_end_steps.size().
int64_t anchor_of_step(size_t step) const;

// The anchor of D: anchor_of_step(last step). Unset when the list is empty
// (an old record only).
std::optional<int64_t> deact_anchor_tick() const;

// The SP end squeeze `k` of sqinouts had its offset measured from.
// A SqIn: the end_tick of the step just before its own SqIn step. The n-th
// SqIn in sqinouts owns the n-th SqIn step. A SqOut: deact_tick().
// Unset when the list does not hold that step (an old record only).
std::optional<int64_t> squeeze_end_tick(size_t k) const;

// The anchor of that end: anchor_of_step(the step before the SqIn step) for
// a SqIn, deact_anchor_tick() for a SqOut. Unset together with
// squeeze_end_tick(k).
std::optional<int64_t> squeeze_anchor_tick(size_t k) const;
```

Part A needs three guarantees from R1's list. The orchestrator should check that Part D's task states them and tests them.

1. **Every SqIn has a `SqIn` step, in the same order as the SqIns in `sqinouts`.** That covers an early SqIn too, whose phrase is collected before the deactivation edge is reached. The step that collected that phrase must be marked `SqIn`, not `Collected`, once the path takes the SqIn branch (`Engine::branch_deactivate`, engine.cpp 665-677). Otherwise the n-th-to-n-th match breaks.
2. **The step before a SqIn step ends exactly at X**, the destination of the deactivation edge the path branched at, which the SqIn's offset was measured from (graph.cpp 235-236 late, 355-356 early). For a late SqIn that step is the previous step as-is. For an early SqIn it is the step before the phrase was collected.
3. **A SqIn phrase is never itself a `Clamped` step.** That holds today: a clamp leaves the path's SP end off the edge's `sqout_time`, so `deactivation_type` returns `DEACT_NONE` (engine.cpp 587-592, graph.cpp 270-275). Part A's tests pin it.

**The SqIn-plus-clamp case**, read through these accessors:

1. Clamp at C, then a SqIn. X was pinned to C, so the SqIn's anchor is C. D's anchor is C too, unless another clamp follows.
2. A SqIn, then a clamp at C. The step before the SqIn step has no Clamped step at or before it, so the SqIn's anchor is the activation. D's anchor is C.
3. Clamp at C1, SqIn, clamp at C2. The SqIn's anchor is C1; D's is C2. This is the case the single old `clamp_tick` could not tell apart, and the step list can.
4. The SqIn phrase as the clamping note: cannot happen (guarantee 3).

A SqOut is measured from D with D's anchor, the same as `transfer_post`.

### Is the per-SqIn scale derivable? Yes. It is still stored.

A SqIn's scale is a pure function of two stored ticks (`squeeze_end_tick`, `squeeze_anchor_tick`) and the chart's timing. So it is derivable. It is still stored, for the same reason `transfer_pre` and `transfer_post` are stored today (model.h lines 304-306): the details view often has no `SongTiming`. `build_activations` takes `const SongTiming*` and is called with `nullptr` (for example `tests/test_path_view.cpp:447`). The rule "display never recomputes" then needs the number on the record. This is not a second copy of a fact: nothing else stores the scale. It replaces `transfer_pre`, which stored one such scale per activation, with one per SqIn, so several SqIns are exact. A SqOut stores none, because its scale is `transfer_post`.

#### Handed to R1 (from the dropped A2 and A6), so nothing is lost

- `paths_json` (`tools/replay_json.cpp:84-93`) and `check_chart` (`tools/replay.cpp:767-778`) should print `act.nominal_end().value_or(-1)` under the unchanged key `nominal_deact_tick`. `video-tools\fcvideo.py` (lines 2505, 2753, 2789) and `analyze.py` (line 362) read that key. At HEAD, `hydra_replay dump` of Allister - Overrated prints 82560 and 167040 for path 0's two activations. That is a ready check.
- `sp_act_at` in `tests/test_preview_view.cpp:152-160` should take the end as a literal tick. Every call except one uses a flat 4/4 song at 480 ticks per beat. The calls are: `3840, 2 → 11520` (lines 755, 759, 837, 865, 903, 1021, 1044, 1397, 1422); `3840, 1 → 7680` (line 768); `3840, 4 → 19200` (line 947, overwritten to 23040 on the next line, as today); `2400, 1 → 6240` (line 1280); `1920, 1 → 5760` and `7680, 1 → 11520` (lines 1458, 1460). The randomized busy chart (line 1841) only tests lookups, so `t + 3840 * sp` with that comment is enough.
- The SqIn and clamp song fixtures above (`sqin_then_collect_song`, `clamp_song`, `wide_search`, `find_act`) are useful to R1's own tests. Whichever task lands first adds them; the other reuses them.

#### Notes for Part B and Part A (note only)

Part B (R3 step 7) owns variants. A variant folded while SP runs keeps its own steps up to the fold, then its leader's after. Its `deact_tick()`, `clamp_tick()` and `collected_phrase_ticks()` follow automatically, because they are reads of that one list. D1 keeps `Variant::end_tail` for this. A variant folded while inactive needs its own bank arrivals before the fold and its leader's after, in its first tail activation, and its own `trailing_bank_ticks` (finding 89: `leftover_sp()` follows, since it is that list's size). Its first tail activation's `skipped_fill_ticks` must be its own before the fold. D2 and D3 add no `bank_tail` or `skip_tail` to `Variant`; Part B adds them. When Part B lands, it extends the root-only order checks in D2 and D3 to variants.

Part A's A3 reads `squeeze_end_tick(i)` and `end_anchor_tick(*squeeze_end_step(i))` for each squeeze's scale, and `deact_tick()` with `end_anchor_tick(sp_end_steps.size() - 1)` for the final end. A2's `SPSqueeze::end_tick` and `anchor_tick`, A2's `transfer_anchor_tick`, and A6's `nominal_deact_tick` are not added. A7's "no fresh record stores unknown" covers the history through D1's corpus case.

## Appendix C: pieces of Part D's first draft that Tasks 5, 6, 20 and 21 cite

Read these only where a task points here. Where a piece conflicts with its task, the task wins: the task was revised under the rulings and this text was not.

#### From first-draft D1, Step 1 (shared fixtures)

- [ ] **Step 1: Add the shared fixtures.** Create `tests/sp_fixtures.h`:

```cpp
// Hand-built songs for the stored SP facts (Part D). Shared by the engine
// tests and the Preview tests so both read the same chart.
//
// The first two run at 240 BPM in 4/4 with 480 ticks a beat: a measure is
// 1920 ticks and 1000 ms, and one SP bar (two measures) burns 2000 ms. A
// note sits on every beat (480 ticks, 250 ms). Phrases end at 480 and 1920,
// so two bars are banked by 1000 ms. The first fill ends at 5760 (3000 ms);
// its spawn deadline is four beats before the fill, 1500 ms, which leaves
// 500 ms of room after the second phrase. Two bars run four measures, so that
// activation's plain SP end is 13440 (7000 ms).
#ifndef HYDRA_TESTS_SP_FIXTURES_H
#define HYDRA_TESTS_SP_FIXTURES_H

#include <algorithm>
#include <cstdint>
#include <vector>

#include "parse/song.h"

namespace hydra::test {

struct FixtureNote {
    int64_t tick;
    bool phrase = false;
    int64_t fill_length = 0;  // 0 = no fill ends here
};

inline Song build_fixture_song(int resolution, double bpm, const std::vector<FixtureNote>& notes) {
    Song song(resolution);
    song.bpm_changes[0] = bpm;
    song.build_timing();
    for (const FixtureNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        if (n.phrase) {
            ts.flag_sp = true;
            ts.sp_phrase_start = n.tick >= resolution ? n.tick - resolution : 0;
        }
        if (n.fill_length > 0) ts.activation_length = n.fill_length;
        song.sequence.push_back(std::move(ts));
    }
    return song;
}

// A note on every beat from 0 to `last`, with the phrase ends and fills given.
inline Song beat_song(const std::vector<int64_t>& phrases,
                      const std::vector<int64_t>& fills, int64_t last) {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= last; t += 480) {
        FixtureNote n{t};
        n.phrase = std::find(phrases.begin(), phrases.end(), t) != phrases.end();
        if (std::find(fills.begin(), fills.end(), t) != fills.end()) n.fill_length = 960;
        notes.push_back(n);
    }
    return build_fixture_song(480, 240.0, notes);
}

// The Epidermis shape (finding 5): the third phrase ends at 13920 (7250 ms),
// a quarter measure after the plain SP end. The player squeezes it in by
// hitting it early. The engine extends SP one bar from the old end, to 17280
// (9000 ms), so at the phrase the meter holds 1 - 0.25 / 2 = 0.875 bars.
inline Song make_late_sqin_song() {
    return beat_song({480, 1920, 13920}, {5760}, 21120);
}

// An early squeeze-out: the third phrase ends at 12960 (6750 ms), inside the
// first window. Hit late, it lands after SP ends and banks one bar instead.
// The fourth phrase, at 14400 (7500 ms), makes two bars for the second fill
// at 17280 (9000 ms). Search it with target ticks {5760, 17280}: only the
// squeeze-out path can take that second fill.
inline Song make_early_sqout_song() {
    return beat_song({480, 1920, 12960, 14400}, {5760, 17280}, 26880);
}

// Finding 52's case, at 120 BPM with 480 ticks a beat (tick t is t*500/480
// ms). Phrases end at 960 (1000 ms) and 18144 (18900 ms): SP is ready at
// 18900 ms. Fill A ends at 19200 (20000 ms) and is 480 ticks (500 ms) long.
// Fill B ends at 24960 (26000 ms) and is 3456 ticks (3600 ms) long. Fill C
// ends at 28800 (30000 ms). Under the 1.0 rule A's deadline is 18968.75 ms
// and B's is 18768.75 ms: A is shown and passed over, B never spawns. Search
// it with FillDeadlineRule::Ch10 and target ticks {28800}.
inline Song make_ch10_fill_song() {
    std::vector<FixtureNote> notes;
    for (int64_t t = 0; t <= 38400; t += 960) {
        FixtureNote n{t};
        n.phrase = t == 960;
        if (t == 19200 || t == 28800) n.fill_length = 480;
        if (t == 24960) n.fill_length = 3456;
        notes.push_back(n);
    }
    notes.push_back({18144, true, 0});
    std::sort(notes.begin(), notes.end(),
              [](const FixtureNote& a, const FixtureNote& b) { return a.tick < b.tick; });
    return build_fixture_song(480, 120.0, notes);
}

}  // namespace hydra::test

#endif  // HYDRA_TESTS_SP_FIXTURES_H
```

#### From first-draft D2, Step 1 (codec cases)

- [ ] **Step 1: Write the failing tests.** Append to `tests/test_search.cpp`:

```cpp
TEST_CASE("Bank rises: a squeezed-out bar arrives at the deact node") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    const std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    const Path& path = paths.front();
    REQUIRE(path.activations.size() == 2);
    CHECK((path.activations[0].bank_rise_ticks == std::vector<int64_t>{480, 1920}));
    // The phrase at 12960 was hit late, just after SP ended at 13440: its
    // bar arrives there. Then the phrase at 14400.
    CHECK((path.activations[1].bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
    CHECK(path.trailing_bank_ticks.empty());
    CHECK(path.leftover_sp == 0);
}
```

In D1's corpus case, track the previous activation per root and add these checks inside the activation loop (declare `std::optional<Activation> prev;` before it and set `prev = act;` at its end):

```cpp
                CHECK(act.bank_rise_ticks.size() == static_cast<size_t>(act.sp_meter));
                const int64_t floor = prev ? *prev->deact_tick : std::numeric_limits<int64_t>::min();
                for (size_t k = 0; k < act.bank_rise_ticks.size(); ++k) {
                    const int64_t t = act.bank_rise_ticks[k];
                    CHECK(t >= floor);
                    CHECK(t <= act.timecode.ticks());
                    if (k > 0) CHECK(t >= act.bank_rise_ticks[k - 1]);
                }
                if (prev && prev->sqout_tick) {
                    REQUIRE_FALSE(act.bank_rise_ticks.empty());
                    CHECK(act.bank_rise_ticks.front() ==
                          std::max(*prev->deact_tick, *prev->sqout_tick));
                }
```

After the activation loop: `CHECK(root.trailing_bank_ticks.size() == static_cast<size_t>(root.leftover_sp));`. These ordering checks run on root paths only. A variant's tail activations are its leader's, and their order against the variant's own windows is Part B's job (see section 3, Notes for Part B).

Append to `tests/test_path_codec.cpp`:

```cpp
TEST_CASE("path codec: a node keeps bank_rise_ticks") {
    Activation act;
    act.timecode = Timecode::raw(17280);
    act.bank_rise_ticks = {13440, 14400};
    Path path;
    path.activations.push_back(act);
    const Path back = decode_path_node(encode_path_node(path));
    CHECK((back.activations.front().bank_rise_ticks == std::vector<int64_t>{13440, 14400}));
}

TEST_CASE("path codec: a root keeps trailing_bank_ticks") {
    HydraRecord rec = fixture().record;
    rec.paths.front().trailing_bank_ticks = {111, 222};
    rec.paths.front().leftover_sp = 2;
    const HydraRecord back = rebuild_record(flatten_record(rec));
    CHECK((back.paths.front().trailing_bank_ticks == std::vector<int64_t>{111, 222}));
}
```

#### First-draft Task D4: The SP gauge draws only stored facts

**Goal:** With a path, `build_sp_meter_curve` reads the bank rises, the SP end steps and the deact node, and nothing else; CONTEXT.md says so without contradicting itself.

**Findings:** 5, 147, 159.

**Decision:** Q4 governs how a late squeeze-in is drawn (recommended answer (b) is what the code below does). The standing rule governs the rest. D4 governs the guard: an activation with no steps is a bug a test must rule out, not a state to draw around.

Why this task: today the gauge is a second engine. With D1 and D2 the record holds every value the gauge shows. Between windows the gauge steps one bar at each stored rise. Inside a window it draws the measures left until the SP end in force, two measures to a bar, cutting at each stored step and at each tempo and meter change. Every comparison is in ticks. The ms values are used only to place the drawing.

Without a path there is no record, so the chart-only gauge stays as it is (Q10). It moves into its own function so the path gauge can be checked by a source scan.

**Files:**
- Modify: `src/app/preview_view.h:75-103` (PreviewActivation gets `sp_end_steps` and `bank_rise_ticks`), `194-223` (PreviewScene gets `trailing_bank_ticks`; the `sp_meter` comment at 205-212 is rewritten).
- Modify: `src/app/preview_view.cpp:54-213` (the gauge), `327-360` (overlay copies the facts), `400-403` (which gauge to build).
- Modify: `CONTEXT.md:206-214` (SP meter gauge entry).
- Test: `tests/test_preview_view.cpp` (new cases after line 1128; fixture edits at 152-160, 732-765, 832-856, 858-892, 894-922, 924-975, 1065-1083, 1417-1431; `scene_difference` at 635-645; the old-scan comparison at 1715-1789 and 1857-1902).

**Acceptance Criteria:**
- [ ] On the late squeeze-in song, the gauge reads 0 just before 7000 ms, 1.0 at 7000 ms, 0.875 at 7250 ms (readout "0.9/4"), 0.5 at 8000 ms and 0 at 9000 ms.
- [ ] A stored bank rise where the chart has no phrase still steps the gauge, and a stored step where no phrase is collected still moves the drain: the gauge follows the record, not the chart.
- [ ] The early squeeze-out song shows the squeezed-out bar arriving at 7000 ms, the deact node, and no step at the phrase (6750 ms).
- [ ] On every corpus path the gauge's value equals the old scan's at every boundary and 300 random times, except inside late squeeze-in windows.
- [ ] The body of `build_sp_meter_curve` names none of `sp_phrases`, `collected_phrase_ticks`, `act.sp_meter`, `sp_bars_to_measures`, `std::min(`.
- [ ] CONTEXT.md's entry says the gauge reads the record and no longer describes the set-difference rule.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="sp meter curve*,drain box*,base + overlay*,preview lookups*,sp meter readout*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
```

**Steps:**

- [ ] **Step 1: Write the failing tests.** In `tests/test_preview_view.cpp` add `#include <fstream>`, `#include <sstream>` and `#include "sp_fixtures.h"`, then append after line 1128:

```cpp
TEST_CASE("sp meter curve: a late squeeze-in refills at the old SP end") {
    // Finding 5. The player hits the late phrase early, so SP never stops:
    // the engine extends one bar from the old end (7000 ms). At the phrase
    // (7250 ms) a quarter measure of that bar is gone: 0.875, not 1.0.
    Song song = test::make_late_sqin_song();
    ScoreGraph graph(song, 4);
    std::vector<Path> paths = run_search(graph, EngineOptions{});
    REQUIRE(!paths.empty());
    const Path& best = paths.front();
    REQUIRE(best.activations.size() == 1);
    REQUIRE(best.activations.front().deact_tick == std::optional<int64_t>(17280));

    PreviewScene scene = build_preview_scene(song, &best);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 3000.0) == doctest::Approx(2.0));   // the activation
    CHECK(sp_meter_bars_at(c, 5000.0) == doctest::Approx(1.0));   // a bar per 2000 ms
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // the refill, at the old end
    CHECK(sp_meter_bars_at(c, 7250.0) == doctest::Approx(0.875)); // the late phrase
    CHECK(sp_meter_readout(c, 7250.0) == "0.9/4");
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(0.5));
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(0.0));   // the deact node
    CHECK(sp_meter_bars_at(c, 9500.0) == doctest::Approx(0.0));   // collected: nothing banks after
}

TEST_CASE("sp meter curve: a squeezed-out bar arrives at the deact node") {
    Song song = test::make_early_sqout_song();
    ScoreGraph graph(song, 4);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{5760, 17280};
    std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    PreviewScene scene = build_preview_scene(song, &paths.front());
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    // Straight through the squeezed-out phrase (6750 ms): no step there.
    CHECK(sp_meter_bars_at(c, 6750.0 - 1e-6) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 6750.0) == doctest::Approx(0.125));
    CHECK(sp_meter_bars_at(c, 7000.0 - 1e-6) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 7000.0) == doctest::Approx(1.0));   // its bar, as SP ends
    CHECK(sp_meter_bars_at(c, 7500.0) == doctest::Approx(2.0));   // the next phrase
    CHECK(sp_meter_bars_at(c, 9000.0) == doctest::Approx(2.0));   // the second activation
    CHECK(sp_meter_bars_at(c, 11000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: the bank between windows is the record's, not a phrase count") {
    // The chart has one phrase, at 960 (1000 ms). The record says bars
    // arrived at 960 and at 1920 (2000 ms), and one more after the window at
    // 12480 (13000 ms). Neither of the last two has a phrase. The gauge
    // follows the record.
    Song song = make_sp_song({960}, /*last_tick=*/15360);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2);
    act.bank_rise_ticks = {960, 1920};
    path.activations = {act};
    path.trailing_bank_ticks = {12480};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 999.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 1000.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 1999.0) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 2000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 12000.0) == doctest::Approx(0.0));
    CHECK(sp_meter_bars_at(c, 13000.0) == doctest::Approx(1.0));
}

TEST_CASE("sp meter curve: the drain follows the stored steps, not the collected list") {
    // The record moves the end at 7680 (8000 ms) from 11520 to 15360 and
    // lists no collected phrase. The gauge still steps there.
    Song song = make_sp_song({960}, /*last_tick=*/17280);
    Path path;
    Activation act = sp_act_at(song, 3840, /*sp_meter=*/2);
    act.sp_end_steps = {{3840, 11520}, {7680, 15360}};
    act.deact_tick = 15360;
    path.activations = {act};
    PreviewScene scene = build_preview_scene(song, &path);
    const SpMeterCurve& c = scene.sp_meter;
    check_curve_well_formed(c);
    CHECK(sp_meter_bars_at(c, 8000.0 - 1e-6) == doctest::Approx(1.0));
    CHECK(sp_meter_bars_at(c, 8000.0) == doctest::Approx(2.0));
    CHECK(sp_meter_bars_at(c, 16000.0) == doctest::Approx(0.0));
}

TEST_CASE("sp meter curve: the path gauge reads stored facts only") {
    // Findings 147 and 159, held in place: the path gauge's own body never
    // touches the chart's phrases, the collected list, the bank count or the
    // cap rule. A copy of any of them coming back fails here.
    std::ifstream in(std::string(HYDRA_SOURCE_DIR) + "/src/app/preview_view.cpp");
    REQUIRE(in);
    std::stringstream ss;
    ss << in.rdbuf();
    const std::string src = ss.str();
    const size_t begin = src.find("SpMeterCurve build_sp_meter_curve(");
    REQUIRE(begin != std::string::npos);
    const size_t end = src.find("\n}\n", begin);
    REQUIRE(end != std::string::npos);
    const std::string body = src.substr(begin, end - begin);
    for (const char* banned : {"sp_phrases", "collected_phrase_ticks", "act.sp_meter",
                               "sp_bars_to_measures", "std::min("}) {
        INFO(banned);
        CHECK(body.find(banned) == std::string::npos);
    }
}
```

- [ ] **Step 2: Update the existing fixtures to state the stored facts.** Each hand-built activation now has to say what the engine would have stamped. In `sp_act_at` (lines 152-160), after setting `deact_tick`:

```cpp
    a.sp_end_steps = {{tick, *a.deact_tick}};
```

Then, in the cases that collect or squeeze out, add the facts the engine would stamp:
- "a phrase collected mid-activation" (858-892) and the split fixture at 755-757 and "drain box: empties in reads the stored end" (1422-1424): add `act.sp_end_steps.push_back({5760, 15360});`.
- "a full bank that collects a phrase" (941-951): `act.sp_end_steps = {{act_tick, act_tick + 8 * 1920}, {phrase_tick, deact}};`.
- "a squeezed-out phrase does not bank mid-drain" (901-903): `path.trailing_bank_ticks = {11520};` (its bar arrives at the deact node).
- "an activation snaps to the recorded bars, then drains" (832-856): delete it. The new "bank between windows is the record's" case replaces it; there is no snap any more.
- "an activation with no recorded bars empties the meter at once" (1065-1083): rename to "an activation the record stamped nothing on draws nothing" and change its expectations to `0.0` at 3999, 4000, 5999 and 6000 ms. Nothing was stamped, so nothing is drawn; the corpus case of D1 proves a fresh record always stamps.

In `scene_difference` (635-645) compare the new members: `x.sp_end_steps == y.sp_end_steps && x.bank_rise_ticks == y.bank_rise_ticks`, and add `if (a.trailing_bank_ticks != b.trailing_bank_ticks) return "trailing_bank_ticks";` after the activations block.

- [ ] **Step 3: Compare with the old scan by value, outside late squeeze-in windows.** The old gauge and the new one cut the curve at different places, so segment-by-segment equality no longer applies. Replace the segment loop in `check_lookups_match_old_scans` (lines 1729-1740) with:

```cpp
    if (compare_gauge) {
        const SpMeterCurve want_curve = old_scan::sp_meter_curve(scene, *scene.timing, sp_cap);
        CHECK(scene.sp_meter.cap == want_curve.cap);
        // A window whose end moved on the old end itself is a late squeeze-in:
        // the old scan drew it wrong (finding 5), so it is left out.
        auto in_late_sqin_window = [&](double ms) {
            for (const PreviewActivation& a : scene.activations) {
                bool late = false;
                for (size_t k = 1; k < a.sp_end_steps.size(); ++k)
                    if (a.sp_end_steps[k].tick >= a.sp_end_steps[k - 1].end_tick) late = true;
                if (late && a.ms <= ms && ms <= a.sp_end_ms) return true;
            }
            return false;
        };
        std::vector<double> probe;
        for (const SpMeterCurve* cv : {&scene.sp_meter, &want_curve})
            for (const SpMeterSegment& s : cv->segments)
                for (double ms : {s.start_ms, s.end_ms})
                    for (double t : {ms, std::nextafter(ms, -1e300), std::nextafter(ms, 1e300)})
                        probe.push_back(t);
        std::uniform_real_distribution<double> any(-1000.0, scene.song_length_ms + 3000.0);
        for (int i = 0; i < 300; ++i) probe.push_back(any(rng));
        for (double t : probe) {
            if (in_late_sqin_window(t)) continue;
            CAPTURE(t);
            CHECK(sp_meter_bars_at(scene.sp_meter, t) ==
                  doctest::Approx(sp_meter_bars_at(want_curve, t)).epsilon(1e-9));
        }
    }
```

Give `check_lookups_match_old_scans` a `bool compare_gauge` parameter. The busy synthetic chart passes `false`: its activations are made up and carry no engine facts. The corpus case passes `true`, which is the real proof that nothing else on screen moved.

The old scan still needs each activation's collected phrases, and `PreviewActivation` no longer carries them. So `old_scan::sp_meter_curve` takes them from the path instead: add a parameter `const std::vector<std::vector<int64_t>>& collected` (one list per activation) and read `collected[i]` where it read `act.collected_phrase_ticks` (lines 1663-1669). `check_lookups_match_old_scans` gets a `const Path*` and builds that list from `path->walk_activations()`; the corpus case (line 1899) passes `&p` with `compare_gauge` true. The corpus no-path call (line 1896) passes `nullptr` with `compare_gauge` true too: with no activations the old scan needs no lists, and it proves the chart-only gauge is unchanged. The busy-chart calls pass their path with `compare_gauge` false.

- [ ] **Step 4: Run them and see them fail.** The build fails on the missing PreviewActivation and PreviewScene members. Once Step 5's header change is in, the late squeeze-in case fails at 7000 ms (reads about 0.118, wants 1.0) and at 7250 ms (reads 1.0, wants 0.875), and the two "record, not chart" cases fail. That is the red.

- [ ] **Step 5: Carry the facts into the scene.** In `src/app/preview_view.h`, replace the `collected_phrase_ticks` member of `PreviewActivation` (lines 88-93) with:

```cpp
    // Copied from the record (Activation::sp_end_steps and bank_rise_ticks):
    // where this window's SP end was set, and where each bar it spends
    // arrived. The gauge draws only from these.
    std::vector<SpEndStep> sp_end_steps;
    std::vector<int64_t> bank_rise_ticks;
```

Add to `PreviewScene` after `activations` (line 199):

```cpp
    // Overlay: where each bar banked after the path's last window arrived
    // (Path::trailing_bank_ticks). Empty without a path.
    std::vector<int64_t> trailing_bank_ticks;
```

Rewrite the comment on `sp_meter` (205-212):

```cpp
    // Banked SP over time, for the meter gauge. With a path every value is
    // the record's: the bank steps at each stored bar arrival, and each
    // window drains from the stored SP end steps to the deact node. Without a
    // path there is no record, so it fills one bar per phrase and pins at the
    // cap: that chart-only view has nothing to spend the bank. Empty when the
    // chart has no SP phrase and the path stamps no bar.
```

In `apply_preview_overlay`, clear `scene.trailing_bank_ticks` with the other overlay fields (line 331-334). In the activation copy (line 346) replace `pa.collected_phrase_ticks = a.collected_phrase_ticks;` with:

```cpp
            pa.sp_end_steps = a.sp_end_steps;
            pa.bank_rise_ticks = a.bank_rise_ticks;
```

and after the activation loop add `scene.trailing_bank_ticks = path->trailing_bank_ticks;`. At line 403:

```cpp
    scene.sp_meter = path != nullptr ? build_sp_meter_curve(scene, timing, sp_cap)
                                     : build_unanalyzed_sp_meter_curve(scene, sp_cap);
```

- [ ] **Step 6: Rewrite the gauge.** Replace lines 54-213 of `src/app/preview_view.cpp` (DrainSplit through the end of `build_sp_meter_curve`) with:

```cpp
void push_segment(SpMeterCurve& curve, double start_ms, double end_ms,
                  double start_bars, double end_bars) {
    if (end_ms <= start_ms) return;  // a zero-width stretch draws nothing
    curve.segments.push_back({start_ms, end_ms, start_bars, end_bars});
}

// The flat last segment. Always emitted, even at zero width: it is what
// sp_meter_bars_at reads back as the value after the curve ends.
void close_curve(SpMeterCurve& curve, double cursor_ms, double song_length_ms, double bank) {
    curve.segments.push_back({cursor_ms, std::max(cursor_ms, song_length_ms), bank, bank});
}

// The SP meter for a chart with no path: no record, so nothing spends the
// bank. It fills one bar at each phrase's last note and pins at the cap
// (CONTEXT.md, "SP meter gauge").
SpMeterCurve build_unanalyzed_sp_meter_curve(const PreviewScene& scene, int sp_cap) {
    SpMeterCurve curve;
    curve.cap = sp_cap < 1 ? 1 : sp_cap;
    if (scene.sp_phrases.empty()) return curve;
    const double cap = static_cast<double>(curve.cap);
    double bank = 0.0;
    double cursor_ms = 0.0;
    for (const PreviewSpan& p : scene.sp_phrases) {
        push_segment(curve, cursor_ms, p.end_ms, bank, bank);
        cursor_ms = std::max(cursor_ms, p.end_ms);
        bank = std::min(bank + 1.0, cap);
    }
    close_curve(curve, cursor_ms, scene.song_length_ms, bank);
    return curve;
}

// The SP meter along a path, drawn only from what the record stamped:
// each bar's arrival between windows (bank_rise_ticks, trailing_bank_ticks),
// and each window's SP end steps up to its deact node. It counts no phrase
// and applies no cap; the engine already did both. Every comparison is in
// ticks; ms only places the drawing.
SpMeterCurve build_sp_meter_curve(const PreviewScene& scene, const SongTiming& timing,
                                  int sp_cap) {
    SpMeterCurve curve;
    curve.cap = sp_cap < 1 ? 1 : sp_cap;
    if (scene.activations.empty() && scene.trailing_bank_ticks.empty()) return curve;

    double bank = 0.0;
    double cursor_ms = 0.0;
    // Flat, one bar up at each stamped arrival.
    auto bank_bars = [&](const std::vector<int64_t>& rises) {
        for (int64_t t : rises) {
            const double ms = timing.ms_index().at(t);
            push_segment(curve, cursor_ms, ms, bank, bank);
            cursor_ms = std::max(cursor_ms, ms);
            bank += 1.0;
        }
    };
    // Bars left at `tick` while SP ends at `end_tick`: the measures between,
    // two to a bar. Never below empty, though a fresh record never asks
    // (SP end steps: every corpus activation is consistent).
    auto bars_left = [&](int64_t tick, int64_t end_tick) {
        const double measures = timing.measures_at_tick_f(static_cast<double>(end_tick)) -
                                timing.measures_at_tick_f(static_cast<double>(tick));
        return std::max(0.0, measures / static_cast<double>(kMeasuresPerSpBar));
    };

    // Tempos and meters are sorted by tick and the activations come in time
    // order, so one moving index per list finds each window's first change.
    size_t next_tempo = 0;
    size_t next_meter = 0;
    for (const PreviewActivation& act : scene.activations) {
        bank_bars(act.bank_rise_ticks);
        push_segment(curve, cursor_ms, act.ms, bank, bank);
        cursor_ms = std::max(cursor_ms, act.ms);
        bank = 0.0;  // the activation spends the bank
        // Nothing stamped (a hand-built activation): no window to draw.
        if (act.sp_end_steps.empty() || !act.has_sp_end) continue;

        // Cut at each stored step and at each tempo and meter change inside
        // the window, where measures stop being linear in ms.
        struct Cut {
            int64_t tick;
            double ms;
        };
        std::vector<Cut> cuts;
        for (size_t k = 1; k < act.sp_end_steps.size(); ++k) {
            const int64_t t = act.sp_end_steps[k].tick;
            cuts.push_back({t, timing.ms_index().at(t)});
        }
        const std::vector<PreviewTempo>& tempos = scene.tempos;
        while (next_tempo < tempos.size() && tempos[next_tempo].tick <= act.tick) ++next_tempo;
        for (size_t i = next_tempo; i < tempos.size() && tempos[i].tick < act.sp_end_tick; ++i)
            cuts.push_back({tempos[i].tick, tempos[i].ms});
        const std::vector<PreviewMeter>& meters = scene.meters;
        while (next_meter < meters.size() && meters[next_meter].tick <= act.tick) ++next_meter;
        for (size_t i = next_meter; i < meters.size() && meters[i].tick < act.sp_end_tick; ++i)
            cuts.push_back({meters[i].tick, timing.ms_index().at(meters[i].tick)});
        std::stable_sort(cuts.begin(), cuts.end(),
                         [](const Cut& a, const Cut& b) { return a.tick < b.tick; });

        size_t step = 0;  // the step in force
        int64_t prev_tick = act.tick;
        double prev_ms = act.ms;
        for (const Cut& cut : cuts) {
            const int64_t end_tick = act.sp_end_steps[step].end_tick;
            push_segment(curve, prev_ms, cut.ms, bars_left(prev_tick, end_tick),
                         bars_left(cut.tick, end_tick));
            while (step + 1 < act.sp_end_steps.size() &&
                   act.sp_end_steps[step + 1].tick <= cut.tick)
                ++step;
            prev_tick = cut.tick;
            prev_ms = cut.ms;
        }
        const int64_t end_tick = act.sp_end_steps[step].end_tick;
        push_segment(curve, prev_ms, act.sp_end_ms, bars_left(prev_tick, end_tick),
                     bars_left(act.sp_end_tick, end_tick));
        cursor_ms = std::max(cursor_ms, act.sp_end_ms);
    }

    bank_bars(scene.trailing_bank_ticks);
    close_curve(curve, cursor_ms, scene.song_length_ms, bank);
    return curve;
}
```

Update the comment on `SpMeterSegment` in `preview_view.h` (lines 148-152) to say the curve is cut "at every tempo change, meter change, stored SP end step, activation and deact node".

- [ ] **Step 7: Fix the CONTEXT.md entry.** Replace lines 206-214 with:

```markdown
**SP meter gauge**:
The vertical gauge on the note highway's right edge showing banked Star Power
at the playhead. With a path, every value is the record's. Between activations
it steps up one bar at each tick where the engine stamped a bar's arrival. In
an active window it shows the measures left until the SP end in force, two
measures to a bar, and the record lists every place that end moved. It empties
exactly at the deact node. The gauge counts no phrases and applies no cap. A
late squeeze-in refills at the old SP end, because the player hits that phrase
early and SP never stops. A squeezed-out phrase's bar arrives when the player
hits it: as SP ends for an early phrase, on its own note for a late one.
Without a path it fills one bar per phrase and pins at the cap, since nothing
spends it and there is no record to read.
```

- [ ] **Step 8: Run everything.** Run the Verify block. The corpus comparison prints no failures. If any value outside a late squeeze-in window differs, stop: that is an unplanned visible change, and it goes back to the user.

- [ ] **Step 9: Commit.** Stage by name: `src/app/preview_view.h src/app/preview_view.cpp CONTEXT.md tests/test_preview_view.cpp`. Commit "Preview SP gauge draws only stored engine facts".

#### First-draft Task D5: The Preview's offered fills read the stored list

**Goal:** The Preview lights as offered exactly the fills the engine stored on each activation.

**Findings:** 52.

**Decision:** Q9 (the visible change under the 1.0 rule). Standing rule otherwise.

Why this task: the nearest-n walk in `apply_preview_overlay` (lines 367-397) is a guess. D3 stored the answer. The Preview now marks each stored tick's fill as offered and the activation's own fill as taken. Everything else stays hidden, as today. Under the 1.1 rule the guess and the answer always agree, so only 1.0 records change.

**Files:**
- Modify: `src/app/preview_view.h:75-103` (PreviewActivation: `skips` becomes `skipped_fill_ticks`; header comment at 62-66).
- Modify: `src/app/preview_view.cpp:345, 362-398`.
- Modify: `CONTEXT.md:196-204` (Path overlay entry).
- Test: `tests/test_preview_view.cpp` (`act_at` at 106-111; cases at 303-338, 732-748, 1065-1083, 1433-1445; `old_scan::fill_states` at 1591-1616; `make_busy_chart` at 1832-1851; `scene_difference` at 638-639; a new case).

**Acceptance Criteria:**
- [ ] On the 1.0-rule fill song, fill A (19200) is Offered, fill B (24960) Hidden, fill C (28800) Taken.
- [ ] The existing fill cases give the same states as today with stored ticks in place of counts.
- [ ] On every corpus path (default 1.1 rule) the fill states equal the old nearest-n scan's.
- [ ] CONTEXT.md's Path overlay entry says offered fills come from the stored list.

**Verify:**
```powershell
.\build_cpp.ps1 -Target hydra_tests
.\build-cpp\Release\hydra_tests.exe -tc="build_preview_scene*,base + overlay*,preview lookups*"
# expect: [doctest] Status: SUCCESS!
.\build-cpp\Release\hydra_tests.exe
# expect: [doctest] Status: SUCCESS!
```

**Steps:**

- [ ] **Step 1: Write the failing test.** Append to `tests/test_preview_view.cpp`:

```cpp
TEST_CASE("build_preview_scene: under the 1.0 rule the offered fill is the one the engine charged") {
    // Finding 52. Fill B's 1.0 deadline falls before fill A's, so B never
    // spawned and A was shown and passed over. The nearest-n guess lit B.
    Song song = test::make_ch10_fill_song();
    ScoreGraph graph(song, 4, FillDeadlineRule::Ch10);
    EngineOptions opts;
    opts.target_act_ticks = std::vector<int64_t>{28800};
    std::vector<Path> paths = run_search(graph, opts);
    REQUIRE(!paths.empty());
    PreviewScene scene = build_preview_scene(song, &paths.front());
    REQUIRE(scene.fills.size() == 3);
    CHECK(scene.fills[0].span.end_tick == 19200);
    CHECK(scene.fills[0].state == PreviewFillState::Offered);
    CHECK(scene.fills[1].state == PreviewFillState::Hidden);
    CHECK(scene.fills[2].state == PreviewFillState::Taken);
}
```

- [ ] **Step 2: Update the fill fixtures.** `act_at` takes the passed-over fills instead of a count:

```cpp
Activation act_at(const Song& song, int64_t tick, std::vector<int64_t> skipped_fills) {
    Activation a;
    a.timecode = song.timecode(tick);
    a.skips = static_cast<int>(skipped_fills.size());
    a.skipped_fill_ticks = std::move(skipped_fills);
    return a;
}
```

The calls become: line 307 `act_at(song, 1440, {960})`; line 321 `act_at(song, 960, {}), act_at(song, 1920, {})`; line 333 `act_at(song, 960, {}), act_at(song, 1920, {1440})`; lines 743-744 the same two shapes; lines 774, 1070 and 1437 `act_at(song, <tick>, {})`.

In `old_scan::fill_states` (line 1604) read `int left = static_cast<int>(a.skipped_fill_ticks.size());`. In `make_busy_chart` (lines 1841-1842), replace `a.skips = skips(rng);` with the nearest-n ticks the old scan would light, so the two still agree on made-up data:

```cpp
        std::vector<int64_t> before;  // fills after the previous activation, before this one
        for (const SongTimestamp& f : song.sequence) {
            const int64_t ft = f.timecode.ticks();
            if (f.activation_length && ft > prev_act && ft < t) before.push_back(ft);
        }
        const int n = std::min(skips(rng), static_cast<int>(before.size()));
        a.skipped_fill_ticks.assign(before.end() - n, before.end());
        a.skips = n;
        prev_act = t;
```

(declare `int64_t prev_act = std::numeric_limits<int64_t>::min();` next to `after`). In `scene_difference` replace `x.skips == y.skips` with `x.skipped_fill_ticks == y.skipped_fill_ticks`.

- [ ] **Step 3: Run them and see them fail.** The build fails on `PreviewActivation::skipped_fill_ticks`. With the header in place, the new case fails: fill B reads Offered and A Hidden.

- [ ] **Step 4: Read the stored list.** In `src/app/preview_view.h`, replace `int skips = 0;` (line 80) with:

```cpp
    // The fills the path was shown and passed over before this activation,
    // copied from the record (Activation::skipped_fill_ticks).
    std::vector<int64_t> skipped_fill_ticks;
```

and change the enum comment (62-66) to "read off the path's stored passed-over fills". In `apply_preview_overlay`, line 345 becomes `pa.skipped_fill_ticks = a.skipped_fill_ticks;`, and lines 366-398 become:

```cpp
    } else {
        // Which fills the game showed is engine truth: each activation lists
        // the fills its path passed over, and its own fill is the taken one.
        // Every other fill stays hidden, including those after the last
        // activation, which the engine records nothing about. Fills and the
        // stored ticks are both in chart order, so one index walks them.
        std::vector<PreviewFill>& fills = scene.fills;
        size_t next_fill = 0;
        auto mark = [&](int64_t tick, PreviewFillState state) {
            while (next_fill < fills.size() && fills[next_fill].span.end_tick < tick) ++next_fill;
            if (next_fill < fills.size() && fills[next_fill].span.end_tick == tick)
                fills[next_fill].state = state;
        };
        for (const PreviewActivation& a : scene.activations) {
            for (int64_t t : a.skipped_fill_ticks) mark(t, PreviewFillState::Offered);
            mark(a.tick, PreviewFillState::Taken);
        }
    }
```

- [ ] **Step 5: Fix the CONTEXT.md entry.** In "Path overlay" (lines 202-204) replace "Offered fills come from the activation's skip count, not a re-derived SP meter; fills after the last activation are hidden because the engine records nothing about them." with "Offered fills are the ones the engine stored on each activation as passed over; nothing guesses them from a count. Fills after the last activation are hidden because the engine records nothing about them."

- [ ] **Step 6: Run everything.** Run the Verify block. The corpus fill comparison passes: under 1.1 the stored list and the old guess agree on every path.

- [ ] **Step 7: Commit.** Stage by name. Commit "Preview lights the offered fills the engine stored".

## Appendix D: Part A's first-draft fixtures (cited by Task 5)

- [ ] **Step 1: Failing tests.** In `tests/test_search.cpp`, inside the anonymous namespace after `build_tail_song` (line 406), add a tempo-aware builder:

```cpp
// build_tail_song with a tempo map: 192 ticks per beat, 768 per measure.
Song build_tempo_song(const std::vector<TailNote>& notes,
                      const std::map<int64_t, double>& bpm) {
    Song song(192);
    song.tpm_changes[0] = 768;
    for (const auto& kv : bpm) song.bpm_changes[kv.first] = kv.second;
    song.build_timing();
    for (const TailNote& n : notes) {
        SongTimestamp ts;
        ts.timecode = song.timecode(n.tick);
        ts.chord.add_note(NoteColor::Red);
        ts.flag_sp = n.sp_phrase;
        if (n.activation) ts.activation_length = 384;
        song.sequence.push_back(ts);
    }
    return song;
}

// The SqIn shape finding 27 is about. Two phrases bank 2 bars; the fill at
// 2304 activates, so SP ends at X = 2304 + 4 measures = 5376. The phrase at
// 5280 sits 250 ms before X: collecting it is the SqIn, and moves the end to
// 6912. The phrase at 6144 is then collected mid-SP and moves it to 8448.
// The tempo halves at 6000, so X (120 BPM) and D (60 BPM) differ, and the old
// "D minus two measures" rebuild (6912, 60 BPM) is wrong.
Song sqin_then_collect_song() {
    return build_tempo_song({{0, true},  {768, true},  {1536},       {2304, false, true},
                             {3072},     {3840},       {4608},       {5280, true},
                             {5376},     {6144, true}, {6912},       {7680},
                             {8448},     {8544}},
                            {{0, 120.0}, {6000, 60.0}});
}

// The clamp shape of finding 28, at an SP cap of 2. The meter is full when
// the activation at 2304 starts, so collecting the phrase at 3072 clamps the
// end to plusmeasure(3072, 4) = 6144 instead of 6912. The tempo halves at
// 2700, between the activation and the collecting note.
Song clamp_song() {
    return build_tempo_song({{0, true}, {768, true}, {1536}, {2304, false, true},
                             {3072, true}, {3840}, {4608}, {5376}, {6144}, {6912}},
                            {{0, 120.0}, {2700, 60.0}});
}

// Keep every path, not only the best score (EngineOptions' default depth is
// 0), so a SqOut or SqIn branch the fixture creates is in the output even
// when it is not optimal.
EngineOptions wide_search() {
    EngineOptions o;
    o.depth_mode = DepthMode::Points;
    o.depth_value = 1000000;
    return o;
}

// The first activation, over every output path, that matches `pred`.
template <typename Pred>
const Activation* find_act(const std::vector<Path>& paths, Pred pred) {
    for (const Path& p : paths)
        for (const Activation& a : p.all_activations())
            if (pred(a)) return &a;
    return nullptr;
}
```

## Appendix E: literal SP ends for test fixtures

See the `sp_act_at` item in Appendix B's "Handed to R1" list.
