# TOOL-AUDIT-T5-F27 (finder F27, 55 rows)

Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, ST = C:\Users\Patrick\.claude\tools\session_cards.
Method: each row's question searched by meaning in the whole scope with grep/sed. Depth is uneven (calls ran out); each row says what was searched. No hook test, build or Hydra test program was run. Nothing graded.

Rulings read first: plan 2026-10-25-hooks-from-scratch.md:108 "Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of progress.py and nowhere else in code; the docs quote them...". Confirmed lines that touch my rows: review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (session 4672d33d); review-and-hooks.md:34 "demand status updates every 5 minutes of real time and reject any calls with no updates after 10 minutes" (session 48cdbb34); agents-and-workflow.md:25 "i don't care if an agent is read only, no status updates is unacceptable" (session e81b56eb). (Line 34 and 25 are under `## Confirmed`; Unconfirmed starts at review-and-hooks.md:37 and agents-and-workflow.md:32.)

Edge columns used below: LIMIT = exactly at the limit; MAIN/AGENT = main session vs agent; CARD = agent type that skips the preamble (card writers); WT = worktree vs main checkout. "n/a" = the home does not speak to that edge.

---------------------------------------------------------------
## Group A: gates state, hand-back, parked, lock

### I9-092 Where is an agent's hand-back saved and who reads it back?
Homes:
- G\percall.py:104 `common.write_json_atomic(agent_stop.handback_path(data["session_id"], data["agent_id"], root), message)` (writer; test_progress.py:388-421 pins it).
- G\agent_stop.py:39 `def handback_path(...)` (the path owner) and :88, :194 read it.
- G\agent_stop.py:8 docstring "percall.py saved (handback_path; gates/README.md says why not the transcript)".
- G\README.md:75 "`<agent_id>.lock` file ... and the agent's last hand-back beside it (`handback_path` in `agent_stop.py`)"; README.md:182 "So `percall.py` saves each `SubagentHandback` call's `message` when the call is made, and `agent_stop.py` reads that saved copy; `handback_path` in `agent_stop.py` names the file."
- G\percall.py REPORT_TOOLS = ("SubagentHandback","StructuredOutput") progress.py:19-20 (which tools count as a hand-back) is a second list next to percall's own check; and Hydra docs derive-once-review.md:128, doc-review.md:106, ~/.claude/agents/reviewer.md:25 each say "Hand it back ... with the SubagentHandback tool when you have it, or as your last message when you do not."
Table: percall/agent_stop code: save at call time, even past budget (README 182 pointer). READMEs/reviewer.md: tell the agent to call the tool or end with a last message. LIMIT: percall saves past the call/time budget (test). MAIN: n/a (hook only sees agents). CARD: agent_stop reads only reviewers; card writers never hand back through a hook-read verdict. WT: path is under G\state, one place for all worktrees.
Kinds: K1 (the path and "why not transcript" are stated in README 75, 182 and the docstring; all point at handback_path, so mostly pointers). K6 candidate: progress.py:19 REPORT_TOOLS and percall's own "SubagentHandback" test. Not verified further.
Splits: none found. Confirmed ruling: review-and-hooks.md:19 (hand-back with no sign-off line records CLEAN).
Tests holding homes equal: test_progress.py:388-421 and test_agent_stop.py read the same path through handback_path; no test compares README to code.

### I9-097 What happens when an agent's record lock is held?
Homes: G\common.py:197-206 (code: "Losing a change silently is the bug this lock fixes, and refusing the call over a lock would punish the agent, so the change still goes in and the output says it may have lost a race."; text "...changed without the lock; a change made at the same moment may be lost."); G\README.md:9 (update_agent holds an OS lock ... hooks that run at once); common.py:15-19 `_LOCK_WAIT, _LOCK_POLL = 2.0, 0.001` with a comment "A holder keeps the lock for one read and one write, a few milliseconds". test_progress.py:807-832 pins "without the lock".
Table: code = still count the call and tell the agent; README = describes the lock only. LIMIT: 2.0 s wait (stated in code and in the test via a wait constant? not checked). MAIN/AGENT/CARD/WT: n/a.
Kinds: single home for behaviour (README 9 is a pointer to update_agent). Searched: common.py, progress.py, README, agents/*.md.
Splits: none. Rulings: none. Test: test_progress.py:807-832 pins the output word.

### I9-098 Is an agent still "parked" after it makes a call?
Homes: G\progress.py:77-79 ("A call means the agent is no longer parked on background work (agent_stop.py sets the flag)." then pops `parked`, `parked_since`); G\agent_stop.py:256-276 (sets `parked`, `parked_since`, and pops both on a finished stop); G\stall.py:238-240 (shows "parked since ..." unless `last > since`, i.e. stall re-derives "a later call ends parking" itself); G\README.md:41 ("with its evidence line says since when it is parked until its next call. Its next stop ... marks it finished and drops both fields.").
Table: progress.py clears on any call; stall.py ALSO treats a call newer than parked_since as not parked (`not (_is_time(last) and last > since)`); agent_stop clears on finish; README describes all three.
Kinds: K6 (the "call ends parking" rule is in two code places, progress.py:77-79 and stall.py:240, plus the README). LIMIT: stall.py uses `last > since` strictly; progress clears on the call itself, so `last == since` cannot happen in practice (not checked). MAIN/CARD/WT: n/a.
Splits: none by value. Rulings: none. Tests: test_progress.py:158-177 (progress side); test_stall.py (stall side) separate; none holds them equal.

---------------------------------------------------------------
## Group B: review handover and document key

### I9-109 What does a handover record hold, and how is the brief hashed?
Homes:
- G\common.py:11 `REVIEW_KINDS = {"handover": ("tip","branches","session","prompt_sha256","time","cwd"), "verdict": (...)}` (field list).
- G\review_dispatch.py:170-172 writes the record `{"kind":"handover","tip":...,"branches":...,"session":...,"prompt_sha256": hashlib.sha256(prompt.encode("utf-8")).hexdigest()...`.
- G\agent_stop.py:199-201 recomputes `sha = hashlib.sha256(first.encode("utf-8")).hexdigest()` from the agent transcript's first user message and finds the handover by `r.get("prompt_sha256") == sha`.
- G\README.md:43 "it finds the handover by the brief's SHA-256".
- Tests pin fixtures: test_review_dispatch.py:26 `BRANCH_COMMIT`, :29 `FIXED_PROMPT_SHA = "7abd310b..."`.
Table: review_dispatch hashes the prompt string it was given; agent_stop hashes the first user message it reads back from a transcript; they must be byte-identical or the stop hook finds no handover. LIMIT/MAIN/WT: handover cwd is the folder the key was judged in (review_dispatch.py:163-165 comment) so a worktree vs main checkout is handled by the record's `cwd`. CARD: n/a.
Kinds: K6 (the hash is computed in two code places by different routes: dispatch from the tool input, stop from the transcript). K1 (field list in common.REVIEW_KINDS and the literal dict in review_dispatch.py).
Splits: none found today. Rulings: none. Test holding them equal: test_agent_stop.py (round trip through both hooks, not read line by line).

### I9-131 How is a document key computed?
Homes:
- G\review_rules.py:65-74 `normalized(raw)` ("a UTF-8 byte-order mark dropped and CRLF made LF") and `doc_key = sha256(normalized(data))`.
- G\publish_gate.py:19-29: its small-text count reads `review_rules.normalized(raw)` then ALSO does `text.replace("\r\n","\n").replace("\r","\n")` (turns lone CR into LF in the count only) and decodes UTF-16 itself.
- G\agent_stop.py:221 and review_rules.py:441/450 call `doc_key`.
- Literal vector d821ef04...: tests\test_review_dispatch.py:23 `DOC_KEY` and tests\test_review_rules.py:333 (same hex typed twice).
- Test row text (I9-131) says "CRs removed"; code removes only CRLF pairs (a lone CR stays in the key). The row's wording and the code differ.
- tests\test_docs_numbers.py:42 TYPED_KEY patterns forbid docs from asking anyone to compute a key.
Kinds: K1 (vector typed in two tests), K6 (publish_gate treats lone CR differently only for its count; the key itself is one owner).
Splits: test text "CRs removed" vs code "CRLF -> LF" (row wording only; the test uses CRLF input).
Rulings: none. Test holding equal: test_publish.py:26 binds `doc_key = real_review_rules.doc_key` (same function, not a copy).

### I9-116 What does the stall evidence line hold; who owns the Agent trailer?
Homes: G\stall.py:210-232 (`_commits_fact`, "import agent_stop  # owns the Agent trailer"); G\agent_stop.py:114 docstring "The one reader of the Agent trailer: stall.py's..." and :117 `--format=...%(trailers:key=Agent,...)`; G\ref_judge.py:182-229 `missing_trailers` and message at :226-229 (requires Task:, Agent:, Session:); Hydra docs\agents\brief-preamble.md:65-67 (`--trailer "Task: <task id>" --trailer "Agent: <your agent id>" --trailer "Session: <session id>"`); G\session_cards_start.ps1 (message tells the card writer's commit step "with the three trailers"; test_h53 pins it); G\tests\test_ref_judge.py:22, test_review_rules.py:21 typed TRAILERS.
Table: ref_judge: refuses commits lacking the three (merge commits exempt per its docstring, user's ruling cited at :184). brief-preamble: tells agents to add all three. session_cards_start.ps1: tells the main session to commit cards with three trailers (CARD edge: the card writers are Haiku agents that skip the preamble, so the trailer instruction reaches them only through the main session). stall: counts only `Agent:`. MAIN/AGENT: main commits as `Agent: orchestrator` (ref_judge.py:228 message says "your agent id, or orchestrator"), so stall's per-agent count never counts the main session. WT: stall looks in cwd repo and HOOKS_REPO only.
Kinds: K1 (trailer set stated in ref_judge, preamble, session_cards_start message, tests), K6 (trailer reading in ref_judge `missing_trailers` and agent_stop `agent_commits` are two code readers; agent_stop says it is "the one reader").
Splits: agent_stop.py:114 "The one reader of the Agent trailer" vs ref_judge.py:186 also reading trailers by key (different purpose: checks presence). Reported as K8-style claim, not graded.
Rulings: none. Tests: none compares preamble to ref_judge.

---------------------------------------------------------------
## Group C: rulings

### I9-140 Where do rulings live and what is a filed ruling line?
Homes:
- G\ruling_rules.py:95-100 `CONFIRMED, UNCONFIRMED = "## Confirmed", "## Unconfirmed"`, `APPROVE, DECLINE = "File it","Do not file"`; LINE/SESSION_NOTE regexes (:105); `parse()` message "one starts '- **<date>**' and holds one quote or one chose".
- Hydra docs\agents\rulings\README.md:3, :16-18, :20-30 (prose shape with examples; ":41 The two labels are `File it` and `Do not file`").
- H\CLAUDE.md:41 "A ruling is filed only when the user clicks "File it" for that exact line in a question box".
- G\README.md:57-61; docs\agents\agent-rules.md:33 ("Nothing is recorded as the user's ruling without the user's explicit approval").
- G\ref_judge.py:41 `RULINGS_DIR = "docs/agents/rulings/"`.
- ST\common.py:53 comment "`## Confirmed` in Hydra's docs/agents/rulings/, so new cards name the section WORDS_HEADING".
- ST\number_rulings.py:104 writes decision-log lines `**D<n> (session <8>)**`, a different shape.
Table: ruling_rules refuses (ref gate, main and worktrees alike since it reads the commit). README tells the session what a line looks like. MAIN: only main sessions' transcripts count for "File it" (README 43 "in a main session's transcript"). CARD: cards "Rulings"/"The user's words" headings are not ruling lines; a card quote is not filed. LIMIT: session = first 8 chars or whole id (README line 30 area; SESSION_NOTE regex allows both).
Kinds: K1 (line shape and the two labels stated in README, CLAUDE.md, ruling_rules, tests), K6 not found; README names itself the format owner while ruling_rules.py is the code that enforces it (K7-ish: no test compares README example to LINE). 
Splits: none found. Confirmed rulings: none quoted specific to the shape.
Tests: test_ruling_rules.py:21-29,125-178 pins the code side only.

### I9-143 Which ruling-file edits are judged and which pass free?
Homes: G\ruling_rules.py (module docstring line 1 "May a change add these lines to docs/agents/rulings/: answered here only."; `_heading_above`, :177, :189, :345); Hydra docs\agents\rulings\README.md:43 ("Git's gate checks the commit. It refuses any added line that is not in the right shape, whose quote the user never typed, or that has no `File it` answer..."); G\README.md:59 ("owns which added lines may go in"). The pass-free list (removed lines, blank lines, headings, README prose, one title line for a new file) is only in code and test_ruling_rules.py:296-329.
Single home for the pass-free rule; README says "any added line" without the exemptions (README 43 reads broader than the code). WT/MAIN: the check reads the commit diff, so a worktree commit is judged the same.
Kinds: K3 (README sentence "refuses any added line that is not in the right shape" vs code that lets prose, headings, blanks pass in README.md and one title line pass). Not graded.
Splits: README.md:43 broad wording vs ruling_rules.py exemptions (stated as a difference of wording).
Tests: test_ruling_rules.py:296-329.

### I9-146 The test says its transcripts copy the shapes in tools/session_cards/fixtures and "the record the main session read back live"
Homes: G\tests\test_ruling_rules.py:1-8 (claim); ST\fixtures\ (real shapes; exists, one jsonl `aaaaaaaa-...jsonl`); the real parser ST\prepass.py `condense`, which ruling_rules.py:220-235 calls (`cards.transcript_files`, `prepass.condense(path, ...)[2]`).
Other code that decides "what is the user's typed message in a transcript": G\review_rules.py:89-109 `human_prompt_text` (skips isMeta, isSidechain, toolUseResult, text-only blocks); G\agent_stop.py:26-36 `first_user_message`; G\agent_stop.py:70 another user-record reader; ST\common.py:384-395, :474 (toolUseResult, isSidechain); ST\prepass.py:89-140; G\stall.py:174-200 `_tools_fact`.
Kinds: K6 (several readers answer "which transcript records are the user's words / the brief": prepass.condense for rulings, human_prompt_text for waivers/allow phrases, first_user_message for briefs). K1 (the test copies shapes of real records, owned by Claude Code's transcript format).
Splits: none shown; the filters differ by purpose (not compared line by line).
Tests: test_ruling_rules uses made-up transcripts; no test holds the readers equal.

---------------------------------------------------------------
## Group D: shell output, refusals, status

### I9-154 How the post-tool hook answers
Homes: G\shell_output.py:170-197 (builds `{"hookSpecificOutput": {"hookEventName":"PostToolUse","updatedToolOutput": out}}`); G\common.py:22-28 (builds the PreToolUse and additionalContext shapes the same way); G\README.md:172 ("To undo only the shell output trim, remove the PostToolUse block from settings.fragment.json"); G\settings.fragment.json registers it. Single owner for the PostToolUse reply; shell_output.py is the only writer of updatedToolOutput.
Edge: crash prints only a systemMessage (test). Searched: gates/*.py, README. Single home plus the README pointer. Kinds: none. Tests: test_shell_output.py:392-419.

### I9-163 Refusal log lines of the git gate
Homes: G\common.py:35-46 `log_refusal` (the only writer; README.md:79 "`common.log_refusal` writes it, and it is the only writer ... `ref_judge.py` beside its refusal, and `publish_gate.py --check`"); G\ref_judge.py:416 calls it with `hook_event_name: "push"|"ref"`; common.py:46 `reason[:300]`; README.md:88 sample query. Brief-preamble/other text states the 300 only through tests. One writer. Kinds: K1 only in tests (cross-refs I9-008/056/075/105 per the row). No split. Tests: test_refusals.py, test_ref_judge.py:823-846.

### I9-174 What status_append.ps1 does
Homes:
- G\status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` literal (own comment: "progress.py owns how a status write is recognised"), :12 StatusDir `status`, AgentsDir `agents`.
- G\common.py:124 `os.path.join(root or STATE_ROOT, "status", check_id(agent_id) + ".md")` and STATE_ROOT (common.py:10) = gates\state; two code places name the same folder.
- G\progress.py:22 `STATUS_PREFIX = '& "C:/Users/Patrick/.claude/gates/status_append.ps1"'` (the call shape, regex :27-30) and `_helper_line` message `"HH:MM done ... | next: ..."`.
- G\settings.fragment.json allow rule for the same helper path (progress.py:21 says it is "spelled as the allow rule ... names it").
- Hydra docs\agents\brief-preamble.md:13-15 ("Append one line to `C:\Users\Patrick\.claude\gates\state\status\<your agent id>.md` every 5 minutes. The form is `HH:MM done ... | next: ...`. ... `& "C:/Users/Patrick/.claude/gates/status_append.ps1" <your agent id> "<line>"`").
- The agent hook text (this run's hook output) tells agents the same call.
Table: preamble tells agent to write every 5 min and gives the call; progress.py enforces shape and budgets (call/time numbers at top of progress.py per plan line 108); status_append.ps1 refuses bare ids with no agent record. MAIN: main session is not an agent record; helper would refuse its id (not exercised). CARD: card writers skip the preamble, so they learn the call only from their brief; budget hooks still apply. WT: script always writes the live state folder even from a worktree copy (script comment); the preamble path is absolute.
Kinds: K6 (state root literal in ps1 vs common.STATE_ROOT; status path in common.py:124 vs ps1), K1 (call text and line form in preamble, progress.py, settings.fragment.json).
Splits: none by value. Confirmed rulings: review-and-hooks.md:34 and agents-and-workflow.md:25 (quoted above). 
Tests: test_status_append.ps1 uses HOOKS_TEST_STATE_ROOT; test_progress.py pins STATUS_PREFIX behaviour. No test holds the ps1 literal equal to common.STATE_ROOT (searched tests dir names only).

---------------------------------------------------------------
## Group E: pointer and handoff checkers (Hydra tools)

### I10-006 Which line starts a memory note's card pointers
Homes: H\tools\check_pointers.py:36 `SESSIONS_LINE = "Sessions:"`; ST\link_memory.py:23 `SESSIONS_PREFIX = "Sessions: "` (the writer); H\CLAUDE.md:27 "Memory files end with a `Sessions:` line that names their cards"; tests ST\test_session_cards.py:529,678,897 and H\tools\test_check_pointers.py:100 each type the literal. 52 memory notes carry a `Sessions:` line.
Kinds: K6 (writer and reader constants in two code places, one with a trailing space), K1 (CLAUDE.md). Splits: none by behaviour (reader matches prefix without the space). Tests: none compares writer and reader.

### I10-007 How a doc keeps naming a file that left the repo
Homes (three implementations of "the removed form"):
- H\tools\check_pointers.py:37-38 `GIT_SHOW = re.compile(r"`git show ([0-9a-fA-F]{7,40}):([^`\s]+)`")`, own comment :161 says CLAUDE.md gives the form.
- H\tests\test_docs_match_code.cpp:1334-1345 `is_removed_citation`: accepts `<7-40 lowercase hex>:<docs path>` anywhere on the line (written out or not), OR the words "never committed". 
- H\CLAUDE.md:57 "as `<path>` (removed; `git show <commit>:<path>`)".
Differences (stated as splits): C++ accepts lowercase hex only, no backticks needed, and "never committed"; Python needs backticks around `git show <hex>:<path>`, accepts upper and lower case, does not accept "never committed". The C++ check covers docs paths; the Python covers pointers of any path shape.
Kinds: K6, K1. Edges: LIMIT 7 hex digits both; MAIN/WT: Python tries doc folder, repo root, hooks root.
Rulings: none. Test holding equal: none.

### I10-011 What counts as a pointer
Homes: H\tools\check_pointers.py:108-134 `is_pointer` (via doc_tokens.shape); H\tools\doc_tokens.py:39-56; H\tests\test_docs_match_code.cpp:1326-1330 `docs_path_re` (docs/ paths), :1040 (backticked code names exist in src/tools/tests), :232 (span names code). Different questions share one idea of "a path in a doc"; docs_path_re is a separate regex from `_BARE_PATH`/`shape`. Kinds: K6 (two path regexes in Python and C++). Splits: none shown by value. Tests: test_check_pointers.py vs the C++ test; none holds equal.

### I10-012 When a pointer is broken, retired, or found elsewhere
Homes: check_pointers.py:136-229 (roots: doc folder, repo root, hooks root; "retired" if under `<hooks root>\hooks` or old hooks folder; "found at"); C++ test docs path check (:1420 "no permanent doc or code comment names a docs path that is not in the tree") checks only repo `docs/` paths against the tree and has no "retired" or "found at" word; G\README.md:71 (registers check_pointers by full path). Observed: live folder `C:\Users\Patrick\.claude\hooks\state\build_slots` still exists while the gates folder is `gates\` (see I10-041).
Kinds: K6 (two checkers answer "does this named file exist": Python for agent docs and memory, C++ for committed docs and comments). Not graded. Edges: WT: check_pointers reads the main checkout when run by the SessionStart hook (README 71).

### I10-014 Output line format
Homes: check_pointers.py:13-14 docstring and :267; G\README.md:71 (describes the session-start message); H\tools\test_check_pointers.py:137 (copies the text "check_pointers.py"). check_handoff prints a different summary shape (see I10-020). Single code home; docstring quotes the format (a home: it states the value). Kinds: K1 (docstring and print). Tests: test_check_pointers.py.

### I10-017 Full commit hash in a Claims row
Homes: check_handoff.py:28 `^[0-9a-fA-F]{40}$`; docs\agents\handoff.md:19 "the full 40-character hash", :51 "(git does not know the hash, or it is not the full 40 characters)". Other places that decide "what is a commit hash": doc_tokens.py:39 `_HEX` 7-40 either case; :49 `_BARE_HEX` lowercase only with a digit and a letter; check_pointers.py:38 7-40 either case; test_docs_match_code.cpp "shown" regex 7-40 lowercase; G\review_rules.py:26-28 waiver phrases `[0-9a-f]{7,40}` (derive-once, real-app) and `{7,64}` (doc-review), :56 `_HEX` 40 or 64 lowercase; ST\common.py:78 `HEX` 7-40 requires a digit; :79, :541 40-64. 
Kinds: K6 (hex-shape rule in at least 8 code places with differing case, length and digit rules). Splits (as separate lines): (a) check_handoff accepts uppercase 40 hex, review_rules._HEX does not; (b) doc_tokens._HEX accepts uppercase 7-40, _BARE_HEX does not; (c) waiver doc-review phrase allows up to 64 hex, the other two kinds allow up to 40.
Rulings: none. Tests: none holds equal.

### I10-018 The Claims table heading
Homes: check_handoff.py:31 `CLAIMS_HEADING = "## Claims"`; handoff.md:15 "a heading that is exactly `## Claims`"; :47; H\.claude\commands\handoff.md (the slash command text names the section; line with "`## Claims` table"); test_check_handoff.py:226 TABLE_HEAD (typed copy), :298; H\tests\test_docs_match_code.cpp:1517-1580 reads handoff/plan tables ("as split_row in tools/check_handoff.py reads it", "as claims_rows in tools/check_handoff.py reads it") with its own `table_cells`/`is_table_row`.
Kinds: K1; K6 (table-row splitting implemented in Python check_handoff and again in C++ `table_cells`, which its own comments call a copy of the Python rule). Splits: none by value found. Tests: none holds C++ and Python equal.

### I10-019 Which claim kinds are manual
Homes: check_handoff.py:32 `MANUAL_KINDS = ("test","ruling","state")`; handoff.md:19-27 (seven kinds table), :39 ("leaves the `test`, `ruling` and `state` rows to the session (it marks them `manual`)"), :49 "covers all seven kinds"; .claude\commands\handoff.md. Kinds: K1 (list stated in handoff.md:39 and code). Splits: none. Tests: test_check_handoff.py.

### I10-020 Row statuses
Homes: check_handoff.py:33 `TABLE_STATUSES = ("ok","missing","not-on-main","not-pushed","manual")`; handoff.md:51 (commit statuses described with the same four words); brief-preamble.md:97 "each Claims row not marked `ok` or `manual`"; handoff.md:~60 same sentence; .claude\commands\handoff.md ("A row marked `not-on-main` or `not-pushed` is right on..."). Row text says "plus unknown" (code only). Table: handoff.md/preamble: report every row not ok/manual before building; command text: fix every `missing`; `not-on-main`/`not-pushed` are not errors (handoff.md ~:60). MAIN/AGENT: continuing session (an agent or main) both run the checker. CARD: n/a.
Kinds: K1. Splits: none. Tests: test_check_handoff.py.

### I10-025 Scrape-mode output titles
Homes: check_handoff.py:310 (`EXISTS`, `MISSING`, `CHANGED SINCE <date>`); handoff.md ~:62 ("It reports each `MISSING` or `CHANGED SINCE` line (scrape mode)"); brief-preamble.md:97 (same words). Kinds: K1 (words stated in preamble and handoff.md; the preamble also lists only two of the three). Splits: none. Tests: test_check_handoff.py (scrape mode).

### I10-029 Token shapes / I10-030 Bare-hex rule
Homes: doc_tokens.py:39-56 (shapes) and :49-50,146-149 (bare hex: 7-40 lowercase with a digit and a letter a-f). Same idea elsewhere: see I10-017 list (C++ docs test spans/regex; session_cards HEX requires digit only). Also `_ID_WORD` "session|agent" id exclusion; ST\common.py has its own session-id regexes; G\ruling_rules.py:105 SESSION_NOTE (8 hex or UUID). 
Kinds: K6 (hex-token rules). Splits: doc_tokens bare hex requires a letter a-f AND a digit; ST\common.py:78 requires a digit only; C++ `shown` requires neither. Tests: none holds equal.

### I10-041 Lock file format (build slot)
Homes: H\tools\build_slot.ps1:116-120 (comment "Read-BuildSlot says what a lock holds"), :204 writer, :239; default dir :78 `$BuildSlotDefaultDir = 'C:\Users\Patrick\.claude\hooks\state\build_slots'` and header comment :10 (same path). Live folders: `C:\Users\Patrick\.claude\hooks\state\build_slots` exists, while the gates state is `C:\Users\Patrick\.claude\gates\state` (no build_slots there). tools\hydra_ui_mcp.py:24 ("The rule is tools/build_slot.ps1's; build_pass only asks") and :720-737 call the script; G\percall.py:20,109 refuse the hydra_build_pass tool to agents. Slot count: build_slot.ps1:75 `$BuildSlotCount = 3` and :73 comment ("the user's limit of three is in ..."), H\CLAUDE.md:76, docs\agents\agent-rules.md:20 (rule 14 "At most three builds run at once"). No other reader of the lock fields was found (grep "expires=", "pass=").
Table: script: refuse a 4th claim; rule 14/CLAUDE.md: tell the main session to build one at a time and agents never build (percall refuses agents the tool). MAIN/AGENT: pass tool refused to agents (percall.py:109). WT: `build_slot.ps1 -Repo <worktree>` builds a worktree under a pass.
Kinds: K1 (limit 3 in script, CLAUDE.md, agent-rules); K6 none on the lock format (one home). Side finding: state folder naming `hooks\state` vs `gates\state` (see Side notes).
Rulings: review-and-hooks (build-pass ruling is quoted in G\README.md:15 as "agents are never allowed to start their own builds, only the main session", not checked against rulings files).

### I10-060 Real-app record file and fields
Homes: H\tools\hydra_ui_mcp.py:78-81 (`REALAPP_FILE = "realapp.jsonl"`, "field names are a contract") and :194-199 (writer); G\review_rules.py:47-49 (`REALAPP_FILE = "realapp.jsonl"` with comment "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them" -- it states the value, so it is a home), :371-395 reader (`kind`, `dirty`, `commit`, `commands` keys); H\docs\agents\ui-testing.md:237-251 (field table; "`kind` | always "realapp""); G\README.md:55 (rule for UI code).
Table: writer writes on window-session end; reader requires clean+used record; docs list the fields. MAIN/AGENT: agents may not start a window (hydra MCP) is not checked here. WT: record sits in the git common dir so all worktrees share it (ui-testing.md:237; review_rules uses `--git-common-dir`).
Kinds: K1 (filename and field names in three places), K8-no (owner claim in review_rules matches).
Splits: none by value. Tests: tools\test_hydra_ui_mcp.py:519-586 pins writer; no test holds the reader's keys equal to the writer's.

### I10-062 What counts as a screenshot reply
Homes: H\tests\ui\uitest_commands.cpp:412 `r.text = "screenshot: " + saved_to + "\n"` and tests\ui\uitest_engine.cpp:462 `std::printf("screenshot: %s\n", path.c_str())` (two producers in the engine); tools\hydra_ui_mcp.py:84 `SHOT_REPLY = "screenshot: "` (comment: "Its owner is the screenshot verb in tests/ui/uitest_commands.cpp"), :656 reads it, :862 docstring; tools\doc_shots.ps1:25 regex `'^screenshot: '`; docs\agents\ui-testing.md:87 ("print `screenshot: <path>`"); tools\test_hydra_ui_mcp.py:507,523,962 typed copies.
Kinds: K1, K6 (the engine prints the prefix in two places). Splits: none by value.

### I10-077 Capture format (capture_window.ps1)
Homes: H\tools\capture_window.ps1:23-50 (type `HydraCaptureV1`, comment "Bump it whenever the C# block below changes"), :93-107 (PrintWindow flag 2, PNG); docs\agents\ui-testing.md:352 (`Save-WindowShot` draws with PrintWindow) and :363 ("Change the version in the type's name whenever the C# block changes"). 
Kinds: K1/K5 (the bump rule is stated in the script and in ui-testing.md, nothing checks it). K7 (a rule with no check: no test compares the C# block to the version name). Tests: none found.

### I10-081 / I10b-072 doc_shots crop rule and the list of doc pictures
Homes (crop rule): H\tools\doc_shots.ps1:27-47 (code: reads ` rect=\((-?\d+),(-?\d+) (\d+)x(\d+)\)` and passes `--below file=y+height`); tools\doc_shots\shots.txt:1-10 header ("A rect line directly before a full-frame screenshot also tells doc_shots.ps1 to keep only the frame below that rectangle"); docs\agents\doc-screenshots.md:15 ("The shot list marks that with a `rect` line directly before a `screenshot` line that has no ref; the script then keeps only the part of the frame below that rectangle. The comment at the top of `tools/doc_shots/shots.txt` says the same."). The rect line text is produced by H\tests\ui\uitest_engine.cpp:409 (`rect=(%.0f,%.0f %.0fx%.0f)`); tools\test_hydra_ui_mcp.py:904,973,988,996 type copies.
Homes (picture list): shots.txt header "This file is the one list of those pictures; the page that explains it is docs/agents/doc-screenshots.md"; doc-screenshots.md:29 ("`tools/doc_shots/shots.txt` is the one list of the pictures"); the pages (docs\UserGuide.md, docs\differences-from-public-hydra.md) link the picture files. 
Test that holds these equal: H\tests\test_docs_match_code.cpp:2036 "every picture the public pages link is a shot the capture script takes, and the file exists". Searched: no test holds the crop rule's text equal.
Kinds: K1 (crop rule stated in script, shots.txt, doc-screenshots.md), K5-no. The picture list is one home plus the test. Splits: none.

### I10-087 Number canon limits (precheck)
Homes: H\tools\precheck\decisions.cpp:22-31 (`kWholeNumberLimit = 1e15`, `kMaxHexDigits = 16`, `kBoldLevel = 99`) and :88-92 `number_canon`; decisions.h:23-30 (comment "below kWholeNumberLimit in decisions.cpp ... 10 significant digits"; "digit separators (, ' _ and space) dropped, 0x hex read as a signed 64-bit value, a C++ suffix"); tests\test_precheck_check3.cpp:106-111 (typed expectations). Other places that read a number out of doc text: H\tests\test_docs_match_code.cpp:360 `(\d+(?:\.\d+)?)` (no separators, no hex) and :553 hex helper; tools\precheck\definitions.cpp:86 (tokenises a number with its digit separators).
Kinds: K6 (what a written number means: decisions.cpp `number_of` vs the docs test's own regex and hex reader). Splits: separators `, ' _` and a trailing `f`/`u` are accepted by precheck and not by the docs test regex (a difference of coverage, not a verified disagreement). Tests: none holds equal.

---------------------------------------------------------------
## Group F: ch_mcp / ch_probe

### I10b-012 Launch forms and agent state words
Homes: H\tools\ch_mcp\bot_run.py:99-107 (`LAUNCH_SUSPENDED="suspended"`, `LAUNCH_SPAWN="spawn"`, `STATE_IDLE/WAITING/PATCHED/REFUSED`, comment "The `state` words of launch.ts's arm and status replies"); agent\launch_core.ts:44 `state: "waiting"|"patched"|"refused"`, launch.ts:107 `{ state: "idle" }`; launch_patch.json `"launch": "suspended"`.
Kinds: K1 (Python copies of the TS words, across languages). Splits: none. Tests: ch_mcp\tests (not read).

### I10b-014 The one-byte launch patch
Homes: H\tools\ch_mcp\launch_patch.json (all values; its "about" says "The agent holds no address or byte of its own: this whole..."). The loader RVA 0x3968D0 is also written in docs\clone-hero\07-internals.md:666,688,698,738 ("Game code (6142): the loader RVA `0x3968D0`") and `code/6142/cli_loader_3968D0.asm`. launch_patch.json:4 `"dump": "C:\\Users\\Patrick\\Downloads\\Hydra\\hydra-data\\ch-dumps\\6142\\dump.cs"` and kit_methods.json:3 repeat the dump path that build_facts.py:28 `DUMPS_ROOT` owns. The patch RVA 0x396EC5 and bytes appear only in the json (not in 07-internals.md; searched docs\clone-hero, ch_mcp py/ts).
Kinds: K1 (loader RVA, build "1.1.0.6142", dump path stated in json, docs and DUMPS_ROOT). The docs pages are checked against the dump by the docs test; no test compares launch_patch.json to the pages (searched test_docs_match_code.cpp for the json: only constants.py addresses are compared, :998).

### I10b-017 Files a good dump must have / I10b-018 Default build and facts names
Homes: dump_build.py:28-34 (REQUIRED_FILES, REPORTED_FILES, STAGES, DONE_LINE); chdis.py:37 `SCRIPT_FILE = "script.json"`; README.md:15,17 (describes the dumper's output). dump_facts.py:53-54,381 `DEFAULT_BUILD = "6142"`, `NAMED_SIGNATURES`, `ENGINE_SIGNATURE`; build_facts.py:28-36,64-103 is described as "the one home" of "Which Clone Hero build is installed, where its dumped facts live" (but DEFAULT_BUILD "6142" is a second place a build number is typed); constants.py:24 "Clone Hero v1.1.0.6142"; launch_patch.json:3 `"build": "1.1.0.6142"`; H\tests\test_docs_match_code.cpp:719 `kBuild11 = "6142"`; kit_methods.json (dump line numbers for build 6142).
Kinds: K1 (build 6142 typed in at least 7 places), K8 (build_facts.py says it is the one home of "which build"; dump_facts.py:381 and launch_patch.json also state a build). Splits: none by value (all say 6142).

### I10b-022 song.ini key for a song title; I10b-023 chart files a song folder may hold
Homes (title): game_files.py:52-55 (`SONG_INI`, `SONG_SECTION="song"`, `SONG_TITLE_KEY="name"`) and `_song_section_lines` :277-290 ("This is the one rule for which lines read_song_ini and set_song_title see"; keys before any section count too); the engine's reader H\src\parse\song.h:177-183 `read_song_ini_keys` ("The library scan ..., the Preview (delay) and the chart loaders ... all read song.ini through here") with the loop at src\parse\song.cpp:2217-2238 (only the `[song]` section counts, `in_song_section` starts false).
Homes (chart files): install_chart.py:37 `CHART_FILES = ("notes.chart","notes.mid")`; src\parse\chart_files.cpp:28-33 `notes_file_format` (owner; also picks .mid over .chart at :39-41); tools\ch_probe\probe_songs.py:314-317 writes `notes.chart`/`song.ini`; game_tools.py:894 and server.py:209 ("Copy one chart folder's song.ini and chart").
SPLITS (separate lines):
- Section rule: Python game_files.py says keys before any section header count; src/parse/song.cpp starts outside the song section so such keys do not count.
- `pro_drums` truth: Python bot_run/game_files `song_ini_pro_drums` is `value.lower() in ("true","1")`; engine song.cpp:2276-2282 `ini_drum_key_is_true` is "integer != 0 (dotnet_int32) or trimmed case-insensitive `true`" (so `2`, `-1`, ` true ` count in the engine, not in Python). Python's use is only a printed "note:" at bot_run.py:762-764.
- `.mid` preferred over `.chart` in the engine (pick_notes_file); CHART_FILES tuple order lists `.chart` first (what install_chart does with the order was not read).
Kinds: K6, K2 (the splits above).

### I10b-028 Dynamics-stop flag bit
Homes: frida_tools.py:66-68 `DYNAMICS_STOP_BIT = 0x10` (comment: "The bit of the engine's flags word that SC-16 in docs/clone-hero/06-scoring.md asks about"); docs\clone-hero\06-scoring.md:175-177 ("When bit 0x10 of the engine flags word is set..."); ch_probe\constants.py defines `FLAGS` offset and `PRECISION_MODE_BIT = 0x1000` only (its header: "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address").
Kinds: K1 (0x10 in code and doc), K8 (constants.py claims to hold every number; the 0x10 bit lives in frida_tools.py). Test: the C++ docs test checks method/field addresses from constants.py (:998), not this bit.

### I10b-030 Menu field kinds and page words
Homes: frida_tools.py:150-167 (`ROW_ADDS_SCROLL`, `NO_ROW_PAGE="Ready"`); menu_fields.json (page enum words at :17-18, "Modifiers","Ready"; 177 lines); docs\clone-hero\07-internals.md IN-58 (cited in the comment: "the cursor alone on Confirm and Difficulty, cursor + scroll on Instrument and Modifiers"); menu_steps.json and menu_tools.py (page words not found there by grep). Test that holds them equal: H\tools\ch_mcp\tests\test_frida_tools.py:789 `assertEqual(words, set(FT.ROW_ADDS_SCROLL) | {FT.NO_ROW_PAGE})`.
Kinds: K1 (page words typed in json and py; test holds them equal).

### I10b-033 / I10b-034 / I10b-035 Kit identity, pressable elements, bindings
Homes: kit_tools.py:59-96 (`KIT_DEFINITION_ID=1`, "MIDI Drums", map category 1, layout 2, tag "hydra-kit", `KIT_REQUIRED_ID`, placeholders), :153-181 (`KIT_ELEMENTS`, `KIT_BINDINGS`; comment "The one table of what can be pressed on the kit"; element ids 0-7,18,19 appear in both tables); docs\clone-hero\07-internals.md:904-908 IN-82 (definition 1 "MIDI Drums", map id 2 at category 1 layout 2, ten elements Snare 0 ... StartPad 18, SelectPad 19, action ids 1,2,3,0,37,38,39,4,16,6, and "The action names are those of `KIT_BINDINGS` in tools/ch_mcp/kit_tools.py"); plan docs\superpowers\plans\2026-11-08-kick-axis-bind.md (rows P6-P9 cited).
Splits: kit_tools.py:~64 says "The fact's home is block IN-76 of docs/clone-hero/07-internals.md"; IN-76 (07-internals.md:833) is titled "Rewired gives a custom controller no default map on the System Player" and does not hold the definition id/name (grep of "MIDI Drums" in 07-internals.md matches only :904,:906,:936, all in IN-82/IN-84 area). The facts are in IN-82, and IN-82 in turn points to kit_tools.py for the action names (each points at the other). K8.
Kinds: K1, K8. Tests: the docs test does not read kit_tools.py's tables (searched :998 area).

### I10b-039 Play stop reasons and event words
Homes: kit_tools.py:709-730 (`STOP_REASONS`, `EVENT_*`, `PRESS_EVENT_FIELDS`); agent\press_schedule.ts:70-76 comment ("The stop reasons: done, past_end, stopped, cap, frozen, jumped_back") and the producing code :328,464,472; agent\kit.ts:1121,1156 ("frozen"); the field list comment in press_schedule.ts:55-65 describes the event fields.
Kinds: K1 (reasons and event words in TS producer and Python consumer, plus the TS header comment). Tests: not read.

### I10b-041 Windows thread states to words
Homes: machine_recorder.py:93-109 (`STATE_READS_AS = {2: RUNNING, 1: READY, 3: READY, 7: READY, 5: WAITING}`, `READY_STATES` derived); tests\test_machine_recorder.py:178 (layout comment of the record: ThreadState at 0x44). No other home (searched tools for ThreadState, Standby, DeferredReady: only the test comment). Single home.

### I10b-051 Song end used for a run
Homes: tools\ch_mcp\hydra_end.py:1-26 (docstring: "Hydra has one song end, app::song_length_ms (D255), and hydra_replay writes it ... as the top-level `song_length_ms` field (tools/replay.cpp)"; `FIELD = "song_length_ms"`, `DUMP_FOLDER_PREFIX = "hydra_end_"`); H\src\app\song_length.cpp (engine owner; stated_length_ms, end_event_counts); tools\replay.cpp and replay_schedule.cpp write the field; trace_hits.py:67-69 ("the end is D331's"); bot_run.py:387 and game_tools.py:662 define Python `song_length_ms` wrappers that delegate to hydra_end; sp_tools.py:395 delegates; ch_probe\probe_songs.py:25. Decision log: D255 (fix-decisions.md:1052), D331 (:1893), D75 (:375). Memory note use-source-facts-not-proxies.md cites D75.
Kinds: pointers plus one value (the field name). This is an engine value carried out through the hydra_replay dump. Splits: none (D255 amends D75 items 1 and 2 per fix-decisions.md:1052; the memory note still cites D75's "chart metadata" wording).

### I10b-053 Build stamp rules
Homes: build_stamp.py:32-43 (`STAMP_FORMAT = 1`, `BUILD_INPUTS`, `AGENT_FOLDER_TEXT = "tools/ch_mcp/agent"`, `NO_REPLAY_BUILD`); agent\package.json:6-12 (the six build scripts each call `py -I ../build_stamp.py`); the stamps agent\*.stamp.json (`"format": 1`); package.json "description" points at docs\agents\clone-hero-mcp.md "Safety rules" and build_stamp.py. 
Kinds: K1 (format 1 in the code and in each stamp file; the stamp files are generated). Tests: not read.

### I10b-054 Dynamics words
Homes: owner H\src\core\model.cpp `dynamic_str` (model.h:70 "none"/"ghost"/"accent"); src\core\replay.h:140 `{"accent",...},{"ghost",...}` (a second C++ spelling in the replay score table); kit_tools.py:258-264 (comment "dynamic_str in src/core/model.cpp owns them"); compare_scorestats.py:40-46 (comment: written out "because this file runs on its own under py -I, and kit_tools imports game_tools, which imports this file"); compare_scorestats.py:76 reads `final["accent"]`/`final["ghost"]`.
Kinds: K1 across three languages' files; K6 for the C++ pair (model.cpp and replay.h). Splits: none. Tests: none holds the Python copies equal to model.cpp (searched ch_mcp\tests names only).

### I10b-059 Function RVAs and field offsets of build 6142
Homes: tools\ch_probe\constants.py:38-156,326-342 (header: "This is the ONE place the numbers live. Every other module imports from here; nothing else hard-codes an address"); tools\ch_probe\SPEC.md (7 of the sample RVAs, offsets like `+0x2e0`, `self+0x198 & 0x1000`); docs\clone-hero\*.md pages and `code/6142/*.md/.asm` (180 matches across 37 files for six of the RVAs), docs\clone-hero\code\facts-6142.tsv (rows 955-967 give `0x20DF680`, `0x20DDDA0`, and cite `RVA_DRUMS_ENGINE_CTOR`, `RVA_WINDOW_FORMULA`); frida_tools.py:68 (0x10 bit); ch_mcp\tools use `C.OFF_*` names (frida_tools.py:118-120, kit_tools.py:277-292) and hold no hex of their own (searched agent\*.ts, ch_mcp\*.py, json).
Test that holds homes equal: H\tests\test_docs_match_code.cpp:996-1010 "Every method address and field offset in tools/ch_probe/constants.py has a row in the 6142 table, so the two homes cannot drift apart" (only the names in `kDataAddressNames`; I did not check that the list covers every constant, e.g. 0x10 bit and clock offsets).
Kinds: K1 (RVAs in constants.py, SPEC.md and doc pages), K8 (constants.py "ONE place" vs the docs pages, which the Clone Hero README treats as the reference).

### I10b-066 ch_probe SPEC and README
Homes: tools\ch_probe\constants.py:3 ("Every address here is transcribed from the handoff spec (tools/ch_probe/SPEC.md)"); ch_probe\README.md:17 ("every reverse-engineering fact is in SPEC.md beside this page") and :61 ("`constants.py`: every address, offset and constant. The one place ..."); SPEC.md:4 status line ("built ... D28 ... answers the hit-window question from the game's code and `poll_windows.csv` and shelves the rest of the plan"); docs\clone-hero\README.md:70 (names code\ pages as the reference).
Kinds: K8 (three owners named for the same facts: SPEC.md, constants.py, docs\clone-hero pages). Not read line by line (303+102 lines). Splits: none by value found.

### I10b-080 Range output prefixes
Homes: tools\precheck\items.cpp:11-12 (`kSizePrefix`, `kLargeRangePrefix`), items.h:50 `kLargeRangeLines = 400`, items.h:56 comment; tests\test_precheck.cpp:411-437 and test_precheck_git.cpp:198-274 type the full lines. No doc or agent file states these words (searched docs\agents, CLAUDE.md, ~/.claude\agents, gates). Single code home plus test copies.

### I10b-090 Scan row kind letters
Homes: tools\precheck\check_rows.cpp:242-249 `row_kind`; check_rows.h:26-29 (comment restates the rule: "A in production code; in a test file E for a row owned by the source-tree reader, C for a row owned by another test file, B for a row owned by production code"); docs\agents\derive-once-review.md:28-38 (kind letters A-H with different, behavioural meanings: "B. A test that recomputes what production computes ...", "C. A copied test helper or fixture", "E. A source scan outside tests/single_owner/ ...") and :38 ("The precheck's lines cover kinds B, C and E ...").
Kinds: K1/K3 (the doc's kinds and the code's letters answer different questions under the same letters: code letters classify where a scan hit sits; doc letters classify the finding). Not graded. Tests: tests\test_precheck*.cpp not read.

### I10b-101 Replay JSON field names
Homes: tools\replay_json.cpp:128-140 (usage text names "warnings", "sections", "measure" with "label"/"decimal"; "m28.3.120"); tools\replay.cpp:425-487 (writer: `{"measure", {"label", ...}, {"decimal", ...}}`, `sections` with `tick`); H\docs\agents\rulings\clone-hero.md:48 (an unrelated Confirmed line mentioning practice sections). No Python or doc reader found (grep of ch_mcp, ch_probe, tools\*.py).
Kinds: K1 (usage text restates field names the writer defines). Not read line by line.

### I10b-108 First-use table
Homes: tools\mutate\first_use_lines.txt (86 lines; header cites `git show 891ab424:docs/audit/2026-11-14-test-audit/mechanical/reread/{A3,...}.md`, "the 62 disputed rows"), first_use_table.md (91 lines, generated by first_use.py from a named run "20261116-123523"; "Nothing is judged here"). Single generated artifact; lines.txt is a hand-picked copy of rows from re-read audit files at a commit. Kinds: K1-no (generated output). Not read in full.

### I10b-111 Handoff table shape (test copy)
Homes: tools\test_check_handoff.py:226 TABLE_HEAD (`## Claims`, `| Kind | Value | Claim | How to check |`); docs\agents\handoff.md:15-17 (same four columns); check_handoff.py:31; C++ test table readers (see I10-018). Kinds: K1. Test holds equal: the test runs the checker on its own typed header, so code and test agree; handoff.md is not compared.

### I10b-115 compare_db fixture layout
Homes: tools\test_compare_db.py:19-154 (SCHEMA with results, paths, path_refs, charts, meta, keyed by result_id/profile_id); tools\compare_db.py:96-146 (`TABLES`, `SUMMARY_TABLES`, `SUMMARY_LEFT_OUT`, `OWN_IDS`, `REFERENCES`; docstring says "The table names and per-file ids are written out in this script"); owner H\src\store\record_store.cpp:272-304 (profiles, records, record_paths, charts, meta tables) and src\store\library_file.cpp:86-94 (reads `results` with columns `structure`, `stars`, `legacy_fills`). I could not find where the `results`, `paths`, `path_refs` tables are created in src (searched CREATE TABLE and path_refs in src: none), so I cannot name the owner of those three tables.
Kinds: K1 (table names in compare_db.py, test, and store), K8/K4 open. Tests: test_compare_db.py runs the real script on a typed schema; none compares it to record_store.cpp.

---------------------------------------------------------------
## Side notes (outside my rows)
1. G\README.md:75/progress.py and build_slot.ps1:78: the live folder `C:\Users\Patrick\.claude\hooks\state\build_slots` exists while hooks now live under `C:\Users\Patrick\.claude\gates`; check_pointers treats a path under `<hooks root>\hooks` as "retired". build_slot.ps1:10 and :78 still name the old `hooks\state` path.
2. Slot limit "three": build_slot.ps1:75, CLAUDE.md:76, docs\agents\agent-rules.md:20 (K1 for whoever owns that row).
3. G\progress.py:19-20 REPORT_TOOLS vs percall's own tool-name checks for SubagentHandback (possible second list).
4. G\tests\test_docs_numbers.py:42 TYPED_KEY is a test that holds docs and the rule "no doc asks for a typed key" equal; mention for the doc-key row of whoever owns doc-review.
5. A hook gap noticed: status_append.ps1 hard-codes the live state folder, so its test must override by environment variable; a copy of the script elsewhere still writes the live folder (by design in its comment; a claim, not evidence).

## Rows not reached
None skipped. Thin depth (read headers only, not line by line): I10b-066, I10b-101, I10b-108, I10b-039 (Python tests), I10b-053 (tests).

## Counts
Rows done: 55 of 55. Splits: I10-017 (3 sub-splits on hex), I10-029/030, I10-007, I10b-022/023 (3), I10b-033/034/035 (IN-76 vs IN-82), I9-143 (README wording vs code), I9-131 (CR vs CRLF wording). Tests that hold homes equal found: test_frida_tools.py:789 (menu pages), test_docs_match_code.cpp:996-1010 (constants.py vs 6142 table), :2036 (shot list vs pages), test_docs_numbers.py:42 (no typed key in docs).
