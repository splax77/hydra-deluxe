# Handoff: maintenance recommendations wave (MR1 to MR4), 2026-10-06

## What this session did

The user asked how large companies keep big codebases maintainable, and what that means for Hydra. Research produced a report with ten ranked recommendations. The report is `reports/Maintaining large codebases with AI.md`, published as the artifact https://claude.ai/artifact/7ZXMRXxoCidzGNDNsv9zfc. The user then approved seven of them, and this session built them.

Three are finished and need nothing more. The project `CLAUDE.md` no longer carries its own "How to explain things" section; the global `C:\Users\Patrick\.claude\CLAUDE.md` is now its only home, and the BAD/GOOD pattern-check example moved there too. The memory index was pruned from 98 lines to 52: 51 memories stay, and 59 finished or superseded ones moved to `memory\archive\`, where sessions don't load them. Nothing was deleted. The user chose PIT's defaults for the mutation probe's time limit.

The other four are code, and they sit on one wave branch that is not merged yet. MR1 adds a copy-paste scan to `tests/test_single_owner.cpp`, with a window of 8 code lines and a baseline of today's 26 copies that can only shrink (ADR 0025). MR2 makes a kind-A review fix end in a scan row, and makes the precheck print a range's size and warn over 400 changed lines. MR3 adds `tools/mutation_probe.ps1`, which plants small bugs one at a time and checks a test fails. MR4 adds `.github/workflows/ci.yml`, which builds and runs the suite and the headless UI tests on GitHub for every push and pull request to `main`.

## Where the wave stands

The wave branch is `claude/mr-wave` in `.claude\worktrees\mr-wave`. Its committed tip is `0cc02db`. A fresh Sonnet reviewer signed it off CLEAN on that key, after one exchange. It found two kind-A copies, and the integrator fixed both: a comment-line check written four times (now `is_line_comment`) and a second C++ comment lexer in the probe (it now loads the precheck's). The review file is in this session's scratchpad as `mr-wave-review-final.md`.

Then `main` moved to `1818f1c`, the SPEDGE wave from another session. SPEDGE also took decision number D81, so the wave's decision record clashed in `docs/audit/2026-10-03-fix-decisions.md`. The user stopped work while the integrator was merging `main` into the wave.

**The merge is half done and uncommitted.** In the mr-wave worktree, `MERGE_HEAD` is `1818f1c` and the resolution is staged. SPEDGE's D81 stays as it is, and this wave's record is now D82. The D59 item 4 note and the three code comments (`kCloneWindowLines`, `$largeRangeLines`, `Get-TestTimeoutMs`) were renumbered to D82. Nothing was built or tested after that. Start by reading `git -C .claude\worktrees\mr-wave diff --cached` and checking every D81 that remains belongs to SPEDGE.

## What to do next

Finish the merge in the mr-wave worktree. Do a warm build of `hydra_tests`, then run `-tc="single-owner*"`, `-sf=*docs_match_code*`, `tools\test_derive_once_precheck.ps1`, `tools\test_mutation_probe.ps1` and the precheck on `main...claude/mr-wave`. Watch the scan test closely. SPEDGE added about 139 lines to `tests/test_highway_draw.cpp`, and the new clone scan will check them. If it fails on a block SPEDGE added, don't add it to `known_clones` quietly; that is a finding against code already on `main`, so take it to the user. Commit the merge with separate `-m` arguments and the Task, Agent and Session trailers.

Then run the full suite once and merge the wave to `main`. The gate keys reviews on the exact tip, so it may ask for a review of the new merge commit. If it does, tell the user what the gate says before dispatching anyone. The user objected to the number of agents this session spawned, so do small mechanical steps like this yourself.

Pushing is the user's call. The first push starts CI's first real run. Three things could fail it. GitHub's Visual Studio may raise a warning the local one doesn't, and `/WX` makes that an error. A few timing tests (a 20 ms query, a 500 ms cancel) could flake on GitHub's 4-core machines. And nobody has run the full suite on a clean machine before. CI skips exactly one test, "PreviewAudioDevice opens and starts the default output device", because GitHub runners have no sound output.

After the merge, the task worktrees `mr1`, `mr2`, `mr3`, `mr4` and `mr-wave` can be removed.

## Left uncommitted in the main checkout

`CLAUDE.md` is modified. This session replaced the "How to explain things" section with a one-line pointer to the global file. The same file also has an "Agent rules" section that was already uncommitted when the session started, and its origin is unknown. Both are left for the user to commit. `reports/` and `research_notes/` are untracked; they hold the research report, its HTML page and the six researchers' notes.

## Open follow-ups

The user hasn't answered one question. The real planning rules live in the user-scope hook `executor_dispatch_brief_gate.ps1`. MR2 suggested adding one sentence to its deny message: "Size each task so its diff stays under the large-range threshold in tools/derive_once_precheck.ps1, so each reviewed range needs no split." Nobody has touched the hook.

The mutation probe's first run, on `src/core/replay.cpp`, caught 15 of 20 planted bugs. One survivor looks like a real test gap. At line 285, when two notes sit equally far from a typed SqOut offset, no test checks which one is picked. The other four survivors look harmless.

The clone baseline has 26 copies to shrink over time. Most are in tests. The ones in shipped code are the HTML header shared by `dm_report.cpp`, `fill_report.cpp` and `report.cpp`, a select block shared by the first two, a read loop in `ma_reader.cpp` and `vorbis_reader.cpp`, a block in `fillcompare.cpp` and `report.cpp`, and a table copied from `dynamics_breakdown.cpp` into its test.

The reviewer left four non-blocking notes. The two tools self-tests carry near-identical git-fixture helpers. Four scripts now load definitions from another script's text, each with its own small loader. `clone_line_text` notices a `/* */` comment only when it starts a line. No self-test covers the probe's timed-out path.

Nineteen `[[links]]` in surviving memories point at files now in `memory\archive\`. They're harmless, but they could be cleaned up.

## Journal state at handoff

Subagents active in the last 30 minutes and not finished:
  agent-abf9aaa0f35cbe9ab: unfinished, last tool call SubagentHandback, touched 00:05
  agent-aeb97c2373907da44: unfinished, last tool call PowerShell, touched 00:07

The reviewer (abf9aaa0f35cbe9ab) had already handed back its CLEAN verdict. This session stopped the integrator (aeb97c2373907da44) in the middle of its merge, which is the half-done state described above.
