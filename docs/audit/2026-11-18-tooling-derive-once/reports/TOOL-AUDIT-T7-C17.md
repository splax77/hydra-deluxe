# TOOL-AUDIT-T7-C17 closure check (sorting S09, ledger L09; 25 ids)
Repo paths are relative to the worktree `C:\Users\Patrick\Downloads\Hydra\hydra-test\worktrees\tooling-audit-records`, except: `gates\` = C:\Users\Patrick\.claude\gates, `agents\` = C:\Users\Patrick\.claude\agents, `memory\` = the project memory folder.
Counts: PASS 22, REOPEN 3, GONE 0. Not reached: none.

### L-F25-02
**Verdict:** PASS (A)
**Homes read:** docs\agents\premise-map.md:56 (`### LS` then `-` or a digit, inside `## The live steps`); tests\test_docs_match_code.cpp:1625-1631 (`is_live_step_id`); agents\planner.md:13 and agents\reviewer.md:13 ("a heading that starts `### LS`").
**Why:** The agent files state a looser rule than the checking code. The page and the test agree, so code wins. No memory note or ruling is a side.

### L-F25-03
**Verdict:** PASS (P)
**Homes read:** tests\test_docs_match_code.cpp:1520,1525,1530; docs\agents\premise-map.md:15-17,37-45 (the same six Kind words); grepped premise-map.md for "2026-11-": no date copy.
**Why:** Words identical; dates live only in the test. Nothing disagrees.

### L-F25-05
**Verdict:** PASS (P)
**Homes read:** docs\agents\this-machine.md:17; CMakeLists.txt:21 (VERSION 3.1.2); docs\development.md:1011-1017 (`-Target v<version> -Title v<version>`); src\app\update_release.cpp:17-18.
**Why:** All agree. Pointer fix changes no behaviour. (No check that tag == v+version; a missing check, not a split.)

### L-F25-06
**Verdict:** PASS (P)
**Homes read:** docs\agents\this-machine.md:85 (five tables); src\store\record_store.cpp:271,280,289,852,865 (the five CREATE TABLEs), :929,1003 (DROP TABLE of old tables incl. `results`); tools\compare_db.py:130-136 (ten names).
**Why:** No src creates results, paths, path_refs, songmeta, dynamics; compare_db keeps old names to read older files. Page and schema agree today.

### L-F25-07
**Verdict:** PASS (A)
**Homes read:** docs\agents\clone-hero-mcp.md:52 (`kit_player="System"`); tools\ch_mcp\server.py:414 (`None`; docstring says pass "System" for a human launch); tools\ch_mcp\game_tools.py:1141,1245; tools\ch_mcp\kit_tools.py:92 (`KIT_PLAYER = "Player1"`), :1063, :1420-1428.
**Why:** Doc signature shows a default the code does not have. Doc against code only.

### L-F25-08
**Verdict:** PASS (P)
**Homes read:** docs\agents\derive-once-review.md:30; docs\agents\test-rubric.md:30,39,148.
**Why:** A display working out a rule the engine already has is a rule written twice; the pages use "kind A" the same way.

### L-F25-09
**Verdict:** PASS (P)
**Homes read:** tools\hydra_ui_mcp.py:81; gates\review_rules.py:49 (both `REALAPP_FILE = "realapp.jsonl"`); tools\test_hydra_ui_mcp.py:104,638 (uses hm.REALAPP_FILE).
**Why:** Equal values, no choice.

### L-F25-10
**Verdict:** PASS (P)
**Homes read:** tools\doc_shots\shrink.py:20-21 (the ledger's `tools\shrink.py` is stale); tests\test_docs_match_code.cpp:2024-2025.
**Why:** Same numbers, only copied.

### L-F25-11
**Verdict:** PASS (P)
**Homes read:** docs\agents\owner-map.md (Status words found/split/carry out/none); agents\scout.md:11, planner.md:19, reviewer.md:17 (restate only `carry out` and `none`); grepped tests\, tools\, gates\*.py, gates\*.ps1 for "owner map"/"owner-map": no hit.
**Why:** Agent files agree with the page. No test checks the shape: a missing check, not a disagreement. Weakest PASS (no copy to replace; P holds only as "nothing disagrees").

### L-F25-13
**Verdict:** PASS (A)
**Homes read:** docs\agents\rulings\README.md:43 ("any added line"), :51 (copies INSTALL_DATE 2026-11-14); gates\ruling_rules.py:99 (`INSTALL_DATE = "2026-11-14"`), :373-376 (skips blank lines, headings, title line), :8-9.
**Why:** The date matches today (a copy); "any added line" is wider than the code. README prose against gate code.

### L-F25-14
**Verdict:** PASS (P)
**Homes read:** docs\agents\issue-tracker.md:20-45; docs\agents\triage-labels.md; C:\Users\Patrick\.claude\settings.json:25,39,53 and gates\settings.fragment.json:15,29,43 (`gh api *` denied); gates\README.md:73.
**Why:** Pages agree with the deny lines. I did not call `gh`; the live label names are unchecked, which is not a disagreement among the homes.

### L-F25-15
**Verdict:** PASS (A)
**Homes read:** docs\agents\doc-screenshots.md:15 ("a `screenshot` line that has no ref"); tools\doc_shots\shots.txt:6-9; tools\doc_shots.ps1:27-38 (`^screenshot\s+(\S+)\s*$` directly after `^rect\s`); docs\agents\ui-testing.md:87; Reset-to-defaults words: doc-screenshots.md:25, shots.txt:12-20 against src\ui\library_dialogs.cpp:132 and src\ui\settings_bar.cpp:226.
**Why:** Page and shots.txt comment describe the script's one-argument test in other words. Doc and comment against a script.

### L-F25-16
**Verdict:** REOPEN (should be Q, with L-F27-18)
**Homes read:** docs\agents\ui-testing.md:237-253 (`commands` = command lines answered; "shows that a window was opened and used on that commit"); gates\review_rules.py:380-412 (`realapp_checked`: dirty False, `commands` int >= 1, commit on branch).
**Why:** The page alone states no gate condition. But L-F27-18 is the same question and is sorted Q (the gate passes a one-command session the page presents as real use). P needs "no choice of which value is right"; what counts as real use is a gate-strength choice. It must follow L-F27-18's exit.

### L-F25-17
**Verdict:** PASS (A)
**Homes read:** docs\agents\handoff.md:33-37,51-57 ("all 40 characters", "skips `docs/handoffs/`"); tools\check_handoff.py:28 (`[0-9a-fA-F]{40}`), :33; tests\test_docs_match_code.cpp:1233 (`kNotPermanentDirs` = docs/handoffs/ and docs/superpowers/); .claude\commands\handoff.md steps 3-4.
**Why:** Page and command file are looser than the checker and test (upper-case hex accepted; two dirs skipped). All doc/command text against code.

### L-F25-18
**Verdict:** REOPEN (should be Q)
**Homes read:** docs\agents\clone-hero-mcp.md:53 (`C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair`, "Edge Walk with one more kick 100 ms after its last one"); tools\ch_probe\probe_songs.py:64-68 (`EDGE_WALK = "Edge Walk"`, NAME_PREFIX); grepped tools\ for "Pair": only tools\ch_probe\tests\edge_walk_pair_schedule.json and test_stall_harness.py; the folder `C:\Clone Hero\songs\Hydra Probe\Edge Walk Pair` exists on disk.
**Why:** The code does not contradict the doc: it writes a different chart ("Edge Walk"). The doc names a second, hand-made chart that no program writes. "The code wins" would strike a real chart the page relies on. Whether the generator should write the Pair chart, or the page should say it is hand-made, is a choice, not a doc-against-code split.

### L-F25-20
**Verdict:** PASS (A)
**Homes read:** docs\agents\doc-screenshots.md:13; tests\ui\uitest_commands.cpp:287-292 (script-mode `refuse` logs `line N: <reason>`), :363-366, :382-385, :432 (`!! failed at line %d` only for engine-action failures); docs\agents\ui-testing.md:97 (states the split correctly).
**Why:** Page names the wrong message for an expect-text or rect miss. Doc against code.

### L-F25-21
**Verdict:** PASS (A)
**Homes read:** docs\agents\ui-testing.md:282 (two messages, "a week"); tools\hydra_ui_mcp.py:412-429,570-601,643 (four-plus messages); gates\cleanup_sweep.ps1:54 (`$ScratchAgeDays = 7`).
**Why:** Page lists fewer messages than the code and copies the age. Doc against code and a script.

### L-F25-22
**Verdict:** PASS (P)
**Homes read:** docs\agents\test-rubric-examples.md:7-9 ("as they stood at commit 003d76a3 ... the Preview's own refill-tick function is gone"); docs\agents\test-rubric.md:110-112,119-120 ("read as they stood at commit 003d76a3"); grepped src, tests, tools for `refill_tick`: no hit.
**Why:** Both pages date the quoted function and agree it is gone now. Small wording looseness (the "two lines" sample has three lines; 12 questions vs "answers 1 to 11" in the record) is not a value disagreement.

### L-F25-23
**Verdict:** PASS (A)
**Homes read:** docs\agents\this-machine.md:85 ("the chart's `md5`"); docs\adr\0028-a-chart-id-is-clone-heros-song-id.md (.srb id is the stored 16-byte checksum; `song_id_read` is the one owner); src\app\analysis.cpp:274-295 (.srb branch reads `read.fields.checksum`).
**Why:** Doc says md5 for every chart; code reads a stored id for .srb. Doc against code.

### L-F25-24
**Verdict:** PASS (P)
**Homes read:** tests\ui\live_protocol.h:56-89 (`reply_line`, `hello_line`); tools\fake_hydra_live.py:103-105,145-149.
**Why:** Same JSON keys today; the fix only points the fake at the header.

### L-F26-01
**Verdict:** PASS (B)
**Homes read:** memory\early-fill-orange-85-d250.md (two numbers, `kEarlyFillSlackMs`); CONTEXT.md:610-614 ("one number on both sides ... `Early fill window` ... 60 ms by default, from 0 to the hit window"); src\core\model.h:106,112-115 (`kHitWindowMs`, `kDefaultEarlyFillWindowMs`, `kMaxEarlyFillWindowMs`); grepped src for `kEarlyFillSlackMs`: none.
**Why:** Note names a constant that does not exist and a split window the code and CONTEXT.md do not have. Memory note against doc and code.

### L-F26-04
**Verdict:** REOPEN (should be Q, or split into a B part plus a separate fix)
**Homes read:** memory\relay-status-in-plain-english.md (skip `test_` lines; second sentence when evidence disagrees); memory\MEMORY.md:46 ("one plain sentence per agent"); gates\stall.py:30-33 (STATUS_HEADER: "Relay the claim ... When the evidence disagrees, add one sentence that says so"); grepped stall.py for `test_` and `fixture`: no hit.
**Why:** Two splits do not fit B. (1) The note's second sentence is not extra: the hook text itself says to add one sentence when the evidence disagrees, so the note matches the code there and the sorting's claim is wrong. (2) The MEMORY.md index line against the note is a memory note against a memory note, not against docs or code. Only the `test_` skip rule is a note against code.

### L-F26-05
**Verdict:** PASS (P)
**Homes read:** memory\windows-installer.md:12-16; installer\hydra.iss:23-24,52,73 (AppId rule, GUID, users-modify); installer\build_installer.ps1:13-14,82-86 (cmake --install; .db/.ini leak fails).
**Why:** Each listed invariant sits in one of the two named files. Loose heading wording only.

### L-F26-06
**Verdict:** PASS (P)
**Homes read:** memory\ch-disco-noflip-rule.md:15; C:\Users\Patrick\Downloads\Hydra\video-tools\layouts.json (splax-2026 score [736,418,326,84] disc [708,648,52,84]; splax-halfspeed [596,456,280,68] / [704,650,68,68]; splax-swim; biiiigboddy-handcam); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:60.
**Why:** No layout has the note's score 596,298,124,34 or disc 708,660,56,56, so "isn't in layouts.json" is true. Note and skill agree.

### L-F26-08
**Verdict:** PASS (P)
**Homes read:** .claude\commands\handoff.md steps 4-5; docs\agents\handoff.md:33,51-57,65; tools\check_handoff.py:33.
**Why:** The command and the page say the same about each status word, and the checker uses the same five.

## Ids not reached
None.
