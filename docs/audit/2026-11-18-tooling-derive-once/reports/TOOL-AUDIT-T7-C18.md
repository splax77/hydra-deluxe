# TOOL-AUDIT-T7-C18 (batch 17: sorting S09, ledger L09)

Paths: G = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, read as now). Memory = C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory.

Totals: 24 PASS, 1 REOPEN (L-F27-27), 0 GONE. All 25 ids reached.

### L-F26-10 (C)
**Verdict:** PASS
**Homes read:** G\waiver.py:3 ("the record forms are in gates/README.md"), :18-44 (writes kind, prefix, prompt_id, transcript_path, session_id, time); G\common.py:251-256 (append_waiver / read_waivers); G\README.md (grepped prompt_id, transcript_path, "kind", waivers.jsonl: lines 13, 69, 75 only say the log exists and is never cut; no field list).
**Why:** The comment points at a README that holds no waiver form. The current place is plain: waiver.py `_record` and `body`. Moved/missing-name case.

### L-F26-11 (A)
**Verdict:** PASS
**Homes read:** G\tests\test_docs_numbers.py:21-30 (five docs; ten RETIRED patterns incl. `every 10 tool calls`); docs/superpowers/plans/2026-10-25-hooks-from-scratch.md:108 (two patterns, "the preamble and CLAUDE.md") and :174 (T11 lists four docs).
**Why:** The split is a plan (a doc) against the test code. No memory note or ruling side.

### L-F26-12 (A)
**Verdict:** PASS
**Homes read:** docs/agents/doc-screenshots.md:15 ("a `screenshot` line that has no ref"); tools/doc_shots/shots.txt:6-9 ("full-frame screenshot"); tools/doc_shots.ps1:27-38 (comment "no bar"; code `^screenshot\s+(\S+)\s*$`, one argument).
**Why:** Three wordings (page, shots.txt comment, script comment) against the script's one-argument test. All doc/comment against real code.

### L-F26-14 (P)
**Verdict:** PASS
**Homes read:** G\tests\test_agent_files.py:1-25 (phrases only; its docstring says the files point at the page); docs/agents/premise-map.md:8-16 (heading `## Premise map`, four columns, `unchecked`); C agents planner.md:13,15 and reviewer.md:13.
**Why:** The test pins phrases in the agent files and never restates the page. No disagreement.

### L-F26-16 (P)
**Verdict:** PASS
**Homes read:** G\common.py:11-13 (REVIEW_KINDS), :36-48 (log_refusal fields time, hook, event, tool, session, agent, decision, reason); G\tests\test_refusals.py:16 FIELDS (same eight); G\tests\test_common.py:219-252 (handover fields match REVIEW_KINDS).
**Why:** Lists agree today; tests only repeat them.

### L-F26-17 (P)
**Verdict:** PASS
**Homes read:** G\tests\test_docs_numbers.py:55-59 (README_NAMES, APPROVAL_HEADING); G\README.md (grep: each of the eight names appears, counts 1-4); docs/agents/rulings/README.md:36 (`## How to file a ruling: the approval step`).
**Why:** Same strings in test, README and rulings page.

### L-F26-18 (P)
**Verdict:** PASS
**Homes read:** G\README.md:41,43,45,47-51; G\agent_stop.py:97-135, 280-345 (outcomes "verdict recorded", "no verdict", "crashed", "branch lost", "not a reviewer"; verdict_word; `_UNFINISHED` note).
**Why:** README prose (parked agent, marked first then verdict, lost branch = no verdict, BLOCKED/CLEAN whole-line rule, verdict_problem field) matches the code. README does not list outcome words, so nothing to disagree.

### L-F26-19 (B)
**Verdict:** PASS
**Homes read:** memory\early-fill-orange-85-d250.md (description and body name `kEarlyFillSlackMs` (60 ms)); MEMORY.md line 22 ("slack 60"); src/core/model.h:100-115 (`kHitWindowMs = 85.0`, `kDefaultEarlyFillWindowMs = 60.0` "the user's own number", `kMaxEarlyFillWindowMs`); Grep of src for `kEarlyFillSlackMs`: no hit.
**Why:** The note and its index line name a constant the code no longer has; the code has a user-setting default. Memory note against code only.

### L-F26-20 (B)
**Verdict:** PASS
**Homes read:** memory\dmleaderboards-comparison.md (routes /api/leaderboard, /stats, /songs/{md5}, /users/compare; "identifier (32-char MD5 chart hash)"; elsewhere "identifier is Clone Hero's song id"); src/net/dmbot_client.h:1-20, 44-55 (two endpoints; `identifier` is Clone Hero's song id); src/app/dm_report.h:1-4. (The ledger names src\app\dmbot_client.h; it now lives in src/net/.)
**Why:** The note against code. The code comment says song id, the note says MD5 in one line. No doc or ruling side.

### L-F27-07 (P)
**Verdict:** PASS
**Homes read:** tools/ch_mcp/frida_tools.py:66-68 (`DYNAMICS_STOP_BIT = 0x10`, points at SC-16); docs/clone-hero/06-scoring.md:175-177 (SC-16 "bit 0x10 of the engine flags word"); tools/ch_probe/constants.py (grep 0x10: none).
**Why:** One value, agreed.

### L-F27-08 (P)
**Verdict:** PASS
**Homes read:** tools/ch_mcp/kit_tools.py:55-66 (definition 1 "MIDI Drums", category 1, layout 2, "fact's home is block IN-76"); docs/clone-hero/07-internals.md:833-836 (IN-76: category 1 "Gameplay", layouts 0 and 2 "Drums") and :904-909 (IN-82: definition 1 "MIDI Drums", map id 2 at category 1 layout 2).
**Why:** Same values everywhere. Minor: IN-76 calls category 1/layout 2 the map for definition 6 while IN-82 says definition 1's map is also at category 1/layout 2; I read that as two maps and not a clash.

### L-F27-10 (A)
**Verdict:** PASS
**Homes read:** G\agent_stop.py:122-135 (comment "The one reader of the Agent trailer: stall.py's evidence line and signed_tip both call it"); G\ref_judge.py:40, 292-310 (`reviewer_reason` reads `values["Agent"]`; `TRAILER_KEYS`); G\stall.py:210-215 (calls agent_stop).
**Why:** A code comment says one reader; ref_judge reads the trailer again with another format. Comment against code. The line moved from :114 to :125.

### L-F27-11 (A)
**Verdict:** PASS
**Homes read:** docs/agents/rulings/README.md:41-45 ("refuses any added line that is not in the right shape"); G\ruling_rules.py:355-384 (`_judge` skips blank lines, HEADINGS, one title line of a new file, and on README only judges lines starting RULING_START).
**Why:** README prose against code behaviour. No other doc or note on a side.

### L-F27-13 (P)
**Verdict:** PASS (with a note)
**Homes read:** G\status_append.ps1:5-14 (literal `C:\Users\Patrick\.claude\gates\state`, comment says always the live folder even from a copy); G\common.py:10,121-125 (STATE_ROOT = gates/state; status/ and agents/ subfolders); G\progress.py:20-32 (STATUS_PREFIX, call regex); G\settings.fragment.json:4; docs/agents/brief-preamble.md:13-15.
**Why:** Values agree today. Note for the main session: the ps1 literal cannot be swapped for a path derived from the script, because its own comment says it must write the live folder when run from a copy. The fix is a pointer comment or an equality test, not a derived path.

### L-F27-14 (P)
**Verdict:** PASS
**Homes read:** tools/check_pointers.py:36, 251 (`SESSIONS_LINE = "Sessions:"`, skips such lines); C:\Users\Patrick\.claude\tools\session_cards\link_memory.py:23, 80-83 (`SESSIONS_PREFIX = "Sessions: "`, writer); CLAUDE.md:27.
**Why:** Same word; the reader is only more lenient (no trailing space).

### L-F27-16 (P)
**Verdict:** PASS
**Homes read:** tools/check_handoff.py:31-33 and 322-349 (CLAIMS_HEADING, MANUAL_KINDS test/ruling/state, TABLE_STATUSES, split_row, claims_rows); docs/agents/handoff.md:13-39, 51-55; docs/agents/brief-preamble.md:97; tests/test_docs_match_code.cpp:1553-1581 (`table_cells`, `is_table_row`, same rule as split_row/claims_rows).
**Why:** Heading, kinds, statuses and the cell rule agree in all homes.

### L-F27-17 (P)
**Verdict:** PASS
**Homes read:** tools/build_slot.ps1:5-14, 75, 78 (`$BuildSlotCount = 3`; folder `...\.claude\hooks\state\build_slots`, which exists on disk); docs/agents/agent-rules.md:20 (rule 14, three at once); docs/agents/rulings/agents-and-workflow.md:8 ("a limit of three builds at once"); G\percall.py:19-20,143; tools/hydra_ui_mcp.py:719-739 (only asks build_slot.ps1).
**Why:** Three everywhere. No page states the lock folder, so the folder is one home.

### L-F27-19 (P)
**Verdict:** PASS
**Homes read:** tests/ui/uitest_commands.cpp:412; tests/ui/uitest_engine.cpp:462; tools/hydra_ui_mcp.py:84, 656, 862; tools/doc_shots.ps1:25; docs/agents/ui-testing.md:87.
**Why:** `screenshot: ` is typed identically in all.

### L-F27-20 (P)
**Verdict:** PASS
**Homes read:** tools/capture_window.ps1:14-50 (type `HydraCaptureV1`, "Bump it whenever the C# block changes"); docs/agents/ui-testing.md:363 (same rule).
**Why:** Same rule, no conflict.

### L-F27-21 (A)
**Verdict:** PASS
**Homes read:** docs/agents/doc-screenshots.md:15, 29; tools/doc_shots/shots.txt:6-9, 42 (`rect //Hydra/##versionbar`); tools/doc_shots.ps1:27-47 (regex `^screenshot\s+(\S+)\s*$`).
**Why:** Page and comment wordings ("no ref", "full-frame", "no bar") against the script's exact test. Same as L-F26-12.

### L-F27-24 (P)
**Verdict:** PASS
**Homes read:** tools/ch_mcp/frida_tools.py:148-167 (ROW_ADDS_SCROLL: Confirm, Difficulty, Instrument, Modifiers; NO_ROW_PAGE Ready); tools/ch_mcp/menu_fields.json:10-22 (values 0-4 Confirm, Instrument, Difficulty, Modifiers, Ready); tools/ch_mcp/tests/test_frida_tools.py:786-789 (test_every_page_word_has_a_row_rule); docs/clone-hero/07-internals.md IN-58 (line 383+).
**Why:** Words equal and a test already holds them equal.

### L-F27-25 (P)
**Verdict:** PASS
**Homes read:** tools/ch_mcp/kit_tools.py:709-730 (six STOP_REASONS done, past_end, stopped, cap, frozen, jumped_back); tools/ch_mcp/agent/press_schedule.ts:50-76 (same six, with meaning).
**Why:** Same six words and event words. kit.ts lines not opened; the two readable homes agree.

### L-F27-26 (P)
**Verdict:** PASS
**Homes read:** src/core/model.cpp:34-41 (`dynamic_str`: none, ghost, accent); src/core/model.h:70; src/core/replay.h:140 (`{"accent",...},{"ghost",...}` score-field names in a table); tools/ch_mcp/kit_tools.py:258-264; tools/ch_mcp/compare_scorestats.py:40-46 (comment says it copies dynamic_str, needs the copy because it runs alone).
**Why:** Same words. Grep of src for "ghost"/"accent" found no other spelling.

### L-F27-27 (P) -- REOPEN
**Verdict:** REOPEN
**Homes read:** tools/ch_probe/constants.py:1-12, 38-200 (RVA_FORMULA_NORMAL "c1" = 0x3140738, c2 0x3140658, c3 0x31406F8, c4 0x3140700; CONST_NORMAL_FRONT 0x31406C8, etc.); tools/ch_probe/SPEC.md:66-110, 142-160 (offsets 0x20, 0x30, 0x38, 0x100, 0x2e0, 0x198, 0x8c and functions 0x20ddda0, 0x20df680, 0x20dcc90, 0x100dab0, 0x100e960 all agree with constants.py); SPEC.md:104-108; docs/clone-hero/code/6142/hit_window_formula_20DDDA0.asm:41,49 (the formula reads [0x3140738] dbl=-0.04 and [0x31406f8]); tests/test_docs_match_code.cpp:731-735, 999-1034 (data-address names are skipped; only method and OFF_ values are held to the 6142 table); docs/clone-hero/code/facts-6142.tsv:955-967.
**Why:** The function RVAs and field offsets agree, but SPEC.md:106 gives `0x0313ECB0 = 0.0110924370` as "the linear coefficient". constants.py calls the linear coefficient `c1` at 0x3140738, and the 6142 disassembly reads 0x3140738 as -0.04. 0x0313ECB0 appears nowhere in constants.py, in docs/clone-hero or in the code folder (searched with grep -i over docs/clone-hero and tools). So there is one real value disagreement (a doc page against code), and no test holds the data addresses to the table. It is not a P row. It should be category A (doc against code), or a question if the main session wants SPEC.md's quoted value judged by someone who knows which build it came from.

### L-F27-28 (A)
**Verdict:** PASS
**Homes read:** docs/agents/derive-once-review.md:21, 38 ("precheck covers kinds B, C and E", plus D numbers and F candidates; "Kind A ... no script can find"); tools/precheck/check_rows.cpp:241-249 (`row_kind` returns A, E, C or B) and :262-266 (check 2 prints "BA", check 1 "C", check 4 "E"); tools/precheck/check_rows.h:24-30.
**Why:** The page says the precheck finds no A and defines B as a recomputing test; the code prints A for production-code rows and B for a test row owned by production code. Doc against code only.

## Ids not reached
None.
