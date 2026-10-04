# Audit: step 2 execution session (bbf41dc4, 2026-10-04 01:16 to 04:30 UTC)

Read-only audit. All numbers come from `metrics\agents.csv`, `metrics\commands.csv`, `metrics\gaps.csv`, the coordinator's `timeline.py`, the five workflow scripts under `bbf41dc4...\workflows\scripts\`, their journals, and the main transcript. Agent ids are the first 8 characters of the transcript name. Times are UTC.

## The short version

The session took 193 minutes. Only 59 of them were the actual execution of the plan (03:29 to 04:28). The 133 minutes before that were spent measuring the rules, asking the user six decision rounds, writing a 4,451-line plan with six agents, reviewing it, fixing it, and twice sitting on a permission prompt that nobody could answer quickly.

The workflow structure itself did not wait on the slowest task. Reviews started the moment each implementer finished, and seven of the ten lanes were done and reviewed by 04:06. The whole 59-minute critical path was one chain: baseline and T0 (16 min) → T0 review (6 min) → T1 (11 min) → T1 review (7 min) → integrate (8 min) → final check (9 min). Three things inside that chain were avoidable and would have cut it to roughly 40 minutes: T0 ran the full test suite seven times chasing a flake, T1/T2 waited for T0's review instead of T0's commit, and every lane started with a cold build while eight other cold builds ran at once.

The biggest single bucket is still builds: 107 minutes of build time across 117 builds, 96 of those minutes inside the execute workflow. Model time (2.3 h waiting for the model plus 1.25 h of it writing) is the second bucket, and 44% of it belongs to the plan-writing agents, which carried 270K to 390K tokens of context and wrote 100K+ characters each.

## 1. Where the time went

### Session timeline

| When | What | Minutes | On the critical path? |
|---|---|---|---|
| 01:16–01:19 | Read handoff, load the workflow skill | 3 | yes |
| 01:19–01:52 | `wf_f56329a2` decision prep: 4 prototype/measure briefs in parallel (a63a1969 disco 20 min, a15217b2 kick2x 28, a1fc2e9c phrasefill 23, af7043d2 small 16) then 4 verifiers (3–10 min each) | 33 | yes |
| 01:49–02:06 | `wf_c132502d` Fill B measure (ada18351, 11 min) + verify (a08d0ac2, 5 min) | 17 | no, overlapped |
| 01:49–01:58 | Six AskUserQuestion rounds (D19–D29) | 9 | yes |
| 02:00–02:49 | `wf_c7630c2f` plan draft: 4 drafters in parallel (13–20 min), then assemble (a9ae80dc, 16 min), then review (a8d9cab7, 12 min) | 49 | yes |
| 02:06–02:15 | Fill B question to the user | 9 | no, overlapped |
| 02:49–02:58 | Wording question (D31) to the user | 9 | yes |
| 02:58–03:07 | **Permission prompt** on a `cat >> decisions.md` heredoc (Bash). A PreToolUse hook answered "ask"; the plan-fix Workflow in the same turn could not launch until the user clicked | 8 | yes |
| 03:07–03:22 | `wf_44f324c2` plan fix (ab88c130, 12 min, 79 Edits) + recheck (aff1cc82, 2 min) | 15 | yes |
| 03:23–03:28 | **Permission prompt** on a plain `Read` of the plan (offset 104, limit 60). The PreToolUse:Read hook returned a permission decision; the result arrived 5.5 min later | 5.5 | yes |
| 03:29–04:28 | `wf_8552b418` execute: baseline, 10 implementers, 10 reviewers, integrate, final check | 59 | yes |
| 04:28–04:30 | Handoff written, Stop hook asked for a prose rewrite | 2 | yes |

Main session: 83 tool calls, 7 minutes of its own model time. 29 of its minutes were AskUserQuestion (10 questions). Nothing was parked: the Workflow harness chained every phase on its own, so the "finished and waiting for the coordinator" bucket is 0.3% (about 1.5 min) for this slice.

### Agent time split (41 workflow agents, 8.2 h of agent wall)

From the coordinator's `timeline.py` on this session: tools 4.6 h (56%), model waiting before first output 2.2 h (27%), model writing 1.2 h (15%), other 0.1 h, parked 0.0 h.

Tool time by category (commands.csv, whole slice): build 107 min (117 calls), read/search shell 45 min (638), run/compare 45 min (213), Edit 35 min (442), test 28 min (60), Read 19 min (428), shell-other 18 min, uitest 8 min, status appends 7 min (101 calls), Grep 6 min.

Inside the execute workflow alone: build 96 min, Edit 29 min, test 22 min, run/compare 21 min, read/search 19 min, uitest 8 min.

### The five most expensive agents

1. **impl:T6 a412534d, 26.4 min wall, 16.5 min building (18 builds).** T6 was six small parser owners in six sub-commits (6a–6f). The agent appended one test case at a time and rebuilt after each (18 builds of 9–205 s), with the first cold build at 205 s under contention. It then built again after each commit for the status check. Only one test run; the time is all compile.
2. **brief:kick2x a15217b2, 27.5 min, 142 calls.** The prototype-and-measure brief: 7 builds (5.8 min, two prototypes), 4 test runs (3.4 min, three of them full suites), 18 library/corpus runs (3.3 min), 24 edits. It was also refused four times by hooks (sed, commit trailer, two patch-script edits) and twice by the status gate.
3. **brief:phrasefill a1fc2e9c, 23.3 min.** 28 run/compare calls (6.2 min) scanning the library and corpus for both prototypes, 27 edits; one 232 s scan at 01:32.
4. **impl:T5 aa0d6c2a, 21.4 min.** Cold build 335 s. Then it chased the `batch-strip-drift` GUI flake: 8 uitest runs (3.8 min) and 3 full suites (3.2 min) while the box was at 87–100% CPU, as its own report says.
5. **brief:disco a63a1969, 20.4 min.** 16 run/compare calls (4.6 min) including a 149 s library pass, 5 builds (2.4 min), one full suite.

The runners-up are the plan drafters: draft:D a6de7755 (19.8 min, 18 min of it model time), draft:C ae9289e2 (16.6, 15 model), assemble a9ae80dc (15.7, 14.6 model), draft:A a38895e3 (15.1, 13.7 model). These four did almost nothing with tools; they read and wrote.

### Why so much model time

Model time is 2.2 h waiting plus 1.25 h writing (3.45 h, 42% of agent wall). Per agent (my per-agent split, `audit\s2_timeline.py`):

- **Plan writing is 44% of it.** draft A–D, assemble, review, fix-plan and recheck together: 63 min of waiting and 31 min of writing, out of 110 min of their wall time. Their contexts reached 272K–386K tokens (the assembler peaked at 386K) because the brief told each one to read the rulings, the decisions file, five measured briefs with verifier corrections, the relevant audit findings, the step-1 plan as a template, and code at 1abd188. Each drafter then wrote 85K–116K characters of tool input (Write and Edit bodies); the assembler wrote 129K. The longest single uninterrupted generation was 4.4 min (draft:D and draft:C). The plan they produced is 340 KB, 4,451 lines.
- **The measurement briefs are 21%.** Four briefs plus Fill B: 31 min waiting, 15 min writing. Contexts 184K–307K; each returned a 15–30 KB structured brief (StructuredOutput) at the end.
- **Implementers and reviewers are 29%** across 23 agents, 1–3 min of waiting each. Contexts 105K–201K because every one of them read the plan's header and its task: the plan was Read 92 times by 26 agents.
- Thinking blocks are redacted in the transcripts (0 thinking characters recorded), so thinking cannot be separated from the pre-output wait; the per-step wait for drafters averaged 7–8 s per tool call, versus 2–3 s for implementers.

Parked time: zero waiting on the coordinator. Waiting on the user: 29 min of questions in the main session, of which about 18 min blocked the critical path (the six decision rounds, 9 min; the wording round, 9 min), plus the two permission prompts (13.5 min) which are also user waits.

## 2. Which time was avoidable

**Cold builds, eight at once (≈32 agent-min, ≈5 min of critical path).** Each of the eight first-wave lanes made a fresh worktree and compiled from nothing while the others did the same: 7–9 builds were in flight every minute from 03:30 to 03:37. First builds: T8 363 s, T7 355 s, T5 335 s, T4 246 s, T0 219 s, T6 205 s, T3 200 s, T9 153 s. Step 1's session (ade9655b) ran 315 builds and only 5 took over 100 s (average 122 s), because its waves were smaller and its trees warmer. The load also caused the timing flakes below.

**Full suites chasing load-induced flakes (≈16 agent-min, ≈7 min of critical path).** T0 (ae5db67d) ran the full `hydra_tests` suite 7 times between 03:36 and 03:44 (7.8 min) because the first run failed `PathsTabCache: 600 cached frames cost far less than 600 rebuilds`, a timing test, under 100% CPU. T5 ran 3 full suites and 8 GUI-suite runs for `batch-strip-drift`. The T7 reviewer (ae6f0ad8) ran the GUI test 5 times, the final check 3 times. The implementer brief literally said "Do run the full hydra_tests suite", so the agents obeyed it. The no-repeat-suite-runs rule was written later the same day (19:19 UTC) and did not exist at 03:30.

**T1/T2 gated on T0's review instead of T0's commit (6.2 min of critical path).** T0 committed at 03:45:40; its review ended 03:51:25; T1 and T2 started 03:51:54. The script's own comment says "waiting for T0's review keeps them off a red base". T0's review found only a non-blocking derive-once nit, so the wait bought nothing.

**Reviewers rebuilding at base to prove "tests fail first" (≈9 agent-min, 0 critical).** review:T4 (acfbaf2d) checked out 1abd188 and the mid commit inside the implementer's worktree and rebuilt the whole tree twice (151 s, 118 s, 81 s). review:T3 (adee9c06) copied the tree to `Temp\claude\t3ff` and built it cold (207 s). Both were doing step 3 of the review prompt ("tests would have failed before the change"). The implementer's own red run is already in its transcript and report.

**Edit hooks slow on song.cpp under load (≈13 agent-min, ≈5 min critical).** impl:T1 (a421c0dc) made 16 Edits to `src/parse/song.cpp` with a median of 37 s each (7.2 min); impl:T3 15 Edits at 24 s median (6.7 min). Twelve of T1's edits were issued in one burst at 03:57:25–03:57:40 and every one took 36–40 s, which points at the PreToolUse:Edit hook chain (the deletion guard does git work) running while 7–9 builds hogged the CPU. The same Edit tool on the plan markdown ran at 0.7 s (fix-plan, 79 edits, 1.1 min). Cause not proven; worth timing the Edit hooks on a 1,400-line tracked file.

**Hook denials and retries (75 in the slice, ≈12 agent-min).** 35 were "sed/head/tail is blocked, use Read" (almost every agent's first instinct for reading a file range); 13 "status line overdue" (three of those on T3 cost 50 s each because the denial itself ran through the slow Edit hook chain); 11 "commit is missing the attribution trailer" — nine of the ten implementers had their first commit refused, because they passed the message via `git commit -F -` or a heredoc, which the hook cannot read, even though the RULES text named the trailers. T7's agent then spent calls reading the hook source to understand it. 6 "source edits go through Edit, not a patch script", 4 "high-risk Remove-Item" (reviewers cleaning their own scratch), 3 Windows-path-in-Bash, 2 deletion-shrink, 1 git reset.

**Two permission prompts on the main session (13.5 min of critical path).** At 02:58:55 a `cat >>` heredoc append to the decisions file got "ask" from a PreToolUse hook and waited 8 min; the plan-fix Workflow issued in the same turn launched only at 03:07:02. At 03:23:03 a plain `Read` of the plan got a permission decision from the PreToolUse:Read hook and returned at 03:28:36. The fail-loudly-never-freeze rule was recorded later that day (16:27 UTC); both stalls predate it. Whether the Read hook can still return "ask" should be checked.

**Serial phases inside the plan-draft workflow (≈28 min of critical path).** After the four drafts (done by 02:20), the assembler ran 16 min and the reviewer 12 min, strictly in sequence. Then the fix (12 min) and recheck (2 min) ran as a second workflow, separated by the user round and the prompt stall. From "drafts done" to "plan ready" was 69 minutes, 28 of them agents working alone on a text file.

**Cold re-reading.** The plan was Read 92 times by 26 agents; `song.cpp` at s1-t3 25 times by 5 agents; the derivation audit 20 times by 5 agents; the step-1 plan 12 times by 5. Each implementer re-derived the plan's header into its own 110K–200K context. Nobody hit the call budget (max 166 calls, fix-plan).

## 3. Review rounds

**Execute workflow, ten task reviews, zero fix rounds.** Every reviewer passed its task in round 1 (journal `wf_8552b418-cd1`). Review wall minutes: T0 5.5, T1 7.2, T2 4.4, T3 11.7, T4 15.5, T5 8.1, T6 9.9, T7 9.2, T8 8.4, T9 8.1 — 88 agent-minutes, of which only T0's and T1's (12.7 min) sat on the critical path. About 46 of the 88 minutes were builds, test runs and corpus comparisons (the "re-run every done-when item" instruction); T4's reviewer alone spent 12 min on them.

What they found, all non-blocking: T0 stores the 2x-kick pitch as its own column when it is always kick − 1 (a91185209, derive-once; the plan's own Step 4 asked for the column, so the fixer's "finding 7" repair did not reach the code); T8 cites an untracked evidence file (a8687b8c); T5 leaves a stale test comment (abae1a7c); T4's plan check command can never print nothing (acfbaf2d); T1's plan says the test fails "at Hard" when it fails at all three (aa247205); T2's verify filter misses a file (a6e87e65); T6 and T7 saw the GUI flake. Four of the seven are plan-text errors, not code; a plan reviewer already ran. The derive-once one would have been prevented by the integration rulings naming "2x pitch = kick − 1, never stored" explicitly.

**Plan review round (wf_c7630c2f review → wf_44f324c2 fix + recheck): 13 findings, 44 minutes end to end** (12 review + 9 user + 8 prompt stall + 15 fix/recheck). Two medium findings were about who bumps the results stamp if T10 lands late, and a second bump departing from D23; one medium listed visible text (T7 filter label, T4 UserGuide line) not covered by the user's Q1–Q5; one wrong line range in T2; one unverifiable "Clone Hero has no such constant" claim; one broken `git grep` in T8's done-when; two derive-once duplicates across parts (T2's pitch vs T0's table, T7's C++ tally vs the page JS); two tests that would not fail first (T6 6d, T5); an acceptance-criteria mismatch (T3); wrong merge notes; bullet-wall prose in Files lists. The recheck found one stale line left (plan line 43).

Could the drafter brief have prevented them? The brief already said "tests first (failing test, then the change)" and "no bullet walls", and those still slipped, so text alone did not hold. The cross-part ones (stamp ownership, duplicate facts across parts A/C/D, merge notes) are the price of four drafters writing in parallel with no shared view; only the whole-plan reader sees them. The mechanical ones (line range, broken grep, acceptance mismatch) could have been caught by a script, not a reviewer: a check that every quoted `lines a-b` block matches `git show 1abd188:path`, and that every command in a "done when" list parses.

## 4. The user's corrections

Two in this session.

1. **01:16, opening message:** "all work must be run in parallel as workflows. do not try to bullshit me again with 50 billion sequential agents." Carried over from the step-1 session's serial-queue incident (parallel-by-default memory, 2026-10-03). The session complied; every phase ran as a Workflow.
2. **01:56, Dynamics-tag question:** the question said "Programmed for Battle's Expert optimal drops 620,730 → 607,580" with no reason. The user: "why is a score dropping? you need to explain before changing anything". The assistant re-asked at 01:56:40 with the reason (the game cannot pay accents on a loosely-spelled tag). Cost: 2.5 min and a second round. The rule that prevents it already exists in memory (no-silent-functionality-changes: every score move says WHY in game terms before asking) and was not applied to that one question; the five other questions in the same batch did carry reasons.

One Stop-hook correction at 04:30 (reply was 12 of 15 lines bare bullets) cost one rewrite.

## 5. Comparison with step 1's plain Agent dispatch

Step 1 (session ade9655b, 342 min, 75 agents) ran waves of 3–6 implementers with hand-written Agent calls and 19 SendMessages; its main session spent 36 min inside Agent calls and 53 min in AskUserQuestion, and its agents logged 65 min of wait/poll. Step 2's main session spent 8 min launching five Workflows and 0 min messaging agents; its agents logged 0 min of wait/poll and were never parked.

Per implementer the two look alike: step 1 "Step1 Task" agents averaged about 12 min wall and 2.5 min of builds; step 2 implementers averaged 15.2 min (median 13.5) and 7.2 min of builds. The extra is the cold-build burst, not the workflow. The workflow's phase structure cost time in exactly two places: T1/T2 gated on T0's review (6 min) and the plan-draft workflow's serial assemble → review → fix chain (28 min of agents writing alone, plus the user and prompt waits between them). The Review and Integrate phases did not wait on the slowest independent lane; T6, the slowest, finished review at 04:06, four minutes before integrate started anyway.

## 6. Concrete speedups for steps 3–5, with estimates

1. **Warm build directories for every worktree.** Have the worktree-creation step copy (robocopy) a pre-built `build-cpp` from the base commit into the new worktree, or point MSVC at a shared object cache. Saves the 150–365 s cold build per lane (≈32 agent-min for 8 lanes) and, because the box stops running 7–9 compilers at once, most of the flake-driven reruns and the slow Edit hooks. Critical path: about 5 min on the first lane plus whatever flakes it avoids. Enforce in the workflow script's `implPrompt` (give the exact command) or better in `build_cpp.ps1` (`-SeedFrom <path>`).

2. **"Only the tests you touched; one full run at the end, no reruns."** The implementer brief said the opposite. Put the no-repeat-suite-runs wording in the RULES string of every workflow script and add a hook that denies a second `hydra_tests.exe` run without a `-tc`/`-sf` filter in the same agent. Saves T0's 7 min on the critical path and ≈16 agent-min.

3. **Skip timing tests under load.** `PathsTabCache` and `batch-strip-drift` fail when the CPU is pegged; both are timing assertions. Mark them with a doctest skip when an env var (`HYDRA_AGENT=1`) is set, or run them only in the final check. Removes the trigger for item 2's reruns (≈8 agent-min of GUI loops in T5 alone).

4. **Gate dependents on the commit, not the review.** In `runTask`, resolve a `committed` promise when the implementer returns and start T1/T2 on it; the review runs in parallel. Saves 6 min of critical path. One line in the script.

5. **Reviewers reuse the implementer's red run.** Drop "tests would have failed before the change" from the reviewer's rebuild list; instead require the implementer to paste the failing run's last 10 lines into its report and let the reviewer read them. Saves ≈9 agent-min (T3, T4) and the cold tree copies.

6. **Fix the permission-prompt path.** Both stalls (13.5 min) came from hooks returning "ask" on a heredoc append and on a Read. Check which hook answers "ask" for Read and for `cat >>`, and make them deny-with-reason or allow; the fail-loudly rule now says so, but it was written after these stalls and should be verified against these two exact calls.

7. **Give agents the working commit command.** Nine of ten implementers lost a round to the trailer hook because they used stdin. Put `git commit -m "<subject>" -m "Task: T<n>" -m "Agent: <id>" -m "Session: <sid>" -m "Co-Authored-By: ..."` verbatim in RULES, and add "never sed/head/tail/awk a file; Read with offset and limit" (35 denials). Saves ≈10 agent-min and the hook-reading detours.

8. **Shrink what each lane reads.** 26 agents read the 340 KB plan. Have the assembler also write one `tasks/T<n>.md` per task (header essentials + that task), and point implementers and reviewers at that file only. Cuts per-lane context from 110K–200K toward 60K; model wait per lane drops roughly in proportion (≈1 min per lane, ≈25 agent-min) and the plan Reads go from 92 to about 30.

9. **Mechanical plan checks before the human-style review.** A script that verifies every quoted `lines a-b` block against `git show <base>:<path>`, runs every "done when" command in dry-run form, and greps the tasks.json acceptance text against the plan's "done when" would have caught 4 of the 13 plan findings (T2 range, T8 grep, T3 mismatch, merge-notes lines) in seconds. Run it in the Assemble phase before dispatching the reviewer; saves one fix item in three and shortens the fix agent (12 min → ≈7).

10. **Fold the plan fix into the review workflow.** The draft workflow ended at the review; a second workflow (fix + recheck) needed a user round and a new launch, and the launch sat behind the prompt stall. Let the draft script run review → fix → recheck itself and only surface the wording questions to the user at the end. Removes the launch gap (8 min stall + 1 min) from the path; the user's 9-minute answer can overlap the fix.

Together, items 1–5 take the execute workflow's critical path from 59 min to roughly 40, and items 6–10 take about 25 min off the 133-minute run-up. The parallel-workflow structure itself was sound; it was the cold start and the full-suite habit that cost the hour.
