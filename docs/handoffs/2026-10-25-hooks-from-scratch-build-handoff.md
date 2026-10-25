# Handoff: build the hooks from scratch

Written 2026-10-25 by the main session of session dee9c61e-7dcb-4a56-a783-a2dcb797c025. The user approved the plan and asked for this handoff so the next session builds it. Nothing in the plan is built yet.

## What to build

The plan is `docs/superpowers/plans/2026-10-25-hooks-from-scratch.md`. It replaces every hook in `C:\Users\Patrick\.claude\hooks` (about 30 gates, 13,622 lines of PowerShell plus 562 lines of C, with 17,259 lines of tests) with a small set of Python scripts in a new `gates\` folder. The target is about 2,200 lines of hook code, with a hard budget of 3,000. The set serves five objectives and nothing else: don't destroy the user's things, one fresh review before `main`, agents bounded and visible, the main session on the rules, and housekeeping. Each objective is guarded where the fact is exact, never by reading free shell text, apart from one strict delete rule and one fixed flag word.

The reasons are in `docs/audit/2026-10-25-hook-design-audit.md`. The short version: most hook bugs came from parsing shell commands and from rebuilding who wrote what out of git history, and the system grew with every fix.

## What the user decided

The user's words on 2026-10-25: "i want to be able to give explicit approval for a full library run. otherwise, approve as reommended and create a hjandoff for the next session to build".

So Q1 keeps the typed `allow library run`. The plan's section 2 says how: `hydra_batch` refuses a no-folder run without `--whole-library`, and `percall.py` lets a call holding that flag through only once per typed allow. Q2 to Q8 take the recommendation written under each question in the plan's section 8. One small call the main session made while folding the answer in: the `gh api` deny rule should cover the posting forms only, not read-only calls (T7 narrows it).

Earlier rulings from the same session, which shaped the plan: "the point of doing an audit is to improve, not make the problem worse", and "make a new plan to get rid of every hook we have now and start over from scratch". Style rules (`sed -i`, `python3`, patch scripts, the bullet check) become instructions in the preamble only.

## How to run it

Start with T0 alone. It probes the Claude Code behaviours the design rests on, on the app's own 2.1.308 binary by full path (`%APPDATA%\Claude\claude-code\2.1.308\...\claude.exe`; the `claude` on PATH is an older 2.1.275). Among them: does SubagentStop fire for a command hook, where a reviewer's hand-back text sits, and does `onFailure: block` work. If a probe fails, T0 stops, and its fallback goes to the user before anything else is built. Then eleven tasks run in parallel, one Opus executor each, in worktrees of `C:\Users\Patrick\.claude` (T9 in a Hydra worktree). The plan's section 7 gives each task's files, tests and order. T10 replays two weeks of real tool calls before cutover, T12 is the cutover, and T13 deletes the old folder after 14 clean days.

Everything still runs under the old hooks until T12 switches `settings.json`, so the old review gates apply to the build itself. The two-commit doc-review fix is already live (`~\.claude` main at 149466a), so a reviewer may fix in several commits.

## Loose ends

The earlier redesign plan, `docs/superpowers/plans/2026-10-25-hook-redesign.md`, is superseded. Its branch `hook-redesign-plan` and worktree `C:\Users\Patrick\Downloads\Hydra\wt-hook-redesign-plan` can be removed, along with the untracked copy in the main checkout. This plan's own worktree is `C:\Users\Patrick\Downloads\Hydra\wt-hooks-scratch-plan` on branch `hooks-from-scratch-plan`.

The reviewer of this plan flagged three things it could not settle. The Claude Code docs claims in the plan (permission-rule behaviour, hook field lists, the 2.1.295 minimum for `onFailure`) were checked by the planner but not by the reviewer; T0 checks them live. Two of the line estimates look optimistic: the ref judge at about 200 lines and the delete rule at about 150. The 3,000-line budget test catches that.
