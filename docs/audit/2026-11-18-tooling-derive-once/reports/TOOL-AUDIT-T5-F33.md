# TOOL-AUDIT-T5-F33 (finder F33, 60 rows)

Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, M = the memory folder under C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory. I read each row in its T3 report, grouped rows by question, and searched the scope per group. I read only the delete-rule and test paragraphs of the preamble.

## Group A. Cleanup sweep numbers and rules (I2-087, 088, 089, 090, 092, 093, 094, 095, 096, 098, 100; I5-175; I9-164 to 168; I10-071; I7-043)

Owner code: G\cleanup_sweep.ps1:49 `$StartGapHours = 1`, :51 `$WorktreeIdleHours = 24`, :54 `$ScratchAgeDays = 7`, :56 `$InfoLogDays = 30`.

Homes of the values:
- G\README.md:69 names `$StartGapHours` and `$InfoLogDays`, says "Its first lines hold every number it uses", and names `Get-RunningSessions` as owner of the running test. Pointer, states no number.
- H\tools\hydra_ui_mcp.py:599-601 comment: "removes it once it has gone a week with no writes; $ScratchAgeDays in cleanup_sweep.ps1 owns that age". Names the owner but also states "a week" (a home). :770 docstring says "after a week with" with no owner named.
- H\docs\agents\ui-testing.md:213: "The SessionStart cleanup sweep removes a kept folder after a week with no writes". Home of the value, no owner named.
- G\shell_output.py:26 comment "which cleanup_sweep.ps1 clears": pointer.
- G\tests\test_h52_cleanup_sweep.ps1 uses its own values (2 days idle, 10 days stale, 40 and 10 day logs, a one-hour holdback tested at under an hour and at two hours, 60 s). Its comment says "The ages themselves live only in cleanup_sweep.ps1; these values sit well clear of them on both sides." It does not read the script's numbers, so it holds nothing equal.
- Retry copy (I2-098): G\cleanup_sweep.ps1:457 `100` tries with `Start-Sleep -Milliseconds 10`; comment C:453-454 claims "the same tries and wait as common._retry". Owner is G\common.py:14 `_RETRIES, _RETRY_SLEEP = 100, 0.01`. K1, agree today, the PowerShell copy cites the Python one.

What each says to do:

| Reader | Owner script | README | ui-testing.md / hydra_ui_mcp.py |
|---|---|---|---|
| Exactly at the limit | 24 h idle: kept when activity is at least that recent (`-ge`, C:258); 7 d: cutoff compare | no number | "after a week" with no side stated |
| Main session vs agent | a SessionStart hook runs it in the main session; agents never run it | same | helper user is the agent; told the folder goes after a week |
| Card-writer agents | not applicable | not applicable | not applicable |
| Worktree vs main checkout | sweeps `<root>\.claude\worktrees` in any repo and `<root>\worktrees` of the hook's own repo only (C:174-178) | no folder named | n/a |

Kinds: K1 (week in ui-testing.md:213 and hydra_ui_mcp.py:599-601, 770; retry numbers), K6 (splits below), K8 (state_prune.ps1 in side notes).

Splits (own lines):
- SPLIT-1, which folders hold task worktrees (I2-093, I9-164). The sweep: `<root>\.claude\worktrees` in any repo plus `<root>\worktrees` in ~/.claude only (G\cleanup_sweep.ps1:174-178). The delete rule: two fixed folders, G\delete_rule.py:17-18 `WORKTREE_FOLDERS = (H\.claude\worktrees, C:\Users\Patrick\.claude\worktrees)`. Prose homes of that list: H\docs\agents\brief-preamble.md:43, H\docs\agents\derive-once-review.md:197, G\README.md:136. Two code places decide it in different shapes. Neither covers H\worktrees\<task>, where this audit's own worktree sits (H\worktrees\tooling-audit-records).
- SPLIT-2, "which sessions still run" (I2-094). G\cleanup_sweep.ps1:355-389: registry pid alive, or transcript written in the last 7 days. C:\Users\Patrick\.claude\tools\session_cards\prepass.py:31 `RUNNING_CUTOFF_SECONDS = 2 * 3600`, reused at prune_list.py:114: a transcript newer than 2 hours is unfinished. Same question in words, two tests.

Tests holding homes equal: none for the sweep numbers. G\tests\test_docs_numbers.py reads progress.py numbers, not these.
Confirmed rulings: none found on sweep ages (searched H\docs\agents\rulings for week, hour, idle).

## Group B. Posture resend (I2-069, I2-070, I2-071, I5-162, I9-081)

Owner code: G\posture.py:16 `RESEND_MINUTES = 60  # the user's answer to Q8 of the hooks plan HX-P2`; send rule in the docstring :3-7; record `<STATE_ROOT>/posture/<session id>.json`.

Homes:
- G\README.md:35 states the rule in words (first prompt, again after RESEND_MINUTES, again at once after a compaction, the record path, matcher `compact`) and says "The number lives at the top of `posture.py`". Pointer for the value, but the rule is spelled out (K1).
- G\settings.fragment.json:109 registers SessionStart matcher `compact` for posture.py.
- H\docs\superpowers\plans\2026-10-31-harness-hooks-and-tools.md:306: "Q8 ... T2 picks 60 minutes ... Recommendation: 60." A plan home. It is a recommendation; the code comment calls 60 the user's answer. I found no `## Confirmed` line for it (searched H\docs\agents\rulings for posture, resend, 60 minutes).
- G\tests\test_posture.py:74 uses `posture.RESEND_MINUTES` by reference: pins code to itself, not to any doc.

What each says: posture.py sends at `now - sent >= RESEND_MINUTES*60`, so exactly at the limit it sends (test checks one second before). README says "once RESEND_MINUTES have passed". Main vs agent: an agent's hand-back fires the same prompt hook and cannot be told apart (posture.py:3-7), so the 60-minute rule is what limits it. Card writers: not applicable. Worktree vs main: same G\state folder.
Kinds: K1. Splits: none. Tests holding homes equal: none for the plan or README.

## Group C. Clone Hero server backups, devices and the save-file ruling (I10b-021, I10b-037, I7-066, I7-067, I7-068, I7-071, I7-072, I7-096, I7-101)

Owner code: H\tools\ch_mcp\game_files.py (REGISTRY_KEY `HKCU\Software\srylain Inc.\Clone Hero`, REGISTRY_BACKUP `rewired.reg`, PAUSE_ON_FOCUS_LOST_RUN_VALUE "0", LEFT_AS_SAVED, REG_CAP_S 30.0 with the comment "the user's ruling of 2026-10-30: Cap at 30 s", REGISTRY_LINE_NAME "rewired registry"). Device list: H\tools\ch_mcp\kit_tools.py:129 `KIT_DEVICES = ("keyboard", "mouse")`, comment cites "the user's rulings of 2026-10-30". H\tools\ch_mcp\agent\kit.ts:199, 380-381 also names the two devices in TypeScript (K6/K1 candidate: the TS decides which devices are switched off).

Prose homes of the same facts:
- H\docs\agents\clone-hero-mcp.md:15 (backs up one thing, "`backup` in game_files.py owns it", the file list scorestats.json, profiles.ini, profiles.bin, scoredata.bin, scoresext.bin, user.bin, both ruling quotes), :19 (key, `rewired.reg`, replies "rewired registry: unchanged" and "restored"), :21 (`pause_on_focus_lost = 0`, reply text, "Leave the line changed"), :25 (keyboard and mouse off, "The user ruled both on 2026-10-30"), :60 (scorestats.json path, `wait_stats`).
- H\tools\ch_mcp\README.md:81 (the same backup paragraph), :89 (keyboard and mouse, "as the user ruled on 2026-10-30").
- H\tools\ch_mcp\server.py:130-135, 160, 181-184, 311: tool texts the model reads, restating "No file in the game's folders is backed up or restored: settings.ini keeps the run's line, and scorestats.json and the ...".

Rulings found under `## Confirmed` (heading at H\docs\agents\rulings\clone-hero.md:7):
- clone-hero.md:8 "2026-11-13 chose "Leave the line changed" to "During a run, the tools change one line of Clone Hero's settings.ini ...""
- clone-hero.md:9 "2026-11-13 "stop messing with the game's save file. just let it stay.""
The 2026-10-30 rulings ("Cap at 30 s", keyboard "Switch it off (Recommended)", mouse "Switch it off too (Recommended)") are cited in code comments and in clone-hero-mcp.md:25 and README:89, but I found no line for them in the rulings folder (grep "Switch it off|Cap at 30": no hits). Their only homes are those comments and docs. Likewise I found no Confirmed line for the 2026-10-31 "question 4" keep of the menu walk (I7-101; homes: clone-hero-mcp.md:54 and :70, tools\ch_mcp\README.md:75, server.py:504 and :579).

What each says: all agree that no game-folder file is backed up or restored and the settings.ini line stays at 0. Main vs agent: ch_start works for either, texts name no difference. Card writers: none. Worktree vs main: a worktree runs its own copy of game_files.py; docs describe main.
Tests: H\tools\ch_mcp\tests\test_server.py:235 only mentions clone-hero-mcp.md in a comment; :266-270 pins the refusal text "settings.ini reads pause_on_focus_lost = 1, not 0". No test holds the docs and the code equal for these strings.
Kinds: K1 (many copies, agree today), K6 candidate (kit.ts and kit_tools.py device names). I7-096 (stats file copy): homes clone-hero-mcp.md:60 and server tool text; owner `wait_stats` in bot_run.py (named, not read).

## Group D. Putting files and index back after a refused move (I3-018, I9-159)

README G\README.md:53 says "`restore_checkout` in `ref_judge.py` owns that test" and spells the rule ("an existing commit or a clean merge"): a pointer that also states the rule.
Two code places answer "is this a clean merge of the base and a reviewed branch commit":
- G\ref_judge.py:305-316 `made_from_existing`: another ref holds the commit, or the commit has exactly two parents, parent 0 equals the current value, and `merge-tree --write-tree <both parents>` gives the commit's tree. Its docstring claims "(the same carry rule as review_rules._code_review)".
- G\review_rules.py:234-244 `_carries`: any commit k in `base..new` that `passes(k)` accepts, where `merge-tree --write-tree base k` gives new's tree; used at review_rules.py:251 for "carry".
The two differ: two-parent shape and parent-0 check and the "another ref holds it" shortcut in one; none of those in the other. K6 and K3 (a copy that claims to be the same rule and is not).
Tests: G\tests\test_ref_judge.py:451-605 test `restore_checkout` only; nothing holds the two merge rules equal.
What each says: exactly at the limit n/a; main vs agent: the gate judges any caller; card writers n/a; worktree vs main: restore acts only when the refused ref is the branch HEAD of the checkout the hook runs in (ref_judge.py:322-335), so a refused move in a worktree restores that worktree.

## Group E. Session card tools (I4-078, I4-104, I4-106, I4-107, I4-108, I9-172, I9-173)

- I4-104 Sources heading dropped: tools\session_cards\migrate.py:26-27 (`SOURCES`), common.py:49 comment "the user dropped it on 2026-10-31", test_session_cards.py:1173 `OLD_CARD_TAIL` repeats the block. No Confirmed line found (grep Sources, dropped, 2026-10-31 in the rulings folder: only unrelated lines).
- I4-106 protected folders: prune_list.py:22 `PROTECTED_FOLDERS = ("memory", "session-cards")`; common.py:34 names the same folder `"session-cards"`; test_session_cards.py:1095 and :1170 repeat both names; tools\session_cards\writer-brief.md:25 states the full data folder path. K1.
- I4-107: prune_list.py:114 uses `prepass.RUNNING_CUTOFF_SECONDS` by reference (not a copy). G\session_cards_start.ps1:16-17 describes the reminder.
- I9-173 500 MB and one day: owner G\session_cards_start.ps1:45 `$RemovalReminderMb = 500`, :47 `$ReminderGapHours = 24`, named in its header :16-17. The test uses 499.9, 500.0 and 25 h as its own thresholds. Not held equal. Exactly at the limit: 500.0 sends (per the row).
- I4-078, I4-108, I9-172: one home each (check.py, prune_list.py, the script and its test). Searched tools\session_cards, gates, H\docs\agents, H\docs\sessions\README.md, M.
Split: SPLIT-2 above (2 h vs 7 d running tests). Card-writer edge: card writers get tools\session_cards\writer-brief.md and skip the preamble; it states the data folder path and links the removed plan, no numbers from these rows.

## Group F. Other rows

- I1-012 report saved before budget check: owner G\percall.py:99-104. G\README.md:182 states "percall.py saves each SubagentHandback call's message when the call is made, and agent_stop.py reads that saved copy"; README:75 names `handback_path`. One owner, rule in prose (K1). Test G\tests\test_agent_stop.py exists; not opened.
- I2-061 watch token: owner G\wake.py:29-39; README:31 describes the watch but not the token (grep token, piling: no hit). One home.
- I3-094 what the fragment owns: owner G\sync_settings.py:8-17 (claims "nowhere else"); README:160 and :168 describe the flow, not the keys. Test G\tests\test_sync_settings.py:13-180 (I9-125) pins the behaviour. Single home plus its test.
- I3-099 hook table: owner G\settings.fragment.json; copied into the live C:\Users\Patrick\.claude\settings.json by sync_settings. Test holding two homes equal: G\tests\test_docs_numbers.py:142 `test_live_settings_carry_the_fragment` (skips in a worktree), plus `test_one_process_per_prompt`, `test_one_hook_after_a_tool_call`, `test_readme_names_new_records`.
- I3-080 (ruling check command, appears in a row of mine): owner G\ruling_rules.py:108 `CHECK_COMMAND`; copies G\settings.fragment.json:6 and live settings.json:12 (the allow rule). Test holding them equal: G\tests\test_docs_numbers.py:137 `test_fragment_allows_the_ruling_check` builds the rule from `ruling_rules.CHECK_COMMAND`. Also G\README.md:61 points at CHECK_COMMAND (pointer).
- I5-030 finished plan or handoff leaves the repo: H\CLAUDE.md:57 only. H\docs\agents\agent-rules.md:7 lists the docs check as part of the merge test list (the check itself). Searched handoff.md, integrate.md, .claude\commands\handoff.md for "remov", "leaves the repo": none. One prose home; the named test is the check.
- I6-143 triage row updated in the same step: H\docs\agents\audit-method.md:110 only (searched "same step" in docs\agents).
- I7-137, 138, 139: H\docs\agents\test-rubric-examples.md:210 and :263 both mention commit 3b699581 (same file); no other home. I7-162: H\docs\agents\this-machine.md:3 only.
- I8-169 bump kResultsStamp at release: M\stored-versions-one-rule.md:17 (and description line 3) and the index twin M\MEMORY.md:27. Not in H\docs\agents or .claude\commands (grep kResultsStamp, stored_versions: no hit). Owner is src\store\stored_versions.h (outside my scope). K7 candidate: a release-time prose rule nothing checks.
- I8-190 empty C:\ytdl folders: M\ytdl-folders-not-a-project-matter.md:11 and index twin M\MEMORY.md:47. No hook, no doc.
- I8-224 close_binary: C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:341-342; same advice in references\javascript-applications.md:173 and connection-and-recovery.md:84 in other words.
- I10-040 when a lock still counts: owner H\tools\build_slot.ps1 (`Test-BuildSlotLive`, :141-146, header :11-12). Test H\tools\test_build_pass.ps1:161-214. H\docs\agents\agent-rules.md:20 and brief-preamble.md:57 state rule 14 but not the lock test. docs\development.md is outside my scope.
- I10-066 window stop wait: owner H\tools\hydra_ui_mcp.py:42 `REPLY_MARGIN_S = 15.0`; H\docs\agents\ui-testing.md:280 names the constant (pointer).
- I10b-113: H\tools\test_ci_build_folder.ps1:1-15 restates the rule in ci_build_folder.ps1; the test is the second home and the pin.

## Rows not reached in depth
All rows got a name or question search. These were classed single-home from the row plus one grep, without reading the owner file: I1-012, I2-061, I4-078, I4-108, I6-143, I7-137, I7-138, I7-139, I7-162, I10b-113, I10-066, I7-096.

## Side notes
- G\session_cards_start.ps1:21 says "so state_prune.ps1 leaves it alone". No state_prune.ps1 exists in G (only this line names it). cleanup_sweep.ps1 (Invoke-StatePrune) is the owner. Wrong owner name (K8).
- H\tools\build_slot.ps1:10 and :78 keep slot locks in C:\Users\Patrick\.claude\hooks\state\build_slots while the gates state is G\state; both folders exist.
- A hook gap: none noticed.
