# Perf exploration: shared brief for every experiment agent

The user asked us to explore speed ideas for Hydra's chart scanning and chart analysis **in a scratchpad, without modifying any of the real code**. You are one of five experiment agents running at the same time. Your own task is in your prompt; this file holds the rules you all share.

Read `C:\Users\Patrick\Downloads\Hydra\hydra-test\docs\agents\brief-preamble.md` first and follow it, with the overrides below. Where this file and the preamble disagree, this file wins for this task (it exists because the task is throwaway experiments, not shippable changes).

## Where you work

- Make a **detached** worktree of main, so no branch is created and main is never touched:
  `git -C C:\Users\Patrick\Downloads\Hydra\hydra-test worktree add --detach C:\Users\Patrick\Downloads\Hydra\hydra-test\.claude\worktrees\perf-<key> main`
- Build it inside a build slot: `pwsh -NoProfile -File C:\Users\Patrick\Downloads\Hydra\hydra-test\tools\build_slot.ps1 -Repo <worktree> -Target <target>`. Targets you will want: `hydra_batch`, `hydra_bench`, and `hydra_tests` only if you run a named test. Build each target by -Target; never build everything. A cold build takes a few minutes.
- **Before you edit anything**, copy the unmodified `hydra_batch.exe` and `hydra_bench.exe` from `<worktree>\build-cpp\Release\` to `<scratch>\baseline\`. That is your A in every A/B timing. Check that its folder holds the same settings file situation as your experimental exe (hydra_batch reads settings next to the exe), so both run the same analysis settings.
- Your scratch folder is `C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\<key>\`. Notes, copied dbs, logs and patches go there.
- **Never commit.** Leave your edits uncommitted in the worktree. At the end, save `git -C <worktree> diff > <scratch>\<key>.patch` so the work survives. Never touch the main checkout's files, other agents' worktrees, or the installed app in `C:\Program Files\Hydra`. Never push.
- Do not remove your worktree at the end; the main session cleans up.

## Data

- The user's library is `C:\Clone Hero` (about 19,500 charts). Read only.
- The real GUI database is `C:\Program Files\Hydra\hydra.db` plus `-wal` and `-shm`. **Copy all three** into your scratch folder and use the copy. Never open the original. Analysis timing on the real db uses `hydra_batch --db <copy> --redo` (memory note: a fresh db hid a 69 s slowdown once). A fresh `--db` is fine for correctness comparisons.
- Giant charts are the stress tests: blink-182 Discography (3 MB MIDI; find it under `C:\Clone Hero` or the repo's testdata), the Endless Setlist .sng/.srb archives (about 1 GB each), Rise Against "Discography (2024)". Find their paths with Glob or Get-ChildItem.
- Read `tools\bench.cpp` and `src\cli\batch.cpp` for flags. `hydra_bench --scan <folder>` prints an enumerate vs read+hash split. `hydra_bench <chart folder>` times parse, graph and search for one chart.

## Timing: one benchmark at a time, machine-wide

Every timing run that matters (anything over a few seconds, and every number you report) goes through the shared lock. So does every whole-library run, even a correctness-only one, because eight busy worker threads would skew another agent's timing. Only short single-chart runs may go outside the lock. The lock script waits up to 2.5 minutes for the lock, then up to 1.5 minutes for compilers and other Hydra processes to finish, before it starts the clock. Call it with the PowerShell tool's timeout at 600000 ms and keep your timing script under 5 minutes, so the whole call fits in 10.

`pwsh -NoProfile -File C:\Users\Patrick\AppData\Local\Temp\claude\C--Users-Patrick-Downloads-Hydra-hydra-test\6dce3c94-eba9-4fd1-96c6-3b1ceb4ec9aa\scratchpad\perf\bench_run.ps1 -Label "<key>-<what>" -Script <your timing .ps1>`

Write the timing script so it runs A and B interleaved (A, B, A, B, A, B) and prints each wall time; report the median of each. Exit code 3 means the lock stayed busy: do other work (correctness checks, reading), then retry. Never run a long timing outside the lock. The log of every run lands in `bench_log.md` next to the script, with how many compiler processes were running at the start; quote that number with your timings.

The OS file cache stays warm between runs and we cannot flush it without admin. Say so: scan numbers are warm-cache numbers unless you show otherwise.

## Correctness is part of every result

A speedup only counts if the output is identical. For scan changes: the same rows in the same order with the same md5s. For analysis changes: byte-identical stored results. The strong check is to run baseline and experimental `hydra_batch` over the whole library into two fresh dbs, then compare the results, paths and charts tables with a Python script (system Python is `py`; see memory note inspect-hydra-db-without-sqlite-cli). Report the count of charts compared and the count that differ. Any difference is a finding to report, not something to fix by loosening the comparison.

## Budget and reporting

- Status lines: append one line to `C:\Users\Patrick\.claude\hooks\state\status\<your agent id>.md` every 10 tool calls or 5 minutes, form `HH:MM done ... | next: ...`, using `& "C:/Users/Patrick/.claude/hooks/status_append.ps1" <your agent id> "<line>"`.
- Keep a running findings file `<scratch>\NOTES.md` and update it after every measurement, so a fresh agent can continue if you run out of calls. Start wrapping up at 100 tool calls; 150 is a hard stop.
- Never use run_in_background and never end your turn waiting on a job.
- Fail loudly: when something blocks you, stop and say exactly what broke.
- Your final report, plain English, under 900 words: for each idea you tried, what you changed (one or two sentences), the A/B numbers (median, with the run count and the compilers-busy count), the correctness result (charts compared, charts differing), and a verdict: worth doing, not worth it, or needs more work, with the reason. End with where your patch and notes are. Follow the "How to explain things" rules in `C:\Users\Patrick\.claude\CLAUDE.md`.
