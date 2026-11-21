# TOOL-AUDIT-T9-K7 closure check (18 ids)

Line numbers in the audit rows have drifted in places; I confirmed each split by the code or text itself. Hydra is C:\Users\Patrick\Downloads\Hydra\hydra-test, hooks are C:\Users\Patrick\.claude.

### L-F25-04
**Verdict**: REOPEN
**Homes read**: docs/agents/premise-map.md:45,52 (a card counts only as the place a ruling's quoted words sit; handoff, plan, card, memory note are not a source for a world fact); CLAUDE.md and agent-rules.md rule 15 (a rule copied from a tool, test or comment is never a source); agent-rules.md:39 (memory note, handoff, "as ruled" never the user's words).
**Why**: The split is "what counts as a source of a fact". Theme 10's answer, "Confirmed only", is about what counts as the user's words. It covers premise-map:45 (card quote) but not the two lists of non-sources (premise-map:52 against rule 15), which are doc against doc. It should be "Merge, I pick" (or exit 4 if the lists answer different questions).

### L-F25-12
**Verdict**: REOPEN
**Homes read**: tools/ch_mcp/server.py:450-451 and docs/agents/clone-hero-mcp.md (only "positive early, negative late"); tools/ch_probe/experiments/edge_probe.py:22 (same sign rule), :173 and :198 (`'early' if lead_ms >= 0`, so zero prints as early).
**Why**: Not two answers. The docs are silent on zero and one printed label calls it early. "Merge, I pick" needs two answers to one question. Better: write the zero case into the docs, or ask the user if zero matters in the game.

### L-F26-03
**Verdict**: PASS
**Homes read**: docs/agents/premise-map.md:45 (a session card's quoted words count as a ruling); doc-review.md:63 and derive-once-review.md:45-52 (accept a card quote or a Confirmed line); CLAUDE.md "Rulings and this machine" and rulings/README.md:16 (only ## Confirmed lines are the user's words).
**Why**: The split is which records count as the user's words. "Confirmed only" decides it. The pages still say otherwise, so the fix is still needed and the closure holds.

### L-F26-09
**Verdict**: PASS
**Homes read**: gates/progress.py:77-79 (any call clears `parked`); gates/stall.py:252-254 (parked also needs `last` not after `since`).
**Why**: Both code, one question (is this agent parked). Merge fits.

### L-F27-01
**Verdict**: PASS
**Homes read**: same as L-F26-09.
**Why**: Same split, same kind.

### L-F27-02
**Verdict**: REOPEN
**Homes read**: gates/review_rules.py:65-74 (`normalized` strips CRLF only; `doc_key` is the one hash); gates/publish_gate.py:15-30 (`is_small_text` reads UTF-16, strips lone CR) and :100 (the key still comes from `review_rules.doc_key`).
**Why**: The key already has one owner and publish_gate uses it. The extra handling is for counting size, a different question. So exit 4, not "Merge, I pick".

### L-F27-03
**Verdict**: PASS
**Homes read**: gates/progress.py:19 (`REPORT_TOOLS` has SubagentHandback and StructuredOutput); gates/percall.py:135 (saves the message only for SubagentHandback), :145 and :207.
**Why**: Two code paths answer "what counts as the hand-back" differently.

### L-F27-04
**Verdict**: PASS
**Homes read**: tools/check_pointers.py:38 (`[0-9a-fA-F]{7,40}`, backticks needed); tests/test_docs_match_code.cpp:1354 (`[0-9a-f]{7,40}:`) and :1342-1346 ("never committed").
**Why**: Python code against C++ code on one question. Kind fits.

### L-F27-05
**Verdict**: PASS
**Homes read**: tools/check_handoff.py:28 (upper or lower, 40); tools/doc_tokens.py:39 (either case), :50 (lowercase); gates/review_rules.py:27-29 (waiver 7-64 for doc-review, 7-40 others), :56 (lowercase).
**Why**: All code, one question (hash shape). Kind fits.

### L-F27-06
**Verdict**: PASS
**Homes read**: tools/ch_mcp/game_files.py:276-294 ("Keys before any section header count too"; first key wins, :297-307); game_files.py:324-325 says Hydra's `read_song_ini_keys` (src/app/analysis.cpp:128) takes the last.
**Why**: Both code, still different. I did not re-check the pro_drums value difference.

### L-F27-09
**Verdict**: PASS
**Homes read**: gates/review_dispatch.py:203 (`prompt_sha256` of the tool prompt); gates/agent_stop.py:29-39 (`first_user_message`) and :315 (its hash).
**Why**: Both code, two routes to one brief hash. Kind fits.

### L-F27-12
**Verdict**: PASS
**Homes read**: gates/review_rules.py:89-101 (`human_prompt_text`); gates/ruling_rules.py:27,230-233 (`prepass.condense`); gates/agent_stop.py:29 (`first_user_message`).
**Why**: Three code paths each find "the user's words" in a transcript.

### L-F27-15
**Verdict**: PASS
**Homes read**: tools/check_pointers.py:6-14,190-202 (any-file pointers, "retired"); tests/test_docs_match_code.cpp:1332-1346 (docs/ paths only, exact).
**Why**: Two code checkers, one question. Still split.

### L-F27-18
**Verdict**: REOPEN
**Homes read**: docs/agents/ui-testing.md:233-251 (`commands` includes screenshots and timeout lines); gates/review_rules.py:393-412 (`n < 1` rejects; docstring says the record "shows a window was used ... not what anyone saw").
**Why**: "Give each a home" is for a rule that lives only in prose. Here a hook decides it and the doc describes the same counting. It is doc against code, or exit 4, not a homeless rule.

### L-F27-22
**Verdict**: PASS
**Homes read**: tools/precheck/decisions.cpp:44-100 (separators `,` `'` space `_`); tests/test_docs_match_code.cpp:375 (`(\d+(?:\.\d+)?)`, reads 1,000,000 as three numbers).
**Why**: Both C++ code, one question. I did not re-check the f/u suffix.

### L-F28-02
**Verdict**: PASS
**Homes read**: C:\Users\Patrick\.claude\model-postures.md:11 ("nudge it once ... then TaskStop"); gates/stall.py:125,144 (printed: stop with TaskStop and re-dispatch) and :474 ("nudge it or stop it").
**Why**: Both docs (printed hook text enforces nothing). Same as L-F06-16. The hook's own texts also differ from each other.

### L-F28-09
**Verdict**: REOPEN
**Homes read**: CLAUDE.md "Past sessions" (a [write-back] card: the next main session adds the fact); tools/session_cards/check.py:39-49,196 and index.py:51 (the index mark); gates/session_cards_start.ps1 (searched for "write", no write-back text).
**Why**: The rule already has a doc home (CLAUDE.md) and an index mark, so it is not prose-only. What is missing is a hook reminder, which is "instruction only", not "give it a home". Under the brief's definition the closure does not fit.

### L-F28-17
**Verdict**: PASS
**Homes read**: docs/agents/agent-rules.md:21,23 (rule 15 quotes "add a rule so sessions find the existing owner before writing one", "the correct rule is to use what hydra already knows...", "Yes, add it", "Yes, write them"); docs/agents/rulings/questions-and-audits.md (## Confirmed at line 7, ## Unconfirmed at 24) and agents-and-workflow.md; docs/sessions/2026-11-14-7d28ae9a.md:21 and 2026-11-15-99e8481e.md:22-28.
**Why**: I grepped rulings and sessions for each quote. They sit only in cards, not under ## Confirmed. "Drop the claim" holds.

## Ids not reached
None.
