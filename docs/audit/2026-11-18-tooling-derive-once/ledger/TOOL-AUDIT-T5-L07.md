# TOOL-AUDIT-T5-L07 ledger (finders F19, F20, F21; re-sends F20b, F21b)

Compiled from the ten reports only (F19, V19, F20, V20, F20b, V20b, F21, V21, F21b, V21b). No file searched again. Where verifier and finder differ, the verifier's reading is used and said so. Abbreviations: DOR = Hydra docs/agents/derive-once-review.md; DR = doc-review.md; IN = integrate.md; UT = ui-testing.md; AR = agent-rules.md; PRE = brief-preamble.md; G = C:\Users\Patrick\.claude\gates; H = Hydra repo; MEM = memory folder; RR = G\review_rules.py. "Rulings" = lines under `## Confirmed` in docs/agents/rulings/.

## F19 rows

### L-F19-01: Does a finding the reviewer cannot fix hold the merge, or is it only passed on in Notes?
- **Source:** I6-041, I6-042, I6-061; F19 SPLIT-1; V19 (SPLIT-1).
- **Kind:** K2, K3.
- **Verdict:** CONFIRMED.
- **Homes:** MEM one-reviewer-rule-d107.md:11 "go in the reviewer's Notes; pass them on, don't hold the merge ("notes are fine")". DOR:106 the orchestrator "does not hold the merge for them, with three exceptions, which hold the merge until the user answers" (not-asked visible change, kind H, kind D; a Note starting "Holds the merge:"). DR:98 "The main session passes your Notes to the user. It does not hold the publish for them." (finder only: no exceptions listed, shorter not-fixable list than DOR:106). DOR:202 "If its Visible changes marks any change not asked, hold the merge" (finder only). CLAUDE.md:74 / AR:18 rule 12 (finder only). MEM approved-work-is-not-reasked.md (finder only).
- **Splits:** (a) MEM d107:11 says do not hold; DOR:106 says hold for kind H, kind D and "Holds the merge:" Notes. They differ for a kind H or kind D finding the reviewer cannot fix. (b) DOR:106 vs DR:98: DR:98 has no merge-holding exceptions and a shorter list (finder only; V19 did not recheck).
- **Ruling quotes:** review-and-hooks.md:39 ("alright approve b"): "Under (b), a kind H finding or a kind D rule from that check that the reviewer cannot fix holds the merge until the user answers, the way a "not asked" visible change does." Names the DOR side only. No Confirmed line says "notes are fine"; the memory note is not the user's words. agents-and-workflow.md:19 (finder only): "if anything changes a display in the app or a record, run it by me."
- **Tests:** none named.
- **Open:** none beyond the above.
- **Hook gap:** none stated. K7 side note (finder only, not verified): no hook reads a "Holds the merge:" Note (G\agent_stop.py:21 comment names only sign-off lines).

### L-F19-02: What is a review keyed on: the exact tree, or the branch-tip commit (with a carry for a clean merge of main)?
- **Source:** I6-117, I6-118, I8-076, I8-125, I8-252; F19 SPLIT-2; F20 S7; V19, V20.
- **Kind:** K2, K3, K8.
- **Verdict:** CONFIRMED.
- **Homes:** IN:54 "The gate keys a review on the exact tree, so any later commit voids it." DOR:191 a merge of main passes when git's merge of old main and a signed-off commit "makes exactly the same tree". RR `_carries` (review_rules.py:233-246) compares merge-tree to the new tree. G README:39, DOR:202, MEM git-traps :18 and one-reviewer-rule-d107.md:15 say the key is the branch tip commit (40-hex). review_dispatch.py:17 the key is a commit hash (finder). V20 read only the call sites of `_carries` (lines 234, 251, 431), so the "equal tree" detail is unchecked there; V19 did check it.
- **Splits:** IN:54 (tree, any later commit voids) vs DOR:191 and the code (a conflict-free merge of main, and a fix review's fix per DOR:173-183, pass without a full review). Differs for a later merge of main into a reviewed branch.
- **Ruling quotes:** none found (V19: no Confirmed line for either side).
- **Tests:** G\tests\test_review_dispatch.py, test_agent_stop.py test the code; none ties the sentences.
- **Open:** none.
- **Hook gap:** none stated. Side (finder only): IN:56 "so you cannot review this range yourself" has no hook check of author vs reviewer (K7).

### L-F19-03: Which report-window tests does ui-testing.md list?
- **Source:** I7-029; F19 SPLIT-3; V19.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED (V19 did not check that each of the 24 names is a registered test case, only that the names appear).
- **Homes:** UT:145 lists 5 (`report-window-sort`, `-filters`, `-states`, `-keys`, `report-windows-both`); UT:147 lists 7 (`-open-path`, `-dm-handover`, `-row-click`, `-reopen`, `-hidden-lets-go`, `-reopen-keeps-filters`, `-out-of-date`). tests/ui/uitest_report_windows.cpp holds 24 distinct names; 12 in neither list: cant-analyze-list, cells-left, chip-label, chip-outline, close-button, column-widths, fits-screen, fresh-state, header-narrow, hidden-new-result, open-analyzes, open-asks.
- **Splits:** the page presents its two groups as the full set; the file registers 12 more. Differs for any report-window test outside the two lists.
- **Ruling quotes:** none found.
- **Tests:** none holds the list equal (finder searched "report-window").
- **Open:** whether all 24 are registered cases.
- **Hook gap:** none.

### L-F19-04: Budget numbers (100/150 calls, 20/30 minutes, 5/10 status) stated in many prose homes
- **Source:** I6-034, I6-038, I6-085; F19 Cluster A, SPLIT-4; V19 (SPLIT-4 not re-opened).
- **Kind:** K1, K3, K5.
- **Verdict:** NOT VERIFIED (V19 did not reopen SPLIT-4 or Cluster A; finder itself calls SPLIT-4 drift, not a value split).
- **Homes:** finder only. Owner: G\progress.py:12-14 "The one home of these numbers". Prose: PRE:73, AR:13, DOR:110, DR:100, DOR:90, DR:91, reviewer.md:11, model-postures.md:5, tools\session_cards\writer-brief.md:101, MEM one-reviewer-rule-d107.md:11/15 ("the reviewer cap is 150 calls like everyone else", no wrap-up point), G README:124.
- **Splits:** finder only: MEM d107:15 omits the 100/20 wrap-up point that the other homes state; writer-brief.md:101 gives card writers only the 20/100 point, not 150/30.
- **Ruling quotes:** review-and-hooks.md:24 (finder only) "Agents have a soft cap of 20 minutes real time ... a hard cap of 30 minutes real time." The 100/150 call counts have no Confirmed line (finder).
- **Tests:** G\tests\test_docs_numbers.py (call counts in CLAUDE.md, AR, PRE, DOR, DR; status interval). Not the minutes in DOR/DR, model-postures, reviewer.md, writer-brief.md.
- **Open:** not reached by a verifier.
- **Hook gap:** none stated.

### L-F19-05: Which files does a reviewer's sign-off fingerprint: "every markdown file" or markdown plus HTML pages?
- **Source:** I6-049, I9-050 area; F19 Cluster C; F21 Group D; F21b split 1; V19, V21, V21b.
- **Kind:** K2.
- **Verdict:** CONFIRMED (three verifiers agree).
- **Homes:** G\agent_stop.py:163 `DOC_SUFFIXES = (".md", ".html", ".htm")`, used :172 and :226, plus the `.doc-review-gate` marker. G README:49 "a markdown file or an HTML page, since Artifact pages are HTML". Prose "every markdown file the range changes": DOR:151, DOR:202, DR:29, C\agents\reviewer.md:33 (V21b adds DOR:151 as a fourth copy; reviewer.md:33 also adds the marker but not HTML).
- **Splits:** the four prose copies name a narrower set than agent_stop.py:163. Differs for an HTML/Artifact page in a reviewed range.
- **Ruling quotes:** none found.
- **Tests:** none for the suffix list vs the prose.
- **Open:** none.
- **Hook gap:** none.

### L-F19-06: Which rule decides that a merge of main into a reviewed branch may land (two code places)?
- **Source:** I6-054; F19 Cluster D; V19.
- **Kind:** K6, K1.
- **Verdict:** CONFIRMED-CHANGED. V19: the two places are not the same rule. `review_rules._carries` scans every commit in base..new for a commit k that passes review, then merge-tree(base,k) equals new's tree. `ref_judge.made_from_existing` (ref_judge.py:305-316; comment "the same carry rule as review_rules._code_review") needs exactly two parents with parents[0]==cur and merge-tree of the two parents equal to new's tree, and also returns True when any other ref already contains new. It never asks whether a parent passed review, so its comment overstates.
- **Homes:** review_rules.py:233-246; ref_judge.py:305-316 (V19); DOR:191 prose; G README (no line opened by finder).
- **Splits:** `_carries` vs `made_from_existing`. Differ for a two-parent merge of an unreviewed commit, and for a carry where the reviewed commit is not the second parent.
- **Ruling quotes:** none found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none stated.

### L-F19-07: What does the user type to waive a review, and does the text say "key" consistently?
- **Source:** I6-053, I6-075, I9-127, I9-128, I9-132; F19 Cluster D; F21b split 8; V19 (I6-075).
- **Kind:** K3, K2.
- **Verdict:** CONFIRMED for I6-075 (wording edge, not a value split). F21b split 8 (the `<key>` placeholder vs the 10-hex print): NOT VERIFIED (V21b: "not checked in depth"; F21b itself says no split in behaviour).
- **Homes:** DR:21 "The brief names the branch tip and the range, and no key for the document" / "nobody computes or types a key for it", then the same paragraph "Only the user can skip a review, by typing `waive doc-review <key>`". G\publish_gate.py:107 prints the key in its refusal. DOR:189 `waive derive-once <key>`; G README:37; review_rules.py:26-28 WAIVER_PHRASE (derive-once 7-40 hex, doc-review 7-64, real-app 7-40); review_rules.py:77-78 prints 10 hex (finder only).
- **Splits:** none in behaviour. Wording: the docs say `<key>` three ways; the refusal prints 10 hex; accepted lengths vary by kind.
- **Ruling quotes:** none found. F21b: review-and-hooks.md:18 "why do we need to go through this whole key process to begin with? ... this is just bad design" (finder only), not naming the phrase.
- **Tests:** G\tests\test_docs_numbers.py test_review_docs_name_no_typed_key did not catch DR:21's phrase (finder).
- **Open:** none run-dependent.
- **Hook gap:** hook gap: yes, G\review_rules.py (finder only, F21b side note: a waiver is judged by prefix of 7+ hex, so any key sharing the prefix is covered). Not verified.

### L-F19-08: Which UI changes need a real-app check, and what clears it?
- **Source:** I7-001, I7-002, I7-003, I10-061, I9-134, I9-135; F19 Cluster I; F21 Group H; F21b I9-134/135; V19, V21b (split 2).
- **Kind:** K2, K3.
- **Verdict:** CONFIRMED (V19) and CONFIRMED-CHANGED (V21b): UT itself states the hook's wider rule at UT:235, so the page holds both readings.
- **Homes:** UT:10 (case 2) real-app check for "A layout, a width, spacing, alignment, or what the screen does when a song opens or closes"; "If the user says to merge before this check is done, say plainly that it is still owed." UT:9 case 1 headless replaces screenshots; UT:11 case 3. UT:235 the record is "a gate on merges that change `src/ui/`" (V21b). RR:31 `UI_CODE = ^src/ui/`; comments-only skipped (:367-368); :428-437 refuses a move to main with no real-app record unless the user types `waive real-app <key>`. G README:55. MEM render-checks-use-the-corpus.md (finder).
- **Splits:** UT:10-11 (visual cases only; clears with "still owed") vs RR:428-437 (any non-comment src/ui change; clears with a record or a typed waiver). Differ for a behaviour-or-text-only src/ui change, and for "just merge".
- **Ruling quotes:** ui-and-display.md:71 "wow you really fucked this up. did you never once do a UI test?"; tests-and-runs.md:18 "you dont need a full library run to check the rendering. this is exactly the kind of time wasting i am trying to avoid." Neither names a trigger or waiver. UT:9's "repeatedly taking screenshots" is under Unconfirmed (finder; V19 did not recheck).
- **Tests:** tools/test_hydra_ui_mcp.py:578-646 (record writer); test_review_rules.py:401-427 (gate); none holds UT:10 equal to UI_CODE.
- **Open:** none.
- **Hook gap:** none stated.

### L-F19-09: Which library charts may a live Clone Hero run play?
- **Source:** I7-078; F19 Cluster I; V19.
- **Kind:** K7.
- **Verdict:** CONFIRMED (one home, nothing checks it). V19 read only the first part of CHM:31 (Bomber, Mouths Like Sidewinder Missiles); that the rest matches the six names is the finder's claim.
- **Homes:** H docs/agents/clone-hero-mcp.md:31 only. V19 grep of tools/ch_mcp/*.py for allowed_song, Bomber, Sidewinder: no hit.
- **Splits:** None.
- **Ruling quotes:** clone-hero.md:24 (2026-11-12, "Human run, allowed"): question text names "the test chart plus six library charts you named (Bomber, Mouths Like Sidewinder Missiles, Rescued, Ghost Of Indecision, Get Ur Freak On, Yandere Complex)". The quotes "yes all create handoff" and "Yes, all recommended" in CHM:31 appear in no Confirmed line (finder).
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none stated.

### L-F19-10: How long may a text be before it needs a reviewed sign-off, and is the 2000-character half the user's?
- **Source:** I6-069, I9-049, I9-099, I9-100, I9-104; F19 Cluster E; F21 Group B; F21b I9-100/103; V19, V21b.
- **Kind:** K4, K7, K1.
- **Verdict:** CONFIRMED (V19).
- **Homes:** G\publish_gate.py:13-14 `SMALL_LINES, SMALL_CHARS = 20, 2000` ("both limits must hold"); :31 UTF-16 units limit; :33 non-blank lines. Pointers only: DR:17, G README:63, publish.ps1:3, review_rules.py:67 comment. Test literals: test_publish.py:100-117; test_agent_stop.py:31 `NOTE = range(25)` (finder).
- **Splits:** None between homes. The 2000-character limit has no Confirmed line.
- **Ruling quotes:** review-and-hooks.md:22 "only large items over 20 lines should go for review". V19 searched that file for 2000/2,000: no hit.
- **Tests:** test_publish.py pins 20/2000 literals, not the constants.
- **Open:** none.
- **Hook gap:** none.

### L-F19-11: Hook names for the publish gate: matcher vs `check()`, and which Artifact actions are skipped
- **Source:** I9-103; F21b I9-100/103; (F19 not involved).
- **Kind:** K1.
- **Verdict:** NOT VERIFIED (V21b: "not reached").
- **Homes:** finder only. G\settings.fragment.json:174 matcher `"Artifact|ExitPlanMode"` and live settings.json:184; publish_gate.py:110-123 `check` (Artifact when action is none, "" or "publish"; line 117 skips every other action).
- **Splits:** finder only: T3's I9-103 says Artifact is skipped when action is `list`; the code skips every action but none/""/publish.
- **Ruling quotes:** review-and-hooks.md:27 (every publish goes to a fresh agent).
- **Tests:** test_sync_settings.py (fragment vs live); no test ties the matcher to `check()`'s names.
- **Open:** not reached.
- **Hook gap:** none stated.

### L-F19-12: Is a plan reviewed only when shown, or "every plan before the user sees it"?
- **Source:** I7-071, I8-131; F19 Cluster E; F20 I8-131.
- **Kind:** K3, K7.
- **Verdict:** NOT VERIFIED (V19, V20 did not reach).
- **Homes:** finder only. DR:17 "committed plans ... are not gated"; a plan shown with ExitPlanMode is gated; MEM plans-reviewed-before-shown.md "Every plan goes on a branch and past a fresh reviewer before the user sees it, in chat too"; CLAUDE.md:55; G README:63. F20: the memory claim that a plan pasted into chat slips past has no home.
- **Splits:** finder only: memory "every plan ... before the user sees it" vs DR:17 "committed plans are not gated".
- **Ruling quotes:** none found.
- **Tests:** none named.
- **Open:** not reached.
- **Hook gap:** none stated (a gap in general words, finder only: pasted-in-chat plans).

### L-F19-13: A report with no sign-off line counts as CLEAN (homes and who holds what)
- **Source:** I6-048, I9-044, I9-045 to I9-051, I9-117; F19 Cluster C; F20 I9-044; F21 Group D; F21b I9-050/051/117; V21 (partial).
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED in part (V21: agent_stop.py:105-109 confirmed: BLOCKED found first, no line records CLEAN with a problem text; ruling line :19 not re-read by V21). Remaining claims NOT VERIFIED.
- **Homes:** G\agent_stop.py:23 `_BLOCKED, _CLEAN`; :105-109. Prose: DOR:151, DR:125, G README:43, reviewer.md:33, DR:98, DOR:86. F21b: stall.py:268-281 text (finder only).
- **Splits:** None between sign-off homes. Finder-only: stall.py:280 "A move of main to a tip with no verdict is refused." (see L-F21-06).
- **Ruling quotes:** review-and-hooks.md:19 "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (finder only). review-and-hooks.md:20 "i asked for the review agent to fix its findings directly, then we're done."
- **Tests:** G\tests\test_agent_stop.py:302-336 (code); no test compares the prose.
- **Open:** none run-dependent.
- **Hook gap:** none.

### L-F19-14: Which test commands does the merge run, and how are the scan and docs tests selected?
- **Source:** I6-040, I6-115, I8-007, I8-098, I8-100, I7-204 to I7-207; F19 Cluster F; F20 S1, S9; V20 (S1, S9).
- **Kind:** K1, K2, K5.
- **Verdict:** CONFIRMED for S1; CONFIRMED-CHANGED (weak) for S9 (V20: the MEMORY.md:20 index line sums up an older rule than its note).
- **Homes:** IN:43 `-tc="single-owner*"`, IN:44 `-sf=*docs_match_code*`. AR:7 docs `"-tc=docs match code*"`, scan `"-sf=*single_owner*"`. CLAUDE.md:57 and G\session_cards_start.ps1:285 `-tc=docs match code*`. MEMORY.md:20 "grep tests for changed files; whole suite only for shared code" vs MEM merge-tests-touch-changed-files.md:15 (run what hydra_precheck prints, "Never type the list by hand"). Owner of flags: tools/precheck/main.cpp:4-12,53-57.
- **Splits:** (a) two filter spellings for the same two tests on IN vs AR/CLAUDE.md; main session reads either. (b) MEMORY.md index line vs its note.
- **Ruling quotes:** None found.
- **Tests:** none holds the commands equal (finder).
- **Open:** whether `-tc="single-owner*"` matches a real test case name needs a test listing (not run).
- **Hook gap:** none.

### L-F19-15: Filing a ruling: labels, question text, install date, and "Yes, no click"
- **Source:** I9-141, I9-142, I9-144, I9-162, I6-219, I6-221, I6-222; F19 Cluster G; F21 Group E; F21b I9-144; V21b (split 9).
- **Kind:** K1, K5, K4.
- **Verdict:** CONFIRMED for the "Yes, no click" point (V21b: grep finds it only at rulings README.md:51). Other claims NOT VERIFIED (V21b did not re-read ruling_rules.py's comment).
- **Homes:** G\ruling_rules.py:99-101 `INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`, `QUESTION`; :335-347 backfill. Prose: rulings README:38-43, :51; CLAUDE.md:41; AR:33; G README:59. Card section names: tools\session_cards\common.py:51-57, CLAUDE.md:25, writer-brief.md:52,67. "One box may hold up to four lines": no code home found (finder only).
- **Splits:** None between homes. The 2026-11-13 approval ("Yes, no click") is cited in rulings README:51 and a ruling_rules.py comment but is in no Confirmed line.
- **Ruling quotes:** None found for "Yes, no click".
- **Tests:** test_ruling_rules.py, test_docs_numbers.py:129-137 hold the fragment and parts of the README; none holds the date or labels in the README equal to the code.
- **Open:** none.
- **Hook gap:** none.

### L-F19-16: Which pages count as public pages (four named pages)?
- **Source:** I6-067, I6-253, I7-181, I9-130, I9-101, I9-139; F19 Cluster E; F20; F21 Group C; F21b I9-130; V20, V21b.
- **Kind:** K1, K6.
- **Verdict:** CONFIRMED (V20: lists agree today).
- **Homes:** G\review_rules.py:24 `PUBLIC_DOCS` (README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md), comment "The pages outsiders read (the user's call, 2026-10-10)". DR:17 (all four). public-doc-style.md:3 (four plus release notes). CLAUDE.md:45 (count only). Test list G\tests\test_review_rules.py:947-949 (finder). H tests/test_docs_match_code.cpp kPicturePages (a different list, F20).
- **Splits:** None.
- **Ruling quotes:** review-and-hooks.md:27 "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues." The "2026-10-10" date is not a Confirmed line.
- **Tests:** none holds the lists equal (V20: not searched; F19/F21b: none found).
- **Open:** none.
- **Hook gap:** none.

### L-F19-17: Which paths are "code" for the review gate, and what do the marker files say owns the rule?
- **Source:** I9-101, I9-129; F21 Group C; F21b side note; V21.
- **Kind:** K8, K1.
- **Verdict:** CONFIRMED (V21). I9-129's row text: V21 did not find it, so what the row says is unchecked.
- **Homes:** H\.derive-once-gate names "Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1)"; H\.doc-review-gate names "(hooks\lib\doc_review_rules.ps1)". V21: C:\Users\Patrick\.claude\hooks exists but holds only `state`; no `lib`. Live owner G\review_rules.py (markers :18, BUILTIN_CODE :22: src, tests, tools, third_party, installer, .github, CMakeLists.txt, CMakePresets.json, build_cpp.ps1). Inventory row I9-129 says the default is `src/` and `.github/workflows/ci.yml`.
- **Splits:** Two stale owner pointers (not two live answers). Row text I9-129 vs code is a row mismatch, not a split.
- **Ruling quotes:** none found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F19-18: GitHub posting: kinds list and which gh commands are denied
- **Source:** I6-072, I6-225, I7-143, I7-210, I9-105; F19 Cluster E; F20; F21b I9-105; V20.
- **Kind:** K1, K6, K7.
- **Verdict:** PARTLY CHECKED by V20 (publish.ps1 line 1 lists the six kinds; DR:17 covers text posted to GitHub; deny lines and test_publish.py not opened). N2 PARTLY CONFIRMED (issue-tracker.md:47 says "gist + link"; V20 found no deny rule for gist in settings.json, noting a grep miss does not prove none exists). F21b I9-105: NOT VERIFIED (V21b: not reached).
- **Homes:** G\publish.ps1:1,8 ValidateSet pr, release, release-edit, issue, issue-comment, pr-comment; DR:19 (six kinds); G README:65; settings.fragment.json:9-15,23-29,37-43 and live settings.json:19-25,33-39,47-53 (finder); issue-tracker.md:42,47 tell agents to run `gh issue edit` and `gh issue close` by hand (finder: not on the deny list); this-machine.md:15; CLAUDE.md:53. Test literal `splax77/hydra-deluxe` in test_publish.py:341-393 (finder only).
- **Splits:** None stated.
- **Ruling quotes:** none found.
- **Tests:** G\tests\test_publish.py (publish.ps1 vs publish_gate.py), test_docs_numbers.py test_live_settings_carry_the_fragment. None holds DR:19's list equal to the ValidateSet (F21b).
- **Open:** not reached beyond the above.
- **Hook gap:** hook gap: yes, G\settings.fragment.json (finder only; some gh commands that change issue text or state, and gist posting, appear not to be on the deny list; V20 could not prove absence).

### L-F19-19: Typed allow phrases for a library run and a key-route call, and where they are stated
- **Source:** I7-076, I8-021, I8-022, I9-074, I9-083, I9-093, I9-094, I9-111, I9-133, I10b-026; F19 Cluster I; F20; F21 Group A; V21.
- **Kind:** K1, K5, K8.
- **Verdict:** CONFIRMED (V21: owner and copies match; reviewer.md lives in C:\Users\Patrick\.claude\agents\, not G\agents, a correction to F21).
- **Homes:** G\review_rules.py:51 `LIBRARY_ALLOW_TEXT = "allow library run"`, :54 `KEY_ALLOW_TEXT = "allow key run"`; comment :50 "a trailing . or ! is allowed"; matching case-insensitive. percall.py uses the constants (no literal). Copies: G README:13, :37; AR:14; PRE:49; clone-hero-mcp.md:29 (also names percall.py as home; finder says the phrase is owned by review_rules.py, mild K8); this-machine.md:91; audit-method.md:125; tools\ch_mcp\server.py:148,466,477; tools\ch_probe\experiments\play_chart.py:1281 (finder); the ch server instructions (finder). MEM ch-runs-must-not-need-focus.md:15 adds "Never start a key-driven run while the user may be using the machine; ask first" (finder).
- **Splits:** None. Docs say "exactly"; the owner accepts case changes and one trailing mark (V21: not a conflict). MEM no-full-library-runs.md:15 "No hook enforces it yet" contradicts percall.py (see L-F20-02).
- **Ruling quotes:** review-and-hooks.md:16 (Confirmed) "i want to be able to give explicit approval for a full library run." Covers the library phrase only. Nothing under Confirmed covers `allow key run`. Line 47 ("The user kept the typed allow library run") is under Unconfirmed.
- **Tests:** test_review_rules.py:852 holds the key phrase equal to its constant; test_progress.py, test_review_dispatch.py, test_posture.py pin literals. No test holds the docs equal to the constants.
- **Open:** none.
- **Hook gap:** hook gap: yes, G\percall.py (finder only, F21 side note: key-route lock lists specific tool names in `KEY_TOOLS`). Not verified.

### L-F19-20: Single-home or example rules in the review and test-rubric pages (no second home found)
- **Source:** I6-027, I6-032, I6-035, I6-036, I6-037, I6-039, I6-050 to I6-052, I6-059, I6-063, I6-076 to I6-080, I6-084, I6-137, I7-110 to I7-118, I7-119 to I7-125, I7-140, I7-169, I7-172, I7-179; F19 Clusters B, D, E, H, J; F20.
- **Kind:** K1, K7, K8 (mostly "single home", some K1).
- **Verdict:** NOT VERIFIED (V19 and V20 did not reach these).
- **Homes:** finder only. One home each: DOR:94/100 (one finding per commit), DOR:157-183 (resolution and fix reviews), DR:3, DR:25, DR:51-63, DR:83, TR:84, TR:205, test-rubric-examples.md:336, DOR:28/74 (kind G). Two-home items: Pitch Dark worked example DOR:57-64 vs tests/test_replay_schedule.cpp:1526-1579 (numbers agree today; no test reads DOR); ADR 0029 fields vs tests/single_owner/engine.cpp:91-105 (agree today); DOR:82 vs fix-decisions.md:297 (D59); DOR:86 vs D59 item 1; audit-method.md:81-87 vs CLAUDE.md:33 vs TR:189; TR:90 vs TR:141-142 vs test-rubric-examples.md:319; TR:59/98-109 verdict vocabulary vs audit-method.md exit numbering (TR:103 points at "exit 2", audit-method.md has no word "Investigate"); TR:183 helper rule vs test-rubric-examples.md:151-159 vs PRE:117 (line-number pointers in TR:183 and examples may drift); plan reviewer checks premise-map.md:74 vs reviewer.md:13 (reviewer.md adds Calls and rulings checks); owner-map.md:13 vs CLAUDE.md:55 / AR:25 (short forms drop the "no map for docs, hooks or file moves" exception); owner-map.md:47 vs reviewer.md:17.
- **Splits:** finder only: TR:88 treats "floor" as a failing stand-in while rulings (clone-hero.md:20) approve a floor; K3 edge, not a split. TR:103 vs audit-method.md exit wording (possible K8, unchecked). Splits: None verified.
- **Ruling quotes:** code-and-engine.md:50,51 (golden) found, not a home. clone-hero.md:20 "Its frame-rate floor is 322.2 frames a second" (finder).
- **Tests:** none named.
- **Open:** all of it not reached by a verifier.
- **Hook gap:** none.

### L-F19-21: Public-doc voice, banned words, format and structure rules (no check, several homes)
- **Source:** I7-182 to I7-188, I6-084; F19; F20.
- **Kind:** K1, K7.
- **Verdict:** NOT VERIFIED (V20 not reached).
- **Homes:** finder only. public-doc-style.md:11-31, :39-49, :53-63, :67-73, :77-79, :83-87; CLAUDE.md:45; ~/.claude/CLAUDE.md:5,8 (a shorter banned-phrase list); MEM every-number-says-where-it-comes-from.md; MEM hydra-deluxe-rename.md and MEMORY.md:26 vs public-doc-style.md:13 (release notes before 2.0.0 say "Hydra": exception only in PS). DR:83 vs public-doc-style.md:83 (different actors). Nothing in the gates matches the banned words (finder searched *.py/*.ps1).
- **Splits:** finder only: two banned-word lists with different contents (public-doc-style.md vs ~/.claude/CLAUDE.md), no check on either.
- **Ruling quotes:** ui-and-display.md:127 is under Unconfirmed (finder).
- **Tests:** none.
- **Open:** not reached.
- **Hook gap:** none.

## F20 rows

### L-F20-01: How many charts does testdata/input hold?
- **Source:** S2; V20.
- **Kind:** K2, K8.
- **Verdict:** CONFIRMED (V20: 98 notes.chart/notes.mid files in the main checkout, 116 chart/mid files; the unit explains both numbers and no page says which unit).
- **Homes:** ui-testing.md:46,48 "98"; AR:14 "about 115"; MEM no-full-library-runs.md:11 "about 115".
- **Splits:** UT 98 vs AR/memory "about 115". Differ by what is counted (songs vs chart/mid files).
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F20-02: Is the whole-library rule enforced by a hook?
- **Source:** S3, I8-113, I8-114, I8-117, I8-250; V20.
- **Kind:** K3, K8.
- **Verdict:** CONFIRMED (S3). The three-lists point (what counts as a full run) is NOT VERIFIED.
- **Homes:** MEM no-full-library-runs.md:15 "No hook enforces it yet." percall.py:14 defines the library flag and its line 4 says it runs that check (V20). G README:13. The note also says "unless it is truly necessary and the user has said yes"; AR:14 says explicit permission in chat. Three lists of what counts as a full run (finder only): I8-114's, AR:14's, PRE:49's.
- **Splits:** memory "No hook enforces it yet" vs percall.py. Memory "unless truly necessary and the user has said yes" vs AR:14 "explicit permission".
- **Ruling quotes:** review-and-hooks.md:16 "i want to be able to give explicit approval for a full library run." (from F21; a memory note is not the user's words).
- **Tests:** not opened.
- **Open:** none.
- **Hook gap:** none.

### L-F20-03: What is the early-fill slack constant called?
- **Source:** S4, I8-062; V20.
- **Kind:** K8.
- **Verdict:** CONFIRMED.
- **Homes:** MEM early-fill-orange-85-d250.md:11 names `kEarlyFillSlackMs` (60 ms) read by `is_e0`; MEMORY.md index "slack 60". V20: grep of src, tests, tools finds no such name. src/core/model.h:106-115 has `kHitWindowMs` 85, `kDefaultEarlyFillWindowMs` 60 (comment "default 60ms, supports 0ms to 85ms", 2026-11-15), `kMaxEarlyFillWindowMs`. `is_e0` exists at model.cpp:471.
- **Splits:** note's name vs code names (stale name).
- **Ruling quotes:** code-and-engine.md:23 "change the early fill search to allow orange timing up to 85ms." (2026-10-15); :56 "make the early fill threshold an adjustable setting pinned to one number in both directions" (2026-11-15). They bear on the numbers, not the stale name; neither names a split.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F20-04: Is a run that beats the engine's optimal score a bug or normal?
- **Source:** S5, I8-045, I8-051, I8-203; V20.
- **Kind:** K3.
- **Verdict:** CONFIRMED (cases may differ).
- **Homes:** skill analyzing-sp-squeezes SKILL.md:55 "a run that beats the engine's optimal is a version problem or a bug, never a real score". MEM dmleaderboards-comparison.md:24-29 "Above-optimal scores are NORMAL, not a bug... never flag as impossible/error". public-doc-style.md:17 "Rows above optimal are expected". MEM derive-display-from-engine-truth.md:25 (I8-045, single home).
- **Splits:** SKILL.md vs dmleaderboards note and public-doc-style.md. May be different cases (own video or CH run vs leaderboard rows); no page says which.
- **Ruling quotes:** None found.
- **Tests:** none named.
- **Open:** which case each page means.
- **Hook gap:** none.

### L-F20-05: Which GitHub repo name is current?
- **Source:** S6; V20.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** MEM hydra-deluxe-rename.md:3 description "now patseguin1-commits/hydra-deluxe"; its line 11 says it moved the same day to `splax77/hydra-deluxe` (current). this-machine.md:15, project CLAUDE.md:13, `git remote` say splax77.
- **Splits:** the note's description line vs its own line 11 (stale description).
- **Ruling quotes:** None found.
- **Tests:** test_publish.py:341-393 uses `splax77/hydra-deluxe` as a literal (finder).
- **Open:** none.
- **Hook gap:** none.

### L-F20-06: Does the stop hook read documents "from the merge base with main", a fault or the rule?
- **Source:** S8, I8-082, I8-083; V20.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** MEM hooks-never-guess.md:13 calls "from the merge base with main" an assumption that stamped 362 unread pages CLEAN. agent_stop.py `documents()` runs `merge-base refs/heads/main tip` (V20); G README:49 "from the merge base with `main`".
- **Splits:** note calls it a fault; code and README state it as the rule. Differs for a branch stacked on another branch.
- **Ruling quotes:** none in any Confirmed line V20 read.
- **Tests:** none named.
- **Open:** none.
- **Hook gap:** none.

### L-F20-07: Display-test text on test-rubric-examples.md says the Preview fix both landed and is still to come
- **Source:** S10, I7-140; V20.
- **Kind:** K2.
- **Verdict:** CONFIRMED.
- **Homes:** test-rubric-examples.md:9 "The Preview fix those four asked for has since landed. The Preview's own refill-tick function is gone". Lines 330-336 "the test to write in the same change as the Preview fix". TR:90 "now removed".
- **Splits:** same page, opposite tense.
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F20-08: Release version number copied outside CMake
- **Source:** I7-144, N1; V20.
- **Kind:** K1.
- **Verdict:** CONFIRMED (agree today; V20 did not search for a test holding them equal).
- **Homes:** CMakeLists.txt:21 `project(Hydra VERSION 3.1.2`; this-machine.md:17 "3.1.2" ("version number is set in one place, the project line of CMakeLists.txt"); MEM windows-installer.md:20.
- **Splits:** None.
- **Ruling quotes:** None found.
- **Tests:** none found (finder searched gates\tests, tools).
- **Open:** whether a test exists.
- **Hook gap:** none.

### L-F20-09: Does tool work (the video tools) get a reviewer before it counts as done?
- **Source:** I8-072; F20b; V20b.
- **Kind:** K5, K7.
- **Verdict:** CONFIRMED (V20b).
- **Homes:** MEM fc-video-workflow.md:17 "tool work ran as one Opus implementer plus one Sonnet reviewer that reproduced every done-when itself ... Keep the reviewer gate". review_rules.py:18 marker `.derive-once-gate`; :21-22 BUILTIN_CODE includes `tools/`; :153-154 `_has_marker`; :408 code gate on only if a rev has the marker. V20b `ls` of C:\Users\Patrick\Downloads\Hydra\video-tools: own .git, no `.derive-once-gate`. DOR:86 softer standard for `tools/` comments (finder). AR rule 5 / CLAUDE.md:67 no "reproduce every done-when" (finder).
- **Splits:** the note's "reviewer gate" for video-tools vs a hook that is off there; Hydra `tools/` has the hook. "Reproduce every done-when" has one home (the note).
- **Ruling quotes:** None found (V20b grep of rulings for "reviewer gate", "tool work", "video-tools").
- **Tests:** none.
- **Open:** what the hook does on a push in the video-tools repo needs a run.
- **Hook gap:** hook gap: yes, G\review_rules.py (finder only: a repo without the marker has no review hook).

### L-F20-10: Does "never a second review" admit the D314 exception?
- **Source:** I8-083, I8-122, I8-124; F20 (partial); F20b; V20b.
- **Kind:** K3.
- **Verdict:** CONFIRMED-CHANGED (V20b: finder read no Confirmed lines; V20b read them and added S4).
- **Homes:** MEM hooks-never-guess.md:17 "removing guesses never reverses a user decision or adds a second review (D107)"; MEM one-reviewer-rule-d107.md:11 "ONE fresh reviewer ... no second round" (no D314 hit by grep). AR:11 "one narrow exception (D314)"; CLAUDE.md rule 5 same; DOR:175 "narrow exception to D107, and the user approved it in D314". MEMORY.md:11 "nothing goes back".
- **Splits:** memory notes omit the D314 exception that AR, CLAUDE.md and DOR:175 state. Differs for a C++ reviewer fix that fails the main session's build.
- **Ruling quotes:** review-and-hooks.md (Confirmed, V20b): 2026-10-10 "i asked for the review agent to fix its findings directly, then we're done." (names the one-review side); 2026-10-09 "1 - send the fixes to a fresh agent" and 2026-10-05 "make that fix and have a new review agent pick up what's left" (lean the other way). None names both sides of the D314 exception. "massive waste of time" is under Unconfirmed (review-and-hooks.md:49).
- **Tests:** none found.
- **Open:** fix-decisions.md D107/D314 not read by V20b.
- **Hook gap:** none.

### L-F20-11: Does every plan for a screen carry one GUI test of the whole main task by clicking?
- **Source:** I8-112; F20b; V20b.
- **Kind:** K7.
- **Verdict:** CONFIRMED (V20b; V20b did not redo the finder's gates search).
- **Homes:** MEM new-screens-start-from-a-clickable-mockup.md:15 "Every plan for a screen includes one GUI test that does the whole main task by clicking on screen, not by calling the engine." V20b grep of agents/planner.md, premise-map.md, owner-map.md, ui-testing.md:3-16: no hit.
- **Splits:** None (one home, a memory note; planner agents never see it).
- **Ruling quotes:** None found.
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F20-12: Look at a capture at 2x zoom before showing it
- **Source:** I8-143; F20b; V20b.
- **Kind:** K3.
- **Verdict:** CONFIRMED (V20b).
- **Homes:** MEM render-checks-use-the-corpus.md:3 "look at the capture at 2x" and :18 "Zoom it 2x and check gaps, column alignment and shared baselines before showing it". UT:10-11 "the captures are shown to the user" (no zoom step). doc-screenshots.md, tools/hydra_ui_mcp.py: no 2x step (finder). V20b grep "zoom|2x|inspect" in ui-testing.md and doc-screenshots.md: no hit.
- **Splits:** the 2x step is in a memory note, not in the page that owns the check.
- **Ruling quotes:** None found (no hit for "2x|zoom").
- **Tests:** none.
- **Open:** none.
- **Hook gap:** none.

### L-F20-13: Single-home or memory-only rules F20 did not get verified
- **Source:** I7-120, I7-123, I7-125, I8-040, I8-070, I8-209, I8-122, I8-124, I8-147, I8-176, I8-204, I8-205, I9-026, I9-044, I8-004, I8-142, I8-021, I8-022; F20.
- **Kind:** K1, K5, K6, K7.
- **Verdict:** NOT VERIFIED (V20 listed these as not reached).
- **Homes:** finder only. I8-040: MEM derive-display-from-engine-truth.md:20 and MEMORY.md:41 promise "a row" for every "handled once" claim; DOR:80 allows a one-sentence reason instead. I8-147: MEM repo-scans-read-main-checkout.md:15 describes a hand script; hydra_precheck is a code owner of the same question (K6 candidate). I8-070/209: MEM fc-video-workflow.md:13 and SKILL.md:61 give the same instruction. I9-026: G\tests\test_docs_numbers.py:37-43 (five files, banned patterns) holds docs free of typed keys. I8-176, I8-204/205: one home each (MEM trace-pass-inputs-before-live-runs.md:21; SKILL.md:56-57). I8-004/142: MEMORY.md:49 and MEM render-checks note repeat UT:10's wording.
- **Splits:** finder only: DOR:80 one-sentence-reason alternative vs memory "a row".
- **Ruling quotes:** None found.
- **Tests:** test_docs_numbers.py:37-43 (I9-026 only).
- **Open:** all of it.
- **Hook gap:** none.

## F21 rows

### L-F21-01: Is the realapp.jsonl file name (and its fields) held in two repos?
- **Source:** I10-061, I9-134, I9-129 area; F21 Group H; F21b split 4; V21, V21b.
- **Kind:** K6, K1, K8.
- **Verdict:** CONFIRMED (V21, V21b).
- **Homes:** H\tools\hydra_ui_mcp.py:81 `REALAPP_FILE = "realapp.jsonl"` (comment :78-80 "The hooks repo's ref gate reads it"); `realapp_record` :182-199 (seven fields). G\review_rules.py:49 same literal, comment :47-48 "Hydra's tools/hydra_ui_mcp.py owns its name (REALAPP_FILE) and its fields (realapp_record); this file only reads them." UT:233-251 field table; G README:55; G\tests\test_review_rules.py:404-405 and H\tools\test_hydra_ui_mcp.py:519-532 hand-hold the field names.
- **Splits:** None today (equal; V21: G tests use review_rules.REALAPP_FILE, so no test ties the two names). The comment says the other repo owns the name while this file defines it again.
- **Ruling quotes:** none found.
- **Tests:** G\tests\test_docs_numbers.py:54 `README_NAMES` includes "realapp.jsonl" (checks the README only, finder).
- **Open:** none.
- **Hook gap:** none.

### L-F21-02: Does "the user typed this" have two code owners?
- **Source:** I9-132; F21b split 6; V21b.
- **Kind:** K6.
- **Verdict:** CONFIRMED (V21b: not run).
- **Homes:** G\review_rules.py:89-113 `human_prompt_text` (:103 `str(o.get("turnOrigin","")).lower() != "human"`; also not isMeta, not isSidechain, no toolUseResult, matching promptId); used by percall.py:34 and review_rules.py:127. tools\session_cards\common.py:372-402 `classify_user` (:397-398 `origin.kind == "human"`; also :433), used by ruling_rules.py via the card tools.
- **Splits:** `turnOrigin` vs `origin.kind` for "did the user type this". Differ for transcript lines where one field is set and the other not; a run would settle which Claude Code versions write which.
- **Ruling quotes:** none found.
- **Tests:** test_review_rules.py:347-399 pins human_prompt_text; test_ruling_rules.py:180-219 pins the card tools; no test ties the two.
- **Open:** which fields each Claude Code version writes (needs a run).
- **Hook gap:** none.

### L-F21-03: Is the reviewer type and the brief fingerprint defined twice?
- **Source:** I9-108, I9-050; F21b split 7; V21b.
- **Kind:** K6, K5, K1.
- **Verdict:** CONFIRMED (V21b; test claim taken from the finder, not opened).
- **Homes:** review_dispatch.py:16 and agent_stop.py:20 both `"reviewer"`. review_dispatch.py:172 `hashlib.sha256(prompt.encode("utf-8"))` vs agent_stop.py:199 `hashlib.sha256(first.encode("utf-8"))` with first = first_user_message of the transcript. Prose spelling the refusal list: G README:39, DOR:17/:200, MEM git-traps-in-shared-checkout.md:17, reviewer.md:9/33.
- **Splits:** two fingerprint computations; the match works only if `prompt` equals the transcript's first user message byte for byte. Differs for a prompt the transcript stores altered (trailing text, wrapping).
- **Ruling quotes:** None found.
- **Tests:** test_agent_stop.py:497-547 holds the hash match through real handover records (finder). No test ties the two REVIEWER_TYPE constants.
- **Open:** none run-dependent.
- **Hook gap:** none.

### L-F21-04: Does the stall hook's refusal text state a rule without its condition (no marker)?
- **Source:** I9-117; F21b split 3; V21b.
- **Kind:** K3.
- **Verdict:** CONFIRMED.
- **Homes:** G\stall.py:280 "A move of main to a tip with no verdict is refused." review_rules.py:408-411 returns "no-gate" when neither marker is on any rev (V21b).
- **Splits:** stall.py text vs review_rules.py: differ for a repo with no marker (allowed).
- **Ruling quotes:** none found.
- **Tests:** test_stall.py (T3) not opened.
- **Open:** none.
- **Hook gap:** none.

### L-F21-05: Does run.py treat the guard's refusal exit code 2 as "not doctest's own"?
- **Source:** I10b-103; F21b split 5; V21b.
- **Kind:** K3, K6.
- **Verdict:** CONFIRMED-CHANGED (weaker than the finder: run.py's comment at lines 71-73 already names run_guard.h's refusal as an abnormal run; the only gap is that the number 2 is not read from run_guard.h and the message says "not doctest's own").
- **Homes:** H\tools\mutate\run.py:74-75 `EXIT_PASSED = 0`, `EXIT_FAILED = 1`; :578-579 "exit code %d, which is not doctest's own". H\tests\run_guard.h:20 `kRefusedExit = 2`; test_main.cpp:49-66. development.md:30,36,45 "exit code 2" for a refused bare run.
- **Splits:** None between homes of the same value; K3-lean.
- **Ruling quotes:** none found.
- **Tests:** none ties run.py to kRefusedExit (finder searched test_run.py).
- **Open:** none.
- **Hook gap:** none.

### L-F21-06: Is the hooks line budget (3100) backed by a ruling?
- **Source:** replay.py side note; F21b; V21b.
- **Kind:** K4.
- **Verdict:** CONFIRMED.
- **Homes:** G\tests\replay.py:41 `LINE_BUDGET = 3100  # the user's hard budget ... only the user changes it`.
- **Splits:** None.
- **Ruling quotes:** review-and-hooks.md:13 (Confirmed) chose "Raise cap to 3,050". "rase cap to 3100" is at :44 under `## Unconfirmed` (Confirmed at :7, Unconfirmed at :37). The code value is not backed by a Confirmed line.
- **Tests:** replay.py itself.
- **Open:** none.
- **Hook gap:** none.

### L-F21-07: Does a test for a reflog replay still exist? (T3 row I9-138)
- **Source:** I9-138, I9-139; F21b; V21b.
- **Kind:** K8.
- **Verdict:** CONFIRMED (V21b: grep "reflog" in test_review_rules.py found nothing; only G\tests\replay.py:16,131).
- **Homes:** G\tests\replay.py:42 `REPOS = {...}`; the test described by I9-138 (test_review_rules.py:911-1013) is not in the file now. Finder: deleted by commit c27420cf (per row I9-192, not re-verified).
- **Splits:** the inventory row describes code that no longer exists.
- **Ruling quotes:** none found.
- **Tests:** n/a.
- **Open:** none.
- **Hook gap:** none.

### L-F21-08: How many builds at once, and who holds the number?
- **Source:** I10b-116; F21b; V21b.
- **Kind:** K1, K5.
- **Verdict:** CONFIRMED for the owner and the two test copies; the six doc copies not opened by V21b.
- **Homes:** H\tools\build_slot.ps1:75 `$BuildSlotCount = 3`. Test strings "all 3 build slots are taken" at test_hydra_ui_mcp.py:683 and tools\test_build_pass.ps1:117. Prose copies (finder only): CLAUDE.md:76, PRE:57, AR:20, development.md:114, G README:15, model-postures.md:11. G\percall.py:20,108-111 refuses the tool to every agent; PRE:57 repeats it (K5).
- **Splits:** None.
- **Ruling quotes:** agents-and-workflow.md:8 (Confirmed) "i want a limit of three builds at once, strictly enforced. agents are never allowed to start their own builds, only the main session, with a limit of one build per session." Names the number, not the tests.
- **Tests:** two tests hard-code the message text with "3", not the count.
- **Open:** none.
- **Hook gap:** none.

### L-F21-09: Reviewer-side facts F21/F21b raised that no verifier reached
- **Source:** I9-100 to I9-105 (rest), I9-120, I9-127, I9-128, I9-135, I9-158, I10-084, I10-088, I10b-076, I10b-082, I10b-083, I10b-087, I10b-088, I10b-089, I10b-107, I10b-114, I10-024, I10-026; F21; F21b.
- **Kind:** K1, K5, K7, K8.
- **Verdict:** NOT VERIFIED (V21: not reached; V21b: not reached).
- **Homes:** finder only. Picker limits `kCommonNameLimit = 30`, `kPickSizeLimit = 40`, `kHeaderIncluderLimit = 20` (tools\precheck\test_pick.h:25-30, comment "They live here and nowhere else.") repeated in development.md:558-563, fix-decisions.md:1646 (D299), tests\test_precheck_pick.cpp:75-81. Precheck kinds: checks.cpp:7-13 vs DOR:21,28-38 ("Seven are listed here"). check_5.cpp:30-33 kWhy "(derive-once review, step 8)" vs DOR:49 step 8 (a step number copied into code). D49 number lists (0/1/2, 1000/1024/1000000, 200 characters) in fix-decisions.md:154 and check_2/check_3/definitions.h (D49 names tools/derive_once_precheck.ps1; existence unchecked). Handoff checker rule: check_handoff.py:391-393 vs handoff.md:57, PRE:97. Docs excluded when proving a function exists: check_handoff.py:90-97 (one home). Ref gate: ref_judge.py:99-103,135,169-171, ref_gate.sh:15-17, PRE:65,71 (preamble summary omits the pushed case); G README:53 vs :57 (read as two checks). Hand-listed known copies check_1.cpp:42-45, check_2.cpp:1-4,82 with "when a row lands, this entry goes" and no check (K7-lean). Stop hook "never blocks": stated in agent_stop.py:10, stall.py:6, waiver.py:6, posture.py:8, README:29; settings.fragment.json:207-219 registration (timeout 20, no onFailure) enforces it; no test ties it (K7-lean). G\ref_judge.py:31,38 `WATCHDOG = 60` vs ref_gate.sh:15-17, install_ref_gate.ps1:13 (held equal by test_ref_judge.py:376-447, :610-614). CODE_BRIEF/DOC_BRIEF in review_rules.py:20 hard-code Hydra doc paths; review_rules.py:435 names `hydra_start('window')` (finder only).
- **Splits:** finder only: none between homes.
- **Ruling quotes:** D299 is the user's words in the decision log, not under `## Confirmed` (finder).
- **Tests:** test_precheck_pick.cpp:75-81; test_ref_judge.py as above; test_docs_numbers.py:137.
- **Open:** all of it.
- **Hook gap:** none.

## Tail lists

Candidates the verifier REFUTED: none (no verifier refuted any candidate in these ten reports).

Rows CONFIRMED-CHANGED: L-F19-06, L-F19-08, L-F19-14, L-F20-10, L-F21-05.

Rows no verifier reached (NOT VERIFIED): L-F19-04, L-F19-11, L-F19-12, L-F19-20, L-F19-21, L-F20-13, L-F21-09. Partly reached: L-F19-07 (second half), L-F19-13, L-F19-15, L-F19-18, L-F20-02 (three-lists point), L-F21-08 (doc copies).

Hook gap rows: L-F19-07 (review_rules.py waiver prefix), L-F19-18 (settings.fragment.json deny list), L-F19-19 (percall.py tool-name list), L-F20-09 (review_rules.py marker per repo). All finder only and unverified except the marker absence in video-tools, which V20b confirmed.

## Counts

Rows: 43 (F19 21, F20 13, F21 9). CONFIRMED 31 (includes rows confirmed on the part a verifier checked: L-F19-07, 13, 15, 18, L-F20-02), CONFIRMED-CHANGED 5, REFUTED 0, NOT VERIFIED 7. Splits: 19 rows carry a verified split (L-F19-01, 02, 03, 05, 06, 08; L-F20-01, 02, 03, 04, 05, 06, 07, 09, 10, 12; L-F21-02, 03, 04); 4 more rows hold finder-only splits no verifier reached (L-F19-04, 12, 20, 21).
