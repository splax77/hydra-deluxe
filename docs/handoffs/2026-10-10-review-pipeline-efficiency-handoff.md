# Handoff: make the review pipeline cheaper (2026-10-10)

**Closed 2026-10-10, nothing left to build.** The user agreed to close it. Later
work covered all four fixes. Rule 5's one-reviewer rule (D107) has the reviewer
fix its own findings, which covers C. A one-review-per-change rule covers B. The
review carry-over in hooks 26d3848 covers A. The "refused merge: abort it" recipe
and getting keys from main_move_check.ps1 cover D. The text below is history.

**For the next session: plan this, don't build it.** Bring the plan to the user
and wait for their yes before touching any hook (repo rule 3). Fixes A and D
below only change how the gates work. Fixes B and C change review rules the user
set (CLAUDE.md rule 5, decisions D61 and D106), so the user decides those
first.

## Why this exists

The user asked why a small change took so many reviews. The change was the .sng
and .srb song id fix, merged to main at b70a1769 (ADR 0028). Its code was about
200 lines. The review phase was the stretch from the first code review to the
merge. It cost:

- Six fresh review agents. Their spawn results add up to about 19 minutes and
  781k subagent tokens: 163k, 171k, 116k, 101k, 122k and 108k.
- Two refused merges. Each one left 13 to 15 files staged in the shared main
  checkout, which then had to be restored by hand.

The reviews did find real problems:

- A false claim in the ADR: that a broken .srb used to fail when clicked.
- A speed-up the ADR claimed but nobody had timed.
- Stale "chart-file MD5" comments in `src/app/dm_report.h` and
  `src/net/dmbot_client.h`.

So the goal is to cut the ceremony, not the checking.

## What happened, in order

1. The code and the two docs (ADR 0028 and `docs/srb-format.md`) went into one
   commit, 36be3a70.
2. A derive-once reviewer and a doc reviewer ran in parallel. Both found
   wording problems only.
3. A fresh Opus fixer fixed the code-comment findings. Its commits were
   53b504b1 and 31ac450f.
4. The main session fixed the docs itself, since it wrote them (rule 5).
5. A derive-once checker signed 31ac450f CLEAN. A doc checker signed both docs
   CLEAN, with small edits of its own.
6. The main session committed the signed docs as b70a1769. The merge gate then
   wanted a new derive-once review keyed on b70a1769, though
   `git diff 31ac450f b70a1769` touches only the two docs.
7. A sixth agent confirmed exactly that and signed b70a1769. Main then moved.

The review files are in this session's scratchpad:
`C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\bc0144a4-aa75-440b-8d75-25c771d52121\scratchpad\`.
They are `derive-once-review-36be3a70.md`, `-31ac450f.md` and `-b70a1769.md`,
plus the `review\` folder for the doc reviews.

## The four fixes to plan

**A. Let a code sign-off survive docs-only commits.** The derive-once gate keys
a review to the exact tip hash, so any later commit voids it, even one that
touches only Markdown. The gate already lets main move with no review when
old..new changes no gated path: `Get-DeriveOnceMainMoveRefusalIn` in
`C:\Users\Patrick\.claude\hooks\lib\derive_once_rules.ps1`, about line 254. The
proposal extends that: if some signed-off key K is an ancestor of the new tip,
and K..new changes no gated path (per `Test-DeriveOnceGatedPath`), allow.

Things to settle in the plan:
- How to find K. Search the submitted reviews for an ancestor of New, or keep a
  per-branch record.
- What happens when main moved between K and New.
- Whether the doc gate still covers the docs added after K. It should, since
  the doc gate is separate.

This alone would have saved the sixth agent.

**B. One review per branch for code and docs together.** Today they are two
pipelines. Each runs review, fix and check with fresh agents, so a branch with
code plus a doc pays twice. The proposal is one reviewer reading both briefs
(`docs/agents/derive-once-review.md` and `docs/agents/doc-review.md`), then one
fix and one checker. It would submit to both gates. Six agents would have been
three. This needs the user: it changes how rule 5 runs, and the two submit
helpers, which today expect separate reviewers.

**C. Let the author fix findings that are only wording.** When every finding is
about wording in comments or docs, the author fixes them and a fresh checker
verifies. That is already the rule for documents the main session wrote. A
finding that changes behavior would still get a fresh fixer. This needs the
user: it amends D106 item 5 and `docs/agents/fix-round.md` ("send a fresh Opus
agent to fix them").

**D. Stop refused merges from dirtying the shared checkout.** When the gate
refuses a fast-forward of main, git has already rewritten the main checkout's
files and index. The gate's own message says so. This session ran into it twice.

It was also partly the session's own mistake. The fleet-control-hooks memory
already records this trap ("never try the merge"). The derive-once key is just
`git rev-parse <branch>`, and doc keys come from `doc_review_key.ps1` per file.

Options to weigh:
- A read-only helper that prints every key a tip would need, so nobody has to
  try a merge to learn them.
- A PreToolUse gate that refuses `git merge`/`git push` to main before git
  runs, when the reference-transaction check would refuse.
- Both.

## Constraints for the plan

- **Hooks live in `C:\Users\Patrick\.claude\hooks\`.** Their tests are
  `hooks\tests\test_h*.ps1`; the fleet-control-hooks memory says how to run
  them. Back up each hook file before editing it, as `*.bak-<tag>`, following
  the existing backups.
- **Fix hooks with tests, never route around them.** That's a memory rule. A
  hook failure must deny with a reason or warn visibly, never "ask".
- **Every gate fails closed.** Fix A must not let an unreviewed code change
  through. Test the case where code sneaks in after K.
- **Search the web for prior art before finalizing the plan.** That's a memory
  rule. For example, look at how CI review-approval systems keep or drop an
  approval on new pushes; GitHub's "dismiss stale approvals" setting is one
  example to check.

## Session state at handoff

The fix itself is done. main is at b70a1769, local only and not pushed; the
user hasn't said whether to push. Branch `song-id-archives` and its worktree
`.claude/worktrees/song-id-archives` still exist.

The journal listed these subagents as unfinished, each with
`SubagentHandback` as its last call:

```
agent-a0145ea1e54b2372c: unfinished, last tool call SubagentHandback, touched 00:14
agent-a6f1087e5b506620b: unfinished, last tool call SubagentHandback, touched 00:15
agent-a75f4ba67fbd9f887: unfinished, last tool call SubagentHandback, touched 00:21
agent-a7a592b949ac0a264: unfinished, last tool call SubagentHandback, touched 00:25
agent-aa1a7e1c45bda7dda: unfinished, last tool call SubagentHandback, touched 00:23
agent-aad04e49dc7f32328: unfinished, last tool call SubagentHandback, touched 00:30
```

All six delivered their final report through `SubagentHandback`, and those
reports were read. Nothing is still running. Launch nothing to "finish" them.
