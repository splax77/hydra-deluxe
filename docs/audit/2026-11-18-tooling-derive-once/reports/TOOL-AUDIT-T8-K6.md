# TOOL-AUDIT-T8-K6 closure check (18 ids)

H = C:\Users\Patrick\Downloads\Hydra\hydra-test ; G = C:\Users\Patrick\.claude ; M = G\projects\C--Users-Patrick-Downloads-Hydra-hydra-test\memory

### L-F30-04
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\agent-rules.md:21 "the new owner goes in core"; :25 "Under an owner map, 'if none does, the new owner goes in core' above waits for the user"; H\docs\agents\owner-map.md:37 "Nobody writes a new owner for it"; G\agents\scout.md:11 "Never propose a new owner".
**Why**: line 25 itself says which side wins under an owner map, and owner-map and scout agree with it. CLAUDE.md rule 15 never says "goes in core", so there is nothing to carve out of.

### L-F30-09
**Verdict**: PASS (exit 3, kind doc-vs-doc).
**Homes read**: H\docs\agents\rulings\README.md:34 "`CONTEXT.md` wins and the session asks the user"; H\docs\agents\brief-preamble.md:99 "change neither"; H\docs\agents\agent-rules.md:41,:43; H\docs\agents\derive-once-review.md:51; H\docs\agents\domain.md:11.
**Why**: all docs, one question (what a session does when a reading and CONTEXT.md differ), worded two ways. The domain.md:11 side only says when the files are created, so it is not part of the split.

### L-F30-10
**Verdict**: PASS (exit 3, doc-vs-doc).
**Homes read**: H\docs\agents\doc-screenshots.md:23 "Run it before each release ... Also run it after any change to a label or a layout that one of the pictures shows"; H\docs\development.md:977-983 "Run it before each release".
**Why**: two docs, one question (when to refresh the pictures), two answers.

### L-F30-11
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\public-doc-style.md:5 (check every name and number against the code) and :77 (a rewrite of an old release note keeps the names of things at the time).
**Why**: line 5 is writing new text, line 77 is rewriting an old note. Different questions.

### L-F30-13
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\audit-method.md:98 "give a recommendation and the strongest case against it"; M\counts-on-screen-must-agree.md:16 and :21 "Never recommend keeping one side 'unchanged'".
**Why**: both recommend. The note bars only one answer (keep a side as is) when the code disagrees with itself, a narrower case than audit-method.md:98.

### L-F30-15
**Verdict**: PASS (exit 3, "Note is wrong").
**Homes read**: M\derive-display-from-engine-truth.md:18 "If none exists, propose one rather than copying"; G\agents\scout.md:11; H\docs\agents\owner-map.md:37; H\docs\agents\agent-rules.md:25; CLAUDE.md rule 15.
**Why**: the note's first half (find the one owner and call it) is in rule 15, so pruning only the "propose one" sentence drops no rule. Every doc says a missing owner goes to the user.

### L-F30-17
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\brief-preamble.md:123; H\docs\agents\doc-review.md:81 ("That file is the one home of the style rules"); G\tools\session_cards\writer-brief.md:94; H\docs\agents\public-doc-style.md:3 (internal docs follow ~/.claude/CLAUDE.md).
**Why**: each page names the global CLAUDE.md as the home and lists a subset; none contradicts another. This is shortened copies, not two answers. If the main session wants the copies trimmed, that is a separate tidy job.

### L-F30-23
**Verdict**: PASS (GONE).
**Homes read**: H\docs\agents\issue-tracker.md:20 (`/triage`) and :40 (`/wayfinder`); G\plugins\cache\claude-plugins-official\mattpocock-skills\1.3.1\skills\engineering\ (holds `triage` and `wayfinder`); G\settings.json:254 `"mattpocock-skills@claude-plugins-official": true`; G\plugins\installed_plugins.json:4-7.
**Why**: exits.md:607 states the split as "~/.claude/skills holds neither". The skills come from an enabled, installed plugin, so they exist. Searched also G\skills (only analyzing-sp-squeezes, reverse-engineer-anything, synced).

### L-F31-01
**Verdict**: PASS (exit 3, "Note is wrong"). Same as L-F30-15.
**Homes read**: M\derive-display-from-engine-truth.md:18 against owner-map.md:37, scout.md:11, agent-rules.md:25.
**Why**: the agent-rules.md:21 side is reconciled by :25, so only the note is out of line.

### L-F31-03
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\integrate.md:23 "If a fold would change a displayed text, a score or a stored record, stop and report it"; H\docs\agents\derive-once-review.md:96 and :106; H\docs\agents\rulings\agents-and-workflow.md:19 (Confirmed 2026-10-04 "if anything changes a display in the app or a record, run it by me").
**Why**: integrator and reviewer both stop on a display or score change. Same answer.

### L-F31-06
**Verdict**: PASS (exit 4).
**Homes read**: H\docs\agents\rulings\ui-and-display.md:56 (complaint) and :57 (Confirmed, same day, "this will be an intentional deviation from the game. it looks better with both."); H\src\parse\song.cpp:188-196 (every_lane_pad, "deliberate deviation ... D254"); M\drum-lanes-d112-2026-10.md.
**Why**: line 56 is the user's complaint, line 57 the user's answer choosing both pads lit. The code follows line 57. Both lines are Confirmed, and they answer different things.

### L-F31-14
**Verdict**: PASS (exit 3, doc-vs-doc).
**Homes read**: H\docs\agents\test-rubric.md:45 ("Agents may not run hydra_replay ... asks the main session"); G\skills\analyzing-sp-squeezes\SKILL.md:18 (the tool "gets the path from `hydra_replay dump`"); H\docs\agents\agent-rules.md rule 14.
**Why**: the skill does not say who runs hydra_replay, so an agent following it can break rule 14. Both sides are docs.

### L-F31-15
**Verdict**: PASS (exit 4).
**Homes read**: M\derive-display-from-engine-truth.md:23 (a squeeze is drawn at its chord); G\skills\analyzing-sp-squeezes\SKILL.md:22-23 (a SqOut gap is measured from the pre-collection end).
**Why**: one says where a squeeze is drawn, the other which SP end a gap is measured from.

### L-F31-23
**Verdict**: PASS (exit 4), with one caution.
**Homes read**: C:\Users\Patrick\Downloads\Hydra\video-tools\README.md:45 (it is now line 45, not 41-43: "a version problem or a bug, never a real score"); G\skills\analyzing-sp-squeezes\SKILL.md:55; M\dmleaderboards-comparison.md:26-29 ("Above-optimal scores are NORMAL ... Just label them").
**Why**: one is a check on one video run, the other how to label leaderboard rows, and both name an older game version as the cause. Caution: "never a real score" and "legitimately achievable then" read close to opposite. If the main session wants it airtight, the README sentence could say it means "a real score on the current rules".

### L-F31-25
**Verdict**: PASS (exit 4).
**Homes read**: G\agents\planner.md (the "Out of scope" list sentence is now in lines 11-13, not :13 alone); M\search-web-before-finalizing-plans.md:15 "Do not leave a 'left alone' list of related defects".
**Why**: the planner's list covers promises with no task, and it goes to the user to approve. The note bars quiet lists of related defects. Different items.

### L-F32-01
**Verdict**: REOPEN for the memory-vs-other half. The doc-vs-doc half passes (exit 3).
**Homes read**: G\model-postures.md:11 (build the branch, then review); H\docs\agents\derive-once-review.md:198 (build the branch yourself, then send a failing branch to a fresh executor); H\docs\agents\integrate.md:35,:41,:56 (join first, build the joined branch once); M\one-build-per-wave.md:9 and 13 (review each branch, build one combined branch, say the build count before launching and get a yes).
**Why**: pruning the note under "Note is wrong" drops rules no doc has. No doc says "say how many builds a wave needs before launching it" (searched H\docs\agents, model-postures.md, CLAUDE.md and the gates READMEs for "how many builds", "build count", "one build per wave": nothing). The note also records the user choosing "One build per wave", and no line in docs\agents\rulings\ holds that (grep found none). So the note may be right and model-postures.md and derive-once-review.md:198 wrong. The memory half depends on the doc-vs-doc pick and should go to the user with it. Only the "build first, then review" timing is the real split.

### L-F32-06
**Verdict**: PASS (exit 4).
**Homes read**: M\names-from-data-not-recall.md:15; G\skills\analyzing-sp-squeezes\SKILL.md:63; H\docs\agents\derive-once-review.md:76 "Every name comes from the repo, not memory".
**Why**: same direction. The note covers names in deliverables, the review page covers names a code reviewer cites.

### L-F32-10
**Verdict**: PASS for the claimed-ruling piece (exit 3, "Drop the claim"). The test_ skip piece stays with the user, as the brief says.
**Homes read**: M\relay-status-in-plain-english.md:15 "The user chose this on 2026-11-16 ("Claim, plus disagreement")"; H\docs\sessions\2026-11-16-e816e44f.md:24 (the card records the click: chose "Claim, plus disagreement"); H\docs\agents\rulings\agents-and-workflow.md:7 `## Confirmed` (only :26 "the status updates should be brief but relayed in plain english" is near it). Searched docs (all of docs\) and G\*.md, gates READMEs for "Claim, plus": found only the card, gates handback files and the memory note.
**Why**: no Confirmed line carries the choice, so "the user chose" cannot stand as a ruling. The card is a pointer, not a ruling. Dropping the claim keeps the relay rule as the session's own.

## Ids not reached
None.
