# TOOL-AUDIT-T5-L08 ledger (finders F22, F23, F24; verifiers V22, V23, V24; no re-send pairs existed)

Short paths: G = C:\Users\Patrick\.claude\gates; SC = C:\Users\Patrick\.claude\tools\session_cards; HR = Hydra repo; WB = SC\writer-brief.md; DOR = HR docs\agents\derive-once-review.md; DR = doc-review.md; PM = premise-map.md; AR = agent-rules.md; BP = brief-preamble.md; AM = audit-method.md; OM = owner-map.md; TR = test-rubric.md; MP = C:\Users\Patrick\.claude\model-postures.md.
I compiled from the six reports only. Where a verifier and a finder disagree, the verifier's reading is used and said so. Inventory row ids are the finders' own (I1-..., I4-... and so on).
No re-send reports (Fxxb, Vxxb) exist for F22, F23 or F24.

Rulings the reports quote (the only `## Confirmed` lines named; none names both sides of any split below):
- agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (F22, F24).
- agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english" (F22).
- agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (F22).
- agents-and-workflow.md:14 "the important part is more organizing for claude so it can easily reference past sessions and see what topics were covered, what roadblocks were encoutnered, and what the user ruled." (F23, V23).
- review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (F24, V24).
- review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time ... hard cap of 30 minutes real time." (F22; not a row).
- review-and-hooks.md (2026-10-31) "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized" (F22; general).
F22 says it did not search every rulings topic file for the 2026-10-31 card-content ruling. V22 did not open the rulings files. V23 and V24 name only the lines above. questions-and-audits.md:28 is under `## Unconfirmed` (F24; V24 did not reopen it).

---------------------------------------------------------------
## Rows from F22

### L-F22-01: What form does a status line take, and how often must it be written?
- **Source:** I1-021, I1-093 (F22); V22 item 1.
- **Kind:** K1 (K3-light).
- **Verdict:** CONFIRMED (V22 item 1).
- **Homes:** G\progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`; progress.py:58 `'%s "%s" "HH:MM done ... | next: ..."'`. HR docs\agents\brief-preamble.md:13 "every 5 minutes. The form is `HH:MM done ... | next: ...`." DOR:9 "every 5 minutes, form `HH:MM done ... | next: ...`". WB:100 "One status line every 5 minutes ... "HH:MM done <n> of <m> cards | next: <session>"" (unquoted agent id; progress.py quotes it; both pass progress.is_status_call). Interval only: HR CLAUDE.md:69 (rule 7), AR:13, G\README.md:124, memory every-brief-carries-status-rule.md:15, memory relay-status-in-plain-english.md. G\status_append.ps1 default line `HH:mm (no message)`.
- **Splits:** None. Edge noted: card writers skip the preamble (agents\card-writer.md:9) and see only WB's form; WB's form carries card counts, the preamble's does not.
- **Ruling quotes:** agents-and-workflow.md:25 and :26 (above); neither names a split.
- **Tests:** G\tests\test_docs_numbers.py:116-120 (interval over DOCS), :122-127, :182-190. Not covered (F22): the form text, WB, G\README.md, MP, memory notes.
- **Open:** none (V22: nothing needs a run).
- **Hook gap:** none for this row (see L-F22-22).

### L-F22-02: What counts as a valid commit hash (length, case, shortest allowed)?
- **Source:** I3-021, I3-034 (F22); I4-029, I4-052 (F23); I6-093 and side note S3 (F24); V22 item 2, V24 S3.
- **Kind:** K6, K2.
- **Verdict:** CONFIRMED (V22 item 2; V24 S3 confirms the check_handoff, doc_tokens and review_rules lengths).
- **Homes:** G\ref_judge.py:195 `len(fields[0]) not in (40, 64)`, lowercase only. G\review_rules.py:56 `^[0-9a-f]{40}([0-9a-f]{24})?$`. G\review_dispatch.py:19 `[0-9A-Fa-f]{40}` only (V22; V24 did not reopen). G\review_rules.py:26-28 waiver phrases: derive-once and real-app `{7,40}`, doc-review `{7,64}`. SC\common.py:78-79 `{7,40}` (candidate hash in free text). HR tools\check_handoff.py:28 `{40}` (exactly 40, V24); tools\doc_tokens.py:39,50 `{7,40}` (V24); check_pointers.py:38, ci_build_folder.ps1:99, effort_test_faults.py:37 (finder only). Prose: G\README.md:39, DOR:17,200,202, handoff.md:19,35 "40-character".
- **Splits:**
  - (1) A 64-character key is accepted by ref_judge and review_rules but a reviewer brief naming one is not found by review_dispatch's 40-hex pattern.
  - (2) The doc-review waiver takes up to 64 characters, the other two waivers up to 40.
  - (3) Case: dispatch and the waiver phrases take upper case; judge and rules take lower case only (the case point was added by V22).
- **Ruling quotes:** None found (F22: no Confirmed line speaks to hash length).
- **Tests:** none found.
- **Open:** none stated. F23 notes the different digits answer different questions (candidate in free text, waiver prefix, full object name) and rates it "K1 weak"; it is the same cluster.
- **Hook gap:** none stated.

### L-F22-03: Which file name and fields does the real-app record use, and who owns them?
- **Source:** I3-031 (F22); V22 item 3.
- **Kind:** K1, K8-light.
- **Verdict:** CONFIRMED (V22 item 3: both constants read, no import).
- **Homes:** G\review_rules.py:48-49 `REALAPP_FILE = "realapp.jsonl"` with the comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them." HR tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"`, :182-198 realapp_record (kind, commit, dirty, commands, shots, started, stopped), comment :78-80 "The hooks repo's ref gate reads it, so its field names are a contract". Prose: G\README.md:55, HR docs\agents\ui-testing.md:237 (finder only).
- **Splits:** None (same name in two constants).
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:54 README_NAMES requires the README to name realapp.jsonl. No test holds the two constants equal.
- **Open:** none.
- **Hook gap:** none.

### L-F22-04: Is Read-HookInput really "the one place a hook reads its input"?
- **Source:** I1-096 (F22); V22 item 4.
- **Kind:** K8, K6.
- **Verdict:** CONFIRMED (V22 item 4).
- **Homes:** G\lib\hook_input.ps1:1 "the one place a hook reads its input" (used by cleanup_sweep.ps1:698, session_cards_start.ps1:299). Python hooks read stdin themselves: common.py:58-62 (`sys.stdin.buffer`, utf-8; V22 confirmed); wake.py:92-96, ref_judge.py:398 (`decode("utf-8","replace")`), publish_gate.py (finder only). Only the PowerShell reader trims U+FEFF (finder only).
- **Splits:** None named.
- **Ruling quotes:** None found.
- **Tests:** none compare them.
- **Open:** none.
- **Hook gap:** none.

### L-F22-05: Who builds the hook output shapes (deny, context, system message)?
- **Source:** I1-059 (F22); V22 item 5.
- **Kind:** K6.
- **Verdict:** CONFIRMED (V22 item 5), except percall.py's "ask" and updatedInput, not reopened by V22.
- **Homes:** G\common.py:21-32 (owner of the Python shape). By hand: session_cards_start.ps1:303,362 and cleanup_sweep.ps1:703,741 build `systemMessage` JSON (V22). Finder only: session_cards_start.ps1:362-363 `hookSpecificOutput` with `hookEventName='SessionStart'`; percall.py:134 builds "ask" from deny; percall.py:158 sets `updatedInput` by hand; prompt.py:60-62. G\README.md:93 describes them.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none holds the PowerShell shape equal to common.py.
- **Open:** percall.py "ask"/updatedInput not reached by the verifier.
- **Hook gap:** none.

### L-F22-06: How many recent tool calls does the stall evidence line show, and where is that number typed?
- **Source:** I2-012 side, I2-025, I2-030 (F22); I5-069 (F24); V22 item 6; V24 Group G.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED (V22 item 6: a fourth home found, the preamble).
- **Homes:** G\stall.py:34 `TOOL_TAIL = 5` (owner), used :207. MP:11 "its last five tools" (no pointer). Memory relay-status-in-plain-english.md:15 "(call counts, last five tools, new commits)". Found by V22, missed by F22: HR docs\agents\brief-preamble.md:23 "last five tool calls ... (five is TOOL_TAIL in stall.py)" (names the owner). G\README.md:29,31,41 describe the line without the number. The parked-line wording " | parked since %H:%M, waiting on its own background work" is only in stall.py (finder).
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:26 (plain-English relay), not about the number.
- **Tests:** test_stall.py exercises the line; nothing compares the docs' "five" (F22, F24 side; V24 did not open test_docs_numbers.py).
- **Open:** none.
- **Hook gap:** none.

### L-F22-07: Does the waiver file have two record shapes with no field list?
- **Source:** I2-083 (F22); V22 item 7.
- **Kind:** K6-light, K1.
- **Verdict:** NOT VERIFIED (V22 checked only waiver.py:22-45; percall's "-used" line not re-read).
- **Homes:** finder only: G\waiver.py:43-44 (allow lines: kind, prefix, prompt_id, transcript_path, session_id, time); G\percall.py:25-40 (second shape `{"kind": kind+"-used", "prompt_id", "time", "claim"}`); G\review_rules.py:127; G\common.py:250-255 (file name, no field list, unlike REVIEW_KINDS).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_common.py:241-248 (finder).
- **Open:** the "-used" shape and percall reads were not reached.
- **Hook gap:** none stated.

### L-F22-08: What tail may follow a quote in a ruling line, in the rulings files and on a card?
- **Source:** I3-070, I3-071, I3-074 (F22); V22 item 8.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED (V22 item 8).
- **Homes:** G\ruling_rules.py:105 SESSION_NOTE needs "session <8 hex or uuid>" (V22); docstring :5-8 "LINE, SESSION_NOTE and parse() are the one home of that shape; the quote and chose parts are the session-card tools' QUOTE and CHOSE". SC\common.py:92-100 RULING, RULING_NOTE (accepts any parenthesised note with one level of inner parentheses), QUOTE, CHOSE: "Every script that reads rulings uses these, so the shapes are defined here only." HR docs\agents\rulings\README.md:16-30,53 (prose and example). "Only a Confirmed line is a ruling" restated in AR:35,37, HR CLAUDE.md:41, DR:63, PM:45, WB:67, SC\common.py:52-53 (finder).
- **Splits:** The tail after the quote: ruling_rules accepts only `(session <id>)`; RULING_NOTE accepts any one-level-parenthesised note. They differ for a card line with a note that is not a session id.
- **Ruling quotes:** None found for this shape.
- **Tests:** test_ruling_rules.py; test_docs_numbers.py:129-137. No test compares README prose to LINE.
- **Open:** none.
- **Hook gap:** none.

### L-F22-09: Brief rules that nothing checks: tag count (2 to 6), "0 to 8" decision lines, the hand-typed D115 list
- **Source:** I4-027, I4-028 (F22); I4-033, I4-089 (F23); V22 item 9, V23 candidate 6.
- **Kind:** K7, K1/K3.
- **Verdict:** CONFIRMED (tag count and D115 by V22; tag count and "0 to 8" by V23: check.py counts neither).
- **Homes:** WB:77 "Use 2 to 6" tags; SC\check.py:136-138 checks only membership in tags.txt (built by common.build_tags, common.py:554-568, from CONTEXT.md plus hand_tags.txt). WB:85 "Most sessions have 0 to 8 such lines" (one home, no check). WB:71 "aim for 20 to 40" lines (finder; the 60 cap is checked, check.py:30 `MAX_CARD_LINES = 60`). WB:78 hand-typed "D115 is a name with no entry"; check.py:139-173 already fails any D-number with no `**D<n>` entry (DECISION_ENTRY, common.py:92); the brief's list is a second copy of what the check derives (V22 read WB:78).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none (test_session_cards.py:198-205 counts 15 hand tags, F23).
- **Open:** none.
- **Hook gap:** none.

### L-F22-10: Is the "a card holds no secret" rule backed by a check on the card?
- **Source:** I4-003 (F22); I4-057, I4-093, I4-110 (F23); V22 item 9 (secrets part), V23 candidate 6 (secret part).
- **Kind:** K7, K6-light.
- **Verdict:** CONFIRMED in the part V22 read: WB:11 "secret, token, password or key", and check.py has no SECRET hit. V23: masking is applied to evidence in prepass, not to the finished card; "not searched further".
- **Homes:** WB:11 "A card holds no secret, token, password or key". SC\common.py:85-91 SECRET (five token shapes) and SECRET_MASK, applied at common.py:308 and in prepass (finder). Test: test_session_cards.py:161-196 builds seven token samples. No hook masks a different list (F23 searched).
- **Splits:** None. The brief's prose has no list; the regex has five shapes.
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:161-196 (samples of the code's shapes).
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-11: Are the four content kinds the user ruled on (2026-10-31) held anywhere but the brief?
- **Source:** I4-006 to I4-009 (F22).
- **Kind:** K7, K4 (I4-006).
- **Verdict:** NOT VERIFIED (V22: "money/security/game-audio wording not reopened").
- **Homes:** finder only: WB:18-21 cite "The user ruled on four kinds of content on 2026-10-31"; SC\common.py:266 comment; common.py:274 LOCAL_FIELD and check.py LOCAL_VALUES enforce the `local:` value. Money/token totals, security wording and game-audio wording: brief only, nothing checks them. For I4-006 the writer is asked to decide whether a session is about the project when unsure.
- **Splits:** None.
- **Ruling quotes:** F22 did not confirm a Confirmed line for the 2026-10-31 ruling (only two topic files read).
- **Tests:** none named.
- **Open:** the ruling's own file and line were not found.
- **Hook gap:** none.

### L-F22-12: May the text in parentheses after a quote contain parentheses?
- **Source:** I4-021 (F22); I4-051 (F23); V22 item 10, V23 S3.
- **Kind:** K2.
- **Verdict:** CONFIRMED (V22 item 10, V23 S3).
- **Homes:** WB:75 "The part in parentheses after a quote may not contain `(` or `)`." SC\common.py:96-98 RULING_NOTE allows one inner level; its comment says "a note nested deeper does not match". Test: test_session_cards.py:439-450 pins that one inner level passes (finder).
- **Splits:** Brief forbids; the checker accepts one inner level. They differ for a note like "(one foo() call)".
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:439-450 pins the looser reading.
- **Open:** none.
- **Hook gap:** none.

### L-F22-13: Who decides which paths count as code for the derive-once gate in Hydra's marker?
- **Source:** I3-035 (F22); V22 item 11.
- **Kind:** K8, K1/K7-light.
- **Verdict:** CONFIRMED (V22 item 11: the marker's claim read; hooks folder holds only `state`; searches found nothing). V22 did not re-read review_rules.py BUILTIN_CODE.
- **Homes:** HR .derive-once-gate: "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file." C:\Users\Patrick\.claude\hooks holds only `state`; no hooks\lib. Finder only: G\review_rules.py:21-22 BUILTIN_CODE; :58-59,157-177 `gate:` grammar; G\.derive-once-gate; G\README.md:55,138; DOR.
- **Splits:** The Hydra marker names a PowerShell function that does not exist; the finder reads review_rules.py (BUILTIN_CODE) as the real decider (finder's reading, unconfirmed by V22).
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:172 covers the gates README only, not this file.
- **Open:** whether BUILTIN_CODE is the real decider.
- **Hook gap:** none stated.

### L-F22-14: Does a gate deny briefs that lack the status sentence?
- **Source:** I1-021 split (F22); side note S2 (F24); V22 item 12, V24 S2.
- **Kind:** K8.
- **Verdict:** CONFIRMED by V22 (item 12: both lines read; the gate files not re-searched). V24 reads it as CONFIRMED-CHANGED: its grep found only note line 15 and could not confirm the finder's quoted body sentence; a gate for the status sentence is unconfirmed. V22 is the verifier for F22 and wins for F22's claim; the V24 doubt is recorded.
- **Homes:** memory every-brief-carries-status-rule.md line 3 "the dispatch gate now denies briefs that lack it"; line 15 "No gate checks a brief for this sentence today" (V22). G\README.md:39 names review_dispatch.py for reviewer briefs only. F22 found no code in review_dispatch.py, percall.py, prompt.py or settings.fragment.json that checks a brief for the status rule. A memory note is not the user's words.
- **Splits:** Note line 3 vs note line 15 (inside one memory note): a gate that denies briefs vs no gate checks a brief.
- **Ruling quotes:** agents-and-workflow.md:25 covers the status rule, not this split.
- **Tests:** none.
- **Open:** whether any gate checks a brief for the sentence (V22 did not re-search the gates).
- **Hook gap:** none stated.

### L-F22-15: Refusal log: file name, eight fields, 300-character reason, event names
- **Source:** I1-051, I1-058, I1-022, I2-054, I3-022 (F22); I5-179 (F24); V24 Group G (eight fields).
- **Kind:** K1.
- **Verdict:** CONFIRMED for the eight fields and the cut (V24: README:81, common.py log_refusal, test_refusals.py FIELDS). NOT VERIFIED by V22 (not in V22's list) for the file-name copies, the 300 in two modules and the event names.
- **Homes:** G\common.py:35-49 (owner; `reason[:300]`). G\README.md:79,81,83,88. G\progress.py:31 `QUOTE_CHARS = 300` (a different question, same number; finder). cleanup_sweep.ps1:55,549,584 (30-day prune, file name restated; finder). bash_windows_rule.py:209 comment (finder). Event names "push" and "ref" (ref_judge.py:416-418) and "publish" (publish_gate.py:130-131), finder only. Run log agent_stop.jsonl name in agent_stop.py:235-237,290, README:75, cleanup_sweep.ps1 (finder only).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_refusals.py:16-20 `FIELDS` (a third copy of the eight names, compared with the written line); test_docs_numbers.py:54.
- **Open:** whether ref_judge.py and publish_gate.py call log_refusal as README:79 says (V24 did not reopen).
- **Hook gap:** none.

### L-F22-16: Which fields may a review record carry?
- **Source:** I1-052, I3-058, I2-053 (F22).
- **Kind:** K1, K6-light.
- **Verdict:** NOT VERIFIED (not in V22's list).
- **Homes:** finder only: G\common.py:11-12 REVIEW_KINDS (owner; append_review :239-243); G\review_dispatch.py:5 and :170-172 (fields spelled in code); G\agent_stop.py:211-214 adds `path` for a document record, which REVIEW_KINDS does not list; G\README.md:75 "REVIEW_KINDS in common.py is the one place the review record fields are checked". Message "Recorded reviewer %s's %s verdict for %s." only at agent_stop.py:228.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_common.py:227; test_review_dispatch.py:111-121; test_review_rules.py:76.
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-17: Where is an agent's record written, which fields, and is update_agent "the one way"?
- **Source:** I1-026, I1-057 (F22).
- **Kind:** K1, K6-light.
- **Verdict:** NOT VERIFIED.
- **Homes:** finder only: progress.py:3,74-75 (first, last, calls, warned_calls, warned_minutes, status_calls); stall.py:77,105,134,153-161,238,479 (reads); agent_stop.py:255-283 (writes finished, parked, parked_since, verdict_problem); G\README.md:75 (path, lock file, no field list); status_append.ps1 searches `state\agents`. common.py:188-206 "The one way a hook changes an agent's record"; README:75 "a lock file that only common.update_agent locks" (finder: held in what it read).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_progress.py, test_stall.py, test_agent_stop.py build records; none compares README text.
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-18: Which usage fields add up to context size (stall.py) versus the cost script?
- **Source:** I2-012 (F22).
- **Kind:** K6-light.
- **Verdict:** NOT VERIFIED.
- **Homes:** finder only: G\stall.py:38 USAGE_FIELDS, docstring :350-351; HR tools\agent_cost.py:63-65 reads the same three fields (different question: cost).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-19: What counts as a running or finished agent?
- **Source:** I2-018, I2-047 (F22).
- **Kind:** K6-light, K1.
- **Verdict:** NOT VERIFIED.
- **Homes:** finder only: stall.running_agents (stall.py:74-77, used by wake.py:42-45); stall.py:105, :134, :479 test `rec.get("finished")` inline instead of calling it; agent_stop.py:3-4,255-283 (writer); G\README.md:41 (prose).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_stall.py, test_wake.py, test_agent_stop.py build records.
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-20: Output markers and notices in the preamble (COMMAND FAILED, trim caps, "never end with exit N")
- **Source:** I1-068 (F22).
- **Kind:** K1, K5-light.
- **Verdict:** NOT VERIFIED.
- **Homes:** finder only: G\shell_output.py:30-32 (owner); HR brief-preamble.md:35 (quotes the line, the "4,000" and "30,000", and the rule "never end one with your own `exit N`"); G\README.md:25.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_shell_output.py:22,149,160,202,211,254 pin the strings; test_docs_numbers.py:198 holds the cap numbers, not the line text.
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-21: Do the docs number tests cover every page that states a budget number?
- **Source:** side note (F22).
- **Kind:** K7.
- **Verdict:** NOT VERIFIED (side note).
- **Homes:** finder only: G\tests\test_docs_numbers.py DOCS = CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md; it leaves out WB, G\README.md, MP, agents\*.md and memory notes, which also state "5 minutes" and "100 tool calls and 20 minutes" (MP:5). The finder did not check each page. Hooks-from-scratch plan line 108 (quoted by F22): "Each rule has one home. The numbers ... sit at the top of progress.py and nowhere else in code; the docs quote them".
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py (the test in question).
- **Open:** which of the omitted pages hold numbers.
- **Hook gap:** none.

### L-F22-22: Does the status helper tie the id to the caller?
- **Source:** side note (F22).
- **Kind:** K1 (hook gap).
- **Verdict:** NOT VERIFIED (side note).
- **Homes:** finder only: G\status_append.ps1 needs only that some agent record exists for the given file name.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** G\tests\test_status_append.ps1 (its own output).
- **Open:** none stated.
- **Hook gap:** yes: G\status_append.ps1.

### L-F22-23: Roadblock shape: how many parts, and which three ending words?
- **Source:** I4-020, I4-030 (F22); I4-066, I4-076 (F23).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED for the part-count wording (V22 and V23 did not read it); V23 candidate 5 (headings, keys, endings) was CONFIRMED-CHANGED only on the statuses (see L-F23-02).
- **Homes:** finder only: check.py:32 ROADBLOCK_ENDINGS ("fixed", "worked around", "dead end"); check.py:185-191 requires at least three parts split on " -> "; WB:47-49,80 says "at least two ` -> ` separators" (same rule counted differently); WB:91 the example, copied as a literal in test_session_cards.py:480-481; "cause not found" in the brief only; CATCHERS at check.py:33-37.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:480-481 (a literal copy of the example).
- **Open:** none stated.
- **Hook gap:** none.

### L-F22-24: Does the posture say the brief names the status helper, and does the memory note say the opposite?
- **Source:** I1-021 note (F22).
- **Kind:** K3-light.
- **Verdict:** NOT VERIFIED.
- **Homes:** finder only: MP:11 "A brief names no status file and no id; it carries only the preamble's status rule"; memory every-brief-carries-status-rule.md:15 (tells the agent to write a status line every 5 minutes with the helper status_append.ps1).
- **Splits:** None named by the finder ("mild difference").
- **Ruling quotes:** agents-and-workflow.md:25 on the status rule itself.
- **Tests:** none.
- **Open:** none stated.
- **Hook gap:** none.

---------------------------------------------------------------
## Rows from F23

### L-F23-01: Where do session cards live, and what does a card keep?
- **Source:** I5-013 to I5-016 (F23); V23 candidate 4.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V23 candidate 4: the docs/sessions path in session_cards_start.ps1:274, test_docs_match_code.cpp:1352 and common.py CARDS_ROOT, plus prose).
- **Homes:** HR CLAUDE.md:25 (Past sessions: "cards live in the repo at `docs/sessions/`, one file per session, with one line per card in `docs/sessions/index.md`"). SC\common.py:36-42 CARDS_ROOT, INDEX_FILE. G\session_cards_start.ps1:274 `$rel = 'docs/sessions'` (a second literal; the other folders come from common.py). HR tests\test_docs_match_code.cpp:1352 `kSessionCardsDir = "docs/sessions/"`; .gitattributes:18 (finder). Finder only: docs\agents\audit-method.md:126 near word-for-word copy of CLAUDE.md:25's "A card keeps the user's typed words, the label they chose and the start of each question, not the full question text"; premise-map.md:45,52; agent-rules.md:37,41; rulings\README.md:20; WB:14; memory question-the-design-before-patching.md:15.
- **Splits:** None. F23 noted a memory-archive note calling condensed files "the permanent evidence" (out of scope, not scanned).
- **Ruling quotes:** agents-and-workflow.md:14 (above); speaks to why cards exist, not to a value here.
- **Tests:** none (F23: no test reads CLAUDE.md's section or the hook message).
- **Open:** whether the CLAUDE.md:25 sentence and audit-method.md:126 stay equal (V23 did not reopen).
- **Hook gap:** none.

### L-F23-02: Card shape: headings, header keys, local flag, check statuses ("checked"/"failed")
- **Source:** I4-047, I4-048, I4-070, I4-045, I4-079b (F23); V23 candidate 5.
- **Kind:** K1.
- **Verdict:** CONFIRMED-CHANGED (V23 candidate 5: the statuses are bare literals in seven files, not five).
- **Homes:** SC\common.py:51 HEADINGS (six), :52-57 WORDS_HEADING/OLD_WORDS_HEADING; WB:71 "The six `##` headings appear in exactly that order"; check.py:118-120 six header keys; HR CLAUDE.md:25. Statuses "checked"/"failed": check.py:294, index.py:41, link_memory.py:41, number_rulings.py:41,70, prune_list.py:94, prepass.py:400 (V23); batch.py:49 also uses "failed"/"condensed" (V23). Test copies of the heading order: G\tests\test_ruling_rules.py:121, test_session_cards.py:216-243 GOOD_CARD. WB:18 states the local flag's effect in prose.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** copies only; none holds the brief equal to code.
- **Open:** none.
- **Hook gap:** none.

### L-F23-03: Write-back rule for Clone Hero facts: page prefix and tag
- **Source:** I4-069, I4-077 (F23); I4-032 (F22); V23 candidate 4 (second half).
- **Kind:** K1.
- **Verdict:** CONFIRMED in kind; the count of five homes was not independently checked (V23 candidate 4).
- **Homes:** SC\check.py:40-43,193-198 WRITE_BACK_PAGES "docs/clone-hero/", WRITE_BACK_TAG "clone-hero"; WB:64,81,88; HR CLAUDE.md:29 "A card marked `[write-back]` in the index names a Clone Hero fact its page lacks"; HR docs\clone-hero\README.md:80; index.py:51 (the mark); hand_tags.txt; agents\card-writer.md:9. Test: test_session_cards.py:460-475 pins the messages. The `docs/clone-hero/` string appears in check.py:42 and WB:81, :88.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:460-475.
- **Open:** none stated.
- **Hook gap:** none.

### L-F23-04: The "..." fragment rule for question text on a chose line
- **Source:** I4-074 (F23); I4-025, I4-026 (F22); V23 candidate 2.
- **Kind:** K6.
- **Verdict:** CONFIRMED (V23 candidate 2).
- **Homes:** SC\check.py:153 splits the question on "..." and checks all fragments (:155); G\ruling_rules.py:290-291 does the same, while importing CHOSE from the owner. WB:75-76.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none holds them equal.
- **Open:** none.
- **Hook gap:** none.

### L-F23-05: Card file name: who builds it and who parses it?
- **Source:** I4-072, I4-050, I4-067 (F23); V23 candidate 3.
- **Kind:** K6, K4.
- **Verdict:** CONFIRMED (V23 candidate 3; the brief's file-name lines not reopened).
- **Homes:** SC\common.py:298-299 card_name `{start[:10]}-{sid[:8]}.md`; check.py:38 CARD_FILE parses it; index.py:21 and migrate.py:23 comments call check the "owner of the card file name's shape". WB:31 asks the writer to compute the date; check.py:90-98 rewrites `agents:` from meta.json while WB:39 asks the writer to type it (K4).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F23-06: Which memory files may a card's `memory:` line name?
- **Source:** I4-034 (F23); V23 S1.
- **Kind:** K2.
- **Verdict:** CONFIRMED (V23 S1).
- **Homes:** WB:63 "memory file names from MEMORY.md"; WB:29 read MEMORY.md "for the `memory:` line"; WB:87 "any file that sits directly in the memory folder, whether or not `MEMORY.md` lists it". SC\link_memory.py:70-71 follows the folder listing; :55-59 a name matches only if the file exists. Edge (finder): link_memory.py:55 reads any line starting `memory:` anywhere in the card.
- **Splits:** WB:63/:29 say names come from MEMORY.md; WB:87 says any file in the folder. They differ for a memory file the index does not list.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F23-07: Which hex strings may a card write in Roadblocks or Outcome?
- **Source:** I4-075 (F23); I4-029 (F22); V23 S2.
- **Kind:** K2, K3.
- **Verdict:** CONFIRMED (V23 S2).
- **Homes:** WB:79 "Never write any other 7-to-40-character hex string in Roadblocks or Outcome". SC\common.py:78 HEX `{7,40}` needs a digit; check.py:176-180 also needs a letter a-f and excludes session-id pieces. (See L-F22-02 for other hash rules.)
- **Splits:** The brief forbids every 7-to-40 hex string; the check flags only strings with a digit and a letter that are not session-id pieces. They differ for an all-digit string of 7 or more characters, or a session-id piece.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none (finder: edge at exactly 7, 40, 41 characters not tested).
- **Hook gap:** none.

### L-F23-08: Which transcript records count as the user's own typing?
- **Source:** I4-053 to I4-056, I4-088, I4-093, I4-057, I4-110 (F23); V23 candidate 1.
- **Kind:** K6.
- **Verdict:** CONFIRMED (V23 candidate 1).
- **Homes:** G\review_rules.py:89-113 human_prompt_text decides by `turnOrigin == "human"`, not isMeta, not isSidechain, no toolUseResult (V23: :103-104), used by percall.py:34 and waived() (:124-131). SC\common.py:372-459 classify_user decides by `origin.kind == "human"` plus typed command args and queued prompts (V23: :397-402); common.py:331-333 "The card checker and the ruling gate both ask this"; ruling_rules.py:27-30, :270-271 reads the card tools' answer. Finder only: agent_stop.py:30,51 and test_review_dispatch.py:330-335 repeat the `<task-notification>` tag; replay_extract.py:69 copies Claude Code's refusal wording (common.py:75 REJECTION_PREFIX).
- **Splits:** Both decide "did the user type this" but read different fields. V23: they differ in the field read and in slash-command and queued prompts (only common handles them). Whether they ever disagree on one real record needs a run over a transcript.
- **Ruling quotes:** None found.
- **Tests:** none holds them equal. test_review_rules.py:891 pins human_prompt_text rejecting turnOrigin sdk, isMeta, isSidechain (not read by F23 beyond grep).
- **Open:** a run over real transcripts to see whether the two ever disagree.
- **Hook gap:** none.

### L-F23-09: Decision log: entry shape, D-number ceiling and first numbered day
- **Source:** I4-049, I4-100 to I4-103 (F23); V23 S5.
- **Kind:** K2, K1/K7.
- **Verdict:** CONFIRMED for the two entry shapes (V23 S5). The ceiling and the first-day constant are finder only.
- **Homes:** Log line 7 `**D1 (finding 28), 2026-10-03.**`; SC\number_rulings.py:104 writes `**{number} (session {sid[:8]}), {date}.**`; 213 log lines start `**D<n> (session` (V23). DECISION_ENTRY (common.py:92) reads only the number. Finder only: common.py:80 `DNUM = \bD(\d{1,4})\b` against number_rulings.py:64 "Below 1000 only: MSVC error codes such as D8003" (D1000-D9999 pass DNUM and are excluded at 1000); number_rulings.py:30 `FIRST_NUMBERED_DAY = "2026-10-03"` and common.py:44 DECISIONS_LOG file name carry the same date.
- **Splits:** The log has two entry shapes from two writers; no consumer misreads it (DECISION_ENTRY needs only the number).
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:859-871 pins "below 1000" by value.
- **Open:** none.
- **Hook gap:** none.

### L-F23-10: How many card-writer batches launch together versus the memory fan-out rule?
- **Source:** G6 (F23); V23 S4.
- **Kind:** K2 (weak).
- **Verdict:** CONFIRMED-CHANGED, weak (V23 S4: the note's 3-4 per round is for repeated discovery rounds, a different job, and the note's other rule says approved independent tasks launch together; batch.py holds no text naming the "at-once limit" the hook message cites).
- **Homes:** SC\batch.py:23-26 `MAX_BATCH_BYTES = 200*1024`, `MAX_BATCH_SESSIONS = 10`, `BATCHES_PER_WAVE = 10`; G\session_cards_start.ps1:277 "(batch.py sizes a wave to the at-once limit)"; memory lean-agent-fanout.md:21 "3-4 agents per round". HR CLAUDE.md rule 14 limits builds (three at once), not agents.
- **Splits:** batch.py:26 (10 per wave) vs lean-agent-fanout.md:21 (3-4 agents per round); V23 did not show them to be one question.
- **Ruling quotes:** None found (a memory note is not the user's words).
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F23-11: Do the hook message and the card-writer brief restate rules owned elsewhere?
- **Source:** G6 (F23); V23 candidate 7.
- **Kind:** K5, K1.
- **Verdict:** CONFIRMED-CHANGED (V23 candidate 7: confirmed in part; session_cards_start.ps1:268 comment says docs/sessions-only commits need no reviewer and every commit needs Task, Agent and Session trailers, owner the ref gate; the progress.py numbers in the brief were not reopened).
- **Homes:** G\session_cards_start.ps1:277,283,285,289 (batch, trailers, docs check command, "A commit that changes only docs/sessions needs no reviewer"); owner review_rules.explain_main_move per the hook's comment. WB:100-101 repeats "One status line every 5 minutes", "Only time counts" and "At 20 minutes or 100 tool calls, stop starting new cards" (numbers owned by G\progress.py; finder). WB:100 says to use `<your agent id>` while the preamble says the hook gives it. agents\card-writer.md:9 "skip the preamble".
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** the progress.py numbers in WB were not reopened.
- **Hook gap:** none stated.

### L-F23-12: Does check.py's IGNORED_LINES have any use?
- **Source:** I4-071, side note (F23); V23 side note.
- **Kind:** K7 (dead value).
- **Verdict:** CONFIRMED (V23: only its definition exists in SC, all SC files searched).
- **Homes:** SC\check.py:39 `IGNORED_LINES`; no other use.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F23-13: Other finder-only notes (not reached by V23)
- **Source:** G3, G5, side notes (F23).
- **Kind:** K1, K5, K6.
- **Verdict:** NOT VERIFIED.
- **Homes (finder only):** SC\link_memory.py:24 `INDEX_FILE = "MEMORY.md"` and common.py:42 `INDEX_FILE = "index.md"` (two constants of one name for two files). WB:75 "never quote ... anything in the `unsent` list" against check.py:164 (check accepts quotes found in typed messages; `unsent` is not in those). WB:18 describes the local flag's effect (owner check.keeps_local/publish, index.kept_locally_line). G\session_cards_start.ps1:48 "30 s timeout, less 5 s" copies a number from settings.fragment.json (not opened). Index text "[stale]" and "[failed]" marks only in index.py:50.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_session_cards.py:650,400,509,518,529,678,897 hand-copy text (finder).
- **Open:** all of it; the eight test rows (112, 114, 117, 118, 120, 121, 124, 127) were read second-hand by F23 and not reread by V23.
- **Hook gap:** none.

### L-F23-14: Card writers skip the preamble, so do the preamble's rules reach them?
- **Source:** side note (F23).
- **Kind:** K5 (hook gap, general).
- **Verdict:** NOT VERIFIED (side note; the finder did not check whether the progress hook counts the card-writer).
- **Homes:** finder only: agents\card-writer.md:9 "That brief replaces the repo's `docs/agents/brief-preamble.md` for you ... skip the preamble"; WB holds a copy of the status and stop numbers (see L-F23-11).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** whether progress.py counts card-writer calls.
- **Hook gap:** yes: agents\card-writer.md (the finder's words: "a way past a hook ... I did not check whether the progress hook counts it").

---------------------------------------------------------------
## Rows from F24

### L-F24-01: What may a plan, review or closure cite as "the user's own words"?
- **Source:** I5-021, I5-056, I6-017, I6-018, I6-023, I6-081, I6-174, I6-123 (F24); side note S6; V24 Group A.
- **Kind:** K1 (about ten homes on "Confirmed"), K2/K3, K6.
- **Verdict:** CONFIRMED-CHANGED (V24: A1 and A2 confirmed with changes, below). K1 on "Confirmed only" CONFIRMED (V24).
- **Homes:** HR CLAUDE.md:41 "Only a line under `## Confirmed` may be cited as the user's ruling." AR:35 "A line under `## Confirmed` is the user's words."; AR:37; AR:39; AR:41 "a decision entry there counts only when it quotes the user's words as found under `## Confirmed` or in a session card" and "Git checks only `docs/agents/rulings/`". DOR:45 (step 4) lists ADR, CONTEXT.md, Confirmed, "User decisions", and later says an entry "counts only when it quotes the user's words as they are found under `## Confirmed` or in a session card". DOR:52 (step 9) "a D-record, a plan's "User decisions", a line under `## Confirmed`..., a session card, or the words your brief quotes". DOR:55 "from a line under `## Confirmed` ... or a session card". DR:63 "Confirmed, a session card or the brief's quote". PM:45 "Confirmed ..., a session card's quoted words ..., or the user's message as the main session quotes it in the reviewer's brief and confirms"; PM:52. TR:76 "a line under `## Confirmed` in `docs/agents/rulings/` that names the value" (no card). AM:25 closure quote naming both sides; AM:126 lists "the decision log, the question files, the plans, and the session cards" (rulings folder not named; V24 confirmed). G\ruling_rules.py:95,270 (code owner of filing). Memory MEMORY.md:1,49. Unconfirmed home, not a ruling: rulings\questions-and-audits.md:28 "It closes only on the user's words that name both sides" (V24 did not reopen).
- **Splits:**
  - A1: the pages give different lists of where the user's words can be (DOR:52, DR:63, PM:45, TR:76, AM:126); TR:76 has no card, AM:126 has no rulings folder, PM:45 adds "and confirms", DR:63 has no D-record. V24 change: DOR:45 does not omit the card (its second sentence names it). They differ by which step or reader applies the page (reviewer step 4, step 9, test reviewer, audit brief).
  - A2: DOR:52 lists "a D-record" and "User decisions" as places the words sit, while AR:41 and DOR:45 say such an entry counts only if it quotes Confirmed or a card. V24: weaker than the finder said; DOR:52 does not say a D-record must itself quote a Confirmed line or card; wording gap, readable as one rule.
- **Ruling quotes:** None found that names both sides of A1 or A2. Closest (about sign-off, not this): review-and-hooks.md:19. questions-and-audits.md:28 is Unconfirmed.
- **Tests:** none for the prose lists. G\tests\test_ruling_rules.py tests the filing gate only; test_docs_numbers.py reads five docs for budget numbers and retired words only.
- **Open:** which checkout's rulings folder is read when a worktree holds an uncommitted line (F24: no home says).
- **Hook gap:** none.

### L-F24-02: Is a session card's quote the user's words, or "a pointer, not proof"?
- **Source:** I5-017 (F24), A3; V24 A3.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED (V24 A3: "pointer, not proof" is about claims on code, so the live split is "a card quote is not a filed ruling" against "a ruling counts when it quotes ... a session card").
- **Homes:** HR CLAUDE.md:25 "a card quote is not a filed ruling", CLAUDE.md:27 "A card is a pointer, not proof: check what it says against the code". WB:67 "holds the user's exact words from this session, not filed rulings: a ruling is only a line filed under `## Confirmed`". DR:63, DOR:52, DOR:55, PM:45 accept a card's quoted words; PM:52 "A session card counts only as the place a `ruling`'s quoted words sit." AR:37 "looks for the quote in a session card". AM:126. F24 found no other "pointer, not proof" phrasing.
- **Splits:** A3: "a card quote is not a filed ruling" (CLAUDE.md:25, WB:67) vs "a ruling counts when it quotes a session card" (DR:63, DOR:52, DOR:55, PM:45, AR:37). PM:52 is the one line that narrows it.
- **Ruling quotes:** None found naming both sides. agents-and-workflow.md:14 speaks to cards existing so Claude can find what the user ruled; it does not name both sides.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F24-03: Is the owner map's shape held by anything but instruction, and do agent files restate it?
- **Source:** I5-029, I5-128, I5-142, I6-149, I6-152 to I6-155 (F24); V24 Group B.
- **Kind:** K7, K1, K5.
- **Verdict:** CONFIRMED (V24: searched HR tests, tools, src, .claude for "Owner map" with no hit, and G\*.py for `O1:` and `carry out` with no hit; scout.md restates in part).
- **Homes:** HR docs\agents\owner-map.md:3 (one-home claim), :21-29 (heading, four columns, `O1:` ids, statuses `found`, `split`, `carry out`, `none`), :35-47 (stops). HR CLAUDE.md:55; AR:25; agents\planner.md:19; agents\reviewer.md:17; agents\scout.md:11 (names two of the four status words, `carry out` and `none`; V24: a partial copy, no disagreement); BP:99,111 ("owner-map.md, "The stops", is the one home of this rule").
- **Splits:** None (F24: scout.md lists two of four status words; OM:25 lists four; V24: partial copy).
- **Ruling quotes:** None found. owner-map.md:7 quotes the user ("Yes, write them") in prose, not in the rulings folder (F24).
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F24-04: Premise map shape: Kind words, heading and cut-off dates in the page and in the docs test
- **Source:** I5-028, I5-125, I6-025, I6-163 to I6-169 and others (F24); side note S1; V24 Group B.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V24: constants read at test_docs_match_code.cpp:1520, :1525, :1528-1530; S1 confirmed).
- **Homes:** HR tests\test_docs_match_code.cpp:1520 `kPremiseMapSince = "2026-11-01"`, :1525 `kDeferredPremiseSince = "2026-11-13"`, :1528 `kQuestionsHeading`, :1529 `kLiveStepsHeading`, :1530 `kKindWords[] = {"code","data","chart","run","ruling","unchecked"}`, comment :1509 "docs/agents/premise-map.md owns the shape". HR docs\agents\premise-map.md:37-46 lists the same six; :76 "the comment above it says which plans count". Restated in HR CLAUDE.md:55, agents\planner.md:13,15, agents\reviewer.md:13, AR:21, docs\agents\clone-hero-mcp.md:58, DR:63.
- **Splits:** None. The cut-off dates are in the test only; the page names neither.
- **Ruling quotes:** None found.
- **Tests:** docs test "every plan that counts holds a Premise map in the shape" holds a plan to the page, but nothing holds the page's list to `kKindWords`; a change to one does not fail the other.
- **Open:** none.
- **Hook gap:** none.

### L-F24-05: Are the four Clone Hero labels in premise-map.md:42 the ones the Clone Hero page uses?
- **Source:** I6-173 (F24); V24 Group B.
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V24: "OPEN. Needs a read of docs/clone-hero/README.md; not reached." F24 also did not open it.)
- **Homes:** finder only: PM:42 quotes "Proven in the game's code", "Proven in a live run", "Hydra assumes it, never checked", "Unknown"; says docs\clone-hero\README.md owns what each means.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** the docs test checks the Kind word only, not the labels.
- **Open:** a read of docs\clone-hero\README.md.
- **Hook gap:** none.

### L-F24-06: The "exact thing" source rule and the "moves or converts a quantity" rule: one home each?
- **Source:** I6-168, I6-025, I6-082, I5-126 (F24).
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V24 did not address these).
- **Homes:** finder only: PM:29 "A source backs a row only when it works the fact out for the thing the row names." DOR:53 (step 10) "the exact thing its owner worked it out for ... read from the owner's code and not from the reader's variable name"; neither page points at the other. DR:63 "Sending a press and reading a value are live parts. Deciding how far a read moves a later press is not."; agents\planner.md:15 "A step that moves or converts a quantity the engine owns is never a live part." Not stated in PM.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none stated.
- **Hook gap:** none.

### L-F24-07: Sign-off line: what the reviewer writes and what the hook reads
- **Source:** I6-047, I6-087 (F24); V24 Group D.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED (V24 Group D).
- **Homes:** DOR:146-151 "Write no hash, key or path with it. The hook reads nothing else in your report." DR:117-125; agents\reviewer.md:28-31 "Write no hash, key or path after it; nothing else in the report is read." G\README.md:43 "a whole line that is exactly `SIGN-OFF: BLOCKED`... A report with neither line is recorded CLEAN too". G\agent_stop.py:23 `_BLOCKED, _CLEAN`, :102-109 verdict_word (`lines = {s.strip() for s in report.splitlines()}`; BLOCKED if the exact string is a whole line, else CLEAN if so, else CLEAN with a missing-line problem; V24). reviewer.md:31 and :33 (V24: both there, wording invites misreading, not a split).
- **Splits:** None. (F24's S4 "same hook in two ways": V24 says not a split.)
- **Ruling quotes:** review-and-hooks.md:19 (Confirmed) "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." It covers the rule that a report with no line is recorded CLEAN; it does not name the added-text case (V24). Also review-and-hooks.md:18 "why does the hook refuse to accept that a review happened because it didnt follow the exact expected format? this is just bad design" (F24).
- **Tests:** G\tests\test_agent_stop.py (`SIGN-OFF: CLEAN` flows around line 305, problem text around 317); no test holds DOR, DR or reviewer.md text to the code.
- **Open:** none.
- **Hook gap:** see L-F24-08.

### L-F24-08: Can added text after the sign-off line turn a BLOCKED into CLEAN?
- **Source:** side note S5 (F24); V24 S5.
- **Kind:** K5 (hook gap).
- **Verdict:** CONFIRMED (V24 S5).
- **Homes:** G\agent_stop.py:102-109 verdict_word: a line `SIGN-OFF: BLOCKED` followed by any text is not an exact whole line, so it matches neither and the report is recorded CLEAN. The docs warn "write no hash... with it" but do not spell out this reason (F24).
- **Splits:** None.
- **Ruling quotes:** review-and-hooks.md:19 covers "no sign-off line records CLEAN"; it does not name the added-text case (V24).
- **Tests:** none named for this case.
- **Open:** none.
- **Hook gap:** yes: G\agent_stop.py.

### L-F24-09: Report section lists for code and document reviews
- **Source:** I6-079, I6-088 (F24).
- **Kind:** K7.
- **Verdict:** NOT VERIFIED (V24: only the DOR sign-off section re-read; the rest not re-verified).
- **Homes:** finder only: DOR:128-151 (eight named sections); DR:108-115 (six: Restatement, Findings, Numbers checked, References opened, Notes, Where I looked); agents\reviewer.md names none; DR:41 cold read "at most five sentences ...".
- **Splits:** None (the finder reads the two lists as different by design of the two reviews; "by design" is the finder's word, not evidence).
- **Ruling quotes:** None found.
- **Tests:** none hold the section names; the hook reads only the sign-off line.
- **Open:** none stated.
- **Hook gap:** none.

### L-F24-10: Handoff: shape, status words, exit codes and tokeniser rules versus the checker
- **Source:** I6-091 to I6-096, I6-099 to I6-102 (F24); S3 partly (see L-F22-02).
- **Kind:** K1, K5.
- **Verdict:** NOT VERIFIED (V24: group E rows beyond the hash-length check not reached).
- **Homes:** finder only: HR docs\agents\handoff.md:11-67 (owner), :51-57 (status words, exit codes), :59 (tokeniser rules; "A hex word is a commit" with no length); HR tools\check_handoff.py (`CLAIMS_HEADING`, `MANUAL_KINDS = ("test","ruling","state")`, `TABLE_STATUSES = ("ok","missing","not-on-main","not-pushed","manual")`, :316 and :393 exit rules); tools\doc_tokens.py (7 to 40 hex); .claude\commands\handoff.md steps 2-4; BP:97; AR:15. No code constant lists all seven Claims kinds in one place (F24). The checker reads only Claims rows, not `## Checker output` or the prose.
- **Splits:** None (F24: agree today).
- **Ruling quotes:** None found.
- **Tests:** tools\test_check_handoff.py tests the checker; no test holds handoff.md's prose to the code.
- **Open:** all of it beyond the hash lengths.
- **Hook gap:** none.

### L-F24-11: What does a finder's report hold? (audit-method vs the T5 finder brief)
- **Source:** I6-131 (F24); F1.
- **Kind:** K2.
- **Verdict:** NOT VERIFIED (V24: F1 not re-opened).
- **Homes:** finder only: AM:50 "a table of inputs and answers for each home, with the edges in. Edges are values like an empty list, exactly 0, a value right at a window edge, or a missing optional." The T5 finder brief (docs\audit\2026-11-18-tooling-derive-once\briefs\T5-finder.md:34,39) asks for "what each home tells its reader to do" with edges "exactly at the limit, the main session against an agent, an agent type that skips the preamble, and a worktree against the main checkout", and "Do not grade candidates".
- **Splits:** F1: the table shape and the edge list differ between AM:50 and the T5 finder brief; each page is for a different reader.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none stated.
- **Hook gap:** none.

### L-F24-12: What does a brief carry?
- **Source:** I5-099 (F24); F2; V24 Group F.
- **Kind:** K2.
- **Verdict:** CONFIRMED-CHANGED (V24: confirmed as readings: different readers, no pointer between the pages).
- **Homes:** BP:3 "task id, the session id, your owned files, your worktree or checkout, your scratch folder and the tests you may run"; AM:120 "the agent's task id, the session id, its files or batch, its caps, and the status rule" (V24 read both lists); memory every-brief-carries-status-rule.md (the status sentence).
- **Splits:** F2: three lists of what a brief carries. BP:3 has worktree, scratch folder and tests; AM:120 has caps and the status rule; the note has the status sentence. Each serves a different reader.
- **Ruling quotes:** agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" speaks to the status-sentence part, not the list split.
- **Tests:** none.
- **Open:** F24 did not check whether the card-writer brief carries the status rule (WB:100 does carry a status line).
- **Hook gap:** none.

### L-F24-13: What does a final report hold by default, and who overrides it?
- **Source:** I5-098 (F24).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V24: beyond reading BP:3 and AM:120, not reached).
- **Homes:** finder only: BP:127 (default list: branch and worktree, commit hashes, summary, test commands and counts, numbers measured, stops); DOR:128-151; DR:108-115; WB:103-105 ("The card files you wrote, the session ids you did not reach ... Nothing else."); agents\reviewer.md:24; T5-finder.md:4. BP:127 begins "Your brief says what to return. If it does not ...", which F24 reads as a default only.
- **Splits:** None (F24: not a disagreement by the page's own words).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none stated.
- **Hook gap:** none.

### L-F24-14: The 2-second wait on the agent record lock
- **Source:** I5-147 (F24); V24 Group G.
- **Kind:** K1.
- **Verdict:** CONFIRMED (V24: common.py:18 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001`; README:9).
- **Homes:** G\README.md:9 "After a 2-second wait it writes anyway and says so in the hook's output; it never refuses a call over the lock." G\common.py:18, `_locked_call` (:164), `update_agent` (:187; finder).
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** test_common.py (F24: not opened).
- **Open:** none.
- **Hook gap:** none.

### L-F24-15: Do README sentences about agent_stop problem logging and onFailure match code?
- **Source:** I5-169, I5-180 (F24).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V24 did not address them).
- **Homes:** finder only: G\README.md:51 "Every problem goes into the agent's log line in `agent_stop.jsonl` and into the agent's own record, as its `verdict_problem` field, which `stall.py` relays" against agent_stop.py:11,278,282 and stall.py:268-273 (agree). G\README.md:93-104 against G\common.py:52-70 `run(body, mode...)` ("closed"/"loud", always returns 0, reads stdin UTF-8); README says `onFailure: block` on refusing hooks; G\settings.fragment.json has 3 `onFailure` entries (:153, :169, :185); the finder did not check they are exactly the refusing hooks.
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:18 "agents should fail loudly" (F22) relates to the "loud" mode; not a split.
- **Tests:** none named.
- **Open:** whether the three onFailure entries are exactly the refusing hooks.
- **Hook gap:** none.

---------------------------------------------------------------
## Ending lists

Candidates the verifiers REFUTED: none.

Rows no verifier reached (NOT VERIFIED): L-F22-07, L-F22-11, L-F22-16, L-F22-17, L-F22-18, L-F22-19, L-F22-20, L-F22-21, L-F22-22, L-F22-23, L-F22-24, L-F23-13, L-F23-14, L-F24-05, L-F24-06, L-F24-09, L-F24-10, L-F24-11, L-F24-13, L-F24-15.
Partly reached (verified only in the part named in the row): L-F22-05 (percall not reopened), L-F22-10 (secrets), L-F22-13 (BUILTIN_CODE as decider), L-F22-14 (V22 CONFIRMED, V24 CONFIRMED-CHANGED), L-F22-15 (file-name copies and event names), L-F23-03 (count of five homes), L-F23-09 (ceiling and first day), L-F23-11.

Counts: 53 rows (F22 24, F23 14, F24 15). CONFIRMED 26, CONFIRMED-CHANGED 7, REFUTED 0, NOT VERIFIED 20. Distinct splits listed: 17 (hash 40 vs 64; hash 7-40 vs 7-64; hash case; parentheses; ruling-line tail; .derive-once-gate decider; memory note line 3 vs 15; `memory:` names; hex rule brief vs check; two human-typing deciders; decision log entry shapes; batch wave vs fan-out note; A1; A2; A3; F1; F2).
Hook gaps flagged: G\status_append.ps1 (L-F22-22), agents\card-writer.md (L-F23-14), G\agent_stop.py (L-F24-08).
