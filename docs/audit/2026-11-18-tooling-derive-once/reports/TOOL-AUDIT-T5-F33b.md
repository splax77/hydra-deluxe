# TOOL-AUDIT-T5-F33b (re-send of F33; 12 rows)

G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, SC = C:\Users\Patrick\.claude\tools\session_cards.
Rows taken (F33's "Rows not reached in depth" list): I1-012, I2-061, I4-078, I4-108, I6-143, I7-137, I7-138, I7-139, I7-162, I10b-113, I10-066, I7-096. F33's other unopened home, G\tests\test_agent_stop.py, was opened (under I1-012).
No `## Confirmed` ruling speaks to any row except the one named under I7-096. No test holds two homes equal except where named.

## I1-012 Is the hand-back report saved before the budget check, so an over-budget report is kept?
Homes:
- G\percall.py:99-102 (owner code): "# Saved first, so a report past the budget is kept; a failed save reaches body's report-tool message." then `if data.get("tool_name") == "SubagentHandback" and isinstance(message, str):` writes `agent_stop.handback_path(...)`.
- G\progress.py:18-19: "# Tools that hand a report back. They always pass, so an agent out of budget can still report." `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; progress.py:119 `exempt = data.get("tool_name") in REPORT_TOOLS`; percall.py:168 `not in progress.REPORT_TOOLS`.
- G\agent_stop.py:40: "Where percall.py saves an agent's last SubagentHandback message".
- G\README.md:11 "In a subagent it first saves a `SubagentHandback` call's report for `agent_stop.py`, then runs `progress.py`"; :41 "A saved hand-back always means finished."; :75 `handback_path`; :182 "Neither SubagentStop's input nor the transcript holds the hand-back for certain."
- G\tests\probe_results.md:71 "SubagentStop does fire after a SubagentHandback, but in 3 of 4 live reviewer stops the hand-back record was not yet in the transcript file".
- C:\Users\Patrick\.claude\agents\reviewer.md:25 "Hand it back with the `SubagentHandback` tool when you have it, or as your last message when you do not."

| home | what the reader does |
|---|---|
| percall.py | saves only a SubagentHandback message, before the budget check |
| progress.py | lets both SubagentHandback and StructuredOutput through when out of budget |
| README :11, :41 | names SubagentHandback only; a saved hand-back means finished |
| reviewer.md | agent may hand back by tool or as last message (no saved copy in the second case) |
| edges | at the limit: out-of-budget agent still passes; main session: branch is subagent-only; card writers: no part; worktree vs main: path is under G\state, one folder for all |

Kinds: K6 (two code places name the report tools: the percall.py literal "SubagentHandback" and progress.REPORT_TOOLS), K1 (README restates the order).
Split: StructuredOutput is "a tool that hands a report back" in progress.py:18-19, but percall.py:102 saves only SubagentHandback, so for a StructuredOutput report agent_stop has no saved hand-back.
Tests: G\tests\test_progress.py:400 `test_saved_past_the_call_budget_and_the_minutes_and_never_denied`, :406 overwrite, :418 failed save; G\tests\test_agent_stop.py:265, :525-549. None holds the two tool lists equal.
Rulings: none.

## I2-061 How do watches avoid piling up?
Homes: G\wake.py:30-34 (owner: token `"%d-%s" % (os.getpid(), os.urandom(8).hex())` written under `WATCH_SUFFIX`; "An older watch sees the new token on its next pass and ends quietly, so watches never pile up."); wake.py:25 `WATCH_SUFFIX = ".wake.json"`. G\README.md:31 "Each watch writes its token to `state\stall\<session_id>.wake.json`, and an older watch that finds a newer token ends quietly, so watches never pile up." (a home: states file name and rule).
Table: wake.py ends the older watch on its next pass; README says the same and adds the file name; README:31 says a subagent's stop does nothing (main session only); card writers not involved; worktree: one G\state.
Kinds: K1 (file name agrees today).
Tests: G\tests\test_wake.py:145-152 `test_newer_watch_ends_the_older_one_quietly` (behaviour via wake.claim, not the README text).
Rulings: none. Splits: none.

## I4-078 Which cards does a run with no card paths check?
Homes: SC\check.py:213-226 `default_cards` docstring "What a run with no card paths checks: the cards root's cards, the local cards, then the staged ones. A staged card replaces the published or local card of the same name ... index.md ... is left out as well."; check.py:5-7 module header says the same rule in words.
Second place choosing the same two folders: G\ruling_rules.py:215 `cards.CARDS_ROOT, cards.local_cards_dir(cards.DATA_ROOT)` (the ruling judge reads published + local; it does not use default_cards or leave out index.md or read staging). Also SC\link_memory.py:70-71 skips INDEX_FILE in the memory folder; SC\number_rulings.py:60 reads every .md under docs and memory (different question).
Table: check.py bare run = published + local + staged (staged wins by name); ruling_rules = published + local; main vs agent: no difference; card writers: they write to staging and run check.py (writer-brief); worktree vs main: both take the root from the cards module/parameters.
Kinds: K6 candidate (two code places choose which card folders count), K1 (docstring and header).
Tests: SC\test_session_cards.py:346 ("index.md beside it is not a card"), :685-690. None ties ruling_rules.py:215 to default_cards.
Rulings: none. Split: ruling_rules reads no staging folder, check.py does; only a split if both claim "the cards".

## I4-108 Which paths may be written to the prune list?
Homes: SC\prune_list.py:1-9 docstring ("It NEVER deletes or moves anything"; "a path only when listable() accepts it"), :23 `PROTECTED_FOLDERS = ("memory", "session-cards")`, :24 `SESSION_ID` full-UUID regex, :62-74 `listable`.
Other places: SC\common.py:34-35 builds the same two folder names ("session-cards", "memory") as path pieces; SC\check.py:38 `CARD_FILE` uses an 8-hex session prefix; G\session_cards_start.ps1:16, :346-350 run prune_list.py and tell the user the MB; G\delete_rule.py and H\docs\agents\brief-preamble.md:43 are the agents' delete rule (another question; they do not name these folders). No doc states prune_list's rule.
Table: prune_list lists only session-named .jsonl or folders directly in scope folders, no links; the user runs the delete; the hook only reports a count. Agents are barred by delete_rule. Card writers: no part.
Kinds: K1 (two protected-folder names stated in prune_list and as path pieces in common.py), K6 weak (session-id shape in prune_list.SESSION_ID and check.CARD_FILE).
Tests: SC\test_session_cards.py:685, :788 (build results); none holds PROTECTED_FOLDERS equal to common.py.
Rulings: none. Splits: none. One owner for the listable rule.

## I6-143 When is a finding's triage row updated?
Homes: H\docs\agents\audit-method.md:110 "The merge that fixes a finding updates its row in the same step. On 2026-10-05 the triage file still showed 192 of 396 findings open after phase 6 ..."; :168-169 ("A triage file that drifted": "192 of 396 findings open", card 2026-10-05-18216767); :108 "records each finding's exit and its evidence".
Searched with no other hit: G\*.py, *.ps1, README, agents\, both CLAUDE.md, H\docs\agents\*.md, memory notes, H\tools. The triage files (H\docs\audit\2026-10-03-fix-triage.json/.md) are the rows' home, outside my scope list.
Table: main session and executors "update the row in the same step" by prose only; derive-once-review.md has no step that checks a triage row.
Kinds: K7 (prose rule, nothing checks it), K1 (the count 192/396 stated twice on one page).
Tests: none. Rulings: none. Splits: none.

## I7-137 The commit that "added" the old test
Home: H\docs\agents\test-rubric-examples.md:210 "the commit that added it, 3b699581, is a bulk checkpoint that names no failure."
Checked: `git cat-file -t 3b699581` = commit; subject "Checkpoint: complete C++ port through Phase 6 alongside the Python oracle". The claim matches. The method behind it is H\docs\agents\test-rubric.md:84 (Q7: find the commit with `git log -S`).
Single home of the hash (searched docs, CLAUDE.md, G, memory). Kinds: none. Tests/rulings: none.

## I7-138 The page says what was removed and why
Homes: H\docs\agents\test-rubric-examples.md:338-342 ("An earlier version held three more examples ... The six good examples of the version after that were dropped too ... G1 was written to fill that gap."); :5 "No existing test was found that passes every question, so G1 was written for this page."; :218 "This test was written for this page." H\docs\agents\test-rubric.md:13-15 tells the page's own history ("An earlier version of this page asked ...", the 13440 check) and uses 13440 again at :25.
Table: readers are auditors and reviewers; no one is told to act. Edges: none apply.
Kinds: K1 (history told in two pages). Checked: the header says "Fourteen bad examples"; the file has 14 `### B` headings.
Tests: none. Rulings: none. Splits: none. (Memory note comments-explain-current-code.md covers comments, not docs.)

## I7-139 G1 stays only while its record holds
Homes: H\docs\agents\test-rubric-examples.md:218 "If a later reader finds an answer in that record that is wrong, G1 comes off this page; it does not stay with a note."; :5, :212 "G1 is that rewrite". H\tests\test_timing.cpp:73 has the quoted test name "timing: the combo multiplier steps up every ten notes and stops at x4", so "It is in tests/test_timing.cpp on main" holds today.
Nothing checks the page against the test (a rename drifts it silently). Kinds: K7 for the removal rule (once, no hook). Single home. Tests/rulings: none.

## I7-162 Staleness rule: "the machine wins. Fix the line in the same commit"
Home: H\docs\agents\this-machine.md:3 "Every path and version on it was checked on the machine on 2026-10-31 ... If a line here disagrees with what you see, the machine wins. Fix the line in the same commit as the work that found it."
Same idea elsewhere: H\docs\agents\agent-rules.md:15 and brief-preamble.md:91-95 ("The code and the source data are the source of truth"); H\CLAUDE.md:9 and brief-preamble.md:105 (a Clone Hero fact is put on its page "in the same commit as the work that found it"; "If your brief owns that page, update it"). H\CLAUDE.md:41 points at this-machine.md (pointer, no value). No memory-note hit for "machine wins".
Table: this-machine.md tells any reader to fix the line in the same commit; CLAUDE.md:9 tells a session to do so for Clone Hero pages; brief-preamble :105 limits it to an agent whose brief owns the page, so a reader without the page edits nothing (a possible split in who acts).
Kinds: K1/K3 (same-commit duty copied to three pages with different actors), K7. Tests: the docs check does not test dates. Rulings: none.

## I10b-113 CI build folder rule pinned by a test
Owner: H\tools\ci_build_folder.ps1:21-32 `$script:BuildFiles` = CMakeLists.txt, CMakePresets.json, build_cpp.ps1, tools/find_cmake.ps1, tools/cmake_presets.ps1, third_party/; `$script:OldTime = [datetime]::new(2000, 1, 1, ...)`.
Homes: H\tools\test_ci_build_folder.ps1:1-15 (restates cases in words); :61 `$old = [datetime]::new(2000, 1, 1, ...)` (its own copy of OldTime); :66, :99-105 use CMakeLists.txt only (one list entry). H\docs\development.md:882-890 (outside scope; side note): "the build starts cold ... when a build file changed since then; the script lists the build files" (pointer plus the cold-start causes). .github\workflows\ci.yml:466-469 names the test.
Table: the script decides; the test pins behaviour with a copied 2000-01-01; development.md tells a human the cases; main vs agent: CI only.
Kinds: K1 (2000-01-01 in script and test). The test holds the two equal (it fails if OldTime changes).
Tests: H\tools\test_ci_build_folder.ps1 itself. Rulings: none. Splits: none.

## I10-066 How long hydra_stop waits for Hydra to exit after quit
Owner: H\tools\hydra_ui_mcp.py:41-42 "# This is the one home of these three numbers." `REPLY_MARGIN_S = 15.0  # how much longer than the timeout in force to wait for a reply`; used at :314 and in `_stop_session` :453-468 `s.proc.wait(timeout=self.reply_margin_s)`, else kill, message "Hydra did not exit within {..:g} s of quit".
Homes: H\docs\agents\ui-testing.md:214 "waits for Hydra to exit, kills it if it does not" (no value); :280 names `REPLY_MARGIN_S` as the wait for each reply and says "All its numbers live at the top of tools/hydra_ui_mcp.py" (pointer); hydra_ui_mcp.py:776-778 docstring. Tests: H\tools\test_hydra_ui_mcp.py:489, :605, :1033, :1039 use `hm.REPLY_MARGIN_S` as a process wait (read the owner, no copy); :251, :265, :589 pass `reply_margin_s=1`.
Kinds: K3 (the constant is described as the wait "for a reply" in the comment and ui-testing.md:280, but `_stop_session` reuses it as the process-exit wait; ui-testing.md:214 gives no length). No value copy found.
Tests holding equal: none. Rulings: none. Splits in value: none.

## I7-096 Who copies the stats file, and where it is
Homes: H\docs\agents\clone-hero-mcp.md:60 "`ch_wait_stats` copies that file into the session folder only for a song launched from the command line, whether the bot or a human player (`human=True`) plays it (`wait_stats` in `tools/ch_mcp/bot_run.py`). For a song started from the menus, copy the file yourself before the next run." with the literal path `C:\Users\Patrick\Documents\Clone Hero\scorestats.json`; the same literal at clone-hero-mcp.md:15. H\tools\ch_mcp\README.md:83 "`ch_wait_stats` copies ... only for a song `ch_start` was given a folder for, which the bot plays from the command line. When a person plays the song instead, the game's own `Documents\Clone Hero\scorestats.json` still holds that run after `ch_stop`". H\tools\ch_mcp\server.py:135 (scorestats left as the game left it). H\docs\clone-hero\06-scoring.md:379 ("the tools read it (`SCORESTATS` in tools/ch_mcp/game_files.py ...)").
Code owner: H\tools\ch_mcp\game_files.py:36 `DOCUMENTS_DIR = os.path.join(_HOME, "Documents", "Clone Hero")`, :40 `SCORESTATS`; bot_run.py:448 path join; bot_run.py:794-801 `wait_stats` ("no song was launched from the command line; nothing to wait for" only when `run.cap_s is None`); bot_run.py:772-790 sets `cap_s` whenever a song_dir is given, bot or human (a human launch needs a song folder).
SPLIT (K2): ch_mcp\README.md:83 says the copy is for a song "which the bot plays" and that when a person plays, only the game's own file holds the run; clone-hero-mcp.md:60 and bot_run.py say a human launch with a song folder is copied too.
K1: the literal path with the user's name appears twice in clone-hero-mcp.md; the code derives it from `_HOME`.
Ruling found (line under the rulings file; I did not verify the heading above it): H\docs\agents\rulings\clone-hero.md:9 "- **2026-11-13** "stop messing with the game's save file. just let it stay." (session e14f7e34)". It covers leaving the file alone, not who copies it.
Tests: none holding page and code equal.

## Side notes
- G\session_cards_start.ps1:21 names `state_prune.ps1`, which does not exist (F33 noted it too).
- G\README.md:182 and G\tests\probe_results.md:71 both state the same 4-run live finding about transcript lag.
- A hook gap: none noticed.
