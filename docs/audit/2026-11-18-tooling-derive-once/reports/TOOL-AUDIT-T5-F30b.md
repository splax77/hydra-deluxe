# TOOL-AUDIT-T5-F30b (re-send of F30)

Repo read: main checkout C:\Users\Patrick\Downloads\Hydra\hydra-test. Codes: AM docs/agents/audit-method.md, OM owner-map.md, PD public-doc-style.md, CH clone-hero-mcp.md, UT ui-testing.md, BP brief-preamble.md. G = C:\Users\Patrick\.claude\gates. M = the memory dir. "The library flag" below means the hook's whole-library command flag (spelled out in percall.py:14).

## Ids taken (from F30's "Rows not reached in depth")
I6-138, I6-139, I6-140, I6-145, I6-157 (beyond OM:33 and rule 15), I6-161 (beyond reviewer.md:17), I6-254, I7-074 and I7-089 (owner code), I7-041 (preset), and the gates tests F30 did not open (test_review_rules, test_percall_rewrite, test_progress, test_review_dispatch, test_posture, replay.py).

## I6-138 main session greps exit-3 quotes
Question: who checks that an exit-3 PASS quote really sits in the cited file, and with what.
Homes: AM:87 only (the main session greps the quoted words; names docs/audit/2026-10-29-audit-of-audits/briefs/checkquotes.py as "one"; legend rule for shortened file names). checkquotes.py exists (briefs/checkquotes.py, with common.md and packets.py beside it). Searched docs/agents, CLAUDE.md, G (py, lib, README), agents, skills, M, tools, .claude/commands for "checkquotes", "exit 3", "legend": no other home. One home. K8 not shown (AM says "is one", not "owns").
Edges: main vs agent: AM:87 gives the grep to the main session; AM:83 gives the PASS (the quote names both sides) to the checker agent. Card writers: not applicable. Limit: no number. Test holding equal: none.

## I6-139 where checkers search for rulings
Homes: AM:89 (cards, not just the decision log; names card 2026-10-24-a124ccd9); AM:126 (the brief carries "where the user's words can be": decision log, question files, plans, session cards); AM:108 (D-records in docs/audit/2026-10-03-fix-decisions.md); CLAUDE.md:25 (look in cards for past decisions) and CLAUDE.md:41 (cite only `## Confirmed`); premise-map.md:45 (a ruling quote must sit under `## Confirmed`, in a card's quoted words under docs/sessions/, or a third place I did not read to the end). The T5 brief line 46 repeats the list (out of scope as a home).
Split line: AM:126 names four places (decision log, question files, plans, cards) and not docs/agents/rulings/; CLAUDE.md:41 names rulings/ (Confirmed only) and not the decision log; premise-map.md:45 names Confirmed lines and card quotes. Three lists of "where the user's words are" with different members. K1/K2-shape.
Test holding them equal: none (searched G\tests and tools tests).

## I6-140 question-writing rules (AM:93-102)
Homes:
- AM:95 "List what the user will see (agent rule 11)"; AM:96 "Quote the predicate, the code's actual test, not a paraphrase"; AM:98 "Name the options without leaning. Then give a recommendation and the strongest case against it."; AM:100 show every value; AM:102 never give a split a purpose nobody wrote down.
- CLAUDE.md:73 and agent-rules.md:17 (rule 11, "You will see" heading, real chart from testdata/input).
- M/questions-describe-the-real-rule.md:3 and :16 ("Quote the real predicate and give one example from each end of its range"); the note says rules 11 to 13 hold the list and "This memory holds what they lack". M/screen-words-not-code-names.md and both MEMORY.md lines.
- docs/agents/doc-review.md:61 (the reviewer weighs a decision question's recommendation).
Split line: AM:98 asks for a recommendation on every question; a memory note on splits (the never-keep-a-split-quietly family, line ending "Never recommend keeping on[e]") says never recommend keeping a split. Different cases, but AM:98 does not say so.
Kinds: K1 (predicate and "example from each end" in AM:96 and the memory note); K5 does not apply, no hook enforces any of it (searched G\*.py and lib for "recommend", "predicate", "You will see": none); K7 for the rules that live only in AM:93-102 and memory, with no home named by the memory note.
Confirmed rulings: docs/agents/rulings/questions-and-audits.md, `## Confirmed`: "2026-10-31 'Yes, require it', to: 'Should every rule question list each screen's before and after text?'" (backs rule 11); "2026-10-31 'Yes, ban them (Recommended)'" on opt-out lines (rule 12). Neither backs AM:96-102.
Tests: none.

## I6-145 what every audit brief carries (AM:118-127)
Homes by item:
- preamble, task id, session id, files, caps, status rule: AM:119-120; BP:13 (status line); memory note every-brief-carries-status-rule.
- four exits and "only the main session picks": AM:121 and AM:19-26, CLAUDE.md:33, docs/agents/test-rubric.md:98 and :189. K1.
- every split its own line: AM:122 and AM:60.
- the four claim words: AM:26 and AM:123 (both spell them out).
- "a search that finds nothing proves nothing": AM:124 and the preamble's rule that the scout prompt cites.
- library run: AM:125, BP:49, agent-rules.md:14 (F30 family B). Hook constants: G\review_rules.py:51 LIBRARY_ALLOW_TEXT = "allow library run"; G\percall.py:14 LIBRARY_FLAG. The phrase is hard-coded as a literal in G\tests\test_review_rules.py:839-845, test_progress.py:427,437,453-455,577,584, test_review_dispatch.py:309; the tests do not import the constant (K1 on the test side; a change to the constant turns them red).
- where the user's words can be: see I6-139.
- exact ids for a second wave: AM:127 only. Searched docs/agents, G\*.py, agents, M for "second wave", "exact ids", "re-send": no other home (posture.py RESEND_MINUTES is a different thing).

## I6-157 one owner per question
Homes: OM:33 ("A question can have only one owner. If two places answer it, the row names both and its Status is `split`. Merging them is the plan's first task (agent rule 15)."); CLAUDE.md:77 and agent-rules.md:21 (rule 15 "Call the one owner, merge several into one first"); M/derive-display-from-engine-truth.md:18 ("Before writing any calculation, find its one owner and call it. If none exists, propose one rather than copying."); M/counts-on-screen-must-agree.md:18 ("One owner per numbering."); G\agents\scout.md:11 ("Never propose a new owner"); OM "The stops" ("Nobody writes a new owner for it").
Split line: M/derive-display-from-engine-truth.md:18 says if no owner exists, propose one; OM's stops and scout.md:11 say a row with no owner stops the work and nobody writes one; the user decides.
Also: rule 15 says "A rule a plan makes up is never put to the user as a question", while OM sends a `none` row to the user. Read together they differ in who decides a missing answer (K3 candidate, ungraded).
Kinds: K1 (OM:33 restates rule 15), K3 (the memory note changes what to do with a missing owner).
Tests: G\tests\test_agent_files.py holds planner.md and reviewer.md phrases against the Premise map page only. Nothing checks the owner-map wording in planner.md:19, reviewer.md:17 or scout.md:11 against OM.

## I6-161 who checks the map
Homes: OM:47 ("The plan's reviewer opens each owner and reads its own words against the row's question"; "reads the plan's steps with the map closed and lists every question they answer"); G\agents\reviewer.md:17 (the same two steps, plus "A plan that lists ways it contradicts the user's words ... sign off BLOCKED and quote the words"; points at OM "Who checks it" and "The stops"); G\agents\planner.md:19 (build only from rows, stop on a missing row); G\agents\scout.md:11; agent-rules.md:25; CLAUDE.md:55; BP:111 (a result that contradicts the user's words stops you).
Who acts: reviewer.md:17 has the reviewer fix a missing-row finding "by naming the row's owner" (it edits the plan) but not resolve a contradiction (BLOCKED). Main session: no line of its own for this step. Card writers: skip the preamble; no owner-map line (not applicable). Limit: no number.
Kinds: K1 (the two checks appear in OM:47 and reviewer.md:17; reviewer.md points at OM and also states them, so it is a home); no hook checks that a plan has an owner map (searched G\*.py and lib for "owner map": none).
Tests: none holds reviewer.md against OM.

## I6-254 internal docs follow another page (PD:3)
PD:3: "Internal docs follow the user's "How to explain things" rules in `~/.claude/CLAUDE.md` instead; this page adds to those".
Other homes: BP:121-123 (states: lead with the plain version, one idea per sentence, gloss jargon, no bullet walls of file:line references; then "That file is the one home of those rules"); doc-review.md:81 (states "jargon is glossed on first use, sentences are short, and no wall of file:line bullets"; also says the global file "is the one home"); M/relay-status-in-plain-english.md; BP:13.
K1 and K7 shape: BP:123 and doc-review.md:81 each name the global file as the one home and each restate part of it, with different parts. Split line: the three lists (global CLAUDE.md, BP:123, doc-review.md:81) are not the same set. The global file also has "prefer short sentences", "when I ask why give the reason", "analogy before the definition", "cut throat-clearing" and the BAD/GOOD example. G\agents\card-writer.md has no plain-English line and reads no preamble. The global file is outside the repo, so a repo doc can only point at its absolute path. Test: none.

## I7-074 keyboard and mouse off during a run (owner code opened)
CH:25's claim "both devices must read back off else setup refuses" holds. tools/ch_mcp/kit_tools.py:533-543 `devices_problem`: "Only a read-back of false passes, so a reply with no read-back is refused too", returning "refused: ... after setup, not off". The device list is `KIT_DEVICES = ("keyboard", "mouse")` at kit_tools.py:129, with a comment quoting the user's 2026-10-30 answers. tools/ch_mcp/agent/kit.ts:199 has its own `type Device = "keyboard" | "mouse"`, switches both at :380-381, and writes the read-backs as keyboard_now / mouse_now (:425-427, :759-760); kit_tools.py builds the same keys as f"{d}_now". So the device list and the reply key names sit in two code files (K6 candidate). No test in tools/ch_mcp/tests mentions KIT_DEVICES or keyboard_now. K8 not shown: CH:25 says kit_tools.py and kit.ts own how it is made and checked, and they do. The Confirmed lines behind the 2026-10-30 answers I did not re-read; kit_tools.py:127-128 quotes them in a comment.

## I7-089 auto-player timing rules (owner code opened)
CH:49 says "path_index_refusal in tools/ch_mcp/game_tools.py owns that check": true. game_tools.py:451 `def path_index_refusal(doc, schedule_json, path_index)` refuses a non-integer (:457-458) and a schedule made from another path (:464-466); called at :1110 and :1191. "default 0": written at game_tools.py:1074 and :1141, server.py:385 and :414 (`path_index: int = 0`) and in CH:49. K1.
Field names: `kept_gems` and `declined` are read at game_tools.py:219-225. The `retimed` words are in tools/ch_probe/experiments/play_chart.py:96-98 (late, inside, plain), with the comment at :87-95 saying a read outside `read_range_ms` makes it `range`; also tools/ch_probe/experiments/overlay_logs.py:23,127 and tools/ch_mcp/agent/press_schedule.ts. CH:49 lists `retimed=inside`, `retimed=plain` and `retimed=late` and does not list `range`: a possible K2 (unchecked whether the live ch_play_chart path emits `range`). No single owner file for the word set. Test holding them equal: none found (searched tools/ch_mcp/tests and tools/ch_probe/tests by name; only schedule fixtures contain "retimed").

## I7-041 where live code exists (UT:205)
UT:205: "The live code sits in the test harness and under `HYDRA_UITEST_ATTACHED`, which the `ship` preset leaves out." Owner: CMakePresets.json:20-26 (preset "ship", displayName "Installer build (no attached GUI tests)", sets "HYDRA_UITEST_ATTACHED" OFF: confirmed); CMakeLists.txt:595-596 (option default ON, "off for the installer"). Other homes: UT:175 ("The installer builds with the `ship` preset"), docs/adr/0030-hydra-owns-every-live-command-rule.md:139 ("The `ship` preset turns that off"), CMakeLists.txt comments at :617 and :664. K1; all agree today. Test: none found by name (searched tests/ and tools tests for HYDRA_UITEST_ATTACHED).

## Gates tests F30 did not open (library-run family)
Opened: test_review_rules.py:839-845 (which typed texts count as the allow phrase), test_percall_rewrite.py:73 and :135 (hook output for a command carrying the library flag), test_progress.py:425-455 and :577-584 (hydra_batch with the library flag and typed allow text), test_review_dispatch.py:309 (more typed variants). They test hook behaviour. None compares hook text with any doc. test_posture.py: no library-run lines. replay.py: matched only by grep in F30; I saw no doc comparison. test_agent_files.py is the one test in G that holds agent files against a Hydra page (Premise map phrases only).
Split line (F30's S3, now seen in code): percall.py:14 refuses only a command containing the flag text; the docs define the rule as any run over the user's whole song library.

## Side notes
1. G\agents\card-writer.md reads no preamble and carries no plain-English, library-run or status line that I saw.
2. A way past the hook exists for running the three build-needing test programs (G README says no hook enforces it; percall covers only hydra_build_pass). Same as F30 side note 4.
3. When I first wrote this file with a shell heredoc, the library-run hook refused the command because the text held the flag. A hook that reads command text, not intent, is a gap of the false-positive kind.
