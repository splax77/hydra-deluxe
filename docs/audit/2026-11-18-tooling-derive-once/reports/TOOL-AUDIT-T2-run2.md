# TOOL-AUDIT-T2-run2 report (agent ad8ddbb95326fa64c)

Commits read: Hydra repo `af9dfac4e246df6b77c4b906f5c7054f47beaa82` (main checkout at C:\Users\Patrick\Downloads\Hydra\hydra-test; the brief and preamble were read there). Tooling repo (~\.claude) `38f9649fe5bdcab5035ebbde9a65bd1fd92f5ef3`.
Files read in full: progress.py, status_append.ps1, brief-preamble.md, writer-brief.md, card-writer.md, agent-rules.md, model-postures.md.
Also read for tracing (outside the seven): stall.py (constants), bash_windows_rule.py:16, shell_output.py:15, delete_rule.py:14-18, review_rules.py:51, settings.fragment.json:4, check.py (grep only), agents/*.md model lines, rulings review-and-hooks.md and agents-and-workflow.md (grep of Confirmed lines), hooks-from-scratch plan line 108, a file count of testdata/input.
No grading and no exits below.

Distinct values traced: about 62 (the hook never warned me).

Plan line 108 says: "The numbers (100, 150, 20, 30, 5, 10) sit at the top of progress.py and nowhere else in code; the docs quote them". So docs that quote the numbers are known copies; each is still a K1 if it states the value.

## C1. How many tool calls and minutes an agent gets (100/150, 20/30)
Question: when is an agent warned and when is it stopped?
Owner (code): progress.py:14 `CALL_WARN, CALL_STOP = 100, 150`; :15 `MINUTES_WARN, MINUTES_STOP = 20, 30`, under the comment "The one home of these numbers (the user's rulings on rule 7). Docs quote them; code reads them here." Rulings: review-and-hooks.md "Agents have a soft cap of 20 minutes real time ... hard cap of 30 minutes"; agents-and-workflow.md 2026-10-04 "soft cap 100 calls (hand off), hard cap 150".
Other homes (all state the values):
- brief-preamble.md:73 "Commit before your 100th tool call" and "At 100 tool calls, or at 20 minutes ... At 150 tool calls or 30 minutes it stops you."
- agent-rules.md rule 7: "At 100 tool calls an agent starts wrapping up ... 150 tool calls is a hard stop ... At 20 minutes ... 30 minutes is a hard stop."
- agent-rules.md hooks paragraph: "the cap is the progress hook's time limit, MINUTES_STOP in progress.py" (pointer) plus "30 minutes after its first call".
- model-postures.md line 5: "inside 100 tool calls and 20 minutes".
- writer-brief.md "How to work" 4: "At 20 minutes or 100 tool calls, stop starting new cards".
- CLAUDE.md rule 7 (not in my seven) is a further copy.
What each home tells its reader:
| Reader and case | progress.py | preamble | rule 7 | postures | writer-brief |
| agent at exactly 100 calls | one warning notice, still allowed | start wrapping up | start wrapping up | tasks must fit inside 100 | stop starting new cards |
| agent at exactly 150 calls / 30 min | refused except git, status, report | "stops you" | hard stop | not stated | not stated |
| 20 minutes | one warning | wrap up | wrap up | fit inside 20 | stop new cards |
| main session | the hook runs only for subagent calls (check() reads data["agent_id"]); main session has no budget | preamble is for agents | "hold for the main session and every agent" | n/a | n/a |
| card-writer (preamble skipped, card-writer.md:9) | hook still applies | not read | n/a | n/a | writer-brief restates the 100/20 itself |
| reviewer | same hook | "stops fixing, signs off" | hands nothing on | n/a | n/a |
Edges: preamble says "Commit before your 100th tool call"; the hook warns at call 100 (record["calls"] >= CALL_WARN), so call 100 itself is the warning call. Rule 7 says the agent "commits what it has and hands the rest to a fresh agent" while preamble says reviewers hand nothing on; both texts say so, they do not collide.
Kinds: K1 (several). Preamble, rule 7, postures and writer-brief all restate values the hook owns. Writer-brief's line is a K3 candidate: it tells a Haiku writer to stop starting new work at the warn point (a hook notice that only warns), where the hook itself does not stop anything until 150/30.

## C2. How often and how late a status line is due (5 / 10 minutes)
Owner: progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`.
Other homes: preamble:13 "every 5 minutes", :21 "When your last line is 5 minutes old, the hook reminds you. When it is 10 minutes old, the hook refuses your calls"; agent-rules.md rule 7 "every 5 minutes" (no 10); writer-brief "One status line every 5 minutes"; stall.py STALL_SECONDS=600 is a different thing (no call at all) but the same 10 minutes; agent-rules hooks paragraph says "no call for 10 minutes ... repeats at 20 and 30 minutes".
Table: agent at exactly 5 min old: hook adds a reminder note; preamble "reminds you"; rule 7 "write every 5 minutes". Exactly 10: hook refuses (status_age >= 10); preamble "refuses your calls until you write one". Calls that still pass at 10: git commands, report tools (hook `exempt`); preamble says "a command that starts with `git`, and your final report" - matches. Status calls themselves are never counted or refused. Main session: not covered by the hook, rule 7 says it holds for it too (no check).
Kinds: K1. A K7 candidate: agent-rules.md says the main session is bound by rule 7 and nothing checks it (the hook only reads data["agent_id"]).

## C3. The exact shape of a status call, and who must supply what
Owner: progress.py:22 STATUS_PREFIX and :26 `_STATUS_CALL`. It accepts the id bare or quoted, one quoted line that bars `"`, `$`, backtick, CR/LF and the curly quotes U+201C/U+201D/U+201E.
Other homes: status_append.ps1:5 "The call shape: the brief preamble's Status lines section" (pointer), but also :8-10 "progress.py owns how a status write is recognised"; preamble:15 and :19 spell the shape out ("exact shape", `$`, backtick or double quote barred, nothing after the closing quote, "powershell -File ... refused ... unless the command starts with git"); writer-brief step 3 gives its own form `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "HH:MM done <n> of <m> cards | next: <session>"`; hook text `_helper_line` gives `"HH:MM done ... | next: ..."` with the id in quotes; settings.fragment.json:4 allow rule `PowerShell(& "C:/Users/Patrick/.claude/gates/status_append.ps1"*)` is a fourth spelling of the path (progress.py:21 says it is "spelled as the allow rule ... names it").
Table: id written bare (preamble, writer-brief) vs quoted (hook's own suggestion): both accepted. Curly quotes: preamble lists only "a `$`, a backtick or a double quote", the code also bars three curly quotes, so a line with a curly quote is refused although the preamble never says so (partial copy). Card-writer: told to skip the preamble, gets the shape from writer-brief only (no "alone in its own call", no `$` warning); writer-brief says "run with the PowerShell tool (not Bash)", preamble says "from PowerShell, alone in its own call".
K4 (two values the agent is asked to type):
 a) the agent id. The hook knows it (it prints it on call 1 and compares the typed one to its own, refusing a mismatch at progress.py:111-115). The agent copies back a value the hook already holds.
 b) the time "HH:MM". Preamble:19 "Write the time yourself (`10:46`), never `$(Get-Date ...)`". status_append.ps1:21 can already stamp the time itself (`(Get-Date).ToString('HH:mm')`) but only when no message is given. The agent cannot know the clock except by guessing; a wrong time cannot be checked by any hook.
Kinds: K1 (shape in preamble, writer-brief, hook, allow rule), K4 (id, time), K6-ish: the helper's path is spelled in progress.py:22, settings.fragment.json:4, preamble:15, writer-brief step 3 and agent-rules (by name).

## C4. Where status lines live (folder, record, id check)
Owners: common.status_path (called by progress.py:88, :150; not read by me) and status_append.ps1:11 hard-coded `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'`, `status` and `agents` subfolders :13-14.
Other homes: preamble:13 states `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` and :17 `...\state\agents\`; progress.py:3 docstring states `gates/state/agents/<session_id>/<agent_id>.json`; status_append.ps1 comment :26-28 says "progress.py makes the record before this script runs"; the hook notice prints the path via common.status_path.
Split candidate: the helper recomputes the state folder (K6: PowerShell constant vs common.status_path in Python) and reads the agents-record layout (`<session>\<id>.json`) that common owns. The ps1 comment says "Always the live state folder ... progress.py owns how a status write is recognised" which names progress.py as owner of recognition while the folder itself is stated in two code places.
Kinds: K6 (folder in two code places), K1 (preamble path).

## C5. The Bash/shell hook numbers (1,000 chars; 4,000 and 30,000 trim)
Owners: bash_windows_rule.py:16 `MAX_CHARS = 1000`; shell_output.py:15 `CAP = 4000`. The 30,000 is Claude Code's, not a Hydra hook (the preamble says so: "Above 30,000 characters it arrives as Claude Code's own notice").
Other homes: preamble:33 "A command over 1,000 characters" and "`bash_windows_rule.py` owns the rule and the number"; :35 "A result over 4,000 characters", "`shell_output.py` owns the number"; ruling 2026-11-06 "length over 1000" (the user's number).
Each states the value and names the owner: a home, per the brief's definition. Preamble also lists "five shapes" (count; also a copy of the rule's list). Kinds: K1 (value with pointer, agree today). The 30,000 figure has no owner in the repo (outside system); K7 candidate only if someone calls it a rule.

## C6. Which model each agent type runs on
Owners: agents/executor.md:4 `model: opus`, planner.md `fable`, scout.md `sonnet`, reviewer.md `sonnet`, card-writer.md:4 `haiku`.
Other homes: agent-rules.md rule 2 ("Read-only scouts run on Sonnet or Haiku"), CLAUDE.md rule 2, model-postures.md:5 (planner Fable, executor Opus, reviewer Sonnet, scout "Sonnet or Haiku", card writer Haiku), writer-brief and card-writer.md:9 (Haiku by frontmatter).
Split: rule 2 and postures say a scout may run on Haiku; scout.md pins `sonnet` as the only model, so an agent file cannot give a Haiku scout unless the dispatch overrides it. K2 candidate (text offers a choice the agent file does not have) and K1 for the rest.
Edge: this very run is a scout on Sonnet 5.5, consistent with rule 2.

## C7. The Co-Authored-By trailer model name
Home: preamble:67 `--trailer "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"` and :69 "Use the model line your brief gives if it differs."
Another place that holds the value: the harness's own attribution reminder (in this session it says Claude Sonnet 5.5), not in my files.
Table: executor (Opus): preamble line right. Reviewer or scout (Sonnet): preamble line wrong unless the brief overrides; the main session is not a preamble reader. A hook (ref gate) requires only Task/Agent/Session trailers (preamble:65), so a wrong model name passes.
Kinds: K4 (agent types a model name the runtime already knows), K1/K3 (a hard-coded model name in the shared preamble that fits only one agent type).

## C8. Which scratch/worktree locations the delete rule trusts
Owner: delete_rule.py:17-18 WORKTREE_FOLDERS = hydra-test\.claude\worktrees and C:\Users\Patrick\.claude\worktrees (plus session scratch).
Other homes: preamble:43 spells both paths and the rule; model-postures.md:5 "each in its own worktree under a `worktrees` folder (the delete rule trusts only those)".
Split to look at: the two texts differ in precision. Postures says "a `worktrees` folder". The path of this run's own worktree is C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records (the session's cwd), which is neither of the two folders in delete_rule.py, so by the preamble ("A worktree cut anywhere else is not a root") no recursive delete passes there. I did not test this; I only compared paths.
Kinds: K1 (preamble), K2 candidate (postures wording vs code), K3 candidate (the folder the brief's worktree uses vs the folder the rule trusts).

## C9. The whole-library permission phrase
Owner: review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`.
Other homes: preamble:49 "exactly `allow library run` ... each such message lets one command through"; agent-rules rule 8 same phrase and "each such message lets one command through"; CLAUDE.md rule 8.
Agree today. Kind: K1. Edge: the guard also refuses `--whole-library` for the main session; rule 8 says "main session and every agent", preamble addresses agents only.

## C10. Corpus size
agent-rules.md rule 8: "checked-in chart corpus in `testdata/input` (about 115 charts)". I counted C:\Users\Patrick\Downloads\Hydra\hydra-test\testdata\input recursively: 98 song.ini, 67 notes.mid and 31 notes.chart (98 charts), one more loose file group not counted. Count against "about 115": 98 charts. Owner: the folder itself. Kind: K2 candidate (copied count that does not match what I counted; my count method is just file names, not a Hydra tool).

## C11. Card-writer rules and the checker's own numbers
Owner: check.py (not in my seven). Found: MAX_CARD_LINES = 60 (:30), CARD_FILE = date + 8 hex id (:38), the header keys (:118), tags must be in tags.txt (:136), write-back tag and page list (:196-197), the "caught by" catchers (:63).
writer-brief.md states them again: "at most 60 lines" (also "aim for 20 to 40"), "exactly those six key: value parts", "Use 2 to 6" tags, "Never write any other 7-to-40-character hex string", the three catchers (`main session`, `hook notice`, `user`), "first 8 chars of the session id" for the file name, and a section headed "Rules a checker enforces (a card that breaks one fails)".
Search I made: grepped check.py for tag-count and hex-length patterns (`tags`, `{7`, `0-9a-f`) and found a tag membership test and an 8-hex filename pattern only; no count of 2 to 6 tags and no 7-to-40-hex rule turned up in those greps. I have not read all of check.py, so this is not proof they are absent.
Kinds: K1 for 60, header parts, catchers; K5 candidate for the card shape spelled in prose where a checker enforces it; K8 candidate: the heading "Rules a checker enforces" covers rule 3 "Use 2 to 6" and rule 5's hex rule, which my greps did not find in the checker.

## C12. What the card writer is asked to type that a tool holds
writer-brief "The exact card shape", header line: `session: <full session id> | commits: <hashes> | agents: <subagents_launched>`, title "from the metadata", start date from the `start:` line ("first 10 characters ... UTC"), `D:` numbers (checked against the decision log by grep), `memory:` names (by Glob), tags.
The writer copies values the condensed metadata already states (session id, title, start date, commits list, subagents_launched). check.py then verifies some of them (session ids, decision entries, tags membership).
Kind: K4 (a Haiku agent types metadata fields a program holds). The quotes are not K4: only the writer can pick the user's decision words, and the checker validates them against user_text.json.

## C13. Paths in writer-brief
Homes with literal paths: DATA = `C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\session-cards\`; MEMORY.md path; decision log `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\audit\2026-10-03-fix-decisions.md`; the clone-hero README path; card-writer.md:9 repeats the writer-brief path and the clone-hero README path; the preamble path lives in the brief. Owner of this machine's paths: docs/agents/this-machine.md (named in CLAUDE.md; not read by me). Kind: K1.
Also the writer-brief pastes the plan's removed path with a commit hash `444631e6`; the commit's existence I did not check.

## C14. The list of hooks and what they enforce
Homes: preamble:5 "A few are enforced by hooks: the status line, the call and time budget, the delete rule, the whole-library guard, the Bash shapes that fail on Windows, the trim of long shell output and the commit trailers"; agent-rules.md hooks paragraph "Hooks check four of these" (progress hook = rule 7; whole-library guard = rule 8; ref gate = rule 5; hook refusing hydra_build_pass = rule 14) plus the stall hook; gates README.md (not read; grep showed lines 29, 31, 116 too long to quote) is named as the home in preamble:5 ("its README.md says what each one does").
The two lists answer different questions (rules vs preamble sections); a hook in one list and not the other: the delete rule, Bash shapes, trim and trailers are in the preamble only; the build pass hook is in agent-rules only. Whichever is the real list of hooks is the gates README.
Kinds: K7 candidate (several lists, one named home is the README, but both pages also enumerate), K1.

## C15. Stall notice schedule and evidence line
Owner: stall.py:22 STALL_SECONDS = 600, :23-24 LIMIT_SECONDS from progress.MINUTES_STOP, STALL_NOTICES = 3, :34 TOOL_TAIL = 5.
Homes: agent-rules.md "no call for 10 minutes ... repeats at 20 and 30 minutes ... the third says it is the last"; "30 minutes after its first call gets one more notice"; preamble:23 "five is TOOL_TAIL in stall.py" and "the stall hook (`stall.py`...)".
These agree with the code (600/1800/3, 5). Kind: K1 (values with pointers).

## C16. Test-running scope for hooks
Homes: agent-rules.md rule 1 "the whole suite only when the change touches shared code: gates\common.py, gates\lib, gates\settings.fragment.json, gates\tests\lib"; model-postures.md line 9 repeats the same four paths ("the whole suite only when the change touches shared code (...)"); preamble:47 "Never run all the hook tests" (agents).
Two homes state the same list, neither names a tool; for Hydra the same sentence names `hydra_precheck.exe` as the one that decides ("That tool decides what counts as shared code"), but for hooks no owner is named. Kind: K1 and K7 candidate (a list in two prose places with no tool behind it; rule 1 says "the user's ruling, 2026-10-14").
Edge: the main session runs it; agents are told never to run all hook tests; no hook checks either.

## C17. Preamble's own claims about enforcement
- preamble:19 "refuses ... counts it toward your budget" - matches progress.py:123-128 (names helper but not exact: refused and counted).
- preamble:21 "Two kinds of call still pass then" - matches `exempt` (git prefix, REPORT_TOOLS). REPORT_TOOLS also lists "StructuredOutput", which the preamble does not mention ("your final report"). K1 partial copy.
- preamble:73 "no agent type file sets a turn cap behind it" - agents/*.md frontmatter that I grepped shows only model lines in the grep output; I did not read the full agent files for a `maxTurns` key.
- preamble:17 "the small file the progress hook keeps for each agent" matches progress.py record.

## C18. Other things I traced that agree
Task/session id forms, the "10 minutes of silence" stall step, "QUOTE_CHARS = 300" (progress.py:31, only in a refusal message, no other home), `_HELPER_PATH` derived from STATUS_PREFIX (one owner inside the file), the id regex `[A-Za-z0-9_-]+` (progress.py only; status_append.ps1 checks agent ids by looking for a record file instead, a second way of deciding "is this an agent id" - K6 candidate: regex shape in the hook vs record lookup in the helper).

## What I did not reach
gates README.md, this-machine.md, check.py in full, delete_rule.py beyond its constants, common.py, whether the ref gate checks Co-Authored-By, full agents/*.md bodies. Time was within budget; nothing was cut by the hook.
