# Handoff: plan the fixes from the 2026-10-03 derivation audit

The audit is finished as far as the user wanted it. The next job is planning the fixes. Nothing has been edited yet. Every code change still needs the user's yes, plan first.

## What exists

The findings are in `docs/audit/2026-10-03-derivation-audit.md`. It holds 352 findings: 122 drifts, 13 double readings, 166 duplicates that agree today, and 51 undecided assumptions. Within each group, findings are ranked by how visible they are, and 26 are wrong or contradictory on screen today. Each finding names the question, every copy, a truth table where useful, the disagreeing input, the screen effect and a proposed owner. Its last line lists the candidate ids it came from.

The coverage ledger is `docs/audit/2026-10-03-derivation-ledger.md`. It has a row for every one of the 2,688 functions in scope, plus the question catalogs the agents wrote.

The prompt that produced the audit is `docs/handoffs/2026-10-03-derive-once-audit-prompt.md`.

## Where the audit stands

Calibration passed. A blind agent caught both canaries in `rate_activation`, and an independent checker confirmed them against the code at 707c285.

The completeness stop rule was **not** met. The passes and rounds found 4, 4, 26, 14, 11 and 12 new items, and the user chose to stop after round 6 and take delivery. So the report's claims are limited to "none beyond these found in the ledgered functions by methods 1 to 4, in the passes that ran". Don't describe the audit as complete.

One verifier accidentally ran `hydra_batch` with no arguments. It wrote to the untracked `build-cpp/Release/hydra.db` at 10:01 on 2026-10-03. If that database matters for a check, rebuild it.

## Where to start planning

Section 4 of the report proposes an order. It's a proposal, not something already agreed. Ask the user which step to plan first.

1. **Let the engine stamp facts the displays now guess**, the ADR 0011 pattern. This covers the pre-SqIn end, the cap-clamp anchor, late-SqIn phrase collection in the Preview gauge, the chords SP pays in the replay, the SqOut rating on phrase notes the engine never squeezes out, and the double rating in `rate_activation`.
2. **Fix the parser drifts.** At Hard and below, the Dynamics tab reads Expert's 2x kicks and disco flip follows Expert's markers. The Dynamics kick totals disagree, and a late dynamics tag behaves oddly. The `.chart` and `.mid` parsers disagree on when an SP phrase ends, and a `.mid` with a zero time signature crashes the app.
3. **Give display formatting one owner per rule.** This covers ms and percent rounding, counted nouns, colours, note names, rich-text stripping, Stale messages, the meaning of "optimal", the Preview's stale chart mode, and the song panel's stale result.
4. **Correct the docs.** That means the User Guide, ADRs 0011, 0012 and 0013, the cap-clamped doc and the CLI messages.
5. **Hold one decisions round with the user** on the 51 undecided assumptions.
6. **Fold the duplicates that agree today**, one module at a time.

## Overlap with other 2026-10-03 work

**One squeeze-rating plan.** `docs/superpowers/plans/2026-10-03-one-squeeze-rating-rule.md` is an existing, untracked plan for the `rate_activation` problem, with seven user decisions in its header. The audit deliberately did not read 2026-10-03 plans, to keep the calibration blind. So some findings it lists as undecided are already decided there. The 1 ms impact cutoff goes, per decision 1. Orange has no threshold, per decision 7. The figure shows whenever the stored multiplier isn't exactly 1, per decision 5. Treat that plan as the starting point for step 1's `rate_activation` part. Before writing a new plan, check which audit findings it already covers, and mark the 1 ms and 0.005 assumption findings as decided.

**Preview loading plan.** `docs/superpowers/plans/2026-10-03-preview-loading-fixes.md` and its spec `docs/handoffs/2026-10-03-preview-loading-audit.md` come from another session. That plan reworks the Preview load path and forbids any visible change. Several audit findings touch the same area: the song-length and transport end, the Preview keeping an old chart mode, the time box tempo pick, and the gauge. Sequence the two plans so they don't edit the same functions at once, and ask the user which goes first.

## Rules the next session must keep

These come from the user's standing preferences. Explain how a thing works today and what would change, and get a yes before editing any code. Any change to what the user sees is a blocking question up front, never a side effect noted afterwards.

Before claiming a fix needs no re-analysis, check `src/store/stored_versions.h` and bump `kResultsStamp` when analysis output changes. Every fact gets one owner, and display code reads stored engine facts instead of re-deriving them. Plans are written in plain English, per CLAUDE.md, and cite prior art found on the web where it exists.

Keep agent fan-out lean. Batch small, uniform jobs about eight per agent. Use three or four agents per discovery round, and pass them the previous round's notes on what was already searched. Stage commits by file name, because other sessions leave edits in this checkout.

## Workflow state at handoff

Nothing is in flight. Every audit workflow has either completed or been stopped on purpose. The stopped ones were replaced by later runs: a leaky first calibration, a run restarted at higher concurrency, a merge run switched to batches, and the serial completeness run switched to parallel rounds. Journal state as reported by the hook:

```
wf_3bb1d621-426  completed  derive-once-calibration      agents=1
wf_f2fe1746-b20  killed     derive-once-calibration      agents=1   (first brief leaked canary shapes; discarded)
wf_a8d171da-c88  completed  derive-once-finders          agents=11
wf_9088ef82-327  killed     derive-once-verify-sweep     agents=9   (restarted at 14-way concurrency)
wf_e772dfbd-ef8  killed     derive-once-verify-sweep     agents=10  (restarted with parallel sweep verification)
wf_5fce0c99-0b7  completed  derive-once-verify-sweep     agents=23  (verifiers)
wf_bf8df99d-254  completed  derive-once-verify-sweep     agents=58  (sweep + its verifiers)
wf_b942899c-272  completed  derive-once-write            agents=23
wf_08f39c23-333  completed  derive-once-write            agents=10
wf_4ac7ed0f-54d  killed     derive-once-consolidate      agents=55  (15 results kept; rest moved to batches)
wf_b9f66201-14e  completed  derive-once-merge-batched    agents=5
wf_7b637b1e-5a2  killed     derive-once-completeness     agents=7   (passes 1-2 kept; switched to parallel rounds)
wf_b8223fb8-df8  completed  derive-once-completeness-rounds agents=57 (rounds 3-6 + canary check)
wf_aa896c9f-b73  completed  derive-once-write            agents=9   (completeness write-up)
```

The intermediate files (candidates, verdicts, the function list script, the ledger merge) lived in the old session's scratchpad and may be gone. Everything needed for planning is in the two `docs/audit/2026-10-03-*` files.
