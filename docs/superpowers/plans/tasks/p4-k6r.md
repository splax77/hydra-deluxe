# Task p4-k6r: record the confirmed numbers (phase 4, K6 records half)

Plan: `docs/superpowers/plans/2026-10-04-phases-3-5.md`, task K6. Handoff: `docs/handoffs/2026-10-04-phase4-handoff.md`. The recheck with evidence is `docs/audit/2026-10-04-session-audit/step45.md` (status table and section F). The user's answers are D48 in `docs/audit/2026-10-03-fix-decisions.md`; Q33 in `docs/audit/2026-10-04-phase-3-5-questions.md` lists the numbers. Read them first, with offset and limit.

No score, path, record or on-screen text changes. The results stamp stays "2.1.0".

## Worktree

You continue in task p4-c1's worktree, `.claude\worktrees\p4-c1`, on branch `claude/p4-c1`, on top of its commits. Its build folder is warm: rebuild with `pwsh .\build_cpp.ps1 -Target hydra_tests`. p4-c1 added `tests/test_docs_match_code.cpp`; read it before you start, because your text must pass it.

## Part 1: CONTEXT.md line ~68 (D48 Q1)

"the first path in a record is optimal" becomes: every path that ties the record's top score is optimal. Name the owner function only if it exists on this branch (phase 3 adds `HydraRecord::is_optimal` on another branch); otherwise describe the rule in words without a backticked name.

## Part 2: write each Q33 number down once

The user confirmed each number as it is (D48 Q33). Give each one sentence in `CONTEXT.md` or the ADR that owns its area: ADR 0006 or 0019 for audio, 0008 for the Preview, 0014 for records and the fingerprint. Read the code for each number first and quote the owner (file and name). Findings:

- **310**: the average multiplier reads 0.000x for a path with no scoring notes (`Path::avg_mult`).
- **333**: the report's tier ladder: 2 ms, half a hit window, one window, one and a half, two, then Beyond.
- **R7.33**: UI timings: 0.5 s "Done!", 2.0 s "Copied!", 0.15 s search refilter, 1.0 s batch refresh. (Phase 3 is naming these constants in `theme.h`/`app_state.h`; describe them in words, don't backtick names that don't exist on this branch.)
- **327**: renderer, audio and leaderboard literals: overlay min scale 0.6, 50 ms cancel poll, flat-line epsilon, the magenta fallback colour, the 64-byte Ogg tag window, the WinHTTP timeouts, status 200 only.
- **336**: star cutoffs multiplied as 32-bit floats, as the decompiled game does (`stars.cpp`). Add it to CONTEXT's "Star cutoff" entry.
- **337**: Preview spans end half a tick past their last note (`kSpanEndTicks`, `track_state.cpp`).
- **339 / 348**: the half-millisecond "on an activation" slack (`kOnActivationMs`, `preview_view.cpp`) and the BPM 0.0 fallback. (The 4/4 default already has one owner under D27.)
- **349**: the rules fingerprint is FNV-1a 64 over name=value text, 17 significant digits, and 0 maps to 1 (`rules.cpp`). ADR 0014 today says only "the fingerprint of the rules".
- **R7.32**: the Preview loading bar's 8/82/4.5 percent shares, MB rounding, one-second pacing and the switch to minutes at 59.5 s (`preview_load_job.cpp`).
- **R7.34**: the audio reader numbers: 120 ms for a zero-byte Opus packet, 512-packet queue, MP3 seek points every 0.5 s, 1 s hop, 4096-frame blocks, the 4 GiB MP3 and 2 GiB Vorbis limits.
- **R7.36**: 16 parked lookups (`app_state.h`), engine progress every 0.005 (`engine.cpp`), 0.9 of the bar for the main pass (`pather.cpp`).
- **R7.39**: correct ADR 0006 (~line 31) and ADR 0019 (~lines 71-72): a chained Opus file plays only the links that match the first link's channel count; the reader stops keeping links at the first one with another channel count or no audio page (`opus_reader.cpp` ~222-236).
- **R7.37**: leave ADR 0019's "decode error ... goes silent" paragraph and CONTEXT's "Mixer" sentence as they are. They wait on two open user calls (audit R7.8, FLAC length 0, and R7.9, Opus seek after damage). Say so in your report.

Don't touch ADR 0020 (another session owns it).

Wherever a number you write is one of the docs test's marked defaults (10 ms Path limit on, 50 ms backend hide off, 3 ms leeway, 500 ms squeeze window), add its marker exactly as the test expects.

## Part 3: name two literals once (code)

- The magenta fallback colour is written twice in `src/render/preview_config.cpp` (~lines 20 and 24). Make it one named constant in that file and use it in both places.
- The "OpusHead" tag is spelled in `src/audio/decode.cpp` (~79) and `src/audio/opus_reader.cpp` (~181). Name it once in `src/audio/decode.h` and use that name in both places. Check with grep that no other copy is left in `src/`.

Values are unchanged. Run only the relevant tests: `-sf=*preview_config*`, the decode and Opus reader test files (`-sf=*decode*`, `-sf=*opus*`), and `-sf=*docs_match_code*`. Never the full suite.

## Owned files

`CONTEXT.md` (line ~68 and the new number sentences), `docs/adr/0006-*.md`, `0008-*.md`, `0014-*.md`, `0019-*.md` (the number sentences and R7.39 only), `src/render/preview_config.cpp`, `src/audio/decode.h`, `src/audio/decode.cpp` and `src/audio/opus_reader.cpp` (the tag use only). Nothing else.

## Commits

One commit for Parts 1 and 2, one for Part 3. Trailers: `Task: p4-k6r`.
