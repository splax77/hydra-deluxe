Read docs/superpowers/plans/tasks/_phase7-preamble.md first; it holds the rules.

# Task DOC: the docs follow the code (words for 23, 95, 323, 325, 330, R7.2, 313, R7.37, and what waves 1 and 2 left)

Task id: DOC. Base: main after M7-2 and M6-J2 (the main session names the hash at launch). Branch: claude/p7-doc (worktree `.claude\worktrees\p7-doc`, made as the preamble says).

Plan row: `docs/superpowers/plans/2026-10-04-phase-7.md`, wave 3 table, "DOC Docs follow the code". Decisions in `docs/audit/2026-10-03-fix-decisions.md`: D51 calls 1, 2, 4, 5, 7, 8, 13 (the 323 and 325 items), 19, 20 and 23, the D51 addendum's ST1 and PS1 items, D55 items 3, 4 and 5, D58 items 1 and 3, and D60. Finding texts: `docs/audit/2026-10-03-derivation-audit.md`, headings `#### 23.`, `#### 95.`, `#### 323.`, `#### 325.`, `#### 330.`, `#### R7.2`, `#### 313.`, `#### R7.37`, and for the ADRs `#### 65.`, `#### R7.8` and `#### R7.9`.

## Goal

The User Guide, CONTEXT.md and ADRs 0014 and 0019 say what phase 7 built. Every sentence below restates a decision; none invents a rule. No code changes, no score or stored record changes, and the stamp stays "2.1.0". The docs test, `tests/test_docs_match_code.cpp`, is the proof: every backticked code name you write must exist under `src/`, `tools/` or `tests/` at the base, so never name a function another wave 3 task is still adding (RP's and LB's new names included).

## What the docs say today

Line numbers are from main; no wave branch changes these four files.

**23.** `docs/UserGuide.md` line 51 (Path limit) and line 82 (`squeeze<=N`) say "hardest squeeze". Line 84 says "A path with no squeeze at all passes any squeeze limit." Lines 108 and 207 say "hardest squeeze or early fill". `CONTEXT.md` line 69 says "hardest squeeze at most N ms" and line 251 ("Difficulty") says "hardest required squeeze". The code counts a required early fill like a squeeze (`Activation::difficulty`, E3), and D51 call 5 keeps that.

**95 and 330.** Guide line 255 (`max_tied_paths`) says "how many paths Hydra Deluxe keeps when several reach the same score" and nothing about the Path limit. Line 51 never says that a path tying the best score is kept over the limit. E1 built both rules in `reduce_group` (D51 calls 1 and 2, D55 item 5).

**323.** Guide line 84 already says `squeeze<N` works the same as `squeeze<=N`; CONTEXT.md line 69 defines only `squeeze<=N`. D51 call 13 records "at most N".

**325.** Guide lines 253 to 259 give each rule's default and no lowest value. The bounds live once in `rules_fields()` (`src/core/rules.cpp` lines 84 to 91): `backend_leeway_ms` 0 or more, `max_tied_paths` 1 or more, `fill_cooldown_measures` 1 or more, `fill_max_distance_beats` 0 or more, `fill_length_measures` above 0, `fill_land_slop_beats` 0 or more; `sqout_rule` takes `first_note` or `whole_chord`. D51 call 13: "The User Guide will list all six."

**R7.2.** Guide line 217 says `Open automatically` "opens the report by itself whenever a batch finishes"; it also opens every leaderboard comparison. D51 call 23 keeps the behaviour and gives the words.

**313.** CONTEXT.md lines 167 to 169 ("Early fill (E)") say "its window is fixed" and give no number. The constant is `kEarlyFillWindowMs` (`src/core/model.h` line 117), 60 ms, recorded by D51 call 7 with the user's reason.

**All-0 (left by E3).** Guide line 106 says the best all-0 path has "no squeeze timing"; CONTEXT.md lines 127 to 129 say it is "found under a 0 ms timing limit". E3 made `Path::needs_timing` the owner (D51 call 4, D13, D58 item 1), and the all-0 button's label is now the score alone with "N below optimal" on its detail line. The `pather.h` comment E3 left was fixed at the M7-2a join (1a37d34), so nothing is left there.

**R7.37 (after AU1).** CONTEXT.md lines 344 to 346 ("Mixer") and ADR 0019 lines 108 to 114 say a stem with a decode error goes silent from that point and nothing more. AU1 built the `StemReader` contract in `src/audio/stem_reader.h`: a header length of 0 is counted by decoding once on open (D51 call 19, RFC 9639), and `failed()` stays true while a seek to before the error plays again, Opus included (D51 call 20). ADR 0019 lines 105 to 106 also place `sniff_format` in `src/audio/decode.cpp`; PS1 moved it to `src/core/audio_sniff.cpp` (D51 addendum).

**65 (ADR 0014, left by ST1).** Lines 51 to 52 promise that a result under other rules is kept and reads Ready again when the rules return. ST1 made that true (D51 call 8): the results table's key carries the rules fingerprint (schema 4, `upgrade_results_key`, `rules_fp_of`, `row_readable_sql` in `src/store/record_store.cpp`), and D55 items 3 and 4 settled reindex and older builds. The ADR still describes `row_ready_sql()` as one undivided check.

## What changes

Guide text names no decision numbers; CONTEXT.md and the ADRs cite them as they do today.

**Guide.** Line 51: "hardest squeeze or required early fill", then one added sentence: "A path that ties the best score is kept even when its timing is over the limit, and its figure shows in orange; below the best score, a path over the limit is dropped." Line 82: "hardest squeeze or required early fill is N ms or less". Line 84: "`squeeze<N` means the same as `squeeze<=N`: at most N. A path that needs no timing passes any squeeze limit." Line 106: the all-0 path "activates at the first chance every time, with no skips, and needs no timing". Lines 108 and 207: "hardest squeeze or required early fill". Line 217: exactly "Open each report in your browser as soon as it's built." Line 255: "how many paths Hydra Deluxe keeps at one score: one count per score, whichever side of the Path limit each path falls on; paths inside the limit come first." Lines 253 to 259: each numeric rule's line gains its lowest allowed value from `rules_fields()`, and `sqout_rule`'s says those two words are the only values (open question 6 for the layout).

**CONTEXT.md.** Line 69: "hardest squeeze or required early fill at most N ms; `squeeze<N` means the same". Lines 127 to 129: "The best path whose activations all record zero skips and that needs no timing (`Path::needs_timing`; D51 call 4, D13)." Lines 167 to 169 gain: the window is 60 ms<!-- default: kEarlyFillWindowMs --> (`kEarlyFillWindowMs`), fixed, not the hit-window setting; the user kept 60 ms (D51 call 7) because paths were cluttered with early fills that never matter, and it was tightened from 85 ms; the hardest real early fill on 18,773 charts was 57.7 ms. Line 251: "hardest squeeze or required early fill". Lines 344 to 346: a stem that hits a decode error goes silent from that point, and a seek to before the damage plays it again (D51 call 20); a stem whose header gives no length, a FLAC whose total is 0, is counted by decoding it once on open (D51 call 19).

**ADR 0019.** Lines 105 to 106 name `src/core/audio_sniff.cpp`. After line 114 an "Amendment, 2026-10-04: stem length and damage (D51 calls 19 and 20)" states the `StemReader` contract: `length_frames` of a header saying 0 is a count made by decoding once on open, every other header keeps the fast open, and such a stem seeks by decoding because dr_flac clamps seeks to the header's total; `failed()` stays true for the reader's life, and a seek before the error plays again in every format, Opus included. The cut-short sentence at lines 110 to 111 stays.

**ADR 0014.** An "Amendment, 2026-10-04: results under other rules are kept (D51 call 8)": the results table's unique key gains the rules fingerprint as a `rules_fp` column filled from the structure head by `rules_fp_of`, rebuilt once by `upgrade_results_key` with every row, id and blob kept; `row_ready_sql()` is `row_readable_sql()` (this build can read the row) plus the rules part; a write under rules A keeps the rules-B row; `hydra_batch --reindex` leaves a row it cannot read untouched (D55 item 3); an older Hydra fails to save into an upgraded file instead of writing a wrong key (D55 item 4). Lines 51 to 52 now hold and stay as they are.

## Owned files (only these may change)

- `docs/UserGuide.md`, `CONTEXT.md`
- `docs/adr/0014-squeeze-out-and-collected-phrases-and-rules-are-stored.md`
- `docs/adr/0019-preview-audio-streams-from-the-compressed-file.md`
- `tests/test_docs_match_code.cpp`: only `known_defaults()`, which gains `kEarlyFillWindowMs` (test case 1).

Not yours: `src/ui/library_dialogs.cpp` (the checkbox hint, LB), `src/ui/settings_bar.cpp` (the Path limit tooltip; open question 1), `src/app/library_query.h`/`.cpp` comments (LB), `docs/development.md`, `docs/agents/ui-testing.md` (LB), `src/search/pather.h`.

## Test cases to add or re-pin

1. The marker in test case `docs match code: every marked default equals the code's value`: add `kEarlyFillWindowMs` to `known_defaults()` and write the marker after "60 ms" in CONTEXT.md's early-fill entry. Run the marker first with the map untouched. Red line: `CONTEXT.md:<line>: unknown default marker "kEarlyFillWindowMs"`. Green once the map has it.
2. `docs match code: every backticked code name exists in src/, tools/ or tests/` passes after every edit; it is the check that each name above exists at the base.
3. `docs match code: the User Guide's hydra_rules.ini sample is the defaults` passes unchanged: the sample block keeps its seven lines.

## Test filters you may run

- `build-cpp\Release\hydra_tests.exe -sf=*docs_match_code*`

Nothing else. Never the full suite, never `hydra_uitest`.

## Stored results

None. No code outside the test's one map changes.

## Not in this task

- The Path limit tooltip in `settings_bar.cpp` and the `library_query` comments that still say "hardest squeeze" (open question 1).
- The `Open automatically` hint in `library_dialogs.cpp` (LB, D51 call 23) and `docs/agents/ui-testing.md` (LB).
- The CONTEXT.md sentences for `kSpActivationBars` and `kDefaultDepthValue` (written by the M6-J4 merge, D54).
- ADR 0019's "straight read gives exactly the old samples" sentence (R7.38) and its chained-file text (R7.39): not in phase 7.
- The report pages' words (RP).

## Done when

- Every line named above reads as written here; the two amendments exist; the marker checks 60.
- The docs test's five cases pass, with case 1's red line recorded.
- `git diff --stat <base>..HEAD` lists only the five owned files.

## Open questions (each with a recommended answer)

1. **The tooltip nobody owns (needs the user: a display text).** The Path limit help in `src/ui/settings_bar.cpp` still says "hardest squeeze"; D51 call 5 fixes the words, and no wave 3 task owns the file (phase 6's J4-3 takes it after M7-3). Recommended: the main session makes that one-line change at the M7-3 join, as M6-J4 does for CONTEXT.md, and LB fixes the `library_query` comments, which it owns.
2. **A guide sentence for the Preview's new line (code-only, docs).** PV built D51 call 18's line. Recommended: one sentence in the guide's Preview section: if the chart file changed since it was analyzed, the Preview draws no path and shows "This chart changed since it was analyzed. Analyze it again to see its path."
3. **A guide sentence for the 1-bar cap (code-only, docs).** Recommended: one sentence under SP cap (line 45): at 1 bar no path can activate Star Power, and the Paths tab says so (D51 call 16).
4. **The over-limit sentence's words (code-only, docs).** D51 call 2 and D55 item 5 give the rule; the sentence under line 51 above is the recommended wording.
5. **The early-fill marker (code-only).** Recommended: yes, add the marker and the map entry, so the docs test fails if the window ever moves without its sentence.
6. **Where the six bounds go (code-only, docs).** Recommended: inline, one clause per rule line ("must be 1 or more"), not a separate table, so each default and its floor read together.

## Commits

One commit, trailers `Task: DOC` plus the preamble's others. Report as the preamble says, with the marker's red line.
