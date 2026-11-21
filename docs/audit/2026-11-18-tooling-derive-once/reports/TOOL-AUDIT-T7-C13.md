# TOOL-AUDIT-T7-C13 closure check (sorting S07, ledger L07)

Abbreviations: H = Hydra repo (C:\Users\Patrick\Downloads\Hydra\hydra-test), G = C:\Users\Patrick\.claude\gates, M = memory folder under C:\Users\Patrick\.claude\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory, DOR = H docs/agents/derive-once-review.md, DR = H docs/agents/doc-review.md, UT = H docs/agents/ui-testing.md.

Counts: PASS 23, REOPEN 1 (L-F21-04), GONE 1 (L-F19-04). Not reached: none.

### L-F19-02
**Verdict:** PASS (A)
**Homes read:** H docs/agents/integrate.md:54 "The gate keys a review on the exact tree, so any later commit voids it."; G review_rules.py:255-265 `_carries` ("True when new is the clean merge of base and a branch commit that passes(commit) accepts"), used at :272 and :452; DOR:189-191 (a conflict-free merge of main passes when git's merge makes exactly the same tree).
**Why:** The split is a page sentence against code. The code carries a clean merge of main, so "any later commit voids it" is wrong. DOR:191 sits on the code's side (it agrees with it). Not a doc-versus-doc split.

### L-F19-03
**Verdict:** PASS (A)
**Homes read:** UT:143-147 (two named groups, 5 + 7 names, "in two groups"); H tests/ui/uitest_report_windows.cpp, 24 distinct "report-window..." names registered (counted by grep), 12 in neither list.
**Why:** Page against a test file (program code). Current files still show 12 unlisted names. I did not check that each of the 24 is a registered case; the string names appear in the file's registration lines.

### L-F19-04
**Verdict:** GONE
**Homes read:** M one-reviewer-rule-d107.md:11 and :15; G progress.py:11-14 (CALL_WARN, CALL_STOP = 100, 150; MINUTES_WARN, MINUTES_STOP = 20, 30); C:\Users\Patrick\.claude\agents\reviewer.md:11; DR:100.
**Why:** The only verified split was that the memory note omitted the 100-call / 20-minute wrap-up point. Line 11 of the note now says "the progress hook, gates\progress.py, gives every agent the wrap-up warning at 100 calls or 20 minutes". Line 15 still says "the reviewer cap is 150 calls like everyone else", which agrees with progress.py. The writer-brief.md:101 path does not exist any more (H tools/session_cards is gone), so that side is gone too. No remaining split.

### L-F19-05
**Verdict:** PASS (A)
**Homes read:** G agent_stop.py:169 `DOC_SUFFIXES = (".md", ".html", ".htm")` (used :178, :278); G README.md:49 (names markdown and HTML pages); DOR:151 "fingerprints every markdown file the range ... changes"; DR:29 "every markdown file the range changes"; DOR:211 area. C:\Users\Patrick\.claude\agents\reviewer.md:33 now says "every document", no longer "markdown", so it is no longer one of the copies.
**Why:** Prose copies (DOR:151 and DR:29 at least) still name a narrower set than the code. Doc against code. Line numbers have moved (DOR:202 is now near 211).

### L-F19-07
**Verdict:** PASS (A)
**Homes read:** DR:21 ("so nobody computes or types a key for it" and, at the end, "typing `waive doc-review <key>`"); G review_rules.py:26-29 WAIVER_PHRASE (doc-review needs 7-64 hex); G publish_gate.py:103-108 (refusal prints the key and `waiver_phrase`).
**Why:** Page wording against code. The first DR:21 sentence is about the reviewed document's own record, so the page is open to misreading, and the code requires a typed hex key for a waiver. Weak case: it is a wording edge, not a behaviour split, and the only "other side" is the same page. A closure that fixes the sentence stands as category A.

### L-F19-08
**Verdict:** PASS (A)
**Homes read:** UT:10 (case 2: layout, width, spacing, alignment, song open/close; "still owed" if the user says merge); UT:235 ("a gate on merges that change `src/ui/`"); G review_rules.py:31 `UI_CODE = ^src/ui/`, :388-389 (non-comment src/ui changes), :392-419 `realapp_checked`, :450-458 (refusal and waiver `waive real-app`).
**Why:** Page against code. The hook asks for a real-app record for any non-comment src/ui change; UT:10 limits it to visual cases and offers "still owed" as a way out.

### L-F19-09
**Verdict:** PASS (P)
**Homes read:** H docs/agents/clone-hero-mcp.md:31 (the allowed-chart sentence; it now also allows Pitch Dark by CHON as a human player). Searched H tools/ with the Grep tool for `Sidewinder|allowed_song|allowed_chart`: only tools/ch_probe/tests/mouths_refine_schedule.json; searched H docs/agents and docs/clone-hero for "Sidewinder": only clone-hero-mcp.md:31 and docs/clone-hero/06-scoring.md:117 (an evidence line, not a rule).
**Why:** One home, no second statement of the list, nothing disagrees.

### L-F19-10
**Verdict:** PASS (P)
**Homes read:** G publish_gate.py:13-14 `SMALL_LINES, SMALL_CHARS = 20, 2000`, :31, :33; DR:17 ("`publish_gate.py` in the gates folder owns that limit"); G README.md:63; G publish.ps1:1-5 (no numbers); G review_rules.py:65-68 (comment points at publish_gate). Grep of DR, README, publish.ps1, review_rules.py for "20 lines", "2000", "non-blank": no hits.
**Why:** The owner holds the numbers and every other home is a pointer. No second copy of a value.

### L-F19-11
**Verdict:** PASS (P)
**Homes read:** G settings.fragment.json:174 matcher "Artifact|ExitPlanMode"; G publish_gate.py:114-125 `check` (ExitPlanMode with a plan; Artifact when action is none, "" or "publish", with a file_path and not an asset).
**Why:** The matcher and `check` name the same two tools. `check` skips more than the row said but the matcher does not claim otherwise.

### L-F19-13
**Verdict:** PASS (P)
**Homes read:** G agent_stop.py:25 `_BLOCKED, _CLEAN`, :116-120 (BLOCKED line wins, CLEAN line, no line records CLEAN); DOR:151; DR:125; G README.md:43; C:\Users\Patrick\.claude\agents\reviewer.md:33.
**Why:** Every prose home says "a report with no sign-off line is recorded CLEAN" and only `SIGN-OFF: BLOCKED` holds. Copies only.

### L-F19-15
**Verdict:** PASS (P)
**Homes read:** G ruling_rules.py:99-100 `INSTALL_DATE = "2026-11-14"`, `APPROVE, DECLINE = "File it", "Do not file"`, :335-347 backfill, :98 comment; H docs/agents/rulings/README.md:3, :41, :51 (names 2026-11-14 and the "Yes, no click" approval); H CLAUDE.md:41; agent-rules.md:33; G README.md:57-59.
**Why:** Labels, date and route agree between code and prose. The "Yes, no click" quote has no Confirmed ruling line (a quote of the user's words in README:51 and a ruling_rules.py comment citing a plan's Approval section), but no home disagrees with another, so it does not break P. The main session may still want that quote asked about on its own.

### L-F19-16
**Verdict:** PASS (P)
**Homes read:** G review_rules.py:23-24 `PUBLIC_DOCS` (README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md); H docs/agents/public-doc-style.md:3 (same four plus release notes); DR:17 (same four); H CLAUDE.md:45 ("four public pages").
**Why:** The four-page lists agree today.

### L-F19-17
**Verdict:** PASS (C)
**Homes read:** H .derive-once-gate ("Test-DeriveOnceGatedPath ... (hooks\lib\derive_once_rules.ps1)"); H .doc-review-gate ("(hooks\lib\doc_review_rules.ps1)"); ls C:\Users\Patrick\.claude\hooks (only `state`); ls G\lib (only hook_input.ps1); G review_rules.py:14-22 (markers, BUILTIN_CODE, comment "Hydra's holds only prose").
**Why:** Both marker files name hook files that no longer exist. The current owner is G review_rules.py (BUILTIN_CODE and the marker paths). Moved or missing path, current place clear. Editing the marker file is itself a gated change (DR:17), which is a process point, not a split.

### L-F19-18
**Verdict:** PASS (P)
**Homes read:** G publish.ps1:1 and :8 (ValidateSet: pr, release, release-edit, issue, issue-comment, pr-comment); DR:19 (same six); G settings.fragment.json:9-15, 23-29, 37-43 (deny `gh pr create`, `release create`, `release edit`, `issue create`, `issue comment`, `pr comment`, `api *` in Bash, PowerShell and `& ` forms); H docs/agents/issue-tracker.md:42 (`gh issue edit` for a label), :47 (`gh issue close`, "a context pointer (gist + link)"); H docs/agents/this-machine.md:15; H CLAUDE.md:53. Grep for "gist" in H docs/agents, G settings fragment and G *.py: only issue-tracker.md:47 and unrelated word matches.
**Why:** Kinds and deny lines agree. The worry that "gist + link" is a GitHub gist (N2) does not hold: it reads as a short summary and a link, and `gh issue edit` there adds a label with no text body. No disagreement.

### L-F19-19
**Verdict:** PASS (P)
**Homes read:** G review_rules.py:50-55 (`LIBRARY_ALLOW_TEXT = "allow library run"`, `KEY_ALLOW_TEXT = "allow key run"`, trailing . or ! allowed, ignore case); G README.md:13, :37; H docs/agents/agent-rules.md:14; brief-preamble.md:49; audit-method.md:125; clone-hero-mcp.md:29.
**Why:** The prose and the code give the same phrases. Copies only.

### L-F19-20
**Verdict:** PASS (P), with a limit stated
**Homes read:** DOR:80-87 (scan-row rule, names ADR 0029 and `parse_rule`); H docs/adr/0029-the-single-owner-rules-are-data.md:77-91 field table against H tests/single_owner/engine.cpp:91-108 (the same required and optional field names); H docs/agents/test-rubric.md:85-92 and :183, :189 (helper rule, audit-method pointer); W11 report (second round: Pitch Dark numbers agree with tests/test_replay_schedule.cpp, TR:103 "exit 2" pointer matches audit-method.md:22, owner-map.md:13 short forms).
**Why:** This row is a bundle of single-home rules and a few two-home items. The two-home items I opened agree. I did not open every single-home row (DOR:94/100, DOR:157-183, DR:3/25/51-63/83, TR:84/205, test-rubric-examples.md:336, DOR:28/74), since a single home cannot disagree with itself. A row not opened is not proven, so the main session should treat this P as the weakest PASS in the batch.

### L-F20-03
**Verdict:** PASS (B)
**Homes read:** M early-fill-orange-85-d250.md:3 and :11 (names `kEarlyFillSlackMs`); H src/core/model.h:100-115 (`kHitWindowMs = 85.0`, `kDefaultEarlyFillWindowMs = 60.0`, `kMaxEarlyFillWindowMs`); grep of H src and tests for `kEarlyFillSlackMs`: no hit under src or tests (only old audit, plan and handoff docs).
**Why:** The only split is a memory note naming a constant that code does not have. Memory against code.

### L-F20-05
**Verdict:** PASS (B)
**Homes read:** M hydra-deluxe-rename.md:3 (description says the repo is patseguin1-commits/hydra-deluxe) and :11 (says it moved to splax77/hydra-deluxe); H docs/agents/this-machine.md:15 ("The repo is `splax77/hydra-deluxe`"); H CLAUDE.md:13; `git remote -v` (origin is github.com/splax77/hydra-deluxe).
**Why:** The note's description is stale against its own body, the docs and the real remote. Memory against docs and git.

### L-F20-08
**Verdict:** PASS (P)
**Homes read:** H CMakeLists.txt:21 `project(Hydra VERSION 3.1.2`; H docs/agents/this-machine.md:17 ("reads VERSION 3.1.2 today"); M windows-installer.md:20 (names how to edit the line; its "2.10.1" is a past example, not a current-version claim).
**Why:** Copies agree today and the doc already says CMakeLists.txt is the one place. A pointer-only fix changes no behaviour.

### L-F20-10
**Verdict:** PASS (B)
**Homes read:** M hooks-never-guess.md:17 ("adds a second review (D107)"); M one-reviewer-rule-d107.md:11, :13, :15 (no D314, no build exception); M MEMORY.md:3, :11; H docs/agents/agent-rules.md:11 (one narrow exception, D314); H CLAUDE.md rule 5; DOR:175 ("a narrow exception to D107, and the user approved it in D314"). Grep of the three memory files for "D314", "exception", "fix review", "build": none that state it.
**Why:** The memory notes omit an exception that the docs state. Memory against docs.

### L-F20-13
**Verdict:** PASS (B), with a limit stated
**Homes read:** M derive-display-from-engine-truth.md:20 ("Back the promise with a row ..."), M MEMORY.md:41; DOR:80 ("a row ... Or the fix's commit body says, in one sentence, why a text search can't catch this copy"); M repo-scans-read-main-checkout.md:15 (a small hand script) against H tools/precheck/main.cpp; M fc-video-workflow.md:13.
**Why:** The verified split is a memory note against a review page, and the repo-scans note is a memory note against code. The single-home items cannot split. Unopened: I8-176, I8-204/205, I8-070/209 SKILL.md:61, I9-026 beyond W11's reading.

### L-F21-01
**Verdict:** PASS (P)
**Homes read:** H tools/hydra_ui_mcp.py:78-81 `REALAPP_FILE = "realapp.jsonl"` (comment says the hooks repo reads it), :182-199 `realapp_record`; G review_rules.py:47-49 (same literal, comment "Hydra's tools/hydra_ui_mcp.py owns its name ... this file only reads them"), :399; UT:233-251 and :237.
**Why:** The name is equal in both repos; the two code constants cannot be merged across repos, and the existing comments already name the owner. Nothing disagrees.

### L-F21-04
**Verdict:** REOPEN
**Homes read:** G stall.py:280-281 ("A move of main to a tip with no verdict is refused."), inside `_verdict_problems` (a string the hook shows the main session); G review_rules.py:421, :430-432 (`explain_main_move` returns "no-gate" when no marker exists, so a move is allowed).
**Why:** Both sides are program code: a message string in stall.py against the decision in review_rules.py. Category A needs a doc, brief, page, README, agent file, CLAUDE.md or comment line against code, and this one is a runtime message against code. The sorting row calls it "the hook's message", not a comment. It should go back as a question (or be sorted by the main session as a message-text fix) rather than stand as "The code wins". If the main session reads a refusal message as a comment line, the PASS would hold, and the stall.py text still lacks the no-marker condition.

### L-F21-05
**Verdict:** PASS (P)
**Homes read:** H tools/mutate/run.py:68-75 (comment names run_guard's refusal as an abnormal run; `EXIT_PASSED = 0`, `EXIT_FAILED = 1`), :579-580 ("not doctest's own"); H tests/run_guard.h:20 `kRefusedExit = 2`; H tests/test_main.cpp:49-66; H docs/development.md:30, :36, :45 ("exit code 2").
**Why:** No home restates the number 2 as a different value, and run.py's comment already names the refusal. No disagreement.

### L-F21-07
**Verdict:** PASS (C)
**Homes read:** G tests/test_review_rules.py (910 lines, so the cited lines 911-1013 cannot exist; grep for "reflog" in all of G tests finds only replay.py); G tests/replay.py:16 and :131 (reflog replay); recent gates commit 2ecdfef "Replay no longer reads the removed hooks folder ...".
**Why:** The inventory row I9-138 describes a test that is gone. The current place for reflog replay is G tests/replay.py. A moved or missing file, current place clear.

## Ids not reached
None.
