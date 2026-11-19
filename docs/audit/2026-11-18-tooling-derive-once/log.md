# Tooling derive-once audit: log

The plan is `docs/superpowers/plans/2026-11-18-tooling-derive-once-audit.md` on branch `claude/tooling-derive-once-audit-plan`, at commit `1aa7bf5a` (reviewed at `caf35607`, then the user's three answers added). The user approved it on 2026-11-18 by choosing "Approve (Recommended)".

## T0: what the scouts read

- Hydra: `main` at `f905e372b5adabbaf24b9a2a498b7044064f4f02`.
- `C:\Users\Patrick\.claude`: `main` at `38f9649fe5bdcab5035ebbde9a65bd1fd92f5ef3`.
- Files in `C:\Users\Patrick\.claude` that differ from that commit, read live by the scouts: `CLAUDE.md` (modified), `skills/analyzing-sp-squeezes/SKILL.md` (modified), `skills/reverse-engineer-anything/` (untracked). Nobody in this session made these edits.
- Running beside the audit: branch `claude/remove-old-hooks` in `C:\Users\Patrick\.claude` removes the old `hooks\` folder, its rollback paragraph in `gates\README.md` and the test tools that read its state, on the user's words "why is there a whole folder of retired hooks? delete that shit". If it merges before a scout reads those files, the scout reads the new state; each report names the commit it read.

## Launches and verdicts
- T2 launched 2026-11-18 20:51: one calibration scout (Sonnet), brief briefs/T2-calibration.md, after the key's commit 509c0c2e.
