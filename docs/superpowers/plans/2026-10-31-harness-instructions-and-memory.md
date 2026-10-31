# Harness cleanup, part 1: instructions and memory (2026-10-31)

Task HX-P1, session 6777b546. This is one of two plans. The other, the HX-P2 plan, owns the hooks and the tools (the handoff checker, the refusal log, the posture delivery, the pointer check). This plan owns the words: the project `CLAUDE.md`, `docs/agents/`, the global `CLAUDE.md`, the posture text, the agent type files, the memory folder and a `/handoff` command. Nothing here is built yet. Every task waits for the user's yes (CLAUDE.md rule 3).

## 1. What this plan does, in plain words

The user approved a cleanup of the harness on 2026-10-31 ("yes plan all of this") and added one rule in their words: "for the rule that says never trust another agent's summary, that includes handoffs. before starting any new work, every claim in the handoff needs to be checked against what actually exists. the code is the source of truth."

So this plan does five things.

1. It writes the handoff rule into the three places every session reads (CLAUDE.md rule 9, the preamble, the posture text), gives handoffs a shape whose claims can be checked, and adds a `/handoff` command that writes one in that shape.
2. It shrinks what loads every time. The project CLAUDE.md (2,225 words, counted on 2026-10-31 by splitting on whitespace) becomes a table of contents of one-line pointers. Every procedure in it moves whole into `docs/agents/`. `MEMORY.md` (2,116 words, 124 note files plus 64 in `archive/`) shrinks to the notes that are still live.
3. It loses no ruling. Before any note moves or goes, every sentence of the user's own words it holds is copied into one repo page, `docs/agents/rulings.md`, with its date and session id. Section 5 lists what is kept and what is dropped.
4. It fixes the contradictions and broken pointers the review found, and where a fix would pick one side of a split, it asks the user instead (section 8).
5. It keeps the 2026-10-25 rulings: style rules stay as instructions, no new hooks come from this plan, and the hook line cap is not touched.

Each claim in the review findings that this plan builds on was checked against the files on 2026-10-31. The ones that did not hold are in section 9.

### Prior art

Two ideas from outside shaped this plan. Claude Code's own docs say a project slash command is a markdown file under `.claude/commands/` and a user command sits under `~/.claude/commands/`; the file name is the command name, and the two scopes must not share a name ([Claude Code slash commands](https://docs.claude.com/en/docs/claude-code/slash-commands)). Several practitioners say the same thing about instruction files: keep the always-loaded file a map, not an encyclopedia, and let the agent open the long page only when the task needs it ([your CLAUDE.md should be a map](https://contra.com/community/HDedmzbW-your-claudemd-should-be-a-map); [progressive disclosure for agents](https://codemyspec.com/blog/progressive-disclosure)). None of them is an official limit; the numbers in this plan are ours.

## 2. Global constraints

These hold for every task.

- G1. Every rule in the project CLAUDE.md today survives, in the same words or plainer ones, in CLAUDE.md or in a `docs/agents/` page. Nothing is dropped by the move. T1 proves it with a sentence check (its Verify).
- G2. Rule numbers 1 to 10 keep their numbers. The preamble, `model-postures.md`, `derive-once-review.md` and `integrate.md` cite rules by number.
- G3. Every user ruling held in a memory note that T4 removes, archives or rewrites is copied, in the user's exact words, into `docs/agents/rulings.md` first. A ruling is any sentence a note quotes from the user. T4 proves it with a quote check (its Verify).
- G4. No task adds a hook, a gate or a settings.json rule. That is the HX-P2 plan's side. (The user's 2026-10-25 ruling: "make a new plan to get rid of every hook we have now and start over from scratch", and style rules are instructions only.)
- G5. No permanent doc cites a scratch folder. The docs test's scratch check (`hydra_tests.exe "-tc=docs match code*"`) runs in every Hydra task's Verify.
- G6. Every `docs/agents/` page passes the docs-vs-code test (`tests/test_docs_match_code.cpp` reads every `docs/agents/*.md`; a backticked name that is not in the code fails it).
- G7. The three places that state the handoff rule (CLAUDE.md rule 9, the preamble, the posture text) use the one sentence in section 3, word for word, so a later edit to one of them is caught by a grep.
- G8. Tasks run in parallel. No two tasks own the same file. T3 runs in a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\`; the other tasks run in worktrees under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`.
- G9. Each Hydra task's commit goes through one fresh reviewer, dispatched as CLAUDE.md "Derive-once review" says. The memory folder is in no git repo, so T4's reviewer brief names the memory folder as a second thing to read, beside the committed `rulings.md`.

## 3. The handoff rule and the handoff shape

### The sentence

The one sentence, used word for word in three places (G7):

> A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong.

CLAUDE.md rule 9 becomes: "Never trust another agent's summary or the comments it left behind. The code and the source data are the source of truth. Check a claim against them before you repeat it or act on it. A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong. `docs/agents/handoff.md` says what a handoff holds and how to check one."

### The shape

A handoff holds prose and one table. The prose says the goal, what was done and what is next, in plain English. The table, under a heading that is exactly `## Claims`, lists every fact the next session will act on. One row per claim, four columns:

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | the full 40-character hash | what the commit holds, and whether it is on `main` or pushed | `git cat-file -e <hash>`; `git branch --contains <hash>`; `git log origin/main --oneline -1` |
| path | a repo-relative path, or a full absolute path outside the repo | what the file holds or which function or line it names | the file exists; the named function or text is in it |
| test | the exact command | the line it printed last time (pass count, or the one red line) | run it; the same line prints |
| ruling | the session id or card id | the user's words, in quotes | the card or transcript holds those words |
| state | a plain fact ("the worktree X exists", "Y is not yet merged") | the fact | the command that shows it |

Rules for the table. A claim with no row is not a claim, and the next session may not act on it. A row whose check fails is wrong until someone fixes the handoff. A hash is written in full, never abbreviated, because the checker and `git cat-file` need the full form. A handoff names no scratch path (G5; `docs/handoffs/` is skipped by the docs test's scratch check today, `kNotPermanentDirs` in `tests/test_docs_match_code.cpp`, so the rule holds by instruction there).

### What this plan needs from the handoff checker in the HX-P2 plan

This plan writes the shape; the HX-P2 plan writes the script that checks it. The interface this plan needs:

- The command is `python -I -S C:\Users\Patrick\.claude\gates\handoff_check.py <handoff file> --repo <repo root>`.
- It reads only the rows under `## Claims`. It checks every `commit` row with `git cat-file -e`, and says whether the hash is reachable from `main` and from `origin/main`. It checks every `path` row with a file-exists test (a repo-relative path against `--repo`, an absolute path as it is). It does not run `test` rows; it prints them for the session to run. It does not judge `ruling` or `state` rows; it prints them.
- It prints one line per row: `ok | missing | not-on-main | not-pushed`, then the kind, the value and the row's Claim text. It exits 0 only when every `commit` and `path` row is `ok`. It prints nothing else, so the session can paste the output into its first status line.
- It is run by the `/handoff` command after writing a handoff, and by the main session before continuing one.

If the HX-P2 plan names the script or the flags differently, `docs/agents/handoff.md` and `.claude/commands/handoff.md` take the HX-P2 plan's names, and the reviewer of whichever lands second makes them agree.

### The `/handoff` command

A project command, `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\commands\handoff.md`, so it is committed and reviewed with the repo and reads `docs/agents/handoff.md` by a relative path. The hooks repo keeps no copy, because its own handoffs are rare and the user scope would shadow the project one if both held a `handoff.md` (the docs say the two scopes must not share a name). The command's text: stop launching work; write `docs/handoffs/<today>-<slug>-handoff.md` in the shape above, with the user's rulings from this session in their words; run the handoff checker on it; paste the checker's output into the handoff under `## Checker output`; commit the handoff on a branch; report the path. `$ARGUMENTS` is the slug.

## 4. The project CLAUDE.md as a table of contents

Today CLAUDE.md is 2,225 words, and about three quarters of them are procedures: "Derive-once review" (about 425 words), "Doc review" (about 439), "Agent rules" (about 819). The two review sections repeat what `docs/agents/derive-once-review.md` ("For the orchestrator") and `docs/agents/doc-review.md` already say, in a second wording.

After T1, CLAUDE.md is under 700 words. Each section is a heading, one to three sentences that say the rule in short, and one pointer to the page that holds the full text. The ten agent rules stay as ten numbered one-line rules (G2) with the pointer `docs/agents/agent-rules.md` for their details. The two review procedures move into the pages that already own them: the orchestrator's steps into `docs/agents/derive-once-review.md` "For the orchestrator", and the doc-review gate rules (what is gated, the small-text pass, the `publish.ps1` rule, the waiver) into a new "What is gated" section at the top of `docs/agents/doc-review.md`.

The "Past sessions" section stays as it is until the session-cards handoff (`docs/handoffs/2026-10-31-session-cards-into-docs-and-handoff-cleanup-handoff.md`) moves the cards; T1 only shortens it to a pointer if its text can move whole into `docs/agents/audit-method.md` or a new page. T1 must not change where the cards live.

## 5. The memory folder: what is kept, what moves, what goes

`MEMORY.md` is 124 index lines. Seventeen say "TOP RULE". The label has no written meaning today. This plan gives it one (section 8, question Q5 asks the user to confirm): a note marked TOP RULE is a standing rule the user gave in their words, and it stays in MEMORY.md; everything else is either a repo doc or an archived note. Until the user answers, T4 keeps every TOP RULE note as it is.

T4 sorts every note into one of four exits. The sort below is this planner's reading of the index lines on 2026-10-31. Five index lines are claimed to disagree with their files (`ch-mcp-server-2026-10`, `album-search-d263-2026-10`, `doc-review-gate-2026-10`, `concept-audit-2026-10`, `every-brief-carries-status-rule`); T4 reads each file before it sorts it, and its report lists any note whose exit changed after reading.

**Exit 1, stays in MEMORY.md (a standing rule, user-ruled).** The 17 TOP RULE notes, plus these RULE notes that hold a user ruling or a rule no repo page holds: `check-long-captures-early`, `name-the-owner-across-boundaries`, `merge-tests-touch-changed-files`, `repo-scans-read-main-checkout`, `error-lines-derived-once`, `comments-explain-current-code`, `boundary-cases-follow-the-rule`, `read-fixed-values-once`, `every-number-says-where-it-comes-from`, `one-word-one-meaning-in-public-text`, `derive-display-from-engine-truth`, `fail-loudly-never-freeze`, `fix-hooks-dont-bypass`, `no-silent-functionality-changes`, `measure-small-explain-first`, `use-source-facts-not-proxies`, `questions-describe-the-real-rule`, `verify-before-claiming-a-source-is-wrong`, `names-from-data-not-recall`, `search-web-before-finalizing-plans`, `stage-by-name-in-shared-checkout`, `refused-merge-abort` (with its pointer fixed, see below), `haiku-for-simple-readers`, `check-the-screen-against-the-data`, `render-checks-use-the-corpus`, `relay-status-in-plain-english`, `session-references-only-itself`, `lean-agent-fanout`, `one-place-promise-covers-every-question`, `every-brief-carries-status-rule` (rewritten: status every 5 minutes, helper under `gates\`, the user's words kept), `db-upgrade-freeze-2026-10-07`, `stored-versions-one-rule`, `ch-runs-must-not-need-focus`, `ch-mcp-feasibility-2026-10`. Their rulings are also copied into `rulings.md` (G3 applies to every note T4 rewrites, and copying the rest costs one pass).

**Exit 2, moves into a repo doc, then leaves MEMORY.md.** Project facts that belong where the code's readers look. The target page must be one this plan owns (`docs/agents/`); facts whose home is `docs/development.md`, `CONTEXT.md` or a Clone Hero page go on the Out of scope list for the user.

- `gui-testing-hydra-uitest` (build and test-engine traps) and `report-capture-scripts`, `branch-window-check-recipe` → `docs/agents/ui-testing.md`.
- `hydra-mcp-server-2026-10` → `docs/agents/ui-testing.md` "Live mode and the MCP server".
- `ch-mcp-server-2026-10` → `docs/agents/clone-hero-mcp.md`; any game fact in it goes to its Clone Hero page (Out of scope list, since those pages are not this plan's).
- `python-locations`, `gh-cli-location`, `ghidra-location`, `vs2026-incomplete-instance`, `opus-model-choice` → a new `docs/agents/this-machine.md` (where the tools are on this machine, read once).
- `inspect-hydra-db-without-sqlite-cli`, `gui-text-probe-for-stored-numbers`, `naming-copies-query-slowdown-2026-10` → `docs/agents/this-machine.md` too, under "Looking at the database".
- `doc-review-gate-2026-10`, `derive-once-gate` facts, `precheck-in-cpp-2026-10`, `cleanup-sweep-hook-2026-10`, `single-owner-rules-as-data-2026-10`, `hook-overhaul-2026-10`, `hook-design-audit-2026-10-25`, `push-gate-trusts-gated-main`, `fleet-control-hooks-2026-09`, `shell-gate-change-2026-10`, `fleet-orchestration-lessons`, `review-loop-audit-2026-10-10`, `declare-doc-sources-before-review`, `worktree-folder-locks`, `agents-write-only-their-own-worktree`, `agent-worktrees-under-worktrees-folder`, `browser-tests-dont-flood-tabs` → their rulings go to `rulings.md`; the hook facts belong in `C:\Users\Patrick\.claude\gates\README.md`, which is the HX-P2 plan's file, so T4's report lists each hook fact for the HX-P2 side and the note is archived (exit 3). The notes that describe retired hooks as live (`fleet-control-hooks-2026-09`, `shell-gate-change-2026-10`, `agents-write-only-their-own-worktree`: no gate blocks cross-worktree writes today, checked by grep of `gates\*.py` on 2026-10-31) are archived, not moved.
- `windows-installer`, `mimalloc-link-order`, `srb-format-support`, `dmleaderboards-comparison`, `video-decode-ceilings`, `fc-video-workflow`, `gameplay-video-score-analysis`, `stitched-chart-auto-fills`, `engine-tie-merge-prune`, `squeeze-transfer-scale`, `min-path-delta-proof`, `sp-squeeze-mechanics`, `fill-spawn-comparison`, `ch-score-breakdown-lines`, `ch-note-shuffle-algorithm`, `ch-disco-noflip-rule`, `ch-plain-drums-drops-dynamics`, `early-fill-orange-85-d250`, `drum-lanes-d112-2026-10`, `song-end-one-rule-d255`, `font-roles-d258-2026-10`, `album-search-d263-2026-10`, `one-record-per-chart-profile-2026-10`, `report-covers-whole-library-2026-10`, `midi-drum-track-names-d252`, `paths-tab-starts-closed-2026-10`, `hydra-deluxe-rename`, `backend-hidden-rows-2-9-1` → Out of scope (their homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages). Until the user approves that move they stay in MEMORY.md, but their rulings are copied to `rulings.md` now.

**Exit 3, archived (finished or stale work).** The note moves to `memory\archive\` and its index line goes. `derivation-audit-2026-10-03`, `maintenance-wave-mr-2026-10`, `perf-exploration-2026-10-06`, `storage-audit-2026-10-06`, `memory-audit-2026-10-07`, `report-windows-2026-10`, `settings-summary-button-2026-10`, `preview-keeps-its-place-2026-10`, `analyze-button-follows-chip`, `session-cards-backfill-2026-10`, `preview-solo-hug-sp-gauge-always-2026-10`, `concept-audit-2026-10`, `gui-profile-recipe-2026-10`, `batch-bad-alloc-backlog-2026-10`, `analyze-job-pointer-reuse-bug` (its fact: `AppState::analyze_generation` is gone from `src/`; only a comment in `src/ui/generation.h` line 5 still names it, which T4 reports for a code fix), `ci-runner-stalls`, `ci-yaml-unparsed-here`, `time-on-the-real-db`, `path-builder-and-libraries-2026-10` (its live pointer is the 2026-10-31 redesign-2 handoff; one index line stays until that build ships), and the hook-history notes named in exit 2.

**Exit 4, deleted outright.** Nothing. A note with no ruling and no fact is archived, not deleted, because the archive is cheap and a deletion cannot be checked.

### Rulings that must survive the move (G3)

A grep for the user's quoted words on 2026-10-31 found 75 quotes in 56 note files (49 outside `archive/`). T4 copies every one of them into `rulings.md` before it touches a note. The ones the plan knows by name, because the findings or this planner read them, are:

- "i don't care if an agent is read only, no status updates is unacceptable." (every-brief-carries-status-rule, 2026-09-05)
- "if anything changes a display in the app or a record, run it by me." (card 2026-10-04-1f13cd5d; cited in the findings)
- "the point of doing an audit is to improve, not make the problem worse" and "make a new plan to get rid of every hook we have now and start over from scratch." and "i want to be able to give explicit approval for a full library run" and "rase cap to 3100" (hook-design-audit-2026-10-25)
- "you dont need a full library run to check the rendering. this is exactly the kind of time wasting i am trying to avoid." (render-checks-use-the-corpus)
- "wow you really fucked this up. did you never once do a UI test?" (ui-changes-need-real-app-check)
- "when did they diverge and why? did i ever ask for it?", "what are we already doing and why are you asking me to re derive it", "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?", "yes everything should use one rule" (never-keep-a-split-quietly)
- "implement all nine recommendations as hooks" (fleet-control-hooks-2026-09)
- "use recommended answers" and "Option 3, narrow only" (shell-gate-change-2026-10)
- "repeatedly taking screenshots is very inefficient" (gui-testing-hydra-uitest)
- "Proceed, my call" (ch-mcp-feasibility-2026-10)
- "85 ms both sides" (fill-spawn-comparison), "go with the game's rule" (ch-note-shuffle-algorithm)
- "permanent docs should never reference or be held in scratch folders" (permanent-docs-never-cite-scratch)
- the quotes in `parallel-by-default` (4), `counts-on-screen-must-agree` (3), `no-silent-functionality-changes` (2), `one-reviewer-rule-d107` (2), `report-covers-whole-library-2026-10` (2), `review-loop-audit-2026-10-10` (2), `path-builder-and-libraries-2026-10` (2), and one each in the other files the grep names.

T4's Verify runs the same grep on the old folder (a copy taken before the first edit) and checks every quoted string is a substring of `rulings.md`. The count printed must equal the count found; any miss fails the task.

What this plan drops, on purpose: nothing ruled. What it keeps but moves: every exit 2 and exit 3 note's rulings. The notes that describe hook behaviour the 2026-10-25 rebuild removed are archived, because the behaviour is gone by the user's ruling, and their words are kept in `rulings.md`.

## 6. Tasks

Each task fits one Opus executor in 100 tool calls and 20 minutes. No two own a file in common. All five start together.

### T1. The project CLAUDE.md becomes a table of contents

**Goal.** CLAUDE.md is a map: under 700 words, every section one to three sentences and a pointer. Every rule it held still exists, in CLAUDE.md or in `docs/agents/`. Rule 9 carries the handoff sentence. Rule 2 names the fifth agent type.

**Files.** `CLAUDE.md`; `docs/agents/agent-rules.md` (new); `docs/agents/derive-once-review.md` (section "For the orchestrator" only); `docs/agents/doc-review.md` (new section "What is gated" at the top).

**Acceptance criteria.**
- AC1. CLAUDE.md is under 700 words (whitespace split). Log line: the word count printed in Verify.
- AC2. Every sentence of the old CLAUDE.md (`git show main:CLAUDE.md`) appears verbatim in the new CLAUDE.md or in one `docs/agents/*.md`, or is listed in the commit body as "rewritten: <old> -> <new>" with the new sentence present. The check script in Verify prints `missing: 0`.
- AC3. Rules 1 to 10 keep their numbers and their first sentence. Verify greps the ten first sentences in the new CLAUDE.md.
- AC4. Rule 9 holds the handoff sentence of section 3 word for word (G7). Verify greps it.
- AC5. Rule 2 reads "Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet. Bulk read-and-fill writers, such as the card writers, run on Haiku." The last sentence is new and follows the user's 2026-10-14 choice (`haiku-for-simple-readers`). Verify greps it.
- AC6. The "Derive-once review" and "Doc review" sections of CLAUDE.md are each at most three sentences plus a pointer, and every sentence they lost is in `derive-once-review.md` or `doc-review.md` (covered by AC2).
- AC7. The docs test passes: `hydra_tests.exe "-tc=docs match code*"` prints its pass line (G5, G6).
- AC8. "Past sessions" keeps the card folder path and the index rule unchanged (its sentences pass AC2 unchanged).

**Steps.** Read CLAUDE.md, `derive-once-review.md` "For the orchestrator", `doc-review.md` and `brief-preamble.md`. Write `docs/agents/agent-rules.md`: the ten rules with every detail sentence CLAUDE.md holds today, plus the "Hooks check three of these" paragraph. Add to `derive-once-review.md` "For the orchestrator" every sentence of CLAUDE.md "Derive-once review" it does not already hold. Add "What is gated" to `doc-review.md` with every sentence of CLAUDE.md "Doc review". Rewrite CLAUDE.md: each section becomes its short form and a pointer. Rules 1 to 10 become one line each, first sentence kept. Put the section 3 sentence into rule 9 and the Haiku sentence into rule 2. Write the AC2 check script in the scratch folder (split old file into sentences at `. `, strip markdown, search each in the new files; print `missing: N` and the misses). Run the docs test. Commit.

**Verify.** `(Get-Content CLAUDE.md -Raw).Split(...).Count` under 700; the AC2 script prints `missing: 0`; `Select-String` for the rule-9 sentence and the rule-2 sentence each print one hit in CLAUDE.md; `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` passes.

### T2. The preamble, the handoff guide and the `/handoff` command

**Goal.** The preamble carries the handoff sentence and says how a handoff is checked. `docs/agents/handoff.md` holds the shape of section 3 and the checker interface. `/handoff` writes one.

**Files.** `docs/agents/brief-preamble.md`; `docs/agents/handoff.md` (new); `.claude/commands/handoff.md` (new); `docs/agents/integrate.md` (line 42 only).

**Acceptance criteria.**
- AC1. The preamble's "The code is the source of truth" section holds the section 3 sentence word for word (G7). Verify greps it.
- AC2. The preamble has a new paragraph "Continuing from a handoff": run the handoff checker, read every row's claim against the code, and a row that fails is reported before any work starts. Verify greps "handoff_check.py" in the preamble.
- AC3. `docs/agents/handoff.md` holds the `## Claims` table with the five kinds and the "How to check" column, the no-abbreviated-hash rule, the no-scratch-path rule, and the checker command and its output lines exactly as section 3 states them. Verify greps `## Claims`, `handoff_check.py`, `not-on-main`.
- AC4. `.claude/commands/handoff.md` exists, reads `docs/agents/handoff.md` by relative path, and names the checker. Verify: file exists; grep for `docs/agents/handoff.md` and `handoff_check.py`.
- AC5. `integrate.md` line 42 no longer says "once it exists"; the docs test command stays. Verify greps "once it exists" in integrate.md and gets 0 hits.
- AC6. The preamble's "Review findings" section and the budget paragraph still say a reviewer lists unfixed findings in Notes at the warning (rule 5 unchanged). Verify greps "lists the rest in its review's Notes".
- AC7. Docs test passes (G5, G6).
- AC8. The preamble grows by at most 150 words. Verify prints the before and after counts.

**Steps.** Read the preamble, section 3 of this plan, `integrate.md`. Edit the preamble: add the sentence under "The code is the source of truth" and the new paragraph after it. Write `handoff.md` from section 3. Write the command file: frontmatter `description: Write a checkable handoff for the next session`, then the steps from section 3 with `$ARGUMENTS` as the slug. Fix integrate.md line 42. Run the docs test. Commit.

**Verify.** The greps in AC1 to AC6; the docs test's pass line; the word counts.

### T3. The global side: posture text and agent type files

**Goal.** The posture text carries the handoff sentence and the Haiku type. `reviewer.md` agrees with rule 5. `scout.md` lets a scout write its scratch folder. No file in `~\.claude\gates\` is touched.

**Files.** `C:\Users\Patrick\.claude\model-postures.md`; `C:\Users\Patrick\.claude\agents\reviewer.md`; `C:\Users\Patrick\.claude\agents\scout.md`; `C:\Users\Patrick\.claude\agents\executor.md`; `C:\Users\Patrick\.claude\agents\planner.md`; `C:\Users\Patrick\.claude\CLAUDE.md` (read only; no change planned, the file is 288 words of style rules the user wrote).

**Acceptance criteria.**
- AC1. `model-postures.md` holds the section 3 sentence word for word (G7), after "Never trust another agent's summary." Verify greps it.
- AC2. The posture text stays under 650 words (549 today). Verify prints the count.
- AC3. `reviewer.md` keeps "Never pass a change by listing an unmet promise as a "drift" or a "review note"" and adds, right after it, the rule 5 exception in these words: "The one exception is the budget warning: at 100 calls or 20 minutes you stop fixing, sign off what you fixed, and list every unfixed finding under a Notes heading in your report, each with the plan line it breaks (CLAUDE.md rule 5)." Pending the user's answer to Q1; if the user picks the other side, the executor writes that side instead and the reviewer checks the brief. Verify greps "Notes heading".
- AC4. `scout.md` says "Change no files except your status line and files in your own scratch folder." Verify greps it.
- AC5. `executor.md` and `planner.md` gain one sentence: "If your brief points you at a handoff, check its Claims table against the code before you build on it (`docs/agents/handoff.md`)." Verify greps it in both.
- AC6. `git diff --stat` on the branch touches only the files listed. Verify prints it.

**Steps.** Cut a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\harness-agents`. Make the edits. Commit with the preamble's form. The main session dispatches the reviewer with the hooks repo's root folder written in the brief, as CLAUDE.md "Derive-once review" says for the other repo.

**Verify.** The greps in AC1, AC3, AC4, AC5; the word count; `git diff --stat main...HEAD`.

### T4. The memory folder and `docs/agents/rulings.md`

**Goal.** Every user ruling held in a memory note is in `docs/agents/rulings.md`. MEMORY.md holds only exit 1 notes and the exit 2 notes the user has not yet approved to move (section 5). Exit 3 notes are in `archive/`. Index lines match their files. Broken pointers in the kept notes are fixed.

**Files.** `docs/agents/rulings.md` (new); `docs/agents/this-machine.md` (new); everything under `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\` including `MEMORY.md` and `archive\`.

**Acceptance criteria.**
- AC1. Before the first edit, the executor copies the memory folder to its scratch folder. The Verify grep for quoted user words (the pattern `(user|User)[^\n]{0,40}(said|ruled|answered|verbatim|exact words|:)\s*["“]` over `*.md`, which found 75 on 2026-10-31) runs on the copy, and every quoted string found is a substring of `rulings.md`. Log line: `quotes: 75 found, 0 missing` (the number found must equal the pre-edit count).
- AC2. `rulings.md` has one line per ruling: date, the words in quotes, the session id or card id, and the rule it backs (a CLAUDE.md rule number, a `docs/agents/` page, or "no page yet").
- AC3. MEMORY.md is under 60 lines and under 1,000 words. Verify prints both.
- AC4. Every file named in MEMORY.md exists in the memory folder, and every file in the folder (not `archive/`) is named in MEMORY.md. Verify: a script prints `index: 0 missing, 0 unindexed`.
- AC5. No kept note names `tools\derive_once_precheck.ps1`, `gates\doc_review_sources.ps1`, `hooks\state\status`, "every 10 tool calls" or "10 calls". `refused-merge-abort` says the key is `git rev-parse <branch>` and the ref gate's refusal names it (`main_move_check.ps1` is in the retired `hooks\` folder, not `gates\`). Verify greps each string in the kept notes and gets 0 hits.
- AC6. `this-machine.md` holds the exit 2 machine facts, and the docs test passes on it (G6).
- AC7. No note is deleted: `(Get-ChildItem memory -Recurse -File).Count` after equals the count before. Verify prints both.
- AC8. T4's report lists: each note whose exit changed after reading; each hook fact for the HX-P2 side; each fact bound for a page outside this plan (for the Out of scope list); the `generation.h` comment.
- AC9. The docs test passes (G5, G6).

**Steps.** Copy the folder. Run the quote grep on the copy and write `rulings.md`. Read each note in section 5's lists in the order given, sorting as it goes; a note that reads differently from its index line is re-sorted and listed. Move exit 3 notes into `archive/` with `Move-Item` (no recursive delete). Rewrite the kept notes named in AC5. Write `this-machine.md`. Rewrite MEMORY.md. Run the AC1, AC4, AC5 scripts and the docs test. Commit `rulings.md` and `this-machine.md`. Report.

**Verify.** The log lines of AC1, AC3, AC4, AC5, AC7; the docs test's pass line. The reviewer's brief names the memory folder beside the commit (G9).

### T5. Broken pointers and the GUI-check split in `docs/agents/`

**Goal.** `clone-hero-mcp.md` names only files that exist. `ui-testing.md` says when the headless test is enough and when the real app on the user's library is also owed, in the terms the user ruled. The uitest page absorbs the memory traps T4 moves (section 5, exit 2).

**Files.** `docs/agents/clone-hero-mcp.md`; `docs/agents/ui-testing.md`.

**Acceptance criteria.**
- AC1. `clone-hero-mcp.md` line 31 no longer names `tools/ch_mcp/start_method.json` (`Test-Path` false on 2026-10-31). The executor finds which file the kit and the song start really read (`tools/ch_mcp/kit_methods.json` and `tools/ch_mcp/start_candidates.json` exist; `tools/ch_mcp/tests/test_frida_attach.py` names neither `start_method` nor `start_candidates`, checked by grep) and names that file, or says the list is not kept in a data file yet. Verify greps `start_method.json` in `docs/agents/` and gets 0 hits, and every path the line names passes `Test-Path`.
- AC2. `ui-testing.md` opens with a three-part rule that keeps all three rulings: `hydra_uitest` replaces screenshots for checking behaviour and text; a change to what the user sees is also seen in the real app with the user's `hydra_ui.ini` and a copy of their database before merge ("did you never once do a UI test?", 2026-10-10); and a rendering check runs on `testdata/input`, never on a whole-library analysis ("you dont need a full library run to check the rendering", 2026-10-14). The three are not in conflict once each names its case; Q3 asks the user to confirm that reading. Verify greps the three quoted phrases.
- AC3. The traps from `gui-testing-hydra-uitest`, `report-capture-scripts` and `branch-window-check-recipe` are in `ui-testing.md` under a "Traps" heading, each as one sentence that points at the owner file rather than restating a rule. Verify greps "## Traps".
- AC4. Docs test passes (G5, G6).

**Steps.** Grep `tools/ch_mcp` for the start list's real owner. Fix the line. Read the three memory notes and the top of `ui-testing.md`. Write the opening rule and the Traps section. Run the docs test. Commit.

**Verify.** The greps in AC1 to AC3; `Test-Path` on each path in the fixed line; the docs test's pass line.

## 7. What the plan leaves to HX-P2

The handoff checker (section 3 interface), the posture delivery (once per session or only on typed prompts), the refusal log, the `ref_judge.py` trailer wording ("or orchestrator": the preamble never uses that word, and this plan does not add it; the HX-P2 plan either drops the phrase or defines it), the pointer check for memory and agent docs, and `gates\README.md`. T4's report hands the hook facts over.

## 8. Questions for the user

Each question quotes both sides, gives an example from each end, names the options and a recommendation, and the strongest case against it.

**Q1. The reviewer's unmet promises.** `C:\Users\Patrick\.claude\agents\reviewer.md` says: "Never pass a change by listing an unmet promise as a "drift" or a "review note"." CLAUDE.md rule 5 says: "A reviewer that runs out of time or tool calls signs off what it fixed and lists the rest in the Notes section of its review; the main session passes those Notes to the user, and the merge does not wait for them." Example from the first side: a reviewer with 40 calls left finds a plan line not built and writes it up as a note instead of fixing it; that is the dodge the sentence forbids. Example from the second: a reviewer at 100 calls has three findings left and no budget; rule 5 says sign off, list them, merge. Options: (a) keep both, with the budget warning as the one exception (T3 AC3 writes this); (b) drop rule 5's Notes clause so every unmet promise blocks; (c) drop the reviewer.md sentence. Recommendation: (a). Case against it: a reviewer can spend its budget slowly and reach the warning with findings it chose not to fix, and (a) lets that through; only the main session reading the Notes catches it.

**Q2. The Haiku agent type.** CLAUDE.md rule 2 says: "Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet." The memory note `fleet-control-hooks-2026-09` records 2026-10-09: "no work goes to Haiku". The memory note `haiku-for-simple-readers` records 2026-10-14: the user switched the card writers to Haiku, and `agents\card-writer.md` has `model: haiku`. Example from the first side: a reviewer on Haiku would miss code findings. Example from the second: a card writer reads a condensed transcript and fills a template, with `check.py` behind it. Options: (a) rule 2 gains the Haiku sentence of T1 AC5 (the later ruling wins); (b) card writers move to Sonnet and the Haiku type goes. Recommendation: (a). Case against it: a fifth model line is one more place for a brief to pick wrong, and the 2026-10-09 "no Haiku" ruling was never withdrawn in words, only overtaken.

**Q3. The GUI check.** CLAUDE.md "GUI testing" says: run `hydra_uitest` "instead of launching the app and taking screenshots". Memory `ui-changes-need-real-app-check` says a layout change is seen "in the real app on the user's real library and settings" before merge. Memory `render-checks-use-the-corpus` says a rendering check runs on `testdata/input`, "never the user library". Example of the first: a button's label changes; `hydra_uitest` reads the text and that is enough. Example of the second: a column-width rule; headless tests passed and the real Library came up squeezed. Example of the third: a report's cell alignment; the corpus shows it in 30 seconds and the library took 75 seconds of re-analysis. This plan reads the three as three cases of one rule (T5 AC2): behaviour and text by uitest; what the user sees also in the real app with the user's ini and db copy; rendering on the corpus, never a whole-library analysis. Options: (a) confirm that reading; (b) every UI change needs the real app on the library; (c) the real-app check is dropped. Recommendation: (a). Case against it: the line between "behaviour" and "what the user sees" is drawn by the agent, and the 2026-10-10 regression was an agent drawing it wrong.

**Q4. Where `/handoff` lives.** Project scope (`hydra-test\.claude\commands\handoff.md`, committed, reviewed) or user scope (`~\.claude\commands\handoff.md`, works in both repos). The docs say the two scopes must not share a name. Example: a hooks-repo session that wants `/handoff` gets nothing under project scope. Options: (a) project scope in Hydra only (T2 builds this); (b) user scope only; (c) both under different names. Recommendation: (a). Case against it: the hooks repo's sessions write handoffs too (five hook handoffs exist under `docs/handoffs/`), and they would have to write one by hand.

**Q5. What "TOP RULE" means.** Seventeen index lines carry it; nothing defines it. Options: (a) it means "a standing rule the user gave in their words; it stays in MEMORY.md and in `rulings.md`" (T4 keeps those 17 as they are); (b) drop the label and let `rulings.md` carry the weight. Recommendation: (a). Case against it: a label with a meaning invites the next session to add an eighteenth without asking, and the index grows again.

**Q6. The bullet-wall Stop check.** The findings claim the old `stop_bullet_check` hook caught bullet walls twice, and the 2026-10-25 rebuild dropped it with the other style gates (it is in the "Gone with nothing in its place" list of `docs/superpowers/plans/2026-10-25-hooks-from-scratch.md`, section 2, checked). Bringing it back is a hook, so it is the HX-P2 plan's question; this plan keeps the plain-English rule as an instruction only. It is listed here so the user sees that both plans leave it as a question and neither builds it unasked.

**Q7. Moving project facts into pages this plan does not own.** Section 5 exit 2 names about 28 notes whose homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages. Options: (a) approve a follow-up task that moves them, with the Clone Hero facts going on their pages as CLAUDE.md "Clone Hero reference" demands; (b) leave them in MEMORY.md. Recommendation: (a). Case against it: it is a sixth task touching pages other work is editing now (the Clone Hero reference), so it waits for a quiet moment.

## 9. Findings claims checked, and the ones that did not hold

Checked true on 2026-10-31: `tools/ch_mcp/start_method.json` is missing (`Test-Path` false; `start_candidates.json`, `kit_methods.json`, `menu_fields.json`, `menu_steps.json` exist). `integrate.md` line 42 says "once it exists". `ref_judge.py` line 229 says "or orchestrator". `reviewer.md` line 11 and `scout.md` line 9 read as the findings say. `card-writer.md` has `model: haiku` and rule 2 names four types. `fleet-control-hooks-2026-09` says "no work goes to Haiku" and "every 10 tool calls"; `every-brief-carries-status-rule` says "every 10 tool calls or 5 minutes" and `hooks\state\status`. `refused-merge-abort` names `main_move_check.ps1`, which exists in `hooks\` and not in `gates\`. The word counts (CLAUDE.md 2,225, MEMORY.md 2,116, preamble 2,373, posture 549) match the findings within one word.

Claims that did not hold, or hold only in part:

1. "123 memory files": the folder holds 124 files outside `archive/` and 64 inside it.
2. "`AppState::analyze_generation` is gone": the member is gone from `src/`, but a comment at `src/ui/generation.h` line 5 still names it, so a grep for the name is not empty. The note is still stale; the code comment is a separate fix (T4 AC8 reports it).
3. "a hook blocks cross-worktree writes" (memory `agents-write-only-their-own-worktree`): no file in `gates\*.py` reads a write path against a worktree; only `delete_rule.py` and `review_dispatch.py` mention worktrees. The note describes a retired hook, so T4 archives it.
4. The findings say `docs/agents/clone-hero-mcp.md` "cites" the missing file once; the line names it twice (the call list and the field-set list).

## 10. Out of scope (for the user to approve)

- Moving the exit 2 project-fact notes whose homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages (Q7).
- The `src/ui/generation.h` line 5 comment that names a member that no longer exists (a code change; `comments-explain-current-code`).
- The hooks repo's `gates\README.md` facts T4 hands over, and every hook (HX-P2 plan).
- Trimming the global `C:\Users\Patrick\.claude\CLAUDE.md`: it is 288 words of the user's own style rules and this plan leaves it alone.
- The session-cards move into `docs/` and the finished-handoff sweep (`docs/handoffs/2026-10-31-session-cards-into-docs-and-handoff-cleanup-handoff.md`): T1 keeps "Past sessions" pointing at today's folder.

## 11. Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every CLAUDE.md rule survives the move | G1, section 4 | T1 | AC2 script prints `missing: 0` |
| Rule numbers stay 1 to 10 | G2 | T1 | AC3 grep of ten first sentences |
| CLAUDE.md under 700 words | section 4, T1 AC1 | T1 | word count printed |
| Rule 9 carries the handoff sentence word for word | section 3, G7 | T1 | AC4 grep |
| Rule 2 names the Haiku type (pending Q2) | T1 AC5 | T1 | grep of the sentence |
| Review sections become short form with pointers | section 4, T1 AC6 | T1 | AC2 and a sentence count |
| "Past sessions" keeps the card folder | section 4, T1 AC8 | T1 | AC2 unchanged sentences |
| Preamble carries the sentence word for word | G7, T2 AC1 | T2 | grep |
| Preamble says how to continue from a handoff | section 3, T2 AC2 | T2 | grep `handoff_check.py` |
| Handoff guide holds the Claims table and checker interface | section 3, T2 AC3 | T2 | greps of `## Claims`, `handoff_check.py`, `not-on-main` |
| `/handoff` command exists and names the guide and checker | section 3, T2 AC4 | T2 | file exists, greps |
| Handoffs name no scratch path | section 3 | T2 | instruction in `handoff.md`; AC3 grep "scratch" |
| Hashes written in full | section 3 | T2 | AC3 grep "40-character" in `handoff.md` |
| integrate.md line 42 fixed | section 1 item 4, T2 AC5 | T2 | grep 0 hits |
| Reviewer Notes rule unchanged in the preamble | T2 AC6 | T2 | grep |
| Preamble grows at most 150 words | T2 AC8 | T2 | counts printed |
| Posture carries the sentence word for word | G7, T3 AC1 | T3 | grep |
| Posture under 650 words | T3 AC2 | T3 | count printed |
| reviewer.md agrees with rule 5 (pending Q1) | T3 AC3 | T3 | grep "Notes heading" |
| scout.md allows the scratch folder | T3 AC4 | T3 | grep |
| executor and planner check handoff claims | T3 AC5 | T3 | grep in both |
| T3 touches no gates file | G4, T3 AC6 | T3 | `git diff --stat` |
| Every ruling survives in rulings.md | G3, section 5 | T4 | AC1 `quotes: N found, 0 missing` with N equal to the pre-edit count |
| rulings.md rows carry date, words, id, rule | T4 AC2 | T4 | reviewer reads; AC1 covers the words |
| MEMORY.md under 60 lines and 1,000 words | section 5, T4 AC3 | T4 | counts printed |
| Index and folder agree | section 5, T4 AC4 | T4 | `index: 0 missing, 0 unindexed` |
| No kept note names a gone file or the old status cadence | section 5, T4 AC5 | T4 | greps 0 hits |
| refused-merge-abort pointer fixed | section 5, T4 AC5 | T4 | grep `main_move_check` 0 hits in kept notes |
| No note deleted, only archived | section 5 exit 4, T4 AC7 | T4 | file counts equal |
| TOP RULE notes untouched pending Q5 | section 5 | T4 | reviewer diffs the 17 files against the copy |
| Hook facts handed to HX-P2 | section 5 exit 2, T4 AC8 | T4 | report lists them |
| this-machine.md holds the machine facts | section 5, T4 AC6 | T4 | docs test passes; reviewer checks the five notes' facts |
| start_method.json pointer fixed | section 1 item 4, T5 AC1 | T5 | grep 0 hits; `Test-Path` on named paths |
| GUI rule keeps all three rulings (pending Q3) | T5 AC2 | T5 | greps of the three quoted phrases |
| uitest traps moved into ui-testing.md | section 5 exit 2, T5 AC3 | T5 | grep `## Traps` |
| No permanent doc cites scratch | G5 | T1, T2, T4, T5 | docs test pass line |
| Every docs/agents page passes the docs test | G6 | T1, T2, T4, T5 | docs test pass line |
| No new hook from this plan | G4 | all | `git diff --stat` on each branch shows no `gates\` or `settings.json` file |
| Tasks share no file | G8 | all | the Files lists above are disjoint (reviewer checks) |
| Hydra commits get one fresh review; T4's reviewer reads the memory folder | G9 | all | the main session's dispatch; the T4 brief names the folder |
| Style rules stay instructions, no Stop hook from this plan | section 1 item 5, Q6 | none | Out of scope; question Q6 |
| Project facts bound for other pages move only with approval | section 5 exit 2, Q7 | none | Out of scope |
