# TOOL-AUDIT-T7-C15 closure check (sorting S08, ledger L08, batch item 14)

Short paths: G = C:\Users\Patrick\.claude\gates; SC = C:\Users\Patrick\.claude\tools\session_cards; WB = SC\writer-brief.md; HR = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout, read now).
Totals: PASS 23, REOPEN 2 (L-F22-04, L-F22-09), GONE 0. Ids not reached: none.

### L-F22-01
**Verdict:** PASS (P)
**Homes read:** G\progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`; progress.py:58 form `HH:MM done ... | next: ...`; HR docs\agents\brief-preamble.md:13 and :21; HR docs\agents\derive-once-review.md:9 and doc-review.md:9; HR CLAUDE.md:69; agent-rules.md:13; G\README.md:124; WB:100; model-postures.md.
**Why:** Every home says every 5 minutes and the same `HH:MM done ... | next: ...` shape. WB:100 adds "<n> of <m> cards" and an unquoted agent id; it is a worked example of the same form, not a different rule. No home contradicts another.

### L-F22-03
**Verdict:** PASS (P)
**Homes read:** G\review_rules.py:48-49 (`REALAPP_FILE = "realapp.jsonl"`, comment says hydra_ui_mcp.py owns it); HR tools\hydra_ui_mcp.py:81 (same value), :197; G\README.md:55; HR docs\agents\ui-testing.md:237.
**Why:** Same name in every home. Only a pointer is missing in the code constant.

### L-F22-04
**Verdict:** REOPEN (P fails; the row is a claim against code, not agreement)
**Homes read:** G\lib\hook_input.ps1:1 "the one place a hook reads its input"; used by G\cleanup_sweep.ps1:698 and session_cards_start.ps1:299; G\common.py:59 (`sys.stdin.buffer` ... decode utf-8 in `run`); G\wake.py:92-96 (its own `sys.stdin.buffer` read and utf-8 decode); G\ref_judge.py:510 (reads git ref lines from stdin, different input); G\publish_gate.py:3, 111.
**Why:** The comment says one place for every hook, but common.run and wake.py each read and decode stdin themselves. That is a comment against code, so it fits a "comment against code" category, not P (P needs no disagreement). Behaviour is the same today (both decode UTF-8). The only extra is the byte-order-mark trim in the PowerShell reader. Main session should pick the category; I suggest A if the user's "The code wins" answer covers a comment line.

### L-F22-05
**Verdict:** PASS (P)
**Homes read:** G\common.py:22-32 (deny, context, system_message); G\percall.py:128, :139, :168 (ask built from common.deny), :192 (updatedInput); G\shell_output.py:197 (updatedToolOutput); G\prompt.py:45-52 (reads the shapes); G\cleanup_sweep.ps1:703,741 and session_cards_start.ps1:363 (hand-built systemMessage and hookSpecificOutput); G\README.md:93.
**Why:** The hand-built shapes use the same keys as common.py (`hookSpecificOutput`, `hookEventName`, `additionalContext`, `systemMessage`). The extras (updatedInput, updatedToolOutput) are separate fields no other home defines differently.

### L-F22-06
**Verdict:** PASS (P)
**Homes read:** G\stall.py:34 `TOOL_TAIL = 5`, :207; model-postures.md:11 "its last five tools"; HR brief-preamble.md:23 "last five tool calls"; memory relay-status-in-plain-english.md:15 "last five tools".
**Why:** All say five. Pointer-only change.

### L-F22-07
**Verdict:** PASS (P)
**Homes read:** G\waiver.py:20-45 (allow records: kind, prefix, prompt_id, transcript_path, session_id, time); G\percall.py:25-40 (kind+"-used" records: prompt_id, time, claim, read only by percall's own matching); G\review_rules.py:118-131 (reads by kind); G\common.py:251-258 (append_waiver, read_waivers, no field list).
**Why:** Each reader filters on its own kind; no reader takes the other shape as its own. No field list exists, none disagrees.

### L-F22-09
**Verdict:** REOPEN (P fails; one hand-typed claim is false today)
**Homes read:** WB:77 "Use 2 to 6" tags (check.py:136-138 checks membership only); WB:71 "aim for 20 to 40" (check.py:30 `MAX_CARD_LINES = 60`); WB:78 "D115 is a name with no entry, so never cite it"; HR docs\audit\2026-10-03-fix-decisions.md:704 `**D115 (one record per chart and profile), 2026-10-11 to 2026-10-15.**`; SC\common.py:92 `DECISION_ENTRY = ^\*\*D(\d+)\b` (matches that line); SC\check.py:170-173.
**Why:** The decision log now has a D115 entry, and the check derives that from the log, so it accepts a card citing D115. WB:78 still tells the writer never to cite it. That is a brief against data the code reads, so it is a split, not agreement. The tag-count and "0 to 8" parts are brief-only guidance with no disagreement, but the D115 sentence breaks P. Suggest A (brief against what check.py derives), or C if the user treats it as a stale name.

### L-F22-10
**Verdict:** PASS (P)
**Homes read:** WB:11 "A card holds no secret, token, password or key"; SC\common.py:81-91 SECRET and SECRET_MASK, :307-316 mask_secrets; SC\check.py (searched "secret": no hit).
**Why:** The brief gives no list; the code masks five token shapes in evidence. Nothing says the check covers the finished card, so nothing contradicts anything.

### L-F22-12
**Verdict:** PASS (A)
**Homes read:** WB:75 "The part in parentheses after a quote may not contain `(` or `)`."; SC\common.py:94-98 RULING_NOTE (allows one inner parenthesis level; comment says deeper does not match); SC\check.py:151 (uses QUOTE/CHOSE from common).
**Why:** A brief line against checker code. The test that pins the looser reading is code too. No other doc is a side.

### L-F22-14
**Verdict:** PASS (B)
**Homes read:** memory every-brief-carries-status-rule.md line 3 (description: "the dispatch gate now denies briefs that lack it") and line 15 ("No gate checks a brief for this sentence today"); C:\Users\Patrick\.claude\settings.json:167-181 (the Agent matcher runs only review_dispatch.py); G\review_dispatch.py (grep "status": no hit); G\percall.py:146 (status check applies to shell calls, not briefs); G\prompt.py, G\posture.py (no hit).
**Why:** The only split is a memory note against code and against itself. Code has no brief-status gate.

### L-F22-15
**Verdict:** PASS (P)
**Homes read:** G\common.py:36-49 (log_refusal: eight fields, `reason[:300]`); G\README.md:79,81; G\tests\test_refusals.py:16 FIELDS (same eight); G\progress.py:31 QUOTE_CHARS = 300 (different question); G\publish_gate.py:130 and ref_judge.py:528 call log_refusal with event "publish", "push", "ref".
**Why:** The eight fields, the cut and the event names agree across code, README and test.

### L-F22-16
**Verdict:** PASS (A)
**Homes read:** G\README.md:75 "`REVIEW_KINDS` in `common.py` is the one place the review record fields are checked"; G\common.py:11-13 REVIEW_KINDS (verdict lists key, verdict, reviewer, session, time), :240-245 append_review; G\agent_stop.py:258-262 (verdict also carries `path` or `handover`); G\review_rules.py:209, :278 (reads `handover` and `path`).
**Why:** A README sentence against code that writes and reads fields REVIEW_KINDS does not list. Both other sides are code.

### L-F22-17
**Verdict:** PASS (P)
**Homes read:** G\common.py:136-138 write_agent ("no lock ... a hook that changes a record calls update_agent"), :189-206 update_agent; callers G\progress.py:103, G\percall.py:115, G\agent_stop.py:304; G\stall.py:75-77, :157, :238; G\README.md:75 (path and lock file, no field list).
**Why:** No file lists the agent record fields; every writer goes through update_agent; the README says the same. Nothing disagrees.

### L-F22-18
**Verdict:** PASS (P)
**Homes read:** G\stall.py:38 USAGE_FIELDS (input_tokens, cache_read_input_tokens, cache_creation_input_tokens), :366; HR tools\agent_cost.py:62-64 reads the same three.
**Why:** Same three fields, no disagreement.

### L-F22-19
**Verdict:** PASS (P)
**Homes read:** G\stall.py:77, :105, :134, :479 (`rec.get("finished")`); G\wake.py:43 (uses stall.running_agents); G\agent_stop.py:3-5, :315, :319 (sets finished).
**Why:** All four tests are the same expression and agree with the writer.

### L-F22-20
**Verdict:** PASS (P)
**Homes read:** G\shell_output.py:15 `CAP = 4000`, :30-32 MARKER_START, FAILED_LINE, TRIMMED_LINE; HR brief-preamble.md:35 (quotes "COMMAND FAILED, exit code N", 4,000, 30,000, "never end one with your own `exit N`"); G\README.md:25.
**Why:** 4,000 and the line texts agree; 30,000 is Claude Code's own cap stated only in the preamble. No home contradicts another.

### L-F22-21
**Verdict:** PASS (P)
**Homes read:** G\progress.py:14-16; model-postures.md:5 (100 calls, 20 minutes); G\README.md:29 (5 minutes, matches stall.py:39 EMPTY_FLEET_SECONDS = 300), :124 (5 and 10); WB:100-101 (5 minutes; 20 minutes or 100 calls); agents\reviewer.md:11; HR CLAUDE.md:69; agent-rules.md:13; brief-preamble.md:21,73; derive-once-review.md:9,90,110; doc-review.md:9,91,100.
**Why:** Every page that states a number agrees with progress.py (100/150 calls, 20/30 minutes, 5/10 status). The test covers fewer pages than state them, but none is wrong today.

### L-F22-23
**Verdict:** PASS (P)
**Homes read:** SC\check.py:32 ROADBLOCK_ENDINGS, :185-191 (`len(parts) < 3` on " -> "); WB:48-50 and WB:80 ("at least two ` -> ` separators").
**Why:** Two separators and three parts are the same rule.

### L-F23-01
**Verdict:** PASS (P)
**Homes read:** HR CLAUDE.md:25; SC\common.py:36-42 CARDS_ROOT, INDEX_FILE; G\session_cards_start.ps1:274 `docs/sessions`; HR tests\test_docs_match_code.cpp:1352; HR docs\agents\audit-method.md:126.
**Why:** The path agrees in all. audit-method.md:126 words it "not the full question or option text" against CLAUDE.md's "not the full question text"; the facts are compatible, so no split.

### L-F23-02
**Verdict:** PASS (P)
**Homes read:** SC\common.py:51-57 HEADINGS and the old "Rulings" name; WB:71; SC\check.py:118-120 (six keys), :294 (checked/failed); SC\index.py:41,50; link_memory.py:41; number_rulings.py:41,70; prune_list.py:94; prepass.py:400,422; batch.py:49.
**Why:** Headings, keys and status words agree across all files read.

### L-F23-03
**Verdict:** PASS (P)
**Homes read:** SC\check.py:40-43 WRITE_BACK_PAGES "docs/clone-hero/", WRITE_BACK_TAG "clone-hero"; WB:64, :81, :88; HR CLAUDE.md:29; HR docs\clone-hero\README.md:80; SC\index.py:51.
**Why:** Prefix, tag and the [write-back] mark agree.

### L-F23-04
**Verdict:** PASS (P)
**Homes read:** SC\check.py:150-157 (split on "..."); G\ruling_rules.py:285-291 (same split, CHOSE imported from the card tools); WB:76.
**Why:** Same fragment rule in both code places and the brief.

### L-F23-05
**Verdict:** PASS (P)
**Homes read:** SC\common.py:298-299 card_name; SC\check.py:38 CARD_FILE, :88-100 (agents field rewritten from meta); WB:31 (file name `<start date>-<first 8 chars>.md`), WB:39 header form.
**Why:** The name shape agrees between writer, parser and brief.

### L-F23-06
**Verdict:** PASS (A)
**Homes read:** WB:29 and WB:63 (names "from MEMORY.md"); WB:87 (any file directly in the memory folder, listed or not); SC\link_memory.py:55-59, :70-71 (folder listing; a name matches only if the file exists).
**Why:** WB:29 and :63 disagree with the code. WB:87 already agrees with the code. The real split is brief lines against code. Note for the main session: WB:63 and WB:87 also disagree with each other, so the fix must change :29 and :63, not :87.

### L-F23-07
**Verdict:** PASS (A)
**Homes read:** WB:79 "Never write any other 7-to-40-character hex string in Roadblocks or Outcome"; SC\common.py:78 HEX (needs a digit, 7 to 40); SC\check.py:174-181 (also needs a letter a-f and skips session-id pieces).
**Why:** A brief line wider than the checker. Both sides are a brief and code only.

## Ids not reached
None.
