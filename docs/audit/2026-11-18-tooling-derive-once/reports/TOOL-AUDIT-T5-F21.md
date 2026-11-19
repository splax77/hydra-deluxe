# TOOL-AUDIT-T5-F21 (finder F21, agent ab22e77a3b3342e97)

Rows were worked in question groups, not one by one. Paths: G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test. I searched G (hooks, README, tests), tools\session_cards, agents\, CLAUDE.md files, H\docs\agents (with rulings\), H\.claude\commands, H\tools, H\docs\development.md and the decision log, and the memory notes (not archive). "None" below means those places were searched and nothing else was found.

## Rows done, by group

### Group A: the typed allow phrases. Rows I9-074, I9-083, I9-093, I9-094, I9-111, I9-133, I10b-026
Question: what exact text does the user type to allow one whole-library run or one key-route call, and how is it matched?

Homes (all quoted at file:line):
- OWNER of the text: G\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :54 `KEY_ALLOW_TEXT = "allow key run"`. Matching is `is_library_allow`/`is_key_allow` there. The comment at :50 says "a trailing . or ! is allowed".
- Hook messages use the constant (pointer only): G\percall.py:53-55 and :93-96 build the refusal from `review_rules.LIBRARY_ALLOW_TEXT` / `KEY_ALLOW_TEXT`. waiver.py holds no literal (grep of G non-test files finds the phrases only in review_rules.py, percall.py by constant, README.md).
- Copies that state the phrase:
  - G\README.md:13 ("unless the user has typed `allow library run`"; "unless the user has typed `allow key run`"; "each typed allow lets one call through ... one `allow key run` serves any one counted call"), :37 (lists both phrases; says `is_library_allow` and `is_key_allow` own the exact match, so :37 is partly a pointer).
  - H\docs\agents\agent-rules.md:14 ("The user allows one run by sending exactly `allow library run`; each such message lets one command through.")
  - H\docs\agents\brief-preamble.md:49 (same, plus "each such message lets one command through" and the two guards).
  - H\docs\agents\clone-hero-mcp.md:29 ("refuses each such call unless the user has typed exactly `allow key run` in chat in this session, and each such message lets one call through"). It also names percall.py as the home.
  - H\docs\agents\this-machine.md:91, audit-method.md:125 (the library phrase, with "CLAUDE.md rule 8").
  - H\tools\ch_mcp\server.py:148, :466, :477 ("hook refuses them unless the user has typed allow key run") and H\tools\ch_probe\experiments\play_chart.py:1281. These are text an agent reads from the MCP tool description.
  - The `ch` MCP server instructions in the system prompt: "a hook refuses them unless the user has typed allow key run" (same text as server.py:148).
  - Unconfirmed ruling H\docs\agents\rulings\review-and-hooks.md:47 (under `## Unconfirmed`, so a home): "The user kept the typed allow library run for whole-library runs."
  - Memory note ch-runs-must-not-need-focus.md:17 mentions the key phrase (long line, not opened in full).
  - Tests that pin the literal: G\tests\test_review_rules.py:839-866 (`KEY_ALLOW_TEXT == "allow key run"`; library phrase by literals), test_progress.py:427-637, test_review_dispatch.py:309-336, test_posture.py:197-246. None of them imports the constant for the library phrase; only the key phrase has an equality test against the constant (test_review_rules.py:852).
- The project CLAUDE.md rule 8 and the global CLAUDE.md do NOT state the phrase (rule 8 says "explicit permission, asked for in chat first").

What each home tells its reader:

| Reader / edge | What the homes say |
|---|---|
| User | type the exact phrase (agent-rules:14, preamble:49, clone-hero-mcp:29, hook refusal text) |
| Exactly at the limit | owner: whole message, case-insensitive, one trailing `.` or `!`, extra mark refused (review_rules.py:50). Docs say only "exactly". test_progress.py:455/569 pins `!!` refused, `Allow Library Run!` accepted. Docs never state the trailing-mark rule; the hooks README :37 says "An allow must be the whole message". |
| Main session vs agent | README:13: refused "in the main session and in every agent". Preamble:49 and agent-rules:14: "the main session and every agent". Agree. Preamble:49 adds a hook text "unless the user has sent exactly `allow library run`". |
| Agent type that skips the preamble (card writers, scouts like me) | The refusal is by hook, so no preamble needed; but only brief-preamble.md:49 and agent-rules.md:14 tell an agent about the allow. agents\*.md (scout, reviewer, planner, executor, card-writer) hold no mention (grep "allow" found only `disallowedTools`). |
| Worktree vs main checkout | none stated; allow is per session (README:13 "An allow from another session never counts"). |

Kinds: K1 (many copies of the phrase, all agree today). K5 for the MCP text/server.py and clone-hero-mcp.md, which spell the lock rule (what is locked, one call per message) instead of pointing at the hook. K7 not claimed: the owner is named (review_rules.py).
Splits: none found. A narrow gap: the docs say "exactly" while the owner accepts case changes and one trailing mark. Not a conflict, but the copies state a stricter rule than the owner.
Confirmed ruling that speaks to it: H\docs\agents\rulings\review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (session dee9c61e). It covers the library phrase only. Nothing under `## Confirmed` covers `allow key run`.
Test that holds homes equal: test_review_rules.py:852 (key phrase constant vs literal). No test holds the docs equal to the constants (test_docs_numbers.py checks budget, bash limit and output cap numbers, not these phrases).

### Group B: the "small text" limit for publishing. Rows I9-049, I9-099, I9-104 (I9-105 partly, see not reached)
Question: how long may a plan, page or GitHub text be before it needs a reviewed sign-off?
Homes:
- OWNER: G\publish_gate.py:14 `SMALL_LINES, SMALL_CHARS = 20, 2000`; :31-33 counts UTF-16 units and non-blank lines. :5 says the file is "the one home" of the rule.
- Pointers: G\publish.ps1:3, G\review_rules.py:67 (comment), H\docs\agents\doc-review.md:17 ("`publish_gate.py` in the gates folder owns that limit"). No number stated in them.
- Test copies: G\tests\test_publish.py:101 (`lines(21)`), :107-117 (literal 20 lines and 2000 chars, 2001 refused). G\tests\test_agent_stop.py:31 `NOTE` is `range(25)` with the comment "past publish_gate's small-text limit" (a second home of the idea, with 25 not derived from SMALL_LINES).
- A Confirmed ruling: H\docs\agents\rulings\review-and-hooks.md:22 "... only large items over 20 lines should go for review ..." (session b5e8d95a). The code's 20 lines matches this. The 2000-character cap is in no ruling I found (searched the decision log and rulings for "2000" with no hit; not exhaustive).
- Also the hooks-from-scratch plan line 108 (required reading) says "The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` and nowhere else in code": those are the progress budget, not this 20.
Table: user/main/agent all read the same hook; docs give no number (good). Exactly at the limit: owner = 20 lines and 2000 characters pass, 21 or 2001 refuse; test_publish.py:107-117 pins both. Card writers: no instruction about this. Worktree vs main: not applicable.
Kinds: K1 (test literals 20/2000/25 copy the owner), K4-free. The 2000-character half has no user words behind it in the rulings I read (K7-adjacent; I do not grade).
Splits: none. Held-equal test: none (tests pin literals, not SMALL_LINES; `test_agent_stop.py:31` uses 25).

### Group C: which repos and files the review gates cover. Rows I9-101, I9-129, I9-130, I9-139
Questions: which repos are gated; which paths are code; which pages are public docs.
Homes:
- OWNER: G\review_rules.py:18 `CODE_MARKER, DOC_MARKER`; :22 `BUILTIN_CODE` = `src|tests|tools|third_party|installer|.github` folders plus `CMakeLists.txt`, `CMakePresets.json`, `build_cpp.ps1`; :24 `PUBLIC_DOCS` (README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md; comment "the user's call, 2026-10-10").
- Public page list copies: H\docs\agents\doc-review.md:17 (all four named); H\docs\agents\public-doc-style.md:3 (same four plus release notes); H\CLAUDE.md:45 ("the four public pages", count only); the test list in G\tests\test_review_rules.py:947-949 (the old converter; the report says the reflog replay was deleted by commit c27420cf, not re-verified).
- Marker files: H\.derive-once-gate says "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file." H\.doc-review-gate says "decided by the user-scope hooks (hooks\lib\doc_review_rules.ps1)". The folder C:\Users\Patrick\.claude\hooks does not exist (ls fails). The owner today is review_rules.py.
- Inventory row I9-129 says the default code paths are "`src/` and CI setup `.github/workflows/ci.yml`". The code (review_rules.py:22) is wider (tests, tools, third_party, installer, CMake files, build_cpp.ps1, all of .github/). The test only exercises src/ and ci.yml.
- G\README.md:138: the gates repo's own marker has `gate:` lines naming `gates/`, `agents/`, `tools/session_cards/`.
- H\docs\agents\derive-once-review.md:21: "The precheck owns that threshold and which files count; this page never restates them" (pointer).
Table: public-page list: owner and doc-review.md:17 agree today. Code paths: no doc found that lists them (searched CLAUDE.md, derive-once-review.md, agent-rules.md, this-machine.md for src/tests/tools/third_party/installer; only CLAUDE.md:49 "touch code"). Main vs agent: the gate is on `main` moves, same for both. Worktree vs main: marker is read from `main`, a worktree branch's own marker does not count (per README; not quoted).
Kinds: K8 for both marker files (H\.derive-once-gate and H\.doc-review-gate name a hooks\lib .ps1 file as owner; that file is gone). K1 for the public list (four copies). K6: none. The test copy of the public list (test_review_rules.py:947-949) is a K1 test-side copy.
Splits: one inventory-row mismatch (I9-129's own text vs review_rules.py:22), flagged for the main session to check the row, not a split between homes. Marker text vs real owner is a K8.
Confirmed rulings: none speak to the code-path list or the marker text; PUBLIC_DOCS comment cites "the user's call, 2026-10-10". Confirmed H\docs\agents\rulings\review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context ..." relates to the doc gate.
Held-equal test: none between the three public-page lists.

### Group D: what a reviewer's hand-back records. Rows I9-045, I9-046, I9-047, I9-048, I9-049 (also), I9-050, I9-051
Questions: how a sign-off is read, which commit it signs, which documents are fingerprinted.
Homes:
- OWNER: G\agent_stop.py:23 (`_BLOCKED, _CLEAN`), :105-109 (a missing line records CLEAN with a problem text), :161-172 (`DOC_SUFFIXES = (".md", ".html", ".htm")` plus the doc marker), `signed_tip` (named in README:45 as owner).
- Copies: G\agents\reviewer.md:28-33; H\docs\agents\derive-once-review.md:146-151 and :202; H\docs\agents\doc-review.md:120-125 and :29; G\README.md:45, :49, :63.
Differences (reader text vs owner):
- "fingerprints every markdown file the range changes" in reviewer.md:33, derive-once-review.md:151, :202, and "documents are every markdown file the range changes" in doc-review.md:29. The owner fingerprints `.md`, `.html`, `.htm` files and the `.doc-review-gate` marker. README:49 states this correctly and names `DOC_SUFFIXES` as the owner. K2 for the four doc lines (they state a narrower set than the owner). reviewer.md:33 does add "and the `.doc-review-gate` marker" but not HTML.
- "Write no hash, key or path with it" (derive-once-review.md:149, reviewer.md:31) vs I9-045 (old Key:/Path: lines ignored). Consistent. test_docs_numbers.py:91 `test_review_docs_name_no_typed_key` holds the docs free of typed keys.
- A report with no sign-off line is recorded CLEAN: agent_stop.py:109, derive-once-review.md:151, reviewer.md:33; Confirmed ruling H\docs\agents\rulings\review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (session 4672d33d). Home and ruling agree.
Table: reviewer agent: writes `SIGN-OFF: CLEAN` or `BLOCKED`; main session: reads the hook's note; card writers: not relevant. At the limit: the hook finds "SIGN-OFF: BLOCKED" first (agent_stop.py:105) so a report holding both is BLOCKED; docs do not say that.
Kinds: K2 (markdown-only wording), K1 for the sign-off line shape (4 homes), K5 for derive-once-review.md:149-151 (spells how the hook reads the report instead of pointing at agent_stop.py).
Held-equal test: none for the sign-off wording or the doc suffix list.

### Group E: rulings and cards. Rows I9-141, I9-142, I9-144, I9-162
Questions: what words the words test needs; what approval needs; the backfill route; where transcripts and cards are read.
Homes:
- OWNER: G\ruling_rules.py (:95 `CONFIRMED, UNCONFIRMED`, :99 `INSTALL_DATE = "2026-11-14"`, :100 `APPROVE, DECLINE = "File it", "Do not file"`, :101 `QUESTION`, :104 `LINE`, :108 `CHECK_COMMAND`, :236-237 `card_rulings`).
- Card section names: owner is G.. no, tools\session_cards\common.py:51-57 (`HEADINGS`, `WORDS_HEADING` "The user's words", `OLD_WORDS_HEADING` "Rulings"). ruling_rules.py reads through `cards.parse_card` and has no literal (comment only at :237). Copies of the names: H\CLAUDE.md:25 ("a section headed "The user's words" in new cards, "Rulings" in older ones"); tools\session_cards\writer-brief.md:52, :67 (card-writer brief); tests (test_session_cards.py:228-297); memory notes relay-status-in-plain-english.md and derive-display-from-engine-truth.md mention one of the two words (not opened).
- Question text and labels: H\docs\agents\rulings\README.md:41 states "`File this ruling in docs/agents/rulings/<topic file>?`, followed by the whole line. The two labels are `File it` and `Do not file`" (a K1 copy of ruling_rules.py:100-101). Also H\CLAUDE.md:41, H\docs\agents\agent-rules.md:33, G\README.md:59.
- Install date: H\docs\agents\rulings\README.md:51 repeats `INSTALL_DATE` and "2026-11-14" and the user's answer "Yes, no click"; ruling_rules.py:97-99 repeats the same session id (ca29fda5) in a comment. K1.
- Check command: H\docs\agents\rulings\README.md:40 `ruling_rules.py --check <file>` (pointer, no path); G\settings.fragment.json allow rule; test_docs_numbers.py:137 (`test_fragment_allows_the_ruling_check`) holds the fragment equal to `CHECK_COMMAND`.
- "up to four lines, one question each" (rulings README:41) is a limit of the question-box tool, not of any hook. K4-adjacent; I did not find its owner in our files.
Table (rule: "a ruling is filed only on click"): user: clicks "File it"; main session: asks the box and commits; agents: their transcript never counts (README:43, agent-rules:33, ruling_rules.py:27-29). Card writers: only writer-brief.md:67 states what the section is; "a card quote is not a filed ruling" is repeated in CLAUDE.md:25 and writer-brief.md:67. At the limit (backfill): line dated before 2026-11-14 passes by the backfill route (ruling_rules.py:335-347); README:51 says the same date.
Kinds: K1 (section names, labels, install date, question text), K5 (README:41-43 spells the approval steps that ruling_rules.py enforces; the README says it is the format owner per the report row).
Splits: none.
Held-equal: test_docs_numbers.py:137, :129-133 check parts of the rulings README (not the labels).

### Group F: the ref gate. Rows I9-156, I9-157, I9-158
Homes: OWNER G\ref_judge.py (:99-103 rewrite refusal, :113 symbolic, :135 shared = main or `refs/remotes/origin/<name>`, :169-171 origin tips, :38 `WATCHDOG`), G\ref_gate.sh:15-17 (the `ref-gate-verdict` line and "did not give a verdict"). Copies: G\README.md:53, :57; H\docs\agents\brief-preamble.md:65, :71 ("refuses a move of `main`, or of any branch origin also has, that is not a plain step forward"); H\docs\agents\agent-rules.md:27, :33; H\CLAUDE.md:49 and derive-once-review.md:189.
Table: main vs agent: same shim for both. Exactly at the limit: a branch is refused when `shared` (`main` or a branch with an origin copy) or pushed; the preamble states "any branch origin also has", and ref_judge.py:135 reads `refs/remotes/origin/<name>`, with pushed-but-not-tracked branches also covered via ls-remote (:169-231). The preamble's one-line summary omits the pushed case. Worktree vs main: not stated.
Kinds: K1 (preamble:71 vs ref_judge.py), K2 minor (preamble omits push-checked case).
Splits: possible tension inside G\README.md between :53 ("a push to `main` of anything else is asked as before") and :57 ("A push is not judged again, because the commits it sends were judged when the local branch moved"). :57 is in the rulings paragraph, so it is likely about the ruling check only, not a conflict. I report it as read, not as a split.

### Group G: the precheck and the test picker. Rows I10-084, I10-088, I10b-076, I10b-082, I10b-083, I10b-087, I10b-088
- I10-084: OWNER H\tools\precheck\test_pick.h:25-30 `kCommonNameLimit = 30`, `kPickSizeLimit = 40`, `kHeaderIncluderLimit = 20`, with the comment "They live here and nowhere else." Copies stating the numbers: H\docs\development.md:558-563 ("Q1 30, Q2 40, Q3 20", then "They live in `tools/precheck/test_pick.h` and nowhere else"), H\docs\audit\2026-10-03-fix-decisions.md:1646 (D299, word for word, the user's words), H\tests\test_precheck_pick.cpp:75-81 (`CHECK(kCommonNameLimit == 30)` etc. = test pins the literals; line 79-81 also checks the defaults struct equals the constants). K1 (development.md and the test repeat the numbers); the doc says "nowhere else" while stating them. Held-equal test: tests\test_precheck_pick.cpp:75-81 (literals against the constants). No conflict.
- I10b-087: which test files are picked: OWNER tools\precheck\test_pick.cpp:26-34,178,320. Copy: H\docs\development.md:542-544 ("a build file, a test-harness file or a header with too many direct includers"; says "`shared_code` in test_pick.h owns the exact rule") and 553-570 (the picker's rules). Mostly pointers. K1 only for the "HYDRA_<TARGET>_EXE" detail (development.md:570 and test_pick.cpp:318, :430).
- I10b-076: the five checks and their kind letters: OWNER tools\precheck\checks.cpp:7-13. Copies: H\docs\agents\derive-once-review.md:21 (which kinds the precheck finds), :28-36 (kind A..H meanings), :38. The kind letters and their meaning are stated in both; derive-once-review.md says "Seven are listed here" and lists A, B, C, D, E, F, H (7); G is described under "One question beyond copies" (not read). K1 (two homes of the kind meanings: checks.cpp and derive-once-review.md).
- I10b-088: kWhy "(derive-once review, step 8)" in check_5.cpp:30-33. derive-once-review.md:49 step 8 is "Check new homes". They agree today. K1/K8-free (a step number copied into code: if the page's step order changes, the code's pointer is wrong).
- I10b-082 / I10-088 / I10b-083: "0, 1, 2 are counts" and "1000, 1024 and 1000000 next to * or / are unit factors" in check_3.cpp:427-432, :490 (cites D49). The decision log H\docs\audit\2026-10-03-fix-decisions.md:154 states both rules (home), and D49's first paragraph (:151) says the user "chose to record these as they are". Another home: tools\precheck\check_2.cpp:125, :138 and definitions.h:18-23 (D49 cited for the namespace look-back of 200 characters; the decision log :154 also states "200 characters"). So the same D49 number list (0/1/2, 1000/1024/1000000, 200 chars, 30 tokens, 4 lines, 500/2-line reach, 3 lines above) is stated in the decision log and code; D49 text names "tools/derive_once_precheck.ps1" as the home of some of them (that script: not checked for existence in this run). K1; K8 possible for the D49 file reference (not checked).
Tables: only the main session or reviewers read these; agents are told "this page never restates" (derive-once-review.md:21) which is the pointer shape.
Splits: none found. Confirmed rulings: the picker limits are the user's words in D299 (quoted in the decision log, not under `## Confirmed`); development.md:558 quotes them too.

### Group H: real-app record, handoff checker. Rows I10-061, I10-024, I10-026
- Real-app record file name and fields: OWNER H\tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` and `realapp_record`. Second literal: G\review_rules.py:49 `REALAPP_FILE = "realapp.jsonl"` with the comment ":47-48 Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them." Doc copy: H\docs\agents\ui-testing.md:233-251 (seven fields, four endings write the record, "A crash, a nonzero exit, a silent Hydra the helper killed ... writes nothing") and G\README.md:55. K6/K1: two code places hold the file name; the gates copy points at the Hydra one but does not read it. I9-134's field list (kind, commit, dirty, commands, shots, started, stopped) agrees with ui-testing.md:239-249 today.
 Held-equal test: none found between the two names (searched H\tools\ and G\tests by name; test_hydra_ui_mcp.py not read).
- Checker pass rule (I10-024): OWNER H\tools\check_handoff.py:391-393 (exit 0 only if every status but ok/manual is 0; no rows = exit 1). Copies: H\docs\agents\handoff.md:57 ("exits 0 only when every row the checker judges (every row but `manual`) is `ok`; it exits 1 otherwise and 2 on a bad argument"), :39, :55, :67; H\.claude\commands\handoff.md:12; H\docs\agents\brief-preamble.md:17 and :97. K1. A small K2: handoff.md:57 says "every row but `manual` is ok" while the code also lets `ok` and `manual` in the sum but treats "unknown" as a failure, which agrees; and the preamble :97 says "each Claims row not marked ok or manual" (agrees). No conflict.
- Docs excluded when proving a function exists (I10-026): check_handoff.py:90-97 only; H\docs\agents\handoff.md and the preamble do not repeat it (searched "no proof", "docs excluded"); one home.

### Misc rows touched lightly. Rows I10b-103, I10b-107, I10b-114
- I10b-107 WAIT_HOME `tests/wait_util.h`: owner H\tools\mutate\timeout_split.py:46. Copies: the tool's own docstring :22 and test fixtures test_timeout_split.py:7-180; H\docs\agents\ui-testing.md:267 names `testwait::kUitestProcessCap` in `tests/wait_util.h` (pointer to the file, a separate concern). One code home; docs point.
- I10b-103: H\tools\mutate\run.py:74-75 `EXIT_PASSED = 0`, `EXIT_FAILED = 1`; used in run.py and test_run.py. The shell hook's "COMMAND FAILED, exit code N" (preamble) is another thing. No second home found in tools\. Not searched in tests\test_main.cpp.
- I10b-114: H\tools\test_release_sign.ps1:1-12; H\docs\development.md:958 mentions release_keygen.ps1. One home for the check; the test and keygen name the same files.

## Rows not reached (searched too thinly to report; the main session should treat them as open)
I9-100, I9-102, I9-103, I9-105, I9-108, I9-117, I9-120, I9-127, I9-128, I9-132, I9-135, I9-138, I9-158 (only the shim line read), I10b-089, I10b-116 (the scout itself did not read test_hydra_ui_mcp.py). Partly done: I9-050, I9-051, I9-130, I9-134, I9-144, I10b-103.
For I9-127/I9-128 specifically: the docs quote the waiver phrase `waive derive-once <10 hex>`; I did not search the other homes of that phrase (waiver and refusal text are in review_rules.py WAIVER_PHRASE, :25-27).

## Side notes
- G\README.md:57 "A push is not judged again" vs :53 "a push to main of anything else is asked as before": read as two different checks; worth a one-line re-read by the main session.
- H\.derive-once-gate and H\.doc-review-gate: both marker files send the reader to hook files under `hooks\lib\` that no longer exist (K8 above).
- G\tests\test_agent_stop.py:31 uses 25 lines as "past publish_gate's small-text limit" (another copy of the 20-line rule's idea).
- H\docs\agents\clone-hero-mcp.md:29 states "the `allow key run` hook in `C:\Users\Patrick\.claude\gates\percall.py`"; the phrase is owned by review_rules.py, percall.py only checks it (a mild K8).
- A hook gap I noticed: the key-route lock lists specific tool names in percall.py (`KEY_TOOLS`), so a key-pressing tool not on that list would not be covered. I give no steps.
- The earlier reflog-replay test and old-hooks reader were deleted by commit c27420cf (per row I9-192); I9-138 and I9-139 describe the version before it and I did not verify the current file.
