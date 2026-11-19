# TOOL-AUDIT-T5-W14 (second-round verifier; ledger L09 rows L-F26-17..20, L-F27-09..29)

All claims are by reading. Nothing was run. H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, ST = C:\Users\Patrick\.claude\tools\session_cards, M = memory folder. I searched H docs\agents\rulings\ only by the words the row names; no `## Confirmed` line naming either side of any split below was found (keyword search only; open for all rows).

### L-F26-17
**Verdict:** CONFIRMED
**Homes as found:** G\tests\test_docs_numbers.py:51-59 `README_NAMES` (8 names), `APPROVAL_HEADING = "## How to file a ruling: the approval step"`, `OLD_HOOK_LIVE`. H docs\agents\rulings\README.md:36 holds that exact heading. G\README.md names all eight (counts 1 to 4 each); README.md:61 states the test fails when the page lacks the step.
**Splits:** None. Heading text is typed in the test and on the page; they agree today. They would differ if either is reworded; the test fails then (one direction).
**Ruling:** none found.
**Open:** none.

### L-F26-18
**Verdict:** CONFIRMED
**Homes as found:** G\README.md:41 says a saved hand-back means finished; with none, the transcript is read for an open background task; "A transcript the hook cannot read leaves the agent marked finished". G\agent_stop.py:84-98 `parked_on` (returns [] when hand-back saved; raises OSError on unreadable) and :256-276 sets `parked`/`parked_since`. The outcome words ("verdict recorded", "no verdict", "not a reviewer", "crashed") are at agent_stop.py:237-247 only; the README never spells them, so no word split.
**Splits:** None between README:41 and code. Side: percall.py saves a hand-back only for SubagentHandback (G\percall.py:102), so an agent ending with StructuredOutput has none saved and goes through the transcript test (same pair as L-F27-03).
**Ruling:** none found.
**Open:** none.

### L-F26-19
**Verdict:** CONFIRMED
**Homes as found:** M\MEMORY.md:22 "reach kHitWindowMs 85 / slack 60". The note early-fill-orange-85-d250.md:11 names `kEarlyFillSlackMs` (60 ms) read by `is_e0`; grep of H src, tests, tools finds no `kEarlyFillSlackMs`. src\core\model.h:106 `kHitWindowMs = 85.0`; :112 `kDefaultEarlyFillWindowMs = 60.0`, whose comment says every default of the setting is this one constant. The relay drift (index says "one plain sentence per agent") is in L-F26-04.
**Splits:** index line and note both name a slack constant the code lacks; code has one 60 ms default constant. They differ on whether a separate white-side number exists.
**Ruling:** none named here (see L-F26-01).
**Open:** the other index lines were not compared.

### L-F26-20
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** M\dmleaderboards-comparison.md (replace rule; routes `/api/leaderboard/{song}`, `/api/user/{id}/stats`, `/api/songs/{md5}`, `/api/users/compare`). H src\app\dm_report.h:3 "leaderboard `identifier` == Hydra `hyhash`". The finder missed a code owner of routes: H src\net\dmbot_client.h:6-7 `GET /api/all-users` and `GET /api/user/{id}/scores`, and :50 "`identifier` is Clone Hero's song id ... the exact join key" (:53 32 hex digits). The note lists neither of the two routes Hydra calls. H docs\clone-hero\07-internals.md IN-55 (956-966) covers scoredata.bin and "Hydra: no owner"; it does not hold the replace rule.
**Splits:** (1) the note's `/api/songs/{md5}` says md5; dmbot_client.h and ADR 0028 call the id Clone Hero's song id. They differ for a chart whose id is not an md5 of the file (the row L-F25-23 case). (2) The replace-on-higher-score rule lives only in the note (prose; no code mirrors it).
**Ruling:** none found.
**Open:** the live API was not asked.

### L-F27-09
**Verdict:** CONFIRMED
**Homes as found:** G\review_dispatch.py:170-173 writes handover with `prompt_sha256 = sha256(prompt)` from the tool input (`ti.get("prompt")`, :158). G\agent_stop.py:26-36 `first_user_message` returns the transcript's first `user` record text (a string, or its text blocks joined with no separator); :199-201 hashes that and matches the handover. G\common.py:11-12 REVIEW_KINDS lists the handover fields.
**Splits:** dispatch hash (tool prompt) vs stop hash (first transcript user record). They differ when the transcript's first user record is not the brief byte for byte: several text blocks joined with "", a compaction or meta record that comes first, or a transcript with no user record (the function then returns None and `.encode` would fail; caught by the crash path at agent_stop.py:247). Whether real transcripts do this needs a run.
**Ruling:** none found.
**Open:** a real reviewer run with a multi-block first message.

### L-F27-10
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** G\agent_stop.py:113-124 `agent_commits` "The one reader of the Agent trailer" (git log with `%(trailers:key=Agent,valueonly,separator=%x09)`, matches by exact id after splitting on tab; no `unfold`). G\stall.py:209-232 calls it. G\ref_judge.py:40 `TRAILER_KEYS`, :224-253 reads the same trailer by key with `unfold,separator=%x2C` and only requires an alphanumeric character in each of Task, Agent, Session; merges exempt (:248). H docs\agents\brief-preamble.md:60-67 names Task:, Agent:, Session: and the merge exemption, and says the stop hook finds commits by the Agent trailer.
**Splits:** (1) "The one reader" is not true by reading: ref_judge.py reads the same trailer with another format string. They answer different questions (is the id this agent's, vs is any value present). They differ for a trailer value with a folded continuation line, or an Agent value that is not the committing agent's id (ref_judge accepts it, agent_stop never counts it). (2) Same wording in the preamble and the two codes; no test holds them equal.
**Ruling:** none found.
**Open:** none.

### L-F27-11
**Verdict:** CONFIRMED
**Homes as found:** H docs\agents\rulings\README.md:43 "It refuses any added line that is not in the right shape, whose quote the user never typed, or that has no `File it` answer". G\ruling_rules.py:366-387 `_judge`: in a topic file a blank line, a heading in HEADINGS, and one title line in a new file (`title_left`) are skipped; any other added line is judged. In the format page README itself only lines starting `RULING_START` are judged (:383-384), so README prose edits pass. Lines moved out from under `## Unconfirmed` are judged too (:367-370, `confirmed_lines`). Removals pass (README:46 "Removing a line is always allowed").
**Splits:** README.md:43 "any added line" vs the code's exemptions (blank, headings, one title, README prose). They differ for a heading edit or a README sentence. Second: README:43 says nothing of moved lines.
**Ruling:** none found.
**Open:** none.

### L-F27-12
**Verdict:** CONFIRMED
**Homes as found:** G\review_rules.py:89-109 `human_prompt_text`: a user record with matching promptId, `turnOrigin` "human", not isMeta, not isSidechain, no toolUseResult, and every content block a text block (otherwise skipped). G\ruling_rules.py:229-232 `user_text` uses ST prepass.condense (kinds answers, messages, queued words; ST common.py:380-396 classes tool_result, rejection, compaction, meta, subagent_brief). G\agent_stop.py:26-36 `first_user_message`: first `type: user` record, no filter, non-text blocks dropped. G\stall.py:190-200 reads agent transcripts by path only.
**Splits:** three readers of "which transcript record is the typed words or the brief": (1) human_prompt_text refuses a record that has any non-text block; first_user_message keeps the text blocks of the same record. (2) first_user_message takes a compaction summary or meta record if it comes first; the other two skip them. (3) prepass counts words the user typed while Claude was busy (queued); human_prompt_text needs a promptId and turnOrigin "human".
**Ruling:** none found.
**Open:** none.

### L-F27-13
**Verdict:** CONFIRMED
**Homes as found:** G\status_append.ps1:11 `$StateRoot = 'C:\Users\Patrick\.claude\gates\state'` (comment :8-10 says always the live folder). G\common.py:10 `STATE_ROOT` is the folder beside the file, :125 status path. G\progress.py:22 `STATUS_PREFIX = '& "C:/Users/Patrick/.claude/gates/status_append.ps1"'`. G\settings.fragment.json:4 allow rule with the same prefix. H docs\agents\brief-preamble.md:13-15 gives the call and the path.
**Splits:** None by value today. The ps1 literal and the folder `common.py` computes agree only while the gates folder stays at that path; no test compares them (by reading). This very session's refusal of a `powershell -File` call (the hook knows the exact shape) shows the prefix is the single shape.
**Ruling:** none found (finder's two quotes not reopened).
**Open:** none.

### L-F27-14
**Verdict:** CONFIRMED
**Homes as found:** H tools\check_pointers.py:36 `SESSIONS_LINE = "Sessions:"`, used at :251 as `line.lstrip().startswith(SESSIONS_LINE)`. ST\link_memory.py:23 `SESSIONS_PREFIX = "Sessions: "`, used :80 (`lines[-1].startswith`) and :83 (writes it as the last line, after a blank).
**Splits:** writer needs the line at the end with a space after the colon and no indent; reader skips any line starting (after indent) with `Sessions:` anywhere in a file. They differ for an indented or mid-file `Sessions:` line, or one without a space.
**Ruling:** none found.
**Open:** none.

### L-F27-15
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H tools\check_pointers.py:108-134 (path test), :184-204 (`_live`, `_broken`: "retired" when the path is inside `<hooks root>\hooks`, else a three-step match giving "found at ..." or "missing"). H tools\doc_tokens.py:39-56 (extension list, `_HEX` 7-40 either case, `_BARE_HEX` lowercase only, `_BARE_PATH`, `_ABS_PATH`). H tests\test_docs_match_code.cpp:1320-1329 `docs_path_re`: only `docs/...` paths, exact tree membership, in permanent docs and code comments (:1406). The live folder C:\Users\Patrick\.claude\hooks\state exists (listed), while check_pointers.py:105 `old_hooks = <hooks root>\hooks` treats anything below as retired. H tools\build_slot.ps1:10,78 name `...\.claude\hooks\state\build_slots`.
**Splits:** (1) Python resolves any extension-listed path in any file kind with fuzzy "found at" matches; C++ checks only `docs/` paths and demands an exact tree entry. They differ for a docs path that exists with other letter case or other folder (Python "found at", C++ red), and for non-docs paths (C++ silent). (2) A path under the old hooks folder reads "retired" in Python though `hooks\state\build_slots` is live today; whether a doc cites such a path through the scan is not read.
**Ruling:** none found.
**Open:** a run of check_pointers on build_slot.ps1's comment.

### L-F27-16
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H tools\check_handoff.py:30-33 `CLAIMS_HEADING = "## Claims"`, `MANUAL_KINDS = ("test","ruling","state")`, `TABLE_STATUSES = ("ok","missing","not-on-main","not-pushed","manual")`; :321-350 `split_row`, `claims_rows`; :305-312 scrape prints EXISTS, MISSING, CHANGED SINCE. H docs\agents\handoff.md:13-27,45-55 holds the heading, the seven kinds and the statuses. H docs\agents\brief-preamble.md:95-97 says to report each MISSING or CHANGED SINCE line (names two of three scrape words; EXISTS needs no report, so not a split). H tests\test_docs_match_code.cpp:1554-1580 `table_cells`/`is_table_row` copy `split_row`/`claims_rows` (comments say so), but the C++ copy reads plan Premise-map and Questions tables (:1730), not a handoff's Claims table (that C++ file has no "Claims" reader).
**Splits:** (1) two copies of the table-cell rule, Python and C++; same today (both strip outer pipes, keep pipes inside backticks, accept a row with two or more pipes). No test holds them equal. (2) The row-status list is typed in check_handoff.py and in handoff.md; no test holds them equal. (3) Python skips rows whose first cell is "kind" or all dashes; the C++ copy was not read for that.
**Ruling:** none found.
**Open:** none.

### L-F27-17
**Verdict:** CONFIRMED
**Homes as found:** H tools\build_slot.ps1:10 and :78 `C:\Users\Patrick\.claude\hooks\state\build_slots` (the folder exists); :75 `$BuildSlotCount = 3`; :152, :197, :250 use it; :193 one live pass per session. G\percall.py:20 `BUILD_PASS_TOOL`, :109 area refuses it to agents. H CLAUDE.md rule 14 "At most three builds run at once ... one at a time". Gates state folder is `...\gates\state` (status_append.ps1:11).
**Splits:** (1) "three" typed in build_slot.ps1, CLAUDE.md rule 14, agent-rules.md rule 14 and the ruling; code is the only enforcer. Agree today. (2) The lock folder lives under the old `hooks\state` while all gates state is under `gates\state`; two state roots answer "where does hook state live". They differ only in location, not value.
**Ruling:** H docs\agents\rulings\agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." It names the limit of three and the one-per-session limit; both match the code. It names nothing about the folder.
**Open:** none.

### L-F27-18
**Verdict:** CONFIRMED
**Homes as found:** H tools\hydra_ui_mcp.py:78-81 `REALAPP_FILE`; :192-199 writer builds `{kind, commit, dirty, commands, shots, started, stopped}`. G\review_rules.py:47-49 `REALAPP_FILE`; :371-395 reader uses only kind, dirty (must be `False`), commit and commands (int >= 1). H docs\agents\ui-testing.md:237-251 lists the seven fields.
**Splits:** (1) file name typed in two files (agree). (2) The gate never reads `shots`, `started` or `stopped`; the page says `commands` counts screenshots and timeout lines, so a session of one screenshot satisfies the gate. They differ on what "used" means: a record with zero real checks but one command passes.
**Ruling:** none found.
**Open:** none.

### L-F27-19
**Verdict:** CONFIRMED
**Homes as found:** H tests\ui\uitest_commands.cpp:412 `r.text = "screenshot: " + saved_to + "\n"`; H tests\ui\uitest_engine.cpp:462 `std::printf("screenshot: %s\n", ...)` when no `saved_to` (the stdout path); H tools\hydra_ui_mcp.py:84 `SHOT_REPLY = "screenshot: "` with comment naming the owner; :656 uses it; H tools\doc_shots.ps1:25 matches `^screenshot: `; H docs\agents\ui-testing.md:87.
**Splits:** None by value; four typed copies plus two producers in C++. They differ only if a verb changes one prefix.
**Ruling:** none found.
**Open:** none.

### L-F27-20
**Verdict:** CONFIRMED
**Homes as found:** H tools\capture_window.ps1:18-20 "Bump it whenever the C# block below changes", type `HydraCaptureV1` (11 uses in this one file). H docs\agents\ui-testing.md trap list says the same. Grep of H tools and H tests for `HydraCaptureV` finds only capture_window.ps1.
**Splits:** None. A rule held by prose in two places with no check; a changed C# body under the same name fails silently (the page says so).
**Ruling:** none found.
**Open:** none.

### L-F27-21
**Verdict:** CONFIRMED
**Homes as found:** H tools\doc_shots.ps1:27-47 (a `rect` command followed by `screenshot <file>` with exactly one argument; keeps the frame below the rectangle; k-th rect command takes the k-th printed ` rect=(` line). H tools\doc_shots\shots.txt:6-9 ("full-frame screenshot"). H docs\agents\doc-screenshots.md:15 ("a `rect` line directly before a `screenshot` line that has no ref"). H tests\ui\uitest_engine.cpp prints the rect text. H tests\test_docs_match_code.cpp:2032-2036 holds the picture list equal.
**Splits:** the three wordings ("no ref", "full-frame", "no bar") answer one question; the code regex `^screenshot\s+(\S+)\s*$` is the real test. Differ when a rect command prints no ` rect=(` line (item not found): the k-th pairing then shifts. Reading only.
**Ruling:** none found.
**Open:** a run with a failing rect.

### L-F27-22
**Verdict:** CONFIRMED
**Homes as found:** H tools\precheck\decisions.cpp:22-31 `kWholeNumberLimit = 1e15`, `kMaxHexDigits = 16`, `kBoldLevel = 99`; :84-92 `number_canon`; separators `, ' _` stripped at :99; hex bound :105. H tests\test_precheck_check3.cpp:106-111 pins `1'000'000`, `1,000,000`, `0xFFFF`, `2.5f`, `0x1Fu`. H tests\test_docs_match_code.cpp:360 `number` regex `(\d+(?:\.\d+)?)` and :553-558 `read_hex` (also 16 digits max, with or without 0x).
**Splits:** the docs-test regex reads `1,000,000` as three numbers; precheck reads one. They differ for a doc line writing a number with a separator or suffix. The hex length (16) agrees.
**Ruling:** none found.
**Open:** none.

### L-F27-23
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H tools\ch_mcp\build_facts.py:28 `DUMPS_ROOT = r"C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-dumps"`, :69-81 `current_build` returns the version file's string as written (the JSON files carry `1.1.0.6142`). H tools\ch_mcp\dump_facts.py:381 `DEFAULT_BUILD = "6142"` (bare). H tools\ch_probe\constants.py:24 "Clone Hero v1.1.0.6142". H tools\ch_mcp\launch_patch.json:3 `"build": "1.1.0.6142"`, :4 dump path `...\ch-dumps\6142\dump.cs` (repeats DUMPS_ROOT plus folder). H tests\test_docs_match_code.cpp:717-720 `kBuild11 = "6142"`, `kBuild10 = "4045"`. 108 matches for 6142 across 18 files under tools\ch_mcp.
**Splits:** two spellings of the build ("1.1.0.6142" from `current_build`, "6142" as folder name and DEFAULT_BUILD); they differ if a caller compares them directly (the dump folder name is derived by a rule not read here). The build number has a default typed in dump_facts.py though `current_build` answers "which build is installed" (K1).
**Ruling:** none found.
**Open:** how the folder name is derived from `current_build`; loader RVA copies not rechecked.

### L-F27-24
**Verdict:** CONFIRMED
**Homes as found:** H tools\ch_mcp\frida_tools.py:148-167 `ROW_ADDS_SCROLL` keys Confirm, Difficulty, Instrument, Modifiers; `NO_ROW_PAGE = "Ready"`. H tools\ch_mcp\menu_fields.json:14-20 holds the page enum 0 Confirm, 1 Instrument, 2 Difficulty, 3 Modifiers, 4 Ready. H tools\ch_mcp\tests\test_frida_tools.py:787-790 `test_every_page_word_has_a_row_rule` holds the word sets equal (read).
**Splits:** None; held equal by that test.
**Ruling:** none found.
**Open:** none.

### L-F27-25
**Verdict:** CONFIRMED
**Homes as found:** H tools\ch_mcp\kit_tools.py:718-725 `STOP_REASON_*` (six words) and `STOP_REASONS`; event words :709-716. H tools\ch_mcp\agent\press_schedule.ts:69-74 lists the six reasons in a comment; code emits them at :328 past_end, :410 done, :420 stopped, :461 cap, :464 jumped_back, :472 frozen; H tools\ch_mcp\agent\kit.ts:1121,1156 also passes `"frozen"` for the stall timer's stop.
**Splits:** (1) six words typed in TS code and Python; no test crosses the language line (tests\test_press_schedule.py not opened). (2) "frozen" has two TS producers: `stopFor` (clock held still) and kit.ts's stall timer (no frame came); they answer different situations with one word.
**Ruling:** none found.
**Open:** test_press_schedule.py not read.

### L-F27-26
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H src\core\model.cpp:34-41 `dynamic_str` ("none"/"ghost"/"accent"); :44-48 a display word made by capitalising it ("Ghost", "Accent"; empty for Normal per model.h comment). H tools\ch_mcp\kit_tools.py:258-264 `DYNAMIC_*` ("owner is dynamic_str"). H tools\ch_mcp\compare_scorestats.py:40-46 (copy, because the file runs alone). The finder's "second C++ spelling" at src\core\replay.h:140 is the score field names `"accent"`, `"ghost"` in `kReplayScoreFields` (a different question: score categories), not the dynamic words.
**Splits:** (1) two Python copies of the dynamic words plus the C++ owner; no test compares them. (2) replay.h's score field names and dynamic_str use the same words for two questions (score category vs a gem's dynamic) and differ only by coincidence.
**Ruling:** none found.
**Open:** none.

### L-F27-27
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H tools\ch_probe\constants.py:1-8 "the ONE place the numbers live ... nothing else hard-codes an address"; :36-44 `RVA_WINDOW_FORMULA = 0x20DDDA0`, `RVA_DRUMS_ENGINE_CTOR = 0x20DF680`. Copies of the same addresses: H tools\ch_probe\SPEC.md:71-123 (lowercase), H docs\clone-hero pages 03, 04, 06, 07 and code\6142\engine_ctor_20DF680.md, facts-6142.tsv, and tools\ch_mcp tests. H tests\test_docs_match_code.cpp:996-1020 holds each name in `kDataAddressNames` assigned in constants.py and every `RVA_*`/`OFF_*` named value (regex `named_value`) to a row of the 6142 table, so the finder's "listed names only" is too weak: named RVA and OFF constants are all compared.
**Splits:** (1) constants.py's "nothing else hard-codes" is not true of SPEC.md (an addendum doc) or the pages; the test does not read SPEC.md (grep of the test for SPEC.md: none). They differ if SPEC.md and constants.py are changed apart. (2) Values equal today by sampled reading (20DF680, 20DDDA0).
**Ruling:** none found.
**Open:** whether SPEC.md's other addresses match the table.

### L-F27-28
**Verdict:** CONFIRMED-CHANGED
**Homes as found:** H tools\precheck\check_rows.cpp:241-249 `row_kind`: A for a hit in a non-test file; in a test file E for owner tests/source_tree.h, C for an owner in another test, else B. :263-265 check 1 prints C, check 2 prints "BA", check 4 prints E. H docs\agents\derive-once-review.md:26-38 defines A to H by finding kind and says "The precheck's lines cover kinds B, C and E".
**Splits:** the code also prints kind A lines (check 2), which the page does not list as precheck coverage. The code letter says where a scan hit sits; the doc letter says what kind of copy a finding is. They agree on A (hit in production = rule written twice there) and differ for B: the doc's B is "a test that recomputes", the code's B is any test hit on a row owned by production.
**Ruling:** none found.
**Open:** none.

### L-F27-29
**Verdict:** CONFIRMED
**Homes as found:** H tools\compare_db.py:96-146 (docstring says table names, own ids and references are written out in the script; TABLES lists ten names). H src\store\record_store.cpp:272,281,290 CREATE TABLE profiles, records, record_paths; :853,866 charts, meta. H src\store\library_file.cpp:86-94 reads a legacy `results` layout (2.2.0 to 2.7.0 columns stars, legacy_fills, structure). grep of src for "CREATE TABLE" in H src\store\*.cpp gives only those five. H docs\adr\0026 is named in the compare_db docstring for the summary mode.
**Splits:** None as a live disagreement: compare_db.py compares older-layout files (results, paths, path_refs, songmeta, dynamics) that no src today creates; this-machine.md:85 names the five current tables. The two lists answer "which tables does this file have": current layout vs the layouts the tool can compare.
**Ruling:** none found.
**Open:** no src owner found for `results`, `paths`, `path_refs` creation (searched src\store CREATE TABLE; path_refs and songmeta in src only in analysis, dynamics_breakdown etc., not read).

## Ids not reached
None. All 25 rows were opened.
