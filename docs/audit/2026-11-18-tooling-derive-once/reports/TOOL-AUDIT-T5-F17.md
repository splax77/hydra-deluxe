# TOOL-AUDIT-T5-F17 (finder F17, agent ae716085238ea2101)

Abbreviations: GR = C:\Users\Patrick\.claude\gates; HR = Hydra repo (worktree tooling-audit-records, same files as main); RR = GR\review_rules.py; AS = GR\agent_stop.py; PC = GR\percall.py; PG = GR\publish_gate.py.
Limits of this run: I did not read the live C:\Users\Patrick\.claude\settings.json (only GR\settings.fragment.json). I grouped the 58 rows into questions (below) and searched each question across the scope, so each row id is listed under its question. Searches used regex on the phrase and on the rule's words (not only the number). Skills\synced was hit by one broad grep and carried no relevant text. Memory archive\ was not searched on purpose (out of scope); two grep passes returned archive lines by accident and I ignored them.

Confirmed rulings that speak (HR docs/agents/rulings/):
- review-and-hooks.md:16 (also tests-and-runs.md:14): "i want to be able to give explicit approval for a full library run." (session dee9c61e). It says the user wants to approve; it does not name the phrase `allow library run`.
- review-and-hooks.md:19: chose "Build option A as planned, with Q1 as recommended: a hand-back with no sign-off line records CLEAN and tells the main session." (session 4672d33d). Covers I2-042 (the rule) fully.
- review-and-hooks.md:18: "why do we need to go through this whole key process ... this is just bad design" (session 4672d33d). Background for the sign-off line and keys.
- review-and-hooks.md:22: "opus, retire, retire, minutes only, only large items over 20 lines should go for review, retire..." (session b5e8d95a). Speaks to the small-text limit (I3-062): "over 20 lines" is the user's; the 2000-character limit is not in any Confirmed line.
- review-and-hooks.md:27: "every time claude publishes a document, it has to go through a review from a new agent with fresh context specifically briefed on these issues." (session 5ecb5b0e). Speaks to I3-065/066 (which calls are gated), not to the "later edits pass" rule.
- agents-and-workflow.md:18: "agents should fail loudly. freezing is unacceptable" (session a9a461da). Speaks to the "loud" failure mode (I2-074/086, I3-060).
- tests-and-runs.md:21: "I did not approve a full library test and I did not approve a 40 minute test." (session e40fb55d).
- Plan line 108 of docs/superpowers/plans/2026-10-25-hooks-from-scratch.md: "Each rule has one home. The numbers (100, 150, 20, 30, 5, 10) sit at the top of `progress.py` ..." The 20 there is the minutes warning, not the small-text 20 lines.
- No Confirmed line names the allow phrases, the waiver phrases, the sign-off lines, the public-page list, the code-path list, the document suffixes, or 2000 characters.

Test that holds homes equal: GR\tests\test_docs_numbers.py holds docs to progress.py numbers, the bash limit and the output cap, retired words, and README names. It holds NONE of the values in my rows. Tests that pin a value by copying the literal (a second copy, not a link): test_review_rules.py:852 (KEY_ALLOW_TEXT == "allow key run"), :839-866 (library allow forms), test_progress.py:437, :515, :637 (refusal text contains "allow key run"), test_ref_judge.py:614 (selftest line literal), :624 (shim marker literal), :416/:604 (verdict line literal). No test ties HR docs, the ch server text or the marker files to review_rules values.

## Q1 What exact words let one whole-library run through? (I1-005, I3-032, I2-081, I2-082 for the allow part)
Homes:
- RR:50-52 `LIBRARY_ALLOW_TEXT = "allow library run"`, regex adds optional `.`/`!`, ignore case, whole message after strip (the owner).
- PC:45-55 refusal text builds the phrase from LIBRARY_ALLOW_TEXT (reads the owner; fine).
- GR\README.md:13 "unless the user has typed `allow library run` in this session"; :37 "`allow library run` or `allow key run`. An allow must be the whole message; `is_library_allow` and `is_key_allow` ... own the exact match." (value stated + pointer = home).
- HR docs/agents/brief-preamble.md:49 "unless the user has sent exactly `allow library run` in this session; each such message lets one command through."
- HR docs/agents/agent-rules.md:14 "The user allows one run by sending exactly `allow library run`; each such message lets one command through."
- HR docs/agents/audit-method.md:125 "the user's `allow library run` in chat (CLAUDE.md agent rule 8)".
- HR docs/agents/this-machine.md:91 "It needs the user's `allow library run` first (CLAUDE.md rule 8)."
- HR docs/agents/rulings/review-and-hooks.md:47 (Unconfirmed): "The user kept the typed allow library run for whole-library runs."
- memory archive note only (out of scope). Active memory no-full-library-runs.md says "ask in chat ... wait" with no phrase.
- C:\Users\Patrick\.claude\model-postures.md:13 "Never run or brief a full library run without the user's permission in chat." (rule, no phrase.)
- HR CLAUDE.md rule 8 (and the same text in the repo-worktree CLAUDE.md): "Nobody runs a full library test without the user's explicit permission, asked for in chat first." (no phrase.)
- Tests: see above.
What each home tells its reader:
| home | reader | does |
| RR/PC | hook | accepts whole message, case-insensitive, optional trailing . or !, one allow per run; refuses with the phrase |
| preamble:49, agent-rules:14 | any agent | "sending exactly" the phrase; each message lets one command through |
| CLAUDE.md rule 8 | main session and agents | ask in chat first, no phrase named |
| model-postures.md:13 | main session after compact | get permission in chat; no phrase |
| README:13 | gate reader | typed in this session; one command per allow |
Edges: at the limit (a message `Allow Library Run!` or `allow library run.`): code accepts; every doc says "exactly" (narrower than code). Main session vs agent: PC applies the check to both and an allow is matched by session_id only, so the first call that arrives, main or agent, uses the allow; docs say "this session". Card writers skip the preamble (card-writer.md:9) so they get no phrase text, but the hook still refuses them (it does not look at agent type). Worktree vs main: allows are in GR\state, not in a repo, so the same.
Kinds: K1 (phrase in 6+ texts); K2-light (docs say "exactly", code accepts case, `.`, `!`); K5 (preamble:49 and agent-rules:14 spell out the hook's rule: flag name, one-per-message); K7-light (rule 8 prose covers "a full library test" in any form, the hook sees only a shell command holding `--whole-library`; compare_db.py and other runs the prose names but the hook cannot see).
Splits: (a) docs "exactly `allow library run`" vs RR:52 accepts case/`.`/`!`. (b) Rule 8 ("any run over the user's whole song library", agent-rules.md:14 lists "whole-library compare or timing run") vs PC:14,47-48 which judges only a shell command string containing `--whole-library`.
Ruling: review-and-hooks.md:16 (user wants to give explicit approval); does not name the phrase.
Also the flag: PC:14 `LIBRARY_FLAG = "--whole-library"` and HR src/cli/batch_scope.h:10 `kWholeLibraryFlag = "--whole-library"` (and src/cli/batch.cpp:3, tools/compare_db.py:29 "(src/cli/batch_scope.h owns that rule)"). Two code homes of the flag word, in two repos, nothing ties them (K6/K1).

## Q2 What exact words let one key-route Clone Hero call through, and which calls are key-route? (I1-006, I3-033, I2-081)
Homes:
- RR:53-55 `KEY_ALLOW_TEXT = "allow key run"` (owner of the words).
- PC:15-18 KEY_TOOLS = ("mcp__ch__ch_key","mcp__ch__ch_press"), KEY_ROUTE_TOOLS = {"mcp__ch__ch_play_chart": "keys"}; PC:91-96 refusal text names the kit alternatives (`ch_kit_press`, `ch_open_song`, `ch_play_chart` route "kit").
- HR tools/ch_mcp/server.py:146-148 "ch_key, ch_press and ... hook refuses them unless the user has typed allow key run."; :466 and :477 same; tools/ch_probe/experiments/play_chart.py:1281 same.
- HR docs/agents/clone-hero-mcp.md:29 "`ch_key`, `ch_press` and `ch_play_chart` with `route` set to "keys" ... refuses each such call unless the user has typed exactly `allow key run` ... each such message lets one call through. ... `ch_play_chart` with no route uses the kit once `ch_kit_setup` has built it."
- GR\README.md:13 (kit alternatives, "one `allow key run` serves any one counted call", `ch_call` handling), :37.
- Active memory ch-runs-must-not-need-focus.md:17 "The key route is locked until the user types "allow key run"."
- The ch MCP server instructions in the session (system text) say the same.
- Tests: test_progress.py:612-637 holds the refusal text to the tool names; test_review_rules.py:852 pins the phrase.
What each tells the reader: server.py/clone-hero-mcp.md tell agents to use the kit route and ask the user in chat; PC refuses; README explains. Edges: at the limit (same as Q1); main vs agent (hook applies to both); card writers (not in CH work); worktree (n/a).
Kinds: K1; K5 (clone-hero-mcp.md:29 and server.py docstrings spell the rule the hook enforces); K6-light (which tools are key-route lives in PC:16-18 and again as prose in 4 places; the ch server owns which route a call takes (`route`) and PC copies the words "keys" and "kit").
Splits: docs say route "keys"; PC compares `route == "keys"` exactly; clone-hero-mcp.md:29 says `ch_play_chart` with no route "uses the kit once ch_kit_setup has built it, and refuses until then" and PC treats no-route as not key-route (consistent today; the server owns the default).
Ruling: none.

## Q3 What does a reviewer's last line mean, and what if it is missing? (I2-041, I2-042)
Homes: AS:21-23 (owner: whole lines `SIGN-OFF: BLOCKED` / `SIGN-OFF: CLEAN`; AS:102-109 verdict_word: neither -> CLEAN with problem). Copies: C:\Users\Patrick\.claude\agents\reviewer.md:25-33; GR\README.md:43; HR docs/agents/doc-review.md:117-125; HR docs/agents/derive-once-review.md:143-151; HR docs/agents/derive-once-review.md:86 (CLEAN when only notes), derive-once-review.md:106; active memory one-reviewer-rule-d107.md. Plus the user-scope CLAUDE.md-style rule 5 ("signs off"). The sentence "A report with no sign-off line is recorded CLEAN, because a reviewer that finished has signed off what it fixed, and the main session is told the line was missing" is repeated word for word in reviewer.md:33, README:43, doc-review.md:125 and derive-once-review.md:151, and paraphrased in AS:21-22.
What each tells the reader: all say BLOCKED only by the exact line; none line = CLEAN. "Write no hash, key or path after it; nothing else is read" appears in reviewer.md:31, derive-once-review.md:149, doc-review.md:123, README:43.
Edges: at the limit: AS:104 strips each line and matches whole lines, so a line with trailing text is not read; docs say the same. Main session vs agent: main session is told (relayed by stall.py). Card writer / non-reviewer types: the hook records only for agent_type "reviewer" (AS:238); doc text says a Workflow-launched reviewer "counts for nothing" (derive-once-review.md:200; model-postures.md:5). Worktree vs main: n/a.
Kinds: K1 (same sentence x4); K5 (the pages spell out what the hook reads instead of pointing at AS).
Splits: none on the line words. See Q4 for what "recorded CLEAN" leads to.
Ruling: review-and-hooks.md:19 (above) covers the rule (no line records CLEAN and tells the main session). Tests: test_agent_stop.py holds AS behavior; no test holds the four prose copies to AS.

## Q4 What is the main session told when a verdict has a problem or none? (I2-031, I2-045)
Homes: stall.py:268-281 text ends "A move of main to a tip with no verdict is refused."; AS:248-251 "No verdict was recorded for reviewer %s: %s. The merge stays refused until a reviewer's verdict is recorded."; AS:242-243 problem text for the cases where a verdict WAS recorded ("Reviewer %s's verdict: ..."); the refusal rule itself is RR:222 ("no reviewer verdict is recorded for %s or any commit before it on its branch"), carry/waiver routes at RR:234-253; HR derive-once-review.md:189-191, :202; README:51.
K3 split: stall.py:280-281 appends "A move of main to a tip with no verdict is refused" to EVERY verdict_problem, but AS:242-243 sets verdict_problem also when a verdict was recorded (missing sign-off line, fallback to handed tip, a file skipped). So the sentence is wrong for those cases. Also both sentences leave out the waiver and carry routes that RR:234-253 allows (K3-light).
Kinds: K3, K1 (the "no verdict, merge refused" idea in 3 places), K6-light.
Edges: main session (receives it); agent (does not); worktree (state is in GR\state, not per worktree).

## Q5 How is a reviewer tied to its handover and which commit does its verdict go to? (I2-044, I2-049)
Homes: review_dispatch.py:146-173 hashes `prompt` (utf-8) into `prompt_sha256`; AS:26-36,199-201 re-derives the same text from the transcript's first user message (text blocks joined) and hashes it again (two code places make the same fingerprint from two sources of the text; K6). AS:127-158 picks the commit; GR\README.md:39,41,45; HR derive-once-review.md:200 ("A reviewer launched from a Workflow script, or as any other agent type, gets no such record"); model-postures.md:5 same; reviewer.md:33 ("checks that the new tip descends from the tip you were handed").
README:45 states the rule fully (commits with the reviewer's `Agent:` trailer; one chain -> newest; else handed tip) and names `signed_tip` as owner: pointer plus value (home).
Kinds: K6 (hash made in two files), K1.
Edges: agent type that skips the preamble: any non-"reviewer" type leaves no record (AS:238); main vs agent n/a; worktree vs main: README:39 and derive-once-review.md:200 say a hook change in the other repo is judged in that repo; derive-once-review.md:200 lists four refusal causes, README:39 lists five (adds "a commit in a repo that has no local `main` branch"): K2-light.

## Q6 Which files count as documents for a verdict, and when does a brief ask a doc review? (I2-050, I2-051, I2-052)
Homes: AS:161-172 `DOC_SUFFIXES = (".md", ".html", ".htm")` plus the marker path; AS:175-178 asks_doc_review (RR.DOC_BRIEF); AS:224-226 message. README:49 states "a markdown file or an HTML page ... `DOC_SUFFIXES` in `agent_stop.py` owns the list" (correct).
Drift: reviewer.md:33 "fingerprints every markdown file the range changes"; derive-once-review.md:151 "every markdown file the range changes"; derive-once-review.md:202 "every markdown file in the range"; doc-review.md:21 says documents are "committed files ... public pages, release notes, plans and Artifact pages alike". The hook also records .html and .htm files; four texts say markdown only (K2). PG gates an Artifact page by path with no suffix check, so a page whose suffix is not in DOC_SUFFIXES can never have a record (not a copy, a consequence).
Also the public-page list (see Q9) is md only.
Kinds: K2 (three texts say "markdown"; the hook records more), K1.
Edges: main vs agent: n/a; card writers: n/a; worktree vs main: range is from `merge-base refs/heads/main` (AS:170): a worktree sees the same main.
Splits: the markdown-only wording vs the three suffixes.

## Q7 How is a verdict word / key kept and what counts as a clean review? (I3-036 doc key bytes, I3-038, I3-039, I3-040, I3-041, I3-042, I3-047, I3-048)
Homes: RR:65-74 doc_key (sha256, BOM dropped, CRLF->LF; PG:20-33 reads the same bytes for the small-text count and calls review_rules.normalized: shared); RR:188-196 `_clean_keys`; RR:234-259 carry/verdict/waiver rules and doc rule; PG:74-88 `_signed_off` restates the doc rule ("once a fresh agent signed off a version of a document, later edits pass ... CLEAN verdict whose path is this document's") and says "Which verdicts count is review_rules._clean_keys" (pointer, not a copy); PG:68-71 document_path_id is "the one place in this file that compares a document's path with a recorded one"; but RR:256-259 also compares the path of a recorded verdict with a changed file (two code places decide "same document path": K6).
Prose copies: HR doc-review.md:17 ("Once a fresh agent has signed off a version of a document, later edits to it pass with no new review ... Text posted to GitHub ... only as the exact text signed off"); doc-review.md:102 (same); CLAUDE.md "Doc review" ("each document gets it once"); GR README:63; derive-once-review.md:189-191 (carry rule: "when git, merging old main with a commit a reviewer signed off, makes exactly the same tree with no conflicts") vs RR:234-244 and ref_judge.py:305-316 (the same carry rule is in RR and restated as a comment in RJ: K1/K6); README:55 restates the waiver-carry for real-app.
Advice text (I3-039): RR:210-231,426-460 gives "dispatch a reviewer (subagent_type reviewer) with the brief <CODE_BRIEF> and the key <hash>, or the user sends just: waive ...". Same advice appears in derive-once-review.md:189, :200, doc-review.md:21, README:39. Words "key" and "hash": the dispatch rule says 40-char commit hash (README:39, derive-once-review.md:200); the waiver accepts 7-40 (derive-once, real-app) or 7-64 (doc-review) hex (RR:26-28); the refusal tells the user to type the first 10 (RR:77-78); docs say only `<key>` (K4-light: the user types a prefix the refusal gives).
Kinds: K1, K6 (path comparison in PG:68-88 and RR:256-259; carry rule in RR and RJ comment), K5 (docs spell out the carry and the doc rule).
Splits: none value-for-value; the "later edits pass" rule is stated as the user's rule in PG:75 ("The user's rule (Hydra CLAUDE.md, "Doc review")") but the HR CLAUDE.md Doc review paragraph says "each document gets it once"; no Confirmed line says "later edits pass" (review-and-hooks.md:27 only says every publish needs a review).
Ruling: review-and-hooks.md:27 (publish needs fresh review); not the later-edits rule.

## Q8 The waiver words (I3-027, I3-038, I2-082) 
Homes: RR:25-28 WAIVER_PHRASE and RR:77-78 waiver_phrase() (owner); waiver.py:3,19-22 (reads RR); README:37,55; doc-review.md:21 "`waive doc-review <key>`"; derive-once-review.md:189 "`waive derive-once <key>`"; reviewer.md none; PG:107 (reads RR). The `real-app` waiver word appears in README:55 and RR; no HR doc page names `waive real-app` (HR ui-testing.md:235 describes the record only).
Kinds: K1 (3 docs). Splits: docs `<key>` vs accepted 7-40/7-64 hex and "first 10" in refusal (K4-light).
Edge: main vs agent: waivers typed in the main session transcript only (human_prompt_text). Card writers skip.

## Q9 Which pages are what outsiders read (doc-review gated)? (I3-026) 
Homes: RR:23-24 PUBLIC_DOCS (owner). HR docs/agents/doc-review.md:17 lists the four paths (`README.md`, `docs/README.md`, `docs/UserGuide.md`, `docs/differences-from-public-hydra.md`); HR docs/agents/public-doc-style.md:3 lists the same four plus "the GitHub release notes"; HR CLAUDE.md "Public doc style" says "the four public pages". Different question in each, same list. No test ties them. Kinds: K1. Splits: none (public-doc-style adds release notes, which are gated by the publish route, not PUBLIC_DOCS).
Also RR:22-24 comments say "(the user's call, 2026-10-10)": no Confirmed line quotes it (checked review-and-hooks.md; the Confirmed 2026-10-10 lines do not name the pages).

## Q10 Which paths count as code, which need a real-app check, what is comment-only? (I3-025, I3-028, I3-029, I3-043, I3-044, I3-045, I3-046)
Homes: RR:21-22 BUILTIN_CODE (owner when the marker has no `gate:` lines); RR:31 UI_CODE `^src/ui/`; RR:33 _C_SOURCE extensions; RR:36-46 raw-string/splice rules; README:55 (value `src/ui/`, pointer to UI_CODE, states the comment-only rule); HR docs/agents/ui-testing.md:235 "a gate on merges that change `src/ui/`" and :233-251 the record fields (`kind`,`commit`,`dirty`,`commands`,`shots`,`started`,`stopped`) while RR:372-395 reads them and HR tools/hydra_ui_mcp.py:81,182-197 writes them (REALAPP_FILE named in RR:49 too: two code places name `realapp.jsonl`; RR comment says tools/hydra_ui_mcp.py "owns" it: K1 not K6, the name is copied); GR\README.md:138 says the gates repo's marker has `gate:` lines `gates/`, `agents/`, `tools/session_cards/`.
K8 (wrong owner claim): HR `.derive-once-gate` lines 2-4: "Which paths count is decided by Test-DeriveOnceGatedPath in the user-scope hook (hooks\lib\derive_once_rules.ps1), not by this file." HR `.doc-review-gate` lines 2-3: "Which steps and paths count is decided by the user-scope hooks (hooks\lib\doc_review_rules.ps1), not by this file." The live gates folder holds no hooks\lib\ and no such file (the only copies are inside old worktrees under C:\Users\Patrick\.claude\worktrees). The real owner today is RR:21-22 BUILTIN_CODE (Hydra's marker has no gate: lines), RR:23-24 PUBLIC_DOCS and PG. RR:21 comment says "Hydra's holds only prose" which is true. The markers' prose is read by nothing but is the first text a reader opens.
Also HR CLAUDE.md "Derive-once review" says "touch code" with no list; derive-once-review.md:189 same. Memory archive notes name an older list (src/tests/tools/CMakeLists) - out of scope.
Real-app check rule homes: README:55 (value: `commands` count at least 1, `dirty` false); ui-testing.md:245-246 (dirty/commands); RR:386-395; no test holds them equal.
Kinds: K8 (both markers), K1 (record fields in ui-testing.md, README:55, RR, hydra_ui_mcp.py), K6-light.
Edges: worktree vs main: marker is read from refs/heads/main (RR:408; PG:47; review_dispatch.py:136), so a worktree edit to a marker is not live until merged; realapp.jsonl is in the git common dir (ui-testing.md:237) shared by worktrees. Main session vs agent: agents may not run hydra_start (agents-and-workflow.md:34 Unconfirmed: "agents do not use the `hydra` MCP server").

## Q11 Which ref is main, which moves are judged, what the judge prints (I3-001, I3-002, I3-003, I3-014, I3-015, I3-016, I3-017, I3-057, I3-059, I3-064)
Homes of the strings: ref_judge.py:31-32 (ALLOW/DENY, SELFTEST_OK); GR\ref_gate.sh:15-16 copies the verdict words (shell cannot import); GR\install_ref_gate.ps1:12-13 copies the selftest line and shim marker (RJ comment line 3 of ref_gate.sh: "The marker above is the old shim's too"); tests pin them (test_ref_judge.py:614,624). Nothing ties ref_gate.sh to ref_judge.py except a test that runs the shim with a stub (test_ref_judge.py:396-416): partial.
`refs/heads/main` as the main ref is a literal or constant in: ref_judge.py:33 (MAIN), AS:170, PG:47, review_dispatch.py:111,123,128,136, cleanup_sweep.ps1:270, RR (docstring/uses). Six code places decide "which ref is main" (K6; none imports ref_judge.MAIN).
"Is this repo gated" is decided in three places: PG:42-60 gated_repo (doc marker on refs/heads/main, throws when git unreadable), review_dispatch.py:128-137 (_has_main and either marker, returns False when no local main), RR:408-410 (`_has_marker` for either marker). Different answers on a repo with no local main or unreadable git (PG raises; review_dispatch returns not-gated; RR per-rev) (K6; K3-light).
Homes of "which moves are judged": ref_judge.py:13,131-145; README:53 ("it asks review_rules.py about every move of main ... A push of local main, or of an earlier point on it, is not asked again"); README:57 for rulings ("For main the change is counted from its tip before the move, and for any other branch from the point where it left main; counted_from owns that ... A push is not judged again"); derive-once-review.md:189 ("Git runs that gate on every move of main made from Claude Code"); CLAUDE.md "Derive-once review" ("The ref gate enforces it"). Missing rules file refuses: README:53,57; ref_judge.py:236-241,275-282.
Kinds: K1 (strings, main ref), K6 (main ref and gated-repo test), K1 (counted_from described in README:57 and RR:34-37 comment in HR? (RU:34-37 prose, from the inventory)).
Edges: main session vs agent: all git calls from Claude Code are judged (CLAUDECODE=1 in ref_gate.sh:8), so agents and main alike; a user's own terminal is not judged (ref_gate.sh:8; stated in no doc page I opened except README). Worktree vs main: hooks are installed in the git common dir (README:53; install_ref_gate.ps1 per inventory) so linked worktrees share them.
No Confirmed line speaks to the shim words. Ruling review-and-hooks.md:30 (chose "Git-side hook (Recommended)" to "How should the review gate stop unreviewed changes reaching main?") speaks to the design only.

## Q12 When does a hook crash refuse and when does it only inform? (I2-074, I2-086, I3-060, I3-083, I3-015 for crash, I3-065 matcher)
Homes: common.py:53-69 run(body, mode) ("closed" denies, "loud" message); README:93-100 (rule and meaning of "closed"/"loud"; "The refusing hooks carry `onFailure: block`"); each script's `__main__` picks the mode: delete_rule.py:187 closed, percall.py:176 closed, publish_gate.py:135 closed, review_dispatch.py:178 closed, posture.py:55 loud, prompt.py:67 loud, waiver.py:50 loud, shell_output.py:201 loud, agent_stop.py:297 loud, ruling_rules.py:43,394 ("a crash refuses, like every refusing gate"), ref_judge.py (crash refuses, :401). prompt.py:17-18 PARTS repeats the mode of three scripts ("each mode is the one its script's __main__ uses"): a second copy of three modes (K1/K6). settings.fragment.json:153,169,185 give onFailure block to percall, review_dispatch and publish_gate only; delete_rule is reached through percall. README:23 and :71 say informing hooks have no onFailure; test_docs_numbers.py:163 holds it for the PostToolUse hook only.
K3 candidate: percall's mode inside prompt.py PARTS is "closed", but prompt.py itself runs "loud" (prompt.py:67) and its registration has no onFailure; prompt.py:6-7 says "A prompt cannot be refused here, so a part's deny or ask is said in the systemMessage instead". So percall's "closed" at prompt time becomes an inform. README:97 says "closed is for hooks that can refuse".
Ruling: agents-and-workflow.md:18 "agents should fail loudly. freezing is unacceptable" (about agents).
Test holding homes equal: none for PARTS vs scripts; test_docs_numbers.py:163 only the PostToolUse case.

## Q13 Did the user type it? (I3-037, I2-084, I2-085, I2-086)
Homes: RR:89-113 human_prompt_text (owner; checks type user, promptId, `turnOrigin == "human"`, not isMeta, not isSidechain, no toolUseResult, text blocks only); waiver.py:3-6 (comment says why it records ids and RR checks later; names RR); README:37; PC:34 uses RR. A second code answer to "what is the user's typed text in a transcript": C:\Users\Patrick\.claude\tools\session_cards\common.py:372-419 classify_user/human_text and :422-435 queued_prompt_text. It reads `origin.kind == "human"` (not `turnOrigin`), strips `<system-reminder>` blocks, accepts slash-command arguments, and treats queued prompts as typed; RR reads `turnOrigin`, rejects any non-text block, does not strip reminders, and does not read queued prompts or slash commands. GR\ruling_rules.py:16,270 calls session_cards is_users_words ("the one home of that test", README:59, check.py:164) for rulings, but RR for allows/waivers. Same question ("did the user type this"), two code owners with different field names and rules (K6; K2 on the field name, K3 on what counts).
Ruling: none. Test: test_session_cards.py:118,721 hold the card tool's reader; test_review_rules covers RR; nothing compares them.
Edges: main vs agent: agent transcripts never count for rulings (rulings/README.md:43) and RR rejects isSidechain; card writers n/a.

## Q14 Rulings approval (I3-073, I3-075, I3-076, I3-077, I3-078, I3-079)
Homes: ruling_rules.py:95-101 CONFIRMED/UNCONFIRMED headings, INSTALL_DATE "2026-11-14", APPROVE/DECLINE = "File it"/"Do not file", QUESTION = "File this ruling in {path}?" (owner); HR CLAUDE.md "Rulings and this machine" ("clicks "File it" for that exact line in a question box, and git refuses any other"); HR docs/agents/agent-rules.md:33; HR docs/agents/rulings/README.md:3,41-43 ("The question reads `File this ruling in docs/agents/rulings/<topic file>?`, followed by the whole line. The two labels are `File it` and `Do not file`."; "An agent's transcript never counts" at :43,47); GR\README.md:57-61; GR\README.md:59 "`is_users_words`... the one home". Rulings README:20 says Unconfirmed holds lines "copied from memory notes on 2026-10-31"; the Unconfirmed section of review-and-hooks.md also holds lines dated 2026-11-06 (lines 40-43), so that README sentence no longer describes what the sections hold (side note, K2-light). Rulings README:49 backfill route text vs RU:327-348 (not re-read).
Kinds: K1 (labels, question text, heading names in 4 texts); K5 (rulings README spells the shape the gate checks); a test holds only the heading and an old phrase (test_docs_numbers.py:129-135), not the labels.
Edges: main vs agent: agents can never approve (README:43, :47); worktree vs main: transcripts are found through pointers in state\rulings.jsonl (README:59).

## Q15 Small-text limit and which calls publish_gate gates (I3-062, I3-063, I3-065, I3-066, I3-067)
Homes: PG:13-14 `SMALL_LINES, SMALL_CHARS = 20, 2000` (owner; PG:5, GR\README.md:63 "the one home of the short-text limit"; PG:103-107 and publish.ps1:3 read it). Prose: HR doc-review.md:17 "A short text passes the last three with no review; `publish_gate.py` ... owns that limit" (pointer, states no number); no other text states 20 or 2000 as the limit (searched HR docs/agents, CLAUDE.md, GR, memory active, agents). The refusal text carries the numbers by reading the constants (PG:107). Archived memory hook-overhaul-2026-10.md:18 "only items over 20 lines" is out of scope.
Which calls: PG:112-120 (ExitPlanMode with a plan; Artifact with action none/""/publish and a file_path, not asset); settings.fragment.json:174 matcher "Artifact|ExitPlanMode"; doc-review.md:17 "an Artifact publish; a plan shown with ExitPlanMode; and text posted to GitHub"; README:63-65; publish.ps1:8,59-62 kinds (pr, release, release-edit, issue, issue-comment, pr-comment) restated in doc-review.md:19 and README:65 and issue-tracker.md (K1; publish.ps1 owns). The tool names and the `action` values copy Claude Code's input shape in two code places (matcher and PG:112-120).
The counting rule (I3-063): PG:29-33 says "as the old PowerShell gate counted them" (an old rule copied, with no old script left to compare: the old files exist only in old worktrees).
Kinds: single home for the numbers (pointer elsewhere). K1 for the call list and publish kinds. The 2000-character half has no Confirmed line (the user's "over 20 lines" is at review-and-hooks.md:22; the 2000 appears in PG:13 as "the user's decisions of 2026-10-10").
Edges: at the limit: both limits must hold (PG:13): exactly 20 non-blank lines or exactly 2000 UTF-16 units passes (PG:31,33 use `>` and `<=`); main vs agent: the gate does not look at agent type; card writers n/a; worktree: gated_repo reads main's marker (Q11).

## Rows with one home only
- I3-021-type hash length rows are not mine. For my rows with no copy found after the searches above: I3-014 (judged moves; README:53 is a pointer-plus-value home, see Q11), I3-016 (README:57 states the counting rule: a home), I3-017 (README:57 and :53 state it: home). I3-063 (the counting method) has one home: PG:29-33; searched PG, README, doc-review.md, tests.

## Candidates by kind (not graded)
K1: allow phrases; sign-off sentence; shim strings; public-page list; waiver words; ruling labels; publish kinds; record fields; failure modes in PARTS.
K2: "exactly" vs case/./! (Q1); "markdown" vs .md/.html/.htm (Q6); derive-once-review.md:200 four causes vs README:39 five.
K3: stall.py:280-281 refusal sentence on every verdict_problem (Q4); percall "closed" inside prompt.py PARTS (Q12).
K4: waiver `<key>` in docs vs 7-hex prefix the refusal gives (Q7/Q8).
K5: preamble:49, agent-rules:14, clone-hero-mcp.md:29, server.py:146-148 spell the hook rule; reviewer.md:31-33, doc-review.md:123-125, derive-once-review.md:149-151 spell the verdict rule; rulings README:41-43.
K6: hash of the brief (review_dispatch.py:172 vs AS:199); "did the user type" (RR:89-113 vs session_cards\common.py:372-435); "is this repo gated" (PG:42, review_dispatch.py:128-137, RR:408); "which ref is main" (6 places); "same document path" (PG:68-88 vs RR:256-259); `--whole-library` word (PC:14 vs src/cli/batch_scope.h:10).
K7: rule 8's "any whole-library run" has the hook only for the flag (Q1).
K8: HR `.derive-once-gate` lines 2-4 and `.doc-review-gate` lines 2-3 name hooks\lib\*.ps1 as owner; the owner is review_rules.py and publish_gate.py.

## Splits (own lines)
1. Docs say the allow must be "exactly `allow library run`" / "`allow key run`"; RR:52,55 accepts any case and one trailing `.` or `!`.
2. Rule 8 / agent-rules.md:14 cover any whole-library compare or timing run; PC:47-48 judges only a shell command that contains `--whole-library`.
3. stall.py:280-281 says a move to a tip with no verdict is refused for every verdict problem; AS:242-243 sets verdict_problem when a verdict was recorded.
4. reviewer.md:33, derive-once-review.md:151,202 say the hook fingerprints "markdown" files; AS:163 records .md, .html and .htm.
5. The two markers say the path rules live in hooks\lib\derive_once_rules.ps1 and doc_review_rules.ps1; those files are not in the live gates folder; RR:21-24 holds the lists.
6. review_rules.human_prompt_text (field `turnOrigin`, text blocks only) vs session_cards\common.py human_text/classify_user (field `origin.kind`, strips reminders, accepts slash-command arguments).
7. derive-once-review.md:200 lists four reasons a reviewer dispatch is refused; README:39 lists five.
8. HR rulings/README.md:20 says Unconfirmed holds lines copied on 2026-10-31; review-and-hooks.md:40-43 (Unconfirmed) are dated 2026-11-06.

## Side notes
- RR:22 comment "Hydra's holds only prose" is true today; it will go stale if the marker gains `gate:` lines.
- test_docs_numbers.py reads HR docs from the main checkout by default (DEFAULT_HYDRA_ROOT), so it checks main's copy, not a branch's.
- A way past a hook, general words only: the whole-library check reads the shell command text, so a run that does not put the flag in that text is not seen by it.
- README:41 says "wording it keys on was read from real agent transcripts" for open_background_tasks; not examined.
- Rows not reached with full search: none skipped; but I did not read the live settings.json, so the matcher/registration side of I3-065 and the failure-mode side of I3-060 are checked against the fragment only; I3-063 and I3-045 (comment-only reading details) were searched for other homes only by name and phrase, not by re-reading the algorithm.
