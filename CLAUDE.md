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

Merges and commits on `main` that touch code need a review from a fresh agent first. The ref gate enforces it. Git runs that gate on every move of `main` made from Claude Code, and it refuses a move whose new tip no fresh reviewer signed off; its refusal names the key to review. A code change always goes on its own branch, in its own worktree, before it is reviewed; it is never reviewed staged on main, even for a one-line fix. Cut an agent's worktree under a `worktrees\` folder (`C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\` for Hydra, `C:\Users\Patrick\.claude\worktrees\` for the hooks repo), because the delete rule lets an agent delete inside its working folder only there.

Dispatch the reviewer through the `Agent` tool with `subagent_type: reviewer`. Give it `docs/agents/derive-once-review.md`, the key (the branch tip's full 40-character hash) and the range `main...<key>`. The brief names that one commit and nothing else key-like. A hook refuses a reviewer dispatch whose brief holds no key, a key that is already on `main`, a key git does not know, or more than one commit. A reviewer launched any other way, from a Workflow script or as another agent type, leaves no verdict, and `main` stays refused. A change in the other repo (a hook change in `C:\Users\Patrick\.claude`, dispatched from a Hydra worktree) is checked in that repo when the brief writes out that repo's root folder and the commit it names exists there; a commit the dispatching repo knows is always checked there. The reviewer reads the code and every document in the range, fixes what it finds, and commits its fixes on the same branch. It ends its report with one line, `SIGN-OFF: CLEAN` or `SIGN-OFF: BLOCKED`. A report with no such line is recorded CLEAN, and the main session is told the line was missing. When the reviewer stops, a hook finds the branch's new tip in git, records the verdict for it, and fingerprints every markdown file the range changes at that tip. If the verdict is lost or recorded with a problem, the main session hears it on its next tool call. `main` may then move to the signed tip, or to a merge of `main` and that tip that git makes with no conflicts and no extra edits. The documents in the range are reviewed by the same reviewer with `docs/agents/doc-review.md` (rule 5). How it all works is in `C:\Users\Patrick\.claude\gates\README.md`. Only the user can skip a review, by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time.

### Doc review

Only what outsiders read, and plans, get a review from a fresh agent, and each document gets it once. Hooks enforce it in repos whose `main` holds a `.doc-review-gate` marker, as this one does. Four steps are gated: a move of `main`, local or pushed, that changes `README.md`, `docs/README.md`, `docs/UserGuide.md` or `docs/differences-from-public-hydra.md`; an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub. A short text passes the last three with no review; `publish_gate.py` in the gates folder owns that limit. Once a fresh agent has signed off a version of a document, later edits to it pass with no new review. That holds for any document a reviewer signed off as a committed file on a branch, when it sits in the repo the gated call runs in, because the record names its repo path and the gate reads it from that repo's top folder. Text posted to GitHub is the exception: it passes only as the exact text a reviewer signed off. A changed line never triggers another review. The marker file is the one exception: every change to it is reviewed. Internal docs (`CLAUDE.md`, `CONTEXT.md`, `docs/agents/`, ADRs, handoffs, committed plans) and questions to the user are not gated.

Text goes to GitHub only through `C:\Users\Patrick\.claude\gates\publish.ps1`. It takes the kind of post (`pr`, `release`, `issue-comment` or `pr-comment`) and a body file, checks the text the way the gate does, and runs `gh` itself only when the check passes. Once the new settings are live, they refuse the posting `gh` commands typed by hand.

A document is reviewed as a committed file on a branch, the same way code is. That covers public pages, release notes, plans and Artifact pages alike. Commit the document on its own branch, then dispatch a reviewer as "Derive-once review" says, with `docs/agents/doc-review.md` added and the document's sources listed. The brief names the branch tip and the range, and no key for the document. The reviewer ends with the same one `SIGN-OFF:` line. When it stops, the hook fingerprints the document at the new tip, so nobody computes or types a key for it. A text that was never committed in a reviewed range has no record and is refused. A body file for a GitHub post that was edited after the review is refused too, because a post passes only as the exact text signed off; commit the edit and review it. How it all works is in `C:\Users\Patrick\.claude\gates\README.md`; that page and the gates code win over the older design notes in `docs/superpowers/plans/2026-10-08-doc-review-gate.md`. Only the user can skip a review, by typing `waive doc-review <key>` in chat.

## Agent rules

These hold for the main session and every agent it sends out. Hooks enforce a few of them (the last paragraph says which); the details live in `docs/agents/brief-preamble.md` (what every agent reads first) and the model-postures file (what the main session gets each prompt).

1. Agents never run the full test suite. They run only the tests for their own change. When the main session merges a Hydra change, it runs what `build-cpp\Release\hydra_precheck.exe --repo <path> --tests <range>` prints on its `tests:` line. That tool decides what counts as shared code and when the whole suite runs, so nobody types the test list by hand. For hooks, the main session runs the whole suite only when the change touches shared code: `gates\common.py`, `gates\lib`, `settings.json`, `gates\tests\lib`. Untouched parts stay as they were, so re-running them proves nothing (the user's ruling, 2026-10-14). Three tests read the whole repo, so they also run at every merge to Hydra's `main`, docs-only ones included: the docs-vs-code check (`hydra_tests.exe "-tc=docs match code*"`), the single-owner scan (`hydra_tests.exe "-sf=*single_owner*"`) and the precheck self-test (`hydra_tests.exe "-sf=*test_precheck*"`). A change in one place can break any of them, and twice only CI caught it (the user's ruling, 2026-10-24).
2. Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet.
3. No work starts without the user's approval. Every plan and every build waits for the user's yes before it begins. In this rule a build means any piece of work: writing code, editing hooks, or launching agents to do either. A change that only touches code needs no approval of its design: pick the design and recommend it, but the build still waits for the yes. A change to a display in the app or a stored record also takes its design to the user first. When the user asks for one specific edit, the request is the approval. Compiling and running tests inside approved work need no separate yes.
4. Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke.
5. One review per change, then done. One fresh reviewer agent reads the change's code and documents, fixes every finding itself, commits its fixes and signs off. Then the change merges. There is no separate fixer, no checker and no second round, and nothing goes back to the user (D107). A reviewer that runs out of time or tool calls signs off what it fixed and lists the rest in the Notes section of its review; the main session passes those Notes to the user, and the merge does not wait for them. No agent is woken up again after its turn ends.
6. Agents run in parallel, in workflows, while the main session merges. Waves of parallel agents are fine. Never queue independent tasks one after another.
7. Agents write a status line every 5 minutes; only time counts, not calls. At 100 tool calls an agent starts wrapping up: it commits what it has and hands the rest to a fresh agent. A reviewer hands nothing on: it signs off what it fixed and lists the rest in the Notes of its review (rule 5). 150 tool calls is a hard stop. Time works the same way. At 20 minutes of real time an agent starts wrapping up, and 30 minutes is a hard stop. Whichever limit comes first applies.
8. Nobody runs a full library test without the user's explicit permission, asked for in chat first. That covers the main session and every agent, and any run over the user's whole song library (for example `hydra_batch --whole-library`, or a whole-library compare or timing run). The user allows one run by sending exactly `allow library run`; each such message lets one command through. Ask only when nothing smaller can answer the question. Otherwise test on the checked-in chart corpus in `testdata/input` (about 115 charts) or on the few charts the change touches.
9. Never trust another agent's summary or the comments it left behind. The code and the source data are the source of truth. Check a claim against them before you repeat it or act on it.
10. After a push, report done and move on. Don't wait for CI or check it unless your change is to CI itself. CI runs on GitHub after the push, and nothing waits for it.

Hooks check three of these. The progress hook keeps rule 7's status lines and budgets. The whole-library guard keeps rule 8. The ref gate keeps rule 5's one review before `main`. The main session also gets a notice when an agent goes quiet for 10 minutes, which helps with rule 4. No hook checks the rest; follow them yourself. The hooks live in `C:\Users\Patrick\.claude\gates\`.
