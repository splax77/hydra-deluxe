# TOOL-AUDIT-T2 report (calibration files)

Commits read. Hydra repo: main checkout af9dfac4e246df6b77c4b906f5c7054f47beaa82; the worktree tooling-audit-records is at ebbb713bb52efe828beddd860156a55445518275. brief-preamble.md and agent-rules.md are byte-identical in both (diff -q silent); I read the worktree copies. Hooks repo (~\.claude): 38f9649fe5bdcab5035ebbde9a65bd1fd92f5ef3.
Files 1, 2, 4, 5, 7 are in ~\.claude (hooks repo). Files 3, 6 are in Hydra. I never saw a hook warning; all seven files were read in full. I also grepped stall.py, common.py, bash_windows_rule.py, posture.py, settings.fragment.json, gates README, rulings and the hooks plan line 108 for context.

Values traced: 46 distinct values, in all (the hook never warned me).

Context ruling (docs/superpowers/plans/2026-10-25-hooks-from-scratch.md line 108): "Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` and nowhere else in code; the docs quote them". So docs quoting them is stated there as accepted; I still list each quoting as K1 because the brief says K1 counts. Ruling lines found: review-and-hooks.md line 24 (20/30 minutes caps, 2026-10-09), lines 33-34 (status every 5 min, deny after 10, 2026-09-04), line 52 (note, not the user's words).

---------------------------------------------------------------

## C1. Budget numbers: when does an agent get warned and stopped (calls and minutes)?
Owner: progress.py line 14-15: `CALL_WARN, CALL_STOP = 100, 150` and `MINUTES_WARN, MINUTES_STOP = 20, 30`, comment "The one home of these numbers (the user's rulings on rule 7). Docs quote them; code reads them here."
Other homes (all state the values):
- brief-preamble.md line 73: "At 100 tool calls, or at 20 minutes of real time counted from your first tool call, it warns you once ... At 150 tool calls or 30 minutes it stops you." Also line 71 "Commit before your 100th tool call".
- agent-rules.md rule 7: "At 100 tool calls an agent starts wrapping up ... 150 tool calls is a hard stop ... At 20 minutes ... 30 minutes is a hard stop."
- CLAUDE.md (Hydra) rule 7 (loaded context, not one of the seven) only says "every 5 minutes".
- model-postures.md line 5: "if a task is too big for one executor inside 100 tool calls and 20 minutes, it is two tasks".
- writer-brief.md line 101: "At 20 minutes or 100 tool calls, stop starting new cards: finish the one in hand and report."
- agent-rules.md line 27 (hooks paragraph): "the cap is the progress hook's time limit, MINUTES_STOP in progress.py" (pointer, names the owner).
- brief-preamble.md line 73 also: "This hook is the only limit on an agent: no agent type file sets a turn cap behind it." (checked: the five agent files have only a model: line plus tools; no cap found in card-writer.md.)
What each home tells its reader:
| Home | At call 100 / minute 20 | At call 150 / minute 30 | Edge exactly at the limit | Main session | Agent that skips the preamble (card-writer) |
|---|---|---|---|---|---|
| progress.py | one notice, "commit what passes and report" | refuse all but git, report tools, status line | warns at calls >= 100 (the 100th call itself carries the notice); count is incremented before the test, so the 150th call is itself refused: 149 real calls pass | not counted (check() runs only for calls inside a subagent) | counted like any subagent |
| brief-preamble | start wrapping up, commit | stopped | "At 150 tool calls ... stops you": reads as 150 allowed; code refuses the 150th | not addressed (says "You have a call budget"; preamble is for agents) | card-writer.md says skip the preamble; the hook still binds it |
| agent-rules rule 7 | "starts wrapping up: commits what it has and hands the rest to a fresh agent" | hard stop | same as preamble | says "Agents" only | not addressed |
| writer-brief | "stop starting new cards: finish the one in hand and report" (no commit; card writers commit nothing) | not stated | "At 20 minutes or 100 tool calls": says stop starting at 100; hook warns at the 100th call | n/a | this is the card writer's only statement; omits the 150/30 hard stop |
| model-postures | sizes a task by 100 calls and 20 minutes (a use of the warning limits as a planning limit, so K3 flavour: the owner's number is a wrap-up warning, here it is a sizing limit) | none | none | main session is the reader | n/a |
Kinds: K1 (several copies, all agree today). K3 candidate: model-postures uses the warn number as "too big for one executor"; the owner calls it a warning to wrap up. K3 candidate: writer-brief says "stop starting new cards" at 100/20 whereas the hook's own warning says "commit what passes" and the preamble says "commit before your 100th call" (card writers commit nothing; the text differs by agent type, not stated anywhere as intended). K8 candidate? brief-preamble line 5 says "the call and time budget" is a hook; true.
SPLIT S1: preamble/rules "150 tool calls ... stops you" vs progress.py: the 150th call is refused (calls incremented first, `>= CALL_STOP`). Two homes, two different counts of how many calls pass (150 vs 149). Edge case; not shown to occur in a real run.
SPLIT S2: preamble line 73 "Whichever limit comes first applies" is stated in preamble and rules; the code does apply both (calls or minutes in the same test), so no split there; listed only so it is not re-found.

## C2. Status-line deadlines: when is an agent reminded and refused?
Owner: progress.py line 16: `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`.
Other homes:
- brief-preamble line 13: "every 5 minutes"; line 21: "When your last line is 5 minutes old, the hook reminds you. When it is 10 minutes old, the hook refuses your calls until you write one."
- agent-rules rule 7: "Agents write a status line every 5 minutes" (CLAUDE.md also).
- writer-brief line 100 (item 3): "One status line every 5 minutes ... Only time counts, not tool calls."
- model-postures: no number.
| Home | Reminder | Refusal | Exactly at the limit | Main session | Card writer |
|---|---|---|---|---|---|
| progress.py | >= 5 min since last status (or first call until one is written) | >= 10 | status_age >= limit, so exactly 5.0 and 10.0 minutes trigger | no check | checked |
| preamble | reminded at 5, refused at 10 | | same | n/a | skip preamble but hook binds |
| writer-brief | asks for a line every 5 minutes; silent on the reminder and the refusal at 10 | | a line due exactly at 5 vs "every 5": same | n/a | the only text it reads |
Kinds: K1 (5 and 10 restated). The preamble also gives a K5 piece: see C3.
Note: preamble "a command that starts with `git`, and your final report" still pass; progress.py: `command.startswith("git ")` after lstrip (so a bare `git` with no space, or a PowerShell `& git`, does not pass) and REPORT_TOOLS = SubagentHandback, StructuredOutput. Preamble says "A command that starts with `git`". Mismatch at the edge `git` with no argument (K2/K3 tiny; not shown to occur).

## C3. What shape is a valid status call? (the helper call)
Owner: progress.py lines 22-27 `STATUS_PREFIX` and `_STATUS_CALL` regex (prefix, id bare or quoted, one quoted line with no `"` `$` backtick, CR/LF or the three curly quotes, nothing after).
Other homes:
- brief-preamble lines 15 and 19: spells the rules out: "Type the line out in full. Write the time yourself (`10:46`), never `$(Get-Date ...)`, and put nothing after the closing quote. A `$`, a backtick or a double quote inside the line, or anything chained after it, breaks the shape." Then line 19 ends "The status-call check in `C:\Users\Patrick\.claude\gates\progress.py` owns the exact shape." (names the owner AND restates the shape: prose doing the hook's job, K5; also a K1 on the forbidden characters list: the preamble lists `$`, backtick, double quote; the regex also bars CR/LF and curly quotes, so the prose list is shorter than the owner's: K2 on the list.)
- preamble line 15: the call text `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"` is the STATUS_PREFIX copied (K1).
- progress.py `_helper_line` (line 57-58) builds the form shown to an agent: `... "HH:MM done ... | next: ..."`; preamble line 13 states the same line form `HH:MM done ... | next: ...` (K1). writer-brief line 100 states `"HH:MM done <n> of <m> cards | next: <session>"` (third copy of the form, with the prefix `& "C:/Users/Patrick/.claude/gates/status_append.ps1"` copied in full: K1 again).
- status_append.ps1 line 9 comment: "progress.py owns how a status write is recognised" (pointer, correct: owner claim checked, true). Line 5 comment: "The call shape: the brief preamble's 'Status lines' section." So the helper points at the preamble for the call shape, while the preamble points at progress.py for the exact shape, and the preamble spells part of it. Chain of pointers with the shape in two places. K5 plus a possible K8 (the helper names the preamble as the owner of the call shape; progress.py is the owner by its own comment and code).
- settings.fragment.json line 4 holds the allow rule `PowerShell(& "C:/.../status_append.ps1"*)`; progress.py line 21 comment calls it "the allow rule in settings.fragment.json names it" (K1: the prefix is written in code and in the allow rule; outside my seven files, noted only).
| Case | progress.py | preamble | writer-brief |
|---|---|---|---|
| id given bare | accepted | form shows bare id | bare `<your agent id>` |
| id given quoted | accepted | silent | silent |
| line containing `$`, backtick, `"` | not a status call; if it names the helper, refused and counted | "breaks the shape ... refuses the call and counts it" | silent |
| line containing a curly quote or a newline | not a status call, refused/counted | not listed (list incomplete) | silent |
| trailing text after the closing quote | fails fullmatch (`\s*` allowed) | "put nothing after the closing quote" | silent |
| the name is another agent's id | refused, not counted | "refused by the progress hook, and the refused call is not counted" | silent |
| the main session writes a status line | no check (check() is for subagents) | not addressed | n/a |
| call starts with `git` and names the helper | exempt, passes (exempt test is before the helper test) | "unless the command starts with `git`" | n/a |
SPLIT S3: the forbidden-character list: preamble (`$`, backtick, double quote) vs progress.py regex (also CR/LF and three curly quotes).
SPLIT S4: who owns the call shape: status_append.ps1 line 5 says the preamble's "Status lines" section; the preamble line 19 and progress.py say progress.py.
Kinds: K5 (preamble spells the shape), K1 (prefix and form copied three times), K2 (S3), K8 candidate (S4).

## C4. Where is a status line written, and which agent ids are valid?
Owner: common.py lines 10, 120-124: STATE_ROOT is the `state` folder beside common.py; `status_path` = STATE_ROOT\status\<id>.md; agent records under STATE_ROOT\agents\<session>\<id>.json.
Other homes:
- status_append.ps1 line 11: `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (hard-coded second copy of the path; comment line 8 says "Always the live state folder, even when this script runs from a copy: progress.py owns how a status write is recognised"). Lines 12-14 and 24 rebuild `status\<id>.md` and `agents` by hand (a second owner in code of the path shape: K6). Line 34-35 re-implements "is this an agent id": a record `<name>.json` under any session folder; common.py (check_id, status_path) is the owner of id validity.
- Line 12: a `HOOKS_TEST_STATE_ROOT` override exists only in the helper (and common.py takes `root` params): two ways to redirect the state root.
- brief-preamble line 13 and 17: the path `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` and `...\state\agents\` stated in prose (K1; both agree today).
- progress.py line 88: tells the agent its status file, built from common.status_path (owner used correctly, not a copy).
- progress.py line 1 docstring: "gates/state/agents/<session_id>/<agent_id>.json" (a pointer with the shape; agrees with common.py).
Edge table:
| Case | status_append.ps1 | common.py/progress.py | preamble |
|---|---|---|---|
| name is an .md path | accepted as is (`-match '\.md$'`), the "known id" check runs only if the file sits directly in the status folder | status_path always builds <id>.md | says give your agent id; "a task id or a made-up name never makes a status file" |
| id with a record in another session | accepted ("in any session") | status call id checked against the calling agent's own id | "an agent record" without saying any session |
| agent id with characters outside A-Z a-z 0-9 _ - | no check except the record lookup | check_id in common.py (not read in full) and the regex only accepts [A-Za-z0-9_-] | silent |
Kinds: K6 (state path and agent-record lookup written twice, once in PowerShell and once in Python), K1 (preamble repeats paths). Not read in full: common.check_id's pattern, so I did not compare it with the regex character class.

## C5. Which hook owns what the preamble says it owns (K8 owner claims)
Claims in brief-preamble.md and agent-rules.md of the form "X owns this". I checked each against the file named.
- Preamble line 33: "`bash_windows_rule.py` in the gates folder owns the rule and the number." Checked: bash_windows_rule.py line 16 `MAX_CHARS = 1000`. The preamble also states "1,000 characters" (K1, agrees). Number of refused shapes "five": not counted against the file (not checked).
- Preamble line 35: "`shell_output.py` owns the number and the lists" and "4,000 characters" and "30,000": the 4,000 number I did not check in shell_output.py (unchecked); K1 if present.
- Preamble line 43: "`delete_rule.py` in the gates folder is the one home of which commands and flags count": not opened (unchecked). Preamble lists the same commands and flags in prose (`rm -rf`, `Remove-Item -Recurse -Force`, `find ... -delete`, `git clean`, `git reset --hard`, `git reset --merge`, `git checkout -f`, `git worktree prune`, `git stash drop`) and the two worktree roots: K1 or K5 depending on delete_rule.py. Unchecked.
- Preamble line 23 and agent-rules line 27: "`stall.py`": `TOOL_TAIL` named, "five" stated. stall.py line 34 `TOOL_TAIL = 5` (checked, agrees, K1 on "five", and the comment in stall.py calls it a display size).
- agent-rules line 27: "The step is STALL_SECONDS in `stall.py`, and the cap is the progress hook's time limit, MINUTES_STOP in `progress.py`." plus "no call for 10 minutes ... repeats at 20 and 30 minutes ... the third says it is the last." stall.py: STALL_SECONDS = 600; STALL_NOTICES = LIMIT_SECONDS // STALL_SECONDS (= 3 today). The 10, 20, 30 and "third" are values derived in code; the prose states the derived results (K1 on a derived number; changes if MINUTES_STOP changes, so K2 risk). Also "An agent that has not handed back 30 minutes after its first call gets one more notice": not verified in stall.py (only lines 20-40 and 100-107 read).
- agent-rules line 27: "Hooks check four of these" then lists progress hook (rule 7), whole-library guard (rule 8), ref gate (rule 5), and a hook that refuses hydra_build_pass (rule 14). Also says the stall notice "helps with rule 4". The preamble line 5 lists enforced items as: "the status line, the call and time budget, the delete rule, the whole-library guard, the Bash shapes that fail on Windows, the trim of long shell output and the commit trailers". Counts differ: agent-rules says "four" rules; the preamble names seven hook behaviours, including the delete rule, Bash shapes, trim and trailers (those enforce preamble rules, not numbered rules). Not necessarily a split, but a K7/K1 pair: two lists of "what hooks check" in two files. Neither is named as the home (gates README.md is named as the place by the preamble: "its README.md says what each one does").
- model-postures.md line 9: "Hook changes go through a worktree of `~\.claude` and get one fresh reviewer like Hydra code (`~\.claude\gates\README.md`)" and "run the tests that touch the changed files, and the whole suite only when the change touches shared code (`gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, the test harness in `gates\tests\lib`)". agent-rules rule 1 states the same list verbatim: "`gates\common.py`, `gates\lib`, `gates\settings.fragment.json`, `gates\tests\lib`" and "the live settings.json is no longer tracked; ... `gates\sync_settings.py`". Both are K1 copies of one list; neither names the other, and no code owner was found (searched: the three files; not searched: gates/tests runner). K7 candidate: a rule stated in two places (rule 1 and model-postures), neither marked as home.
| Home | whole hook suite runs when | main session | agent |
|---|---|---|---|
| agent-rules rule 1 | change touches the four shared paths | yes | agents "never run the full test suite" |
| model-postures | same four paths | yes, "Hook changes ... get one fresh reviewer" | preamble: "Never run all the hook tests" |
| preamble line 47 | never | n/a | "Never run all the hook tests" |
Edge: a hook change that touches `gates\common.py` runs the whole suite (main session), while preamble forbids the same to agents: agent vs main session differ by design of role, stated in both.

## C6. Model assignment per agent type
Owner (in code): each agent file's `model:` line: card-writer haiku, executor opus, planner fable, reviewer sonnet, scout sonnet (checked in ~\.claude\agents).
Other homes:
- agent-rules rule 2: "Planning agents run on Fable. Code executors run on Opus. Code reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Bulk read-and-fill writers, such as the card writers, run on Haiku."
- CLAUDE.md rule 2: same.
- model-postures.md line 5: "Executors on Opus. Reviewers run on Sonnet. Read-only scouts run on Sonnet or Haiku. Card writers run on Haiku. Every dispatch names its agent type as `subagent_type`: planner (Fable), executor (Opus), reviewer (Sonnet), scout (Sonnet or Haiku) or card-writer (Haiku)."
- card-writer.md line 4: `model: haiku`.
- brief-preamble line 67 commit trailer: "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>" and line 69 "Use the model line your brief gives if it differs." (This brief's own system reminder gives Sonnet 5.5; the preamble value is a default copied; K4 for the commit trailer: an agent is asked to supply a model line the harness knows.)
Split S5: scout: agent rule 2 and postures say "Sonnet or Haiku"; scout.md says `model: sonnet` only. Two homes, one allows Haiku.
Kinds: K1 (rule 2 restated in three places), K2 partial (S5), K4 (trailer model line).
Edge: an agent type with no agent file (e.g. a Workflow script agent, or the main session itself) takes whatever the dispatch passes; no home says which.

## C7. Who commits, and the trailer shape
Owner: the ref gate (git side), named in the preamble line 65 as refusing "any new commit that lacks a `Task:`, `Agent:` or `Session:` trailer, except a merge commit". Code not read (outside my seven).
Homes:
- preamble line 65-67: the trailer list and the exact git command.
- card-writer.md line 9: "you commit nothing and run no tests, so skip the preamble."
- writer-brief line 33: "Write only those card files ... never call any other tool (no ... git tools)."
- brief T2 itself and this brief's system reminder give another Co-Authored-By line ("Claude Sonnet 5.5").
- model-postures: "Executors ... briefed with the preamble first, then the task id, the session id, the files it owns, the tests it may run and the status rule." The preamble line 3 says the brief "names your task id, the session id, your owned files, your worktree or checkout, your scratch folder and the tests you may run" (list of what a brief holds, restated in two files with different contents: postures lacks worktree and scratch folder; the status rule is in postures only). K3/K1: two lists of what a brief must contain, no owner named, no check. K7 candidate.
Edge: preamble line 71 "Commit before your 100th tool call" (K1 of 100, C1); a scout has nothing to commit (scout brief says change no file).

## C8. Where agents' time and call limits come from vs the stall hook's text (K3)
- progress.py line 99-100: the warn text says "You have run %d minutes of %d. Start wrapping up: commit what passes and report what is unfinished." Preamble line 73: "start wrapping up, commit what passes and report what is unfinished, so a fresh agent can take the rest. ... A reviewer hands nothing on: it signs off what it fixed and lists the rest in its review's Notes". agent-rules rule 7 and rule 5 say the same for reviewers. The hook's warning text itself tells a reviewer to "commit what passes and report what is unfinished" with no reviewer branch. The hook text and the prose disagree for the reviewer type (hook has no agent-type input). Split S6: reviewer at the warning: hook says "commit what passes and report what is unfinished"; preamble/rules say it "stops fixing, signs off what it fixed and lists the rest in its review's Notes". Different instructions to the same agent at the same moment. Kind K2/K3.

## C9. The user's ruling for the status deadline vs the stated rule (context check, not a candidate)
review-and-hooks.md lines 33-34: "demand status every 10 tool calls and reject the next call after 15" (2026-09-04) and "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes". The first (calls) is replaced; the second matches C2. line 50 note says "status line minutes-only". The code uses minutes only. Not a candidate.

## C10. Paths and names scattered across the files
- `C:\Users\Patrick\.claude\gates\status_append.ps1`: the preamble (line 15, forward slashes), writer-brief line 100, progress.py line 22, status_append's own self (no path). Three spellings of one path in prose and code, agree today (K1).
- `C:\Users\Patrick\.claude\projects\C--Users-...-hydra-test\session-cards\` DATA folder and its subfolders (batches, condensed, staging, local-cards): stated once in writer-brief lines 25-31, and the check script (not read) is the owner; writer-brief says "The main session's check moves each card" and "Do NOT run check.py" and names the script in prose. The path was not found in card-writer.md (searched). Home for the data layout is not named (K7? only one place found; not a candidate).
- The memory index path, the decision log path and the CH README path appear in writer-brief; card-writer.md line 9 also restates the CH README path (K1: the clone-hero README path in two files).
- `C:\Users\Patrick\.claude\CLAUDE.md` named as "the one home" of plain-English rules in the preamble line 123; writer-brief line 94 gives its own plain-English rule ("Write in plain English, short sentences, no jargon without a gloss"), a paraphrase of the owner (K1 paraphrase; K3 risk since the card writer is told it does not read the preamble).
- model-postures.md line 11 gives a second path style for worktrees ("worktrees folder") and the preamble line 43 names the two worktrees roots with full paths; postures does not.

## C11. The card format and checker rules (writer-brief) vs the owner
- Owner: the checker (`check.py`, named in writer-brief line 33 and not read) for the card shape; the card shape text in writer-brief lines 37-65, "at most 60 lines; aim for 20 to 40" (line 71), "2 to 6" tags (rule 3), "7-to-40-character hex" (rule 5), tags from `DATA\tags.txt`. The brief states each as "Rules a checker enforces". These are copies of check.py's numbers unless check.py reads them from the brief: not checked (check.py not in my seven; unverified). If check.py holds the limits, writer-brief is K1 of 60/20-40/2-6/7-40; if it does not, writer-brief is the home. Pointer: the brief names the checker as owner of the shape.
- writer-brief line 5: "The plan behind this is `docs/superpowers/plans/2026-10-14-session-cards-backfill.md` (removed; `git show 444631e6:...`)": a commit hash copied (not verified).
- writer-brief line 19-21: four content rulings "on 2026-10-31"; no pointer to a ruling file (K7? The user's words for these were not found in the two rulings files I grepped for "local" ... I did not search the card index). Searched: docs/agents/rulings/review-and-hooks.md and agents-and-workflow.md for the status terms only; not for card content. Unchecked.
- writer-brief line 41-42: "Memory index: ...MEMORY.md ... `memory:` line" and "write-back:" rule: restated in the Hydra CLAUDE.md "Past sessions" section ("A card marked `[write-back]`..."). K1 across repos.
- Roadblock catcher words "main session", "hook notice", "user" (rule at line 91): a closed list; the checker presumably owns it (unchecked).
- card-writer.md `tools: Read, Glob, Grep, Write, PowerShell` vs writer-brief line 33 "Use only Read, Glob, Grep and Write, plus the PowerShell status-line command" (K1 of the tool list; and card-writer.md says "Use PowerShell only for the status-line command"; the agent file grants PowerShell wholesale and the prose limits it: K5, prose doing the permission job; no hook was found for it, searched: card-writer.md and the gates README grep for card-writer returned nothing I read).
- card-writer.md line 6 `disallowedTools: Agent` and preamble "You have no helper agents": two statements, the file is the enforcer; preamble line 77 repeats in prose (K5, small).

## C12. Other single values noted
- agent-rules.md line 7: "about 115 charts" in testdata/input (rule 8). I counted: find of notes.chart or notes.mid under testdata/input gives 98 files; all files 214; top-level entries 6; 32 entries one level below. Drifted or a different count; the definition of "chart" is not given (K2 candidate, count method unclear).
- agent-rules rule 14: "At most three builds run at once" and CLAUDE.md copy; no hook owner read (the build pass tool is outside my seven; unchecked).
- agent-rules rule 1: "Three tests read the whole repo" lists three commands; the precheck tool prints them (rule says so: "The precheck prints them as three `always:` lines"); the list is copied here and in CLAUDE.md-adjacent text (K1; owner is hydra_precheck, not read).
- model-postures.md line 15: "Keep this conversation under 500K tokens; the context hook warns you above that." stall.py line 27 `CTX_THRESHOLD = 500000` (K1, agrees) and its text line 36 says "Auto-compact should have fired at 500K (50% of the window)" (K1 plus a derived 50%).
- posture.py line 16 `RESEND_MINUTES = 60` is not restated in the seven files except via the gates README (README says "The number lives at the top of posture.py": correct pointer). Not a candidate.
- agent-rules lines 33-43 cite dates and quoted words; not values.

## What I did not reach or check
shell_output.py (4,000 and 30,000), delete_rule.py (commands, flags, roots), check.py (card limits), the ref gate (trailers), common.check_id pattern, the rest of stall.py (the extra 30-minute notice), the "five shapes" count in bash_windows_rule.py, the card index for the four 2026-10-31 content rulings, and whether rule 14's build limit has a code owner.
