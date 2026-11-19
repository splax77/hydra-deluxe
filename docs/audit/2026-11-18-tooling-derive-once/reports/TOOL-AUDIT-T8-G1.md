# TOOL-AUDIT-T8-G1: 238 Q rows grouped into 20 decisions

Source rows: the category Q lines of the TSV files in `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\sorting\TOOL-AUDIT-T6-S*.tsv`. Every quote and file:line below is copied from a row's `sides` column. Ids are written without the `L-` prefix shown in the files (F01-06 is L-F01-06). Abbreviations as the rows use them: BP = docs/agents/brief-preamble.md, AR = agent-rules.md, DOR = derive-once-review.md, MP = model-postures.md, GR = the gates folder. No answer is recommended. Counts per theme are at the end.

## 1. Rulings cited by code or docs that no Confirmed line holds

Decision: Should each place that says "the user ruled / answered / approved" be kept only after the user files that line under ## Confirmed, or have the claim dropped from the code, doc or comment?

Side A, keep the claim and file the ruling. Example: `quiet.py:43` cites "the user's ruling of 2026-11-07 for the 3 s scan cap" (F05-03). Under this side the user confirms the number and the line is filed; agents then cite it.
Side B, drop the claim and treat the value as an unconfirmed pick. Example: `bot_run.py:64-91` comments say the user answered the caps, but "the cited plans say recommended or a guess with no ruling" (F05-04). Under this side the comment stops naming the user, and the value is a default an agent may not cite as the user's.
Other example: `gates/README.md:19` "The user ruled on 2026-10-31 that ... losing battle" while `review-and-hooks.md:40` sits under Unconfirmed (F08-13, F16-23).

Ids: 24 ids: F01-06, F01-15, F02-10, F02-11, F04-06, F04-10, F05-03, F05-04, F05-05, F08-06, F08-11, F08-13, F09-06, F10-08, F12-15, F13-11, F16-23, F18-03, F18-16, F18-17, F22-11, F28-17, F32-21, F33-04

## 2. Rules that live only in a memory note or prose and need a home (or none)

Decision: For each rule that exists today only in one memory note, one index line or three prose lists, should it be moved into a doc or hook as the rule's home, or stay a memory-only habit?

Side A, give it a home. Example: `lean-agent-fanout.md:3,20-21` "8 per agent, 3-4 agents" with "no doc or hook" (F03-04). Agents would then read the rule in a doc and a hook or check could cite it.
Side B, leave it in memory. Example: `question-the-design-before-patching.md:11` is a rule "with no hook or doc" (F03-12). Agents who do not load memory would keep ignoring it.
Other example: `stored-versions-one-rule.md:17` kResultsStamp release rule, "no doc, hook or test" (F33-10); `this-machine.md` has "no dumpbin or ffmpeg line, paths live in three memory notes" (F10-09).

Ids: 17 ids: F03-04, F03-12, F03-14, F03-17, F10-09, F10-22, F13-03, F17-17, F18-11, F18-13, F20-11, F20-12, F28-09, F31-22, F32-07, F32-12, F33-10

## 3. Prose rules no hook checks

Decision: For each rule that exists as an instruction with no hook behind it, should a hook enforce it, or does it stay an instruction only?

Side A, add a check. Example: `agents/scout.md:9` "change no files except status and scratch" while `scout.md:5` grants Edit and Write and no hook limits it (F09-09). A scout writing a repo file would be refused.
Side B, instruction only. Example: `BP:15-19` says what a status line should say and `progress.py:26-27` accepts any text (F01-26). Agents would be trusted and the main session spot-checks.
Other example: `BP:77` forbids background runs while `agent_stop.py:44-51` "treats them as parked" (F13-21, F15-14).

Ids: 10 ids: F01-26, F02-05, F09-09, F13-07, F13-21, F15-14, F20-09, F24-08, F27-18, F28-19

## 4. What counts as a whole-library run, and the allow phrase

Decision: Should the whole-library rule be what the docs say (any run over the whole library, phrase asked for in chat) or what the hook checks (the `--whole-library` flag, with a tolerant phrase)?

Side A, the docs. Example: `agent-rules.md:14` and the memory note "cover any run over the whole library", and `BP:49` "exactly allow library run".
Side B, the code. Example: `percall.py:14` "checks only the --whole-library flag" and `review_rules.py:51-52` "accepts any case and a trailing . or !" (F17-01).
Agents would be refused (or not) on a compare run over many charts without the flag, and on a differently spelled phrase. Memory note `no-full-library-runs.md:15` says "No hook enforces it yet" (F18-01, F20-02).

Ids: 5 ids: F12-10, F13-22, F17-01, F18-01, F20-02

## 5. Deletes: which commands are refused or asked, and which worktree folders are safe

Decision: Should the delete hooks, cleanup sweep and settings agree on one list of folders and one list of commands, and which list rules?

Side A, the narrow list. Example: `delete_rule.py:14-18` trusts only `H\.claude\worktrees` and the user-level folder; `BP:43` names the same.
Side B, the wider practice. Example: `cleanup_sweep.ps1:174-178` also trusts `<root>\worktrees`, and "this audit's worktree sits at H\worktrees" (F10-11).
Commands: `settings.fragment.json:59-63` "asks on four patterns" while `delete_rule.py:19,31-35` "refuses a wider list for agents" (F12-07, F13-04, F16-03). Agents would meet a refusal where the main session only gets a question, or the reverse.

Ids: 7 ids: F07-11, F10-11, F12-07, F13-04, F13-05, F16-03, F33-02

## 6. Where a shell command is cut and how a quoted word is read

Decision: Should the hooks that read shell commands share one rule for cutting a command at operators and for quoted words, and which of today's readings is it?

Side A, quote-blind cut. Example: `delete_rule.py:29,77` "cuts at an operator inside quotes" and `delete_rule.py:66-69` does not read a quoted first word.
Side B, quote-aware. Example: `bash_windows_rule.py:40` "does not cut in quotes"; `shell_output.py:44-52` and `media_rule.py:14-15` strip quotes (F01-03, F12-04, F16-01).
Different cuts also give different answers for `a;b` (F16-02). Under each side a hidden delete or media write passes or is refused differently (F08-15).

Ids: 7 ids: F01-03, F08-15, F12-02, F12-03, F12-04, F16-01, F16-02

## 7. The same text read two ways: agent ids, hashes, keys and "did the user type this"

Decision: Where two code paths decide what a valid id, hash, key or typed prompt looks like, should one reading rule for all of them, and which one?

Side A, the strict reading. Example: `progress.py:26` accepts only `[A-Za-z0-9_-]+`; `review_rules.py:56` "lowercase only".
Side B, the looser reading. Example: `common.py:113-116` check_id passes "a.b"; `check_handoff.py:28` "upper case ok" (F06-06, F27-05).
Typed words: `review_rules.py:89-113` reads turnOrigin; `session_cards/common.py:372-435` reads origin.kind and strips reminders (F17-14, F21-02, F23-08, F27-12). Brief hash: `review_dispatch.py:172` sha256(prompt) vs `agent_stop.py:199` sha256(first user message) (F21-03, F27-09).

Ids: 19 ids: F06-06, F07-06, F17-08, F17-09, F17-14, F21-02, F21-03, F22-02, F22-08, F22-22, F23-08, F23-09, F24-10, F27-02, F27-04, F27-05, F27-09, F27-12, F27-22

## 8. Scripts and tools that set the same folder, timeout or number differently

Decision: Where two scripts or tools hold different values for the same setting (state folder, timeout, wait, flag name, install pick), which value stays?

Side A. Example: `build_slot.ps1:10,78` uses `...\.claude\hooks\state\build_slots`.
Side B. Example: `common.py:10 STATE_ROOT is ...\gates\state` (F11-01, F28-20, F29-04).
Other: `cleanup_sweep.ps1:58` GitTimeoutSec 600 vs `settings.fragment.json:81` timeout 30 (F01-05); `shot.ps1:2` Wait 10 vs `capture_window.ps1:56` 12 (F05-13, F16-20); `squeeze_scout.py:153` `--replay` vs `fcvideo.py:2906` `--hydra-replay` (F10-06).

Ids: 11 ids: F01-05, F05-13, F10-06, F11-01, F11-02, F11-03, F11-11, F16-20, F28-20, F29-04, F33-03

## 9. Two code paths answer one question differently (gates, checkers, tools)

Decision: Where two code places answer the same game or gate question differently, should they be merged into one reading, and which one?

Side A. Example: `review_rules.py:233-246 _carries` "scans for a passing commit".
Side B. Example: `ref_judge.py:305-316 made_from_existing` "two parents, no pass check" (F19-06, F33-06).
Other: `percall.py:163` main session = no agent_id vs `stall.py:421` "agent_id or no session_id" (F06-08); `progress.py:77-79` clears parked on any call vs `stall.py:236-240` "also requires last > since" (F26-09, F27-01); `game_files.py:276-290` song.ini keys before a header count vs `song.cpp:2217-2238` ignore them (F27-06); `squeeze_rating.cpp:185-188` two branches at one edge (F32-03).

Ids: 26 ids: F02-08, F06-08, F06-12, F08-10, F11-16, F12-05, F12-12, F13-01, F14-08, F16-07, F16-14, F17-04, F17-12, F17-13, F19-06, F25-12, F26-09, F27-01, F27-03, F27-06, F27-15, F29-16, F32-03, F33-06, F33-08, F33-12

## 10. What counts as "the user's words" for a ruling or a source

Decision: Which places may be cited as the user's words: only a line under ## Confirmed, or also a session card's quote, the brief's quote, chat and plans?

Side A, Confirmed only. Example: `CLAUDE.md:25` "a card quote is not a filed ruling" and `CLAUDE.md` "Only a line under ## Confirmed may be cited".
Side B, wider list. Example: `doc-review.md:63` and `derive-once-review.md:52` "accept a card's quoted words" (F24-02, F25-01, F26-03); `premise-map.md:45` adds chat (F30-12). Also what does not count as a source: `premise-map.md:52` vs rule 15 (F25-04).
Under each side a reviewer or planner accepts or rejects a card quote as proof.

Ids: 8 ids: F18-10, F24-01, F24-02, F25-01, F25-04, F26-03, F29-06, F30-12

## 11. Who defines a project word

Decision: Does CONTEXT.md alone define a project word, or does code usage, or a skill, also settle it?

Side A. Example: `agent-rules.md:41` "CONTEXT.md decides".
Side B. Example: `agent-rules.md:43` "code usage settles the meaning"; `domain.md:11` "lets a skill create terms" (F28-15, F29-13, F30-09).
Agents would stop and ask, or read the code and go on.

Ids: 3 ids: F28-15, F29-13, F30-09

## 12. Who writes a missing owner

Decision: When no existing code owns a question, may an agent propose or write a new owner, or must it stop and report?

Side A, may write. Example: `agent-rules.md:21` "new owner goes in core"; `derive-display-from-engine-truth.md:18` says propose one.
Side B, never. Example: `owner-map.md:37` and `scout.md:11` say "nobody proposes one" (F31-01).
Scouts and planners would either name a new owner or return the row as "none".

Ids: 4 ids: F28-18, F30-04, F30-15, F31-01

## 13. What holds a merge and what a reviewer follows

Decision: Does a held finding block the merge (BLOCKED) or stay a Note, and what else does the reviewer have to do or skip?

Side A, a finding holds. Example: `derive-once-review.md:149` and `reviewer.md:31` write BLOCKED; `DOR:106` holds for kind H, kind D.
Side B, nothing is held. Example: `one-reviewer-rule-d107.md:11` "do not hold"; `doc-review.md:98` "holds nothing" (F19-01); `MP:5` "nothing goes back to the executor" vs `agent-rules.md:11` "nothing goes back to the user" (F18-04).
Other: kind F wording (F18-12), known_clones entries (F18-20), a display-changing fold (F31-03), two report lists (F24-09), a sign-off with text after BLOCKED (F24-08 is in group 3).

Ids: 7 ids: F18-02, F18-04, F18-12, F18-20, F19-01, F24-09, F31-03

## 14. Build schedule, the word "build" and roles

Decision: Is a build one per branch before review or one per combined wave, and does "build" mean compiling or any piece of work?

Side A. Example: `model-postures.md:11` and `derive-once-review.md:198` "build each branch before review".
Side B. Example: `one-build-per-wave.md` builds "one combined branch" (F29-01, F32-01).
Word meaning: `agent-rules.md:9` "build is any piece of work" vs `agent-rules.md:20` "build means compiling" (F28-16, F29-10). Integrator: `integrate.md:27` hold last merge uncommitted vs `BP:71` commit after each step (F13-09); `integrate.md:3,41-46` vs `MP:5,11` (F29-18).

Ids: 6 ids: F13-09, F28-16, F29-01, F29-10, F29-18, F32-01

## 15. Agent files, what a brief carries, trailers

Decision: Should the agent files, brief lists and trailer rules match the prose rules, and which wins where they differ?

Side A, the prose. Example: `CLAUDE.md` rule 2 and `MP:5` "Sonnet or Haiku, read-only".
Side B, the agent file. Example: `agents/scout.md:4` pins `model: sonnet` and `:5` lists Edit, Write (F06-13, F07-01).
Brief contents: `MP:5` omits worktree and scratch folder; `PRE:3` omits the status rule; three lists (F06-15, F24-12). Trailers: `BP:67-69` asks for Co-Authored-By and your own id; `ref_judge.py:40,242-252` any alphanumeric and "orchestrator" (F07-09, F13-20). Status named in a brief (F22-24); who updates the Clone Hero page (F28-21).

Ids: 8 ids: F06-13, F06-15, F07-01, F07-09, F13-20, F22-24, F24-12, F28-21

## 16. Call counts versus minutes, warnings, and stuck agents

Decision: Is an agent limited by minutes only or also by call counts, and what does the warning and a stuck agent trigger?

Side A, minutes. Example: `CLAUDE.md:69` rule 7 "only time counts, not calls"; `review-and-hooks.md:34` "every 5 minutes ... reject after 10 minutes".
Side B, calls. Example: `review-and-hooks.md:33` "every 10 tool calls, reject after 15"; `progress.py:14-16` counts 100/150 calls (F01-01, F01-08, F07-08).
At the warning: `writer-brief.md:101` "stop starting new cards" vs `progress.py:94-100` "commit and report" vs `reviewer.md:11` "stop fixing, sign off" (F01-02). Stuck: `model-postures.md:11` nudge once then TaskStop vs `stall.py:124,143` no nudge (F28-02). Context warning text (F01-04, F04-05); git-chained exemption (F01-09).

Ids: 8 ids: F01-01, F01-02, F01-04, F01-08, F01-09, F04-05, F07-08, F28-02

## 17. A code value or comment that differs from a Confirmed ruling

Decision: Should the code follow the Confirmed line, or should the user change the line?

Side A, the ruling. Example: `review-and-hooks.md:13` Confirmed "Raise cap to 3,050" (F04-07, F07-16, F21-06).
Side B, the code. Example: `replay.py:41 LINE_BUDGET = 3100`.
Other: `review-and-hooks.md:22` "only large items over 20 lines" vs `items.h:50 kLargeRangeLines 400` (F02-12); `ui-and-display.md:56` lane must match chart notes vs `drum-lanes-d112` "every pad lit" (F31-06); `rulings/code-and-engine.md:22` bans history in comments vs `BP:119` why-comments (F29-07).

Ids: 6 ids: F02-12, F04-07, F07-16, F21-06, F29-07, F31-06

## 18. Plain English and writing-style rules copied in several places

Decision: Which wording of the plain-English and banned-phrase rules is the one home, and should the others point at it?

Side A. Example: `brief-preamble.md:123` keeps the gloss.
Side B. Example: `model-postures.md:13` "drops the gloss" (F28-13, F29-08, F30-17). `public-doc-style.md:27-31` nine phrases vs `~/.claude/CLAUDE.md:5` three phrases (F19-21, F14-12). Comments: `comments-explain-current-code.md` allows saying what a constant means vs `BP:119` "never restates the rule" (F26-13).

Ids: 6 ids: F14-12, F19-21, F26-13, F28-13, F29-08, F30-17

## 19. How agents run routine commands (gh, copying the database, shell and python spelling, git forms)

Decision: For each routine command, which of the docs' ways is the one agents use?

Side A. Example: `this-machine.md:81` copy `hydra.db` plus -wal and -shm by hand; `this-machine.md:51` use pwsh.
Side B. Example: `ui-testing.md:195` SQLite backup to one file; `gates README.md:152` `powershell -ExecutionPolicy Bypass` (F09-13, F14-02, F14-03).
Other: `issue-tracker.md:14` "hand-types gh issue close --comment" vs `this-machine.md:15` "text never goes through hand-typed gh" (F13-08, F13-17, F16-08); `BP:33` `python -I` vs `this-machine.md:4-8` `py` (F13-06); `BP:63` bans `add -A` and `add .` vs `git-traps` bans `add -A` and `add -u` (F15-01); hook tests agents may run (F13-23, F29-03).

Ids: 10 ids: F09-13, F13-06, F13-08, F13-17, F13-23, F14-02, F14-03, F15-01, F16-08, F29-03

## 20. Small one-offs

Decision: Each row here is its own small question; they are listed for one pass. Examples with both sides:
- Scrolling: `search-before-clicking-a-song.md:11,13` never scroll vs `render-checks-use-the-corpus.md:19` "Scroll to the middle" (F03-20, F14-10); also F05-08, F14-11.
- Above-optimal score: `SKILL.md:55` "bug, never a real score" vs `dmleaderboards-comparison.md:26-29` "NORMAL" (F20-04, F31-23).
- Plan review: `plans-reviewed-before-shown.md` "every plan" vs `doc-review.md:17` "committed plans are not gated" (F19-12); Out of scope list: `planner.md:13` vs `search-web-before-finalizing-plans.md:11-15` (F31-25).
- Proper names: `names-from-data-not-recall.md:15` web lookup vs `derive-once-review.md:76` only the repo (F32-06).
- Game build number: `build_facts.py:69-81` "1.1.0.6142" vs `dump_facts.py:381` "6142" (F25-19, F27-23).
- Also: stale or missing pointers and counts, doc-against-doc number differences, and one-line gaps (rest of the list).

Ids: 46 ids: F02-02, F02-13, F02-20, F03-02, F03-08, F03-20, F05-07, F05-08, F05-12, F07-02, F07-10, F07-17, F10-23, F11-04, F13-24, F14-10, F14-11, F15-15, F16-19, F17-16, F18-18, F18-21, F18-22, F19-12, F19-14, F20-01, F20-04, F20-06, F20-07, F22-13, F24-11, F25-19, F26-02, F26-15, F27-23, F28-06, F29-11, F30-10, F30-11, F30-13, F30-23, F31-14, F31-15, F31-23, F31-25, F32-06

## Counts per theme

Theme 1: 24
Theme 2: 17
Theme 3: 10
Theme 4: 5
Theme 5: 7
Theme 6: 7
Theme 7: 19
Theme 8: 11
Theme 9: 26
Theme 10: 8
Theme 11: 3
Theme 12: 4
Theme 13: 7
Theme 14: 6
Theme 15: 8
Theme 16: 8
Theme 17: 6
Theme 18: 6
Theme 19: 10
Theme 20: 46
Total: 238
