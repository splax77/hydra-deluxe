# TOOL-AUDIT-T5-V30 (verifier of F30). All claims by reading; nothing run. Repo = main checkout C:\Users\Patrick\Downloads\Hydra\hydra-test; gates = C:\Users\Patrick\.claude\gates.

Checked 6 headline candidates plus S1-S6. The rest of F30's families were NOT reached (list at the end).

## 1. Exe list "agents never run hydra_tests/uitest/replay" copied (Family A, K1/K5/K7): CONFIRMED
Quoted: CLAUDE.md rule 14 ("Agents may run `hydra_precheck.exe` from main, but not `hydra_tests.exe`, `hydra_uitest.exe` or `hydra_replay.exe`"); agent-rules.md:20 (same list plus "Any test that needs a build is the main session's too"); executor.md:11 (list plus the user quote "any tests that require a build are done by the main session, not agents."); reviewer.md:21 (same quote and list); brief-preamble.md Tests section (list, "not even the main checkout's copies"); gates README.md:17 ("No hook enforces that part ... An agent may not run hydra_tests.exe, hydra_uitest.exe or hydra_replay.exe").
Hook side: percall.py lines ~108-111 refuse only hydra_build_pass ("Agents never build, so hydra_build_pass is refused to every agent"). The three-exe part is prose only (K5/K7). Not re-opened: DOR:116, UT:17/27/185, TM:95, CH:7 (F30's quotes unverified for these). DS (doc-screenshots.md) I did read: "the main session runs it, never an agent (agent rule 14)".
Situation where hook and prose differ: an agent running one of the three exes is not stopped by any hook (README:17 says so itself). A gap exists in the gates; stated in general words only.
Ruling: rulings/agents-and-workflow.md:8,:9 not re-opened by me. The user quote is in executor.md:11 and reviewer.md:21 as read.

## 2. `allow library run` rule copied; hook tests only the whole-library flag (Family B, K1/K5): CONFIRMED
percall.py:14 `LIBRARY_FLAG` (the flag text); :48 no refusal unless the command holds that flag text. review_rules.py:50-51 `LIBRARY_ALLOW_TEXT = "allow library run"`. agent-rules.md:14 (rule 8): "any run over the user's whole song library (for example the hydra_batch flag, or a whole-library compare or timing run). The user allows one run by sending exactly `allow library run`; each such message lets one command through. Ask only when nothing smaller can answer". brief-preamble.md Tests: the hook "refuses any shell command that holds" the flag, "unless the user has sent exactly `allow library run`".
S3 CONFIRMED-CHANGED: the preamble describes the hook exactly (flag text), but agent-rules.md:14 and the preamble's own first sentence ("no compare or timing run over the user's whole song library") name runs wider than the flag. A library-wide compare or timing run without the flag is prose-only. Situation: a command over the library that lacks the flag text. UT:203 ("narrow the library with a search first") not re-opened.
Rulings: rulings/tests-and-runs.md:14,:18,:21 not re-opened; as F30 quotes them, none names a flag-versus-wider-rule split, so they do not cover S3.
Side effect seen: my own first attempt to write this report through a shell heredoc was refused by the hook because the text contained the flag string. The hook tests the command text, not what the command does.

## 3. Owner-map / premise-map rules restated (Families C, D): PARTLY CHECKED
owner-map.md:11-13: "Every plan that changes what Hydra decides or shows needs an owner map ... A plan that only moves files, edits docs or changes hooks needs none." CLAUDE.md's Owner map paragraph has no carve-out: CONFIRMED. planner.md, reviewer.md, scout.md not re-opened: unchecked. Wording is shorter, not opposed; no split.

## 4. Public-pages list (side note 2): CONFIRMED
review_rules.py:23-24: "The pages outsiders read (the user's call, 2026-10-10). Each is reviewed once." PUBLIC_DOCS = README.md, docs/README.md, docs/UserGuide.md, docs/differences-from-public-hydra.md. CLAUDE.md says "the four public pages". public-doc-style.md:3 list not re-opened. They agree today; the hook is the code home.

## 5. Triage labels (I6-237): CONFIRMED
triage-labels.md table: every row's two label strings identical; last line "Edit the right-hand column to match whatever vocabulary you actually use." CLAUDE.md: "each label string equal to its name". Agree today; stale if the right column is edited.

## 6. Project-word rule copied 5 times (Family F): NOT REACHED.

## Splits (every one my tables show)
- S1 (agent-rules vs agent files, exe lists): CONFIRMED-CHANGED to plain K1 copy. agent-rules.md:20, executor.md:11, reviewer.md:21, preamble, CLAUDE.md agree word for word on the list. Not a split today.
- S2 (SP-squeezes skill runs hydra_replay without rule 14): REFUTED as a split. SKILL.md:18 and :44 describe what the script `fcvideo.py` does ("hydra_replay dump"; "hands the observed activations to hydra_replay.exe (repo tool)"); they do not tell an agent to run hydra_replay. SKILL.md:35 is a table row saying not to launch the GUI / hydra_uitest. Whether an agent running the script would run hydra_replay: open; only a run settles it; I did not read video-tools\fcvideo.py.
- S3: CONFIRMED-CHANGED, see 2.
- S4 (CH:43 "ends with ch_stop's restore lines"): CONFIRMED as a rule nothing checks. clone-hero-mcp.md:43 states it. tests/test_docs_match_code.cpp has `ch_stop` once (line 1978, inside a sample Calls: line); premise-map.md has it only in its own example Calls line (:61). No test found requiring a ch_stop step in every live step; gates not searched for it. K7, not a two-answer split.
- S5 (screenshots triggers): CONFIRMED. doc-screenshots.md "When to run it": "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows." development.md:977-983: "Run it before each release". development.md points to doc-screenshots.md for the rest. Situation: a label or layout change between releases.
- S6 (public-doc-style 77 vs 5): CONFIRMED. PD:77: a release-note rewrite keeps "the names of things at the time, even ones that changed later". PD:5: "check every name and number against the code before you write it". The page does not say which wins for an old release note. No ruling checked.
- Splits added: none.

## Not reached
Family C beyond the carve-out; D except S4; E; F; G except I6-237; H except S5; J; K; L; the other homes in Family A (DOR:116, UT, TM, CH:7, test-rubric); every rulings-file line; planner/scout/reviewer text.
