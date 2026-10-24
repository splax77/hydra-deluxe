Ask questions if something is unclear. How to explain things lives in the user's global `~/.claude/CLAUDE.md`.

## Agent skills

### Issue tracker

Issues live as GitHub issues in `splax77/hydra-deluxe`, managed with the `gh` CLI. See `docs/agents/issue-tracker.md`.

### Triage labels

The five canonical triage roles, each label string equal to its name. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.

### Past sessions

Every past session of this project has a short card: its goal, what went wrong and why, the user's rulings in their exact words, and how it ended. The cards live in `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\cards\`, one line per card in `index.md` beside them. Most old transcripts are removed, so the cards and the `condensed\` files next to them are the record.

Look there when the user mentions an earlier session or a past decision, asks to continue or redo something, or before you propose an approach that may have been tried and dropped. The index is too big to read whole: grep `index.md` and the cards by keyword, tag, D-number or session id, open the matching card, and read its `condensed\<id>.condensed.md` only when you need detail. Memory files end with a `Sessions:` line that names their cards. A card is a pointer, not proof: check what it says against the code before you act on it (agent rule 9).

A start-of-session hook asks the main session to write cards for finished sessions that have none. Do it alongside the user's request, as the hook's message says.

### GUI testing

To verify anything in the GUI, run `hydra_uitest` (headless, drives the real UI by widget label, returns text) instead of launching the app and taking screenshots. See `docs/agents/ui-testing.md`. To drive Hydra's GUI one step at a time and read it back as text, use the project's `hydra` MCP server: call `hydra_start` (headless on the test charts, or the real window on a snapshot copy of the user's library), then the other `hydra_*` tools; "Live mode and the MCP server" in `docs/agents/ui-testing.md` explains them.

### Public doc style

Anything a player reads (the four public pages and the GitHub release notes) follows `docs/agents/public-doc-style.md`: second person, no first person, no AI-writing tells, every label and number checked against the code.

### Derive-once review

Merges and commits on `main` that touch code need a review from a fresh agent first. A hook enforces it. Dispatch the reviewer with `docs/agents/derive-once-review.md`, the key and the range the hook's message gives. When the change also holds documents, the same reviewer reviews them with `docs/agents/doc-review.md` (rule 5). A code change always goes on its own branch, in its own worktree, before it is reviewed; it is never reviewed staged on main, even for a one-line fix. A hook, `reviewer_dispatch_gate.ps1`, refuses a reviewer dispatch, through `Agent` or `Workflow`, whose change sits on main, whose documents have no declared sources, or whose brief forbids the reviewer to commit; the hook owns what each of those means. Only the user can skip a review, by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time.

### Doc review

Only what outsiders read, and plans, get a review from a fresh agent, and each document gets it once. Hooks enforce it in repos whose `main` holds a `.doc-review-gate` marker, as this one does. Four steps are gated: a move of `main`, local or pushed, that changes `README.md`, `docs/README.md`, `docs/UserGuide.md` or `docs/differences-from-public-hydra.md`; an Artifact publish; a plan shown with ExitPlanMode; and text posted with `gh`. Once a fresh agent has signed off a version of a document, later edits to it pass with no new review. A changed line never triggers another review. The marker file is the one exception: every change to it is reviewed. Internal docs (`CLAUDE.md`, `CONTEXT.md`, `docs/agents/`, ADRs, handoffs, committed plans) and questions to the user are not gated. The design history is in `docs/superpowers/plans/2026-10-08-doc-review-gate.md`, which predates these changes; this section and the hook code win where they differ. Dispatch the reviewer with `docs/agents/doc-review.md`, the key and the review kind the hook's message gives. Only the user can skip a review, by typing `waive doc-review <key>` in chat.

## Agent rules

These hold for the main session and every agent it sends out. Hooks enforce most of them; the details live in `docs/agents/brief-preamble.md` (what every agent reads first) and the model-postures file (what the main session gets each prompt).

1. Agents never run the full test suite. They run only the tests for their own change. When the main session merges, it runs the tests that touch the changed files. It runs the whole suite only when the change touches shared code: a header many files include, the build files, or the test harness (for hooks: `hooks\lib`, `settings.json`, `hooks\tests\lib`). Untouched parts stay as they were, so re-running them proves nothing (the user's ruling, 2026-10-14). Three tests read the whole repo, so they also run at every merge to Hydra's `main`, docs-only ones included: the docs-vs-code check (`hydra_tests.exe "-tc=docs match code*"`), the single-owner scan (`hydra_tests.exe "-sf=*single_owner*"`) and the precheck self-test (`pwsh -NoProfile -File tools/test_derive_once_precheck.ps1`). A change in one place can break any of them, and twice only CI caught it (the user's ruling, 2026-10-24).
2. Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet.
3. No work starts without the user's approval. Every plan and every build waits for the user's yes before it begins. In this rule a build means any piece of work: writing code, editing hooks, or launching agents to do either. A change that only touches code needs no approval of its design: pick the design and recommend it, but the build still waits for the yes. A change to a display in the app or a stored record also takes its design to the user first. When the user asks for one specific edit, the request is the approval. Compiling and running tests inside approved work need no separate yes.
4. Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke.
5. One review per change, then done. One fresh reviewer agent reads the change's code and documents, fixes every finding itself, commits its fixes and signs off. Then the change merges. There is no separate fixer, no checker and no second round, and nothing goes back to the user (D107). A reviewer that runs out of time or tool calls signs off what it fixed and lists the rest in the Notes section of its review; the main session passes those Notes to the user, and the merge does not wait for them. No agent is woken up again after its turn ends.
6. Agents run in parallel, in workflows, while the main session merges. Waves of parallel agents are fine. Never queue independent tasks one after another.
7. Agents write a status line every 10 tool calls and every 5 minutes. At 100 tool calls an agent starts wrapping up: it commits what it has and hands the rest to a fresh agent. A reviewer hands nothing on: it signs off what it fixed and lists the rest in the Notes of its review (rule 5). 150 tool calls is a hard stop. Time works the same way. At 20 minutes of real time an agent starts wrapping up, and 30 minutes is a hard stop. Whichever limit comes first applies.
8. Nobody runs a full library test without the user's explicit permission, asked for in chat first. That covers the main session and every agent, and any run over the user's whole song library (for example `hydra_batch` with no arguments, or a whole-library compare or timing run). Ask only when nothing smaller can answer the question. Otherwise test on the checked-in chart corpus in `testdata/input` (about 115 charts) or on the few charts the change touches.
9. Never trust another agent's summary or the comments it left behind. The code and the source data are the source of truth. Check a claim against them before you repeat it or act on it.
10. After a push, report done and move on. Don't wait for CI or check it unless your change is to CI itself. CI runs on GitHub after the push, and nothing waits for it.

No hook checks rule 3, rule 9 or rule 10. For rule 2, a hook checks only that a Fable session's dispatches name a model (Plan dispatches excepted), not which one. Follow these yourself.
