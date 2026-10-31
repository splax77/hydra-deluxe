# Harness cleanup, part 1: instructions and memory (2026-10-31)

Task HX-P1, session 6777b546. This is one of two plans. The other, the HX-P2 plan, owns the hooks and the tools (the handoff checker, the refusal log, the posture delivery, the pointer check). This plan owns the words: the project `CLAUDE.md`, `docs/agents/`, the global `CLAUDE.md`, the posture text, the agent type files, the memory folder and a `/handoff` command. Nothing here is built yet. Every task waits for the user's yes (CLAUDE.md rule 3).

## 1. What this plan does, in plain words

The user approved a cleanup of the harness on 2026-10-31 ("yes plan all of this") and added one rule in their words: "for the rule that says never trust another agent's summary, that includes handoffs. before starting any new work, every claim in the handoff needs to be checked against what actually exists. the code is the source of truth."

So this plan does five things.

1. It writes the handoff rule into the three places every session reads (CLAUDE.md rule 9, the preamble, the posture text), gives handoffs a shape whose claims can be checked, and adds a `/handoff` command that writes one in that shape.
2. It shrinks what loads every time. The project CLAUDE.md (2,225 words, counted on 2026-10-31 by splitting on whitespace) becomes a table of contents: its three long procedures (the two review sections and the agent rules) shrink to short forms with pointers, and their full text moves into `docs/agents/`. `MEMORY.md` (2,139 words when re-counted at review, 124 note files plus 64 in `archive/`; sessions keep adding notes, so every count in this plan is a snapshot) shrinks to the notes that are still live.
3. It loses no ruling. Every sentence of the user's own words that a note holds, quoted or stated as the user's decision, is copied into one repo page, `docs/agents/rulings.md`, with its date and session id, and the note itself is only moved, never edited, except to fix a stale file name. Section 5 lists what is kept and what is dropped.
4. It fixes the contradictions and broken pointers the review found, and where a fix would pick one side of a split, it asks the user instead (section 8).
5. It keeps the 2026-10-25 rulings: style rules stay as instructions, no new hooks come from this plan, and the hook line cap is not touched.

Each claim in the review findings that this plan builds on was checked against the files on 2026-10-31. The ones that did not hold are in section 9.

### Prior art

Two ideas from outside shaped this plan. Claude Code's own docs say a project slash command is a markdown file under `.claude/commands/` and a user command sits under `~/.claude/commands/`; the file name is the command name, and the two scopes must not share a name ([Claude Code slash commands](https://docs.claude.com/en/docs/claude-code/slash-commands)). Several practitioners say the same thing about instruction files: keep the always-loaded file a map, not an encyclopedia, and let the agent open the long page only when the task needs it ([your CLAUDE.md should be a map](https://contra.com/community/HDedmzbW-your-claudemd-should-be-a-map); [progressive disclosure for agents](https://codemyspec.com/blog/progressive-disclosure)). None of them is an official limit; the numbers in this plan are ours.

## 2. Global constraints

These hold for every task. "G" is a global constraint and "AC" (in section 6) is an acceptance criterion. The "docs test" is Hydra's own test that reads the docs and checks them against the code (G5, G6).

- G1. Every rule in the project CLAUDE.md today survives, in the same words or plainer ones, in CLAUDE.md or in a `docs/agents/` page. Nothing is dropped by the move. T1 proves it with a sentence check (its Verify).
- G2. Rule numbers 1 to 10 keep their numbers. The preamble, `model-postures.md`, `derive-once-review.md` and `integrate.md` cite rules by number.
- G3. Every user ruling held in a memory note that T4c removes, archives or rewrites is copied, in the words the note holds, into `docs/agents/rulings.md`. A ruling is any sentence a note quotes from the user, and also any line in which a note states what the user ruled, decided, chose, wanted or ordered without quoting it. The second kind is copied as the note words it, marked "as the note records it", and is never reworded. T4 proves both kinds with the checks in its Verify. A kept note is edited only to fix a stale file or script name, on that sentence alone.
- G4. No task adds a hook, a gate or a settings.json rule. That is the HX-P2 plan's side. (The user's 2026-10-25 ruling: "make a new plan to get rid of every hook we have now and start over from scratch", and style rules are instructions only.)
- G5. No permanent doc cites a scratch folder. The docs test's scratch check (`hydra_tests.exe "-tc=docs match code*"`) runs in every Hydra task's Verify.
- G6. Every `docs/agents/` page passes the docs-vs-code test (`tests/test_docs_match_code.cpp` reads every `docs/agents/*.md`; a backticked name that is not in the code fails it).
- G7. The three places that state the handoff rule (CLAUDE.md rule 9, the preamble, the posture text) use the one sentence in section 3, word for word, so a later edit to one of them is caught by a grep.
- G8. Tasks run in parallel (T4a, T4b and T4c are three parallel tasks, section 6). No two tasks own the same file. T3 runs in a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\`; the other tasks run in worktrees under `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\`.
- G9. Each Hydra task's commit goes through one fresh reviewer, dispatched as CLAUDE.md "Derive-once review" says. The memory folder is in no git repo, so T4c leaves no commit to review. The main session dispatches T4a's reviewer after T4c has reported, and that reviewer's brief names the memory folder (and the pre-edit copy) as a second thing to read, beside the committed `rulings.md`; it checks T4c's acceptance criteria there.
- G10. Each task is sized for one executor: at most 100 tool calls and 20 minutes (the preamble's limits). A task that runs out hands the rest to a fresh agent; nothing in this plan needs more than that.
- G11. Nothing builds until the user has answered the questions in section 8. The main session writes each answer into the brief of the task it touches: Q1 to T3, Q2 to T1, Q3 to T5, Q4 to T2, Q5 and Q7 to T4c, Q8 to T3. A task whose question is open does not build the line marked "pending".

## 3. The handoff rule and the handoff shape

### The sentence

The one sentence, used word for word in three places (G7):

> A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong.

CLAUDE.md rule 9 becomes: "Never trust another agent's summary or the comments it left behind. The code and the source data are the source of truth. Check a claim against them before you repeat it or act on it. A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong. `docs/agents/handoff.md` says what a handoff holds and how to check one."

### The shape

A handoff holds prose and one table. The prose says the goal, what was done and what is next, in plain English. The table, under a heading that is exactly `## Claims`, lists every fact the next session will act on. One row per claim, four columns. The Value column is written in backticks, because the checker reads backticked tokens (see below):

| Kind | Value | Claim | How to check |
|---|---|---|---|
| commit | the full 40-character hash, in backticks | what the commit holds, and whether it is on `main` or pushed | the checker finds the hash; `git branch --contains <hash>`; `git log origin/main --oneline -1` |
| branch | the branch name, in backticks | what the branch holds and whether it is merged | the checker finds the branch; `git log main..<branch>` |
| path | a repo-relative path, or a full absolute path outside the repo, in backticks | what the file holds | the checker finds the file |
| function | `name()` or `Class::method`, in backticks | what it does and where the handoff says it lives | the checker finds it with `git grep` |
| test | the exact command, in backticks | the line it printed last time (pass count, or the one red line) | run it; the same line prints |
| ruling | the session id or card id | the user's words, in quotes | the card or transcript holds those words |
| state | a plain fact ("the worktree X exists", "Y is not yet merged") | the fact | the command that shows it |

Rules for the table. A claim with no row is not a claim, and the next session may not act on it. A row whose check fails is wrong until someone fixes the handoff. A hash is written in full, never abbreviated. A handoff names no scratch path (G5; `docs/handoffs/` is skipped by the docs test's scratch check today, `kNotPermanentDirs` in `tests/test_docs_match_code.cpp`, so the rule holds by instruction there). The checker does not judge the Claim column or the `test`, `ruling` and `state` rows; the session reads those against the code and runs the tests.

### What this plan needs from the handoff checker in the HX-P2 plan

This plan writes the shape; the HX-P2 plan writes the script that checks it (its task T6, on branch `claude/harness-plan-hooks`, in the Hydra repo under `tools/`, so it does not count against the hook line cap). This plan copies its interface from that task and does not invent its own:

- The command is `py -I tools/check_handoff.py <handoff.md> [--repo <root>] [--since <YYYY-MM-DD>]`. The repo defaults to the one that holds the script. The date defaults to the date in the handoff's file name.
- It reads every token in backticks, plus bare hex words and bare paths, and sorts each by shape: a hex word is a commit, a branch name git knows is a branch, a token with a path separator or a known extension is a path, `name()` or `Name::name` is a function. A token of none of these shapes is ignored, so prose in backticks never fails the check. A line that names the hooks repo's root folder is also checked in that repo.
- It has two modes. A handoff with a `## Claims` table (every new one) is read row by row: each row prints one status word, `ok`, `missing`, `not-on-main`, `not-pushed` or `manual`, and the run exits 0 only when every `commit` and `path` row is `ok`. An older handoff with no table is scraped as described above.
- In scrape mode it prints three sections, `EXISTS`, `MISSING` and `CHANGED SINCE <date>`, then one summary line, `checked N claims: E exist, M missing, C changed since <date>`. It exits 0 when nothing is missing, 1 when anything is, 2 on a bad argument.
- It is run by the `/handoff` command after writing a handoff, and by the main session before continuing one. A `MISSING` line or a `CHANGED SINCE` line is reported before any work starts. The checker proves a name exists; whether the handoff's sentence about it is true is still the session's reading of the code.

The checker has not been built, so nothing in this plan runs it. If the HX-P2 plan changes the command or the output words before it builds, `docs/agents/handoff.md`, the preamble paragraph and `.claude/commands/handoff.md` take its names, and the reviewer of whichever lands second makes them agree.

### The `/handoff` command

A project command, `C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\commands\handoff.md`, so it is committed and reviewed with the repo and reads `docs/agents/handoff.md` by a relative path. The repo's `.gitignore` line 33 ignores everything under `.claude/` except `output-styles/` (`git check-ignore -v .claude/commands/handoff.md` names that line on 2026-10-31), so T2 also adds one exception line, `!.claude/commands/`, beside the existing `!.claude/output-styles/` one; without it the command could not be committed. The hooks repo keeps no copy, because its own handoffs are rare and the user scope would shadow the project one if both held a `handoff.md` (the docs say the two scopes must not share a name). The command's text: stop launching work; write `docs/handoffs/<today>-<slug>-handoff.md` in the shape above, with the user's rulings from this session in their words; run the handoff checker on it (`py -I tools/check_handoff.py`), fix any `MISSING` line in the handoff, and paste the summary line into the handoff under `## Checker output`; commit the handoff on a branch; report the path. `$ARGUMENTS` is the slug.

## 4. The project CLAUDE.md as a table of contents

Today CLAUDE.md is 2,225 words, and about three quarters of them are procedures: "Derive-once review" (about 425 words), "Doc review" (about 439), "Agent rules" (about 819). The two review sections repeat what `docs/agents/derive-once-review.md` ("For the orchestrator") and `docs/agents/doc-review.md` already say, in a second wording.

After T1, CLAUDE.md is under 1,000 words. The arithmetic: the sections T1 leaves alone (the intro, agent skills, Clone Hero reference, the three tracker lines, Past sessions, Audits, GUI testing, Public doc style) add up to 542 words by the same whitespace count. The ten rules in one-line form, the handoff sentence in rule 9 and the Haiku sentence in rule 2 take about 190. The two review sections in short form take about 150. That is about 880. A target of 700 was in the first draft of this plan; it cannot hold while the untouched sections and the "Past sessions" text stay as they are (AC8), so the plan sets 1,000, our own number like the others here. T1 shortens only the two review sections and the agent rules; every other section stays word for word, because their long text has no `docs/agents/` page that T1 owns to move into. Each shortened section is a heading, one to three sentences that say the rule in short, and one pointer to the page that holds the full text. The ten agent rules stay as ten numbered one-line rules (G2) with the pointer `docs/agents/agent-rules.md` for their details. The two review procedures move into the pages that already own them: the orchestrator's steps into `docs/agents/derive-once-review.md` "For the orchestrator", and the doc-review gate rules (what is gated, the small-text pass, the `publish.ps1` rule, the waiver) into a new "What is gated" section at the top of `docs/agents/doc-review.md`.

The "Past sessions" section stays word for word until the session-cards handoff (`docs/handoffs/2026-10-31-session-cards-into-docs-and-handoff-cleanup-handoff.md`) moves the cards. T1 must not change where the cards live.

## 5. The memory folder: what is kept, what moves, what goes

`MEMORY.md` is 124 index lines. Seventeen say "TOP RULE". The label has no written meaning today. This plan gives it one (section 8, question Q5 asks the user to confirm): a note marked TOP RULE is a standing rule the user gave in their words, and it stays in MEMORY.md; everything else is either a repo doc or an archived note. Until the user answers, T4c keeps every TOP RULE note and its index line, changing only a stale file or script name on the one sentence that holds it (`one-reviewer-rule-d107` line 15 names `main_move_check.ps1`, which is not in `gates\`). That includes `hook-design-audit-2026-10-25`, which is a TOP RULE note and is therefore not archived with the other hook-history notes.

T4c sorts every note into one of four exits. The sort below is this planner's reading of the index lines on 2026-10-31. Every note in the folder that day is named in the exit lists below (checked by a script at review: the one that was missing, `measure-memory-without-harness`, is now in exit 1). A note added to the folder after that day is not sorted by this plan: T4c leaves it and its index line as they are and names it in its report. Five index lines are claimed to disagree with their files (`ch-mcp-server-2026-10`, `album-search-d263-2026-10`, `doc-review-gate-2026-10`, `concept-audit-2026-10`, `every-brief-carries-status-rule`). One real case, checked at review: the index line for `ch-mcp-server-2026-10` says "LS-K0 focus grab accepted by user, LS-K1 and LS-K2 passed", while the note's own description says "LS-K0 failed on focus (0.5 s grab at launch); LS-K1 to LS-K5 not run". Neither text is the source of truth; T4c decides which is current from `docs/agents/clone-hero-mcp.md`, the git log of `tools/ch_mcp/` and the newest handoff on the kit, and writes only what they show. T4c reads each of the five files before it sorts it, and its report lists any note whose exit changed after reading.

**Exit 1, stays in MEMORY.md (a standing rule, user-ruled).** The 17 TOP RULE notes, plus these RULE notes that hold a user ruling or a rule no repo page holds: `check-long-captures-early`, `name-the-owner-across-boundaries`, `merge-tests-touch-changed-files`, `repo-scans-read-main-checkout`, `error-lines-derived-once`, `comments-explain-current-code`, `boundary-cases-follow-the-rule`, `read-fixed-values-once`, `every-number-says-where-it-comes-from`, `one-word-one-meaning-in-public-text`, `derive-display-from-engine-truth`, `fail-loudly-never-freeze`, `fix-hooks-dont-bypass`, `no-silent-functionality-changes`, `measure-small-explain-first`, `use-source-facts-not-proxies`, `questions-describe-the-real-rule`, `verify-before-claiming-a-source-is-wrong`, `names-from-data-not-recall`, `search-web-before-finalizing-plans`, `stage-by-name-in-shared-checkout`, `refused-merge-abort` (with its pointer fixed, see below), `haiku-for-simple-readers`, `check-the-screen-against-the-data`, `render-checks-use-the-corpus`, `relay-status-in-plain-english`, `session-references-only-itself`, `lean-agent-fanout`, `one-place-promise-covers-every-question`, `every-brief-carries-status-rule` (rewritten: status every 5 minutes, helper under `gates\`, the user's words kept), `db-upgrade-freeze-2026-10-07`, `stored-versions-one-rule`, `ch-runs-must-not-need-focus`, `ch-mcp-feasibility-2026-10`, `measure-memory-without-harness` (a RULE note from 2026-10-31, D282). Their rulings are also copied into `rulings.md` (G3 applies to every note T4c rewrites, and copying the rest costs one pass).

**Exit 2, moves into a repo doc, then leaves MEMORY.md.** Project facts that belong where the code's readers look. The target page must be one this plan owns (`docs/agents/`); facts whose home is `docs/development.md`, `CONTEXT.md` or a Clone Hero page go on the Out of scope list for the user.

- `gui-testing-hydra-uitest` (build and test-engine traps) and `report-capture-scripts`, `branch-window-check-recipe` → `docs/agents/ui-testing.md`.
- `hydra-mcp-server-2026-10` → `docs/agents/ui-testing.md` "Live mode and the MCP server".
- `ch-mcp-server-2026-10` → `docs/agents/clone-hero-mcp.md`; any game fact in it goes to its Clone Hero page (Out of scope list, since those pages are not this plan's).
- T5 writes the content into those two pages and T4c archives the five notes above (and `gui-testing-hydra-uitest`, `report-capture-scripts`, `branch-window-check-recipe`) in the same wave. If T5 fails its review the notes are still in `archive/`, so nothing is lost.
- `python-locations`, `gh-cli-location`, `ghidra-location`, `vs2026-incomplete-instance`, `opus-model-choice` → a new `docs/agents/this-machine.md` (where the tools are on this machine, read once).
- `inspect-hydra-db-without-sqlite-cli`, `gui-text-probe-for-stored-numbers`, `naming-copies-query-slowdown-2026-10` → `docs/agents/this-machine.md` too, under "Looking at the database".
- `doc-review-gate-2026-10`, `precheck-in-cpp-2026-10`, `cleanup-sweep-hook-2026-10`, `single-owner-rules-as-data-2026-10`, `hook-overhaul-2026-10`, `push-gate-trusts-gated-main`, `fleet-control-hooks-2026-09`, `shell-gate-change-2026-10`, `fleet-orchestration-lessons`, `review-loop-audit-2026-10-10`, `declare-doc-sources-before-review`, `worktree-folder-locks`, `agents-write-only-their-own-worktree`, `agent-worktrees-under-worktrees-folder`, `browser-tests-dont-flood-tabs` → their rulings go to `rulings.md`; the hook facts belong in `C:\Users\Patrick\.claude\gates\README.md`, which is the HX-P2 plan's file, so T4c's report lists each hook fact for the HX-P2 side and the note is archived (exit 3). `hook-design-audit-2026-10-25` is not in this list: it is a TOP RULE note and stays (exit 1). The notes that describe retired hooks as live (`fleet-control-hooks-2026-09`, `shell-gate-change-2026-10`, `agents-write-only-their-own-worktree`: no gate blocks cross-worktree writes today, checked by grep of `gates\*.py` on 2026-10-31) are archived, not moved.
- `windows-installer`, `mimalloc-link-order`, `srb-format-support`, `dmleaderboards-comparison`, `video-decode-ceilings`, `fc-video-workflow`, `gameplay-video-score-analysis`, `stitched-chart-auto-fills`, `engine-tie-merge-prune`, `squeeze-transfer-scale`, `min-path-delta-proof`, `sp-squeeze-mechanics`, `fill-spawn-comparison`, `ch-score-breakdown-lines`, `ch-note-shuffle-algorithm`, `ch-disco-noflip-rule`, `ch-plain-drums-drops-dynamics`, `early-fill-orange-85-d250`, `drum-lanes-d112-2026-10`, `song-end-one-rule-d255`, `font-roles-d258-2026-10`, `album-search-d263-2026-10`, `one-record-per-chart-profile-2026-10`, `report-covers-whole-library-2026-10`, `midi-drum-track-names-d252`, `paths-tab-starts-closed-2026-10`, `hydra-deluxe-rename`, `backend-hidden-rows-2-9-1` → Out of scope (their homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages). Until the user approves that move they stay in MEMORY.md, but their rulings are copied to `rulings.md` now.

**Exit 3, archived (finished or stale work).** The note moves to `memory\archive\` and its index line goes. `derivation-audit-2026-10-03`, `maintenance-wave-mr-2026-10`, `perf-exploration-2026-10-06`, `storage-audit-2026-10-06`, `memory-audit-2026-10-07`, `report-windows-2026-10`, `settings-summary-button-2026-10`, `preview-keeps-its-place-2026-10`, `analyze-button-follows-chip`, `session-cards-backfill-2026-10`, `preview-solo-hug-sp-gauge-always-2026-10`, `concept-audit-2026-10`, `gui-profile-recipe-2026-10`, `batch-bad-alloc-backlog-2026-10`, `analyze-job-pointer-reuse-bug` (its fact: `AppState::analyze_generation` is gone from `src/`; only a comment in `src/ui/generation.h` line 5 still names it, which T4c reports for a code fix), `ci-runner-stalls`, `ci-yaml-unparsed-here`, `path-builder-and-libraries-2026-10` (its live pointer is the 2026-10-31 redesign-2 handoff; one index line stays until that build ships), and the hook-history notes named in exit 2.

**Exit 4, deleted outright.** Nothing. A note with no ruling and no fact is archived, not deleted, because the archive is cheap and a deletion cannot be checked.

### Rulings that must survive the move (G3)

A grep for the user's quoted words (the pattern in T4a's AC1) found 75 quotes in 56 note files when the plan was written, and 79 matches in 58 files (48 outside `archive/`) when it was re-run at review: the folder keeps changing, so the count is read off the copy T4a works from and never off this plan. The pattern finds only quotes introduced by "user said", "user ruled" and similar words, so it is a floor. T4a also reads the grep hits with their context, and runs the second pattern in its AC1 for rulings stated without quotes. T4a copies every one of them into `rulings.md`. The ones the plan knows by name, because the findings or this planner read them, are (each quote was found word for word in the file named on 2026-10-31):

- "i don't care if an agent is read only, no status updates is unacceptable." (every-brief-carries-status-rule, 2026-09-05)
- "if anything changes a display in the app or a record, run it by me." (`archive/confirm-before-coding`; the full sentence, in card 2026-10-04-1f13cd5d, continues "if it only changes code you can use your recommendation without input.")
- "the point of doing an audit is to improve, not make the problem worse" and "make a new plan to get rid of every hook we have now and start over from scratch." and "i want to be able to give explicit approval for a full library run" and "rase cap to 3100" (hook-design-audit-2026-10-25)
- "you dont need a full library run to check the rendering. this is exactly the kind of time wasting i am trying to avoid." (render-checks-use-the-corpus)
- "wow you really fucked this up. did you never once do a UI test?" (ui-changes-need-real-app-check)
- "when did they diverge and why? did i ever ask for it?", "what are we already doing and why are you asking me to re derive it", "why are we even using two different systems? why are you still trying to convince me of more convoluted bullshit?", "yes everything should use one rule" (never-keep-a-split-quietly)
- "implement all nine recommendations as hooks" and "Option 3, narrow only" (fleet-control-hooks-2026-09)
- "use recommended answers" (shell-gate-change-2026-10)
- "repeatedly taking screenshots is very inefficient" (gui-testing-hydra-uitest)
- "Proceed, my call" (ch-mcp-feasibility-2026-10)
- "85 ms both sides" (fill-spawn-comparison), "go with the game's rule" (ch-note-shuffle-algorithm)
- "permanent docs should never reference or be held in scratch folders" (permanent-docs-never-cite-scratch)
- the quotes in `parallel-by-default` (4), `counts-on-screen-must-agree` (3), `no-silent-functionality-changes` (2), `one-reviewer-rule-d107` (3 at review), `report-covers-whole-library-2026-10` (2), `review-loop-audit-2026-10-10` (2), `path-builder-and-libraries-2026-10` (2), and one each in the other files the grep names.

T4a's Verify runs the grep on the copy and checks every quoted string is a substring of `rulings.md`. The count printed must equal the count found; any miss fails the task.

What this plan drops, on purpose: nothing ruled. What it keeps but moves: every exit 2 and exit 3 note's rulings. The notes that move to `archive/` stay whole in that folder, so even a ruling that neither grep finds is not erased; it only stops loading every session. The notes that describe hook behaviour the 2026-10-25 rebuild removed are archived, because the behaviour is gone by the user's ruling, and their words are kept in `rulings.md`.

## 6. Tasks

Each task fits one Opus executor in 100 tool calls and 20 minutes (G10). No two own a file in common. All seven start together: T1, T2, T3, T4a, T4b, T4c and T5. T4 was one task in the first draft; reading 124 notes, writing the rulings page, writing a machine-facts page and rewriting the index does not fit 100 calls, so it is three.

### T1. The project CLAUDE.md becomes a table of contents

**Goal.** CLAUDE.md is a map: under 1,000 words (section 4 shows the arithmetic); the two review sections and the agent rules are in short form with pointers, and the other sections are unchanged. Every rule it held still exists, in CLAUDE.md or in `docs/agents/`. Rule 9 carries the handoff sentence. Rule 2 names the fifth agent type.

**Files.** `CLAUDE.md`; `docs/agents/agent-rules.md` (new); `docs/agents/derive-once-review.md` (section "For the orchestrator" only); `docs/agents/doc-review.md` (new section "What is gated" at the top).

**Acceptance criteria.**
- AC1. CLAUDE.md is under 1,000 words (whitespace split). Log line: the word count printed in Verify.
- AC2. Every sentence of the old CLAUDE.md (`git show main:CLAUDE.md`) appears verbatim in the new CLAUDE.md or in one `docs/agents/*.md`, or is listed in the commit body as "rewritten: <old> -> <new>" with the new sentence present. The check script in Verify prints `missing: 0`.
- AC3. Rules 1 to 10 keep their numbers and their first sentence. Verify greps the ten first sentences in the new CLAUDE.md.
- AC4. Rule 9 holds the handoff sentence of section 3 word for word (G7). Verify greps it.
- AC5. Rule 2 reads "Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet. Bulk read-and-fill writers, such as the card writers, run on Haiku." The last sentence is new and follows the user's 2026-10-14 choice (`haiku-for-simple-readers`). Pending Q2: if the user picks the other option, the sentence is not added. Verify greps it.
- AC6. The "Derive-once review" and "Doc review" sections of CLAUDE.md are each at most three sentences plus a pointer, and every sentence they lost is in `derive-once-review.md` or `doc-review.md` (covered by AC2).
- AC7. The docs test passes: `hydra_tests.exe "-tc=docs match code*"` prints its pass line (G5, G6).
- AC8. "Past sessions" keeps the card folder path and the index rule unchanged (its sentences pass AC2 unchanged).

**Steps.** Read CLAUDE.md, `derive-once-review.md` "For the orchestrator", `doc-review.md` and `brief-preamble.md`. Write `docs/agents/agent-rules.md`: the ten rules with every detail sentence CLAUDE.md holds today, plus the "Hooks check three of these" paragraph. Add to `derive-once-review.md` "For the orchestrator" every sentence of CLAUDE.md "Derive-once review" it does not already hold. Add "What is gated" to `doc-review.md` with every sentence of CLAUDE.md "Doc review". Rewrite CLAUDE.md: the two review sections and the agent rules become their short form and a pointer; every other section stays word for word. Rules 1 to 10 become one line each, first sentence kept. Put the section 3 sentence into rule 9 and the Haiku sentence into rule 2. Write the AC2 check script in the scratch folder (split old file into sentences at `. `, strip markdown, search each in the new files; print `missing: N` and the misses). Run the docs test. Commit.

**Verify.** `(Get-Content CLAUDE.md -Raw).Split(...).Count` under 1,000; the AC2 script prints `missing: 0`; `Select-String` for the rule-9 sentence and the rule-2 sentence each print one hit in CLAUDE.md; `build-cpp\Release\hydra_tests.exe "-tc=docs match code*"` passes.

### T2. The preamble, the handoff guide and the `/handoff` command

**Goal.** The preamble carries the handoff sentence and says how a handoff is checked. `docs/agents/handoff.md` holds the shape of section 3 and the checker interface. `/handoff` writes one.

**Files.** `docs/agents/brief-preamble.md`; `docs/agents/handoff.md` (new); `.claude/commands/handoff.md` (new); `.gitignore` (one added line); `docs/agents/integrate.md` (line 42 only).

**Acceptance criteria.**
- AC1. The preamble's "The code is the source of truth" section holds the section 3 sentence word for word (G7). Verify greps it.
- AC2. The preamble has a new paragraph "Continuing from a handoff": run the handoff checker, read every claim against the code, and report a `MISSING` or `CHANGED SINCE` line before any work starts. Verify greps "check_handoff.py" in the preamble.
- AC3. `docs/agents/handoff.md` holds the `## Claims` table with the seven kinds and the "How to check" column, the backticks rule for the Value column, the no-abbreviated-hash rule, the no-scratch-path rule, and the checker command and its three output sections exactly as section 3 states them. Verify greps `## Claims`, `check_handoff.py`, `CHANGED SINCE`, `40-character`, and "scratch".
- AC4. `.claude/commands/handoff.md` exists, reads `docs/agents/handoff.md` by relative path, and names the checker. `.gitignore` no longer ignores it. Verify: file exists; grep for `docs/agents/handoff.md` and `check_handoff.py`; `git ls-files .claude/commands/handoff.md` prints the path and `git check-ignore .claude/commands/handoff.md` prints nothing.
- AC5. `integrate.md` line 42 no longer says "once it exists"; the docs test command stays. Verify greps "once it exists" in integrate.md and gets 0 hits.
- AC6. The preamble's "Review findings" section and the budget paragraph still say a reviewer lists unfixed findings in Notes at the warning (rule 5 unchanged). Verify greps "lists the rest in its review's Notes".
- AC7. Docs test passes (G5, G6).
- AC8. The preamble grows by at most 150 words. Verify prints the before and after counts.

**Steps.** Read the preamble, section 3 of this plan, `integrate.md`. Edit the preamble: add the sentence under "The code is the source of truth" and the new paragraph after it. Write `handoff.md` from section 3. Write the command file: frontmatter `description: Write a checkable handoff for the next session`, then the steps from section 3 with `$ARGUMENTS` as the slug. Fix integrate.md line 42. Run the docs test. Commit.

**Verify.** The greps in AC1 to AC6; the docs test's pass line; the word counts.

### T3. The global side: posture text and agent type files

**Goal.** The posture text carries the handoff sentence and the Haiku type. `reviewer.md` agrees with rule 5. `scout.md` lets a scout write its scratch folder (pending Q8). No file in `~\.claude\gates\` is touched.

**Files.** `C:\Users\Patrick\.claude\model-postures.md`; `C:\Users\Patrick\.claude\agents\reviewer.md`; `C:\Users\Patrick\.claude\agents\scout.md`; `C:\Users\Patrick\.claude\agents\executor.md`; `C:\Users\Patrick\.claude\agents\planner.md`; `C:\Users\Patrick\.claude\CLAUDE.md` (read only; no change planned, the file is 279 words of style rules the user wrote).

**Acceptance criteria.**
- AC1. `model-postures.md` holds the section 3 sentence word for word (G7), after "Never trust another agent's summary." Verify greps it.
- AC2. The posture text stays under 650 words (549 today). Verify prints the count.
- AC3. `reviewer.md` keeps "Never pass a change by listing an unmet promise as a "drift" or a "review note"" and adds, right after it, the rule 5 exception in these words: "The one exception is the budget warning: at 100 calls or 20 minutes you stop fixing, sign off what you fixed, and list every unfixed finding under a Notes heading in your report, each with the plan line it breaks (CLAUDE.md rule 5)." The sentence names the budget warning only; the other Notes path, a finding that needs the user (a new number, a displayed text, a choice), is in `derive-once-review.md` and `doc-review.md` and stays there, and Q1 shows the user all three texts. Pending the user's answer to Q1; if the user picks the other side, the executor writes that side instead and the reviewer checks the brief. Verify greps "Notes heading".
- AC4 (pending Q8). `scout.md` says "Change no files except your status line and files in your own scratch folder." If the user picks the other option, `scout.md` is not changed on this line. Verify greps it.
- AC5. `executor.md` and `planner.md` gain one sentence: "If your brief points you at a handoff, check its Claims table against the code before you build on it (`docs/agents/handoff.md`)." Verify greps it in both.
- AC6. `git diff --stat` on the branch touches only the files listed. Verify prints it.

**Steps.** Cut a worktree of `C:\Users\Patrick\.claude` under `C:\Users\Patrick\.claude\worktrees\harness-agents`. Make the edits. Commit with the preamble's form. The main session dispatches the reviewer with the hooks repo's root folder written in the brief, as CLAUDE.md "Derive-once review" says for the other repo.

**Verify.** The greps in AC1, AC3, AC4, AC5; the word count; `git diff --stat main...HEAD`.

### T4. The memory folder, split in three

The main session copies the whole memory folder once, before dispatching, to the session's scratch folder, and names that copy in the briefs of T4a, T4b and T4c. All three read the copy for counts and the live folder for content. The three tasks own different files, so they run together.

### T4a. `docs/agents/rulings.md`

**Goal.** Every user ruling held in a memory note is in `docs/agents/rulings.md`, in the words the note holds.

**Files.** `docs/agents/rulings.md` (new). It reads the memory copy and edits nothing in it.

**Acceptance criteria.**
- AC1. Two greps run on the copy. The first is for quoted user words: the pattern `(user|User)[^\n]{0,40}(said|ruled|answered|verbatim|exact words|:)\s*["“]` over every `*.md`, including `archive\` (75 matches when the plan was written, 79 at review). Every quoted string found is a substring of `rulings.md`. Log line: `quotes: N found, 0 missing`, where N is what the grep finds on the copy. The second grep is for rulings stated without quotes: lines matching `(?i)\b(user|you)\b.{0,50}\b(ruled|decided|chose|wants?|said|asked|ruling|approved|picked)\b|user's (rule|order|choice|words|decision)` (about 35 lines in the archive-bound notes alone when tried at review). Each such line, or the sentence in it that holds the ruling, is in `rulings.md` as the note words it, marked "as the note records it". Log line: `unquoted: M found, 0 missing`. A reading of the grep hits with their context is part of the task, because both patterns are floors, not ceilings.
- AC2. `rulings.md` has one line per ruling: date, the words, the session id or card id, and the rule it backs (a CLAUDE.md rule number, a `docs/agents/` page, or "no page yet"). The date and ids come from the note's own `Sessions:` line and text; where a note gives none, the cell says "not stated" and nothing is guessed.
- AC3. The quotes the plan names in section 5 are each in `rulings.md` word for word.
- AC4. The docs test passes (G5, G6); `rulings.md` holds quoted words, not backticked code names.

**Steps.** Run the two greps on the copy and read each hit with three lines of context. Write `rulings.md` in a few large writes, newest first. Run the AC1 checks as a script in the scratch folder. Run the docs test. Commit. If the run reaches 100 calls, commit what is written and say which notes are done; a fresh agent takes the rest (the preamble's rule).

**Verify.** The log lines of AC1; the greps for the AC3 quotes; the docs test's pass line.

### T4b. `docs/agents/this-machine.md`

**Goal.** Where things are on this machine is written once, in the repo, and checked against the machine.

**Files.** `docs/agents/this-machine.md` (new). It reads the notes and edits none.

**Acceptance criteria.**
- AC1. It holds the facts of `python-locations`, `gh-cli-location`, `ghidra-location`, `vs2026-incomplete-instance` and `opus-model-choice`, and under "Looking at the database" the facts of `inspect-hydra-db-without-sqlite-cli`, `gui-text-probe-for-stored-numbers` and `naming-copies-query-slowdown-2026-10`.
- AC2. Every path or version it states was checked on the machine (`Test-Path`, `--version`) or on the file the note cites, and the page says "checked <date>" once. A fact that fails the check is left out and named in the report (agent rule 9).
- AC3. The docs test passes (G5, G6).

**Steps.** Read the eight notes. Check each fact. Write the page. Run the docs test. Commit. Report the facts it dropped.

**Verify.** The `Test-Path` and `--version` lines in the report; the docs test's pass line.

### T4c. The memory index, the archive and the stale pointers

**Goal.** MEMORY.md holds only the notes section 5 keeps. Exit 3 notes are in `archive/`. Index lines match their files. Stale file names in kept notes are fixed.

**Files.** Everything under `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory\` including `MEMORY.md` and `archive\`. No file in the repo.

**Acceptance criteria.**
- AC1. MEMORY.md holds exactly the exit 1 notes (the 17 TOP RULE notes and the RULE notes listed), the exit 2 notes that wait for Q7 and the one `path-builder-and-libraries-2026-10` line. Counted from today's index lines, that is 77 lines and 1,388 words for the first two groups, so MEMORY.md ends near 80 lines and 1,450 words (before, 125 lines and about 2,140 words). The earlier target of 60 lines and 1,000 words holds only after the Q7 follow-up moves the waiting notes, when the count is about 49 lines and 955 words. Verify prints lines and words before and after; both must be lower than before, and the report says how far it is from the 80-line estimate.
- AC2. Every file named in MEMORY.md exists in the memory folder, and every file in the folder (not `archive/`) is named in MEMORY.md. Verify: a script prints `index: 0 missing, 0 unindexed`.
- AC3. No kept note names `tools\derive_once_precheck.ps1`, `gates\doc_review_sources.ps1`, `doc_review_sources.ps1`, `main_move_check.ps1`, `hooks\state\status`, "every 10 tool calls" or "10 calls" as a file that exists or a cadence that applies. A grep of the live folder on 2026-10-31 found hits in `one-reviewer-rule-d107` (a TOP RULE note, line 15), `refused-merge-abort` (line 15), `every-brief-carries-status-rule` (line 15) and `windows-installer` (lines 33 and 37), and in notes this plan archives. Each hit in a kept note is fixed on its own sentence: the `refused-merge-abort` sentence says the review key is the branch tip, `git rev-parse <branch>`, and that the ref gate's refusal names it; the `every-brief-carries-status-rule` sentence says status every 5 minutes with the helper under `gates\` (the preamble owns the exact form). Verify greps each string in the kept notes and gets 0 hits, and diffs every kept note against the copy: the only changed lines are the ones that held a hit, and every quoted word is untouched.
- AC4. For each of the five index lines section 5 names as disagreeing with its note, the kept line says what the note and the code show, decided as section 5 says. The report lists the other index lines it found disagreeing with their notes.
- AC5. No note is deleted: `(Get-ChildItem memory -Recurse -File).Count` after equals the count before. Verify prints both.
- AC6. The report lists: each note whose exit changed after reading; each hook fact for the HX-P2 side; each fact bound for a page outside this plan (for the Out of scope list); the `generation.h` comment; any note added to the folder after 2026-10-31.

**Steps.** Read the five notes whose index lines disagree and the notes that carry a grep hit. Move the exit 3 notes, and the exit 2 notes whose content T5 and T4b put in the repo, into `archive/` with one `Move-Item` per note (no recursive delete). Fix the stale sentences. Rewrite MEMORY.md from the kept notes' existing index lines with the five lines corrected. Run the AC2 and AC3 scripts. Report. Nothing here is committed to git, because the folder is in no repo; the reviewer reads the folder (G9).

**Verify.** The counts of AC1 and AC5; the log lines of AC2 and AC3; the diff of AC3.

### T5. Broken pointers and the GUI-check split in `docs/agents/`

**Goal.** `clone-hero-mcp.md` names only files that exist. `ui-testing.md` says when the headless test is enough and when the real app on the user's library is also owed, in the terms the user ruled. The two pages absorb the facts of the five memory notes that section 5, exit 2 sends to them; T4c then archives those notes.

**Files.** `docs/agents/clone-hero-mcp.md`; `docs/agents/ui-testing.md`.

**Acceptance criteria.**
- AC1. `clone-hero-mcp.md` line 31 no longer names `tools/ch_mcp/start_method.json` as a file that exists (it names it twice, and `Test-Path` is false on 2026-10-31). Line 54 names it a third time as a file the main session will write after live step LS-M, and says "until LS-M has run, neither that file nor that tool exists"; that is true today (no `ch_start_song` in `tools/ch_mcp/*.py`), so line 54 stays unless LS-M has run by the time T5 builds. The executor finds which file the kit and the song start really read (`tools/ch_mcp/kit_methods.json` and `tools/ch_mcp/start_candidates.json` exist; `tools/ch_mcp/tests/test_frida_attach.py` names neither `start_method` nor `start_candidates`, checked by grep) and names that file, or says the list is not kept in a data file yet. Verify greps `start_method.json` in `docs/agents/`: no hit on line 31, and any hit left is in a sentence that says the file does not exist yet; every other path the line names passes `Test-Path`.
- AC2. `ui-testing.md` opens with a three-part rule that keeps all three rulings: `hydra_uitest` replaces screenshots for checking behaviour and text; a change to what the user sees is also seen in the real app with the user's `hydra_ui.ini` and a copy of their database before merge ("did you never once do a UI test?", 2026-10-10); and a rendering check runs on `testdata/input`, never on a whole-library analysis ("you dont need a full library run to check the rendering", 2026-10-14). The three are not in conflict once each names its case; Q3 asks the user to confirm that reading, and this criterion waits for the answer (pending Q3). The opening rule also replaces line 5 of the page ("Use this instead of launching `Hydra.exe` and taking screenshots. Reach for a screenshot only when something looks visually wrong."), which is a fourth text on the same question. Verify greps the three quoted phrases.
- AC3. The traps from `gui-testing-hydra-uitest`, `report-capture-scripts` and `branch-window-check-recipe` are in `ui-testing.md` under a "Traps" heading, each as one sentence that points at the owner file rather than restating a rule. Verify greps "## Traps".
- AC4. Each fact in memory notes `hydra-mcp-server-2026-10` (into "Live mode and the MCP server" of `ui-testing.md`) and `ch-mcp-server-2026-10` (into `clone-hero-mcp.md`) that the page does not already hold is added, after the executor checks it against the code or the page (agent rule 9). A fact that fails the check is left out and named in the report. A Clone Hero game fact is not added here; it goes in the report for its Clone Hero page. The note's status claims (which live steps passed) are not copied, because the two texts of that note disagree (section 5) and the pages state what the code and handoffs show.
- AC5. Docs test passes (G5, G6).

**Steps.** Grep `tools/ch_mcp` for the start list's real owner. Fix the line. Read the five memory notes and the top of `ui-testing.md`. Write the opening rule, the Traps section and the added MCP facts. Run the docs test. Commit.

**Verify.** The greps in AC1 to AC3; `Test-Path` on each path in the fixed line; the docs test's pass line.

## 7. What the plan leaves to HX-P2

The handoff checker (`tools/check_handoff.py`, its task T6; section 3 copies its interface), the posture delivery (once per session or only on typed prompts), the refusal log, the `ref_judge.py` trailer wording ("or orchestrator": the preamble never uses that word, and this plan does not add it; the HX-P2 plan either drops the phrase or defines it), the pointer check for memory and agent docs, and `gates\README.md`. T4's report hands the hook facts over.

## 8. Questions for the user

Each question quotes both sides, gives an example from each end, names the options and a recommendation, and the strongest case against it.

**Q1. The reviewer's unmet promises.** `C:\Users\Patrick\.claude\agents\reviewer.md` says: "Never pass a change by listing an unmet promise as a "drift" or a "review note"." CLAUDE.md rule 5 says: "A reviewer that runs out of time or tool calls signs off what it fixed and lists the rest in the Notes section of its review; the main session passes those Notes to the user, and the merge does not wait for them." A third text, `docs/agents/derive-once-review.md` "A finding you should not fix yourself", sends a finding that needs the user (a new number, a displayed text, a choice between two behaviours) to Notes as a question, whatever the budget; the same reviewer file says "If you can't fix it inside the task's files, sign off BLOCKED". Example from the first side: a reviewer with 40 calls left finds a plan line not built and writes it up as a note instead of fixing it; that is the dodge the sentence forbids. Example from the second: a reviewer at 100 calls has three findings left and no budget; rule 5 says sign off, list them, merge. Options: (a) keep all three, with the budget warning as the one exception to the first (T3 AC3 writes this; the third text already stands beside it); (b) drop rule 5's Notes clause so every unmet promise blocks; (c) drop the reviewer.md sentence. Recommendation: (a). Case against it: a reviewer can spend its budget slowly and reach the warning with findings it chose not to fix, and (a) lets that through; only the main session reading the Notes catches it.

**Q2. The Haiku agent type.** CLAUDE.md rule 2 says: "Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet." The memory note `fleet-control-hooks-2026-09` records, for 2026-10-09, in its own words and not as a quote from the user: "Same day: no work goes to Haiku (posture + executor_dispatch_brief_gate/fanout_track messages)"; that was the harness setting of the day, and `~\.claude\model-postures.md` today already lists `card-writer (Haiku)`, so the posture text and rule 2 disagree now. The memory note `haiku-for-simple-readers` records 2026-10-14: its text is "the user switched the rest to Haiku", and `agents\card-writer.md` has `model: haiku`. The user's own words for either choice are in no note, so this is a choice between two records, not between two quotes. Example from the first side: a reviewer on Haiku would miss code findings. Example from the second: a card writer reads a condensed transcript and fills a template, with `check.py` behind it. Options: (a) rule 2 gains the Haiku sentence of T1 AC5 (the later ruling wins); (b) card writers move to Sonnet and the Haiku type goes. Recommendation: (a). Case against it: a fifth model line is one more place for a brief to pick wrong, and the 2026-10-09 "no Haiku" ruling was never withdrawn in words, only overtaken.

**Q3. The GUI check.** CLAUDE.md "GUI testing" says: "To verify anything in the GUI, run `hydra_uitest` ... instead of launching the app and taking screenshots." Memory `ui-changes-need-real-app-check` has this description: "a UI layout change is not done until it is seen in the real app on the user's real library and settings", and in its body asks for "screenshots of the real dev app with the user's hydra_ui.ini and a copy of their DB, shown to the user". Memory `render-checks-use-the-corpus` says in its description "A real-app rendering check never needs the user's whole library", and in its body "Use the user's real db copy only for the Library view itself, never to trigger analysis"; the MEMORY.md index line for it is shorter and says "never the user library", which is stronger than the note. `docs/agents/ui-testing.md` line 5 says: "Use this instead of launching `Hydra.exe` and taking screenshots. Reach for a screenshot only when something looks visually wrong." Example of the first: a button's label changes; `hydra_uitest` reads the text and that is enough. Example of the second: a column-width rule; headless tests passed and the real Library came up squeezed. Example of the third: a report's cell alignment; the corpus shows it in 30 seconds and the library took 75 seconds of re-analysis. This plan reads the three as three cases of one rule (T5 AC2): behaviour and text by uitest; what the user sees also in the real app with the user's ini and db copy; rendering on the corpus, never a whole-library analysis. Options: (a) confirm that reading; (b) every UI change needs the real app on the library; (c) the real-app check is dropped. Under (a) T4c also rewrites the `render-checks-use-the-corpus` index line to match its note's body (the real db copy is allowed for the Library view). Recommendation: (a). Case against it: the line between "behaviour" and "what the user sees" is drawn by the agent, and the 2026-10-10 regression was an agent drawing it wrong.

**Q4. Where `/handoff` lives.** Project scope (`hydra-test\.claude\commands\handoff.md`, committed, reviewed) or user scope (`~\.claude\commands\handoff.md`, works in both repos). The docs say the two scopes must not share a name. Example: a hooks-repo session that wants `/handoff` gets nothing under project scope. Options: (a) project scope in Hydra only (T2 builds this); (b) user scope only; (c) both under different names. Recommendation: (a). Case against it: the hooks repo's sessions write handoffs too (eight handoffs with hook, gate or reviewer-dispatch in their names exist under `docs/handoffs/` in the main checkout, counted on 2026-10-31), and they would have to write one by hand.

**Q5. What "TOP RULE" means.** Seventeen index lines carry it; nothing defines it. Options: (a) it means "a standing rule the user gave in their words; it stays in MEMORY.md and in `rulings.md`" (T4c keeps those 17 as they are, except for a stale file name); (b) drop the label and let `rulings.md` carry the weight. Recommendation: (a). Case against it: a label with a meaning invites the next session to add an eighteenth without asking, and the index grows again.

**Q6. The bullet-wall Stop check.** The findings claim the old `stop_bullet_check` hook caught bullet walls twice, and the 2026-10-25 rebuild dropped it with the other style gates (it is in the "Gone with nothing in its place" list of `docs/superpowers/plans/2026-10-25-hooks-from-scratch.md`, section 2, checked). Bringing it back is a hook, so it is the HX-P2 plan's question; this plan keeps the plain-English rule as an instruction only. It is listed here so the user sees that both plans leave it as a question and neither builds it unasked.

**Q7. Moving project facts into pages this plan does not own.** Section 5 exit 2 names about 28 notes whose homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages. Options: (a) approve a follow-up task that moves them, with the Clone Hero facts going on their pages as CLAUDE.md "Clone Hero reference" demands; (b) leave them in MEMORY.md. Recommendation: (a). Case against it: it is an eighth task touching pages other work is editing now (the Clone Hero reference), so it waits for a quiet moment.

**Q8. Scouts and their scratch folder.** `agents\scout.md` says: "Change no files except your status line." Its description says "A read-only look at the code, the data or the web". The preamble, which the scout is told to follow, says: "Touch only the files your brief owns, plus your scratch folder." Example of the first side: a scout that must not leave a trace in any repo, so the user can rely on "read-only". Example of the second: a scout that reads a 5,000-line trace and wants to keep a working copy of the lines it filtered in the session's scratch folder. Options: (a) the scout may also write files in its own scratch folder, never in a repo (T3 AC4 writes this); (b) keep the scout strictly read-only and let it return everything in its report. Recommendation: (a). Case against it: "read-only" is a promise the user can check at a glance, and a scratch folder is shared by the whole session, so a scout could overwrite another agent's file there.

## 9. Findings claims checked, and the ones that did not hold

Checked true on 2026-10-31: `tools/ch_mcp/start_method.json` is missing (`Test-Path` false; `start_candidates.json`, `kit_methods.json`, `menu_fields.json`, `menu_steps.json` exist). `integrate.md` line 42 says "once it exists". `ref_judge.py` line 229 says "or orchestrator". `reviewer.md` line 11 and `scout.md` line 9 read as the findings say. `card-writer.md` has `model: haiku` and rule 2 names four types. `fleet-control-hooks-2026-09` says "Same day: no work goes to Haiku" (its own words, not the user's) and "every 10 tool calls"; `every-brief-carries-status-rule` says "every 10 tool calls or 5 minutes" and `hooks\state\status`. `refused-merge-abort` names `main_move_check.ps1`, which exists in `hooks\` and not in `gates\`. The word counts for CLAUDE.md (2,225), the preamble (2,373) and the posture text (549) matched at review; MEMORY.md was 2,139 at review against the 2,116 the planner counted (the folder changes between sessions), and the global `CLAUDE.md` is 279 words.

Claims that did not hold, or hold only in part:

1. "123 memory files": the folder holds 124 files outside `archive/` and 64 inside it.
2. "`AppState::analyze_generation` is gone": the member is gone from `src/`, but a comment at `src/ui/generation.h` line 5 still names it, so a grep for the name is not empty. The note is still stale; the code comment is a separate fix (T4 AC8 reports it).
3. "a hook blocks cross-worktree writes" (memory `agents-write-only-their-own-worktree`): no file in `gates\*.py` reads a write path against a worktree; only `delete_rule.py` and `review_dispatch.py` mention worktrees. The note describes a retired hook, so T4 archives it.
4. The findings say `docs/agents/clone-hero-mcp.md` "cites" the missing file once; line 31 names it twice (the call list and the field-set list) and line 54 names it a third time, as a file not yet written (T5 AC1).

## 10. Out of scope (for the user to approve)

- Moving the exit 2 project-fact notes whose homes are `docs/development.md`, `CONTEXT.md`, ADRs or Clone Hero pages (Q7).
- The `src/ui/generation.h` line 5 comment that names a member that no longer exists (a code change; `comments-explain-current-code`).
- The hooks repo's `gates\README.md` facts T4c hands over, and every hook (HX-P2 plan).
- Trimming the global `C:\Users\Patrick\.claude\CLAUDE.md`: it is 279 words of the user's own style rules and this plan leaves it alone.
- The session-cards move into `docs/` and the finished-handoff sweep (`docs/handoffs/2026-10-31-session-cards-into-docs-and-handoff-cleanup-handoff.md`): T1 keeps "Past sessions" pointing at today's folder.

## 11. Promise map

| Promise | Plan line | Task | Check |
|---|---|---|---|
| Every CLAUDE.md rule survives the move | G1, section 4 | T1 | AC2 script prints `missing: 0` |
| Rule numbers stay 1 to 10 | G2 | T1 | AC3 grep of ten first sentences |
| CLAUDE.md under 1,000 words, other sections unchanged | section 4, T1 AC1 | T1 | word count printed; AC2 script |
| Rule 9 carries the handoff sentence word for word | section 3, G7 | T1 | AC4 grep |
| Rule 2 names the Haiku type (pending Q2) | T1 AC5 | T1 | grep of the sentence |
| Review sections become short form with pointers | section 4, T1 AC6 | T1 | AC2 and a sentence count |
| "Past sessions" keeps the card folder | section 4, T1 AC8 | T1 | AC2 unchanged sentences |
| Preamble carries the sentence word for word | G7, T2 AC1 | T2 | grep |
| Preamble says how to continue from a handoff | section 3, T2 AC2 | T2 | grep `check_handoff.py` |
| Handoff guide holds the Claims table and checker interface | section 3, T2 AC3 | T2 | greps of `## Claims`, `check_handoff.py`, `CHANGED SINCE` |
| `/handoff` command exists, is committable and names the guide and checker | section 3, T2 AC4 | T2 | file exists, greps, `git ls-files` prints it, `git check-ignore` prints nothing |
| Checker interface matches the hooks plan's T6 | section 3 | T2 (words), HX-P2 T6 (script) | the reviewer of whichever plan lands second compares the two |
| Handoffs name no scratch path | section 3 | T2 | instruction in `handoff.md`; AC3 grep "scratch" |
| Hashes written in full | section 3 | T2 | AC3 grep "40-character" in `handoff.md` |
| integrate.md line 42 fixed | section 1 item 4, T2 AC5 | T2 | grep 0 hits |
| Reviewer Notes rule unchanged in the preamble | T2 AC6 | T2 | grep |
| Preamble grows at most 150 words | T2 AC8 | T2 | counts printed |
| Posture carries the sentence word for word | G7, T3 AC1 | T3 | grep |
| Posture under 650 words | T3 AC2 | T3 | count printed |
| reviewer.md agrees with rule 5 (pending Q1) | T3 AC3 | T3 | grep "Notes heading" |
| scout.md allows the scratch folder (pending Q8) | T3 AC4 | T3 | grep |
| executor and planner check handoff claims | T3 AC5 | T3 | grep in both |
| T3 touches no gates file | G4, T3 AC6 | T3 | `git diff --stat` |
| Every quoted ruling survives in rulings.md | G3, section 5 | T4a | AC1 `quotes: N found, 0 missing`, N read off the copy |
| Every unquoted ruling survives as the note words it | G3 | T4a | AC1 `unquoted: M found, 0 missing` |
| rulings.md rows carry date, words, id, rule | T4a AC2 | T4a | reviewer reads; AC1 covers the words |
| The named quotes of section 5 are in rulings.md | T4a AC3 | T4a | greps |
| MEMORY.md holds the kept notes and is shorter than before (about 80 lines, 1,450 words; 49 lines and 955 words after Q7) | section 5, T4c AC1 | T4c | counts printed before and after |
| Index and folder agree | section 5, T4c AC2 | T4c | `index: 0 missing, 0 unindexed` |
| No kept note names a gone file or the old status cadence | section 5, T4c AC3 | T4c | greps 0 hits in kept notes; diff shows only those lines changed |
| refused-merge-abort pointer fixed | section 5, T4c AC3 | T4c | grep `main_move_check` 0 hits in kept notes |
| The five disagreeing index lines are corrected from code and docs | section 5, T4c AC4 | T4c | reviewer reads the five lines against `clone-hero-mcp.md` and the notes |
| No note deleted, only archived | section 5 exit 4, T4c AC5 | T4c | file counts equal |
| TOP RULE notes keep their words (a stale file name may change) | section 5 | T4c | diff of the 17 files against the copy shows only stale-name lines |
| Hook facts handed to HX-P2 | section 5 exit 2, T4c AC6 | T4c | report lists them |
| this-machine.md holds the machine facts, each checked on the machine | section 5, T4b AC1, AC2 | T4b | `Test-Path` and `--version` lines; docs test passes |
| MCP facts and uitest traps reach the two pages before their notes are archived | section 5 exit 2, T5 AC3, AC4 | T5 | greps; the report lists facts left out |
| Every question the plan resolves is asked in section 8 | G11, section 8 | none | Q1 to Q8 each quote both texts; the main session puts each answer in its brief |
| start_method.json no longer named as an existing file | section 1 item 4, T5 AC1 | T5 | no hit on line 31; hits left say the file is not yet written; `Test-Path` on named paths |
| GUI rule keeps all three rulings (pending Q3) | T5 AC2 | T5 | greps of the three quoted phrases |
| uitest traps moved into ui-testing.md | section 5 exit 2, T5 AC3 | T5 | grep `## Traps` |
| No permanent doc cites scratch | G5 | T1, T2, T4a, T4b, T5 | docs test pass line |
| Every docs/agents page passes the docs test | G6 | T1, T2, T4a, T4b, T5 | docs test pass line |
| Each task fits 100 calls and 20 minutes | G10 | all | T4 split in three; T4a commits partial work and hands on at 100 calls |
| No new hook from this plan | G4 | all | `git diff --stat` on each branch shows no `gates\` or `settings.json` file |
| Tasks share no file | G8 | all | the Files lists above are disjoint (reviewer checks) |
| Hydra commits get one fresh review; T4a's reviewer reads the memory folder after T4c reports | G9 | all | the main session's dispatch; the T4a reviewer's brief names the folder and the copy |
| Style rules stay instructions, no Stop hook from this plan | section 1 item 5, Q6 | none | Out of scope; question Q6 |
| Project facts bound for other pages move only with approval | section 5 exit 2, Q7 | none | Out of scope |
