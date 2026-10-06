Ask questions if something is unclear. How to explain things lives in the user's global `~/.claude/CLAUDE.md`.

## Agent skills

### Issue tracker

Issues live as GitHub issues in `splax77/hydra-deluxe`, managed with the `gh` CLI. See `docs/agents/issue-tracker.md`.

### Triage labels

The five canonical triage roles, each label string equal to its name. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context: `CONTEXT.md` and `docs/adr/` at the repo root. See `docs/agents/domain.md`.

### GUI testing

To verify anything in the GUI, run `hydra_uitest` (headless, drives the real UI by widget label, returns text) instead of launching the app and taking screenshots. See `docs/agents/ui-testing.md`.

### Derive-once review

Merges and commits on `main` that touch code need a review from a fresh agent first. A hook enforces it. Dispatch the reviewer with `docs/agents/derive-once-review.md`, the key and the range the hook's message gives. Only the user can skip a review, by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time.

## Agent rules

These hold for the main session and every agent it sends out. Hooks enforce most of them; the details live in `docs/agents/brief-preamble.md` (what every agent reads first) and the model-postures file (what the main session gets each prompt).

1. Agents never run the full test suite. They run only the tests for their own change. The main session runs the full suite once, when it merges.
2. Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet.
3. Anything that changes a display in the app or a stored record goes to the user first. A change that only touches code proceeds on your recommendation, and you report it after.
4. Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke.
5. A reviewer agent must clear the derive-once audit before a change merges (see above). One round of review at most: the reviewer reports, the author fixes once, the reviewer fixes anything left and signs off.
6. Agents run in parallel, in workflows, while the main session merges. Waves of parallel agents are fine. Never queue independent tasks one after another.
7. Agents write a status line every 10 tool calls and every 5 minutes. At 100 tool calls an agent starts wrapping up: it commits what it has and hands the rest to a fresh agent. 150 tool calls is a hard stop.
