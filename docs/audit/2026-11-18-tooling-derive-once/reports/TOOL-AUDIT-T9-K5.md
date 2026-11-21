# TOOL-AUDIT-T9-K5 closure check (17 ids)

Summary: PASS 11, REOPEN 5 (L-F17-09, L-F17-17, L-F18-11, L-F18-13, and none other; see below), GONE 1 (L-F18-16). Hydra = C:\Users\Patrick\Downloads\Hydra\hydra-test; gates = C:\Users\Patrick\.claude\gates. Line numbers are today's; several gates files have shifted from the ledger's numbers.

Correction to the count line above: REOPEN is 4 (L-F17-09, L-F17-17, L-F18-11, L-F18-13), PASS is 12, GONE is 1 (L-F18-16). Total 17.

### L-F16-20
**Verdict:** PASS (Merge, I pick: both code).
**Homes read:** tools/capture/shot.ps1:2 `[int]$Wait = 10`; tools/capture_window.ps1:56 `Start-Hydra ... [int]$Wait = 12`.
**Why:** two scripts still hold different defaults for one setting. Both are code.

### L-F16-23
**Verdict:** PASS (Drop the claim).
**Homes read:** gates/README.md:19 (user ruled 2026-10-31 "trying to ban specific command patterns is a losing battle...") and :27 (user ruled 2026-11-06 "Exempt git commit"). docs/agents/rulings/review-and-hooks.md:40 has "Exempt git commit" under `## Unconfirmed` (heading :37). Grepped rulings/ for "losing battle", "ban specific", "command patterns", "another pattern": no hit anywhere. Card docs/sessions/2026-10-31-6777b546.md has the first quote (a pointer only).
**Why:** both claims still stand and no Confirmed line backs either.

### L-F17-01
**Verdict:** PASS (exit 4 runs half; The code wins phrase half).
**Homes read:** gates/review_rules.py:50-52 (`LIBRARY_ALLOW_TEXT`, regex with `[.!]?`, IGNORECASE); gates/percall.py `_library_refusal` (judges only the library flag in a shell command; tells the user to send "exactly: allow library run"); Hydra src/cli/batch_scope.h:10 `kWholeLibraryFlag`, `batch_scope()` returns Refused with no flag and no folders; src/cli/batch.cpp:96 refuses on Refused/Ambiguous; tools/compare_db.py:29 says the flag is needed.
**Why:** the program still makes the flag the only way to start a whole-library run, so the docs' wording and the hook's flag check answer one question. The phrase split (docs "exactly" against tolerant code) is doc against code, and loosening the doc opens no gate. The third split, the flag spelled in two repos, is the same value in both (exit 4 covers it).

### L-F17-04
**Verdict:** PASS (Merge, I pick: both printed hook text).
**Homes read:** gates/stall.py:293-295 "A move of main to a tip with no verdict is refused." (relays every `verdict_problem`); gates/agent_stop.py:419 `problem = "Reviewer %s's verdict: ..."` set when a verdict was recorded with problems, and :422/:430 "No verdict was recorded...".
**Why:** the split persists: stall.py says "no verdict" for every problem, while agent_stop raises a problem after a verdict was recorded. Neither text enforces anything (stall never refuses). Same-kind pair, pick comes later.

### L-F17-08
**Verdict:** PASS (Merge, I pick: both code).
**Homes read:** gates/publish_gate.py:68-88 `document_path_id` (absolute, forward slashes, lower case) and `_signed_off`; gates/review_rules.py:278 `any(r.get("path") == path ...)` (exact repo-relative string).
**Why:** two code paths still decide "same document" differently.

### L-F17-09
**Verdict:** REOPEN (wrong kind).
**Homes read:** Hydra docs/agents/doc-review.md:21 "typing `waive doc-review <key>`"; docs/agents/derive-once-review.md:18 (the line numbered 189 in the ledger) "`waive derive-once <key>`"; gates/README.md:37; gates/review_rules.py:25-28 `WAIVER_PHRASE` (7-40 hex, 7-64 for doc-review, 7-40 real-app) and :77 `waiver_phrase` (first 10 of the key); gates/waiver.py:19-22.
**Why:** it left by "Merge, I pick", which needs two code sides or two doc sides. Here one side is a doc (`<key>`, a placeholder) and the other is code (a prefix rule). The 40 against 64 difference is two different hashes (commit hash against sha256 of a document), not two answers to one question. The honest exits are "The code wins" (doc says what the code takes) or exit 4. The ledger itself says "None value-for-value". The sorter note (sorting/TOOL-AUDIT-T6-S06.tsv:33) calls it a guessed minimum with no ruling, which is also not a code-vs-code split.

### L-F17-12
**Verdict:** PASS (Merge, I pick: both code).
**Homes read:** gates/publish_gate.py:12 `MARKER = ".doc-review-gate"`, :42-60 `gated_repo` (marker on main; raises when git cannot read the repo); gates/review_dispatch.py:151-155 `_gated` (needs a local main and either `CODE_MARKER` or `DOC_MARKER`, returns False otherwise); gates/review_rules.py:18.
**Why:** two code paths still answer "is this repo gated" differently (one marker against either; raise against not-gated).

### L-F17-13
**Verdict:** PASS (Merge, I pick: both code).
**Homes read:** gates/percall.py:215 `common.run(body, "closed")`; gates/prompt.py:17 `PARTS = (("posture","loud"),("percall","closed"),("waiver","loud"))`; prompt.py:6-7 "A prompt cannot be refused here...".
**Why:** the mode string for percall is written in two code places (today they agree). Both sides are code; the ledger's verifier already reduced the reading to this. The ledger's percall.py:176 is now `stall.run`; the mode is at :215.

### L-F17-14
**Verdict:** PASS (Merge, I pick: both code).
**Homes read:** gates/review_rules.py:89-113 `human_prompt_text` (reads `turnOrigin` == human, rejects non-text blocks, isMeta/isSidechain/toolUseResult); C:\Users\Patrick\.claude\tools\session_cards\common.py:63 `SYSTEM_REMINDER`, :397-433 (reads `origin.kind`, accepts slash-command args, requires `commandMode == "prompt"` for attachments).
**Why:** two code readers of "did the user type this" still differ.

### L-F17-17
**Verdict:** REOPEN (wrong kind).
**Homes read:** gates/publish_gate.py:13-14 `# The user's decisions of 2026-10-10 ...` `SMALL_LINES, SMALL_CHARS = 20, 2000`, :31-33 counting; Hydra docs/agents/rulings/review-and-hooks.md:22 (names the 20-line side only); grep of rulings/ for 2000 / 2,000: only an unrelated clone-hero.md:26 hit. Card docs/sessions/2026-10-10-b5e8d95a.md:26 records D191 "Also cap 2,000 chars (Recommended)" (a pointer, not Confirmed).
**Why:** "Give each a home" is for a rule held only in prose or a memory note. This rule already lives in a hook (publish_gate.py names itself its one home) and the docs point at it without a number. The live problem is the code comment calling 2000 "the user's decision" with no Confirmed line: that is "Drop the claim" (theme 1), or exit 4 for the docs-point-without-number part. Closing it as "give a home" adds a home that already exists and leaves the claim in place.

### L-F18-01
**Verdict:** PASS (exit 4; The code wins; Note is wrong).
**Homes read:** Hydra docs/agents/agent-rules.md:14 "sending exactly `allow library run`"; docs/agents/brief-preamble.md:49; gates/review_rules.py:51-52 (tolerant); gates/percall.py `_library_refusal` (the hook does refuse); memory no-full-library-runs.md (holds a "No hook enforces it yet" line, now stale); src/cli/batch_scope.h.
**Why:** all three splits are covered: phrase (code wins), scope (exit 4, flag still the only door), note against hook (Note is wrong). The note's rule is also in CLAUDE.md rule 8 and brief-preamble.md (Tests), so pruning the stale line drops no rule.

### L-F18-03
**Verdict:** PASS (Drop the claim).
**Homes read:** Hydra docs/agents/derive-once-review.md:52 (step 9) "The user ruled this on 2026-11-12 ("alright approve b")."; docs/agents/rulings/review-and-hooks.md:39 under `## Unconfirmed` (:37); grepped rulings/ for "approve b" and "a finding a reviewer cannot fix": only that Unconfirmed line. Card docs/sessions/2026-11-13-bed936f0.md:23 quotes it (pointer only).
**Why:** the doc still names the user's ruling and no Confirmed line holds it. (The ledger's :106 is now :52 for that sentence.)

### L-F18-10
**Verdict:** PASS (Confirmed only).
**Homes read:** Hydra docs/agents/derive-once-review.md:45 (decision entry only with a Confirmed line or quote) and :52 (accepts "a D-record, a plan's "User decisions", a Confirmed line, a session card, or the words your brief quotes"); docs/agents/agent-rules.md:35-41 (only Confirmed is the user's words; Unconfirmed, readings, notes, handoffs, plan "as ruled" never count).
**Why:** the split persists (:52 lists a card and a brief quote with no Confirmed condition). The answer "only Confirmed lines close" decides it; the page fix is a doc edit and leaves no gate open.

### L-F18-11
**Verdict:** REOPEN (wrong kind; no split left).
**Homes read:** Hydra docs/agents/agent-rules.md:7 ("the whole suite only when the change touches shared code: gates\common.py, gates\lib, gates\settings.fragment.json, gates\tests\lib"); C:\Users\Patrick\.claude\model-postures.md:11 (same four); gates/README.md ("It runs every test only when it touches what every hook shares: common.py, lib\, settings.fragment.json or tests\lib\"); tools/precheck/test_pick.h:92-95 `shared_code` (Hydra C++ only, not the hooks list). Searched gates/*.py for the list: none.
**Why:** "Give each a home" assumes a rule with no doc or hook. This list is already in agent-rules.md, the posture file and the gates README, and the three agree. That is exit 4 (same thing, no split), not a homeless rule. Nothing needs moving.

### L-F18-13
**Verdict:** REOPEN (splits not covered).
**Homes read:** C:\Users\Patrick\.claude\model-postures.md:7 earlyStart rule (no checker found: grepped ~/.claude gates, tools, agents and Hydra docs/agents; only plan 2026-11-08-player-audit-fixes.md uses `earlyStart`); gates/agent_stop.py:23 and gates/review_dispatch.py:18 `REVIEWER_TYPE = "reviewer"` (two constants; percall.py:104 reads agent_stop's); Hydra CLAUDE.md "Past sessions" ("every past session ... short card ... in the repo at docs/sessions/") against C:\Users\Patrick\.claude\tools\session_cards\writer-brief.md:18 (a session not about the project is kept locally) and common.py:283 `is_local_only`.
**Why:** the row carries three things. The earlyStart rule has a doc home (model-postures.md) and only lacks a checker, so "give a home" does not fit. The two `REVIEWER_TYPE` constants are code against code, which "give a home" does not cover. The CLAUDE.md against writer-brief card split is doc against doc, also not covered. Each needs its own exit.

### L-F18-16
**Verdict:** GONE.
**Homes read:** gates/review_dispatch.py (grepped "2026-10-24", "approved", "exception", "judged_folder": no hit; the docstring claim is absent); gates/README.md, agents/reviewer.md, model-postures.md, Hydra derive-once-review.md and agent-rules.md (grepped "2026-10-24", "cross-repo", "other repo"): nothing claims the user approved a cross-repo exception. Rulings hits for 2026-10-24 (review-and-hooks.md:48, tests-and-runs.md:15,16,33, ui-and-display.md:47-51) are other subjects.
**Why:** the sentence "the one exception the user approved on 2026-10-24" no longer exists in today's files, so there is nothing to drop. review_dispatch.py was changed by later commits (d748a26, 39d89c3 in ~/.claude).

### L-F18-17
**Verdict:** PASS (Drop the claim).
**Homes read:** gates/ref_judge.py:243-245 `missing_trailers` docstring "the user's ruling named an agent's own commits"; rulings/review-and-hooks.md:57 under `## Unconfirmed` is the only trailer line; grep of rulings/ for "trailer" found no Confirmed line. gates/README.md:162 "Fast-forward only" is prose; the ref gate (README:55) owns the no-rewrite rule.
**Why:** the claim stands in the code comment and nothing under `## Confirmed` holds it.

Ids not reached: none.
