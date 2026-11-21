# TOOL-AUDIT-T5-F21b (re-send finder, agent abad15fb6704f3476)

G = C:\Users\Patrick\.claude\gates, H = C:\Users\Patrick\Downloads\Hydra\hydra-test, C = C:\Users\Patrick\.claude.

## Rows I took (from F21's "not reached" and "partly done" lists)
I9-050, I9-051, I9-100, I9-102, I9-103, I9-105, I9-108, I9-117, I9-120, I9-127, I9-128, I9-130, I9-132, I9-134, I9-135, I9-138, I9-144, I9-158, I10b-089, I10b-103, I10b-116.
I did not reach in depth: I9-105 (only the owner and kind list were checked, not publish.ps1's flag letters), I9-135 (only the owner and docs were searched, not the comment reader's logic), I9-130 (only the public-page list homes). Not done at all: I9-100's "path match ignores case" detail beyond reading publish_gate.py; the memory notes were searched by keyword only.
What I searched: all of G (code, README, fragment, tests by grep), C\agents, CLAUDE.md, model-postures.md, tools\session_cards\common.py, the live settings.json, the memory notes (not archive), H\CLAUDE.md, H\docs\agents (with rulings), H\tools, H\tests\test_main.cpp and run_guard.h. A grep with a brace glob over C\projects\...\memory\ returned nothing at first; the direct folder grep worked, so the memory result below comes from the direct grep.

## Rulings found (user's own words, under ## Confirmed)
- H\docs\agents\rulings\review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues." (speaks to I9-100, I9-103, I9-105).
- review-and-hooks.md:22 "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire. send agents to compile a full plan" (speaks to the small-text limit behind I9-100/I9-103).
- review-and-hooks.md:18 "why do we need to go through this whole key process to begin with? why does the hook refuse to accept that a review happened because it didnt follow the exact expected format? this is just bad design" (speaks to I9-127/I9-128: typed `<key>` waivers, key shape).
- review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (allow phrase family).
- review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done." and :19 "a hand-back with no sign-off line records CLEAN and tells the main session." (speaks to I9-050/I9-051/I9-117: CLEAN default; the stop hook informs).
- review-and-hooks.md:24 "Agents have a soft cap of 20 minutes real time ... hard cap of 30 minutes real time." (I9-112/113 neighbours; I9-117 rides on it).
- review-and-hooks.md:30 chose "Git-side hook (Recommended)" to "How should the review gate stop unreviewed changes reaching main?" (I9-127, I9-158).
- review-and-hooks.md:13 chose "Raise cap to 3,050" to "... hook code is at 2,999 of your 3,000 cap ..." (see side note on LINE_BUDGET).
- agents-and-workflow.md:8 "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." (speaks to I10b-116's `all 3 build slots`).
- No Confirmed line holds "Yes, no click" (see I9-144).

## I9-100 When a long document passes the publish gate; I9-103 which tools the publish gate reads; I9-102 typed waiver for a long document
Question: when may a plan or Artifact page go out without a fresh review, and which calls does the gate look at.
Homes:
- OWNER of the code: G\publish_gate.py:14 `SMALL_LINES, SMALL_CHARS = 20, 2000`; :74-88 `_signed_off` (verdict for the key, or verdict whose recorded path is this file's path); :101 `review_rules.waived("doc-review", key, root)`; :110-123 `check` (ExitPlanMode with a plan; Artifact when action is none, "" or "publish", file_path set, asset not true).
- G\settings.fragment.json:174 matcher `"Artifact|ExitPlanMode"` and the live C\settings.json:184 (same matcher, copy by sync). The tool names are stated in the matcher and again in check(); a name added to one is not read by the other.
- G\README.md:63 "refuses to show a plan with ExitPlanMode, or publish an Artifact page, that no fresh agent signed off. A text passes when a CLEAN record holds its fingerprint, or when a CLEAN record names its file." and "`publish_gate.py` is also the one home of the short-text limit."
- H\docs\agents\doc-review.md:17 "Four steps are gated: a move of `main`, local or pushed, that changes `README.md`, `docs/README.md`, `docs/UserGuide.md` or `docs/differences-from-public-hydra.md`; an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub. A short text passes the last three with no review; `publish_gate.py` ... owns that limit. Once a fresh agent has signed off a version of a document, later edits to it pass with no new review. ... Text posted to GitHub is the exception: it passes only as the exact text a reviewer signed off." Also :21 and :102 repeat "later edits pass" and "exact text".
- H\CLAUDE.md "Doc review" section: "Only what outsiders read, and plans, get a review from a fresh agent, and each document gets it once."
- G\publish_gate.py:75 docstring "The user's rule (Hydra CLAUDE.md, "Doc review"): once a fresh agent signed off a version of a document, later edits pass."
- G\review_rules.py:128-129 `waived` and :26-28 `WAIVER_PHRASE`: the phrase is `waive doc-review` plus 7 to 64 hex; the refusal prints `waiver_phrase()` = 10 hex (review_rules.py:77-78).
- docs say the phrase as `waive doc-review <key>` (doc-review.md:21; CLAUDE.md names the waiver by pointer). README:37 says `<key>` too.
Table of what each home tells its reader:
| Home | Reader | Says |
|---|---|---|
| publish_gate.py | main session or agent | refused above 20 non-blank lines or 2000 chars with no CLEAN record; edits to a signed-off path pass |
| doc-review.md:17 | the sending session | same, plus GitHub text passes only as the exact text |
| README:63 | main session | same, names the limit's one home |
| settings fragment/live | Claude Code | calls the hook only for the two tool names |
| exactly at the limit | 20 lines and 2000 characters pass; 21 or 2001 refused (test_publish.py:100-117, T3 row I9-099) | prose pages say "short" or "large items over 20 lines" (ruling :22) with no number except the ruling |
| main vs agent | no difference in the gate; the typed waiver is the user's | doc-review.md:21 says only the user skips |
| card writers | not covered: the gate reads tool calls, not agent type | |
| worktree vs main | the gate judges the repo of the call's cwd; the marker is read from `refs/heads/main` of that repo (publish_gate.py:47); a plan in a worktree is judged by that repo's main marker | doc-review.md:17 "sits in the repo the gated call runs in" |
Kinds: K1 (tool names in matcher and in check()); K1 (20 lines in ruling :22, publish_gate.py:14, derive-once docs; limit owner named in README:63 and doc-review.md:17, so those are pointers); K3-ish: doc-review.md:17 "a move of main ... that changes the four public pages" is review_rules.PUBLIC_DOCS (a different gate from publish_gate) stated in the same sentence as the publish gate.
Split: T3's I9-103 says Artifact is skipped "when action is `list`"; the code (publish_gate.py:117) skips every action except none, "" or "publish", so any other action value (not only `list`) is skipped.
Test holding homes equal: test_sync_settings.py (fragment vs live, T3 I9-125); no test ties the matcher `Artifact|ExitPlanMode` to `check()`'s names (searched G\tests for ExitPlanMode: test_publish.py exercises check() only).

## I9-105 How GitHub text reaches gh
Homes: G\publish.ps1:1 and :8 (`ValidateSet('pr','release','release-edit','issue','issue-comment','pr-comment')`), :29 `Test-OtherTextFlag`, :49-50 Target/Title rules, :55-62 the switch. G\README.md:65 lists the six kinds and says publish.ps1 is "the one way text is posted to GitHub". H\docs\agents\doc-review.md:19 lists the same six kinds. H\CLAUDE.md "Doc review" names publish.ps1. settings.fragment.json holds the `gh` deny rules (README:73).
Split/gap: T3 I9-105 lists five kinds the tests cover; the script and both docs list six (`pr-comment` is in the docs and the script, not in the tests' list as T3 reports it). Kind list stated three times (script, README:65, doc-review.md:19); no test found that holds the docs' list equal to the script's ValidateSet (searched G\tests for "pr-comment": only test_publish.py usage not checked in depth).
Kinds: K1. Pointer/home: README:65 and doc-review.md:19 state the value, so they are homes.
Not searched in depth: flag letters (T3 I9-106 owns that).

## I9-108 How a reviewer dispatch is judged
Homes:
- OWNER: G\review_dispatch.py:16 `REVIEWER_TYPE = "reviewer"`, :19 `_KEY` (one 40-hex standing alone), :20-21 `_INSTEAD`, :22-27 `_ISOLATED`, :149-150 isolation refused, :160-162 one key only, :102-114 `_key_problem`.
- G\agent_stop.py:20 `REVIEWER_TYPE = "reviewer"` (a second code definition of the same string, not imported from review_dispatch).
- G\README.md:39 spells out the refusals and the reason for the isolation refusal.
- H\docs\agents\derive-once-review.md:17 "A hook refuses a reviewer dispatch with no key, a key already on `main`, a key git does not know or more than one commit"; :200 repeats the refusal list and the isolation reason ("An isolated agent may run git only in its own new worktree...; the hook refuses a reviewer dispatch that sets `isolation`").
- C\projects\...\memory\git-traps-in-shared-checkout.md:17 "A reviewer never sets `isolation` either... a hook refuses a reviewer dispatch that sets `isolation`".
- C\agents\reviewer.md:9 (brief gives "one commit, the branch's tip"), :33 (hook behaviour).
- Refusal text _INSTEAD says "write the base as the word main, not as a hash" and docs say the range is `main...<key>` (derive-once-review.md:17; doc-review.md:29).
- The key length (40) is stated in review_dispatch.py:19, README:39, derive-once-review.md:17/:200/:202, doc-review.md:29, handoff.md:19 (another rule: commit column).
Table:
| Home | Tells | To |
|---|---|---|
| review_dispatch.py | refuse isolation, 0 or 2+ keys, key on main, unknown key | the dispatcher at call time |
| README:39, derive-once-review.md:200 | spell the same refusals out | main session (K5: prose spells the rule out instead of pointing at the hook) |
| memory git-traps:17 | same, as a memory rule | main session (K5) |
| exactly at the limit | 40 hex alone passes; 39 or 41-hex runs are not keys (a 64-hex doc key is ignored, test T3 row) | docs say "the full 40-character hash" |
| main vs agent | the hook only judges `subagent_type reviewer`; a Workflow call or another agent type passes untouched, and derive-once-review.md:200 says "A reviewer launched from a Workflow script, or as any other agent type, gets no such record, so its sign-off counts for nothing." | |
| card writers | not reviewers; not touched | |
| worktree vs main | key is judged in cwd's repo, or in the one other repo root named in the brief (`judged_folder`, review_dispatch.py:65-99); derive-once-review.md:200 states this rule again in prose | |
Kinds: K5 (README:39, derive-once-review.md:200, memory:17 spell out the refusal list and reason); K6 (REVIEWER_TYPE defined in review_dispatch.py:16 and agent_stop.py:20; and the brief fingerprint is computed twice: review_dispatch.py:172 hashes `prompt` and agent_stop.py:199 hashes `first_user_message` text; a change to one breaks the match; no single function); K1 (40-hex rule).
Test holding homes equal: none found that ties agent_stop's REVIEWER_TYPE to review_dispatch's; test_agent_stop.py:497-547 (T3 I9-050) holds the hash match through real handover records.

## I9-050 Binding a stop to its handover; I9-051 the stop hook never blocks; I9-117 verdict problem relay
Homes:
- G\agent_stop.py:6-11 docstring ("bound to its handover record by the hash of its transcript's first user message ... it never blocks a hand-back"), :23 `_BLOCKED, _CLEAN` lines, :189-228 `_verdict`, :250-251 problem text "The merge stays refused until a reviewer's verdict is recorded."
- G\stall.py:268-281 `_verdict_problems`; :280 text "Check gates\state\agent_stop.jsonl for the tip that was signed. A move of main to a tip with no verdict is refused." (restates the ref gate's rule). stall.py:5-9 "Both only inform".
- G\README.md:29 (stall relays `verdict_problem` once "with the agent id and what to do"), :41-51 (agent_stop in prose: hash binding, "A saved hand-back always means finished", CLEAN default :43, the four steps), :51.
- G\settings.fragment.json:207-219 SubagentStop registration: timeout 20, no onFailure (this is the registration that makes "never blocks" true); G\common.py:52-70 `run(..., "loud")` (agent_stop.py:297 uses mode "loud").
- "never blocks / only informs" stated in agent_stop.py:10, stall.py:6, waiver.py:6, posture.py:8, README:29.
- SIGN-OFF lines: G\agent_stop.py:23 owns the text; copies in C\agents\reviewer.md:25-33, H\docs\agents\doc-review.md:117-125, H\docs\agents\derive-once-review.md:141-149, README:43.
- "A report with no sign-off line is recorded CLEAN" stated at agent_stop.py:21-22,109; README:43; reviewer.md:33; doc-review.md:125; and the Confirmed ruling review-and-hooks.md:19.
Table:
| Home | Says |
|---|---|
| agent_stop.py | no hand-back: no verdict, problem reported; no handover match: no verdict; neither line: CLEAN with a problem |
| reviewer.md:33, doc-review.md:125 | same, told to the reviewer ("write the line anyway") |
| main vs agent | verdict only for agent_type `reviewer`; main session and other agent types: nothing written (agent_stop.py:238) |
| card writers (Haiku agents, agent type other than reviewer) | agent_stop marks them finished only; no verdict path |
| worktree vs main | `cwd` of the handover decides which repo `signed_tip` asks (agent_stop.py:204) |
| exactly at the limit | n/a (no number) |
Kinds: K1 (CLEAN default, 6 homes, all agree today); K5 (reviewer.md:25-33 and docs spell the sign-off format; the hook owns it); K6 as above (two hash computations, two REVIEWER_TYPE); K7-lean: "never blocks" is stated in five code docstrings and the README; the registration without onFailure and mode "loud" are what enforce it, and no test found ties the claim to the fragment (the nearest, test_docs_numbers.py:163, is for the shell trim hook only).
Split: derive-once-review.md:202, reviewer.md:33 and doc-review.md:29 say the hook fingerprints "every markdown file" the range changes; agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")` and README:49 say markdown or HTML. The prose in three places is narrower than the code (K2).
Split: stall.py:280 says "A move of main to a tip with no verdict is refused." That is true only for gated repos (review_rules returns "no-gate" when no marker; review_rules.py:410-411), so the stall text states the rule without its condition (K3-lean).

## I9-120 Stall output never decides
Homes: G\stall.py:5-9 ("never a decision"), G\README.md:29 "It never refuses anything", G\wake.py/README:31 (the Stop hook wakes with exit 2, which is a decision by another channel: README:31 "writes it to stderr and exits 2, which wakes the session"), G\common.py:51 and :113-116 `check_id` (the ValueError for `..`, owned in common, used by stall.py:51 and :180). Test: test_stall.py:22-26,606-613 (T3). Nothing else found in H or C. One code owner; docs state "informs only".
Kind: K1 for "only informs" (stall.py:6, README:29), no split.

## I9-127 When may main move to a tip; I9-128 shape of a refusal; I9-130 which documents need a doc-review record; I9-132 which typed text counts; I9-134, I9-135 real-app and comments-only
Homes of the waiver phrase (question: what does the user type, and what is accepted):
- OWNER: G\review_rules.py:26-28 `WAIVER_PHRASE` (derive-once: 7 to 40 hex; doc-review: 7 to 64; real-app: 7 to 40); :77-78 `waiver_phrase` prints 10 hex; waiver.py:16-27 records what WAIVER_PHRASE accepts.
- Copies of the phrase: G\README.md:37 and :55 (`waive derive-once <key>`, `waive real-app <key>`); H\CLAUDE.md "Derive-once review" and "Doc review" (by pointer plus "waiver"); H\docs\agents\derive-once-review.md:189 "by typing `waive derive-once <key>` in chat. Never ask the user to waive one to save time."; H\docs\agents\doc-review.md:21 "`waive doc-review <key>`"; review-and-hooks.md Unconfirmed :58 (note quote, `"waive derive-once <key>" as the whole message`).
- Refusal text in review_rules.py:427,437,446,460 builds each phrase from `waiver_phrase`, so those are pointers.
Split (K4/K3): the docs say `<key>` and README:37 says "the user never computes one"; the accepted forms are 7 to 40 or 64 hex but the refusal prints exactly 10 hex; derive-once-review.md:189 says `<key>` where the key elsewhere in the same page (:17, :200) is the 40-hex tip, so a reader can read it as the full 40-hex key. Both work (prefix match, review_rules.py:120). No split in behaviour; the docs state the placeholder three different ways.
Homes of "which typed text counts" (I9-132): G\review_rules.py:89-113 `human_prompt_text` (`type` user, `turnOrigin` "human", not isMeta, not isSidechain, no toolUseResult, matching `promptId`); percall.py:34 and review_rules.py:127 call it. A SECOND code place decides the same question: C\tools\session_cards\common.py:372-402 `classify_user` (uses `origin.kind` == "human", `isMeta`, `isSidechain`+null parent, `toolUseResult`, compaction summaries, and for rulings `promptSource: typed` per T3 I9-141), used by G\ruling_rules.py via the card tools (ruling_rules.py:27-30). The two read different fields for "typed by the user" (`turnOrigin` versus `origin.kind`).
Kind: K6 (two code owners of "the user typed this"). Neither imports the other. Tests: test_review_rules.py:347-399 pins human_prompt_text; test_ruling_rules.py:180-219 pins the card tools' side; no test ties the two.
I9-127/I9-128 other homes: refusal text is built in review_rules.py:398-471; README:53 says "A refused move of `main` names its cause first ... then gives one `Instead:` line; `_code_cause` in `review_rules.py` owns the causes for code" (pointer, but it also states the shape: home). CODE_BRIEF/DOC_BRIEF at review_rules.py:20 name `docs/agents/derive-once-review.md` and `docs/agents/doc-review.md` (Hydra repo paths hard-coded in the hooks repo, a copy of file names; agent_stop.py:178 `asks_doc_review` matches on DOC_BRIEF). The refusal also names `hydra_start('window')` (review_rules.py:435), a Hydra tool name inside the hooks repo.
Table (main move):
| Home | Says |
|---|---|
| review_rules.explain_main_move | main moves only with CLEAN for the tip, a clean carry-merge, or a typed waiver; gated by marker on main |
| derive-once-review.md:189, :191 | same; "ref gate lets that merge onto main when git ... makes exactly the same tree" (a restatement of `_carries`, review_rules.py:234-244) |
| README:53 | same, plus "A push of local main ... is not asked again" |
| derive-once-review.md:189 | "Only the user can skip a review" |
| exactly at the limit | n/a; waiver prefix 7 hex passes, 6 hex does not (test I9-111) |
| main vs agent | agents cannot type a waiver (human_prompt_text needs a human turn) |
| card writers | no path |
| worktree vs main | the judge is per repo; derive-once-review.md:197 "its own worktree" |
Kinds: K1, K5 (derive-once-review.md:191 spells the carry rule; the hook owns it), K6 (above), K8-lean: review_rules.py:47-48 comment says hydra_ui_mcp.py "owns" REALAPP_FILE while review_rules.py:49 defines the same literal "realapp.jsonl" itself.

I9-130 (public-page list): OWNER G\review_rules.py:24 `PUBLIC_DOCS` (README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md). Copies: H\docs\agents\doc-review.md:17 (the same four, by name), H\docs\agents\public-doc-style.md:3 (same four plus "the GitHub release notes"), H\CLAUDE.md "Public doc style" ("the four public pages and the GitHub release notes"), G\tests\test_review_rules.py (old-key converter list, T3 I9-139, includes .doc-review-gate). The Confirmed ruling for the publish rule is review-and-hooks.md:27; "The pages outsiders read (the user's call, 2026-10-10)" at review_rules.py:23 names a date, not a Confirmed line. Kind K1. No test holds the three lists equal (searched G\tests and H\tools for UserGuide.md).

I9-134 / I9-135 (real-app): OWNER of the reader: G\review_rules.py:371-395 `realapp_checked`; UI_CODE at :31 (`^src/ui/`); :428-437 the rule in the refusal. OWNER of the writer: H\tools\hydra_ui_mcp.py:79-81 `REALAPP_FILE = "realapp.jsonl"` ("its field names are a contract") and :182-199 `realapp_record` (seven fields). Copies: G\review_rules.py:49 (same literal again), G\README.md:55, H\docs\agents\ui-testing.md:235-251 (field table with seven fields, "The field names below are that gate's contract"), G\tests\test_review_rules.py:404-405 (a hand-built record with the seven field names), H\tools\test_hydra_ui_mcp.py:519-532 (asserts the same field order).
Split (K3): H\docs\agents\ui-testing.md:10-11 says the real-app check before merge is for case 2 ("what the user sees": layout, width, spacing, alignment) and case 3 (report drawing); case 1 (behaviour and text) needs no real-app check. The hook (review_rules.py:428-437) demands a real-app record or `waive real-app` for any non-comment change under `src/ui/` (and BUILTIN_CODE/marker code path), whatever the case.
Test holding homes equal: none across the two repos (the writer's test and the reader's test each use their own copy of the field names; searched G\tests for hydra_ui_mcp: only a docstring).
I9-135 comment reader: one code owner (`_c_code`, `change_is_comments_only`, review_rules.py:262 on); README:55 states the rule in prose ("A file whose change touches only comments and blank lines is skipped ... Its code ... must be the same at both ends"). I did not compare the C++ comment rules to another reader (not searched: any C++ comment parser in H\tools).

## I9-144 Backfill route for old lines; I9-138 reflog replay; I9-158 judge failure
I9-144: OWNER G\ruling_rules.py:97-99 `INSTALL_DATE = "2026-11-14"` with the comment quoting the user's answer "Yes, no click" (2026-11-13, session ca29fda5). Copies: H\docs\agents\rulings\README.md:51 states "(`INSTALL_DATE` in `ruling_rules.py`, 2026-11-14)" and "The user approved it on 2026-11-13 ("Yes, no click")" and lists four conditions; ruling_rules.py:21-23 docstring states the conditions; test_ruling_rules.py uses `rr.INSTALL_DATE` (no copy). The words "Yes, no click" are not in any `## Confirmed` line (grep of H\docs\agents\rulings found them only at README.md:51), so the user's approval is cited in prose, in a code comment and in the rulings README, with no filed line. Kind K1 for the date (README:51 and code agree today); the date in README:51 is a stated value.
I9-138: the test T3 describes (reflog replay in test_review_rules.py:911-1013) is not in the file now (grep for reflog, GATES_REFLOG_REPLAY, reflog_moves in G\tests found only G\tests\replay.py:16,131 about reading main from the reflog for the Agent hook). So I9-138's row describes code that no longer exists, as F21 suspected. G\tests\replay.py:42 holds `REPOS = {"hydra": r"C:\Users\Patrick\Downloads\Hydra\hydra-test", "claude": r"C:\Users\Patrick\.claude"}`, and :41 `LINE_BUDGET = 3100`.
I9-158: OWNER G\ref_judge.py:31 `ALLOW, DENY` verdict lines, :38 `WATCHDOG = 60`, :390 "the judge ran over %d seconds". Copies: G\ref_gate.sh:15-16 (the two verdict lines as literal strings), :17 the refusal text "did not give a verdict (missing, crashed or cut off)"; G\install_ref_gate.ps1:13 literal `'ref-judge selftest: allow ok, deny ok'` against ref_judge.py:32 `SELFTEST_OK`. README:53,162 describe the shim in prose; nothing else states 60 s. Tests that hold them equal: G\tests\test_ref_judge.py:376-447 (the shim is run for real against a judge, so the two verdict lines are held equal end to end) and :610-614 (selftest text equal to the installer's). Kinds: K1 (verdict lines in judge and shim; selftest line in judge and installer), held by tests.

## I10b-089 Hand-listed known copies and spellings
Homes: H\tools\precheck\check_1.cpp:42-45 and check_2.cpp:1-4,82 (lists of fixture spellings with no scan row; "when a row lands, this entry goes"), scan rows in check_rows.cpp. This is the tool's own to-do note: the list is meant to shrink and there is no check that a row's arrival removes the entry. Nothing else found in G or C that states these spellings (searched G\tests, C\agents, docs\agents for "spellings"). One home each (two files); K7-lean (a rule, "an entry goes when a row lands", with no check).

## I10b-103 hydra_tests exit codes
Homes: H\tools\mutate\run.py:71-75 `EXIT_PASSED = 0`, `EXIT_FAILED = 1` ("hydra_tests' exit codes: doctest's own 0 and 1"), used at :470, :579-582, :938. OWNER of the codes in C++: H\tests\test_main.cpp:49-66 (returns `context.run()`'s code, or `run_guard::kRefusedExit`) and H\tests\run_guard.h:20 `kRefusedExit = 2`. H\docs\development.md:30,36,45 state "exit code 2" for a refused bare run, and :522 "exit code 3 ... retry later" (a different tool).
Split (K3-lean): run.py:579-580 reports any code other than 0 or 1 as "not doctest's own", so it does not know the guard's 2 is Hydra's own refusal code; run.py does not read `kRefusedExit`. No test ties them (searched H\tools\mutate\test_run.py for kRefusedExit: none). The shell hook's `[real exit code N]` (G\shell_output.py TAILS) is a different thing.

## I10b-116 Hydra UI MCP tests (test_hydra_ui_mcp.py, read now by test name, plus the classes BuildPassTool and the timeout tests)
Copies found in it:
- :683 `all 3 build slots are taken` and H\tools\test_build_pass.ps1:117 (same string): the count three. OWNER: H\tools\build_slot.ps1:75 `$BuildSlotCount = 3` (comment :71-74 points at D49 and `docs/agents/rulings/agents-and-workflow.md`). Other homes of three: H\CLAUDE.md:76 (rule 14), H\docs\agents\brief-preamble.md:57, H\docs\agents\agent-rules.md:20 (quotes the ruling), H\docs\development.md:114, G\README.md:15 (build pass), C\model-postures.md:11. The user's ruling: agents-and-workflow.md:8 (Confirmed, quoted above). Kind K1; two tests hold the owner's message text (both hard-code the "3"), not the count.
- :864-882 `wait 31 is longer than the 30 s timeout in force`, :874 default timeout 30: the default belongs to Hydra's headless command engine (not searched here). K1 to check by a later finder.
- :404-405 in the gates test (see I9-134) and :519-532 here pin the realapp field names.
- G\percall.py:20 `BUILD_PASS_TOOL = "mcp__hydra__hydra_build_pass"` and :108-111: the hook refuses that tool to every agent; H\docs\agents\brief-preamble.md:57 says "A hook refuses that tool to every agent" (K5: prose repeats a rule the hook enforces; pointer-like but states the rule). The text of percall.py:109-111 says "the main session builds your branch and runs any test that needs a build" (same rule as rule 14, a copy of the rule in a hook message).

## Splits (each on its own line)
1. derive-once-review.md:202, reviewer.md:33, doc-review.md:29 ("every markdown file") against agent_stop.py:163 and README:49 (markdown, .html, .htm). Code owner is broader than three prose copies.
2. ui-testing.md:10-11 (real-app check only for cases 2 and 3) against review_rules.py:428-437 (record or waiver for every non-comment `src/ui` change).
3. stall.py:280 "A move of main to a tip with no verdict is refused." against review_rules.py:410-411 (no marker on main: allowed).
4. review_rules.py:47-48 says hydra_ui_mcp.py "owns" `REALAPP_FILE`, and review_rules.py:49 defines the same literal again.
5. run.py:579-580 treats exit code 2 as "not doctest's own" while run_guard.h:20 makes 2 Hydra's own refusal code.
6. review_rules.human_prompt_text (`turnOrigin == "human"`) against the card tools' classify_user (`origin.kind == "human"`) for "did the user type this".
7. review_dispatch.py:16 and agent_stop.py:20 both define REVIEWER_TYPE; review_dispatch.py:172 and agent_stop.py:199 compute the brief's fingerprint separately.
8. waiver docs: `<key>` (derive-once-review.md:189, doc-review.md:21, README:37) against the 10-hex phrase the refusal prints (review_rules.py:77-78) and the accepted 7 to 40 or 64 hex (review_rules.py:26-28).
9. Rulings README:51 and ruling_rules.py:98 cite "Yes, no click" as the user's approval; no `## Confirmed` line holds it.

## Side notes
- G\review_rules.py:21-22 BUILTIN_CODE covers `src|tests|tools|third_party|installer|.github` plus CMakeLists.txt, CMakePresets.json, build_cpp.ps1, while the T3 row I9-129 describes the default as `src/` and `.github/workflows/ci.yml`; H\.derive-once-gate:2-4 still points at `hooks\lib\derive_once_rules.ps1` (F21 already noted).
- G\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget ... only the user changes it`. The Confirmed line review-and-hooks.md:13 says the cap was raised to 3,050; 3,100 appears only in an Unconfirmed line (review-and-hooks.md:44 "rase cap to 3100"). The code value is therefore not backed by a Confirmed line.
- G\README.md:15 and H\docs\agents\agent-rules.md:20 hold a copy of the build-count rule; C\model-postures.md:11 mentions build_pass.
- G\tests\test_docs_numbers.py:54 `README_NAMES` includes "realapp.jsonl": a test that holds README names against code (the only cross-check of that file name I found; it checks the README, not the writer in hydra_ui_mcp.py).
- Hook gap noticed: a typed waiver for a document kind is judged by prefix (7+ hex), so any key sharing that prefix is covered; I give no steps.
- Not opened: H\tools\test_build_pass.ps1 beyond line 117; H\tests wait constants behind the 30 s timeout.
