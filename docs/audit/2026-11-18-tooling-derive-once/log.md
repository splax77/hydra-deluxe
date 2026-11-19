# Tooling derive-once audit: log

The plan is `docs/superpowers/plans/2026-11-18-tooling-derive-once-audit.md` on branch `claude/tooling-derive-once-audit-plan`, at commit `1aa7bf5a` (reviewed at `caf35607`, then the user's three answers added). The user approved it on 2026-11-18 by choosing "Approve (Recommended)".

## T0: what the scouts read

- Hydra: `main` at `f905e372b5adabbaf24b9a2a498b7044064f4f02`.
- `C:\Users\Patrick\.claude`: `main` at `38f9649fe5bdcab5035ebbde9a65bd1fd92f5ef3`.
- Files in `C:\Users\Patrick\.claude` that differ from that commit, read live by the scouts: `CLAUDE.md` (modified), `skills/analyzing-sp-squeezes/SKILL.md` (modified), `skills/reverse-engineer-anything/` (untracked). Nobody in this session made these edits.
- Running beside the audit: branch `claude/remove-old-hooks` in `C:\Users\Patrick\.claude` removes the old `hooks\` folder, its rollback paragraph in `gates\README.md` and the test tools that read its state, on the user's words "why is there a whole folder of retired hooks? delete that shit". If it merges before a scout reads those files, the scout reads the new state; each report names the commit it read.

## Launches and verdicts
- T2 launched 2026-11-18 20:51: one calibration scout (Sonnet), brief briefs/T2-calibration.md, after the key's commit 509c0c2e.
- T2 run 1 verdict (report `reports/TOOL-AUDIT-T2-run1.md`, 17 calls, about 2 minutes, never warned by the hook): K-A missed, K-B found (C1, writer-brief line 101 named as a copy of progress.py's numbers), K-C found (C1 kinds line: writer-brief's "stop starting new cards" named a K3 candidate against the hook's warning). K-A: the scout quoted the preamble's "Write the time yourself" line but filed it as a K1 copy of the line form and a K5 on the call shape, never as a value the receiving program could supply itself. The audit stopped here; the miss was reported to the user.
- Measured count, run 1: 46 distinct values over seven files, with no hook warning, so this is a floor on one scout's reach, not its limit.
- Brief fix for run 2 (`briefs/T2-calibration-run2.md`): the K4 line now asks, for every value a text tells someone to type or report, whether the receiving program could get it itself and whether the one asked can know it, says one line can be K1 and K4 at once, and gives a made-up Hydra example (song length, D75). It holds none of the key's sentences.
- T2 run 2 launched 2026-11-18 20:54: one fresh calibration scout (Sonnet). This is the last run the plan allows.
