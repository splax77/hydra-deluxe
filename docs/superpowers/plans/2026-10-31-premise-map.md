# Plan: every plan proves its premises (the Premise map)

Written 2026-10-31 by planning agent a674b1c8bb5277e69 (task PREMISE-MAP-PLAN, session 90d40bc9). Waits for the user's yes before any task starts (agent rule 3).

## The plain version

Four sessions in a row lost live runs or built the wrong thing because a plan stated a fact nobody had opened the source of. The project already makes a plan prove every promise it makes (the Promise map in `C:\Users\Patrick\.claude\agents\planner.md`). It never asks a plan to prove the facts it stands on. This plan adds that second half: a Premise map, one row per fact the plan depends on, each with the exact place the fact was read, and a test that fails a plan without one.

A premise is a fact the plan uses but does not build. "A kit on Player1 makes a drums player." "A command-line launch always means a bot." "Window Map has Star Power phrases." Each of those was false, and each sat in a plan as a flat sentence. The rule from now on: a fact either names its source (a code line, a game data file, a chart file, a run's result file) or it is marked unchecked, and an unchecked fact can only become a check task or a question for the user. It never carries into a task or a live step as settled.

The four misses, and where each is checked, are in the Premise map at the end of this plan (rows P1 to P4). This is the first plan that carries one, so it holds itself to its own rule.

Five changes do the work. The planner's instructions ask for the Premise map and a call list per live step. The reviewer's instructions make the reviewer open every row's source and then read the live steps cold for premises the map left out. A new agent page in Hydra, `docs/agents/premise-map.md`, is the one home of the table's shape, the way `docs/agents/handoff.md` is for the Claims table. A new check in the docs test fails a plan that breaks the shape. And the live-run page tells the main session to show the user each step's call list, with its never-tried pieces, in the go request.

## Rulings that bind this plan

- "this is now four sessions in a row that have faced the same issue: blindly accepting the plan's framing without verifying against the underlying code. this is becoming a bigger problem. how do we address this?" and "Yes, plan it" (the user, session 90d40bc9, 2026-10-31). The five approved points are repeated under "What the user approved".
- D107 (`docs/audit/2026-10-03-fix-decisions.md`): one fresh reviewer per change, which fixes and signs off.
- Agent rules 1, 2, 3, 5 and 9 (`docs/agents/agent-rules.md`).

## What the user approved

1. Every plan carries a Premise map, the twin of the Promise map: one row per fact the plan depends on, with the kind of source and its exact location. A fact whose only source is another plan, a handoff, a doc, a memory note or nothing is marked unchecked and must become a check task or a user question. It never carries into a task or a live step as settled.
2. Every live step lists every tool call it makes, each marked seen working on this exact target (with the run) or never tried. A first run never chains several never-tried pieces, so a failure names one piece.
3. The plan reviewer opens each Premise map row's source and confirms the fact is there, then reads the live steps cold and lists the premises the map left out.
4. A test enforces it: the docs check fails a plan with live steps and no Premise map, or a row with no source.
5. Before any live run, the go request shows the user the step's call list with its unchecked items.

## What was verified in the files, and how

Everything here was read in this session from the files named; the Premise map at the end gives each a row with its line.

The planner file asks for a Promise map in one paragraph (planner.md line 11) and says nothing about premises. The reviewer file checks the Promise map "line by line" (reviewer.md line 11) and nothing else about a plan's facts. Both files sit in `agents/` of the hooks repo, which that repo's `.derive-once-gate` marker names (`C:\Users\Patrick\.claude\gates\README.md` line 112), so a change to either needs its own worktree under `C:\Users\Patrick\.claude\worktrees\` and one fresh reviewer, dispatched with the hooks repo's root folder written in the brief (derive-once-review.md line 157). The test command form for that repo is on README lines 116 to 122.

Plans are reviewed as committed files on a branch, and the plan shown with ExitPlanMode is gated (doc-review.md lines 17 and 21; `publish_gate.py` line 112 reads the `plan` field). Committed plans are internal docs and not gated after that. So the plan reviewer is the one place a premise gets a human-like check, and it is the reviewer file that must ask for it.

The docs test (`tests/test_docs_match_code.cpp`) runs seven checks, listed in its header (lines 1 to 33). It reads every markdown file git lists through `markdown_docs()` (line 1236, on `sourcetree::markdown_files`, `tests/source_tree.h` line 85). Checks 6 and 7 skip `docs/handoffs/` and `docs/superpowers/` through `kNotPermanentDirs` (line 1226), so today no check reads a plan at all. Each check is a function over `std::vector<Doc>` (struct at line 70) that pushes `doc:line: message` strings, with a repo-wide TEST_CASE and a second TEST_CASE on hand-made `Doc` values that pins each form the check catches and each form it lets pass (lines 1263 to 1302 are the pattern). The new check copies that pattern. The docs check runs at every merge to Hydra's `main` (agent rule 1), so a plan that breaks the shape is refused at the merge.

The handoff checker (`tools/check_handoff.py`) reads a table only under a heading that is exactly `## Claims` (`CLAIMS_HEADING`, line 31; `claims_rows`, lines 335 to 349), takes the kind from the first column, reads the backticked token of the value column (line 380), marks the kinds it cannot judge `manual` (line 32) and prints one status word per row. The Premise map reuses that shape: a heading that is exactly `## Premise map`, a Kind column with a fixed word list, a Source column read as a backticked token, and a status the reader can act on.

Three plans in the repo have a live-step section, all under the heading `## The live steps` with a parenthesis after it (`2026-10-30-ch-mcp-in-game-input.md` line 198, `2026-10-30-ch-mcp-star-power-and-song-start.md` line 214, `2026-10-31-ch-mcp-direct-start.md` line 193). Their steps are `### LS-...` headings. That is the heading the test keys on.

The live-run page (`docs/agents/clone-hero-mcp.md`, "The shape of a live run", lines 39 to 50) says each run names its pass and fail lines, its cap and ends with `ch_stop`. It says nothing about a call list or what the go request shows.

The memory rule "trace every value a run's pass line needs" exists (`C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\trace-pass-inputs-before-live-runs.md`) and was missed today. That is why this plan puts the rule in a test and in the reviewer's steps, not in another note.

## The design

### The Premise map's shape (one home: `docs/agents/premise-map.md`, task PM-3)

The table sits under a heading that is exactly `## Premise map`. It has four columns.

| Premise | Kind | Source | Status |
|---|---|---|---|
| one fact the plan depends on, in one sentence | `code`, `data`, `chart`, `run`, or `unchecked` | the exact place, in backticks: a repo path with a line or a block id, a game file path, a chart path, or a run's result file | `checked` with the date, or for `unchecked`: the task id that checks it, or `question N` |

The Kind words mean: `code` is a source or disassembly file (Hydra code, a `docs/clone-hero/code/` listing, `dump.cs`); `data` is a game data file or a game page block that cites one (page 07 `IN-nn` counts, because every block carries its evidence line); `chart` is a chart file; `run` is a result file of a live run kept under the runs folder. A row whose only source is another plan, a handoff, a doc with no evidence line, a memory note, a card or nothing is `unchecked`, and its Status must name a task in the plan or a question number. A `Source` cell that is empty, or not in backticks, is a shape error.

The rule the page states, which the planner and the reviewer both point at: an unchecked premise never carries into a task or a live step as settled. A task or live step that uses an unchecked premise names the row.

### The call list (same page)

Under every `### LS-` heading of a `## The live steps` section, before the next heading, one line starts with `Calls:` and lists every tool call the step makes, in order. Each call is written as `` `tool` (seen: <run id>) `` or `` `tool` (never tried) ``. "Seen" means the call ran on this exact target before: the same player, the same route, the same chart. A run id is the step and run that showed it, for example `LS-D0 run 1`. A step with more than one never-tried call fails the shape. Its author splits it, so a failure names one piece.

### The reviewer's two premise checks (task PM-1)

The reviewer of a plan opens each `checked` row's source and confirms the fact is there, as doc-review step 2 already does for references; a row whose source does not say what the row says is a finding. Then, with the map closed, the reviewer reads each live step and each task and writes down every fact it depends on. A fact with no row is a finding, fixed by adding the row (as `unchecked` when the reviewer cannot source it). This is the twin of the Promise map check the reviewer file already has.

### The test (task PM-2)

Check 8 in `tests/test_docs_match_code.cpp`. Which plans it reads is decided here, and question 1 puts the choice to the user: a plan under `docs/superpowers/plans/` counts when its file-name date is on or after a constant the test owns, `kPremiseMapSince`, or when it already has a `## Premise map` heading (so a retrofit is held to the shape, and an older plan without one is left alone). For a plan that counts, the check fails with `doc:line: message` when: there is no `## Premise map` heading; the table under it has no rows; a row's Kind is not one of the five words; a row's Source is empty or not backticked; an `unchecked` row's Status names neither a task id in the plan (a `### <id>:` heading) nor `question N`; the plan has a `## The live steps` heading and a `### LS-` step under it has no `Calls:` line; a `Calls:` entry lacks `(seen: ...)` or `(never tried)`; a step has two or more `(never tried)` entries. The comment above the check points at `docs/agents/premise-map.md` for the shape and restates none of it.

### The go request (task PM-3)

"The shape of a live run" gains one paragraph: before the main session asks the user for a run, its message shows that step's `Calls:` line as the plan holds it, and names each `(never tried)` entry and each `unchecked` row the step uses. The user's go answers that message, not a bare step name. No hook checks this (question 3).

### Who owns which question

The hooks repo owns what a planner and a reviewer are told (`agents/planner.md`, `agents/reviewer.md`). Hydra owns the shape (`docs/agents/premise-map.md`), the test that enforces it (`tests/test_docs_match_code.cpp`) and the live-run rule (`docs/agents/clone-hero-mcp.md`). The planner file points at the Hydra page for the shape, the same way the preamble points at `docs/agents/handoff.md`; the shape is written in one place.

## Questions for the user

1. **Which plans the test reads.** Recommended: every plan whose file-name date is on or after `kPremiseMapSince = 2026-11-01`, plus any plan that already has a `## Premise map` heading. Plans before that date with no map are left as they are, because rewriting approved plans is its own question. The other side: every plan with a `## The live steps` heading, whatever its date. That would fail the three existing live-step plans at the next merge until each gets a map, and two of them are finished (their LS-K steps ran or were struck). A third side: every plan of any date, which fails about thirty plans at once.
2. **Retrofit the direct-start plan.** Its LS-D2 plays Window Map, which has no Star Power phrases (row P4), so its pass line cannot happen, and its LS-D1 ran and failed on a premise (row P3). Recommended: a small task PM-4 adds a Premise map and `Calls:` lines to that plan and changes LS-D2's chart to one with phrases (Senri was its other side, question 5 of that plan); the test then holds it to the shape because it has the heading. The other side: leave the plan as it stands and write LS-D2 again in a fresh plan when the kit's player question is settled.
3. **Is the go request enforced by a hook?** Recommended: no. It is an instruction on the live-run page, and the plan reviewer checks that each step's `Calls:` line exists; the main session copies that line into its go message. A hook would have to read the session's chat for the call list, and the hooks rebuild (2026-10-25) ruled out free-text parsing. The other side: a PreToolUse hook on `ch_start` that refuses a `human=True` launch unless a state file names the step and its call list, written by the main session first. That is a new chokepoint and a new state file, and it is not in this plan.
4. **The call-list chain rule as a hard fail.** Recommended: two or more `(never tried)` entries in one step fail the test, with the message naming the step. The other side: one never-tried entry is allowed per step only for the first run, and later runs may add more; the test cannot tell runs apart, so this would be instruction only.
5. **Where the agent-file test lives.** Recommended: a new `gates/tests/test_agent_files.py` in the hooks repo that reads `agents/planner.md` and `agents/reviewer.md` and checks the three sentences PM-1 adds are there (the Premise map heading, the `Calls:` line, the reviewer's two checks). The other side: `gates/tests/test_docs_numbers.py` names `agents/` today (a grep found the name; the file was not read), and the executor may add the check there if it already reads those files.

## Tasks

Each task fits one executor under 100 calls and 20 minutes. PM-1 and PM-3 start at once. PM-2 starts after PM-3 is merged, because its comment names the new page and check 7 fails a code comment that names a docs path not in the tree. PM-4 (if the user says yes to question 2) starts after PM-3 is merged and runs beside PM-2.

### PM-1: the planner and reviewer files (hooks repo; earlyStart: yes)

Goal: the planner asks for a Premise map and a `Calls:` line per live step; the reviewer opens each row's source and reads the live steps cold.

Repo and worktree: `C:\Users\Patrick\.claude`, a worktree under `C:\Users\Patrick\.claude\worktrees\premise-map-agents` on branch `premise-map-agents`. Executor on Opus (agent rule 2; the `agents/` folder is gated like code, README line 112). One fresh reviewer, dispatched with that repo's root folder in the brief (derive-once-review.md line 157).

Files: `agents/planner.md`, `agents/reviewer.md`, and the test from question 5 (`gates/tests/test_agent_files.py`, or `gates/tests/test_docs_numbers.py`).

Acceptance criteria:
- planner.md holds a paragraph right after the Promise map paragraph that says: every plan ends with a `## Premise map` table, its shape is on Hydra's `docs/agents/premise-map.md`, every fact the plan depends on gets a row, a row with no code, data, chart or run source is `unchecked` and must name a check task or a user question, and an unchecked premise never carries into a task or live step as settled. It says every `### LS-` step holds a `Calls:` line with each call marked seen or never tried, and that a step never chains two never-tried calls. Proof: the test from question 5 asserts each of those sentences' key phrases (`## Premise map`, `docs/agents/premise-map.md`, `Calls:`, `never tried`, `unchecked`) is in planner.md.
- reviewer.md holds, after its Promise map sentence: when reviewing a plan, open each `checked` row's source and confirm the fact is there; then read the live steps and tasks with the map closed and list every fact they depend on that has no row; both are findings to fix. Proof: the same test asserts `Premise map` and `no row` are in reviewer.md.
- The test fails when the phrase is removed. Proof: the executor runs it once red with one phrase deleted and keeps the red line in its report (preamble, "Tests").

Steps: read both files; add the paragraphs; write the test; run it red then green; commit with the trailers.

Verify: `C:\Users\Patrick\AppData\Local\Python\pythoncore-3.14-64\python.exe -I -m unittest discover -t . -s gates/tests -p test_agent_files.py` from the worktree root (README lines 116 to 122), expected `OK`. No other test runs.

### PM-3: the shape page and the go-request rule (Hydra docs; earlyStart: yes)

Goal: one page owns the Premise map's and the call list's shape; the live-run page tells the main session what a go request shows.

Repo and worktree: Hydra, a worktree under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\premise-map-page` on branch `claude/premise-map-page`. Executor on Opus. One fresh reviewer with `docs/agents/doc-review.md` and these sources: this plan's "The design" section, `docs/agents/handoff.md` (the pattern), `tools/check_handoff.py` lines 31 to 32 and 335 to 349.

Files: `docs/agents/premise-map.md` (new), `docs/agents/clone-hero-mcp.md` (one paragraph in "The shape of a live run"), `CLAUDE.md` (one line under "Agent rules" or "Doc review" pointing at the page, so a session finds it the way it finds handoff.md).

Acceptance criteria:
- The page states the table's heading, four columns, five Kind words with their meaning, the Source rule (backticked, exact place), the Status rule for unchecked rows, the `Calls:` line's form with its two markers, and the one-never-tried-per-step rule, in plain English, as "The design" above. Proof: the docs check passes on the page (`-tc=docs match code*`, its backticked names resolve), and PM-2's test comment points at this page.
- The live-run page says the go request shows the step's `Calls:` line, each `(never tried)` entry and each `unchecked` row the step uses, and that the user's go answers that message. Proof: the reviewer's cold read; and the docs check passes.
- Every path the page names is in the tree (check 7). Proof: `-tc=docs match code*` passes.

Steps: write the page from "The design"; add the paragraph; add the CLAUDE.md line; run the docs check; commit.

Verify: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`, all cases pass. Nothing else.

### PM-2: check 8 in the docs test (Hydra C++; earlyStart: no; after PM-3 is merged)

Goal: the docs check fails a plan that counts and breaks the shape.

Repo and worktree: Hydra, a worktree under `.claude\worktrees\premise-map-check` on branch `claude/premise-map-check`. Executor on Opus. One fresh reviewer with `docs/agents/derive-once-review.md`.

Files: `tests/test_docs_match_code.cpp` (header list gains item 8; a new `check 8` section with `check_premise_maps(const std::vector<Doc>&, std::vector<std::string>&)`, the constant `kPremiseMapSince`, and two TEST_CASEs); `tests/test_docs_match_code.cpp` is the only code file. If the executor finds the plan-folder walk needs a helper in `tests/source_tree.h`, it stops and reports, because that header is shared by the single-owner scans.

Acceptance criteria:
- Which plans count is the rule of question 1 as the user answers it. Proof: the hand-made TEST_CASE holds a plan dated before the cutoff with no map (passes), one dated before with a map that breaks the shape (fails), one dated on the cutoff with no map (fails), and one outside `docs/superpowers/plans/` (ignored).
- Each failure form from "The test" above fails with `doc:line: message` naming the row or the step. Proof: the hand-made TEST_CASE holds one `Doc` line per form (no heading; empty table; bad Kind; empty Source; unbackticked Source; unchecked row with no task or question; live step with no `Calls:`; a `Calls:` entry with neither marker; two `(never tried)` in one step) and a passing plan, and checks the problem count and each line prefix, as lines 1270 to 1302 do for check 6.
- The repo-wide TEST_CASE passes on the tree as merged, this plan included. Proof: `-tc=docs match code: every plan that counts*` passes.
- The test's comment points at `docs/agents/premise-map.md` and does not restate the shape (preamble, "If you write code"). Proof: the reviewer reads it; and check 7 passes because the page exists.
- This plan's own map and the cutoff agree: this plan is dated 2026-10-31 and has a map, so it counts under the recommended rule by its heading. Proof: the repo-wide case passes with this plan in the tree.

Steps: read check 6 and check 7 as the pattern; write `check_premise_maps` and the two cases; run the file's cases; commit.

Verify: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`, all pass, with the two new case names in the output. The first build is warm if `build-cpp` exists, else one cold build with `build_cpp.ps1`.

### PM-4: retrofit the direct-start plan (Hydra docs; only if question 2 is yes; after PM-3 is merged)

Goal: `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` gets a Premise map and `Calls:` lines, and LS-D2 names a chart that has Star Power phrases.

Executor on Opus; one fresh reviewer with doc-review.md and these sources: page 07 IN-61, IN-62, IN-65, IN-67 and IN-76, page 05 SP-12 and SP-17, page 06 SC-15, the Window Map chart, and the LS-D1 run 1 result files named in IN-76.

Acceptance criteria:
- The map has a row for each fact in the plan's "What was verified in the code" section and each live step's pass line, with IN-76 as the `run` row behind LS-D1's result and the kit-player question marked `unchecked` with the question it goes to. Proof: PM-2's repo-wide case passes on the plan (it has the heading, so it counts), and the reviewer's cold read.
- LS-D2's chart row cites the chart's `S` lines by path. Proof: the reviewer opens the chart.
- Each LS step's `Calls:` line marks `ch_start(human=True)`, `ch_attach` and `ch_status` as seen in LS-D0 or LS-D1 run 1 (their result files), `ch_kit_setup(player=...)` as never tried on whichever player the plan now names, and the rest as the result files show. Proof: PM-2's case; the reviewer opens the run folder.

Verify: `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"`.

## Order and parallelism

Wave 1: PM-1 (hooks repo) and PM-3 (Hydra) together. Wave 2, after PM-3 merges: PM-2, and PM-4 if approved, together. The main session merges each after its reviewer signs off and runs what `hydra_precheck.exe --tests` prints for the range (agent rule 1); for PM-1 it runs the one unittest file.

## Out of scope (needs the user's yes to leave out)

- A hook that refuses a live run without a shown call list (question 3, other side).
- Premise maps for the roughly thirty plans before the cutoff (question 1, third side).
- A checker script for the Premise map like `check_handoff.py`. The docs test checks the shape at every merge; opening each source is the reviewer's job, because a line number or a block id cannot be judged by a script beyond "the file exists".
- Teaching the test to tell a step's first run from later runs (question 4, other side).

## Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every plan carries a Premise map in the shape on the page | "What the user approved" 1; "The design" | PM-1 (ask), PM-2 (enforce) | `test_agent_files.py` finds the phrases in planner.md; check 8's hand-made case fails a counting plan with no heading |
| An unchecked row names a task or a question | "The design", shape | PM-2 | hand-made case: unchecked row with neither fails |
| A row's Source is backticked and non-empty | "The design", shape | PM-2 | hand-made case: empty and unbackticked Source fail |
| Kind is one of five words | "The design", shape | PM-2 | hand-made case: bad Kind fails |
| An unchecked premise never carries into a task or live step as settled | "What the user approved" 1 | PM-1 (planner and reviewer text), PM-3 (page) | `test_agent_files.py` finds `unchecked` in planner.md; the reviewer's cold read is the human check; no test can read a plan's prose for this, so the page says it and the reviewer checks it |
| Every live step has a `Calls:` line with each call marked seen or never tried | "What the user approved" 2 | PM-2 | hand-made case: step with no `Calls:` fails; entry with no marker fails |
| A step never chains two never-tried calls | "What the user approved" 2 | PM-2 | hand-made case: two `(never tried)` fail (question 4) |
| The reviewer opens each row's source and reads the live steps cold | "What the user approved" 3 | PM-1 | `test_agent_files.py` finds `Premise map` and `no row` in reviewer.md |
| The docs check fails a plan with live steps and no map, or a row with no source | "What the user approved" 4 | PM-2 | the two hand-made cases above; runs at every merge (agent rule 1) |
| Which plans count is the cutoff-or-heading rule | "The test"; question 1 | PM-2 | hand-made case: before-cutoff no map passes; before-cutoff with map is checked; on-cutoff no map fails; outside the folder ignored |
| The go request shows the call list and unchecked items | "What the user approved" 5 | PM-3 | instruction on the live-run page, reviewed by PM-3's reviewer; no test (question 3) |
| The shape is written in one place | "Who owns which question" | PM-2, PM-3 | the test's comment points at the page and restates nothing (reviewer); check 7 proves the page path exists |
| PM-2's comment names a page that is in the tree | "Tasks", order | PM-2 after PM-3 | check 7 of the docs test |
| This plan passes its own check | "The plain version" | PM-2 | the repo-wide case passes with this plan in the tree |
| LS-D2 names a chart with Star Power phrases | question 2 | PM-4 | the reviewer opens the chart; PM-2's case holds the plan to the shape |

## Premise map

| Premise | Kind | Source | Status |
|---|---|---|---|
| P1: LS-K3's plan assumed a kit on Player1 made a drums player; the screen's controller type comes from the profile's controller_type, default Guitar | `data` | `docs/clone-hero/07-internals.md` IN-65 (line 663) and IN-67 (line 683); `docs/sessions/2026-10-31-597e0795.md` (run 1 stopped at the player screen) and `docs/sessions/index.md` line 438 (run 2 stopped there) | checked 2026-10-31; "all three runs" is partly checked: cards for runs 1 and 2 were found, a run 3 card was not found by the words "player screen" in the index |
| P2: the loader sets is_bot with one instruction, so the menu walk's premise (a command-line launch always means a bot) was never read in code | `data` | `docs/clone-hero/07-internals.md` IN-61 (line 623, byte 0x396EC6); `docs/superpowers/plans/2026-10-31-ch-mcp-direct-start.md` lines 25 to 31 | checked 2026-10-31 |
| P3: the kit's setup loads the player's default maps, and Rewired's data gives System none | `run` | `docs/clone-hero/07-internals.md` IN-76 (line 773), evidence files under `hydra-data\ch-mcp-runs\2026-10-31\ls-d1-run1\` as that block names them | checked 2026-10-31 (the block was read; the run folder was not opened by this agent) |
| P4: Window Map has no Star Power phrases, so LS-D2's pass line cannot happen | `chart` | `C:\Clone Hero\songs\Hydra Probe\Window Map\notes.chart`: 0 lines matching `= S `, 142 lines matching `= N 0 ` (counted this session); `docs/clone-hero/05-star-power.md` SP-12 (line 123), SP-17 (line 175); the plan's question 5 at `2026-10-31-ch-mcp-direct-start.md` line 134 | checked 2026-10-31 |
| P5: the direct start always sets the DoubleKick modifier | `data` | `docs/clone-hero/07-internals.md` IN-62 (line 633) | checked 2026-10-31 |
| P6: `Drums,Expert` is the default player word and plain Drums drops accents and ghosts | `code`, `data` | `tools/ch_mcp/bot_run.py` line 60 (`DEFAULT_PLAYER`); `docs/clone-hero/06-scoring.md` SC-15 (line 155) | checked 2026-10-31 |
| P7: ch_start returns after the resume, so "arm before the resume" is not a caller's option | `code` | `tools/ch_mcp/bot_run.py` `launch_human` line 615, the resume at line 693; `docs/agents/clone-hero-mcp.md` line 43 | partly checked: found by grep, the function body was not read end to end; PM-4's reviewer reads it |
| P8: planner.md asks for a Promise map and nothing about premises | `code` | `C:\Users\Patrick\.claude\agents\planner.md` line 11 | checked 2026-10-31 |
| P9: reviewer.md checks the Promise map line by line and nothing else about a plan's facts | `code` | `C:\Users\Patrick\.claude\agents\reviewer.md` line 11 | checked 2026-10-31 |
| P10: the hooks repo gates `agents/` like code; a change needs a worktree and a fresh reviewer with the repo root in the brief | `code` | `C:\Users\Patrick\.claude\gates\README.md` line 112; `docs/agents/derive-once-review.md` line 157 | checked 2026-10-31 |
| P11: the hooks repo's test command form | `code` | `C:\Users\Patrick\.claude\gates\README.md` lines 116 to 122 | checked 2026-10-31 |
| P12: plans are reviewed as committed files; ExitPlanMode is gated and reads the `plan` field | `code` | `docs/agents/doc-review.md` lines 17 and 21; `C:\Users\Patrick\.claude\gates\publish_gate.py` line 112 | checked 2026-10-31 |
| P13: the docs test has seven checks, walks git-listed markdown, and checks 6 and 7 skip `docs/superpowers/` | `code` | `tests/test_docs_match_code.cpp` lines 1 to 33, 1226, 1236; `tests/source_tree.h` line 85 | checked 2026-10-31 |
| P14: the check pattern is a function over `Doc` values plus a repo-wide case and a hand-made case | `code` | `tests/test_docs_match_code.cpp` lines 70, 400, 1249 to 1302 | checked 2026-10-31 |
| P15: the docs check runs at every merge to Hydra's main | `code` | `docs/agents/agent-rules.md` rule 1 (line 7) | checked 2026-10-31 |
| P16: the handoff checker reads a table under an exact heading, by Kind, backticked value, with manual kinds | `code` | `tools/check_handoff.py` lines 31, 32, 335 to 349, 352 to 367, 380 | checked 2026-10-31 |
| P17: three plans use the heading `## The live steps` and `### LS-` steps | `code` | `docs/superpowers/plans/2026-10-30-ch-mcp-in-game-input.md` line 198; `2026-10-30-ch-mcp-star-power-and-song-start.md` line 214; `2026-10-31-ch-mcp-direct-start.md` lines 193 to 215 | checked 2026-10-31 (a grep for headings with "live step", "live run" or "live check"; a plan that names its steps another way would be missed) |
| P18: the live-run page says nothing about a call list or the go request | `code` | `docs/agents/clone-hero-mcp.md` lines 39 to 50 | checked 2026-10-31 |
| P19: the memory rule about tracing pass-line inputs exists and was still missed | `data` | `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\trace-pass-inputs-before-live-runs.md` (exists); the miss is P3 and P4 | checked 2026-10-31 |
| P20: `gates/tests/test_docs_numbers.py` names `agents/` | `code` | `C:\Users\Patrick\.claude\gates\tests\test_docs_numbers.py` (grep hit on the folder name; the file was not read) | partly checked; question 5 lets the executor decide after reading it |
| P21: the user approved the five points | `unchecked` | the user's chat words in session 90d40bc9 ("Yes, plan it"), relayed in this agent's brief; the session card is not yet written | question: the main session confirms the quote when it writes the card; this plan's reviewer cannot open it |

## Prior art

Searched on 2026-10-31. Three ideas were used.

A pre-mortem (Gary Klein's method) starts from "the plan has already failed" and traces each failure back to the assumption that caused it, then binds each to a plan change; a skill page on the method warns that generic risks with no tie to the specific plan mean the exercise is not working ([thinking pre mortem](https://skills.cat/skills/tjboudreaux/cc-thinking-skills/thinking-pre-mortem)). The reviewer's cold read of the live steps is that step, done by someone who is not the plan's author, which a coaching scorecard also recommends ([Pre-Mortem Scorecard](https://tools.successcoaching.co/MLP_Tool_7_Pre_Mortem_Scorecard.html)); its go criteria, that every load-bearing assumption is tested or has a monitor and that an untested one is a no-go, are the Status column's rule.

An assumptions register records each provisional input with its source, owner and the verification route that closes it, and stays short ([solar design assumptions register](https://www.surgepv.com/blog/solar-design-assumptions-register)). The Premise map's four columns are that register cut to what a reviewer can open.

Goal Structuring Notation gives assumptions their own node so an argument cannot hide one in prose ([NASA GSN briefing](https://ntrs.nasa.gov/api/citations/20160005295/downloads/20160005295.pdf)); one research-software assurance case has a goal "all operational assumptions are defined" beside a goal "the inputs satisfy them" ([Using Assurance Cases to Guide V&V of Research Software](https://arxiv.org/html/2411.03291v1)). The map's `checked` and `unchecked` statuses are those two goals.

Other sources: [A Methodology for Automating Assurance Case Generation](https://arxiv.org/pdf/2003.05388), [Security Assurance Cases](https://arxiv.org/pdf/2003.14151), [NASA IV&V assurance cases](https://ntrs.nasa.gov/api/citations/20200001646/downloads/20200001646.pdf). Nothing was found on an assumptions register in a systems-engineering standard with a verify-before-test rule; that search was not pursued further.
