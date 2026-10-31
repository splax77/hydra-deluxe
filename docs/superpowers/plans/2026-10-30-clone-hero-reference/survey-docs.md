# Survey: the docs record and the public Hydra

Written 2026-10-30 by planning agent a9e3fccb5e22bd654 (task `ch-reference-plan`, slice "docs record and public Hydra"). Read-only. I read the documents named below in the main checkout `C:\Users\Patrick\Downloads\Hydra\hydra-test` and the public Hydra tree at commit 3b49012c. I did not open Clone Hero, the dumps, or Ghidra, and I did not run Hydra. Every line count below comes from `Get-Content` on the file; every byte count from `Get-Item`. Where I say a doc "claims" a label, that is the doc's own wording, not something I verified against the game.

One thing to know before the plan is sized: **the disassembly listings that back `ch-evidence.md` are not in the repo.** The doc points at two session scratch folders (lines 7 and 11: `...\9168b711-...\scratchpad\il2cpp\` and `...\cebf3006-...\scratchpad\triage\re\`). The feasibility study says in its own case that such scratch paths "no longer exist" (line 17 of the results handoff). So the only committed `.asm` evidence today is under `docs/audit/note-shuffle/` and `docs/audit/drum-lanes/`. Every other address in the record (hit window, SP cap, phrase edges, disco, dynamics tag, fill landing, calibration, star base) has to be re-derived into `docs/clone-hero/code/` by the reverse-engineering scouts, or the reference labels it "proven in code, listing not kept" and says so.

## 1. Document by document

### docs/audit/ch-evidence.md

273 lines, 21,234 bytes, dated 2026-10-03. It answers twelve numbered questions from the derivation audit. Areas: 1 (quoted `.chart` events, Q141), 2 (disco per difficulty Q11, dynamics tag Q64, `.chart` ghosts and accents Q320, MIDI 2x kick on every difficulty in "surprises" item 6), 3 (hit window cap and floor Q77, playback speed Q222), 4 (fill length Q53, 6/8 beats Q314, fill rules Q315), 5 (phrase edge Q21, SP cap clamp Q37, 3 ms leeway Q304).

It already labels how each fact is known, with a five-step scale at lines 13-19: "CH code", "Measured", "Hydra doc", "Format doc", "Other tool". That maps onto the request's four labels as CH code = proven in code, Measured = proven live, and the other three = assumed. It names an RVA for every CH-code claim.

Two parts are stale. Q53 (line 92) says "I could not find Clone Hero's fill generator", and Q315's table (line 229) says "generator not found". The generator was found on 2026-10-12: `0x20D2B30`, named in `src/core/song.cpp` line 169, ADR 0010 line 177 and `CONTEXT.md` line 584 (decisions D229 and D234). Nobody went back to update ch-evidence.md. The whole file is replaceable by a pointer once the reference exists: keep it as a dated record with one line at the top saying the living copy is `docs/clone-hero/`, or cut it to that line. The address list is its only unique content, and the reference takes that.

### docs/adr/0023-chart-rules-follow-clone-hero-code.md

114 lines, 6,370 bytes. Areas 1, 2, 4, 5. It does not use the four labels, but its structure implies them: the rules at lines 17-67 each carry an RVA (proven in code, as the ADR claims), lines 69-85 are the deliberate differences, and lines 87-104 are the unverified rules. It flags its own weak spots in-line, for example line 38-39 ("This assumes Clone Hero's MIDI reader does not trim the text first; that was not checked") and line 42 ("The case of a chord at that tick without that colour was not traced").

The "Not verified against Clone Hero" list, exactly, lines 87-104 (six bullets, all area 4 except where noted):

1. A generated fill's length when two or more meter changes fall between two chords (lines 92-93).
2. How many ticks make a beat in compound meters like 6/8, including the 4-beat deadline (lines 94-96).
3. Whether one authored fill turns off generated fills for the whole chart (line 97).
4. Whether Clone Hero carries its search position from one fill to the next (lines 98-99).
5. Generated-fill tie to the later chord (`Song::check_activations`, D74) (lines 100-101).
6. Generated-fill distance and length cut to whole ticks (`Song::check_activations`, D74) (lines 102-104).

Items 1, 2, 5 and 6 are partly overtaken by the 0x20D2B30 reading of 2026-10-12 (ADR 0010 lines 167-191 and `src/core/song.cpp` lines 169-184 describe the generator's placement, +1 tick, cooldown `add ebx, 4`, and the grid restart at 0x20DACA0). Item 3 is still open: the audit-of-audits block `docs/audit/2026-10-29-audit-of-audits/blocks/AOA-W2c.md` line 72 says the 0x20D2B30 read "read its placement, not the caller that decides whether to run it". Item 4 is untouched. This list is what the request says becomes a pointer; the rest of the ADR is the decision and stays.

The deliberate differences (lines 69-85), three of them: `drums0dnoflip` turns flip off though the game reads it as on (0x215CD50; a game bug; 48 library charts plus 5 chart-difficulty pairs on the other `drumsN...` spellings); the 3 ms backend leeway, Hydra's own rule (D29); and a fill with a chord on one side only, which the game takes at any distance and Hydra drops (D30).

### docs/adr/0028-a-chart-id-is-clone-heros-song-id.md

75 lines, 4,056 bytes. Area 1 (the song id). Its evidence section (lines 23-46) is live-run evidence: scans of `songcache.bin` on 2026-10-09 and 2026-10-10 (54 `.sng`, 30 `.srb`, 300 folder charts; 26 of 30 `.srb` checksums equal the notes MD5). No game code is cited. Under the request's labels the rule is "proven in a live run" with the dates but no kept result file (line 44: "a temporary test that was not kept"). The evidence section becomes a pointer; the decision stays.

### docs/srb-format.md

252 lines, 11,153 bytes. Area 1. Its label is one sentence at line 5-6: "worked out by inspecting the files directly, and every fact below was checked against the real ones". No game code read. It marks its own unknowns: header meaning (line 49), the audio type tag (line 125), which blob is which instrument (line 144). It is already a topic page, so the plan can link it from `docs/clone-hero/` rather than copy it, or move it under `docs/clone-hero/` and leave a pointer.

### docs/audit/note-shuffle/ (the model for docs/clone-hero/code/)

The folder layout, from a recursive listing:

- `step0/FINDINGS.md` (9,473 bytes): the written answers, one section per open question, with a "How it was done" paragraph (three scouts, one skeptic each, "None of the three answers was refuted") and a "Game tests" section with the live results.
- `step0/agent-results.json` (97,969 bytes): each scout's full answer with addresses.
- `step0/open1/`, `open2/`, `open34/`: one folder per question. Each holds the `.asm` listings named `<role>_<RVA>.asm` (for example `open1/chart_addnote_215BBF0.asm`, `open34/engine_ctor_20f79e0.asm`), the caller lists as `callers*.txt`, string cross-references as `strrefs.txt`, and the small Python helpers that produced them (`dis.py`, `fxref.py`, `tools.py`).
- `step0/open1-refute/`, `open2-refute/`, `open34-refute/`: the skeptic's own listings, same naming, plus a short `README.md` where it wrote one.
- `step0/corpus/` (results.csv 343,565 bytes, the capped reference, classify scripts), `step0/kick2x/scan.py`.
- `game-tests/`: `README.md`, `predictions.json`, `build-scripts/`, and ten probe songs under `songs/`.
- `scout-flag/` and `scout-strings/`: an earlier session's `.asm` files named `f<RVA>.asm` plus the Python shuffle reference.
- `probes/` and `verify/`: Python references and a video-check script.

What makes it the model: one `.asm` file per function, named by role and RVA; a refute folder beside each answer folder; a FINDINGS page that says which claims are code reading and which are game-tested (line 63: "Not tested in the game: the note order inside a .chart chord, and the flam copy's position"). It has no per-rule label table and no RVA-to-file index, which the reference should add.

Areas: 1 (ticks not rescaled, both readers), 2 (chord sort order, flam copies, 2x kick merge, Note Shuffle freeze cases), 6 (star base taken after the shuffle). Labels: explicit per section ("read", "game tests ... every prediction held"). Replaceable by a pointer: the rule statements in FINDINGS.md; the `.asm` folders stay as evidence and get linked or moved under `docs/clone-hero/code/`.

### docs/audit/drum-lanes/ (a second .asm folder, not named in the request)

`FINDINGS-play-time.md` (21,659 bytes) plus eleven `.asm` files named `f<RVA>.asm` and a `play/` subfolder with six more. Areas 2 (lane flags at note+0x7C), 3 (grant budget, 0.075 s lane-press window, lines 39-42), 5 (line 46: when SP can activate, `0x20F57F0`, a missed fill-end note is granted). Labels: code reading throughout, dated 2026-10-10. The reference should mine it; the plan's scout list should include it.

### docs/research/2026-10-28-clone-hero-mcp-feasibility.md

174 lines, 27,495 bytes. Areas 6 (the `scorestats.json` field list at line 31; plain Drums clears accent and ghost bits at 0x20EFF3B, line 133; keyboard input never registers dynamics, line 131), 7 (obfuscation table lines 45-50, `GameManager` fields line 54, the three per-note methods lines 143-147, bot never uses Star Power line 129, dump locations line 41, the v1.0 build line 162). Labels: partial and in prose, for example "that is read from the code, not run" (line 133) and "That reading of the code is not tested" (line 131). Replaceable by a pointer: the "Results of the four tests" facts (lines 111-168). The study's reasoning stays as research history.

Note: its raw evidence folder moved. Line 113 names a scratch path; the results handoff line 17 says the same files now live in `C:\Users\Patrick\Downloads\Hydra\hydra-data\ch-mcp-runs\2026-10-28\`. The reference's "proven in a live run" rows should cite that durable folder.

### docs/agents/clone-hero-mcp.md

41 lines, 4,805 bytes. Area 7 only. Game facts it holds: the results screen takes no keys after a bot run (line 35), a finished `GameManager` stays in memory with `m_CachedPtr` cleared (lines 37-39), both dated 2026-10-29. Labels: dated live checks, in prose. Those two paragraphs become pointers; the tool how-to stays.

### Plans and handoffs (grep for ch_probe, ch_mcp, overlay, fill, squeeze, hit-window, note-shuffle, star)

Sizes are line counts. "CH lines" means how many lines matched my Clone Hero keyword skim, capped at nine per file, so it is a flavour, not a count.

- `plans/2026-09-25-hit-window-testing.md`, 126 lines. Area 3. Labels: a status note at the top (D28) says steps 1-3 are answered from code plus `poll_windows.csv`, steps 4-6 shelved. Facts: cap 171.43 ms, floor 96.29 ms, both gaps halved and clamped 37.5-85 ms (lines 8-14), constants' home `tools/ch_probe/constants.py` (line 15). Lines 3-16 become a pointer; the probe-song description stays.
- `handoffs/2026-09-26-ch-probe-live-runs-handoff.md`, 133 lines. Areas 3 and 7. Procedure only, no new rules. Nothing to replace.
- `plans/2026-10-28-overlay-spike.md`, 273 lines, and `handoffs/2026-10-28-overlay-spike-sitting-handoff.md`, 111 lines (untracked in git, per the session's `git status`). Areas 5 (the SP fields +0x68, +0x1B0, +0x42, +0x48, +0x120, handoff lines 45-49; the meter is 30 x resolution in game units against Hydra's 32, line 50; the game skipped path 3's E0 fill, line 51), 3 (calibration trace lines 67-88 with RVAs 0x374D50, 0xB41A0, 0x16C480, 0x16C490, 0xC05A0; the game's own 2.5 ms average, line 65), 6 (checksum is plain MD5 for `.chart`, line 54; ghost and accent pay from a keyboard, line 52), 7. Labels: the best in the record. Every calibration claim is tagged in italics, "Scout's disassembly", "checked by the main session from the file's bytes", "not fully proven" (lines 73-81). The write-up it was meant to feed, `docs/research/2026-10-28-overlay-spike-results.md`, is a 17-line unfilled template. So the run facts live only in an untracked handoff. The plan should either have the reference cite the handoff after it is committed, or treat these rows as "proven in a live run" with the result files in `tools/ch_probe/experiments/results/overlay-spike/` (28 files, also untracked).
- `plans/2026-10-28-ch-mcp-server.md`, 319 lines; `plans/2026-10-28-ch-mcp-feasibility-tests.md`, 215 lines; `handoffs/2026-10-28-ch-mcp-feasibility-tests-handoff.md`, 53 lines; `handoffs/2026-10-28-ch-mcp-results-handoff.md`, 53 lines. Area 7. The results handoff holds four user rulings in the user's words (lines 19-24: EULA "Proceed, my call"; plain Drums "Keep Hydra as is"; "Cap them too"; the Mono reader). The feasibility-tests handoff line 41 says `ch_probe`'s hit-check RVA `0x100DAB0` "is not the drums version of that method", which contradicts the comment in `constants.py` line 55 that still calls it the hit check. That is a split for the reference to show.
- `plans/2026-10-08-note-shuffle.md`, 157 lines; `handoffs/2026-10-08-note-shuffle-handoff.md`, 41 lines; `handoffs/2026-10-09-note-shuffle-build-handoff.md`, 51 lines. Areas 2 and 6. Plan line 51 names a second modifier after the shuffle (bit 0x20, function 0x20F1900, called at 0x20D3329) that Hydra does not model. Labels: FINDINGS.md carries them; the plan and handoffs restate. All three become pointers for the rules; the plan stays as the design record.
- `plans/2026-09-27-stars-tab.md`, 528 lines. Area 6. Lines 15-27: "Clone Hero's code was decompiled on 2026-09-27"; the nine-float table 0.1, 0.5, 1.0, 2.0, 2.8, 3.6, 4.4, 5.2, 6.0; cutoff is ceil of a 32-bit float product; the solo bonus is left out of the compared score. **No RVA anywhere in the plan**, so under the request's rule ("never write a function from memory") the star-cutoff row is "proven in code, listing not kept" until a scout re-finds it.
- `plans/2026-10-18-song-length-end-event.md`, 229 lines. Areas 1 and 3. Lines 28-49: the game's song-end rule from a decompile (`out1.c`, `FUN_18212e130`), the `.chart` flag at `settings+0x2C`, and an open question whether `[end]` matching is case-sensitive (line 49). Named by a Ghidra function label, not an RVA, and the `out1.c` file is not in the repo.
- `plans/2026-09-24-derivation-fixes.md`, 11,027 lines. Areas 3 and 5. Line 19: "nobody knows yet which way Clone Hero applies the audio delay" (Task 17). Line 58: the 3 ms leeway, the squeeze-out rule, the fill constants and the 1/32-beat slop are "your own" (the user's) rules. Line 77 cites the song.ini wiki page. The delay-versus-Offset answer is not in this plan; the memory note `ch-audio-delay-offset-rule` (outside my slice) claims it.
- `plans/2026-10-03-step2-parser-rules.md`, 5,653 lines, and `handoffs/2026-10-03-step2-parser-rules-handoff.md`, 80 lines. Restates D19-D30 with the same RVAs as ch-evidence.md. Handoff line 46 names `0x215BD50` as the cheapest breakpoint for the fill questions. Pointers only.
- `plans/2026-10-03-step1-engine-stores-what-displays-guess.md`, 4,672 lines; `plans/2026-10-03-one-squeeze-rating-rule.md`, 563 lines; `plans/2026-10-15-early-fill-orange-85.md`, 105 lines; `plans/2026-10-22-squeeze-badge-beside-early-fill.md`, 154 lines; `plans/2026-09-27-preview-sp-drain-box.md`, 1,289 lines; `handoffs/2026-10-06-sp-end-triangles-handoff.md`, 47 lines; `handoffs/2026-09-24-backend-scoring-drift.md`, 108 lines. All Hydra-side design around area 5. No new game facts; the drift handoff gives the leeway's history (commits 3b69958 and ba0885a, lines 51-52). Nothing to replace.
- `plans/2026-09-26-codebase-audit-fixes.md`, 18,059 lines. Built `ch_probe`; area 7 tooling. Nothing to replace.
- `handoffs/2026-09-29-public-vs-deluxe-library-timing.md`, 258 lines. Timing, not rules. Skip.

### Other docs that hold game rules (found by grep, not in the request's list)

- `docs/adr/0010-legacy-fill-deadline-is-a-cli-only-mode.md`, 203 lines. Area 4. Lines 167-191: the 1.1 deadline is resolution x 4 ticks before the fill's start (0x20DDB70, `[+0x300] = resolution << 2` at 0x20DF7B7); the generator 0x20D2B30 and its +1 at 0x20D2C91; the grid restart at 0x20DACA0; SP length by continuous measure count; 1.0 mode keeps Hydra's old generator. Claimed proven in code (D115 read it). No `.asm` kept.
- `docs/adr/0012-kick-dynamics-are-priced.md`, 90 lines. Areas 2 and 6. Line 27 says the symmetric kick rule "is what Clone Hero's source" does, with no address.
- `CONTEXT.md` lines 562-592 (fill deadline, generator, 1.0 mode) and 469-494 (score rules, SP bars). Area 4, 5, 6. These are Hydra's owner notes and should point at the reference, not restate.
- `docs/audit/2026-10-29-audit-of-audits/` blocks AOA-A1, AOA-U, AOA-W2c and `questions.md` line 95: the open split on "any authored fill stops generation".

## 2. The rules the docs already state, by area

Each line: rule, source file and line, and the label the doc itself claims (code = proven in code per the doc; live = proven in a live run per the doc; assumed = Hydra's rule, doc says unverified; unknown). I did not verify any of these against the game.

### Area 1: finding and reading a chart

- A `.chart` `E` event matches `[mix`, `mix`, `solo`, `soloend` on trimmed raw text, case-sensitive; `E "solo"` is nothing. ch-evidence.md 150-167 (0x213CAF0, 0x20FE510). code.
- Whether the game takes `mix_3_drums0d` or `mix 3 drums0d` and Hydra's regex does not. ch-evidence.md 167. unknown.
- MIDI drum track = "part drums" or "part drum" any case, first in file order, never track 0; events track = first "events" any case. ADR 0023 lines 46-57 (0x2155FDF, 0x2156134, 0x21564A8, 0x21564D7, 0x2155F8D, 0x2155F36, 0x2155F3D). code.
- A track with two names at tick 0 takes its first recognised name. fix-decisions D78 line 346. live (the game plays Hormone's Echo), no code.
- Folder chart id = MD5 of the notes file; `.sng` id = MD5 of the unmasked notes file; `.srb` id = stored 16-byte checksum. ADR 0028 lines 12-18. live (songcache scans 2026-10-09/10).
- `.chart` MD5 is also the id in `scorestats.json`. overlay handoff line 54. live.
- `.srb` layout: 16-byte header, four raw-DEFLATE streams, encrypted Opus audio. srb-format.md 30-37. live (file inspection); header meaning unknown (line 49).
- Ticks are not rescaled by either reader; the seed uses file ticks. note-shuffle FINDINGS 11-15. code.
- Song end = earlier of stated length and end event; `[end]` / `end` both count in `.chart`. song-length plan lines 28-49; D255 (fix-decisions line 898). code (decompile, no RVA kept); `[end]` case sensitivity unknown (line 49).
- Section names shown as Clone Hero shows them, from its 2,718-entry list. D224, D228 (lines 807, 815). code claimed in the session, no address in the docs.

### Area 2: building the note list

- Disco flip is per difficulty in both formats. ch-evidence 25-37 (0x215C750, 0x213D076, 0x2155050); ADR 0023 17-20. code.
- `drums0dnoflip` reads as flip ON in the game (7th-character bug, 0x215CD50); Hydra reads it as off on purpose. ADR 0023 71-77. code, deliberate difference.
- 2x kick: MIDI 59/71/83/95 per difficulty, `.chart` N 32; the Double Kick modifier is the only gate. ADR 0023 21-24 (0x21555CD, 0x210D9C0, 0x20D32B7); ch-evidence 273. code.
- A 2x kick and a normal kick on one tick merge; with 2x off the tick has no kick. FINDINGS 23; D105 (line 491). code and live (songs F1, F2).
- Chord order is kick, red, yellow, blue, green with flag tie-break; the shuffle walks that order. FINDINGS 19. code (.mid also live via song B; .chart code only, line 63).
- Flam copies: fired only on a single-pad chord; copy sits one pad up. FINDINGS 21. code, not tested live (line 63).
- Note Shuffle: xorshift seed from the first four notes, redraw loop, copy rule, two freeze cases. FINDINGS 33-39 and the plan. code plus live (freezes seen on C1, D1).
- MIDI dynamics tag counts only as the two exact spellings, per note in file order, local to the drum track pass. ch-evidence 114-129 (0x2155050, 0x21557A5, 0x21557BB). code; "whether the reader trims first" unknown (ADR 0023 38-39).
- `.chart` ghosts and accents always count; N 34-38 accent, 40-44 ghost, 66-68 cymbal. ch-evidence 247-262 (0x213D620, 0x213D160, 0x215E280, 0x215DBD0). code.
- A `.chart` modifier with no note at its tick is skipped. ADR 0023 40-42 (0x215DDB0). code; chord-without-colour case untraced.
- Plain Drums folds cymbals to toms (0x20F16E0) and clears accent and ghost bits (0x20EFF3B); MIDI goes the same way. feasibility 133. code ("read from the code, not run"); user ruled it a bug, Hydra keeps dynamics.
- Keyboard input never registers dynamics; the bot does. feasibility 131. live (one run) plus user ruling; the 0x20DE770 velocity reading "not tested".
- Lane flags at note+0x7C (0x4000, 0x8000, 0x10000); only the lane builder and the drum engine use them. drum-lanes FINDINGS 15-19. code.
- Kick dynamics are priced symmetrically. ADR 0012 line 27. claimed from "Clone Hero's source", no address: treat as assumed until a scout finds it.
- A second modifier after the shuffle (bit 0x20, 0x20F1900) that Hydra does not model. note-shuffle plan line 51. code, unmodelled.

### Area 3: timing

- Hit window: each half-gap = Clamp(gap x 0.5, 0.0375, 0.085); the sum feeds the drum formula at 0x20DDDA0; output caps at 171.43 ms and floors at 96.29 ms; the engine uses half. ch-evidence 133-146 (0x20F3A90, 0x20F7210); hit-window plan 8-14; D28. code plus live (`poll_windows.csv`).
- Hydra uses a fixed 85 ms per side. hit-window plan line 6. assumed (deliberate, D28).
- Audio calibration shifts the judged clock; video shifts drawing only; `songTime` = raw minus audio; engine+0x100 = songTime. overlay handoff 71-81 (0x374D50, 0xB41A0, 0x16C480, 0x16C490, 0xC05A0). code, with "drawing only" marked not fully proven; the two live runs show neither setting moves engine+0x100 against chart times (lines 57-61). live.
- Where the chart Offset or song.ini delay enters the raw clock: not traced. overlay handoff line 88. unknown in the docs. (The derivation-fixes plan line 19 says nobody knew; a memory note outside my slice claims an answer.)
- Playback speed: no speed term in the score math; SP counted in ticks. ch-evidence 171-185 (0x20F5F70, 0x20F6EB0, 0x20F6A50). code.
- Lane grants: fewer than 2 (single) or 4 (double) since the last lane press, or a lane press in the last 0.075 s. drum-lanes FINDINGS 39-42. code.
- The game's own average of +2.5 ms against clock reads of about -0.5 ms suggests ~3 ms polling delay. overlay handoff 65. inference, labelled so.

### Area 4: fills

- Authored fill lands on the nearest chord in [start, end + floor(res/32) + 1]; tie to the later chord; none means dropped; the +1 sits on top of `hydra_rules.ini` slop. ch-evidence 220-243 (0x20CFF60, 0x5DE030, 0x20D0061-0x20D008D); ADR 0023 28-35; D22, D30. code.
- A fill with a chord on one side only is taken at any distance by the game; Hydra drops it. ADR 0023 82-85 (D30). code, deliberate difference.
- 1.1 generator 0x20D2B30: starts at the second measure line, nearest chord within half a beat + 1 tick (inc at 0x20D2C91), fill starts on the line, cooldown `add ebx, 4`. ADR 0010 177-183; `src/core/song.cpp` 169-184; D229, D234. code, no listing kept.
- 1.1 deadline: resolution x 4 ticks before the fill's start (0x20DDB70, 0x20DF7B7). ADR 0010 171-175; CONTEXT.md 568. code, no listing kept.
- The grid restarts at the first beat at or after a time-signature change (0x20DACA0). ADR 0010 184-186. code.
- 1.0 fill rule (`legacy_fills`): Hydra's own model, kept exactly. ADR 0010 190; `src/core/song.cpp` 127-134. assumed (no 1.0 code read in the docs).
- The game did not offer path 3's E0 fill at tick 34944 on Trippin. overlay handoff 51. live, unexplained.
- Any authored fill means no generated fills. `src/core/song.cpp` 106-107. assumed; game side unknown (ADR 0023 line 97; AOA-W2c line 72).
- Fill search carrying its position between fills. ADR 0023 98-99. unknown.
- 6/8 beats and multi-meter fill length. ADR 0023 92-96; D26. assumed; partly overtaken by the 0x20D2B30 read but no doc closes it.
- A missed fill-end note is granted when SP can activate (0x20F57F0). drum-lanes FINDINGS 46. code.

### Area 5: Star Power

- Phrase membership: start <= tick < start + length; zero-length pays nothing; a phrase past the last note pays on it; flags 0x80 and 0x100. ch-evidence 41-66 (0x20D2440, 0x20D0330, 0x5DB320, 0x215C320); D21. code.
- SP amount is whole ticks at engine+0x68, max at +0x1B0; activation 0x20F5640 sets +0x42, doubles +0x48, computes the end at +0x120 via 0x20F37E0; per-update 0x20F6800 clamps to max and recomputes the end. ch-evidence 70-86. code. Live confirmation of the fields: overlay handoff 45-49 (all 11 phrase ends raised +0x68 by 1,438-1,440; +0x1B0 held 5,760; ends within 2.9 and 1.2 ms of Hydra's). live.
- The meter unit is 30 x resolution in the game, 32 in Hydra; why 30 is unknown. overlay handoff 50. live; unknown cause.
- Full meter = 4 bars (Hydra's `kSpActivationBars` 2 banked bars per activation). CONTEXT.md 477; ch-evidence 86. assumed in Hydra's terms.
- SP length by continuous measure count across a meter change. ADR 0010 187; D231. code claimed, no address.
- The 3 ms backend leeway is Hydra's own; no 0.003 in 126 engine methods; the game ends SP at a time checked per frame. ch-evidence 189-201; ADR 0023 78-81; D29. deliberate difference (no code).
- Squeeze window 140 ms and the leeway come from the public Hydra (see section 4). assumed.
- The bot never uses Star Power. feasibility 129. live.
- Drum activation is a press on a fill's last note; no separate activation key seen. overlay plan line 21. assumed (no code).

### Area 6: scoring

- Points added as base x multiplier at engine+0x94 (0x20F5F70, 0x20F6EB0, 0x20F6A50). ch-evidence 177. code.
- Per-note score method 0x20DEEC0 takes (note, base points = 50); dynamics method 0x20DE770. feasibility 144-147; constants.py 70-73. live (once per note) plus code.
- Star cutoffs: nine floats 0.1 ... 6.0, ceil of a 32-bit float product, solo bonus excluded from the compared score. stars-tab plan 15-27. code claimed, no RVA.
- Star base is taken after the shuffle. FINDINGS 27-29 (0x20F4680 called from the engine ctor). code plus live (song E: 106 notes, 19,925 points, 5 stars).
- Plain Drums score differs by exactly the dynamics points (8,600 on Trippin). feasibility 125, 133. live.
- Bot runs match Hydra's `total_notes`, `sp_phrases_total` and no-SP score on three charts. feasibility 121-127. live.
- `scorestats.json` fields: about 45 per player, listed by name at feasibility line 31 (`score, base_score, note_score, sustain_score, sp_score, combo_score, solo_bonus_total, squeezed_score, notes_hit, total_notes, max_streak, sp_phrases_earned, sp_phrases_total, sp_activations, time_in_sp, avg_multiplier, ghost/accent counts, is_fc, is_bot`, hit counts per tenth) and `sp_bar_ticks` (overlay handoff 54). The only field-by-field reader is `tools/ch_mcp/compare_scorestats.py`. No doc defines every field. unknown for the rest.
- Solo bonus formula: no doc in my slice states it; D251 (fix-decisions 874) says the session decoded the solo box. Outside the docs it is a memory note. unknown in docs.
- Multiplier by combo 1/2/3/4 at 10/20/30: public Hydra `hymisc.to_multiplier` only. assumed.
- Dynamic cymbal's extra points go where the game puts them. D208 (line 775). live claim, no detail in the log line.

### Area 7: internals

- `GameManager` keeps 40 of 60 field names (`songTime`, `songLength`, `isPaused`, `isSongOver`, `scoreManager`, `starProgress`, ...). feasibility 54. code (dump).
- Obfuscation counts per assembly. feasibility 45-50. code (dump, regex count, "close, not exact").
- Three per-note methods: 0x20DEEC0, 0x20DE770, 0x100E6E0. feasibility 143-147. live.
- `ch_probe` engine fields: window +0x20, back +0x30, front +0x38, clock +0x100, score +0x94, hit time +0x2E0 (unconfirmed). constants.py 85-106 with per-constant "proven live" / "NOT yet confirmed" notes. mixed, labelled in the file.
- `0x100DAB0` is or is not the drums hit check: constants.py 55 says it is; the feasibility-tests handoff line 41 says it is not. split.
- v1.0 and v1.1 reshuffle most scrambled names (164 of 539 keep name and shape). feasibility 164. live (dump compare).
- A destroyed `GameManager` stays in memory; `m_CachedPtr` tells it apart. clone-hero-mcp.md 37-39. live.
- Results screen takes no keys after a bot run. clone-hero-mcp.md 35. live (one run).
- `SongEntry` route to the checksum (`GlobalVariables.instance` -> `currentSongEntry` +0x28 -> `checksum` +0x38). overlay handoff 95, from `il2cpp-fields.md`. code (dump).

## 3. D-records that rule on a Clone Hero behaviour

From `docs/audit/2026-10-03-fix-decisions.md` (1,062 lines). I filtered lines that start a D-record and mention Clone Hero, the game, or an RVA, then kept those that rule on game behaviour. Line numbers are the record's first line.

- D10 (line 25), area 5: multiplier disc drops x8 to x4 at SP end, confirmed on FC video.
- D19 (49), area 2: disco flip per difficulty; `drums0dnoflip` stays off.
- D20 (51), area 2: 2x kick per difficulty; Double Kick is the only gate.
- D21 (53), area 5: phrase-end rule, 0x20D2440.
- D22 (55), area 4: fill landing window +1.
- D24 (59), area 2: dynamics tag exact spellings, file order.
- D26 (63), area 4: no live check now; fill length and 6/8 stay unverified.
- D28 (67), area 3: hit-window steps 1-3 answered from code; 4-6 shelved.
- D29 (69), area 5: SP-end extension stops at a full meter as the game does; +3.0 ms stays uncounted.
- D30 (71), area 4: authored fills placed one by one after the chord list.
- D31 (75), area 2: 2x Bass tooltip names Clone Hero's Double Kick.
- D47 (113), area 1: `.chart` notes read in tick order before the phrase and fill rules.
- D74 (321), area 4: generated-fill tie and whole-tick cuts stay (unverified).
- D75 (328), area 1/3: song length from chart metadata, not audio.
- D78 (346), area 1: a track's first recognised name.
- D102 (450), area 4: E1 counts as no skip.
- D104 (482), area 2: Note Shuffle design.
- D105 (491), area 2: 2x kick and normal kick on one tick merge.
- D112 (525), area 2: drum lanes in the Preview.
- D115 item 6 (587), area 4: 1.0 mode keeps the old generator; SP length uses the exact measure count.
- D138 (635), area 1: never commit the real `.srb` bundles; read only the notes part.
- D170 (699), area 1: song-id change (ADR 0028).
- D208 (775), area 6: "matching the game is correct" for a dynamic cymbal's points.
- D222 (803), area 6: "Average Multiplier" is the game's stat name.
- D224 (807), D228 (815), area 1: section names as Clone Hero shows them; ship its name list.
- D227 (813), D237 (833), area 3: beat lines and weak-beat lines follow Clone Hero in x/8 meters.
- D229 (817), D234 (827), area 4: generated fills start on the measure line; a chord just before the line can take it.
- D230 (819), area 4: Clone Hero's grid is the one owner for fills, lines and position text.
- D231 (821), area 5: SP length measured continuously across a meter change.
- D250 (867), area 4/5: early-fill orange timing up to 85 ms (a Hydra setting).
- D251 (874), area 6: solo box decoded from 1.1.0.6142; 10 ms step.
- D252 (880), D253 (886), area 1: drum track names; track 0 never drums; first events track.
- D254 (892), area 2: lanes light every pad after Note Shuffle.
- D255 (898), area 1/3: song length reads the chart's end event.

ADR 0023's deliberate-differences list is in section 1 above (three items, lines 69-85).

## 4. The public Hydra at 3b49012c

`git show 3b49012c --stat`: "Version 1.3.1", Fri Aug 7 2026, one file changed (`hydra/hymisc.py`). `git ls-tree -r --name-only 3b49012c` lists six library files, three entry scripts, `README.md`, `docs/UserGuide.md`, resources, and `test/` with the IB24 and Summer Blast '25 charts (the same corpus as `testdata/input/common`). The files that hold Clone Hero rules, with the keyword counts I measured (`Select-String` per pattern):

- `hydra/hysong.py` (910 lines). Areas 1, 2, 4, 5. `MidiParser` (line 225) and `ChartParser`: disco regexes `r'\[?mix.3.drums\d?d\]?'` and the `dnoflip` form (lines 269-270, Expert-only, the rule D19 replaced); dynamics regex line 272; fill note 120 handling lines 286-289; SP note 116 lines 290-293; `.sng` reader with `XORMASK_OFFSET = 10` (line 175). `Song.check_activations` (lines 97-143) is the 1.0 generator Hydra Deluxe keeps as `place_fills_ch10`: within half a beat of a downbeat, tie to the later note, half-measure length, `ACT_COOLDOWN_MEASURES = 4`, `MAX_DISTANCE = self.tick_resolution // 2`. 120 lines match cymbal/disco/dynamic/ghost/accent/2x/kick; 63 match fill/activation.
- `hydra/hypath.py` (820 lines). Area 5 and 6. The 140 ms squeeze window (lines 86-87, 177); `is_leeway = be.offset_ms > 0 and be.offset_ms < 3` (line 678), the origin of the 3 ms leeway; SqIn/SqOut backend logic (lines 148-171); line 621 "Thanks to the timing window, the cutoff is -70ms not 0ms"; the score-per-chord function at line 746. 37 lines match window/leeway/backend.
- `hydra/hydata.py` (1,038 lines). Area 5 and 6 records: `MultSqueeze`, `FrontendSqueeze`, `BackendSqueeze`, `SqIn`, `SqOut`, `Path.avg_mult` (line 372). 39 lines match squeeze.
- `hydra/hymisc.py` (223 lines). Area 6: `to_multiplier(combo)` (line 229: 1/2/3/4 at 10/20/30) and the measure/tick timecode class. No star-cutoff table anywhere in the public tree (`git grep` for `0.1, 0.5`, `4.4`, `cutoff` finds only hypath.py 621).
- `hydra/hyencode.py` (1,108 lines). Area 2: chord encoding (551 keyword lines, all pad names).
- `hydra/hyutil.py` (307 lines). Area 1: file discovery (`.mid`, `.chart`, `.sng`; 26 matches), no `.srb`.
- `hydra_app.py` (1,140 lines) and `docs/UserGuide.md` (72 lines). Display and the user-facing wording of squeezes, windows and fills.

So the public Hydra is the home of three "Hydra assumes it" rules the reference must map: the 3 ms leeway, the 140 ms squeeze window, and the 1.0 fill generator. It has no star cutoffs, no `.srb`, no hit-window model beyond the -70 ms comment, and Expert-only disco.

## 5. The `claude/ch-sp-plan` branch, Ladies Talk, and the fill rule

**The plan file does not exist yet, committed or not.** The branch `claude/ch-sp-plan` is at 9f007234, the same commit `main` was merged to before 55a0e7f8 (`git log --oneline -3` in the worktree). `git ls-tree -r --name-only claude/ch-sp-plan -- docs/superpowers/plans` has no `2026-10-30` file. The worktree `.claude/worktrees/ch-sp-plan` has a clean `git status --short`, and a recursive search for `*star-power*` there finds nothing. `docs/superpowers/plans/` on main has no `2026-10-30*` file either. So there are no live checks to avoid duplicating yet. The reference plan should hand its live-check list over as a named section the other plan can take in whole, and should say which rows need only a bot run (the `ch` server has that today) and which need Star Power activation and a human-style start (what that plan is for).

**Ladies Talk facts in the record.** Searching the repo (outside `testdata/` and the note-shuffle corpus) and the project's memory and card folders for "Ladies Talk", "76 to 108" and "measures 76" finds the fact only in the request itself (`docs/handoffs/2026-10-30-clone-hero-reference-session-prompt.md` line 54) and this session's transcript. The card record mentions the chart only for squeeze scales (condensed card `ade9655b-...` lines 650, 761, 904, 1612: two variants; 15 scales on 8 charts under D1). The note-shuffle corpus ran it (results.csv lines 738-753, all 16 settings finished). So "written fills skip the solo" was not written down anywhere before the request.

I checked it against the chart. `testdata/input/common/IB24/T6/Senri Kawaguchi - Ladies Talk [Kiyo Sen] (Drumeo Performance)/notes.mid`, read with `ladies_talk_fills.py` in this scratch folder (`py -I`, a 100-line hand parser, no mido; now committed beside this survey): 480 ticks per beat; drum track "PART DRUMS"; 32 written fills, each on all five lanes 120-124; two solos (note 103) at ticks 5760-13440 and 140400-186480. The fills run every three to four measures up to tick 132960, then nothing until tick 191760, so the 58,800-tick gap holds the whole second solo. The measure numbers my script prints assume 4/4 and the chart alternates 3/4 and 4/4 from tick 15360, so its "74 to 98" for the solo is not the game's count; the request's 76 to 108 should be re-read on Hydra's `MeasureIndex` grid by whoever writes the check. The fact that matters is tick-level: no written fill between 132960 and 191760, solo inside it.

**Where Hydra's "any written fill means no generated fills" rule lives.** `src/core/song.cpp` lines 105-107, `Song::check_activations`: `for (const SongTimestamp& ts : sequence) if (ts.has_activation()) return;  // chart already has fills.` It runs after `place_authored_fills` (`src/parse/song.cpp` lines 363 and 1119/1874), so a chart whose only authored fill was dropped still gets generated fills, and a chart with one placed fill gets none under both generators (the return comes before the 1.0/1.1 choice at lines 112-115). The audit-of-audits names this as an open split with no ruling (`docs/audit/2026-10-29-audit-of-audits/blocks/AOA-W2c.md` line 72; `questions.md` line 95). The game side is "unknown" in ch-evidence.md line 229 and ADR 0023 line 97. The public Hydra had the same rule (`hysong.py` line 105, `if all(not ts.has_activation() ...)`), so it is inherited, not decided.

## Gaps the plan should size for

1. No committed `.asm` for most RVAs in the record (ch-evidence.md, ADR 0010, overlay calibration trace, stars-tab, song-length). Only note-shuffle and drum-lanes keep listings. The reverse-engineering wave must re-derive these into `docs/clone-hero/code/`, or the pages label them "proven in code, listing not kept".
2. Two docs cite a decompile with no RVA at all: stars-tab plan (star cutoffs) and song-length plan (`FUN_18212e130`, `out1.c`).
3. `docs/research/2026-10-28-overlay-spike-results.md` is a 17-line unfilled template; the SP-field and calibration live facts live only in an untracked handoff and 28 untracked result files.
4. Several handoffs the request names are untracked (`git status` at session start lists `2026-10-28-ch-mcp-feasibility-tests-handoff.md`, `2026-10-28-ch-mcp-results-handoff.md`, `2026-10-28-overlay-spike-sitting-handoff.md` and others as `??`). A pointer can only replace a committed file.
5. ch-evidence.md is stale on the fill generator (Q53, Q315) and nobody updated it; this is the "one home" failure the request describes.
6. One live split inside the tools: `0x100DAB0` called the hit check in `constants.py` line 55, called "not the drums version" in the feasibility-tests handoff line 41.
7. The `scorestats.json` fields have no defining doc; only a name list (feasibility line 31) and `compare_scorestats.py`.
8. The song.ini delay versus `.chart` Offset rule is absent from the docs in my slice (derivation-fixes plan line 19 says nobody knew); it lives in a memory note, which another scout should check.

## Promise map

This survey is a scout report, not a plan, so its promises are claims of fact. Each one names the check that proves it.

| Claim | Where in this file | Check |
|---|---|---|
| ch-evidence.md's listings are in dead scratch folders | intro; gap 1 | ch-evidence.md lines 7 and 11 name scratch paths; results handoff line 17 says scratch paths no longer exist; `git ls-files docs/audit` shows `.asm` only under note-shuffle and drum-lanes |
| ch-evidence.md Q53/Q315 are stale on the generator | section 1 | ch-evidence.md lines 92 and 229 say "not found"; `src/core/song.cpp` line 169 names 0x20D2B30 |
| ADR 0023's not-verified list is six bullets at lines 87-104 | section 1 | Read of the file, lines 87-104 |
| The ch-sp-plan plan file does not exist | section 5 | `git ls-tree -r --name-only claude/ch-sp-plan -- docs/superpowers/plans` (no 2026-10-30 file); `Test-Path` on the worktree path returned False; worktree `git status --short` empty |
| Ladies Talk has 32 written fills, none between ticks 132960 and 191760, solo at 140400-186480 | section 5 | `py -I ladies_talk_fills.py <notes.mid>` output in this scratch folder |
| The "any written fill" rule is at `src/core/song.cpp` 106-107 and precedes the generator choice | section 5 | Read of lines 105-115 |
| Public Hydra holds the 3 ms leeway, 140 ms window, 1.0 generator | section 4 | `git show 3b49012c:hydra/hypath.py` lines 86-87 and 678; `hysong.py` lines 97-143 |
| Public Hydra has no star-cutoff table | section 4 | `git grep -n -E '0\.1, ?0\.5|4\.4|cutoff' 3b49012c -- hydra hydra_app.py` returns only hypath.py 621 |
| D-record list | section 3 | PowerShell filter over `^\*\*D\d+` lines containing Clone Hero / game / RVA; line numbers printed by the filter |
| overlay-spike-results.md is a 17-line template | gap 3 | `Get-Content` line count 17 |
