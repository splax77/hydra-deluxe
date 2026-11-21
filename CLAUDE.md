Ask questions if something is unclear. How to explain things lives in the user's global `~/.claude/CLAUDE.md`.

## Agent skills

Clone Hero MCP: to drive Clone Hero as text, use the project's `ch` MCP server; see `docs/agents/clone-hero-mcp.md`.

### Clone Hero reference

Read `docs/clone-hero/README.md` before any Clone Hero question, and before any plan, test or probe that touches the game. A session or agent that learns a Clone Hero fact updates its page in the same commit as the work that found it, never in a memory note.

### Issue tracker

Issues live as GitHub issues in `splax77/hydra-deluxe`, managed with the `gh` CLI. See `docs/agents/issue-tracker.md`.

### Triage labels

The five canonical triage roles, each label string equal to its name. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.

### Past sessions

Every past session of this project has a short card: its goal, what went wrong and why, the user's words quoted exactly (a section headed "The user's words" in new cards, "Rulings" in older ones; a card quote is not a filed ruling), and how it ended. The cards live in the repo at `docs/sessions/`, one file per session, with one line per card in `docs/sessions/index.md`. The cards are the record. A card keeps the user's typed words, the label they chose and the start of each question, not the full question text. The condensed transcripts the cards are written from are local working files, not a reference, and no doc cites them.

Look there when the user mentions an earlier session or a past decision, asks to continue or redo something, or before you propose an approach that may have been tried and dropped. The index is too big to read whole: grep the index and the cards by keyword, tag, D-number or session id, and open the matching card. Memory files end with a `Sessions:` line that names their cards. A card is a pointer, not proof: check what it says against the code before you act on it (agent rule 9).

A start-of-session hook asks the main session to write cards for finished sessions that have none. Do it alongside the user's request, as the hook's message says. A card marked `[write-back]` in the index names a Clone Hero fact its page lacks, and the next main session adds that fact to the page alongside the user's request, the same way it writes missing cards.

### Audits

Before you plan an audit, brief an audit agent, or close any audit finding, read `docs/agents/audit-method.md`. It says how an audit runs and why past audits lost findings. Every finding leaves by one of four exits. A split goes to the user unless a ruling quoted in the user's own words names both of its sides, and a fresh agent checks every closure.

### GUI testing

Which check a GUI change needs is the first section of `docs/agents/ui-testing.md`. For behaviour and text, the main session runs `hydra_uitest` (headless, drives the real UI by widget label, returns text), or drives the GUI one step at a time with the project's `hydra` MCP server; "Live mode and the MCP server" on that page explains it.

### Rulings and this machine

The user's rulings, in their own words, are in `docs/agents/rulings/`, indexed by its `README.md`. A ruling is filed only when the user clicks "File it" for that exact line in a question box, and git refuses any other; its `README.md` gives the steps. Only a line under `## Confirmed` may be cited as the user's ruling. Only the user changes what a `CONTEXT.md` term means; the full rule is in `docs/agents/agent-rules.md`, "Rulings and definitions". This machine's tools and paths are in `docs/agents/this-machine.md`.

### Public doc style

Anything a player reads (the four public pages and the GitHub release notes) follows `docs/agents/public-doc-style.md`: second person, no first person, no AI-writing tells, every label and number checked against the code.

### Derive-once review

Merges and commits on `main` that touch code need a review from a fresh agent first. The ref gate enforces it. A code change always goes on its own branch, in its own worktree, before it is reviewed; it is never reviewed staged on main, even for a one-line fix. The steps, and the user's waiver, are in `docs/agents/derive-once-review.md`, "For the orchestrator".

### Doc review

Only what outsiders read, and plans, get a review from a fresh agent, and each document gets it once. A document is reviewed as a committed file on a branch, the same way code is. Text goes to GitHub only through `C:\Users\Patrick\.claude\gates\publish.ps1`. The gated steps, how to send a document for review and the user's waiver are in `docs/agents/doc-review.md`, "What is gated".

Every new plan carries a Premise map: one row per fact the plan stands on, each naming where it was read or marked `unchecked`, and every live step lists its tool calls. `docs/agents/premise-map.md` says the shape. A plan that changes what Hydra decides or shows is built from an Owner map a scout writes first: each question in game words, beside the existing code that answers it. A row with no owner, or a result that contradicts the user's words, stops the work and goes to the user. `docs/agents/owner-map.md` says the shape and the stops.

A finished plan or handoff leaves the repo in the same merge that finishes its work. A line that still names a removed plan must say how to read it now, as `<path>` (removed; `git show <commit>:<path>`). A handoff is never cited, by any file: it is a summary for the next session, not proof of anything (`docs/agents/handoff.md`, "A handoff is never a source").

## Agent rules

These hold for the main session and every agent it sends out. Each is in short form; the full text, and which hooks check which rule, is in `docs/agents/agent-rules.md`.

1. Agents never run the full test suite, and they run no test that needs a build (rule 14). Any other test an agent runs is for its own change.
2. Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Bulk read-and-fill writers, such as the card writers, run on Haiku.
3. No work starts without the user's approval. Every plan and every build waits for the user's yes before it begins.
4. Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke.
5. One review per change, then done. One fresh reviewer agent reads the change's code and documents, fixes every finding itself, commits its fixes and signs off. One narrow exception: a reviewer's C++ fix that fails the main session's build or a test goes to a fresh executor, and a fresh reviewer reads only that fix.
6. Agents run in parallel, in workflows, while the main session merges. Never queue independent tasks one after another.
7. Agents write a status line every 5 minutes; only time counts, not calls.
8. Nobody runs a full library test without the user's explicit permission, asked for in chat first.
9. Never trust another agent's summary or the comments it left behind. The code and the source data are the source of truth. Check a claim against them before you repeat it or act on it. A handoff is another agent's summary. Before any new work starts from a handoff, every claim in it is checked against what actually exists: the code is the source of truth, and a handoff claim that does not match the code is wrong. `docs/agents/handoff.md` says what a handoff holds and how to check one.
10. After a push, report done and move on. Don't wait for CI or check it unless your change is to CI itself.
11. A question that changes what Hydra does lists what the user will see: every screen that reads the changed value, with its text before and after from a real chart.
12. A visible change the user's words did not ask for is a question before merge, never an opt-out line in a report. Silence is not a yes.
13. A commit or release never holds an edit nobody in the session made or was asked to make; found edits wait for the user.
14. At most three builds run at once. Agents never build. A main session builds, one at a time, through a pass from `hydra_build_pass`, and runs every test that needs a build. A failure before review goes to a fresh executor on the same branch. The main session runs a test red only when a brief asks; otherwise the reviewer reads the test and says what would make it fail. Agents may run `hydra_precheck.exe` from main, but not `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`.
15. Find the existing owner before you write one, in plans and briefs as well as code. Search core, app, ui and tools for every place that already answers the question, including answers hidden in text or signs. Call the one owner, merge several into one first, or name the search that found none. A rule copied from a tool, test or comment is never a source. A display decides nothing: each value it shows is an engine value, carried out if the engine does not expose it yet. A rule a plan makes up is never put to the user as a question.
