# TOOL-AUDIT-T5-W01 (second-round verifier, 20 rows from TOOL-AUDIT-T5-L01)

Short names as in the ledger: G = C:\Users\Patrick\.claude\gates; H = C:\Users\Patrick\Downloads\Hydra\hydra-test; SC = C:\Users\Patrick\.claude\tools\session_cards; BP = H docs\agents\brief-preamble.md; AR = agent-rules.md; DOR/DR = derive-once-review.md / doc-review.md. Every claim below is "by reading" unless it says otherwise. I ran nothing. Rulings dir = H docs\agents\rulings\.

### L-F01-09
**Verdict:** CONFIRMED (by reading), with one split the row missed.
**Homes as found:** G\progress.py:19 `REPORT_TOOLS = ("SubagentHandback", "StructuredOutput")`; :66 `command = str(...) .lstrip() if shell else ""` (so non-shell tools get an empty command and are exempt only through REPORT_TOOLS); :119 `exempt = data.get("tool_name") in REPORT_TOOLS or command.startswith("git ")`. BP:21 "a command that starts with `git`, and your final report". G\README.md:124 "a git command and the final report still pass".
**Splits:**
- `git` alone with no space is not exempt (the code tests `"git "`); BP:21 says "starts with `git`". Situation: a bare `git` call. The code answers "starts with git plus a space"; BP answers "starts with git". Edge only.
- Missed by the row: the test is on the start of the whole command text, not of one part. A command such as `git status && <anything>` starts with `git ` and so is exempt as a whole (the hook does not split parts here, unlike delete_rule). BP:21 reads as "a command that starts with git", which is what the code does; whether that is meant is not stated anywhere I read. (Hook gap, general words: the exemption looks at the start of the text only.)
**Ruling:** none searched under Confirmed for the git exemption beyond the status lines; review-and-hooks.md:34 names status updates and refusing after 10 minutes, not the git exception.
**Open:** a run with a bare `git` call or a chained command would settle the edge (not run).

### L-F01-10
**Verdict:** CONFIRMED (weak K1).
**Homes as found:** G\progress.py:31 `QUOTE_CHARS = 300`, used at :128 `command[:QUOTE_CHARS]`; G\common.py:46 `"reason": reason[:300]` in the refusal log.
**Splits:** none by value. Two different outputs (what a refusal quotes to the agent; what the log keeps) with the same literal 300 and no link between them.
**Ruling:** none.
**Open:** none.

### L-F01-11
**Verdict:** CONFIRMED-CHANGED.
**Homes as found:** G\progress.py:147-152 `_status_time` returns max(status file mtime, `record["first"]`), and falls back to `record["first"]` when there is no status file ("its first call stands in until it writes one"). Minutes for the time limit come from `record["first"]` (progress.py:~106 `minutes = (now - record["first"]) / 60.0`). BP:73 "counted from your first tool call"; DOR:90 and DR:91 as the ledger says (not re-opened).
**Splits:** the row says the record is created on the first call. progress.py:85-87 shows a comment "the status helper may have made the record already" (`record["calls"] == 0`). So the record, and with it `first`, can be made by the status helper before the agent's first counted tool call. BP:73 says "your first tool call". Situation: an agent whose first call is the status helper. Not run-checked.
**Ruling:** none found.
**Open:** which code writes `first` when the helper makes the record (status_append.ps1 / common.update_agent not opened by me).

### L-F01-12
**Verdict:** CONFIRMED.
**Homes as found:** G\common.py:15-18 `_RETRIES, _RETRY_SLEEP = 100, 0.01` (file replace retry) and `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001` (agent record lock); SC\common.py:140-145 `LOCK_WAIT_SECONDS = 15` with the comment "15 s keeps the SessionStart hook inside its 25 s budget", `LOCK_STALE_SECONDS = 2 * 60`, `LOCK_POLL_SECONDS = 0.05`, "The user's choices (2026-10-25)". SC\test_session_cards.py:1296 reads `common.LOCK_STALE_SECONDS` (derived).
**Splits:** two lock-wait owners in two tool families (2.0 s vs 15 s, polls 0.001 vs 0.05). Different locks, so different numbers are not by themselves a conflict. The 25 in SC\common.py:141 is a prose copy of G\session_cards_start.ps1:49 (see L-F01-20 in the ledger).
**Ruling:** "The user's choices (2026-10-25)" is a code comment. No Confirmed line searched for it (I did not grep rulings for it).
**Open:** none.

### L-F01-13
**Verdict:** CONFIRMED.
**Homes as found:** G\shell_output.py:15 `CAP = 4000`; :16 `HEAD_CHARS, TAIL_CHARS = 1500, 1500`; :18 `ERROR_CHARS = CAP - HEAD_CHARS - TAIL_CHARS` (derived); :183 `if len(text) > CAP` (so exactly 4000 is not trimmed). BP:35 "A result over 4,000 characters"; G\README.md:23 names the one hook, :154 test_docs_numbers reads docs. test_docs_numbers.py:198 `test_preamble_quotes_the_output_cap` (name read; body not opened).
**Splits:** none. 1500 appears in no doc I opened (README/BP grep for 4,000 only).
**Ruling:** none searched beyond the ledger's.
**Open:** none.

### L-F01-14
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:22 `STALL_SECONDS = 600`, :24 `STALL_NOTICES = LIMIT_SECONDS // STALL_SECONDS` (derived from progress.MINUTES_STOP), :25 `SCAN_EVERY = 60`. AR:27 "a stall notice when an agent has made no call for 10 minutes ... The notice repeats at 20 and 30 minutes of the same silence, and the third says it is the last. The step is STALL_SECONDS in `stall.py`, and the cap is the progress hook's time limit, MINUTES_STOP in `progress.py`." So AR:27 states 10/20/30 as numbers and also names the constants.
**Splits:** none. 10 as `STATUS_DENY_MINUTES` (status age) and 600 as `STALL_SECONDS` (silence) are different questions with the same number.
**Ruling:** none found (not searched under Confirmed for "stall").
**Open:** test_stall.py/test_wake.py pins not opened by me.

### L-F01-16
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:25 `SCAN_EVERY = 60`; G\settings.fragment.json registers wake with "timeout": 660 (grep hit seen); G\tests\test_wake.py has `hooks[0]["timeout"] == stall.STALL_SECONDS + stall.SCAN_EVERY` (grep tail shows `ks[0]["timeout"], stall.STALL_SECONDS + stall.SCAN_EVERY)`), so 660 is held equal to the sum by a test. wake.py comment names the sum.
**Splits:** none; 660 = 600 + 60 holds today.
**Ruling:** none.
**Open:** none beyond: only a run would show the test passes.

### L-F01-17
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:34 `TOOL_TAIL = 5 # display size`; :207 `names[-TOOL_TAIL:]`. BP:23 "(five is TOOL_TAIL in `stall.py`)". C:\Users\Patrick\.claude\model-postures.md:11 "its last five tools". G\README.md names "its last five tools" in the evidence line text.
**Splits:** none. Three prose copies of "five".
**Ruling:** none.
**Open:** none.

### L-F01-18
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:39 `EMPTY_FLEET_SECONDS = 300`; :332 `now - spell < EMPTY_FLEET_SECONDS`. G\README.md:29 "no agent has written for 5 minutes" (value copy in prose).
**Splits:** none. 5 minutes here (fleet silence) is a different question from `STATUS_NAG_MINUTES = 5` (status age).
**Ruling:** none.
**Open:** none.

### L-F01-19
**Verdict:** CONFIRMED.
**Homes as found:** G\prompt.py:19-21 comment `(code.claude.com/docs/en/hooks, "capped at 10,000 characters")` and `FIELD_CAP = 10000`. G\README.md names `FIELD_CAP` ("Claude Code caps each field at `FIELD_CAP` characters"). G\tests\probe_results.md:127 "The excerpt is about 10,000 characters taken from the start and end of the first 30,000" and :223 "About 10,000 characters" (a different cap: the failed-tool excerpt).
**Splits:** none. The same number 10,000 means two caps (hook-context field; failed-tool excerpt).
**Ruling:** none.
**Open:** whether the cited Claude Code page still says 10,000 is an outside fact, not checked.

### L-F01-21
**Verdict:** CONFIRMED.
**Homes as found:** G\ruling_rules.py:98 comment `answer "Yes, no click" (2026-11-13, session ca29fda5)`, :99 `INSTALL_DATE = "2026-11-14"`, used at :335, :338, :347. H docs\agents\rulings\README.md:51 "The user approved it on 2026-11-13 ("Yes, no click")" and "(`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14)".
**Splits:** none in value. The date is written twice (code, README prose).
**Ruling:** "Yes, no click" appears in a code comment and in the rulings README; I did not find it as a line under `## Confirmed` (not grepped in the five ruling files beyond the README).
**Open:** none.

### L-F01-22
**Verdict:** CONFIRMED (copy homes read; header shape open).
**Homes as found:** SC\check.py:28-30 "The user raised it from the plan's 45 on 2026-10-14", `MAX_CARD_LINES = 60`, used :112-113. SC\writer-brief.md:71 "The whole card is at most 60 lines; aim for 20 to 40. A long session may use the room up to 60". SC\test_session_cards.py:322 `check.MAX_CARD_LINES` (derived).
**Splits:** none in the 60. writer-brief:71 also states the header shape (six `key: value` parts) in prose; I did not read check.py's header check, so that comparison is open.
**Ruling:** agents-and-workflow.md:15 under Confirmed, `**2026-10-14** "continue, and raise the cap to 60." (session 205c56ae)`. It names the cap value; no split involved.
**Open:** header shape vs check.py.

### L-F01-23
**Verdict:** CONFIRMED.
**Homes as found:** SC\prepass.py:31 `RUNNING_CUTOFF_SECONDS = 2 * 3600`, used :391; SC\prune_list.py:114 reads `prepass.RUNNING_CUTOFF_SECONDS`; SC\test_session_cards.py:1009 `OLD + 7200 - 1`, :1011 `OLD + 7200` (literal copy in a test). Different "2 hours": G\session_cards_start.ps1:43 `$ClaimHours = 2`.
**Splits:** none. The test holds the number as a literal, not read from the constant.
**Ruling:** none found (code comment only).
**Open:** none.

### L-F01-24
**Verdict:** CONFIRMED-CHANGED (more places than the row says).
**Homes as found:** SC\batch.py:24-26 `MAX_BATCH_BYTES`, `MAX_BATCH_SESSIONS`, `BATCHES_PER_WAVE = 10`; :65 deletes `wave-*-batch-*.txt` by glob; :70-71 writes `f"wave-{wave}-batch-{num}.txt"`. G\session_cards_start.ps1:261 reads with `-Filter 'wave-*-batch-*.txt'` and `-match '^wave-(\d+)-batch-(\d+)\.txt$'`, :262 sorts with two more regexes of the same shape. writer-brief.md:26 "Each line is `<session id> TAB <condensed file path> TAB <KB>`".
**Splits:** the file-name format is written in four places (batch.py f-string and glob, ps1 filter and regexes), none built from another. All agree today. I searched SC\test_session_cards.py for "wave-" and found no hit, so no test holds them equal.
**Ruling:** none ("user-approved with the plan" is a code comment).
**Open:** none.

### L-F01-25
**Verdict:** CONFIRMED.
**Homes as found:** SC\writer-brief.md:100 `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "HH:MM done <n> of <m> cards | next: <session>"`; G\progress.py:22 `STATUS_PREFIX`, :26-27 `_STATUS_CALL` accepts a bare or quoted id, then a quoted line with no `"`, `$`, backtick, CR/LF or curly quotes; :58 `_helper_line` builds the same shape. BP:15 and DOR:9/DR:9 carry the same call text. test_docs_numbers.py:182 checks only the preamble's copy against the prefix (by name; body not opened).
**Splits:** none. Writer-brief's example line fits the regex.
**Ruling:** none.
**Open:** none.

### L-F01-26
**Verdict:** CONFIRMED.
**Homes as found:** G\progress.py:26-27 the regex accepts any text between the quotes that avoids `"`, `$`, backtick, CR/LF and curly quotes, including an empty text; :38-48 `is_status_call` / `status_call_id` use only that match. BP:15-19 says what the line should say ("in plain words").
**Splits:** the call shape is checked by code; the content is a prose rule only. I searched progress.py only; stall.py adds an evidence line it builds itself and does not check the line's words (not read in full).
**Ruling:** the related Confirmed lines are agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable"; neither it nor any line I read sets what the text must say.
**Open:** none.
**Hook gap:** yes, G\progress.py (the status line's wording is not checked).

### L-F02-01
**Verdict:** CONFIRMED-CHANGED (words only; weaker than the row).
**Homes as found:** DOR:9 and DR:9: "Append one line to `C:\...\status\<your agent id>.md` every 5 minutes, form `HH:MM done ... | next: ...`. Use `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"` to write it, alone in its own call." BP:15 "Do not write the file with Edit, a heredoc or `>>`". progress.py:26-27 accepts only the helper shape.
**Splits:** DOR/DR say "Append one line to <file>" and in the next sentence name the helper, so they do not leave the reader without the helper. They lack BP:15's "do not write the file with Edit, a heredoc or `>>`". Situation: a reviewer reading only DOR/DR who appends the file directly: the hook then counts the call as an ordinary call and the status stays overdue (by reading progress.py:119-125 and the BP text).
**Ruling:** none.
**Open:** none.

### L-F02-02
**Verdict:** CONFIRMED (by reading).
**Homes as found:** G\tests\test_docs_numbers.py:22 `DEFAULT_HYDRA_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-test"`; :70 `return os.environ.get("GATES_HYDRA_ROOT") or DEFAULT_HYDRA_ROOT`; :6 docstring "from the GATES_HYDRA_ROOT environment variable, or from the Hydra main checkout when that is unset"; :144 `self.skipTest("no settings.json beside gates (a worktree); the live file is checked on main only")`.
**Splits:** a doc edit on a branch is read by this test only when the env var points at that worktree; otherwise the main checkout is read.
**Ruling:** none.
**Open:** a run with and without the variable.

### L-F02-03
**Verdict:** CONFIRMED.
**Homes as found:** H CLAUDE.md rule 4 and AR:10 (rule 4 line): "Agents never stall or go quiet for long. When they hit a problem, they fail loudly: stop and say what broke." Both read identical. BP:107-111 "When blocked: stop and report ... never retry the same call hoping it passes". AR:27 "a stall notice ... which helps with rule 4".
**Splits:** none. The "fail loudly" half has no hook I found (I read only AR:27, BP and G\README; I did not search stall.py or progress.py for any check that an agent reported a failure).
**Ruling:** agents-and-workflow.md Confirmed line 18: `**2026-10-04** "agents should fail loudly. freezing is unacceptable" (session a9a461da)`. It names the rule, so there is no split.
**Open:** none.

### L-F02-10
**Verdict:** CONFIRMED.
**Homes as found:** H tools\replay_schedule.cpp:36-40 comment "The user set it: question 3 of the optimal-score run plan, which has left the tree (removed; git show d7e47a20:docs/superpowers/plans/2026-11-06-optimal-score-run.md)", `constexpr double kSpEndLeadMs = 15.0;`; :70 exported as `sp_end_lead_ms`; used at :168, :188, :289, :930-992. DOR:108 "The schedule presses 15 ms before a Star Power end (`kSpEndLeadMs` in `tools/replay_schedule.cpp`), and no game rule sets that margin. If it had no ruling, you could not pick the number" (an example sentence with a value copy). Tests that hold the 15 as a literal: tools\ch_probe\tests\test_play_chart.py:1621, 1731, 1737; tools\ch_mcp\tests\test_game_tools.py:109-122, 134 (fixture strings with "15 ms"). tests\test_replay_schedule.cpp:721 not opened by me.
**Splits:** the code comment says the user set the number; DOR:108 says "no game rule sets that margin" and, in the conditional, treats a missing ruling as the case that holds the merge. A grep of all ruling files for "15 ms", "15ms", "kSpEnd", "end lead" and "before the end" found nothing, so no Confirmed line backs the 15. The only source for "the user set it" is a comment pointing at a removed plan.
**Ruling:** none under `## Confirmed`.
**Open:** whether the removed plan (commit d7e47a20) holds the user's words was not checked; no game rule for the margin was checked against docs\clone-hero\.

---
Not reached: none. All 20 ids were read.
