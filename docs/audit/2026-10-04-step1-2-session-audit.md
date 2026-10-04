# Where steps 1 and 2 spent their time (audit, 2026-10-04)

This audit covers the five sessions that planned, ran, reviewed and merged steps 1 and 2: cebf3006 and de7ddd82 (triage, plans, the review gate's design), ade9655b (step 1), bbf41dc4 (step 2) and a9a461da (the review gate going live, then step 1, step 2 and T10/T11 merged through it). Four Fable auditors each read one slice. Every number comes from the transcripts, through two scripts that split each agent's clock into states that don't overlap. The full slice reports are in `docs/audit/2026-10-04-session-audit/`.

## The short version

The agents used 52 hours between them. Half of that was the model, half was tools. The model wasn't slow because it thought hard: no transcript stores any extended thinking. It was slow because agents took many small steps (100 to 280 each), and every step re-sent 150k to 250k tokens of context. A step costs about 3.5 seconds under 100k tokens of context and about 7 seconds above 250k.

The wall-clock damage came from five things:
- **Review rounds.** Merging steps 1 and 2 and T10/T11 took 14 review rounds and 257 minutes. Two thirds of the 51 findings were test-side copies, and many of those copies came straight from code listings pasted into the plans.
- **Questions asked late.** About half of the 47 decisions were asked after the plan was approved. Six came up in the middle of a review chain, and each answer added code that needed another round.
- **Freezes.** The fleet stopped three times for 13 to 32 minutes. Twice it waited on a permission dialog nobody was there to answer. Once a reviewer parked and was forgotten across a context compaction.
- **Repeated whole test suites.** Agents ran whole suites again and again, sometimes because my own brief told them to. The rule against it was stated at 14:16 and still broken at 19:19, because the brief template never changed.
- **Cold builds.** Up to nine cold builds ran at once, each 150 to 365 seconds.

## Where the 52 agent-hours went

| State | Hours | Share |
|---|---|---|
| Model, before its first output each step | 18.2 | 35% |
| Model, writing long output (edits, files, reports) | 6.7 | 13% |
| Tools | 24.2 | 46% |
| Finished, parked until the coordinator messaged again | 2.2 | 4% |
| Harness records | 0.8 | 2% |

Inside the tool time, builds took 5.4 h, shell reads and searches 4.7 h, tests 4.2 h (0.9 h of it GUI tests), other shell commands 3.1 h and sleeping or polling 2.2 h. Comparison runs took 1.9 h, edits 1.7 h and file reads 1.2 h. These buckets add up to a little more than 24.2 h because parallel tool calls overlap.

| Session | Wall clock | Agent-hours | Main cost |
|---|---|---|---|
| Planning (cebf3006, de7ddd82) | 64 + 70 min of work | 4.8 | drafters with 200k-430k contexts writing 700-1,340-line drafts twice |
| Step 1 (ade9655b) | 5 h 42 min | 20.0 | a third went to the gate hooks; 203 agent-min fixing bugs outside the plan |
| Step 2 (bbf41dc4) | 3 h 13 min | 8.2 | 133 min of run-up before 59 min of execution; 107 min of builds |
| Merging (a9a461da) | 6 h 44 min | 19.0 | 14 review rounds, 257 min; 96 min of parked agents |

## What cost the most, and why

**Plans carried code, and the code carried the copies.** Both plans pasted full C++ listings. The step 1 plan's own rule said "pin literal ticks", yet its listings recompute them 15 times. The step 2 plan told every task to write its own test file, and listed a chart builder and MIDI writer per file. The reviews then found exactly those copies. Neither plan named the one scan file (`tests/test_single_owner.cpp`), had a shared test-helper task, or listed the numbers its tests would pin.

**Review rounds were long because the first review was everyone's first check.** Of the 51 findings, the auditors judged about 29 findable by a script: test helpers defined twice, a new number with no decision behind it, a test that recomputes the answer, a scan outside the scan file. Three whole rounds held nothing else. Six findings were new copies written by fix rounds. Seven were fixes only half done, often because the fixer ran out of its 150-call budget mid-round.

**Questions arrived one at a time.** The step 1 decisions were asked in five separate calls, five minutes apart. During the merges, each pending number (the D40 and D43 test limits, the D45 rulings, the depth of 40, the 120 windows) became its own round trip, and then its own review round.

**The fleet froze three times.** Each freeze was 13 to 32 minutes with nothing running:
- At 02:27 a `cat >>` append to the decisions file sat behind what was most likely a permission dialog. The step 1 auditor inferred it but couldn't prove it, because the transcript doesn't show the cause. Nothing ran for 31 minutes.
- In step 2, a heredoc append and a plain file read each got "ask" from a hook. Together they cost 13.5 minutes.
- The first step 1 reviewer started helper agents, ended its turn to wait for them, and was forgotten across a context compaction. That cost 20 to 27 minutes on the critical path.

**Whole suites, again and again.** Step 2's first task ran the full suite seven times chasing a timing test that only fails under load. Two gate agents re-ran every hook suite eight and five times, about 85 agent-minutes. The UTF-8 hook agent spent 40 of its 80 minutes waiting on four full suite runs. My briefs asked for those runs.

**Work outside the plan.** One step 1 review found old engine bugs. Ten agents then spent 203 agent-minutes fixing them, and by their own counts no library score changed. Separately, five agents spent 111 agent-minutes polishing a command-parsing design that the next review threw out.

**Smaller drags.**
- About 95 calls tried `sed`, `head` or `tail` and were denied.
- Nine of ten step 2 implementers had their first commit refused, because they passed the message on stdin and the trailer hook can't read it.
- Agents weren't told how to find their own agent ID.
- Edits to the 1,400-line `song.cpp` took 24 to 40 seconds each while nine builds ran. The cause is unproven.
- Reviewers rebuilt the old base to prove a test failed first. That re-proved something the implementer's own report already showed.

## Your corrections, and the rule each one needed

- **"Complete insanity" (serial queue).** Fixed by launching every task whose inputs exist at once (memory `parallel-by-default`). Step 2 complied with it.
- **"Running the suite 10 times", "80 minutes is insane", "reiterating a rule you ignored".** Text alone failed three times, because my brief template kept "full suite ONCE". It needs a hook that refuses an unfiltered suite run from an agent.
- **"Stop that workflow, it's already finished".** A script should exit on its last result.
- **"Why two fix agents for T10?"** Read the running script's control flow before dispatching anything.
- **"Fail loudly, freezing is unacceptable".** Already a rule. It still needs the two "ask" paths found here checked against the hooks.
- **"Why is a score dropping?"** Every question that moves a score gives the reason in game terms. The rule existed and one question skipped it.

## What we change before phases 3 to 5

Each lesson becomes something that enforces itself, not a reminder. Phase 0 of the plan builds them. The estimates come from the slice reports.

1. **A pre-review check script.** The implementer runs it before committing. The reviewer re-runs it, and the review must quote its output. It lists test helpers defined in two files, the recompute spellings, new numbers with no decision, and scans outside the scan file. On this data it saves 90 to 120 of the 257 review minutes.
2. **Plans without code listings.** Each task gets signatures, test names and pinned numbers, written out in its own short task file. Plans name the shared test fixtures and the scan file, and carry a test-limits table. That removes the copies the plans seeded, and cuts each agent's context by about 50k tokens, roughly 2 seconds a step.
3. **Every question asked before execution, in one sheet.** The pre-review script also lists any new number or behaviour with no decision, so the stragglers come to you in one batch before review starts, not one per round.
4. **A hook that refuses whole-suite runs from agents.** Only the main session's merge step may run them.
5. **No permission dialogs on the fleet's path.** The main session writes ledgers with Edit or Write, never `cat >>`. Writes to `docs/audit`, `docs/handoffs` and `.superpowers` are pre-allowed. The two hooks that answered "ask" get checked and fixed. The watchdog alerts when no agent has been alive for 5 minutes while tasks are still open.
6. **Reviewers in the foreground, without helpers.** Reviewers run on Sonnet, start no helper agents, get the recurring kinds of finding up front, and keep each finding to one paragraph. They reuse the implementer's failing-first run instead of rebuilding the old base.
7. **Fix rounds sweep by kind.** A fix round sweeps the whole branch by kind of finding, not just the listed spots, with a grep proof per finding. Production findings and test findings go to two parallel agents, each starting fresh, committing per finding and capped at 100 calls.
8. **Bugs outside the plan get filed, not fixed.** A review finding outside the plan goes to the ledger with a library count. It is fixed only on your "fix now".
9. **Integrators check the base.** A branch built on an older main is checked against the owners main has gained since, before it merges.
10. **Workflow scripts tightened.** Later tasks wait on an earlier task's commit, not its review. A script exits on its last result.
11. **Fewer cold builds at once.** Cold builds are staggered to at most three at a time. Seeding a new worktree from an existing build gets measured, and stays only if it pays.
12. **Timing tests skipped under agent load.** The two timing tests that fail under load (`PathsTabCache`, `batch-strip-drift`) skip when an agent runs them. The main session's full suite still runs them.
13. **One shared brief preamble.** It covers how to find your agent ID, the exact commit command with trailers, Read with offset and limit instead of `sed`, relevant tests only, committing before 110 calls, running helpers in the foreground, and stopping with the exact error when blocked.
14. **A quieter monitor.** Status batches arrive every 10 minutes with only the lines that changed. That keeps the main session from compacting mid-merge.
