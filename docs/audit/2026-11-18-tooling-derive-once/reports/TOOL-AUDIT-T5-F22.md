# TOOL-AUDIT-T5-F22 (finder F22, session f6a384b5-9eed-4607-95cb-681ec635d569, agent a891c66bbd8697769)

Short paths: G = C:\Users\Patrick\.claude\gates, SC = C:\Users\Patrick\.claude\tools\session_cards, H = Hydra main checkout (C:\Users\Patrick\Downloads\Hydra\hydra-test).
Searched for every row: gates (py, ps1, sh, README, settings.fragment.json, tests), tools/session_cards (py, writer-brief.md), agents/*.md, model-postures.md, ~\.claude\CLAUDE.md, Hydra CLAUDE.md, Hydra docs/agents (with rulings), Hydra tools/*.py, memory notes (not archive), both .derive-once-gate files. I searched by the question (words, numbers, shapes), not only by names.
Read first as asked: hooks-from-scratch plan line 108 ("Each rule has one home. The numbers ... sit at the top of progress.py and nowhere else in code; the docs quote them, and a test in T11 greps ... The user approved matching the code: 100 calls, minutes only."); the Confirmed lines of rulings/review-and-hooks.md and agents-and-workflow.md.
Rows reached: all 57, at uneven depth (full write-ups below; one-home rows listed at the end). Rows not reached: none. Depth caveat: for I4-006..I4-009 I did not search every rulings topic file for the 2026-10-31 card-content ruling.

## Rulings found (Confirmed lines: the user's words, not homes)
- agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (session e81b56eb). Speaks to I1-021 and I1-093.
- agents-and-workflow.md:26 "the status updates should be brief but relayed in plain english" (e81b56eb). Speaks to the relay in I2-025.
- agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (a9a461da). Speaks to the "loud" mode of common.run and agent_stop's problem notes.
- review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time ... hard cap of 30 minutes real time." (e82ae000). Beside I1-021, not a row of mine.
- review-and-hooks.md (2026-10-31) "im not too concerned about the exact number as much as efficiency and functionality. if a hook is doing something unnecessary or contains a lot of arbitrary rules that needs to scrutinized" (6777b546). General.
No Confirmed line I read speaks to hash length, the refusal log, record file names, the ruling-line shape or the card-brief rules.

## I1-021 The status line's form ("HH:MM done ... | next: ...") and the 5-minute interval
Homes:
1. G\progress.py:58 `'%s "%s" "HH:MM done ... | next: ..."' % (STATUS_PREFIX, agent_id)` (owner; hook notes and refusals are built from it). progress.py:16 `STATUS_NAG_MINUTES, STATUS_DENY_MINUTES = 5, 10`.
2. H\docs\agents\brief-preamble.md:13 "every 5 minutes. The form is `HH:MM done ... | next: ...`."
3. H\docs\agents\derive-once-review.md:9 "every 5 minutes, form `HH:MM done ... | next: ...`" plus the helper call.
4. SC\writer-brief.md:100 `"HH:MM done <n> of <m> cards | next: <session>"` and "One status line every 5 minutes".
5. Interval only: H\CLAUDE.md:69 (rule 7), H\docs\agents\agent-rules.md:13, G\README.md:124 ("At 5 minutes old it adds a reminder. At 10 minutes old it refuses"), memory every-brief-carries-status-rule.md:15, memory relay-status-in-plain-english.md.
6. G\status_append.ps1 default line `HH:mm (no message)` (I1-093).
What each tells its reader:
| Home | Reader | Tells them |
|---|---|---|
| progress.py | every agent, by hook note | write the line; nag at 5, refuse at 10 (git and final report still pass) |
| preamble | every agent except card writers | line every 5 min in the form above |
| derive-once-review.md:9 | reviewers | same plus the helper call |
| writer-brief.md:100 | card writers (Haiku; agents\card-writer.md:9 says skip the preamble) | line every 5 min with its own form ("n of m cards"); helper call with an unquoted id (progress.is_status_call accepts quoted or unquoted) |
| main session | CLAUDE.md:69 states it for all agents; the main session has no status duty (progress is for agents; stall relays) |
| exactly at 5 min | README:124 says the hook adds a reminder at 5; the docs say "every 5 minutes", so a line at exactly 5 minutes is already nagged: docs and hook meet at the same number, the reminder fires when the line is 5 minutes old |
| worktree vs main | status files live in ~\.claude\gates\state, same for both |
Kinds: K1 (interval and form, agree today). K3-light: writer-brief form carries card counts where the preamble form does not.
Test holding homes equal: G\tests\test_docs_numbers.py:116-120 test_status_interval_is_the_nag_minutes over DOCS (CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md); :122-127 rule 7 quotes the budget; :182-190 preamble quotes "every 5 minutes", "10 minutes" and STATUS_PREFIX. Not covered by any test: the form text "HH:MM done ... | next: ...", writer-brief.md, G\README.md, model-postures.md, the memory notes.
Split (own line): memory every-brief-carries-status-rule.md front-matter description says "the dispatch gate now denies briefs that lack it"; the same note's line 15 says "No gate checks a brief for this sentence today". I found no code in review_dispatch.py, percall.py, prompt.py or settings.fragment.json that checks a brief for the status rule (percall.py:112 only uses is_status_call). The front-matter claim has no code behind it (K8, inside one note).
Mild difference: model-postures.md:11 "A brief names no status file and no id; it carries only the preamble's status rule" against the memory note line 15 ("tells the agent to write a status line every 5 minutes with the helper status_append.ps1"): one wants the helper named in the brief, the posture says only the preamble's rule (K3-light).

## I1-093 / I1-095 status_append.ps1 default line, messages and exit codes
Only home of the default line and of "status_append: appended to <file>" and the refusal wording is the script. G\README.md:65 describes the refusal and states the exit ("writes nothing and exits 1"): a pointer that states a value (K1). Test: G\tests\test_status_append.ps1 holds the script's own output. The refusal text tells the agent to "Use the agent id the progress hook gave you at your first call"; progress.py:88 gives it.

## I1-026 Agent record location and fields
Homes: progress.py:3,74-75 (writer of first, last, calls, warned_calls, warned_minutes, status_calls); stall.py:77,105,134,153-161,238,479 (reads finished, first, last, status_calls, parked_since); agent_stop.py:255-283 (writes finished, parked, parked_since, verdict_problem); G\README.md:75 gives path `agents\<session_id>\<agent_id>.json` and the lock file; status_append.ps1 searches `state\agents` for `<id>.json` in every session folder. cleanup_sweep.ps1: grep for finished/first/last found no field names; README:69 says it prunes old records.
Kinds: K1 (path in README, script, progress.py); K6-light (record fields written by three modules, no single field list; README:75 lists none). Test: test_progress.py, test_stall.py, test_agent_stop.py each build records with field names; none compares README text.

## I1-051 / I1-058 / I1-022 The refusal log
Homes: common.py:35-49 (owner: file name, 8 fields, `reason[:300]`, logs deny and ask); progress.py:31 `QUOTE_CHARS = 300` (how much of a refused command is quoted back; same number, a different question); G\README.md:79,83,88 (file name, "whenever the answer denies or asks", sample query); cleanup_sweep.ps1:55,549,584 (30-day prune, file name restated); bash_windows_rule.py:209 comment "The tag starts the reason, so a count of refusals can read the part from it"; test_refusals.py:20 `FIELDS = {...}` (the 8 fields).
Kinds: K1 (file name in four files; the number 300 in two modules; no test ties the two 300s).
Test holding homes equal: test_refusals.py (field set vs writer); test_docs_numbers.py:54 README_NAMES requires README to name refusals.jsonl.

## I1-052 / I3-058 / I2-053 Review record kinds and fields
Homes: common.py:11-12 REVIEW_KINDS (owner; append_review :239-243 refuses a record missing a field); review_dispatch.py:5 docstring "common.REVIEW_KINDS" (pointer) and :170-172 builds the handover with the fields spelled in code; agent_stop.py:211-214 builds the verdict and adds `path` for a document record (`dict(r, path=path)`), a field REVIEW_KINDS does not list; G\README.md:75 "REVIEW_KINDS in common.py is the one place the review record fields are checked". The message "Recorded reviewer %s's %s verdict for %s." exists only at agent_stop.py:228 (test_agent_stop.py:642 checks "Recorded reviewer").
Kinds: K1 (field lists), K6-light (`path` is read by review_rules for documents but declared in no field list; the README's "one place" holds for required fields only).
Tests: test_common.py:227 handover round trip; test_review_dispatch.py:111-121 pins the handover shape; test_review_rules.py:76 builds a verdict.

## I3-021 / I3-034 What is a valid commit hash (shape)
Homes, each in its own words:
- G\ref_judge.py:195 `len(fields[0]) not in (40, 64)` plus `strip("0123456789abcdef")` (lowercase only).
- G\review_rules.py:56 `^[0-9a-f]{40}([0-9a-f]{24})?$`; :57 `^0+$`.
- G\review_dispatch.py:19 `[0-9A-Fa-f]{40}` (upper case accepted, lowered at :32; 64-character hashes not accepted).
- G\review_rules.py:26-28 waiver phrases: derive-once and real-app `{7,40}`, doc-review `{7,64}`.
- SC\common.py:78-79 `{7,40}`.
- H\tools\check_handoff.py:28 `{40}` both cases; check_pointers.py:38 `{7,40}`; doc_tokens.py:39,50 `{7,40}`; ci_build_folder.ps1:99 `{40}` lowercase; effort_test_faults.py:37 `{40}`.
- Prose: G\README.md:39, derive-once-review.md:17,200,202, handoff.md:19,35 all say "40-character".
Kinds: K6 (several code places decide "is this a hash"), K2 at the edges: the word "hash" means 40 only (dispatch), 40 or 64 (judge, rules), 7-40 (derive-once and real-app waivers) or 7-64 (doc-review waiver); case differs (dispatch and check_handoff take upper case; judge and rules lowercase only).
Split (own line): a 64-character (sha256) key is accepted by ref_judge and review_rules but a reviewer brief naming one is not found by review_dispatch's 40-hex pattern. Edge, exactly at the limit: 40 vs 64 as above. Main vs agent: reviewers' keys are 40 by rule (derive-once-review.md:17). Worktree vs main: no difference found.
Test holding homes equal: none found.

## I1-057 update_agent "the one way"
common.py:188-206 "The one way a hook changes an agent's record"; README:75 "a lock file that only common.update_agent locks". Callers: progress.py, stall.py, agent_stop.py finish(). status_append.ps1 and cleanup_sweep.ps1 do not write the record. The claim held in what I read. Kind: K1 (README repeats the claim).

## I1-059 Output shapes deny / context / system_message
Owner common.py:21-32. By-hand copies: session_cards_start.ps1:362-363 builds `systemMessage` and `hookSpecificOutput` with `hookEventName='SessionStart'` and `additionalContext`; session_cards_start.ps1:303 and cleanup_sweep.ps1 print `systemMessage` JSON themselves; percall.py:134 builds an "ask" from deny with `permissionDecision="ask"`; percall.py:158 sets `updatedInput` by hand; prompt.py:60-62 joins part replies. README:93 describes them. Kinds: K6 (shape in common.py plus two PowerShell hooks plus percall). No test holds the PowerShell shape equal to common.py.

## I1-096 "the one place a hook reads its input" (lib\hook_input.ps1:1)
Claim: Read-HookInput is the one place a hook reads its input. cleanup_sweep.ps1:698 and session_cards_start.ps1:299 use it. Python hooks read stdin themselves: common.py:58-62 (`sys.stdin.buffer`, decode "utf-8"), wake.py:92-96 (own read), ref_judge.py:398 (`decode("utf-8","replace")`), publish_gate.py (reads the call itself). Only the PowerShell reader trims U+FEFF. README:93 says the Python reading is common.run's job.
Kinds: K8 (the "one place" claim holds for PowerShell hooks only), K6 (UTF-8 stdin read written four times in Python and once in ps1). No test compares them.

## I1-068 Output markers and notices
Owner shell_output.py:30-32. Quoted in H\docs\agents\brief-preamble.md:35 (`COMMAND FAILED, exit code N`, the trim notice described, "4,000" and "30,000") and G\README.md:25. Tests: test_shell_output.py:22,149,160,202,211,254 pin the strings; test_docs_numbers.py:198 test_preamble_quotes_the_output_cap holds the cap numbers, not the line text. Kind: K1. Preamble line 35 also says "never end one with your own `exit N`" and names shell_output.py as owner of "the number and the lists": a pointer plus a prose rule of the hook (K5-light).

## I2-012 Which usage fields add up to context size
Homes: stall.py:38 USAGE_FIELDS (owner, docstring :350-351 "the three input counts added up", "Ported from stall_inject.ps1"); H\tools\agent_cost.py:63-65 reads the same three fields (input, cache read, cache creation) to price a run, not to size a context. Kind: K6-light (same fields, different question: size vs cost). No test holds them equal.

## I2-015 / I2-032 / I2-035 / I2-039 stall.py's own words
Each has one home in stall.py: `.workflow.json` (:41), `journal.jsonl` with `type` started/result and `agentId` (:284-295, 310), TROUBLE_KEYS (:381), trouble() order (:408). I searched all of the scope for the suffix, journal.jsonl, agentId and the order words; the only other hits are unrelated (H\tools\effort_test_taint.py:141 reads `agentId` from transcripts; SC\test_session_cards.py:740 fakes a journal.jsonl). wake.py has its own suffixes `.wake.json` and `.wake-crash.json` (I2-060, not mine).

## I2-025 / I2-030 Evidence line and parked note
Homes: stall.py:151-242 (owner; TOOL_TAIL = 5 at :34, "tools:" line :207); model-postures.md:11 "an EVIDENCE line under it with the agent's call counts, its last five tools and its new commits"; memory relay-status-in-plain-english.md:15 "(call counts, last five tools, new commits)"; G\README.md:29,31,41 describe the line in prose without the number. Parked wording " | parked since %H:%M, waiting on its own background work" is only in stall.py; README:41 says "its evidence line says since when it is parked". Kinds: K1 ("five" in three homes). Test: test_stall.py exercises the lines; nothing compares model-postures.md's "five".

## I2-018 / I2-047 What counts as running, parked, finished
Homes: stall.running_agents (stall.py:74-77, "by the `finished` field agent_stop.py writes"), used by wake.py:42-45; agent_stop.py:3-4 and :255-283 (writer of finished, parked, parked_since, verdict_problem); G\README.md:41 (prose of the same rule with the field names); stall.py:105, :134 and :479 test `rec.get("finished")` again inline instead of calling running_agents. Kinds: K6-light (one helper, three inline copies in its own file), K1 (README). Tests: test_stall.py, test_wake.py, test_agent_stop.py build records.

## I2-054 Run log agent_stop.jsonl
Homes: agent_stop.py:235-237,290 (writer); G\README.md:75 ("one line per agent stop"); cleanup_sweep.ps1:55,549-550,584 (30-day prune, file name restated). Kind: K1 (file name in three files). No test compares them.

## I2-058 Exit codes of the wake watch
wake.py:22-24 (owner: WAKE = 2, QUIET = 0, UNHEARD = 1; comment says exit 2 wakes the session). settings.fragment.json:236 `asyncRewake`; wake.py docstring points to tests\probe_results.md "Wake and ..." as the evidence. README:31 describes waking without the digit. test_wake.py asserts wake.WAKE by name. One real home.

## I2-073 Event name of the posture reply
posture.py:51 picks "SessionStart" or "UserPromptSubmit"; settings.fragment.json registers posture at SessionStart matcher "compact" (about lines 109-113) and prompt.py at UserPromptSubmit (:124-129); the event name is also passed to common.context. Kind: K1 (event names in the settings and in posture.py agree today). posture.py docstring: sent on the first prompt, every RESEND_MINUTES (60), and after a compaction. Main vs agent: the posture goes to the main session only.

## I2-077 / I2-080 prompt.py join and input
`JOIN = "\n\n"` and `json.dumps(data).encode("ascii")` have one home (prompt.py:22,40,53-62); README describes prompt.py's joining in prose without the separator. Test: test_posture.py covers parts. One home.

## I2-083 Waiver record fields
Homes: waiver.py:43-44 (allow lines: kind, prefix for a waiver phrase, prompt_id, transcript_path, session_id, time); percall.py:25-40 writes a second line shape into the same file, `{"kind": kind+"-used", "prompt_id", "time", "claim"}`, and reads session_id, transcript_path and prompt_id of the first; review_rules.py:127 reads prompt_id and transcript_path; common.py:250-255 owns the file name, with no field list (unlike REVIEW_KINDS). Test: test_common.py:241-248. Kinds: K6-light (two writers, two shapes, no field list), K1.

## I2-110 Claim file
One home: session_cards_start.ps1:43 `$ClaimHours = 2`, :199-235 (claim.txt: session id, tab, UTC time; the future-time rule). Searched gates, tools/session_cards, agents, CLAUDE.md, Hydra docs/agents: no other statement. Test: test_h53_session_cards_start.ps1:102 builds claim.txt. Hydra CLAUDE.md "Past sessions" points at "the hook's message" (a pointer). No kind.

## I3-022 Logged refusal event names "push" and "ref"
ref_judge.py:416-418 logs event "push" or "ref" via common.log_refusal; publish_gate.py:130-131 logs event "publish" the same way; common.run logs hook_event_name for others. README:79 names the two hooks that log themselves (no value). Kind: K1. No test names those event values in the log.

## I3-031 Real-app record file name and fields
Homes: H\tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` and :182-198 realapp_record (kind, commit, dirty, commands, shots, started, stopped); G\review_rules.py:48-49 `REALAPP_FILE = "realapp.jsonl"` with the comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them." and reads kind, dirty, commands >= 1, commit at :385; hydra_ui_mcp.py:78-80 comment "The hooks repo's ref gate reads it, so its field names are a contract"; G\README.md:55 and H\docs\agents\ui-testing.md:237 describe it in prose. test_docs_numbers.py:54 README_NAMES requires README to name realapp.jsonl.
Kinds: K1 (the name is a constant in two code files, not an import), K8-light (the "owns" claim is true for the writer; the reader holds its own copy). No test holds the two constants equal.
Worktree vs main: the file sits in the git common dir (hydra_ui_mcp.py:183-186), read the same way at review_rules.py:378.

## I3-035 Marker `gate:` line grammar
Homes: review_rules.py:58-59,157-177 (owner; a bad line raises; marker must be plain UTF-8); G\.derive-once-gate (comment "gates/review_rules.py reads these lines; see gates/README.md"); G\README.md:138; H\.derive-once-gate (no `gate:` lines): "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file."; review_rules.py:21-22 BUILTIN_CODE is what applies to a marker with no gate lines.
K8 candidate: the Hydra marker names a PowerShell function in `hooks\lib\derive_once_rules.ps1` as the decider. C:\Users\Patrick\.claude\hooks holds only `state`, and a grep for Test-DeriveOnceGatedPath and derive_once_rules across gates, README, Hydra docs/agents and tools found nothing; the decider is review_rules.py (BUILTIN_CODE). test_docs_numbers.py:172 test_readme_names_no_old_hook_as_live covers the gates README only, not this Hydra file.
Also: what counts as code is stated in review_rules.py:22 (BUILTIN_CODE), as prose in G\README.md:55 (UI code under src/ui/) and in derive-once-review.md. K1/K7-light.

## I3-070 / I3-071 / I3-074 Ruling-line shape, headings, pointer file
Homes: ruling_rules.py:95 (headings), :103-105 LINE and SESSION_NOTE, :122-157 parse (uses `cards.CHOSE` and `cards.QUOTE` from the card tools: a reuse), :102 POINTERS, :323-325 entry fields; its docstring :5-8 "LINE, SESSION_NOTE and parse() are the one home of that shape; the quote and chose parts are the session-card tools' QUOTE and CHOSE". SC\common.py:92-100 RULING, RULING_NOTE, QUOTE, CHOSE ("Every script that reads rulings uses these, so the shapes are defined here only"). H\docs\agents\rulings\README.md:16-30,53 describes the headings and the two line shapes with an example (`- **2026-11-13** "..." (session 7d28ae9a)`). "Only a Confirmed line is a ruling" is restated in H\docs\agents\agent-rules.md:35,37, H\CLAUDE.md:41, doc-review.md:63, premise-map.md:45, SC\writer-brief.md:67, SC\common.py:52-53. SC\check.py:144-170 reads card ruling lines with the same QUOTE/CHOSE.
Kinds: K6 (two grammars for the tail after the quote: ruling_rules accepts only `(session <id>)`; RULING_NOTE accepts any parenthesised note with one level of inner parentheses), K1 (README prose vs LINE).
Tests: test_ruling_rules.py; test_docs_numbers.py:129-137 checks Hydra's rulings README holds the approval step. No test compares README prose to LINE.

## I4 rows (SC\writer-brief.md against its checkers)
Edge for every I4 row: card writers skip the repo preamble (agents\card-writer.md:9), so these rules reach them through the brief only; only check.py checks the result afterwards. I name below what check.py or common.py checks and what it does not.
- I4-003 secrets: brief:11. common.py:85-91 SECRET (five token shapes) and SECRET_MASK, applied at common.py:308 to the evidence the writer reads. check.py has no secret check on the finished card (searched check.py for SECRET). K6-light: prose says "secret, token, password or key"; the regex covers five token shapes.
- I4-004 file names: brief:12-13. One home (the "kept locally" hits in check.py/index.py are about local cards). H\CLAUDE.md:25 does not state it.
- I4-005 transcripts: brief:14; H\CLAUDE.md:25 "The condensed transcripts ... are local working files, not a reference, and no doc cites them"; common.py:49-50 comment; check.py docstring "never opens a raw transcript". K1. No check that a card does not name a transcript.
- I4-006 / I4-007 / I4-008 / I4-009 the four content kinds: brief:18-21 cite "The user ruled on four kinds of content on 2026-10-31". common.py:266 comment names the ruling for local sessions; common.py:274 LOCAL_FIELD and check.py (end of the check function) LOCAL_VALUES enforce the `local:` header value. Money/token totals, security wording and game-audio wording: brief only, nothing checks them. I did not confirm a Confirmed line for that ruling (only two topic files read). Kind: K7 for the three unchecked; I4-006 K4: the writer is asked to decide whether a session is about the project when unsure ("mark it local: yes and name it ... so the main session decides").
- I4-012 evidence files: brief:27; prepass.py:7-10 writes `<id>.condensed.md`, `<id>.user_text.json`, `<id>.meta.json`; check.py reads user_text.json and meta.json. K1. No test compares the brief to prepass.
- I4-019 card shape: brief:37-65; common.py:51 HEADINGS; check.py:116-120 header keys (session, project, commits, D, tags, agents); check.py:30 MAX_CARD_LINES = 60 (a cap stated in code; the brief's own mention of it I did not search). K1 across three files. Test: SC\test_session_cards.py exercises the checker.
- I4-020 roadblock shape: brief:47-49,80; check.py:32 ROADBLOCK_ENDINGS ("fixed", "worked around", "dead end"); check.py:185-191 requires at least three parts split on " -> "; brief says "at least two ` -> ` separators" (same rule, counted differently). CATCHERS at check.py:33-37 ("main session", "hook notice", "user") with a comment that the writer brief says which lines carry a catcher. K1.
- I4-021 user's-words lines: brief:53-55; common.py:95-101 RULING, QUOTE, CHOSE; check.py:141-170. Split (own line): brief:75 says text in parentheses after a quote may not contain `(` or `)`; common.py:98-100 RULING_NOTE accepts one level of inner parentheses (comment: "a note nested deeper does not match"). K2.
- I4-022 heading name: brief:67; common.py:51-57 WORDS_HEADING, OLD_WORDS_HEADING "Rulings" ("a card may carry either, never both"); H\CLAUDE.md:25. K1, three homes agree.
- I4-023 repeats after compaction: brief:69; common.py COMPACTION_LABEL and CONTINUED text ("This session is being continued from a previous conversation") mark the repeat in the evidence. The rule itself is only in the brief.
- I4-025 / I4-026 exact quotes, chose lines: brief:75-76; check.py:141-170 (quote through common.is_users_words, chose by label in `chosen`, question fragments split on "..."); ruling_rules.py uses the same is_users_words ("the one home of that test"). K1.
- I4-027 tag rules: brief:77 "Use 2 to 6"; check.py:136-138 checks only membership in tags.txt (I found no count check in check.py, common.py or index.py); tags.txt is built by common.build_tags (common.py:554-561) from CONTEXT.md plus SC\hand_tags.txt. K7: a count rule stated only in the brief.
- I4-028 D-numbers: brief:78; check.py:139-173 (every D must have a `**D<n>` entry via DECISION_ENTRY, common.py:92). The brief's "D115 is a name with no entry" is a hand-typed list. The decision log (H\docs\audit\2026-10-03-fix-decisions.md:1773) uses "D115" as a name in prose; a grep for `^\*\*D115` in that file found nothing. The checker already fails any D-number with no entry, so the brief's hand list is a second home of what the check derives (K1/K3).
- I4-029 commit hashes: brief:79; common.py:78 HEX `{7,40}`; check.py:174-184 (header commits and hex in Roadblocks/Outcome must exist in a repo). K6 with the hash-shape section above.
- I4-030 roadblock ending: brief:80; check.py:32,185-191. K1 (the same three words); "cause not found" is in the brief only.
- I4-032 write-back: brief:81,88; check.py:38-43,193-198 WRITE_BACK_PAGES "docs/clone-hero/", WRITE_BACK_TAG "clone-hero"; agents\card-writer.md:9 tells the writer to read the Clone Hero README for the page list; H\CLAUDE.md:25 describes `[write-back]`; index.py reads the line. K1. Worktree vs main: the brief fixes the main checkout path of the README, so a writer reads main's page list.

## Rows with one home (what I searched)
I1-093, I2-015, I2-030, I2-032, I2-035, I2-039, I2-058, I2-077, I2-080, I2-110, I4-004, I4-023. For each I searched the whole scope list by the quoted value and by the question in other words (launch and journal names, trouble order, claim file and hours, join separator, how to name files in a card, what to do with a repeat) and found no second home besides those named above.

## Side notes (outside my rows)
- G\README.md:83: `ref_judge.py --selftest` writes one fake deny line into the real refusals.jsonl (any count of refusals includes it).
- H\.derive-once-gate comment names `hooks\lib\derive_once_rules.ps1`, which does not exist (see I3-035).
- G\tests\test_docs_numbers.py DOCS (CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md) leaves out SC\writer-brief.md, G\README.md, model-postures.md, agents\*.md and memory notes, which also state "5 minutes", "100 tool calls and 20 minutes" (model-postures.md:5) and similar. I did not check each for numbers.
- Hook gap (general words only): the status helper only needs that some agent record exists for the file name it is given, so the id is not tied to the caller.
- Two spellings of the status call: writer-brief.md:100 shows the id unquoted, progress.py and the preamble quote it; both pass progress.is_status_call.
