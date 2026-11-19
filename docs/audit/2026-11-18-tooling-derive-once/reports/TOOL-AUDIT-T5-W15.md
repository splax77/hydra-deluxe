# TOOL-AUDIT-T5-W15: second-round verification of ledger L10 rows

Short names as the ledger gives them: PC = Hydra CLAUDE.md, AR = docs/agents/agent-rules.md, BP = docs/agents/brief-preamble.md, MP = C:\Users\Patrick\.claude\model-postures.md, GR = gates README.md (C:\Users\Patrick\.claude\gates\README.md), DOR = derive-once-review.md, OM = owner-map.md, AM = audit-method.md, CH = clone-hero-mcp.md, UT = ui-testing.md, IN = integrate.md, H = C:\Users\Patrick\Downloads\Hydra\hydra-test. Every claim is by reading. I ran nothing.

Totals over the 20 rows: CONFIRMED 11, CONFIRMED-CHANGED 9, REFUTED 0. Not reached: none (L-F30-23 only partly checked, see its row).

### L-F28-07
**Verdict:** CONFIRMED.
**Homes as found:** gates\common.py:53-76 `run(body, mode)`: "closed: a crash denies the call", "loud: a crash prints a systemMessage", always returns 0 (line 70-71 builds the deny or the systemMessage). GR:95-100 says the same and adds "A hook that cannot start at all ... carries `onFailure: block`". cleanup_sweep.ps1:698-705 and session_cards_start.ps1:299-304: when the shared input helper fails, they print one stderr line and a systemMessage and `exit 0` ("D46: this hook cannot refuse, so ... does nothing and warns"). Both scripts' headers say "Exit 0 always" (cleanup_sweep.ps1:31, session_cards_start.ps1:23-25). D46 amendment at H\docs\audit\2026-10-03-fix-decisions.md:110: "fail loudly, never freeze ... Hooks that cannot refuse ... show a visible warning (systemMessage)".
**Splits:** None found. Python informing hooks ("loud") and the two PowerShell informing hooks all warn and exit 0. Refusing Python hooks deny; no refusing PowerShell hook was in this row. A PowerShell hook crash after the helper loads is caught only per-step (cleanup_sweep.ps1:296, 313, 650-658 add to a Failures list); I did not trace every path.
**Ruling:** agents-and-workflow.md:18 (Confirmed) "agents should fail loudly. freezing is unacceptable". It names no split.
**Open:** whether a PowerShell crash outside the per-step catches prints a systemMessage. Only a run would settle it.

### L-F28-08
**Verdict:** CONFIRMED.
**Homes as found:** cleanup_sweep.ps1:33-36 header names -ScratchRoot, -StateRoot, -ClaudeRepo, -ClaudeHome, -Foreground; param block :37-46 holds those plus -Worker, -Cwd, -SessionId (the three extra are internal worker arguments the header does not name). Proof line names tests\test_h52_cleanup_sweep.ps1 (GR:152 names it too). session_cards_start.ps1:27-31 header names -ToolsDir, -DataDir, -CardsDir, -StateRoot; param block :33-39 adds `-BudgetSec`, which the header lines 27-31 do not name. "The test uses all four" (line 31): test_h53 line 8 says "Every run passes -ToolsDir and -StateRoot"; -DataDir (lines 92, 245) and -CardsDir (93, 256) appear in some runs only. So "all four" is true across the file, not in each run.
**Splits:** None in values. Two header lists are shorter than their param blocks (-Worker/-Cwd/-SessionId; -BudgetSec).
**Ruling:** none found.
**Open:** none.

### L-F28-11
**Verdict:** CONFIRMED.
**Homes as found:** session_cards_start.ps1:267-268 comment ("a commit on main that changes only docs/sessions needs no reviewer, but every commit needs the Task, Agent and Session trailers"); the note text at :289 says the same. review_rules.py:419 `explain_main_move`; :440-443 computes `code = [p for p in changed + deleted if _is_code(p, entries)]` and `how = ... if code else "no-review-needed"`. `_is_code` (:181-186) with no "gate:" lines uses BUILTIN_CODE (:22): src, tests, tools, third_party, installer, .github, CMakeLists.txt, CMakePresets.json, build_cpp.ps1. H\.derive-once-gate holds no "gate:" line (I read the file: four lines of prose), so the built-in list applies. docs/sessions/ is not on it. The doc gate (:459-481) only looks at PUBLIC_DOCS (:24) and the marker files; docs/sessions is not there either. So such a commit gets "no-review-needed". The trailer rule is ref_judge.py's (a different gate; not opened).
**Splits:** None. The comment's list and the code agree today. The comment restates the rule instead of pointing at the owner (kind K1 by BP:119's own rule), but it is true now.
**Ruling:** none found.
**Open:** none.

### L-F28-13
**Verdict:** CONFIRMED-CHANGED. The row says BP drops the gloss. It does not.
**Homes as found:** global C:\Users\Patrick\.claude\CLAUDE.md "How to explain things" (lead with plain version, one idea per sentence, gloss, cut throat-clearing, short sentences, give the reason for "why", analogy, pre-show BAD/GOOD check). BP:121-123 "Lead with the plain version. One idea per sentence. Gloss any jargon in one line right after it. No bullet walls of file:line references ... That file is the one home of those rules." MP:13 "Write plain English: lead with the plain version, one idea per sentence, no bullet walls of file:line references." (no gloss). writer-brief.md:94 "Write in plain English, short sentences, no jargon without a gloss. Name files by name ... keep sentences around them." stall.py:32 only "in plain English". doc-review.md:81 has its own list (gloss on first use, short sentences, no wall of bullets, chat leftovers, boilerplate sections) and also calls the global file "the one home".
**Splits:**
- BP keeps the gloss; MP:13 drops it. They differ for a main session reading only MP.
- Neither BP nor MP carries "short sentences", "give the reason", "analogy", "cut throat-clearing" or the pre-show check; the global file does.
- BP:123 and doc-review.md:81 each call the global file "the one home" while each restates a different subset.
**Ruling:** agents-and-workflow.md:26 (Confirmed) "the status updates should be brief but relayed in plain english". It covers status updates and names no split. The long docs-and-writing.md:20 line ("how do I get you to listen") sits below `## Unconfirmed` (heading at line 17), so it is not the user's ruling.
**Open:** none.

### L-F28-18
**Verdict:** CONFIRMED-CHANGED. One claim wrong; one split found in AR itself.
**Homes as found:** AR:7-21 and PC rules 1, 3, 9-15 carry the same rule texts in short and long form; I compared each pair and found no number or wording that disagrees (three builds, 5 minutes, 100/150 calls, 20/30 minutes, `allow library run`, the three hydra programs). The row says the hook-side shared-code list "is only in AR:7". That is wrong. The same four items (`common.py`, `lib`, `settings.fragment.json`, `tests\lib`) are listed at AR:7, MP:11 ("gates\common.py, gates\lib, gates\settings.fragment.json, the test harness in gates\tests\lib") and GR:156. They agree today.
**Splits:**
- AR:21 rule 15 says "If none does, the new owner goes in core, and the plan's Premise map names the search". AR:23 says a display never gets a new rule, and AR:25 says that under an owner map "a question with no owner goes to the user first". OM:37 says "Nobody writes a new owner". The cases differ (code change without an owner map, against a plan under an owner map), and rule 15's own text does not say which one applies where. PC rule 15 carries only the "name the search that found none" form.
- AR:7 `-sf=*single_owner*` and PC:57 vs IN:43 `-tc="single-owner*"`: see ledger L-F29-17; not re-checked here.
**Ruling:** questions-and-audits.md and tests-and-runs.md not re-opened. The only user quote I read for rules 14 is agents-and-workflow.md:8-9; none names a split.
**Open:** whether the user quotes in AR:21/23/25 ("the correct rule is to use what hydra already knows", "Yes, write them") sit under `## Confirmed` anywhere. I grepped the whole rulings folder for "Yes, write them", "add a rule so sessions find" and "why do planners keep failing": no hit.

### L-F28-19
**Verdict:** CONFIRMED.
**Homes as found:** BP:41 "Touch only the files your brief owns ... Never delete, move or rewrite anything else: not other files in the repo, not other worktrees, not other agents' files in the scratch folder." delete_rule.py:161-163 refuses only a recursive or forced delete word (`DELETE_WORDS`, `_is_force_flag`) and git's discard commands (:155-157); :168 only looks at Bash and PowerShell. I searched gates\*.py for "owned files", "owns", "Touch only", "brief owns": no hook compares an Edit, Write, move or plain `rm file` against the brief's file list ("owns" hits in agent_stop.py are code comments about review_rules).
**Splits:**
- BP:41 says an agent never touches another worktree. BP:43 and GR:19 say the delete hook passes a path inside any worktree (it cannot tell which is the agent's). They differ for a delete inside another agent's worktree; BP:43 states the gap itself.
- BP:41 "move or rewrite" has no hook at all (K5).
**Ruling:** none found.
**Open:** none.
**Hook gap:** yes, in the delete hook and Edit/Write handling: nothing checks "move or rewrite" or an edit outside the owned files.

### L-F28-21
**Verdict:** CONFIRMED-CHANGED (a narrow split the row missed).
**Homes as found:** PC:5 and :9 (read first; update the page in the same commit); BP:105 (same, with "If your brief owns that page ..., If not, put the fact in your report"); card-writer.md:9 (reads the page list in docs/clone-hero/README.md and sets a card's `write-back:` line); premise-map.md:42 and :78 point to the page; ch-runs-must-not-need-focus.md:17 points to clone-hero-mcp.md. docs/clone-hero/README.md opens "the one home of what the project knows". One home, with pointers.
**Splits:**
- PC:9 says "A session or agent that learns a Clone Hero fact updates its page in the same commit"; BP:105 narrows it to an agent whose brief owns the page, and tells any other agent to put the fact in its report. They differ for an executor that does not own the page.
**Ruling:** none found.
**Open:** none.

### L-F29-05
**Verdict:** CONFIRMED (small correction).
**Homes as found:** PC rule 9 (CLAUDE.md:71), AR:15, MP:13 and BP:93-95 carry the same text; the handoff sentence is word for word in PC:71 and AR:15 and in BP:95. handoff.md:3 is not word for word: it says "It is another agent's summary, so CLAUDE.md rule 9 applies to it: before any new work starts from a handoff, every claim in it is checked against the code." Absence rule: BP:103, DOR:76, doc-review.md:65, AM:124 all say a search that finds nothing proves nothing, with the same meaning (BP adds "widen the search past the first name you tried"). BP:109-111 points at owner-map.md "The stops"; OM:3 says it is the "one home of the map's shape and of the two stops" and OM:35 holds the stops, so the pointer is true.
**Splits:** None.
**Ruling:** agents-and-workflow.md:10 (Confirmed) "so this whole time we've been accepting another agent's framing without verifying any of this is actually necessary." It names no split.
**Open:** none.

### L-F29-07
**Verdict:** CONFIRMED.
**Homes as found:** BP:115-119 (owner search, "a test calls the production function", new number needs a user decision, comments name where a rule lives and never restate it); DOR:82 "A comment that restates a rule is kind D"; DOR:104 "A comment that describes what a rule matches is a new copy too"; DOR:45 lists four places a decision can live; PC and AR:21 rule 15 (search core, app, ui, tools). The no-history rule for comments lives in memory comments-explain-current-code.md:11-15 and in the Confirmed ruling H\docs\agents\rulings\code-and-engine.md:22 (Confirmed section starts line 7, Unconfirmed at 61): "comments exist to explain the current code, not give a history lesson" (session c35a4b7d). BP and DOR do not carry that rule (BP:119 only says comments "may say why a rule exists").
**Splits:**
- BP:119 permits a comment saying "why a rule exists"; the Confirmed ruling at code-and-engine.md:22 says no history in comments. They differ for a "why" comment that tells how a value used to be. BP does not say which wins. The ruling names only the history side.
**Ruling:** code-and-engine.md:22, quoted above. It does not name BP:119's side.
**Open:** none.

### L-F29-08
**Verdict:** CONFIRMED-CHANGED. Same question as L-F28-13; the same correction applies.
**Homes as found:** global CLAUDE.md "How to explain things"; BP:123 (keeps the gloss); MP:13 (drops the gloss); PC:1 only points to the global file; stall.py:30-33 STATUS_HEADER says "Relay the claim to the user briefly, in plain English"; MP:11 says the same.
**Splits:** as in L-F28-13 (MP:13 drops the gloss; both BP and MP drop short sentences, reasons, analogy, throat-clearing, pre-show check).
**Ruling:** agents-and-workflow.md:26 (Confirmed) "the status updates should be brief but relayed in plain english". No split named.
**Open:** whether test_stall.py:305 still pins stall.py's phrase (not opened).

### L-F29-09
**Verdict:** CONFIRMED.
**Homes as found:** AR:9 "The main session's compiling and test runs inside approved work need no separate yes (rule 14)." memory one-build-per-wave.md:15 "before launching work that will need builds, say how many builds it needs and get a yes." (memory, written by a session; its note carries the user's words "3 builds at once? what? i didnt accept doing lots of builds" which are not under `## Confirmed`). MP:3 "A code-only change needs no approval of its design, but its build still waits." (here "build" is rule 3's piece of work, see L-F28-16).
**Splits:**
- AR:9 says no separate yes for compiles inside approved work; one-build-per-wave.md:15 says to state the build count and get a yes first. They differ when an approved plan implies many builds.
- MP:3 says "its build still waits" for the user's yes; AR:9 last sentence says compiling needs none. MP:3 uses rule 3's meaning of build, AR:9 rule 14's; the page does not say so (L-F28-16).
**Ruling:** agents-and-workflow.md:19 (Confirmed) "if anything changes a display in the app or a record, run it by me. if it only changes code you can use your recommendation without input."; :24 "you can't just go straight into writing code without confirming first. you need to tell me exactly how that is being counted right now." Neither names both sides. Note :8 says "a limit of one build per session"; it says nothing about how many builds a wave needs.
**Open:** session 8e6a2908 card not searched (docs/sessions).

### L-F29-12
**Verdict:** CONFIRMED.
**Homes as found:** BP:5 lists enforced rules: status line, call and time budget, delete rule, whole-library guard, Bash shapes, trim of long output, commit trailers. GR:11-17 lists what percall.py runs: progress, build-pass refusal, delete_rule, whole-library check, bash_windows_rule, media_rule, key-route check, shell_output rewrite. So BP:5 leaves out the key-route check (`allow key run`), the media guard (media_rule), the ruling check and the publish gate; the build-pass refusal is in BP:57 instead. AR:27 "Hooks check four of these" names progress, whole-library guard, ref gate, build-pass refusal (plus stall notice for rule 4). Of the docs I searched (H\docs\agents), only AM mentions media_rule/key_call. GR:116 holds the one-registration rule; settings.fragment.json has the Stop registration (timeout 660 at :235).
**Splits:** None in values. Three lists of "what hooks enforce" (BP:5, AR:27, GR) cover different subsets and nothing holds them equal.
**Ruling:** none found.
**Open:** whether test_docs_numbers.py holds any of the lists (not opened).
**Hook gap:** none.

### L-F29-18
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** IN:3 (the join gives the review and "the main session's one suite run" one tree), IN:5 (no new feature code), IN:21-23 (fold a copy; if a fold changes a displayed text, a score or a stored record "stop and report it. Do not pick"), IN:29 (rebuild appended scan rows), IN:41-48 (the main session builds once; scan and docs test; "Never loosen a scan row"), IN:60 (report). DOR:106 holds a parallel stop ("a change to a displayed text, a score or a stored record ... Do not make those changes"). DOR:34 kind E: "a scan row loose enough to let copies through". I searched test-rubric.md for "loosen", "scan row", "integrat": no hit (answers I6-116: "never loosen a scan row" has its only home at IN:48, with DOR:34 as the reviewer-side check). BP:3 names "integrators" as an agent kind. Agent files in C:\Users\Patrick\.claude\agents: card-writer, executor, planner, scout, reviewer; MP:5 lists those five as the only dispatch types. Searched "integrat" in model-postures.md, agents\*.md, gates\*.py: only MP:1 ("the integration"). So no agent type is named for an integrator.
**Splits:**
- IN:3 and IN:41-46: the main session builds the joined branch once and runs the scan and docs tests before review, then runs the merge's tests once after review. MP:11 and DOR:198 say to build each executor's branch before review. They differ in what is built before review (see ledger L-F29-01).
- BP:3 and IN:3 name an integrator role; MP:5 says every dispatch names one of five types and none is an integrator. They differ in how an integrator is dispatched.
**Ruling:** none found.
**Open:** which type the main session uses for an integrator. Not stated in anything I read.

### L-F29-19
**Verdict:** CONFIRMED.
**Homes as found:** BP:77 "Never use run_in_background ... You have no helper agents"; DOR:5 and doc-review.md:5 "You use no helper agents and start no background jobs"; scout.md:9 and executor.md:9 "You have no helper agents". All five agent files carry `disallowedTools: Agent` (line 6 of each): so "no helper agents" is enforced by the tool list. "No background jobs" has no refusal: I searched gates\*.py for `run_in_background`: only shell_output.py:75 (skips its rewrite for such a command) and agent_stop.py:46-51, :66-97 (counts a transcript's background tasks and marks the agent "parked"). progress.py:77 comments on an agent no longer parked. A command can also be moved to the background by the tool after a timeout (agent_stop.py:49-51), which the BP rule cannot stop.
**Splits:** None in text. BP:77 says never; agent_stop.py treats a parked agent as normal and keeps watching it (stall.py:239 prints "parked since"). They differ for an agent that starts a background job: the prose bans it, the hooks tolerate and track it.
**Ruling:** none found.
**Open:** none.
**Hook gap:** yes: nothing refuses `run_in_background` for an agent (G\progress.py, G\shell_output.py, G\percall.py).

### L-F29-20
**Verdict:** CONFIRMED.
**Homes as found:** AM:19-26 four exits (exit 3 needs a quote that "names both sides"; exit 4 needs one sentence; "on purpose" and "by design" not evidence); AM:36 quotes "the first pass should cover everything" (card docs/sessions/2026-10-11-8bffee2b.md:16 has "the first pass should cover everything. create a full plan for this.") and D119 "Leave nothing in a "pre-existing, left alone" pile." (decision log line 751); AM:44 calibration file: `git cat-file -t 707c285` is a commit and it holds src/core/squeeze_rating.cpp; AM:118-127 what every brief carries (matches BP:47-49 and AR:14 on the library run, and this brief's own list). PC "Audits" says "Every finding leaves by one of four exits ... A split goes to the user unless a ruling quoted in the user's own words names both of its sides". DOR:106 holds no four-exit list (I read it: it is "A finding you should not fix yourself"), so a cross-reference from DOR:106 to the four exits is wrong; the four exits live at AM:19-26 and are listed again at AM:121.
**Splits:** None. AM:126 lists a different set of places for the user's words than PC:41 (see ledger L-F30-12).
**Ruling:** none matched.
**Open:** whether the AM:36 quote "Leave nothing ..." appears under `## Confirmed` (it is in the decision log, quoting session b5aa7c31; I did not find it in the rulings folder).

### L-F30-04
**Verdict:** CONFIRMED-CHANGED. The row mixes two families.
**Homes as found (owner map):** OM:11-13 "A plan that only moves files, edits docs or changes hooks needs none." PC's Owner map paragraph and AR:25 say "A plan that changes what Hydra decides or shows is built from an owner map" and carry no carve-out. planner.md:19 restates OM:43; reviewer.md:17 restates OM:47 and adds "sign off BLOCKED"; scout.md:11 restates the table's columns and the stops (and "Never propose a new owner"); BP:111 points at "The stops". These are restatements, not just pointers. CH:43 and CH:58, and premise-map.md:78, are about the Premise map and live-run call lists, not the owner map.
**Splits:**
- OM:13 carve-out vs PC and AR:25 text: they differ for a plan that only edits docs or hooks.
- AR:21 rule 15 "If none does, the new owner goes in core" vs OM:37 and scout.md:11 "Nobody writes a new owner" (AR:25 resolves it for plans under an owner map, but rule 15's own sentence does not say so).
**Ruling:** OM:7 and AR:25 quote the user ("so why do planners keep failing to find the rules we already have?", "Yes, write them"). I grepped the whole rulings folder for those phrases: no hit, so they are not filed rulings.
**Tests:** I searched tests\test_docs_match_code.cpp and gates\tests for "Owner map" and "owner-map": 0 hits. Nothing checks the owner map text.
**Open:** I6-156 `SPSqueeze::needs_no_timing` comment not opened (OM:31 cites it).

### L-F30-09
**Verdict:** CONFIRMED-CHANGED. The finder's split is not the one it names.
**Homes as found:** rulings\README.md:22 "A ruling that uses a project word does not change what the word means. `CONTEXT.md` decides that." README.md:34 "If the reading and `CONTEXT.md` disagree, `CONTEXT.md` wins and the session asks the user." (a session's reading of a ruling against the glossary). DOR:51 and AR:41/43: "CONTEXT.md decides what a project word means" then "The code's usage settles the meaning ... If a definition's text and the code's usage disagree, ... question for the user ... Change neither". BP:99 holds only the second part. doc-review.md:63 holds the first. domain.md:43 "use the term as defined in CONTEXT.md"; domain.md:11 and :45 say the `/domain-modeling` skill creates or fills the glossary (skills dir C:\Users\Patrick\.claude\skills holds only analyzing-sp-squeezes, reverse-engineer-anything, synced, so I did not find that skill there).
**Splits:**
- README.md:34 and BP:99/DOR:51 are not opposite: README.md:34 covers a session's reading against the glossary ("CONTEXT.md wins and the session asks"); BP:99 covers the glossary text against the code's usage ("change neither"). They differ in the input that disagrees. Neither says what a session does when its reading and the code's usage disagree.
- DOR:51 / AR:41-43: "CONTEXT.md decides" and "the code's usage settles the meaning" are two answers to "who defines a word"; the next sentence sends a disagreement to the user. (Ledger L-F28-15 and L-F29-13.)
- domain.md:11/:45 let a skill create terms; BP:99 and AR:43 say no executor or reviewer rewrites a definition (the user's click approves). They differ for a session running that skill.
**Ruling:** I filtered rulings\*.md `## Confirmed` sections for "CONTEXT": the only hits are the two rule lines in README.md (22 and 34), which are the folder's rules, not user quotes. No user quote there names the CONTEXT.md-against-usage split.
**Open:** none.

### L-F30-17
**Verdict:** CONFIRMED.
**Homes as found:** public-doc-style.md:3 ("Internal docs follow the user's 'How to explain things' rules in ~/.claude/CLAUDE.md instead; this page adds to those"); BP:121-123 (four parts: plain version first, one idea per sentence, gloss, no bullet walls; "That file is the one home of those rules"); doc-review.md:81 (gloss on first use, short sentences, no wall of file:line bullets, no chat leftovers, no boilerplate sections; also "the one home"). Global CLAUDE.md holds more (reason for "why", analogy, throat-clearing, short sentences, pre-show BAD/GOOD check). C:\Users\Patrick\.claude\agents: only scout.md:9, executor.md:9 and planner.md:9 say "plain English"; card-writer.md:9 has none (its writer-brief.md:94 has it).
**Splits:**
- BP:123, doc-review.md:81 and writer-brief.md:94 are three different subsets of the global file's rule, and each calls the global file the one home. They differ in what a reader of one page sees. doc-review.md:81 adds two checks the global file does not hold.
**Ruling:** agents-and-workflow.md:26 (Confirmed) covers status updates only. None names a split.
**Open:** none.

### L-F30-20
**Verdict:** CONFIRMED-CHANGED. The finder's "different tools" is wrong; it is one function, and DOR:66 is the page that is off.
**Homes as found:** DOR:66 "`retime_from_end` in tools/ch_probe/experiments/play_chart.py moves each later press by the read end minus the planned end, one for one." CH:49: the player "moves each later press timed from that read by how far the read moved that press's own end ... the file gives each such press a `conversion`, and the player multiplies the read's error by it and works out no ratio itself." play_chart.py:361-369 `own_end_now_ms` = `own_stored_end_ms + conversion[side] * (read_end_ms - read_stored_end_ms)`, with `side` picked by the sign of the read against the stored end. The kit route runs the same function: play_chart.py:825 `presses[cursor + 1:] = press_messages(retime_from_end(rest, chord.index, end_read, chord_lanes))` inside `play_in_game`; kit_tools.py:19 and :800 say GamePlayer is "the kit agent as play_chart.play_in_game uses it"; kit_tools.py:770-774 `retime_message` carries "every entry not yet reported, as retime_from_end left them"; press_schedule.ts:78 "takes the entries Python's retime_from_end gave".
**Splits:**
- DOR:66 ("one for one") vs CH:49 and play_chart.py:361-369 (multiplied by the file's conversion). They differ whenever the conversion is not 1. This is the same split as ledger L-F29-14.
- play_chart.py:365 chooses the late or the early conversion by the sign of the read against the stored end, a rule hidden in a sign; whether that sign test matches replay_schedule.cpp's `end_conversion` (:267-276, not opened here) is not checked.
**Ruling:** none found.
**Tests:** test_server.py test named in the ledger tests the `timing` argument only (not re-opened).
**Open:** nothing a run is needed for.

### L-F30-23
**Verdict:** CONFIRMED-CHANGED (partly checked; the rows I opened are true, the rest not reached).
**Homes as found:**
- this-machine.md (TM:3): says "If a line here disagrees with what you see, the machine wins." TM:95 names `build_path_buttons()` and `build_activations()` in src/app/path_view.cpp; both exist: path_view.cpp:225 `ActivationsView build_activations(const Path& path, const HydraRecord& /*record*/, ...)` and :529 `PathButtonsView build_path_buttons(const HydraRecord& record, int depth_mode, int depth_value)`.
- ADR 0030 line 19 "Hydra's C++ side owns every command rule."; tools/hydra_ui_mcp.py:12 "The helper only carries messages. Hydra owns every command rule"; tools/test_hydra_ui_mcp.py:5 "every command rule and the helper only carries messages". Three homes, same sentence, agree.
- UT:317-327 holds the `hydra` helper trap and the user's words "you created the server, you figure it out", and rulings/ui-and-display.md:64 (Confirmed section runs line 7 to 102) holds "you created the server, you figure it out." (session 0ac4e8d8). Quote matches.
- issue-tracker.md:20 "`/triage` reads this flag" and :40 "Used by `/wayfinder`": skills C:\Users\Patrick\.claude\skills has neither; the Hydra repo's .claude\commands holds only handoff.md. domain.md names `/domain-modeling`, `/grill-with-docs`, `/improve-codebase-architecture`: also not in the skills folder.
**Splits:** None found in the rows I opened.
**Ruling:** ui-and-display.md:64 quoted above; it names no split.
**Open:** UT:360-367 and the remaining doc-style single homes were not opened; whether the `/triage`, `/wayfinder` and `/domain-modeling` skills exist elsewhere (a plugin) is unknown.

## Ids not reached
None. L-F30-23 was only partly checked (see its Open line).
