# TOOL-AUDIT-T8-K1 closure check, 18 ids. All 18 PASS. Ids not reached: none.
H = Hydra main, C = C:\Users\Patrick\.claude, M = C\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory

### L-F02-02
Verdict: PASS. Homes: C\gates\tests\test_docs_numbers.py:6,76 (GATES_HYDRA_ROOT or main checkout); C\gates\README.md:156 same. Test and README agree; a wish to read a branch is not a split.

### L-F02-12
Verdict: PASS. Homes: C\gates\publish_gate.py:13-14,31-33 (SMALL_LINES, SMALL_CHARS = 20, 2000, applied to text sent for doc review); rulings\review-and-hooks.md:22 (Confirmed 2026-10-10); H\tools\precheck\items.h:50 and items.cpp:102-104 (400-line range warning). Different questions. The ledger's "no gate applies 20" is wrong: publish_gate applies it.

### L-F02-13
Verdict: PASS. Homes: H\docs\agents\audit-method.md:104 (number is the main session's suggestion); plans 2026-11-08-player-audit.md:102, 2026-11-08-clone-hero-docs-gaps-search.md:187, 2026-11-14-test-suite-audit.md:194. Plans choose their own number; no file claims the user set it.

### L-F02-20
Verdict: PASS. Homes: C\gates\README.md:120 states the inference itself ("infers it for the user's settings.json, which it did not touch"); README:192 agrees.

### L-F03-02
Verdict: PASS (both exits). Moved pointer: C\skills\analyzing-sp-squeezes\SKILL.md:10 and C:\Users\Patrick\Downloads\Hydra\squeeze-tools\squeeze_scout.py:5,34 say kSqueezeWindowMs lives in src/search/graph.h; H\src\core\model.h:121 defines it; graph.h:216 only mentions it in a comment. squeeze_scout.py:33 names kDefaultHitWindowMs; model.h:106 has only kHitWindowMs. Code-vs-code: squeeze_scout.py:33-34 types 85.0 and 500.0 by hand; equal to model.h today. Both kinds hold.

### L-F03-08
Verdict: PASS. Homes: M\no-full-library-runs.md:17 (18,811, 2026-10-08 event); M\min-path-delta-proof.md:29 (18,610, 2026-08 work). Dated counts, not a claim of current size.

### L-F04-13
Verdict: PASS. Homes: C\skills\reverse-engineer-anything\references\javascript-applications.md:35-39; SKILL.md:186; references\native-and-artifacts.md:33. The skill's own copies agree; the other side is the third-party REA server, no readable file. Searched C skills, agents and CLAUDE.md for max_heap/analysis_timeout: only that one line.

### L-F04-18
Verdict: PASS. Homes: M\no-full-library-runs.md:17, M\min-path-delta-proof.md:29, M\stored-versions-one-rule.md:11, M\measure-small-explain-first.md:11; rulings\code-and-engine.md:27, ui-and-display.md:20. Each is a dated count inside a story; none states a standing size.

### L-F05-06
Verdict: PASS (small note). Homes: H\tools\ch_mcp\machine_report.py:49-52,250-262; H\tools\ch_mcp\game_files.py:217-253. One prints framerate/vsync lines from any section; the other returns one [game] value and refuses duplicates. Different questions. Note: both hand-split key=value lines, a small duplicate of parsing but not a differing answer.

### L-F05-07
Verdict: PASS. Homes: H\tools\ch_mcp\menu_tools.py:45-51 (comment says Blue is an unverified guess); rulings\clone-hero.md:81. One home, flagged unverified by itself; no second statement found in docs\agents, docs\clone-hero, tools\ch_mcp.

### L-F05-08
Verdict: PASS. Homes: rulings\ui-and-display.md:27 (Confirmed 2026-10-30, session 83eb8bac, about scrolling Hydra's library); H\tools\ch_mcp\menu_tools.py:38-39,389-403 (SEARCH_DOWN_CAP = 10 presses in Clone Hero's menu). Different lists; no file says the ruling covers the game's list.

### L-F05-12
Verdict: PASS. Homes: H\tools\capture\report_shot.ps1:5-13 (ClickFromRight = 80, a measurement); H\src\ui\library_toolbar.cpp:130-142 (button placed at right edge). Script says where to click, code says where it is drawn; both say right edge.

### L-F06-13
Verdict: PASS. Homes: C\agents\scout.md:3-9 (model sonnet; tools include Edit, Write; prose allows writes to status line and scratch folder). Read-only means repo scope, as the file says.

### L-F06-15
Verdict: PASS (both kinds). Doc-vs-doc: H\docs\agents\brief-preamble.md:3 (worktree, scratch, tests; no status rule); C\model-postures.md:5,11 (status rule, no worktree/scratch; "A brief names no status file and no id"); H\docs\agents\audit-method.md:120 (caps, status rule). Memory-vs-other: M\every-brief-carries-status-rule.md description says "the dispatch gate now denies briefs that lack it", its body says "No gate checks a brief for this sentence"; C\gates has no such check (status wording appears only in progress.py and stall.py). The rule has doc homes (model-postures:5, preamble Status lines) and the user's words are Confirmed at rulings\agents-and-workflow.md:25, so pruning the note's wrong claim drops no rule.

### L-F06-16
Verdict: PASS, and the doc-vs-doc reading is right. Homes: C\model-postures.md:11 (nudge once, then TaskStop); C\gates\stall.py:122-125, 143-144 (notice: stop and re-dispatch), :474 (check-in: "nudge it or stop it"). stall.py only prints these lines; nothing enforces either order, so both sides are instructions. The ledger row also asked whether the memory note's quoted choice was made: M\relay-status-in-plain-english.md:15 ("Claim, plus disagreement", 2026-11-16). Only a session card has it (docs\sessions\2026-11-16-e816e44f.md:24); grep of rulings finds nothing. That claimed-ruling piece is already carried by L-F32-10 and L-F26-04 (exits.md:497,644), so it does not break this closure. The same card lists "Stop and re-dispatch" as the user's stop-authority choice; a card quote is a pointer, not a ruling, so the pick stays with the fix plan.

### L-F07-01
Verdict: PASS. Homes: C\agents\scout.md:4 (model: sonnet); H\CLAUDE.md rule 2 and C\model-postures.md:5 ("Sonnet or Haiku"); brief-preamble.md:67-69 (brief may change the model line). Sonnet is inside the allowed set.

### L-F07-02
Verdict: PASS (both exits). Memory-vs-other: M\archive\agent-worktree-base-is-origin-main.md:15 "Reviewers still use isolation: worktree" vs H\docs\agents\derive-once-review.md:200 and C\gates\review_dispatch.py:171-172 (deny when isolation set); live note M\git-traps-in-shared-checkout.md:17 already says reviewers never set isolation, so the rule survives elsewhere. Doc-vs-code: C\gates\README.md:39 and the review doc say a brief lacking the two lines is refused; review_dispatch.py:188 `if not keys and not labelled and not _gated(cwd): return None` lets it pass in a repo with no review marker on main (_gated, :151-155). That is not a guard gap CLAUDE.md requires, so "the code wins" stands; the doc should say "in a repo with a review marker".

### L-F07-09
Verdict: PASS. Homes: brief-preamble.md:66-69; C\gates\ref_judge.py:40 (TRAILER_KEYS), :242-252 (only checks each value holds an alphanumeric). Doc says what to write; hook checks presence. Hook gap (record only): the Agent value is not checked to be the committer's own id.
