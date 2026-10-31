# The Clone Hero reference: finish handoff (2026-10-30, session b02ab3e6)

The build is about four fifths done. Every page is written and reviewed except the last three branches, whose reviewers the user stopped mid-review, so none of them signed off. The code tools are on `main`. The pages are not: they sit on the join branch `claude/chref` and three branches beside it. Nothing is pushed.

The plan is `docs/superpowers/plans/2026-10-30-clone-hero-reference.md`; the build handoff that started this session is `docs/handoffs/2026-10-30-clone-hero-reference-build-handoff.md`. The user's ruling on the plan's ten questions is D277 in `docs/audit/2026-10-03-fix-decisions.md` (it is on `main`; another session's commit 2d323e06 carried it, then renumbered its own record to D278).

## What is on `main`

The four code tasks landed one at a time, each after one fresh reviewer signed it off CLEAN, and each time the three always-run checks passed (docs match code, the single-owner scan, the precheck self-test).

P0 restored the capstone wrapper as `tools/ch_mcp/chdis.py`; it reproduces all 201 committed single listings (183 whole, 18 as far as the committed file goes). Its reviewer added `decode_f32` to `tools/ch_probe/process.py`. K5b gave `tools/ch_mcp/dump_facts.py` a `--tsv` mode that writes the facts tables and a `--check` mode; its reviewer taught it to read the generic-instantiation `|-RVA:` lines in `dump.cs`. K5a made `tests/test_docs_match_code.cpp` read `docs/clone-hero/` and check every game fact against the facts tables and every rule block's seven lines; while no page holds a rule block it checks nothing, so it passes on `main` today. K5a's merge also brought K1's ten page shells to `main`.

In the hooks repo, K4 is merged: a session card can carry a `write-back:` line, `check.py` checks it, and the index marks it `[write-back]`. The untracked `session-cards\writer-brief.md` was edited to match (its old copy is in the scratch copy below, `k4\writer-brief.orig.md`).

## What is on branches, and its state

| Branch | What it holds | State |
|---|---|---|
| `claude/chref` (join, at 5fbe71ae) | the evidence files, K3, W1 to W4 | each part reviewed CLEAN and merged in |
| `claude/chref-w4b` (a7ab73ae) | README, questions, live checks, the new-listings index | reviewer stopped, no sign-off |
| `claude/chref-k2a` (2f2d6e83) | old ADR, audit and `CONTEXT.md` copies cut to pointers | reviewer stopped, no sign-off; it left two uncommitted edits in its worktree (`CONTEXT.md`, `FINDINGS-play-time.md`) |
| `claude/chref-k2b` (765dd714) | old plans, handoffs and research cut to pointers | reviewer stopped, no sign-off |
| `claude/chref-evidence` (97a491cc) | 25 overlay-spike result files and three handoffs | reviewed CLEAN; on the join, not yet on `main` |

The join holds seven area pages with 345 rule blocks: 177 proven in the game's code, 32 proven in a live run, 34 Hydra assumptions and 102 Unknown (W4b's count from the pages). It also holds 79 listings under `code/6142/` and two 1.0 decompiles under `code/4045/`. Every worktree is under `.claude\worktrees\chref-*`.

## What is left, in order

1. Decide what to do with the K2a reviewer's two uncommitted edits. A fresh reviewer can keep or redo them; they are partial work from a stopped review.
2. Dispatch one fresh reviewer each for W4b, K2a and K2b, through the `Agent` tool with `subagent_type: reviewer` and the tip hash. The briefs this session used are in the scratch copy: `wave3\page-review-brief.md` for W4b, and the K2a and K2b points below. A stopped reviewer left no verdict, so these are their first reviews, not second rounds.
3. Merge each signed branch into the join.
4. J1: cut from the join, run `dump_facts.py --tsv` for 6142 and 4045 to write `docs/clone-hero/code/facts-6142.tsv` and `facts-4045.tsv`, build `hydra_tests` cold in its worktree, and run `"-tc=docs match code*"`, `"-sf=*single_owner*"` and `"-sf=*test_precheck*"`. The version strings the tool needs are `--version 1.1.0.6142-final` and `--version v1.0.0.4045-master`; the 4045 run hashes `C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll`. Plan lines 328 to 330 give J1's caps.
5. Land the evidence branch on `main` by itself (it is signed, so a clean merge of `main` with 97a491cc passes the gate). Then merge `main` into the join.
6. Move `main` to the join tip. This must be docs only (see the gate note below), and it runs the precheck's `always:` lines first.
7. The memory edits (plan question 3, approved in full by D277 item 3), one note at a time, by the main session. Memory is not in git, so copy each note before you change it. One correction to the plan's table: `ch-star-cutoffs`' re-dump recipe needs no README line, because `tools/ch_mcp/dump_build.py` line 104 already sets `DOTNET_ROLL_FORWARD=Major`; the note's line points at that.
8. Add the hand-over line to the Star Power plan on `main` naming `docs/clone-hero/live-checks.md` (plan line 334; check with `git grep live-checks.md main -- docs/superpowers/plans/2026-10-30-ch-mcp-star-power-and-song-start.md`). Another session is building from that plan, so add only that one line.

## The gate note that changed the plan's last step

The plan says `main` moves once, to the join tip. The ref gate refuses that when the move carries code: it accepts only a signed tip, or a clean merge of `main` with one signed commit (`_code_review` in `C:\Users\Patrick\.claude\gates\review_rules.py`). Every path under `tools/` counts as code (`BUILTIN_CODE` there). So each reviewed code change landed on `main` alone, and the evidence branch, whose files are under `tools/`, must land alone too. What is left for the join is docs only, which the gate lets through. Nothing under `docs/clone-hero/` is a public page.

## Briefs for the stopped K2a and K2b reviews

K2a: the range is `5fbe71ae...2f2d6e83`. Check every row id a pointer names against its page. Check that no fact was deleted without a home. Fix the leftover "Beat" entry in `CONTEXT.md` (near lines 739 to 748), which still carries three RVAs and "resolution × 4" that page 04's FL-26 holds. Note which page blocks cite "ch-evidence.md lines N" or ADR 0028's old lines, since that text now lives only at 5fbe71ae.

K2b: the range is `5fbe71ae...765dd714`. Check every row id a pointer names. Confirm the eight facts the executor marked "no row on the pages" really have no row. Check that IN-03 and IN-36 on page 07 still have evidence, now that `docs/agents/clone-hero-mcp.md` lines 37 to 47 are pointers.

W4b's reviewer also gets two page points W4b found. CR-39 may be settled: the decompile's `[end]` calls pass MemoryExtensions.StartsWith, a case-sensitive prefix test. And `0x20F0320` has two descriptions, the lane remap in its listing note and the flam step in NL-38.

## For the user (the reviewers' Notes, passed on)

- The two formula tables in `tools/ch_probe/constants.py`, `RVA_FORMULA_NORMAL` and `RVA_FORMULA_PRECISION`, carry each other's names, so the probe tool's normal-mode window prediction runs on the precision constants (Q-34 and Q-76; W4's reviewer confirmed it against the live poll file to five decimals). Nothing in `src/` reads them. The fix is a small tool change outside this build.
- Q-50 (a hit up to one frame past the Star Power end still scores doubled) is not proven: the listing shows the end check's place in the frame, not where a press is scored. SP-29 is Unknown and on the live list.
- Q-51 holds: the game doubles an accent or ghost only when the hit sets a performed flag; a keyboard run got 0 of 43 and the bot all 43. Hydra pays every one.
- W1's reviewer ran out of time before opening every block's evidence; NL-53 to NL-59 and CR-21 and CR-24 to CR-28 were not sampled.
- The delete rule refused an agent's recursive delete inside its own `.claude\worktrees\` worktree, saying its working folder was not in a worktrees folder; it seems to judge the session's starting folder. It needs its own fix in the hooks repo.
- The plan's P0 pin path is one folder off: the file is `docs/audit/drum-lanes/play/f20deec0.asm`.

## The scratch copy

This session's scratch folder is copied, all 1,004 files, to `C:\Users\Patrick\Downloads\Hydra\hydra-data\chref-scratch-2026-10-30\`. It holds the claim tables (`wave1\*.md`, `wave2\<scout>\claims.md`), the scouts' listings, the writers' question and live files (`wave3\w*-questions.md`, `wave3\w*-live.md`), the canonical-listing map (`wave3\listing-owners.txt`), and the writer and reviewer briefs (`wave3\writer-brief.md`, `wave3\page-review-brief.md`). The briefs name this session's scratch path; point them at the copy.
