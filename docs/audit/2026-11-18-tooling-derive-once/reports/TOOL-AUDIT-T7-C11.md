# TOOL-AUDIT-T7-C11 closure check (sorting TOOL-AUDIT-T6-S06.tsv, ledger TOOL-AUDIT-T5-L06.md)

Paths: GATES = C:\Users\Patrick\.claude\gates, HYDRA = C:\Users\Patrick\Downloads\Hydra\hydra-test (main checkout). Line numbers are as read now.
Totals: PASS 22, REOPEN 1, GONE 2, not reached 0.

### L-F16-04
**Verdict:** PASS (P)
**Homes read:** GATES/percall.py:15-18 (KEY_TOOLS = ch_key, ch_press; KEY_ROUTE_TOOLS = ch_play_chart -> "keys"), :70-84 (_key_call); GATES/README.md:13; HYDRA/tools/ch_mcp/game_tools.py:70-72 (ROUTE_KIT "kit", ROUTE_KEYS "keys"), :1081-1108; HYDRA/docs/agents/clone-hero-mcp.md:29; ch_mcp/server.py:148,:466,:477.
**Why:** Same tool set and same "keys" value in all homes. Nothing disagrees about which calls count. (clone-hero-mcp.md:29 says "typed exactly", but that is the allow-phrase edge, see L-F17-02, not this row's question.)

### L-F16-05
**Verdict:** PASS (P)
**Homes read:** GATES/shell_output.py:14-38 (CAP 4000, HEAD/TAIL 1500, lists, TAILS); HYDRA/docs/agents/brief-preamble.md:35 ("over 4,000 characters"; 30,000 is Claude Code's own ceiling, not in the hook); GATES/README.md:23-25.
**Why:** Preamble copies 4,000, code is 4000; README points to CAP, BUILD_TEST_COMMANDS, NO_TRIM_COMMANDS in code. No differing value.

### L-F16-06
**Verdict:** PASS (A)
**Homes read:** HYDRA/docs/agents/brief-preamble.md:73 ("At 150 tool calls or 30 minutes it stops you"); GATES/progress.py:13-16 (CALL_WARN, CALL_STOP = 100, 150), :85-95 (increment then warn at >= 100), :129 (refuses when calls >= CALL_STOP, so the 150th call is refused); HYDRA/docs/agents/agent-rules.md:13.
**Why:** The one split is doc wording at the edge call against a program file. Neither side is a memory note or ruling.

### L-F16-09
**Verdict:** PASS (P)
**Homes read:** GATES/ruling_rules.py:105-110 (CHECK_COMMAND); GATES/settings.fragment.json:6 (allow rule, same string); HYDRA/docs/agents/rulings/README.md:40 (prose "ruling_rules.py --check <file>", names no other form).
**Why:** The command words agree.

### L-F16-10
**Verdict:** PASS (P)
**Homes read:** GATES/wake.py:22-24 (WAKE 2, QUIET 0, UNHEARD 1), :66-114 (_crash, main); GATES/README.md:31 (crash wakes once, same crash exits 1); GATES/tests/lib/harness.ps1:15-25,:51,:86 (TimeoutSec 120, HarnessDrainMs 5000).
**Why:** README matches wake.py. No doc states the harness defaults, so nothing differs. Live hook timeouts in settings answer a different question.

### L-F16-11
**Verdict:** PASS (P), with a note
**Homes read:** GATES/tests/replay.py:44-62 (old-hook regexes, incl. `Budget of \d+ tool calls reached`), :283-308 (tightening substrings incl. "Your budget is spent"); GATES/progress.py:127-131 (live text).
**Why:** The old regex deliberately describes a retired hook's text (replay.py:44-45 comment; obj3 is "not replayed"); the live-text substring in tightening() matches progress.py:130. No two homes claim one thing differently. Note: the sorting line says the strings "match the live texts"; the old budget regex does not equal the live text, it is history, and there is nothing to point at the owner for it. The row still has no disagreement.

### L-F16-12
**Verdict:** PASS (P)
**Homes read:** GATES/delete_rule.py:187, percall.py:210, prompt.py:18 (PARTS "closed" for percall), common.py:53-72 (modes, closed denies on crash); GATES/README.md:97.
**Why:** All "closed"; README says a crash denies the call.

### L-F16-15
**Verdict:** PASS (P)
**Homes read:** HYDRA/tools/check_pointers.py:15-16,:32,:83 (HOOK_EXTENSIONS .py .ps1 .sh); HYDRA/tools/doc_tokens.py:30-37,:77,:88 (KNOWN_EXTENSIONS, BRANCH_PREFIXES "claude/","hx-"); HYDRA/docs/agents/handoff.md:57-59 (exit 0/1/2; "known extension"); HYDRA/tools/new_worktree.ps1:1-8 (claude/<TaskId>); GATES/README.md:71; GATES/settings.fragment.json:95-106 (timeout 15); brief-preamble.md:97.
**Why:** Two extension lists answer two questions (hook files vs any path token); exit codes 0/1/2 and "always exits 0 with --session-start" agree in code, handoff.md and README. "hx-" appears only in doc_tokens.py (searched HYDRA/tools and docs/agents). No disagreement.

### L-F16-16
**Verdict:** PASS (P)
**Homes read:** HYDRA/tools/build_slot.ps1:90,:326,:329-330; HYDRA/Directory.Build.targets:9,:13; HYDRA/docs/development.md:143; GATES/percall.py:20 (BUILD_PASS_TOOL); HYDRA/tools/hydra_ui_mcp.py:874 (def hydra_build_pass); GATES/README.md:15.
**Why:** Same CLAUDECODE == 1 condition; error text and tool name consistent.

### L-F16-17
**Verdict:** PASS (P)
**Homes read:** HYDRA/tools/hydra_ui_mcp.py:627-632,:694-708; docs/agents/ui-testing.md:24,:229; tools/test_hydra_ui_mcp.py:773,:949,:956; tools/doc_shots.ps1:8.
**Why:** "stopped at line N of M" is the same in code, doc and tests; exe path build-cpp\Release\hydra_uitest.exe the same in doc_shots.ps1 and ui-testing.md. ui-testing.md does not restate "one command is one line" (searched), so no second copy.

### L-F16-18
**Verdict:** GONE
**Homes read:** HYDRA/tools/fake_hydra_live.py:10-13 (docstring "50,000 characters"), :63-65 (BIG_LINE = 45 x + " row" = 49 chars; BIG_TEXT = 1000 lines joined by newline plus a final newline); HYDRA/tools/hydra_ui_mcp.py REPLY_CAP_CHARS (12,000 cap; ch_mcp/server.py:43).
**Why:** The claimed split (docstring 50,000 against BIG_LINE "about 49,000") does not exist: 1000 lines of 49 characters plus 1000 newlines is exactly 50,000. The comment above BIG_LINE says "49 characters plus a newline", which agrees. Nothing to close.

### L-F16-21
**Verdict:** PASS (C)
**Homes read:** HYDRA/src/core/model.h:121 (kSqueezeWindowMs), :130-134 (within_squeeze_window); C:\Users\Patrick\.claude\skills\analyzing-sp-squeezes\SKILL.md:10 ("kSqueezeWindowMs, src/search/graph.h"); HYDRA/src/search/graph.h:216 (comment only); HYDRA/docs/clone-hero/05-star-power.md:399 ("line 123"), :417 and :419 ("line 111").
**Why:** Every split is a stale file or line cite. Current places: model.h:121 for the constant, model.h:133 for within_squeeze_window. Clear.

### L-F16-22
**Verdict:** PASS (P)
**Homes read:** HYDRA/tools/doc_shots/shots.txt:10-20 (Reset to defaults, expect "Analysis settings: defaults"); docs/agents/doc-screenshots.md:20-25; tests/ui/uitest_library.cpp:254,:273,:322; src/ui/library_dialogs.cpp:137; tools/precheck/main.cpp:10-11,:41 (main...HEAD default); docs/agents/derive-once-review.md:17,:24 (explicit range); tools/mutate/run.py:79 (32767); GATES/bash_windows_rule.py:16 (1000); tools/capture/steps.ps1:3-8.
**Why:** Copies agree; the 32767 and 1000 limits answer different questions.

### L-F17-02
**Verdict:** REOPEN. Category should be Q, asked together with L-F17-01.
**Homes read:** GATES/review_rules.py:50-55 (`_KEY_ALLOW = re.compile(re.escape("allow key run") + r"[.!]?", re.IGNORECASE)`, "matched the same way" as the library phrase); HYDRA/docs/agents/clone-hero-mcp.md:29 ("unless the user has typed exactly `allow key run`"); HYDRA/tools/ch_mcp/server.py:148,:466,:477 (say "typed allow key run"); GATES/README.md:13,:37.
**Why:** The row is not free of disagreement. clone-hero-mcp.md:29 says "exactly"; the hook accepts any case and one trailing "." or "!". That is the same doc-against-hook edge that L-F17-01 is a Q for (sorting line for L-F17-02 itself says it is "covered by the L-F17-01 question"). A P row needs every home to agree; this one does not on the key phrase, and the same user answer settles both. Move it into the L-F17-01 question.

### L-F17-03
**Verdict:** PASS (P)
**Homes read:** GATES/agent_stop.py:21-23,:113-120 (verdict_word: neither line -> CLEAN with a problem noted); C:\Users\Patrick\.claude\agents\reviewer.md:33; HYDRA/docs/agents/doc-review.md:125; HYDRA/docs/agents/derive-once-review.md:151; GATES/README.md:43.
**Why:** The sentence "A report with no sign-off line is recorded CLEAN ... the main session is told the line was missing" is identical in reviewer.md, doc-review.md and derive-once-review.md and matches verdict_word.

### L-F17-05
**Verdict:** PASS (P)
**Homes read:** GATES/review_dispatch.py:184 (prompt_sha256 = sha256 of prompt utf-8); GATES/agent_stop.py:238-240 (sha256 of the child's first message, matched to handover prompt_sha256); GATES/README.md:47,:188; GATES/tests/probe_results.md:28; C:\Users\Patrick\.claude\model-postures.md:5.
**Why:** Both ends hash the same way; docs state the same. No stated disagreement on the hash.

### L-F17-06
**Verdict:** GONE
**Homes read:** HYDRA/docs/agents/derive-once-review.md:17,:200,:207; GATES/README.md:39; GATES/review_dispatch.py:75-132 (_review_problem), :140-149.
**Why:** The "four reasons" list (no key, already on main, unknown to git, more than one commit) is no longer in derive-once-review.md; line 207 now says the hook refuses lines missing/doubled/out of form and "a key, start or folder that git does not confirm", and that `review_dispatch.py` owns the full list. README.md:39 no longer lists a "no local main" cause; it points to `_review_problem`. The code now refuses on "no local branch named <start>" (:124), not "no local main". Neither doc counts causes, so four-vs-five is gone.

### L-F17-07
**Verdict:** PASS (A)
**Homes read:** GATES/agent_stop.py:169 (`DOC_SUFFIXES = (".md", ".html", ".htm")`), :178; HYDRA/docs/agents/derive-once-review.md:151,:211 ("every markdown file"); HYDRA/docs/agents/doc-review.md:29 ("every markdown file the range changes"); C:\Users\Patrick\.claude\agents\reviewer.md:33 (now says "every document", no longer a split); GATES/README.md:49.
**Why:** Remaining split is two docs saying markdown against a program file that also takes HTML. Both sides are doc against code; the code wins.

### L-F17-10
**Verdict:** PASS (P)
**Homes read:** GATES/review_rules.py:24 (PUBLIC_DOCS: README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md); HYDRA/docs/agents/doc-review.md:17 (same four); HYDRA/docs/agents/public-doc-style.md:3 (same four plus release notes); HYDRA/CLAUDE.md:45 ("the four public pages").
**Why:** Same four pages everywhere.

### L-F17-11
**Verdict:** PASS (C)
**Homes read:** HYDRA/.derive-once-gate line 3-4 (names hooks\lib\derive_once_rules.ps1); HYDRA/.doc-review-gate line 2-3 (names hooks\lib\doc_review_rules.ps1); GATES/review_rules.py:18-25 (CODE_MARKER, BUILTIN_CODE :22, PUBLIC_DOCS :24), :158-180 (_code_paths); searched C:\Users\Patrick\.claude\hooks\lib: does not exist.
**Why:** Both marker comments cite files that no longer exist. Current owner is GATES/review_rules.py: BUILTIN_CODE for the code-path list and PUBLIC_DOCS (plus the gate markers) for the doc pages. Clear.

### L-F17-15
**Verdict:** PASS (P)
**Homes read:** GATES/ruling_rules.py:93-101 (headings, INSTALL_DATE 2026-11-14, "File it"/"Do not file", QUESTION "File this ruling in {path}?"); HYDRA/docs/agents/rulings/README.md:3,:41-43,:51; HYDRA/docs/agents/agent-rules.md:33; HYDRA/CLAUDE.md:41; GATES/README.md:57-59.
**Why:** Labels, question text, heading names and install date agree in every copy.

### L-F17-18
**Verdict:** PASS (P)
**Homes read:** GATES/publish_gate.py:110-121 (ExitPlanMode with plan; Artifact with action None/""/"publish", file_path, asset not True); GATES/settings.fragment.json:174 and C:\Users\Patrick\.claude\settings.json:184 (matcher "Artifact|ExitPlanMode"); HYDRA/docs/agents/doc-review.md:17.
**Why:** Matcher and code agree and the doc names the same gated steps.

### L-F17-19
**Verdict:** PASS (P)
**Homes read:** GATES/review_rules.py:366 (change_is_comments_only), `_c_code` present; GATES/README.md:55.
**Why:** README restates the case in words and names the owner functions; no differing value.

### L-F17-20
**Verdict:** PASS (P)
**Homes read:** GATES/settings.fragment.json and C:\Users\Patrick\.claude\settings.json, both read as UTF-8 and compared as parsed data.
**Why:** The hooks blocks are equal (same serialized form), and fragment permissions allow/deny/ask are all present in the live file.

### L-F18-05
**Verdict:** PASS (A)
**Homes read:** GATES/agent_stop.py:24 (comment citing Hydra CLAUDE.md rule 5), :169; HYDRA/docs/agents/derive-once-review.md:151,:211; HYDRA/docs/agents/doc-review.md:29; reviewer.md:33; HYDRA/CLAUDE.md rule 5 ("fixes every finding itself, commits ... and signs off").
**Why:** Same markdown-versus-DOC_SUFFIXES split as L-F17-07: docs against a program file. The rule 5 comment cite fits what rule 5 says about a reviewer who signs off what it fixed.

## Ids not reached
None.
