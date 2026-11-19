# TOOL-AUDIT-T8-K3 closure check (18 ids): 17 PASS, 1 REOPEN (L-F18-12, doc-vs-code half only). All reached.
H = C:\Users\Patrick\Downloads\Hydra\hydra-test, G = C:\Users\Patrick\.claude\gates, M = the memory folder.

### L-F14-12
**Verdict** PASS (exit 4). **Homes read** H\docs\agents\public-doc-style.md:31 (throat-clearing list for player pages); C:\Users\Patrick\.claude\CLAUDE.md:5 (three examples, for explaining to the user). **Why** Different readers; the global list is examples; neither overrides the other.

### L-F15-01
**Verdict** PASS (exit 3 Note is wrong). **Homes read** M\git-traps-in-shared-checkout.md:19 ("Never git add -A or git add -u"); H\docs\agents\brief-preamble.md:63 ("Stage files by name ... Never git add -A or git add ."). Searched G\*.py, ~\.claude\agents\*.md and H\docs\agents\*.md for "add -u": only the note names it. **Why** The preamble already holds the rule (stage by name; a broad add sweeps others' work); `-u` is one case of it. Caveat: the exact `-u` wording lives only in the note, so the fix may be to add "or -u" to the preamble, not only to prune the note.

### L-F15-15
**Verdict** PASS (exit 3 Merge, I pick). **Homes read** C:\Users\Patrick\.claude\skills\reverse-engineer-anything\SKILL.md:37-41 (15 ids); references\connection-and-recovery.md:46-50 (23 ids; adds qwen_code, trae, pi, hermes, qoder, droid, zcode, dsh). **Why** Both are docs listing "the current supported client" ids and they disagree.

### L-F16-19
**Verdict** PASS (exit 4). **Homes read** H\tools\ch_mcp\build_facts.py:69-102 (current_build reads version.json; same_build drops the leading v from each); dump_facts.py:27,994-995,1069 and README.md:20,27 (--version typed by hand, "stored as given"). **Why** The typed value is the only input a dump.cs has; the code already copes with the spelling difference. No question is answered two ways.

### L-F17-02
**Verdict** PASS (exit 3 The code wins). **Homes read** H\docs\agents\clone-hero-mcp.md:29 ("typed exactly `allow key run`"); G\review_rules.py:54-55 (_KEY_ALLOW regex, trailing . or ! allowed, IGNORECASE) and :86 (fullmatch on stripped text); G\percall.py:96 uses it. **Why** Real enforcing code. Making the doc match it opens no guard gap.

### L-F17-16
**Verdict** PASS (exit 3 Merge, I pick). **Homes read** H\docs\agents\rulings\README.md:19-20 ("`## Unconfirmed` holds the lines sessions copied from memory notes on 2026-10-31, untouched"); H\docs\agents\rulings\review-and-hooks.md:37-46 (`## Unconfirmed` holds 2026-11-12, 11-06, 10-30, 10-25 lines). **Why** Both docs; the README describes a frozen 10-31 copy, the file holds later lines.

### L-F18-02
**Verdict** PASS (exit 3 Merge, I pick). **Homes read** H\docs\agents\derive-once-review.md:106 (three exceptions hold the merge, reviewer signs CLEAN, Note starts "Holds the merge:") against :149 ("SIGN-OFF: BLOCKED only when a finding ... must hold the merge"); C:\Users\Patrick\.claude\agents\reviewer.md:31; G\agent_stop.py:26,169 read only the BLOCKED line; a search of G (.py, .md) for "Holds the merge" found no reader. **Why** Docs disagree on how an unfixable merge-holding finding is signalled. Doc-vs-doc is the right kind; the missing hook reader is a gap note, not a second split.

### L-F18-04
**Verdict** PASS (exit 4). **Homes read** C:\Users\Patrick\.claude\model-postures.md:5 ("Nothing goes back to the executor and no second round (D107)"); H\docs\agents\agent-rules.md:11 ("no second round, and nothing goes back to the user (D107)"). **Why** One says nothing returns to the executor, the other nothing returns to the user; no reader acts differently. The real tension (Notes that hold the merge) is L-F18-02 / L-F19-01.

### L-F18-12
**Verdict** REOPEN for the doc-vs-code half. The doc-vs-doc half holds (exit 3 Merge, I pick).
**Homes read** H\docs\agents\derive-once-review.md:21 ("finds kinds B, C, E and F mechanically"), :38 ("For kind F it lists only candidates ... step 8 is where you judge them"), :49; H\tools\precheck\check_5.cpp:30-31 (kWhy "a second home for an idea that already has one") and :187-238 (F items added with kWhy).
**Why** The doc-vs-code kind rests only on kWhy, text the program prints. It enforces nothing: the program lists duplicates and the reviewer judges them. "The code wins" fails when the code side is only printed text. The split is line 21 against lines 38/49, both docs. The row should be doc-vs-doc alone; the printed wording is a copy edit inside the same pick, not a code-wins exit.

### L-F18-14
**Verdict** PASS (doc-vs-doc exit 3 Merge, I pick; moved-pointer exit 3 Fix without asking). **Homes read** C:\Users\Patrick\.claude\agents\reviewer.md:11 ("each with the plan line it breaks (CLAUDE.md rule 5)"); H\docs\agents\derive-once-review.md:110 ("Write each finding you did not fix under Notes, with what is left to do"); H\CLAUDE.md rule 5 (line 67): no "plan line" text; "plan line" also absent from agent-rules.md and derive-once-review.md. **Why** Both kinds hold. The pointer's target never held a plan-line requirement, so the fix is to drop or re-point the parenthesis (gone, nothing replacing it).

### L-F18-18
**Verdict** PASS (exit 3 The code wins). **Homes read** H\docs\agents\ui-testing.md:11-13 (headless check "takes the place of" launching the app; case 2 limits the real-app check to layout and what the user sees), :233 (gate on merges that change src/ui/); G\review_rules.py:31 (UI_CODE = ^src/ui/), :389 (only comment-only changes skipped), :399; H\tools\hydra_ui_mcp.py:81. **Why** The hook is real code and refuses a text-only src/ui change with no record. Aligning the doc to it is stricter and opens no gap. (The two typed "realapp.jsonl" copies agree; not a split.)

### L-F18-20
**Verdict** PASS (doc-vs-doc exit 3 Merge, I pick; doc-vs-code exit 3 The code wins), with a flag. **Homes read** H\docs\agents\derive-once-review.md:38 ("A change that adds an entry to `known_clones` is a finding"); H\docs\adr\0029-the-single-owner-rules-are-data.md:130-142 (dated entry, remove_by within 14 days); H\tests\single_owner\engine.cpp:176-189,207,215-223 (loader accepts dated known_clones); baseline.json holds `"known_clones": []`. **Why** Kinds hold. Flag: the doc-vs-code half and the doc-vs-doc half are one split. If the doc-vs-doc pick favours the review page, "code wins" would have chosen the other side. Pick once, not twice.

### L-F18-21
**Verdict** PASS (exit 4). **Homes read** H\docs\agents\derive-once-review.md:52 (D13 used as an example of a ruling not covering a wording); H\docs\audit\2026-10-03-fix-decisions.md:35,1680 (D302 reverses D13). **Why** The page uses D13 as an illustration, not as a current rule; no reader acts differently.

### L-F18-22
**Verdict** PASS (exit 3 Merge, I pick). **Homes read** H\docs\agents\derive-once-review.md:57-64 (ratio 1.1067, plan 158,254.77, game 158,268.99, 14.21 ms); H\docs\clone-hero\05-star-power.md:437 (chord 466 ratio 1.3926, planned 158,262.53, game 158,268.4, 5.8 ms); hydra-data\ch-mcp-runs\2026-11-13\pitch-dark-ratios-prep\schedule.json (1.1067 / 158254 / 158268.99 count 0; 158262.5305185332 present). **Why** Two docs give different figures for the same chord. For the pick: the review page may describe the tool before a fix and the star-power page after it (tools\replay_schedule.cpp:437 now chooses a ratio per step); the pick should say which.

### L-F19-01
**Verdict** PASS (claimed-ruling exit 3 Drop the claim; memory-vs-other exit 3 Note is wrong), with a flag. **Homes read** H\docs\agents\derive-once-review.md:106 ("The user ruled this on 2026-11-12 ('alright approve b')"); H\docs\agents\rulings\review-and-hooks.md:7 (## Confirmed), :37 (## Unconfirmed), :39 (the 11-12 line, under Unconfirmed); grep of rulings for "approve b" finds only that line; H\docs\sessions\2026-11-13-bed936f0.md:23 quotes "alright approve b" (a card, not a ruling); M\one-reviewer-rule-d107.md ("pass them on, don't hold the merge"). **Why** No Confirmed line, so the claim drops. Flag: once it drops, the doc's merge-hold rule has no filed ruling behind it. The memory note is not the user's words either, but the main session should not assume the note simply loses.

### L-F19-12
**Verdict** PASS (exit 4). **Homes read** M\plans-reviewed-before-shown.md:13 ("a plan pasted into chat slips past it. The review rule still applies"); H\docs\agents\doc-review.md:17 (gated steps; plans via ExitPlanMode); G\publish_gate.py:112 (only ExitPlanMode with a plan). **Why** The doc says which plans a hook checks; the note says the rule holds anyway; CLAUDE.md "plans get a review" agrees. Hook gap: a plan pasted in chat has no hook (publish_gate.py:112).

### L-F19-14
**Verdict** PASS (exit 4). **Homes read** H\docs\agents\integrate.md:43-44 (`-tc="single-owner*"`, `-sf=*docs_match_code*`); H\docs\agents\agent-rules.md:7 (`-tc=docs match code*`, `-sf=*single_owner*`); H\tests\test_docs_match_code.cpp has 19 "docs match code" cases plus one "single-owner" case (:155) and one "code lexer" case (:2189) (R5 said 20 cases; it is 21). **Why** Two spellings that pick nearly the same tests; nobody acts differently.

### L-F19-21
**Verdict** PASS (exit 4). Same two homes as L-F14-12 (public-doc-style.md:31, ~\.claude\CLAUDE.md:5). Different readers.

## Ids not reached
None.
