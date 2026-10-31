# Hook audit, 2026-10-10

## The short answer

The hooks mostly enforce the right rules, but the setup has grown past what it needs. It is about 45 live PowerShell scripts and 11,000 lines. They start 2 to 8 PowerShell processes on every tool call, and about one shell call in 33 gets denied. Four things are actually broken. The Stop gate has never worked. The dispatchers let a call through when a gate crashes. The watchdog kills any ffmpeg on the machine that runs over 15 minutes. The progress gate thinks every agent is an implementer, so reviewers never get their D107 wording. A second group of hooks has been doing nothing since the main session moved from Fable to Opus around 2026-10-02. The rest is consolidation: one process per event instead of several, one shell parse per call instead of three or more, one shared library for derive-once and doc review, and real version control in place of 195 `.bak` files.

The git-side review gate is the strongest part of the setup. Every refusal it made in the last 14 days was justified, and nothing routed around it.

## How this was measured

Eleven read-only Sonnet scouts did the work. Two read the web: Anthropic's own docs and changelog, and write-ups from people running large hook setups. Seven each audited one family of hooks. One scanned the last 14 days of transcripts for denials. One checked CLAUDE.md and the agent briefs against the hook code. Their full notes were written to the session's scratch folder (files `A.md` to `I.md`, `W1.md` and `W2.md`), which was not kept. I re-checked the claims marked "checked" below against the code myself.

Here are the base numbers. Each one names where it comes from.

| What | Number | Source |
|---|---|---|
| Live hook scripts, lines of code | about 45 scripts, 10,966 lines | `wc -l` over `~\.claude\hooks\*.ps1`, `lib\*.ps1`, `*.sh` |
| Tool calls in the last 14 days | 92,021 | transcript scan (scout G), 1,666 transcripts |
| PreToolUse denials in that time | 2,733 (2.97% of calls) | same scan |
| "Status line due" nags | 4,692 | same scan |
| Bare PowerShell start | 245 ms median (pwsh 7: 270 ms) | five timed runs (scout H) |
| Shell dispatcher, one call | about 1.0 s | run by hand with a made-up payload (scout H) |
| `.bak` and `.retired` files | 195 files, about 2.6 MB, 74% of the hooks folder | directory listing (scout H) |
| `hooks\state` | about 6,570 files, 39 MB, oldest 2026-08-29, nothing prunes it | directory listing (scouts E, H) |

Claude Code runs all the hooks that match one call in parallel (Anthropic's hooks reference, scout W1). So the wait you feel is roughly the slowest hook, not the sum. For a shell call that's the dispatcher, at about a second.

## Broken: fix these first

**The Stop gate has never blocked anything (checked).** `stop_orchestrator_gate.ps1` is meant to stop the orchestrator ending a turn while agents still run. On a Stop event it prints a permission-style `deny` (line 27). Stop hooks don't read that shape; they want `decision: block` with a reason. It then exits with code 2 and writes nothing to stderr, so Claude gets a block with an empty reason. The scan found 234 recorded blocks, and in 226 of them the agent did not carry on. Meanwhile the gate ran 1,683 times in 8 days, for 83 minutes in total, and its slowest run took 124 seconds. The recommendation is to retire it. If you still want the rule, rewrite it in a few dozen lines with the right output shape and a retry cap.

**The dispatchers let a call through when a gate breaks (checked).** This matches the other session's handoff, `docs/handoffs/2026-10-10-dispatcher-fail-closed-handoff.md` (removed; `git show 444631e6:docs/handoffs/2026-10-10-dispatcher-fail-closed-handoff.md`). Both dispatchers skip a gate whose file is missing (`dispatch_bash_ps.ps1:68`, `dispatch_star.ps1:45`). They also treat a gate that throws as "no opinion" (lines 75 and 49). `lib\verdict_merge.ps1:38` drops any output it can't parse. If the dispatcher itself crashes, Claude Code treats the non-zero exit as a non-blocking error, and the call goes ahead. Anthropic's docs say only exit 2 blocks. You already approved the rule: a gate that prints nothing counts as broken and is denied. The built-in `onFailure: "block"` setting would cover a dispatcher crash, but the docs say it arrived in v2.1.295. The `claude` on PATH here is v2.1.275, so the fix can't rely on it yet.

**The watchdog can kill your video encodes (checked).** `agent_watchdog.ps1:162-175` kills every `ffmpeg.exe` on the machine older than 15 minutes. It doesn't check who started it, so it would kill a long encode you started yourself. The watchdog has other problems too. Of its 65 stall alerts, 46 were false. It only reads the last three records of a transcript, and hook messages now land after an agent's hand-back. It never killed anything else. Nothing writes its `.stop` file, so each copy lives up to 8 hours after its session ends; four were running at about 100 MB each during the audit. Its alerts only reach the orchestrator on the next tool call anyway. The recommendation is to replace the resident process with a quick check inside `stall_inject.ps1`, and to drop the ffmpeg kill or limit it to processes an agent started.

**Reviewers never get their D107 wording (checked).** `agent_progress_gate.ps1:170` works out the agent's role from `transcript_path`. That field holds the parent's transcript, not the agent's, so the role always comes out as implementer. All 1,962 agent state files say `implementer`, even though 220 derive-once reviews were recorded. So the "you are the reviewer: stop fixing, sign off, list the rest in Notes" text has never been shown.

## Dormant since the move to Opus

Several hooks return early unless the main model is Fable. Every main session since about 2026-10-02 has run on Opus (scout F checked 72 Fable sessions, the last on 10-02; scout I checked six recent ones). That means four things are switched off. The first is `fable_delegate_gate.ps1`, the inline-edit budget, which also holds the only guard against ffmpeg or yt-dlp writing over source media outside scratch. The second is `executor_dispatch_brief_gate.ps1`, the real rule-2 check. The third is the orchestrator section of `model-postures.md`. The fourth is the agent-dispatch half of the fanout hooks.

The per-prompt text is now wrong for the main session. Every prompt this session got arrived with the executor posture: "You are executing one task that someone else scoped... Run your Sonnet workers in the foreground". That tells the orchestrator it is an executor. It also contradicts the preamble, which tells executors they have no helper agents. The cost is about 480 tokens a prompt.

This needs your call, because it depends on whether you plan to go back to Fable. If Opus stays, retire the edit budget, move the media guard into the shell dispatcher, and rewrite the posture text so the main session gets orchestrator guidance. If Fable might come back, keep the gates but fix the posture now.

## Noise: hooks that cost more than they catch

These are ranked by denials in the last 14 days (scout G's scan). The false-positive rates come from about 20 sampled denials, so treat them as rough.

**The progress gate's status-line rule** made 680 denials and 4,692 nags. In 551 of 678 denials, the last status line was 0 or 1 minutes old. The agent had simply made 15 fast reads in a row. It even blocked agents handing back their final report, 12 times. The time half of the rule works; the call-count half mostly creates friction. The recommendation is to count minutes only, or raise the call count, and never block the hand-back.

**The shell-read gate** made about 1,120 denials across its old and new wording, and it still fires 40 to 85 times a day. One denial throws away the whole compound command, including any unrelated git calls in it. The failure it prevents, "file has not been read yet" on a later Edit, appears about 15 times in all the logs. It appeared zero times on the two days before the gate existed. Scout A suggests retiring it or cutting it from 486 lines to about 40. You asked for its exact "Instead:" lines, so this is your call.

**The risky-command gate** is about half false positives. It matches words, not commands. It blocked "DROP TABLE" and "Remove-Item -Recurse -Force" when they appeared inside status-line text and plan documents. The fix belongs in the shared parser: strip quoted text and heredoc bodies before matching. The DROP TABLE and TRUNCATE patterns can go.

**The suite-run gate** made 154 denials, about a quarter of them false. A status line that merely mentioned `hydra_tests` was refused. Its `test_h*` wildcard rule also blocked a scout's harmless read of the test files. It has its own regex scanner instead of the shared parser.

**The attribution gate's patch-script rule**: 37 of its 46 patch-script denials hit helper files in scratch, not source.

**The fanout hooks** nag the model to delegate once it touches five files in a turn. The rule isn't in CLAUDE.md, it pulls against your lean-fan-out memory, and it fired on this audit while I was checking the scouts' work. `fanout_track` runs before every Read, Grep, Glob, Edit and Write. The recommendation is to retire all three.

**The rerun gate** counts launches, not changes, so a normal edit, build and test loop trips it. Unrelated commands that start with `$wt =` share one counter. No written rule asks for it. The recommendation is to retire it.

## What's working

The git-side gate is the single decider for commits and moves on `main`. That's `.git/hooks/reference-transaction` and `pre-push`, which run `git_ref_gate.sh` and `git_ref_gate.ps1`. The Claude-side derive-once hooks only record reviews and waivers, because git can't see who is calling. It made 37 refusals in 14 days, and all were justified. Keep this design.

The shell dispatcher pattern works where it's used: one process runs all 13 shell gates. `lib\shell_scan.ps1` is a 1,058-line hand-written shell parser, and no false denial traced back to it. `no_background_in_subagent_gate` made 13 denials, all justified. The deletion shrink guard is now quiet: 93 denials, only 4 since 10-05. The doc-review gate does what it says. All 53 reviews came back CLEAN, no hook asks instead of denying, and every error path denies with an "Instead:" line. The bullet-check Stop hook in `settings.local.json` works and is loop-safe.

## Consolidate

**One process per event.** Shell calls already have a dispatcher. Edit and Write start 4 processes, an Agent call starts 8, and every prompt you send starts 6. Give each event one dispatcher, as the shell one does. Two hooks, `fanout_track` and `fable_delegate_gate`, currently run twice on every shell call: once inside the dispatcher and again under their own registration.

**Parse once.** Inside the shell dispatcher, 13 gates each re-read the input JSON, and about 9 of them reload the 49 KB parser and re-scan the command. That is most of why the dispatcher takes a second against a quarter-second to start. Parse once in the dispatcher and hand the result to every gate. `cmd_writes_lib.ps1` and `suite_run_gate.ps1` have their own scanners; move both onto the shared one.

**One review library.** `lib\doc_review_rules.ps1` (995 lines) re-implements the transcript reader, the waiver phrase check, the `Test-Waived` loop, the repo check and the deny formatting from `lib\derive_once_rules.ps1`. The two waiver hooks are the same code with one word changed. Both load about 85 KB of libraries on every prompt, before they even check whether the prompt is a waiver. Merge the shared parts and keep the keys separate. About 330 lines of doc-review code are already dead: question reviews and edit reviews, which this morning's scope cut made unreachable. Scout D thinks the whole gate fits in 700 to 900 lines, down from 2,025.

**Split the attribution gate.** `git_attribution_gate.ps1` (594 lines) does four jobs under one misleading name. It stops tampering with the git gate (about 250 lines). It stops subagents rewriting history (about 100). It checks the `Agent:` trailer (about 120). It blocks patch scripts (about 15). It also reads 40 lines of transcript on every shell call for a rule that only matters to `git worktree remove`.

**Native features instead of hooks.** These replacements come from Anthropic's docs (scout W1). Agent definition files in `.claude/agents/` can fix each role's model and tool list. Four files would do it: planner on Fable, executor on Opus, reviewer and scout on Sonnet. That replaces the rule-2 model check, and dropping the `Agent` tool from the Sonnet roles replaces `no_recursive_delegation_gate`. `CLAUDE_CODE_MAX_SUBAGENT_SPAWN_DEPTH=1` does the same job from settings. `CLAUDE_CODE_DISABLE_BACKGROUND_TASKS=1` could replace `no_background_in_subagent_gate`, but it would also turn background tasks off for the main session, so check that first. `CLAUDE_CODE_STOP_HOOK_BLOCK_CAP` caps Stop-hook loops. Four things still need our own hooks: the budget and time caps, the preflight check, the media guard and the review gates. There is no native wall-clock cap, and the sandbox doesn't run on native Windows.

## Clean up

These are cheap and safe. Put `~\.claude` under git and delete the 195 `.bak` and `.retired` files that stand in for it. Add pruning for `hooks\state`: the agent counters, the watchdog logs, `shrink_guard` (31.9 MB), and doc review's `pending\` and `sources\`. Delete the seven stale hook copies in the project's `.claude\hooks\` (dated 3 September; nothing calls them), and the duplicate `model-postures.md` there. Remove the two plan hooks in `settings.local.json`; the folder they watch doesn't exist, so they never fire. The SubagentStop entry is an LLM-judged hook that runs on every subagent finish. It has no recorded catch in the 60 newest sessions, so fold its two checks into the agent definitions or drop it.

Some hooks have no real test. `posture_inject`, the fanout pair, `no_recursive_delegation_gate`, `no_background_in_subagent_gate`, `transcript_lib` and the inline bullet check are covered only by the UTF-8 smoke test, or not at all.

## CLAUDE.md and the agent briefs

The two CLAUDE.md files total 74 lines, well under Anthropic's suggested 200, so their size is fine. Their accuracy isn't. Scout I checked each claim against the hook code.

CLAUDE.md's "Doc review" section says all Markdown under `docs/` plus CLAUDE.md, CONTEXT.md and README.md is gated. The code gates only four public pages (`lib\doc_review_rules.ps1:53`, marked as your call from today), so the two folder exemptions described there no longer matter. Rule 2's sentence understates what the dispatch checker does, though that checker is dormant anyway. `docs/agents/doc-review.md` still describes edit reviews and decision questions that no longer exist.

The preamble has two mistakes. It says to commit before the 110th call, but the hook warns at 100 and stops at 150. It also says to write a `.new` file and move it over the old one to get past the shrink guard. That isn't needed for committed files, and the move skips the guard on exactly the text it protects.

D107, the status-line rule and the call and time limits are each written in four to six places. Scout I proposes one home for each: the preamble for how agents behave, and CLAUDE.md for policy, with every other copy shrunk to a pointer. Anthropic's guidance agrees: once a hook enforces a rule, the prose can shrink to a pointer.

## Decisions for you

1. **Fable or Opus for the main session?** This decides whether the Fable-only gates are retired or kept, and what the per-prompt posture should say.
2. **The shell-read gate.** Retire it, or cut it to a short rule? It makes about half of all denials, against a failure that showed up zero times before it existed.
3. **The Stop gate.** Retire it, or rewrite it properly with a retry cap?
4. **The status-line rule.** Count minutes only, or keep a call count, and if so how many calls?
5. **gh comments and plans.** Should every new gh comment, and every plan shown for approval, still need a fresh reviewer? Scout D flagged this as heavy for issue triage.
6. **The fanout and rerun gates.** Retire both?

## Proposed order of work

None of this has been built. Each piece waits for your yes.

The first wave fixes what is broken. It makes the dispatchers fail closed, following the other session's handoff. It retires or rewrites the Stop gate, removes the machine-wide ffmpeg kill, and fixes the reviewer role detection. These are small and independent, so they can run in parallel.

The second wave cuts noise once you've answered the decisions above: the status-line rule, the shell-read gate, the risky gate's word matching, the suite gate's false matches, and the fanout and rerun gates.

The third wave consolidates. It gives each event one dispatcher, parses the command once, merges the review library, splits the attribution gate, and adds native agent definitions.

The cleanup can run alongside any wave: git for `~\.claude`, state pruning, stale copies, and the CLAUDE.md and preamble corrections. That last piece is a doc change, so it goes to you first.

## Sources

- Anthropic hooks reference and guide, settings and subagent docs, and the Claude Code changelog v2.1.288 to v2.1.296. URLs are in `W1.md`.
- Community reports: Blake Crosley's 95-hook write-up (blakecrosley.com/en/blog/claude-code-hooks), and anthropics/claude-code issues 29709 (Edit blocked, then written through Bash), 54360 and 55754 (Stop-hook loops), 24757 (lost exit codes on Windows), and 43612 and 27755 (hooks inside subagents). URLs are in `W2.md`.
- Transcript scan script: `G_scan.py`, written to the session's scratch folder and not kept.
