# TOOL-AUDIT-T7-C06 (batch: L-F09-10, 11, 12, 14, 15, 16, 17, 18, 19)
Paths: H = C:\Users\Patrick\Downloads\Hydra\hydra-test, GR = C:\Users\Patrick\.claude\gates, SC = C:\Users\Patrick\.claude\tools\session_cards.

### L-F09-10
**Verdict:** PASS (category P). Limit: this is a bundle of about 15 small rows; I read the homes below, not every inventory id inside it.
**Homes read:** GR\README.md:21 against GR\media_rule.py:1-12 (media guard, D189/D273, MEDIA_ROOT C:\ytdl: same words); GR\common.py:10 STATE_ROOT, :121,:125 (state\agents, state\status) against GR\status_append.ps1:11-14 and README:65 (state\agents\); GR\README.md:162 against GR\install_ref_gate.ps1:1-5 (one install covers all worktrees); H\docs\agents\handoff.md:37 (no scratch path) against H\tests\test_docs_match_code.cpp:1226-1233 (kNotPermanentDirs = docs/handoffs/, docs/superpowers/) and H\.claude\commands\handoff.md:2-10 (points to handoff.md); H\docs\agents\premise-map.md:44 (ch-mcp-runs path; only other mention is a test comment, tools\ch_mcp\tests\test_compare_scorestats.py:52; nothing in tools\ch_mcp\*.py sets a different folder); test-rubric.md named by derive-once-review.md:46 and test-rubric-examples.md:3, derivation-audit by derive-once-review.md:48 (both files exist); domain.md pointer at CLAUDE.md:21.
**Why:** Every pair says the same thing today. Searched ch_mcp code for ch-mcp-runs and found no competing run-results folder. Pointers only, no value to choose.

### L-F09-11
**Verdict:** PASS (P).
**Homes read:** H\docs\agents\audit-method.md:38,108,164,190; agent-rules.md:20; brief-preamble.md:89; derive-once-review.md:48,90; doc-review.md:91; test-rubric.md:65; SC\writer-brief.md:30; H\tools\doc_shots\shrink.py:18; H\tools\mutate\run.py:91. Checked the named audit files exist in H\docs\audit (fix-decisions.md, derivation-audit.md, phase-3-5-questions.md, concept-audit\findings\V5.md, questions.md, 2026-10-29-audit-of-audits\).
**Why:** All name the same decision log path, `docs/audit/2026-10-03-fix-decisions.md`; writer-brief.md:30 gives the full absolute path of the same file. No disagreement.

### L-F09-12
**Verdict:** REOPEN. Category A fails; it should be a question (or be re-sorted with real code named).
**Homes read:** H\docs\agents\ui-testing.md:195 (last sentence: "`snapshot_library` is the only code that knows the library folder's path"), :350; H\docs\agents\this-machine.md:77; H\tools\hydra_ui_mcp.py:17 (docstring says the same), :55 `DEFAULT_LIBRARY_DIR`; H\tools\compare_db.py:51,:87; H\tools\bench.cpp:986.
**Why:** The sides the row names are docs and prose: UT:195 against TM:77, UT:350 and compare_db.py:51,:87. compare_db.py:51 and :87 sit inside the file's module docstring (a recipe in prose), not code that runs; TM:77 and UT:350 are docs. So the split has doc-against-doc sides, which breaks A. Real code that does contradict the claim exists: H\tools\bench.cpp:986 `out.push_back("C:\\Program Files\\Hydra");`. The row never names it. If the main session wants A, the row must be rewritten so the split is UT:195 against bench.cpp:986, dropping TM:77, UT:350 and the compare_db docstring as sides. As recorded, it does not hold.

### L-F09-14
**Verdict:** PASS (P).
**Homes read:** H\docs\agents\rulings\README.md:5-14 (eight topic files) against the folder listing (agents-and-workflow, clone-hero, code-and-engine, docs-and-writing, questions-and-audits, review-and-hooks, tests-and-runs, ui-and-display: the same eight); H\tools\compare_db.py:26 (tests-and-runs.md); H\docs\agents\test-rubric.md:78 (code-and-engine).
**Why:** List and folder agree; the two other mentions name files that exist.

### L-F09-15
**Verdict:** PASS (P).
**Homes read:** H\docs\clone-hero\code\README.md:237; H\docs\agents\this-machine.md:27 (pointer to it); H\tools\ch_mcp\build_facts.py:28; kit_methods.json:3; launch_patch.json:4; tools\ch_mcp\README.md:7,12,20,58; tests\test_dump_facts.py:282 (a refuse-inside-repo test using a repo-relative path to be refused, not a second dump root).
**Why:** All name `...\hydra-data\ch-dumps\6142\...`; I confirmed ghidra\ghidra_decompile_all.txt and dump.cs exist there. No disagreement.

### L-F09-16
**Verdict:** PASS (A). One split, a README line against a program.
**Homes read:** GR\README.md:71 ("It only informs: it always exits 0 ... Run it by hand ... with `py -I tools/check_pointers.py`"); H\tools\check_pointers.py:14-16 docstring ("Exit 0 when nothing is broken, 1 when something is, 2 on a bad argument. With --session-start ... always exits 0") and code at :285-310 (the --session-start branch returns 0 in every case; the by-hand branch goes on and exits by result); C:\Users\Patrick\.claude\settings.json:111 registers the script.
**Why:** The README sentence is a doc line; the other side is the program. Code wins. No doc, memory or ruling on the other side.

### L-F09-17
**Verdict:** PASS (P).
**Homes read:** SC\common.py:34,36 (DATA_ROOT, HYDRA_REPO), :554-568 build_tags; SC\prepass.py:456; H\tools\precheck\decisions.cpp:317-321; check_3.cpp:534; H\tests\test_docs_match_code.cpp:128; GR\delete_rule.py:17.
**Why:** All point at the same CONTEXT.md in the same repo. Only build_tags parses the bold terms; the others just read the file. Nothing disagrees.

### L-F09-18
**Verdict:** PASS (P).
**Homes read:** SC\migrate.py:97-98 (`--old-cards` = DATA_ROOT\cards, `--old-index`); SC\common.py:34,42; SC\writer-brief.md:25 (same data folder, full path); GR\ruling_rules.py:213-215 and SC\check.py:225,252,282 (use CARDS_ROOT and local_cards_dir, not the old folder). DATA\cards exists on disk.
**Why:** The old-folder path is spelled the same wherever it is spelled. Note: ruling_rules.py is in GR, not SC.

### L-F09-19
**Verdict:** PASS (P).
**Homes read:** H\tools\agent_cost.py:33-45 (PRICES: claude-fable-5-1, claude-opus-5-5, claude-sonnet-5-5, claude-haiku-5-5); C:\Users\Patrick\.claude\settings.json:3 (`ANTHROPIC_DEFAULT_OPUS_MODEL: claude-opus-5-5`); searched agents\*.md and model-postures.md for claude-opus/sonnet/haiku ids and found none.
**Why:** The one id both places hold is the same today. Pointing agent_cost.py at settings.json would change no behaviour for that id.

## Counts
PASS 8, REOPEN 1 (L-F09-12), GONE 0.
## Ids not reached
None.
