# TOOL-AUDIT-T7-C01 (closure checker, batch 0: sorting S01, ledger L01)

Short names: G = C:\Users\Patrick\.claude\gates; SC = C:\Users\Patrick\.claude\tools\session_cards; H = C:\Users\Patrick\Downloads\Hydra\hydra-test (docs read in the worktree copy, same files); BP = docs\agents\brief-preamble.md; AR = agent-rules.md.
Counts: PASS 25, REOPEN 0, GONE 0. Not reached: none.
Notes for the main session (not verdict changes): L-F02-07 passes only because no code states a total for the quiet check (see its section); L-F02-06 passes because every split is a memory note.

### L-F01-07
**Verdict:** PASS (A)
**Homes read:** G\session_cards_start.ps1:277 "(batch.py sizes a wave to the at-once limit)"; SC\batch.py:23-26 (MAX_BATCH_BYTES, MAX_BATCH_SESSIONS 10, BATCHES_PER_WAVE 10), :70 `wave, num = i // BATCHES_PER_WAVE + 1 ...`; AR rule 14 "three builds at once" (context only). Searched SC\*.py and the ps1 for "at-once"/"at once": only the ps1 line and an unrelated common.py:121 comment.
**Why:** One side is a prompt sentence in the hook script, the other is batch.py code. The code has no "at-once limit", only a wave of 10 batches. No doc, memory or ruling is a side.

### L-F01-10
**Verdict:** PASS (P)
**Homes read:** G\progress.py:31 `QUOTE_CHARS = 300`, :128 use; G\common.py:47 `reason[:300]`; G\README.md:81 "(its first 300 characters)".
**Why:** Two different outputs, same number, no disagreement; README agrees with common.py.

### L-F01-11
**Verdict:** PASS (A)
**Homes read:** BP:73 "20 minutes of real time counted from your first tool call"; progress.py:70-90 (record created with "first": now in `count`, before the status-call branch) and :87-90 comment "the status helper may have made the record already"; :96,:118 minutes from record["first"].
**Why:** A brief line against hook code. The first time can be set by a status-helper call, which BP:73 does not say. Code wins.

### L-F01-12
**Verdict:** PASS (P)
**Homes read:** G\common.py:10-22 (_RETRIES 100 x 0.01; _LOCK_WAIT 2.0, _LOCK_POLL 0.001); SC\common.py:140-146 (LOCK_WAIT_SECONDS 15, "25 s budget" in comment); G\session_cards_start.ps1:48-49 ($HookBudgetSec 25), settings.fragment.json:93 "timeout": 30.
**Why:** Different locks, different numbers, no disagreement; the 25 in the SC comment equals the ps1 constant.

### L-F01-13
**Verdict:** PASS (P)
**Homes read:** G\shell_output.py:14-17 (CAP 4000, HEAD/TAIL 1500); BP:35 "over 4,000 characters". Grepped docs\agents\*.md and G\README.md for 4,000/4000: only BP:35.
**Why:** Same value, owner says docs point there.

### L-F01-14
**Verdict:** PASS (P)
**Homes read:** G\stall.py:22-24 (STALL_SECONDS 600, LIMIT_SECONDS, STALL_NOTICES); AR:27 "no call for 10 minutes ... repeats at 20 and 30 minutes ... The step is STALL_SECONDS in stall.py"; G\README.md:29 pointer.
**Why:** All agree (600 s, 10/20/30 min); AR already names the constant.

### L-F01-16
**Verdict:** PASS (P)
**Homes read:** G\stall.py:22,25 (600, SCAN_EVERY 60); G\wake.py:11-12 comment, :63-66; G\settings.fragment.json:235 "timeout": 660.
**Why:** 660 = 600 + 60 and wake.py's comment says so; no disagreement.

### L-F01-17
**Verdict:** PASS (P)
**Homes read:** G\stall.py `TOOL_TAIL = 5`; BP:23 "(five is TOOL_TAIL in stall.py)" and "last five tool calls"; C:\Users\Patrick\.claude\model-postures.md:11 "last five tools".
**Why:** All say five.

### L-F01-18
**Verdict:** PASS (P)
**Homes read:** G\stall.py:39 `EMPTY_FLEET_SECONDS = 300`; G\README.md:29 "no agent has written for 5 minutes".
**Why:** 300 s = 5 minutes; agree.

### L-F01-19
**Verdict:** PASS (P)
**Homes read:** G\prompt.py:19-21 `FIELD_CAP = 10000`; G\tests\probe_results.md:127 and :223 "about 10,000 characters" (the failed-tool excerpt); G\README.md (names FIELD_CAP).
**Why:** Two different Claude Code caps with the same number; nothing says different values.

### L-F01-20
**Verdict:** PASS (P)
**Homes read:** G\session_cards_start.ps1:43-49 (ClaimHours 2, RemovalReminderMb 500, ReminderGapHours 24, HookBudgetSec 25 with comment "30 s timeout, less 5 s"); G\settings.fragment.json:91-93 "timeout": 30; SC\common.py:141 "25 s budget".
**Why:** 25 and 30 and 2 h/500 MB/24 h all agree; only the relation "30 less 5" is prose in a comment.

### L-F01-21
**Verdict:** PASS (P)
**Homes read:** G\ruling_rules.py:99 `INSTALL_DATE = "2026-11-14"`; docs\agents\rulings\README.md:51 "(`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14)".
**Why:** Same date; README already names the constant.

### L-F01-22
**Verdict:** PASS (P)
**Homes read:** SC\check.py:30 `MAX_CARD_LINES = 60`, :112-113; SC\writer-brief.md:71 "at most 60 lines; aim for 20 to 40", header "exactly those six key: value parts"; docs\agents\rulings\agents-and-workflow.md:15 (Confirmed) "continue, and raise the cap to 60."
**Why:** 60 everywhere. (Header shape not line-compared; writer-brief:71 and check.py were both opened and nothing disagrees on the cap.)

### L-F01-23
**Verdict:** PASS (P)
**Homes read:** SC\prepass.py:31 `RUNNING_CUTOFF_SECONDS = 2 * 3600`, :391; SC\prune_list.py:114; SC\test_session_cards.py:1009,1011 literals `OLD + 7200`.
**Why:** 7200 equals 2 h; agree.

### L-F01-24
**Verdict:** PASS (P)
**Homes read:** SC\batch.py:68-71 writes `wave-{wave}-batch-{num}.txt` and line shape `sid TAB path TAB KB`; G\session_cards_start.ps1:261-262 reads `wave-(\d+)-batch-(\d+)\.txt`; SC\writer-brief.md line shape (searched "wave-" there: no hit; the line shape is stated in the batch-file section).
**Why:** The name format is typed in batch.py and the ps1 and they match today.

### L-F01-25
**Verdict:** PASS (P)
**Homes read:** SC\writer-brief.md:99-101 (status call `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "HH:MM done <n> of <m> cards | next: <session>"`); G\progress.py:22 STATUS_PREFIX, :26-27 `_STATUS_CALL` (text may not hold `$`, backtick, double quote, curly quotes), :58.
**Why:** The copy fits the accepted shape; no split.

### L-F02-01
**Verdict:** PASS (P)
**Homes read:** docs\agents\derive-once-review.md:9 and doc-review.md:9 ("Append one line ... Use `& ...status_append.ps1` ... to write it, alone in its own call"); BP:13-15 ("Do not write the file with Edit, a heredoc or `>>`").
**Why:** DOR/DR name the helper too, so they do not contradict BP; they only omit the prohibition. Replacing with a pointer changes no behaviour.

### L-F02-03
**Verdict:** PASS (P)
**Homes read:** CLAUDE.md:66 and AR:10, identical text "Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke."; rulings\agents-and-workflow.md:15-18 area (Confirmed line quoted by the ledger, not re-opened beyond the grep).
**Why:** Identical copies; no split.

### L-F02-04
**Verdict:** PASS (P)
**Homes read:** BP:79-85 "Any step that repeats or waits needs a cap ... a line saying what to report when the cap is hit"; audit-method.md:40 "Every stage that repeats needs a cap and a line saying what happens at the cap"; clone-hero-mcp.md:37 "Every wait has a cap and says so in its reply ... No tool retries on its own."
**Why:** Same rule, different audience; nothing contradicts.

### L-F02-06
**Verdict:** PASS (B)
**Homes read:** memory no-full-library-runs.md:11 "`testdata/input` (about 115 charts)"; memory questions-describe-the-real-rule.md:24 "Strip the `[N/97]` prefix"; docs\agents\ui-testing.md:46,48 "98 charts"; tests\test_engine_sweeps.cpp:656 `CHECK(charts == 98)`. Counted today: 98 song.ini and 98 notes.chart/notes.mid/*.sng files under testdata\input.
**Why:** The only splits are two memory notes (115, 97) against docs and code (98). The ui-testing drift claim was refuted earlier and ui-testing.md still agrees with the files.

### L-F02-07
**Verdict:** PASS (P) with a flag
**Homes read:** tools\ch_mcp\quiet.py:29-31 (QUIET_SAMPLE_S = 5, used :197 and :203); tools\ch_mcp\server.py:128,:158 "about 6 s"; docs\agents\clone-hero-mcp.md:15 "adds about 6 s; quiet.py owns the check".
**Why:** The code states only the 5 s CPU sample; no code says a total, and the process scan time is not held anywhere. So "about 6 s" is a measured-looking claim with nothing to disagree with. Flag: if the main session counts 5 against 6 as a split, this should be reopened as a question. Searched quiet.py for other sleeps or intervals: none besides line 355 (a generic helper).

### L-F02-09
**Verdict:** PASS (P)
**Homes read:** tools\doc_shots\shrink.py:17-21 (500 * 1024, 4 * 1024 * 1024, "checks the same two at merge"); tests\test_docs_match_code.cpp:2024-2025 same values; docs\agents\doc-screenshots.md:17 names shrink.py's constants.
**Why:** Values agree in both code homes; doc points to the owner.

### L-F02-14
**Verdict:** PASS (P)
**Homes read:** docs\agents\premise-map.md:68 "A step with two or more is a shape error"; tests\test_docs_match_code.cpp:1849 `if (never_tried >= 2)`.
**Why:** Same rule in doc and test.

### L-F02-15
**Verdict:** PASS (P)
**Homes read:** docs\agents\audit-method.md:13,23,135,140 (the 10 ms band told as history: "a constant D269 has since deleted"); src\core\rules.h:30-32 `backend_leeway_ms = 3.0`.
**Why:** The doc tells the old band as history and the 3 ms leeway agrees with code; no live claim of 10 ms.

### L-F02-16
**Verdict:** PASS (P)
**Homes read:** docs\agents\public-doc-style.md:7 (ten linked sources); docs\research\ai-generated-docs-review.md (its own "Sources:" paragraphs, a different subject: failures of AI-written documents).
**Why:** Two lists for related but different questions; nothing contradicts.

## Ids not reached
None.
