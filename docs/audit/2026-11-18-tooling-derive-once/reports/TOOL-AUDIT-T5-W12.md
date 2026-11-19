# TOOL-AUDIT-T5-W12 (second-round verifier, ledger TOOL-AUDIT-T5-L08, 22 rows)

Short paths: G = C:\Users\Patrick\.claude\gates; SC = C:\Users\Patrick\.claude\tools\session_cards; HR = C:\Users\Patrick\Downloads\Hydra\hydra-test; WB = SC\writer-brief.md; MP = C:\Users\Patrick\.claude\model-postures.md; AG = C:\Users\Patrick\.claude\agents. All claims are "by reading"; I ran nothing.
Rulings folder note (applies to every row): I searched every file in HR\docs\agents\rulings\ for lines dated 2026-10-31 and for the words "kept locally", "money", "dollar", "defender", "game audio", "not about the project". No `## Confirmed` line names any card-content rule, any hash length, any status form beyond the lines quoted in the ledger. Where a row says "Ruling: none found", that is the search.

### L-F22-07
**Verdict:** CONFIRMED-CHANGED (it is three record shapes, not two).
**Homes as found:** G\waiver.py:22-31 `_record` returns `{"kind", "prefix"}` for a review waiver, `{"kind":"library-run"}` or `{"kind":"key-run"}` (no prefix) for an allow; waiver.py:43-44 adds `prompt_id`, `transcript_path`, `session_id`, `time`. G\percall.py:31-38 writes a third shape `{"kind": kind+"-used", "prompt_id", "time", "claim"}`. G\review_rules.py:116-122 `_waiver_records` reads `prefix` and requires it to be 7+ lowercase hex that the key starts with. G\common.py:250-255 `append_waiver`/`read_waivers` name the file and no fields (REVIEW_KINDS exists for reviews only, common.py:11).
**Splits:**
- Which fields a waivers.jsonl line has: waiver (prefix, no claim), allow (no prefix), used (claim, no transcript_path/session_id). Each reader (review_rules, percall) reads only its own kind, so they differ only for a reader that assumed one shape for all lines. No file lists the three shapes.
**Ruling:** none found.
**Open:** none.

### L-F22-11
**Verdict:** CONFIRMED.
**Homes as found:** WB:16 "The user ruled on four kinds of content on 2026-10-31." with the four bullets (local; no money or token totals; security vague; game audio reword) at WB:18-21. SC\common.py:266 docstring "The user ruled on 2026-11-... " style comment for the local kind; common.py:274-275 `LOCAL_FIELD = "local"`, `LOCAL_VALUES = ("yes","no")`; SC\check.py:199-201 checks only the header value. Money, token totals, security wording and game-audio wording have no check in check.py (grep of SC\*.py for these words found only the SECRET regex, common.py:85-91, which is a different question: secrets in text). Deciding "about the project" is left to the writer (WB:18).
**Splits:** none by reading.
**Ruling:** none found. The four 2026-10-31 lines in the rulings folder (clone-hero.md:37, :80, :81; questions-and-audits.md:10) are about other matters; no Confirmed line is about card content. The ledger's question (whether the ruling has a file and line) is answered: no Confirmed home.
**Open:** whether the 2026-10-31 ruling sits on a session card (not searched; cards are not the rulings folder).

### L-F22-15
**Verdict:** CONFIRMED.
**Homes as found:** G\common.py:35-49 `log_refusal` (owner; `reason[:300]` at :46; file name "refusals.jsonl" :43; eight fields time, hook, event, tool, session, agent, decision, reason). Callers: common.py:71 (`run`, event from the hook input), G\publish_gate.py:130 with `hook_event_name: "publish"`, G\ref_judge.py:416 with `"push"` or `"ref"` (so the README:79 claim that those two call it is true). File-name copies: cleanup_sweep.ps1:55,549,584 ("refusals.jsonl", "agent_stop.jsonl"); stall.py:280 names `agent_stop.jsonl`; agent_stop.py:290,292. The 300: common.py:46 (reason cut); progress.py:31 `QUOTE_CHARS = 300` (a quoted command, a different question); ref_judge.py:57 `err[:300]` (a git error text, a third use); stall.py:39 `EMPTY_FLEET_SECONDS = 300` (unrelated seconds).
**Splits:**
- Refusal-reason cut (300) in common.py:46 against the quoted-command cut in progress.py:31: they answer different questions and differ for a command or reason longer than 300 characters; a change to one would not move the other.
**Ruling:** none found.
**Open:** none.

### L-F22-16
**Verdict:** CONFIRMED-CHANGED (the README's "the one place the fields are checked" is narrower than it reads).
**Homes as found:** G\common.py:11-12 `REVIEW_KINDS`: handover (tip, branches, session, prompt_sha256, time, cwd), verdict (key, verdict, reviewer, session, time). common.py:239-243 `append_review` raises only when a listed field is missing; extra fields pass. G\review_dispatch.py:170-173 writes exactly the six handover fields. G\agent_stop.py:211-214 writes a verdict with an extra `path` (document key) that REVIEW_KINDS does not list. Readers: review_rules.py:190 reads `key`, `verdict`; review_rules.py:257 and publish_gate.py:83 read `path`. G\README.md:75 "REVIEW_KINDS in common.py is the one place the review record fields are checked." The message "Recorded reviewer %s's %s verdict for %s." is at agent_stop.py:228 only.
**Splits:**
- `path` on a verdict record: written by agent_stop and read by review_rules and publish_gate, listed nowhere; a record without it is a commit verdict, with it a document verdict. The field list and the readers differ for document verdicts.
**Ruling:** none found.
**Open:** none.

### L-F22-17
**Verdict:** CONFIRMED.
**Homes as found:** G\common.py:188-206 `update_agent` ("The one way a hook changes an agent's record", under a lock); common.py:136-137 `write_agent` writes with no lock and its comment says a hook that changes a record calls update_agent. Writers by grep over G\*.py and *.ps1 (tests excluded): progress.py:103 (first, last, calls, warned_calls, warned_minutes, finished, status_calls, drops parked/parked_since), agent_stop.py:284 (finished, parked, parked_since, verdict_problem), agent_stop.py:~323-357 (verdict_problem). No non-test caller of `write_agent` found. Readers: stall.py:54-68 `_session_agents`, stall.py:105,134,479, wake.py via stall.running_agents; status_append.ps1:14,27-37 reads only the file name under `agents\`. The handback file shares the folder (agent_stop.py:41; written by percall.py:104 with plain write_json_atomic, outside update_agent, and it is not the record). README.md:75 gives the path, the lock file and no field list.
**Splits:** none. No file lists the record's fields; they sit in progress.py:74-75 and agent_stop.py only.
**Ruling:** none found.
**Open:** none.

### L-F22-18
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:38 `USAGE_FIELDS = ("input_tokens","cache_read_input_tokens","cache_creation_input_tokens")`, used stall.py:366 inside `context_tokens` (docstring :350-351 "the three input counts added up"). HR\tools\agent_cost.py:63-65 reads the same three names as bare literals (`fresh`, `read`, `written`) and sums them again at the long-context test (`fresh + read + written > limit`). Grep for `cache_read_input_tokens` over G, SC and HR\tools found only stall.py, agent_cost.py and their tests.
**Splits:** none today. They answer two questions (context size to warn at 500K; dollars for one request) from the same three fields; two repos, no shared constant.
**Ruling:** none found.
**Open:** none.

### L-F22-19
**Verdict:** CONFIRMED.
**Homes as found:** G\stall.py:74-77 `running_agents` (`not rec.get("finished")`), used by G\wake.py:45. The same test typed inline at stall.py:105 (silence notice), :134 (time-limit notice), :479 (evidence lines). Writer: agent_stop.py:269-276 sets `finished` False and `parked` True for an agent parked on background work, True otherwise; progress.py:75 starts it False. G\README.md:41 says it in prose.
**Splits:** none by reading: all four tests are the same expression, and a record with no `finished` key counts as running in all. Where it would differ: a future change to the rule (for example a "parked" third state) made in one spot only.
**Ruling:** none found.
**Open:** none.

### L-F22-20
**Verdict:** CONFIRMED-CHANGED (the "30,000" has no home in the gates).
**Homes as found:** G\shell_output.py:14-15 `CAP = 4000`, HEAD/TAIL 1500; :30-32 `MARKER_START`, `FAILED_LINE = "COMMAND FAILED, exit code %d"`, `TRIMMED_LINE = "[trimmed: %d characters, full text at %s]"`; shell_output.py:182 comment names "Claude Code's ceiling" with no number. HR\docs\agents\brief-preamble.md (the shell-output paragraph, line 35) quotes "4,000", "30,000", the failed line and "never end one with your own `exit N`". G\README.md:25 carries the exit-N rule; `grep -c "30,000"` in G\README.md and a grep of G\*.py, *.md found no other copy of 30,000, so that number is Claude Code's own and stands in the preamble only. G\tests\test_docs_numbers.py:198-203 holds the preamble to `shell_output.CAP` (4,000); nothing holds it to the failed line text or to 30,000.
**Splits:**
- 30,000 (preamble) against no number in the gate: the gate cannot disagree, because it never reads it; if Claude Code changes its ceiling, only the preamble line goes stale.
**Ruling:** none found.
**Open:** whether 30,000 is Claude Code's real ceiling (an outside fact a run or its docs would settle).

### L-F22-21
**Verdict:** CONFIRMED-CHANGED (more pages carry the numbers than the finder listed; all agree with progress.py today).
**Homes as found:** G\tests\test_docs_numbers.py:28-30 `DOCS` = CLAUDE.md, agent-rules.md, brief-preamble.md, derive-once-review.md, doc-review.md. Owner G\progress.py:14-16: CALL_WARN/CALL_STOP 100/150, MINUTES_WARN/MINUTES_STOP 20/30, STATUS_NAG/DENY 5/10. Pages stating a number and outside DOCS: MP:5 ("100 tool calls and 20 minutes", the warn pair, used as a task-size limit); WB:100-101 ("every 5 minutes", "At 20 minutes or 100 tool calls, stop starting new cards"); G\README.md:124 (5 and 10 minutes); AG\reviewer.md:11 ("at 100 calls or 20 minutes"); the memory note every-brief-carries-status-rule.md and relay-status-in-plain-english.md ("5 minutes"). The README is read by the test for names only (test_docs_numbers.py:54, :177-180), not numbers.
**Splits:** none today; all agree. WB and reviewer.md use the warn pair (20 min / 100 calls) as a stop-starting-new-work point, while progress.py refuses at 30 / 150; that is a meaning, not a number, difference.
**Ruling:** none found (the 2026-10-31 "not too concerned about the exact number" line named in the ledger is general; not read as covering this).
**Open:** none.

### L-F22-22
**Verdict:** CONFIRMED-CHANGED (the helper alone does not tie the id, but progress.py does for subagent calls).
**Homes as found:** G\status_append.ps1:27-37: a target in the status folder is accepted when any `agents\<session>\<name>.json` exists, in any session; it never learns who is calling. G\progress.py:46-49 `status_call_id`; progress.py:~118-122 (in `check`): a status call naming an id other than the caller's agent id is refused and not counted ("This status call names %s, but your agent id is %s"). G\percall.py:138 runs `progress.check` for every call with an `agent_id`, of every agent type; the main session (no agent_id) takes the other branch (percall.py:197) and is not tied to any id. status_append.ps1:20-22 also takes a `.md` path outside the status folder with no id check.
**Splits:**
- Id tied to caller: tied by progress.py for a subagent's exact status call; not tied by the helper itself, for a call it receives from a place progress.py does not see (the main session, or the helper run from another script).
**Ruling:** agents-and-workflow.md:25 (status lines matter) does not name this split.
**Open:** none. Hook gap: yes, G\status_append.ps1 alone does not tie the id to the caller; the subagent-side tie lives in progress.py.

### L-F22-23
**Verdict:** CONFIRMED-CHANGED (the part-count wording does not disagree).
**Homes as found:** SC\check.py:32 `ROADBLOCK_ENDINGS = ("fixed","worked around","dead end")`; check.py:185-191 splits on " -> " and requires `len(parts) >= 3`, the line to start "- ", the last part to start with an ending; check.py:33-37 `CATCHERS = ("main session","hook notice","user")`, check.py:30 `MAX_CARD_LINES = 60`. WB:47-50 the three line forms; WB:80 "at least two ` -> ` separators" (two separators = three parts, the same rule); WB:91 lists the three catchers and an example. "cause not found" appears in WB:80 only; check.py accepts any middle text. test_session_cards.py:480-481 copies the example as a literal.
**Splits:** none. The ledger's "counted differently" is the same rule: at least two separators is at least three parts.
**Ruling:** none found. The CATCHERS comment (check.py:34-36) cites a 2026-11-16 answer in a supervisor plan, which I did not open.
**Open:** none.

### L-F22-24
**Verdict:** CONFIRMED-CHANGED (a real, mild split between two statements about the brief).
**Homes as found:** MP:11 "A brief names no status file and no id; it carries only the preamble's status rule." Memory note every-brief-carries-status-rule.md ("How to apply"): every Agent prompt "tells the agent to write a status line every 5 minutes with the helper `C:\Users\Patrick\.claude\gates\status_append.ps1`", and "No gate checks a brief for this sentence today". WB:100 names the helper path and `<your agent id>`. HR docs\agents\brief-preamble.md (Status lines) and G\progress.py:60-62 give the id and the helper call at the agent's first call, so the hook delivers both. The T5-verify2 brief of this task says "Status lines as the preamble says" and names neither.
**Splits:**
- Does a brief name the helper: MP:11 says no; the memory note and WB:100 say yes. They differ for any brief writer who follows one page.
- Who gives the agent id: WB:100 asks for `<your agent id>` in the card-writer's brief; the preamble and progress.py:60-62 say the hook gives it at the first call.
**Ruling:** agents-and-workflow.md:25 ("i don't care if an agent is read only, no status updates is unacceptable") is about the rule itself, not this split. A memory note is not a ruling.
**Open:** none.

### L-F23-09
**Verdict:** CONFIRMED-CHANGED (the log has many entry shapes, not two; and a second D-number reader does not use the ceiling).
**Homes as found:** HR\docs\audit\2026-10-03-fix-decisions.md: 213-plus lines of `**D<n> (session ...` from number_rulings.py:104, but earlier entries open with other parentheses (`**D4 (finding`, `**D35 (ready`, `**D36 (extreme`, `**D44 (`, `**D49 (developer`, `**D88 (mimalloc`...). SC\common.py:92 `DECISION_ENTRY = ^\*\*D(\d+)\b` (reads the number only; used check.py:72, number_rulings.py:86). SC\common.py:80 `DNUM = \bD(\d{1,4})\b`. SC\number_rulings.py:62-64 keeps only numbers below 1000 when scanning for the highest ("MSVC error codes such as D8003"). SC\check.py:170 `dnums += ["D"+n for n in DNUM.findall(meta_txt)]` has no ceiling, then each must have a log entry (check.py:139-173 region). SC\number_rulings.py:30 `FIRST_NUMBERED_DAY = "2026-10-03"`; common.py:44 `DECISIONS_LOG` file name carries the same date.
**Splits:**
- Ceiling on D-numbers: number_rulings.py drops 1000 and above when finding the highest used number; check.py:170 treats any `D` plus 1-4 digits in a ruling line's note as a decision number needing a log entry. They differ for a note naming D8003 (an MSVC code): the writer would be told it has no log entry. This split the ledger did not list.
- Entry shape: many parenthetical shapes after the number; no consumer reads them.
**Ruling:** none found.
**Open:** whether a real card note ever holds a D1000+ token (a run over the cards would settle it).

### L-F23-13
**Verdict:** CONFIRMED-CHANGED (four sub-claims; three hold as stated, the `unsent` one is narrower).
**Homes as found:** (a) SC\link_memory.py:24 `INDEX_FILE = "MEMORY.md"` against SC\common.py:42 `INDEX_FILE = "index.md"`: same constant name, two files (memory index; cards index); never imported into one module (link_memory.py:71, check.py:224, index.py:65, migrate.py:88,99). (b) WB:74-75 "Never quote ... anything in the `unsent` list"; SC\common.py:324-328 `users_words` builds the checkable set from `messages` and the typed parts of `answers` only; SC\prepass.py:110,173 builds `unsent` separately; check.py:155-157 accepts a quote that sits inside one of those. So an unsent text is refused unless it also appears inside a sent message. (c) G\session_cards_start.ps1:44-48 `$HookBudgetSec = 25` with the comment "registration's 30 s timeout, less 5 s"; G\settings.fragment.json:93 `"timeout": 30` is that registration. (d) Index marks: SC\index.py:50 builds " [stale]" and " [failed]", :51 " [write-back]"; only [write-back] is named elsewhere (HR CLAUDE.md:29). The checks in SC\tests were read second-hand by F23; I did not open them.
**Splits:**
- `unsent`: the brief forbids quoting the list; the check forbids only text found in no message. They differ for an unsent text that also appears inside a sent message.
- The 25 s budget is a hand copy of 30 minus 5; if the registration's timeout changes the script does not follow.
**Ruling:** none found.
**Open:** none.

### L-F23-14
**Verdict:** REFUTED as a hook gap (the card-writer is counted by the progress hook); the prose point stands.
**Homes as found:** AG\card-writer.md:9 "That brief replaces the repo's `docs/agents/brief-preamble.md` for you ... skip the preamble." G\percall.py:1-3 and :138 route every call that carries an `agent_id` into `progress.check`; the only agent-type test in percall.py is :104 (the reviewer hand-back), so the call budget (100 warn / 150 stop), 20/30 minute limit and 5/10 minute status deadline apply to a card-writer as to any agent. progress.py:60-62 gives the id at the first call. WB:100-101 copies the status rule and the stop pair as prose. The preamble's other rules are not delivered to a card-writer in prose: no Windows-shell rules, no commit rule, no output-trim notice (the hooks apply them anyway: percall.py:167 media rule, shell_output rewrite).
**Splits:**
- Preamble prose for a card-writer: WB:100-101 covers only the status and stop rules; the remainder is enforced by hooks only. They differ for any rule with no hook behind it (the preamble names those as instructions only).
**Ruling:** none found.
**Open:** none.

### L-F24-05
**Verdict:** CONFIRMED.
**Homes as found:** HR\docs\agents\premise-map.md:42 quotes the four labels; HR\docs\clone-hero\README.md:23-26 defines them (the strings "Proven in the game's code", "Proven in a live run", "Hydra assumes it, never checked", "Unknown") and :118 gives the template "exactly one of the four"; :55 table headers repeat them. HR\tests\test_docs_match_code.cpp:711-713 `kAssumedLabel` and `kLabels[]` hold the same four, and :922 checks each Clone Hero block's Label against them. All four strings agree word for word.
**Splits:** none today. A third copy (the test's) is held to the Clone Hero blocks, not to premise-map.md:42 or the README prose.
**Ruling:** none found.
**Open:** none.

### L-F24-06
**Verdict:** CONFIRMED.
**Homes as found:** Rule "source for the exact thing": HR\docs\agents\premise-map.md:29; HR\docs\agents\derive-once-review.md:36 (kind H) and :53 (step 10, "the exact thing its owner worked it out for"). Neither page points at the other (grep of both for the other's name: no hit). Rule "moves or converts a quantity": HR\docs\agents\doc-review.md:63 (Plans paragraph, inside "Step 2: trace and recompute", line 47-66), which cites derive-once-review.md step 10; AG\planner.md:15 restates it and points to premise-map.md and "Step 2: trace and recompute" on doc-review.md. premise-map.md has no sentence on "live part" arithmetic (its live-step text is lines 54-70, about the call list). So the pointer in planner.md:15 to premise-map.md is for the premise rules, and to doc-review.md for this one.
**Splits:** none. Two rules, each stated at its home with at most one restatement.
**Ruling:** none found.
**Open:** none.

### L-F24-09
**Verdict:** CONFIRMED-CHANGED (the combined case is the question the ledger missed).
**Homes as found:** HR\docs\agents\derive-once-review.md:126-151 eight bullets: Questions this change answers, Findings, Visible changes, Precheck lines dropped, Notes, Touched already on the fix list, Scan rows added, Where I looked. HR\docs\agents\doc-review.md:104-125 six: Restatement, Findings, Numbers checked, References opened, Notes, Where I looked. AG\reviewer.md:20-26 names none and points at the two pages. G\agent_stop.py reads only the sign-off line (agent_stop.py:102-109).
**Splits:**
- Report sections when one reviewer reviews both a range's code and its documents: doc-review.md:123 says the same one line signs both, "as derive-once-review.md says"; neither page says which section list applies or whether both do. Three sections (Findings, Notes, Where I looked) are named in both lists; Restatement, Numbers checked, References opened exist only in the doc list; the other five only in the code list.
**Ruling:** none found.
**Open:** none.

### L-F24-10
**Verdict:** CONFIRMED-CHANGED (several small splits, all between the table mode and the scrape mode).
**Homes as found:** HR\docs\agents\handoff.md:11-30 (table, seven kinds: commit, branch, path, function, test, ruling, state), :32-41 rules ("all 40 characters"), :51-57 status words and exit rule, :59 scrape mode ("A hex word is a commit"). HR\tools\check_handoff.py:28 `_FULL_HASH = ^[0-9a-fA-F]{40}$`; :31-33 `CLAIMS_HEADING`, `MANUAL_KINDS`, `TABLE_STATUSES`; :353-365 `row_status` (manual kinds, commit, then path/branch/function, else "unknown"); :376-393 table summary and exit (`0` only if every status other than ok and manual is zero, so "unknown" also gives 1); :316 scrape exit (`1 if missing else 0`: "CHANGED SINCE" does not change it). Argument errors go through `ap.error` (:428-439), which exits 2. HR\tools\doc_tokens.py:39 `_HEX = ^[0-9a-fA-F]{7,40}$`, :50 `_BARE_HEX` lower case only, 7-40. HR\tools\test_check_handoff.py:294 holds a row list ending in "unknown". HR\.claude\commands\handoff.md:7-23 steps agree with handoff.md.
**Splits:**
- Hash length: table mode takes exactly 40 (a shorter one is "missing"); scrape mode takes 7 to 40 for any hex word. They differ for a short hash in a handoff with a table versus one without.
- Hash case: check_handoff.py:28 and doc_tokens `_HEX` accept upper case; `_BARE_HEX` and the doc do not mention it.
- A seventh status word "unknown" (kind not among the seven) is printed and gives exit 1; handoff.md:51-57 names five statuses and no "unknown".
- Exit rule: handoff.md:57 states exit 0/1/2 for table rows; scrape mode's exit ignores changed-since lines.
**Ruling:** none found.
**Open:** none.

### L-F24-11
**Verdict:** CONFIRMED.
**Homes as found:** HR\docs\agents\audit-method.md:50 (finders report "each home ... and a table of inputs and answers for each home, with the edges in. Edges are values like an empty list, exactly 0, a value right at a window edge, or a missing optional. A finder does not grade its own candidates."). HR\worktrees\tooling-audit-records\docs\audit\2026-11-18-tooling-derive-once\briefs\T5-finder.md:33-41: a table of "what each home tells its reader to do" with edges "exactly at the limit, the main session against an agent, an agent type that skips the preamble (card writers), and a worktree against the main checkout", plus kinds, splits as own lines, rulings, tests; "Do not grade candidates and pick no exit." The T5 brief does not name audit-method.md (grep: no hit).
**Splits:**
- Table content and edge list: audit-method.md asks inputs and answers with value edges; the T5 brief asks what each home tells its reader and names situation edges. They differ for a finder who follows only one: the first would test 0 and the window edge, the second the agent type and the checkout.
**Ruling:** none found.
**Open:** none.

### L-F24-13
**Verdict:** CONFIRMED.
**Homes as found:** HR\docs\agents\brief-preamble.md:125-127 "Your brief says what to return. If it does not, return: your branch and worktree ... commit hashes, summary, test commands with pass counts, numbers measured, anything you stopped on." HR\docs\agents\derive-once-review.md:126-151 (eight sections); doc-review.md:104-125 (six); WB:103-105 ("The card files you wrote, the session ids you did not reach ... Nothing else."); AG\reviewer.md:24-26 (the sign-off). T5-verify2.md (this task's brief) asks only for a file and a five-line summary.
**Splits:** none by reading. Each brief that names its own return list overrides the preamble default by the preamble's own words; the review pages and the card writer's brief are such briefs.
**Ruling:** none found.
**Open:** none.

### L-F24-15
**Verdict:** CONFIRMED-CHANGED (the first sentence and the onFailure entries match; "every Python hook" is wider than the code).
**Homes as found:** G\README.md:51 (verdict_problem in the log line and the agent record, which stall.py relays) against G\agent_stop.py:13 (docstring), :109 `_UNFINISHED` note, :323-325, :355-357 (set and pop) and G\stall.py:268-273 `_verdict_problems`: they agree, including the "started and had not finished" note and the clearing on a later clean stop. G\README.md:93-104 against G\common.py:52-70 `run(body, mode...)`: "closed" denies on a crash, "loud" prints a systemMessage, always returns 0. G\settings.fragment.json has three `onFailure: block` entries (:153 percall.py, :169 review_dispatch.py, :185 publish_gate.py); these are the registered hooks that can refuse (their __main__ use "closed": percall.py:210, review_dispatch.py:190, publish_gate.py:135). The "loud" hooks (agent_stop, posture, prompt, waiver, shell_output) carry none. delete_rule.py:187 is "closed" too but is called inside percall.py, not registered itself. prompt.py:19 runs percall as a "closed" part inside a "loud" hook and turns a refusal into a message, so a prompt is never refused (prompt.py:46-48).
**Splits:**
- README:93 "Every Python hook works the same way ... common.run does all of that": G\wake.py:90-110 (`main`) reads stdin itself, writes to stderr and exits 2 on purpose (WAKE = 2, wake.py:22), and G\ref_judge.py:492-538 is a git hook with its own contract; neither calls common.run. The README describes wake.py separately (README:31) but the "every" sentence is not true of it.
**Ruling:** agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" relates to "loud"; it names neither side of this.
**Open:** none.

## Ids not reached
None. All 22 rows were read.

## Counts
CONFIRMED = L-F22-11, L-F22-15, L-F22-17, L-F22-18, L-F22-19, L-F24-05, L-F24-06, L-F24-11, L-F24-13 (9). CONFIRMED-CHANGED = L-F22-07, L-F22-16, L-F22-20, L-F22-21, L-F22-22, L-F22-23, L-F22-24, L-F23-09, L-F23-13, L-F24-09, L-F24-10, L-F24-15 (12). REFUTED = L-F23-14 (1). Total 22.
